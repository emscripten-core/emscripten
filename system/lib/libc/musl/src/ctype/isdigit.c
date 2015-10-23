#include <ctype.h>
#undef isdigit

int isdigit(int c)
{
	return (unsigned)c-'0' < 10;
}
