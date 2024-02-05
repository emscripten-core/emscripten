#include <ctype.h>

char *strupr(char *str)
{
	char *ret = str;
	while(*str)
	{
		*str = toupper(*str);
		++str;
	}
	return ret;
}
