#include <wctype.h>

int iswcntrl_l(wint_t c, locale_t l)
{
	return iswcntrl(c);
}
