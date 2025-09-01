#pragma once

#include "../unity.h"  // IWYU pragma: keep

// inspired by:
// - [2023 VoxelRifts - Making C Easy - Arenas](https://www.youtube.com/watch?v=3IAlJSIjvH0)
// - [2021 Mr. 4th Programming - Memory Management](https://www.youtube.com/watch?v=L79vSP8yV2g)
// - [2023 Ryan Fleury - Enter the Arena](https://www.youtube.com/watch?v=TZ5a3gCCZYo)

// @class Arena
// Function | Purpose
// --- | ---
// Arena__alloc(sz) | Allocate new arena with given size
// Arena__zero(arena) | Zero entire arena buffer (SLOW)
// Arena__allocZ(sz) | Allocate + zero-initialize arena
// Arena__zeroRange(p, sz) | Zero specific memory range (SLOW)
// Arena__cap(a) | Get arena capacity in bytes
// Arena__used(a) | Get used bytes in arena
// Arena__remain(a) | Get remaining bytes in arena
// Arena__ptr(arena, ptr) | Check if pointer is within arena bounds
// Arena__push(a, sz) | Allocate block from arena (primary function)
// Arena__free(a) | Free arena buffer
// Arena__reset(a) | Reset arena position to beginning

// malloc a new arena (used exactly once in each process/thread)
Arena* Arena__alloc(u64 sz) {
  Arena* arena = (Arena*)malloc(sizeof(Arena));
  if (!arena)
    return NULL;

  arena->buf = (u8*)malloc(sz);
  if (!arena->buf) {
    free(arena);
    return NULL;
  }

  arena->pos = arena->buf;
  arena->end = arena->buf + sz;
  return arena;
}

// zero a whole arena
// WARN: memset() is VERY SLOW!
void Arena__zero(Arena* arena) {
  ASSERT_CONTEXT(arena && arena->buf, "Arena is NULL or uninitialized");
  memset(arena->buf, 0, arena->end - arena->buf);
}

// alloc + zero-init
Arena* Arena__allocZ(u64 sz) {
  Arena* arena = Arena__alloc(sz);
  if (arena) {
    Arena__zero(arena);
  }
  return arena;
}

// zero an individual var (ie. for reuse)
// WARN: memset() is VERY SLOW!
void Arena__zeroRange(u8* p, u64 sz) {
  ASSERT_CONTEXT(p, "Pointer is NULL");
  memset(p, 0, sz);
}

// return capacity in bytes
static inline u32 Arena__cap(Arena* a) {
  ASSERT_CONTEXT(a && a->buf, "Arena is NULL or uninitialized");
  return (u32)(a->end - a->buf);
}

// return used in bytes
static inline u32 Arena__used(Arena* a) {
  ASSERT_CONTEXT(a && a->buf, "Arena is NULL or uninitialized");
  return (u32)(a->pos - a->buf);
}

// return remaining in bytes
static inline u32 Arena__remain(Arena* a) {
  ASSERT_CONTEXT(a && a->buf, "Arena is NULL or uninitialized");
  return (u32)(a->end - a->pos);
}

// is pointer inside arena space? (prevent segfault)
static inline bool Arena__ptr(Arena* arena, void* ptr) {
  ASSERT_CONTEXT(arena && arena->buf, "Arena is NULL or uninitialized");
  u8* p = (u8*)ptr;
  return p >= arena->buf && p < arena->end;
}

// reserve a block of memory from the area (most commonly used function)
void* Arena__push(Arena* a, u64 sz) {
  ASSERT_CONTEXT(a && a->buf, "Arena is NULL or uninitialized");
  ASSERT_CONTEXT(
      a->pos + sz <= a->end,
      "Arena exhausted. requested %llu bytes, available %llu bytes",
      sz,
      Arena__remain(a));

  void* result = a->pos;
  a->pos += sz;
  return result;
}

// free the allocated memory
void Arena__free(Arena* a) {
  if (a) {
    if (a->buf) {
      free(a->buf);
    }
    free(a);
  }
}

// reset arena write ptr to beginning of arena
void Arena__reset(Arena* a) {
  ASSERT_CONTEXT(a && a->buf, "Arena is NULL or uninitialized");
  a->pos = a->buf;
}
