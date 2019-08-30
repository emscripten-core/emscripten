#include <wctype.h>

int iswspace_l(wint_t c, locale_t l)
{
	return iswspace(c);
}
