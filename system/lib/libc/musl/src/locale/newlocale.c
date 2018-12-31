#include <stdlib.h>
#include <string.h>
#include "locale_impl.h"
#include "libc.h"

int __loc_is_allocated(locale_t loc)
{
	return loc && loc != C_LOCALE && loc != UTF8_LOCALE;
}

locale_t __newlocale(int mask, const char *name, locale_t loc)
{
	int i, j;
	struct __locale_struct tmp;
	const struct __locale_map *lm;

	/* For locales with allocated storage, modify in-place. */
	if (__loc_is_allocated(loc)) {
		for (i=0; i<LC_ALL; i++)
			if (mask & (1<<i))
				loc->cat[i] = __get_locale(i, name);
		return loc;
	}

	/* Otherwise, build a temporary locale object, which will only
	 * be instantiated in allocated storage if it does not match
	 * one of the built-in static locales. This makes the common
	 * usage case for newlocale, getting a C locale with predictable
	 * behavior, very fast, and more importantly, fail-safe. */
	for (j=i=0; i<LC_ALL; i++) {
		if (loc && !(mask & (1<<i)))
			lm = loc->cat[i];
		else
			lm = __get_locale(i, mask & (1<<i) ? name : "");
		if (lm) j++;
		tmp.cat[i] = lm;
	}

	if (!j)
		return C_LOCALE;
	if (j==1 && tmp.cat[LC_CTYPE]==&__c_dot_utf8)
		return UTF8_LOCALE;

	return loc;
}

weak_alias(__newlocale, newlocale);
