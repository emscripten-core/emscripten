#include <unistd.h>
#include "syscall.h"

int link(const char *existing, const char *new)
{
	return syscall(SYS_link, existing, new);
}
