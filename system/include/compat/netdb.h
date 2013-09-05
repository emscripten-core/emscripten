#ifndef _COMPAT_NETDB_H_
#define _COMPAT_NETDB_H_

#include_next <netdb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The musl includes only define these things for old sources or
   when certain flags are activated. We want these available
   all of the time for now. */
struct hostent *gethostbyname (const char *);
struct hostent *gethostbyaddr (const void *, socklen_t, int);

int gethostbyname_r(const char *, struct hostent *, char *, size_t, struct hostent **, int *);

#ifdef __cplusplus
}
#endif

#endif /* _COMPAT_NETDB_H_ */
