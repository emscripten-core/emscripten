#include <sys/stat.h>

int futimens(int fd, const struct timespec times[2])
{
#if __EMSCRIPTEN__
	#define AT_EMPTY_PATH 0x1000
	return utimensat(fd, 0, times, AT_EMPTY_PATH);
#else
	return utimensat(fd, 0, times, 0);
#endif
}
