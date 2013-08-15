#line 2 "perl.c"
/*    perl.c
 *
 *    Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001
 *    2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
 *     by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *      A ship then new they built for him
 *      of mithril and of elven-glass
 *              --from Bilbo's song of Eärendil
 *
 *     [p.236 of _The Lord of the Rings_, II/i: "Many Meetings"]
 */

/* This file contains the top-level functions that are used to create, use
 * and destroy a perl interpreter, plus the functions used by XS code to
 * call back into perl. Note that it does not contain the actual main()
 * function of the interpreter; that can be found in perlmain.c
 */

#if defined(PERL_IS_MINIPERL) && !defined(USE_SITECUSTOMIZE)
#  define USE_SITECUSTOMIZE
#endif

#include "EXTERN.h"
#define PERL_IN_PERL_C
#include "perl.h"
#include "patchlevel.h"			/* for local_patches */
#include "XSUB.h"

#ifdef NETWARE
#include "nwutil.h"
#endif

#ifdef USE_KERN_PROC_PATHNAME
#  include <sys/sysctl.h>
#endif

#ifdef USE_NSGETEXECUTABLEPATH
#  include <mach-o/dyld.h>
#endif

#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
#  ifdef I_SYSUIO
#    include <sys/uio.h>
#  endif

union control_un {
  struct cmsghdr cm;
  char control[CMSG_SPACE(sizeof(int))];
};

#endif

#ifdef __BEOS__
#  define HZ 1000000
#endif

#ifndef HZ
#  ifdef CLK_TCK
#    define HZ CLK_TCK
#  else
#    define HZ 60
#  endif
#endif

#if !defined(STANDARD_C) && !defined(HAS_GETENV_PROTOTYPE) && !defined(PERL_MICRO)
char *getenv (char *); /* Usually in <stdlib.h> */
#endif

static I32 read_e_script(pTHX_ int idx, SV *buf_sv, int maxlen);

#ifdef SETUID_SCRIPTS_ARE_SECURE_NOW
#  define validate_suid(rsfp) NOOP
#else
#  define validate_suid(rsfp) S_validate_suid(aTHX_ rsfp)
#endif

#define CALL_BODY_SUB(myop) \
    if (PL_op == (myop)) \
	PL_op = PL_ppaddr[OP_ENTERSUB](aTHX); \
    if (PL_op) \
	CALLRUNOPS(aTHX);

#define CALL_LIST_BODY(cv) \
    PUSHMARK(PL_stack_sp); \
    call_sv(MUTABLE_SV((cv)), G_EVAL|G_DISCARD|G_VOID);

static void
S_init_tls_and_interp(PerlInterpreter *my_perl)
{
    dVAR;
    if (!PL_curinterp) {
	PERL_SET_INTERP(my_perl);
#if defined(USE_ITHREADS)
	INIT_THREADS;
	ALLOC_THREAD_KEY;
	PERL_SET_THX(my_perl);
	OP_REFCNT_INIT;
	OP_CHECK_MUTEX_INIT;
	HINTS_REFCNT_INIT;
	MUTEX_INIT(&PL_dollarzero_mutex);
	MUTEX_INIT(&PL_my_ctx_mutex);
#  endif
    }
#if defined(USE_ITHREADS)
    else
#else
    /* This always happens for non-ithreads  */
#endif
    {
	PERL_SET_THX(my_perl);
    }
}


/* these implement the PERL_SYS_INIT, PERL_SYS_INIT3, PERL_SYS_TERM macros */

void
Perl_sys_init(int* argc, char*** argv)
{
    dVAR;

    PERL_ARGS_ASSERT_SYS_INIT;

    PERL_UNUSED_ARG(argc); /* may not be used depending on _BODY macro */
    PERL_UNUSED_ARG(argv);
    PERL_SYS_INIT_BODY(argc, argv);
}

void
Perl_sys_init3(int* argc, char*** argv, char*** env)
{
    dVAR;

    PERL_ARGS_ASSERT_SYS_INIT3;

    PERL_UNUSED_ARG(argc); /* may not be used depending on _BODY macro */
    PERL_UNUSED_ARG(argv);
    PERL_UNUSED_ARG(env);
    PERL_SYS_INIT3_BODY(argc, argv, env);
}

void
Perl_sys_term()
{
    dVAR;
    if (!PL_veto_cleanup) {
	PERL_SYS_TERM_BODY();
    }
}


#ifdef PERL_IMPLICIT_SYS
PerlInterpreter *
perl_alloc_using(struct IPerlMem* ipM, struct IPerlMem* ipMS,
		 struct IPerlMem* ipMP, struct IPerlEnv* ipE,
		 struct IPerlStdIO* ipStd, struct IPerlLIO* ipLIO,
		 struct IPerlDir* ipD, struct IPerlSock* ipS,
		 struct IPerlProc* ipP)
{
    PerlInterpreter *my_perl;

    PERL_ARGS_ASSERT_PERL_ALLOC_USING;

    /* Newx() needs interpreter, so call malloc() instead */
    my_perl = (PerlInterpreter*)(*ipM->pMalloc)(ipM, sizeof(PerlInterpreter));
    S_init_tls_and_interp(my_perl);
    Zero(my_perl, 1, PerlInterpreter);
    PL_Mem = ipM;
    PL_MemShared = ipMS;
    PL_MemParse = ipMP;
    PL_Env = ipE;
    PL_StdIO = ipStd;
    PL_LIO = ipLIO;
    PL_Dir = ipD;
    PL_Sock = ipS;
    PL_Proc = ipP;
    INIT_TRACK_MEMPOOL(PL_memory_debug_header, my_perl);

    return my_perl;
}
#else

/*
=head1 Embedding Functions

=for apidoc perl_alloc

Allocates a new Perl interpreter.  See L<perlembed>.

=cut
*/

PerlInterpreter *
perl_alloc(void)
{
    PerlInterpreter *my_perl;

    /* Newx() needs interpreter, so call malloc() instead */
    my_perl = (PerlInterpreter*)PerlMem_malloc(sizeof(PerlInterpreter));

    S_init_tls_and_interp(my_perl);
#ifndef PERL_TRACK_MEMPOOL
    return (PerlInterpreter *) ZeroD(my_perl, 1, PerlInterpreter);
#else
    Zero(my_perl, 1, PerlInterpreter);
    INIT_TRACK_MEMPOOL(PL_memory_debug_header, my_perl);
    return my_perl;
#endif
}
#endif /* PERL_IMPLICIT_SYS */

/*
=for apidoc perl_construct

Initializes a new Perl interpreter.  See L<perlembed>.

=cut
*/

void
perl_construct(pTHXx)
{
    dVAR;

    PERL_ARGS_ASSERT_PERL_CONSTRUCT;

#ifdef MULTIPLICITY
    init_interp();
    PL_perl_destruct_level = 1;
#else
    PERL_UNUSED_ARG(my_perl);
   if (PL_perl_destruct_level > 0)
       init_interp();
#endif
    PL_curcop = &PL_compiling;	/* needed by ckWARN, right away */

    /* set read-only and try to insure than we wont see REFCNT==0
       very often */

    SvREADONLY_on(&PL_sv_undef);
    SvREFCNT(&PL_sv_undef) = (~(U32)0)/2;

    sv_setpv(&PL_sv_no,PL_No);
    /* value lookup in void context - happens to have the side effect
       of caching the numeric forms. However, as &PL_sv_no doesn't contain
       a string that is a valid numer, we have to turn the public flags by
       hand:  */
    SvNV(&PL_sv_no);
    SvIV(&PL_sv_no);
    SvIOK_on(&PL_sv_no);
    SvNOK_on(&PL_sv_no);
    SvREADONLY_on(&PL_sv_no);
    SvREFCNT(&PL_sv_no) = (~(U32)0)/2;

    sv_setpv(&PL_sv_yes,PL_Yes);
    SvNV(&PL_sv_yes);
    SvIV(&PL_sv_yes);
    SvREADONLY_on(&PL_sv_yes);
    SvREFCNT(&PL_sv_yes) = (~(U32)0)/2;

    SvREADONLY_on(&PL_sv_placeholder);
    SvREFCNT(&PL_sv_placeholder) = (~(U32)0)/2;

    PL_sighandlerp = (Sighandler_t) Perl_sighandler;
#ifdef PERL_USES_PL_PIDSTATUS
    PL_pidstatus = newHV();
#endif

    PL_rs = newSVpvs("\n");

    init_stacks();

    init_ids();

    JMPENV_BOOTSTRAP;
    STATUS_ALL_SUCCESS;

    init_i18nl10n(1);
    SET_NUMERIC_STANDARD();

#if defined(LOCAL_PATCH_COUNT)
    PL_localpatches = local_patches;	/* For possible -v */
#endif

#ifdef HAVE_INTERP_INTERN
    sys_intern_init();
#endif

    PerlIO_init(aTHX);			/* Hook to IO system */

    PL_fdpid = newAV();			/* for remembering popen pids by fd */
    PL_modglobal = newHV();		/* pointers to per-interpreter module globals */
    PL_errors = newSVpvs("");
    sv_setpvs(PERL_DEBUG_PAD(0), "");	/* For regex debugging. */
    sv_setpvs(PERL_DEBUG_PAD(1), "");	/* ext/re needs these */
    sv_setpvs(PERL_DEBUG_PAD(2), "");	/* even without DEBUGGING. */
#ifdef USE_ITHREADS
    /* First entry is a list of empty elements. It needs to be initialised
       else all hell breaks loose in S_find_uninit_var().  */
    Perl_av_create_and_push(aTHX_ &PL_regex_padav, newSVpvs(""));
    PL_regex_pad = AvARRAY(PL_regex_padav);
#endif
#ifdef USE_REENTRANT_API
    Perl_reentrant_init(aTHX);
#endif

    /* Note that strtab is a rather special HV.  Assumptions are made
       about not iterating on it, and not adding tie magic to it.
       It is properly deallocated in perl_destruct() */
    PL_strtab = newHV();

    HvSHAREKEYS_off(PL_strtab);			/* mandatory */
    hv_ksplit(PL_strtab, 512);

#if defined(__DYNAMIC__) && (defined(NeXT) || defined(__NeXT__))
    _dyld_lookup_and_bind
	("__environ", (unsigned long *) &environ_pointer, NULL);
#endif /* environ */

#ifndef PERL_MICRO
#   ifdef  USE_ENVIRON_ARRAY
    PL_origenviron = environ;
#   endif
#endif

    /* Use sysconf(_SC_CLK_TCK) if available, if not
     * available or if the sysconf() fails, use the HZ.
     * BeOS has those, but returns the wrong value.
     * The HZ if not originally defined has been by now
     * been defined as CLK_TCK, if available. */
#if defined(HAS_SYSCONF) && defined(_SC_CLK_TCK) && !defined(__BEOS__)
    PL_clocktick = sysconf(_SC_CLK_TCK);
    if (PL_clocktick <= 0)
#endif
	 PL_clocktick = HZ;

    PL_stashcache = newHV();

    PL_patchlevel = newSVpvs("v" PERL_VERSION_STRING);
    PL_apiversion = newSVpvs("v" PERL_API_VERSION_STRING);

#ifdef HAS_MMAP
    if (!PL_mmap_page_size) {
#if defined(HAS_SYSCONF) && (defined(_SC_PAGESIZE) || defined(_SC_MMAP_PAGE_SIZE))
      {
	SETERRNO(0, SS_NORMAL);
#   ifdef _SC_PAGESIZE
	PL_mmap_page_size = sysconf(_SC_PAGESIZE);
#   else
	PL_mmap_page_size = sysconf(_SC_MMAP_PAGE_SIZE);
#   endif
	if ((long) PL_mmap_page_size < 0) {
	  if (errno) {
	    SV * const error = ERRSV;
	    SvUPGRADE(error, SVt_PV);
	    Perl_croak(aTHX_ "panic: sysconf: %s", SvPV_nolen_const(error));
	  }
	  else
	    Perl_croak(aTHX_ "panic: sysconf: pagesize unknown");
	}
      }
#else
#   ifdef HAS_GETPAGESIZE
      PL_mmap_page_size = getpagesize();
#   else
#       if defined(I_SYS_PARAM) && defined(PAGESIZE)
      PL_mmap_page_size = PAGESIZE;       /* compiletime, bad */
#       endif
#   endif
#endif
      if (PL_mmap_page_size <= 0)
	Perl_croak(aTHX_ "panic: bad pagesize %" IVdf,
		   (IV) PL_mmap_page_size);
    }
#endif /* HAS_MMAP */

#if defined(HAS_TIMES) && defined(PERL_NEED_TIMESBASE)
    PL_timesbase.tms_utime  = 0;
    PL_timesbase.tms_stime  = 0;
    PL_timesbase.tms_cutime = 0;
    PL_timesbase.tms_cstime = 0;
#endif

    PL_osname = Perl_savepvn(aTHX_ STR_WITH_LEN(OSNAME));

    PL_registered_mros = newHV();
    /* Start with 1 bucket, for DFS.  It's unlikely we'll need more.  */
    HvMAX(PL_registered_mros) = 0;

    ENTER;
}

/*
=for apidoc nothreadhook

Stub that provides thread hook for perl_destruct when there are
no threads.

=cut
*/

int
Perl_nothreadhook(pTHX)
{
    PERL_UNUSED_CONTEXT;
    return 0;
}

#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
void
Perl_dump_sv_child(pTHX_ SV *sv)
{
    ssize_t got;
    const int sock = PL_dumper_fd;
    const int debug_fd = PerlIO_fileno(Perl_debug_log);
    union control_un control;
    struct msghdr msg;
    struct iovec vec[2];
    struct cmsghdr *cmptr;
    int returned_errno;
    unsigned char buffer[256];

    PERL_ARGS_ASSERT_DUMP_SV_CHILD;

    if(sock == -1 || debug_fd == -1)
	return;

    PerlIO_flush(Perl_debug_log);

    /* All these shenanigans are to pass a file descriptor over to our child for
       it to dump out to.  We can't let it hold open the file descriptor when it
       forks, as the file descriptor it will dump to can turn out to be one end
       of pipe that some other process will wait on for EOF. (So as it would
       be open, the wait would be forever.)  */

    msg.msg_control = control.control;
    msg.msg_controllen = sizeof(control.control);
    /* We're a connected socket so we don't need a destination  */
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = vec;
    msg.msg_iovlen = 1;

    cmptr = CMSG_FIRSTHDR(&msg);
    cmptr->cmsg_len = CMSG_LEN(sizeof(int));
    cmptr->cmsg_level = SOL_SOCKET;
    cmptr->cmsg_type = SCM_RIGHTS;
    *((int *)CMSG_DATA(cmptr)) = 1;

    vec[0].iov_base = (void*)&sv;
    vec[0].iov_len = sizeof(sv);
    got = sendmsg(sock, &msg, 0);

    if(got < 0) {
	perror("Debug leaking scalars parent sendmsg failed");
	abort();
    }
    if(got < sizeof(sv)) {
	perror("Debug leaking scalars parent short sendmsg");
	abort();
    }

    /* Return protocol is
       int:		errno value
       unsigned char:	length of location string (0 for empty)
       unsigned char*:	string (not terminated)
    */
    vec[0].iov_base = (void*)&returned_errno;
    vec[0].iov_len = sizeof(returned_errno);
    vec[1].iov_base = buffer;
    vec[1].iov_len = 1;

    got = readv(sock, vec, 2);

    if(got < 0) {
	perror("Debug leaking scalars parent read failed");
	PerlIO_flush(PerlIO_stderr());
	abort();
    }
    if(got < sizeof(returned_errno) + 1) {
	perror("Debug leaking scalars parent short read");
	PerlIO_flush(PerlIO_stderr());
	abort();
    }

    if (*buffer) {
	got = read(sock, buffer + 1, *buffer);
	if(got < 0) {
	    perror("Debug leaking scalars parent read 2 failed");
	    PerlIO_flush(PerlIO_stderr());
	    abort();
	}

	if(got < *buffer) {
	    perror("Debug leaking scalars parent short read 2");
	    PerlIO_flush(PerlIO_stderr());
	    abort();
	}
    }

    if (returned_errno || *buffer) {
	Perl_warn(aTHX_ "Debug leaking scalars child failed%s%.*s with errno"
		  " %d: %s", (*buffer ? " at " : ""), (int) *buffer, buffer + 1,
		  returned_errno, strerror(returned_errno));
    }
}
#endif

/*
=for apidoc perl_destruct

Shuts down a Perl interpreter.  See L<perlembed>.

=cut
*/

int
perl_destruct(pTHXx)
{
    dVAR;
    VOL signed char destruct_level;  /* see possible values in intrpvar.h */
    HV *hv;
#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
    pid_t child;
#endif

    PERL_ARGS_ASSERT_PERL_DESTRUCT;
#ifndef MULTIPLICITY
    PERL_UNUSED_ARG(my_perl);
#endif

    assert(PL_scopestack_ix == 1);

    /* wait for all pseudo-forked children to finish */
    PERL_WAIT_FOR_CHILDREN;

    destruct_level = PL_perl_destruct_level;
#ifdef DEBUGGING
    {
	const char * const s = PerlEnv_getenv("PERL_DESTRUCT_LEVEL");
	if (s) {
            const int i = atoi(s);
	    if (destruct_level < i)
		destruct_level = i;
	}
    }
#endif

    if (PL_exit_flags & PERL_EXIT_DESTRUCT_END) {
        dJMPENV;
        int x = 0;

        JMPENV_PUSH(x);
	PERL_UNUSED_VAR(x);
        if (PL_endav && !PL_minus_c) {
	    PERL_SET_PHASE(PERL_PHASE_END);
            call_list(PL_scopestack_ix, PL_endav);
	}
        JMPENV_POP;
    }
    LEAVE;
    FREETMPS;
    assert(PL_scopestack_ix == 0);

    /* Need to flush since END blocks can produce output */
    my_fflush_all();

    if (PL_threadhook(aTHX)) {
        /* Threads hook has vetoed further cleanup */
	PL_veto_cleanup = TRUE;
        return STATUS_EXIT;
    }

#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
    if (destruct_level != 0) {
	/* Fork here to create a child. Our child's job is to preserve the
	   state of scalars prior to destruction, so that we can instruct it
	   to dump any scalars that we later find have leaked.
	   There's no subtlety in this code - it assumes POSIX, and it doesn't
	   fail gracefully  */
	int fd[2];

	if(socketpair(AF_UNIX, SOCK_STREAM, 0, fd)) {
	    perror("Debug leaking scalars socketpair failed");
	    abort();
	}

	child = fork();
	if(child == -1) {
	    perror("Debug leaking scalars fork failed");
	    abort();
	}
	if (!child) {
	    /* We are the child */
	    const int sock = fd[1];
	    const int debug_fd = PerlIO_fileno(Perl_debug_log);
	    int f;
	    const char *where;
	    /* Our success message is an integer 0, and a char 0  */
	    static const char success[sizeof(int) + 1] = {0};

	    close(fd[0]);

	    /* We need to close all other file descriptors otherwise we end up
	       with interesting hangs, where the parent closes its end of a
	       pipe, and sits waiting for (another) child to terminate. Only
	       that child never terminates, because it never gets EOF, because
	       we also have the far end of the pipe open.  We even need to
	       close the debugging fd, because sometimes it happens to be one
	       end of a pipe, and a process is waiting on the other end for
	       EOF. Normally it would be closed at some point earlier in
	       destruction, but if we happen to cause the pipe to remain open,
	       EOF never occurs, and we get an infinite hang. Hence all the
	       games to pass in a file descriptor if it's actually needed.  */

	    f = sysconf(_SC_OPEN_MAX);
	    if(f < 0) {
		where = "sysconf failed";
		goto abort;
	    }
	    while (f--) {
		if (f == sock)
		    continue;
		close(f);
	    }

	    while (1) {
		SV *target;
		union control_un control;
		struct msghdr msg;
		struct iovec vec[1];
		struct cmsghdr *cmptr;
		ssize_t got;
		int got_fd;

		msg.msg_control = control.control;
		msg.msg_controllen = sizeof(control.control);
		/* We're a connected socket so we don't need a source  */
		msg.msg_name = NULL;
		msg.msg_namelen = 0;
		msg.msg_iov = vec;
		msg.msg_iovlen = sizeof(vec)/sizeof(vec[0]);

		vec[0].iov_base = (void*)&target;
		vec[0].iov_len = sizeof(target);

		got = recvmsg(sock, &msg, 0);

		if(got == 0)
		    break;
		if(got < 0) {
		    where = "recv failed";
		    goto abort;
		}
		if(got < sizeof(target)) {
		    where = "short recv";
		    goto abort;
		}

		if(!(cmptr = CMSG_FIRSTHDR(&msg))) {
		    where = "no cmsg";
		    goto abort;
		}
		if(cmptr->cmsg_len != CMSG_LEN(sizeof(int))) {
		    where = "wrong cmsg_len";
		    goto abort;
		}
		if(cmptr->cmsg_level != SOL_SOCKET) {
		    where = "wrong cmsg_level";
		    goto abort;
		}
		if(cmptr->cmsg_type != SCM_RIGHTS) {
		    where = "wrong cmsg_type";
		    goto abort;
		}

		got_fd = *(int*)CMSG_DATA(cmptr);
		/* For our last little bit of trickery, put the file descriptor
		   back into Perl_debug_log, as if we never actually closed it
		*/
		if(got_fd != debug_fd) {
		    if (dup2(got_fd, debug_fd) == -1) {
			where = "dup2";
			goto abort;
		    }
		}
		sv_dump(target);

		PerlIO_flush(Perl_debug_log);

		got = write(sock, &success, sizeof(success));

		if(got < 0) {
		    where = "write failed";
		    goto abort;
		}
		if(got < sizeof(success)) {
		    where = "short write";
		    goto abort;
		}
	    }
	    _exit(0);
	abort:
	    {
		int send_errno = errno;
		unsigned char length = (unsigned char) strlen(where);
		struct iovec failure[3] = {
		    {(void*)&send_errno, sizeof(send_errno)},
		    {&length, 1},
		    {(void*)where, length}
		};
		int got = writev(sock, failure, 3);
		/* Bad news travels fast. Faster than data. We'll get a SIGPIPE
		   in the parent if we try to read from the socketpair after the
		   child has exited, even if there was data to read.
		   So sleep a bit to give the parent a fighting chance of
		   reading the data.  */
		sleep(2);
		_exit((got == -1) ? errno : 0);
	    }
	    /* End of child.  */
	}
	PL_dumper_fd = fd[0];
	close(fd[1]);
    }
#endif

    /* We must account for everything.  */

    /* Destroy the main CV and syntax tree */
    /* Do this now, because destroying ops can cause new SVs to be generated
       in Perl_pad_swipe, and when running with -DDEBUG_LEAKING_SCALARS they
       PL_curcop to point to a valid op from which the filename structure
       member is copied.  */
    PL_curcop = &PL_compiling;
    if (PL_main_root) {
	/* ensure comppad/curpad to refer to main's pad */
	if (CvPADLIST(PL_main_cv)) {
	    PAD_SET_CUR_NOSAVE(CvPADLIST(PL_main_cv), 1);
	}
	op_free(PL_main_root);
	PL_main_root = NULL;
    }
    PL_main_start = NULL;
    /* note that  PL_main_cv isn't usually actually freed at this point,
     * due to the CvOUTSIDE refs from subs compiled within it. It will
     * get freed once all the subs are freed in sv_clean_all(), for
     * destruct_level > 0 */
    SvREFCNT_dec(PL_main_cv);
    PL_main_cv = NULL;
    PERL_SET_PHASE(PERL_PHASE_DESTRUCT);

    /* Tell PerlIO we are about to tear things apart in case
       we have layers which are using resources that should
       be cleaned up now.
     */

    PerlIO_destruct(aTHX);

    if (PL_sv_objcount) {
	/*
	 * Try to destruct global references.  We do this first so that the
	 * destructors and destructees still exist.  Some sv's might remain.
	 * Non-referenced objects are on their own.
	 */
	sv_clean_objs();
	PL_sv_objcount = 0;
    }

    /* unhook hooks which will soon be, or use, destroyed data */
    SvREFCNT_dec(PL_warnhook);
    PL_warnhook = NULL;
    SvREFCNT_dec(PL_diehook);
    PL_diehook = NULL;

    /* call exit list functions */
    while (PL_exitlistlen-- > 0)
	PL_exitlist[PL_exitlistlen].fn(aTHX_ PL_exitlist[PL_exitlistlen].ptr);

    Safefree(PL_exitlist);

    PL_exitlist = NULL;
    PL_exitlistlen = 0;

    SvREFCNT_dec(PL_registered_mros);

    /* jettison our possibly duplicated environment */
    /* if PERL_USE_SAFE_PUTENV is defined environ will not have been copied
     * so we certainly shouldn't free it here
     */
#ifndef PERL_MICRO
#if defined(USE_ENVIRON_ARRAY) && !defined(PERL_USE_SAFE_PUTENV)
    if (environ != PL_origenviron && !PL_use_safe_putenv
#ifdef USE_ITHREADS
	/* only main thread can free environ[0] contents */
	&& PL_curinterp == aTHX
#endif
	)
    {
	I32 i;

	for (i = 0; environ[i]; i++)
	    safesysfree(environ[i]);

	/* Must use safesysfree() when working with environ. */
	safesysfree(environ);

	environ = PL_origenviron;
    }
#endif
#endif /* !PERL_MICRO */

    if (destruct_level == 0) {

	DEBUG_P(debprofdump());

#if defined(PERLIO_LAYERS)
	/* No more IO - including error messages ! */
	PerlIO_cleanup(aTHX);
#endif

	CopFILE_free(&PL_compiling);
	CopSTASH_free(&PL_compiling);

	/* The exit() function will do everything that needs doing. */
        return STATUS_EXIT;
    }

#ifdef USE_ITHREADS
    /* the syntax tree is shared between clones
     * so op_free(PL_main_root) only ReREFCNT_dec's
     * REGEXPs in the parent interpreter
     * we need to manually ReREFCNT_dec for the clones
     */
    SvREFCNT_dec(PL_regex_padav);
    PL_regex_padav = NULL;
    PL_regex_pad = NULL;
#endif

    SvREFCNT_dec(MUTABLE_SV(PL_stashcache));
    PL_stashcache = NULL;

    /* loosen bonds of global variables */

    /* XXX can PL_parser still be non-null here? */
    if(PL_parser && PL_parser->rsfp) {
	(void)PerlIO_close(PL_parser->rsfp);
	PL_parser->rsfp = NULL;
    }

    if (PL_minus_F) {
	Safefree(PL_splitstr);
	PL_splitstr = NULL;
    }

    /* switches */
    PL_minus_n      = FALSE;
    PL_minus_p      = FALSE;
    PL_minus_l      = FALSE;
    PL_minus_a      = FALSE;
    PL_minus_F      = FALSE;
    PL_doswitches   = FALSE;
    PL_dowarn       = G_WARN_OFF;
    PL_sawampersand = FALSE;	/* must save all match strings */
    PL_unsafe       = FALSE;

    Safefree(PL_inplace);
    PL_inplace = NULL;
    SvREFCNT_dec(PL_patchlevel);
    SvREFCNT_dec(PL_apiversion);

    if (PL_e_script) {
	SvREFCNT_dec(PL_e_script);
	PL_e_script = NULL;
    }

    PL_perldb = 0;

    /* magical thingies */

    SvREFCNT_dec(PL_ofsgv);	/* *, */
    PL_ofsgv = NULL;

    SvREFCNT_dec(PL_ors_sv);	/* $\ */
    PL_ors_sv = NULL;

    SvREFCNT_dec(PL_rs);	/* $/ */
    PL_rs = NULL;

    Safefree(PL_osname);	/* $^O */
    PL_osname = NULL;

    SvREFCNT_dec(PL_statname);
    PL_statname = NULL;
    PL_statgv = NULL;

    /* defgv, aka *_ should be taken care of elsewhere */

    /* float buffer */
    Safefree(PL_efloatbuf);
    PL_efloatbuf = NULL;
    PL_efloatsize = 0;

    /* startup and shutdown function lists */
    SvREFCNT_dec(PL_beginav);
    SvREFCNT_dec(PL_beginav_save);
    SvREFCNT_dec(PL_endav);
    SvREFCNT_dec(PL_checkav);
    SvREFCNT_dec(PL_checkav_save);
    SvREFCNT_dec(PL_unitcheckav);
    SvREFCNT_dec(PL_unitcheckav_save);
    SvREFCNT_dec(PL_initav);
    PL_beginav = NULL;
    PL_beginav_save = NULL;
    PL_endav = NULL;
    PL_checkav = NULL;
    PL_checkav_save = NULL;
    PL_unitcheckav = NULL;
    PL_unitcheckav_save = NULL;
    PL_initav = NULL;

    /* shortcuts just get cleared */
    PL_envgv = NULL;
    PL_incgv = NULL;
    PL_hintgv = NULL;
    PL_errgv = NULL;
    PL_argvgv = NULL;
    PL_argvoutgv = NULL;
    PL_stdingv = NULL;
    PL_stderrgv = NULL;
    PL_last_in_gv = NULL;
    PL_replgv = NULL;
    PL_DBgv = NULL;
    PL_DBline = NULL;
    PL_DBsub = NULL;
    PL_DBsingle = NULL;
    PL_DBtrace = NULL;
    PL_DBsignal = NULL;
    PL_DBcv = NULL;
    PL_dbargs = NULL;
    PL_debstash = NULL;

    SvREFCNT_dec(PL_argvout_stack);
    PL_argvout_stack = NULL;

    SvREFCNT_dec(PL_modglobal);
    PL_modglobal = NULL;
    SvREFCNT_dec(PL_preambleav);
    PL_preambleav = NULL;
    SvREFCNT_dec(PL_subname);
    PL_subname = NULL;
#ifdef PERL_USES_PL_PIDSTATUS
    SvREFCNT_dec(PL_pidstatus);
    PL_pidstatus = NULL;
#endif
    SvREFCNT_dec(PL_toptarget);
    PL_toptarget = NULL;
    SvREFCNT_dec(PL_bodytarget);
    PL_bodytarget = NULL;
    PL_formtarget = NULL;

    /* free locale stuff */
#ifdef USE_LOCALE_COLLATE
    Safefree(PL_collation_name);
    PL_collation_name = NULL;
#endif

#ifdef USE_LOCALE_NUMERIC
    Safefree(PL_numeric_name);
    PL_numeric_name = NULL;
    SvREFCNT_dec(PL_numeric_radix_sv);
    PL_numeric_radix_sv = NULL;
#endif

    /* clear utf8 character classes */
    SvREFCNT_dec(PL_utf8_alnum);
    SvREFCNT_dec(PL_utf8_alpha);
    SvREFCNT_dec(PL_utf8_space);
    SvREFCNT_dec(PL_utf8_graph);
    SvREFCNT_dec(PL_utf8_digit);
    SvREFCNT_dec(PL_utf8_upper);
    SvREFCNT_dec(PL_utf8_lower);
    SvREFCNT_dec(PL_utf8_print);
    SvREFCNT_dec(PL_utf8_punct);
    SvREFCNT_dec(PL_utf8_xdigit);
    SvREFCNT_dec(PL_utf8_mark);
    SvREFCNT_dec(PL_utf8_toupper);
    SvREFCNT_dec(PL_utf8_totitle);
    SvREFCNT_dec(PL_utf8_tolower);
    SvREFCNT_dec(PL_utf8_tofold);
    SvREFCNT_dec(PL_utf8_idstart);
    SvREFCNT_dec(PL_utf8_idcont);
    SvREFCNT_dec(PL_utf8_foldclosures);
    PL_utf8_alnum	= NULL;
    PL_utf8_alpha	= NULL;
    PL_utf8_space	= NULL;
    PL_utf8_graph	= NULL;
    PL_utf8_digit	= NULL;
    PL_utf8_upper	= NULL;
    PL_utf8_lower	= NULL;
    PL_utf8_print	= NULL;
    PL_utf8_punct	= NULL;
    PL_utf8_xdigit	= NULL;
    PL_utf8_mark	= NULL;
    PL_utf8_toupper	= NULL;
    PL_utf8_totitle	= NULL;
    PL_utf8_tolower	= NULL;
    PL_utf8_tofold	= NULL;
    PL_utf8_idstart	= NULL;
    PL_utf8_idcont	= NULL;
    PL_utf8_foldclosures = NULL;

    if (!specialWARN(PL_compiling.cop_warnings))
	PerlMemShared_free(PL_compiling.cop_warnings);
    PL_compiling.cop_warnings = NULL;
    cophh_free(CopHINTHASH_get(&PL_compiling));
    CopHINTHASH_set(&PL_compiling, cophh_new_empty());
    CopFILE_free(&PL_compiling);
    CopSTASH_free(&PL_compiling);

    /* Prepare to destruct main symbol table.  */

    hv = PL_defstash;
    /* break ref loop  *:: <=> %:: */
    (void)hv_delete(hv, "main::", 6, G_DISCARD);
    PL_defstash = 0;
    SvREFCNT_dec(hv);
    SvREFCNT_dec(PL_curstname);
    PL_curstname = NULL;

    /* clear queued errors */
    SvREFCNT_dec(PL_errors);
    PL_errors = NULL;

    SvREFCNT_dec(PL_isarev);

    FREETMPS;
    if (destruct_level >= 2) {
	if (PL_scopestack_ix != 0)
	    Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
			     "Unbalanced scopes: %ld more ENTERs than LEAVEs\n",
			     (long)PL_scopestack_ix);
	if (PL_savestack_ix != 0)
	    Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
			     "Unbalanced saves: %ld more saves than restores\n",
			     (long)PL_savestack_ix);
	if (PL_tmps_floor != -1)
	    Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),"Unbalanced tmps: %ld more allocs than frees\n",
			     (long)PL_tmps_floor + 1);
	if (cxstack_ix != -1)
	    Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),"Unbalanced context: %ld more PUSHes than POPs\n",
			     (long)cxstack_ix + 1);
    }

#ifdef PERL_IMPLICIT_CONTEXT
    /* the entries in this list are allocated via SV PVX's, so get freed
     * in sv_clean_all */
    Safefree(PL_my_cxt_list);
#endif

    /* Now absolutely destruct everything, somehow or other, loops or no. */

    /* the 2 is for PL_fdpid and PL_strtab */
    while (sv_clean_all() > 2)
	;

    AvREAL_off(PL_fdpid);		/* no surviving entries */
    SvREFCNT_dec(PL_fdpid);		/* needed in io_close() */
    PL_fdpid = NULL;

#ifdef HAVE_INTERP_INTERN
    sys_intern_clear();
#endif

    /* Destruct the global string table. */
    {
	/* Yell and reset the HeVAL() slots that are still holding refcounts,
	 * so that sv_free() won't fail on them.
	 * Now that the global string table is using a single hunk of memory
	 * for both HE and HEK, we either need to explicitly unshare it the
	 * correct way, or actually free things here.
	 */
	I32 riter = 0;
	const I32 max = HvMAX(PL_strtab);
	HE * const * const array = HvARRAY(PL_strtab);
	HE *hent = array[0];

	for (;;) {
	    if (hent && ckWARN_d(WARN_INTERNAL)) {
		HE * const next = HeNEXT(hent);
		Perl_warner(aTHX_ packWARN(WARN_INTERNAL),
		     "Unbalanced string table refcount: (%ld) for \"%s\"",
		     (long)hent->he_valu.hent_refcount, HeKEY(hent));
		Safefree(hent);
		hent = next;
	    }
	    if (!hent) {
		if (++riter > max)
		    break;
		hent = array[riter];
	    }
	}

	Safefree(array);
	HvARRAY(PL_strtab) = 0;
	HvTOTALKEYS(PL_strtab) = 0;
    }
    SvREFCNT_dec(PL_strtab);

#ifdef USE_ITHREADS
    /* free the pointer tables used for cloning */
    ptr_table_free(PL_ptr_table);
    PL_ptr_table = (PTR_TBL_t*)NULL;
#endif

    /* free special SVs */

    SvREFCNT(&PL_sv_yes) = 0;
    sv_clear(&PL_sv_yes);
    SvANY(&PL_sv_yes) = NULL;
    SvFLAGS(&PL_sv_yes) = 0;

    SvREFCNT(&PL_sv_no) = 0;
    sv_clear(&PL_sv_no);
    SvANY(&PL_sv_no) = NULL;
    SvFLAGS(&PL_sv_no) = 0;

    {
        int i;
        for (i=0; i<=2; i++) {
            SvREFCNT(PERL_DEBUG_PAD(i)) = 0;
            sv_clear(PERL_DEBUG_PAD(i));
            SvANY(PERL_DEBUG_PAD(i)) = NULL;
            SvFLAGS(PERL_DEBUG_PAD(i)) = 0;
        }
    }

    if (PL_sv_count != 0 && ckWARN_d(WARN_INTERNAL))
	Perl_warner(aTHX_ packWARN(WARN_INTERNAL),"Scalars leaked: %ld\n", (long)PL_sv_count);

#ifdef DEBUG_LEAKING_SCALARS
    if (PL_sv_count != 0) {
	SV* sva;
	SV* sv;
	register SV* svend;

	for (sva = PL_sv_arenaroot; sva; sva = MUTABLE_SV(SvANY(sva))) {
	    svend = &sva[SvREFCNT(sva)];
	    for (sv = sva + 1; sv < svend; ++sv) {
		if (SvTYPE(sv) != (svtype)SVTYPEMASK) {
		    PerlIO_printf(Perl_debug_log, "leaked: sv=0x%p"
			" flags=0x%"UVxf
			" refcnt=%"UVuf pTHX__FORMAT "\n"
			"\tallocated at %s:%d %s %s (parent 0x%"UVxf");"
			"serial %"UVuf"\n",
			(void*)sv, (UV)sv->sv_flags, (UV)sv->sv_refcnt
			pTHX__VALUE,
			sv->sv_debug_file ? sv->sv_debug_file : "(unknown)",
			sv->sv_debug_line,
			sv->sv_debug_inpad ? "for" : "by",
			sv->sv_debug_optype ?
			    PL_op_name[sv->sv_debug_optype]: "(none)",
			PTR2UV(sv->sv_debug_parent),
			sv->sv_debug_serial
		    );
#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
		    Perl_dump_sv_child(aTHX_ sv);
#endif
		}
	    }
	}
    }
#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
    {
	int status;
	fd_set rset;
	/* Wait for up to 4 seconds for child to terminate.
	   This seems to be the least effort way of timing out on reaping
	   its exit status.  */
	struct timeval waitfor = {4, 0};
	int sock = PL_dumper_fd;

	shutdown(sock, 1);
	FD_ZERO(&rset);
	FD_SET(sock, &rset);
	select(sock + 1, &rset, NULL, NULL, &waitfor);
	waitpid(child, &status, WNOHANG);
	close(sock);
    }
#endif
#endif
#ifdef DEBUG_LEAKING_SCALARS_ABORT
    if (PL_sv_count)
	abort();
#endif
    PL_sv_count = 0;

#ifdef PERL_DEBUG_READONLY_OPS
    free(PL_slabs);
    PL_slabs = NULL;
    PL_slab_count = 0;
#endif

#if defined(PERLIO_LAYERS)
    /* No more IO - including error messages ! */
    PerlIO_cleanup(aTHX);
#endif

    /* sv_undef needs to stay immortal until after PerlIO_cleanup
       as currently layers use it rather than NULL as a marker
       for no arg - and will try and SvREFCNT_dec it.
     */
    SvREFCNT(&PL_sv_undef) = 0;
    SvREADONLY_off(&PL_sv_undef);

    Safefree(PL_origfilename);
    PL_origfilename = NULL;
    Safefree(PL_reg_start_tmp);
    PL_reg_start_tmp = (char**)NULL;
    PL_reg_start_tmpl = 0;
    Safefree(PL_reg_curpm);
    Safefree(PL_reg_poscache);
    free_tied_hv_pool();
    Safefree(PL_op_mask);
    Safefree(PL_psig_name);
    PL_psig_name = (SV**)NULL;
    PL_psig_ptr = (SV**)NULL;
    {
	/* We need to NULL PL_psig_pend first, so that
	   signal handlers know not to use it */
	int *psig_save = PL_psig_pend;
	PL_psig_pend = (int*)NULL;
	Safefree(psig_save);
    }
    PL_formfeed = NULL;
    nuke_stacks();
    PL_tainting = FALSE;
    PL_taint_warn = FALSE;
    PL_hints = 0;		/* Reset hints. Should hints be per-interpreter ? */
    PL_debug = 0;

    DEBUG_P(debprofdump());

#ifdef USE_REENTRANT_API
    Perl_reentrant_free(aTHX);
#endif

    sv_free_arenas();

    while (PL_regmatch_slab) {
	regmatch_slab  *s = PL_regmatch_slab;
	PL_regmatch_slab = PL_regmatch_slab->next;
	Safefree(s);
    }

    /* As the absolutely last thing, free the non-arena SV for mess() */

    if (PL_mess_sv) {
	/* we know that type == SVt_PVMG */

	/* it could have accumulated taint magic */
	MAGIC* mg;
	MAGIC* moremagic;
	for (mg = SvMAGIC(PL_mess_sv); mg; mg = moremagic) {
	    moremagic = mg->mg_moremagic;
	    if (mg->mg_ptr && mg->mg_type != PERL_MAGIC_regex_global
		&& mg->mg_len >= 0)
		Safefree(mg->mg_ptr);
	    Safefree(mg);
	}

	/* we know that type >= SVt_PV */
	SvPV_free(PL_mess_sv);
	Safefree(SvANY(PL_mess_sv));
	Safefree(PL_mess_sv);
	PL_mess_sv = NULL;
    }
    return STATUS_EXIT;
}

/*
=for apidoc perl_free

Releases a Perl interpreter.  See L<perlembed>.

=cut
*/

void
perl_free(pTHXx)
{
    dVAR;

    PERL_ARGS_ASSERT_PERL_FREE;

    if (PL_veto_cleanup)
	return;

#ifdef PERL_TRACK_MEMPOOL
    {
	/*
	 * Don't free thread memory if PERL_DESTRUCT_LEVEL is set to a non-zero
	 * value as we're probably hunting memory leaks then
	 */
	const char * const s = PerlEnv_getenv("PERL_DESTRUCT_LEVEL");
	if (!s || atoi(s) == 0) {
	    const U32 old_debug = PL_debug;
	    /* Emulate the PerlHost behaviour of free()ing all memory allocated in this
	       thread at thread exit.  */
	    if (DEBUG_m_TEST) {
		PerlIO_puts(Perl_debug_log, "Disabling memory debugging as we "
			    "free this thread's memory\n");
		PL_debug &= ~ DEBUG_m_FLAG;
	    }
	    while(aTHXx->Imemory_debug_header.next != &(aTHXx->Imemory_debug_header))
		safesysfree(sTHX + (char *)(aTHXx->Imemory_debug_header.next));
	    PL_debug = old_debug;
	}
    }
#endif

#if defined(WIN32) || defined(NETWARE)
#  if defined(PERL_IMPLICIT_SYS)
    {
#    ifdef NETWARE
	void *host = nw_internal_host;
#    else
	void *host = w32_internal_host;
#    endif
	PerlMem_free(aTHXx);
#    ifdef NETWARE
	nw_delete_internal_host(host);
#    else
	win32_delete_internal_host(host);
#    endif
    }
#  else
    PerlMem_free(aTHXx);
#  endif
#else
    PerlMem_free(aTHXx);
#endif
}

#if defined(USE_ITHREADS)
/* provide destructors to clean up the thread key when libperl is unloaded */
#ifndef WIN32 /* handled during DLL_PROCESS_DETACH in win32/perllib.c */

#if defined(__hpux) && !(defined(__ux_version) && __ux_version <= 1020) && !defined(__GNUC__)
#pragma fini "perl_fini"
#elif defined(__sun) && !defined(__GNUC__)
#pragma fini (perl_fini)
#endif

static void
#if defined(__GNUC__)
__attribute__((destructor))
#endif
perl_fini(void)
{
    dVAR;
    if (PL_curinterp  && !PL_veto_cleanup)
	FREE_THREAD_KEY;
}

#endif /* WIN32 */
#endif /* THREADS */

void
Perl_call_atexit(pTHX_ ATEXIT_t fn, void *ptr)
{
    dVAR;
    Renew(PL_exitlist, PL_exitlistlen+1, PerlExitListEntry);
    PL_exitlist[PL_exitlistlen].fn = fn;
    PL_exitlist[PL_exitlistlen].ptr = ptr;
    ++PL_exitlistlen;
}

STATIC void
S_set_caret_X(pTHX) {
    dVAR;
    GV* tmpgv = gv_fetchpvs("\030", GV_ADD|GV_NOTQUAL, SVt_PV); /* $^X */
    if (tmpgv) {
	SV *const caret_x = GvSV(tmpgv);
#if defined(OS2)
	sv_setpv(caret_x, os2_execname(aTHX));
#else
#  ifdef USE_KERN_PROC_PATHNAME
	size_t size = 0;
	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = -1;

	if (sysctl(mib, 4, NULL, &size, NULL, 0) == 0
	    && size > 0 && size < MAXPATHLEN * MAXPATHLEN) {
	    sv_grow(caret_x, size);

	    if (sysctl(mib, 4, SvPVX(caret_x), &size, NULL, 0) == 0
		&& size > 2) {
		SvPOK_only(caret_x);
		SvCUR_set(caret_x, size - 1);
		SvTAINT(caret_x);
		return;
	    }
	}
#  elif defined(USE_NSGETEXECUTABLEPATH)
	char buf[1];
	uint32_t size = sizeof(buf);

	_NSGetExecutablePath(buf, &size);
	if (size < MAXPATHLEN * MAXPATHLEN) {
	    sv_grow(caret_x, size);
	    if (_NSGetExecutablePath(SvPVX(caret_x), &size) == 0) {
		char *const tidied = realpath(SvPVX(caret_x), NULL);
		if (tidied) {
		    sv_setpv(caret_x, tidied);
		    free(tidied);
		} else {
		    SvPOK_only(caret_x);
		    SvCUR_set(caret_x, size);
		}
		return;
	    }
	}
#  elif defined(HAS_PROCSELFEXE)
	char buf[MAXPATHLEN];
	int len = readlink(PROCSELFEXE_PATH, buf, sizeof(buf) - 1);

	/* On Playstation2 Linux V1.0 (kernel 2.2.1) readlink(/proc/self/exe)
	   includes a spurious NUL which will cause $^X to fail in system
	   or backticks (this will prevent extensions from being built and
	   many tests from working). readlink is not meant to add a NUL.
	   Normal readlink works fine.
	*/
	if (len > 0 && buf[len-1] == '\0') {
	    len--;
	}

	/* FreeBSD's implementation is acknowledged to be imperfect, sometimes
	   returning the text "unknown" from the readlink rather than the path
	   to the executable (or returning an error from the readlink). Any
	   valid path has a '/' in it somewhere, so use that to validate the
	   result. See http://www.freebsd.org/cgi/query-pr.cgi?pr=35703
	*/
	if (len > 0 && memchr(buf, '/', len)) {
	    sv_setpvn(caret_x, buf, len);
	    return;
	}
#  endif
	/* Fallback to this:  */
	sv_setpv(caret_x, PL_origargv[0]);
#endif
    }
}

/*
=for apidoc perl_parse

Tells a Perl interpreter to parse a Perl script.  See L<perlembed>.

=cut
*/

#define SET_CURSTASH(newstash)                       \
	if (PL_curstash != newstash) {                \
	    SvREFCNT_dec(PL_curstash);                 \
	    PL_curstash = (HV *)SvREFCNT_inc(newstash); \
	}

int
perl_parse(pTHXx_ XSINIT_t xsinit, int argc, char **argv, char **env)
{
    dVAR;
    I32 oldscope;
    int ret;
    dJMPENV;

    PERL_ARGS_ASSERT_PERL_PARSE;
#ifndef MULTIPLICITY
    PERL_UNUSED_ARG(my_perl);
#endif

#if defined(USE_HASH_SEED) || defined(USE_HASH_SEED_EXPLICIT)
    /* [perl #22371] Algorimic Complexity Attack on Perl 5.6.1, 5.8.0
     * This MUST be done before any hash stores or fetches take place.
     * If you set PL_rehash_seed (and presumably also PL_rehash_seed_set)
     * yourself, it is your responsibility to provide a good random seed!
     * You can also define PERL_HASH_SEED in compile time, see hv.h. */
    if (!PL_rehash_seed_set)
	 PL_rehash_seed = get_hash_seed();
    {
	const char * const s = PerlEnv_getenv("PERL_HASH_SEED_DEBUG");

	if (s && (atoi(s) == 1))
	    PerlIO_printf(Perl_debug_log, "HASH_SEED = %"UVuf"\n", PL_rehash_seed);
    }
#endif /* #if defined(USE_HASH_SEED) || defined(USE_HASH_SEED_EXPLICIT) */

    PL_origargc = argc;
    PL_origargv = argv;

    if (PL_origalen != 0) {
	PL_origalen = 1; /* don't use old PL_origalen if perl_parse() is called again */
    }
    else {
	/* Set PL_origalen be the sum of the contiguous argv[]
	 * elements plus the size of the env in case that it is
	 * contiguous with the argv[].  This is used in mg.c:Perl_magic_set()
	 * as the maximum modifiable length of $0.  In the worst case
	 * the area we are able to modify is limited to the size of
	 * the original argv[0].  (See below for 'contiguous', though.)
	 * --jhi */
	 const char *s = NULL;
	 int i;
	 const UV mask =
	   ~(UV)(PTRSIZE == 4 ? 3 : PTRSIZE == 8 ? 7 : PTRSIZE == 16 ? 15 : 0);
         /* Do the mask check only if the args seem like aligned. */
	 const UV aligned =
	   (mask < ~(UV)0) && ((PTR2UV(argv[0]) & mask) == PTR2UV(argv[0]));

	 /* See if all the arguments are contiguous in memory.  Note
	  * that 'contiguous' is a loose term because some platforms
	  * align the argv[] and the envp[].  If the arguments look
	  * like non-aligned, assume that they are 'strictly' or
	  * 'traditionally' contiguous.  If the arguments look like
	  * aligned, we just check that they are within aligned
	  * PTRSIZE bytes.  As long as no system has something bizarre
	  * like the argv[] interleaved with some other data, we are
	  * fine.  (Did I just evoke Murphy's Law?)  --jhi */
	 if (PL_origargv && PL_origargc >= 1 && (s = PL_origargv[0])) {
	      while (*s) s++;
	      for (i = 1; i < PL_origargc; i++) {
		   if ((PL_origargv[i] == s + 1
#ifdef OS2
			|| PL_origargv[i] == s + 2
#endif
			    )
		       ||
		       (aligned &&
			(PL_origargv[i] >  s &&
			 PL_origargv[i] <=
			 INT2PTR(char *, PTR2UV(s + PTRSIZE) & mask)))
			)
		   {
			s = PL_origargv[i];
			while (*s) s++;
		   }
		   else
			break;
	      }
	 }

#ifndef PERL_USE_SAFE_PUTENV
	 /* Can we grab env area too to be used as the area for $0? */
	 if (s && PL_origenviron && !PL_use_safe_putenv) {
	      if ((PL_origenviron[0] == s + 1)
		  ||
		  (aligned &&
		   (PL_origenviron[0] >  s &&
		    PL_origenviron[0] <=
		    INT2PTR(char *, PTR2UV(s + PTRSIZE) & mask)))
		 )
	      {
#ifndef OS2		/* ENVIRON is read by the kernel too. */
		   s = PL_origenviron[0];
		   while (*s) s++;
#endif
		   my_setenv("NoNe  SuCh", NULL);
		   /* Force copy of environment. */
		   for (i = 1; PL_origenviron[i]; i++) {
			if (PL_origenviron[i] == s + 1
			    ||
			    (aligned &&
			     (PL_origenviron[i] >  s &&
			      PL_origenviron[i] <=
			      INT2PTR(char *, PTR2UV(s + PTRSIZE) & mask)))
			   )
			{
			     s = PL_origenviron[i];
			     while (*s) s++;
			}
			else
			     break;
		   }
	      }
	 }
#endif /* !defined(PERL_USE_SAFE_PUTENV) */

	 PL_origalen = s ? s - PL_origargv[0] + 1 : 0;
    }

    if (PL_do_undump) {

	/* Come here if running an undumped a.out. */

	PL_origfilename = savepv(argv[0]);
	PL_do_undump = FALSE;
	cxstack_ix = -1;		/* start label stack again */
	init_ids();
	assert (!PL_tainted);
	TAINT;
	S_set_caret_X(aTHX);
	TAINT_NOT;
	init_postdump_symbols(argc,argv,env);
	return 0;
    }

    if (PL_main_root) {
	op_free(PL_main_root);
	PL_main_root = NULL;
    }
    PL_main_start = NULL;
    SvREFCNT_dec(PL_main_cv);
    PL_main_cv = NULL;

    time(&PL_basetime);
    oldscope = PL_scopestack_ix;
    PL_dowarn = G_WARN_OFF;

    JMPENV_PUSH(ret);
    switch (ret) {
    case 0:
	parse_body(env,xsinit);
	if (PL_unitcheckav) {
	    call_list(oldscope, PL_unitcheckav);
	}
	if (PL_checkav) {
	    PERL_SET_PHASE(PERL_PHASE_CHECK);
	    call_list(oldscope, PL_checkav);
	}
	ret = 0;
	break;
    case 1:
	STATUS_ALL_FAILURE;
	/* FALL THROUGH */
    case 2:
	/* my_exit() was called */
	while (PL_scopestack_ix > oldscope)
	    LEAVE;
	FREETMPS;
	SET_CURSTASH(PL_defstash);
	if (PL_unitcheckav) {
	    call_list(oldscope, PL_unitcheckav);
	}
	if (PL_checkav) {
	    PERL_SET_PHASE(PERL_PHASE_CHECK);
	    call_list(oldscope, PL_checkav);
	}
	ret = STATUS_EXIT;
	break;
    case 3:
	PerlIO_printf(Perl_error_log, "panic: top_env\n");
	ret = 1;
	break;
    }
    JMPENV_POP;
    return ret;
}

/* This needs to stay in perl.c, as perl.c is compiled with different flags for
   miniperl, and we need to see those flags reflected in the values here.  */

/* What this returns is subject to change.  Use the public interface in Config.
 */
static void
S_Internals_V(pTHX_ CV *cv)
{
    dXSARGS;
#ifdef LOCAL_PATCH_COUNT
    const int local_patch_count = LOCAL_PATCH_COUNT;
#else
    const int local_patch_count = 0;
#endif
    const int entries = 3 + local_patch_count;
    int i;
    static char non_bincompat_options[] =
#  ifdef DEBUGGING
			     " DEBUGGING"
#  endif
#  ifdef NO_MATHOMS
			     " NO_MATHOMS"
#  endif
#  ifdef PERL_DISABLE_PMC
			     " PERL_DISABLE_PMC"
#  endif
#  ifdef PERL_DONT_CREATE_GVSV
			     " PERL_DONT_CREATE_GVSV"
#  endif
#  ifdef PERL_EXTERNAL_GLOB
			     " PERL_EXTERNAL_GLOB"
#  endif
#  ifdef PERL_IS_MINIPERL
			     " PERL_IS_MINIPERL"
#  endif
#  ifdef PERL_MALLOC_WRAP
			     " PERL_MALLOC_WRAP"
#  endif
#  ifdef PERL_MEM_LOG
			     " PERL_MEM_LOG"
#  endif
#  ifdef PERL_MEM_LOG_NOIMPL
			     " PERL_MEM_LOG_NOIMPL"
#  endif
#  ifdef PERL_PRESERVE_IVUV
			     " PERL_PRESERVE_IVUV"
#  endif
#  ifdef PERL_RELOCATABLE_INCPUSH
			     " PERL_RELOCATABLE_INCPUSH"
#  endif
#  ifdef PERL_USE_DEVEL
			     " PERL_USE_DEVEL"
#  endif
#  ifdef PERL_USE_SAFE_PUTENV
			     " PERL_USE_SAFE_PUTENV"
#  endif
#  ifdef UNLINK_ALL_VERSIONS
			     " UNLINK_ALL_VERSIONS"
#  endif
#  ifdef USE_ATTRIBUTES_FOR_PERLIO
			     " USE_ATTRIBUTES_FOR_PERLIO"
#  endif
#  ifdef USE_FAST_STDIO
			     " USE_FAST_STDIO"
#  endif
#  ifdef USE_LOCALE
			     " USE_LOCALE"
#  endif
#  ifdef USE_LOCALE_CTYPE
			     " USE_LOCALE_CTYPE"
#  endif
#  ifdef USE_PERL_ATOF
			     " USE_PERL_ATOF"
#  endif
#  ifdef USE_SITECUSTOMIZE
			     " USE_SITECUSTOMIZE"
#  endif
	;
    PERL_UNUSED_ARG(cv);
    PERL_UNUSED_ARG(items);

    EXTEND(SP, entries);

    PUSHs(sv_2mortal(newSVpv(PL_bincompat_options, 0)));
    PUSHs(Perl_newSVpvn_flags(aTHX_ non_bincompat_options,
			      sizeof(non_bincompat_options) - 1, SVs_TEMP));

#ifdef __DATE__
#  ifdef __TIME__
    PUSHs(Perl_newSVpvn_flags(aTHX_
			      STR_WITH_LEN("Compiled at " __DATE__ " " __TIME__),
			      SVs_TEMP));
#  else
    PUSHs(Perl_newSVpvn_flags(aTHX_ STR_WITH_LEN("Compiled on " __DATE__),
			      SVs_TEMP));
#  endif
#else
    PUSHs(&PL_sv_undef);
#endif

    for (i = 1; i <= local_patch_count; i++) {
	/* This will be an undef, if PL_localpatches[i] is NULL.  */
	PUSHs(sv_2mortal(newSVpv(PL_localpatches[i], 0)));
    }

    XSRETURN(entries);
}

#define INCPUSH_UNSHIFT			0x01
#define INCPUSH_ADD_OLD_VERS		0x02
#define INCPUSH_ADD_VERSIONED_SUB_DIRS	0x04
#define INCPUSH_ADD_ARCHONLY_SUB_DIRS	0x08
#define INCPUSH_NOT_BASEDIR		0x10
#define INCPUSH_CAN_RELOCATE		0x20
#define INCPUSH_ADD_SUB_DIRS	\
    (INCPUSH_ADD_VERSIONED_SUB_DIRS|INCPUSH_ADD_ARCHONLY_SUB_DIRS)

STATIC void *
S_parse_body(pTHX_ char **env, XSINIT_t xsinit)
{
    dVAR;
    PerlIO *rsfp;
    int argc = PL_origargc;
    char **argv = PL_origargv;
    const char *scriptname = NULL;
    VOL bool dosearch = FALSE;
    register char c;
    bool doextract = FALSE;
    const char *cddir = NULL;
#ifdef USE_SITECUSTOMIZE
    bool minus_f = FALSE;
#endif
    SV *linestr_sv = NULL;
    bool add_read_e_script = FALSE;
    U32 lex_start_flags = 0;

    PERL_SET_PHASE(PERL_PHASE_START);

    init_main_stash();

    {
	const char *s;
    for (argc--,argv++; argc > 0; argc--,argv++) {
	if (argv[0][0] != '-' || !argv[0][1])
	    break;
	s = argv[0]+1;
      reswitch:
	switch ((c = *s)) {
	case 'C':
#ifndef PERL_STRICT_CR
	case '\r':
#endif
	case ' ':
	case '0':
	case 'F':
	case 'a':
	case 'c':
	case 'd':
	case 'D':
	case 'h':
	case 'i':
	case 'l':
	case 'M':
	case 'm':
	case 'n':
	case 'p':
	case 's':
	case 'u':
	case 'U':
	case 'v':
	case 'W':
	case 'X':
	case 'w':
	    if ((s = moreswitches(s)))
		goto reswitch;
	    break;

	case 't':
	    CHECK_MALLOC_TOO_LATE_FOR('t');
	    if( !PL_tainting ) {
	         PL_taint_warn = TRUE;
	         PL_tainting = TRUE;
	    }
	    s++;
	    goto reswitch;
	case 'T':
	    CHECK_MALLOC_TOO_LATE_FOR('T');
	    PL_tainting = TRUE;
	    PL_taint_warn = FALSE;
	    s++;
	    goto reswitch;

	case 'E':
	    PL_minus_E = TRUE;
	    /* FALL THROUGH */
	case 'e':
	    forbid_setid('e', FALSE);
	    if (!PL_e_script) {
		PL_e_script = newSVpvs("");
		add_read_e_script = TRUE;
	    }
	    if (*++s)
		sv_catpv(PL_e_script, s);
	    else if (argv[1]) {
		sv_catpv(PL_e_script, argv[1]);
		argc--,argv++;
	    }
	    else
		Perl_croak(aTHX_ "No code specified for -%c", c);
	    sv_catpvs(PL_e_script, "\n");
	    break;

	case 'f':
#ifdef USE_SITECUSTOMIZE
	    minus_f = TRUE;
#endif
	    s++;
	    goto reswitch;

	case 'I':	/* -I handled both here and in moreswitches() */
	    forbid_setid('I', FALSE);
	    if (!*++s && (s=argv[1]) != NULL) {
		argc--,argv++;
	    }
	    if (s && *s) {
		STRLEN len = strlen(s);
		incpush(s, len, INCPUSH_ADD_SUB_DIRS|INCPUSH_ADD_OLD_VERS);
	    }
	    else
		Perl_croak(aTHX_ "No directory specified for -I");
	    break;
	case 'S':
	    forbid_setid('S', FALSE);
	    dosearch = TRUE;
	    s++;
	    goto reswitch;
	case 'V':
	    {
		SV *opts_prog;

		if (*++s != ':')  {
		    opts_prog = newSVpvs("use Config; Config::_V()");
		}
		else {
		    ++s;
		    opts_prog = Perl_newSVpvf(aTHX_
					      "use Config; Config::config_vars(qw%c%s%c)",
					      0, s, 0);
		    s += strlen(s);
		}
		Perl_av_create_and_push(aTHX_ &PL_preambleav, opts_prog);
		/* don't look for script or read stdin */
		scriptname = BIT_BUCKET;
		goto reswitch;
	    }
	case 'x':
	    doextract = TRUE;
	    s++;
	    if (*s)
		cddir = s;
	    break;
	case 0:
	    break;
	case '-':
	    if (!*++s || isSPACE(*s)) {
		argc--,argv++;
		goto switch_end;
	    }
	    /* catch use of gnu style long options.
	       Both of these exit immediately.  */
	    if (strEQ(s, "version"))
		minus_v();
	    if (strEQ(s, "help"))
		usage();
	    s--;
	    /* FALL THROUGH */
	default:
	    Perl_croak(aTHX_ "Unrecognized switch: -%s  (-h will show valid options)",s);
	}
    }
    }

  switch_end:

    {
	char *s;

    if (
#ifndef SECURE_INTERNAL_GETENV
        !PL_tainting &&
#endif
	(s = PerlEnv_getenv("PERL5OPT")))
    {
	while (isSPACE(*s))
	    s++;
	if (*s == '-' && *(s+1) == 'T') {
	    CHECK_MALLOC_TOO_LATE_FOR('T');
	    PL_tainting = TRUE;
            PL_taint_warn = FALSE;
	}
	else {
	    char *popt_copy = NULL;
	    while (s && *s) {
	        const char *d;
		while (isSPACE(*s))
		    s++;
		if (*s == '-') {
		    s++;
		    if (isSPACE(*s))
			continue;
		}
		d = s;
		if (!*s)
		    break;
		if (!strchr("CDIMUdmtwW", *s))
		    Perl_croak(aTHX_ "Illegal switch in PERL5OPT: -%c", *s);
		while (++s && *s) {
		    if (isSPACE(*s)) {
			if (!popt_copy) {
			    popt_copy = SvPVX(sv_2mortal(newSVpv(d,0)));
			    s = popt_copy + (s - d);
			    d = popt_copy;
			}
		        *s++ = '\0';
			break;
		    }
		}
		if (*d == 't') {
		    if( !PL_tainting ) {
		        PL_taint_warn = TRUE;
		        PL_tainting = TRUE;
		    }
		} else {
		    moreswitches(d);
		}
	    }
	}
    }
    }

    /* Set $^X early so that it can be used for relocatable paths in @INC  */
    /* and for SITELIB_EXP in USE_SITECUSTOMIZE                            */
    assert (!PL_tainted);
    TAINT;
    S_set_caret_X(aTHX);
    TAINT_NOT;

#if defined(USE_SITECUSTOMIZE)
    if (!minus_f) {
	/* The games with local $! are to avoid setting errno if there is no
	   sitecustomize script.  "q%c...%c", 0, ..., 0 becomes "q\0...\0",
	   ie a q() operator with a NUL byte as a the delimiter. This avoids
	   problems with pathnames containing (say) '  */
#  ifdef PERL_IS_MINIPERL
	AV *const inc = GvAV(PL_incgv);
	SV **const inc0 = inc ? av_fetch(inc, 0, FALSE) : NULL;

	if (inc0) {
	    (void)Perl_av_create_and_unshift_one(aTHX_ &PL_preambleav,
						 Perl_newSVpvf(aTHX_
							       "BEGIN { do {local $!; -f q%c%"SVf"/buildcustomize.pl%c} && do q%c%"SVf"/buildcustomize.pl%c }",
							       0, *inc0, 0,
							       0, *inc0, 0));
	}
#  else
	/* SITELIB_EXP is a function call on Win32.  */
	const char *const raw_sitelib = SITELIB_EXP;
	if (raw_sitelib) {
	    /* process .../.. if PERL_RELOCATABLE_INC is defined */
	    SV *sitelib_sv = mayberelocate(raw_sitelib, strlen(raw_sitelib),
					   INCPUSH_CAN_RELOCATE);
	    const char *const sitelib = SvPVX(sitelib_sv);
	    (void)Perl_av_create_and_unshift_one(aTHX_ &PL_preambleav,
						 Perl_newSVpvf(aTHX_
							       "BEGIN { do {local $!; -f q%c%s/sitecustomize.pl%c} && do q%c%s/sitecustomize.pl%c }",
							       0, sitelib, 0,
							       0, sitelib, 0));
	    assert (SvREFCNT(sitelib_sv) == 1);
	    SvREFCNT_dec(sitelib_sv);
	}
#  endif
    }
#endif

    if (!scriptname)
	scriptname = argv[0];
    if (PL_e_script) {
	argc++,argv--;
	scriptname = BIT_BUCKET;	/* don't look for script or read stdin */
    }
    else if (scriptname == NULL) {
#ifdef MSDOS
	if ( PerlLIO_isatty(PerlIO_fileno(PerlIO_stdin())) )
	    moreswitches("h");
#endif
	scriptname = "-";
    }

    assert (!PL_tainted);
    init_perllib();

    {
	bool suidscript = FALSE;

	rsfp = open_script(scriptname, dosearch, &suidscript);
	if (!rsfp) {
	    rsfp = PerlIO_stdin();
	    lex_start_flags = LEX_DONT_CLOSE_RSFP;
	}

	validate_suid(rsfp);

#ifndef PERL_MICRO
#  if defined(SIGCHLD) || defined(SIGCLD)
	{
#  ifndef SIGCHLD
#    define SIGCHLD SIGCLD
#  endif
	    Sighandler_t sigstate = rsignal_state(SIGCHLD);
	    if (sigstate == (Sighandler_t) SIG_IGN) {
		Perl_ck_warner(aTHX_ packWARN(WARN_SIGNAL),
			       "Can't ignore signal CHLD, forcing to default");
		(void)rsignal(SIGCHLD, (Sighandler_t)SIG_DFL);
	    }
	}
#  endif
#endif

	if (doextract) {

	    /* This will croak if suidscript is true, as -x cannot be used with
	       setuid scripts.  */
	    forbid_setid('x', suidscript);
	    /* Hence you can't get here if suidscript is true */

	    linestr_sv = newSV_type(SVt_PV);
	    lex_start_flags |= LEX_START_COPIED;
	    find_beginning(linestr_sv, rsfp);
	    if (cddir && PerlDir_chdir( (char *)cddir ) < 0)
		Perl_croak(aTHX_ "Can't chdir to %s",cddir);
	}
    }

    PL_main_cv = PL_compcv = MUTABLE_CV(newSV_type(SVt_PVCV));
    CvUNIQUE_on(PL_compcv);

    CvPADLIST(PL_compcv) = pad_new(0);

    PL_isarev = newHV();

    boot_core_PerlIO();
    boot_core_UNIVERSAL();
    boot_core_mro();
    newXS("Internals::V", S_Internals_V, __FILE__);

    if (xsinit)
	(*xsinit)(aTHX);	/* in case linked C routines want magical variables */
#ifndef PERL_MICRO
#if defined(VMS) || defined(WIN32) || defined(DJGPP) || defined(__CYGWIN__) || defined(EPOC) || defined(SYMBIAN)
    init_os_extras();
#endif
#endif

#ifdef USE_SOCKS
#   ifdef HAS_SOCKS5_INIT
    socks5_init(argv[0]);
#   else
    SOCKSinit(argv[0]);
#   endif
#endif

    init_predump_symbols();
    /* init_postdump_symbols not currently designed to be called */
    /* more than once (ENV isn't cleared first, for example)	 */
    /* But running with -u leaves %ENV & @ARGV undefined!    XXX */
    if (!PL_do_undump)
	init_postdump_symbols(argc,argv,env);

    /* PL_unicode is turned on by -C, or by $ENV{PERL_UNICODE},
     * or explicitly in some platforms.
     * locale.c:Perl_init_i18nl10n() if the environment
     * look like the user wants to use UTF-8. */
#if defined(__SYMBIAN32__)
    PL_unicode = PERL_UNICODE_STD_FLAG; /* See PERL_SYMBIAN_CONSOLE_UTF8. */
#endif
#  ifndef PERL_IS_MINIPERL
    if (PL_unicode) {
	 /* Requires init_predump_symbols(). */
	 if (!(PL_unicode & PERL_UNICODE_LOCALE_FLAG) || PL_utf8locale) {
	      IO* io;
	      PerlIO* fp;
	      SV* sv;

	      /* Turn on UTF-8-ness on STDIN, STDOUT, STDERR
	       * and the default open disciplines. */
	      if ((PL_unicode & PERL_UNICODE_STDIN_FLAG) &&
		  PL_stdingv  && (io = GvIO(PL_stdingv)) &&
		  (fp = IoIFP(io)))
		   PerlIO_binmode(aTHX_ fp, IoTYPE(io), 0, ":utf8");
	      if ((PL_unicode & PERL_UNICODE_STDOUT_FLAG) &&
		  PL_defoutgv && (io = GvIO(PL_defoutgv)) &&
		  (fp = IoOFP(io)))
		   PerlIO_binmode(aTHX_ fp, IoTYPE(io), 0, ":utf8");
	      if ((PL_unicode & PERL_UNICODE_STDERR_FLAG) &&
		  PL_stderrgv && (io = GvIO(PL_stderrgv)) &&
		  (fp = IoOFP(io)))
		   PerlIO_binmode(aTHX_ fp, IoTYPE(io), 0, ":utf8");
	      if ((PL_unicode & PERL_UNICODE_INOUT_FLAG) &&
		  (sv = GvSV(gv_fetchpvs("\017PEN", GV_ADD|GV_NOTQUAL,
					 SVt_PV)))) {
		   U32 in  = PL_unicode & PERL_UNICODE_IN_FLAG;
		   U32 out = PL_unicode & PERL_UNICODE_OUT_FLAG;
		   if (in) {
			if (out)
			     sv_setpvs(sv, ":utf8\0:utf8");
			else
			     sv_setpvs(sv, ":utf8\0");
		   }
		   else if (out)
			sv_setpvs(sv, "\0:utf8");
		   SvSETMAGIC(sv);
	      }
	 }
    }
#endif

    {
	const char *s;
    if ((s = PerlEnv_getenv("PERL_SIGNALS"))) {
	 if (strEQ(s, "unsafe"))
	      PL_signals |=  PERL_SIGNALS_UNSAFE_FLAG;
	 else if (strEQ(s, "safe"))
	      PL_signals &= ~PERL_SIGNALS_UNSAFE_FLAG;
	 else
	      Perl_croak(aTHX_ "PERL_SIGNALS illegal: \"%s\"", s);
    }
    }

#ifdef PERL_MAD
    {
	const char *s;
    if ((s = PerlEnv_getenv("PERL_XMLDUMP"))) {
	PL_madskills = 1;
	PL_minus_c = 1;
	if (!s || !s[0])
	    PL_xmlfp = PerlIO_stdout();
	else {
	    PL_xmlfp = PerlIO_open(s, "w");
	    if (!PL_xmlfp)
		Perl_croak(aTHX_ "Can't open %s", s);
	}
	my_setenv("PERL_XMLDUMP", NULL);	/* hide from subprocs */
    }
    }

    {
	const char *s;
    if ((s = PerlEnv_getenv("PERL_MADSKILLS"))) {
	PL_madskills = atoi(s);
	my_setenv("PERL_MADSKILLS", NULL);	/* hide from subprocs */
    }
    }
#endif

    lex_start(linestr_sv, rsfp, lex_start_flags);
    if(linestr_sv)
	SvREFCNT_dec(linestr_sv);

    PL_subname = newSVpvs("main");

    if (add_read_e_script)
	filter_add(read_e_script, NULL);

    /* now parse the script */

    SETERRNO(0,SS_NORMAL);
    if (yyparse(GRAMPROG) || PL_parser->error_count) {
	if (PL_minus_c)
	    Perl_croak(aTHX_ "%s had compilation errors.\n", PL_origfilename);
	else {
	    Perl_croak(aTHX_ "Execution of %s aborted due to compilation errors.\n",
		       PL_origfilename);
	}
    }
    CopLINE_set(PL_curcop, 0);
    SET_CURSTASH(PL_defstash);
    if (PL_e_script) {
	SvREFCNT_dec(PL_e_script);
	PL_e_script = NULL;
    }

    if (PL_do_undump)
	my_unexec();

    if (isWARN_ONCE) {
	SAVECOPFILE(PL_curcop);
	SAVECOPLINE(PL_curcop);
	gv_check(PL_defstash);
    }

    LEAVE;
    FREETMPS;

#ifdef MYMALLOC
    {
	const char *s;
    if ((s=PerlEnv_getenv("PERL_DEBUG_MSTATS")) && atoi(s) >= 2)
	dump_mstats("after compilation:");
    }
#endif

    ENTER;
    PL_restartjmpenv = NULL;
    PL_restartop = 0;
    return NULL;
}

/*
=for apidoc perl_run

Tells a Perl interpreter to run.  See L<perlembed>.

=cut
*/

int
perl_run(pTHXx)
{
    dVAR;
    I32 oldscope;
    int ret = 0;
    dJMPENV;

    PERL_ARGS_ASSERT_PERL_RUN;
#ifndef MULTIPLICITY
    PERL_UNUSED_ARG(my_perl);
#endif

    oldscope = PL_scopestack_ix;
#ifdef VMS
    VMSISH_HUSHED = 0;
#endif

    JMPENV_PUSH(ret);
    switch (ret) {
    case 1:
	cxstack_ix = -1;		/* start context stack again */
	goto redo_body;
    case 0:				/* normal completion */
 redo_body:
	run_body(oldscope);
	/* FALL THROUGH */
    case 2:				/* my_exit() */
	while (PL_scopestack_ix > oldscope)
	    LEAVE;
	FREETMPS;
	SET_CURSTASH(PL_defstash);
	if (!(PL_exit_flags & PERL_EXIT_DESTRUCT_END) &&
	    PL_endav && !PL_minus_c) {
	    PERL_SET_PHASE(PERL_PHASE_END);
	    call_list(oldscope, PL_endav);
	}
#ifdef MYMALLOC
	if (PerlEnv_getenv("PERL_DEBUG_MSTATS"))
	    dump_mstats("after execution:  ");
#endif
	ret = STATUS_EXIT;
	break;
    case 3:
	if (PL_restartop) {
	    POPSTACK_TO(PL_mainstack);
	    goto redo_body;
	}
	PerlIO_printf(Perl_error_log, "panic: restartop in perl_run\n");
	FREETMPS;
	ret = 1;
	break;
    }

    JMPENV_POP;
    return ret;
}

STATIC void
S_run_body(pTHX_ I32 oldscope)
{
    dVAR;
    DEBUG_r(PerlIO_printf(Perl_debug_log, "%s $` $& $' support.\n",
                    PL_sawampersand ? "Enabling" : "Omitting"));

    if (!PL_restartop) {
#ifdef PERL_MAD
	if (PL_xmlfp) {
	    xmldump_all();
	    exit(0);	/* less likely to core dump than my_exit(0) */
	}
#endif
#ifdef DEBUGGING
	if (DEBUG_x_TEST || DEBUG_B_TEST)
	    dump_all_perl(!DEBUG_B_TEST);
	if (!DEBUG_q_TEST)
	  PERL_DEBUG(PerlIO_printf(Perl_debug_log, "\nEXECUTING...\n\n"));
#endif

	if (PL_minus_c) {
	    PerlIO_printf(Perl_error_log, "%s syntax OK\n", PL_origfilename);
	    my_exit(0);
	}
	if (PERLDB_SINGLE && PL_DBsingle)
	    sv_setiv(PL_DBsingle, 1);
	if (PL_initav) {
	    PERL_SET_PHASE(PERL_PHASE_INIT);
	    call_list(oldscope, PL_initav);
	}
#ifdef PERL_DEBUG_READONLY_OPS
	Perl_pending_Slabs_to_ro(aTHX);
#endif
    }

    /* do it */

    PERL_SET_PHASE(PERL_PHASE_RUN);

    if (PL_restartop) {
	PL_restartjmpenv = NULL;
	PL_op = PL_restartop;
	PL_restartop = 0;
	CALLRUNOPS(aTHX);
    }
    else if (PL_main_start) {
	CvDEPTH(PL_main_cv) = 1;
	PL_op = PL_main_start;
	CALLRUNOPS(aTHX);
    }
    my_exit(0);
    /* NOTREACHED */
}

/*
=head1 SV Manipulation Functions

=for apidoc p||get_sv

Returns the SV of the specified Perl scalar.  C<flags> are passed to
C<gv_fetchpv>. If C<GV_ADD> is set and the
Perl variable does not exist then it will be created.  If C<flags> is zero
and the variable does not exist then NULL is returned.

=cut
*/

SV*
Perl_get_sv(pTHX_ const char *name, I32 flags)
{
    GV *gv;

    PERL_ARGS_ASSERT_GET_SV;

    gv = gv_fetchpv(name, flags, SVt_PV);
    if (gv)
	return GvSV(gv);
    return NULL;
}

/*
=head1 Array Manipulation Functions

=for apidoc p||get_av

Returns the AV of the specified Perl global or package array with the given
name (so it won't work on lexical variables).  C<flags> are passed
to C<gv_fetchpv>. If C<GV_ADD> is set and the
Perl variable does not exist then it will be created.  If C<flags> is zero
and the variable does not exist then NULL is returned.

Perl equivalent: C<@{"$name"}>.

=cut
*/

AV*
Perl_get_av(pTHX_ const char *name, I32 flags)
{
    GV* const gv = gv_fetchpv(name, flags, SVt_PVAV);

    PERL_ARGS_ASSERT_GET_AV;

    if (flags)
    	return GvAVn(gv);
    if (gv)
	return GvAV(gv);
    return NULL;
}

/*
=head1 Hash Manipulation Functions

=for apidoc p||get_hv

Returns the HV of the specified Perl hash.  C<flags> are passed to
C<gv_fetchpv>. If C<GV_ADD> is set and the
Perl variable does not exist then it will be created.  If C<flags> is zero
and the variable does not exist then NULL is returned.

=cut
*/

HV*
Perl_get_hv(pTHX_ const char *name, I32 flags)
{
    GV* const gv = gv_fetchpv(name, flags, SVt_PVHV);

    PERL_ARGS_ASSERT_GET_HV;

    if (flags)
    	return GvHVn(gv);
    if (gv)
	return GvHV(gv);
    return NULL;
}

/*
=head1 CV Manipulation Functions

=for apidoc p||get_cvn_flags

Returns the CV of the specified Perl subroutine.  C<flags> are passed to
C<gv_fetchpvn_flags>. If C<GV_ADD> is set and the Perl subroutine does not
exist then it will be declared (which has the same effect as saying
C<sub name;>).  If C<GV_ADD> is not set and the subroutine does not exist
then NULL is returned.

=for apidoc p||get_cv

Uses C<strlen> to get the length of C<name>, then calls C<get_cvn_flags>.

=cut
*/

CV*
Perl_get_cvn_flags(pTHX_ const char *name, STRLEN len, I32 flags)
{
    GV* const gv = gv_fetchpvn_flags(name, len, flags, SVt_PVCV);
    /* XXX this is probably not what they think they're getting.
     * It has the same effect as "sub name;", i.e. just a forward
     * declaration! */

    PERL_ARGS_ASSERT_GET_CVN_FLAGS;

    if ((flags & ~GV_NOADD_MASK) && !GvCVu(gv)) {
	SV *const sv = newSVpvn_flags(name, len, flags & SVf_UTF8);
    	return newSUB(start_subparse(FALSE, 0),
		      newSVOP(OP_CONST, 0, sv),
		      NULL, NULL);
    }
    if (gv)
	return GvCVu(gv);
    return NULL;
}

/* Nothing in core calls this now, but we can't replace it with a macro and
   move it to mathoms.c as a macro would evaluate name twice.  */
CV*
Perl_get_cv(pTHX_ const char *name, I32 flags)
{
    PERL_ARGS_ASSERT_GET_CV;

    return get_cvn_flags(name, strlen(name), flags);
}

/* Be sure to refetch the stack pointer after calling these routines. */

/*

=head1 Callback Functions

=for apidoc p||call_argv

Performs a callback to the specified named and package-scoped Perl subroutine
with C<argv> (a NULL-terminated array of strings) as arguments. See L<perlcall>.

Approximate Perl equivalent: C<&{"$sub_name"}(@$argv)>.

=cut
*/

I32
Perl_call_argv(pTHX_ const char *sub_name, I32 flags, register char **argv)

          		/* See G_* flags in cop.h */
                     	/* null terminated arg list */
{
    dVAR;
    dSP;

    PERL_ARGS_ASSERT_CALL_ARGV;

    PUSHMARK(SP);
    if (argv) {
	while (*argv) {
	    mXPUSHs(newSVpv(*argv,0));
	    argv++;
	}
	PUTBACK;
    }
    return call_pv(sub_name, flags);
}

/*
=for apidoc p||call_pv

Performs a callback to the specified Perl sub.  See L<perlcall>.

=cut
*/

I32
Perl_call_pv(pTHX_ const char *sub_name, I32 flags)
              		/* name of the subroutine */
          		/* See G_* flags in cop.h */
{
    PERL_ARGS_ASSERT_CALL_PV;

    return call_sv(MUTABLE_SV(get_cv(sub_name, GV_ADD)), flags);
}

/*
=for apidoc p||call_method

Performs a callback to the specified Perl method.  The blessed object must
be on the stack.  See L<perlcall>.

=cut
*/

I32
Perl_call_method(pTHX_ const char *methname, I32 flags)
               		/* name of the subroutine */
          		/* See G_* flags in cop.h */
{
    STRLEN len;
    PERL_ARGS_ASSERT_CALL_METHOD;

    len = strlen(methname);

    /* XXX: sv_2mortal(newSVpvn_share(methname, len)) can be faster */
    return call_sv(newSVpvn_flags(methname, len, SVs_TEMP), flags | G_METHOD);
}

/* May be called with any of a CV, a GV, or an SV containing the name. */
/*
=for apidoc p||call_sv

Performs a callback to the Perl sub whose name is in the SV.  See
L<perlcall>.

=cut
*/

I32
Perl_call_sv(pTHX_ SV *sv, VOL I32 flags)
          		/* See G_* flags in cop.h */
{
    dVAR; dSP;
    LOGOP myop;		/* fake syntax tree node */
    UNOP method_op;
    I32 oldmark;
    VOL I32 retval = 0;
    I32 oldscope;
    bool oldcatch = CATCH_GET;
    int ret;
    OP* const oldop = PL_op;
    dJMPENV;

    PERL_ARGS_ASSERT_CALL_SV;

    if (flags & G_DISCARD) {
	ENTER;
	SAVETMPS;
    }
    if (!(flags & G_WANT)) {
	/* Backwards compatibility - as G_SCALAR was 0, it could be omitted.
	 */
	flags |= G_SCALAR;
    }

    Zero(&myop, 1, LOGOP);
    myop.op_next = NULL;
    if (!(flags & G_NOARGS))
	myop.op_flags |= OPf_STACKED;
    myop.op_flags |= OP_GIMME_REVERSE(flags);
    SAVEOP();
    PL_op = (OP*)&myop;

    EXTEND(PL_stack_sp, 1);
    *++PL_stack_sp = sv;
    oldmark = TOPMARK;
    oldscope = PL_scopestack_ix;

    if (PERLDB_SUB && PL_curstash != PL_debstash
	   /* Handle first BEGIN of -d. */
	  && (PL_DBcv || (PL_DBcv = GvCV(PL_DBsub)))
	   /* Try harder, since this may have been a sighandler, thus
	    * curstash may be meaningless. */
	  && (SvTYPE(sv) != SVt_PVCV || CvSTASH((const CV *)sv) != PL_debstash)
	  && !(flags & G_NODEBUG))
	PL_op->op_private |= OPpENTERSUB_DB;

    if (flags & G_METHOD) {
	Zero(&method_op, 1, UNOP);
	method_op.op_next = PL_op;
	method_op.op_ppaddr = PL_ppaddr[OP_METHOD];
	method_op.op_type = OP_METHOD;
	myop.op_ppaddr = PL_ppaddr[OP_ENTERSUB];
	myop.op_type = OP_ENTERSUB;
	PL_op = (OP*)&method_op;
    }

    if (!(flags & G_EVAL)) {
	CATCH_SET(TRUE);
	CALL_BODY_SUB((OP*)&myop);
	retval = PL_stack_sp - (PL_stack_base + oldmark);
	CATCH_SET(oldcatch);
    }
    else {
	myop.op_other = (OP*)&myop;
	PL_markstack_ptr--;
	create_eval_scope(flags|G_FAKINGEVAL);
	PL_markstack_ptr++;

	JMPENV_PUSH(ret);

	switch (ret) {
	case 0:
 redo_body:
	    CALL_BODY_SUB((OP*)&myop);
	    retval = PL_stack_sp - (PL_stack_base + oldmark);
	    if (!(flags & G_KEEPERR)) {
		CLEAR_ERRSV();
	    }
	    break;
	case 1:
	    STATUS_ALL_FAILURE;
	    /* FALL THROUGH */
	case 2:
	    /* my_exit() was called */
	    SET_CURSTASH(PL_defstash);
	    FREETMPS;
	    JMPENV_POP;
	    my_exit_jump();
	    /* NOTREACHED */
	case 3:
	    if (PL_restartop) {
		PL_restartjmpenv = NULL;
		PL_op = PL_restartop;
		PL_restartop = 0;
		goto redo_body;
	    }
	    PL_stack_sp = PL_stack_base + oldmark;
	    if ((flags & G_WANT) == G_ARRAY)
		retval = 0;
	    else {
		retval = 1;
		*++PL_stack_sp = &PL_sv_undef;
	    }
	    break;
	}

	if (PL_scopestack_ix > oldscope)
	    delete_eval_scope();
	JMPENV_POP;
    }

    if (flags & G_DISCARD) {
	PL_stack_sp = PL_stack_base + oldmark;
	retval = 0;
	FREETMPS;
	LEAVE;
    }
    PL_op = oldop;
    return retval;
}

/* Eval a string. The G_EVAL flag is always assumed. */

/*
=for apidoc p||eval_sv

Tells Perl to C<eval> the string in the SV. It supports the same flags
as C<call_sv>, with the obvious exception of G_EVAL. See L<perlcall>.

=cut
*/

I32
Perl_eval_sv(pTHX_ SV *sv, I32 flags)

          		/* See G_* flags in cop.h */
{
    dVAR;
    dSP;
    UNOP myop;		/* fake syntax tree node */
    VOL I32 oldmark = SP - PL_stack_base;
    VOL I32 retval = 0;
    int ret;
    OP* const oldop = PL_op;
    dJMPENV;

    PERL_ARGS_ASSERT_EVAL_SV;

    if (flags & G_DISCARD) {
	ENTER;
	SAVETMPS;
    }

    SAVEOP();
    PL_op = (OP*)&myop;
    Zero(PL_op, 1, UNOP);
    EXTEND(PL_stack_sp, 1);
    *++PL_stack_sp = sv;

    if (!(flags & G_NOARGS))
	myop.op_flags = OPf_STACKED;
    myop.op_next = NULL;
    myop.op_type = OP_ENTEREVAL;
    myop.op_flags |= OP_GIMME_REVERSE(flags);
    if (flags & G_KEEPERR)
	myop.op_flags |= OPf_SPECIAL;

    /* fail now; otherwise we could fail after the JMPENV_PUSH but
     * before a PUSHEVAL, which corrupts the stack after a croak */
    TAINT_PROPER("eval_sv()");

    JMPENV_PUSH(ret);
    switch (ret) {
    case 0:
 redo_body:
	if (PL_op == (OP*)(&myop)) {
	    PL_op = PL_ppaddr[OP_ENTEREVAL](aTHX);
	    if (!PL_op)
		goto fail; /* failed in compilation */
	}
	CALLRUNOPS(aTHX);
	retval = PL_stack_sp - (PL_stack_base + oldmark);
	if (!(flags & G_KEEPERR)) {
	    CLEAR_ERRSV();
	}
	break;
    case 1:
	STATUS_ALL_FAILURE;
	/* FALL THROUGH */
    case 2:
	/* my_exit() was called */
	SET_CURSTASH(PL_defstash);
	FREETMPS;
	JMPENV_POP;
	my_exit_jump();
	/* NOTREACHED */
    case 3:
	if (PL_restartop) {
	    PL_restartjmpenv = NULL;
	    PL_op = PL_restartop;
	    PL_restartop = 0;
	    goto redo_body;
	}
      fail:
	PL_stack_sp = PL_stack_base + oldmark;
	if ((flags & G_WANT) == G_ARRAY)
	    retval = 0;
	else {
	    retval = 1;
	    *++PL_stack_sp = &PL_sv_undef;
	}
	break;
    }

    JMPENV_POP;
    if (flags & G_DISCARD) {
	PL_stack_sp = PL_stack_base + oldmark;
	retval = 0;
	FREETMPS;
	LEAVE;
    }
    PL_op = oldop;
    return retval;
}

/*
=for apidoc p||eval_pv

Tells Perl to C<eval> the given string and return an SV* result.

=cut
*/

SV*
Perl_eval_pv(pTHX_ const char *p, I32 croak_on_error)
{
    dVAR;
    dSP;
    SV* sv = newSVpv(p, 0);

    PERL_ARGS_ASSERT_EVAL_PV;

    eval_sv(sv, G_SCALAR);
    SvREFCNT_dec(sv);

    SPAGAIN;
    sv = POPs;
    PUTBACK;

    if (croak_on_error && SvTRUE(ERRSV)) {
	Perl_croak(aTHX_ "%s", SvPVx_nolen_const(ERRSV));
    }

    return sv;
}

/* Require a module. */

/*
=head1 Embedding Functions

=for apidoc p||require_pv

Tells Perl to C<require> the file named by the string argument.  It is
analogous to the Perl code C<eval "require '$file'">.  It's even
implemented that way; consider using load_module instead.

=cut */

void
Perl_require_pv(pTHX_ const char *pv)
{
    dVAR;
    dSP;
    SV* sv;

    PERL_ARGS_ASSERT_REQUIRE_PV;

    PUSHSTACKi(PERLSI_REQUIRE);
    PUTBACK;
    sv = Perl_newSVpvf(aTHX_ "require q%c%s%c", 0, pv, 0);
    eval_sv(sv_2mortal(sv), G_DISCARD);
    SPAGAIN;
    POPSTACK;
}

STATIC void
S_usage(pTHX)		/* XXX move this out into a module ? */
{
    /* This message really ought to be max 23 lines.
     * Removed -h because the user already knows that option. Others? */

    /* Grouped as 6 lines per C string literal, to keep under the ANSI C 89
       minimum of 509 character string literals.  */
    static const char * const usage_msg[] = {
"  -0[octal]         specify record separator (\\0, if no argument)\n"
"  -a                autosplit mode with -n or -p (splits $_ into @F)\n"
"  -C[number/list]   enables the listed Unicode features\n"
"  -c                check syntax only (runs BEGIN and CHECK blocks)\n"
"  -d[:debugger]     run program under debugger\n"
"  -D[number/list]   set debugging flags (argument is a bit mask or alphabets)\n",
"  -e program        one line of program (several -e's allowed, omit programfile)\n"
"  -E program        like -e, but enables all optional features\n"
"  -f                don't do $sitelib/sitecustomize.pl at startup\n"
"  -F/pattern/       split() pattern for -a switch (//'s are optional)\n"
"  -i[extension]     edit <> files in place (makes backup if extension supplied)\n"
"  -Idirectory       specify @INC/#include directory (several -I's allowed)\n",
"  -l[octal]         enable line ending processing, specifies line terminator\n"
"  -[mM][-]module    execute \"use/no module...\" before executing program\n"
"  -n                assume \"while (<>) { ... }\" loop around program\n"
"  -p                assume loop like -n but print line also, like sed\n"
"  -s                enable rudimentary parsing for switches after programfile\n"
"  -S                look for programfile using PATH environment variable\n",
"  -t                enable tainting warnings\n"
"  -T                enable tainting checks\n"
"  -u                dump core after parsing program\n"
"  -U                allow unsafe operations\n"
"  -v                print version, patchlevel and license\n"
"  -V[:variable]     print configuration summary (or a single Config.pm variable)\n",
"  -w                enable many useful warnings\n"
"  -W                enable all warnings\n"
"  -x[directory]     ignore text before #!perl line (optionally cd to directory)\n"
"  -X                disable all warnings\n"
"  \n"
"Run 'perldoc perl' for more help with Perl.\n\n",
NULL
};
    const char * const *p = usage_msg;
    PerlIO *out = PerlIO_stdout();

    PerlIO_printf(out,
		  "\nUsage: %s [switches] [--] [programfile] [arguments]\n",
		  PL_origargv[0]);
    while (*p)
	PerlIO_puts(out, *p++);
    my_exit(0);
}

/* convert a string of -D options (or digits) into an int.
 * sets *s to point to the char after the options */

#ifdef DEBUGGING
int
Perl_get_debug_opts(pTHX_ const char **s, bool givehelp)
{
    static const char * const usage_msgd[] = {
      " Debugging flag values: (see also -d)\n"
      "  p  Tokenizing and parsing (with v, displays parse stack)\n"
      "  s  Stack snapshots (with v, displays all stacks)\n"
      "  l  Context (loop) stack processing\n"
      "  t  Trace execution\n"
      "  o  Method and overloading resolution\n",
      "  c  String/numeric conversions\n"
      "  P  Print profiling info, source file input state\n"
      "  m  Memory and SV allocation\n"
      "  f  Format processing\n"
      "  r  Regular expression parsing and execution\n"
      "  x  Syntax tree dump\n",
      "  u  Tainting checks\n"
      "  H  Hash dump -- usurps values()\n"
      "  X  Scratchpad allocation\n"
      "  D  Cleaning up\n"
      "  T  Tokenising\n"
      "  R  Include reference counts of dumped variables (eg when using -Ds)\n",
      "  J  Do not s,t,P-debug (Jump over) opcodes within package DB\n"
      "  v  Verbose: use in conjunction with other flags\n"
      "  C  Copy On Write\n"
      "  A  Consistency checks on internal structures\n"
      "  q  quiet - currently only suppresses the 'EXECUTING' message\n"
      "  M  trace smart match resolution\n"
      "  B  dump suBroutine definitions, including special Blocks like BEGIN\n",
      NULL
    };
    int i = 0;

    PERL_ARGS_ASSERT_GET_DEBUG_OPTS;

    if (isALPHA(**s)) {
	/* if adding extra options, remember to update DEBUG_MASK */
	static const char debopts[] = "psltocPmfrxuUHXDSTRJvCAqMB";

	for (; isALNUM(**s); (*s)++) {
	    const char * const d = strchr(debopts,**s);
	    if (d)
		i |= 1 << (d - debopts);
	    else if (ckWARN_d(WARN_DEBUGGING))
	        Perl_warner(aTHX_ packWARN(WARN_DEBUGGING),
		    "invalid option -D%c, use -D'' to see choices\n", **s);
	}
    }
    else if (isDIGIT(**s)) {
	i = atoi(*s);
	for (; isALNUM(**s); (*s)++) ;
    }
    else if (givehelp) {
      const char *const *p = usage_msgd;
      while (*p) PerlIO_puts(PerlIO_stdout(), *p++);
    }
#  ifdef EBCDIC
    if ((i & DEBUG_p_FLAG) && ckWARN_d(WARN_DEBUGGING))
	Perl_warner(aTHX_ packWARN(WARN_DEBUGGING),
		"-Dp not implemented on this platform\n");
#  endif
    return i;
}
#endif

/* This routine handles any switches that can be given during run */

const char *
Perl_moreswitches(pTHX_ const char *s)
{
    dVAR;
    UV rschar;
    const char option = *s; /* used to remember option in -m/-M code */

    PERL_ARGS_ASSERT_MORESWITCHES;

    switch (*s) {
    case '0':
    {
	 I32 flags = 0;
	 STRLEN numlen;

	 SvREFCNT_dec(PL_rs);
	 if (s[1] == 'x' && s[2]) {
	      const char *e = s+=2;
	      U8 *tmps;

	      while (*e)
		e++;
	      numlen = e - s;
	      flags = PERL_SCAN_SILENT_ILLDIGIT;
	      rschar = (U32)grok_hex(s, &numlen, &flags, NULL);
	      if (s + numlen < e) {
		   rschar = 0; /* Grandfather -0xFOO as -0 -xFOO. */
		   numlen = 0;
		   s--;
	      }
	      PL_rs = newSVpvs("");
	      SvGROW(PL_rs, (STRLEN)(UNISKIP(rschar) + 1));
	      tmps = (U8*)SvPVX(PL_rs);
	      uvchr_to_utf8(tmps, rschar);
	      SvCUR_set(PL_rs, UNISKIP(rschar));
	      SvUTF8_on(PL_rs);
	 }
	 else {
	      numlen = 4;
	      rschar = (U32)grok_oct(s, &numlen, &flags, NULL);
	      if (rschar & ~((U8)~0))
		   PL_rs = &PL_sv_undef;
	      else if (!rschar && numlen >= 2)
		   PL_rs = newSVpvs("");
	      else {
		   char ch = (char)rschar;
		   PL_rs = newSVpvn(&ch, 1);
	      }
	 }
	 sv_setsv(get_sv("/", GV_ADD), PL_rs);
	 return s + numlen;
    }
    case 'C':
        s++;
        PL_unicode = parse_unicode_opts( (const char **)&s );
	if (PL_unicode & PERL_UNICODE_UTF8CACHEASSERT_FLAG)
	    PL_utf8cache = -1;
	return s;
    case 'F':
	PL_minus_F = TRUE;
	PL_splitstr = ++s;
	while (*s && !isSPACE(*s)) ++s;
	PL_splitstr = savepvn(PL_splitstr, s - PL_splitstr);
	return s;
    case 'a':
	PL_minus_a = TRUE;
	s++;
	return s;
    case 'c':
	PL_minus_c = TRUE;
	s++;
	return s;
    case 'd':
	forbid_setid('d', FALSE);
	s++;

        /* -dt indicates to the debugger that threads will be used */
	if (*s == 't' && !isALNUM(s[1])) {
	    ++s;
	    my_setenv("PERL5DB_THREADED", "1");
	}

	/* The following permits -d:Mod to accepts arguments following an =
	   in the fashion that -MSome::Mod does. */
	if (*s == ':' || *s == '=') {
	    const char *start;
	    const char *end;
	    SV *sv;

	    if (*++s == '-') {
		++s;
		sv = newSVpvs("no Devel::");
	    } else {
		sv = newSVpvs("use Devel::");
	    }

	    start = s;
	    end = s + strlen(s);

	    /* We now allow -d:Module=Foo,Bar and -d:-Module */
	    while(isALNUM(*s) || *s==':') ++s;
	    if (*s != '=')
		sv_catpvn(sv, start, end - start);
	    else {
		sv_catpvn(sv, start, s-start);
		/* Don't use NUL as q// delimiter here, this string goes in the
		 * environment. */
		Perl_sv_catpvf(aTHX_ sv, " split(/,/,q{%s});", ++s);
	    }
	    s = end;
	    my_setenv("PERL5DB", SvPV_nolen_const(sv));
	    SvREFCNT_dec(sv);
	}
	if (!PL_perldb) {
	    PL_perldb = PERLDB_ALL;
	    init_debugger();
	}
	return s;
    case 'D':
    {
#ifdef DEBUGGING
	forbid_setid('D', FALSE);
	s++;
	PL_debug = get_debug_opts( (const char **)&s, 1) | DEBUG_TOP_FLAG;
#else /* !DEBUGGING */
	if (ckWARN_d(WARN_DEBUGGING))
	    Perl_warner(aTHX_ packWARN(WARN_DEBUGGING),
	           "Recompile perl with -DDEBUGGING to use -D switch (did you mean -d ?)\n");
	for (s++; isALNUM(*s); s++) ;
#endif
	return s;
    }
    case 'h':
	usage();
    case 'i':
	Safefree(PL_inplace);
#if defined(__CYGWIN__) /* do backup extension automagically */
	if (*(s+1) == '\0') {
	PL_inplace = savepvs(".bak");
	return s+1;
	}
#endif /* __CYGWIN__ */
	{
	    const char * const start = ++s;
	    while (*s && !isSPACE(*s))
		++s;

	    PL_inplace = savepvn(start, s - start);
	}
	if (*s) {
	    ++s;
	    if (*s == '-')	/* Additional switches on #! line. */
		s++;
	}
	return s;
    case 'I':	/* -I handled both here and in parse_body() */
	forbid_setid('I', FALSE);
	++s;
	while (*s && isSPACE(*s))
	    ++s;
	if (*s) {
	    const char *e, *p;
	    p = s;
	    /* ignore trailing spaces (possibly followed by other switches) */
	    do {
		for (e = p; *e && !isSPACE(*e); e++) ;
		p = e;
		while (isSPACE(*p))
		    p++;
	    } while (*p && *p != '-');
	    incpush(s, e-s,
		    INCPUSH_ADD_SUB_DIRS|INCPUSH_ADD_OLD_VERS|INCPUSH_UNSHIFT);
	    s = p;
	    if (*s == '-')
		s++;
	}
	else
	    Perl_croak(aTHX_ "No directory specified for -I");
	return s;
    case 'l':
	PL_minus_l = TRUE;
	s++;
	if (PL_ors_sv) {
	    SvREFCNT_dec(PL_ors_sv);
	    PL_ors_sv = NULL;
	}
	if (isDIGIT(*s)) {
            I32 flags = 0;
	    STRLEN numlen;
	    PL_ors_sv = newSVpvs("\n");
	    numlen = 3 + (*s == '0');
	    *SvPVX(PL_ors_sv) = (char)grok_oct(s, &numlen, &flags, NULL);
	    s += numlen;
	}
	else {
	    if (RsPARA(PL_rs)) {
		PL_ors_sv = newSVpvs("\n\n");
	    }
	    else {
		PL_ors_sv = newSVsv(PL_rs);
	    }
	}
	return s;
    case 'M':
	forbid_setid('M', FALSE);	/* XXX ? */
	/* FALL THROUGH */
    case 'm':
	forbid_setid('m', FALSE);	/* XXX ? */
	if (*++s) {
	    const char *start;
	    const char *end;
	    SV *sv;
	    const char *use = "use ";
	    bool colon = FALSE;
	    /* -M-foo == 'no foo'	*/
	    /* Leading space on " no " is deliberate, to make both
	       possibilities the same length.  */
	    if (*s == '-') { use = " no "; ++s; }
	    sv = newSVpvn(use,4);
	    start = s;
	    /* We allow -M'Module qw(Foo Bar)'	*/
	    while(isALNUM(*s) || *s==':') {
		if( *s++ == ':' ) {
		    if( *s == ':' )
			s++;
		    else
			colon = TRUE;
		}
	    }
	    if (s == start)
		Perl_croak(aTHX_ "Module name required with -%c option",
				    option);
	    if (colon)
		Perl_croak(aTHX_ "Invalid module name %.*s with -%c option: "
				    "contains single ':'",
				    (int)(s - start), start, option);
	    end = s + strlen(s);
	    if (*s != '=') {
		sv_catpvn(sv, start, end - start);
		if (option == 'm') {
		    if (*s != '\0')
			Perl_croak(aTHX_ "Can't use '%c' after -mname", *s);
		    sv_catpvs( sv, " ()");
		}
	    } else {
		sv_catpvn(sv, start, s-start);
		/* Use NUL as q''-delimiter.  */
		sv_catpvs(sv, " split(/,/,q\0");
		++s;
		sv_catpvn(sv, s, end - s);
		sv_catpvs(sv,  "\0)");
	    }
	    s = end;
	    Perl_av_create_and_push(aTHX_ &PL_preambleav, sv);
	}
	else
	    Perl_croak(aTHX_ "Missing argument to -%c", option);
	return s;
    case 'n':
	PL_minus_n = TRUE;
	s++;
	return s;
    case 'p':
	PL_minus_p = TRUE;
	s++;
	return s;
    case 's':
	forbid_setid('s', FALSE);
	PL_doswitches = TRUE;
	s++;
	return s;
    case 't':
    case 'T':
        if (!PL_tainting)
	    TOO_LATE_FOR(*s);
        s++;
	return s;
    case 'u':
	PL_do_undump = TRUE;
	s++;
	return s;
    case 'U':
	PL_unsafe = TRUE;
	s++;
	return s;
    case 'v':
	minus_v();
    case 'w':
	if (! (PL_dowarn & G_WARN_ALL_MASK)) {
	    PL_dowarn |= G_WARN_ON;
	}
	s++;
	return s;
    case 'W':
	PL_dowarn = G_WARN_ALL_ON|G_WARN_ON;
        if (!specialWARN(PL_compiling.cop_warnings))
            PerlMemShared_free(PL_compiling.cop_warnings);
	PL_compiling.cop_warnings = pWARN_ALL ;
	s++;
	return s;
    case 'X':
	PL_dowarn = G_WARN_ALL_OFF;
        if (!specialWARN(PL_compiling.cop_warnings))
            PerlMemShared_free(PL_compiling.cop_warnings);
	PL_compiling.cop_warnings = pWARN_NONE ;
	s++;
	return s;
    case '*':
    case ' ':
        while( *s == ' ' )
          ++s;
	if (s[0] == '-')	/* Additional switches on #! line. */
	    return s+1;
	break;
    case '-':
    case 0:
#if defined(WIN32) || !defined(PERL_STRICT_CR)
    case '\r':
#endif
    case '\n':
    case '\t':
	break;
#ifdef ALTERNATE_SHEBANG
    case 'S':			/* OS/2 needs -S on "extproc" line. */
	break;
#endif
    case 'e': case 'f': case 'x': case 'E':
#ifndef ALTERNATE_SHEBANG
    case 'S':
#endif
    case 'V':
	Perl_croak(aTHX_ "Can't emulate -%.1s on #! line",s);
    default:
	Perl_croak(aTHX_
	    "Unrecognized switch: -%.1s  (-h will show valid options)",s
	);
    }
    return NULL;
}


STATIC void
S_minus_v(pTHX)
{
	if (!sv_derived_from(PL_patchlevel, "version"))
	    upg_version(PL_patchlevel, TRUE);
#if !defined(DGUX)
	{
	    SV* level= vstringify(PL_patchlevel);
#ifdef PERL_PATCHNUM
#  ifdef PERL_GIT_UNCOMMITTED_CHANGES
	    SV *num = newSVpvs(PERL_PATCHNUM "*");
#  else
	    SV *num = newSVpvs(PERL_PATCHNUM);
#  endif

	    if (sv_len(num)>=sv_len(level) && strnEQ(SvPV_nolen(num),SvPV_nolen(level),sv_len(level))) {
		SvREFCNT_dec(level);
		level= num;
	    } else {
		Perl_sv_catpvf(aTHX_ level, " (%"SVf")", num);
		SvREFCNT_dec(num);
	    }
 #endif
	    PerlIO_printf(PerlIO_stdout(),
		"\nThis is perl "	STRINGIFY(PERL_REVISION)
		", version "		STRINGIFY(PERL_VERSION)
		", subversion "		STRINGIFY(PERL_SUBVERSION)
		" (%"SVf") built for "	ARCHNAME, level
		);
	    SvREFCNT_dec(level);
	}
#else /* DGUX */
/* Adjust verbose output as in the perl that ships with the DG/UX OS from EMC */
	PerlIO_printf(PerlIO_stdout(),
		Perl_form(aTHX_ "\nThis is perl, %"SVf"\n",
		    SVfARG(vstringify(PL_patchlevel))));
	PerlIO_printf(PerlIO_stdout(),
			Perl_form(aTHX_ "        built under %s at %s %s\n",
					OSNAME, __DATE__, __TIME__));
	PerlIO_printf(PerlIO_stdout(),
			Perl_form(aTHX_ "        OS Specific Release: %s\n",
					OSVERS));
#endif /* !DGUX */
#if defined(LOCAL_PATCH_COUNT)
	if (LOCAL_PATCH_COUNT > 0)
	    PerlIO_printf(PerlIO_stdout(),
			  "\n(with %d registered patch%s, "
			  "see perl -V for more detail)",
			  LOCAL_PATCH_COUNT,
			  (LOCAL_PATCH_COUNT!=1) ? "es" : "");
#endif

	PerlIO_printf(PerlIO_stdout(),
		      "\n\nCopyright 1987-2012, Larry Wall\n");
#ifdef MSDOS
	PerlIO_printf(PerlIO_stdout(),
		      "\nMS-DOS port Copyright (c) 1989, 1990, Diomidis Spinellis\n");
#endif
#ifdef DJGPP
	PerlIO_printf(PerlIO_stdout(),
		      "djgpp v2 port (jpl5003c) by Hirofumi Watanabe, 1996\n"
		      "djgpp v2 port (perl5004+) by Laszlo Molnar, 1997-1999\n");
#endif
#ifdef OS2
	PerlIO_printf(PerlIO_stdout(),
		      "\n\nOS/2 port Copyright (c) 1990, 1991, Raymond Chen, Kai Uwe Rommel\n"
		      "Version 5 port Copyright (c) 1994-2002, Andreas Kaiser, Ilya Zakharevich\n");
#endif
#ifdef atarist
	PerlIO_printf(PerlIO_stdout(),
		      "atariST series port, ++jrb  bammi@cadence.com\n");
#endif
#ifdef __BEOS__
	PerlIO_printf(PerlIO_stdout(),
		      "BeOS port Copyright Tom Spindler, 1997-1999\n");
#endif
#ifdef MPE
	PerlIO_printf(PerlIO_stdout(),
		      "MPE/iX port Copyright by Mark Klein and Mark Bixby, 1996-2003\n");
#endif
#ifdef OEMVS
	PerlIO_printf(PerlIO_stdout(),
		      "MVS (OS390) port by Mortice Kern Systems, 1997-1999\n");
#endif
#ifdef __VOS__
	PerlIO_printf(PerlIO_stdout(),
		      "Stratus VOS port by Paul.Green@stratus.com, 1997-2002\n");
#endif
#ifdef __OPEN_VM
	PerlIO_printf(PerlIO_stdout(),
		      "VM/ESA port by Neale Ferguson, 1998-1999\n");
#endif
#ifdef POSIX_BC
	PerlIO_printf(PerlIO_stdout(),
		      "BS2000 (POSIX) port by Start Amadeus GmbH, 1998-1999\n");
#endif
#ifdef EPOC
	PerlIO_printf(PerlIO_stdout(),
		      "EPOC port by Olaf Flebbe, 1999-2002\n");
#endif
#ifdef UNDER_CE
	PerlIO_printf(PerlIO_stdout(),"WINCE port by Rainer Keuchel, 2001-2002\n");
	PerlIO_printf(PerlIO_stdout(),"Built on " __DATE__ " " __TIME__ "\n\n");
	wce_hitreturn();
#endif
#ifdef __SYMBIAN32__
	PerlIO_printf(PerlIO_stdout(),
		      "Symbian port by Nokia, 2004-2005\n");
#endif
#ifdef BINARY_BUILD_NOTICE
	BINARY_BUILD_NOTICE;
#endif
	PerlIO_printf(PerlIO_stdout(),
		      "\n\
Perl may be copied only under the terms of either the Artistic License or the\n\
GNU General Public License, which may be found in the Perl 5 source kit.\n\n\
Complete documentation for Perl, including FAQ lists, should be found on\n\
this system using \"man perl\" or \"perldoc perl\".  If you have access to the\n\
Internet, point your browser at http://www.perl.org/, the Perl Home Page.\n\n");
	my_exit(0);
}

/* compliments of Tom Christiansen */

/* unexec() can be found in the Gnu emacs distribution */
/* Known to work with -DUNEXEC and using unexelf.c from GNU emacs-20.2 */

void
Perl_my_unexec(pTHX)
{
    PERL_UNUSED_CONTEXT;
#ifdef UNEXEC
    SV *    prog = newSVpv(BIN_EXP, 0);
    SV *    file = newSVpv(PL_origfilename, 0);
    int    status = 1;
    extern int etext;

    sv_catpvs(prog, "/perl");
    sv_catpvs(file, ".perldump");

    unexec(SvPVX(file), SvPVX(prog), &etext, sbrk(0), 0);
    /* unexec prints msg to stderr in case of failure */
    PerlProc_exit(status);
#else
#  ifdef VMS
#    include <lib$routines.h>
     lib$signal(SS$_DEBUG);  /* ssdef.h #included from vmsish.h */
#  elif defined(WIN32) || defined(__CYGWIN__)
    Perl_croak(aTHX_ "dump is not supported");
#  else
    ABORT();		/* for use with undump */
#  endif
#endif
}

/* initialize curinterp */
STATIC void
S_init_interp(pTHX)
{
    dVAR;
#ifdef MULTIPLICITY
#  define PERLVAR(prefix,var,type)
#  define PERLVARA(prefix,var,n,type)
#  if defined(PERL_IMPLICIT_CONTEXT)
#    define PERLVARI(prefix,var,type,init)	aTHX->prefix##var = init;
#    define PERLVARIC(prefix,var,type,init)	aTHX->prefix##var = init;
#  else
#    define PERLVARI(prefix,var,type,init)	PERL_GET_INTERP->var = init;
#    define PERLVARIC(prefix,var,type,init)	PERL_GET_INTERP->var = init;
#  endif
#  include "intrpvar.h"
#  undef PERLVAR
#  undef PERLVARA
#  undef PERLVARI
#  undef PERLVARIC
#else
#  define PERLVAR(prefix,var,type)
#  define PERLVARA(prefix,var,n,type)
#  define PERLVARI(prefix,var,type,init)	PL_##var = init;
#  define PERLVARIC(prefix,var,type,init)	PL_##var = init;
#  include "intrpvar.h"
#  undef PERLVAR
#  undef PERLVARA
#  undef PERLVARI
#  undef PERLVARIC
#endif

    /* As these are inside a structure, PERLVARI isn't capable of initialising
       them  */
    PL_reg_oldcurpm = PL_reg_curpm = NULL;
    PL_reg_poscache = PL_reg_starttry = NULL;
}

STATIC void
S_init_main_stash(pTHX)
{
    dVAR;
    GV *gv;

    PL_curstash = PL_defstash = (HV *)SvREFCNT_inc_simple_NN(newHV());
    /* We know that the string "main" will be in the global shared string
       table, so it's a small saving to use it rather than allocate another
       8 bytes.  */
    PL_curstname = newSVpvs_share("main");
    gv = gv_fetchpvs("main::", GV_ADD|GV_NOTQUAL, SVt_PVHV);
    /* If we hadn't caused another reference to "main" to be in the shared
       string table above, then it would be worth reordering these two,
       because otherwise all we do is delete "main" from it as a consequence
       of the SvREFCNT_dec, only to add it again with hv_name_set */
    SvREFCNT_dec(GvHV(gv));
    hv_name_set(PL_defstash, "main", 4, 0);
    GvHV(gv) = MUTABLE_HV(SvREFCNT_inc_simple(PL_defstash));
    SvREADONLY_on(gv);
    PL_incgv = gv_HVadd(gv_AVadd(gv_fetchpvs("INC", GV_ADD|GV_NOTQUAL,
					     SVt_PVAV)));
    SvREFCNT_inc_simple_void(PL_incgv); /* Don't allow it to be freed */
    GvMULTI_on(PL_incgv);
    PL_hintgv = gv_fetchpvs("\010", GV_ADD|GV_NOTQUAL, SVt_PV); /* ^H */
    GvMULTI_on(PL_hintgv);
    PL_defgv = gv_fetchpvs("_", GV_ADD|GV_NOTQUAL, SVt_PVAV);
    SvREFCNT_inc_simple_void(PL_defgv);
    PL_errgv = gv_HVadd(gv_fetchpvs("@", GV_ADD|GV_NOTQUAL, SVt_PV));
    SvREFCNT_inc_simple_void(PL_errgv);
    GvMULTI_on(PL_errgv);
    PL_replgv = gv_fetchpvs("\022", GV_ADD|GV_NOTQUAL, SVt_PV); /* ^R */
    GvMULTI_on(PL_replgv);
    (void)Perl_form(aTHX_ "%240s","");	/* Preallocate temp - for immediate signals. */
#ifdef PERL_DONT_CREATE_GVSV
    gv_SVadd(PL_errgv);
#endif
    sv_grow(ERRSV, 240);	/* Preallocate - for immediate signals. */
    CLEAR_ERRSV();
    SET_CURSTASH(PL_defstash);
    CopSTASH_set(&PL_compiling, PL_defstash);
    PL_debstash = GvHV(gv_fetchpvs("DB::", GV_ADDMULTI, SVt_PVHV));
    PL_globalstash = GvHV(gv_fetchpvs("CORE::GLOBAL::", GV_ADDMULTI,
				      SVt_PVHV));
    /* We must init $/ before switches are processed. */
    sv_setpvs(get_sv("/", GV_ADD), "\n");
}

STATIC PerlIO *
S_open_script(pTHX_ const char *scriptname, bool dosearch, bool *suidscript)
{
    int fdscript = -1;
    PerlIO *rsfp = NULL;
    dVAR;

    PERL_ARGS_ASSERT_OPEN_SCRIPT;

    if (PL_e_script) {
	PL_origfilename = savepvs("-e");
    }
    else {
	/* if find_script() returns, it returns a malloc()-ed value */
	scriptname = PL_origfilename = find_script(scriptname, dosearch, NULL, 1);

	if (strnEQ(scriptname, "/dev/fd/", 8) && isDIGIT(scriptname[8]) ) {
            const char *s = scriptname + 8;
	    fdscript = atoi(s);
	    while (isDIGIT(*s))
		s++;
	    if (*s) {
		/* PSz 18 Feb 04
		 * Tell apart "normal" usage of fdscript, e.g.
		 * with bash on FreeBSD:
		 *   perl <( echo '#!perl -DA'; echo 'print "$0\n"')
		 * from usage in suidperl.
		 * Does any "normal" usage leave garbage after the number???
		 * Is it a mistake to use a similar /dev/fd/ construct for
		 * suidperl?
		 */
		*suidscript = TRUE;
		/* PSz 20 Feb 04
		 * Be supersafe and do some sanity-checks.
		 * Still, can we be sure we got the right thing?
		 */
		if (*s != '/') {
		    Perl_croak(aTHX_ "Wrong syntax (suid) fd script name \"%s\"\n", s);
		}
		if (! *(s+1)) {
		    Perl_croak(aTHX_ "Missing (suid) fd script name\n");
		}
		scriptname = savepv(s + 1);
		Safefree(PL_origfilename);
		PL_origfilename = (char *)scriptname;
	    }
	}
    }

    CopFILE_free(PL_curcop);
    CopFILE_set(PL_curcop, PL_origfilename);
    if (*PL_origfilename == '-' && PL_origfilename[1] == '\0')
	scriptname = (char *)"";
    if (fdscript >= 0) {
	rsfp = PerlIO_fdopen(fdscript,PERL_SCRIPT_MODE);
    }
    else if (!*scriptname) {
	forbid_setid(0, *suidscript);
	return NULL;
    }
    else {
#ifdef FAKE_BIT_BUCKET
	/* This hack allows one not to have /dev/null (or BIT_BUCKET as it
	 * is called) and still have the "-e" work.  (Believe it or not,
	 * a /dev/null is required for the "-e" to work because source
	 * filter magic is used to implement it. ) This is *not* a general
	 * replacement for a /dev/null.  What we do here is create a temp
	 * file (an empty file), open up that as the script, and then
	 * immediately close and unlink it.  Close enough for jazz. */
#define FAKE_BIT_BUCKET_PREFIX "/tmp/perlnull-"
#define FAKE_BIT_BUCKET_SUFFIX "XXXXXXXX"
#define FAKE_BIT_BUCKET_TEMPLATE FAKE_BIT_BUCKET_PREFIX FAKE_BIT_BUCKET_SUFFIX
	char tmpname[sizeof(FAKE_BIT_BUCKET_TEMPLATE)] = {
	    FAKE_BIT_BUCKET_TEMPLATE
	};
	const char * const err = "Failed to create a fake bit bucket";
	if (strEQ(scriptname, BIT_BUCKET)) {
#ifdef HAS_MKSTEMP /* Hopefully mkstemp() is safe here. */
	    int tmpfd = mkstemp(tmpname);
	    if (tmpfd > -1) {
		scriptname = tmpname;
		close(tmpfd);
	    } else
		Perl_croak(aTHX_ err);
#else
#  ifdef HAS_MKTEMP
	    scriptname = mktemp(tmpname);
	    if (!scriptname)
		Perl_croak(aTHX_ err);
#  endif
#endif
	}
#endif
	rsfp = PerlIO_open(scriptname,PERL_SCRIPT_MODE);
#ifdef FAKE_BIT_BUCKET
	if (memEQ(scriptname, FAKE_BIT_BUCKET_PREFIX,
		  sizeof(FAKE_BIT_BUCKET_PREFIX) - 1)
	    && strlen(scriptname) == sizeof(tmpname) - 1) {
	    unlink(scriptname);
	}
	scriptname = BIT_BUCKET;
#endif
    }
    if (!rsfp) {
	/* PSz 16 Sep 03  Keep neat error message */
	if (PL_e_script)
	    Perl_croak(aTHX_ "Can't open "BIT_BUCKET": %s\n", Strerror(errno));
	else
	    Perl_croak(aTHX_ "Can't open perl script \"%s\": %s\n",
		    CopFILE(PL_curcop), Strerror(errno));
    }
#if defined(HAS_FCNTL) && defined(F_SETFD)
    /* ensure close-on-exec */
    fcntl(PerlIO_fileno(rsfp), F_SETFD, 1);
#endif
    return rsfp;
}

/* Mention
 * I_SYSSTATVFS	HAS_FSTATVFS
 * I_SYSMOUNT
 * I_STATFS	HAS_FSTATFS	HAS_GETFSSTAT
 * I_MNTENT	HAS_GETMNTENT	HAS_HASMNTOPT
 * here so that metaconfig picks them up. */


#ifdef SETUID_SCRIPTS_ARE_SECURE_NOW
/* Don't even need this function.  */
#else
STATIC void
S_validate_suid(pTHX_ PerlIO *rsfp)
{
    const UV  my_uid = PerlProc_getuid();
    const UV my_euid = PerlProc_geteuid();
    const UV  my_gid = PerlProc_getgid();
    const UV my_egid = PerlProc_getegid();

    PERL_ARGS_ASSERT_VALIDATE_SUID;

    if (my_euid != my_uid || my_egid != my_gid) {	/* (suidperl doesn't exist, in fact) */
	dVAR;

	PerlLIO_fstat(PerlIO_fileno(rsfp),&PL_statbuf);	/* may be either wrapped or real suid */
	if ((my_euid != my_uid && my_euid == PL_statbuf.st_uid && PL_statbuf.st_mode & S_ISUID)
	    ||
	    (my_egid != my_gid && my_egid == PL_statbuf.st_gid && PL_statbuf.st_mode & S_ISGID)
	   )
	    if (!PL_do_undump)
		Perl_croak(aTHX_ "YOU HAVEN'T DISABLED SET-ID SCRIPTS IN THE KERNEL YET!\n\
FIX YOUR KERNEL, PUT A C WRAPPER AROUND THIS SCRIPT, OR USE -u AND UNDUMP!\n");
	/* not set-id, must be wrapped */
    }
}
#endif /* SETUID_SCRIPTS_ARE_SECURE_NOW */

STATIC void
S_find_beginning(pTHX_ SV* linestr_sv, PerlIO *rsfp)
{
    dVAR;
    const char *s;
    register const char *s2;

    PERL_ARGS_ASSERT_FIND_BEGINNING;

    /* skip forward in input to the real script? */

    do {
	if ((s = sv_gets(linestr_sv, rsfp, 0)) == NULL)
	    Perl_croak(aTHX_ "No Perl script found in input\n");
	s2 = s;
    } while (!(*s == '#' && s[1] == '!' && ((s = instr(s,"perl")) || (s = instr(s2,"PERL")))));
    PerlIO_ungetc(rsfp, '\n');		/* to keep line count right */
    while (*s && !(isSPACE (*s) || *s == '#')) s++;
    s2 = s;
    while (*s == ' ' || *s == '\t') s++;
    if (*s++ == '-') {
	while (isDIGIT(s2[-1]) || s2[-1] == '-' || s2[-1] == '.'
	       || s2[-1] == '_') s2--;
	if (strnEQ(s2-4,"perl",4))
	    while ((s = moreswitches(s)))
		;
    }
}


STATIC void
S_init_ids(pTHX)
{
    dVAR;
    const UV my_uid = PerlProc_getuid();
    const UV my_euid = PerlProc_geteuid();
    const UV my_gid = PerlProc_getgid();
    const UV my_egid = PerlProc_getegid();

    /* Should not happen: */
    CHECK_MALLOC_TAINT(my_uid && (my_euid != my_uid || my_egid != my_gid));
    PL_tainting |= (my_uid && (my_euid != my_uid || my_egid != my_gid));
    /* BUG */
    /* PSz 27 Feb 04
     * Should go by suidscript, not uid!=euid: why disallow
     * system("ls") in scripts run from setuid things?
     * Or, is this run before we check arguments and set suidscript?
     * What about SETUID_SCRIPTS_ARE_SECURE_NOW: could we use fdscript then?
     * (We never have suidscript, can we be sure to have fdscript?)
     * Or must then go by UID checks? See comments in forbid_setid also.
     */
}

/* This is used very early in the lifetime of the program,
 * before even the options are parsed, so PL_tainting has
 * not been initialized properly.  */
bool
Perl_doing_taint(int argc, char *argv[], char *envp[])
{
#ifndef PERL_IMPLICIT_SYS
    /* If we have PERL_IMPLICIT_SYS we can't call getuid() et alia
     * before we have an interpreter-- and the whole point of this
     * function is to be called at such an early stage.  If you are on
     * a system with PERL_IMPLICIT_SYS but you do have a concept of
     * "tainted because running with altered effective ids', you'll
     * have to add your own checks somewhere in here.  The two most
     * known samples of 'implicitness' are Win32 and NetWare, neither
     * of which has much of concept of 'uids'. */
    int uid  = PerlProc_getuid();
    int euid = PerlProc_geteuid();
    int gid  = PerlProc_getgid();
    int egid = PerlProc_getegid();
    (void)envp;

#ifdef VMS
    uid  |=  gid << 16;
    euid |= egid << 16;
#endif
    if (uid && (euid != uid || egid != gid))
	return 1;
#endif /* !PERL_IMPLICIT_SYS */
    /* This is a really primitive check; environment gets ignored only
     * if -T are the first chars together; otherwise one gets
     *  "Too late" message. */
    if ( argc > 1 && argv[1][0] == '-'
         && (argv[1][1] == 't' || argv[1][1] == 'T') )
	return 1;
    return 0;
}

/* Passing the flag as a single char rather than a string is a slight space
   optimisation.  The only message that isn't /^-.$/ is
   "program input from stdin", which is substituted in place of '\0', which
   could never be a command line flag.  */
STATIC void
S_forbid_setid(pTHX_ const char flag, const bool suidscript) /* g */
{
    dVAR;
    char string[3] = "-x";
    const char *message = "program input from stdin";

    if (flag) {
	string[1] = flag;
	message = string;
    }

#ifdef SETUID_SCRIPTS_ARE_SECURE_NOW
    if (PerlProc_getuid() != PerlProc_geteuid())
        Perl_croak(aTHX_ "No %s allowed while running setuid", message);
    if (PerlProc_getgid() != PerlProc_getegid())
        Perl_croak(aTHX_ "No %s allowed while running setgid", message);
#endif /* SETUID_SCRIPTS_ARE_SECURE_NOW */
    if (suidscript)
        Perl_croak(aTHX_ "No %s allowed with (suid) fdscript", message);
}

void
Perl_init_dbargs(pTHX)
{
    AV *const args = PL_dbargs = GvAV(gv_AVadd((gv_fetchpvs("DB::args",
							    GV_ADDMULTI,
							    SVt_PVAV))));

    if (AvREAL(args)) {
	/* Someone has already created it.
	   It might have entries, and if we just turn off AvREAL(), they will
	   "leak" until global destruction.  */
	av_clear(args);
	if (SvTIED_mg((const SV *)args, PERL_MAGIC_tied))
	    Perl_croak(aTHX_ "Cannot set tied @DB::args");
    }
    AvREIFY_only(PL_dbargs);
}

void
Perl_init_debugger(pTHX)
{
    dVAR;
    HV * const ostash = PL_curstash;

    PL_curstash = (HV *)SvREFCNT_inc_simple(PL_debstash);

    Perl_init_dbargs(aTHX);
    PL_DBgv = gv_fetchpvs("DB::DB", GV_ADDMULTI, SVt_PVGV);
    PL_DBline = gv_fetchpvs("DB::dbline", GV_ADDMULTI, SVt_PVAV);
    PL_DBsub = gv_HVadd(gv_fetchpvs("DB::sub", GV_ADDMULTI, SVt_PVHV));
    PL_DBsingle = GvSV((gv_fetchpvs("DB::single", GV_ADDMULTI, SVt_PV)));
    if (!SvIOK(PL_DBsingle))
	sv_setiv(PL_DBsingle, 0);
    PL_DBtrace = GvSV((gv_fetchpvs("DB::trace", GV_ADDMULTI, SVt_PV)));
    if (!SvIOK(PL_DBtrace))
	sv_setiv(PL_DBtrace, 0);
    PL_DBsignal = GvSV((gv_fetchpvs("DB::signal", GV_ADDMULTI, SVt_PV)));
    if (!SvIOK(PL_DBsignal))
	sv_setiv(PL_DBsignal, 0);
    SvREFCNT_dec(PL_curstash);
    PL_curstash = ostash;
}

#ifndef STRESS_REALLOC
#define REASONABLE(size) (size)
#else
#define REASONABLE(size) (1) /* unreasonable */
#endif

void
Perl_init_stacks(pTHX)
{
    dVAR;
    /* start with 128-item stack and 8K cxstack */
    PL_curstackinfo = new_stackinfo(REASONABLE(128),
				 REASONABLE(8192/sizeof(PERL_CONTEXT) - 1));
    PL_curstackinfo->si_type = PERLSI_MAIN;
    PL_curstack = PL_curstackinfo->si_stack;
    PL_mainstack = PL_curstack;		/* remember in case we switch stacks */

    PL_stack_base = AvARRAY(PL_curstack);
    PL_stack_sp = PL_stack_base;
    PL_stack_max = PL_stack_base + AvMAX(PL_curstack);

    Newx(PL_tmps_stack,REASONABLE(128),SV*);
    PL_tmps_floor = -1;
    PL_tmps_ix = -1;
    PL_tmps_max = REASONABLE(128);

    Newx(PL_markstack,REASONABLE(32),I32);
    PL_markstack_ptr = PL_markstack;
    PL_markstack_max = PL_markstack + REASONABLE(32);

    SET_MARK_OFFSET;

    Newx(PL_scopestack,REASONABLE(32),I32);
#ifdef DEBUGGING
    Newx(PL_scopestack_name,REASONABLE(32),const char*);
#endif
    PL_scopestack_ix = 0;
    PL_scopestack_max = REASONABLE(32);

    Newx(PL_savestack,REASONABLE(128),ANY);
    PL_savestack_ix = 0;
    PL_savestack_max = REASONABLE(128);
}

#undef REASONABLE

STATIC void
S_nuke_stacks(pTHX)
{
    dVAR;
    while (PL_curstackinfo->si_next)
	PL_curstackinfo = PL_curstackinfo->si_next;
    while (PL_curstackinfo) {
	PERL_SI *p = PL_curstackinfo->si_prev;
	/* curstackinfo->si_stack got nuked by sv_free_arenas() */
	Safefree(PL_curstackinfo->si_cxstack);
	Safefree(PL_curstackinfo);
	PL_curstackinfo = p;
    }
    Safefree(PL_tmps_stack);
    Safefree(PL_markstack);
    Safefree(PL_scopestack);
#ifdef DEBUGGING
    Safefree(PL_scopestack_name);
#endif
    Safefree(PL_savestack);
}

void
Perl_populate_isa(pTHX_ const char *name, STRLEN len, ...)
{
    GV *const gv = gv_fetchpvn(name, len, GV_ADD | GV_ADDMULTI, SVt_PVAV);
    AV *const isa = GvAVn(gv);
    va_list args;

    PERL_ARGS_ASSERT_POPULATE_ISA;

    if(AvFILLp(isa) != -1)
	return;

    /* NOTE: No support for tied ISA */

    va_start(args, len);
    do {
	const char *const parent = va_arg(args, const char*);
	size_t parent_len;

	if (!parent)
	    break;
	parent_len = va_arg(args, size_t);

	/* Arguments are supplied with a trailing ::  */
	assert(parent_len > 2);
	assert(parent[parent_len - 1] == ':');
	assert(parent[parent_len - 2] == ':');
	av_push(isa, newSVpvn(parent, parent_len - 2));
	(void) gv_fetchpvn(parent, parent_len, GV_ADD, SVt_PVGV);
    } while (1);
    va_end(args);
}


STATIC void
S_init_predump_symbols(pTHX)
{
    dVAR;
    GV *tmpgv;
    IO *io;

    sv_setpvs(get_sv("\"", GV_ADD), " ");
    PL_ofsgv = (GV*)SvREFCNT_inc(gv_fetchpvs(",", GV_ADD|GV_NOTQUAL, SVt_PV));


    /* Historically, PVIOs were blessed into IO::Handle, unless
       FileHandle was loaded, in which case they were blessed into
       that. Action at a distance.
       However, if we simply bless into IO::Handle, we break code
       that assumes that PVIOs will have (among others) a seek
       method. IO::File inherits from IO::Handle and IO::Seekable,
       and provides the needed methods. But if we simply bless into
       it, then we break code that assumed that by loading
       IO::Handle, *it* would work.
       So a compromise is to set up the correct @IO::File::ISA,
       so that code that does C<use IO::Handle>; will still work.
    */

    Perl_populate_isa(aTHX_ STR_WITH_LEN("IO::File::ISA"),
		      STR_WITH_LEN("IO::Handle::"),
		      STR_WITH_LEN("IO::Seekable::"),
		      STR_WITH_LEN("Exporter::"),
		      NULL);

    PL_stdingv = gv_fetchpvs("STDIN", GV_ADD|GV_NOTQUAL, SVt_PVIO);
    GvMULTI_on(PL_stdingv);
    io = GvIOp(PL_stdingv);
    IoTYPE(io) = IoTYPE_RDONLY;
    IoIFP(io) = PerlIO_stdin();
    tmpgv = gv_fetchpvs("stdin", GV_ADD|GV_NOTQUAL, SVt_PV);
    GvMULTI_on(tmpgv);
    GvIOp(tmpgv) = MUTABLE_IO(SvREFCNT_inc_simple(io));

    tmpgv = gv_fetchpvs("STDOUT", GV_ADD|GV_NOTQUAL, SVt_PVIO);
    GvMULTI_on(tmpgv);
    io = GvIOp(tmpgv);
    IoTYPE(io) = IoTYPE_WRONLY;
    IoOFP(io) = IoIFP(io) = PerlIO_stdout();
    setdefout(tmpgv);
    tmpgv = gv_fetchpvs("stdout", GV_ADD|GV_NOTQUAL, SVt_PV);
    GvMULTI_on(tmpgv);
    GvIOp(tmpgv) = MUTABLE_IO(SvREFCNT_inc_simple(io));

    PL_stderrgv = gv_fetchpvs("STDERR", GV_ADD|GV_NOTQUAL, SVt_PVIO);
    GvMULTI_on(PL_stderrgv);
    io = GvIOp(PL_stderrgv);
    IoTYPE(io) = IoTYPE_WRONLY;
    IoOFP(io) = IoIFP(io) = PerlIO_stderr();
    tmpgv = gv_fetchpvs("stderr", GV_ADD|GV_NOTQUAL, SVt_PV);
    GvMULTI_on(tmpgv);
    GvIOp(tmpgv) = MUTABLE_IO(SvREFCNT_inc_simple(io));

    PL_statname = newSVpvs("");		/* last filename we did stat on */
}

void
Perl_init_argv_symbols(pTHX_ register int argc, register char **argv)
{
    dVAR;

    PERL_ARGS_ASSERT_INIT_ARGV_SYMBOLS;

    argc--,argv++;	/* skip name of script */
    if (PL_doswitches) {
	for (; argc > 0 && **argv == '-'; argc--,argv++) {
	    char *s;
	    if (!argv[0][1])
		break;
	    if (argv[0][1] == '-' && !argv[0][2]) {
		argc--,argv++;
		break;
	    }
	    if ((s = strchr(argv[0], '='))) {
		const char *const start_name = argv[0] + 1;
		sv_setpv(GvSV(gv_fetchpvn_flags(start_name, s - start_name,
						TRUE, SVt_PV)), s + 1);
	    }
	    else
		sv_setiv(GvSV(gv_fetchpv(argv[0]+1, GV_ADD, SVt_PV)),1);
	}
    }
    if ((PL_argvgv = gv_fetchpvs("ARGV", GV_ADD|GV_NOTQUAL, SVt_PVAV))) {
	GvMULTI_on(PL_argvgv);
	(void)gv_AVadd(PL_argvgv);
	av_clear(GvAVn(PL_argvgv));
	for (; argc > 0; argc--,argv++) {
	    SV * const sv = newSVpv(argv[0],0);
	    av_push(GvAVn(PL_argvgv),sv);
	    if (!(PL_unicode & PERL_UNICODE_LOCALE_FLAG) || PL_utf8locale) {
		 if (PL_unicode & PERL_UNICODE_ARGV_FLAG)
		      SvUTF8_on(sv);
	    }
	    if (PL_unicode & PERL_UNICODE_WIDESYSCALLS_FLAG) /* Sarathy? */
		 (void)sv_utf8_decode(sv);
	}
    }
}

STATIC void
S_init_postdump_symbols(pTHX_ register int argc, register char **argv, register char **env)
{
    dVAR;
    GV* tmpgv;

    PERL_ARGS_ASSERT_INIT_POSTDUMP_SYMBOLS;

    PL_toptarget = newSV_type(SVt_PVFM);
    sv_setpvs(PL_toptarget, "");
    PL_bodytarget = newSV_type(SVt_PVFM);
    sv_setpvs(PL_bodytarget, "");
    PL_formtarget = PL_bodytarget;

    TAINT;

    init_argv_symbols(argc,argv);

    if ((tmpgv = gv_fetchpvs("0", GV_ADD|GV_NOTQUAL, SVt_PV))) {
	sv_setpv(GvSV(tmpgv),PL_origfilename);
    }
    if ((PL_envgv = gv_fetchpvs("ENV", GV_ADD|GV_NOTQUAL, SVt_PVHV))) {
	HV *hv;
	bool env_is_not_environ;
	GvMULTI_on(PL_envgv);
	hv = GvHVn(PL_envgv);
	hv_magic(hv, NULL, PERL_MAGIC_env);
#ifndef PERL_MICRO
#ifdef USE_ENVIRON_ARRAY
	/* Note that if the supplied env parameter is actually a copy
	   of the global environ then it may now point to free'd memory
	   if the environment has been modified since. To avoid this
	   problem we treat env==NULL as meaning 'use the default'
	*/
	if (!env)
	    env = environ;
	env_is_not_environ = env != environ;
	if (env_is_not_environ
#  ifdef USE_ITHREADS
	    && PL_curinterp == aTHX
#  endif
	   )
	{
	    environ[0] = NULL;
	}
	if (env) {
	  char *s, *old_var;
	  SV *sv;
	  for (; *env; env++) {
	    old_var = *env;

	    if (!(s = strchr(old_var,'=')) || s == old_var)
		continue;

#if defined(MSDOS) && !defined(DJGPP)
	    *s = '\0';
	    (void)strupr(old_var);
	    *s = '=';
#endif
	    sv = newSVpv(s+1, 0);
	    (void)hv_store(hv, old_var, s - old_var, sv, 0);
	    if (env_is_not_environ)
	        mg_set(sv);
	  }
      }
#endif /* USE_ENVIRON_ARRAY */
#endif /* !PERL_MICRO */
    }
    TAINT_NOT;

    /* touch @F array to prevent spurious warnings 20020415 MJD */
    if (PL_minus_a) {
      (void) get_av("main::F", GV_ADD | GV_ADDMULTI);
    }
}

STATIC void
S_init_perllib(pTHX)
{
    dVAR;
#ifndef VMS
    const char *perl5lib = NULL;
#endif
    const char *s;
#if defined(WIN32) && !defined(PERL_IS_MINIPERL)
    STRLEN len;
#endif

    if (!PL_tainting) {
#ifndef VMS
	perl5lib = PerlEnv_getenv("PERL5LIB");
/*
 * It isn't possible to delete an environment variable with
 * PERL_USE_SAFE_PUTENV set unless unsetenv() is also available, so in that
 * case we treat PERL5LIB as undefined if it has a zero-length value.
 */
#if defined(PERL_USE_SAFE_PUTENV) && ! defined(HAS_UNSETENV)
	if (perl5lib && *perl5lib != '\0')
#else
	if (perl5lib)
#endif
	    incpush_use_sep(perl5lib, 0, INCPUSH_ADD_SUB_DIRS);
	else {
	    s = PerlEnv_getenv("PERLLIB");
	    if (s)
		incpush_use_sep(s, 0, 0);
	}
#else /* VMS */
	/* Treat PERL5?LIB as a possible search list logical name -- the
	 * "natural" VMS idiom for a Unix path string.  We allow each
	 * element to be a set of |-separated directories for compatibility.
	 */
	char buf[256];
	int idx = 0;
	if (my_trnlnm("PERL5LIB",buf,0))
	    do {
		incpush_use_sep(buf, 0, INCPUSH_ADD_SUB_DIRS);
	    } while (my_trnlnm("PERL5LIB",buf,++idx));
	else {
	    while (my_trnlnm("PERLLIB",buf,idx++))
		incpush_use_sep(buf, 0, 0);
	}
#endif /* VMS */
    }

#ifndef PERL_IS_MINIPERL
    /* miniperl gets just -I..., the split of $ENV{PERL5LIB}, and "." in @INC
       (and not the architecture specific directories from $ENV{PERL5LIB}) */

/* Use the ~-expanded versions of APPLLIB (undocumented),
    SITEARCH, SITELIB, VENDORARCH, VENDORLIB, ARCHLIB and PRIVLIB
*/
#ifdef APPLLIB_EXP
    S_incpush_use_sep(aTHX_ STR_WITH_LEN(APPLLIB_EXP),
		      INCPUSH_ADD_SUB_DIRS|INCPUSH_CAN_RELOCATE);
#endif

#ifdef SITEARCH_EXP
    /* sitearch is always relative to sitelib on Windows for
     * DLL-based path intuition to work correctly */
#  if !defined(WIN32)
	S_incpush_use_sep(aTHX_ STR_WITH_LEN(SITEARCH_EXP),
			  INCPUSH_CAN_RELOCATE);
#  endif
#endif

#ifdef SITELIB_EXP
#  if defined(WIN32)
    /* this picks up sitearch as well */
	s = win32_get_sitelib(PERL_FS_VERSION, &len);
	if (s)
	    incpush_use_sep(s, len, INCPUSH_ADD_SUB_DIRS|INCPUSH_CAN_RELOCATE);
#  else
	S_incpush_use_sep(aTHX_ STR_WITH_LEN(SITELIB_EXP), INCPUSH_CAN_RELOCATE);
#  endif
#endif

#ifdef PERL_VENDORARCH_EXP
    /* vendorarch is always relative to vendorlib on Windows for
     * DLL-based path intuition to work correctly */
#  if !defined(WIN32)
    S_incpush_use_sep(aTHX_ STR_WITH_LEN(PERL_VENDORARCH_EXP),
		      INCPUSH_CAN_RELOCATE);
#  endif
#endif

#ifdef PERL_VENDORLIB_EXP
#  if defined(WIN32)
    /* this picks up vendorarch as well */
	s = win32_get_vendorlib(PERL_FS_VERSION, &len);
	if (s)
	    incpush_use_sep(s, len, INCPUSH_ADD_SUB_DIRS|INCPUSH_CAN_RELOCATE);
#  else
	S_incpush_use_sep(aTHX_ STR_WITH_LEN(PERL_VENDORLIB_EXP),
			  INCPUSH_CAN_RELOCATE);
#  endif
#endif

#ifdef ARCHLIB_EXP
    S_incpush_use_sep(aTHX_ STR_WITH_LEN(ARCHLIB_EXP), INCPUSH_CAN_RELOCATE);
#endif

#ifndef PRIVLIB_EXP
#  define PRIVLIB_EXP "/usr/local/lib/perl5:/usr/local/lib/perl"
#endif

#if defined(WIN32)
    s = win32_get_privlib(PERL_FS_VERSION, &len);
    if (s)
	incpush_use_sep(s, len, INCPUSH_ADD_SUB_DIRS|INCPUSH_CAN_RELOCATE);
#else
#  ifdef NETWARE
    S_incpush_use_sep(aTHX_ PRIVLIB_EXP, 0, INCPUSH_CAN_RELOCATE);
#  else
    S_incpush_use_sep(aTHX_ STR_WITH_LEN(PRIVLIB_EXP), INCPUSH_CAN_RELOCATE);
#  endif
#endif

#ifdef PERL_OTHERLIBDIRS
    S_incpush_use_sep(aTHX_ STR_WITH_LEN(PERL_OTHERLIBDIRS),
		      INCPUSH_ADD_VERSIONED_SUB_DIRS|INCPUSH_NOT_BASEDIR
		      |INCPUSH_CAN_RELOCATE);
#endif

    if (!PL_tainting) {
#ifndef VMS
/*
 * It isn't possible to delete an environment variable with
 * PERL_USE_SAFE_PUTENV set unless unsetenv() is also available, so in that
 * case we treat PERL5LIB as undefined if it has a zero-length value.
 */
#if defined(PERL_USE_SAFE_PUTENV) && ! defined(HAS_UNSETENV)
	if (perl5lib && *perl5lib != '\0')
#else
	if (perl5lib)
#endif
	    incpush_use_sep(perl5lib, 0,
			    INCPUSH_ADD_OLD_VERS|INCPUSH_NOT_BASEDIR);
#else /* VMS */
	/* Treat PERL5?LIB as a possible search list logical name -- the
	 * "natural" VMS idiom for a Unix path string.  We allow each
	 * element to be a set of |-separated directories for compatibility.
	 */
	char buf[256];
	int idx = 0;
	if (my_trnlnm("PERL5LIB",buf,0))
	    do {
		incpush_use_sep(buf, 0,
				INCPUSH_ADD_OLD_VERS|INCPUSH_NOT_BASEDIR);
	    } while (my_trnlnm("PERL5LIB",buf,++idx));
#endif /* VMS */
    }

/* Use the ~-expanded versions of APPLLIB (undocumented),
    SITELIB and VENDORLIB for older versions
*/
#ifdef APPLLIB_EXP
    S_incpush_use_sep(aTHX_ STR_WITH_LEN(APPLLIB_EXP), INCPUSH_ADD_OLD_VERS
		      |INCPUSH_NOT_BASEDIR|INCPUSH_CAN_RELOCATE);
#endif

#if defined(SITELIB_STEM) && defined(PERL_INC_VERSION_LIST)
    /* Search for version-specific dirs below here */
    S_incpush_use_sep(aTHX_ STR_WITH_LEN(SITELIB_STEM),
		      INCPUSH_ADD_OLD_VERS|INCPUSH_CAN_RELOCATE);
#endif


#if defined(PERL_VENDORLIB_STEM) && defined(PERL_INC_VERSION_LIST)
    /* Search for version-specific dirs below here */
    S_incpush_use_sep(aTHX_ STR_WITH_LEN(PERL_VENDORLIB_STEM),
		      INCPUSH_ADD_OLD_VERS|INCPUSH_CAN_RELOCATE);
#endif

#ifdef PERL_OTHERLIBDIRS
    S_incpush_use_sep(aTHX_ STR_WITH_LEN(PERL_OTHERLIBDIRS),
		      INCPUSH_ADD_OLD_VERS|INCPUSH_ADD_ARCHONLY_SUB_DIRS
		      |INCPUSH_CAN_RELOCATE);
#endif
#endif /* !PERL_IS_MINIPERL */

    if (!PL_tainting)
	S_incpush(aTHX_ STR_WITH_LEN("."), 0);
}

#if defined(DOSISH) || defined(EPOC) || defined(__SYMBIAN32__)
#    define PERLLIB_SEP ';'
#else
#  if defined(VMS)
#    define PERLLIB_SEP '|'
#  else
#    define PERLLIB_SEP ':'
#  endif
#endif
#ifndef PERLLIB_MANGLE
#  define PERLLIB_MANGLE(s,n) (s)
#endif

#ifndef PERL_IS_MINIPERL
/* Push a directory onto @INC if it exists.
   Generate a new SV if we do this, to save needing to copy the SV we push
   onto @INC  */
STATIC SV *
S_incpush_if_exists(pTHX_ AV *const av, SV *dir, SV *const stem)
{
    dVAR;
    Stat_t tmpstatbuf;

    PERL_ARGS_ASSERT_INCPUSH_IF_EXISTS;

    if (PerlLIO_stat(SvPVX_const(dir), &tmpstatbuf) >= 0 &&
	S_ISDIR(tmpstatbuf.st_mode)) {
	av_push(av, dir);
	dir = newSVsv(stem);
    } else {
	/* Truncate dir back to stem.  */
	SvCUR_set(dir, SvCUR(stem));
    }
    return dir;
}
#endif

STATIC SV *
S_mayberelocate(pTHX_ const char *const dir, STRLEN len, U32 flags)
{
    const U8 canrelocate = (U8)flags & INCPUSH_CAN_RELOCATE;
    SV *libdir;

    PERL_ARGS_ASSERT_MAYBERELOCATE;
    assert(len > 0);

	if (len) {
	    /* I am not convinced that this is valid when PERLLIB_MANGLE is
	       defined to so something (in os2/os2.c), but the code has been
	       this way, ignoring any possible changed of length, since
	       760ac839baf413929cd31cc32ffd6dba6b781a81 (5.003_02) so I'll leave
	       it be.  */
	    libdir = newSVpvn(PERLLIB_MANGLE(dir, len), len);
	} else {
	    libdir = newSVpv(PERLLIB_MANGLE(dir, 0), 0);
	}

#ifdef VMS
    {
	char *unix;

	if ((unix = tounixspec_ts(SvPV(libdir,len),NULL)) != NULL) {
	    len = strlen(unix);
	    while (unix[len-1] == '/') len--;  /* Cosmetic */
	    sv_usepvn(libdir,unix,len);
	}
	else
	    PerlIO_printf(Perl_error_log,
		          "Failed to unixify @INC element \"%s\"\n",
			  SvPV_nolen_const(libdir));
    }
#endif

	/* Do the if() outside the #ifdef to avoid warnings about an unused
	   parameter.  */
	if (canrelocate) {
#ifdef PERL_RELOCATABLE_INC
	/*
	 * Relocatable include entries are marked with a leading .../
	 *
	 * The algorithm is
	 * 0: Remove that leading ".../"
	 * 1: Remove trailing executable name (anything after the last '/')
	 *    from the perl path to give a perl prefix
	 * Then
	 * While the @INC element starts "../" and the prefix ends with a real
	 * directory (ie not . or ..) chop that real directory off the prefix
	 * and the leading "../" from the @INC element. ie a logical "../"
	 * cleanup
	 * Finally concatenate the prefix and the remainder of the @INC element
	 * The intent is that /usr/local/bin/perl and .../../lib/perl5
	 * generates /usr/local/lib/perl5
	 */
	    const char *libpath = SvPVX(libdir);
	    STRLEN libpath_len = SvCUR(libdir);
	    if (libpath_len >= 4 && memEQ (libpath, ".../", 4)) {
		/* Game on!  */
		SV * const caret_X = get_sv("\030", 0);
		/* Going to use the SV just as a scratch buffer holding a C
		   string:  */
		SV *prefix_sv;
		char *prefix;
		char *lastslash;

		/* $^X is *the* source of taint if tainting is on, hence
		   SvPOK() won't be true.  */
		assert(caret_X);
		assert(SvPOKp(caret_X));
		prefix_sv = newSVpvn_flags(SvPVX(caret_X), SvCUR(caret_X),
					   SvUTF8(caret_X));
		/* Firstly take off the leading .../
		   If all else fail we'll do the paths relative to the current
		   directory.  */
		sv_chop(libdir, libpath + 4);
		/* Don't use SvPV as we're intentionally bypassing taining,
		   mortal copies that the mg_get of tainting creates, and
		   corruption that seems to come via the save stack.
		   I guess that the save stack isn't correctly set up yet.  */
		libpath = SvPVX(libdir);
		libpath_len = SvCUR(libdir);

		/* This would work more efficiently with memrchr, but as it's
		   only a GNU extension we'd need to probe for it and
		   implement our own. Not hard, but maybe not worth it?  */

		prefix = SvPVX(prefix_sv);
		lastslash = strrchr(prefix, '/');

		/* First time in with the *lastslash = '\0' we just wipe off
		   the trailing /perl from (say) /usr/foo/bin/perl
		*/
		if (lastslash) {
		    SV *tempsv;
		    while ((*lastslash = '\0'), /* Do that, come what may.  */
			   (libpath_len >= 3 && memEQ(libpath, "../", 3)
			    && (lastslash = strrchr(prefix, '/')))) {
			if (lastslash[1] == '\0'
			    || (lastslash[1] == '.'
				&& (lastslash[2] == '/' /* ends "/."  */
				    || (lastslash[2] == '/'
					&& lastslash[3] == '/' /* or "/.."  */
					)))) {
			    /* Prefix ends "/" or "/." or "/..", any of which
			       are fishy, so don't do any more logical cleanup.
			    */
			    break;
			}
			/* Remove leading "../" from path  */
			libpath += 3;
			libpath_len -= 3;
			/* Next iteration round the loop removes the last
			   directory name from prefix by writing a '\0' in
			   the while clause.  */
		    }
		    /* prefix has been terminated with a '\0' to the correct
		       length. libpath points somewhere into the libdir SV.
		       We need to join the 2 with '/' and drop the result into
		       libdir.  */
		    tempsv = Perl_newSVpvf(aTHX_ "%s/%s", prefix, libpath);
		    SvREFCNT_dec(libdir);
		    /* And this is the new libdir.  */
		    libdir = tempsv;
		    if (PL_tainting &&
			(PerlProc_getuid() != PerlProc_geteuid() ||
			 PerlProc_getgid() != PerlProc_getegid())) {
			/* Need to taint relocated paths if running set ID  */
			SvTAINTED_on(libdir);
		    }
		}
		SvREFCNT_dec(prefix_sv);
	    }
#endif
	}
    return libdir;
}

STATIC void
S_incpush(pTHX_ const char *const dir, STRLEN len, U32 flags)
{
    dVAR;
#ifndef PERL_IS_MINIPERL
    const U8 using_sub_dirs
	= (U8)flags & (INCPUSH_ADD_VERSIONED_SUB_DIRS
		       |INCPUSH_ADD_ARCHONLY_SUB_DIRS|INCPUSH_ADD_OLD_VERS);
    const U8 add_versioned_sub_dirs
	= (U8)flags & INCPUSH_ADD_VERSIONED_SUB_DIRS;
    const U8 add_archonly_sub_dirs
	= (U8)flags & INCPUSH_ADD_ARCHONLY_SUB_DIRS;
#ifdef PERL_INC_VERSION_LIST
    const U8 addoldvers  = (U8)flags & INCPUSH_ADD_OLD_VERS;
#endif
#endif
    const U8 unshift     = (U8)flags & INCPUSH_UNSHIFT;
    const U8 push_basedir = (flags & INCPUSH_NOT_BASEDIR) ? 0 : 1;
    AV *const inc = GvAVn(PL_incgv);

    PERL_ARGS_ASSERT_INCPUSH;
    assert(len > 0);

    /* Could remove this vestigial extra block, if we don't mind a lot of
       re-indenting diff noise.  */
    {
	SV *const libdir = mayberelocate(dir, len, flags);
	/* Change 20189146be79a0596543441fa369c6bf7f85103f, to fix RT#6665,
	   arranged to unshift #! line -I onto the front of @INC. However,
	   -I can add version and architecture specific libraries, and they
	   need to go first. The old code assumed that it was always
	   pushing. Hence to make it work, need to push the architecture
	   (etc) libraries onto a temporary array, then "unshift" that onto
	   the front of @INC.  */
#ifndef PERL_IS_MINIPERL
	AV *const av = (using_sub_dirs) ? (unshift ? newAV() : inc) : NULL;

	/*
	 * BEFORE pushing libdir onto @INC we may first push version- and
	 * archname-specific sub-directories.
	 */
	if (using_sub_dirs) {
	    SV *subdir = newSVsv(libdir);
#ifdef PERL_INC_VERSION_LIST
	    /* Configure terminates PERL_INC_VERSION_LIST with a NULL */
	    const char * const incverlist[] = { PERL_INC_VERSION_LIST };
	    const char * const *incver;
#endif

	    if (add_versioned_sub_dirs) {
		/* .../version/archname if -d .../version/archname */
		sv_catpvs(subdir, "/" PERL_FS_VERSION "/" ARCHNAME);
		subdir = S_incpush_if_exists(aTHX_ av, subdir, libdir);

		/* .../version if -d .../version */
		sv_catpvs(subdir, "/" PERL_FS_VERSION);
		subdir = S_incpush_if_exists(aTHX_ av, subdir, libdir);
	    }

#ifdef PERL_INC_VERSION_LIST
	    if (addoldvers) {
		for (incver = incverlist; *incver; incver++) {
		    /* .../xxx if -d .../xxx */
		    Perl_sv_catpvf(aTHX_ subdir, "/%s", *incver);
		    subdir = S_incpush_if_exists(aTHX_ av, subdir, libdir);
		}
	    }
#endif

	    if (add_archonly_sub_dirs) {
		/* .../archname if -d .../archname */
		sv_catpvs(subdir, "/" ARCHNAME);
		subdir = S_incpush_if_exists(aTHX_ av, subdir, libdir);

	    }

	    assert (SvREFCNT(subdir) == 1);
	    SvREFCNT_dec(subdir);
	}
#endif /* !PERL_IS_MINIPERL */
	/* finally add this lib directory at the end of @INC */
	if (unshift) {
#ifdef PERL_IS_MINIPERL
	    const U32 extra = 0;
#else
	    U32 extra = av_len(av) + 1;
#endif
	    av_unshift(inc, extra + push_basedir);
	    if (push_basedir)
		av_store(inc, extra, libdir);
#ifndef PERL_IS_MINIPERL
	    while (extra--) {
		/* av owns a reference, av_store() expects to be donated a
		   reference, and av expects to be sane when it's cleared.
		   If I wanted to be naughty and wrong, I could peek inside the
		   implementation of av_clear(), realise that it uses
		   SvREFCNT_dec() too, so av's array could be a run of NULLs,
		   and so directly steal from it (with a memcpy() to inc, and
		   then memset() to NULL them out. But people copy code from the
		   core expecting it to be best practise, so let's use the API.
		   Although studious readers will note that I'm not checking any
		   return codes.  */
		av_store(inc, extra, SvREFCNT_inc(*av_fetch(av, extra, FALSE)));
	    }
	    SvREFCNT_dec(av);
#endif
	}
	else if (push_basedir) {
	    av_push(inc, libdir);
	}

	if (!push_basedir) {
	    assert (SvREFCNT(libdir) == 1);
	    SvREFCNT_dec(libdir);
	}
    }
}

STATIC void
S_incpush_use_sep(pTHX_ const char *p, STRLEN len, U32 flags)
{
    const char *s;
    const char *end;
    /* This logic has been broken out from S_incpush(). It may be possible to
       simplify it.  */

    PERL_ARGS_ASSERT_INCPUSH_USE_SEP;

    /* perl compiled with -DPERL_RELOCATABLE_INCPUSH will ignore the len
     * argument to incpush_use_sep.  This allows creation of relocatable
     * Perl distributions that patch the binary at install time.  Those
     * distributions will have to provide their own relocation tools; this
     * is not a feature otherwise supported by core Perl.
     */
#ifndef PERL_RELOCATABLE_INCPUSH
    if (!len)
#endif
	len = strlen(p);

    end = p + len;

    /* Break at all separators */
    while ((s = (const char*)memchr(p, PERLLIB_SEP, end - p))) {
	if (s == p) {
	    /* skip any consecutive separators */

	    /* Uncomment the next line for PATH semantics */
	    /* But you'll need to write tests */
	    /* av_push(GvAVn(PL_incgv), newSVpvs(".")); */
	} else {
	    incpush(p, (STRLEN)(s - p), flags);
	}
	p = s + 1;
    }
    if (p != end)
	incpush(p, (STRLEN)(end - p), flags);

}

void
Perl_call_list(pTHX_ I32 oldscope, AV *paramList)
{
    dVAR;
    SV *atsv;
    volatile const line_t oldline = PL_curcop ? CopLINE(PL_curcop) : 0;
    CV *cv;
    STRLEN len;
    int ret;
    dJMPENV;

    PERL_ARGS_ASSERT_CALL_LIST;

    while (av_len(paramList) >= 0) {
	cv = MUTABLE_CV(av_shift(paramList));
	if (PL_savebegin) {
	    if (paramList == PL_beginav) {
		/* save PL_beginav for compiler */
		Perl_av_create_and_push(aTHX_ &PL_beginav_save, MUTABLE_SV(cv));
	    }
	    else if (paramList == PL_checkav) {
		/* save PL_checkav for compiler */
		Perl_av_create_and_push(aTHX_ &PL_checkav_save, MUTABLE_SV(cv));
	    }
	    else if (paramList == PL_unitcheckav) {
		/* save PL_unitcheckav for compiler */
		Perl_av_create_and_push(aTHX_ &PL_unitcheckav_save, MUTABLE_SV(cv));
	    }
	} else {
	    if (!PL_madskills)
		SAVEFREESV(cv);
	}
	JMPENV_PUSH(ret);
	switch (ret) {
	case 0:
#ifdef PERL_MAD
	    if (PL_madskills)
		PL_madskills |= 16384;
#endif
	    CALL_LIST_BODY(cv);
#ifdef PERL_MAD
	    if (PL_madskills)
		PL_madskills &= ~16384;
#endif
	    atsv = ERRSV;
	    (void)SvPV_const(atsv, len);
	    if (len) {
		PL_curcop = &PL_compiling;
		CopLINE_set(PL_curcop, oldline);
		if (paramList == PL_beginav)
		    sv_catpvs(atsv, "BEGIN failed--compilation aborted");
		else
		    Perl_sv_catpvf(aTHX_ atsv,
				   "%s failed--call queue aborted",
				   paramList == PL_checkav ? "CHECK"
				   : paramList == PL_initav ? "INIT"
				   : paramList == PL_unitcheckav ? "UNITCHECK"
				   : "END");
		while (PL_scopestack_ix > oldscope)
		    LEAVE;
		JMPENV_POP;
		Perl_croak(aTHX_ "%"SVf"", SVfARG(atsv));
	    }
	    break;
	case 1:
	    STATUS_ALL_FAILURE;
	    /* FALL THROUGH */
	case 2:
	    /* my_exit() was called */
	    while (PL_scopestack_ix > oldscope)
		LEAVE;
	    FREETMPS;
	    SET_CURSTASH(PL_defstash);
	    PL_curcop = &PL_compiling;
	    CopLINE_set(PL_curcop, oldline);
	    JMPENV_POP;
	    my_exit_jump();
	    /* NOTREACHED */
	case 3:
	    if (PL_restartop) {
		PL_curcop = &PL_compiling;
		CopLINE_set(PL_curcop, oldline);
		JMPENV_JUMP(3);
	    }
	    PerlIO_printf(Perl_error_log, "panic: restartop in call_list\n");
	    FREETMPS;
	    break;
	}
	JMPENV_POP;
    }
}

void
Perl_my_exit(pTHX_ U32 status)
{
    dVAR;
    switch (status) {
    case 0:
	STATUS_ALL_SUCCESS;
	break;
    case 1:
	STATUS_ALL_FAILURE;
	break;
    default:
	STATUS_EXIT_SET(status);
	break;
    }
    my_exit_jump();
}

void
Perl_my_failure_exit(pTHX)
{
    dVAR;
#ifdef VMS
     /* We have been called to fall on our sword.  The desired exit code
      * should be already set in STATUS_UNIX, but could be shifted over
      * by 8 bits.  STATUS_UNIX_EXIT_SET will handle the cases where a
      * that code is set.
      *
      * If an error code has not been set, then force the issue.
      */
    if (MY_POSIX_EXIT) {

        /* According to the die_exit.t tests, if errno is non-zero */
        /* It should be used for the error status. */

	if (errno == EVMSERR) {
	    STATUS_NATIVE = vaxc$errno;
	} else {

            /* According to die_exit.t tests, if the child_exit code is */
            /* also zero, then we need to exit with a code of 255 */
            if ((errno != 0) && (errno < 256))
		STATUS_UNIX_EXIT_SET(errno);
            else if (STATUS_UNIX < 255) {
		STATUS_UNIX_EXIT_SET(255);
            }

	}

	/* The exit code could have been set by $? or vmsish which
	 * means that it may not have fatal set.  So convert
	 * success/warning codes to fatal with out changing
	 * the POSIX status code.  The severity makes VMS native
	 * status handling work, while UNIX mode programs use the
	 * the POSIX exit codes.
	 */
	 if ((STATUS_NATIVE & (STS$K_SEVERE|STS$K_ERROR)) == 0) {
	    STATUS_NATIVE &= STS$M_COND_ID;
	    STATUS_NATIVE |= STS$K_ERROR | STS$M_INHIB_MSG;
         }
    }
    else {
	/* Traditionally Perl on VMS always expects a Fatal Error. */
	if (vaxc$errno & 1) {

	    /* So force success status to failure */
	    if (STATUS_NATIVE & 1)
		STATUS_ALL_FAILURE;
	}
	else {
	    if (!vaxc$errno) {
		STATUS_UNIX = EINTR; /* In case something cares */
		STATUS_ALL_FAILURE;
	    }
	    else {
		int severity;
		STATUS_NATIVE = vaxc$errno; /* Should already be this */

		/* Encode the severity code */
		severity = STATUS_NATIVE & STS$M_SEVERITY;
		STATUS_UNIX = (severity ? severity : 1) << 8;

		/* Perl expects this to be a fatal error */
		if (severity != STS$K_SEVERE)
		    STATUS_ALL_FAILURE;
	    }
	}
    }

#else
    int exitstatus;
    if (errno & 255)
	STATUS_UNIX_SET(errno);
    else {
	exitstatus = STATUS_UNIX >> 8;
	if (exitstatus & 255)
	    STATUS_UNIX_SET(exitstatus);
	else
	    STATUS_UNIX_SET(255);
    }
#endif
    my_exit_jump();
}

STATIC void
S_my_exit_jump(pTHX)
{
    dVAR;

    if (PL_e_script) {
	SvREFCNT_dec(PL_e_script);
	PL_e_script = NULL;
    }

    POPSTACK_TO(PL_mainstack);
    dounwind(-1);
    LEAVE_SCOPE(0);

    JMPENV_JUMP(2);
}

static I32
read_e_script(pTHX_ int idx, SV *buf_sv, int maxlen)
{
    dVAR;
    const char * const p  = SvPVX_const(PL_e_script);
    const char *nl = strchr(p, '\n');

    PERL_UNUSED_ARG(idx);
    PERL_UNUSED_ARG(maxlen);

    nl = (nl) ? nl+1 : SvEND(PL_e_script);
    if (nl-p == 0) {
	filter_del(read_e_script);
	return 0;
    }
    sv_catpvn(buf_sv, p, nl-p);
    sv_chop(PL_e_script, nl);
    return 1;
}

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
