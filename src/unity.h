// Single Compilation Unit 1 (Unity Build: main.exe) -------------
#pragma once

// ---
// Global Dependencies

#define _XOPEN_SOURCE 500  // enable POSIX features in standard headers
// #define _CRT_SECURE_NO_WARNINGS  // ignore warnings about fopen()
#include <signal.h>  // IWYU pragma: keep // signal()
#include <stdarg.h>  // IWYU pragma: keep // va_list
#include <stdio.h>  // IWYU pragma: keep // sprintf()
#include <stdlib.h>  // IWYU pragma: keep  // malloc(), exit()

// ---
// Types

#include <stddef.h>  // NULL, offsetof()
// #define NULL (0)

// fixed-width types
#include <stdint.h>
// aliases
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

// Booleans
#include <stdbool.h>  // typedef bool, #define true (1), false (0)

// cstr
#include <ctype.h>  // IWYU pragma: keep // tolower()
#include <string.h>  // IWYU pragma: keep // memset

// Arrays
typedef struct {
  u32 ct;
  void* ptr;
} ARange;
#define ARRAYSIZE(A) (sizeof(A) / sizeof((A)[0]))
#define ARANGE(A) ((ARange){.ct = ARRAYSIZE(A), .ptr = A})
typedef struct {
  u32 ct;
  u32 stride;
  void* ptr;
} ARange2;
#define ARANGE2(A) ((ARange2){.ct = ARRAYSIZE(A), sizeof((A)[0]), .ptr = A})

// ---
// Structs

// allow intercept from unit test
bool __asserted = false;
bool __expect_assert = false;

static const char* S_CONSOLE_LOG1 = "*** TRACE %s:%u %s\n";
static const char* S_CONSOLE_LOG3 = "Assertion failed: %s\n  at %s:%u\n";
#define DEBUG_TRACE Console__log(S_CONSOLE_LOG1, __FILE__, __LINE__, __func__)
// TODO: define -DDEBUG_FAST (`-O0`) and -DDEBUG_SLOW from build system, defaulting to `-O3` for prod builds
#ifdef DEBUG_SLOW
#define LOG_DEBUGF(s, ...) Console__log(s "\n", ##__VA_ARGS__)
#define LOG_ERRORF(s, ...) Console__err(s "\n", ##__VA_ARGS__)
#define ASSERT(cond)                                           \
  if (!(cond)) {                                               \
    DEBUGGER;                                                  \
    Console__abort(S_CONSOLE_LOG3, #cond, __FILE__, __LINE__); \
  }
#define ASSERT_CONTEXT(cond, ctx, ...)                            \
  if (!(cond)) {                                                  \
    DEBUGGER;                                                     \
    Console__abort(                                               \
        "Assertion failed: %s\n  at %s:%u\n  Context: " ctx "\n", \
        #cond,                                                    \
        __FILE__,                                                 \
        __LINE__,                                                 \
        ##__VA_ARGS__);                                           \
  }
#else
#define LOG_DEBUGF(s, ...)
#define ASSERT(cond)
#define ASSERT_CONTEXT(cond, ctx, ...)
#endif

#include "common/Log.c"  // IWYU pragma: keep

// Breakpoints

#include "common/Breakpoint.c"  // IWYU pragma: keep

// Time

#include <time.h>  // IWYU pragma: keep

#include "common/Time.c"  // IWYU pragma: keep

// Arena

typedef struct {
  u8* buf;
  u8* pos;
  u8* end;
} Arena;

#include "common/Arena.c"  // IWYU pragma: keep

// Threads

#ifdef _WIN32
//#define THREAD_FN_RET DWORD WINAPI unsigned long __stdcall
#define THREAD_FN_RET unsigned long __stdcall
#define THREAD_FN_RET_VAL (0)
// #define THREAD_FN_PARAM1 LPVOID
#define THREAD_FN_PARAM1 void*

typedef struct {
  /*HANDLE*/ void* _win;
} Mutex;

typedef struct {
  /*HANDLE*/ void* _win;
} Thread;

typedef THREAD_FN_RET (*thread_fn_t)(THREAD_FN_PARAM1);
#else
#include <pthread.h>  // POSIX (Linux, macOS)

#define THREAD_FN_RET void*
#define THREAD_FN_RET_VAL (NULL)
#define THREAD_FN_PARAM1 void*

typedef struct {
  pthread_mutex_t _nix;
} Mutex;

typedef struct {
  pthread_t _nix;
} Thread;

typedef THREAD_FN_RET (*thread_fn_t)(THREAD_FN_PARAM1);
#endif

#define MAX_THREADS (64)

#include "common/Thread.c"  // IWYU pragma: keep

// Profiler

// comment next line when not in use
// #define PROFILER__INSTRUMENTED
#ifdef PROFILER__INSTRUMENTED
#define PROFILE__BEGIN(id) Profiler__beginTrace(id)
#define PROFILE__END(id) Profiler__endTrace(id)
#define PROFILE__PRINT() Profiler__printf()
#else
#define PROFILE__BEGIN(id)
#define PROFILE__END(id)
#define PROFILE__PRINT()
#endif

typedef enum {
  PROFILED_FN_NONE,

  PROFILED_FN_COUNT,
} ProfiledFns;

typedef struct Trace {
  u64 last_time;
  u64 total_time;
  u32 call_count;
} Trace;

#define MAX_PROFILE_FNS (99)

typedef struct Profiler {
  Trace traces[MAX_PROFILE_FNS];
} Profiler;

Profiler profiler;

#include "common/Profiler.c"  // IWYU pragma: keep

// List

#define GENERIC_LIST(N, T)   \
  typedef struct N##__Node { \
    struct N##__Node* next;  \
    T data;                  \
  } N##__Node;               \
  typedef struct {           \
    u32 len;                 \
    N##__Node* head;         \
    N##__Node* tail;         \
  } N

GENERIC_LIST(List, void*);

typedef s8 (*List__sorter_t)(const void* a, const void* b);

typedef struct {
  List* list;
  List__Node* node;
  u32 i;
} ListIt;  // List Iterator

#include "common/List.c"  // IWYU pragma: keep

// Buffers

typedef struct {
  // |                     |
  // 0      |      |       n
  u8 *data, *read, *write, *end;
} ByteBuffer;

// #include "common/ByteBuffer.c"  // IWYU pragma: keep

typedef struct {
  u16 head; /* Next write position (aka end) */
  u16 tail; /* Oldest item (aka start) */
} RingBuf;  // Ring Buffer

#include "common/Ring.c"  // IWYU pragma: keep

// Math

// min, max, clamp
#define Math__min(a, b) (((a) < (b)) ? (a) : (b))
#define Math__max(a, b) (((a) > (b)) ? (a) : (b))
#define Math__clampi(min, n, max) (((n) < (min)) ? (min) : ((max) < (n)) ? (max) : (n))
#define Math__clampb(min, n, max) \
  if (n < min) {                  \
    n = min;                      \
  } else if (max < n) {           \
    n = max;                      \
  }
#define Math__between(min, n, max) (((min) < (n)) && ((n) < (max)))

// Strings (Views)

typedef enum {
  STR_STATIC = 0,  // immutable const char*
  STR_STACK = 1,  // mutable char*
  STR_MALLOC = 2,  // malloc() heap
  STR_ARENA1 = 3,  // Arena__Push(_G.arena) heap
  STR_ARENA2 = 4,  // Arena__Push(_G.frameArena) heap
} Str8Lifetime;

typedef struct {
  char* str;
  u16 len;  // 65535 max

  bool slice;  // termination: 0 = null, 1 = slice
  // IMPORTANT: mutability of str BUFFER, not the Str8 wrapper struct
  bool mut;  // mutability: 0 = immutable, 1 = mutable
  Str8Lifetime life;  // lifetime
} Str8;

// #include "common/String.c"  // IWYU pragma: keep

// JSON

typedef enum {
  JSON_INVALID,
  JSON_EOF,

  // Punctuation
  JSON_OCURLY,
  JSON_CCURLY,
  JSON_OBRACKET,
  JSON_CBRACKET,
  JSON_COMMA,
  JSON_COLON,

  // Literals
  JSON_TRUE,
  JSON_FALSE,
  JSON_NULL,

  // Types
  JSON_STRING,
  JSON_NUMBER,
  JSON_BOOL,

  JSON_COUNT,
} JsonTok;  // JSON Token

typedef struct {
  JsonTok token;
  const char* symbol;
} JsonSym;  // JSON Symbol

// iterator
typedef struct {
  Str8 data;  // parser boundary
  u32 cur;  // read cursor

  // current token
  JsonTok token;  // kind
  union {
    f64 number;  // number value
    Str8 str;  // string value (view/slice)
  } token_value;

  // debug; mainly referenced when errors log
  const char* file_path;
  u32 token_start;
} Json;

// #include "common/Json.c"  // IWYU pragma: keep

// File

#include "common/File.c"  // IWYU pragma: keep

// Socket

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include <ws2tcpip.h>
#endif

#ifdef __linux__
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

typedef enum {
  SERVER_SOCKET,
  CLIENT_SOCKET,
} SocketOpts;

typedef enum {
  SOCKET_NONE,
  SOCKET_ACCEPTING,
  SOCKET_CONNECTED,
  SOCKET_CLOSED,
} SocketState;

typedef enum {
  SESSION_NONE,
  SESSION_SERVER_HANDSHAKE_AWAIT,
  SESSION_SERVER_HANDSHAKE_RESPONDED,
  SESSION_CLIENT_HANDSHAKE_REQUESTED,
  SESSION_SERVER_CONNECTED,
  SESSION_CLIENT_HANDSHAKE_RECEIVED,
  SESSION_CLIENT_HELLO_SENT,
  SESSION_CLIENT_WASM_CONNECT_CB,
  SESSION_CLIENT_CONNECTED,
  SESSION_SERVER_HUNGUP,
} SessionState;

typedef struct {
  char addr[256], port[6];
  u32 opts;
#ifdef __linux__
  u64 _nix_socket;
  struct sockaddr_in _nix_addr;
#endif
#ifdef _WIN32
  u64 _win_socket;
  struct addrinfo* _win_addr;
#endif
#ifdef __EMSCRIPTEN__
  u32 _web_socket;
#endif
  ByteBuffer message, readBuf, writeBuf;  // datagram
  SocketState state;
  SessionState sessionState;
  u64 connectedAt;
  u16 ping;
  f32 ts;  // last client seconds since connect (for RTT)
  u8 rate;  // KB/sec
  u8 cl_updaterate;  // snapshot/sec
  u8 cl_interp;  // lag compensation (ms)
  u64 lastPacket, lastSnapshot;
  void* userdata;
} Socket;

typedef void (*Socket__alloc_t)(Socket** sock);
typedef void (*Socket__accept_t)(Socket* listener, Socket* accepted);
typedef void (*Socket__connect_t)(Socket* client);
typedef void (*Socket__recv_t)(Socket* sock, u8* buf, u32 len);
typedef void (*Socket__send_t)(Socket* sock, u8* buf, u32 len);

// #include "common/Sock.c"  // IWYU pragma: keep

// Engine

typedef struct Engine__State {
  Arena* arena;  // long-term allocations
  Arena* frameArena;  // temporary allocations

  // Net
  Socket__alloc_t onsockalloc;
  Socket__accept_t onsockaccept;
  Socket__connect_t onsockconnect;
  Socket__recv_t onsockrecv;
  Socket__send_t onsocksend;

  // Add engine-specific state variables here

} Engine__State;

Engine__State* _G = &(Engine__State){0};  // stack allocation in main thread

// ---
// Includes (order matters)

// clang-format off
#include "common/String.c"  // IWYU pragma: keep
#include "common/ByteBuffer.c"  // IWYU pragma: keep
#include "common/Sock.c"  // IWYU pragma: keep
#include "common/Json.c"  // IWYU pragma: keep
// clang-format on