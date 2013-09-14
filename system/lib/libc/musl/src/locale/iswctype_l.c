#include <wctype.h>
#include "libc.h"

int iswctype_l(wint_t c, wctype_t t, locale_t l)
{
	return iswctype(c, t);
}

weak_alias(iswctype_l, __iswctype_l);
