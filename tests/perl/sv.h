/*    sv.h
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifdef sv_flags
#undef sv_flags		/* Convex has this in <signal.h> for sigvec() */
#endif

/*
=head1 SV Flags

=for apidoc AmU||svtype
An enum of flags for Perl types.  These are found in the file B<sv.h>
in the C<svtype> enum.  Test these flags with the C<SvTYPE> macro.

=for apidoc AmU||SVt_PV
Pointer type flag for scalars.  See C<svtype>.

=for apidoc AmU||SVt_IV
Integer type flag for scalars.  See C<svtype>.

=for apidoc AmU||SVt_NV
Double type flag for scalars.  See C<svtype>.

=for apidoc AmU||SVt_PVMG
Type flag for blessed scalars.  See C<svtype>.

=for apidoc AmU||SVt_PVAV
Type flag for arrays.  See C<svtype>.

=for apidoc AmU||SVt_PVHV
Type flag for hashes.  See C<svtype>.

=for apidoc AmU||SVt_PVCV
Type flag for code refs.  See C<svtype>.

=cut
*/

typedef enum {
	SVt_NULL,	/* 0 */
	SVt_BIND,	/* 1 */
	SVt_IV,		/* 2 */
	SVt_NV,		/* 3 */
	/* RV was here, before it was merged with IV.  */
	SVt_PV,		/* 4 */
	SVt_PVIV,	/* 5 */
	SVt_PVNV,	/* 6 */
	SVt_PVMG,	/* 7 */
	SVt_REGEXP,	/* 8 */
	/* PVBM was here, before BIND replaced it.  */
	SVt_PVGV,	/* 9 */
	SVt_PVLV,	/* 10 */
	SVt_PVAV,	/* 11 */
	SVt_PVHV,	/* 12 */
	SVt_PVCV,	/* 13 */
	SVt_PVFM,	/* 14 */
	SVt_PVIO,	/* 15 */
	SVt_LAST	/* keep last in enum. used to size arrays */
} svtype;

/* *** any alterations to the SV types above need to be reflected in
 * SVt_MASK and the various PL_valid_types_* tables */

#define SVt_MASK 0xf	/* smallest bitmask that covers all types */

#ifndef PERL_CORE
/* Although Fast Boyer Moore tables are now being stored in PVGVs, for most
   purposes eternal code wanting to consider PVBM probably needs to think of
   PVMG instead.  */
#  define SVt_PVBM	SVt_PVMG
/* Anything wanting to create a reference from clean should ensure that it has
   a scalar of type SVt_IV now:  */
#  define SVt_RV	SVt_IV
#endif

/* There is collusion here with sv_clear - sv_clear exits early for SVt_NULL
   so never reaches the clause at the end that uses sv_type_details->body_size
   to determine whether to call safefree(). Hence body_size can be set
   non-zero to record the size of HEs, without fear of bogus frees.  */
#if defined(PERL_IN_HV_C) || defined(PERL_IN_XS_APITEST)
#define HE_SVSLOT	SVt_NULL
#endif

#define PERL_ARENA_ROOTS_SIZE	(SVt_LAST)

/* typedefs to eliminate some typing */
typedef struct he HE;
typedef struct hek HEK;

/* Using C's structural equivalence to help emulate C++ inheritance here... */

/* start with 2 sv-head building blocks */
#define _SV_HEAD(ptrtype) \
    ptrtype	sv_any;		/* pointer to body */	\
    U32		sv_refcnt;	/* how many references to us */	\
    U32		sv_flags	/* what we are */

#define _SV_HEAD_UNION \
    union {				\
	char*   svu_pv;		/* pointer to malloced string */	\
	IV      svu_iv;			\
	UV      svu_uv;			\
	SV*     svu_rv;		/* pointer to another SV */		\
	SV**    svu_array;		\
	HE**	svu_hash;		\
	GP*	svu_gp;			\
	PerlIO *svu_fp;			\
    }	sv_u


struct STRUCT_SV {		/* struct sv { */
    _SV_HEAD(void*);
    _SV_HEAD_UNION;
#ifdef DEBUG_LEAKING_SCALARS
    PERL_BITFIELD32 sv_debug_optype:9;	/* the type of OP that allocated us */
    PERL_BITFIELD32 sv_debug_inpad:1;	/* was allocated in a pad for an OP */
    PERL_BITFIELD32 sv_debug_line:16;	/* the line where we were allocated */
    UV		    sv_debug_serial;	/* serial number of sv allocation   */
    char *	    sv_debug_file;	/* the file where we were allocated */
    SV *	    sv_debug_parent;	/* what we were cloned from (ithreads)*/
#endif
};

struct gv {
    _SV_HEAD(XPVGV*);		/* pointer to xpvgv body */
    _SV_HEAD_UNION;
};

struct cv {
    _SV_HEAD(XPVCV*);		/* pointer to xpvcv body */
    _SV_HEAD_UNION;
};

struct av {
    _SV_HEAD(XPVAV*);		/* pointer to xpvav body */
    _SV_HEAD_UNION;
};

struct hv {
    _SV_HEAD(XPVHV*);		/* pointer to xpvhv body */
    _SV_HEAD_UNION;
};

struct io {
    _SV_HEAD(XPVIO*);		/* pointer to xpvio body */
    _SV_HEAD_UNION;
};

struct p5rx {
    _SV_HEAD(struct regexp*);	/* pointer to regexp body */
    _SV_HEAD_UNION;
};

#undef _SV_HEAD
#undef _SV_HEAD_UNION		/* ensure no pollution */

/*
=head1 SV Manipulation Functions

=for apidoc Am|U32|SvREFCNT|SV* sv
Returns the value of the object's reference count.

=for apidoc Am|SV*|SvREFCNT_inc|SV* sv
Increments the reference count of the given SV.

All of the following SvREFCNT_inc* macros are optimized versions of
SvREFCNT_inc, and can be replaced with SvREFCNT_inc.

=for apidoc Am|SV*|SvREFCNT_inc_NN|SV* sv
Same as SvREFCNT_inc, but can only be used if you know I<sv>
is not NULL.  Since we don't have to check the NULLness, it's faster
and smaller.

=for apidoc Am|void|SvREFCNT_inc_void|SV* sv
Same as SvREFCNT_inc, but can only be used if you don't need the
return value.  The macro doesn't need to return a meaningful value.

=for apidoc Am|void|SvREFCNT_inc_void_NN|SV* sv
Same as SvREFCNT_inc, but can only be used if you don't need the return
value, and you know that I<sv> is not NULL.  The macro doesn't need
to return a meaningful value, or check for NULLness, so it's smaller
and faster.

=for apidoc Am|SV*|SvREFCNT_inc_simple|SV* sv
Same as SvREFCNT_inc, but can only be used with expressions without side
effects.  Since we don't have to store a temporary value, it's faster.

=for apidoc Am|SV*|SvREFCNT_inc_simple_NN|SV* sv
Same as SvREFCNT_inc_simple, but can only be used if you know I<sv>
is not NULL.  Since we don't have to check the NULLness, it's faster
and smaller.

=for apidoc Am|void|SvREFCNT_inc_simple_void|SV* sv
Same as SvREFCNT_inc_simple, but can only be used if you don't need the
return value.  The macro doesn't need to return a meaningful value.

=for apidoc Am|void|SvREFCNT_inc_simple_void_NN|SV* sv
Same as SvREFCNT_inc, but can only be used if you don't need the return
value, and you know that I<sv> is not NULL.  The macro doesn't need
to return a meaningful value, or check for NULLness, so it's smaller
and faster.

=for apidoc Am|void|SvREFCNT_dec|SV* sv
Decrements the reference count of the given SV.

=for apidoc Am|svtype|SvTYPE|SV* sv
Returns the type of the SV.  See C<svtype>.

=for apidoc Am|void|SvUPGRADE|SV* sv|svtype type
Used to upgrade an SV to a more complex form.  Uses C<sv_upgrade> to
perform the upgrade if necessary.  See C<svtype>.

=cut
*/

#define SvANY(sv)	(sv)->sv_any
#define SvFLAGS(sv)	(sv)->sv_flags
#define SvREFCNT(sv)	(sv)->sv_refcnt

#if defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
#  define SvREFCNT_inc(sv)		\
    ({					\
	SV * const _sv = MUTABLE_SV(sv);	\
	if (_sv)			\
	     (SvREFCNT(_sv))++;		\
	_sv;				\
    })
#  define SvREFCNT_inc_simple(sv)	\
    ({					\
	if (sv)				\
	     (SvREFCNT(sv))++;		\
	MUTABLE_SV(sv);				\
    })
#  define SvREFCNT_inc_NN(sv)		\
    ({					\
	SV * const _sv = MUTABLE_SV(sv);	\
	SvREFCNT(_sv)++;		\
	_sv;				\
    })
#  define SvREFCNT_inc_void(sv)		\
    ({					\
	SV * const _sv = MUTABLE_SV(sv);	\
	if (_sv)			\
	    (void)(SvREFCNT(_sv)++);	\
    })
#else
#  define SvREFCNT_inc(sv)	\
	((PL_Sv=MUTABLE_SV(sv)) ? (++(SvREFCNT(PL_Sv)),PL_Sv) : NULL)
#  define SvREFCNT_inc_simple(sv) \
	((sv) ? (SvREFCNT(sv)++,MUTABLE_SV(sv)) : NULL)
#  define SvREFCNT_inc_NN(sv) \
	(PL_Sv=MUTABLE_SV(sv),++(SvREFCNT(PL_Sv)),PL_Sv)
#  define SvREFCNT_inc_void(sv) \
	(void)((PL_Sv=MUTABLE_SV(sv)) ? ++(SvREFCNT(PL_Sv)) : 0)
#endif

/* These guys don't need the curly blocks */
#define SvREFCNT_inc_simple_void(sv)	STMT_START { if (sv) SvREFCNT(sv)++; } STMT_END
#define SvREFCNT_inc_simple_NN(sv)	(++(SvREFCNT(sv)),MUTABLE_SV(sv))
#define SvREFCNT_inc_void_NN(sv)	(void)(++SvREFCNT(MUTABLE_SV(sv)))
#define SvREFCNT_inc_simple_void_NN(sv)	(void)(++SvREFCNT(MUTABLE_SV(sv)))

#if defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
#  define SvREFCNT_dec(sv)		\
    ({					\
	SV * const _sv = MUTABLE_SV(sv);	\
	if (_sv) {			\
	    if (SvREFCNT(_sv)) {	\
		if (--(SvREFCNT(_sv)) == 0) \
		    Perl_sv_free2(aTHX_ _sv);	\
	    } else {			\
		sv_free(_sv);		\
	    }				\
	}				\
    })
#else
#define SvREFCNT_dec(sv)	sv_free(MUTABLE_SV(sv))
#endif

#define SVTYPEMASK	0xff
#define SvTYPE(sv)	((svtype)((sv)->sv_flags & SVTYPEMASK))

/* Sadly there are some parts of the core that have pointers to already-freed
   SV heads, and rely on being able to tell that they are now free. So mark
   them all by using a consistent macro.  */
#define SvIS_FREED(sv)	((sv)->sv_flags == SVTYPEMASK)

#define SvUPGRADE(sv, mt) (SvTYPE(sv) >= (mt) || (sv_upgrade(sv, mt), 1))

#define SVf_IOK		0x00000100  /* has valid public integer value */
#define SVf_NOK		0x00000200  /* has valid public numeric value */
#define SVf_POK		0x00000400  /* has valid public pointer value */
#define SVf_ROK		0x00000800  /* has a valid reference pointer */

#define SVp_IOK		0x00001000  /* has valid non-public integer value */
#define SVp_NOK		0x00002000  /* has valid non-public numeric value */
#define SVp_POK		0x00004000  /* has valid non-public pointer value */
#define SVp_SCREAM	0x00008000  /* has been studied? */
#define SVphv_CLONEABLE	SVp_SCREAM  /* PVHV (stashes) clone its objects */
#define SVpgv_GP	SVp_SCREAM  /* GV has a valid GP */
#define SVprv_PCS_IMPORTED  SVp_SCREAM  /* RV is a proxy for a constant
				       subroutine in another package. Set the
				       CvIMPORTED_CV_ON() if it needs to be
				       expanded to a real GV */
/*                      0x00010000  *** FREE SLOT */
#define SVs_PADTMP	0x00020000  /* in use as tmp; only if ! SVs_PADMY */
#define SVs_PADSTALE	0x00020000  /* lexical has gone out of scope;
					only valid for SVs_PADMY */
#define SVpad_TYPED	0x00020000  /* pad name is a Typed Lexical */
#define SVs_PADMY	0x00040000  /* in use a "my" variable */
#define SVpad_OUR	0x00040000  /* pad name is "our" instead of "my" */
#define SVs_TEMP	0x00080000  /* string is stealable? */
#define SVs_OBJECT	0x00100000  /* is "blessed" */
#define SVs_GMG		0x00200000  /* has magical get method */
#define SVs_SMG		0x00400000  /* has magical set method */
#define SVs_RMG		0x00800000  /* has random magical methods */

#define SVf_FAKE	0x01000000  /* 0: glob is just a copy
				       1: SV head arena wasn't malloc()ed
				       2: in conjunction with SVf_READONLY
					  marks a shared hash key scalar
					  (SvLEN == 0) or a copy on write
					  string (SvLEN != 0) [SvIsCOW(sv)]
				       3: For PVCV, whether CvUNIQUE(cv)
					  refers to an eval or once only
					  [CvEVAL(cv), CvSPECIAL(cv)]
				       4: On a pad name SV, that slot in the
					  frame AV is a REFCNT'ed reference
					  to a lexical from "outside". */
#define SVphv_REHASH	SVf_FAKE    /* 5: On a PVHV, hash values are being
					  recalculated */
#define SVf_OOK		0x02000000  /* has valid offset value. For a PVHV this
				       means that a hv_aux struct is present
				       after the main array */
#define SVf_BREAK	0x04000000  /* refcnt is artificially low - used by
				       SVs in final arena cleanup.
				       Set in S_regtry on PL_reg_curpm, so that
				       perl_destruct will skip it. */
#define SVf_READONLY	0x08000000  /* may not be modified */




#define SVf_THINKFIRST	(SVf_READONLY|SVf_ROK|SVf_FAKE)

#define SVf_OK		(SVf_IOK|SVf_NOK|SVf_POK|SVf_ROK| \
			 SVp_IOK|SVp_NOK|SVp_POK|SVpgv_GP)

#define PRIVSHIFT 4	/* (SVp_?OK >> PRIVSHIFT) == SVf_?OK */

#define SVf_AMAGIC	0x10000000  /* has magical overloaded methods */

/* Ensure this value does not clash with the GV_ADD* flags in gv.h: */
#define SVf_UTF8        0x20000000  /* SvPV is UTF-8 encoded
				       This is also set on RVs whose overloaded
				       stringification is UTF-8. This might
				       only happen as a side effect of SvPV() */
					   

/* Some private flags. */

/* PVAV could probably use 0x2000000 without conflict. I assume that PVFM can
   be UTF-8 encoded, and PVCVs could well have UTF-8 prototypes. PVIOs haven't
   been restructured, so sometimes get used as string buffers.  */

/* PVHV */
#define SVphv_SHAREKEYS 0x20000000  /* PVHV keys live on shared string table */

/* PVNV, PVMG only, and only used in pads. Should be safe to test on any scalar
   SV, as the core is careful to avoid setting both.

   SVf_POK, SVp_POK also set:
   0x00004400   Normal
   0x0000C400   Studied (SvSCREAM)
   0x40004400   FBM compiled (SvVALID)
   0x4000C400   pad name.

   0x00008000   GV with GP
   0x00008800   RV with PCS imported
*/
#define SVpad_NAME	(SVp_SCREAM|SVpbm_VALID)
				    /* This SV is a name in the PAD, so
				       SVpad_TYPED, SVpad_OUR and SVpad_STATE
				       apply */
/* PVAV */
#define SVpav_REAL	0x40000000  /* free old entries */
/* PVHV */
#define SVphv_LAZYDEL	0x40000000  /* entry in xhv_eiter must be deleted */
/* This is only set true on a PVGV when it's playing "PVBM", but is tested for
   on any regular scalar (anything <= PVLV) */
#define SVpbm_VALID	0x40000000
/* Only used in toke.c on an SV stored in PL_lex_repl */
#define SVrepl_EVAL	0x40000000  /* Replacement part of s///e */

/* IV, PVIV, PVNV, PVMG, PVGV and (I assume) PVLV  */
#define SVf_IVisUV	0x80000000  /* use XPVUV instead of XPVIV */
/* PVAV */
#define SVpav_REIFY 	0x80000000  /* can become real */
/* PVHV */
#define SVphv_HASKFLAGS	0x80000000  /* keys have flag byte after hash */
/* PVGV when SVpbm_VALID is true */
#define SVpbm_TAIL	0x80000000
/* RV upwards. However, SVf_ROK and SVp_IOK are exclusive  */
#define SVprv_WEAKREF   0x80000000  /* Weak reference */
/* pad name vars only */
#define SVpad_STATE	0x80000000  /* pad name is a "state" var */

#define _XPV_HEAD							\
    HV*		xmg_stash;	/* class package */			\
    union _xmgu	xmg_u;							\
    STRLEN	xpv_cur;	/* length of svu_pv as a C string */    \
    STRLEN	xpv_len 	/* allocated size */

union _xnvu {
    NV	    xnv_nv;		/* numeric value, if any */
    HV *    xgv_stash;
    struct {
	U32 xlow;
	U32 xhigh;
    }	    xpad_cop_seq;	/* used by pad.c for cop_sequence */
    struct {
	I32 xbm_useful;
	U8  xbm_rare;		/* rarest character in string */
    }	    xbm_s;		/* fields from PVBM */
};

union _xivu {
    IV	    xivu_iv;		/* integer value */
    UV	    xivu_uv;
    HEK *   xivu_namehek;	/* xpvlv, xpvgv: GvNAME */
};

union _xmgu {
    MAGIC*  xmg_magic;		/* linked list of magicalness */
    HV*	    xmg_ourstash;	/* Stash for our (when SvPAD_OUR is true) */
    STRLEN  xmg_hash_index;	/* used while freeing hash entries */
};

struct xpv {
    _XPV_HEAD;
};

struct xpviv {
    _XPV_HEAD;
    union _xivu xiv_u;
};

#define xiv_iv xiv_u.xivu_iv

struct xpvuv {
    _XPV_HEAD;
    union _xivu xuv_u;
};

#define xuv_uv xuv_u.xivu_uv

struct xpvnv {
    _XPV_HEAD;
    union _xivu xiv_u;
    union _xnvu xnv_u;
};

/* This structure must match the beginning of struct xpvhv in hv.h. */
struct xpvmg {
    _XPV_HEAD;
    union _xivu xiv_u;
    union _xnvu xnv_u;
};

struct xpvlv {
    _XPV_HEAD;
    union _xivu xiv_u;
    union _xnvu xnv_u;
    STRLEN	xlv_targoff;
    STRLEN	xlv_targlen;
    SV*		xlv_targ;
    char	xlv_type;	/* k=keys .=pos x=substr v=vec /=join/re
				 * y=alem/helem/iter t=tie T=tied HE */
    char	xlv_flags;	/* 1 = negative offset  2 = negative len */
};

/* This structure works in 3 ways - regular scalar, GV with GP, or fast
   Boyer-Moore.  */
struct xpvgv {
    _XPV_HEAD;
    union _xivu xiv_u;
    union _xnvu xnv_u;
};

/* This structure must match XPVCV in cv.h */

typedef U16 cv_flags_t;

#define _XPVCV_COMMON								\
    HV *	xcv_stash;							\
    union {									\
	OP *	xcv_start;							\
	ANY	xcv_xsubany;							\
    }		xcv_start_u;					    		\
    union {									\
	OP *	xcv_root;							\
	void	(*xcv_xsub) (pTHX_ CV*);					\
    }		xcv_root_u;							\
    GV *	xcv_gv;								\
    char *	xcv_file;							\
    AV *	xcv_padlist;							\
    CV *	xcv_outside;							\
    U32		xcv_outside_seq; /* the COP sequence (at the point of our	\
				  * compilation) in the lexically enclosing	\
				  * sub */					\
    cv_flags_t	xcv_flags

struct xpvfm {
    _XPV_HEAD;
    _XPVCV_COMMON;
    IV		xfm_lines;
};


struct xpvio {
    _XPV_HEAD;
    union _xivu xiv_u;
    /* ifp and ofp are normally the same, but sockets need separate streams */
    PerlIO *	xio_ofp;
    /* Cray addresses everything by word boundaries (64 bits) and
     * code and data pointers cannot be mixed (which is exactly what
     * Perl_filter_add() tries to do with the dirp), hence the
     *  following union trick (as suggested by Gurusamy Sarathy).
     * For further information see Geir Johansen's problem report
     * titled [ID 20000612.002] Perl problem on Cray system
     * The any pointer (known as IoANY()) will also be a good place
     * to hang any IO disciplines to.
     */
    union {
	DIR *	xiou_dirp;	/* for opendir, readdir, etc */
	void *	xiou_any;	/* for alignment */
    } xio_dirpu;
    /* IV xio_lines is now in IVX  $. */
    IV		xio_page;	/* $% */
    IV		xio_page_len;	/* $= */
    IV		xio_lines_left;	/* $- */
    char *	xio_top_name;	/* $^ */
    GV *	xio_top_gv;	/* $^ */
    char *	xio_fmt_name;	/* $~ */
    GV *	xio_fmt_gv;	/* $~ */
    char *	xio_bottom_name;/* $^B */
    GV *	xio_bottom_gv;	/* $^B */
    char	xio_type;
    U8		xio_flags;
};

#define xio_dirp	xio_dirpu.xiou_dirp
#define xio_any		xio_dirpu.xiou_any

#define IOf_ARGV	1	/* this fp iterates over ARGV */
#define IOf_START	2	/* check for null ARGV and substitute '-' */
#define IOf_FLUSH	4	/* this fp wants a flush after write op */
#define IOf_DIDTOP	8	/* just did top of form */
#define IOf_UNTAINT	16	/* consider this fp (and its data) "safe" */
#define IOf_NOLINE	32	/* slurped a pseudo-line from empty file */
#define IOf_FAKE_DIRP	64	/* xio_dirp is fake (source filters kludge)
				   Also, when this is set, SvPVX() is valid */

/* The following macros define implementation-independent predicates on SVs. */

/*
=for apidoc Am|U32|SvNIOK|SV* sv
Returns a U32 value indicating whether the SV contains a number, integer or
double.

=for apidoc Am|U32|SvNIOKp|SV* sv
Returns a U32 value indicating whether the SV contains a number, integer or
double.  Checks the B<private> setting.  Use C<SvNIOK> instead.

=for apidoc Am|void|SvNIOK_off|SV* sv
Unsets the NV/IV status of an SV.

=for apidoc Am|U32|SvOK|SV* sv
Returns a U32 value indicating whether the value is defined. This is
only meaningful for scalars.

=for apidoc Am|U32|SvIOKp|SV* sv
Returns a U32 value indicating whether the SV contains an integer.  Checks
the B<private> setting.  Use C<SvIOK> instead.

=for apidoc Am|U32|SvNOKp|SV* sv
Returns a U32 value indicating whether the SV contains a double.  Checks the
B<private> setting.  Use C<SvNOK> instead.

=for apidoc Am|U32|SvPOKp|SV* sv
Returns a U32 value indicating whether the SV contains a character string.
Checks the B<private> setting.  Use C<SvPOK> instead.

=for apidoc Am|U32|SvIOK|SV* sv
Returns a U32 value indicating whether the SV contains an integer.

=for apidoc Am|void|SvIOK_on|SV* sv
Tells an SV that it is an integer.

=for apidoc Am|void|SvIOK_off|SV* sv
Unsets the IV status of an SV.

=for apidoc Am|void|SvIOK_only|SV* sv
Tells an SV that it is an integer and disables all other OK bits.

=for apidoc Am|void|SvIOK_only_UV|SV* sv
Tells and SV that it is an unsigned integer and disables all other OK bits.

=for apidoc Am|bool|SvIOK_UV|SV* sv
Returns a boolean indicating whether the SV contains an unsigned integer.

=for apidoc Am|bool|SvUOK|SV* sv
Returns a boolean indicating whether the SV contains an unsigned integer.

=for apidoc Am|bool|SvIOK_notUV|SV* sv
Returns a boolean indicating whether the SV contains a signed integer.

=for apidoc Am|U32|SvNOK|SV* sv
Returns a U32 value indicating whether the SV contains a double.

=for apidoc Am|void|SvNOK_on|SV* sv
Tells an SV that it is a double.

=for apidoc Am|void|SvNOK_off|SV* sv
Unsets the NV status of an SV.

=for apidoc Am|void|SvNOK_only|SV* sv
Tells an SV that it is a double and disables all other OK bits.

=for apidoc Am|U32|SvPOK|SV* sv
Returns a U32 value indicating whether the SV contains a character
string.

=for apidoc Am|void|SvPOK_on|SV* sv
Tells an SV that it is a string.

=for apidoc Am|void|SvPOK_off|SV* sv
Unsets the PV status of an SV.

=for apidoc Am|void|SvPOK_only|SV* sv
Tells an SV that it is a string and disables all other OK bits.
Will also turn off the UTF-8 status.

=for apidoc Am|bool|SvVOK|SV* sv
Returns a boolean indicating whether the SV contains a v-string.

=for apidoc Am|U32|SvOOK|SV* sv
Returns a U32 indicating whether the pointer to the string buffer is offset.
This hack is used internally to speed up removal of characters from the
beginning of a SvPV.  When SvOOK is true, then the start of the
allocated string buffer is actually C<SvOOK_offset()> bytes before SvPVX.
This offset used to be stored in SvIVX, but is now stored within the spare
part of the buffer.

=for apidoc Am|U32|SvROK|SV* sv
Tests if the SV is an RV.

=for apidoc Am|void|SvROK_on|SV* sv
Tells an SV that it is an RV.

=for apidoc Am|void|SvROK_off|SV* sv
Unsets the RV status of an SV.

=for apidoc Am|SV*|SvRV|SV* sv
Dereferences an RV to return the SV.

=for apidoc Am|IV|SvIVX|SV* sv
Returns the raw value in the SV's IV slot, without checks or conversions.
Only use when you are sure SvIOK is true.  See also C<SvIV()>.

=for apidoc Am|UV|SvUVX|SV* sv
Returns the raw value in the SV's UV slot, without checks or conversions.
Only use when you are sure SvIOK is true.  See also C<SvUV()>.

=for apidoc Am|NV|SvNVX|SV* sv
Returns the raw value in the SV's NV slot, without checks or conversions.
Only use when you are sure SvNOK is true.  See also C<SvNV()>.

=for apidoc Am|char*|SvPVX|SV* sv
Returns a pointer to the physical string in the SV.  The SV must contain a
string.

This is also used to store the name of an autoloaded subroutine in an XS
AUTOLOAD routine.  See L<perlguts/Autoloading with XSUBs>.

=for apidoc Am|STRLEN|SvCUR|SV* sv
Returns the length of the string which is in the SV.  See C<SvLEN>.

=for apidoc Am|STRLEN|SvLEN|SV* sv
Returns the size of the string buffer in the SV, not including any part
attributable to C<SvOOK>.  See C<SvCUR>.

=for apidoc Am|char*|SvEND|SV* sv
Returns a pointer to the spot just after the last character in
the string which is in the SV, where there is usually a trailing
null (even though Perl scalars do not strictly require it).
See C<SvCUR>.  Access the character as *(SvEND(sv)).

Warning: If C<SvCUR> is equal to C<SvLEN>, then C<SvEND> points to
unallocated memory.

=for apidoc Am|HV*|SvSTASH|SV* sv
Returns the stash of the SV.

=for apidoc Am|void|SvIV_set|SV* sv|IV val
Set the value of the IV pointer in sv to val.  It is possible to perform
the same function of this macro with an lvalue assignment to C<SvIVX>.
With future Perls, however, it will be more efficient to use 
C<SvIV_set> instead of the lvalue assignment to C<SvIVX>.

=for apidoc Am|void|SvNV_set|SV* sv|NV val
Set the value of the NV pointer in sv to val.  See C<SvIV_set>.

=for apidoc Am|void|SvPV_set|SV* sv|char* val
Set the value of the PV pointer in sv to val.  See C<SvIV_set>.

=for apidoc Am|void|SvUV_set|SV* sv|UV val
Set the value of the UV pointer in sv to val.  See C<SvIV_set>.

=for apidoc Am|void|SvRV_set|SV* sv|SV* val
Set the value of the RV pointer in sv to val.  See C<SvIV_set>.

=for apidoc Am|void|SvMAGIC_set|SV* sv|MAGIC* val
Set the value of the MAGIC pointer in sv to val.  See C<SvIV_set>.

=for apidoc Am|void|SvSTASH_set|SV* sv|HV* val
Set the value of the STASH pointer in sv to val.  See C<SvIV_set>.

=for apidoc Am|void|SvCUR_set|SV* sv|STRLEN len
Set the current length of the string which is in the SV.  See C<SvCUR>
and C<SvIV_set>.

=for apidoc Am|void|SvLEN_set|SV* sv|STRLEN len
Set the actual length of the string which is in the SV.  See C<SvIV_set>.

=cut
*/

#define SvNIOK(sv)		(SvFLAGS(sv) & (SVf_IOK|SVf_NOK))
#define SvNIOKp(sv)		(SvFLAGS(sv) & (SVp_IOK|SVp_NOK))
#define SvNIOK_off(sv)		(SvFLAGS(sv) &= ~(SVf_IOK|SVf_NOK| \
						  SVp_IOK|SVp_NOK|SVf_IVisUV))

#if defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
#define assert_not_ROK(sv)	({assert(!SvROK(sv) || !SvRV(sv));}),
#define assert_not_glob(sv)	({assert(!isGV_with_GP(sv));}),
#else
#define assert_not_ROK(sv)	
#define assert_not_glob(sv)	
#endif

#define SvOK(sv)		((SvTYPE(sv) == SVt_BIND)		\
				 ? (SvFLAGS(SvRV(sv)) & SVf_OK)		\
				 : (SvFLAGS(sv) & SVf_OK))
#define SvOK_off(sv)		(assert_not_ROK(sv) assert_not_glob(sv)	\
				 SvFLAGS(sv) &=	~(SVf_OK|		\
						  SVf_IVisUV|SVf_UTF8),	\
							SvOOK_off(sv))
#define SvOK_off_exc_UV(sv)	(assert_not_ROK(sv)			\
				 SvFLAGS(sv) &=	~(SVf_OK|		\
						  SVf_UTF8),		\
							SvOOK_off(sv))

#define SvOKp(sv)		(SvFLAGS(sv) & (SVp_IOK|SVp_NOK|SVp_POK))
#define SvIOKp(sv)		(SvFLAGS(sv) & SVp_IOK)
#define SvIOKp_on(sv)		(assert_not_glob(sv) SvRELEASE_IVX_(sv)	\
				    SvFLAGS(sv) |= SVp_IOK)
#define SvNOKp(sv)		(SvFLAGS(sv) & SVp_NOK)
#define SvNOKp_on(sv)		(assert_not_glob(sv) SvFLAGS(sv) |= SVp_NOK)
#define SvPOKp(sv)		(SvFLAGS(sv) & SVp_POK)
#define SvPOKp_on(sv)		(assert_not_ROK(sv) assert_not_glob(sv)	\
				 SvFLAGS(sv) |= SVp_POK)

#define SvIOK(sv)		(SvFLAGS(sv) & SVf_IOK)
#define SvIOK_on(sv)		(assert_not_glob(sv) SvRELEASE_IVX_(sv)	\
				    SvFLAGS(sv) |= (SVf_IOK|SVp_IOK))
#define SvIOK_off(sv)		(SvFLAGS(sv) &= ~(SVf_IOK|SVp_IOK|SVf_IVisUV))
#define SvIOK_only(sv)		(SvOK_off(sv), \
				    SvFLAGS(sv) |= (SVf_IOK|SVp_IOK))
#define SvIOK_only_UV(sv)	(assert_not_glob(sv) SvOK_off_exc_UV(sv), \
				    SvFLAGS(sv) |= (SVf_IOK|SVp_IOK))

#define SvIOK_UV(sv)		((SvFLAGS(sv) & (SVf_IOK|SVf_IVisUV))	\
				 == (SVf_IOK|SVf_IVisUV))
#define SvUOK(sv)		SvIOK_UV(sv)
#define SvIOK_notUV(sv)		((SvFLAGS(sv) & (SVf_IOK|SVf_IVisUV))	\
				 == SVf_IOK)

#define SvIsUV(sv)		(SvFLAGS(sv) & SVf_IVisUV)
#define SvIsUV_on(sv)		(SvFLAGS(sv) |= SVf_IVisUV)
#define SvIsUV_off(sv)		(SvFLAGS(sv) &= ~SVf_IVisUV)

#define SvNOK(sv)		(SvFLAGS(sv) & SVf_NOK)
#define SvNOK_on(sv)		(assert_not_glob(sv) \
				 SvFLAGS(sv) |= (SVf_NOK|SVp_NOK))
#define SvNOK_off(sv)		(SvFLAGS(sv) &= ~(SVf_NOK|SVp_NOK))
#define SvNOK_only(sv)		(SvOK_off(sv), \
				    SvFLAGS(sv) |= (SVf_NOK|SVp_NOK))

/*
=for apidoc Am|U32|SvUTF8|SV* sv
Returns a U32 value indicating the UTF-8 status of an SV.  If things are set-up
properly, this indicates whether or not the SV contains UTF-8 encoded data.
Call this after SvPV() in case any call to string overloading updates the
internal flag.

=for apidoc Am|void|SvUTF8_on|SV *sv
Turn on the UTF-8 status of an SV (the data is not changed, just the flag).
Do not use frivolously.

=for apidoc Am|void|SvUTF8_off|SV *sv
Unsets the UTF-8 status of an SV (the data is not changed, just the flag).
Do not use frivolously.

=for apidoc Am|void|SvPOK_only_UTF8|SV* sv
Tells an SV that it is a string and disables all other OK bits,
and leaves the UTF-8 status as it was.

=cut
 */

/* Ensure the return value of this macro does not clash with the GV_ADD* flags
in gv.h: */
#define SvUTF8(sv)		(SvFLAGS(sv) & SVf_UTF8)
#define SvUTF8_on(sv)		(SvFLAGS(sv) |= (SVf_UTF8))
#define SvUTF8_off(sv)		(SvFLAGS(sv) &= ~(SVf_UTF8))

#define SvPOK(sv)		(SvFLAGS(sv) & SVf_POK)
#define SvPOK_on(sv)		(assert_not_ROK(sv) assert_not_glob(sv)	\
				 SvFLAGS(sv) |= (SVf_POK|SVp_POK))
#define SvPOK_off(sv)		(SvFLAGS(sv) &= ~(SVf_POK|SVp_POK))
#define SvPOK_only(sv)		(assert_not_ROK(sv) assert_not_glob(sv)	\
				 SvFLAGS(sv) &= ~(SVf_OK|		\
						  SVf_IVisUV|SVf_UTF8),	\
				    SvFLAGS(sv) |= (SVf_POK|SVp_POK))
#define SvPOK_only_UTF8(sv)	(assert_not_ROK(sv) assert_not_glob(sv)	\
				 SvFLAGS(sv) &= ~(SVf_OK|		\
						  SVf_IVisUV),		\
				    SvFLAGS(sv) |= (SVf_POK|SVp_POK))

#define SvVOK(sv)		(SvMAGICAL(sv)				\
				 && mg_find(sv,PERL_MAGIC_vstring))
/* returns the vstring magic, if any */
#define SvVSTRING_mg(sv)	(SvMAGICAL(sv) \
				 ? mg_find(sv,PERL_MAGIC_vstring) : NULL)

#define SvOOK(sv)		(SvFLAGS(sv) & SVf_OOK)
#define SvOOK_on(sv)		(SvFLAGS(sv) |= SVf_OOK)
#define SvOOK_off(sv)		((void)(SvOOK(sv) && sv_backoff(sv)))

#define SvFAKE(sv)		(SvFLAGS(sv) & SVf_FAKE)
#define SvFAKE_on(sv)		(SvFLAGS(sv) |= SVf_FAKE)
#define SvFAKE_off(sv)		(SvFLAGS(sv) &= ~SVf_FAKE)

#define SvROK(sv)		(SvFLAGS(sv) & SVf_ROK)
#define SvROK_on(sv)		(SvFLAGS(sv) |= SVf_ROK)
#define SvROK_off(sv)		(SvFLAGS(sv) &= ~(SVf_ROK))

#define SvMAGICAL(sv)		(SvFLAGS(sv) & (SVs_GMG|SVs_SMG|SVs_RMG))
#define SvMAGICAL_on(sv)	(SvFLAGS(sv) |= (SVs_GMG|SVs_SMG|SVs_RMG))
#define SvMAGICAL_off(sv)	(SvFLAGS(sv) &= ~(SVs_GMG|SVs_SMG|SVs_RMG))

#define SvGMAGICAL(sv)		(SvFLAGS(sv) & SVs_GMG)
#define SvGMAGICAL_on(sv)	(SvFLAGS(sv) |= SVs_GMG)
#define SvGMAGICAL_off(sv)	(SvFLAGS(sv) &= ~SVs_GMG)

#define SvSMAGICAL(sv)		(SvFLAGS(sv) & SVs_SMG)
#define SvSMAGICAL_on(sv)	(SvFLAGS(sv) |= SVs_SMG)
#define SvSMAGICAL_off(sv)	(SvFLAGS(sv) &= ~SVs_SMG)

#define SvRMAGICAL(sv)		(SvFLAGS(sv) & SVs_RMG)
#define SvRMAGICAL_on(sv)	(SvFLAGS(sv) |= SVs_RMG)
#define SvRMAGICAL_off(sv)	(SvFLAGS(sv) &= ~SVs_RMG)

#define SvAMAGIC(sv)		(SvROK(sv) && (SvFLAGS(SvRV(sv)) & SVf_AMAGIC))
#if defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
#  define SvAMAGIC_on(sv)	({ SV * const kloink = sv;		\
				   assert(SvROK(kloink));		\
				   SvFLAGS(SvRV(kloink)) |= SVf_AMAGIC;	\
				})
#  define SvAMAGIC_off(sv)	({ SV * const kloink = sv;		\
				   if(SvROK(kloink))			\
					SvFLAGS(SvRV(kloink)) &= ~SVf_AMAGIC;\
				})
#else
#  define SvAMAGIC_on(sv)	(SvFLAGS(SvRV(sv)) |= SVf_AMAGIC)
#  define SvAMAGIC_off(sv) \
	(SvROK(sv) && (SvFLAGS(SvRV(sv)) &= ~SVf_AMAGIC))
#endif

/*
=for apidoc Am|U32|SvGAMAGIC|SV* sv

Returns true if the SV has get magic or
overloading.  If either is true then
the scalar is active data, and has the potential to return a new value every
time it is accessed.  Hence you must be careful to
only read it once per user logical operation and work
with that returned value.  If neither is true then
the scalar's value cannot change unless written to.

=cut
*/

#define SvGAMAGIC(sv)           (SvGMAGICAL(sv) || SvAMAGIC(sv))

#define Gv_AMG(stash)           (PL_amagic_generation && Gv_AMupdate(stash, FALSE))

#define SvWEAKREF(sv)		((SvFLAGS(sv) & (SVf_ROK|SVprv_WEAKREF)) \
				  == (SVf_ROK|SVprv_WEAKREF))
#define SvWEAKREF_on(sv)	(SvFLAGS(sv) |=  (SVf_ROK|SVprv_WEAKREF))
#define SvWEAKREF_off(sv)	(SvFLAGS(sv) &= ~(SVf_ROK|SVprv_WEAKREF))

#define SvPCS_IMPORTED(sv)	((SvFLAGS(sv) & (SVf_ROK|SVprv_PCS_IMPORTED)) \
				 == (SVf_ROK|SVprv_PCS_IMPORTED))
#define SvPCS_IMPORTED_on(sv)	(SvFLAGS(sv) |=  (SVf_ROK|SVprv_PCS_IMPORTED))
#define SvPCS_IMPORTED_off(sv)	(SvFLAGS(sv) &= ~(SVf_ROK|SVprv_PCS_IMPORTED))

#define SvTHINKFIRST(sv)	(SvFLAGS(sv) & SVf_THINKFIRST)

#define SvPADMY(sv)		(SvFLAGS(sv) & SVs_PADMY)
#define SvPADMY_on(sv)		(SvFLAGS(sv) |= SVs_PADMY)

/* SVs_PADTMP and SVs_PADSTALE share the same bit, mediated by SVs_PADMY */

#define SvPADTMP(sv)	((SvFLAGS(sv) & (SVs_PADMY|SVs_PADTMP)) == SVs_PADTMP)
#define SvPADSTALE(sv)	((SvFLAGS(sv) & (SVs_PADMY|SVs_PADSTALE)) \
				    == (SVs_PADMY|SVs_PADSTALE))

#if defined (DEBUGGING) && defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
#  define SvPADTMP_on(sv)	({			\
	    SV *const _svpad = MUTABLE_SV(sv);		\
	    assert(!(SvFLAGS(_svpad) & SVs_PADMY));	\
	    SvFLAGS(_svpad) |= SVs_PADTMP;		\
	})
#  define SvPADTMP_off(sv)	({			\
	    SV *const _svpad = MUTABLE_SV(sv);		\
	    assert(!(SvFLAGS(_svpad) & SVs_PADMY));	\
	    SvFLAGS(_svpad) &= ~SVs_PADTMP;		\
	})
#  define SvPADSTALE_on(sv)	({			\
	    SV *const _svpad = MUTABLE_SV(sv);		\
	    assert(SvFLAGS(_svpad) & SVs_PADMY);	\
	    SvFLAGS(_svpad) |= SVs_PADSTALE;		\
	})
#  define SvPADSTALE_off(sv)	({			\
	    SV *const _svpad = MUTABLE_SV(sv);		\
	    assert(SvFLAGS(_svpad) & SVs_PADMY);	\
	    SvFLAGS(_svpad) &= ~SVs_PADSTALE;		\
	})
#else
#  define SvPADTMP_on(sv)	(SvFLAGS(sv) |= SVs_PADTMP)
#  define SvPADTMP_off(sv)	(SvFLAGS(sv) &= ~SVs_PADTMP)
#  define SvPADSTALE_on(sv)	(SvFLAGS(sv) |= SVs_PADSTALE)
#  define SvPADSTALE_off(sv)	(SvFLAGS(sv) &= ~SVs_PADSTALE)
#endif

#define SvTEMP(sv)		(SvFLAGS(sv) & SVs_TEMP)
#define SvTEMP_on(sv)		(SvFLAGS(sv) |= SVs_TEMP)
#define SvTEMP_off(sv)		(SvFLAGS(sv) &= ~SVs_TEMP)

#define SvOBJECT(sv)		(SvFLAGS(sv) & SVs_OBJECT)
#define SvOBJECT_on(sv)		(SvFLAGS(sv) |= SVs_OBJECT)
#define SvOBJECT_off(sv)	(SvFLAGS(sv) &= ~SVs_OBJECT)

#define SvREADONLY(sv)		(SvFLAGS(sv) & SVf_READONLY)
#define SvREADONLY_on(sv)	(SvFLAGS(sv) |= SVf_READONLY)
#define SvREADONLY_off(sv)	(SvFLAGS(sv) &= ~SVf_READONLY)

#define SvSCREAM(sv) ((SvFLAGS(sv) & (SVp_SCREAM|SVp_POK)) == (SVp_SCREAM|SVp_POK))
#define SvSCREAM_on(sv)		(SvFLAGS(sv) |= SVp_SCREAM)
#define SvSCREAM_off(sv)	(SvFLAGS(sv) &= ~SVp_SCREAM)

#ifndef PERL_CORE
#  define SvCOMPILED(sv)	0
#  define SvCOMPILED_on(sv)
#  define SvCOMPILED_off(sv)
#endif

#define SvEVALED(sv)		(SvFLAGS(sv) & SVrepl_EVAL)
#define SvEVALED_on(sv)		(SvFLAGS(sv) |= SVrepl_EVAL)
#define SvEVALED_off(sv)	(SvFLAGS(sv) &= ~SVrepl_EVAL)

#if defined (DEBUGGING) && defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
#  define SvVALID(sv)		({ const SV *const _svvalid = (const SV*)(sv); \
				   if (SvFLAGS(_svvalid) & SVpbm_VALID)	\
				       assert(!isGV_with_GP(_svvalid));	\
				   (SvFLAGS(_svvalid) & SVpbm_VALID);	\
				})
#  define SvVALID_on(sv)	({ SV *const _svvalid = MUTABLE_SV(sv);	\
				   assert(!isGV_with_GP(_svvalid));	\
				   (SvFLAGS(_svvalid) |= SVpbm_VALID);	\
				})
#  define SvVALID_off(sv)	({ SV *const _svvalid = MUTABLE_SV(sv);	\
				   assert(!isGV_with_GP(_svvalid));	\
				   (SvFLAGS(_svvalid) &= ~SVpbm_VALID);	\
				})

#  define SvTAIL(sv)	({ const SV *const _svtail = (const SV *)(sv);	\
			    assert(SvTYPE(_svtail) != SVt_PVAV);		\
			    assert(SvTYPE(_svtail) != SVt_PVHV);		\
			    (SvFLAGS(sv) & (SVpbm_TAIL|SVpbm_VALID))	\
				== (SVpbm_TAIL|SVpbm_VALID);		\
			})
#else
#  define SvVALID(sv)		(SvFLAGS(sv) & SVpbm_VALID)
#  define SvVALID_on(sv)	(SvFLAGS(sv) |= SVpbm_VALID)
#  define SvVALID_off(sv)	(SvFLAGS(sv) &= ~SVpbm_VALID)
#  define SvTAIL(sv)	    ((SvFLAGS(sv) & (SVpbm_TAIL|SVpbm_VALID))	\
			     == (SVpbm_TAIL|SVpbm_VALID))

#endif
#define SvTAIL_on(sv)		(SvFLAGS(sv) |= SVpbm_TAIL)
#define SvTAIL_off(sv)		(SvFLAGS(sv) &= ~SVpbm_TAIL)


#define SvPAD_TYPED(sv) \
	((SvFLAGS(sv) & (SVpad_NAME|SVpad_TYPED)) == (SVpad_NAME|SVpad_TYPED))

#define SvPAD_OUR(sv)	\
	((SvFLAGS(sv) & (SVpad_NAME|SVpad_OUR)) == (SVpad_NAME|SVpad_OUR))

#define SvPAD_STATE(sv)	\
	((SvFLAGS(sv) & (SVpad_NAME|SVpad_STATE)) == (SVpad_NAME|SVpad_STATE))

#if defined (DEBUGGING) && defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
#  define SvPAD_TYPED_on(sv)	({					\
	    SV *const _svpad = MUTABLE_SV(sv);				\
	    assert(SvTYPE(_svpad) == SVt_PVMG);				\
	    (SvFLAGS(_svpad) |= SVpad_NAME|SVpad_TYPED);		\
	})
#define SvPAD_OUR_on(sv)	({					\
	    SV *const _svpad = MUTABLE_SV(sv);				\
	    assert(SvTYPE(_svpad) == SVt_PVMG);				\
	    (SvFLAGS(_svpad) |= SVpad_NAME|SVpad_OUR);			\
	})
#define SvPAD_STATE_on(sv)	({					\
	    SV *const _svpad = MUTABLE_SV(sv);				\
	    assert(SvTYPE(_svpad) == SVt_PVNV || SvTYPE(_svpad) == SVt_PVMG); \
	    (SvFLAGS(_svpad) |= SVpad_NAME|SVpad_STATE);		\
	})
#else
#  define SvPAD_TYPED_on(sv)	(SvFLAGS(sv) |= SVpad_NAME|SVpad_TYPED)
#  define SvPAD_OUR_on(sv)	(SvFLAGS(sv) |= SVpad_NAME|SVpad_OUR)
#  define SvPAD_STATE_on(sv)	(SvFLAGS(sv) |= SVpad_NAME|SVpad_STATE)
#endif

#define SvOURSTASH(sv)	\
	(SvPAD_OUR(sv) ? ((XPVMG*) SvANY(sv))->xmg_u.xmg_ourstash : NULL)
#define SvOURSTASH_set(sv, st)					\
        STMT_START {						\
	    assert(SvTYPE(sv) == SVt_PVMG);			\
	    ((XPVMG*) SvANY(sv))->xmg_u.xmg_ourstash = st;	\
	} STMT_END

#define SvRVx(sv) SvRV(sv)

#ifdef PERL_DEBUG_COW
/* Need -0.0 for SvNVX to preserve IEEE FP "negative zero" because
   +0.0 + -0.0 => +0.0 but -0.0 + -0.0 => -0.0 */
#  define SvIVX(sv) (0 + ((XPVIV*) SvANY(sv))->xiv_iv)
#  define SvUVX(sv) (0 + ((XPVUV*) SvANY(sv))->xuv_uv)
#  define SvNVX(sv) (-0.0 + ((XPVNV*) SvANY(sv))->xnv_u.xnv_nv)
#  define SvRV(sv) (0 + (sv)->sv_u.svu_rv)
#  define SvRV_const(sv) (0 + (sv)->sv_u.svu_rv)
/* Don't test the core XS code yet.  */
#  if defined (PERL_CORE) && PERL_DEBUG_COW > 1
#    define SvPVX(sv) (0 + (assert(!SvREADONLY(sv)), (sv)->sv_u.svu_pv))
#  else
#  define SvPVX(sv) SvPVX_mutable(sv)
#  endif
#  define SvCUR(sv) (0 + ((XPV*) SvANY(sv))->xpv_cur)
#  define SvLEN(sv) (0 + ((XPV*) SvANY(sv))->xpv_len)
#  define SvEND(sv) ((sv)->sv_u.svu_pv + ((XPV*)SvANY(sv))->xpv_cur)

#  ifdef DEBUGGING
#    define SvMAGIC(sv)	(0 + *(assert(SvTYPE(sv) >= SVt_PVMG), &((XPVMG*)  SvANY(sv))->xmg_u.xmg_magic))
#    define SvSTASH(sv)	(0 + *(assert(SvTYPE(sv) >= SVt_PVMG), &((XPVMG*)  SvANY(sv))->xmg_stash))
#  else
#    define SvMAGIC(sv)	(0 + ((XPVMG*)  SvANY(sv))->xmg_u.xmg_magic)
#    define SvSTASH(sv)	(0 + ((XPVMG*)  SvANY(sv))->xmg_stash)
#  endif
#else
#  define SvLEN(sv) ((XPV*) SvANY(sv))->xpv_len
#  define SvEND(sv) ((sv)->sv_u.svu_pv + ((XPV*)SvANY(sv))->xpv_cur)

#  if defined (DEBUGGING) && defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
/* These get expanded inside other macros that already use a variable _sv  */
#    define SvPVX(sv)							\
	(*({ SV *const _svpvx = MUTABLE_SV(sv);				\
	    assert(PL_valid_types_PVX[SvTYPE(_svpvx) & SVt_MASK]);	\
	    assert(!isGV_with_GP(_svpvx));				\
	    assert(!(SvTYPE(_svpvx) == SVt_PVIO				\
		     && !(IoFLAGS(_svpvx) & IOf_FAKE_DIRP)));		\
	    &((_svpvx)->sv_u.svu_pv);					\
	 }))
#    define SvCUR(sv)							\
	(*({ const SV *const _svcur = (const SV *)(sv);			\
	    assert(PL_valid_types_PVX[SvTYPE(_svcur) & SVt_MASK]);	\
	    assert(!isGV_with_GP(_svcur));				\
	    assert(!(SvTYPE(_svcur) == SVt_PVIO				\
		     && !(IoFLAGS(_svcur) & IOf_FAKE_DIRP)));		\
	    &(((XPV*) MUTABLE_PTR(SvANY(_svcur)))->xpv_cur);		\
	 }))
#    define SvIVX(sv)							\
	(*({ const SV *const _svivx = (const SV *)(sv);			\
	    assert(PL_valid_types_IVX[SvTYPE(_svivx) & SVt_MASK]);	\
	    assert(!isGV_with_GP(_svivx));				\
	    &(((XPVIV*) MUTABLE_PTR(SvANY(_svivx)))->xiv_iv);		\
	 }))
#    define SvUVX(sv)							\
	(*({ const SV *const _svuvx = (const SV *)(sv);			\
	    assert(PL_valid_types_IVX[SvTYPE(_svuvx) & SVt_MASK]);	\
	    assert(!isGV_with_GP(_svuvx));				\
	    &(((XPVUV*) MUTABLE_PTR(SvANY(_svuvx)))->xuv_uv);		\
	 }))
#    define SvNVX(sv)							\
	(*({ const SV *const _svnvx = (const SV *)(sv);			\
	    assert(PL_valid_types_NVX[SvTYPE(_svnvx) & SVt_MASK]);	\
	    assert(!isGV_with_GP(_svnvx));				\
	    &(((XPVNV*) MUTABLE_PTR(SvANY(_svnvx)))->xnv_u.xnv_nv);	\
	 }))
#    define SvRV(sv)							\
	(*({ SV *const _svrv = MUTABLE_SV(sv);				\
	    assert(PL_valid_types_RV[SvTYPE(_svrv) & SVt_MASK]);	\
	    assert(!isGV_with_GP(_svrv));				\
	    assert(!(SvTYPE(_svrv) == SVt_PVIO				\
		     && !(IoFLAGS(_svrv) & IOf_FAKE_DIRP)));		\
	    &((_svrv)->sv_u.svu_rv);					\
	 }))
#    define SvRV_const(sv)						\
	({ const SV *const _svrv = (const SV *)(sv);			\
	    assert(PL_valid_types_RV[SvTYPE(_svrv) & SVt_MASK]);	\
	    assert(!isGV_with_GP(_svrv));				\
	    assert(!(SvTYPE(_svrv) == SVt_PVIO				\
		     && !(IoFLAGS(_svrv) & IOf_FAKE_DIRP)));		\
	    (_svrv)->sv_u.svu_rv;					\
	 })
#    define SvMAGIC(sv)							\
	(*({ const SV *const _svmagic = (const SV *)(sv);		\
	    assert(SvTYPE(_svmagic) >= SVt_PVMG);			\
	    if(SvTYPE(_svmagic) == SVt_PVMG)				\
		assert(!SvPAD_OUR(_svmagic));				\
	    &(((XPVMG*) MUTABLE_PTR(SvANY(_svmagic)))->xmg_u.xmg_magic); \
	  }))
#    define SvSTASH(sv)							\
	(*({ const SV *const _svstash = (const SV *)(sv);		\
	    assert(SvTYPE(_svstash) >= SVt_PVMG);			\
	    &(((XPVMG*) MUTABLE_PTR(SvANY(_svstash)))->xmg_stash);	\
	  }))
#  else
#    define SvPVX(sv) ((sv)->sv_u.svu_pv)
#    define SvCUR(sv) ((XPV*) SvANY(sv))->xpv_cur
#    define SvIVX(sv) ((XPVIV*) SvANY(sv))->xiv_iv
#    define SvUVX(sv) ((XPVUV*) SvANY(sv))->xuv_uv
#    define SvNVX(sv) ((XPVNV*) SvANY(sv))->xnv_u.xnv_nv
#    define SvRV(sv) ((sv)->sv_u.svu_rv)
#    define SvRV_const(sv) (0 + (sv)->sv_u.svu_rv)
#    define SvMAGIC(sv)	((XPVMG*)  SvANY(sv))->xmg_u.xmg_magic
#    define SvSTASH(sv)	((XPVMG*)  SvANY(sv))->xmg_stash
#  endif
#endif

#ifndef PERL_POISON
/* Given that these two are new, there can't be any existing code using them
 *  as LVALUEs  */
#  define SvPVX_mutable(sv)	(0 + (sv)->sv_u.svu_pv)
#  define SvPVX_const(sv)	((const char*)(0 + (sv)->sv_u.svu_pv))
#else
/* Except for the poison code, which uses & to scribble over the pointer after
   free() is called.  */
#  define SvPVX_mutable(sv)	((sv)->sv_u.svu_pv)
#  define SvPVX_const(sv)	((const char*)((sv)->sv_u.svu_pv))
#endif

#define SvIVXx(sv) SvIVX(sv)
#define SvUVXx(sv) SvUVX(sv)
#define SvNVXx(sv) SvNVX(sv)
#define SvPVXx(sv) SvPVX(sv)
#define SvLENx(sv) SvLEN(sv)
#define SvENDx(sv) ((PL_Sv = (sv)), SvEND(PL_Sv))


/* Ask a scalar nicely to try to become an IV, if possible.
   Not guaranteed to stay returning void */
/* Macro won't actually call sv_2iv if already IOK */
#define SvIV_please(sv) \
	STMT_START {if (!SvIOKp(sv) && (SvNOK(sv) || SvPOK(sv))) \
		(void) SvIV(sv); } STMT_END
#define SvIV_please_nomg(sv) \
	STMT_START {if (!SvIOKp(sv) && (SvNOK(sv) || SvPOK(sv))) \
		(void) SvIV_nomg(sv); } STMT_END
#define SvIV_set(sv, val) \
	STMT_START { \
		assert(PL_valid_types_IV_set[SvTYPE(sv) & SVt_MASK]);	\
		assert(!isGV_with_GP(sv));		\
		(((XPVIV*)  SvANY(sv))->xiv_iv = (val)); } STMT_END
#define SvNV_set(sv, val) \
	STMT_START { \
		assert(PL_valid_types_NV_set[SvTYPE(sv) & SVt_MASK]);	\
		assert(!isGV_with_GP(sv));		\
		(((XPVNV*)SvANY(sv))->xnv_u.xnv_nv = (val)); } STMT_END
#define SvPV_set(sv, val) \
	STMT_START { \
		assert(PL_valid_types_PVX[SvTYPE(sv) & SVt_MASK]);	\
		assert(!isGV_with_GP(sv));		\
		assert(!(SvTYPE(sv) == SVt_PVIO		\
		     && !(IoFLAGS(sv) & IOf_FAKE_DIRP))); \
		((sv)->sv_u.svu_pv = (val)); } STMT_END
#define SvUV_set(sv, val) \
	STMT_START { \
		assert(PL_valid_types_IV_set[SvTYPE(sv) & SVt_MASK]);	\
		assert(!isGV_with_GP(sv));		\
		(((XPVUV*)SvANY(sv))->xuv_uv = (val)); } STMT_END
#define SvRV_set(sv, val) \
        STMT_START { \
		assert(PL_valid_types_RV[SvTYPE(sv) & SVt_MASK]);	\
		assert(!isGV_with_GP(sv));		\
		assert(!(SvTYPE(sv) == SVt_PVIO		\
		     && !(IoFLAGS(sv) & IOf_FAKE_DIRP))); \
                ((sv)->sv_u.svu_rv = (val)); } STMT_END
#define SvMAGIC_set(sv, val) \
        STMT_START { assert(SvTYPE(sv) >= SVt_PVMG); \
                (((XPVMG*)SvANY(sv))->xmg_u.xmg_magic = (val)); } STMT_END
#define SvSTASH_set(sv, val) \
        STMT_START { assert(SvTYPE(sv) >= SVt_PVMG); \
                (((XPVMG*)  SvANY(sv))->xmg_stash = (val)); } STMT_END
#define SvCUR_set(sv, val) \
	STMT_START { \
		assert(PL_valid_types_PVX[SvTYPE(sv) & SVt_MASK]);	\
		assert(!isGV_with_GP(sv));		\
		assert(!(SvTYPE(sv) == SVt_PVIO		\
		     && !(IoFLAGS(sv) & IOf_FAKE_DIRP))); \
		(((XPV*)  SvANY(sv))->xpv_cur = (val)); } STMT_END
#define SvLEN_set(sv, val) \
	STMT_START { \
		assert(PL_valid_types_PVX[SvTYPE(sv) & SVt_MASK]);	\
		assert(!isGV_with_GP(sv));	\
		assert(!(SvTYPE(sv) == SVt_PVIO		\
		     && !(IoFLAGS(sv) & IOf_FAKE_DIRP))); \
		(((XPV*)  SvANY(sv))->xpv_len = (val)); } STMT_END
#define SvEND_set(sv, val) \
	STMT_START { assert(SvTYPE(sv) >= SVt_PV); \
		SvCUR_set(sv, (val) - SvPVX(sv)); } STMT_END

#define SvPV_renew(sv,n) \
	STMT_START { SvLEN_set(sv, n); \
		SvPV_set((sv), (MEM_WRAP_CHECK_(n,char)			\
				(char*)saferealloc((Malloc_t)SvPVX(sv), \
						   (MEM_SIZE)((n)))));  \
		 } STMT_END

#define SvPV_shrink_to_cur(sv) STMT_START { \
		   const STRLEN _lEnGtH = SvCUR(sv) + 1; \
		   SvPV_renew(sv, _lEnGtH); \
		 } STMT_END

#define SvPV_free(sv)							\
    STMT_START {							\
		     assert(SvTYPE(sv) >= SVt_PV);			\
		     if (SvLEN(sv)) {					\
			 assert(!SvROK(sv));				\
			 if(SvOOK(sv)) {				\
			     STRLEN zok; 				\
			     SvOOK_offset(sv, zok);			\
			     SvPV_set(sv, SvPVX_mutable(sv) - zok);	\
			     SvFLAGS(sv) &= ~SVf_OOK;			\
			 }						\
			 Safefree(SvPVX(sv));				\
		     }							\
		 } STMT_END

#ifdef PERL_CORE
/* Code that crops up in three places to take a scalar and ready it to hold
   a reference */
#  define prepare_SV_for_RV(sv)						\
    STMT_START {							\
		    if (SvTYPE(sv) < SVt_PV && SvTYPE(sv) != SVt_IV)	\
			sv_upgrade(sv, SVt_IV);				\
		    else if (SvTYPE(sv) >= SVt_PV) {			\
			SvPV_free(sv);					\
			SvLEN_set(sv, 0);				\
                        SvCUR_set(sv, 0);				\
		    }							\
		 } STMT_END
#endif

#ifndef PERL_CORE
#  define BmFLAGS(sv)		(SvTAIL(sv) ? FBMcf_TAIL : 0)
#endif

#if defined (DEBUGGING) && defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
#  define BmRARE(sv)							\
	(*({ SV *const _bmrare = MUTABLE_SV(sv);			\
		assert(SvTYPE(_bmrare) == SVt_PVMG);			\
		assert(SvVALID(_bmrare));				\
	    &(((XPVMG*) SvANY(_bmrare))->xnv_u.xbm_s.xbm_rare);		\
	 }))
#  define BmUSEFUL(sv)							\
	(*({ SV *const _bmuseful = MUTABLE_SV(sv);			\
	    assert(SvTYPE(_bmuseful) == SVt_PVMG);			\
	    assert(SvVALID(_bmuseful));					\
	    assert(!SvIOK(_bmuseful));					\
	    &(((XPVMG*) SvANY(_bmuseful))->xnv_u.xbm_s.xbm_useful);	\
	 }))
#  define BmPREVIOUS(sv)						\
    (*({ SV *const _bmprevious = MUTABLE_SV(sv);			\
		assert(SvTYPE(_bmprevious) == SVt_PVMG);		\
		assert(SvVALID(_bmprevious));				\
	    &(((XPVMG*) SvANY(_bmprevious))->xiv_u.xivu_uv);		\
	 }))
#else
#  define BmRARE(sv)		((XPVMG*) SvANY(sv))->xnv_u.xbm_s.xbm_rare
#  define BmUSEFUL(sv)		((XPVMG*) SvANY(sv))->xnv_u.xbm_s.xbm_useful
#  define BmPREVIOUS(sv)	((XPVMG*) SvANY(sv))->xiv_u.xivu_uv

#endif

#define FmLINES(sv)	((XPVFM*)  SvANY(sv))->xfm_lines

#define LvTYPE(sv)	((XPVLV*)  SvANY(sv))->xlv_type
#define LvTARG(sv)	((XPVLV*)  SvANY(sv))->xlv_targ
#define LvTARGOFF(sv)	((XPVLV*)  SvANY(sv))->xlv_targoff
#define LvTARGLEN(sv)	((XPVLV*)  SvANY(sv))->xlv_targlen
#define LvFLAGS(sv)	((XPVLV*)  SvANY(sv))->xlv_flags

#define IoIFP(sv)	(sv)->sv_u.svu_fp
#define IoOFP(sv)	((XPVIO*)  SvANY(sv))->xio_ofp
#define IoDIRP(sv)	((XPVIO*)  SvANY(sv))->xio_dirp
#define IoANY(sv)	((XPVIO*)  SvANY(sv))->xio_any
#define IoLINES(sv)	((XPVIO*)  SvANY(sv))->xiv_u.xivu_iv
#define IoPAGE(sv)	((XPVIO*)  SvANY(sv))->xio_page
#define IoPAGE_LEN(sv)	((XPVIO*)  SvANY(sv))->xio_page_len
#define IoLINES_LEFT(sv)((XPVIO*)  SvANY(sv))->xio_lines_left
#define IoTOP_NAME(sv)	((XPVIO*)  SvANY(sv))->xio_top_name
#define IoTOP_GV(sv)	((XPVIO*)  SvANY(sv))->xio_top_gv
#define IoFMT_NAME(sv)	((XPVIO*)  SvANY(sv))->xio_fmt_name
#define IoFMT_GV(sv)	((XPVIO*)  SvANY(sv))->xio_fmt_gv
#define IoBOTTOM_NAME(sv)((XPVIO*) SvANY(sv))->xio_bottom_name
#define IoBOTTOM_GV(sv)	((XPVIO*)  SvANY(sv))->xio_bottom_gv
#define IoTYPE(sv)	((XPVIO*)  SvANY(sv))->xio_type
#define IoFLAGS(sv)	((XPVIO*)  SvANY(sv))->xio_flags

/* IoTYPE(sv) is a single character telling the type of I/O connection. */
#define IoTYPE_RDONLY		'<'
#define IoTYPE_WRONLY		'>'
#define IoTYPE_RDWR		'+'
#define IoTYPE_APPEND 		'a'
#define IoTYPE_PIPE		'|'
#define IoTYPE_STD		'-'	/* stdin or stdout */
#define IoTYPE_SOCKET		's'
#define IoTYPE_CLOSED		' '
#define IoTYPE_IMPLICIT		'I'	/* stdin or stdout or stderr */
#define IoTYPE_NUMERIC		'#'	/* fdopen */

/*
=for apidoc Am|bool|SvTAINTED|SV* sv
Checks to see if an SV is tainted.  Returns TRUE if it is, FALSE if
not.

=for apidoc Am|void|SvTAINTED_on|SV* sv
Marks an SV as tainted if tainting is enabled.

=for apidoc Am|void|SvTAINTED_off|SV* sv
Untaints an SV.  Be I<very> careful with this routine, as it short-circuits
some of Perl's fundamental security features.  XS module authors should not
use this function unless they fully understand all the implications of
unconditionally untainting the value. Untainting should be done in the
standard perl fashion, via a carefully crafted regexp, rather than directly
untainting variables.

=for apidoc Am|void|SvTAINT|SV* sv
Taints an SV if tainting is enabled, and if some input to the current
expression is tainted--usually a variable, but possibly also implicit
inputs such as locale settings.  C<SvTAINT> propagates that taintedness to
the outputs of an expression in a pessimistic fashion; i.e., without paying
attention to precisely which outputs are influenced by which inputs.

=cut
*/

#define sv_taint(sv)	  sv_magic((sv), NULL, PERL_MAGIC_taint, NULL, 0)

#define SvTAINTED(sv)	  (SvMAGICAL(sv) && sv_tainted(sv))
#define SvTAINTED_on(sv)  STMT_START{ if(PL_tainting){sv_taint(sv);}   }STMT_END
#define SvTAINTED_off(sv) STMT_START{ if(PL_tainting){sv_untaint(sv);} }STMT_END

#define SvTAINT(sv)			\
    STMT_START {			\
	if (PL_tainting) {		\
	    if (PL_tainted)		\
		SvTAINTED_on(sv);	\
	}				\
    } STMT_END

/*
=for apidoc Am|char*|SvPV_force|SV* sv|STRLEN len
Like C<SvPV> but will force the SV into containing just a string
(C<SvPOK_only>).  You want force if you are going to update the C<SvPVX>
directly.

=for apidoc Am|char*|SvPV_force_nomg|SV* sv|STRLEN len
Like C<SvPV> but will force the SV into containing just a string
(C<SvPOK_only>).  You want force if you are going to update the C<SvPVX>
directly.  Doesn't process magic.

=for apidoc Am|char*|SvPV|SV* sv|STRLEN len
Returns a pointer to the string in the SV, or a stringified form of
the SV if the SV does not contain a string.  The SV may cache the
stringified version becoming C<SvPOK>.  Handles 'get' magic.  See also
C<SvPVx> for a version which guarantees to evaluate sv only once.

=for apidoc Am|char*|SvPVx|SV* sv|STRLEN len
A version of C<SvPV> which guarantees to evaluate C<sv> only once.
Only use this if C<sv> is an expression with side effects, otherwise use the
more efficient C<SvPV>.

=for apidoc Am|char*|SvPV_nomg|SV* sv|STRLEN len
Like C<SvPV> but doesn't process magic.

=for apidoc Am|char*|SvPV_nolen|SV* sv
Returns a pointer to the string in the SV, or a stringified form of
the SV if the SV does not contain a string.  The SV may cache the
stringified form becoming C<SvPOK>.  Handles 'get' magic.

=for apidoc Am|char*|SvPV_nomg_nolen|SV* sv
Like C<SvPV_nolen> but doesn't process magic.

=for apidoc Am|IV|SvIV|SV* sv
Coerces the given SV to an integer and returns it.  See C<SvIVx> for a
version which guarantees to evaluate sv only once.

=for apidoc Am|IV|SvIV_nomg|SV* sv
Like C<SvIV> but doesn't process magic.

=for apidoc Am|IV|SvIVx|SV* sv
Coerces the given SV to an integer and returns it.
Guarantees to evaluate C<sv> only once.  Only use
this if C<sv> is an expression with side effects,
otherwise use the more efficient C<SvIV>.

=for apidoc Am|NV|SvNV|SV* sv
Coerce the given SV to a double and return it.  See C<SvNVx> for a version
which guarantees to evaluate sv only once.

=for apidoc Am|NV|SvNV_nomg|SV* sv
Like C<SvNV> but doesn't process magic.

=for apidoc Am|NV|SvNVx|SV* sv
Coerces the given SV to a double and returns it.
Guarantees to evaluate C<sv> only once.  Only use
this if C<sv> is an expression with side effects,
otherwise use the more efficient C<SvNV>.

=for apidoc Am|UV|SvUV|SV* sv
Coerces the given SV to an unsigned integer and returns it.  See C<SvUVx>
for a version which guarantees to evaluate sv only once.

=for apidoc Am|UV|SvUV_nomg|SV* sv
Like C<SvUV> but doesn't process magic.

=for apidoc Am|UV|SvUVx|SV* sv
Coerces the given SV to an unsigned integer and
returns it.  Guarantees to C<sv> only once.  Only
use this if C<sv> is an expression with side effects,
otherwise use the more efficient C<SvUV>.

=for apidoc Am|bool|SvTRUE|SV* sv
Returns a boolean indicating whether Perl would evaluate the SV as true or
false.  See SvOK() for a defined/undefined test.  Handles 'get' magic
unless the scalar is already SvPOK, SvIOK or SvNOK (the public, not the
private flags).

=for apidoc Am|bool|SvTRUE_nomg|SV* sv
Returns a boolean indicating whether Perl would evaluate the SV as true or
false.  See SvOK() for a defined/undefined test.  Does not handle 'get' magic.

=for apidoc Am|char*|SvPVutf8_force|SV* sv|STRLEN len
Like C<SvPV_force>, but converts sv to utf8 first if necessary.

=for apidoc Am|char*|SvPVutf8|SV* sv|STRLEN len
Like C<SvPV>, but converts sv to utf8 first if necessary.

=for apidoc Am|char*|SvPVutf8_nolen|SV* sv
Like C<SvPV_nolen>, but converts sv to utf8 first if necessary.

=for apidoc Am|char*|SvPVbyte_force|SV* sv|STRLEN len
Like C<SvPV_force>, but converts sv to byte representation first if necessary.

=for apidoc Am|char*|SvPVbyte|SV* sv|STRLEN len
Like C<SvPV>, but converts sv to byte representation first if necessary.

=for apidoc Am|char*|SvPVbyte_nolen|SV* sv
Like C<SvPV_nolen>, but converts sv to byte representation first if necessary.

=for apidoc Am|char*|SvPVutf8x_force|SV* sv|STRLEN len
Like C<SvPV_force>, but converts sv to utf8 first if necessary.
Guarantees to evaluate sv only once; use the more efficient C<SvPVutf8_force>
otherwise.

=for apidoc Am|char*|SvPVutf8x|SV* sv|STRLEN len
Like C<SvPV>, but converts sv to utf8 first if necessary.
Guarantees to evaluate sv only once; use the more efficient C<SvPVutf8>
otherwise.

=for apidoc Am|char*|SvPVbytex_force|SV* sv|STRLEN len
Like C<SvPV_force>, but converts sv to byte representation first if necessary.
Guarantees to evaluate sv only once; use the more efficient C<SvPVbyte_force>
otherwise.

=for apidoc Am|char*|SvPVbytex|SV* sv|STRLEN len
Like C<SvPV>, but converts sv to byte representation first if necessary.
Guarantees to evaluate sv only once; use the more efficient C<SvPVbyte>
otherwise.

=for apidoc Am|bool|SvIsCOW|SV* sv
Returns a boolean indicating whether the SV is Copy-On-Write (either shared
hash key scalars, or full Copy On Write scalars if 5.9.0 is configured for
COW).

=for apidoc Am|bool|SvIsCOW_shared_hash|SV* sv
Returns a boolean indicating whether the SV is Copy-On-Write shared hash key
scalar.

=for apidoc Am|void|sv_catpvn_nomg|SV* sv|const char* ptr|STRLEN len
Like C<sv_catpvn> but doesn't process magic.

=for apidoc Am|void|sv_catpv_nomg|SV* sv|const char* ptr
Like C<sv_catpv> but doesn't process magic.

=for apidoc Am|void|sv_setsv_nomg|SV* dsv|SV* ssv
Like C<sv_setsv> but doesn't process magic.

=for apidoc Am|void|sv_catsv_nomg|SV* dsv|SV* ssv
Like C<sv_catsv> but doesn't process magic.

=for apidoc Amdb|STRLEN|sv_utf8_upgrade_nomg|NN SV *sv

Like sv_utf8_upgrade, but doesn't do magic on C<sv>.

=cut
*/

/* Let us hope that bitmaps for UV and IV are the same */
#define SvIV(sv) (SvIOK(sv) ? SvIVX(sv) : sv_2iv(sv))
#define SvUV(sv) (SvIOK(sv) ? SvUVX(sv) : sv_2uv(sv))
#define SvNV(sv) (SvNOK(sv) ? SvNVX(sv) : sv_2nv(sv))

#define SvIV_nomg(sv) (SvIOK(sv) ? SvIVX(sv) : sv_2iv_flags(sv, 0))
#define SvUV_nomg(sv) (SvIOK(sv) ? SvUVX(sv) : sv_2uv_flags(sv, 0))
#define SvNV_nomg(sv) (SvNOK(sv) ? SvNVX(sv) : sv_2nv_flags(sv, 0))

/* ----*/

#define SvPV(sv, lp) SvPV_flags(sv, lp, SV_GMAGIC)
#define SvPV_const(sv, lp) SvPV_flags_const(sv, lp, SV_GMAGIC)
#define SvPV_mutable(sv, lp) SvPV_flags_mutable(sv, lp, SV_GMAGIC)

#define SvPV_flags(sv, lp, flags) \
    ((SvFLAGS(sv) & (SVf_POK)) == SVf_POK \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_2pv_flags(sv, &lp, flags))
#define SvPV_flags_const(sv, lp, flags) \
    ((SvFLAGS(sv) & (SVf_POK)) == SVf_POK \
     ? ((lp = SvCUR(sv)), SvPVX_const(sv)) : \
     (const char*) sv_2pv_flags(sv, &lp, flags|SV_CONST_RETURN))
#define SvPV_flags_const_nolen(sv, flags) \
    ((SvFLAGS(sv) & (SVf_POK)) == SVf_POK \
     ? SvPVX_const(sv) : \
     (const char*) sv_2pv_flags(sv, 0, flags|SV_CONST_RETURN))
#define SvPV_flags_mutable(sv, lp, flags) \
    ((SvFLAGS(sv) & (SVf_POK)) == SVf_POK \
     ? ((lp = SvCUR(sv)), SvPVX_mutable(sv)) : \
     sv_2pv_flags(sv, &lp, flags|SV_MUTABLE_RETURN))

#define SvPV_force(sv, lp) SvPV_force_flags(sv, lp, SV_GMAGIC)
#define SvPV_force_nolen(sv) SvPV_force_flags_nolen(sv, SV_GMAGIC)
#define SvPV_force_mutable(sv, lp) SvPV_force_flags_mutable(sv, lp, SV_GMAGIC)

#define SvPV_force_nomg(sv, lp) SvPV_force_flags(sv, lp, 0)
#define SvPV_force_nomg_nolen(sv) SvPV_force_flags_nolen(sv, 0)

#define SvPV_force_flags(sv, lp, flags) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_THINKFIRST)) == SVf_POK \
    ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_pvn_force_flags(sv, &lp, flags))
#define SvPV_force_flags_nolen(sv, flags) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_THINKFIRST)) == SVf_POK \
    ? SvPVX(sv) : sv_pvn_force_flags(sv, 0, flags))
#define SvPV_force_flags_mutable(sv, lp, flags) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_THINKFIRST)) == SVf_POK \
    ? ((lp = SvCUR(sv)), SvPVX_mutable(sv)) \
     : sv_pvn_force_flags(sv, &lp, flags|SV_MUTABLE_RETURN))

#define SvPV_nolen(sv) \
    ((SvFLAGS(sv) & (SVf_POK)) == SVf_POK \
     ? SvPVX(sv) : sv_2pv_flags(sv, 0, SV_GMAGIC))

#define SvPV_nomg_nolen(sv) \
    ((SvFLAGS(sv) & (SVf_POK)) == SVf_POK \
     ? SvPVX(sv) : sv_2pv_flags(sv, 0, 0))

#define SvPV_nolen_const(sv) \
    ((SvFLAGS(sv) & (SVf_POK)) == SVf_POK \
     ? SvPVX_const(sv) : sv_2pv_flags(sv, 0, SV_GMAGIC|SV_CONST_RETURN))

#define SvPV_nomg(sv, lp) SvPV_flags(sv, lp, 0)
#define SvPV_nomg_const(sv, lp) SvPV_flags_const(sv, lp, 0)
#define SvPV_nomg_const_nolen(sv) SvPV_flags_const_nolen(sv, 0)

/* ----*/

#define SvPVutf8(sv, lp) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8)) == (SVf_POK|SVf_UTF8) \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_2pvutf8(sv, &lp))

#define SvPVutf8_force(sv, lp) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8|SVf_THINKFIRST)) == (SVf_POK|SVf_UTF8) \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_pvutf8n_force(sv, &lp))


#define SvPVutf8_nolen(sv) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8)) == (SVf_POK|SVf_UTF8)\
     ? SvPVX(sv) : sv_2pvutf8(sv, 0))

/* ----*/

#define SvPVbyte(sv, lp) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8)) == (SVf_POK) \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_2pvbyte(sv, &lp))

#define SvPVbyte_force(sv, lp) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8|SVf_THINKFIRST)) == (SVf_POK) \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_pvbyten_force(sv, &lp))

#define SvPVbyte_nolen(sv) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8)) == (SVf_POK)\
     ? SvPVX(sv) : sv_2pvbyte(sv, 0))


    
/* define FOOx(): idempotent versions of FOO(). If possible, use a local
 * var to evaluate the arg once; failing that, use a global if possible;
 * failing that, call a function to do the work
 */

#define SvPVx_force(sv, lp) sv_pvn_force(sv, &lp)
#define SvPVutf8x_force(sv, lp) sv_pvutf8n_force(sv, &lp)
#define SvPVbytex_force(sv, lp) sv_pvbyten_force(sv, &lp)

#if defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)

#  define SvIVx(sv) ({SV *_sv = MUTABLE_SV(sv); SvIV(_sv); })
#  define SvUVx(sv) ({SV *_sv = MUTABLE_SV(sv); SvUV(_sv); })
#  define SvNVx(sv) ({SV *_sv = MUTABLE_SV(sv); SvNV(_sv); })
#  define SvPVx(sv, lp) ({SV *_sv = (sv); SvPV(_sv, lp); })
#  define SvPVx_const(sv, lp) ({SV *_sv = (sv); SvPV_const(_sv, lp); })
#  define SvPVx_nolen(sv) ({SV *_sv = (sv); SvPV_nolen(_sv); })
#  define SvPVx_nolen_const(sv) ({SV *_sv = (sv); SvPV_nolen_const(_sv); })
#  define SvPVutf8x(sv, lp) ({SV *_sv = (sv); SvPVutf8(_sv, lp); })
#  define SvPVbytex(sv, lp) ({SV *_sv = (sv); SvPVbyte(_sv, lp); })
#  define SvPVbytex_nolen(sv) ({SV *_sv = (sv); SvPVbyte_nolen(_sv); })
#  define SvTRUE(sv) (						\
    !sv								\
    ? 0								\
    :    SvPOK(sv)						\
	?   (({XPV *nxpv = (XPV*)SvANY(sv);			\
	     nxpv &&						\
	     (nxpv->xpv_cur > 1 ||				\
	      (nxpv->xpv_cur && *(sv)->sv_u.svu_pv != '0')); })	\
	     ? 1						\
	     : 0)						\
	:							\
	    SvIOK(sv)						\
	    ? SvIVX(sv) != 0					\
	    :   SvNOK(sv)					\
		? SvNVX(sv) != 0.0				\
		: sv_2bool(sv) )
#  define SvTRUE_nomg(sv) (					\
    !sv								\
    ? 0								\
    :    SvPOK(sv)						\
	?   (({XPV *nxpv = (XPV*)SvANY(sv);			\
	     nxpv &&						\
	     (nxpv->xpv_cur > 1 ||				\
	      (nxpv->xpv_cur && *(sv)->sv_u.svu_pv != '0')); })	\
	     ? 1						\
	     : 0)						\
	:							\
	    SvIOK(sv)						\
	    ? SvIVX(sv) != 0					\
	    :   SvNOK(sv)					\
		? SvNVX(sv) != 0.0				\
		: sv_2bool_flags(sv,0) )
#  define SvTRUEx(sv) ({SV *_sv = (sv); SvTRUE(_sv); })

#else /* __GNUC__ */

/* These inlined macros use globals, which will require a thread
 * declaration in user code, so we avoid them under threads */

#  define SvIVx(sv) ((PL_Sv = (sv)), SvIV(PL_Sv))
#  define SvUVx(sv) ((PL_Sv = (sv)), SvUV(PL_Sv))
#  define SvNVx(sv) ((PL_Sv = (sv)), SvNV(PL_Sv))
#  define SvPVx(sv, lp) ((PL_Sv = (sv)), SvPV(PL_Sv, lp))
#  define SvPVx_const(sv, lp) ((PL_Sv = (sv)), SvPV_const(PL_Sv, lp))
#  define SvPVx_nolen(sv) ((PL_Sv = (sv)), SvPV_nolen(PL_Sv))
#  define SvPVx_nolen_const(sv) ((PL_Sv = (sv)), SvPV_nolen_const(PL_Sv))
#  define SvPVutf8x(sv, lp) ((PL_Sv = (sv)), SvPVutf8(PL_Sv, lp))
#  define SvPVbytex(sv, lp) ((PL_Sv = (sv)), SvPVbyte(PL_Sv, lp))
#  define SvPVbytex_nolen(sv) ((PL_Sv = (sv)), SvPVbyte_nolen(PL_Sv))
#  define SvTRUE(sv) (						\
    !sv								\
    ? 0								\
    :    SvPOK(sv)						\
	?   ((PL_Xpv = (XPV*)SvANY(PL_Sv = (sv))) &&		\
	     (PL_Xpv->xpv_cur > 1 ||				\
	      (PL_Xpv->xpv_cur && *PL_Sv->sv_u.svu_pv != '0'))	\
	     ? 1						\
	     : 0)						\
	:							\
	    SvIOK(sv)						\
	    ? SvIVX(sv) != 0					\
	    :   SvNOK(sv)					\
		? SvNVX(sv) != 0.0				\
		: sv_2bool(sv) )
#  define SvTRUE_nomg(sv) (					\
    !sv								\
    ? 0								\
    :    SvPOK(sv)						\
	?   ((PL_Xpv = (XPV*)SvANY(PL_Sv = (sv))) &&		\
	     (PL_Xpv->xpv_cur > 1 ||				\
	      (PL_Xpv->xpv_cur && *PL_Sv->sv_u.svu_pv != '0'))	\
	     ? 1						\
	     : 0)						\
	:							\
	    SvIOK(sv)						\
	    ? SvIVX(sv) != 0					\
	    :   SvNOK(sv)					\
		? SvNVX(sv) != 0.0				\
		: sv_2bool_flags(sv,0) )
#  define SvTRUEx(sv) ((PL_Sv = (sv)), SvTRUE(PL_Sv))
#endif /* __GNU__ */

#define SvIsCOW(sv)	((SvFLAGS(sv) & (SVf_FAKE | SVf_READONLY)) == \
			   (SVf_FAKE | SVf_READONLY) && !isGV_with_GP(sv) \
			   && SvTYPE(sv) != SVt_REGEXP)
#define SvIsCOW_shared_hash(sv)	(SvIsCOW(sv) && SvLEN(sv) == 0)

#define SvSHARED_HEK_FROM_PV(pvx) \
	((struct hek*)(pvx - STRUCT_OFFSET(struct hek, hek_key)))
#define SvSHARED_HASH(sv) (0 + SvSHARED_HEK_FROM_PV(SvPVX_const(sv))->hek_hash)

/* flag values for sv_*_flags functions */
#define SV_IMMEDIATE_UNREF	1
#define SV_GMAGIC		2
#define SV_COW_DROP_PV		4
#define SV_UTF8_NO_ENCODING	8
#define SV_NOSTEAL		16
#define SV_CONST_RETURN		32
#define SV_MUTABLE_RETURN	64
#define SV_SMAGIC		128
#define SV_HAS_TRAILING_NUL	256
#define SV_COW_SHARED_HASH_KEYS	512
/* This one is only enabled for PERL_OLD_COPY_ON_WRITE */
#define SV_COW_OTHER_PVS	1024
/* Make sv_2pv_flags return NULL if something is undefined.  */
#define SV_UNDEF_RETURNS_NULL	2048
/* Tell sv_utf8_upgrade() to not check to see if an upgrade is really needed.
 * This is used when the caller has already determined it is, and avoids
 * redundant work */
#define SV_FORCE_UTF8_UPGRADE	4096
/* if (after resolving magic etc), the SV is found to be overloaded,
 * don't call the overload magic, just return as-is */
#define SV_SKIP_OVERLOAD	8192
/* It is not yet clear whether we want this as an API, or what the
 * constants should be named. */
#ifdef PERL_CORE
# define SV_CATBYTES		16384
# define SV_CATUTF8		32768
#endif

/* The core is safe for this COW optimisation. XS code on CPAN may not be.
   So only default to doing the COW setup if we're in the core.
 */
#ifdef PERL_CORE
#  ifndef SV_DO_COW_SVSETSV
#    define SV_DO_COW_SVSETSV	SV_COW_SHARED_HASH_KEYS|SV_COW_OTHER_PVS
#  endif
#endif

#ifndef SV_DO_COW_SVSETSV
#  define SV_DO_COW_SVSETSV	0
#endif


#define sv_unref(sv)    	sv_unref_flags(sv, 0)
#define sv_force_normal(sv)	sv_force_normal_flags(sv, 0)
#define sv_usepvn(sv, p, l)	sv_usepvn_flags(sv, p, l, 0)
#define sv_usepvn_mg(sv, p, l)	sv_usepvn_flags(sv, p, l, SV_SMAGIC)

/* We are about to replace the SV's current value. So if it's copy on write
   we need to normalise it. Use the SV_COW_DROP_PV flag hint to say that
   the value is about to get thrown away, so drop the PV rather than go to
   the effort of making a read-write copy only for it to get immediately
   discarded.  */

#define SV_CHECK_THINKFIRST_COW_DROP(sv) if (SvTHINKFIRST(sv)) \
				    sv_force_normal_flags(sv, SV_COW_DROP_PV)

#ifdef PERL_OLD_COPY_ON_WRITE
#define SvRELEASE_IVX(sv)   \
    ((SvIsCOW(sv) ? sv_force_normal_flags(sv, 0) : (void) 0), 0)
#  define SvIsCOW_normal(sv)	(SvIsCOW(sv) && SvLEN(sv))
#  define SvRELEASE_IVX_(sv)	SvRELEASE_IVX(sv),
#else
#  define SvRELEASE_IVX(sv)   0
/* This little game brought to you by the need to shut this warning up:
mg.c: In function 'Perl_magic_get':
mg.c:1024: warning: left-hand operand of comma expression has no effect
*/
#  define SvRELEASE_IVX_(sv)  /**/
#endif /* PERL_OLD_COPY_ON_WRITE */

#define CAN_COW_MASK	(SVs_OBJECT|SVs_GMG|SVs_SMG|SVs_RMG|SVf_IOK|SVf_NOK| \
			 SVf_POK|SVf_ROK|SVp_IOK|SVp_NOK|SVp_POK|SVf_FAKE| \
			 SVf_OOK|SVf_BREAK|SVf_READONLY)
#define CAN_COW_FLAGS	(SVp_POK|SVf_POK)

#define SV_CHECK_THINKFIRST(sv) if (SvTHINKFIRST(sv)) \
				    sv_force_normal_flags(sv, 0)


/* all these 'functions' are now just macros */

#define sv_pv(sv) SvPV_nolen(sv)
#define sv_pvutf8(sv) SvPVutf8_nolen(sv)
#define sv_pvbyte(sv) SvPVbyte_nolen(sv)

#define sv_pvn_force_nomg(sv, lp) sv_pvn_force_flags(sv, lp, 0)
#define sv_utf8_upgrade_flags(sv, flags) sv_utf8_upgrade_flags_grow(sv, flags, 0)
#define sv_utf8_upgrade_nomg(sv) sv_utf8_upgrade_flags(sv, 0)
#define sv_catpvn_nomg(dsv, sstr, slen) sv_catpvn_flags(dsv, sstr, slen, 0)
#define sv_catpv_nomg(dsv, sstr) sv_catpv_flags(dsv, sstr, 0)
#define sv_setsv(dsv, ssv) \
	sv_setsv_flags(dsv, ssv, SV_GMAGIC|SV_DO_COW_SVSETSV)
#define sv_setsv_nomg(dsv, ssv) sv_setsv_flags(dsv, ssv, SV_DO_COW_SVSETSV)
#define sv_catsv(dsv, ssv) sv_catsv_flags(dsv, ssv, SV_GMAGIC)
#define sv_catsv_nomg(dsv, ssv) sv_catsv_flags(dsv, ssv, 0)
#define sv_catsv_mg(dsv, ssv) sv_catsv_flags(dsv, ssv, SV_GMAGIC|SV_SMAGIC)
#define sv_catpvn(dsv, sstr, slen) sv_catpvn_flags(dsv, sstr, slen, SV_GMAGIC)
#define sv_catpvn_mg(sv, sstr, slen) \
	sv_catpvn_flags(sv, sstr, slen, SV_GMAGIC|SV_SMAGIC);
#define sv_2pv(sv, lp) sv_2pv_flags(sv, lp, SV_GMAGIC)
#define sv_2pv_nolen(sv) sv_2pv(sv, 0)
#define sv_2pvbyte_nolen(sv) sv_2pvbyte(sv, 0)
#define sv_2pvutf8_nolen(sv) sv_2pvutf8(sv, 0)
#define sv_2pv_nomg(sv, lp) sv_2pv_flags(sv, lp, 0)
#define sv_pvn_force(sv, lp) sv_pvn_force_flags(sv, lp, SV_GMAGIC)
#define sv_utf8_upgrade(sv) sv_utf8_upgrade_flags(sv, SV_GMAGIC)
#define sv_2iv(sv) sv_2iv_flags(sv, SV_GMAGIC)
#define sv_2uv(sv) sv_2uv_flags(sv, SV_GMAGIC)
#define sv_2nv(sv) sv_2nv_flags(sv, SV_GMAGIC)
#define sv_eq(sv1, sv2) sv_eq_flags(sv1, sv2, SV_GMAGIC)
#define sv_cmp(sv1, sv2) sv_cmp_flags(sv1, sv2, SV_GMAGIC)
#define sv_cmp_locale(sv1, sv2) sv_cmp_locale_flags(sv1, sv2, SV_GMAGIC)
#define sv_collxfrm(sv, nxp) sv_cmp_flags(sv, nxp, SV_GMAGIC)
#define sv_2bool(sv) sv_2bool_flags(sv, SV_GMAGIC)
#define sv_insert(bigstr, offset, len, little, littlelen)		\
	Perl_sv_insert_flags(aTHX_ (bigstr),(offset), (len), (little),	\
			     (littlelen), SV_GMAGIC)

/* Should be named SvCatPVN_utf8_upgrade? */
#define sv_catpvn_utf8_upgrade(dsv, sstr, slen, nsv)	\
	STMT_START {					\
	    if (!(nsv))					\
		nsv = newSVpvn_flags(sstr, slen, SVs_TEMP);	\
	    else					\
		sv_setpvn(nsv, sstr, slen);		\
	    SvUTF8_off(nsv);				\
	    sv_utf8_upgrade(nsv);			\
	    sv_catsv(dsv, nsv);	\
	} STMT_END

/*
=for apidoc Am|SV*|newRV_inc|SV* sv

Creates an RV wrapper for an SV.  The reference count for the original SV is
incremented.

=cut
*/

#define newRV_inc(sv)	newRV(sv)

/* the following macros update any magic values this sv is associated with */

/*
=head1 Magical Functions

=for apidoc Am|void|SvGETMAGIC|SV* sv
Invokes C<mg_get> on an SV if it has 'get' magic.  This macro evaluates its
argument more than once.

=for apidoc Am|void|SvSETMAGIC|SV* sv
Invokes C<mg_set> on an SV if it has 'set' magic.  This macro evaluates its
argument more than once.

=for apidoc Am|void|SvSetSV|SV* dsb|SV* ssv
Calls C<sv_setsv> if dsv is not the same as ssv.  May evaluate arguments
more than once.

=for apidoc Am|void|SvSetSV_nosteal|SV* dsv|SV* ssv
Calls a non-destructive version of C<sv_setsv> if dsv is not the same as
ssv.  May evaluate arguments more than once.

=for apidoc Am|void|SvSetMagicSV|SV* dsb|SV* ssv
Like C<SvSetSV>, but does any set magic required afterwards.

=for apidoc Am|void|SvSetMagicSV_nosteal|SV* dsv|SV* ssv
Like C<SvSetSV_nosteal>, but does any set magic required afterwards.

=for apidoc Am|void|SvSHARE|SV* sv
Arranges for sv to be shared between threads if a suitable module
has been loaded.

=for apidoc Am|void|SvLOCK|SV* sv
Arranges for a mutual exclusion lock to be obtained on sv if a suitable module
has been loaded.

=for apidoc Am|void|SvUNLOCK|SV* sv
Releases a mutual exclusion lock on sv if a suitable module
has been loaded.

=head1 SV Manipulation Functions

=for apidoc Am|char *|SvGROW|SV* sv|STRLEN len
Expands the character buffer in the SV so that it has room for the
indicated number of bytes (remember to reserve space for an extra trailing
NUL character).  Calls C<sv_grow> to perform the expansion if necessary.
Returns a pointer to the character buffer.

=cut
*/

#define SvSHARE(sv) PL_sharehook(aTHX_ sv)
#define SvLOCK(sv) PL_lockhook(aTHX_ sv)
#define SvUNLOCK(sv) PL_unlockhook(aTHX_ sv)
#define SvDESTROYABLE(sv) PL_destroyhook(aTHX_ sv)

#define SvGETMAGIC(x) ((void)(SvGMAGICAL(x) && mg_get(x)))
#define SvSETMAGIC(x) STMT_START { if (SvSMAGICAL(x)) mg_set(x); } STMT_END

#define SvSetSV_and(dst,src,finally) \
	STMT_START {					\
	    if ((dst) != (src)) {			\
		sv_setsv(dst, src);			\
		finally;				\
	    }						\
	} STMT_END
#define SvSetSV_nosteal_and(dst,src,finally) \
	STMT_START {					\
	    if ((dst) != (src)) {			\
		sv_setsv_flags(dst, src, SV_GMAGIC | SV_NOSTEAL | SV_DO_COW_SVSETSV);	\
		finally;				\
	    }						\
	} STMT_END

#define SvSetSV(dst,src) \
		SvSetSV_and(dst,src,/*nothing*/;)
#define SvSetSV_nosteal(dst,src) \
		SvSetSV_nosteal_and(dst,src,/*nothing*/;)

#define SvSetMagicSV(dst,src) \
		SvSetSV_and(dst,src,SvSETMAGIC(dst))
#define SvSetMagicSV_nosteal(dst,src) \
		SvSetSV_nosteal_and(dst,src,SvSETMAGIC(dst))


#if !defined(SKIP_DEBUGGING)
#define SvPEEK(sv) sv_peek(sv)
#else
#define SvPEEK(sv) ""
#endif

#define SvIMMORTAL(sv) ((sv)==&PL_sv_undef || (sv)==&PL_sv_yes || (sv)==&PL_sv_no || (sv)==&PL_sv_placeholder)

/*
=for apidoc Am|SV *|boolSV|bool b

Returns a true SV if C<b> is a true value, or a false SV if C<b> is 0.

See also C<PL_sv_yes> and C<PL_sv_no>.

=cut
*/

#define boolSV(b) ((b) ? &PL_sv_yes : &PL_sv_no)

#define isGV(sv) (SvTYPE(sv) == SVt_PVGV)
/* If I give every macro argument a different name, then there won't be bugs
   where nested macros get confused. Been there, done that.  */
#define isGV_with_GP(pwadak) \
	(((SvFLAGS(pwadak) & (SVp_POK|SVpgv_GP)) == SVpgv_GP)	\
	&& (SvTYPE(pwadak) == SVt_PVGV || SvTYPE(pwadak) == SVt_PVLV))
#define isGV_with_GP_on(sv)	STMT_START {			       \
	assert (SvTYPE(sv) == SVt_PVGV || SvTYPE(sv) == SVt_PVLV); \
	assert (!SvPOKp(sv));					       \
	assert (!SvIOKp(sv));					       \
	(SvFLAGS(sv) |= SVpgv_GP);				       \
    } STMT_END
#define isGV_with_GP_off(sv)	STMT_START {			       \
	assert (SvTYPE(sv) == SVt_PVGV || SvTYPE(sv) == SVt_PVLV); \
	assert (!SvPOKp(sv));					       \
	assert (!SvIOKp(sv));					       \
	(SvFLAGS(sv) &= ~SVpgv_GP);				       \
    } STMT_END


#define SvGROW(sv,len) (SvLEN(sv) < (len) ? sv_grow(sv,len) : SvPVX(sv))
#define SvGROW_mutable(sv,len) \
    (SvLEN(sv) < (len) ? sv_grow(sv,len) : SvPVX_mutable(sv))
#define Sv_Grow sv_grow

#define CLONEf_COPY_STACKS 1
#define CLONEf_KEEP_PTR_TABLE 2
#define CLONEf_CLONE_HOST 4
#define CLONEf_JOIN_IN 8

struct clone_params {
  AV* stashes;
  UV  flags;
  PerlInterpreter *proto_perl;
  PerlInterpreter *new_perl;
  AV *unreferenced;
};

/*
=for apidoc Am|SV*|newSVpvn_utf8|NULLOK const char* s|STRLEN len|U32 utf8

Creates a new SV and copies a string into it.  If utf8 is true, calls
C<SvUTF8_on> on the new SV.  Implemented as a wrapper around C<newSVpvn_flags>.

=cut
*/

#define newSVpvn_utf8(s, len, u) newSVpvn_flags((s), (len), (u) ? SVf_UTF8 : 0)

/*
=for apidoc Am|void|SvOOK_offset|NN SV*sv|STRLEN len

Reads into I<len> the offset from SvPVX back to the true start of the
allocated buffer, which will be non-zero if C<sv_chop> has been used to
efficiently remove characters from start of the buffer.  Implemented as a
macro, which takes the address of I<len>, which must be of type C<STRLEN>.
Evaluates I<sv> more than once.  Sets I<len> to 0 if C<SvOOK(sv)> is false.

=cut
*/

#ifdef DEBUGGING
/* Does the bot know something I don't?
10:28 <@Nicholas> metabatman
10:28 <+meta> Nicholas: crash
*/
#  define SvOOK_offset(sv, offset) STMT_START {				\
	assert(sizeof(offset) == sizeof(STRLEN));			\
	if (SvOOK(sv)) {						\
	    const U8 *crash = (U8*)SvPVX_const(sv);			\
	    offset = *--crash;						\
 	    if (!offset) {						\
		crash -= sizeof(STRLEN);				\
		Copy(crash, (U8 *)&offset, sizeof(STRLEN), U8);		\
	    }								\
	    {								\
		/* Validate the preceding buffer's sentinels to		\
		   verify that no-one is using it.  */			\
		const U8 *const bonk = (U8 *) SvPVX_const(sv) - offset;	\
		while (crash > bonk) {					\
		    --crash;						\
		    assert (*crash == (U8)PTR2UV(crash));		\
		}							\
	    }								\
	} else {							\
	    offset = 0;							\
	}								\
    } STMT_END
#else
    /* This is the same code, but avoids using any temporary variables:  */
#  define SvOOK_offset(sv, offset) STMT_START {				\
	assert(sizeof(offset) == sizeof(STRLEN));			\
	if (SvOOK(sv)) {						\
	    offset = ((U8*)SvPVX_const(sv))[-1];			\
	    if (!offset) {						\
		Copy(SvPVX_const(sv) - 1 - sizeof(STRLEN),		\
		     (U8 *)&offset, sizeof(STRLEN), U8);		\
	    }								\
	} else {							\
	    offset = 0;							\
	}								\
    } STMT_END
#endif

#define newIO()	MUTABLE_IO(newSV_type(SVt_PVIO))

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
