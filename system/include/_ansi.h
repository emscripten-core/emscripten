/* Provide support for both ANSI and non-ANSI environments.  */

/* Some ANSI environments are "broken" in the sense that __STDC__ cannot be
   relied upon to have it's intended meaning.  Therefore we must use our own
   concoction: _HAVE_STDC.  Always use _HAVE_STDC instead of __STDC__ in newlib
   sources!

   To get a strict ANSI C environment, define macro __STRICT_ANSI__.  This will
   "comment out" the non-ANSI parts of the ANSI header files (non-ANSI header
   files aren't affected).  */

#ifndef	_ANSIDECL_H_
#define	_ANSIDECL_H_

#include <newlib.h>
#include <sys/config.h>

/* First try to figure out whether we really are in an ANSI C environment.  */
/* FIXME: This probably needs some work.  Perhaps sys/config.h can be
   prevailed upon to give us a clue.  */

#ifdef __STDC__
#define _HAVE_STDC
#endif

/*  ISO C++.  */

#ifdef __cplusplus
#if !(defined(_BEGIN_STD_C) && defined(_END_STD_C))
#ifdef _HAVE_STD_CXX
#define _BEGIN_STD_C namespace std { extern "C" {
#define _END_STD_C  } }
#else
#define _BEGIN_STD_C extern "C" {
#define _END_STD_C  }
#endif
#if defined(__GNUC__) && \
  ( (__GNUC__ >= 4) || \
    ( (__GNUC__ >= 3) && defined(__GNUC_MINOR__) && (__GNUC_MINOR__ >= 3) ) )
#define _NOTHROW __attribute__ ((nothrow))
#else
#define _NOTHROW throw()
#endif
#endif
#else
#define _BEGIN_STD_C
#define _END_STD_C
#define _NOTHROW
#endif

#ifdef _HAVE_STDC
#define	_PTR		void *
#define	_AND		,
#define	_NOARGS		void
#define	_CONST		const
#define	_VOLATILE	volatile
#define	_SIGNED		signed
#define	_DOTS		, ...
#define _VOID void
#ifdef __CYGWIN__
#define	_EXFUN_NOTHROW(name, proto)	__cdecl name proto _NOTHROW
#define	_EXFUN(name, proto)		__cdecl name proto
#define	_EXPARM(name, proto)		(* __cdecl name) proto
#define	_EXFNPTR(name, proto)		(__cdecl * name) proto
#else
#define	_EXFUN_NOTHROW(name, proto)	name proto _NOTHROW
#define	_EXFUN(name, proto)		name proto
#define _EXPARM(name, proto)		(* name) proto
#define _EXFNPTR(name, proto)		(* name) proto
#endif
#define	_DEFUN(name, arglist, args)	name(args)
#define	_DEFUN_VOID(name)		name(_NOARGS)
#define _CAST_VOID (void)
#ifndef _LONG_DOUBLE
#define _LONG_DOUBLE long double
#endif
#ifndef _LONG_LONG_TYPE
#define _LONG_LONG_TYPE long long
#endif
#ifndef _PARAMS
#define _PARAMS(paramlist)		paramlist
#endif
#else	
#define	_PTR		char *
#define	_AND		;
#define	_NOARGS
#define	_CONST
#define	_VOLATILE
#define	_SIGNED
#define	_DOTS
#define _VOID void
#define	_EXFUN(name, proto)		name()
#define	_EXFUN_NOTHROW(name, proto)	name()
#define	_DEFUN(name, arglist, args)	name arglist args;
#define	_DEFUN_VOID(name)		name()
#define _CAST_VOID
#define _LONG_DOUBLE double
#define _LONG_LONG_TYPE long
#ifndef _PARAMS
#define _PARAMS(paramlist)		()
#endif
#endif

/* Support gcc's __attribute__ facility.  */

#ifdef __GNUC__
#define _ATTRIBUTE(attrs) __attribute__ (attrs)
#else
#define _ATTRIBUTE(attrs)
#endif

/*  The traditional meaning of 'extern inline' for GCC is not
  to emit the function body unless the address is explicitly
  taken.  However this behaviour is changing to match the C99
  standard, which uses 'extern inline' to indicate that the
  function body *must* be emitted.  If we are using GCC, but do
  not have the new behaviour, we need to use extern inline; if
  we are using a new GCC with the C99-compatible behaviour, or
  a non-GCC compiler (which we will have to hope is C99, since
  there is no other way to achieve the effect of omitting the
  function if it isn't referenced) we just use plain 'inline',
  which c99 defines to mean more-or-less the same as the Gnu C
  'extern inline'.  */
#if defined(__GNUC__) && !defined(__GNUC_STDC_INLINE__)
/* We're using GCC, but without the new C99-compatible behaviour.  */
#define _ELIDABLE_INLINE extern __inline__ _ATTRIBUTE ((__always_inline__))
#else
/* We're using GCC in C99 mode, or an unknown compiler which 
  we just have to hope obeys the C99 semantics of inline.  */
#define _ELIDABLE_INLINE __inline__
#endif

#endif /* _ANSIDECL_H_ */
