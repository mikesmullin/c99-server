#pragma once

#include "../unity.h"  // IWYU pragma: keep

// @class Time
// Function | Purpose
// --- | ---
// Time__sec2ms(sec) | Convert seconds to milliseconds
// Time__ms2sec(ms) | Convert milliseconds to seconds
// Time__sec2ticks(sec) | Convert seconds to engine ticks
// Time__ms2ticks(ms) | Convert milliseconds to engine ticks
// Time__tick2ms(tick) | Convert ticks to milliseconds
// Time__tick2sec(tick) | Convert ticks to seconds
// Time__since(ms) | Milliseconds elapsed since timestamp

// @class Timer (T)
// Function | Purpose
// --- | ---
// T_paused(t) | Check if timer is paused
// T_cancel(t) | Cancel timer early
// T_canceled(t) | Check if timer is canceled
// T_began(t) | Check if timer started
// T_complete(t) | Complete timer immediately
// T_ended(t, duration) | Check if timer ended
// T_completed(t, duration) | Check if timer completed successfully
// T_rdy(t, duration) | Check if timer ready or ended
// T_busy(t, duration) | Check if timer still running
// T_ms(t) | Get elapsed milliseconds
// T_remain(t, duration) | Get remaining milliseconds
// T_sec(t) | Get elapsed seconds
// T_pct(t, duration) | Get elapsed progress percentage
// T_lerp(t, duration, a, b) | Map progress to range a..b
// T_play(t) | Start or restart timer
// T_pause(t) | Pause timer and store elapsed
// T_resume(t) | Resume timer from elapsed

// @class Cooldown (CD)
// Function | Purpose
// --- | ---
// CD_cancel(cd) | Cancel cooldown early
// CD_canceled(cd) | Check if cooldown canceled
// CD_complete(cd) | Complete cooldown immediately
// CD_ended(cd) | Check if cooldown ended
// CD_completed(cd) | Check if cooldown completed successfully
// CD_rdy(cd) | Check if cooldown ready or ended
// CD_busy(cd) | Check if cooldown still cooling
// CD_remain(cd) | Remaining milliseconds
// CD_remainS(cd) | Remaining seconds
// CD_ms(cd, duration) | Elapsed milliseconds
// CD_sec(cd, duration) | Elapsed seconds
// CD_pct(t, duration) | Elapsed progress percentage
// CD_lerp(t, duration, a, b) | Map progress to range a..b
// CD_play(t, duration) | Set cooldown to expire after duration
// CD_playS(t, duration) | Set cooldown by seconds duration
// CD_rdy_set(t, duration) | If ready, start and report true

// @class Ticker (TK)
// Function | Purpose
// --- | ---
// TK_play(tk, duration) | Set ticker to expire after ticks
// TK_cancel(tk) | Cancel ticker early
// TK_canceled(tk) | Check if ticker canceled
// TK_end(tk) | End ticker immediately
// TK_ended(tk) | Check if ticker ended
// TK_completed(tk) | Check if ticker completed successfully
// TK_rdy(tk) | Check if ticker ready or ended
// TK_busy(tk) | Check if ticker still running
// TK_remain(tk) | Remaining ticks
// TK_ticks(tk, duration) | Elapsed ticks
// TK_pct(tk, duration) | Elapsed progress percentage
// TK_lerp(tk, duration, a, b) | Map progress to range a..b
// TK_rdy_set(tk, duration) | If ready, start and report true

// ---
// Time

inline f32 Time__sec2ms(f32 sec) {
  return sec * 1000.0f;
}

inline f32 Time__ms2sec(u32 ms) {
  return ms / 1000.0f;
}

inline u32 Time__sec2ticks(f32 sec) {
  u32 ticks = Math__ceil(sec / _G->fixedTime);
  return ticks;
}

inline u32 Time__ms2ticks(u32 ms) {
  return Time__sec2ticks(Time__ms2sec(ms));
}

inline u32 Time__tick2ms(u32 tick) {
  return Time__sec2ms(tick * _G->fixedTime);
}

inline f32 Time__tick2sec(u32 tick) {
  return Time__ms2sec(Time__tick2ms(tick));
}

inline u32 Time__since(u32 ms) {
  return _G->now - ms;
}

// --
// @class Timer (T)
// based on past wall-clock time (individually pausable, not paused w/ engine)

// is timer paused?
inline bool T_paused(Timer t) {
  return TIMER_PAUSE_MASK == (t & TIMER_PAUSE_MASK);
}

// abort early (considered uncompleted)
inline void T_cancel(Timer* t) {
  *t = 0;
}

// canceled (or never started)?
inline bool T_canceled(const Timer t) {
  return 0 == t;
}

// began (not canceled)
inline bool T_began(const Timer t) {
  return !T_canceled(t);
}

// force to end immediately (considered completed)
inline void T_complete(Timer* t) {
  *t = 2;
}

// ended (canceled or completed, but not paused)?
inline bool T_ended(const Timer t, u32 duration) {
  return !T_paused(t) && t + duration < _G->now;
}

// completed successfully?
inline bool T_completed(const Timer t, u32 duration) {
  return T_ended(t, duration) && !T_canceled(t);
}

// a) never started, b) was cancelled, or c) completed successfully ?
inline bool T_rdy(const Timer t, u32 duration) {
  return T_canceled(t) || T_ended(t, duration);
}

// still ticking?
inline bool T_busy(const Timer t, u32 duration) {
  return !T_rdy(t, duration);
}

// elapsed in ms (permitted to exceed duration)
inline u32 T_ms(const Timer t) {
  if (T_paused(t)) {
    u32 ms = t ^ TIMER_PAUSE_MASK;
    return ms;
  }
  return T_canceled(t) ? 0 : _G->now - t;
}

// remaining in ms (clamped >= 0)
inline u32 T_remain(const Timer t, u32 duration) {
  f32 elapsed = T_ms(t);
  return T_canceled(t) || elapsed > duration ? 0 : duration - elapsed;
}

// elapsed in sec
inline f32 T_sec(const Timer t) {
  return Time__ms2sec(T_ms(t));
}

// elapsed as progress percentage
f32 T_pct(const Timer t, u32 duration) {
  // clang-format off
  if (0 == duration) return 0.0f;  // avoid div/0
  if (T_completed(t, duration)) return 1.0f;  // completed
  // clang-format on
  return T_ms(t) / (f32)duration;
}

// map progress to range a..b
inline f32 T_lerp(const Timer t, u32 duration, f32 a, f32 b) {
  return lerp(T_pct(t, duration), a, b);
}

// reset beginning (in ms)
inline void T_play(Timer* t) {
  *t = _G->now;
}

// mark paused, store elapsed time
inline void T_pause(Timer* t) {
  u32 ms = T_ms(*t);
  *t = (ms | TIMER_PAUSE_MASK);
}

// resume playback from last elapsed
inline void T_resume(Timer* t) {
  if (T_paused(*t)) {
    u32 elapsed = *t ^ TIMER_PAUSE_MASK;
    u32 adjusted = _G->now - elapsed;
    *t = adjusted;
  }
}

// --
// @class Cooldown (CD)
// based on future wall-clock time (not pausable)

// abort early (considered uncompleted)
inline void CD_cancel(Cooldown* cd) {
  *cd = 0;
}

// canceled (or never started)?
inline bool CD_canceled(const Cooldown cd) {
  return 0 == cd;
}

// force to end immediately (considered completed)
inline void CD_complete(Cooldown* cd) {
  *cd = _G->now - 1;
}

// ended (canceled or completed)?
inline bool CD_ended(const Cooldown cd) {
  return cd < _G->now;
}

// completed successfully?
inline bool CD_completed(const Cooldown cd) {
  return CD_ended(cd) && !CD_canceled(cd);
}

// a) never started, b) was cancelled, or c) completed successfully ?
inline bool CD_rdy(const Cooldown cd) {
  return CD_canceled(cd) || CD_ended(cd);
}

// still ticking?
inline bool CD_busy(const Cooldown cd) {
  return !CD_rdy(cd);
}

// remaining in ms
inline u32 CD_remain(const Cooldown cd) {
  return CD_ended(cd) ? 0 : cd - _G->now;
}

// remaining in sec
inline f32 CD_remainS(const Cooldown cd) {
  return Time__ms2sec(CD_remain(cd));
}

// elapsed in ms
inline u32 CD_ms(const Cooldown cd, u32 duration) {
  u32 r = CD_remain(cd);
  return CD_canceled(cd) || r > duration ? 0 : duration - r;
}

// elapsed in sec
inline f32 CD_sec(const Cooldown cd, u32 duration) {
  return Time__ms2sec(CD_ms(cd, duration));
}

// elapsed as progress percentage
f32 CD_pct(const Cooldown t, u32 duration) {
  // clang-format off
  if (0 == duration) return 0.0f;  // avoid div/0
  if (CD_completed(t)) return 1.0f;  // completed
  // clang-format on
  return (CD_ms(t, duration) / (f32)duration);
}

// map progress to range a..b
inline f32 CD_lerp(const Cooldown t, u32 duration, f32 a, f32 b) {
  return lerp(CD_pct(t, duration), a, b);
}

// reset beginning (in ms)
inline void CD_play(Cooldown* t, s32 duration) {
  *t = _G->now + duration;
}

// reset beginning (in sec)
inline void CD_playS(Cooldown* t, f32 duration) {
  CD_play(t, Time__sec2ms(duration));
}

// if ready, set again. (ie. Cooldown timer)
// @return isReady - false while waiting, true on the reset frame.
inline bool CD_rdy_set(Cooldown* t, u32 duration) {
  if (CD_rdy(*t)) {
    CD_play(t, duration);  // in ms
    return true;
  }
  return false;
}

// ---
// @class Ticker (TK) timers
// these are based on ticks (or in-game time; not wall-clock time)

// [re]set ts when ticker should expire
inline void TK_play(Ticker* tk, u32 duration) {
  u32 ticks = duration;
  *tk = _G->ptick + ticks;
}

// abort ticker early (considered uncompleted)
inline void TK_cancel(Ticker* tk) {
  *tk = 0;
}

// canceled (or never started)?
inline bool TK_canceled(const Ticker tk) {
  return 0 == tk;
}

// force ticker to end immediately (considered completed)
inline void TK_end(Ticker* tk) {
  *tk = _G->ptick - 1;
}

// ended (canceled, or completed)?
inline bool TK_ended(const Ticker tk) {
  return tk < _G->ptick;
}

// completed successfully?
inline bool TK_completed(const Ticker tk) {
  return TK_ended(tk) && !TK_canceled(tk);
}

// ticker a) never started, b) was cancelled, or c) completed successfully ?
inline bool TK_rdy(const Ticker tk) {
  return TK_canceled(tk) || TK_ended(tk);
}

// still cooling down?
inline bool TK_busy(const Ticker tk) {
  return !TK_rdy(tk);
}

// time remaining? (0 = expired)
inline u32 TK_remain(const Ticker tk) {
  return TK_ended(tk) ? 0 : tk - _G->ptick;
}

// elapsed in ticks
inline u32 TK_ticks(const Ticker tk, u32 duration) {
  u32 r = TK_remain(tk);
  return TK_canceled(tk) || r > duration ? 0 : duration - r;
}

// elapsed (as percentage)? (1.0 = expired)
inline f32 TK_pct(const Ticker tk, u32 duration) {
  // clang-format off
  if (0 == duration) return 0.0f;  // avoid div/0
  if (TK_canceled(tk)) return 0.0f;  // aborted
  if (TK_completed(tk)) return 1.0f;  // completed
  // clang-format on
  u32 remain = TK_remain(tk);
  return 1.0f - (remain / (f32)duration);
}

// map progress to range a..b
inline f32 TK_lerp(const Ticker tk, u32 duration, f32 a, f32 b) {
  f32 pct = TK_pct(tk, duration);
  f32 value = lerp(pct, a, b);
  return value;
}

// if ready, set again. (in ms)
// @return isReady - false while waiting, true on the reset frame.
inline bool TK_rdy_set(Ticker* tk, u32 duration) {
  if (TK_rdy(*tk)) {
    TK_play(tk, duration);
    return true;
  }
  return false;
}