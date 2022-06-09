/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

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
typedef struct em_queued_call
{
  int functionEnum;
  void *functionPtr;
  _Atomic uint32_t operationDone;
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

void __emscripten_init_main_thread_js(void* tb);
void _emscripten_thread_profiler_enable();
void __emscripten_thread_cleanup(pthread_t thread);

hidden void* _emscripten_tls_init(void);
hidden void _emscripten_tls_free(void);

// Checks certain structural invariants.  This allows us to detect when
// already-freed threads are used in some APIs.  Technically this is undefined
// behaviour, but we have a couple of places where we add these checks so that
// we can pass more of the posixtest suite that vanilla musl.
int _emscripten_thread_is_valid(pthread_t thread);

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
