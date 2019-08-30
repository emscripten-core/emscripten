#include <wctype.h>

wint_t towctrans_l(wint_t c, wctrans_t t, locale_t l)
{
	return towctrans(c, t);
}
