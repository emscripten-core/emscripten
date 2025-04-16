#include <assert.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <emscripten/heap.h>
#include <malloc.h>

#ifdef __EMSCRIPTEN_WASM_WORKERS__
#error __EMSCRIPTEN_WASM_WORKERS__ should not be defined when building this file!
#endif

emscripten_wasm_worker_t emscripten_create_wasm_worker(void *stackPlusTLSAddress, size_t stackPlusTLSSize) {
  return 0;
}

emscripten_wasm_worker_t emscripten_malloc_wasm_worker(size_t stackSize) {
  return 0;
}

void emscripten_wasm_worker_sleep(int64_t nsecs) {
}

void emscripten_lock_init(emscripten_lock_t *lock) {
}

bool emscripten_lock_wait_acquire(emscripten_lock_t *lock, int64_t maxWaitNanoseconds) {
  return true;
}

void emscripten_lock_waitinf_acquire(emscripten_lock_t *lock) {
}

bool emscripten_lock_busyspin_wait_acquire(emscripten_lock_t *lock, double maxWaitMilliseconds) {
  return true;
}

void emscripten_lock_busyspin_waitinf_acquire(emscripten_lock_t *lock) {
}

bool emscripten_lock_try_acquire(emscripten_lock_t *lock) {
  return true;
}

void emscripten_lock_release(emscripten_lock_t *lock) {
}

void emscripten_semaphore_init(emscripten_semaphore_t *sem, int num) {
}

int emscripten_semaphore_try_acquire(emscripten_semaphore_t *sem, int num) {
  *sem -= num;
  return *sem;
}

int emscripten_semaphore_wait_acquire(emscripten_semaphore_t *sem, int num, int64_t maxWaitNanoseconds) {
  *sem -= num;
  return *sem;
}

int emscripten_semaphore_waitinf_acquire(emscripten_semaphore_t *sem, int num) {
  *sem -= num;
  return *sem;
}

uint32_t emscripten_semaphore_release(emscripten_semaphore_t *sem, int num) {
  *sem += num;
  return *sem - num;
}

void emscripten_condvar_init(emscripten_condvar_t *condvar) {
}

void emscripten_condvar_waitinf(emscripten_condvar_t *condvar, emscripten_lock_t *lock) {
}

bool emscripten_condvar_wait(emscripten_condvar_t *condvar, emscripten_lock_t *lock, int64_t maxWaitNanoseconds) {
  return true;
}

ATOMICS_WAIT_TOKEN_T emscripten_condvar_wait_async(emscripten_condvar_t *condvar,
                                                  emscripten_lock_t *lock,
                                                  void (*asyncWaitFinished)(int32_t *address, uint32_t value, ATOMICS_WAIT_RESULT_T waitResult, void *userData),
                                                  void *userData,
                                                  double maxWaitMilliseconds) {
  return 0;
}

void emscripten_condvar_signal(emscripten_condvar_t *condvar, int64_t numWaitersToSignal) {
}
