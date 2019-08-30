#include <wctype.h>
#include "libc.h"

wint_t towupper_l(wint_t c, locale_t l)
{
	return towupper(c);
}

weak_alias(towupper_l, __towupper_l);
