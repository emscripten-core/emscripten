/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Helper functions for JavaScript: JS calls these to do memory operations, and
// we get ASan coverage that way (as we can't see direct typed array view access
// in JS).

#include <stddef.h>
#include <stdint.h>

// Note that we receive *shifted* values here. That is,
//
//  HEAP32[x >> 2]
//
// is turned into
//
//  _asan_c_load_4(x >> 2);
//
// and therefore we must shift to get the actual pointer. Doing it this way
// lets us behave the same as JS would wrt shifts and alignment.

int32_t _asan_c_load_1(int8_t* ptr) {
  return *ptr;
}
uint32_t _asan_c_load_1u(uint8_t* ptr) {
  return *ptr;
}
int32_t _asan_c_load_2(uintptr_t shifted) {
  int16_t* ptr = (int16_t*)(shifted << 1);
  return *ptr;
}
uint32_t _asan_c_load_2u(uintptr_t shifted) {
  uint16_t* ptr = (uint16_t*)(shifted << 1);
  return *ptr;
}
int32_t _asan_c_load_4(uintptr_t shifted) {
  int32_t* ptr = (int32_t*)(shifted << 2);
  return *ptr;
}
uint32_t _asan_c_load_4u(uintptr_t shifted) {
  uint32_t* ptr = (uint32_t*)(shifted << 2);
  return *ptr;
}
float _asan_c_load_f(uintptr_t shifted) {
  float* ptr = (float*)(shifted << 2);
  return *ptr;
}
double _asan_c_load_d(uintptr_t shifted) {
  double* ptr = (double*)(shifted << 3);
  return *ptr;
}

// Note that the stores return the value, which is what JS does, as you can
// do
//     x = HEAP32[..] = val;
int32_t _asan_c_store_1(int8_t* ptr, int8_t val) {
  return *ptr = val;
}
uint32_t _asan_c_store_1u(uint8_t* ptr, uint8_t val) {
  return *ptr = val;
}
int32_t _asan_c_store_2(uintptr_t shifted, int16_t val) {
  int16_t* ptr = (int16_t*)(shifted << 1);
  return *ptr = val;
}
uint32_t _asan_c_store_2u(uintptr_t shifted, uint16_t val) {
  uint16_t* ptr = (uint16_t*)(shifted << 1);
  return *ptr = val;
}
int32_t _asan_c_store_4(uintptr_t shifted, int32_t val) {
  int32_t* ptr = (int32_t*)(shifted << 2);
  return *ptr = val;
}
uint32_t _asan_c_store_4u(uintptr_t shifted, uint32_t val) {
  uint32_t* ptr = (uint32_t*)(shifted << 2);
  return *ptr = val;
}
float _asan_c_store_f(uintptr_t shifted, float val) {
  float* ptr = (float*)(shifted << 2);
  return *ptr = val;
}
double _asan_c_store_d(uintptr_t shifted, double val) {
  double* ptr = (double*)(shifted << 3);
  return *ptr = val;
}
