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

// Note on 64bit atomics ops: All 64-bit atomic ops defined here, while single
// instruction under wasm, will be emulated by using locks in wasm2js mode.
// This is also true for C/C++ native atomics as well as intrinsics.

// Atomically stores the given value to the memory location, and returns the
// value that was there prior to the store.
uint8_t emscripten_atomic_exchange_u8(void/*uint8_t*/ *addr, uint8_t newVal);
uint16_t emscripten_atomic_exchange_u16(void/*uint16_t*/ *addr, uint16_t newVal);
uint32_t emscripten_atomic_exchange_u32(void/*uint32_t*/ *addr, uint32_t newVal);
uint64_t emscripten_atomic_exchange_u64(void/*uint64_t*/ *addr, uint64_t newVal);

// CAS returns the *old* value that was in the memory location before the
// operation took place.
// That is, if the return value when calling this function equals to 'oldVal',
// then the operation succeeded, otherwise it was ignored.
uint8_t emscripten_atomic_cas_u8(void/*uint8_t*/ *addr, uint8_t oldVal, uint8_t newVal);
uint16_t emscripten_atomic_cas_u16(void/*uint16_t*/ *addr, uint16_t oldVal, uint16_t newVal);
uint32_t emscripten_atomic_cas_u32(void/*uint32_t*/ *addr, uint32_t oldVal, uint32_t newVal);
uint64_t emscripten_atomic_cas_u64(void/*uint64_t*/ *addr, uint64_t oldVal, uint64_t newVal);

uint8_t emscripten_atomic_load_u8(const void/*uint8_t*/ *addr);
uint16_t emscripten_atomic_load_u16(const void/*uint16_t*/ *addr);
uint32_t emscripten_atomic_load_u32(const void/*uint32_t*/ *addr);
float emscripten_atomic_load_f32(const void/*float*/ *addr);
uint64_t emscripten_atomic_load_u64(const void/*uint64_t*/ *addr);
double emscripten_atomic_load_f64(const void/*double*/ *addr);

// Returns the value that was stored (i.e. 'val')
uint8_t emscripten_atomic_store_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_store_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_store_u32(void/*uint32_t*/ *addr, uint32_t val);
float emscripten_atomic_store_f32(void/*float*/ *addr, float val);
uint64_t emscripten_atomic_store_u64(void/*uint64_t*/ *addr, uint64_t val);
double emscripten_atomic_store_f64(void/*double*/ *addr, double val);

void emscripten_atomic_fence(void);

// Each of the functions below (add, sub, and, or, xor) return the value that
// was in the memory location before the operation occurred.
uint8_t emscripten_atomic_add_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_add_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_add_u32(void/*uint32_t*/ *addr, uint32_t val);
uint64_t emscripten_atomic_add_u64(void/*uint64_t*/ *addr, uint64_t val);

uint8_t emscripten_atomic_sub_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_sub_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_sub_u32(void/*uint32_t*/ *addr, uint32_t val);
uint64_t emscripten_atomic_sub_u64(void/*uint64_t*/ *addr, uint64_t val);

uint8_t emscripten_atomic_and_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_and_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_and_u32(void/*uint32_t*/ *addr, uint32_t val);
uint64_t emscripten_atomic_and_u64(void/*uint64_t*/ *addr, uint64_t val);

uint8_t emscripten_atomic_or_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_or_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_or_u32(void/*uint32_t*/ *addr, uint32_t val);
uint64_t emscripten_atomic_or_u64(void/*uint64_t*/ *addr, uint64_t val);

uint8_t emscripten_atomic_xor_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_xor_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_xor_u32(void/*uint32_t*/ *addr, uint32_t val);
uint64_t emscripten_atomic_xor_u64(void/*uint64_t*/ *addr, uint64_t val);

#ifdef __cplusplus
}
#endif
