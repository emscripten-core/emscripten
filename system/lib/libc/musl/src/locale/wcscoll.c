#include <wchar.h>
#include <locale.h>
#include "libc.h"

/* FIXME: stub */
int __wcscoll_l(const wchar_t *l, const wchar_t *r, locale_t locale)
{
	return wcscmp(l, r);
}

int wcscoll(const wchar_t *l, const wchar_t *r)
{
	return __wcscoll_l(l, r, 0);
}

weak_alias(__wcscoll_l, wcscoll_l);
