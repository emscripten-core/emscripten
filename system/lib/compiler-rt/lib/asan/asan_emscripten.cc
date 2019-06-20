#include "asan_internal.h"

#if SANITIZER_EMSCRIPTEN
#include <emscripten.h>
#include <cstddef>

namespace __asan {

void InitializeShadowMemory() {}
void AsanCheckDynamicRTPrereqs() {}
void AsanCheckIncompatibleRT() {}
void InitializePlatformInterceptors() {}
void InitializePlatformExceptionHandlers() {}
bool IsSystemHeapAddress (uptr addr) { return false; }

void *AsanDoesNotSupportStaticLinkage() {
  // Actually, we have to do static linkage on Emscripten.
  return nullptr;
}

void InitializeAsanInterceptors() {}

} // namespace __asan

namespace __lsan {

// XXX HACK: Emscripten doesn't support thread local storage, so a hack was
// introduced where we skip the allocator cache in the common module.
// Now we have to define this symbol to keep that hack working when using
// LSan as part of ASan.
void GetAllocatorCacheRange(uptr *begin, uptr *end) {
  *begin = *end = 0;
}

} // namespace __lsan

extern "C" void *emscripten_builtin_memset(void * ptr, int value, std::size_t num) {
  return __asan::internal_memset(ptr, value, num);
}

#endif // SANITIZER_EMSCRIPTEN
