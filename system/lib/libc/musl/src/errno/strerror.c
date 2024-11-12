#include <errno.h>
#include <stddef.h>
#include <string.h>
#include "locale_impl.h"

/* mips has one error code outside of the 8-bit range due to a
 * historical typo, so we just remap it. */
#if EDQUOT==1133
#define EDQUOT_ORIG 1133
#undef  EDQUOT
#define EDQUOT 109
#endif

static const struct errmsgstr_t {
#define E(n, s) char str##n[sizeof(s)];
#include "__strerror.h"
#undef E
} errmsgstr = {
#define E(n, s) s,
#include "__strerror.h"
#undef E
};

static const unsigned short errmsgidx[] = {
#define E(n, s) [n] = offsetof(struct errmsgstr_t, str##n),
#include "__strerror.h"
#undef E
};

char *__strerror_l(int e, locale_t loc)
{
	const char *s;
#ifdef EDQUOT_ORIG
	if (e==EDQUOT) e=0;
	else if (e==EDQUOT_ORIG) e=EDQUOT;
#endif
	if (e >= sizeof errmsgidx / sizeof *errmsgidx) e = 0;
	s = (char *)&errmsgstr + errmsgidx[e];
#ifdef __EMSCRIPTEN__
	// strerror is a (debug) dependency of many emscripten syscalls which mean it
	// must be excluded from LTO, along with all of its dependencies.
	// In order to limit the transitive dependencies we disable localization of
	// rrno messages here.
	return (char *)s;
#else
	return (char *)LCTRANS(s, LC_MESSAGES, loc);
#endif
}

char *strerror(int e)
{
#ifdef __EMSCRIPTEN__
	return __strerror_l(e, NULL);
#else
	return __strerror_l(e, CURRENT_LOCALE);
#endif
}

weak_alias(__strerror_l, strerror_l);
