#pragma once

#include "../unity.h"  // IWYU pragma: keep

// inspired by:
// - [Javascript debugger](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Statements/debugger)

// clang-format off
#define DEBUGGER if(Breakpoint__isDebugger()) __asm__("int3")  // GCC/Clang x86/x86_64
// clang-format on

bool Breakpoint__isDebugger(void) {
  FILE* fp = fopen("/proc/self/status", "r");
  if (fp == NULL) {
    return false;
  }
  char line[256];
  while (fgets(line, sizeof(line), fp)) {
    if (strncmp(line, "TracerPid:", 10) == 0) {
      int pid = atoi(line + 10);
      fclose(fp);
      return pid != 0;
    }
  }
  fclose(fp);
  return false;
}
