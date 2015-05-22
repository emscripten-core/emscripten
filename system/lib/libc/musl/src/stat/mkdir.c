#include <sys/stat.h>
#include "syscall.h"

int mkdir(const char *path, mode_t mode)
{
	return syscall(SYS_mkdir, path, mode);
}
