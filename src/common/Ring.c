#pragma once

#include "../unity.h"  // IWYU pragma: keep

// ---
// Ring Buffer

inline void Ring__clear(RingBuf* rb) {
  rb->head = rb->tail;
}

// true if there are exactly zero valid items
inline bool Ring__empty(RingBuf* rb) {
  return rb->head == rb->tail;
}

// true if every slot is occupied with a valid value
inline bool Ring__full(RingBuf* rb, u16 cap) {
  // NOTE: to disambiguate empty/full state, head is only allowed to decrement onto tail, not increment onto it
  return (rb->head + 1) % cap == rb->tail;
}

inline bool Ring__push(RingBuf* rb, u16 cap) {
  if (Ring__full(rb, cap)) {
    return false;  // can't write
  }
  rb->head = (rb->head + 1) % cap;
  return true;
}

inline bool Ring__pop(RingBuf* rb, u16 cap) {
  if (Ring__empty(rb)) {
    return false;  // can't read
  }
  rb->tail = (rb->tail + 1) % cap;
  return true;
}

// convert offset to index
inline u16 Ring__at(RingBuf* rb, u16 offset, u16 cap) {
  return (rb->tail + offset) % cap;
}