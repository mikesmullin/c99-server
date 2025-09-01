#pragma once

#include "../unity.h"  // IWYU pragma: keep

// inspired by:
// - [2012 Quake1](https://github.com/id-Software/Quake)

// @class ByteBuffer (SZ)
// Function | Purpose
// --- | ---
// SZ_reset(buf) | Reset buffer to initial state and clear data
// SZ_alloc(arena, buf, sz) | Allocate buffer from arena with specified size
// SZ_wrap(buf, data, len, sz) | Wrap existing data in buffer
// SZ_defrag(buf) | Defragment buffer by moving data to start
// SZ_overflow_read(buf, len) | Check if read operation would overflow
// SZ_overflow_write(buf, len) | Check if write operation would overflow
// SZ_readable(buf, len) | Get readable bytes count
// SZ_writable(buf, len) | Get writable bytes count
// SZ_seek(buf, offset) | Advance read cursor by offset
// SZ_read(buf, dst, len) | Read data from buffer into destination
// SZ_read_unsafe(buf, len) | Read data without bounds checking
// SZ_write(buf, data, len) | Write data to buffer
// SZ_write_unsafe(buf, data, len) | Write data without bounds checking
// SZ_copy(src, dst) | Copy data from source to destination buffer
// SZ_copyNS(src, dst) | Copy data without advancing source cursor
// SZ_print(buf, prefix, len) | Print buffer contents in hex format
// SZ_equal(buf, needle, len) | Compare buffer contents with string

// ByteBuffer (SZ)
// manages the CRUD for the ByteBuffer data struct

// Reset buffer to initial state and clear data
void SZ_reset(ByteBuffer* buf) {
  buf->read = buf->data;
  buf->write = buf->data;
  memset(buf->data, 0, buf->end - buf->data);
}

// Allocate buffer from arena with specified size
void SZ_alloc(Arena* arena, ByteBuffer* buf, u32 sz) {
  buf->data = (u8*)Arena__push(arena, sz);
  buf->end = buf->data + sz;
  SZ_reset(buf);
}

// Wrap existing data in buffer
void SZ_wrap(ByteBuffer* buf, u8* data, u32 len, u32 sz) {
  buf->data = buf->read = data;
  buf->write = buf->read + len;
  buf->end = buf->read + sz;
}

// Defragment buffer by moving data to start
void SZ_defrag(ByteBuffer* buf) {
  u32 len = buf->write - buf->read;
  if (0 == len) {
    buf->read = buf->write = buf->data;
    return;
  }

  u32 gap = buf->read - buf->data;
  if (0 == gap)
    return;
  memcpy(buf->data, buf->read, len);
  buf->read = buf->data;
  buf->write = buf->read + len;
}

// Check if read operation would overflow
bool SZ_overflow_read(ByteBuffer* buf, u32 len) {
  return buf->read + len > buf->write;
}

// Check if write operation would overflow
bool SZ_overflow_write(ByteBuffer* buf, u32 len) {
  return buf->write + len > buf->end;
}

// Get readable bytes count
u32 SZ_readable(ByteBuffer* buf, u32 len) {
  return buf->write - buf->read + len;
}

// Get writable bytes count
u32 SZ_writable(ByteBuffer* buf, u32 len) {
  return buf->end - buf->write + len;
}

// Advance read cursor by offset
s8 SZ_seek(ByteBuffer* buf, u32 offset) {
  if (SZ_overflow_read(buf, offset))
    return 0;
  buf->read += offset;
  return 1;
}

// Read data from buffer into destination
s8 SZ_read(ByteBuffer* buf, u8** dst, u32 len) {
  if (SZ_overflow_read(buf, len))
    return 0;
  *dst = buf->read;
  buf->read += len;
  return 1;
}

// Read data without bounds checking
u8* SZ_read_unsafe(ByteBuffer* buf, u32 len) {
  u8* r = buf->read;
  buf->read += len;
  return r;
}

// Write data to buffer
s8 SZ_write(ByteBuffer* buf, u8* data, u32 len) {
  if (SZ_overflow_write(buf, len))
    return 0;
  memcpy(buf->write, data, len);
  buf->write += len;
  return 1;
}

// Write data without bounds checking
s8 SZ_write_unsafe(ByteBuffer* buf, u8* data, u32 len) {
  memcpy(buf->write, data, len);
  buf->write += len;
  return 1;
}

// Copy data from source to destination buffer
s8 SZ_copy(ByteBuffer* src, ByteBuffer* dst) {
  u32 len = SZ_readable(src, 0);
  if (len < 1)
    return 0;  // nothing to copy
  if (SZ_writable(dst, len) >= 0) {
    SZ_write(dst, src->read, len);
    SZ_seek(src, len);
    return 1;  // copied
  }
  return 0;  // src could not fit in dst
}

// Copy data without advancing source cursor (NS = No Seek)
s8 SZ_copyNS(ByteBuffer* src, ByteBuffer* dst) {
  u32 len = SZ_readable(src, 0);
  if (len < 1)
    return 0;  // nothing to copy
  if (SZ_writable(dst, len) >= 0) {
    SZ_write(dst, src->read, len);
    return 1;  // copied
  }
  return 0;  // src could not fit in dst
}

// Print buffer contents in hex format
void SZ_print(ByteBuffer* buf, const char* prefix, u32 len) {
  if (0 == len)
    len = buf->write - buf->read;
  if (len < 1)
    return;
  char debug[4096] = "";
  hexdump(buf->read, len, debug, sizeof(debug));
  u64 read = buf->read - buf->data;
  u64 write = buf->write - buf->data;
  u64 end = buf->end - buf->data;
  LOG_DEBUGF(
      "%s. len: %lu, read: %llu, write: %llu, end: %llu, data:\n%s",
      prefix,
      len,
      read,
      write,
      end,
      debug);
}

// Compare buffer contents with string
bool SZ_equal(ByteBuffer* buf, const char* needle, const u32 len) {
  if (buf->end > buf->read &&  //
      0 == strncmp(
               (char*)buf->read,  //
               needle,
               len))
    return true;
  else
    return false;
}