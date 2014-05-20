#include <ctype.h>

void strlwr(char *str)
{
	while(*str)
	{
		*str = tolower(*str);
		++str;
	}
}
