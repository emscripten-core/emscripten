#include <stdlib.h>
#include <ctype.h>

unsigned long long strtoull_l(const char *restrict s, char **restrict p, int base, locale_t loc)
{
	return strtoull(s, p, base);
}

long long strtoll_l(const char *restrict s, char **restrict p, int base, locale_t loc)
{
	return strtoll(s, p, base);
}

unsigned long strtoul_l(const char *restrict s, char **restrict p, int base, locale_t loc)
{
	return strtoul(s, p, base);
}

long strtol_l(const char *restrict s, char **restrict p, int base, locale_t loc)
{
	return strtol(s, p, base);
}
