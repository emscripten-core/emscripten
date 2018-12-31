#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "locale_impl.h"
#include "libc.h"
#include "atomic.h"

static char buf[LC_ALL*(LOCALE_NAME_MAX+1)];

static char *setlocale_one_unlocked(int cat, const char *name)
{
	const struct __locale_map *lm;

	if (name) libc.global_locale.cat[cat] = lm = __get_locale(cat, name);
	else lm = libc.global_locale.cat[cat];

	return lm ? (char *)lm->name : "C";
}

char *__strchrnul(const char *, int);

char *setlocale(int cat, const char *name)
{
	static volatile int lock[2];

	if ((unsigned)cat > LC_ALL) return 0;

	LOCK(lock);

	/* For LC_ALL, setlocale is required to return a string which
	 * encodes the current setting for all categories. The format of
	 * this string is unspecified, and only the following code, which
	 * performs both the serialization and deserialization, depends
	 * on the format, so it can easily be changed if needed. */
	if (cat == LC_ALL) {
		int i;
		if (name) {
			char part[LOCALE_NAME_MAX+1] = "C.UTF-8";
			const char *p = name;
			for (i=0; i<LC_ALL; i++) {
				const char *z = __strchrnul(p, ';');
				if (z-p <= LOCALE_NAME_MAX) {
					memcpy(part, p, z-p);
					part[z-p] = 0;
					if (*z) p = z+1;
				}
				setlocale_one_unlocked(i, part);
			}
		}
		char *s = buf;
		for (i=0; i<LC_ALL; i++) {
			const struct __locale_map *lm =
				libc.global_locale.cat[i];
			const char *part = lm ? lm->name : "C";
			size_t l = strlen(part);
			memcpy(s, part, l);
			s[l] = ';';
			s += l+1;
		}
		*--s = 0;
		UNLOCK(lock);
		return buf;
	}

	char *ret = setlocale_one_unlocked(cat, name);

	UNLOCK(lock);

	return ret;
}
