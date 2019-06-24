#include <string.h>
#include <stdint.h>

#define WT size_t
#define WS (sizeof(WT))

// XXX EMSCRIPTEN ASAN: build an uninstrumented version of memmove
#if defined(__EMSCRIPTEN__) && defined(__has_feature)
#if __has_feature(address_sanitizer)
#define memmove __attribute__((no_sanitize("address"))) emscripten_builtin_memmove
#endif
#endif

void *memmove(void *dest, const void *src, size_t n)
{
	char *d = dest;
	const char *s = src;

	if (d==s) return d;
	if (s+n <= d || d+n <= s) return memcpy(d, s, n);

	if (d<s) {
		if ((uintptr_t)s % WS == (uintptr_t)d % WS) {
			while ((uintptr_t)d % WS) {
				if (!n--) return dest;
				*d++ = *s++;
			}
			for (; n>=WS; n-=WS, d+=WS, s+=WS) *(WT *)d = *(WT *)s;
		}
		for (; n; n--) *d++ = *s++;
	} else {
		if ((uintptr_t)s % WS == (uintptr_t)d % WS) {
			while ((uintptr_t)(d+n) % WS) {
				if (!n--) return dest;
				d[n] = s[n];
			}
			while (n>=WS) n-=WS, *(WT *)(d+n) = *(WT *)(s+n);
		}
		while (n) n--, d[n] = s[n];
	}

	return dest;
}
