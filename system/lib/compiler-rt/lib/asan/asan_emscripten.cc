#include "asan_interceptors.h"
#include "asan_internal.h"
#include "asan_mapping.h"
#include "asan_poisoning.h"
#include "asan_stack.h"
#include "asan_thread.h"

#if SANITIZER_EMSCRIPTEN
#include <emscripten.h>
#include <cstddef>

namespace __asan {

void InitializeShadowMemory() {
  // Poison the shadow memory of the shadow area at the start of the address
  // space. This helps catching null pointer dereference.
  FastPoisonShadow(kLowShadowBeg, kLowShadowEnd - kLowShadowBeg, 0xff);
}

void AsanCheckDynamicRTPrereqs() {}
void AsanCheckIncompatibleRT() {}
void InitializePlatformInterceptors() {}
void InitializePlatformExceptionHandlers() {}
bool IsSystemHeapAddress (uptr addr) { return false; }

void *AsanDoesNotSupportStaticLinkage() {
  // On Linux, this is some magic that fails linking with -static.
  // On Emscripten, we have to do static linking, so we stub this out.
  return nullptr;
}

void InitializeAsanInterceptors() {}

// We can use a plain thread_local variable for TSD.
static thread_local void *per_thread;

void *AsanTSDGet() { return per_thread; }

void AsanTSDSet(void *tsd) { per_thread = tsd; }

// There's no initialization needed, and the passed-in destructor
// will never be called.  Instead, our own thread destruction hook
// (below) will call AsanThread::TSDDtor directly.
void AsanTSDInit(void (*destructor)(void *tsd)) {
  DCHECK(destructor == &PlatformTSDDtor);
}

void PlatformTSDDtor(void *tsd) { UNREACHABLE(__func__); }

extern "C" {
  void *emscripten_builtin_malloc(size_t size);
  void emscripten_builtin_free(void *memory);
  int emscripten_builtin_pthread_create(void *thread, void *attr,
                                        void *(*callback)(void *), void *arg);
  int pthread_attr_getdetachstate(void *attr, int *detachstate);
}

static thread_return_t THREAD_CALLING_CONV asan_thread_start(void *arg) {
  atomic_uintptr_t *param = reinterpret_cast<atomic_uintptr_t *>(arg);
  AsanThread *t = nullptr;
  while ((t = reinterpret_cast<AsanThread *>(
              atomic_load(param, memory_order_acquire))) == nullptr)
    internal_sched_yield();
  emscripten_builtin_free(param);
  SetCurrentThread(t);
  return t->ThreadStart(GetTid(), nullptr);
}

INTERCEPTOR(int, pthread_create, void *thread,
    void *attr, void *(*start_routine)(void*), void *arg) {
  EnsureMainThreadIDIsCorrect();
  // Strict init-order checking is thread-hostile.
  if (flags()->strict_init_order)
    StopInitOrderChecking();
  GET_STACK_TRACE_THREAD;
  int detached = 0;
  if (attr)
    pthread_attr_getdetachstate(attr, &detached);
  atomic_uintptr_t *param = (atomic_uintptr_t *)
      emscripten_builtin_malloc(sizeof(atomic_uintptr_t));
  atomic_store(param, 0, memory_order_relaxed);
  int result;
  {
    // Ignore all allocations made by pthread_create: thread stack/TLS may be
    // stored by pthread for future reuse even after thread destruction, and
    // the linked list it's stored in doesn't even hold valid pointers to the
    // objects, the latter are calculated by obscure pointer arithmetic.
#if CAN_SANITIZE_LEAKS
    __lsan::ScopedInterceptorDisabler disabler;
#endif
    result = REAL(pthread_create)(thread, attr, asan_thread_start, param);
  }
  if (result == 0) {
    u32 current_tid = GetCurrentTidOrInvalid();
    AsanThread *t =
        AsanThread::Create(start_routine, arg, current_tid, &stack, detached);
    atomic_store(param, reinterpret_cast<uptr>(t), memory_order_release);
  }
  return result;
}

} // namespace __asan

namespace __lsan {

#ifndef USE_THREADS
// XXX HACK: Emscripten treats thread_local variables the same as globals in
// non-threaded builds, so a hack was introduced where we skip the allocator
// cache in the common module. Now we have to define this symbol to keep that
// hack working when using LSan as part of ASan without threads.
void GetAllocatorCacheRange(uptr *begin, uptr *end) {
  *begin = *end = 0;
}
#endif

u32 GetCurrentThread() { return __asan::GetCurrentThread()->tid(); }

} // namespace __lsan

#endif // SANITIZER_EMSCRIPTEN
