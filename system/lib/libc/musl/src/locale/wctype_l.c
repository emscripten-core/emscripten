#include <wctype.h>
#include "libc.h"

wctype_t wctype_l(const char *s, locale_t l)
{
	return wctype(s);
}

weak_alias(wctype_l, __wctype_l);
