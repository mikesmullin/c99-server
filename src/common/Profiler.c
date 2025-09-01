#pragma once

#include "../unity.h"  // IWYU pragma: keep

// NOTE: This is quick, but [Tracy](https://github.com/wolfpld/tracy) is superior.

void Profiler__reset(void) {
  memset(profiler.traces, 0, sizeof(Trace) * MAX_PROFILE_FNS);
}

void Profiler__beginTrace(int id) {
  u64 unow = Time__perf_now();
  profiler.traces[id].last_time = -unow;
  profiler.traces[id].call_count++;
}

void Profiler__endTrace(int id) {
  u64 unow = Time__perf_now();
  profiler.traces[id].last_time += unow;
  profiler.traces[id].total_time += profiler.traces[id].last_time;
}

void Profiler__printf() {
  LOG_DEBUGF("\nProfiler:");
  for (u32 id = 0; id < MAX_PROFILE_FNS; id++) {
    if (0 == profiler.traces[id].call_count)
      continue;
    LOG_DEBUGF(
        "  fn %2u took %5llu ticks %5.1lf us %5.1lf ms avg (%5d calls)",
        id,
        profiler.traces[id].total_time / profiler.traces[id].call_count,
        Time__us(profiler.traces[id].total_time) / profiler.traces[id].call_count,
        Time__ms(profiler.traces[id].total_time) / profiler.traces[id].call_count,
        profiler.traces[id].call_count);
  }
}