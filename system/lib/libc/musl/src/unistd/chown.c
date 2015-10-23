#include <unistd.h>
#include "syscall.h"

int chown(const char *path, uid_t uid, gid_t gid)
{
	return syscall(SYS_chown, path, uid, gid);
}
