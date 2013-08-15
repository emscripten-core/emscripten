/*    pad.h
 *
 *    Copyright (C) 2002, 2003, 2005, 2006, 2007, 2008,
 *    2009, 2010, 2011 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * This file defines the types and macros associated with the API for
 * manipulating scratchpads, which are used by perl to store lexical
 * variables, op targets and constants.
 */

/*
=head1 Pad Data Structures
*/


/* a padlist is currently just an AV; but that might change,
 * so hide the type. Ditto a pad.  */

typedef AV PADLIST;
typedef AV PAD;


/* offsets within a pad */

#if PTRSIZE == 4
typedef U32TYPE PADOFFSET;
#else
#   if PTRSIZE == 8
typedef U64TYPE PADOFFSET;
#   endif
#endif
#define NOT_IN_PAD ((PADOFFSET) -1)

/* a value that PL_cop_seqmax is guaranteed never to be,
 * flagging that a lexical is being introduced, or has not yet left scope
 */
#define PERL_PADSEQ_INTRO  U32_MAX


/* B.xs needs these for the benefit of B::Deparse */
/* Low range end is exclusive (valid from the cop seq after this one) */
/* High range end is inclusive (valid up to this cop seq) */

#if defined (DEBUGGING) && defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
#  define COP_SEQ_RANGE_LOW(sv)						\
	(({ const SV *const _sv_cop_seq_range_low = (const SV *) (sv);	\
	  assert(SvTYPE(_sv_cop_seq_range_low) == SVt_NV		\
		 || SvTYPE(_sv_cop_seq_range_low) >= SVt_PVNV);		\
	  assert(SvTYPE(_sv_cop_seq_range_low) != SVt_PVAV);		\
	  assert(SvTYPE(_sv_cop_seq_range_low) != SVt_PVHV);		\
	  assert(SvTYPE(_sv_cop_seq_range_low) != SVt_PVCV);		\
	  assert(SvTYPE(_sv_cop_seq_range_low) != SVt_PVFM);		\
	  assert(!isGV_with_GP(_sv_cop_seq_range_low));			\
	  ((XPVNV*) MUTABLE_PTR(SvANY(_sv_cop_seq_range_low)))->xnv_u.xpad_cop_seq.xlow; \
	 }))
#  define COP_SEQ_RANGE_HIGH(sv)					\
	(({ const SV *const _sv_cop_seq_range_high = (const SV *) (sv);	\
	  assert(SvTYPE(_sv_cop_seq_range_high) == SVt_NV 		\
                 || SvTYPE(_sv_cop_seq_range_high) >= SVt_PVNV);	\
	  assert(SvTYPE(_sv_cop_seq_range_high) != SVt_PVAV);		\
	  assert(SvTYPE(_sv_cop_seq_range_high) != SVt_PVHV);		\
	  assert(SvTYPE(_sv_cop_seq_range_high) != SVt_PVCV);		\
	  assert(SvTYPE(_sv_cop_seq_range_high) != SVt_PVFM);		\
	  assert(!isGV_with_GP(_sv_cop_seq_range_high));		\
	  ((XPVNV*) MUTABLE_PTR(SvANY(_sv_cop_seq_range_high)))->xnv_u.xpad_cop_seq.xhigh; \
	 }))
#  define PARENT_PAD_INDEX(sv)						\
	(({ const SV *const _sv_parent_pad_index = (const SV *) (sv);	\
	  assert(SvTYPE(_sv_parent_pad_index) == SVt_NV			\
		 || SvTYPE(_sv_parent_pad_index) >= SVt_PVNV);		\
	  assert(SvTYPE(_sv_parent_pad_index) != SVt_PVAV);		\
	  assert(SvTYPE(_sv_parent_pad_index) != SVt_PVHV);		\
	  assert(SvTYPE(_sv_parent_pad_index) != SVt_PVCV);		\
	  assert(SvTYPE(_sv_parent_pad_index) != SVt_PVFM);		\
	  assert(!isGV_with_GP(_sv_parent_pad_index));			\
	  ((XPVNV*) MUTABLE_PTR(SvANY(_sv_parent_pad_index)))->xnv_u.xpad_cop_seq.xlow; \
	 }))
#  define PARENT_FAKELEX_FLAGS(sv)					\
	(({ const SV *const _sv_parent_fakelex_flags = (const SV *) (sv); \
	  assert(SvTYPE(_sv_parent_fakelex_flags) == SVt_NV  		\
		 || SvTYPE(_sv_parent_fakelex_flags) >= SVt_PVNV);	\
	  assert(SvTYPE(_sv_parent_fakelex_flags) != SVt_PVAV);		\
	  assert(SvTYPE(_sv_parent_fakelex_flags) != SVt_PVHV);		\
	  assert(SvTYPE(_sv_parent_fakelex_flags) != SVt_PVCV);		\
	  assert(SvTYPE(_sv_parent_fakelex_flags) != SVt_PVFM);		\
	  assert(!isGV_with_GP(_sv_parent_fakelex_flags));		\
	  ((XPVNV*) MUTABLE_PTR(SvANY(_sv_parent_fakelex_flags)))->xnv_u.xpad_cop_seq.xhigh; \
	 }))
#else
#  define COP_SEQ_RANGE_LOW(sv)		\
	(0 + (((XPVNV*) SvANY(sv))->xnv_u.xpad_cop_seq.xlow))
#  define COP_SEQ_RANGE_HIGH(sv)	\
	(0 + (((XPVNV*) SvANY(sv))->xnv_u.xpad_cop_seq.xhigh))


#  define PARENT_PAD_INDEX(sv)		\
	(0 + (((XPVNV*) SvANY(sv))->xnv_u.xpad_cop_seq.xlow))
#  define PARENT_FAKELEX_FLAGS(sv)	\
	(0 + (((XPVNV*) SvANY(sv))->xnv_u.xpad_cop_seq.xhigh))
#endif

/* Flags set in the SvIVX field of FAKE namesvs */

#define PAD_FAKELEX_ANON   1 /* the lex is declared in an ANON, or ... */
#define PAD_FAKELEX_MULTI  2 /* the lex can be instantiated multiple times */

/* flags for the pad_new() function */

#define padnew_CLONE	1	/* this pad is for a cloned CV */
#define padnew_SAVE	2	/* save old globals */
#define padnew_SAVESUB	4	/* also save extra stuff for start of sub */

/* values for the pad_tidy() function */

typedef enum {
	padtidy_SUB,		/* tidy up a pad for a sub, */
	padtidy_SUBCLONE,	/* a cloned sub, */
	padtidy_FORMAT		/* or a format */
} padtidy_type;

/* flags for pad_add_name_pvn. */

#define padadd_OUR		0x01	   /* our declaration. */
#define padadd_STATE		0x02	   /* state declaration. */
#define padadd_NO_DUP_CHECK	0x04	   /* skip warning on dups. */
#define padadd_UTF8_NAME	SVf_UTF8   /* name is UTF-8 encoded. */

/* ASSERT_CURPAD_LEGAL and ASSERT_CURPAD_ACTIVE respectively determine
 * whether PL_comppad and PL_curpad are consistent and whether they have
 * active values */

#ifndef PERL_MAD
#  define pad_peg(label)
#endif

#ifdef DEBUGGING
#  define ASSERT_CURPAD_LEGAL(label) \
    pad_peg(label); \
    if (PL_comppad ? (AvARRAY(PL_comppad) != PL_curpad) : (PL_curpad != 0))  \
	Perl_croak(aTHX_ "panic: illegal pad in %s: 0x%"UVxf"[0x%"UVxf"]",\
	    label, PTR2UV(PL_comppad), PTR2UV(PL_curpad));


#  define ASSERT_CURPAD_ACTIVE(label) \
    pad_peg(label); \
    if (!PL_comppad || (AvARRAY(PL_comppad) != PL_curpad))		  \
	Perl_croak(aTHX_ "panic: invalid pad in %s: 0x%"UVxf"[0x%"UVxf"]",\
	    label, PTR2UV(PL_comppad), PTR2UV(PL_curpad));
#else
#  define ASSERT_CURPAD_LEGAL(label)
#  define ASSERT_CURPAD_ACTIVE(label)
#endif



/* Note: the following three macros are actually defined in scope.h, but
 * they are documented here for completeness, since they directly or
 * indirectly affect pads.

=for apidoc m|void|SAVEPADSV	|PADOFFSET po
Save a pad slot (used to restore after an iteration)

XXX DAPM it would make more sense to make the arg a PADOFFSET
=for apidoc m|void|SAVECLEARSV	|SV **svp
Clear the pointed to pad value on scope exit. (i.e. the runtime action of 'my')

=for apidoc m|void|SAVECOMPPAD
save PL_comppad and PL_curpad





=for apidoc m|SV *|PAD_SETSV	|PADOFFSET po|SV* sv
Set the slot at offset C<po> in the current pad to C<sv>

=for apidoc m|void|PAD_SV	|PADOFFSET po
Get the value at offset C<po> in the current pad

=for apidoc m|SV *|PAD_SVl	|PADOFFSET po
Lightweight and lvalue version of C<PAD_SV>.
Get or set the value at offset C<po> in the current pad.
Unlike C<PAD_SV>, does not print diagnostics with -DX.
For internal use only.

=for apidoc m|SV *|PAD_BASE_SV	|PADLIST padlist|PADOFFSET po
Get the value from slot C<po> in the base (DEPTH=1) pad of a padlist

=for apidoc m|void|PAD_SET_CUR	|PADLIST padlist|I32 n
Set the current pad to be pad C<n> in the padlist, saving
the previous current pad. NB currently this macro expands to a string too
long for some compilers, so it's best to replace it with

    SAVECOMPPAD();
    PAD_SET_CUR_NOSAVE(padlist,n);


=for apidoc m|void|PAD_SET_CUR_NOSAVE	|PADLIST padlist|I32 n
like PAD_SET_CUR, but without the save

=for apidoc m|void|PAD_SAVE_SETNULLPAD
Save the current pad then set it to null.

=for apidoc m|void|PAD_SAVE_LOCAL|PAD *opad|PAD *npad
Save the current pad to the local variable opad, then make the
current pad equal to npad

=for apidoc m|void|PAD_RESTORE_LOCAL|PAD *opad
Restore the old pad saved into the local variable opad by PAD_SAVE_LOCAL()

=cut
*/

#ifdef DEBUGGING
#  define PAD_SV(po)	   pad_sv(po)
#  define PAD_SETSV(po,sv) pad_setsv(po,sv)
#else
#  define PAD_SV(po)       (PL_curpad[po])
#  define PAD_SETSV(po,sv) PL_curpad[po] = (sv)
#endif

#define PAD_SVl(po)       (PL_curpad[po])

#define PAD_BASE_SV(padlist, po) \
	(AvARRAY(padlist)[1]) 	\
	? AvARRAY(MUTABLE_AV((AvARRAY(padlist)[1])))[po] : NULL;


#define PAD_SET_CUR_NOSAVE(padlist,nth) \
	PL_comppad = (PAD*) (AvARRAY(padlist)[nth]);		\
	PL_curpad = AvARRAY(PL_comppad);			\
	DEBUG_Xv(PerlIO_printf(Perl_debug_log,			\
	      "Pad 0x%"UVxf"[0x%"UVxf"] set_cur    depth=%d\n",	\
	      PTR2UV(PL_comppad), PTR2UV(PL_curpad), (int)(nth)));


#define PAD_SET_CUR(padlist,nth) \
	SAVECOMPPAD();						\
	PAD_SET_CUR_NOSAVE(padlist,nth);


#define PAD_SAVE_SETNULLPAD()	SAVECOMPPAD(); \
	PL_comppad = NULL; PL_curpad = NULL;	\
	DEBUG_Xv(PerlIO_printf(Perl_debug_log, "Pad set_null\n"));

#define PAD_SAVE_LOCAL(opad,npad) \
	opad = PL_comppad;					\
	PL_comppad = (npad);					\
	PL_curpad =  PL_comppad ? AvARRAY(PL_comppad) : NULL;	\
	DEBUG_Xv(PerlIO_printf(Perl_debug_log,			\
	      "Pad 0x%"UVxf"[0x%"UVxf"] save_local\n",		\
	      PTR2UV(PL_comppad), PTR2UV(PL_curpad)));

#define PAD_RESTORE_LOCAL(opad) \
        assert(!opad || !SvIS_FREED(opad));					\
	PL_comppad = opad;						\
	PL_curpad =  PL_comppad ? AvARRAY(PL_comppad) : NULL;	\
	DEBUG_Xv(PerlIO_printf(Perl_debug_log,			\
	      "Pad 0x%"UVxf"[0x%"UVxf"] restore_local\n",	\
	      PTR2UV(PL_comppad), PTR2UV(PL_curpad)));


/*
=for apidoc m|void|CX_CURPAD_SAVE|struct context
Save the current pad in the given context block structure.

=for apidoc m|SV *|CX_CURPAD_SV|struct context|PADOFFSET po
Access the SV at offset po in the saved current pad in the given
context block structure (can be used as an lvalue).

=cut
*/

#define CX_CURPAD_SAVE(block)  (block).oldcomppad = PL_comppad
#define CX_CURPAD_SV(block,po) (AvARRAY(MUTABLE_AV(((block).oldcomppad)))[po])


/*
=for apidoc m|U32|PAD_COMPNAME_FLAGS|PADOFFSET po
Return the flags for the current compiling pad name
at offset C<po>. Assumes a valid slot entry.

=for apidoc m|char *|PAD_COMPNAME_PV|PADOFFSET po
Return the name of the current compiling pad name
at offset C<po>. Assumes a valid slot entry.

=for apidoc m|HV *|PAD_COMPNAME_TYPE|PADOFFSET po
Return the type (stash) of the current compiling pad name at offset
C<po>. Must be a valid name. Returns null if not typed.

=for apidoc m|HV *|PAD_COMPNAME_OURSTASH|PADOFFSET po
Return the stash associated with an C<our> variable.
Assumes the slot entry is a valid C<our> lexical.

=for apidoc m|STRLEN|PAD_COMPNAME_GEN|PADOFFSET po
The generation number of the name at offset C<po> in the current
compiling pad (lvalue). Note that C<SvUVX> is hijacked for this purpose.

=for apidoc m|STRLEN|PAD_COMPNAME_GEN_set|PADOFFSET po|int gen
Sets the generation number of the name at offset C<po> in the current
ling pad (lvalue) to C<gen>.  Note that C<SvUV_set> is hijacked for this purpose.

=cut

*/

#define PAD_COMPNAME_SV(po) (*av_fetch(PL_comppad_name, (po), FALSE))
#define PAD_COMPNAME_FLAGS(po) SvFLAGS(PAD_COMPNAME_SV(po))
#define PAD_COMPNAME_FLAGS_isOUR(po) SvPAD_OUR(PAD_COMPNAME_SV(po))
#define PAD_COMPNAME_PV(po) SvPV_nolen(PAD_COMPNAME_SV(po))

#define PAD_COMPNAME_TYPE(po) pad_compname_type(po)

#define PAD_COMPNAME_OURSTASH(po) \
    (SvOURSTASH(PAD_COMPNAME_SV(po)))

#define PAD_COMPNAME_GEN(po) ((STRLEN)SvUVX(AvARRAY(PL_comppad_name)[po]))

#define PAD_COMPNAME_GEN_set(po, gen) SvUV_set(AvARRAY(PL_comppad_name)[po], (UV)(gen))


/*
=for apidoc m|void|PAD_CLONE_VARS|PerlInterpreter *proto_perl|CLONE_PARAMS* param
Clone the state variables associated with running and compiling pads.

=cut
*/

/* NB - we set PL_comppad to null unless it points at a value that
 * has already been dup'ed, ie it points to part of an active padlist.
 * Otherwise PL_comppad ends up being a leaked scalar in code like
 * the following:
 *     threads->create(sub { threads->create(sub {...} ) } );
 * where the second thread dups the outer sub's comppad but not the
 * sub's CV or padlist. */

#define PAD_CLONE_VARS(proto_perl, param)				\
    PL_comppad			= av_dup(proto_perl->Icomppad, param);	\
    PL_curpad = PL_comppad ?  AvARRAY(PL_comppad) : NULL;		\
    PL_comppad_name		= av_dup(proto_perl->Icomppad_name, param); \
    PL_comppad_name_fill	= proto_perl->Icomppad_name_fill;	\
    PL_comppad_name_floor	= proto_perl->Icomppad_name_floor;	\
    PL_min_intro_pending	= proto_perl->Imin_intro_pending;	\
    PL_max_intro_pending	= proto_perl->Imax_intro_pending;	\
    PL_padix			= proto_perl->Ipadix;			\
    PL_padix_floor		= proto_perl->Ipadix_floor;		\
    PL_pad_reset_pending	= proto_perl->Ipad_reset_pending;	\
    PL_cop_seqmax		= proto_perl->Icop_seqmax;

/*
=for apidoc Am|PADOFFSET|pad_add_name_pvs|const char *name|U32 flags|HV *typestash|HV *ourstash

Exactly like L</pad_add_name_pvn>, but takes a literal string instead
of a string/length pair.

=cut
*/

#define pad_add_name_pvs(name,flags,typestash,ourstash) \
    Perl_pad_add_name_pvn(aTHX_ STR_WITH_LEN(name), flags, typestash, ourstash)

/*
=for apidoc Am|PADOFFSET|pad_findmy_pvs|const char *name|U32 flags

Exactly like L</pad_findmy_pvn>, but takes a literal string instead
of a string/length pair.

=cut
*/

#define pad_findmy_pvs(name,flags) \
    Perl_pad_findmy_pvn(aTHX_ STR_WITH_LEN(name), flags)

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
