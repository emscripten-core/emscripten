/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <pthread.h>

#define EM_THREAD_NAME_MAX 32

#define EM_THREAD_STATUS int
#define EM_THREAD_STATUS_NOTSTARTED 0
#define EM_THREAD_STATUS_RUNNING    1
#define EM_THREAD_STATUS_SLEEPING   2 // Performing an unconditional sleep (usleep, etc.)
#define EM_THREAD_STATUS_WAITFUTEX  3 // Waiting for an explicit low-level futex (emscripten_futex_wait)
#define EM_THREAD_STATUS_WAITMUTEX  4 // Waiting for a pthread_mutex_t
#define EM_THREAD_STATUS_WAITPROXY  5 // Waiting for a proxied operation to finish.
#define EM_THREAD_STATUS_FINISHED   6
#define EM_THREAD_STATUS_NUMFIELDS  7

typedef struct thread_profiler_block {
  // One of THREAD_STATUS_*
  _Atomic int threadStatus;
  // Wallclock time denoting when the current thread state was entered in.
  double currentStatusStartTime;
  // Accumulated duration times denoting how much time has been spent in each
  // state, in msecs.
  double timeSpentInStatus[EM_THREAD_STATUS_NUMFIELDS];
  // A human-readable name for this thread.
  char name[EM_THREAD_NAME_MAX];
} thread_profiler_block;

// Called whenever a thread performs a blocking action (or calls sched_yield).
// This function takes care of running the event queue and other housekeeping
// tasks.
//
// If that caller already know the current time it can pass it vai the now
// argument.  This can save _emscripten_check_timers from needing to call out to
// JS to get the current time.  Passing 0 means that caller doesn't know the
// the current time.
void _emscripten_yield(double now);

void _emscripten_init_main_thread_js(void* tb);
void _emscripten_thread_profiler_enable();
void _emscripten_thread_cleanup(pthread_t thread);

hidden void* _emscripten_tls_init(void);
hidden void _emscripten_tls_free(void);

// Marks the given thread as strongly referenced. This is used to prevent the
// Node.js application from exiting as long as there are strongly referenced
// threads still running. Normally you don't need to call this function, and
// the pthread behaviour will match native in that background threads won't
// keep runtime alive, but waiting for them via e.g. pthread_join will. 
// However, this is useful for features like PROXY_TO_PTHREAD where we want to
// keep running as long as the detached pthread is.
void _emscripten_thread_set_strongref(pthread_t thread);

// Checks certain structural invariants.  This allows us to detect when
// already-freed threads are used in some APIs.  Technically this is undefined
// behaviour, but we have a couple of places where we add these checks so that
// we can pass more of the posixtest suite that vanilla musl.
int _emscripten_thread_is_valid(pthread_t thread);

void _emscripten_thread_exit_joinable(pthread_t thread);
void _emscripten_thread_exit(void* result);
void _emscripten_process_dlopen_queue(void);

#ifdef NDEBUG
#define emscripten_set_current_thread_status(newStatus)
#define emscripten_conditional_set_current_thread_status(expectedStatus, newStatus)
#else
// Allocate the thread profile block for the given thread.
void _emscripten_thread_profiler_init(pthread_t thread);

// Sets the profiler status of the calling thread. This is a no-op if thread
// profiling is not active.
// This is an internal function and generally not intended for user code.
// When thread profiler is not enabled (not building with --threadprofiler),
// this is a no-op.
void emscripten_set_current_thread_status(EM_THREAD_STATUS newStatus);

// Sets the profiler status of the calling thread, but only if it was in the
// expected status beforehand.
// This is an internal function and generally not intended for user code.
// When thread profiler is not enabled (not building with --threadprofiler),
// this is a no-op.
void emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS expectedStatus, EM_THREAD_STATUS newStatus);
#endif

int __pthread_kill_js(pthread_t t, int sig);
int __pthread_create_js(struct __pthread *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
int _emscripten_default_pthread_stack_size();
void __set_thread_state(pthread_t ptr, int is_main, int is_runtime, int can_block);

double _emscripten_receive_on_main_thread_js(int funcIndex, void* emAsmAddr, pthread_t callingThread, int numCallArgs, double* args);

// Return non-zero if the calling thread supports Atomic.wait (For example
// if called from the main browser thread, this function will return zero
// since blocking is not allowed there).
int _emscripten_thread_supports_atomics_wait(void);
