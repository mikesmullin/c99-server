#pragma once

#define MAX_THREADS (64)

#include "../unity.h"  // IWYU pragma: keep

// @class Mutex
// Function | Purpose
// --- | ---
// Thread__Mutex_create(m) | Create a new mutex
// Thread__Mutex_lock(m) | Acquire mutex lock
// Thread__Mutex_unlock(m) | Release mutex lock
// Thread__Mutex_destroy(m) | Destroy mutex

// @class Thread
// Function | Purpose
// --- | ---
// Thread__create(t, fn, userdata) | Create and start a new thread
// Thread__join(t[], len) | Wait for threads to complete
// Thread__destroy(t[], len) | Clean up thread resources

// Create a new mutex
bool Thread__Mutex_create(Mutex* m) {
#ifdef _WIN32
  m->_win = CreateMutex(NULL, FALSE, NULL);
  return m->_win != NULL;
#elif __linux__
  return 0 != pthread_mutex_init(&m->_nix, NULL);
#elif __EMSCRIPTEN__
  return false;
#endif
}

// Acquire mutex lock
void Thread__Mutex_lock(Mutex* m) {
#ifdef _WIN32
  WaitForSingleObject(m->_win, INFINITE);
#elif __linux__
  pthread_mutex_lock(&m->_nix);
#elif __EMSCRIPTEN__
  return;
#endif
}

// Release mutex lock
void Thread__Mutex_unlock(Mutex* m) {
#ifdef _WIN32
  ReleaseMutex(m->_win);
#elif __linux__
  pthread_mutex_unlock(&m->_nix);
#elif __EMSCRIPTEN__
  return;
#endif
}

// Destroy mutex
void Thread__Mutex_destroy(Mutex* m) {
#ifdef _WIN32
  CloseHandle(m->_win);
#elif __linux__
  pthread_mutex_destroy(&m->_nix);
#elif __EMSCRIPTEN__
  return;
#endif
}

// Create and start a new thread
bool Thread__create(Thread* t, thread_fn_t fn, void* userdata) {
#ifdef _WIN32
  t->_win = CreateThread(NULL, 0, fn, userdata, 0, NULL);
  return NULL != t->_win;
#elif __linux__
  return 0 == pthread_create(&t->_nix, NULL, fn, userdata);
#elif __EMSCRIPTEN__
  // TODO: implement WebWorkers as threads?
  return false;
#endif
}

// Wait for threads to complete
void Thread__join(Thread t[], u32 len) {
#ifdef _WIN32
  WaitForMultipleObjects(len, (const HANDLE*)t, TRUE, INFINITE);
#elif __linux__
  for (u32 i = 0; i < len && i < MAX_THREADS; i++) {
    pthread_join(t[i]._nix, NULL);
  }
#elif __EMSCRIPTEN__
  return;
#endif
}

// Clean up thread resources
void Thread__destroy(Thread t[], u32 len) {
#ifdef _WIN32
  for (u32 i = 0; i < len && i < MAX_THREADS; i++) {
    CloseHandle(t[i]._win);
  }
#elif __linux__
  return;
#elif __EMSCRIPTEN__
  return;
#endif
}