#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdlib.h>

int emscripten_has_threading_support() { return 0; }

int emscripten_num_logical_cores() { return 1; }

void emscripten_force_num_logical_cores(int cores) {
  // no-op, in singlethreaded builds we will always report exactly one core.
}

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

int pthread_mutex_init(
  pthread_mutex_t* __restrict mutex, const pthread_mutexattr_t* __restrict attr) {
  return 0;
}

int pthread_mutex_lock(pthread_mutex_t* mutex) { return 0; }

int pthread_mutex_unlock(pthread_mutex_t* mutex) { return 0; }

int pthread_mutex_trylock(pthread_mutex_t* mutex) { return 0; }

struct timespec;

int pthread_mutex_timedlock(
  pthread_mutex_t* __restrict mutex, const struct timespec* __restrict t) {
  return 0;
}

int pthread_mutex_destroy(pthread_mutex_t* mutex) { return 0; }

int pthread_mutex_consistent(pthread_mutex_t* mutex) { return 0; }

int pthread_barrier_init(
  pthread_barrier_t* __restrict mutex, const pthread_barrierattr_t* __restrict attr, unsigned u) {
  return 0;
}

int pthread_barrier_destroy(pthread_barrier_t* mutex) { return 0; }

int pthread_barrier_wait(pthread_barrier_t* mutex) { return 0; }

// pthread_key_t is 32-bit, so to be able to store pointers in there, we sadly
// have to track an array of them.
static size_t num_tls_entries = 0;
static size_t max_tls_entries = 0;
struct entry_t { const void* value; int allocated; };
static struct entry_t* tls_entries = NULL;

int pthread_key_create(pthread_key_t* key, void (*destructor)(void*)) {
  if (key == 0)
    return EINVAL;
  if (!max_tls_entries) {
    // First time we're called, allocate entry table.
    max_tls_entries = 4;
    tls_entries = (struct entry_t*)malloc(max_tls_entries * sizeof(void *));
  }
  // Find empty spot.
  size_t entry = 0;
  for (; entry < num_tls_entries; entry++) {
    if (!tls_entries[entry].allocated) break;
  }
  if (entry == max_tls_entries) {
    // No empty spots, table full: double the table.
    max_tls_entries *= 2;
    tls_entries =
      (struct entry_t*)realloc(tls_entries, num_tls_entries * sizeof(void *));
  }
  if (entry == num_tls_entries) {
    // No empty spots, but table not full.
    num_tls_entries++;
  }
  struct entry_t* e = &tls_entries[entry];
  e->value = NULL;
  e->allocated = 1;
  // Key can't be zero.
  *key = (pthread_key_t)entry + 1;
  return 0;
}

int pthread_key_delete(pthread_key_t key) {
  if (key == 0 || key > num_tls_entries)
    return EINVAL;
  struct entry_t* e = &tls_entries[key - 1];
  if (!e->allocated)
    return EINVAL;
  e->value = NULL;
  e->allocated = 0;
  return 0;
}

void* pthread_getspecific(pthread_key_t key) {
  if (key == 0 || key > num_tls_entries)
    return NULL;
  struct entry_t* e = &tls_entries[key - 1];
  if (!e->allocated)
    return NULL;
  return (void*)e->value;
}

int pthread_setspecific(pthread_key_t key, const void* value) {
  if (key == 0 || key > num_tls_entries)
    return EINVAL;
  struct entry_t* e = &tls_entries[key - 1];
  if (!e->allocated)
    return EINVAL;
  e->value = value;
  return 0;
}

/*magic number to detect if we have not run yet*/
#define PTHREAD_ONCE_MAGIC_ID 0x13579BDF

int pthread_once(pthread_once_t* once_control, void (*init_routine)(void)) {
  if (*once_control != PTHREAD_ONCE_MAGIC_ID) {
    init_routine();
    *once_control = PTHREAD_ONCE_MAGIC_ID;
  }
  return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
  return 0;
}

int pthread_cond_signal(pthread_cond_t *cond) {
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

int pthread_cond_timedwait(pthread_cond_t *__restrict x, pthread_mutex_t *__restrict y, const struct timespec *__restrict z) {
  return 0;
}

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void)) {
  return 0;
}

int pthread_cancel(pthread_t thread) {
  return 0;
}

_Noreturn void pthread_exit(void* status) {
   exit((int)status);
}

int pthread_detach(pthread_t t) {
  return 0;
}

int emscripten_main_browser_thread_id() {
  return (int)pthread_self();
}

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

int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t* clk) {
  return 0;
}

int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *shared) {
  return 0;
}

int pthread_attr_init(pthread_attr_t *attr) {
  return 0;
}

int pthread_getattr_np(pthread_t thread, pthread_attr_t *attr) {
  return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr) {
  return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate) {
  return 0;
}

int pthread_setcancelstate() {
  return 0;
}

int pthread_setcanceltype() {
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

int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t* attr, int *pshared) {
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

int pthread_attr_setdetachstate(pthread_attr_t* attr, int detachstate) {
  return 0;
}

int pthread_attr_setschedparam(pthread_attr_t* attr, const struct sched_param* param) {
  return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize) {
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
