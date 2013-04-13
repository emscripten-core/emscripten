#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include <netdb.h>
#include <sys/select.h>
#include <sys/uio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Note that the values of these constants are mostly arbitrary numbers.
#define SOMAXCONN 128
#define PF_LOCAL 1
#define PF_UNIX PF_LOCAL
#define PF_INET 2
#define SO_BROADCAST 6
#define AF_UNIX PF_UNIX

#define AF_UNSPEC 0
#define SOCK_STREAM 200
#define SOL_SOCKET 50
#define SO_ERROR 10
#define SOCK_DGRAM 20
#define SO_REUSEADDR 30
#define SO_SNDBUF 40
#define SO_RCVBUF 60
#define SO_LINGER 70
#define SO_NOSIGPIPE 80
#define SO_KEEPALIVE 90
#define SO_OOBINLINE 100
#define SO_NO_CHECK 110
#define SO_PRIORITY 120
#define SO_LINGER 130
#define SO_BSDCOMPAT 140

#define SHUT_RD 1
#define SHUT_RDWR 2

typedef unsigned int sa_family_t;
#define AF_INET 1
#define AF_INET6 6
#define PF_INET6 AF_INET6

struct sockaddr {
  sa_family_t sa_family; 
  char        sa_data[];
};

struct sockaddr_storage {
  sa_family_t    ss_family;
  unsigned short ss_port;
  unsigned long  ss_addr;
  char           ss_zero[6];
};

ssize_t recvfrom(int socket, void *buffer, size_t length, int flags, struct sockaddr *address, socklen_t *address_len);
int getpeername(int socket, struct sockaddr *address, socklen_t *address_len);
int getsockname(int socket, struct sockaddr *address, socklen_t *address_len);
int socket(int domain, int type, int protocol); 
int bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); 
int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen); 
int shutdown(int sockfd, int how);
int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);
ssize_t recv(int s, void *buf, size_t len, int flags);
ssize_t send(int s, const void *buf, size_t len, int flags);
int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);
ssize_t sendto(int s, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
int socketpair(int domain, int type, int protocol, int sv[2]);

struct msghdr
{
  void         *msg_name;
  socklen_t     msg_namelen;
  struct iovec *msg_iov;
  size_t        msg_iovlen;
  void *        msg_control;
  size_t        msg_controllen;
  int           msg_flags;
};

struct linger {
  int l_onoff;
  int l_linger;
};

#define SIOCATMARK 0x8905

#define SOCK_RAW 111
#define SOCK_SEQPACKET 555

#define PF_APPLETALK 5

#ifdef __cplusplus
}
#endif

#endif

