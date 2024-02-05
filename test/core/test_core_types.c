/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef __EMSCRIPTEN__
#error __EMSCRIPTEN__ is not defined
#endif
#ifndef __wasm__
#error __wasm__ is not defined
#endif
#if !defined(__wasm32__) && !defined(__wasm64__)
#error __wasm32__/__wasm64__ is not defined
#endif
#ifdef __cplusplus
#ifndef _GNU_SOURCE
#error _GNU_SOURCE is not defined in C++
#endif
#endif
#ifndef __unix__
#error __unix__ is not defined
#endif
#ifndef __LITTLE_ENDIAN__
#error __LITTLE_ENDIAN__ is not defined
#endif

#ifdef __clang__
#if __has_feature(pnacl)
#error has feature pnacl
#endif
#endif

#ifdef __native_client__
#error __native_client__ is defined
#endif
#ifdef __pnacl__
#error __pnacl__ is defined
#endif
#ifdef __ELF__
#error __ELF__ is defined
#endif
#ifdef __i386__
#error __i386__ is defined
#endif
#ifdef __i386
#error __i386 is defined
#endif
#ifdef i386
#error i386 is defined
#endif
#ifdef __SSE__
#error __SSE__ is defined
#endif
#ifdef __SSE2__
#error __SSE2__ is defined
#endif
#ifdef __SSE_MATH__
#error __SSE_MATH__ is defined
#endif
#ifdef __SSE2_MATH__
#error __SSE2_MATH__ is defined
#endif
#ifdef __MMX__
#error __MMX__ is defined
#endif
#ifdef __APPLE__
#error __APPLE__ is defined
#endif
#ifdef __linux__
#error __linux__ is defined
#endif
#ifdef __BIG_ENDIAN__
#error __BIG_ENDIAN__ is defined
#endif

// We prefer to use __EMSCRIPTEN__, but for compatibility, we define
// EMSCRIPTEN too.
#ifndef EMSCRIPTEN
#error EMSCRIPTEN is not defined
#endif

#include <stdalign.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#define STRINGIZE_HELPER(x) #x
#define STRINGIZE(x) STRINGIZE_HELPER(x)

int main() {
#if __wasm64__
  assert(__SIZEOF_POINTER__ == 8);
#else
  assert(__SIZEOF_POINTER__ == 4);
#endif
  assert(sizeof(void*) == __SIZEOF_POINTER__);
  assert(sizeof(long) == __SIZEOF_POINTER__);
  assert(sizeof(intptr_t) == __SIZEOF_POINTER__);
  assert(sizeof(size_t) == __SIZEOF_POINTER__);
  assert(sizeof(ptrdiff_t) == __SIZEOF_POINTER__);
  assert(sizeof(intmax_t) == 8);
  assert(alignof(double) == 8);
  assert(sizeof(long double) == 16);
  assert(alignof(long double) == 8);
  assert(alignof(max_align_t) == 8);
  // __BIGGEST_ALIGNMENT__ corresponds to the default stack alignment
  // used by llvm.  Unlike `alignof(max_align_t)` it includes SIMD types.
  // We use this in emscripten as our default stack alignment.
  assert(__BIGGEST_ALIGNMENT__ == 16);
  assert(__FLT_EVAL_METHOD__ == 0);
  assert(strcmp(STRINGIZE(__USER_LABEL_PREFIX__), "") == 0);
  return 0;
}
