#include <stdlib.h>
#include <pthread.h>

extern "C" {

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
	void **ptr = (void**)malloc(sizeof(void*));
	*ptr = NULL;
	*key = (pthread_key_t)ptr;
	return 0;
}

int pthread_key_delete(pthread_key_t key)
{
	free((void*)key);
	return 0;
}

void *pthread_getspecific(pthread_key_t key)
{
	return *(void**)key;
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
	*(void**)key = (void*)value;
	return 0;
}

}
