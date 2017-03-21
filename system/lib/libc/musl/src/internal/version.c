#include "version.h"

static const char version[] = VERSION;

__attribute__((__visibility__("hidden")))
const char *__libc_get_version()
{
	return version;
}
