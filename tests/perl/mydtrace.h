/*    mydtrace.h
 *
 *    Copyright (C) 2008, 2010, 2011 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 *	Provides macros that wrap the various DTrace probes we use. We add
 *	an extra level of wrapping to encapsulate the _ENABLED tests.
 */

#if defined(USE_DTRACE) && defined(PERL_CORE)

#  include "perldtrace.h"

#  if defined(STAP_PROBE_ADDR) && !defined(DEBUGGING)

/* SystemTap 1.2 uses a construct that chokes on passing a char array
 * as a char *, in this case hek_key in struct hek.  Workaround it
 * with a temporary.
 */

#    define ENTRY_PROBE(func, file, line, stash)  	\
    if (PERL_SUB_ENTRY_ENABLED()) {	        	\
	const char *tmp_func = func;			\
	PERL_SUB_ENTRY(tmp_func, file, line, stash); 	\
    }

#    define RETURN_PROBE(func, file, line, stash) 	\
    if (PERL_SUB_RETURN_ENABLED()) {    		\
	const char *tmp_func = func;			\
	PERL_SUB_RETURN(tmp_func, file, line, stash);	\
    }

#  else

#    define ENTRY_PROBE(func, file, line, stash) 	\
    if (PERL_SUB_ENTRY_ENABLED()) {	        	\
	PERL_SUB_ENTRY(func, file, line, stash); 	\
    }

#    define RETURN_PROBE(func, file, line, stash)	\
    if (PERL_SUB_RETURN_ENABLED()) {    		\
	PERL_SUB_RETURN(func, file, line, stash); 	\
    }

#  endif

#  define PHASE_CHANGE_PROBE(new_phase, old_phase)      \
    if (PERL_PHASE_CHANGE_ENABLED()) {                  \
	PERL_PHASE_CHANGE(new_phase, old_phase);        \
    }

#else

/* NOPs */
#  define ENTRY_PROBE(func, file, line, stash)
#  define RETURN_PROBE(func, file, line, stash)
#  define PHASE_CHANGE_PROBE(new_phase, old_phase)

#endif

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
