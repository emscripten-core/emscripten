/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE
#include "pthread_impl.h"
#include "stdio_impl.h"
#include "assert.h"
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>
// Included for emscripten_builtin_free / emscripten_builtin_malloc
// TODO(sbc): Should these be in their own header to avoid emmalloc here?
#include <emscripten/emmalloc.h>

#define STACK_ALIGN 16

// See musl's pthread_create.c

extern int __pthread_create_js(struct pthread *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
extern void _emscripten_thread_init(int, int, int, int);
extern int _emscripten_default_pthread_stack_size();
extern void __pthread_detached_exit();
extern void* _emscripten_tls_base();
extern int8_t __dso_handle;

static void dummy_0()
{
}
weak_alias(dummy_0, __pthread_tsd_run_dtors);

static void __run_cleanup_handlers() {
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
}

void __do_cleanup_pop(struct __ptcb *cb) {
  __pthread_self()->cancelbuf = cb->__next;
}

static FILE *volatile dummy_file = 0;
weak_alias(dummy_file, __stdin_used);
weak_alias(dummy_file, __stdout_used);
weak_alias(dummy_file, __stderr_used);

static void init_file_lock(FILE *f) {
  if (f && f->lock<0) f->lock = 0;
}

static pid_t next_tid = 0;

// In case the stub syscall is not linked it
static long dummy_getpid() {
  return 42;
}
weak_alias(dummy_getpid, __syscall_getpid);

int __pthread_create(pthread_t* restrict res,
                     const pthread_attr_t* restrict attrp,
                     void* (*entry)(void*),
                     void* restrict arg) {
  // Note on LSAN: lsan intercepts/wraps calls to pthread_create so any
  // allocation we we do here should be considered leaks.
  // See: lsan_interceptors.cpp.
  if (!res) {
    return EINVAL;
  }

  // Create threads with monotonically increasing TID starting with the main
  // thread which has TID == PID.
  if (!next_tid) {
    next_tid = getpid() + 1;
  }

  if (!libc.threaded) {
    for (FILE *f=*__ofl_lock(); f; f=f->next)
      init_file_lock(f);
    __ofl_unlock();
    init_file_lock(__stdin_used);
    init_file_lock(__stdout_used);
    init_file_lock(__stderr_used);
    libc.threaded = 1;
  }

  struct pthread *self = __pthread_self();

  // Allocate thread block (pthread_t structure).
  struct pthread *new = malloc(sizeof(struct pthread));
  // zero-initialize thread structure.
  memset(new, 0, sizeof(struct pthread));

  // The pthread struct has a field that points to itself - this is used as a
  // magic ID to detect whether the pthread_t structure is 'alive'.
  new->self = new;
  new->tid = next_tid++;

  // pthread struct robust_list head should point to itself.
  new->robust_list.head = &new->robust_list.head;

  new->locale = &libc.global_locale;

  // Allocate memory for thread-local storage and initialize it to zero.
  new->tsd = malloc(PTHREAD_KEYS_MAX * sizeof(void*));
  memset(new->tsd, 0, PTHREAD_KEYS_MAX * sizeof(void*));

  new->detach_state = DT_JOINABLE;

  if (attrp && attrp != __ATTRP_C11_THREAD) {
    if (attrp->_a_detach) {
      new->detach_state = DT_DETACHED;
    }
    new->stack_size = attrp->_a_stacksize;
    new->stack = (void*)attrp->_a_stackaddr;
  } else {
    new->stack_size = _emscripten_default_pthread_stack_size();
  }

  if (!new->stack) {
    char* stackBase = memalign(STACK_ALIGN, new->stack_size);
    // musl stores top of the stack in pthread_t->stack (i.e. the high
    // end from which it grows down).
    new->stack = stackBase + new->stack_size;
    new->stack_owned = 1;
  }

  //printf("start __pthread_create: %p\n", self);
  int rtn = __pthread_create_js(new, attrp, entry, arg);
  if (rtn != 0)
    return rtn;

  // TODO(sbc): Implement circular list of threads
  /*
  __tl_lock();

  new->next = self->next;
  new->prev = self;
  new->next->prev = new;
  new->prev->next = new;

  __tl_unlock();
  */

  *res = new;
  //printf("done __pthread_create self=%p next=%p prev=%p new=%p\n", self, self->next, self->prev, new);
  return 0;
}

/*
 * Called from JS main thread to free data accociated a thread
 * that is no longer running.
 */
void _emscripten_thread_free_data(pthread_t t) {
  if (t->profilerBlock) {
    emscripten_builtin_free(t->profilerBlock);
  }
  if (t->stack_owned) {
    emscripten_builtin_free(((char*)t->stack) - t->stack_size);
  }
  // To aid in debugging set all fields to zero
  memset(t, 0, sizeof(*t));
  emscripten_builtin_free(t);
}

static void free_tls_data() {
  void* tls_block = _emscripten_tls_base();
  if (tls_block) {
#ifdef DEBUG_TLS
    printf("tls free: thread[%p] dso[%p] <- %p\n", pthread_self(), &__dso_handle, tls_block);
#endif
    emscripten_builtin_free(tls_block);
  }
}

void _emscripten_thread_exit(void* result) {
  struct pthread *self = __pthread_self();
  assert(self);

  self->canceldisable = PTHREAD_CANCEL_DISABLE;
  self->cancelasync = PTHREAD_CANCEL_DEFERRED;
  self->result = result;

  // Run any handlers registered with pthread_cleanup_push
  __run_cleanup_handlers();

  // Call into the musl function that runs destructors of all thread-specific data.
  __pthread_tsd_run_dtors();

  free_tls_data();

  // TODO(sbc): Implement circular list of threads
  /*
  __tl_lock();

  self->next->prev = self->prev;
  self->prev->next = self->next;
  self->prev = self->next = self;

  __tl_unlock();
  */

  if (self == emscripten_main_browser_thread_id()) {
    exit(0);
    return;
  }

  // We have the call the buildin free here since lsan handling for this thread
  // gets shut down during __pthread_tsd_run_dtors.
  emscripten_builtin_free(self->tsd);
  self->tsd = NULL;

  // Not hosting a pthread anymore in this worker set __pthread_self to NULL
  _emscripten_thread_init(0, 0, 0, 1);

  /* This atomic potentially competes with a concurrent pthread_detach
   * call; the loser is responsible for freeing thread resources. */
  int state = a_cas(&self->detach_state, DT_JOINABLE, DT_EXITING);

  // Mark the thread as no longer running.
  // When we publish this, the main thread is free to deallocate the thread
  // object and we are done.
  if (state == DT_DETACHED) {
    self->detach_state = DT_EXITED;
    __pthread_detached_exit();
  } else {
    self->detach_state = DT_EXITING;
    // wake any threads that might be waiting for us to exit
    emscripten_futex_wake(&self->detach_state, INT_MAX);
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
