/*    gv.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *   'Mercy!' cried Gandalf.  'If the giving of information is to be the cure
 * of your inquisitiveness, I shall spend all the rest of my days in answering
 * you.  What more do you want to know?'
 *   'The names of all the stars, and of all living things, and the whole
 * history of Middle-earth and Over-heaven and of the Sundering Seas,'
 * laughed Pippin.
 *
 *     [p.599 of _The Lord of the Rings_, III/xi: "The Palantír"]
 */

/*
=head1 GV Functions

A GV is a structure which corresponds to to a Perl typeglob, ie *foo.
It is a structure that holds a pointer to a scalar, an array, a hash etc,
corresponding to $foo, @foo, %foo.

GVs are usually found as values in stashes (symbol table hashes) where
Perl stores its global variables.

=cut
*/

#include "EXTERN.h"
#define PERL_IN_GV_C
#include "perl.h"
#include "overload.c"
#include "keywords.h"
#include "feature.h"

static const char S_autoload[] = "AUTOLOAD";
static const STRLEN S_autolen = sizeof(S_autoload)-1;

GV *
Perl_gv_add_by_type(pTHX_ GV *gv, svtype type)
{
    SV **where;

    if (
        !gv
     || (
            SvTYPE((const SV *)gv) != SVt_PVGV
         && SvTYPE((const SV *)gv) != SVt_PVLV
        )
    ) {
	const char *what;
	if (type == SVt_PVIO) {
	    /*
	     * if it walks like a dirhandle, then let's assume that
	     * this is a dirhandle.
	     */
	    what = OP_IS_DIRHOP(PL_op->op_type) ?
		"dirhandle" : "filehandle";
	} else if (type == SVt_PVHV) {
	    what = "hash";
	} else {
	    what = type == SVt_PVAV ? "array" : "scalar";
	}
	/* diag_listed_as: Bad symbol for filehandle */
	Perl_croak(aTHX_ "Bad symbol for %s", what);
    }

    if (type == SVt_PVHV) {
	where = (SV **)&GvHV(gv);
    } else if (type == SVt_PVAV) {
	where = (SV **)&GvAV(gv);
    } else if (type == SVt_PVIO) {
	where = (SV **)&GvIOp(gv);
    } else {
	where = &GvSV(gv);
    }

    if (!*where)
	*where = newSV_type(type);
    return gv;
}

GV *
Perl_gv_fetchfile(pTHX_ const char *name)
{
    PERL_ARGS_ASSERT_GV_FETCHFILE;
    return gv_fetchfile_flags(name, strlen(name), 0);
}

GV *
Perl_gv_fetchfile_flags(pTHX_ const char *const name, const STRLEN namelen,
			const U32 flags)
{
    dVAR;
    char smallbuf[128];
    char *tmpbuf;
    const STRLEN tmplen = namelen + 2;
    GV *gv;

    PERL_ARGS_ASSERT_GV_FETCHFILE_FLAGS;
    PERL_UNUSED_ARG(flags);

    if (!PL_defstash)
	return NULL;

    if (tmplen <= sizeof smallbuf)
	tmpbuf = smallbuf;
    else
	Newx(tmpbuf, tmplen, char);
    /* This is where the debugger's %{"::_<$filename"} hash is created */
    tmpbuf[0] = '_';
    tmpbuf[1] = '<';
    memcpy(tmpbuf + 2, name, namelen);
    gv = *(GV**)hv_fetch(PL_defstash, tmpbuf, tmplen, TRUE);
    if (!isGV(gv)) {
	gv_init(gv, PL_defstash, tmpbuf, tmplen, FALSE);
#ifdef PERL_DONT_CREATE_GVSV
	GvSV(gv) = newSVpvn(name, namelen);
#else
	sv_setpvn(GvSV(gv), name, namelen);
#endif
    }
    if ((PERLDB_LINE || PERLDB_SAVESRC) && !GvAV(gv))
	    hv_magic(GvHVn(gv_AVadd(gv)), NULL, PERL_MAGIC_dbfile);
    if (tmpbuf != smallbuf)
	Safefree(tmpbuf);
    return gv;
}

/*
=for apidoc gv_const_sv

If C<gv> is a typeglob whose subroutine entry is a constant sub eligible for
inlining, or C<gv> is a placeholder reference that would be promoted to such
a typeglob, then returns the value returned by the sub.  Otherwise, returns
NULL.

=cut
*/

SV *
Perl_gv_const_sv(pTHX_ GV *gv)
{
    PERL_ARGS_ASSERT_GV_CONST_SV;

    if (SvTYPE(gv) == SVt_PVGV)
	return cv_const_sv(GvCVu(gv));
    return SvROK(gv) ? SvRV(gv) : NULL;
}

GP *
Perl_newGP(pTHX_ GV *const gv)
{
    GP *gp;
    U32 hash;
#ifdef USE_ITHREADS
    const char *const file
	= (PL_curcop && CopFILE(PL_curcop)) ? CopFILE(PL_curcop) : "";
    const STRLEN len = strlen(file);
#else
    SV *const temp_sv = CopFILESV(PL_curcop);
    const char *file;
    STRLEN len;

    PERL_ARGS_ASSERT_NEWGP;

    if (temp_sv) {
	file = SvPVX(temp_sv);
	len = SvCUR(temp_sv);
    } else {
	file = "";
	len = 0;
    }
#endif

    PERL_HASH(hash, file, len);

    Newxz(gp, 1, GP);

#ifndef PERL_DONT_CREATE_GVSV
    gp->gp_sv = newSV(0);
#endif

    gp->gp_line = PL_curcop ? CopLINE(PL_curcop) : 0;
    /* XXX Ideally this cast would be replaced with a change to const char*
       in the struct.  */
    gp->gp_file_hek = share_hek(file, len, hash);
    gp->gp_egv = gv;
    gp->gp_refcnt = 1;

    return gp;
}

/* Assign CvGV(cv) = gv, handling weak references.
 * See also S_anonymise_cv_maybe */

void
Perl_cvgv_set(pTHX_ CV* cv, GV* gv)
{
    GV * const oldgv = CvGV(cv);
    PERL_ARGS_ASSERT_CVGV_SET;

    if (oldgv == gv)
	return;

    if (oldgv) {
	if (CvCVGV_RC(cv)) {
	    SvREFCNT_dec(oldgv);
	    CvCVGV_RC_off(cv);
	}
	else {
	    sv_del_backref(MUTABLE_SV(oldgv), MUTABLE_SV(cv));
	}
    }

    SvANY(cv)->xcv_gv = gv;
    assert(!CvCVGV_RC(cv));

    if (!gv)
	return;

    if (isGV_with_GP(gv) && GvGP(gv) && (GvCV(gv) == cv || GvFORM(gv) == cv))
	Perl_sv_add_backref(aTHX_ MUTABLE_SV(gv), MUTABLE_SV(cv));
    else {
	CvCVGV_RC_on(cv);
	SvREFCNT_inc_simple_void_NN(gv);
    }
}

/* Assign CvSTASH(cv) = st, handling weak references. */

void
Perl_cvstash_set(pTHX_ CV *cv, HV *st)
{
    HV *oldst = CvSTASH(cv);
    PERL_ARGS_ASSERT_CVSTASH_SET;
    if (oldst == st)
	return;
    if (oldst)
	sv_del_backref(MUTABLE_SV(oldst), MUTABLE_SV(cv));
    SvANY(cv)->xcv_stash = st;
    if (st)
	Perl_sv_add_backref(aTHX_ MUTABLE_SV(st), MUTABLE_SV(cv));
}

/*
=for apidoc gv_init_pvn

Converts a scalar into a typeglob.  This is an incoercible typeglob;
assigning a reference to it will assign to one of its slots, instead of
overwriting it as happens with typeglobs created by SvSetSV.  Converting
any scalar that is SvOK() may produce unpredictable results and is reserved
for perl's internal use.

C<gv> is the scalar to be converted.

C<stash> is the parent stash/package, if any.

C<name> and C<len> give the name.  The name must be unqualified;
that is, it must not include the package name.  If C<gv> is a
stash element, it is the caller's responsibility to ensure that the name
passed to this function matches the name of the element.  If it does not
match, perl's internal bookkeeping will get out of sync.

C<flags> can be set to SVf_UTF8 if C<name> is a UTF8 string, or
the return value of SvUTF8(sv).  It can also take the
GV_ADDMULTI flag, which means to pretend that the GV has been
seen before (i.e., suppress "Used once" warnings).

=for apidoc gv_init

The old form of gv_init_pvn().  It does not work with UTF8 strings, as it
has no flags parameter.  If the C<multi> parameter is set, the
GV_ADDMULTI flag will be passed to gv_init_pvn().

=for apidoc gv_init_pv

Same as gv_init_pvn(), but takes a nul-terminated string for the name
instead of separate char * and length parameters.

=for apidoc gv_init_sv

Same as gv_init_pvn(), but takes an SV * for the name instead of separate
char * and length parameters.  C<flags> is currently unused.

=cut
*/

void
Perl_gv_init_sv(pTHX_ GV *gv, HV *stash, SV* namesv, U32 flags)
{
   char *namepv;
   STRLEN namelen;
   PERL_ARGS_ASSERT_GV_INIT_SV;
   namepv = SvPV(namesv, namelen);
   if (SvUTF8(namesv))
       flags |= SVf_UTF8;
   gv_init_pvn(gv, stash, namepv, namelen, flags);
}

void
Perl_gv_init_pv(pTHX_ GV *gv, HV *stash, const char *name, U32 flags)
{
   PERL_ARGS_ASSERT_GV_INIT_PV;
   gv_init_pvn(gv, stash, name, strlen(name), flags);
}

void
Perl_gv_init_pvn(pTHX_ GV *gv, HV *stash, const char *name, STRLEN len, U32 flags)
{
    dVAR;
    const U32 old_type = SvTYPE(gv);
    const bool doproto = old_type > SVt_NULL;
    char * const proto = (doproto && SvPOK(gv))
	? ((void)(SvIsCOW(gv) && (sv_force_normal((SV *)gv), 0)), SvPVX(gv))
	: NULL;
    const STRLEN protolen = proto ? SvCUR(gv) : 0;
    const U32 proto_utf8  = proto ? SvUTF8(gv) : 0;
    SV *const has_constant = doproto && SvROK(gv) ? SvRV(gv) : NULL;
    const U32 exported_constant = has_constant ? SvPCS_IMPORTED(gv) : 0;

    PERL_ARGS_ASSERT_GV_INIT_PVN;
    assert (!(proto && has_constant));

    if (has_constant) {
	/* The constant has to be a simple scalar type.  */
	switch (SvTYPE(has_constant)) {
	case SVt_PVAV:
	case SVt_PVHV:
	case SVt_PVCV:
	case SVt_PVFM:
	case SVt_PVIO:
            Perl_croak(aTHX_ "Cannot convert a reference to %s to typeglob",
		       sv_reftype(has_constant, 0));
	default: NOOP;
	}
	SvRV_set(gv, NULL);
	SvROK_off(gv);
    }


    if (old_type < SVt_PVGV) {
	if (old_type >= SVt_PV)
	    SvCUR_set(gv, 0);
	sv_upgrade(MUTABLE_SV(gv), SVt_PVGV);
    }
    if (SvLEN(gv)) {
	if (proto) {
	    SvPV_set(gv, NULL);
	    SvLEN_set(gv, 0);
	    SvPOK_off(gv);
	} else
	    Safefree(SvPVX_mutable(gv));
    }
    SvIOK_off(gv);
    isGV_with_GP_on(gv);

    GvGP_set(gv, Perl_newGP(aTHX_ gv));
    GvSTASH(gv) = stash;
    if (stash)
	Perl_sv_add_backref(aTHX_ MUTABLE_SV(stash), MUTABLE_SV(gv));
    gv_name_set(gv, name, len, GV_ADD | ( flags & SVf_UTF8 ? SVf_UTF8 : 0 ));
    if (flags & GV_ADDMULTI || doproto)	/* doproto means it */
	GvMULTI_on(gv);			/* _was_ mentioned */
    if (doproto) {			/* Replicate part of newSUB here. */
	CV *cv;
	ENTER;
	if (has_constant) {
	    /* newCONSTSUB takes ownership of the reference from us.  */
	    cv = newCONSTSUB_flags(stash, name, len, flags, has_constant);
	    /* In case op.c:S_process_special_blocks stole it: */
	    if (!GvCV(gv))
		GvCV_set(gv, (CV *)SvREFCNT_inc_simple_NN(cv));
	    assert(GvCV(gv) == cv); /* newCONSTSUB should have set this */
	    /* If this reference was a copy of another, then the subroutine
	       must have been "imported", by a Perl space assignment to a GV
	       from a reference to CV.  */
	    if (exported_constant)
		GvIMPORTED_CV_on(gv);
	} else {
	    (void) start_subparse(0,0);	/* Create empty CV in compcv. */
	    cv = PL_compcv;
	    GvCV_set(gv,cv);
	}
	LEAVE;

        mro_method_changed_in(GvSTASH(gv)); /* sub Foo::bar($) { (shift) } sub ASDF::baz($); *ASDF::baz = \&Foo::bar */
	CvGV_set(cv, gv);
	CvFILE_set_from_cop(cv, PL_curcop);
	CvSTASH_set(cv, PL_curstash);
	if (proto) {
	    sv_usepvn_flags(MUTABLE_SV(cv), proto, protolen,
			    SV_HAS_TRAILING_NUL);
            if ( proto_utf8 ) SvUTF8_on(MUTABLE_SV(cv));
	}
    }
}

STATIC void
S_gv_init_svtype(pTHX_ GV *gv, const svtype sv_type)
{
    PERL_ARGS_ASSERT_GV_INIT_SVTYPE;

    switch (sv_type) {
    case SVt_PVIO:
	(void)GvIOn(gv);
	break;
    case SVt_PVAV:
	(void)GvAVn(gv);
	break;
    case SVt_PVHV:
	(void)GvHVn(gv);
	break;
#ifdef PERL_DONT_CREATE_GVSV
    case SVt_NULL:
    case SVt_PVCV:
    case SVt_PVFM:
    case SVt_PVGV:
	break;
    default:
	if(GvSVn(gv)) {
	    /* Work round what appears to be a bug in Sun C++ 5.8 2005/10/13
	       If we just cast GvSVn(gv) to void, it ignores evaluating it for
	       its side effect */
	}
#endif
    }
}

static void core_xsub(pTHX_ CV* cv);

static GV *
S_maybe_add_coresub(pTHX_ HV * const stash, GV *gv,
                          const char * const name, const STRLEN len)
{
    const int code = keyword(name, len, 1);
    static const char file[] = __FILE__;
    CV *cv, *oldcompcv = NULL;
    int opnum = 0;
    SV *opnumsv;
    bool ampable = TRUE; /* &{}-able */
    COP *oldcurcop = NULL;
    yy_parser *oldparser = NULL;
    I32 oldsavestack_ix = 0;

    assert(gv || stash);
    assert(name);

    if (code >= 0) return NULL; /* not overridable */
    switch (-code) {
     /* no support for \&CORE::infix;
        no support for funcs that take labels, as their parsing is
        weird  */
    case KEY_and: case KEY_cmp: case KEY_CORE: case KEY_dump:
    case KEY_eq: case KEY_ge:
    case KEY_gt: case KEY_le: case KEY_lt: case KEY_ne:
    case KEY_or: case KEY_x: case KEY_xor:
	return NULL;
    case KEY_chdir:
    case KEY_chomp: case KEY_chop:
    case KEY_each: case KEY_eof: case KEY_exec:
    case KEY_keys:
    case KEY_lstat:
    case KEY_pop:
    case KEY_push:
    case KEY_shift:
    case KEY_splice:
    case KEY_stat:
    case KEY_system:
    case KEY_truncate: case KEY_unlink:
    case KEY_unshift:
    case KEY_values:
	ampable = FALSE;
    }
    if (!gv) {
	gv = (GV *)newSV(0);
	gv_init(gv, stash, name, len, TRUE);
    }
    GvMULTI_on(gv);
    if (ampable) {
	ENTER;
	oldcurcop = PL_curcop;
	oldparser = PL_parser;
	lex_start(NULL, NULL, 0);
	oldcompcv = PL_compcv;
	PL_compcv = NULL; /* Prevent start_subparse from setting
	                     CvOUTSIDE. */
	oldsavestack_ix = start_subparse(FALSE,0);
	cv = PL_compcv;
    }
    else {
	/* Avoid calling newXS, as it calls us, and things start to
	   get hairy. */
	cv = MUTABLE_CV(newSV_type(SVt_PVCV));
	GvCV_set(gv,cv);
	GvCVGEN(gv) = 0;
	mro_method_changed_in(GvSTASH(gv));
	CvISXSUB_on(cv);
	CvXSUB(cv) = core_xsub;
    }
    CvGV_set(cv, gv); /* This stops new ATTRSUB from setting CvFILE
                         from PL_curcop. */
    (void)gv_fetchfile(file);
    CvFILE(cv) = (char *)file;
    /* XXX This is inefficient, as doing things this order causes
           a prototype check in newATTRSUB.  But we have to do
           it this order as we need an op number before calling
           new ATTRSUB. */
    (void)core_prototype((SV *)cv, name, code, &opnum);
    if (stash)
	(void)hv_store(stash,name,len,(SV *)gv,0);
    if (ampable) {
	CvLVALUE_on(cv);
	newATTRSUB_flags(
		   oldsavestack_ix, (OP *)gv,
	           NULL,NULL,
	           coresub_op(
	             opnum
	               ? newSVuv((UV)opnum)
	               : newSVpvn(name,len),
	             code, opnum
	           ),
	           1
	);
	assert(GvCV(gv) == cv);
	if (opnum != OP_VEC && opnum != OP_SUBSTR)
	    CvLVALUE_off(cv); /* Now *that* was a neat trick. */
	LEAVE;
	PL_parser = oldparser;
	PL_curcop = oldcurcop;
	PL_compcv = oldcompcv;
    }
    opnumsv = opnum ? newSVuv((UV)opnum) : (SV *)NULL;
    cv_set_call_checker(
       cv, Perl_ck_entersub_args_core, opnumsv ? opnumsv : (SV *)cv
    );
    SvREFCNT_dec(opnumsv);
    return gv;
}

/*
=for apidoc gv_fetchmeth

Like L</gv_fetchmeth_pvn>, but lacks a flags parameter.

=for apidoc gv_fetchmeth_sv

Exactly like L</gv_fetchmeth_pvn>, but takes the name string in the form
of an SV instead of a string/length pair.

=cut
*/

GV *
Perl_gv_fetchmeth_sv(pTHX_ HV *stash, SV *namesv, I32 level, U32 flags)
{
   char *namepv;
   STRLEN namelen;
   PERL_ARGS_ASSERT_GV_FETCHMETH_SV;
   namepv = SvPV(namesv, namelen);
   if (SvUTF8(namesv))
       flags |= SVf_UTF8;
   return gv_fetchmeth_pvn(stash, namepv, namelen, level, flags);
}

/*
=for apidoc gv_fetchmeth_pv

Exactly like L</gv_fetchmeth_pvn>, but takes a nul-terminated string 
instead of a string/length pair.

=cut
*/

GV *
Perl_gv_fetchmeth_pv(pTHX_ HV *stash, const char *name, I32 level, U32 flags)
{
    PERL_ARGS_ASSERT_GV_FETCHMETH_PV;
    return gv_fetchmeth_pvn(stash, name, strlen(name), level, flags);
}

/*
=for apidoc gv_fetchmeth_pvn

Returns the glob with the given C<name> and a defined subroutine or
C<NULL>.  The glob lives in the given C<stash>, or in the stashes
accessible via @ISA and UNIVERSAL::.

The argument C<level> should be either 0 or -1.  If C<level==0>, as a
side-effect creates a glob with the given C<name> in the given C<stash>
which in the case of success contains an alias for the subroutine, and sets
up caching info for this glob.

Currently, the only significant value for C<flags> is SVf_UTF8.

This function grants C<"SUPER"> token as a postfix of the stash name. The
GV returned from C<gv_fetchmeth> may be a method cache entry, which is not
visible to Perl code.  So when calling C<call_sv>, you should not use
the GV directly; instead, you should use the method's CV, which can be
obtained from the GV with the C<GvCV> macro.

=cut
*/

/* NOTE: No support for tied ISA */

GV *
Perl_gv_fetchmeth_pvn(pTHX_ HV *stash, const char *name, STRLEN len, I32 level, U32 flags)
{
    dVAR;
    GV** gvp;
    AV* linear_av;
    SV** linear_svp;
    SV* linear_sv;
    HV* cstash;
    GV* candidate = NULL;
    CV* cand_cv = NULL;
    GV* topgv = NULL;
    const char *hvname;
    I32 create = (level >= 0) ? 1 : 0;
    I32 items;
    STRLEN packlen;
    U32 topgen_cmp;
    U32 is_utf8 = flags & SVf_UTF8;

    PERL_ARGS_ASSERT_GV_FETCHMETH_PVN;

    /* UNIVERSAL methods should be callable without a stash */
    if (!stash) {
	create = 0;  /* probably appropriate */
	if(!(stash = gv_stashpvs("UNIVERSAL", 0)))
	    return 0;
    }

    assert(stash);

    hvname = HvNAME_get(stash);
    if (!hvname)
      Perl_croak(aTHX_ "Can't use anonymous symbol table for method lookup");

    assert(hvname);
    assert(name);

    DEBUG_o( Perl_deb(aTHX_ "Looking for method %s in package %s\n",name,hvname) );

    topgen_cmp = HvMROMETA(stash)->cache_gen + PL_sub_generation;

    /* check locally for a real method or a cache entry */
    gvp = (GV**)hv_fetch(stash, name, is_utf8 ? -(I32)len : (I32)len, create);
    if(gvp) {
        topgv = *gvp;
      have_gv:
        assert(topgv);
        if (SvTYPE(topgv) != SVt_PVGV)
            gv_init_pvn(topgv, stash, name, len, GV_ADDMULTI|is_utf8);
        if ((cand_cv = GvCV(topgv))) {
            /* If genuine method or valid cache entry, use it */
            if (!GvCVGEN(topgv) || GvCVGEN(topgv) == topgen_cmp) {
                return topgv;
            }
            else {
                /* stale cache entry, junk it and move on */
	        SvREFCNT_dec(cand_cv);
	        GvCV_set(topgv, NULL);
		cand_cv = NULL;
	        GvCVGEN(topgv) = 0;
            }
        }
        else if (GvCVGEN(topgv) == topgen_cmp) {
            /* cache indicates no such method definitively */
            return 0;
        }
	else if (len > 1 /* shortest is uc */ && HvNAMELEN_get(stash) == 4
              && strnEQ(hvname, "CORE", 4)
              && S_maybe_add_coresub(aTHX_ NULL,topgv,name,len))
	    goto have_gv;
    }

    packlen = HvNAMELEN_get(stash);
    if (packlen >= 7 && strEQ(hvname + packlen - 7, "::SUPER")) {
        HV* basestash;
        packlen -= 7;
        basestash = gv_stashpvn(hvname, packlen,
                                GV_ADD | (HvNAMEUTF8(stash) ? SVf_UTF8 : 0));
        linear_av = mro_get_linear_isa(basestash);
    }
    else {
        linear_av = mro_get_linear_isa(stash); /* has ourselves at the top of the list */
    }

    linear_svp = AvARRAY(linear_av) + 1; /* skip over self */
    items = AvFILLp(linear_av); /* no +1, to skip over self */
    while (items--) {
        linear_sv = *linear_svp++;
        assert(linear_sv);
        cstash = gv_stashsv(linear_sv, 0);

        if (!cstash) {
	    Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
                           "Can't locate package %"SVf" for @%"HEKf"::ISA",
			   SVfARG(linear_sv),
                           HEKfARG(HvNAME_HEK(stash)));
            continue;
        }

        assert(cstash);

        gvp = (GV**)hv_fetch(cstash, name, is_utf8 ? -(I32)len : (I32)len, 0);
        if (!gvp) {
            if (len > 1 && HvNAMELEN_get(cstash) == 4) {
                const char *hvname = HvNAME(cstash); assert(hvname);
                if (strnEQ(hvname, "CORE", 4)
                 && (candidate =
                      S_maybe_add_coresub(aTHX_ cstash,NULL,name,len)
                    ))
                    goto have_candidate;
            }
            continue;
        }
        else candidate = *gvp;
       have_candidate:
        assert(candidate);
        if (SvTYPE(candidate) != SVt_PVGV)
            gv_init_pvn(candidate, cstash, name, len, GV_ADDMULTI|is_utf8);
        if (SvTYPE(candidate) == SVt_PVGV && (cand_cv = GvCV(candidate)) && !GvCVGEN(candidate)) {
            /*
             * Found real method, cache method in topgv if:
             *  1. topgv has no synonyms (else inheritance crosses wires)
             *  2. method isn't a stub (else AUTOLOAD fails spectacularly)
             */
            if (topgv && (GvREFCNT(topgv) == 1) && (CvROOT(cand_cv) || CvXSUB(cand_cv))) {
                  CV *old_cv = GvCV(topgv);
                  SvREFCNT_dec(old_cv);
                  SvREFCNT_inc_simple_void_NN(cand_cv);
                  GvCV_set(topgv, cand_cv);
                  GvCVGEN(topgv) = topgen_cmp;
            }
	    return candidate;
        }
    }

    /* Check UNIVERSAL without caching */
    if(level == 0 || level == -1) {
        candidate = gv_fetchmeth_pvn(NULL, name, len, 1, flags);
        if(candidate) {
            cand_cv = GvCV(candidate);
            if (topgv && (GvREFCNT(topgv) == 1) && (CvROOT(cand_cv) || CvXSUB(cand_cv))) {
                  CV *old_cv = GvCV(topgv);
                  SvREFCNT_dec(old_cv);
                  SvREFCNT_inc_simple_void_NN(cand_cv);
                  GvCV_set(topgv, cand_cv);
                  GvCVGEN(topgv) = topgen_cmp;
            }
            return candidate;
        }
    }

    if (topgv && GvREFCNT(topgv) == 1) {
        /* cache the fact that the method is not defined */
        GvCVGEN(topgv) = topgen_cmp;
    }

    return 0;
}

/*
=for apidoc gv_fetchmeth_autoload

This is the old form of L</gv_fetchmeth_pvn_autoload>, which has no flags
parameter.

=for apidoc gv_fetchmeth_sv_autoload

Exactly like L</gv_fetchmeth_pvn_autoload>, but takes the name string in the form
of an SV instead of a string/length pair.

=cut
*/

GV *
Perl_gv_fetchmeth_sv_autoload(pTHX_ HV *stash, SV *namesv, I32 level, U32 flags)
{
   char *namepv;
   STRLEN namelen;
   PERL_ARGS_ASSERT_GV_FETCHMETH_SV_AUTOLOAD;
   namepv = SvPV(namesv, namelen);
   if (SvUTF8(namesv))
       flags |= SVf_UTF8;
   return gv_fetchmeth_pvn_autoload(stash, namepv, namelen, level, flags);
}

/*
=for apidoc gv_fetchmeth_pv_autoload

Exactly like L</gv_fetchmeth_pvn_autoload>, but takes a nul-terminated string
instead of a string/length pair.

=cut
*/

GV *
Perl_gv_fetchmeth_pv_autoload(pTHX_ HV *stash, const char *name, I32 level, U32 flags)
{
    PERL_ARGS_ASSERT_GV_FETCHMETH_PV_AUTOLOAD;
    return gv_fetchmeth_pvn_autoload(stash, name, strlen(name), level, flags);
}

/*
=for apidoc gv_fetchmeth_pvn_autoload

Same as gv_fetchmeth_pvn(), but looks for autoloaded subroutines too.
Returns a glob for the subroutine.

For an autoloaded subroutine without a GV, will create a GV even
if C<level < 0>.  For an autoloaded subroutine without a stub, GvCV()
of the result may be zero.

Currently, the only significant value for C<flags> is SVf_UTF8.

=cut
*/

GV *
Perl_gv_fetchmeth_pvn_autoload(pTHX_ HV *stash, const char *name, STRLEN len, I32 level, U32 flags)
{
    GV *gv = gv_fetchmeth_pvn(stash, name, len, level, flags);

    PERL_ARGS_ASSERT_GV_FETCHMETH_PVN_AUTOLOAD;

    if (!gv) {
	CV *cv;
	GV **gvp;

	if (!stash)
	    return NULL;	/* UNIVERSAL::AUTOLOAD could cause trouble */
	if (len == S_autolen && memEQ(name, S_autoload, S_autolen))
	    return NULL;
	if (!(gv = gv_fetchmeth_pvn(stash, S_autoload, S_autolen, FALSE, flags)))
	    return NULL;
	cv = GvCV(gv);
	if (!(CvROOT(cv) || CvXSUB(cv)))
	    return NULL;
	/* Have an autoload */
	if (level < 0)	/* Cannot do without a stub */
	    gv_fetchmeth_pvn(stash, name, len, 0, flags);
	gvp = (GV**)hv_fetch(stash, name,
                        (flags & SVf_UTF8) ? -(I32)len : (I32)len, (level >= 0));
	if (!gvp)
	    return NULL;
	return *gvp;
    }
    return gv;
}

/*
=for apidoc gv_fetchmethod_autoload

Returns the glob which contains the subroutine to call to invoke the method
on the C<stash>.  In fact in the presence of autoloading this may be the
glob for "AUTOLOAD".  In this case the corresponding variable $AUTOLOAD is
already setup.

The third parameter of C<gv_fetchmethod_autoload> determines whether
AUTOLOAD lookup is performed if the given method is not present: non-zero
means yes, look for AUTOLOAD; zero means no, don't look for AUTOLOAD.
Calling C<gv_fetchmethod> is equivalent to calling C<gv_fetchmethod_autoload>
with a non-zero C<autoload> parameter.

These functions grant C<"SUPER"> token as a prefix of the method name. Note
that if you want to keep the returned glob for a long time, you need to
check for it being "AUTOLOAD", since at the later time the call may load a
different subroutine due to $AUTOLOAD changing its value. Use the glob
created via a side effect to do this.

These functions have the same side-effects and as C<gv_fetchmeth> with
C<level==0>.  C<name> should be writable if contains C<':'> or C<'
''>. The warning against passing the GV returned by C<gv_fetchmeth> to
C<call_sv> apply equally to these functions.

=cut
*/

STATIC HV*
S_gv_get_super_pkg(pTHX_ const char* name, I32 namelen, U32 flags)
{
    AV* superisa;
    GV** gvp;
    GV* gv;
    HV* stash;

    PERL_ARGS_ASSERT_GV_GET_SUPER_PKG;

    stash = gv_stashpvn(name, namelen, flags);
    if(stash) return stash;

    /* If we must create it, give it an @ISA array containing
       the real package this SUPER is for, so that it's tied
       into the cache invalidation code correctly */
    stash = gv_stashpvn(name, namelen, GV_ADD | flags);
    gvp = (GV**)hv_fetchs(stash, "ISA", TRUE);
    gv = *gvp;
    gv_init(gv, stash, "ISA", 3, TRUE);
    superisa = GvAVn(gv);
    GvMULTI_on(gv);
    sv_magic(MUTABLE_SV(superisa), MUTABLE_SV(gv), PERL_MAGIC_isa, NULL, 0);
#ifdef USE_ITHREADS
    av_push(superisa, newSVpvn_flags(CopSTASHPV(PL_curcop),
                                     CopSTASH_len(PL_curcop) < 0
					? -CopSTASH_len(PL_curcop)
					:  CopSTASH_len(PL_curcop),
                                     SVf_UTF8*(CopSTASH_len(PL_curcop) < 0)
                                    ));
#else
    av_push(superisa, newSVhek(CopSTASH(PL_curcop)
			       ? HvNAME_HEK(CopSTASH(PL_curcop)) : NULL));
#endif

    return stash;
}

GV *
Perl_gv_fetchmethod_autoload(pTHX_ HV *stash, const char *name, I32 autoload)
{
    PERL_ARGS_ASSERT_GV_FETCHMETHOD_AUTOLOAD;

    return gv_fetchmethod_flags(stash, name, autoload ? GV_AUTOLOAD : 0);
}

GV *
Perl_gv_fetchmethod_sv_flags(pTHX_ HV *stash, SV *namesv, U32 flags)
{
    char *namepv;
    STRLEN namelen;
    PERL_ARGS_ASSERT_GV_FETCHMETHOD_SV_FLAGS;
    namepv = SvPV(namesv, namelen);
    if (SvUTF8(namesv))
       flags |= SVf_UTF8;
    return gv_fetchmethod_pvn_flags(stash, namepv, namelen, flags);
}

GV *
Perl_gv_fetchmethod_pv_flags(pTHX_ HV *stash, const char *name, U32 flags)
{
    PERL_ARGS_ASSERT_GV_FETCHMETHOD_PV_FLAGS;
    return gv_fetchmethod_pvn_flags(stash, name, strlen(name), flags);
}

/* Don't merge this yet, as it's likely to get a len parameter, and possibly
   even a U32 hash */
GV *
Perl_gv_fetchmethod_pvn_flags(pTHX_ HV *stash, const char *name, const STRLEN len, U32 flags)
{
    dVAR;
    register const char *nend;
    const char *nsplit = NULL;
    GV* gv;
    HV* ostash = stash;
    const char * const origname = name;
    SV *const error_report = MUTABLE_SV(stash);
    const U32 autoload = flags & GV_AUTOLOAD;
    const U32 do_croak = flags & GV_CROAK;
    const U32 is_utf8  = flags & SVf_UTF8;

    PERL_ARGS_ASSERT_GV_FETCHMETHOD_PVN_FLAGS;

    if (SvTYPE(stash) < SVt_PVHV)
	stash = NULL;
    else {
	/* The only way stash can become NULL later on is if nsplit is set,
	   which in turn means that there is no need for a SVt_PVHV case
	   the error reporting code.  */
    }

    for (nend = name; *nend || nend != (origname + len); nend++) {
	if (*nend == '\'') {
	    nsplit = nend;
	    name = nend + 1;
	}
	else if (*nend == ':' && *(nend + 1) == ':') {
	    nsplit = nend++;
	    name = nend + 1;
	}
    }
    if (nsplit) {
	if ((nsplit - origname) == 5 && memEQ(origname, "SUPER", 5)) {
	    /* ->SUPER::method should really be looked up in original stash */
	    SV * const tmpstr = sv_2mortal(Perl_newSVpvf(aTHX_
		     "%"HEKf"::SUPER",
		      HEKfARG(HvNAME_HEK((HV*)CopSTASH(PL_curcop)))
	    ));
	    /* __PACKAGE__::SUPER stash should be autovivified */
	    stash = gv_get_super_pkg(SvPVX_const(tmpstr), SvCUR(tmpstr), SvUTF8(tmpstr));
	    DEBUG_o( Perl_deb(aTHX_ "Treating %s as %s::%s\n",
			 origname, HvNAME_get(stash), name) );
	}
	else {
            /* don't autovifify if ->NoSuchStash::method */
            stash = gv_stashpvn(origname, nsplit - origname, is_utf8);

	    /* however, explicit calls to Pkg::SUPER::method may
	       happen, and may require autovivification to work */
	    if (!stash && (nsplit - origname) >= 7 &&
		strnEQ(nsplit - 7, "::SUPER", 7) &&
		gv_stashpvn(origname, nsplit - origname - 7, is_utf8))
	      stash = gv_get_super_pkg(origname, nsplit - origname, flags);
	}
	ostash = stash;
    }

    gv = gv_fetchmeth_pvn(stash, name, nend - name, 0, flags);
    if (!gv) {
	if (strEQ(name,"import") || strEQ(name,"unimport"))
	    gv = MUTABLE_GV(&PL_sv_yes);
	else if (autoload)
	    gv = gv_autoload_pvn(
		ostash, name, nend - name, GV_AUTOLOAD_ISMETHOD|flags
	    );
	if (!gv && do_croak) {
	    /* Right now this is exclusively for the benefit of S_method_common
	       in pp_hot.c  */
	    if (stash) {
		/* If we can't find an IO::File method, it might be a call on
		 * a filehandle. If IO:File has not been loaded, try to
		 * require it first instead of croaking */
		const char *stash_name = HvNAME_get(stash);
		if (stash_name && memEQs(stash_name, HvNAMELEN_get(stash), "IO::File")
		    && !Perl_hv_common(aTHX_ GvHVn(PL_incgv), NULL,
				       STR_WITH_LEN("IO/File.pm"), 0,
				       HV_FETCH_ISEXISTS, NULL, 0)
		) {
		    require_pv("IO/File.pm");
		    gv = gv_fetchmeth_pvn(stash, name, nend - name, 0, flags);
		    if (gv)
			return gv;
		}
		Perl_croak(aTHX_
			   "Can't locate object method \"%"SVf
			   "\" via package \"%"HEKf"\"",
			            SVfARG(newSVpvn_flags(name, nend - name,
                                           SVs_TEMP | is_utf8)),
                                    HEKfARG(HvNAME_HEK(stash)));
	    }
	    else {
                SV* packnamesv;

		if (nsplit) {
		    packnamesv = newSVpvn_flags(origname, nsplit - origname,
                                                    SVs_TEMP | is_utf8);
		} else {
		    packnamesv = sv_2mortal(newSVsv(error_report));
		}

		Perl_croak(aTHX_
			   "Can't locate object method \"%"SVf"\" via package \"%"SVf"\""
			   " (perhaps you forgot to load \"%"SVf"\"?)",
			   SVfARG(newSVpvn_flags(name, nend - name,
                                SVs_TEMP | is_utf8)),
                           SVfARG(packnamesv), SVfARG(packnamesv));
	    }
	}
    }
    else if (autoload) {
	CV* const cv = GvCV(gv);
	if (!CvROOT(cv) && !CvXSUB(cv)) {
	    GV* stubgv;
	    GV* autogv;

	    if (CvANON(cv))
		stubgv = gv;
	    else {
		stubgv = CvGV(cv);
		if (GvCV(stubgv) != cv)		/* orphaned import */
		    stubgv = gv;
	    }
            autogv = gv_autoload_pvn(GvSTASH(stubgv),
                                  GvNAME(stubgv), GvNAMELEN(stubgv),
                                  GV_AUTOLOAD_ISMETHOD
                                   | (GvNAMEUTF8(stubgv) ? SVf_UTF8 : 0));
	    if (autogv)
		gv = autogv;
	}
    }

    return gv;
}

GV*
Perl_gv_autoload_sv(pTHX_ HV *stash, SV* namesv, U32 flags)
{
   char *namepv;
   STRLEN namelen;
   PERL_ARGS_ASSERT_GV_AUTOLOAD_SV;
   namepv = SvPV(namesv, namelen);
   if (SvUTF8(namesv))
       flags |= SVf_UTF8;
   return gv_autoload_pvn(stash, namepv, namelen, flags);
}

GV*
Perl_gv_autoload_pv(pTHX_ HV *stash, const char *namepv, U32 flags)
{
   PERL_ARGS_ASSERT_GV_AUTOLOAD_PV;
   return gv_autoload_pvn(stash, namepv, strlen(namepv), flags);
}

GV*
Perl_gv_autoload_pvn(pTHX_ HV *stash, const char *name, STRLEN len, U32 flags)
{
    dVAR;
    GV* gv;
    CV* cv;
    HV* varstash;
    GV* vargv;
    SV* varsv;
    SV *packname = NULL;
    U32 is_utf8 = flags & SVf_UTF8 ? SVf_UTF8 : 0;

    PERL_ARGS_ASSERT_GV_AUTOLOAD_PVN;

    if (len == S_autolen && memEQ(name, S_autoload, S_autolen))
	return NULL;
    if (stash) {
	if (SvTYPE(stash) < SVt_PVHV) {
            STRLEN packname_len = 0;
            const char * const packname_ptr = SvPV_const(MUTABLE_SV(stash), packname_len);
            packname = newSVpvn_flags(packname_ptr, packname_len,
                                      SVs_TEMP | SvUTF8(stash));
	    stash = NULL;
	}
	else
	    packname = sv_2mortal(newSVhek(HvNAME_HEK(stash)));
    }
    if (!(gv = gv_fetchmeth_pvn(stash, S_autoload, S_autolen, FALSE, is_utf8)))
	return NULL;
    cv = GvCV(gv);

    if (!(CvROOT(cv) || CvXSUB(cv)))
	return NULL;

    /*
     * Inheriting AUTOLOAD for non-methods works ... for now.
     */
    if (
        !(flags & GV_AUTOLOAD_ISMETHOD)
     && (GvCVGEN(gv) || GvSTASH(gv) != stash)
    )
	Perl_ck_warner_d(aTHX_ packWARN(WARN_DEPRECATED),
			 "Use of inherited AUTOLOAD for non-method %"SVf"::%"SVf"() is deprecated",
			 SVfARG(packname),
                         SVfARG(newSVpvn_flags(name, len, SVs_TEMP | is_utf8)));

    if (CvISXSUB(cv)) {
        /* Instead of forcing the XSUB do another lookup for $AUTOLOAD
         * and split that value on the last '::', pass along the same data
         * via the SvPVX field in the CV, and the stash in CvSTASH.
         *
         * Due to an unfortunate accident of history, the SvPVX field
         * serves two purposes.  It is also used for the subroutine's pro-
         * type.  Since SvPVX has been documented as returning the sub name
         * for a long time, but not as returning the prototype, we have
         * to preserve the SvPVX AUTOLOAD behaviour and put the prototype
         * elsewhere.
         *
         * We put the prototype in the same allocated buffer, but after
         * the sub name.  The SvPOK flag indicates the presence of a proto-
         * type.  The CvAUTOLOAD flag indicates the presence of a sub name.
         * If both flags are on, then SvLEN is used to indicate the end of
         * the prototype (artificially lower than what is actually allo-
         * cated), at the risk of having to reallocate a few bytes unneces-
         * sarily--but that should happen very rarely, if ever.
         *
         * We use SvUTF8 for both prototypes and sub names, so if one is
         * UTF8, the other must be upgraded.
         */
	CvSTASH_set(cv, stash);
	if (SvPOK(cv)) { /* Ouch! */
	    SV *tmpsv = newSVpvn_flags(name, len, is_utf8);
	    STRLEN ulen;
	    const char *proto = CvPROTO(cv);
	    assert(proto);
	    if (SvUTF8(cv))
		sv_utf8_upgrade_flags_grow(tmpsv, 0, CvPROTOLEN(cv) + 2);
	    ulen = SvCUR(tmpsv);
	    SvCUR(tmpsv)++; /* include null in string */
	    sv_catpvn_flags(
		tmpsv, proto, CvPROTOLEN(cv), SV_CATBYTES*!SvUTF8(cv)
	    );
	    SvTEMP_on(tmpsv); /* Allow theft */
	    sv_setsv_nomg((SV *)cv, tmpsv);
	    SvTEMP_off(tmpsv);
	    SvREFCNT_dec(tmpsv);
	    SvLEN(cv) = SvCUR(cv) + 1;
	    SvCUR(cv) = ulen;
	}
	else {
	  sv_setpvn((SV *)cv, name, len);
	  SvPOK_off(cv);
	  if (is_utf8)
            SvUTF8_on(cv);
	  else SvUTF8_off(cv);
	}
	CvAUTOLOAD_on(cv);
    }

    /*
     * Given &FOO::AUTOLOAD, set $FOO::AUTOLOAD to desired function name.
     * The subroutine's original name may not be "AUTOLOAD", so we don't
     * use that, but for lack of anything better we will use the sub's
     * original package to look up $AUTOLOAD.
     */
    varstash = GvSTASH(CvGV(cv));
    vargv = *(GV**)hv_fetch(varstash, S_autoload, S_autolen, TRUE);
    ENTER;

    if (!isGV(vargv)) {
	gv_init_pvn(vargv, varstash, S_autoload, S_autolen, 0);
#ifdef PERL_DONT_CREATE_GVSV
	GvSV(vargv) = newSV(0);
#endif
    }
    LEAVE;
    varsv = GvSVn(vargv);
    sv_setsv(varsv, packname);
    sv_catpvs(varsv, "::");
    /* Ensure SvSETMAGIC() is called if necessary. In particular, to clear
       tainting if $FOO::AUTOLOAD was previously tainted, but is not now.  */
    sv_catpvn_flags(
	varsv, name, len,
	SV_SMAGIC|(is_utf8 ? SV_CATUTF8 : SV_CATBYTES)
    );
    if (is_utf8)
        SvUTF8_on(varsv);
    return gv;
}


/* require_tie_mod() internal routine for requiring a module
 * that implements the logic of automatic ties like %! and %-
 *
 * The "gv" parameter should be the glob.
 * "varpv" holds the name of the var, used for error messages.
 * "namesv" holds the module name. Its refcount will be decremented.
 * "methpv" holds the method name to test for to check that things
 *   are working reasonably close to as expected.
 * "flags": if flag & 1 then save the scalar before loading.
 * For the protection of $! to work (it is set by this routine)
 * the sv slot must already be magicalized.
 */
STATIC HV*
S_require_tie_mod(pTHX_ GV *gv, const char *varpv, SV* namesv, const char *methpv,const U32 flags)
{
    dVAR;
    HV* stash = gv_stashsv(namesv, 0);

    PERL_ARGS_ASSERT_REQUIRE_TIE_MOD;

    if (!stash || !(gv_fetchmethod_autoload(stash, methpv, FALSE))) {
	SV *module = newSVsv(namesv);
	char varname = *varpv; /* varpv might be clobbered by load_module,
				  so save it. For the moment it's always
				  a single char. */
	const char type = varname == '[' ? '$' : '%';
	dSP;
	ENTER;
	if ( flags & 1 )
	    save_scalar(gv);
	PUSHSTACKi(PERLSI_MAGIC);
	Perl_load_module(aTHX_ PERL_LOADMOD_NOIMPORT, module, NULL);
	POPSTACK;
	LEAVE;
	SPAGAIN;
	stash = gv_stashsv(namesv, 0);
	if (!stash)
	    Perl_croak(aTHX_ "panic: Can't use %c%c because %"SVf" is not available",
		    type, varname, SVfARG(namesv));
	else if (!gv_fetchmethod(stash, methpv))
	    Perl_croak(aTHX_ "panic: Can't use %c%c because %"SVf" does not support method %s",
		    type, varname, SVfARG(namesv), methpv);
    }
    SvREFCNT_dec(namesv);
    return stash;
}

/*
=for apidoc gv_stashpv

Returns a pointer to the stash for a specified package.  Uses C<strlen> to
determine the length of C<name>, then calls C<gv_stashpvn()>.

=cut
*/

HV*
Perl_gv_stashpv(pTHX_ const char *name, I32 create)
{
    PERL_ARGS_ASSERT_GV_STASHPV;
    return gv_stashpvn(name, strlen(name), create);
}

/*
=for apidoc gv_stashpvn

Returns a pointer to the stash for a specified package.  The C<namelen>
parameter indicates the length of the C<name>, in bytes.  C<flags> is passed
to C<gv_fetchpvn_flags()>, so if set to C<GV_ADD> then the package will be
created if it does not already exist.  If the package does not exist and
C<flags> is 0 (or any other setting that does not create packages) then NULL
is returned.


=cut
*/

HV*
Perl_gv_stashpvn(pTHX_ const char *name, U32 namelen, I32 flags)
{
    char smallbuf[128];
    char *tmpbuf;
    HV *stash;
    GV *tmpgv;
    U32 tmplen = namelen + 2;

    PERL_ARGS_ASSERT_GV_STASHPVN;

    if (tmplen <= sizeof smallbuf)
	tmpbuf = smallbuf;
    else
	Newx(tmpbuf, tmplen, char);
    Copy(name, tmpbuf, namelen, char);
    tmpbuf[namelen]   = ':';
    tmpbuf[namelen+1] = ':';
    tmpgv = gv_fetchpvn_flags(tmpbuf, tmplen, flags, SVt_PVHV);
    if (tmpbuf != smallbuf)
	Safefree(tmpbuf);
    if (!tmpgv)
	return NULL;
    stash = GvHV(tmpgv);
    if (!(flags & ~GV_NOADD_MASK) && !stash) return NULL;
    assert(stash);
    if (!HvNAME_get(stash)) {
	hv_name_set(stash, name, namelen, flags & SVf_UTF8 ? SVf_UTF8 : 0 );
	
	/* FIXME: This is a repeat of logic in gv_fetchpvn_flags */
	/* If the containing stash has multiple effective
	   names, see that this one gets them, too. */
	if (HvAUX(GvSTASH(tmpgv))->xhv_name_count)
	    mro_package_moved(stash, NULL, tmpgv, 1);
    }
    return stash;
}

/*
=for apidoc gv_stashsv

Returns a pointer to the stash for a specified package.  See C<gv_stashpvn>.

=cut
*/

HV*
Perl_gv_stashsv(pTHX_ SV *sv, I32 flags)
{
    STRLEN len;
    const char * const ptr = SvPV_const(sv,len);

    PERL_ARGS_ASSERT_GV_STASHSV;

    return gv_stashpvn(ptr, len, flags | SvUTF8(sv));
}


GV *
Perl_gv_fetchpv(pTHX_ const char *nambeg, I32 add, const svtype sv_type) {
    PERL_ARGS_ASSERT_GV_FETCHPV;
    return gv_fetchpvn_flags(nambeg, strlen(nambeg), add, sv_type);
}

GV *
Perl_gv_fetchsv(pTHX_ SV *name, I32 flags, const svtype sv_type) {
    STRLEN len;
    const char * const nambeg =
       SvPV_flags_const(name, len, flags & GV_NO_SVGMAGIC ? 0 : SV_GMAGIC);
    PERL_ARGS_ASSERT_GV_FETCHSV;
    return gv_fetchpvn_flags(nambeg, len, flags | SvUTF8(name), sv_type);
}

STATIC void
S_gv_magicalize_isa(pTHX_ GV *gv)
{
    AV* av;

    PERL_ARGS_ASSERT_GV_MAGICALIZE_ISA;

    av = GvAVn(gv);
    GvMULTI_on(gv);
    sv_magic(MUTABLE_SV(av), MUTABLE_SV(gv), PERL_MAGIC_isa,
	     NULL, 0);
}

STATIC void
S_gv_magicalize_overload(pTHX_ GV *gv)
{
    HV* hv;

    PERL_ARGS_ASSERT_GV_MAGICALIZE_OVERLOAD;

    hv = GvHVn(gv);
    GvMULTI_on(gv);
    hv_magic(hv, NULL, PERL_MAGIC_overload);
}

GV *
Perl_gv_fetchpvn_flags(pTHX_ const char *nambeg, STRLEN full_len, I32 flags,
		       const svtype sv_type)
{
    dVAR;
    register const char *name = nambeg;
    register GV *gv = NULL;
    GV**gvp;
    I32 len;
    register const char *name_cursor;
    HV *stash = NULL;
    const I32 no_init = flags & (GV_NOADD_NOINIT | GV_NOINIT);
    const I32 no_expand = flags & GV_NOEXPAND;
    const I32 add = flags & ~GV_NOADD_MASK;
    const U32 is_utf8 = flags & SVf_UTF8;
    bool addmg = !!(flags & GV_ADDMG);
    const char *const name_end = nambeg + full_len;
    const char *const name_em1 = name_end - 1;
    U32 faking_it;

    PERL_ARGS_ASSERT_GV_FETCHPVN_FLAGS;

    if (flags & GV_NOTQUAL) {
	/* Caller promised that there is no stash, so we can skip the check. */
	len = full_len;
	goto no_stash;
    }

    if (full_len > 2 && *name == '*' && isIDFIRST_lazy_if(name + 1, is_utf8)) {
	/* accidental stringify on a GV? */
	name++;
    }

    for (name_cursor = name; name_cursor < name_end; name_cursor++) {
	if (name_cursor < name_em1 &&
	    ((*name_cursor == ':'
	     && name_cursor[1] == ':')
	    || *name_cursor == '\''))
	{
	    if (!stash)
		stash = PL_defstash;
	    if (!stash || !SvREFCNT(stash)) /* symbol table under destruction */
		return NULL;

	    len = name_cursor - name;
	    if (name_cursor > nambeg) { /* Skip for initial :: or ' */
		const char *key;
		if (*name_cursor == ':') {
		    key = name;
		    len += 2;
		} else {
		    char *tmpbuf;
		    Newx(tmpbuf, len+2, char);
		    Copy(name, tmpbuf, len, char);
		    tmpbuf[len++] = ':';
		    tmpbuf[len++] = ':';
		    key = tmpbuf;
		}
		gvp = (GV**)hv_fetch(stash, key, is_utf8 ? -len : len, add);
		gv = gvp ? *gvp : NULL;
		if (gv && gv != (const GV *)&PL_sv_undef) {
		    if (SvTYPE(gv) != SVt_PVGV)
			gv_init_pvn(gv, stash, key, len, (add & GV_ADDMULTI)|is_utf8);
		    else
			GvMULTI_on(gv);
		}
		if (key != name)
		    Safefree(key);
		if (!gv || gv == (const GV *)&PL_sv_undef)
		    return NULL;

		if (!(stash = GvHV(gv)))
		{
		    stash = GvHV(gv) = newHV();
		    if (!HvNAME_get(stash)) {
			if (GvSTASH(gv) == PL_defstash && len == 6
			 && strnEQ(name, "CORE", 4))
			    hv_name_set(stash, "CORE", 4, 0);
			else
			    hv_name_set(
				stash, nambeg, name_cursor-nambeg, is_utf8
			    );
			/* If the containing stash has multiple effective
			   names, see that this one gets them, too. */
			if (HvAUX(GvSTASH(gv))->xhv_name_count)
			    mro_package_moved(stash, NULL, gv, 1);
		    }
		}
		else if (!HvNAME_get(stash))
		    hv_name_set(stash, nambeg, name_cursor - nambeg, is_utf8);
	    }

	    if (*name_cursor == ':')
		name_cursor++;
	    name = name_cursor+1;
	    if (name == name_end)
		return gv
		    ? gv : MUTABLE_GV(*hv_fetchs(PL_defstash, "main::", TRUE));
	}
    }
    len = name_cursor - name;

    /* No stash in name, so see how we can default */

    if (!stash) {
    no_stash:
	if (len && isIDFIRST_lazy(name)) {
	    bool global = FALSE;

	    switch (len) {
	    case 1:
		if (*name == '_')
		    global = TRUE;
		break;
	    case 3:
		if ((name[0] == 'I' && name[1] == 'N' && name[2] == 'C')
		    || (name[0] == 'E' && name[1] == 'N' && name[2] == 'V')
		    || (name[0] == 'S' && name[1] == 'I' && name[2] == 'G'))
		    global = TRUE;
		break;
	    case 4:
		if (name[0] == 'A' && name[1] == 'R' && name[2] == 'G'
		    && name[3] == 'V')
		    global = TRUE;
		break;
	    case 5:
		if (name[0] == 'S' && name[1] == 'T' && name[2] == 'D'
		    && name[3] == 'I' && name[4] == 'N')
		    global = TRUE;
		break;
	    case 6:
		if ((name[0] == 'S' && name[1] == 'T' && name[2] == 'D')
		    &&((name[3] == 'O' && name[4] == 'U' && name[5] == 'T')
		       ||(name[3] == 'E' && name[4] == 'R' && name[5] == 'R')))
		    global = TRUE;
		break;
	    case 7:
		if (name[0] == 'A' && name[1] == 'R' && name[2] == 'G'
		    && name[3] == 'V' && name[4] == 'O' && name[5] == 'U'
		    && name[6] == 'T')
		    global = TRUE;
		break;
	    }

	    if (global)
		stash = PL_defstash;
	    else if (IN_PERL_COMPILETIME) {
		stash = PL_curstash;
		if (add && (PL_hints & HINT_STRICT_VARS) &&
		    sv_type != SVt_PVCV &&
		    sv_type != SVt_PVGV &&
		    sv_type != SVt_PVFM &&
		    sv_type != SVt_PVIO &&
		    !(len == 1 && sv_type == SVt_PV &&
		      (*name == 'a' || *name == 'b')) )
		{
		    gvp = (GV**)hv_fetch(stash,name,is_utf8 ? -len : len,0);
		    if (!gvp ||
			*gvp == (const GV *)&PL_sv_undef ||
			SvTYPE(*gvp) != SVt_PVGV)
		    {
			stash = NULL;
		    }
		    else if ((sv_type == SVt_PV   && !GvIMPORTED_SV(*gvp)) ||
			     (sv_type == SVt_PVAV && !GvIMPORTED_AV(*gvp)) ||
			     (sv_type == SVt_PVHV && !GvIMPORTED_HV(*gvp)) )
		    {
                        SV* namesv = newSVpvn_flags(name, len, SVs_TEMP | is_utf8);
			/* diag_listed_as: Variable "%s" is not imported%s */
			Perl_ck_warner_d(
			    aTHX_ packWARN(WARN_MISC),
			    "Variable \"%c%"SVf"\" is not imported",
			    sv_type == SVt_PVAV ? '@' :
			    sv_type == SVt_PVHV ? '%' : '$',
			    SVfARG(namesv));
			if (GvCVu(*gvp))
			    Perl_ck_warner_d(
				aTHX_ packWARN(WARN_MISC),
				"\t(Did you mean &%"SVf" instead?)\n", SVfARG(namesv)
			    );
			stash = NULL;
		    }
		}
	    }
	    else
		stash = CopSTASH(PL_curcop);
	}
	else
	    stash = PL_defstash;
    }

    /* By this point we should have a stash and a name */

    if (!stash) {
	if (add) {
	    SV * const err = Perl_mess(aTHX_
		 "Global symbol \"%s%"SVf"\" requires explicit package name",
		 (sv_type == SVt_PV ? "$"
		  : sv_type == SVt_PVAV ? "@"
		  : sv_type == SVt_PVHV ? "%"
		  : ""), SVfARG(newSVpvn_flags(name, len, SVs_TEMP | is_utf8)));
	    GV *gv;
	    if (USE_UTF8_IN_NAMES)
		SvUTF8_on(err);
	    qerror(err);
	    gv = gv_fetchpvs("<none>::", GV_ADDMULTI, SVt_PVHV);
	    if(!gv) {
		/* symbol table under destruction */
		return NULL;
	    }	
	    stash = GvHV(gv);
	}
	else
	    return NULL;
    }

    if (!SvREFCNT(stash))	/* symbol table under destruction */
	return NULL;

    gvp = (GV**)hv_fetch(stash,name,is_utf8 ? -len : len,add);
    if (!gvp || *gvp == (const GV *)&PL_sv_undef) {
	if (addmg) gv = (GV *)newSV(0);
	else return NULL;
    }
    else gv = *gvp, addmg = 0;
    /* From this point on, addmg means gv has not been inserted in the
       symtab yet. */

    if (SvTYPE(gv) == SVt_PVGV) {
	if (add) {
	    GvMULTI_on(gv);
	    gv_init_svtype(gv, sv_type);
	    if (len == 1 && stash == PL_defstash) {
	      if (sv_type == SVt_PVHV || sv_type == SVt_PVGV) {
	        if (*name == '!')
		    require_tie_mod(gv, "!", newSVpvs("Errno"), "TIEHASH", 1);
		else if (*name == '-' || *name == '+')
		    require_tie_mod(gv, name, newSVpvs("Tie::Hash::NamedCapture"), "TIEHASH", 0);
	      }
	      if (sv_type==SVt_PV || sv_type==SVt_PVGV) {
	       if (*name == '[')
		require_tie_mod(gv,name,newSVpvs("arybase"),"FETCH",0);
	       else if (*name == '&' || *name == '`' || *name == '\'') {
		PL_sawampersand = TRUE;
		(void)GvSVn(gv);
	       }
	      }
	    }
	    else if (len == 3 && sv_type == SVt_PVAV
	          && strnEQ(name, "ISA", 3)
	          && (!GvAV(gv) || !SvSMAGICAL(GvAV(gv))))
		gv_magicalize_isa(gv);
	}
	return gv;
    } else if (no_init) {
	assert(!addmg);
	return gv;
    } else if (no_expand && SvROK(gv)) {
	assert(!addmg);
	return gv;
    }

    /* Adding a new symbol.
       Unless of course there was already something non-GV here, in which case
       we want to behave as if there was always a GV here, containing some sort
       of subroutine.
       Otherwise we run the risk of creating things like GvIO, which can cause
       subtle bugs. eg the one that tripped up SQL::Translator  */

    faking_it = SvOK(gv);

    if (add & GV_ADDWARN)
	Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL), "Had to create %"SVf" unexpectedly",
                SVfARG(newSVpvn_flags(nambeg, name_end-nambeg, SVs_TEMP | is_utf8 )));
    gv_init_pvn(gv, stash, name, len, (add & GV_ADDMULTI)|is_utf8);

    if ( isIDFIRST_lazy_if(name, is_utf8)
                && ! (isLEXWARN_on ? ckWARN(WARN_ONCE) : (PL_dowarn & G_WARN_ON ) ) )
        GvMULTI_on(gv) ;

    /* set up magic where warranted */
    if (stash != PL_defstash) { /* not the main stash */
	/* We only have to check for four names here: EXPORT, ISA, OVERLOAD
	   and VERSION. All the others apply only to the main stash or to
	   CORE (which is checked right after this). */
	if (len > 2) {
	    const char * const name2 = name + 1;
	    switch (*name) {
	    case 'E':
		if (strnEQ(name2, "XPORT", 5))
		    GvMULTI_on(gv);
		break;
	    case 'I':
		if (strEQ(name2, "SA"))
		    gv_magicalize_isa(gv);
		break;
	    case 'O':
		if (strEQ(name2, "VERLOAD"))
		    gv_magicalize_overload(gv);
		break;
	    case 'V':
		if (strEQ(name2, "ERSION"))
		    GvMULTI_on(gv);
		break;
	    default:
		goto try_core;
	    }
	    goto add_magical_gv;
	}
      try_core:
	if (len > 1 /* shortest is uc */ && HvNAMELEN_get(stash) == 4) {
	  /* Avoid null warning: */
	  const char * const stashname = HvNAME(stash); assert(stashname);
	  if (strnEQ(stashname, "CORE", 4))
	    S_maybe_add_coresub(aTHX_ 0, gv, name, len);
	}
    }
    else if (len > 1) {
#ifndef EBCDIC
	if (*name > 'V' ) {
	    NOOP;
	    /* Nothing else to do.
	       The compiler will probably turn the switch statement into a
	       branch table. Make sure we avoid even that small overhead for
	       the common case of lower case variable names.  */
	} else
#endif
	{
	    const char * const name2 = name + 1;
	    switch (*name) {
	    case 'A':
		if (strEQ(name2, "RGV")) {
		    IoFLAGS(GvIOn(gv)) |= IOf_ARGV|IOf_START;
		}
		else if (strEQ(name2, "RGVOUT")) {
		    GvMULTI_on(gv);
		}
		break;
	    case 'E':
		if (strnEQ(name2, "XPORT", 5))
		    GvMULTI_on(gv);
		break;
	    case 'I':
		if (strEQ(name2, "SA")) {
		    gv_magicalize_isa(gv);
		}
		break;
	    case 'O':
		if (strEQ(name2, "VERLOAD")) {
		    gv_magicalize_overload(gv);
		}
		break;
	    case 'S':
		if (strEQ(name2, "IG")) {
		    HV *hv;
		    I32 i;
		    if (!PL_psig_name) {
			Newxz(PL_psig_name, 2 * SIG_SIZE, SV*);
			Newxz(PL_psig_pend, SIG_SIZE, int);
			PL_psig_ptr = PL_psig_name + SIG_SIZE;
		    } else {
			/* I think that the only way to get here is to re-use an
			   embedded perl interpreter, where the previous
			   use didn't clean up fully because
			   PL_perl_destruct_level was 0. I'm not sure that we
			   "support" that, in that I suspect in that scenario
			   there are sufficient other garbage values left in the
			   interpreter structure that something else will crash
			   before we get here. I suspect that this is one of
			   those "doctor, it hurts when I do this" bugs.  */
			Zero(PL_psig_name, 2 * SIG_SIZE, SV*);
			Zero(PL_psig_pend, SIG_SIZE, int);
		    }
		    GvMULTI_on(gv);
		    hv = GvHVn(gv);
		    hv_magic(hv, NULL, PERL_MAGIC_sig);
		    for (i = 1; i < SIG_SIZE; i++) {
			SV * const * const init = hv_fetch(hv, PL_sig_name[i], strlen(PL_sig_name[i]), 1);
			if (init)
			    sv_setsv(*init, &PL_sv_undef);
		    }
		}
		break;
	    case 'V':
		if (strEQ(name2, "ERSION"))
		    GvMULTI_on(gv);
		break;
            case '\003':        /* $^CHILD_ERROR_NATIVE */
		if (strEQ(name2, "HILD_ERROR_NATIVE"))
		    goto magicalize;
		break;
	    case '\005':	/* $^ENCODING */
		if (strEQ(name2, "NCODING"))
		    goto magicalize;
		break;
	    case '\007':	/* $^GLOBAL_PHASE */
		if (strEQ(name2, "LOBAL_PHASE"))
		    goto ro_magicalize;
		break;
            case '\015':        /* $^MATCH */
                if (strEQ(name2, "ATCH"))
		    goto magicalize;
	    case '\017':	/* $^OPEN */
		if (strEQ(name2, "PEN"))
		    goto magicalize;
		break;
	    case '\020':        /* $^PREMATCH  $^POSTMATCH */
	        if (strEQ(name2, "REMATCH") || strEQ(name2, "OSTMATCH"))
		    goto magicalize;
		break;
	    case '\024':	/* ${^TAINT} */
		if (strEQ(name2, "AINT"))
		    goto ro_magicalize;
		break;
	    case '\025':	/* ${^UNICODE}, ${^UTF8LOCALE} */
		if (strEQ(name2, "NICODE"))
		    goto ro_magicalize;
		if (strEQ(name2, "TF8LOCALE"))
		    goto ro_magicalize;
		if (strEQ(name2, "TF8CACHE"))
		    goto magicalize;
		break;
	    case '\027':	/* $^WARNING_BITS */
		if (strEQ(name2, "ARNING_BITS"))
		    goto magicalize;
		break;
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
	    {
		/* Ensures that we have an all-digit variable, ${"1foo"} fails
		   this test  */
		/* This snippet is taken from is_gv_magical */
		const char *end = name + len;
		while (--end > name) {
		    if (!isDIGIT(*end))	goto add_magical_gv;
		}
		goto magicalize;
	    }
	    }
	}
    } else {
	/* Names of length 1.  (Or 0. But name is NUL terminated, so that will
	   be case '\0' in this switch statement (ie a default case)  */
	switch (*name) {
	case '&':		/* $& */
	case '`':		/* $` */
	case '\'':		/* $' */
	    if (!(
		sv_type == SVt_PVAV ||
		sv_type == SVt_PVHV ||
		sv_type == SVt_PVCV ||
		sv_type == SVt_PVFM ||
		sv_type == SVt_PVIO
		)) { PL_sawampersand = TRUE; }
	    goto magicalize;

	case ':':		/* $: */
	    sv_setpv(GvSVn(gv),PL_chopset);
	    goto magicalize;

	case '?':		/* $? */
#ifdef COMPLEX_STATUS
	    SvUPGRADE(GvSVn(gv), SVt_PVLV);
#endif
	    goto magicalize;

	case '!':		/* $! */
	    GvMULTI_on(gv);
	    /* If %! has been used, automatically load Errno.pm. */

	    sv_magic(GvSVn(gv), MUTABLE_SV(gv), PERL_MAGIC_sv, name, len);

            /* magicalization must be done before require_tie_mod is called */
	    if (sv_type == SVt_PVHV || sv_type == SVt_PVGV)
	    {
		if (addmg) (void)hv_store(stash,name,len,(SV *)gv,0);
		addmg = 0;
		require_tie_mod(gv, "!", newSVpvs("Errno"), "TIEHASH", 1);
	    }

	    break;
	case '-':		/* $- */
	case '+':		/* $+ */
	GvMULTI_on(gv); /* no used once warnings here */
        {
            AV* const av = GvAVn(gv);
	    SV* const avc = (*name == '+') ? MUTABLE_SV(av) : NULL;

	    sv_magic(MUTABLE_SV(av), avc, PERL_MAGIC_regdata, NULL, 0);
            sv_magic(GvSVn(gv), MUTABLE_SV(gv), PERL_MAGIC_sv, name, len);
            if (avc)
                SvREADONLY_on(GvSVn(gv));
            SvREADONLY_on(av);

            if (sv_type == SVt_PVHV || sv_type == SVt_PVGV)
	    {
		if (addmg) (void)hv_store(stash,name,len,(SV *)gv,0);
		addmg = 0;
                require_tie_mod(gv, name, newSVpvs("Tie::Hash::NamedCapture"), "TIEHASH", 0);
	    }

            break;
	}
	case '*':		/* $* */
	case '#':		/* $# */
	    if (sv_type == SVt_PV)
		/* diag_listed_as: $* is no longer supported */
		Perl_ck_warner_d(aTHX_ packWARN2(WARN_DEPRECATED, WARN_SYNTAX),
				 "$%c is no longer supported", *name);
	    break;
	case '|':		/* $| */
	    sv_setiv(GvSVn(gv), (IV)(IoFLAGS(GvIOp(PL_defoutgv)) & IOf_FLUSH) != 0);
	    goto magicalize;

	case '\010':	/* $^H */
	    {
		HV *const hv = GvHVn(gv);
		hv_magic(hv, NULL, PERL_MAGIC_hints);
	    }
	    goto magicalize;
	case '[':		/* $[ */
	    if ((sv_type == SVt_PV || sv_type == SVt_PVGV)
	     && FEATURE_ARYBASE_IS_ENABLED) {
		if (addmg) (void)hv_store(stash,name,len,(SV *)gv,0);
		require_tie_mod(gv,name,newSVpvs("arybase"),"FETCH",0);
		addmg = 0;
	    }
	    else goto magicalize;
            break;
	case '\023':	/* $^S */
	ro_magicalize:
	    SvREADONLY_on(GvSVn(gv));
	    /* FALL THROUGH */
	case '0':		/* $0 */
	case '1':		/* $1 */
	case '2':		/* $2 */
	case '3':		/* $3 */
	case '4':		/* $4 */
	case '5':		/* $5 */
	case '6':		/* $6 */
	case '7':		/* $7 */
	case '8':		/* $8 */
	case '9':		/* $9 */
	case '^':		/* $^ */
	case '~':		/* $~ */
	case '=':		/* $= */
	case '%':		/* $% */
	case '.':		/* $. */
	case '(':		/* $( */
	case ')':		/* $) */
	case '<':		/* $< */
	case '>':		/* $> */
	case '\\':		/* $\ */
	case '/':		/* $/ */
	case '$':		/* $$ */
	case '\001':	/* $^A */
	case '\003':	/* $^C */
	case '\004':	/* $^D */
	case '\005':	/* $^E */
	case '\006':	/* $^F */
	case '\011':	/* $^I, NOT \t in EBCDIC */
	case '\016':	/* $^N */
	case '\017':	/* $^O */
	case '\020':	/* $^P */
	case '\024':	/* $^T */
	case '\027':	/* $^W */
	magicalize:
	    sv_magic(GvSVn(gv), MUTABLE_SV(gv), PERL_MAGIC_sv, name, len);
	    break;

	case '\014':	/* $^L */
	    sv_setpvs(GvSVn(gv),"\f");
	    PL_formfeed = GvSVn(gv);
	    break;
	case ';':		/* $; */
	    sv_setpvs(GvSVn(gv),"\034");
	    break;
	case ']':		/* $] */
	{
	    SV * const sv = GvSV(gv);
	    if (!sv_derived_from(PL_patchlevel, "version"))
		upg_version(PL_patchlevel, TRUE);
	    GvSV(gv) = vnumify(PL_patchlevel);
	    SvREADONLY_on(GvSV(gv));
	    SvREFCNT_dec(sv);
	}
	break;
	case '\026':	/* $^V */
	{
	    SV * const sv = GvSV(gv);
	    GvSV(gv) = new_version(PL_patchlevel);
	    SvREADONLY_on(GvSV(gv));
	    SvREFCNT_dec(sv);
	}
	break;
	}
    }
  add_magical_gv:
    if (addmg) {
	if (GvAV(gv) || GvHV(gv) || GvIO(gv) || GvCV(gv) || (
	     GvSV(gv) && (SvOK(GvSV(gv)) || SvMAGICAL(GvSV(gv)))
	   ))
	    (void)hv_store(stash,name,len,(SV *)gv,0);
	else SvREFCNT_dec(gv), gv = NULL;
    }
    if (gv) gv_init_svtype(gv, faking_it ? SVt_PVCV : sv_type);
    return gv;
}

void
Perl_gv_fullname4(pTHX_ SV *sv, const GV *gv, const char *prefix, bool keepmain)
{
    const char *name;
    const HV * const hv = GvSTASH(gv);

    PERL_ARGS_ASSERT_GV_FULLNAME4;

    sv_setpv(sv, prefix ? prefix : "");

    if (hv && (name = HvNAME(hv))) {
      const STRLEN len = HvNAMELEN(hv);
      if (keepmain || strnNE(name, "main", len)) {
	sv_catpvn_flags(sv,name,len,HvNAMEUTF8(hv)?SV_CATUTF8:SV_CATBYTES);
	sv_catpvs(sv,"::");
      }
    }
    else sv_catpvs(sv,"__ANON__::");
    sv_catsv(sv,sv_2mortal(newSVhek(GvNAME_HEK(gv))));
}

void
Perl_gv_efullname4(pTHX_ SV *sv, const GV *gv, const char *prefix, bool keepmain)
{
    const GV * const egv = GvEGVx(gv);

    PERL_ARGS_ASSERT_GV_EFULLNAME4;

    gv_fullname4(sv, egv ? egv : gv, prefix, keepmain);
}

void
Perl_gv_check(pTHX_ const HV *stash)
{
    dVAR;
    register I32 i;

    PERL_ARGS_ASSERT_GV_CHECK;

    if (!HvARRAY(stash))
	return;
    for (i = 0; i <= (I32) HvMAX(stash); i++) {
        const HE *entry;
	for (entry = HvARRAY(stash)[i]; entry; entry = HeNEXT(entry)) {
            register GV *gv;
            HV *hv;
	    if (HeKEY(entry)[HeKLEN(entry)-1] == ':' &&
		(gv = MUTABLE_GV(HeVAL(entry))) && isGV(gv) && (hv = GvHV(gv)))
	    {
		if (hv != PL_defstash && hv != stash)
		     gv_check(hv);              /* nested package */
	    }
            else if ( *HeKEY(entry) != '_'
                        && isIDFIRST_lazy_if(HeKEY(entry), HeUTF8(entry)) ) {
                const char *file;
		gv = MUTABLE_GV(HeVAL(entry));
		if (SvTYPE(gv) != SVt_PVGV || GvMULTI(gv))
		    continue;
		file = GvFILE(gv);
		CopLINE_set(PL_curcop, GvLINE(gv));
#ifdef USE_ITHREADS
		CopFILE(PL_curcop) = (char *)file;	/* set for warning */
#else
		CopFILEGV(PL_curcop)
		    = gv_fetchfile_flags(file, HEK_LEN(GvFILE_HEK(gv)), 0);
#endif
		Perl_warner(aTHX_ packWARN(WARN_ONCE),
			"Name \"%"HEKf"::%"HEKf
			"\" used only once: possible typo",
                            HEKfARG(HvNAME_HEK(stash)),
                            HEKfARG(GvNAME_HEK(gv)));
	    }
	}
    }
}

GV *
Perl_newGVgen_flags(pTHX_ const char *pack, U32 flags)
{
    dVAR;
    PERL_ARGS_ASSERT_NEWGVGEN_FLAGS;

    return gv_fetchpv(Perl_form(aTHX_ "%"SVf"::_GEN_%ld",
                                    SVfARG(newSVpvn_flags(pack, strlen(pack),
                                            SVs_TEMP | flags)),
                                (long)PL_gensym++),
                      GV_ADD, SVt_PVGV);
}

/* hopefully this is only called on local symbol table entries */

GP*
Perl_gp_ref(pTHX_ GP *gp)
{
    dVAR;
    if (!gp)
	return NULL;
    gp->gp_refcnt++;
    if (gp->gp_cv) {
	if (gp->gp_cvgen) {
	    /* If the GP they asked for a reference to contains
               a method cache entry, clear it first, so that we
               don't infect them with our cached entry */
	    SvREFCNT_dec(gp->gp_cv);
	    gp->gp_cv = NULL;
	    gp->gp_cvgen = 0;
	}
    }
    return gp;
}

void
Perl_gp_free(pTHX_ GV *gv)
{
    dVAR;
    GP* gp;
    int attempts = 100;

    if (!gv || !isGV_with_GP(gv) || !(gp = GvGP(gv)))
	return;
    if (gp->gp_refcnt == 0) {
	Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
			 "Attempt to free unreferenced glob pointers"
			 pTHX__FORMAT pTHX__VALUE);
        return;
    }
    if (--gp->gp_refcnt > 0) {
	if (gp->gp_egv == gv)
	    gp->gp_egv = 0;
	GvGP_set(gv, NULL);
        return;
    }

    while (1) {
      /* Copy and null out all the glob slots, so destructors do not see
         freed SVs. */
      HEK * const file_hek = gp->gp_file_hek;
      SV  * const sv       = gp->gp_sv;
      AV  * const av       = gp->gp_av;
      HV  * const hv       = gp->gp_hv;
      IO  * const io       = gp->gp_io;
      CV  * const cv       = gp->gp_cv;
      CV  * const form     = gp->gp_form;

      gp->gp_file_hek = NULL;
      gp->gp_sv       = NULL;
      gp->gp_av       = NULL;
      gp->gp_hv       = NULL;
      gp->gp_io       = NULL;
      gp->gp_cv       = NULL;
      gp->gp_form     = NULL;

      if (file_hek)
	unshare_hek(file_hek);

      SvREFCNT_dec(sv);
      SvREFCNT_dec(av);
      /* FIXME - another reference loop GV -> symtab -> GV ?
         Somehow gp->gp_hv can end up pointing at freed garbage.  */
      if (hv && SvTYPE(hv) == SVt_PVHV) {
        const HEK *hvname_hek = HvNAME_HEK(hv);
        if (PL_stashcache && hvname_hek)
           (void)hv_delete(PL_stashcache, HEK_KEY(hvname_hek),
                      (HEK_UTF8(hvname_hek) ? -HEK_LEN(hvname_hek) : HEK_LEN(hvname_hek)),
                      G_DISCARD);
	SvREFCNT_dec(hv);
      }
      SvREFCNT_dec(io);
      SvREFCNT_dec(cv);
      SvREFCNT_dec(form);

      if (!gp->gp_file_hek
       && !gp->gp_sv
       && !gp->gp_av
       && !gp->gp_hv
       && !gp->gp_io
       && !gp->gp_cv
       && !gp->gp_form) break;

      if (--attempts == 0) {
	Perl_die(aTHX_
	  "panic: gp_free failed to free glob pointer - "
	  "something is repeatedly re-creating entries"
	);
      }
    }

    Safefree(gp);
    GvGP_set(gv, NULL);
}

int
Perl_magic_freeovrld(pTHX_ SV *sv, MAGIC *mg)
{
    AMT * const amtp = (AMT*)mg->mg_ptr;
    PERL_UNUSED_ARG(sv);

    PERL_ARGS_ASSERT_MAGIC_FREEOVRLD;

    if (amtp && AMT_AMAGIC(amtp)) {
	int i;
	for (i = 1; i < NofAMmeth; i++) {
	    CV * const cv = amtp->table[i];
	    if (cv) {
		SvREFCNT_dec(MUTABLE_SV(cv));
		amtp->table[i] = NULL;
	    }
	}
    }
 return 0;
}

/* Updates and caches the CV's */
/* Returns:
 * 1 on success and there is some overload
 * 0 if there is no overload
 * -1 if some error occurred and it couldn't croak
 */

int
Perl_Gv_AMupdate(pTHX_ HV *stash, bool destructing)
{
  dVAR;
  MAGIC* const mg = mg_find((const SV *)stash, PERL_MAGIC_overload_table);
  AMT amt;
  const struct mro_meta* stash_meta = HvMROMETA(stash);
  U32 newgen;

  PERL_ARGS_ASSERT_GV_AMUPDATE;

  newgen = PL_sub_generation + stash_meta->pkg_gen + stash_meta->cache_gen;
  if (mg) {
      const AMT * const amtp = (AMT*)mg->mg_ptr;
      if (amtp->was_ok_am == PL_amagic_generation
	  && amtp->was_ok_sub == newgen) {
	  return AMT_OVERLOADED(amtp) ? 1 : 0;
      }
      sv_unmagic(MUTABLE_SV(stash), PERL_MAGIC_overload_table);
  }

  DEBUG_o( Perl_deb(aTHX_ "Recalcing overload magic in package %s\n",HvNAME_get(stash)) );

  Zero(&amt,1,AMT);
  amt.was_ok_am = PL_amagic_generation;
  amt.was_ok_sub = newgen;
  amt.fallback = AMGfallNO;
  amt.flags = 0;

  {
    int filled = 0, have_ovl = 0;
    int i, lim = 1;

    /* Work with "fallback" key, which we assume to be first in PL_AMG_names */

    /* Try to find via inheritance. */
    GV *gv = gv_fetchmeth_pvn(stash, PL_AMG_names[0], 2, -1, 0);
    SV * const sv = gv ? GvSV(gv) : NULL;
    CV* cv;

    if (!gv)
	lim = DESTROY_amg;		/* Skip overloading entries. */
#ifdef PERL_DONT_CREATE_GVSV
    else if (!sv) {
	NOOP;   /* Equivalent to !SvTRUE and !SvOK  */
    }
#endif
    else if (SvTRUE(sv))
	amt.fallback=AMGfallYES;
    else if (SvOK(sv))
	amt.fallback=AMGfallNEVER;

    for (i = 1; i < lim; i++)
	amt.table[i] = NULL;
    for (; i < NofAMmeth; i++) {
	const char * const cooky = PL_AMG_names[i];
	/* Human-readable form, for debugging: */
	const char * const cp = (i >= DESTROY_amg ? cooky : AMG_id2name(i));
	const STRLEN l = PL_AMG_namelens[i];

	DEBUG_o( Perl_deb(aTHX_ "Checking overloading of \"%s\" in package \"%.256s\"\n",
		     cp, HvNAME_get(stash)) );
	/* don't fill the cache while looking up!
	   Creation of inheritance stubs in intermediate packages may
	   conflict with the logic of runtime method substitution.
	   Indeed, for inheritance A -> B -> C, if C overloads "+0",
	   then we could have created stubs for "(+0" in A and C too.
	   But if B overloads "bool", we may want to use it for
	   numifying instead of C's "+0". */
	if (i >= DESTROY_amg)
	    gv = Perl_gv_fetchmeth_pvn_autoload(aTHX_ stash, cooky, l, 0, 0);
	else				/* Autoload taken care of below */
	    gv = Perl_gv_fetchmeth_pvn(aTHX_ stash, cooky, l, -1, 0);
        cv = 0;
        if (gv && (cv = GvCV(gv))) {
	    if(GvNAMELEN(CvGV(cv)) == 3 && strEQ(GvNAME(CvGV(cv)), "nil")){
	      const char * const hvname = HvNAME_get(GvSTASH(CvGV(cv)));
	      if (hvname && HEK_LEN(HvNAME_HEK(GvSTASH(CvGV(cv)))) == 8
	       && strEQ(hvname, "overload")) {
		/* This is a hack to support autoloading..., while
		   knowing *which* methods were declared as overloaded. */
		/* GvSV contains the name of the method. */
		GV *ngv = NULL;
		SV *gvsv = GvSV(gv);

		DEBUG_o( Perl_deb(aTHX_ "Resolving method \"%"SVf256\
			"\" for overloaded \"%s\" in package \"%.256s\"\n",
			     (void*)GvSV(gv), cp, HvNAME(stash)) );
		if (!gvsv || !SvPOK(gvsv)
		    || !(ngv = gv_fetchmethod_sv_flags(stash, gvsv, 0)))
		{
		    /* Can be an import stub (created by "can"). */
		    if (destructing) {
			return -1;
		    }
		    else {
			const SV * const name = (gvsv && SvPOK(gvsv))
                                                    ? gvsv
                                                    : newSVpvs_flags("???", SVs_TEMP);
			/* diag_listed_as: Can't resolve method "%s" overloading "%s" in package "%s" */
			Perl_croak(aTHX_ "%s method \"%"SVf256
				    "\" overloading \"%s\" "\
				    "in package \"%"HEKf256"\"",
				   (GvCVGEN(gv) ? "Stub found while resolving"
				    : "Can't resolve"),
				   SVfARG(name), cp,
                                   HEKfARG(
					HvNAME_HEK(stash)
				   ));
		    }
		}
		cv = GvCV(gv = ngv);
	      }
	    }
	    DEBUG_o( Perl_deb(aTHX_ "Overloading \"%s\" in package \"%.256s\" via \"%.256s::%.256s\"\n",
			 cp, HvNAME_get(stash), HvNAME_get(GvSTASH(CvGV(cv))),
			 GvNAME(CvGV(cv))) );
	    filled = 1;
	    if (i < DESTROY_amg)
		have_ovl = 1;
	} else if (gv) {		/* Autoloaded... */
	    cv = MUTABLE_CV(gv);
	    filled = 1;
	}
	amt.table[i]=MUTABLE_CV(SvREFCNT_inc_simple(cv));
    }
    if (filled) {
      AMT_AMAGIC_on(&amt);
      if (have_ovl)
	  AMT_OVERLOADED_on(&amt);
      sv_magic(MUTABLE_SV(stash), 0, PERL_MAGIC_overload_table,
						(char*)&amt, sizeof(AMT));
      return have_ovl;
    }
  }
  /* Here we have no table: */
  /* no_table: */
  AMT_AMAGIC_off(&amt);
  sv_magic(MUTABLE_SV(stash), 0, PERL_MAGIC_overload_table,
						(char*)&amt, sizeof(AMTS));
  return 0;
}


CV*
Perl_gv_handler(pTHX_ HV *stash, I32 id)
{
    dVAR;
    MAGIC *mg;
    AMT *amtp;
    U32 newgen;
    struct mro_meta* stash_meta;

    if (!stash || !HvNAME_get(stash))
        return NULL;

    stash_meta = HvMROMETA(stash);
    newgen = PL_sub_generation + stash_meta->pkg_gen + stash_meta->cache_gen;

    mg = mg_find((const SV *)stash, PERL_MAGIC_overload_table);
    if (!mg) {
      do_update:
	/* If we're looking up a destructor to invoke, we must avoid
	 * that Gv_AMupdate croaks, because we might be dying already */
	if (Gv_AMupdate(stash, cBOOL(id == DESTROY_amg)) == -1) {
	    /* and if it didn't found a destructor, we fall back
	     * to a simpler method that will only look for the
	     * destructor instead of the whole magic */
	    if (id == DESTROY_amg) {
		GV * const gv = gv_fetchmethod(stash, "DESTROY");
		if (gv)
		    return GvCV(gv);
	    }
	    return NULL;
	}
	mg = mg_find((const SV *)stash, PERL_MAGIC_overload_table);
    }
    assert(mg);
    amtp = (AMT*)mg->mg_ptr;
    if ( amtp->was_ok_am != PL_amagic_generation
	 || amtp->was_ok_sub != newgen )
	goto do_update;
    if (AMT_AMAGIC(amtp)) {
	CV * const ret = amtp->table[id];
	if (ret && isGV(ret)) {		/* Autoloading stab */
	    /* Passing it through may have resulted in a warning
	       "Inherited AUTOLOAD for a non-method deprecated", since
	       our caller is going through a function call, not a method call.
	       So return the CV for AUTOLOAD, setting $AUTOLOAD. */
	    GV * const gv = gv_fetchmethod(stash, PL_AMG_names[id]);

	    if (gv && GvCV(gv))
		return GvCV(gv);
	}
	return ret;
    }

    return NULL;
}


/* Implement tryAMAGICun_MG macro.
   Do get magic, then see if the stack arg is overloaded and if so call it.
   Flags:
	AMGf_set     return the arg using SETs rather than assigning to
		     the targ
	AMGf_numeric apply sv_2num to the stack arg.
*/

bool
Perl_try_amagic_un(pTHX_ int method, int flags) {
    dVAR;
    dSP;
    SV* tmpsv;
    SV* const arg = TOPs;

    SvGETMAGIC(arg);

    if (SvAMAGIC(arg) && (tmpsv = amagic_call(arg, &PL_sv_undef, method,
					      AMGf_noright | AMGf_unary))) {
	if (flags & AMGf_set) {
	    SETs(tmpsv);
	}
	else {
	    dTARGET;
	    if (SvPADMY(TARG)) {
		sv_setsv(TARG, tmpsv);
		SETTARG;
	    }
	    else
		SETs(tmpsv);
	}
	PUTBACK;
	return TRUE;
    }

    if ((flags & AMGf_numeric) && SvROK(arg))
	*sp = sv_2num(arg);
    return FALSE;
}


/* Implement tryAMAGICbin_MG macro.
   Do get magic, then see if the two stack args are overloaded and if so
   call it.
   Flags:
	AMGf_set     return the arg using SETs rather than assigning to
		     the targ
	AMGf_assign  op may be called as mutator (eg +=)
	AMGf_numeric apply sv_2num to the stack arg.
*/

bool
Perl_try_amagic_bin(pTHX_ int method, int flags) {
    dVAR;
    dSP;
    SV* const left = TOPm1s;
    SV* const right = TOPs;

    SvGETMAGIC(left);
    if (left != right)
	SvGETMAGIC(right);

    if (SvAMAGIC(left) || SvAMAGIC(right)) {
	SV * const tmpsv = amagic_call(left, right, method,
		    ((flags & AMGf_assign) && opASSIGN ? AMGf_assign: 0));
	if (tmpsv) {
	    if (flags & AMGf_set) {
		(void)POPs;
		SETs(tmpsv);
	    }
	    else {
		dATARGET;
		(void)POPs;
		if (opASSIGN || SvPADMY(TARG)) {
		    sv_setsv(TARG, tmpsv);
		    SETTARG;
		}
		else
		    SETs(tmpsv);
	    }
	    PUTBACK;
	    return TRUE;
	}
    }
    if(left==right && SvGMAGICAL(left)) {
	SV * const left = sv_newmortal();
	*(sp-1) = left;
	/* Print the uninitialized warning now, so it includes the vari-
	   able name. */
	if (!SvOK(right)) {
	    if (ckWARN(WARN_UNINITIALIZED)) report_uninit(right);
	    sv_setsv_flags(left, &PL_sv_no, 0);
	}
	else sv_setsv_flags(left, right, 0);
	SvGETMAGIC(right);
    }
    if (flags & AMGf_numeric) {
	if (SvROK(TOPm1s))
	    *(sp-1) = sv_2num(TOPm1s);
	if (SvROK(right))
	    *sp     = sv_2num(right);
    }
    return FALSE;
}

SV *
Perl_amagic_deref_call(pTHX_ SV *ref, int method) {
    SV *tmpsv = NULL;

    PERL_ARGS_ASSERT_AMAGIC_DEREF_CALL;

    while (SvAMAGIC(ref) && 
	   (tmpsv = amagic_call(ref, &PL_sv_undef, method,
				AMGf_noright | AMGf_unary))) { 
	if (!SvROK(tmpsv))
	    Perl_croak(aTHX_ "Overloaded dereference did not return a reference");
	if (tmpsv == ref || SvRV(tmpsv) == SvRV(ref)) {
	    /* Bail out if it returns us the same reference.  */
	    return tmpsv;
	}
	ref = tmpsv;
    }
    return tmpsv ? tmpsv : ref;
}

bool
Perl_amagic_is_enabled(pTHX_ int method)
{
      SV *lex_mask = cop_hints_fetch_pvs(PL_curcop, "overloading", 0);

      assert(PL_curcop->cop_hints & HINT_NO_AMAGIC);

      if ( !lex_mask || !SvOK(lex_mask) )
	  /* overloading lexically disabled */
	  return FALSE;
      else if ( lex_mask && SvPOK(lex_mask) ) {
	  /* we have an entry in the hints hash, check if method has been
	   * masked by overloading.pm */
	  STRLEN len;
	  const int offset = method / 8;
	  const int bit    = method % 8;
	  char *pv = SvPV(lex_mask, len);

	  /* Bit set, so this overloading operator is disabled */
	  if ( (STRLEN)offset < len && pv[offset] & ( 1 << bit ) )
	      return FALSE;
      }
      return TRUE;
}

SV*
Perl_amagic_call(pTHX_ SV *left, SV *right, int method, int flags)
{
  dVAR;
  MAGIC *mg;
  CV *cv=NULL;
  CV **cvp=NULL, **ocvp=NULL;
  AMT *amtp=NULL, *oamtp=NULL;
  int off = 0, off1, lr = 0, notfound = 0;
  int postpr = 0, force_cpy = 0;
  int assign = AMGf_assign & flags;
  const int assignshift = assign ? 1 : 0;
  int use_default_op = 0;
#ifdef DEBUGGING
  int fl=0;
#endif
  HV* stash=NULL;

  PERL_ARGS_ASSERT_AMAGIC_CALL;

  if ( PL_curcop->cop_hints & HINT_NO_AMAGIC ) {
      if (!amagic_is_enabled(method)) return NULL;
  }

  if (!(AMGf_noleft & flags) && SvAMAGIC(left)
      && (stash = SvSTASH(SvRV(left)))
      && (mg = mg_find((const SV *)stash, PERL_MAGIC_overload_table))
      && (ocvp = cvp = (AMT_AMAGIC((AMT*)mg->mg_ptr)
			? (oamtp = amtp = (AMT*)mg->mg_ptr)->table
			: NULL))
      && ((cv = cvp[off=method+assignshift])
	  || (assign && amtp->fallback > AMGfallNEVER && /* fallback to
						          * usual method */
		  (
#ifdef DEBUGGING
		   fl = 1,
#endif
		   cv = cvp[off=method])))) {
    lr = -1;			/* Call method for left argument */
  } else {
    if (cvp && amtp->fallback > AMGfallNEVER && flags & AMGf_unary) {
      int logic;

      /* look for substituted methods */
      /* In all the covered cases we should be called with assign==0. */
	 switch (method) {
	 case inc_amg:
	   force_cpy = 1;
	   if ((cv = cvp[off=add_ass_amg])
	       || ((cv = cvp[off = add_amg]) && (force_cpy = 0, postpr = 1))) {
	     right = &PL_sv_yes; lr = -1; assign = 1;
	   }
	   break;
	 case dec_amg:
	   force_cpy = 1;
	   if ((cv = cvp[off = subtr_ass_amg])
	       || ((cv = cvp[off = subtr_amg]) && (force_cpy = 0, postpr=1))) {
	     right = &PL_sv_yes; lr = -1; assign = 1;
	   }
	   break;
	 case bool__amg:
	   (void)((cv = cvp[off=numer_amg]) || (cv = cvp[off=string_amg]));
	   break;
	 case numer_amg:
	   (void)((cv = cvp[off=string_amg]) || (cv = cvp[off=bool__amg]));
	   break;
	 case string_amg:
	   (void)((cv = cvp[off=numer_amg]) || (cv = cvp[off=bool__amg]));
	   break;
         case not_amg:
           (void)((cv = cvp[off=bool__amg])
                  || (cv = cvp[off=numer_amg])
                  || (cv = cvp[off=string_amg]));
           if (cv)
               postpr = 1;
           break;
	 case copy_amg:
	   {
	     /*
		  * SV* ref causes confusion with the interpreter variable of
		  * the same name
		  */
	     SV* const tmpRef=SvRV(left);
	     if (!SvROK(tmpRef) && SvTYPE(tmpRef) <= SVt_PVMG) {
		/*
		 * Just to be extra cautious.  Maybe in some
		 * additional cases sv_setsv is safe, too.
		 */
		SV* const newref = newSVsv(tmpRef);
		SvOBJECT_on(newref);
		/* As a bit of a source compatibility hack, SvAMAGIC() and
		   friends dereference an RV, to behave the same was as when
		   overloading was stored on the reference, not the referant.
		   Hence we can't use SvAMAGIC_on()
		*/
		SvFLAGS(newref) |= SVf_AMAGIC;
		SvSTASH_set(newref, MUTABLE_HV(SvREFCNT_inc(SvSTASH(tmpRef))));
		return newref;
	     }
	   }
	   break;
	 case abs_amg:
	   if ((cvp[off1=lt_amg] || cvp[off1=ncmp_amg])
	       && ((cv = cvp[off=neg_amg]) || (cv = cvp[off=subtr_amg]))) {
	     SV* const nullsv=sv_2mortal(newSViv(0));
	     if (off1==lt_amg) {
	       SV* const lessp = amagic_call(left,nullsv,
				       lt_amg,AMGf_noright);
	       logic = SvTRUE(lessp);
	     } else {
	       SV* const lessp = amagic_call(left,nullsv,
				       ncmp_amg,AMGf_noright);
	       logic = (SvNV(lessp) < 0);
	     }
	     if (logic) {
	       if (off==subtr_amg) {
		 right = left;
		 left = nullsv;
		 lr = 1;
	       }
	     } else {
	       return left;
	     }
	   }
	   break;
	 case neg_amg:
	   if ((cv = cvp[off=subtr_amg])) {
	     right = left;
	     left = sv_2mortal(newSViv(0));
	     lr = 1;
	   }
	   break;
	 case int_amg:
	 case iter_amg:			/* XXXX Eventually should do to_gv. */
	 case ftest_amg:		/* XXXX Eventually should do to_gv. */
	 case regexp_amg:
	     /* FAIL safe */
	     return NULL;	/* Delegate operation to standard mechanisms. */
	     break;
	 case to_sv_amg:
	 case to_av_amg:
	 case to_hv_amg:
	 case to_gv_amg:
	 case to_cv_amg:
	     /* FAIL safe */
	     return left;	/* Delegate operation to standard mechanisms. */
	     break;
	 default:
	   goto not_found;
	 }
	 if (!cv) goto not_found;
    } else if (!(AMGf_noright & flags) && SvAMAGIC(right)
	       && (stash = SvSTASH(SvRV(right)))
	       && (mg = mg_find((const SV *)stash, PERL_MAGIC_overload_table))
	       && (cvp = (AMT_AMAGIC((AMT*)mg->mg_ptr)
			  ? (amtp = (AMT*)mg->mg_ptr)->table
			  : NULL))
	       && (cv = cvp[off=method])) { /* Method for right
					     * argument found */
      lr=1;
    } else if (((cvp && amtp->fallback > AMGfallNEVER)
                || (ocvp && oamtp->fallback > AMGfallNEVER))
	       && !(flags & AMGf_unary)) {
				/* We look for substitution for
				 * comparison operations and
				 * concatenation */
      if (method==concat_amg || method==concat_ass_amg
	  || method==repeat_amg || method==repeat_ass_amg) {
	return NULL;		/* Delegate operation to string conversion */
      }
      off = -1;
      switch (method) {
	 case lt_amg:
	 case le_amg:
	 case gt_amg:
	 case ge_amg:
	 case eq_amg:
	 case ne_amg:
             off = ncmp_amg;
             break;
	 case slt_amg:
	 case sle_amg:
	 case sgt_amg:
	 case sge_amg:
	 case seq_amg:
	 case sne_amg:
             off = scmp_amg;
             break;
	 }
      if (off != -1) {
          if (ocvp && (oamtp->fallback > AMGfallNEVER)) {
              cv = ocvp[off];
              lr = -1;
          }
          if (!cv && (cvp && amtp->fallback > AMGfallNEVER)) {
              cv = cvp[off];
              lr = 1;
          }
      }
      if (cv)
          postpr = 1;
      else
          goto not_found;
    } else {
    not_found:			/* No method found, either report or croak */
      switch (method) {
	 case to_sv_amg:
	 case to_av_amg:
	 case to_hv_amg:
	 case to_gv_amg:
	 case to_cv_amg:
	     /* FAIL safe */
	     return left;	/* Delegate operation to standard mechanisms. */
	     break;
      }
      if (ocvp && (cv=ocvp[nomethod_amg])) { /* Call report method */
	notfound = 1; lr = -1;
      } else if (cvp && (cv=cvp[nomethod_amg])) {
	notfound = 1; lr = 1;
      } else if ((use_default_op =
                  (!ocvp || oamtp->fallback >= AMGfallYES)
                  && (!cvp || amtp->fallback >= AMGfallYES))
                 && !DEBUG_o_TEST) {
	/* Skip generating the "no method found" message.  */
	return NULL;
      } else {
	SV *msg;
	if (off==-1) off=method;
	msg = sv_2mortal(Perl_newSVpvf(aTHX_
		      "Operation \"%s\": no method found,%sargument %s%"SVf"%s%"SVf,
 		      AMG_id2name(method + assignshift),
 		      (flags & AMGf_unary ? " " : "\n\tleft "),
 		      SvAMAGIC(left)?
 		        "in overloaded package ":
 		        "has no overloaded magic",
 		      SvAMAGIC(left)?
		        SVfARG(sv_2mortal(newSVhek(HvNAME_HEK(SvSTASH(SvRV(left)))))):
		        SVfARG(&PL_sv_no),
 		      SvAMAGIC(right)?
 		        ",\n\tright argument in overloaded package ":
 		        (flags & AMGf_unary
 			 ? ""
 			 : ",\n\tright argument has no overloaded magic"),
 		      SvAMAGIC(right)?
		        SVfARG(sv_2mortal(newSVhek(HvNAME_HEK(SvSTASH(SvRV(right)))))):
		        SVfARG(&PL_sv_no)));
        if (use_default_op) {
	  DEBUG_o( Perl_deb(aTHX_ "%"SVf, SVfARG(msg)) );
	} else {
	  Perl_croak(aTHX_ "%"SVf, SVfARG(msg));
	}
	return NULL;
      }
      force_cpy = force_cpy || assign;
    }
  }
#ifdef DEBUGGING
  if (!notfound) {
    DEBUG_o(Perl_deb(aTHX_
		     "Overloaded operator \"%s\"%s%s%s:\n\tmethod%s found%s in package %"SVf"%s\n",
		     AMG_id2name(off),
		     method+assignshift==off? "" :
		     " (initially \"",
		     method+assignshift==off? "" :
		     AMG_id2name(method+assignshift),
		     method+assignshift==off? "" : "\")",
		     flags & AMGf_unary? "" :
		     lr==1 ? " for right argument": " for left argument",
		     flags & AMGf_unary? " for argument" : "",
		     stash ? SVfARG(sv_2mortal(newSVhek(HvNAME_HEK(stash)))) : SVfARG(newSVpvs_flags("null", SVs_TEMP)),
		     fl? ",\n\tassignment variant used": "") );
  }
#endif
    /* Since we use shallow copy during assignment, we need
     * to dublicate the contents, probably calling user-supplied
     * version of copy operator
     */
    /* We need to copy in following cases:
     * a) Assignment form was called.
     * 		assignshift==1,  assign==T, method + 1 == off
     * b) Increment or decrement, called directly.
     * 		assignshift==0,  assign==0, method + 0 == off
     * c) Increment or decrement, translated to assignment add/subtr.
     * 		assignshift==0,  assign==T,
     *		force_cpy == T
     * d) Increment or decrement, translated to nomethod.
     * 		assignshift==0,  assign==0,
     *		force_cpy == T
     * e) Assignment form translated to nomethod.
     * 		assignshift==1,  assign==T, method + 1 != off
     *		force_cpy == T
     */
    /*	off is method, method+assignshift, or a result of opcode substitution.
     *	In the latter case assignshift==0, so only notfound case is important.
     */
  if ( (lr == -1) && ( ( (method + assignshift == off)
	&& (assign || (method == inc_amg) || (method == dec_amg)))
      || force_cpy) )
  {
      /* newSVsv does not behave as advertised, so we copy missing
       * information by hand */
      SV *tmpRef = SvRV(left);
      SV *rv_copy;
      if (SvREFCNT(tmpRef) > 1 && (rv_copy = AMG_CALLunary(left,copy_amg))) {
	  SvRV_set(left, rv_copy);
	  SvSETMAGIC(left);
	  SvREFCNT_dec(tmpRef);  
      }
  }

  {
    dSP;
    BINOP myop;
    SV* res;
    const bool oldcatch = CATCH_GET;

    CATCH_SET(TRUE);
    Zero(&myop, 1, BINOP);
    myop.op_last = (OP *) &myop;
    myop.op_next = NULL;
    myop.op_flags = OPf_WANT_SCALAR | OPf_STACKED;

    PUSHSTACKi(PERLSI_OVERLOAD);
    ENTER;
    SAVEOP();
    PL_op = (OP *) &myop;
    if (PERLDB_SUB && PL_curstash != PL_debstash)
	PL_op->op_private |= OPpENTERSUB_DB;
    PUTBACK;
    Perl_pp_pushmark(aTHX);

    EXTEND(SP, notfound + 5);
    PUSHs(lr>0? right: left);
    PUSHs(lr>0? left: right);
    PUSHs( lr > 0 ? &PL_sv_yes : ( assign ? &PL_sv_undef : &PL_sv_no ));
    if (notfound) {
      PUSHs(newSVpvn_flags(AMG_id2name(method + assignshift),
			   AMG_id2namelen(method + assignshift), SVs_TEMP));
    }
    PUSHs(MUTABLE_SV(cv));
    PUTBACK;

    if ((PL_op = PL_ppaddr[OP_ENTERSUB](aTHX)))
      CALLRUNOPS(aTHX);
    LEAVE;
    SPAGAIN;

    res=POPs;
    PUTBACK;
    POPSTACK;
    CATCH_SET(oldcatch);

    if (postpr) {
      int ans;
      switch (method) {
      case le_amg:
      case sle_amg:
	ans=SvIV(res)<=0; break;
      case lt_amg:
      case slt_amg:
	ans=SvIV(res)<0; break;
      case ge_amg:
      case sge_amg:
	ans=SvIV(res)>=0; break;
      case gt_amg:
      case sgt_amg:
	ans=SvIV(res)>0; break;
      case eq_amg:
      case seq_amg:
	ans=SvIV(res)==0; break;
      case ne_amg:
      case sne_amg:
	ans=SvIV(res)!=0; break;
      case inc_amg:
      case dec_amg:
	SvSetSV(left,res); return left;
      case not_amg:
	ans=!SvTRUE(res); break;
      default:
        ans=0; break;
      }
      return boolSV(ans);
    } else if (method==copy_amg) {
      if (!SvROK(res)) {
	Perl_croak(aTHX_ "Copy method did not return a reference");
      }
      return SvREFCNT_inc(SvRV(res));
    } else {
      return res;
    }
  }
}

void
Perl_gv_name_set(pTHX_ GV *gv, const char *name, U32 len, U32 flags)
{
    dVAR;
    U32 hash;

    PERL_ARGS_ASSERT_GV_NAME_SET;

    if (len > I32_MAX)
	Perl_croak(aTHX_ "panic: gv name too long (%"UVuf")", (UV) len);

    if (!(flags & GV_ADD) && GvNAME_HEK(gv)) {
	unshare_hek(GvNAME_HEK(gv));
    }

    PERL_HASH(hash, name, len);
    GvNAME_HEK(gv) = share_hek(name, (flags & SVf_UTF8 ? -(I32)len : (I32)len), hash);
}

/*
=for apidoc gv_try_downgrade

If the typeglob C<gv> can be expressed more succinctly, by having
something other than a real GV in its place in the stash, replace it
with the optimised form.  Basic requirements for this are that C<gv>
is a real typeglob, is sufficiently ordinary, and is only referenced
from its package.  This function is meant to be used when a GV has been
looked up in part to see what was there, causing upgrading, but based
on what was found it turns out that the real GV isn't required after all.

If C<gv> is a completely empty typeglob, it is deleted from the stash.

If C<gv> is a typeglob containing only a sufficiently-ordinary constant
sub, the typeglob is replaced with a scalar-reference placeholder that
more compactly represents the same thing.

=cut
*/

void
Perl_gv_try_downgrade(pTHX_ GV *gv)
{
    HV *stash;
    CV *cv;
    HEK *namehek;
    SV **gvp;
    PERL_ARGS_ASSERT_GV_TRY_DOWNGRADE;

    /* XXX Why and where does this leave dangling pointers during global
       destruction? */
    if (PL_phase == PERL_PHASE_DESTRUCT) return;

    if (!(SvREFCNT(gv) == 1 && SvTYPE(gv) == SVt_PVGV && !SvFAKE(gv) &&
	    !SvOBJECT(gv) && !SvREADONLY(gv) &&
	    isGV_with_GP(gv) && GvGP(gv) &&
	    !GvINTRO(gv) && GvREFCNT(gv) == 1 &&
	    !GvSV(gv) && !GvAV(gv) && !GvHV(gv) && !GvIOp(gv) && !GvFORM(gv) &&
	    GvEGVx(gv) == gv && (stash = GvSTASH(gv))))
	return;
    if (SvMAGICAL(gv)) {
        MAGIC *mg;
	/* only backref magic is allowed */
	if (SvGMAGICAL(gv) || SvSMAGICAL(gv))
	    return;
        for (mg = SvMAGIC(gv); mg; mg = mg->mg_moremagic) {
            if (mg->mg_type != PERL_MAGIC_backref)
                return;
	}
    }
    cv = GvCV(gv);
    if (!cv) {
	HEK *gvnhek = GvNAME_HEK(gv);
	(void)hv_delete(stash, HEK_KEY(gvnhek),
	    HEK_UTF8(gvnhek) ? -HEK_LEN(gvnhek) : HEK_LEN(gvnhek), G_DISCARD);
    } else if (GvMULTI(gv) && cv &&
	    !SvOBJECT(cv) && !SvMAGICAL(cv) && !SvREADONLY(cv) &&
	    CvSTASH(cv) == stash && CvGV(cv) == gv &&
	    CvCONST(cv) && !CvMETHOD(cv) && !CvLVALUE(cv) && !CvUNIQUE(cv) &&
	    !CvNODEBUG(cv) && !CvCLONE(cv) && !CvCLONED(cv) && !CvANON(cv) &&
	    (namehek = GvNAME_HEK(gv)) &&
	    (gvp = hv_fetch(stash, HEK_KEY(namehek),
			HEK_LEN(namehek)*(HEK_UTF8(namehek) ? -1 : 1), 0)) &&
	    *gvp == (SV*)gv) {
	SV *value = SvREFCNT_inc(CvXSUBANY(cv).any_ptr);
	SvREFCNT(gv) = 0;
	sv_clear((SV*)gv);
	SvREFCNT(gv) = 1;
	SvFLAGS(gv) = SVt_IV|SVf_ROK;
	SvANY(gv) = (XPVGV*)((char*)&(gv->sv_u.svu_iv) -
				STRUCT_OFFSET(XPVIV, xiv_iv));
	SvRV_set(gv, value);
    }
}

#include "XSUB.h"

static void
core_xsub(pTHX_ CV* cv)
{
    Perl_croak(aTHX_
       "&CORE::%s cannot be called directly", GvNAME(CvGV(cv))
    );
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
