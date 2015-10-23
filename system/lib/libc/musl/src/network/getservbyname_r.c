#define _GNU_SOURCE
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>

int getservbyname_r(const char *name, const char *prots,
	struct servent *se, char *buf, size_t buflen, struct servent **res)
{
	struct addrinfo *ai, hint = { .ai_family = AF_INET };
	int i;

	if (!prots) {
		int r = getservbyname_r(name, "tcp", se, buf, buflen, res);
		if (r) r = getservbyname_r(name, "udp", se, buf, buflen, res);
		return r;
	}

	/* Align buffer */
	i = (uintptr_t)buf & sizeof(char *)-1;
	if (!i) i = sizeof(char *);
	if (buflen < 3*sizeof(char *)-i)
		return ERANGE;
	buf += sizeof(char *)-i;
	buflen -= sizeof(char *)-i;

	if (!strcmp(prots, "tcp")) hint.ai_protocol = IPPROTO_TCP;
	else if (!strcmp(prots, "udp")) hint.ai_protocol = IPPROTO_UDP;
	else return EINVAL;

	switch (getaddrinfo(0, name, &hint, &ai)) {
	case EAI_MEMORY:
	case EAI_SYSTEM:
		return ENOMEM;
	default:
		return ENOENT;
	case 0:
		break;
	}

	se->s_name = (char *)name;
	se->s_aliases = (void *)buf;
	se->s_aliases[0] = se->s_name;
	se->s_aliases[1] = 0;
	se->s_port = ((struct sockaddr_in *)ai->ai_addr)->sin_port;
	se->s_proto = (char *)prots;

	freeaddrinfo(ai);
	*res = se;
	return 0;
}
