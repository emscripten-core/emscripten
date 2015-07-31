#include <fcntl.h>
#include "libc.h"

int creat(const char *filename, mode_t mode)
{
	return open(filename, O_CREAT|O_WRONLY|O_TRUNC, mode);
}

LFS64(creat);
