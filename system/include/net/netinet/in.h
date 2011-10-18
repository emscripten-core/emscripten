
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

