
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

