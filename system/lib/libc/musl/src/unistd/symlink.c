#include <unistd.h>
#include "syscall.h"

int symlink(const char *existing, const char *new)
{
	return syscall(SYS_symlink, existing, new);
}
