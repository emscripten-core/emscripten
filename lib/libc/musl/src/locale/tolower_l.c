#include <ctype.h>

int tolower_l(int c, locale_t l)
{
	return tolower(c);
}
