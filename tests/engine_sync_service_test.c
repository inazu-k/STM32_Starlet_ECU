#include "engine_sync_service.h"
#include <stdio.h>
#include <stdlib.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #x); exit(1); } } while (0)

typedef struct {
    EngineSyncService service;
    uint32_t tick, mask;
    unsigned enters, leaves;
    bool overflow_on_leave, produce_on_leave, produce_on_enter;
} Fixture;

static void push(Fixture *f, uint32_t tick, unsigned tooth)
{
    f->tick = tick;
    engine_sync_service_push_isr(&f->service, tick,
                                tooth % 24 == 0, tooth % 24 == 12);
}

static uint32_t enter(void *context)
{
    Fixture *f = context;
    if (f->produce_on_enter) {
        f->produce_on_enter = false;
        push(f, f->tick, 1);
    }
    uint32_t previous = f->mask;
    f->mask = 1;
    ++f->enters;
    return previous;
}

static void leave(void *context, uint32_t key)
{
    Fixture *f = context;
    CHECK(f->mask == 1);
    f->mask = key;
    ++f->leaves;
    if (f->overflow_on_leave) {
        f->overflow_on_leave = false;
        for (unsigned n = 0; n <= SYNC_QUEUE_CAPACITY; ++n)
            push(f, f->tick + 1000, 1);
    }
    if (f->produce_on_leave) push(f, f->tick + 1000, 1);
}

static uint32_t now(void *context)
{
    Fixture *f = context;
    CHECK(f->mask == 1); /* Empty check and clock sample are protected. */
    return f->tick;
}

static void init(Fixture *f)
{
    *f = (Fixture){0};
    SyncPlatform platform = {enter, leave, now, f};
    CHECK(engine_sync_service_init(&f->service,
                                  (SyncConfig){100, 2000, 3000}, platform));
}

static void acquire(Fixture *f, uint32_t tick, unsigned start)
{
    for (unsigned n = 0; n <= 24; ++n) push(f, tick + n * 1000, start + n);
    engine_sync_service_run(&f->service);
    CHECK(engine_sync_service_get_status(&f->service).core.state == SYNC_LOCKED);
}

int main(void)
{
    Fixture f;
    /* Backlog must use event timestamps, including wrap, rather than wall time. */
    init(&f);
    acquire(&f, UINT32_MAX - 12000, 12);
    SyncStatus status = engine_sync_service_get_status(&f.service);
    CHECK(status.core.tooth == 12 && status.core.period_ticks == 1000);
    CHECK(status.high_water == 25 && status.stops == 0 && status.queued == 0);

    /* An empty queue is polled and a fixed timeout occurs exactly once. */
    f.tick += 3000;
    engine_sync_service_run(&f.service);
    status = engine_sync_service_get_status(&f.service);
    CHECK(status.stops == 1 && status.sync_losses == 1);
    CHECK(status.core.fault == SYNC_TIMEOUT && !status.core.have_ne);
    engine_sync_service_run(&f.service);
    CHECK(engine_sync_service_get_status(&f.service).stops == 1);
    acquire(&f, f.tick + 10000, 0);

    /* Both sides of a stop/restart can arrive in one main-loop batch. */
    init(&f);
    for (unsigned n = 0; n <= 24; ++n) push(&f, n * 1000, n);
    uint32_t restart = f.tick + 10000;
    acquire(&f, restart, 12);
    status = engine_sync_service_get_status(&f.service);
    CHECK(status.stops == 1 && status.core.tooth == 12);

    /* Exactly 64 events fit; the next is lost, without overwriting old events. */
    init(&f);
    acquire(&f, 0, 0);
    for (unsigned n = 1; n <= SYNC_QUEUE_CAPACITY; ++n)
        push(&f, 24000 + n * 1000, n);
    CHECK(f.service.count == SYNC_QUEUE_CAPACITY && !f.service.loss_pending);
    uint32_t oldest = f.service.queue[f.service.tail].tick;
    engine_sync_service_push_isr(&f.service, 90000, false, false);
    CHECK(f.service.queue[f.service.tail].tick == oldest);
    status = engine_sync_service_get_status(&f.service);
    CHECK(status.loss_pending && status.dropped_events == 1);
    CHECK(status.core.state == SYNC_SEARCH && status.core.fault == SYNC_CAPTURE_LOSS);
    engine_sync_service_run(&f.service);
    status = engine_sync_service_get_status(&f.service);
    CHECK(status.recoveries == 1 && status.generation == 1 && status.queued == 0);
    CHECK(status.sync_losses == 1 && !status.loss_pending);
    /* Clear old timing at the stop boundary, then reacquire in the new generation. */
    acquire(&f, f.tick + 10000, 0);

    /* Overflow between pop and decode must invalidate any locally decoded result. */
    push(&f, f.tick + 1000, 1);
    f.overflow_on_leave = true;
    engine_sync_service_run(&f.service);
    status = engine_sync_service_get_status(&f.service);
    CHECK(status.core.state == SYNC_SEARCH && status.recoveries == 2);
    CHECK(status.queued == 0 && status.generation == 2);

    /* A producer that keeps feeding cannot make one service invocation unbounded. */
    init(&f);
    push(&f, 0, 1);
    f.produce_on_leave = true;
    engine_sync_service_run(&f.service);
    f.produce_on_leave = false;
    CHECK(f.service.count > 0 && f.enters == SYNC_QUEUE_CAPACITY + 1);
    CHECK(f.leaves == f.enters);

    /* Arrival just before the empty check is consumed before current-time poll. */
    init(&f);
    acquire(&f, 0, 0);
    f.tick += 1000;
    f.produce_on_enter = true;
    engine_sync_service_run(&f.service);
    status = engine_sync_service_get_status(&f.service);
    CHECK(status.core.state == SYNC_LOCKED && status.core.tooth == 1);

    /* GPIO classification and pending loss visibility. */
    init(&f);
    engine_sync_service_push_isr(&f.service, 0, true, true);
    engine_sync_service_run(&f.service);
    CHECK(engine_sync_service_get_status(&f.service).core.fault == SYNC_BAD_MARKER);
    engine_sync_service_loss_isr(&f.service);
    engine_sync_service_loss_isr(&f.service);
    CHECK(engine_sync_service_get_status(&f.service).dropped_events == 2);
    engine_sync_service_run(&f.service);
    CHECK(engine_sync_service_get_status(&f.service).recoveries == 1);

    /* Critical sections restore a pre-existing interrupt mask. */
    f.mask = 1;
    engine_sync_service_run(&f.service);
    (void)engine_sync_service_get_status(&f.service);
    CHECK(f.mask == 1 && f.enters == f.leaves);
    CHECK(!engine_sync_service_init(&f.service, (SyncConfig){100,2000,3000},
                                    (SyncPlatform){0}));
    CHECK(!engine_sync_service_init(&f.service, (SyncConfig){0,2000,3000},
                                    (SyncPlatform){enter, leave, now, &f}));
    puts("engine_sync_service: all checks passed");
    return 0;
}
