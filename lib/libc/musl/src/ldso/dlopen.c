#include <dlfcn.h>
#include "libc.h"

__attribute__((__visibility__("hidden")))
void __dl_seterr(const char *, ...);

static void *stub_dlopen(const char *file, int mode)
{
	__dl_seterr("Dynamic loading not supported");
	return 0;
}

weak_alias(stub_dlopen, dlopen);
