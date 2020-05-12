/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Helper functions for JavaScript: JS calls these to do memory operations, and
// we get ASan coverage that way (as we can't see direct typed array view access
// in JS).
//
// Note that we check for alignment here, in order to not change behavior, as in
// JS an unaligned operation would fail.

#include <emscripten.h>
#include <stddef.h>
#include <stdint.h>

extern "C" {

EM_JS(void, abort_on_bad_alignment, (void* ptr, int size), {
  abort("unaligned pointer access on " + ptr + " of size " + size);
});

int32_t asan_js_load_1(int8_t* ptr) {
  return *ptr;
}
uint32_t asan_js_load_1u(uint8_t* ptr) {
  return *ptr;
}
int32_t asan_js_load_2(int16_t* ptr) {
  if (size_t(ptr) & 1) abort_on_bad_alignment(ptr, 2);
  return *ptr;
}
uint32_t asan_js_load_2u(uint16_t* ptr) {
  if (size_t(ptr) & 1) abort_on_bad_alignment(ptr, 2);
  return *ptr;
}
int32_t asan_js_load_4(int32_t* ptr) {
  if (size_t(ptr) & 3) abort_on_bad_alignment(ptr, 4);
  return *ptr;
}
uint32_t asan_js_load_4u(uint32_t* ptr) {
  if (size_t(ptr) & 3) abort_on_bad_alignment(ptr, 4);
  return *ptr;
}
float asan_js_load_f(float* ptr) {
  if (size_t(ptr) & 3) abort_on_bad_alignment(ptr, 4);
  return *ptr;
}
double asan_js_load_d(double* ptr) {
  if (size_t(ptr) & 7) abort_on_bad_alignment(ptr, 8);
  return *ptr;
}

// Note that the stores return the value, which is what JS does, as you can
// do
//     x = HEAP32[..] = val;
int32_t asan_js_store_1(int8_t* ptr, int8_t val) {
  return *ptr = val;
}
uint32_t asan_js_store_1u(uint8_t* ptr, uint8_t val) {
  return *ptr = val;
}
int32_t asan_js_store_2(int16_t* ptr, int16_t val) {
  if (size_t(ptr) & 1) abort_on_bad_alignment(ptr, 2);
  return *ptr = val;
}
uint32_t asan_js_store_2u(uint16_t* ptr, uint16_t val) {
  if (size_t(ptr) & 1) abort_on_bad_alignment(ptr, 2);
  return *ptr = val;
}
int32_t asan_js_store_4(int32_t* ptr, int32_t val) {
  if (size_t(ptr) & 3) abort_on_bad_alignment(ptr, 4);
  return *ptr = val;
}
uint32_t asan_js_store_4u(uint32_t* ptr, uint32_t val) {
  if (size_t(ptr) & 3) abort_on_bad_alignment(ptr, 4);
  return *ptr = val;
}
float asan_js_store_f(float* ptr, float val) {
  if (size_t(ptr) & 3) abort_on_bad_alignment(ptr, 4);
  return *ptr = val;
}
double asan_js_store_d(double* ptr, double val) {
  if (size_t(ptr) & 7) abort_on_bad_alignment(ptr, 8);
  return *ptr = val;
}

}
