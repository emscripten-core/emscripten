/*    sv.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 by Larry Wall
 *    and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'I wonder what the Entish is for "yes" and "no",' he thought.
 *                                                      --Pippin
 *
 *     [p.480 of _The Lord of the Rings_, III/iv: "Treebeard"]
 */

/*
 *
 *
 * This file contains the code that creates, manipulates and destroys
 * scalar values (SVs). The other types (AV, HV, GV, etc.) reuse the
 * structure of an SV, so their creation and destruction is handled
 * here; higher-level functions are in av.c, hv.c, and so on. Opcode
 * level functions (eg. substr, split, join) for each of the types are
 * in the pp*.c files.
 */

#include "EXTERN.h"
#define PERL_IN_SV_C
#include "perl.h"
#include "regcomp.h"

#ifndef HAS_C99
# if __STDC_VERSION__ >= 199901L && !defined(VMS)
#  define HAS_C99 1
# endif
#endif
#if HAS_C99
# include <stdint.h>
#endif

#define FCALL *f

#ifdef __Lynx__
/* Missing proto on LynxOS */
  char *gconvert(double, int, int,  char *);
#endif

#ifdef PERL_UTF8_CACHE_ASSERT
/* if adding more checks watch out for the following tests:
 *   t/op/index.t t/op/length.t t/op/pat.t t/op/substr.t
 *   lib/utf8.t lib/Unicode/Collate/t/index.t
 * --jhi
 */
#   define ASSERT_UTF8_CACHE(cache) \
    STMT_START { if (cache) { assert((cache)[0] <= (cache)[1]); \
			      assert((cache)[2] <= (cache)[3]); \
			      assert((cache)[3] <= (cache)[1]);} \
			      } STMT_END
#else
#   define ASSERT_UTF8_CACHE(cache) NOOP
#endif

#ifdef PERL_OLD_COPY_ON_WRITE
#define SV_COW_NEXT_SV(sv)	INT2PTR(SV *,SvUVX(sv))
#define SV_COW_NEXT_SV_SET(current,next)	SvUV_set(current, PTR2UV(next))
/* This is a pessimistic view. Scalar must be purely a read-write PV to copy-
   on-write.  */
#endif

/* ============================================================================

=head1 Allocation and deallocation of SVs.

An SV (or AV, HV, etc.) is allocated in two parts: the head (struct
sv, av, hv...) contains type and reference count information, and for
many types, a pointer to the body (struct xrv, xpv, xpviv...), which
contains fields specific to each type.  Some types store all they need
in the head, so don't have a body.

In all but the most memory-paranoid configurations (ex: PURIFY), heads
and bodies are allocated out of arenas, which by default are
approximately 4K chunks of memory parcelled up into N heads or bodies.
Sv-bodies are allocated by their sv-type, guaranteeing size
consistency needed to allocate safely from arrays.

For SV-heads, the first slot in each arena is reserved, and holds a
link to the next arena, some flags, and a note of the number of slots.
Snaked through each arena chain is a linked list of free items; when
this becomes empty, an extra arena is allocated and divided up into N
items which are threaded into the free list.

SV-bodies are similar, but they use arena-sets by default, which
separate the link and info from the arena itself, and reclaim the 1st
slot in the arena.  SV-bodies are further described later.

The following global variables are associated with arenas:

    PL_sv_arenaroot	pointer to list of SV arenas
    PL_sv_root		pointer to list of free SV structures

    PL_body_arenas	head of linked-list of body arenas
    PL_body_roots[]	array of pointers to list of free bodies of svtype
			arrays are indexed by the svtype needed

A few special SV heads are not allocated from an arena, but are
instead directly created in the interpreter structure, eg PL_sv_undef.
The size of arenas can be changed from the default by setting
PERL_ARENA_SIZE appropriately at compile time.

The SV arena serves the secondary purpose of allowing still-live SVs
to be located and destroyed during final cleanup.

At the lowest level, the macros new_SV() and del_SV() grab and free
an SV head.  (If debugging with -DD, del_SV() calls the function S_del_sv()
to return the SV to the free list with error checking.) new_SV() calls
more_sv() / sv_add_arena() to add an extra arena if the free list is empty.
SVs in the free list have their SvTYPE field set to all ones.

At the time of very final cleanup, sv_free_arenas() is called from
perl_destruct() to physically free all the arenas allocated since the
start of the interpreter.

The function visit() scans the SV arenas list, and calls a specified
function for each SV it finds which is still live - ie which has an SvTYPE
other than all 1's, and a non-zero SvREFCNT. visit() is used by the
following functions (specified as [function that calls visit()] / [function
called by visit() for each SV]):

    sv_report_used() / do_report_used()
			dump all remaining SVs (debugging aid)

    sv_clean_objs() / do_clean_objs(),do_clean_named_objs(),
		      do_clean_named_io_objs()
			Attempt to free all objects pointed to by RVs,
			and try to do the same for all objects indirectly
			referenced by typeglobs too.  Called once from
			perl_destruct(), prior to calling sv_clean_all()
			below.

    sv_clean_all() / do_clean_all()
			SvREFCNT_dec(sv) each remaining SV, possibly
			triggering an sv_free(). It also sets the
			SVf_BREAK flag on the SV to indicate that the
			refcnt has been artificially lowered, and thus
			stopping sv_free() from giving spurious warnings
			about SVs which unexpectedly have a refcnt
			of zero.  called repeatedly from perl_destruct()
			until there are no SVs left.

=head2 Arena allocator API Summary

Private API to rest of sv.c

    new_SV(),  del_SV(),

    new_XPVNV(), del_XPVGV(),
    etc

Public API:

    sv_report_used(), sv_clean_objs(), sv_clean_all(), sv_free_arenas()

=cut

 * ========================================================================= */

/*
 * "A time to plant, and a time to uproot what was planted..."
 */

#ifdef PERL_MEM_LOG
#  define MEM_LOG_NEW_SV(sv, file, line, func)	\
	    Perl_mem_log_new_sv(sv, file, line, func)
#  define MEM_LOG_DEL_SV(sv, file, line, func)	\
	    Perl_mem_log_del_sv(sv, file, line, func)
#else
#  define MEM_LOG_NEW_SV(sv, file, line, func)	NOOP
#  define MEM_LOG_DEL_SV(sv, file, line, func)	NOOP
#endif

#ifdef DEBUG_LEAKING_SCALARS
#  define FREE_SV_DEBUG_FILE(sv) Safefree((sv)->sv_debug_file)
#  define DEBUG_SV_SERIAL(sv)						    \
    DEBUG_m(PerlIO_printf(Perl_debug_log, "0x%"UVxf": (%05ld) del_SV\n",    \
	    PTR2UV(sv), (long)(sv)->sv_debug_serial))
#else
#  define FREE_SV_DEBUG_FILE(sv)
#  define DEBUG_SV_SERIAL(sv)	NOOP
#endif

#ifdef PERL_POISON
#  define SvARENA_CHAIN(sv)	((sv)->sv_u.svu_rv)
#  define SvARENA_CHAIN_SET(sv,val)	(sv)->sv_u.svu_rv = MUTABLE_SV((val))
/* Whilst I'd love to do this, it seems that things like to check on
   unreferenced scalars
#  define POSION_SV_HEAD(sv)	PoisonNew(sv, 1, struct STRUCT_SV)
*/
#  define POSION_SV_HEAD(sv)	PoisonNew(&SvANY(sv), 1, void *), \
				PoisonNew(&SvREFCNT(sv), 1, U32)
#else
#  define SvARENA_CHAIN(sv)	SvANY(sv)
#  define SvARENA_CHAIN_SET(sv,val)	SvANY(sv) = (void *)(val)
#  define POSION_SV_HEAD(sv)
#endif

/* Mark an SV head as unused, and add to free list.
 *
 * If SVf_BREAK is set, skip adding it to the free list, as this SV had
 * its refcount artificially decremented during global destruction, so
 * there may be dangling pointers to it. The last thing we want in that
 * case is for it to be reused. */

#define plant_SV(p) \
    STMT_START {					\
	const U32 old_flags = SvFLAGS(p);			\
	MEM_LOG_DEL_SV(p, __FILE__, __LINE__, FUNCTION__);  \
	DEBUG_SV_SERIAL(p);				\
	FREE_SV_DEBUG_FILE(p);				\
	POSION_SV_HEAD(p);				\
	SvFLAGS(p) = SVTYPEMASK;			\
	if (!(old_flags & SVf_BREAK)) {		\
	    SvARENA_CHAIN_SET(p, PL_sv_root);	\
	    PL_sv_root = (p);				\
	}						\
	--PL_sv_count;					\
    } STMT_END

#define uproot_SV(p) \
    STMT_START {					\
	(p) = PL_sv_root;				\
	PL_sv_root = MUTABLE_SV(SvARENA_CHAIN(p));		\
	++PL_sv_count;					\
    } STMT_END


/* make some more SVs by adding another arena */

STATIC SV*
S_more_sv(pTHX)
{
    dVAR;
    SV* sv;
    char *chunk;                /* must use New here to match call to */
    Newx(chunk,PERL_ARENA_SIZE,char);  /* Safefree() in sv_free_arenas() */
    sv_add_arena(chunk, PERL_ARENA_SIZE, 0);
    uproot_SV(sv);
    return sv;
}

/* new_SV(): return a new, empty SV head */

#ifdef DEBUG_LEAKING_SCALARS
/* provide a real function for a debugger to play with */
STATIC SV*
S_new_SV(pTHX_ const char *file, int line, const char *func)
{
    SV* sv;

    if (PL_sv_root)
	uproot_SV(sv);
    else
	sv = S_more_sv(aTHX);
    SvANY(sv) = 0;
    SvREFCNT(sv) = 1;
    SvFLAGS(sv) = 0;
    sv->sv_debug_optype = PL_op ? PL_op->op_type : 0;
    sv->sv_debug_line = (U16) (PL_parser && PL_parser->copline != NOLINE
		? PL_parser->copline
		:  PL_curcop
		    ? CopLINE(PL_curcop)
		    : 0
	    );
    sv->sv_debug_inpad = 0;
    sv->sv_debug_parent = NULL;
    sv->sv_debug_file = PL_curcop ? savepv(CopFILE(PL_curcop)): NULL;

    sv->sv_debug_serial = PL_sv_serial++;

    MEM_LOG_NEW_SV(sv, file, line, func);
    DEBUG_m(PerlIO_printf(Perl_debug_log, "0x%"UVxf": (%05ld) new_SV (from %s:%d [%s])\n",
	    PTR2UV(sv), (long)sv->sv_debug_serial, file, line, func));

    return sv;
}
#  define new_SV(p) (p)=S_new_SV(aTHX_ __FILE__, __LINE__, FUNCTION__)

#else
#  define new_SV(p) \
    STMT_START {					\
	if (PL_sv_root)					\
	    uproot_SV(p);				\
	else						\
	    (p) = S_more_sv(aTHX);			\
	SvANY(p) = 0;					\
	SvREFCNT(p) = 1;				\
	SvFLAGS(p) = 0;					\
	MEM_LOG_NEW_SV(p, __FILE__, __LINE__, FUNCTION__);  \
    } STMT_END
#endif


/* del_SV(): return an empty SV head to the free list */

#ifdef DEBUGGING

#define del_SV(p) \
    STMT_START {					\
	if (DEBUG_D_TEST)				\
	    del_sv(p);					\
	else						\
	    plant_SV(p);				\
    } STMT_END

STATIC void
S_del_sv(pTHX_ SV *p)
{
    dVAR;

    PERL_ARGS_ASSERT_DEL_SV;

    if (DEBUG_D_TEST) {
	SV* sva;
	bool ok = 0;
	for (sva = PL_sv_arenaroot; sva; sva = MUTABLE_SV(SvANY(sva))) {
	    const SV * const sv = sva + 1;
	    const SV * const svend = &sva[SvREFCNT(sva)];
	    if (p >= sv && p < svend) {
		ok = 1;
		break;
	    }
	}
	if (!ok) {
	    Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
			     "Attempt to free non-arena SV: 0x%"UVxf
			     pTHX__FORMAT, PTR2UV(p) pTHX__VALUE);
	    return;
	}
    }
    plant_SV(p);
}

#else /* ! DEBUGGING */

#define del_SV(p)   plant_SV(p)

#endif /* DEBUGGING */


/*
=head1 SV Manipulation Functions

=for apidoc sv_add_arena

Given a chunk of memory, link it to the head of the list of arenas,
and split it into a list of free SVs.

=cut
*/

static void
S_sv_add_arena(pTHX_ char *const ptr, const U32 size, const U32 flags)
{
    dVAR;
    SV *const sva = MUTABLE_SV(ptr);
    register SV* sv;
    register SV* svend;

    PERL_ARGS_ASSERT_SV_ADD_ARENA;

    /* The first SV in an arena isn't an SV. */
    SvANY(sva) = (void *) PL_sv_arenaroot;		/* ptr to next arena */
    SvREFCNT(sva) = size / sizeof(SV);		/* number of SV slots */
    SvFLAGS(sva) = flags;			/* FAKE if not to be freed */

    PL_sv_arenaroot = sva;
    PL_sv_root = sva + 1;

    svend = &sva[SvREFCNT(sva) - 1];
    sv = sva + 1;
    while (sv < svend) {
	SvARENA_CHAIN_SET(sv, (sv + 1));
#ifdef DEBUGGING
	SvREFCNT(sv) = 0;
#endif
	/* Must always set typemask because it's always checked in on cleanup
	   when the arenas are walked looking for objects.  */
	SvFLAGS(sv) = SVTYPEMASK;
	sv++;
    }
    SvARENA_CHAIN_SET(sv, 0);
#ifdef DEBUGGING
    SvREFCNT(sv) = 0;
#endif
    SvFLAGS(sv) = SVTYPEMASK;
}

/* visit(): call the named function for each non-free SV in the arenas
 * whose flags field matches the flags/mask args. */

STATIC I32
S_visit(pTHX_ SVFUNC_t f, const U32 flags, const U32 mask)
{
    dVAR;
    SV* sva;
    I32 visited = 0;

    PERL_ARGS_ASSERT_VISIT;

    for (sva = PL_sv_arenaroot; sva; sva = MUTABLE_SV(SvANY(sva))) {
	register const SV * const svend = &sva[SvREFCNT(sva)];
	register SV* sv;
	for (sv = sva + 1; sv < svend; ++sv) {
	    if (SvTYPE(sv) != (svtype)SVTYPEMASK
		    && (sv->sv_flags & mask) == flags
		    && SvREFCNT(sv))
	    {
		(FCALL)(aTHX_ sv);
		++visited;
	    }
	}
    }
    return visited;
}

#ifdef DEBUGGING

/* called by sv_report_used() for each live SV */

static void
do_report_used(pTHX_ SV *const sv)
{
    if (SvTYPE(sv) != (svtype)SVTYPEMASK) {
	PerlIO_printf(Perl_debug_log, "****\n");
	sv_dump(sv);
    }
}
#endif

/*
=for apidoc sv_report_used

Dump the contents of all SVs not yet freed (debugging aid).

=cut
*/

void
Perl_sv_report_used(pTHX)
{
#ifdef DEBUGGING
    visit(do_report_used, 0, 0);
#else
    PERL_UNUSED_CONTEXT;
#endif
}

/* called by sv_clean_objs() for each live SV */

static void
do_clean_objs(pTHX_ SV *const ref)
{
    dVAR;
    assert (SvROK(ref));
    {
	SV * const target = SvRV(ref);
	if (SvOBJECT(target)) {
	    DEBUG_D((PerlIO_printf(Perl_debug_log, "Cleaning object ref:\n "), sv_dump(ref)));
	    if (SvWEAKREF(ref)) {
		sv_del_backref(target, ref);
		SvWEAKREF_off(ref);
		SvRV_set(ref, NULL);
	    } else {
		SvROK_off(ref);
		SvRV_set(ref, NULL);
		SvREFCNT_dec(target);
	    }
	}
    }

    /* XXX Might want to check arrays, etc. */
}


/* clear any slots in a GV which hold objects - except IO;
 * called by sv_clean_objs() for each live GV */

static void
do_clean_named_objs(pTHX_ SV *const sv)
{
    dVAR;
    SV *obj;
    assert(SvTYPE(sv) == SVt_PVGV);
    assert(isGV_with_GP(sv));
    if (!GvGP(sv))
	return;

    /* freeing GP entries may indirectly free the current GV;
     * hold onto it while we mess with the GP slots */
    SvREFCNT_inc(sv);

    if ( ((obj = GvSV(sv) )) && SvOBJECT(obj)) {
	DEBUG_D((PerlIO_printf(Perl_debug_log,
		"Cleaning named glob SV object:\n "), sv_dump(obj)));
	GvSV(sv) = NULL;
	SvREFCNT_dec(obj);
    }
    if ( ((obj = MUTABLE_SV(GvAV(sv)) )) && SvOBJECT(obj)) {
	DEBUG_D((PerlIO_printf(Perl_debug_log,
		"Cleaning named glob AV object:\n "), sv_dump(obj)));
	GvAV(sv) = NULL;
	SvREFCNT_dec(obj);
    }
    if ( ((obj = MUTABLE_SV(GvHV(sv)) )) && SvOBJECT(obj)) {
	DEBUG_D((PerlIO_printf(Perl_debug_log,
		"Cleaning named glob HV object:\n "), sv_dump(obj)));
	GvHV(sv) = NULL;
	SvREFCNT_dec(obj);
    }
    if ( ((obj = MUTABLE_SV(GvCV(sv)) )) && SvOBJECT(obj)) {
	DEBUG_D((PerlIO_printf(Perl_debug_log,
		"Cleaning named glob CV object:\n "), sv_dump(obj)));
	GvCV_set(sv, NULL);
	SvREFCNT_dec(obj);
    }
    SvREFCNT_dec(sv); /* undo the inc above */
}

/* clear any IO slots in a GV which hold objects (except stderr, defout);
 * called by sv_clean_objs() for each live GV */

static void
do_clean_named_io_objs(pTHX_ SV *const sv)
{
    dVAR;
    SV *obj;
    assert(SvTYPE(sv) == SVt_PVGV);
    assert(isGV_with_GP(sv));
    if (!GvGP(sv) || sv == (SV*)PL_stderrgv || sv == (SV*)PL_defoutgv)
	return;

    SvREFCNT_inc(sv);
    if ( ((obj = MUTABLE_SV(GvIO(sv)) )) && SvOBJECT(obj)) {
	DEBUG_D((PerlIO_printf(Perl_debug_log,
		"Cleaning named glob IO object:\n "), sv_dump(obj)));
	GvIOp(sv) = NULL;
	SvREFCNT_dec(obj);
    }
    SvREFCNT_dec(sv); /* undo the inc above */
}

/* Void wrapper to pass to visit() */
static void
do_curse(pTHX_ SV * const sv) {
    if ((PL_stderrgv && GvGP(PL_stderrgv) && (SV*)GvIO(PL_stderrgv) == sv)
     || (PL_defoutgv && GvGP(PL_defoutgv) && (SV*)GvIO(PL_defoutgv) == sv))
	return;
    (void)curse(sv, 0);
}

/*
=for apidoc sv_clean_objs

Attempt to destroy all objects not yet freed.

=cut
*/

void
Perl_sv_clean_objs(pTHX)
{
    dVAR;
    GV *olddef, *olderr;
    PL_in_clean_objs = TRUE;
    visit(do_clean_objs, SVf_ROK, SVf_ROK);
    /* Some barnacles may yet remain, clinging to typeglobs.
     * Run the non-IO destructors first: they may want to output
     * error messages, close files etc */
    visit(do_clean_named_objs, SVt_PVGV|SVpgv_GP, SVTYPEMASK|SVp_POK|SVpgv_GP);
    visit(do_clean_named_io_objs, SVt_PVGV|SVpgv_GP, SVTYPEMASK|SVp_POK|SVpgv_GP);
    /* And if there are some very tenacious barnacles clinging to arrays,
       closures, or what have you.... */
    visit(do_curse, SVs_OBJECT, SVs_OBJECT);
    olddef = PL_defoutgv;
    PL_defoutgv = NULL; /* disable skip of PL_defoutgv */
    if (olddef && isGV_with_GP(olddef))
	do_clean_named_io_objs(aTHX_ MUTABLE_SV(olddef));
    olderr = PL_stderrgv;
    PL_stderrgv = NULL; /* disable skip of PL_stderrgv */
    if (olderr && isGV_with_GP(olderr))
	do_clean_named_io_objs(aTHX_ MUTABLE_SV(olderr));
    SvREFCNT_dec(olddef);
    PL_in_clean_objs = FALSE;
}

/* called by sv_clean_all() for each live SV */

static void
do_clean_all(pTHX_ SV *const sv)
{
    dVAR;
    if (sv == (const SV *) PL_fdpid || sv == (const SV *)PL_strtab) {
	/* don't clean pid table and strtab */
	return;
    }
    DEBUG_D((PerlIO_printf(Perl_debug_log, "Cleaning loops: SV at 0x%"UVxf"\n", PTR2UV(sv)) ));
    SvFLAGS(sv) |= SVf_BREAK;
    SvREFCNT_dec(sv);
}

/*
=for apidoc sv_clean_all

Decrement the refcnt of each remaining SV, possibly triggering a
cleanup.  This function may have to be called multiple times to free
SVs which are in complex self-referential hierarchies.

=cut
*/

I32
Perl_sv_clean_all(pTHX)
{
    dVAR;
    I32 cleaned;
    PL_in_clean_all = TRUE;
    cleaned = visit(do_clean_all, 0,0);
    return cleaned;
}

/*
  ARENASETS: a meta-arena implementation which separates arena-info
  into struct arena_set, which contains an array of struct
  arena_descs, each holding info for a single arena.  By separating
  the meta-info from the arena, we recover the 1st slot, formerly
  borrowed for list management.  The arena_set is about the size of an
  arena, avoiding the needless malloc overhead of a naive linked-list.

  The cost is 1 arena-set malloc per ~320 arena-mallocs, + the unused
  memory in the last arena-set (1/2 on average).  In trade, we get
  back the 1st slot in each arena (ie 1.7% of a CV-arena, less for
  smaller types).  The recovery of the wasted space allows use of
  small arenas for large, rare body types, by changing array* fields
  in body_details_by_type[] below.
*/
struct arena_desc {
    char       *arena;		/* the raw storage, allocated aligned */
    size_t      size;		/* its size ~4k typ */
    svtype	utype;		/* bodytype stored in arena */
};

struct arena_set;

/* Get the maximum number of elements in set[] such that struct arena_set
   will fit within PERL_ARENA_SIZE, which is probably just under 4K, and
   therefore likely to be 1 aligned memory page.  */

#define ARENAS_PER_SET  ((PERL_ARENA_SIZE - sizeof(struct arena_set*) \
			  - 2 * sizeof(int)) / sizeof (struct arena_desc))

struct arena_set {
    struct arena_set* next;
    unsigned int   set_size;	/* ie ARENAS_PER_SET */
    unsigned int   curr;	/* index of next available arena-desc */
    struct arena_desc set[ARENAS_PER_SET];
};

/*
=for apidoc sv_free_arenas

Deallocate the memory used by all arenas.  Note that all the individual SV
heads and bodies within the arenas must already have been freed.

=cut
*/
void
Perl_sv_free_arenas(pTHX)
{
    dVAR;
    SV* sva;
    SV* svanext;
    unsigned int i;

    /* Free arenas here, but be careful about fake ones.  (We assume
       contiguity of the fake ones with the corresponding real ones.) */

    for (sva = PL_sv_arenaroot; sva; sva = svanext) {
	svanext = MUTABLE_SV(SvANY(sva));
	while (svanext && SvFAKE(svanext))
	    svanext = MUTABLE_SV(SvANY(svanext));

	if (!SvFAKE(sva))
	    Safefree(sva);
    }

    {
	struct arena_set *aroot = (struct arena_set*) PL_body_arenas;

	while (aroot) {
	    struct arena_set *current = aroot;
	    i = aroot->curr;
	    while (i--) {
		assert(aroot->set[i].arena);
		Safefree(aroot->set[i].arena);
	    }
	    aroot = aroot->next;
	    Safefree(current);
	}
    }
    PL_body_arenas = 0;

    i = PERL_ARENA_ROOTS_SIZE;
    while (i--)
	PL_body_roots[i] = 0;

    PL_sv_arenaroot = 0;
    PL_sv_root = 0;
}

/*
  Here are mid-level routines that manage the allocation of bodies out
  of the various arenas.  There are 5 kinds of arenas:

  1. SV-head arenas, which are discussed and handled above
  2. regular body arenas
  3. arenas for reduced-size bodies
  4. Hash-Entry arenas

  Arena types 2 & 3 are chained by body-type off an array of
  arena-root pointers, which is indexed by svtype.  Some of the
  larger/less used body types are malloced singly, since a large
  unused block of them is wasteful.  Also, several svtypes dont have
  bodies; the data fits into the sv-head itself.  The arena-root
  pointer thus has a few unused root-pointers (which may be hijacked
  later for arena types 4,5)

  3 differs from 2 as an optimization; some body types have several
  unused fields in the front of the structure (which are kept in-place
  for consistency).  These bodies can be allocated in smaller chunks,
  because the leading fields arent accessed.  Pointers to such bodies
  are decremented to point at the unused 'ghost' memory, knowing that
  the pointers are used with offsets to the real memory.


=head1 SV-Body Allocation

Allocation of SV-bodies is similar to SV-heads, differing as follows;
the allocation mechanism is used for many body types, so is somewhat
more complicated, it uses arena-sets, and has no need for still-live
SV detection.

At the outermost level, (new|del)_X*V macros return bodies of the
appropriate type.  These macros call either (new|del)_body_type or
(new|del)_body_allocated macro pairs, depending on specifics of the
type.  Most body types use the former pair, the latter pair is used to
allocate body types with "ghost fields".

"ghost fields" are fields that are unused in certain types, and
consequently don't need to actually exist.  They are declared because
they're part of a "base type", which allows use of functions as
methods.  The simplest examples are AVs and HVs, 2 aggregate types
which don't use the fields which support SCALAR semantics.

For these types, the arenas are carved up into appropriately sized
chunks, we thus avoid wasted memory for those unaccessed members.
When bodies are allocated, we adjust the pointer back in memory by the
size of the part not allocated, so it's as if we allocated the full
structure.  (But things will all go boom if you write to the part that
is "not there", because you'll be overwriting the last members of the
preceding structure in memory.)

We calculate the correction using the STRUCT_OFFSET macro on the first
member present. If the allocated structure is smaller (no initial NV
actually allocated) then the net effect is to subtract the size of the NV
from the pointer, to return a new pointer as if an initial NV were actually
allocated. (We were using structures named *_allocated for this, but
this turned out to be a subtle bug, because a structure without an NV
could have a lower alignment constraint, but the compiler is allowed to
optimised accesses based on the alignment constraint of the actual pointer
to the full structure, for example, using a single 64 bit load instruction
because it "knows" that two adjacent 32 bit members will be 8-byte aligned.)

This is the same trick as was used for NV and IV bodies. Ironically it
doesn't need to be used for NV bodies any more, because NV is now at
the start of the structure. IV bodies don't need it either, because
they are no longer allocated.

In turn, the new_body_* allocators call S_new_body(), which invokes
new_body_inline macro, which takes a lock, and takes a body off the
linked list at PL_body_roots[sv_type], calling Perl_more_bodies() if
necessary to refresh an empty list.  Then the lock is released, and
the body is returned.

Perl_more_bodies allocates a new arena, and carves it up into an array of N
bodies, which it strings into a linked list.  It looks up arena-size
and body-size from the body_details table described below, thus
supporting the multiple body-types.

If PURIFY is defined, or PERL_ARENA_SIZE=0, arenas are not used, and
the (new|del)_X*V macros are mapped directly to malloc/free.

For each sv-type, struct body_details bodies_by_type[] carries
parameters which control these aspects of SV handling:

Arena_size determines whether arenas are used for this body type, and if
so, how big they are.  PURIFY or PERL_ARENA_SIZE=0 set this field to
zero, forcing individual mallocs and frees.

Body_size determines how big a body is, and therefore how many fit into
each arena.  Offset carries the body-pointer adjustment needed for
"ghost fields", and is used in *_allocated macros.

But its main purpose is to parameterize info needed in
Perl_sv_upgrade().  The info here dramatically simplifies the function
vs the implementation in 5.8.8, making it table-driven.  All fields
are used for this, except for arena_size.

For the sv-types that have no bodies, arenas are not used, so those
PL_body_roots[sv_type] are unused, and can be overloaded.  In
something of a special case, SVt_NULL is borrowed for HE arenas;
PL_body_roots[HE_SVSLOT=SVt_NULL] is filled by S_more_he, but the
bodies_by_type[SVt_NULL] slot is not used, as the table is not
available in hv.c.

*/

struct body_details {
    U8 body_size;	/* Size to allocate  */
    U8 copy;		/* Size of structure to copy (may be shorter)  */
    U8 offset;
    unsigned int type : 4;	    /* We have space for a sanity check.  */
    unsigned int cant_upgrade : 1;  /* Cannot upgrade this type */
    unsigned int zero_nv : 1;	    /* zero the NV when upgrading from this */
    unsigned int arena : 1;	    /* Allocated from an arena */
    size_t arena_size;		    /* Size of arena to allocate */
};

#define HADNV FALSE
#define NONV TRUE


#ifdef PURIFY
/* With -DPURFIY we allocate everything directly, and don't use arenas.
   This seems a rather elegant way to simplify some of the code below.  */
#define HASARENA FALSE
#else
#define HASARENA TRUE
#endif
#define NOARENA FALSE

/* Size the arenas to exactly fit a given number of bodies.  A count
   of 0 fits the max number bodies into a PERL_ARENA_SIZE.block,
   simplifying the default.  If count > 0, the arena is sized to fit
   only that many bodies, allowing arenas to be used for large, rare
   bodies (XPVFM, XPVIO) without undue waste.  The arena size is
   limited by PERL_ARENA_SIZE, so we can safely oversize the
   declarations.
 */
#define FIT_ARENA0(body_size)				\
    ((size_t)(PERL_ARENA_SIZE / body_size) * body_size)
#define FIT_ARENAn(count,body_size)			\
    ( count * body_size <= PERL_ARENA_SIZE)		\
    ? count * body_size					\
    : FIT_ARENA0 (body_size)
#define FIT_ARENA(count,body_size)			\
    count 						\
    ? FIT_ARENAn (count, body_size)			\
    : FIT_ARENA0 (body_size)

/* Calculate the length to copy. Specifically work out the length less any
   final padding the compiler needed to add.  See the comment in sv_upgrade
   for why copying the padding proved to be a bug.  */

#define copy_length(type, last_member) \
	STRUCT_OFFSET(type, last_member) \
	+ sizeof (((type*)SvANY((const SV *)0))->last_member)

static const struct body_details bodies_by_type[] = {
    /* HEs use this offset for their arena.  */
    { 0, 0, 0, SVt_NULL, FALSE, NONV, NOARENA, 0 },

    /* The bind placeholder pretends to be an RV for now.
       Also it's marked as "can't upgrade" to stop anyone using it before it's
       implemented.  */
    { 0, 0, 0, SVt_BIND, TRUE, NONV, NOARENA, 0 },

    /* IVs are in the head, so the allocation size is 0.  */
    { 0,
      sizeof(IV), /* This is used to copy out the IV body.  */
      STRUCT_OFFSET(XPVIV, xiv_iv), SVt_IV, FALSE, NONV,
      NOARENA /* IVS don't need an arena  */, 0
    },

    { sizeof(NV), sizeof(NV),
      STRUCT_OFFSET(XPVNV, xnv_u),
      SVt_NV, FALSE, HADNV, HASARENA, FIT_ARENA(0, sizeof(NV)) },

    { sizeof(XPV) - STRUCT_OFFSET(XPV, xpv_cur),
      copy_length(XPV, xpv_len) - STRUCT_OFFSET(XPV, xpv_cur),
      + STRUCT_OFFSET(XPV, xpv_cur),
      SVt_PV, FALSE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(XPV) - STRUCT_OFFSET(XPV, xpv_cur)) },

    { sizeof(XPVIV) - STRUCT_OFFSET(XPV, xpv_cur),
      copy_length(XPVIV, xiv_u) - STRUCT_OFFSET(XPV, xpv_cur),
      + STRUCT_OFFSET(XPV, xpv_cur),
      SVt_PVIV, FALSE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(XPVIV) - STRUCT_OFFSET(XPV, xpv_cur)) },

    { sizeof(XPVNV) - STRUCT_OFFSET(XPV, xpv_cur),
      copy_length(XPVNV, xnv_u) - STRUCT_OFFSET(XPV, xpv_cur),
      + STRUCT_OFFSET(XPV, xpv_cur),
      SVt_PVNV, FALSE, HADNV, HASARENA,
      FIT_ARENA(0, sizeof(XPVNV) - STRUCT_OFFSET(XPV, xpv_cur)) },

    { sizeof(XPVMG), copy_length(XPVMG, xnv_u), 0, SVt_PVMG, FALSE, HADNV,
      HASARENA, FIT_ARENA(0, sizeof(XPVMG)) },

    { sizeof(regexp),
      sizeof(regexp),
      0,
      SVt_REGEXP, FALSE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(regexp))
    },

    { sizeof(XPVGV), sizeof(XPVGV), 0, SVt_PVGV, TRUE, HADNV,
      HASARENA, FIT_ARENA(0, sizeof(XPVGV)) },
    
    { sizeof(XPVLV), sizeof(XPVLV), 0, SVt_PVLV, TRUE, HADNV,
      HASARENA, FIT_ARENA(0, sizeof(XPVLV)) },

    { sizeof(XPVAV),
      copy_length(XPVAV, xav_alloc),
      0,
      SVt_PVAV, TRUE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(XPVAV)) },

    { sizeof(XPVHV),
      copy_length(XPVHV, xhv_max),
      0,
      SVt_PVHV, TRUE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(XPVHV)) },

    { sizeof(XPVCV),
      sizeof(XPVCV),
      0,
      SVt_PVCV, TRUE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(XPVCV)) },

    { sizeof(XPVFM),
      sizeof(XPVFM),
      0,
      SVt_PVFM, TRUE, NONV, NOARENA,
      FIT_ARENA(20, sizeof(XPVFM)) },

    { sizeof(XPVIO),
      sizeof(XPVIO),
      0,
      SVt_PVIO, TRUE, NONV, HASARENA,
      FIT_ARENA(24, sizeof(XPVIO)) },
};

#define new_body_allocated(sv_type)		\
    (void *)((char *)S_new_body(aTHX_ sv_type)	\
	     - bodies_by_type[sv_type].offset)

/* return a thing to the free list */

#define del_body(thing, root)				\
    STMT_START {					\
	void ** const thing_copy = (void **)thing;	\
	*thing_copy = *root;				\
	*root = (void*)thing_copy;			\
    } STMT_END

#ifdef PURIFY

#define new_XNV()	safemalloc(sizeof(XPVNV))
#define new_XPVNV()	safemalloc(sizeof(XPVNV))
#define new_XPVMG()	safemalloc(sizeof(XPVMG))

#define del_XPVGV(p)	safefree(p)

#else /* !PURIFY */

#define new_XNV()	new_body_allocated(SVt_NV)
#define new_XPVNV()	new_body_allocated(SVt_PVNV)
#define new_XPVMG()	new_body_allocated(SVt_PVMG)

#define del_XPVGV(p)	del_body(p + bodies_by_type[SVt_PVGV].offset,	\
				 &PL_body_roots[SVt_PVGV])

#endif /* PURIFY */

/* no arena for you! */

#define new_NOARENA(details) \
	safemalloc((details)->body_size + (details)->offset)
#define new_NOARENAZ(details) \
	safecalloc((details)->body_size + (details)->offset, 1)

void *
Perl_more_bodies (pTHX_ const svtype sv_type, const size_t body_size,
		  const size_t arena_size)
{
    dVAR;
    void ** const root = &PL_body_roots[sv_type];
    struct arena_desc *adesc;
    struct arena_set *aroot = (struct arena_set *) PL_body_arenas;
    unsigned int curr;
    char *start;
    const char *end;
    const size_t good_arena_size = Perl_malloc_good_size(arena_size);
#if defined(DEBUGGING) && !defined(PERL_GLOBAL_STRUCT_PRIVATE)
    static bool done_sanity_check;

    /* PERL_GLOBAL_STRUCT_PRIVATE cannot coexist with global
     * variables like done_sanity_check. */
    if (!done_sanity_check) {
	unsigned int i = SVt_LAST;

	done_sanity_check = TRUE;

	while (i--)
	    assert (bodies_by_type[i].type == i);
    }
#endif

    assert(arena_size);

    /* may need new arena-set to hold new arena */
    if (!aroot || aroot->curr >= aroot->set_size) {
	struct arena_set *newroot;
	Newxz(newroot, 1, struct arena_set);
	newroot->set_size = ARENAS_PER_SET;
	newroot->next = aroot;
	aroot = newroot;
	PL_body_arenas = (void *) newroot;
	DEBUG_m(PerlIO_printf(Perl_debug_log, "new arenaset %p\n", (void*)aroot));
    }

    /* ok, now have arena-set with at least 1 empty/available arena-desc */
    curr = aroot->curr++;
    adesc = &(aroot->set[curr]);
    assert(!adesc->arena);
    
    Newx(adesc->arena, good_arena_size, char);
    adesc->size = good_arena_size;
    adesc->utype = sv_type;
    DEBUG_m(PerlIO_printf(Perl_debug_log, "arena %d added: %p size %"UVuf"\n", 
			  curr, (void*)adesc->arena, (UV)good_arena_size));

    start = (char *) adesc->arena;

    /* Get the address of the byte after the end of the last body we can fit.
       Remember, this is integer division:  */
    end = start + good_arena_size / body_size * body_size;

    /* computed count doesn't reflect the 1st slot reservation */
#if defined(MYMALLOC) || defined(HAS_MALLOC_GOOD_SIZE)
    DEBUG_m(PerlIO_printf(Perl_debug_log,
			  "arena %p end %p arena-size %d (from %d) type %d "
			  "size %d ct %d\n",
			  (void*)start, (void*)end, (int)good_arena_size,
			  (int)arena_size, sv_type, (int)body_size,
			  (int)good_arena_size / (int)body_size));
#else
    DEBUG_m(PerlIO_printf(Perl_debug_log,
			  "arena %p end %p arena-size %d type %d size %d ct %d\n",
			  (void*)start, (void*)end,
			  (int)arena_size, sv_type, (int)body_size,
			  (int)good_arena_size / (int)body_size));
#endif
    *root = (void *)start;

    while (1) {
	/* Where the next body would start:  */
	char * const next = start + body_size;

	if (next >= end) {
	    /* This is the last body:  */
	    assert(next == end);

	    *(void **)start = 0;
	    return *root;
	}

	*(void**) start = (void *)next;
	start = next;
    }
}

/* grab a new thing from the free list, allocating more if necessary.
   The inline version is used for speed in hot routines, and the
   function using it serves the rest (unless PURIFY).
*/
#define new_body_inline(xpv, sv_type) \
    STMT_START { \
	void ** const r3wt = &PL_body_roots[sv_type]; \
	xpv = (PTR_TBL_ENT_t*) (*((void **)(r3wt))      \
	  ? *((void **)(r3wt)) : Perl_more_bodies(aTHX_ sv_type, \
					     bodies_by_type[sv_type].body_size,\
					     bodies_by_type[sv_type].arena_size)); \
	*(r3wt) = *(void**)(xpv); \
    } STMT_END

#ifndef PURIFY

STATIC void *
S_new_body(pTHX_ const svtype sv_type)
{
    dVAR;
    void *xpv;
    new_body_inline(xpv, sv_type);
    return xpv;
}

#endif

static const struct body_details fake_rv =
    { 0, 0, 0, SVt_IV, FALSE, NONV, NOARENA, 0 };

/*
=for apidoc sv_upgrade

Upgrade an SV to a more complex form.  Generally adds a new body type to the
SV, then copies across as much information as possible from the old body.
It croaks if the SV is already in a more complex form than requested.  You
generally want to use the C<SvUPGRADE> macro wrapper, which checks the type
before calling C<sv_upgrade>, and hence does not croak.  See also
C<svtype>.

=cut
*/

void
Perl_sv_upgrade(pTHX_ register SV *const sv, svtype new_type)
{
    dVAR;
    void*	old_body;
    void*	new_body;
    const svtype old_type = SvTYPE(sv);
    const struct body_details *new_type_details;
    const struct body_details *old_type_details
	= bodies_by_type + old_type;
    SV *referant = NULL;

    PERL_ARGS_ASSERT_SV_UPGRADE;

    if (old_type == new_type)
	return;

    /* This clause was purposefully added ahead of the early return above to
       the shared string hackery for (sort {$a <=> $b} keys %hash), with the
       inference by Nick I-S that it would fix other troublesome cases. See
       changes 7162, 7163 (f130fd4589cf5fbb24149cd4db4137c8326f49c1 and parent)

       Given that shared hash key scalars are no longer PVIV, but PV, there is
       no longer need to unshare so as to free up the IVX slot for its proper
       purpose. So it's safe to move the early return earlier.  */

    if (new_type != SVt_PV && SvIsCOW(sv)) {
	sv_force_normal_flags(sv, 0);
    }

    old_body = SvANY(sv);

    /* Copying structures onto other structures that have been neatly zeroed
       has a subtle gotcha. Consider XPVMG

       +------+------+------+------+------+-------+-------+
       |     NV      | CUR  | LEN  |  IV  | MAGIC | STASH |
       +------+------+------+------+------+-------+-------+
       0      4      8     12     16     20      24      28

       where NVs are aligned to 8 bytes, so that sizeof that structure is
       actually 32 bytes long, with 4 bytes of padding at the end:

       +------+------+------+------+------+-------+-------+------+
       |     NV      | CUR  | LEN  |  IV  | MAGIC | STASH | ???  |
       +------+------+------+------+------+-------+-------+------+
       0      4      8     12     16     20      24      28     32

       so what happens if you allocate memory for this structure:

       +------+------+------+------+------+-------+-------+------+------+...
       |     NV      | CUR  | LEN  |  IV  | MAGIC | STASH |  GP  | NAME |
       +------+------+------+------+------+-------+-------+------+------+...
       0      4      8     12     16     20      24      28     32     36

       zero it, then copy sizeof(XPVMG) bytes on top of it? Not quite what you
       expect, because you copy the area marked ??? onto GP. Now, ??? may have
       started out as zero once, but it's quite possible that it isn't. So now,
       rather than a nicely zeroed GP, you have it pointing somewhere random.
       Bugs ensue.

       (In fact, GP ends up pointing at a previous GP structure, because the
       principle cause of the padding in XPVMG getting garbage is a copy of
       sizeof(XPVMG) bytes from a XPVGV structure in sv_unglob. Right now
       this happens to be moot because XPVGV has been re-ordered, with GP
       no longer after STASH)

       So we are careful and work out the size of used parts of all the
       structures.  */

    switch (old_type) {
    case SVt_NULL:
	break;
    case SVt_IV:
	if (SvROK(sv)) {
	    referant = SvRV(sv);
	    old_type_details = &fake_rv;
	    if (new_type == SVt_NV)
		new_type = SVt_PVNV;
	} else {
	    if (new_type < SVt_PVIV) {
		new_type = (new_type == SVt_NV)
		    ? SVt_PVNV : SVt_PVIV;
	    }
	}
	break;
    case SVt_NV:
	if (new_type < SVt_PVNV) {
	    new_type = SVt_PVNV;
	}
	break;
    case SVt_PV:
	assert(new_type > SVt_PV);
	assert(SVt_IV < SVt_PV);
	assert(SVt_NV < SVt_PV);
	break;
    case SVt_PVIV:
	break;
    case SVt_PVNV:
	break;
    case SVt_PVMG:
	/* Because the XPVMG of PL_mess_sv isn't allocated from the arena,
	   there's no way that it can be safely upgraded, because perl.c
	   expects to Safefree(SvANY(PL_mess_sv))  */
	assert(sv != PL_mess_sv);
	/* This flag bit is used to mean other things in other scalar types.
	   Given that it only has meaning inside the pad, it shouldn't be set
	   on anything that can get upgraded.  */
	assert(!SvPAD_TYPED(sv));
	break;
    default:
	if (old_type_details->cant_upgrade)
	    Perl_croak(aTHX_ "Can't upgrade %s (%" UVuf ") to %" UVuf,
		       sv_reftype(sv, 0), (UV) old_type, (UV) new_type);
    }

    if (old_type > new_type)
	Perl_croak(aTHX_ "sv_upgrade from type %d down to type %d",
		(int)old_type, (int)new_type);

    new_type_details = bodies_by_type + new_type;

    SvFLAGS(sv) &= ~SVTYPEMASK;
    SvFLAGS(sv) |= new_type;

    /* This can't happen, as SVt_NULL is <= all values of new_type, so one of
       the return statements above will have triggered.  */
    assert (new_type != SVt_NULL);
    switch (new_type) {
    case SVt_IV:
	assert(old_type == SVt_NULL);
	SvANY(sv) = (XPVIV*)((char*)&(sv->sv_u.svu_iv) - STRUCT_OFFSET(XPVIV, xiv_iv));
	SvIV_set(sv, 0);
	return;
    case SVt_NV:
	assert(old_type == SVt_NULL);
	SvANY(sv) = new_XNV();
	SvNV_set(sv, 0);
	return;
    case SVt_PVHV:
    case SVt_PVAV:
	assert(new_type_details->body_size);

#ifndef PURIFY	
	assert(new_type_details->arena);
	assert(new_type_details->arena_size);
	/* This points to the start of the allocated area.  */
	new_body_inline(new_body, new_type);
	Zero(new_body, new_type_details->body_size, char);
	new_body = ((char *)new_body) - new_type_details->offset;
#else
	/* We always allocated the full length item with PURIFY. To do this
	   we fake things so that arena is false for all 16 types..  */
	new_body = new_NOARENAZ(new_type_details);
#endif
	SvANY(sv) = new_body;
	if (new_type == SVt_PVAV) {
	    AvMAX(sv)	= -1;
	    AvFILLp(sv)	= -1;
	    AvREAL_only(sv);
	    if (old_type_details->body_size) {
		AvALLOC(sv) = 0;
	    } else {
		/* It will have been zeroed when the new body was allocated.
		   Lets not write to it, in case it confuses a write-back
		   cache.  */
	    }
	} else {
	    assert(!SvOK(sv));
	    SvOK_off(sv);
#ifndef NODEFAULT_SHAREKEYS
	    HvSHAREKEYS_on(sv);         /* key-sharing on by default */
#endif
	    HvMAX(sv) = 7; /* (start with 8 buckets) */
	}

	/* SVt_NULL isn't the only thing upgraded to AV or HV.
	   The target created by newSVrv also is, and it can have magic.
	   However, it never has SvPVX set.
	*/
	if (old_type == SVt_IV) {
	    assert(!SvROK(sv));
	} else if (old_type >= SVt_PV) {
	    assert(SvPVX_const(sv) == 0);
	}

	if (old_type >= SVt_PVMG) {
	    SvMAGIC_set(sv, ((XPVMG*)old_body)->xmg_u.xmg_magic);
	    SvSTASH_set(sv, ((XPVMG*)old_body)->xmg_stash);
	} else {
	    sv->sv_u.svu_array = NULL; /* or svu_hash  */
	}
	break;


    case SVt_REGEXP:
	/* This ensures that SvTHINKFIRST(sv) is true, and hence that
	   sv_force_normal_flags(sv) is called.  */
	SvFAKE_on(sv);
    case SVt_PVIV:
	/* XXX Is this still needed?  Was it ever needed?   Surely as there is
	   no route from NV to PVIV, NOK can never be true  */
	assert(!SvNOKp(sv));
	assert(!SvNOK(sv));
    case SVt_PVIO:
    case SVt_PVFM:
    case SVt_PVGV:
    case SVt_PVCV:
    case SVt_PVLV:
    case SVt_PVMG:
    case SVt_PVNV:
    case SVt_PV:

	assert(new_type_details->body_size);
	/* We always allocated the full length item with PURIFY. To do this
	   we fake things so that arena is false for all 16 types..  */
	if(new_type_details->arena) {
	    /* This points to the start of the allocated area.  */
	    new_body_inline(new_body, new_type);
	    Zero(new_body, new_type_details->body_size, char);
	    new_body = ((char *)new_body) - new_type_details->offset;
	} else {
	    new_body = new_NOARENAZ(new_type_details);
	}
	SvANY(sv) = new_body;

	if (old_type_details->copy) {
	    /* There is now the potential for an upgrade from something without
	       an offset (PVNV or PVMG) to something with one (PVCV, PVFM)  */
	    int offset = old_type_details->offset;
	    int length = old_type_details->copy;

	    if (new_type_details->offset > old_type_details->offset) {
		const int difference
		    = new_type_details->offset - old_type_details->offset;
		offset += difference;
		length -= difference;
	    }
	    assert (length >= 0);
		
	    Copy((char *)old_body + offset, (char *)new_body + offset, length,
		 char);
	}

#ifndef NV_ZERO_IS_ALLBITS_ZERO
	/* If NV 0.0 is stores as all bits 0 then Zero() already creates a
	 * correct 0.0 for us.  Otherwise, if the old body didn't have an
	 * NV slot, but the new one does, then we need to initialise the
	 * freshly created NV slot with whatever the correct bit pattern is
	 * for 0.0  */
	if (old_type_details->zero_nv && !new_type_details->zero_nv
	    && !isGV_with_GP(sv))
	    SvNV_set(sv, 0);
#endif

	if (new_type == SVt_PVIO) {
	    IO * const io = MUTABLE_IO(sv);
	    GV *iogv = gv_fetchpvs("IO::File::", GV_ADD, SVt_PVHV);

	    SvOBJECT_on(io);
	    /* Clear the stashcache because a new IO could overrule a package
	       name */
	    hv_clear(PL_stashcache);

	    SvSTASH_set(io, MUTABLE_HV(SvREFCNT_inc(GvHV(iogv))));
	    IoPAGE_LEN(sv) = 60;
	}
	if (old_type < SVt_PV) {
	    /* referant will be NULL unless the old type was SVt_IV emulating
	       SVt_RV */
	    sv->sv_u.svu_rv = referant;
	}
	break;
    default:
	Perl_croak(aTHX_ "panic: sv_upgrade to unknown type %lu",
		   (unsigned long)new_type);
    }

    if (old_type > SVt_IV) {
#ifdef PURIFY
	safefree(old_body);
#else
	/* Note that there is an assumption that all bodies of types that
	   can be upgraded came from arenas. Only the more complex non-
	   upgradable types are allowed to be directly malloc()ed.  */
	assert(old_type_details->arena);
	del_body((void*)((char*)old_body + old_type_details->offset),
		 &PL_body_roots[old_type]);
#endif
    }
}

/*
=for apidoc sv_backoff

Remove any string offset.  You should normally use the C<SvOOK_off> macro
wrapper instead.

=cut
*/

int
Perl_sv_backoff(pTHX_ register SV *const sv)
{
    STRLEN delta;
    const char * const s = SvPVX_const(sv);

    PERL_ARGS_ASSERT_SV_BACKOFF;
    PERL_UNUSED_CONTEXT;

    assert(SvOOK(sv));
    assert(SvTYPE(sv) != SVt_PVHV);
    assert(SvTYPE(sv) != SVt_PVAV);

    SvOOK_offset(sv, delta);
    
    SvLEN_set(sv, SvLEN(sv) + delta);
    SvPV_set(sv, SvPVX(sv) - delta);
    Move(s, SvPVX(sv), SvCUR(sv)+1, char);
    SvFLAGS(sv) &= ~SVf_OOK;
    return 0;
}

/*
=for apidoc sv_grow

Expands the character buffer in the SV.  If necessary, uses C<sv_unref> and
upgrades the SV to C<SVt_PV>.  Returns a pointer to the character buffer.
Use the C<SvGROW> wrapper instead.

=cut
*/

char *
Perl_sv_grow(pTHX_ register SV *const sv, register STRLEN newlen)
{
    register char *s;

    PERL_ARGS_ASSERT_SV_GROW;

    if (PL_madskills && newlen >= 0x100000) {
	PerlIO_printf(Perl_debug_log,
		      "Allocation too large: %"UVxf"\n", (UV)newlen);
    }
#ifdef HAS_64K_LIMIT
    if (newlen >= 0x10000) {
	PerlIO_printf(Perl_debug_log,
		      "Allocation too large: %"UVxf"\n", (UV)newlen);
	my_exit(1);
    }
#endif /* HAS_64K_LIMIT */
    if (SvROK(sv))
	sv_unref(sv);
    if (SvTYPE(sv) < SVt_PV) {
	sv_upgrade(sv, SVt_PV);
	s = SvPVX_mutable(sv);
    }
    else if (SvOOK(sv)) {	/* pv is offset? */
	sv_backoff(sv);
	s = SvPVX_mutable(sv);
	if (newlen > SvLEN(sv))
	    newlen += 10 * (newlen - SvCUR(sv)); /* avoid copy each time */
#ifdef HAS_64K_LIMIT
	if (newlen >= 0x10000)
	    newlen = 0xFFFF;
#endif
    }
    else
	s = SvPVX_mutable(sv);

    if (newlen > SvLEN(sv)) {		/* need more room? */
	STRLEN minlen = SvCUR(sv);
	minlen += (minlen >> PERL_STRLEN_EXPAND_SHIFT) + 10;
	if (newlen < minlen)
	    newlen = minlen;
#ifndef Perl_safesysmalloc_size
	newlen = PERL_STRLEN_ROUNDUP(newlen);
#endif
	if (SvLEN(sv) && s) {
	    s = (char*)saferealloc(s, newlen);
	}
	else {
	    s = (char*)safemalloc(newlen);
	    if (SvPVX_const(sv) && SvCUR(sv)) {
	        Move(SvPVX_const(sv), s, (newlen < SvCUR(sv)) ? newlen : SvCUR(sv), char);
	    }
	}
	SvPV_set(sv, s);
#ifdef Perl_safesysmalloc_size
	/* Do this here, do it once, do it right, and then we will never get
	   called back into sv_grow() unless there really is some growing
	   needed.  */
	SvLEN_set(sv, Perl_safesysmalloc_size(s));
#else
        SvLEN_set(sv, newlen);
#endif
    }
    return s;
}

/*
=for apidoc sv_setiv

Copies an integer into the given SV, upgrading first if necessary.
Does not handle 'set' magic.  See also C<sv_setiv_mg>.

=cut
*/

void
Perl_sv_setiv(pTHX_ register SV *const sv, const IV i)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_SETIV;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    switch (SvTYPE(sv)) {
    case SVt_NULL:
    case SVt_NV:
	sv_upgrade(sv, SVt_IV);
	break;
    case SVt_PV:
	sv_upgrade(sv, SVt_PVIV);
	break;

    case SVt_PVGV:
	if (!isGV_with_GP(sv))
	    break;
    case SVt_PVAV:
    case SVt_PVHV:
    case SVt_PVCV:
    case SVt_PVFM:
    case SVt_PVIO:
	/* diag_listed_as: Can't coerce %s to %s in %s */
	Perl_croak(aTHX_ "Can't coerce %s to integer in %s", sv_reftype(sv,0),
		   OP_DESC(PL_op));
    default: NOOP;
    }
    (void)SvIOK_only(sv);			/* validate number */
    SvIV_set(sv, i);
    SvTAINT(sv);
}

/*
=for apidoc sv_setiv_mg

Like C<sv_setiv>, but also handles 'set' magic.

=cut
*/

void
Perl_sv_setiv_mg(pTHX_ register SV *const sv, const IV i)
{
    PERL_ARGS_ASSERT_SV_SETIV_MG;

    sv_setiv(sv,i);
    SvSETMAGIC(sv);
}

/*
=for apidoc sv_setuv

Copies an unsigned integer into the given SV, upgrading first if necessary.
Does not handle 'set' magic.  See also C<sv_setuv_mg>.

=cut
*/

void
Perl_sv_setuv(pTHX_ register SV *const sv, const UV u)
{
    PERL_ARGS_ASSERT_SV_SETUV;

    /* With the if statement to ensure that integers are stored as IVs whenever
       possible:
       u=1.49  s=0.52  cu=72.49  cs=10.64  scripts=270  tests=20865

       without
       u=1.35  s=0.47  cu=73.45  cs=11.43  scripts=270  tests=20865

       If you wish to remove the following if statement, so that this routine
       (and its callers) always return UVs, please benchmark to see what the
       effect is. Modern CPUs may be different. Or may not :-)
    */
    if (u <= (UV)IV_MAX) {
       sv_setiv(sv, (IV)u);
       return;
    }
    sv_setiv(sv, 0);
    SvIsUV_on(sv);
    SvUV_set(sv, u);
}

/*
=for apidoc sv_setuv_mg

Like C<sv_setuv>, but also handles 'set' magic.

=cut
*/

void
Perl_sv_setuv_mg(pTHX_ register SV *const sv, const UV u)
{
    PERL_ARGS_ASSERT_SV_SETUV_MG;

    sv_setuv(sv,u);
    SvSETMAGIC(sv);
}

/*
=for apidoc sv_setnv

Copies a double into the given SV, upgrading first if necessary.
Does not handle 'set' magic.  See also C<sv_setnv_mg>.

=cut
*/

void
Perl_sv_setnv(pTHX_ register SV *const sv, const NV num)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_SETNV;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    switch (SvTYPE(sv)) {
    case SVt_NULL:
    case SVt_IV:
	sv_upgrade(sv, SVt_NV);
	break;
    case SVt_PV:
    case SVt_PVIV:
	sv_upgrade(sv, SVt_PVNV);
	break;

    case SVt_PVGV:
	if (!isGV_with_GP(sv))
	    break;
    case SVt_PVAV:
    case SVt_PVHV:
    case SVt_PVCV:
    case SVt_PVFM:
    case SVt_PVIO:
	/* diag_listed_as: Can't coerce %s to %s in %s */
	Perl_croak(aTHX_ "Can't coerce %s to number in %s", sv_reftype(sv,0),
		   OP_DESC(PL_op));
    default: NOOP;
    }
    SvNV_set(sv, num);
    (void)SvNOK_only(sv);			/* validate number */
    SvTAINT(sv);
}

/*
=for apidoc sv_setnv_mg

Like C<sv_setnv>, but also handles 'set' magic.

=cut
*/

void
Perl_sv_setnv_mg(pTHX_ register SV *const sv, const NV num)
{
    PERL_ARGS_ASSERT_SV_SETNV_MG;

    sv_setnv(sv,num);
    SvSETMAGIC(sv);
}

/* Print an "isn't numeric" warning, using a cleaned-up,
 * printable version of the offending string
 */

STATIC void
S_not_a_number(pTHX_ SV *const sv)
{
     dVAR;
     SV *dsv;
     char tmpbuf[64];
     const char *pv;

     PERL_ARGS_ASSERT_NOT_A_NUMBER;

     if (DO_UTF8(sv)) {
          dsv = newSVpvs_flags("", SVs_TEMP);
          pv = sv_uni_display(dsv, sv, 10, UNI_DISPLAY_ISPRINT);
     } else {
	  char *d = tmpbuf;
	  const char * const limit = tmpbuf + sizeof(tmpbuf) - 8;
	  /* each *s can expand to 4 chars + "...\0",
	     i.e. need room for 8 chars */
	
	  const char *s = SvPVX_const(sv);
	  const char * const end = s + SvCUR(sv);
	  for ( ; s < end && d < limit; s++ ) {
	       int ch = *s & 0xFF;
	       if (ch & 128 && !isPRINT_LC(ch)) {
		    *d++ = 'M';
		    *d++ = '-';
		    ch &= 127;
	       }
	       if (ch == '\n') {
		    *d++ = '\\';
		    *d++ = 'n';
	       }
	       else if (ch == '\r') {
		    *d++ = '\\';
		    *d++ = 'r';
	       }
	       else if (ch == '\f') {
		    *d++ = '\\';
		    *d++ = 'f';
	       }
	       else if (ch == '\\') {
		    *d++ = '\\';
		    *d++ = '\\';
	       }
	       else if (ch == '\0') {
		    *d++ = '\\';
		    *d++ = '0';
	       }
	       else if (isPRINT_LC(ch))
		    *d++ = ch;
	       else {
		    *d++ = '^';
		    *d++ = toCTRL(ch);
	       }
	  }
	  if (s < end) {
	       *d++ = '.';
	       *d++ = '.';
	       *d++ = '.';
	  }
	  *d = '\0';
	  pv = tmpbuf;
    }

    if (PL_op)
	Perl_warner(aTHX_ packWARN(WARN_NUMERIC),
		    /* diag_listed_as: Argument "%s" isn't numeric%s */
		    "Argument \"%s\" isn't numeric in %s", pv,
		    OP_DESC(PL_op));
    else
	Perl_warner(aTHX_ packWARN(WARN_NUMERIC),
		    /* diag_listed_as: Argument "%s" isn't numeric%s */
		    "Argument \"%s\" isn't numeric", pv);
}

/*
=for apidoc looks_like_number

Test if the content of an SV looks like a number (or is a number).
C<Inf> and C<Infinity> are treated as numbers (so will not issue a
non-numeric warning), even if your atof() doesn't grok them.  Get-magic is
ignored.

=cut
*/

I32
Perl_looks_like_number(pTHX_ SV *const sv)
{
    register const char *sbegin;
    STRLEN len;

    PERL_ARGS_ASSERT_LOOKS_LIKE_NUMBER;

    if (SvPOK(sv) || SvPOKp(sv)) {
	sbegin = SvPV_nomg_const(sv, len);
    }
    else
	return SvFLAGS(sv) & (SVf_NOK|SVp_NOK|SVf_IOK|SVp_IOK);
    return grok_number(sbegin, len, NULL);
}

STATIC bool
S_glob_2number(pTHX_ GV * const gv)
{
    SV *const buffer = sv_newmortal();

    PERL_ARGS_ASSERT_GLOB_2NUMBER;

    gv_efullname3(buffer, gv, "*");

    /* We know that all GVs stringify to something that is not-a-number,
	so no need to test that.  */
    if (ckWARN(WARN_NUMERIC))
	not_a_number(buffer);
    /* We just want something true to return, so that S_sv_2iuv_common
	can tail call us and return true.  */
    return TRUE;
}

/* Actually, ISO C leaves conversion of UV to IV undefined, but
   until proven guilty, assume that things are not that bad... */

/*
   NV_PRESERVES_UV:

   As 64 bit platforms often have an NV that doesn't preserve all bits of
   an IV (an assumption perl has been based on to date) it becomes necessary
   to remove the assumption that the NV always carries enough precision to
   recreate the IV whenever needed, and that the NV is the canonical form.
   Instead, IV/UV and NV need to be given equal rights. So as to not lose
   precision as a side effect of conversion (which would lead to insanity
   and the dragon(s) in t/op/numconvert.t getting very angry) the intent is
   1) to distinguish between IV/UV/NV slots that have cached a valid
      conversion where precision was lost and IV/UV/NV slots that have a
      valid conversion which has lost no precision
   2) to ensure that if a numeric conversion to one form is requested that
      would lose precision, the precise conversion (or differently
      imprecise conversion) is also performed and cached, to prevent
      requests for different numeric formats on the same SV causing
      lossy conversion chains. (lossless conversion chains are perfectly
      acceptable (still))


   flags are used:
   SvIOKp is true if the IV slot contains a valid value
   SvIOK  is true only if the IV value is accurate (UV if SvIOK_UV true)
   SvNOKp is true if the NV slot contains a valid value
   SvNOK  is true only if the NV value is accurate

   so
   while converting from PV to NV, check to see if converting that NV to an
   IV(or UV) would lose accuracy over a direct conversion from PV to
   IV(or UV). If it would, cache both conversions, return NV, but mark
   SV as IOK NOKp (ie not NOK).

   While converting from PV to IV, check to see if converting that IV to an
   NV would lose accuracy over a direct conversion from PV to NV. If it
   would, cache both conversions, flag similarly.

   Before, the SV value "3.2" could become NV=3.2 IV=3 NOK, IOK quite
   correctly because if IV & NV were set NV *always* overruled.
   Now, "3.2" will become NV=3.2 IV=3 NOK, IOKp, because the flag's meaning
   changes - now IV and NV together means that the two are interchangeable:
   SvIVX == (IV) SvNVX && SvNVX == (NV) SvIVX;

   The benefit of this is that operations such as pp_add know that if
   SvIOK is true for both left and right operands, then integer addition
   can be used instead of floating point (for cases where the result won't
   overflow). Before, floating point was always used, which could lead to
   loss of precision compared with integer addition.

   * making IV and NV equal status should make maths accurate on 64 bit
     platforms
   * may speed up maths somewhat if pp_add and friends start to use
     integers when possible instead of fp. (Hopefully the overhead in
     looking for SvIOK and checking for overflow will not outweigh the
     fp to integer speedup)
   * will slow down integer operations (callers of SvIV) on "inaccurate"
     values, as the change from SvIOK to SvIOKp will cause a call into
     sv_2iv each time rather than a macro access direct to the IV slot
   * should speed up number->string conversion on integers as IV is
     favoured when IV and NV are equally accurate

   ####################################################################
   You had better be using SvIOK_notUV if you want an IV for arithmetic:
   SvIOK is true if (IV or UV), so you might be getting (IV)SvUV.
   On the other hand, SvUOK is true iff UV.
   ####################################################################

   Your mileage will vary depending your CPU's relative fp to integer
   performance ratio.
*/

#ifndef NV_PRESERVES_UV
#  define IS_NUMBER_UNDERFLOW_IV 1
#  define IS_NUMBER_UNDERFLOW_UV 2
#  define IS_NUMBER_IV_AND_UV    2
#  define IS_NUMBER_OVERFLOW_IV  4
#  define IS_NUMBER_OVERFLOW_UV  5

/* sv_2iuv_non_preserve(): private routine for use by sv_2iv() and sv_2uv() */

/* For sv_2nv these three cases are "SvNOK and don't bother casting"  */
STATIC int
S_sv_2iuv_non_preserve(pTHX_ register SV *const sv
#  ifdef DEBUGGING
		       , I32 numtype
#  endif
		       )
{
    dVAR;

    PERL_ARGS_ASSERT_SV_2IUV_NON_PRESERVE;

    DEBUG_c(PerlIO_printf(Perl_debug_log,"sv_2iuv_non '%s', IV=0x%"UVxf" NV=%"NVgf" inttype=%"UVXf"\n", SvPVX_const(sv), SvIVX(sv), SvNVX(sv), (UV)numtype));
    if (SvNVX(sv) < (NV)IV_MIN) {
	(void)SvIOKp_on(sv);
	(void)SvNOK_on(sv);
	SvIV_set(sv, IV_MIN);
	return IS_NUMBER_UNDERFLOW_IV;
    }
    if (SvNVX(sv) > (NV)UV_MAX) {
	(void)SvIOKp_on(sv);
	(void)SvNOK_on(sv);
	SvIsUV_on(sv);
	SvUV_set(sv, UV_MAX);
	return IS_NUMBER_OVERFLOW_UV;
    }
    (void)SvIOKp_on(sv);
    (void)SvNOK_on(sv);
    /* Can't use strtol etc to convert this string.  (See truth table in
       sv_2iv  */
    if (SvNVX(sv) <= (UV)IV_MAX) {
        SvIV_set(sv, I_V(SvNVX(sv)));
        if ((NV)(SvIVX(sv)) == SvNVX(sv)) {
            SvIOK_on(sv); /* Integer is precise. NOK, IOK */
        } else {
            /* Integer is imprecise. NOK, IOKp */
        }
        return SvNVX(sv) < 0 ? IS_NUMBER_UNDERFLOW_UV : IS_NUMBER_IV_AND_UV;
    }
    SvIsUV_on(sv);
    SvUV_set(sv, U_V(SvNVX(sv)));
    if ((NV)(SvUVX(sv)) == SvNVX(sv)) {
        if (SvUVX(sv) == UV_MAX) {
            /* As we know that NVs don't preserve UVs, UV_MAX cannot
               possibly be preserved by NV. Hence, it must be overflow.
               NOK, IOKp */
            return IS_NUMBER_OVERFLOW_UV;
        }
        SvIOK_on(sv); /* Integer is precise. NOK, UOK */
    } else {
        /* Integer is imprecise. NOK, IOKp */
    }
    return IS_NUMBER_OVERFLOW_IV;
}
#endif /* !NV_PRESERVES_UV*/

STATIC bool
S_sv_2iuv_common(pTHX_ SV *const sv)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_2IUV_COMMON;

    if (SvNOKp(sv)) {
	/* erm. not sure. *should* never get NOKp (without NOK) from sv_2nv
	 * without also getting a cached IV/UV from it at the same time
	 * (ie PV->NV conversion should detect loss of accuracy and cache
	 * IV or UV at same time to avoid this. */
	/* IV-over-UV optimisation - choose to cache IV if possible */

	if (SvTYPE(sv) == SVt_NV)
	    sv_upgrade(sv, SVt_PVNV);

	(void)SvIOKp_on(sv);	/* Must do this first, to clear any SvOOK */
	/* < not <= as for NV doesn't preserve UV, ((NV)IV_MAX+1) will almost
	   certainly cast into the IV range at IV_MAX, whereas the correct
	   answer is the UV IV_MAX +1. Hence < ensures that dodgy boundary
	   cases go to UV */
#if defined(NAN_COMPARE_BROKEN) && defined(Perl_isnan)
	if (Perl_isnan(SvNVX(sv))) {
	    SvUV_set(sv, 0);
	    SvIsUV_on(sv);
	    return FALSE;
	}
#endif
	if (SvNVX(sv) < (NV)IV_MAX + 0.5) {
	    SvIV_set(sv, I_V(SvNVX(sv)));
	    if (SvNVX(sv) == (NV) SvIVX(sv)
#ifndef NV_PRESERVES_UV
		&& (((UV)1 << NV_PRESERVES_UV_BITS) >
		    (UV)(SvIVX(sv) > 0 ? SvIVX(sv) : -SvIVX(sv)))
		/* Don't flag it as "accurately an integer" if the number
		   came from a (by definition imprecise) NV operation, and
		   we're outside the range of NV integer precision */
#endif
		) {
		if (SvNOK(sv))
		    SvIOK_on(sv);  /* Can this go wrong with rounding? NWC */
		else {
		    /* scalar has trailing garbage, eg "42a" */
		}
		DEBUG_c(PerlIO_printf(Perl_debug_log,
				      "0x%"UVxf" iv(%"NVgf" => %"IVdf") (precise)\n",
				      PTR2UV(sv),
				      SvNVX(sv),
				      SvIVX(sv)));

	    } else {
		/* IV not precise.  No need to convert from PV, as NV
		   conversion would already have cached IV if it detected
		   that PV->IV would be better than PV->NV->IV
		   flags already correct - don't set public IOK.  */
		DEBUG_c(PerlIO_printf(Perl_debug_log,
				      "0x%"UVxf" iv(%"NVgf" => %"IVdf") (imprecise)\n",
				      PTR2UV(sv),
				      SvNVX(sv),
				      SvIVX(sv)));
	    }
	    /* Can the above go wrong if SvIVX == IV_MIN and SvNVX < IV_MIN,
	       but the cast (NV)IV_MIN rounds to a the value less (more
	       negative) than IV_MIN which happens to be equal to SvNVX ??
	       Analogous to 0xFFFFFFFFFFFFFFFF rounding up to NV (2**64) and
	       NV rounding back to 0xFFFFFFFFFFFFFFFF, so UVX == UV(NVX) and
	       (NV)UVX == NVX are both true, but the values differ. :-(
	       Hopefully for 2s complement IV_MIN is something like
	       0x8000000000000000 which will be exact. NWC */
	}
	else {
	    SvUV_set(sv, U_V(SvNVX(sv)));
	    if (
		(SvNVX(sv) == (NV) SvUVX(sv))
#ifndef  NV_PRESERVES_UV
		/* Make sure it's not 0xFFFFFFFFFFFFFFFF */
		/*&& (SvUVX(sv) != UV_MAX) irrelevant with code below */
		&& (((UV)1 << NV_PRESERVES_UV_BITS) > SvUVX(sv))
		/* Don't flag it as "accurately an integer" if the number
		   came from a (by definition imprecise) NV operation, and
		   we're outside the range of NV integer precision */
#endif
		&& SvNOK(sv)
		)
		SvIOK_on(sv);
	    SvIsUV_on(sv);
	    DEBUG_c(PerlIO_printf(Perl_debug_log,
				  "0x%"UVxf" 2iv(%"UVuf" => %"IVdf") (as unsigned)\n",
				  PTR2UV(sv),
				  SvUVX(sv),
				  SvUVX(sv)));
	}
    }
    else if (SvPOKp(sv) && SvLEN(sv)) {
	UV value;
	const int numtype = grok_number(SvPVX_const(sv), SvCUR(sv), &value);
	/* We want to avoid a possible problem when we cache an IV/ a UV which
	   may be later translated to an NV, and the resulting NV is not
	   the same as the direct translation of the initial string
	   (eg 123.456 can shortcut to the IV 123 with atol(), but we must
	   be careful to ensure that the value with the .456 is around if the
	   NV value is requested in the future).
	
	   This means that if we cache such an IV/a UV, we need to cache the
	   NV as well.  Moreover, we trade speed for space, and do not
	   cache the NV if we are sure it's not needed.
	 */

	/* SVt_PVNV is one higher than SVt_PVIV, hence this order  */
	if ((numtype & (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT))
	     == IS_NUMBER_IN_UV) {
	    /* It's definitely an integer, only upgrade to PVIV */
	    if (SvTYPE(sv) < SVt_PVIV)
		sv_upgrade(sv, SVt_PVIV);
	    (void)SvIOK_on(sv);
	} else if (SvTYPE(sv) < SVt_PVNV)
	    sv_upgrade(sv, SVt_PVNV);

	/* If NVs preserve UVs then we only use the UV value if we know that
	   we aren't going to call atof() below. If NVs don't preserve UVs
	   then the value returned may have more precision than atof() will
	   return, even though value isn't perfectly accurate.  */
	if ((numtype & (IS_NUMBER_IN_UV
#ifdef NV_PRESERVES_UV
			| IS_NUMBER_NOT_INT
#endif
	    )) == IS_NUMBER_IN_UV) {
	    /* This won't turn off the public IOK flag if it was set above  */
	    (void)SvIOKp_on(sv);

	    if (!(numtype & IS_NUMBER_NEG)) {
		/* positive */;
		if (value <= (UV)IV_MAX) {
		    SvIV_set(sv, (IV)value);
		} else {
		    /* it didn't overflow, and it was positive. */
		    SvUV_set(sv, value);
		    SvIsUV_on(sv);
		}
	    } else {
		/* 2s complement assumption  */
		if (value <= (UV)IV_MIN) {
		    SvIV_set(sv, -(IV)value);
		} else {
		    /* Too negative for an IV.  This is a double upgrade, but
		       I'm assuming it will be rare.  */
		    if (SvTYPE(sv) < SVt_PVNV)
			sv_upgrade(sv, SVt_PVNV);
		    SvNOK_on(sv);
		    SvIOK_off(sv);
		    SvIOKp_on(sv);
		    SvNV_set(sv, -(NV)value);
		    SvIV_set(sv, IV_MIN);
		}
	    }
	}
	/* For !NV_PRESERVES_UV and IS_NUMBER_IN_UV and IS_NUMBER_NOT_INT we
           will be in the previous block to set the IV slot, and the next
           block to set the NV slot.  So no else here.  */
	
	if ((numtype & (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT))
	    != IS_NUMBER_IN_UV) {
	    /* It wasn't an (integer that doesn't overflow the UV). */
	    SvNV_set(sv, Atof(SvPVX_const(sv)));

	    if (! numtype && ckWARN(WARN_NUMERIC))
		not_a_number(sv);

#if defined(USE_LONG_DOUBLE)
	    DEBUG_c(PerlIO_printf(Perl_debug_log, "0x%"UVxf" 2iv(%" PERL_PRIgldbl ")\n",
				  PTR2UV(sv), SvNVX(sv)));
#else
	    DEBUG_c(PerlIO_printf(Perl_debug_log, "0x%"UVxf" 2iv(%"NVgf")\n",
				  PTR2UV(sv), SvNVX(sv)));
#endif

#ifdef NV_PRESERVES_UV
            (void)SvIOKp_on(sv);
            (void)SvNOK_on(sv);
            if (SvNVX(sv) < (NV)IV_MAX + 0.5) {
                SvIV_set(sv, I_V(SvNVX(sv)));
                if ((NV)(SvIVX(sv)) == SvNVX(sv)) {
                    SvIOK_on(sv);
                } else {
		    NOOP;  /* Integer is imprecise. NOK, IOKp */
                }
                /* UV will not work better than IV */
            } else {
                if (SvNVX(sv) > (NV)UV_MAX) {
                    SvIsUV_on(sv);
                    /* Integer is inaccurate. NOK, IOKp, is UV */
                    SvUV_set(sv, UV_MAX);
                } else {
                    SvUV_set(sv, U_V(SvNVX(sv)));
                    /* 0xFFFFFFFFFFFFFFFF not an issue in here, NVs
                       NV preservse UV so can do correct comparison.  */
                    if ((NV)(SvUVX(sv)) == SvNVX(sv)) {
                        SvIOK_on(sv);
                    } else {
			NOOP;   /* Integer is imprecise. NOK, IOKp, is UV */
                    }
                }
		SvIsUV_on(sv);
            }
#else /* NV_PRESERVES_UV */
            if ((numtype & (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT))
                == (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT)) {
                /* The IV/UV slot will have been set from value returned by
                   grok_number above.  The NV slot has just been set using
                   Atof.  */
	        SvNOK_on(sv);
                assert (SvIOKp(sv));
            } else {
                if (((UV)1 << NV_PRESERVES_UV_BITS) >
                    U_V(SvNVX(sv) > 0 ? SvNVX(sv) : -SvNVX(sv))) {
                    /* Small enough to preserve all bits. */
                    (void)SvIOKp_on(sv);
                    SvNOK_on(sv);
                    SvIV_set(sv, I_V(SvNVX(sv)));
                    if ((NV)(SvIVX(sv)) == SvNVX(sv))
                        SvIOK_on(sv);
                    /* Assumption: first non-preserved integer is < IV_MAX,
                       this NV is in the preserved range, therefore: */
                    if (!(U_V(SvNVX(sv) > 0 ? SvNVX(sv) : -SvNVX(sv))
                          < (UV)IV_MAX)) {
                        Perl_croak(aTHX_ "sv_2iv assumed (U_V(fabs((double)SvNVX(sv))) < (UV)IV_MAX) but SvNVX(sv)=%"NVgf" U_V is 0x%"UVxf", IV_MAX is 0x%"UVxf"\n", SvNVX(sv), U_V(SvNVX(sv)), (UV)IV_MAX);
                    }
                } else {
                    /* IN_UV NOT_INT
                         0      0	already failed to read UV.
                         0      1       already failed to read UV.
                         1      0       you won't get here in this case. IV/UV
                         	        slot set, public IOK, Atof() unneeded.
                         1      1       already read UV.
                       so there's no point in sv_2iuv_non_preserve() attempting
                       to use atol, strtol, strtoul etc.  */
#  ifdef DEBUGGING
                    sv_2iuv_non_preserve (sv, numtype);
#  else
                    sv_2iuv_non_preserve (sv);
#  endif
                }
            }
#endif /* NV_PRESERVES_UV */
	/* It might be more code efficient to go through the entire logic above
	   and conditionally set with SvIOKp_on() rather than SvIOK(), but it
	   gets complex and potentially buggy, so more programmer efficient
	   to do it this way, by turning off the public flags:  */
	if (!numtype)
	    SvFLAGS(sv) &= ~(SVf_IOK|SVf_NOK);
	}
    }
    else  {
	if (isGV_with_GP(sv))
	    return glob_2number(MUTABLE_GV(sv));

	if (!SvPADTMP(sv)) {
	    if (!PL_localizing && ckWARN(WARN_UNINITIALIZED))
		report_uninit(sv);
	}
	if (SvTYPE(sv) < SVt_IV)
	    /* Typically the caller expects that sv_any is not NULL now.  */
	    sv_upgrade(sv, SVt_IV);
	/* Return 0 from the caller.  */
	return TRUE;
    }
    return FALSE;
}

/*
=for apidoc sv_2iv_flags

Return the integer value of an SV, doing any necessary string
conversion.  If flags includes SV_GMAGIC, does an mg_get() first.
Normally used via the C<SvIV(sv)> and C<SvIVx(sv)> macros.

=cut
*/

IV
Perl_sv_2iv_flags(pTHX_ register SV *const sv, const I32 flags)
{
    dVAR;
    if (!sv)
	return 0;
    if (SvGMAGICAL(sv) || SvVALID(sv)) {
	/* FBMs use the space for SvIVX and SvNVX for other purposes, and use
	   the same flag bit as SVf_IVisUV, so must not let them cache IVs.
	   In practice they are extremely unlikely to actually get anywhere
	   accessible by user Perl code - the only way that I'm aware of is when
	   a constant subroutine which is used as the second argument to index.
	*/
	if (flags & SV_GMAGIC)
	    mg_get(sv);
	if (SvIOKp(sv))
	    return SvIVX(sv);
	if (SvNOKp(sv)) {
	    return I_V(SvNVX(sv));
	}
	if (SvPOKp(sv) && SvLEN(sv)) {
	    UV value;
	    const int numtype
		= grok_number(SvPVX_const(sv), SvCUR(sv), &value);

	    if ((numtype & (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT))
		== IS_NUMBER_IN_UV) {
		/* It's definitely an integer */
		if (numtype & IS_NUMBER_NEG) {
		    if (value < (UV)IV_MIN)
			return -(IV)value;
		} else {
		    if (value < (UV)IV_MAX)
			return (IV)value;
		}
	    }
	    if (!numtype) {
		if (ckWARN(WARN_NUMERIC))
		    not_a_number(sv);
	    }
	    return I_V(Atof(SvPVX_const(sv)));
	}
        if (SvROK(sv)) {
	    goto return_rok;
	}
	assert(SvTYPE(sv) >= SVt_PVMG);
	/* This falls through to the report_uninit inside S_sv_2iuv_common.  */
    } else if (SvTHINKFIRST(sv)) {
	if (SvROK(sv)) {
	return_rok:
	    if (SvAMAGIC(sv)) {
		SV * tmpstr;
		if (flags & SV_SKIP_OVERLOAD)
		    return 0;
		tmpstr = AMG_CALLunary(sv, numer_amg);
		if (tmpstr && (!SvROK(tmpstr) || (SvRV(tmpstr) != SvRV(sv)))) {
		    return SvIV(tmpstr);
		}
	    }
	    return PTR2IV(SvRV(sv));
	}
	if (SvIsCOW(sv)) {
	    sv_force_normal_flags(sv, 0);
	}
	if (SvREADONLY(sv) && !SvOK(sv)) {
	    if (ckWARN(WARN_UNINITIALIZED))
		report_uninit(sv);
	    return 0;
	}
    }
    if (!SvIOKp(sv)) {
	if (S_sv_2iuv_common(aTHX_ sv))
	    return 0;
    }
    DEBUG_c(PerlIO_printf(Perl_debug_log, "0x%"UVxf" 2iv(%"IVdf")\n",
	PTR2UV(sv),SvIVX(sv)));
    return SvIsUV(sv) ? (IV)SvUVX(sv) : SvIVX(sv);
}

/*
=for apidoc sv_2uv_flags

Return the unsigned integer value of an SV, doing any necessary string
conversion.  If flags includes SV_GMAGIC, does an mg_get() first.
Normally used via the C<SvUV(sv)> and C<SvUVx(sv)> macros.

=cut
*/

UV
Perl_sv_2uv_flags(pTHX_ register SV *const sv, const I32 flags)
{
    dVAR;
    if (!sv)
	return 0;
    if (SvGMAGICAL(sv) || SvVALID(sv)) {
	/* FBMs use the space for SvIVX and SvNVX for other purposes, and use
	   the same flag bit as SVf_IVisUV, so must not let them cache IVs.  */
	if (flags & SV_GMAGIC)
	    mg_get(sv);
	if (SvIOKp(sv))
	    return SvUVX(sv);
	if (SvNOKp(sv))
	    return U_V(SvNVX(sv));
	if (SvPOKp(sv) && SvLEN(sv)) {
	    UV value;
	    const int numtype
		= grok_number(SvPVX_const(sv), SvCUR(sv), &value);

	    if ((numtype & (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT))
		== IS_NUMBER_IN_UV) {
		/* It's definitely an integer */
		if (!(numtype & IS_NUMBER_NEG))
		    return value;
	    }
	    if (!numtype) {
		if (ckWARN(WARN_NUMERIC))
		    not_a_number(sv);
	    }
	    return U_V(Atof(SvPVX_const(sv)));
	}
        if (SvROK(sv)) {
	    goto return_rok;
	}
	assert(SvTYPE(sv) >= SVt_PVMG);
	/* This falls through to the report_uninit inside S_sv_2iuv_common.  */
    } else if (SvTHINKFIRST(sv)) {
	if (SvROK(sv)) {
	return_rok:
	    if (SvAMAGIC(sv)) {
		SV *tmpstr;
		if (flags & SV_SKIP_OVERLOAD)
		    return 0;
		tmpstr = AMG_CALLunary(sv, numer_amg);
		if (tmpstr && (!SvROK(tmpstr) || (SvRV(tmpstr) != SvRV(sv)))) {
		    return SvUV(tmpstr);
		}
	    }
	    return PTR2UV(SvRV(sv));
	}
	if (SvIsCOW(sv)) {
	    sv_force_normal_flags(sv, 0);
	}
	if (SvREADONLY(sv) && !SvOK(sv)) {
	    if (ckWARN(WARN_UNINITIALIZED))
		report_uninit(sv);
	    return 0;
	}
    }
    if (!SvIOKp(sv)) {
	if (S_sv_2iuv_common(aTHX_ sv))
	    return 0;
    }

    DEBUG_c(PerlIO_printf(Perl_debug_log, "0x%"UVxf" 2uv(%"UVuf")\n",
			  PTR2UV(sv),SvUVX(sv)));
    return SvIsUV(sv) ? SvUVX(sv) : (UV)SvIVX(sv);
}

/*
=for apidoc sv_2nv_flags

Return the num value of an SV, doing any necessary string or integer
conversion.  If flags includes SV_GMAGIC, does an mg_get() first.
Normally used via the C<SvNV(sv)> and C<SvNVx(sv)> macros.

=cut
*/

NV
Perl_sv_2nv_flags(pTHX_ register SV *const sv, const I32 flags)
{
    dVAR;
    if (!sv)
	return 0.0;
    if (SvGMAGICAL(sv) || SvVALID(sv)) {
	/* FBMs use the space for SvIVX and SvNVX for other purposes, and use
	   the same flag bit as SVf_IVisUV, so must not let them cache NVs.  */
	if (flags & SV_GMAGIC)
	    mg_get(sv);
	if (SvNOKp(sv))
	    return SvNVX(sv);
	if ((SvPOKp(sv) && SvLEN(sv)) && !SvIOKp(sv)) {
	    if (!SvIOKp(sv) && ckWARN(WARN_NUMERIC) &&
		!grok_number(SvPVX_const(sv), SvCUR(sv), NULL))
		not_a_number(sv);
	    return Atof(SvPVX_const(sv));
	}
	if (SvIOKp(sv)) {
	    if (SvIsUV(sv))
		return (NV)SvUVX(sv);
	    else
		return (NV)SvIVX(sv);
	}
        if (SvROK(sv)) {
	    goto return_rok;
	}
	assert(SvTYPE(sv) >= SVt_PVMG);
	/* This falls through to the report_uninit near the end of the
	   function. */
    } else if (SvTHINKFIRST(sv)) {
	if (SvROK(sv)) {
	return_rok:
	    if (SvAMAGIC(sv)) {
		SV *tmpstr;
		if (flags & SV_SKIP_OVERLOAD)
		    return 0;
		tmpstr = AMG_CALLunary(sv, numer_amg);
                if (tmpstr && (!SvROK(tmpstr) || (SvRV(tmpstr) != SvRV(sv)))) {
		    return SvNV(tmpstr);
		}
	    }
	    return PTR2NV(SvRV(sv));
	}
	if (SvIsCOW(sv)) {
	    sv_force_normal_flags(sv, 0);
	}
	if (SvREADONLY(sv) && !SvOK(sv)) {
	    if (ckWARN(WARN_UNINITIALIZED))
		report_uninit(sv);
	    return 0.0;
	}
    }
    if (SvTYPE(sv) < SVt_NV) {
	/* The logic to use SVt_PVNV if necessary is in sv_upgrade.  */
	sv_upgrade(sv, SVt_NV);
#ifdef USE_LONG_DOUBLE
	DEBUG_c({
	    STORE_NUMERIC_LOCAL_SET_STANDARD();
	    PerlIO_printf(Perl_debug_log,
			  "0x%"UVxf" num(%" PERL_PRIgldbl ")\n",
			  PTR2UV(sv), SvNVX(sv));
	    RESTORE_NUMERIC_LOCAL();
	});
#else
	DEBUG_c({
	    STORE_NUMERIC_LOCAL_SET_STANDARD();
	    PerlIO_printf(Perl_debug_log, "0x%"UVxf" num(%"NVgf")\n",
			  PTR2UV(sv), SvNVX(sv));
	    RESTORE_NUMERIC_LOCAL();
	});
#endif
    }
    else if (SvTYPE(sv) < SVt_PVNV)
	sv_upgrade(sv, SVt_PVNV);
    if (SvNOKp(sv)) {
        return SvNVX(sv);
    }
    if (SvIOKp(sv)) {
	SvNV_set(sv, SvIsUV(sv) ? (NV)SvUVX(sv) : (NV)SvIVX(sv));
#ifdef NV_PRESERVES_UV
	if (SvIOK(sv))
	    SvNOK_on(sv);
	else
	    SvNOKp_on(sv);
#else
	/* Only set the public NV OK flag if this NV preserves the IV  */
	/* Check it's not 0xFFFFFFFFFFFFFFFF */
	if (SvIOK(sv) &&
	    SvIsUV(sv) ? ((SvUVX(sv) != UV_MAX)&&(SvUVX(sv) == U_V(SvNVX(sv))))
		       : (SvIVX(sv) == I_V(SvNVX(sv))))
	    SvNOK_on(sv);
	else
	    SvNOKp_on(sv);
#endif
    }
    else if (SvPOKp(sv) && SvLEN(sv)) {
	UV value;
	const int numtype = grok_number(SvPVX_const(sv), SvCUR(sv), &value);
	if (!SvIOKp(sv) && !numtype && ckWARN(WARN_NUMERIC))
	    not_a_number(sv);
#ifdef NV_PRESERVES_UV
	if ((numtype & (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT))
	    == IS_NUMBER_IN_UV) {
	    /* It's definitely an integer */
	    SvNV_set(sv, (numtype & IS_NUMBER_NEG) ? -(NV)value : (NV)value);
	} else
	    SvNV_set(sv, Atof(SvPVX_const(sv)));
	if (numtype)
	    SvNOK_on(sv);
	else
	    SvNOKp_on(sv);
#else
	SvNV_set(sv, Atof(SvPVX_const(sv)));
	/* Only set the public NV OK flag if this NV preserves the value in
	   the PV at least as well as an IV/UV would.
	   Not sure how to do this 100% reliably. */
	/* if that shift count is out of range then Configure's test is
	   wonky. We shouldn't be in here with NV_PRESERVES_UV_BITS ==
	   UV_BITS */
	if (((UV)1 << NV_PRESERVES_UV_BITS) >
	    U_V(SvNVX(sv) > 0 ? SvNVX(sv) : -SvNVX(sv))) {
	    SvNOK_on(sv); /* Definitely small enough to preserve all bits */
	} else if (!(numtype & IS_NUMBER_IN_UV)) {
            /* Can't use strtol etc to convert this string, so don't try.
               sv_2iv and sv_2uv will use the NV to convert, not the PV.  */
            SvNOK_on(sv);
        } else {
            /* value has been set.  It may not be precise.  */
	    if ((numtype & IS_NUMBER_NEG) && (value > (UV)IV_MIN)) {
		/* 2s complement assumption for (UV)IV_MIN  */
                SvNOK_on(sv); /* Integer is too negative.  */
            } else {
                SvNOKp_on(sv);
                SvIOKp_on(sv);

                if (numtype & IS_NUMBER_NEG) {
                    SvIV_set(sv, -(IV)value);
                } else if (value <= (UV)IV_MAX) {
		    SvIV_set(sv, (IV)value);
		} else {
		    SvUV_set(sv, value);
		    SvIsUV_on(sv);
		}

                if (numtype & IS_NUMBER_NOT_INT) {
                    /* I believe that even if the original PV had decimals,
                       they are lost beyond the limit of the FP precision.
                       However, neither is canonical, so both only get p
                       flags.  NWC, 2000/11/25 */
                    /* Both already have p flags, so do nothing */
                } else {
		    const NV nv = SvNVX(sv);
                    if (SvNVX(sv) < (NV)IV_MAX + 0.5) {
                        if (SvIVX(sv) == I_V(nv)) {
                            SvNOK_on(sv);
                        } else {
                            /* It had no "." so it must be integer.  */
                        }
			SvIOK_on(sv);
                    } else {
                        /* between IV_MAX and NV(UV_MAX).
                           Could be slightly > UV_MAX */

                        if (numtype & IS_NUMBER_NOT_INT) {
                            /* UV and NV both imprecise.  */
                        } else {
			    const UV nv_as_uv = U_V(nv);

                            if (value == nv_as_uv && SvUVX(sv) != UV_MAX) {
                                SvNOK_on(sv);
                            }
			    SvIOK_on(sv);
                        }
                    }
                }
            }
        }
	/* It might be more code efficient to go through the entire logic above
	   and conditionally set with SvNOKp_on() rather than SvNOK(), but it
	   gets complex and potentially buggy, so more programmer efficient
	   to do it this way, by turning off the public flags:  */
	if (!numtype)
	    SvFLAGS(sv) &= ~(SVf_IOK|SVf_NOK);
#endif /* NV_PRESERVES_UV */
    }
    else  {
	if (isGV_with_GP(sv)) {
	    glob_2number(MUTABLE_GV(sv));
	    return 0.0;
	}

	if (!PL_localizing && !SvPADTMP(sv) && ckWARN(WARN_UNINITIALIZED))
	    report_uninit(sv);
	assert (SvTYPE(sv) >= SVt_NV);
	/* Typically the caller expects that sv_any is not NULL now.  */
	/* XXX Ilya implies that this is a bug in callers that assume this
	   and ideally should be fixed.  */
	return 0.0;
    }
#if defined(USE_LONG_DOUBLE)
    DEBUG_c({
	STORE_NUMERIC_LOCAL_SET_STANDARD();
	PerlIO_printf(Perl_debug_log, "0x%"UVxf" 2nv(%" PERL_PRIgldbl ")\n",
		      PTR2UV(sv), SvNVX(sv));
	RESTORE_NUMERIC_LOCAL();
    });
#else
    DEBUG_c({
	STORE_NUMERIC_LOCAL_SET_STANDARD();
	PerlIO_printf(Perl_debug_log, "0x%"UVxf" 1nv(%"NVgf")\n",
		      PTR2UV(sv), SvNVX(sv));
	RESTORE_NUMERIC_LOCAL();
    });
#endif
    return SvNVX(sv);
}

/*
=for apidoc sv_2num

Return an SV with the numeric value of the source SV, doing any necessary
reference or overload conversion.  You must use the C<SvNUM(sv)> macro to
access this function.

=cut
*/

SV *
Perl_sv_2num(pTHX_ register SV *const sv)
{
    PERL_ARGS_ASSERT_SV_2NUM;

    if (!SvROK(sv))
	return sv;
    if (SvAMAGIC(sv)) {
	SV * const tmpsv = AMG_CALLunary(sv, numer_amg);
	TAINT_IF(tmpsv && SvTAINTED(tmpsv));
	if (tmpsv && (!SvROK(tmpsv) || (SvRV(tmpsv) != SvRV(sv))))
	    return sv_2num(tmpsv);
    }
    return sv_2mortal(newSVuv(PTR2UV(SvRV(sv))));
}

/* uiv_2buf(): private routine for use by sv_2pv_flags(): print an IV or
 * UV as a string towards the end of buf, and return pointers to start and
 * end of it.
 *
 * We assume that buf is at least TYPE_CHARS(UV) long.
 */

static char *
S_uiv_2buf(char *const buf, const IV iv, UV uv, const int is_uv, char **const peob)
{
    char *ptr = buf + TYPE_CHARS(UV);
    char * const ebuf = ptr;
    int sign;

    PERL_ARGS_ASSERT_UIV_2BUF;

    if (is_uv)
	sign = 0;
    else if (iv >= 0) {
	uv = iv;
	sign = 0;
    } else {
	uv = -iv;
	sign = 1;
    }
    do {
	*--ptr = '0' + (char)(uv % 10);
    } while (uv /= 10);
    if (sign)
	*--ptr = '-';
    *peob = ebuf;
    return ptr;
}

/*
=for apidoc sv_2pv_flags

Returns a pointer to the string value of an SV, and sets *lp to its length.
If flags includes SV_GMAGIC, does an mg_get() first.  Coerces sv to a
string if necessary.  Normally invoked via the C<SvPV_flags> macro.
C<sv_2pv()> and C<sv_2pv_nomg> usually end up here too.

=cut
*/

char *
Perl_sv_2pv_flags(pTHX_ register SV *const sv, STRLEN *const lp, const I32 flags)
{
    dVAR;
    register char *s;

    if (!sv) {
	if (lp)
	    *lp = 0;
	return (char *)"";
    }
    if (SvGMAGICAL(sv)) {
	if (flags & SV_GMAGIC)
	    mg_get(sv);
	if (SvPOKp(sv)) {
	    if (lp)
		*lp = SvCUR(sv);
	    if (flags & SV_MUTABLE_RETURN)
		return SvPVX_mutable(sv);
	    if (flags & SV_CONST_RETURN)
		return (char *)SvPVX_const(sv);
	    return SvPVX(sv);
	}
	if (SvIOKp(sv) || SvNOKp(sv)) {
	    char tbuf[64];  /* Must fit sprintf/Gconvert of longest IV/NV */
	    STRLEN len;

	    if (SvIOKp(sv)) {
		len = SvIsUV(sv)
		    ? my_snprintf(tbuf, sizeof(tbuf), "%"UVuf, (UV)SvUVX(sv))
		    : my_snprintf(tbuf, sizeof(tbuf), "%"IVdf, (IV)SvIVX(sv));
	    } else if(SvNVX(sv) == 0.0) {
		    tbuf[0] = '0';
		    tbuf[1] = 0;
		    len = 1;
	    } else {
		Gconvert(SvNVX(sv), NV_DIG, 0, tbuf);
		len = strlen(tbuf);
	    }
	    assert(!SvROK(sv));
	    {
		dVAR;

		SvUPGRADE(sv, SVt_PV);
		if (lp)
		    *lp = len;
		s = SvGROW_mutable(sv, len + 1);
		SvCUR_set(sv, len);
		SvPOKp_on(sv);
		return (char*)memcpy(s, tbuf, len + 1);
	    }
	}
        if (SvROK(sv)) {
	    goto return_rok;
	}
	assert(SvTYPE(sv) >= SVt_PVMG);
	/* This falls through to the report_uninit near the end of the
	   function. */
    } else if (SvTHINKFIRST(sv)) {
	if (SvROK(sv)) {
	return_rok:
            if (SvAMAGIC(sv)) {
		SV *tmpstr;
		if (flags & SV_SKIP_OVERLOAD)
		    return NULL;
		tmpstr = AMG_CALLunary(sv, string_amg);
		TAINT_IF(tmpstr && SvTAINTED(tmpstr));
		if (tmpstr && (!SvROK(tmpstr) || (SvRV(tmpstr) != SvRV(sv)))) {
		    /* Unwrap this:  */
		    /* char *pv = lp ? SvPV(tmpstr, *lp) : SvPV_nolen(tmpstr);
		     */

		    char *pv;
		    if ((SvFLAGS(tmpstr) & (SVf_POK)) == SVf_POK) {
			if (flags & SV_CONST_RETURN) {
			    pv = (char *) SvPVX_const(tmpstr);
			} else {
			    pv = (flags & SV_MUTABLE_RETURN)
				? SvPVX_mutable(tmpstr) : SvPVX(tmpstr);
			}
			if (lp)
			    *lp = SvCUR(tmpstr);
		    } else {
			pv = sv_2pv_flags(tmpstr, lp, flags);
		    }
		    if (SvUTF8(tmpstr))
			SvUTF8_on(sv);
		    else
			SvUTF8_off(sv);
		    return pv;
		}
	    }
	    {
		STRLEN len;
		char *retval;
		char *buffer;
		SV *const referent = SvRV(sv);

		if (!referent) {
		    len = 7;
		    retval = buffer = savepvn("NULLREF", len);
		} else if (SvTYPE(referent) == SVt_REGEXP && (
			      !(PL_curcop->cop_hints & HINT_NO_AMAGIC)
			   || amagic_is_enabled(string_amg)
			  )) {
		    REGEXP * const re = (REGEXP *)MUTABLE_PTR(referent);
		    I32 seen_evals = 0;

		    assert(re);
			
		    /* If the regex is UTF-8 we want the containing scalar to
		       have an UTF-8 flag too */
		    if (RX_UTF8(re))
			SvUTF8_on(sv);
		    else
			SvUTF8_off(sv);	

		    if ((seen_evals = RX_SEEN_EVALS(re)))
			PL_reginterp_cnt += seen_evals;

		    if (lp)
			*lp = RX_WRAPLEN(re);
 
		    return RX_WRAPPED(re);
		} else {
		    const char *const typestr = sv_reftype(referent, 0);
		    const STRLEN typelen = strlen(typestr);
		    UV addr = PTR2UV(referent);
		    const char *stashname = NULL;
		    STRLEN stashnamelen = 0; /* hush, gcc */
		    const char *buffer_end;

		    if (SvOBJECT(referent)) {
			const HEK *const name = HvNAME_HEK(SvSTASH(referent));

			if (name) {
			    stashname = HEK_KEY(name);
			    stashnamelen = HEK_LEN(name);

			    if (HEK_UTF8(name)) {
				SvUTF8_on(sv);
			    } else {
				SvUTF8_off(sv);
			    }
			} else {
			    stashname = "__ANON__";
			    stashnamelen = 8;
			}
			len = stashnamelen + 1 /* = */ + typelen + 3 /* (0x */
			    + 2 * sizeof(UV) + 2 /* )\0 */;
		    } else {
			len = typelen + 3 /* (0x */
			    + 2 * sizeof(UV) + 2 /* )\0 */;
		    }

		    Newx(buffer, len, char);
		    buffer_end = retval = buffer + len;

		    /* Working backwards  */
		    *--retval = '\0';
		    *--retval = ')';
		    do {
			*--retval = PL_hexdigit[addr & 15];
		    } while (addr >>= 4);
		    *--retval = 'x';
		    *--retval = '0';
		    *--retval = '(';

		    retval -= typelen;
		    memcpy(retval, typestr, typelen);

		    if (stashname) {
			*--retval = '=';
			retval -= stashnamelen;
			memcpy(retval, stashname, stashnamelen);
		    }
		    /* retval may not necessarily have reached the start of the
		       buffer here.  */
		    assert (retval >= buffer);

		    len = buffer_end - retval - 1; /* -1 for that \0  */
		}
		if (lp)
		    *lp = len;
		SAVEFREEPV(buffer);
		return retval;
	    }
	}
	if (SvREADONLY(sv) && !SvOK(sv)) {
	    if (lp)
		*lp = 0;
	    if (flags & SV_UNDEF_RETURNS_NULL)
		return NULL;
	    if (ckWARN(WARN_UNINITIALIZED))
		report_uninit(sv);
	    return (char *)"";
	}
    }
    if (SvIOK(sv) || ((SvIOKp(sv) && !SvNOKp(sv)))) {
	/* I'm assuming that if both IV and NV are equally valid then
	   converting the IV is going to be more efficient */
	const U32 isUIOK = SvIsUV(sv);
	char buf[TYPE_CHARS(UV)];
	char *ebuf, *ptr;
	STRLEN len;

	if (SvTYPE(sv) < SVt_PVIV)
	    sv_upgrade(sv, SVt_PVIV);
 	ptr = uiv_2buf(buf, SvIVX(sv), SvUVX(sv), isUIOK, &ebuf);
	len = ebuf - ptr;
	/* inlined from sv_setpvn */
	s = SvGROW_mutable(sv, len + 1);
	Move(ptr, s, len, char);
	s += len;
	*s = '\0';
    }
    else if (SvNOKp(sv)) {
	if (SvTYPE(sv) < SVt_PVNV)
	    sv_upgrade(sv, SVt_PVNV);
	if (SvNVX(sv) == 0.0) {
	    s = SvGROW_mutable(sv, 2);
	    *s++ = '0';
	    *s = '\0';
	} else {
	    dSAVE_ERRNO;
	    /* The +20 is pure guesswork.  Configure test needed. --jhi */
	    s = SvGROW_mutable(sv, NV_DIG + 20);
	    /* some Xenix systems wipe out errno here */
	    Gconvert(SvNVX(sv), NV_DIG, 0, s);
	    RESTORE_ERRNO;
	    while (*s) s++;
	}
#ifdef hcx
	if (s[-1] == '.')
	    *--s = '\0';
#endif
    }
    else {
	if (isGV_with_GP(sv)) {
	    GV *const gv = MUTABLE_GV(sv);
	    SV *const buffer = sv_newmortal();

	    gv_efullname3(buffer, gv, "*");

	    assert(SvPOK(buffer));
	    if (lp) {
		    *lp = SvCUR(buffer);
	    }
            if ( SvUTF8(buffer) ) SvUTF8_on(sv);
	    return SvPVX(buffer);
	}

	if (lp)
	    *lp = 0;
	if (flags & SV_UNDEF_RETURNS_NULL)
	    return NULL;
	if (!PL_localizing && !SvPADTMP(sv) && ckWARN(WARN_UNINITIALIZED))
	    report_uninit(sv);
	if (SvTYPE(sv) < SVt_PV)
	    /* Typically the caller expects that sv_any is not NULL now.  */
	    sv_upgrade(sv, SVt_PV);
	return (char *)"";
    }
    {
	const STRLEN len = s - SvPVX_const(sv);
	if (lp) 
	    *lp = len;
	SvCUR_set(sv, len);
    }
    SvPOK_on(sv);
    DEBUG_c(PerlIO_printf(Perl_debug_log, "0x%"UVxf" 2pv(%s)\n",
			  PTR2UV(sv),SvPVX_const(sv)));
    if (flags & SV_CONST_RETURN)
	return (char *)SvPVX_const(sv);
    if (flags & SV_MUTABLE_RETURN)
	return SvPVX_mutable(sv);
    return SvPVX(sv);
}

/*
=for apidoc sv_copypv

Copies a stringified representation of the source SV into the
destination SV.  Automatically performs any necessary mg_get and
coercion of numeric values into strings.  Guaranteed to preserve
UTF8 flag even from overloaded objects.  Similar in nature to
sv_2pv[_flags] but operates directly on an SV instead of just the
string.  Mostly uses sv_2pv_flags to do its work, except when that
would lose the UTF-8'ness of the PV.

=cut
*/

void
Perl_sv_copypv(pTHX_ SV *const dsv, register SV *const ssv)
{
    STRLEN len;
    const char * const s = SvPV_const(ssv,len);

    PERL_ARGS_ASSERT_SV_COPYPV;

    sv_setpvn(dsv,s,len);
    if (SvUTF8(ssv))
	SvUTF8_on(dsv);
    else
	SvUTF8_off(dsv);
}

/*
=for apidoc sv_2pvbyte

Return a pointer to the byte-encoded representation of the SV, and set *lp
to its length.  May cause the SV to be downgraded from UTF-8 as a
side-effect.

Usually accessed via the C<SvPVbyte> macro.

=cut
*/

char *
Perl_sv_2pvbyte(pTHX_ register SV *sv, STRLEN *const lp)
{
    PERL_ARGS_ASSERT_SV_2PVBYTE;

    if ((SvTHINKFIRST(sv) && !SvIsCOW(sv)) || isGV_with_GP(sv)) {
	SV *sv2 = sv_newmortal();
	sv_copypv(sv2,sv);
	sv = sv2;
    }
    else SvGETMAGIC(sv);
    sv_utf8_downgrade(sv,0);
    return lp ? SvPV_nomg(sv,*lp) : SvPV_nomg_nolen(sv);
}

/*
=for apidoc sv_2pvutf8

Return a pointer to the UTF-8-encoded representation of the SV, and set *lp
to its length.  May cause the SV to be upgraded to UTF-8 as a side-effect.

Usually accessed via the C<SvPVutf8> macro.

=cut
*/

char *
Perl_sv_2pvutf8(pTHX_ register SV *sv, STRLEN *const lp)
{
    PERL_ARGS_ASSERT_SV_2PVUTF8;

    if ((SvTHINKFIRST(sv) && !SvIsCOW(sv)) || isGV_with_GP(sv))
	sv = sv_mortalcopy(sv);
    sv_utf8_upgrade(sv);
    if (SvGMAGICAL(sv)) SvFLAGS(sv) &= ~SVf_POK;
    assert(SvPOKp(sv));
    return lp ? SvPV_nomg(sv,*lp) : SvPV_nomg_nolen(sv);
}


/*
=for apidoc sv_2bool

This macro is only used by sv_true() or its macro equivalent, and only if
the latter's argument is neither SvPOK, SvIOK nor SvNOK.
It calls sv_2bool_flags with the SV_GMAGIC flag.

=for apidoc sv_2bool_flags

This function is only used by sv_true() and friends,  and only if
the latter's argument is neither SvPOK, SvIOK nor SvNOK.  If the flags
contain SV_GMAGIC, then it does an mg_get() first.


=cut
*/

bool
Perl_sv_2bool_flags(pTHX_ register SV *const sv, const I32 flags)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_2BOOL_FLAGS;

    if(flags & SV_GMAGIC) SvGETMAGIC(sv);

    if (!SvOK(sv))
	return 0;
    if (SvROK(sv)) {
	if (SvAMAGIC(sv)) {
	    SV * const tmpsv = AMG_CALLunary(sv, bool__amg);
	    if (tmpsv && (!SvROK(tmpsv) || (SvRV(tmpsv) != SvRV(sv))))
		return cBOOL(SvTRUE(tmpsv));
	}
	return SvRV(sv) != 0;
    }
    if (SvPOKp(sv)) {
	register XPV* const Xpvtmp = (XPV*)SvANY(sv);
	if (Xpvtmp &&
		(*sv->sv_u.svu_pv > '0' ||
		Xpvtmp->xpv_cur > 1 ||
		(Xpvtmp->xpv_cur && *sv->sv_u.svu_pv != '0')))
	    return 1;
	else
	    return 0;
    }
    else {
	if (SvIOKp(sv))
	    return SvIVX(sv) != 0;
	else {
	    if (SvNOKp(sv))
		return SvNVX(sv) != 0.0;
	    else {
		if (isGV_with_GP(sv))
		    return TRUE;
		else
		    return FALSE;
	    }
	}
    }
}

/*
=for apidoc sv_utf8_upgrade

Converts the PV of an SV to its UTF-8-encoded form.
Forces the SV to string form if it is not already.
Will C<mg_get> on C<sv> if appropriate.
Always sets the SvUTF8 flag to avoid future validity checks even
if the whole string is the same in UTF-8 as not.
Returns the number of bytes in the converted string

This is not as a general purpose byte encoding to Unicode interface:
use the Encode extension for that.

=for apidoc sv_utf8_upgrade_nomg

Like sv_utf8_upgrade, but doesn't do magic on C<sv>.

=for apidoc sv_utf8_upgrade_flags

Converts the PV of an SV to its UTF-8-encoded form.
Forces the SV to string form if it is not already.
Always sets the SvUTF8 flag to avoid future validity checks even
if all the bytes are invariant in UTF-8.
If C<flags> has C<SV_GMAGIC> bit set,
will C<mg_get> on C<sv> if appropriate, else not.
Returns the number of bytes in the converted string
C<sv_utf8_upgrade> and
C<sv_utf8_upgrade_nomg> are implemented in terms of this function.

This is not as a general purpose byte encoding to Unicode interface:
use the Encode extension for that.

=cut

The grow version is currently not externally documented.  It adds a parameter,
extra, which is the number of unused bytes the string of 'sv' is guaranteed to
have free after it upon return.  This allows the caller to reserve extra space
that it intends to fill, to avoid extra grows.

Also externally undocumented for the moment is the flag SV_FORCE_UTF8_UPGRADE,
which can be used to tell this function to not first check to see if there are
any characters that are different in UTF-8 (variant characters) which would
force it to allocate a new string to sv, but to assume there are.  Typically
this flag is used by a routine that has already parsed the string to find that
there are such characters, and passes this information on so that the work
doesn't have to be repeated.

(One might think that the calling routine could pass in the position of the
first such variant, so it wouldn't have to be found again.  But that is not the
case, because typically when the caller is likely to use this flag, it won't be
calling this routine unless it finds something that won't fit into a byte.
Otherwise it tries to not upgrade and just use bytes.  But some things that
do fit into a byte are variants in utf8, and the caller may not have been
keeping track of these.)

If the routine itself changes the string, it adds a trailing NUL.  Such a NUL
isn't guaranteed due to having other routines do the work in some input cases,
or if the input is already flagged as being in utf8.

The speed of this could perhaps be improved for many cases if someone wanted to
write a fast function that counts the number of variant characters in a string,
especially if it could return the position of the first one.

*/

STRLEN
Perl_sv_utf8_upgrade_flags_grow(pTHX_ register SV *const sv, const I32 flags, STRLEN extra)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_UTF8_UPGRADE_FLAGS_GROW;

    if (sv == &PL_sv_undef)
	return 0;
    if (!SvPOK(sv)) {
	STRLEN len = 0;
	if (SvREADONLY(sv) && (SvPOKp(sv) || SvIOKp(sv) || SvNOKp(sv))) {
	    (void) sv_2pv_flags(sv,&len, flags);
	    if (SvUTF8(sv)) {
		if (extra) SvGROW(sv, SvCUR(sv) + extra);
		return len;
	    }
	} else {
	    (void) SvPV_force_flags(sv,len,flags & SV_GMAGIC);
	}
    }

    if (SvUTF8(sv)) {
	if (extra) SvGROW(sv, SvCUR(sv) + extra);
	return SvCUR(sv);
    }

    if (SvIsCOW(sv)) {
        sv_force_normal_flags(sv, 0);
    }

    if (PL_encoding && !(flags & SV_UTF8_NO_ENCODING)) {
        sv_recode_to_utf8(sv, PL_encoding);
	if (extra) SvGROW(sv, SvCUR(sv) + extra);
	return SvCUR(sv);
    }

    if (SvCUR(sv) == 0) {
	if (extra) SvGROW(sv, extra);
    } else { /* Assume Latin-1/EBCDIC */
	/* This function could be much more efficient if we
	 * had a FLAG in SVs to signal if there are any variant
	 * chars in the PV.  Given that there isn't such a flag
	 * make the loop as fast as possible (although there are certainly ways
	 * to speed this up, eg. through vectorization) */
	U8 * s = (U8 *) SvPVX_const(sv);
	U8 * e = (U8 *) SvEND(sv);
	U8 *t = s;
	STRLEN two_byte_count = 0;
	
	if (flags & SV_FORCE_UTF8_UPGRADE) goto must_be_utf8;

	/* See if really will need to convert to utf8.  We mustn't rely on our
	 * incoming SV being well formed and having a trailing '\0', as certain
	 * code in pp_formline can send us partially built SVs. */

	while (t < e) {
	    const U8 ch = *t++;
	    if (NATIVE_IS_INVARIANT(ch)) continue;

	    t--;    /* t already incremented; re-point to first variant */
	    two_byte_count = 1;
	    goto must_be_utf8;
	}

	/* utf8 conversion not needed because all are invariants.  Mark as
	 * UTF-8 even if no variant - saves scanning loop */
	SvUTF8_on(sv);
	if (extra) SvGROW(sv, SvCUR(sv) + extra);
	return SvCUR(sv);

must_be_utf8:

	/* Here, the string should be converted to utf8, either because of an
	 * input flag (two_byte_count = 0), or because a character that
	 * requires 2 bytes was found (two_byte_count = 1).  t points either to
	 * the beginning of the string (if we didn't examine anything), or to
	 * the first variant.  In either case, everything from s to t - 1 will
	 * occupy only 1 byte each on output.
	 *
	 * There are two main ways to convert.  One is to create a new string
	 * and go through the input starting from the beginning, appending each
	 * converted value onto the new string as we go along.  It's probably
	 * best to allocate enough space in the string for the worst possible
	 * case rather than possibly running out of space and having to
	 * reallocate and then copy what we've done so far.  Since everything
	 * from s to t - 1 is invariant, the destination can be initialized
	 * with these using a fast memory copy
	 *
	 * The other way is to figure out exactly how big the string should be
	 * by parsing the entire input.  Then you don't have to make it big
	 * enough to handle the worst possible case, and more importantly, if
	 * the string you already have is large enough, you don't have to
	 * allocate a new string, you can copy the last character in the input
	 * string to the final position(s) that will be occupied by the
	 * converted string and go backwards, stopping at t, since everything
	 * before that is invariant.
	 *
	 * There are advantages and disadvantages to each method.
	 *
	 * In the first method, we can allocate a new string, do the memory
	 * copy from the s to t - 1, and then proceed through the rest of the
	 * string byte-by-byte.
	 *
	 * In the second method, we proceed through the rest of the input
	 * string just calculating how big the converted string will be.  Then
	 * there are two cases:
	 *  1)	if the string has enough extra space to handle the converted
	 *	value.  We go backwards through the string, converting until we
	 *	get to the position we are at now, and then stop.  If this
	 *	position is far enough along in the string, this method is
	 *	faster than the other method.  If the memory copy were the same
	 *	speed as the byte-by-byte loop, that position would be about
	 *	half-way, as at the half-way mark, parsing to the end and back
	 *	is one complete string's parse, the same amount as starting
	 *	over and going all the way through.  Actually, it would be
	 *	somewhat less than half-way, as it's faster to just count bytes
	 *	than to also copy, and we don't have the overhead of allocating
	 *	a new string, changing the scalar to use it, and freeing the
	 *	existing one.  But if the memory copy is fast, the break-even
	 *	point is somewhere after half way.  The counting loop could be
	 *	sped up by vectorization, etc, to move the break-even point
	 *	further towards the beginning.
	 *  2)	if the string doesn't have enough space to handle the converted
	 *	value.  A new string will have to be allocated, and one might
	 *	as well, given that, start from the beginning doing the first
	 *	method.  We've spent extra time parsing the string and in
	 *	exchange all we've gotten is that we know precisely how big to
	 *	make the new one.  Perl is more optimized for time than space,
	 *	so this case is a loser.
	 * So what I've decided to do is not use the 2nd method unless it is
	 * guaranteed that a new string won't have to be allocated, assuming
	 * the worst case.  I also decided not to put any more conditions on it
	 * than this, for now.  It seems likely that, since the worst case is
	 * twice as big as the unknown portion of the string (plus 1), we won't
	 * be guaranteed enough space, causing us to go to the first method,
	 * unless the string is short, or the first variant character is near
	 * the end of it.  In either of these cases, it seems best to use the
	 * 2nd method.  The only circumstance I can think of where this would
	 * be really slower is if the string had once had much more data in it
	 * than it does now, but there is still a substantial amount in it  */

	{
	    STRLEN invariant_head = t - s;
	    STRLEN size = invariant_head + (e - t) * 2 + 1 + extra;
	    if (SvLEN(sv) < size) {

		/* Here, have decided to allocate a new string */

		U8 *dst;
		U8 *d;

		Newx(dst, size, U8);

		/* If no known invariants at the beginning of the input string,
		 * set so starts from there.  Otherwise, can use memory copy to
		 * get up to where we are now, and then start from here */

		if (invariant_head <= 0) {
		    d = dst;
		} else {
		    Copy(s, dst, invariant_head, char);
		    d = dst + invariant_head;
		}

		while (t < e) {
		    const UV uv = NATIVE8_TO_UNI(*t++);
		    if (UNI_IS_INVARIANT(uv))
			*d++ = (U8)UNI_TO_NATIVE(uv);
		    else {
			*d++ = (U8)UTF8_EIGHT_BIT_HI(uv);
			*d++ = (U8)UTF8_EIGHT_BIT_LO(uv);
		    }
		}
		*d = '\0';
		SvPV_free(sv); /* No longer using pre-existing string */
		SvPV_set(sv, (char*)dst);
		SvCUR_set(sv, d - dst);
		SvLEN_set(sv, size);
	    } else {

		/* Here, have decided to get the exact size of the string.
		 * Currently this happens only when we know that there is
		 * guaranteed enough space to fit the converted string, so
		 * don't have to worry about growing.  If two_byte_count is 0,
		 * then t points to the first byte of the string which hasn't
		 * been examined yet.  Otherwise two_byte_count is 1, and t
		 * points to the first byte in the string that will expand to
		 * two.  Depending on this, start examining at t or 1 after t.
		 * */

		U8 *d = t + two_byte_count;


		/* Count up the remaining bytes that expand to two */

		while (d < e) {
		    const U8 chr = *d++;
		    if (! NATIVE_IS_INVARIANT(chr)) two_byte_count++;
		}

		/* The string will expand by just the number of bytes that
		 * occupy two positions.  But we are one afterwards because of
		 * the increment just above.  This is the place to put the
		 * trailing NUL, and to set the length before we decrement */

		d += two_byte_count;
		SvCUR_set(sv, d - s);
		*d-- = '\0';


		/* Having decremented d, it points to the position to put the
		 * very last byte of the expanded string.  Go backwards through
		 * the string, copying and expanding as we go, stopping when we
		 * get to the part that is invariant the rest of the way down */

		e--;
		while (e >= t) {
		    const U8 ch = NATIVE8_TO_UNI(*e--);
		    if (UNI_IS_INVARIANT(ch)) {
			*d-- = UNI_TO_NATIVE(ch);
		    } else {
			*d-- = (U8)UTF8_EIGHT_BIT_LO(ch);
			*d-- = (U8)UTF8_EIGHT_BIT_HI(ch);
		    }
		}
	    }

	    if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
		/* Update pos. We do it at the end rather than during
		 * the upgrade, to avoid slowing down the common case
		 * (upgrade without pos) */
		MAGIC * mg = mg_find(sv, PERL_MAGIC_regex_global);
		if (mg) {
		    I32 pos = mg->mg_len;
		    if (pos > 0 && (U32)pos > invariant_head) {
			U8 *d = (U8*) SvPVX(sv) + invariant_head;
			STRLEN n = (U32)pos - invariant_head;
			while (n > 0) {
			    if (UTF8_IS_START(*d))
				d++;
			    d++;
			    n--;
			}
			mg->mg_len  = d - (U8*)SvPVX(sv);
		    }
		}
		if ((mg = mg_find(sv, PERL_MAGIC_utf8)))
		    magic_setutf8(sv,mg); /* clear UTF8 cache */
	    }
	}
    }

    /* Mark as UTF-8 even if no variant - saves scanning loop */
    SvUTF8_on(sv);
    return SvCUR(sv);
}

/*
=for apidoc sv_utf8_downgrade

Attempts to convert the PV of an SV from characters to bytes.
If the PV contains a character that cannot fit
in a byte, this conversion will fail;
in this case, either returns false or, if C<fail_ok> is not
true, croaks.

This is not as a general purpose Unicode to byte encoding interface:
use the Encode extension for that.

=cut
*/

bool
Perl_sv_utf8_downgrade(pTHX_ register SV *const sv, const bool fail_ok)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_UTF8_DOWNGRADE;

    if (SvPOKp(sv) && SvUTF8(sv)) {
        if (SvCUR(sv)) {
	    U8 *s;
	    STRLEN len;
	    int mg_flags = SV_GMAGIC;

            if (SvIsCOW(sv)) {
                sv_force_normal_flags(sv, 0);
            }
	    if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
		/* update pos */
		MAGIC * mg = mg_find(sv, PERL_MAGIC_regex_global);
		if (mg) {
		    I32 pos = mg->mg_len;
		    if (pos > 0) {
			sv_pos_b2u(sv, &pos);
			mg_flags = 0; /* sv_pos_b2u does get magic */
			mg->mg_len  = pos;
		    }
		}
		if ((mg = mg_find(sv, PERL_MAGIC_utf8)))
		    magic_setutf8(sv,mg); /* clear UTF8 cache */

	    }
	    s = (U8 *) SvPV_flags(sv, len, mg_flags);

	    if (!utf8_to_bytes(s, &len)) {
	        if (fail_ok)
		    return FALSE;
		else {
		    if (PL_op)
		        Perl_croak(aTHX_ "Wide character in %s",
				   OP_DESC(PL_op));
		    else
		        Perl_croak(aTHX_ "Wide character");
		}
	    }
	    SvCUR_set(sv, len);
	}
    }
    SvUTF8_off(sv);
    return TRUE;
}

/*
=for apidoc sv_utf8_encode

Converts the PV of an SV to UTF-8, but then turns the C<SvUTF8>
flag off so that it looks like octets again.

=cut
*/

void
Perl_sv_utf8_encode(pTHX_ register SV *const sv)
{
    PERL_ARGS_ASSERT_SV_UTF8_ENCODE;

    if (SvREADONLY(sv)) {
	sv_force_normal_flags(sv, 0);
    }
    (void) sv_utf8_upgrade(sv);
    SvUTF8_off(sv);
}

/*
=for apidoc sv_utf8_decode

If the PV of the SV is an octet sequence in UTF-8
and contains a multiple-byte character, the C<SvUTF8> flag is turned on
so that it looks like a character.  If the PV contains only single-byte
characters, the C<SvUTF8> flag stays off.
Scans PV for validity and returns false if the PV is invalid UTF-8.

=cut
*/

bool
Perl_sv_utf8_decode(pTHX_ register SV *const sv)
{
    PERL_ARGS_ASSERT_SV_UTF8_DECODE;

    if (SvPOKp(sv)) {
        const U8 *start, *c;
        const U8 *e;

	/* The octets may have got themselves encoded - get them back as
	 * bytes
	 */
	if (!sv_utf8_downgrade(sv, TRUE))
	    return FALSE;

        /* it is actually just a matter of turning the utf8 flag on, but
         * we want to make sure everything inside is valid utf8 first.
         */
        c = start = (const U8 *) SvPVX_const(sv);
	if (!is_utf8_string(c, SvCUR(sv)))
	    return FALSE;
        e = (const U8 *) SvEND(sv);
        while (c < e) {
	    const U8 ch = *c++;
            if (!UTF8_IS_INVARIANT(ch)) {
		SvUTF8_on(sv);
		break;
	    }
        }
	if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
	    /* adjust pos to the start of a UTF8 char sequence */
	    MAGIC * mg = mg_find(sv, PERL_MAGIC_regex_global);
	    if (mg) {
		I32 pos = mg->mg_len;
		if (pos > 0) {
		    for (c = start + pos; c > start; c--) {
			if (UTF8_IS_START(*c))
			    break;
		    }
		    mg->mg_len  = c - start;
		}
	    }
	    if ((mg = mg_find(sv, PERL_MAGIC_utf8)))
		magic_setutf8(sv,mg); /* clear UTF8 cache */
	}
    }
    return TRUE;
}

/*
=for apidoc sv_setsv

Copies the contents of the source SV C<ssv> into the destination SV
C<dsv>.  The source SV may be destroyed if it is mortal, so don't use this
function if the source SV needs to be reused.  Does not handle 'set' magic.
Loosely speaking, it performs a copy-by-value, obliterating any previous
content of the destination.

You probably want to use one of the assortment of wrappers, such as
C<SvSetSV>, C<SvSetSV_nosteal>, C<SvSetMagicSV> and
C<SvSetMagicSV_nosteal>.

=for apidoc sv_setsv_flags

Copies the contents of the source SV C<ssv> into the destination SV
C<dsv>.  The source SV may be destroyed if it is mortal, so don't use this
function if the source SV needs to be reused.  Does not handle 'set' magic.
Loosely speaking, it performs a copy-by-value, obliterating any previous
content of the destination.
If the C<flags> parameter has the C<SV_GMAGIC> bit set, will C<mg_get> on
C<ssv> if appropriate, else not.  If the C<flags>
parameter has the C<NOSTEAL> bit set then the
buffers of temps will not be stolen.  <sv_setsv>
and C<sv_setsv_nomg> are implemented in terms of this function.

You probably want to use one of the assortment of wrappers, such as
C<SvSetSV>, C<SvSetSV_nosteal>, C<SvSetMagicSV> and
C<SvSetMagicSV_nosteal>.

This is the primary function for copying scalars, and most other
copy-ish functions and macros use this underneath.

=cut
*/

static void
S_glob_assign_glob(pTHX_ SV *const dstr, SV *const sstr, const int dtype)
{
    I32 mro_changes = 0; /* 1 = method, 2 = isa, 3 = recursive isa */
    HV *old_stash = NULL;

    PERL_ARGS_ASSERT_GLOB_ASSIGN_GLOB;

    if (dtype != SVt_PVGV && !isGV_with_GP(dstr)) {
	const char * const name = GvNAME(sstr);
	const STRLEN len = GvNAMELEN(sstr);
	{
	    if (dtype >= SVt_PV) {
		SvPV_free(dstr);
		SvPV_set(dstr, 0);
		SvLEN_set(dstr, 0);
		SvCUR_set(dstr, 0);
	    }
	    SvUPGRADE(dstr, SVt_PVGV);
	    (void)SvOK_off(dstr);
	    /* We have to turn this on here, even though we turn it off
	       below, as GvSTASH will fail an assertion otherwise. */
	    isGV_with_GP_on(dstr);
	}
	GvSTASH(dstr) = GvSTASH(sstr);
	if (GvSTASH(dstr))
	    Perl_sv_add_backref(aTHX_ MUTABLE_SV(GvSTASH(dstr)), dstr);
        gv_name_set(MUTABLE_GV(dstr), name, len,
                        GV_ADD | (GvNAMEUTF8(sstr) ? SVf_UTF8 : 0 ));
	SvFAKE_on(dstr);	/* can coerce to non-glob */
    }

    if(GvGP(MUTABLE_GV(sstr))) {
        /* If source has method cache entry, clear it */
        if(GvCVGEN(sstr)) {
            SvREFCNT_dec(GvCV(sstr));
            GvCV_set(sstr, NULL);
            GvCVGEN(sstr) = 0;
        }
        /* If source has a real method, then a method is
           going to change */
        else if(
         GvCV((const GV *)sstr) && GvSTASH(dstr) && HvENAME(GvSTASH(dstr))
        ) {
            mro_changes = 1;
        }
    }

    /* If dest already had a real method, that's a change as well */
    if(
        !mro_changes && GvGP(MUTABLE_GV(dstr)) && GvCVu((const GV *)dstr)
     && GvSTASH(dstr) && HvENAME(GvSTASH(dstr))
    ) {
        mro_changes = 1;
    }

    /* We don't need to check the name of the destination if it was not a
       glob to begin with. */
    if(dtype == SVt_PVGV) {
        const char * const name = GvNAME((const GV *)dstr);
        if(
            strEQ(name,"ISA")
         /* The stash may have been detached from the symbol table, so
            check its name. */
         && GvSTASH(dstr) && HvENAME(GvSTASH(dstr))
         && GvAV((const GV *)sstr)
        )
            mro_changes = 2;
        else {
            const STRLEN len = GvNAMELEN(dstr);
            if ((len > 1 && name[len-2] == ':' && name[len-1] == ':')
             || (len == 1 && name[0] == ':')) {
                mro_changes = 3;

                /* Set aside the old stash, so we can reset isa caches on
                   its subclasses. */
                if((old_stash = GvHV(dstr)))
                    /* Make sure we do not lose it early. */
                    SvREFCNT_inc_simple_void_NN(
                     sv_2mortal((SV *)old_stash)
                    );
            }
        }
    }

    gp_free(MUTABLE_GV(dstr));
    isGV_with_GP_off(dstr); /* SvOK_off does not like globs. */
    (void)SvOK_off(dstr);
    isGV_with_GP_on(dstr);
    GvINTRO_off(dstr);		/* one-shot flag */
    GvGP_set(dstr, gp_ref(GvGP(sstr)));
    if (SvTAINTED(sstr))
	SvTAINT(dstr);
    if (GvIMPORTED(dstr) != GVf_IMPORTED
	&& CopSTASH_ne(PL_curcop, GvSTASH(dstr)))
	{
	    GvIMPORTED_on(dstr);
	}
    GvMULTI_on(dstr);
    if(mro_changes == 2) {
	MAGIC *mg;
	SV * const sref = (SV *)GvAV((const GV *)dstr);
	if (SvSMAGICAL(sref) && (mg = mg_find(sref, PERL_MAGIC_isa))) {
	    if (SvTYPE(mg->mg_obj) != SVt_PVAV) {
		AV * const ary = newAV();
		av_push(ary, mg->mg_obj); /* takes the refcount */
		mg->mg_obj = (SV *)ary;
	    }
	    av_push((AV *)mg->mg_obj, SvREFCNT_inc_simple_NN(dstr));
	}
	else sv_magic(sref, dstr, PERL_MAGIC_isa, NULL, 0);
	mro_isa_changed_in(GvSTASH(dstr));
    }
    else if(mro_changes == 3) {
	HV * const stash = GvHV(dstr);
	if(old_stash ? (HV *)HvENAME_get(old_stash) : stash)
	    mro_package_moved(
		stash, old_stash,
		(GV *)dstr, 0
	    );
    }
    else if(mro_changes) mro_method_changed_in(GvSTASH(dstr));
    return;
}

static void
S_glob_assign_ref(pTHX_ SV *const dstr, SV *const sstr)
{
    SV * const sref = SvREFCNT_inc(SvRV(sstr));
    SV *dref = NULL;
    const int intro = GvINTRO(dstr);
    SV **location;
    U8 import_flag = 0;
    const U32 stype = SvTYPE(sref);

    PERL_ARGS_ASSERT_GLOB_ASSIGN_REF;

    if (intro) {
	GvINTRO_off(dstr);	/* one-shot flag */
	GvLINE(dstr) = CopLINE(PL_curcop);
	GvEGV(dstr) = MUTABLE_GV(dstr);
    }
    GvMULTI_on(dstr);
    switch (stype) {
    case SVt_PVCV:
	location = (SV **) &(GvGP(dstr)->gp_cv); /* XXX bypassing GvCV_set */
	import_flag = GVf_IMPORTED_CV;
	goto common;
    case SVt_PVHV:
	location = (SV **) &GvHV(dstr);
	import_flag = GVf_IMPORTED_HV;
	goto common;
    case SVt_PVAV:
	location = (SV **) &GvAV(dstr);
	import_flag = GVf_IMPORTED_AV;
	goto common;
    case SVt_PVIO:
	location = (SV **) &GvIOp(dstr);
	goto common;
    case SVt_PVFM:
	location = (SV **) &GvFORM(dstr);
	goto common;
    default:
	location = &GvSV(dstr);
	import_flag = GVf_IMPORTED_SV;
    common:
	if (intro) {
	    if (stype == SVt_PVCV) {
		/*if (GvCVGEN(dstr) && (GvCV(dstr) != (const CV *)sref || GvCVGEN(dstr))) {*/
		if (GvCVGEN(dstr)) {
		    SvREFCNT_dec(GvCV(dstr));
		    GvCV_set(dstr, NULL);
		    GvCVGEN(dstr) = 0; /* Switch off cacheness. */
		}
	    }
	    SAVEGENERICSV(*location);
	}
	else
	    dref = *location;
	if (stype == SVt_PVCV && (*location != sref || GvCVGEN(dstr))) {
	    CV* const cv = MUTABLE_CV(*location);
	    if (cv) {
		if (!GvCVGEN((const GV *)dstr) &&
		    (CvROOT(cv) || CvXSUB(cv)) &&
		    /* redundant check that avoids creating the extra SV
		       most of the time: */
		    (CvCONST(cv) || ckWARN(WARN_REDEFINE)))
		    {
			SV * const new_const_sv =
			    CvCONST((const CV *)sref)
				 ? cv_const_sv((const CV *)sref)
				 : NULL;
			report_redefined_cv(
			   sv_2mortal(Perl_newSVpvf(aTHX_
				"%"HEKf"::%"HEKf,
				HEKfARG(
				 HvNAME_HEK(GvSTASH((const GV *)dstr))
				),
				HEKfARG(GvENAME_HEK(MUTABLE_GV(dstr)))
			   )),
			   cv,
			   CvCONST((const CV *)sref) ? &new_const_sv : NULL
			);
		    }
		if (!intro)
		    cv_ckproto_len_flags(cv, (const GV *)dstr,
				   SvPOK(sref) ? CvPROTO(sref) : NULL,
				   SvPOK(sref) ? CvPROTOLEN(sref) : 0,
                                   SvPOK(sref) ? SvUTF8(sref) : 0);
	    }
	    GvCVGEN(dstr) = 0; /* Switch off cacheness. */
	    GvASSUMECV_on(dstr);
	    if(GvSTASH(dstr)) mro_method_changed_in(GvSTASH(dstr)); /* sub foo { 1 } sub bar { 2 } *bar = \&foo */
	}
	*location = sref;
	if (import_flag && !(GvFLAGS(dstr) & import_flag)
	    && CopSTASH_ne(PL_curcop, GvSTASH(dstr))) {
	    GvFLAGS(dstr) |= import_flag;
	}
	if (stype == SVt_PVHV) {
	    const char * const name = GvNAME((GV*)dstr);
	    const STRLEN len = GvNAMELEN(dstr);
	    if (
	        (
	           (len > 1 && name[len-2] == ':' && name[len-1] == ':')
	        || (len == 1 && name[0] == ':')
	        )
	     && (!dref || HvENAME_get(dref))
	    ) {
		mro_package_moved(
		    (HV *)sref, (HV *)dref,
		    (GV *)dstr, 0
		);
	    }
	}
	else if (
	    stype == SVt_PVAV && sref != dref
	 && strEQ(GvNAME((GV*)dstr), "ISA")
	 /* The stash may have been detached from the symbol table, so
	    check its name before doing anything. */
	 && GvSTASH(dstr) && HvENAME(GvSTASH(dstr))
	) {
	    MAGIC *mg;
	    MAGIC * const omg = dref && SvSMAGICAL(dref)
	                         ? mg_find(dref, PERL_MAGIC_isa)
	                         : NULL;
	    if (SvSMAGICAL(sref) && (mg = mg_find(sref, PERL_MAGIC_isa))) {
		if (SvTYPE(mg->mg_obj) != SVt_PVAV) {
		    AV * const ary = newAV();
		    av_push(ary, mg->mg_obj); /* takes the refcount */
		    mg->mg_obj = (SV *)ary;
		}
		if (omg) {
		    if (SvTYPE(omg->mg_obj) == SVt_PVAV) {
			SV **svp = AvARRAY((AV *)omg->mg_obj);
			I32 items = AvFILLp((AV *)omg->mg_obj) + 1;
			while (items--)
			    av_push(
			     (AV *)mg->mg_obj,
			     SvREFCNT_inc_simple_NN(*svp++)
			    );
		    }
		    else
			av_push(
			 (AV *)mg->mg_obj,
			 SvREFCNT_inc_simple_NN(omg->mg_obj)
			);
		}
		else
		    av_push((AV *)mg->mg_obj,SvREFCNT_inc_simple_NN(dstr));
	    }
	    else
	    {
		sv_magic(
		 sref, omg ? omg->mg_obj : dstr, PERL_MAGIC_isa, NULL, 0
		);
		mg = mg_find(sref, PERL_MAGIC_isa);
	    }
	    /* Since the *ISA assignment could have affected more than
	       one stash, don't call mro_isa_changed_in directly, but let
	       magic_clearisa do it for us, as it already has the logic for
	       dealing with globs vs arrays of globs. */
	    assert(mg);
	    Perl_magic_clearisa(aTHX_ NULL, mg);
	}
	break;
    }
    SvREFCNT_dec(dref);
    if (SvTAINTED(sstr))
	SvTAINT(dstr);
    return;
}

void
Perl_sv_setsv_flags(pTHX_ SV *dstr, register SV* sstr, const I32 flags)
{
    dVAR;
    register U32 sflags;
    register int dtype;
    register svtype stype;

    PERL_ARGS_ASSERT_SV_SETSV_FLAGS;

    if (sstr == dstr)
	return;

    if (SvIS_FREED(dstr)) {
	Perl_croak(aTHX_ "panic: attempt to copy value %" SVf
		   " to a freed scalar %p", SVfARG(sstr), (void *)dstr);
    }
    SV_CHECK_THINKFIRST_COW_DROP(dstr);
    if (!sstr)
	sstr = &PL_sv_undef;
    if (SvIS_FREED(sstr)) {
	Perl_croak(aTHX_ "panic: attempt to copy freed scalar %p to %p",
		   (void*)sstr, (void*)dstr);
    }
    stype = SvTYPE(sstr);
    dtype = SvTYPE(dstr);

    (void)SvAMAGIC_off(dstr);
    if ( SvVOK(dstr) )
    {
	/* need to nuke the magic */
	sv_unmagic(dstr, PERL_MAGIC_vstring);
    }

    /* There's a lot of redundancy below but we're going for speed here */

    switch (stype) {
    case SVt_NULL:
      undef_sstr:
	if (dtype != SVt_PVGV && dtype != SVt_PVLV) {
	    (void)SvOK_off(dstr);
	    return;
	}
	break;
    case SVt_IV:
	if (SvIOK(sstr)) {
	    switch (dtype) {
	    case SVt_NULL:
		sv_upgrade(dstr, SVt_IV);
		break;
	    case SVt_NV:
	    case SVt_PV:
		sv_upgrade(dstr, SVt_PVIV);
		break;
	    case SVt_PVGV:
	    case SVt_PVLV:
		goto end_of_first_switch;
	    }
	    (void)SvIOK_only(dstr);
	    SvIV_set(dstr,  SvIVX(sstr));
	    if (SvIsUV(sstr))
		SvIsUV_on(dstr);
	    /* SvTAINTED can only be true if the SV has taint magic, which in
	       turn means that the SV type is PVMG (or greater). This is the
	       case statement for SVt_IV, so this cannot be true (whatever gcov
	       may say).  */
	    assert(!SvTAINTED(sstr));
	    return;
	}
	if (!SvROK(sstr))
	    goto undef_sstr;
	if (dtype < SVt_PV && dtype != SVt_IV)
	    sv_upgrade(dstr, SVt_IV);
	break;

    case SVt_NV:
	if (SvNOK(sstr)) {
	    switch (dtype) {
	    case SVt_NULL:
	    case SVt_IV:
		sv_upgrade(dstr, SVt_NV);
		break;
	    case SVt_PV:
	    case SVt_PVIV:
		sv_upgrade(dstr, SVt_PVNV);
		break;
	    case SVt_PVGV:
	    case SVt_PVLV:
		goto end_of_first_switch;
	    }
	    SvNV_set(dstr, SvNVX(sstr));
	    (void)SvNOK_only(dstr);
	    /* SvTAINTED can only be true if the SV has taint magic, which in
	       turn means that the SV type is PVMG (or greater). This is the
	       case statement for SVt_NV, so this cannot be true (whatever gcov
	       may say).  */
	    assert(!SvTAINTED(sstr));
	    return;
	}
	goto undef_sstr;

    case SVt_PVFM:
#ifdef PERL_OLD_COPY_ON_WRITE
	if ((SvFLAGS(sstr) & CAN_COW_MASK) == CAN_COW_FLAGS) {
	    if (dtype < SVt_PVIV)
		sv_upgrade(dstr, SVt_PVIV);
	    break;
	}
	/* Fall through */
#endif
    case SVt_PV:
	if (dtype < SVt_PV)
	    sv_upgrade(dstr, SVt_PV);
	break;
    case SVt_PVIV:
	if (dtype < SVt_PVIV)
	    sv_upgrade(dstr, SVt_PVIV);
	break;
    case SVt_PVNV:
	if (dtype < SVt_PVNV)
	    sv_upgrade(dstr, SVt_PVNV);
	break;
    default:
	{
	const char * const type = sv_reftype(sstr,0);
	if (PL_op)
	    /* diag_listed_as: Bizarre copy of %s */
	    Perl_croak(aTHX_ "Bizarre copy of %s in %s", type, OP_DESC(PL_op));
	else
	    Perl_croak(aTHX_ "Bizarre copy of %s", type);
	}
	break;

    case SVt_REGEXP:
	if (dtype < SVt_REGEXP)
	    sv_upgrade(dstr, SVt_REGEXP);
	break;

	/* case SVt_BIND: */
    case SVt_PVLV:
    case SVt_PVGV:
    case SVt_PVMG:
	if (SvGMAGICAL(sstr) && (flags & SV_GMAGIC)) {
	    mg_get(sstr);
	    if (SvTYPE(sstr) != stype)
		stype = SvTYPE(sstr);
	}
	if (isGV_with_GP(sstr) && dtype <= SVt_PVLV) {
		    glob_assign_glob(dstr, sstr, dtype);
		    return;
	}
	if (stype == SVt_PVLV)
	    SvUPGRADE(dstr, SVt_PVNV);
	else
	    SvUPGRADE(dstr, (svtype)stype);
    }
 end_of_first_switch:

    /* dstr may have been upgraded.  */
    dtype = SvTYPE(dstr);
    sflags = SvFLAGS(sstr);

    if (dtype == SVt_PVCV || dtype == SVt_PVFM) {
	/* Assigning to a subroutine sets the prototype.  */
	if (SvOK(sstr)) {
	    STRLEN len;
	    const char *const ptr = SvPV_const(sstr, len);

            SvGROW(dstr, len + 1);
            Copy(ptr, SvPVX(dstr), len + 1, char);
            SvCUR_set(dstr, len);
	    SvPOK_only(dstr);
	    SvFLAGS(dstr) |= sflags & SVf_UTF8;
	    CvAUTOLOAD_off(dstr);
	} else {
	    SvOK_off(dstr);
	}
    } else if (dtype == SVt_PVAV || dtype == SVt_PVHV) {
	const char * const type = sv_reftype(dstr,0);
	if (PL_op)
	    /* diag_listed_as: Cannot copy to %s */
	    Perl_croak(aTHX_ "Cannot copy to %s in %s", type, OP_DESC(PL_op));
	else
	    Perl_croak(aTHX_ "Cannot copy to %s", type);
    } else if (sflags & SVf_ROK) {
	if (isGV_with_GP(dstr)
	    && SvTYPE(SvRV(sstr)) == SVt_PVGV && isGV_with_GP(SvRV(sstr))) {
	    sstr = SvRV(sstr);
	    if (sstr == dstr) {
		if (GvIMPORTED(dstr) != GVf_IMPORTED
		    && CopSTASH_ne(PL_curcop, GvSTASH(dstr)))
		{
		    GvIMPORTED_on(dstr);
		}
		GvMULTI_on(dstr);
		return;
	    }
	    glob_assign_glob(dstr, sstr, dtype);
	    return;
	}

	if (dtype >= SVt_PV) {
	    if (isGV_with_GP(dstr)) {
		glob_assign_ref(dstr, sstr);
		return;
	    }
	    if (SvPVX_const(dstr)) {
		SvPV_free(dstr);
		SvLEN_set(dstr, 0);
                SvCUR_set(dstr, 0);
	    }
	}
	(void)SvOK_off(dstr);
	SvRV_set(dstr, SvREFCNT_inc(SvRV(sstr)));
	SvFLAGS(dstr) |= sflags & SVf_ROK;
	assert(!(sflags & SVp_NOK));
	assert(!(sflags & SVp_IOK));
	assert(!(sflags & SVf_NOK));
	assert(!(sflags & SVf_IOK));
    }
    else if (isGV_with_GP(dstr)) {
	if (!(sflags & SVf_OK)) {
	    Perl_ck_warner(aTHX_ packWARN(WARN_MISC),
			   "Undefined value assigned to typeglob");
	}
	else {
	    GV *gv = gv_fetchsv_nomg(sstr, GV_ADD, SVt_PVGV);
	    if (dstr != (const SV *)gv) {
		const char * const name = GvNAME((const GV *)dstr);
		const STRLEN len = GvNAMELEN(dstr);
		HV *old_stash = NULL;
		bool reset_isa = FALSE;
		if ((len > 1 && name[len-2] == ':' && name[len-1] == ':')
		 || (len == 1 && name[0] == ':')) {
		    /* Set aside the old stash, so we can reset isa caches
		       on its subclasses. */
		    if((old_stash = GvHV(dstr))) {
			/* Make sure we do not lose it early. */
			SvREFCNT_inc_simple_void_NN(
			 sv_2mortal((SV *)old_stash)
			);
		    }
		    reset_isa = TRUE;
		}

		if (GvGP(dstr))
		    gp_free(MUTABLE_GV(dstr));
		GvGP_set(dstr, gp_ref(GvGP(gv)));

		if (reset_isa) {
		    HV * const stash = GvHV(dstr);
		    if(
		        old_stash ? (HV *)HvENAME_get(old_stash) : stash
		    )
			mro_package_moved(
			 stash, old_stash,
			 (GV *)dstr, 0
			);
		}
	    }
	}
    }
    else if (dtype == SVt_REGEXP && stype == SVt_REGEXP) {
	reg_temp_copy((REGEXP*)dstr, (REGEXP*)sstr);
    }
    else if (sflags & SVp_POK) {
        bool isSwipe = 0;

	/*
	 * Check to see if we can just swipe the string.  If so, it's a
	 * possible small lose on short strings, but a big win on long ones.
	 * It might even be a win on short strings if SvPVX_const(dstr)
	 * has to be allocated and SvPVX_const(sstr) has to be freed.
	 * Likewise if we can set up COW rather than doing an actual copy, we
	 * drop to the else clause, as the swipe code and the COW setup code
	 * have much in common.
	 */

	/* Whichever path we take through the next code, we want this true,
	   and doing it now facilitates the COW check.  */
	(void)SvPOK_only(dstr);

	if (
	    /* If we're already COW then this clause is not true, and if COW
	       is allowed then we drop down to the else and make dest COW 
	       with us.  If caller hasn't said that we're allowed to COW
	       shared hash keys then we don't do the COW setup, even if the
	       source scalar is a shared hash key scalar.  */
            (((flags & SV_COW_SHARED_HASH_KEYS)
	       ? (sflags & (SVf_FAKE|SVf_READONLY)) != (SVf_FAKE|SVf_READONLY)
	       : 1 /* If making a COW copy is forbidden then the behaviour we
		       desire is as if the source SV isn't actually already
		       COW, even if it is.  So we act as if the source flags
		       are not COW, rather than actually testing them.  */
	      )
#ifndef PERL_OLD_COPY_ON_WRITE
	     /* The change that added SV_COW_SHARED_HASH_KEYS makes the logic
		when PERL_OLD_COPY_ON_WRITE is defined a little wrong.
		Conceptually PERL_OLD_COPY_ON_WRITE being defined should
		override SV_COW_SHARED_HASH_KEYS, because it means "always COW"
		but in turn, it's somewhat dead code, never expected to go
		live, but more kept as a placeholder on how to do it better
		in a newer implementation.  */
	     /* If we are COW and dstr is a suitable target then we drop down
		into the else and make dest a COW of us.  */
	     || (SvFLAGS(dstr) & CAN_COW_MASK) != CAN_COW_FLAGS
#endif
	     )
            &&
            !(isSwipe =
                 (sflags & SVs_TEMP) &&   /* slated for free anyway? */
                 !(sflags & SVf_OOK) &&   /* and not involved in OOK hack? */
	         (!(flags & SV_NOSTEAL)) &&
					/* and we're allowed to steal temps */
                 SvREFCNT(sstr) == 1 &&   /* and no other references to it? */
                 SvLEN(sstr))             /* and really is a string */
#ifdef PERL_OLD_COPY_ON_WRITE
            && ((flags & SV_COW_SHARED_HASH_KEYS)
		? (!((sflags & CAN_COW_MASK) == CAN_COW_FLAGS
		     && (SvFLAGS(dstr) & CAN_COW_MASK) == CAN_COW_FLAGS
		     && SvTYPE(sstr) >= SVt_PVIV && SvTYPE(sstr) != SVt_PVFM))
		: 1)
#endif
            ) {
            /* Failed the swipe test, and it's not a shared hash key either.
               Have to copy the string.  */
	    STRLEN len = SvCUR(sstr);
            SvGROW(dstr, len + 1);	/* inlined from sv_setpvn */
            Move(SvPVX_const(sstr),SvPVX(dstr),len,char);
            SvCUR_set(dstr, len);
            *SvEND(dstr) = '\0';
        } else {
            /* If PERL_OLD_COPY_ON_WRITE is not defined, then isSwipe will always
               be true in here.  */
            /* Either it's a shared hash key, or it's suitable for
               copy-on-write or we can swipe the string.  */
            if (DEBUG_C_TEST) {
                PerlIO_printf(Perl_debug_log, "Copy on write: sstr --> dstr\n");
                sv_dump(sstr);
                sv_dump(dstr);
            }
#ifdef PERL_OLD_COPY_ON_WRITE
            if (!isSwipe) {
                if ((sflags & (SVf_FAKE | SVf_READONLY))
                    != (SVf_FAKE | SVf_READONLY)) {
                    SvREADONLY_on(sstr);
                    SvFAKE_on(sstr);
                    /* Make the source SV into a loop of 1.
                       (about to become 2) */
                    SV_COW_NEXT_SV_SET(sstr, sstr);
                }
            }
#endif
            /* Initial code is common.  */
	    if (SvPVX_const(dstr)) {	/* we know that dtype >= SVt_PV */
		SvPV_free(dstr);
	    }

            if (!isSwipe) {
                /* making another shared SV.  */
                STRLEN cur = SvCUR(sstr);
                STRLEN len = SvLEN(sstr);
#ifdef PERL_OLD_COPY_ON_WRITE
                if (len) {
		    assert (SvTYPE(dstr) >= SVt_PVIV);
                    /* SvIsCOW_normal */
                    /* splice us in between source and next-after-source.  */
                    SV_COW_NEXT_SV_SET(dstr, SV_COW_NEXT_SV(sstr));
                    SV_COW_NEXT_SV_SET(sstr, dstr);
                    SvPV_set(dstr, SvPVX_mutable(sstr));
                } else
#endif
		{
                    /* SvIsCOW_shared_hash */
                    DEBUG_C(PerlIO_printf(Perl_debug_log,
                                          "Copy on write: Sharing hash\n"));

		    assert (SvTYPE(dstr) >= SVt_PV);
                    SvPV_set(dstr,
			     HEK_KEY(share_hek_hek(SvSHARED_HEK_FROM_PV(SvPVX_const(sstr)))));
		}
                SvLEN_set(dstr, len);
                SvCUR_set(dstr, cur);
                SvREADONLY_on(dstr);
                SvFAKE_on(dstr);
            }
            else
                {	/* Passes the swipe test.  */
                SvPV_set(dstr, SvPVX_mutable(sstr));
                SvLEN_set(dstr, SvLEN(sstr));
                SvCUR_set(dstr, SvCUR(sstr));

                SvTEMP_off(dstr);
                (void)SvOK_off(sstr);	/* NOTE: nukes most SvFLAGS on sstr */
                SvPV_set(sstr, NULL);
                SvLEN_set(sstr, 0);
                SvCUR_set(sstr, 0);
                SvTEMP_off(sstr);
            }
        }
	if (sflags & SVp_NOK) {
	    SvNV_set(dstr, SvNVX(sstr));
	}
	if (sflags & SVp_IOK) {
	    SvIV_set(dstr, SvIVX(sstr));
	    /* Must do this otherwise some other overloaded use of 0x80000000
	       gets confused. I guess SVpbm_VALID */
	    if (sflags & SVf_IVisUV)
		SvIsUV_on(dstr);
	}
	SvFLAGS(dstr) |= sflags & (SVf_IOK|SVp_IOK|SVf_NOK|SVp_NOK|SVf_UTF8);
	{
	    const MAGIC * const smg = SvVSTRING_mg(sstr);
	    if (smg) {
		sv_magic(dstr, NULL, PERL_MAGIC_vstring,
			 smg->mg_ptr, smg->mg_len);
		SvRMAGICAL_on(dstr);
	    }
	}
    }
    else if (sflags & (SVp_IOK|SVp_NOK)) {
	(void)SvOK_off(dstr);
	SvFLAGS(dstr) |= sflags & (SVf_IOK|SVp_IOK|SVf_IVisUV|SVf_NOK|SVp_NOK);
	if (sflags & SVp_IOK) {
	    /* XXXX Do we want to set IsUV for IV(ROK)?  Be extra safe... */
	    SvIV_set(dstr, SvIVX(sstr));
	}
	if (sflags & SVp_NOK) {
	    SvNV_set(dstr, SvNVX(sstr));
	}
    }
    else {
	if (isGV_with_GP(sstr)) {
	    gv_efullname3(dstr, MUTABLE_GV(sstr), "*");
	}
	else
	    (void)SvOK_off(dstr);
    }
    if (SvTAINTED(sstr))
	SvTAINT(dstr);
}

/*
=for apidoc sv_setsv_mg

Like C<sv_setsv>, but also handles 'set' magic.

=cut
*/

void
Perl_sv_setsv_mg(pTHX_ SV *const dstr, register SV *const sstr)
{
    PERL_ARGS_ASSERT_SV_SETSV_MG;

    sv_setsv(dstr,sstr);
    SvSETMAGIC(dstr);
}

#ifdef PERL_OLD_COPY_ON_WRITE
SV *
Perl_sv_setsv_cow(pTHX_ SV *dstr, SV *sstr)
{
    STRLEN cur = SvCUR(sstr);
    STRLEN len = SvLEN(sstr);
    register char *new_pv;

    PERL_ARGS_ASSERT_SV_SETSV_COW;

    if (DEBUG_C_TEST) {
	PerlIO_printf(Perl_debug_log, "Fast copy on write: %p -> %p\n",
		      (void*)sstr, (void*)dstr);
	sv_dump(sstr);
	if (dstr)
		    sv_dump(dstr);
    }

    if (dstr) {
	if (SvTHINKFIRST(dstr))
	    sv_force_normal_flags(dstr, SV_COW_DROP_PV);
	else if (SvPVX_const(dstr))
	    Safefree(SvPVX_const(dstr));
    }
    else
	new_SV(dstr);
    SvUPGRADE(dstr, SVt_PVIV);

    assert (SvPOK(sstr));
    assert (SvPOKp(sstr));
    assert (!SvIOK(sstr));
    assert (!SvIOKp(sstr));
    assert (!SvNOK(sstr));
    assert (!SvNOKp(sstr));

    if (SvIsCOW(sstr)) {

	if (SvLEN(sstr) == 0) {
	    /* source is a COW shared hash key.  */
	    DEBUG_C(PerlIO_printf(Perl_debug_log,
				  "Fast copy on write: Sharing hash\n"));
	    new_pv = HEK_KEY(share_hek_hek(SvSHARED_HEK_FROM_PV(SvPVX_const(sstr))));
	    goto common_exit;
	}
	SV_COW_NEXT_SV_SET(dstr, SV_COW_NEXT_SV(sstr));
    } else {
	assert ((SvFLAGS(sstr) & CAN_COW_MASK) == CAN_COW_FLAGS);
	SvUPGRADE(sstr, SVt_PVIV);
	SvREADONLY_on(sstr);
	SvFAKE_on(sstr);
	DEBUG_C(PerlIO_printf(Perl_debug_log,
			      "Fast copy on write: Converting sstr to COW\n"));
	SV_COW_NEXT_SV_SET(dstr, sstr);
    }
    SV_COW_NEXT_SV_SET(sstr, dstr);
    new_pv = SvPVX_mutable(sstr);

  common_exit:
    SvPV_set(dstr, new_pv);
    SvFLAGS(dstr) = (SVt_PVIV|SVf_POK|SVp_POK|SVf_FAKE|SVf_READONLY);
    if (SvUTF8(sstr))
	SvUTF8_on(dstr);
    SvLEN_set(dstr, len);
    SvCUR_set(dstr, cur);
    if (DEBUG_C_TEST) {
	sv_dump(dstr);
    }
    return dstr;
}
#endif

/*
=for apidoc sv_setpvn

Copies a string into an SV.  The C<len> parameter indicates the number of
bytes to be copied.  If the C<ptr> argument is NULL the SV will become
undefined.  Does not handle 'set' magic.  See C<sv_setpvn_mg>.

=cut
*/

void
Perl_sv_setpvn(pTHX_ register SV *const sv, register const char *const ptr, register const STRLEN len)
{
    dVAR;
    register char *dptr;

    PERL_ARGS_ASSERT_SV_SETPVN;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    if (!ptr) {
	(void)SvOK_off(sv);
	return;
    }
    else {
        /* len is STRLEN which is unsigned, need to copy to signed */
	const IV iv = len;
	if (iv < 0)
	    Perl_croak(aTHX_ "panic: sv_setpvn called with negative strlen %"
		       IVdf, iv);
    }
    SvUPGRADE(sv, SVt_PV);

    dptr = SvGROW(sv, len + 1);
    Move(ptr,dptr,len,char);
    dptr[len] = '\0';
    SvCUR_set(sv, len);
    (void)SvPOK_only_UTF8(sv);		/* validate pointer */
    SvTAINT(sv);
    if (SvTYPE(sv) == SVt_PVCV) CvAUTOLOAD_off(sv);
}

/*
=for apidoc sv_setpvn_mg

Like C<sv_setpvn>, but also handles 'set' magic.

=cut
*/

void
Perl_sv_setpvn_mg(pTHX_ register SV *const sv, register const char *const ptr, register const STRLEN len)
{
    PERL_ARGS_ASSERT_SV_SETPVN_MG;

    sv_setpvn(sv,ptr,len);
    SvSETMAGIC(sv);
}

/*
=for apidoc sv_setpv

Copies a string into an SV.  The string must be null-terminated.  Does not
handle 'set' magic.  See C<sv_setpv_mg>.

=cut
*/

void
Perl_sv_setpv(pTHX_ register SV *const sv, register const char *const ptr)
{
    dVAR;
    register STRLEN len;

    PERL_ARGS_ASSERT_SV_SETPV;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    if (!ptr) {
	(void)SvOK_off(sv);
	return;
    }
    len = strlen(ptr);
    SvUPGRADE(sv, SVt_PV);

    SvGROW(sv, len + 1);
    Move(ptr,SvPVX(sv),len+1,char);
    SvCUR_set(sv, len);
    (void)SvPOK_only_UTF8(sv);		/* validate pointer */
    SvTAINT(sv);
    if (SvTYPE(sv) == SVt_PVCV) CvAUTOLOAD_off(sv);
}

/*
=for apidoc sv_setpv_mg

Like C<sv_setpv>, but also handles 'set' magic.

=cut
*/

void
Perl_sv_setpv_mg(pTHX_ register SV *const sv, register const char *const ptr)
{
    PERL_ARGS_ASSERT_SV_SETPV_MG;

    sv_setpv(sv,ptr);
    SvSETMAGIC(sv);
}

void
Perl_sv_sethek(pTHX_ register SV *const sv, const HEK *const hek)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_SETHEK;

    if (!hek) {
	return;
    }

    if (HEK_LEN(hek) == HEf_SVKEY) {
	sv_setsv(sv, *(SV**)HEK_KEY(hek));
        return;
    } else {
	const int flags = HEK_FLAGS(hek);
	if (flags & HVhek_WASUTF8) {
	    STRLEN utf8_len = HEK_LEN(hek);
	    char *as_utf8 = (char *)bytes_to_utf8((U8*)HEK_KEY(hek), &utf8_len);
	    sv_usepvn_flags(sv, as_utf8, utf8_len, SV_HAS_TRAILING_NUL);
	    SvUTF8_on(sv);
            return;
	} else if (flags & (HVhek_REHASH|HVhek_UNSHARED)) {
	    sv_setpvn(sv, HEK_KEY(hek), HEK_LEN(hek));
	    if (HEK_UTF8(hek))
		SvUTF8_on(sv);
	    else SvUTF8_off(sv);
            return;
	}
        {
	    SV_CHECK_THINKFIRST_COW_DROP(sv);
	    SvUPGRADE(sv, SVt_PV);
	    SvPV_set(sv,(char *)HEK_KEY(share_hek_hek(hek)));
	    SvCUR_set(sv, HEK_LEN(hek));
	    SvLEN_set(sv, 0);
	    SvREADONLY_on(sv);
	    SvFAKE_on(sv);
	    SvPOK_on(sv);
	    if (HEK_UTF8(hek))
		SvUTF8_on(sv);
	    else SvUTF8_off(sv);
            return;
	}
    }
}


/*
=for apidoc sv_usepvn_flags

Tells an SV to use C<ptr> to find its string value.  Normally the
string is stored inside the SV but sv_usepvn allows the SV to use an
outside string.  The C<ptr> should point to memory that was allocated
by C<malloc>.  It must be the start of a mallocked block
of memory, and not a pointer to the middle of it.  The
string length, C<len>, must be supplied.  By default
this function will realloc (i.e. move) the memory pointed to by C<ptr>,
so that pointer should not be freed or used by the programmer after
giving it to sv_usepvn, and neither should any pointers from "behind"
that pointer (e.g. ptr + 1) be used.

If C<flags> & SV_SMAGIC is true, will call SvSETMAGIC.  If C<flags> &
SV_HAS_TRAILING_NUL is true, then C<ptr[len]> must be NUL, and the realloc
will be skipped (i.e. the buffer is actually at least 1 byte longer than
C<len>, and already meets the requirements for storing in C<SvPVX>).

=cut
*/

void
Perl_sv_usepvn_flags(pTHX_ SV *const sv, char *ptr, const STRLEN len, const U32 flags)
{
    dVAR;
    STRLEN allocate;

    PERL_ARGS_ASSERT_SV_USEPVN_FLAGS;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    SvUPGRADE(sv, SVt_PV);
    if (!ptr) {
	(void)SvOK_off(sv);
	if (flags & SV_SMAGIC)
	    SvSETMAGIC(sv);
	return;
    }
    if (SvPVX_const(sv))
	SvPV_free(sv);

#ifdef DEBUGGING
    if (flags & SV_HAS_TRAILING_NUL)
	assert(ptr[len] == '\0');
#endif

    allocate = (flags & SV_HAS_TRAILING_NUL)
	? len + 1 :
#ifdef Perl_safesysmalloc_size
	len + 1;
#else 
	PERL_STRLEN_ROUNDUP(len + 1);
#endif
    if (flags & SV_HAS_TRAILING_NUL) {
	/* It's long enough - do nothing.
	   Specifically Perl_newCONSTSUB is relying on this.  */
    } else {
#ifdef DEBUGGING
	/* Force a move to shake out bugs in callers.  */
	char *new_ptr = (char*)safemalloc(allocate);
	Copy(ptr, new_ptr, len, char);
	PoisonFree(ptr,len,char);
	Safefree(ptr);
	ptr = new_ptr;
#else
	ptr = (char*) saferealloc (ptr, allocate);
#endif
    }
#ifdef Perl_safesysmalloc_size
    SvLEN_set(sv, Perl_safesysmalloc_size(ptr));
#else
    SvLEN_set(sv, allocate);
#endif
    SvCUR_set(sv, len);
    SvPV_set(sv, ptr);
    if (!(flags & SV_HAS_TRAILING_NUL)) {
	ptr[len] = '\0';
    }
    (void)SvPOK_only_UTF8(sv);		/* validate pointer */
    SvTAINT(sv);
    if (flags & SV_SMAGIC)
	SvSETMAGIC(sv);
}

#ifdef PERL_OLD_COPY_ON_WRITE
/* Need to do this *after* making the SV normal, as we need the buffer
   pointer to remain valid until after we've copied it.  If we let go too early,
   another thread could invalidate it by unsharing last of the same hash key
   (which it can do by means other than releasing copy-on-write Svs)
   or by changing the other copy-on-write SVs in the loop.  */
STATIC void
S_sv_release_COW(pTHX_ register SV *sv, const char *pvx, SV *after)
{
    PERL_ARGS_ASSERT_SV_RELEASE_COW;

    { /* this SV was SvIsCOW_normal(sv) */
         /* we need to find the SV pointing to us.  */
        SV *current = SV_COW_NEXT_SV(after);

        if (current == sv) {
            /* The SV we point to points back to us (there were only two of us
               in the loop.)
               Hence other SV is no longer copy on write either.  */
            SvFAKE_off(after);
            SvREADONLY_off(after);
        } else {
            /* We need to follow the pointers around the loop.  */
            SV *next;
            while ((next = SV_COW_NEXT_SV(current)) != sv) {
                assert (next);
                current = next;
                 /* don't loop forever if the structure is bust, and we have
                    a pointer into a closed loop.  */
                assert (current != after);
                assert (SvPVX_const(current) == pvx);
            }
            /* Make the SV before us point to the SV after us.  */
            SV_COW_NEXT_SV_SET(current, after);
        }
    }
}
#endif
/*
=for apidoc sv_force_normal_flags

Undo various types of fakery on an SV: if the PV is a shared string, make
a private copy; if we're a ref, stop refing; if we're a glob, downgrade to
an xpvmg; if we're a copy-on-write scalar, this is the on-write time when
we do the copy, and is also used locally.  If C<SV_COW_DROP_PV> is set
then a copy-on-write scalar drops its PV buffer (if any) and becomes
SvPOK_off rather than making a copy.  (Used where this
scalar is about to be set to some other value.)  In addition,
the C<flags> parameter gets passed to C<sv_unref_flags()>
when unreffing.  C<sv_force_normal> calls this function
with flags set to 0.

=cut
*/

void
Perl_sv_force_normal_flags(pTHX_ register SV *const sv, const U32 flags)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_FORCE_NORMAL_FLAGS;

#ifdef PERL_OLD_COPY_ON_WRITE
    if (SvREADONLY(sv)) {
	if (SvFAKE(sv)) {
	    const char * const pvx = SvPVX_const(sv);
	    const STRLEN len = SvLEN(sv);
	    const STRLEN cur = SvCUR(sv);
	    /* next COW sv in the loop.  If len is 0 then this is a shared-hash
	       key scalar, so we mustn't attempt to call SV_COW_NEXT_SV(), as
	       we'll fail an assertion.  */
	    SV * const next = len ? SV_COW_NEXT_SV(sv) : 0;

            if (DEBUG_C_TEST) {
                PerlIO_printf(Perl_debug_log,
                              "Copy on write: Force normal %ld\n",
                              (long) flags);
                sv_dump(sv);
            }
            SvFAKE_off(sv);
            SvREADONLY_off(sv);
            /* This SV doesn't own the buffer, so need to Newx() a new one:  */
            SvPV_set(sv, NULL);
            SvLEN_set(sv, 0);
            if (flags & SV_COW_DROP_PV) {
                /* OK, so we don't need to copy our buffer.  */
                SvPOK_off(sv);
            } else {
                SvGROW(sv, cur + 1);
                Move(pvx,SvPVX(sv),cur,char);
                SvCUR_set(sv, cur);
                *SvEND(sv) = '\0';
            }
	    if (len) {
		sv_release_COW(sv, pvx, next);
	    } else {
		unshare_hek(SvSHARED_HEK_FROM_PV(pvx));
	    }
            if (DEBUG_C_TEST) {
                sv_dump(sv);
            }
	}
	else if (IN_PERL_RUNTIME)
	    Perl_croak_no_modify(aTHX);
    }
#else
    if (SvREADONLY(sv)) {
	if (SvIsCOW(sv)) {
	    const char * const pvx = SvPVX_const(sv);
	    const STRLEN len = SvCUR(sv);
	    SvFAKE_off(sv);
	    SvREADONLY_off(sv);
	    SvPV_set(sv, NULL);
	    SvLEN_set(sv, 0);
	    if (flags & SV_COW_DROP_PV) {
		/* OK, so we don't need to copy our buffer.  */
		SvPOK_off(sv);
	    } else {
		SvGROW(sv, len + 1);
		Move(pvx,SvPVX(sv),len,char);
		*SvEND(sv) = '\0';
	    }
	    unshare_hek(SvSHARED_HEK_FROM_PV(pvx));
	}
	else if (IN_PERL_RUNTIME)
	    Perl_croak_no_modify(aTHX);
    }
#endif
    if (SvROK(sv))
	sv_unref_flags(sv, flags);
    else if (SvFAKE(sv) && isGV_with_GP(sv))
	sv_unglob(sv, flags);
    else if (SvFAKE(sv) && SvTYPE(sv) == SVt_REGEXP) {
	/* Need to downgrade the REGEXP to a simple(r) scalar. This is analogous
	   to sv_unglob. We only need it here, so inline it.  */
	const svtype new_type = SvMAGIC(sv) || SvSTASH(sv) ? SVt_PVMG : SVt_PV;
	SV *const temp = newSV_type(new_type);
	void *const temp_p = SvANY(sv);

	if (new_type == SVt_PVMG) {
	    SvMAGIC_set(temp, SvMAGIC(sv));
	    SvMAGIC_set(sv, NULL);
	    SvSTASH_set(temp, SvSTASH(sv));
	    SvSTASH_set(sv, NULL);
	}
	SvCUR_set(temp, SvCUR(sv));
	/* Remember that SvPVX is in the head, not the body. */
	if (SvLEN(temp)) {
	    SvLEN_set(temp, SvLEN(sv));
	    /* This signals "buffer is owned by someone else" in sv_clear,
	       which is the least effort way to stop it freeing the buffer.
	    */
	    SvLEN_set(sv, SvLEN(sv)+1);
	} else {
	    /* Their buffer is already owned by someone else. */
	    SvPVX(sv) = savepvn(SvPVX(sv), SvCUR(sv));
	    SvLEN_set(temp, SvCUR(sv)+1);
	}

	/* Now swap the rest of the bodies. */

	SvFLAGS(sv) &= ~(SVf_FAKE|SVTYPEMASK);
	SvFLAGS(sv) |= new_type;
	SvANY(sv) = SvANY(temp);

	SvFLAGS(temp) &= ~(SVTYPEMASK);
	SvFLAGS(temp) |= SVt_REGEXP|SVf_FAKE;
	SvANY(temp) = temp_p;

	SvREFCNT_dec(temp);
    }
}

/*
=for apidoc sv_chop

Efficient removal of characters from the beginning of the string buffer.
SvPOK(sv) must be true and the C<ptr> must be a pointer to somewhere inside
the string buffer.  The C<ptr> becomes the first character of the adjusted
string.  Uses the "OOK hack".

Beware: after this function returns, C<ptr> and SvPVX_const(sv) may no longer
refer to the same chunk of data.

The unfortunate similarity of this function's name to that of Perl's C<chop>
operator is strictly coincidental.  This function works from the left;
C<chop> works from the right.

=cut
*/

void
Perl_sv_chop(pTHX_ register SV *const sv, register const char *const ptr)
{
    STRLEN delta;
    STRLEN old_delta;
    U8 *p;
#ifdef DEBUGGING
    const U8 *evacp;
    STRLEN evacn;
#endif
    STRLEN max_delta;

    PERL_ARGS_ASSERT_SV_CHOP;

    if (!ptr || !SvPOKp(sv))
	return;
    delta = ptr - SvPVX_const(sv);
    if (!delta) {
	/* Nothing to do.  */
	return;
    }
    max_delta = SvLEN(sv) ? SvLEN(sv) : SvCUR(sv);
    if (delta > max_delta)
	Perl_croak(aTHX_ "panic: sv_chop ptr=%p, start=%p, end=%p",
		   ptr, SvPVX_const(sv), SvPVX_const(sv) + max_delta);
    /* SvPVX(sv) may move in SV_CHECK_THINKFIRST(sv), so don't use ptr any more */
    SV_CHECK_THINKFIRST(sv);

    if (!SvOOK(sv)) {
	if (!SvLEN(sv)) { /* make copy of shared string */
	    const char *pvx = SvPVX_const(sv);
	    const STRLEN len = SvCUR(sv);
	    SvGROW(sv, len + 1);
	    Move(pvx,SvPVX(sv),len,char);
	    *SvEND(sv) = '\0';
	}
	SvOOK_on(sv);
	old_delta = 0;
    } else {
	SvOOK_offset(sv, old_delta);
    }
    SvLEN_set(sv, SvLEN(sv) - delta);
    SvCUR_set(sv, SvCUR(sv) - delta);
    SvPV_set(sv, SvPVX(sv) + delta);

    p = (U8 *)SvPVX_const(sv);

#ifdef DEBUGGING
    /* how many bytes were evacuated?  we will fill them with sentinel
       bytes, except for the part holding the new offset of course. */
    evacn = delta;
    if (old_delta)
	evacn += (old_delta < 0x100 ? 1 : 1 + sizeof(STRLEN));
    assert(evacn);
    assert(evacn <= delta + old_delta);
    evacp = p - evacn;
#endif

    delta += old_delta;
    assert(delta);
    if (delta < 0x100) {
	*--p = (U8) delta;
    } else {
	*--p = 0;
	p -= sizeof(STRLEN);
	Copy((U8*)&delta, p, sizeof(STRLEN), U8);
    }

#ifdef DEBUGGING
    /* Fill the preceding buffer with sentinals to verify that no-one is
       using it.  */
    while (p > evacp) {
	--p;
	*p = (U8)PTR2UV(p);
    }
#endif
}

/*
=for apidoc sv_catpvn

Concatenates the string onto the end of the string which is in the SV.  The
C<len> indicates number of bytes to copy.  If the SV has the UTF-8
status set, then the bytes appended should be valid UTF-8.
Handles 'get' magic, but not 'set' magic.  See C<sv_catpvn_mg>.

=for apidoc sv_catpvn_flags

Concatenates the string onto the end of the string which is in the SV.  The
C<len> indicates number of bytes to copy.  If the SV has the UTF-8
status set, then the bytes appended should be valid UTF-8.
If C<flags> has the C<SV_SMAGIC> bit set, will
C<mg_set> on C<dsv> afterwards if appropriate.
C<sv_catpvn> and C<sv_catpvn_nomg> are implemented
in terms of this function.

=cut
*/

void
Perl_sv_catpvn_flags(pTHX_ register SV *const dsv, register const char *sstr, register const STRLEN slen, const I32 flags)
{
    dVAR;
    STRLEN dlen;
    const char * const dstr = SvPV_force_flags(dsv, dlen, flags);

    PERL_ARGS_ASSERT_SV_CATPVN_FLAGS;
    assert((flags & (SV_CATBYTES|SV_CATUTF8)) != (SV_CATBYTES|SV_CATUTF8));

    if (!(flags & SV_CATBYTES) || !SvUTF8(dsv)) {
      if (flags & SV_CATUTF8 && !SvUTF8(dsv)) {
	 sv_utf8_upgrade_flags_grow(dsv, 0, slen + 1);
	 dlen = SvCUR(dsv);
      }
      else SvGROW(dsv, dlen + slen + 1);
      if (sstr == dstr)
	sstr = SvPVX_const(dsv);
      Move(sstr, SvPVX(dsv) + dlen, slen, char);
      SvCUR_set(dsv, SvCUR(dsv) + slen);
    }
    else {
	/* We inline bytes_to_utf8, to avoid an extra malloc. */
	const char * const send = sstr + slen;
	U8 *d;

	/* Something this code does not account for, which I think is
	   impossible; it would require the same pv to be treated as
	   bytes *and* utf8, which would indicate a bug elsewhere. */
	assert(sstr != dstr);

	SvGROW(dsv, dlen + slen * 2 + 1);
	d = (U8 *)SvPVX(dsv) + dlen;

	while (sstr < send) {
	    const UV uv = NATIVE_TO_ASCII((U8)*sstr++);
	    if (UNI_IS_INVARIANT(uv))
		*d++ = (U8)UTF_TO_NATIVE(uv);
	    else {
		*d++ = (U8)UTF8_EIGHT_BIT_HI(uv);
		*d++ = (U8)UTF8_EIGHT_BIT_LO(uv);
	    }
	}
	SvCUR_set(dsv, d-(const U8 *)SvPVX(dsv));
    }
    *SvEND(dsv) = '\0';
    (void)SvPOK_only_UTF8(dsv);		/* validate pointer */
    SvTAINT(dsv);
    if (flags & SV_SMAGIC)
	SvSETMAGIC(dsv);
}

/*
=for apidoc sv_catsv

Concatenates the string from SV C<ssv> onto the end of the string in
SV C<dsv>.  Modifies C<dsv> but not C<ssv>.  Handles 'get' magic, but
not 'set' magic.  See C<sv_catsv_mg>.

=for apidoc sv_catsv_flags

Concatenates the string from SV C<ssv> onto the end of the string in
SV C<dsv>.  Modifies C<dsv> but not C<ssv>.  If C<flags> has C<SV_GMAGIC>
bit set, will C<mg_get> on the C<ssv>, if appropriate, before
reading it.  If the C<flags> contain C<SV_SMAGIC>, C<mg_set> will be
called on the modified SV afterward, if appropriate.  C<sv_catsv>
and C<sv_catsv_nomg> are implemented in terms of this function.

=cut */

void
Perl_sv_catsv_flags(pTHX_ SV *const dsv, register SV *const ssv, const I32 flags)
{
    dVAR;
 
    PERL_ARGS_ASSERT_SV_CATSV_FLAGS;

   if (ssv) {
	STRLEN slen;
	const char *spv = SvPV_flags_const(ssv, slen, flags);
	if (spv) {
	    if (SvGMAGICAL(dsv) && (flags & SV_GMAGIC))
		mg_get(dsv);
	    sv_catpvn_flags(dsv, spv, slen,
			    DO_UTF8(ssv) ? SV_CATUTF8 : SV_CATBYTES);
	}
    }
    if (flags & SV_SMAGIC)
	SvSETMAGIC(dsv);
}

/*
=for apidoc sv_catpv

Concatenates the string onto the end of the string which is in the SV.
If the SV has the UTF-8 status set, then the bytes appended should be
valid UTF-8.  Handles 'get' magic, but not 'set' magic.  See C<sv_catpv_mg>.

=cut */

void
Perl_sv_catpv(pTHX_ register SV *const sv, register const char *ptr)
{
    dVAR;
    register STRLEN len;
    STRLEN tlen;
    char *junk;

    PERL_ARGS_ASSERT_SV_CATPV;

    if (!ptr)
	return;
    junk = SvPV_force(sv, tlen);
    len = strlen(ptr);
    SvGROW(sv, tlen + len + 1);
    if (ptr == junk)
	ptr = SvPVX_const(sv);
    Move(ptr,SvPVX(sv)+tlen,len+1,char);
    SvCUR_set(sv, SvCUR(sv) + len);
    (void)SvPOK_only_UTF8(sv);		/* validate pointer */
    SvTAINT(sv);
}

/*
=for apidoc sv_catpv_flags

Concatenates the string onto the end of the string which is in the SV.
If the SV has the UTF-8 status set, then the bytes appended should
be valid UTF-8.  If C<flags> has the C<SV_SMAGIC> bit set, will C<mg_set>
on the modified SV if appropriate.

=cut
*/

void
Perl_sv_catpv_flags(pTHX_ SV *dstr, const char *sstr, const I32 flags)
{
    PERL_ARGS_ASSERT_SV_CATPV_FLAGS;
    sv_catpvn_flags(dstr, sstr, strlen(sstr), flags);
}

/*
=for apidoc sv_catpv_mg

Like C<sv_catpv>, but also handles 'set' magic.

=cut
*/

void
Perl_sv_catpv_mg(pTHX_ register SV *const sv, register const char *const ptr)
{
    PERL_ARGS_ASSERT_SV_CATPV_MG;

    sv_catpv(sv,ptr);
    SvSETMAGIC(sv);
}

/*
=for apidoc newSV

Creates a new SV.  A non-zero C<len> parameter indicates the number of
bytes of preallocated string space the SV should have.  An extra byte for a
trailing NUL is also reserved.  (SvPOK is not set for the SV even if string
space is allocated.)  The reference count for the new SV is set to 1.

In 5.9.3, newSV() replaces the older NEWSV() API, and drops the first
parameter, I<x>, a debug aid which allowed callers to identify themselves.
This aid has been superseded by a new build option, PERL_MEM_LOG (see
L<perlhacktips/PERL_MEM_LOG>).  The older API is still there for use in XS
modules supporting older perls.

=cut
*/

SV *
Perl_newSV(pTHX_ const STRLEN len)
{
    dVAR;
    register SV *sv;

    new_SV(sv);
    if (len) {
	sv_upgrade(sv, SVt_PV);
	SvGROW(sv, len + 1);
    }
    return sv;
}
/*
=for apidoc sv_magicext

Adds magic to an SV, upgrading it if necessary.  Applies the
supplied vtable and returns a pointer to the magic added.

Note that C<sv_magicext> will allow things that C<sv_magic> will not.
In particular, you can add magic to SvREADONLY SVs, and add more than
one instance of the same 'how'.

If C<namlen> is greater than zero then a C<savepvn> I<copy> of C<name> is
stored, if C<namlen> is zero then C<name> is stored as-is and - as another
special case - if C<(name && namlen == HEf_SVKEY)> then C<name> is assumed
to contain an C<SV*> and is stored as-is with its REFCNT incremented.

(This is now used as a subroutine by C<sv_magic>.)

=cut
*/
MAGIC *	
Perl_sv_magicext(pTHX_ SV *const sv, SV *const obj, const int how, 
                const MGVTBL *const vtable, const char *const name, const I32 namlen)
{
    dVAR;
    MAGIC* mg;

    PERL_ARGS_ASSERT_SV_MAGICEXT;

    SvUPGRADE(sv, SVt_PVMG);
    Newxz(mg, 1, MAGIC);
    mg->mg_moremagic = SvMAGIC(sv);
    SvMAGIC_set(sv, mg);

    /* Sometimes a magic contains a reference loop, where the sv and
       object refer to each other.  To prevent a reference loop that
       would prevent such objects being freed, we look for such loops
       and if we find one we avoid incrementing the object refcount.

       Note we cannot do this to avoid self-tie loops as intervening RV must
       have its REFCNT incremented to keep it in existence.

    */
    if (!obj || obj == sv ||
	how == PERL_MAGIC_arylen ||
	how == PERL_MAGIC_symtab ||
	(SvTYPE(obj) == SVt_PVGV &&
	    (GvSV(obj) == sv || GvHV(obj) == (const HV *)sv
	     || GvAV(obj) == (const AV *)sv || GvCV(obj) == (const CV *)sv
	     || GvIOp(obj) == (const IO *)sv || GvFORM(obj) == (const CV *)sv)))
    {
	mg->mg_obj = obj;
    }
    else {
	mg->mg_obj = SvREFCNT_inc_simple(obj);
	mg->mg_flags |= MGf_REFCOUNTED;
    }

    /* Normal self-ties simply pass a null object, and instead of
       using mg_obj directly, use the SvTIED_obj macro to produce a
       new RV as needed.  For glob "self-ties", we are tieing the PVIO
       with an RV obj pointing to the glob containing the PVIO.  In
       this case, to avoid a reference loop, we need to weaken the
       reference.
    */

    if (how == PERL_MAGIC_tiedscalar && SvTYPE(sv) == SVt_PVIO &&
        obj && SvROK(obj) && GvIO(SvRV(obj)) == (const IO *)sv)
    {
      sv_rvweaken(obj);
    }

    mg->mg_type = how;
    mg->mg_len = namlen;
    if (name) {
	if (namlen > 0)
	    mg->mg_ptr = savepvn(name, namlen);
	else if (namlen == HEf_SVKEY) {
	    /* Yes, this is casting away const. This is only for the case of
	       HEf_SVKEY. I think we need to document this aberation of the
	       constness of the API, rather than making name non-const, as
	       that change propagating outwards a long way.  */
	    mg->mg_ptr = (char*)SvREFCNT_inc_simple_NN((SV *)name);
	} else
	    mg->mg_ptr = (char *) name;
    }
    mg->mg_virtual = (MGVTBL *) vtable;

    mg_magical(sv);
    if (SvGMAGICAL(sv))
	SvFLAGS(sv) &= ~(SVf_IOK|SVf_NOK|SVf_POK);
    return mg;
}

/*
=for apidoc sv_magic

Adds magic to an SV.  First upgrades C<sv> to type C<SVt_PVMG> if
necessary, then adds a new magic item of type C<how> to the head of the
magic list.

See C<sv_magicext> (which C<sv_magic> now calls) for a description of the
handling of the C<name> and C<namlen> arguments.

You need to use C<sv_magicext> to add magic to SvREADONLY SVs and also
to add more than one instance of the same 'how'.

=cut
*/

void
Perl_sv_magic(pTHX_ register SV *const sv, SV *const obj, const int how, 
             const char *const name, const I32 namlen)
{
    dVAR;
    const MGVTBL *vtable;
    MAGIC* mg;
    unsigned int flags;
    unsigned int vtable_index;

    PERL_ARGS_ASSERT_SV_MAGIC;

    if (how < 0 || (unsigned)how > C_ARRAY_LENGTH(PL_magic_data)
	|| ((flags = PL_magic_data[how]),
	    (vtable_index = flags & PERL_MAGIC_VTABLE_MASK)
	    > magic_vtable_max))
	Perl_croak(aTHX_ "Don't know how to handle magic of type \\%o", how);

    /* PERL_MAGIC_ext is reserved for use by extensions not perl internals.
       Useful for attaching extension internal data to perl vars.
       Note that multiple extensions may clash if magical scalars
       etc holding private data from one are passed to another. */

    vtable = (vtable_index == magic_vtable_max)
	? NULL : PL_magic_vtables + vtable_index;

#ifdef PERL_OLD_COPY_ON_WRITE
    if (SvIsCOW(sv))
        sv_force_normal_flags(sv, 0);
#endif
    if (SvREADONLY(sv)) {
	if (
	    /* its okay to attach magic to shared strings */
	    !SvIsCOW(sv)

	    && IN_PERL_RUNTIME
	    && !PERL_MAGIC_TYPE_READONLY_ACCEPTABLE(how)
	   )
	{
	    Perl_croak_no_modify(aTHX);
	}
    }
    if (SvMAGICAL(sv) || (how == PERL_MAGIC_taint && SvTYPE(sv) >= SVt_PVMG)) {
	if (SvMAGIC(sv) && (mg = mg_find(sv, how))) {
	    /* sv_magic() refuses to add a magic of the same 'how' as an
	       existing one
	     */
	    if (how == PERL_MAGIC_taint) {
		mg->mg_len |= 1;
		/* Any scalar which already had taint magic on which someone
		   (erroneously?) did SvIOK_on() or similar will now be
		   incorrectly sporting public "OK" flags.  */
		SvFLAGS(sv) &= ~(SVf_IOK|SVf_NOK|SVf_POK);
	    }
	    return;
	}
    }

    /* Rest of work is done else where */
    mg = sv_magicext(sv,obj,how,vtable,name,namlen);

    switch (how) {
    case PERL_MAGIC_taint:
	mg->mg_len = 1;
	break;
    case PERL_MAGIC_ext:
    case PERL_MAGIC_dbfile:
	SvRMAGICAL_on(sv);
	break;
    }
}

static int
S_sv_unmagicext_flags(pTHX_ SV *const sv, const int type, MGVTBL *vtbl, const U32 flags)
{
    MAGIC* mg;
    MAGIC** mgp;

    assert(flags <= 1);

    if (SvTYPE(sv) < SVt_PVMG || !SvMAGIC(sv))
	return 0;
    mgp = &(((XPVMG*) SvANY(sv))->xmg_u.xmg_magic);
    for (mg = *mgp; mg; mg = *mgp) {
	const MGVTBL* const virt = mg->mg_virtual;
	if (mg->mg_type == type && (!flags || virt == vtbl)) {
	    *mgp = mg->mg_moremagic;
	    if (virt && virt->svt_free)
		virt->svt_free(aTHX_ sv, mg);
	    if (mg->mg_ptr && mg->mg_type != PERL_MAGIC_regex_global) {
		if (mg->mg_len > 0)
		    Safefree(mg->mg_ptr);
		else if (mg->mg_len == HEf_SVKEY)
		    SvREFCNT_dec(MUTABLE_SV(mg->mg_ptr));
		else if (mg->mg_type == PERL_MAGIC_utf8)
		    Safefree(mg->mg_ptr);
            }
	    if (mg->mg_flags & MGf_REFCOUNTED)
		SvREFCNT_dec(mg->mg_obj);
	    Safefree(mg);
	}
	else
	    mgp = &mg->mg_moremagic;
    }
    if (SvMAGIC(sv)) {
	if (SvMAGICAL(sv))	/* if we're under save_magic, wait for restore_magic; */
	    mg_magical(sv);	/*    else fix the flags now */
    }
    else {
	SvMAGICAL_off(sv);
	SvFLAGS(sv) |= (SvFLAGS(sv) & (SVp_IOK|SVp_NOK|SVp_POK)) >> PRIVSHIFT;
    }
    return 0;
}

/*
=for apidoc sv_unmagic

Removes all magic of type C<type> from an SV.

=cut
*/

int
Perl_sv_unmagic(pTHX_ SV *const sv, const int type)
{
    PERL_ARGS_ASSERT_SV_UNMAGIC;
    return S_sv_unmagicext_flags(aTHX_ sv, type, NULL, 0);
}

/*
=for apidoc sv_unmagicext

Removes all magic of type C<type> with the specified C<vtbl> from an SV.

=cut
*/

int
Perl_sv_unmagicext(pTHX_ SV *const sv, const int type, MGVTBL *vtbl)
{
    PERL_ARGS_ASSERT_SV_UNMAGICEXT;
    return S_sv_unmagicext_flags(aTHX_ sv, type, vtbl, 1);
}

/*
=for apidoc sv_rvweaken

Weaken a reference: set the C<SvWEAKREF> flag on this RV; give the
referred-to SV C<PERL_MAGIC_backref> magic if it hasn't already; and
push a back-reference to this RV onto the array of backreferences
associated with that magic.  If the RV is magical, set magic will be
called after the RV is cleared.

=cut
*/

SV *
Perl_sv_rvweaken(pTHX_ SV *const sv)
{
    SV *tsv;

    PERL_ARGS_ASSERT_SV_RVWEAKEN;

    if (!SvOK(sv))  /* let undefs pass */
	return sv;
    if (!SvROK(sv))
	Perl_croak(aTHX_ "Can't weaken a nonreference");
    else if (SvWEAKREF(sv)) {
	Perl_ck_warner(aTHX_ packWARN(WARN_MISC), "Reference is already weak");
	return sv;
    }
    else if (SvREADONLY(sv)) croak_no_modify();
    tsv = SvRV(sv);
    Perl_sv_add_backref(aTHX_ tsv, sv);
    SvWEAKREF_on(sv);
    SvREFCNT_dec(tsv);
    return sv;
}

/* Give tsv backref magic if it hasn't already got it, then push a
 * back-reference to sv onto the array associated with the backref magic.
 *
 * As an optimisation, if there's only one backref and it's not an AV,
 * store it directly in the HvAUX or mg_obj slot, avoiding the need to
 * allocate an AV. (Whether the slot holds an AV tells us whether this is
 * active.)
 */

/* A discussion about the backreferences array and its refcount:
 *
 * The AV holding the backreferences is pointed to either as the mg_obj of
 * PERL_MAGIC_backref, or in the specific case of a HV, from the
 * xhv_backreferences field. The array is created with a refcount
 * of 2. This means that if during global destruction the array gets
 * picked on before its parent to have its refcount decremented by the
 * random zapper, it won't actually be freed, meaning it's still there for
 * when its parent gets freed.
 *
 * When the parent SV is freed, the extra ref is killed by
 * Perl_sv_kill_backrefs.  The other ref is killed, in the case of magic,
 * by mg_free() / MGf_REFCOUNTED, or for a hash, by Perl_hv_kill_backrefs.
 *
 * When a single backref SV is stored directly, it is not reference
 * counted.
 */

void
Perl_sv_add_backref(pTHX_ SV *const tsv, SV *const sv)
{
    dVAR;
    SV **svp;
    AV *av = NULL;
    MAGIC *mg = NULL;

    PERL_ARGS_ASSERT_SV_ADD_BACKREF;

    /* find slot to store array or singleton backref */

    if (SvTYPE(tsv) == SVt_PVHV) {
	svp = (SV**)Perl_hv_backreferences_p(aTHX_ MUTABLE_HV(tsv));
    } else {
	if (! ((mg =
	    (SvMAGICAL(tsv) ? mg_find(tsv, PERL_MAGIC_backref) : NULL))))
	{
	    sv_magic(tsv, NULL, PERL_MAGIC_backref, NULL, 0);
	    mg = mg_find(tsv, PERL_MAGIC_backref);
	}
	svp = &(mg->mg_obj);
    }

    /* create or retrieve the array */

    if (   (!*svp && SvTYPE(sv) == SVt_PVAV)
	|| (*svp && SvTYPE(*svp) != SVt_PVAV)
    ) {
	/* create array */
	av = newAV();
	AvREAL_off(av);
	SvREFCNT_inc_simple_void(av);
	/* av now has a refcnt of 2; see discussion above */
	if (*svp) {
	    /* move single existing backref to the array */
	    av_extend(av, 1);
	    AvARRAY(av)[++AvFILLp(av)] = *svp; /* av_push() */
	}
	*svp = (SV*)av;
	if (mg)
	    mg->mg_flags |= MGf_REFCOUNTED;
    }
    else
	av = MUTABLE_AV(*svp);

    if (!av) {
	/* optimisation: store single backref directly in HvAUX or mg_obj */
	*svp = sv;
	return;
    }
    /* push new backref */
    assert(SvTYPE(av) == SVt_PVAV);
    if (AvFILLp(av) >= AvMAX(av)) {
        av_extend(av, AvFILLp(av)+1);
    }
    AvARRAY(av)[++AvFILLp(av)] = sv; /* av_push() */
}

/* delete a back-reference to ourselves from the backref magic associated
 * with the SV we point to.
 */

void
Perl_sv_del_backref(pTHX_ SV *const tsv, SV *const sv)
{
    dVAR;
    SV **svp = NULL;

    PERL_ARGS_ASSERT_SV_DEL_BACKREF;

    if (SvTYPE(tsv) == SVt_PVHV) {
	if (SvOOK(tsv))
	    svp = (SV**)Perl_hv_backreferences_p(aTHX_ MUTABLE_HV(tsv));
    }
    else if (SvIS_FREED(tsv) && PL_phase == PERL_PHASE_DESTRUCT) {
	/* It's possible for the the last (strong) reference to tsv to have
	   become freed *before* the last thing holding a weak reference.
	   If both survive longer than the backreferences array, then when
	   the referent's reference count drops to 0 and it is freed, it's
	   not able to chase the backreferences, so they aren't NULLed.

	   For example, a CV holds a weak reference to its stash. If both the
	   CV and the stash survive longer than the backreferences array,
	   and the CV gets picked for the SvBREAK() treatment first,
	   *and* it turns out that the stash is only being kept alive because
	   of an our variable in the pad of the CV, then midway during CV
	   destruction the stash gets freed, but CvSTASH() isn't set to NULL.
	   It ends up pointing to the freed HV. Hence it's chased in here, and
	   if this block wasn't here, it would hit the !svp panic just below.

	   I don't believe that "better" destruction ordering is going to help
	   here - during global destruction there's always going to be the
	   chance that something goes out of order. We've tried to make it
	   foolproof before, and it only resulted in evolutionary pressure on
	   fools. Which made us look foolish for our hubris. :-(
	*/
	return;
    }
    else {
	MAGIC *const mg
	    = SvMAGICAL(tsv) ? mg_find(tsv, PERL_MAGIC_backref) : NULL;
	svp =  mg ? &(mg->mg_obj) : NULL;
    }

    if (!svp)
	Perl_croak(aTHX_ "panic: del_backref, svp=0");
    if (!*svp) {
	/* It's possible that sv is being freed recursively part way through the
	   freeing of tsv. If this happens, the backreferences array of tsv has
	   already been freed, and so svp will be NULL. If this is the case,
	   we should not panic. Instead, nothing needs doing, so return.  */
	if (PL_phase == PERL_PHASE_DESTRUCT && SvREFCNT(tsv) == 0)
	    return;
	Perl_croak(aTHX_ "panic: del_backref, *svp=%p phase=%s refcnt=%" UVuf,
		   *svp, PL_phase_names[PL_phase], (UV)SvREFCNT(tsv));
    }

    if (SvTYPE(*svp) == SVt_PVAV) {
#ifdef DEBUGGING
	int count = 1;
#endif
	AV * const av = (AV*)*svp;
	SSize_t fill;
	assert(!SvIS_FREED(av));
	fill = AvFILLp(av);
	assert(fill > -1);
	svp = AvARRAY(av);
	/* for an SV with N weak references to it, if all those
	 * weak refs are deleted, then sv_del_backref will be called
	 * N times and O(N^2) compares will be done within the backref
	 * array. To ameliorate this potential slowness, we:
	 * 1) make sure this code is as tight as possible;
	 * 2) when looking for SV, look for it at both the head and tail of the
	 *    array first before searching the rest, since some create/destroy
	 *    patterns will cause the backrefs to be freed in order.
	 */
	if (*svp == sv) {
	    AvARRAY(av)++;
	    AvMAX(av)--;
	}
	else {
	    SV **p = &svp[fill];
	    SV *const topsv = *p;
	    if (topsv != sv) {
#ifdef DEBUGGING
		count = 0;
#endif
		while (--p > svp) {
		    if (*p == sv) {
			/* We weren't the last entry.
			   An unordered list has this property that you
			   can take the last element off the end to fill
			   the hole, and it's still an unordered list :-)
			*/
			*p = topsv;
#ifdef DEBUGGING
			count++;
#else
			break; /* should only be one */
#endif
		    }
		}
	    }
	}
	assert(count ==1);
	AvFILLp(av) = fill-1;
    }
    else if (SvIS_FREED(*svp) && PL_phase == PERL_PHASE_DESTRUCT) {
	/* freed AV; skip */
    }
    else {
	/* optimisation: only a single backref, stored directly */
	if (*svp != sv)
	    Perl_croak(aTHX_ "panic: del_backref, *svp=%p, sv=%p", *svp, sv);
	*svp = NULL;
    }

}

void
Perl_sv_kill_backrefs(pTHX_ SV *const sv, AV *const av)
{
    SV **svp;
    SV **last;
    bool is_array;

    PERL_ARGS_ASSERT_SV_KILL_BACKREFS;

    if (!av)
	return;

    /* after multiple passes through Perl_sv_clean_all() for a thinngy
     * that has badly leaked, the backref array may have gotten freed,
     * since we only protect it against 1 round of cleanup */
    if (SvIS_FREED(av)) {
	if (PL_in_clean_all) /* All is fair */
	    return;
	Perl_croak(aTHX_
		   "panic: magic_killbackrefs (freed backref AV/SV)");
    }


    is_array = (SvTYPE(av) == SVt_PVAV);
    if (is_array) {
	assert(!SvIS_FREED(av));
	svp = AvARRAY(av);
	if (svp)
	    last = svp + AvFILLp(av);
    }
    else {
	/* optimisation: only a single backref, stored directly */
	svp = (SV**)&av;
	last = svp;
    }

    if (svp) {
	while (svp <= last) {
	    if (*svp) {
		SV *const referrer = *svp;
		if (SvWEAKREF(referrer)) {
		    /* XXX Should we check that it hasn't changed? */
		    assert(SvROK(referrer));
		    SvRV_set(referrer, 0);
		    SvOK_off(referrer);
		    SvWEAKREF_off(referrer);
		    SvSETMAGIC(referrer);
		} else if (SvTYPE(referrer) == SVt_PVGV ||
			   SvTYPE(referrer) == SVt_PVLV) {
		    assert(SvTYPE(sv) == SVt_PVHV); /* stash backref */
		    /* You lookin' at me?  */
		    assert(GvSTASH(referrer));
		    assert(GvSTASH(referrer) == (const HV *)sv);
		    GvSTASH(referrer) = 0;
		} else if (SvTYPE(referrer) == SVt_PVCV ||
			   SvTYPE(referrer) == SVt_PVFM) {
		    if (SvTYPE(sv) == SVt_PVHV) { /* stash backref */
			/* You lookin' at me?  */
			assert(CvSTASH(referrer));
			assert(CvSTASH(referrer) == (const HV *)sv);
			SvANY(MUTABLE_CV(referrer))->xcv_stash = 0;
		    }
		    else {
			assert(SvTYPE(sv) == SVt_PVGV);
			/* You lookin' at me?  */
			assert(CvGV(referrer));
			assert(CvGV(referrer) == (const GV *)sv);
			anonymise_cv_maybe(MUTABLE_GV(sv),
						MUTABLE_CV(referrer));
		    }

		} else {
		    Perl_croak(aTHX_
			       "panic: magic_killbackrefs (flags=%"UVxf")",
			       (UV)SvFLAGS(referrer));
		}

		if (is_array)
		    *svp = NULL;
	    }
	    svp++;
	}
    }
    if (is_array) {
	AvFILLp(av) = -1;
	SvREFCNT_dec(av); /* remove extra count added by sv_add_backref() */
    }
    return;
}

/*
=for apidoc sv_insert

Inserts a string at the specified offset/length within the SV.  Similar to
the Perl substr() function.  Handles get magic.

=for apidoc sv_insert_flags

Same as C<sv_insert>, but the extra C<flags> are passed to the
C<SvPV_force_flags> that applies to C<bigstr>.

=cut
*/

void
Perl_sv_insert_flags(pTHX_ SV *const bigstr, const STRLEN offset, const STRLEN len, const char *const little, const STRLEN littlelen, const U32 flags)
{
    dVAR;
    register char *big;
    register char *mid;
    register char *midend;
    register char *bigend;
    register SSize_t i;		/* better be sizeof(STRLEN) or bad things happen */
    STRLEN curlen;

    PERL_ARGS_ASSERT_SV_INSERT_FLAGS;

    if (!bigstr)
	Perl_croak(aTHX_ "Can't modify nonexistent substring");
    SvPV_force_flags(bigstr, curlen, flags);
    (void)SvPOK_only_UTF8(bigstr);
    if (offset + len > curlen) {
	SvGROW(bigstr, offset+len+1);
	Zero(SvPVX(bigstr)+curlen, offset+len-curlen, char);
	SvCUR_set(bigstr, offset+len);
    }

    SvTAINT(bigstr);
    i = littlelen - len;
    if (i > 0) {			/* string might grow */
	big = SvGROW(bigstr, SvCUR(bigstr) + i + 1);
	mid = big + offset + len;
	midend = bigend = big + SvCUR(bigstr);
	bigend += i;
	*bigend = '\0';
	while (midend > mid)		/* shove everything down */
	    *--bigend = *--midend;
	Move(little,big+offset,littlelen,char);
	SvCUR_set(bigstr, SvCUR(bigstr) + i);
	SvSETMAGIC(bigstr);
	return;
    }
    else if (i == 0) {
	Move(little,SvPVX(bigstr)+offset,len,char);
	SvSETMAGIC(bigstr);
	return;
    }

    big = SvPVX(bigstr);
    mid = big + offset;
    midend = mid + len;
    bigend = big + SvCUR(bigstr);

    if (midend > bigend)
	Perl_croak(aTHX_ "panic: sv_insert, midend=%p, bigend=%p",
		   midend, bigend);

    if (mid - big > bigend - midend) {	/* faster to shorten from end */
	if (littlelen) {
	    Move(little, mid, littlelen,char);
	    mid += littlelen;
	}
	i = bigend - midend;
	if (i > 0) {
	    Move(midend, mid, i,char);
	    mid += i;
	}
	*mid = '\0';
	SvCUR_set(bigstr, mid - big);
    }
    else if ((i = mid - big)) {	/* faster from front */
	midend -= littlelen;
	mid = midend;
	Move(big, midend - i, i, char);
	sv_chop(bigstr,midend-i);
	if (littlelen)
	    Move(little, mid, littlelen,char);
    }
    else if (littlelen) {
	midend -= littlelen;
	sv_chop(bigstr,midend);
	Move(little,midend,littlelen,char);
    }
    else {
	sv_chop(bigstr,midend);
    }
    SvSETMAGIC(bigstr);
}

/*
=for apidoc sv_replace

Make the first argument a copy of the second, then delete the original.
The target SV physically takes over ownership of the body of the source SV
and inherits its flags; however, the target keeps any magic it owns,
and any magic in the source is discarded.
Note that this is a rather specialist SV copying operation; most of the
time you'll want to use C<sv_setsv> or one of its many macro front-ends.

=cut
*/

void
Perl_sv_replace(pTHX_ register SV *const sv, register SV *const nsv)
{
    dVAR;
    const U32 refcnt = SvREFCNT(sv);

    PERL_ARGS_ASSERT_SV_REPLACE;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    if (SvREFCNT(nsv) != 1) {
	Perl_croak(aTHX_ "panic: reference miscount on nsv in sv_replace()"
		   " (%" UVuf " != 1)", (UV) SvREFCNT(nsv));
    }
    if (SvMAGICAL(sv)) {
	if (SvMAGICAL(nsv))
	    mg_free(nsv);
	else
	    sv_upgrade(nsv, SVt_PVMG);
	SvMAGIC_set(nsv, SvMAGIC(sv));
	SvFLAGS(nsv) |= SvMAGICAL(sv);
	SvMAGICAL_off(sv);
	SvMAGIC_set(sv, NULL);
    }
    SvREFCNT(sv) = 0;
    sv_clear(sv);
    assert(!SvREFCNT(sv));
#ifdef DEBUG_LEAKING_SCALARS
    sv->sv_flags  = nsv->sv_flags;
    sv->sv_any    = nsv->sv_any;
    sv->sv_refcnt = nsv->sv_refcnt;
    sv->sv_u      = nsv->sv_u;
#else
    StructCopy(nsv,sv,SV);
#endif
    if(SvTYPE(sv) == SVt_IV) {
	SvANY(sv)
	    = (XPVIV*)((char*)&(sv->sv_u.svu_iv) - STRUCT_OFFSET(XPVIV, xiv_iv));
    }
	

#ifdef PERL_OLD_COPY_ON_WRITE
    if (SvIsCOW_normal(nsv)) {
	/* We need to follow the pointers around the loop to make the
	   previous SV point to sv, rather than nsv.  */
	SV *next;
	SV *current = nsv;
	while ((next = SV_COW_NEXT_SV(current)) != nsv) {
	    assert(next);
	    current = next;
	    assert(SvPVX_const(current) == SvPVX_const(nsv));
	}
	/* Make the SV before us point to the SV after us.  */
	if (DEBUG_C_TEST) {
	    PerlIO_printf(Perl_debug_log, "previous is\n");
	    sv_dump(current);
	    PerlIO_printf(Perl_debug_log,
                          "move it from 0x%"UVxf" to 0x%"UVxf"\n",
			  (UV) SV_COW_NEXT_SV(current), (UV) sv);
	}
	SV_COW_NEXT_SV_SET(current, sv);
    }
#endif
    SvREFCNT(sv) = refcnt;
    SvFLAGS(nsv) |= SVTYPEMASK;		/* Mark as freed */
    SvREFCNT(nsv) = 0;
    del_SV(nsv);
}

/* We're about to free a GV which has a CV that refers back to us.
 * If that CV will outlive us, make it anonymous (i.e. fix up its CvGV
 * field) */

STATIC void
S_anonymise_cv_maybe(pTHX_ GV *gv, CV* cv)
{
    SV *gvname;
    GV *anongv;

    PERL_ARGS_ASSERT_ANONYMISE_CV_MAYBE;

    /* be assertive! */
    assert(SvREFCNT(gv) == 0);
    assert(isGV(gv) && isGV_with_GP(gv));
    assert(GvGP(gv));
    assert(!CvANON(cv));
    assert(CvGV(cv) == gv);

    /* will the CV shortly be freed by gp_free() ? */
    if (GvCV(gv) == cv && GvGP(gv)->gp_refcnt < 2 && SvREFCNT(cv) < 2) {
	SvANY(cv)->xcv_gv = NULL;
	return;
    }

    /* if not, anonymise: */
    gvname = (GvSTASH(gv) && HvNAME(GvSTASH(gv)) && HvENAME(GvSTASH(gv)))
                    ? newSVhek(HvENAME_HEK(GvSTASH(gv)))
                    : newSVpvn_flags( "__ANON__", 8, 0 );
    sv_catpvs(gvname, "::__ANON__");
    anongv = gv_fetchsv(gvname, GV_ADDMULTI, SVt_PVCV);
    SvREFCNT_dec(gvname);

    CvANON_on(cv);
    CvCVGV_RC_on(cv);
    SvANY(cv)->xcv_gv = MUTABLE_GV(SvREFCNT_inc(anongv));
}


/*
=for apidoc sv_clear

Clear an SV: call any destructors, free up any memory used by the body,
and free the body itself.  The SV's head is I<not> freed, although
its type is set to all 1's so that it won't inadvertently be assumed
to be live during global destruction etc.
This function should only be called when REFCNT is zero.  Most of the time
you'll want to call C<sv_free()> (or its macro wrapper C<SvREFCNT_dec>)
instead.

=cut
*/

void
Perl_sv_clear(pTHX_ SV *const orig_sv)
{
    dVAR;
    HV *stash;
    U32 type;
    const struct body_details *sv_type_details;
    SV* iter_sv = NULL;
    SV* next_sv = NULL;
    register SV *sv = orig_sv;
    STRLEN hash_index;

    PERL_ARGS_ASSERT_SV_CLEAR;

    /* within this loop, sv is the SV currently being freed, and
     * iter_sv is the most recent AV or whatever that's being iterated
     * over to provide more SVs */

    while (sv) {

	type = SvTYPE(sv);

	assert(SvREFCNT(sv) == 0);
	assert(SvTYPE(sv) != (svtype)SVTYPEMASK);

	if (type <= SVt_IV) {
	    /* See the comment in sv.h about the collusion between this
	     * early return and the overloading of the NULL slots in the
	     * size table.  */
	    if (SvROK(sv))
		goto free_rv;
	    SvFLAGS(sv) &= SVf_BREAK;
	    SvFLAGS(sv) |= SVTYPEMASK;
	    goto free_head;
	}

	assert(!SvOBJECT(sv) || type >= SVt_PVMG); /* objs are always >= MG */

	if (type >= SVt_PVMG) {
	    if (SvOBJECT(sv)) {
		if (!curse(sv, 1)) goto get_next_sv;
		type = SvTYPE(sv); /* destructor may have changed it */
	    }
	    /* Free back-references before magic, in case the magic calls
	     * Perl code that has weak references to sv. */
	    if (type == SVt_PVHV) {
		Perl_hv_kill_backrefs(aTHX_ MUTABLE_HV(sv));
		if (SvMAGIC(sv))
		    mg_free(sv);
	    }
	    else if (type == SVt_PVMG && SvPAD_OUR(sv)) {
		SvREFCNT_dec(SvOURSTASH(sv));
	    } else if (SvMAGIC(sv)) {
		/* Free back-references before other types of magic. */
		sv_unmagic(sv, PERL_MAGIC_backref);
		mg_free(sv);
	    }
	    SvMAGICAL_off(sv);
	    if (type == SVt_PVMG && SvPAD_TYPED(sv))
		SvREFCNT_dec(SvSTASH(sv));
	}
	switch (type) {
	    /* case SVt_BIND: */
	case SVt_PVIO:
	    if (IoIFP(sv) &&
		IoIFP(sv) != PerlIO_stdin() &&
		IoIFP(sv) != PerlIO_stdout() &&
		IoIFP(sv) != PerlIO_stderr() &&
		!(IoFLAGS(sv) & IOf_FAKE_DIRP))
	    {
		io_close(MUTABLE_IO(sv), FALSE);
	    }
	    if (IoDIRP(sv) && !(IoFLAGS(sv) & IOf_FAKE_DIRP))
		PerlDir_close(IoDIRP(sv));
	    IoDIRP(sv) = (DIR*)NULL;
	    Safefree(IoTOP_NAME(sv));
	    Safefree(IoFMT_NAME(sv));
	    Safefree(IoBOTTOM_NAME(sv));
	    if ((const GV *)sv == PL_statgv)
		PL_statgv = NULL;
	    goto freescalar;
	case SVt_REGEXP:
	    /* FIXME for plugins */
	    pregfree2((REGEXP*) sv);
	    goto freescalar;
	case SVt_PVCV:
	case SVt_PVFM:
	    cv_undef(MUTABLE_CV(sv));
	    /* If we're in a stash, we don't own a reference to it.
	     * However it does have a back reference to us, which needs to
	     * be cleared.  */
	    if ((stash = CvSTASH(sv)))
		sv_del_backref(MUTABLE_SV(stash), sv);
	    goto freescalar;
	case SVt_PVHV:
	    if (PL_last_swash_hv == (const HV *)sv) {
		PL_last_swash_hv = NULL;
	    }
	    if (HvTOTALKEYS((HV*)sv) > 0) {
		const char *name;
		/* this statement should match the one at the beginning of
		 * hv_undef_flags() */
		if (   PL_phase != PERL_PHASE_DESTRUCT
		    && (name = HvNAME((HV*)sv)))
		{
		    if (PL_stashcache)
			(void)hv_delete(PL_stashcache, name,
			    HvNAMEUTF8((HV*)sv) ? -HvNAMELEN_get((HV*)sv) : HvNAMELEN_get((HV*)sv), G_DISCARD);
		    hv_name_set((HV*)sv, NULL, 0, 0);
		}

		/* save old iter_sv in unused SvSTASH field */
		assert(!SvOBJECT(sv));
		SvSTASH(sv) = (HV*)iter_sv;
		iter_sv = sv;

		/* save old hash_index in unused SvMAGIC field */
		assert(!SvMAGICAL(sv));
		assert(!SvMAGIC(sv));
		((XPVMG*) SvANY(sv))->xmg_u.xmg_hash_index = hash_index;
		hash_index = 0;

		next_sv = Perl_hfree_next_entry(aTHX_ (HV*)sv, &hash_index);
		goto get_next_sv; /* process this new sv */
	    }
	    /* free empty hash */
	    Perl_hv_undef_flags(aTHX_ MUTABLE_HV(sv), HV_NAME_SETALL);
	    assert(!HvARRAY((HV*)sv));
	    break;
	case SVt_PVAV:
	    {
		AV* av = MUTABLE_AV(sv);
		if (PL_comppad == av) {
		    PL_comppad = NULL;
		    PL_curpad = NULL;
		}
		if (AvREAL(av) && AvFILLp(av) > -1) {
		    next_sv = AvARRAY(av)[AvFILLp(av)--];
		    /* save old iter_sv in top-most slot of AV,
		     * and pray that it doesn't get wiped in the meantime */
		    AvARRAY(av)[AvMAX(av)] = iter_sv;
		    iter_sv = sv;
		    goto get_next_sv; /* process this new sv */
		}
		Safefree(AvALLOC(av));
	    }

	    break;
	case SVt_PVLV:
	    if (LvTYPE(sv) == 'T') { /* for tie: return HE to pool */
		SvREFCNT_dec(HeKEY_sv((HE*)LvTARG(sv)));
		HeNEXT((HE*)LvTARG(sv)) = PL_hv_fetch_ent_mh;
		PL_hv_fetch_ent_mh = (HE*)LvTARG(sv);
	    }
	    else if (LvTYPE(sv) != 't') /* unless tie: unrefcnted fake SV**  */
		SvREFCNT_dec(LvTARG(sv));
	case SVt_PVGV:
	    if (isGV_with_GP(sv)) {
		if(GvCVu((const GV *)sv) && (stash = GvSTASH(MUTABLE_GV(sv)))
		   && HvENAME_get(stash))
		    mro_method_changed_in(stash);
		gp_free(MUTABLE_GV(sv));
		if (GvNAME_HEK(sv))
		    unshare_hek(GvNAME_HEK(sv));
		/* If we're in a stash, we don't own a reference to it.
		 * However it does have a back reference to us, which
		 * needs to be cleared.  */
		if (!SvVALID(sv) && (stash = GvSTASH(sv)))
			sv_del_backref(MUTABLE_SV(stash), sv);
	    }
	    /* FIXME. There are probably more unreferenced pointers to SVs
	     * in the interpreter struct that we should check and tidy in
	     * a similar fashion to this:  */
	    /* See also S_sv_unglob, which does the same thing. */
	    if ((const GV *)sv == PL_last_in_gv)
		PL_last_in_gv = NULL;
	    else if ((const GV *)sv == PL_statgv)
		PL_statgv = NULL;
	case SVt_PVMG:
	case SVt_PVNV:
	case SVt_PVIV:
	case SVt_PV:
	  freescalar:
	    /* Don't bother with SvOOK_off(sv); as we're only going to
	     * free it.  */
	    if (SvOOK(sv)) {
		STRLEN offset;
		SvOOK_offset(sv, offset);
		SvPV_set(sv, SvPVX_mutable(sv) - offset);
		/* Don't even bother with turning off the OOK flag.  */
	    }
	    if (SvROK(sv)) {
	    free_rv:
		{
		    SV * const target = SvRV(sv);
		    if (SvWEAKREF(sv))
			sv_del_backref(target, sv);
		    else
			next_sv = target;
		}
	    }
#ifdef PERL_OLD_COPY_ON_WRITE
	    else if (SvPVX_const(sv)
		     && !(SvTYPE(sv) == SVt_PVIO
		     && !(IoFLAGS(sv) & IOf_FAKE_DIRP)))
	    {
		if (SvIsCOW(sv)) {
		    if (DEBUG_C_TEST) {
			PerlIO_printf(Perl_debug_log, "Copy on write: clear\n");
			sv_dump(sv);
		    }
		    if (SvLEN(sv)) {
			sv_release_COW(sv, SvPVX_const(sv), SV_COW_NEXT_SV(sv));
		    } else {
			unshare_hek(SvSHARED_HEK_FROM_PV(SvPVX_const(sv)));
		    }

		    SvFAKE_off(sv);
		} else if (SvLEN(sv)) {
		    Safefree(SvPVX_const(sv));
		}
	    }
#else
	    else if (SvPVX_const(sv) && SvLEN(sv)
		     && !(SvTYPE(sv) == SVt_PVIO
		     && !(IoFLAGS(sv) & IOf_FAKE_DIRP)))
		Safefree(SvPVX_mutable(sv));
	    else if (SvPVX_const(sv) && SvIsCOW(sv)) {
		unshare_hek(SvSHARED_HEK_FROM_PV(SvPVX_const(sv)));
		SvFAKE_off(sv);
	    }
#endif
	    break;
	case SVt_NV:
	    break;
	}

      free_body:

	SvFLAGS(sv) &= SVf_BREAK;
	SvFLAGS(sv) |= SVTYPEMASK;

	sv_type_details = bodies_by_type + type;
	if (sv_type_details->arena) {
	    del_body(((char *)SvANY(sv) + sv_type_details->offset),
		     &PL_body_roots[type]);
	}
	else if (sv_type_details->body_size) {
	    safefree(SvANY(sv));
	}

      free_head:
	/* caller is responsible for freeing the head of the original sv */
	if (sv != orig_sv && !SvREFCNT(sv))
	    del_SV(sv);

	/* grab and free next sv, if any */
      get_next_sv:
	while (1) {
	    sv = NULL;
	    if (next_sv) {
		sv = next_sv;
		next_sv = NULL;
	    }
	    else if (!iter_sv) {
		break;
	    } else if (SvTYPE(iter_sv) == SVt_PVAV) {
		AV *const av = (AV*)iter_sv;
		if (AvFILLp(av) > -1) {
		    sv = AvARRAY(av)[AvFILLp(av)--];
		}
		else { /* no more elements of current AV to free */
		    sv = iter_sv;
		    type = SvTYPE(sv);
		    /* restore previous value, squirrelled away */
		    iter_sv = AvARRAY(av)[AvMAX(av)];
		    Safefree(AvALLOC(av));
		    goto free_body;
		}
	    } else if (SvTYPE(iter_sv) == SVt_PVHV) {
		sv = Perl_hfree_next_entry(aTHX_ (HV*)iter_sv, &hash_index);
		if (!sv && !HvTOTALKEYS((HV *)iter_sv)) {
		    /* no more elements of current HV to free */
		    sv = iter_sv;
		    type = SvTYPE(sv);
		    /* Restore previous values of iter_sv and hash_index,
		     * squirrelled away */
		    assert(!SvOBJECT(sv));
		    iter_sv = (SV*)SvSTASH(sv);
		    assert(!SvMAGICAL(sv));
		    hash_index = ((XPVMG*) SvANY(sv))->xmg_u.xmg_hash_index;

		    /* free any remaining detritus from the hash struct */
		    Perl_hv_undef_flags(aTHX_ MUTABLE_HV(sv), HV_NAME_SETALL);
		    assert(!HvARRAY((HV*)sv));
		    goto free_body;
		}
	    }

	    /* unrolled SvREFCNT_dec and sv_free2 follows: */

	    if (!sv)
		continue;
	    if (!SvREFCNT(sv)) {
		sv_free(sv);
		continue;
	    }
	    if (--(SvREFCNT(sv)))
		continue;
#ifdef DEBUGGING
	    if (SvTEMP(sv)) {
		Perl_ck_warner_d(aTHX_ packWARN(WARN_DEBUGGING),
			 "Attempt to free temp prematurely: SV 0x%"UVxf
			 pTHX__FORMAT, PTR2UV(sv) pTHX__VALUE);
		continue;
	    }
#endif
	    if (SvREADONLY(sv) && SvIMMORTAL(sv)) {
		/* make sure SvREFCNT(sv)==0 happens very seldom */
		SvREFCNT(sv) = (~(U32)0)/2;
		continue;
	    }
	    break;
	} /* while 1 */

    } /* while sv */
}

/* This routine curses the sv itself, not the object referenced by sv. So
   sv does not have to be ROK. */

static bool
S_curse(pTHX_ SV * const sv, const bool check_refcnt) {
    dVAR;

    PERL_ARGS_ASSERT_CURSE;
    assert(SvOBJECT(sv));

    if (PL_defstash &&	/* Still have a symbol table? */
	SvDESTROYABLE(sv))
    {
	dSP;
	HV* stash;
	do {
	    CV* destructor;
	    stash = SvSTASH(sv);
	    destructor = StashHANDLER(stash,DESTROY);
	    if (destructor
		/* A constant subroutine can have no side effects, so
		   don't bother calling it.  */
		&& !CvCONST(destructor)
		/* Don't bother calling an empty destructor or one that
		   returns immediately. */
		&& (CvISXSUB(destructor)
		|| (CvSTART(destructor)
		    && (CvSTART(destructor)->op_next->op_type
					!= OP_LEAVESUB)
		    && (CvSTART(destructor)->op_next->op_type
					!= OP_PUSHMARK
			|| CvSTART(destructor)->op_next->op_next->op_type
					!= OP_RETURN
		       )
		   ))
	       )
	    {
		SV* const tmpref = newRV(sv);
		SvREADONLY_on(tmpref); /* DESTROY() could be naughty */
		ENTER;
		PUSHSTACKi(PERLSI_DESTROY);
		EXTEND(SP, 2);
		PUSHMARK(SP);
		PUSHs(tmpref);
		PUTBACK;
		call_sv(MUTABLE_SV(destructor),
			    G_DISCARD|G_EVAL|G_KEEPERR|G_VOID);
		POPSTACK;
		SPAGAIN;
		LEAVE;
		if(SvREFCNT(tmpref) < 2) {
		    /* tmpref is not kept alive! */
		    SvREFCNT(sv)--;
		    SvRV_set(tmpref, NULL);
		    SvROK_off(tmpref);
		}
		SvREFCNT_dec(tmpref);
	    }
	} while (SvOBJECT(sv) && SvSTASH(sv) != stash);


	if (check_refcnt && SvREFCNT(sv)) {
	    if (PL_in_clean_objs)
		Perl_croak(aTHX_
		  "DESTROY created new reference to dead object '%"HEKf"'",
		   HEKfARG(HvNAME_HEK(stash)));
	    /* DESTROY gave object new lease on life */
	    return FALSE;
	}
    }

    if (SvOBJECT(sv)) {
	SvREFCNT_dec(SvSTASH(sv)); /* possibly of changed persuasion */
	SvOBJECT_off(sv);	/* Curse the object. */
	if (SvTYPE(sv) != SVt_PVIO)
	    --PL_sv_objcount;/* XXX Might want something more general */
    }
    return TRUE;
}

/*
=for apidoc sv_newref

Increment an SV's reference count.  Use the C<SvREFCNT_inc()> wrapper
instead.

=cut
*/

SV *
Perl_sv_newref(pTHX_ SV *const sv)
{
    PERL_UNUSED_CONTEXT;
    if (sv)
	(SvREFCNT(sv))++;
    return sv;
}

/*
=for apidoc sv_free

Decrement an SV's reference count, and if it drops to zero, call
C<sv_clear> to invoke destructors and free up any memory used by
the body; finally, deallocate the SV's head itself.
Normally called via a wrapper macro C<SvREFCNT_dec>.

=cut
*/

void
Perl_sv_free(pTHX_ SV *const sv)
{
    dVAR;
    if (!sv)
	return;
    if (SvREFCNT(sv) == 0) {
	if (SvFLAGS(sv) & SVf_BREAK)
	    /* this SV's refcnt has been artificially decremented to
	     * trigger cleanup */
	    return;
	if (PL_in_clean_all) /* All is fair */
	    return;
	if (SvREADONLY(sv) && SvIMMORTAL(sv)) {
	    /* make sure SvREFCNT(sv)==0 happens very seldom */
	    SvREFCNT(sv) = (~(U32)0)/2;
	    return;
	}
	if (ckWARN_d(WARN_INTERNAL)) {
#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
	    Perl_dump_sv_child(aTHX_ sv);
#else
  #ifdef DEBUG_LEAKING_SCALARS
	    sv_dump(sv);
  #endif
#ifdef DEBUG_LEAKING_SCALARS_ABORT
	    if (PL_warnhook == PERL_WARNHOOK_FATAL
		|| ckDEAD(packWARN(WARN_INTERNAL))) {
		/* Don't let Perl_warner cause us to escape our fate:  */
		abort();
	    }
#endif
	    /* This may not return:  */
	    Perl_warner(aTHX_ packWARN(WARN_INTERNAL),
                        "Attempt to free unreferenced scalar: SV 0x%"UVxf
                        pTHX__FORMAT, PTR2UV(sv) pTHX__VALUE);
#endif
	}
#ifdef DEBUG_LEAKING_SCALARS_ABORT
	abort();
#endif
	return;
    }
    if (--(SvREFCNT(sv)) > 0)
	return;
    Perl_sv_free2(aTHX_ sv);
}

void
Perl_sv_free2(pTHX_ SV *const sv)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_FREE2;

#ifdef DEBUGGING
    if (SvTEMP(sv)) {
	Perl_ck_warner_d(aTHX_ packWARN(WARN_DEBUGGING),
			 "Attempt to free temp prematurely: SV 0x%"UVxf
			 pTHX__FORMAT, PTR2UV(sv) pTHX__VALUE);
	return;
    }
#endif
    if (SvREADONLY(sv) && SvIMMORTAL(sv)) {
	/* make sure SvREFCNT(sv)==0 happens very seldom */
	SvREFCNT(sv) = (~(U32)0)/2;
	return;
    }
    sv_clear(sv);
    if (! SvREFCNT(sv))
	del_SV(sv);
}

/*
=for apidoc sv_len

Returns the length of the string in the SV.  Handles magic and type
coercion.  See also C<SvCUR>, which gives raw access to the xpv_cur slot.

=cut
*/

STRLEN
Perl_sv_len(pTHX_ register SV *const sv)
{
    STRLEN len;

    if (!sv)
	return 0;

    if (SvGMAGICAL(sv))
	len = mg_length(sv);
    else
        (void)SvPV_const(sv, len);
    return len;
}

/*
=for apidoc sv_len_utf8

Returns the number of characters in the string in an SV, counting wide
UTF-8 bytes as a single character.  Handles magic and type coercion.

=cut
*/

/*
 * The length is cached in PERL_MAGIC_utf8, in the mg_len field.  Also the
 * mg_ptr is used, by sv_pos_u2b() and sv_pos_b2u() - see the comments below.
 * (Note that the mg_len is not the length of the mg_ptr field.
 * This allows the cache to store the character length of the string without
 * needing to malloc() extra storage to attach to the mg_ptr.)
 *
 */

STRLEN
Perl_sv_len_utf8(pTHX_ register SV *const sv)
{
    if (!sv)
	return 0;

    if (SvGMAGICAL(sv))
	return mg_length(sv);
    else
    {
	STRLEN len;
	const U8 *s = (U8*)SvPV_const(sv, len);

	if (PL_utf8cache) {
	    STRLEN ulen;
	    MAGIC *mg = SvMAGICAL(sv) ? mg_find(sv, PERL_MAGIC_utf8) : NULL;

	    if (mg && (mg->mg_len != -1 || mg->mg_ptr)) {
		if (mg->mg_len != -1)
		    ulen = mg->mg_len;
		else {
		    /* We can use the offset cache for a headstart.
		       The longer value is stored in the first pair.  */
		    STRLEN *cache = (STRLEN *) mg->mg_ptr;

		    ulen = cache[0] + Perl_utf8_length(aTHX_ s + cache[1],
						       s + len);
		}
		
		if (PL_utf8cache < 0) {
		    const STRLEN real = Perl_utf8_length(aTHX_ s, s + len);
		    assert_uft8_cache_coherent("sv_len_utf8", ulen, real, sv);
		}
	    }
	    else {
		ulen = Perl_utf8_length(aTHX_ s, s + len);
		utf8_mg_len_cache_update(sv, &mg, ulen);
	    }
	    return ulen;
	}
	return Perl_utf8_length(aTHX_ s, s + len);
    }
}

/* Walk forwards to find the byte corresponding to the passed in UTF-8
   offset.  */
static STRLEN
S_sv_pos_u2b_forwards(const U8 *const start, const U8 *const send,
		      STRLEN *const uoffset_p, bool *const at_end)
{
    const U8 *s = start;
    STRLEN uoffset = *uoffset_p;

    PERL_ARGS_ASSERT_SV_POS_U2B_FORWARDS;

    while (s < send && uoffset) {
	--uoffset;
	s += UTF8SKIP(s);
    }
    if (s == send) {
	*at_end = TRUE;
    }
    else if (s > send) {
	*at_end = TRUE;
	/* This is the existing behaviour. Possibly it should be a croak, as
	   it's actually a bounds error  */
	s = send;
    }
    *uoffset_p -= uoffset;
    return s - start;
}

/* Given the length of the string in both bytes and UTF-8 characters, decide
   whether to walk forwards or backwards to find the byte corresponding to
   the passed in UTF-8 offset.  */
static STRLEN
S_sv_pos_u2b_midway(const U8 *const start, const U8 *send,
		    STRLEN uoffset, const STRLEN uend)
{
    STRLEN backw = uend - uoffset;

    PERL_ARGS_ASSERT_SV_POS_U2B_MIDWAY;

    if (uoffset < 2 * backw) {
	/* The assumption is that going forwards is twice the speed of going
	   forward (that's where the 2 * backw comes from).
	   (The real figure of course depends on the UTF-8 data.)  */
	const U8 *s = start;

	while (s < send && uoffset--)
	    s += UTF8SKIP(s);
	assert (s <= send);
	if (s > send)
	    s = send;
	return s - start;
    }

    while (backw--) {
	send--;
	while (UTF8_IS_CONTINUATION(*send))
	    send--;
    }
    return send - start;
}

/* For the string representation of the given scalar, find the byte
   corresponding to the passed in UTF-8 offset.  uoffset0 and boffset0
   give another position in the string, *before* the sought offset, which
   (which is always true, as 0, 0 is a valid pair of positions), which should
   help reduce the amount of linear searching.
   If *mgp is non-NULL, it should point to the UTF-8 cache magic, which
   will be used to reduce the amount of linear searching. The cache will be
   created if necessary, and the found value offered to it for update.  */
static STRLEN
S_sv_pos_u2b_cached(pTHX_ SV *const sv, MAGIC **const mgp, const U8 *const start,
		    const U8 *const send, STRLEN uoffset,
		    STRLEN uoffset0, STRLEN boffset0)
{
    STRLEN boffset = 0; /* Actually always set, but let's keep gcc happy.  */
    bool found = FALSE;
    bool at_end = FALSE;

    PERL_ARGS_ASSERT_SV_POS_U2B_CACHED;

    assert (uoffset >= uoffset0);

    if (!uoffset)
	return 0;

    if (!SvREADONLY(sv)
	&& PL_utf8cache
	&& (*mgp || (SvTYPE(sv) >= SVt_PVMG &&
		     (*mgp = mg_find(sv, PERL_MAGIC_utf8))))) {
	if ((*mgp)->mg_ptr) {
	    STRLEN *cache = (STRLEN *) (*mgp)->mg_ptr;
	    if (cache[0] == uoffset) {
		/* An exact match. */
		return cache[1];
	    }
	    if (cache[2] == uoffset) {
		/* An exact match. */
		return cache[3];
	    }

	    if (cache[0] < uoffset) {
		/* The cache already knows part of the way.   */
		if (cache[0] > uoffset0) {
		    /* The cache knows more than the passed in pair  */
		    uoffset0 = cache[0];
		    boffset0 = cache[1];
		}
		if ((*mgp)->mg_len != -1) {
		    /* And we know the end too.  */
		    boffset = boffset0
			+ sv_pos_u2b_midway(start + boffset0, send,
					      uoffset - uoffset0,
					      (*mgp)->mg_len - uoffset0);
		} else {
		    uoffset -= uoffset0;
		    boffset = boffset0
			+ sv_pos_u2b_forwards(start + boffset0,
					      send, &uoffset, &at_end);
		    uoffset += uoffset0;
		}
	    }
	    else if (cache[2] < uoffset) {
		/* We're between the two cache entries.  */
		if (cache[2] > uoffset0) {
		    /* and the cache knows more than the passed in pair  */
		    uoffset0 = cache[2];
		    boffset0 = cache[3];
		}

		boffset = boffset0
		    + sv_pos_u2b_midway(start + boffset0,
					  start + cache[1],
					  uoffset - uoffset0,
					  cache[0] - uoffset0);
	    } else {
		boffset = boffset0
		    + sv_pos_u2b_midway(start + boffset0,
					  start + cache[3],
					  uoffset - uoffset0,
					  cache[2] - uoffset0);
	    }
	    found = TRUE;
	}
	else if ((*mgp)->mg_len != -1) {
	    /* If we can take advantage of a passed in offset, do so.  */
	    /* In fact, offset0 is either 0, or less than offset, so don't
	       need to worry about the other possibility.  */
	    boffset = boffset0
		+ sv_pos_u2b_midway(start + boffset0, send,
				      uoffset - uoffset0,
				      (*mgp)->mg_len - uoffset0);
	    found = TRUE;
	}
    }

    if (!found || PL_utf8cache < 0) {
	STRLEN real_boffset;
	uoffset -= uoffset0;
	real_boffset = boffset0 + sv_pos_u2b_forwards(start + boffset0,
						      send, &uoffset, &at_end);
	uoffset += uoffset0;

	if (found && PL_utf8cache < 0)
	    assert_uft8_cache_coherent("sv_pos_u2b_cache", boffset,
				       real_boffset, sv);
	boffset = real_boffset;
    }

    if (PL_utf8cache) {
	if (at_end)
	    utf8_mg_len_cache_update(sv, mgp, uoffset);
	else
	    utf8_mg_pos_cache_update(sv, mgp, boffset, uoffset, send - start);
    }
    return boffset;
}


/*
=for apidoc sv_pos_u2b_flags

Converts the value pointed to by offsetp from a count of UTF-8 chars from
the start of the string, to a count of the equivalent number of bytes; if
lenp is non-zero, it does the same to lenp, but this time starting from
the offset, rather than from the start
of the string.  Handles type coercion.
I<flags> is passed to C<SvPV_flags>, and usually should be
C<SV_GMAGIC|SV_CONST_RETURN> to handle magic.

=cut
*/

/*
 * sv_pos_u2b_flags() uses, like sv_pos_b2u(), the mg_ptr of the potential
 * PERL_MAGIC_utf8 of the sv to store the mapping between UTF-8 and
 * byte offsets.  See also the comments of S_utf8_mg_pos_cache_update().
 *
 */

STRLEN
Perl_sv_pos_u2b_flags(pTHX_ SV *const sv, STRLEN uoffset, STRLEN *const lenp,
		      U32 flags)
{
    const U8 *start;
    STRLEN len;
    STRLEN boffset;

    PERL_ARGS_ASSERT_SV_POS_U2B_FLAGS;

    start = (U8*)SvPV_flags(sv, len, flags);
    if (len) {
	const U8 * const send = start + len;
	MAGIC *mg = NULL;
	boffset = sv_pos_u2b_cached(sv, &mg, start, send, uoffset, 0, 0);

	if (lenp
	    && *lenp /* don't bother doing work for 0, as its bytes equivalent
			is 0, and *lenp is already set to that.  */) {
	    /* Convert the relative offset to absolute.  */
	    const STRLEN uoffset2 = uoffset + *lenp;
	    const STRLEN boffset2
		= sv_pos_u2b_cached(sv, &mg, start, send, uoffset2,
				      uoffset, boffset) - boffset;

	    *lenp = boffset2;
	}
    } else {
	if (lenp)
	    *lenp = 0;
	boffset = 0;
    }

    return boffset;
}

/*
=for apidoc sv_pos_u2b

Converts the value pointed to by offsetp from a count of UTF-8 chars from
the start of the string, to a count of the equivalent number of bytes; if
lenp is non-zero, it does the same to lenp, but this time starting from
the offset, rather than from the start of the string.  Handles magic and
type coercion.

Use C<sv_pos_u2b_flags> in preference, which correctly handles strings longer
than 2Gb.

=cut
*/

/*
 * sv_pos_u2b() uses, like sv_pos_b2u(), the mg_ptr of the potential
 * PERL_MAGIC_utf8 of the sv to store the mapping between UTF-8 and
 * byte offsets.  See also the comments of S_utf8_mg_pos_cache_update().
 *
 */

/* This function is subject to size and sign problems */

void
Perl_sv_pos_u2b(pTHX_ register SV *const sv, I32 *const offsetp, I32 *const lenp)
{
    PERL_ARGS_ASSERT_SV_POS_U2B;

    if (lenp) {
	STRLEN ulen = (STRLEN)*lenp;
	*offsetp = (I32)sv_pos_u2b_flags(sv, (STRLEN)*offsetp, &ulen,
					 SV_GMAGIC|SV_CONST_RETURN);
	*lenp = (I32)ulen;
    } else {
	*offsetp = (I32)sv_pos_u2b_flags(sv, (STRLEN)*offsetp, NULL,
					 SV_GMAGIC|SV_CONST_RETURN);
    }
}

static void
S_utf8_mg_len_cache_update(pTHX_ SV *const sv, MAGIC **const mgp,
			   const STRLEN ulen)
{
    PERL_ARGS_ASSERT_UTF8_MG_LEN_CACHE_UPDATE;
    if (SvREADONLY(sv))
	return;

    if (!*mgp && (SvTYPE(sv) < SVt_PVMG ||
		  !(*mgp = mg_find(sv, PERL_MAGIC_utf8)))) {
	*mgp = sv_magicext(sv, 0, PERL_MAGIC_utf8, &PL_vtbl_utf8, 0, 0);
    }
    assert(*mgp);

    (*mgp)->mg_len = ulen;
    /* For now, treat "overflowed" as "still unknown". See RT #72924.  */
    if (ulen != (STRLEN) (*mgp)->mg_len)
	(*mgp)->mg_len = -1;
}

/* Create and update the UTF8 magic offset cache, with the proffered utf8/
   byte length pairing. The (byte) length of the total SV is passed in too,
   as blen, because for some (more esoteric) SVs, the call to SvPV_const()
   may not have updated SvCUR, so we can't rely on reading it directly.

   The proffered utf8/byte length pairing isn't used if the cache already has
   two pairs, and swapping either for the proffered pair would increase the
   RMS of the intervals between known byte offsets.

   The cache itself consists of 4 STRLEN values
   0: larger UTF-8 offset
   1: corresponding byte offset
   2: smaller UTF-8 offset
   3: corresponding byte offset

   Unused cache pairs have the value 0, 0.
   Keeping the cache "backwards" means that the invariant of
   cache[0] >= cache[2] is maintained even with empty slots, which means that
   the code that uses it doesn't need to worry if only 1 entry has actually
   been set to non-zero.  It also makes the "position beyond the end of the
   cache" logic much simpler, as the first slot is always the one to start
   from.   
*/
static void
S_utf8_mg_pos_cache_update(pTHX_ SV *const sv, MAGIC **const mgp, const STRLEN byte,
                           const STRLEN utf8, const STRLEN blen)
{
    STRLEN *cache;

    PERL_ARGS_ASSERT_UTF8_MG_POS_CACHE_UPDATE;

    if (SvREADONLY(sv))
	return;

    if (!*mgp && (SvTYPE(sv) < SVt_PVMG ||
		  !(*mgp = mg_find(sv, PERL_MAGIC_utf8)))) {
	*mgp = sv_magicext(sv, 0, PERL_MAGIC_utf8, (MGVTBL*)&PL_vtbl_utf8, 0,
			   0);
	(*mgp)->mg_len = -1;
    }
    assert(*mgp);

    if (!(cache = (STRLEN *)(*mgp)->mg_ptr)) {
	Newxz(cache, PERL_MAGIC_UTF8_CACHESIZE * 2, STRLEN);
	(*mgp)->mg_ptr = (char *) cache;
    }
    assert(cache);

    if (PL_utf8cache < 0 && SvPOKp(sv)) {
	/* SvPOKp() because it's possible that sv has string overloading, and
	   therefore is a reference, hence SvPVX() is actually a pointer.
	   This cures the (very real) symptoms of RT 69422, but I'm not actually
	   sure whether we should even be caching the results of UTF-8
	   operations on overloading, given that nothing stops overloading
	   returning a different value every time it's called.  */
	const U8 *start = (const U8 *) SvPVX_const(sv);
	const STRLEN realutf8 = utf8_length(start, start + byte);

	assert_uft8_cache_coherent("utf8_mg_pos_cache_update", utf8, realutf8,
				   sv);
    }

    /* Cache is held with the later position first, to simplify the code
       that deals with unbounded ends.  */
       
    ASSERT_UTF8_CACHE(cache);
    if (cache[1] == 0) {
	/* Cache is totally empty  */
	cache[0] = utf8;
	cache[1] = byte;
    } else if (cache[3] == 0) {
	if (byte > cache[1]) {
	    /* New one is larger, so goes first.  */
	    cache[2] = cache[0];
	    cache[3] = cache[1];
	    cache[0] = utf8;
	    cache[1] = byte;
	} else {
	    cache[2] = utf8;
	    cache[3] = byte;
	}
    } else {
#define THREEWAY_SQUARE(a,b,c,d) \
	    ((float)((d) - (c))) * ((float)((d) - (c))) \
	    + ((float)((c) - (b))) * ((float)((c) - (b))) \
	       + ((float)((b) - (a))) * ((float)((b) - (a)))

	/* Cache has 2 slots in use, and we know three potential pairs.
	   Keep the two that give the lowest RMS distance. Do the
	   calculation in bytes simply because we always know the byte
	   length.  squareroot has the same ordering as the positive value,
	   so don't bother with the actual square root.  */
	const float existing = THREEWAY_SQUARE(0, cache[3], cache[1], blen);
	if (byte > cache[1]) {
	    /* New position is after the existing pair of pairs.  */
	    const float keep_earlier
		= THREEWAY_SQUARE(0, cache[3], byte, blen);
	    const float keep_later
		= THREEWAY_SQUARE(0, cache[1], byte, blen);

	    if (keep_later < keep_earlier) {
		if (keep_later < existing) {
		    cache[2] = cache[0];
		    cache[3] = cache[1];
		    cache[0] = utf8;
		    cache[1] = byte;
		}
	    }
	    else {
		if (keep_earlier < existing) {
		    cache[0] = utf8;
		    cache[1] = byte;
		}
	    }
	}
	else if (byte > cache[3]) {
	    /* New position is between the existing pair of pairs.  */
	    const float keep_earlier
		= THREEWAY_SQUARE(0, cache[3], byte, blen);
	    const float keep_later
		= THREEWAY_SQUARE(0, byte, cache[1], blen);

	    if (keep_later < keep_earlier) {
		if (keep_later < existing) {
		    cache[2] = utf8;
		    cache[3] = byte;
		}
	    }
	    else {
		if (keep_earlier < existing) {
		    cache[0] = utf8;
		    cache[1] = byte;
		}
	    }
	}
	else {
 	    /* New position is before the existing pair of pairs.  */
	    const float keep_earlier
		= THREEWAY_SQUARE(0, byte, cache[3], blen);
	    const float keep_later
		= THREEWAY_SQUARE(0, byte, cache[1], blen);

	    if (keep_later < keep_earlier) {
		if (keep_later < existing) {
		    cache[2] = utf8;
		    cache[3] = byte;
		}
	    }
	    else {
		if (keep_earlier < existing) {
		    cache[0] = cache[2];
		    cache[1] = cache[3];
		    cache[2] = utf8;
		    cache[3] = byte;
		}
	    }
	}
    }
    ASSERT_UTF8_CACHE(cache);
}

/* We already know all of the way, now we may be able to walk back.  The same
   assumption is made as in S_sv_pos_u2b_midway(), namely that walking
   backward is half the speed of walking forward. */
static STRLEN
S_sv_pos_b2u_midway(pTHX_ const U8 *const s, const U8 *const target,
                    const U8 *end, STRLEN endu)
{
    const STRLEN forw = target - s;
    STRLEN backw = end - target;

    PERL_ARGS_ASSERT_SV_POS_B2U_MIDWAY;

    if (forw < 2 * backw) {
	return utf8_length(s, target);
    }

    while (end > target) {
	end--;
	while (UTF8_IS_CONTINUATION(*end)) {
	    end--;
	}
	endu--;
    }
    return endu;
}

/*
=for apidoc sv_pos_b2u

Converts the value pointed to by offsetp from a count of bytes from the
start of the string, to a count of the equivalent number of UTF-8 chars.
Handles magic and type coercion.

=cut
*/

/*
 * sv_pos_b2u() uses, like sv_pos_u2b(), the mg_ptr of the potential
 * PERL_MAGIC_utf8 of the sv to store the mapping between UTF-8 and
 * byte offsets.
 *
 */
void
Perl_sv_pos_b2u(pTHX_ register SV *const sv, I32 *const offsetp)
{
    const U8* s;
    const STRLEN byte = *offsetp;
    STRLEN len = 0; /* Actually always set, but let's keep gcc happy.  */
    STRLEN blen;
    MAGIC* mg = NULL;
    const U8* send;
    bool found = FALSE;

    PERL_ARGS_ASSERT_SV_POS_B2U;

    if (!sv)
	return;

    s = (const U8*)SvPV_const(sv, blen);

    if (blen < byte)
	Perl_croak(aTHX_ "panic: sv_pos_b2u: bad byte offset, blen=%"UVuf
		   ", byte=%"UVuf, (UV)blen, (UV)byte);

    send = s + byte;

    if (!SvREADONLY(sv)
	&& PL_utf8cache
	&& SvTYPE(sv) >= SVt_PVMG
	&& (mg = mg_find(sv, PERL_MAGIC_utf8)))
    {
	if (mg->mg_ptr) {
	    STRLEN * const cache = (STRLEN *) mg->mg_ptr;
	    if (cache[1] == byte) {
		/* An exact match. */
		*offsetp = cache[0];
		return;
	    }
	    if (cache[3] == byte) {
		/* An exact match. */
		*offsetp = cache[2];
		return;
	    }

	    if (cache[1] < byte) {
		/* We already know part of the way. */
		if (mg->mg_len != -1) {
		    /* Actually, we know the end too.  */
		    len = cache[0]
			+ S_sv_pos_b2u_midway(aTHX_ s + cache[1], send,
					      s + blen, mg->mg_len - cache[0]);
		} else {
		    len = cache[0] + utf8_length(s + cache[1], send);
		}
	    }
	    else if (cache[3] < byte) {
		/* We're between the two cached pairs, so we do the calculation
		   offset by the byte/utf-8 positions for the earlier pair,
		   then add the utf-8 characters from the string start to
		   there.  */
		len = S_sv_pos_b2u_midway(aTHX_ s + cache[3], send,
					  s + cache[1], cache[0] - cache[2])
		    + cache[2];

	    }
	    else { /* cache[3] > byte */
		len = S_sv_pos_b2u_midway(aTHX_ s, send, s + cache[3],
					  cache[2]);

	    }
	    ASSERT_UTF8_CACHE(cache);
	    found = TRUE;
	} else if (mg->mg_len != -1) {
	    len = S_sv_pos_b2u_midway(aTHX_ s, send, s + blen, mg->mg_len);
	    found = TRUE;
	}
    }
    if (!found || PL_utf8cache < 0) {
	const STRLEN real_len = utf8_length(s, send);

	if (found && PL_utf8cache < 0)
	    assert_uft8_cache_coherent("sv_pos_b2u", len, real_len, sv);
	len = real_len;
    }
    *offsetp = len;

    if (PL_utf8cache) {
	if (blen == byte)
	    utf8_mg_len_cache_update(sv, &mg, len);
	else
	    utf8_mg_pos_cache_update(sv, &mg, byte, len, blen);
    }
}

static void
S_assert_uft8_cache_coherent(pTHX_ const char *const func, STRLEN from_cache,
			     STRLEN real, SV *const sv)
{
    PERL_ARGS_ASSERT_ASSERT_UFT8_CACHE_COHERENT;

    /* As this is debugging only code, save space by keeping this test here,
       rather than inlining it in all the callers.  */
    if (from_cache == real)
	return;

    /* Need to turn the assertions off otherwise we may recurse infinitely
       while printing error messages.  */
    SAVEI8(PL_utf8cache);
    PL_utf8cache = 0;
    Perl_croak(aTHX_ "panic: %s cache %"UVuf" real %"UVuf" for %"SVf,
	       func, (UV) from_cache, (UV) real, SVfARG(sv));
}

/*
=for apidoc sv_eq

Returns a boolean indicating whether the strings in the two SVs are
identical.  Is UTF-8 and 'use bytes' aware, handles get magic, and will
coerce its args to strings if necessary.

=for apidoc sv_eq_flags

Returns a boolean indicating whether the strings in the two SVs are
identical.  Is UTF-8 and 'use bytes' aware and coerces its args to strings
if necessary.  If the flags include SV_GMAGIC, it handles get-magic, too.

=cut
*/

I32
Perl_sv_eq_flags(pTHX_ register SV *sv1, register SV *sv2, const U32 flags)
{
    dVAR;
    const char *pv1;
    STRLEN cur1;
    const char *pv2;
    STRLEN cur2;
    I32  eq     = 0;
    SV* svrecode = NULL;

    if (!sv1) {
	pv1 = "";
	cur1 = 0;
    }
    else {
	/* if pv1 and pv2 are the same, second SvPV_const call may
	 * invalidate pv1 (if we are handling magic), so we may need to
	 * make a copy */
	if (sv1 == sv2 && flags & SV_GMAGIC
	 && (SvTHINKFIRST(sv1) || SvGMAGICAL(sv1))) {
	    pv1 = SvPV_const(sv1, cur1);
	    sv1 = newSVpvn_flags(pv1, cur1, SVs_TEMP | SvUTF8(sv2));
	}
	pv1 = SvPV_flags_const(sv1, cur1, flags);
    }

    if (!sv2){
	pv2 = "";
	cur2 = 0;
    }
    else
	pv2 = SvPV_flags_const(sv2, cur2, flags);

    if (cur1 && cur2 && SvUTF8(sv1) != SvUTF8(sv2) && !IN_BYTES) {
        /* Differing utf8ness.
	 * Do not UTF8size the comparands as a side-effect. */
	 if (PL_encoding) {
	      if (SvUTF8(sv1)) {
		   svrecode = newSVpvn(pv2, cur2);
		   sv_recode_to_utf8(svrecode, PL_encoding);
		   pv2 = SvPV_const(svrecode, cur2);
	      }
	      else {
		   svrecode = newSVpvn(pv1, cur1);
		   sv_recode_to_utf8(svrecode, PL_encoding);
		   pv1 = SvPV_const(svrecode, cur1);
	      }
	      /* Now both are in UTF-8. */
	      if (cur1 != cur2) {
		   SvREFCNT_dec(svrecode);
		   return FALSE;
	      }
	 }
	 else {
	      if (SvUTF8(sv1)) {
		  /* sv1 is the UTF-8 one  */
		  return bytes_cmp_utf8((const U8*)pv2, cur2,
					(const U8*)pv1, cur1) == 0;
	      }
	      else {
		  /* sv2 is the UTF-8 one  */
		  return bytes_cmp_utf8((const U8*)pv1, cur1,
					(const U8*)pv2, cur2) == 0;
	      }
	 }
    }

    if (cur1 == cur2)
	eq = (pv1 == pv2) || memEQ(pv1, pv2, cur1);
	
    SvREFCNT_dec(svrecode);

    return eq;
}

/*
=for apidoc sv_cmp

Compares the strings in two SVs.  Returns -1, 0, or 1 indicating whether the
string in C<sv1> is less than, equal to, or greater than the string in
C<sv2>.  Is UTF-8 and 'use bytes' aware, handles get magic, and will
coerce its args to strings if necessary.  See also C<sv_cmp_locale>.

=for apidoc sv_cmp_flags

Compares the strings in two SVs.  Returns -1, 0, or 1 indicating whether the
string in C<sv1> is less than, equal to, or greater than the string in
C<sv2>.  Is UTF-8 and 'use bytes' aware and will coerce its args to strings
if necessary.  If the flags include SV_GMAGIC, it handles get magic.  See
also C<sv_cmp_locale_flags>.

=cut
*/

I32
Perl_sv_cmp(pTHX_ register SV *const sv1, register SV *const sv2)
{
    return sv_cmp_flags(sv1, sv2, SV_GMAGIC);
}

I32
Perl_sv_cmp_flags(pTHX_ register SV *const sv1, register SV *const sv2,
		  const U32 flags)
{
    dVAR;
    STRLEN cur1, cur2;
    const char *pv1, *pv2;
    char *tpv = NULL;
    I32  cmp;
    SV *svrecode = NULL;

    if (!sv1) {
	pv1 = "";
	cur1 = 0;
    }
    else
	pv1 = SvPV_flags_const(sv1, cur1, flags);

    if (!sv2) {
	pv2 = "";
	cur2 = 0;
    }
    else
	pv2 = SvPV_flags_const(sv2, cur2, flags);

    if (cur1 && cur2 && SvUTF8(sv1) != SvUTF8(sv2) && !IN_BYTES) {
        /* Differing utf8ness.
	 * Do not UTF8size the comparands as a side-effect. */
	if (SvUTF8(sv1)) {
	    if (PL_encoding) {
		 svrecode = newSVpvn(pv2, cur2);
		 sv_recode_to_utf8(svrecode, PL_encoding);
		 pv2 = SvPV_const(svrecode, cur2);
	    }
	    else {
		const int retval = -bytes_cmp_utf8((const U8*)pv2, cur2,
						   (const U8*)pv1, cur1);
		return retval ? retval < 0 ? -1 : +1 : 0;
	    }
	}
	else {
	    if (PL_encoding) {
		 svrecode = newSVpvn(pv1, cur1);
		 sv_recode_to_utf8(svrecode, PL_encoding);
		 pv1 = SvPV_const(svrecode, cur1);
	    }
	    else {
		const int retval = bytes_cmp_utf8((const U8*)pv1, cur1,
						  (const U8*)pv2, cur2);
		return retval ? retval < 0 ? -1 : +1 : 0;
	    }
	}
    }

    if (!cur1) {
	cmp = cur2 ? -1 : 0;
    } else if (!cur2) {
	cmp = 1;
    } else {
        const I32 retval = memcmp((const void*)pv1, (const void*)pv2, cur1 < cur2 ? cur1 : cur2);

	if (retval) {
	    cmp = retval < 0 ? -1 : 1;
	} else if (cur1 == cur2) {
	    cmp = 0;
        } else {
	    cmp = cur1 < cur2 ? -1 : 1;
	}
    }

    SvREFCNT_dec(svrecode);
    if (tpv)
	Safefree(tpv);

    return cmp;
}

/*
=for apidoc sv_cmp_locale

Compares the strings in two SVs in a locale-aware manner.  Is UTF-8 and
'use bytes' aware, handles get magic, and will coerce its args to strings
if necessary.  See also C<sv_cmp>.

=for apidoc sv_cmp_locale_flags

Compares the strings in two SVs in a locale-aware manner.  Is UTF-8 and
'use bytes' aware and will coerce its args to strings if necessary.  If the
flags contain SV_GMAGIC, it handles get magic.  See also C<sv_cmp_flags>.

=cut
*/

I32
Perl_sv_cmp_locale(pTHX_ register SV *const sv1, register SV *const sv2)
{
    return sv_cmp_locale_flags(sv1, sv2, SV_GMAGIC);
}

I32
Perl_sv_cmp_locale_flags(pTHX_ register SV *const sv1, register SV *const sv2,
			 const U32 flags)
{
    dVAR;
#ifdef USE_LOCALE_COLLATE

    char *pv1, *pv2;
    STRLEN len1, len2;
    I32 retval;

    if (PL_collation_standard)
	goto raw_compare;

    len1 = 0;
    pv1 = sv1 ? sv_collxfrm_flags(sv1, &len1, flags) : (char *) NULL;
    len2 = 0;
    pv2 = sv2 ? sv_collxfrm_flags(sv2, &len2, flags) : (char *) NULL;

    if (!pv1 || !len1) {
	if (pv2 && len2)
	    return -1;
	else
	    goto raw_compare;
    }
    else {
	if (!pv2 || !len2)
	    return 1;
    }

    retval = memcmp((void*)pv1, (void*)pv2, len1 < len2 ? len1 : len2);

    if (retval)
	return retval < 0 ? -1 : 1;

    /*
     * When the result of collation is equality, that doesn't mean
     * that there are no differences -- some locales exclude some
     * characters from consideration.  So to avoid false equalities,
     * we use the raw string as a tiebreaker.
     */

  raw_compare:
    /*FALLTHROUGH*/

#endif /* USE_LOCALE_COLLATE */

    return sv_cmp(sv1, sv2);
}


#ifdef USE_LOCALE_COLLATE

/*
=for apidoc sv_collxfrm

This calls C<sv_collxfrm_flags> with the SV_GMAGIC flag.  See
C<sv_collxfrm_flags>.

=for apidoc sv_collxfrm_flags

Add Collate Transform magic to an SV if it doesn't already have it.  If the
flags contain SV_GMAGIC, it handles get-magic.

Any scalar variable may carry PERL_MAGIC_collxfrm magic that contains the
scalar data of the variable, but transformed to such a format that a normal
memory comparison can be used to compare the data according to the locale
settings.

=cut
*/

char *
Perl_sv_collxfrm_flags(pTHX_ SV *const sv, STRLEN *const nxp, const I32 flags)
{
    dVAR;
    MAGIC *mg;

    PERL_ARGS_ASSERT_SV_COLLXFRM_FLAGS;

    mg = SvMAGICAL(sv) ? mg_find(sv, PERL_MAGIC_collxfrm) : (MAGIC *) NULL;
    if (!mg || !mg->mg_ptr || *(U32*)mg->mg_ptr != PL_collation_ix) {
	const char *s;
	char *xf;
	STRLEN len, xlen;

	if (mg)
	    Safefree(mg->mg_ptr);
	s = SvPV_flags_const(sv, len, flags);
	if ((xf = mem_collxfrm(s, len, &xlen))) {
	    if (! mg) {
#ifdef PERL_OLD_COPY_ON_WRITE
		if (SvIsCOW(sv))
		    sv_force_normal_flags(sv, 0);
#endif
		mg = sv_magicext(sv, 0, PERL_MAGIC_collxfrm, &PL_vtbl_collxfrm,
				 0, 0);
		assert(mg);
	    }
	    mg->mg_ptr = xf;
	    mg->mg_len = xlen;
	}
	else {
	    if (mg) {
		mg->mg_ptr = NULL;
		mg->mg_len = -1;
	    }
	}
    }
    if (mg && mg->mg_ptr) {
	*nxp = mg->mg_len;
	return mg->mg_ptr + sizeof(PL_collation_ix);
    }
    else {
	*nxp = 0;
	return NULL;
    }
}

#endif /* USE_LOCALE_COLLATE */

static char *
S_sv_gets_append_to_utf8(pTHX_ SV *const sv, PerlIO *const fp, I32 append)
{
    SV * const tsv = newSV(0);
    ENTER;
    SAVEFREESV(tsv);
    sv_gets(tsv, fp, 0);
    sv_utf8_upgrade_nomg(tsv);
    SvCUR_set(sv,append);
    sv_catsv(sv,tsv);
    LEAVE;
    return (SvCUR(sv) - append) ? SvPVX(sv) : NULL;
}

static char *
S_sv_gets_read_record(pTHX_ SV *const sv, PerlIO *const fp, I32 append)
{
    I32 bytesread;
    const U32 recsize = SvUV(SvRV(PL_rs)); /* RsRECORD() guarantees > 0. */
      /* Grab the size of the record we're getting */
    char *const buffer = SvGROW(sv, (STRLEN)(recsize + append + 1)) + append;
#ifdef VMS
    int fd;
#endif

    /* Go yank in */
#ifdef VMS
    /* VMS wants read instead of fread, because fread doesn't respect */
    /* RMS record boundaries. This is not necessarily a good thing to be */
    /* doing, but we've got no other real choice - except avoid stdio
       as implementation - perhaps write a :vms layer ?
    */
    fd = PerlIO_fileno(fp);
    if (fd != -1) {
	bytesread = PerlLIO_read(fd, buffer, recsize);
    }
    else /* in-memory file from PerlIO::Scalar */
#endif
    {
	bytesread = PerlIO_read(fp, buffer, recsize);
    }

    if (bytesread < 0)
	bytesread = 0;
    SvCUR_set(sv, bytesread + append);
    buffer[bytesread] = '\0';
    return (SvCUR(sv) - append) ? SvPVX(sv) : NULL;
}

/*
=for apidoc sv_gets

Get a line from the filehandle and store it into the SV, optionally
appending to the currently-stored string.

=cut
*/

char *
Perl_sv_gets(pTHX_ register SV *const sv, register PerlIO *const fp, I32 append)
{
    dVAR;
    const char *rsptr;
    STRLEN rslen;
    register STDCHAR rslast;
    register STDCHAR *bp;
    register I32 cnt;
    I32 i = 0;
    I32 rspara = 0;

    PERL_ARGS_ASSERT_SV_GETS;

    if (SvTHINKFIRST(sv))
	sv_force_normal_flags(sv, append ? 0 : SV_COW_DROP_PV);
    /* XXX. If you make this PVIV, then copy on write can copy scalars read
       from <>.
       However, perlbench says it's slower, because the existing swipe code
       is faster than copy on write.
       Swings and roundabouts.  */
    SvUPGRADE(sv, SVt_PV);

    SvSCREAM_off(sv);

    if (append) {
	if (PerlIO_isutf8(fp)) {
	    if (!SvUTF8(sv)) {
		sv_utf8_upgrade_nomg(sv);
		sv_pos_u2b(sv,&append,0);
	    }
	} else if (SvUTF8(sv)) {
	    return S_sv_gets_append_to_utf8(aTHX_ sv, fp, append);
	}
    }

    SvPOK_only(sv);
    if (!append) {
        SvCUR_set(sv,0);
    }
    if (PerlIO_isutf8(fp))
	SvUTF8_on(sv);

    if (IN_PERL_COMPILETIME) {
	/* we always read code in line mode */
	rsptr = "\n";
	rslen = 1;
    }
    else if (RsSNARF(PL_rs)) {
    	/* If it is a regular disk file use size from stat() as estimate
	   of amount we are going to read -- may result in mallocing
	   more memory than we really need if the layers below reduce
	   the size we read (e.g. CRLF or a gzip layer).
	 */
	Stat_t st;
	if (!PerlLIO_fstat(PerlIO_fileno(fp), &st) && S_ISREG(st.st_mode))  {
	    const Off_t offset = PerlIO_tell(fp);
	    if (offset != (Off_t) -1 && st.st_size + append > offset) {
	     	(void) SvGROW(sv, (STRLEN)((st.st_size - offset) + append + 1));
	    }
	}
	rsptr = NULL;
	rslen = 0;
    }
    else if (RsRECORD(PL_rs)) {
	return S_sv_gets_read_record(aTHX_ sv, fp, append);
    }
    else if (RsPARA(PL_rs)) {
	rsptr = "\n\n";
	rslen = 2;
	rspara = 1;
    }
    else {
	/* Get $/ i.e. PL_rs into same encoding as stream wants */
	if (PerlIO_isutf8(fp)) {
	    rsptr = SvPVutf8(PL_rs, rslen);
	}
	else {
	    if (SvUTF8(PL_rs)) {
		if (!sv_utf8_downgrade(PL_rs, TRUE)) {
		    Perl_croak(aTHX_ "Wide character in $/");
		}
	    }
	    rsptr = SvPV_const(PL_rs, rslen);
	}
    }

    rslast = rslen ? rsptr[rslen - 1] : '\0';

    if (rspara) {		/* have to do this both before and after */
	do {			/* to make sure file boundaries work right */
	    if (PerlIO_eof(fp))
		return 0;
	    i = PerlIO_getc(fp);
	    if (i != '\n') {
		if (i == -1)
		    return 0;
		PerlIO_ungetc(fp,i);
		break;
	    }
	} while (i != EOF);
    }

    /* See if we know enough about I/O mechanism to cheat it ! */

    /* This used to be #ifdef test - it is made run-time test for ease
       of abstracting out stdio interface. One call should be cheap
       enough here - and may even be a macro allowing compile
       time optimization.
     */

    if (PerlIO_fast_gets(fp)) {

    /*
     * We're going to steal some values from the stdio struct
     * and put EVERYTHING in the innermost loop into registers.
     */
    register STDCHAR *ptr;
    STRLEN bpx;
    I32 shortbuffered;

#if defined(VMS) && defined(PERLIO_IS_STDIO)
    /* An ungetc()d char is handled separately from the regular
     * buffer, so we getc() it back out and stuff it in the buffer.
     */
    i = PerlIO_getc(fp);
    if (i == EOF) return 0;
    *(--((*fp)->_ptr)) = (unsigned char) i;
    (*fp)->_cnt++;
#endif

    /* Here is some breathtakingly efficient cheating */

    cnt = PerlIO_get_cnt(fp);			/* get count into register */
    /* make sure we have the room */
    if ((I32)(SvLEN(sv) - append) <= cnt + 1) {
    	/* Not room for all of it
	   if we are looking for a separator and room for some
	 */
	if (rslen && cnt > 80 && (I32)SvLEN(sv) > append) {
	    /* just process what we have room for */
	    shortbuffered = cnt - SvLEN(sv) + append + 1;
	    cnt -= shortbuffered;
	}
	else {
	    shortbuffered = 0;
	    /* remember that cnt can be negative */
	    SvGROW(sv, (STRLEN)(append + (cnt <= 0 ? 2 : (cnt + 1))));
	}
    }
    else
	shortbuffered = 0;
    bp = (STDCHAR*)SvPVX_const(sv) + append;  /* move these two too to registers */
    ptr = (STDCHAR*)PerlIO_get_ptr(fp);
    DEBUG_P(PerlIO_printf(Perl_debug_log,
	"Screamer: entering, ptr=%"UVuf", cnt=%ld\n",PTR2UV(ptr),(long)cnt));
    DEBUG_P(PerlIO_printf(Perl_debug_log,
	"Screamer: entering: PerlIO * thinks ptr=%"UVuf", cnt=%ld, base=%"UVuf"\n",
	       PTR2UV(PerlIO_get_ptr(fp)), (long)PerlIO_get_cnt(fp),
	       PTR2UV(PerlIO_has_base(fp) ? PerlIO_get_base(fp) : 0)));
    for (;;) {
      screamer:
	if (cnt > 0) {
	    if (rslen) {
		while (cnt > 0) {		     /* this     |  eat */
		    cnt--;
		    if ((*bp++ = *ptr++) == rslast)  /* really   |  dust */
			goto thats_all_folks;	     /* screams  |  sed :-) */
		}
	    }
	    else {
	        Copy(ptr, bp, cnt, char);	     /* this     |  eat */
		bp += cnt;			     /* screams  |  dust */
		ptr += cnt;			     /* louder   |  sed :-) */
		cnt = 0;
		assert (!shortbuffered);
		goto cannot_be_shortbuffered;
	    }
	}
	
	if (shortbuffered) {		/* oh well, must extend */
	    cnt = shortbuffered;
	    shortbuffered = 0;
	    bpx = bp - (STDCHAR*)SvPVX_const(sv); /* box up before relocation */
	    SvCUR_set(sv, bpx);
	    SvGROW(sv, SvLEN(sv) + append + cnt + 2);
	    bp = (STDCHAR*)SvPVX_const(sv) + bpx; /* unbox after relocation */
	    continue;
	}

    cannot_be_shortbuffered:
	DEBUG_P(PerlIO_printf(Perl_debug_log,
			      "Screamer: going to getc, ptr=%"UVuf", cnt=%ld\n",
			      PTR2UV(ptr),(long)cnt));
	PerlIO_set_ptrcnt(fp, (STDCHAR*)ptr, cnt); /* deregisterize cnt and ptr */

	DEBUG_Pv(PerlIO_printf(Perl_debug_log,
	    "Screamer: pre: FILE * thinks ptr=%"UVuf", cnt=%ld, base=%"UVuf"\n",
	    PTR2UV(PerlIO_get_ptr(fp)), (long)PerlIO_get_cnt(fp),
	    PTR2UV(PerlIO_has_base (fp) ? PerlIO_get_base(fp) : 0)));

	/* This used to call 'filbuf' in stdio form, but as that behaves like
	   getc when cnt <= 0 we use PerlIO_getc here to avoid introducing
	   another abstraction.  */
	i   = PerlIO_getc(fp);		/* get more characters */

	DEBUG_Pv(PerlIO_printf(Perl_debug_log,
	    "Screamer: post: FILE * thinks ptr=%"UVuf", cnt=%ld, base=%"UVuf"\n",
	    PTR2UV(PerlIO_get_ptr(fp)), (long)PerlIO_get_cnt(fp),
	    PTR2UV(PerlIO_has_base (fp) ? PerlIO_get_base(fp) : 0)));

	cnt = PerlIO_get_cnt(fp);
	ptr = (STDCHAR*)PerlIO_get_ptr(fp);	/* reregisterize cnt and ptr */
	DEBUG_P(PerlIO_printf(Perl_debug_log,
	    "Screamer: after getc, ptr=%"UVuf", cnt=%ld\n",PTR2UV(ptr),(long)cnt));

	if (i == EOF)			/* all done for ever? */
	    goto thats_really_all_folks;

	bpx = bp - (STDCHAR*)SvPVX_const(sv);	/* box up before relocation */
	SvCUR_set(sv, bpx);
	SvGROW(sv, bpx + cnt + 2);
	bp = (STDCHAR*)SvPVX_const(sv) + bpx;	/* unbox after relocation */

	*bp++ = (STDCHAR)i;		/* store character from PerlIO_getc */

	if (rslen && (STDCHAR)i == rslast)  /* all done for now? */
	    goto thats_all_folks;
    }

thats_all_folks:
    if ((rslen > 1 && (STRLEN)(bp - (STDCHAR*)SvPVX_const(sv)) < rslen) ||
	  memNE((char*)bp - rslen, rsptr, rslen))
	goto screamer;				/* go back to the fray */
thats_really_all_folks:
    if (shortbuffered)
	cnt += shortbuffered;
	DEBUG_P(PerlIO_printf(Perl_debug_log,
	    "Screamer: quitting, ptr=%"UVuf", cnt=%ld\n",PTR2UV(ptr),(long)cnt));
    PerlIO_set_ptrcnt(fp, (STDCHAR*)ptr, cnt);	/* put these back or we're in trouble */
    DEBUG_P(PerlIO_printf(Perl_debug_log,
	"Screamer: end: FILE * thinks ptr=%"UVuf", cnt=%ld, base=%"UVuf"\n",
	PTR2UV(PerlIO_get_ptr(fp)), (long)PerlIO_get_cnt(fp),
	PTR2UV(PerlIO_has_base (fp) ? PerlIO_get_base(fp) : 0)));
    *bp = '\0';
    SvCUR_set(sv, bp - (STDCHAR*)SvPVX_const(sv));	/* set length */
    DEBUG_P(PerlIO_printf(Perl_debug_log,
	"Screamer: done, len=%ld, string=|%.*s|\n",
	(long)SvCUR(sv),(int)SvCUR(sv),SvPVX_const(sv)));
    }
   else
    {
       /*The big, slow, and stupid way. */
#ifdef USE_HEAP_INSTEAD_OF_STACK	/* Even slower way. */
	STDCHAR *buf = NULL;
	Newx(buf, 8192, STDCHAR);
	assert(buf);
#else
	STDCHAR buf[8192];
#endif

screamer2:
	if (rslen) {
            register const STDCHAR * const bpe = buf + sizeof(buf);
	    bp = buf;
	    while ((i = PerlIO_getc(fp)) != EOF && (*bp++ = (STDCHAR)i) != rslast && bp < bpe)
		; /* keep reading */
	    cnt = bp - buf;
	}
	else {
	    cnt = PerlIO_read(fp,(char*)buf, sizeof(buf));
	    /* Accommodate broken VAXC compiler, which applies U8 cast to
	     * both args of ?: operator, causing EOF to change into 255
	     */
	    if (cnt > 0)
		 i = (U8)buf[cnt - 1];
	    else
		 i = EOF;
	}

	if (cnt < 0)
	    cnt = 0;  /* we do need to re-set the sv even when cnt <= 0 */
	if (append)
	     sv_catpvn(sv, (char *) buf, cnt);
	else
	     sv_setpvn(sv, (char *) buf, cnt);

	if (i != EOF &&			/* joy */
	    (!rslen ||
	     SvCUR(sv) < rslen ||
	     memNE(SvPVX_const(sv) + SvCUR(sv) - rslen, rsptr, rslen)))
	{
	    append = -1;
	    /*
	     * If we're reading from a TTY and we get a short read,
	     * indicating that the user hit his EOF character, we need
	     * to notice it now, because if we try to read from the TTY
	     * again, the EOF condition will disappear.
	     *
	     * The comparison of cnt to sizeof(buf) is an optimization
	     * that prevents unnecessary calls to feof().
	     *
	     * - jik 9/25/96
	     */
	    if (!(cnt < (I32)sizeof(buf) && PerlIO_eof(fp)))
		goto screamer2;
	}

#ifdef USE_HEAP_INSTEAD_OF_STACK
	Safefree(buf);
#endif
    }

    if (rspara) {		/* have to do this both before and after */
        while (i != EOF) {	/* to make sure file boundaries work right */
	    i = PerlIO_getc(fp);
	    if (i != '\n') {
		PerlIO_ungetc(fp,i);
		break;
	    }
	}
    }

    return (SvCUR(sv) - append) ? SvPVX(sv) : NULL;
}

/*
=for apidoc sv_inc

Auto-increment of the value in the SV, doing string to numeric conversion
if necessary.  Handles 'get' magic and operator overloading.

=cut
*/

void
Perl_sv_inc(pTHX_ register SV *const sv)
{
    if (!sv)
	return;
    SvGETMAGIC(sv);
    sv_inc_nomg(sv);
}

/*
=for apidoc sv_inc_nomg

Auto-increment of the value in the SV, doing string to numeric conversion
if necessary.  Handles operator overloading.  Skips handling 'get' magic.

=cut
*/

void
Perl_sv_inc_nomg(pTHX_ register SV *const sv)
{
    dVAR;
    register char *d;
    int flags;

    if (!sv)
	return;
    if (SvTHINKFIRST(sv)) {
	if (SvIsCOW(sv) || isGV_with_GP(sv))
	    sv_force_normal_flags(sv, 0);
	if (SvREADONLY(sv)) {
	    if (IN_PERL_RUNTIME)
		Perl_croak_no_modify(aTHX);
	}
	if (SvROK(sv)) {
	    IV i;
	    if (SvAMAGIC(sv) && AMG_CALLunary(sv, inc_amg))
		return;
	    i = PTR2IV(SvRV(sv));
	    sv_unref(sv);
	    sv_setiv(sv, i);
	}
    }
    flags = SvFLAGS(sv);
    if ((flags & (SVp_NOK|SVp_IOK)) == SVp_NOK) {
	/* It's (privately or publicly) a float, but not tested as an
	   integer, so test it to see. */
	(void) SvIV(sv);
	flags = SvFLAGS(sv);
    }
    if ((flags & SVf_IOK) || ((flags & (SVp_IOK | SVp_NOK)) == SVp_IOK)) {
	/* It's publicly an integer, or privately an integer-not-float */
#ifdef PERL_PRESERVE_IVUV
      oops_its_int:
#endif
	if (SvIsUV(sv)) {
	    if (SvUVX(sv) == UV_MAX)
		sv_setnv(sv, UV_MAX_P1);
	    else
		(void)SvIOK_only_UV(sv);
		SvUV_set(sv, SvUVX(sv) + 1);
	} else {
	    if (SvIVX(sv) == IV_MAX)
		sv_setuv(sv, (UV)IV_MAX + 1);
	    else {
		(void)SvIOK_only(sv);
		SvIV_set(sv, SvIVX(sv) + 1);
	    }	
	}
	return;
    }
    if (flags & SVp_NOK) {
	const NV was = SvNVX(sv);
	if (NV_OVERFLOWS_INTEGERS_AT &&
	    was >= NV_OVERFLOWS_INTEGERS_AT) {
	    /* diag_listed_as: Lost precision when %s %f by 1 */
	    Perl_ck_warner(aTHX_ packWARN(WARN_IMPRECISION),
			   "Lost precision when incrementing %" NVff " by 1",
			   was);
	}
	(void)SvNOK_only(sv);
        SvNV_set(sv, was + 1.0);
	return;
    }

    if (!(flags & SVp_POK) || !*SvPVX_const(sv)) {
	if ((flags & SVTYPEMASK) < SVt_PVIV)
	    sv_upgrade(sv, ((flags & SVTYPEMASK) > SVt_IV ? SVt_PVIV : SVt_IV));
	(void)SvIOK_only(sv);
	SvIV_set(sv, 1);
	return;
    }
    d = SvPVX(sv);
    while (isALPHA(*d)) d++;
    while (isDIGIT(*d)) d++;
    if (d < SvEND(sv)) {
#ifdef PERL_PRESERVE_IVUV
	/* Got to punt this as an integer if needs be, but we don't issue
	   warnings. Probably ought to make the sv_iv_please() that does
	   the conversion if possible, and silently.  */
	const int numtype = grok_number(SvPVX_const(sv), SvCUR(sv), NULL);
	if (numtype && !(numtype & IS_NUMBER_INFINITY)) {
	    /* Need to try really hard to see if it's an integer.
	       9.22337203685478e+18 is an integer.
	       but "9.22337203685478e+18" + 0 is UV=9223372036854779904
	       so $a="9.22337203685478e+18"; $a+0; $a++
	       needs to be the same as $a="9.22337203685478e+18"; $a++
	       or we go insane. */
	
	    (void) sv_2iv(sv);
	    if (SvIOK(sv))
		goto oops_its_int;

	    /* sv_2iv *should* have made this an NV */
	    if (flags & SVp_NOK) {
		(void)SvNOK_only(sv);
                SvNV_set(sv, SvNVX(sv) + 1.0);
		return;
	    }
	    /* I don't think we can get here. Maybe I should assert this
	       And if we do get here I suspect that sv_setnv will croak. NWC
	       Fall through. */
#if defined(USE_LONG_DOUBLE)
	    DEBUG_c(PerlIO_printf(Perl_debug_log,"sv_inc punt failed to convert '%s' to IOK or NOKp, UV=0x%"UVxf" NV=%"PERL_PRIgldbl"\n",
				  SvPVX_const(sv), SvIVX(sv), SvNVX(sv)));
#else
	    DEBUG_c(PerlIO_printf(Perl_debug_log,"sv_inc punt failed to convert '%s' to IOK or NOKp, UV=0x%"UVxf" NV=%"NVgf"\n",
				  SvPVX_const(sv), SvIVX(sv), SvNVX(sv)));
#endif
	}
#endif /* PERL_PRESERVE_IVUV */
	sv_setnv(sv,Atof(SvPVX_const(sv)) + 1.0);
	return;
    }
    d--;
    while (d >= SvPVX_const(sv)) {
	if (isDIGIT(*d)) {
	    if (++*d <= '9')
		return;
	    *(d--) = '0';
	}
	else {
#ifdef EBCDIC
	    /* MKS: The original code here died if letters weren't consecutive.
	     * at least it didn't have to worry about non-C locales.  The
	     * new code assumes that ('z'-'a')==('Z'-'A'), letters are
	     * arranged in order (although not consecutively) and that only
	     * [A-Za-z] are accepted by isALPHA in the C locale.
	     */
	    if (*d != 'z' && *d != 'Z') {
		do { ++*d; } while (!isALPHA(*d));
		return;
	    }
	    *(d--) -= 'z' - 'a';
#else
	    ++*d;
	    if (isALPHA(*d))
		return;
	    *(d--) -= 'z' - 'a' + 1;
#endif
	}
    }
    /* oh,oh, the number grew */
    SvGROW(sv, SvCUR(sv) + 2);
    SvCUR_set(sv, SvCUR(sv) + 1);
    for (d = SvPVX(sv) + SvCUR(sv); d > SvPVX_const(sv); d--)
	*d = d[-1];
    if (isDIGIT(d[1]))
	*d = '1';
    else
	*d = d[1];
}

/*
=for apidoc sv_dec

Auto-decrement of the value in the SV, doing string to numeric conversion
if necessary.  Handles 'get' magic and operator overloading.

=cut
*/

void
Perl_sv_dec(pTHX_ register SV *const sv)
{
    dVAR;
    if (!sv)
	return;
    SvGETMAGIC(sv);
    sv_dec_nomg(sv);
}

/*
=for apidoc sv_dec_nomg

Auto-decrement of the value in the SV, doing string to numeric conversion
if necessary.  Handles operator overloading.  Skips handling 'get' magic.

=cut
*/

void
Perl_sv_dec_nomg(pTHX_ register SV *const sv)
{
    dVAR;
    int flags;

    if (!sv)
	return;
    if (SvTHINKFIRST(sv)) {
	if (SvIsCOW(sv) || isGV_with_GP(sv))
	    sv_force_normal_flags(sv, 0);
	if (SvREADONLY(sv)) {
	    if (IN_PERL_RUNTIME)
		Perl_croak_no_modify(aTHX);
	}
	if (SvROK(sv)) {
	    IV i;
	    if (SvAMAGIC(sv) && AMG_CALLunary(sv, dec_amg))
		return;
	    i = PTR2IV(SvRV(sv));
	    sv_unref(sv);
	    sv_setiv(sv, i);
	}
    }
    /* Unlike sv_inc we don't have to worry about string-never-numbers
       and keeping them magic. But we mustn't warn on punting */
    flags = SvFLAGS(sv);
    if ((flags & SVf_IOK) || ((flags & (SVp_IOK | SVp_NOK)) == SVp_IOK)) {
	/* It's publicly an integer, or privately an integer-not-float */
#ifdef PERL_PRESERVE_IVUV
      oops_its_int:
#endif
	if (SvIsUV(sv)) {
	    if (SvUVX(sv) == 0) {
		(void)SvIOK_only(sv);
		SvIV_set(sv, -1);
	    }
	    else {
		(void)SvIOK_only_UV(sv);
		SvUV_set(sv, SvUVX(sv) - 1);
	    }	
	} else {
	    if (SvIVX(sv) == IV_MIN) {
		sv_setnv(sv, (NV)IV_MIN);
		goto oops_its_num;
	    }
	    else {
		(void)SvIOK_only(sv);
		SvIV_set(sv, SvIVX(sv) - 1);
	    }	
	}
	return;
    }
    if (flags & SVp_NOK) {
    oops_its_num:
	{
	    const NV was = SvNVX(sv);
	    if (NV_OVERFLOWS_INTEGERS_AT &&
		was <= -NV_OVERFLOWS_INTEGERS_AT) {
		/* diag_listed_as: Lost precision when %s %f by 1 */
		Perl_ck_warner(aTHX_ packWARN(WARN_IMPRECISION),
			       "Lost precision when decrementing %" NVff " by 1",
			       was);
	    }
	    (void)SvNOK_only(sv);
	    SvNV_set(sv, was - 1.0);
	    return;
	}
    }
    if (!(flags & SVp_POK)) {
	if ((flags & SVTYPEMASK) < SVt_PVIV)
	    sv_upgrade(sv, ((flags & SVTYPEMASK) > SVt_IV) ? SVt_PVIV : SVt_IV);
	SvIV_set(sv, -1);
	(void)SvIOK_only(sv);
	return;
    }
#ifdef PERL_PRESERVE_IVUV
    {
	const int numtype = grok_number(SvPVX_const(sv), SvCUR(sv), NULL);
	if (numtype && !(numtype & IS_NUMBER_INFINITY)) {
	    /* Need to try really hard to see if it's an integer.
	       9.22337203685478e+18 is an integer.
	       but "9.22337203685478e+18" + 0 is UV=9223372036854779904
	       so $a="9.22337203685478e+18"; $a+0; $a--
	       needs to be the same as $a="9.22337203685478e+18"; $a--
	       or we go insane. */
	
	    (void) sv_2iv(sv);
	    if (SvIOK(sv))
		goto oops_its_int;

	    /* sv_2iv *should* have made this an NV */
	    if (flags & SVp_NOK) {
		(void)SvNOK_only(sv);
                SvNV_set(sv, SvNVX(sv) - 1.0);
		return;
	    }
	    /* I don't think we can get here. Maybe I should assert this
	       And if we do get here I suspect that sv_setnv will croak. NWC
	       Fall through. */
#if defined(USE_LONG_DOUBLE)
	    DEBUG_c(PerlIO_printf(Perl_debug_log,"sv_dec punt failed to convert '%s' to IOK or NOKp, UV=0x%"UVxf" NV=%"PERL_PRIgldbl"\n",
				  SvPVX_const(sv), SvIVX(sv), SvNVX(sv)));
#else
	    DEBUG_c(PerlIO_printf(Perl_debug_log,"sv_dec punt failed to convert '%s' to IOK or NOKp, UV=0x%"UVxf" NV=%"NVgf"\n",
				  SvPVX_const(sv), SvIVX(sv), SvNVX(sv)));
#endif
	}
    }
#endif /* PERL_PRESERVE_IVUV */
    sv_setnv(sv,Atof(SvPVX_const(sv)) - 1.0);	/* punt */
}

/* this define is used to eliminate a chunk of duplicated but shared logic
 * it has the suffix __SV_C to signal that it isnt API, and isnt meant to be
 * used anywhere but here - yves
 */
#define PUSH_EXTEND_MORTAL__SV_C(AnSv) \
    STMT_START {      \
	EXTEND_MORTAL(1); \
	PL_tmps_stack[++PL_tmps_ix] = (AnSv); \
    } STMT_END

/*
=for apidoc sv_mortalcopy

Creates a new SV which is a copy of the original SV (using C<sv_setsv>).
The new SV is marked as mortal.  It will be destroyed "soon", either by an
explicit call to FREETMPS, or by an implicit call at places such as
statement boundaries.  See also C<sv_newmortal> and C<sv_2mortal>.

=cut
*/

/* Make a string that will exist for the duration of the expression
 * evaluation.  Actually, it may have to last longer than that, but
 * hopefully we won't free it until it has been assigned to a
 * permanent location. */

SV *
Perl_sv_mortalcopy(pTHX_ SV *const oldstr)
{
    dVAR;
    register SV *sv;

    new_SV(sv);
    sv_setsv(sv,oldstr);
    PUSH_EXTEND_MORTAL__SV_C(sv);
    SvTEMP_on(sv);
    return sv;
}

/*
=for apidoc sv_newmortal

Creates a new null SV which is mortal.  The reference count of the SV is
set to 1.  It will be destroyed "soon", either by an explicit call to
FREETMPS, or by an implicit call at places such as statement boundaries.
See also C<sv_mortalcopy> and C<sv_2mortal>.

=cut
*/

SV *
Perl_sv_newmortal(pTHX)
{
    dVAR;
    register SV *sv;

    new_SV(sv);
    SvFLAGS(sv) = SVs_TEMP;
    PUSH_EXTEND_MORTAL__SV_C(sv);
    return sv;
}


/*
=for apidoc newSVpvn_flags

Creates a new SV and copies a string into it.  The reference count for the
SV is set to 1.  Note that if C<len> is zero, Perl will create a zero length
string.  You are responsible for ensuring that the source string is at least
C<len> bytes long.  If the C<s> argument is NULL the new SV will be undefined.
Currently the only flag bits accepted are C<SVf_UTF8> and C<SVs_TEMP>.
If C<SVs_TEMP> is set, then C<sv_2mortal()> is called on the result before
returning.  If C<SVf_UTF8> is set, C<s>
is considered to be in UTF-8 and the
C<SVf_UTF8> flag will be set on the new SV.
C<newSVpvn_utf8()> is a convenience wrapper for this function, defined as

    #define newSVpvn_utf8(s, len, u)			\
	newSVpvn_flags((s), (len), (u) ? SVf_UTF8 : 0)

=cut
*/

SV *
Perl_newSVpvn_flags(pTHX_ const char *const s, const STRLEN len, const U32 flags)
{
    dVAR;
    register SV *sv;

    /* All the flags we don't support must be zero.
       And we're new code so I'm going to assert this from the start.  */
    assert(!(flags & ~(SVf_UTF8|SVs_TEMP)));
    new_SV(sv);
    sv_setpvn(sv,s,len);

    /* This code used to a sv_2mortal(), however we now unroll the call to sv_2mortal()
     * and do what it does ourselves here.
     * Since we have asserted that flags can only have the SVf_UTF8 and/or SVs_TEMP flags
     * set above we can use it to enable the sv flags directly (bypassing SvTEMP_on), which
     * in turn means we dont need to mask out the SVf_UTF8 flag below, which means that we
     * eliminate quite a few steps than it looks - Yves (explaining patch by gfx)
     */

    SvFLAGS(sv) |= flags;

    if(flags & SVs_TEMP){
	PUSH_EXTEND_MORTAL__SV_C(sv);
    }

    return sv;
}

/*
=for apidoc sv_2mortal

Marks an existing SV as mortal.  The SV will be destroyed "soon", either
by an explicit call to FREETMPS, or by an implicit call at places such as
statement boundaries.  SvTEMP() is turned on which means that the SV's
string buffer can be "stolen" if this SV is copied.  See also C<sv_newmortal>
and C<sv_mortalcopy>.

=cut
*/

SV *
Perl_sv_2mortal(pTHX_ register SV *const sv)
{
    dVAR;
    if (!sv)
	return NULL;
    if (SvREADONLY(sv) && SvIMMORTAL(sv))
	return sv;
    PUSH_EXTEND_MORTAL__SV_C(sv);
    SvTEMP_on(sv);
    return sv;
}

/*
=for apidoc newSVpv

Creates a new SV and copies a string into it.  The reference count for the
SV is set to 1.  If C<len> is zero, Perl will compute the length using
strlen().  For efficiency, consider using C<newSVpvn> instead.

=cut
*/

SV *
Perl_newSVpv(pTHX_ const char *const s, const STRLEN len)
{
    dVAR;
    register SV *sv;

    new_SV(sv);
    sv_setpvn(sv, s, len || s == NULL ? len : strlen(s));
    return sv;
}

/*
=for apidoc newSVpvn

Creates a new SV and copies a buffer into it, which may contain NUL characters
(C<\0>) and other binary data.  The reference count for the SV is set to 1.
Note that if C<len> is zero, Perl will create a zero length (Perl) string.  You
are responsible for ensuring that the source buffer is at least
C<len> bytes long.  If the C<buffer> argument is NULL the new SV will be
undefined.

=cut
*/

SV *
Perl_newSVpvn(pTHX_ const char *const buffer, const STRLEN len)
{
    dVAR;
    register SV *sv;

    new_SV(sv);
    sv_setpvn(sv,buffer,len);
    return sv;
}

/*
=for apidoc newSVhek

Creates a new SV from the hash key structure.  It will generate scalars that
point to the shared string table where possible.  Returns a new (undefined)
SV if the hek is NULL.

=cut
*/

SV *
Perl_newSVhek(pTHX_ const HEK *const hek)
{
    dVAR;
    if (!hek) {
	SV *sv;

	new_SV(sv);
	return sv;
    }

    if (HEK_LEN(hek) == HEf_SVKEY) {
	return newSVsv(*(SV**)HEK_KEY(hek));
    } else {
	const int flags = HEK_FLAGS(hek);
	if (flags & HVhek_WASUTF8) {
	    /* Trouble :-)
	       Andreas would like keys he put in as utf8 to come back as utf8
	    */
	    STRLEN utf8_len = HEK_LEN(hek);
	    SV * const sv = newSV_type(SVt_PV);
	    char *as_utf8 = (char *)bytes_to_utf8 ((U8*)HEK_KEY(hek), &utf8_len);
	    /* bytes_to_utf8() allocates a new string, which we can repurpose: */
	    sv_usepvn_flags(sv, as_utf8, utf8_len, SV_HAS_TRAILING_NUL);
	    SvUTF8_on (sv);
	    return sv;
	} else if (flags & (HVhek_REHASH|HVhek_UNSHARED)) {
	    /* We don't have a pointer to the hv, so we have to replicate the
	       flag into every HEK. This hv is using custom a hasing
	       algorithm. Hence we can't return a shared string scalar, as
	       that would contain the (wrong) hash value, and might get passed
	       into an hv routine with a regular hash.
	       Similarly, a hash that isn't using shared hash keys has to have
	       the flag in every key so that we know not to try to call
	       share_hek_hek on it.  */

	    SV * const sv = newSVpvn (HEK_KEY(hek), HEK_LEN(hek));
	    if (HEK_UTF8(hek))
		SvUTF8_on (sv);
	    return sv;
	}
	/* This will be overwhelminly the most common case.  */
	{
	    /* Inline most of newSVpvn_share(), because share_hek_hek() is far
	       more efficient than sharepvn().  */
	    SV *sv;

	    new_SV(sv);
	    sv_upgrade(sv, SVt_PV);
	    SvPV_set(sv, (char *)HEK_KEY(share_hek_hek(hek)));
	    SvCUR_set(sv, HEK_LEN(hek));
	    SvLEN_set(sv, 0);
	    SvREADONLY_on(sv);
	    SvFAKE_on(sv);
	    SvPOK_on(sv);
	    if (HEK_UTF8(hek))
		SvUTF8_on(sv);
	    return sv;
	}
    }
}

/*
=for apidoc newSVpvn_share

Creates a new SV with its SvPVX_const pointing to a shared string in the string
table.  If the string does not already exist in the table, it is
created first.  Turns on READONLY and FAKE.  If the C<hash> parameter
is non-zero, that value is used; otherwise the hash is computed.
The string's hash can later be retrieved from the SV
with the C<SvSHARED_HASH()> macro.  The idea here is
that as the string table is used for shared hash keys these strings will have
SvPVX_const == HeKEY and hash lookup will avoid string compare.

=cut
*/

SV *
Perl_newSVpvn_share(pTHX_ const char *src, I32 len, U32 hash)
{
    dVAR;
    register SV *sv;
    bool is_utf8 = FALSE;
    const char *const orig_src = src;

    if (len < 0) {
	STRLEN tmplen = -len;
        is_utf8 = TRUE;
	/* See the note in hv.c:hv_fetch() --jhi */
	src = (char*)bytes_from_utf8((const U8*)src, &tmplen, &is_utf8);
	len = tmplen;
    }
    if (!hash)
	PERL_HASH(hash, src, len);
    new_SV(sv);
    /* The logic for this is inlined in S_mro_get_linear_isa_dfs(), so if it
       changes here, update it there too.  */
    sv_upgrade(sv, SVt_PV);
    SvPV_set(sv, sharepvn(src, is_utf8?-len:len, hash));
    SvCUR_set(sv, len);
    SvLEN_set(sv, 0);
    SvREADONLY_on(sv);
    SvFAKE_on(sv);
    SvPOK_on(sv);
    if (is_utf8)
        SvUTF8_on(sv);
    if (src != orig_src)
	Safefree(src);
    return sv;
}

/*
=for apidoc newSVpv_share

Like C<newSVpvn_share>, but takes a nul-terminated string instead of a
string/length pair.

=cut
*/

SV *
Perl_newSVpv_share(pTHX_ const char *src, U32 hash)
{
    return newSVpvn_share(src, strlen(src), hash);
}

#if defined(PERL_IMPLICIT_CONTEXT)

/* pTHX_ magic can't cope with varargs, so this is a no-context
 * version of the main function, (which may itself be aliased to us).
 * Don't access this version directly.
 */

SV *
Perl_newSVpvf_nocontext(const char *const pat, ...)
{
    dTHX;
    register SV *sv;
    va_list args;

    PERL_ARGS_ASSERT_NEWSVPVF_NOCONTEXT;

    va_start(args, pat);
    sv = vnewSVpvf(pat, &args);
    va_end(args);
    return sv;
}
#endif

/*
=for apidoc newSVpvf

Creates a new SV and initializes it with the string formatted like
C<sprintf>.

=cut
*/

SV *
Perl_newSVpvf(pTHX_ const char *const pat, ...)
{
    register SV *sv;
    va_list args;

    PERL_ARGS_ASSERT_NEWSVPVF;

    va_start(args, pat);
    sv = vnewSVpvf(pat, &args);
    va_end(args);
    return sv;
}

/* backend for newSVpvf() and newSVpvf_nocontext() */

SV *
Perl_vnewSVpvf(pTHX_ const char *const pat, va_list *const args)
{
    dVAR;
    register SV *sv;

    PERL_ARGS_ASSERT_VNEWSVPVF;

    new_SV(sv);
    sv_vsetpvfn(sv, pat, strlen(pat), args, NULL, 0, NULL);
    return sv;
}

/*
=for apidoc newSVnv

Creates a new SV and copies a floating point value into it.
The reference count for the SV is set to 1.

=cut
*/

SV *
Perl_newSVnv(pTHX_ const NV n)
{
    dVAR;
    register SV *sv;

    new_SV(sv);
    sv_setnv(sv,n);
    return sv;
}

/*
=for apidoc newSViv

Creates a new SV and copies an integer into it.  The reference count for the
SV is set to 1.

=cut
*/

SV *
Perl_newSViv(pTHX_ const IV i)
{
    dVAR;
    register SV *sv;

    new_SV(sv);
    sv_setiv(sv,i);
    return sv;
}

/*
=for apidoc newSVuv

Creates a new SV and copies an unsigned integer into it.
The reference count for the SV is set to 1.

=cut
*/

SV *
Perl_newSVuv(pTHX_ const UV u)
{
    dVAR;
    register SV *sv;

    new_SV(sv);
    sv_setuv(sv,u);
    return sv;
}

/*
=for apidoc newSV_type

Creates a new SV, of the type specified.  The reference count for the new SV
is set to 1.

=cut
*/

SV *
Perl_newSV_type(pTHX_ const svtype type)
{
    register SV *sv;

    new_SV(sv);
    sv_upgrade(sv, type);
    return sv;
}

/*
=for apidoc newRV_noinc

Creates an RV wrapper for an SV.  The reference count for the original
SV is B<not> incremented.

=cut
*/

SV *
Perl_newRV_noinc(pTHX_ SV *const tmpRef)
{
    dVAR;
    register SV *sv = newSV_type(SVt_IV);

    PERL_ARGS_ASSERT_NEWRV_NOINC;

    SvTEMP_off(tmpRef);
    SvRV_set(sv, tmpRef);
    SvROK_on(sv);
    return sv;
}

/* newRV_inc is the official function name to use now.
 * newRV_inc is in fact #defined to newRV in sv.h
 */

SV *
Perl_newRV(pTHX_ SV *const sv)
{
    dVAR;

    PERL_ARGS_ASSERT_NEWRV;

    return newRV_noinc(SvREFCNT_inc_simple_NN(sv));
}

/*
=for apidoc newSVsv

Creates a new SV which is an exact duplicate of the original SV.
(Uses C<sv_setsv>.)

=cut
*/

SV *
Perl_newSVsv(pTHX_ register SV *const old)
{
    dVAR;
    register SV *sv;

    if (!old)
	return NULL;
    if (SvTYPE(old) == (svtype)SVTYPEMASK) {
	Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL), "semi-panic: attempt to dup freed string");
	return NULL;
    }
    new_SV(sv);
    /* SV_GMAGIC is the default for sv_setv()
       SV_NOSTEAL prevents TEMP buffers being, well, stolen, and saves games
       with SvTEMP_off and SvTEMP_on round a call to sv_setsv.  */
    sv_setsv_flags(sv, old, SV_GMAGIC | SV_NOSTEAL);
    return sv;
}

/*
=for apidoc sv_reset

Underlying implementation for the C<reset> Perl function.
Note that the perl-level function is vaguely deprecated.

=cut
*/

void
Perl_sv_reset(pTHX_ register const char *s, HV *const stash)
{
    dVAR;
    char todo[PERL_UCHAR_MAX+1];

    PERL_ARGS_ASSERT_SV_RESET;

    if (!stash)
	return;

    if (!*s) {		/* reset ?? searches */
	MAGIC * const mg = mg_find((const SV *)stash, PERL_MAGIC_symtab);
	if (mg) {
	    const U32 count = mg->mg_len / sizeof(PMOP**);
	    PMOP **pmp = (PMOP**) mg->mg_ptr;
	    PMOP *const *const end = pmp + count;

	    while (pmp < end) {
#ifdef USE_ITHREADS
                SvREADONLY_off(PL_regex_pad[(*pmp)->op_pmoffset]);
#else
		(*pmp)->op_pmflags &= ~PMf_USED;
#endif
		++pmp;
	    }
	}
	return;
    }

    /* reset variables */

    if (!HvARRAY(stash))
	return;

    Zero(todo, 256, char);
    while (*s) {
	I32 max;
	I32 i = (unsigned char)*s;
	if (s[1] == '-') {
	    s += 2;
	}
	max = (unsigned char)*s++;
	for ( ; i <= max; i++) {
	    todo[i] = 1;
	}
	for (i = 0; i <= (I32) HvMAX(stash); i++) {
	    HE *entry;
	    for (entry = HvARRAY(stash)[i];
		 entry;
		 entry = HeNEXT(entry))
	    {
		register GV *gv;
		register SV *sv;

		if (!todo[(U8)*HeKEY(entry)])
		    continue;
		gv = MUTABLE_GV(HeVAL(entry));
		sv = GvSV(gv);
		if (sv) {
		    if (SvTHINKFIRST(sv)) {
			if (!SvREADONLY(sv) && SvROK(sv))
			    sv_unref(sv);
			/* XXX Is this continue a bug? Why should THINKFIRST
			   exempt us from resetting arrays and hashes?  */
			continue;
		    }
		    SvOK_off(sv);
		    if (SvTYPE(sv) >= SVt_PV) {
			SvCUR_set(sv, 0);
			if (SvPVX_const(sv) != NULL)
			    *SvPVX(sv) = '\0';
			SvTAINT(sv);
		    }
		}
		if (GvAV(gv)) {
		    av_clear(GvAV(gv));
		}
		if (GvHV(gv) && !HvNAME_get(GvHV(gv))) {
#if defined(VMS)
		    Perl_die(aTHX_ "Can't reset %%ENV on this system");
#else /* ! VMS */
		    hv_clear(GvHV(gv));
#  if defined(USE_ENVIRON_ARRAY)
		    if (gv == PL_envgv)
		        my_clearenv();
#  endif /* USE_ENVIRON_ARRAY */
#endif /* VMS */
		}
	    }
	}
    }
}

/*
=for apidoc sv_2io

Using various gambits, try to get an IO from an SV: the IO slot if its a
GV; or the recursive result if we're an RV; or the IO slot of the symbol
named after the PV if we're a string.

'Get' magic is ignored on the sv passed in, but will be called on
C<SvRV(sv)> if sv is an RV.

=cut
*/

IO*
Perl_sv_2io(pTHX_ SV *const sv)
{
    IO* io;
    GV* gv;

    PERL_ARGS_ASSERT_SV_2IO;

    switch (SvTYPE(sv)) {
    case SVt_PVIO:
	io = MUTABLE_IO(sv);
	break;
    case SVt_PVGV:
    case SVt_PVLV:
	if (isGV_with_GP(sv)) {
	    gv = MUTABLE_GV(sv);
	    io = GvIO(gv);
	    if (!io)
		Perl_croak(aTHX_ "Bad filehandle: %"HEKf,
                                    HEKfARG(GvNAME_HEK(gv)));
	    break;
	}
	/* FALL THROUGH */
    default:
	if (!SvOK(sv))
	    Perl_croak(aTHX_ PL_no_usym, "filehandle");
	if (SvROK(sv)) {
	    SvGETMAGIC(SvRV(sv));
	    return sv_2io(SvRV(sv));
	}
	gv = gv_fetchsv_nomg(sv, 0, SVt_PVIO);
	if (gv)
	    io = GvIO(gv);
	else
	    io = 0;
	if (!io) {
	    SV *newsv = sv;
	    if (SvGMAGICAL(sv)) {
		newsv = sv_newmortal();
		sv_setsv_nomg(newsv, sv);
	    }
	    Perl_croak(aTHX_ "Bad filehandle: %"SVf, SVfARG(newsv));
	}
	break;
    }
    return io;
}

/*
=for apidoc sv_2cv

Using various gambits, try to get a CV from an SV; in addition, try if
possible to set C<*st> and C<*gvp> to the stash and GV associated with it.
The flags in C<lref> are passed to gv_fetchsv.

=cut
*/

CV *
Perl_sv_2cv(pTHX_ SV *sv, HV **const st, GV **const gvp, const I32 lref)
{
    dVAR;
    GV *gv = NULL;
    CV *cv = NULL;

    PERL_ARGS_ASSERT_SV_2CV;

    if (!sv) {
	*st = NULL;
	*gvp = NULL;
	return NULL;
    }
    switch (SvTYPE(sv)) {
    case SVt_PVCV:
	*st = CvSTASH(sv);
	*gvp = NULL;
	return MUTABLE_CV(sv);
    case SVt_PVHV:
    case SVt_PVAV:
	*st = NULL;
	*gvp = NULL;
	return NULL;
    default:
	SvGETMAGIC(sv);
	if (SvROK(sv)) {
	    if (SvAMAGIC(sv))
		sv = amagic_deref_call(sv, to_cv_amg);

	    sv = SvRV(sv);
	    if (SvTYPE(sv) == SVt_PVCV) {
		cv = MUTABLE_CV(sv);
		*gvp = NULL;
		*st = CvSTASH(cv);
		return cv;
	    }
	    else if(SvGETMAGIC(sv), isGV_with_GP(sv))
		gv = MUTABLE_GV(sv);
	    else
		Perl_croak(aTHX_ "Not a subroutine reference");
	}
	else if (isGV_with_GP(sv)) {
	    gv = MUTABLE_GV(sv);
	}
	else {
	    gv = gv_fetchsv_nomg(sv, lref, SVt_PVCV);
	}
	*gvp = gv;
	if (!gv) {
	    *st = NULL;
	    return NULL;
	}
	/* Some flags to gv_fetchsv mean don't really create the GV  */
	if (!isGV_with_GP(gv)) {
	    *st = NULL;
	    return NULL;
	}
	*st = GvESTASH(gv);
	if (lref & ~GV_ADDMG && !GvCVu(gv)) {
	    SV *tmpsv;
	    ENTER;
	    tmpsv = newSV(0);
	    gv_efullname3(tmpsv, gv, NULL);
	    /* XXX this is probably not what they think they're getting.
	     * It has the same effect as "sub name;", i.e. just a forward
	     * declaration! */
	    newSUB(start_subparse(FALSE, 0),
		   newSVOP(OP_CONST, 0, tmpsv),
		   NULL, NULL);
	    LEAVE;
	    if (!GvCVu(gv))
		Perl_croak(aTHX_ "Unable to create sub named \"%"SVf"\"",
			   SVfARG(SvOK(sv) ? sv : &PL_sv_no));
	}
	return GvCVu(gv);
    }
}

/*
=for apidoc sv_true

Returns true if the SV has a true value by Perl's rules.
Use the C<SvTRUE> macro instead, which may call C<sv_true()> or may
instead use an in-line version.

=cut
*/

I32
Perl_sv_true(pTHX_ register SV *const sv)
{
    if (!sv)
	return 0;
    if (SvPOK(sv)) {
	register const XPV* const tXpv = (XPV*)SvANY(sv);
	if (tXpv &&
		(tXpv->xpv_cur > 1 ||
		(tXpv->xpv_cur && *sv->sv_u.svu_pv != '0')))
	    return 1;
	else
	    return 0;
    }
    else {
	if (SvIOK(sv))
	    return SvIVX(sv) != 0;
	else {
	    if (SvNOK(sv))
		return SvNVX(sv) != 0.0;
	    else
		return sv_2bool(sv);
	}
    }
}

/*
=for apidoc sv_pvn_force

Get a sensible string out of the SV somehow.
A private implementation of the C<SvPV_force> macro for compilers which
can't cope with complex macro expressions.  Always use the macro instead.

=for apidoc sv_pvn_force_flags

Get a sensible string out of the SV somehow.
If C<flags> has C<SV_GMAGIC> bit set, will C<mg_get> on C<sv> if
appropriate, else not.  C<sv_pvn_force> and C<sv_pvn_force_nomg> are
implemented in terms of this function.
You normally want to use the various wrapper macros instead: see
C<SvPV_force> and C<SvPV_force_nomg>

=cut
*/

char *
Perl_sv_pvn_force_flags(pTHX_ SV *const sv, STRLEN *const lp, const I32 flags)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_PVN_FORCE_FLAGS;

    if (flags & SV_GMAGIC) SvGETMAGIC(sv);
    if (SvTHINKFIRST(sv) && !SvROK(sv))
        sv_force_normal_flags(sv, 0);

    if (SvPOK(sv)) {
	if (lp)
	    *lp = SvCUR(sv);
    }
    else {
	char *s;
	STRLEN len;
 
	if (SvREADONLY(sv) && !(flags & SV_MUTABLE_RETURN)) {
	    const char * const ref = sv_reftype(sv,0);
	    if (PL_op)
		Perl_croak(aTHX_ "Can't coerce readonly %s to string in %s",
			   ref, OP_DESC(PL_op));
	    else
		Perl_croak(aTHX_ "Can't coerce readonly %s to string", ref);
	}
	if ((SvTYPE(sv) > SVt_PVLV && SvTYPE(sv) != SVt_PVFM)
	    || isGV_with_GP(sv))
	    /* diag_listed_as: Can't coerce %s to %s in %s */
	    Perl_croak(aTHX_ "Can't coerce %s to string in %s", sv_reftype(sv,0),
		OP_DESC(PL_op));
	s = sv_2pv_flags(sv, &len, flags &~ SV_GMAGIC);
	if (lp)
	    *lp = len;

	if (s != SvPVX_const(sv)) {	/* Almost, but not quite, sv_setpvn() */
	    if (SvROK(sv))
		sv_unref(sv);
	    SvUPGRADE(sv, SVt_PV);		/* Never FALSE */
	    SvGROW(sv, len + 1);
	    Move(s,SvPVX(sv),len,char);
	    SvCUR_set(sv, len);
	    SvPVX(sv)[len] = '\0';
	}
	if (!SvPOK(sv)) {
	    SvPOK_on(sv);		/* validate pointer */
	    SvTAINT(sv);
	    DEBUG_c(PerlIO_printf(Perl_debug_log, "0x%"UVxf" 2pv(%s)\n",
				  PTR2UV(sv),SvPVX_const(sv)));
	}
    }
    return SvPVX_mutable(sv);
}

/*
=for apidoc sv_pvbyten_force

The backend for the C<SvPVbytex_force> macro.  Always use the macro
instead.

=cut
*/

char *
Perl_sv_pvbyten_force(pTHX_ SV *const sv, STRLEN *const lp)
{
    PERL_ARGS_ASSERT_SV_PVBYTEN_FORCE;

    sv_pvn_force(sv,lp);
    sv_utf8_downgrade(sv,0);
    *lp = SvCUR(sv);
    return SvPVX(sv);
}

/*
=for apidoc sv_pvutf8n_force

The backend for the C<SvPVutf8x_force> macro.  Always use the macro
instead.

=cut
*/

char *
Perl_sv_pvutf8n_force(pTHX_ SV *const sv, STRLEN *const lp)
{
    PERL_ARGS_ASSERT_SV_PVUTF8N_FORCE;

    sv_pvn_force(sv,lp);
    sv_utf8_upgrade(sv);
    *lp = SvCUR(sv);
    return SvPVX(sv);
}

/*
=for apidoc sv_reftype

Returns a string describing what the SV is a reference to.

=cut
*/

const char *
Perl_sv_reftype(pTHX_ const SV *const sv, const int ob)
{
    PERL_ARGS_ASSERT_SV_REFTYPE;
    if (ob && SvOBJECT(sv)) {
	return SvPV_nolen_const(sv_ref(NULL, sv, ob));
    }
    else {
	switch (SvTYPE(sv)) {
	case SVt_NULL:
	case SVt_IV:
	case SVt_NV:
	case SVt_PV:
	case SVt_PVIV:
	case SVt_PVNV:
	case SVt_PVMG:
				if (SvVOK(sv))
				    return "VSTRING";
				if (SvROK(sv))
				    return "REF";
				else
				    return "SCALAR";

	case SVt_PVLV:		return (char *)  (SvROK(sv) ? "REF"
				/* tied lvalues should appear to be
				 * scalars for backwards compatibility */
				: (LvTYPE(sv) == 't' || LvTYPE(sv) == 'T')
				    ? "SCALAR" : "LVALUE");
	case SVt_PVAV:		return "ARRAY";
	case SVt_PVHV:		return "HASH";
	case SVt_PVCV:		return "CODE";
	case SVt_PVGV:		return (char *) (isGV_with_GP(sv)
				    ? "GLOB" : "SCALAR");
	case SVt_PVFM:		return "FORMAT";
	case SVt_PVIO:		return "IO";
	case SVt_BIND:		return "BIND";
	case SVt_REGEXP:	return "REGEXP";
	default:		return "UNKNOWN";
	}
    }
}

/*
=for apidoc sv_ref

Returns a SV describing what the SV passed in is a reference to.

=cut
*/

SV *
Perl_sv_ref(pTHX_ register SV *dst, const SV *const sv, const int ob)
{
    PERL_ARGS_ASSERT_SV_REF;

    if (!dst)
        dst = sv_newmortal();

    if (ob && SvOBJECT(sv)) {
	HvNAME_get(SvSTASH(sv))
                    ? sv_sethek(dst, HvNAME_HEK(SvSTASH(sv)))
                    : sv_setpvn(dst, "__ANON__", 8);
    }
    else {
        const char * reftype = sv_reftype(sv, 0);
        sv_setpv(dst, reftype);
    }
    return dst;
}

/*
=for apidoc sv_isobject

Returns a boolean indicating whether the SV is an RV pointing to a blessed
object.  If the SV is not an RV, or if the object is not blessed, then this
will return false.

=cut
*/

int
Perl_sv_isobject(pTHX_ SV *sv)
{
    if (!sv)
	return 0;
    SvGETMAGIC(sv);
    if (!SvROK(sv))
	return 0;
    sv = SvRV(sv);
    if (!SvOBJECT(sv))
	return 0;
    return 1;
}

/*
=for apidoc sv_isa

Returns a boolean indicating whether the SV is blessed into the specified
class.  This does not check for subtypes; use C<sv_derived_from> to verify
an inheritance relationship.

=cut
*/

int
Perl_sv_isa(pTHX_ SV *sv, const char *const name)
{
    const char *hvname;

    PERL_ARGS_ASSERT_SV_ISA;

    if (!sv)
	return 0;
    SvGETMAGIC(sv);
    if (!SvROK(sv))
	return 0;
    sv = SvRV(sv);
    if (!SvOBJECT(sv))
	return 0;
    hvname = HvNAME_get(SvSTASH(sv));
    if (!hvname)
	return 0;

    return strEQ(hvname, name);
}

/*
=for apidoc newSVrv

Creates a new SV for the RV, C<rv>, to point to.  If C<rv> is not an RV then
it will be upgraded to one.  If C<classname> is non-null then the new SV will
be blessed in the specified package.  The new SV is returned and its
reference count is 1.

=cut
*/

SV*
Perl_newSVrv(pTHX_ SV *const rv, const char *const classname)
{
    dVAR;
    SV *sv;

    PERL_ARGS_ASSERT_NEWSVRV;

    new_SV(sv);

    SV_CHECK_THINKFIRST_COW_DROP(rv);
    (void)SvAMAGIC_off(rv);

    if (SvTYPE(rv) >= SVt_PVMG) {
	const U32 refcnt = SvREFCNT(rv);
	SvREFCNT(rv) = 0;
	sv_clear(rv);
	SvFLAGS(rv) = 0;
	SvREFCNT(rv) = refcnt;

	sv_upgrade(rv, SVt_IV);
    } else if (SvROK(rv)) {
	SvREFCNT_dec(SvRV(rv));
    } else {
	prepare_SV_for_RV(rv);
    }

    SvOK_off(rv);
    SvRV_set(rv, sv);
    SvROK_on(rv);

    if (classname) {
	HV* const stash = gv_stashpv(classname, GV_ADD);
	(void)sv_bless(rv, stash);
    }
    return sv;
}

/*
=for apidoc sv_setref_pv

Copies a pointer into a new SV, optionally blessing the SV.  The C<rv>
argument will be upgraded to an RV.  That RV will be modified to point to
the new SV.  If the C<pv> argument is NULL then C<PL_sv_undef> will be placed
into the SV.  The C<classname> argument indicates the package for the
blessing.  Set C<classname> to C<NULL> to avoid the blessing.  The new SV
will have a reference count of 1, and the RV will be returned.

Do not use with other Perl types such as HV, AV, SV, CV, because those
objects will become corrupted by the pointer copy process.

Note that C<sv_setref_pvn> copies the string while this copies the pointer.

=cut
*/

SV*
Perl_sv_setref_pv(pTHX_ SV *const rv, const char *const classname, void *const pv)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_SETREF_PV;

    if (!pv) {
	sv_setsv(rv, &PL_sv_undef);
	SvSETMAGIC(rv);
    }
    else
	sv_setiv(newSVrv(rv,classname), PTR2IV(pv));
    return rv;
}

/*
=for apidoc sv_setref_iv

Copies an integer into a new SV, optionally blessing the SV.  The C<rv>
argument will be upgraded to an RV.  That RV will be modified to point to
the new SV.  The C<classname> argument indicates the package for the
blessing.  Set C<classname> to C<NULL> to avoid the blessing.  The new SV
will have a reference count of 1, and the RV will be returned.

=cut
*/

SV*
Perl_sv_setref_iv(pTHX_ SV *const rv, const char *const classname, const IV iv)
{
    PERL_ARGS_ASSERT_SV_SETREF_IV;

    sv_setiv(newSVrv(rv,classname), iv);
    return rv;
}

/*
=for apidoc sv_setref_uv

Copies an unsigned integer into a new SV, optionally blessing the SV.  The C<rv>
argument will be upgraded to an RV.  That RV will be modified to point to
the new SV.  The C<classname> argument indicates the package for the
blessing.  Set C<classname> to C<NULL> to avoid the blessing.  The new SV
will have a reference count of 1, and the RV will be returned.

=cut
*/

SV*
Perl_sv_setref_uv(pTHX_ SV *const rv, const char *const classname, const UV uv)
{
    PERL_ARGS_ASSERT_SV_SETREF_UV;

    sv_setuv(newSVrv(rv,classname), uv);
    return rv;
}

/*
=for apidoc sv_setref_nv

Copies a double into a new SV, optionally blessing the SV.  The C<rv>
argument will be upgraded to an RV.  That RV will be modified to point to
the new SV.  The C<classname> argument indicates the package for the
blessing.  Set C<classname> to C<NULL> to avoid the blessing.  The new SV
will have a reference count of 1, and the RV will be returned.

=cut
*/

SV*
Perl_sv_setref_nv(pTHX_ SV *const rv, const char *const classname, const NV nv)
{
    PERL_ARGS_ASSERT_SV_SETREF_NV;

    sv_setnv(newSVrv(rv,classname), nv);
    return rv;
}

/*
=for apidoc sv_setref_pvn

Copies a string into a new SV, optionally blessing the SV.  The length of the
string must be specified with C<n>.  The C<rv> argument will be upgraded to
an RV.  That RV will be modified to point to the new SV.  The C<classname>
argument indicates the package for the blessing.  Set C<classname> to
C<NULL> to avoid the blessing.  The new SV will have a reference count
of 1, and the RV will be returned.

Note that C<sv_setref_pv> copies the pointer while this copies the string.

=cut
*/

SV*
Perl_sv_setref_pvn(pTHX_ SV *const rv, const char *const classname,
                   const char *const pv, const STRLEN n)
{
    PERL_ARGS_ASSERT_SV_SETREF_PVN;

    sv_setpvn(newSVrv(rv,classname), pv, n);
    return rv;
}

/*
=for apidoc sv_bless

Blesses an SV into a specified package.  The SV must be an RV.  The package
must be designated by its stash (see C<gv_stashpv()>).  The reference count
of the SV is unaffected.

=cut
*/

SV*
Perl_sv_bless(pTHX_ SV *const sv, HV *const stash)
{
    dVAR;
    SV *tmpRef;

    PERL_ARGS_ASSERT_SV_BLESS;

    if (!SvROK(sv))
        Perl_croak(aTHX_ "Can't bless non-reference value");
    tmpRef = SvRV(sv);
    if (SvFLAGS(tmpRef) & (SVs_OBJECT|SVf_READONLY)) {
	if (SvIsCOW(tmpRef))
	    sv_force_normal_flags(tmpRef, 0);
	if (SvREADONLY(tmpRef))
	    Perl_croak_no_modify(aTHX);
	if (SvOBJECT(tmpRef)) {
	    if (SvTYPE(tmpRef) != SVt_PVIO)
		--PL_sv_objcount;
	    SvREFCNT_dec(SvSTASH(tmpRef));
	}
    }
    SvOBJECT_on(tmpRef);
    if (SvTYPE(tmpRef) != SVt_PVIO)
	++PL_sv_objcount;
    SvUPGRADE(tmpRef, SVt_PVMG);
    SvSTASH_set(tmpRef, MUTABLE_HV(SvREFCNT_inc_simple(stash)));

    if (Gv_AMG(stash))
	SvAMAGIC_on(sv);
    else
	(void)SvAMAGIC_off(sv);

    if(SvSMAGICAL(tmpRef))
        if(mg_find(tmpRef, PERL_MAGIC_ext) || mg_find(tmpRef, PERL_MAGIC_uvar))
            mg_set(tmpRef);



    return sv;
}

/* Downgrades a PVGV to a PVMG. If it's actually a PVLV, we leave the type
 * as it is after unglobbing it.
 */

PERL_STATIC_INLINE void
S_sv_unglob(pTHX_ SV *const sv, U32 flags)
{
    dVAR;
    void *xpvmg;
    HV *stash;
    SV * const temp = flags & SV_COW_DROP_PV ? NULL : sv_newmortal();

    PERL_ARGS_ASSERT_SV_UNGLOB;

    assert(SvTYPE(sv) == SVt_PVGV || SvTYPE(sv) == SVt_PVLV);
    SvFAKE_off(sv);
    if (!(flags & SV_COW_DROP_PV))
	gv_efullname3(temp, MUTABLE_GV(sv), "*");

    if (GvGP(sv)) {
        if(GvCVu((const GV *)sv) && (stash = GvSTASH(MUTABLE_GV(sv)))
	   && HvNAME_get(stash))
            mro_method_changed_in(stash);
	gp_free(MUTABLE_GV(sv));
    }
    if (GvSTASH(sv)) {
	sv_del_backref(MUTABLE_SV(GvSTASH(sv)), sv);
	GvSTASH(sv) = NULL;
    }
    GvMULTI_off(sv);
    if (GvNAME_HEK(sv)) {
	unshare_hek(GvNAME_HEK(sv));
    }
    isGV_with_GP_off(sv);

    if(SvTYPE(sv) == SVt_PVGV) {
	/* need to keep SvANY(sv) in the right arena */
	xpvmg = new_XPVMG();
	StructCopy(SvANY(sv), xpvmg, XPVMG);
	del_XPVGV(SvANY(sv));
	SvANY(sv) = xpvmg;

	SvFLAGS(sv) &= ~SVTYPEMASK;
	SvFLAGS(sv) |= SVt_PVMG;
    }

    /* Intentionally not calling any local SET magic, as this isn't so much a
       set operation as merely an internal storage change.  */
    if (flags & SV_COW_DROP_PV) SvOK_off(sv);
    else sv_setsv_flags(sv, temp, 0);

    if ((const GV *)sv == PL_last_in_gv)
	PL_last_in_gv = NULL;
    else if ((const GV *)sv == PL_statgv)
	PL_statgv = NULL;
}

/*
=for apidoc sv_unref_flags

Unsets the RV status of the SV, and decrements the reference count of
whatever was being referenced by the RV.  This can almost be thought of
as a reversal of C<newSVrv>.  The C<cflags> argument can contain
C<SV_IMMEDIATE_UNREF> to force the reference count to be decremented
(otherwise the decrementing is conditional on the reference count being
different from one or the reference being a readonly SV).
See C<SvROK_off>.

=cut
*/

void
Perl_sv_unref_flags(pTHX_ SV *const ref, const U32 flags)
{
    SV* const target = SvRV(ref);

    PERL_ARGS_ASSERT_SV_UNREF_FLAGS;

    if (SvWEAKREF(ref)) {
    	sv_del_backref(target, ref);
	SvWEAKREF_off(ref);
	SvRV_set(ref, NULL);
	return;
    }
    SvRV_set(ref, NULL);
    SvROK_off(ref);
    /* You can't have a || SvREADONLY(target) here, as $a = $$a, where $a was
       assigned to as BEGIN {$a = \"Foo"} will fail.  */
    if (SvREFCNT(target) != 1 || (flags & SV_IMMEDIATE_UNREF))
	SvREFCNT_dec(target);
    else /* XXX Hack, but hard to make $a=$a->[1] work otherwise */
	sv_2mortal(target);	/* Schedule for freeing later */
}

/*
=for apidoc sv_untaint

Untaint an SV.  Use C<SvTAINTED_off> instead.

=cut
*/

void
Perl_sv_untaint(pTHX_ SV *const sv)
{
    PERL_ARGS_ASSERT_SV_UNTAINT;

    if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
	MAGIC * const mg = mg_find(sv, PERL_MAGIC_taint);
	if (mg)
	    mg->mg_len &= ~1;
    }
}

/*
=for apidoc sv_tainted

Test an SV for taintedness.  Use C<SvTAINTED> instead.

=cut
*/

bool
Perl_sv_tainted(pTHX_ SV *const sv)
{
    PERL_ARGS_ASSERT_SV_TAINTED;

    if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
	const MAGIC * const mg = mg_find(sv, PERL_MAGIC_taint);
	if (mg && (mg->mg_len & 1) )
	    return TRUE;
    }
    return FALSE;
}

/*
=for apidoc sv_setpviv

Copies an integer into the given SV, also updating its string value.
Does not handle 'set' magic.  See C<sv_setpviv_mg>.

=cut
*/

void
Perl_sv_setpviv(pTHX_ SV *const sv, const IV iv)
{
    char buf[TYPE_CHARS(UV)];
    char *ebuf;
    char * const ptr = uiv_2buf(buf, iv, 0, 0, &ebuf);

    PERL_ARGS_ASSERT_SV_SETPVIV;

    sv_setpvn(sv, ptr, ebuf - ptr);
}

/*
=for apidoc sv_setpviv_mg

Like C<sv_setpviv>, but also handles 'set' magic.

=cut
*/

void
Perl_sv_setpviv_mg(pTHX_ SV *const sv, const IV iv)
{
    PERL_ARGS_ASSERT_SV_SETPVIV_MG;

    sv_setpviv(sv, iv);
    SvSETMAGIC(sv);
}

#if defined(PERL_IMPLICIT_CONTEXT)

/* pTHX_ magic can't cope with varargs, so this is a no-context
 * version of the main function, (which may itself be aliased to us).
 * Don't access this version directly.
 */

void
Perl_sv_setpvf_nocontext(SV *const sv, const char *const pat, ...)
{
    dTHX;
    va_list args;

    PERL_ARGS_ASSERT_SV_SETPVF_NOCONTEXT;

    va_start(args, pat);
    sv_vsetpvf(sv, pat, &args);
    va_end(args);
}

/* pTHX_ magic can't cope with varargs, so this is a no-context
 * version of the main function, (which may itself be aliased to us).
 * Don't access this version directly.
 */

void
Perl_sv_setpvf_mg_nocontext(SV *const sv, const char *const pat, ...)
{
    dTHX;
    va_list args;

    PERL_ARGS_ASSERT_SV_SETPVF_MG_NOCONTEXT;

    va_start(args, pat);
    sv_vsetpvf_mg(sv, pat, &args);
    va_end(args);
}
#endif

/*
=for apidoc sv_setpvf

Works like C<sv_catpvf> but copies the text into the SV instead of
appending it.  Does not handle 'set' magic.  See C<sv_setpvf_mg>.

=cut
*/

void
Perl_sv_setpvf(pTHX_ SV *const sv, const char *const pat, ...)
{
    va_list args;

    PERL_ARGS_ASSERT_SV_SETPVF;

    va_start(args, pat);
    sv_vsetpvf(sv, pat, &args);
    va_end(args);
}

/*
=for apidoc sv_vsetpvf

Works like C<sv_vcatpvf> but copies the text into the SV instead of
appending it.  Does not handle 'set' magic.  See C<sv_vsetpvf_mg>.

Usually used via its frontend C<sv_setpvf>.

=cut
*/

void
Perl_sv_vsetpvf(pTHX_ SV *const sv, const char *const pat, va_list *const args)
{
    PERL_ARGS_ASSERT_SV_VSETPVF;

    sv_vsetpvfn(sv, pat, strlen(pat), args, NULL, 0, NULL);
}

/*
=for apidoc sv_setpvf_mg

Like C<sv_setpvf>, but also handles 'set' magic.

=cut
*/

void
Perl_sv_setpvf_mg(pTHX_ SV *const sv, const char *const pat, ...)
{
    va_list args;

    PERL_ARGS_ASSERT_SV_SETPVF_MG;

    va_start(args, pat);
    sv_vsetpvf_mg(sv, pat, &args);
    va_end(args);
}

/*
=for apidoc sv_vsetpvf_mg

Like C<sv_vsetpvf>, but also handles 'set' magic.

Usually used via its frontend C<sv_setpvf_mg>.

=cut
*/

void
Perl_sv_vsetpvf_mg(pTHX_ SV *const sv, const char *const pat, va_list *const args)
{
    PERL_ARGS_ASSERT_SV_VSETPVF_MG;

    sv_vsetpvfn(sv, pat, strlen(pat), args, NULL, 0, NULL);
    SvSETMAGIC(sv);
}

#if defined(PERL_IMPLICIT_CONTEXT)

/* pTHX_ magic can't cope with varargs, so this is a no-context
 * version of the main function, (which may itself be aliased to us).
 * Don't access this version directly.
 */

void
Perl_sv_catpvf_nocontext(SV *const sv, const char *const pat, ...)
{
    dTHX;
    va_list args;

    PERL_ARGS_ASSERT_SV_CATPVF_NOCONTEXT;

    va_start(args, pat);
    sv_vcatpvf(sv, pat, &args);
    va_end(args);
}

/* pTHX_ magic can't cope with varargs, so this is a no-context
 * version of the main function, (which may itself be aliased to us).
 * Don't access this version directly.
 */

void
Perl_sv_catpvf_mg_nocontext(SV *const sv, const char *const pat, ...)
{
    dTHX;
    va_list args;

    PERL_ARGS_ASSERT_SV_CATPVF_MG_NOCONTEXT;

    va_start(args, pat);
    sv_vcatpvf_mg(sv, pat, &args);
    va_end(args);
}
#endif

/*
=for apidoc sv_catpvf

Processes its arguments like C<sprintf> and appends the formatted
output to an SV.  If the appended data contains "wide" characters
(including, but not limited to, SVs with a UTF-8 PV formatted with %s,
and characters >255 formatted with %c), the original SV might get
upgraded to UTF-8.  Handles 'get' magic, but not 'set' magic.  See
C<sv_catpvf_mg>.  If the original SV was UTF-8, the pattern should be
valid UTF-8; if the original SV was bytes, the pattern should be too.

=cut */

void
Perl_sv_catpvf(pTHX_ SV *const sv, const char *const pat, ...)
{
    va_list args;

    PERL_ARGS_ASSERT_SV_CATPVF;

    va_start(args, pat);
    sv_vcatpvf(sv, pat, &args);
    va_end(args);
}

/*
=for apidoc sv_vcatpvf

Processes its arguments like C<vsprintf> and appends the formatted output
to an SV.  Does not handle 'set' magic.  See C<sv_vcatpvf_mg>.

Usually used via its frontend C<sv_catpvf>.

=cut
*/

void
Perl_sv_vcatpvf(pTHX_ SV *const sv, const char *const pat, va_list *const args)
{
    PERL_ARGS_ASSERT_SV_VCATPVF;

    sv_vcatpvfn(sv, pat, strlen(pat), args, NULL, 0, NULL);
}

/*
=for apidoc sv_catpvf_mg

Like C<sv_catpvf>, but also handles 'set' magic.

=cut
*/

void
Perl_sv_catpvf_mg(pTHX_ SV *const sv, const char *const pat, ...)
{
    va_list args;

    PERL_ARGS_ASSERT_SV_CATPVF_MG;

    va_start(args, pat);
    sv_vcatpvf_mg(sv, pat, &args);
    va_end(args);
}

/*
=for apidoc sv_vcatpvf_mg

Like C<sv_vcatpvf>, but also handles 'set' magic.

Usually used via its frontend C<sv_catpvf_mg>.

=cut
*/

void
Perl_sv_vcatpvf_mg(pTHX_ SV *const sv, const char *const pat, va_list *const args)
{
    PERL_ARGS_ASSERT_SV_VCATPVF_MG;

    sv_vcatpvfn(sv, pat, strlen(pat), args, NULL, 0, NULL);
    SvSETMAGIC(sv);
}

/*
=for apidoc sv_vsetpvfn

Works like C<sv_vcatpvfn> but copies the text into the SV instead of
appending it.

Usually used via one of its frontends C<sv_vsetpvf> and C<sv_vsetpvf_mg>.

=cut
*/

void
Perl_sv_vsetpvfn(pTHX_ SV *const sv, const char *const pat, const STRLEN patlen,
                 va_list *const args, SV **const svargs, const I32 svmax, bool *const maybe_tainted)
{
    PERL_ARGS_ASSERT_SV_VSETPVFN;

    sv_setpvs(sv, "");
    sv_vcatpvfn(sv, pat, patlen, args, svargs, svmax, maybe_tainted);
}


/*
 * Warn of missing argument to sprintf, and then return a defined value
 * to avoid inappropriate "use of uninit" warnings [perl #71000].
 */
#define WARN_MISSING WARN_UNINITIALIZED /* Not sure we want a new category */
STATIC SV*
S_vcatpvfn_missing_argument(pTHX) {
    if (ckWARN(WARN_MISSING)) {
	Perl_warner(aTHX_ packWARN(WARN_MISSING), "Missing argument in %s",
		PL_op ? OP_DESC(PL_op) : "sv_vcatpvfn()");
    }
    return &PL_sv_no;
}


STATIC I32
S_expect_number(pTHX_ char **const pattern)
{
    dVAR;
    I32 var = 0;

    PERL_ARGS_ASSERT_EXPECT_NUMBER;

    switch (**pattern) {
    case '1': case '2': case '3':
    case '4': case '5': case '6':
    case '7': case '8': case '9':
	var = *(*pattern)++ - '0';
	while (isDIGIT(**pattern)) {
	    const I32 tmp = var * 10 + (*(*pattern)++ - '0');
	    if (tmp < var)
		Perl_croak(aTHX_ "Integer overflow in format string for %s", (PL_op ? OP_DESC(PL_op) : "sv_vcatpvfn"));
	    var = tmp;
	}
    }
    return var;
}

STATIC char *
S_F0convert(NV nv, char *const endbuf, STRLEN *const len)
{
    const int neg = nv < 0;
    UV uv;

    PERL_ARGS_ASSERT_F0CONVERT;

    if (neg)
	nv = -nv;
    if (nv < UV_MAX) {
	char *p = endbuf;
	nv += 0.5;
	uv = (UV)nv;
	if (uv & 1 && uv == nv)
	    uv--;			/* Round to even */
	do {
	    const unsigned dig = uv % 10;
	    *--p = '0' + dig;
	} while (uv /= 10);
	if (neg)
	    *--p = '-';
	*len = endbuf - p;
	return p;
    }
    return NULL;
}


/*
=for apidoc sv_vcatpvfn

Processes its arguments like C<vsprintf> and appends the formatted output
to an SV.  Uses an array of SVs if the C style variable argument list is
missing (NULL).  When running with taint checks enabled, indicates via
C<maybe_tainted> if results are untrustworthy (often due to the use of
locales).

Usually used via one of its frontends C<sv_vcatpvf> and C<sv_vcatpvf_mg>.

=cut
*/


#define VECTORIZE_ARGS	vecsv = va_arg(*args, SV*);\
			vecstr = (U8*)SvPV_const(vecsv,veclen);\
			vec_utf8 = DO_UTF8(vecsv);

/* XXX maybe_tainted is never assigned to, so the doc above is lying. */

void
Perl_sv_vcatpvfn(pTHX_ SV *const sv, const char *const pat, const STRLEN patlen,
                 va_list *const args, SV **const svargs, const I32 svmax, bool *const maybe_tainted)
{
    dVAR;
    char *p;
    char *q;
    const char *patend;
    STRLEN origlen;
    I32 svix = 0;
    static const char nullstr[] = "(null)";
    SV *argsv = NULL;
    bool has_utf8 = DO_UTF8(sv);    /* has the result utf8? */
    const bool pat_utf8 = has_utf8; /* the pattern is in utf8? */
    SV *nsv = NULL;
    /* Times 4: a decimal digit takes more than 3 binary digits.
     * NV_DIG: mantissa takes than many decimal digits.
     * Plus 32: Playing safe. */
    char ebuf[IV_DIG * 4 + NV_DIG + 32];
    /* large enough for "%#.#f" --chip */
    /* what about long double NVs? --jhi */

    PERL_ARGS_ASSERT_SV_VCATPVFN;
    PERL_UNUSED_ARG(maybe_tainted);

    /* no matter what, this is a string now */
    (void)SvPV_force(sv, origlen);

    /* special-case "", "%s", and "%-p" (SVf - see below) */
    if (patlen == 0)
	return;
    if (patlen == 2 && pat[0] == '%' && pat[1] == 's') {
	if (args) {
	    const char * const s = va_arg(*args, char*);
	    sv_catpv(sv, s ? s : nullstr);
	}
	else if (svix < svmax) {
	    sv_catsv(sv, *svargs);
	}
	else
	    S_vcatpvfn_missing_argument(aTHX);
	return;
    }
    if (args && patlen == 3 && pat[0] == '%' &&
		pat[1] == '-' && pat[2] == 'p') {
	argsv = MUTABLE_SV(va_arg(*args, void*));
	sv_catsv(sv, argsv);
	return;
    }

#ifndef USE_LONG_DOUBLE
    /* special-case "%.<number>[gf]" */
    if ( !args && patlen <= 5 && pat[0] == '%' && pat[1] == '.'
	 && (pat[patlen-1] == 'g' || pat[patlen-1] == 'f') ) {
	unsigned digits = 0;
	const char *pp;

	pp = pat + 2;
	while (*pp >= '0' && *pp <= '9')
	    digits = 10 * digits + (*pp++ - '0');
	if (pp - pat == (int)patlen - 1 && svix < svmax) {
	    const NV nv = SvNV(*svargs);
	    if (*pp == 'g') {
		/* Add check for digits != 0 because it seems that some
		   gconverts are buggy in this case, and we don't yet have
		   a Configure test for this.  */
		if (digits && digits < sizeof(ebuf) - NV_DIG - 10) {
		     /* 0, point, slack */
		    Gconvert(nv, (int)digits, 0, ebuf);
		    sv_catpv(sv, ebuf);
		    if (*ebuf)	/* May return an empty string for digits==0 */
			return;
		}
	    } else if (!digits) {
		STRLEN l;

		if ((p = F0convert(nv, ebuf + sizeof ebuf, &l))) {
		    sv_catpvn(sv, p, l);
		    return;
		}
	    }
	}
    }
#endif /* !USE_LONG_DOUBLE */

    if (!args && svix < svmax && DO_UTF8(*svargs))
	has_utf8 = TRUE;

    patend = (char*)pat + patlen;
    for (p = (char*)pat; p < patend; p = q) {
	bool alt = FALSE;
	bool left = FALSE;
	bool vectorize = FALSE;
	bool vectorarg = FALSE;
	bool vec_utf8 = FALSE;
	char fill = ' ';
	char plus = 0;
	char intsize = 0;
	STRLEN width = 0;
	STRLEN zeros = 0;
	bool has_precis = FALSE;
	STRLEN precis = 0;
	const I32 osvix = svix;
	bool is_utf8 = FALSE;  /* is this item utf8?   */
#ifdef HAS_LDBL_SPRINTF_BUG
	/* This is to try to fix a bug with irix/nonstop-ux/powerux and
	   with sfio - Allen <allens@cpan.org> */
	bool fix_ldbl_sprintf_bug = FALSE;
#endif

	char esignbuf[4];
	U8 utf8buf[UTF8_MAXBYTES+1];
	STRLEN esignlen = 0;

	const char *eptr = NULL;
	const char *fmtstart;
	STRLEN elen = 0;
	SV *vecsv = NULL;
	const U8 *vecstr = NULL;
	STRLEN veclen = 0;
	char c = 0;
	int i;
	unsigned base = 0;
	IV iv = 0;
	UV uv = 0;
	/* we need a long double target in case HAS_LONG_DOUBLE but
	   not USE_LONG_DOUBLE
	*/
#if defined(HAS_LONG_DOUBLE) && LONG_DOUBLESIZE > DOUBLESIZE
	long double nv;
#else
	NV nv;
#endif
	STRLEN have;
	STRLEN need;
	STRLEN gap;
	const char *dotstr = ".";
	STRLEN dotstrlen = 1;
	I32 efix = 0; /* explicit format parameter index */
	I32 ewix = 0; /* explicit width index */
	I32 epix = 0; /* explicit precision index */
	I32 evix = 0; /* explicit vector index */
	bool asterisk = FALSE;

	/* echo everything up to the next format specification */
	for (q = p; q < patend && *q != '%'; ++q) ;
	if (q > p) {
	    if (has_utf8 && !pat_utf8)
		sv_catpvn_utf8_upgrade(sv, p, q - p, nsv);
	    else
		sv_catpvn(sv, p, q - p);
	    p = q;
	}
	if (q++ >= patend)
	    break;

	fmtstart = q;

/*
    We allow format specification elements in this order:
	\d+\$              explicit format parameter index
	[-+ 0#]+           flags
	v|\*(\d+\$)?v      vector with optional (optionally specified) arg
	0		   flag (as above): repeated to allow "v02" 	
	\d+|\*(\d+\$)?     width using optional (optionally specified) arg
	\.(\d*|\*(\d+\$)?) precision using optional (optionally specified) arg
	[hlqLV]            size
    [%bcdefginopsuxDFOUX] format (mandatory)
*/

	if (args) {
/*  
	As of perl5.9.3, printf format checking is on by default.
	Internally, perl uses %p formats to provide an escape to
	some extended formatting.  This block deals with those
	extensions: if it does not match, (char*)q is reset and
	the normal format processing code is used.

	Currently defined extensions are:
		%p		include pointer address (standard)	
		%-p	(SVf)	include an SV (previously %_)
		%-<num>p	include an SV with precision <num>	
		%2p		include a HEK
		%3p		include a HEK with precision of 256
		%<num>p		(where num != 2 or 3) reserved for future
				extensions

	Robin Barker 2005-07-14 (but modified since)

		%1p	(VDf)	removed.  RMB 2007-10-19
*/
 	    char* r = q; 
	    bool sv = FALSE;	
	    STRLEN n = 0;
	    if (*q == '-')
		sv = *q++;
	    n = expect_number(&q);
	    if (*q++ == 'p') {
		if (sv) {			/* SVf */
		    if (n) {
			precis = n;
			has_precis = TRUE;
		    }
		    argsv = MUTABLE_SV(va_arg(*args, void*));
		    eptr = SvPV_const(argsv, elen);
		    if (DO_UTF8(argsv))
			is_utf8 = TRUE;
		    goto string;
		}
		else if (n==2 || n==3) {	/* HEKf */
		    HEK * const hek = va_arg(*args, HEK *);
		    eptr = HEK_KEY(hek);
		    elen = HEK_LEN(hek);
		    if (HEK_UTF8(hek)) is_utf8 = TRUE;
		    if (n==3) precis = 256, has_precis = TRUE;
		    goto string;
		}
		else if (n) {
		    Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
				     "internal %%<num>p might conflict with future printf extensions");
		}
	    }
	    q = r; 
	}

	if ( (width = expect_number(&q)) ) {
	    if (*q == '$') {
		++q;
		efix = width;
	    } else {
		goto gotwidth;
	    }
	}

	/* FLAGS */

	while (*q) {
	    switch (*q) {
	    case ' ':
	    case '+':
		if (plus == '+' && *q == ' ') /* '+' over ' ' */
		    q++;
		else
		    plus = *q++;
		continue;

	    case '-':
		left = TRUE;
		q++;
		continue;

	    case '0':
		fill = *q++;
		continue;

	    case '#':
		alt = TRUE;
		q++;
		continue;

	    default:
		break;
	    }
	    break;
	}

      tryasterisk:
	if (*q == '*') {
	    q++;
	    if ( (ewix = expect_number(&q)) )
		if (*q++ != '$')
		    goto unknown;
	    asterisk = TRUE;
	}
	if (*q == 'v') {
	    q++;
	    if (vectorize)
		goto unknown;
	    if ((vectorarg = asterisk)) {
		evix = ewix;
		ewix = 0;
		asterisk = FALSE;
	    }
	    vectorize = TRUE;
	    goto tryasterisk;
	}

	if (!asterisk)
	{
	    if( *q == '0' )
		fill = *q++;
	    width = expect_number(&q);
	}

	if (vectorize && vectorarg) {
	    /* vectorizing, but not with the default "." */
	    if (args)
		vecsv = va_arg(*args, SV*);
	    else if (evix) {
		vecsv = (evix > 0 && evix <= svmax)
		    ? svargs[evix-1] : S_vcatpvfn_missing_argument(aTHX);
	    } else {
		vecsv = svix < svmax
		    ? svargs[svix++] : S_vcatpvfn_missing_argument(aTHX);
	    }
	    dotstr = SvPV_const(vecsv, dotstrlen);
	    /* Keep the DO_UTF8 test *after* the SvPV call, else things go
	       bad with tied or overloaded values that return UTF8.  */
	    if (DO_UTF8(vecsv))
		is_utf8 = TRUE;
	    else if (has_utf8) {
		vecsv = sv_mortalcopy(vecsv);
		sv_utf8_upgrade(vecsv);
		dotstr = SvPV_const(vecsv, dotstrlen);
		is_utf8 = TRUE;
	    }		    
	}

	if (asterisk) {
	    if (args)
		i = va_arg(*args, int);
	    else
		i = (ewix ? ewix <= svmax : svix < svmax) ?
		    SvIVx(svargs[ewix ? ewix-1 : svix++]) : 0;
	    left |= (i < 0);
	    width = (i < 0) ? -i : i;
	}
      gotwidth:

	/* PRECISION */

	if (*q == '.') {
	    q++;
	    if (*q == '*') {
		q++;
		if ( ((epix = expect_number(&q))) && (*q++ != '$') )
		    goto unknown;
		/* XXX: todo, support specified precision parameter */
		if (epix)
		    goto unknown;
		if (args)
		    i = va_arg(*args, int);
		else
		    i = (ewix ? ewix <= svmax : svix < svmax)
			? SvIVx(svargs[ewix ? ewix-1 : svix++]) : 0;
		precis = i;
		has_precis = !(i < 0);
	    }
	    else {
		precis = 0;
		while (isDIGIT(*q))
		    precis = precis * 10 + (*q++ - '0');
		has_precis = TRUE;
	    }
	}

	if (vectorize) {
	    if (args) {
		VECTORIZE_ARGS
	    }
	    else if (efix ? (efix > 0 && efix <= svmax) : svix < svmax) {
		vecsv = svargs[efix ? efix-1 : svix++];
		vecstr = (U8*)SvPV_const(vecsv,veclen);
		vec_utf8 = DO_UTF8(vecsv);

		/* if this is a version object, we need to convert
		 * back into v-string notation and then let the
		 * vectorize happen normally
		 */
		if (sv_isobject(vecsv) && sv_derived_from(vecsv, "version")) {
		    char *version = savesvpv(vecsv);
		    if ( hv_exists(MUTABLE_HV(SvRV(vecsv)), "alpha", 5 ) ) {
			Perl_warner(aTHX_ packWARN(WARN_INTERNAL),
			"vector argument not supported with alpha versions");
			goto unknown;
		    }
		    vecsv = sv_newmortal();
		    scan_vstring(version, version + veclen, vecsv);
		    vecstr = (U8*)SvPV_const(vecsv, veclen);
		    vec_utf8 = DO_UTF8(vecsv);
		    Safefree(version);
		}
	    }
	    else {
		vecstr = (U8*)"";
		veclen = 0;
	    }
	}

	/* SIZE */

	switch (*q) {
#ifdef WIN32
	case 'I':			/* Ix, I32x, and I64x */
#  ifdef WIN64
	    if (q[1] == '6' && q[2] == '4') {
		q += 3;
		intsize = 'q';
		break;
	    }
#  endif
	    if (q[1] == '3' && q[2] == '2') {
		q += 3;
		break;
	    }
#  ifdef WIN64
	    intsize = 'q';
#  endif
	    q++;
	    break;
#endif
#if defined(HAS_QUAD) || defined(HAS_LONG_DOUBLE)
	case 'L':			/* Ld */
	    /*FALLTHROUGH*/
#ifdef HAS_QUAD
	case 'q':			/* qd */
#endif
	    intsize = 'q';
	    q++;
	    break;
#endif
	case 'l':
	    ++q;
#if defined(HAS_QUAD) || defined(HAS_LONG_DOUBLE)
	    if (*q == 'l') {	/* lld, llf */
		intsize = 'q';
		++q;
	    }
	    else
#endif
		intsize = 'l';
	    break;
	case 'h':
	    if (*++q == 'h') {	/* hhd, hhu */
		intsize = 'c';
		++q;
	    }
	    else
		intsize = 'h';
	    break;
	case 'V':
	case 'z':
	case 't':
#if HAS_C99
        case 'j':
#endif
	    intsize = *q++;
	    break;
	}

	/* CONVERSION */

	if (*q == '%') {
	    eptr = q++;
	    elen = 1;
	    if (vectorize) {
		c = '%';
		goto unknown;
	    }
	    goto string;
	}

	if (!vectorize && !args) {
	    if (efix) {
		const I32 i = efix-1;
		argsv = (i >= 0 && i < svmax)
		    ? svargs[i] : S_vcatpvfn_missing_argument(aTHX);
	    } else {
		argsv = (svix >= 0 && svix < svmax)
		    ? svargs[svix++] : S_vcatpvfn_missing_argument(aTHX);
	    }
	}

	switch (c = *q++) {

	    /* STRINGS */

	case 'c':
	    if (vectorize)
		goto unknown;
	    uv = (args) ? va_arg(*args, int) : SvIV(argsv);
	    if ((uv > 255 ||
		 (!UNI_IS_INVARIANT(uv) && SvUTF8(sv)))
		&& !IN_BYTES) {
		eptr = (char*)utf8buf;
		elen = uvchr_to_utf8((U8*)eptr, uv) - utf8buf;
		is_utf8 = TRUE;
	    }
	    else {
		c = (char)uv;
		eptr = &c;
		elen = 1;
	    }
	    goto string;

	case 's':
	    if (vectorize)
		goto unknown;
	    if (args) {
		eptr = va_arg(*args, char*);
		if (eptr)
		    elen = strlen(eptr);
		else {
		    eptr = (char *)nullstr;
		    elen = sizeof nullstr - 1;
		}
	    }
	    else {
		eptr = SvPV_const(argsv, elen);
		if (DO_UTF8(argsv)) {
		    STRLEN old_precis = precis;
		    if (has_precis && precis < elen) {
			STRLEN ulen = sv_len_utf8(argsv);
			I32 p = precis > ulen ? ulen : precis;
			sv_pos_u2b(argsv, &p, 0); /* sticks at end */
			precis = p;
		    }
		    if (width) { /* fudge width (can't fudge elen) */
			if (has_precis && precis < elen)
			    width += precis - old_precis;
			else
			    width += elen - sv_len_utf8(argsv);
		    }
		    is_utf8 = TRUE;
		}
	    }

	string:
	    if (has_precis && precis < elen)
		elen = precis;
	    break;

	    /* INTEGERS */

	case 'p':
	    if (alt || vectorize)
		goto unknown;
	    uv = PTR2UV(args ? va_arg(*args, void*) : argsv);
	    base = 16;
	    goto integer;

	case 'D':
#ifdef IV_IS_QUAD
	    intsize = 'q';
#else
	    intsize = 'l';
#endif
	    /*FALLTHROUGH*/
	case 'd':
	case 'i':
#if vdNUMBER
	format_vd:
#endif
	    if (vectorize) {
		STRLEN ulen;
		if (!veclen)
		    continue;
		if (vec_utf8)
		    uv = utf8n_to_uvchr(vecstr, veclen, &ulen,
					UTF8_ALLOW_ANYUV);
		else {
		    uv = *vecstr;
		    ulen = 1;
		}
		vecstr += ulen;
		veclen -= ulen;
		if (plus)
		     esignbuf[esignlen++] = plus;
	    }
	    else if (args) {
		switch (intsize) {
		case 'c':	iv = (char)va_arg(*args, int); break;
		case 'h':	iv = (short)va_arg(*args, int); break;
		case 'l':	iv = va_arg(*args, long); break;
		case 'V':	iv = va_arg(*args, IV); break;
		case 'z':	iv = va_arg(*args, SSize_t); break;
		case 't':	iv = va_arg(*args, ptrdiff_t); break;
		default:	iv = va_arg(*args, int); break;
#if HAS_C99
		case 'j':	iv = va_arg(*args, intmax_t); break;
#endif
		case 'q':
#ifdef HAS_QUAD
				iv = va_arg(*args, Quad_t); break;
#else
				goto unknown;
#endif
		}
	    }
	    else {
		IV tiv = SvIV(argsv); /* work around GCC bug #13488 */
		switch (intsize) {
		case 'c':	iv = (char)tiv; break;
		case 'h':	iv = (short)tiv; break;
		case 'l':	iv = (long)tiv; break;
		case 'V':
		default:	iv = tiv; break;
		case 'q':
#ifdef HAS_QUAD
				iv = (Quad_t)tiv; break;
#else
				goto unknown;
#endif
		}
	    }
	    if ( !vectorize )	/* we already set uv above */
	    {
		if (iv >= 0) {
		    uv = iv;
		    if (plus)
			esignbuf[esignlen++] = plus;
		}
		else {
		    uv = -iv;
		    esignbuf[esignlen++] = '-';
		}
	    }
	    base = 10;
	    goto integer;

	case 'U':
#ifdef IV_IS_QUAD
	    intsize = 'q';
#else
	    intsize = 'l';
#endif
	    /*FALLTHROUGH*/
	case 'u':
	    base = 10;
	    goto uns_integer;

	case 'B':
	case 'b':
	    base = 2;
	    goto uns_integer;

	case 'O':
#ifdef IV_IS_QUAD
	    intsize = 'q';
#else
	    intsize = 'l';
#endif
	    /*FALLTHROUGH*/
	case 'o':
	    base = 8;
	    goto uns_integer;

	case 'X':
	case 'x':
	    base = 16;

	uns_integer:
	    if (vectorize) {
		STRLEN ulen;
	vector:
		if (!veclen)
		    continue;
		if (vec_utf8)
		    uv = utf8n_to_uvchr(vecstr, veclen, &ulen,
					UTF8_ALLOW_ANYUV);
		else {
		    uv = *vecstr;
		    ulen = 1;
		}
		vecstr += ulen;
		veclen -= ulen;
	    }
	    else if (args) {
		switch (intsize) {
		case 'c':  uv = (unsigned char)va_arg(*args, unsigned); break;
		case 'h':  uv = (unsigned short)va_arg(*args, unsigned); break;
		case 'l':  uv = va_arg(*args, unsigned long); break;
		case 'V':  uv = va_arg(*args, UV); break;
		case 'z':  uv = va_arg(*args, Size_t); break;
	        case 't':  uv = va_arg(*args, ptrdiff_t); break; /* will sign extend, but there is no uptrdiff_t, so oh well */
#if HAS_C99
		case 'j':  uv = va_arg(*args, uintmax_t); break;
#endif
		default:   uv = va_arg(*args, unsigned); break;
		case 'q':
#ifdef HAS_QUAD
			   uv = va_arg(*args, Uquad_t); break;
#else
			   goto unknown;
#endif
		}
	    }
	    else {
		UV tuv = SvUV(argsv); /* work around GCC bug #13488 */
		switch (intsize) {
		case 'c':	uv = (unsigned char)tuv; break;
		case 'h':	uv = (unsigned short)tuv; break;
		case 'l':	uv = (unsigned long)tuv; break;
		case 'V':
		default:	uv = tuv; break;
		case 'q':
#ifdef HAS_QUAD
				uv = (Uquad_t)tuv; break;
#else
				goto unknown;
#endif
		}
	    }

	integer:
	    {
		char *ptr = ebuf + sizeof ebuf;
		bool tempalt = uv ? alt : FALSE; /* Vectors can't change alt */
		zeros = 0;

		switch (base) {
		    unsigned dig;
		case 16:
		    p = (char *)((c == 'X') ? PL_hexdigit + 16 : PL_hexdigit);
		    do {
			dig = uv & 15;
			*--ptr = p[dig];
		    } while (uv >>= 4);
		    if (tempalt) {
			esignbuf[esignlen++] = '0';
			esignbuf[esignlen++] = c;  /* 'x' or 'X' */
		    }
		    break;
		case 8:
		    do {
			dig = uv & 7;
			*--ptr = '0' + dig;
		    } while (uv >>= 3);
		    if (alt && *ptr != '0')
			*--ptr = '0';
		    break;
		case 2:
		    do {
			dig = uv & 1;
			*--ptr = '0' + dig;
		    } while (uv >>= 1);
		    if (tempalt) {
			esignbuf[esignlen++] = '0';
			esignbuf[esignlen++] = c;
		    }
		    break;
		default:		/* it had better be ten or less */
		    do {
			dig = uv % base;
			*--ptr = '0' + dig;
		    } while (uv /= base);
		    break;
		}
		elen = (ebuf + sizeof ebuf) - ptr;
		eptr = ptr;
		if (has_precis) {
		    if (precis > elen)
			zeros = precis - elen;
		    else if (precis == 0 && elen == 1 && *eptr == '0'
			     && !(base == 8 && alt)) /* "%#.0o" prints "0" */
			elen = 0;

		/* a precision nullifies the 0 flag. */
		    if (fill == '0')
			fill = ' ';
		}
	    }
	    break;

	    /* FLOATING POINT */

	case 'F':
	    c = 'f';		/* maybe %F isn't supported here */
	    /*FALLTHROUGH*/
	case 'e': case 'E':
	case 'f':
	case 'g': case 'G':
	    if (vectorize)
		goto unknown;

	    /* This is evil, but floating point is even more evil */

	    /* for SV-style calling, we can only get NV
	       for C-style calling, we assume %f is double;
	       for simplicity we allow any of %Lf, %llf, %qf for long double
	    */
	    switch (intsize) {
	    case 'V':
#if defined(USE_LONG_DOUBLE)
		intsize = 'q';
#endif
		break;
/* [perl #20339] - we should accept and ignore %lf rather than die */
	    case 'l':
		/*FALLTHROUGH*/
	    default:
#if defined(USE_LONG_DOUBLE)
		intsize = args ? 0 : 'q';
#endif
		break;
	    case 'q':
#if defined(HAS_LONG_DOUBLE)
		break;
#else
		/*FALLTHROUGH*/
#endif
	    case 'c':
	    case 'h':
	    case 'z':
	    case 't':
	    case 'j':
		goto unknown;
	    }

	    /* now we need (long double) if intsize == 'q', else (double) */
	    nv = (args) ?
#if LONG_DOUBLESIZE > DOUBLESIZE
		intsize == 'q' ?
		    va_arg(*args, long double) :
		    va_arg(*args, double)
#else
		    va_arg(*args, double)
#endif
		: SvNV(argsv);

	    need = 0;
	    /* nv * 0 will be NaN for NaN, +Inf and -Inf, and 0 for anything
	       else. frexp() has some unspecified behaviour for those three */
	    if (c != 'e' && c != 'E' && (nv * 0) == 0) {
		i = PERL_INT_MIN;
		/* FIXME: if HAS_LONG_DOUBLE but not USE_LONG_DOUBLE this
		   will cast our (long double) to (double) */
		(void)Perl_frexp(nv, &i);
		if (i == PERL_INT_MIN)
		    Perl_die(aTHX_ "panic: frexp");
		if (i > 0)
		    need = BIT_DIGITS(i);
	    }
	    need += has_precis ? precis : 6; /* known default */

	    if (need < width)
		need = width;

#ifdef HAS_LDBL_SPRINTF_BUG
	    /* This is to try to fix a bug with irix/nonstop-ux/powerux and
	       with sfio - Allen <allens@cpan.org> */

#  ifdef DBL_MAX
#    define MY_DBL_MAX DBL_MAX
#  else /* XXX guessing! HUGE_VAL may be defined as infinity, so not using */
#    if DOUBLESIZE >= 8
#      define MY_DBL_MAX 1.7976931348623157E+308L
#    else
#      define MY_DBL_MAX 3.40282347E+38L
#    endif
#  endif

#  ifdef HAS_LDBL_SPRINTF_BUG_LESS1 /* only between -1L & 1L - Allen */
#    define MY_DBL_MAX_BUG 1L
#  else
#    define MY_DBL_MAX_BUG MY_DBL_MAX
#  endif

#  ifdef DBL_MIN
#    define MY_DBL_MIN DBL_MIN
#  else  /* XXX guessing! -Allen */
#    if DOUBLESIZE >= 8
#      define MY_DBL_MIN 2.2250738585072014E-308L
#    else
#      define MY_DBL_MIN 1.17549435E-38L
#    endif
#  endif

	    if ((intsize == 'q') && (c == 'f') &&
		((nv < MY_DBL_MAX_BUG) && (nv > -MY_DBL_MAX_BUG)) &&
		(need < DBL_DIG)) {
		/* it's going to be short enough that
		 * long double precision is not needed */

		if ((nv <= 0L) && (nv >= -0L))
		    fix_ldbl_sprintf_bug = TRUE; /* 0 is 0 - easiest */
		else {
		    /* would use Perl_fp_class as a double-check but not
		     * functional on IRIX - see perl.h comments */

		    if ((nv >= MY_DBL_MIN) || (nv <= -MY_DBL_MIN)) {
			/* It's within the range that a double can represent */
#if defined(DBL_MAX) && !defined(DBL_MIN)
			if ((nv >= ((long double)1/DBL_MAX)) ||
			    (nv <= (-(long double)1/DBL_MAX)))
#endif
			fix_ldbl_sprintf_bug = TRUE;
		    }
		}
		if (fix_ldbl_sprintf_bug == TRUE) {
		    double temp;

		    intsize = 0;
		    temp = (double)nv;
		    nv = (NV)temp;
		}
	    }

#  undef MY_DBL_MAX
#  undef MY_DBL_MAX_BUG
#  undef MY_DBL_MIN

#endif /* HAS_LDBL_SPRINTF_BUG */

	    need += 20; /* fudge factor */
	    if (PL_efloatsize < need) {
		Safefree(PL_efloatbuf);
		PL_efloatsize = need + 20; /* more fudge */
		Newx(PL_efloatbuf, PL_efloatsize, char);
		PL_efloatbuf[0] = '\0';
	    }

	    if ( !(width || left || plus || alt) && fill != '0'
		 && has_precis && intsize != 'q' ) {	/* Shortcuts */
		/* See earlier comment about buggy Gconvert when digits,
		   aka precis is 0  */
		if ( c == 'g' && precis) {
		    Gconvert((NV)nv, (int)precis, 0, PL_efloatbuf);
		    /* May return an empty string for digits==0 */
		    if (*PL_efloatbuf) {
			elen = strlen(PL_efloatbuf);
			goto float_converted;
		    }
		} else if ( c == 'f' && !precis) {
		    if ((eptr = F0convert(nv, ebuf + sizeof ebuf, &elen)))
			break;
		}
	    }
	    {
		char *ptr = ebuf + sizeof ebuf;
		*--ptr = '\0';
		*--ptr = c;
		/* FIXME: what to do if HAS_LONG_DOUBLE but not PERL_PRIfldbl? */
#if defined(HAS_LONG_DOUBLE) && defined(PERL_PRIfldbl)
		if (intsize == 'q') {
		    /* Copy the one or more characters in a long double
		     * format before the 'base' ([efgEFG]) character to
		     * the format string. */
		    static char const prifldbl[] = PERL_PRIfldbl;
		    char const *p = prifldbl + sizeof(prifldbl) - 3;
		    while (p >= prifldbl) { *--ptr = *p--; }
		}
#endif
		if (has_precis) {
		    base = precis;
		    do { *--ptr = '0' + (base % 10); } while (base /= 10);
		    *--ptr = '.';
		}
		if (width) {
		    base = width;
		    do { *--ptr = '0' + (base % 10); } while (base /= 10);
		}
		if (fill == '0')
		    *--ptr = fill;
		if (left)
		    *--ptr = '-';
		if (plus)
		    *--ptr = plus;
		if (alt)
		    *--ptr = '#';
		*--ptr = '%';

		/* No taint.  Otherwise we are in the strange situation
		 * where printf() taints but print($float) doesn't.
		 * --jhi */
#if defined(HAS_LONG_DOUBLE)
		elen = ((intsize == 'q')
			? my_snprintf(PL_efloatbuf, PL_efloatsize, ptr, nv)
			: my_snprintf(PL_efloatbuf, PL_efloatsize, ptr, (double)nv));
#else
		elen = my_sprintf(PL_efloatbuf, ptr, nv);
#endif
	    }
	float_converted:
	    eptr = PL_efloatbuf;
	    break;

	    /* SPECIAL */

	case 'n':
	    if (vectorize)
		goto unknown;
	    i = SvCUR(sv) - origlen;
	    if (args) {
		switch (intsize) {
		case 'c':	*(va_arg(*args, char*)) = i; break;
		case 'h':	*(va_arg(*args, short*)) = i; break;
		default:	*(va_arg(*args, int*)) = i; break;
		case 'l':	*(va_arg(*args, long*)) = i; break;
		case 'V':	*(va_arg(*args, IV*)) = i; break;
		case 'z':	*(va_arg(*args, SSize_t*)) = i; break;
		case 't':	*(va_arg(*args, ptrdiff_t*)) = i; break;
#if HAS_C99
		case 'j':	*(va_arg(*args, intmax_t*)) = i; break;
#endif
		case 'q':
#ifdef HAS_QUAD
				*(va_arg(*args, Quad_t*)) = i; break;
#else
				goto unknown;
#endif
		}
	    }
	    else
		sv_setuv_mg(argsv, has_utf8 ? (UV)sv_len_utf8(sv) : (UV)i);
	    continue;	/* not "break" */

	    /* UNKNOWN */

	default:
      unknown:
	    if (!args
		&& (PL_op->op_type == OP_PRTF || PL_op->op_type == OP_SPRINTF)
		&& ckWARN(WARN_PRINTF))
	    {
		SV * const msg = sv_newmortal();
		Perl_sv_setpvf(aTHX_ msg, "Invalid conversion in %sprintf: ",
			  (PL_op->op_type == OP_PRTF) ? "" : "s");
		if (fmtstart < patend) {
		    const char * const fmtend = q < patend ? q : patend;
		    const char * f;
		    sv_catpvs(msg, "\"%");
		    for (f = fmtstart; f < fmtend; f++) {
			if (isPRINT(*f)) {
			    sv_catpvn(msg, f, 1);
			} else {
			    Perl_sv_catpvf(aTHX_ msg,
					   "\\%03"UVof, (UV)*f & 0xFF);
			}
		    }
		    sv_catpvs(msg, "\"");
		} else {
		    sv_catpvs(msg, "end of string");
		}
		Perl_warner(aTHX_ packWARN(WARN_PRINTF), "%"SVf, SVfARG(msg)); /* yes, this is reentrant */
	    }

	    /* output mangled stuff ... */
	    if (c == '\0')
		--q;
	    eptr = p;
	    elen = q - p;

	    /* ... right here, because formatting flags should not apply */
	    SvGROW(sv, SvCUR(sv) + elen + 1);
	    p = SvEND(sv);
	    Copy(eptr, p, elen, char);
	    p += elen;
	    *p = '\0';
	    SvCUR_set(sv, p - SvPVX_const(sv));
	    svix = osvix;
	    continue;	/* not "break" */
	}

	if (is_utf8 != has_utf8) {
	    if (is_utf8) {
		if (SvCUR(sv))
		    sv_utf8_upgrade(sv);
	    }
	    else {
		const STRLEN old_elen = elen;
		SV * const nsv = newSVpvn_flags(eptr, elen, SVs_TEMP);
		sv_utf8_upgrade(nsv);
		eptr = SvPVX_const(nsv);
		elen = SvCUR(nsv);

		if (width) { /* fudge width (can't fudge elen) */
		    width += elen - old_elen;
		}
		is_utf8 = TRUE;
	    }
	}

	have = esignlen + zeros + elen;
	if (have < zeros)
	    Perl_croak_nocontext("%s", PL_memory_wrap);

	need = (have > width ? have : width);
	gap = need - have;

	if (need >= (((STRLEN)~0) - SvCUR(sv) - dotstrlen - 1))
	    Perl_croak_nocontext("%s", PL_memory_wrap);
	SvGROW(sv, SvCUR(sv) + need + dotstrlen + 1);
	p = SvEND(sv);
	if (esignlen && fill == '0') {
	    int i;
	    for (i = 0; i < (int)esignlen; i++)
		*p++ = esignbuf[i];
	}
	if (gap && !left) {
	    memset(p, fill, gap);
	    p += gap;
	}
	if (esignlen && fill != '0') {
	    int i;
	    for (i = 0; i < (int)esignlen; i++)
		*p++ = esignbuf[i];
	}
	if (zeros) {
	    int i;
	    for (i = zeros; i; i--)
		*p++ = '0';
	}
	if (elen) {
	    Copy(eptr, p, elen, char);
	    p += elen;
	}
	if (gap && left) {
	    memset(p, ' ', gap);
	    p += gap;
	}
	if (vectorize) {
	    if (veclen) {
		Copy(dotstr, p, dotstrlen, char);
		p += dotstrlen;
	    }
	    else
		vectorize = FALSE;		/* done iterating over vecstr */
	}
	if (is_utf8)
	    has_utf8 = TRUE;
	if (has_utf8)
	    SvUTF8_on(sv);
	*p = '\0';
	SvCUR_set(sv, p - SvPVX_const(sv));
	if (vectorize) {
	    esignlen = 0;
	    goto vector;
	}
    }
    SvTAINT(sv);
}

/* =========================================================================

=head1 Cloning an interpreter

All the macros and functions in this section are for the private use of
the main function, perl_clone().

The foo_dup() functions make an exact copy of an existing foo thingy.
During the course of a cloning, a hash table is used to map old addresses
to new addresses.  The table is created and manipulated with the
ptr_table_* functions.

=cut

 * =========================================================================*/


#if defined(USE_ITHREADS)

/* XXX Remove this so it doesn't have to go thru the macro and return for nothing */
#ifndef GpREFCNT_inc
#  define GpREFCNT_inc(gp)	((gp) ? (++(gp)->gp_refcnt, (gp)) : (GP*)NULL)
#endif


/* Certain cases in Perl_ss_dup have been merged, by relying on the fact
   that currently av_dup, gv_dup and hv_dup are the same as sv_dup.
   If this changes, please unmerge ss_dup.
   Likewise, sv_dup_inc_multiple() relies on this fact.  */
#define sv_dup_inc_NN(s,t)	SvREFCNT_inc_NN(sv_dup_inc(s,t))
#define av_dup(s,t)	MUTABLE_AV(sv_dup((const SV *)s,t))
#define av_dup_inc(s,t)	MUTABLE_AV(sv_dup_inc((const SV *)s,t))
#define hv_dup(s,t)	MUTABLE_HV(sv_dup((const SV *)s,t))
#define hv_dup_inc(s,t)	MUTABLE_HV(sv_dup_inc((const SV *)s,t))
#define cv_dup(s,t)	MUTABLE_CV(sv_dup((const SV *)s,t))
#define cv_dup_inc(s,t)	MUTABLE_CV(sv_dup_inc((const SV *)s,t))
#define io_dup(s,t)	MUTABLE_IO(sv_dup((const SV *)s,t))
#define io_dup_inc(s,t)	MUTABLE_IO(sv_dup_inc((const SV *)s,t))
#define gv_dup(s,t)	MUTABLE_GV(sv_dup((const SV *)s,t))
#define gv_dup_inc(s,t)	MUTABLE_GV(sv_dup_inc((const SV *)s,t))
#define SAVEPV(p)	((p) ? savepv(p) : NULL)
#define SAVEPVN(p,n)	((p) ? savepvn(p,n) : NULL)

/* clone a parser */

yy_parser *
Perl_parser_dup(pTHX_ const yy_parser *const proto, CLONE_PARAMS *const param)
{
    yy_parser *parser;

    PERL_ARGS_ASSERT_PARSER_DUP;

    if (!proto)
	return NULL;

    /* look for it in the table first */
    parser = (yy_parser *)ptr_table_fetch(PL_ptr_table, proto);
    if (parser)
	return parser;

    /* create anew and remember what it is */
    Newxz(parser, 1, yy_parser);
    ptr_table_store(PL_ptr_table, proto, parser);

    /* XXX these not yet duped */
    parser->old_parser = NULL;
    parser->stack = NULL;
    parser->ps = NULL;
    parser->stack_size = 0;
    /* XXX parser->stack->state = 0; */

    /* XXX eventually, just Copy() most of the parser struct ? */

    parser->lex_brackets = proto->lex_brackets;
    parser->lex_casemods = proto->lex_casemods;
    parser->lex_brackstack = savepvn(proto->lex_brackstack,
		    (proto->lex_brackets < 120 ? 120 : proto->lex_brackets));
    parser->lex_casestack = savepvn(proto->lex_casestack,
		    (proto->lex_casemods < 12 ? 12 : proto->lex_casemods));
    parser->lex_defer	= proto->lex_defer;
    parser->lex_dojoin	= proto->lex_dojoin;
    parser->lex_expect	= proto->lex_expect;
    parser->lex_formbrack = proto->lex_formbrack;
    parser->lex_inpat	= proto->lex_inpat;
    parser->lex_inwhat	= proto->lex_inwhat;
    parser->lex_op	= proto->lex_op;
    parser->lex_repl	= sv_dup_inc(proto->lex_repl, param);
    parser->lex_starts	= proto->lex_starts;
    parser->lex_stuff	= sv_dup_inc(proto->lex_stuff, param);
    parser->multi_close	= proto->multi_close;
    parser->multi_open	= proto->multi_open;
    parser->multi_start	= proto->multi_start;
    parser->multi_end	= proto->multi_end;
    parser->pending_ident = proto->pending_ident;
    parser->preambled	= proto->preambled;
    parser->sublex_info	= proto->sublex_info; /* XXX not quite right */
    parser->linestr	= sv_dup_inc(proto->linestr, param);
    parser->expect	= proto->expect;
    parser->copline	= proto->copline;
    parser->last_lop_op	= proto->last_lop_op;
    parser->lex_state	= proto->lex_state;
    parser->rsfp	= fp_dup(proto->rsfp, '<', param);
    /* rsfp_filters entries have fake IoDIRP() */
    parser->rsfp_filters= av_dup_inc(proto->rsfp_filters, param);
    parser->in_my	= proto->in_my;
    parser->in_my_stash	= hv_dup(proto->in_my_stash, param);
    parser->error_count	= proto->error_count;


    parser->linestr	= sv_dup_inc(proto->linestr, param);

    {
	char * const ols = SvPVX(proto->linestr);
	char * const ls  = SvPVX(parser->linestr);

	parser->bufptr	    = ls + (proto->bufptr >= ols ?
				    proto->bufptr -  ols : 0);
	parser->oldbufptr   = ls + (proto->oldbufptr >= ols ?
				    proto->oldbufptr -  ols : 0);
	parser->oldoldbufptr= ls + (proto->oldoldbufptr >= ols ?
				    proto->oldoldbufptr -  ols : 0);
	parser->linestart   = ls + (proto->linestart >= ols ?
				    proto->linestart -  ols : 0);
	parser->last_uni    = ls + (proto->last_uni >= ols ?
				    proto->last_uni -  ols : 0);
	parser->last_lop    = ls + (proto->last_lop >= ols ?
				    proto->last_lop -  ols : 0);

	parser->bufend	    = ls + SvCUR(parser->linestr);
    }

    Copy(proto->tokenbuf, parser->tokenbuf, 256, char);


#ifdef PERL_MAD
    parser->endwhite	= proto->endwhite;
    parser->faketokens	= proto->faketokens;
    parser->lasttoke	= proto->lasttoke;
    parser->nextwhite	= proto->nextwhite;
    parser->realtokenstart = proto->realtokenstart;
    parser->skipwhite	= proto->skipwhite;
    parser->thisclose	= proto->thisclose;
    parser->thismad	= proto->thismad;
    parser->thisopen	= proto->thisopen;
    parser->thisstuff	= proto->thisstuff;
    parser->thistoken	= proto->thistoken;
    parser->thiswhite	= proto->thiswhite;

    Copy(proto->nexttoke, parser->nexttoke, 5, NEXTTOKE);
    parser->curforce	= proto->curforce;
#else
    Copy(proto->nextval, parser->nextval, 5, YYSTYPE);
    Copy(proto->nexttype, parser->nexttype, 5,	I32);
    parser->nexttoke	= proto->nexttoke;
#endif

    /* XXX should clone saved_curcop here, but we aren't passed
     * proto_perl; so do it in perl_clone_using instead */

    return parser;
}


/* duplicate a file handle */

PerlIO *
Perl_fp_dup(pTHX_ PerlIO *const fp, const char type, CLONE_PARAMS *const param)
{
    PerlIO *ret;

    PERL_ARGS_ASSERT_FP_DUP;
    PERL_UNUSED_ARG(type);

    if (!fp)
	return (PerlIO*)NULL;

    /* look for it in the table first */
    ret = (PerlIO*)ptr_table_fetch(PL_ptr_table, fp);
    if (ret)
	return ret;

    /* create anew and remember what it is */
    ret = PerlIO_fdupopen(aTHX_ fp, param, PERLIO_DUP_CLONE);
    ptr_table_store(PL_ptr_table, fp, ret);
    return ret;
}

/* duplicate a directory handle */

DIR *
Perl_dirp_dup(pTHX_ DIR *const dp, CLONE_PARAMS *const param)
{
    DIR *ret;

#ifdef HAS_FCHDIR
    DIR *pwd;
    register const Direntry_t *dirent;
    char smallbuf[256];
    char *name = NULL;
    STRLEN len = 0;
    long pos;
#endif

    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_DIRP_DUP;

    if (!dp)
	return (DIR*)NULL;

    /* look for it in the table first */
    ret = (DIR*)ptr_table_fetch(PL_ptr_table, dp);
    if (ret)
	return ret;

#ifdef HAS_FCHDIR

    PERL_UNUSED_ARG(param);

    /* create anew */

    /* open the current directory (so we can switch back) */
    if (!(pwd = PerlDir_open("."))) return (DIR *)NULL;

    /* chdir to our dir handle and open the present working directory */
    if (fchdir(my_dirfd(dp)) < 0 || !(ret = PerlDir_open("."))) {
	PerlDir_close(pwd);
	return (DIR *)NULL;
    }
    /* Now we should have two dir handles pointing to the same dir. */

    /* Be nice to the calling code and chdir back to where we were. */
    fchdir(my_dirfd(pwd)); /* If this fails, then what? */

    /* We have no need of the pwd handle any more. */
    PerlDir_close(pwd);

#ifdef DIRNAMLEN
# define d_namlen(d) (d)->d_namlen
#else
# define d_namlen(d) strlen((d)->d_name)
#endif
    /* Iterate once through dp, to get the file name at the current posi-
       tion. Then step back. */
    pos = PerlDir_tell(dp);
    if ((dirent = PerlDir_read(dp))) {
	len = d_namlen(dirent);
	if (len <= sizeof smallbuf) name = smallbuf;
	else Newx(name, len, char);
	Move(dirent->d_name, name, len, char);
    }
    PerlDir_seek(dp, pos);

    /* Iterate through the new dir handle, till we find a file with the
       right name. */
    if (!dirent) /* just before the end */
	for(;;) {
	    pos = PerlDir_tell(ret);
	    if (PerlDir_read(ret)) continue; /* not there yet */
	    PerlDir_seek(ret, pos); /* step back */
	    break;
	}
    else {
	const long pos0 = PerlDir_tell(ret);
	for(;;) {
	    pos = PerlDir_tell(ret);
	    if ((dirent = PerlDir_read(ret))) {
		if (len == d_namlen(dirent)
		 && memEQ(name, dirent->d_name, len)) {
		    /* found it */
		    PerlDir_seek(ret, pos); /* step back */
		    break;
		}
		/* else we are not there yet; keep iterating */
	    }
	    else { /* This is not meant to happen. The best we can do is
	              reset the iterator to the beginning. */
		PerlDir_seek(ret, pos0);
		break;
	    }
	}
    }
#undef d_namlen

    if (name && name != smallbuf)
	Safefree(name);
#endif

#ifdef WIN32
    ret = win32_dirp_dup(dp, param);
#endif

    /* pop it in the pointer table */
    if (ret)
	ptr_table_store(PL_ptr_table, dp, ret);

    return ret;
}

/* duplicate a typeglob */

GP *
Perl_gp_dup(pTHX_ GP *const gp, CLONE_PARAMS *const param)
{
    GP *ret;

    PERL_ARGS_ASSERT_GP_DUP;

    if (!gp)
	return (GP*)NULL;
    /* look for it in the table first */
    ret = (GP*)ptr_table_fetch(PL_ptr_table, gp);
    if (ret)
	return ret;

    /* create anew and remember what it is */
    Newxz(ret, 1, GP);
    ptr_table_store(PL_ptr_table, gp, ret);

    /* clone */
    /* ret->gp_refcnt must be 0 before any other dups are called. We're relying
       on Newxz() to do this for us.  */
    ret->gp_sv		= sv_dup_inc(gp->gp_sv, param);
    ret->gp_io		= io_dup_inc(gp->gp_io, param);
    ret->gp_form	= cv_dup_inc(gp->gp_form, param);
    ret->gp_av		= av_dup_inc(gp->gp_av, param);
    ret->gp_hv		= hv_dup_inc(gp->gp_hv, param);
    ret->gp_egv	= gv_dup(gp->gp_egv, param);/* GvEGV is not refcounted */
    ret->gp_cv		= cv_dup_inc(gp->gp_cv, param);
    ret->gp_cvgen	= gp->gp_cvgen;
    ret->gp_line	= gp->gp_line;
    ret->gp_file_hek	= hek_dup(gp->gp_file_hek, param);
    return ret;
}

/* duplicate a chain of magic */

MAGIC *
Perl_mg_dup(pTHX_ MAGIC *mg, CLONE_PARAMS *const param)
{
    MAGIC *mgret = NULL;
    MAGIC **mgprev_p = &mgret;

    PERL_ARGS_ASSERT_MG_DUP;

    for (; mg; mg = mg->mg_moremagic) {
	MAGIC *nmg;

	if ((param->flags & CLONEf_JOIN_IN)
		&& mg->mg_type == PERL_MAGIC_backref)
	    /* when joining, we let the individual SVs add themselves to
	     * backref as needed. */
	    continue;

	Newx(nmg, 1, MAGIC);
	*mgprev_p = nmg;
	mgprev_p = &(nmg->mg_moremagic);

	/* There was a comment "XXX copy dynamic vtable?" but as we don't have
	   dynamic vtables, I'm not sure why Sarathy wrote it. The comment dates
	   from the original commit adding Perl_mg_dup() - revision 4538.
	   Similarly there is the annotation "XXX random ptr?" next to the
	   assignment to nmg->mg_ptr.  */
	*nmg = *mg;

	/* FIXME for plugins
	if (nmg->mg_type == PERL_MAGIC_qr) {
	    nmg->mg_obj	= MUTABLE_SV(CALLREGDUPE((REGEXP*)nmg->mg_obj, param));
	}
	else
	*/
	nmg->mg_obj = (nmg->mg_flags & MGf_REFCOUNTED)
			  ? nmg->mg_type == PERL_MAGIC_backref
				/* The backref AV has its reference
				 * count deliberately bumped by 1 */
				? SvREFCNT_inc(av_dup_inc((const AV *)
						    nmg->mg_obj, param))
				: sv_dup_inc(nmg->mg_obj, param)
			  : sv_dup(nmg->mg_obj, param);

	if (nmg->mg_ptr && nmg->mg_type != PERL_MAGIC_regex_global) {
	    if (nmg->mg_len > 0) {
		nmg->mg_ptr	= SAVEPVN(nmg->mg_ptr, nmg->mg_len);
		if (nmg->mg_type == PERL_MAGIC_overload_table &&
			AMT_AMAGIC((AMT*)nmg->mg_ptr))
		{
		    AMT * const namtp = (AMT*)nmg->mg_ptr;
		    sv_dup_inc_multiple((SV**)(namtp->table),
					(SV**)(namtp->table), NofAMmeth, param);
		}
	    }
	    else if (nmg->mg_len == HEf_SVKEY)
		nmg->mg_ptr = (char*)sv_dup_inc((const SV *)nmg->mg_ptr, param);
	}
	if ((nmg->mg_flags & MGf_DUP) && nmg->mg_virtual && nmg->mg_virtual->svt_dup) {
	    nmg->mg_virtual->svt_dup(aTHX_ nmg, param);
	}
    }
    return mgret;
}

#endif /* USE_ITHREADS */

struct ptr_tbl_arena {
    struct ptr_tbl_arena *next;
    struct ptr_tbl_ent array[1023/3]; /* as ptr_tbl_ent has 3 pointers.  */
};

/* create a new pointer-mapping table */

PTR_TBL_t *
Perl_ptr_table_new(pTHX)
{
    PTR_TBL_t *tbl;
    PERL_UNUSED_CONTEXT;

    Newx(tbl, 1, PTR_TBL_t);
    tbl->tbl_max	= 511;
    tbl->tbl_items	= 0;
    tbl->tbl_arena	= NULL;
    tbl->tbl_arena_next	= NULL;
    tbl->tbl_arena_end	= NULL;
    Newxz(tbl->tbl_ary, tbl->tbl_max + 1, PTR_TBL_ENT_t*);
    return tbl;
}

#define PTR_TABLE_HASH(ptr) \
  ((PTR2UV(ptr) >> 3) ^ (PTR2UV(ptr) >> (3 + 7)) ^ (PTR2UV(ptr) >> (3 + 17)))

/* map an existing pointer using a table */

STATIC PTR_TBL_ENT_t *
S_ptr_table_find(PTR_TBL_t *const tbl, const void *const sv)
{
    PTR_TBL_ENT_t *tblent;
    const UV hash = PTR_TABLE_HASH(sv);

    PERL_ARGS_ASSERT_PTR_TABLE_FIND;

    tblent = tbl->tbl_ary[hash & tbl->tbl_max];
    for (; tblent; tblent = tblent->next) {
	if (tblent->oldval == sv)
	    return tblent;
    }
    return NULL;
}

void *
Perl_ptr_table_fetch(pTHX_ PTR_TBL_t *const tbl, const void *const sv)
{
    PTR_TBL_ENT_t const *const tblent = ptr_table_find(tbl, sv);

    PERL_ARGS_ASSERT_PTR_TABLE_FETCH;
    PERL_UNUSED_CONTEXT;

    return tblent ? tblent->newval : NULL;
}

/* add a new entry to a pointer-mapping table */

void
Perl_ptr_table_store(pTHX_ PTR_TBL_t *const tbl, const void *const oldsv, void *const newsv)
{
    PTR_TBL_ENT_t *tblent = ptr_table_find(tbl, oldsv);

    PERL_ARGS_ASSERT_PTR_TABLE_STORE;
    PERL_UNUSED_CONTEXT;

    if (tblent) {
	tblent->newval = newsv;
    } else {
	const UV entry = PTR_TABLE_HASH(oldsv) & tbl->tbl_max;

	if (tbl->tbl_arena_next == tbl->tbl_arena_end) {
	    struct ptr_tbl_arena *new_arena;

	    Newx(new_arena, 1, struct ptr_tbl_arena);
	    new_arena->next = tbl->tbl_arena;
	    tbl->tbl_arena = new_arena;
	    tbl->tbl_arena_next = new_arena->array;
	    tbl->tbl_arena_end = new_arena->array
		+ sizeof(new_arena->array) / sizeof(new_arena->array[0]);
	}

	tblent = tbl->tbl_arena_next++;

	tblent->oldval = oldsv;
	tblent->newval = newsv;
	tblent->next = tbl->tbl_ary[entry];
	tbl->tbl_ary[entry] = tblent;
	tbl->tbl_items++;
	if (tblent->next && tbl->tbl_items > tbl->tbl_max)
	    ptr_table_split(tbl);
    }
}

/* double the hash bucket size of an existing ptr table */

void
Perl_ptr_table_split(pTHX_ PTR_TBL_t *const tbl)
{
    PTR_TBL_ENT_t **ary = tbl->tbl_ary;
    const UV oldsize = tbl->tbl_max + 1;
    UV newsize = oldsize * 2;
    UV i;

    PERL_ARGS_ASSERT_PTR_TABLE_SPLIT;
    PERL_UNUSED_CONTEXT;

    Renew(ary, newsize, PTR_TBL_ENT_t*);
    Zero(&ary[oldsize], newsize-oldsize, PTR_TBL_ENT_t*);
    tbl->tbl_max = --newsize;
    tbl->tbl_ary = ary;
    for (i=0; i < oldsize; i++, ary++) {
	PTR_TBL_ENT_t **entp = ary;
	PTR_TBL_ENT_t *ent = *ary;
	PTR_TBL_ENT_t **curentp;
	if (!ent)
	    continue;
	curentp = ary + oldsize;
	do {
	    if ((newsize & PTR_TABLE_HASH(ent->oldval)) != i) {
		*entp = ent->next;
		ent->next = *curentp;
		*curentp = ent;
	    }
	    else
		entp = &ent->next;
	    ent = *entp;
	} while (ent);
    }
}

/* remove all the entries from a ptr table */
/* Deprecated - will be removed post 5.14 */

void
Perl_ptr_table_clear(pTHX_ PTR_TBL_t *const tbl)
{
    if (tbl && tbl->tbl_items) {
	struct ptr_tbl_arena *arena = tbl->tbl_arena;

	Zero(tbl->tbl_ary, tbl->tbl_max + 1, struct ptr_tbl_ent **);

	while (arena) {
	    struct ptr_tbl_arena *next = arena->next;

	    Safefree(arena);
	    arena = next;
	};

	tbl->tbl_items = 0;
	tbl->tbl_arena = NULL;
	tbl->tbl_arena_next = NULL;
	tbl->tbl_arena_end = NULL;
    }
}

/* clear and free a ptr table */

void
Perl_ptr_table_free(pTHX_ PTR_TBL_t *const tbl)
{
    struct ptr_tbl_arena *arena;

    if (!tbl) {
        return;
    }

    arena = tbl->tbl_arena;

    while (arena) {
	struct ptr_tbl_arena *next = arena->next;

	Safefree(arena);
	arena = next;
    }

    Safefree(tbl->tbl_ary);
    Safefree(tbl);
}

#if defined(USE_ITHREADS)

void
Perl_rvpv_dup(pTHX_ SV *const dstr, const SV *const sstr, CLONE_PARAMS *const param)
{
    PERL_ARGS_ASSERT_RVPV_DUP;

    if (SvROK(sstr)) {
	if (SvWEAKREF(sstr)) {
	    SvRV_set(dstr, sv_dup(SvRV_const(sstr), param));
	    if (param->flags & CLONEf_JOIN_IN) {
		/* if joining, we add any back references individually rather
		 * than copying the whole backref array */
		Perl_sv_add_backref(aTHX_ SvRV(dstr), dstr);
	    }
	}
	else
	    SvRV_set(dstr, sv_dup_inc(SvRV_const(sstr), param));
    }
    else if (SvPVX_const(sstr)) {
	/* Has something there */
	if (SvLEN(sstr)) {
	    /* Normal PV - clone whole allocated space */
	    SvPV_set(dstr, SAVEPVN(SvPVX_const(sstr), SvLEN(sstr)-1));
	    if (SvREADONLY(sstr) && SvFAKE(sstr)) {
		/* Not that normal - actually sstr is copy on write.
		   But we are a true, independent SV, so:  */
		SvREADONLY_off(dstr);
		SvFAKE_off(dstr);
	    }
	}
	else {
	    /* Special case - not normally malloced for some reason */
	    if (isGV_with_GP(sstr)) {
		/* Don't need to do anything here.  */
	    }
	    else if ((SvREADONLY(sstr) && SvFAKE(sstr))) {
		/* A "shared" PV - clone it as "shared" PV */
		SvPV_set(dstr,
			 HEK_KEY(hek_dup(SvSHARED_HEK_FROM_PV(SvPVX_const(sstr)),
					 param)));
	    }
	    else {
		/* Some other special case - random pointer */
		SvPV_set(dstr, (char *) SvPVX_const(sstr));		
	    }
	}
    }
    else {
	/* Copy the NULL */
	SvPV_set(dstr, NULL);
    }
}

/* duplicate a list of SVs. source and dest may point to the same memory.  */
static SV **
S_sv_dup_inc_multiple(pTHX_ SV *const *source, SV **dest,
		      SSize_t items, CLONE_PARAMS *const param)
{
    PERL_ARGS_ASSERT_SV_DUP_INC_MULTIPLE;

    while (items-- > 0) {
	*dest++ = sv_dup_inc(*source++, param);
    }

    return dest;
}

/* duplicate an SV of any type (including AV, HV etc) */

static SV *
S_sv_dup_common(pTHX_ const SV *const sstr, CLONE_PARAMS *const param)
{
    dVAR;
    SV *dstr;

    PERL_ARGS_ASSERT_SV_DUP_COMMON;

    if (SvTYPE(sstr) == (svtype)SVTYPEMASK) {
#ifdef DEBUG_LEAKING_SCALARS_ABORT
	abort();
#endif
	return NULL;
    }
    /* look for it in the table first */
    dstr = MUTABLE_SV(ptr_table_fetch(PL_ptr_table, sstr));
    if (dstr)
	return dstr;

    if(param->flags & CLONEf_JOIN_IN) {
        /** We are joining here so we don't want do clone
	    something that is bad **/
	if (SvTYPE(sstr) == SVt_PVHV) {
	    const HEK * const hvname = HvNAME_HEK(sstr);
	    if (hvname) {
		/** don't clone stashes if they already exist **/
		dstr = MUTABLE_SV(gv_stashpvn(HEK_KEY(hvname), HEK_LEN(hvname),
                                                HEK_UTF8(hvname) ? SVf_UTF8 : 0));
		ptr_table_store(PL_ptr_table, sstr, dstr);
		return dstr;
	    }
        }
	else if (SvTYPE(sstr) == SVt_PVGV && !SvFAKE(sstr)) {
	    HV *stash = GvSTASH(sstr);
	    const HEK * hvname;
	    if (stash && (hvname = HvNAME_HEK(stash))) {
		/** don't clone GVs if they already exist **/
		SV **svp;
		stash = gv_stashpvn(HEK_KEY(hvname), HEK_LEN(hvname),
				    HEK_UTF8(hvname) ? SVf_UTF8 : 0);
		svp = hv_fetch(
			stash, GvNAME(sstr),
			GvNAMEUTF8(sstr)
			    ? -GvNAMELEN(sstr)
			    :  GvNAMELEN(sstr),
			0
		      );
		if (svp && *svp && SvTYPE(*svp) == SVt_PVGV) {
		    ptr_table_store(PL_ptr_table, sstr, *svp);
		    return *svp;
		}
	    }
        }
    }

    /* create anew and remember what it is */
    new_SV(dstr);

#ifdef DEBUG_LEAKING_SCALARS
    dstr->sv_debug_optype = sstr->sv_debug_optype;
    dstr->sv_debug_line = sstr->sv_debug_line;
    dstr->sv_debug_inpad = sstr->sv_debug_inpad;
    dstr->sv_debug_parent = (SV*)sstr;
    FREE_SV_DEBUG_FILE(dstr);
    dstr->sv_debug_file = savepv(sstr->sv_debug_file);
#endif

    ptr_table_store(PL_ptr_table, sstr, dstr);

    /* clone */
    SvFLAGS(dstr)	= SvFLAGS(sstr);
    SvFLAGS(dstr)	&= ~SVf_OOK;		/* don't propagate OOK hack */
    SvREFCNT(dstr)	= 0;			/* must be before any other dups! */

#ifdef DEBUGGING
    if (SvANY(sstr) && PL_watch_pvx && SvPVX_const(sstr) == PL_watch_pvx)
	PerlIO_printf(Perl_debug_log, "watch at %p hit, found string \"%s\"\n",
		      (void*)PL_watch_pvx, SvPVX_const(sstr));
#endif

    /* don't clone objects whose class has asked us not to */
    if (SvOBJECT(sstr) && ! (SvFLAGS(SvSTASH(sstr)) & SVphv_CLONEABLE)) {
	SvFLAGS(dstr) = 0;
	return dstr;
    }

    switch (SvTYPE(sstr)) {
    case SVt_NULL:
	SvANY(dstr)	= NULL;
	break;
    case SVt_IV:
	SvANY(dstr)	= (XPVIV*)((char*)&(dstr->sv_u.svu_iv) - STRUCT_OFFSET(XPVIV, xiv_iv));
	if(SvROK(sstr)) {
	    Perl_rvpv_dup(aTHX_ dstr, sstr, param);
	} else {
	    SvIV_set(dstr, SvIVX(sstr));
	}
	break;
    case SVt_NV:
	SvANY(dstr)	= new_XNV();
	SvNV_set(dstr, SvNVX(sstr));
	break;
	/* case SVt_BIND: */
    default:
	{
	    /* These are all the types that need complex bodies allocating.  */
	    void *new_body;
	    const svtype sv_type = SvTYPE(sstr);
	    const struct body_details *const sv_type_details
		= bodies_by_type + sv_type;

	    switch (sv_type) {
	    default:
		Perl_croak(aTHX_ "Bizarre SvTYPE [%" IVdf "]", (IV)SvTYPE(sstr));
		break;

	    case SVt_PVGV:
	    case SVt_PVIO:
	    case SVt_PVFM:
	    case SVt_PVHV:
	    case SVt_PVAV:
	    case SVt_PVCV:
	    case SVt_PVLV:
	    case SVt_REGEXP:
	    case SVt_PVMG:
	    case SVt_PVNV:
	    case SVt_PVIV:
	    case SVt_PV:
		assert(sv_type_details->body_size);
		if (sv_type_details->arena) {
		    new_body_inline(new_body, sv_type);
		    new_body
			= (void*)((char*)new_body - sv_type_details->offset);
		} else {
		    new_body = new_NOARENA(sv_type_details);
		}
	    }
	    assert(new_body);
	    SvANY(dstr) = new_body;

#ifndef PURIFY
	    Copy(((char*)SvANY(sstr)) + sv_type_details->offset,
		 ((char*)SvANY(dstr)) + sv_type_details->offset,
		 sv_type_details->copy, char);
#else
	    Copy(((char*)SvANY(sstr)),
		 ((char*)SvANY(dstr)),
		 sv_type_details->body_size + sv_type_details->offset, char);
#endif

	    if (sv_type != SVt_PVAV && sv_type != SVt_PVHV
		&& !isGV_with_GP(dstr)
		&& !(sv_type == SVt_PVIO && !(IoFLAGS(dstr) & IOf_FAKE_DIRP)))
		Perl_rvpv_dup(aTHX_ dstr, sstr, param);

	    /* The Copy above means that all the source (unduplicated) pointers
	       are now in the destination.  We can check the flags and the
	       pointers in either, but it's possible that there's less cache
	       missing by always going for the destination.
	       FIXME - instrument and check that assumption  */
	    if (sv_type >= SVt_PVMG) {
		if ((sv_type == SVt_PVMG) && SvPAD_OUR(dstr)) {
		    SvOURSTASH_set(dstr, hv_dup_inc(SvOURSTASH(dstr), param));
		} else if (SvMAGIC(dstr))
		    SvMAGIC_set(dstr, mg_dup(SvMAGIC(dstr), param));
		if (SvSTASH(dstr))
		    SvSTASH_set(dstr, hv_dup_inc(SvSTASH(dstr), param));
	    }

	    /* The cast silences a GCC warning about unhandled types.  */
	    switch ((int)sv_type) {
	    case SVt_PV:
		break;
	    case SVt_PVIV:
		break;
	    case SVt_PVNV:
		break;
	    case SVt_PVMG:
		break;
	    case SVt_REGEXP:
		/* FIXME for plugins */
		re_dup_guts((REGEXP*) sstr, (REGEXP*) dstr, param);
		break;
	    case SVt_PVLV:
		/* XXX LvTARGOFF sometimes holds PMOP* when DEBUGGING */
		if (LvTYPE(dstr) == 't') /* for tie: unrefcnted fake (SV**) */
		    LvTARG(dstr) = dstr;
		else if (LvTYPE(dstr) == 'T') /* for tie: fake HE */
		    LvTARG(dstr) = MUTABLE_SV(he_dup((HE*)LvTARG(dstr), 0, param));
		else
		    LvTARG(dstr) = sv_dup_inc(LvTARG(dstr), param);
	    case SVt_PVGV:
		/* non-GP case already handled above */
		if(isGV_with_GP(sstr)) {
		    GvNAME_HEK(dstr) = hek_dup(GvNAME_HEK(dstr), param);
		    /* Don't call sv_add_backref here as it's going to be
		       created as part of the magic cloning of the symbol
		       table--unless this is during a join and the stash
		       is not actually being cloned.  */
		    /* Danger Will Robinson - GvGP(dstr) isn't initialised
		       at the point of this comment.  */
		    GvSTASH(dstr) = hv_dup(GvSTASH(dstr), param);
		    if (param->flags & CLONEf_JOIN_IN)
			Perl_sv_add_backref(aTHX_ MUTABLE_SV(GvSTASH(dstr)), dstr);
		    GvGP_set(dstr, gp_dup(GvGP(sstr), param));
		    (void)GpREFCNT_inc(GvGP(dstr));
		}
		break;
	    case SVt_PVIO:
		/* PL_parser->rsfp_filters entries have fake IoDIRP() */
		if(IoFLAGS(dstr) & IOf_FAKE_DIRP) {
		    /* I have no idea why fake dirp (rsfps)
		       should be treated differently but otherwise
		       we end up with leaks -- sky*/
		    IoTOP_GV(dstr)      = gv_dup_inc(IoTOP_GV(dstr), param);
		    IoFMT_GV(dstr)      = gv_dup_inc(IoFMT_GV(dstr), param);
		    IoBOTTOM_GV(dstr)   = gv_dup_inc(IoBOTTOM_GV(dstr), param);
		} else {
		    IoTOP_GV(dstr)      = gv_dup(IoTOP_GV(dstr), param);
		    IoFMT_GV(dstr)      = gv_dup(IoFMT_GV(dstr), param);
		    IoBOTTOM_GV(dstr)   = gv_dup(IoBOTTOM_GV(dstr), param);
		    if (IoDIRP(dstr)) {
			IoDIRP(dstr)	= dirp_dup(IoDIRP(dstr), param);
		    } else {
			NOOP;
			/* IoDIRP(dstr) is already a copy of IoDIRP(sstr)  */
		    }
		    IoIFP(dstr)	= fp_dup(IoIFP(sstr), IoTYPE(dstr), param);
		}
		if (IoOFP(dstr) == IoIFP(sstr))
		    IoOFP(dstr) = IoIFP(dstr);
		else
		    IoOFP(dstr)	= fp_dup(IoOFP(dstr), IoTYPE(dstr), param);
		IoTOP_NAME(dstr)	= SAVEPV(IoTOP_NAME(dstr));
		IoFMT_NAME(dstr)	= SAVEPV(IoFMT_NAME(dstr));
		IoBOTTOM_NAME(dstr)	= SAVEPV(IoBOTTOM_NAME(dstr));
		break;
	    case SVt_PVAV:
		/* avoid cloning an empty array */
		if (AvARRAY((const AV *)sstr) && AvFILLp((const AV *)sstr) >= 0) {
		    SV **dst_ary, **src_ary;
		    SSize_t items = AvFILLp((const AV *)sstr) + 1;

		    src_ary = AvARRAY((const AV *)sstr);
		    Newxz(dst_ary, AvMAX((const AV *)sstr)+1, SV*);
		    ptr_table_store(PL_ptr_table, src_ary, dst_ary);
		    AvARRAY(MUTABLE_AV(dstr)) = dst_ary;
		    AvALLOC((const AV *)dstr) = dst_ary;
		    if (AvREAL((const AV *)sstr)) {
			dst_ary = sv_dup_inc_multiple(src_ary, dst_ary, items,
						      param);
		    }
		    else {
			while (items-- > 0)
			    *dst_ary++ = sv_dup(*src_ary++, param);
		    }
		    items = AvMAX((const AV *)sstr) - AvFILLp((const AV *)sstr);
		    while (items-- > 0) {
			*dst_ary++ = &PL_sv_undef;
		    }
		}
		else {
		    AvARRAY(MUTABLE_AV(dstr))	= NULL;
		    AvALLOC((const AV *)dstr)	= (SV**)NULL;
		    AvMAX(  (const AV *)dstr)	= -1;
		    AvFILLp((const AV *)dstr)	= -1;
		}
		break;
	    case SVt_PVHV:
		if (HvARRAY((const HV *)sstr)) {
		    STRLEN i = 0;
		    const bool sharekeys = !!HvSHAREKEYS(sstr);
		    XPVHV * const dxhv = (XPVHV*)SvANY(dstr);
		    XPVHV * const sxhv = (XPVHV*)SvANY(sstr);
		    char *darray;
		    Newx(darray, PERL_HV_ARRAY_ALLOC_BYTES(dxhv->xhv_max+1)
			+ (SvOOK(sstr) ? sizeof(struct xpvhv_aux) : 0),
			char);
		    HvARRAY(dstr) = (HE**)darray;
		    while (i <= sxhv->xhv_max) {
			const HE * const source = HvARRAY(sstr)[i];
			HvARRAY(dstr)[i] = source
			    ? he_dup(source, sharekeys, param) : 0;
			++i;
		    }
		    if (SvOOK(sstr)) {
			const struct xpvhv_aux * const saux = HvAUX(sstr);
			struct xpvhv_aux * const daux = HvAUX(dstr);
			/* This flag isn't copied.  */
			SvOOK_on(dstr);

			if (saux->xhv_name_count) {
			    HEK ** const sname = saux->xhv_name_u.xhvnameu_names;
			    const I32 count
			     = saux->xhv_name_count < 0
			        ? -saux->xhv_name_count
			        :  saux->xhv_name_count;
			    HEK **shekp = sname + count;
			    HEK **dhekp;
			    Newx(daux->xhv_name_u.xhvnameu_names, count, HEK *);
			    dhekp = daux->xhv_name_u.xhvnameu_names + count;
			    while (shekp-- > sname) {
				dhekp--;
				*dhekp = hek_dup(*shekp, param);
			    }
			}
			else {
			    daux->xhv_name_u.xhvnameu_name
				= hek_dup(saux->xhv_name_u.xhvnameu_name,
					  param);
			}
			daux->xhv_name_count = saux->xhv_name_count;

			daux->xhv_riter = saux->xhv_riter;
			daux->xhv_eiter = saux->xhv_eiter
			    ? he_dup(saux->xhv_eiter,
					cBOOL(HvSHAREKEYS(sstr)), param) : 0;
			/* backref array needs refcnt=2; see sv_add_backref */
			daux->xhv_backreferences =
			    (param->flags & CLONEf_JOIN_IN)
				/* when joining, we let the individual GVs and
				 * CVs add themselves to backref as
				 * needed. This avoids pulling in stuff
				 * that isn't required, and simplifies the
				 * case where stashes aren't cloned back
				 * if they already exist in the parent
				 * thread */
			    ? NULL
			    : saux->xhv_backreferences
				? (SvTYPE(saux->xhv_backreferences) == SVt_PVAV)
				    ? MUTABLE_AV(SvREFCNT_inc(
					  sv_dup_inc((const SV *)
					    saux->xhv_backreferences, param)))
				    : MUTABLE_AV(sv_dup((const SV *)
					    saux->xhv_backreferences, param))
				: 0;

                        daux->xhv_mro_meta = saux->xhv_mro_meta
                            ? mro_meta_dup(saux->xhv_mro_meta, param)
                            : 0;

			/* Record stashes for possible cloning in Perl_clone(). */
			if (HvNAME(sstr))
			    av_push(param->stashes, dstr);
		    }
		}
		else
		    HvARRAY(MUTABLE_HV(dstr)) = NULL;
		break;
	    case SVt_PVCV:
		if (!(param->flags & CLONEf_COPY_STACKS)) {
		    CvDEPTH(dstr) = 0;
		}
		/*FALLTHROUGH*/
	    case SVt_PVFM:
		/* NOTE: not refcounted */
		SvANY(MUTABLE_CV(dstr))->xcv_stash =
		    hv_dup(CvSTASH(dstr), param);
		if ((param->flags & CLONEf_JOIN_IN) && CvSTASH(dstr))
		    Perl_sv_add_backref(aTHX_ MUTABLE_SV(CvSTASH(dstr)), dstr);
		if (!CvISXSUB(dstr)) {
		    OP_REFCNT_LOCK;
		    CvROOT(dstr) = OpREFCNT_inc(CvROOT(dstr));
		    OP_REFCNT_UNLOCK;
		} else if (CvCONST(dstr)) {
		    CvXSUBANY(dstr).any_ptr =
			sv_dup_inc((const SV *)CvXSUBANY(dstr).any_ptr, param);
		}
		if (CvDYNFILE(dstr)) CvFILE(dstr) = SAVEPV(CvFILE(dstr));
		/* don't dup if copying back - CvGV isn't refcounted, so the
		 * duped GV may never be freed. A bit of a hack! DAPM */
		SvANY(MUTABLE_CV(dstr))->xcv_gv =
		    CvCVGV_RC(dstr)
		    ? gv_dup_inc(CvGV(sstr), param)
		    : (param->flags & CLONEf_JOIN_IN)
			? NULL
			: gv_dup(CvGV(sstr), param);

		CvPADLIST(dstr) = padlist_dup(CvPADLIST(sstr), param);
		CvOUTSIDE(dstr)	=
		    CvWEAKOUTSIDE(sstr)
		    ? cv_dup(    CvOUTSIDE(dstr), param)
		    : cv_dup_inc(CvOUTSIDE(dstr), param);
		break;
	    }
	}
    }

    if (SvOBJECT(dstr) && SvTYPE(dstr) != SVt_PVIO)
	++PL_sv_objcount;

    return dstr;
 }

SV *
Perl_sv_dup_inc(pTHX_ const SV *const sstr, CLONE_PARAMS *const param)
{
    PERL_ARGS_ASSERT_SV_DUP_INC;
    return sstr ? SvREFCNT_inc(sv_dup_common(sstr, param)) : NULL;
}

SV *
Perl_sv_dup(pTHX_ const SV *const sstr, CLONE_PARAMS *const param)
{
    SV *dstr = sstr ? sv_dup_common(sstr, param) : NULL;
    PERL_ARGS_ASSERT_SV_DUP;

    /* Track every SV that (at least initially) had a reference count of 0.
       We need to do this by holding an actual reference to it in this array.
       If we attempt to cheat, turn AvREAL_off(), and store only pointers
       (akin to the stashes hash, and the perl stack), we come unstuck if
       a weak reference (or other SV legitimately SvREFCNT() == 0 for this
       thread) is manipulated in a CLONE method, because CLONE runs before the
       unreferenced array is walked to find SVs still with SvREFCNT() == 0
       (and fix things up by giving each a reference via the temps stack).
       Instead, during CLONE, if the 0-referenced SV has SvREFCNT_inc() and
       then SvREFCNT_dec(), it will be cleaned up (and added to the free list)
       before the walk of unreferenced happens and a reference to that is SV
       added to the temps stack. At which point we have the same SV considered
       to be in use, and free to be re-used. Not good.
    */
    if (dstr && !(param->flags & CLONEf_COPY_STACKS) && !SvREFCNT(dstr)) {
	assert(param->unreferenced);
	av_push(param->unreferenced, SvREFCNT_inc(dstr));
    }

    return dstr;
}

/* duplicate a context */

PERL_CONTEXT *
Perl_cx_dup(pTHX_ PERL_CONTEXT *cxs, I32 ix, I32 max, CLONE_PARAMS* param)
{
    PERL_CONTEXT *ncxs;

    PERL_ARGS_ASSERT_CX_DUP;

    if (!cxs)
	return (PERL_CONTEXT*)NULL;

    /* look for it in the table first */
    ncxs = (PERL_CONTEXT*)ptr_table_fetch(PL_ptr_table, cxs);
    if (ncxs)
	return ncxs;

    /* create anew and remember what it is */
    Newx(ncxs, max + 1, PERL_CONTEXT);
    ptr_table_store(PL_ptr_table, cxs, ncxs);
    Copy(cxs, ncxs, max + 1, PERL_CONTEXT);

    while (ix >= 0) {
	PERL_CONTEXT * const ncx = &ncxs[ix];
	if (CxTYPE(ncx) == CXt_SUBST) {
	    Perl_croak(aTHX_ "Cloning substitution context is unimplemented");
	}
	else {
	    ncx->blk_oldcop = (COP*)any_dup(ncx->blk_oldcop, param->proto_perl);
	    switch (CxTYPE(ncx)) {
	    case CXt_SUB:
		ncx->blk_sub.cv		= (ncx->blk_sub.olddepth == 0
					   ? cv_dup_inc(ncx->blk_sub.cv, param)
					   : cv_dup(ncx->blk_sub.cv,param));
		ncx->blk_sub.argarray	= (CxHASARGS(ncx)
					   ? av_dup_inc(ncx->blk_sub.argarray,
							param)
					   : NULL);
		ncx->blk_sub.savearray	= av_dup_inc(ncx->blk_sub.savearray,
						     param);
		ncx->blk_sub.oldcomppad = (PAD*)ptr_table_fetch(PL_ptr_table,
					   ncx->blk_sub.oldcomppad);
		break;
	    case CXt_EVAL:
		ncx->blk_eval.old_namesv = sv_dup_inc(ncx->blk_eval.old_namesv,
						      param);
		ncx->blk_eval.cur_text	= sv_dup(ncx->blk_eval.cur_text, param);
		ncx->blk_eval.cv = cv_dup(ncx->blk_eval.cv, param);
		break;
	    case CXt_LOOP_LAZYSV:
		ncx->blk_loop.state_u.lazysv.end
		    = sv_dup_inc(ncx->blk_loop.state_u.lazysv.end, param);
		/* We are taking advantage of av_dup_inc and sv_dup_inc
		   actually being the same function, and order equivalence of
		   the two unions.
		   We can assert the later [but only at run time :-(]  */
		assert ((void *) &ncx->blk_loop.state_u.ary.ary ==
			(void *) &ncx->blk_loop.state_u.lazysv.cur);
	    case CXt_LOOP_FOR:
		ncx->blk_loop.state_u.ary.ary
		    = av_dup_inc(ncx->blk_loop.state_u.ary.ary, param);
	    case CXt_LOOP_LAZYIV:
	    case CXt_LOOP_PLAIN:
		if (CxPADLOOP(ncx)) {
		    ncx->blk_loop.itervar_u.oldcomppad
			= (PAD*)ptr_table_fetch(PL_ptr_table,
					ncx->blk_loop.itervar_u.oldcomppad);
		} else {
		    ncx->blk_loop.itervar_u.gv
			= gv_dup((const GV *)ncx->blk_loop.itervar_u.gv,
				    param);
		}
		break;
	    case CXt_FORMAT:
		ncx->blk_format.cv	= cv_dup(ncx->blk_format.cv, param);
		ncx->blk_format.gv	= gv_dup(ncx->blk_format.gv, param);
		ncx->blk_format.dfoutgv	= gv_dup_inc(ncx->blk_format.dfoutgv,
						     param);
		break;
	    case CXt_BLOCK:
	    case CXt_NULL:
	    case CXt_WHEN:
	    case CXt_GIVEN:
		break;
	    }
	}
	--ix;
    }
    return ncxs;
}

/* duplicate a stack info structure */

PERL_SI *
Perl_si_dup(pTHX_ PERL_SI *si, CLONE_PARAMS* param)
{
    PERL_SI *nsi;

    PERL_ARGS_ASSERT_SI_DUP;

    if (!si)
	return (PERL_SI*)NULL;

    /* look for it in the table first */
    nsi = (PERL_SI*)ptr_table_fetch(PL_ptr_table, si);
    if (nsi)
	return nsi;

    /* create anew and remember what it is */
    Newxz(nsi, 1, PERL_SI);
    ptr_table_store(PL_ptr_table, si, nsi);

    nsi->si_stack	= av_dup_inc(si->si_stack, param);
    nsi->si_cxix	= si->si_cxix;
    nsi->si_cxmax	= si->si_cxmax;
    nsi->si_cxstack	= cx_dup(si->si_cxstack, si->si_cxix, si->si_cxmax, param);
    nsi->si_type	= si->si_type;
    nsi->si_prev	= si_dup(si->si_prev, param);
    nsi->si_next	= si_dup(si->si_next, param);
    nsi->si_markoff	= si->si_markoff;

    return nsi;
}

#define POPINT(ss,ix)	((ss)[--(ix)].any_i32)
#define TOPINT(ss,ix)	((ss)[ix].any_i32)
#define POPLONG(ss,ix)	((ss)[--(ix)].any_long)
#define TOPLONG(ss,ix)	((ss)[ix].any_long)
#define POPIV(ss,ix)	((ss)[--(ix)].any_iv)
#define TOPIV(ss,ix)	((ss)[ix].any_iv)
#define POPUV(ss,ix)	((ss)[--(ix)].any_uv)
#define TOPUV(ss,ix)	((ss)[ix].any_uv)
#define POPBOOL(ss,ix)	((ss)[--(ix)].any_bool)
#define TOPBOOL(ss,ix)	((ss)[ix].any_bool)
#define POPPTR(ss,ix)	((ss)[--(ix)].any_ptr)
#define TOPPTR(ss,ix)	((ss)[ix].any_ptr)
#define POPDPTR(ss,ix)	((ss)[--(ix)].any_dptr)
#define TOPDPTR(ss,ix)	((ss)[ix].any_dptr)
#define POPDXPTR(ss,ix)	((ss)[--(ix)].any_dxptr)
#define TOPDXPTR(ss,ix)	((ss)[ix].any_dxptr)

/* XXXXX todo */
#define pv_dup_inc(p)	SAVEPV(p)
#define pv_dup(p)	SAVEPV(p)
#define svp_dup_inc(p,pp)	any_dup(p,pp)

/* map any object to the new equivent - either something in the
 * ptr table, or something in the interpreter structure
 */

void *
Perl_any_dup(pTHX_ void *v, const PerlInterpreter *proto_perl)
{
    void *ret;

    PERL_ARGS_ASSERT_ANY_DUP;

    if (!v)
	return (void*)NULL;

    /* look for it in the table first */
    ret = ptr_table_fetch(PL_ptr_table, v);
    if (ret)
	return ret;

    /* see if it is part of the interpreter structure */
    if (v >= (void*)proto_perl && v < (void*)(proto_perl+1))
	ret = (void*)(((char*)aTHX) + (((char*)v) - (char*)proto_perl));
    else {
	ret = v;
    }

    return ret;
}

/* duplicate the save stack */

ANY *
Perl_ss_dup(pTHX_ PerlInterpreter *proto_perl, CLONE_PARAMS* param)
{
    dVAR;
    ANY * const ss	= proto_perl->Isavestack;
    const I32 max	= proto_perl->Isavestack_max;
    I32 ix		= proto_perl->Isavestack_ix;
    ANY *nss;
    const SV *sv;
    const GV *gv;
    const AV *av;
    const HV *hv;
    void* ptr;
    int intval;
    long longval;
    GP *gp;
    IV iv;
    I32 i;
    char *c = NULL;
    void (*dptr) (void*);
    void (*dxptr) (pTHX_ void*);

    PERL_ARGS_ASSERT_SS_DUP;

    Newxz(nss, max, ANY);

    while (ix > 0) {
	const UV uv = POPUV(ss,ix);
	const U8 type = (U8)uv & SAVE_MASK;

	TOPUV(nss,ix) = uv;
	switch (type) {
	case SAVEt_CLEARSV:
	    break;
	case SAVEt_HELEM:		/* hash element */
	    sv = (const SV *)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = sv_dup_inc(sv, param);
	    /* fall through */
	case SAVEt_ITEM:			/* normal string */
        case SAVEt_GVSV:			/* scalar slot in GV */
        case SAVEt_SV:				/* scalar reference */
	    sv = (const SV *)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = sv_dup_inc(sv, param);
	    /* fall through */
	case SAVEt_FREESV:
	case SAVEt_MORTALIZESV:
	    sv = (const SV *)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = sv_dup_inc(sv, param);
	    break;
	case SAVEt_SHARED_PVREF:		/* char* in shared space */
	    c = (char*)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = savesharedpv(c);
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
	    break;
        case SAVEt_GENERIC_SVREF:		/* generic sv */
        case SAVEt_SVREF:			/* scalar reference */
	    sv = (const SV *)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = sv_dup_inc(sv, param);
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = svp_dup_inc((SV**)ptr, proto_perl);/* XXXXX */
	    break;
        case SAVEt_HV:				/* hash reference */
        case SAVEt_AV:				/* array reference */
	    sv = (const SV *) POPPTR(ss,ix);
	    TOPPTR(nss,ix) = sv_dup_inc(sv, param);
	    /* fall through */
	case SAVEt_COMPPAD:
	case SAVEt_NSTAB:
	    sv = (const SV *) POPPTR(ss,ix);
	    TOPPTR(nss,ix) = sv_dup(sv, param);
	    break;
	case SAVEt_INT:				/* int reference */
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
	    intval = (int)POPINT(ss,ix);
	    TOPINT(nss,ix) = intval;
	    break;
	case SAVEt_LONG:			/* long reference */
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
	    longval = (long)POPLONG(ss,ix);
	    TOPLONG(nss,ix) = longval;
	    break;
	case SAVEt_I32:				/* I32 reference */
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
	    i = POPINT(ss,ix);
	    TOPINT(nss,ix) = i;
	    break;
	case SAVEt_IV:				/* IV reference */
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
	    iv = POPIV(ss,ix);
	    TOPIV(nss,ix) = iv;
	    break;
	case SAVEt_HPTR:			/* HV* reference */
	case SAVEt_APTR:			/* AV* reference */
	case SAVEt_SPTR:			/* SV* reference */
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
	    sv = (const SV *)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = sv_dup(sv, param);
	    break;
	case SAVEt_VPTR:			/* random* reference */
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
	    /* Fall through */
	case SAVEt_INT_SMALL:
	case SAVEt_I32_SMALL:
	case SAVEt_I16:				/* I16 reference */
	case SAVEt_I8:				/* I8 reference */
	case SAVEt_BOOL:
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
	    break;
	case SAVEt_GENERIC_PVREF:		/* generic char* */
	case SAVEt_PPTR:			/* char* reference */
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
	    c = (char*)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = pv_dup(c);
	    break;
	case SAVEt_GP:				/* scalar reference */
	    gp = (GP*)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = gp = gp_dup(gp, param);
	    (void)GpREFCNT_inc(gp);
	    gv = (const GV *)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = gv_dup_inc(gv, param);
	    break;
	case SAVEt_FREEOP:
	    ptr = POPPTR(ss,ix);
	    if (ptr && (((OP*)ptr)->op_private & OPpREFCOUNTED)) {
		/* these are assumed to be refcounted properly */
		OP *o;
		switch (((OP*)ptr)->op_type) {
		case OP_LEAVESUB:
		case OP_LEAVESUBLV:
		case OP_LEAVEEVAL:
		case OP_LEAVE:
		case OP_SCOPE:
		case OP_LEAVEWRITE:
		    TOPPTR(nss,ix) = ptr;
		    o = (OP*)ptr;
		    OP_REFCNT_LOCK;
		    (void) OpREFCNT_inc(o);
		    OP_REFCNT_UNLOCK;
		    break;
		default:
		    TOPPTR(nss,ix) = NULL;
		    break;
		}
	    }
	    else
		TOPPTR(nss,ix) = NULL;
	    break;
	case SAVEt_FREECOPHH:
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = cophh_copy((COPHH *)ptr);
	    break;
	case SAVEt_DELETE:
	    hv = (const HV *)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = hv_dup_inc(hv, param);
	    i = POPINT(ss,ix);
	    TOPINT(nss,ix) = i;
	    /* Fall through */
	case SAVEt_FREEPV:
	    c = (char*)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = pv_dup_inc(c);
	    break;
	case SAVEt_STACK_POS:		/* Position on Perl stack */
	    i = POPINT(ss,ix);
	    TOPINT(nss,ix) = i;
	    break;
	case SAVEt_DESTRUCTOR:
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = any_dup(ptr, proto_perl);	/* XXX quite arbitrary */
	    dptr = POPDPTR(ss,ix);
	    TOPDPTR(nss,ix) = DPTR2FPTR(void (*)(void*),
					any_dup(FPTR2DPTR(void *, dptr),
						proto_perl));
	    break;
	case SAVEt_DESTRUCTOR_X:
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = any_dup(ptr, proto_perl);	/* XXX quite arbitrary */
	    dxptr = POPDXPTR(ss,ix);
	    TOPDXPTR(nss,ix) = DPTR2FPTR(void (*)(pTHX_ void*),
					 any_dup(FPTR2DPTR(void *, dxptr),
						 proto_perl));
	    break;
	case SAVEt_REGCONTEXT:
	case SAVEt_ALLOC:
	    ix -= uv >> SAVE_TIGHT_SHIFT;
	    break;
	case SAVEt_AELEM:		/* array element */
	    sv = (const SV *)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = sv_dup_inc(sv, param);
	    i = POPINT(ss,ix);
	    TOPINT(nss,ix) = i;
	    av = (const AV *)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = av_dup_inc(av, param);
	    break;
	case SAVEt_OP:
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = ptr;
	    break;
	case SAVEt_HINTS:
	    ptr = POPPTR(ss,ix);
	    ptr = cophh_copy((COPHH*)ptr);
	    TOPPTR(nss,ix) = ptr;
	    i = POPINT(ss,ix);
	    TOPINT(nss,ix) = i;
	    if (i & HINT_LOCALIZE_HH) {
		hv = (const HV *)POPPTR(ss,ix);
		TOPPTR(nss,ix) = hv_dup_inc(hv, param);
	    }
	    break;
	case SAVEt_PADSV_AND_MORTALIZE:
	    longval = (long)POPLONG(ss,ix);
	    TOPLONG(nss,ix) = longval;
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
	    sv = (const SV *)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = sv_dup_inc(sv, param);
	    break;
	case SAVEt_SET_SVFLAGS:
	    i = POPINT(ss,ix);
	    TOPINT(nss,ix) = i;
	    i = POPINT(ss,ix);
	    TOPINT(nss,ix) = i;
	    sv = (const SV *)POPPTR(ss,ix);
	    TOPPTR(nss,ix) = sv_dup(sv, param);
	    break;
	case SAVEt_RE_STATE:
	    {
		const struct re_save_state *const old_state
		    = (struct re_save_state *)
		    (ss + ix - SAVESTACK_ALLOC_FOR_RE_SAVE_STATE);
		struct re_save_state *const new_state
		    = (struct re_save_state *)
		    (nss + ix - SAVESTACK_ALLOC_FOR_RE_SAVE_STATE);

		Copy(old_state, new_state, 1, struct re_save_state);
		ix -= SAVESTACK_ALLOC_FOR_RE_SAVE_STATE;

		new_state->re_state_bostr
		    = pv_dup(old_state->re_state_bostr);
		new_state->re_state_reginput
		    = pv_dup(old_state->re_state_reginput);
		new_state->re_state_regeol
		    = pv_dup(old_state->re_state_regeol);
		new_state->re_state_regoffs
		    = (regexp_paren_pair*)
			any_dup(old_state->re_state_regoffs, proto_perl);
		new_state->re_state_reglastparen
		    = (U32*) any_dup(old_state->re_state_reglastparen, 
			      proto_perl);
		new_state->re_state_reglastcloseparen
		    = (U32*)any_dup(old_state->re_state_reglastcloseparen,
			      proto_perl);
		/* XXX This just has to be broken. The old save_re_context
		   code did SAVEGENERICPV(PL_reg_start_tmp);
		   PL_reg_start_tmp is char **.
		   Look above to what the dup code does for
		   SAVEt_GENERIC_PVREF
		   It can never have worked.
		   So this is merely a faithful copy of the exiting bug:  */
		new_state->re_state_reg_start_tmp
		    = (char **) pv_dup((char *)
				      old_state->re_state_reg_start_tmp);
		/* I assume that it only ever "worked" because no-one called
		   (pseudo)fork while the regexp engine had re-entered itself.
		*/
#ifdef PERL_OLD_COPY_ON_WRITE
		new_state->re_state_nrs
		    = sv_dup(old_state->re_state_nrs, param);
#endif
		new_state->re_state_reg_magic
		    = (MAGIC*) any_dup(old_state->re_state_reg_magic, 
			       proto_perl);
		new_state->re_state_reg_oldcurpm
		    = (PMOP*) any_dup(old_state->re_state_reg_oldcurpm, 
			      proto_perl);
		new_state->re_state_reg_curpm
		    = (PMOP*)  any_dup(old_state->re_state_reg_curpm, 
			       proto_perl);
		new_state->re_state_reg_oldsaved
		    = pv_dup(old_state->re_state_reg_oldsaved);
		new_state->re_state_reg_poscache
		    = pv_dup(old_state->re_state_reg_poscache);
		new_state->re_state_reg_starttry
		    = pv_dup(old_state->re_state_reg_starttry);
		break;
	    }
	case SAVEt_COMPILE_WARNINGS:
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = DUP_WARNINGS((STRLEN*)ptr);
	    break;
	case SAVEt_PARSER:
	    ptr = POPPTR(ss,ix);
	    TOPPTR(nss,ix) = parser_dup((const yy_parser*)ptr, param);
	    break;
	default:
	    Perl_croak(aTHX_
		       "panic: ss_dup inconsistency (%"IVdf")", (IV) type);
	}
    }

    return nss;
}


/* if sv is a stash, call $class->CLONE_SKIP(), and set the SVphv_CLONEABLE
 * flag to the result. This is done for each stash before cloning starts,
 * so we know which stashes want their objects cloned */

static void
do_mark_cloneable_stash(pTHX_ SV *const sv)
{
    const HEK * const hvname = HvNAME_HEK((const HV *)sv);
    if (hvname) {
	GV* const cloner = gv_fetchmethod_autoload(MUTABLE_HV(sv), "CLONE_SKIP", 0);
	SvFLAGS(sv) |= SVphv_CLONEABLE; /* clone objects by default */
	if (cloner && GvCV(cloner)) {
	    dSP;
	    UV status;

	    ENTER;
	    SAVETMPS;
	    PUSHMARK(SP);
	    mXPUSHs(newSVhek(hvname));
	    PUTBACK;
	    call_sv(MUTABLE_SV(GvCV(cloner)), G_SCALAR);
	    SPAGAIN;
	    status = POPu;
	    PUTBACK;
	    FREETMPS;
	    LEAVE;
	    if (status)
		SvFLAGS(sv) &= ~SVphv_CLONEABLE;
	}
    }
}



/*
=for apidoc perl_clone

Create and return a new interpreter by cloning the current one.

perl_clone takes these flags as parameters:

CLONEf_COPY_STACKS - is used to, well, copy the stacks also,
without it we only clone the data and zero the stacks,
with it we copy the stacks and the new perl interpreter is
ready to run at the exact same point as the previous one.
The pseudo-fork code uses COPY_STACKS while the
threads->create doesn't.

CLONEf_KEEP_PTR_TABLE -
perl_clone keeps a ptr_table with the pointer of the old
variable as a key and the new variable as a value,
this allows it to check if something has been cloned and not
clone it again but rather just use the value and increase the
refcount.  If KEEP_PTR_TABLE is not set then perl_clone will kill
the ptr_table using the function
C<ptr_table_free(PL_ptr_table); PL_ptr_table = NULL;>,
reason to keep it around is if you want to dup some of your own
variable who are outside the graph perl scans, example of this
code is in threads.xs create.

CLONEf_CLONE_HOST -
This is a win32 thing, it is ignored on unix, it tells perls
win32host code (which is c++) to clone itself, this is needed on
win32 if you want to run two threads at the same time,
if you just want to do some stuff in a separate perl interpreter
and then throw it away and return to the original one,
you don't need to do anything.

=cut
*/

/* XXX the above needs expanding by someone who actually understands it ! */
EXTERN_C PerlInterpreter *
perl_clone_host(PerlInterpreter* proto_perl, UV flags);

PerlInterpreter *
perl_clone(PerlInterpreter *proto_perl, UV flags)
{
   dVAR;
#ifdef PERL_IMPLICIT_SYS

    PERL_ARGS_ASSERT_PERL_CLONE;

   /* perlhost.h so we need to call into it
   to clone the host, CPerlHost should have a c interface, sky */

   if (flags & CLONEf_CLONE_HOST) {
       return perl_clone_host(proto_perl,flags);
   }
   return perl_clone_using(proto_perl, flags,
			    proto_perl->IMem,
			    proto_perl->IMemShared,
			    proto_perl->IMemParse,
			    proto_perl->IEnv,
			    proto_perl->IStdIO,
			    proto_perl->ILIO,
			    proto_perl->IDir,
			    proto_perl->ISock,
			    proto_perl->IProc);
}

PerlInterpreter *
perl_clone_using(PerlInterpreter *proto_perl, UV flags,
		 struct IPerlMem* ipM, struct IPerlMem* ipMS,
		 struct IPerlMem* ipMP, struct IPerlEnv* ipE,
		 struct IPerlStdIO* ipStd, struct IPerlLIO* ipLIO,
		 struct IPerlDir* ipD, struct IPerlSock* ipS,
		 struct IPerlProc* ipP)
{
    /* XXX many of the string copies here can be optimized if they're
     * constants; they need to be allocated as common memory and just
     * their pointers copied. */

    IV i;
    CLONE_PARAMS clone_params;
    CLONE_PARAMS* const param = &clone_params;

    PerlInterpreter * const my_perl = (PerlInterpreter*)(*ipM->pMalloc)(ipM, sizeof(PerlInterpreter));

    PERL_ARGS_ASSERT_PERL_CLONE_USING;
#else		/* !PERL_IMPLICIT_SYS */
    IV i;
    CLONE_PARAMS clone_params;
    CLONE_PARAMS* param = &clone_params;
    PerlInterpreter * const my_perl = (PerlInterpreter*)PerlMem_malloc(sizeof(PerlInterpreter));

    PERL_ARGS_ASSERT_PERL_CLONE;
#endif		/* PERL_IMPLICIT_SYS */

    /* for each stash, determine whether its objects should be cloned */
    S_visit(proto_perl, do_mark_cloneable_stash, SVt_PVHV, SVTYPEMASK);
    PERL_SET_THX(my_perl);

#ifdef DEBUGGING
    PoisonNew(my_perl, 1, PerlInterpreter);
    PL_op = NULL;
    PL_curcop = NULL;
    PL_defstash = NULL; /* may be used by perl malloc() */
    PL_markstack = 0;
    PL_scopestack = 0;
    PL_scopestack_name = 0;
    PL_savestack = 0;
    PL_savestack_ix = 0;
    PL_savestack_max = -1;
    PL_sig_pending = 0;
    PL_parser = NULL;
    Zero(&PL_debug_pad, 1, struct perl_debug_pad);
#  ifdef DEBUG_LEAKING_SCALARS
    PL_sv_serial = (((UV)my_perl >> 2) & 0xfff) * 1000000;
#  endif
#else	/* !DEBUGGING */
    Zero(my_perl, 1, PerlInterpreter);
#endif	/* DEBUGGING */

#ifdef PERL_IMPLICIT_SYS
    /* host pointers */
    PL_Mem		= ipM;
    PL_MemShared	= ipMS;
    PL_MemParse		= ipMP;
    PL_Env		= ipE;
    PL_StdIO		= ipStd;
    PL_LIO		= ipLIO;
    PL_Dir		= ipD;
    PL_Sock		= ipS;
    PL_Proc		= ipP;
#endif		/* PERL_IMPLICIT_SYS */

    param->flags = flags;
    /* Nothing in the core code uses this, but we make it available to
       extensions (using mg_dup).  */
    param->proto_perl = proto_perl;
    /* Likely nothing will use this, but it is initialised to be consistent
       with Perl_clone_params_new().  */
    param->new_perl = my_perl;
    param->unreferenced = NULL;

    INIT_TRACK_MEMPOOL(my_perl->Imemory_debug_header, my_perl);

    PL_body_arenas = NULL;
    Zero(&PL_body_roots, 1, PL_body_roots);
    
    PL_sv_count		= 0;
    PL_sv_objcount	= 0;
    PL_sv_root		= NULL;
    PL_sv_arenaroot	= NULL;

    PL_debug		= proto_perl->Idebug;

    PL_hash_seed	= proto_perl->Ihash_seed;
    PL_rehash_seed	= proto_perl->Irehash_seed;

    SvANY(&PL_sv_undef)		= NULL;
    SvREFCNT(&PL_sv_undef)	= (~(U32)0)/2;
    SvFLAGS(&PL_sv_undef)	= SVf_READONLY|SVt_NULL;
    SvREFCNT(&PL_sv_no)		= (~(U32)0)/2;
    SvFLAGS(&PL_sv_no)		= SVp_IOK|SVf_IOK|SVp_NOK|SVf_NOK
				  |SVp_POK|SVf_POK|SVf_READONLY|SVt_PVNV;

    SvANY(&PL_sv_yes)		= new_XPVNV();
    SvREFCNT(&PL_sv_yes)	= (~(U32)0)/2;
    SvFLAGS(&PL_sv_yes)		= SVp_IOK|SVf_IOK|SVp_NOK|SVf_NOK
				  |SVp_POK|SVf_POK|SVf_READONLY|SVt_PVNV;

    /* dbargs array probably holds garbage */
    PL_dbargs		= NULL;

    PL_compiling = proto_perl->Icompiling;

#ifdef PERL_DEBUG_READONLY_OPS
    PL_slabs = NULL;
    PL_slab_count = 0;
#endif

    /* pseudo environmental stuff */
    PL_origargc		= proto_perl->Iorigargc;
    PL_origargv		= proto_perl->Iorigargv;

    /* Set tainting stuff before PerlIO_debug can possibly get called */
    PL_tainting		= proto_perl->Itainting;
    PL_taint_warn	= proto_perl->Itaint_warn;

    PL_minus_c		= proto_perl->Iminus_c;

    PL_localpatches	= proto_perl->Ilocalpatches;
    PL_splitstr		= proto_perl->Isplitstr;
    PL_minus_n		= proto_perl->Iminus_n;
    PL_minus_p		= proto_perl->Iminus_p;
    PL_minus_l		= proto_perl->Iminus_l;
    PL_minus_a		= proto_perl->Iminus_a;
    PL_minus_E		= proto_perl->Iminus_E;
    PL_minus_F		= proto_perl->Iminus_F;
    PL_doswitches	= proto_perl->Idoswitches;
    PL_dowarn		= proto_perl->Idowarn;
    PL_sawampersand	= proto_perl->Isawampersand;
    PL_unsafe		= proto_perl->Iunsafe;
    PL_perldb		= proto_perl->Iperldb;
    PL_perl_destruct_level = proto_perl->Iperl_destruct_level;
    PL_exit_flags       = proto_perl->Iexit_flags;

    /* XXX time(&PL_basetime) when asked for? */
    PL_basetime		= proto_perl->Ibasetime;

    PL_maxsysfd		= proto_perl->Imaxsysfd;
    PL_statusvalue	= proto_perl->Istatusvalue;
#ifdef VMS
    PL_statusvalue_vms	= proto_perl->Istatusvalue_vms;
#else
    PL_statusvalue_posix = proto_perl->Istatusvalue_posix;
#endif

    /* RE engine related */
    Zero(&PL_reg_state, 1, struct re_save_state);
    PL_reginterp_cnt	= 0;
    PL_regmatch_slab	= NULL;

    PL_sub_generation	= proto_perl->Isub_generation;

    /* funky return mechanisms */
    PL_forkprocess	= proto_perl->Iforkprocess;

    /* internal state */
    PL_maxo		= proto_perl->Imaxo;

    PL_main_start	= proto_perl->Imain_start;
    PL_eval_root	= proto_perl->Ieval_root;
    PL_eval_start	= proto_perl->Ieval_start;

    PL_filemode		= proto_perl->Ifilemode;
    PL_lastfd		= proto_perl->Ilastfd;
    PL_oldname		= proto_perl->Ioldname;		/* XXX not quite right */
    PL_Argv		= NULL;
    PL_Cmd		= NULL;
    PL_gensym		= proto_perl->Igensym;

    PL_laststatval	= proto_perl->Ilaststatval;
    PL_laststype	= proto_perl->Ilaststype;
    PL_mess_sv		= NULL;

    PL_profiledata	= NULL;

    PL_generation	= proto_perl->Igeneration;

    PL_in_clean_objs	= proto_perl->Iin_clean_objs;
    PL_in_clean_all	= proto_perl->Iin_clean_all;

    PL_delaymagic_uid	= proto_perl->Idelaymagic_uid;
    PL_delaymagic_euid	= proto_perl->Idelaymagic_euid;
    PL_delaymagic_gid	= proto_perl->Idelaymagic_gid;
    PL_delaymagic_egid	= proto_perl->Idelaymagic_egid;
    PL_nomemok		= proto_perl->Inomemok;
    PL_an		= proto_perl->Ian;
    PL_evalseq		= proto_perl->Ievalseq;
    PL_origenviron	= proto_perl->Iorigenviron;	/* XXX not quite right */
    PL_origalen		= proto_perl->Iorigalen;

    PL_sighandlerp	= proto_perl->Isighandlerp;

    PL_runops		= proto_perl->Irunops;

    PL_subline		= proto_perl->Isubline;

#ifdef FCRYPT
    PL_cryptseen	= proto_perl->Icryptseen;
#endif

    PL_hints		= proto_perl->Ihints;

    PL_amagic_generation	= proto_perl->Iamagic_generation;

#ifdef USE_LOCALE_COLLATE
    PL_collation_ix	= proto_perl->Icollation_ix;
    PL_collation_standard	= proto_perl->Icollation_standard;
    PL_collxfrm_base	= proto_perl->Icollxfrm_base;
    PL_collxfrm_mult	= proto_perl->Icollxfrm_mult;
#endif /* USE_LOCALE_COLLATE */

#ifdef USE_LOCALE_NUMERIC
    PL_numeric_standard	= proto_perl->Inumeric_standard;
    PL_numeric_local	= proto_perl->Inumeric_local;
#endif /* !USE_LOCALE_NUMERIC */

    /* Did the locale setup indicate UTF-8? */
    PL_utf8locale	= proto_perl->Iutf8locale;
    /* Unicode features (see perlrun/-C) */
    PL_unicode		= proto_perl->Iunicode;

    /* Pre-5.8 signals control */
    PL_signals		= proto_perl->Isignals;

    /* times() ticks per second */
    PL_clocktick	= proto_perl->Iclocktick;

    /* Recursion stopper for PerlIO_find_layer */
    PL_in_load_module	= proto_perl->Iin_load_module;

    /* sort() routine */
    PL_sort_RealCmp	= proto_perl->Isort_RealCmp;

    /* Not really needed/useful since the reenrant_retint is "volatile",
     * but do it for consistency's sake. */
    PL_reentrant_retint	= proto_perl->Ireentrant_retint;

    /* Hooks to shared SVs and locks. */
    PL_sharehook	= proto_perl->Isharehook;
    PL_lockhook		= proto_perl->Ilockhook;
    PL_unlockhook	= proto_perl->Iunlockhook;
    PL_threadhook	= proto_perl->Ithreadhook;
    PL_destroyhook	= proto_perl->Idestroyhook;
    PL_signalhook	= proto_perl->Isignalhook;

    PL_globhook		= proto_perl->Iglobhook;

    /* swatch cache */
    PL_last_swash_hv	= NULL;	/* reinits on demand */
    PL_last_swash_klen	= 0;
    PL_last_swash_key[0]= '\0';
    PL_last_swash_tmps	= (U8*)NULL;
    PL_last_swash_slen	= 0;

    PL_glob_index	= proto_perl->Iglob_index;
    PL_srand_called	= proto_perl->Isrand_called;

    if (flags & CLONEf_COPY_STACKS) {
	/* next allocation will be PL_tmps_stack[PL_tmps_ix+1] */
	PL_tmps_ix		= proto_perl->Itmps_ix;
	PL_tmps_max		= proto_perl->Itmps_max;
	PL_tmps_floor		= proto_perl->Itmps_floor;

	/* next push_scope()/ENTER sets PL_scopestack[PL_scopestack_ix]
	 * NOTE: unlike the others! */
	PL_scopestack_ix	= proto_perl->Iscopestack_ix;
	PL_scopestack_max	= proto_perl->Iscopestack_max;

	/* next SSPUSHFOO() sets PL_savestack[PL_savestack_ix]
	 * NOTE: unlike the others! */
	PL_savestack_ix		= proto_perl->Isavestack_ix;
	PL_savestack_max	= proto_perl->Isavestack_max;
    }

    PL_start_env	= proto_perl->Istart_env;	/* XXXXXX */
    PL_top_env		= &PL_start_env;

    PL_op		= proto_perl->Iop;

    PL_Sv		= NULL;
    PL_Xpv		= (XPV*)NULL;
    my_perl->Ina	= proto_perl->Ina;

    PL_statbuf		= proto_perl->Istatbuf;
    PL_statcache	= proto_perl->Istatcache;

#ifdef HAS_TIMES
    PL_timesbuf		= proto_perl->Itimesbuf;
#endif

    PL_tainted		= proto_perl->Itainted;
    PL_curpm		= proto_perl->Icurpm;	/* XXX No PMOP ref count */

    PL_chopset		= proto_perl->Ichopset;	/* XXX never deallocated */

    PL_restartjmpenv	= proto_perl->Irestartjmpenv;
    PL_restartop	= proto_perl->Irestartop;
    PL_in_eval		= proto_perl->Iin_eval;
    PL_delaymagic	= proto_perl->Idelaymagic;
    PL_phase		= proto_perl->Iphase;
    PL_localizing	= proto_perl->Ilocalizing;

    PL_hv_fetch_ent_mh	= NULL;
    PL_modcount		= proto_perl->Imodcount;
    PL_lastgotoprobe	= NULL;
    PL_dumpindent	= proto_perl->Idumpindent;

    PL_efloatbuf	= NULL;		/* reinits on demand */
    PL_efloatsize	= 0;			/* reinits on demand */

    /* regex stuff */

    PL_regdummy		= proto_perl->Iregdummy;
    PL_colorset		= 0;		/* reinits PL_colors[] */
    /*PL_colors[6]	= {0,0,0,0,0,0};*/

    /* Pluggable optimizer */
    PL_peepp		= proto_perl->Ipeepp;
    PL_rpeepp		= proto_perl->Irpeepp;
    /* op_free() hook */
    PL_opfreehook	= proto_perl->Iopfreehook;

#ifdef USE_REENTRANT_API
    /* XXX: things like -Dm will segfault here in perlio, but doing
     *  PERL_SET_CONTEXT(proto_perl);
     * breaks too many other things
     */
    Perl_reentrant_init(aTHX);
#endif

    /* create SV map for pointer relocation */
    PL_ptr_table = ptr_table_new();

    /* initialize these special pointers as early as possible */
    ptr_table_store(PL_ptr_table, &proto_perl->Isv_undef, &PL_sv_undef);

    SvANY(&PL_sv_no)		= new_XPVNV();
    SvPV_set(&PL_sv_no, savepvn(PL_No, 0));
    SvCUR_set(&PL_sv_no, 0);
    SvLEN_set(&PL_sv_no, 1);
    SvIV_set(&PL_sv_no, 0);
    SvNV_set(&PL_sv_no, 0);
    ptr_table_store(PL_ptr_table, &proto_perl->Isv_no, &PL_sv_no);

    SvPV_set(&PL_sv_yes, savepvn(PL_Yes, 1));
    SvCUR_set(&PL_sv_yes, 1);
    SvLEN_set(&PL_sv_yes, 2);
    SvIV_set(&PL_sv_yes, 1);
    SvNV_set(&PL_sv_yes, 1);
    ptr_table_store(PL_ptr_table, &proto_perl->Isv_yes, &PL_sv_yes);

    /* create (a non-shared!) shared string table */
    PL_strtab		= newHV();
    HvSHAREKEYS_off(PL_strtab);
    hv_ksplit(PL_strtab, HvTOTALKEYS(proto_perl->Istrtab));
    ptr_table_store(PL_ptr_table, proto_perl->Istrtab, PL_strtab);

    /* These two PVs will be free'd special way so must set them same way op.c does */
    PL_compiling.cop_stashpv = savesharedpv(PL_compiling.cop_stashpv);
    ptr_table_store(PL_ptr_table, proto_perl->Icompiling.cop_stashpv, PL_compiling.cop_stashpv);

    PL_compiling.cop_file    = savesharedpv(PL_compiling.cop_file);
    ptr_table_store(PL_ptr_table, proto_perl->Icompiling.cop_file, PL_compiling.cop_file);

    ptr_table_store(PL_ptr_table, &proto_perl->Icompiling, &PL_compiling);
    PL_compiling.cop_warnings = DUP_WARNINGS(PL_compiling.cop_warnings);
    CopHINTHASH_set(&PL_compiling, cophh_copy(CopHINTHASH_get(&PL_compiling)));
    PL_curcop		= (COP*)any_dup(proto_perl->Icurcop, proto_perl);

    param->stashes      = newAV();  /* Setup array of objects to call clone on */
    /* This makes no difference to the implementation, as it always pushes
       and shifts pointers to other SVs without changing their reference
       count, with the array becoming empty before it is freed. However, it
       makes it conceptually clear what is going on, and will avoid some
       work inside av.c, filling slots between AvFILL() and AvMAX() with
       &PL_sv_undef, and SvREFCNT_dec()ing those.  */
    AvREAL_off(param->stashes);

    if (!(flags & CLONEf_COPY_STACKS)) {
	param->unreferenced = newAV();
    }

#ifdef PERLIO_LAYERS
    /* Clone PerlIO tables as soon as we can handle general xx_dup() */
    PerlIO_clone(aTHX_ proto_perl, param);
#endif

    PL_envgv		= gv_dup(proto_perl->Ienvgv, param);
    PL_incgv		= gv_dup(proto_perl->Iincgv, param);
    PL_hintgv		= gv_dup(proto_perl->Ihintgv, param);
    PL_origfilename	= SAVEPV(proto_perl->Iorigfilename);
    PL_diehook		= sv_dup_inc(proto_perl->Idiehook, param);
    PL_warnhook		= sv_dup_inc(proto_perl->Iwarnhook, param);

    /* switches */
    PL_patchlevel	= sv_dup_inc(proto_perl->Ipatchlevel, param);
    PL_apiversion	= sv_dup_inc(proto_perl->Iapiversion, param);
    PL_inplace		= SAVEPV(proto_perl->Iinplace);
    PL_e_script		= sv_dup_inc(proto_perl->Ie_script, param);

    /* magical thingies */
    PL_formfeed		= sv_dup(proto_perl->Iformfeed, param);

    PL_encoding		= sv_dup(proto_perl->Iencoding, param);

    sv_setpvs(PERL_DEBUG_PAD(0), "");	/* For regex debugging. */
    sv_setpvs(PERL_DEBUG_PAD(1), "");	/* ext/re needs these */
    sv_setpvs(PERL_DEBUG_PAD(2), "");	/* even without DEBUGGING. */

   
    /* Clone the regex array */
    /* ORANGE FIXME for plugins, probably in the SV dup code.
       newSViv(PTR2IV(CALLREGDUPE(
       INT2PTR(REGEXP *, SvIVX(regex)), param))))
    */
    PL_regex_padav = av_dup_inc(proto_perl->Iregex_padav, param);
    PL_regex_pad = AvARRAY(PL_regex_padav);

    /* shortcuts to various I/O objects */
    PL_ofsgv            = gv_dup_inc(proto_perl->Iofsgv, param);
    PL_stdingv		= gv_dup(proto_perl->Istdingv, param);
    PL_stderrgv		= gv_dup(proto_perl->Istderrgv, param);
    PL_defgv		= gv_dup(proto_perl->Idefgv, param);
    PL_argvgv		= gv_dup(proto_perl->Iargvgv, param);
    PL_argvoutgv	= gv_dup(proto_perl->Iargvoutgv, param);
    PL_argvout_stack	= av_dup_inc(proto_perl->Iargvout_stack, param);

    /* shortcuts to regexp stuff */
    PL_replgv		= gv_dup(proto_perl->Ireplgv, param);

    /* shortcuts to misc objects */
    PL_errgv		= gv_dup(proto_perl->Ierrgv, param);

    /* shortcuts to debugging objects */
    PL_DBgv		= gv_dup(proto_perl->IDBgv, param);
    PL_DBline		= gv_dup(proto_perl->IDBline, param);
    PL_DBsub		= gv_dup(proto_perl->IDBsub, param);
    PL_DBsingle		= sv_dup(proto_perl->IDBsingle, param);
    PL_DBtrace		= sv_dup(proto_perl->IDBtrace, param);
    PL_DBsignal		= sv_dup(proto_perl->IDBsignal, param);

    /* symbol tables */
    PL_defstash		= hv_dup_inc(proto_perl->Idefstash, param);
    PL_curstash		= hv_dup_inc(proto_perl->Icurstash, param);
    PL_debstash		= hv_dup(proto_perl->Idebstash, param);
    PL_globalstash	= hv_dup(proto_perl->Iglobalstash, param);
    PL_curstname	= sv_dup_inc(proto_perl->Icurstname, param);

    PL_beginav		= av_dup_inc(proto_perl->Ibeginav, param);
    PL_beginav_save	= av_dup_inc(proto_perl->Ibeginav_save, param);
    PL_checkav_save	= av_dup_inc(proto_perl->Icheckav_save, param);
    PL_unitcheckav      = av_dup_inc(proto_perl->Iunitcheckav, param);
    PL_unitcheckav_save = av_dup_inc(proto_perl->Iunitcheckav_save, param);
    PL_endav		= av_dup_inc(proto_perl->Iendav, param);
    PL_checkav		= av_dup_inc(proto_perl->Icheckav, param);
    PL_initav		= av_dup_inc(proto_perl->Iinitav, param);

    PL_isarev		= hv_dup_inc(proto_perl->Iisarev, param);

    /* subprocess state */
    PL_fdpid		= av_dup_inc(proto_perl->Ifdpid, param);

    if (proto_perl->Iop_mask)
	PL_op_mask	= SAVEPVN(proto_perl->Iop_mask, PL_maxo);
    else
	PL_op_mask 	= NULL;
    /* PL_asserting        = proto_perl->Iasserting; */

    /* current interpreter roots */
    PL_main_cv		= cv_dup_inc(proto_perl->Imain_cv, param);
    OP_REFCNT_LOCK;
    PL_main_root	= OpREFCNT_inc(proto_perl->Imain_root);
    OP_REFCNT_UNLOCK;

    /* runtime control stuff */
    PL_curcopdb		= (COP*)any_dup(proto_perl->Icurcopdb, proto_perl);

    PL_preambleav	= av_dup_inc(proto_perl->Ipreambleav, param);

    PL_ors_sv		= sv_dup_inc(proto_perl->Iors_sv, param);

    /* interpreter atexit processing */
    PL_exitlistlen	= proto_perl->Iexitlistlen;
    if (PL_exitlistlen) {
	Newx(PL_exitlist, PL_exitlistlen, PerlExitListEntry);
	Copy(proto_perl->Iexitlist, PL_exitlist, PL_exitlistlen, PerlExitListEntry);
    }
    else
	PL_exitlist	= (PerlExitListEntry*)NULL;

    PL_my_cxt_size = proto_perl->Imy_cxt_size;
    if (PL_my_cxt_size) {
	Newx(PL_my_cxt_list, PL_my_cxt_size, void *);
	Copy(proto_perl->Imy_cxt_list, PL_my_cxt_list, PL_my_cxt_size, void *);
#ifdef PERL_GLOBAL_STRUCT_PRIVATE
	Newx(PL_my_cxt_keys, PL_my_cxt_size, const char *);
	Copy(proto_perl->Imy_cxt_keys, PL_my_cxt_keys, PL_my_cxt_size, char *);
#endif
    }
    else {
	PL_my_cxt_list	= (void**)NULL;
#ifdef PERL_GLOBAL_STRUCT_PRIVATE
	PL_my_cxt_keys	= (const char**)NULL;
#endif
    }
    PL_modglobal	= hv_dup_inc(proto_perl->Imodglobal, param);
    PL_custom_op_names  = hv_dup_inc(proto_perl->Icustom_op_names,param);
    PL_custom_op_descs  = hv_dup_inc(proto_perl->Icustom_op_descs,param);
    PL_custom_ops	= hv_dup_inc(proto_perl->Icustom_ops, param);

    PL_compcv			= cv_dup(proto_perl->Icompcv, param);

    PAD_CLONE_VARS(proto_perl, param);

#ifdef HAVE_INTERP_INTERN
    sys_intern_dup(&proto_perl->Isys_intern, &PL_sys_intern);
#endif

    PL_DBcv		= cv_dup(proto_perl->IDBcv, param);

#ifdef PERL_USES_PL_PIDSTATUS
    PL_pidstatus	= newHV();			/* XXX flag for cloning? */
#endif
    PL_osname		= SAVEPV(proto_perl->Iosname);
    PL_parser		= parser_dup(proto_perl->Iparser, param);

    /* XXX this only works if the saved cop has already been cloned */
    if (proto_perl->Iparser) {
	PL_parser->saved_curcop = (COP*)any_dup(
				    proto_perl->Iparser->saved_curcop,
				    proto_perl);
    }

    PL_subname		= sv_dup_inc(proto_perl->Isubname, param);

#ifdef USE_LOCALE_COLLATE
    PL_collation_name	= SAVEPV(proto_perl->Icollation_name);
#endif /* USE_LOCALE_COLLATE */

#ifdef USE_LOCALE_NUMERIC
    PL_numeric_name	= SAVEPV(proto_perl->Inumeric_name);
    PL_numeric_radix_sv	= sv_dup_inc(proto_perl->Inumeric_radix_sv, param);
#endif /* !USE_LOCALE_NUMERIC */

    /* Unicode inversion lists */
    PL_ASCII		= sv_dup_inc(proto_perl->IASCII, param);
    PL_Latin1		= sv_dup_inc(proto_perl->ILatin1, param);

    PL_PerlSpace	= sv_dup_inc(proto_perl->IPerlSpace, param);
    PL_XPerlSpace	= sv_dup_inc(proto_perl->IXPerlSpace, param);

    PL_L1PosixAlnum	= sv_dup_inc(proto_perl->IL1PosixAlnum, param);
    PL_PosixAlnum	= sv_dup_inc(proto_perl->IPosixAlnum, param);

    PL_L1PosixAlpha	= sv_dup_inc(proto_perl->IL1PosixAlpha, param);
    PL_PosixAlpha	= sv_dup_inc(proto_perl->IPosixAlpha, param);

    PL_PosixBlank	= sv_dup_inc(proto_perl->IPosixBlank, param);
    PL_XPosixBlank	= sv_dup_inc(proto_perl->IXPosixBlank, param);

    PL_L1Cased		= sv_dup_inc(proto_perl->IL1Cased, param);

    PL_PosixCntrl	= sv_dup_inc(proto_perl->IPosixCntrl, param);
    PL_XPosixCntrl	= sv_dup_inc(proto_perl->IXPosixCntrl, param);

    PL_PosixDigit	= sv_dup_inc(proto_perl->IPosixDigit, param);

    PL_L1PosixGraph	= sv_dup_inc(proto_perl->IL1PosixGraph, param);
    PL_PosixGraph	= sv_dup_inc(proto_perl->IPosixGraph, param);

    PL_L1PosixLower	= sv_dup_inc(proto_perl->IL1PosixLower, param);
    PL_PosixLower	= sv_dup_inc(proto_perl->IPosixLower, param);

    PL_L1PosixPrint	= sv_dup_inc(proto_perl->IL1PosixPrint, param);
    PL_PosixPrint	= sv_dup_inc(proto_perl->IPosixPrint, param);

    PL_L1PosixPunct	= sv_dup_inc(proto_perl->IL1PosixPunct, param);
    PL_PosixPunct	= sv_dup_inc(proto_perl->IPosixPunct, param);

    PL_PosixSpace	= sv_dup_inc(proto_perl->IPosixSpace, param);
    PL_XPosixSpace	= sv_dup_inc(proto_perl->IXPosixSpace, param);

    PL_L1PosixUpper	= sv_dup_inc(proto_perl->IL1PosixUpper, param);
    PL_PosixUpper	= sv_dup_inc(proto_perl->IPosixUpper, param);

    PL_L1PosixWord	= sv_dup_inc(proto_perl->IL1PosixWord, param);
    PL_PosixWord	= sv_dup_inc(proto_perl->IPosixWord, param);

    PL_PosixXDigit	= sv_dup_inc(proto_perl->IPosixXDigit, param);
    PL_XPosixXDigit	= sv_dup_inc(proto_perl->IXPosixXDigit, param);

    PL_VertSpace	= sv_dup_inc(proto_perl->IVertSpace, param);

    /* utf8 character class swashes */
    PL_utf8_alnum	= sv_dup_inc(proto_perl->Iutf8_alnum, param);
    PL_utf8_alpha	= sv_dup_inc(proto_perl->Iutf8_alpha, param);
    PL_utf8_space	= sv_dup_inc(proto_perl->Iutf8_space, param);
    PL_utf8_graph	= sv_dup_inc(proto_perl->Iutf8_graph, param);
    PL_utf8_digit	= sv_dup_inc(proto_perl->Iutf8_digit, param);
    PL_utf8_upper	= sv_dup_inc(proto_perl->Iutf8_upper, param);
    PL_utf8_lower	= sv_dup_inc(proto_perl->Iutf8_lower, param);
    PL_utf8_print	= sv_dup_inc(proto_perl->Iutf8_print, param);
    PL_utf8_punct	= sv_dup_inc(proto_perl->Iutf8_punct, param);
    PL_utf8_xdigit	= sv_dup_inc(proto_perl->Iutf8_xdigit, param);
    PL_utf8_mark	= sv_dup_inc(proto_perl->Iutf8_mark, param);
    PL_utf8_X_begin	= sv_dup_inc(proto_perl->Iutf8_X_begin, param);
    PL_utf8_X_extend	= sv_dup_inc(proto_perl->Iutf8_X_extend, param);
    PL_utf8_X_prepend	= sv_dup_inc(proto_perl->Iutf8_X_prepend, param);
    PL_utf8_X_non_hangul	= sv_dup_inc(proto_perl->Iutf8_X_non_hangul, param);
    PL_utf8_X_L	= sv_dup_inc(proto_perl->Iutf8_X_L, param);
    PL_utf8_X_LV	= sv_dup_inc(proto_perl->Iutf8_X_LV, param);
    PL_utf8_X_LVT	= sv_dup_inc(proto_perl->Iutf8_X_LVT, param);
    PL_utf8_X_T	= sv_dup_inc(proto_perl->Iutf8_X_T, param);
    PL_utf8_X_V	= sv_dup_inc(proto_perl->Iutf8_X_V, param);
    PL_utf8_X_LV_LVT_V	= sv_dup_inc(proto_perl->Iutf8_X_LV_LVT_V, param);
    PL_utf8_toupper	= sv_dup_inc(proto_perl->Iutf8_toupper, param);
    PL_utf8_totitle	= sv_dup_inc(proto_perl->Iutf8_totitle, param);
    PL_utf8_tolower	= sv_dup_inc(proto_perl->Iutf8_tolower, param);
    PL_utf8_tofold	= sv_dup_inc(proto_perl->Iutf8_tofold, param);
    PL_utf8_idstart	= sv_dup_inc(proto_perl->Iutf8_idstart, param);
    PL_utf8_xidstart	= sv_dup_inc(proto_perl->Iutf8_xidstart, param);
    PL_utf8_perl_idstart = sv_dup_inc(proto_perl->Iutf8_perl_idstart, param);
    PL_utf8_idcont	= sv_dup_inc(proto_perl->Iutf8_idcont, param);
    PL_utf8_xidcont	= sv_dup_inc(proto_perl->Iutf8_xidcont, param);
    PL_utf8_foldable	= sv_dup_inc(proto_perl->Iutf8_foldable, param);
    PL_utf8_quotemeta	= sv_dup_inc(proto_perl->Iutf8_quotemeta, param);
    PL_ASCII		= sv_dup_inc(proto_perl->IASCII, param);
    PL_AboveLatin1	= sv_dup_inc(proto_perl->IAboveLatin1, param);
    PL_Latin1		= sv_dup_inc(proto_perl->ILatin1, param);


    if (proto_perl->Ipsig_pend) {
	Newxz(PL_psig_pend, SIG_SIZE, int);
    }
    else {
	PL_psig_pend	= (int*)NULL;
    }

    if (proto_perl->Ipsig_name) {
	Newx(PL_psig_name, 2 * SIG_SIZE, SV*);
	sv_dup_inc_multiple(proto_perl->Ipsig_name, PL_psig_name, 2 * SIG_SIZE,
			    param);
	PL_psig_ptr = PL_psig_name + SIG_SIZE;
    }
    else {
	PL_psig_ptr	= (SV**)NULL;
	PL_psig_name	= (SV**)NULL;
    }

    if (flags & CLONEf_COPY_STACKS) {
	Newx(PL_tmps_stack, PL_tmps_max, SV*);
	sv_dup_inc_multiple(proto_perl->Itmps_stack, PL_tmps_stack,
			    PL_tmps_ix+1, param);

	/* next PUSHMARK() sets *(PL_markstack_ptr+1) */
	i = proto_perl->Imarkstack_max - proto_perl->Imarkstack;
	Newxz(PL_markstack, i, I32);
	PL_markstack_max	= PL_markstack + (proto_perl->Imarkstack_max
						  - proto_perl->Imarkstack);
	PL_markstack_ptr	= PL_markstack + (proto_perl->Imarkstack_ptr
						  - proto_perl->Imarkstack);
	Copy(proto_perl->Imarkstack, PL_markstack,
	     PL_markstack_ptr - PL_markstack + 1, I32);

	/* next push_scope()/ENTER sets PL_scopestack[PL_scopestack_ix]
	 * NOTE: unlike the others! */
	Newxz(PL_scopestack, PL_scopestack_max, I32);
	Copy(proto_perl->Iscopestack, PL_scopestack, PL_scopestack_ix, I32);

#ifdef DEBUGGING
	Newxz(PL_scopestack_name, PL_scopestack_max, const char *);
	Copy(proto_perl->Iscopestack_name, PL_scopestack_name, PL_scopestack_ix, const char *);
#endif
	/* NOTE: si_dup() looks at PL_markstack */
	PL_curstackinfo		= si_dup(proto_perl->Icurstackinfo, param);

	/* PL_curstack		= PL_curstackinfo->si_stack; */
	PL_curstack		= av_dup(proto_perl->Icurstack, param);
	PL_mainstack		= av_dup(proto_perl->Imainstack, param);

	/* next PUSHs() etc. set *(PL_stack_sp+1) */
	PL_stack_base		= AvARRAY(PL_curstack);
	PL_stack_sp		= PL_stack_base + (proto_perl->Istack_sp
						   - proto_perl->Istack_base);
	PL_stack_max		= PL_stack_base + AvMAX(PL_curstack);

	/*Newxz(PL_savestack, PL_savestack_max, ANY);*/
	PL_savestack		= ss_dup(proto_perl, param);
    }
    else {
	init_stacks();
	ENTER;			/* perl_destruct() wants to LEAVE; */
    }

    PL_statgv		= gv_dup(proto_perl->Istatgv, param);
    PL_statname		= sv_dup_inc(proto_perl->Istatname, param);

    PL_rs		= sv_dup_inc(proto_perl->Irs, param);
    PL_last_in_gv	= gv_dup(proto_perl->Ilast_in_gv, param);
    PL_defoutgv		= gv_dup_inc(proto_perl->Idefoutgv, param);
    PL_toptarget	= sv_dup_inc(proto_perl->Itoptarget, param);
    PL_bodytarget	= sv_dup_inc(proto_perl->Ibodytarget, param);
    PL_formtarget	= sv_dup(proto_perl->Iformtarget, param);

    PL_errors		= sv_dup_inc(proto_perl->Ierrors, param);

    PL_sortcop		= (OP*)any_dup(proto_perl->Isortcop, proto_perl);
    PL_sortstash	= hv_dup(proto_perl->Isortstash, param);
    PL_firstgv		= gv_dup(proto_perl->Ifirstgv, param);
    PL_secondgv		= gv_dup(proto_perl->Isecondgv, param);

    PL_stashcache       = newHV();

    PL_watchaddr	= (char **) ptr_table_fetch(PL_ptr_table,
					    proto_perl->Iwatchaddr);
    PL_watchok		= PL_watchaddr ? * PL_watchaddr : NULL;
    if (PL_debug && PL_watchaddr) {
	PerlIO_printf(Perl_debug_log,
	  "WATCHING: %"UVxf" cloned as %"UVxf" with value %"UVxf"\n",
	  PTR2UV(proto_perl->Iwatchaddr), PTR2UV(PL_watchaddr),
	  PTR2UV(PL_watchok));
    }

    PL_registered_mros  = hv_dup_inc(proto_perl->Iregistered_mros, param);
    PL_blockhooks	= av_dup_inc(proto_perl->Iblockhooks, param);
    PL_utf8_foldclosures = hv_dup_inc(proto_perl->Iutf8_foldclosures, param);

    /* Call the ->CLONE method, if it exists, for each of the stashes
       identified by sv_dup() above.
    */
    while(av_len(param->stashes) != -1) {
	HV* const stash = MUTABLE_HV(av_shift(param->stashes));
	GV* const cloner = gv_fetchmethod_autoload(stash, "CLONE", 0);
	if (cloner && GvCV(cloner)) {
	    dSP;
	    ENTER;
	    SAVETMPS;
	    PUSHMARK(SP);
	    mXPUSHs(newSVhek(HvNAME_HEK(stash)));
	    PUTBACK;
	    call_sv(MUTABLE_SV(GvCV(cloner)), G_DISCARD);
	    FREETMPS;
	    LEAVE;
	}
    }

    if (!(flags & CLONEf_KEEP_PTR_TABLE)) {
        ptr_table_free(PL_ptr_table);
        PL_ptr_table = NULL;
    }

    if (!(flags & CLONEf_COPY_STACKS)) {
	unreferenced_to_tmp_stack(param->unreferenced);
    }

    SvREFCNT_dec(param->stashes);

    /* orphaned? eg threads->new inside BEGIN or use */
    if (PL_compcv && ! SvREFCNT(PL_compcv)) {
	SvREFCNT_inc_simple_void(PL_compcv);
	SAVEFREESV(PL_compcv);
    }

    return my_perl;
}

static void
S_unreferenced_to_tmp_stack(pTHX_ AV *const unreferenced)
{
    PERL_ARGS_ASSERT_UNREFERENCED_TO_TMP_STACK;
    
    if (AvFILLp(unreferenced) > -1) {
	SV **svp = AvARRAY(unreferenced);
	SV **const last = svp + AvFILLp(unreferenced);
	SSize_t count = 0;

	do {
	    if (SvREFCNT(*svp) == 1)
		++count;
	} while (++svp <= last);

	EXTEND_MORTAL(count);
	svp = AvARRAY(unreferenced);

	do {
	    if (SvREFCNT(*svp) == 1) {
		/* Our reference is the only one to this SV. This means that
		   in this thread, the scalar effectively has a 0 reference.
		   That doesn't work (cleanup never happens), so donate our
		   reference to it onto the save stack. */
		PL_tmps_stack[++PL_tmps_ix] = *svp;
	    } else {
		/* As an optimisation, because we are already walking the
		   entire array, instead of above doing either
		   SvREFCNT_inc(*svp) or *svp = &PL_sv_undef, we can instead
		   release our reference to the scalar, so that at the end of
		   the array owns zero references to the scalars it happens to
		   point to. We are effectively converting the array from
		   AvREAL() on to AvREAL() off. This saves the av_clear()
		   (triggered by the SvREFCNT_dec(unreferenced) below) from
		   walking the array a second time.  */
		SvREFCNT_dec(*svp);
	    }

	} while (++svp <= last);
	AvREAL_off(unreferenced);
    }
    SvREFCNT_dec(unreferenced);
}

void
Perl_clone_params_del(CLONE_PARAMS *param)
{
    /* This seemingly funky ordering keeps the build with PERL_GLOBAL_STRUCT
       happy: */
    PerlInterpreter *const to = param->new_perl;
    dTHXa(to);
    PerlInterpreter *const was = PERL_GET_THX;

    PERL_ARGS_ASSERT_CLONE_PARAMS_DEL;

    if (was != to) {
	PERL_SET_THX(to);
    }

    SvREFCNT_dec(param->stashes);
    if (param->unreferenced)
	unreferenced_to_tmp_stack(param->unreferenced);

    Safefree(param);

    if (was != to) {
	PERL_SET_THX(was);
    }
}

CLONE_PARAMS *
Perl_clone_params_new(PerlInterpreter *const from, PerlInterpreter *const to)
{
    dVAR;
    /* Need to play this game, as newAV() can call safesysmalloc(), and that
       does a dTHX; to get the context from thread local storage.
       FIXME - under PERL_CORE Newx(), Safefree() and friends should expand to
       a version that passes in my_perl.  */
    PerlInterpreter *const was = PERL_GET_THX;
    CLONE_PARAMS *param;

    PERL_ARGS_ASSERT_CLONE_PARAMS_NEW;

    if (was != to) {
	PERL_SET_THX(to);
    }

    /* Given that we've set the context, we can do this unshared.  */
    Newx(param, 1, CLONE_PARAMS);

    param->flags = 0;
    param->proto_perl = from;
    param->new_perl = to;
    param->stashes = (AV *)Perl_newSV_type(to, SVt_PVAV);
    AvREAL_off(param->stashes);
    param->unreferenced = (AV *)Perl_newSV_type(to, SVt_PVAV);

    if (was != to) {
	PERL_SET_THX(was);
    }
    return param;
}

#endif /* USE_ITHREADS */

/*
=head1 Unicode Support

=for apidoc sv_recode_to_utf8

The encoding is assumed to be an Encode object, on entry the PV
of the sv is assumed to be octets in that encoding, and the sv
will be converted into Unicode (and UTF-8).

If the sv already is UTF-8 (or if it is not POK), or if the encoding
is not a reference, nothing is done to the sv.  If the encoding is not
an C<Encode::XS> Encoding object, bad things will happen.
(See F<lib/encoding.pm> and L<Encode>.)

The PV of the sv is returned.

=cut */

char *
Perl_sv_recode_to_utf8(pTHX_ SV *sv, SV *encoding)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_RECODE_TO_UTF8;

    if (SvPOK(sv) && !SvUTF8(sv) && !IN_BYTES && SvROK(encoding)) {
	SV *uni;
	STRLEN len;
	const char *s;
	dSP;
	ENTER;
	SAVETMPS;
	save_re_context();
	PUSHMARK(sp);
	EXTEND(SP, 3);
	XPUSHs(encoding);
	XPUSHs(sv);
/*
  NI-S 2002/07/09
  Passing sv_yes is wrong - it needs to be or'ed set of constants
  for Encode::XS, while UTf-8 decode (currently) assumes a true value means
  remove converted chars from source.

  Both will default the value - let them.

	XPUSHs(&PL_sv_yes);
*/
	PUTBACK;
	call_method("decode", G_SCALAR);
	SPAGAIN;
	uni = POPs;
	PUTBACK;
	s = SvPV_const(uni, len);
	if (s != SvPVX_const(sv)) {
	    SvGROW(sv, len + 1);
	    Move(s, SvPVX(sv), len + 1, char);
	    SvCUR_set(sv, len);
	}
	FREETMPS;
	LEAVE;
	if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
	    /* clear pos and any utf8 cache */
	    MAGIC * mg = mg_find(sv, PERL_MAGIC_regex_global);
	    if (mg)
		mg->mg_len = -1;
	    if ((mg = mg_find(sv, PERL_MAGIC_utf8)))
		magic_setutf8(sv,mg); /* clear UTF8 cache */
	}
	SvUTF8_on(sv);
	return SvPVX(sv);
    }
    return SvPOKp(sv) ? SvPVX(sv) : NULL;
}

/*
=for apidoc sv_cat_decode

The encoding is assumed to be an Encode object, the PV of the ssv is
assumed to be octets in that encoding and decoding the input starts
from the position which (PV + *offset) pointed to.  The dsv will be
concatenated the decoded UTF-8 string from ssv.  Decoding will terminate
when the string tstr appears in decoding output or the input ends on
the PV of the ssv.  The value which the offset points will be modified
to the last input position on the ssv.

Returns TRUE if the terminator was found, else returns FALSE.

=cut */

bool
Perl_sv_cat_decode(pTHX_ SV *dsv, SV *encoding,
		   SV *ssv, int *offset, char *tstr, int tlen)
{
    dVAR;
    bool ret = FALSE;

    PERL_ARGS_ASSERT_SV_CAT_DECODE;

    if (SvPOK(ssv) && SvPOK(dsv) && SvROK(encoding) && offset) {
	SV *offsv;
	dSP;
	ENTER;
	SAVETMPS;
	save_re_context();
	PUSHMARK(sp);
	EXTEND(SP, 6);
	XPUSHs(encoding);
	XPUSHs(dsv);
	XPUSHs(ssv);
	offsv = newSViv(*offset);
	mXPUSHs(offsv);
	mXPUSHp(tstr, tlen);
	PUTBACK;
	call_method("cat_decode", G_SCALAR);
	SPAGAIN;
	ret = SvTRUE(TOPs);
	*offset = SvIV(offsv);
	PUTBACK;
	FREETMPS;
	LEAVE;
    }
    else
        Perl_croak(aTHX_ "Invalid argument to sv_cat_decode");
    return ret;

}

/* ---------------------------------------------------------------------
 *
 * support functions for report_uninit()
 */

/* the maxiumum size of array or hash where we will scan looking
 * for the undefined element that triggered the warning */

#define FUV_MAX_SEARCH_SIZE 1000

/* Look for an entry in the hash whose value has the same SV as val;
 * If so, return a mortal copy of the key. */

STATIC SV*
S_find_hash_subscript(pTHX_ const HV *const hv, const SV *const val)
{
    dVAR;
    register HE **array;
    I32 i;

    PERL_ARGS_ASSERT_FIND_HASH_SUBSCRIPT;

    if (!hv || SvMAGICAL(hv) || !HvARRAY(hv) ||
			(HvTOTALKEYS(hv) > FUV_MAX_SEARCH_SIZE))
	return NULL;

    array = HvARRAY(hv);

    for (i=HvMAX(hv); i>0; i--) {
	register HE *entry;
	for (entry = array[i]; entry; entry = HeNEXT(entry)) {
	    if (HeVAL(entry) != val)
		continue;
	    if (    HeVAL(entry) == &PL_sv_undef ||
		    HeVAL(entry) == &PL_sv_placeholder)
		continue;
	    if (!HeKEY(entry))
		return NULL;
	    if (HeKLEN(entry) == HEf_SVKEY)
		return sv_mortalcopy(HeKEY_sv(entry));
	    return sv_2mortal(newSVhek(HeKEY_hek(entry)));
	}
    }
    return NULL;
}

/* Look for an entry in the array whose value has the same SV as val;
 * If so, return the index, otherwise return -1. */

STATIC I32
S_find_array_subscript(pTHX_ const AV *const av, const SV *const val)
{
    dVAR;

    PERL_ARGS_ASSERT_FIND_ARRAY_SUBSCRIPT;

    if (!av || SvMAGICAL(av) || !AvARRAY(av) ||
			(AvFILLp(av) > FUV_MAX_SEARCH_SIZE))
	return -1;

    if (val != &PL_sv_undef) {
	SV ** const svp = AvARRAY(av);
	I32 i;

	for (i=AvFILLp(av); i>=0; i--)
	    if (svp[i] == val)
		return i;
    }
    return -1;
}

/* S_varname(): return the name of a variable, optionally with a subscript.
 * If gv is non-zero, use the name of that global, along with gvtype (one
 * of "$", "@", "%"); otherwise use the name of the lexical at pad offset
 * targ.  Depending on the value of the subscript_type flag, return:
 */

#define FUV_SUBSCRIPT_NONE	1	/* "@foo"          */
#define FUV_SUBSCRIPT_ARRAY	2	/* "$foo[aindex]"  */
#define FUV_SUBSCRIPT_HASH	3	/* "$foo{keyname}" */
#define FUV_SUBSCRIPT_WITHIN	4	/* "within @foo"   */

SV*
Perl_varname(pTHX_ const GV *const gv, const char gvtype, PADOFFSET targ,
	const SV *const keyname, I32 aindex, int subscript_type)
{

    SV * const name = sv_newmortal();
    if (gv && isGV(gv)) {
	char buffer[2];
	buffer[0] = gvtype;
	buffer[1] = 0;

	/* as gv_fullname4(), but add literal '^' for $^FOO names  */

	gv_fullname4(name, gv, buffer, 0);

	if ((unsigned int)SvPVX(name)[1] <= 26) {
	    buffer[0] = '^';
	    buffer[1] = SvPVX(name)[1] + 'A' - 1;

	    /* Swap the 1 unprintable control character for the 2 byte pretty
	       version - ie substr($name, 1, 1) = $buffer; */
	    sv_insert(name, 1, 1, buffer, 2);
	}
    }
    else {
	CV * const cv = gv ? (CV *)gv : find_runcv(NULL);
	SV *sv;
	AV *av;

	assert(!cv || SvTYPE(cv) == SVt_PVCV);

	if (!cv || !CvPADLIST(cv))
	    return NULL;
	av = MUTABLE_AV((*av_fetch(CvPADLIST(cv), 0, FALSE)));
	sv = *av_fetch(av, targ, FALSE);
	sv_setsv(name, sv);
    }

    if (subscript_type == FUV_SUBSCRIPT_HASH) {
	SV * const sv = newSV(0);
	*SvPVX(name) = '$';
	Perl_sv_catpvf(aTHX_ name, "{%s}",
	    pv_pretty(sv, SvPVX_const(keyname), SvCUR(keyname), 32, NULL, NULL,
		    PERL_PV_PRETTY_DUMP | PERL_PV_ESCAPE_UNI_DETECT ));
	SvREFCNT_dec(sv);
    }
    else if (subscript_type == FUV_SUBSCRIPT_ARRAY) {
	*SvPVX(name) = '$';
	Perl_sv_catpvf(aTHX_ name, "[%"IVdf"]", (IV)aindex);
    }
    else if (subscript_type == FUV_SUBSCRIPT_WITHIN) {
	/* We know that name has no magic, so can use 0 instead of SV_GMAGIC */
	Perl_sv_insert_flags(aTHX_ name, 0, 0,  STR_WITH_LEN("within "), 0);
    }

    return name;
}


/*
=for apidoc find_uninit_var

Find the name of the undefined variable (if any) that caused the operator
to issue a "Use of uninitialized value" warning.
If match is true, only return a name if its value matches uninit_sv.
So roughly speaking, if a unary operator (such as OP_COS) generates a
warning, then following the direct child of the op may yield an
OP_PADSV or OP_GV that gives the name of the undefined variable.  On the
other hand, with OP_ADD there are two branches to follow, so we only print
the variable name if we get an exact match.

The name is returned as a mortal SV.

Assumes that PL_op is the op that originally triggered the error, and that
PL_comppad/PL_curpad points to the currently executing pad.

=cut
*/

STATIC SV *
S_find_uninit_var(pTHX_ const OP *const obase, const SV *const uninit_sv,
		  bool match)
{
    dVAR;
    SV *sv;
    const GV *gv;
    const OP *o, *o2, *kid;

    if (!obase || (match && (!uninit_sv || uninit_sv == &PL_sv_undef ||
			    uninit_sv == &PL_sv_placeholder)))
	return NULL;

    switch (obase->op_type) {

    case OP_RV2AV:
    case OP_RV2HV:
    case OP_PADAV:
    case OP_PADHV:
      {
	const bool pad  = (obase->op_type == OP_PADAV || obase->op_type == OP_PADHV);
	const bool hash = (obase->op_type == OP_PADHV || obase->op_type == OP_RV2HV);
	I32 index = 0;
	SV *keysv = NULL;
	int subscript_type = FUV_SUBSCRIPT_WITHIN;

	if (pad) { /* @lex, %lex */
	    sv = PAD_SVl(obase->op_targ);
	    gv = NULL;
	}
	else {
	    if (cUNOPx(obase)->op_first->op_type == OP_GV) {
	    /* @global, %global */
		gv = cGVOPx_gv(cUNOPx(obase)->op_first);
		if (!gv)
		    break;
		sv = hash ? MUTABLE_SV(GvHV(gv)): MUTABLE_SV(GvAV(gv));
	    }
	    else if (obase == PL_op) /* @{expr}, %{expr} */
		return find_uninit_var(cUNOPx(obase)->op_first,
						    uninit_sv, match);
	    else /* @{expr}, %{expr} as a sub-expression */
		return NULL;
	}

	/* attempt to find a match within the aggregate */
	if (hash) {
	    keysv = find_hash_subscript((const HV*)sv, uninit_sv);
	    if (keysv)
		subscript_type = FUV_SUBSCRIPT_HASH;
	}
	else {
	    index = find_array_subscript((const AV *)sv, uninit_sv);
	    if (index >= 0)
		subscript_type = FUV_SUBSCRIPT_ARRAY;
	}

	if (match && subscript_type == FUV_SUBSCRIPT_WITHIN)
	    break;

	return varname(gv, hash ? '%' : '@', obase->op_targ,
				    keysv, index, subscript_type);
      }

    case OP_RV2SV:
	if (cUNOPx(obase)->op_first->op_type == OP_GV) {
	    /* $global */
	    gv = cGVOPx_gv(cUNOPx(obase)->op_first);
	    if (!gv || !GvSTASH(gv))
		break;
	    if (match && (GvSV(gv) != uninit_sv))
		break;
	    return varname(gv, '$', 0, NULL, 0, FUV_SUBSCRIPT_NONE);
	}
	/* ${expr} */
	return find_uninit_var(cUNOPx(obase)->op_first, uninit_sv, 1);

    case OP_PADSV:
	if (match && PAD_SVl(obase->op_targ) != uninit_sv)
	    break;
	return varname(NULL, '$', obase->op_targ,
				    NULL, 0, FUV_SUBSCRIPT_NONE);

    case OP_GVSV:
	gv = cGVOPx_gv(obase);
	if (!gv || (match && GvSV(gv) != uninit_sv) || !GvSTASH(gv))
	    break;
	return varname(gv, '$', 0, NULL, 0, FUV_SUBSCRIPT_NONE);

    case OP_AELEMFAST_LEX:
	if (match) {
	    SV **svp;
	    AV *av = MUTABLE_AV(PAD_SV(obase->op_targ));
	    if (!av || SvRMAGICAL(av))
		break;
	    svp = av_fetch(av, (I32)obase->op_private, FALSE);
	    if (!svp || *svp != uninit_sv)
		break;
	}
	return varname(NULL, '$', obase->op_targ,
		       NULL, (I32)obase->op_private, FUV_SUBSCRIPT_ARRAY);
    case OP_AELEMFAST:
	{
	    gv = cGVOPx_gv(obase);
	    if (!gv)
		break;
	    if (match) {
		SV **svp;
		AV *const av = GvAV(gv);
		if (!av || SvRMAGICAL(av))
		    break;
		svp = av_fetch(av, (I32)obase->op_private, FALSE);
		if (!svp || *svp != uninit_sv)
		    break;
	    }
	    return varname(gv, '$', 0,
		    NULL, (I32)obase->op_private, FUV_SUBSCRIPT_ARRAY);
	}
	break;

    case OP_EXISTS:
	o = cUNOPx(obase)->op_first;
	if (!o || o->op_type != OP_NULL ||
		! (o->op_targ == OP_AELEM || o->op_targ == OP_HELEM))
	    break;
	return find_uninit_var(cBINOPo->op_last, uninit_sv, match);

    case OP_AELEM:
    case OP_HELEM:
    {
	bool negate = FALSE;

	if (PL_op == obase)
	    /* $a[uninit_expr] or $h{uninit_expr} */
	    return find_uninit_var(cBINOPx(obase)->op_last, uninit_sv, match);

	gv = NULL;
	o = cBINOPx(obase)->op_first;
	kid = cBINOPx(obase)->op_last;

	/* get the av or hv, and optionally the gv */
	sv = NULL;
	if  (o->op_type == OP_PADAV || o->op_type == OP_PADHV) {
	    sv = PAD_SV(o->op_targ);
	}
	else if ((o->op_type == OP_RV2AV || o->op_type == OP_RV2HV)
		&& cUNOPo->op_first->op_type == OP_GV)
	{
	    gv = cGVOPx_gv(cUNOPo->op_first);
	    if (!gv)
		break;
	    sv = o->op_type
		== OP_RV2HV ? MUTABLE_SV(GvHV(gv)) : MUTABLE_SV(GvAV(gv));
	}
	if (!sv)
	    break;

	if (kid && kid->op_type == OP_NEGATE) {
	    negate = TRUE;
	    kid = cUNOPx(kid)->op_first;
	}

	if (kid && kid->op_type == OP_CONST && SvOK(cSVOPx_sv(kid))) {
	    /* index is constant */
	    SV* kidsv;
	    if (negate) {
		kidsv = sv_2mortal(newSVpvs("-"));
		sv_catsv(kidsv, cSVOPx_sv(kid));
	    }
	    else
		kidsv = cSVOPx_sv(kid);
	    if (match) {
		if (SvMAGICAL(sv))
		    break;
		if (obase->op_type == OP_HELEM) {
		    HE* he = hv_fetch_ent(MUTABLE_HV(sv), kidsv, 0, 0);
		    if (!he || HeVAL(he) != uninit_sv)
			break;
		}
		else {
		    SV * const * const svp = av_fetch(MUTABLE_AV(sv),
			negate ? - SvIV(cSVOPx_sv(kid)) : SvIV(cSVOPx_sv(kid)),
			FALSE);
		    if (!svp || *svp != uninit_sv)
			break;
		}
	    }
	    if (obase->op_type == OP_HELEM)
		return varname(gv, '%', o->op_targ,
			    kidsv, 0, FUV_SUBSCRIPT_HASH);
	    else
		return varname(gv, '@', o->op_targ, NULL,
		    negate ? - SvIV(cSVOPx_sv(kid)) : SvIV(cSVOPx_sv(kid)),
		    FUV_SUBSCRIPT_ARRAY);
	}
	else  {
	    /* index is an expression;
	     * attempt to find a match within the aggregate */
	    if (obase->op_type == OP_HELEM) {
		SV * const keysv = find_hash_subscript((const HV*)sv, uninit_sv);
		if (keysv)
		    return varname(gv, '%', o->op_targ,
						keysv, 0, FUV_SUBSCRIPT_HASH);
	    }
	    else {
		const I32 index
		    = find_array_subscript((const AV *)sv, uninit_sv);
		if (index >= 0)
		    return varname(gv, '@', o->op_targ,
					NULL, index, FUV_SUBSCRIPT_ARRAY);
	    }
	    if (match)
		break;
	    return varname(gv,
		(o->op_type == OP_PADAV || o->op_type == OP_RV2AV)
		? '@' : '%',
		o->op_targ, NULL, 0, FUV_SUBSCRIPT_WITHIN);
	}
	break;
    }

    case OP_AASSIGN:
	/* only examine RHS */
	return find_uninit_var(cBINOPx(obase)->op_first, uninit_sv, match);

    case OP_OPEN:
	o = cUNOPx(obase)->op_first;
	if (o->op_type == OP_PUSHMARK)
	    o = o->op_sibling;

	if (!o->op_sibling) {
	    /* one-arg version of open is highly magical */

	    if (o->op_type == OP_GV) { /* open FOO; */
		gv = cGVOPx_gv(o);
		if (match && GvSV(gv) != uninit_sv)
		    break;
		return varname(gv, '$', 0,
			    NULL, 0, FUV_SUBSCRIPT_NONE);
	    }
	    /* other possibilities not handled are:
	     * open $x; or open my $x;	should return '${*$x}'
	     * open expr;		should return '$'.expr ideally
	     */
	     break;
	}
	goto do_op;

    /* ops where $_ may be an implicit arg */
    case OP_TRANS:
    case OP_TRANSR:
    case OP_SUBST:
    case OP_MATCH:
	if ( !(obase->op_flags & OPf_STACKED)) {
	    if (uninit_sv == ((obase->op_private & OPpTARGET_MY)
				 ? PAD_SVl(obase->op_targ)
				 : DEFSV))
	    {
		sv = sv_newmortal();
		sv_setpvs(sv, "$_");
		return sv;
	    }
	}
	goto do_op;

    case OP_PRTF:
    case OP_PRINT:
    case OP_SAY:
	match = 1; /* print etc can return undef on defined args */
	/* skip filehandle as it can't produce 'undef' warning  */
	o = cUNOPx(obase)->op_first;
	if ((obase->op_flags & OPf_STACKED) && o->op_type == OP_PUSHMARK)
	    o = o->op_sibling->op_sibling;
	goto do_op2;


    case OP_ENTEREVAL: /* could be eval $undef or $x='$undef'; eval $x */
    case OP_CUSTOM: /* XS or custom code could trigger random warnings */

	/* the following ops are capable of returning PL_sv_undef even for
	 * defined arg(s) */

    case OP_BACKTICK:
    case OP_PIPE_OP:
    case OP_FILENO:
    case OP_BINMODE:
    case OP_TIED:
    case OP_GETC:
    case OP_SYSREAD:
    case OP_SEND:
    case OP_IOCTL:
    case OP_SOCKET:
    case OP_SOCKPAIR:
    case OP_BIND:
    case OP_CONNECT:
    case OP_LISTEN:
    case OP_ACCEPT:
    case OP_SHUTDOWN:
    case OP_SSOCKOPT:
    case OP_GETPEERNAME:
    case OP_FTRREAD:
    case OP_FTRWRITE:
    case OP_FTREXEC:
    case OP_FTROWNED:
    case OP_FTEREAD:
    case OP_FTEWRITE:
    case OP_FTEEXEC:
    case OP_FTEOWNED:
    case OP_FTIS:
    case OP_FTZERO:
    case OP_FTSIZE:
    case OP_FTFILE:
    case OP_FTDIR:
    case OP_FTLINK:
    case OP_FTPIPE:
    case OP_FTSOCK:
    case OP_FTBLK:
    case OP_FTCHR:
    case OP_FTTTY:
    case OP_FTSUID:
    case OP_FTSGID:
    case OP_FTSVTX:
    case OP_FTTEXT:
    case OP_FTBINARY:
    case OP_FTMTIME:
    case OP_FTATIME:
    case OP_FTCTIME:
    case OP_READLINK:
    case OP_OPEN_DIR:
    case OP_READDIR:
    case OP_TELLDIR:
    case OP_SEEKDIR:
    case OP_REWINDDIR:
    case OP_CLOSEDIR:
    case OP_GMTIME:
    case OP_ALARM:
    case OP_SEMGET:
    case OP_GETLOGIN:
    case OP_UNDEF:
    case OP_SUBSTR:
    case OP_AEACH:
    case OP_EACH:
    case OP_SORT:
    case OP_CALLER:
    case OP_DOFILE:
    case OP_PROTOTYPE:
    case OP_NCMP:
    case OP_SMARTMATCH:
    case OP_UNPACK:
    case OP_SYSOPEN:
    case OP_SYSSEEK:
	match = 1;
	goto do_op;

    case OP_ENTERSUB:
    case OP_GOTO:
	/* XXX tmp hack: these two may call an XS sub, and currently
	  XS subs don't have a SUB entry on the context stack, so CV and
	  pad determination goes wrong, and BAD things happen. So, just
	  don't try to determine the value under those circumstances.
	  Need a better fix at dome point. DAPM 11/2007 */
	break;

    case OP_FLIP:
    case OP_FLOP:
    {
	GV * const gv = gv_fetchpvs(".", GV_NOTQUAL, SVt_PV);
	if (gv && GvSV(gv) == uninit_sv)
	    return newSVpvs_flags("$.", SVs_TEMP);
	goto do_op;
    }

    case OP_POS:
	/* def-ness of rval pos() is independent of the def-ness of its arg */
	if ( !(obase->op_flags & OPf_MOD))
	    break;

    case OP_SCHOMP:
    case OP_CHOMP:
	if (SvROK(PL_rs) && uninit_sv == SvRV(PL_rs))
	    return newSVpvs_flags("${$/}", SVs_TEMP);
	/*FALLTHROUGH*/

    default:
    do_op:
	if (!(obase->op_flags & OPf_KIDS))
	    break;
	o = cUNOPx(obase)->op_first;
	
    do_op2:
	if (!o)
	    break;

	/* This loop checks all the kid ops, skipping any that cannot pos-
	 * sibly be responsible for the uninitialized value; i.e., defined
	 * constants and ops that return nothing.  If there is only one op
	 * left that is not skipped, then we *know* it is responsible for
	 * the uninitialized value.  If there is more than one op left, we
	 * have to look for an exact match in the while() loop below.
	 */
	o2 = NULL;
	for (kid=o; kid; kid = kid->op_sibling) {
	    if (kid) {
		const OPCODE type = kid->op_type;
		if ( (type == OP_CONST && SvOK(cSVOPx_sv(kid)))
		  || (type == OP_NULL  && ! (kid->op_flags & OPf_KIDS))
		  || (type == OP_PUSHMARK)
		)
		continue;
	    }
	    if (o2) { /* more than one found */
		o2 = NULL;
		break;
	    }
	    o2 = kid;
	}
	if (o2)
	    return find_uninit_var(o2, uninit_sv, match);

	/* scan all args */
	while (o) {
	    sv = find_uninit_var(o, uninit_sv, 1);
	    if (sv)
		return sv;
	    o = o->op_sibling;
	}
	break;
    }
    return NULL;
}


/*
=for apidoc report_uninit

Print appropriate "Use of uninitialized variable" warning.

=cut
*/

void
Perl_report_uninit(pTHX_ const SV *uninit_sv)
{
    dVAR;
    if (PL_op) {
	SV* varname = NULL;
	if (uninit_sv && PL_curpad) {
	    varname = find_uninit_var(PL_op, uninit_sv,0);
	    if (varname)
		sv_insert(varname, 0, 0, " ", 1);
	}
	/* diag_listed_as: Use of uninitialized value%s */
	Perl_warner(aTHX_ packWARN(WARN_UNINITIALIZED), PL_warn_uninit_sv,
		SVfARG(varname ? varname : &PL_sv_no),
		" in ", OP_DESC(PL_op));
    }
    else
	Perl_warner(aTHX_ packWARN(WARN_UNINITIALIZED), PL_warn_uninit,
		    "", "", "");
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
