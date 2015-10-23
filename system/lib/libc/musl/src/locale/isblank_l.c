#include <ctype.h>

int isblank_l(int c, locale_t l)
{
	return isblank(c);
}
