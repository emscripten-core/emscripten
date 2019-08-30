#include <ctype.h>

int ispunct_l(int c, locale_t l)
{
	return ispunct(c);
}
