#define _GNU_SOURCE
#include <unistd.h>
#include <time.h>
#ifdef __EMSCRTIPEN__
#include <emscripten/threading.h>
#endif

int usleep(unsigned useconds)
{
#ifdef __EMSCRTIPEN__
	emscripten_thread_sleep(usec / 1e3);
	return 0;
#else
	struct timespec tv = {
		.tv_sec = useconds/1000000,
		.tv_nsec = (useconds%1000000)*1000
	};
	return nanosleep(&tv, &tv);
#endif
}
