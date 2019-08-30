#include <dlfcn.h>

void *__dlsym(void *restrict, const char *restrict, void *restrict);

void *dlsym(void *restrict p, const char *restrict s)
{
	return __dlsym(p, s, 0);
}
