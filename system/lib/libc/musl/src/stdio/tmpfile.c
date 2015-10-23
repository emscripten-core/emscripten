#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "stdio_impl.h"

#define MAXTRIES 100

FILE *tmpfile(void)
{
	char buf[L_tmpnam], *s;
	int fd;
	FILE *f;
	int try;
	for (try=0; try<MAXTRIES; try++) {
		s = tmpnam(buf);
		if (!s) return 0;
		fd = syscall(SYS_open, s, O_RDWR|O_CREAT|O_EXCL|O_LARGEFILE, 0600);
		if (fd >= 0) {
			f = __fdopen(fd, "w+");
			__syscall(SYS_unlink, s);
			return f;
		}
	}
	return 0;
}

LFS64(tmpfile);
