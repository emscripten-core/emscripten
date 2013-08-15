/*    miniperlmain.c
 *
 *    Copyright (C) 1994, 1995, 1996, 1997, 1999, 2000, 2001, 2002, 2003,
 *    2004, 2005, 2006, 2007, by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *      The Road goes ever on and on
 *          Down from the door where it began.
 *
 *     [Bilbo on p.35 of _The Lord of the Rings_, I/i: "A Long-Expected Party"]
 *     [Frodo on p.73 of _The Lord of the Rings_, I/iii: "Three Is Company"]
 */

/* This file contains the main() function for the perl interpreter.
 * Note that miniperlmain.c contains main() for the 'miniperl' binary,
 * while perlmain.c contains main() for the 'perl' binary.
 *
 * Miniperl is like perl except that it does not support dynamic loading,
 * and in fact is used to build the dynamic modules needed for the 'real'
 * perl executable.
 */

#ifdef OEMVS
#ifdef MYMALLOC
/* sbrk is limited to first heap segment so make it big */
#pragma runopts(HEAP(8M,500K,ANYWHERE,KEEP,8K,4K) STACK(,,ANY,) ALL31(ON))
#else
#pragma runopts(HEAP(2M,500K,ANYWHERE,KEEP,8K,4K) STACK(,,ANY,) ALL31(ON))
#endif
#endif


#include "EXTERN.h"
#define PERL_IN_MINIPERLMAIN_C
#include "perl.h"

static void xs_init (pTHX);
static PerlInterpreter *my_perl;

#if defined (atarist)
/* The Atari operating system doesn't have a dynamic stack.  The
   stack size is determined from this value.  */
long _stksize = 64 * 1024;
#endif

#if defined(PERL_GLOBAL_STRUCT_PRIVATE)
/* The static struct perl_vars* may seem counterproductive since the
 * whole idea PERL_GLOBAL_STRUCT_PRIVATE was to avoid statics, but note
 * that this static is not in the shared perl library, the globals PL_Vars
 * and PL_VarsPtr will stay away. */
static struct perl_vars* my_plvarsp;
struct perl_vars* Perl_GetVarsPrivate(void) { return my_plvarsp; }
#endif

#ifdef NO_ENV_ARRAY_IN_MAIN
extern char **environ;
int
main(int argc, char **argv)
#else
int
main(int argc, char **argv, char **env)
#endif
{
    dVAR;
    int exitstatus, i;
#ifdef PERL_GLOBAL_STRUCT
    struct perl_vars *plvarsp = init_global_struct();
#  ifdef PERL_GLOBAL_STRUCT_PRIVATE
    my_vars = my_plvarsp = plvarsp;
#  endif
#endif /* PERL_GLOBAL_STRUCT */
#ifndef NO_ENV_ARRAY_IN_MAIN
    PERL_UNUSED_ARG(env);
#endif
#ifndef PERL_USE_SAFE_PUTENV
    PL_use_safe_putenv = FALSE;
#endif /* PERL_USE_SAFE_PUTENV */

    /* if user wants control of gprof profiling off by default */
    /* noop unless Configure is given -Accflags=-DPERL_GPROF_CONTROL */
    PERL_GPROF_MONCONTROL(0);

#ifdef NO_ENV_ARRAY_IN_MAIN
    PERL_SYS_INIT3(&argc,&argv,&environ);
#else
    PERL_SYS_INIT3(&argc,&argv,&env);
#endif

#if defined(USE_ITHREADS)
    /* XXX Ideally, this should really be happening in perl_alloc() or
     * perl_construct() to keep libperl.a transparently fork()-safe.
     * It is currently done here only because Apache/mod_perl have
     * problems due to lack of a call to cancel pthread_atfork()
     * handlers when shared objects that contain the handlers may
     * be dlclose()d.  This forces applications that embed perl to
     * call PTHREAD_ATFORK() explicitly, but if and only if it hasn't
     * been called at least once before in the current process.
     * --GSAR 2001-07-20 */
    PTHREAD_ATFORK(Perl_atfork_lock,
                   Perl_atfork_unlock,
                   Perl_atfork_unlock);
#endif

    if (!PL_do_undump) {
	my_perl = perl_alloc();
	if (!my_perl)
	    exit(1);
	perl_construct(my_perl);
	PL_perl_destruct_level = 0;
    }
    PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
    exitstatus = perl_parse(my_perl, xs_init, argc, argv, (char **)NULL);
    if (!exitstatus)
        perl_run(my_perl);

#ifndef PERL_MICRO
    /* Unregister our signal handler before destroying my_perl */
    for (i = 0; PL_sig_name[i]; i++) {
	if (rsignal_state(PL_sig_num[i]) == (Sighandler_t) PL_csighandlerp) {
	    rsignal(PL_sig_num[i], (Sighandler_t) SIG_DFL);
	}
    }
#endif

    exitstatus = perl_destruct(my_perl);

    perl_free(my_perl);

#if defined(USE_ENVIRON_ARRAY) && defined(PERL_TRACK_MEMPOOL) && !defined(NO_ENV_ARRAY_IN_MAIN)
    /*
     * The old environment may have been freed by perl_free()
     * when PERL_TRACK_MEMPOOL is defined, but without having
     * been restored by perl_destruct() before (this is only
     * done if destruct_level > 0).
     *
     * It is important to have a valid environment for atexit()
     * routines that are eventually called.
     */
    environ = env;
#endif

#ifdef PERL_GLOBAL_STRUCT
    free_global_struct(plvarsp);
#endif /* PERL_GLOBAL_STRUCT */

    PERL_SYS_TERM();

    exit(exitstatus);
    return exitstatus;
}

/* Register any extra external extensions */

/* Do not delete this line--writemain depends on it */

static void
xs_init(pTHX)
{
    PERL_UNUSED_CONTEXT;
    dXSUB_SYS;
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
