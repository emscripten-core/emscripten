/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <emscripten/atomic.h>
#include <emscripten/em_types.h>
#include <emscripten/threading_primitives.h>

#ifdef __cplusplus
extern "C" {
#endif

#define emscripten_wasm_worker_t int
#define EMSCRIPTEN_WASM_WORKER_ID_PARENT 0

// Creates a new Worker() that is attached to executing this
// WebAssembly.Instance and WebAssembly.Memory.
//
// emscripten_malloc_wasm_worker:
//   Creates a new Worker, dynamically allocating stack and TLS for it.
//   Unfortunately due to the asynchronous no-notifications nature of how Worker
//   API specification teardown behaves, the dynamically allocated memory can
//   never be freed, so use this function only in scenarios where the page does
//   not need to deinitialize/tear itself down.
//
// emscripten_create_wasm_worker:
//   Creates a Wasm Worker given a preallocated region for stack and TLS data.
//   Use this function to manually manage the memory that a Worker should use.
//   This function does not use any dynamic memory allocation.
//   Unlike with the above function, this variant requires that size of the
//   region provided is large enough to hold both stack and TLS area.
//   The size of the TLS area can be determined at runtime by calling
//   __builtin_wasm_tls_size().
//
// Returns an ID that represents the given Worker. If not building with Wasm
// workers enabled (-sWASM_WORKERS=0), these functions will return 0 to denote
// failure.
// Note that the Worker will be loaded up asynchronously, and initially will not
// be executing any code. Use emscripten_wasm_worker_post_function_*() set of
// functions to start executing code on the Worker.
emscripten_wasm_worker_t emscripten_malloc_wasm_worker(size_t stackSize);
emscripten_wasm_worker_t emscripten_create_wasm_worker(void * _Nonnull stackPlusTLSAddress, size_t stackPlusTLSSize);

// Terminates the given Wasm Worker some time after it has finished executing
// its current, or possibly some subsequent posted functions. Note that this
// function is not C++ RAII safe, but you must manually coordinate to release
// any resources from the given Worker that it may have allocated from the heap
// or may have stored on its TLS slots.  There are no TLS destructors that would
// execute.
// Exists, but is a no-op if not building with Wasm Workers enabled
// (-sWASM_WORKERS=0)
void emscripten_terminate_wasm_worker(emscripten_wasm_worker_t id);

// Note the comment on emscripten_terminate_wasm_worker(id) about thread
// destruction.
// Exists, but is a no-op if not building with Wasm Workers enabled
// (-sWASM_WORKERS=0)
void emscripten_terminate_all_wasm_workers(void);

// Returns true if the current thread is executing a Wasm Worker, false
// otherwise.  Note that calling this function can be relatively slow as it
// incurs a Wasm->JS transition, so avoid calling it in hot paths.
bool emscripten_current_thread_is_wasm_worker(void);

// Returns a unique ID that identifies the calling Wasm Worker. Similar to
// pthread_self().  The main browser thread will return 0 as the ID. First Wasm
// Worker will return 1, and so on.
uint32_t emscripten_wasm_worker_self_id(void);

// emscripten_wasm_worker_post_function_*: Post a pointer to a C/C++ function to
// be executed on the target Wasm Worker (via sending a postMessage() to the
// target thread). Notes: If running inside a Wasm Worker, specify worker ID 0
// to pass a message to the parent thread.  When specifying non-zero ID, the
// target worker must have been created by the calling thread. That is, a Wasm
// Worker can only send a message to its parent or its children, but not to its
// siblings.  The target function pointer will be executed on the target Worker
// only after it yields back to its event loop. If the target Wasm Worker
// executes an infinite loop that never yields, then the function pointer will
// never be called.
// Passing messages between threads with this family of functions is relatively
// slow and has a really high latency cost compared to direct coordination using
// atomics and synchronization primitives like mutexes. Additionally these 
// functions will generate garbage on the JS heap.  Therefore avoid using these 
// functions where performance is critical.
void emscripten_wasm_worker_post_function_v(emscripten_wasm_worker_t id, void (* _Nonnull funcPtr)(void));
void emscripten_wasm_worker_post_function_vi(emscripten_wasm_worker_t id, void (* _Nonnull funcPtr)(int), int arg0);
void emscripten_wasm_worker_post_function_vii(emscripten_wasm_worker_t id, void (* _Nonnull funcPtr)(int, int), int arg0, int arg1);
void emscripten_wasm_worker_post_function_viii(emscripten_wasm_worker_t id, void (* _Nonnull funcPtr)(int, int, int), int arg0, int arg1, int arg2);
void emscripten_wasm_worker_post_function_vd(emscripten_wasm_worker_t id, void (* _Nonnull funcPtr)(double), double arg0);
void emscripten_wasm_worker_post_function_vdd(emscripten_wasm_worker_t id, void (* _Nonnull funcPtr)(double, double), double arg0, double arg1);
void emscripten_wasm_worker_post_function_vddd(emscripten_wasm_worker_t id, void (* _Nonnull funcPtr)(double, double, double), double arg0, double arg1, double arg2);
void emscripten_wasm_worker_post_function_sig(emscripten_wasm_worker_t id, void * _Nonnull funcPtr, const char * _Nonnull sig, ...);

// Sleeps the calling wasm worker for the given nanoseconds. Calling this
// function on the main thread either results in a TypeError exception
// (Firefox), or a silent return without waiting (Chrome), see
// https://github.com/WebAssembly/threads/issues/174
void emscripten_wasm_worker_sleep(int64_t nanoseconds);

// Returns the value of navigator.hardwareConcurrency, i.e. the number of
// logical threads available for the user agent. NOTE: If the execution
// environment does not support navigator.hardwareConcurrency, this function
// will return zero to signal no support. (If the value 1 is returned, then it
// means that navigator.hardwareConcurrency is supported, but there is only one
// logical thread of concurrency available)
int emscripten_navigator_hardware_concurrency(void);

// Legacy names for emscripten_atomic_wait/notify functions, defined in
// emscripten/atomic.h
#define emscripten_wasm_wait_i32 emscripten_atomic_wait_u32
#define emscripten_wasm_wait_i64 emscripten_atomic_wait_u64
#define emscripten_wasm_notify emscripten_atomic_notify
#pragma clang deprecated(emscripten_wasm_wait_i32, "use emscripten_atomic_wait_u32 instead")
#pragma clang deprecated(emscripten_wasm_wait_i64, "use emscripten_atomic_wait_u64 instead")
#pragma clang deprecated(emscripten_wasm_notify, "use emscripten_atomic_notify instead")

#ifdef __cplusplus
}
#endif
