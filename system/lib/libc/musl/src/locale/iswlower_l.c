#include <wctype.h>

int iswlower_l(wint_t c, locale_t l)
{
	return iswlower(c);
}
