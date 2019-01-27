#include <stdio.h>

extern "C" int foo(void);

int main()
{
	printf("%d\n", foo());
}
