/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "libc.h"
#include "pthread_impl.h"
#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <threads.h>
#include <unistd.h>

#include <emscripten.h>
#include <emscripten/proxying.h>
#include <emscripten/stack.h>
#include <emscripten/threading.h>

#include "threading_internal.h"
#include "emscripten_internal.h"

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

int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *restrict attr, int *restrict prioceiling) {
  // Not supported either in Emscripten or musl, return a faked value.
  if (prioceiling) *prioceiling = 99;
  return 0;
}

int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling) {
  // Not supported either in Emscripten or musl, return an error.
  return EPERM;
}

void emscripten_thread_sleep(double msecs) {
  // We include emscripten_current_thread_process_queued_calls before and
  // after sleeping since that is how we recieve "async" signals.
  // We include __pthread_testcancel here because clock_nanosleep is
  // a pthread cancelation point.
  emscripten_current_thread_process_queued_calls();
  __pthread_testcancel();
  if (msecs > 0) {
    uint32_t dummyZeroAddress = 0;
    double target = emscripten_get_now() + msecs;
    do {
      emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_RUNNING,
                                                       EM_THREAD_STATUS_SLEEPING);
      emscripten_futex_wait(&dummyZeroAddress, 0, msecs);
      emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_SLEEPING,
                                                       EM_THREAD_STATUS_RUNNING);
      emscripten_current_thread_process_queued_calls();
      __pthread_testcancel();
      msecs = target - emscripten_get_now();
    } while (msecs > 0);
  }
}

static struct pthread __main_pthread;

pthread_t emscripten_main_runtime_thread_id() {
  return &__main_pthread;
}

void emscripten_current_thread_process_queued_calls() {
  emscripten_proxy_execute_queue(emscripten_proxy_get_system_queue());
}

void emscripten_main_thread_process_queued_calls() {
  assert(emscripten_is_main_runtime_thread());
  emscripten_current_thread_process_queued_calls();
}

int _emscripten_thread_is_valid(pthread_t thread) {
  return thread->self == thread;
}

static void *dummy_tsd[1] = { 0 };
weak_alias(dummy_tsd, __pthread_tsd_main);

// See system/lib/README.md for static constructor ordering.
__attribute__((constructor(48)))
void _emscripten_init_main_thread(void) {
  // The pthread struct has a field that points to itself - this is used as
  // a magic ID to detect whether the pthread_t structure is 'alive'.
  __main_pthread.self = &__main_pthread;
  __main_pthread.detach_state = DT_JOINABLE;
  // pthread struct robust_list head should point to itself.
  __main_pthread.robust_list.head = &__main_pthread.robust_list.head;
  // Main thread ID is always 1.  It can't be 0 because musl assumes
  // tid is always non-zero.
  __main_pthread.tid = getpid();
  // pthread struct prev and next should initially point to itself (see __init_tp),
  // this is used by pthread_key_delete for deleting thread-specific data.
  __main_pthread.next = __main_pthread.prev = &__main_pthread;
  // Initialize thread-specific data area.
  __main_pthread.tsd = (void **)__pthread_tsd_main;

  _emscripten_init_main_thread_js(&__main_pthread);

  __main_pthread.stack = (void*)emscripten_stack_get_base();
  __main_pthread.stack_size = emscripten_stack_get_base() - emscripten_stack_get_end();

  _emscripten_thread_mailbox_init(&__main_pthread);
  _emscripten_thread_mailbox_await(&__main_pthread);
}
