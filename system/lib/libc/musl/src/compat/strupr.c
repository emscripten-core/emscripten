#include <ctype.h>

void strupr(char *str)
{
	while(*str)
	{
		*str = toupper(*str);
		++str;
	}
}
