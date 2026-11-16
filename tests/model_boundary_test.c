#include "ecu_model.h"
#include "injection_plan.h"
#include <stdio.h>

static unsigned failures;
#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while (0)

int main(void)
{
    /* 遅れて終了を反映したのにon_time=trueとなるケースを検出する。 */
    InjectionPlan plan={0};
    CHECK(injection_reserve(&plan,0,100,100,50)==INJ_RESERVED);
    CHECK(injection_plan_advance(&plan,150));
    CHECK(plan.group[0].active);
    CHECK(!injection_plan_advance(&plan,201));
    CHECK(!plan.group[0].active);
    CHECK(injection_plan_advance(&plan,202));
    /* 遅れた旧終了と次の正常開始が同じ呼出しでも、遅刻を隠さない。 */
    CHECK(injection_reserve(&plan,0,300,100,50)==INJ_RESERVED);
    CHECK(injection_plan_advance(&plan,350));
    CHECK(injection_reserve(&plan,0,350,150,50)==INJ_RESERVED);
    CHECK(!injection_plan_advance(&plan,450));
    CHECK(plan.group[0].active && plan.group[0].active_end==500);
    CHECK(injection_plan_advance(&plan,500));
    /* 独立した時系列で、時刻周回を跨いだ遅刻終了も検出する。 */
    plan=(InjectionPlan){0};
    CHECK(injection_reserve(&plan,1,UINT32_MAX-50,100,20)==INJ_RESERVED);
    CHECK(injection_plan_advance(&plan,29));
    CHECK(!injection_plan_advance(&plan,50));
    CHECK(!plan.group[1].active);

    /* 明らかに不正な寸法は、軸配列を読む前に拒否する。
     * 小配列で不正メタデータを渡し、ASanでも境界外読取りがないことを確認する。 */
    float axis[]={0}, values[]={1}, out;
    CHECK(!ecu_lookup1((EcuTable1){axis,values,SIZE_MAX},0,&out));
    CHECK(!ecu_lookup2((EcuTable2){axis,axis,values,SIZE_MAX,2},0,0,&out));
    CHECK(!ecu_lookup2((EcuTable2){axis,axis,values,2,SIZE_MAX/2},0,0,&out));
    CHECK(!ecu_lookup2((EcuTable2){axis,axis,values,0,2},0,0,&out));
    CHECK(ecu_lookup1((EcuTable1){axis,values,1},0,&out) && out==1);
    CHECK(ecu_lookup2((EcuTable2){axis,axis,values,1,1},0,0,&out) && out==1);
    if (failures) return 1;
    puts("model_boundary: all checks passed");
    return 0;
}
