#include <ctype.h>

int isupper_l(int c, locale_t l)
{
	return isupper(c);
}
