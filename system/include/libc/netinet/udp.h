#ifndef _NETINET_UDP_H
#define _NETINET_UDP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct udphdr {
	uint16_t source;
	uint16_t dest;
	uint16_t len;
	uint16_t check;
};

#define uh_sport source
#define uh_dport dest
#define uh_ulen len
#define uh_sum check

#define UDP_CORK	1
#define UDP_ENCAP	100

#define UDP_ENCAP_ESPINUDP_NON_IKE 1
#define UDP_ENCAP_ESPINUDP	2
#define UDP_ENCAP_L2TPINUDP	3

#define SOL_UDP            17

#ifdef __cplusplus
}
#endif

#endif
