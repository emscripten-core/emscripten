#define _GNU_SOURCE
#include <dlfcn.h>

int __dladdr(const void *, Dl_info *);

int dladdr(const void *addr, Dl_info *info)
{
	return __dladdr(addr, info);
}
