#include <ctype.h>

int isxdigit(int c)
{
	return isdigit(c) || ((unsigned)c|32)-'a' < 6;
}
