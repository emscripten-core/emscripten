#ifndef _STDINT_H
#define _STDINT_H

#define __NEED_int8_t
#define __NEED_int16_t
#define __NEED_int32_t
#define __NEED_int64_t

#define __NEED_uint8_t
#define __NEED_uint16_t
#define __NEED_uint32_t
#define __NEED_uint64_t

#define __NEED_intptr_t
#define __NEED_uintptr_t

#define __NEED_intmax_t
#define __NEED_uintmax_t

#include <bits/alltypes.h>

// XXX EMSCRIPTEN: This file has been modified from the upstream musl version
// to make use of clang pre-defined macros whereever possible, eliminating
// possible inconsistencies.

typedef __INT_FAST8_TYPE__  int_fast8_t;
typedef __INT_FAST16_TYPE__ int_fast16_t;
typedef __INT_FAST32_TYPE__ int_fast32_t;
typedef __INT_FAST64_TYPE__ int_fast64_t;

typedef __INT_LEAST8_TYPE__  int_least8_t;
typedef __INT_LEAST16_TYPE__ int_least16_t;
typedef __INT_LEAST32_TYPE__ int_least32_t;
typedef __INT_LEAST64_TYPE__ int_least64_t;

typedef __UINT_FAST8_TYPE__  uint_fast8_t;
typedef __UINT_FAST16_TYPE__ uint_fast16_t;
typedef __UINT_FAST32_TYPE__ uint_fast32_t;
typedef __UINT_FAST64_TYPE__ uint_fast64_t;

typedef __UINT_LEAST8_TYPE__  uint_least8_t;
typedef __UINT_LEAST16_TYPE__ uint_least16_t;
typedef __UINT_LEAST32_TYPE__ uint_least32_t;
typedef __UINT_LEAST64_TYPE__ uint_least64_t;

#define INT8_MIN   (-1-__INT8_MAX__)
#define INT16_MIN  (-1-__INT16_MAX__)
#define INT32_MIN  (-1-__INT32_MAX__)
#define INT64_MIN  (-1-__INT64_MAX__)

#define INT8_MAX   __INT8_MAX__
#define INT16_MAX  __INT16_MAX__
#define INT32_MAX  __INT32_MAX__
#define INT64_MAX  __INT64_MAX__

#define UINT8_MAX  __UINT8_MAX__
#define UINT16_MAX __UINT16_MAX__
#define UINT32_MAX __UINT32_MAX__
#define UINT64_MAX __UINT64_MAX__

#define INT_FAST8_MIN   (-1-__INT_FAST8_MAX__)
#define INT_FAST16_MIN  (-1-__INT_FAST16_MAX__)
#define INT_FAST32_MIN  (-1-__INT_FAST32_MAX__)
#define INT_FAST64_MIN  (-1-__INT_FAST64_MAX__)

#define INT_LEAST8_MIN   (-1-__INT_LEAST8_MAX__)
#define INT_LEAST16_MIN  (-1-__INT_LEAST16_MAX__)
#define INT_LEAST32_MIN  (-1-__INT_LEAST32_MAX__)
#define INT_LEAST64_MIN  (-1-__INT_LEAST64_MAX__)

#define INT_FAST8_MAX   __INT_FAST8_MAX__
#define INT_FAST16_MAX  __INT_FAST16_MAX__
#define INT_FAST32_MAX  __INT_FAST32_MAX__
#define INT_FAST64_MAX  __INT_FAST64_MAX__

#define INT_LEAST8_MAX   __INT_LEAST8_MAX__
#define INT_LEAST16_MAX  __INT_LEAST16_MAX__
#define INT_LEAST32_MAX  __INT_LEAST32_MAX__
#define INT_LEAST64_MAX  __INT_LEAST64_MAX__

#define UINT_FAST8_MAX  __UINT_FAST8_MAX__
#define UINT_FAST16_MAX __UINT_FAST16_MAX__
#define UINT_FAST32_MAX __UINT_FAST32_MAX__
#define UINT_FAST64_MAX __UINT_FAST64_MAX__

#define UINT_LEAST8_MAX  __UINT_LEAST8_MAX__
#define UINT_LEAST16_MAX __UINT_LEAST16_MAX__
#define UINT_LEAST32_MAX __UINT_LEAST32_MAX__
#define UINT_LEAST64_MAX __UINT_LEAST64_MAX__

#define INTMAX_MIN  (-1-__INTMAX_MAX__)
#define INTMAX_MAX  __INTMAX_MAX__
#define UINTMAX_MAX __UINTMAX_MAX__

#define WCHAR_MAX   __WCHAR_MAX__
#define WINT_MAX    __WINT_MAX__
#define INTPTR_MAX  __INTPTR_MAX__
#define UINTPTR_MAX __UINTPTR_MAX__
#define PTRDIFF_MAX __PTRDIFF_MAX__
#define SIZE_MAX    __SIZE_MAX__

#define WINT_MIN    (-1-__WINT_MAX__)
#define WCHAR_MIN   (-1-__WCHAR_MAX__)
#define INTPTR_MIN  (-1-__INTPTR_MAX__)
#define PTRDIFF_MIN (-1-__PTRDIFF_MAX__)

#define SIG_ATOMIC_MIN  INT32_MIN
#define SIG_ATOMIC_MAX  INT32_MAX

#define INT8_C   __INT8_C
#define INT16_C  __INT16_C
#define INT32_C  __INT32_C
#define INT64_C  __INT64_C
#define INTMAX_C __INTMAX_C

#define UINT8_C   __UINT8_C
#define UINT16_C  __UINT16_C
#define UINT32_C  __UINT32_C
#define UINT64_C  __UINT64_C
#define UINTMAX_C __UINTMAX_C

#endif
