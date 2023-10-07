/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _EM_INLINE static __inline__ __attribute__((always_inline, nodebug))

// Note on 64bit atomics ops: All 64-bit atomic ops defined here, while single
// instruction under wasm, will be emulated by using locks in wasm2js mode.
// This is also true for C/C++ native atomics as well as intrinsics.

// Atomically stores the given value to the memory location, and returns the
// value that was there prior to the store.
_EM_INLINE uint8_t emscripten_atomic_exchange_u8(void /*uint8_t*/* addr __attribute__((nonnull)), uint8_t newVal) {
  return __c11_atomic_exchange((_Atomic uint8_t*)addr, newVal, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint16_t emscripten_atomic_exchange_u16(void /*uint16_t*/* addr __attribute__((nonnull)), uint16_t newVal) {
  return __c11_atomic_exchange((_Atomic uint16_t*)addr, newVal, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint32_t emscripten_atomic_exchange_u32(void /*uint32_t*/* addr __attribute__((nonnull)), uint32_t newVal) {
  return __c11_atomic_exchange((_Atomic uint32_t*)addr, newVal, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint64_t emscripten_atomic_exchange_u64(void /*uint64_t*/* addr __attribute__((nonnull)), uint64_t newVal) {
  return __c11_atomic_exchange((_Atomic uint64_t*)addr, newVal, __ATOMIC_SEQ_CST);
}

// CAS returns the *old* value that was in the memory location before the
// operation took place.
// That is, if the return value when calling this function equals to 'oldVal',
// then the operation succeeded, otherwise it was ignored.
_EM_INLINE uint8_t emscripten_atomic_cas_u8(void /*uint8_t*/* addr __attribute__((nonnull)), uint8_t oldVal, uint8_t newVal) {
  uint8_t expected = oldVal;
  __c11_atomic_compare_exchange_strong((_Atomic uint8_t*)addr, &expected, newVal, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return expected;
}
_EM_INLINE uint16_t emscripten_atomic_cas_u16(void /*uint16_t*/* addr __attribute__((nonnull)), uint16_t oldVal, uint16_t newVal) {
  uint16_t expected = oldVal;
  __c11_atomic_compare_exchange_strong((_Atomic uint16_t*)addr, &expected, newVal, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return expected;
}
_EM_INLINE uint32_t emscripten_atomic_cas_u32(void /*uint32_t*/* addr __attribute__((nonnull)), uint32_t oldVal, uint32_t newVal) {
  uint32_t expected = oldVal;
  __c11_atomic_compare_exchange_strong((_Atomic uint32_t*)addr, &expected, newVal, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return expected;
}
_EM_INLINE uint64_t emscripten_atomic_cas_u64(void /*uint64_t*/* addr __attribute__((nonnull)), uint64_t oldVal, uint64_t newVal) {
  uint64_t expected = oldVal;
  __c11_atomic_compare_exchange_strong((_Atomic uint64_t*)addr, &expected, newVal, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return expected;
}

_EM_INLINE uint8_t emscripten_atomic_load_u8(const void /*uint8_t*/* addr __attribute__((nonnull))) {
  return __c11_atomic_load((_Atomic(uint8_t)*)addr, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint16_t emscripten_atomic_load_u16(const void /*uint16_t*/* addr __attribute__((nonnull))) {
  return __c11_atomic_load((_Atomic(uint16_t)*)addr, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint32_t emscripten_atomic_load_u32(const void /*uint32_t*/* addr __attribute__((nonnull))) {
  return __c11_atomic_load((_Atomic(uint32_t)*)addr, __ATOMIC_SEQ_CST);
}
_EM_INLINE float emscripten_atomic_load_f32(const void /*float*/* addr __attribute__((nonnull))) {
  return __c11_atomic_load((_Atomic(float)*)addr, __ATOMIC_SEQ_CST);
}
_EM_INLINE
uint64_t emscripten_atomic_load_u64(const void /*uint64_t*/* addr __attribute__((nonnull))) {
  return __c11_atomic_load((_Atomic(uint64_t)*)addr, __ATOMIC_SEQ_CST);
}
_EM_INLINE
double emscripten_atomic_load_f64(const void /*double*/* addr __attribute__((nonnull))) {
  return __c11_atomic_load((_Atomic(double)*)addr, __ATOMIC_SEQ_CST);
}

// Returns the value that was stored (i.e. 'val')
_EM_INLINE uint8_t emscripten_atomic_store_u8(void /*uint8_t*/* addr __attribute__((nonnull)), uint8_t val) {
  __c11_atomic_store((_Atomic(uint8_t)*)addr, val, __ATOMIC_SEQ_CST);
  return val;
}
_EM_INLINE uint16_t emscripten_atomic_store_u16(void /*uint16_t*/* addr __attribute__((nonnull)), uint16_t val) {
  __c11_atomic_store((_Atomic(uint16_t)*)addr, val, __ATOMIC_SEQ_CST);
  return val;
}
_EM_INLINE uint32_t emscripten_atomic_store_u32(void /*uint32_t*/* addr __attribute__((nonnull)), uint32_t val) {
  __c11_atomic_store((_Atomic(uint32_t)*)addr, val, __ATOMIC_SEQ_CST);
  return val;
}
_EM_INLINE float emscripten_atomic_store_f32(void /*float*/* addr __attribute__((nonnull)), float val) {
  __c11_atomic_store((_Atomic(float)*)addr, val, __ATOMIC_SEQ_CST);
  return val;
}
_EM_INLINE uint64_t emscripten_atomic_store_u64(void /*uint64_t*/* addr __attribute__((nonnull)), uint64_t val) {
  __c11_atomic_store((_Atomic(uint64_t)*)addr, val, __ATOMIC_SEQ_CST);
  return val;
}
_EM_INLINE double emscripten_atomic_store_f64(void /*double*/* addr __attribute__((nonnull)), double val) {
  __c11_atomic_store((_Atomic(double)*)addr, val, __ATOMIC_SEQ_CST);
  return val;
}

// Each of the functions below (add, sub, and, or, xor) return the value that
// was in the memory location before the operation occurred.
_EM_INLINE uint8_t emscripten_atomic_add_u8(void /*uint8_t*/* addr __attribute__((nonnull)), uint8_t val) {
  return __c11_atomic_fetch_add((_Atomic uint8_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint16_t emscripten_atomic_add_u16(void /*uint16_t*/* addr __attribute__((nonnull)), uint16_t val) {
  return __c11_atomic_fetch_add((_Atomic uint16_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint32_t emscripten_atomic_add_u32(void /*uint32_t*/* addr __attribute__((nonnull)), uint32_t val) {
  return __c11_atomic_fetch_add((_Atomic uint32_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint64_t emscripten_atomic_add_u64(void /*uint64_t*/* addr __attribute__((nonnull)), uint64_t val) {
  return __c11_atomic_fetch_add((_Atomic uint64_t*)addr, val, __ATOMIC_SEQ_CST);
}

_EM_INLINE uint8_t emscripten_atomic_sub_u8(void /*uint8_t*/* addr __attribute__((nonnull)), uint8_t val) {
  return __c11_atomic_fetch_sub((_Atomic uint8_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint16_t emscripten_atomic_sub_u16(void /*uint16_t*/* addr __attribute__((nonnull)), uint16_t val) {
  return __c11_atomic_fetch_sub((_Atomic uint16_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint32_t emscripten_atomic_sub_u32(void /*uint32_t*/* addr __attribute__((nonnull)), uint32_t val) {
  return __c11_atomic_fetch_sub((_Atomic uint32_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint64_t emscripten_atomic_sub_u64(void /*uint64_t*/* addr __attribute__((nonnull)), uint64_t val) {
  return __c11_atomic_fetch_sub((_Atomic uint64_t*)addr, val, __ATOMIC_SEQ_CST);
}

_EM_INLINE uint8_t emscripten_atomic_and_u8(void /*uint8_t*/* addr __attribute__((nonnull)), uint8_t val) {
  return __c11_atomic_fetch_and((_Atomic uint8_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint16_t emscripten_atomic_and_u16(void /*uint16_t*/* addr __attribute__((nonnull)), uint16_t val) {
  return __c11_atomic_fetch_and((_Atomic uint16_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint32_t emscripten_atomic_and_u32(void /*uint32_t*/* addr __attribute__((nonnull)), uint32_t val) {
  return __c11_atomic_fetch_and((_Atomic uint32_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint64_t emscripten_atomic_and_u64(void /*uint64_t*/* addr __attribute__((nonnull)), uint64_t val) {
  return __c11_atomic_fetch_and((_Atomic uint64_t*)addr, val, __ATOMIC_SEQ_CST);
}

_EM_INLINE uint8_t emscripten_atomic_or_u8(void /*uint8_t*/* addr __attribute__((nonnull)), uint8_t val) {
  return __c11_atomic_fetch_or((_Atomic uint8_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint16_t emscripten_atomic_or_u16(void /*uint16_t*/* addr __attribute__((nonnull)), uint16_t val) {
  return __c11_atomic_fetch_or((_Atomic uint16_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint32_t emscripten_atomic_or_u32(void /*uint32_t*/* addr __attribute__((nonnull)), uint32_t val) {
  return __c11_atomic_fetch_or((_Atomic uint32_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint64_t emscripten_atomic_or_u64(void /*uint64_t*/* addr __attribute__((nonnull)), uint64_t val) {
  return __c11_atomic_fetch_or((_Atomic uint64_t*)addr, val, __ATOMIC_SEQ_CST);
}

_EM_INLINE uint8_t emscripten_atomic_xor_u8(void /*uint8_t*/* addr __attribute__((nonnull)), uint8_t val) {
  return __c11_atomic_fetch_xor((_Atomic uint8_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint16_t emscripten_atomic_xor_u16(void /*uint16_t*/* addr __attribute__((nonnull)), uint16_t val) {
  return __c11_atomic_fetch_xor((_Atomic uint16_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint32_t emscripten_atomic_xor_u32(void /*uint32_t*/* addr __attribute__((nonnull)), uint32_t val) {
  return __c11_atomic_fetch_xor((_Atomic uint32_t*)addr, val, __ATOMIC_SEQ_CST);
}
_EM_INLINE uint64_t emscripten_atomic_xor_u64(void /*uint64_t*/* addr __attribute__((nonnull)), uint64_t val) {
  return __c11_atomic_fetch_xor((_Atomic uint64_t*)addr, val, __ATOMIC_SEQ_CST);
}

_EM_INLINE void emscripten_atomic_fence(void) {
  // Fake a fence with an arbitrary atomic operation
  uint8_t temp = 0;
  emscripten_atomic_or_u8(&temp, 0);
}

#define ATOMICS_WAIT_RESULT_T int

// Numbering dictated by https://github.com/WebAssembly/threads/blob/master/proposals/threads/Overview.md#wait
#define ATOMICS_WAIT_OK 0
#define ATOMICS_WAIT_NOT_EQUAL 1
#define ATOMICS_WAIT_TIMED_OUT 2

#define ATOMICS_WAIT_DURATION_INFINITE -1ll

// Issues the wasm 'memory.atomic.wait32' instruction:
// If the given memory address contains value 'expectedValue', puts the calling
// thread to sleep to wait for that address to be notified.
// Returns one of the ATOMICS_WAIT_* return codes.
// NOTE: This function takes in the wait value in int64_t nanosecond units. Pass
// in maxWaitNanoseconds = -1 (or ATOMICS_WAIT_DURATION_INFINITE) to wait
// infinitely long.
_EM_INLINE ATOMICS_WAIT_RESULT_T emscripten_atomic_wait_u32(void /*uint32_t*/*addr __attribute__((nonnull)), uint32_t expectedValue, int64_t maxWaitNanoseconds) {
  return __builtin_wasm_memory_atomic_wait32((int32_t*)addr, expectedValue, maxWaitNanoseconds);
}

// Issues the wasm 'memory.atomic.wait64' instruction:
// If the given memory address contains value 'expectedValue', puts the calling
// thread to sleep to wait for that address to be notified.
// Returns one of the ATOMICS_WAIT_* return codes.
// NOTE: This function takes in the wait value in int64_t nanosecond units. Pass
// in maxWaitNanoseconds = -1 (or ATOMICS_WAIT_DURATION_INFINITE) to wait
// infinitely long.
_EM_INLINE ATOMICS_WAIT_RESULT_T emscripten_atomic_wait_u64(void /*uint64_t*/*addr __attribute__((nonnull)), uint64_t expectedValue, int64_t maxWaitNanoseconds) {
  return __builtin_wasm_memory_atomic_wait64((int64_t*)addr, expectedValue, maxWaitNanoseconds);
}

#define EMSCRIPTEN_NOTIFY_ALL_WAITERS (-1LL)

// Issues the wasm 'memory.atomic.notify' instruction:
// Notifies the given number of threads waiting on a location.
// Pass count == EMSCRIPTEN_NOTIFY_ALL_WAITERS to notify all waiters on the
// given location.
// Returns the number of threads that were woken up.
// Note: this function is used to notify both waiters waiting on an u32 and u64
// addresses.
_EM_INLINE int64_t emscripten_atomic_notify(void *addr __attribute__((nonnull)), int64_t count) {
  return __builtin_wasm_memory_atomic_notify((int*)addr, count);
}

#undef _EM_INLINE

#ifdef __cplusplus
}
#endif
