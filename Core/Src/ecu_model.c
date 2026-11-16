#include "ecu_model.h"
#include <math.h>

/* 軸の不正、重複、非有限値を補間前に検出する。 */
static bool axis_valid(const float *axis, size_t n)
{
    if (!axis || n == 0) return false;
    for (size_t i = 0; i < n; ++i)
        if (!isfinite(axis[i]) || (i && axis[i] <= axis[i-1])) return false;
    return true;
}

/* 範囲外は端点とし、不等間隔軸の区間比を求める。 */
static size_t bracket(const float *axis, size_t n, float value, float *ratio)
{
    *ratio = 0;
    if (n == 1 || value <= axis[0]) return 0;
    if (value >= axis[n-1]) return n-1;
    size_t i = 0;
    while (value > axis[i+1]) ++i;
    *ratio = (float)(((double)value-axis[i])/((double)axis[i+1]-axis[i]));
    return i;
}

bool ecu_lookup1(EcuTable1 t, float x, float *out)
{
    if (!out || !isfinite(x) || !t.values || !axis_valid(t.axis, t.count)) return false;
    for (size_t j=0; j<t.count; ++j) if (!isfinite(t.values[j])) return false;
    float f;
    size_t i = bracket(t.axis, t.count, x, &f);
    float value = t.values[i];
    if (f != 0) value = value*(1-f) + t.values[i+1]*f;
    if (!isfinite(value)) return false;
    *out = value;
    return true;
}

bool ecu_lookup2(EcuTable2 t, float x, float y, float *out)
{
    if (!out || !isfinite(x) || !isfinite(y) || !t.values ||
        !axis_valid(t.x,t.nx) || !axis_valid(t.y,t.ny) || t.ny > SIZE_MAX/t.nx) return false;
    for (size_t k=0;k<t.nx*t.ny;++k) if (!isfinite(t.values[k])) return false;
    float fx, fy;
    size_t i=bracket(t.x,t.nx,x,&fx), j=bracket(t.y,t.ny,y,&fy);
    size_t ii=fx == 0 ? i : i+1, jj=fy == 0 ? j : j+1;
    float a=t.values[j*t.nx+i]*(1-fx)+t.values[j*t.nx+ii]*fx;
    float b=t.values[jj*t.nx+i]*(1-fx)+t.values[jj*t.nx+ii]*fx;
    float value=a*(1-fy)+b*fy;
    if (!isfinite(value)) return false;
    *out=value;
    return true;
}

bool ecu_phase_set(EcuPhase *p, float offset, bool stopped)
{
    if (!p || !stopped || !isfinite(offset) || offset < 0 || offset >= 720) return false;
    *p=(EcuPhase){offset,true};
    return true;
}

bool ecu_phase_angle(const EcuPhase *p, const EngineSync *s, float *degrees)
{
    if (!p || !s || !degrees || !p->calibrated || !isfinite(p->offset_deg) ||
        p->offset_deg<0 || p->offset_deg>=720 || !s->configured ||
        s->state!=SYNC_LOCKED || s->tooth>=24) return false;
    float angle=30.0f*s->tooth-p->offset_deg;
    if (angle<0) angle+=720;
    *degrees=angle;
    return true;
}

bool ecu_rpm(const EngineSync *s, uint32_t hz, float *rpm)
{
    if (!s || !rpm || !hz || !s->configured || !s->have_ne || !s->period_ticks) return false;
    *rpm=5.0f*(float)hz/(float)s->period_ticks;
    return isfinite(*rpm) && *rpm>0;
}

bool ecu_fuel(const EcuFuelConfig *c, EcuFuelInput in, bool simultaneous, EcuFuelResult *out)
{
    if (!c || !out) return false;
    *out=(EcuFuelResult){0};
    if (!in.valid || !isfinite(in.rpm) || in.rpm<=0 || !isfinite(in.map_kpa) || in.map_kpa<=0 ||
        !isfinite(in.battery_v) || in.battery_v<=0 || !isfinite(in.air_mass_at_100_ve) ||
        in.air_mass_at_100_ve<=0 || !isfinite(c->stoich_afr) || c->stoich_afr<=0 ||
        !isfinite(c->injector_mass_per_ms) || c->injector_mass_per_ms<=0 ||
        !isfinite(c->crank_factor) || c->crank_factor<=0) return false;
    float ve, lambda, dead;
    if (!ecu_lookup2(c->ve,in.rpm,in.map_kpa,&ve) || ve<0 ||
        !ecu_lookup2(c->lambda,in.rpm,in.map_kpa,&lambda) || lambda<=0 ||
        !ecu_lookup1(c->dead_ms,in.battery_v,&dead) || dead<0) return false;
    float mass=in.air_mass_at_100_ve*(ve/100.0f)/(lambda*c->stoich_afr);
    if (in.rpm<500) mass*=c->crank_factor;
    float event=simultaneous ? mass/2 : mass;
    float pulse=event/c->injector_mass_per_ms+dead;
    if (!isfinite(mass) || !isfinite(event) || !isfinite(pulse) || pulse<0) return false;
    *out=(EcuFuelResult){mass,event,pulse};
    return true;
}
