#include <dirent.h>
#include <unistd.h>
#include "__dirent.h"
#include "lock.h"

void rewinddir(DIR *dir)
{
	LOCK(dir->lock);
#if !defined(__EMSCRIPTEN__)
	lseek(dir->fd, 0, SEEK_SET);
#endif
	dir->buf_pos = dir->buf_end = 0;
	dir->tell = 0;
	UNLOCK(dir->lock);
}
