/* */

#define AF_UNSPEC 100
#define SOCK_STREAM 200
#define SOL_SOCKET 50
#define SO_ERROR 10
#define SOCK_DGRAM 20
#define SO_REUSEADDR 30
#define SO_SNDBUF 40
#define SO_RCVBUF 60

typedef int socklen_t;

typedef unsigned int sa_family_t;
#define AF_INET 1
#define AF_INET6 6

struct sockaddr {
  sa_family_t sa_family; 
  char        sa_data[];
};

struct sockaddr_storage {
  sa_family_t ss_family;
};

ssize_t recvfrom(int socket, void *buffer, size_t length, int flags, struct sockaddr *address, socklen_t *address_len);
int getpeername(int socket, struct sockaddr *address, socklen_t *address_len);
int getsockname(int socket, struct sockaddr *address, socklen_t *address_len);
int socket(int domain, int type, int protocol); 
int bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); 
int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen); 
int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);
ssize_t recv(int s, void *buf, size_t len, int flags);
ssize_t send(int s, const void *buf, size_t len, int flags);
int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);
ssize_t sendto(int s, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);

