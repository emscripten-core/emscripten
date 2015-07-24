#include <unistd.h>
#include "syscall.h"

int fsync(int fd)
{
	return syscall(SYS_fsync, fd);
}
