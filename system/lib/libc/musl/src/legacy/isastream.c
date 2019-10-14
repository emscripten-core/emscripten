#include <stropts.h>
#include <fcntl.h>

int isastream(int fd)
{
#ifdef __EMSCRIPTEN__
	__wasi_fdstat_t statbuf;
	int err = __wasi_fd_fdstat_get(fd, &statbuf);
	if (err != __WASI_ESUCCESS) {
		errno = err;
		return 0;
	}
	return 1;
#endif
	return fcntl(fd, F_GETFD) < 0 ? -1 : 0;
}
