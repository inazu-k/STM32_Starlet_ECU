#include "ecu_model.h"
#include "injection_plan.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#define NEAR(a,b) assert(fabs((double)(a)-(double)(b))<0.001f)

int main(void)
{
    float x[]={100,500,2000}, y[]={20,100};
    float values[]={10,20,50,30,40,70};
    EcuTable2 table={x,y,values,3,2};
    float out=0;
    assert(ecu_lookup2(table,300,60,&out)); NEAR(out,25);
    assert(ecu_lookup2(table,0,60,&out)); NEAR(out,20);
    assert(ecu_lookup2(table,300,200,&out)); NEAR(out,35);
    assert(ecu_lookup2(table,9000,0,&out)); NEAR(out,50);
    assert(ecu_lookup2(table,500,100,&out)); NEAR(out,40);
    float volts[]={8,12,16}, dead[]={2,1,0.5};
    EcuTable1 dt={volts,dead,3};
    assert(ecu_lookup1(dt,10,&out)); NEAR(out,1.5);
    assert(ecu_lookup1(dt,3,&out)); NEAR(out,2);
    assert(ecu_lookup1(dt,30,&out)); NEAR(out,0.5);
    assert(!ecu_lookup1(dt,NAN,&out));
    float bad_axis[]={1,1};
    assert(!ecu_lookup1((EcuTable1){bad_axis,dead,2},1,&out));
    assert(!ecu_lookup2((EcuTable2){x,y,values,0,2},1,1,&out));
    float invalid[]={NAN};
    assert(!ecu_lookup1((EcuTable1){x,invalid,1},100,&out));

    /* VEとλは独立した軸を使い、500 rpm以上では増量しない。 */
    float vx[]={100,1000}, vy[]={20,100}, ve[]={100,100,100,100};
    float lx[]={300,600,3000}, ly[]={10,200}, lambda[]={1,1,1,1,1,1};
    EcuFuelConfig config={{vx,vy,ve,2,2},{lx,ly,lambda,3,2},dt,10,2,2};
    EcuFuelInput input={500,80,12,100,true};
    EcuFuelResult fuel;
    assert(ecu_fuel(&config,input,true,&fuel));
    NEAR(fuel.mass_720,10); NEAR(fuel.event_mass,5); NEAR(fuel.pulse_ms,3.5);
    input.rpm=499;
    assert(ecu_fuel(&config,input,true,&fuel)); NEAR(fuel.pulse_ms,6);
    input.rpm=500;
    assert(ecu_fuel(&config,input,false,&fuel)); NEAR(fuel.pulse_ms,6);
    /* λ側の別軸で線形に変わる値を使い、VE軸の流用を検出する。 */
    float varying_lambda[]={1,4,7,1,4,7};
    config.lambda.values=varying_lambda;
    input.rpm=500;
    assert(ecu_fuel(&config,input,false,&fuel));
    NEAR(fuel.mass_720,10.0/3.0);
    config.lambda.values=lambda;
    input.valid=false;
    assert(!ecu_fuel(&config,input,false,&fuel)); NEAR(fuel.pulse_ms,0);
    input.valid=true; input.battery_v=NAN;
    assert(!ecu_fuel(&config,input,false,&fuel));

    EngineSync sync;
    assert(engine_sync_init(&sync,(SyncConfig){100,20000,50000}));
    sync.have_ne=true; sync.period_ticks=10000;
    assert(ecu_rpm(&sync,1000000,&out)); NEAR(out,500);
    EcuPhase phase={0};
    assert(!ecu_phase_angle(&phase,&sync,&out));
    assert(!ecu_phase_set(&phase,20,false));
    assert(!ecu_phase_set(&phase,720,true));
    assert(ecu_phase_set(&phase,20.5f,true));
    sync.state=SYNC_LOCKED; sync.tooth=0;
    assert(ecu_phase_angle(&phase,&sync,&out)); NEAR(out,699.5);
    sync.tooth=12;
    assert(ecu_phase_angle(&phase,&sync,&out)); NEAR(out,339.5);
    engine_sync_poll(&sync,50000);
    assert(!ecu_rpm(&sync,1000000,&out));
    assert(!ecu_phase_angle(&phase,&sync,&out));

    InjectionControl control={0}; InjectionPlan plan={0};
    assert(injection_control_ne(&control,499,true,false)==INJ_MODE_CHANGED);
    for (unsigned n=1;n<12;++n) assert(injection_control_ne(&control,499,true,false)==INJ_NO_EVENT);
    assert(injection_control_ne(&control,499,true,false)==INJ_BOTH_DUE);
    assert(injection_control_ne(&control,500,true,true)==INJ_MODE_CHANGED);
    assert(control.mode==INJ_ANGLED);
    assert(injection_control_ne(&control,499,true,true)==INJ_MODE_CHANGED);
    for (unsigned n=1;n<12;++n) assert(injection_control_ne(&control,499,true,true)==INJ_NO_EVENT);
    /* 待機満了と同時に同期・回転数条件を満たすと角度指定を優先する。 */
    assert(injection_control_ne(&control,500,true,true)==INJ_MODE_CHANGED);
    assert(injection_control_ne(&control,600,true,false)==INJ_NEEDS_POLICY);
    uint32_t delay;
    assert(injection_end_delay(100,190,0,1000,&delay) && delay==3000);
    assert(injection_end_delay(100,190,1,1000,&delay) && delay==15000);
    assert(injection_reserve(&plan,0,100,1000,200)==INJ_RESERVED);
    assert(injection_plan_advance(&plan,900));
    assert(plan.group[0].active && plan.group[0].active_end==1100);
    assert(injection_reserve(&plan,0,900,100,200)==INJ_SKIP_LATE);
    assert(injection_reserve(&plan,0,900,250,100)==INJ_SKIP_OVERLAP);
    assert(injection_reserve(&plan,0,900,400,100)==INJ_RESERVED);
    injection_cancel_pending(&plan);
    assert(plan.group[0].active && !plan.group[0].pending && plan.group[0].active_end==1100);
    assert(injection_plan_advance(&plan,1100) && !plan.group[0].active);
    /* 実行中と非重複の未来予約を共存させ、予定境界で次へ移る。 */
    assert(injection_reserve(&plan,0,1100,100,50)==INJ_RESERVED);
    assert(injection_plan_advance(&plan,1150));
    assert(injection_reserve(&plan,0,1150,150,100)==INJ_RESERVED);
    assert(injection_plan_advance(&plan,1200));
    assert(plan.group[0].active && plan.group[0].active_end==1300);
    assert(injection_plan_advance(&plan,1300));
    assert(!plan.group[0].active);
    /* 時刻周回、遅刻開始、停止と再始動で旧予約を破棄する。 */
    assert(injection_reserve(&plan,1,UINT32_MAX-50,100,20)==INJ_RESERVED);
    assert(injection_plan_advance(&plan,29) && plan.group[1].active);
    assert(injection_plan_advance(&plan,49) && !plan.group[1].active);
    assert(injection_reserve(&plan,0,100,100,20)==INJ_RESERVED);
    assert(!injection_plan_advance(&plan,181));
    assert(!plan.group[0].active && !plan.group[0].pending);
    assert(injection_reserve(&plan,0,200,100,20)==INJ_RESERVED);
    assert(injection_plan_advance(&plan,280));
    injection_stop(&control,&plan);
    assert(control.mode==INJ_STOPPED && !control.seen_phase && plan.generation==1);
    assert(!plan.group[0].active && !plan.group[0].pending);
    assert(injection_plan_advance(&plan,300));
    assert(injection_control_ne(&control,400,true,false)==INJ_MODE_CHANGED);
    assert(control.periods==0);
    puts("ecu_model/injection_plan: all checks passed");
}
