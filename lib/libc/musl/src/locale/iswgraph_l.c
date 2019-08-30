#include <wctype.h>

int iswgraph_l(wint_t c, locale_t l)
{
	return iswgraph(c);
}
