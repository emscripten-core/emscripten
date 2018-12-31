#include <string.h>
#include "pthread_impl.h"
#include "libc.h"

void __reset_tls()
{
	pthread_t self = __pthread_self();
	struct tls_module *p;
	size_t i, n = (size_t)self->dtv[0];
	if (n) for (p=libc.tls_head, i=1; i<=n; i++, p=p->next) {
		if (!self->dtv[i]) continue;
		memcpy(self->dtv[i], p->image, p->len);
		memset((char *)self->dtv[i]+p->len, 0,
			p->size - p->len);
	}
}
