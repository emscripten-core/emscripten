#include <pthread.h>

int (pthread_equal)(pthread_t a, pthread_t b)
{
	return a==b;
}
