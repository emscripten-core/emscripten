#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "syscall.h"

void *__mmap(void *, size_t, int, int, int, off_t);

const char unsigned *__map_file(const char *pathname, size_t *size)
{
	struct stat st;
	const unsigned char *map = MAP_FAILED;
	int flags = O_RDONLY|O_LARGEFILE|O_CLOEXEC|O_NONBLOCK;
	int fd = __syscall(SYS_open, pathname, flags);
	if (fd < 0) return 0;
	if (!__syscall(SYS_fstat, fd, &st))
		map = __mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	__syscall(SYS_close, fd);
	*size = st.st_size;
	return map == MAP_FAILED ? 0 : map;
}
