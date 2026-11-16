#include "efi_calc.h"
#include "main.h"
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
static EfiSample samples[EFI_SENSOR_COUNT];
static bool throttle_closed, throttle_valid, idle;

/* チャネルごとに独立した保存先を持ち、取得失敗時は値を有効としない。
 * MAP=PC3/IN13、水温=PC4/IN14、A/F=PA6/IN6、ノック=PA7/IN7。
 * 校正・フィルタ周期が未確定なので、生値を加工せず公開する。 */
void SensorDataSample(void)
{
    const uint32_t channels[EFI_SENSOR_COUNT]={ADC_CHANNEL_13,ADC_CHANNEL_14,ADC_CHANNEL_6,ADC_CHANNEL_7};
    ADC_HandleTypeDef *handles[EFI_SENSOR_COUNT]={&hadc1,&hadc2,&hadc1,&hadc1};
    for (unsigned i=0;i<EFI_SENSOR_COUNT;++i) {
        ADC_HandleTypeDef *h=handles[i];
        samples[i].valid=false;
        ADC_ChannelConfTypeDef channel={0};
        channel.Channel=channels[i];
        channel.Rank=1;
        channel.SamplingTime=ADC_SAMPLETIME_480CYCLES;
        if (HAL_ADC_ConfigChannel(h,&channel)!=HAL_OK) continue;
        if (HAL_ADC_Start(h)!=HAL_OK) {
            (void)HAL_ADC_Stop(h);
            continue;
        }
        bool converted=HAL_ADC_PollForConversion(h,10)==HAL_OK;
        uint32_t raw=converted ? HAL_ADC_GetValue(h) : 0;
        bool stopped=HAL_ADC_Stop(h)==HAL_OK;
        if (converted && stopped && raw<=4095U) {
            samples[i]=(EfiSample){(uint16_t)raw,true,HAL_GetTick()};
        }
    }
}

/* main側から直列に呼ぶ。古い値はvalid=falseなら利用しない。 */
EfiSample EFI_GetSample(EfiSensor sensor)
{
    if ((unsigned)sensor>=EFI_SENSOR_COUNT) return (EfiSample){0};
    return samples[sensor];
}

void EFI_SetThrottleClosed(bool closed, bool valid)
{
    throttle_closed=closed;
    throttle_valid=valid;
    idle=valid && closed;
}

/* NE候補・ボタン・水温をTPSとして解釈しない。 */
void ReadGPIOPort(void) { }
void JdgIdle(void) { idle=throttle_valid && throttle_closed; }
bool EFI_GetIdle(void) { return idle; }
bool EFI_IdleValid(void) { return throttle_valid; }
