#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int __inet_aton(const char *, struct in_addr *);

in_addr_t inet_addr(const char *p)
{
	struct in_addr a;
	if (!__inet_aton(p, &a)) return -1;
	return a.s_addr;
}
