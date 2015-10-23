#include <ctype.h>

int isspace_l(int c, locale_t l)
{
	return isspace(c);
}
