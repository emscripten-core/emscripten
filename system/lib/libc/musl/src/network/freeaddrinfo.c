#include <stdlib.h>
#include <stddef.h>
#include <netdb.h>
#include "lookup.h"
#include "lock.h"

void freeaddrinfo(struct addrinfo *p)
{
#if __EMSCRIPTEN__
	// Emscripten allocates each node and its ai_addr separately (no aibuf block,
	// no aliasing), and getaddrinfo may return a linked list, so walk it freeing
	// each node and its address.
	while (p) {
		struct addrinfo *next = p->ai_next;
		free(p->ai_addr);
		free(p);
		p = next;
	}
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
