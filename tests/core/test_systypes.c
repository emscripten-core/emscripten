/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <sys/types.h>

// Declare puts manually, we don't want to include any other headers here
#ifdef __cplusplus
extern "C"
#endif
int puts(const char*);

int main() {
  int8_t i8 = 0;
  u_int8_t ui8 = 0;
  int16_t i16 = 0;
  u_int16_t ui16 = 0;
  int32_t i32 = 0;
  u_int32_t ui32 = 0;
  int64_t i64 = 0;
  u_int64_t ui64 = 0;
  puts("success");
  return 0;
}
