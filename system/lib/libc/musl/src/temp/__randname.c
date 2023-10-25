#include <time.h>
#include <stdint.h>
#include "pthread_impl.h"

/* This assumes that a check for the
   template size has already been made */
char *__randname(char *template)
{
	int i;
	struct timespec ts;
	unsigned long r;

	__clock_gettime(CLOCK_REALTIME, &ts);
	r = ts.tv_sec + ts.tv_nsec + __pthread_self()->tid * 65537UL;

	/* XXX EMSCRIPTEN: avoid repeating the same result when __clock_gettime does not change between calls. */
	static unsigned int counter = 0;
	r += counter++;

	for (i=0; i<6; i++, r>>=5)
		template[i] = 'A'+(r&15)+(r&16)*2;

	return template;
}
