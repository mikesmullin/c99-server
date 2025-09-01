#define UNIT_TEST

#include "../../../src/unity.h"  // IWYU pragma: keep

// Worker thread function that simulates processing work items
THREAD_FN_RET _Thread__worker(THREAD_FN_PARAM1 userdata) {
  u32 threadId = (u32)(uintptr_t)userdata;
  LOG_DEBUGF("Worker thread %u started\n", threadId);

  // Simulate processing 5 work items
  for (u32 i = 0; i < 5; i++) {
    LOG_DEBUGF("Thread %u processing item %u\n", threadId, i);
    Time__sleep_ms(200);  // Simulate work
  }

  LOG_DEBUGF("Worker thread %u finished\n", threadId);
  return THREAD_FN_RET_VAL;
}

// @describe Thread
// @tag common
int main() {
  _G->arena = Arena__allocZ(10 * 1024);

  // Demonstrate threading with worker pool
  const u32 numThreads = 3;
  Thread threads[MAX_THREADS];
  bool threadCreated[MAX_THREADS] = {0};

  LOG_DEBUGF("Creating %u worker threads...\n", numThreads);

  // Create worker threads
  for (u32 i = 0; i < numThreads && i < MAX_THREADS; i++) {
    threadCreated[i] = Thread__create(&threads[i], _Thread__worker, (void*)(uintptr_t)i);
    ASSERT_CONTEXT(threadCreated[i], "Failed to create thread %u\n", i);
  }

  // Wait for all threads to complete
  LOG_DEBUGF("Waiting for threads to complete...\n");
  Thread__join(threads, numThreads);

  // Clean up thread resources
  Thread__destroy(threads, numThreads);
  LOG_DEBUGF("All worker threads completed\n");

  return 0;
}