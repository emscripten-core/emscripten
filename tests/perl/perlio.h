/*    perlio.h
 *
 *    Copyright (C) 1996, 1997, 1999, 2000, 2001, 2002, 2003,
 *    2004, 2005, 2006, 2007, by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef _PERLIO_H
#define _PERLIO_H
/*
  Interface for perl to IO functions.
  There is a hierarchy of Configure determined #define controls:
   USE_STDIO   - forces PerlIO_xxx() to be #define-d onto stdio functions.
                 This is used for x2p subdirectory and for conservative
                 builds - "just like perl5.00X used to be".
                 This dominates over the others.

   USE_PERLIO  - The primary Configure variable that enables PerlIO.
                 If USE_PERLIO is _NOT_ set
                   then USE_STDIO above will be set to be conservative.
                 If USE_PERLIO is set
                   then there are two modes determined by USE_SFIO:

   USE_SFIO    - If set causes PerlIO_xxx() to be #define-d onto sfio functions.
                 A backward compatibility mode for some specialist applications.

                 If USE_SFIO is not set then PerlIO_xxx() are real functions
                 defined in perlio.c which implement extra functionality
                 required for utf8 support.

   One further note - the table-of-functions scheme controlled
   by PERL_IMPLICIT_SYS turns on USE_PERLIO so that iperlsys.h can
   #define PerlIO_xxx() to go via the function table, without having
   to #undef them from (say) stdio forms.

*/

#if defined(PERL_IMPLICIT_SYS)
#ifndef USE_PERLIO
#ifndef NETWARE
/* # define USE_PERLIO */
#endif
#endif
#endif

#ifndef USE_PERLIO
# define USE_STDIO
#endif

#ifdef USE_STDIO
#  ifndef PERLIO_IS_STDIO
#      define PERLIO_IS_STDIO
#  endif
#endif

/* --------------------  End of Configure controls ---------------------------- */

/*
 * Although we may not want stdio to be used including <stdio.h> here
 * avoids issues where stdio.h has strange side effects
 */
#include <stdio.h>

#ifdef __BEOS__
int fseeko(FILE *stream, off_t offset, int whence);
off_t ftello(FILE *stream);
#endif

#if defined(USE_64_BIT_STDIO) && defined(HAS_FTELLO) && !defined(USE_FTELL64)
#define ftell ftello
#endif

#if defined(USE_64_BIT_STDIO) && defined(HAS_FSEEKO) && !defined(USE_FSEEK64)
#define fseek fseeko
#endif

/* BS2000 includes are sometimes a bit non standard :-( */
#if defined(POSIX_BC) && defined(O_BINARY) && !defined(O_TEXT)
#undef O_BINARY
#endif

#ifdef PERLIO_IS_STDIO
/* #define PerlIO_xxxx() as equivalent stdio function */
#include "perlsdio.h"
#else				/* PERLIO_IS_STDIO */
#ifdef USE_SFIO
/* #define PerlIO_xxxx() as equivalent sfio function */
#include "perlsfio.h"
#endif				/* USE_SFIO */
#endif				/* PERLIO_IS_STDIO */

#ifndef PerlIO
/* ----------- PerlIO implementation ---------- */
/* PerlIO not #define-d to something else - define the implementation */

typedef struct _PerlIO PerlIOl;
typedef struct _PerlIO_funcs PerlIO_funcs;
typedef PerlIOl *PerlIO;
#define PerlIO PerlIO
#define PERLIO_LAYERS 1

/* Making the big PerlIO_funcs vtables const is good (enables placing
 * them in the const section which is good for speed, security, and
 * embeddability) but this cannot be done by default because of
 * backward compatibility. */
#ifdef PERLIO_FUNCS_CONST
#define PERLIO_FUNCS_DECL(funcs) const PerlIO_funcs funcs
#define PERLIO_FUNCS_CAST(funcs) (PerlIO_funcs*)(funcs)
#else
#define PERLIO_FUNCS_DECL(funcs) PerlIO_funcs funcs
#define PERLIO_FUNCS_CAST(funcs) (funcs)
#endif

PERL_EXPORT_C void PerlIO_define_layer(pTHX_ PerlIO_funcs *tab);
PERL_EXPORT_C PerlIO_funcs *PerlIO_find_layer(pTHX_ const char *name,
                                              STRLEN len,
				              int load);
PERL_EXPORT_C PerlIO *PerlIO_push(pTHX_ PerlIO *f, PERLIO_FUNCS_DECL(*tab),
			          const char *mode, SV *arg);
PERL_EXPORT_C void PerlIO_pop(pTHX_ PerlIO *f);
PERL_EXPORT_C AV* PerlIO_get_layers(pTHX_ PerlIO *f);
PERL_EXPORT_C void PerlIO_clone(pTHX_ PerlInterpreter *proto,
                                CLONE_PARAMS *param);

#endif				/* PerlIO */

/* ----------- End of implementation choices  ---------- */

#ifndef PERLIO_IS_STDIO
/* Not using stdio _directly_ as PerlIO */

/* We now need to determine  what happens if source trys to use stdio.
 * There are three cases based on PERLIO_NOT_STDIO which XS code
 * can set how it wants.
 */

#ifdef PERL_CORE
/* Make a choice for perl core code
   - currently this is set to try and catch lingering raw stdio calls.
     This is a known issue with some non UNIX ports which still use
     "native" stdio features.
*/
#ifndef PERLIO_NOT_STDIO
#define PERLIO_NOT_STDIO 1
#endif
#else
#ifndef PERLIO_NOT_STDIO
#define PERLIO_NOT_STDIO 0
#endif
#endif

#ifdef PERLIO_NOT_STDIO
#if PERLIO_NOT_STDIO
/*
 * PERLIO_NOT_STDIO #define'd as 1
 * Case 1: Strong denial of stdio - make all stdio calls (we can think of) errors
 */
#include "nostdio.h"
#else				/* if PERLIO_NOT_STDIO */
/*
 * PERLIO_NOT_STDIO #define'd as 0
 * Case 2: Declares that both PerlIO and stdio can be used
 */
#endif				/* if PERLIO_NOT_STDIO */
#else				/* ifdef PERLIO_NOT_STDIO */
/*
 * PERLIO_NOT_STDIO not defined
 * Case 3: Try and fake stdio calls as PerlIO calls
 */
#include "fakesdio.h"
#endif				/* ifndef PERLIO_NOT_STDIO */
#endif				/* PERLIO_IS_STDIO */

/* ----------- fill in things that have not got #define'd  ---------- */

#ifndef Fpos_t
#define Fpos_t Off_t
#endif

#ifndef EOF
#define EOF (-1)
#endif

/* This is to catch case with no stdio */
#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

/* The default buffer size for the perlio buffering layer */
#ifndef PERLIOBUF_DEFAULT_BUFSIZ
#define PERLIOBUF_DEFAULT_BUFSIZ (BUFSIZ > 8192 ? BUFSIZ : 8192)
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif

#define PERLIO_DUP_CLONE	1
#define PERLIO_DUP_FD		2

/* --------------------- Now prototypes for functions --------------- */

START_EXTERN_C
#ifndef __attribute__format__
#  ifdef HASATTRIBUTE_FORMAT
#    define __attribute__format__(x,y,z) __attribute__((format(x,y,z)))
#  else
#    define __attribute__format__(x,y,z)
#  endif
#endif
#ifndef PerlIO_init
PERL_EXPORT_C void PerlIO_init(pTHX);
#endif
#ifndef PerlIO_stdoutf
PERL_EXPORT_C int PerlIO_stdoutf(const char *, ...)
    __attribute__format__(__printf__, 1, 2);
#endif
#ifndef PerlIO_puts
PERL_EXPORT_C int PerlIO_puts(PerlIO *, const char *);
#endif
#ifndef PerlIO_open
PERL_EXPORT_C PerlIO *PerlIO_open(const char *, const char *);
#endif
#ifndef PerlIO_openn
PERL_EXPORT_C PerlIO *PerlIO_openn(pTHX_ const char *layers, const char *mode,
				   int fd, int imode, int perm, PerlIO *old,
				   int narg, SV **arg);
#endif
#ifndef PerlIO_eof
PERL_EXPORT_C int PerlIO_eof(PerlIO *);
#endif
#ifndef PerlIO_error
PERL_EXPORT_C int PerlIO_error(PerlIO *);
#endif
#ifndef PerlIO_clearerr
PERL_EXPORT_C void PerlIO_clearerr(PerlIO *);
#endif
#ifndef PerlIO_getc
PERL_EXPORT_C int PerlIO_getc(PerlIO *);
#endif
#ifndef PerlIO_putc
PERL_EXPORT_C int PerlIO_putc(PerlIO *, int);
#endif
#ifndef PerlIO_ungetc
PERL_EXPORT_C int PerlIO_ungetc(PerlIO *, int);
#endif
#ifndef PerlIO_fdopen
PERL_EXPORT_C PerlIO *PerlIO_fdopen(int, const char *);
#endif
#ifndef PerlIO_importFILE
PERL_EXPORT_C PerlIO *PerlIO_importFILE(FILE *, const char *);
#endif
#ifndef PerlIO_exportFILE
PERL_EXPORT_C FILE *PerlIO_exportFILE(PerlIO *, const char *);
#endif
#ifndef PerlIO_findFILE
PERL_EXPORT_C FILE *PerlIO_findFILE(PerlIO *);
#endif
#ifndef PerlIO_releaseFILE
PERL_EXPORT_C void PerlIO_releaseFILE(PerlIO *, FILE *);
#endif
#ifndef PerlIO_read
PERL_EXPORT_C SSize_t PerlIO_read(PerlIO *, void *, Size_t);
#endif
#ifndef PerlIO_unread
PERL_EXPORT_C SSize_t PerlIO_unread(PerlIO *, const void *, Size_t);
#endif
#ifndef PerlIO_write
PERL_EXPORT_C SSize_t PerlIO_write(PerlIO *, const void *, Size_t);
#endif
#ifndef PerlIO_setlinebuf
PERL_EXPORT_C void PerlIO_setlinebuf(PerlIO *);
#endif
#ifndef PerlIO_printf
PERL_EXPORT_C int PerlIO_printf(PerlIO *, const char *, ...)
    __attribute__format__(__printf__, 2, 3);
#endif
#ifndef PerlIO_sprintf
PERL_EXPORT_C int PerlIO_sprintf(char *, int, const char *, ...)
    __attribute__format__(__printf__, 3, 4);
#endif
#ifndef PerlIO_vprintf
PERL_EXPORT_C int PerlIO_vprintf(PerlIO *, const char *, va_list);
#endif
#ifndef PerlIO_tell
PERL_EXPORT_C Off_t PerlIO_tell(PerlIO *);
#endif
#ifndef PerlIO_seek
PERL_EXPORT_C int PerlIO_seek(PerlIO *, Off_t, int);
#endif
#ifndef PerlIO_rewind
PERL_EXPORT_C void PerlIO_rewind(PerlIO *);
#endif
#ifndef PerlIO_has_base
PERL_EXPORT_C int PerlIO_has_base(PerlIO *);
#endif
#ifndef PerlIO_has_cntptr
PERL_EXPORT_C int PerlIO_has_cntptr(PerlIO *);
#endif
#ifndef PerlIO_fast_gets
PERL_EXPORT_C int PerlIO_fast_gets(PerlIO *);
#endif
#ifndef PerlIO_canset_cnt
PERL_EXPORT_C int PerlIO_canset_cnt(PerlIO *);
#endif
#ifndef PerlIO_get_ptr
PERL_EXPORT_C STDCHAR *PerlIO_get_ptr(PerlIO *);
#endif
#ifndef PerlIO_get_cnt
PERL_EXPORT_C int PerlIO_get_cnt(PerlIO *);
#endif
#ifndef PerlIO_set_cnt
PERL_EXPORT_C void PerlIO_set_cnt(PerlIO *, int);
#endif
#ifndef PerlIO_set_ptrcnt
PERL_EXPORT_C void PerlIO_set_ptrcnt(PerlIO *, STDCHAR *, int);
#endif
#ifndef PerlIO_get_base
PERL_EXPORT_C STDCHAR *PerlIO_get_base(PerlIO *);
#endif
#ifndef PerlIO_get_bufsiz
PERL_EXPORT_C int PerlIO_get_bufsiz(PerlIO *);
#endif
#ifndef PerlIO_tmpfile
PERL_EXPORT_C PerlIO *PerlIO_tmpfile(void);
#endif
#ifndef PerlIO_stdin
PERL_EXPORT_C PerlIO *PerlIO_stdin(void);
#endif
#ifndef PerlIO_stdout
PERL_EXPORT_C PerlIO *PerlIO_stdout(void);
#endif
#ifndef PerlIO_stderr
PERL_EXPORT_C PerlIO *PerlIO_stderr(void);
#endif
#ifndef PerlIO_getpos
PERL_EXPORT_C int PerlIO_getpos(PerlIO *, SV *);
#endif
#ifndef PerlIO_setpos
PERL_EXPORT_C int PerlIO_setpos(PerlIO *, SV *);
#endif
#ifndef PerlIO_fdupopen
PERL_EXPORT_C PerlIO *PerlIO_fdupopen(pTHX_ PerlIO *, CLONE_PARAMS *, int);
#endif
#if !defined(PerlIO_modestr) && !defined(PERLIO_IS_STDIO)
PERL_EXPORT_C char *PerlIO_modestr(PerlIO *, char *buf);
#endif
#ifndef PerlIO_isutf8
PERL_EXPORT_C int PerlIO_isutf8(PerlIO *);
#endif
#ifndef PerlIO_apply_layers
PERL_EXPORT_C int PerlIO_apply_layers(pTHX_ PerlIO *f, const char *mode,
				      const char *names);
#endif
#ifndef PerlIO_binmode
PERL_EXPORT_C int PerlIO_binmode(pTHX_ PerlIO *f, int iotype, int omode,
			  	 const char *names);
#endif
#ifndef PerlIO_getname
PERL_EXPORT_C char *PerlIO_getname(PerlIO *, char *);
#endif

PERL_EXPORT_C void PerlIO_destruct(pTHX);

PERL_EXPORT_C int PerlIO_intmode2str(int rawmode, char *mode, int *writing);

#ifdef PERLIO_LAYERS
PERL_EXPORT_C void PerlIO_cleanup(pTHX);

PERL_EXPORT_C void PerlIO_debug(const char *fmt, ...)
    __attribute__format__(__printf__, 1, 2);
typedef struct PerlIO_list_s PerlIO_list_t;


#endif

END_EXTERN_C
#endif				/* _PERLIO_H */

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
