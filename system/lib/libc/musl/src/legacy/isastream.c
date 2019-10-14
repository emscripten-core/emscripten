#include <stropts.h>
#include <fcntl.h>

int isastream(int fd)
{
#ifdef __EMSCRIPTEN__
	return __wasi_fd_is_valid(fd);
#else
	return fcntl(fd, F_GETFD) < 0 ? -1 : 0;
#endif
}
