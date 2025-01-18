/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread_impl.h"
#include <emscripten/stack.h>
#include <emscripten/threading.h>
#include <emscripten/emscripten.h>

int emscripten_has_threading_support() { return 0; }

int emscripten_num_logical_cores() { return 1; }

int emscripten_futex_wait(
  volatile void /*uint32_t*/* addr, uint32_t val, double maxWaitMilliseconds) {
  // nop
  return 0; // success
}

int emscripten_futex_wake(volatile void /*uint32_t*/* addr, int count) {
  // nop
  return 0; // success
}

int emscripten_is_main_runtime_thread() { return 1; }

void emscripten_main_thread_process_queued_calls() {
  // nop
}

void emscripten_current_thread_process_queued_calls() {
  // nop
}

static void dummy(double now)
{
}

weak_alias(dummy, _emscripten_check_timers);

void _emscripten_yield(double now) {
  _emscripten_check_timers(now);
}

int pthread_mutex_init(
  pthread_mutex_t* __restrict mutex, const pthread_mutexattr_t* __restrict attr) {
  return 0;
}

int __pthread_mutex_lock(pthread_mutex_t* mutex) { return 0; }

weak_alias(__pthread_mutex_lock, pthread_mutex_lock);

int __pthread_mutex_unlock(pthread_mutex_t* mutex) { return 0; }

weak_alias(__pthread_mutex_unlock, pthread_mutex_unlock);

int __pthread_mutex_trylock(pthread_mutex_t* mutex) { return 0; }

weak_alias(__pthread_mutex_trylock, pthread_mutex_trylock);

struct timespec;

int __pthread_mutex_timedlock(
  pthread_mutex_t* __restrict mutex, const struct timespec* __restrict t) {
  return 0;
}

weak_alias(__pthread_mutex_timedlock, pthread_mutex_timedlock);

int pthread_mutex_destroy(pthread_mutex_t* mutex) { return 0; }

int pthread_mutex_consistent(pthread_mutex_t* mutex) { return 0; }

int pthread_barrier_init(
  pthread_barrier_t* __restrict mutex, const pthread_barrierattr_t* __restrict attr, unsigned u) {
  return 0;
}

int pthread_barrier_destroy(pthread_barrier_t* mutex) { return 0; }

int pthread_barrier_wait(pthread_barrier_t* mutex) { return 0; }

int __pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) {
  return EAGAIN;
}

weak_alias(__pthread_create, emscripten_builtin_pthread_create);
weak_alias(__pthread_create, pthread_create);

int __pthread_join(pthread_t thread, void **retval) {
  return EINVAL;
}

weak_alias(__pthread_join, emscripten_builtin_pthread_join);
weak_alias(__pthread_join, pthread_join);

static void* tls_entries[PTHREAD_KEYS_MAX];
static bool tls_key_used[PTHREAD_KEYS_MAX];

int __pthread_key_create(pthread_key_t* key, void (*destructor)(void*)) {
  if (key == 0) {
    return EINVAL;
  }
  // Find empty spot.
  for (pthread_key_t entry = 0; entry < PTHREAD_KEYS_MAX; entry++) {
    if (!tls_key_used[entry]) {
      tls_key_used[entry] = true;
      tls_entries[entry] = NULL;
      *key = entry;
      return 0;
    }
  }
  // No empty spots, return an error
  return EAGAIN;
}

int __pthread_key_delete(pthread_key_t key) {
  if (key < 0 || key >= PTHREAD_KEYS_MAX) {
    return EINVAL;
  }
  if (!tls_key_used[key]) {
    return EINVAL;
  }
  tls_key_used[key] = false;
  tls_entries[key] = NULL;
  return 0;
}

weak_alias(__pthread_key_delete, pthread_key_delete);
weak_alias(__pthread_key_create, pthread_key_create);

void* pthread_getspecific(pthread_key_t key) {
  if (key < 0 || key >= PTHREAD_KEYS_MAX) {
    return NULL;
  }
  if (!tls_key_used[key]) {
    return NULL;
  }
  return tls_entries[key];
}

int pthread_setspecific(pthread_key_t key, const void* value) {
  if (key < 0 || key >= PTHREAD_KEYS_MAX) {
    return EINVAL;
  }
  if (!tls_key_used[key]) {
    return EINVAL;
  }
  tls_entries[key] = (void*)value;
  return 0;
}

/*magic number to detect if we have not run yet*/
#define PTHREAD_ONCE_MAGIC_ID 0x13579BDF

int __pthread_once(pthread_once_t* once_control, void (*init_routine)(void)) {
  if (*once_control != PTHREAD_ONCE_MAGIC_ID) {
    init_routine();
    *once_control = PTHREAD_ONCE_MAGIC_ID;
  }
  return 0;
}

weak_alias(__pthread_once, pthread_once);

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
  return 0;
}

int pthread_cond_signal(pthread_cond_t *cond) {
  return 0;
}

int __private_cond_signal(pthread_cond_t *c, int n) {
  return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond) {
  return 0;
}

int pthread_cond_init(pthread_cond_t *__restrict x, const pthread_condattr_t *__restrict y) {
  return 0;
}

int pthread_cond_destroy(pthread_cond_t * x) {
  return 0;
}

int __pthread_cond_timedwait(pthread_cond_t *__restrict x, pthread_mutex_t *__restrict y, const struct timespec *__restrict z) {
  return 0;
}

weak_alias(__pthread_cond_timedwait, pthread_cond_timedwait);

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void)) {
  return 0;
}

int pthread_cancel(pthread_t thread) {
  return 0;
}

void pthread_testcancel() {}

_Noreturn void __pthread_exit(void* status) {
   exit(0);
}

weak_alias(__pthread_exit, emscripten_builtin_pthread_exit);
weak_alias(__pthread_exit, pthread_exit);

int __pthread_detach(pthread_t t) {
  return 0;
}

weak_alias(__pthread_detach, emscripten_builtin_pthread_detach);
weak_alias(__pthread_detach, pthread_detach);
weak_alias(__pthread_detach, thrd_detach);

// pthread_equal is defined as a macro in C, as a function for C++; undef it
// here so we define the function for C++ that links to us.
#ifdef pthread_equal
#undef pthread_equal
#endif

int pthread_equal(pthread_t t1, pthread_t t2) {
  return t1 == t2;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr) {
  return 0;
}

int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol) {
  return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
  return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
  return 0;
}

int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared) {
  // XXX implement if/when getpshared is required
  return 0;
}

int pthread_condattr_init(pthread_condattr_t * attr) {
  return 0;
}

int pthread_condattr_destroy(pthread_condattr_t *attr) {
  return 0;
}

int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clk) {
  return 0;
}

int pthread_condattr_setpshared(pthread_condattr_t *attr, int shared) {
  return 0;
}

int pthread_getattr_np(pthread_t thread, pthread_attr_t *attr) {
  return 0;
}

int pthread_setcancelstate(int state, int* oldstate) {
  return 0;
}

int pthread_setcanceltype(int type, int* oldtype) {
  return 0;
}

int pthread_rwlock_init(pthread_rwlock_t* rwlock, const pthread_rwlockattr_t* attr) {
  return 0;
}

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock) {
  return 0;
}

int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock) {
  return 0;
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t* rwlock) {
  return 0;
}

int pthread_rwlock_timedrdlock(pthread_rwlock_t* rwlock, const struct timespec* abs_timeout) {
  return 0;
}

int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock) {
  return 0;
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock) {
  return 0;
}

int pthread_rwlock_timedwrlock(pthread_rwlock_t* rwlock, const struct timespec* abs_timeout) {
  return 0;
}

int pthread_rwlock_unlock(pthread_rwlock_t *rwlock) {
  return 0;
}

int pthread_rwlockattr_init(pthread_rwlockattr_t *attr) {
  return 0;
}

int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr) {
  return 0;
}

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t* attr, int pshared) {
  return 0;
}

int pthread_spin_init(pthread_spinlock_t *lock, int pshared) {
  return 0;
}

int pthread_spin_destroy(pthread_spinlock_t *lock) {
  return 0;
}

int pthread_spin_lock(pthread_spinlock_t *lock) {
  return 0;
}

int pthread_spin_trylock(pthread_spinlock_t *lock) {
  return 0;
}

int pthread_spin_unlock(pthread_spinlock_t *lock) {
  return 0;
}

int sem_init(sem_t *sem, int pshared, unsigned int value) {
  return 0;
}

int sem_post(sem_t *sem) {
  return 0;
}

int sem_wait(sem_t *sem) {
  return 0;
}

int sem_trywait(sem_t *sem) {
  return 0;
}

int sem_destroy(sem_t *sem) {
  return 0;
}

void __wait(volatile int *addr, volatile int *waiters, int val, int priv) {}

void __lock(void* ptr) {}

void __unlock(void* ptr) {}

void __acquire_ptc() {}

void __release_ptc() {}

// When pthreads is not enabled, we can't use the Atomics futex api to do
// proper sleeps, so simulate a busy spin wait loop instead.
void emscripten_thread_sleep(double msecs) {
  double start = emscripten_get_now();
  double now = start;
  do {
    _emscripten_yield(now);
    now = emscripten_get_now();
  } while (now - start < msecs);
}
