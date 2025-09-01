#include "unity.h"

static void _Main__onSignal(int sig) {
  printf("Caught signal %d, shutting down gracefully...\n", sig);
  exit(0);
}

int main(int argc, char* argv[]) {
  signal(SIGINT, _Main__onSignal);
  Console__init();

  _G->arena = Arena__allocZ(1024);
  ASSERT_CONTEXT(_G->arena, "Failed to allocate arena");
  _G->frameArena = Arena__allocZ(1024);
  ASSERT_CONTEXT(_G->frameArena, "Failed to allocate frame arena");

  printf("Starting application...\n");
  while (true) {
    // printf("  %ld\n", unixtime());
    printf("  ts: %ld pnow: %ld now: %ld\n", Time__unix_ts(), Time__perf_now(), Time__now());
    // fflush(stdout);
    Time__sleep_ms(1000);
  }
  return 0;
}