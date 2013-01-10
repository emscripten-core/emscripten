#ifndef _NETDB_H
#define _NETDB_H

#ifdef __cplusplus
extern "C" {
#endif

#define HOST_NOT_FOUND 1
#define TRY_AGAIN 2
#define NO_RECOVERY 3
#define NO_DATA 4
#define NO_ADDRESS 5

#define IP_TOS 1
#define IP_TTL 2
#define IP_HDRINCL 3
#define IP_OPTIONS 4
#define IP_ROUTER_ALERT 5
#define IP_RECVOPTS 6
#define IP_RETOPTS 7
#define IP_PKTINFO 8
#define IP_PKTOPTIONS 9
#define IP_MTU_DISCOVER 10
#define IP_RECVERR 11
#define IP_RECVTTL 12
#define IP_RECVTOS 13
#define IP_MTU 14
#define IP_FREEBIND 15
#define IP_IPSEC_POLICY 16
#define IP_XFRM_POLICY 17
#define IP_PASSSEC 18
#define IP_TRANSPARENT 19

typedef int socklen_t;

struct hostent {
  char*  h_name;
  char** h_aliases;
  int    h_addrtype;
  int    h_length;
  char** h_addr_list;
};
#define h_addr h_addr_list[0]

struct hostent* gethostbyaddr(const void* addr, socklen_t len, int type);
struct hostent* gethostbyname(const char* name);
void sethostent(int stayopen);
void endhostent(void);
void herror(const char* s);
const char* hstrerror(int err);

extern int h_errno;

#ifdef __cplusplus
}
#endif

#endif
