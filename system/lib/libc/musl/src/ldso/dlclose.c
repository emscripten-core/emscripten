#include <dlfcn.h>

__attribute__((__visibility__("hidden")))
int __dl_invalid_handle(void *);

int dlclose(void *p)
{
	return __dl_invalid_handle(p);
}
