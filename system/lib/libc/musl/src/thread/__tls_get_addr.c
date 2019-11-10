#include <stddef.h>
#include "pthread_impl.h"
#include "libc.h"

__attribute__((__visibility__("hidden")))
void *__tls_get_new(size_t *);

void *__tls_get_addr(size_t *v)
{
	pthread_t self = __pthread_self();
	if (v[0]<=(size_t)self->dtv[0])
		return (char *)self->dtv[v[0]]+v[1]+DTP_OFFSET;
	return __tls_get_new(v);
}

weak_alias(__tls_get_addr, __tls_get_new);
