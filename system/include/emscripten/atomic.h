/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <inttypes.h>

#include <emscripten/em_types.h>

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

#define EMSCRIPTEN_WAIT_ASYNC_INFINITY __builtin_inf()

// Represents a pending 'Atomics.waitAsync' wait operation.
#define ATOMICS_WAIT_TOKEN_T int32_t

#define EMSCRIPTEN_IS_VALID_WAIT_TOKEN(token) ((token) <= 0)

// Issues the JavaScript 'Atomics.waitAsync' instruction:
// performs an asynchronous wait operation on the main thread. If the given
// 'addr' contains 'value', issues a deferred wait that will invoke the
// specified callback function 'asyncWaitFinished' once that address has been
// notified by another thread.
// NOTE: Unlike functions emscripten_atomic_wait_u32() and
// emscripten_atomic_wait_u64() which take in the wait timeout parameter as int64
// nanosecond units, this function takes in the wait timeout parameter as double
// millisecond units. See https://github.com/WebAssembly/threads/issues/175 for
// more information.
// Pass in maxWaitMilliseconds == EMSCRIPTEN_WAIT_ASYNC_INFINITY
// (==__builtin_inf()) to wait infinitely long.
// Returns one of:
//  - ATOMICS_WAIT_NOT_EQUAL if the waitAsync operation could not be registered
//    since the memory value did not contain the value 'value'.
//  - ATOMICS_WAIT_TIMED_OUT if the waitAsync operation timeout parameter was <= 0.
//  - Any other value: denotes a 'wait token' that can be passed to function
//    emscripten_atomic_cancel_wait_async() to unregister an asynchronous wait.
//    You can use the macro EMSCRIPTEN_IS_VALID_WAIT_TOKEN(retval) to check if
//    this function returned a valid wait token.
ATOMICS_WAIT_TOKEN_T emscripten_atomic_wait_async(void *addr __attribute__((nonnull)),
                                                  uint32_t value,
                                                  void (*asyncWaitFinished)(int32_t *addr, uint32_t value, ATOMICS_WAIT_RESULT_T waitResult, void *userData) __attribute__((nonnull)),
                                                  void *userData,
                                                  double maxWaitMilliseconds);

// Unregisters a pending Atomics.waitAsync operation that was established via a
// call to emscripten_atomic_wait_async() in the calling thread. Pass in the
// wait token handle that was received as the return value from the wait
// function.  Returns EMSCRIPTEN_RESULT_SUCCESS if the cancellation was
// successful, or EMSCRIPTEN_RESULT_INVALID_PARAM if the asynchronous wait has
// already resolved prior and the callback has already been called.
// NOTE: Because of needing to work around issue
// https://github.com/WebAssembly/threads/issues/176, calling this function has
// an effect of introducing spurious wakeups to any other threads waiting on the
// same address that the async wait denoted by the token does. This means that
// in order to safely use this function, the mechanisms used in any wait code on
// that address must be written to be spurious wakeup safe. (this is the case
// for all the synchronization primitives declared in this header, but if you
// are rolling out your own, you need to be aware of this). If
// https://github.com/tc39/proposal-cancellation/issues/29 is resolved, then the
// spurious wakeups can be avoided.
EMSCRIPTEN_RESULT emscripten_atomic_cancel_wait_async(ATOMICS_WAIT_TOKEN_T waitToken);

// Cancels all pending async waits in the calling thread. Because of
// https://github.com/WebAssembly/threads/issues/176, if you are using
// asynchronous waits in your application, and need to be able to let GC reclaim
// Wasm heap memory when deinitializing an application, you *must* call this
// function to help the GC unpin all necessary memory.  Otherwise, you can wrap
// the Wasm content in an iframe and unload the iframe to let GC occur.
// (navigating away from the page or closing that tab will also naturally
// reclaim the memory)
int emscripten_atomic_cancel_all_wait_asyncs(void);

// Cancels all pending async waits in the calling thread to the given memory
// address.  Returns the number of async waits canceled.
int emscripten_atomic_cancel_all_wait_asyncs_at_address(void *addr __attribute__((nonnull)));

#undef _EM_INLINE

#ifdef __cplusplus
}
#endif
