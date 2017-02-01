#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define KEY_ALLOCATED_ID ((void*)0x12345678)

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
	void **ptr = (void**)malloc(sizeof(void*)*2);
	ptr[0] = NULL;
	ptr[1] = KEY_ALLOCATED_ID;
	*key = (pthread_key_t)ptr;
	return 0;
}

int pthread_key_delete(pthread_key_t key)
{
	if (!key) return EINVAL;
	void **ptr = (void**)key;
	if (ptr[1] != KEY_ALLOCATED_ID) return EINVAL;
	ptr[1] = NULL;
	free((void*)ptr);
	return 0;
}

void *pthread_getspecific(pthread_key_t key)
{
	if (!key) return 0;
	void **ptr = (void**)key;
	if (ptr[1] != KEY_ALLOCATED_ID) return 0;
	return ptr[0];
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
	if (!key) return EINVAL;
	void **ptr = (void**)key;
	if (ptr[1] != KEY_ALLOCATED_ID) return EINVAL;
	ptr[0] = (void*)value;
	return 0;
}
