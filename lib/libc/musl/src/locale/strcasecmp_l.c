#include <strings.h>

int strcasecmp_l(const char *l, const char *r, locale_t loc)
{
	return strcasecmp(l, r);
}
