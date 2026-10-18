#include "engine_sync.h"
#include <stddef.h>

static void invalidate(EngineSync *sync, SyncFault fault)
{
    sync->state = SYNC_SEARCH;
    sync->fault = fault;
    sync->tooth = 0;
    sync->teeth_since_marker = 0;
    sync->confirmations = 0;
    sync->period_ticks = 0;
}

bool engine_sync_init(EngineSync *sync, SyncConfig config)
{
    if (sync == NULL) return false;
    *sync = (EngineSync){0};
    sync->config = config;
    sync->configured = config.min_period_ticks > 0 &&
        config.min_period_ticks <= config.max_period_ticks &&
        config.max_period_ticks < config.stop_timeout_ticks &&
        config.stop_timeout_ticks < UINT32_C(0x80000000);
    invalidate(sync, sync->configured ? SYNC_OK : SYNC_BAD_CONFIG);
    return sync->configured;
}

void engine_sync_tooth(EngineSync *sync, uint32_t tick, SyncMarker marker)
{
    if (sync == NULL || !sync->configured) return;
    if (sync->have_ne) {
        uint32_t period = tick - sync->last_tick;
        if (period < sync->config.min_period_ticks ||
            period > sync->config.max_period_ticks) {
            invalidate(sync, SYNC_BAD_PERIOD);
            return; /* Never reuse a faulty event as a new phase anchor. */
        }
        sync->period_ticks = period;
    }
    sync->last_tick = tick;
    sync->have_ne = true;
    if (marker != SYNC_G_NONE && marker != SYNC_G1 && marker != SYNC_G2) {
        invalidate(sync, SYNC_BAD_MARKER);
        return;
    }

    if (sync->state == SYNC_SEARCH) {
        if (marker == SYNC_G_NONE) return;
        sync->tooth = marker == SYNC_G1 ? 0 : 12;
        sync->teeth_since_marker = 0;
        sync->confirmations = 0;
        sync->state = SYNC_CANDIDATE;
        return;
    }

    sync->tooth = (uint8_t)((sync->tooth + 1) % 24);
    ++sync->teeth_since_marker;
    if (sync->teeth_since_marker == 12) {
        SyncMarker expected = sync->tooth == 0 ? SYNC_G1 : SYNC_G2;
        if (marker != expected) {
            invalidate(sync, marker == SYNC_G_NONE ?
                       SYNC_MISSING_MARKER : SYNC_BAD_MARKER);
            return;
        }
        sync->teeth_since_marker = 0;
        if (sync->confirmations < 2) ++sync->confirmations;
        if (sync->confirmations == 2) {
            sync->state = SYNC_LOCKED;
            sync->fault = SYNC_OK;
        }
    } else if (marker != SYNC_G_NONE) {
        invalidate(sync, SYNC_BAD_MARKER);
    }
}

void engine_sync_poll(EngineSync *sync, uint32_t now)
{
    if (sync != NULL && sync->configured && sync->have_ne &&
        (uint32_t)(now - sync->last_tick) >= sync->config.stop_timeout_ticks) {
        invalidate(sync, SYNC_TIMEOUT);
        sync->have_ne = false;
    }
}

void engine_sync_capture_loss(EngineSync *sync)
{
    if (sync != NULL && sync->configured) invalidate(sync, SYNC_CAPTURE_LOSS);
}
