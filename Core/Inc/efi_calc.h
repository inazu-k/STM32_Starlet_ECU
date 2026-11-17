#ifndef EFI_CALC_H
#define EFI_CALC_H
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* 候補端子の生値。校正済み物理量とは区別する。有効性は今回の取得成否。 */
typedef enum { EFI_MAP, EFI_COOLANT, EFI_AF, EFI_KNOCK, EFI_SENSOR_COUNT } EfiSensor;
typedef struct { uint16_t raw; bool valid; uint32_t sampled_ms; } EfiSample;
void SensorDataSample(void);
EfiSample EFI_GetSample(EfiSensor sensor);
/* TPSの端子・しきい値が未確定のため、全閉を確認した上位層だけが供給する。 */
void EFI_SetThrottleClosed(bool closed, bool valid);
bool EFI_GetIdle(void);
bool EFI_IdleValid(void);
#ifdef __cplusplus
}
#endif
#endif
