#pragma once

#include "../unity.h"

// inspired by:
// - [Javascript console.log](https://developer.mozilla.org/en-US/docs/Web/API/console/log_static)

// @class Console
// Function | Purpose
// --- | ---
// Console__init(void) | Disable output buffering for stdout and stderr
// Console__trapAssert(void) | Enable assertion trapping for unit tests
// Console__didAssert(void) | Check if an assertion was triggered during testing
// Console__resetAssert(void) | Reset assertion trap state to default
// Console__log(const char* txt, ...) | Print formatted message to stdout
// Console__error(const char* txt, ...) | Print formatted error message to stderr
// Console__abort(const char* txt, ...) | Print formatted message to stderr and abort program

// Disable output buffering for stdout and stderr
void Console__init(void) {
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
}

// Enable assertion trapping for unit tests
void Console__trapAssert(void) {
  __expect_assert = true;
}

// Check if an assertion was triggered during testing
bool Console__didAssert(void) {
  return __asserted;
}

// Reset assertion trap state to default
void Console__resetAssert(void) {
  __expect_assert = false;
  __asserted = false;
}

// Print formatted message to stdout
void Console__log(const char* txt, ...) {
  va_list myargs;
  va_start(myargs, txt);
  vfprintf(stdout, txt, myargs);
  va_end(myargs);
  fflush(stdout);
}

// Print formatted error message to stderr
void Console__error(const char* txt, ...) {
  va_list myargs;
  va_start(myargs, txt);
  vfprintf(stderr, txt, myargs);
  va_end(myargs);
  fflush(stderr);
}

// Print formatted message to stderr and abort program
void Console__abort(const char* txt, ...) {
  // allow intercept from unit test
  if (__expect_assert) {
    __asserted = true;
    return;
  }

  va_list myargs;
  va_start(myargs, txt);
  vfprintf(stderr, txt, myargs);
  va_end(myargs);
  fflush(stderr);
  abort();
}