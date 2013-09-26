#include <wchar.h>

size_t wcsxfrm_l(wchar_t *restrict dest, const wchar_t *restrict src, size_t n, locale_t locale)
{
	return wcsxfrm(dest, src, n);
}
