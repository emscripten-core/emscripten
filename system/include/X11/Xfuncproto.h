/* Xfuncproto.h.  Generated from Xfuncproto.h.in by configure.  */
/*
 *
Copyright 1989, 1991, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 */

/* Definitions to make function prototypes manageable */

#ifndef _XFUNCPROTO_H_
#define _XFUNCPROTO_H_

#ifndef NeedFunctionPrototypes
#define NeedFunctionPrototypes 1
#endif /* NeedFunctionPrototypes */

#ifndef NeedVarargsPrototypes
#define NeedVarargsPrototypes 1
#endif /* NeedVarargsPrototypes */

#if NeedFunctionPrototypes

#ifndef NeedNestedPrototypes
#define NeedNestedPrototypes 1
#endif /* NeedNestedPrototypes */

#ifndef _Xconst
#define _Xconst const
#endif /* _Xconst */

/* Function prototype configuration (see configure for more info) */
#ifndef NARROWPROTO
#define NARROWPROTO /**/
#endif
#ifndef FUNCPROTO
#define FUNCPROTO 15
#endif

#ifndef NeedWidePrototypes
#ifdef NARROWPROTO
#define NeedWidePrototypes 0
#else
#define NeedWidePrototypes 1		/* default to make interropt. easier */
#endif
#endif /* NeedWidePrototypes */

#endif /* NeedFunctionPrototypes */

#ifndef _XFUNCPROTOBEGIN
#if defined(__cplusplus) || defined(c_plusplus) /* for C++ V2.0 */
#define _XFUNCPROTOBEGIN extern "C" {	/* do not leave open across includes */
#define _XFUNCPROTOEND }
#else
#define _XFUNCPROTOBEGIN
#define _XFUNCPROTOEND
#endif
#endif /* _XFUNCPROTOBEGIN */

#if defined(__GNUC__) && (__GNUC__ >= 4)
# define _X_SENTINEL(x) __attribute__ ((__sentinel__(x)))
# define _X_ATTRIBUTE_PRINTF(x,y) __attribute__((__format__(__printf__,x,y)))
#else
# define _X_SENTINEL(x)
# define _X_ATTRIBUTE_PRINTF(x,y)
#endif /* GNUC >= 4 */

#if defined(__GNUC__) && (__GNUC__ >= 4) && !defined(__CYGWIN__)
# define _X_EXPORT      __attribute__((visibility("default")))
# define _X_HIDDEN      __attribute__((visibility("hidden")))
# define _X_INTERNAL    __attribute__((visibility("internal")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
# define _X_EXPORT      __global
# define _X_HIDDEN      __hidden
# define _X_INTERNAL    __hidden
#else /* not gcc >= 4 and not Sun Studio >= 8 */
# define _X_EXPORT
# define _X_HIDDEN
# define _X_INTERNAL
#endif /* GNUC >= 4 */

#if defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 303)
# define _X_LIKELY(x)   __builtin_expect(!!(x), 1)
# define _X_UNLIKELY(x) __builtin_expect(!!(x), 0)
# define _X_INLINE      inline
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
# define _X_LIKELY(x)   (x)
# define _X_UNLIKELY(x) (x)
# define _X_INLINE      inline
#else /* not gcc >= 3.3 and not Sun Studio >= 8 */
# define _X_LIKELY(x)   (x)
# define _X_UNLIKELY(x) (x)
# define _X_INLINE
#endif

#if defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 301)
# define _X_DEPRECATED  __attribute__((deprecated))
#else /* not gcc >= 3.1 */
# define _X_DEPRECATED
#endif

#if defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 205)
# define _X_NORETURN __attribute((noreturn))
#else
# define _X_NORETURN
#endif /* GNUC  */

#endif /* _XFUNCPROTO_H_ */
