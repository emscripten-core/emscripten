#define _GNU_SOURCE
#include <netdb.h>

struct servent *getservbyname(const char *name, const char *prots)
{
	static struct servent se;
	static long buf[32/sizeof(long)];
	struct servent *res;
	if (getservbyname_r(name, prots, &se, (void *)buf, sizeof buf, &res))
		return 0;
	return &se;
}
