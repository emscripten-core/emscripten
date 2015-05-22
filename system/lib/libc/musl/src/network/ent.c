#include "libc.h"

void sethostent(int x)
{
}

void *gethostent()
{
	return 0;
}

void endhostent(void)
{
}

weak_alias(sethostent, setnetent);
weak_alias(gethostent, getnetent);
weak_alias(endhostent, endnetent);
