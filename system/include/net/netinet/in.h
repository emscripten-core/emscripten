
#ifndef _NET_NETINET_IN_H
#define _NET_NETINET_IN_H

#ifdef __cplusplus
extern "C" {
#endif

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
  short          sin_family;
  unsigned short sin_port;
  struct in_addr sin_addr;
  char           sin_zero[8];
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

#ifdef __cplusplus
}
#endif

#endif

