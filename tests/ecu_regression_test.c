#include "ecu_model.h"
#include "injection_plan.h"
#include <float.h>
#include <math.h>
#include <stdio.h>

static unsigned failures;
#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while (0)

int main(void)
{
    /* 初回位相未確定の500rpm以上→低回転でも切替点から12周期待つ。 */
    InjectionControl control={0};
    CHECK(injection_control_ne(&control,600,true,false)==INJ_MODE_CHANGED);
    for (unsigned n=0;n<11;++n)
        CHECK(injection_control_ne(&control,600,true,false)==INJ_NO_EVENT);
    CHECK(injection_control_ne(&control,499,true,false)==INJ_MODE_CHANGED);
    CHECK(control.periods==0);
    for (unsigned n=0;n<11;++n)
        CHECK(injection_control_ne(&control,499,true,false)==INJ_NO_EVENT);
    CHECK(injection_control_ne(&control,499,true,false)==INJ_BOTH_DUE);
    for (unsigned n=0;n<11;++n)
        CHECK(injection_control_ne(&control,499,true,false)==INJ_NO_EVENT);
    CHECK(injection_control_ne(&control,499,true,false)==INJ_BOTH_DUE);

    /* 未決定の異常から、次の正常入力だけで通常要求へ自動復帰しない。 */
    CHECK(injection_control_ne(&control,499,false,false)==INJ_NEEDS_POLICY);
    CHECK(injection_control_ne(&control,499,true,false)==INJ_NEEDS_POLICY);
    CHECK(injection_control_ne(&control,600,true,true)==INJ_NEEDS_POLICY);
    injection_stop(&control,NULL);
    CHECK(injection_control_ne(&control,600,true,true)==INJ_MODE_CHANGED);
    CHECK(injection_control_ne(&control,600,true,false)==INJ_NEEDS_POLICY);
    CHECK(injection_control_ne(&control,600,true,true)==INJ_NEEDS_POLICY);

    /* 有限入力でもfloat中間演算がoverflowし、燃料0で成功していた条件。 */
    float axis[]={1}, ve[]={100}, lambda[]={1e20f}, dead[]={0};
    EcuFuelConfig config={{axis,axis,ve,1,1},{axis,axis,lambda,1,1},
                          {axis,dead,1},1e20f,1,1};
    EcuFuelInput input={600,80,12,1e30f,true};
    EcuFuelResult result;
    double expected=(double)input.air_mass_at_100_ve/((double)lambda[0]*config.stoich_afr);
    CHECK(ecu_fuel(&config,input,false,&result));
    CHECK(fabs(((double)result.mass_720-expected)/expected)<1e-5);
    CHECK(result.pulse_ms>0);
    result=(EcuFuelResult){1,1,1};
    CHECK(!ecu_fuel(NULL,input,false,&result));
    CHECK(result.mass_720==0 && result.event_mass==0 && result.pulse_ms==0);
    /* 最終値がfloatで表現できない場合も成功として返さない。 */
    lambda[0]=1e-20f; config.stoich_afr=1e-20f;
    input.air_mass_at_100_ve=FLT_MAX;
    CHECK(!ecu_fuel(&config,input,false,&result));
    CHECK(result.pulse_ms==0);
    lambda[0]=1e20f; config.stoich_afr=1e20f;
    input.air_mass_at_100_ve=FLT_MIN;
    CHECK(!ecu_fuel(&config,input,false,&result));
    CHECK(result.mass_720==0);

    /* float丸めで720度ちょうどを返すケースも[0,720)を守る。 */
    EngineSync sync;
    CHECK(engine_sync_init(&sync,(SyncConfig){1,100,1000}));
    sync.state=SYNC_LOCKED; sync.have_ne=true;
    EcuPhase phase={0}; float angle;
    CHECK(ecu_phase_set(&phase,FLT_MIN,true));
    CHECK(ecu_phase_angle(&phase,&sync,&angle));
    CHECK(angle>=0 && angle<720);
    if (failures) return 1;
    puts("ecu_regression: all checks passed");
    return 0;
}
