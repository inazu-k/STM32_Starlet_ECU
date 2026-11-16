#ifndef ECU_MODEL_H
#define ECU_MODEL_H
#include "engine_sync.h"
#include <stddef.h>
/* 配列の所有権は呼出し元。値は y 行、x 列の順。単位は各利用側で明記する。 */
typedef struct { const float *axis, *values; size_t count; } EcuTable1;
typedef struct {
    const float *x, *y, *values;
    size_t nx, ny;
} EcuTable2;
/* 有限・厳密昇順の軸を検証し、参照座標だけをクランプして補間する。 */
bool ecu_lookup1(EcuTable1 table, float x, float *out);
bool ecu_lookup2(EcuTable2 table, float x, float y, float *out);
/* δは基準NE歯から#1圧縮TDCへの進行角。未校正を0度と見なさない。 */
typedef struct { float offset_deg; bool calibrated; } EcuPhase;
bool ecu_phase_set(EcuPhase *phase, float offset_deg, bool stopped);
bool ecu_phase_angle(const EcuPhase *phase, const EngineSync *sync, float *degrees);
bool ecu_rpm(const EngineSync *sync, uint32_t timer_hz, float *rpm);
/* 構造用の内部単位。空気質量の計算式・校正は未確定のため上位から供給する。
 * マップx=rpm、y=MAP絶対圧kPa、VE=%、λ=無次元、dead time=ms。
 * 質量は同一の任意単位、流量はその単位/ms。実車値の既定値は持たない。 */
typedef struct {
    EcuTable2 ve, lambda;
    EcuTable1 dead_ms;
    float stoich_afr, injector_mass_per_ms, crank_factor;
} EcuFuelConfig;
typedef struct {
    float rpm, map_kpa, battery_v;
    float air_mass_at_100_ve;
    bool valid;
} EcuFuelInput;
typedef struct { float mass_720, event_mass, pulse_ms; } EcuFuelResult;
/* simultaneous=true は360度ごとの2分配。水温・O2補正は行わない。 */
bool ecu_fuel(const EcuFuelConfig *config, EcuFuelInput input,
              bool simultaneous, EcuFuelResult *result);
#endif
