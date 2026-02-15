# Pin map — Starlet ECU

以下は `hardware/pinmap.csv` から生成したボードのピン一覧です。

| Signal name | Board pin | MCU pin | System function | Function | Direction | Init | Notes |
|---|---:|---:|---|---:|---|---|---|
| TMS | PA13 | PA13 |  | SYS_JTMS-SWDIO | SWD | N/A | JTAG/SWD |
| TCK | PA14 | PA14 |  | SYS_JTCK-SWCLK | SWD | N/A | JTAG/SWD |
| USART2_TX | PA2 | PA2 |  | USART2_TX | AF | N/A | USART2 TX |
| USART2_RX | PA3 | PA3 |  | USART2_RX | AF | N/A | USART2 RX |
| ADC_IN4 | PA4 | PA4 | 油圧センサ入力 | ADC1_IN4 | ADC | N/A |  |
| LD2_Green | PA5 | PA5 |  | GPIO_Output | Output | LOW | User LED (LD2) |
| ADC_IN6 | PA6 | PA6 | A/Fセンサ IN | ADC1_IN6 | ADC | N/A |  |
| ADC_IN7 | PA7 | PA7 | ノックセンサ IN | ADC1_IN7 | ADC | N/A |  |
| PA8_OUT | PA8 | PA8 | インジェクタ #40 | GPIO_Output | Output | LOW |  |
| PA9_OUT | PA9 | PA9 | インジェクタ #30 | GPIO_Output | Output | LOW |  |
| SWO | PB3 | PB3 |  | SYS_JTDO-SWO | AF | N/A | Trace/SWO |
| PB4_IN | PB4 | PB4 | NE入力 | GPIO_Input | Input | N/A |  |
| PB7_OUT | PB7 | PB7 | インジェクタ #20 | GPIO_Output | Output | LOW |  |
| PB8_IN | PB8 | PB8 | G1 IN | GPIO_Input | Input | N/A |  |
| PB9_IN | PB9 | PB9 | G2 IN | GPIO_Input | Input | N/A |  |
| PC10_OUT | PC10 | PC10 | 点火信号 | GPIO_Output | Output | LOW |  |
| PC12_OUT | PC12 | PC12 | インジェクタ #10 | GPIO_Output | Output | LOW |  |
| B1_Button | PC13 | PC13 |  | GPIO_EXTI13 | EXTI | FALLING | User button (B1) |
| PC14_LSE_IN | PC14 | PC14 |  | RCC_OSC32_IN | Oscillator | N/A | LSE in |
| PC15_LSE_OUT | PC15 | PC15 |  | RCC_OSC32_OUT | Oscillator | N/A | LSE out |
| ADC_IN12 | PC2 | PC2 | 油圧センサ IN | ADC2_IN12 | ADC | N/A |  |
| ADC_IN13 | PC3 | PC3 | MAPSセンサ IN | ADC1_IN13 | ADC | N/A |  |
| ADC_IN14 | PC4 | PC4 | 水温センサ IN | ADC2_IN14 | ADC | N/A |  |
| PC6_IN | PC6 | PC6 | NE入力 | GPIO_Input | Input | N/A |  |
| PH0_HSE_IN | PH0 | PH0 |  | RCC_OSC_IN | Oscillator | N/A | HSE in |
| PH1_HSE_OUT | PH1 | PH1 |  | RCC_OSC_OUT | Oscillator | N/A | HSE out |
