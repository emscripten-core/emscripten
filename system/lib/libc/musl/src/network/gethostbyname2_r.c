#define _GNU_SOURCE

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdint.h>

int gethostbyname2_r(const char *name, int af,
	struct hostent *h, char *buf, size_t buflen,
	struct hostent **res, int *err)
{
	struct addrinfo hint = {
		.ai_family = af==AF_INET6 ? af : AF_INET,
		.ai_flags = AI_CANONNAME
	};
	struct addrinfo *ai, *p;
	int i;
	size_t need;
	const char *canon;

	*res = 0;

	af = hint.ai_family;

	/* Align buffer */
	i = (uintptr_t)buf & sizeof(char *)-1;
	if (i) {
		if (buflen < sizeof(char *)-i) return ERANGE;
		buf += sizeof(char *)-i;
		buflen -= sizeof(char *)-i;
	}

	switch (getaddrinfo(name, 0, &hint, &ai)) {
	case EAI_NONAME:
		*err = HOST_NOT_FOUND;
		return errno;
	case EAI_AGAIN:
		*err = TRY_AGAIN;
		return errno;
	default:
	case EAI_MEMORY:
	case EAI_SYSTEM:
	case EAI_FAIL:
		*err = NO_RECOVERY;
		return errno;
	case 0:
		break;
	}

	h->h_addrtype = af;
	h->h_length = af==AF_INET6 ? 16 : 4;

	canon = ai->ai_canonname ? ai->ai_canonname : name;
	need = 4*sizeof(char *);
	for (i=0, p=ai; p; i++, p=p->ai_next)
		need += sizeof(char *) + h->h_length;
	need += strlen(name)+1;
	need += strlen(canon)+1;

	if (need > buflen) {
		freeaddrinfo(ai);
		return ERANGE;
	}

	h->h_aliases = (void *)buf;
	buf += 3*sizeof(char *);
	h->h_addr_list = (void *)buf;
	buf += (i+1)*sizeof(char *);

	h->h_name = h->h_aliases[0] = buf;
	strcpy(h->h_name, canon);
	buf += strlen(h->h_name)+1;

	if (strcmp(h->h_name, name)) {
		h->h_aliases[1] = buf;
		strcpy(h->h_aliases[1], name);
		buf += strlen(h->h_aliases[1])+1;
	} else h->h_aliases[1] = 0;

	h->h_aliases[2] = 0;

	for (i=0, p=ai; p; i++, p=p->ai_next) {
		h->h_addr_list[i] = (void *)buf;
		buf += h->h_length;
		memcpy(h->h_addr_list[i],
			&((struct sockaddr_in *)p->ai_addr)->sin_addr,
			h->h_length);
	}
	h->h_addr_list[i] = 0;

	*res = h;
	freeaddrinfo(ai);
	return 0;
}
