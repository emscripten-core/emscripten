#define _GNU_SOURCE
#include <unistd.h>
#include "syscall.h"

void syncfs(int fd)
{
	__syscall(SYS_syncfs, fd);
}
