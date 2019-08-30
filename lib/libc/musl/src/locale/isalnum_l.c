#include <ctype.h>

int isalnum_l(int c, locale_t l)
{
	return isalnum(c);
}
