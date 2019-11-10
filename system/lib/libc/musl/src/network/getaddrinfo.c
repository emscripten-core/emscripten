#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include "lookup.h"

int getaddrinfo(const char *restrict host, const char *restrict serv, const struct addrinfo *restrict hint, struct addrinfo **restrict res)
{
	struct service ports[MAXSERVS];
	struct address addrs[MAXADDRS];
	char canon[256], *outcanon;
	int nservs, naddrs, nais, canon_len, i, j, k;
	int family = AF_UNSPEC, flags = 0, proto = 0, socktype = 0;
	struct aibuf {
		struct addrinfo ai;
		union sa {
			struct sockaddr_in sin;
			struct sockaddr_in6 sin6;
		} sa;
	} *out;

	if (!host && !serv) return EAI_NONAME;

	if (hint) {
		family = hint->ai_family;
		flags = hint->ai_flags;
		proto = hint->ai_protocol;
		socktype = hint->ai_socktype;

		const int mask = AI_PASSIVE | AI_CANONNAME | AI_NUMERICHOST |
			AI_V4MAPPED | AI_ALL | AI_ADDRCONFIG | AI_NUMERICSERV;
		if ((flags & mask) != flags)
			return EAI_BADFLAGS;

		switch (family) {
		case AF_INET:
		case AF_INET6:
		case AF_UNSPEC:
			break;
		default:
			return EAI_FAMILY;
		}
	}

	nservs = __lookup_serv(ports, serv, proto, socktype, flags);
	if (nservs < 0) return nservs;

	naddrs = __lookup_name(addrs, canon, host, family, flags);
	if (naddrs < 0) return naddrs;

	nais = nservs * naddrs;
	canon_len = strlen(canon);
	out = calloc(1, nais * sizeof(*out) + canon_len + 1);
	if (!out) return EAI_MEMORY;

	if (canon_len) {
		outcanon = (void *)&out[nais];
		memcpy(outcanon, canon, canon_len+1);
	} else {
		outcanon = 0;
	}

	for (k=i=0; i<naddrs; i++) for (j=0; j<nservs; j++, k++) {
		out[k].ai = (struct addrinfo){
			.ai_family = addrs[i].family,
			.ai_socktype = ports[j].socktype,
			.ai_protocol = ports[j].proto,
			.ai_addrlen = addrs[i].family == AF_INET
				? sizeof(struct sockaddr_in)
				: sizeof(struct sockaddr_in6),
			.ai_addr = (void *)&out[k].sa,
			.ai_canonname = outcanon,
			.ai_next = &out[k+1].ai };
		switch (addrs[i].family) {
		case AF_INET:
			out[k].sa.sin.sin_family = AF_INET;
			out[k].sa.sin.sin_port = htons(ports[j].port);
			memcpy(&out[k].sa.sin.sin_addr, &addrs[i].addr, 4);
			break;
		case AF_INET6:
			out[k].sa.sin6.sin6_family = AF_INET6;
			out[k].sa.sin6.sin6_port = htons(ports[j].port);
			out[k].sa.sin6.sin6_scope_id = addrs[i].scopeid;
			memcpy(&out[k].sa.sin6.sin6_addr, &addrs[i].addr, 16);
			break;			
		}
	}
	out[nais-1].ai.ai_next = 0;
	*res = &out->ai;
	return 0;
}
