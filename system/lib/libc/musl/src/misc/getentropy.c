#define _BSD_SOURCE
#include <unistd.h>
#include <sys/random.h>
#include <pthread.h>
#include <errno.h>

#ifdef __EMSCRIPTEN__
#include <wasi/wasi-helpers.h>
#endif

int getentropy(void *buffer, size_t len)
{
	int cs, ret = 0;
	char *pos = buffer;

	if (len > 256) {
		errno = EIO;
		return -1;
	}

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

#ifdef __EMSCRIPTEN__
	ret = __wasi_syscall_ret(__wasi_random_get(buffer, len));
#else
	while (len) {
		ret = getrandom(pos, len, 0);
		if (ret < 0) {
			if (errno == EINTR) continue;
			else break;
		}
		pos += ret;
		len -= ret;
		ret = 0;
	}
#endif

	pthread_setcancelstate(cs, 0);

	return ret;
}
