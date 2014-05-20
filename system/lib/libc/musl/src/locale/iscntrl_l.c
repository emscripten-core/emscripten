#include <ctype.h>

int iscntrl_l(int c, locale_t l)
{
	return iscntrl(c);
}
