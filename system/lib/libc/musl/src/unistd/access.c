#include <unistd.h>
#include "syscall.h"

int access(const char *filename, int amode)
{
	return syscall(SYS_access, filename, amode);
}
