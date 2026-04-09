# Design Doc: Wasm Worker Pthread Compatibility

- **Status**: Draft
- **Bug**: https://github.com/emscripten-core/emscripten/issues/26631

## Context

Wasm Workers in Emscripten are a lightweight alternative to pthreads. They use
the same memory and can use the same synchronization primitives, but they do not
have a full `struct pthread` and thus many pthread-based APIs (like
`pthread_self()`) currently do not work when called from a Wasm Worker.

This is not an issue in pure Wasm Workers programs but we also support hybrid
programs that run both pthreads and Wasm Workers.  In this cases the pthread
API is available, but will fail in undefined ways if called from Wasm Workers.

This document proposes a plan to improve the hybrid mode by adding the pthread
metadata (`struct pthread`) to each Wasm Worker, allowing the pthread API (or at
least some subset of it) APIs to used from Wasm Workers.

## Proposed Changes

### 1. Memory Layout

Currently, Wasm Workers allocate space for TLS and stack: `[TLS data] [Stack]`.
We propose to change this to: `[struct pthread] [TLS data] [Stack]`.

The `struct pthread` will be located at the very beginning of the allocated
memory block for each Wasm Worker.

### 2. `struct pthread` Initialization

The `struct pthread` will be initialized by the creator thread in `emscripten_create_wasm_worker` (or `emscripten_malloc_wasm_worker`).
This includes:
- Zero-initializing the structure.
- Setting the `self` pointer to the start of the `struct pthread`.
- Initializing essential fields like `tid`.

On the worker thread side, `_emscripten_wasm_worker_initialize` will need to set
the thread-local pointer (returned by `__get_tp()`) to the `struct pthread`
location.

### 3. `__get_tp` Support

We will modify `system/lib/pthread/emscripten_thread_state.S` to provide a
`__get_tp` implementation for Wasm workers that returns the address of the
`struct pthread`. This will allow `__pthread_self()` and other related functions
to work correctly.

### 4. Supported Pthread API Subset

We intend to support a subset of the pthread API within Wasm workers:
- `pthread_self()`: Returns the worker's `struct pthread` pointer.
- `pthread_equal()`: Works normally.
- `pthread_getspecific()` / `pthread_setspecific()`: TSD (Thread Specific Data) should work if `tsd` field in `struct pthread` is initialized.
- `pthread_mutex_*`: Mutexes will work as they rely on `struct pthread` for owner tracking.
- `pthread_cond_*`: Condition variables will work as they rely on `struct pthread` for waiter tracking.
- Low-level synchronization primitives that use `struct pthread` (e.g., some internal locks).

APIs that will NOT be supported (or will have limited support):
- `pthread_create()` / `pthread_join()` / `pthread_detach()`: Wasm workers have their own creation and lifecycle management.
- `pthread_cancel()`: Not supported in Wasm workers.
- `pthread_kill()`: Not supported in Wasm workers.

## Implementation Plan

1. Modify `emscripten_create_wasm_worker` in `system/lib/wasm_worker/library_wasm_worker.c` to account for `sizeof(struct pthread)` in memory allocation and initialize the structure.
2. Update `_emscripten_wasm_worker_initialize` in `system/lib/wasm_worker/wasm_worker_initialize.S` to set the thread pointer.
3. Modify `system/lib/pthread/emscripten_thread_state.S` to enable `__get_tp` for Wasm workers.
4. Review and test essential pthread functions (like TSD) in Wasm workers.
5. Document the supported and unsupported pthread APIs for Wasm workers.

## Verification
- Add a new test that makes use of `pthread_self()` and low level synchronization APIs from a Wasm Worker.
- Verify that existing Wasm worker tests still pass.
