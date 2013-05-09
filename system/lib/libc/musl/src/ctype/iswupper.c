#include <wctype.h>

int iswupper(wint_t wc)
{
	return towlower(wc) != wc;
}
