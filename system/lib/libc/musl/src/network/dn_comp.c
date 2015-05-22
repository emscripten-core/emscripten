#include <resolv.h>
#include "libc.h"

int __dn_comp(const char *src, unsigned char *dst, int space, unsigned char **dnptrs, unsigned char **lastdnptr)
{
	return -1;
}

weak_alias(__dn_comp, dn_comp);
