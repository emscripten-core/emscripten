#include <strings.h>
#include <locale.h>

int strncasecmp_l(const char *l, const char *r, size_t n, locale_t loc)
{
	return strncasecmp(l, r, n);
}
