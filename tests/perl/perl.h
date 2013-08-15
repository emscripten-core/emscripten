/*    perl.h
 *
 *    Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001
 *    2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef H_PERL
#define H_PERL 1

#ifdef PERL_FOR_X2P
/*
 * This file is being used for x2p stuff.
 * Above symbol is defined via -D in 'x2p/Makefile.SH'
 * Decouple x2p stuff from some of perls more extreme eccentricities.
 */
#undef MULTIPLICITY
#undef USE_STDIO
#define USE_STDIO
#endif /* PERL_FOR_X2P */

#if defined(DGUX)
#include <sys/fcntl.h>
#endif

#ifdef VOIDUSED
#   undef VOIDUSED
#endif
#define VOIDUSED 1

#ifdef PERL_MICRO
#   include "uconfig.h"
#else
#   ifndef USE_CROSS_COMPILE
#       include "config.h"
#   else
#       include "xconfig.h"
#   endif
#endif

/* See L<perlguts/"The Perl API"> for detailed notes on
 * PERL_IMPLICIT_CONTEXT and PERL_IMPLICIT_SYS */

/* Note that from here --> to <-- the same logic is
 * repeated in makedef.pl, so be certain to update
 * both places when editing. */

#ifdef PERL_IMPLICIT_SYS
/* PERL_IMPLICIT_SYS implies PerlMemShared != PerlMem
   so use slab allocator to avoid lots of MUTEX overhead
 */
#  ifndef PL_OP_SLAB_ALLOC
#    define PL_OP_SLAB_ALLOC
#  endif
#endif

#ifdef USE_ITHREADS
#  if !defined(MULTIPLICITY)
#    define MULTIPLICITY
#  endif
#endif

#ifdef PERL_GLOBAL_STRUCT_PRIVATE
#  ifndef PERL_GLOBAL_STRUCT
#    define PERL_GLOBAL_STRUCT
#  endif
#endif

#ifdef PERL_GLOBAL_STRUCT
#  ifndef MULTIPLICITY
#    define MULTIPLICITY
#  endif
#endif

#ifdef MULTIPLICITY
#  ifndef PERL_IMPLICIT_CONTEXT
#    define PERL_IMPLICIT_CONTEXT
#  endif
#endif

/* undef WIN32 when building on Cygwin (for libwin32) - gph */
#ifdef __CYGWIN__
#   undef WIN32
#   undef _WIN32
#endif

#if defined(__SYMBIAN32__) || (defined(__VC32__) && defined(WINS))
#   ifndef SYMBIAN
#       define SYMBIAN
#   endif
#endif

#ifdef __SYMBIAN32__
#  include "symbian/symbian_proto.h"
#endif

/* Any stack-challenged places.  The limit varies (and often
 * is configurable), but using more than a kilobyte of stack
 * is usually dubious in these systems. */
#if defined(EPOC) || defined(__SYMBIAN32__)
/* EPOC/Symbian: need to work around the SDK features. *
 * On WINS: MS VC5 generates calls to _chkstk,         *
 * if a "large" stack frame is allocated.              *
 * gcc on MARM does not generate calls like these.     */
#   define USE_HEAP_INSTEAD_OF_STACK
#endif

#/* Use the reentrant APIs like localtime_r and getpwent_r */
/* Win32 has naturally threadsafe libraries, no need to use any _r variants. */
#if defined(USE_ITHREADS) && !defined(USE_REENTRANT_API) && !defined(NETWARE) && !defined(WIN32) && !defined(PERL_DARWIN)
#   define USE_REENTRANT_API
#endif

/* <--- here ends the logic shared by perl.h and makedef.pl */

/*
 * PERL_DARWIN for MacOSX (__APPLE__ exists but is not officially sanctioned)
 * (The -DPERL_DARWIN comes from the hints/darwin.sh.)
 * __bsdi__ for BSD/OS
 */
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(PERL_DARWIN) || defined(__bsdi__) || defined(BSD41) || defined(BSD42) || defined(BSD43) || defined(BSD44)
#   ifndef BSDish
#       define BSDish
#   endif
#endif

#undef START_EXTERN_C
#undef END_EXTERN_C
#undef EXTERN_C
#ifdef __cplusplus
#  define START_EXTERN_C extern "C" {
#  define END_EXTERN_C }
#  define EXTERN_C extern "C"
#else
#  define START_EXTERN_C
#  define END_EXTERN_C
#  define EXTERN_C extern
#endif

/* Fallback definitions in case we don't have definitions from config.h.
   This should only matter for systems that don't use Configure and
   haven't been modified to define PERL_STATIC_INLINE yet.
*/
#if !defined(PERL_STATIC_INLINE)
#  ifdef HAS_STATIC_INLINE
#    define PERL_STATIC_INLINE static inline
#  else
#    define PERL_STATIC_INLINE static
#  endif
#endif

#ifdef PERL_GLOBAL_STRUCT
#  ifndef PERL_GET_VARS
#    ifdef PERL_GLOBAL_STRUCT_PRIVATE
       EXTERN_C struct perl_vars* Perl_GetVarsPrivate();
#      define PERL_GET_VARS() Perl_GetVarsPrivate() /* see miniperlmain.c */
#      ifndef PERLIO_FUNCS_CONST
#        define PERLIO_FUNCS_CONST /* Can't have these lying around. */
#      endif
#    else
#      define PERL_GET_VARS() PL_VarsPtr
#    endif
#  endif
#endif

#define pVAR    register struct perl_vars* my_vars PERL_UNUSED_DECL

#ifdef PERL_GLOBAL_STRUCT
#  define dVAR		pVAR    = (struct perl_vars*)PERL_GET_VARS()
#else
#  define dVAR		dNOOP
#endif

#ifdef PERL_IMPLICIT_CONTEXT
#  ifndef MULTIPLICITY
#    define MULTIPLICITY
#  endif
#  define tTHX	PerlInterpreter*
#  define pTHX  register tTHX my_perl PERL_UNUSED_DECL
#  define aTHX	my_perl
#  ifdef PERL_GLOBAL_STRUCT
#    define dTHXa(a)	dVAR; pTHX = (tTHX)a
#  else
#    define dTHXa(a)	pTHX = (tTHX)a
#  endif
#  ifdef PERL_GLOBAL_STRUCT
#    define dTHX		dVAR; pTHX = PERL_GET_THX
#  else
#    define dTHX		pTHX = PERL_GET_THX
#  endif
#  define pTHX_		pTHX,
#  define aTHX_		aTHX,
#  define pTHX_1	2
#  define pTHX_2	3
#  define pTHX_3	4
#  define pTHX_4	5
#  define pTHX_5	6
#  define pTHX_6	7
#  define pTHX_7	8
#  define pTHX_8	9
#  define pTHX_9	10
#  if defined(DEBUGGING) && !defined(PERL_TRACK_MEMPOOL)
#    define PERL_TRACK_MEMPOOL
#  endif
#else
#  undef PERL_TRACK_MEMPOOL
#endif

#define STATIC static

#ifndef PERL_CORE
/* Do not use these macros. They were part of PERL_OBJECT, which was an
 * implementation of multiplicity using C++ objects. They have been left
 * here solely for the sake of XS code which has incorrectly
 * cargo-culted them.
 */
#define CPERLscope(x) x
#define CPERLarg void
#define CPERLarg_
#define _CPERLarg
#define PERL_OBJECT_THIS
#define _PERL_OBJECT_THIS
#define PERL_OBJECT_THIS_
#define CALL_FPTR(fptr) (*fptr)
#define MEMBER_TO_FPTR(name) name
#endif /* !PERL_CORE */

#define CALLRUNOPS  PL_runops

#define CALLREGCOMP(sv, flags) Perl_pregcomp(aTHX_ (sv),(flags))

#define CALLREGCOMP_ENG(prog, sv, flags) (prog)->comp(aTHX_ sv, flags)
#define CALLREGEXEC(prog,stringarg,strend,strbeg,minend,screamer,data,flags) \
    RX_ENGINE(prog)->exec(aTHX_ (prog),(stringarg),(strend), \
        (strbeg),(minend),(screamer),(data),(flags))
#define CALLREG_INTUIT_START(prog,sv,strpos,strend,flags,data) \
    RX_ENGINE(prog)->intuit(aTHX_ (prog), (sv), (strpos), \
        (strend),(flags),(data))
#define CALLREG_INTUIT_STRING(prog) \
    RX_ENGINE(prog)->checkstr(aTHX_ (prog))

#define CALLREGFREE(prog) \
    Perl_pregfree(aTHX_ (prog))

#define CALLREGFREE_PVT(prog) \
    if(prog) RX_ENGINE(prog)->free(aTHX_ (prog))

#define CALLREG_NUMBUF_FETCH(rx,paren,usesv)                                \
    RX_ENGINE(rx)->numbered_buff_FETCH(aTHX_ (rx),(paren),(usesv))

#define CALLREG_NUMBUF_STORE(rx,paren,value) \
    RX_ENGINE(rx)->numbered_buff_STORE(aTHX_ (rx),(paren),(value))

#define CALLREG_NUMBUF_LENGTH(rx,sv,paren)                              \
    RX_ENGINE(rx)->numbered_buff_LENGTH(aTHX_ (rx),(sv),(paren))

#define CALLREG_NAMED_BUFF_FETCH(rx, key, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), (key), NULL, ((flags) | RXapif_FETCH))

#define CALLREG_NAMED_BUFF_STORE(rx, key, value, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), (key), (value), ((flags) | RXapif_STORE))

#define CALLREG_NAMED_BUFF_DELETE(rx, key, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx),(key), NULL, ((flags) | RXapif_DELETE))

#define CALLREG_NAMED_BUFF_CLEAR(rx, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), NULL, NULL, ((flags) | RXapif_CLEAR))

#define CALLREG_NAMED_BUFF_EXISTS(rx, key, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), (key), NULL, ((flags) | RXapif_EXISTS))

#define CALLREG_NAMED_BUFF_FIRSTKEY(rx, flags) \
    RX_ENGINE(rx)->named_buff_iter(aTHX_ (rx), NULL, ((flags) | RXapif_FIRSTKEY))

#define CALLREG_NAMED_BUFF_NEXTKEY(rx, lastkey, flags) \
    RX_ENGINE(rx)->named_buff_iter(aTHX_ (rx), (lastkey), ((flags) | RXapif_NEXTKEY))

#define CALLREG_NAMED_BUFF_SCALAR(rx, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), NULL, NULL, ((flags) | RXapif_SCALAR))

#define CALLREG_NAMED_BUFF_COUNT(rx) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), NULL, NULL, RXapif_REGNAMES_COUNT)

#define CALLREG_NAMED_BUFF_ALL(rx, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), NULL, NULL, flags)

#define CALLREG_PACKAGE(rx) \
    RX_ENGINE(rx)->qr_package(aTHX_ (rx))

#if defined(USE_ITHREADS)
#define CALLREGDUPE(prog,param) \
    Perl_re_dup(aTHX_ (prog),(param))

#define CALLREGDUPE_PVT(prog,param) \
    (prog ? RX_ENGINE(prog)->dupe(aTHX_ (prog),(param)) \
          : (REGEXP *)NULL)
#endif





/*
 * Because of backward compatibility reasons the PERL_UNUSED_DECL
 * cannot be changed from postfix to PERL_UNUSED_DECL(x).  Sigh.
 *
 * Note that there are C compilers such as MetroWerks CodeWarrior
 * which do not have an "inlined" way (like the gcc __attribute__) of
 * marking unused variables (they need e.g. a #pragma) and therefore
 * cpp macros like PERL_UNUSED_DECL cannot work for this purpose, even
 * if it were PERL_UNUSED_DECL(x), which it cannot be (see above).
 *
 */

#if defined(__SYMBIAN32__) && defined(__GNUC__)
#  ifdef __cplusplus
#    define PERL_UNUSED_DECL
#  else
#    define PERL_UNUSED_DECL __attribute__((unused))
#  endif
#endif

#ifndef PERL_UNUSED_DECL
#  if defined(HASATTRIBUTE_UNUSED) && !defined(__cplusplus)
#    define PERL_UNUSED_DECL __attribute__unused__
#  else
#    define PERL_UNUSED_DECL
#  endif
#endif

/* gcc -Wall:
 * for silencing unused variables that are actually used most of the time,
 * but we cannot quite get rid of, such as "ax" in PPCODE+noargs xsubs
 */
#ifndef PERL_UNUSED_ARG
#  if defined(lint) && defined(S_SPLINT_S) /* www.splint.org */
#    include <note.h>
#    define PERL_UNUSED_ARG(x) NOTE(ARGUNUSED(x))
#  else
#    define PERL_UNUSED_ARG(x) ((void)x)
#  endif
#endif
#ifndef PERL_UNUSED_VAR
#  define PERL_UNUSED_VAR(x) ((void)x)
#endif

#ifdef USE_ITHREADS
#  define PERL_UNUSED_CONTEXT PERL_UNUSED_ARG(my_perl)
#else
#  define PERL_UNUSED_CONTEXT
#endif

#define NOOP /*EMPTY*/(void)0
/* cea2e8a9dd23747f accidentally lost the comment originally from the first
   check in of thread.h, explaining why we need dNOOP at all:  */
/* Rats: if dTHR is just blank then the subsequent ";" throws an error */
/* Declaring a *function*, instead of a variable, ensures that we don't rely
   on being able to suppress "unused" warnings.  */
#define dNOOP extern int Perl___notused(void)

#ifndef pTHX
/* Don't bother defining tTHX and sTHX; using them outside
 * code guarded by PERL_IMPLICIT_CONTEXT is an error.
 */
#  define pTHX		void
#  define pTHX_
#  define aTHX
#  define aTHX_
#  define dTHXa(a)	dNOOP
#  define dTHX		dNOOP
#  define pTHX_1	1
#  define pTHX_2	2
#  define pTHX_3	3
#  define pTHX_4	4
#  define pTHX_5	5
#  define pTHX_6	6
#  define pTHX_7	7
#  define pTHX_8	8
#  define pTHX_9	9
#endif

#ifndef dVAR
#  define dVAR		dNOOP
#endif

/* these are only defined for compatibility; should not be used internally */
#if !defined(pTHXo) && !defined(PERL_CORE)
#  define pTHXo		pTHX
#  define pTHXo_	pTHX_
#  define aTHXo		aTHX
#  define aTHXo_	aTHX_
#  define dTHXo		dTHX
#  define dTHXoa(x)	dTHXa(x)
#endif

#ifndef pTHXx
#  define pTHXx		register PerlInterpreter *my_perl
#  define pTHXx_	pTHXx,
#  define aTHXx		my_perl
#  define aTHXx_	aTHXx,
#  define dTHXx		dTHX
#endif

/* Under PERL_IMPLICIT_SYS (used in Windows for fork emulation)
 * PerlIO_foo() expands to PL_StdIO->pFOO(PL_StdIO, ...).
 * dTHXs is therefore needed for all functions using PerlIO_foo(). */
#ifdef PERL_IMPLICIT_SYS
#  ifdef PERL_GLOBAL_STRUCT_PRIVATE
#    define dTHXs		dVAR; dTHX
#  else
#    define dTHXs		dTHX
#  endif
#else
#  ifdef PERL_GLOBAL_STRUCT_PRIVATE
#    define dTHXs		dVAR
#  else
#    define dTHXs		dNOOP
#  endif
#endif

/* Some platforms require marking function declarations
 * for them to be exportable.  Used in perlio.h, proto.h
 * is handled either by the makedef.pl or by defining the
 * PERL_CALLCONV to be something special.  See also the
 * definition of XS() in XSUB.h. */
#ifndef PERL_EXPORT_C
#  ifdef __cplusplus
#    define PERL_EXPORT_C extern "C"
#  else
#    define PERL_EXPORT_C extern
#  endif
#endif
#ifndef PERL_XS_EXPORT_C
#  ifdef __cplusplus
#    define PERL_XS_EXPORT_C extern "C"
#  else
#    define PERL_XS_EXPORT_C
#  endif
#endif

#ifdef OP_IN_REGISTER
#  ifdef __GNUC__
#    define stringify_immed(s) #s
#    define stringify(s) stringify_immed(s)
register struct op *Perl_op asm(stringify(OP_IN_REGISTER));
#  endif
#endif

/* gcc (-ansi) -pedantic doesn't allow gcc statement expressions,
 * g++ allows them but seems to have problems with them
 * (insane errors ensue).
 * g++ does not give insane errors now (RMB 2008-01-30, gcc 4.2.2).
 */
#if defined(PERL_GCC_PEDANTIC) || \
    (defined(__GNUC__) && defined(__cplusplus) && \
	((__GNUC__ < 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ < 2))))
#  ifndef PERL_GCC_BRACE_GROUPS_FORBIDDEN
#    define PERL_GCC_BRACE_GROUPS_FORBIDDEN
#  endif
#endif

#if defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN) && !defined(__cplusplus)
#  ifndef PERL_USE_GCC_BRACE_GROUPS
#    define PERL_USE_GCC_BRACE_GROUPS
#  endif
#endif

/*
 * STMT_START { statements; } STMT_END;
 * can be used as a single statement, as in
 * if (x) STMT_START { ... } STMT_END; else ...
 *
 * Trying to select a version that gives no warnings...
 */
#if !(defined(STMT_START) && defined(STMT_END))
# ifdef PERL_USE_GCC_BRACE_GROUPS
#   define STMT_START	(void)(	/* gcc supports "({ STATEMENTS; })" */
#   define STMT_END	)
# else
   /* Now which other defined()s do we need here ??? */
#  if (VOIDFLAGS) && (defined(sun) || defined(__sun__)) && !defined(__GNUC__)
#   define STMT_START	if (1)
#   define STMT_END	else (void)0
#  else
#   define STMT_START	do
#   define STMT_END	while (0)
#  endif
# endif
#endif

#ifndef BYTEORDER  /* Should never happen -- byteorder is in config.h */
#   define BYTEORDER 0x1234
#endif

/* Overall memory policy? */
#ifndef CONSERVATIVE
#   define LIBERAL 1
#endif

#if 'A' == 65 && 'I' == 73 && 'J' == 74 && 'Z' == 90
#define ASCIIish
#else
#undef  ASCIIish
#endif

/*
 * The following contortions are brought to you on behalf of all the
 * standards, semi-standards, de facto standards, not-so-de-facto standards
 * of the world, as well as all the other botches anyone ever thought of.
 * The basic theory is that if we work hard enough here, the rest of the
 * code can be a lot prettier.  Well, so much for theory.  Sorry, Henry...
 */

/* define this once if either system, instead of cluttering up the src */
#if defined(MSDOS) || defined(atarist) || defined(WIN32) || defined(NETWARE)
#define DOSISH 1
#endif

#if defined(__STDC__) || defined(_AIX) || defined(__stdc__) || defined(__cplusplus) || defined(EPOC) || defined(NETWARE) || defined(__SYMBIAN32__)
# define STANDARD_C 1
#endif

#if defined(__cplusplus) || defined(WIN32) || defined(__sgi) || defined(__EMX__) || defined(__DGUX) || defined(EPOC) || defined(__QNX__) || defined(NETWARE) || defined(PERL_MICRO)
# define DONT_DECLARE_STD 1
#endif

#if defined(HASVOLATILE) || defined(STANDARD_C)
#	define VOL volatile
#else
#   define VOL
#endif

#define TAINT		(PL_tainted = TRUE)
#define TAINT_NOT	(PL_tainted = FALSE)
#define TAINT_IF(c)	if (c) { PL_tainted = TRUE; }
#define TAINT_ENV()	if (PL_tainting) { taint_env(); }
#define TAINT_PROPER(s)	if (PL_tainting) { taint_proper(NULL, s); }

/* flags used internally only within pp_subst and pp_substcont */
#ifdef PERL_CORE
#  define SUBST_TAINT_STR      1	/* string tainted */
#  define SUBST_TAINT_PAT      2	/* pattern tainted */
#  define SUBST_TAINT_REPL     4	/* replacement tainted */
#  define SUBST_TAINT_RETAINT  8	/* use re'taint' in scope */
#  define SUBST_TAINT_BOOLRET 16	/* return is boolean (don't taint) */
#endif

/* XXX All process group stuff is handled in pp_sys.c.  Should these
   defines move there?  If so, I could simplify this a lot. --AD  9/96.
*/
/* Process group stuff changed from traditional BSD to POSIX.
   perlfunc.pod documents the traditional BSD-style syntax, so we'll
   try to preserve that, if possible.
*/
#ifdef HAS_SETPGID
#  define BSD_SETPGRP(pid, pgrp)	setpgid((pid), (pgrp))
#else
#  if defined(HAS_SETPGRP) && defined(USE_BSD_SETPGRP)
#    define BSD_SETPGRP(pid, pgrp)	setpgrp((pid), (pgrp))
#  else
#    ifdef HAS_SETPGRP2  /* DG/UX */
#      define BSD_SETPGRP(pid, pgrp)	setpgrp2((pid), (pgrp))
#    endif
#  endif
#endif
#if defined(BSD_SETPGRP) && !defined(HAS_SETPGRP)
#  define HAS_SETPGRP  /* Well, effectively it does . . . */
#endif

/* getpgid isn't POSIX, but at least Solaris and Linux have it, and it makes
    our life easier :-) so we'll try it.
*/
#ifdef HAS_GETPGID
#  define BSD_GETPGRP(pid)		getpgid((pid))
#else
#  if defined(HAS_GETPGRP) && defined(USE_BSD_GETPGRP)
#    define BSD_GETPGRP(pid)		getpgrp((pid))
#  else
#    ifdef HAS_GETPGRP2  /* DG/UX */
#      define BSD_GETPGRP(pid)		getpgrp2((pid))
#    endif
#  endif
#endif
#if defined(BSD_GETPGRP) && !defined(HAS_GETPGRP)
#  define HAS_GETPGRP  /* Well, effectively it does . . . */
#endif

/* These are not exact synonyms, since setpgrp() and getpgrp() may
   have different behaviors, but perl.h used to define USE_BSDPGRP
   (prior to 5.003_05) so some extension might depend on it.
*/
#if defined(USE_BSD_SETPGRP) || defined(USE_BSD_GETPGRP)
#  ifndef USE_BSDPGRP
#    define USE_BSDPGRP
#  endif
#endif

/* HP-UX 10.X CMA (Common Multithreaded Architecture) insists that
   pthread.h must be included before all other header files.
*/
#if defined(USE_ITHREADS) && defined(PTHREAD_H_FIRST) && defined(I_PTHREAD)
#  include <pthread.h>
#endif

#ifndef _TYPES_		/* If types.h defines this it's easy. */
#   ifndef major		/* Does everyone's types.h define this? */
#	include <sys/types.h>
#   endif
#endif

#ifdef __cplusplus
#  ifndef I_STDARG
#    define I_STDARG 1
#  endif
#endif

#ifdef I_STDARG
#  include <stdarg.h>
#else
#  ifdef I_VARARGS
#    include <varargs.h>
#  endif
#endif

#ifdef USE_NEXT_CTYPE

#if NX_CURRENT_COMPILER_RELEASE >= 500
#  include <bsd/ctypes.h>
#else
#  if NX_CURRENT_COMPILER_RELEASE >= 400
#    include <objc/NXCType.h>
#  else /*  NX_CURRENT_COMPILER_RELEASE < 400 */
#    include <appkit/NXCType.h>
#  endif /*  NX_CURRENT_COMPILER_RELEASE >= 400 */
#endif /*  NX_CURRENT_COMPILER_RELEASE >= 500 */

#else /* !USE_NEXT_CTYPE */
#include <ctype.h>
#endif /* USE_NEXT_CTYPE */

#ifdef METHOD 	/* Defined by OSF/1 v3.0 by ctype.h */
#undef METHOD
#endif

#ifdef PERL_MICRO
#   define NO_LOCALE
#endif

#ifdef I_LOCALE
#   include <locale.h>
#endif

#if !defined(NO_LOCALE) && defined(HAS_SETLOCALE)
#   define USE_LOCALE
#   if !defined(NO_LOCALE_COLLATE) && defined(LC_COLLATE) \
       && defined(HAS_STRXFRM)
#	define USE_LOCALE_COLLATE
#   endif
#   if !defined(NO_LOCALE_CTYPE) && defined(LC_CTYPE)
#	define USE_LOCALE_CTYPE
#   endif
#   if !defined(NO_LOCALE_NUMERIC) && defined(LC_NUMERIC)
#	define USE_LOCALE_NUMERIC
#   endif
#endif /* !NO_LOCALE && HAS_SETLOCALE */

#include <setjmp.h>

#ifdef I_SYS_PARAM
#   ifdef PARAM_NEEDS_TYPES
#	include <sys/types.h>
#   endif
#   include <sys/param.h>
#endif

/* Use all the "standard" definitions? */
#if defined(STANDARD_C) && defined(I_STDLIB)
#   include <stdlib.h>
#endif

/* If this causes problems, set i_unistd=undef in the hint file.  */
#ifdef I_UNISTD
#   include <unistd.h>
#endif

/* for WCOREDUMP */
#ifdef I_SYS_WAIT
#   include <sys/wait.h>
#endif

#ifdef __SYMBIAN32__
#   undef _SC_ARG_MAX /* Symbian has _SC_ARG_MAX but no sysconf() */
#endif

#if defined(HAS_SYSCALL) && !defined(HAS_SYSCALL_PROTO) && !defined(PERL_MICRO)
EXTERN_C int syscall(int, ...);
#endif

#if defined(HAS_USLEEP) && !defined(HAS_USLEEP_PROTO) && !defined(PERL_MICRO)
EXTERN_C int usleep(unsigned int);
#endif

/* Funky places that do not have socket stuff. */
#if defined(__LIBCATAMOUNT__)
#  define MYSWAP
#endif

#ifdef PERL_MICRO /* Last chance to export Perl_my_swap */
#  define MYSWAP
#endif

#ifdef PERL_CORE

/* macros for correct constant construction */
# if INTSIZE >= 2
#  define U16_CONST(x) ((U16)x##U)
# else
#  define U16_CONST(x) ((U16)x##UL)
# endif

# if INTSIZE >= 4
#  define U32_CONST(x) ((U32)x##U)
# else
#  define U32_CONST(x) ((U32)x##UL)
# endif

# ifdef HAS_QUAD
#  if INTSIZE >= 8
#   define U64_CONST(x) ((U64)x##U)
#  elif LONGSIZE >= 8
#   define U64_CONST(x) ((U64)x##UL)
#  elif QUADKIND == QUAD_IS_LONG_LONG
#   define U64_CONST(x) ((U64)x##ULL)
#  else /* best guess we can make */
#   define U64_CONST(x) ((U64)x##UL)
#  endif
# endif

/* byte-swapping functions for big-/little-endian conversion */
# define _swab_16_(x) ((U16)( \
         (((U16)(x) & U16_CONST(0x00ff)) << 8) | \
         (((U16)(x) & U16_CONST(0xff00)) >> 8) ))

# define _swab_32_(x) ((U32)( \
         (((U32)(x) & U32_CONST(0x000000ff)) << 24) | \
         (((U32)(x) & U32_CONST(0x0000ff00)) <<  8) | \
         (((U32)(x) & U32_CONST(0x00ff0000)) >>  8) | \
         (((U32)(x) & U32_CONST(0xff000000)) >> 24) ))

# ifdef HAS_QUAD
#  define _swab_64_(x) ((U64)( \
          (((U64)(x) & U64_CONST(0x00000000000000ff)) << 56) | \
          (((U64)(x) & U64_CONST(0x000000000000ff00)) << 40) | \
          (((U64)(x) & U64_CONST(0x0000000000ff0000)) << 24) | \
          (((U64)(x) & U64_CONST(0x00000000ff000000)) <<  8) | \
          (((U64)(x) & U64_CONST(0x000000ff00000000)) >>  8) | \
          (((U64)(x) & U64_CONST(0x0000ff0000000000)) >> 24) | \
          (((U64)(x) & U64_CONST(0x00ff000000000000)) >> 40) | \
          (((U64)(x) & U64_CONST(0xff00000000000000)) >> 56) ))
# endif

/*----------------------------------------------------------------------------*/
# if BYTEORDER == 0x1234 || BYTEORDER == 0x12345678  /*     little-endian     */
/*----------------------------------------------------------------------------*/
#  define my_htole16(x)		(x)
#  define my_letoh16(x)		(x)
#  define my_htole32(x)		(x)
#  define my_letoh32(x)		(x)
#  define my_htobe16(x)		_swab_16_(x)
#  define my_betoh16(x)		_swab_16_(x)
#  define my_htobe32(x)		_swab_32_(x)
#  define my_betoh32(x)		_swab_32_(x)
#  ifdef HAS_QUAD
#   define my_htole64(x)	(x)
#   define my_letoh64(x)	(x)
#   define my_htobe64(x)	_swab_64_(x)
#   define my_betoh64(x)	_swab_64_(x)
#  endif
#  define my_htoles(x)		(x)
#  define my_letohs(x)		(x)
#  define my_htolei(x)		(x)
#  define my_letohi(x)		(x)
#  define my_htolel(x)		(x)
#  define my_letohl(x)		(x)
#  if SHORTSIZE == 1
#   define my_htobes(x)		(x)
#   define my_betohs(x)		(x)
#  elif SHORTSIZE == 2
#   define my_htobes(x)		_swab_16_(x)
#   define my_betohs(x)		_swab_16_(x)
#  elif SHORTSIZE == 4
#   define my_htobes(x)		_swab_32_(x)
#   define my_betohs(x)		_swab_32_(x)
#  elif SHORTSIZE == 8
#   define my_htobes(x)		_swab_64_(x)
#   define my_betohs(x)		_swab_64_(x)
#  else
#   define PERL_NEED_MY_HTOBES
#   define PERL_NEED_MY_BETOHS
#  endif
#  if INTSIZE == 1
#   define my_htobei(x)		(x)
#   define my_betohi(x)		(x)
#  elif INTSIZE == 2
#   define my_htobei(x)		_swab_16_(x)
#   define my_betohi(x)		_swab_16_(x)
#  elif INTSIZE == 4
#   define my_htobei(x)		_swab_32_(x)
#   define my_betohi(x)		_swab_32_(x)
#  elif INTSIZE == 8
#   define my_htobei(x)		_swab_64_(x)
#   define my_betohi(x)		_swab_64_(x)
#  else
#   define PERL_NEED_MY_HTOBEI
#   define PERL_NEED_MY_BETOHI
#  endif
#  if LONGSIZE == 1
#   define my_htobel(x)		(x)
#   define my_betohl(x)		(x)
#  elif LONGSIZE == 2
#   define my_htobel(x)		_swab_16_(x)
#   define my_betohl(x)		_swab_16_(x)
#  elif LONGSIZE == 4
#   define my_htobel(x)		_swab_32_(x)
#   define my_betohl(x)		_swab_32_(x)
#  elif LONGSIZE == 8
#   define my_htobel(x)		_swab_64_(x)
#   define my_betohl(x)		_swab_64_(x)
#  else
#   define PERL_NEED_MY_HTOBEL
#   define PERL_NEED_MY_BETOHL
#  endif
#  define my_htolen(p,n)	NOOP
#  define my_letohn(p,n)	NOOP
#  define my_htoben(p,n)	my_swabn(p,n)
#  define my_betohn(p,n)	my_swabn(p,n)
/*----------------------------------------------------------------------------*/
# elif BYTEORDER == 0x4321 || BYTEORDER == 0x87654321  /*     big-endian      */
/*----------------------------------------------------------------------------*/
#  define my_htobe16(x)		(x)
#  define my_betoh16(x)		(x)
#  define my_htobe32(x)		(x)
#  define my_betoh32(x)		(x)
#  define my_htole16(x)		_swab_16_(x)
#  define my_letoh16(x)		_swab_16_(x)
#  define my_htole32(x)		_swab_32_(x)
#  define my_letoh32(x)		_swab_32_(x)
#  ifdef HAS_QUAD
#   define my_htobe64(x)	(x)
#   define my_betoh64(x)	(x)
#   define my_htole64(x)	_swab_64_(x)
#   define my_letoh64(x)	_swab_64_(x)
#  endif
#  define my_htobes(x)		(x)
#  define my_betohs(x)		(x)
#  define my_htobei(x)		(x)
#  define my_betohi(x)		(x)
#  define my_htobel(x)		(x)
#  define my_betohl(x)		(x)
#  if SHORTSIZE == 1
#   define my_htoles(x)		(x)
#   define my_letohs(x)		(x)
#  elif SHORTSIZE == 2
#   define my_htoles(x)		_swab_16_(x)
#   define my_letohs(x)		_swab_16_(x)
#  elif SHORTSIZE == 4
#   define my_htoles(x)		_swab_32_(x)
#   define my_letohs(x)		_swab_32_(x)
#  elif SHORTSIZE == 8
#   define my_htoles(x)		_swab_64_(x)
#   define my_letohs(x)		_swab_64_(x)
#  else
#   define PERL_NEED_MY_HTOLES
#   define PERL_NEED_MY_LETOHS
#  endif
#  if INTSIZE == 1
#   define my_htolei(x)		(x)
#   define my_letohi(x)		(x)
#  elif INTSIZE == 2
#   define my_htolei(x)		_swab_16_(x)
#   define my_letohi(x)		_swab_16_(x)
#  elif INTSIZE == 4
#   define my_htolei(x)		_swab_32_(x)
#   define my_letohi(x)		_swab_32_(x)
#  elif INTSIZE == 8
#   define my_htolei(x)		_swab_64_(x)
#   define my_letohi(x)		_swab_64_(x)
#  else
#   define PERL_NEED_MY_HTOLEI
#   define PERL_NEED_MY_LETOHI
#  endif
#  if LONGSIZE == 1
#   define my_htolel(x)		(x)
#   define my_letohl(x)		(x)
#  elif LONGSIZE == 2
#   define my_htolel(x)		_swab_16_(x)
#   define my_letohl(x)		_swab_16_(x)
#  elif LONGSIZE == 4
#   define my_htolel(x)		_swab_32_(x)
#   define my_letohl(x)		_swab_32_(x)
#  elif LONGSIZE == 8
#   define my_htolel(x)		_swab_64_(x)
#   define my_letohl(x)		_swab_64_(x)
#  else
#   define PERL_NEED_MY_HTOLEL
#   define PERL_NEED_MY_LETOHL
#  endif
#  define my_htolen(p,n)	my_swabn(p,n)
#  define my_letohn(p,n)	my_swabn(p,n)
#  define my_htoben(p,n)	NOOP
#  define my_betohn(p,n)	NOOP
/*----------------------------------------------------------------------------*/
# else /*                       all other byte-orders                         */
/*----------------------------------------------------------------------------*/
#  define PERL_NEED_MY_HTOLE16
#  define PERL_NEED_MY_LETOH16
#  define PERL_NEED_MY_HTOBE16
#  define PERL_NEED_MY_BETOH16
#  define PERL_NEED_MY_HTOLE32
#  define PERL_NEED_MY_LETOH32
#  define PERL_NEED_MY_HTOBE32
#  define PERL_NEED_MY_BETOH32
#  ifdef HAS_QUAD
#   define PERL_NEED_MY_HTOLE64
#   define PERL_NEED_MY_LETOH64
#   define PERL_NEED_MY_HTOBE64
#   define PERL_NEED_MY_BETOH64
#  endif
#  define PERL_NEED_MY_HTOLES
#  define PERL_NEED_MY_LETOHS
#  define PERL_NEED_MY_HTOBES
#  define PERL_NEED_MY_BETOHS
#  define PERL_NEED_MY_HTOLEI
#  define PERL_NEED_MY_LETOHI
#  define PERL_NEED_MY_HTOBEI
#  define PERL_NEED_MY_BETOHI
#  define PERL_NEED_MY_HTOLEL
#  define PERL_NEED_MY_LETOHL
#  define PERL_NEED_MY_HTOBEL
#  define PERL_NEED_MY_BETOHL
/*----------------------------------------------------------------------------*/
# endif /*                     end of byte-order macros                       */
/*----------------------------------------------------------------------------*/

/* The old value was hard coded at 1008. (4096-16) seems to be a bit faster,
   at least on FreeBSD.  YMMV, so experiment.  */
#ifndef PERL_ARENA_SIZE
#define PERL_ARENA_SIZE 4080
#endif

/* Maximum level of recursion */
#ifndef PERL_SUB_DEPTH_WARN
#define PERL_SUB_DEPTH_WARN 100
#endif

#endif /* PERL_CORE */

/* We no longer default to creating a new SV for GvSV.
   Do this before embed.  */
#ifndef PERL_CREATE_GVSV
#  ifndef PERL_DONT_CREATE_GVSV
#    define PERL_DONT_CREATE_GVSV
#  endif
#endif

#if !defined(HAS_WAITPID) && !defined(HAS_WAIT4) || defined(HAS_WAITPID_RUNTIME)
#define PERL_USES_PL_PIDSTATUS
#endif

#if !defined(OS2) && !defined(WIN32) && !defined(DJGPP) && !defined(EPOC) && !defined(__SYMBIAN32__)
#define PERL_DEFAULT_DO_EXEC3_IMPLEMENTATION
#endif

#define MEM_SIZE Size_t

/* Round all values passed to malloc up, by default to a multiple of
   sizeof(size_t)
*/
#ifndef PERL_STRLEN_ROUNDUP_QUANTUM
#define PERL_STRLEN_ROUNDUP_QUANTUM Size_t_size
#endif

/* sv_grow() will expand strings by at least a certain percentage of
   the previously *used* length to avoid excessive calls to realloc().
   The default is 25% of the current length.
*/
#ifndef PERL_STRLEN_EXPAND_SHIFT
#  define PERL_STRLEN_EXPAND_SHIFT 2
#endif

#if defined(STANDARD_C) && defined(I_STDDEF)
#   include <stddef.h>
#   define STRUCT_OFFSET(s,m)  offsetof(s,m)
#else
#   define STRUCT_OFFSET(s,m)  (Size_t)(&(((s *)0)->m))
#endif

#ifndef __SYMBIAN32__
#  if defined(I_STRING) || defined(__cplusplus)
#     include <string.h>
#  else
#     include <strings.h>
#  endif
#endif

/* This comes after <stdlib.h> so we don't try to change the standard
 * library prototypes; we'll use our own in proto.h instead. */

#ifdef MYMALLOC
#  ifdef PERL_POLLUTE_MALLOC
#   ifndef PERL_EXTMALLOC_DEF
#    define Perl_malloc		malloc
#    define Perl_calloc		calloc
#    define Perl_realloc	realloc
#    define Perl_mfree		free
#   endif
#  else
#    define EMBEDMYMALLOC	/* for compatibility */
#  endif

#  define safemalloc  Perl_malloc
#  define safecalloc  Perl_calloc
#  define saferealloc Perl_realloc
#  define safefree    Perl_mfree
#  define CHECK_MALLOC_TOO_LATE_FOR_(code)	STMT_START {		\
	if (!PL_tainting && MallocCfg_ptr[MallocCfg_cfg_env_read])	\
		code;							\
    } STMT_END
#  define CHECK_MALLOC_TOO_LATE_FOR(ch)				\
	CHECK_MALLOC_TOO_LATE_FOR_(MALLOC_TOO_LATE_FOR(ch))
#  define panic_write2(s)		write(2, s, strlen(s))
#  define CHECK_MALLOC_TAINT(newval)				\
	CHECK_MALLOC_TOO_LATE_FOR_(				\
		if (newval) {					\
		  panic_write2("panic: tainting with $ENV{PERL_MALLOC_OPT}\n");\
		  exit(1); })
#  define MALLOC_CHECK_TAINT(argc,argv,env)	STMT_START {	\
	if (doing_taint(argc,argv,env)) {			\
		MallocCfg_ptr[MallocCfg_skip_cfg_env] = 1;	\
    }} STMT_END;
#else  /* MYMALLOC */
#  define safemalloc  safesysmalloc
#  define safecalloc  safesyscalloc
#  define saferealloc safesysrealloc
#  define safefree    safesysfree
#  define CHECK_MALLOC_TOO_LATE_FOR(ch)		((void)0)
#  define CHECK_MALLOC_TAINT(newval)		((void)0)
#  define MALLOC_CHECK_TAINT(argc,argv,env)
#endif /* MYMALLOC */

/* diag_listed_as: "-T" is on the #! line, it must also be used on the command line */
#define TOO_LATE_FOR_(ch,what)	Perl_croak(aTHX_ "\"-%c\" is on the #! line, it must also be used on the command line%s", (char)(ch), what)
#define TOO_LATE_FOR(ch)	TOO_LATE_FOR_(ch, "")
#define MALLOC_TOO_LATE_FOR(ch)	TOO_LATE_FOR_(ch, " with $ENV{PERL_MALLOC_OPT}")
#define MALLOC_CHECK_TAINT2(argc,argv)	MALLOC_CHECK_TAINT(argc,argv,NULL)

#if !defined(HAS_STRCHR) && defined(HAS_INDEX) && !defined(strchr)
#define strchr index
#define strrchr rindex
#endif

#ifdef I_MEMORY
#  include <memory.h>
#endif

#ifdef HAS_MEMCPY
#  if !defined(STANDARD_C) && !defined(I_STRING) && !defined(I_MEMORY)
#    ifndef memcpy
        extern char * memcpy (char*, char*, int);
#    endif
#  endif
#else
#   ifndef memcpy
#	ifdef HAS_BCOPY
#	    define memcpy(d,s,l) bcopy(s,d,l)
#	else
#	    define memcpy(d,s,l) my_bcopy(s,d,l)
#	endif
#   endif
#endif /* HAS_MEMCPY */

#ifdef HAS_MEMSET
#  if !defined(STANDARD_C) && !defined(I_STRING) && !defined(I_MEMORY)
#    ifndef memset
	extern char *memset (char*, int, int);
#    endif
#  endif
#else
#  undef  memset
#  define memset(d,c,l) my_memset(d,c,l)
#endif /* HAS_MEMSET */

#if !defined(HAS_MEMMOVE) && !defined(memmove)
#   if defined(HAS_BCOPY) && defined(HAS_SAFE_BCOPY)
#	define memmove(d,s,l) bcopy(s,d,l)
#   else
#	if defined(HAS_MEMCPY) && defined(HAS_SAFE_MEMCPY)
#	    define memmove(d,s,l) memcpy(d,s,l)
#	else
#	    define memmove(d,s,l) my_bcopy(s,d,l)
#	endif
#   endif
#endif

#if defined(mips) && defined(ultrix) && !defined(__STDC__)
#   undef HAS_MEMCMP
#endif

#if defined(HAS_MEMCMP) && defined(HAS_SANE_MEMCMP)
#  if !defined(STANDARD_C) && !defined(I_STRING) && !defined(I_MEMORY)
#    ifndef memcmp
	extern int memcmp (char*, char*, int);
#    endif
#  endif
#  ifdef BUGGY_MSC
#    pragma function(memcmp)
#  endif
#else
#   ifndef memcmp
#	define memcmp 	my_memcmp
#   endif
#endif /* HAS_MEMCMP && HAS_SANE_MEMCMP */

#ifndef memzero
#   ifdef HAS_MEMSET
#	define memzero(d,l) memset(d,0,l)
#   else
#	ifdef HAS_BZERO
#	    define memzero(d,l) bzero(d,l)
#	else
#	    define memzero(d,l) my_bzero(d,l)
#	endif
#   endif
#endif

#ifndef PERL_MICRO
#ifndef memchr
#   ifndef HAS_MEMCHR
#       define memchr(s,c,n) ninstr((char*)(s), ((char*)(s)) + n, &(c), &(c) + 1)
#   endif
#endif
#endif

#ifndef HAS_BCMP
#   ifndef bcmp
#	define bcmp(s1,s2,l) memcmp(s1,s2,l)
#   endif
#endif /* !HAS_BCMP */

#ifdef I_NETINET_IN
#   include <netinet/in.h>
#endif

#ifdef I_ARPA_INET
#   include <arpa/inet.h>
#endif

#if defined(SF_APPEND) && defined(USE_SFIO) && defined(I_SFIO)
/* <sfio.h> defines SF_APPEND and <sys/stat.h> might define SF_APPEND
 * (the neo-BSD seem to do this).  */
#   undef SF_APPEND
#endif

#ifdef I_SYS_STAT
#   include <sys/stat.h>
#endif

/* Microsoft VC's sys/stat.h defines all S_Ixxx macros except S_IFIFO.
   This definition should ideally go into win32/win32.h, but S_IFIFO is
   used later here in perl.h before win32/win32.h is being included. */
#if !defined(S_IFIFO) && defined(_S_IFIFO)
#   define S_IFIFO _S_IFIFO
#endif

/* The stat macros for Amdahl UTS, Unisoft System V/88 (and derivatives
   like UTekV) are broken, sometimes giving false positives.  Undefine
   them here and let the code below set them to proper values.

   The ghs macro stands for GreenHills Software C-1.8.5 which
   is the C compiler for sysV88 and the various derivatives.
   This header file bug is corrected in gcc-2.5.8 and later versions.
   --Kaveh Ghazi (ghazi@noc.rutgers.edu) 10/3/94.  */

#if defined(uts) || (defined(m88k) && defined(ghs))
#   undef S_ISDIR
#   undef S_ISCHR
#   undef S_ISBLK
#   undef S_ISREG
#   undef S_ISFIFO
#   undef S_ISLNK
#endif

#ifdef I_TIME
#   include <time.h>
#endif

#ifdef I_SYS_TIME
#   ifdef I_SYS_TIME_KERNEL
#	define KERNEL
#   endif
#   include <sys/time.h>
#   ifdef I_SYS_TIME_KERNEL
#	undef KERNEL
#   endif
#endif

#if defined(HAS_TIMES) && defined(I_SYS_TIMES)
#    include <sys/times.h>
#endif

#if defined(HAS_STRERROR) && (!defined(HAS_MKDIR) || !defined(HAS_RMDIR))
#   undef HAS_STRERROR
#endif

#include <errno.h>

#if defined(WIN32) && defined(PERL_IMPLICIT_SYS)
#  define WIN32SCK_IS_STDSCK		/* don't pull in custom wsock layer */
#endif

/* In Tru64 use the 4.4BSD struct msghdr, not the 4.3 one.
 * This is important for using IPv6.
 * For OSF/1 3.2, however, defining _SOCKADDR_LEN would be
 * a bad idea since it breaks send() and recv(). */
#if defined(__osf__) && defined(__alpha) && !defined(_SOCKADDR_LEN) && !defined(DEC_OSF1_3_X)
#   define _SOCKADDR_LEN
#endif

#if defined(HAS_SOCKET) && !defined(WIN32) /* WIN32 handles sockets via win32.h */
# include <sys/socket.h>
# if defined(USE_SOCKS) && defined(I_SOCKS)
#   if !defined(INCLUDE_PROTOTYPES)
#       define INCLUDE_PROTOTYPES /* for <socks.h> */
#       define PERL_SOCKS_NEED_PROTOTYPES
#   endif
#   include <socks.h>
#   ifdef PERL_SOCKS_NEED_PROTOTYPES /* keep cpp space clean */
#       undef INCLUDE_PROTOTYPES
#       undef PERL_SOCKS_NEED_PROTOTYPES
#   endif
# endif
# ifdef I_NETDB
#  ifdef NETWARE
#   include<stdio.h>
#  endif
#  include <netdb.h>
# endif
# ifndef ENOTSOCK
#  ifdef I_NET_ERRNO
#   include <net/errno.h>
#  endif
# endif
#endif

/* sockatmark() is so new (2001) that many places might have it hidden
 * behind some -D_BLAH_BLAH_SOURCE guard.  The __THROW magic is required
 * e.g. in Gentoo, see http://bugs.gentoo.org/show_bug.cgi?id=12605 */
#if defined(HAS_SOCKATMARK) && !defined(HAS_SOCKATMARK_PROTO)
# if defined(__THROW) && defined(__GLIBC__)
int sockatmark(int) __THROW;
# else
int sockatmark(int);
# endif
#endif

#if defined(__osf__) && defined(__cplusplus) && !defined(_XOPEN_SOURCE_EXTENDED) /* Tru64 "cxx" (C++), see hints/dec_osf.sh for why the _XOPEN_SOURCE_EXTENDED cannot be defined. */
EXTERN_C int fchdir(int);
EXTERN_C int flock(int, int);
EXTERN_C int fseeko(FILE *, off_t, int);
EXTERN_C off_t ftello(FILE *);
#endif

#if defined(__SUNPRO_CC) /* SUNWspro CC (C++) */
EXTERN_C char *crypt(const char *, const char *);
EXTERN_C char **environ;
#endif

#if defined(__cplusplus)
#  if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
EXTERN_C char **environ;
#  elif defined(__CYGWIN__)
EXTERN_C char *crypt(const char *, const char *);
#endif
#endif

#ifdef SETERRNO
# undef SETERRNO  /* SOCKS might have defined this */
#endif

#ifdef VMS
#   define SETERRNO(errcode,vmserrcode) \
	STMT_START {			\
	    set_errno(errcode);		\
	    set_vaxc_errno(vmserrcode);	\
	} STMT_END
#   define dSAVEDERRNO    int saved_errno; unsigned saved_vms_errno
#   define dSAVE_ERRNO    int saved_errno = errno; unsigned saved_vms_errno = vaxc$errno
#   define SAVE_ERRNO     ( saved_errno = errno, saved_vms_errno = vaxc$errno )
#   define RESTORE_ERRNO  SETERRNO(saved_errno, saved_vms_errno)

#   define LIB_INVARG 		LIB$_INVARG
#   define RMS_DIR    		RMS$_DIR
#   define RMS_FAC    		RMS$_FAC
#   define RMS_FEX    		RMS$_FEX
#   define RMS_FNF    		RMS$_FNF
#   define RMS_IFI    		RMS$_IFI
#   define RMS_ISI    		RMS$_ISI
#   define RMS_PRV    		RMS$_PRV
#   define SS_ACCVIO      	SS$_ACCVIO
#   define SS_DEVOFFLINE	SS$_DEVOFFLINE
#   define SS_IVCHAN  		SS$_IVCHAN
#   define SS_NORMAL  		SS$_NORMAL
#else
#   define SETERRNO(errcode,vmserrcode) (errno = (errcode))
#   define dSAVEDERRNO    int saved_errno
#   define dSAVE_ERRNO    int saved_errno = errno
#   define SAVE_ERRNO     (saved_errno = errno)
#   define RESTORE_ERRNO  (errno = saved_errno)

#   define LIB_INVARG 		0
#   define RMS_DIR    		0
#   define RMS_FAC    		0
#   define RMS_FEX    		0
#   define RMS_FNF    		0
#   define RMS_IFI    		0
#   define RMS_ISI    		0
#   define RMS_PRV    		0
#   define SS_ACCVIO      	0
#   define SS_DEVOFFLINE	0
#   define SS_IVCHAN  		0
#   define SS_NORMAL  		0
#endif

#define ERRSV GvSVn(PL_errgv)

#define CLEAR_ERRSV() STMT_START {					\
    if (!GvSV(PL_errgv)) {						\
	sv_setpvs(GvSV(gv_add_by_type(PL_errgv, SVt_PV)), "");		\
    } else if (SvREADONLY(GvSV(PL_errgv))) {				\
	SvREFCNT_dec(GvSV(PL_errgv));					\
	GvSV(PL_errgv) = newSVpvs("");					\
    } else {								\
	SV *const errsv = GvSV(PL_errgv);				\
	sv_setpvs(errsv, "");						\
	if (SvMAGICAL(errsv)) {						\
	    mg_free(errsv);						\
	}								\
	SvPOK_only(errsv);						\
    }									\
    } STMT_END


#ifdef PERL_CORE
# define DEFSV (0 + GvSVn(PL_defgv))
# define DEFSV_set(sv) \
    (SvREFCNT_dec(GvSV(PL_defgv)), GvSV(PL_defgv) = SvREFCNT_inc(sv))
# define SAVE_DEFSV                \
    (                               \
	save_gp(PL_defgv, 0),        \
	GvINTRO_off(PL_defgv),        \
	SAVEGENERICSV(GvSV(PL_defgv)), \
	GvSV(PL_defgv) = NULL           \
    )
#else
# define DEFSV GvSVn(PL_defgv)
# define DEFSV_set(sv) (GvSV(PL_defgv) = (sv))
# define SAVE_DEFSV SAVESPTR(GvSV(PL_defgv))
#endif

#define ERRHV GvHV(PL_errgv)	/* XXX unused, here for compatibility */

#ifndef errno
	extern int errno;     /* ANSI allows errno to be an lvalue expr.
			       * For example in multithreaded environments
			       * something like this might happen:
			       * extern int *_errno(void);
			       * #define errno (*_errno()) */
#endif

#ifdef HAS_STRERROR
#       ifdef VMS
	char *strerror (int,...);
#       else
#ifndef DONT_DECLARE_STD
	char *strerror (int);
#endif
#       endif
#       ifndef Strerror
#           define Strerror strerror
#       endif
#else
#    ifdef HAS_SYS_ERRLIST
	extern int sys_nerr;
	extern char *sys_errlist[];
#       ifndef Strerror
#           define Strerror(e) \
		((e) < 0 || (e) >= sys_nerr ? "(unknown)" : sys_errlist[e])
#       endif
#   endif
#endif

#ifdef I_SYS_IOCTL
#   ifndef _IOCTL_
#	include <sys/ioctl.h>
#   endif
#endif

#if defined(mc300) || defined(mc500) || defined(mc700) || defined(mc6000)
#   ifdef HAS_SOCKETPAIR
#	undef HAS_SOCKETPAIR
#   endif
#   ifdef I_NDBM
#	undef I_NDBM
#   endif
#endif

#ifndef HAS_SOCKETPAIR
#   ifdef HAS_SOCKET
#	define socketpair Perl_my_socketpair
#   endif
#endif

#if INTSIZE == 2
#   define htoni htons
#   define ntohi ntohs
#else
#   define htoni htonl
#   define ntohi ntohl
#endif

/* Configure already sets Direntry_t */
#if defined(I_DIRENT)
#   include <dirent.h>
    /* NeXT needs dirent + sys/dir.h */
#   if  defined(I_SYS_DIR) && (defined(NeXT) || defined(__NeXT__))
#	include <sys/dir.h>
#   endif
#else
#   ifdef I_SYS_NDIR
#	include <sys/ndir.h>
#   else
#	ifdef I_SYS_DIR
#	    ifdef hp9000s500
#		include <ndir.h>	/* may be wrong in the future */
#	    else
#		include <sys/dir.h>
#	    endif
#	endif
#   endif
#endif

#ifdef PERL_MICRO
#   ifndef DIR
#      define DIR void
#   endif
#endif

#ifdef FPUTS_BOTCH
/* work around botch in SunOS 4.0.1 and 4.0.2 */
#   ifndef fputs
#	define fputs(sv,fp) fprintf(fp,"%s",sv)
#   endif
#endif

/*
 * The following gobbledygook brought to you on behalf of __STDC__.
 * (I could just use #ifndef __STDC__, but this is more bulletproof
 * in the face of half-implementations.)
 */

#if defined(I_SYSMODE) && !defined(PERL_MICRO)
#include <sys/mode.h>
#endif

#ifndef S_IFMT
#   ifdef _S_IFMT
#	define S_IFMT _S_IFMT
#   else
#	define S_IFMT 0170000
#   endif
#endif

#ifndef S_ISDIR
#   define S_ISDIR(m) ((m & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISCHR
#   define S_ISCHR(m) ((m & S_IFMT) == S_IFCHR)
#endif

#ifndef S_ISBLK
#   ifdef S_IFBLK
#	define S_ISBLK(m) ((m & S_IFMT) == S_IFBLK)
#   else
#	define S_ISBLK(m) (0)
#   endif
#endif

#ifndef S_ISREG
#   define S_ISREG(m) ((m & S_IFMT) == S_IFREG)
#endif

#ifndef S_ISFIFO
#   ifdef S_IFIFO
#	define S_ISFIFO(m) ((m & S_IFMT) == S_IFIFO)
#   else
#	define S_ISFIFO(m) (0)
#   endif
#endif

#ifndef S_ISLNK
#   ifdef _S_ISLNK
#	define S_ISLNK(m) _S_ISLNK(m)
#   else
#	ifdef _S_IFLNK
#	    define S_ISLNK(m) ((m & S_IFMT) == _S_IFLNK)
#	else
#	    ifdef S_IFLNK
#		define S_ISLNK(m) ((m & S_IFMT) == S_IFLNK)
#	    else
#		define S_ISLNK(m) (0)
#	    endif
#	endif
#   endif
#endif

#ifndef S_ISSOCK
#   ifdef _S_ISSOCK
#	define S_ISSOCK(m) _S_ISSOCK(m)
#   else
#	ifdef _S_IFSOCK
#	    define S_ISSOCK(m) ((m & S_IFMT) == _S_IFSOCK)
#	else
#	    ifdef S_IFSOCK
#		define S_ISSOCK(m) ((m & S_IFMT) == S_IFSOCK)
#	    else
#		define S_ISSOCK(m) (0)
#	    endif
#	endif
#   endif
#endif

#ifndef S_IRUSR
#   ifdef S_IREAD
#	define S_IRUSR S_IREAD
#	define S_IWUSR S_IWRITE
#	define S_IXUSR S_IEXEC
#   else
#	define S_IRUSR 0400
#	define S_IWUSR 0200
#	define S_IXUSR 0100
#   endif
#endif

#ifndef S_IRGRP
#   ifdef S_IRUSR
#       define S_IRGRP (S_IRUSR>>3)
#       define S_IWGRP (S_IWUSR>>3)
#       define S_IXGRP (S_IXUSR>>3)
#   else
#       define S_IRGRP 0040
#       define S_IWGRP 0020
#       define S_IXGRP 0010
#   endif
#endif

#ifndef S_IROTH
#   ifdef S_IRUSR
#       define S_IROTH (S_IRUSR>>6)
#       define S_IWOTH (S_IWUSR>>6)
#       define S_IXOTH (S_IXUSR>>6)
#   else
#       define S_IROTH 0040
#       define S_IWOTH 0020
#       define S_IXOTH 0010
#   endif
#endif

#ifndef S_ISUID
#   define S_ISUID 04000
#endif

#ifndef S_ISGID
#   define S_ISGID 02000
#endif

#ifndef S_IRWXU
#   define S_IRWXU (S_IRUSR|S_IWUSR|S_IXUSR)
#endif

#ifndef S_IRWXG
#   define S_IRWXG (S_IRGRP|S_IWGRP|S_IXGRP)
#endif

#ifndef S_IRWXO
#   define S_IRWXO (S_IROTH|S_IWOTH|S_IXOTH)
#endif

/* BeOS 5.0 and Haiku R1 seem to define S_IREAD and S_IWRITE in <posix/fcntl.h>
 * which would get included through <sys/file.h >, but that is 3000
 * lines in the future.  --jhi */

#if !defined(S_IREAD) && !(defined(__BEOS__) || defined(__HAIKU__))
#   define S_IREAD S_IRUSR
#endif

#if !defined(S_IWRITE) && !(defined(__BEOS__) || defined(__HAIKU__))
#   define S_IWRITE S_IWUSR
#endif

#ifndef S_IEXEC
#   define S_IEXEC S_IXUSR
#endif

#ifdef ff_next
#   undef ff_next
#endif

#if defined(cray) || defined(gould) || defined(i860) || defined(pyr)
#   define SLOPPYDIVIDE
#endif

#ifdef UV
#undef UV
#endif

#ifdef	SPRINTF_E_BUG
#  define sprintf UTS_sprintf_wrap
#endif

/* For the times when you want the return value of sprintf, and you want it
   to be the length. Can't have a thread variable passed in, because C89 has
   no varargs macros.
*/
#ifdef SPRINTF_RETURNS_STRLEN
#  define my_sprintf sprintf
#else
#  define my_sprintf Perl_my_sprintf
#endif

/*
 * If we have v?snprintf() and the C99 variadic macros, we can just
 * use just the v?snprintf().  It is nice to try to trap the buffer
 * overflow, however, so if we are DEBUGGING, and we cannot use the
 * gcc statement expressions, then use the function wrappers which try
 * to trap the overflow.  If we can use the gcc statement expressions,
 * we can try that even with the version that uses the C99 variadic
 * macros.
 */

/* Note that we do not check against snprintf()/vsnprintf() returning
 * negative values because that is non-standard behaviour and we use
 * snprintf/vsnprintf only iff HAS_VSNPRINTF has been defined, and
 * that should be true only if the snprintf()/vsnprintf() are true
 * to the standard. */

#if defined(HAS_SNPRINTF) && defined(HAS_C99_VARIADIC_MACROS) && !(defined(DEBUGGING) && !defined(PERL_USE_GCC_BRACE_GROUPS)) && !defined(PERL_GCC_PEDANTIC)
#  ifdef PERL_USE_GCC_BRACE_GROUPS
#      define my_snprintf(buffer, len, ...) ({ int __len__ = snprintf(buffer, len, __VA_ARGS__); if ((len) > 0 && (Size_t)__len__ >= (len)) Perl_croak_nocontext("panic: snprintf buffer overflow"); __len__; })
#      define PERL_MY_SNPRINTF_GUARDED
#  else
#    define my_snprintf(buffer, len, ...) snprintf(buffer, len, __VA_ARGS__)
#  endif
#else
#  define my_snprintf  Perl_my_snprintf
#  define PERL_MY_SNPRINTF_GUARDED
#endif

#if defined(HAS_VSNPRINTF) && defined(HAS_C99_VARIADIC_MACROS) && !(defined(DEBUGGING) && !defined(PERL_USE_GCC_BRACE_GROUPS)) && !defined(PERL_GCC_PEDANTIC)
#  ifdef PERL_USE_GCC_BRACE_GROUPS
#      define my_vsnprintf(buffer, len, ...) ({ int __len__ = vsnprintf(buffer, len, __VA_ARGS__); if ((len) > 0 && (Size_t)__len__ >= (len)) Perl_croak_nocontext("panic: vsnprintf buffer overflow"); __len__; })
#      define PERL_MY_VSNPRINTF_GUARDED
#  else
#    define my_vsnprintf(buffer, len, ...) vsnprintf(buffer, len, __VA_ARGS__)
#  endif
#else
#  define my_vsnprintf Perl_my_vsnprintf
#  define PERL_MY_VSNPRINTF_GUARDED
#endif

#ifdef HAS_STRLCAT
#  define my_strlcat    strlcat
#else
#  define my_strlcat    Perl_my_strlcat
#endif

#ifdef HAS_STRLCPY
#  define my_strlcpy	strlcpy
#else
#  define my_strlcpy	Perl_my_strlcpy
#endif

/* Configure gets this right but the UTS compiler gets it wrong.
   -- Hal Morris <hom00@utsglobal.com> */
#ifdef UTS
#  undef  UVTYPE
#  define UVTYPE unsigned
#endif

/*
    The IV type is supposed to be long enough to hold any integral
    value or a pointer.
    --Andy Dougherty	August 1996
*/

typedef IVTYPE IV;
typedef UVTYPE UV;

#if defined(USE_64_BIT_INT) && defined(HAS_QUAD)
#  if QUADKIND == QUAD_IS_INT64_T && defined(INT64_MAX)
#    define IV_MAX INT64_MAX
#    define IV_MIN INT64_MIN
#    define UV_MAX UINT64_MAX
#    ifndef UINT64_MIN
#      define UINT64_MIN 0
#    endif
#    define UV_MIN UINT64_MIN
#  else
#    define IV_MAX PERL_QUAD_MAX
#    define IV_MIN PERL_QUAD_MIN
#    define UV_MAX PERL_UQUAD_MAX
#    define UV_MIN PERL_UQUAD_MIN
#  endif
#  define IV_IS_QUAD
#  define UV_IS_QUAD
#else
#  if defined(INT32_MAX) && IVSIZE == 4
#    define IV_MAX INT32_MAX
#    define IV_MIN INT32_MIN
#    ifndef UINT32_MAX_BROKEN /* e.g. HP-UX with gcc messes this up */
#        define UV_MAX UINT32_MAX
#    else
#        define UV_MAX 4294967295U
#    endif
#    ifndef UINT32_MIN
#      define UINT32_MIN 0
#    endif
#    define UV_MIN UINT32_MIN
#  else
#    define IV_MAX PERL_LONG_MAX
#    define IV_MIN PERL_LONG_MIN
#    define UV_MAX PERL_ULONG_MAX
#    define UV_MIN PERL_ULONG_MIN
#  endif
#  if IVSIZE == 8
#    define IV_IS_QUAD
#    define UV_IS_QUAD
#    ifndef HAS_QUAD
#      define HAS_QUAD
#    endif
#  else
#    undef IV_IS_QUAD
#    undef UV_IS_QUAD
#    undef HAS_QUAD
#  endif
#endif

#ifndef HAS_QUAD
# undef PERL_NEED_MY_HTOLE64
# undef PERL_NEED_MY_LETOH64
# undef PERL_NEED_MY_HTOBE64
# undef PERL_NEED_MY_BETOH64
#endif

#if defined(uts) || defined(UTS)
#	undef UV_MAX
#	define UV_MAX (4294967295u)
#endif

#define IV_DIG (BIT_DIGITS(IVSIZE * 8))
#define UV_DIG (BIT_DIGITS(UVSIZE * 8))

#ifndef NO_PERL_PRESERVE_IVUV
#define PERL_PRESERVE_IVUV	/* We like our integers to stay integers. */
#endif

/*
 *  The macros INT2PTR and NUM2PTR are (despite their names)
 *  bi-directional: they will convert int/float to or from pointers.
 *  However the conversion to int/float are named explicitly:
 *  PTR2IV, PTR2UV, PTR2NV.
 *
 *  For int conversions we do not need two casts if pointers are
 *  the same size as IV and UV.   Otherwise we need an explicit
 *  cast (PTRV) to avoid compiler warnings.
 */
#if (IVSIZE == PTRSIZE) && (UVSIZE == PTRSIZE)
#  define PTRV			UV
#  define INT2PTR(any,d)	(any)(d)
#else
#  if PTRSIZE == LONGSIZE
#    define PTRV		unsigned long
#    define PTR2ul(p)		(unsigned long)(p)
#  else
#    define PTRV		unsigned
#  endif
#endif

#ifndef INT2PTR
#  define INT2PTR(any,d)	(any)(PTRV)(d)
#endif

#ifndef PTR2ul
#  define PTR2ul(p)	INT2PTR(unsigned long,p)
#endif

#define NUM2PTR(any,d)	(any)(PTRV)(d)
#define PTR2IV(p)	INT2PTR(IV,p)
#define PTR2UV(p)	INT2PTR(UV,p)
#define PTR2NV(p)	NUM2PTR(NV,p)
#define PTR2nat(p)	(PTRV)(p)	/* pointer to integer of PTRSIZE */

/* According to strict ANSI C89 one cannot freely cast between
 * data pointers and function (code) pointers.  There are at least
 * two ways around this.  One (used below) is to do two casts,
 * first the other pointer to an (unsigned) integer, and then
 * the integer to the other pointer.  The other way would be
 * to use unions to "overlay" the pointers.  For an example of
 * the latter technique, see union dirpu in struct xpvio in sv.h.
 * The only feasible use is probably temporarily storing
 * function pointers in a data pointer (such as a void pointer). */

#define DPTR2FPTR(t,p) ((t)PTR2nat(p))	/* data pointer to function pointer */
#define FPTR2DPTR(t,p) ((t)PTR2nat(p))	/* function pointer to data pointer */

#ifdef USE_LONG_DOUBLE
#  if defined(HAS_LONG_DOUBLE) && LONG_DOUBLESIZE == DOUBLESIZE
#      define LONG_DOUBLE_EQUALS_DOUBLE
#  endif
#  if !(defined(HAS_LONG_DOUBLE) && (LONG_DOUBLESIZE > DOUBLESIZE))
#     undef USE_LONG_DOUBLE /* Ouch! */
#  endif
#endif

#ifdef OVR_DBL_DIG
/* Use an overridden DBL_DIG */
# ifdef DBL_DIG
#  undef DBL_DIG
# endif
# define DBL_DIG OVR_DBL_DIG
#else
/* The following is all to get DBL_DIG, in order to pick a nice
   default value for printing floating point numbers in Gconvert
   (see config.h). (It also has other uses, such as figuring out if
   a given precision of printing can be done with a double instead of
   a long double - Allen).
*/
#ifdef I_LIMITS
#include <limits.h>
#endif
#ifdef I_FLOAT
#include <float.h>
#endif
#ifndef HAS_DBL_DIG
#define DBL_DIG	15   /* A guess that works lots of places */
#endif
#endif

#ifdef OVR_LDBL_DIG
/* Use an overridden LDBL_DIG */
# ifdef LDBL_DIG
#  undef LDBL_DIG
# endif
# define LDBL_DIG OVR_LDBL_DIG
#else
/* The following is all to get LDBL_DIG, in order to pick a nice
   default value for printing floating point numbers in Gconvert.
   (see config.h)
*/
# ifdef I_LIMITS
#   include <limits.h>
# endif
# ifdef I_FLOAT
#  include <float.h>
# endif
# ifndef HAS_LDBL_DIG
#  if LONG_DOUBLESIZE == 10
#   define LDBL_DIG 18 /* assume IEEE */
#  else
#   if LONG_DOUBLESIZE == 12
#    define LDBL_DIG 18 /* gcc? */
#   else
#    if LONG_DOUBLESIZE == 16
#     define LDBL_DIG 33 /* assume IEEE */
#    else
#     if LONG_DOUBLESIZE == DOUBLESIZE
#      define LDBL_DIG DBL_DIG /* bummer */
#     endif
#    endif
#   endif
#  endif
# endif
#endif

/*
 * This is for making sure we have a good DBL_MAX value, if possible,
 * either for usage as NV_MAX or for usage in figuring out if we can
 * fit a given long double into a double, if bug-fixing makes it
 * necessary to do so. - Allen <allens@cpan.org>
 */

#ifdef I_LIMITS
#  include <limits.h>
#endif

#ifdef I_VALUES
#  if !(defined(DBL_MIN) && defined(DBL_MAX) && defined(I_LIMITS))
#    include <values.h>
#    if defined(MAXDOUBLE) && !defined(DBL_MAX)
#      define DBL_MAX MAXDOUBLE
#    endif
#    if defined(MINDOUBLE) && !defined(DBL_MIN)
#      define DBL_MIN MINDOUBLE
#    endif
#  endif
#endif /* defined(I_VALUES) */

typedef NVTYPE NV;

#ifdef I_IEEEFP
#   include <ieeefp.h>
#endif

#ifdef USE_LONG_DOUBLE
#   ifdef I_SUNMATH
#       include <sunmath.h>
#   endif
#   define NV_DIG LDBL_DIG
#   ifdef LDBL_MANT_DIG
#       define NV_MANT_DIG LDBL_MANT_DIG
#   endif
#   ifdef LDBL_MIN
#       define NV_MIN LDBL_MIN
#   endif
#   ifdef LDBL_MAX
#       define NV_MAX LDBL_MAX
#   endif
#   ifdef LDBL_MIN_10_EXP
#       define NV_MIN_10_EXP LDBL_MIN_10_EXP
#   endif
#   ifdef LDBL_MAX_10_EXP
#       define NV_MAX_10_EXP LDBL_MAX_10_EXP
#   endif
#   ifdef LDBL_EPSILON
#       define NV_EPSILON LDBL_EPSILON
#   endif
#   ifdef LDBL_MAX
#       define NV_MAX LDBL_MAX
/* Having LDBL_MAX doesn't necessarily mean that we have LDBL_MIN... -Allen */
#   else
#       ifdef HUGE_VALL
#           define NV_MAX HUGE_VALL
#       else
#           ifdef HUGE_VAL
#               define NV_MAX ((NV)HUGE_VAL)
#           endif
#       endif
#   endif
#   ifdef HAS_SQRTL
#       define Perl_cos cosl
#       define Perl_sin sinl
#       define Perl_sqrt sqrtl
#       define Perl_exp expl
#       define Perl_log logl
#       define Perl_atan2 atan2l
#       define Perl_pow powl
#       define Perl_floor floorl
#       define Perl_ceil ceill
#       define Perl_fmod fmodl
#   endif
/* e.g. libsunmath doesn't have modfl and frexpl as of mid-March 2000 */
#   ifdef HAS_MODFL
#       define Perl_modf(x,y) modfl(x,y)
/* eg glibc 2.2 series seems to provide modfl on ppc and arm, but has no
   prototype in <math.h> */
#       ifndef HAS_MODFL_PROTO
EXTERN_C long double modfl(long double, long double *);
#	endif
#   else
#       if defined(HAS_AINTL) && defined(HAS_COPYSIGNL)
        extern long double Perl_my_modfl(long double x, long double *ip);
#           define Perl_modf(x,y) Perl_my_modfl(x,y)
#       endif
#   endif
#   ifdef HAS_FREXPL
#       define Perl_frexp(x,y) frexpl(x,y)
#   else
#       if defined(HAS_ILOGBL) && defined(HAS_SCALBNL)
        extern long double Perl_my_frexpl(long double x, int *e);
#           define Perl_frexp(x,y) Perl_my_frexpl(x,y)
#       endif
#   endif
#   ifndef Perl_isnan
#       ifdef HAS_ISNANL
#           define Perl_isnan(x) isnanl(x)
#       endif
#   endif
#   ifndef Perl_isinf
#       ifdef HAS_FINITEL
#           define Perl_isinf(x) !(finitel(x)||Perl_isnan(x))
#       endif
#   endif
#else
#   define NV_DIG DBL_DIG
#   ifdef DBL_MANT_DIG
#       define NV_MANT_DIG DBL_MANT_DIG
#   endif
#   ifdef DBL_MIN
#       define NV_MIN DBL_MIN
#   endif
#   ifdef DBL_MAX
#       define NV_MAX DBL_MAX
#   endif
#   ifdef DBL_MIN_10_EXP
#       define NV_MIN_10_EXP DBL_MIN_10_EXP
#   endif
#   ifdef DBL_MAX_10_EXP
#       define NV_MAX_10_EXP DBL_MAX_10_EXP
#   endif
#   ifdef DBL_EPSILON
#       define NV_EPSILON DBL_EPSILON
#   endif
#   ifdef DBL_MAX               /* XXX Does DBL_MAX imply having DBL_MIN? */
#       define NV_MAX DBL_MAX
#       define NV_MIN DBL_MIN
#   else
#       ifdef HUGE_VAL
#           define NV_MAX HUGE_VAL
#       endif
#   endif
#   define Perl_cos cos
#   define Perl_sin sin
#   define Perl_sqrt sqrt
#   define Perl_exp exp
#   define Perl_log log
#   define Perl_atan2 atan2
#   define Perl_pow pow
#   define Perl_floor floor
#   define Perl_ceil ceil
#   define Perl_fmod fmod
#   define Perl_modf(x,y) modf(x,y)
#   define Perl_frexp(x,y) frexp(x,y)
#endif

/* rumor has it that Win32 has _fpclass() */

/* SGI has fpclassl... but not with the same result values,
 * and it's via a typedef (not via #define), so will need to redo Configure
 * to use. Not worth the trouble, IMO, at least until the below is used
 * more places. Also has fp_class_l, BTW, via fp_class.h. Feel free to check
 * with me for the SGI manpages, SGI testing, etcetera, if you want to
 * try getting this to work with IRIX. - Allen <allens@cpan.org> */

#if !defined(Perl_fp_class) && (defined(HAS_FPCLASS)||defined(HAS_FPCLASSL))
#    ifdef I_IEEFP
#        include <ieeefp.h>
#    endif
#    ifdef I_FP
#        include <fp.h>
#    endif
#    if defined(USE_LONG_DOUBLE) && defined(HAS_FPCLASSL)
#        define Perl_fp_class()		fpclassl(x)
#    else
#        define Perl_fp_class()		fpclass(x)
#    endif
#    define Perl_fp_class_snan(x)	(Perl_fp_class(x)==FP_CLASS_SNAN)
#    define Perl_fp_class_qnan(x)	(Perl_fp_class(x)==FP_CLASS_QNAN)
#    define Perl_fp_class_nan(x)	(Perl_fp_class(x)==FP_CLASS_SNAN||Perl_fp_class(x)==FP_CLASS_QNAN)
#    define Perl_fp_class_ninf(x)	(Perl_fp_class(x)==FP_CLASS_NINF)
#    define Perl_fp_class_pinf(x)	(Perl_fp_class(x)==FP_CLASS_PINF)
#    define Perl_fp_class_inf(x)	(Perl_fp_class(x)==FP_CLASS_NINF||Perl_fp_class(x)==FP_CLASS_PINF)
#    define Perl_fp_class_nnorm(x)	(Perl_fp_class(x)==FP_CLASS_NNORM)
#    define Perl_fp_class_pnorm(x)	(Perl_fp_class(x)==FP_CLASS_PNORM)
#    define Perl_fp_class_norm(x)	(Perl_fp_class(x)==FP_CLASS_NNORM||Perl_fp_class(x)==FP_CLASS_PNORM)
#    define Perl_fp_class_ndenorm(x)	(Perl_fp_class(x)==FP_CLASS_NDENORM)
#    define Perl_fp_class_pdenorm(x)	(Perl_fp_class(x)==FP_CLASS_PDENORM)
#    define Perl_fp_class_denorm(x)	(Perl_fp_class(x)==FP_CLASS_NDENORM||Perl_fp_class(x)==FP_CLASS_PDENORM)
#    define Perl_fp_class_nzero(x)	(Perl_fp_class(x)==FP_CLASS_NZERO)
#    define Perl_fp_class_pzero(x)	(Perl_fp_class(x)==FP_CLASS_PZERO)
#    define Perl_fp_class_zero(x)	(Perl_fp_class(x)==FP_CLASS_NZERO||Perl_fp_class(x)==FP_CLASS_PZERO)
#endif

#if !defined(Perl_fp_class) && defined(HAS_FP_CLASS) && !defined(PERL_MICRO)
#    include <math.h>
#    if !defined(FP_SNAN) && defined(I_FP_CLASS)
#        include <fp_class.h>
#    endif
#    define Perl_fp_class(x)		fp_class(x)
#    define Perl_fp_class_snan(x)	(fp_class(x)==FP_SNAN)
#    define Perl_fp_class_qnan(x)	(fp_class(x)==FP_QNAN)
#    define Perl_fp_class_nan(x)	(fp_class(x)==FP_SNAN||fp_class(x)==FP_QNAN)
#    define Perl_fp_class_ninf(x)	(fp_class(x)==FP_NEG_INF)
#    define Perl_fp_class_pinf(x)	(fp_class(x)==FP_POS_INF)
#    define Perl_fp_class_inf(x)	(fp_class(x)==FP_NEG_INF||fp_class(x)==FP_POS_INF)
#    define Perl_fp_class_nnorm(x)	(fp_class(x)==FP_NEG_NORM)
#    define Perl_fp_class_pnorm(x)	(fp_class(x)==FP_POS_NORM)
#    define Perl_fp_class_norm(x)	(fp_class(x)==FP_NEG_NORM||fp_class(x)==FP_POS_NORM)
#    define Perl_fp_class_ndenorm(x)	(fp_class(x)==FP_NEG_DENORM)
#    define Perl_fp_class_pdenorm(x)	(fp_class(x)==FP_POS_DENORM)
#    define Perl_fp_class_denorm(x)	(fp_class(x)==FP_NEG_DENORM||fp_class(x)==FP_POS_DENORM)
#    define Perl_fp_class_nzero(x)	(fp_class(x)==FP_NEG_ZERO)
#    define Perl_fp_class_pzero(x)	(fp_class(x)==FP_POS_ZERO)
#    define Perl_fp_class_zero(x)	(fp_class(x)==FP_NEG_ZERO||fp_class(x)==FP_POS_ZERO)
#endif

#if !defined(Perl_fp_class) && defined(HAS_FPCLASSIFY)
#    include <math.h>
#    define Perl_fp_class(x)		fpclassify(x)
#    define Perl_fp_class_nan(x)	(fp_classify(x)==FP_SNAN||fp_classify(x)==FP_QNAN)
#    define Perl_fp_class_inf(x)	(fp_classify(x)==FP_INFINITE)
#    define Perl_fp_class_norm(x)	(fp_classify(x)==FP_NORMAL)
#    define Perl_fp_class_denorm(x)	(fp_classify(x)==FP_SUBNORMAL)
#    define Perl_fp_class_zero(x)	(fp_classify(x)==FP_ZERO)
#endif

#if !defined(Perl_fp_class) && defined(HAS_CLASS)
#    include <math.h>
#    ifndef _cplusplus
#        define Perl_fp_class(x)	class(x)
#    else
#        define Perl_fp_class(x)	_class(x)
#    endif
#    define Perl_fp_class_snan(x)	(Perl_fp_class(x)==FP_NANS)
#    define Perl_fp_class_qnan(x)	(Perl_fp_class(x)==FP_NANQ)
#    define Perl_fp_class_nan(x)	(Perl_fp_class(x)==FP_SNAN||Perl_fp_class(x)==FP_QNAN)
#    define Perl_fp_class_ninf(x)	(Perl_fp_class(x)==FP_MINUS_INF)
#    define Perl_fp_class_pinf(x)	(Perl_fp_class(x)==FP_PLUS_INF)
#    define Perl_fp_class_inf(x)	(Perl_fp_class(x)==FP_MINUS_INF||Perl_fp_class(x)==FP_PLUS_INF)
#    define Perl_fp_class_nnorm(x)	(Perl_fp_class(x)==FP_MINUS_NORM)
#    define Perl_fp_class_pnorm(x)	(Perl_fp_class(x)==FP_PLUS_NORM)
#    define Perl_fp_class_norm(x)	(Perl_fp_class(x)==FP_MINUS_NORM||Perl_fp_class(x)==FP_PLUS_NORM)
#    define Perl_fp_class_ndenorm(x)	(Perl_fp_class(x)==FP_MINUS_DENORM)
#    define Perl_fp_class_pdenorm(x)	(Perl_fp_class(x)==FP_PLUS_DENORM)
#    define Perl_fp_class_denorm(x)	(Perl_fp_class(x)==FP_MINUS_DENORM||Perl_fp_class(x)==FP_PLUS_DENORM)
#    define Perl_fp_class_nzero(x)	(Perl_fp_class(x)==FP_MINUS_ZERO)
#    define Perl_fp_class_pzero(x)	(Perl_fp_class(x)==FP_PLUS_ZERO)
#    define Perl_fp_class_zero(x)	(Perl_fp_class(x)==FP_MINUS_ZERO||Perl_fp_class(x)==FP_PLUS_ZERO)
#endif

/* rumor has it that Win32 has _isnan() */

#ifndef Perl_isnan
#   ifdef HAS_ISNAN
#       define Perl_isnan(x) isnan((NV)x)
#   else
#       ifdef Perl_fp_class_nan
#           define Perl_isnan(x) Perl_fp_class_nan(x)
#       else
#           ifdef HAS_UNORDERED
#               define Perl_isnan(x) unordered((x), 0.0)
#           else
#               define Perl_isnan(x) ((x)!=(x))
#           endif
#       endif
#   endif
#endif

#ifdef UNDER_CE
int isnan(double d);
#endif

#ifndef Perl_isinf
#   ifdef HAS_ISINF
#       define Perl_isinf(x) isinf((NV)x)
#   else
#       ifdef Perl_fp_class_inf
#           define Perl_isinf(x) Perl_fp_class_inf(x)
#       else
#           define Perl_isinf(x) ((x)==NV_INF)
#       endif
#   endif
#endif

#ifndef Perl_isfinite
#   ifdef HAS_FINITE
#       define Perl_isfinite(x) finite((NV)x)
#   else
#       ifdef HAS_ISFINITE
#           define Perl_isfinite(x) isfinite(x)
#       else
#           ifdef Perl_fp_class_finite
#               define Perl_isfinite(x) Perl_fp_class_finite(x)
#           else
#               define Perl_isfinite(x) !(Perl_is_inf(x)||Perl_is_nan(x))
#           endif
#       endif
#   endif
#endif

/* The default is to use Perl's own atof() implementation (in numeric.c).
 * Usually that is the one to use but for some platforms (e.g. UNICOS)
 * it is however best to use the native implementation of atof.
 * You can experiment with using your native one by -DUSE_PERL_ATOF=0.
 * Some good tests to try out with either setting are t/base/num.t,
 * t/op/numconvert.t, and t/op/pack.t. Note that if using long doubles
 * you may need to be using a different function than atof! */

#ifndef USE_PERL_ATOF
#   ifndef _UNICOS
#       define USE_PERL_ATOF
#   endif
#else
#   if USE_PERL_ATOF == 0
#       undef USE_PERL_ATOF
#   endif
#endif

#ifdef USE_PERL_ATOF
#   define Perl_atof(s) Perl_my_atof(s)
#   define Perl_atof2(s, n) Perl_my_atof2(aTHX_ (s), &(n))
#else
#   define Perl_atof(s) (NV)atof(s)
#   define Perl_atof2(s, n) ((n) = atof(s))
#endif

/* Previously these definitions used hardcoded figures.
 * It is hoped these formula are more portable, although
 * no data one way or another is presently known to me.
 * The "PERL_" names are used because these calculated constants
 * do not meet the ANSI requirements for LONG_MAX, etc., which
 * need to be constants acceptable to #if - kja
 *    define PERL_LONG_MAX        2147483647L
 *    define PERL_LONG_MIN        (-LONG_MAX - 1)
 *    define PERL ULONG_MAX       4294967295L
 */

#ifdef I_LIMITS  /* Needed for cast_xxx() functions below. */
#  include <limits.h>
#endif
/* Included values.h above if necessary; still including limits.h down here,
 * despite doing above, because math.h might have overridden... XXX - Allen */

/*
 * Try to figure out max and min values for the integral types.  THE CORRECT
 * SOLUTION TO THIS MESS: ADAPT enquire.c FROM GCC INTO CONFIGURE.  The
 * following hacks are used if neither limits.h or values.h provide them:
 * U<TYPE>_MAX: for types >= int: ~(unsigned TYPE)0
 *              for types <  int:  (unsigned TYPE)~(unsigned)0
 *	The argument to ~ must be unsigned so that later signed->unsigned
 *	conversion can't modify the value's bit pattern (e.g. -0 -> +0),
 *	and it must not be smaller than int because ~ does integral promotion.
 * <type>_MAX: (<type>) (U<type>_MAX >> 1)
 * <type>_MIN: -<type>_MAX - <is_twos_complement_architecture: (3 & -1) == 3>.
 *	The latter is a hack which happens to work on some machines but
 *	does *not* catch any random system, or things like integer types
 *	with NaN if that is possible.
 *
 * All of the types are explicitly cast to prevent accidental loss of
 * numeric range, and in the hope that they will be less likely to confuse
 * over-eager optimizers.
 *
 */

#define PERL_UCHAR_MIN ((unsigned char)0)

#ifdef UCHAR_MAX
#  define PERL_UCHAR_MAX ((unsigned char)UCHAR_MAX)
#else
#  ifdef MAXUCHAR
#    define PERL_UCHAR_MAX ((unsigned char)MAXUCHAR)
#  else
#    define PERL_UCHAR_MAX       ((unsigned char)~(unsigned)0)
#  endif
#endif

/*
 * CHAR_MIN and CHAR_MAX are not included here, as the (char) type may be
 * ambiguous. It may be equivalent to (signed char) or (unsigned char)
 * depending on local options. Until Configure detects this (or at least
 * detects whether the "signed" keyword is available) the CHAR ranges
 * will not be included. UCHAR functions normally.
 *                                                           - kja
 */

#define PERL_USHORT_MIN ((unsigned short)0)

#ifdef USHORT_MAX
#  define PERL_USHORT_MAX ((unsigned short)USHORT_MAX)
#else
#  ifdef MAXUSHORT
#    define PERL_USHORT_MAX ((unsigned short)MAXUSHORT)
#  else
#    ifdef USHRT_MAX
#      define PERL_USHORT_MAX ((unsigned short)USHRT_MAX)
#    else
#      define PERL_USHORT_MAX       ((unsigned short)~(unsigned)0)
#    endif
#  endif
#endif

#ifdef SHORT_MAX
#  define PERL_SHORT_MAX ((short)SHORT_MAX)
#else
#  ifdef MAXSHORT    /* Often used in <values.h> */
#    define PERL_SHORT_MAX ((short)MAXSHORT)
#  else
#    ifdef SHRT_MAX
#      define PERL_SHORT_MAX ((short)SHRT_MAX)
#    else
#      define PERL_SHORT_MAX      ((short) (PERL_USHORT_MAX >> 1))
#    endif
#  endif
#endif

#ifdef SHORT_MIN
#  define PERL_SHORT_MIN ((short)SHORT_MIN)
#else
#  ifdef MINSHORT
#    define PERL_SHORT_MIN ((short)MINSHORT)
#  else
#    ifdef SHRT_MIN
#      define PERL_SHORT_MIN ((short)SHRT_MIN)
#    else
#      define PERL_SHORT_MIN        (-PERL_SHORT_MAX - ((3 & -1) == 3))
#    endif
#  endif
#endif

#ifdef UINT_MAX
#  define PERL_UINT_MAX ((unsigned int)UINT_MAX)
#else
#  ifdef MAXUINT
#    define PERL_UINT_MAX ((unsigned int)MAXUINT)
#  else
#    define PERL_UINT_MAX       (~(unsigned int)0)
#  endif
#endif

#define PERL_UINT_MIN ((unsigned int)0)

#ifdef INT_MAX
#  define PERL_INT_MAX ((int)INT_MAX)
#else
#  ifdef MAXINT    /* Often used in <values.h> */
#    define PERL_INT_MAX ((int)MAXINT)
#  else
#    define PERL_INT_MAX        ((int)(PERL_UINT_MAX >> 1))
#  endif
#endif

#ifdef INT_MIN
#  define PERL_INT_MIN ((int)INT_MIN)
#else
#  ifdef MININT
#    define PERL_INT_MIN ((int)MININT)
#  else
#    define PERL_INT_MIN        (-PERL_INT_MAX - ((3 & -1) == 3))
#  endif
#endif

#ifdef ULONG_MAX
#  define PERL_ULONG_MAX ((unsigned long)ULONG_MAX)
#else
#  ifdef MAXULONG
#    define PERL_ULONG_MAX ((unsigned long)MAXULONG)
#  else
#    define PERL_ULONG_MAX       (~(unsigned long)0)
#  endif
#endif

#define PERL_ULONG_MIN ((unsigned long)0L)

#ifdef LONG_MAX
#  define PERL_LONG_MAX ((long)LONG_MAX)
#else
#  ifdef MAXLONG    /* Often used in <values.h> */
#    define PERL_LONG_MAX ((long)MAXLONG)
#  else
#    define PERL_LONG_MAX        ((long) (PERL_ULONG_MAX >> 1))
#  endif
#endif

#ifdef LONG_MIN
#  define PERL_LONG_MIN ((long)LONG_MIN)
#else
#  ifdef MINLONG
#    define PERL_LONG_MIN ((long)MINLONG)
#  else
#    define PERL_LONG_MIN        (-PERL_LONG_MAX - ((3 & -1) == 3))
#  endif
#endif

#ifdef UV_IS_QUAD

#    define PERL_UQUAD_MAX	(~(UV)0)
#    define PERL_UQUAD_MIN	((UV)0)
#    define PERL_QUAD_MAX 	((IV) (PERL_UQUAD_MAX >> 1))
#    define PERL_QUAD_MIN 	(-PERL_QUAD_MAX - ((3 & -1) == 3))

#endif

struct RExC_state_t;
struct _reg_trie_data;

typedef MEM_SIZE STRLEN;

#ifdef PERL_MAD
typedef struct token TOKEN;
typedef struct madprop MADPROP;
typedef struct nexttoken NEXTTOKE;
#endif
typedef struct op OP;
typedef struct cop COP;
typedef struct unop UNOP;
typedef struct binop BINOP;
typedef struct listop LISTOP;
typedef struct logop LOGOP;
typedef struct pmop PMOP;
typedef struct svop SVOP;
typedef struct padop PADOP;
typedef struct pvop PVOP;
typedef struct loop LOOP;

typedef struct block_hooks BHK;
typedef struct custom_op XOP;

typedef struct interpreter PerlInterpreter;

/* Amdahl's <ksync.h> has struct sv */
/* SGI's <sys/sema.h> has struct sv */
#if defined(UTS) || defined(__sgi)
#   define STRUCT_SV perl_sv
#else
#   define STRUCT_SV sv
#endif
typedef struct STRUCT_SV SV;
typedef struct av AV;
typedef struct hv HV;
typedef struct cv CV;
typedef struct p5rx REGEXP;
typedef struct gp GP;
typedef struct gv GV;
typedef struct io IO;
typedef struct context PERL_CONTEXT;
typedef struct block BLOCK;

typedef struct magic MAGIC;
typedef struct xpv XPV;
typedef struct xpviv XPVIV;
typedef struct xpvuv XPVUV;
typedef struct xpvnv XPVNV;
typedef struct xpvmg XPVMG;
typedef struct xpvlv XPVLV;
typedef struct xpvav XPVAV;
typedef struct xpvhv XPVHV;
typedef struct xpvgv XPVGV;
typedef struct xpvcv XPVCV;
typedef struct xpvbm XPVBM;
typedef struct xpvfm XPVFM;
typedef struct xpvio XPVIO;
typedef struct mgvtbl MGVTBL;
typedef union any ANY;
typedef struct ptr_tbl_ent PTR_TBL_ENT_t;
typedef struct ptr_tbl PTR_TBL_t;
typedef struct clone_params CLONE_PARAMS;

#include "handy.h"

#if defined(USE_LARGE_FILES) && !defined(NO_64_BIT_RAWIO)
#   if LSEEKSIZE == 8 && !defined(USE_64_BIT_RAWIO)
#       define USE_64_BIT_RAWIO	/* implicit */
#   endif
#endif

/* Notice the use of HAS_FSEEKO: now we are obligated to always use
 * fseeko/ftello if possible.  Don't go #defining ftell to ftello yourself,
 * however, because operating systems like to do that themself. */
#ifndef FSEEKSIZE
#   ifdef HAS_FSEEKO
#       define FSEEKSIZE LSEEKSIZE
#   else
#       define FSEEKSIZE LONGSIZE
#   endif
#endif

#if defined(USE_LARGE_FILES) && !defined(NO_64_BIT_STDIO)
#   if FSEEKSIZE == 8 && !defined(USE_64_BIT_STDIO)
#       define USE_64_BIT_STDIO /* implicit */
#   endif
#endif

#ifdef USE_64_BIT_RAWIO
#   ifdef HAS_OFF64_T
#       undef Off_t
#       define Off_t off64_t
#       undef LSEEKSIZE
#       define LSEEKSIZE 8
#   endif
/* Most 64-bit environments have defines like _LARGEFILE_SOURCE that
 * will trigger defines like the ones below.  Some 64-bit environments,
 * however, do not.  Therefore we have to explicitly mix and match. */
#   if defined(USE_OPEN64)
#       define open open64
#   endif
#   if defined(USE_LSEEK64)
#       define lseek lseek64
#   else
#       if defined(USE_LLSEEK)
#           define lseek llseek
#       endif
#   endif
#   if defined(USE_STAT64)
#       define stat stat64
#   endif
#   if defined(USE_FSTAT64)
#       define fstat fstat64
#   endif
#   if defined(USE_LSTAT64)
#       define lstat lstat64
#   endif
#   if defined(USE_FLOCK64)
#       define flock flock64
#   endif
#   if defined(USE_LOCKF64)
#       define lockf lockf64
#   endif
#   if defined(USE_FCNTL64)
#       define fcntl fcntl64
#   endif
#   if defined(USE_TRUNCATE64)
#       define truncate truncate64
#   endif
#   if defined(USE_FTRUNCATE64)
#       define ftruncate ftruncate64
#   endif
#endif

#ifdef USE_64_BIT_STDIO
#   ifdef HAS_FPOS64_T
#       undef Fpos_t
#       define Fpos_t fpos64_t
#   endif
/* Most 64-bit environments have defines like _LARGEFILE_SOURCE that
 * will trigger defines like the ones below.  Some 64-bit environments,
 * however, do not. */
#   if defined(USE_FOPEN64)
#       define fopen fopen64
#   endif
#   if defined(USE_FSEEK64)
#       define fseek fseek64 /* don't do fseeko here, see perlio.c */
#   endif
#   if defined(USE_FTELL64)
#       define ftell ftell64 /* don't do ftello here, see perlio.c */
#   endif
#   if defined(USE_FSETPOS64)
#       define fsetpos fsetpos64
#   endif
#   if defined(USE_FGETPOS64)
#       define fgetpos fgetpos64
#   endif
#   if defined(USE_TMPFILE64)
#       define tmpfile tmpfile64
#   endif
#   if defined(USE_FREOPEN64)
#       define freopen freopen64
#   endif
#endif

#if defined(OS2)
#  include "iperlsys.h"
#endif

#if defined(__OPEN_VM)
#   include "vmesa/vmesaish.h"
#   define ISHISH "vmesa"
#endif

#ifdef DOSISH
#   if defined(OS2)
#       include "os2ish.h"
#   else
#       include "dosish.h"
#   endif
#   define ISHISH "dos"
#endif

#if defined(VMS)
#   include "vmsish.h"
#   define ISHISH "vms"
#endif

#if defined(PLAN9)
#   include "./plan9/plan9ish.h"
#   define ISHISH "plan9"
#endif

#if defined(MPE)
#  include "mpeix/mpeixish.h"
#  define ISHISH "mpeix"
#endif

#if defined(__VOS__)
#   ifdef __GNUC__
#     include "./vos/vosish.h"
#   else
#     include "vos/vosish.h"
#   endif
#   define ISHISH "vos"
#endif

#if defined(EPOC)
#   include "epocish.h"
#   define ISHISH "epoc"
#endif

#ifdef __SYMBIAN32__
#   include "symbian/symbianish.h"
#   define ISHISH "symbian"
#endif


#if defined(__HAIKU__)
#   include "haiku/haikuish.h"
#   define ISHISH "haiku"
#elif defined(__BEOS__)
#   include "beos/beosish.h"
#   define ISHISH "beos"
#endif

#ifndef ISHISH
#   include "unixish.h"
#   define ISHISH "unix"
#endif

/* NSIG logic from Configure --> */
/* Strange style to avoid deeply-nested #if/#else/#endif */
#ifndef NSIG
#  ifdef _NSIG
#    define NSIG (_NSIG)
#  endif
#endif

#ifndef NSIG
#  ifdef SIGMAX
#    define NSIG (SIGMAX+1)
#  endif
#endif

#ifndef NSIG
#  ifdef SIG_MAX
#    define NSIG (SIG_MAX+1)
#  endif
#endif

#ifndef NSIG
#  ifdef _SIG_MAX
#    define NSIG (_SIG_MAX+1)
#  endif
#endif

#ifndef NSIG
#  ifdef MAXSIG
#    define NSIG (MAXSIG+1)
#  endif
#endif

#ifndef NSIG
#  ifdef MAX_SIG
#    define NSIG (MAX_SIG+1)
#  endif
#endif

#ifndef NSIG
#  ifdef SIGARRAYSIZE
#    define NSIG SIGARRAYSIZE /* Assume ary[SIGARRAYSIZE] */
#  endif
#endif

#ifndef NSIG
#  ifdef _sys_nsig
#    define NSIG (_sys_nsig) /* Solaris 2.5 */
#  endif
#endif

/* Default to some arbitrary number that's big enough to get most
   of the common signals.
*/
#ifndef NSIG
#    define NSIG 50
#endif
/* <-- NSIG logic from Configure */

#ifndef NO_ENVIRON_ARRAY
#  define USE_ENVIRON_ARRAY
#endif

/*
 * initialise to avoid floating-point exceptions from overflow, etc
 */
#ifndef PERL_FPU_INIT
#  ifdef HAS_FPSETMASK
#    if HAS_FLOATINGPOINT_H
#      include <floatingpoint.h>
#    endif
/* Some operating systems have this as a macro, which in turn expands to a comma
   expression, and the last sub-expression is something that gets calculated,
   and then they have the gall to warn that a value computed is not used. Hence
   cast to void.  */
#    define PERL_FPU_INIT (void)fpsetmask(0)
#  else
#    if defined(SIGFPE) && defined(SIG_IGN) && !defined(PERL_MICRO)
#      define PERL_FPU_INIT       PL_sigfpe_saved = (Sighandler_t) signal(SIGFPE, SIG_IGN)
#      define PERL_FPU_PRE_EXEC   { Sigsave_t xfpe; rsignal_save(SIGFPE, PL_sigfpe_saved, &xfpe);
#      define PERL_FPU_POST_EXEC    rsignal_restore(SIGFPE, &xfpe); }
#    else
#      define PERL_FPU_INIT

#    endif
#  endif
#endif
#ifndef PERL_FPU_PRE_EXEC
#  define PERL_FPU_PRE_EXEC   {
#  define PERL_FPU_POST_EXEC  }
#endif

#ifndef PERL_SYS_INIT3_BODY
#  define PERL_SYS_INIT3_BODY(argvp,argcp,envp) PERL_SYS_INIT_BODY(argvp,argcp)
#endif

/*
=for apidoc Am|void|PERL_SYS_INIT|int argc|char** argv
Provides system-specific tune up of the C runtime environment necessary to
run Perl interpreters. This should be called only once, before creating
any Perl interpreters.

=for apidoc Am|void|PERL_SYS_INIT3|int argc|char** argv|char** env
Provides system-specific tune up of the C runtime environment necessary to
run Perl interpreters. This should be called only once, before creating
any Perl interpreters.

=for apidoc Am|void|PERL_SYS_TERM|
Provides system-specific clean up of the C runtime environment after
running Perl interpreters. This should be called only once, after
freeing any remaining Perl interpreters.

=cut
 */

#define PERL_SYS_INIT(argc, argv)	Perl_sys_init(argc, argv)
#define PERL_SYS_INIT3(argc, argv, env)	Perl_sys_init3(argc, argv, env)
#define PERL_SYS_TERM()			Perl_sys_term()

#ifndef PERL_WRITE_MSG_TO_CONSOLE
#  define PERL_WRITE_MSG_TO_CONSOLE(io, msg, len) PerlIO_write(io, msg, len)
#endif

#ifndef MAXPATHLEN
#  ifdef PATH_MAX
#    ifdef _POSIX_PATH_MAX
#       if PATH_MAX > _POSIX_PATH_MAX
/* POSIX 1990 (and pre) was ambiguous about whether PATH_MAX
 * included the null byte or not.  Later amendments of POSIX,
 * XPG4, the Austin Group, and the Single UNIX Specification
 * all explicitly include the null byte in the PATH_MAX.
 * Ditto for _POSIX_PATH_MAX. */
#         define MAXPATHLEN PATH_MAX
#       else
#         define MAXPATHLEN _POSIX_PATH_MAX
#       endif
#    else
#      define MAXPATHLEN (PATH_MAX+1)
#    endif
#  else
#    define MAXPATHLEN 1024	/* Err on the large side. */
#  endif
#endif

/* In case Configure was not used (we are using a "canned config"
 * such as Win32, or a cross-compilation setup, for example) try going
 * by the gcc major and minor versions.  One useful URL is
 * http://www.ohse.de/uwe/articles/gcc-attributes.html,
 * but contrary to this information warn_unused_result seems
 * not to be in gcc 3.3.5, at least. --jhi
 * Also, when building extensions with an installed perl, this allows
 * the user to upgrade gcc and get the right attributes, rather than
 * relying on the list generated at Configure time.  --AD
 * Set these up now otherwise we get confused when some of the <*thread.h>
 * includes below indirectly pull in <perlio.h> (which needs to know if we
 * have HASATTRIBUTE_FORMAT).
 */

#ifndef PERL_MICRO
#if defined __GNUC__ && !defined(__INTEL_COMPILER)
#  if __GNUC__ == 3 && __GNUC_MINOR__ >= 1 || __GNUC__ > 3 /* 3.1 -> */
#    define HASATTRIBUTE_DEPRECATED
#  endif
#  if __GNUC__ >= 3 /* 3.0 -> */ /* XXX Verify this version */
#    define HASATTRIBUTE_FORMAT
#    if defined __MINGW32__
#      define PRINTF_FORMAT_NULL_OK
#    endif
#  endif
#  if __GNUC__ >= 3 /* 3.0 -> */
#    define HASATTRIBUTE_MALLOC
#  endif
#  if __GNUC__ == 3 && __GNUC_MINOR__ >= 3 || __GNUC__ > 3 /* 3.3 -> */
#    define HASATTRIBUTE_NONNULL
#  endif
#  if __GNUC__ == 2 && __GNUC_MINOR__ >= 5 || __GNUC__ > 2 /* 2.5 -> */
#    define HASATTRIBUTE_NORETURN
#  endif
#  if __GNUC__ >= 3 /* gcc 3.0 -> */
#    define HASATTRIBUTE_PURE
#  endif
#  if __GNUC__ == 3 && __GNUC_MINOR__ >= 4 || __GNUC__ > 3 /* 3.4 -> */
#    define HASATTRIBUTE_UNUSED
#  endif
#  if __GNUC__ == 3 && __GNUC_MINOR__ == 3 && !defined(__cplusplus)
#    define HASATTRIBUTE_UNUSED /* gcc-3.3, but not g++-3.3. */
#  endif
#  if __GNUC__ == 3 && __GNUC_MINOR__ >= 4 || __GNUC__ > 3 /* 3.4 -> */
#    define HASATTRIBUTE_WARN_UNUSED_RESULT
#  endif
#endif
#endif /* #ifndef PERL_MICRO */

/* USE_5005THREADS needs to be after unixish.h as <pthread.h> includes
 * <sys/signal.h> which defines NSIG - which will stop inclusion of <signal.h>
 * this results in many functions being undeclared which bothers C++
 * May make sense to have threads after "*ish.h" anyway
 */

#if defined(USE_ITHREADS)
#  ifdef NETWARE
#   include <nw5thread.h>
#  else
#  ifdef FAKE_THREADS
#    include "fakethr.h"
#  else
#    ifdef WIN32
#      include <win32thread.h>
#    else
#      ifdef OS2
#        include "os2thread.h"
#      else
#        ifdef I_MACH_CTHREADS
#          include <mach/cthreads.h>
#          if (defined(NeXT) || defined(__NeXT__)) && defined(PERL_POLLUTE_MALLOC)
#            define MUTEX_INIT_CALLS_MALLOC
#          endif
typedef cthread_t	perl_os_thread;
typedef mutex_t		perl_mutex;
typedef condition_t	perl_cond;
typedef void *		perl_key;
#        else /* Posix threads */
#          ifdef I_PTHREAD
#            include <pthread.h>
#          endif
typedef pthread_t	perl_os_thread;
typedef pthread_mutex_t	perl_mutex;
typedef pthread_cond_t	perl_cond;
typedef pthread_key_t	perl_key;
#        endif /* I_MACH_CTHREADS */
#      endif /* OS2 */
#    endif /* WIN32 */
#  endif /* FAKE_THREADS */
#endif	/* NETWARE */
#endif /* USE_ITHREADS */

#if defined(WIN32)
#  include "win32.h"
#endif

#ifdef NETWARE
#  include "netware.h"
#endif

#define STATUS_UNIX	PL_statusvalue
#ifdef VMS
#   define STATUS_NATIVE	PL_statusvalue_vms
/*
 * vaxc$errno is only guaranteed to be valid if errno == EVMSERR, otherwise
 * its contents can not be trusted.  Unfortunately, Perl seems to check
 * it on exit, so it when PL_statusvalue_vms is updated, vaxc$errno should
 * be updated also.
 */
#  include <stsdef.h>
#  include <ssdef.h>
/* Presume this because if VMS changes it, it will require a new
 * set of APIs for waiting on children for binary compatibility.
 */
#  define child_offset_bits (8)
#  ifndef C_FAC_POSIX
#  define C_FAC_POSIX 0x35A000
#  endif

/*  STATUS_EXIT - validates and returns a NATIVE exit status code for the
 * platform from the existing UNIX or Native status values.
 */

#   define STATUS_EXIT \
	(((I32)PL_statusvalue_vms == -1 ? SS$_ABORT : PL_statusvalue_vms) | \
	   (VMSISH_HUSHED ? STS$M_INHIB_MSG : 0))


/* STATUS_NATIVE_CHILD_SET - Calculate UNIX status that matches the child
 * exit code and shifts the UNIX value over the correct number of bits to
 * be a child status.  Usually the number of bits is 8, but that could be
 * platform dependent.  The NATIVE status code is presumed to have either
 * from a child process.
 */

/* This is complicated.  The child processes return a true native VMS
   status which must be saved.  But there is an assumption in Perl that
   the UNIX child status has some relationship to errno values, so
   Perl tries to translate it to text in some of the tests.
   In order to get the string translation correct, for the error, errno
   must be EVMSERR, but that generates a different text message
   than what the test programs are expecting.  So an errno value must
   be derived from the native status value when an error occurs.
   That will hide the true native status message.  With this version of
   perl, the true native child status can always be retrieved so that
   is not a problem.  But in this case, Pl_statusvalue and errno may
   have different values in them.
 */

#   define STATUS_NATIVE_CHILD_SET(n) \
	STMT_START {							\
	    I32 evalue = (I32)n;					\
	    if (evalue == EVMSERR) {					\
	      PL_statusvalue_vms = vaxc$errno;				\
	      PL_statusvalue = evalue;					\
	    } else {							\
	      PL_statusvalue_vms = evalue;				\
	      if (evalue == -1) {					\
		PL_statusvalue = -1;					\
		PL_statusvalue_vms = SS$_ABORT; /* Should not happen */ \
	      } else							\
		PL_statusvalue = Perl_vms_status_to_unix(evalue, 1);	\
	      set_vaxc_errno(evalue);					\
	      if ((PL_statusvalue_vms & C_FAC_POSIX) == C_FAC_POSIX)	\
		  set_errno(EVMSERR);					\
	      else set_errno(Perl_vms_status_to_unix(evalue, 0));	\
	      PL_statusvalue = PL_statusvalue << child_offset_bits;	\
	    }								\
	} STMT_END

#   ifdef VMSISH_STATUS
#	define STATUS_CURRENT	(VMSISH_STATUS ? STATUS_NATIVE : STATUS_UNIX)
#   else
#	define STATUS_CURRENT	STATUS_UNIX
#   endif

  /* STATUS_UNIX_SET - takes a UNIX/POSIX errno value and attempts to update
   * the NATIVE status to an equivalent value.  Can not be used to translate
   * exit code values as exit code values are not guaranteed to have any
   * relationship at all to errno values.
   * This is used when Perl is forcing errno to have a specific value.
   */
#   define STATUS_UNIX_SET(n)				\
	STMT_START {					\
	    I32 evalue = (I32)n;			\
	    PL_statusvalue = evalue;			\
	    if (PL_statusvalue != -1) {			\
		if (PL_statusvalue != EVMSERR) {	\
		  PL_statusvalue &= 0xFFFF;		\
		  if (MY_POSIX_EXIT)			\
		    PL_statusvalue_vms=PL_statusvalue ? SS$_ABORT : SS$_NORMAL;\
		  else PL_statusvalue_vms = Perl_unix_status_to_vms(evalue); \
		}					\
		else {					\
		  PL_statusvalue_vms = vaxc$errno;	\
		}					\
	    }						\
	    else PL_statusvalue_vms = SS$_ABORT;	\
	    set_vaxc_errno(PL_statusvalue_vms);		\
	} STMT_END

  /* STATUS_UNIX_EXIT_SET - Takes a UNIX/POSIX exit code and sets
   * the NATIVE error status based on it.
   *
   * When in the default mode to comply with the Perl VMS documentation,
   * 0 is a success and any other code sets the NATIVE status to a failure
   * code of SS$_ABORT.
   *
   * In the new POSIX EXIT mode, native status will be set so that the
   * actual exit code will can be retrieved by the calling program or
   * shell.
   *
   * If the exit code is not clearly a UNIX parent or child exit status,
   * it will be passed through as a VMS status.
   */

#   define STATUS_UNIX_EXIT_SET(n)			\
	STMT_START {					\
	    I32 evalue = (I32)n;			\
	    PL_statusvalue = evalue;			\
	    if (MY_POSIX_EXIT) { \
	      if (evalue <= 0xFF00) {		\
		  if (evalue > 0xFF)			\
		    evalue = (evalue >> child_offset_bits) & 0xFF; \
		  PL_statusvalue_vms =		\
		    (C_FAC_POSIX | (evalue << 3 ) |	\
		    ((evalue == 1) ? (STS$K_ERROR | STS$M_INHIB_MSG) : 1)); \
	      } else /* forgive them Perl, for they have sinned */ \
		PL_statusvalue_vms = evalue; \
	    } else { \
	      if (evalue == 0)			\
		PL_statusvalue_vms = SS$_NORMAL;	\
	      else if (evalue <= 0xFF00) \
		PL_statusvalue_vms = SS$_ABORT; \
	      else { /* forgive them Perl, for they have sinned */ \
		  if (evalue != EVMSERR) PL_statusvalue_vms = evalue; \
		  else PL_statusvalue_vms = vaxc$errno;	\
		  /* And obviously used a VMS status value instead of UNIX */ \
		  PL_statusvalue = EVMSERR;		\
	      } \
	      set_vaxc_errno(PL_statusvalue_vms);	\
	    }						\
	} STMT_END


  /* STATUS_EXIT_SET - Takes a NATIVE/UNIX/POSIX exit code
   * and sets the NATIVE error status based on it.  This special case
   * is needed to maintain compatibility with past VMS behavior.
   *
   * In the default mode on VMS, this number is passed through as
   * both the NATIVE and UNIX status.  Which makes it different
   * that the STATUS_UNIX_EXIT_SET.
   *
   * In the new POSIX EXIT mode, native status will be set so that the
   * actual exit code will can be retrieved by the calling program or
   * shell.
   *
   * A POSIX exit code is from 0 to 255.  If the exit code is higher
   * than this, it needs to be assumed that it is a VMS exit code and
   * passed through.
   */

#   define STATUS_EXIT_SET(n)				\
	STMT_START {					\
	    I32 evalue = (I32)n;			\
	    PL_statusvalue = evalue;			\
	    if (MY_POSIX_EXIT)				\
		if (evalue > 255) PL_statusvalue_vms = evalue; else {	\
		  PL_statusvalue_vms = \
		    (C_FAC_POSIX | (evalue << 3 ) |	\
		     ((evalue == 1) ? (STS$K_ERROR | STS$M_INHIB_MSG) : 1));} \
	    else					\
		PL_statusvalue_vms = evalue ? evalue : SS$_NORMAL; \
	    set_vaxc_errno(PL_statusvalue_vms);		\
	} STMT_END


 /* This macro forces a success status */
#   define STATUS_ALL_SUCCESS	\
	(PL_statusvalue = 0, PL_statusvalue_vms = SS$_NORMAL)

 /* This macro forces a failure status */
#   define STATUS_ALL_FAILURE	(PL_statusvalue = 1, \
     vaxc$errno = PL_statusvalue_vms = MY_POSIX_EXIT ? \
	(C_FAC_POSIX | (1 << 3) | STS$K_ERROR | STS$M_INHIB_MSG) : SS$_ABORT)

#else
#   define STATUS_NATIVE	PL_statusvalue_posix
#   if defined(WCOREDUMP)
#       define STATUS_NATIVE_CHILD_SET(n)                  \
            STMT_START {                                   \
                PL_statusvalue_posix = (n);                \
                if (PL_statusvalue_posix == -1)            \
                    PL_statusvalue = -1;                   \
                else {                                     \
                    PL_statusvalue =                       \
                        (WIFEXITED(PL_statusvalue_posix) ? (WEXITSTATUS(PL_statusvalue_posix) << 8) : 0) |  \
                        (WIFSIGNALED(PL_statusvalue_posix) ? (WTERMSIG(PL_statusvalue_posix) & 0x7F) : 0) | \
                        (WIFSIGNALED(PL_statusvalue_posix) && WCOREDUMP(PL_statusvalue_posix) ? 0x80 : 0);  \
                }                                          \
            } STMT_END
#   elif defined(WIFEXITED)
#       define STATUS_NATIVE_CHILD_SET(n)                  \
            STMT_START {                                   \
                PL_statusvalue_posix = (n);                \
                if (PL_statusvalue_posix == -1)            \
                    PL_statusvalue = -1;                   \
                else {                                     \
                    PL_statusvalue =                       \
                        (WIFEXITED(PL_statusvalue_posix) ? (WEXITSTATUS(PL_statusvalue_posix) << 8) : 0) |  \
                        (WIFSIGNALED(PL_statusvalue_posix) ? (WTERMSIG(PL_statusvalue_posix) & 0x7F) : 0);  \
                }                                          \
            } STMT_END
#   else
#       define STATUS_NATIVE_CHILD_SET(n)                  \
            STMT_START {                                   \
                PL_statusvalue_posix = (n);                \
                if (PL_statusvalue_posix == -1)            \
                    PL_statusvalue = -1;                   \
                else {                                     \
                    PL_statusvalue =                       \
                        PL_statusvalue_posix & 0xFFFF;     \
                }                                          \
            } STMT_END
#   endif
#   define STATUS_UNIX_SET(n)		\
	STMT_START {			\
	    PL_statusvalue = (n);		\
	    if (PL_statusvalue != -1)	\
		PL_statusvalue &= 0xFFFF;	\
	} STMT_END
#   define STATUS_UNIX_EXIT_SET(n) STATUS_UNIX_SET(n)
#   define STATUS_EXIT_SET(n) STATUS_UNIX_SET(n)
#   define STATUS_CURRENT STATUS_UNIX
#   define STATUS_EXIT STATUS_UNIX
#   define STATUS_ALL_SUCCESS	(PL_statusvalue = 0, PL_statusvalue_posix = 0)
#   define STATUS_ALL_FAILURE	(PL_statusvalue = 1, PL_statusvalue_posix = 1)
#endif

/* flags in PL_exit_flags for nature of exit() */
#define PERL_EXIT_EXPECTED	0x01
#define PERL_EXIT_DESTRUCT_END  0x02  /* Run END in perl_destruct */

#ifndef PERL_CORE
/* format to use for version numbers in file/directory names */
/* XXX move to Configure? */
/* This was only ever used for the current version, and that can be done at
   compile time, as PERL_FS_VERSION, so should we just delete it?  */
#  ifndef PERL_FS_VER_FMT
#    define PERL_FS_VER_FMT	"%d.%d.%d"
#  endif
#endif

#ifndef PERL_FS_VERSION
#  define PERL_FS_VERSION	PERL_VERSION_STRING
#endif

/* This defines a way to flush all output buffers.  This may be a
 * performance issue, so we allow people to disable it.  Also, if
 * we are using stdio, there are broken implementations of fflush(NULL)
 * out there, Solaris being the most prominent.
 */
#ifndef PERL_FLUSHALL_FOR_CHILD
# if defined(USE_PERLIO) || defined(FFLUSH_NULL) || defined(USE_SFIO)
#  define PERL_FLUSHALL_FOR_CHILD	PerlIO_flush((PerlIO*)NULL)
# else
#  ifdef FFLUSH_ALL
#   define PERL_FLUSHALL_FOR_CHILD	my_fflush_all()
#  else
#   define PERL_FLUSHALL_FOR_CHILD	NOOP
#  endif
# endif
#endif

#ifndef PERL_WAIT_FOR_CHILDREN
#  define PERL_WAIT_FOR_CHILDREN	NOOP
#endif

/* the traditional thread-unsafe notion of "current interpreter". */
#ifndef PERL_SET_INTERP
#  define PERL_SET_INTERP(i)		(PL_curinterp = (PerlInterpreter*)(i))
#endif

#ifndef PERL_GET_INTERP
#  define PERL_GET_INTERP		(PL_curinterp)
#endif

#if defined(PERL_IMPLICIT_CONTEXT) && !defined(PERL_GET_THX)
#  ifdef MULTIPLICITY
#    define PERL_GET_THX		((PerlInterpreter *)PERL_GET_CONTEXT)
#  endif
#  define PERL_SET_THX(t)		PERL_SET_CONTEXT(t)
#endif

/*
    This replaces the previous %_ "hack" by the "%p" hacks.
    All that is required is that the perl source does not
    use "%-p" or "%-<number>p" or "%<number>p" formats.
    These formats will still work in perl code.
    See comments in sv.c for further details.

    Robin Barker 2005-07-14

    No longer use %1p for VDf = %vd.  RMB 2007-10-19
*/

#ifndef SVf_
#  define SVf_(n) "-" STRINGIFY(n) "p"
#endif

#ifndef SVf
#  define SVf "-p"
#endif

#ifndef SVf32
#  define SVf32 SVf_(32)
#endif

#ifndef SVf256
#  define SVf256 SVf_(256)
#endif

#define SVfARG(p) ((void*)(p))

#ifndef HEKf
#  define HEKf "2p"
#endif

/* Not ideal, but we cannot easily include a number in an already-numeric
 * format sequence. */
#ifndef HEKf256
#  define HEKf256 "3p"
#endif

#define HEKfARG(p) ((void*)(p))

#ifdef PERL_CORE
/* not used; but needed for backward compatibility with XS code? - RMB */
#  undef VDf
#else
#  ifndef VDf
#    define VDf "vd"
#  endif
#endif

#ifdef PERL_CORE
/* not used; but needed for backward compatibility with XS code? - RMB */
#  undef UVf
#else
#  ifndef UVf
#    define UVf UVuf
#  endif
#endif

#ifdef HASATTRIBUTE_DEPRECATED
#  define __attribute__deprecated__         __attribute__((deprecated))
#endif
#ifdef HASATTRIBUTE_FORMAT
#  define __attribute__format__(x,y,z)      __attribute__((format(x,y,z)))
#endif
#ifdef HASATTRIBUTE_MALLOC
#  define __attribute__malloc__             __attribute__((__malloc__))
#endif
#ifdef HASATTRIBUTE_NONNULL
#  define __attribute__nonnull__(a)         __attribute__((nonnull(a)))
#endif
#ifdef HASATTRIBUTE_NORETURN
#  define __attribute__noreturn__           __attribute__((noreturn))
#endif
#ifdef HASATTRIBUTE_PURE
#  define __attribute__pure__               __attribute__((pure))
#endif
#ifdef HASATTRIBUTE_UNUSED
#  define __attribute__unused__             __attribute__((unused))
#endif
#ifdef HASATTRIBUTE_WARN_UNUSED_RESULT
#  define __attribute__warn_unused_result__ __attribute__((warn_unused_result))
#endif

/* If we haven't defined the attributes yet, define them to blank. */
#ifndef __attribute__deprecated__
#  define __attribute__deprecated__
#endif
#ifndef __attribute__format__
#  define __attribute__format__(x,y,z)
#endif
#ifndef __attribute__malloc__
#  define __attribute__malloc__
#endif
#ifndef __attribute__nonnull__
#  define __attribute__nonnull__(a)
#endif
#ifndef __attribute__noreturn__
#  define __attribute__noreturn__
#endif
#ifndef __attribute__pure__
#  define __attribute__pure__
#endif
#ifndef __attribute__unused__
#  define __attribute__unused__
#endif
#ifndef __attribute__warn_unused_result__
#  define __attribute__warn_unused_result__
#endif

/* For functions that are marked as __attribute__noreturn__, it's not
   appropriate to call return.  In either case, include the lint directive.
 */
#ifdef HASATTRIBUTE_NORETURN
#  define NORETURN_FUNCTION_END /* NOTREACHED */
#else
#  define NORETURN_FUNCTION_END /* NOTREACHED */ return 0
#endif

/* Some OS warn on NULL format to printf */
#ifdef PRINTF_FORMAT_NULL_OK
#  define __attribute__format__null_ok__(x,y,z)  __attribute__format__(x,y,z)
#else
#  define __attribute__format__null_ok__(x,y,z)
#endif

#ifdef HAS_BUILTIN_EXPECT
#  define EXPECT(expr,val)                  __builtin_expect(expr,val)
#else
#  define EXPECT(expr,val)                  (expr)
#endif
#define LIKELY(cond)                        EXPECT(cond,1)
#define UNLIKELY(cond)                      EXPECT(cond,0)
#ifdef HAS_BUILTIN_CHOOSE_EXPR
/* placeholder */
#endif

/* Some unistd.h's give a prototype for pause() even though
   HAS_PAUSE ends up undefined.  This causes the #define
   below to be rejected by the compiler.  Sigh.
*/
#ifdef HAS_PAUSE
#define Pause	pause
#else
#define Pause() sleep((32767<<16)+32767)
#endif

#ifndef IOCPARM_LEN
#   ifdef IOCPARM_MASK
	/* on BSDish systems we're safe */
#	define IOCPARM_LEN(x)  (((x) >> 16) & IOCPARM_MASK)
#   else
#	if defined(_IOC_SIZE) && defined(__GLIBC__)
	/* on Linux systems we're safe; except when we're not [perl #38223] */
#	    define IOCPARM_LEN(x) (_IOC_SIZE(x) < 256 ? 256 : _IOC_SIZE(x))
#	else
	/* otherwise guess at what's safe */
#	    define IOCPARM_LEN(x)	256
#	endif
#   endif
#endif

#if defined(__CYGWIN__)
/* USEMYBINMODE
 *   This symbol, if defined, indicates that the program should
 *   use the routine my_binmode(FILE *fp, char iotype, int mode) to insure
 *   that a file is in "binary" mode -- that is, that no translation
 *   of bytes occurs on read or write operations.
 */
#  define USEMYBINMODE /**/
#  include <io.h> /* for setmode() prototype */
#  define my_binmode(fp, iotype, mode) \
            (PerlLIO_setmode(fileno(fp), mode) != -1 ? TRUE : FALSE)
#endif

#ifdef __CYGWIN__
void init_os_extras(void);
#endif

#ifdef UNION_ANY_DEFINITION
UNION_ANY_DEFINITION;
#else
union any {
    void*	any_ptr;
    I32		any_i32;
    IV		any_iv;
    UV		any_uv;
    long	any_long;
    bool	any_bool;
    void	(*any_dptr) (void*);
    void	(*any_dxptr) (pTHX_ void*);
};
#endif

typedef I32 (*filter_t) (pTHX_ int, SV *, int);

#define FILTER_READ(idx, sv, len)  filter_read(idx, sv, len)
#define FILTER_DATA(idx) \
	    (PL_parser ? AvARRAY(PL_parser->rsfp_filters)[idx] : NULL)
#define FILTER_ISREADER(idx) \
	    (PL_parser && PL_parser->rsfp_filters \
		&& idx >= AvFILLp(PL_parser->rsfp_filters))
#define PERL_FILTER_EXISTS(i) \
	    (PL_parser && PL_parser->rsfp_filters \
		&& (i) <= av_len(PL_parser->rsfp_filters))

#if defined(_AIX) && !defined(_AIX43)
#if defined(USE_REENTRANT) || defined(_REENTRANT) || defined(_THREAD_SAFE)
/* We cannot include <crypt.h> to get the struct crypt_data
 * because of setkey prototype problems when threading */
typedef        struct crypt_data {     /* straight from /usr/include/crypt.h */
    /* From OSF, Not needed in AIX
       char C[28], D[28];
    */
    char E[48];
    char KS[16][48];
    char block[66];
    char iobuf[16];
} CRYPTD;
#endif /* threading */
#endif /* AIX */

#if !defined(OS2)
#  include "iperlsys.h"
#endif

#ifdef __LIBCATAMOUNT__
#undef HAS_PASSWD  /* unixish.h but not unixish enough. */
#undef HAS_GROUP
#define FAKE_BIT_BUCKET
#endif

/* [perl #22371] Algorimic Complexity Attack on Perl 5.6.1, 5.8.0.
 * Note that the USE_HASH_SEED and USE_HASH_SEED_EXPLICIT are *NOT*
 * defined by Configure, despite their names being similar to the
 * other defines like USE_ITHREADS.  Configure in fact knows nothing
 * about the randomised hashes.  Therefore to enable/disable the hash
 * randomisation defines use the Configure -Accflags=... instead. */
#if !defined(NO_HASH_SEED) && !defined(USE_HASH_SEED) && !defined(USE_HASH_SEED_EXPLICIT)
#  define USE_HASH_SEED
#endif

/* Win32 defines a type 'WORD' in windef.h. This conflicts with the enumerator
 * 'WORD' defined in perly.h. The yytokentype enum is only a debugging aid, so
 * it's not really needed.
 */
#if defined(WIN32)
#  define YYTOKENTYPE
#endif
#include "perly.h"

#ifdef PERL_MAD
struct nexttoken {
    YYSTYPE next_val;	/* value of next token, if any */
    I32 next_type;	/* type of next token */
    MADPROP *next_mad;	/* everything else about that token */
};
#endif

/* macros to define bit-fields in structs. */
#ifndef PERL_BITFIELD8
#  define PERL_BITFIELD8 unsigned
#endif
#ifndef PERL_BITFIELD16
#  define PERL_BITFIELD16 unsigned
#endif
#ifndef PERL_BITFIELD32
#  define PERL_BITFIELD32 unsigned
#endif

#include "sv.h"
#include "regexp.h"
#include "util.h"
#include "form.h"
#include "gv.h"
#include "pad.h"
#include "cv.h"
#include "opnames.h"
#include "op.h"
#include "hv.h"
#include "cop.h"
#include "av.h"
#include "mg.h"
#include "scope.h"
#include "warnings.h"
#include "utf8.h"

/* these would be in doio.h if there was such a file */
#define my_stat()  my_stat_flags(SV_GMAGIC)
#define my_lstat() my_lstat_flags(SV_GMAGIC)

/* defined in sv.c, but also used in [ach]v.c */
#undef _XPV_HEAD
#undef _XPVMG_HEAD
#undef _XPVCV_COMMON

typedef struct _sublex_info SUBLEXINFO;
struct _sublex_info {
    U8 super_state;	/* lexer state to save */
    U16 sub_inwhat;	/* "lex_inwhat" to use */
    OP *sub_op;		/* "lex_op" to use */
    char *super_bufptr;	/* PL_parser->bufptr that was */
    char *super_bufend;	/* PL_parser->bufend that was */
};

#include "parser.h"

typedef struct magic_state MGS;	/* struct magic_state defined in mg.c */

struct scan_data_t;		/* Used in S_* functions in regcomp.c */
struct regnode_charclass_class;	/* Used in S_* functions in regcomp.c */

struct ptr_tbl_ent {
    struct ptr_tbl_ent*		next;
    const void*			oldval;
    void*			newval;
};

struct ptr_tbl {
    struct ptr_tbl_ent**	tbl_ary;
    UV				tbl_max;
    UV				tbl_items;
    struct ptr_tbl_arena	*tbl_arena;
    struct ptr_tbl_ent		*tbl_arena_next;
    struct ptr_tbl_ent		*tbl_arena_end;
};

#if defined(iAPX286) || defined(M_I286) || defined(I80286)
#   define I286
#endif

#if defined(htonl) && !defined(HAS_HTONL)
#define HAS_HTONL
#endif
#if defined(htons) && !defined(HAS_HTONS)
#define HAS_HTONS
#endif
#if defined(ntohl) && !defined(HAS_NTOHL)
#define HAS_NTOHL
#endif
#if defined(ntohs) && !defined(HAS_NTOHS)
#define HAS_NTOHS
#endif
#ifndef HAS_HTONL
#if (BYTEORDER & 0xffff) != 0x4321
#define HAS_HTONS
#define HAS_HTONL
#define HAS_NTOHS
#define HAS_NTOHL
#define MYSWAP
#define htons my_swap
#define htonl my_htonl
#define ntohs my_swap
#define ntohl my_ntohl
#endif
#else
#if (BYTEORDER & 0xffff) == 0x4321
#undef HAS_HTONS
#undef HAS_HTONL
#undef HAS_NTOHS
#undef HAS_NTOHL
#endif
#endif

/*
 * Little-endian byte order functions - 'v' for 'VAX', or 'reVerse'.
 * -DWS
 */
#if BYTEORDER != 0x1234
# define HAS_VTOHL
# define HAS_VTOHS
# define HAS_HTOVL
# define HAS_HTOVS
# if BYTEORDER == 0x4321 || BYTEORDER == 0x87654321
#  define vtohl(x)	((((x)&0xFF)<<24)	\
			+(((x)>>24)&0xFF)	\
			+(((x)&0x0000FF00)<<8)	\
			+(((x)&0x00FF0000)>>8)	)
#  define vtohs(x)	((((x)&0xFF)<<8) + (((x)>>8)&0xFF))
#  define htovl(x)	vtohl(x)
#  define htovs(x)	vtohs(x)
# endif
	/* otherwise default to functions in util.c */
#ifndef htovs
short htovs(short n);
short vtohs(short n);
long htovl(long n);
long vtohl(long n);
#endif
#endif

/* *MAX Plus 1. A floating point value.
   Hopefully expressed in a way that dodgy floating point can't mess up.
   >> 2 rather than 1, so that value is safely less than I32_MAX after 1
   is added to it
   May find that some broken compiler will want the value cast to I32.
   [after the shift, as signed >> may not be as secure as unsigned >>]
*/
#define I32_MAX_P1 (2.0 * (1 + (((U32)I32_MAX) >> 1)))
#define U32_MAX_P1 (4.0 * (1 + ((U32_MAX) >> 2)))
/* For compilers that can't correctly cast NVs over 0x7FFFFFFF (or
   0x7FFFFFFFFFFFFFFF) to an unsigned integer. In the future, sizeof(UV)
   may be greater than sizeof(IV), so don't assume that half max UV is max IV.
*/
#define U32_MAX_P1_HALF (2.0 * (1 + ((U32_MAX) >> 2)))

#define UV_MAX_P1 (4.0 * (1 + ((UV_MAX) >> 2)))
#define IV_MAX_P1 (2.0 * (1 + (((UV)IV_MAX) >> 1)))
#define UV_MAX_P1_HALF (2.0 * (1 + ((UV_MAX) >> 2)))

/* This may look like unnecessary jumping through hoops, but converting
   out of range floating point values to integers *is* undefined behaviour,
   and it is starting to bite.
*/
#ifndef CAST_INLINE
#define I_32(what) (cast_i32((NV)(what)))
#define U_32(what) (cast_ulong((NV)(what)))
#define I_V(what) (cast_iv((NV)(what)))
#define U_V(what) (cast_uv((NV)(what)))
#else
#define I_32(n) ((n) < I32_MAX_P1 ? ((n) < I32_MIN ? I32_MIN : (I32) (n)) \
                  : ((n) < U32_MAX_P1 ? (I32)(U32) (n) \
                     : ((n) > 0 ? (I32) U32_MAX : 0 /* NaN */)))
#define U_32(n) ((n) < 0.0 ? ((n) < I32_MIN ? (UV) I32_MIN : (U32)(I32) (n)) \
                  : ((n) < U32_MAX_P1 ? (U32) (n) \
                     : ((n) > 0 ? U32_MAX : 0 /* NaN */)))
#define I_V(n) ((n) < IV_MAX_P1 ? ((n) < IV_MIN ? IV_MIN : (IV) (n)) \
                  : ((n) < UV_MAX_P1 ? (IV)(UV) (n) \
                     : ((n) > 0 ? (IV)UV_MAX : 0 /* NaN */)))
#define U_V(n) ((n) < 0.0 ? ((n) < IV_MIN ? (UV) IV_MIN : (UV)(IV) (n)) \
                  : ((n) < UV_MAX_P1 ? (UV) (n) \
                     : ((n) > 0 ? UV_MAX : 0 /* NaN */)))
#endif

#define U_S(what) ((U16)U_32(what))
#define U_I(what) ((unsigned int)U_32(what))
#define U_L(what) U_32(what)

#ifdef HAS_SIGNBIT
#  define Perl_signbit signbit
#endif

/* These do not care about the fractional part, only about the range. */
#define NV_WITHIN_IV(nv) (I_V(nv) >= IV_MIN && I_V(nv) <= IV_MAX)
#define NV_WITHIN_UV(nv) ((nv)>=0.0 && U_V(nv) >= UV_MIN && U_V(nv) <= UV_MAX)

/* Used with UV/IV arguments: */
					/* XXXX: need to speed it up */
#define CLUMP_2UV(iv)	((iv) < 0 ? 0 : (UV)(iv))
#define CLUMP_2IV(uv)	((uv) > (UV)IV_MAX ? IV_MAX : (IV)(uv))

#ifndef MAXSYSFD
#   define MAXSYSFD 2
#endif

#ifndef __cplusplus
#if !(defined(UNDER_CE) || defined(SYMBIAN))
Uid_t getuid (void);
Uid_t geteuid (void);
Gid_t getgid (void);
Gid_t getegid (void);
#endif
#endif

#ifndef Perl_debug_log
#  define Perl_debug_log	PerlIO_stderr()
#endif

#ifndef Perl_error_log
#  define Perl_error_log	(PL_stderrgv			\
				 && isGV(PL_stderrgv)		\
				 && GvIOp(PL_stderrgv)          \
				 && IoOFP(GvIOp(PL_stderrgv))	\
				 ? IoOFP(GvIOp(PL_stderrgv))	\
				 : PerlIO_stderr())
#endif


#define DEBUG_p_FLAG		0x00000001 /*      1 */
#define DEBUG_s_FLAG		0x00000002 /*      2 */
#define DEBUG_l_FLAG		0x00000004 /*      4 */
#define DEBUG_t_FLAG		0x00000008 /*      8 */
#define DEBUG_o_FLAG		0x00000010 /*     16 */
#define DEBUG_c_FLAG		0x00000020 /*     32 */
#define DEBUG_P_FLAG		0x00000040 /*     64 */
#define DEBUG_m_FLAG		0x00000080 /*    128 */
#define DEBUG_f_FLAG		0x00000100 /*    256 */
#define DEBUG_r_FLAG		0x00000200 /*    512 */
#define DEBUG_x_FLAG		0x00000400 /*   1024 */
#define DEBUG_u_FLAG		0x00000800 /*   2048 */
/* U is reserved for Unofficial, exploratory hacking */
#define DEBUG_U_FLAG		0x00001000 /*   4096 */
#define DEBUG_H_FLAG		0x00002000 /*   8192 */
#define DEBUG_X_FLAG		0x00004000 /*  16384 */
#define DEBUG_D_FLAG		0x00008000 /*  32768 */
/* 0x00010000 is unused, used to be S */
#define DEBUG_T_FLAG		0x00020000 /* 131072 */
#define DEBUG_R_FLAG		0x00040000 /* 262144 */
#define DEBUG_J_FLAG		0x00080000 /* 524288 */
#define DEBUG_v_FLAG		0x00100000 /*1048576 */
#define DEBUG_C_FLAG		0x00200000 /*2097152 */
#define DEBUG_A_FLAG		0x00400000 /*4194304 */
#define DEBUG_q_FLAG		0x00800000 /*8388608 */
#define DEBUG_M_FLAG		0x01000000 /*16777216*/
#define DEBUG_B_FLAG		0x02000000 /*33554432*/
#define DEBUG_MASK		0x03FEEFFF /* mask of all the standard flags */

#define DEBUG_DB_RECURSE_FLAG	0x40000000
#define DEBUG_TOP_FLAG		0x80000000 /* XXX what's this for ??? Signal
					      that something was done? */

#  define DEBUG_p_TEST_ (PL_debug & DEBUG_p_FLAG)
#  define DEBUG_s_TEST_ (PL_debug & DEBUG_s_FLAG)
#  define DEBUG_l_TEST_ (PL_debug & DEBUG_l_FLAG)
#  define DEBUG_t_TEST_ (PL_debug & DEBUG_t_FLAG)
#  define DEBUG_o_TEST_ (PL_debug & DEBUG_o_FLAG)
#  define DEBUG_c_TEST_ (PL_debug & DEBUG_c_FLAG)
#  define DEBUG_P_TEST_ (PL_debug & DEBUG_P_FLAG)
#  define DEBUG_m_TEST_ (PL_debug & DEBUG_m_FLAG)
#  define DEBUG_f_TEST_ (PL_debug & DEBUG_f_FLAG)
#  define DEBUG_r_TEST_ (PL_debug & DEBUG_r_FLAG)
#  define DEBUG_x_TEST_ (PL_debug & DEBUG_x_FLAG)
#  define DEBUG_u_TEST_ (PL_debug & DEBUG_u_FLAG)
#  define DEBUG_U_TEST_ (PL_debug & DEBUG_U_FLAG)
#  define DEBUG_H_TEST_ (PL_debug & DEBUG_H_FLAG)
#  define DEBUG_X_TEST_ (PL_debug & DEBUG_X_FLAG)
#  define DEBUG_D_TEST_ (PL_debug & DEBUG_D_FLAG)
#  define DEBUG_T_TEST_ (PL_debug & DEBUG_T_FLAG)
#  define DEBUG_R_TEST_ (PL_debug & DEBUG_R_FLAG)
#  define DEBUG_J_TEST_ (PL_debug & DEBUG_J_FLAG)
#  define DEBUG_v_TEST_ (PL_debug & DEBUG_v_FLAG)
#  define DEBUG_C_TEST_ (PL_debug & DEBUG_C_FLAG)
#  define DEBUG_A_TEST_ (PL_debug & DEBUG_A_FLAG)
#  define DEBUG_q_TEST_ (PL_debug & DEBUG_q_FLAG)
#  define DEBUG_M_TEST_ (PL_debug & DEBUG_M_FLAG)
#  define DEBUG_B_TEST_ (PL_debug & DEBUG_B_FLAG)
#  define DEBUG_Xv_TEST_ (DEBUG_X_TEST_ && DEBUG_v_TEST_)
#  define DEBUG_Uv_TEST_ (DEBUG_U_TEST_ && DEBUG_v_TEST_)
#  define DEBUG_Pv_TEST_ (DEBUG_P_TEST_ && DEBUG_v_TEST_)

#ifdef DEBUGGING

#  define DEBUG_p_TEST DEBUG_p_TEST_
#  define DEBUG_s_TEST DEBUG_s_TEST_
#  define DEBUG_l_TEST DEBUG_l_TEST_
#  define DEBUG_t_TEST DEBUG_t_TEST_
#  define DEBUG_o_TEST DEBUG_o_TEST_
#  define DEBUG_c_TEST DEBUG_c_TEST_
#  define DEBUG_P_TEST DEBUG_P_TEST_
#  define DEBUG_m_TEST DEBUG_m_TEST_
#  define DEBUG_f_TEST DEBUG_f_TEST_
#  define DEBUG_r_TEST DEBUG_r_TEST_
#  define DEBUG_x_TEST DEBUG_x_TEST_
#  define DEBUG_u_TEST DEBUG_u_TEST_
#  define DEBUG_U_TEST DEBUG_U_TEST_
#  define DEBUG_H_TEST DEBUG_H_TEST_
#  define DEBUG_X_TEST DEBUG_X_TEST_
#  define DEBUG_D_TEST DEBUG_D_TEST_
#  define DEBUG_T_TEST DEBUG_T_TEST_
#  define DEBUG_R_TEST DEBUG_R_TEST_
#  define DEBUG_J_TEST DEBUG_J_TEST_
#  define DEBUG_v_TEST DEBUG_v_TEST_
#  define DEBUG_C_TEST DEBUG_C_TEST_
#  define DEBUG_A_TEST DEBUG_A_TEST_
#  define DEBUG_q_TEST DEBUG_q_TEST_
#  define DEBUG_M_TEST DEBUG_M_TEST_
#  define DEBUG_B_TEST DEBUG_B_TEST_
#  define DEBUG_Xv_TEST DEBUG_Xv_TEST_
#  define DEBUG_Uv_TEST DEBUG_Uv_TEST_
#  define DEBUG_Pv_TEST DEBUG_Pv_TEST_

#  define PERL_DEB(a)                  a
#  define PERL_DEBUG(a) if (PL_debug)  a
#  define DEBUG_p(a) if (DEBUG_p_TEST) a
#  define DEBUG_s(a) if (DEBUG_s_TEST) a
#  define DEBUG_l(a) if (DEBUG_l_TEST) a
#  define DEBUG_t(a) if (DEBUG_t_TEST) a
#  define DEBUG_o(a) if (DEBUG_o_TEST) a
#  define DEBUG_c(a) if (DEBUG_c_TEST) a
#  define DEBUG_P(a) if (DEBUG_P_TEST) a

     /* Temporarily turn off memory debugging in case the a
      * does memory allocation, either directly or indirectly. */
#  define DEBUG_m(a)  \
    STMT_START {							\
        if (PERL_GET_INTERP) { dTHX; if (DEBUG_m_TEST) {PL_debug&=~DEBUG_m_FLAG; a; PL_debug|=DEBUG_m_FLAG;} } \
    } STMT_END

#  define DEBUG__(t, a) \
	STMT_START { \
		if (t) STMT_START {a;} STMT_END; \
	} STMT_END

#  define DEBUG_f(a) DEBUG__(DEBUG_f_TEST, a)
#ifndef PERL_EXT_RE_BUILD
#  define DEBUG_r(a) DEBUG__(DEBUG_r_TEST, a)
#else
#  define DEBUG_r(a) STMT_START {a;} STMT_END
#endif /* PERL_EXT_RE_BUILD */
#  define DEBUG_x(a) DEBUG__(DEBUG_x_TEST, a)
#  define DEBUG_u(a) DEBUG__(DEBUG_u_TEST, a)
#  define DEBUG_U(a) DEBUG__(DEBUG_U_TEST, a)
#  define DEBUG_H(a) DEBUG__(DEBUG_H_TEST, a)
#  define DEBUG_X(a) DEBUG__(DEBUG_X_TEST, a)
#  define DEBUG_D(a) DEBUG__(DEBUG_D_TEST, a)
#  define DEBUG_Xv(a) DEBUG__(DEBUG_Xv_TEST, a)
#  define DEBUG_Uv(a) DEBUG__(DEBUG_Uv_TEST, a)
#  define DEBUG_Pv(a) DEBUG__(DEBUG_Pv_TEST, a)

#  define DEBUG_T(a) DEBUG__(DEBUG_T_TEST, a)
#  define DEBUG_R(a) DEBUG__(DEBUG_R_TEST, a)
#  define DEBUG_v(a) DEBUG__(DEBUG_v_TEST, a)
#  define DEBUG_C(a) DEBUG__(DEBUG_C_TEST, a)
#  define DEBUG_A(a) DEBUG__(DEBUG_A_TEST, a)
#  define DEBUG_q(a) DEBUG__(DEBUG_q_TEST, a)
#  define DEBUG_M(a) DEBUG__(DEBUG_M_TEST, a)
#  define DEBUG_B(a) DEBUG__(DEBUG_B_TEST, a)

#else /* DEBUGGING */

#  define DEBUG_p_TEST (0)
#  define DEBUG_s_TEST (0)
#  define DEBUG_l_TEST (0)
#  define DEBUG_t_TEST (0)
#  define DEBUG_o_TEST (0)
#  define DEBUG_c_TEST (0)
#  define DEBUG_P_TEST (0)
#  define DEBUG_m_TEST (0)
#  define DEBUG_f_TEST (0)
#  define DEBUG_r_TEST (0)
#  define DEBUG_x_TEST (0)
#  define DEBUG_u_TEST (0)
#  define DEBUG_U_TEST (0)
#  define DEBUG_H_TEST (0)
#  define DEBUG_X_TEST (0)
#  define DEBUG_D_TEST (0)
#  define DEBUG_T_TEST (0)
#  define DEBUG_R_TEST (0)
#  define DEBUG_J_TEST (0)
#  define DEBUG_v_TEST (0)
#  define DEBUG_C_TEST (0)
#  define DEBUG_A_TEST (0)
#  define DEBUG_q_TEST (0)
#  define DEBUG_M_TEST (0)
#  define DEBUG_B_TEST (0)
#  define DEBUG_Xv_TEST (0)
#  define DEBUG_Uv_TEST (0)
#  define DEBUG_Pv_TEST (0)

#  define PERL_DEB(a)
#  define PERL_DEBUG(a)
#  define DEBUG_p(a)
#  define DEBUG_s(a)
#  define DEBUG_l(a)
#  define DEBUG_t(a)
#  define DEBUG_o(a)
#  define DEBUG_c(a)
#  define DEBUG_P(a)
#  define DEBUG_m(a)
#  define DEBUG_f(a)
#  define DEBUG_r(a)
#  define DEBUG_x(a)
#  define DEBUG_u(a)
#  define DEBUG_U(a)
#  define DEBUG_H(a)
#  define DEBUG_X(a)
#  define DEBUG_D(a)
#  define DEBUG_T(a)
#  define DEBUG_R(a)
#  define DEBUG_v(a)
#  define DEBUG_C(a)
#  define DEBUG_A(a)
#  define DEBUG_q(a)
#  define DEBUG_M(a)
#  define DEBUG_B(a)
#  define DEBUG_Xv(a)
#  define DEBUG_Uv(a)
#  define DEBUG_Pv(a)
#endif /* DEBUGGING */


#define DEBUG_SCOPE(where) \
    DEBUG_l( \
    Perl_deb(aTHX_ "%s scope %ld (savestack=%ld) at %s:%d\n",	\
		    where, (long)PL_scopestack_ix, (long)PL_savestack_ix, \
		    __FILE__, __LINE__));

#if defined(DEBUGGING) && defined(I_ASSERT)
#  include <assert.h>
#endif

/* Keep the old croak based assert for those who want it, and as a fallback if
   the platform is so heretically non-ANSI that it can't assert.  */

#define Perl_assert(what)	PERL_DEB( 				\
	((what) ? ((void) 0) :						\
	    (Perl_croak_nocontext("Assertion %s failed: file \"" __FILE__ \
			"\", line %d", STRINGIFY(what), __LINE__),	\
	    (void) 0)))

#ifndef assert
#  define assert(what)	Perl_assert(what)
#endif

struct ufuncs {
    I32 (*uf_val)(pTHX_ IV, SV*);
    I32 (*uf_set)(pTHX_ IV, SV*);
    IV uf_index;
};

/* In pre-5.7-Perls the PERL_MAGIC_uvar magic didn't get the thread context.
 * XS code wanting to be backward compatible can do something
 * like the following:

#ifndef PERL_MG_UFUNC
#define PERL_MG_UFUNC(name,ix,sv) I32 name(IV ix, SV *sv)
#endif

static PERL_MG_UFUNC(foo_get, index, val)
{
    sv_setsv(val, ...);
    return TRUE;
}

-- Doug MacEachern

*/

#ifndef PERL_MG_UFUNC
#define PERL_MG_UFUNC(name,ix,sv) I32 name(pTHX_ IV ix, SV *sv)
#endif

/* Fix these up for __STDC__ */
#ifndef DONT_DECLARE_STD
char *mktemp (char*);
#ifndef atof
double atof (const char*);
#endif
#endif

#ifndef STANDARD_C
/* All of these are in stdlib.h or time.h for ANSI C */
Time_t time();
struct tm *gmtime(), *localtime();
#if defined(OEMVS) || defined(__OPEN_VM)
char *(strchr)(), *(strrchr)();
char *(strcpy)(), *(strcat)();
#else
char *strchr(), *strrchr();
char *strcpy(), *strcat();
#endif
#endif /* ! STANDARD_C */


#ifdef I_MATH
#    include <math.h>
#else
START_EXTERN_C
	    double exp (double);
	    double log (double);
	    double log10 (double);
	    double sqrt (double);
	    double frexp (double,int*);
	    double ldexp (double,int);
	    double modf (double,double*);
	    double sin (double);
	    double cos (double);
	    double atan2 (double,double);
	    double pow (double,double);
END_EXTERN_C
#endif

#if !defined(NV_INF) && defined(USE_LONG_DOUBLE) && defined(LDBL_INFINITY)
#  define NV_INF LDBL_INFINITY
#endif
#if !defined(NV_INF) && defined(DBL_INFINITY)
#  define NV_INF (NV)DBL_INFINITY
#endif
#if !defined(NV_INF) && defined(INFINITY)
#  define NV_INF (NV)INFINITY
#endif
#if !defined(NV_INF) && defined(INF)
#  define NV_INF (NV)INF
#endif
#if !defined(NV_INF) && defined(USE_LONG_DOUBLE) && defined(HUGE_VALL)
#  define NV_INF (NV)HUGE_VALL
#endif
#if !defined(NV_INF) && defined(HUGE_VAL)
#  define NV_INF (NV)HUGE_VAL
#endif

#if !defined(NV_NAN) && defined(USE_LONG_DOUBLE)
#   if !defined(NV_NAN) && defined(LDBL_NAN)
#       define NV_NAN LDBL_NAN
#   endif
#   if !defined(NV_NAN) && defined(LDBL_QNAN)
#       define NV_NAN LDBL_QNAN
#   endif
#   if !defined(NV_NAN) && defined(LDBL_SNAN)
#       define NV_NAN LDBL_SNAN
#   endif
#endif
#if !defined(NV_NAN) && defined(DBL_NAN)
#  define NV_NAN (NV)DBL_NAN
#endif
#if !defined(NV_NAN) && defined(DBL_QNAN)
#  define NV_NAN (NV)DBL_QNAN
#endif
#if !defined(NV_NAN) && defined(DBL_SNAN)
#  define NV_NAN (NV)DBL_SNAN
#endif
#if !defined(NV_NAN) && defined(QNAN)
#  define NV_NAN (NV)QNAN
#endif
#if !defined(NV_NAN) && defined(SNAN)
#  define NV_NAN (NV)SNAN
#endif
#if !defined(NV_NAN) && defined(NAN)
#  define NV_NAN (NV)NAN
#endif

#ifndef __cplusplus
#  if defined(NeXT) || defined(__NeXT__) /* or whatever catches all NeXTs */
char *crypt ();       /* Maybe more hosts will need the unprototyped version */
#  else
#    if !defined(WIN32) && !defined(VMS)
#ifndef crypt
char *crypt (const char*, const char*);
#endif
#    endif /* !WIN32 */
#  endif /* !NeXT && !__NeXT__ */
#  ifndef DONT_DECLARE_STD
#    ifndef getenv
char *getenv (const char*);
#    endif /* !getenv */
#    if !defined(HAS_LSEEK_PROTO) && !defined(EPOC) && !defined(__hpux)
#      ifdef _FILE_OFFSET_BITS
#        if _FILE_OFFSET_BITS == 64
Off_t lseek (int,Off_t,int);
#        endif
#      endif
#    endif
#  endif /* !DONT_DECLARE_STD */
#ifndef getlogin
char *getlogin (void);
#endif
#endif /* !__cplusplus */

/* Fixme on VMS.  This needs to be a run-time, not build time options */
/* Also rename() is affected by this */
#ifdef UNLINK_ALL_VERSIONS /* Currently only makes sense for VMS */
#define UNLINK unlnk
I32 unlnk (pTHX_ const char*);
#else
#define UNLINK PerlLIO_unlink
#endif

/* some versions of glibc are missing the setresuid() proto */
#if defined(HAS_SETRESUID) && !defined(HAS_SETRESUID_PROTO)
int setresuid(uid_t ruid, uid_t euid, uid_t suid);
#endif
/* some versions of glibc are missing the setresgid() proto */
#if defined(HAS_SETRESGID) && !defined(HAS_SETRESGID_PROTO)
int setresgid(gid_t rgid, gid_t egid, gid_t sgid);
#endif

#ifndef HAS_SETREUID
#  ifdef HAS_SETRESUID
#    define setreuid(r,e) setresuid(r,e,(Uid_t)-1)
#    define HAS_SETREUID
#  endif
#endif
#ifndef HAS_SETREGID
#  ifdef HAS_SETRESGID
#    define setregid(r,e) setresgid(r,e,(Gid_t)-1)
#    define HAS_SETREGID
#  endif
#endif

/* Sighandler_t defined in iperlsys.h */

#ifdef HAS_SIGACTION
typedef struct sigaction Sigsave_t;
#else
typedef Sighandler_t Sigsave_t;
#endif

#define SCAN_DEF 0
#define SCAN_TR 1
#define SCAN_REPL 2

#ifdef DEBUGGING
# ifndef register
#  define register
# endif
# define RUNOPS_DEFAULT Perl_runops_debug
#else
# define RUNOPS_DEFAULT Perl_runops_standard
#endif

#ifdef USE_PERLIO
EXTERN_C void PerlIO_teardown(void);
# ifdef USE_ITHREADS
#  define PERLIO_INIT MUTEX_INIT(&PL_perlio_mutex)
#  define PERLIO_TERM 				\
	STMT_START {				\
		PerlIO_teardown();		\
		MUTEX_DESTROY(&PL_perlio_mutex);\
	} STMT_END
# else
#  define PERLIO_INIT
#  define PERLIO_TERM	PerlIO_teardown()
# endif
#else
#  define PERLIO_INIT
#  define PERLIO_TERM
#endif

#ifdef MYMALLOC
#  ifdef MUTEX_INIT_CALLS_MALLOC
#    define MALLOC_INIT					\
	STMT_START {					\
		PL_malloc_mutex = NULL;			\
		MUTEX_INIT(&PL_malloc_mutex);		\
	} STMT_END
#    define MALLOC_TERM					\
	STMT_START {					\
		perl_mutex tmp = PL_malloc_mutex;	\
		PL_malloc_mutex = NULL;			\
		MUTEX_DESTROY(&tmp);			\
	} STMT_END
#  else
#    define MALLOC_INIT MUTEX_INIT(&PL_malloc_mutex)
#    define MALLOC_TERM MUTEX_DESTROY(&PL_malloc_mutex)
#  endif
#else
#  define MALLOC_INIT
#  define MALLOC_TERM
#endif

#if defined(PERL_IMPLICIT_CONTEXT)

struct perl_memory_debug_header;
struct perl_memory_debug_header {
  tTHX	interpreter;
#  ifdef PERL_POISON
  MEM_SIZE size;
#  endif
  struct perl_memory_debug_header *prev;
  struct perl_memory_debug_header *next;
};

#  define sTHX	(sizeof(struct perl_memory_debug_header) + \
	(MEM_ALIGNBYTES - sizeof(struct perl_memory_debug_header) \
	 %MEM_ALIGNBYTES) % MEM_ALIGNBYTES)

#else
#  define sTHX	0
#endif

#ifdef PERL_TRACK_MEMPOOL
#  define INIT_TRACK_MEMPOOL(header, interp)			\
	STMT_START {						\
		(header).interpreter = (interp);		\
		(header).prev = (header).next = &(header);	\
	} STMT_END
#  else
#  define INIT_TRACK_MEMPOOL(header, interp)
#endif

#ifdef I_MALLOCMALLOC
/* Needed for malloc_size(), malloc_good_size() on some systems */
#  include <malloc/malloc.h>
#endif

#ifdef MYMALLOC
#  define Perl_safesysmalloc_size(where)	Perl_malloced_size(where)
#else
#  ifdef HAS_MALLOC_SIZE
#    ifdef PERL_TRACK_MEMPOOL
#	define Perl_safesysmalloc_size(where)			\
	    (malloc_size(((char *)(where)) - sTHX) - sTHX)
#    else
#	define Perl_safesysmalloc_size(where) malloc_size(where)
#    endif
#  endif
#  ifdef HAS_MALLOC_GOOD_SIZE
#    ifdef PERL_TRACK_MEMPOOL
#	define Perl_malloc_good_size(how_much)			\
	    (malloc_good_size((how_much) + sTHX) - sTHX)
#    else
#	define Perl_malloc_good_size(how_much) malloc_good_size(how_much)
#    endif
#  else
/* Having this as the identity operation makes some code simpler.  */
#	define Perl_malloc_good_size(how_much)	(how_much)
#  endif
#endif

typedef int (*runops_proc_t)(pTHX);
typedef void (*share_proc_t) (pTHX_ SV *sv);
typedef int  (*thrhook_proc_t) (pTHX);
typedef OP* (*PPADDR_t[]) (pTHX);
typedef bool (*destroyable_proc_t) (pTHX_ SV *sv);
typedef void (*despatch_signals_proc_t) (pTHX);

/* NeXT has problems with crt0.o globals */
#if defined(__DYNAMIC__) && \
    (defined(NeXT) || defined(__NeXT__) || defined(PERL_DARWIN))
#  if defined(NeXT) || defined(__NeXT)
#    include <mach-o/dyld.h>
#    define environ (*environ_pointer)
EXT char *** environ_pointer;
#  else
#    if defined(PERL_DARWIN) && defined(PERL_CORE)
#      include <crt_externs.h>	/* for the env array */
#      define environ (*_NSGetEnviron())
#    endif
#  endif
#else
   /* VMS and some other platforms don't use the environ array */
#  ifdef USE_ENVIRON_ARRAY
#    if !defined(DONT_DECLARE_STD) || \
        (defined(__svr4__) && defined(__GNUC__) && defined(sun)) || \
        defined(__sgi) || \
        defined(__DGUX)
extern char **	environ;	/* environment variables supplied via exec */
#    endif
#  endif
#endif

#define PERL_PATCHLEVEL_H_IMPLICIT
#include "patchlevel.h"
#undef PERL_PATCHLEVEL_H_IMPLICIT

#define PERL_VERSION_STRING	STRINGIFY(PERL_REVISION) "." \
				STRINGIFY(PERL_VERSION) "." \
				STRINGIFY(PERL_SUBVERSION)

#define PERL_API_VERSION_STRING	STRINGIFY(PERL_API_REVISION) "." \
				STRINGIFY(PERL_API_VERSION) "." \
				STRINGIFY(PERL_API_SUBVERSION)

START_EXTERN_C

/* handy constants */
EXTCONST char PL_warn_uninit[]
  INIT("Use of uninitialized value%s%s%s");
EXTCONST char PL_warn_uninit_sv[]
  INIT("Use of uninitialized value%"SVf"%s%s");
EXTCONST char PL_warn_nosemi[]
  INIT("Semicolon seems to be missing");
EXTCONST char PL_warn_reserved[]
  INIT("Unquoted string \"%s\" may clash with future reserved word");
EXTCONST char PL_warn_nl[]
  INIT("Unsuccessful %s on filename containing newline");
EXTCONST char PL_no_wrongref[]
  INIT("Can't use %s ref as %s ref");
/* The core no longer needs these here. If you require the string constant,
   please inline a copy into your own code.  */
EXTCONST char PL_no_symref[] __attribute__deprecated__
  INIT("Can't use string (\"%.32s\") as %s ref while \"strict refs\" in use");
EXTCONST char PL_no_symref_sv[] __attribute__deprecated__
  INIT("Can't use string (\"%" SVf32 "\") as %s ref while \"strict refs\" in use");
EXTCONST char PL_no_usym[]
  INIT("Can't use an undefined value as %s reference");
EXTCONST char PL_no_aelem[]
  INIT("Modification of non-creatable array value attempted, subscript %d");
EXTCONST char PL_no_helem_sv[]
  INIT("Modification of non-creatable hash value attempted, subscript \"%"SVf"\"");
EXTCONST char PL_no_modify[]
  INIT("Modification of a read-only value attempted");
EXTCONST char PL_no_mem[]
  INIT("Out of memory!\n");
EXTCONST char PL_no_security[]
  INIT("Insecure dependency in %s%s");
EXTCONST char PL_no_sock_func[]
  INIT("Unsupported socket function \"%s\" called");
EXTCONST char PL_no_dir_func[]
  INIT("Unsupported directory function \"%s\" called");
EXTCONST char PL_no_func[]
  INIT("The %s function is unimplemented");
EXTCONST char PL_no_myglob[]
  INIT("\"%s\" variable %s can't be in a package");
EXTCONST char PL_no_localize_ref[]
  INIT("Can't localize through a reference");
EXTCONST char PL_memory_wrap[]
  INIT("panic: memory wrap");

EXTCONST char PL_Yes[]
  INIT("1");
EXTCONST char PL_No[]
  INIT("");
EXTCONST char PL_hexdigit[]
  INIT("0123456789abcdef0123456789ABCDEF");

/* This is constant on most architectures, a global on OS/2 */
#ifndef OS2
EXTCONST char PL_sh_path[]
  INIT(SH_PATH); /* full path of shell */
#endif

#ifdef CSH
EXTCONST char PL_cshname[]
  INIT(CSH);
#  define PL_cshlen	(sizeof(CSH "") - 1)
#endif

/* These are baked at compile time into any shared perl library.
   In future releases this will allow us in main() to sanity test the
   library we're linking against.  */

EXTCONST U8 PL_revision
  INIT(PERL_REVISION);
EXTCONST U8 PL_version
  INIT(PERL_VERSION);
EXTCONST U8 PL_subversion
  INIT(PERL_SUBVERSION);

EXTCONST char PL_uuemap[65]
  INIT("`!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_");

#ifdef DOINIT
EXTCONST char PL_uudmap[256] =
#  ifdef PERL_MICRO
#    include "uuudmap.h"
#  else
#    include "uudmap.h"
#  endif
;
EXTCONST char PL_bitcount[256] =
#  ifdef PERL_MICRO
#    include "ubitcount.h"
#else
#    include "bitcount.h"
#  endif
;
EXTCONST char* const PL_sig_name[] = { SIG_NAME };
EXTCONST int         PL_sig_num[]  = { SIG_NUM };
#else
EXTCONST char PL_uudmap[256];
EXTCONST char PL_bitcount[256];
EXTCONST char* const PL_sig_name[];
EXTCONST int         PL_sig_num[];
#endif

/* fast conversion and case folding tables.  The folding tables complement the
 * fold, so that 'a' maps to 'A' and 'A' maps to 'a', ignoring more complicated
 * folds such as outside the range or to multiple characters. */

#ifdef DOINIT
#ifndef EBCDIC

/* The EBCDIC fold table depends on the code page, and hence is found in
 * utfebcdic.h */

EXTCONST  unsigned char PL_fold[] = {
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,
	40,	41,	42,	43,	44,	45,	46,	47,
	48,	49,	50,	51,	52,	53,	54,	55,
	56,	57,	58,	59,	60,	61,	62,	63,
	64,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
	'x',	'y',	'z',	91,	92,	93,	94,	95,
	96,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
	'X',	'Y',	'Z',	123,	124,	125,	126,	127,
	128,	129,	130,	131,	132,	133,	134,	135,
	136,	137,	138,	139,	140,	141,	142,	143,
	144,	145,	146,	147,	148,	149,	150,	151,
	152,	153,	154,	155,	156,	157,	158,	159,
	160,	161,	162,	163,	164,	165,	166,	167,
	168,	169,	170,	171,	172,	173,	174,	175,
	176,	177,	178,	179,	180,	181,	182,	183,
	184,	185,	186,	187,	188,	189,	190,	191,
	192,	193,	194,	195,	196,	197,	198,	199,
	200,	201,	202,	203,	204,	205,	206,	207,
	208,	209,	210,	211,	212,	213,	214,	215,
	216,	217,	218,	219,	220,	221,	222,	223,
	224,	225,	226,	227,	228,	229,	230,	231,
	232,	233,	234,	235,	236,	237,	238,	239,
	240,	241,	242,	243,	244,	245,	246,	247,
	248,	249,	250,	251,	252,	253,	254,	255
};
EXTCONST  unsigned char PL_fold_latin1[] = {
    /* Full latin1 complement folding, except for three problematic code points:
     *	Micro sign (181 = 0xB5) and y with diearesis (255 = 0xFF) have their
     *	fold complements outside the Latin1 range, so can't match something
     *	that isn't in utf8.
     *	German lower case sharp s (223 = 0xDF) folds to two characters, 'ss',
     *	not one, so can't be represented in this table.
     *
     * All have to be specially handled */
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,
	40,	41,	42,	43,	44,	45,	46,	47,
	48,	49,	50,	51,	52,	53,	54,	55,
	56,	57,	58,	59,	60,	61,	62,	63,
	64,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
	'x',	'y',	'z',	91,	92,	93,	94,	95,
	96,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
	'X',	'Y',	'Z',	123,	124,	125,	126,	127,
	128,	129,	130,	131,	132,	133,	134,	135,
	136,	137,	138,	139,	140,	141,	142,	143,
	144,	145,	146,	147,	148,	149,	150,	151,
	152,	153,	154,	155,	156,	157,	158,	159,
	160,	161,	162,	163,	164,	165,	166,	167,
	168,	169,	170,	171,	172,	173,	174,	175,
	176,	177,	178,	179,	180,	181 /*micro */,	182,	183,
	184,	185,	186,	187,	188,	189,	190,	191,
	192+32,	193+32,	194+32,	195+32,	196+32,	197+32,	198+32,	199+32,
	200+32,	201+32,	202+32,	203+32,	204+32,	205+32,	206+32,	207+32,
	208+32,	209+32,	210+32,	211+32,	212+32,	213+32,	214+32,	215,
	216+32,	217+32,	218+32,	219+32,	220+32,	221+32,	222+32,	223 /* ss */,
	224-32,	225-32,	226-32,	227-32,	228-32,	229-32,	230-32,	231-32,
	232-32,	233-32,	234-32,	235-32,	236-32,	237-32,	238-32,	239-32,
	240-32,	241-32,	242-32,	243-32,	244-32,	245-32,	246-32,	247,
	248-32,	249-32,	250-32,	251-32,	252-32,	253-32,	254-32,
	255 /* y with diaeresis */
};
#endif  /* !EBCDIC, but still in DOINIT */

/* If these tables are accessed through ebcdic, the access will be converted to
 * latin1 first */
EXTCONST  unsigned char PL_latin1_lc[] = {  /* lowercasing */
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,
	40,	41,	42,	43,	44,	45,	46,	47,
	48,	49,	50,	51,	52,	53,	54,	55,
	56,	57,	58,	59,	60,	61,	62,	63,
	64,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
	'x',	'y',	'z',	91,	92,	93,	94,	95,
	96,	97,	98,	99,	100,	101,	102,	103,
	104,	105,	106,	107,	108,	109,	110,	111,
	112,	113,	114,	115,	116,	117,	118,	119,
	120,	121,	122,	123,	124,	125,	126,	127,
	128,	129,	130,	131,	132,	133,	134,	135,
	136,	137,	138,	139,	140,	141,	142,	143,
	144,	145,	146,	147,	148,	149,	150,	151,
	152,	153,	154,	155,	156,	157,	158,	159,
	160,	161,	162,	163,	164,	165,	166,	167,
	168,	169,	170,	171,	172,	173,	174,	175,
	176,	177,	178,	179,	180,	181,	182,	183,
	184,	185,	186,	187,	188,	189,	190,	191,
	192+32,	193+32,	194+32,	195+32,	196+32,	197+32,	198+32,	199+32,
	200+32,	201+32,	202+32,	203+32,	204+32,	205+32,	206+32,	207+32,
	208+32,	209+32,	210+32,	211+32,	212+32,	213+32,	214+32,	215,
	216+32,	217+32,	218+32,	219+32,	220+32,	221+32,	222+32,	223,
	224,	225,	226,	227,	228,	229,	230,	231,
	232,	233,	234,	235,	236,	237,	238,	239,
	240,	241,	242,	243,	244,	245,	246,	247,
	248,	249,	250,	251,	252,	253,	254,	255
};

/* upper and title case of latin1 characters, modified so that the three tricky
 * ones are mapped to 255 (which is one of the three) */
EXTCONST  unsigned char PL_mod_latin1_uc[] = {
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,
	40,	41,	42,	43,	44,	45,	46,	47,
	48,	49,	50,	51,	52,	53,	54,	55,
	56,	57,	58,	59,	60,	61,	62,	63,
	64,	65,	66,	67,	68,	69,	70,	71,
	72,	73,	74,	75,	76,	77,	78,	79,
	80,	81,	82,	83,	84,	85,	86,	87,
	88,	89,	90,	91,	92,	93,	94,	95,
	96,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
	'X',	'Y',	'Z',	123,	124,	125,	126,	127,
	128,	129,	130,	131,	132,	133,	134,	135,
	136,	137,	138,	139,	140,	141,	142,	143,
	144,	145,	146,	147,	148,	149,	150,	151,
	152,	153,	154,	155,	156,	157,	158,	159,
	160,	161,	162,	163,	164,	165,	166,	167,
	168,	169,	170,	171,	172,	173,	174,	175,
	176,	177,	178,	179,	180,	255 /*micro*/,	182,	183,
	184,	185,	186,	187,	188,	189,	190,	191,
	192,	193,	194,	195,	196,	197,	198,	199,
	200,	201,	202,	203,	204,	205,	206,	207,
	208,	209,	210,	211,	212,	213,	214,	215,
	216,	217,	218,	219,	220,	221,	222,	255 /*sharp s*/,
	224-32,	225-32,	226-32,	227-32,	228-32,	229-32,	230-32,	231-32,
	232-32,	233-32,	234-32,	235-32,	236-32,	237-32,	238-32,	239-32,
	240-32,	241-32,	242-32,	243-32,	244-32,	245-32,	246-32,	247,
	248-32,	249-32,	250-32,	251-32,	252-32,	253-32,	254-32,	255
};
#else	/* ! DOINIT */
EXTCONST unsigned char PL_fold[];
EXTCONST unsigned char PL_fold_latin1[];
EXTCONST unsigned char PL_mod_latin1_uc[];
EXTCONST unsigned char PL_latin1_lc[];
#endif

#ifndef PERL_GLOBAL_STRUCT /* or perlvars.h */
#ifdef DOINIT
EXT unsigned char PL_fold_locale[] = { /* Unfortunately not EXTCONST. */
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,
	40,	41,	42,	43,	44,	45,	46,	47,
	48,	49,	50,	51,	52,	53,	54,	55,
	56,	57,	58,	59,	60,	61,	62,	63,
	64,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
	'x',	'y',	'z',	91,	92,	93,	94,	95,
	96,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
	'X',	'Y',	'Z',	123,	124,	125,	126,	127,
	128,	129,	130,	131,	132,	133,	134,	135,
	136,	137,	138,	139,	140,	141,	142,	143,
	144,	145,	146,	147,	148,	149,	150,	151,
	152,	153,	154,	155,	156,	157,	158,	159,
	160,	161,	162,	163,	164,	165,	166,	167,
	168,	169,	170,	171,	172,	173,	174,	175,
	176,	177,	178,	179,	180,	181,	182,	183,
	184,	185,	186,	187,	188,	189,	190,	191,
	192,	193,	194,	195,	196,	197,	198,	199,
	200,	201,	202,	203,	204,	205,	206,	207,
	208,	209,	210,	211,	212,	213,	214,	215,
	216,	217,	218,	219,	220,	221,	222,	223,
	224,	225,	226,	227,	228,	229,	230,	231,
	232,	233,	234,	235,	236,	237,	238,	239,
	240,	241,	242,	243,	244,	245,	246,	247,
	248,	249,	250,	251,	252,	253,	254,	255
};
#else
EXT unsigned char PL_fold_locale[]; /* Unfortunately not EXTCONST. */
#endif
#endif /* !PERL_GLOBAL_STRUCT */

#ifdef DOINIT
#ifdef EBCDIC
EXTCONST unsigned char PL_freq[] = {/* EBCDIC frequencies for mixed English/C */
    1,      2,      84,     151,    154,    155,    156,    157,
    165,    246,    250,    3,      158,    7,      18,     29,
    40,     51,     62,     73,     85,     96,     107,    118,
    129,    140,    147,    148,    149,    150,    152,    153,
    255,      6,      8,      9,     10,     11,     12,     13,
     14,     15,     24,     25,     26,     27,     28,    226,
     29,     30,     31,     32,     33,     43,     44,     45,
     46,     47,     48,     49,     50,     76,     77,     78,
     79,     80,     81,     82,     83,     84,     85,     86,
     87,     94,     95,    234,    181,    233,    187,    190,
    180,     96,     97,     98,     99,    100,    101,    102,
    104,    112,    182,    174,    236,    232,    229,    103,
    228,    226,    114,    115,    116,    117,    118,    119,
    120,    121,    122,    235,    176,    230,    194,    162,
    130,    131,    132,    133,    134,    135,    136,    137,
    138,    139,    201,    205,    163,    217,    220,    224,
    5,      248,    227,    244,    242,    255,    241,    231,
    240,    253,    16,     197,    19,     20,     21,     187,
    23,     169,    210,    245,    237,    249,    247,    239,
    168,    252,    34,     196,    36,     37,     38,     39,
    41,     42,     251,    254,    238,    223,    221,    213,
    225,    177,    52,     53,     54,     55,     56,     57,
    58,     59,     60,     61,     63,     64,     65,     66,
    67,     68,     69,     70,     71,     72,     74,     75,
    205,    208,    186,    202,    200,    218,    198,    179,
    178,    214,    88,     89,     90,     91,     92,     93,
    217,    166,    170,    207,    199,    209,    206,    204,
    160,    212,    105,    106,    108,    109,    110,    111,
    203,    113,    216,    215,    192,    175,    193,    243,
    172,    161,    123,    124,    125,    126,    127,    128,
    222,    219,    211,    195,    188,    193,    185,    184,
    191,    183,    141,    142,    143,    144,    145,    146
};
#else  /* ascii rather than ebcdic */
EXTCONST unsigned char PL_freq[] = {	/* letter frequencies for mixed English/C */
	1,	2,	84,	151,	154,	155,	156,	157,
	165,	246,	250,	3,	158,	7,	18,	29,
	40,	51,	62,	73,	85,	96,	107,	118,
	129,	140,	147,	148,	149,	150,	152,	153,
	255,	182,	224,	205,	174,	176,	180,	217,
	233,	232,	236,	187,	235,	228,	234,	226,
	222,	219,	211,	195,	188,	193,	185,	184,
	191,	183,	201,	229,	181,	220,	194,	162,
	163,	208,	186,	202,	200,	218,	198,	179,
	178,	214,	166,	170,	207,	199,	209,	206,
	204,	160,	212,	216,	215,	192,	175,	173,
	243,	172,	161,	190,	203,	189,	164,	230,
	167,	248,	227,	244,	242,	255,	241,	231,
	240,	253,	169,	210,	245,	237,	249,	247,
	239,	168,	252,	251,	254,	238,	223,	221,
	213,	225,	177,	197,	171,	196,	159,	4,
	5,	6,	8,	9,	10,	11,	12,	13,
	14,	15,	16,	17,	19,	20,	21,	22,
	23,	24,	25,	26,	27,	28,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,
	41,	42,	43,	44,	45,	46,	47,	48,
	49,	50,	52,	53,	54,	55,	56,	57,
	58,	59,	60,	61,	63,	64,	65,	66,
	67,	68,	69,	70,	71,	72,	74,	75,
	76,	77,	78,	79,	80,	81,	82,	83,
	86,	87,	88,	89,	90,	91,	92,	93,
	94,	95,	97,	98,	99,	100,	101,	102,
	103,	104,	105,	106,	108,	109,	110,	111,
	112,	113,	114,	115,	116,	117,	119,	120,
	121,	122,	123,	124,	125,	126,	127,	128,
	130,	131,	132,	133,	134,	135,	136,	137,
	138,	139,	141,	142,	143,	144,	145,	146
};
#endif
#else
EXTCONST unsigned char PL_freq[];
#endif

#ifdef DEBUGGING
#ifdef DOINIT
EXTCONST char* const PL_block_type[] = {
	"NULL",
	"WHEN",
	"BLOCK",
	"GIVEN",
	"LOOP_FOR",
	"LOOP_PLAIN",
	"LOOP_LAZYSV",
	"LOOP_LAZYIV",
	"SUB",
	"FORMAT",
	"EVAL",
	"SUBST"
};
#else
EXTCONST char* PL_block_type[];
#endif
#endif

/* These are all the compile time options that affect binary compatibility.
   Other compile time options that are binary compatible are in perl.c
   Both are combined for the output of perl -V
   However, this string will be embedded in any shared perl library, which will
   allow us add a comparison check in perlmain.c in the near future.  */
#ifdef DOINIT
EXTCONST char PL_bincompat_options[] =
#  ifdef DEBUG_LEAKING_SCALARS
			     " DEBUG_LEAKING_SCALARS"
#  endif
#  ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
			     " DEBUG_LEAKING_SCALARS_FORK_DUMP"
#  endif
#  ifdef FAKE_THREADS
			     " FAKE_THREADS"
#  endif
#  ifdef FCRYPT
			     " FCRYPT"
#  endif
#  ifdef HAS_TIMES
			     " HAS_TIMES"
#  endif
#  ifdef HAVE_INTERP_INTERN
			     " HAVE_INTERP_INTERN"
#  endif
#  ifdef MULTIPLICITY
			     " MULTIPLICITY"
#  endif
#  ifdef MYMALLOC
			     " MYMALLOC"
#  endif
#  ifdef PERLIO_LAYERS
			     " PERLIO_LAYERS"
#  endif
#  ifdef PERL_DEBUG_READONLY_OPS
			     " PERL_DEBUG_READONLY_OPS"
#  endif
#  ifdef PERL_GLOBAL_STRUCT
			     " PERL_GLOBAL_STRUCT"
#  endif
#  ifdef PERL_IMPLICIT_CONTEXT
			     " PERL_IMPLICIT_CONTEXT"
#  endif
#  ifdef PERL_IMPLICIT_SYS
			     " PERL_IMPLICIT_SYS"
#  endif
#  ifdef PERL_MAD
			     " PERL_MAD"
#  endif
#  ifdef PERL_MICRO
			     " PERL_MICRO"
#  endif
#  ifdef PERL_NEED_APPCTX
			     " PERL_NEED_APPCTX"
#  endif
#  ifdef PERL_NEED_TIMESBASE
			     " PERL_NEED_TIMESBASE"
#  endif
#  ifdef PERL_OLD_COPY_ON_WRITE
			     " PERL_OLD_COPY_ON_WRITE"
#  endif
#  ifdef PERL_POISON
			     " PERL_POISON"
#  endif
#  ifdef PERL_TRACK_MEMPOOL
			     " PERL_TRACK_MEMPOOL"
#  endif
#  ifdef PERL_USES_PL_PIDSTATUS
			     " PERL_USES_PL_PIDSTATUS"
#  endif
#  ifdef PL_OP_SLAB_ALLOC
			     " PL_OP_SLAB_ALLOC"
#  endif
#  ifdef USE_64_BIT_ALL
			     " USE_64_BIT_ALL"
#  endif
#  ifdef USE_64_BIT_INT
			     " USE_64_BIT_INT"
#  endif
#  ifdef USE_IEEE
			     " USE_IEEE"
#  endif
#  ifdef USE_ITHREADS
			     " USE_ITHREADS"
#  endif
#  ifdef USE_LARGE_FILES
			     " USE_LARGE_FILES"
#  endif
#  ifdef USE_LOCALE_COLLATE
			     " USE_LOCALE_COLLATE"
#  endif
#  ifdef USE_LOCALE_NUMERIC
			     " USE_LOCALE_NUMERIC"
#  endif
#  ifdef USE_LONG_DOUBLE
			     " USE_LONG_DOUBLE"
#  endif
#  ifdef USE_PERLIO
			     " USE_PERLIO"
#  endif
#  ifdef USE_REENTRANT_API
			     " USE_REENTRANT_API"
#  endif
#  ifdef USE_SFIO
			     " USE_SFIO"
#  endif
#  ifdef USE_SOCKS
			     " USE_SOCKS"
#  endif
#  ifdef VMS_DO_SOCKETS
			     " VMS_DO_SOCKETS"
#  endif
#  ifdef VMS_SHORTEN_LONG_SYMBOLS
			     " VMS_SHORTEN_LONG_SYMBOLS"
#  endif
#  ifdef VMS_WE_ARE_CASE_SENSITIVE
			     " VMS_SYMBOL_CASE_AS_IS"
#  endif
  "";
#else
EXTCONST char PL_bincompat_options[];
#endif

#ifndef PERL_SET_PHASE
#  define PERL_SET_PHASE(new_phase) \
    PHASE_CHANGE_PROBE(PL_phase_names[new_phase], PL_phase_names[PL_phase]); \
    PL_phase = new_phase;
#endif

/* The interpreter phases. If these ever change, PL_phase_names right below will
 * need to be updated accordingly. */
enum perl_phase {
    PERL_PHASE_CONSTRUCT	= 0,
    PERL_PHASE_START		= 1,
    PERL_PHASE_CHECK		= 2,
    PERL_PHASE_INIT		= 3,
    PERL_PHASE_RUN		= 4,
    PERL_PHASE_END		= 5,
    PERL_PHASE_DESTRUCT		= 6
};

#ifdef DOINIT
EXTCONST char *const PL_phase_names[] = {
    "CONSTRUCT",
    "START",
    "CHECK",
    "INIT",
    "RUN",
    "END",
    "DESTRUCT"
};
#else
EXTCONST char *const PL_phase_names[];
#endif

#ifndef PERL_CORE
/* Do not use this macro. It only exists for extensions that rely on PL_dirty
 * instead of using the newer PL_phase, which provides everything PL_dirty
 * provided, and more. */
#  define PL_dirty (PL_phase == PERL_PHASE_DESTRUCT)
#endif /* !PERL_CORE */

END_EXTERN_C

/*****************************************************************************/
/* This lexer/parser stuff is currently global since yacc is hard to reenter */
/*****************************************************************************/
/* XXX This needs to be revisited, since BEGIN makes yacc re-enter... */

#ifdef __Lynx__
/* LynxOS defines these in scsi.h which is included via ioctl.h */
#ifdef FORMAT
#undef FORMAT
#endif
#ifdef SPACE
#undef SPACE
#endif
#endif

#define LEX_NOTPARSING		11	/* borrowed from toke.c */

typedef enum {
    XOPERATOR,
    XTERM,
    XREF,
    XSTATE,
    XBLOCK,
    XATTRBLOCK,
    XATTRTERM,
    XTERMBLOCK,
    XTERMORDORDOR /* evil hack */
    /* update exp_name[] in toke.c if adding to this enum */
} expectation;

/* Hints are now stored in a dedicated U32, so the bottom 8 bits are no longer
   special and there is no need for HINT_PRIVATE_MASK for COPs
   However, bitops store HINT_INTEGER in their op_private.  */
#define HINT_INTEGER		0x00000001 /* integer pragma */
#define HINT_STRICT_REFS	0x00000002 /* strict pragma */
#define HINT_LOCALE		0x00000004 /* locale pragma */
#define HINT_BYTES		0x00000008 /* bytes pragma */
#define HINT_LOCALE_NOT_CHARS	0x00000010 /* locale ':not_characters' pragma */

#define HINT_EXPLICIT_STRICT_REFS	0x00000020 /* strict.pm */
#define HINT_EXPLICIT_STRICT_SUBS	0x00000040 /* strict.pm */
#define HINT_EXPLICIT_STRICT_VARS	0x00000080 /* strict.pm */

#define HINT_BLOCK_SCOPE	0x00000100
#define HINT_STRICT_SUBS	0x00000200 /* strict pragma */
#define HINT_STRICT_VARS	0x00000400 /* strict pragma */
#define HINT_UNI_8_BIT		0x00000800 /* unicode_strings feature */

/* The HINT_NEW_* constants are used by the overload pragma */
#define HINT_NEW_INTEGER	0x00001000
#define HINT_NEW_FLOAT		0x00002000
#define HINT_NEW_BINARY		0x00004000
#define HINT_NEW_STRING		0x00008000
#define HINT_NEW_RE		0x00010000
#define HINT_LOCALIZE_HH	0x00020000 /* %^H needs to be copied */
#define HINT_LEXICAL_IO_IN	0x00040000 /* ${^OPEN} is set for input */
#define HINT_LEXICAL_IO_OUT	0x00080000 /* ${^OPEN} is set for output */

#define HINT_RE_TAINT		0x00100000 /* re pragma */
#define HINT_RE_EVAL		0x00200000 /* re pragma */

#define HINT_FILETEST_ACCESS	0x00400000 /* filetest pragma */
#define HINT_UTF8		0x00800000 /* utf8 pragma */

#define HINT_NO_AMAGIC		0x01000000 /* overloading pragma */

#define HINT_RE_FLAGS		0x02000000 /* re '/xism' pragma */

#define HINT_FEATURE_MASK	0x1c000000 /* 3 bits for feature bundles */

				/* Note: Used for NATIVE_HINTS, currently
				   defined by vms/vmsish.h:
				0x40000000
				0x80000000
				 */

/* The following are stored in $^H{sort}, not in PL_hints */
#define HINT_SORT_SORT_BITS	0x000000FF /* allow 256 different ones */
#define HINT_SORT_QUICKSORT	0x00000001
#define HINT_SORT_MERGESORT	0x00000002
#define HINT_SORT_STABLE	0x00000100 /* sort styles (currently one) */

/* Various states of the input record separator SV (rs) */
#define RsSNARF(sv)   (! SvOK(sv))
#define RsSIMPLE(sv)  (SvOK(sv) && (! SvPOK(sv) || SvCUR(sv)))
#define RsPARA(sv)    (SvPOK(sv) && ! SvCUR(sv))
#define RsRECORD(sv)  (SvROK(sv) && (SvIV(SvRV(sv)) > 0))

/* A struct for keeping various DEBUGGING related stuff,
 * neatly packed.  Currently only scratch variables for
 * constructing debug output are included.  Needed always,
 * not just when DEBUGGING, though, because of the re extension. c*/
struct perl_debug_pad {
  SV pad[3];
};

#define PERL_DEBUG_PAD(i)	&(PL_debug_pad.pad[i])
#define PERL_DEBUG_PAD_ZERO(i)	(SvPVX(PERL_DEBUG_PAD(i))[0] = 0, \
	(((XPV*) SvANY(PERL_DEBUG_PAD(i)))->xpv_cur = 0), \
	PERL_DEBUG_PAD(i))

/* Enable variables which are pointers to functions */
typedef void (*peep_t)(pTHX_ OP* o);
typedef regexp* (*regcomp_t) (pTHX_ char* exp, char* xend, PMOP* pm);
typedef I32     (*regexec_t) (pTHX_ regexp* prog, char* stringarg,
				      char* strend, char* strbeg, I32 minend,
				      SV* screamer, void* data, U32 flags);
typedef char*   (*re_intuit_start_t) (pTHX_ regexp *prog, SV *sv,
						char *strpos, char *strend,
						U32 flags,
						re_scream_pos_data *d);
typedef SV*	(*re_intuit_string_t) (pTHX_ regexp *prog);
typedef void	(*regfree_t) (pTHX_ struct regexp* r);
typedef regexp* (*regdupe_t) (pTHX_ const regexp* r, CLONE_PARAMS *param);
typedef I32     (*re_fold_t)(const char *, char const *, I32);

typedef void (*DESTRUCTORFUNC_NOCONTEXT_t) (void*);
typedef void (*DESTRUCTORFUNC_t) (pTHX_ void*);
typedef void (*SVFUNC_t) (pTHX_ SV* const);
typedef I32  (*SVCOMPARE_t) (pTHX_ SV* const, SV* const);
typedef void (*XSINIT_t) (pTHX);
typedef void (*ATEXIT_t) (pTHX_ void*);
typedef void (*XSUBADDR_t) (pTHX_ CV *);

typedef OP* (*Perl_ppaddr_t)(pTHX);
typedef OP* (*Perl_check_t) (pTHX_ OP*);
typedef void(*Perl_ophook_t)(pTHX_ OP*);
typedef int (*Perl_keyword_plugin_t)(pTHX_ char*, STRLEN, OP**);
typedef void(*Perl_cpeep_t)(pTHX_ OP *, OP *);

typedef void(*globhook_t)(pTHX);

#define KEYWORD_PLUGIN_DECLINE 0
#define KEYWORD_PLUGIN_STMT    1
#define KEYWORD_PLUGIN_EXPR    2

/* Interpreter exitlist entry */
typedef struct exitlistentry {
    void (*fn) (pTHX_ void*);
    void *ptr;
} PerlExitListEntry;

/* if you only have signal() and it resets on each signal, FAKE_PERSISTENT_SIGNAL_HANDLERS fixes */
/* These have to be before perlvars.h */
#if !defined(HAS_SIGACTION) && defined(VMS)
#  define  FAKE_PERSISTENT_SIGNAL_HANDLERS
#endif
/* if we're doing kill() with sys$sigprc on VMS, FAKE_DEFAULT_SIGNAL_HANDLERS */
#if defined(KILL_BY_SIGPRC)
#  define  FAKE_DEFAULT_SIGNAL_HANDLERS
#endif

#if !defined(MULTIPLICITY)

struct interpreter {
    char broiled;
};

#else

/* If we have multiple interpreters define a struct
   holding variables which must be per-interpreter
   If we don't have threads anything that would have
   be per-thread is per-interpreter.
*/

/* Set up PERLVAR macros for populating structs */
#  define PERLVAR(prefix,var,type) type prefix##var;
#  define PERLVARA(prefix,var,n,type) type prefix##var[n];
#  define PERLVARI(prefix,var,type,init) type prefix##var;
#  define PERLVARIC(prefix,var,type,init) type prefix##var;

struct interpreter {
#  include "intrpvar.h"
};

EXTCONST U16 PL_interp_size
  INIT(sizeof(struct interpreter));

#  define PERL_INTERPRETER_SIZE_UPTO_MEMBER(member)			\
    STRUCT_OFFSET(struct interpreter, member) +				\
    sizeof(((struct interpreter*)0)->member)

/* This will be useful for subsequent releases, because this has to be the
   same in your libperl as in main(), else you have a mismatch and must abort.
*/
EXTCONST U16 PL_interp_size_5_16_0
  INIT(PERL_INTERPRETER_SIZE_UPTO_MEMBER(PERL_LAST_5_16_0_INTERP_MEMBER));


#  ifdef PERL_GLOBAL_STRUCT
/* MULTIPLICITY is automatically defined when PERL_GLOBAL_STRUCT is defined,
   hence it's safe and sane to nest this within #ifdef MULTIPLICITY  */

struct perl_vars {
#    include "perlvars.h"
};

EXTCONST U16 PL_global_struct_size
  INIT(sizeof(struct perl_vars));

#    ifdef PERL_CORE
#      ifndef PERL_GLOBAL_STRUCT_PRIVATE
EXT struct perl_vars PL_Vars;
EXT struct perl_vars *PL_VarsPtr INIT(&PL_Vars);
#        undef PERL_GET_VARS
#        define PERL_GET_VARS() PL_VarsPtr
#      endif /* !PERL_GLOBAL_STRUCT_PRIVATE */
#    else /* PERL_CORE */
#      if !defined(__GNUC__) || !defined(WIN32)
EXT
#      endif /* WIN32 */
struct perl_vars *PL_VarsPtr;
#      define PL_Vars (*((PL_VarsPtr) \
		       ? PL_VarsPtr : (PL_VarsPtr = Perl_GetVars(aTHX))))
#    endif /* PERL_CORE */
#  endif /* PERL_GLOBAL_STRUCT */

/* Done with PERLVAR macros for now ... */
#  undef PERLVAR
#  undef PERLVARA
#  undef PERLVARI
#  undef PERLVARIC

#endif /* MULTIPLICITY */

struct tempsym; /* defined in pp_pack.c */

#include "thread.h"
#include "pp.h"

#ifndef PERL_CALLCONV
#  ifdef __cplusplus
#    define PERL_CALLCONV extern "C"
#  else
#    define PERL_CALLCONV
#  endif
#endif
#undef PERL_CKDEF
#undef PERL_PPDEF
#define PERL_CKDEF(s)	PERL_CALLCONV OP *s (pTHX_ OP *o);
#define PERL_PPDEF(s)	PERL_CALLCONV OP *s (pTHX);

#ifdef MYMALLOC
#  include "malloc_ctl.h"
#endif

#include "proto.h"

/* this has structure inits, so it cannot be included before here */
#include "opcode.h"

/* The following must follow proto.h as #defines mess up syntax */

#if !defined(PERL_FOR_X2P)
#  include "embedvar.h"
#endif
#ifndef PERL_MAD
#  undef PL_madskills
#  undef PL_xmlfp
#  define PL_madskills 0
#  define PL_xmlfp 0
#endif

/* Now include all the 'global' variables
 * If we don't have threads or multiple interpreters
 * these include variables that would have been their struct-s
 */

#define PERLVAR(prefix,var,type) EXT type PL_##var;
#define PERLVARA(prefix,var,n,type) EXT type PL_##var[n];
#define PERLVARI(prefix,var,type,init) EXT type  PL_##var INIT(init);
#define PERLVARIC(prefix,var,type,init) EXTCONST type PL_##var INIT(init);

#if !defined(MULTIPLICITY)
START_EXTERN_C
#  include "intrpvar.h"
END_EXTERN_C
#endif

#ifdef PERL_CORE
/* All core uses now exterminated. Ensure no zombies can return:  */
#  undef PL_na
#endif

/* Now all the config stuff is setup we can include embed.h
   In particular, need the relevant *ish file included already, as it may
   define HAVE_INTERP_INTERN  */
#include "embed.h"
#ifndef PERL_MAD
#  undef op_getmad
#  define op_getmad(arg,pegop,slot) NOOP
#endif

#ifndef PERL_GLOBAL_STRUCT
START_EXTERN_C

#  include "perlvars.h"

END_EXTERN_C
#endif

#undef PERLVAR
#undef PERLVARA
#undef PERLVARI
#undef PERLVARIC

START_EXTERN_C

/* dummy variables that hold pointers to both runops functions, thus forcing
 * them *both* to get linked in (useful for Peek.xs, debugging etc) */

EXTCONST runops_proc_t PL_runops_std
  INIT(Perl_runops_standard);
EXTCONST runops_proc_t PL_runops_dbg
  INIT(Perl_runops_debug);

/* PERL_GLOBAL_STRUCT_PRIVATE wants to keep global data like the
 * magic vtables const, but this is incompatible with SWIG which
 * does want to modify the vtables. */
#ifdef PERL_GLOBAL_STRUCT_PRIVATE
#  define EXT_MGVTBL EXTCONST MGVTBL
#else
#  define EXT_MGVTBL EXT MGVTBL
#endif

#define PERL_MAGIC_READONLY_ACCEPTABLE 0x40
#define PERL_MAGIC_VALUE_MAGIC 0x80
#define PERL_MAGIC_VTABLE_MASK 0x3F
#define PERL_MAGIC_TYPE_READONLY_ACCEPTABLE(t) \
    (PL_magic_data[(U8)(t)] & PERL_MAGIC_READONLY_ACCEPTABLE)
#define PERL_MAGIC_TYPE_IS_VALUE_MAGIC(t) \
    (PL_magic_data[(U8)(t)] & PERL_MAGIC_VALUE_MAGIC)

#include "mg_vtable.h"

#ifdef DOINIT
EXTCONST U8 PL_magic_data[256] =
#  ifdef PERL_MICRO
#    include "umg_data.h"
#  else
#    include "mg_data.h"
#  endif
;
#else
EXTCONST U8 PL_magic_data[256];
#endif

#ifdef DOINIT
		        /* NL BD IV NV PV PI PN MG RX GV LV AV HV CV FM IO */
EXTCONST bool
PL_valid_types_IVX[]    = { 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0 };
EXTCONST bool
PL_valid_types_NVX[]    = { 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0 };
EXTCONST bool
PL_valid_types_PVX[]    = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1 };
EXTCONST bool
PL_valid_types_RV[]     = { 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1 };
EXTCONST bool
PL_valid_types_IV_set[] = { 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1 };
EXTCONST bool
PL_valid_types_NV_set[] = { 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };

#else

EXTCONST bool PL_valid_types_IVX[];
EXTCONST bool PL_valid_types_NVX[];
EXTCONST bool PL_valid_types_PVX[];
EXTCONST bool PL_valid_types_RV[];
EXTCONST bool PL_valid_types_IV_set[];
EXTCONST bool PL_valid_types_NV_set[];

#endif


#include "overload.h"

END_EXTERN_C

struct am_table {
  U8 flags;
  U8 fallback;
  U16 spare;
  U32 was_ok_sub;
  long was_ok_am;
  CV* table[NofAMmeth];
};
struct am_table_short {
  U8 flags;
  U8 fallback;
  U16 spare;
  U32 was_ok_sub;
  long was_ok_am;
};
typedef struct am_table AMT;
typedef struct am_table_short AMTS;

#define AMGfallNEVER	1
#define AMGfallNO	2
#define AMGfallYES	3

#define AMTf_AMAGIC		1
#define AMTf_OVERLOADED		2
#define AMT_AMAGIC(amt)		((amt)->flags & AMTf_AMAGIC)
#define AMT_AMAGIC_on(amt)	((amt)->flags |= AMTf_AMAGIC)
#define AMT_AMAGIC_off(amt)	((amt)->flags &= ~AMTf_AMAGIC)
#define AMT_OVERLOADED(amt)	((amt)->flags & AMTf_OVERLOADED)
#define AMT_OVERLOADED_on(amt)	((amt)->flags |= AMTf_OVERLOADED)
#define AMT_OVERLOADED_off(amt)	((amt)->flags &= ~AMTf_OVERLOADED)

#define StashHANDLER(stash,meth)	gv_handler((stash),CAT2(meth,_amg))

/*
 * some compilers like to redefine cos et alia as faster
 * (and less accurate?) versions called F_cos et cetera (Quidquid
 * latine dictum sit, altum viditur.)  This trick collides with
 * the Perl overloading (amg).  The following #defines fool both.
 */

#ifdef _FASTMATH
#   ifdef atan2
#       define F_atan2_amg  atan2_amg
#   endif
#   ifdef cos
#       define F_cos_amg    cos_amg
#   endif
#   ifdef exp
#       define F_exp_amg    exp_amg
#   endif
#   ifdef log
#       define F_log_amg    log_amg
#   endif
#   ifdef pow
#       define F_pow_amg    pow_amg
#   endif
#   ifdef sin
#       define F_sin_amg    sin_amg
#   endif
#   ifdef sqrt
#       define F_sqrt_amg   sqrt_amg
#   endif
#endif /* _FASTMATH */

#define PERLDB_ALL		(PERLDBf_SUB	| PERLDBf_LINE	|	\
				 PERLDBf_NOOPT	| PERLDBf_INTER	|	\
				 PERLDBf_SUBLINE| PERLDBf_SINGLE|	\
				 PERLDBf_NAMEEVAL| PERLDBf_NAMEANON |   \
				 PERLDBf_SAVESRC)
					/* No _NONAME, _GOTO */
#define PERLDBf_SUB		0x01	/* Debug sub enter/exit */
#define PERLDBf_LINE		0x02	/* Keep line # */
#define PERLDBf_NOOPT		0x04	/* Switch off optimizations */
#define PERLDBf_INTER		0x08	/* Preserve more data for
					   later inspections  */
#define PERLDBf_SUBLINE		0x10	/* Keep subr source lines */
#define PERLDBf_SINGLE		0x20	/* Start with single-step on */
#define PERLDBf_NONAME		0x40	/* For _SUB: no name of the subr */
#define PERLDBf_GOTO		0x80	/* Report goto: call DB::goto */
#define PERLDBf_NAMEEVAL	0x100	/* Informative names for evals */
#define PERLDBf_NAMEANON	0x200	/* Informative names for anon subs */
#define PERLDBf_SAVESRC  	0x400	/* Save source lines into @{"_<$filename"} */
#define PERLDBf_SAVESRC_NOSUBS	0x800	/* Including evals that generate no subroutines */
#define PERLDBf_SAVESRC_INVALID	0x1000	/* Save source that did not compile */

#define PERLDB_SUB	(PL_perldb && (PL_perldb & PERLDBf_SUB))
#define PERLDB_LINE	(PL_perldb && (PL_perldb & PERLDBf_LINE))
#define PERLDB_NOOPT	(PL_perldb && (PL_perldb & PERLDBf_NOOPT))
#define PERLDB_INTER	(PL_perldb && (PL_perldb & PERLDBf_INTER))
#define PERLDB_SUBLINE	(PL_perldb && (PL_perldb & PERLDBf_SUBLINE))
#define PERLDB_SINGLE	(PL_perldb && (PL_perldb & PERLDBf_SINGLE))
#define PERLDB_SUB_NN	(PL_perldb && (PL_perldb & (PERLDBf_NONAME)))
#define PERLDB_GOTO	(PL_perldb && (PL_perldb & PERLDBf_GOTO))
#define PERLDB_NAMEEVAL	(PL_perldb && (PL_perldb & PERLDBf_NAMEEVAL))
#define PERLDB_NAMEANON	(PL_perldb && (PL_perldb & PERLDBf_NAMEANON))
#define PERLDB_SAVESRC 	(PL_perldb && (PL_perldb & PERLDBf_SAVESRC))
#define PERLDB_SAVESRC_NOSUBS	(PL_perldb && (PL_perldb & PERLDBf_SAVESRC_NOSUBS))
#define PERLDB_SAVESRC_INVALID	(PL_perldb && (PL_perldb & PERLDBf_SAVESRC_INVALID))

/* #ifdef USE_LOCALE_NUMERIC */

#define PL_numeric_local TRUE
#define PL_numeric_standard TRUE

#define SET_NUMERIC_STANDARD() \
	set_numeric_standard();

#define SET_NUMERIC_LOCAL() \
	set_numeric_local();

#define STORE_NUMERIC_STANDARD_SET_LOCAL() \
	bool was_standard = PL_numeric_standard && IN_LOCALE; \
	if (was_standard) SET_NUMERIC_LOCAL();

/* Returns non-zero If the plain locale pragma without a parameter is in effect
 */
#define IN_LOCALE_RUNTIME	(CopHINTS_get(PL_curcop) & HINT_LOCALE)

/* Returns non-zero If either form of the locale pragma is in effect */
#define IN_SOME_LOCALE_FORM_RUNTIME   \
		(CopHINTS_get(PL_curcop) & (HINT_LOCALE|HINT_LOCALE_NOT_CHARS))

#define IN_LOCALE_COMPILETIME	(PL_hints & HINT_LOCALE)
#define IN_SOME_LOCALE_FORM_COMPILETIME \
			    (PL_hints & (HINT_LOCALE|HINT_LOCALE_NOT_CHARS))

#define IN_LOCALE \
	(IN_PERL_COMPILETIME ? IN_LOCALE_COMPILETIME : IN_LOCALE_RUNTIME)
#define IN_SOME_LOCALE_FORM \
	(IN_PERL_COMPILETIME ? IN_SOME_LOCALE_FORM_COMPILETIME \
	                     : IN_SOME_LOCALE_FORM_RUNTIME)

#define STORE_NUMERIC_LOCAL_SET_STANDARD() \
	bool was_local = PL_numeric_local && IN_LOCALE; \
	if (was_local) SET_NUMERIC_STANDARD();

#define STORE_NUMERIC_STANDARD_SET_LOCAL() \
	bool was_standard = PL_numeric_standard && IN_LOCALE; \
	if (was_standard) SET_NUMERIC_LOCAL();

#define RESTORE_NUMERIC_LOCAL() \
	if (was_local) SET_NUMERIC_LOCAL();

#define RESTORE_NUMERIC_STANDARD() \
	if (was_standard) SET_NUMERIC_STANDARD();

#define Atof				my_atof

/* #else */

#ifdef USE_LOCALE_NUMERIC

#define SET_NUMERIC_STANDARD()  	/**/
#define SET_NUMERIC_LOCAL()     	/**/
#define IS_NUMERIC_RADIX(a, b)		(0)
#define STORE_NUMERIC_LOCAL_SET_STANDARD()	/**/
#define STORE_NUMERIC_STANDARD_SET_LOCAL()	/**/
#define RESTORE_NUMERIC_LOCAL()		/**/
#define RESTORE_NUMERIC_STANDARD()	/**/
#define Atof				my_atof
#define IN_LOCALE_RUNTIME		0

#endif /* !USE_LOCALE_NUMERIC */

#if !defined(Strtol) && defined(USE_64_BIT_INT) && defined(IV_IS_QUAD) && QUADKIND == QUAD_IS_LONG_LONG
#    ifdef __hpux
#        define strtoll __strtoll	/* secret handshake */
#    endif
#    ifdef WIN64
#        define strtoll _strtoi64	/* secret handshake */
#    endif
#   if !defined(Strtol) && defined(HAS_STRTOLL)
#       define Strtol	strtoll
#   endif
#    if !defined(Strtol) && defined(HAS_STRTOQ)
#       define Strtol	strtoq
#    endif
/* is there atoq() anywhere? */
#endif
#if !defined(Strtol) && defined(HAS_STRTOL)
#   define Strtol	strtol
#endif
#ifndef Atol
/* It would be more fashionable to use Strtol() to define atol()
 * (as is done for Atoul(), see below) but for backward compatibility
 * we just assume atol(). */
#   if defined(USE_64_BIT_INT) && defined(IV_IS_QUAD) && QUADKIND == QUAD_IS_LONG_LONG && defined(HAS_ATOLL)
#    ifdef WIN64
#       define atoll    _atoi64		/* secret handshake */
#    endif
#       define Atol	atoll
#   else
#       define Atol	atol
#   endif
#endif

#if !defined(Strtoul) && defined(USE_64_BIT_INT) && defined(UV_IS_QUAD) && QUADKIND == QUAD_IS_LONG_LONG
#    ifdef __hpux
#        define strtoull __strtoull	/* secret handshake */
#    endif
#    ifdef WIN64
#        define strtoull _strtoui64	/* secret handshake */
#    endif
#    if !defined(Strtoul) && defined(HAS_STRTOULL)
#       define Strtoul	strtoull
#    endif
#    if !defined(Strtoul) && defined(HAS_STRTOUQ)
#       define Strtoul	strtouq
#    endif
/* is there atouq() anywhere? */
#endif
#if !defined(Strtoul) && defined(HAS_STRTOUL)
#   define Strtoul	strtoul
#endif
#if !defined(Strtoul) && defined(HAS_STRTOL) /* Last resort. */
#   define Strtoul(s, e, b)	strchr((s), '-') ? ULONG_MAX : (unsigned long)strtol((s), (e), (b))
#endif
#ifndef Atoul
#   define Atoul(s)	Strtoul(s, NULL, 10)
#endif


/* if these never got defined, they need defaults */
#ifndef PERL_SET_CONTEXT
#  define PERL_SET_CONTEXT(i)		PERL_SET_INTERP(i)
#endif

#ifndef PERL_GET_CONTEXT
#  define PERL_GET_CONTEXT		PERL_GET_INTERP
#endif

#ifndef PERL_GET_THX
#  define PERL_GET_THX			((void*)NULL)
#endif

#ifndef PERL_SET_THX
#  define PERL_SET_THX(t)		NOOP
#endif

#ifndef PERL_SCRIPT_MODE
#define PERL_SCRIPT_MODE "r"
#endif

/*
 * Some operating systems are stingy with stack allocation,
 * so perl may have to guard against stack overflow.
 */
#ifndef PERL_STACK_OVERFLOW_CHECK
#define PERL_STACK_OVERFLOW_CHECK()  NOOP
#endif

/*
 * Some nonpreemptive operating systems find it convenient to
 * check for asynchronous conditions after each op execution.
 * Keep this check simple, or it may slow down execution
 * massively.
 */

#ifndef PERL_MICRO
#	ifndef PERL_ASYNC_CHECK
#		define PERL_ASYNC_CHECK() if (PL_sig_pending) PL_signalhook(aTHX)
#	endif
#endif

#ifndef PERL_ASYNC_CHECK
#   define PERL_ASYNC_CHECK()  NOOP
#endif

/*
 * On some operating systems, a memory allocation may succeed,
 * but put the process too close to the system's comfort limit.
 * In this case, PERL_ALLOC_CHECK frees the pointer and sets
 * it to NULL.
 */
#ifndef PERL_ALLOC_CHECK
#define PERL_ALLOC_CHECK(p)  NOOP
#endif

#ifdef HAS_SEM
#   include <sys/ipc.h>
#   include <sys/sem.h>
#   ifndef HAS_UNION_SEMUN	/* Provide the union semun. */
    union semun {
	int		val;
	struct semid_ds	*buf;
	unsigned short	*array;
    };
#   endif
#   ifdef USE_SEMCTL_SEMUN
#	ifdef IRIX32_SEMUN_BROKEN_BY_GCC
            union gccbug_semun {
		int             val;
		struct semid_ds *buf;
		unsigned short  *array;
		char            __dummy[5];
	    };
#           define semun gccbug_semun
#	endif
#       define Semctl(id, num, cmd, semun) semctl(id, num, cmd, semun)
#   else
#       ifdef USE_SEMCTL_SEMID_DS
#           ifdef EXTRA_F_IN_SEMUN_BUF
#               define Semctl(id, num, cmd, semun) semctl(id, num, cmd, semun.buff)
#           else
#               define Semctl(id, num, cmd, semun) semctl(id, num, cmd, semun.buf)
#           endif
#       endif
#   endif
#endif

/*
 * Boilerplate macros for initializing and accessing interpreter-local
 * data from C.  All statics in extensions should be reworked to use
 * this, if you want to make the extension thread-safe.  See
 * ext/XS/APItest/APItest.xs for an example of the use of these macros,
 * and perlxs.pod for more.
 *
 * Code that uses these macros is responsible for the following:
 * 1. #define MY_CXT_KEY to a unique string, e.g.
 *    "DynaLoader::_guts" XS_VERSION
 *    XXX in the current implementation, this string is ignored.
 * 2. Declare a typedef named my_cxt_t that is a structure that contains
 *    all the data that needs to be interpreter-local.
 * 3. Use the START_MY_CXT macro after the declaration of my_cxt_t.
 * 4. Use the MY_CXT_INIT macro such that it is called exactly once
 *    (typically put in the BOOT: section).
 * 5. Use the members of the my_cxt_t structure everywhere as
 *    MY_CXT.member.
 * 6. Use the dMY_CXT macro (a declaration) in all the functions that
 *    access MY_CXT.
 */

#if defined(PERL_IMPLICIT_CONTEXT)

/* START_MY_CXT must appear in all extensions that define a my_cxt_t structure,
 * right after the definition (i.e. at file scope).  The non-threads
 * case below uses it to declare the data as static. */
#  ifdef PERL_GLOBAL_STRUCT_PRIVATE
#    define START_MY_CXT
#    define MY_CXT_INDEX Perl_my_cxt_index(aTHX_ MY_CXT_KEY)
#    define MY_CXT_INIT_ARG MY_CXT_KEY
#  else
#    define START_MY_CXT static int my_cxt_index = -1;
#    define MY_CXT_INDEX my_cxt_index
#    define MY_CXT_INIT_ARG &my_cxt_index
#  endif /* #ifdef PERL_GLOBAL_STRUCT_PRIVATE */

/* Creates and zeroes the per-interpreter data.
 * (We allocate my_cxtp in a Perl SV so that it will be released when
 * the interpreter goes away.) */
#  define MY_CXT_INIT \
	my_cxt_t *my_cxtp = \
	    (my_cxt_t*)Perl_my_cxt_init(aTHX_ MY_CXT_INIT_ARG, sizeof(my_cxt_t)); \
	PERL_UNUSED_VAR(my_cxtp)
#  define MY_CXT_INIT_INTERP(my_perl) \
	my_cxt_t *my_cxtp = \
	    (my_cxt_t*)Perl_my_cxt_init(my_perl, MY_CXT_INIT_ARG, sizeof(my_cxt_t)); \
	PERL_UNUSED_VAR(my_cxtp)

/* This declaration should be used within all functions that use the
 * interpreter-local data. */
#  define dMY_CXT	\
	my_cxt_t *my_cxtp = (my_cxt_t *)PL_my_cxt_list[MY_CXT_INDEX]
#  define dMY_CXT_INTERP(my_perl)	\
	my_cxt_t *my_cxtp = (my_cxt_t *)(my_perl)->Imy_cxt_list[MY_CXT_INDEX]

/* Clones the per-interpreter data. */
#  define MY_CXT_CLONE \
	my_cxt_t *my_cxtp = (my_cxt_t*)SvPVX(newSV(sizeof(my_cxt_t)-1));\
	Copy(PL_my_cxt_list[MY_CXT_INDEX], my_cxtp, 1, my_cxt_t);\
	PL_my_cxt_list[MY_CXT_INDEX] = my_cxtp				\


/* This macro must be used to access members of the my_cxt_t structure.
 * e.g. MY_CXT.some_data */
#  define MY_CXT		(*my_cxtp)

/* Judicious use of these macros can reduce the number of times dMY_CXT
 * is used.  Use is similar to pTHX, aTHX etc. */
#  define pMY_CXT	my_cxt_t *my_cxtp
#  define pMY_CXT_	pMY_CXT,
#  define _pMY_CXT	,pMY_CXT
#  define aMY_CXT	my_cxtp
#  define aMY_CXT_	aMY_CXT,
#  define _aMY_CXT	,aMY_CXT

#else /* PERL_IMPLICIT_CONTEXT */

#  define START_MY_CXT		static my_cxt_t my_cxt;
#  define dMY_CXT_SV	    	dNOOP
#  define dMY_CXT		dNOOP
#  define dMY_CXT_INTERP(my_perl) dNOOP
#  define MY_CXT_INIT		NOOP
#  define MY_CXT_CLONE		NOOP
#  define MY_CXT		my_cxt

#  define pMY_CXT		void
#  define pMY_CXT_
#  define _pMY_CXT
#  define aMY_CXT
#  define aMY_CXT_
#  define _aMY_CXT

#endif /* !defined(PERL_IMPLICIT_CONTEXT) */

#ifdef I_FCNTL
#  include <fcntl.h>
#endif

#ifdef __Lynx__
#  include <fcntl.h>
#endif

#ifdef I_SYS_FILE
#  include <sys/file.h>
#endif

#if defined(HAS_FLOCK) && !defined(HAS_FLOCK_PROTO)
int flock(int fd, int op);
#endif

#ifndef O_RDONLY
/* Assume UNIX defaults */
#    define O_RDONLY	0000
#    define O_WRONLY	0001
#    define O_RDWR	0002
#    define O_CREAT	0100
#endif

#ifndef O_BINARY
#  define O_BINARY 0
#endif

#ifndef O_TEXT
#  define O_TEXT 0
#endif

#if O_TEXT != O_BINARY
    /* If you have different O_TEXT and O_BINARY and you are a CLRF shop,
     * that is, you are somehow DOSish. */
#   if defined(__BEOS__) || defined(__HAIKU__) || defined(__VOS__) || \
	defined(__CYGWIN__)
    /* BeOS/Haiku has O_TEXT != O_BINARY but O_TEXT and O_BINARY have no effect;
     * BeOS/Haiku is always UNIXoid (LF), not DOSish (CRLF). */
    /* VOS has O_TEXT != O_BINARY, and they have effect,
     * but VOS always uses LF, never CRLF. */
    /* If you have O_TEXT different from your O_BINARY but you still are
     * not a CRLF shop. */
#       undef PERLIO_USING_CRLF
#   else
    /* If you really are DOSish. */
#      define PERLIO_USING_CRLF 1
#   endif
#endif

#ifdef I_LIBUTIL
#   include <libutil.h>		/* setproctitle() in some FreeBSDs */
#endif

#ifndef EXEC_ARGV_CAST
#define EXEC_ARGV_CAST(x) (char **)x
#endif

#define IS_NUMBER_IN_UV		      0x01 /* number within UV range (maybe not
					      int).  value returned in pointed-
					      to UV */
#define IS_NUMBER_GREATER_THAN_UV_MAX 0x02 /* pointed to UV undefined */
#define IS_NUMBER_NOT_INT	      0x04 /* saw . or E notation */
#define IS_NUMBER_NEG		      0x08 /* leading minus sign */
#define IS_NUMBER_INFINITY	      0x10 /* this is big */
#define IS_NUMBER_NAN                 0x20 /* this is not */

#define GROK_NUMERIC_RADIX(sp, send) grok_numeric_radix(sp, send)

/* Input flags: */
#define PERL_SCAN_ALLOW_UNDERSCORES   0x01 /* grok_??? accept _ in numbers */
#define PERL_SCAN_DISALLOW_PREFIX     0x02 /* grok_??? reject 0x in hex etc */
#define PERL_SCAN_SILENT_ILLDIGIT     0x04 /* grok_??? not warn about illegal digits */
#define PERL_SCAN_SILENT_NON_PORTABLE 0x08 /* grok_??? not warn about very large
					      numbers which are <= UV_MAX */
/* Output flags: */
#define PERL_SCAN_GREATER_THAN_UV_MAX 0x02 /* should this merge with above? */

/* to let user control profiling */
#ifdef PERL_GPROF_CONTROL
extern void moncontrol(int);
#define PERL_GPROF_MONCONTROL(x) moncontrol(x)
#else
#define PERL_GPROF_MONCONTROL(x)
#endif

#ifdef UNDER_CE
#include "wince.h"
#endif

/* ISO 6429 NEL - C1 control NExt Line */
/* See http://www.unicode.org/unicode/reports/tr13/ */
#ifdef EBCDIC	/* In EBCDIC NEL is just an alias for LF */
#   if '^' == 95	/* CP 1047: MVS OpenEdition - OS/390 - z/OS */
#       define NEXT_LINE_CHAR	0x15
#   else		/* CDRA */
#       define NEXT_LINE_CHAR	0x25
#   endif
#else
#   define NEXT_LINE_CHAR	0x85
#endif

/* The UTF-8 bytes of the Unicode LS and PS, U+2028 and U+2029 */
#define UNICODE_LINE_SEPA_0	0xE2
#define UNICODE_LINE_SEPA_1	0x80
#define UNICODE_LINE_SEPA_2	0xA8
#define UNICODE_PARA_SEPA_0	0xE2
#define UNICODE_PARA_SEPA_1	0x80
#define UNICODE_PARA_SEPA_2	0xA9

#ifndef PIPESOCK_MODE
#  define PIPESOCK_MODE
#endif

#ifndef SOCKET_OPEN_MODE
#  define SOCKET_OPEN_MODE	PIPESOCK_MODE
#endif

#ifndef PIPE_OPEN_MODE
#  define PIPE_OPEN_MODE	PIPESOCK_MODE
#endif

#define PERL_MAGIC_UTF8_CACHESIZE	2

#define PERL_UNICODE_STDIN_FLAG			0x0001
#define PERL_UNICODE_STDOUT_FLAG		0x0002
#define PERL_UNICODE_STDERR_FLAG		0x0004
#define PERL_UNICODE_IN_FLAG			0x0008
#define PERL_UNICODE_OUT_FLAG			0x0010
#define PERL_UNICODE_ARGV_FLAG			0x0020
#define PERL_UNICODE_LOCALE_FLAG		0x0040
#define PERL_UNICODE_WIDESYSCALLS_FLAG		0x0080 /* for Sarathy */
#define PERL_UNICODE_UTF8CACHEASSERT_FLAG	0x0100

#define PERL_UNICODE_STD_FLAG		\
	(PERL_UNICODE_STDIN_FLAG	| \
	 PERL_UNICODE_STDOUT_FLAG	| \
	 PERL_UNICODE_STDERR_FLAG)

#define PERL_UNICODE_INOUT_FLAG		\
	(PERL_UNICODE_IN_FLAG	| \
	 PERL_UNICODE_OUT_FLAG)

#define PERL_UNICODE_DEFAULT_FLAGS	\
	(PERL_UNICODE_STD_FLAG		| \
	 PERL_UNICODE_INOUT_FLAG	| \
	 PERL_UNICODE_LOCALE_FLAG)

#define PERL_UNICODE_ALL_FLAGS			0x01ff

#define PERL_UNICODE_STDIN			'I'
#define PERL_UNICODE_STDOUT			'O'
#define PERL_UNICODE_STDERR			'E'
#define PERL_UNICODE_STD			'S'
#define PERL_UNICODE_IN				'i'
#define PERL_UNICODE_OUT			'o'
#define PERL_UNICODE_INOUT			'D'
#define PERL_UNICODE_ARGV			'A'
#define PERL_UNICODE_LOCALE			'L'
#define PERL_UNICODE_WIDESYSCALLS		'W'
#define PERL_UNICODE_UTF8CACHEASSERT		'a'

#define PERL_SIGNALS_UNSAFE_FLAG	0x0001

/* Use instead of abs() since abs() forces its argument to be an int,
 * but also beware since this evaluates its argument twice, so no x++. */
#define PERL_ABS(x) ((x) < 0 ? -(x) : (x))

#if defined(__DECC) && defined(__osf__)
#pragma message disable (mainparm) /* Perl uses the envp in main(). */
#endif

#define do_open(g, n, l, a, rm, rp, sf) \
	do_openn(g, n, l, a, rm, rp, sf, (SV **) NULL, 0)
#ifdef PERL_DEFAULT_DO_EXEC3_IMPLEMENTATION
#  define do_exec(cmd)			do_exec3(cmd,0,0)
#endif
#ifdef OS2
#  define do_aexec			Perl_do_aexec
#else
#  define do_aexec(really, mark,sp)	do_aexec5(really, mark, sp, 0, 0)
#endif

#if defined(OEMVS)
#define NO_ENV_ARRAY_IN_MAIN
#endif

/* These are used by Perl_pv_escape() and Perl_pv_pretty()
 * are here so that they are available throughout the core
 * NOTE that even though some are for _escape and some for _pretty
 * there must not be any clashes as the flags from _pretty are
 * passed straight through to _escape.
 */

#define PERL_PV_ESCAPE_QUOTE        0x0001
#define PERL_PV_PRETTY_QUOTE        PERL_PV_ESCAPE_QUOTE

#define PERL_PV_PRETTY_ELLIPSES     0x0002
#define PERL_PV_PRETTY_LTGT         0x0004

#define PERL_PV_ESCAPE_FIRSTCHAR    0x0008

#define PERL_PV_ESCAPE_UNI          0x0100
#define PERL_PV_ESCAPE_UNI_DETECT   0x0200
#define PERL_PV_ESCAPE_NONASCII     0x0400

#define PERL_PV_ESCAPE_ALL	    0x1000
#define PERL_PV_ESCAPE_NOBACKSLASH  0x2000
#define PERL_PV_ESCAPE_NOCLEAR      0x4000
#define PERL_PV_ESCAPE_RE           0x8000

#define PERL_PV_PRETTY_NOCLEAR      PERL_PV_ESCAPE_NOCLEAR

/* used by pv_display in dump.c*/
#define PERL_PV_PRETTY_DUMP  PERL_PV_PRETTY_ELLIPSES|PERL_PV_PRETTY_QUOTE
#define PERL_PV_PRETTY_REGPROP PERL_PV_PRETTY_ELLIPSES|PERL_PV_PRETTY_LTGT|PERL_PV_ESCAPE_RE|PERL_PV_ESCAPE_NONASCII

/*

   (KEEP THIS LAST IN perl.h!)

   Mention

   NV_PRESERVES_UV

   HAS_MKSTEMP
   HAS_MKSTEMPS
   HAS_MKDTEMP

   HAS_GETCWD

   HAS_MMAP
   HAS_MPROTECT
   HAS_MSYNC
   HAS_MADVISE
   HAS_MUNMAP
   I_SYSMMAN
   Mmap_t

   NVef
   NVff
   NVgf

   HAS_UALARM
   HAS_USLEEP

   HAS_SETITIMER
   HAS_GETITIMER

   HAS_SENDMSG
   HAS_RECVMSG
   HAS_READV
   HAS_WRITEV
   I_SYSUIO
   HAS_STRUCT_MSGHDR
   HAS_STRUCT_CMSGHDR

   HAS_NL_LANGINFO

   HAS_DIRFD

   so that Configure picks them up.

   (KEEP THIS LAST IN perl.h!)

*/

#endif /* Include guard */

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
