#include <ctype.h>

int iscntrl(int c)
{
	return (unsigned)c < 0x20 || c == 0x7f;
}
