#ifndef ENGINE_SYNC_SERVICE_H
#define ENGINE_SYNC_SERVICE_H

#include "engine_sync.h"

#define SYNC_QUEUE_CAPACITY 64U

typedef struct {
    uint32_t (*enter)(void *context);
    void (*leave)(void *context, uint32_t key);
    uint32_t (*now)(void *context);
    void *context;
} SyncPlatform;

typedef struct {
    uint32_t tick;
    uint32_t generation;
    SyncMarker marker;
} SyncInputEvent;

typedef struct {
    EngineSync core;
    uint32_t dropped_events;
    uint32_t recoveries;
    uint32_t stops;
    uint32_t sync_losses;
    uint32_t generation;
    uint32_t queued;
    uint32_t high_water;
    bool loss_pending;
} SyncStatus;

typedef struct {
    EngineSync core; /* Main context only. Use get_status rather than direct access. */
    SyncPlatform platform;
    SyncInputEvent queue[SYNC_QUEUE_CAPACITY];
    volatile uint32_t head, tail, count;
    volatile uint32_t dropped_events, high_water;
    volatile bool loss_pending;
    uint32_t generation, recoveries, stops, sync_losses;
    bool initialized;
} EngineSyncService;

/* Init with producer IRQ disabled. Hooks must exclude producer and restore mask. */
bool engine_sync_service_init(EngineSyncService *service, SyncConfig config,
                              SyncPlatform platform);
/* Single producer ISR only; never call from a second/nested producer. */
void engine_sync_service_push_isr(EngineSyncService *service, uint32_t tick,
                                  bool g1, bool g2);
void engine_sync_service_loss_isr(EngineSyncService *service);
/* Main only, bounded to at most one queue capacity of events per invocation. */
void engine_sync_service_run(EngineSyncService *service);
/* Main only; locks to return a coherent result including pending input loss. */
SyncStatus engine_sync_service_get_status(EngineSyncService *service);

#endif
