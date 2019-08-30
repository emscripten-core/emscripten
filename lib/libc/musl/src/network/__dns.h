#include <stddef.h>

#define RR_A 1
#define RR_CNAME 5
#define RR_PTR 12
#define RR_AAAA 28

int __dns_count_addrs(const unsigned char *, int);
int __dns_get_rr(void *, size_t, size_t, size_t, const unsigned char *, int, int);

int __dns_query(unsigned char *, const void *, int, int);
int __ipparse(void *, int, const char *);

int __dns_doqueries(unsigned char *, const char *, int *, int);
