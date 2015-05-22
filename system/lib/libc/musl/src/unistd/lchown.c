#include <unistd.h>
#include "syscall.h"

int lchown(const char *path, uid_t uid, gid_t gid)
{
	return syscall(SYS_lchown, path, uid, gid);
}
