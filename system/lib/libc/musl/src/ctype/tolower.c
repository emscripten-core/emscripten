#include <ctype.h>

int tolower(int c)
{
	if (isupper(c)) return c | 32;
	return c;
}
