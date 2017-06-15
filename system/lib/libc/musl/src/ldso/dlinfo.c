#define _GNU_SOURCE
#include <dlfcn.h>

__attribute__((__visibility__("hidden")))
int __dl_invalid_handle(void *);

__attribute__((__visibility__("hidden")))
void __dl_seterr(const char *, ...);

int dlinfo(void *dso, int req, void *res)
{
	if (__dl_invalid_handle(dso)) return -1;
	if (req != RTLD_DI_LINKMAP) {
		__dl_seterr("Unsupported request %d", req);
		return -1;
	}
	*(struct link_map **)res = dso;
	return 0;
}
