#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include "syscall.h"

char *ctermid(char *s)
{
	static char s2[L_ctermid];
	int fd;
	if (!s) s = s2;
	*s = 0;
	fd = open("/dev/tty", O_WRONLY | O_NOCTTY | O_CLOEXEC);
	if (fd >= 0) {
		ttyname_r(fd, s, L_ctermid);
		__syscall(SYS_close, fd);
	}
	return s;
}
