#ifdef SHARED

#include "version.h"

static const char version[] = VERSION;

const char *__libc_get_version()
{
	return version;
}

#endif
