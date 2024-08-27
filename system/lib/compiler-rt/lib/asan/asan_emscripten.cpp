#include "asan_interceptors.h"
#include "asan_internal.h"
#include "asan_mapping.h"
#include "asan_poisoning.h"
#include "asan_stack.h"
#include "asan_thread.h"
#include "lsan/lsan_common.h"  // for CAN_SANITIZE_LEAKS

#if SANITIZER_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/heap.h>
#include <cassert>
#include <cstddef>
#include <pthread.h>
#define __ATTRP_C11_THREAD ((void*)(uptr)-1)

namespace __asan {

void InitializeShadowMemory() {
  // Poison the shadow memory of the shadow area at the start of the address
  // space. This helps catching null pointer dereference.
  FastPoisonShadow(kLowShadowBeg, kLowShadowEnd - kLowShadowBeg, 0xff);

  // Assert that the shadow region is large enough.  We don't want to start
  // running into the static data region which starts right after the shadow
  // region.
  uptr max_address =
    (__builtin_wasm_memory_size(0) * uint64_t(WASM_PAGE_SIZE)) - 1;
  uptr max_shadow_address = MEM_TO_SHADOW(max_address);
  // TODO(sbc): In the growable memory case we should really be checking this
  // every time we grow.
  assert(max_shadow_address <= kLowShadowEnd && "shadow region is too small");
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

void FlushUnneededASanShadowMemory(uptr p, uptr size) {}

extern "C" {
int emscripten_builtin_pthread_create(pthread_t *thread,
                                      const pthread_attr_t *attr,
                                      void *(*callback)(void *), void *arg);
}

struct ThreadStartParams {
  thread_callback_t start_routine;
  void *arg;
};

static thread_return_t THREAD_CALLING_CONV asan_thread_start(void *arg) {
  AsanThread *t = (AsanThread *)arg;
  SetCurrentThread(t);
  t->ThreadStart(GetTid());
  ThreadStartParams params;
  t->GetStartData(params);
  auto res = (*params.start_routine)(params.arg);
  return res;
}

INTERCEPTOR(int, pthread_create, pthread_t *thread,
    const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg) {
  EnsureMainThreadIDIsCorrect();
  // Strict init-order checking is thread-hostile.
  if (flags()->strict_init_order)
    StopInitOrderChecking();
  GET_STACK_TRACE_THREAD;
  int detached = 0;
  if (attr && attr != __ATTRP_C11_THREAD)
    pthread_attr_getdetachstate(attr, &detached);

  u32 current_tid = GetCurrentTidOrInvalid();
  ThreadStartParams params = {start_routine, arg};
  AsanThread* t = AsanThread::Create(params, current_tid, &stack, detached);

  int result;
  {
    // Ignore all allocations made by pthread_create: thread stack/TLS may be
    // stored by pthread for future reuse even after thread destruction, and
    // the linked list it's stored in doesn't even hold valid pointers to the
    // objects, the latter are calculated by obscure pointer arithmetic.
#if CAN_SANITIZE_LEAKS
    __lsan::ScopedInterceptorDisabler disabler;
#endif
    result = REAL(pthread_create)(thread, attr, asan_thread_start, t);
  }
  if (result != 0) {
    // If the thread didn't start delete the AsanThread to avoid leaking it.
    // Note AsanThreadContexts never get destroyed so the AsanThreadContext
    // that was just created for the AsanThread is wasted.
    t->Destroy();
  }
  return result;
}

} // namespace __asan

namespace __lsan {

#ifndef __EMSCRIPTEN_PTHREADS__
// XXX HACK: Emscripten treats thread_local variables the same as globals in
// non-threaded builds, so a hack was introduced where we skip the allocator
// cache in the common module. Now we have to define this symbol to keep that
// hack working when using LSan as part of ASan without threads.
void GetAllocatorCacheRange(uptr *begin, uptr *end) {
  *begin = *end = 0;
}
#endif

} // namespace __lsan

#endif // SANITIZER_EMSCRIPTEN
