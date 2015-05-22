#include <spawn.h>

int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags)
{
	attr->__flags = flags;
	return 0;
}
