#include <stdio.h>

int func1()
{
	throw 1;
}

typedef int (*foo)(void);

int main()
{
	foo f = func1;
	try {
		f();
	} catch(...)
	{
	}
}
