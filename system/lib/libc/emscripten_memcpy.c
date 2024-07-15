/*
 * A simple memcpy optimized for wasm.
 */

#include <stdint.h>
#include <string.h>
#include <emscripten/emscripten.h>
#include "libc.h"
#include "emscripten_internal.h"

// Use the simple/naive version of memcpy when building with asan
#if defined(EMSCRIPTEN_OPTIMIZE_FOR_OZ) || __has_feature(address_sanitizer)

static void *__memcpy(void *dest, const void *src, size_t n) {
  unsigned char *d = (unsigned char *)dest;
  const unsigned char *s = (const unsigned char *)src;
#pragma clang loop unroll(disable)
  while(n--) *d++ = *s++;
  return dest;
}

#elif defined(__wasm_bulk_memory__)

static void *__memcpy(void *restrict dest, const void *restrict src, size_t n) {
  return _emscripten_memcpy_bulkmem(dest, src, n);
}

#else

static void *__memcpy(void *restrict dest, const void *restrict src, size_t n) {
  unsigned char *d = dest;
  const unsigned char *s = src;

  unsigned char *aligned_d_end;
  unsigned char *block_aligned_d_end;
  unsigned char *d_end;

#if !defined(EMSCRIPTEN_STANDALONE_WASM)
  if (n >= 512) {
    _emscripten_memcpy_js(dest, src, n);
    return dest;
  }
#endif

  d_end = d + n;
  if ((((uintptr_t)d) & 3) == (((uintptr_t)s) & 3)) {
    // The initial unaligned < 4-byte front.
    while ((((uintptr_t)d) & 3) && d < d_end) {
      *d++ = *s++;
    }
    aligned_d_end = (unsigned char *)(((uintptr_t)d_end) & -4);
    if (((uintptr_t)aligned_d_end) >= 64) {
      block_aligned_d_end = aligned_d_end - 64;
      while (d <= block_aligned_d_end) {
        // TODO: we could use 64-bit ops here, but we'd need to make sure the
        //       alignment is 64-bit, which might cost us
        *(((uint32_t*)d)) = *(((uint32_t*)s));
        *(((uint32_t*)d) + 1) = *(((uint32_t*)s) + 1);
        *(((uint32_t*)d) + 2) = *(((uint32_t*)s) + 2);
        *(((uint32_t*)d) + 3) = *(((uint32_t*)s) + 3);
        *(((uint32_t*)d) + 4) = *(((uint32_t*)s) + 4);
        *(((uint32_t*)d) + 5) = *(((uint32_t*)s) + 5);
        *(((uint32_t*)d) + 6) = *(((uint32_t*)s) + 6);
        *(((uint32_t*)d) + 7) = *(((uint32_t*)s) + 7);
        *(((uint32_t*)d) + 8) = *(((uint32_t*)s) + 8);
        *(((uint32_t*)d) + 9) = *(((uint32_t*)s) + 9);
        *(((uint32_t*)d) + 10) = *(((uint32_t*)s) + 10);
        *(((uint32_t*)d) + 11) = *(((uint32_t*)s) + 11);
        *(((uint32_t*)d) + 12) = *(((uint32_t*)s) + 12);
        *(((uint32_t*)d) + 13) = *(((uint32_t*)s) + 13);
        *(((uint32_t*)d) + 14) = *(((uint32_t*)s) + 14);
        *(((uint32_t*)d) + 15) = *(((uint32_t*)s) + 15);
        d += 64;
        s += 64;
      }
    }
    while (d < aligned_d_end) {
      *((uint32_t *)d) = *((uint32_t *)s);
      d += 4;
      s += 4;
    }
  } else {
    // In the unaligned copy case, unroll a bit as well.
    if (((uintptr_t)d_end) >= 4) {
      aligned_d_end = d_end - 4;
      while (d <= aligned_d_end) {
        *d = *s;
        *(d + 1) = *(s + 1);
        *(d + 2) = *(s + 2);
        *(d + 3) = *(s + 3);
        d += 4;
        s += 4;
      }
    }
  }
  // The remaining unaligned < 4 byte tail.
  while (d < d_end) {
    *d++ = *s++;
  }
  return dest;
}

#endif

weak_alias(__memcpy, emscripten_builtin_memcpy);
weak_alias(__memcpy, memcpy);
