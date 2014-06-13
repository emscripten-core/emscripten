#include <ctype.h>

int isxdigit_l(int c, locale_t l)
{
	return isxdigit(c);
}
