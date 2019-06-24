#include <string.h>

char *__stpcpy(char *, const char *);

#define USE_FAST_COPY 1

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#undef USE_FAST_COPY
#define USE_FAST_COPY 0
#endif
#endif

char *strcpy(char *restrict dest, const char *restrict src)
{
#if USE_FAST_COPY
	__stpcpy(dest, src);
	return dest;
#else
	const unsigned char *s = src;
	unsigned char *d = dest;
	while ((*d++ = *s++));
	return dest;
#endif
}
