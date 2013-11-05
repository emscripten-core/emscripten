#include <wctype.h>
#include "libc.h"

wint_t towlower_l(wint_t c, locale_t l)
{
	return towlower(c);
}

weak_alias(towlower_l, __towlower_l);
