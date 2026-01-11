#include "efi_calc.h"
#include "main.h"
/* `main.c` に定義された ADC ハンドルへの参照 */
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

/* モジュール状態（簡易） */
static uint32_t adc1_raw = 0;
static uint32_t adc2_raw = 0;
static uint8_t gpio_port_state = 0;
static uint8_t is_idle = 0;
static uint16_t injection_us = 0; /* fuel injection pulse width in microseconds */
static int16_t ignition_deg = 0;   /* ignition timing advance in degrees */

/* MUX control helper (file-scope). If the project defines the GPIO macros
   EFI_MUX_SEL0_GPIO_Port / EFI_MUX_SEL0_Pin and EFI_MUX_SEL1_GPIO_Port / EFI_MUX_SEL1_Pin
   this function will drive them; otherwise it is a no-op. An optional
   EFI_MUX_EN_GPIO_Port / EFI_MUX_EN_Pin can be defined to toggle enable. */
#if defined(EFI_MUX_SEL0_GPIO_Port) && defined(EFI_MUX_SEL0_Pin) && defined(EFI_MUX_SEL1_GPIO_Port) && defined(EFI_MUX_SEL1_Pin)
static void set_mux(uint8_t sel)
{
    HAL_GPIO_WritePin(EFI_MUX_SEL0_GPIO_Port, EFI_MUX_SEL0_Pin, (sel & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EFI_MUX_SEL1_GPIO_Port, EFI_MUX_SEL1_Pin, (sel & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
#ifdef EFI_MUX_EN_GPIO_Port
    HAL_GPIO_WritePin(EFI_MUX_EN_GPIO_Port, EFI_MUX_EN_Pin, GPIO_PIN_SET);
#endif
    HAL_Delay(1);
}
#else
static void set_mux(uint8_t sel) { (void)sel; }
#endif

void SensorDataSample(void)
{
        /* ポーリングによる4チャネルのサンプリング:
             index 0 -> ADC_CHANNEL_3  (ADC1)
             index 1 -> ADC_CHANNEL_5  (ADC1)
             index 2 -> ADC_CHANNEL_6  (ADC1)
             index 3 -> ADC_CHANNEL_14 (ADC2)

             外部アナログMUXを使用する場合は、プロジェクト側で以下のマクロを定義すると
             MUX制御が有効になります:
                 EFI_MUX_SEL0_GPIO_Port, EFI_MUX_SEL0_Pin
                 EFI_MUX_SEL1_GPIO_Port, EFI_MUX_SEL1_Pin
                 任意で EFI_MUX_EN_GPIO_Port, EFI_MUX_EN_Pin
        */

    const uint32_t channels[4] = {ADC_CHANNEL_3, ADC_CHANNEL_5, ADC_CHANNEL_6, ADC_CHANNEL_14};
    ADC_HandleTypeDef *handles[4] = {&hadc1, &hadc1, &hadc1, &hadc2};
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

    /* drive MUX if present */
    /* set_mux() is defined at file-scope */

    /* 簡易 IIR 平滑化: new = (prev*(N-1) + sample) / N */
    const uint8_t IIR_N = 4;
    static uint32_t smoothed[4] = {0,0,0,0};

    for (uint8_t i = 0; i < 4; ++i) {
        ADC_HandleTypeDef *h = handles[i];
        uint32_t ch = channels[i];

        /* MUX を駆動（存在する場合） */
        /* set_mux() はファイル内で定義されている */
        set_mux(i);

        /* configure channel */
        sConfig.Channel = ch;
        if (HAL_ADC_ConfigChannel(h, &sConfig) != HAL_OK) {
            /* leave previous value on error */
            continue;
        }

        if (HAL_ADC_Start(h) != HAL_OK) {
            continue;
        }

        if (HAL_ADC_PollForConversion(h, 10) == HAL_OK) {
            uint32_t val = HAL_ADC_GetValue(h);
            /* clamp 12-bit value */
            if (val > 4095U) val = 4095U;
            /* IIR smoothing */
            smoothed[i] = (smoothed[i] * (IIR_N - 1) + val) / IIR_N;
            /* store into module state */
            if (i == 0) adc1_raw = smoothed[i];
            else if (i == 1) adc2_raw = smoothed[i];
            else if (i == 2) adc1_raw = smoothed[i];
            else if (i == 3) adc2_raw = smoothed[i];
        }

        HAL_ADC_Stop(h);
    }
}

void ReadGPIOPort(void)
{
    /* 入力ピン数本の状態をバイトにパックして簡易チェック用とする */
    gpio_port_state = 0;
    /* 例: MX_GPIO_Init で設定されている B1（ユーザボタン）、PC6 などを読む */
    if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) gpio_port_state |= 0x01;
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_SET) gpio_port_state |= 0x02;
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_SET) gpio_port_state |= 0x04;
}

void JdgIdle(void)
{
    /* アイドル判定（簡易）: ADC2（例: スロットル）が閾値以下かつ該当GPIOがセットされていなければアイドルと判断 */
    const uint32_t throttle_threshold = 100; /* 12ビットADC想定 */
    if ((adc2_raw < throttle_threshold) && ((gpio_port_state & 0x04) == 0)) {
        is_idle = 1;
    } else {
        is_idle = 0;
    }
}

void CalcFuelInjection(void)
{
    /* 簡易マップ: adc1_raw (0..4095) を噴射パルス幅（500..3000 us）に変換 */
    const uint16_t min_us = 500;
    const uint16_t max_us = 3000;
    uint32_t val = adc1_raw;
    if (val > 4095) val = 4095;
    /* If idle, use lower fixed injection */
    if (is_idle) {
        injection_us = 700;
    } else {
        injection_us = (uint16_t)(min_us + (val * (max_us - min_us) / 4095U));
    }
}

void CalcIgnitionTiming(void)
{
    /* 簡易例: センサ値に応じて点火進角を算出（adc1_raw を -5..25 度にマップ） */
    int32_t val = (int32_t)adc1_raw;
    if (val > 4095) val = 4095;
    ignition_deg = (int16_t)(-5 + (val * 30 / 4095));
    if (is_idle) {
        ignition_deg = 5; /* small fixed advance at idle */
    }
}

/* デバッグ/テレメトリ用の簡易ゲッタ（ヘッダには宣言していない） */
uint32_t EFI_GetADC1(void) { return adc1_raw; }
uint32_t EFI_GetADC2(void) { return adc2_raw; }
uint8_t EFI_GetIdle(void) { return is_idle; }
uint16_t EFI_GetInjectionUs(void) { return injection_us; }
int16_t EFI_GetIgnitionDeg(void) { return ignition_deg; }
