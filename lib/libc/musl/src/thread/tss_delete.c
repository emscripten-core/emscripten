#include <threads.h>

int __pthread_key_delete(tss_t k);

void tss_delete(tss_t key)
{
	__pthread_key_delete(key);
}
