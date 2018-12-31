#include <ctype.h>

int toupper_l(int c, locale_t l)
{
	return toupper(c);
}
