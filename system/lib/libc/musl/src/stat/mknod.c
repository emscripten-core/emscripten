#include <sys/stat.h>
#include "syscall.h"

int mknod(const char *path, mode_t mode, dev_t dev)
{
	return syscall(SYS_mknod, path, mode, dev);
}
