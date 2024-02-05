#if __EMSCRIPTEN__
#include <stdint.h>
#endif
#include <string.h>

int memcmp(const void *vl, const void *vr, size_t n)
{
	const unsigned char *l=vl, *r=vr;

// XXX EMSCRIPTEN: add an optimized version.
#if !defined(EMSCRIPTEN_OPTIMIZE_FOR_OZ) && !__has_feature(address_sanitizer)
	// If we have enough bytes, and everything is aligned, loop on words instead
	// of single bytes.
	if (n >= 4 && !((((uintptr_t)l) & 3) | (((uintptr_t)r) & 3))) {
		while (n >= 4) {
			if (*((uint32_t *)l) != *((uint32_t *)r)) {
				// Go to the single-byte loop to find the specific byte.
				break;
			}
			l += 4;
			r += 4;
			n -= 4;
		}
	}
#endif

#if defined(EMSCRIPTEN_OPTIMIZE_FOR_OZ)
#pragma clang loop unroll(disable)
#endif
	for (; n && *l == *r; n--, l++, r++);
	return n ? *l-*r : 0;
}
