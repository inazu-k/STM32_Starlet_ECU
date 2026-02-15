# Pin map — Starlet ECU

以下は CubeMX `.ioc` の設定（stm32_starlet_ecu.ioc）に基づき、MCU 側のピン記載と信号を反映した一覧です。

| Board pin | MCU pin | System function (README) | Function (from .ioc) | Direction | Notes |
|---|---:|---|---|---:|---|
| PA13 | PA13 | (JTAG) | SYS_JTMS-SWDIO | SWD | GPIO_Label=TMS |
| PA14 | PA14 | (JTAG) | SYS_JTCK-SWCLK | SWD | GPIO_Label=TCK |
| PA2 | PA2 |  | USART2_TX | AF | GPIO_Label=USART_TX |
| PA3 | PA3 |  | USART2_RX | AF | GPIO_Label=USART_RX |
| PA4 | PA4 | 油圧センサ入力 (README) | ADCx_IN4 (ADC1_IN4) | ADC | .ioc: ADCx_IN4 |
| PA5 | PA5 |  | GPIO_Output | Output | GPIO_Label=LD2 [Green Led] |
| PA6 | PA6 | A/Fセンサ IN (README) | ADCx_IN6 (ADC1_IN6) | ADC | .ioc: ADCx_IN6 |
| PA7 | PA7 | ノックセンサ IN (README) | ADCx_IN7 (ADC1_IN7) | ADC | .ioc: ADCx_IN7 |
| PA8 | PA8 | インジェクタ #40 (README) | GPIO_Output | Output | .ioc: GPIO_Output |
| PA9 | PA9 | インジェクタ #30 (README) | GPIO_Output | Output | .ioc: GPIO_Output |
| PB3 | PB3 | (Trace) | SYS_JTDO-SWO | AF | GPIO_Label=SWO |
| PB4 | PB4 | NE入力 (README) | GPIO_Input | Input | .ioc: GPIO_Input |
| PB7 | PB7 | インジェクタ #20 (README) | GPIO_Output | Output | .ioc: GPIO_Output |
| PB8 | PB8 | G1 IN (README) | GPIO_Input | Input | .ioc: GPIO_Input |
| PB9 | PB9 | G2 IN (README) | GPIO_Input | Input | .ioc: GPIO_Input |
| PC10 | PC10 | 点火信号 (README) | GPIO_Output | Output | .ioc: GPIO_Output |
| PC12 | PC12 | インジェクタ #10 (README) | GPIO_Output | Output | .ioc: GPIO_Output |
| PC13 | PC13 |  | GPXTI13 (GPIO_EXTI13) | EXTI | GPIO_Label=B1 [Blue PushButton] |
| PC14 | PC14 |  | RCC_OSC32_IN | Oscillator | PC14-OSC32_IN |
| PC15 | PC15 |  | RCC_OSC32_OUT | Oscillator | PC15-OSC32_OUT |
| PC2 | PC2 | 油圧センサ IN (README) | ADCx_IN12 (ADC2_IN12) | ADC | .ioc: ADCx_IN12 |
| PC3 | PC3 | MAPSセンサ IN (README) | ADCx_IN13 (ADC1_IN13) | ADC | .ioc: ADCx_IN13 |
| PC4 | PC4 | 水温センサ IN (README) | ADCx_IN14 (ADC2_IN14) | ADC | .ioc: ADCx_IN14 |
| PC6 | PC6 | NE入力 (README) | GPIO_Input | Input | .ioc: GPIO_Input |
| PH0 | PH0 |  | RCC_OSC_IN | Oscillator | PH0-OSC_IN |
| PH1 | PH1 |  | RCC_OSC_OUT | Oscillator | PH1-OSC_OUT |

### 注意

- `Function (from .ioc)` 列は `stm32_starlet_ecu.ioc` の `<PIN>.Signal` または `SH.*` のマッピングに従っています。
- `System function (README)` は README の CN7/CN10 テーブルにある設計上の機能を併記しています。
- プロジェクトルールに従い、.ioc に未記載（未使用）と判断される端子は `Output` に設定しています。


