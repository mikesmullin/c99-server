
#pragma once

#include "../unity.h"  // IWYU pragma: keep

// inspired by:
// - [Mr. 4th impl](https://www.youtube.com/watch?v=2wio9UOFcow)
// - Ryan Fleury's impl for Epic > RAD Debugger
//   - https://github.com/EpicGamesExt/raddebugger/blob/94c4000603010aed97a45ca4d87a501a264f66b0/src/metagen/metagen_base/metagen_base_strings.h#L16
//   - https://github.com/EpicGamesExt/raddebugger/blob/94c4000603010aed97a45ca4d87a501a264f66b0/src/metagen/metagen_base/metagen_base_strings.c

// ---
// Str8

u64 Str8__cstrlen(const char* c) {
  u64 i = 0;
  for (; 0 != c[i]; i++);
  return i;
}

void Str8__init(Str8* s) {
  // deferred calc of strlen for cstr w/ len=0
  if (0 == s->len && !s->slice) {
    s->len = Str8__cstrlen(s->str);
  }
}

// classification

static inline bool Str8__isSpaceC(u8 c) {
  return ' ' == c || '\n' == c || '\t' == c || '\r' == c || '\f' == c || '\v' == c;
}

static inline bool Str8__isUpperC(u8 c) {
  return 'A' <= c && c <= 'Z';
}

static inline bool Str8__isLowerC(u8 c) {
  return 'a' <= c && c <= 'z';
}

static inline bool Str8__isAlphaC(u8 c) {
  return Str8__isUpperC(c) || Str8__isLowerC(c);
}

static inline bool Str8__isNumericC(u8 c) {
  return '0' <= c && c <= '9';
}

static inline bool Str8__isDigitC(u8 c) {
  return Str8__isNumericC(c) || '.' == c;
}

static inline bool Str8__isSlashC(u8 c) {
  return (c == '/' || c == '\\');
}

// format

static inline u8 Str8__toLowerC(u8 c) {
  return Str8__isUpperC(c) ? c + ('a' - 'A') : c;
}

static inline u8 Str8__toUpperC(u8 c) {
  return Str8__isLowerC(c) ? c + ('A' - 'a') : c;
}

// validation

static const char* S_STR8_ERR1 = "Expected Str8 to be mutable.";

#define STR8_ASSERT_MUTABLE(S) ASSERT_CONTEXT(S->mut, "%s", S_STR8_ERR1)

static void STR8_INHERIT_NULL(Str8* dst, Str8* s, u16 end) {
  if (!s->slice && end == s->len && 0 == s->str[s->len]) {
    dst->slice = false;
  } else {
    dst->slice = true;
  }
}

// path

// alloc on stack (mutable, empty)
Str8 Str8__stack() {
  // copy
  return (Str8){
      .len = 0,
      .life = STR_STACK,
      .mut = true,
      .slice = false,
      .str = NULL,
  };
}

// concatenate to arena (heap)
void Str8__cat(Str8* dst, u32 ct, ...) {
  dst->len = 0;
  dst->life = STR_ARENA1;
  dst->mut = true;
  dst->slice = false;
  dst->str = NULL;

  va_list args;
  va_start(args, ct);
  for (u32 i = 0; i < ct; i++) {
    Str8* s = va_arg(args, Str8*);
    Str8__init(s);
    dst->len += s->len;
  }
  va_end(args);

  dst->str = (char*)Arena__push(_G->arena, dst->len + 1);
  char* p = dst->str;

  va_start(args, ct);
  for (u32 i = 0; i < ct; i++) {
    Str8* s = va_arg(args, Str8*);
    memcpy(p, s->str, s->len);
    p += s->len;
  }
  va_end(args);

  *p = 0;  // null-terminate (for convenience)
}

// slices inherit the lifetime and mutability of an existing buf
void Str8__sliceOf(Str8* dst, Str8* src) {
  dst->slice = true;
  dst->mut = src->mut;  // inherit
  dst->life = src->life;  // inherit
}

// test equality, case-sensitive (0x01 == 0x01) (ie. binary comparison)
bool Str8__cmp(Str8* a, Str8* b) {
  if (a->len != b->len) {
    return false;
  }
  for (u16 i = 0; i < a->len; i++) {
    if (a->str[i] != b->str[i]) {
      return false;
    }
  }
  return true;
}

// test equality, case-insensitive ('A' == 'a')
bool Str8__icmp(Str8* a, Str8* b) {
  Str8__init(a);
  Str8__init(b);
  if (a->len != b->len) {
    return false;
  }
  for (u16 i = 0; i < a->len; i++) {
    if (Str8__toUpperC(a->str[i]) != Str8__toUpperC(b->str[i])) {
      return false;
    }
  }
  return true;
}

void Str8__basename(Str8* dst, Str8* s) {
  Str8__init(s);
  Str8__sliceOf(dst, s);
  u16 begin = 0, end = s->len;
  for (u16 i = s->len; i > 0; i--) {
    char c = s->str[i];
    if ('.' == c) {
      end = i;
    } else if ('/' == c || '\\' == c) {
      begin = i + 1;
      break;
    }
  }

  dst->str = (char*)s->str + begin;
  dst->len = end - begin;
  STR8_INHERIT_NULL(dst, s, end);
}

void Str8__fileext(Str8* dst, Str8* s) {
  Str8__init(s);
  Str8__sliceOf(dst, s);
  u16 begin = 0, end = s->len;
  for (u16 i = s->len; i > 0; i--) {
    char c = s->str[i];
    if ('.' == c) {
      begin = i + 1;
      break;
    } else if ('/' == c || '\\' == c) {
      begin = i + 1;
      break;
    }
  }

  dst->str = (char*)s->str + begin;
  dst->len = end - begin;
  STR8_INHERIT_NULL(dst, s, end);
}

void Str8__filename(Str8* dst, Str8* s) {
  Str8__init(s);
  Str8__sliceOf(dst, s);
  u16 begin = 0, end = s->len;
  for (u16 i = s->len; i > 0; i--) {
    char c = s->str[i];
    if ('/' == c || '\\' == c) {
      begin = i + 1;
      break;
    }
  }

  dst->str = (char*)s->str + begin;
  dst->len = end - begin;
  STR8_INHERIT_NULL(dst, s, end);
}

void Str8__dirname(Str8* dst, Str8* s) {
  Str8__init(s);
  Str8__sliceOf(dst, s);
  u16 end = s->len;
  for (u16 i = s->len; i > 0; i--) {
    char c = s->str[i];
    if ('/' == c || '\\' == c) {
      end = i + 1;
      break;
    }
  }

  dst->str = (char*)s->str;
  dst->len = end;
  STR8_INHERIT_NULL(dst, s, end);
}

char* Str8__cstr(Str8* s) {
  Str8__init(s);
  if (!s->slice && 0 == s->str[s->len]) {
    return s->str;
  }
  // copy
  char* r = Arena__push(_G->arena, s->len + 1);
  memcpy(r, s->str, s->len);
  r[s->len] = 0;  // null-terminate
  return r;
}

// TODO: Str8__split(&parts, file, &(Str8){"/"}, 0);

// Perform sprintf-style pattern matching against a given string.
// For each pattern matched, return a Str8 view.
// @returns u64 count of matched chars
//
// usage:
//   Str8 s1;
//   Str8__scan(line, "pattern %s", &s1);
u64 Str8__scan(Str8* input, const char* format, ...) {
  Str8__init(input);
  va_list args;
  va_start(args, format);
  const char* p_input = input->str;
  const char* p_format = format;
  const char* p_end = input->str + input->len;
  while (*p_format && *p_input && p_input < p_end) {
    if ('%' == *p_format) {
      p_format++;  // Move past initial '%'

      if ('d' == *p_format) {  // Handle integer
        Str8* p_int = va_arg(args, Str8*);
        p_int->life = input->life;
        p_int->mut = false;
        p_int->slice = true;

        // Skip whitespace
        while (isspace(*p_input) && p_input < p_end) p_input++;

        // Handle optional sign
        if ('-' == *p_input) {
          if (NULL == p_int->str) {
            p_int->str = (char*)p_input;  // str begins here
          }
          p_int->len++;
          p_input++;
        }

        // Parse integer
        if (isdigit(*p_input) && p_input < p_end) {
          while (isdigit(*p_input) && p_input < p_end) {
            if (NULL == p_int->str) {
              p_int->str = (char*)p_input;  // str begins here
            }
            p_int->len++;
            p_input++;
          }
          p_format++;  // Move past 'd'
        } else {
          break;  // Failed to match an integer
        }
      } else if ('f' == *p_format) {  // Handle float
        Str8* p_float = va_arg(args, Str8*);
        p_float->life = input->life;
        p_float->mut = false;
        p_float->slice = true;

        // Skip whitespace
        while (isspace(*p_input) && p_input < p_end) p_input++;

        // Handle optional sign
        if ('-' == *p_input && p_input < p_end) {
          if (NULL == p_float->str) {
            p_float->str = (char*)p_input;  // str begins here
          }
          p_float->len++;
          p_input++;
        }

        // Parse integer part of the float
        if (isdigit(*p_input) && p_input < p_end) {
          while (isdigit(*p_input) && p_input < p_end) {
            if (NULL == p_float->str) {
              p_float->str = (char*)p_input;  // str begins here
            }
            p_float->len++;
            p_input++;
          }

          // Parse fractional part
          if ('.' == *p_input && p_input < p_end) {
            p_float->len++;
            p_input++;  // Skip '.'
            while (isdigit(*p_input) && p_input < p_end) {
              p_float->len++;
              p_input++;
            }
          }
          p_format++;  // Move past 'f'
        } else {
          break;  // Failed to match a float
        }
      } else if (*p_format == 's') {  // Handle string
        Str8* p_str = va_arg(args, Str8*);
        p_str->life = input->life;
        p_str->mut = false;
        p_str->slice = true;

        // Skip whitespace
        while (isspace(*p_input) && p_input < p_end) p_input++;

        while (*p_input && !isspace(*p_input) && p_input < p_end) {
          if (NULL == p_str->str) {
            p_str->str = (char*)p_input;  // str begins here
          }
          p_str->len++;
          p_input++;
        }
        p_format++;  // Move past 's'
      } else {
        // Unsupported format specifier
        break;
      }
    } else if (isspace(*p_format)) {
      // Skip spaces in format
      while (isspace(*p_format)) p_format++;
      while (isspace(*p_input) && p_input < p_end) p_input++;
    } else {
      // Literal character match in the format
      if (*p_format == *p_input && p_input < p_end) {
        p_format++;
        p_input++;
      } else {
        break;  // Mismatch between input and format
      }
    }
  }
  va_end(args);
  return p_input - input->str;  // matched chars
}

// ---
// cstr utils

u32 cstr__len(const char* c) {
  u32 i = 0;
  for (; 0 != c[i]; i++);
  return i;
}

// copy cstr to heap (for long-term storage).
// ideal for references to static strings that get hot-reloaded
const char* cstr_arena1(const char* cstr) {
  u32 len = Str8__cstrlen(cstr);
  const char* r = Arena__push(_G->arena, len + 1);
  memcpy((char*)r, cstr, len);
  return r;
}

static inline f32 cstr__toF32(const char* s) {
  return (f32)strtof(s, NULL);
}

static inline u32 cstr__toU32(const char* s) {
  return (u32)strtoul(s, NULL, 10);
}

static inline u32 cstr__toHex(const char* s) {
  return (u32)strtoul(s, NULL, 16);
}

static inline bool cstr__eq(u32 len, const char* a, const char* b) {
  return 0 == strncmp(a, b, len);
}

char* cstr__vformat(Arena* arena, u32 maxlen, const char* fmt, va_list args) {
  char buf[maxlen];
  u32 l = vsnprintf(buf, maxlen, fmt, args) + 1;
  char* s = Arena__push(arena, l);
  memcpy(s, buf, l);
  s[l] = 0;  // null-terminate
  return s;
}

char* cstr__format(Arena* arena, u32 maxlen, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* s = cstr__vformat(arena, maxlen, fmt, args);
  va_end(args);
  return s;
}

// Format-as-String Utilities

// (debug) convert byte array to human-readable string
void hexdump(const u8* data, u32 len, char* out, u32 maxLen) {
  if (!data || !out || maxLen < 64) {
    return;  // Ensure enough space for one line
  }
  char* p = out;
  char* end = out + maxLen - 1;  // Reserve space for null terminator

  for (u32 i = 0; i < len && p < end; i += 16) {
    // Address
    p += snprintf(p, end - p, "%08x  ", i);

    // Hex bytes
    for (u32 j = 0; j < 16 && p < end; j++) {
      // clang-format off
      p += snprintf(p, end - p, i + j < len ? "%02x " : "   ", i + j < len ? data[i + j] : 0);
      // clang-format on
    }

    // ASCII representation
    p += snprintf(p, end - p, "|");
    for (u32 j = 0; j < 16 && p < end; j++) {
      // clang-format off
      p += snprintf(p, end - p, "%c", i + j < len ? (isprint(data[i + j]) ? data[i + j] : '.') : ' ');
      // clang-format on
    }

    p += snprintf(p, end - p, "|\n");
  }
  *p = '\0';
}

// convert u8 to written 8-bit binary notation
// ie. char c[11]; u82bin(c, alice.tags1);
void u82bin(char* c, u8 b) {
  c[0] = '0';
  c[1] = 'b';
  c[2] = (b & (1 << 7)) ? '1' : '0';
  c[3] = (b & (1 << 6)) ? '1' : '0';
  c[4] = (b & (1 << 5)) ? '1' : '0';
  c[5] = (b & (1 << 4)) ? '1' : '0';
  c[6] = (b & (1 << 3)) ? '1' : '0';
  c[7] = (b & (1 << 2)) ? '1' : '0';
  c[8] = (b & (1 << 1)) ? '1' : '0';
  c[9] = (b & (1 << 0)) ? '1' : '0';
  c[10] = 0;  // null-terminator
}

char* _format_bytes(char* buf, u64 bytes, bool round) {
  static const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
  u8 idx = 0;
  f64 value = (f64)bytes;

  // divide into largest possible unit
  while (value >= 1024 && idx < ARRAYSIZE(units)) {
    value /= 1024;
    idx++;
  }

  snprintf(buf, 16, round || !idx ? "%.0f%s" : "%.2f%s", value, units[idx]);
  return buf;
}

// human-readable bytes (e.g., 1024 -> "1 KB")
char* format_bytes(u64 bytes, bool round) {
  char* buf = Arena__push(_G->frameArena, 16);
  return _format_bytes(buf, bytes, round);
}