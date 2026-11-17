#include "efi_calc.h"
#include "main.h"
#include <assert.h>
#include <stdio.h>
ADC_HandleTypeDef hadc1={1}, hadc2={2};
static unsigned channel, fail_channel, fail_stage, stops;
static uint32_t tick;
HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef *h, ADC_ChannelConfTypeDef *c)
{
    assert(h->id==(c->Channel==14 ? 2U:1U));
    assert(c->Rank==1 && c->SamplingTime==480);
    channel=c->Channel;
    return channel==fail_channel && fail_stage==1 ? HAL_ERROR:HAL_OK;
}
HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *h)
{
    (void)h;
    return channel==fail_channel && fail_stage==2 ? HAL_ERROR:HAL_OK;
}
HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef *h,uint32_t timeout)
{
    (void)h; assert(timeout==10);
    return channel==fail_channel && fail_stage==3 ? HAL_ERROR:HAL_OK;
}
uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *h) { (void)h; return channel*100; }
HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef *h)
{
    (void)h; ++stops;
    return channel==fail_channel && fail_stage==4 ? HAL_ERROR:HAL_OK;
}
uint32_t HAL_GetTick(void) { return tick; }
int main(void)
{
    assert(!EFI_IdleValid() && !EFI_GetIdle());
    assert(!EFI_GetSample(EFI_MAP).valid);
    tick=42; SensorDataSample();
    unsigned expected[]={1300,1400,600,700};
    for (unsigned i=0;i<EFI_SENSOR_COUNT;++i) {
        EfiSample sample=EFI_GetSample((EfiSensor)i);
        assert(sample.valid && sample.raw==expected[i] && sample.sampled_ms==42);
    }
    assert(stops==4);
    for (unsigned stage=1;stage<=4;++stage) {
        fail_channel=14; fail_stage=stage; tick=100+stage;
        SensorDataSample();
        assert(!EFI_GetSample(EFI_COOLANT).valid);
        assert(EFI_GetSample(EFI_MAP).valid && EFI_GetSample(EFI_MAP).raw==1300);
        assert(EFI_GetSample(EFI_AF).valid && EFI_GetSample(EFI_KNOCK).valid);
    }
    assert(!EFI_GetSample((EfiSensor)-1).valid);
    EFI_SetThrottleClosed(true,true); assert(EFI_GetIdle() && EFI_IdleValid());
    fail_stage=0; SensorDataSample(); assert(EFI_GetIdle());
    EFI_SetThrottleClosed(true,false); assert(!EFI_GetIdle() && !EFI_IdleValid());
    EFI_SetThrottleClosed(false,true); assert(!EFI_GetIdle() && EFI_IdleValid());
    EFI_SetThrottleClosed(false,false); assert(!EFI_GetIdle() && !EFI_IdleValid());
    puts("sensors: all checks passed");
}
