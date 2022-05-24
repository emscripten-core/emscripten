/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE
#include "../internal/libc.h"
#include "../internal/pthread_impl.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <termios.h>
#include <threads.h>
#include <unistd.h>
#include <utime.h>

#include <emscripten.h>
#include <emscripten/proxying.h>
#include <emscripten/stack.h>
#include <emscripten/threading.h>

#include "threading_internal.h"

int emscripten_pthread_attr_gettransferredcanvases(const pthread_attr_t* a, const char** str) {
  *str = a->_a_transferredcanvases;
  return 0;
}

int emscripten_pthread_attr_settransferredcanvases(pthread_attr_t* a, const char* str) {
  a->_a_transferredcanvases = str;
  return 0;
}

int sched_get_priority_max(int policy) {
  // Web workers do not actually support prioritizing threads,
  // but mimic values that Linux apparently reports, see
  // http://man7.org/linux/man-pages/man2/sched_get_priority_min.2.html
  if (policy == SCHED_FIFO || policy == SCHED_RR)
    return 99;
  else
    return 0;
}

int sched_get_priority_min(int policy) {
  // Web workers do not actually support prioritizing threads,
  // but mimic values that Linux apparently reports, see
  // http://man7.org/linux/man-pages/man2/sched_get_priority_min.2.html
  if (policy == SCHED_FIFO || policy == SCHED_RR)
    return 1;
  else
    return 0;
}

int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *restrict attr, int *restrict prioceiling)
{
  // Not supported either in Emscripten or musl, return a faked value.
  if (prioceiling) *prioceiling = 99;
  return 0;
}

int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
  // Not supported either in Emscripten or musl, return an error.
  return EPERM;
}

static uint32_t dummyZeroAddress = 0;

void emscripten_thread_sleep(double msecs) {
  double now = emscripten_get_now();
  double target = now + msecs;

  // If we have less than this many msecs left to wait, busy spin that instead.
  double min_ms_slice_to_sleep = 0.1;

  // runtime thread may need to run proxied calls, so sleep in very small slices to be responsive.
  double max_ms_slice_to_sleep = emscripten_is_main_runtime_thread() ? 1 : 100;

  emscripten_conditional_set_current_thread_status(
    EM_THREAD_STATUS_RUNNING, EM_THREAD_STATUS_SLEEPING);

  do {
    // Keep processing the main loop of the calling thread.
    __pthread_testcancel(); // pthreads spec: sleep is a cancellation point, so must test if this
                            // thread is cancelled during the sleep.
    emscripten_current_thread_process_queued_calls();

    now = emscripten_get_now();
    double ms_to_sleep = target - now;
    if (ms_to_sleep < min_ms_slice_to_sleep)
      continue;
    if (ms_to_sleep > max_ms_slice_to_sleep)
      ms_to_sleep = max_ms_slice_to_sleep;
    emscripten_futex_wait(&dummyZeroAddress, 0, ms_to_sleep);
    now = emscripten_get_now();
  } while (now < target);

  emscripten_conditional_set_current_thread_status(
    EM_THREAD_STATUS_SLEEPING, EM_THREAD_STATUS_RUNNING);
}

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
      case EM_FUNC_SIG_PARAM_I64:
        q->args[i].i64 = va_arg(args, int64_t);
        break;
      case EM_FUNC_SIG_PARAM_F:
        q->args[i].f = (float)va_arg(args, double);
        break;
      case EM_FUNC_SIG_PARAM_D:
        q->args[i].d = va_arg(args, double);
        break;
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

extern EMSCRIPTEN_RESULT _emscripten_set_offscreencanvas_size(const char *target, int width, int height);
extern double emscripten_receive_on_main_thread_js(int functionIndex, int numCallArgs, double* args);

static void _do_call(void* arg) {
  em_queued_call* q = (em_queued_call*)arg;
  // C function pointer
  assert(EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(q->functionEnum) <= EM_QUEUED_CALL_MAX_ARGS);
  switch (q->functionEnum) {
    case EM_PROXIED_RESIZE_OFFSCREENCANVAS:
      q->returnValue.i =
        _emscripten_set_offscreencanvas_size(q->args[0].cp, q->args[1].i, q->args[2].i);
      break;
    case EM_PROXIED_JS_FUNCTION:
      q->returnValue.d =
        emscripten_receive_on_main_thread_js((int)(size_t)q->functionPtr, q->args[0].i, &q->args[1].d);
      break;
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

static struct pthread __main_pthread;

pthread_t emscripten_main_browser_thread_id() {
  return &__main_pthread;
}

static pthread_t normalize_thread(pthread_t target_thread) {
  assert(target_thread);
  if (target_thread == EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD) {
    return emscripten_main_browser_thread_id();
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

void emscripten_async_run_in_main_thread(em_queued_call* call) {
  do_dispatch_to_thread(emscripten_main_browser_thread_id(), call);
}

void emscripten_sync_run_in_main_thread(em_queued_call* call) {
  emscripten_async_run_in_main_thread(call);

  // Enter to wait for the operation to complete.
  emscripten_wait_for_call_v(call, INFINITY);
}

void* emscripten_sync_run_in_main_thread_0(int function) {
  em_queued_call q = {function};
  q.returnValue.vp = 0;
  emscripten_sync_run_in_main_thread(&q);
  return q.returnValue.vp;
}

void* emscripten_sync_run_in_main_thread_1(int function, void* arg1) {
  em_queued_call q = {function};
  q.args[0].vp = arg1;
  q.returnValue.vp = 0;
  emscripten_sync_run_in_main_thread(&q);
  return q.returnValue.vp;
}

void* emscripten_sync_run_in_main_thread_2(
  int function, void* arg1, void* arg2) {
  em_queued_call q = {function};
  q.args[0].vp = arg1;
  q.args[1].vp = arg2;
  q.returnValue.vp = 0;
  emscripten_sync_run_in_main_thread(&q);
  return q.returnValue.vp;
}

void* emscripten_sync_run_in_main_thread_3(
  int function, void* arg1, void* arg2, void* arg3) {
  em_queued_call q = {function};
  q.args[0].vp = arg1;
  q.args[1].vp = arg2;
  q.args[2].vp = arg3;
  q.returnValue.vp = 0;
  emscripten_sync_run_in_main_thread(&q);
  return q.returnValue.vp;
}

void* emscripten_sync_run_in_main_thread_4(
  int function, void* arg1, void* arg2, void* arg3, void* arg4) {
  em_queued_call q = {function};
  q.args[0].vp = arg1;
  q.args[1].vp = arg2;
  q.args[2].vp = arg3;
  q.args[3].vp = arg4;
  q.returnValue.vp = 0;
  emscripten_sync_run_in_main_thread(&q);
  return q.returnValue.vp;
}

void* emscripten_sync_run_in_main_thread_5(
  int function, void* arg1, void* arg2, void* arg3, void* arg4, void* arg5) {
  em_queued_call q = {function};
  q.args[0].vp = arg1;
  q.args[1].vp = arg2;
  q.args[2].vp = arg3;
  q.args[3].vp = arg4;
  q.args[4].vp = arg5;
  q.returnValue.vp = 0;
  emscripten_sync_run_in_main_thread(&q);
  return q.returnValue.vp;
}

void* emscripten_sync_run_in_main_thread_6(
  int function, void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6) {
  em_queued_call q = {function};
  q.args[0].vp = arg1;
  q.args[1].vp = arg2;
  q.args[2].vp = arg3;
  q.args[3].vp = arg4;
  q.args[4].vp = arg5;
  q.args[5].vp = arg6;
  q.returnValue.vp = 0;
  emscripten_sync_run_in_main_thread(&q);
  return q.returnValue.vp;
}

void* emscripten_sync_run_in_main_thread_7(int function, void* arg1,
  void* arg2, void* arg3, void* arg4, void* arg5, void* arg6, void* arg7) {
  em_queued_call q = {function};
  q.args[0].vp = arg1;
  q.args[1].vp = arg2;
  q.args[2].vp = arg3;
  q.args[3].vp = arg4;
  q.args[4].vp = arg5;
  q.args[5].vp = arg6;
  q.args[6].vp = arg7;
  q.returnValue.vp = 0;
  emscripten_sync_run_in_main_thread(&q);
  return q.returnValue.vp;
}

void emscripten_current_thread_process_queued_calls() {
  emscripten_proxy_execute_queue(emscripten_proxy_get_system_queue());
}

void emscripten_main_thread_process_queued_calls() {
  assert(emscripten_is_main_runtime_thread());
  emscripten_current_thread_process_queued_calls();
}

int emscripten_sync_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void* func_ptr, ...) {
  em_queued_call q = {sig, func_ptr};

  va_list args;
  va_start(args, func_ptr);
  init_em_queued_call_args(&q, sig, args);
  va_end(args);
  emscripten_sync_run_in_main_thread(&q);
  return q.returnValue.i;
}

double emscripten_run_in_main_runtime_thread_js(int index, int num_args, int64_t* buffer, int sync) {
  em_queued_call q;
  em_queued_call *c;
  if (sync) {
    q.operationDone = 0;
    q.satelliteData = 0;
    c = &q;
  } else {
    c = em_queued_call_malloc();
  }
  c->calleeDelete = 1-sync;
  c->functionEnum = EM_PROXIED_JS_FUNCTION;
  // Index not needed to ever be more than 32-bit.
  c->functionPtr = (void*)(size_t)index;
  assert(num_args+1 <= EM_QUEUED_JS_CALL_MAX_ARGS);
  // The types are only known at runtime in these calls, so we store values that
  // must be able to contain any valid JS value, including a 64-bit BigInt if
  // BigInt support is enabled. We store to an i64, which can contain both a
  // BigInt and a JS Number which is a 64-bit double.
  c->args[0].i = num_args;
  for (int i = 0; i < num_args; i++) {
    c->args[i+1].i64 = buffer[i];
  }

  if (sync) {
    emscripten_sync_run_in_main_thread(&q);
    // TODO: support BigInt return values somehow.
    return q.returnValue.d;
  } else {
    // 'async' runs are fire and forget, where the caller detaches itself from the call object after
    // returning here, and it is the callee's responsibility to free up the memory after the call
    // has been performed.
    emscripten_async_run_in_main_thread(c);
    return 0;
  }
}

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

int _emscripten_thread_is_valid(pthread_t thread) {
  return thread->self == thread;
}

static void *dummy_tsd[1] = { 0 };
weak_alias(dummy_tsd, __pthread_tsd_main);

// See system/lib/README.md for static constructor ordering.
__attribute__((constructor(48)))
void __emscripten_init_main_thread(void) {
  __emscripten_init_main_thread_js(&__main_pthread);

  // The pthread struct has a field that points to itself - this is used as
  // a magic ID to detect whether the pthread_t structure is 'alive'.
  __main_pthread.self = &__main_pthread;
  __main_pthread.stack = (void*)emscripten_stack_get_base();
  __main_pthread.stack_size = emscripten_stack_get_base() - emscripten_stack_get_end();
  __main_pthread.detach_state = DT_JOINABLE;
  // pthread struct robust_list head should point to itself.
  __main_pthread.robust_list.head = &__main_pthread.robust_list.head;
  // Main thread ID is always 1.  It can't be 0 because musl assumes
  // tid is always non-zero.
  __main_pthread.tid = getpid();
  __main_pthread.locale = &libc.global_locale;
  // TODO(sbc): Implement circular list of threads
  //__main_pthread.next = __main_pthread.prev = &__main_pthread;
  __main_pthread.tsd = (void **)__pthread_tsd_main;
}
