#define MAP_FAILED ((void *) -1)

#define	PROT_NONE      0
#define	PROT_READ      1
#define	PROT_WRITE     2
#define	PROT_EXEC      4
#define	PROT_GROWSDOWN 0x01000000
#define	PROT_GROWSUP   0x02000000

#define	MAP_SHARED     0x01
#define	MAP_PRIVATE    0x02
#define	MAP_FIXED      0x10

#define MAP_TYPE       0x0f
#define MAP_FILE       0x00
#define MAP_ANON       0x20
#define MAP_ANONYMOUS  MAP_ANON
#define MAP_32BIT      0x40
#define MAP_NORESERVE  0x4000
#define MAP_GROWSDOWN  0x0100
#define MAP_DENYWRITE  0x0800
#define MAP_EXECUTABLE 0x1000
#define MAP_LOCKED     0x2000
#define MAP_POPULATE   0x8000
#define MAP_NONBLOCK   0x10000
#define MAP_STACK      0x20000
#define MAP_HUGETLB    0x40000

#define POSIX_MADV_NORMAL       0
#define POSIX_MADV_RANDOM       1
#define POSIX_MADV_SEQUENTIAL   2
#define POSIX_MADV_WILLNEED     3
#define POSIX_MADV_DONTNEED     0

#define MS_ASYNC        1
#define MS_INVALIDATE   2
#define MS_SYNC         4

#define MCL_CURRENT     1
#define MCL_FUTURE      2

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define MADV_NORMAL      0
#define MADV_RANDOM      1
#define MADV_SEQUENTIAL  2
#define MADV_WILLNEED    3
#define MADV_DONTNEED    4
#define MADV_REMOVE      9
#define MADV_DONTFORK    10
#define MADV_DOFORK      11
#define MADV_MERGEABLE   12
#define MADV_UNMERGEABLE 13
#define MADV_HUGEPAGE    14
#define MADV_NOHUGEPAGE  15
#define MADV_DONTDUMP    16
#define MADV_DODUMP      17
#define MADV_HWPOISON    100
#define MADV_SOFT_OFFLINE 101

#define MREMAP_MAYMOVE  1
#define MREMAP_FIXED    2
#endif
