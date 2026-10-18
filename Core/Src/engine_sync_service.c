#include "engine_sync_service.h"
#include <stddef.h>

static void increment(uint32_t *value)
{
    if (*value != UINT32_MAX) ++*value;
}

static void account(EngineSyncService *s, SyncState before, bool had_ne)
{
    if (before == SYNC_LOCKED && s->core.state != SYNC_LOCKED)
        increment(&s->sync_losses);
    if (had_ne && !s->core.have_ne && s->core.fault == SYNC_TIMEOUT)
        increment(&s->stops);
}

static void poll(EngineSyncService *s, uint32_t tick)
{
    SyncState before = s->core.state;
    bool had_ne = s->core.have_ne;
    engine_sync_poll(&s->core, tick);
    account(s, before, had_ne);
}

/* Caller holds the platform critical section. Producer remains suspended. */
static bool recover(EngineSyncService *s)
{
    if (!s->loss_pending) return false;
    s->tail = s->head;
    s->count = 0;
    ++s->generation;
    SyncState before = s->core.state;
    engine_sync_capture_loss(&s->core);
    account(s, before, s->core.have_ne);
    increment(&s->recoveries);
    s->loss_pending = false;
    return true;
}

bool engine_sync_service_init(EngineSyncService *s, SyncConfig config,
                              SyncPlatform platform)
{
    if (s == NULL) return false;
    *s = (EngineSyncService){0};
    s->platform = platform;
    if (platform.enter == NULL || platform.leave == NULL || platform.now == NULL)
        return false;
    s->initialized = engine_sync_init(&s->core, config);
    return s->initialized;
}

void engine_sync_service_loss_isr(EngineSyncService *s)
{
    if (s == NULL || !s->initialized) return;
    s->loss_pending = true;
    if (s->dropped_events != UINT32_MAX) ++s->dropped_events;
}

void engine_sync_service_push_isr(EngineSyncService *s, uint32_t tick,
                                  bool g1, bool g2)
{
    if (s == NULL || !s->initialized) return;
    if (s->loss_pending || s->count == SYNC_QUEUE_CAPACITY) {
        engine_sync_service_loss_isr(s);
        return;
    }
    SyncMarker marker = g1 ? (g2 ? SYNC_G_INVALID : SYNC_G1) :
                            (g2 ? SYNC_G2 : SYNC_G_NONE);
    s->queue[s->head] = (SyncInputEvent){tick, s->generation, marker};
    s->head = (s->head + 1U) % SYNC_QUEUE_CAPACITY;
    ++s->count;
    if (s->count > s->high_water) s->high_water = s->count;
}

void engine_sync_service_run(EngineSyncService *s)
{
    if (s == NULL || !s->initialized) return;
    for (unsigned n = 0; n < SYNC_QUEUE_CAPACITY; ++n) {
        uint32_t key = s->platform.enter(s->platform.context);
        if (recover(s)) {
            s->platform.leave(s->platform.context, key);
            return;
        }
        if (s->count == 0) {
            /* Empty check, clock sample and poll cannot race with the ISR. */
            poll(s, s->platform.now(s->platform.context));
            s->platform.leave(s->platform.context, key);
            return;
        }
        SyncInputEvent event = s->queue[s->tail];
        s->tail = (s->tail + 1U) % SYNC_QUEUE_CAPACITY;
        --s->count;
        s->platform.leave(s->platform.context, key);

        if (event.generation != s->generation) continue;
        poll(s, event.tick); /* Stop/restart boundary before a new tooth. */
        SyncState before = s->core.state;
        bool had_ne = s->core.have_ne;
        engine_sync_tooth(&s->core, event.tick, event.marker);
        account(s, before, had_ne);
    }
    uint32_t key = s->platform.enter(s->platform.context);
    if (!recover(s) && s->count == 0)
        poll(s, s->platform.now(s->platform.context));
    s->platform.leave(s->platform.context, key);
}

SyncStatus engine_sync_service_get_status(EngineSyncService *s)
{
    SyncStatus status = {0};
    if (s == NULL || !s->initialized) return status;
    uint32_t key = s->platform.enter(s->platform.context);
    status.core = s->core;
    status.dropped_events = s->dropped_events;
    status.recoveries = s->recoveries;
    status.stops = s->stops;
    status.sync_losses = s->sync_losses;
    status.generation = s->generation;
    status.queued = s->count;
    status.high_water = s->high_water;
    status.loss_pending = s->loss_pending;
    if (s->loss_pending) engine_sync_capture_loss(&status.core);
    s->platform.leave(s->platform.context, key);
    return status;
}
