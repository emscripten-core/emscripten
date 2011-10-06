/* */

typedef int socklen_t;

typedef unsigned int sa_family_t;
#define AF_INET 1
#define AF_INET6 6

struct sockaddr {
  sa_family_t sa_family; 
  char        sa_data[];
};


