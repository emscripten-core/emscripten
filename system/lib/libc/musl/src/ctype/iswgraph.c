#include <wctype.h>

int iswgraph(wint_t wc)
{
	/* ISO C defines this function as: */
	return !iswspace(wc) && iswprint(wc);
}
