/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <assert.h>
#include <math.h>
#include <stdatomic.h>

#include <emscripten/threading.h>
#include <emscripten/console.h>

#include "../internal/pthread_impl.h"

#include "threading_internal.h"
#include "emscripten_internal.h"

typedef union em_variant_val {
  int i;
  int64_t j;
  float f;
  double d;
  void *vp;
  char *cp;
} em_variant_val;

typedef struct em_queued_call {
  int functionEnum;
  void *functionPtr;
  _Atomic uint32_t operationDone;
  em_variant_val args[EM_QUEUED_JS_CALL_MAX_ARGS];
  em_variant_val returnValue;

  // Sets the PThread.currentProxiedOperationCallerThread global for the
  // duration of the proxied call.
  pthread_t callingThread;

  // An optional pointer to a secondary data block that should be free()d when
  // this queued call is freed.
  void *satelliteData;

  // If true, the caller has "detached" itself from this call object and the
  // Emscripten main runtime thread should free up this em_queued_call object
  // after it has been executed. If false, the caller is in control of the
  // memory.
  int calleeDelete;
} em_queued_call;

// Proxied C/C++ functions support at most this many arguments. Dispatch is
// static/strongly typed by signature.
#define EM_QUEUED_CALL_MAX_ARGS 11

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

#ifdef __wasm64__
typedef int (*em_func_ij)(uint64_t);
typedef uint64_t (*em_func_ji)(int);
typedef int (*em_func_ijj)(uint64_t, uint64_t);
typedef int (*em_func_iij)(int, uint64_t);
typedef int (*em_func_iijj)(int, uint64_t, uint64_t);
typedef uint64_t (*em_func_jjj)(uint64_t, uint64_t);
typedef void (*em_func_vij)(int, uint64_t);
typedef void (*em_func_viij)(int, int, uint64_t);
typedef void (*em_func_viji)(int, uint64_t, int);
typedef void (*em_func_vijji)(int, uint64_t, uint64_t, int);
typedef void (*em_func_viijj)(int, int, uint64_t, uint64_t);
typedef void (*em_func_viiij)(int, int, int, uint64_t);
typedef void (*em_func_viiiiij)(int, int, int, int, int, uint64_t);
typedef void (*em_func_viiiiiij)(int, int, int, int, int, int, uint64_t);
typedef void (*em_func_viiiiiiiij)(int, int, int, int, int, int, int, int, uint64_t);
#endif

// Allocator and deallocator for em_queued_call objects.
static em_queued_call* em_queued_call_malloc() {
  em_queued_call* call = (em_queued_call*)malloc(sizeof(em_queued_call));
  assert(call); // Not a programming error, but use assert() in debug builds to catch OOM scenarios.
  if (call) {
    call->operationDone = 0;
    call->functionPtr = 0;
    call->satelliteData = 0;
  }
  return call;
}

static void em_queued_call_free(em_queued_call* call) {
  if (call)
    free(call->satelliteData);
  free(call);
}

static void init_em_queued_call_args(em_queued_call* q,
                                     EM_FUNC_SIGNATURE sig,
                                     va_list args) {
  EM_FUNC_SIGNATURE argumentsType = sig & EM_FUNC_SIG_ARGUMENTS_TYPE_MASK;
  int numArguments = EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(sig);
  for (int i = 0; i < numArguments; ++i) {
    switch ((argumentsType & EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_MASK)) {
      case EM_FUNC_SIG_PARAM_I:
        q->args[i].i = va_arg(args, int);
        break;
      case EM_FUNC_SIG_PARAM_J:
        q->args[i].j = va_arg(args, int64_t);
        break;
      case EM_FUNC_SIG_PARAM_F:
        q->args[i].f = (float)va_arg(args, double);
        break;
      case EM_FUNC_SIG_PARAM_D:
        q->args[i].d = va_arg(args, double);
        break;
      default:
        assert(false && "unknown proxy param type");
    }
    argumentsType >>= EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_SHIFT;
  }
}

static em_queued_call* em_queued_call_create(EM_FUNC_SIGNATURE sig,
                                             void* func,
                                             void* satellite,
                                             va_list args) {
  em_queued_call* call = em_queued_call_malloc();
  if (call) {
    call->functionEnum = sig;
    call->functionPtr = func;
    call->satelliteData = satellite;
    init_em_queued_call_args(call, sig, args);
  }
  return call;
}

void emscripten_async_waitable_close(em_queued_call* call) {
  assert(call->operationDone);
  em_queued_call_free(call);
}

static void _do_call(void* arg) {
  em_queued_call* q = (em_queued_call*)arg;
  // C function pointer
  assert(EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(q->functionEnum) <= EM_QUEUED_CALL_MAX_ARGS);
  switch (q->functionEnum) {
    case EM_FUNC_SIG_V:
      ((em_func_v)q->functionPtr)();
      break;
    case EM_FUNC_SIG_VI:
      ((em_func_vi)q->functionPtr)(q->args[0].i);
      break;
    case EM_FUNC_SIG_VF:
      ((em_func_vf)q->functionPtr)(q->args[0].f);
      break;
    case EM_FUNC_SIG_VII:
      ((em_func_vii)q->functionPtr)(q->args[0].i, q->args[1].i);
      break;
    case EM_FUNC_SIG_VIF:
      ((em_func_vif)q->functionPtr)(q->args[0].i, q->args[1].f);
      break;
    case EM_FUNC_SIG_VFF:
      ((em_func_vff)q->functionPtr)(q->args[0].f, q->args[1].f);
      break;
    case EM_FUNC_SIG_VIII:
      ((em_func_viii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i);
      break;
    case EM_FUNC_SIG_VIIF:
      ((em_func_viif)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].f);
      break;
    case EM_FUNC_SIG_VIFF:
      ((em_func_viff)q->functionPtr)(q->args[0].i, q->args[1].f, q->args[2].f);
      break;
    case EM_FUNC_SIG_VFFF:
      ((em_func_vfff)q->functionPtr)(q->args[0].f, q->args[1].f, q->args[2].f);
      break;
    case EM_FUNC_SIG_VIIII:
      ((em_func_viiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i);
      break;
    case EM_FUNC_SIG_VIIFI:
      ((em_func_viifi)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].f, q->args[3].i);
      break;
    case EM_FUNC_SIG_VIFFF:
      ((em_func_vifff)q->functionPtr)(q->args[0].i, q->args[1].f, q->args[2].f, q->args[3].f);
      break;
    case EM_FUNC_SIG_VFFFF:
      ((em_func_vffff)q->functionPtr)(q->args[0].f, q->args[1].f, q->args[2].f, q->args[3].f);
      break;
    case EM_FUNC_SIG_VIIIII:
      ((em_func_viiiii)q->functionPtr)(
        q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i);
      break;
    case EM_FUNC_SIG_VIFFFF:
      ((em_func_viffff)q->functionPtr)(
        q->args[0].i, q->args[1].f, q->args[2].f, q->args[3].f, q->args[4].f);
      break;
    case EM_FUNC_SIG_VIIIIII:
      ((em_func_viiiiii)q->functionPtr)(
        q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i);
      break;
    case EM_FUNC_SIG_VIIIIIII:
      ((em_func_viiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i,
        q->args[4].i, q->args[5].i, q->args[6].i);
      break;
    case EM_FUNC_SIG_VIIIIIIII:
      ((em_func_viiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i,
        q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i);
      break;
    case EM_FUNC_SIG_VIIIIIIIII:
      ((em_func_viiiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i,
        q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i, q->args[8].i);
      break;
    case EM_FUNC_SIG_VIIIIIIIIII:
      ((em_func_viiiiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i,
        q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i, q->args[8].i,
        q->args[9].i);
      break;
    case EM_FUNC_SIG_VIIIIIIIIIII:
      ((em_func_viiiiiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i,
        q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i, q->args[8].i,
        q->args[9].i, q->args[10].i);
      break;
#ifdef __wasm64__
    case EM_FUNC_SIG_IP:
      q->returnValue.i = ((em_func_ij)q->functionPtr)(q->args[0].j);
      break;
    case EM_FUNC_SIG_IIPP:
      q->returnValue.i = ((em_func_iijj)q->functionPtr)(q->args[0].i, q->args[1].j, q->args[2].j);
      break;
    case EM_FUNC_SIG_PI:
      q->returnValue.j = ((em_func_ji)q->functionPtr)(q->args[0].i);
      break;
    case EM_FUNC_SIG_IIP:
      q->returnValue.i = ((em_func_iij)q->functionPtr)(q->args[0].i, q->args[1].j);
      break;
    case EM_FUNC_SIG_PPP:
      q->returnValue.j = ((em_func_jjj)q->functionPtr)(q->args[0].j, q->args[1].j);
      break;
    case EM_FUNC_SIG_VIP:
      ((em_func_vij)q->functionPtr)(q->args[0].i, q->args[1].j);
      break;
    case EM_FUNC_SIG_VIIP:
      ((em_func_viij)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].j);
      break;
    case EM_FUNC_SIG_VIIPP:
      ((em_func_viijj)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].j, q->args[3].j);
      break;
    case EM_FUNC_SIG_VIIIP:
      ((em_func_viiij)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].j);
      break;
    case EM_FUNC_SIG_VIPPI:
      ((em_func_vijji)q->functionPtr)(q->args[0].i, q->args[1].j, q->args[2].j, q->args[3].i);
      break;
    case EM_FUNC_SIG_VIPI:
      ((em_func_viji)q->functionPtr)(q->args[0].i, q->args[1].j, q->args[3].i);
      break;
    case EM_FUNC_SIG_VIIIIIP:
      ((em_func_viiiiij)q->functionPtr)(
        q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].j);
      break;
    case EM_FUNC_SIG_VIIIIIIP:
      ((em_func_viiiiiij)q->functionPtr)(
        q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].j);
      break;
    case EM_FUNC_SIG_VIIIIIIIIP:
      ((em_func_viiiiiiiij)q->functionPtr)(
        q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i, q->args[8].j);
      break;
#endif
    case EM_FUNC_SIG_I:
      q->returnValue.i = ((em_func_i)q->functionPtr)();
      break;
    case EM_FUNC_SIG_II:
      q->returnValue.i = ((em_func_ii)q->functionPtr)(q->args[0].i);
      break;
    case EM_FUNC_SIG_III:
      q->returnValue.i = ((em_func_iii)q->functionPtr)(q->args[0].i, q->args[1].i);
      break;
    case EM_FUNC_SIG_IIII:
      q->returnValue.i = ((em_func_iiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i);
      break;
    case EM_FUNC_SIG_IIIII:
      q->returnValue.i =
        ((em_func_iiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i);
      break;
    case EM_FUNC_SIG_IIIIII:
      q->returnValue.i = ((em_func_iiiiii)q->functionPtr)(
        q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i);
      break;
    case EM_FUNC_SIG_IIIIIII:
      q->returnValue.i = ((em_func_iiiiiii)q->functionPtr)(
        q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i);
      break;
    case EM_FUNC_SIG_IIIIIIII:
      q->returnValue.i = ((em_func_iiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i,
        q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i);
      break;
    case EM_FUNC_SIG_IIIIIIIII:
      q->returnValue.i = ((em_func_iiiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i,
        q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i);
      break;
    case EM_FUNC_SIG_IIIIIIIIII:
      q->returnValue.i =
        ((em_func_iiiiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i,
          q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i, q->args[8].i);
      break;
    default:
      assert(0 && "Invalid Emscripten pthread _do_call opcode!");
  }

  // If the caller is detached from this operation, it is the main thread's responsibility to free
  // up the call object.
  if (q->calleeDelete) {
    em_queued_call_free(q);
    // No need to wake a listener, nothing is listening to this since the call object is detached.
  } else {
    // The caller owns this call object, it is listening to it and will free it up.
    q->operationDone = 1;
    emscripten_futex_wake(&q->operationDone, INT_MAX);
  }
}

static pthread_t normalize_thread(pthread_t target_thread) {
  assert(target_thread);
  if (target_thread == EM_CALLBACK_THREAD_CONTEXT_MAIN_RUNTIME_THREAD) {
    return emscripten_main_runtime_thread_id();
  }
  if (target_thread == EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD) {
    return pthread_self();
  }
  return target_thread;
}

// Execute `call` and return 1 only if already on the `target_thread`. Otherwise
// return 0.
static int maybe_call_on_current_thread(pthread_t target_thread,
                                        em_queued_call* call) {
  if (pthread_equal(target_thread, pthread_self())) {
    _do_call(call);
    return 1;
  }
  return 0;
}

// Execute or proxy `call`. Return 1 if the work was executed or otherwise
// return 0.
static int do_dispatch_to_thread(pthread_t target_thread,
                                 em_queued_call* call) {
  target_thread = normalize_thread(target_thread);
  if (maybe_call_on_current_thread(target_thread, call)) {
    return 1;
  }
  emscripten_proxy_async(
    emscripten_proxy_get_system_queue(), target_thread, _do_call, call);
  return 0;
}

int emscripten_dispatch_to_thread_args(pthread_t target_thread,
                                       EM_FUNC_SIGNATURE sig,
                                       void* func_ptr,
                                       void* satellite,
                                       va_list args) {
  em_queued_call* q = em_queued_call_create(sig, func_ptr, satellite, args);
  assert(q);
  // TODO: handle errors in a better way, this pattern appears in several places
  //       in this file. The current behavior makes the calling thread hang as
  //       it waits (for synchronous calls).
  // If we failed to allocate, return 0 which means we did not execute anything
  // (we also never will in that case).
  if (!q)
    return 0;

  // `q` will not be used after it is called, so let the call clean it up.
  q->calleeDelete = 1;
  return do_dispatch_to_thread(target_thread, q);
}

void emscripten_async_run_in_main_thread(em_queued_call* call) {
  do_dispatch_to_thread(emscripten_main_runtime_thread_id(), call);
}

int emscripten_dispatch_to_thread_(pthread_t target_thread,
                                   EM_FUNC_SIGNATURE sig,
                                   void* func_ptr,
                                   void* satellite,
                                   ...) {
  va_list args;
  va_start(args, satellite);
  int ret = emscripten_dispatch_to_thread_args(
    target_thread, sig, func_ptr, satellite, args);
  va_end(args);
  return ret;
}

int emscripten_dispatch_to_thread_async_args(pthread_t target_thread,
                                             EM_FUNC_SIGNATURE sig,
                                             void* func_ptr,
                                             void* satellite,
                                             va_list args) {
  // Check if we are already on the target thread.
  if (pthread_equal(target_thread, pthread_self())) {
    // Setup is the same as in emscripten_dispatch_to_thread_args.
    em_queued_call* q = em_queued_call_create(sig, func_ptr, satellite, args);
    assert(q);
    if (!q)
      return 0;
    q->calleeDelete = 1;

    // Schedule the call to run later on this thread.
    emscripten_set_timeout(_do_call, 0, q);
    return 0;
  }

  // Otherwise, dispatch as usual.
  return emscripten_dispatch_to_thread_args(
    target_thread, sig, func_ptr, satellite, args);
}

int emscripten_dispatch_to_thread_async_(pthread_t target_thread,
                                         EM_FUNC_SIGNATURE sig,
                                         void* func_ptr,
                                         void* satellite,
                                         ...) {
  va_list args;
  va_start(args, satellite);
  int ret = emscripten_dispatch_to_thread_async_args(
    target_thread, sig, func_ptr, satellite, args);
  va_end(args);
  return ret;
}

static void sync_run_in_main_thread(em_queued_call* call) {
  emscripten_async_run_in_main_thread(call);

  // Enter to wait for the operation to complete.
  emscripten_wait_for_call_v(call, INFINITY);
}

int emscripten_sync_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void* func_ptr, ...) {
  em_queued_call q = {sig, func_ptr};

  va_list args;
  va_start(args, func_ptr);
  init_em_queued_call_args(&q, sig, args);
  va_end(args);
  sync_run_in_main_thread(&q);
  return q.returnValue.i;
}

#ifdef __wasm64__
void* emscripten_sync_run_in_main_runtime_thread_ptr_(EM_FUNC_SIGNATURE sig, void* func_ptr, ...) {
  em_queued_call q = {sig, func_ptr};

  va_list args;
  va_start(args, func_ptr);
  init_em_queued_call_args(&q, sig, args);
  va_end(args);
  sync_run_in_main_thread(&q);
  emscripten_outf("sync_run_in_main_ptr: %p\n", q.returnValue.vp);
  return q.returnValue.vp;
}
#endif

void emscripten_async_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void* func_ptr, ...) {
  em_queued_call* q = em_queued_call_malloc();
  if (!q)
    return;
  q->functionEnum = sig;
  q->functionPtr = func_ptr;

  va_list args;
  va_start(args, func_ptr);
  init_em_queued_call_args(q, sig, args);
  va_end(args);
  // 'async' runs are fire and forget, where the caller detaches itself from the call object after
  // returning here, and it is the callee's responsibility to free up the memory after the call has
  // been performed.
  q->calleeDelete = 1;
  emscripten_async_run_in_main_thread(q);
}

em_queued_call* emscripten_async_waitable_run_in_main_runtime_thread_(
  EM_FUNC_SIGNATURE sig, void* func_ptr, ...) {
  em_queued_call* q = em_queued_call_malloc();
  if (!q)
    return NULL;
  q->functionEnum = sig;
  q->functionPtr = func_ptr;

  va_list args;
  va_start(args, func_ptr);
  init_em_queued_call_args(q, sig, args);
  va_end(args);
  // 'async waitable' runs are waited on by the caller, so the call object needs to remain alive for
  // the caller to access it after the operation is done. The caller is responsible in cleaning up
  // the object after done.
  q->calleeDelete = 0;
  emscripten_async_run_in_main_thread(q);
  return q;
}

EMSCRIPTEN_RESULT emscripten_wait_for_call_v(em_queued_call* call, double timeoutMSecs) {
  int r;

  int done = atomic_load(&call->operationDone);
  if (!done) {
    double now = emscripten_get_now();
    double waitEndTime = now + timeoutMSecs;
    emscripten_set_current_thread_status(EM_THREAD_STATUS_WAITPROXY);
    while (!done && now < waitEndTime) {
      r = emscripten_futex_wait(&call->operationDone, 0, waitEndTime - now);
      done = atomic_load(&call->operationDone);
      now = emscripten_get_now();
    }
    emscripten_set_current_thread_status(EM_THREAD_STATUS_RUNNING);
  }
  if (done)
    return EMSCRIPTEN_RESULT_SUCCESS;
  else
    return EMSCRIPTEN_RESULT_TIMED_OUT;
}

EMSCRIPTEN_RESULT emscripten_wait_for_call_i(
  em_queued_call* call, double timeoutMSecs, int* outResult) {
  EMSCRIPTEN_RESULT res = emscripten_wait_for_call_v(call, timeoutMSecs);
  if (res == EMSCRIPTEN_RESULT_SUCCESS && outResult)
    *outResult = call->returnValue.i;
  return res;
}
