#include <assert.h>
#include <pthread.h>

int main()
{
	pthread_key_t key;
	pthread_key_create(&key, NULL);
	void *val = pthread_getspecific(key);
	assert(val == 0);
	pthread_setspecific(key, (void*)1);
	val = pthread_getspecific(key);
	assert(val == (void*)1);

#ifdef REPORT_RESULT
	int result = 0;
	REPORT_RESULT();
#endif
}
