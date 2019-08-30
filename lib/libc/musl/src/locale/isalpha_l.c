#include <ctype.h>

int isalpha_l(int c, locale_t l)
{
	return isalpha(c);
}
