#include <unistd.h>
#include <errno.h>

void __procfdname(char *, unsigned);

int fexecve(int fd, char *const argv[], char *const envp[])
{
	char buf[15 + 3*sizeof(int)];
	__procfdname(buf, fd);
	execve(buf, argv, envp);
	if (errno == ENOENT) errno = EBADF;
	return -1;
}
