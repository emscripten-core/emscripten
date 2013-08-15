/*    perlsdio.h
 *
 *    Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001,
 *    2002, 2003, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifdef PERLIO_IS_STDIO

#ifdef NETWARE
	#include "nwstdio.h"
#else

/*
 * This file #define-s the PerlIO_xxx abstraction onto stdio functions.
 * Make this as close to original stdio as possible.
 */
#define PerlIO				FILE
#define PerlIO_stderr()			PerlSIO_stderr
#define PerlIO_stdout()			PerlSIO_stdout
#define PerlIO_stdin()			PerlSIO_stdin

#define PerlIO_isutf8(f)		0

#define PerlIO_printf			PerlSIO_printf
#define PerlIO_stdoutf			PerlSIO_stdoutf
#define PerlIO_vprintf(f,fmt,a)		PerlSIO_vprintf(f,fmt,a)
#define PerlIO_write(f,buf,count)	PerlSIO_fwrite(buf,1,count,f)
#define PerlIO_unread(f,buf,count)	(-1)
#define PerlIO_open			PerlSIO_fopen
#define PerlIO_fdopen			PerlSIO_fdopen
#define PerlIO_reopen			PerlSIO_freopen
#define PerlIO_close(f)			PerlSIO_fclose(f)
#define PerlIO_puts(f,s)		PerlSIO_fputs(s,f)
#define PerlIO_putc(f,c)		PerlSIO_fputc(c,f)
#if defined(VMS)
#  if defined(__DECC)
     /* Unusual definition of ungetc() here to accommodate fast_sv_gets()'
      * belief that it can mix getc/ungetc with reads from stdio buffer */
     int decc$ungetc(int __c, FILE *__stream);
#    define PerlIO_ungetc(f,c) ((c) == EOF ? EOF : \
            ((*(f) && !((*(f))->_flag & _IONBF) && \
            ((*(f))->_ptr > (*(f))->_base)) ? \
            ((*(f))->_cnt++, *(--(*(f))->_ptr) = (c)) : decc$ungetc(c,f)))
#  else
#    define PerlIO_ungetc(f,c)		ungetc(c,f)
#  endif
   /* Work around bug in DECCRTL/AXP (DECC v5.x) and some versions of old
    * VAXCRTL which causes read from a pipe after EOF has been returned
    * once to hang.
    */
#  define PerlIO_getc(f) \
		(feof(f) ? EOF : getc(f))
#  define PerlIO_read(f,buf,count) \
		(feof(f) ? 0 : (SSize_t)fread(buf,1,count,f))
#  define PerlIO_tell(f)		ftell(f)
#else
#  define PerlIO_getc(f)		PerlSIO_fgetc(f)
#  define PerlIO_ungetc(f,c)		PerlSIO_ungetc(c,f)
#  define PerlIO_read(f,buf,count)	(SSize_t)PerlSIO_fread(buf,1,count,f)
#  define PerlIO_tell(f)		PerlSIO_ftell(f)
#endif
#define PerlIO_eof(f)			PerlSIO_feof(f)
#define PerlIO_getname(f,b)		fgetname(f,b)
#define PerlIO_error(f)			PerlSIO_ferror(f)
#define PerlIO_fileno(f)		PerlSIO_fileno(f)
#define PerlIO_clearerr(f)		PerlSIO_clearerr(f)
#define PerlIO_flush(f)			PerlSIO_fflush(f)
#if defined(VMS) && !defined(__DECC)
/* Old VAXC RTL doesn't reset EOF on seek; Perl folk seem to expect this */
#define PerlIO_seek(f,o,w)	(((f) && (*f) && ((*f)->_flag &= ~_IOEOF)),fseek(f,o,w))
#else
#  define PerlIO_seek(f,o,w)		PerlSIO_fseek(f,o,w)
#endif

#define PerlIO_rewind(f)		PerlSIO_rewind(f)
#define PerlIO_tmpfile()		PerlSIO_tmpfile()

#define PerlIO_importFILE(f,fl)		(f)
#define PerlIO_exportFILE(f,fl)		(f)
#define PerlIO_findFILE(f)		(f)
#define PerlIO_releaseFILE(p,f)		((void) 0)

#ifdef HAS_SETLINEBUF
#define PerlIO_setlinebuf(f)		PerlSIO_setlinebuf(f);
#else
#define PerlIO_setlinebuf(f)		PerlSIO_setvbuf(f, NULL, _IOLBF, 0);
#endif

/* Now our interface to Configure's FILE_xxx macros */

#ifdef USE_STDIO_PTR
#define PerlIO_has_cntptr(f)		1
#define PerlIO_get_ptr(f)		PerlSIO_get_ptr(f)
#define PerlIO_get_cnt(f)		PerlSIO_get_cnt(f)

#ifdef STDIO_CNT_LVALUE
#define PerlIO_canset_cnt(f)		1
#define PerlIO_set_cnt(f,c)		PerlSIO_set_cnt(f,c)
#ifdef STDIO_PTR_LVALUE
#ifdef STDIO_PTR_LVAL_NOCHANGE_CNT
#define PerlIO_fast_gets(f)		1
#endif
#endif /* STDIO_PTR_LVALUE */
#else /* STDIO_CNT_LVALUE */
#define PerlIO_canset_cnt(f)		0
#define PerlIO_set_cnt(f,c)		abort()
#endif

#ifdef STDIO_PTR_LVALUE
#ifdef STDIO_PTR_LVAL_NOCHANGE_CNT
#define PerlIO_set_ptrcnt(f,p,c)      STMT_START {PerlSIO_set_ptr(f,p), PerlIO_set_cnt(f,c);} STMT_END
#else
#ifdef STDIO_PTR_LVAL_SETS_CNT
/* assert() may pre-process to ""; potential syntax error (FILE_ptr(), ) */
#define PerlIO_set_ptrcnt(f,p,c)      STMT_START {PerlSIO_set_ptr(f,p); assert(PerlSIO_get_cnt(f) == (c));} STMT_END
#define PerlIO_fast_gets(f)		1
#else
#define PerlIO_set_ptrcnt(f,p,c)	abort()
#endif
#endif
#endif

#else  /* USE_STDIO_PTR */

#define PerlIO_has_cntptr(f)		0
#define PerlIO_canset_cnt(f)		0
#define PerlIO_get_cnt(f)		(abort(),0)
#define PerlIO_get_ptr(f)		(abort(),(void *)0)
#define PerlIO_set_cnt(f,c)		abort()
#define PerlIO_set_ptrcnt(f,p,c)	abort()

#endif /* USE_STDIO_PTR */

#ifndef PerlIO_fast_gets
#define PerlIO_fast_gets(f)		0
#endif


#ifdef FILE_base
#define PerlIO_has_base(f)		1
#define PerlIO_get_base(f)		PerlSIO_get_base(f)
#define PerlIO_get_bufsiz(f)		PerlSIO_get_bufsiz(f)
#else
#define PerlIO_has_base(f)		0
#define PerlIO_get_base(f)		(abort(),(void *)0)
#define PerlIO_get_bufsiz(f)		(abort(),0)
#endif

#endif	/* NETWARE */
#endif /* PERLIO_IS_STDIO */

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
