# Pin map — Starlet ECU

以下は README の CN7 / CN10 テーブルに記載された全ピン情報を反映した一覧です。

| Board pin | MCU pin | System function | Function | Notes |
|---|---:|---|---|---|
| CN7-1 | PC10 | 点火信号 |  |  |
| CN7-2 | PC11 |  |  |  |
| CN7-3 | PC12 | インジェクタ #10 |  |  |
| CN7-4 | PD2 |  |  |  |
| CN7-5 | VDD | 電源 VDD |  |  |
| CN7-6 | E5V | 外部からの5V入力 |  |  |
| CN7-7 | BOOT0 | ブート選択 |  |  |
| CN7-8 | GND | GND |  |  |
| CN7-9 | - |  |  |  |
| CN7-10 | - |  |  |  |
| CN7-11 | - |  |  |  |
| CN7-12 | IOREF |  |  |  |
| CN7-13 | PA13 |  |  |  |
| CN7-14 | RESET | 外部リセット |  |  |
| CN7-15 | PA14 |  |  |  |
| CN7-16 | +3.3V | 3.3V電源 |  |  |
| CN7-17 | PA15 |  |  |  |
| CN7-18 | +5V | 5V電源 |  |  |
| CN7-19 | GND | GND |  |  |
| CN7-20 | GND | GND |  |  |
| CN7-21 | PB7 | インジェクタ #20 |  |  |
| CN7-22 | GND |  |  |  |
| CN7-23 | PC13 |  |  |  |
| CN7-24 | VIN | 外部からの7~12V入力 |  |  |
| CN7-25 | PC14 |  |  |  |
| CN7-26 | - |  |  |  |
| CN7-27 | PC15 |  |  |  |
| CN7-28 | PA0 |  |  |  |
| CN7-29 | PH0 |  |  |  |
| CN7-30 | PA1 |  |  |  |
| CN7-31 | PH1 |  |  |  |
| CN7-32 | PA4 | 油圧センサ入力 | ADC123_IN3 |  |
| CN7-33 | VBAT |  |  |  |
| CN7-34 | PB0 |  |  |  |
| CN7-35 | PC2 | 油圧センサ IN |  |  |
| CN7-36 | PC1 or PB9 | 半田オプション |  |  |
| CN7-37 | PC3 | MAPSセンサ IN | ADC12_IN5 |  |
| CN7-38 | PC0 or PB8 | 半田オプション |  |  |

## CN10

| Board pin | MCU pin | System function | Function | Notes |
|---|---:|---|---|---|
| CN10-1 | PC9 |  |  |  |
| CN10-2 | PC8 |  |  |  |
| CN10-3 | PB8 | G1 IN |  |  |
| CN10-4 | PC6 | NE入力 |  |  |
| CN10-5 | PB9 | G2 IN |  |  |
| CN10-6 | PC5 |  |  |  |
| CN10-7 | AVDD | アナログ電源 |  |  |
| CN10-8 | U5V | USBからの5V |  |  |
| CN10-9 | GND |  |  |  |
| CN10-10 | - |  |  |  |
| CN10-11 | PA5 |  |  |  |
| CN10-12 | PA12 |  |  |  |
| CN10-13 | PA6 | A/Fセンサ IN | ADC12_IN6 |  |
| CN10-14 | PA11 |  |  |  |
| CN10-15 | PA7 | ノックセンサ IN | ADC12_IN7 |  |
| CN10-16 | PB12 |  |  |  |
| CN10-17 | PB6 |  |  |  |
| CN10-18 | - |  |  |  |
| CN10-19 | PC7 |  |  |  |
| CN10-20 | GND |  |  |  |
| CN10-21 | PA9 | インジェクタ #30 |  |  |
| CN10-22 | PB2 |  |  |  |
| CN10-23 | PA8 | インジェクタ #40 |  |  |
| CN10-24 | PB1 |  |  |  |
| CN10-25 | PB10 |  |  |  |
| CN10-26 | PB15 |  |  |  |
| CN10-27 | PB4 | NE入力 |  |  |
| CN10-28 | PB14 |  |  |  |
| CN10-29 | PB5 |  |  |  |
| CN10-30 | PB13 |  |  |  |
| CN10-31 | PB3 |  |  |  |
| CN10-32 | AGND |  |  |  |
| CN10-33 | PA11 |  |  |  |
| CN10-34 | PC4 | 水温センサ IN | ADC12_IN14 |  |
| CN10-35 | PA2 |  |  |  |
| CN10-36 | 0 |  |  |  |
| CN10-37 | PA3 |  |  |  |
| CN10-38 | - |  |  |  |

### 備考

- テーブルの `Function` 列は README の「MCU機能」をそのまま記載しています。空欄は README に該当情報がなかったものです。
- 一部ピンは "PC1 or PB9" のようにオプションで選択可能な設計になっています。pinmap.md では両方の表記を併記しています。

