#include <wctype.h>

int iswprint_l(wint_t c, locale_t l)
{
	return iswprint(c);
}
