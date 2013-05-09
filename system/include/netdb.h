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

#define EAI_ADDRFAMILY 1
#define EAI_AGAIN 2
#define EAI_BADFLAGS 3
#define EAI_FAIL 4
#define EAI_FAMILY 5
#define EAI_MEMORY 6
#define EAI_NODATA 7
#define EAI_NONAME 8
#define EAI_SERVICE 9
#define EAI_SOCKTYPE 10
#define EAI_SYSTEM 11
#define EAI_BADHINTS 12
#define EAI_PROTOCOL 13
#define EAI_OVERFLOW 14
#define EAI_MAX 15

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

struct addrinfo
{
  int ai_flags;
  int ai_family;
  int ai_socktype;
  int ai_protocol;
  socklen_t ai_addrlen;
  struct sockaddr *ai_addr;
  char *ai_canonname;
  struct addrinfo *ai_next;
};

extern int getaddrinfo(const char *name, const char *service, const struct addrinfo *req, struct addrinfo **pai);
extern void freeaddrinfo(struct addrinfo *ai);
extern int getnameinfo (struct sockaddr *sa, socklen_t salen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, unsigned int flags);
const char *gai_strerror(int ecode);

struct hostent
{
  char  *h_name;
  char **h_aliases;
  int    h_addrtype;
  int    h_length;
  char **h_addr_list;
};
#define h_addr h_addr_list[0]

struct hostent* gethostbyaddr(const void* addr, socklen_t len, int type);
struct hostent* gethostbyname(const char* name);
void sethostent(int stayopen);
void endhostent(void);
void herror(const char* s);
const char* hstrerror(int err);

extern int h_errno;

#include <netinet/in.h>

#ifdef __cplusplus
}
#endif

#endif
