#include <wctype.h>

int iswalpha_l(wint_t c, locale_t l)
{
	return iswalpha(c);
}
