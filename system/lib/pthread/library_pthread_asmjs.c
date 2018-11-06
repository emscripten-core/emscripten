/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// This file contains support machinery to implement multithreading support for asm.js specifically.
// Split out to a separate file so that its contents can be easily left out when only doing Wasm
// multithreading.

#include <pthread.h>
#include <emscripten/threading.h>
#include <emscripten.h>

// Use an array of multiple interleaved spinlock mutexes to separate memory addresses to ease pressure when locking.
// This is outright horrible, but enables easily porting code that does require 64-bit atomics.
// Eventually in the long run we'd hope to have real support for 64-bit atomics in the browser, after
// which this emulation can be removed.
#define NUM_64BIT_LOCKS 256
static int emulated64BitAtomicsLocks[NUM_64BIT_LOCKS] = {};

#define SPINLOCK_ACQUIRE(addr) do { while(emscripten_atomic_cas_u32((void*)(addr), 0, 1)) /*nop*/; } while(0)
#define SPINLOCK_RELEASE(addr) emscripten_atomic_store_u32((void*)(addr), 0)

float EMSCRIPTEN_KEEPALIVE emscripten_atomic_load_f32(const void *addr)
{
	union {
		float f;
		uint32_t u;
	} u;
	u.u = emscripten_atomic_load_u32(addr);
	return u.f;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_exchange_u64(void/*uint64_t*/ *addr, uint64_t newVal)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldValInMemory = *(uint64_t*)addr;
	*(uint64_t*)addr = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldValInMemory;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_cas_u64(void/*uint64_t*/ *addr, uint64_t oldVal, uint64_t newVal)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldValInMemory = *(uint64_t*)addr;
	if (oldValInMemory == oldVal)
		*(uint64_t*)addr = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldValInMemory;
}

double EMSCRIPTEN_KEEPALIVE emscripten_atomic_load_f64(const void *addr)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	double val = *(double*)addr;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return val;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_load_u64(const void *addr)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t val = *(uint64_t*)addr;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return val;
}

float EMSCRIPTEN_KEEPALIVE emscripten_atomic_store_f32(void *addr, float val)
{
	union {
		float f;
		uint32_t u;
	} u;
	u.f = val;
	return emscripten_atomic_store_u32(addr, u.u);
}

double EMSCRIPTEN_KEEPALIVE emscripten_atomic_store_f64(void *addr, double val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	double *a = (double*)addr;
	*a = val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return val;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_store_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t*)addr;
	*a = val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return val;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_add_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal + val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_add. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_add_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal + val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_sub_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal - val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_sub. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_sub_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal - val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_and_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal & val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_and. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_and_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal & val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_or_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal | val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_or. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_or_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal | val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_xor_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal ^ val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_xor. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_xor_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal ^ val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t __atomic_load_8(void *ptr, int memmodel)
{
  return emscripten_atomic_load_u64(ptr);
}

void __atomic_store_8(void *ptr, uint64_t value, int memmodel)
{
  emscripten_atomic_store_u64(ptr, value);
}

uint64_t __atomic_exchange_8(void *ptr, uint64_t value, int memmodel)
{
  return emscripten_atomic_exchange_u64(ptr, value);
}

uint64_t __atomic_compare_exchange_8(void *ptr, uint64_t *expected, uint64_t desired, int weak, int success_memmodel, int failure_memmodel)
{
  return emscripten_atomic_cas_u64(ptr, *expected, desired);
}

uint64_t __atomic_fetch_add_8(void *ptr, uint64_t value, int memmodel)
{
  return _emscripten_atomic_fetch_and_add_u64(ptr, value);
}

uint64_t __atomic_fetch_sub_8(void *ptr, uint64_t value, int memmodel)
{
  return _emscripten_atomic_fetch_and_sub_u64(ptr, value);
}

uint64_t __atomic_fetch_and_8(void *ptr, uint64_t value, int memmodel)
{
  return _emscripten_atomic_fetch_and_and_u64(ptr, value);
}

uint64_t __atomic_fetch_or_8(void *ptr, uint64_t value, int memmodel)
{
  return _emscripten_atomic_fetch_and_or_u64(ptr, value);
}

uint64_t __atomic_fetch_xor_8(void *ptr, uint64_t value, int memmodel)
{
  return _emscripten_atomic_fetch_and_xor_u64(ptr, value);
}
