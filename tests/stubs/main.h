#ifndef TEST_MAIN_H
#define TEST_MAIN_H
#include <stdint.h>
typedef struct { unsigned id; } ADC_HandleTypeDef;
typedef struct { uint32_t Channel, Rank, SamplingTime; } ADC_ChannelConfTypeDef;
typedef enum { HAL_OK, HAL_ERROR } HAL_StatusTypeDef;
#define ADC_CHANNEL_13 13U
#define ADC_CHANNEL_14 14U
#define ADC_CHANNEL_6 6U
#define ADC_CHANNEL_7 7U
#define ADC_SAMPLETIME_480CYCLES 480U
HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef *, ADC_ChannelConfTypeDef *);
HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *);
HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef *, uint32_t);
uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *);
HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef *);
uint32_t HAL_GetTick(void);
#endif
