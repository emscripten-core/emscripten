#include <wchar.h>
#include <wctype.h>

int iswalnum(wint_t wc)
{
	return iswdigit(wc) || iswalpha(wc);
}
