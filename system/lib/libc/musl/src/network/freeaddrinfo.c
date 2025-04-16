#include <stdlib.h>
#include <stddef.h>
#include <netdb.h>
#include "lookup.h"
#include "lock.h"

void freeaddrinfo(struct addrinfo *p)
{
#if __EMSCRIPTEN__
	// Emscripten's usage of this structure is very simple: we always allocate
	// ai_addr, and do not use the linked list aspect at all. There is also no
	// aliasing with aibuf.
	free(p->ai_addr);
	free(p);
#else
	size_t cnt;
	for (cnt=1; p->ai_next; cnt++, p=p->ai_next);
	struct aibuf *b = (void *)((char *)p - offsetof(struct aibuf, ai));
	b -= b->slot;
	LOCK(b->lock);
	if (!(b->ref -= cnt)) free(b);
	else UNLOCK(b->lock);
#endif
}
