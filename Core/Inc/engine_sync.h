#ifndef ENGINE_SYNC_H
#define ENGINE_SYNC_H

#include <stdbool.h>
#include <stdint.h>

typedef enum { SYNC_G_NONE, SYNC_G1, SYNC_G2, SYNC_G_INVALID } SyncMarker;
typedef enum { SYNC_SEARCH, SYNC_CANDIDATE, SYNC_LOCKED } SyncState;
typedef enum {
    SYNC_OK, SYNC_BAD_CONFIG, SYNC_BAD_PERIOD, SYNC_BAD_MARKER,
    SYNC_MISSING_MARKER, SYNC_TIMEOUT, SYNC_CAPTURE_LOSS
} SyncFault;

typedef struct {
    uint32_t min_period_ticks;
    uint32_t max_period_ticks;
    uint32_t stop_timeout_ticks;
} SyncConfig;

typedef struct {
    SyncConfig config;
    SyncState state;
    SyncFault fault;
    uint32_t last_tick;
    uint32_t period_ticks;
    uint8_t tooth; /* Valid absolute phase only when state == SYNC_LOCKED. */
    uint8_t teeth_since_marker;
    uint8_t confirmations;
    bool have_ne;
    bool configured;
} EngineSync;

/* All calls must be serialized. No output control is performed by this module. */
bool engine_sync_init(EngineSync *sync, SyncConfig config);
/*
 * One call per acquired NE falling edge, in chronological order. marker is
 * the paired GPIO snapshot: NONE=both low, G1=only G1 high, G2=only G2 high,
 * INVALID=both high or an unreliable snapshot. The acquisition layer must
 * guarantee the documented G setup/hold time and pair samples with NE events.
 * tick is the acquisition-layer timestamp, not main processing time. A GPIO
 * IRQ timestamp includes interrupt latency. Config units must match.
 * Unsigned 32-bit tick wrap is supported; gaps >= 2^31 ticks are not supported.
 */
void engine_sync_tooth(EngineSync *sync, uint32_t tick, SyncMarker marker);
/* Call regularly, after queued teeth drain, on the same tick timeline.
 * After a long gap, poll BEFORE feeding a new tooth to clear stale timing.
 */
void engine_sync_poll(EngineSync *sync, uint32_t now);
/* Signal capture overrun, lost queue entries or other untrusted input. */
void engine_sync_capture_loss(EngineSync *sync);

#endif
