/*    util.c
 *
 *    Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001,
 *    2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'Very useful, no doubt, that was to Saruman; yet it seems that he was
 *  not content.'                                    --Gandalf to Pippin
 *
 *     [p.598 of _The Lord of the Rings_, III/xi: "The Palantír"]
 */

/* This file contains assorted utility routines.
 * Which is a polite way of saying any stuff that people couldn't think of
 * a better place for. Amongst other things, it includes the warning and
 * dieing stuff, plus wrappers for malloc code.
 */

#include "EXTERN.h"
#define PERL_IN_UTIL_C
#include "perl.h"

#ifdef USE_PERLIO
#include "perliol.h" /* For PerlIOUnix_refcnt */
#endif

#ifndef PERL_MICRO
#include <signal.h>
#ifndef SIG_ERR
# define SIG_ERR ((Sighandler_t) -1)
#endif
#endif

#ifdef __Lynx__
/* Missing protos on LynxOS */
int putenv(char *);
#endif

#ifdef HAS_SELECT
# ifdef I_SYS_SELECT
#  include <sys/select.h>
# endif
#endif

#define FLUSH

#if defined(HAS_FCNTL) && defined(F_SETFD) && !defined(FD_CLOEXEC)
#  define FD_CLOEXEC 1			/* NeXT needs this */
#endif

/* NOTE:  Do not call the next three routines directly.  Use the macros
 * in handy.h, so that we can easily redefine everything to do tracking of
 * allocated hunks back to the original New to track down any memory leaks.
 * XXX This advice seems to be widely ignored :-(   --AD  August 1996.
 */

static char *
S_write_no_mem(pTHX)
{
    dVAR;
    /* Can't use PerlIO to write as it allocates memory */
    PerlLIO_write(PerlIO_fileno(Perl_error_log),
		  PL_no_mem, strlen(PL_no_mem));
    my_exit(1);
    NORETURN_FUNCTION_END;
}

#if defined (DEBUGGING) || defined(PERL_IMPLICIT_SYS) || defined (PERL_TRACK_MEMPOOL)
#  define ALWAYS_NEED_THX
#endif

/* paranoid version of system's malloc() */

Malloc_t
Perl_safesysmalloc(MEM_SIZE size)
{
#ifdef ALWAYS_NEED_THX
    dTHX;
#endif
    Malloc_t ptr;
#ifdef HAS_64K_LIMIT
	if (size > 0xffff) {
	    PerlIO_printf(Perl_error_log,
			  "Allocation too large: %lx\n", size) FLUSH;
	    my_exit(1);
	}
#endif /* HAS_64K_LIMIT */
#ifdef PERL_TRACK_MEMPOOL
    size += sTHX;
#endif
#ifdef DEBUGGING
    if ((SSize_t)size < 0)
	Perl_croak_nocontext("panic: malloc, size=%"UVuf, (UV) size);
#endif
    ptr = (Malloc_t)PerlMem_malloc(size?size:1);	/* malloc(0) is NASTY on our system */
    PERL_ALLOC_CHECK(ptr);
    if (ptr != NULL) {
#ifdef PERL_TRACK_MEMPOOL
	struct perl_memory_debug_header *const header
	    = (struct perl_memory_debug_header *)ptr;
#endif

#ifdef PERL_POISON
	PoisonNew(((char *)ptr), size, char);
#endif

#ifdef PERL_TRACK_MEMPOOL
	header->interpreter = aTHX;
	/* Link us into the list.  */
	header->prev = &PL_memory_debug_header;
	header->next = PL_memory_debug_header.next;
	PL_memory_debug_header.next = header;
	header->next->prev = header;
#  ifdef PERL_POISON
	header->size = size;
#  endif
        ptr = (Malloc_t)((char*)ptr+sTHX);
#endif
	DEBUG_m(PerlIO_printf(Perl_debug_log, "0x%"UVxf": (%05ld) malloc %ld bytes\n",PTR2UV(ptr),(long)PL_an++,(long)size));
	return ptr;
}
    else {
#ifndef ALWAYS_NEED_THX
	dTHX;
#endif
	if (PL_nomemok)
	    return NULL;
	else {
	    return write_no_mem();
	}
    }
    /*NOTREACHED*/
}

/* paranoid version of system's realloc() */

Malloc_t
Perl_safesysrealloc(Malloc_t where,MEM_SIZE size)
{
#ifdef ALWAYS_NEED_THX
    dTHX;
#endif
    Malloc_t ptr;
#if !defined(STANDARD_C) && !defined(HAS_REALLOC_PROTOTYPE) && !defined(PERL_MICRO)
    Malloc_t PerlMem_realloc();
#endif /* !defined(STANDARD_C) && !defined(HAS_REALLOC_PROTOTYPE) */

#ifdef HAS_64K_LIMIT
    if (size > 0xffff) {
	PerlIO_printf(Perl_error_log,
		      "Reallocation too large: %lx\n", size) FLUSH;
	my_exit(1);
    }
#endif /* HAS_64K_LIMIT */
    if (!size) {
	safesysfree(where);
	return NULL;
    }

    if (!where)
	return safesysmalloc(size);
#ifdef PERL_TRACK_MEMPOOL
    where = (Malloc_t)((char*)where-sTHX);
    size += sTHX;
    {
	struct perl_memory_debug_header *const header
	    = (struct perl_memory_debug_header *)where;

	if (header->interpreter != aTHX) {
	    Perl_croak_nocontext("panic: realloc from wrong pool, %p!=%p",
				 header->interpreter, aTHX);
	}
	assert(header->next->prev == header);
	assert(header->prev->next == header);
#  ifdef PERL_POISON
	if (header->size > size) {
	    const MEM_SIZE freed_up = header->size - size;
	    char *start_of_freed = ((char *)where) + size;
	    PoisonFree(start_of_freed, freed_up, char);
	}
	header->size = size;
#  endif
    }
#endif
#ifdef DEBUGGING
    if ((SSize_t)size < 0)
	Perl_croak_nocontext("panic: realloc, size=%"UVuf, (UV)size);
#endif
    ptr = (Malloc_t)PerlMem_realloc(where,size);
    PERL_ALLOC_CHECK(ptr);

    /* MUST do this fixup first, before doing ANYTHING else, as anything else
       might allocate memory/free/move memory, and until we do the fixup, it
       may well be chasing (and writing to) free memory.  */
#ifdef PERL_TRACK_MEMPOOL
    if (ptr != NULL) {
	struct perl_memory_debug_header *const header
	    = (struct perl_memory_debug_header *)ptr;

#  ifdef PERL_POISON
	if (header->size < size) {
	    const MEM_SIZE fresh = size - header->size;
	    char *start_of_fresh = ((char *)ptr) + size;
	    PoisonNew(start_of_fresh, fresh, char);
	}
#  endif

	header->next->prev = header;
	header->prev->next = header;

        ptr = (Malloc_t)((char*)ptr+sTHX);
    }
#endif

    /* In particular, must do that fixup above before logging anything via
     *printf(), as it can reallocate memory, which can cause SEGVs.  */

    DEBUG_m(PerlIO_printf(Perl_debug_log, "0x%"UVxf": (%05ld) rfree\n",PTR2UV(where),(long)PL_an++));
    DEBUG_m(PerlIO_printf(Perl_debug_log, "0x%"UVxf": (%05ld) realloc %ld bytes\n",PTR2UV(ptr),(long)PL_an++,(long)size));


    if (ptr != NULL) {
	return ptr;
    }
    else {
#ifndef ALWAYS_NEED_THX
	dTHX;
#endif
	if (PL_nomemok)
	    return NULL;
	else {
	    return write_no_mem();
	}
    }
    /*NOTREACHED*/
}

/* safe version of system's free() */

Free_t
Perl_safesysfree(Malloc_t where)
{
#ifdef ALWAYS_NEED_THX
    dTHX;
#else
    dVAR;
#endif
    DEBUG_m( PerlIO_printf(Perl_debug_log, "0x%"UVxf": (%05ld) free\n",PTR2UV(where),(long)PL_an++));
    if (where) {
#ifdef PERL_TRACK_MEMPOOL
        where = (Malloc_t)((char*)where-sTHX);
	{
	    struct perl_memory_debug_header *const header
		= (struct perl_memory_debug_header *)where;

	    if (header->interpreter != aTHX) {
		Perl_croak_nocontext("panic: free from wrong pool, %p!=%p",
				     header->interpreter, aTHX);
	    }
	    if (!header->prev) {
		Perl_croak_nocontext("panic: duplicate free");
	    }
	    if (!(header->next))
		Perl_croak_nocontext("panic: bad free, header->next==NULL");
	    if (header->next->prev != header || header->prev->next != header) {
		Perl_croak_nocontext("panic: bad free, ->next->prev=%p, "
				     "header=%p, ->prev->next=%p",
				     header->next->prev, header,
				     header->prev->next);
	    }
	    /* Unlink us from the chain.  */
	    header->next->prev = header->prev;
	    header->prev->next = header->next;
#  ifdef PERL_POISON
	    PoisonNew(where, header->size, char);
#  endif
	    /* Trigger the duplicate free warning.  */
	    header->next = NULL;
	}
#endif
	PerlMem_free(where);
    }
}

/* safe version of system's calloc() */

Malloc_t
Perl_safesyscalloc(MEM_SIZE count, MEM_SIZE size)
{
#ifdef ALWAYS_NEED_THX
    dTHX;
#endif
    Malloc_t ptr;
#if defined(PERL_TRACK_MEMPOOL) || defined(HAS_64K_LIMIT) || defined(DEBUGGING)
    MEM_SIZE total_size = 0;
#endif

    /* Even though calloc() for zero bytes is strange, be robust. */
    if (size && (count <= MEM_SIZE_MAX / size)) {
#if defined(PERL_TRACK_MEMPOOL) || defined(HAS_64K_LIMIT) || defined(DEBUGGING)
	total_size = size * count;
#endif
    }
    else
	Perl_croak_nocontext("%s", PL_memory_wrap);
#ifdef PERL_TRACK_MEMPOOL
    if (sTHX <= MEM_SIZE_MAX - (MEM_SIZE)total_size)
	total_size += sTHX;
    else
	Perl_croak_nocontext("%s", PL_memory_wrap);
#endif
#ifdef HAS_64K_LIMIT
    if (total_size > 0xffff) {
	PerlIO_printf(Perl_error_log,
		      "Allocation too large: %lx\n", total_size) FLUSH;
	my_exit(1);
    }
#endif /* HAS_64K_LIMIT */
#ifdef DEBUGGING
    if ((SSize_t)size < 0 || (SSize_t)count < 0)
	Perl_croak_nocontext("panic: calloc, size=%"UVuf", count=%"UVuf,
			     (UV)size, (UV)count);
#endif
#ifdef PERL_TRACK_MEMPOOL
    /* Have to use malloc() because we've added some space for our tracking
       header.  */
    /* malloc(0) is non-portable. */
    ptr = (Malloc_t)PerlMem_malloc(total_size ? total_size : 1);
#else
    /* Use calloc() because it might save a memset() if the memory is fresh
       and clean from the OS.  */
    if (count && size)
	ptr = (Malloc_t)PerlMem_calloc(count, size);
    else /* calloc(0) is non-portable. */
	ptr = (Malloc_t)PerlMem_calloc(count ? count : 1, size ? size : 1);
#endif
    PERL_ALLOC_CHECK(ptr);
    DEBUG_m(PerlIO_printf(Perl_debug_log, "0x%"UVxf": (%05ld) calloc %ld x %ld bytes\n",PTR2UV(ptr),(long)PL_an++,(long)count,(long)total_size));
    if (ptr != NULL) {
#ifdef PERL_TRACK_MEMPOOL
	{
	    struct perl_memory_debug_header *const header
		= (struct perl_memory_debug_header *)ptr;

	    memset((void*)ptr, 0, total_size);
	    header->interpreter = aTHX;
	    /* Link us into the list.  */
	    header->prev = &PL_memory_debug_header;
	    header->next = PL_memory_debug_header.next;
	    PL_memory_debug_header.next = header;
	    header->next->prev = header;
#  ifdef PERL_POISON
	    header->size = total_size;
#  endif
	    ptr = (Malloc_t)((char*)ptr+sTHX);
	}
#endif
	return ptr;
    }
    else {
#ifndef ALWAYS_NEED_THX
	dTHX;
#endif
	if (PL_nomemok)
	    return NULL;
	return write_no_mem();
    }
}

/* These must be defined when not using Perl's malloc for binary
 * compatibility */

#ifndef MYMALLOC

Malloc_t Perl_malloc (MEM_SIZE nbytes)
{
    dTHXs;
    return (Malloc_t)PerlMem_malloc(nbytes);
}

Malloc_t Perl_calloc (MEM_SIZE elements, MEM_SIZE size)
{
    dTHXs;
    return (Malloc_t)PerlMem_calloc(elements, size);
}

Malloc_t Perl_realloc (Malloc_t where, MEM_SIZE nbytes)
{
    dTHXs;
    return (Malloc_t)PerlMem_realloc(where, nbytes);
}

Free_t   Perl_mfree (Malloc_t where)
{
    dTHXs;
    PerlMem_free(where);
}

#endif

/* copy a string up to some (non-backslashed) delimiter, if any */

char *
Perl_delimcpy(register char *to, register const char *toend, register const char *from, register const char *fromend, register int delim, I32 *retlen)
{
    register I32 tolen;

    PERL_ARGS_ASSERT_DELIMCPY;

    for (tolen = 0; from < fromend; from++, tolen++) {
	if (*from == '\\') {
	    if (from[1] != delim) {
		if (to < toend)
		    *to++ = *from;
		tolen++;
	    }
	    from++;
	}
	else if (*from == delim)
	    break;
	if (to < toend)
	    *to++ = *from;
    }
    if (to < toend)
	*to = '\0';
    *retlen = tolen;
    return (char *)from;
}

/* return ptr to little string in big string, NULL if not found */
/* This routine was donated by Corey Satten. */

char *
Perl_instr(register const char *big, register const char *little)
{
    register I32 first;

    PERL_ARGS_ASSERT_INSTR;

    if (!little)
	return (char*)big;
    first = *little++;
    if (!first)
	return (char*)big;
    while (*big) {
	register const char *s, *x;
	if (*big++ != first)
	    continue;
	for (x=big,s=little; *s; /**/ ) {
	    if (!*x)
		return NULL;
	    if (*s != *x)
		break;
	    else {
		s++;
		x++;
	    }
	}
	if (!*s)
	    return (char*)(big-1);
    }
    return NULL;
}

/* same as instr but allow embedded nulls.  The end pointers point to 1 beyond
 * the final character desired to be checked */

char *
Perl_ninstr(const char *big, const char *bigend, const char *little, const char *lend)
{
    PERL_ARGS_ASSERT_NINSTR;
    if (little >= lend)
        return (char*)big;
    {
        const char first = *little;
        const char *s, *x;
        bigend -= lend - little++;
    OUTER:
        while (big <= bigend) {
            if (*big++ == first) {
                for (x=big,s=little; s < lend; x++,s++) {
                    if (*s != *x)
                        goto OUTER;
                }
                return (char*)(big-1);
            }
        }
    }
    return NULL;
}

/* reverse of the above--find last substring */

char *
Perl_rninstr(register const char *big, const char *bigend, const char *little, const char *lend)
{
    register const char *bigbeg;
    register const I32 first = *little;
    register const char * const littleend = lend;

    PERL_ARGS_ASSERT_RNINSTR;

    if (little >= littleend)
	return (char*)bigend;
    bigbeg = big;
    big = bigend - (littleend - little++);
    while (big >= bigbeg) {
	register const char *s, *x;
	if (*big-- != first)
	    continue;
	for (x=big+2,s=little; s < littleend; /**/ ) {
	    if (*s != *x)
		break;
	    else {
		x++;
		s++;
	    }
	}
	if (s >= littleend)
	    return (char*)(big+1);
    }
    return NULL;
}

/* As a space optimization, we do not compile tables for strings of length
   0 and 1, and for strings of length 2 unless FBMcf_TAIL.  These are
   special-cased in fbm_instr().

   If FBMcf_TAIL, the table is created as if the string has a trailing \n. */

/*
=head1 Miscellaneous Functions

=for apidoc fbm_compile

Analyses the string in order to make fast searches on it using fbm_instr()
-- the Boyer-Moore algorithm.

=cut
*/

void
Perl_fbm_compile(pTHX_ SV *sv, U32 flags)
{
    dVAR;
    register const U8 *s;
    STRLEN i;
    STRLEN len;
    STRLEN rarest = 0;
    U32 frequency = 256;
    MAGIC *mg;

    PERL_ARGS_ASSERT_FBM_COMPILE;

    /* Refuse to fbm_compile a studied scalar, as this gives more flexibility in
       SV flag usage.  No real-world code would ever end up using a studied
       scalar as a compile-time second argument to index, so this isn't a real
       pessimisation.  */
    if (SvSCREAM(sv))
	return;

    if (SvVALID(sv))
	return;

    if (flags & FBMcf_TAIL) {
	MAGIC * const mg = SvUTF8(sv) && SvMAGICAL(sv) ? mg_find(sv, PERL_MAGIC_utf8) : NULL;
	sv_catpvs(sv, "\n");		/* Taken into account in fbm_instr() */
	if (mg && mg->mg_len >= 0)
	    mg->mg_len++;
    }
    s = (U8*)SvPV_force_mutable(sv, len);
    if (len == 0)		/* TAIL might be on a zero-length string. */
	return;
    SvUPGRADE(sv, SVt_PVMG);
    SvIOK_off(sv);
    SvNOK_off(sv);
    SvVALID_on(sv);

    /* "deep magic", the comment used to add. The use of MAGIC itself isn't
       really. MAGIC was originally added in 79072805bf63abe5 (perl 5.0 alpha 2)
       to call SvVALID_off() if the scalar was assigned to.

       The comment itself (and "deeper magic" below) date back to
       378cc40b38293ffc (perl 2.0). "deep magic" was an annotation on
       str->str_pok |= 2;
       where the magic (presumably) was that the scalar had a BM table hidden
       inside itself.

       As MAGIC is always present on BMs [in Perl 5 :-)], we can use it to store
       the table instead of the previous (somewhat hacky) approach of co-opting
       the string buffer and storing it after the string.  */

    assert(!mg_find(sv, PERL_MAGIC_bm));
    mg = sv_magicext(sv, NULL, PERL_MAGIC_bm, &PL_vtbl_bm, NULL, 0);
    assert(mg);

    if (len > 2) {
	/* Shorter strings are special-cased in Perl_fbm_instr(), and don't use
	   the BM table.  */
	const U8 mlen = (len>255) ? 255 : (U8)len;
	const unsigned char *const sb = s + len - mlen; /* first char (maybe) */
	register U8 *table;

	Newx(table, 256, U8);
	memset((void*)table, mlen, 256);
	mg->mg_ptr = (char *)table;
	mg->mg_len = 256;

	s += len - 1; /* last char */
	i = 0;
	while (s >= sb) {
	    if (table[*s] == mlen)
		table[*s] = (U8)i;
	    s--, i++;
	}
    }

    s = (const unsigned char*)(SvPVX_const(sv));	/* deeper magic */
    for (i = 0; i < len; i++) {
	if (PL_freq[s[i]] < frequency) {
	    rarest = i;
	    frequency = PL_freq[s[i]];
	}
    }
    BmRARE(sv) = s[rarest];
    BmPREVIOUS(sv) = rarest;
    BmUSEFUL(sv) = 100;			/* Initial value */
    if (flags & FBMcf_TAIL)
	SvTAIL_on(sv);
    DEBUG_r(PerlIO_printf(Perl_debug_log, "rarest char %c at %"UVuf"\n",
			  BmRARE(sv), BmPREVIOUS(sv)));
}

/* If SvTAIL(littlestr), it has a fake '\n' at end. */
/* If SvTAIL is actually due to \Z or \z, this gives false positives
   if multiline */

/*
=for apidoc fbm_instr

Returns the location of the SV in the string delimited by C<str> and
C<strend>.  It returns C<NULL> if the string can't be found.  The C<sv>
does not have to be fbm_compiled, but the search will not be as fast
then.

=cut
*/

char *
Perl_fbm_instr(pTHX_ unsigned char *big, register unsigned char *bigend, SV *littlestr, U32 flags)
{
    register unsigned char *s;
    STRLEN l;
    register const unsigned char *little
	= (const unsigned char *)SvPV_const(littlestr,l);
    register STRLEN littlelen = l;
    register const I32 multiline = flags & FBMrf_MULTILINE;

    PERL_ARGS_ASSERT_FBM_INSTR;

    if ((STRLEN)(bigend - big) < littlelen) {
	if ( SvTAIL(littlestr)
	     && ((STRLEN)(bigend - big) == littlelen - 1)
	     && (littlelen == 1
		 || (*big == *little &&
		     memEQ((char *)big, (char *)little, littlelen - 1))))
	    return (char*)big;
	return NULL;
    }

    switch (littlelen) { /* Special cases for 0, 1 and 2  */
    case 0:
	return (char*)big;		/* Cannot be SvTAIL! */
    case 1:
	    if (SvTAIL(littlestr) && !multiline) { /* Anchor only! */
		/* Know that bigend != big.  */
		if (bigend[-1] == '\n')
		    return (char *)(bigend - 1);
		return (char *) bigend;
	    }
	    s = big;
	    while (s < bigend) {
		if (*s == *little)
		    return (char *)s;
		s++;
	    }
	    if (SvTAIL(littlestr))
		return (char *) bigend;
	    return NULL;
    case 2:
	if (SvTAIL(littlestr) && !multiline) {
	    if (bigend[-1] == '\n' && bigend[-2] == *little)
		return (char*)bigend - 2;
	    if (bigend[-1] == *little)
		return (char*)bigend - 1;
	    return NULL;
	}
	{
	    /* This should be better than FBM if c1 == c2, and almost
	       as good otherwise: maybe better since we do less indirection.
	       And we save a lot of memory by caching no table. */
	    const unsigned char c1 = little[0];
	    const unsigned char c2 = little[1];

	    s = big + 1;
	    bigend--;
	    if (c1 != c2) {
		while (s <= bigend) {
		    if (s[0] == c2) {
			if (s[-1] == c1)
			    return (char*)s - 1;
			s += 2;
			continue;
		    }
		  next_chars:
		    if (s[0] == c1) {
			if (s == bigend)
			    goto check_1char_anchor;
			if (s[1] == c2)
			    return (char*)s;
			else {
			    s++;
			    goto next_chars;
			}
		    }
		    else
			s += 2;
		}
		goto check_1char_anchor;
	    }
	    /* Now c1 == c2 */
	    while (s <= bigend) {
		if (s[0] == c1) {
		    if (s[-1] == c1)
			return (char*)s - 1;
		    if (s == bigend)
			goto check_1char_anchor;
		    if (s[1] == c1)
			return (char*)s;
		    s += 3;
		}
		else
		    s += 2;
	    }
	}
      check_1char_anchor:		/* One char and anchor! */
	if (SvTAIL(littlestr) && (*bigend == *little))
	    return (char *)bigend;	/* bigend is already decremented. */
	return NULL;
    default:
	break; /* Only lengths 0 1 and 2 have special-case code.  */
    }

    if (SvTAIL(littlestr) && !multiline) {	/* tail anchored? */
	s = bigend - littlelen;
	if (s >= big && bigend[-1] == '\n' && *s == *little
	    /* Automatically of length > 2 */
	    && memEQ((char*)s + 1, (char*)little + 1, littlelen - 2))
	{
	    return (char*)s;		/* how sweet it is */
	}
	if (s[1] == *little
	    && memEQ((char*)s + 2, (char*)little + 1, littlelen - 2))
	{
	    return (char*)s + 1;	/* how sweet it is */
	}
	return NULL;
    }
    if (!SvVALID(littlestr)) {
	char * const b = ninstr((char*)big,(char*)bigend,
			 (char*)little, (char*)little + littlelen);

	if (!b && SvTAIL(littlestr)) {	/* Automatically multiline!  */
	    /* Chop \n from littlestr: */
	    s = bigend - littlelen + 1;
	    if (*s == *little
		&& memEQ((char*)s + 1, (char*)little + 1, littlelen - 2))
	    {
		return (char*)s;
	    }
	    return NULL;
	}
	return b;
    }

    /* Do actual FBM.  */
    if (littlelen > (STRLEN)(bigend - big))
	return NULL;

    {
	const MAGIC *const mg = mg_find(littlestr, PERL_MAGIC_bm);
	const unsigned char * const table = (const unsigned char *) mg->mg_ptr;
	register const unsigned char *oldlittle;

	--littlelen;			/* Last char found by table lookup */

	s = big + littlelen;
	little += littlelen;		/* last char */
	oldlittle = little;
	if (s < bigend) {
	    register I32 tmp;

	  top2:
	    if ((tmp = table[*s])) {
		if ((s += tmp) < bigend)
		    goto top2;
		goto check_end;
	    }
	    else {		/* less expensive than calling strncmp() */
		register unsigned char * const olds = s;

		tmp = littlelen;

		while (tmp--) {
		    if (*--s == *--little)
			continue;
		    s = olds + 1;	/* here we pay the price for failure */
		    little = oldlittle;
		    if (s < bigend)	/* fake up continue to outer loop */
			goto top2;
		    goto check_end;
		}
		return (char *)s;
	    }
	}
      check_end:
	if ( s == bigend
	     && SvTAIL(littlestr)
	     && memEQ((char *)(bigend - littlelen),
		      (char *)(oldlittle - littlelen), littlelen) )
	    return (char*)bigend - littlelen;
	return NULL;
    }
}

/* start_shift, end_shift are positive quantities which give offsets
   of ends of some substring of bigstr.
   If "last" we want the last occurrence.
   old_posp is the way of communication between consequent calls if
   the next call needs to find the .
   The initial *old_posp should be -1.

   Note that we take into account SvTAIL, so one can get extra
   optimizations if _ALL flag is set.
 */

/* If SvTAIL is actually due to \Z or \z, this gives false positives
   if PL_multiline.  In fact if !PL_multiline the authoritative answer
   is not supported yet. */

char *
Perl_screaminstr(pTHX_ SV *bigstr, SV *littlestr, I32 start_shift, I32 end_shift, I32 *old_posp, I32 last)
{
    dVAR;
    register const unsigned char *big;
    U32 pos = 0; /* hush a gcc warning */
    register I32 previous;
    register I32 first;
    register const unsigned char *little;
    register I32 stop_pos;
    register const unsigned char *littleend;
    bool found = FALSE;
    const MAGIC * mg;
    const void *screamnext_raw = NULL; /* hush a gcc warning */
    bool cant_find = FALSE; /* hush a gcc warning */

    PERL_ARGS_ASSERT_SCREAMINSTR;

    assert(SvMAGICAL(bigstr));
    mg = mg_find(bigstr, PERL_MAGIC_study);
    assert(mg);
    assert(SvTYPE(littlestr) == SVt_PVMG);
    assert(SvVALID(littlestr));

    if (mg->mg_private == 1) {
	const U8 *const screamfirst = (U8 *)mg->mg_ptr;
	const U8 *const screamnext = screamfirst + 256;

	screamnext_raw = (const void *)screamnext;

	pos = *old_posp == -1
	    ? screamfirst[BmRARE(littlestr)] : screamnext[*old_posp];
	cant_find = pos == (U8)~0;
    } else if (mg->mg_private == 2) {
	const U16 *const screamfirst = (U16 *)mg->mg_ptr;
	const U16 *const screamnext = screamfirst + 256;

	screamnext_raw = (const void *)screamnext;

	pos = *old_posp == -1
	    ? screamfirst[BmRARE(littlestr)] : screamnext[*old_posp];
	cant_find = pos == (U16)~0;
    } else if (mg->mg_private == 4) {
	const U32 *const screamfirst = (U32 *)mg->mg_ptr;
	const U32 *const screamnext = screamfirst + 256;

	screamnext_raw = (const void *)screamnext;

	pos = *old_posp == -1
	    ? screamfirst[BmRARE(littlestr)] : screamnext[*old_posp];
	cant_find = pos == (U32)~0;
    } else
	Perl_croak(aTHX_ "panic: unknown study size %u", mg->mg_private);

    if (cant_find) {
      cant_find:
	if ( BmRARE(littlestr) == '\n'
	     && BmPREVIOUS(littlestr) == SvCUR(littlestr) - 1) {
	    little = (const unsigned char *)(SvPVX_const(littlestr));
	    littleend = little + SvCUR(littlestr);
	    first = *little++;
	    goto check_tail;
	}
	return NULL;
    }

    little = (const unsigned char *)(SvPVX_const(littlestr));
    littleend = little + SvCUR(littlestr);
    first = *little++;
    /* The value of pos we can start at: */
    previous = BmPREVIOUS(littlestr);
    big = (const unsigned char *)(SvPVX_const(bigstr));
    /* The value of pos we can stop at: */
    stop_pos = SvCUR(bigstr) - end_shift - (SvCUR(littlestr) - 1 - previous);
    if (previous + start_shift > stop_pos) {
/*
  stop_pos does not include SvTAIL in the count, so this check is incorrect
  (I think) - see [ID 20010618.006] and t/op/study.t. HVDS 2001/06/19
*/
#if 0
	if (previous + start_shift == stop_pos + 1) /* A fake '\n'? */
	    goto check_tail;
#endif
	return NULL;
    }
    if (mg->mg_private == 1) {
	const U8 *const screamnext = (const U8 *const) screamnext_raw;
	while ((I32)pos < previous + start_shift) {
	    pos = screamnext[pos];
	    if (pos == (U8)~0)
		goto cant_find;
	}
    } else if (mg->mg_private == 2) {
	const U16 *const screamnext = (const U16 *const) screamnext_raw;
	while ((I32)pos < previous + start_shift) {
	    pos = screamnext[pos];
	    if (pos == (U16)~0)
		goto cant_find;
	}
    } else if (mg->mg_private == 4) {
	const U32 *const screamnext = (const U32 *const) screamnext_raw;
	while ((I32)pos < previous + start_shift) {
	    pos = screamnext[pos];
	    if (pos == (U32)~0)
		goto cant_find;
	}
    }
    big -= previous;
    while (1) {
	if ((I32)pos >= stop_pos) break;
	if (big[pos] == first) {
	    const unsigned char *s = little;
	    const unsigned char *x = big + pos + 1;
	    while (s < littleend) {
		if (*s != *x++)
		    break;
		++s;
	    }
	    if (s == littleend) {
		*old_posp = (I32)pos;
		if (!last) return (char *)(big+pos);
		found = TRUE;
	    }
	}
	if (mg->mg_private == 1) {
	    pos = ((const U8 *const)screamnext_raw)[pos];
	    if (pos == (U8)~0)
		break;
	} else if (mg->mg_private == 2) {
	    pos = ((const U16 *const)screamnext_raw)[pos];
	    if (pos == (U16)~0)
		break;
	} else if (mg->mg_private == 4) {
	    pos = ((const U32 *const)screamnext_raw)[pos];
	    if (pos == (U32)~0)
		break;
	}
    };
    if (last && found)
	return (char *)(big+(*old_posp));
  check_tail:
    if (!SvTAIL(littlestr) || (end_shift > 0))
	return NULL;
    /* Ignore the trailing "\n".  This code is not microoptimized */
    big = (const unsigned char *)(SvPVX_const(bigstr) + SvCUR(bigstr));
    stop_pos = littleend - little;	/* Actual littlestr len */
    if (stop_pos == 0)
	return (char*)big;
    big -= stop_pos;
    if (*big == first
	&& ((stop_pos == 1) ||
	    memEQ((char *)(big + 1), (char *)little, stop_pos - 1)))
	return (char*)big;
    return NULL;
}

/*
=for apidoc foldEQ

Returns true if the leading len bytes of the strings s1 and s2 are the same
case-insensitively; false otherwise.  Uppercase and lowercase ASCII range bytes
match themselves and their opposite case counterparts.  Non-cased and non-ASCII
range bytes match only themselves.

=cut
*/


I32
Perl_foldEQ(const char *s1, const char *s2, register I32 len)
{
    register const U8 *a = (const U8 *)s1;
    register const U8 *b = (const U8 *)s2;

    PERL_ARGS_ASSERT_FOLDEQ;

    while (len--) {
	if (*a != *b && *a != PL_fold[*b])
	    return 0;
	a++,b++;
    }
    return 1;
}
I32
Perl_foldEQ_latin1(const char *s1, const char *s2, register I32 len)
{
    /* Compare non-utf8 using Unicode (Latin1) semantics.  Does not work on
     * MICRO_SIGN, LATIN_SMALL_LETTER_SHARP_S, nor
     * LATIN_SMALL_LETTER_Y_WITH_DIAERESIS, and does not check for these.  Nor
     * does it check that the strings each have at least 'len' characters */

    register const U8 *a = (const U8 *)s1;
    register const U8 *b = (const U8 *)s2;

    PERL_ARGS_ASSERT_FOLDEQ_LATIN1;

    while (len--) {
	if (*a != *b && *a != PL_fold_latin1[*b]) {
	    return 0;
	}
	a++, b++;
    }
    return 1;
}

/*
=for apidoc foldEQ_locale

Returns true if the leading len bytes of the strings s1 and s2 are the same
case-insensitively in the current locale; false otherwise.

=cut
*/

I32
Perl_foldEQ_locale(const char *s1, const char *s2, register I32 len)
{
    dVAR;
    register const U8 *a = (const U8 *)s1;
    register const U8 *b = (const U8 *)s2;

    PERL_ARGS_ASSERT_FOLDEQ_LOCALE;

    while (len--) {
	if (*a != *b && *a != PL_fold_locale[*b])
	    return 0;
	a++,b++;
    }
    return 1;
}

/* copy a string to a safe spot */

/*
=head1 Memory Management

=for apidoc savepv

Perl's version of C<strdup()>. Returns a pointer to a newly allocated
string which is a duplicate of C<pv>. The size of the string is
determined by C<strlen()>. The memory allocated for the new string can
be freed with the C<Safefree()> function.

=cut
*/

char *
Perl_savepv(pTHX_ const char *pv)
{
    PERL_UNUSED_CONTEXT;
    if (!pv)
	return NULL;
    else {
	char *newaddr;
	const STRLEN pvlen = strlen(pv)+1;
	Newx(newaddr, pvlen, char);
	return (char*)memcpy(newaddr, pv, pvlen);
    }
}

/* same thing but with a known length */

/*
=for apidoc savepvn

Perl's version of what C<strndup()> would be if it existed. Returns a
pointer to a newly allocated string which is a duplicate of the first
C<len> bytes from C<pv>, plus a trailing NUL byte. The memory allocated for
the new string can be freed with the C<Safefree()> function.

=cut
*/

char *
Perl_savepvn(pTHX_ const char *pv, register I32 len)
{
    register char *newaddr;
    PERL_UNUSED_CONTEXT;

    Newx(newaddr,len+1,char);
    /* Give a meaning to NULL pointer mainly for the use in sv_magic() */
    if (pv) {
	/* might not be null terminated */
    	newaddr[len] = '\0';
    	return (char *) CopyD(pv,newaddr,len,char);
    }
    else {
	return (char *) ZeroD(newaddr,len+1,char);
    }
}

/*
=for apidoc savesharedpv

A version of C<savepv()> which allocates the duplicate string in memory
which is shared between threads.

=cut
*/
char *
Perl_savesharedpv(pTHX_ const char *pv)
{
    register char *newaddr;
    STRLEN pvlen;
    if (!pv)
	return NULL;

    pvlen = strlen(pv)+1;
    newaddr = (char*)PerlMemShared_malloc(pvlen);
    if (!newaddr) {
	return write_no_mem();
    }
    return (char*)memcpy(newaddr, pv, pvlen);
}

/*
=for apidoc savesharedpvn

A version of C<savepvn()> which allocates the duplicate string in memory
which is shared between threads. (With the specific difference that a NULL
pointer is not acceptable)

=cut
*/
char *
Perl_savesharedpvn(pTHX_ const char *const pv, const STRLEN len)
{
    char *const newaddr = (char*)PerlMemShared_malloc(len + 1);

    /* PERL_ARGS_ASSERT_SAVESHAREDPVN; */

    if (!newaddr) {
	return write_no_mem();
    }
    newaddr[len] = '\0';
    return (char*)memcpy(newaddr, pv, len);
}

/*
=for apidoc savesvpv

A version of C<savepv()>/C<savepvn()> which gets the string to duplicate from
the passed in SV using C<SvPV()>

=cut
*/

char *
Perl_savesvpv(pTHX_ SV *sv)
{
    STRLEN len;
    const char * const pv = SvPV_const(sv, len);
    register char *newaddr;

    PERL_ARGS_ASSERT_SAVESVPV;

    ++len;
    Newx(newaddr,len,char);
    return (char *) CopyD(pv,newaddr,len,char);
}

/*
=for apidoc savesharedsvpv

A version of C<savesharedpv()> which allocates the duplicate string in
memory which is shared between threads.

=cut
*/

char *
Perl_savesharedsvpv(pTHX_ SV *sv)
{
    STRLEN len;
    const char * const pv = SvPV_const(sv, len);

    PERL_ARGS_ASSERT_SAVESHAREDSVPV;

    return savesharedpvn(pv, len);
}

/* the SV for Perl_form() and mess() is not kept in an arena */

STATIC SV *
S_mess_alloc(pTHX)
{
    dVAR;
    SV *sv;
    XPVMG *any;

    if (PL_phase != PERL_PHASE_DESTRUCT)
	return newSVpvs_flags("", SVs_TEMP);

    if (PL_mess_sv)
	return PL_mess_sv;

    /* Create as PVMG now, to avoid any upgrading later */
    Newx(sv, 1, SV);
    Newxz(any, 1, XPVMG);
    SvFLAGS(sv) = SVt_PVMG;
    SvANY(sv) = (void*)any;
    SvPV_set(sv, NULL);
    SvREFCNT(sv) = 1 << 30; /* practically infinite */
    PL_mess_sv = sv;
    return sv;
}

#if defined(PERL_IMPLICIT_CONTEXT)
char *
Perl_form_nocontext(const char* pat, ...)
{
    dTHX;
    char *retval;
    va_list args;
    PERL_ARGS_ASSERT_FORM_NOCONTEXT;
    va_start(args, pat);
    retval = vform(pat, &args);
    va_end(args);
    return retval;
}
#endif /* PERL_IMPLICIT_CONTEXT */

/*
=head1 Miscellaneous Functions
=for apidoc form

Takes a sprintf-style format pattern and conventional
(non-SV) arguments and returns the formatted string.

    (char *) Perl_form(pTHX_ const char* pat, ...)

can be used any place a string (char *) is required:

    char * s = Perl_form("%d.%d",major,minor);

Uses a single private buffer so if you want to format several strings you
must explicitly copy the earlier strings away (and free the copies when you
are done).

=cut
*/

char *
Perl_form(pTHX_ const char* pat, ...)
{
    char *retval;
    va_list args;
    PERL_ARGS_ASSERT_FORM;
    va_start(args, pat);
    retval = vform(pat, &args);
    va_end(args);
    return retval;
}

char *
Perl_vform(pTHX_ const char *pat, va_list *args)
{
    SV * const sv = mess_alloc();
    PERL_ARGS_ASSERT_VFORM;
    sv_vsetpvfn(sv, pat, strlen(pat), args, NULL, 0, NULL);
    return SvPVX(sv);
}

/*
=for apidoc Am|SV *|mess|const char *pat|...

Take a sprintf-style format pattern and argument list.  These are used to
generate a string message.  If the message does not end with a newline,
then it will be extended with some indication of the current location
in the code, as described for L</mess_sv>.

Normally, the resulting message is returned in a new mortal SV.
During global destruction a single SV may be shared between uses of
this function.

=cut
*/

#if defined(PERL_IMPLICIT_CONTEXT)
SV *
Perl_mess_nocontext(const char *pat, ...)
{
    dTHX;
    SV *retval;
    va_list args;
    PERL_ARGS_ASSERT_MESS_NOCONTEXT;
    va_start(args, pat);
    retval = vmess(pat, &args);
    va_end(args);
    return retval;
}
#endif /* PERL_IMPLICIT_CONTEXT */

SV *
Perl_mess(pTHX_ const char *pat, ...)
{
    SV *retval;
    va_list args;
    PERL_ARGS_ASSERT_MESS;
    va_start(args, pat);
    retval = vmess(pat, &args);
    va_end(args);
    return retval;
}

STATIC const COP*
S_closest_cop(pTHX_ const COP *cop, const OP *o)
{
    dVAR;
    /* Look for PL_op starting from o.  cop is the last COP we've seen. */

    PERL_ARGS_ASSERT_CLOSEST_COP;

    if (!o || o == PL_op)
	return cop;

    if (o->op_flags & OPf_KIDS) {
	const OP *kid;
	for (kid = cUNOPo->op_first; kid; kid = kid->op_sibling) {
	    const COP *new_cop;

	    /* If the OP_NEXTSTATE has been optimised away we can still use it
	     * the get the file and line number. */

	    if (kid->op_type == OP_NULL && kid->op_targ == OP_NEXTSTATE)
		cop = (const COP *)kid;

	    /* Keep searching, and return when we've found something. */

	    new_cop = closest_cop(cop, kid);
	    if (new_cop)
		return new_cop;
	}
    }

    /* Nothing found. */

    return NULL;
}

/*
=for apidoc Am|SV *|mess_sv|SV *basemsg|bool consume

Expands a message, intended for the user, to include an indication of
the current location in the code, if the message does not already appear
to be complete.

C<basemsg> is the initial message or object.  If it is a reference, it
will be used as-is and will be the result of this function.  Otherwise it
is used as a string, and if it already ends with a newline, it is taken
to be complete, and the result of this function will be the same string.
If the message does not end with a newline, then a segment such as C<at
foo.pl line 37> will be appended, and possibly other clauses indicating
the current state of execution.  The resulting message will end with a
dot and a newline.

Normally, the resulting message is returned in a new mortal SV.
During global destruction a single SV may be shared between uses of this
function.  If C<consume> is true, then the function is permitted (but not
required) to modify and return C<basemsg> instead of allocating a new SV.

=cut
*/

SV *
Perl_mess_sv(pTHX_ SV *basemsg, bool consume)
{
    dVAR;
    SV *sv;

    PERL_ARGS_ASSERT_MESS_SV;

    if (SvROK(basemsg)) {
	if (consume) {
	    sv = basemsg;
	}
	else {
	    sv = mess_alloc();
	    sv_setsv(sv, basemsg);
	}
	return sv;
    }

    if (SvPOK(basemsg) && consume) {
	sv = basemsg;
    }
    else {
	sv = mess_alloc();
	sv_copypv(sv, basemsg);
    }

    if (!SvCUR(sv) || *(SvEND(sv) - 1) != '\n') {
	/*
	 * Try and find the file and line for PL_op.  This will usually be
	 * PL_curcop, but it might be a cop that has been optimised away.  We
	 * can try to find such a cop by searching through the optree starting
	 * from the sibling of PL_curcop.
	 */

	const COP *cop = closest_cop(PL_curcop, PL_curcop->op_sibling);
	if (!cop)
	    cop = PL_curcop;

	if (CopLINE(cop))
	    Perl_sv_catpvf(aTHX_ sv, " at %s line %"IVdf,
	    OutCopFILE(cop), (IV)CopLINE(cop));
	/* Seems that GvIO() can be untrustworthy during global destruction. */
	if (GvIO(PL_last_in_gv) && (SvTYPE(GvIOp(PL_last_in_gv)) == SVt_PVIO)
		&& IoLINES(GvIOp(PL_last_in_gv)))
	{
	    const bool line_mode = (RsSIMPLE(PL_rs) &&
			      SvCUR(PL_rs) == 1 && *SvPVX_const(PL_rs) == '\n');
	    Perl_sv_catpvf(aTHX_ sv, ", <%"SVf"> %s %"IVdf,
			   SVfARG(PL_last_in_gv == PL_argvgv
                                 ? &PL_sv_no
                                 : sv_2mortal(newSVhek(GvNAME_HEK(PL_last_in_gv)))),
			   line_mode ? "line" : "chunk",
			   (IV)IoLINES(GvIOp(PL_last_in_gv)));
	}
	if (PL_phase == PERL_PHASE_DESTRUCT)
	    sv_catpvs(sv, " during global destruction");
	sv_catpvs(sv, ".\n");
    }
    return sv;
}

/*
=for apidoc Am|SV *|vmess|const char *pat|va_list *args

C<pat> and C<args> are a sprintf-style format pattern and encapsulated
argument list.  These are used to generate a string message.  If the
message does not end with a newline, then it will be extended with
some indication of the current location in the code, as described for
L</mess_sv>.

Normally, the resulting message is returned in a new mortal SV.
During global destruction a single SV may be shared between uses of
this function.

=cut
*/

SV *
Perl_vmess(pTHX_ const char *pat, va_list *args)
{
    dVAR;
    SV * const sv = mess_alloc();

    PERL_ARGS_ASSERT_VMESS;

    sv_vsetpvfn(sv, pat, strlen(pat), args, NULL, 0, NULL);
    return mess_sv(sv, 1);
}

void
Perl_write_to_stderr(pTHX_ SV* msv)
{
    dVAR;
    IO *io;
    MAGIC *mg;

    PERL_ARGS_ASSERT_WRITE_TO_STDERR;

    if (PL_stderrgv && SvREFCNT(PL_stderrgv) 
	&& (io = GvIO(PL_stderrgv))
	&& (mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar))) 
	Perl_magic_methcall(aTHX_ MUTABLE_SV(io), mg, "PRINT",
			    G_SCALAR | G_DISCARD | G_WRITING_TO_STDERR, 1, msv);
    else {
#ifdef USE_SFIO
	/* SFIO can really mess with your errno */
	dSAVED_ERRNO;
#endif
	PerlIO * const serr = Perl_error_log;

	do_print(msv, serr);
	(void)PerlIO_flush(serr);
#ifdef USE_SFIO
	RESTORE_ERRNO;
#endif
    }
}

/*
=head1 Warning and Dieing
*/

/* Common code used in dieing and warning */

STATIC SV *
S_with_queued_errors(pTHX_ SV *ex)
{
    PERL_ARGS_ASSERT_WITH_QUEUED_ERRORS;
    if (PL_errors && SvCUR(PL_errors) && !SvROK(ex)) {
	sv_catsv(PL_errors, ex);
	ex = sv_mortalcopy(PL_errors);
	SvCUR_set(PL_errors, 0);
    }
    return ex;
}

STATIC bool
S_invoke_exception_hook(pTHX_ SV *ex, bool warn)
{
    dVAR;
    HV *stash;
    GV *gv;
    CV *cv;
    SV **const hook = warn ? &PL_warnhook : &PL_diehook;
    /* sv_2cv might call Perl_croak() or Perl_warner() */
    SV * const oldhook = *hook;

    if (!oldhook)
	return FALSE;

    ENTER;
    SAVESPTR(*hook);
    *hook = NULL;
    cv = sv_2cv(oldhook, &stash, &gv, 0);
    LEAVE;
    if (cv && !CvDEPTH(cv) && (CvROOT(cv) || CvXSUB(cv))) {
	dSP;
	SV *exarg;

	ENTER;
	save_re_context();
	if (warn) {
	    SAVESPTR(*hook);
	    *hook = NULL;
	}
	exarg = newSVsv(ex);
	SvREADONLY_on(exarg);
	SAVEFREESV(exarg);

	PUSHSTACKi(warn ? PERLSI_WARNHOOK : PERLSI_DIEHOOK);
	PUSHMARK(SP);
	XPUSHs(exarg);
	PUTBACK;
	call_sv(MUTABLE_SV(cv), G_DISCARD);
	POPSTACK;
	LEAVE;
	return TRUE;
    }
    return FALSE;
}

/*
=for apidoc Am|OP *|die_sv|SV *baseex

Behaves the same as L</croak_sv>, except for the return type.
It should be used only where the C<OP *> return type is required.
The function never actually returns.

=cut
*/

OP *
Perl_die_sv(pTHX_ SV *baseex)
{
    PERL_ARGS_ASSERT_DIE_SV;
    croak_sv(baseex);
    /* NOTREACHED */
    return NULL;
}

/*
=for apidoc Am|OP *|die|const char *pat|...

Behaves the same as L</croak>, except for the return type.
It should be used only where the C<OP *> return type is required.
The function never actually returns.

=cut
*/

#if defined(PERL_IMPLICIT_CONTEXT)
OP *
Perl_die_nocontext(const char* pat, ...)
{
    dTHX;
    va_list args;
    va_start(args, pat);
    vcroak(pat, &args);
    /* NOTREACHED */
    va_end(args);
    return NULL;
}
#endif /* PERL_IMPLICIT_CONTEXT */

OP *
Perl_die(pTHX_ const char* pat, ...)
{
    va_list args;
    va_start(args, pat);
    vcroak(pat, &args);
    /* NOTREACHED */
    va_end(args);
    return NULL;
}

/*
=for apidoc Am|void|croak_sv|SV *baseex

This is an XS interface to Perl's C<die> function.

C<baseex> is the error message or object.  If it is a reference, it
will be used as-is.  Otherwise it is used as a string, and if it does
not end with a newline then it will be extended with some indication of
the current location in the code, as described for L</mess_sv>.

The error message or object will be used as an exception, by default
returning control to the nearest enclosing C<eval>, but subject to
modification by a C<$SIG{__DIE__}> handler.  In any case, the C<croak_sv>
function never returns normally.

To die with a simple string message, the L</croak> function may be
more convenient.

=cut
*/

void
Perl_croak_sv(pTHX_ SV *baseex)
{
    SV *ex = with_queued_errors(mess_sv(baseex, 0));
    PERL_ARGS_ASSERT_CROAK_SV;
    invoke_exception_hook(ex, FALSE);
    die_unwind(ex);
}

/*
=for apidoc Am|void|vcroak|const char *pat|va_list *args

This is an XS interface to Perl's C<die> function.

C<pat> and C<args> are a sprintf-style format pattern and encapsulated
argument list.  These are used to generate a string message.  If the
message does not end with a newline, then it will be extended with
some indication of the current location in the code, as described for
L</mess_sv>.

The error message will be used as an exception, by default
returning control to the nearest enclosing C<eval>, but subject to
modification by a C<$SIG{__DIE__}> handler.  In any case, the C<croak>
function never returns normally.

For historical reasons, if C<pat> is null then the contents of C<ERRSV>
(C<$@>) will be used as an error message or object instead of building an
error message from arguments.  If you want to throw a non-string object,
or build an error message in an SV yourself, it is preferable to use
the L</croak_sv> function, which does not involve clobbering C<ERRSV>.

=cut
*/

void
Perl_vcroak(pTHX_ const char* pat, va_list *args)
{
    SV *ex = with_queued_errors(pat ? vmess(pat, args) : mess_sv(ERRSV, 0));
    invoke_exception_hook(ex, FALSE);
    die_unwind(ex);
}

/*
=for apidoc Am|void|croak|const char *pat|...

This is an XS interface to Perl's C<die> function.

Take a sprintf-style format pattern and argument list.  These are used to
generate a string message.  If the message does not end with a newline,
then it will be extended with some indication of the current location
in the code, as described for L</mess_sv>.

The error message will be used as an exception, by default
returning control to the nearest enclosing C<eval>, but subject to
modification by a C<$SIG{__DIE__}> handler.  In any case, the C<croak>
function never returns normally.

For historical reasons, if C<pat> is null then the contents of C<ERRSV>
(C<$@>) will be used as an error message or object instead of building an
error message from arguments.  If you want to throw a non-string object,
or build an error message in an SV yourself, it is preferable to use
the L</croak_sv> function, which does not involve clobbering C<ERRSV>.

=cut
*/

#if defined(PERL_IMPLICIT_CONTEXT)
void
Perl_croak_nocontext(const char *pat, ...)
{
    dTHX;
    va_list args;
    va_start(args, pat);
    vcroak(pat, &args);
    /* NOTREACHED */
    va_end(args);
}
#endif /* PERL_IMPLICIT_CONTEXT */

void
Perl_croak(pTHX_ const char *pat, ...)
{
    va_list args;
    va_start(args, pat);
    vcroak(pat, &args);
    /* NOTREACHED */
    va_end(args);
}

/*
=for apidoc Am|void|croak_no_modify

Exactly equivalent to C<Perl_croak(aTHX_ "%s", PL_no_modify)>, but generates
terser object code than using C<Perl_croak>. Less code used on exception code
paths reduces CPU cache pressure.

=cut
*/

void
Perl_croak_no_modify(pTHX)
{
    Perl_croak(aTHX_ "%s", PL_no_modify);
}

/*
=for apidoc Am|void|warn_sv|SV *baseex

This is an XS interface to Perl's C<warn> function.

C<baseex> is the error message or object.  If it is a reference, it
will be used as-is.  Otherwise it is used as a string, and if it does
not end with a newline then it will be extended with some indication of
the current location in the code, as described for L</mess_sv>.

The error message or object will by default be written to standard error,
but this is subject to modification by a C<$SIG{__WARN__}> handler.

To warn with a simple string message, the L</warn> function may be
more convenient.

=cut
*/

void
Perl_warn_sv(pTHX_ SV *baseex)
{
    SV *ex = mess_sv(baseex, 0);
    PERL_ARGS_ASSERT_WARN_SV;
    if (!invoke_exception_hook(ex, TRUE))
	write_to_stderr(ex);
}

/*
=for apidoc Am|void|vwarn|const char *pat|va_list *args

This is an XS interface to Perl's C<warn> function.

C<pat> and C<args> are a sprintf-style format pattern and encapsulated
argument list.  These are used to generate a string message.  If the
message does not end with a newline, then it will be extended with
some indication of the current location in the code, as described for
L</mess_sv>.

The error message or object will by default be written to standard error,
but this is subject to modification by a C<$SIG{__WARN__}> handler.

Unlike with L</vcroak>, C<pat> is not permitted to be null.

=cut
*/

void
Perl_vwarn(pTHX_ const char* pat, va_list *args)
{
    SV *ex = vmess(pat, args);
    PERL_ARGS_ASSERT_VWARN;
    if (!invoke_exception_hook(ex, TRUE))
	write_to_stderr(ex);
}

/*
=for apidoc Am|void|warn|const char *pat|...

This is an XS interface to Perl's C<warn> function.

Take a sprintf-style format pattern and argument list.  These are used to
generate a string message.  If the message does not end with a newline,
then it will be extended with some indication of the current location
in the code, as described for L</mess_sv>.

The error message or object will by default be written to standard error,
but this is subject to modification by a C<$SIG{__WARN__}> handler.

Unlike with L</croak>, C<pat> is not permitted to be null.

=cut
*/

#if defined(PERL_IMPLICIT_CONTEXT)
void
Perl_warn_nocontext(const char *pat, ...)
{
    dTHX;
    va_list args;
    PERL_ARGS_ASSERT_WARN_NOCONTEXT;
    va_start(args, pat);
    vwarn(pat, &args);
    va_end(args);
}
#endif /* PERL_IMPLICIT_CONTEXT */

void
Perl_warn(pTHX_ const char *pat, ...)
{
    va_list args;
    PERL_ARGS_ASSERT_WARN;
    va_start(args, pat);
    vwarn(pat, &args);
    va_end(args);
}

#if defined(PERL_IMPLICIT_CONTEXT)
void
Perl_warner_nocontext(U32 err, const char *pat, ...)
{
    dTHX; 
    va_list args;
    PERL_ARGS_ASSERT_WARNER_NOCONTEXT;
    va_start(args, pat);
    vwarner(err, pat, &args);
    va_end(args);
}
#endif /* PERL_IMPLICIT_CONTEXT */

void
Perl_ck_warner_d(pTHX_ U32 err, const char* pat, ...)
{
    PERL_ARGS_ASSERT_CK_WARNER_D;

    if (Perl_ckwarn_d(aTHX_ err)) {
	va_list args;
	va_start(args, pat);
	vwarner(err, pat, &args);
	va_end(args);
    }
}

void
Perl_ck_warner(pTHX_ U32 err, const char* pat, ...)
{
    PERL_ARGS_ASSERT_CK_WARNER;

    if (Perl_ckwarn(aTHX_ err)) {
	va_list args;
	va_start(args, pat);
	vwarner(err, pat, &args);
	va_end(args);
    }
}

void
Perl_warner(pTHX_ U32  err, const char* pat,...)
{
    va_list args;
    PERL_ARGS_ASSERT_WARNER;
    va_start(args, pat);
    vwarner(err, pat, &args);
    va_end(args);
}

void
Perl_vwarner(pTHX_ U32  err, const char* pat, va_list* args)
{
    dVAR;
    PERL_ARGS_ASSERT_VWARNER;
    if (PL_warnhook == PERL_WARNHOOK_FATAL || ckDEAD(err)) {
	SV * const msv = vmess(pat, args);

	invoke_exception_hook(msv, FALSE);
	die_unwind(msv);
    }
    else {
	Perl_vwarn(aTHX_ pat, args);
    }
}

/* implements the ckWARN? macros */

bool
Perl_ckwarn(pTHX_ U32 w)
{
    dVAR;
    /* If lexical warnings have not been set, use $^W.  */
    if (isLEXWARN_off)
	return PL_dowarn & G_WARN_ON;

    return ckwarn_common(w);
}

/* implements the ckWARN?_d macro */

bool
Perl_ckwarn_d(pTHX_ U32 w)
{
    dVAR;
    /* If lexical warnings have not been set then default classes warn.  */
    if (isLEXWARN_off)
	return TRUE;

    return ckwarn_common(w);
}

static bool
S_ckwarn_common(pTHX_ U32 w)
{
    if (PL_curcop->cop_warnings == pWARN_ALL)
	return TRUE;

    if (PL_curcop->cop_warnings == pWARN_NONE)
	return FALSE;

    /* Check the assumption that at least the first slot is non-zero.  */
    assert(unpackWARN1(w));

    /* Check the assumption that it is valid to stop as soon as a zero slot is
       seen.  */
    if (!unpackWARN2(w)) {
	assert(!unpackWARN3(w));
	assert(!unpackWARN4(w));
    } else if (!unpackWARN3(w)) {
	assert(!unpackWARN4(w));
    }
	
    /* Right, dealt with all the special cases, which are implemented as non-
       pointers, so there is a pointer to a real warnings mask.  */
    do {
	if (isWARN_on(PL_curcop->cop_warnings, unpackWARN1(w)))
	    return TRUE;
    } while (w >>= WARNshift);

    return FALSE;
}

/* Set buffer=NULL to get a new one.  */
STRLEN *
Perl_new_warnings_bitfield(pTHX_ STRLEN *buffer, const char *const bits,
			   STRLEN size) {
    const MEM_SIZE len_wanted =
	sizeof(STRLEN) + (size > WARNsize ? size : WARNsize);
    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_NEW_WARNINGS_BITFIELD;

    buffer = (STRLEN*)
	(specialWARN(buffer) ?
	 PerlMemShared_malloc(len_wanted) :
	 PerlMemShared_realloc(buffer, len_wanted));
    buffer[0] = size;
    Copy(bits, (buffer + 1), size, char);
    if (size < WARNsize)
	Zero((char *)(buffer + 1) + size, WARNsize - size, char);
    return buffer;
}

/* since we've already done strlen() for both nam and val
 * we can use that info to make things faster than
 * sprintf(s, "%s=%s", nam, val)
 */
#define my_setenv_format(s, nam, nlen, val, vlen) \
   Copy(nam, s, nlen, char); \
   *(s+nlen) = '='; \
   Copy(val, s+(nlen+1), vlen, char); \
   *(s+(nlen+1+vlen)) = '\0'

#ifdef USE_ENVIRON_ARRAY
       /* VMS' my_setenv() is in vms.c */
#if !defined(WIN32) && !defined(NETWARE)
void
Perl_my_setenv(pTHX_ const char *nam, const char *val)
{
  dVAR;
#ifdef USE_ITHREADS
  /* only parent thread can modify process environment */
  if (PL_curinterp == aTHX)
#endif
  {
#ifndef PERL_USE_SAFE_PUTENV
    if (!PL_use_safe_putenv) {
    /* most putenv()s leak, so we manipulate environ directly */
    register I32 i;
    register const I32 len = strlen(nam);
    int nlen, vlen;

    /* where does it go? */
    for (i = 0; environ[i]; i++) {
        if (strnEQ(environ[i],nam,len) && environ[i][len] == '=')
            break;
    }

    if (environ == PL_origenviron) {   /* need we copy environment? */
       I32 j;
       I32 max;
       char **tmpenv;

       max = i;
       while (environ[max])
           max++;
       tmpenv = (char**)safesysmalloc((max+2) * sizeof(char*));
       for (j=0; j<max; j++) {         /* copy environment */
           const int len = strlen(environ[j]);
           tmpenv[j] = (char*)safesysmalloc((len+1)*sizeof(char));
           Copy(environ[j], tmpenv[j], len+1, char);
       }
       tmpenv[max] = NULL;
       environ = tmpenv;               /* tell exec where it is now */
    }
    if (!val) {
       safesysfree(environ[i]);
       while (environ[i]) {
           environ[i] = environ[i+1];
           i++;
	}
       return;
    }
    if (!environ[i]) {                 /* does not exist yet */
       environ = (char**)safesysrealloc(environ, (i+2) * sizeof(char*));
       environ[i+1] = NULL;    /* make sure it's null terminated */
    }
    else
       safesysfree(environ[i]);
       nlen = strlen(nam);
       vlen = strlen(val);

       environ[i] = (char*)safesysmalloc((nlen+vlen+2) * sizeof(char));
       /* all that work just for this */
       my_setenv_format(environ[i], nam, nlen, val, vlen);
    } else {
# endif
#   if defined(__CYGWIN__) || defined(EPOC) || defined(__SYMBIAN32__) || defined(__riscos__)
#       if defined(HAS_UNSETENV)
        if (val == NULL) {
            (void)unsetenv(nam);
        } else {
            (void)setenv(nam, val, 1);
        }
#       else /* ! HAS_UNSETENV */
        (void)setenv(nam, val, 1);
#       endif /* HAS_UNSETENV */
#   else
#       if defined(HAS_UNSETENV)
        if (val == NULL) {
            (void)unsetenv(nam);
        } else {
	    const int nlen = strlen(nam);
	    const int vlen = strlen(val);
	    char * const new_env =
                (char*)safesysmalloc((nlen + vlen + 2) * sizeof(char));
            my_setenv_format(new_env, nam, nlen, val, vlen);
            (void)putenv(new_env);
        }
#       else /* ! HAS_UNSETENV */
        char *new_env;
	const int nlen = strlen(nam);
	int vlen;
        if (!val) {
	   val = "";
        }
        vlen = strlen(val);
        new_env = (char*)safesysmalloc((nlen + vlen + 2) * sizeof(char));
        /* all that work just for this */
        my_setenv_format(new_env, nam, nlen, val, vlen);
        (void)putenv(new_env);
#       endif /* HAS_UNSETENV */
#   endif /* __CYGWIN__ */
#ifndef PERL_USE_SAFE_PUTENV
    }
#endif
  }
}

#else /* WIN32 || NETWARE */

void
Perl_my_setenv(pTHX_ const char *nam, const char *val)
{
    dVAR;
    register char *envstr;
    const int nlen = strlen(nam);
    int vlen;

    if (!val) {
       val = "";
    }
    vlen = strlen(val);
    Newx(envstr, nlen+vlen+2, char);
    my_setenv_format(envstr, nam, nlen, val, vlen);
    (void)PerlEnv_putenv(envstr);
    Safefree(envstr);
}

#endif /* WIN32 || NETWARE */

#endif /* !VMS && !EPOC*/

#ifdef UNLINK_ALL_VERSIONS
I32
Perl_unlnk(pTHX_ const char *f)	/* unlink all versions of a file */
{
    I32 retries = 0;

    PERL_ARGS_ASSERT_UNLNK;

    while (PerlLIO_unlink(f) >= 0)
	retries++;
    return retries ? 0 : -1;
}
#endif

/* this is a drop-in replacement for bcopy() */
#if (!defined(HAS_MEMCPY) && !defined(HAS_BCOPY)) || (!defined(HAS_MEMMOVE) && !defined(HAS_SAFE_MEMCPY) && !defined(HAS_SAFE_BCOPY))
char *
Perl_my_bcopy(register const char *from,register char *to,register I32 len)
{
    char * const retval = to;

    PERL_ARGS_ASSERT_MY_BCOPY;

    if (from - to >= 0) {
	while (len--)
	    *to++ = *from++;
    }
    else {
	to += len;
	from += len;
	while (len--)
	    *(--to) = *(--from);
    }
    return retval;
}
#endif

/* this is a drop-in replacement for memset() */
#ifndef HAS_MEMSET
void *
Perl_my_memset(register char *loc, register I32 ch, register I32 len)
{
    char * const retval = loc;

    PERL_ARGS_ASSERT_MY_MEMSET;

    while (len--)
	*loc++ = ch;
    return retval;
}
#endif

/* this is a drop-in replacement for bzero() */
#if !defined(HAS_BZERO) && !defined(HAS_MEMSET)
char *
Perl_my_bzero(register char *loc, register I32 len)
{
    char * const retval = loc;

    PERL_ARGS_ASSERT_MY_BZERO;

    while (len--)
	*loc++ = 0;
    return retval;
}
#endif

/* this is a drop-in replacement for memcmp() */
#if !defined(HAS_MEMCMP) || !defined(HAS_SANE_MEMCMP)
I32
Perl_my_memcmp(const char *s1, const char *s2, register I32 len)
{
    register const U8 *a = (const U8 *)s1;
    register const U8 *b = (const U8 *)s2;
    register I32 tmp;

    PERL_ARGS_ASSERT_MY_MEMCMP;

    while (len--) {
        if ((tmp = *a++ - *b++))
	    return tmp;
    }
    return 0;
}
#endif /* !HAS_MEMCMP || !HAS_SANE_MEMCMP */

#ifndef HAS_VPRINTF
/* This vsprintf replacement should generally never get used, since
   vsprintf was available in both System V and BSD 2.11.  (There may
   be some cross-compilation or embedded set-ups where it is needed,
   however.)

   If you encounter a problem in this function, it's probably a symptom
   that Configure failed to detect your system's vprintf() function.
   See the section on "item vsprintf" in the INSTALL file.

   This version may compile on systems with BSD-ish <stdio.h>,
   but probably won't on others.
*/

#ifdef USE_CHAR_VSPRINTF
char *
#else
int
#endif
vsprintf(char *dest, const char *pat, void *args)
{
    FILE fakebuf;

#if defined(STDIO_PTR_LVALUE) && defined(STDIO_CNT_LVALUE)
    FILE_ptr(&fakebuf) = (STDCHAR *) dest;
    FILE_cnt(&fakebuf) = 32767;
#else
    /* These probably won't compile -- If you really need
       this, you'll have to figure out some other method. */
    fakebuf._ptr = dest;
    fakebuf._cnt = 32767;
#endif
#ifndef _IOSTRG
#define _IOSTRG 0
#endif
    fakebuf._flag = _IOWRT|_IOSTRG;
    _doprnt(pat, args, &fakebuf);	/* what a kludge */
#if defined(STDIO_PTR_LVALUE)
    *(FILE_ptr(&fakebuf)++) = '\0';
#else
    /* PerlIO has probably #defined away fputc, but we want it here. */
#  ifdef fputc
#    undef fputc  /* XXX Should really restore it later */
#  endif
    (void)fputc('\0', &fakebuf);
#endif
#ifdef USE_CHAR_VSPRINTF
    return(dest);
#else
    return 0;		/* perl doesn't use return value */
#endif
}

#endif /* HAS_VPRINTF */

#ifdef MYSWAP
#if BYTEORDER != 0x4321
short
Perl_my_swap(pTHX_ short s)
{
#if (BYTEORDER & 1) == 0
    short result;

    result = ((s & 255) << 8) + ((s >> 8) & 255);
    return result;
#else
    return s;
#endif
}

long
Perl_my_htonl(pTHX_ long l)
{
    union {
	long result;
	char c[sizeof(long)];
    } u;

#if BYTEORDER == 0x1234 || BYTEORDER == 0x12345678
#if BYTEORDER == 0x12345678
    u.result = 0; 
#endif 
    u.c[0] = (l >> 24) & 255;
    u.c[1] = (l >> 16) & 255;
    u.c[2] = (l >> 8) & 255;
    u.c[3] = l & 255;
    return u.result;
#else
#if ((BYTEORDER - 0x1111) & 0x444) || !(BYTEORDER & 0xf)
    Perl_croak(aTHX_ "Unknown BYTEORDER\n");
#else
    register I32 o;
    register I32 s;

    for (o = BYTEORDER - 0x1111, s = 0; s < (sizeof(long)*8); o >>= 4, s += 8) {
	u.c[o & 0xf] = (l >> s) & 255;
    }
    return u.result;
#endif
#endif
}

long
Perl_my_ntohl(pTHX_ long l)
{
    union {
	long l;
	char c[sizeof(long)];
    } u;

#if BYTEORDER == 0x1234
    u.c[0] = (l >> 24) & 255;
    u.c[1] = (l >> 16) & 255;
    u.c[2] = (l >> 8) & 255;
    u.c[3] = l & 255;
    return u.l;
#else
#if ((BYTEORDER - 0x1111) & 0x444) || !(BYTEORDER & 0xf)
    Perl_croak(aTHX_ "Unknown BYTEORDER\n");
#else
    register I32 o;
    register I32 s;

    u.l = l;
    l = 0;
    for (o = BYTEORDER - 0x1111, s = 0; s < (sizeof(long)*8); o >>= 4, s += 8) {
	l |= (u.c[o & 0xf] & 255) << s;
    }
    return l;
#endif
#endif
}

#endif /* BYTEORDER != 0x4321 */
#endif /* MYSWAP */

/*
 * Little-endian byte order functions - 'v' for 'VAX', or 'reVerse'.
 * If these functions are defined,
 * the BYTEORDER is neither 0x1234 nor 0x4321.
 * However, this is not assumed.
 * -DWS
 */

#define HTOLE(name,type)					\
	type							\
	name (register type n)					\
	{							\
	    union {						\
		type value;					\
		char c[sizeof(type)];				\
	    } u;						\
	    register U32 i;					\
	    register U32 s = 0;					\
	    for (i = 0; i < sizeof(u.c); i++, s += 8) {		\
		u.c[i] = (n >> s) & 0xFF;			\
	    }							\
	    return u.value;					\
	}

#define LETOH(name,type)					\
	type							\
	name (register type n)					\
	{							\
	    union {						\
		type value;					\
		char c[sizeof(type)];				\
	    } u;						\
	    register U32 i;					\
	    register U32 s = 0;					\
	    u.value = n;					\
	    n = 0;						\
	    for (i = 0; i < sizeof(u.c); i++, s += 8) {		\
		n |= ((type)(u.c[i] & 0xFF)) << s;		\
	    }							\
	    return n;						\
	}

/*
 * Big-endian byte order functions.
 */

#define HTOBE(name,type)					\
	type							\
	name (register type n)					\
	{							\
	    union {						\
		type value;					\
		char c[sizeof(type)];				\
	    } u;						\
	    register U32 i;					\
	    register U32 s = 8*(sizeof(u.c)-1);			\
	    for (i = 0; i < sizeof(u.c); i++, s -= 8) {		\
		u.c[i] = (n >> s) & 0xFF;			\
	    }							\
	    return u.value;					\
	}

#define BETOH(name,type)					\
	type							\
	name (register type n)					\
	{							\
	    union {						\
		type value;					\
		char c[sizeof(type)];				\
	    } u;						\
	    register U32 i;					\
	    register U32 s = 8*(sizeof(u.c)-1);			\
	    u.value = n;					\
	    n = 0;						\
	    for (i = 0; i < sizeof(u.c); i++, s -= 8) {		\
		n |= ((type)(u.c[i] & 0xFF)) << s;		\
	    }							\
	    return n;						\
	}

/*
 * If we just can't do it...
 */

#define NOT_AVAIL(name,type)                                    \
        type                                                    \
        name (register type n)                                  \
        {                                                       \
            Perl_croak_nocontext(#name "() not available");     \
            return n; /* not reached */                         \
        }


#if defined(HAS_HTOVS) && !defined(htovs)
HTOLE(htovs,short)
#endif
#if defined(HAS_HTOVL) && !defined(htovl)
HTOLE(htovl,long)
#endif
#if defined(HAS_VTOHS) && !defined(vtohs)
LETOH(vtohs,short)
#endif
#if defined(HAS_VTOHL) && !defined(vtohl)
LETOH(vtohl,long)
#endif

#ifdef PERL_NEED_MY_HTOLE16
# if U16SIZE == 2
HTOLE(Perl_my_htole16,U16)
# else
NOT_AVAIL(Perl_my_htole16,U16)
# endif
#endif
#ifdef PERL_NEED_MY_LETOH16
# if U16SIZE == 2
LETOH(Perl_my_letoh16,U16)
# else
NOT_AVAIL(Perl_my_letoh16,U16)
# endif
#endif
#ifdef PERL_NEED_MY_HTOBE16
# if U16SIZE == 2
HTOBE(Perl_my_htobe16,U16)
# else
NOT_AVAIL(Perl_my_htobe16,U16)
# endif
#endif
#ifdef PERL_NEED_MY_BETOH16
# if U16SIZE == 2
BETOH(Perl_my_betoh16,U16)
# else
NOT_AVAIL(Perl_my_betoh16,U16)
# endif
#endif

#ifdef PERL_NEED_MY_HTOLE32
# if U32SIZE == 4
HTOLE(Perl_my_htole32,U32)
# else
NOT_AVAIL(Perl_my_htole32,U32)
# endif
#endif
#ifdef PERL_NEED_MY_LETOH32
# if U32SIZE == 4
LETOH(Perl_my_letoh32,U32)
# else
NOT_AVAIL(Perl_my_letoh32,U32)
# endif
#endif
#ifdef PERL_NEED_MY_HTOBE32
# if U32SIZE == 4
HTOBE(Perl_my_htobe32,U32)
# else
NOT_AVAIL(Perl_my_htobe32,U32)
# endif
#endif
#ifdef PERL_NEED_MY_BETOH32
# if U32SIZE == 4
BETOH(Perl_my_betoh32,U32)
# else
NOT_AVAIL(Perl_my_betoh32,U32)
# endif
#endif

#ifdef PERL_NEED_MY_HTOLE64
# if U64SIZE == 8
HTOLE(Perl_my_htole64,U64)
# else
NOT_AVAIL(Perl_my_htole64,U64)
# endif
#endif
#ifdef PERL_NEED_MY_LETOH64
# if U64SIZE == 8
LETOH(Perl_my_letoh64,U64)
# else
NOT_AVAIL(Perl_my_letoh64,U64)
# endif
#endif
#ifdef PERL_NEED_MY_HTOBE64
# if U64SIZE == 8
HTOBE(Perl_my_htobe64,U64)
# else
NOT_AVAIL(Perl_my_htobe64,U64)
# endif
#endif
#ifdef PERL_NEED_MY_BETOH64
# if U64SIZE == 8
BETOH(Perl_my_betoh64,U64)
# else
NOT_AVAIL(Perl_my_betoh64,U64)
# endif
#endif

#ifdef PERL_NEED_MY_HTOLES
HTOLE(Perl_my_htoles,short)
#endif
#ifdef PERL_NEED_MY_LETOHS
LETOH(Perl_my_letohs,short)
#endif
#ifdef PERL_NEED_MY_HTOBES
HTOBE(Perl_my_htobes,short)
#endif
#ifdef PERL_NEED_MY_BETOHS
BETOH(Perl_my_betohs,short)
#endif

#ifdef PERL_NEED_MY_HTOLEI
HTOLE(Perl_my_htolei,int)
#endif
#ifdef PERL_NEED_MY_LETOHI
LETOH(Perl_my_letohi,int)
#endif
#ifdef PERL_NEED_MY_HTOBEI
HTOBE(Perl_my_htobei,int)
#endif
#ifdef PERL_NEED_MY_BETOHI
BETOH(Perl_my_betohi,int)
#endif

#ifdef PERL_NEED_MY_HTOLEL
HTOLE(Perl_my_htolel,long)
#endif
#ifdef PERL_NEED_MY_LETOHL
LETOH(Perl_my_letohl,long)
#endif
#ifdef PERL_NEED_MY_HTOBEL
HTOBE(Perl_my_htobel,long)
#endif
#ifdef PERL_NEED_MY_BETOHL
BETOH(Perl_my_betohl,long)
#endif

void
Perl_my_swabn(void *ptr, int n)
{
    register char *s = (char *)ptr;
    register char *e = s + (n-1);
    register char tc;

    PERL_ARGS_ASSERT_MY_SWABN;

    for (n /= 2; n > 0; s++, e--, n--) {
      tc = *s;
      *s = *e;
      *e = tc;
    }
}

PerlIO *
Perl_my_popen_list(pTHX_ const char *mode, int n, SV **args)
{
#if (!defined(DOSISH) || defined(HAS_FORK) || defined(AMIGAOS)) && !defined(OS2) && !defined(VMS) && !defined(__OPEN_VM) && !defined(EPOC) && !defined(NETWARE) && !defined(__LIBCATAMOUNT__)
    dVAR;
    int p[2];
    register I32 This, that;
    register Pid_t pid;
    SV *sv;
    I32 did_pipes = 0;
    int pp[2];

    PERL_ARGS_ASSERT_MY_POPEN_LIST;

    PERL_FLUSHALL_FOR_CHILD;
    This = (*mode == 'w');
    that = !This;
    if (PL_tainting) {
	taint_env();
	taint_proper("Insecure %s%s", "EXEC");
    }
    if (PerlProc_pipe(p) < 0)
	return NULL;
    /* Try for another pipe pair for error return */
    if (PerlProc_pipe(pp) >= 0)
	did_pipes = 1;
    while ((pid = PerlProc_fork()) < 0) {
	if (errno != EAGAIN) {
	    PerlLIO_close(p[This]);
	    PerlLIO_close(p[that]);
	    if (did_pipes) {
		PerlLIO_close(pp[0]);
		PerlLIO_close(pp[1]);
	    }
	    return NULL;
	}
	Perl_ck_warner(aTHX_ packWARN(WARN_PIPE), "Can't fork, trying again in 5 seconds");
	sleep(5);
    }
    if (pid == 0) {
	/* Child */
#undef THIS
#undef THAT
#define THIS that
#define THAT This
	/* Close parent's end of error status pipe (if any) */
	if (did_pipes) {
	    PerlLIO_close(pp[0]);
#if defined(HAS_FCNTL) && defined(F_SETFD)
	    /* Close error pipe automatically if exec works */
	    fcntl(pp[1], F_SETFD, FD_CLOEXEC);
#endif
	}
	/* Now dup our end of _the_ pipe to right position */
	if (p[THIS] != (*mode == 'r')) {
	    PerlLIO_dup2(p[THIS], *mode == 'r');
	    PerlLIO_close(p[THIS]);
	    if (p[THAT] != (*mode == 'r'))	/* if dup2() didn't close it */
		PerlLIO_close(p[THAT]);	/* close parent's end of _the_ pipe */
	}
	else
	    PerlLIO_close(p[THAT]);	/* close parent's end of _the_ pipe */
#if !defined(HAS_FCNTL) || !defined(F_SETFD)
	/* No automatic close - do it by hand */
#  ifndef NOFILE
#  define NOFILE 20
#  endif
	{
	    int fd;

	    for (fd = PL_maxsysfd + 1; fd < NOFILE; fd++) {
		if (fd != pp[1])
		    PerlLIO_close(fd);
	    }
	}
#endif
	do_aexec5(NULL, args-1, args-1+n, pp[1], did_pipes);
	PerlProc__exit(1);
#undef THIS
#undef THAT
    }
    /* Parent */
    do_execfree();	/* free any memory malloced by child on fork */
    if (did_pipes)
	PerlLIO_close(pp[1]);
    /* Keep the lower of the two fd numbers */
    if (p[that] < p[This]) {
	PerlLIO_dup2(p[This], p[that]);
	PerlLIO_close(p[This]);
	p[This] = p[that];
    }
    else
	PerlLIO_close(p[that]);		/* close child's end of pipe */

    sv = *av_fetch(PL_fdpid,p[This],TRUE);
    SvUPGRADE(sv,SVt_IV);
    SvIV_set(sv, pid);
    PL_forkprocess = pid;
    /* If we managed to get status pipe check for exec fail */
    if (did_pipes && pid > 0) {
	int errkid;
	unsigned n = 0;
	SSize_t n1;

	while (n < sizeof(int)) {
	    n1 = PerlLIO_read(pp[0],
			      (void*)(((char*)&errkid)+n),
			      (sizeof(int)) - n);
	    if (n1 <= 0)
		break;
	    n += n1;
	}
	PerlLIO_close(pp[0]);
	did_pipes = 0;
	if (n) {			/* Error */
	    int pid2, status;
	    PerlLIO_close(p[This]);
	    if (n != sizeof(int))
		Perl_croak(aTHX_ "panic: kid popen errno read, n=%u", n);
	    do {
		pid2 = wait4pid(pid, &status, 0);
	    } while (pid2 == -1 && errno == EINTR);
	    errno = errkid;		/* Propagate errno from kid */
	    return NULL;
	}
    }
    if (did_pipes)
	 PerlLIO_close(pp[0]);
    return PerlIO_fdopen(p[This], mode);
#else
#  ifdef OS2	/* Same, without fork()ing and all extra overhead... */
    return my_syspopen4(aTHX_ NULL, mode, n, args);
#  else
    Perl_croak(aTHX_ "List form of piped open not implemented");
    return (PerlIO *) NULL;
#  endif
#endif
}

    /* VMS' my_popen() is in VMS.c, same with OS/2. */
#if (!defined(DOSISH) || defined(HAS_FORK) || defined(AMIGAOS)) && !defined(VMS) && !defined(__OPEN_VM) && !defined(EPOC) && !defined(__LIBCATAMOUNT__)
PerlIO *
Perl_my_popen(pTHX_ const char *cmd, const char *mode)
{
    dVAR;
    int p[2];
    register I32 This, that;
    register Pid_t pid;
    SV *sv;
    const I32 doexec = !(*cmd == '-' && cmd[1] == '\0');
    I32 did_pipes = 0;
    int pp[2];

    PERL_ARGS_ASSERT_MY_POPEN;

    PERL_FLUSHALL_FOR_CHILD;
#ifdef OS2
    if (doexec) {
	return my_syspopen(aTHX_ cmd,mode);
    }
#endif
    This = (*mode == 'w');
    that = !This;
    if (doexec && PL_tainting) {
	taint_env();
	taint_proper("Insecure %s%s", "EXEC");
    }
    if (PerlProc_pipe(p) < 0)
	return NULL;
    if (doexec && PerlProc_pipe(pp) >= 0)
	did_pipes = 1;
    while ((pid = PerlProc_fork()) < 0) {
	if (errno != EAGAIN) {
	    PerlLIO_close(p[This]);
	    PerlLIO_close(p[that]);
	    if (did_pipes) {
		PerlLIO_close(pp[0]);
		PerlLIO_close(pp[1]);
	    }
	    if (!doexec)
		Perl_croak(aTHX_ "Can't fork: %s", Strerror(errno));
	    return NULL;
	}
	Perl_ck_warner(aTHX_ packWARN(WARN_PIPE), "Can't fork, trying again in 5 seconds");
	sleep(5);
    }
    if (pid == 0) {

#undef THIS
#undef THAT
#define THIS that
#define THAT This
	if (did_pipes) {
	    PerlLIO_close(pp[0]);
#if defined(HAS_FCNTL) && defined(F_SETFD)
	    fcntl(pp[1], F_SETFD, FD_CLOEXEC);
#endif
	}
	if (p[THIS] != (*mode == 'r')) {
	    PerlLIO_dup2(p[THIS], *mode == 'r');
	    PerlLIO_close(p[THIS]);
	    if (p[THAT] != (*mode == 'r'))	/* if dup2() didn't close it */
		PerlLIO_close(p[THAT]);
	}
	else
	    PerlLIO_close(p[THAT]);
#ifndef OS2
	if (doexec) {
#if !defined(HAS_FCNTL) || !defined(F_SETFD)
#ifndef NOFILE
#define NOFILE 20
#endif
	    {
		int fd;

		for (fd = PL_maxsysfd + 1; fd < NOFILE; fd++)
		    if (fd != pp[1])
			PerlLIO_close(fd);
	    }
#endif
	    /* may or may not use the shell */
	    do_exec3(cmd, pp[1], did_pipes);
	    PerlProc__exit(1);
	}
#endif	/* defined OS2 */

#ifdef PERLIO_USING_CRLF
   /* Since we circumvent IO layers when we manipulate low-level
      filedescriptors directly, need to manually switch to the
      default, binary, low-level mode; see PerlIOBuf_open(). */
   PerlLIO_setmode((*mode == 'r'), O_BINARY);
#endif 
	PL_forkprocess = 0;
#ifdef PERL_USES_PL_PIDSTATUS
	hv_clear(PL_pidstatus);	/* we have no children */
#endif
	return NULL;
#undef THIS
#undef THAT
    }
    do_execfree();	/* free any memory malloced by child on vfork */
    if (did_pipes)
	PerlLIO_close(pp[1]);
    if (p[that] < p[This]) {
	PerlLIO_dup2(p[This], p[that]);
	PerlLIO_close(p[This]);
	p[This] = p[that];
    }
    else
	PerlLIO_close(p[that]);

    sv = *av_fetch(PL_fdpid,p[This],TRUE);
    SvUPGRADE(sv,SVt_IV);
    SvIV_set(sv, pid);
    PL_forkprocess = pid;
    if (did_pipes && pid > 0) {
	int errkid;
	unsigned n = 0;
	SSize_t n1;

	while (n < sizeof(int)) {
	    n1 = PerlLIO_read(pp[0],
			      (void*)(((char*)&errkid)+n),
			      (sizeof(int)) - n);
	    if (n1 <= 0)
		break;
	    n += n1;
	}
	PerlLIO_close(pp[0]);
	did_pipes = 0;
	if (n) {			/* Error */
	    int pid2, status;
	    PerlLIO_close(p[This]);
	    if (n != sizeof(int))
		Perl_croak(aTHX_ "panic: kid popen errno read, n=%u", n);
	    do {
		pid2 = wait4pid(pid, &status, 0);
	    } while (pid2 == -1 && errno == EINTR);
	    errno = errkid;		/* Propagate errno from kid */
	    return NULL;
	}
    }
    if (did_pipes)
	 PerlLIO_close(pp[0]);
    return PerlIO_fdopen(p[This], mode);
}
#else
#if defined(atarist) || defined(EPOC)
FILE *popen();
PerlIO *
Perl_my_popen(pTHX_ const char *cmd, const char *mode)
{
    PERL_ARGS_ASSERT_MY_POPEN;
    PERL_FLUSHALL_FOR_CHILD;
    /* Call system's popen() to get a FILE *, then import it.
       used 0 for 2nd parameter to PerlIO_importFILE;
       apparently not used
    */
    return PerlIO_importFILE(popen(cmd, mode), 0);
}
#else
#if defined(DJGPP)
FILE *djgpp_popen();
PerlIO *
Perl_my_popen(pTHX_ const char *cmd, const char *mode)
{
    PERL_FLUSHALL_FOR_CHILD;
    /* Call system's popen() to get a FILE *, then import it.
       used 0 for 2nd parameter to PerlIO_importFILE;
       apparently not used
    */
    return PerlIO_importFILE(djgpp_popen(cmd, mode), 0);
}
#else
#if defined(__LIBCATAMOUNT__)
PerlIO *
Perl_my_popen(pTHX_ const char *cmd, const char *mode)
{
    return NULL;
}
#endif
#endif
#endif

#endif /* !DOSISH */

/* this is called in parent before the fork() */
void
Perl_atfork_lock(void)
{
   dVAR;
#if defined(USE_ITHREADS)
    /* locks must be held in locking order (if any) */
#  ifdef MYMALLOC
    MUTEX_LOCK(&PL_malloc_mutex);
#  endif
    OP_REFCNT_LOCK;
#endif
}

/* this is called in both parent and child after the fork() */
void
Perl_atfork_unlock(void)
{
    dVAR;
#if defined(USE_ITHREADS)
    /* locks must be released in same order as in atfork_lock() */
#  ifdef MYMALLOC
    MUTEX_UNLOCK(&PL_malloc_mutex);
#  endif
    OP_REFCNT_UNLOCK;
#endif
}

Pid_t
Perl_my_fork(void)
{
#if defined(HAS_FORK)
    Pid_t pid;
#if defined(USE_ITHREADS) && !defined(HAS_PTHREAD_ATFORK)
    atfork_lock();
    pid = fork();
    atfork_unlock();
#else
    /* atfork_lock() and atfork_unlock() are installed as pthread_atfork()
     * handlers elsewhere in the code */
    pid = fork();
#endif
    return pid;
#else
    /* this "canna happen" since nothing should be calling here if !HAS_FORK */
    Perl_croak_nocontext("fork() not available");
    return 0;
#endif /* HAS_FORK */
}

#ifdef DUMP_FDS
void
Perl_dump_fds(pTHX_ const char *const s)
{
    int fd;
    Stat_t tmpstatbuf;

    PERL_ARGS_ASSERT_DUMP_FDS;

    PerlIO_printf(Perl_debug_log,"%s", s);
    for (fd = 0; fd < 32; fd++) {
	if (PerlLIO_fstat(fd,&tmpstatbuf) >= 0)
	    PerlIO_printf(Perl_debug_log," %d",fd);
    }
    PerlIO_printf(Perl_debug_log,"\n");
    return;
}
#endif	/* DUMP_FDS */

#ifndef HAS_DUP2
int
dup2(int oldfd, int newfd)
{
#if defined(HAS_FCNTL) && defined(F_DUPFD)
    if (oldfd == newfd)
	return oldfd;
    PerlLIO_close(newfd);
    return fcntl(oldfd, F_DUPFD, newfd);
#else
#define DUP2_MAX_FDS 256
    int fdtmp[DUP2_MAX_FDS];
    I32 fdx = 0;
    int fd;

    if (oldfd == newfd)
	return oldfd;
    PerlLIO_close(newfd);
    /* good enough for low fd's... */
    while ((fd = PerlLIO_dup(oldfd)) != newfd && fd >= 0) {
	if (fdx >= DUP2_MAX_FDS) {
	    PerlLIO_close(fd);
	    fd = -1;
	    break;
	}
	fdtmp[fdx++] = fd;
    }
    while (fdx > 0)
	PerlLIO_close(fdtmp[--fdx]);
    return fd;
#endif
}
#endif

#ifndef PERL_MICRO
#ifdef HAS_SIGACTION

Sighandler_t
Perl_rsignal(pTHX_ int signo, Sighandler_t handler)
{
    dVAR;
    struct sigaction act, oact;

#ifdef USE_ITHREADS
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
	return (Sighandler_t) SIG_ERR;
#endif

    act.sa_handler = (void(*)(int))handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_RESTART
    if (PL_signals & PERL_SIGNALS_UNSAFE_FLAG)
        act.sa_flags |= SA_RESTART;	/* SVR4, 4.3+BSD */
#endif
#if defined(SA_NOCLDWAIT) && !defined(BSDish) /* See [perl #18849] */
    if (signo == SIGCHLD && handler == (Sighandler_t) SIG_IGN)
	act.sa_flags |= SA_NOCLDWAIT;
#endif
    if (sigaction(signo, &act, &oact) == -1)
    	return (Sighandler_t) SIG_ERR;
    else
    	return (Sighandler_t) oact.sa_handler;
}

Sighandler_t
Perl_rsignal_state(pTHX_ int signo)
{
    struct sigaction oact;
    PERL_UNUSED_CONTEXT;

    if (sigaction(signo, (struct sigaction *)NULL, &oact) == -1)
	return (Sighandler_t) SIG_ERR;
    else
	return (Sighandler_t) oact.sa_handler;
}

int
Perl_rsignal_save(pTHX_ int signo, Sighandler_t handler, Sigsave_t *save)
{
    dVAR;
    struct sigaction act;

    PERL_ARGS_ASSERT_RSIGNAL_SAVE;

#ifdef USE_ITHREADS
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
	return -1;
#endif

    act.sa_handler = (void(*)(int))handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_RESTART
    if (PL_signals & PERL_SIGNALS_UNSAFE_FLAG)
        act.sa_flags |= SA_RESTART;	/* SVR4, 4.3+BSD */
#endif
#if defined(SA_NOCLDWAIT) && !defined(BSDish) /* See [perl #18849] */
    if (signo == SIGCHLD && handler == (Sighandler_t) SIG_IGN)
	act.sa_flags |= SA_NOCLDWAIT;
#endif
    return sigaction(signo, &act, save);
}

int
Perl_rsignal_restore(pTHX_ int signo, Sigsave_t *save)
{
    dVAR;
#ifdef USE_ITHREADS
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
	return -1;
#endif

    return sigaction(signo, save, (struct sigaction *)NULL);
}

#else /* !HAS_SIGACTION */

Sighandler_t
Perl_rsignal(pTHX_ int signo, Sighandler_t handler)
{
#if defined(USE_ITHREADS) && !defined(WIN32)
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
	return (Sighandler_t) SIG_ERR;
#endif

    return PerlProc_signal(signo, handler);
}

static Signal_t
sig_trap(int signo)
{
    dVAR;
    PL_sig_trapped++;
}

Sighandler_t
Perl_rsignal_state(pTHX_ int signo)
{
    dVAR;
    Sighandler_t oldsig;

#if defined(USE_ITHREADS) && !defined(WIN32)
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
	return (Sighandler_t) SIG_ERR;
#endif

    PL_sig_trapped = 0;
    oldsig = PerlProc_signal(signo, sig_trap);
    PerlProc_signal(signo, oldsig);
    if (PL_sig_trapped)
	PerlProc_kill(PerlProc_getpid(), signo);
    return oldsig;
}

int
Perl_rsignal_save(pTHX_ int signo, Sighandler_t handler, Sigsave_t *save)
{
#if defined(USE_ITHREADS) && !defined(WIN32)
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
	return -1;
#endif
    *save = PerlProc_signal(signo, handler);
    return (*save == (Sighandler_t) SIG_ERR) ? -1 : 0;
}

int
Perl_rsignal_restore(pTHX_ int signo, Sigsave_t *save)
{
#if defined(USE_ITHREADS) && !defined(WIN32)
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
	return -1;
#endif
    return (PerlProc_signal(signo, *save) == (Sighandler_t) SIG_ERR) ? -1 : 0;
}

#endif /* !HAS_SIGACTION */
#endif /* !PERL_MICRO */

    /* VMS' my_pclose() is in VMS.c; same with OS/2 */
#if (!defined(DOSISH) || defined(HAS_FORK) || defined(AMIGAOS)) && !defined(VMS) && !defined(__OPEN_VM) && !defined(EPOC) && !defined(__LIBCATAMOUNT__)
I32
Perl_my_pclose(pTHX_ PerlIO *ptr)
{
    dVAR;
    Sigsave_t hstat, istat, qstat;
    int status;
    SV **svp;
    Pid_t pid;
    Pid_t pid2 = 0;
    bool close_failed;
    dSAVEDERRNO;
    const int fd = PerlIO_fileno(ptr);

#ifdef USE_PERLIO
    /* Find out whether the refcount is low enough for us to wait for the
       child proc without blocking. */
    const bool should_wait = PerlIOUnix_refcnt(fd) == 1;
#else
    const bool should_wait = 1;
#endif

    svp = av_fetch(PL_fdpid,fd,TRUE);
    pid = (SvTYPE(*svp) == SVt_IV) ? SvIVX(*svp) : -1;
    SvREFCNT_dec(*svp);
    *svp = &PL_sv_undef;
#ifdef OS2
    if (pid == -1) {			/* Opened by popen. */
	return my_syspclose(ptr);
    }
#endif
    close_failed = (PerlIO_close(ptr) == EOF);
    SAVE_ERRNO;
#ifdef UTS
    if(PerlProc_kill(pid, 0) < 0) { return(pid); }   /* HOM 12/23/91 */
#endif
#ifndef PERL_MICRO
    rsignal_save(SIGHUP,  (Sighandler_t) SIG_IGN, &hstat);
    rsignal_save(SIGINT,  (Sighandler_t) SIG_IGN, &istat);
    rsignal_save(SIGQUIT, (Sighandler_t) SIG_IGN, &qstat);
#endif
    if (should_wait) do {
	pid2 = wait4pid(pid, &status, 0);
    } while (pid2 == -1 && errno == EINTR);
#ifndef PERL_MICRO
    rsignal_restore(SIGHUP, &hstat);
    rsignal_restore(SIGINT, &istat);
    rsignal_restore(SIGQUIT, &qstat);
#endif
    if (close_failed) {
	RESTORE_ERRNO;
	return -1;
    }
    return(
      should_wait
       ? pid2 < 0 ? pid2 : status == 0 ? 0 : (errno = 0, status)
       : 0
    );
}
#else
#if defined(__LIBCATAMOUNT__)
I32
Perl_my_pclose(pTHX_ PerlIO *ptr)
{
    return -1;
}
#endif
#endif /* !DOSISH */

#if  (!defined(DOSISH) || defined(OS2) || defined(WIN32) || defined(NETWARE)) && !defined(__LIBCATAMOUNT__)
I32
Perl_wait4pid(pTHX_ Pid_t pid, int *statusp, int flags)
{
    dVAR;
    I32 result = 0;
    PERL_ARGS_ASSERT_WAIT4PID;
    if (!pid)
	return -1;
#ifdef PERL_USES_PL_PIDSTATUS
    {
	if (pid > 0) {
	    /* The keys in PL_pidstatus are now the raw 4 (or 8) bytes of the
	       pid, rather than a string form.  */
	    SV * const * const svp = hv_fetch(PL_pidstatus,(const char*) &pid,sizeof(Pid_t),FALSE);
	    if (svp && *svp != &PL_sv_undef) {
		*statusp = SvIVX(*svp);
		(void)hv_delete(PL_pidstatus,(const char*) &pid,sizeof(Pid_t),
				G_DISCARD);
		return pid;
	    }
	}
	else {
	    HE *entry;

	    hv_iterinit(PL_pidstatus);
	    if ((entry = hv_iternext(PL_pidstatus))) {
		SV * const sv = hv_iterval(PL_pidstatus,entry);
		I32 len;
		const char * const spid = hv_iterkey(entry,&len);

		assert (len == sizeof(Pid_t));
		memcpy((char *)&pid, spid, len);
		*statusp = SvIVX(sv);
		/* The hash iterator is currently on this entry, so simply
		   calling hv_delete would trigger the lazy delete, which on
		   aggregate does more work, beacuse next call to hv_iterinit()
		   would spot the flag, and have to call the delete routine,
		   while in the meantime any new entries can't re-use that
		   memory.  */
		hv_iterinit(PL_pidstatus);
		(void)hv_delete(PL_pidstatus,spid,len,G_DISCARD);
		return pid;
	    }
	}
    }
#endif
#ifdef HAS_WAITPID
#  ifdef HAS_WAITPID_RUNTIME
    if (!HAS_WAITPID_RUNTIME)
	goto hard_way;
#  endif
    result = PerlProc_waitpid(pid,statusp,flags);
    goto finish;
#endif
#if !defined(HAS_WAITPID) && defined(HAS_WAIT4)
    result = wait4((pid==-1)?0:pid,statusp,flags,NULL);
    goto finish;
#endif
#ifdef PERL_USES_PL_PIDSTATUS
#if defined(HAS_WAITPID) && defined(HAS_WAITPID_RUNTIME)
  hard_way:
#endif
    {
	if (flags)
	    Perl_croak(aTHX_ "Can't do waitpid with flags");
	else {
	    while ((result = PerlProc_wait(statusp)) != pid && pid > 0 && result >= 0)
		pidgone(result,*statusp);
	    if (result < 0)
		*statusp = -1;
	}
    }
#endif
#if defined(HAS_WAITPID) || defined(HAS_WAIT4)
  finish:
#endif
    if (result < 0 && errno == EINTR) {
	PERL_ASYNC_CHECK();
	errno = EINTR; /* reset in case a signal handler changed $! */
    }
    return result;
}
#endif /* !DOSISH || OS2 || WIN32 || NETWARE */

#ifdef PERL_USES_PL_PIDSTATUS
void
S_pidgone(pTHX_ Pid_t pid, int status)
{
    register SV *sv;

    sv = *hv_fetch(PL_pidstatus,(const char*)&pid,sizeof(Pid_t),TRUE);
    SvUPGRADE(sv,SVt_IV);
    SvIV_set(sv, status);
    return;
}
#endif

#if defined(atarist) || defined(OS2) || defined(EPOC)
int pclose();
#ifdef HAS_FORK
int					/* Cannot prototype with I32
					   in os2ish.h. */
my_syspclose(PerlIO *ptr)
#else
I32
Perl_my_pclose(pTHX_ PerlIO *ptr)
#endif
{
    /* Needs work for PerlIO ! */
    FILE * const f = PerlIO_findFILE(ptr);
    const I32 result = pclose(f);
    PerlIO_releaseFILE(ptr,f);
    return result;
}
#endif

#if defined(DJGPP)
int djgpp_pclose();
I32
Perl_my_pclose(pTHX_ PerlIO *ptr)
{
    /* Needs work for PerlIO ! */
    FILE * const f = PerlIO_findFILE(ptr);
    I32 result = djgpp_pclose(f);
    result = (result << 8) & 0xff00;
    PerlIO_releaseFILE(ptr,f);
    return result;
}
#endif

#define PERL_REPEATCPY_LINEAR 4
void
Perl_repeatcpy(register char *to, register const char *from, I32 len, register IV count)
{
    PERL_ARGS_ASSERT_REPEATCPY;

    if (count < 0)
	Perl_croak_nocontext("%s",PL_memory_wrap);

    if (len == 1)
	memset(to, *from, count);
    else if (count) {
	register char *p = to;
	IV items, linear, half;

	linear = count < PERL_REPEATCPY_LINEAR ? count : PERL_REPEATCPY_LINEAR;
	for (items = 0; items < linear; ++items) {
	    register const char *q = from;
	    IV todo;
	    for (todo = len; todo > 0; todo--)
		*p++ = *q++;
        }

	half = count / 2;
	while (items <= half) {
	    IV size = items * len;
	    memcpy(p, to, size);
	    p     += size;
	    items *= 2;
	}

	if (count > items)
	    memcpy(p, to, (count - items) * len);
    }
}

#ifndef HAS_RENAME
I32
Perl_same_dirent(pTHX_ const char *a, const char *b)
{
    char *fa = strrchr(a,'/');
    char *fb = strrchr(b,'/');
    Stat_t tmpstatbuf1;
    Stat_t tmpstatbuf2;
    SV * const tmpsv = sv_newmortal();

    PERL_ARGS_ASSERT_SAME_DIRENT;

    if (fa)
	fa++;
    else
	fa = a;
    if (fb)
	fb++;
    else
	fb = b;
    if (strNE(a,b))
	return FALSE;
    if (fa == a)
	sv_setpvs(tmpsv, ".");
    else
	sv_setpvn(tmpsv, a, fa - a);
    if (PerlLIO_stat(SvPVX_const(tmpsv), &tmpstatbuf1) < 0)
	return FALSE;
    if (fb == b)
	sv_setpvs(tmpsv, ".");
    else
	sv_setpvn(tmpsv, b, fb - b);
    if (PerlLIO_stat(SvPVX_const(tmpsv), &tmpstatbuf2) < 0)
	return FALSE;
    return tmpstatbuf1.st_dev == tmpstatbuf2.st_dev &&
	   tmpstatbuf1.st_ino == tmpstatbuf2.st_ino;
}
#endif /* !HAS_RENAME */

char*
Perl_find_script(pTHX_ const char *scriptname, bool dosearch,
		 const char *const *const search_ext, I32 flags)
{
    dVAR;
    const char *xfound = NULL;
    char *xfailed = NULL;
    char tmpbuf[MAXPATHLEN];
    register char *s;
    I32 len = 0;
    int retval;
    char *bufend;
#if defined(DOSISH) && !defined(OS2) && !defined(atarist)
#  define SEARCH_EXTS ".bat", ".cmd", NULL
#  define MAX_EXT_LEN 4
#endif
#ifdef OS2
#  define SEARCH_EXTS ".cmd", ".btm", ".bat", ".pl", NULL
#  define MAX_EXT_LEN 4
#endif
#ifdef VMS
#  define SEARCH_EXTS ".pl", ".com", NULL
#  define MAX_EXT_LEN 4
#endif
    /* additional extensions to try in each dir if scriptname not found */
#ifdef SEARCH_EXTS
    static const char *const exts[] = { SEARCH_EXTS };
    const char *const *const ext = search_ext ? search_ext : exts;
    int extidx = 0, i = 0;
    const char *curext = NULL;
#else
    PERL_UNUSED_ARG(search_ext);
#  define MAX_EXT_LEN 0
#endif

    PERL_ARGS_ASSERT_FIND_SCRIPT;

    /*
     * If dosearch is true and if scriptname does not contain path
     * delimiters, search the PATH for scriptname.
     *
     * If SEARCH_EXTS is also defined, will look for each
     * scriptname{SEARCH_EXTS} whenever scriptname is not found
     * while searching the PATH.
     *
     * Assuming SEARCH_EXTS is C<".foo",".bar",NULL>, PATH search
     * proceeds as follows:
     *   If DOSISH or VMSISH:
     *     + look for ./scriptname{,.foo,.bar}
     *     + search the PATH for scriptname{,.foo,.bar}
     *
     *   If !DOSISH:
     *     + look *only* in the PATH for scriptname{,.foo,.bar} (note
     *       this will not look in '.' if it's not in the PATH)
     */
    tmpbuf[0] = '\0';

#ifdef VMS
#  ifdef ALWAYS_DEFTYPES
    len = strlen(scriptname);
    if (!(len == 1 && *scriptname == '-') && scriptname[len-1] != ':') {
	int idx = 0, deftypes = 1;
	bool seen_dot = 1;

	const int hasdir = !dosearch || (strpbrk(scriptname,":[</") != NULL);
#  else
    if (dosearch) {
	int idx = 0, deftypes = 1;
	bool seen_dot = 1;

	const int hasdir = (strpbrk(scriptname,":[</") != NULL);
#  endif
	/* The first time through, just add SEARCH_EXTS to whatever we
	 * already have, so we can check for default file types. */
	while (deftypes ||
	       (!hasdir && my_trnlnm("DCL$PATH",tmpbuf,idx++)) )
	{
	    if (deftypes) {
		deftypes = 0;
		*tmpbuf = '\0';
	    }
	    if ((strlen(tmpbuf) + strlen(scriptname)
		 + MAX_EXT_LEN) >= sizeof tmpbuf)
		continue;	/* don't search dir with too-long name */
	    my_strlcat(tmpbuf, scriptname, sizeof(tmpbuf));
#else  /* !VMS */

#ifdef DOSISH
    if (strEQ(scriptname, "-"))
 	dosearch = 0;
    if (dosearch) {		/* Look in '.' first. */
	const char *cur = scriptname;
#ifdef SEARCH_EXTS
	if ((curext = strrchr(scriptname,'.')))	/* possible current ext */
	    while (ext[i])
		if (strEQ(ext[i++],curext)) {
		    extidx = -1;		/* already has an ext */
		    break;
		}
	do {
#endif
	    DEBUG_p(PerlIO_printf(Perl_debug_log,
				  "Looking for %s\n",cur));
	    if (PerlLIO_stat(cur,&PL_statbuf) >= 0
		&& !S_ISDIR(PL_statbuf.st_mode)) {
		dosearch = 0;
		scriptname = cur;
#ifdef SEARCH_EXTS
		break;
#endif
	    }
#ifdef SEARCH_EXTS
	    if (cur == scriptname) {
		len = strlen(scriptname);
		if (len+MAX_EXT_LEN+1 >= sizeof(tmpbuf))
		    break;
		my_strlcpy(tmpbuf, scriptname, sizeof(tmpbuf));
		cur = tmpbuf;
	    }
	} while (extidx >= 0 && ext[extidx]	/* try an extension? */
		 && my_strlcpy(tmpbuf+len, ext[extidx++], sizeof(tmpbuf) - len));
#endif
    }
#endif

    if (dosearch && !strchr(scriptname, '/')
#ifdef DOSISH
		 && !strchr(scriptname, '\\')
#endif
		 && (s = PerlEnv_getenv("PATH")))
    {
	bool seen_dot = 0;

	bufend = s + strlen(s);
	while (s < bufend) {
#if defined(atarist) || defined(DOSISH)
	    for (len = 0; *s
#  ifdef atarist
		    && *s != ','
#  endif
		    && *s != ';'; len++, s++) {
		if (len < sizeof tmpbuf)
		    tmpbuf[len] = *s;
	    }
	    if (len < sizeof tmpbuf)
		tmpbuf[len] = '\0';
#else  /* ! (atarist || DOSISH) */
	    s = delimcpy(tmpbuf, tmpbuf + sizeof tmpbuf, s, bufend,
			':',
			&len);
#endif /* ! (atarist || DOSISH) */
	    if (s < bufend)
		s++;
	    if (len + 1 + strlen(scriptname) + MAX_EXT_LEN >= sizeof tmpbuf)
		continue;	/* don't search dir with too-long name */
	    if (len
#  if defined(atarist) || defined(DOSISH)
		&& tmpbuf[len - 1] != '/'
		&& tmpbuf[len - 1] != '\\'
#  endif
	       )
		tmpbuf[len++] = '/';
	    if (len == 2 && tmpbuf[0] == '.')
		seen_dot = 1;
	    (void)my_strlcpy(tmpbuf + len, scriptname, sizeof(tmpbuf) - len);
#endif  /* !VMS */

#ifdef SEARCH_EXTS
	    len = strlen(tmpbuf);
	    if (extidx > 0)	/* reset after previous loop */
		extidx = 0;
	    do {
#endif
	    	DEBUG_p(PerlIO_printf(Perl_debug_log, "Looking for %s\n",tmpbuf));
		retval = PerlLIO_stat(tmpbuf,&PL_statbuf);
		if (S_ISDIR(PL_statbuf.st_mode)) {
		    retval = -1;
		}
#ifdef SEARCH_EXTS
	    } while (  retval < 0		/* not there */
		    && extidx>=0 && ext[extidx]	/* try an extension? */
		    && my_strlcpy(tmpbuf+len, ext[extidx++], sizeof(tmpbuf) - len)
		);
#endif
	    if (retval < 0)
		continue;
	    if (S_ISREG(PL_statbuf.st_mode)
		&& cando(S_IRUSR,TRUE,&PL_statbuf)
#if !defined(DOSISH)
		&& cando(S_IXUSR,TRUE,&PL_statbuf)
#endif
		)
	    {
		xfound = tmpbuf;		/* bingo! */
		break;
	    }
	    if (!xfailed)
		xfailed = savepv(tmpbuf);
	}
#ifndef DOSISH
	if (!xfound && !seen_dot && !xfailed &&
	    (PerlLIO_stat(scriptname,&PL_statbuf) < 0
	     || S_ISDIR(PL_statbuf.st_mode)))
#endif
	    seen_dot = 1;			/* Disable message. */
	if (!xfound) {
	    if (flags & 1) {			/* do or die? */
		/* diag_listed_as: Can't execute %s */
		Perl_croak(aTHX_ "Can't %s %s%s%s",
		      (xfailed ? "execute" : "find"),
		      (xfailed ? xfailed : scriptname),
		      (xfailed ? "" : " on PATH"),
		      (xfailed || seen_dot) ? "" : ", '.' not in PATH");
	    }
	    scriptname = NULL;
	}
	Safefree(xfailed);
	scriptname = xfound;
    }
    return (scriptname ? savepv(scriptname) : NULL);
}

#ifndef PERL_GET_CONTEXT_DEFINED

void *
Perl_get_context(void)
{
    dVAR;
#if defined(USE_ITHREADS)
#  ifdef OLD_PTHREADS_API
    pthread_addr_t t;
    int error = pthread_getspecific(PL_thr_key, &t)
    if (error)
	Perl_croak_nocontext("panic: pthread_getspecific, error=%d", error);
    return (void*)t;
#  else
#    ifdef I_MACH_CTHREADS
    return (void*)cthread_data(cthread_self());
#    else
    return (void*)PTHREAD_GETSPECIFIC(PL_thr_key);
#    endif
#  endif
#else
    return (void*)NULL;
#endif
}

void
Perl_set_context(void *t)
{
    dVAR;
    PERL_ARGS_ASSERT_SET_CONTEXT;
#if defined(USE_ITHREADS)
#  ifdef I_MACH_CTHREADS
    cthread_set_data(cthread_self(), t);
#  else
    {
	const int error = pthread_setspecific(PL_thr_key, t);
	if (error)
	    Perl_croak_nocontext("panic: pthread_setspecific, error=%d", error);
    }
#  endif
#else
    PERL_UNUSED_ARG(t);
#endif
}

#endif /* !PERL_GET_CONTEXT_DEFINED */

#if defined(PERL_GLOBAL_STRUCT) && !defined(PERL_GLOBAL_STRUCT_PRIVATE)
struct perl_vars *
Perl_GetVars(pTHX)
{
 return &PL_Vars;
}
#endif

char **
Perl_get_op_names(pTHX)
{
    PERL_UNUSED_CONTEXT;
    return (char **)PL_op_name;
}

char **
Perl_get_op_descs(pTHX)
{
    PERL_UNUSED_CONTEXT;
    return (char **)PL_op_desc;
}

const char *
Perl_get_no_modify(pTHX)
{
    PERL_UNUSED_CONTEXT;
    return PL_no_modify;
}

U32 *
Perl_get_opargs(pTHX)
{
    PERL_UNUSED_CONTEXT;
    return (U32 *)PL_opargs;
}

PPADDR_t*
Perl_get_ppaddr(pTHX)
{
    dVAR;
    PERL_UNUSED_CONTEXT;
    return (PPADDR_t*)PL_ppaddr;
}

#ifndef HAS_GETENV_LEN
char *
Perl_getenv_len(pTHX_ const char *env_elem, unsigned long *len)
{
    char * const env_trans = PerlEnv_getenv(env_elem);
    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_GETENV_LEN;
    if (env_trans)
	*len = strlen(env_trans);
    return env_trans;
}
#endif


MGVTBL*
Perl_get_vtbl(pTHX_ int vtbl_id)
{
    PERL_UNUSED_CONTEXT;

    return (vtbl_id < 0 || vtbl_id >= magic_vtable_max)
	? NULL : PL_magic_vtables + vtbl_id;
}

I32
Perl_my_fflush_all(pTHX)
{
#if defined(USE_PERLIO) || defined(FFLUSH_NULL) || defined(USE_SFIO)
    return PerlIO_flush(NULL);
#else
# if defined(HAS__FWALK)
    extern int fflush(FILE *);
    /* undocumented, unprototyped, but very useful BSDism */
    extern void _fwalk(int (*)(FILE *));
    _fwalk(&fflush);
    return 0;
# else
#  if defined(FFLUSH_ALL) && defined(HAS_STDIO_STREAM_ARRAY)
    long open_max = -1;
#   ifdef PERL_FFLUSH_ALL_FOPEN_MAX
    open_max = PERL_FFLUSH_ALL_FOPEN_MAX;
#   else
#    if defined(HAS_SYSCONF) && defined(_SC_OPEN_MAX)
    open_max = sysconf(_SC_OPEN_MAX);
#     else
#      ifdef FOPEN_MAX
    open_max = FOPEN_MAX;
#      else
#       ifdef OPEN_MAX
    open_max = OPEN_MAX;
#       else
#        ifdef _NFILE
    open_max = _NFILE;
#        endif
#       endif
#      endif
#     endif
#    endif
    if (open_max > 0) {
      long i;
      for (i = 0; i < open_max; i++)
	    if (STDIO_STREAM_ARRAY[i]._file >= 0 &&
		STDIO_STREAM_ARRAY[i]._file < open_max &&
		STDIO_STREAM_ARRAY[i]._flag)
		PerlIO_flush(&STDIO_STREAM_ARRAY[i]);
      return 0;
    }
#  endif
    SETERRNO(EBADF,RMS_IFI);
    return EOF;
# endif
#endif
}

void
Perl_report_wrongway_fh(pTHX_ const GV *gv, const char have)
{
    if (ckWARN(WARN_IO)) {
        SV * const name
           = gv && (isGV(gv) || isGV_with_GP(gv))
                ? sv_2mortal(newSVhek(GvENAME_HEK((gv))))
                : NULL;
	const char * const direction = have == '>' ? "out" : "in";

	if (name && SvPOK(name) && *SvPV_nolen(name))
	    Perl_warner(aTHX_ packWARN(WARN_IO),
			"Filehandle %"SVf" opened only for %sput",
			name, direction);
	else
	    Perl_warner(aTHX_ packWARN(WARN_IO),
			"Filehandle opened only for %sput", direction);
    }
}

void
Perl_report_evil_fh(pTHX_ const GV *gv)
{
    const IO *io = gv ? GvIO(gv) : NULL;
    const PERL_BITFIELD16 op = PL_op->op_type;
    const char *vile;
    I32 warn_type;

    if (io && IoTYPE(io) == IoTYPE_CLOSED) {
	vile = "closed";
	warn_type = WARN_CLOSED;
    }
    else {
	vile = "unopened";
	warn_type = WARN_UNOPENED;
    }

    if (ckWARN(warn_type)) {
        SV * const name
            = gv && (isGV(gv) || isGV_with_GP(gv)) && GvENAMELEN(gv) ?
                                     sv_2mortal(newSVhek(GvENAME_HEK(gv))) : NULL;
	const char * const pars =
	    (const char *)(OP_IS_FILETEST(op) ? "" : "()");
	const char * const func =
	    (const char *)
	    (op == OP_READLINE   ? "readline"  :	/* "<HANDLE>" not nice */
	     op == OP_LEAVEWRITE ? "write" :		/* "write exit" not nice */
	     PL_op_desc[op]);
	const char * const type =
	    (const char *)
	    (OP_IS_SOCKET(op) || (io && IoTYPE(io) == IoTYPE_SOCKET)
	     ? "socket" : "filehandle");
	const bool have_name = name && SvPOK(name) && *SvPV_nolen(name);
	Perl_warner(aTHX_ packWARN(warn_type),
		   "%s%s on %s %s%s%"SVf, func, pars, vile, type,
		    have_name ? " " : "",
		    SVfARG(have_name ? name : &PL_sv_no));
	if (io && IoDIRP(io) && !(IoFLAGS(io) & IOf_FAKE_DIRP))
		Perl_warner(
			    aTHX_ packWARN(warn_type),
			"\t(Are you trying to call %s%s on dirhandle%s%"SVf"?)\n",
			func, pars, have_name ? " " : "",
			SVfARG(have_name ? name : &PL_sv_no)
			    );
    }
}

/* To workaround core dumps from the uninitialised tm_zone we get the
 * system to give us a reasonable struct to copy.  This fix means that
 * strftime uses the tm_zone and tm_gmtoff values returned by
 * localtime(time()). That should give the desired result most of the
 * time. But probably not always!
 *
 * This does not address tzname aspects of NETaa14816.
 *
 */

#ifdef HAS_GNULIBC
# ifndef STRUCT_TM_HASZONE
#    define STRUCT_TM_HASZONE
# endif
#endif

#ifdef STRUCT_TM_HASZONE /* Backward compat */
# ifndef HAS_TM_TM_ZONE
#    define HAS_TM_TM_ZONE
# endif
#endif

void
Perl_init_tm(pTHX_ struct tm *ptm)	/* see mktime, strftime and asctime */
{
#ifdef HAS_TM_TM_ZONE
    Time_t now;
    const struct tm* my_tm;
    PERL_ARGS_ASSERT_INIT_TM;
    (void)time(&now);
    my_tm = localtime(&now);
    if (my_tm)
        Copy(my_tm, ptm, 1, struct tm);
#else
    PERL_ARGS_ASSERT_INIT_TM;
    PERL_UNUSED_ARG(ptm);
#endif
}

/*
 * mini_mktime - normalise struct tm values without the localtime()
 * semantics (and overhead) of mktime().
 */
void
Perl_mini_mktime(pTHX_ struct tm *ptm)
{
    int yearday;
    int secs;
    int month, mday, year, jday;
    int odd_cent, odd_year;
    PERL_UNUSED_CONTEXT;

    PERL_ARGS_ASSERT_MINI_MKTIME;

#define	DAYS_PER_YEAR	365
#define	DAYS_PER_QYEAR	(4*DAYS_PER_YEAR+1)
#define	DAYS_PER_CENT	(25*DAYS_PER_QYEAR-1)
#define	DAYS_PER_QCENT	(4*DAYS_PER_CENT+1)
#define	SECS_PER_HOUR	(60*60)
#define	SECS_PER_DAY	(24*SECS_PER_HOUR)
/* parentheses deliberately absent on these two, otherwise they don't work */
#define	MONTH_TO_DAYS	153/5
#define	DAYS_TO_MONTH	5/153
/* offset to bias by March (month 4) 1st between month/mday & year finding */
#define	YEAR_ADJUST	(4*MONTH_TO_DAYS+1)
/* as used here, the algorithm leaves Sunday as day 1 unless we adjust it */
#define	WEEKDAY_BIAS	6	/* (1+6)%7 makes Sunday 0 again */

/*
 * Year/day algorithm notes:
 *
 * With a suitable offset for numeric value of the month, one can find
 * an offset into the year by considering months to have 30.6 (153/5) days,
 * using integer arithmetic (i.e., with truncation).  To avoid too much
 * messing about with leap days, we consider January and February to be
 * the 13th and 14th month of the previous year.  After that transformation,
 * we need the month index we use to be high by 1 from 'normal human' usage,
 * so the month index values we use run from 4 through 15.
 *
 * Given that, and the rules for the Gregorian calendar (leap years are those
 * divisible by 4 unless also divisible by 100, when they must be divisible
 * by 400 instead), we can simply calculate the number of days since some
 * arbitrary 'beginning of time' by futzing with the (adjusted) year number,
 * the days we derive from our month index, and adding in the day of the
 * month.  The value used here is not adjusted for the actual origin which
 * it normally would use (1 January A.D. 1), since we're not exposing it.
 * We're only building the value so we can turn around and get the
 * normalised values for the year, month, day-of-month, and day-of-year.
 *
 * For going backward, we need to bias the value we're using so that we find
 * the right year value.  (Basically, we don't want the contribution of
 * March 1st to the number to apply while deriving the year).  Having done
 * that, we 'count up' the contribution to the year number by accounting for
 * full quadracenturies (400-year periods) with their extra leap days, plus
 * the contribution from full centuries (to avoid counting in the lost leap
 * days), plus the contribution from full quad-years (to count in the normal
 * leap days), plus the leftover contribution from any non-leap years.
 * At this point, if we were working with an actual leap day, we'll have 0
 * days left over.  This is also true for March 1st, however.  So, we have
 * to special-case that result, and (earlier) keep track of the 'odd'
 * century and year contributions.  If we got 4 extra centuries in a qcent,
 * or 4 extra years in a qyear, then it's a leap day and we call it 29 Feb.
 * Otherwise, we add back in the earlier bias we removed (the 123 from
 * figuring in March 1st), find the month index (integer division by 30.6),
 * and the remainder is the day-of-month.  We then have to convert back to
 * 'real' months (including fixing January and February from being 14/15 in
 * the previous year to being in the proper year).  After that, to get
 * tm_yday, we work with the normalised year and get a new yearday value for
 * January 1st, which we subtract from the yearday value we had earlier,
 * representing the date we've re-built.  This is done from January 1
 * because tm_yday is 0-origin.
 *
 * Since POSIX time routines are only guaranteed to work for times since the
 * UNIX epoch (00:00:00 1 Jan 1970 UTC), the fact that this algorithm
 * applies Gregorian calendar rules even to dates before the 16th century
 * doesn't bother me.  Besides, you'd need cultural context for a given
 * date to know whether it was Julian or Gregorian calendar, and that's
 * outside the scope for this routine.  Since we convert back based on the
 * same rules we used to build the yearday, you'll only get strange results
 * for input which needed normalising, or for the 'odd' century years which
 * were leap years in the Julian calendar but not in the Gregorian one.
 * I can live with that.
 *
 * This algorithm also fails to handle years before A.D. 1 gracefully, but
 * that's still outside the scope for POSIX time manipulation, so I don't
 * care.
 */

    year = 1900 + ptm->tm_year;
    month = ptm->tm_mon;
    mday = ptm->tm_mday;
    /* allow given yday with no month & mday to dominate the result */
    if (ptm->tm_yday >= 0 && mday <= 0 && month <= 0) {
	month = 0;
	mday = 0;
	jday = 1 + ptm->tm_yday;
    }
    else {
	jday = 0;
    }
    if (month >= 2)
	month+=2;
    else
	month+=14, year--;
    yearday = DAYS_PER_YEAR * year + year/4 - year/100 + year/400;
    yearday += month*MONTH_TO_DAYS + mday + jday;
    /*
     * Note that we don't know when leap-seconds were or will be,
     * so we have to trust the user if we get something which looks
     * like a sensible leap-second.  Wild values for seconds will
     * be rationalised, however.
     */
    if ((unsigned) ptm->tm_sec <= 60) {
	secs = 0;
    }
    else {
	secs = ptm->tm_sec;
	ptm->tm_sec = 0;
    }
    secs += 60 * ptm->tm_min;
    secs += SECS_PER_HOUR * ptm->tm_hour;
    if (secs < 0) {
	if (secs-(secs/SECS_PER_DAY*SECS_PER_DAY) < 0) {
	    /* got negative remainder, but need positive time */
	    /* back off an extra day to compensate */
	    yearday += (secs/SECS_PER_DAY)-1;
	    secs -= SECS_PER_DAY * (secs/SECS_PER_DAY - 1);
	}
	else {
	    yearday += (secs/SECS_PER_DAY);
	    secs -= SECS_PER_DAY * (secs/SECS_PER_DAY);
	}
    }
    else if (secs >= SECS_PER_DAY) {
	yearday += (secs/SECS_PER_DAY);
	secs %= SECS_PER_DAY;
    }
    ptm->tm_hour = secs/SECS_PER_HOUR;
    secs %= SECS_PER_HOUR;
    ptm->tm_min = secs/60;
    secs %= 60;
    ptm->tm_sec += secs;
    /* done with time of day effects */
    /*
     * The algorithm for yearday has (so far) left it high by 428.
     * To avoid mistaking a legitimate Feb 29 as Mar 1, we need to
     * bias it by 123 while trying to figure out what year it
     * really represents.  Even with this tweak, the reverse
     * translation fails for years before A.D. 0001.
     * It would still fail for Feb 29, but we catch that one below.
     */
    jday = yearday;	/* save for later fixup vis-a-vis Jan 1 */
    yearday -= YEAR_ADJUST;
    year = (yearday / DAYS_PER_QCENT) * 400;
    yearday %= DAYS_PER_QCENT;
    odd_cent = yearday / DAYS_PER_CENT;
    year += odd_cent * 100;
    yearday %= DAYS_PER_CENT;
    year += (yearday / DAYS_PER_QYEAR) * 4;
    yearday %= DAYS_PER_QYEAR;
    odd_year = yearday / DAYS_PER_YEAR;
    year += odd_year;
    yearday %= DAYS_PER_YEAR;
    if (!yearday && (odd_cent==4 || odd_year==4)) { /* catch Feb 29 */
	month = 1;
	yearday = 29;
    }
    else {
	yearday += YEAR_ADJUST;	/* recover March 1st crock */
	month = yearday*DAYS_TO_MONTH;
	yearday -= month*MONTH_TO_DAYS;
	/* recover other leap-year adjustment */
	if (month > 13) {
	    month-=14;
	    year++;
	}
	else {
	    month-=2;
	}
    }
    ptm->tm_year = year - 1900;
    if (yearday) {
      ptm->tm_mday = yearday;
      ptm->tm_mon = month;
    }
    else {
      ptm->tm_mday = 31;
      ptm->tm_mon = month - 1;
    }
    /* re-build yearday based on Jan 1 to get tm_yday */
    year--;
    yearday = year*DAYS_PER_YEAR + year/4 - year/100 + year/400;
    yearday += 14*MONTH_TO_DAYS + 1;
    ptm->tm_yday = jday - yearday;
    /* fix tm_wday if not overridden by caller */
    if ((unsigned)ptm->tm_wday > 6)
	ptm->tm_wday = (jday + WEEKDAY_BIAS) % 7;
}

char *
Perl_my_strftime(pTHX_ const char *fmt, int sec, int min, int hour, int mday, int mon, int year, int wday, int yday, int isdst)
{
#ifdef HAS_STRFTIME
  char *buf;
  int buflen;
  struct tm mytm;
  int len;

  PERL_ARGS_ASSERT_MY_STRFTIME;

  init_tm(&mytm);	/* XXX workaround - see init_tm() above */
  mytm.tm_sec = sec;
  mytm.tm_min = min;
  mytm.tm_hour = hour;
  mytm.tm_mday = mday;
  mytm.tm_mon = mon;
  mytm.tm_year = year;
  mytm.tm_wday = wday;
  mytm.tm_yday = yday;
  mytm.tm_isdst = isdst;
  mini_mktime(&mytm);
  /* use libc to get the values for tm_gmtoff and tm_zone [perl #18238] */
#if defined(HAS_MKTIME) && (defined(HAS_TM_TM_GMTOFF) || defined(HAS_TM_TM_ZONE))
  STMT_START {
    struct tm mytm2;
    mytm2 = mytm;
    mktime(&mytm2);
#ifdef HAS_TM_TM_GMTOFF
    mytm.tm_gmtoff = mytm2.tm_gmtoff;
#endif
#ifdef HAS_TM_TM_ZONE
    mytm.tm_zone = mytm2.tm_zone;
#endif
  } STMT_END;
#endif
  buflen = 64;
  Newx(buf, buflen, char);
  len = strftime(buf, buflen, fmt, &mytm);
  /*
  ** The following is needed to handle to the situation where
  ** tmpbuf overflows.  Basically we want to allocate a buffer
  ** and try repeatedly.  The reason why it is so complicated
  ** is that getting a return value of 0 from strftime can indicate
  ** one of the following:
  ** 1. buffer overflowed,
  ** 2. illegal conversion specifier, or
  ** 3. the format string specifies nothing to be returned(not
  **	  an error).  This could be because format is an empty string
  **    or it specifies %p that yields an empty string in some locale.
  ** If there is a better way to make it portable, go ahead by
  ** all means.
  */
  if ((len > 0 && len < buflen) || (len == 0 && *fmt == '\0'))
    return buf;
  else {
    /* Possibly buf overflowed - try again with a bigger buf */
    const int fmtlen = strlen(fmt);
    int bufsize = fmtlen + buflen;

    Renew(buf, bufsize, char);
    while (buf) {
      buflen = strftime(buf, bufsize, fmt, &mytm);
      if (buflen > 0 && buflen < bufsize)
	break;
      /* heuristic to prevent out-of-memory errors */
      if (bufsize > 100*fmtlen) {
	Safefree(buf);
	buf = NULL;
	break;
      }
      bufsize *= 2;
      Renew(buf, bufsize, char);
    }
    return buf;
  }
#else
  Perl_croak(aTHX_ "panic: no strftime");
  return NULL;
#endif
}


#define SV_CWD_RETURN_UNDEF \
sv_setsv(sv, &PL_sv_undef); \
return FALSE

#define SV_CWD_ISDOT(dp) \
    (dp->d_name[0] == '.' && (dp->d_name[1] == '\0' || \
	(dp->d_name[1] == '.' && dp->d_name[2] == '\0')))

/*
=head1 Miscellaneous Functions

=for apidoc getcwd_sv

Fill the sv with current working directory

=cut
*/

/* Originally written in Perl by John Bazik; rewritten in C by Ben Sugars.
 * rewritten again by dougm, optimized for use with xs TARG, and to prefer
 * getcwd(3) if available
 * Comments from the orignal:
 *     This is a faster version of getcwd.  It's also more dangerous
 *     because you might chdir out of a directory that you can't chdir
 *     back into. */

int
Perl_getcwd_sv(pTHX_ register SV *sv)
{
#ifndef PERL_MICRO
    dVAR;
#ifndef INCOMPLETE_TAINTS
    SvTAINTED_on(sv);
#endif

    PERL_ARGS_ASSERT_GETCWD_SV;

#ifdef HAS_GETCWD
    {
	char buf[MAXPATHLEN];

	/* Some getcwd()s automatically allocate a buffer of the given
	 * size from the heap if they are given a NULL buffer pointer.
	 * The problem is that this behaviour is not portable. */
	if (getcwd(buf, sizeof(buf) - 1)) {
	    sv_setpv(sv, buf);
	    return TRUE;
	}
	else {
	    sv_setsv(sv, &PL_sv_undef);
	    return FALSE;
	}
    }

#else

    Stat_t statbuf;
    int orig_cdev, orig_cino, cdev, cino, odev, oino, tdev, tino;
    int pathlen=0;
    Direntry_t *dp;

    SvUPGRADE(sv, SVt_PV);

    if (PerlLIO_lstat(".", &statbuf) < 0) {
	SV_CWD_RETURN_UNDEF;
    }

    orig_cdev = statbuf.st_dev;
    orig_cino = statbuf.st_ino;
    cdev = orig_cdev;
    cino = orig_cino;

    for (;;) {
	DIR *dir;
	int namelen;
	odev = cdev;
	oino = cino;

	if (PerlDir_chdir("..") < 0) {
	    SV_CWD_RETURN_UNDEF;
	}
	if (PerlLIO_stat(".", &statbuf) < 0) {
	    SV_CWD_RETURN_UNDEF;
	}

	cdev = statbuf.st_dev;
	cino = statbuf.st_ino;

	if (odev == cdev && oino == cino) {
	    break;
	}
	if (!(dir = PerlDir_open("."))) {
	    SV_CWD_RETURN_UNDEF;
	}

	while ((dp = PerlDir_read(dir)) != NULL) {
#ifdef DIRNAMLEN
	    namelen = dp->d_namlen;
#else
	    namelen = strlen(dp->d_name);
#endif
	    /* skip . and .. */
	    if (SV_CWD_ISDOT(dp)) {
		continue;
	    }

	    if (PerlLIO_lstat(dp->d_name, &statbuf) < 0) {
		SV_CWD_RETURN_UNDEF;
	    }

	    tdev = statbuf.st_dev;
	    tino = statbuf.st_ino;
	    if (tino == oino && tdev == odev) {
		break;
	    }
	}

	if (!dp) {
	    SV_CWD_RETURN_UNDEF;
	}

	if (pathlen + namelen + 1 >= MAXPATHLEN) {
	    SV_CWD_RETURN_UNDEF;
	}

	SvGROW(sv, pathlen + namelen + 1);

	if (pathlen) {
	    /* shift down */
	    Move(SvPVX_const(sv), SvPVX(sv) + namelen + 1, pathlen, char);
	}

	/* prepend current directory to the front */
	*SvPVX(sv) = '/';
	Move(dp->d_name, SvPVX(sv)+1, namelen, char);
	pathlen += (namelen + 1);

#ifdef VOID_CLOSEDIR
	PerlDir_close(dir);
#else
	if (PerlDir_close(dir) < 0) {
	    SV_CWD_RETURN_UNDEF;
	}
#endif
    }

    if (pathlen) {
	SvCUR_set(sv, pathlen);
	*SvEND(sv) = '\0';
	SvPOK_only(sv);

	if (PerlDir_chdir(SvPVX_const(sv)) < 0) {
	    SV_CWD_RETURN_UNDEF;
	}
    }
    if (PerlLIO_stat(".", &statbuf) < 0) {
	SV_CWD_RETURN_UNDEF;
    }

    cdev = statbuf.st_dev;
    cino = statbuf.st_ino;

    if (cdev != orig_cdev || cino != orig_cino) {
	Perl_croak(aTHX_ "Unstable directory path, "
		   "current directory changed unexpectedly");
    }

    return TRUE;
#endif

#else
    return FALSE;
#endif
}

#define VERSION_MAX 0x7FFFFFFF

/*
=for apidoc prescan_version

Validate that a given string can be parsed as a version object, but doesn't
actually perform the parsing.  Can use either strict or lax validation rules.
Can optionally set a number of hint variables to save the parsing code
some time when tokenizing.

=cut
*/
const char *
Perl_prescan_version(pTHX_ const char *s, bool strict,
		     const char **errstr,
		     bool *sqv, int *ssaw_decimal, int *swidth, bool *salpha) {
    bool qv = (sqv ? *sqv : FALSE);
    int width = 3;
    int saw_decimal = 0;
    bool alpha = FALSE;
    const char *d = s;

    PERL_ARGS_ASSERT_PRESCAN_VERSION;

    if (qv && isDIGIT(*d))
	goto dotted_decimal_version;

    if (*d == 'v') { /* explicit v-string */
	d++;
	if (isDIGIT(*d)) {
	    qv = TRUE;
	}
	else { /* degenerate v-string */
	    /* requires v1.2.3 */
	    BADVERSION(s,errstr,"Invalid version format (dotted-decimal versions require at least three parts)");
	}

dotted_decimal_version:
	if (strict && d[0] == '0' && isDIGIT(d[1])) {
	    /* no leading zeros allowed */
	    BADVERSION(s,errstr,"Invalid version format (no leading zeros)");
	}

	while (isDIGIT(*d)) 	/* integer part */
	    d++;

	if (*d == '.')
	{
	    saw_decimal++;
	    d++; 		/* decimal point */
	}
	else
	{
	    if (strict) {
		/* require v1.2.3 */
		BADVERSION(s,errstr,"Invalid version format (dotted-decimal versions require at least three parts)");
	    }
	    else {
		goto version_prescan_finish;
	    }
	}

	{
	    int i = 0;
	    int j = 0;
	    while (isDIGIT(*d)) {	/* just keep reading */
		i++;
		while (isDIGIT(*d)) {
		    d++; j++;
		    /* maximum 3 digits between decimal */
		    if (strict && j > 3) {
			BADVERSION(s,errstr,"Invalid version format (maximum 3 digits between decimals)");
		    }
		}
		if (*d == '_') {
		    if (strict) {
			BADVERSION(s,errstr,"Invalid version format (no underscores)");
		    }
		    if ( alpha ) {
			BADVERSION(s,errstr,"Invalid version format (multiple underscores)");
		    }
		    d++;
		    alpha = TRUE;
		}
		else if (*d == '.') {
		    if (alpha) {
			BADVERSION(s,errstr,"Invalid version format (underscores before decimal)");
		    }
		    saw_decimal++;
		    d++;
		}
		else if (!isDIGIT(*d)) {
		    break;
		}
		j = 0;
	    }

	    if (strict && i < 2) {
		/* requires v1.2.3 */
		BADVERSION(s,errstr,"Invalid version format (dotted-decimal versions require at least three parts)");
	    }
	}
    } 					/* end if dotted-decimal */
    else
    {					/* decimal versions */
	/* special strict case for leading '.' or '0' */
	if (strict) {
	    if (*d == '.') {
		BADVERSION(s,errstr,"Invalid version format (0 before decimal required)");
	    }
	    if (*d == '0' && isDIGIT(d[1])) {
		BADVERSION(s,errstr,"Invalid version format (no leading zeros)");
	    }
	}

	/* and we never support negative versions */
	if ( *d == '-') {
	    BADVERSION(s,errstr,"Invalid version format (negative version number)");
	}

	/* consume all of the integer part */
	while (isDIGIT(*d))
	    d++;

	/* look for a fractional part */
	if (*d == '.') {
	    /* we found it, so consume it */
	    saw_decimal++;
	    d++;
	}
	else if (!*d || *d == ';' || isSPACE(*d) || *d == '{' || *d == '}') {
	    if ( d == s ) {
		/* found nothing */
		BADVERSION(s,errstr,"Invalid version format (version required)");
	    }
	    /* found just an integer */
	    goto version_prescan_finish;
	}
	else if ( d == s ) {
	    /* didn't find either integer or period */
	    BADVERSION(s,errstr,"Invalid version format (non-numeric data)");
	}
	else if (*d == '_') {
	    /* underscore can't come after integer part */
	    if (strict) {
		BADVERSION(s,errstr,"Invalid version format (no underscores)");
	    }
	    else if (isDIGIT(d[1])) {
		BADVERSION(s,errstr,"Invalid version format (alpha without decimal)");
	    }
	    else {
		BADVERSION(s,errstr,"Invalid version format (misplaced underscore)");
	    }
	}
	else {
	    /* anything else after integer part is just invalid data */
	    BADVERSION(s,errstr,"Invalid version format (non-numeric data)");
	}

	/* scan the fractional part after the decimal point*/

	if (!isDIGIT(*d) && (strict || ! (!*d || *d == ';' || isSPACE(*d) || *d == '{' || *d == '}') )) {
		/* strict or lax-but-not-the-end */
		BADVERSION(s,errstr,"Invalid version format (fractional part required)");
	}

	while (isDIGIT(*d)) {
	    d++;
	    if (*d == '.' && isDIGIT(d[-1])) {
		if (alpha) {
		    BADVERSION(s,errstr,"Invalid version format (underscores before decimal)");
		}
		if (strict) {
		    BADVERSION(s,errstr,"Invalid version format (dotted-decimal versions must begin with 'v')");
		}
		d = (char *)s; 		/* start all over again */
		qv = TRUE;
		goto dotted_decimal_version;
	    }
	    if (*d == '_') {
		if (strict) {
		    BADVERSION(s,errstr,"Invalid version format (no underscores)");
		}
		if ( alpha ) {
		    BADVERSION(s,errstr,"Invalid version format (multiple underscores)");
		}
		if ( ! isDIGIT(d[1]) ) {
		    BADVERSION(s,errstr,"Invalid version format (misplaced underscore)");
		}
		d++;
		alpha = TRUE;
	    }
	}
    }

version_prescan_finish:
    while (isSPACE(*d))
	d++;

    if (!isDIGIT(*d) && (! (!*d || *d == ';' || *d == '{' || *d == '}') )) {
	/* trailing non-numeric data */
	BADVERSION(s,errstr,"Invalid version format (non-numeric data)");
    }

    if (sqv)
	*sqv = qv;
    if (swidth)
	*swidth = width;
    if (ssaw_decimal)
	*ssaw_decimal = saw_decimal;
    if (salpha)
	*salpha = alpha;
    return d;
}

/*
=for apidoc scan_version

Returns a pointer to the next character after the parsed
version string, as well as upgrading the passed in SV to
an RV.

Function must be called with an already existing SV like

    sv = newSV(0);
    s = scan_version(s, SV *sv, bool qv);

Performs some preprocessing to the string to ensure that
it has the correct characteristics of a version.  Flags the
object if it contains an underscore (which denotes this
is an alpha version).  The boolean qv denotes that the version
should be interpreted as if it had multiple decimals, even if
it doesn't.

=cut
*/

const char *
Perl_scan_version(pTHX_ const char *s, SV *rv, bool qv)
{
    const char *start;
    const char *pos;
    const char *last;
    const char *errstr = NULL;
    int saw_decimal = 0;
    int width = 3;
    bool alpha = FALSE;
    bool vinf = FALSE;
    AV * const av = newAV();
    SV * const hv = newSVrv(rv, "version"); /* create an SV and upgrade the RV */

    PERL_ARGS_ASSERT_SCAN_VERSION;

    (void)sv_upgrade(hv, SVt_PVHV); /* needs to be an HV type */

#ifndef NODEFAULT_SHAREKEYS
    HvSHAREKEYS_on(hv);         /* key-sharing on by default */
#endif

    while (isSPACE(*s)) /* leading whitespace is OK */
	s++;

    last = prescan_version(s, FALSE, &errstr, &qv, &saw_decimal, &width, &alpha);
    if (errstr) {
	/* "undef" is a special case and not an error */
	if ( ! ( *s == 'u' && strEQ(s,"undef")) ) {
	    Perl_croak(aTHX_ "%s", errstr);
	}
    }

    start = s;
    if (*s == 'v')
	s++;
    pos = s;

    if ( qv )
	(void)hv_stores(MUTABLE_HV(hv), "qv", newSViv(qv));
    if ( alpha )
	(void)hv_stores(MUTABLE_HV(hv), "alpha", newSViv(alpha));
    if ( !qv && width < 3 )
	(void)hv_stores(MUTABLE_HV(hv), "width", newSViv(width));
    
    while (isDIGIT(*pos))
	pos++;
    if (!isALPHA(*pos)) {
	I32 rev;

	for (;;) {
	    rev = 0;
	    {
  		/* this is atoi() that delimits on underscores */
  		const char *end = pos;
  		I32 mult = 1;
		I32 orev;

		/* the following if() will only be true after the decimal
		 * point of a version originally created with a bare
		 * floating point number, i.e. not quoted in any way
		 */
		if ( !qv && s > start && saw_decimal == 1 ) {
		    mult *= 100;
 		    while ( s < end ) {
			orev = rev;
 			rev += (*s - '0') * mult;
 			mult /= 10;
			if (   (PERL_ABS(orev) > PERL_ABS(rev)) 
			    || (PERL_ABS(rev) > VERSION_MAX )) {
			    Perl_ck_warner(aTHX_ packWARN(WARN_OVERFLOW), 
					   "Integer overflow in version %d",VERSION_MAX);
			    s = end - 1;
			    rev = VERSION_MAX;
			    vinf = 1;
			}
 			s++;
			if ( *s == '_' )
			    s++;
 		    }
  		}
 		else {
 		    while (--end >= s) {
			orev = rev;
 			rev += (*end - '0') * mult;
 			mult *= 10;
			if (   (PERL_ABS(orev) > PERL_ABS(rev)) 
			    || (PERL_ABS(rev) > VERSION_MAX )) {
			    Perl_ck_warner(aTHX_ packWARN(WARN_OVERFLOW), 
					   "Integer overflow in version");
			    end = s - 1;
			    rev = VERSION_MAX;
			    vinf = 1;
			}
 		    }
 		} 
  	    }

  	    /* Append revision */
	    av_push(av, newSViv(rev));
	    if ( vinf ) {
		s = last;
		break;
	    }
	    else if ( *pos == '.' )
		s = ++pos;
	    else if ( *pos == '_' && isDIGIT(pos[1]) )
		s = ++pos;
	    else if ( *pos == ',' && isDIGIT(pos[1]) )
		s = ++pos;
	    else if ( isDIGIT(*pos) )
		s = pos;
	    else {
		s = pos;
		break;
	    }
	    if ( qv ) {
		while ( isDIGIT(*pos) )
		    pos++;
	    }
	    else {
		int digits = 0;
		while ( ( isDIGIT(*pos) || *pos == '_' ) && digits < 3 ) {
		    if ( *pos != '_' )
			digits++;
		    pos++;
		}
	    }
	}
    }
    if ( qv ) { /* quoted versions always get at least three terms*/
	I32 len = av_len(av);
	/* This for loop appears to trigger a compiler bug on OS X, as it
	   loops infinitely. Yes, len is negative. No, it makes no sense.
	   Compiler in question is:
	   gcc version 3.3 20030304 (Apple Computer, Inc. build 1640)
	   for ( len = 2 - len; len > 0; len-- )
	   av_push(MUTABLE_AV(sv), newSViv(0));
	*/
	len = 2 - len;
	while (len-- > 0)
	    av_push(av, newSViv(0));
    }

    /* need to save off the current version string for later */
    if ( vinf ) {
	SV * orig = newSVpvn("v.Inf", sizeof("v.Inf")-1);
	(void)hv_stores(MUTABLE_HV(hv), "original", orig);
	(void)hv_stores(MUTABLE_HV(hv), "vinf", newSViv(1));
    }
    else if ( s > start ) {
	SV * orig = newSVpvn(start,s-start);
	if ( qv && saw_decimal == 1 && *start != 'v' ) {
	    /* need to insert a v to be consistent */
	    sv_insert(orig, 0, 0, "v", 1);
	}
	(void)hv_stores(MUTABLE_HV(hv), "original", orig);
    }
    else {
	(void)hv_stores(MUTABLE_HV(hv), "original", newSVpvs("0"));
	av_push(av, newSViv(0));
    }

    /* And finally, store the AV in the hash */
    (void)hv_stores(MUTABLE_HV(hv), "version", newRV_noinc(MUTABLE_SV(av)));

    /* fix RT#19517 - special case 'undef' as string */
    if ( *s == 'u' && strEQ(s,"undef") ) {
	s += 5;
    }

    return s;
}

/*
=for apidoc new_version

Returns a new version object based on the passed in SV:

    SV *sv = new_version(SV *ver);

Does not alter the passed in ver SV.  See "upg_version" if you
want to upgrade the SV.

=cut
*/

SV *
Perl_new_version(pTHX_ SV *ver)
{
    dVAR;
    SV * const rv = newSV(0);
    PERL_ARGS_ASSERT_NEW_VERSION;
    if ( sv_isobject(ver) && sv_derived_from(ver, "version") )
	 /* can just copy directly */
    {
	I32 key;
	AV * const av = newAV();
	AV *sav;
	/* This will get reblessed later if a derived class*/
	SV * const hv = newSVrv(rv, "version"); 
	(void)sv_upgrade(hv, SVt_PVHV); /* needs to be an HV type */
#ifndef NODEFAULT_SHAREKEYS
	HvSHAREKEYS_on(hv);         /* key-sharing on by default */
#endif

	if ( SvROK(ver) )
	    ver = SvRV(ver);

	/* Begin copying all of the elements */
	if ( hv_exists(MUTABLE_HV(ver), "qv", 2) )
	    (void)hv_stores(MUTABLE_HV(hv), "qv", newSViv(1));

	if ( hv_exists(MUTABLE_HV(ver), "alpha", 5) )
	    (void)hv_stores(MUTABLE_HV(hv), "alpha", newSViv(1));
	
	if ( hv_exists(MUTABLE_HV(ver), "width", 5 ) )
	{
	    const I32 width = SvIV(*hv_fetchs(MUTABLE_HV(ver), "width", FALSE));
	    (void)hv_stores(MUTABLE_HV(hv), "width", newSViv(width));
	}

	if ( hv_exists(MUTABLE_HV(ver), "original", 8 ) )
	{
	    SV * pv = *hv_fetchs(MUTABLE_HV(ver), "original", FALSE);
	    (void)hv_stores(MUTABLE_HV(hv), "original", newSVsv(pv));
	}

	sav = MUTABLE_AV(SvRV(*hv_fetchs(MUTABLE_HV(ver), "version", FALSE)));
	/* This will get reblessed later if a derived class*/
	for ( key = 0; key <= av_len(sav); key++ )
	{
	    const I32 rev = SvIV(*av_fetch(sav, key, FALSE));
	    av_push(av, newSViv(rev));
	}

	(void)hv_stores(MUTABLE_HV(hv), "version", newRV_noinc(MUTABLE_SV(av)));
	return rv;
    }
#ifdef SvVOK
    {
	const MAGIC* const mg = SvVSTRING_mg(ver);
	if ( mg ) { /* already a v-string */
	    const STRLEN len = mg->mg_len;
	    char * const version = savepvn( (const char*)mg->mg_ptr, len);
	    sv_setpvn(rv,version,len);
	    /* this is for consistency with the pure Perl class */
	    if ( isDIGIT(*version) )
		sv_insert(rv, 0, 0, "v", 1);
	    Safefree(version);
	}
	else {
#endif
	sv_setsv(rv,ver); /* make a duplicate */
#ifdef SvVOK
	}
    }
#endif
    return upg_version(rv, FALSE);
}

/*
=for apidoc upg_version

In-place upgrade of the supplied SV to a version object.

    SV *sv = upg_version(SV *sv, bool qv);

Returns a pointer to the upgraded SV.  Set the boolean qv if you want
to force this SV to be interpreted as an "extended" version.

=cut
*/

SV *
Perl_upg_version(pTHX_ SV *ver, bool qv)
{
    const char *version, *s;
#ifdef SvVOK
    const MAGIC *mg;
#endif

    PERL_ARGS_ASSERT_UPG_VERSION;

    if ( SvNOK(ver) && !( SvPOK(ver) && sv_len(ver) == 3 ) )
    {
	STRLEN len;

	/* may get too much accuracy */ 
	char tbuf[64];
	SV *sv = SvNVX(ver) > 10e50 ? newSV(64) : 0;
	char *buf;
#ifdef USE_LOCALE_NUMERIC
	char *loc = savepv(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
#endif
	if (sv) {
	    Perl_sv_setpvf(aTHX_ sv, "%.9"NVff, SvNVX(ver));
	    buf = SvPV(sv, len);
	}
	else {
	    len = my_snprintf(tbuf, sizeof(tbuf), "%.9"NVff, SvNVX(ver));
	    buf = tbuf;
	}
#ifdef USE_LOCALE_NUMERIC
	setlocale(LC_NUMERIC, loc);
	Safefree(loc);
#endif
	while (buf[len-1] == '0' && len > 0) len--;
	if ( buf[len-1] == '.' ) len--; /* eat the trailing decimal */
	version = savepvn(buf, len);
	SvREFCNT_dec(sv);
    }
#ifdef SvVOK
    else if ( (mg = SvVSTRING_mg(ver)) ) { /* already a v-string */
	version = savepvn( (const char*)mg->mg_ptr,mg->mg_len );
	qv = TRUE;
    }
#endif
    else /* must be a string or something like a string */
    {
	STRLEN len;
	version = savepv(SvPV(ver,len));
#ifndef SvVOK
#  if PERL_VERSION > 5
	/* This will only be executed for 5.6.0 - 5.8.0 inclusive */
	if ( len >= 3 && !instr(version,".") && !instr(version,"_")) {
	    /* may be a v-string */
	    char *testv = (char *)version;
	    STRLEN tlen = len;
	    for (tlen=0; tlen < len; tlen++, testv++) {
		/* if one of the characters is non-text assume v-string */
		if (testv[0] < ' ') {
		    SV * const nsv = sv_newmortal();
		    const char *nver;
		    const char *pos;
		    int saw_decimal = 0;
		    sv_setpvf(nsv,"v%vd",ver);
		    pos = nver = savepv(SvPV_nolen(nsv));

		    /* scan the resulting formatted string */
		    pos++; /* skip the leading 'v' */
		    while ( *pos == '.' || isDIGIT(*pos) ) {
			if ( *pos == '.' )
			    saw_decimal++ ;
			pos++;
		    }

		    /* is definitely a v-string */
		    if ( saw_decimal >= 2 ) {	
			Safefree(version);
			version = nver;
		    }
		    break;
		}
	    }
	}
#  endif
#endif
    }

    s = scan_version(version, ver, qv);
    if ( *s != '\0' ) 
	Perl_ck_warner(aTHX_ packWARN(WARN_MISC), 
		       "Version string '%s' contains invalid data; "
		       "ignoring: '%s'", version, s);
    Safefree(version);
    return ver;
}

/*
=for apidoc vverify

Validates that the SV contains valid internal structure for a version object.
It may be passed either the version object (RV) or the hash itself (HV).  If
the structure is valid, it returns the HV.  If the structure is invalid,
it returns NULL.

    SV *hv = vverify(sv);

Note that it only confirms the bare minimum structure (so as not to get
confused by derived classes which may contain additional hash entries):

=over 4

=item * The SV is an HV or a reference to an HV

=item * The hash contains a "version" key

=item * The "version" key has a reference to an AV as its value

=back

=cut
*/

SV *
Perl_vverify(pTHX_ SV *vs)
{
    SV *sv;

    PERL_ARGS_ASSERT_VVERIFY;

    if ( SvROK(vs) )
	vs = SvRV(vs);

    /* see if the appropriate elements exist */
    if ( SvTYPE(vs) == SVt_PVHV
	 && hv_exists(MUTABLE_HV(vs), "version", 7)
	 && (sv = SvRV(*hv_fetchs(MUTABLE_HV(vs), "version", FALSE)))
	 && SvTYPE(sv) == SVt_PVAV )
	return vs;
    else
	return NULL;
}

/*
=for apidoc vnumify

Accepts a version object and returns the normalized floating
point representation.  Call like:

    sv = vnumify(rv);

NOTE: you can pass either the object directly or the SV
contained within the RV.

The SV returned has a refcount of 1.

=cut
*/

SV *
Perl_vnumify(pTHX_ SV *vs)
{
    I32 i, len, digit;
    int width;
    bool alpha = FALSE;
    SV *sv;
    AV *av;

    PERL_ARGS_ASSERT_VNUMIFY;

    /* extract the HV from the object */
    vs = vverify(vs);
    if ( ! vs )
	Perl_croak(aTHX_ "Invalid version object");

    /* see if various flags exist */
    if ( hv_exists(MUTABLE_HV(vs), "alpha", 5 ) )
	alpha = TRUE;
    if ( hv_exists(MUTABLE_HV(vs), "width", 5 ) )
	width = SvIV(*hv_fetchs(MUTABLE_HV(vs), "width", FALSE));
    else
	width = 3;


    /* attempt to retrieve the version array */
    if ( !(av = MUTABLE_AV(SvRV(*hv_fetchs(MUTABLE_HV(vs), "version", FALSE))) ) ) {
	return newSVpvs("0");
    }

    len = av_len(av);
    if ( len == -1 )
    {
	return newSVpvs("0");
    }

    digit = SvIV(*av_fetch(av, 0, 0));
    sv = Perl_newSVpvf(aTHX_ "%d.", (int)PERL_ABS(digit));
    for ( i = 1 ; i < len ; i++ )
    {
	digit = SvIV(*av_fetch(av, i, 0));
	if ( width < 3 ) {
	    const int denom = (width == 2 ? 10 : 100);
	    const div_t term = div((int)PERL_ABS(digit),denom);
	    Perl_sv_catpvf(aTHX_ sv, "%0*d_%d", width, term.quot, term.rem);
	}
	else {
	    Perl_sv_catpvf(aTHX_ sv, "%0*d", width, (int)digit);
	}
    }

    if ( len > 0 )
    {
	digit = SvIV(*av_fetch(av, len, 0));
	if ( alpha && width == 3 ) /* alpha version */
	    sv_catpvs(sv,"_");
	Perl_sv_catpvf(aTHX_ sv, "%0*d", width, (int)digit);
    }
    else /* len == 0 */
    {
	sv_catpvs(sv, "000");
    }
    return sv;
}

/*
=for apidoc vnormal

Accepts a version object and returns the normalized string
representation.  Call like:

    sv = vnormal(rv);

NOTE: you can pass either the object directly or the SV
contained within the RV.

The SV returned has a refcount of 1.

=cut
*/

SV *
Perl_vnormal(pTHX_ SV *vs)
{
    I32 i, len, digit;
    bool alpha = FALSE;
    SV *sv;
    AV *av;

    PERL_ARGS_ASSERT_VNORMAL;

    /* extract the HV from the object */
    vs = vverify(vs);
    if ( ! vs )
	Perl_croak(aTHX_ "Invalid version object");

    if ( hv_exists(MUTABLE_HV(vs), "alpha", 5 ) )
	alpha = TRUE;
    av = MUTABLE_AV(SvRV(*hv_fetchs(MUTABLE_HV(vs), "version", FALSE)));

    len = av_len(av);
    if ( len == -1 )
    {
	return newSVpvs("");
    }
    digit = SvIV(*av_fetch(av, 0, 0));
    sv = Perl_newSVpvf(aTHX_ "v%"IVdf, (IV)digit);
    for ( i = 1 ; i < len ; i++ ) {
	digit = SvIV(*av_fetch(av, i, 0));
	Perl_sv_catpvf(aTHX_ sv, ".%"IVdf, (IV)digit);
    }

    if ( len > 0 )
    {
	/* handle last digit specially */
	digit = SvIV(*av_fetch(av, len, 0));
	if ( alpha )
	    Perl_sv_catpvf(aTHX_ sv, "_%"IVdf, (IV)digit);
	else
	    Perl_sv_catpvf(aTHX_ sv, ".%"IVdf, (IV)digit);
    }

    if ( len <= 2 ) { /* short version, must be at least three */
	for ( len = 2 - len; len != 0; len-- )
	    sv_catpvs(sv,".0");
    }
    return sv;
}

/*
=for apidoc vstringify

In order to maintain maximum compatibility with earlier versions
of Perl, this function will return either the floating point
notation or the multiple dotted notation, depending on whether
the original version contained 1 or more dots, respectively.

The SV returned has a refcount of 1.

=cut
*/

SV *
Perl_vstringify(pTHX_ SV *vs)
{
    PERL_ARGS_ASSERT_VSTRINGIFY;

    /* extract the HV from the object */
    vs = vverify(vs);
    if ( ! vs )
	Perl_croak(aTHX_ "Invalid version object");

    if (hv_exists(MUTABLE_HV(vs), "original",  sizeof("original") - 1)) {
	SV *pv;
	pv = *hv_fetchs(MUTABLE_HV(vs), "original", FALSE);
	if ( SvPOK(pv) )
	    return newSVsv(pv);
	else
	    return &PL_sv_undef;
    }
    else {
	if ( hv_exists(MUTABLE_HV(vs), "qv", 2) )
	    return vnormal(vs);
	else
	    return vnumify(vs);
    }
}

/*
=for apidoc vcmp

Version object aware cmp.  Both operands must already have been 
converted into version objects.

=cut
*/

int
Perl_vcmp(pTHX_ SV *lhv, SV *rhv)
{
    I32 i,l,m,r,retval;
    bool lalpha = FALSE;
    bool ralpha = FALSE;
    I32 left = 0;
    I32 right = 0;
    AV *lav, *rav;

    PERL_ARGS_ASSERT_VCMP;

    /* extract the HVs from the objects */
    lhv = vverify(lhv);
    rhv = vverify(rhv);
    if ( ! ( lhv && rhv ) )
	Perl_croak(aTHX_ "Invalid version object");

    /* get the left hand term */
    lav = MUTABLE_AV(SvRV(*hv_fetchs(MUTABLE_HV(lhv), "version", FALSE)));
    if ( hv_exists(MUTABLE_HV(lhv), "alpha", 5 ) )
	lalpha = TRUE;

    /* and the right hand term */
    rav = MUTABLE_AV(SvRV(*hv_fetchs(MUTABLE_HV(rhv), "version", FALSE)));
    if ( hv_exists(MUTABLE_HV(rhv), "alpha", 5 ) )
	ralpha = TRUE;

    l = av_len(lav);
    r = av_len(rav);
    m = l < r ? l : r;
    retval = 0;
    i = 0;
    while ( i <= m && retval == 0 )
    {
	left  = SvIV(*av_fetch(lav,i,0));
	right = SvIV(*av_fetch(rav,i,0));
	if ( left < right  )
	    retval = -1;
	if ( left > right )
	    retval = +1;
	i++;
    }

    /* tiebreaker for alpha with identical terms */
    if ( retval == 0 && l == r && left == right && ( lalpha || ralpha ) )
    {
	if ( lalpha && !ralpha )
	{
	    retval = -1;
	}
	else if ( ralpha && !lalpha)
	{
	    retval = +1;
	}
    }

    if ( l != r && retval == 0 ) /* possible match except for trailing 0's */
    {
	if ( l < r )
	{
	    while ( i <= r && retval == 0 )
	    {
		if ( SvIV(*av_fetch(rav,i,0)) != 0 )
		    retval = -1; /* not a match after all */
		i++;
	    }
	}
	else
	{
	    while ( i <= l && retval == 0 )
	    {
		if ( SvIV(*av_fetch(lav,i,0)) != 0 )
		    retval = +1; /* not a match after all */
		i++;
	    }
	}
    }
    return retval;
}

#if !defined(HAS_SOCKETPAIR) && defined(HAS_SOCKET) && defined(AF_INET) && defined(PF_INET) && defined(SOCK_DGRAM) && defined(HAS_SELECT)
#   define EMULATE_SOCKETPAIR_UDP
#endif

#ifdef EMULATE_SOCKETPAIR_UDP
static int
S_socketpair_udp (int fd[2]) {
    dTHX;
    /* Fake a datagram socketpair using UDP to localhost.  */
    int sockets[2] = {-1, -1};
    struct sockaddr_in addresses[2];
    int i;
    Sock_size_t size = sizeof(struct sockaddr_in);
    unsigned short port;
    int got;

    memset(&addresses, 0, sizeof(addresses));
    i = 1;
    do {
	sockets[i] = PerlSock_socket(AF_INET, SOCK_DGRAM, PF_INET);
	if (sockets[i] == -1)
	    goto tidy_up_and_fail;

	addresses[i].sin_family = AF_INET;
	addresses[i].sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addresses[i].sin_port = 0;	/* kernel choses port.  */
	if (PerlSock_bind(sockets[i], (struct sockaddr *) &addresses[i],
		sizeof(struct sockaddr_in)) == -1)
	    goto tidy_up_and_fail;
    } while (i--);

    /* Now have 2 UDP sockets. Find out which port each is connected to, and
       for each connect the other socket to it.  */
    i = 1;
    do {
	if (PerlSock_getsockname(sockets[i], (struct sockaddr *) &addresses[i],
		&size) == -1)
	    goto tidy_up_and_fail;
	if (size != sizeof(struct sockaddr_in))
	    goto abort_tidy_up_and_fail;
	/* !1 is 0, !0 is 1 */
	if (PerlSock_connect(sockets[!i], (struct sockaddr *) &addresses[i],
		sizeof(struct sockaddr_in)) == -1)
	    goto tidy_up_and_fail;
    } while (i--);

    /* Now we have 2 sockets connected to each other. I don't trust some other
       process not to have already sent a packet to us (by random) so send
       a packet from each to the other.  */
    i = 1;
    do {
	/* I'm going to send my own port number.  As a short.
	   (Who knows if someone somewhere has sin_port as a bitfield and needs
	   this routine. (I'm assuming crays have socketpair)) */
	port = addresses[i].sin_port;
	got = PerlLIO_write(sockets[i], &port, sizeof(port));
	if (got != sizeof(port)) {
	    if (got == -1)
		goto tidy_up_and_fail;
	    goto abort_tidy_up_and_fail;
	}
    } while (i--);

    /* Packets sent. I don't trust them to have arrived though.
       (As I understand it Solaris TCP stack is multithreaded. Non-blocking
       connect to localhost will use a second kernel thread. In 2.6 the
       first thread running the connect() returns before the second completes,
       so EINPROGRESS> In 2.7 the improved stack is faster and connect()
       returns 0. Poor programs have tripped up. One poor program's authors'
       had a 50-1 reverse stock split. Not sure how connected these were.)
       So I don't trust someone not to have an unpredictable UDP stack.
    */

    {
	struct timeval waitfor = {0, 100000}; /* You have 0.1 seconds */
	int max = sockets[1] > sockets[0] ? sockets[1] : sockets[0];
	fd_set rset;

	FD_ZERO(&rset);
	FD_SET((unsigned int)sockets[0], &rset);
	FD_SET((unsigned int)sockets[1], &rset);

	got = PerlSock_select(max + 1, &rset, NULL, NULL, &waitfor);
	if (got != 2 || !FD_ISSET(sockets[0], &rset)
		|| !FD_ISSET(sockets[1], &rset)) {
	    /* I hope this is portable and appropriate.  */
	    if (got == -1)
		goto tidy_up_and_fail;
	    goto abort_tidy_up_and_fail;
	}
    }

    /* And the paranoia department even now doesn't trust it to have arrive
       (hence MSG_DONTWAIT). Or that what arrives was sent by us.  */
    {
	struct sockaddr_in readfrom;
	unsigned short buffer[2];

	i = 1;
	do {
#ifdef MSG_DONTWAIT
	    got = PerlSock_recvfrom(sockets[i], (char *) &buffer,
		    sizeof(buffer), MSG_DONTWAIT,
		    (struct sockaddr *) &readfrom, &size);
#else
	    got = PerlSock_recvfrom(sockets[i], (char *) &buffer,
		    sizeof(buffer), 0,
		    (struct sockaddr *) &readfrom, &size);
#endif

	    if (got == -1)
		goto tidy_up_and_fail;
	    if (got != sizeof(port)
		    || size != sizeof(struct sockaddr_in)
		    /* Check other socket sent us its port.  */
		    || buffer[0] != (unsigned short) addresses[!i].sin_port
		    /* Check kernel says we got the datagram from that socket */
		    || readfrom.sin_family != addresses[!i].sin_family
		    || readfrom.sin_addr.s_addr != addresses[!i].sin_addr.s_addr
		    || readfrom.sin_port != addresses[!i].sin_port)
		goto abort_tidy_up_and_fail;
	} while (i--);
    }
    /* My caller (my_socketpair) has validated that this is non-NULL  */
    fd[0] = sockets[0];
    fd[1] = sockets[1];
    /* I hereby declare this connection open.  May God bless all who cross
       her.  */
    return 0;

  abort_tidy_up_and_fail:
    errno = ECONNABORTED;
  tidy_up_and_fail:
    {
	dSAVE_ERRNO;
	if (sockets[0] != -1)
	    PerlLIO_close(sockets[0]);
	if (sockets[1] != -1)
	    PerlLIO_close(sockets[1]);
	RESTORE_ERRNO;
	return -1;
    }
}
#endif /*  EMULATE_SOCKETPAIR_UDP */

#if !defined(HAS_SOCKETPAIR) && defined(HAS_SOCKET) && defined(AF_INET) && defined(PF_INET)
int
Perl_my_socketpair (int family, int type, int protocol, int fd[2]) {
    /* Stevens says that family must be AF_LOCAL, protocol 0.
       I'm going to enforce that, then ignore it, and use TCP (or UDP).  */
    dTHX;
    int listener = -1;
    int connector = -1;
    int acceptor = -1;
    struct sockaddr_in listen_addr;
    struct sockaddr_in connect_addr;
    Sock_size_t size;

    if (protocol
#ifdef AF_UNIX
	|| family != AF_UNIX
#endif
    ) {
	errno = EAFNOSUPPORT;
	return -1;
    }
    if (!fd) {
	errno = EINVAL;
	return -1;
    }

#ifdef EMULATE_SOCKETPAIR_UDP
    if (type == SOCK_DGRAM)
	return S_socketpair_udp(fd);
#endif

    listener = PerlSock_socket(AF_INET, type, 0);
    if (listener == -1)
	return -1;
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    listen_addr.sin_port = 0;	/* kernel choses port.  */
    if (PerlSock_bind(listener, (struct sockaddr *) &listen_addr,
	    sizeof(listen_addr)) == -1)
	goto tidy_up_and_fail;
    if (PerlSock_listen(listener, 1) == -1)
	goto tidy_up_and_fail;

    connector = PerlSock_socket(AF_INET, type, 0);
    if (connector == -1)
	goto tidy_up_and_fail;
    /* We want to find out the port number to connect to.  */
    size = sizeof(connect_addr);
    if (PerlSock_getsockname(listener, (struct sockaddr *) &connect_addr,
	    &size) == -1)
	goto tidy_up_and_fail;
    if (size != sizeof(connect_addr))
	goto abort_tidy_up_and_fail;
    if (PerlSock_connect(connector, (struct sockaddr *) &connect_addr,
	    sizeof(connect_addr)) == -1)
	goto tidy_up_and_fail;

    size = sizeof(listen_addr);
    acceptor = PerlSock_accept(listener, (struct sockaddr *) &listen_addr,
	    &size);
    if (acceptor == -1)
	goto tidy_up_and_fail;
    if (size != sizeof(listen_addr))
	goto abort_tidy_up_and_fail;
    PerlLIO_close(listener);
    /* Now check we are talking to ourself by matching port and host on the
       two sockets.  */
    if (PerlSock_getsockname(connector, (struct sockaddr *) &connect_addr,
	    &size) == -1)
	goto tidy_up_and_fail;
    if (size != sizeof(connect_addr)
	    || listen_addr.sin_family != connect_addr.sin_family
	    || listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr
	    || listen_addr.sin_port != connect_addr.sin_port) {
	goto abort_tidy_up_and_fail;
    }
    fd[0] = connector;
    fd[1] = acceptor;
    return 0;

  abort_tidy_up_and_fail:
#ifdef ECONNABORTED
  errno = ECONNABORTED;	/* This would be the standard thing to do. */
#else
#  ifdef ECONNREFUSED
  errno = ECONNREFUSED;	/* E.g. Symbian does not have ECONNABORTED. */
#  else
  errno = ETIMEDOUT;	/* Desperation time. */
#  endif
#endif
  tidy_up_and_fail:
    {
	dSAVE_ERRNO;
	if (listener != -1)
	    PerlLIO_close(listener);
	if (connector != -1)
	    PerlLIO_close(connector);
	if (acceptor != -1)
	    PerlLIO_close(acceptor);
	RESTORE_ERRNO;
	return -1;
    }
}
#else
/* In any case have a stub so that there's code corresponding
 * to the my_socketpair in embed.fnc. */
int
Perl_my_socketpair (int family, int type, int protocol, int fd[2]) {
#ifdef HAS_SOCKETPAIR
    return socketpair(family, type, protocol, fd);
#else
    return -1;
#endif
}
#endif

/*

=for apidoc sv_nosharing

Dummy routine which "shares" an SV when there is no sharing module present.
Or "locks" it. Or "unlocks" it. In other words, ignores its single SV argument.
Exists to avoid test for a NULL function pointer and because it could
potentially warn under some level of strict-ness.

=cut
*/

void
Perl_sv_nosharing(pTHX_ SV *sv)
{
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(sv);
}

/*

=for apidoc sv_destroyable

Dummy routine which reports that object can be destroyed when there is no
sharing module present.  It ignores its single SV argument, and returns
'true'.  Exists to avoid test for a NULL function pointer and because it
could potentially warn under some level of strict-ness.

=cut
*/

bool
Perl_sv_destroyable(pTHX_ SV *sv)
{
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(sv);
    return TRUE;
}

U32
Perl_parse_unicode_opts(pTHX_ const char **popt)
{
  const char *p = *popt;
  U32 opt = 0;

  PERL_ARGS_ASSERT_PARSE_UNICODE_OPTS;

  if (*p) {
       if (isDIGIT(*p)) {
	    opt = (U32) atoi(p);
	    while (isDIGIT(*p))
		p++;
	    if (*p && *p != '\n' && *p != '\r') {
	     if(isSPACE(*p)) goto the_end_of_the_opts_parser;
	     else
		 Perl_croak(aTHX_ "Unknown Unicode option letter '%c'", *p);
	    }
       }
       else {
	    for (; *p; p++) {
		 switch (*p) {
		 case PERL_UNICODE_STDIN:
		      opt |= PERL_UNICODE_STDIN_FLAG;	break;
		 case PERL_UNICODE_STDOUT:
		      opt |= PERL_UNICODE_STDOUT_FLAG;	break;
		 case PERL_UNICODE_STDERR:
		      opt |= PERL_UNICODE_STDERR_FLAG;	break;
		 case PERL_UNICODE_STD:
		      opt |= PERL_UNICODE_STD_FLAG;    	break;
		 case PERL_UNICODE_IN:
		      opt |= PERL_UNICODE_IN_FLAG;	break;
		 case PERL_UNICODE_OUT:
		      opt |= PERL_UNICODE_OUT_FLAG;	break;
		 case PERL_UNICODE_INOUT:
		      opt |= PERL_UNICODE_INOUT_FLAG;	break;
		 case PERL_UNICODE_LOCALE:
		      opt |= PERL_UNICODE_LOCALE_FLAG;	break;
		 case PERL_UNICODE_ARGV:
		      opt |= PERL_UNICODE_ARGV_FLAG;	break;
		 case PERL_UNICODE_UTF8CACHEASSERT:
		      opt |= PERL_UNICODE_UTF8CACHEASSERT_FLAG; break;
		 default:
		      if (*p != '\n' && *p != '\r') {
			if(isSPACE(*p)) goto the_end_of_the_opts_parser;
			else
			  Perl_croak(aTHX_
				     "Unknown Unicode option letter '%c'", *p);
		      }
		 }
	    }
       }
  }
  else
       opt = PERL_UNICODE_DEFAULT_FLAGS;

  the_end_of_the_opts_parser:

  if (opt & ~PERL_UNICODE_ALL_FLAGS)
       Perl_croak(aTHX_ "Unknown Unicode option value %"UVuf,
		  (UV) (opt & ~PERL_UNICODE_ALL_FLAGS));

  *popt = p;

  return opt;
}

U32
Perl_seed(pTHX)
{
    dVAR;
    /*
     * This is really just a quick hack which grabs various garbage
     * values.  It really should be a real hash algorithm which
     * spreads the effect of every input bit onto every output bit,
     * if someone who knows about such things would bother to write it.
     * Might be a good idea to add that function to CORE as well.
     * No numbers below come from careful analysis or anything here,
     * except they are primes and SEED_C1 > 1E6 to get a full-width
     * value from (tv_sec * SEED_C1 + tv_usec).  The multipliers should
     * probably be bigger too.
     */
#if RANDBITS > 16
#  define SEED_C1	1000003
#define   SEED_C4	73819
#else
#  define SEED_C1	25747
#define   SEED_C4	20639
#endif
#define   SEED_C2	3
#define   SEED_C3	269
#define   SEED_C5	26107

#ifndef PERL_NO_DEV_RANDOM
    int fd;
#endif
    U32 u;
#ifdef VMS
#  include <starlet.h>
    /* when[] = (low 32 bits, high 32 bits) of time since epoch
     * in 100-ns units, typically incremented ever 10 ms.        */
    unsigned int when[2];
#else
#  ifdef HAS_GETTIMEOFDAY
    struct timeval when;
#  else
    Time_t when;
#  endif
#endif

/* This test is an escape hatch, this symbol isn't set by Configure. */
#ifndef PERL_NO_DEV_RANDOM
#ifndef PERL_RANDOM_DEVICE
   /* /dev/random isn't used by default because reads from it will block
    * if there isn't enough entropy available.  You can compile with
    * PERL_RANDOM_DEVICE to it if you'd prefer Perl to block until there
    * is enough real entropy to fill the seed. */
#  define PERL_RANDOM_DEVICE "/dev/urandom"
#endif
    fd = PerlLIO_open(PERL_RANDOM_DEVICE, 0);
    if (fd != -1) {
    	if (PerlLIO_read(fd, (void*)&u, sizeof u) != sizeof u)
	    u = 0;
	PerlLIO_close(fd);
	if (u)
	    return u;
    }
#endif

#ifdef VMS
    _ckvmssts(sys$gettim(when));
    u = (U32)SEED_C1 * when[0] + (U32)SEED_C2 * when[1];
#else
#  ifdef HAS_GETTIMEOFDAY
    PerlProc_gettimeofday(&when,NULL);
    u = (U32)SEED_C1 * when.tv_sec + (U32)SEED_C2 * when.tv_usec;
#  else
    (void)time(&when);
    u = (U32)SEED_C1 * when;
#  endif
#endif
    u += SEED_C3 * (U32)PerlProc_getpid();
    u += SEED_C4 * (U32)PTR2UV(PL_stack_sp);
#ifndef PLAN9           /* XXX Plan9 assembler chokes on this; fix needed  */
    u += SEED_C5 * (U32)PTR2UV(&when);
#endif
    return u;
}

UV
Perl_get_hash_seed(pTHX)
{
    dVAR;
     const char *s = PerlEnv_getenv("PERL_HASH_SEED");
     UV myseed = 0;

     if (s)
	while (isSPACE(*s))
	    s++;
     if (s && isDIGIT(*s))
	  myseed = (UV)Atoul(s);
     else
#ifdef USE_HASH_SEED_EXPLICIT
     if (s)
#endif
     {
	  /* Compute a random seed */
	  (void)seedDrand01((Rand_seed_t)seed());
	  myseed = (UV)(Drand01() * (NV)UV_MAX);
#if RANDBITS < (UVSIZE * 8)
	  /* Since there are not enough randbits to to reach all
	   * the bits of a UV, the low bits might need extra
	   * help.  Sum in another random number that will
	   * fill in the low bits. */
	  myseed +=
	       (UV)(Drand01() * (NV)((((UV)1) << ((UVSIZE * 8 - RANDBITS))) - 1));
#endif /* RANDBITS < (UVSIZE * 8) */
	  if (myseed == 0) { /* Superparanoia. */
	      myseed = (UV)(Drand01() * (NV)UV_MAX); /* One more chance. */
	      if (myseed == 0)
		  Perl_croak(aTHX_ "Your random numbers are not that random");
	  }
     }
     PL_rehash_seed_set = TRUE;

     return myseed;
}

#ifdef USE_ITHREADS
bool
Perl_stashpv_hvname_match(pTHX_ const COP *c, const HV *hv)
{
    const char * stashpv = CopSTASHPV(c);
    const char * name    = HvNAME_get(hv);
    const bool utf8 = CopSTASH_len(c) < 0;
    const I32  len  = utf8 ? -CopSTASH_len(c) : CopSTASH_len(c);
    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_STASHPV_HVNAME_MATCH;

    if (!stashpv || !name)
	return stashpv == name;
    if ( !HvNAMEUTF8(hv) != !utf8 ) {
        if (utf8) {
            return (bytes_cmp_utf8(
                        (const U8*)stashpv, len,
                        (const U8*)name, HEK_LEN(HvNAME_HEK(hv))) == 0);
        } else {
            return (bytes_cmp_utf8(
                        (const U8*)name, HEK_LEN(HvNAME_HEK(hv)),
                        (const U8*)stashpv, len) == 0);
        }
    }
    else
        return (stashpv == name
                    || (HEK_LEN(HvNAME_HEK(hv)) == len
			 && memEQ(stashpv, name, len)));
    /*NOTREACHED*/
    return FALSE;
}
#endif


#ifdef PERL_GLOBAL_STRUCT

#define PERL_GLOBAL_STRUCT_INIT
#include "opcode.h" /* the ppaddr and check */

struct perl_vars *
Perl_init_global_struct(pTHX)
{
    struct perl_vars *plvarsp = NULL;
# ifdef PERL_GLOBAL_STRUCT
    const IV nppaddr = sizeof(Gppaddr)/sizeof(Perl_ppaddr_t);
    const IV ncheck  = sizeof(Gcheck) /sizeof(Perl_check_t);
#  ifdef PERL_GLOBAL_STRUCT_PRIVATE
    /* PerlMem_malloc() because can't use even safesysmalloc() this early. */
    plvarsp = (struct perl_vars*)PerlMem_malloc(sizeof(struct perl_vars));
    if (!plvarsp)
        exit(1);
#  else
    plvarsp = PL_VarsPtr;
#  endif /* PERL_GLOBAL_STRUCT_PRIVATE */
#  undef PERLVAR
#  undef PERLVARA
#  undef PERLVARI
#  undef PERLVARIC
#  define PERLVAR(prefix,var,type) /**/
#  define PERLVARA(prefix,var,n,type) /**/
#  define PERLVARI(prefix,var,type,init) plvarsp->prefix##var = init;
#  define PERLVARIC(prefix,var,type,init) plvarsp->prefix##var = init;
#  include "perlvars.h"
#  undef PERLVAR
#  undef PERLVARA
#  undef PERLVARI
#  undef PERLVARIC
#  ifdef PERL_GLOBAL_STRUCT
    plvarsp->Gppaddr =
	(Perl_ppaddr_t*)
	PerlMem_malloc(nppaddr * sizeof(Perl_ppaddr_t));
    if (!plvarsp->Gppaddr)
        exit(1);
    plvarsp->Gcheck  =
	(Perl_check_t*)
	PerlMem_malloc(ncheck  * sizeof(Perl_check_t));
    if (!plvarsp->Gcheck)
        exit(1);
    Copy(Gppaddr, plvarsp->Gppaddr, nppaddr, Perl_ppaddr_t); 
    Copy(Gcheck,  plvarsp->Gcheck,  ncheck,  Perl_check_t); 
#  endif
#  ifdef PERL_SET_VARS
    PERL_SET_VARS(plvarsp);
#  endif
# undef PERL_GLOBAL_STRUCT_INIT
# endif
    return plvarsp;
}

#endif /* PERL_GLOBAL_STRUCT */

#ifdef PERL_GLOBAL_STRUCT

void
Perl_free_global_struct(pTHX_ struct perl_vars *plvarsp)
{
    PERL_ARGS_ASSERT_FREE_GLOBAL_STRUCT;
# ifdef PERL_GLOBAL_STRUCT
#  ifdef PERL_UNSET_VARS
    PERL_UNSET_VARS(plvarsp);
#  endif
    free(plvarsp->Gppaddr);
    free(plvarsp->Gcheck);
#  ifdef PERL_GLOBAL_STRUCT_PRIVATE
    free(plvarsp);
#  endif
# endif
}

#endif /* PERL_GLOBAL_STRUCT */

#ifdef PERL_MEM_LOG

/* -DPERL_MEM_LOG: the Perl_mem_log_..() is compiled, including the
 * the default implementation, unless -DPERL_MEM_LOG_NOIMPL is also
 * given, and you supply your own implementation.
 *
 * The default implementation reads a single env var, PERL_MEM_LOG,
 * expecting one or more of the following:
 *
 *    \d+ - fd		fd to write to		: must be 1st (atoi)
 *    'm' - memlog	was PERL_MEM_LOG=1
 *    's' - svlog	was PERL_SV_LOG=1
 *    't' - timestamp	was PERL_MEM_LOG_TIMESTAMP=1
 *
 * This makes the logger controllable enough that it can reasonably be
 * added to the system perl.
 */

/* -DPERL_MEM_LOG_SPRINTF_BUF_SIZE=X: size of a (stack-allocated) buffer
 * the Perl_mem_log_...() will use (either via sprintf or snprintf).
 */
#define PERL_MEM_LOG_SPRINTF_BUF_SIZE 128

/* -DPERL_MEM_LOG_FD=N: the file descriptor the Perl_mem_log_...()
 * writes to.  In the default logger, this is settable at runtime.
 */
#ifndef PERL_MEM_LOG_FD
#  define PERL_MEM_LOG_FD 2 /* If STDERR is too boring for you. */
#endif

#ifndef PERL_MEM_LOG_NOIMPL

# ifdef DEBUG_LEAKING_SCALARS
#   define SV_LOG_SERIAL_FMT	    " [%lu]"
#   define _SV_LOG_SERIAL_ARG(sv)   , (unsigned long) (sv)->sv_debug_serial
# else
#   define SV_LOG_SERIAL_FMT
#   define _SV_LOG_SERIAL_ARG(sv)
# endif

static void
S_mem_log_common(enum mem_log_type mlt, const UV n, 
		 const UV typesize, const char *type_name, const SV *sv,
		 Malloc_t oldalloc, Malloc_t newalloc,
		 const char *filename, const int linenumber,
		 const char *funcname)
{
    const char *pmlenv;

    PERL_ARGS_ASSERT_MEM_LOG_COMMON;

    pmlenv = PerlEnv_getenv("PERL_MEM_LOG");
    if (!pmlenv)
	return;
    if (mlt < MLT_NEW_SV ? strchr(pmlenv,'m') : strchr(pmlenv,'s'))
    {
	/* We can't use SVs or PerlIO for obvious reasons,
	 * so we'll use stdio and low-level IO instead. */
	char buf[PERL_MEM_LOG_SPRINTF_BUF_SIZE];

#   ifdef HAS_GETTIMEOFDAY
#     define MEM_LOG_TIME_FMT	"%10d.%06d: "
#     define MEM_LOG_TIME_ARG	(int)tv.tv_sec, (int)tv.tv_usec
	struct timeval tv;
	gettimeofday(&tv, 0);
#   else
#     define MEM_LOG_TIME_FMT	"%10d: "
#     define MEM_LOG_TIME_ARG	(int)when
        Time_t when;
        (void)time(&when);
#   endif
	/* If there are other OS specific ways of hires time than
	 * gettimeofday() (see ext/Time-HiRes), the easiest way is
	 * probably that they would be used to fill in the struct
	 * timeval. */
	{
	    STRLEN len;
	    int fd = atoi(pmlenv);
	    if (!fd)
		fd = PERL_MEM_LOG_FD;

	    if (strchr(pmlenv, 't')) {
		len = my_snprintf(buf, sizeof(buf),
				MEM_LOG_TIME_FMT, MEM_LOG_TIME_ARG);
		PerlLIO_write(fd, buf, len);
	    }
	    switch (mlt) {
	    case MLT_ALLOC:
		len = my_snprintf(buf, sizeof(buf),
			"alloc: %s:%d:%s: %"IVdf" %"UVuf
			" %s = %"IVdf": %"UVxf"\n",
			filename, linenumber, funcname, n, typesize,
			type_name, n * typesize, PTR2UV(newalloc));
		break;
	    case MLT_REALLOC:
		len = my_snprintf(buf, sizeof(buf),
			"realloc: %s:%d:%s: %"IVdf" %"UVuf
			" %s = %"IVdf": %"UVxf" -> %"UVxf"\n",
			filename, linenumber, funcname, n, typesize,
			type_name, n * typesize, PTR2UV(oldalloc),
			PTR2UV(newalloc));
		break;
	    case MLT_FREE:
		len = my_snprintf(buf, sizeof(buf),
			"free: %s:%d:%s: %"UVxf"\n",
			filename, linenumber, funcname,
			PTR2UV(oldalloc));
		break;
	    case MLT_NEW_SV:
	    case MLT_DEL_SV:
		len = my_snprintf(buf, sizeof(buf),
			"%s_SV: %s:%d:%s: %"UVxf SV_LOG_SERIAL_FMT "\n",
			mlt == MLT_NEW_SV ? "new" : "del",
			filename, linenumber, funcname,
			PTR2UV(sv) _SV_LOG_SERIAL_ARG(sv));
		break;
	    default:
		len = 0;
	    }
	    PerlLIO_write(fd, buf, len);
	}
    }
}
#endif /* !PERL_MEM_LOG_NOIMPL */

#ifndef PERL_MEM_LOG_NOIMPL
# define \
    mem_log_common_if(alty, num, tysz, tynm, sv, oal, nal, flnm, ln, fnnm) \
    mem_log_common   (alty, num, tysz, tynm, sv, oal, nal, flnm, ln, fnnm)
#else
/* this is suboptimal, but bug compatible.  User is providing their
   own implementation, but is getting these functions anyway, and they
   do nothing. But _NOIMPL users should be able to cope or fix */
# define \
    mem_log_common_if(alty, num, tysz, tynm, u, oal, nal, flnm, ln, fnnm) \
    /* mem_log_common_if_PERL_MEM_LOG_NOIMPL */
#endif

Malloc_t
Perl_mem_log_alloc(const UV n, const UV typesize, const char *type_name,
		   Malloc_t newalloc, 
		   const char *filename, const int linenumber,
		   const char *funcname)
{
    mem_log_common_if(MLT_ALLOC, n, typesize, type_name,
		      NULL, NULL, newalloc,
		      filename, linenumber, funcname);
    return newalloc;
}

Malloc_t
Perl_mem_log_realloc(const UV n, const UV typesize, const char *type_name,
		     Malloc_t oldalloc, Malloc_t newalloc, 
		     const char *filename, const int linenumber, 
		     const char *funcname)
{
    mem_log_common_if(MLT_REALLOC, n, typesize, type_name,
		      NULL, oldalloc, newalloc, 
		      filename, linenumber, funcname);
    return newalloc;
}

Malloc_t
Perl_mem_log_free(Malloc_t oldalloc, 
		  const char *filename, const int linenumber, 
		  const char *funcname)
{
    mem_log_common_if(MLT_FREE, 0, 0, "", NULL, oldalloc, NULL, 
		      filename, linenumber, funcname);
    return oldalloc;
}

void
Perl_mem_log_new_sv(const SV *sv, 
		    const char *filename, const int linenumber,
		    const char *funcname)
{
    mem_log_common_if(MLT_NEW_SV, 0, 0, "", sv, NULL, NULL,
		      filename, linenumber, funcname);
}

void
Perl_mem_log_del_sv(const SV *sv,
		    const char *filename, const int linenumber, 
		    const char *funcname)
{
    mem_log_common_if(MLT_DEL_SV, 0, 0, "", sv, NULL, NULL, 
		      filename, linenumber, funcname);
}

#endif /* PERL_MEM_LOG */

/*
=for apidoc my_sprintf

The C library C<sprintf>, wrapped if necessary, to ensure that it will return
the length of the string written to the buffer. Only rare pre-ANSI systems
need the wrapper function - usually this is a direct call to C<sprintf>.

=cut
*/
#ifndef SPRINTF_RETURNS_STRLEN
int
Perl_my_sprintf(char *buffer, const char* pat, ...)
{
    va_list args;
    PERL_ARGS_ASSERT_MY_SPRINTF;
    va_start(args, pat);
    vsprintf(buffer, pat, args);
    va_end(args);
    return strlen(buffer);
}
#endif

/*
=for apidoc my_snprintf

The C library C<snprintf> functionality, if available and
standards-compliant (uses C<vsnprintf>, actually).  However, if the
C<vsnprintf> is not available, will unfortunately use the unsafe
C<vsprintf> which can overrun the buffer (there is an overrun check,
but that may be too late).  Consider using C<sv_vcatpvf> instead, or
getting C<vsnprintf>.

=cut
*/
int
Perl_my_snprintf(char *buffer, const Size_t len, const char *format, ...)
{
    dTHX;
    int retval;
    va_list ap;
    PERL_ARGS_ASSERT_MY_SNPRINTF;
    va_start(ap, format);
#ifdef HAS_VSNPRINTF
    retval = vsnprintf(buffer, len, format, ap);
#else
    retval = vsprintf(buffer, format, ap);
#endif
    va_end(ap);
    /* vsprintf() shows failure with < 0 */
    if (retval < 0
#ifdef HAS_VSNPRINTF
    /* vsnprintf() shows failure with >= len */
        ||
        (len > 0 && (Size_t)retval >= len) 
#endif
    )
	Perl_croak(aTHX_ "panic: my_snprintf buffer overflow");
    return retval;
}

/*
=for apidoc my_vsnprintf

The C library C<vsnprintf> if available and standards-compliant.
However, if if the C<vsnprintf> is not available, will unfortunately
use the unsafe C<vsprintf> which can overrun the buffer (there is an
overrun check, but that may be too late).  Consider using
C<sv_vcatpvf> instead, or getting C<vsnprintf>.

=cut
*/
int
Perl_my_vsnprintf(char *buffer, const Size_t len, const char *format, va_list ap)
{
    dTHX;
    int retval;
#ifdef NEED_VA_COPY
    va_list apc;

    PERL_ARGS_ASSERT_MY_VSNPRINTF;

    Perl_va_copy(ap, apc);
# ifdef HAS_VSNPRINTF
    retval = vsnprintf(buffer, len, format, apc);
# else
    retval = vsprintf(buffer, format, apc);
# endif
#else
# ifdef HAS_VSNPRINTF
    retval = vsnprintf(buffer, len, format, ap);
# else
    retval = vsprintf(buffer, format, ap);
# endif
#endif /* #ifdef NEED_VA_COPY */
    /* vsprintf() shows failure with < 0 */
    if (retval < 0
#ifdef HAS_VSNPRINTF
    /* vsnprintf() shows failure with >= len */
        ||
        (len > 0 && (Size_t)retval >= len) 
#endif
    )
	Perl_croak(aTHX_ "panic: my_vsnprintf buffer overflow");
    return retval;
}

void
Perl_my_clearenv(pTHX)
{
    dVAR;
#if ! defined(PERL_MICRO)
#  if defined(PERL_IMPLICIT_SYS) || defined(WIN32)
    PerlEnv_clearenv();
#  else /* ! (PERL_IMPLICIT_SYS || WIN32) */
#    if defined(USE_ENVIRON_ARRAY)
#      if defined(USE_ITHREADS)
    /* only the parent thread can clobber the process environment */
    if (PL_curinterp == aTHX)
#      endif /* USE_ITHREADS */
    {
#      if ! defined(PERL_USE_SAFE_PUTENV)
    if ( !PL_use_safe_putenv) {
      I32 i;
      if (environ == PL_origenviron)
        environ = (char**)safesysmalloc(sizeof(char*));
      else
        for (i = 0; environ[i]; i++)
          (void)safesysfree(environ[i]);
    }
    environ[0] = NULL;
#      else /* PERL_USE_SAFE_PUTENV */
#        if defined(HAS_CLEARENV)
    (void)clearenv();
#        elif defined(HAS_UNSETENV)
    int bsiz = 80; /* Most envvar names will be shorter than this. */
    int bufsiz = bsiz * sizeof(char); /* sizeof(char) paranoid? */
    char *buf = (char*)safesysmalloc(bufsiz);
    while (*environ != NULL) {
      char *e = strchr(*environ, '=');
      int l = e ? e - *environ : (int)strlen(*environ);
      if (bsiz < l + 1) {
        (void)safesysfree(buf);
        bsiz = l + 1; /* + 1 for the \0. */
        buf = (char*)safesysmalloc(bufsiz);
      } 
      memcpy(buf, *environ, l);
      buf[l] = '\0';
      (void)unsetenv(buf);
    }
    (void)safesysfree(buf);
#        else /* ! HAS_CLEARENV && ! HAS_UNSETENV */
    /* Just null environ and accept the leakage. */
    *environ = NULL;
#        endif /* HAS_CLEARENV || HAS_UNSETENV */
#      endif /* ! PERL_USE_SAFE_PUTENV */
    }
#    endif /* USE_ENVIRON_ARRAY */
#  endif /* PERL_IMPLICIT_SYS || WIN32 */
#endif /* PERL_MICRO */
}

#ifdef PERL_IMPLICIT_CONTEXT

/* Implements the MY_CXT_INIT macro. The first time a module is loaded,
the global PL_my_cxt_index is incremented, and that value is assigned to
that module's static my_cxt_index (who's address is passed as an arg).
Then, for each interpreter this function is called for, it makes sure a
void* slot is available to hang the static data off, by allocating or
extending the interpreter's PL_my_cxt_list array */

#ifndef PERL_GLOBAL_STRUCT_PRIVATE
void *
Perl_my_cxt_init(pTHX_ int *index, size_t size)
{
    dVAR;
    void *p;
    PERL_ARGS_ASSERT_MY_CXT_INIT;
    if (*index == -1) {
	/* this module hasn't been allocated an index yet */
#if defined(USE_ITHREADS)
	MUTEX_LOCK(&PL_my_ctx_mutex);
#endif
	*index = PL_my_cxt_index++;
#if defined(USE_ITHREADS)
	MUTEX_UNLOCK(&PL_my_ctx_mutex);
#endif
    }
    
    /* make sure the array is big enough */
    if (PL_my_cxt_size <= *index) {
	if (PL_my_cxt_size) {
	    while (PL_my_cxt_size <= *index)
		PL_my_cxt_size *= 2;
	    Renew(PL_my_cxt_list, PL_my_cxt_size, void *);
	}
	else {
	    PL_my_cxt_size = 16;
	    Newx(PL_my_cxt_list, PL_my_cxt_size, void *);
	}
    }
    /* newSV() allocates one more than needed */
    p = (void*)SvPVX(newSV(size-1));
    PL_my_cxt_list[*index] = p;
    Zero(p, size, char);
    return p;
}

#else /* #ifndef PERL_GLOBAL_STRUCT_PRIVATE */

int
Perl_my_cxt_index(pTHX_ const char *my_cxt_key)
{
    dVAR;
    int index;

    PERL_ARGS_ASSERT_MY_CXT_INDEX;

    for (index = 0; index < PL_my_cxt_index; index++) {
	const char *key = PL_my_cxt_keys[index];
	/* try direct pointer compare first - there are chances to success,
	 * and it's much faster.
	 */
	if ((key == my_cxt_key) || strEQ(key, my_cxt_key))
	    return index;
    }
    return -1;
}

void *
Perl_my_cxt_init(pTHX_ const char *my_cxt_key, size_t size)
{
    dVAR;
    void *p;
    int index;

    PERL_ARGS_ASSERT_MY_CXT_INIT;

    index = Perl_my_cxt_index(aTHX_ my_cxt_key);
    if (index == -1) {
	/* this module hasn't been allocated an index yet */
#if defined(USE_ITHREADS)
	MUTEX_LOCK(&PL_my_ctx_mutex);
#endif
	index = PL_my_cxt_index++;
#if defined(USE_ITHREADS)
	MUTEX_UNLOCK(&PL_my_ctx_mutex);
#endif
    }

    /* make sure the array is big enough */
    if (PL_my_cxt_size <= index) {
	int old_size = PL_my_cxt_size;
	int i;
	if (PL_my_cxt_size) {
	    while (PL_my_cxt_size <= index)
		PL_my_cxt_size *= 2;
	    Renew(PL_my_cxt_list, PL_my_cxt_size, void *);
	    Renew(PL_my_cxt_keys, PL_my_cxt_size, const char *);
	}
	else {
	    PL_my_cxt_size = 16;
	    Newx(PL_my_cxt_list, PL_my_cxt_size, void *);
	    Newx(PL_my_cxt_keys, PL_my_cxt_size, const char *);
	}
	for (i = old_size; i < PL_my_cxt_size; i++) {
	    PL_my_cxt_keys[i] = 0;
	    PL_my_cxt_list[i] = 0;
	}
    }
    PL_my_cxt_keys[index] = my_cxt_key;
    /* newSV() allocates one more than needed */
    p = (void*)SvPVX(newSV(size-1));
    PL_my_cxt_list[index] = p;
    Zero(p, size, char);
    return p;
}
#endif /* #ifndef PERL_GLOBAL_STRUCT_PRIVATE */
#endif /* PERL_IMPLICIT_CONTEXT */

void
Perl_xs_version_bootcheck(pTHX_ U32 items, U32 ax, const char *xs_p,
			  STRLEN xs_len)
{
    SV *sv;
    const char *vn = NULL;
    SV *const module = PL_stack_base[ax];

    PERL_ARGS_ASSERT_XS_VERSION_BOOTCHECK;

    if (items >= 2)	 /* version supplied as bootstrap arg */
	sv = PL_stack_base[ax + 1];
    else {
	/* XXX GV_ADDWARN */
	vn = "XS_VERSION";
	sv = get_sv(Perl_form(aTHX_ "%"SVf"::%s", module, vn), 0);
	if (!sv || !SvOK(sv)) {
	    vn = "VERSION";
	    sv = get_sv(Perl_form(aTHX_ "%"SVf"::%s", module, vn), 0);
	}
    }
    if (sv) {
	SV *xssv = Perl_newSVpvn_flags(aTHX_ xs_p, xs_len, SVs_TEMP);
	SV *pmsv = sv_isobject(sv) && sv_derived_from(sv, "version")
	    ? sv : sv_2mortal(new_version(sv));
	xssv = upg_version(xssv, 0);
	if ( vcmp(pmsv,xssv) ) {
	    SV *string = vstringify(xssv);
	    SV *xpt = Perl_newSVpvf(aTHX_ "%"SVf" object version %"SVf
				    " does not match ", module, string);

	    SvREFCNT_dec(string);
	    string = vstringify(pmsv);

	    if (vn) {
		Perl_sv_catpvf(aTHX_ xpt, "$%"SVf"::%s %"SVf, module, vn,
			       string);
	    } else {
		Perl_sv_catpvf(aTHX_ xpt, "bootstrap parameter %"SVf, string);
	    }
	    SvREFCNT_dec(string);

	    Perl_sv_2mortal(aTHX_ xpt);
	    Perl_croak_sv(aTHX_ xpt);
	}
    }
}

void
Perl_xs_apiversion_bootcheck(pTHX_ SV *module, const char *api_p,
			     STRLEN api_len)
{
    SV *xpt = NULL;
    SV *compver = Perl_newSVpvn_flags(aTHX_ api_p, api_len, SVs_TEMP);
    SV *runver;

    PERL_ARGS_ASSERT_XS_APIVERSION_BOOTCHECK;

    /* This might croak  */
    compver = upg_version(compver, 0);
    /* This should never croak */
    runver = new_version(PL_apiversion);
    if (vcmp(compver, runver)) {
	SV *compver_string = vstringify(compver);
	SV *runver_string = vstringify(runver);
	xpt = Perl_newSVpvf(aTHX_ "Perl API version %"SVf
			    " of %"SVf" does not match %"SVf,
			    compver_string, module, runver_string);
	Perl_sv_2mortal(aTHX_ xpt);

	SvREFCNT_dec(compver_string);
	SvREFCNT_dec(runver_string);
    }
    SvREFCNT_dec(runver);
    if (xpt)
	Perl_croak_sv(aTHX_ xpt);
}

#ifndef HAS_STRLCAT
Size_t
Perl_my_strlcat(char *dst, const char *src, Size_t size)
{
    Size_t used, length, copy;

    used = strlen(dst);
    length = strlen(src);
    if (size > 0 && used < size - 1) {
        copy = (length >= size - used) ? size - used - 1 : length;
        memcpy(dst + used, src, copy);
        dst[used + copy] = '\0';
    }
    return used + length;
}
#endif

#ifndef HAS_STRLCPY
Size_t
Perl_my_strlcpy(char *dst, const char *src, Size_t size)
{
    Size_t length, copy;

    length = strlen(src);
    if (size > 0) {
        copy = (length >= size) ? size - 1 : length;
        memcpy(dst, src, copy);
        dst[copy] = '\0';
    }
    return length;
}
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1300) && (_MSC_VER < 1400) && (WINVER < 0x0500)
/* VC7 or 7.1, building with pre-VC7 runtime libraries. */
long _ftol( double ); /* Defined by VC6 C libs. */
long _ftol2( double dblSource ) { return _ftol( dblSource ); }
#endif

PERL_STATIC_INLINE bool
S_gv_has_usable_name(pTHX_ GV *gv)
{
    GV **gvp;
    return GvSTASH(gv)
	&& HvENAME(GvSTASH(gv))
	&& (gvp = (GV **)hv_fetch(
			GvSTASH(gv), GvNAME(gv),
			GvNAMEUTF8(gv) ? -GvNAMELEN(gv) : GvNAMELEN(gv), 0
	   ))
	&& *gvp == gv;
}

void
Perl_get_db_sub(pTHX_ SV **svp, CV *cv)
{
    dVAR;
    SV * const dbsv = GvSVn(PL_DBsub);
    const bool save_taint = PL_tainted;

    /* When we are called from pp_goto (svp is null),
     * we do not care about using dbsv to call CV;
     * it's for informational purposes only.
     */

    PERL_ARGS_ASSERT_GET_DB_SUB;

    PL_tainted = FALSE;
    save_item(dbsv);
    if (!PERLDB_SUB_NN) {
	GV *gv = CvGV(cv);

	if (!svp) {
	    gv_efullname3(dbsv, gv, NULL);
	}
	else if ( (CvFLAGS(cv) & (CVf_ANON | CVf_CLONED))
	     || strEQ(GvNAME(gv), "END")
	     || ( /* Could be imported, and old sub redefined. */
		 (GvCV(gv) != cv || !S_gv_has_usable_name(aTHX_ gv))
		 &&
		 !( (SvTYPE(*svp) == SVt_PVGV)
		    && (GvCV((const GV *)*svp) == cv)
		    /* Use GV from the stack as a fallback. */
		    && S_gv_has_usable_name(aTHX_ gv = (GV *)*svp) 
		  )
		)
	) {
	    /* GV is potentially non-unique, or contain different CV. */
	    SV * const tmp = newRV(MUTABLE_SV(cv));
	    sv_setsv(dbsv, tmp);
	    SvREFCNT_dec(tmp);
	}
	else {
	    sv_sethek(dbsv, HvENAME_HEK(GvSTASH(gv)));
	    sv_catpvs(dbsv, "::");
	    sv_catpvn_flags(
	      dbsv, GvNAME(gv), GvNAMELEN(gv),
	      GvNAMEUTF8(gv) ? SV_CATUTF8 : SV_CATBYTES
	    );
	}
    }
    else {
	const int type = SvTYPE(dbsv);
	if (type < SVt_PVIV && type != SVt_IV)
	    sv_upgrade(dbsv, SVt_PVIV);
	(void)SvIOK_on(dbsv);
	SvIV_set(dbsv, PTR2IV(cv));	/* Do it the quickest way  */
    }
    TAINT_IF(save_taint);
}

int
Perl_my_dirfd(pTHX_ DIR * dir) {

    /* Most dirfd implementations have problems when passed NULL. */
    if(!dir)
        return -1;
#ifdef HAS_DIRFD
    return dirfd(dir);
#elif defined(HAS_DIR_DD_FD)
    return dir->dd_fd;
#else
    Perl_die(aTHX_ PL_no_func, "dirfd");
   /* NOT REACHED */
    return 0;
#endif 
}

REGEXP *
Perl_get_re_arg(pTHX_ SV *sv) {

    if (sv) {
        if (SvMAGICAL(sv))
            mg_get(sv);
        if (SvROK(sv))
	    sv = MUTABLE_SV(SvRV(sv));
        if (SvTYPE(sv) == SVt_REGEXP)
            return (REGEXP*) sv;
    }
 
    return NULL;
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
