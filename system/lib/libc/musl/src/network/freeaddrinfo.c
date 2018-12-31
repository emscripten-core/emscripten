#include <stdlib.h>
#include <netdb.h>

void freeaddrinfo(struct addrinfo *p)
{
	free(p);
}
