/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE
#include "pthread_impl.h"
#include "assert.h"
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <threads.h>
#include <emscripten/emmalloc.h>

// See musl's pthread_create.c

extern int __cxa_thread_atexit(void (*)(void *), void *, void *);
extern int __pthread_create_js(struct pthread *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
extern void _emscripten_thread_init(int, int, int);
extern void __pthread_exit_run_handlers();
extern void __pthread_detached_exit();
extern int8_t __dso_handle;

static void dummy_0()
{
}
weak_alias(dummy_0, __pthread_tsd_run_dtors);

void __run_cleanup_handlers(void* _unused) {
  pthread_t self = __pthread_self();
  while (self->cancelbuf) {
    void (*f)(void *) = self->cancelbuf->__f;
    void *x = self->cancelbuf->__x;
    self->cancelbuf = self->cancelbuf->__next;
    f(x);
  }
}

void __do_cleanup_push(struct __ptcb *cb) {
  struct pthread *self = __pthread_self();
  cb->__next = self->cancelbuf;
  self->cancelbuf = cb;
  static thread_local bool registered = false;
  if (!registered) {
    __cxa_thread_atexit(__run_cleanup_handlers, NULL, &__dso_handle);
    registered = true;
  }
}

void __do_cleanup_pop(struct __ptcb *cb) {
  __pthread_self()->cancelbuf = cb->__next;
}

int __pthread_create(pthread_t *restrict res, const pthread_attr_t *restrict attrp, void *(*entry)(void *), void *restrict arg) {
  // Note on LSAN: lsan intercepts/wraps calls to pthread_create so any
  // allocation we we do here should be considered leaks.
  // See: lsan_interceptors.cpp.
  if (!res) {
    return EINVAL;
  }

  // Allocate thread block (pthread_t structure).
  struct pthread *new = malloc(sizeof(struct pthread));
  // zero-initialize thread structure.
  memset(new, 0, sizeof(struct pthread));

  // The pthread struct has a field that points to itself - this is used as a
  // magic ID to detect whether the pthread_t structure is 'alive'.
  new->self = new;
  new->tid = (uintptr_t)new;

  // pthread struct robust_list head should point to itself.
  new->robust_list.head = &new->robust_list.head;

  new->locale = &libc.global_locale;

  // Allocate memory for thread-local storage and initialize it to zero.
  new->tsd = malloc(PTHREAD_KEYS_MAX * sizeof(void*));
  memset(new->tsd, 0, PTHREAD_KEYS_MAX * sizeof(void*));

  *res = new;
  return __pthread_create_js(new, attrp, entry, arg);
}

void _emscripten_thread_exit(void* result) {
  struct pthread *self = __pthread_self();
  assert(self);

  self->canceldisable = PTHREAD_CANCEL_DISABLE;
  self->cancelasync = PTHREAD_CANCEL_DEFERRED;
  self->result = result;

  __pthread_exit_run_handlers();

  // Call into the musl function that runs destructors of all thread-specific data.
  __pthread_tsd_run_dtors();

  __lock(self->exitlock);

  if (self == emscripten_main_browser_thread_id()) {
    // FIXME(sbc): When pthread_exit causes the entire application to exit
    // we should be returning zero (according to the man page for pthread_exit).
    exit((intptr_t)result);
    return;
  }

  // We have the call the buildin free here since lsan handling for this thread
  // gets shut down during __pthread_tsd_run_dtors.
  emscripten_builtin_free(self->tsd);
  self->tsd = NULL;

  // Not hosting a pthread anymore in this worker set __pthread_self to NULL
  _emscripten_thread_init(0, 0, 0);

  // Cache deteched state since once we set threadStatus to 1, the `self` struct
  // could be freed and reused.
  int detatched = self->detached;

  // Mark the thread as no longer running so it can be joined.
  // Once we publish this, any threads that are waiting to join with us can
  // proceed and this worker can be recycled and used on another thread.
  self->threadStatus = 1;
  emscripten_futex_wake(&self->threadStatus, INT_MAX); // wake all threads

  if (detatched) {
    __pthread_detached_exit();
  }
}

// Mark as `no_sanitize("address"` since emscripten_pthread_exit destroys
// the current thread and runs its exit handlers.  Without this asan injects
// a call to __asan_handle_no_return before emscripten_unwind_to_js_event_loop
// which seem to cause a crash later down the line.
__attribute__((no_sanitize("address")))
_Noreturn void __pthread_exit(void* retval) {
  _emscripten_thread_exit(retval);
  emscripten_unwind_to_js_event_loop();
}

weak_alias(__pthread_create, emscripten_builtin_pthread_create);
weak_alias(__pthread_create, pthread_create);
weak_alias(__pthread_exit, pthread_exit);
