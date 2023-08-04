/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <inttypes.h>
#include <pthread.h>

#include <emscripten/html5.h>  // for EMSCRIPTEN_RESULT
#include <emscripten/atomic.h>

// Legacy proxying functions.  See proxying.h for the new proxying system.
#include "threading_legacy.h"

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

// If the given memory address contains value val, puts the calling thread to
// sleep waiting for that address to be notified.
// Returns -EINVAL if addr is null.
int emscripten_futex_wait(volatile void/*uint32_t*/ * _Nonnull addr, uint32_t val, double maxWaitMilliseconds);

// Wakes the given number of threads waiting on a location. Pass count ==
// INT_MAX to wake all waiters on that location.
// Returns -EINVAL if addr is null.
int emscripten_futex_wake(volatile void/*uint32_t*/ * _Nonnull addr, int count);

// Returns 1 if the current thread is the thread that hosts the Emscripten
// runtime.
int emscripten_is_main_runtime_thread(void);

// Returns 1 if the current thread is the main browser thread.  In the case that
// the emscripten module is run in a worker there may be no pthread for which
// this returns 1.
int emscripten_is_main_browser_thread(void);

// A temporary workaround to issue
// https://github.com/emscripten-core/emscripten/issues/3495:
// Call this in the body of all lock-free atomic (cas) loops that the main
// thread might enter which don't otherwise call to any pthread api calls
// (mutexes) or C runtime functions that are considered cancellation points.
void emscripten_main_thread_process_queued_calls(void);

void emscripten_current_thread_process_queued_calls(void);

// Returns the thread ID of the thread that hosts the Emscripten runtime.
pthread_t emscripten_main_runtime_thread_id(void);

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

// Sets the name of the given thread. Pass pthread_self() as the thread ID to
// set the name of the calling thread.
// The name parameter is a UTF-8 encoded string which is truncated to 32 bytes.
// When thread profiler is not enabled (not building with --threadprofiler),
// this is a no-op.
void emscripten_set_thread_name(pthread_t threadId, const char *name __attribute__((nonnull)));

// Gets the stored pointer to a string representing the canvases to transfer to
// the created thread.
int emscripten_pthread_attr_gettransferredcanvases(const pthread_attr_t *a __attribute__((nonnull)), const char **str __attribute__((nonnull)));

// Specifies a comma-delimited list of canvas DOM element IDs to transfer to the
// thread to be created.
// Note: this pointer is weakly stored (not copied) to the given pthread_attr_t,
// so must be held alive until pthread_create() has been called. If 0 or "", no
// canvases are transferred.
// The special value "#canvas" denotes the element stored in Module.canvas.
int emscripten_pthread_attr_settransferredcanvases(pthread_attr_t *a __attribute__((nonnull)), const char *str __attribute__((nonnull)));

// Called when blocking on the main thread. This will error if main thread
// blocking is not enabled, see ALLOW_BLOCKING_ON_MAIN_THREAD.
void emscripten_check_blocking_allowed(void);

#ifdef __cplusplus
}
#endif
