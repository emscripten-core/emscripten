
#ifndef _NET_ARPA_INET_H
#define _NET_ARPA_INET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

int inet_aton(const char *cp, struct in_addr *addr);
char *inet_ntoa(struct in_addr in);

typedef long in_addr_t;
in_addr_t inet_addr(const char *cp);

#ifdef __cplusplus
}
#endif

#endif

