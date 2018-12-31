#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "__dns.h"

int __ipparse(void *dest, int family, const char *s0)
{
	const char *s = s0;
	unsigned char *d = dest;
	unsigned long a[16] = { 0 };
	char *z;
	int i;

	if (family == AF_INET6) goto not_v4;

	for (i=0; i<4; i++) {
		a[i] = strtoul(s, &z, 0);
		if (z==s || (*z && *z != '.') || !isdigit(*s)) {
			if (family == AF_INET) return -1;
			goto not_v4;
		}
		if (!*z) break;
		s=z+1;
	}
	if (i==4) return -1;
	switch (i) {
	case 0:
		a[1] = a[0] & 0xffffff;
		a[0] >>= 24;
	case 1:
		a[2] = a[1] & 0xffff;
		a[1] >>= 16;
	case 2:
		a[3] = a[2] & 0xff;
		a[2] >>= 8;
	}
	((struct sockaddr_in *)d)->sin_family = AF_INET;
	d = (void *)&((struct sockaddr_in *)d)->sin_addr;
	for (i=0; i<4; i++) {
		if (a[i] > 255) return -1;
		d[i] = a[i];
	}
	return 0;

not_v4:
	s = s0;
	((struct sockaddr_in6 *)d)->sin6_family = AF_INET6;
	return inet_pton(AF_INET6, s, (void *)&((struct sockaddr_in6 *)d)->sin6_addr) <= 0 ? -1 : 0;
}
