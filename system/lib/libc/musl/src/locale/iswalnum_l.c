#include <wctype.h>

int iswalnum_l(wint_t c, locale_t l)
{
	return iswalnum(c);
}
