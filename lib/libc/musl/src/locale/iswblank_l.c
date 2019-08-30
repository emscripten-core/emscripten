#include <wctype.h>

int iswblank_l(wint_t c, locale_t l)
{
	return iswblank(c);
}
