/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/**
 *  \file SDL_stdinc.h
 *  
 *  This is a general header that includes C language support.
 */

#ifndef _SDL_stdinc_h
#define _SDL_stdinc_h

#include "SDL_config.h"


#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#if defined(STDC_HEADERS)
# include <stdlib.h>
# include <stddef.h>
# include <stdarg.h>
#else
# if defined(HAVE_STDLIB_H)
#  include <stdlib.h>
# elif defined(HAVE_MALLOC_H)
#  include <malloc.h>
# endif
# if defined(HAVE_STDDEF_H)
#  include <stddef.h>
# endif
# if defined(HAVE_STDARG_H)
#  include <stdarg.h>
# endif
#endif
#ifdef HAVE_STRING_H
# if !defined(STDC_HEADERS) && defined(HAVE_MEMORY_H)
#  include <memory.h>
# endif
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#if defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#elif defined(__EMSCRIPTEN__) || defined(HAVE_STDINT_H)
# include <stdint.h>
#endif
#ifdef HAVE_CTYPE_H
# include <ctype.h>
#endif
#ifdef HAVE_MATH_H
# include <math.h>
#endif
#if defined(HAVE_ICONV) && defined(HAVE_ICONV_H)
# include <iconv.h>
#endif

/**
 *  The number of elements in an array.
 */
#define SDL_arraysize(array)	(sizeof(array)/sizeof(array[0]))
#define SDL_TABLESIZE(table)	SDL_arraysize(table)

/**
 *  \name Cast operators
 *  
 *  Use proper C++ casts when compiled as C++ to be compatible with the option
 *  -Wold-style-cast of GCC (and -Werror=old-style-cast in GCC 4.2 and above).
 */
/*@{*/
#ifdef __cplusplus
#define SDL_reinterpret_cast(type, expression) reinterpret_cast<type>(expression)
#define SDL_static_cast(type, expression) static_cast<type>(expression)
#else
#define SDL_reinterpret_cast(type, expression) ((type)(expression))
#define SDL_static_cast(type, expression) ((type)(expression))
#endif
/*@}*//*Cast operators*/

/* Define a four character code as a Uint32 */
#define SDL_FOURCC(A, B, C, D) \
    ((SDL_static_cast(Uint32, SDL_static_cast(Uint8, (A))) << 0) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (B))) << 8) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (C))) << 16) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (D))) << 24))

/**
 *  \name Basic data types
 */
/*@{*/

typedef enum
{
    SDL_FALSE = 0,
    SDL_TRUE = 1
} SDL_bool;

/**
 * \brief A signed 8-bit integer type.
 */
typedef int8_t Sint8;
/**
 * \brief An unsigned 8-bit integer type.
 */
typedef uint8_t Uint8;
/**
 * \brief A signed 16-bit integer type.
 */
typedef int16_t Sint16;
/**
 * \brief An unsigned 16-bit integer type.
 */
typedef uint16_t Uint16;
/**
 * \brief A signed 32-bit integer type.
 */
typedef int32_t Sint32;
/**
 * \brief An unsigned 32-bit integer type.
 */
typedef uint32_t Uint32;

/**
 * \brief A signed 64-bit integer type.
 */
typedef int64_t Sint64;
/**
 * \brief An unsigned 64-bit integer type.
 */
typedef uint64_t Uint64;

/*@}*//*Basic data types*/


#define SDL_COMPILE_TIME_ASSERT(name, x)               \
       typedef int SDL_dummy_ ## name[(x) * 2 - 1]
/** \cond */
#ifndef DOXYGEN_SHOULD_IGNORE_THIS
SDL_COMPILE_TIME_ASSERT(uint8, sizeof(Uint8) == 1);
SDL_COMPILE_TIME_ASSERT(sint8, sizeof(Sint8) == 1);
SDL_COMPILE_TIME_ASSERT(uint16, sizeof(Uint16) == 2);
SDL_COMPILE_TIME_ASSERT(sint16, sizeof(Sint16) == 2);
SDL_COMPILE_TIME_ASSERT(uint32, sizeof(Uint32) == 4);
SDL_COMPILE_TIME_ASSERT(sint32, sizeof(Sint32) == 4);
SDL_COMPILE_TIME_ASSERT(uint64, sizeof(Uint64) == 8);
SDL_COMPILE_TIME_ASSERT(sint64, sizeof(Sint64) == 8);
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */
/** \endcond */

/* Check to make sure enums are the size of ints, for structure packing.
   For both Watcom C/C++ and Borland C/C++ the compiler option that makes
   enums having the size of an int must be enabled.
   This is "-b" for Borland C/C++ and "-ei" for Watcom C/C++ (v11).
*/
/* Enable enums always int in CodeWarrior (for MPW use "-enum int") */
#ifdef __MWERKS__
#pragma enumsalwaysint on
#endif

/** \cond */
#ifndef DOXYGEN_SHOULD_IGNORE_THIS
#if !defined(__NINTENDODS__) && !defined(__ANDROID__) 
   /* TODO: include/SDL_stdinc.h:174: error: size of array 'SDL_dummy_enum' is negative */
typedef enum
{
    DUMMY_ENUM_VALUE
} SDL_DUMMY_ENUM;

SDL_COMPILE_TIME_ASSERT(enum, sizeof(SDL_DUMMY_ENUM) == sizeof(int));
#endif
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */
/** \endcond */

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#ifdef HAVE_MALLOC
#define SDL_malloc	malloc
#else
extern DECLSPEC void *SDLCALL SDL_malloc(size_t size);
#endif

#ifdef HAVE_CALLOC
#define SDL_calloc	calloc
#else
extern DECLSPEC void *SDLCALL SDL_calloc(size_t nmemb, size_t size);
#endif

#ifdef HAVE_REALLOC
#define SDL_realloc	realloc
#else
extern DECLSPEC void *SDLCALL SDL_realloc(void *mem, size_t size);
#endif

#ifdef HAVE_FREE
#define SDL_free	free
#else
extern DECLSPEC void SDLCALL SDL_free(void *mem);
#endif

#if defined(HAVE_ALLOCA) && !defined(alloca)
# if defined(HAVE_ALLOCA_H)
#  include <alloca.h>
# elif defined(__GNUC__)
#  define alloca __builtin_alloca
# elif defined(_MSC_VER)
#  include <malloc.h>
#  define alloca _alloca
# elif defined(__WATCOMC__)
#  include <malloc.h>
# elif defined(__BORLANDC__)
#  include <malloc.h>
# elif defined(__DMC__)
#  include <stdlib.h>
# elif defined(__AIX__)
#pragma alloca
# elif defined(__MRC__)
void *alloca(unsigned);
# else
char *alloca();
# endif
#endif
#ifdef HAVE_ALLOCA
#define SDL_stack_alloc(type, count)    (type*)alloca(sizeof(type)*(count))
#define SDL_stack_free(data)
#else
#define SDL_stack_alloc(type, count)    (type*)SDL_malloc(sizeof(type)*(count))
#define SDL_stack_free(data)            SDL_free(data)
#endif

#ifdef HAVE_GETENV
#define SDL_getenv	getenv
#else
extern DECLSPEC char *SDLCALL SDL_getenv(const char *name);
#endif

/* SDL_putenv() has moved to SDL_compat. */
#ifdef HAVE_SETENV
#define SDL_setenv	setenv
#else
extern DECLSPEC int SDLCALL SDL_setenv(const char *name, const char *value,
                                       int overwrite);
#endif

#ifdef HAVE_QSORT
#define SDL_qsort	qsort
#else
extern DECLSPEC void SDLCALL SDL_qsort(void *base, size_t nmemb, size_t size,
                                       int (*compare) (const void *,
                                                       const void *));
#endif

#ifdef HAVE_ABS
#define SDL_abs		abs
#else
#define SDL_abs(X)	((X) < 0 ? -(X) : (X))
#endif

#define SDL_min(x, y)	(((x) < (y)) ? (x) : (y))
#define SDL_max(x, y)	(((x) > (y)) ? (x) : (y))

#ifdef HAVE_CTYPE_H
#define SDL_isdigit(X)  isdigit(X)
#define SDL_isspace(X)  isspace(X)
#define SDL_toupper(X)  toupper(X)
#define SDL_tolower(X)  tolower(X)
#else
#define SDL_isdigit(X)  (((X) >= '0') && ((X) <= '9'))
#define SDL_isspace(X)  (((X) == ' ') || ((X) == '\t') || ((X) == '\r') || ((X) == '\n'))
#define SDL_toupper(X)  (((X) >= 'a') && ((X) <= 'z') ? ('A'+((X)-'a')) : (X))
#define SDL_tolower(X)  (((X) >= 'A') && ((X) <= 'Z') ? ('a'+((X)-'A')) : (X))
#endif

#ifdef HAVE_MEMSET
#define SDL_memset      memset
#else
extern DECLSPEC void *SDLCALL SDL_memset(void *dst, int c, size_t len);
#endif
#define SDL_zero(x)	SDL_memset(&(x), 0, sizeof((x)))
#define SDL_zerop(x)	SDL_memset((x), 0, sizeof(*(x)))

#if defined(__GNUC__) && defined(i386)
#define SDL_memset4(dst, val, len)				\
do {								\
	int u0, u1, u2;						\
	__asm__ __volatile__ (					\
		"cld\n\t"					\
		"rep ; stosl\n\t"				\
		: "=&D" (u0), "=&a" (u1), "=&c" (u2)		\
		: "0" (dst), "1" (val), "2" (SDL_static_cast(Uint32, len))	\
		: "memory" );					\
} while(0)
#endif
#ifndef SDL_memset4
#define SDL_memset4(dst, val, len)		\
do {						\
	unsigned _count = (len);		\
	unsigned _n = (_count + 3) / 4;		\
	Uint32 *_p = SDL_static_cast(Uint32 *, dst);		\
	Uint32 _val = (val);			\
	if (len == 0) break;			\
        switch (_count % 4) {			\
        case 0: do {    *_p++ = _val;		\
        case 3:         *_p++ = _val;		\
        case 2:         *_p++ = _val;		\
        case 1:         *_p++ = _val;		\
		} while ( --_n );		\
	}					\
} while(0)
#endif

/* We can count on memcpy existing on Mac OS X and being well-tuned. */
#if defined(__MACOSX__)
#define SDL_memcpy      memcpy
#elif defined(__GNUC__) && defined(i386)
#define SDL_memcpy(dst, src, len)					  \
do {									  \
	int u0, u1, u2;						  	  \
	__asm__ __volatile__ (						  \
		"cld\n\t"						  \
		"rep ; movsl\n\t"					  \
		"testb $2,%b4\n\t"					  \
		"je 1f\n\t"						  \
		"movsw\n"						  \
		"1:\ttestb $1,%b4\n\t"					  \
		"je 2f\n\t"						  \
		"movsb\n"						  \
		"2:"							  \
		: "=&c" (u0), "=&D" (u1), "=&S" (u2)			  \
		: "0" (SDL_static_cast(unsigned, len)/4), "q" (len), "1" (dst),"2" (src) \
		: "memory" );						  \
} while(0)
#endif
#ifndef SDL_memcpy
#ifdef HAVE_MEMCPY
#define SDL_memcpy      memcpy
#elif defined(HAVE_BCOPY)
#define SDL_memcpy(d, s, n)	bcopy((s), (d), (n))
#else
extern DECLSPEC void *SDLCALL SDL_memcpy(void *dst, const void *src,
                                         size_t len);
#endif
#endif

/* We can count on memcpy existing on Mac OS X and being well-tuned. */
#if defined(__MACOSX__)
#define SDL_memcpy4(dst, src, len)	SDL_memcpy((dst), (src), (len) << 2)
#elif defined(__GNUC__) && defined(i386)
#define SDL_memcpy4(dst, src, len)				\
do {								\
	int ecx, edi, esi;					\
	__asm__ __volatile__ (					\
		"cld\n\t"					\
		"rep ; movsl"					\
		: "=&c" (ecx), "=&D" (edi), "=&S" (esi)		\
		: "0" (SDL_static_cast(unsigned, len)), "1" (dst), "2" (src)	\
		: "memory" );					\
} while(0)
#endif
#ifndef SDL_memcpy4
#define SDL_memcpy4(dst, src, len)	SDL_memcpy((dst), (src), (len) << 2)
#endif

#ifdef HAVE_MEMMOVE
#define SDL_memmove     memmove
#else
extern DECLSPEC void *SDLCALL SDL_memmove(void *dst, const void *src,
                                          size_t len);
#endif

#ifdef HAVE_MEMCMP
#define SDL_memcmp      memcmp
#else
extern DECLSPEC int SDLCALL SDL_memcmp(const void *s1, const void *s2,
                                       size_t len);
#endif

#ifdef HAVE_STRLEN
#define SDL_strlen      strlen
#else
extern DECLSPEC size_t SDLCALL SDL_strlen(const char *string);
#endif

#ifdef HAVE_WCSLEN
#define SDL_wcslen      wcslen
#else
#if !defined(wchar_t) && defined(__NINTENDODS__)
#define wchar_t short           /* TODO: figure out why libnds doesn't have this */
#endif
extern DECLSPEC size_t SDLCALL SDL_wcslen(const wchar_t * string);
#endif

#ifdef HAVE_WCSLCPY
#define SDL_wcslcpy      wcslcpy
#else
extern DECLSPEC size_t SDLCALL SDL_wcslcpy(wchar_t *dst, const wchar_t *src, size_t maxlen);
#endif

#ifdef HAVE_WCSLCAT
#define SDL_wcslcat      wcslcat
#else
extern DECLSPEC size_t SDLCALL SDL_wcslcat(wchar_t *dst, const wchar_t *src, size_t maxlen);
#endif


#ifdef HAVE_STRLCPY
#define SDL_strlcpy     strlcpy
#else
extern DECLSPEC size_t SDLCALL SDL_strlcpy(char *dst, const char *src,
                                           size_t maxlen);
#endif

extern DECLSPEC size_t SDLCALL SDL_utf8strlcpy(char *dst, const char *src,
                                            size_t dst_bytes);

#ifdef HAVE_STRLCAT
#define SDL_strlcat    strlcat
#else
extern DECLSPEC size_t SDLCALL SDL_strlcat(char *dst, const char *src,
                                           size_t maxlen);
#endif

#ifdef HAVE_STRDUP
#define SDL_strdup     strdup
#else
extern DECLSPEC char *SDLCALL SDL_strdup(const char *string);
#endif

#ifdef HAVE__STRREV
#define SDL_strrev      _strrev
#else
extern DECLSPEC char *SDLCALL SDL_strrev(char *string);
#endif

#ifdef HAVE__STRUPR
#define SDL_strupr      _strupr
#else
extern DECLSPEC char *SDLCALL SDL_strupr(char *string);
#endif

#ifdef HAVE__STRLWR
#define SDL_strlwr      _strlwr
#else
extern DECLSPEC char *SDLCALL SDL_strlwr(char *string);
#endif

#ifdef HAVE_STRCHR
#define SDL_strchr      strchr
#elif defined(HAVE_INDEX)
#define SDL_strchr      index
#else
extern DECLSPEC char *SDLCALL SDL_strchr(const char *string, int c);
#endif

#ifdef HAVE_STRRCHR
#define SDL_strrchr     strrchr
#elif defined(HAVE_RINDEX)
#define SDL_strrchr     rindex
#else
extern DECLSPEC char *SDLCALL SDL_strrchr(const char *string, int c);
#endif

#ifdef HAVE_STRSTR
#define SDL_strstr      strstr
#else
extern DECLSPEC char *SDLCALL SDL_strstr(const char *haystack,
                                         const char *needle);
#endif

#ifdef HAVE_ITOA
#define SDL_itoa        itoa
#else
#define SDL_itoa(value, string, radix)	SDL_ltoa((long)value, string, radix)
#endif

#ifdef HAVE__LTOA
#define SDL_ltoa        _ltoa
#else
extern DECLSPEC char *SDLCALL SDL_ltoa(long value, char *string, int radix);
#endif

#ifdef HAVE__UITOA
#define SDL_uitoa       _uitoa
#else
#define SDL_uitoa(value, string, radix)	SDL_ultoa((long)value, string, radix)
#endif

#ifdef HAVE__ULTOA
#define SDL_ultoa       _ultoa
#else
extern DECLSPEC char *SDLCALL SDL_ultoa(unsigned long value, char *string,
                                        int radix);
#endif

#ifdef HAVE_STRTOL
#define SDL_strtol      strtol
#else
extern DECLSPEC long SDLCALL SDL_strtol(const char *string, char **endp,
                                        int base);
#endif

#ifdef HAVE_STRTOUL
#define SDL_strtoul      strtoul
#else
extern DECLSPEC unsigned long SDLCALL SDL_strtoul(const char *string,
                                                  char **endp, int base);
#endif

#ifdef HAVE__I64TOA
#define SDL_lltoa       _i64toa
#else
extern DECLSPEC char *SDLCALL SDL_lltoa(Sint64 value, char *string,
                                        int radix);
#endif

#ifdef HAVE__UI64TOA
#define SDL_ulltoa      _ui64toa
#else
extern DECLSPEC char *SDLCALL SDL_ulltoa(Uint64 value, char *string,
                                         int radix);
#endif

#ifdef HAVE_STRTOLL
#define SDL_strtoll     strtoll
#else
extern DECLSPEC Sint64 SDLCALL SDL_strtoll(const char *string, char **endp,
                                           int base);
#endif

#ifdef HAVE_STRTOULL
#define SDL_strtoull     strtoull
#else
extern DECLSPEC Uint64 SDLCALL SDL_strtoull(const char *string, char **endp,
                                            int base);
#endif

#ifdef HAVE_STRTOD
#define SDL_strtod      strtod
#else
extern DECLSPEC double SDLCALL SDL_strtod(const char *string, char **endp);
#endif

#ifdef HAVE_ATOI
#define SDL_atoi        atoi
#else
#define SDL_atoi(X)     SDL_strtol(X, NULL, 0)
#endif

#ifdef HAVE_ATOF
#define SDL_atof        atof
#else
#define SDL_atof(X)     SDL_strtod(X, NULL)
#endif

#ifdef HAVE_STRCMP
#define SDL_strcmp      strcmp
#else
extern DECLSPEC int SDLCALL SDL_strcmp(const char *str1, const char *str2);
#endif

#ifdef HAVE_STRNCMP
#define SDL_strncmp     strncmp
#else
extern DECLSPEC int SDLCALL SDL_strncmp(const char *str1, const char *str2,
                                        size_t maxlen);
#endif

#ifdef HAVE_STRCASECMP
#define SDL_strcasecmp  strcasecmp
#elif defined(HAVE__STRICMP)
#define SDL_strcasecmp  _stricmp
#else
extern DECLSPEC int SDLCALL SDL_strcasecmp(const char *str1,
                                           const char *str2);
#endif

#ifdef HAVE_STRNCASECMP
#define SDL_strncasecmp strncasecmp
#elif defined(HAVE__STRNICMP)
#define SDL_strncasecmp _strnicmp
#else
extern DECLSPEC int SDLCALL SDL_strncasecmp(const char *str1,
                                            const char *str2, size_t maxlen);
#endif

#ifdef HAVE_SSCANF
#define SDL_sscanf      sscanf
#else
extern DECLSPEC int SDLCALL SDL_sscanf(const char *text, const char *fmt,
                                       ...);
#endif

#ifdef HAVE_SNPRINTF
#define SDL_snprintf    snprintf
#else
extern DECLSPEC int SDLCALL SDL_snprintf(char *text, size_t maxlen,
                                         const char *fmt, ...);
#endif

#ifdef HAVE_VSNPRINTF
#define SDL_vsnprintf   vsnprintf
#else
extern DECLSPEC int SDLCALL SDL_vsnprintf(char *text, size_t maxlen,
                                          const char *fmt, va_list ap);
#endif

#ifndef HAVE_M_PI
#define M_PI    3.14159265358979323846264338327950288   /* pi */
#endif

#ifdef HAVE_ATAN
#define SDL_atan        atan
#else
extern DECLSPEC double SDLCALL SDL_atan(double x);
#endif

#ifdef HAVE_ATAN2
#define SDL_atan2       atan2
#else
extern DECLSPEC double SDLCALL SDL_atan2(double y, double x);
#endif

#ifdef HAVE_CEIL
#define SDL_ceil        ceil
#else
#define SDL_ceil(x)     ((double)(int)((x)+0.5))
#endif

#ifdef HAVE_COPYSIGN
#define SDL_copysign    copysign
#else
extern DECLSPEC double SDLCALL SDL_copysign(double x, double y);
#endif

#ifdef HAVE_COS
#define SDL_cos         cos
#else
extern DECLSPEC double SDLCALL SDL_cos(double x);
#endif

#ifdef HAVE_COSF
#define SDL_cosf        cosf
#else
#define SDL_cosf(x) (float)SDL_cos((double)x)
#endif

#ifdef HAVE_FABS
#define SDL_fabs        fabs
#else
extern DECLSPEC double SDLCALL SDL_fabs(double x);
#endif

#ifdef HAVE_FLOOR
#define SDL_floor       floor
#else
extern DECLSPEC double SDLCALL SDL_floor(double x);
#endif

#ifdef HAVE_LOG
#define SDL_log         log
#else
extern DECLSPEC double SDLCALL SDL_log(double x);
#endif

#ifdef HAVE_POW
#define SDL_pow         pow
#else
extern DECLSPEC double SDLCALL SDL_pow(double x, double y);
#endif

#ifdef HAVE_SCALBN
#define SDL_scalbn      scalbn
#else
extern DECLSPEC double SDLCALL SDL_scalbn(double x, int n);
#endif

#ifdef HAVE_SIN
#define SDL_sin         sin
#else
extern DECLSPEC double SDLCALL SDL_sin(double x);
#endif

#ifdef HAVE_SINF
#define SDL_sinf        sinf
#else
#define SDL_sinf(x) (float)SDL_sin((double)x)
#endif

#ifdef HAVE_SQRT
#define SDL_sqrt        sqrt
#else
extern DECLSPEC double SDLCALL SDL_sqrt(double x);
#endif

/* The SDL implementation of iconv() returns these error codes */
#define SDL_ICONV_ERROR		(size_t)-1
#define SDL_ICONV_E2BIG		(size_t)-2
#define SDL_ICONV_EILSEQ	(size_t)-3
#define SDL_ICONV_EINVAL	(size_t)-4

#if defined(HAVE_ICONV) && defined(HAVE_ICONV_H)
#define SDL_iconv_t     iconv_t
#define SDL_iconv_open  iconv_open
#define SDL_iconv_close iconv_close
#else
typedef struct _SDL_iconv_t *SDL_iconv_t;
extern DECLSPEC SDL_iconv_t SDLCALL SDL_iconv_open(const char *tocode,
                                                   const char *fromcode);
extern DECLSPEC int SDLCALL SDL_iconv_close(SDL_iconv_t cd);
#endif
extern DECLSPEC size_t SDLCALL SDL_iconv(SDL_iconv_t cd, const char **inbuf,
                                         size_t * inbytesleft, char **outbuf,
                                         size_t * outbytesleft);
/**
 *  This function converts a string between encodings in one pass, returning a
 *  string that must be freed with SDL_free() or NULL on error.
 */
extern DECLSPEC char *SDLCALL SDL_iconv_string(const char *tocode,
                                               const char *fromcode,
                                               const char *inbuf,
                                               size_t inbytesleft);
#define SDL_iconv_utf8_locale(S)	SDL_iconv_string("", "UTF-8", S, SDL_strlen(S)+1)
#define SDL_iconv_utf8_ucs2(S)		(Uint16 *)SDL_iconv_string("UCS-2", "UTF-8", S, SDL_strlen(S)+1)
#define SDL_iconv_utf8_ucs4(S)		(Uint32 *)SDL_iconv_string("UCS-4", "UTF-8", S, SDL_strlen(S)+1)

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#include "close_code.h"

#endif /* _SDL_stdinc_h */

/* vi: set ts=4 sw=4 expandtab: */
