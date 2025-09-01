#pragma once

#include "../unity.h"  // IWYU pragma: keep

// @class File
// Function | Purpose
// --- | ---
// File__open(stream, filename, mode) | Open file stream with specified mode, return 0 on success or -1 on failure
// File__eof(stream) | Check if end-of-file reached
// File__read(buffer, buffer_sz, element_sz, element_ct, stream) | Read data from file into buffer
// File__write(buffer, element_sz, element_ct, stream) | Write data from buffer to file
// File__close(stream) | Close file stream, return error code

// Open file stream with specified mode, return 0 on success or -1 on failure
s8 File__open(FILE** stream, const char* filename, const char* mode) {
  *stream = fopen(filename, mode);
  return *stream ? 0 : -1;  // Check fopen result
}

// Check if end-of-file reached
s8 File__eof(FILE* stream) {
  return feof(stream);
}

// Read data from file into buffer
u64 File__read(void* buffer, size_t buffer_sz, size_t element_sz, size_t element_ct, FILE* stream) {
  return fread(buffer, element_sz, element_ct, stream);
}

// Write data from buffer to file
u64 File__write(const void* buffer, size_t element_sz, size_t element_ct, FILE* stream) {
  return fwrite(buffer, element_sz, element_ct, stream);
}

// Close file stream, return error code
s8 File__close(FILE* stream) {
  return fclose(stream);
}
