#include <dirent.h>
#include <unistd.h>
#include "__dirent.h"
#include "lock.h"

void seekdir(DIR *dir, long off)
{
	LOCK(dir->lock);
	dir->tell = lseek(dir->fd, off, SEEK_SET);
#if defined(__EMSCRIPTEN__)
	// The above relies on `lseek` to work on a directory fd, which is not
	// guaranteed on WASI. Just set `off` again if the above failed.
	dir->tell = off;
#endif
	dir->buf_pos = dir->buf_end = 0;
	UNLOCK(dir->lock);
}
