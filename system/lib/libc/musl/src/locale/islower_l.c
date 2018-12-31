#include <ctype.h>

int islower_l(int c, locale_t l)
{
	return islower(c);
}
