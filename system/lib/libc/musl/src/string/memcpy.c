#include <string.h>
#include <stdint.h>
#include <endian.h>

#ifdef __EMSCRIPTEN__
// An external JS implementation that is efficient for very large copies, using
// HEAPU8.set()
extern void *emscripten_memcpy_big(void *restrict dest, const void *restrict src, size_t n);
#endif

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

#ifdef __EMSCRIPTEN__

	unsigned char *aligned_d_end;
	unsigned char *block_aligned_d_end;
	unsigned char *d_end;

	if (n >= 8192) {
		return emscripten_memcpy_big(dest, src, n);
	}

	d_end = d + n;
	if ((((uintptr_t)d) & 3) == (((uintptr_t)s) & 3)) {
		// The initial unaligned < 4-byte front.
		while ((((uintptr_t)d) & 3) && d < d_end) {
			*d++ = *s++;
		}
		aligned_d_end = (unsigned char *)(((uintptr_t)d_end) & -4);
		if (aligned_d_end >= 64) {
			block_aligned_d_end = aligned_d_end - 64;
			while (d <= block_aligned_d_end) {
				*(((uint64_t*)d)) = *(((uint64_t*)s));
				*(((uint64_t*)d) + 1) = *(((uint64_t*)s) + 1);
				*(((uint64_t*)d) + 2) = *(((uint64_t*)s) + 2);
				*(((uint64_t*)d) + 3) = *(((uint64_t*)s) + 3);
				*(((uint64_t*)d) + 4) = *(((uint64_t*)s) + 4);
				*(((uint64_t*)d) + 5) = *(((uint64_t*)s) + 5);
				*(((uint64_t*)d) + 6) = *(((uint64_t*)s) + 6);
				*(((uint64_t*)d) + 7) = *(((uint64_t*)s) + 7);
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
		if (d_end >= 4) {
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

#else // __EMSCRIPTEN__

#ifdef __GNUC__

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define LS >>
#define RS <<
#else
#define LS <<
#define RS >>
#endif

	typedef uint32_t __attribute__((__may_alias__)) u32;
	uint32_t w, x;

	for (; (uintptr_t)s % 4 && n; n--) *d++ = *s++;

	if ((uintptr_t)d % 4 == 0) {
		for (; n>=16; s+=16, d+=16, n-=16) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			*(u32 *)(d+4) = *(u32 *)(s+4);
			*(u32 *)(d+8) = *(u32 *)(s+8);
			*(u32 *)(d+12) = *(u32 *)(s+12);
		}
		if (n&8) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			*(u32 *)(d+4) = *(u32 *)(s+4);
			d += 8; s += 8;
		}
		if (n&4) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			d += 4; s += 4;
		}
		if (n&2) {
			*d++ = *s++; *d++ = *s++;
		}
		if (n&1) {
			*d = *s;
		}
		return dest;
	}

	if (n >= 32) switch ((uintptr_t)d % 4) {
	case 1:
		w = *(u32 *)s;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		n -= 3;
		for (; n>=17; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+1);
			*(u32 *)(d+0) = (w LS 24) | (x RS 8);
			w = *(u32 *)(s+5);
			*(u32 *)(d+4) = (x LS 24) | (w RS 8);
			x = *(u32 *)(s+9);
			*(u32 *)(d+8) = (w LS 24) | (x RS 8);
			w = *(u32 *)(s+13);
			*(u32 *)(d+12) = (x LS 24) | (w RS 8);
		}
		break;
	case 2:
		w = *(u32 *)s;
		*d++ = *s++;
		*d++ = *s++;
		n -= 2;
		for (; n>=18; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+2);
			*(u32 *)(d+0) = (w LS 16) | (x RS 16);
			w = *(u32 *)(s+6);
			*(u32 *)(d+4) = (x LS 16) | (w RS 16);
			x = *(u32 *)(s+10);
			*(u32 *)(d+8) = (w LS 16) | (x RS 16);
			w = *(u32 *)(s+14);
			*(u32 *)(d+12) = (x LS 16) | (w RS 16);
		}
		break;
	case 3:
		w = *(u32 *)s;
		*d++ = *s++;
		n -= 1;
		for (; n>=19; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+3);
			*(u32 *)(d+0) = (w LS 8) | (x RS 24);
			w = *(u32 *)(s+7);
			*(u32 *)(d+4) = (x LS 8) | (w RS 24);
			x = *(u32 *)(s+11);
			*(u32 *)(d+8) = (w LS 8) | (x RS 24);
			w = *(u32 *)(s+15);
			*(u32 *)(d+12) = (x LS 8) | (w RS 24);
		}
		break;
	}
	if (n&16) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&8) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&4) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&2) {
		*d++ = *s++; *d++ = *s++;
	}
	if (n&1) {
		*d = *s;
	}
	return dest;
#endif

	for (; n; n--) *d++ = *s++;
	return dest;

#endif // __EMSCRIPTEN__
}
