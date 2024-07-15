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
#include <emscripten/heap.h>
#include <emscripten/threading.h>

#define STACK_ALIGN __BIGGEST_ALIGNMENT__
#define TSD_ALIGN (sizeof(void*))

// Comment this line to enable tracing of thread creation and destruction:
// #define PTHREAD_DEBUG
#ifdef PTHREAD_DEBUG
#define dbg(fmt, ...) emscripten_dbgf(fmt, ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif

// See musl's pthread_create.c

static void dummy_0() {}
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
static int dummy_getpid(void) {
  return 42;
}
weak_alias(dummy_getpid, __syscall_getpid);

static int tl_lock_count;
static int tl_lock_waiters;

volatile int __thread_list_lock;

void __tl_lock(void) {
  int tid = __pthread_self()->tid;
  int val = __thread_list_lock;
  if (val == tid) {
    tl_lock_count++;
    return;
  }
  while ((val = a_cas(&__thread_list_lock, 0, tid)))
    __wait(&__thread_list_lock, &tl_lock_waiters, val, 0);
}

void __tl_unlock(void) {
  if (tl_lock_count) {
    tl_lock_count--;
    return;
  }
  a_store(&__thread_list_lock, 0);
  if (tl_lock_waiters) __wake(&__thread_list_lock, 1, 0);
}

void __tl_sync(pthread_t td)
{
	a_barrier();
	int val = __thread_list_lock;
	if (!val) return;
	__wait(&__thread_list_lock, &tl_lock_waiters, val, 0);
	if (tl_lock_waiters) __wake(&__thread_list_lock, 1, 0);
}

/* pthread_key_create.c overrides this */
static volatile size_t dummy = 0;
weak_alias(dummy, __pthread_tsd_size);

#define ROUND_UP(x, ALIGNMENT) (((x)+ALIGNMENT-1)&-ALIGNMENT)

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

  pthread_attr_t attr = { 0 };
  if (attrp && attrp != __ATTRP_C11_THREAD) attr = *attrp;
  if (!attr._a_stacksize) {
    attr._a_stacksize = __default_stacksize;
  }

  // Allocate memory for new thread.  The layout of the thread block is
  // as follows.  From low to high address:
  //
  // 1. pthread struct (sizeof struct pthread)
  // 2. tls data       (__builtin_wasm_tls_size())
  // 3. tsd pointers   (__pthread_tsd_size)
  // 4. stack          (__default_stacksize AKA -sDEFAULT_PTHREAD_STACK_SIZE)
  size_t size = sizeof(struct pthread);
  if (__builtin_wasm_tls_size()) {
    size += __builtin_wasm_tls_size() + __builtin_wasm_tls_align() - 1;
  }
  size += __pthread_tsd_size + TSD_ALIGN - 1;
  size_t zero_size = size;
  if (!attr._a_stackaddr) {
    size += attr._a_stacksize + STACK_ALIGN - 1;
  }

  // Allocate all the data for the new thread and zero-initialize all parts
  // except for the stack.
  unsigned char* block = emscripten_builtin_malloc(size);
  memset(block, 0, zero_size);

  uintptr_t offset = (uintptr_t)block;

  // 1. pthread struct
  struct pthread *new = (struct pthread*)offset;
  offset += sizeof(struct pthread);

  new->map_base = block;
  new->map_size = size;

  // The pthread struct has a field that points to itself - this is used as a
  // magic ID to detect whether the pthread_t structure is 'alive'.
  new->self = new;
  new->tid = next_tid++;

  // pthread struct robust_list head should point to itself.
  new->robust_list.head = &new->robust_list.head;

  new->locale = &libc.global_locale;
  if (attr._a_detach) {
    new->detach_state = DT_DETACHED;
  } else {
    new->detach_state = DT_JOINABLE;
  }
  new->stack_size = attr._a_stacksize;

  // 2. tls data
  if (__builtin_wasm_tls_size()) {
    offset = ROUND_UP(offset, __builtin_wasm_tls_align());
    new->tls_base = (void*)offset;
    offset += __builtin_wasm_tls_size();
  }

  // 3. tsd slots
  if (__pthread_tsd_size) {
    offset = ROUND_UP(offset, TSD_ALIGN);
    new->tsd = (void*)offset;
    offset += __pthread_tsd_size;
  }

  // 4. stack data
  // musl stores top of the stack in pthread_t->stack (i.e. the high
  // end from which it grows down).
  if (attr._a_stackaddr) {
    new->stack = (void*)attr._a_stackaddr;
  } else {
    offset = ROUND_UP(offset + new->stack_size, STACK_ALIGN);
    new->stack = (void*)offset;
  }

  // Check that we didn't use more data than we allocated.
  assert(offset < (uintptr_t)block + size);

#ifndef NDEBUG
  _emscripten_thread_profiler_init(new);
#endif

  _emscripten_thread_mailbox_init(new);

  struct pthread *self = __pthread_self();
  dbg("start __pthread_create: new=%p new_end=%p stack=%p->%p "
      "stack_size=%zu tls_base=%p",
      new,
      new + 1,
      (char*)new->stack - new->stack_size,
      new->stack,
      new->stack_size,
      new->tls_base);

  // thread may already be running/exited after the _pthread_create_js call below
  __tl_lock();

  new->next = self->next;
  new->prev = self;
  new->next->prev = new;
  new->prev->next = new;

  __tl_unlock();

  // Set libc.need_locks before calling __pthread_create_js since
  // by the time it returns the thread could be running and we
  // want libc.need_locks to be set from the moment it starts.
  if (!libc.threads_minus_1++) libc.need_locks = 1;

  int rtn = __pthread_create_js(new, &attr, entry, arg);
  if (rtn != 0) {
    if (!--libc.threads_minus_1) libc.need_locks = 0;

    // undo previous addition to the thread list
    __tl_lock();

    new->next->prev = new->prev;
    new->prev->next = new->next;
    new->next = new->prev = new;

    __tl_unlock();

    return rtn;
  }

  dbg("done __pthread_create next=%p prev=%p new=%p",
      self->next,
      self->prev,
      new);

  *res = new;
  return 0;
}

/*
 * Called from JS main thread to free data accociated a thread
 * that is no longer running.
 */
void _emscripten_thread_free_data(pthread_t t) {
  // A thread can never free its own thread data.
  assert(t != pthread_self());
#ifndef NDEBUG
  if (t->profilerBlock) {
    emscripten_builtin_free(t->profilerBlock);
  }
#endif

  // Free all the enture thread block (called map_base because
  // musl normally allocates this using mmap).  This region
  // includes the pthread structure itself.
  unsigned char* block = t->map_base;
  dbg("_emscripten_thread_free_data thread=%p map_base=%p", t, block);
  // To aid in debugging, set the entire region to zero.
  memset(block, 0, sizeof(struct pthread));
  emscripten_builtin_free(block);
}

void _emscripten_thread_exit(void* result) {
  struct pthread *self = __pthread_self();
  assert(self);

  self->canceldisable = PTHREAD_CANCEL_DISABLE;
  self->cancelasync = PTHREAD_CANCEL_DEFERRED;
  self->result = result;

  _emscripten_thread_mailbox_shutdown(self);

  // Run any handlers registered with pthread_cleanup_push
  __run_cleanup_handlers();

  // Call into the musl function that runs destructors of all thread-specific data.
  __pthread_tsd_run_dtors();

  if (!--libc.threads_minus_1) libc.need_locks = 0;

  __tl_lock();

  self->next->prev = self->prev;
  self->prev->next = self->next;
  self->prev = self->next = self;

  __tl_unlock();

  if (emscripten_is_main_runtime_thread()) {
    exit(0);
    return;
  }

  // Not hosting a pthread anymore in this worker set __pthread_self to NULL
  __set_thread_state(NULL, 0, 0, 1);

  /* This atomic potentially competes with a concurrent pthread_detach
   * call; the loser is responsible for freeing thread resources. */
  int state = a_cas(&self->detach_state, DT_JOINABLE, DT_EXITING);

  if (state == DT_DETACHED) {
    _emscripten_thread_cleanup(self);
  } else {
    // Mark the thread as no longer running, so it can be joined.
    // Once we publish this, any threads that are waiting to join with us can
    // proceed and this worker can be recycled and used on another thread.
#ifdef EMSCRIPTEN_DYNAMIC_LINKING
    // When dynamic linking is enabled we need to keep track of zombie threads
    _emscripten_thread_exit_joinable(self);
#endif
    a_store(&self->detach_state, DT_EXITED);
    __wake(&self->detach_state, 1, 1); // Wake any joiner.
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
weak_alias(__pthread_exit, emscripten_builtin_pthread_exit);
weak_alias(__pthread_exit, pthread_exit);
