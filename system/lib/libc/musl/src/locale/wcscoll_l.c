#include <wchar.h>

int wcscoll_l(const wchar_t *l, const wchar_t *r, locale_t locale)
{
	return wcscoll(l, r);
}
