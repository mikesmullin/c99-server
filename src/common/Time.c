#pragma once

#include "../unity.h"  // IWYU pragma: keep

// inspired by:
// - [Javascript RAF](https://developer.mozilla.org/en-US/docs/Web/API/Window/requestAnimationFrame)

// @class Time
// Function | Purpose
// --- | ---
// Time__sleep_ms(ms) | Sleep for specified milliseconds using nanosleep
// Time__unix_ts() | Get current Unix timestamp in seconds
// Time__perf_now() | Get high-resolution monotonic time in nanoseconds
// Time__now() | Get milliseconds elapsed since process start
// Time__us(ns) | Convert nanoseconds to microseconds
// Time__ms(ns) | Convert nanoseconds to milliseconds

// Sleep for specified milliseconds using nanosleep
void Time__sleep_ms(u32 ms) {
  struct timespec req;
  req.tv_sec = ms / 1000;
  req.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&req, NULL);
}

// Get current Unix timestamp in seconds
s64 Time__unix_ts(void) {
  time_t now = time(NULL);
  if (now == (time_t)-1) {
    return 0;  // Error case
  }
  return (s64)now;
}

// Get high-resolution monotonic time in nanoseconds
u64 Time__perf_now(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
    return 0;  // Error case
  }
  return (u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec;
}

// Get milliseconds elapsed since process start
u64 Time__now(void) {
  static u64 start_ns = 0;
  if (start_ns == 0) {
    start_ns = Time__perf_now();
  }
  u64 current_ns = Time__perf_now();
  return (current_ns - start_ns) / 1000000ULL;
}

// Convert nanoseconds to microseconds
u64 Time__us(u64 ns) {
  return ns / 1000ULL;
}

// Convert nanoseconds to milliseconds
u64 Time__ms(u64 ns) {
  return ns / 1000000ULL;
}
