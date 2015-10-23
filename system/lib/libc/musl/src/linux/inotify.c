#include <sys/inotify.h>
#include "syscall.h"

int inotify_init()
{
	return syscall(SYS_inotify_init);
}
int inotify_init1(int flags)
{
	return syscall(SYS_inotify_init1, flags);
}

int inotify_add_watch(int fd, const char *pathname, uint32_t mask)
{
	return syscall(SYS_inotify_add_watch, fd, pathname, mask);
}

int inotify_rm_watch(int fd, int wd)
{
	return syscall(SYS_inotify_rm_watch, fd, wd);
}
