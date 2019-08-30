#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

int emscripten_has_threading_support() { return 0; }

int emscripten_num_logical_cores() { return 1; }

void emscripten_force_num_logical_cores(int cores) {
  // no-op, in singlethreaded builds we will always report exactly one core.
}

uint8_t emscripten_atomic_exchange_u8(void /*uint8_t*/* addr, uint8_t newVal) {
  uint8_t old = *(uint8_t*)addr;
  *(uint8_t*)addr = newVal;
  return old;
}

uint16_t emscripten_atomic_exchange_u16(void /*uint16_t*/* addr, uint16_t newVal) {
  uint16_t old = *(uint16_t*)addr;
  *(uint16_t*)addr = newVal;
  return old;
}

uint32_t emscripten_atomic_exchange_u32(void /*uint32_t*/* addr, uint32_t newVal) {
  uint32_t old = *(uint32_t*)addr;
  *(uint32_t*)addr = newVal;
  return old;
}

uint64_t emscripten_atomic_exchange_u64(void /*uint64_t*/* addr, uint64_t newVal) {
  uint64_t old = *(uint64_t*)addr;
  *(uint64_t*)addr = newVal;
  return old;
}

uint8_t emscripten_atomic_cas_u8(void /*uint8_t*/* addr, uint8_t oldVal, uint8_t newVal) {
  uint8_t old = *(uint8_t*)addr;
  if (old == oldVal)
    *(uint8_t*)addr = newVal;
  return old;
}

uint16_t emscripten_atomic_cas_u16(void /*uint16_t*/* addr, uint16_t oldVal, uint16_t newVal) {
  uint16_t old = *(uint16_t*)addr;
  if (old == oldVal)
    *(uint16_t*)addr = newVal;
  return old;
}

uint32_t emscripten_atomic_cas_u32(void /*uint32_t*/* addr, uint32_t oldVal, uint32_t newVal) {
  uint32_t old = *(uint32_t*)addr;
  if (old == oldVal)
    *(uint32_t*)addr = newVal;
  return old;
}

uint64_t emscripten_atomic_cas_u64(void /*uint64_t*/* addr, uint64_t oldVal, uint64_t newVal) {
  uint64_t old = *(uint64_t*)addr;
  if (old == oldVal)
    *(uint64_t*)addr = newVal;
  return old;
}

uint8_t emscripten_atomic_load_u8(const void /*uint8_t*/* addr) { return *(uint8_t*)addr; }

uint16_t emscripten_atomic_load_u16(const void /*uint16_t*/* addr) { return *(uint16_t*)addr; }

uint32_t emscripten_atomic_load_u32(const void /*uint32_t*/* addr) { return *(uint32_t*)addr; }

float emscripten_atomic_load_f32(const void /*float*/* addr) { return *(float*)addr; }

uint64_t emscripten_atomic_load_u64(const void /*uint64_t*/* addr) { return *(uint64_t*)addr; }

double emscripten_atomic_load_f64(const void /*double*/* addr) { return *(double*)addr; }

uint8_t emscripten_atomic_store_u8(void /*uint8_t*/* addr, uint8_t val) {
  return *(uint8_t*)addr = val;
}

uint16_t emscripten_atomic_store_u16(void /*uint16_t*/* addr, uint16_t val) {
  return *(uint16_t*)addr = val;
}

uint32_t emscripten_atomic_store_u32(void /*uint32_t*/* addr, uint32_t val) {
  return *(uint32_t*)addr = val;
}

float emscripten_atomic_store_f32(void /*float*/* addr, float val) { return *(float*)addr = val; }

uint64_t emscripten_atomic_store_u64(void /*uint64_t*/* addr, uint64_t val) {
  return *(uint64_t*)addr = val;
}

double emscripten_atomic_store_f64(void /*double*/* addr, double val) {
  return *(double*)addr = val;
}

void emscripten_atomic_fence() {
  // nop
}

uint8_t emscripten_atomic_add_u8(void /*uint8_t*/* addr, uint8_t val) {
  uint8_t old = *(uint8_t*)addr;
  *(uint8_t*)addr = old + val;
  return old;
}

uint16_t emscripten_atomic_add_u16(void /*uint16_t*/* addr, uint16_t val) {
  uint16_t old = *(uint16_t*)addr;
  *(uint16_t*)addr = old + val;
  return old;
}

uint32_t emscripten_atomic_add_u32(void /*uint32_t*/* addr, uint32_t val) {
  uint32_t old = *(uint32_t*)addr;
  *(uint32_t*)addr = old + val;
  return old;
}

uint64_t emscripten_atomic_add_u64(void /*uint64_t*/* addr, uint64_t val) {
  uint64_t old = *(uint64_t*)addr;
  *(uint64_t*)addr = old + val;
  return old;
}

uint8_t emscripten_atomic_sub_u8(void /*uint8_t*/* addr, uint8_t val) {
  uint8_t old = *(uint8_t*)addr;
  *(uint8_t*)addr = old - val;
  return old;
}

uint16_t emscripten_atomic_sub_u16(void /*uint16_t*/* addr, uint16_t val) {
  uint16_t old = *(uint16_t*)addr;
  *(uint16_t*)addr = old - val;
  return old;
}

uint32_t emscripten_atomic_sub_u32(void /*uint32_t*/* addr, uint32_t val) {
  uint32_t old = *(uint32_t*)addr;
  *(uint32_t*)addr = old - val;
  return old;
}

uint64_t emscripten_atomic_sub_u64(void /*uint64_t*/* addr, uint64_t val) {
  uint64_t old = *(uint64_t*)addr;
  *(uint64_t*)addr = old - val;
  return old;
}

uint8_t emscripten_atomic_and_u8(void /*uint8_t*/* addr, uint8_t val) {
  uint8_t old = *(uint8_t*)addr;
  *(uint8_t*)addr = old & val;
  return old;
}

uint16_t emscripten_atomic_and_u16(void /*uint16_t*/* addr, uint16_t val) {
  uint16_t old = *(uint16_t*)addr;
  *(uint16_t*)addr = old & val;
  return old;
}

uint32_t emscripten_atomic_and_u32(void /*uint32_t*/* addr, uint32_t val) {
  uint32_t old = *(uint32_t*)addr;
  *(uint32_t*)addr = old & val;
  return old;
}

uint64_t emscripten_atomic_and_u64(void /*uint64_t*/* addr, uint64_t val) {
  uint64_t old = *(uint64_t*)addr;
  *(uint64_t*)addr = old & val;
  return old;
}

uint8_t emscripten_atomic_or_u8(void /*uint8_t*/* addr, uint8_t val) {
  uint8_t old = *(uint8_t*)addr;
  *(uint8_t*)addr = old | val;
  return old;
}

uint16_t emscripten_atomic_or_u16(void /*uint16_t*/* addr, uint16_t val) {
  uint16_t old = *(uint16_t*)addr;
  *(uint16_t*)addr = old | val;
  return old;
}

uint32_t emscripten_atomic_or_u32(void /*uint32_t*/* addr, uint32_t val) {
  uint32_t old = *(uint32_t*)addr;
  *(uint32_t*)addr = old | val;
  return old;
}

uint64_t emscripten_atomic_or_u64(void /*uint64_t*/* addr, uint64_t val) {
  uint64_t old = *(uint64_t*)addr;
  *(uint64_t*)addr = old | val;
  return old;
}

uint8_t emscripten_atomic_xor_u8(void /*uint8_t*/* addr, uint8_t val) {
  uint8_t old = *(uint8_t*)addr;
  *(uint8_t*)addr = old ^ val;
  return old;
}

uint16_t emscripten_atomic_xor_u16(void /*uint16_t*/* addr, uint16_t val) {
  uint16_t old = *(uint16_t*)addr;
  *(uint16_t*)addr = old ^ val;
  return old;
}

uint32_t emscripten_atomic_xor_u32(void /*uint32_t*/* addr, uint32_t val) {
  uint32_t old = *(uint32_t*)addr;
  *(uint32_t*)addr = old ^ val;
  return old;
}

uint64_t emscripten_atomic_xor_u64(void /*uint64_t*/* addr, uint64_t val) {
  uint64_t old = *(uint64_t*)addr;
  *(uint64_t*)addr = old ^ val;
  return old;
}

uint64_t _emscripten_atomic_fetch_and_add_u64(void* addr, uint64_t val) {
  return emscripten_atomic_add_u64(addr, val);
}

uint64_t _emscripten_atomic_fetch_and_sub_u64(void* addr, uint64_t val) {
  return emscripten_atomic_sub_u64(addr, val);
}

uint64_t _emscripten_atomic_fetch_and_and_u64(void* addr, uint64_t val) {
  return emscripten_atomic_and_u64(addr, val);
}

uint64_t _emscripten_atomic_fetch_and_or_u64(void* addr, uint64_t val) {
  return emscripten_atomic_or_u64(addr, val);
}

uint64_t _emscripten_atomic_fetch_and_xor_u64(void* addr, uint64_t val) {
  return emscripten_atomic_xor_u64(addr, val);
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

#define pthread_mutex_t unsigned
#define pthread_mutexattr_t unsigned

int pthread_mutex_init(
  pthread_mutex_t* __restrict mutex, const pthread_mutexattr_t* __restrict attr) {
  return 0;
}

#ifndef EMSCRIPTEN_ASAN_OVERRIDE
int pthread_mutex_lock(pthread_mutex_t* mutex) { return 0; }

int pthread_mutex_unlock(pthread_mutex_t* mutex) { return 0; }
#endif

int pthread_mutex_trylock(pthread_mutex_t* mutex) { return 0; }

struct timespec;

int pthread_mutex_timedlock(
  pthread_mutex_t* __restrict mutex, const struct timespec* __restrict t) {
  return 0;
}

int pthread_mutex_destroy(pthread_mutex_t* mutex) { return 0; }

int pthread_mutex_consistent(pthread_mutex_t* mutex) { return 0; }

#define pthread_barrier_t unsigned
#define pthread_barrierattr_t unsigned

int pthread_barrier_init(
  pthread_barrier_t* __restrict mutex, const pthread_barrierattr_t* __restrict attr, unsigned u) {
  return 0;
}

int pthread_barrier_destroy(pthread_barrier_t* mutex) { return 0; }

int pthread_barrier_wait(pthread_barrier_t* mutex) { return 0; }

#define pthread_key_t uintptr_t

/* magic value to track a validly constructed TLS slot */
#define PTHREAD_TLS_MAGIC_ID 0x02468ACE

int pthread_key_create(pthread_key_t* key, void (*destructor)(void*)) {
  if (key == 0)
    return EINVAL;
  uintptr_t* tls = (uintptr_t*)malloc(sizeof(uintptr_t) * 2);
  tls[0] = 0;
  tls[1] = PTHREAD_TLS_MAGIC_ID;
  *key = (pthread_key_t)tls;
  return 0;
}

int pthread_key_delete(pthread_key_t key) {
  uintptr_t* tls = (uintptr_t*)key;
  if (tls[1] != PTHREAD_TLS_MAGIC_ID)
    return EINVAL;
  tls[0] = tls[1] = 0;
  free(tls);
  return 0;
}

void* pthread_getspecific(pthread_key_t key) {
  uintptr_t* tls = (uintptr_t*)key;
  if (tls[1] != PTHREAD_TLS_MAGIC_ID)
    return NULL;
  return (void*)tls[0];
}

int pthread_setspecific(pthread_key_t key, const void* value) {
  uintptr_t* tls = (uintptr_t*)key;
  if (tls[1] != PTHREAD_TLS_MAGIC_ID)
    return EINVAL;
  tls[0] = (uintptr_t)value;
  return 0;
}

#define pthread_once_t int

/*magic number to detect if we have not run yet*/
#define PTHREAD_ONCE_MAGIC_ID 0x13579BDF

int pthread_once(pthread_once_t* once_control, void (*init_routine)(void)) {
  if (*once_control != PTHREAD_ONCE_MAGIC_ID) {
    init_routine();
    *once_control = PTHREAD_ONCE_MAGIC_ID;
  }
  return 0;
}
