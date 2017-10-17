#ifndef LOOKUP_H
#define LOOKUP_H

#include <stdint.h>
#include <stddef.h>

struct address {
	int family;
	unsigned scopeid;
	uint8_t addr[16];
	int sortkey;
};

struct service {
	uint16_t port;
	unsigned char proto, socktype;
};

#define MAXNS 3

struct resolvconf {
	struct address ns[MAXNS];
	unsigned nns, attempts, ndots;
	unsigned timeout;
};

/* The limit of 48 results is a non-sharp bound on the number of addresses
 * that can fit in one 512-byte DNS packet full of v4 results and a second
 * packet full of v6 results. Due to headers, the actual limit is lower. */
#define MAXADDRS 48
#define MAXSERVS 2

int __lookup_serv(struct service buf[static MAXSERVS], const char *name, int proto, int socktype, int flags);
int __lookup_name(struct address buf[static MAXADDRS], char canon[static 256], const char *name, int family, int flags);
int __lookup_ipliteral(struct address buf[static 1], const char *name, int family);

int __get_resolv_conf(struct resolvconf *, char *, size_t);

#endif
