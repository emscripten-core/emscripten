#include <sys/sendfile.h>
#include "syscall.h"
#include "libc.h"

ssize_t sendfile(int out_fd, int in_fd, off_t *ofs, size_t count)
{
	return syscall(SYS_sendfile, out_fd, in_fd, ofs, count);
}

LFS64(sendfile);
