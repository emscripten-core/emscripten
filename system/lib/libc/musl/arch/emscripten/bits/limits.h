#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define PAGE_SIZE 65536
#define LONG_BIT 32
#endif

#if __LP64__
#define LONG_MAX  0x7fffffffffffffffL
#else
#define LONG_MAX  0x7fffffffL
#endif
#define LLONG_MAX  0x7fffffffffffffffLL
