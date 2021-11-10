#include "stdio_impl.h"
#include "intscan.h"
#include "shgetc.h"
#include <inttypes.h>
#include <limits.h>
#include <ctype.h>

#ifdef __EMSCRIPTEN__
#include <errno.h>

// Loosely based on __intscan but simplified, and optimized for size
// - Doesn't use FILE or getc/ungetc, just operates directly on memory
// - Avoids lookup table
// - Avoids special cases loops for certain bases
// - Skips an early exit with EINVAL when char 0 is greater than base.  Its not
//   clear this was correct, and glibc seems not do this either.
static unsigned long long strtox(const char *s, char **p, int base, unsigned long long lim) {
	int neg=0;
	unsigned long long y=0;
	const char* orig = s;

	if (base > 36) {
		errno = EINVAL;
		return 0;
	}

	while (*s && isspace(*s)) { s++; };

	// Handle sign
	if (*s=='+' || *s=='-') {
		neg = -(*s=='-');
		s++;
	}

	int found_digit = 0;

	// Handle hex/octal prefix 0x/00
	if ((base == 0 || base == 16) && *s=='0') {
		found_digit = 1;
		s++;
		if ((*s|32)=='x') {
			s++;
			base = 16;
		} else if (base == 0) {
			base = 8;
		}
	} else if (base == 0) {
		base = 10;
	}

	int val;
	int overflow = 0;
	for (y=0; ; s++) {
		if ('0' <= *s && *s <= '9') val = *s -'0';
		else if ('a' <= *s && *s <= 'z') val = 10 + *s -'a';
		else if ('A' <= *s && *s <= 'Z') val = 10 + *s -'A';
		else break;
		if (val>=base) break;
		if (y > ULLONG_MAX/base || (base*y>ULLONG_MAX-val)) {
			overflow = 1;
			continue;
		}
		found_digit = 1;
		y = y*base + val;
	}
	if (p) {
		if (found_digit) {
			*p = (char*)s;
		} else {
			*p = (char*)orig;
		}
	}
	if (overflow) {
		// We exit'd the above loop due to overflow
		errno = ERANGE;
		y = lim;
		if (lim&1) neg = 0;
	}
	if (y>=lim) {
		if (!(lim&1) && !neg) {
			errno = ERANGE;
			return lim-1;
		} else if (y>lim) {
			errno = ERANGE;
			return lim;
		}
	}
	return (y^neg)-neg;
}
#else
static unsigned long long strtox(const char *s, char **p, int base, unsigned long long lim)
{
	FILE f;
	sh_fromstring(&f, s);
	shlim(&f, 0);
	unsigned long long y = __intscan(&f, base, 1, lim);
	if (p) {
		size_t cnt = shcnt(&f);
		*p = (char *)s + cnt;
	}
	return y;
}
#endif

unsigned long long strtoull(const char *restrict s, char **restrict p, int base)
{
	return strtox(s, p, base, ULLONG_MAX);
}

long long strtoll(const char *restrict s, char **restrict p, int base)
{
	return strtox(s, p, base, LLONG_MIN);
}

unsigned long strtoul(const char *restrict s, char **restrict p, int base)
{
	return strtox(s, p, base, ULONG_MAX);
}

long strtol(const char *restrict s, char **restrict p, int base)
{
	return strtox(s, p, base, 0UL+LONG_MIN);
}

intmax_t strtoimax(const char *restrict s, char **restrict p, int base)
{
	return strtoll(s, p, base);
}

uintmax_t strtoumax(const char *restrict s, char **restrict p, int base)
{
	return strtoull(s, p, base);
}

weak_alias(strtol, __strtol_internal);
weak_alias(strtoul, __strtoul_internal);
weak_alias(strtoll, __strtoll_internal);
weak_alias(strtoull, __strtoull_internal);
weak_alias(strtoimax, __strtoimax_internal);
weak_alias(strtoumax, __strtoumax_internal);
