#define _GNU_SOURCE
#include <dlfcn.h>

int __dlinfo(void *, int, void *);

int dlinfo(void *dso, int req, void *res)
{
	return __dlinfo(dso, req, res);
}
