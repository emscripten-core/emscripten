#include <wctype.h>

int iswlower(wint_t wc)
{
	return towupper(wc) != wc || wc == 0xdf;
}
