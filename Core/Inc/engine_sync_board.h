#ifndef ENGINE_SYNC_BOARD_H
#define ENGINE_SYNC_BOARD_H

#include "engine_sync_service.h"

/* Bench only: call init after SystemClock_Config and MX_GPIO_Init. */
bool engine_sync_board_init(void);
void engine_sync_board_run(void);
void engine_sync_board_ne_irq(void);
SyncStatus engine_sync_board_status(void);

#endif
