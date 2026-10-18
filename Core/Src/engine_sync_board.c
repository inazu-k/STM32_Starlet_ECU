#include "engine_sync_board.h"
#include "main.h"
#include <stddef.h>

/* Bench signal-generator settings; NOT calibrated engine operating limits. */
#define SYNC_BENCH_MIN_US 500U
#define SYNC_BENCH_MAX_US 100000U
#define SYNC_BENCH_STOP_US 250000U

static EngineSyncService service;
static TIM_HandleTypeDef timebase;

static uint32_t enter(void *context)
{
    (void)context;
    uint32_t mask = __get_PRIMASK();
    __disable_irq();
    __DMB();
    return mask;
}

static void leave(void *context, uint32_t mask)
{
    (void)context;
    __DMB();
    __set_PRIMASK(mask);
}

static uint32_t now(void *context)
{
    (void)context;
    return TIM5->CNT;
}

bool engine_sync_board_init(void)
{
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    __HAL_RCC_TIM5_CLK_ENABLE();
    uint32_t clock = HAL_RCC_GetPCLK1Freq();
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != 0U) clock *= 2U;
    /* Current project uses the default TIMPRE=0 APB timer clock rule. */
    if ((RCC->DCKCFGR & RCC_DCKCFGR_TIMPRE) != 0U ||
        clock < 1000000U || clock % 1000000U != 0U) return false;
    timebase.Instance = TIM5;
    timebase.Init.Prescaler = clock / 1000000U - 1U;
    timebase.Init.CounterMode = TIM_COUNTERMODE_UP;
    timebase.Init.Period = UINT32_MAX;
    timebase.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    timebase.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&timebase) != HAL_OK ||
        HAL_TIM_Base_Start(&timebase) != HAL_OK) return false;

    SyncConfig config = {SYNC_BENCH_MIN_US, SYNC_BENCH_MAX_US, SYNC_BENCH_STOP_US};
    SyncPlatform platform = {enter, leave, now, NULL};
    if (!engine_sync_service_init(&service, config, platform)) return false;

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &gpio);
    gpio.Pin = GPIO_PIN_6;
    gpio.Mode = GPIO_MODE_IT_FALLING;
    HAL_GPIO_Init(GPIOC, &gpio);
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
    HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    return true;
}

void engine_sync_board_ne_irq(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_6) == RESET) return;
    /* Clear first, so a later edge can pend again while this handler runs. */
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
    uint32_t tick = TIM5->CNT;
    uint32_t levels = GPIOB->IDR;
    engine_sync_service_push_isr(&service, tick,
        (levels & GPIO_PIN_8) != 0U, (levels & GPIO_PIN_9) != 0U);
}

void engine_sync_board_run(void)
{
    engine_sync_service_run(&service);
}

SyncStatus engine_sync_board_status(void)
{
    return engine_sync_service_get_status(&service);
}
