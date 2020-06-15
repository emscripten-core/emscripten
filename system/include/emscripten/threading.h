/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <inttypes.h>
#include <pthread.h>
#include <emscripten/html5.h>

#ifdef __cplusplus
extern "C" {
#endif

// Returns true if the current browser is able to spawn threads with
// pthread_create(), and the compiled page was built with threading support
// enabled. If this returns 0, calls to pthread_create() will fail with return
// code EAGAIN.
int emscripten_has_threading_support(void);

// Returns the number of logical cores on the system.
int emscripten_num_logical_cores(void);

// Configures the number of logical cores on the system. This can be called at
// startup to specify the number of cores emscripten_num_logical_cores()
// reports. The Emscripten system itself does not use this value internally
// anywhere, it is just a hint to help developers have a single access point
// 'emscripten_num_logical_cores()' to query the number of cores in the system.
void emscripten_force_num_logical_cores(int cores);

// Atomically stores the given value to the memory location, and returns the
// value that was there prior to the store.
uint8_t emscripten_atomic_exchange_u8(void/*uint8_t*/ *addr, uint8_t newVal);
uint16_t emscripten_atomic_exchange_u16(void/*uint16_t*/ *addr, uint16_t newVal);
uint32_t emscripten_atomic_exchange_u32(void/*uint32_t*/ *addr, uint32_t newVal);
uint64_t emscripten_atomic_exchange_u64(void/*uint64_t*/ *addr, uint64_t newVal); // In asm.js/asm2wasm this is emulated with locks, very slow!

// CAS returns the *old* value that was in the memory location before the
// operation took place.
// That is, if the return value when calling this function equals to 'oldVal',
// then the operation succeeded, otherwise it was ignored.
uint8_t emscripten_atomic_cas_u8(void/*uint8_t*/ *addr, uint8_t oldVal, uint8_t newVal);
uint16_t emscripten_atomic_cas_u16(void/*uint16_t*/ *addr, uint16_t oldVal, uint16_t newVal);
uint32_t emscripten_atomic_cas_u32(void/*uint32_t*/ *addr, uint32_t oldVal, uint32_t newVal);
// In Wasm, this is a native instruction. In asm.js this is emulated with locks,
// very slow!
uint64_t emscripten_atomic_cas_u64(void/*uint64_t*/ *addr, uint64_t oldVal, uint64_t newVal);

uint8_t emscripten_atomic_load_u8(const void/*uint8_t*/ *addr);
uint16_t emscripten_atomic_load_u16(const void/*uint16_t*/ *addr);
uint32_t emscripten_atomic_load_u32(const void/*uint32_t*/ *addr);
float emscripten_atomic_load_f32(const void/*float*/ *addr);
// In Wasm, this is a native instruction. In asm.js this is emulated with locks,
// very slow!
uint64_t emscripten_atomic_load_u64(const void/*uint64_t*/ *addr);
// In Wasm, this is a native instruction. In asm.js this is emulated with locks,
// very slow!
double emscripten_atomic_load_f64(const void/*double*/ *addr);

// Returns the value that was stored (i.e. 'val')
uint8_t emscripten_atomic_store_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_store_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_store_u32(void/*uint32_t*/ *addr, uint32_t val);
float emscripten_atomic_store_f32(void/*float*/ *addr, float val);
// In Wasm, this is a native instruction. In asm.js this is emulated with locks,
// very slow!
uint64_t emscripten_atomic_store_u64(void/*uint64_t*/ *addr, uint64_t val);
// In Wasm, this is a native instruction. In asm.js this is emulated with locks,
// very slow!
double emscripten_atomic_store_f64(void/*double*/ *addr, double val);

void emscripten_atomic_fence(void);

// Each of the functions below (add, sub, and, or, xor) return the value that
// was in the memory location before the operation occurred.
uint8_t emscripten_atomic_add_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_add_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_add_u32(void/*uint32_t*/ *addr, uint32_t val);
// In Wasm, this is a native instruction. In asm.js this is emulated with locks,
// very slow!
uint64_t emscripten_atomic_add_u64(void/*uint64_t*/ *addr, uint64_t val);

uint8_t emscripten_atomic_sub_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_sub_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_sub_u32(void/*uint32_t*/ *addr, uint32_t val);
// In Wasm, this is a native instruction. In asm.js this is emulated with locks,
// very slow!
uint64_t emscripten_atomic_sub_u64(void/*uint64_t*/ *addr, uint64_t val);

uint8_t emscripten_atomic_and_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_and_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_and_u32(void/*uint32_t*/ *addr, uint32_t val);
// In Wasm, this is a native instruction. In asm.js this is emulated with locks,
// very slow!
uint64_t emscripten_atomic_and_u64(void/*uint64_t*/ *addr, uint64_t val);

uint8_t emscripten_atomic_or_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_or_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_or_u32(void/*uint32_t*/ *addr, uint32_t val);
// In Wasm, this is a native instruction. In asm.js this is emulated with locks,
// very slow!
uint64_t emscripten_atomic_or_u64(void/*uint64_t*/ *addr, uint64_t val);

uint8_t emscripten_atomic_xor_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_xor_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_xor_u32(void/*uint32_t*/ *addr, uint32_t val);
// In Wasm, this is a native instruction. In asm.js this is emulated with locks,
// very slow!
uint64_t emscripten_atomic_xor_u64(void/*uint64_t*/ *addr, uint64_t val);

// If the given memory address contains value val, puts the calling thread to
// sleep waiting for that address to be notified.
int emscripten_futex_wait(volatile void/*uint32_t*/ *addr, uint32_t val, double maxWaitMilliseconds);

// Wakes the given number of threads waiting on a location. Pass count ==
// INT_MAX to wake all waiters on that location.
int emscripten_futex_wake(volatile void/*uint32_t*/ *addr, int count);

typedef union em_variant_val
{
  int i;
  int64_t i64;
  float f;
  double d;
  void *vp;
  char *cp;
} em_variant_val;

// Proxied C/C++ functions support at most this many arguments. Dispatch is
// static/strongly typed by signature.
#define EM_QUEUED_CALL_MAX_ARGS 11
// Proxied JS function can support a few more arguments than proxied C/C++
// functions, because the dispatch is variadic and signature independent.
#define EM_QUEUED_JS_CALL_MAX_ARGS 20
typedef struct em_queued_call
{
  int functionEnum;
  void *functionPtr;
  int operationDone;
  em_variant_val args[EM_QUEUED_JS_CALL_MAX_ARGS];
  em_variant_val returnValue;

  // An optional pointer to a secondary data block that should be free()d when
  // this queued call is freed.
  void *satelliteData;

  // If true, the caller has "detached" itself from this call object and the
  // Emscripten main runtime thread should free up this em_queued_call object
  // after it has been executed. If false, the caller is in control of the
  // memory.
  int calleeDelete;
} em_queued_call;

void emscripten_sync_run_in_main_thread(em_queued_call *call);
void *emscripten_sync_run_in_main_thread_0(int function);
void *emscripten_sync_run_in_main_thread_1(int function, void *arg1);
void *emscripten_sync_run_in_main_thread_2(int function, void *arg1, void *arg2);
void *emscripten_sync_run_in_main_thread_3(int function, void *arg1, void *arg2, void *arg3);
void *emscripten_sync_run_in_main_thread_7(int function, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7);

typedef void (*em_func_v)(void);
typedef void (*em_func_vi)(int);
typedef void (*em_func_vf)(float);
typedef void (*em_func_vii)(int, int);
typedef void (*em_func_vif)(int, float);
typedef void (*em_func_vff)(float, float);
typedef void (*em_func_viii)(int, int, int);
typedef void (*em_func_viif)(int, int, float);
typedef void (*em_func_viff)(int, float, float);
typedef void (*em_func_vfff)(float, float, float);
typedef void (*em_func_viiii)(int, int, int, int);
typedef void (*em_func_viifi)(int, int, float, int);
typedef void (*em_func_vifff)(int, float, float, float);
typedef void (*em_func_vffff)(float, float, float, float);
typedef void (*em_func_viiiii)(int, int, int, int, int);
typedef void (*em_func_viffff)(int, float, float, float, float);
typedef void (*em_func_viiiiii)(int, int, int, int, int, int);
typedef void (*em_func_viiiiiii)(int, int, int, int, int, int, int);
typedef void (*em_func_viiiiiiii)(int, int, int, int, int, int, int, int);
typedef void (*em_func_viiiiiiiii)(int, int, int, int, int, int, int, int, int);
typedef void (*em_func_viiiiiiiiii)(int, int, int, int, int, int, int, int, int, int);
typedef void (*em_func_viiiiiiiiiii)(int, int, int, int, int, int, int, int, int, int, int);
typedef int (*em_func_i)(void);
typedef int (*em_func_ii)(int);
typedef int (*em_func_iii)(int, int);
typedef int (*em_func_iiii)(int, int, int);
typedef int (*em_func_iiiii)(int, int, int, int);
typedef int (*em_func_iiiiii)(int, int, int, int, int);
typedef int (*em_func_iiiiiii)(int, int, int, int, int, int);
typedef int (*em_func_iiiiiiii)(int, int, int, int, int, int, int);
typedef int (*em_func_iiiiiiiii)(int, int, int, int, int, int, int, int);
typedef int (*em_func_iiiiiiiiii)(int, int, int, int, int, int, int, int, int);

// Encode function signatures into a single uint32_t integer.
// N.B. This encoding scheme is internal to the implementation, and can change
// in the future. Do not depend on the exact numbers in this scheme.
#define EM_FUNC_SIGNATURE unsigned int

// The encoding scheme is as follows:
// - highest three bits identify the type of the return value
#define EM_FUNC_SIG_RETURN_VALUE_MASK (0x7U << 29)

#define EM_FUNC_SIG_RETURN_VALUE_V   0
#define EM_FUNC_SIG_RETURN_VALUE_I   (0x1U << 29)
#define EM_FUNC_SIG_RETURN_VALUE_I64 (0x2U << 29)
#define EM_FUNC_SIG_RETURN_VALUE_F   (0x3U << 29)
#define EM_FUNC_SIG_RETURN_VALUE_D   (0x4U << 29)

// - next highest four bits specify the number of input parameters to the
//   function (allowed values are 0-12, inclusively)
#define EM_FUNC_SIG_NUM_PARAMETERS_SHIFT 25
#define EM_FUNC_SIG_NUM_PARAMETERS_MASK (0xFU << EM_FUNC_SIG_NUM_PARAMETERS_SHIFT)
#define EM_FUNC_SIG_WITH_N_PARAMETERS(x) (((EM_FUNC_SIGNATURE)(x)) << EM_FUNC_SIG_NUM_PARAMETERS_SHIFT)

// - starting from the lowest bits upwards, each pair of two subsequent bits
//   specifies the type of an input parameter.
//   That is, bits 1:0 encode the type of the first input, bits 3:2 encode the
//   type of the second input, and so on.
#define EM_FUNC_SIG_ARGUMENTS_TYPE_MASK (~(EM_FUNC_SIG_RETURN_VALUE_MASK | EM_FUNC_SIG_NUM_PARAMETERS_MASK))
#define EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_MASK 0x3U
#define EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_SHIFT 2

#define EM_FUNC_SIG_PARAM_I   0
#define EM_FUNC_SIG_PARAM_I64 0x1U
#define EM_FUNC_SIG_PARAM_F   0x2U
#define EM_FUNC_SIG_PARAM_D   0x3U
#define EM_FUNC_SIG_SET_PARAM(i, type) ((EM_FUNC_SIGNATURE)(type) << (2*i))

// Extra types used in WebGL glGet*() calls (not used in proxying)
#define EM_FUNC_SIG_PARAM_B   0x4U
#define EM_FUNC_SIG_PARAM_F2I 0x5U

// In total, the above encoding scheme gives the following 32-bit structure for
// the proxied function signatures (highest -> lowest bit order):
// RRRiiiiSbbaa99887766554433221100
// where RRR is return type
// iiii is the number of inputs
// S denotes a special function (internal proxying mechanism for functions
// related to built-in threading APIs, like thread creation itself)
// 00-bb encode the type of up to 12 function parameters

#define EM_FUNC_SIG_V     (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(0))
#define EM_FUNC_SIG_D     (EM_FUNC_SIG_RETURN_VALUE_D | EM_FUNC_SIG_WITH_N_PARAMETERS(0))
#define EM_FUNC_SIG_VI    (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(1) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VF    (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(1) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_FI    (EM_FUNC_SIG_RETURN_VALUE_F | EM_FUNC_SIG_WITH_N_PARAMETERS(1) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_DI    (EM_FUNC_SIG_RETURN_VALUE_D | EM_FUNC_SIG_WITH_N_PARAMETERS(1) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VII   (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(2) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIF   (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(2) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VFF   (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(2) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VIII  (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIF  (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VIFF  (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VFFF  (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIFI (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIFFF (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VFFFF (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_IIFFF (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(5) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIFFFF (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(5) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VIIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(6) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(7) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIIIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(8) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIIIIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(9) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(8, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIIIIIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(10) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(8, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(9, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIIIIIIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(11) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(8, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(9, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(10, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_I     (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(0))
#define EM_FUNC_SIG_II    (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(1) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_III   (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(2) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIII  (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIIII (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIIIII (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(5) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIIIIII (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(6) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIIIIIII (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(7) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIIIIIIII (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(8) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIIIIIIIII (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(9) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(8, EM_FUNC_SIG_PARAM_I))

#define EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(x) ((((EM_FUNC_SIGNATURE)x) & EM_FUNC_SIG_NUM_PARAMETERS_MASK) >> EM_FUNC_SIG_NUM_PARAMETERS_SHIFT)

// There are some built-in special proxied functions, that embed the signatures
// inside the above encoding scheme
#define EM_FUNC_SIG_SPECIAL_INTERNAL (1 << 24)
#define EM_PROXIED_FUNC_SPECIAL(x) (EM_FUNC_SIG_SPECIAL_INTERNAL | ((x) << 20))

#define EM_PROXIED_PTHREAD_CREATE (EM_PROXIED_FUNC_SPECIAL(0) | EM_FUNC_SIG_IIIII)
#define EM_PROXIED_CREATE_CONTEXT (EM_PROXIED_FUNC_SPECIAL(2) | EM_FUNC_SIG_III)
#define EM_PROXIED_RESIZE_OFFSCREENCANVAS (EM_PROXIED_FUNC_SPECIAL(3) | EM_FUNC_SIG_IIII)
#define EM_PROXIED_JS_FUNCTION (EM_PROXIED_FUNC_SPECIAL(4) | EM_FUNC_SIG_D)

// Runs the given function synchronously on the main Emscripten runtime thread.
// If this thread is the main thread, the operation is immediately performed,
// and the result is returned.
// If the current thread is not the main Emscripten runtime thread (but a
// pthread), the function
// will be proxied to be called by the main thread.
//  - Calling emscripten_sync_* functions requires that the application was
//    compiled with pthreads support enabled (-s USE_PTHREADS=1/2) and that the
//    browser supports SharedArrayBuffer specification.
int emscripten_sync_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr, ...);

// The 'async' variant of the run_in_main_thread functions are otherwise the
// same as the synchronous ones, except that the operation is performed in a
// fire and forget manner. The call is placed to the command queue of the main
// Emscripten runtime thread, but its completion is not waited for. As a result,
// if the function did have a return value, the return value is not received.
//  - Note that multiple asynchronous commands from a single pthread/Worker are
//    guaranteed to be executed on the main thread in the program order they
//    were called in.
void emscripten_async_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr, ...);

// The 'async_waitable' variant of the run_in_main_runtime_thread functions run
// like the 'async' variants, except that while the operation starts off
// asynchronously, the result is then later waited upon to receive the return
// value.
//  - The object returned by this function call is dynamically allocated, and
//    should be freed up via a call to emscripten_async_waitable_close() after
//    the wait has been performed.
em_queued_call *emscripten_async_waitable_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr, ...);

// Since we can't validate the function pointer type, allow implicit casting of
// functions to void* without complaining.
#define emscripten_sync_run_in_main_runtime_thread(sig, func_ptr, ...) emscripten_sync_run_in_main_runtime_thread_((sig), (void*)(func_ptr),##__VA_ARGS__)
#define emscripten_async_run_in_main_runtime_thread(sig, func_ptr, ...) emscripten_async_run_in_main_runtime_thread_((sig), (void*)(func_ptr),##__VA_ARGS__)
#define emscripten_async_waitable_run_in_main_runtime_thread(sig, func_ptr, ...) emscripten_async_waitable_run_in_main_runtime_thread_((sig), (void*)(func_ptr),##__VA_ARGS__)

EMSCRIPTEN_RESULT emscripten_wait_for_call_v(em_queued_call *call, double timeoutMSecs);
EMSCRIPTEN_RESULT emscripten_wait_for_call_i(em_queued_call *call, double timeoutMSecs, int *outResult);

void emscripten_async_waitable_close(em_queued_call *call);

int _emscripten_call_on_thread(int force_async, pthread_t target_thread, EM_FUNC_SIGNATURE sig, void *func_ptr, void *satellite, ...); // internal

// Runs the given function on the specified thread. If we are currently on
// that target thread then we just execute the call synchronously; otherwise it
// is queued on that thread to execute asynchronously.
// Returns 1 if it executed the code (i.e., it was on the target thread), and 0
// otherwise.
#define emscripten_dispatch_to_thread(target_thread, sig, func_ptr, satellite, ...) _emscripten_call_on_thread(0, (target_thread), (sig), (void*)(func_ptr), (satellite),##__VA_ARGS__)

// Similar to emscripten_dispatch_to_thread, but always runs the
// function asynchronously, even if on the same thread. This is less efficient
// but may be simpler to reason about in some cases.
#define emscripten_dispatch_to_thread_async(target_thread, sig, func_ptr, satellite, ...) _emscripten_call_on_thread(1, (target_thread), (sig), (void*)(func_ptr), (satellite),##__VA_ARGS__)

// Returns 1 if the current thread is the thread that hosts the Emscripten runtime.
int emscripten_is_main_runtime_thread(void);

// Returns 1 if the current thread is the main browser thread.
int emscripten_is_main_browser_thread(void);

// A temporary workaround to issue
// https://github.com/emscripten-core/emscripten/issues/3495:
// Call this in the body of all lock-free atomic (cas) loops that the main
// thread might enter which don't otherwise call to any pthread api calls
// (mutexes) or C runtime functions that are considered cancellation points.
void emscripten_main_thread_process_queued_calls(void);

void emscripten_current_thread_process_queued_calls(void);

pthread_t emscripten_main_browser_thread_id(void);

// Synchronously sleeps the calling thread for the given number of milliseconds.
// Note: Calling this on the main browser thread is _very_ _very_ bad for
// application logic throttling, because it does not save any battery, it will
// spin up the CPU at 100%, lock up the UI, printfs will not come through on web
// page or the console, and eventually it will show up the slow script dialog.
// Calling this function in a pthread (Web Worker) is fine, and a good way to go
// if you need to synchronously sleep for a specific amount of time while saving
// power.
// Note 2: This function will process the pthread-specific event queue for the
//         calling thread while sleeping, and this function also acts as a
//         cancellation point.
// Note 3: This function is enabled when targeting pthreads (SharedArrayBuffer),
//         not to be confused with
//         similarly named function emscripten_sleep(), which is intended for
//         Asyncify builds.
void emscripten_thread_sleep(double msecs);

#define EM_THREAD_STATUS int
#define EM_THREAD_STATUS_NOTSTARTED 0
#define EM_THREAD_STATUS_RUNNING    1
#define EM_THREAD_STATUS_SLEEPING   2 // Performing an unconditional sleep (usleep, etc.)
#define EM_THREAD_STATUS_WAITFUTEX  3 // Waiting for an explicit low-level futex (emscripten_futex_wait)
#define EM_THREAD_STATUS_WAITMUTEX  4 // Waiting for a pthread_mutex_t
#define EM_THREAD_STATUS_WAITPROXY  5 // Waiting for a proxied operation to finish.
#define EM_THREAD_STATUS_FINISHED   6
#define EM_THREAD_STATUS_NUMFIELDS  7

// Sets the profiler status of the calling thread. This is a no-op if thread
// profiling is not active.
// This is an internal function and generally not intended for user code.
// When thread profiler is not enabled (not building with --threadprofiling),
// this is a no-op.
void emscripten_set_current_thread_status(EM_THREAD_STATUS newStatus);

// Sets the profiler status of the calling thread, but only if it was in the
// expected status beforehand.
// This is an internal function and generally not intended for user code.
// When thread profiler is not enabled (not building with --threadprofiling),
// this is a no-op.
void emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS expectedStatus, EM_THREAD_STATUS newStatus);

// Sets the name of the given thread. Pass pthread_self() as the thread ID to
// set the name of the calling thread.
// The name parameter is a UTF-8 encoded string which is truncated to 32 bytes.
// When thread profiler is not enabled (not building with --threadprofiling),
// this is a no-op.
void emscripten_set_thread_name(pthread_t threadId, const char *name);

// Gets the stored pointer to a string representing the canvases to transfer to
// the created thread.
int emscripten_pthread_attr_gettransferredcanvases(const pthread_attr_t *a, const char **str);

// Specifies a comma-delimited list of canvas DOM element IDs to transfer to the
// thread to be created.
// Note: this pointer is weakly stored (not copied) to the given pthread_attr_t,
// so must be held alive until pthread_create() has been called. If 0 or "", no
// canvases are transferred.
// The special value "#canvas" denotes the element stored in Module.canvas.
int emscripten_pthread_attr_settransferredcanvases(pthread_attr_t *a, const char *str);

struct thread_profiler_block
{
  // One of THREAD_STATUS_*
  int threadStatus;
  // Wallclock time denoting when the current thread state was entered in.
  double currentStatusStartTime;
  // Accumulated duration times denoting how much time has been spent in each
  // state, in msecs.
  double timeSpentInStatus[EM_THREAD_STATUS_NUMFIELDS];
  // A human-readable name for this thread.
  char name[32];
};

// Called when blocking on the main thread. This will error if main thread
// blocking is not enabled, see ALLOW_BLOCKING_ON_MAIN_THREAD.
void emscripten_check_blocking_allowed(void);

#ifdef __cplusplus
}
#endif
