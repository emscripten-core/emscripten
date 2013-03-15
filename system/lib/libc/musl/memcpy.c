#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define SS (sizeof(size_t))
#define ALIGN (sizeof(size_t)-1)
#define ONES ((size_t)-1/UCHAR_MAX)

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

	if (((uintptr_t)d & ALIGN) != ((uintptr_t)s & ALIGN))
		goto misaligned;

	for (; ((uintptr_t)d & ALIGN) && n; n--) *d++ = *s++;
	if (n) {
		size_t *wd = (void *)d;
		const size_t *ws = (const void *)s;

		for (; n>=SS; n-=SS) *wd++ = *ws++;
		d = (void *)wd;
		s = (const void *)ws;
misaligned:
		for (; n; n--) *d++ = *s++;
	}
	return dest;
}
