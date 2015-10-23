#include <locale.h>

char *setlocale(int category, const char *locale)
{
	/* Note: plain "C" would be better, but puts some broken
	 * software into legacy 8-bit-codepage mode, ignoring
	 * the standard library's multibyte encoding */
	return "C.UTF-8";
}
