#include <stdio.h>
#include <wchar.h>

wint_t btowc(int c)
{
	return c<128U ? c : EOF;
}
