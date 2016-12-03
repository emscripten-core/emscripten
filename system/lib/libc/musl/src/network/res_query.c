#include <resolv.h>
#include <netdb.h>
#include "libc.h"

int __res_mkquery(int, const char *, int, int, const unsigned char *, int, const unsigned char*, unsigned char *, int);
int __res_send(const unsigned char *, int, unsigned char *, int);

int __res_query(const char *name, int class, int type, unsigned char *dest, int len)
{
	unsigned char q[280];
	int ql = __res_mkquery(0, name, class, type, 0, 0, 0, q, sizeof q);
	if (ql < 0) return ql;
	return __res_send(q, ql, dest, len);
}

weak_alias(__res_query, res_query);
weak_alias(__res_query, res_search);
