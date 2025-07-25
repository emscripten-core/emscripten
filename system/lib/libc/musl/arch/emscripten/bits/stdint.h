typedef int32_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef uint32_t uint_fast16_t;
typedef uint32_t uint_fast32_t;

#define INT_FAST16_MIN  INT32_MIN
#define INT_FAST32_MIN  INT32_MIN

#define INT_FAST16_MAX  INT32_MAX
#define INT_FAST32_MAX  INT32_MAX

#define UINT_FAST16_MAX UINT32_MAX
#define UINT_FAST32_MAX UINT32_MAX

#define INTPTR_MIN      (-1-__INTPTR_MAX__)
#define INTPTR_MAX      __INTPTR_MAX__
#define UINTPTR_MAX     __UINTPTR_MAX__
#define PTRDIFF_MIN     (-1-__PTRDIFF_MAX__)
#define PTRDIFF_MAX     __PTRDIFF_MAX__
#define SIZE_MAX        __SIZE_MAX__
