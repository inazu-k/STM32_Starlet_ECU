#include "injection_plan.h"
#include <math.h>
#include <stddef.h>
#define HALF UINT32_C(0x80000000)

InjectionAction injection_control_ne(InjectionControl *c, float rpm, bool valid, bool locked)
{
    if (!c || !valid || !isfinite(rpm) || rpm<=0 || (c->seen_phase && !locked))
        return INJ_NEEDS_POLICY;
    if (locked) c->seen_phase=true;
    InjectionMode mode=rpm>=500 && locked ? INJ_ANGLED : INJ_SIMULTANEOUS;
    if (mode!=c->mode) {
        c->mode=mode;
        c->periods=0;
        return INJ_MODE_CHANGED;
    }
    if (mode==INJ_SIMULTANEOUS && ++c->periods==12) {
        c->periods=0;
        return INJ_BOTH_DUE;
    }
    return INJ_NO_EVENT;
}

void injection_stop(InjectionControl *c, InjectionPlan *p)
{
    if (c) *c=(InjectionControl){0};
    if (p) {
        uint32_t generation=p->generation+1;
        *p=(InjectionPlan){0};
        p->generation=generation;
    }
}

void injection_cancel_pending(InjectionPlan *p)
{
    if (!p) return;
    for (unsigned g=0;g<2;++g) p->group[g].pending=false;
}

InjectionReserve injection_reserve(InjectionPlan *p, unsigned g, uint32_t now,
                                   uint32_t end_delay, uint32_t width)
{
    if (!p || g>=2 || !width || width>=HALF || !end_delay || end_delay>=HALF)
        return INJ_BAD_REQUEST;
    InjectionPulse *pulse=&p->group[g];
    if (pulse->pending) return INJ_BAD_REQUEST;
    if (width>end_delay) return INJ_SKIP_LATE;
    uint32_t start_delay=end_delay-width;
    uint32_t remaining=pulse->active_end-now;
    if (pulse->active && remaining<HALF && start_delay<remaining) return INJ_SKIP_OVERLAP;
    pulse->start=now+start_delay;
    pulse->end=now+end_delay;
    pulse->pending=true;
    return INJ_RESERVED;
}

bool injection_end_delay(float current, float end_a, unsigned g, uint32_t period, uint32_t *delay)
{
    if (!delay || g>=2 || !period || !isfinite(current) || current<0 || current>=720 ||
        !isfinite(end_a) || end_a<0 || end_a>=720) return false;
    double delta=(double)end_a+360.0*g-current;
    if (delta<=0) delta+=720;
    if (delta>720) delta-=720;
    double ticks=delta*period/30.0;
    if (ticks<1 || ticks>=HALF) return false;
    *delay=(uint32_t)ticks;
    return true;
}

bool injection_plan_advance(InjectionPlan *p, uint32_t now)
{
    if (!p) return false;
    bool on_time=true;
    for (unsigned g=0;g<2;++g) {
        InjectionPulse *pulse=&p->group[g];
        if (pulse->active && (uint32_t)(now-pulse->active_end)<HALF) pulse->active=false;
        if (pulse->pending && (uint32_t)(now-pulse->start)<HALF) {
            pulse->pending=false;
            if (now!=pulse->start) { on_time=false; continue; }
            pulse->active=true;
            pulse->active_end=pulse->end;
        }
    }
    return on_time;
}
