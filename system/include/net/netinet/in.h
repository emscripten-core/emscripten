
#ifndef _NET_NETINET_IN_H
#define _NET_NETINET_IN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>

enum {
    IPPROTO_IP = 0,
#define IPPROTO_IP IPPROTO_IP
    IPPROTO_TCP = 1,
#define IPPROTO_TCP IPPROTO_TCP
    IPPROTO_UDP = 2,
#define IPPROTO_UDP IPPROTO_UDP
    IPPROTO_MAX
};

#define INET_ADDRSTRLEN 16

#define INADDR_ANY 0

struct in_addr {
  unsigned long s_addr;
};

struct sockaddr_in {
  int            sin_family;
  unsigned short sin_port;
  struct in_addr sin_addr;
  char           sin_zero[6];
};

struct in6_addr {
  unsigned char s6_addr[16];
};

struct sockaddr_in6 {
  short           sin6_family;
  short           sin6_port;
  int             sin6_flowinfo;
  struct in6_addr sin6_addr;
  int             sin6_scope_id;
};

struct ip_mreq {
  struct in_addr imr_multiaddr;
  struct in_addr imr_interface;
};

#define IP_MULTICAST_IF 32
#define IP_MULTICAST_TTL 33
#define IP_MULTICAST_LOOP 34
#define IP_ADD_MEMBERSHIP 35
#define IP_DROP_MEMBERSHIP 36
#define IP_UNBLOCK_SOURCE 37
#define IP_BLOCK_SOURCE 38
#define IP_ADD_SOURCE_MEMBERSHIP 39
#define IP_DROP_SOURCE_MEMBERSHIP 40
#define IP_MSFILTER 41
#define MCAST_JOIN_GROUP 42
#define MCAST_BLOCK_SOURCE 43
#define MCAST_UNBLOCK_SOURCE 44
#define MCAST_LEAVE_GROUP 45
#define MCAST_JOIN_SOURCE_GROUP 46
#define MCAST_LEAVE_SOURCE_GROUP 47
#define MCAST_MSFILTER 48
#define IP_MULTICAST_ALL 49
#define IP_UNICAST_IF 50

#ifdef __cplusplus
}
#endif

#endif

