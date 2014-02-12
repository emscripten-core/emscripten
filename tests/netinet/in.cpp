#include <netinet/in.h>
extern "C" int puts(const char *);
int main() {
	struct in6_addr in6any = IN6ADDR_ANY_INIT;
	struct in6_addr in6loopback = IN6ADDR_LOOPBACK_INIT;
	int i;
	for (i = 0; i < 16; ++i)
		if (in6any.s6_addr[i] != in6addr_any.s6_addr[i])
			return puts("in6addr_any != IN6ADDR_ANY_INIT\n");
	for (i = 0; i < 16; ++i)
		if (in6loopback.s6_addr[i] != in6addr_loopback.s6_addr[i])
			return puts("in6addr_loopback != IN6ADDR_LOOPBACK_INIT\n");
	return puts("pass");
}
