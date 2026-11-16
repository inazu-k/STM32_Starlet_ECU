#ifndef INJECTION_PLAN_H
#define INJECTION_PLAN_H
#include <stdbool.h>
#include <stdint.h>
/* 論理計画のみ。実ピン・タイマ割り込みへは接続しない。全APIは直列呼出し。 */
typedef enum { INJ_STOPPED, INJ_SIMULTANEOUS, INJ_ANGLED } InjectionMode;
typedef enum { INJ_NO_EVENT, INJ_BOTH_DUE, INJ_MODE_CHANGED, INJ_NEEDS_POLICY } InjectionAction;
typedef struct {
    InjectionMode mode;
    unsigned periods;
    bool seen_phase;
    bool cranking; /* 噴射方式とは独立した500rpm未満の運転状態。 */
    bool policy_required; /* 未定義の異常から通常要求へ自動復帰させない。 */
} InjectionControl;
typedef struct {
    uint32_t start, end, active_end;
    bool pending, active;
} InjectionPulse;
typedef struct { InjectionPulse group[2]; uint32_t generation; } InjectionPlan;
typedef enum { INJ_RESERVED, INJ_SKIP_LATE, INJ_SKIP_OVERLAP, INJ_BAD_REQUEST } InjectionReserve;
/* 有効回転を確認したNEをcount=0とし、以後の有効NEごとに1回呼ぶ。
 * 停止以外の入力無効・一度成立した位相の喪失は仕様追加確認が必要と返す。
 * この状態はラッチされ、停止リセットまで通常要求を再開しない。
 * 500rpm以上から未満への遷移は同じ同時噴射方式でも待機を初期化する。 */
InjectionAction injection_control_ne(InjectionControl *control, float rpm,
                                     bool valid, bool phase_locked);
/* 停止で全予約・実行中状態とカウンタを消去。校正設定は別管理。 */
void injection_stop(InjectionControl *control, InjectionPlan *plan);
/* 通常方式切替時は未開始だけ取消し、実行中の終了時刻を保持する。 */
void injection_cancel_pending(InjectionPlan *plan);
/* 未来の終了角までの時間から開始を逆算。過去開始・旧パルス重複は見送る。
 * ticks_to_end/width は同じタイマ単位、予約範囲は2^31 tick未満。 */
InjectionReserve injection_reserve(InjectionPlan *plan, unsigned group,
                                  uint32_t now, uint32_t ticks_to_end, uint32_t width);
/* 30度のNE周期から次の終了角までのtick数を求める（歯間一定速の論理予測）。 */
bool injection_end_delay(float current_deg, float end_a_deg, unsigned group,
                         uint32_t period_ticks, uint32_t *delay);
/* 予定時刻に呼ぶ仮想タイマ。開始／終了の遅刻はfalseを返す。
 * 遅刻開始は実行しない。遅刻終了はactiveを消去して遅刻を通知する。
 * 同じ呼出しの他の正常イベントは処理する。時刻は正順、処理空白は2^31未満。
 * 実機の遅刻処理は未確定。テスト／ベンチ計画専用。 */
bool injection_plan_advance(InjectionPlan *plan, uint32_t now);
#endif
