#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "__dns.h"

in_addr_t inet_network(const char *p)
{
	return ntohl(inet_addr(p));
}

int inet_aton(const char *cp, struct in_addr *inp)
{
	struct sockaddr_in sin;
	if (__ipparse(&sin, AF_INET, cp) < 0) return 0;
	*inp = sin.sin_addr;
	return 1;
}

struct in_addr inet_makeaddr(in_addr_t n, in_addr_t h)
{
	if (n < 256) h |= n<<24;
	else if (n < 65536) h |= n<<16;
	else h |= n<<8;
	return (struct in_addr){ h };
}

in_addr_t inet_lnaof(struct in_addr in)
{
	uint32_t h = in.s_addr;
	if (h>>24 < 128) return h & 0xffffff;
	if (h>>24 < 192) return h & 0xffff;
	return h & 0xff;
}

in_addr_t inet_netof(struct in_addr in)
{
	uint32_t h = in.s_addr;
	if (h>>24 < 128) return h >> 24;
	if (h>>24 < 192) return h >> 16;
	return h >> 8;
}
