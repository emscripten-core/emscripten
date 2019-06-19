#include "asan_internal.h"

#if SANITIZER_EMSCRIPTEN
#include "asan_emscripten.h"
#include <emscripten.h>

using namespace __asan;

extern "C" {

EM_JS(void, emasan_poison_init, (), {
  Module._asan_shadow = new Uint8Array();
  Module._asan_can_poison = false;
});

EM_JS(void, emasan_set_can_poison, (bool value), {
  Module._asan_can_poison = value;
});

EM_JS(bool, emasan_can_poison, (), {
  return !!Module._asan_can_poison;
});

EM_JS(void, emasan_poison, (uptr aligned_beg, uptr aligned_size, u8 value), {
  Module._asan_shadow.fill(value, begin >> 3, (begin + size) >> 3);
});

EM_JS(void, emasan_poison_right, (uptr aligned_addr, uptr size,
                                  uptr redzone_size, u8 value, bool partial), {
  var shadow = aligned_addr >> 3;
  var buffer = Module._asan_shadow;
  for (var i = 0; i < redzone_size; i += 8, ++shadow) {
    if (i + 8 <= size) {
      buffer[shadow] = 0; // fully addressable
    } else if (i >= size) {
      buffer[shadow] = value;
    } else {
      // first i bytes are addressable
      buffer[shadow] = partial ? size - i : 0;
    }
  }
});

EM_JS(void, emasan_intra_object_red_zone, (uptr ptr, uptr end, bool poison), {
  var shadow = Module._asan_shadow;
  if (ptr & 7) {
    shadow[ptr >> 3] = poison ? ptr & 7 : 0;
    ptr = (ptr | 7) + 1;
  }
  for (; ptr < end; ptr += 8) {
    // kAsanIntraObjectRedzone is 0xbb
    shadow[ptr >> 8] = poison ? 0xbb : 0;
  }
});

EM_JS(bool, emasan_is_poisoned, (uptr a), {
  var shadow_value = Module._asan_shadow[a >> 3];
  if (shadow_value) {
    var last_accessed_byte = a & 7;
    return last_accessed_byte >= shadow_value;
  }
  return false;
});

EM_JS(bool, emasan_check_poison, (uptr addr, uptr size), {
  var buffer = Module._asan_shadow;
  var s1 = buffer[addr >> 8];
  var s2 = size > 8 ? buffer[(addr >> 8) + 1] : 0;
  if (s1 || s2) {
    return size > 8 || (addr & 7) + size - 1 >= s1;
  } else {
    return 0;
  }
});

EM_JS(bool, emasan_range_good_aligned, (uptr begin, uptr end), {
  var buffer = Module._asan_shadow.subarray(begin >> 3, end >> 3);
  return buffer.every(function (a) { return !a; });
});

EM_JS(u8, emasan_shadow_read, (uptr addr), {
  return Module._asan_shadow[addr >> 3];
});

EM_JS(void, emasan_shadow_write, (uptr addr, u8 value), {
  Module._asan_shadow[addr >> 3] = value;
});

} // extern "C"

namespace __asan {

void InitializeShadowMemory() {
  emasan_poison_init();
}

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

#endif // SANITIZER_EMSCRIPTEN
