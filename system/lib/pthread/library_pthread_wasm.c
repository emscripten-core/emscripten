/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// This file contains support machinery to implement multithreading support for asm.js specifically.
// Split out to a separate file so that its contents can be easily left out when only doing Wasm
// multithreading.

#include <emscripten.h>
#include <emscripten/threading.h>
#include <pthread.h>

uint8_t emscripten_atomic_exchange_u8(void /*uint8_t*/* addr, uint8_t newVal) {
  return __c11_atomic_exchange((_Atomic uintptr_t*)addr, newVal, __ATOMIC_SEQ_CST);
}
uint16_t emscripten_atomic_exchange_u16(void /*uint16_t*/* addr, uint16_t newVal) {
  return __c11_atomic_exchange((_Atomic uintptr_t*)addr, newVal, __ATOMIC_SEQ_CST);
}
uint32_t emscripten_atomic_exchange_u32(void /*uint32_t*/* addr, uint32_t newVal) {
  return __c11_atomic_exchange((_Atomic uintptr_t*)addr, newVal, __ATOMIC_SEQ_CST);
}
uint64_t emscripten_atomic_exchange_u64(void /*uint64_t*/* addr, uint64_t newVal) {
  return __c11_atomic_exchange((_Atomic uintptr_t*)addr, newVal, __ATOMIC_SEQ_CST);
}

uint8_t emscripten_atomic_cas_u8(void /*uint8_t*/* addr, uint8_t oldVal, uint8_t newVal) {
  uint8_t expected = oldVal;
  __c11_atomic_compare_exchange_strong(
    (_Atomic uint8_t*)addr, &expected, newVal, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return expected;
}
uint16_t emscripten_atomic_cas_u16(void /*uint16_t*/* addr, uint16_t oldVal, uint16_t newVal) {
  uint16_t expected = oldVal;
  __c11_atomic_compare_exchange_strong(
    (_Atomic uint16_t*)addr, &expected, newVal, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return expected;
}
uint32_t emscripten_atomic_cas_u32(void /*uint32_t*/* addr, uint32_t oldVal, uint32_t newVal) {
  uint32_t expected = oldVal;
  __c11_atomic_compare_exchange_strong(
    (_Atomic uint32_t*)addr, &expected, newVal, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return expected;
}
uint64_t emscripten_atomic_cas_u64(void /*uint64_t*/* addr, uint64_t oldVal, uint64_t newVal) {
  uint64_t expected = oldVal;
  __c11_atomic_compare_exchange_strong(
    (_Atomic uint64_t*)addr, &expected, newVal, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return expected;
}

uint8_t emscripten_atomic_load_u8(const void /*uint8_t*/* addr) {
  return __c11_atomic_load((_Atomic(uint8_t)*)addr, __ATOMIC_SEQ_CST);
}
uint16_t emscripten_atomic_load_u16(const void /*uint16_t*/* addr) {
  return __c11_atomic_load((_Atomic(uint16_t)*)addr, __ATOMIC_SEQ_CST);
}
uint32_t emscripten_atomic_load_u32(const void /*uint32_t*/* addr) {
  return __c11_atomic_load((_Atomic(uint32_t)*)addr, __ATOMIC_SEQ_CST);
}
float emscripten_atomic_load_f32(const void /*float*/* addr) {
  return __c11_atomic_load((_Atomic(float)*)addr, __ATOMIC_SEQ_CST);
}
uint64_t emscripten_atomic_load_u64(const void /*uint64_t*/* addr) {
  return __c11_atomic_load((_Atomic(uint64_t)*)addr, __ATOMIC_SEQ_CST);
}
double emscripten_atomic_load_f64(const void /*double*/* addr) {
  return __c11_atomic_load((_Atomic(double)*)addr, __ATOMIC_SEQ_CST);
}

// Returns the value that was stored (i.e. 'val')
uint8_t emscripten_atomic_store_u8(void /*uint8_t*/* addr, uint8_t val) {
  __c11_atomic_store((_Atomic(uint8_t)*)addr, val, __ATOMIC_SEQ_CST);
  return val;
}
uint16_t emscripten_atomic_store_u16(void /*uint16_t*/* addr, uint16_t val) {
  __c11_atomic_store((_Atomic(uint16_t)*)addr, val, __ATOMIC_SEQ_CST);
  return val;
}
uint32_t emscripten_atomic_store_u32(void /*uint32_t*/* addr, uint32_t val) {
  __c11_atomic_store((_Atomic(uint32_t)*)addr, val, __ATOMIC_SEQ_CST);
  return val;
}
float emscripten_atomic_store_f32(void /*float*/* addr, float val) {
  __c11_atomic_store((_Atomic(float)*)addr, val, __ATOMIC_SEQ_CST);
  return val;
}
uint64_t emscripten_atomic_store_u64(void /*uint64_t*/* addr, uint64_t val) {
  __c11_atomic_store((_Atomic(uint64_t)*)addr, val, __ATOMIC_SEQ_CST);
  return val;
}
double emscripten_atomic_store_f64(void /*double*/* addr, double val) {
  __c11_atomic_store((_Atomic(double)*)addr, val, __ATOMIC_SEQ_CST);
  return val;
}

void emscripten_atomic_fence(void) {
  // Fake a fence with an arbitrary atomic operation
  uint8_t temp = 0;
  emscripten_atomic_or_u8(&temp, 0);
}

// Each of the functions below (add, sub, and, or, xor) return the value that was in the memory
// location before the operation occurred.
uint8_t emscripten_atomic_add_u8(void /*uint8_t*/* addr, uint8_t val) {
  return __c11_atomic_fetch_add((_Atomic uint8_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint16_t emscripten_atomic_add_u16(void /*uint16_t*/* addr, uint16_t val) {
  return __c11_atomic_fetch_add((_Atomic uint16_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint32_t emscripten_atomic_add_u32(void /*uint32_t*/* addr, uint32_t val) {
  return __c11_atomic_fetch_add((_Atomic uint32_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint64_t emscripten_atomic_add_u64(void /*uint64_t*/* addr, uint64_t val) {
  return __c11_atomic_fetch_add((_Atomic uint64_t*)addr, val, __ATOMIC_SEQ_CST);
}

uint8_t emscripten_atomic_sub_u8(void /*uint8_t*/* addr, uint8_t val) {
  return __c11_atomic_fetch_sub((_Atomic uint8_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint16_t emscripten_atomic_sub_u16(void /*uint16_t*/* addr, uint16_t val) {
  return __c11_atomic_fetch_sub((_Atomic uint16_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint32_t emscripten_atomic_sub_u32(void /*uint32_t*/* addr, uint32_t val) {
  return __c11_atomic_fetch_sub((_Atomic uint32_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint64_t emscripten_atomic_sub_u64(void /*uint64_t*/* addr, uint64_t val) {
  return __c11_atomic_fetch_sub((_Atomic uint64_t*)addr, val, __ATOMIC_SEQ_CST);
}

uint8_t emscripten_atomic_and_u8(void /*uint8_t*/* addr, uint8_t val) {
  return __c11_atomic_fetch_and((_Atomic uint8_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint16_t emscripten_atomic_and_u16(void /*uint16_t*/* addr, uint16_t val) {
  return __c11_atomic_fetch_and((_Atomic uint16_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint32_t emscripten_atomic_and_u32(void /*uint32_t*/* addr, uint32_t val) {
  return __c11_atomic_fetch_and((_Atomic uint32_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint64_t emscripten_atomic_and_u64(void /*uint64_t*/* addr, uint64_t val) {
  return __c11_atomic_fetch_and((_Atomic uint64_t*)addr, val, __ATOMIC_SEQ_CST);
}

uint8_t emscripten_atomic_or_u8(void /*uint8_t*/* addr, uint8_t val) {
  return __c11_atomic_fetch_or((_Atomic uint8_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint16_t emscripten_atomic_or_u16(void /*uint16_t*/* addr, uint16_t val) {
  return __c11_atomic_fetch_or((_Atomic uint16_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint32_t emscripten_atomic_or_u32(void /*uint32_t*/* addr, uint32_t val) {
  return __c11_atomic_fetch_or((_Atomic uint32_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint64_t emscripten_atomic_or_u64(void /*uint64_t*/* addr, uint64_t val) {
  return __c11_atomic_fetch_or((_Atomic uint64_t*)addr, val, __ATOMIC_SEQ_CST);
}

uint8_t emscripten_atomic_xor_u8(void /*uint8_t*/* addr, uint8_t val) {
  return __c11_atomic_fetch_xor((_Atomic uint8_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint16_t emscripten_atomic_xor_u16(void /*uint16_t*/* addr, uint16_t val) {
  return __c11_atomic_fetch_xor((_Atomic uint16_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint32_t emscripten_atomic_xor_u32(void /*uint32_t*/* addr, uint32_t val) {
  return __c11_atomic_fetch_xor((_Atomic uint32_t*)addr, val, __ATOMIC_SEQ_CST);
}
uint64_t emscripten_atomic_xor_u64(void /*uint64_t*/* addr, uint64_t val) {
  return __c11_atomic_fetch_xor((_Atomic uint64_t*)addr, val, __ATOMIC_SEQ_CST);
}

extern void __wasm_init_tls(void *memory);
void *emscripten_builtin_memalign(size_t align, size_t size);
void emscripten_builtin_free(void *memory);

// Note that ASan constructor priority is 50, and we must be higher.
__attribute__((constructor(49)))
void EMSCRIPTEN_KEEPALIVE emscripten_tls_init(void) {
  size_t tls_size = __builtin_wasm_tls_size();
  size_t tls_align = __builtin_wasm_tls_align();
  if (tls_size) {
    void *tls_block = emscripten_builtin_memalign(tls_align, tls_size);
    __wasm_init_tls(tls_block);
    pthread_cleanup_push(emscripten_builtin_free, tls_block);
  }
}
