#define _GNU_SOURCE

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>

struct hostent *gethostbyname(const char *name)
{
	return gethostbyname2(name, AF_INET);
}

#if 0
struct hostent *gethostbyname(const char *name)
{
	static struct hostent h;
	static char *h_aliases[3];
	static char h_canon[256];
	static char *h_addr_list[10];
	static char h_addr_data[10][4];
	static const struct addrinfo hint = {
		.ai_family = AF_INET, .ai_flags = AI_CANONNAME
	};
	struct addrinfo *ai, *p;
	int i;

	switch (getaddrinfo(name, 0, &hint, &ai)) {
	case EAI_NONAME:
		h_errno = HOST_NOT_FOUND;
		break;
	case EAI_AGAIN:
		h_errno = TRY_AGAIN;
		break;
	case EAI_FAIL:
		h_errno = NO_RECOVERY;
		break;
	default:
	case EAI_MEMORY:
	case EAI_SYSTEM:
		h_errno = NO_DATA;
		break;
	case 0:
		break;
	}

	strcpy(h_canon, ai->ai_canonname);
	h.h_name = h_canon;
	h.h_aliases = h_aliases;
	h.h_aliases[0] = h_canon;
	h.h_aliases[1] = strcmp(h_canon, name) ? (char *)name : 0;
	h.h_length = 4;
	h.h_addr_list = h_addr_list;
	for (i=0, p=ai; i<sizeof h_addr_data/4 && p; i++, p=p->ai_next) {
		h.h_addr_list[i] = h_addr_data[i];
		memcpy(h.h_addr_list[i],
			&((struct sockaddr_in *)p->ai_addr)->sin_addr, 4);
	}
	h.h_addr_list[i] = 0;
	h.h_addrtype = AF_INET;
	freeaddrinfo(ai);
	return &h;
}
#endif
