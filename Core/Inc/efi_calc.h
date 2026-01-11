#ifndef EFI_CALC_H
#define EFI_CALC_H

#ifdef __cplusplus
extern "C" {
#endif

void SensorDataSample(void);
void ReadGPIOPort(void);
void JdgIdle(void);
void CalcFuelInjection(void);
void CalcIgnitionTiming(void);

#ifdef __cplusplus
}
#endif

#endif /* EFI_CALC_H */
