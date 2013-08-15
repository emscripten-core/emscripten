/*    pp.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'It's a big house this, and very peculiar.  Always a bit more
 *  to discover, and no knowing what you'll find round a corner.
 *  And Elves, sir!'                            --Samwise Gamgee
 *
 *     [p.225 of _The Lord of the Rings_, II/i: "Many Meetings"]
 */

/* This file contains general pp ("push/pop") functions that execute the
 * opcodes that make up a perl program. A typical pp function expects to
 * find its arguments on the stack, and usually pushes its results onto
 * the stack, hence the 'pp' terminology. Each OP structure contains
 * a pointer to the relevant pp_foo() function.
 */

#include "EXTERN.h"
#define PERL_IN_PP_C
#include "perl.h"
#include "keywords.h"

#include "reentr.h"

/* XXX I can't imagine anyone who doesn't have this actually _needs_
   it, since pid_t is an integral type.
   --AD  2/20/1998
*/
#ifdef NEED_GETPID_PROTO
extern Pid_t getpid (void);
#endif

/*
 * Some BSDs and Cygwin default to POSIX math instead of IEEE.
 * This switches them over to IEEE.
 */
#if defined(LIBM_LIB_VERSION)
    _LIB_VERSION_TYPE _LIB_VERSION = _IEEE_;
#endif

/* variations on pp_null */

PP(pp_stub)
{
    dVAR;
    dSP;
    if (GIMME_V == G_SCALAR)
	XPUSHs(&PL_sv_undef);
    RETURN;
}

/* Pushy stuff. */

PP(pp_padav)
{
    dVAR; dSP; dTARGET;
    I32 gimme;
    assert(SvTYPE(TARG) == SVt_PVAV);
    if (PL_op->op_private & OPpLVAL_INTRO)
	if (!(PL_op->op_private & OPpPAD_STATE))
	    SAVECLEARSV(PAD_SVl(PL_op->op_targ));
    EXTEND(SP, 1);
    if (PL_op->op_flags & OPf_REF) {
	PUSHs(TARG);
	RETURN;
    } else if (PL_op->op_private & OPpMAYBE_LVSUB) {
       const I32 flags = is_lvalue_sub();
       if (flags && !(flags & OPpENTERSUB_INARGS)) {
	if (GIMME == G_SCALAR)
	    /* diag_listed_as: Can't return %s to lvalue scalar context */
	    Perl_croak(aTHX_ "Can't return array to lvalue scalar context");
	PUSHs(TARG);
	RETURN;
       }
    }
    gimme = GIMME_V;
    if (gimme == G_ARRAY) {
	const I32 maxarg = AvFILL(MUTABLE_AV(TARG)) + 1;
	EXTEND(SP, maxarg);
	if (SvMAGICAL(TARG)) {
	    U32 i;
	    for (i=0; i < (U32)maxarg; i++) {
		SV * const * const svp = av_fetch(MUTABLE_AV(TARG), i, FALSE);
		SP[i+1] = (svp) ? *svp : &PL_sv_undef;
	    }
	}
	else {
	    Copy(AvARRAY((const AV *)TARG), SP+1, maxarg, SV*);
	}
	SP += maxarg;
    }
    else if (gimme == G_SCALAR) {
	SV* const sv = sv_newmortal();
	const I32 maxarg = AvFILL(MUTABLE_AV(TARG)) + 1;
	sv_setiv(sv, maxarg);
	PUSHs(sv);
    }
    RETURN;
}

PP(pp_padhv)
{
    dVAR; dSP; dTARGET;
    I32 gimme;

    assert(SvTYPE(TARG) == SVt_PVHV);
    XPUSHs(TARG);
    if (PL_op->op_private & OPpLVAL_INTRO)
	if (!(PL_op->op_private & OPpPAD_STATE))
	    SAVECLEARSV(PAD_SVl(PL_op->op_targ));
    if (PL_op->op_flags & OPf_REF)
	RETURN;
    else if (PL_op->op_private & OPpMAYBE_LVSUB) {
      const I32 flags = is_lvalue_sub();
      if (flags && !(flags & OPpENTERSUB_INARGS)) {
	if (GIMME == G_SCALAR)
	    /* diag_listed_as: Can't return %s to lvalue scalar context */
	    Perl_croak(aTHX_ "Can't return hash to lvalue scalar context");
	RETURN;
      }
    }
    gimme = GIMME_V;
    if (gimme == G_ARRAY) {
	RETURNOP(Perl_do_kv(aTHX));
    }
    else if (gimme == G_SCALAR) {
	SV* const sv = Perl_hv_scalar(aTHX_ MUTABLE_HV(TARG));
	SETs(sv);
    }
    RETURN;
}

/* Translations. */

static const char S_no_symref_sv[] =
    "Can't use string (\"%" SVf32 "\"%s) as %s ref while \"strict refs\" in use";

/* In some cases this function inspects PL_op.  If this function is called
   for new op types, more bool parameters may need to be added in place of
   the checks.

   When noinit is true, the absence of a gv will cause a retval of undef.
   This is unrelated to the cv-to-gv assignment case.
*/

static SV *
S_rv2gv(pTHX_ SV *sv, const bool vivify_sv, const bool strict,
              const bool noinit)
{
    dVAR;
    if (!isGV(sv) || SvFAKE(sv)) SvGETMAGIC(sv);
    if (SvROK(sv)) {
	if (SvAMAGIC(sv)) {
	    sv = amagic_deref_call(sv, to_gv_amg);
	}
      wasref:
	sv = SvRV(sv);
	if (SvTYPE(sv) == SVt_PVIO) {
	    GV * const gv = MUTABLE_GV(sv_newmortal());
	    gv_init(gv, 0, "__ANONIO__", 10, 0);
	    GvIOp(gv) = MUTABLE_IO(sv);
	    SvREFCNT_inc_void_NN(sv);
	    sv = MUTABLE_SV(gv);
	}
	else if (!isGV_with_GP(sv))
	    return (SV *)Perl_die(aTHX_ "Not a GLOB reference");
    }
    else {
	if (!isGV_with_GP(sv)) {
	    if (!SvOK(sv)) {
		/* If this is a 'my' scalar and flag is set then vivify
		 * NI-S 1999/05/07
		 */
		if (vivify_sv && sv != &PL_sv_undef) {
		    GV *gv;
		    if (SvREADONLY(sv))
			Perl_croak_no_modify(aTHX);
		    if (cUNOP->op_targ) {
			SV * const namesv = PAD_SV(cUNOP->op_targ);
			gv = MUTABLE_GV(newSV(0));
			gv_init_sv(gv, CopSTASH(PL_curcop), namesv, 0);
		    }
		    else {
			const char * const name = CopSTASHPV(PL_curcop);
			gv = newGVgen_flags(name,
                                        HvNAMEUTF8(CopSTASH(PL_curcop)) ? SVf_UTF8 : 0 );
		    }
		    prepare_SV_for_RV(sv);
		    SvRV_set(sv, MUTABLE_SV(gv));
		    SvROK_on(sv);
		    SvSETMAGIC(sv);
		    goto wasref;
		}
		if (PL_op->op_flags & OPf_REF || strict)
		    return (SV *)Perl_die(aTHX_ PL_no_usym, "a symbol");
		if (ckWARN(WARN_UNINITIALIZED))
		    report_uninit(sv);
		return &PL_sv_undef;
	    }
	    if (noinit)
	    {
		if (!(sv = MUTABLE_SV(gv_fetchsv_nomg(
		           sv, GV_ADDMG, SVt_PVGV
		   ))))
		    return &PL_sv_undef;
	    }
	    else {
		if (strict)
		    return
		     (SV *)Perl_die(aTHX_
		            S_no_symref_sv,
		            sv,
		            (SvPOK(sv) && SvCUR(sv)>32 ? "..." : ""),
		            "a symbol"
		           );
		if ((PL_op->op_private & (OPpLVAL_INTRO|OPpDONT_INIT_GV))
		    == OPpDONT_INIT_GV) {
		    /* We are the target of a coderef assignment.  Return
		       the scalar unchanged, and let pp_sasssign deal with
		       things.  */
		    return sv;
		}
		sv = MUTABLE_SV(gv_fetchsv_nomg(sv, GV_ADD, SVt_PVGV));
	    }
	    /* FAKE globs in the symbol table cause weird bugs (#77810) */
	    SvFAKE_off(sv);
	}
    }
    if (SvFAKE(sv) && !(PL_op->op_private & OPpALLOW_FAKE)) {
	SV *newsv = sv_newmortal();
	sv_setsv_flags(newsv, sv, 0);
	SvFAKE_off(newsv);
	sv = newsv;
    }
    return sv;
}

PP(pp_rv2gv)
{
    dVAR; dSP; dTOPss;

    sv = S_rv2gv(aTHX_
          sv, PL_op->op_private & OPpDEREF,
          PL_op->op_private & HINT_STRICT_REFS,
          ((PL_op->op_flags & OPf_SPECIAL) && !(PL_op->op_flags & OPf_MOD))
             || PL_op->op_type == OP_READLINE
         );
    if (PL_op->op_private & OPpLVAL_INTRO)
	save_gp(MUTABLE_GV(sv), !(PL_op->op_flags & OPf_SPECIAL));
    SETs(sv);
    RETURN;
}

/* Helper function for pp_rv2sv and pp_rv2av  */
GV *
Perl_softref2xv(pTHX_ SV *const sv, const char *const what,
		const svtype type, SV ***spp)
{
    dVAR;
    GV *gv;

    PERL_ARGS_ASSERT_SOFTREF2XV;

    if (PL_op->op_private & HINT_STRICT_REFS) {
	if (SvOK(sv))
	    Perl_die(aTHX_ S_no_symref_sv, sv, (SvPOK(sv) && SvCUR(sv)>32 ? "..." : ""), what);
	else
	    Perl_die(aTHX_ PL_no_usym, what);
    }
    if (!SvOK(sv)) {
	if (
	  PL_op->op_flags & OPf_REF &&
	  PL_op->op_next->op_type != OP_BOOLKEYS
	)
	    Perl_die(aTHX_ PL_no_usym, what);
	if (ckWARN(WARN_UNINITIALIZED))
	    report_uninit(sv);
	if (type != SVt_PV && GIMME_V == G_ARRAY) {
	    (*spp)--;
	    return NULL;
	}
	**spp = &PL_sv_undef;
	return NULL;
    }
    if ((PL_op->op_flags & OPf_SPECIAL) &&
	!(PL_op->op_flags & OPf_MOD))
	{
	    if (!(gv = gv_fetchsv_nomg(sv, GV_ADDMG, type)))
		{
		    **spp = &PL_sv_undef;
		    return NULL;
		}
	}
    else {
	gv = gv_fetchsv_nomg(sv, GV_ADD, type);
    }
    return gv;
}

PP(pp_rv2sv)
{
    dVAR; dSP; dTOPss;
    GV *gv = NULL;

    SvGETMAGIC(sv);
    if (SvROK(sv)) {
	if (SvAMAGIC(sv)) {
	    sv = amagic_deref_call(sv, to_sv_amg);
	}

	sv = SvRV(sv);
	switch (SvTYPE(sv)) {
	case SVt_PVAV:
	case SVt_PVHV:
	case SVt_PVCV:
	case SVt_PVFM:
	case SVt_PVIO:
	    DIE(aTHX_ "Not a SCALAR reference");
	default: NOOP;
	}
    }
    else {
	gv = MUTABLE_GV(sv);

	if (!isGV_with_GP(gv)) {
	    gv = Perl_softref2xv(aTHX_ sv, "a SCALAR", SVt_PV, &sp);
	    if (!gv)
		RETURN;
	}
	sv = GvSVn(gv);
    }
    if (PL_op->op_flags & OPf_MOD) {
	if (PL_op->op_private & OPpLVAL_INTRO) {
	    if (cUNOP->op_first->op_type == OP_NULL)
		sv = save_scalar(MUTABLE_GV(TOPs));
	    else if (gv)
		sv = save_scalar(gv);
	    else
		Perl_croak(aTHX_ "%s", PL_no_localize_ref);
	}
	else if (PL_op->op_private & OPpDEREF)
	    sv = vivify_ref(sv, PL_op->op_private & OPpDEREF);
    }
    SETs(sv);
    RETURN;
}

PP(pp_av2arylen)
{
    dVAR; dSP;
    AV * const av = MUTABLE_AV(TOPs);
    const I32 lvalue = PL_op->op_flags & OPf_MOD || LVRET;
    if (lvalue) {
	SV ** const sv = Perl_av_arylen_p(aTHX_ MUTABLE_AV(av));
	if (!*sv) {
	    *sv = newSV_type(SVt_PVMG);
	    sv_magic(*sv, MUTABLE_SV(av), PERL_MAGIC_arylen, NULL, 0);
	}
	SETs(*sv);
    } else {
	SETs(sv_2mortal(newSViv(AvFILL(MUTABLE_AV(av)))));
    }
    RETURN;
}

PP(pp_pos)
{
    dVAR; dSP; dPOPss;

    if (PL_op->op_flags & OPf_MOD || LVRET) {
	SV * const ret = sv_2mortal(newSV_type(SVt_PVLV));  /* Not TARG RT#67838 */
	sv_magic(ret, NULL, PERL_MAGIC_pos, NULL, 0);
	LvTYPE(ret) = '.';
	LvTARG(ret) = SvREFCNT_inc_simple(sv);
	PUSHs(ret);    /* no SvSETMAGIC */
	RETURN;
    }
    else {
	if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
	    const MAGIC * const mg = mg_find(sv, PERL_MAGIC_regex_global);
	    if (mg && mg->mg_len >= 0) {
		dTARGET;
		I32 i = mg->mg_len;
		if (DO_UTF8(sv))
		    sv_pos_b2u(sv, &i);
		PUSHi(i);
		RETURN;
	    }
	}
	RETPUSHUNDEF;
    }
}

PP(pp_rv2cv)
{
    dVAR; dSP;
    GV *gv;
    HV *stash_unused;
    const I32 flags = (PL_op->op_flags & OPf_SPECIAL)
	? GV_ADDMG
	: ((PL_op->op_private & (OPpLVAL_INTRO|OPpMAY_RETURN_CONSTANT)) == OPpMAY_RETURN_CONSTANT)
	    ? GV_ADD|GV_NOEXPAND
	    : GV_ADD;
    /* We usually try to add a non-existent subroutine in case of AUTOLOAD. */
    /* (But not in defined().) */

    CV *cv = sv_2cv(TOPs, &stash_unused, &gv, flags);
    if (cv) {
	if (CvCLONE(cv))
	    cv = MUTABLE_CV(sv_2mortal(MUTABLE_SV(cv_clone(cv))));
    }
    else if ((flags == (GV_ADD|GV_NOEXPAND)) && gv && SvROK(gv)) {
	cv = MUTABLE_CV(gv);
    }    
    else
	cv = MUTABLE_CV(&PL_sv_undef);
    SETs(MUTABLE_SV(cv));
    RETURN;
}

PP(pp_prototype)
{
    dVAR; dSP;
    CV *cv;
    HV *stash;
    GV *gv;
    SV *ret = &PL_sv_undef;

    if (SvPOK(TOPs) && SvCUR(TOPs) >= 7) {
	const char * s = SvPVX_const(TOPs);
	if (strnEQ(s, "CORE::", 6)) {
	    const int code = keyword(s + 6, SvCUR(TOPs) - 6, 1);
	    if (!code || code == -KEY_CORE)
		DIE(aTHX_ "Can't find an opnumber for \"%s\"", s+6);
	    if (code < 0) {	/* Overridable. */
		SV * const sv = core_prototype(NULL, s + 6, code, NULL);
		if (sv) ret = sv;
	    }
	    goto set;
	}
    }
    cv = sv_2cv(TOPs, &stash, &gv, 0);
    if (cv && SvPOK(cv))
	ret = newSVpvn_flags(
	    CvPROTO(cv), CvPROTOLEN(cv), SVs_TEMP | SvUTF8(cv)
	);
  set:
    SETs(ret);
    RETURN;
}

PP(pp_anoncode)
{
    dVAR; dSP;
    CV *cv = MUTABLE_CV(PAD_SV(PL_op->op_targ));
    if (CvCLONE(cv))
	cv = MUTABLE_CV(sv_2mortal(MUTABLE_SV(cv_clone(cv))));
    EXTEND(SP,1);
    PUSHs(MUTABLE_SV(cv));
    RETURN;
}

PP(pp_srefgen)
{
    dVAR; dSP;
    *SP = refto(*SP);
    RETURN;
}

PP(pp_refgen)
{
    dVAR; dSP; dMARK;
    if (GIMME != G_ARRAY) {
	if (++MARK <= SP)
	    *MARK = *SP;
	else
	    *MARK = &PL_sv_undef;
	*MARK = refto(*MARK);
	SP = MARK;
	RETURN;
    }
    EXTEND_MORTAL(SP - MARK);
    while (++MARK <= SP)
	*MARK = refto(*MARK);
    RETURN;
}

STATIC SV*
S_refto(pTHX_ SV *sv)
{
    dVAR;
    SV* rv;

    PERL_ARGS_ASSERT_REFTO;

    if (SvTYPE(sv) == SVt_PVLV && LvTYPE(sv) == 'y') {
	if (LvTARGLEN(sv))
	    vivify_defelem(sv);
	if (!(sv = LvTARG(sv)))
	    sv = &PL_sv_undef;
	else
	    SvREFCNT_inc_void_NN(sv);
    }
    else if (SvTYPE(sv) == SVt_PVAV) {
	if (!AvREAL((const AV *)sv) && AvREIFY((const AV *)sv))
	    av_reify(MUTABLE_AV(sv));
	SvTEMP_off(sv);
	SvREFCNT_inc_void_NN(sv);
    }
    else if (SvPADTMP(sv) && !IS_PADGV(sv))
        sv = newSVsv(sv);
    else {
	SvTEMP_off(sv);
	SvREFCNT_inc_void_NN(sv);
    }
    rv = sv_newmortal();
    sv_upgrade(rv, SVt_IV);
    SvRV_set(rv, sv);
    SvROK_on(rv);
    return rv;
}

PP(pp_ref)
{
    dVAR; dSP; dTARGET;
    SV * const sv = POPs;

    if (sv)
	SvGETMAGIC(sv);

    if (!sv || !SvROK(sv))
	RETPUSHNO;

    (void)sv_ref(TARG,SvRV(sv),TRUE);
    PUSHTARG;
    RETURN;
}

PP(pp_bless)
{
    dVAR; dSP;
    HV *stash;

    if (MAXARG == 1)
      curstash:
	stash = CopSTASH(PL_curcop);
    else {
	SV * const ssv = POPs;
	STRLEN len;
	const char *ptr;

	if (!ssv) goto curstash;
	if (!SvGMAGICAL(ssv) && !SvAMAGIC(ssv) && SvROK(ssv))
	    Perl_croak(aTHX_ "Attempt to bless into a reference");
	ptr = SvPV_const(ssv,len);
	if (len == 0)
	    Perl_ck_warner(aTHX_ packWARN(WARN_MISC),
			   "Explicit blessing to '' (assuming package main)");
	stash = gv_stashpvn(ptr, len, GV_ADD|SvUTF8(ssv));
    }

    (void)sv_bless(TOPs, stash);
    RETURN;
}

PP(pp_gelem)
{
    dVAR; dSP;

    SV *sv = POPs;
    STRLEN len;
    const char * const elem = SvPV_const(sv, len);
    GV * const gv = MUTABLE_GV(POPs);
    SV * tmpRef = NULL;

    sv = NULL;
    if (elem) {
	/* elem will always be NUL terminated.  */
	const char * const second_letter = elem + 1;
	switch (*elem) {
	case 'A':
	    if (len == 5 && strEQ(second_letter, "RRAY"))
		tmpRef = MUTABLE_SV(GvAV(gv));
	    break;
	case 'C':
	    if (len == 4 && strEQ(second_letter, "ODE"))
		tmpRef = MUTABLE_SV(GvCVu(gv));
	    break;
	case 'F':
	    if (len == 10 && strEQ(second_letter, "ILEHANDLE")) {
		/* finally deprecated in 5.8.0 */
		deprecate("*glob{FILEHANDLE}");
		tmpRef = MUTABLE_SV(GvIOp(gv));
	    }
	    else
		if (len == 6 && strEQ(second_letter, "ORMAT"))
		    tmpRef = MUTABLE_SV(GvFORM(gv));
	    break;
	case 'G':
	    if (len == 4 && strEQ(second_letter, "LOB"))
		tmpRef = MUTABLE_SV(gv);
	    break;
	case 'H':
	    if (len == 4 && strEQ(second_letter, "ASH"))
		tmpRef = MUTABLE_SV(GvHV(gv));
	    break;
	case 'I':
	    if (*second_letter == 'O' && !elem[2] && len == 2)
		tmpRef = MUTABLE_SV(GvIOp(gv));
	    break;
	case 'N':
	    if (len == 4 && strEQ(second_letter, "AME"))
		sv = newSVhek(GvNAME_HEK(gv));
	    break;
	case 'P':
	    if (len == 7 && strEQ(second_letter, "ACKAGE")) {
		const HV * const stash = GvSTASH(gv);
		const HEK * const hek = stash ? HvNAME_HEK(stash) : NULL;
		sv = hek ? newSVhek(hek) : newSVpvs("__ANON__");
	    }
	    break;
	case 'S':
	    if (len == 6 && strEQ(second_letter, "CALAR"))
		tmpRef = GvSVn(gv);
	    break;
	}
    }
    if (tmpRef)
	sv = newRV(tmpRef);
    if (sv)
	sv_2mortal(sv);
    else
	sv = &PL_sv_undef;
    XPUSHs(sv);
    RETURN;
}

/* Pattern matching */

PP(pp_study)
{
    dVAR; dSP; dPOPss;
    register unsigned char *s;
    char *sfirst_raw;
    STRLEN len;
    MAGIC *mg = SvMAGICAL(sv) ? mg_find(sv, PERL_MAGIC_study) : NULL;
    U8 quanta;
    STRLEN size;

    if (mg && SvSCREAM(sv))
	RETPUSHYES;

    s = (unsigned char*)(SvPV(sv, len));
    if (len == 0 || len > I32_MAX || !SvPOK(sv) || SvUTF8(sv) || SvVALID(sv)) {
	/* No point in studying a zero length string, and not safe to study
	   anything that doesn't appear to be a simple scalar (and hence might
	   change between now and when the regexp engine runs without our set
	   magic ever running) such as a reference to an object with overloaded
	   stringification.  Also refuse to study an FBM scalar, as this gives
	   more flexibility in SV flag usage.  No real-world code would ever
	   end up studying an FBM scalar, so this isn't a real pessimisation.
	   Endemic use of I32 in Perl_screaminstr makes it hard to safely push
	   the study length limit from I32_MAX to U32_MAX - 1.
	*/
	RETPUSHNO;
    }

    /* Make study a no-op. It's no longer useful and its existence
       complicates matters elsewhere. This is a low-impact band-aid.
       The relevant code will be neatly removed in a future release. */
    RETPUSHYES;

    if (len < 0xFF) {
	quanta = 1;
    } else if (len < 0xFFFF) {
	quanta = 2;
    } else
	quanta = 4;

    size = (256 + len) * quanta;
    sfirst_raw = (char *)safemalloc(size);

    if (!sfirst_raw)
	DIE(aTHX_ "do_study: out of memory");

    SvSCREAM_on(sv);
    if (!mg)
	mg = sv_magicext(sv, NULL, PERL_MAGIC_study, &PL_vtbl_regexp, NULL, 0);
    mg->mg_ptr = sfirst_raw;
    mg->mg_len = size;
    mg->mg_private = quanta;

    memset(sfirst_raw, ~0, 256 * quanta);

    /* The assumption here is that most studied strings are fairly short, hence
       the pain of the extra code is worth it, given the memory savings.
       80 character string, 336 bytes as U8, down from 1344 as U32
       800 character string, 2112 bytes as U16, down from 4224 as U32
    */
       
    if (quanta == 1) {
	U8 *const sfirst = (U8 *)sfirst_raw;
	U8 *const snext = sfirst + 256;
	while (len-- > 0) {
	    const U8 ch = s[len];
	    snext[len] = sfirst[ch];
	    sfirst[ch] = len;
	}
    } else if (quanta == 2) {
	U16 *const sfirst = (U16 *)sfirst_raw;
	U16 *const snext = sfirst + 256;
	while (len-- > 0) {
	    const U8 ch = s[len];
	    snext[len] = sfirst[ch];
	    sfirst[ch] = len;
	}
    } else  {
	U32 *const sfirst = (U32 *)sfirst_raw;
	U32 *const snext = sfirst + 256;
	while (len-- > 0) {
	    const U8 ch = s[len];
	    snext[len] = sfirst[ch];
	    sfirst[ch] = len;
	}
    }

    RETPUSHYES;
}

PP(pp_trans)
{
    dVAR; dSP; dTARG;
    SV *sv;

    if (PL_op->op_flags & OPf_STACKED)
	sv = POPs;
    else if (PL_op->op_private & OPpTARGET_MY)
	sv = GETTARGET;
    else {
	sv = DEFSV;
	EXTEND(SP,1);
    }
    TARG = sv_newmortal();
    if(PL_op->op_type == OP_TRANSR) {
	STRLEN len;
	const char * const pv = SvPV(sv,len);
	SV * const newsv = newSVpvn_flags(pv, len, SVs_TEMP|SvUTF8(sv));
	do_trans(newsv);
	PUSHs(newsv);
    }
    else PUSHi(do_trans(sv));
    RETURN;
}

/* Lvalue operators. */

static void
S_do_chomp(pTHX_ SV *retval, SV *sv, bool chomping)
{
    dVAR;
    STRLEN len;
    char *s;

    PERL_ARGS_ASSERT_DO_CHOMP;

    if (chomping && (RsSNARF(PL_rs) || RsRECORD(PL_rs)))
	return;
    if (SvTYPE(sv) == SVt_PVAV) {
	I32 i;
	AV *const av = MUTABLE_AV(sv);
	const I32 max = AvFILL(av);

	for (i = 0; i <= max; i++) {
	    sv = MUTABLE_SV(av_fetch(av, i, FALSE));
	    if (sv && ((sv = *(SV**)sv), sv != &PL_sv_undef))
		do_chomp(retval, sv, chomping);
	}
        return;
    }
    else if (SvTYPE(sv) == SVt_PVHV) {
	HV* const hv = MUTABLE_HV(sv);
	HE* entry;
        (void)hv_iterinit(hv);
        while ((entry = hv_iternext(hv)))
            do_chomp(retval, hv_iterval(hv,entry), chomping);
	return;
    }
    else if (SvREADONLY(sv)) {
        if (SvFAKE(sv)) {
            /* SV is copy-on-write */
	    sv_force_normal_flags(sv, 0);
        }
        else
            Perl_croak_no_modify(aTHX);
    }

    if (PL_encoding) {
	if (!SvUTF8(sv)) {
	    /* XXX, here sv is utf8-ized as a side-effect!
	       If encoding.pm is used properly, almost string-generating
	       operations, including literal strings, chr(), input data, etc.
	       should have been utf8-ized already, right?
	    */
	    sv_recode_to_utf8(sv, PL_encoding);
	}
    }

    s = SvPV(sv, len);
    if (chomping) {
	char *temp_buffer = NULL;
	SV *svrecode = NULL;

	if (s && len) {
	    s += --len;
	    if (RsPARA(PL_rs)) {
		if (*s != '\n')
		    goto nope;
		++SvIVX(retval);
		while (len && s[-1] == '\n') {
		    --len;
		    --s;
		    ++SvIVX(retval);
		}
	    }
	    else {
		STRLEN rslen, rs_charlen;
		const char *rsptr = SvPV_const(PL_rs, rslen);

		rs_charlen = SvUTF8(PL_rs)
		    ? sv_len_utf8(PL_rs)
		    : rslen;

		if (SvUTF8(PL_rs) != SvUTF8(sv)) {
		    /* Assumption is that rs is shorter than the scalar.  */
		    if (SvUTF8(PL_rs)) {
			/* RS is utf8, scalar is 8 bit.  */
			bool is_utf8 = TRUE;
			temp_buffer = (char*)bytes_from_utf8((U8*)rsptr,
							     &rslen, &is_utf8);
			if (is_utf8) {
			    /* Cannot downgrade, therefore cannot possibly match
			     */
			    assert (temp_buffer == rsptr);
			    temp_buffer = NULL;
			    goto nope;
			}
			rsptr = temp_buffer;
		    }
		    else if (PL_encoding) {
			/* RS is 8 bit, encoding.pm is used.
			 * Do not recode PL_rs as a side-effect. */
			svrecode = newSVpvn(rsptr, rslen);
			sv_recode_to_utf8(svrecode, PL_encoding);
			rsptr = SvPV_const(svrecode, rslen);
			rs_charlen = sv_len_utf8(svrecode);
		    }
		    else {
			/* RS is 8 bit, scalar is utf8.  */
			temp_buffer = (char*)bytes_to_utf8((U8*)rsptr, &rslen);
			rsptr = temp_buffer;
		    }
		}
		if (rslen == 1) {
		    if (*s != *rsptr)
			goto nope;
		    ++SvIVX(retval);
		}
		else {
		    if (len < rslen - 1)
			goto nope;
		    len -= rslen - 1;
		    s -= rslen - 1;
		    if (memNE(s, rsptr, rslen))
			goto nope;
		    SvIVX(retval) += rs_charlen;
		}
	    }
	    s = SvPV_force_nomg_nolen(sv);
	    SvCUR_set(sv, len);
	    *SvEND(sv) = '\0';
	    SvNIOK_off(sv);
	    SvSETMAGIC(sv);
	}
    nope:

	SvREFCNT_dec(svrecode);

	Safefree(temp_buffer);
    } else {
	if (len && !SvPOK(sv))
	    s = SvPV_force_nomg(sv, len);
	if (DO_UTF8(sv)) {
	    if (s && len) {
		char * const send = s + len;
		char * const start = s;
		s = send - 1;
		while (s > start && UTF8_IS_CONTINUATION(*s))
		    s--;
		if (is_utf8_string((U8*)s, send - s)) {
		    sv_setpvn(retval, s, send - s);
		    *s = '\0';
		    SvCUR_set(sv, s - start);
		    SvNIOK_off(sv);
		    SvUTF8_on(retval);
		}
	    }
	    else
		sv_setpvs(retval, "");
	}
	else if (s && len) {
	    s += --len;
	    sv_setpvn(retval, s, 1);
	    *s = '\0';
	    SvCUR_set(sv, len);
	    SvUTF8_off(sv);
	    SvNIOK_off(sv);
	}
	else
	    sv_setpvs(retval, "");
	SvSETMAGIC(sv);
    }
}

PP(pp_schop)
{
    dVAR; dSP; dTARGET;
    const bool chomping = PL_op->op_type == OP_SCHOMP;

    if (chomping)
	sv_setiv(TARG, 0);
    do_chomp(TARG, TOPs, chomping);
    SETTARG;
    RETURN;
}

PP(pp_chop)
{
    dVAR; dSP; dMARK; dTARGET; dORIGMARK;
    const bool chomping = PL_op->op_type == OP_CHOMP;

    if (chomping)
	sv_setiv(TARG, 0);
    while (MARK < SP)
	do_chomp(TARG, *++MARK, chomping);
    SP = ORIGMARK;
    XPUSHTARG;
    RETURN;
}

PP(pp_undef)
{
    dVAR; dSP;
    SV *sv;

    if (!PL_op->op_private) {
	EXTEND(SP, 1);
	RETPUSHUNDEF;
    }

    sv = POPs;
    if (!sv)
	RETPUSHUNDEF;

    SV_CHECK_THINKFIRST_COW_DROP(sv);

    switch (SvTYPE(sv)) {
    case SVt_NULL:
	break;
    case SVt_PVAV:
	av_undef(MUTABLE_AV(sv));
	break;
    case SVt_PVHV:
	hv_undef(MUTABLE_HV(sv));
	break;
    case SVt_PVCV:
	if (cv_const_sv((const CV *)sv))
	    Perl_ck_warner(aTHX_ packWARN(WARN_MISC),
                          "Constant subroutine %"SVf" undefined",
			   SVfARG(CvANON((const CV *)sv)
                             ? newSVpvs_flags("(anonymous)", SVs_TEMP)
                             : sv_2mortal(newSVhek(GvENAME_HEK(CvGV((const CV *)sv))))));
	/* FALLTHROUGH */
    case SVt_PVFM:
	{
	    /* let user-undef'd sub keep its identity */
	    GV* const gv = CvGV((const CV *)sv);
	    cv_undef(MUTABLE_CV(sv));
	    CvGV_set(MUTABLE_CV(sv), gv);
	}
	break;
    case SVt_PVGV:
	if (SvFAKE(sv)) {
	    SvSetMagicSV(sv, &PL_sv_undef);
	    break;
	}
	else if (isGV_with_GP(sv)) {
	    GP *gp;
            HV *stash;

            /* undef *Pkg::meth_name ... */
            bool method_changed
             =   GvCVu((const GV *)sv) && (stash = GvSTASH((const GV *)sv))
	      && HvENAME_get(stash);
            /* undef *Foo:: */
            if((stash = GvHV((const GV *)sv))) {
                if(HvENAME_get(stash))
                    SvREFCNT_inc_simple_void_NN(sv_2mortal((SV *)stash));
                else stash = NULL;
            }

	    gp_free(MUTABLE_GV(sv));
	    Newxz(gp, 1, GP);
	    GvGP_set(sv, gp_ref(gp));
	    GvSV(sv) = newSV(0);
	    GvLINE(sv) = CopLINE(PL_curcop);
	    GvEGV(sv) = MUTABLE_GV(sv);
	    GvMULTI_on(sv);

            if(stash)
                mro_package_moved(NULL, stash, (const GV *)sv, 0);
            stash = NULL;
            /* undef *Foo::ISA */
            if( strEQ(GvNAME((const GV *)sv), "ISA")
             && (stash = GvSTASH((const GV *)sv))
             && (method_changed || HvENAME(stash)) )
                mro_isa_changed_in(stash);
            else if(method_changed)
                mro_method_changed_in(
                 GvSTASH((const GV *)sv)
                );

	    break;
	}
	/* FALL THROUGH */
    default:
	if (SvTYPE(sv) >= SVt_PV && SvPVX_const(sv) && SvLEN(sv)) {
	    SvPV_free(sv);
	    SvPV_set(sv, NULL);
	    SvLEN_set(sv, 0);
	}
	SvOK_off(sv);
	SvSETMAGIC(sv);
    }

    RETPUSHUNDEF;
}

PP(pp_postinc)
{
    dVAR; dSP; dTARGET;
    const bool inc =
	PL_op->op_type == OP_POSTINC || PL_op->op_type == OP_I_POSTINC;
    if (SvTYPE(TOPs) >= SVt_PVAV || (isGV_with_GP(TOPs) && !SvFAKE(TOPs)))
	Perl_croak_no_modify(aTHX);
    if (SvROK(TOPs))
	TARG = sv_newmortal();
    sv_setsv(TARG, TOPs);
    if (!SvREADONLY(TOPs) && SvIOK_notUV(TOPs) && !SvNOK(TOPs) && !SvPOK(TOPs)
        && SvIVX(TOPs) != (inc ? IV_MAX : IV_MIN))
    {
	SvIV_set(TOPs, SvIVX(TOPs) + (inc ? 1 : -1));
	SvFLAGS(TOPs) &= ~(SVp_NOK|SVp_POK);
    }
    else if (inc)
	sv_inc_nomg(TOPs);
    else sv_dec_nomg(TOPs);
    SvSETMAGIC(TOPs);
    /* special case for undef: see thread at 2003-03/msg00536.html in archive */
    if (inc && !SvOK(TARG))
	sv_setiv(TARG, 0);
    SETs(TARG);
    return NORMAL;
}

/* Ordinary operators. */

PP(pp_pow)
{
    dVAR; dSP; dATARGET; SV *svl, *svr;
#ifdef PERL_PRESERVE_IVUV
    bool is_int = 0;
#endif
    tryAMAGICbin_MG(pow_amg, AMGf_assign|AMGf_numeric);
    svr = TOPs;
    svl = TOPm1s;
#ifdef PERL_PRESERVE_IVUV
    /* For integer to integer power, we do the calculation by hand wherever
       we're sure it is safe; otherwise we call pow() and try to convert to
       integer afterwards. */
    {
	SvIV_please_nomg(svr);
	if (SvIOK(svr)) {
	    SvIV_please_nomg(svl);
	    if (SvIOK(svl)) {
		UV power;
		bool baseuok;
		UV baseuv;

		if (SvUOK(svr)) {
		    power = SvUVX(svr);
		} else {
		    const IV iv = SvIVX(svr);
		    if (iv >= 0) {
			power = iv;
		    } else {
			goto float_it; /* Can't do negative powers this way.  */
		    }
		}

		baseuok = SvUOK(svl);
		if (baseuok) {
		    baseuv = SvUVX(svl);
		} else {
		    const IV iv = SvIVX(svl);
		    if (iv >= 0) {
			baseuv = iv;
			baseuok = TRUE; /* effectively it's a UV now */
		    } else {
			baseuv = -iv; /* abs, baseuok == false records sign */
		    }
		}
                /* now we have integer ** positive integer. */
                is_int = 1;

                /* foo & (foo - 1) is zero only for a power of 2.  */
                if (!(baseuv & (baseuv - 1))) {
                    /* We are raising power-of-2 to a positive integer.
                       The logic here will work for any base (even non-integer
                       bases) but it can be less accurate than
                       pow (base,power) or exp (power * log (base)) when the
                       intermediate values start to spill out of the mantissa.
                       With powers of 2 we know this can't happen.
                       And powers of 2 are the favourite thing for perl
                       programmers to notice ** not doing what they mean. */
                    NV result = 1.0;
                    NV base = baseuok ? baseuv : -(NV)baseuv;

		    if (power & 1) {
			result *= base;
		    }
		    while (power >>= 1) {
			base *= base;
			if (power & 1) {
			    result *= base;
			}
		    }
                    SP--;
                    SETn( result );
                    SvIV_please_nomg(svr);
                    RETURN;
		} else {
		    register unsigned int highbit = 8 * sizeof(UV);
		    register unsigned int diff = 8 * sizeof(UV);
		    while (diff >>= 1) {
			highbit -= diff;
			if (baseuv >> highbit) {
			    highbit += diff;
			}
		    }
		    /* we now have baseuv < 2 ** highbit */
		    if (power * highbit <= 8 * sizeof(UV)) {
			/* result will definitely fit in UV, so use UV math
			   on same algorithm as above */
			register UV result = 1;
			register UV base = baseuv;
			const bool odd_power = cBOOL(power & 1);
			if (odd_power) {
			    result *= base;
			}
			while (power >>= 1) {
			    base *= base;
			    if (power & 1) {
				result *= base;
			    }
			}
			SP--;
			if (baseuok || !odd_power)
			    /* answer is positive */
			    SETu( result );
			else if (result <= (UV)IV_MAX)
			    /* answer negative, fits in IV */
			    SETi( -(IV)result );
			else if (result == (UV)IV_MIN) 
			    /* 2's complement assumption: special case IV_MIN */
			    SETi( IV_MIN );
			else
			    /* answer negative, doesn't fit */
			    SETn( -(NV)result );
			RETURN;
		    } 
		}
	    }
	}
    }
  float_it:
#endif    
    {
	NV right = SvNV_nomg(svr);
	NV left  = SvNV_nomg(svl);
	(void)POPs;

#if defined(USE_LONG_DOUBLE) && defined(HAS_AIX_POWL_NEG_BASE_BUG)
    /*
    We are building perl with long double support and are on an AIX OS
    afflicted with a powl() function that wrongly returns NaNQ for any
    negative base.  This was reported to IBM as PMR #23047-379 on
    03/06/2006.  The problem exists in at least the following versions
    of AIX and the libm fileset, and no doubt others as well:

	AIX 4.3.3-ML10      bos.adt.libm 4.3.3.50
	AIX 5.1.0-ML04      bos.adt.libm 5.1.0.29
	AIX 5.2.0           bos.adt.libm 5.2.0.85

    So, until IBM fixes powl(), we provide the following workaround to
    handle the problem ourselves.  Our logic is as follows: for
    negative bases (left), we use fmod(right, 2) to check if the
    exponent is an odd or even integer:

	- if odd,  powl(left, right) == -powl(-left, right)
	- if even, powl(left, right) ==  powl(-left, right)

    If the exponent is not an integer, the result is rightly NaNQ, so
    we just return that (as NV_NAN).
    */

	if (left < 0.0) {
	    NV mod2 = Perl_fmod( right, 2.0 );
	    if (mod2 == 1.0 || mod2 == -1.0) {	/* odd integer */
		SETn( -Perl_pow( -left, right) );
	    } else if (mod2 == 0.0) {		/* even integer */
		SETn( Perl_pow( -left, right) );
	    } else {				/* fractional power */
		SETn( NV_NAN );
	    }
	} else {
	    SETn( Perl_pow( left, right) );
	}
#else
	SETn( Perl_pow( left, right) );
#endif  /* HAS_AIX_POWL_NEG_BASE_BUG */

#ifdef PERL_PRESERVE_IVUV
	if (is_int)
	    SvIV_please_nomg(svr);
#endif
	RETURN;
    }
}

PP(pp_multiply)
{
    dVAR; dSP; dATARGET; SV *svl, *svr;
    tryAMAGICbin_MG(mult_amg, AMGf_assign|AMGf_numeric);
    svr = TOPs;
    svl = TOPm1s;
#ifdef PERL_PRESERVE_IVUV
    SvIV_please_nomg(svr);
    if (SvIOK(svr)) {
	/* Unless the left argument is integer in range we are going to have to
	   use NV maths. Hence only attempt to coerce the right argument if
	   we know the left is integer.  */
	/* Left operand is defined, so is it IV? */
	SvIV_please_nomg(svl);
	if (SvIOK(svl)) {
	    bool auvok = SvUOK(svl);
	    bool buvok = SvUOK(svr);
	    const UV topmask = (~ (UV)0) << (4 * sizeof (UV));
	    const UV botmask = ~((~ (UV)0) << (4 * sizeof (UV)));
	    UV alow;
	    UV ahigh;
	    UV blow;
	    UV bhigh;

	    if (auvok) {
		alow = SvUVX(svl);
	    } else {
		const IV aiv = SvIVX(svl);
		if (aiv >= 0) {
		    alow = aiv;
		    auvok = TRUE; /* effectively it's a UV now */
		} else {
		    alow = -aiv; /* abs, auvok == false records sign */
		}
	    }
	    if (buvok) {
		blow = SvUVX(svr);
	    } else {
		const IV biv = SvIVX(svr);
		if (biv >= 0) {
		    blow = biv;
		    buvok = TRUE; /* effectively it's a UV now */
		} else {
		    blow = -biv; /* abs, buvok == false records sign */
		}
	    }

	    /* If this does sign extension on unsigned it's time for plan B  */
	    ahigh = alow >> (4 * sizeof (UV));
	    alow &= botmask;
	    bhigh = blow >> (4 * sizeof (UV));
	    blow &= botmask;
	    if (ahigh && bhigh) {
		NOOP;
		/* eg 32 bit is at least 0x10000 * 0x10000 == 0x100000000
		   which is overflow. Drop to NVs below.  */
	    } else if (!ahigh && !bhigh) {
		/* eg 32 bit is at most 0xFFFF * 0xFFFF == 0xFFFE0001
		   so the unsigned multiply cannot overflow.  */
		const UV product = alow * blow;
		if (auvok == buvok) {
		    /* -ve * -ve or +ve * +ve gives a +ve result.  */
		    SP--;
		    SETu( product );
		    RETURN;
		} else if (product <= (UV)IV_MIN) {
		    /* 2s complement assumption that (UV)-IV_MIN is correct.  */
		    /* -ve result, which could overflow an IV  */
		    SP--;
		    SETi( -(IV)product );
		    RETURN;
		} /* else drop to NVs below. */
	    } else {
		/* One operand is large, 1 small */
		UV product_middle;
		if (bhigh) {
		    /* swap the operands */
		    ahigh = bhigh;
		    bhigh = blow; /* bhigh now the temp var for the swap */
		    blow = alow;
		    alow = bhigh;
		}
		/* now, ((ahigh * blow) << half_UV_len) + (alow * blow)
		   multiplies can't overflow. shift can, add can, -ve can.  */
		product_middle = ahigh * blow;
		if (!(product_middle & topmask)) {
		    /* OK, (ahigh * blow) won't lose bits when we shift it.  */
		    UV product_low;
		    product_middle <<= (4 * sizeof (UV));
		    product_low = alow * blow;

		    /* as for pp_add, UV + something mustn't get smaller.
		       IIRC ANSI mandates this wrapping *behaviour* for
		       unsigned whatever the actual representation*/
		    product_low += product_middle;
		    if (product_low >= product_middle) {
			/* didn't overflow */
			if (auvok == buvok) {
			    /* -ve * -ve or +ve * +ve gives a +ve result.  */
			    SP--;
			    SETu( product_low );
			    RETURN;
			} else if (product_low <= (UV)IV_MIN) {
			    /* 2s complement assumption again  */
			    /* -ve result, which could overflow an IV  */
			    SP--;
			    SETi( -(IV)product_low );
			    RETURN;
			} /* else drop to NVs below. */
		    }
		} /* product_middle too large */
	    } /* ahigh && bhigh */
	} /* SvIOK(svl) */
    } /* SvIOK(svr) */
#endif
    {
      NV right = SvNV_nomg(svr);
      NV left  = SvNV_nomg(svl);
      (void)POPs;
      SETn( left * right );
      RETURN;
    }
}

PP(pp_divide)
{
    dVAR; dSP; dATARGET; SV *svl, *svr;
    tryAMAGICbin_MG(div_amg, AMGf_assign|AMGf_numeric);
    svr = TOPs;
    svl = TOPm1s;
    /* Only try to do UV divide first
       if ((SLOPPYDIVIDE is true) or
           (PERL_PRESERVE_IVUV is true and one or both SV is a UV too large
            to preserve))
       The assumption is that it is better to use floating point divide
       whenever possible, only doing integer divide first if we can't be sure.
       If NV_PRESERVES_UV is true then we know at compile time that no UV
       can be too large to preserve, so don't need to compile the code to
       test the size of UVs.  */

#ifdef SLOPPYDIVIDE
#  define PERL_TRY_UV_DIVIDE
    /* ensure that 20./5. == 4. */
#else
#  ifdef PERL_PRESERVE_IVUV
#    ifndef NV_PRESERVES_UV
#      define PERL_TRY_UV_DIVIDE
#    endif
#  endif
#endif

#ifdef PERL_TRY_UV_DIVIDE
    SvIV_please_nomg(svr);
    if (SvIOK(svr)) {
        SvIV_please_nomg(svl);
        if (SvIOK(svl)) {
            bool left_non_neg = SvUOK(svl);
            bool right_non_neg = SvUOK(svr);
            UV left;
            UV right;

            if (right_non_neg) {
                right = SvUVX(svr);
            }
	    else {
		const IV biv = SvIVX(svr);
                if (biv >= 0) {
                    right = biv;
                    right_non_neg = TRUE; /* effectively it's a UV now */
                }
		else {
                    right = -biv;
                }
            }
            /* historically undef()/0 gives a "Use of uninitialized value"
               warning before dieing, hence this test goes here.
               If it were immediately before the second SvIV_please, then
               DIE() would be invoked before left was even inspected, so
               no inspection would give no warning.  */
            if (right == 0)
                DIE(aTHX_ "Illegal division by zero");

            if (left_non_neg) {
                left = SvUVX(svl);
            }
	    else {
		const IV aiv = SvIVX(svl);
                if (aiv >= 0) {
                    left = aiv;
                    left_non_neg = TRUE; /* effectively it's a UV now */
                }
		else {
                    left = -aiv;
                }
            }

            if (left >= right
#ifdef SLOPPYDIVIDE
                /* For sloppy divide we always attempt integer division.  */
#else
                /* Otherwise we only attempt it if either or both operands
                   would not be preserved by an NV.  If both fit in NVs
                   we fall through to the NV divide code below.  However,
                   as left >= right to ensure integer result here, we know that
                   we can skip the test on the right operand - right big
                   enough not to be preserved can't get here unless left is
                   also too big.  */

                && (left > ((UV)1 << NV_PRESERVES_UV_BITS))
#endif
                ) {
                /* Integer division can't overflow, but it can be imprecise.  */
		const UV result = left / right;
                if (result * right == left) {
                    SP--; /* result is valid */
                    if (left_non_neg == right_non_neg) {
                        /* signs identical, result is positive.  */
                        SETu( result );
                        RETURN;
                    }
                    /* 2s complement assumption */
                    if (result <= (UV)IV_MIN)
                        SETi( -(IV)result );
                    else {
                        /* It's exact but too negative for IV. */
                        SETn( -(NV)result );
                    }
                    RETURN;
                } /* tried integer divide but it was not an integer result */
            } /* else (PERL_ABS(result) < 1.0) or (both UVs in range for NV) */
        } /* left wasn't SvIOK */
    } /* right wasn't SvIOK */
#endif /* PERL_TRY_UV_DIVIDE */
    {
	NV right = SvNV_nomg(svr);
	NV left  = SvNV_nomg(svl);
	(void)POPs;(void)POPs;
#if defined(NAN_COMPARE_BROKEN) && defined(Perl_isnan)
	if (! Perl_isnan(right) && right == 0.0)
#else
	if (right == 0.0)
#endif
	    DIE(aTHX_ "Illegal division by zero");
	PUSHn( left / right );
	RETURN;
    }
}

PP(pp_modulo)
{
    dVAR; dSP; dATARGET;
    tryAMAGICbin_MG(modulo_amg, AMGf_assign|AMGf_numeric);
    {
	UV left  = 0;
	UV right = 0;
	bool left_neg = FALSE;
	bool right_neg = FALSE;
	bool use_double = FALSE;
	bool dright_valid = FALSE;
	NV dright = 0.0;
	NV dleft  = 0.0;
	SV * const svr = TOPs;
	SV * const svl = TOPm1s;
	SvIV_please_nomg(svr);
        if (SvIOK(svr)) {
            right_neg = !SvUOK(svr);
            if (!right_neg) {
                right = SvUVX(svr);
            } else {
		const IV biv = SvIVX(svr);
                if (biv >= 0) {
                    right = biv;
                    right_neg = FALSE; /* effectively it's a UV now */
                } else {
                    right = -biv;
                }
            }
        }
        else {
	    dright = SvNV_nomg(svr);
	    right_neg = dright < 0;
	    if (right_neg)
		dright = -dright;
            if (dright < UV_MAX_P1) {
                right = U_V(dright);
                dright_valid = TRUE; /* In case we need to use double below.  */
            } else {
                use_double = TRUE;
            }
	}

        /* At this point use_double is only true if right is out of range for
           a UV.  In range NV has been rounded down to nearest UV and
           use_double false.  */
        SvIV_please_nomg(svl);
	if (!use_double && SvIOK(svl)) {
            if (SvIOK(svl)) {
                left_neg = !SvUOK(svl);
                if (!left_neg) {
                    left = SvUVX(svl);
                } else {
		    const IV aiv = SvIVX(svl);
                    if (aiv >= 0) {
                        left = aiv;
                        left_neg = FALSE; /* effectively it's a UV now */
                    } else {
                        left = -aiv;
                    }
                }
            }
        }
	else {
	    dleft = SvNV_nomg(svl);
	    left_neg = dleft < 0;
	    if (left_neg)
		dleft = -dleft;

            /* This should be exactly the 5.6 behaviour - if left and right are
               both in range for UV then use U_V() rather than floor.  */
	    if (!use_double) {
                if (dleft < UV_MAX_P1) {
                    /* right was in range, so is dleft, so use UVs not double.
                     */
                    left = U_V(dleft);
                }
                /* left is out of range for UV, right was in range, so promote
                   right (back) to double.  */
                else {
                    /* The +0.5 is used in 5.6 even though it is not strictly
                       consistent with the implicit +0 floor in the U_V()
                       inside the #if 1. */
                    dleft = Perl_floor(dleft + 0.5);
                    use_double = TRUE;
                    if (dright_valid)
                        dright = Perl_floor(dright + 0.5);
                    else
                        dright = right;
                }
            }
        }
	sp -= 2;
	if (use_double) {
	    NV dans;

	    if (!dright)
		DIE(aTHX_ "Illegal modulus zero");

	    dans = Perl_fmod(dleft, dright);
	    if ((left_neg != right_neg) && dans)
		dans = dright - dans;
	    if (right_neg)
		dans = -dans;
	    sv_setnv(TARG, dans);
	}
	else {
	    UV ans;

	    if (!right)
		DIE(aTHX_ "Illegal modulus zero");

	    ans = left % right;
	    if ((left_neg != right_neg) && ans)
		ans = right - ans;
	    if (right_neg) {
		/* XXX may warn: unary minus operator applied to unsigned type */
		/* could change -foo to be (~foo)+1 instead	*/
		if (ans <= ~((UV)IV_MAX)+1)
		    sv_setiv(TARG, ~ans+1);
		else
		    sv_setnv(TARG, -(NV)ans);
	    }
	    else
		sv_setuv(TARG, ans);
	}
	PUSHTARG;
	RETURN;
    }
}

PP(pp_repeat)
{
    dVAR; dSP; dATARGET;
    register IV count;
    SV *sv;

    if (GIMME == G_ARRAY && PL_op->op_private & OPpREPEAT_DOLIST) {
	/* TODO: think of some way of doing list-repeat overloading ??? */
	sv = POPs;
	SvGETMAGIC(sv);
    }
    else {
	tryAMAGICbin_MG(repeat_amg, AMGf_assign);
	sv = POPs;
    }

    if (SvIOKp(sv)) {
	 if (SvUOK(sv)) {
	      const UV uv = SvUV_nomg(sv);
	      if (uv > IV_MAX)
		   count = IV_MAX; /* The best we can do? */
	      else
		   count = uv;
	 } else {
	      const IV iv = SvIV_nomg(sv);
	      if (iv < 0)
		   count = 0;
	      else
		   count = iv;
	 }
    }
    else if (SvNOKp(sv)) {
	 const NV nv = SvNV_nomg(sv);
	 if (nv < 0.0)
	      count = 0;
	 else
	      count = (IV)nv;
    }
    else
	 count = SvIV_nomg(sv);

    if (GIMME == G_ARRAY && PL_op->op_private & OPpREPEAT_DOLIST) {
	dMARK;
	static const char oom_list_extend[] = "Out of memory during list extend";
	const I32 items = SP - MARK;
	const I32 max = items * count;

	MEM_WRAP_CHECK_1(max, SV*, oom_list_extend);
	/* Did the max computation overflow? */
	if (items > 0 && max > 0 && (max < items || max < count))
	   Perl_croak(aTHX_ oom_list_extend);
	MEXTEND(MARK, max);
	if (count > 1) {
	    while (SP > MARK) {
#if 0
	      /* This code was intended to fix 20010809.028:

	         $x = 'abcd';
		 for (($x =~ /./g) x 2) {
		     print chop; # "abcdabcd" expected as output.
		 }

	       * but that change (#11635) broke this code:

	       $x = [("foo")x2]; # only one "foo" ended up in the anonlist.

	       * I can't think of a better fix that doesn't introduce
	       * an efficiency hit by copying the SVs. The stack isn't
	       * refcounted, and mortalisation obviously doesn't
	       * Do The Right Thing when the stack has more than
	       * one pointer to the same mortal value.
	       * .robin.
	       */
		if (*SP) {
		    *SP = sv_2mortal(newSVsv(*SP));
		    SvREADONLY_on(*SP);
		}
#else
               if (*SP)
		   SvTEMP_off((*SP));
#endif
		SP--;
	    }
	    MARK++;
	    repeatcpy((char*)(MARK + items), (char*)MARK,
		items * sizeof(const SV *), count - 1);
	    SP += max;
	}
	else if (count <= 0)
	    SP -= items;
    }
    else {	/* Note: mark already snarfed by pp_list */
	SV * const tmpstr = POPs;
	STRLEN len;
	bool isutf;
	static const char oom_string_extend[] =
	  "Out of memory during string extend";

	if (TARG != tmpstr)
	    sv_setsv_nomg(TARG, tmpstr);
	SvPV_force_nomg(TARG, len);
	isutf = DO_UTF8(TARG);
	if (count != 1) {
	    if (count < 1)
		SvCUR_set(TARG, 0);
	    else {
		const STRLEN max = (UV)count * len;
		if (len > MEM_SIZE_MAX / count)
		     Perl_croak(aTHX_ oom_string_extend);
	        MEM_WRAP_CHECK_1(max, char, oom_string_extend);
		SvGROW(TARG, max + 1);
		repeatcpy(SvPVX(TARG) + len, SvPVX(TARG), len, count - 1);
		SvCUR_set(TARG, SvCUR(TARG) * count);
	    }
	    *SvEND(TARG) = '\0';
	}
	if (isutf)
	    (void)SvPOK_only_UTF8(TARG);
	else
	    (void)SvPOK_only(TARG);

	if (PL_op->op_private & OPpREPEAT_DOLIST) {
	    /* The parser saw this as a list repeat, and there
	       are probably several items on the stack. But we're
	       in scalar context, and there's no pp_list to save us
	       now. So drop the rest of the items -- robin@kitsite.com
	     */
	    dMARK;
	    SP = MARK;
	}
	PUSHTARG;
    }
    RETURN;
}

PP(pp_subtract)
{
    dVAR; dSP; dATARGET; bool useleft; SV *svl, *svr;
    tryAMAGICbin_MG(subtr_amg, AMGf_assign|AMGf_numeric);
    svr = TOPs;
    svl = TOPm1s;
    useleft = USE_LEFT(svl);
#ifdef PERL_PRESERVE_IVUV
    /* See comments in pp_add (in pp_hot.c) about Overflow, and how
       "bad things" happen if you rely on signed integers wrapping.  */
    SvIV_please_nomg(svr);
    if (SvIOK(svr)) {
	/* Unless the left argument is integer in range we are going to have to
	   use NV maths. Hence only attempt to coerce the right argument if
	   we know the left is integer.  */
	register UV auv = 0;
	bool auvok = FALSE;
	bool a_valid = 0;

	if (!useleft) {
	    auv = 0;
	    a_valid = auvok = 1;
	    /* left operand is undef, treat as zero.  */
	} else {
	    /* Left operand is defined, so is it IV? */
	    SvIV_please_nomg(svl);
	    if (SvIOK(svl)) {
		if ((auvok = SvUOK(svl)))
		    auv = SvUVX(svl);
		else {
		    register const IV aiv = SvIVX(svl);
		    if (aiv >= 0) {
			auv = aiv;
			auvok = 1;	/* Now acting as a sign flag.  */
		    } else { /* 2s complement assumption for IV_MIN */
			auv = (UV)-aiv;
		    }
		}
		a_valid = 1;
	    }
	}
	if (a_valid) {
	    bool result_good = 0;
	    UV result;
	    register UV buv;
	    bool buvok = SvUOK(svr);
	
	    if (buvok)
		buv = SvUVX(svr);
	    else {
		register const IV biv = SvIVX(svr);
		if (biv >= 0) {
		    buv = biv;
		    buvok = 1;
		} else
		    buv = (UV)-biv;
	    }
	    /* ?uvok if value is >= 0. basically, flagged as UV if it's +ve,
	       else "IV" now, independent of how it came in.
	       if a, b represents positive, A, B negative, a maps to -A etc
	       a - b =>  (a - b)
	       A - b => -(a + b)
	       a - B =>  (a + b)
	       A - B => -(a - b)
	       all UV maths. negate result if A negative.
	       subtract if signs same, add if signs differ. */

	    if (auvok ^ buvok) {
		/* Signs differ.  */
		result = auv + buv;
		if (result >= auv)
		    result_good = 1;
	    } else {
		/* Signs same */
		if (auv >= buv) {
		    result = auv - buv;
		    /* Must get smaller */
		    if (result <= auv)
			result_good = 1;
		} else {
		    result = buv - auv;
		    if (result <= buv) {
			/* result really should be -(auv-buv). as its negation
			   of true value, need to swap our result flag  */
			auvok = !auvok;
			result_good = 1;
		    }
		}
	    }
	    if (result_good) {
		SP--;
		if (auvok)
		    SETu( result );
		else {
		    /* Negate result */
		    if (result <= (UV)IV_MIN)
			SETi( -(IV)result );
		    else {
			/* result valid, but out of range for IV.  */
			SETn( -(NV)result );
		    }
		}
		RETURN;
	    } /* Overflow, drop through to NVs.  */
	}
    }
#endif
    {
	NV value = SvNV_nomg(svr);
	(void)POPs;

	if (!useleft) {
	    /* left operand is undef, treat as zero - value */
	    SETn(-value);
	    RETURN;
	}
	SETn( SvNV_nomg(svl) - value );
	RETURN;
    }
}

PP(pp_left_shift)
{
    dVAR; dSP; dATARGET; SV *svl, *svr;
    tryAMAGICbin_MG(lshift_amg, AMGf_assign|AMGf_numeric);
    svr = POPs;
    svl = TOPs;
    {
      const IV shift = SvIV_nomg(svr);
      if (PL_op->op_private & HINT_INTEGER) {
	const IV i = SvIV_nomg(svl);
	SETi(i << shift);
      }
      else {
	const UV u = SvUV_nomg(svl);
	SETu(u << shift);
      }
      RETURN;
    }
}

PP(pp_right_shift)
{
    dVAR; dSP; dATARGET; SV *svl, *svr;
    tryAMAGICbin_MG(rshift_amg, AMGf_assign|AMGf_numeric);
    svr = POPs;
    svl = TOPs;
    {
      const IV shift = SvIV_nomg(svr);
      if (PL_op->op_private & HINT_INTEGER) {
	const IV i = SvIV_nomg(svl);
	SETi(i >> shift);
      }
      else {
	const UV u = SvUV_nomg(svl);
	SETu(u >> shift);
      }
      RETURN;
    }
}

PP(pp_lt)
{
    dVAR; dSP;
    SV *left, *right;

    tryAMAGICbin_MG(lt_amg, AMGf_set|AMGf_numeric);
    right = POPs;
    left  = TOPs;
    SETs(boolSV(
	(SvIOK_notUV(left) && SvIOK_notUV(right))
	? (SvIVX(left) < SvIVX(right))
	: (do_ncmp(left, right) == -1)
    ));
    RETURN;
}

PP(pp_gt)
{
    dVAR; dSP;
    SV *left, *right;

    tryAMAGICbin_MG(gt_amg, AMGf_set|AMGf_numeric);
    right = POPs;
    left  = TOPs;
    SETs(boolSV(
	(SvIOK_notUV(left) && SvIOK_notUV(right))
	? (SvIVX(left) > SvIVX(right))
	: (do_ncmp(left, right) == 1)
    ));
    RETURN;
}

PP(pp_le)
{
    dVAR; dSP;
    SV *left, *right;

    tryAMAGICbin_MG(le_amg, AMGf_set|AMGf_numeric);
    right = POPs;
    left  = TOPs;
    SETs(boolSV(
	(SvIOK_notUV(left) && SvIOK_notUV(right))
	? (SvIVX(left) <= SvIVX(right))
	: (do_ncmp(left, right) <= 0)
    ));
    RETURN;
}

PP(pp_ge)
{
    dVAR; dSP;
    SV *left, *right;

    tryAMAGICbin_MG(ge_amg, AMGf_set|AMGf_numeric);
    right = POPs;
    left  = TOPs;
    SETs(boolSV(
	(SvIOK_notUV(left) && SvIOK_notUV(right))
	? (SvIVX(left) >= SvIVX(right))
	: ( (do_ncmp(left, right) & 2) == 0)
    ));
    RETURN;
}

PP(pp_ne)
{
    dVAR; dSP;
    SV *left, *right;

    tryAMAGICbin_MG(ne_amg, AMGf_set|AMGf_numeric);
    right = POPs;
    left  = TOPs;
    SETs(boolSV(
	(SvIOK_notUV(left) && SvIOK_notUV(right))
	? (SvIVX(left) != SvIVX(right))
	: (do_ncmp(left, right) != 0)
    ));
    RETURN;
}

/* compare left and right SVs. Returns:
 * -1: <
 *  0: ==
 *  1: >
 *  2: left or right was a NaN
 */
I32
Perl_do_ncmp(pTHX_ SV* const left, SV * const right)
{
    dVAR;

    PERL_ARGS_ASSERT_DO_NCMP;
#ifdef PERL_PRESERVE_IVUV
    SvIV_please_nomg(right);
    /* Fortunately it seems NaN isn't IOK */
    if (SvIOK(right)) {
	SvIV_please_nomg(left);
	if (SvIOK(left)) {
	    if (!SvUOK(left)) {
		const IV leftiv = SvIVX(left);
		if (!SvUOK(right)) {
		    /* ## IV <=> IV ## */
		    const IV rightiv = SvIVX(right);
		    return (leftiv > rightiv) - (leftiv < rightiv);
		}
		/* ## IV <=> UV ## */
		if (leftiv < 0)
		    /* As (b) is a UV, it's >=0, so it must be < */
		    return -1;
		{
		    const UV rightuv = SvUVX(right);
		    return ((UV)leftiv > rightuv) - ((UV)leftiv < rightuv);
		}
	    }

	    if (SvUOK(right)) {
		/* ## UV <=> UV ## */
		const UV leftuv = SvUVX(left);
		const UV rightuv = SvUVX(right);
		return (leftuv > rightuv) - (leftuv < rightuv);
	    }
	    /* ## UV <=> IV ## */
	    {
		const IV rightiv = SvIVX(right);
		if (rightiv < 0)
		    /* As (a) is a UV, it's >=0, so it cannot be < */
		    return 1;
		{
		    const UV leftuv = SvUVX(left);
		    return (leftuv > (UV)rightiv) - (leftuv < (UV)rightiv);
		}
	    }
	    /* NOTREACHED */
	}
    }
#endif
    {
      NV const rnv = SvNV_nomg(right);
      NV const lnv = SvNV_nomg(left);

#if defined(NAN_COMPARE_BROKEN) && defined(Perl_isnan)
      if (Perl_isnan(lnv) || Perl_isnan(rnv)) {
	  return 2;
       }
      return (lnv > rnv) - (lnv < rnv);
#else
      if (lnv < rnv)
	return -1;
      if (lnv > rnv)
	return 1;
      if (lnv == rnv)
	return 0;
      return 2;
#endif
    }
}


PP(pp_ncmp)
{
    dVAR; dSP;
    SV *left, *right;
    I32 value;
    tryAMAGICbin_MG(ncmp_amg, AMGf_numeric);
    right = POPs;
    left  = TOPs;
    value = do_ncmp(left, right);
    if (value == 2) {
	SETs(&PL_sv_undef);
    }
    else {
	dTARGET;
	SETi(value);
    }
    RETURN;
}

PP(pp_sle)
{
    dVAR; dSP;

    int amg_type = sle_amg;
    int multiplier = 1;
    int rhs = 1;

    switch (PL_op->op_type) {
    case OP_SLT:
	amg_type = slt_amg;
	/* cmp < 0 */
	rhs = 0;
	break;
    case OP_SGT:
	amg_type = sgt_amg;
	/* cmp > 0 */
	multiplier = -1;
	rhs = 0;
	break;
    case OP_SGE:
	amg_type = sge_amg;
	/* cmp >= 0 */
	multiplier = -1;
	break;
    }

    tryAMAGICbin_MG(amg_type, AMGf_set);
    {
      dPOPTOPssrl;
      const int cmp = (IN_LOCALE_RUNTIME
		 ? sv_cmp_locale_flags(left, right, 0)
		 : sv_cmp_flags(left, right, 0));
      SETs(boolSV(cmp * multiplier < rhs));
      RETURN;
    }
}

PP(pp_seq)
{
    dVAR; dSP;
    tryAMAGICbin_MG(seq_amg, AMGf_set);
    {
      dPOPTOPssrl;
      SETs(boolSV(sv_eq_flags(left, right, 0)));
      RETURN;
    }
}

PP(pp_sne)
{
    dVAR; dSP;
    tryAMAGICbin_MG(sne_amg, AMGf_set);
    {
      dPOPTOPssrl;
      SETs(boolSV(!sv_eq_flags(left, right, 0)));
      RETURN;
    }
}

PP(pp_scmp)
{
    dVAR; dSP; dTARGET;
    tryAMAGICbin_MG(scmp_amg, 0);
    {
      dPOPTOPssrl;
      const int cmp = (IN_LOCALE_RUNTIME
		 ? sv_cmp_locale_flags(left, right, 0)
		 : sv_cmp_flags(left, right, 0));
      SETi( cmp );
      RETURN;
    }
}

PP(pp_bit_and)
{
    dVAR; dSP; dATARGET;
    tryAMAGICbin_MG(band_amg, AMGf_assign);
    {
      dPOPTOPssrl;
      if (SvNIOKp(left) || SvNIOKp(right)) {
	const bool left_ro_nonnum  = !SvNIOKp(left) && SvREADONLY(left);
	const bool right_ro_nonnum = !SvNIOKp(right) && SvREADONLY(right);
	if (PL_op->op_private & HINT_INTEGER) {
	  const IV i = SvIV_nomg(left) & SvIV_nomg(right);
	  SETi(i);
	}
	else {
	  const UV u = SvUV_nomg(left) & SvUV_nomg(right);
	  SETu(u);
	}
	if (left_ro_nonnum && left != TARG) SvNIOK_off(left);
	if (right_ro_nonnum) SvNIOK_off(right);
      }
      else {
	do_vop(PL_op->op_type, TARG, left, right);
	SETTARG;
      }
      RETURN;
    }
}

PP(pp_bit_or)
{
    dVAR; dSP; dATARGET;
    const int op_type = PL_op->op_type;

    tryAMAGICbin_MG((op_type == OP_BIT_OR ? bor_amg : bxor_amg), AMGf_assign);
    {
      dPOPTOPssrl;
      if (SvNIOKp(left) || SvNIOKp(right)) {
	const bool left_ro_nonnum  = !SvNIOKp(left) && SvREADONLY(left);
	const bool right_ro_nonnum = !SvNIOKp(right) && SvREADONLY(right);
	if (PL_op->op_private & HINT_INTEGER) {
	  const IV l = (USE_LEFT(left) ? SvIV_nomg(left) : 0);
	  const IV r = SvIV_nomg(right);
	  const IV result = op_type == OP_BIT_OR ? (l | r) : (l ^ r);
	  SETi(result);
	}
	else {
	  const UV l = (USE_LEFT(left) ? SvUV_nomg(left) : 0);
	  const UV r = SvUV_nomg(right);
	  const UV result = op_type == OP_BIT_OR ? (l | r) : (l ^ r);
	  SETu(result);
	}
	if (left_ro_nonnum && left != TARG) SvNIOK_off(left);
	if (right_ro_nonnum) SvNIOK_off(right);
      }
      else {
	do_vop(op_type, TARG, left, right);
	SETTARG;
      }
      RETURN;
    }
}

PP(pp_negate)
{
    dVAR; dSP; dTARGET;
    tryAMAGICun_MG(neg_amg, AMGf_numeric);
    {
	SV * const sv = TOPs;
	const int flags = SvFLAGS(sv);

        if( !SvNIOK( sv ) && looks_like_number( sv ) ){
           SvIV_please( sv );
        }   

	if ((flags & SVf_IOK) || ((flags & (SVp_IOK | SVp_NOK)) == SVp_IOK)) {
	    /* It's publicly an integer, or privately an integer-not-float */
	oops_its_an_int:
	    if (SvIsUV(sv)) {
		if (SvIVX(sv) == IV_MIN) {
		    /* 2s complement assumption. */
		    SETi(SvIVX(sv));	/* special case: -((UV)IV_MAX+1) == IV_MIN */
		    RETURN;
		}
		else if (SvUVX(sv) <= IV_MAX) {
		    SETi(-SvIVX(sv));
		    RETURN;
		}
	    }
	    else if (SvIVX(sv) != IV_MIN) {
		SETi(-SvIVX(sv));
		RETURN;
	    }
#ifdef PERL_PRESERVE_IVUV
	    else {
		SETu((UV)IV_MIN);
		RETURN;
	    }
#endif
	}
	if (SvNIOKp(sv))
	    SETn(-SvNV_nomg(sv));
	else if (SvPOKp(sv)) {
	    STRLEN len;
	    const char * const s = SvPV_nomg_const(sv, len);
	    if (isIDFIRST(*s)) {
		sv_setpvs(TARG, "-");
		sv_catsv(TARG, sv);
	    }
	    else if (*s == '+' || *s == '-') {
		sv_setsv_nomg(TARG, sv);
		*SvPV_force_nomg(TARG, len) = *s == '-' ? '+' : '-';
	    }
	    else if (DO_UTF8(sv)) {
		SvIV_please_nomg(sv);
		if (SvIOK(sv))
		    goto oops_its_an_int;
		if (SvNOK(sv))
		    sv_setnv(TARG, -SvNV_nomg(sv));
		else {
		    sv_setpvs(TARG, "-");
		    sv_catsv(TARG, sv);
		}
	    }
	    else {
		SvIV_please_nomg(sv);
		if (SvIOK(sv))
		  goto oops_its_an_int;
		sv_setnv(TARG, -SvNV_nomg(sv));
	    }
	    SETTARG;
	}
	else
	    SETn(-SvNV_nomg(sv));
    }
    RETURN;
}

PP(pp_not)
{
    dVAR; dSP;
    tryAMAGICun_MG(not_amg, AMGf_set);
    *PL_stack_sp = boolSV(!SvTRUE_nomg(*PL_stack_sp));
    return NORMAL;
}

PP(pp_complement)
{
    dVAR; dSP; dTARGET;
    tryAMAGICun_MG(compl_amg, AMGf_numeric);
    {
      dTOPss;
      if (SvNIOKp(sv)) {
	if (PL_op->op_private & HINT_INTEGER) {
	  const IV i = ~SvIV_nomg(sv);
	  SETi(i);
	}
	else {
	  const UV u = ~SvUV_nomg(sv);
	  SETu(u);
	}
      }
      else {
	register U8 *tmps;
	register I32 anum;
	STRLEN len;

	(void)SvPV_nomg_const(sv,len); /* force check for uninit var */
	sv_setsv_nomg(TARG, sv);
	tmps = (U8*)SvPV_force_nomg(TARG, len);
	anum = len;
	if (SvUTF8(TARG)) {
	  /* Calculate exact length, let's not estimate. */
	  STRLEN targlen = 0;
	  STRLEN l;
	  UV nchar = 0;
	  UV nwide = 0;
	  U8 * const send = tmps + len;
	  U8 * const origtmps = tmps;
	  const UV utf8flags = UTF8_ALLOW_ANYUV;

	  while (tmps < send) {
	    const UV c = utf8n_to_uvchr(tmps, send-tmps, &l, utf8flags);
	    tmps += l;
	    targlen += UNISKIP(~c);
	    nchar++;
	    if (c > 0xff)
		nwide++;
	  }

	  /* Now rewind strings and write them. */
	  tmps = origtmps;

	  if (nwide) {
	      U8 *result;
	      U8 *p;

	      Newx(result, targlen + 1, U8);
	      p = result;
	      while (tmps < send) {
		  const UV c = utf8n_to_uvchr(tmps, send-tmps, &l, utf8flags);
		  tmps += l;
		  p = uvchr_to_utf8_flags(p, ~c, UNICODE_ALLOW_ANY);
	      }
	      *p = '\0';
	      sv_usepvn_flags(TARG, (char*)result, targlen,
			      SV_HAS_TRAILING_NUL);
	      SvUTF8_on(TARG);
	  }
	  else {
	      U8 *result;
	      U8 *p;

	      Newx(result, nchar + 1, U8);
	      p = result;
	      while (tmps < send) {
		  const U8 c = (U8)utf8n_to_uvchr(tmps, send-tmps, &l, utf8flags);
		  tmps += l;
		  *p++ = ~c;
	      }
	      *p = '\0';
	      sv_usepvn_flags(TARG, (char*)result, nchar, SV_HAS_TRAILING_NUL);
	      SvUTF8_off(TARG);
	  }
	  SETTARG;
	  RETURN;
	}
#ifdef LIBERAL
	{
	    register long *tmpl;
	    for ( ; anum && (unsigned long)tmps % sizeof(long); anum--, tmps++)
		*tmps = ~*tmps;
	    tmpl = (long*)tmps;
	    for ( ; anum >= (I32)sizeof(long); anum -= (I32)sizeof(long), tmpl++)
		*tmpl = ~*tmpl;
	    tmps = (U8*)tmpl;
	}
#endif
	for ( ; anum > 0; anum--, tmps++)
	    *tmps = ~*tmps;
	SETTARG;
      }
      RETURN;
    }
}

/* integer versions of some of the above */

PP(pp_i_multiply)
{
    dVAR; dSP; dATARGET;
    tryAMAGICbin_MG(mult_amg, AMGf_assign);
    {
      dPOPTOPiirl_nomg;
      SETi( left * right );
      RETURN;
    }
}

PP(pp_i_divide)
{
    IV num;
    dVAR; dSP; dATARGET;
    tryAMAGICbin_MG(div_amg, AMGf_assign);
    {
      dPOPTOPssrl;
      IV value = SvIV_nomg(right);
      if (value == 0)
	  DIE(aTHX_ "Illegal division by zero");
      num = SvIV_nomg(left);

      /* avoid FPE_INTOVF on some platforms when num is IV_MIN */
      if (value == -1)
          value = - num;
      else
          value = num / value;
      SETi(value);
      RETURN;
    }
}

#if defined(__GLIBC__) && IVSIZE == 8
STATIC
PP(pp_i_modulo_0)
#else
PP(pp_i_modulo)
#endif
{
     /* This is the vanilla old i_modulo. */
     dVAR; dSP; dATARGET;
     tryAMAGICbin_MG(modulo_amg, AMGf_assign);
     {
	  dPOPTOPiirl_nomg;
	  if (!right)
	       DIE(aTHX_ "Illegal modulus zero");
	  /* avoid FPE_INTOVF on some platforms when left is IV_MIN */
	  if (right == -1)
	      SETi( 0 );
	  else
	      SETi( left % right );
	  RETURN;
     }
}

#if defined(__GLIBC__) && IVSIZE == 8
STATIC
PP(pp_i_modulo_1)

{
     /* This is the i_modulo with the workaround for the _moddi3 bug
      * in (at least) glibc 2.2.5 (the PERL_ABS() the workaround).
      * See below for pp_i_modulo. */
     dVAR; dSP; dATARGET;
     tryAMAGICbin_MG(modulo_amg, AMGf_assign);
     {
	  dPOPTOPiirl_nomg;
	  if (!right)
	       DIE(aTHX_ "Illegal modulus zero");
	  /* avoid FPE_INTOVF on some platforms when left is IV_MIN */
	  if (right == -1)
	      SETi( 0 );
	  else
	      SETi( left % PERL_ABS(right) );
	  RETURN;
     }
}

PP(pp_i_modulo)
{
     dVAR; dSP; dATARGET;
     tryAMAGICbin_MG(modulo_amg, AMGf_assign);
     {
	  dPOPTOPiirl_nomg;
	  if (!right)
	       DIE(aTHX_ "Illegal modulus zero");
	  /* The assumption is to use hereafter the old vanilla version... */
	  PL_op->op_ppaddr =
	       PL_ppaddr[OP_I_MODULO] =
	           Perl_pp_i_modulo_0;
	  /* .. but if we have glibc, we might have a buggy _moddi3
	   * (at least glicb 2.2.5 is known to have this bug), in other
	   * words our integer modulus with negative quad as the second
	   * argument might be broken.  Test for this and re-patch the
	   * opcode dispatch table if that is the case, remembering to
	   * also apply the workaround so that this first round works
	   * right, too.  See [perl #9402] for more information. */
	  {
	       IV l =   3;
	       IV r = -10;
	       /* Cannot do this check with inlined IV constants since
		* that seems to work correctly even with the buggy glibc. */
	       if (l % r == -3) {
		    /* Yikes, we have the bug.
		     * Patch in the workaround version. */
		    PL_op->op_ppaddr =
			 PL_ppaddr[OP_I_MODULO] =
			     &Perl_pp_i_modulo_1;
		    /* Make certain we work right this time, too. */
		    right = PERL_ABS(right);
	       }
	  }
	  /* avoid FPE_INTOVF on some platforms when left is IV_MIN */
	  if (right == -1)
	      SETi( 0 );
	  else
	      SETi( left % right );
	  RETURN;
     }
}
#endif

PP(pp_i_add)
{
    dVAR; dSP; dATARGET;
    tryAMAGICbin_MG(add_amg, AMGf_assign);
    {
      dPOPTOPiirl_ul_nomg;
      SETi( left + right );
      RETURN;
    }
}

PP(pp_i_subtract)
{
    dVAR; dSP; dATARGET;
    tryAMAGICbin_MG(subtr_amg, AMGf_assign);
    {
      dPOPTOPiirl_ul_nomg;
      SETi( left - right );
      RETURN;
    }
}

PP(pp_i_lt)
{
    dVAR; dSP;
    tryAMAGICbin_MG(lt_amg, AMGf_set);
    {
      dPOPTOPiirl_nomg;
      SETs(boolSV(left < right));
      RETURN;
    }
}

PP(pp_i_gt)
{
    dVAR; dSP;
    tryAMAGICbin_MG(gt_amg, AMGf_set);
    {
      dPOPTOPiirl_nomg;
      SETs(boolSV(left > right));
      RETURN;
    }
}

PP(pp_i_le)
{
    dVAR; dSP;
    tryAMAGICbin_MG(le_amg, AMGf_set);
    {
      dPOPTOPiirl_nomg;
      SETs(boolSV(left <= right));
      RETURN;
    }
}

PP(pp_i_ge)
{
    dVAR; dSP;
    tryAMAGICbin_MG(ge_amg, AMGf_set);
    {
      dPOPTOPiirl_nomg;
      SETs(boolSV(left >= right));
      RETURN;
    }
}

PP(pp_i_eq)
{
    dVAR; dSP;
    tryAMAGICbin_MG(eq_amg, AMGf_set);
    {
      dPOPTOPiirl_nomg;
      SETs(boolSV(left == right));
      RETURN;
    }
}

PP(pp_i_ne)
{
    dVAR; dSP;
    tryAMAGICbin_MG(ne_amg, AMGf_set);
    {
      dPOPTOPiirl_nomg;
      SETs(boolSV(left != right));
      RETURN;
    }
}

PP(pp_i_ncmp)
{
    dVAR; dSP; dTARGET;
    tryAMAGICbin_MG(ncmp_amg, 0);
    {
      dPOPTOPiirl_nomg;
      I32 value;

      if (left > right)
	value = 1;
      else if (left < right)
	value = -1;
      else
	value = 0;
      SETi(value);
      RETURN;
    }
}

PP(pp_i_negate)
{
    dVAR; dSP; dTARGET;
    tryAMAGICun_MG(neg_amg, 0);
    {
	SV * const sv = TOPs;
	IV const i = SvIV_nomg(sv);
	SETi(-i);
	RETURN;
    }
}

/* High falutin' math. */

PP(pp_atan2)
{
    dVAR; dSP; dTARGET;
    tryAMAGICbin_MG(atan2_amg, 0);
    {
      dPOPTOPnnrl_nomg;
      SETn(Perl_atan2(left, right));
      RETURN;
    }
}

PP(pp_sin)
{
    dVAR; dSP; dTARGET;
    int amg_type = sin_amg;
    const char *neg_report = NULL;
    NV (*func)(NV) = Perl_sin;
    const int op_type = PL_op->op_type;

    switch (op_type) {
    case OP_COS:
	amg_type = cos_amg;
	func = Perl_cos;
	break;
    case OP_EXP:
	amg_type = exp_amg;
	func = Perl_exp;
	break;
    case OP_LOG:
	amg_type = log_amg;
	func = Perl_log;
	neg_report = "log";
	break;
    case OP_SQRT:
	amg_type = sqrt_amg;
	func = Perl_sqrt;
	neg_report = "sqrt";
	break;
    }


    tryAMAGICun_MG(amg_type, 0);
    {
      SV * const arg = POPs;
      const NV value = SvNV_nomg(arg);
      if (neg_report) {
	  if (op_type == OP_LOG ? (value <= 0.0) : (value < 0.0)) {
	      SET_NUMERIC_STANDARD();
	      /* diag_listed_as: Can't take log of %g */
	      DIE(aTHX_ "Can't take %s of %"NVgf, neg_report, value);
	  }
      }
      XPUSHn(func(value));
      RETURN;
    }
}

/* Support Configure command-line overrides for rand() functions.
   After 5.005, perhaps we should replace this by Configure support
   for drand48(), random(), or rand().  For 5.005, though, maintain
   compatibility by calling rand() but allow the user to override it.
   See INSTALL for details.  --Andy Dougherty  15 July 1998
*/
/* Now it's after 5.005, and Configure supports drand48() and random(),
   in addition to rand().  So the overrides should not be needed any more.
   --Jarkko Hietaniemi	27 September 1998
 */

#ifndef HAS_DRAND48_PROTO
extern double drand48 (void);
#endif

PP(pp_rand)
{
    dVAR; dSP; dTARGET;
    NV value;
    if (MAXARG < 1)
	value = 1.0;
    else if (!TOPs) {
	value = 1.0; (void)POPs;
    }
    else
	value = POPn;
    if (value == 0.0)
	value = 1.0;
    if (!PL_srand_called) {
	(void)seedDrand01((Rand_seed_t)seed());
	PL_srand_called = TRUE;
    }
    value *= Drand01();
    XPUSHn(value);
    RETURN;
}

PP(pp_srand)
{
    dVAR; dSP; dTARGET;
    const UV anum = (MAXARG < 1 || (!TOPs && !POPs)) ? seed() : POPu;
    (void)seedDrand01((Rand_seed_t)anum);
    PL_srand_called = TRUE;
    if (anum)
	XPUSHu(anum);
    else {
	/* Historically srand always returned true. We can avoid breaking
	   that like this:  */
	sv_setpvs(TARG, "0 but true");
	XPUSHTARG;
    }
    RETURN;
}

PP(pp_int)
{
    dVAR; dSP; dTARGET;
    tryAMAGICun_MG(int_amg, AMGf_numeric);
    {
      SV * const sv = TOPs;
      const IV iv = SvIV_nomg(sv);
      /* XXX it's arguable that compiler casting to IV might be subtly
	 different from modf (for numbers inside (IV_MIN,UV_MAX)) in which
	 else preferring IV has introduced a subtle behaviour change bug. OTOH
	 relying on floating point to be accurate is a bug.  */

      if (!SvOK(sv)) {
        SETu(0);
      }
      else if (SvIOK(sv)) {
	if (SvIsUV(sv))
	    SETu(SvUV_nomg(sv));
	else
	    SETi(iv);
      }
      else {
	  const NV value = SvNV_nomg(sv);
	  if (value >= 0.0) {
	      if (value < (NV)UV_MAX + 0.5) {
		  SETu(U_V(value));
	      } else {
		  SETn(Perl_floor(value));
	      }
	  }
	  else {
	      if (value > (NV)IV_MIN - 0.5) {
		  SETi(I_V(value));
	      } else {
		  SETn(Perl_ceil(value));
	      }
	  }
      }
    }
    RETURN;
}

PP(pp_abs)
{
    dVAR; dSP; dTARGET;
    tryAMAGICun_MG(abs_amg, AMGf_numeric);
    {
      SV * const sv = TOPs;
      /* This will cache the NV value if string isn't actually integer  */
      const IV iv = SvIV_nomg(sv);

      if (!SvOK(sv)) {
        SETu(0);
      }
      else if (SvIOK(sv)) {
	/* IVX is precise  */
	if (SvIsUV(sv)) {
	  SETu(SvUV_nomg(sv));	/* force it to be numeric only */
	} else {
	  if (iv >= 0) {
	    SETi(iv);
	  } else {
	    if (iv != IV_MIN) {
	      SETi(-iv);
	    } else {
	      /* 2s complement assumption. Also, not really needed as
		 IV_MIN and -IV_MIN should both be %100...00 and NV-able  */
	      SETu(IV_MIN);
	    }
	  }
	}
      } else{
	const NV value = SvNV_nomg(sv);
	if (value < 0.0)
	  SETn(-value);
	else
	  SETn(value);
      }
    }
    RETURN;
}

PP(pp_oct)
{
    dVAR; dSP; dTARGET;
    const char *tmps;
    I32 flags = PERL_SCAN_ALLOW_UNDERSCORES;
    STRLEN len;
    NV result_nv;
    UV result_uv;
    SV* const sv = POPs;

    tmps = (SvPV_const(sv, len));
    if (DO_UTF8(sv)) {
	 /* If Unicode, try to downgrade
	  * If not possible, croak. */
	 SV* const tsv = sv_2mortal(newSVsv(sv));
	
	 SvUTF8_on(tsv);
	 sv_utf8_downgrade(tsv, FALSE);
	 tmps = SvPV_const(tsv, len);
    }
    if (PL_op->op_type == OP_HEX)
	goto hex;

    while (*tmps && len && isSPACE(*tmps))
        tmps++, len--;
    if (*tmps == '0')
        tmps++, len--;
    if (*tmps == 'x' || *tmps == 'X') {
    hex:
        result_uv = grok_hex (tmps, &len, &flags, &result_nv);
    }
    else if (*tmps == 'b' || *tmps == 'B')
        result_uv = grok_bin (tmps, &len, &flags, &result_nv);
    else
        result_uv = grok_oct (tmps, &len, &flags, &result_nv);

    if (flags & PERL_SCAN_GREATER_THAN_UV_MAX) {
        XPUSHn(result_nv);
    }
    else {
        XPUSHu(result_uv);
    }
    RETURN;
}

/* String stuff. */

PP(pp_length)
{
    dVAR; dSP; dTARGET;
    SV * const sv = TOPs;

    if (SvGAMAGIC(sv)) {
	/* For an overloaded or magic scalar, we can't know in advance if
	   it's going to be UTF-8 or not. Also, we can't call sv_len_utf8 as
	   it likes to cache the length. Maybe that should be a documented
	   feature of it.
	*/
	STRLEN len;
	const char *const p
	    = sv_2pv_flags(sv, &len,
			   SV_UNDEF_RETURNS_NULL|SV_CONST_RETURN|SV_GMAGIC);

	if (!p) {
	    if (!SvPADTMP(TARG)) {
		sv_setsv(TARG, &PL_sv_undef);
		SETTARG;
	    }
	    SETs(&PL_sv_undef);
	}
	else if (DO_UTF8(sv)) {
	    SETi(utf8_length((U8*)p, (U8*)p + len));
	}
	else
	    SETi(len);
    } else if (SvOK(sv)) {
	/* Neither magic nor overloaded.  */
	if (DO_UTF8(sv))
	    SETi(sv_len_utf8(sv));
	else
	    SETi(sv_len(sv));
    } else {
	if (!SvPADTMP(TARG)) {
	    sv_setsv_nomg(TARG, &PL_sv_undef);
	    SETTARG;
	}
	SETs(&PL_sv_undef);
    }
    RETURN;
}

/* Returns false if substring is completely outside original string.
   No length is indicated by len_iv = 0 and len_is_uv = 0.  len_is_uv must
   always be true for an explicit 0.
*/
bool
Perl_translate_substr_offsets(pTHX_ STRLEN curlen, IV pos1_iv,
				    bool pos1_is_uv, IV len_iv,
				    bool len_is_uv, STRLEN *posp,
				    STRLEN *lenp)
{
    IV pos2_iv;
    int    pos2_is_uv;

    PERL_ARGS_ASSERT_TRANSLATE_SUBSTR_OFFSETS;

    if (!pos1_is_uv && pos1_iv < 0 && curlen) {
	pos1_is_uv = curlen-1 > ~(UV)pos1_iv;
	pos1_iv += curlen;
    }
    if ((pos1_is_uv || pos1_iv > 0) && (UV)pos1_iv > curlen)
	return FALSE;

    if (len_iv || len_is_uv) {
	if (!len_is_uv && len_iv < 0) {
	    pos2_iv = curlen + len_iv;
	    if (curlen)
		pos2_is_uv = curlen-1 > ~(UV)len_iv;
	    else
		pos2_is_uv = 0;
	} else {  /* len_iv >= 0 */
	    if (!pos1_is_uv && pos1_iv < 0) {
		pos2_iv = pos1_iv + len_iv;
		pos2_is_uv = (UV)len_iv > (UV)IV_MAX;
	    } else {
		if ((UV)len_iv > curlen-(UV)pos1_iv)
		    pos2_iv = curlen;
		else
		    pos2_iv = pos1_iv+len_iv;
		pos2_is_uv = 1;
	    }
	}
    }
    else {
	pos2_iv = curlen;
	pos2_is_uv = 1;
    }

    if (!pos2_is_uv && pos2_iv < 0) {
	if (!pos1_is_uv && pos1_iv < 0)
	    return FALSE;
	pos2_iv = 0;
    }
    else if (!pos1_is_uv && pos1_iv < 0)
	pos1_iv = 0;

    if ((UV)pos2_iv < (UV)pos1_iv)
	pos2_iv = pos1_iv;
    if ((UV)pos2_iv > curlen)
	pos2_iv = curlen;

    /* pos1_iv and pos2_iv both in 0..curlen, so the cast is safe */
    *posp = (STRLEN)( (UV)pos1_iv );
    *lenp = (STRLEN)( (UV)pos2_iv - (UV)pos1_iv );

    return TRUE;
}

PP(pp_substr)
{
    dVAR; dSP; dTARGET;
    SV *sv;
    STRLEN curlen;
    STRLEN utf8_curlen;
    SV *   pos_sv;
    IV     pos1_iv;
    int    pos1_is_uv;
    SV *   len_sv;
    IV     len_iv = 0;
    int    len_is_uv = 0;
    I32 lvalue = PL_op->op_flags & OPf_MOD || LVRET;
    const bool rvalue = (GIMME_V != G_VOID);
    const char *tmps;
    SV *repl_sv = NULL;
    const char *repl = NULL;
    STRLEN repl_len;
    int num_args = PL_op->op_private & 7;
    bool repl_need_utf8_upgrade = FALSE;
    bool repl_is_utf8 = FALSE;

    if (num_args > 2) {
	if (num_args > 3) {
	  if(!(repl_sv = POPs)) num_args--;
	}
	if ((len_sv = POPs)) {
	    len_iv    = SvIV(len_sv);
	    len_is_uv = len_iv ? SvIOK_UV(len_sv) : 1;
	}
	else num_args--;
    }
    pos_sv     = POPs;
    pos1_iv    = SvIV(pos_sv);
    pos1_is_uv = SvIOK_UV(pos_sv);
    sv = POPs;
    if (PL_op->op_private & OPpSUBSTR_REPL_FIRST) {
	assert(!repl_sv);
	repl_sv = POPs;
    }
    PUTBACK;
    if (repl_sv) {
	repl = SvPV_const(repl_sv, repl_len);
	repl_is_utf8 = DO_UTF8(repl_sv) && repl_len;
	if (repl_is_utf8) {
	    if (!DO_UTF8(sv))
		sv_utf8_upgrade(sv);
	}
	else if (DO_UTF8(sv))
	    repl_need_utf8_upgrade = TRUE;
    }
    else if (lvalue) {
	SV * ret;
	ret = sv_2mortal(newSV_type(SVt_PVLV));  /* Not TARG RT#67838 */
	sv_magic(ret, NULL, PERL_MAGIC_substr, NULL, 0);
	LvTYPE(ret) = 'x';
	LvTARG(ret) = SvREFCNT_inc_simple(sv);
	LvTARGOFF(ret) =
	    pos1_is_uv || pos1_iv >= 0
		? (STRLEN)(UV)pos1_iv
		: (LvFLAGS(ret) |= 1, (STRLEN)(UV)-pos1_iv);
	LvTARGLEN(ret) =
	    len_is_uv || len_iv > 0
		? (STRLEN)(UV)len_iv
		: (LvFLAGS(ret) |= 2, (STRLEN)(UV)-len_iv);

	SPAGAIN;
	PUSHs(ret);    /* avoid SvSETMAGIC here */
	RETURN;
    }
    tmps = SvPV_const(sv, curlen);
    if (DO_UTF8(sv)) {
        utf8_curlen = sv_len_utf8(sv);
	if (utf8_curlen == curlen)
	    utf8_curlen = 0;
	else
	    curlen = utf8_curlen;
    }
    else
	utf8_curlen = 0;

    {
	STRLEN pos, len, byte_len, byte_pos;

	if (!translate_substr_offsets(
		curlen, pos1_iv, pos1_is_uv, len_iv, len_is_uv, &pos, &len
	)) goto bound_fail;

	byte_len = len;
	byte_pos = utf8_curlen
	    ? sv_pos_u2b_flags(sv, pos, &byte_len, SV_CONST_RETURN) : pos;

	tmps += byte_pos;

	if (rvalue) {
	    SvTAINTED_off(TARG);			/* decontaminate */
	    SvUTF8_off(TARG);			/* decontaminate */
	    sv_setpvn(TARG, tmps, byte_len);
#ifdef USE_LOCALE_COLLATE
	    sv_unmagic(TARG, PERL_MAGIC_collxfrm);
#endif
	    if (utf8_curlen)
		SvUTF8_on(TARG);
	}

	if (repl) {
	    SV* repl_sv_copy = NULL;

	    if (repl_need_utf8_upgrade) {
		repl_sv_copy = newSVsv(repl_sv);
		sv_utf8_upgrade(repl_sv_copy);
		repl = SvPV_const(repl_sv_copy, repl_len);
		repl_is_utf8 = DO_UTF8(repl_sv_copy) && repl_len;
	    }
	    if (SvROK(sv))
		Perl_ck_warner(aTHX_ packWARN(WARN_SUBSTR),
			    "Attempt to use reference as lvalue in substr"
		);
	    if (!SvOK(sv))
		sv_setpvs(sv, "");
	    sv_insert_flags(sv, byte_pos, byte_len, repl, repl_len, 0);
	    if (repl_is_utf8)
		SvUTF8_on(sv);
	    SvREFCNT_dec(repl_sv_copy);
	}
    }
    SPAGAIN;
    if (rvalue) {
	SvSETMAGIC(TARG);
	PUSHs(TARG);
    }
    RETURN;

bound_fail:
    if (repl)
	Perl_croak(aTHX_ "substr outside of string");
    Perl_ck_warner(aTHX_ packWARN(WARN_SUBSTR), "substr outside of string");
    RETPUSHUNDEF;
}

PP(pp_vec)
{
    dVAR; dSP;
    register const IV size   = POPi;
    register const IV offset = POPi;
    register SV * const src = POPs;
    const I32 lvalue = PL_op->op_flags & OPf_MOD || LVRET;
    SV * ret;

    if (lvalue) {			/* it's an lvalue! */
	ret = sv_2mortal(newSV_type(SVt_PVLV));  /* Not TARG RT#67838 */
	sv_magic(ret, NULL, PERL_MAGIC_vec, NULL, 0);
	LvTYPE(ret) = 'v';
	LvTARG(ret) = SvREFCNT_inc_simple(src);
	LvTARGOFF(ret) = offset;
	LvTARGLEN(ret) = size;
    }
    else {
	dTARGET;
	SvTAINTED_off(TARG);		/* decontaminate */
	ret = TARG;
    }

    sv_setuv(ret, do_vecget(src, offset, size));
    PUSHs(ret);
    RETURN;
}

PP(pp_index)
{
    dVAR; dSP; dTARGET;
    SV *big;
    SV *little;
    SV *temp = NULL;
    STRLEN biglen;
    STRLEN llen = 0;
    I32 offset;
    I32 retval;
    const char *big_p;
    const char *little_p;
    bool big_utf8;
    bool little_utf8;
    const bool is_index = PL_op->op_type == OP_INDEX;
    const bool threeargs = MAXARG >= 3 && (TOPs || ((void)POPs,0));

    if (threeargs)
	offset = POPi;
    little = POPs;
    big = POPs;
    big_p = SvPV_const(big, biglen);
    little_p = SvPV_const(little, llen);

    big_utf8 = DO_UTF8(big);
    little_utf8 = DO_UTF8(little);
    if (big_utf8 ^ little_utf8) {
	/* One needs to be upgraded.  */
	if (little_utf8 && !PL_encoding) {
	    /* Well, maybe instead we might be able to downgrade the small
	       string?  */
	    char * const pv = (char*)bytes_from_utf8((U8 *)little_p, &llen,
						     &little_utf8);
	    if (little_utf8) {
		/* If the large string is ISO-8859-1, and it's not possible to
		   convert the small string to ISO-8859-1, then there is no
		   way that it could be found anywhere by index.  */
		retval = -1;
		goto fail;
	    }

	    /* At this point, pv is a malloc()ed string. So donate it to temp
	       to ensure it will get free()d  */
	    little = temp = newSV(0);
	    sv_usepvn(temp, pv, llen);
	    little_p = SvPVX(little);
	} else {
	    temp = little_utf8
		? newSVpvn(big_p, biglen) : newSVpvn(little_p, llen);

	    if (PL_encoding) {
		sv_recode_to_utf8(temp, PL_encoding);
	    } else {
		sv_utf8_upgrade(temp);
	    }
	    if (little_utf8) {
		big = temp;
		big_utf8 = TRUE;
		big_p = SvPV_const(big, biglen);
	    } else {
		little = temp;
		little_p = SvPV_const(little, llen);
	    }
	}
    }
    if (SvGAMAGIC(big)) {
	/* Life just becomes a lot easier if I use a temporary here.
	   Otherwise I need to avoid calls to sv_pos_u2b(), which (dangerously)
	   will trigger magic and overloading again, as will fbm_instr()
	*/
	big = newSVpvn_flags(big_p, biglen,
			     SVs_TEMP | (big_utf8 ? SVf_UTF8 : 0));
	big_p = SvPVX(big);
    }
    if (SvGAMAGIC(little) || (is_index && !SvOK(little))) {
	/* index && SvOK() is a hack. fbm_instr() calls SvPV_const, which will
	   warn on undef, and we've already triggered a warning with the
	   SvPV_const some lines above. We can't remove that, as we need to
	   call some SvPV to trigger overloading early and find out if the
	   string is UTF-8.
	   This is all getting to messy. The API isn't quite clean enough,
	   because data access has side effects.
	*/
	little = newSVpvn_flags(little_p, llen,
				SVs_TEMP | (little_utf8 ? SVf_UTF8 : 0));
	little_p = SvPVX(little);
    }

    if (!threeargs)
	offset = is_index ? 0 : biglen;
    else {
	if (big_utf8 && offset > 0)
	    sv_pos_u2b(big, &offset, 0);
	if (!is_index)
	    offset += llen;
    }
    if (offset < 0)
	offset = 0;
    else if (offset > (I32)biglen)
	offset = biglen;
    if (!(little_p = is_index
	  ? fbm_instr((unsigned char*)big_p + offset,
		      (unsigned char*)big_p + biglen, little, 0)
	  : rninstr(big_p,  big_p  + offset,
		    little_p, little_p + llen)))
	retval = -1;
    else {
	retval = little_p - big_p;
	if (retval > 0 && big_utf8)
	    sv_pos_b2u(big, &retval);
    }
    SvREFCNT_dec(temp);
 fail:
    PUSHi(retval);
    RETURN;
}

PP(pp_sprintf)
{
    dVAR; dSP; dMARK; dORIGMARK; dTARGET;
    SvTAINTED_off(TARG);
    do_sprintf(TARG, SP-MARK, MARK+1);
    TAINT_IF(SvTAINTED(TARG));
    SP = ORIGMARK;
    PUSHTARG;
    RETURN;
}

PP(pp_ord)
{
    dVAR; dSP; dTARGET;

    SV *argsv = POPs;
    STRLEN len;
    const U8 *s = (U8*)SvPV_const(argsv, len);

    if (PL_encoding && SvPOK(argsv) && !DO_UTF8(argsv)) {
        SV * const tmpsv = sv_2mortal(newSVsv(argsv));
        s = (U8*)sv_recode_to_utf8(tmpsv, PL_encoding);
        argsv = tmpsv;
    }

    XPUSHu(DO_UTF8(argsv) ?
	   utf8n_to_uvchr(s, UTF8_MAXBYTES, 0, UTF8_ALLOW_ANYUV) :
	   (UV)(*s & 0xff));

    RETURN;
}

PP(pp_chr)
{
    dVAR; dSP; dTARGET;
    char *tmps;
    UV value;

    if (((SvIOK_notUV(TOPs) && SvIV(TOPs) < 0)
	 ||
	 (SvNOK(TOPs) && SvNV(TOPs) < 0.0))) {
	if (IN_BYTES) {
	    value = POPu; /* chr(-1) eq chr(0xff), etc. */
	} else {
	    (void) POPs; /* Ignore the argument value. */
	    value = UNICODE_REPLACEMENT;
	}
    } else {
	value = POPu;
    }

    SvUPGRADE(TARG,SVt_PV);

    if (value > 255 && !IN_BYTES) {
	SvGROW(TARG, (STRLEN)UNISKIP(value)+1);
	tmps = (char*)uvchr_to_utf8_flags((U8*)SvPVX(TARG), value, 0);
	SvCUR_set(TARG, tmps - SvPVX_const(TARG));
	*tmps = '\0';
	(void)SvPOK_only(TARG);
	SvUTF8_on(TARG);
	XPUSHs(TARG);
	RETURN;
    }

    SvGROW(TARG,2);
    SvCUR_set(TARG, 1);
    tmps = SvPVX(TARG);
    *tmps++ = (char)value;
    *tmps = '\0';
    (void)SvPOK_only(TARG);

    if (PL_encoding && !IN_BYTES) {
        sv_recode_to_utf8(TARG, PL_encoding);
	tmps = SvPVX(TARG);
	if (SvCUR(TARG) == 0 || !is_utf8_string((U8*)tmps, SvCUR(TARG)) ||
	    UNICODE_IS_REPLACEMENT(utf8_to_uvchr_buf((U8*)tmps, (U8*) tmps + SvCUR(TARG), NULL))) {
	    SvGROW(TARG, 2);
	    tmps = SvPVX(TARG);
	    SvCUR_set(TARG, 1);
	    *tmps++ = (char)value;
	    *tmps = '\0';
	    SvUTF8_off(TARG);
	}
    }

    XPUSHs(TARG);
    RETURN;
}

PP(pp_crypt)
{
#ifdef HAS_CRYPT
    dVAR; dSP; dTARGET;
    dPOPTOPssrl;
    STRLEN len;
    const char *tmps = SvPV_const(left, len);

    if (DO_UTF8(left)) {
         /* If Unicode, try to downgrade.
	  * If not possible, croak.
	  * Yes, we made this up.  */
	 SV* const tsv = sv_2mortal(newSVsv(left));

	 SvUTF8_on(tsv);
	 sv_utf8_downgrade(tsv, FALSE);
	 tmps = SvPV_const(tsv, len);
    }
#   ifdef USE_ITHREADS
#     ifdef HAS_CRYPT_R
    if (!PL_reentrant_buffer->_crypt_struct_buffer) {
      /* This should be threadsafe because in ithreads there is only
       * one thread per interpreter.  If this would not be true,
       * we would need a mutex to protect this malloc. */
        PL_reentrant_buffer->_crypt_struct_buffer =
	  (struct crypt_data *)safemalloc(sizeof(struct crypt_data));
#if defined(__GLIBC__) || defined(__EMX__)
	if (PL_reentrant_buffer->_crypt_struct_buffer) {
	    PL_reentrant_buffer->_crypt_struct_buffer->initialized = 0;
	    /* work around glibc-2.2.5 bug */
	    PL_reentrant_buffer->_crypt_struct_buffer->current_saltbits = 0;
	}
#endif
    }
#     endif /* HAS_CRYPT_R */
#   endif /* USE_ITHREADS */
#   ifdef FCRYPT
    sv_setpv(TARG, fcrypt(tmps, SvPV_nolen_const(right)));
#   else
    sv_setpv(TARG, PerlProc_crypt(tmps, SvPV_nolen_const(right)));
#   endif
    SETTARG;
    RETURN;
#else
    DIE(aTHX_
      "The crypt() function is unimplemented due to excessive paranoia.");
#endif
}

/* Generally UTF-8 and UTF-EBCDIC are indistinguishable at this level.  So 
 * most comments below say UTF-8, when in fact they mean UTF-EBCDIC as well */

/* Generates code to store a unicode codepoint c that is known to occupy
 * exactly two UTF-8 and UTF-EBCDIC bytes; it is stored into p and p+1,
 * and p is advanced to point to the next available byte after the two bytes */
#define CAT_UNI_TO_UTF8_TWO_BYTE(p, c)					    \
    STMT_START {							    \
	*(p)++ = UTF8_TWO_BYTE_HI(c);					    \
	*((p)++) = UTF8_TWO_BYTE_LO(c);					    \
    } STMT_END

PP(pp_ucfirst)
{
    /* Actually is both lcfirst() and ucfirst().  Only the first character
     * changes.  This means that possibly we can change in-place, ie., just
     * take the source and change that one character and store it back, but not
     * if read-only etc, or if the length changes */

    dVAR;
    dSP;
    SV *source = TOPs;
    STRLEN slen; /* slen is the byte length of the whole SV. */
    STRLEN need;
    SV *dest;
    bool inplace;   /* ? Convert first char only, in-place */
    bool doing_utf8 = FALSE;		   /* ? using utf8 */
    bool convert_source_to_utf8 = FALSE;   /* ? need to convert */
    const int op_type = PL_op->op_type;
    const U8 *s;
    U8 *d;
    U8 tmpbuf[UTF8_MAXBYTES_CASE+1];
    STRLEN ulen;    /* ulen is the byte length of the original Unicode character
		     * stored as UTF-8 at s. */
    STRLEN tculen;  /* tculen is the byte length of the freshly titlecased (or
		     * lowercased) character stored in tmpbuf.  May be either
		     * UTF-8 or not, but in either case is the number of bytes */
    bool tainted = FALSE;

    SvGETMAGIC(source);
    if (SvOK(source)) {
	s = (const U8*)SvPV_nomg_const(source, slen);
    } else {
	if (ckWARN(WARN_UNINITIALIZED))
	    report_uninit(source);
	s = (const U8*)"";
	slen = 0;
    }

    /* We may be able to get away with changing only the first character, in
     * place, but not if read-only, etc.  Later we may discover more reasons to
     * not convert in-place. */
    inplace = SvPADTMP(source) && !SvREADONLY(source) && SvTEMP(source);

    /* First calculate what the changed first character should be.  This affects
     * whether we can just swap it out, leaving the rest of the string unchanged,
     * or even if have to convert the dest to UTF-8 when the source isn't */

    if (! slen) {   /* If empty */
	need = 1; /* still need a trailing NUL */
	ulen = 0;
    }
    else if (DO_UTF8(source)) {	/* Is the source utf8? */
	doing_utf8 = TRUE;
        ulen = UTF8SKIP(s);
        if (op_type == OP_UCFIRST) {
	    _to_utf8_title_flags(s, tmpbuf, &tculen,
				 cBOOL(IN_LOCALE_RUNTIME), &tainted);
	}
        else {
	    _to_utf8_lower_flags(s, tmpbuf, &tculen,
				 cBOOL(IN_LOCALE_RUNTIME), &tainted);
	}

        /* we can't do in-place if the length changes.  */
        if (ulen != tculen) inplace = FALSE;
        need = slen + 1 - ulen + tculen;
    }
    else { /* Non-zero length, non-UTF-8,  Need to consider locale and if
	    * latin1 is treated as caseless.  Note that a locale takes
	    * precedence */ 
	ulen = 1;	/* Original character is 1 byte */
	tculen = 1;	/* Most characters will require one byte, but this will
			 * need to be overridden for the tricky ones */
	need = slen + 1;

	if (op_type == OP_LCFIRST) {

	    /* lower case the first letter: no trickiness for any character */
	    *tmpbuf = (IN_LOCALE_RUNTIME) ? toLOWER_LC(*s) :
			((IN_UNI_8_BIT) ? toLOWER_LATIN1(*s) : toLOWER(*s));
	}
	/* is ucfirst() */
	else if (IN_LOCALE_RUNTIME) {
	    *tmpbuf = toUPPER_LC(*s);	/* This would be a bug if any locales
					 * have upper and title case different
					 */
	}
	else if (! IN_UNI_8_BIT) {
	    *tmpbuf = toUPPER(*s);	/* Returns caseless for non-ascii, or
					 * on EBCDIC machines whatever the
					 * native function does */
	}
	else { /* is ucfirst non-UTF-8, not in locale, and cased latin1 */
	    UV title_ord = _to_upper_title_latin1(*s, tmpbuf, &tculen, 's');
	    if (tculen > 1) {
		assert(tculen == 2);

                /* If the result is an upper Latin1-range character, it can
                 * still be represented in one byte, which is its ordinal */
		if (UTF8_IS_DOWNGRADEABLE_START(*tmpbuf)) {
		    *tmpbuf = (U8) title_ord;
		    tculen = 1;
		}
		else {
                    /* Otherwise it became more than one ASCII character (in
                     * the case of LATIN_SMALL_LETTER_SHARP_S) or changed to
                     * beyond Latin1, so the number of bytes changed, so can't
                     * replace just the first character in place. */
		    inplace = FALSE;

		    /* If the result won't fit in a byte, the entire result will
		     * have to be in UTF-8.  Assume worst case sizing in
		     * conversion. (all latin1 characters occupy at most two bytes
		     * in utf8) */
		    if (title_ord > 255) {
			doing_utf8 = TRUE;
			convert_source_to_utf8 = TRUE;
			need = slen * 2 + 1;

                        /* The (converted) UTF-8 and UTF-EBCDIC lengths of all
                         * (both) characters whose title case is above 255 is
                         * 2. */
			ulen = 2;
		    }
                    else { /* LATIN_SMALL_LETTER_SHARP_S expands by 1 byte */
			need = slen + 1 + 1;
		    }
		}
	    }
	} /* End of use Unicode (Latin1) semantics */
    } /* End of changing the case of the first character */

    /* Here, have the first character's changed case stored in tmpbuf.  Ready to
     * generate the result */
    if (inplace) {

	/* We can convert in place.  This means we change just the first
	 * character without disturbing the rest; no need to grow */
	dest = source;
	s = d = (U8*)SvPV_force_nomg(source, slen);
    } else {
	dTARGET;

	dest = TARG;

	/* Here, we can't convert in place; we earlier calculated how much
	 * space we will need, so grow to accommodate that */
	SvUPGRADE(dest, SVt_PV);
	d = (U8*)SvGROW(dest, need);
	(void)SvPOK_only(dest);

	SETs(dest);
    }

    if (doing_utf8) {
	if (! inplace) {
	    if (! convert_source_to_utf8) {

		/* Here  both source and dest are in UTF-8, but have to create
		 * the entire output.  We initialize the result to be the
		 * title/lower cased first character, and then append the rest
		 * of the string. */
		sv_setpvn(dest, (char*)tmpbuf, tculen);
		if (slen > ulen) {
		    sv_catpvn(dest, (char*)(s + ulen), slen - ulen);
		}
	    }
	    else {
		const U8 *const send = s + slen;

		/* Here the dest needs to be in UTF-8, but the source isn't,
		 * except we earlier UTF-8'd the first character of the source
		 * into tmpbuf.  First put that into dest, and then append the
		 * rest of the source, converting it to UTF-8 as we go. */

		/* Assert tculen is 2 here because the only two characters that
		 * get to this part of the code have 2-byte UTF-8 equivalents */
		*d++ = *tmpbuf;
		*d++ = *(tmpbuf + 1);
		s++;	/* We have just processed the 1st char */

		for (; s < send; s++) {
		    d = uvchr_to_utf8(d, *s);
		}
		*d = '\0';
		SvCUR_set(dest, d - (U8*)SvPVX_const(dest));
	    }
	    SvUTF8_on(dest);
	}
	else {   /* in-place UTF-8.  Just overwrite the first character */
	    Copy(tmpbuf, d, tculen, U8);
	    SvCUR_set(dest, need - 1);
	}

	if (tainted) {
	    TAINT;
	    SvTAINTED_on(dest);
	}
    }
    else {  /* Neither source nor dest are in or need to be UTF-8 */
	if (slen) {
	    if (IN_LOCALE_RUNTIME) {
		TAINT;
		SvTAINTED_on(dest);
	    }
	    if (inplace) {  /* in-place, only need to change the 1st char */
		*d = *tmpbuf;
	    }
	    else {	/* Not in-place */

		/* Copy the case-changed character(s) from tmpbuf */
		Copy(tmpbuf, d, tculen, U8);
		d += tculen - 1; /* Code below expects d to point to final
				  * character stored */
	    }
	}
	else {	/* empty source */
	    /* See bug #39028: Don't taint if empty  */
	    *d = *s;
	}

	/* In a "use bytes" we don't treat the source as UTF-8, but, still want
	 * the destination to retain that flag */
	if (SvUTF8(source))
	    SvUTF8_on(dest);

	if (!inplace) {	/* Finish the rest of the string, unchanged */
	    /* This will copy the trailing NUL  */
	    Copy(s + 1, d + 1, slen, U8);
	    SvCUR_set(dest, need - 1);
	}
    }
    if (dest != source && SvTAINTED(source))
	SvTAINT(dest);
    SvSETMAGIC(dest);
    RETURN;
}

/* There's so much setup/teardown code common between uc and lc, I wonder if
   it would be worth merging the two, and just having a switch outside each
   of the three tight loops.  There is less and less commonality though */
PP(pp_uc)
{
    dVAR;
    dSP;
    SV *source = TOPs;
    STRLEN len;
    STRLEN min;
    SV *dest;
    const U8 *s;
    U8 *d;

    SvGETMAGIC(source);

    if (SvPADTMP(source) && !SvREADONLY(source) && !SvAMAGIC(source)
	&& SvTEMP(source) && !DO_UTF8(source)
	&& (IN_LOCALE_RUNTIME || ! IN_UNI_8_BIT)) {

	/* We can convert in place.  The reason we can't if in UNI_8_BIT is to
	 * make the loop tight, so we overwrite the source with the dest before
	 * looking at it, and we need to look at the original source
	 * afterwards.  There would also need to be code added to handle
	 * switching to not in-place in midstream if we run into characters
	 * that change the length.
	 */
	dest = source;
	s = d = (U8*)SvPV_force_nomg(source, len);
	min = len + 1;
    } else {
	dTARGET;

	dest = TARG;

	/* The old implementation would copy source into TARG at this point.
	   This had the side effect that if source was undef, TARG was now
	   an undefined SV with PADTMP set, and they don't warn inside
	   sv_2pv_flags(). However, we're now getting the PV direct from
	   source, which doesn't have PADTMP set, so it would warn. Hence the
	   little games.  */

	if (SvOK(source)) {
	    s = (const U8*)SvPV_nomg_const(source, len);
	} else {
	    if (ckWARN(WARN_UNINITIALIZED))
		report_uninit(source);
	    s = (const U8*)"";
	    len = 0;
	}
	min = len + 1;

	SvUPGRADE(dest, SVt_PV);
	d = (U8*)SvGROW(dest, min);
	(void)SvPOK_only(dest);

	SETs(dest);
    }

    /* Overloaded values may have toggled the UTF-8 flag on source, so we need
       to check DO_UTF8 again here.  */

    if (DO_UTF8(source)) {
	const U8 *const send = s + len;
	U8 tmpbuf[UTF8_MAXBYTES+1];
	bool tainted = FALSE;

	/* All occurrences of these are to be moved to follow any other marks.
	 * This is context-dependent.  We may not be passed enough context to
	 * move the iota subscript beyond all of them, but we do the best we can
	 * with what we're given.  The result is always better than if we
	 * hadn't done this.  And, the problem would only arise if we are
	 * passed a character without all its combining marks, which would be
	 * the caller's mistake.  The information this is based on comes from a
	 * comment in Unicode SpecialCasing.txt, (and the Standard's text
	 * itself) and so can't be checked properly to see if it ever gets
	 * revised.  But the likelihood of it changing is remote */
	bool in_iota_subscript = FALSE;

	while (s < send) {
	    STRLEN u;
	    STRLEN ulen;
	    UV uv;
	    if (in_iota_subscript && ! is_utf8_mark(s)) {

		/* A non-mark.  Time to output the iota subscript */
#define GREEK_CAPITAL_LETTER_IOTA 0x0399
#define COMBINING_GREEK_YPOGEGRAMMENI 0x0345

		CAT_UNI_TO_UTF8_TWO_BYTE(d, GREEK_CAPITAL_LETTER_IOTA);
		in_iota_subscript = FALSE;
            }

            /* Then handle the current character.  Get the changed case value
             * and copy it to the output buffer */

            u = UTF8SKIP(s);
            uv = _to_utf8_upper_flags(s, tmpbuf, &ulen,
				      cBOOL(IN_LOCALE_RUNTIME), &tainted);
            if (uv == GREEK_CAPITAL_LETTER_IOTA
                && utf8_to_uvchr_buf(s, send, 0) == COMBINING_GREEK_YPOGEGRAMMENI)
            {
                in_iota_subscript = TRUE;
            }
            else {
                if (ulen > u && (SvLEN(dest) < (min += ulen - u))) {
                    /* If the eventually required minimum size outgrows the
                     * available space, we need to grow. */
                    const UV o = d - (U8*)SvPVX_const(dest);

                    /* If someone uppercases one million U+03B0s we SvGROW()
                     * one million times.  Or we could try guessing how much to
                     * allocate without allocating too much.  Such is life.
                     * See corresponding comment in lc code for another option
                     * */
                    SvGROW(dest, min);
                    d = (U8*)SvPVX(dest) + o;
                }
                Copy(tmpbuf, d, ulen, U8);
                d += ulen;
            }
            s += u;
	}
	if (in_iota_subscript) {
	    CAT_UNI_TO_UTF8_TWO_BYTE(d, GREEK_CAPITAL_LETTER_IOTA);
	}
	SvUTF8_on(dest);
	*d = '\0';

	SvCUR_set(dest, d - (U8*)SvPVX_const(dest));
	if (tainted) {
	    TAINT;
	    SvTAINTED_on(dest);
	}
    }
    else {	/* Not UTF-8 */
	if (len) {
	    const U8 *const send = s + len;

	    /* Use locale casing if in locale; regular style if not treating
	     * latin1 as having case; otherwise the latin1 casing.  Do the
	     * whole thing in a tight loop, for speed, */
	    if (IN_LOCALE_RUNTIME) {
		TAINT;
		SvTAINTED_on(dest);
		for (; s < send; d++, s++)
		    *d = toUPPER_LC(*s);
	    }
	    else if (! IN_UNI_8_BIT) {
		for (; s < send; d++, s++) {
		    *d = toUPPER(*s);
		}
	    }
	    else {
		for (; s < send; d++, s++) {
		    *d = toUPPER_LATIN1_MOD(*s);
		    if (LIKELY(*d != LATIN_SMALL_LETTER_Y_WITH_DIAERESIS)) continue;

		    /* The mainstream case is the tight loop above.  To avoid
		     * extra tests in that, all three characters that require
		     * special handling are mapped by the MOD to the one tested
		     * just above.  
		     * Use the source to distinguish between the three cases */

		    if (*s == LATIN_SMALL_LETTER_SHARP_S) {

			/* uc() of this requires 2 characters, but they are
			 * ASCII.  If not enough room, grow the string */
			if (SvLEN(dest) < ++min) {	
			    const UV o = d - (U8*)SvPVX_const(dest);
			    SvGROW(dest, min);
			    d = (U8*)SvPVX(dest) + o;
			}
			*d++ = 'S'; *d = 'S'; /* upper case is 'SS' */
			continue;   /* Back to the tight loop; still in ASCII */
		    }

		    /* The other two special handling characters have their
		     * upper cases outside the latin1 range, hence need to be
		     * in UTF-8, so the whole result needs to be in UTF-8.  So,
		     * here we are somewhere in the middle of processing a
		     * non-UTF-8 string, and realize that we will have to convert
		     * the whole thing to UTF-8.  What to do?  There are
		     * several possibilities.  The simplest to code is to
		     * convert what we have so far, set a flag, and continue on
		     * in the loop.  The flag would be tested each time through
		     * the loop, and if set, the next character would be
		     * converted to UTF-8 and stored.  But, I (khw) didn't want
		     * to slow down the mainstream case at all for this fairly
		     * rare case, so I didn't want to add a test that didn't
		     * absolutely have to be there in the loop, besides the
		     * possibility that it would get too complicated for
		     * optimizers to deal with.  Another possibility is to just
		     * give up, convert the source to UTF-8, and restart the
		     * function that way.  Another possibility is to convert
		     * both what has already been processed and what is yet to
		     * come separately to UTF-8, then jump into the loop that
		     * handles UTF-8.  But the most efficient time-wise of the
		     * ones I could think of is what follows, and turned out to
		     * not require much extra code.  */

		    /* Convert what we have so far into UTF-8, telling the
		     * function that we know it should be converted, and to
		     * allow extra space for what we haven't processed yet.
		     * Assume the worst case space requirements for converting
		     * what we haven't processed so far: that it will require
		     * two bytes for each remaining source character, plus the
		     * NUL at the end.  This may cause the string pointer to
		     * move, so re-find it. */

		    len = d - (U8*)SvPVX_const(dest);
		    SvCUR_set(dest, len);
		    len = sv_utf8_upgrade_flags_grow(dest,
						SV_GMAGIC|SV_FORCE_UTF8_UPGRADE,
						(send -s) * 2 + 1);
		    d = (U8*)SvPVX(dest) + len;

		    /* Now process the remainder of the source, converting to
		     * upper and UTF-8.  If a resulting byte is invariant in
		     * UTF-8, output it as-is, otherwise convert to UTF-8 and
		     * append it to the output. */
		    for (; s < send; s++) {
			(void) _to_upper_title_latin1(*s, d, &len, 'S');
			d += len;
		    }

		    /* Here have processed the whole source; no need to continue
		     * with the outer loop.  Each character has been converted
		     * to upper case and converted to UTF-8 */

		    break;
		} /* End of processing all latin1-style chars */
	    } /* End of processing all chars */
	} /* End of source is not empty */

	if (source != dest) {
	    *d = '\0';  /* Here d points to 1 after last char, add NUL */
	    SvCUR_set(dest, d - (U8*)SvPVX_const(dest));
	}
    } /* End of isn't utf8 */
    if (dest != source && SvTAINTED(source))
	SvTAINT(dest);
    SvSETMAGIC(dest);
    RETURN;
}

PP(pp_lc)
{
    dVAR;
    dSP;
    SV *source = TOPs;
    STRLEN len;
    STRLEN min;
    SV *dest;
    const U8 *s;
    U8 *d;

    SvGETMAGIC(source);

    if (SvPADTMP(source) && !SvREADONLY(source) && !SvAMAGIC(source)
	&& SvTEMP(source) && !DO_UTF8(source)) {

	/* We can convert in place, as lowercasing anything in the latin1 range
	 * (or else DO_UTF8 would have been on) doesn't lengthen it */
	dest = source;
	s = d = (U8*)SvPV_force_nomg(source, len);
	min = len + 1;
    } else {
	dTARGET;

	dest = TARG;

	/* The old implementation would copy source into TARG at this point.
	   This had the side effect that if source was undef, TARG was now
	   an undefined SV with PADTMP set, and they don't warn inside
	   sv_2pv_flags(). However, we're now getting the PV direct from
	   source, which doesn't have PADTMP set, so it would warn. Hence the
	   little games.  */

	if (SvOK(source)) {
	    s = (const U8*)SvPV_nomg_const(source, len);
	} else {
	    if (ckWARN(WARN_UNINITIALIZED))
		report_uninit(source);
	    s = (const U8*)"";
	    len = 0;
	}
	min = len + 1;

	SvUPGRADE(dest, SVt_PV);
	d = (U8*)SvGROW(dest, min);
	(void)SvPOK_only(dest);

	SETs(dest);
    }

    /* Overloaded values may have toggled the UTF-8 flag on source, so we need
       to check DO_UTF8 again here.  */

    if (DO_UTF8(source)) {
	const U8 *const send = s + len;
	U8 tmpbuf[UTF8_MAXBYTES_CASE+1];
	bool tainted = FALSE;

	while (s < send) {
	    const STRLEN u = UTF8SKIP(s);
	    STRLEN ulen;

	    _to_utf8_lower_flags(s, tmpbuf, &ulen,
				 cBOOL(IN_LOCALE_RUNTIME), &tainted);

	    /* Here is where we would do context-sensitive actions.  See the
	     * commit message for this comment for why there isn't any */

	    if (ulen > u && (SvLEN(dest) < (min += ulen - u))) {

		/* If the eventually required minimum size outgrows the
		 * available space, we need to grow. */
		const UV o = d - (U8*)SvPVX_const(dest);

		/* If someone lowercases one million U+0130s we SvGROW() one
		 * million times.  Or we could try guessing how much to
		 * allocate without allocating too much.  Such is life.
		 * Another option would be to grow an extra byte or two more
		 * each time we need to grow, which would cut down the million
		 * to 500K, with little waste */
		SvGROW(dest, min);
		d = (U8*)SvPVX(dest) + o;
	    }

	    /* Copy the newly lowercased letter to the output buffer we're
	     * building */
	    Copy(tmpbuf, d, ulen, U8);
	    d += ulen;
	    s += u;
	}   /* End of looping through the source string */
	SvUTF8_on(dest);
	*d = '\0';
	SvCUR_set(dest, d - (U8*)SvPVX_const(dest));
	if (tainted) {
	    TAINT;
	    SvTAINTED_on(dest);
	}
    } else {	/* Not utf8 */
	if (len) {
	    const U8 *const send = s + len;

	    /* Use locale casing if in locale; regular style if not treating
	     * latin1 as having case; otherwise the latin1 casing.  Do the
	     * whole thing in a tight loop, for speed, */
	    if (IN_LOCALE_RUNTIME) {
		TAINT;
		SvTAINTED_on(dest);
		for (; s < send; d++, s++)
		    *d = toLOWER_LC(*s);
	    }
	    else if (! IN_UNI_8_BIT) {
		for (; s < send; d++, s++) {
		    *d = toLOWER(*s);
		}
	    }
	    else {
		for (; s < send; d++, s++) {
		    *d = toLOWER_LATIN1(*s);
		}
	    }
	}
	if (source != dest) {
	    *d = '\0';
	    SvCUR_set(dest, d - (U8*)SvPVX_const(dest));
	}
    }
    if (dest != source && SvTAINTED(source))
	SvTAINT(dest);
    SvSETMAGIC(dest);
    RETURN;
}

PP(pp_quotemeta)
{
    dVAR; dSP; dTARGET;
    SV * const sv = TOPs;
    STRLEN len;
    register const char *s = SvPV_const(sv,len);

    SvUTF8_off(TARG);				/* decontaminate */
    if (len) {
	register char *d;
	SvUPGRADE(TARG, SVt_PV);
	SvGROW(TARG, (len * 2) + 1);
	d = SvPVX(TARG);
	if (DO_UTF8(sv)) {
	    while (len) {
		STRLEN ulen = UTF8SKIP(s);
		bool to_quote = FALSE;

		if (UTF8_IS_INVARIANT(*s)) {
		    if (_isQUOTEMETA(*s)) {
			to_quote = TRUE;
		    }
		}
		else if (UTF8_IS_DOWNGRADEABLE_START(*s)) {

		    /* In locale, we quote all non-ASCII Latin1 chars.
		     * Otherwise use the quoting rules */
		    if (IN_LOCALE_RUNTIME
			|| _isQUOTEMETA(TWO_BYTE_UTF8_TO_UNI(*s, *(s + 1))))
		    {
			to_quote = TRUE;
		    }
		}
		else if (_is_utf8_quotemeta((U8 *) s)) {
		    to_quote = TRUE;
		}

		if (to_quote) {
		    *d++ = '\\';
		}
		if (ulen > len)
		    ulen = len;
		len -= ulen;
		while (ulen--)
		    *d++ = *s++;
	    }
	    SvUTF8_on(TARG);
	}
	else if (IN_UNI_8_BIT) {
	    while (len--) {
		if (_isQUOTEMETA(*s))
		    *d++ = '\\';
		*d++ = *s++;
	    }
	}
	else {
	    /* For non UNI_8_BIT (and hence in locale) just quote all \W
	     * including everything above ASCII */
	    while (len--) {
		if (!isWORDCHAR_A(*s))
		    *d++ = '\\';
		*d++ = *s++;
	    }
	}
	*d = '\0';
	SvCUR_set(TARG, d - SvPVX_const(TARG));
	(void)SvPOK_only_UTF8(TARG);
    }
    else
	sv_setpvn(TARG, s, len);
    SETTARG;
    RETURN;
}

PP(pp_fc)
{
    dVAR;
    dTARGET;
    dSP;
    SV *source = TOPs;
    STRLEN len;
    STRLEN min;
    SV *dest;
    const U8 *s;
    const U8 *send;
    U8 *d;
    U8 tmpbuf[UTF8_MAXBYTES * UTF8_MAX_FOLD_CHAR_EXPAND + 1];
    const bool full_folding = TRUE;
    const U8 flags = ( full_folding      ? FOLD_FLAGS_FULL   : 0 )
                   | ( IN_LOCALE_RUNTIME ? FOLD_FLAGS_LOCALE : 0 );

    /* This is a facsimile of pp_lc, but with a thousand bugs thanks to me.
     * You are welcome(?) -Hugmeir
     */

    SvGETMAGIC(source);

    dest = TARG;

    if (SvOK(source)) {
        s = (const U8*)SvPV_nomg_const(source, len);
    } else {
        if (ckWARN(WARN_UNINITIALIZED))
	    report_uninit(source);
	s = (const U8*)"";
	len = 0;
    }

    min = len + 1;

    SvUPGRADE(dest, SVt_PV);
    d = (U8*)SvGROW(dest, min);
    (void)SvPOK_only(dest);

    SETs(dest);

    send = s + len;
    if (DO_UTF8(source)) { /* UTF-8 flagged string. */
        bool tainted = FALSE;
        while (s < send) {
            const STRLEN u = UTF8SKIP(s);
            STRLEN ulen;

            _to_utf8_fold_flags(s, tmpbuf, &ulen, flags, &tainted);

            if (ulen > u && (SvLEN(dest) < (min += ulen - u))) {
                const UV o = d - (U8*)SvPVX_const(dest);
                SvGROW(dest, min);
                d = (U8*)SvPVX(dest) + o;
            }

            Copy(tmpbuf, d, ulen, U8);
            d += ulen;
            s += u;
        }
        SvUTF8_on(dest);
	if (tainted) {
	    TAINT;
	    SvTAINTED_on(dest);
	}
    } /* Unflagged string */
    else if (len) {
        /* For locale, bytes, and nothing, the behavior is supposed to be the
         * same as lc().
         */
        if ( IN_LOCALE_RUNTIME ) { /* Under locale */
            TAINT;
            SvTAINTED_on(dest);
            for (; s < send; d++, s++)
                *d = toLOWER_LC(*s);
        }
        else if ( !IN_UNI_8_BIT ) { /* Under nothing, or bytes */
            for (; s < send; d++, s++)
                *d = toLOWER(*s);
        }
        else {
            /* For ASCII and the Latin-1 range, there's only two troublesome folds,
            * \x{DF} (\N{LATIN SMALL LETTER SHARP S}), which under full casefolding
            * becomes 'ss', and \x{B5} (\N{MICRO SIGN}), which under any fold becomes
            * \x{3BC} (\N{GREEK SMALL LETTER MU}) -- For the rest, the casefold is
            * their lowercase.
            */
            for (; s < send; d++, s++) {
                if (*s == MICRO_SIGN) {
                    /* \N{MICRO SIGN}'s casefold is \N{GREEK SMALL LETTER MU}, which
                    * is outside of the latin-1 range. There's a couple of ways to
                    * deal with this -- khw discusses them in pp_lc/uc, so go there :)
                    * What we do here is upgrade what we had already casefolded,
                    * then enter an inner loop that appends the rest of the characters
                    * as UTF-8.
                    */
                    len = d - (U8*)SvPVX_const(dest);
                    SvCUR_set(dest, len);
                    len = sv_utf8_upgrade_flags_grow(dest,
                                                SV_GMAGIC|SV_FORCE_UTF8_UPGRADE,
						/* The max expansion for latin1
						 * chars is 1 byte becomes 2 */
                                                (send -s) * 2 + 1);
                    d = (U8*)SvPVX(dest) + len;

                    CAT_UNI_TO_UTF8_TWO_BYTE(d, GREEK_SMALL_LETTER_MU);
                    s++;
                    for (; s < send; s++) {
                        STRLEN ulen;
                        UV fc = _to_uni_fold_flags(*s, tmpbuf, &ulen, flags);
                        if UNI_IS_INVARIANT(fc) {
                            if ( full_folding && *s == LATIN_SMALL_LETTER_SHARP_S) {
                                *d++ = 's';
                                *d++ = 's';
                            }
                            else
                                *d++ = (U8)fc;
                        }
                        else {
                            Copy(tmpbuf, d, ulen, U8);
                            d += ulen;
                        }
                    }
                    break;
                }
                else if (full_folding && *s == LATIN_SMALL_LETTER_SHARP_S) {
                    /* Under full casefolding, LATIN SMALL LETTER SHARP S becomes "ss",
                    * which may require growing the SV.
                    */
                    if (SvLEN(dest) < ++min) {
                        const UV o = d - (U8*)SvPVX_const(dest);
                        SvGROW(dest, min);
                        d = (U8*)SvPVX(dest) + o;
                     }
                    *(d)++ = 's';
                    *d = 's';
                }
                else { /* If it's not one of those two, the fold is their lower case */
                    *d = toLOWER_LATIN1(*s);
                }
             }
        }
    }
    *d = '\0';
    SvCUR_set(dest, d - (U8*)SvPVX_const(dest));

    if (SvTAINTED(source))
	SvTAINT(dest);
    SvSETMAGIC(dest);
    RETURN;
}

/* Arrays. */

PP(pp_aslice)
{
    dVAR; dSP; dMARK; dORIGMARK;
    register AV *const av = MUTABLE_AV(POPs);
    register const I32 lval = (PL_op->op_flags & OPf_MOD || LVRET);

    if (SvTYPE(av) == SVt_PVAV) {
	const bool localizing = PL_op->op_private & OPpLVAL_INTRO;
	bool can_preserve = FALSE;

	if (localizing) {
	    MAGIC *mg;
	    HV *stash;

	    can_preserve = SvCANEXISTDELETE(av);
	}

	if (lval && localizing) {
	    register SV **svp;
	    I32 max = -1;
	    for (svp = MARK + 1; svp <= SP; svp++) {
		const I32 elem = SvIV(*svp);
		if (elem > max)
		    max = elem;
	    }
	    if (max > AvMAX(av))
		av_extend(av, max);
	}

	while (++MARK <= SP) {
	    register SV **svp;
	    I32 elem = SvIV(*MARK);
	    bool preeminent = TRUE;

	    if (localizing && can_preserve) {
		/* If we can determine whether the element exist,
		 * Try to preserve the existenceness of a tied array
		 * element by using EXISTS and DELETE if possible.
		 * Fallback to FETCH and STORE otherwise. */
		preeminent = av_exists(av, elem);
	    }

	    svp = av_fetch(av, elem, lval);
	    if (lval) {
		if (!svp || *svp == &PL_sv_undef)
		    DIE(aTHX_ PL_no_aelem, elem);
		if (localizing) {
		    if (preeminent)
			save_aelem(av, elem, svp);
		    else
			SAVEADELETE(av, elem);
		}
	    }
	    *MARK = svp ? *svp : &PL_sv_undef;
	}
    }
    if (GIMME != G_ARRAY) {
	MARK = ORIGMARK;
	*++MARK = SP > ORIGMARK ? *SP : &PL_sv_undef;
	SP = MARK;
    }
    RETURN;
}

/* Smart dereferencing for keys, values and each */
PP(pp_rkeys)
{
    dVAR;
    dSP;
    dPOPss;

    SvGETMAGIC(sv);

    if (
         !SvROK(sv)
      || (sv = SvRV(sv),
            (SvTYPE(sv) != SVt_PVHV && SvTYPE(sv) != SVt_PVAV)
          || SvOBJECT(sv)
         )
    ) {
	DIE(aTHX_
	   "Type of argument to %s must be unblessed hashref or arrayref",
	    PL_op_desc[PL_op->op_type] );
    }

    if (PL_op->op_flags & OPf_SPECIAL && SvTYPE(sv) == SVt_PVAV)
	DIE(aTHX_
	   "Can't modify %s in %s",
	    PL_op_desc[PL_op->op_type], PL_op_desc[PL_op->op_next->op_type]
	);

    /* Delegate to correct function for op type */
    PUSHs(sv);
    if (PL_op->op_type == OP_RKEYS || PL_op->op_type == OP_RVALUES) {
	return (SvTYPE(sv) == SVt_PVHV) ? Perl_do_kv(aTHX) : Perl_pp_akeys(aTHX);
    }
    else {
	return (SvTYPE(sv) == SVt_PVHV) ? Perl_pp_each(aTHX) : Perl_pp_aeach(aTHX);
    }
}

PP(pp_aeach)
{
    dVAR;
    dSP;
    AV *array = MUTABLE_AV(POPs);
    const I32 gimme = GIMME_V;
    IV *iterp = Perl_av_iter_p(aTHX_ array);
    const IV current = (*iterp)++;

    if (current > av_len(array)) {
	*iterp = 0;
	if (gimme == G_SCALAR)
	    RETPUSHUNDEF;
	else
	    RETURN;
    }

    EXTEND(SP, 2);
    mPUSHi(current);
    if (gimme == G_ARRAY) {
	SV **const element = av_fetch(array, current, 0);
        PUSHs(element ? *element : &PL_sv_undef);
    }
    RETURN;
}

PP(pp_akeys)
{
    dVAR;
    dSP;
    AV *array = MUTABLE_AV(POPs);
    const I32 gimme = GIMME_V;

    *Perl_av_iter_p(aTHX_ array) = 0;

    if (gimme == G_SCALAR) {
	dTARGET;
	PUSHi(av_len(array) + 1);
    }
    else if (gimme == G_ARRAY) {
        IV n = Perl_av_len(aTHX_ array);
        IV i;

        EXTEND(SP, n + 1);

	if (PL_op->op_type == OP_AKEYS || PL_op->op_type == OP_RKEYS) {
	    for (i = 0;  i <= n;  i++) {
		mPUSHi(i);
	    }
	}
	else {
	    for (i = 0;  i <= n;  i++) {
		SV *const *const elem = Perl_av_fetch(aTHX_ array, i, 0);
		PUSHs(elem ? *elem : &PL_sv_undef);
	    }
	}
    }
    RETURN;
}

/* Associative arrays. */

PP(pp_each)
{
    dVAR;
    dSP;
    HV * hash = MUTABLE_HV(POPs);
    HE *entry;
    const I32 gimme = GIMME_V;

    PUTBACK;
    /* might clobber stack_sp */
    entry = hv_iternext(hash);
    SPAGAIN;

    EXTEND(SP, 2);
    if (entry) {
	SV* const sv = hv_iterkeysv(entry);
	PUSHs(sv);	/* won't clobber stack_sp */
	if (gimme == G_ARRAY) {
	    SV *val;
	    PUTBACK;
	    /* might clobber stack_sp */
	    val = hv_iterval(hash, entry);
	    SPAGAIN;
	    PUSHs(val);
	}
    }
    else if (gimme == G_SCALAR)
	RETPUSHUNDEF;

    RETURN;
}

STATIC OP *
S_do_delete_local(pTHX)
{
    dVAR;
    dSP;
    const I32 gimme = GIMME_V;
    const MAGIC *mg;
    HV *stash;

    if (PL_op->op_private & OPpSLICE) {
	dMARK; dORIGMARK;
	SV * const osv = POPs;
	const bool tied = SvRMAGICAL(osv)
			    && mg_find((const SV *)osv, PERL_MAGIC_tied);
	const bool can_preserve = SvCANEXISTDELETE(osv)
				    || mg_find((const SV *)osv, PERL_MAGIC_env);
	const U32 type = SvTYPE(osv);
	if (type == SVt_PVHV) {			/* hash element */
	    HV * const hv = MUTABLE_HV(osv);
	    while (++MARK <= SP) {
		SV * const keysv = *MARK;
		SV *sv = NULL;
		bool preeminent = TRUE;
		if (can_preserve)
		    preeminent = hv_exists_ent(hv, keysv, 0);
		if (tied) {
		    HE *he = hv_fetch_ent(hv, keysv, 1, 0);
		    if (he)
			sv = HeVAL(he);
		    else
			preeminent = FALSE;
		}
		else {
		    sv = hv_delete_ent(hv, keysv, 0, 0);
		    SvREFCNT_inc_simple_void(sv); /* De-mortalize */
		}
		if (preeminent) {
		    save_helem_flags(hv, keysv, &sv, SAVEf_KEEPOLDELEM);
		    if (tied) {
			*MARK = sv_mortalcopy(sv);
			mg_clear(sv);
		    } else
			*MARK = sv;
		}
		else {
		    SAVEHDELETE(hv, keysv);
		    *MARK = &PL_sv_undef;
		}
	    }
	}
	else if (type == SVt_PVAV) {                  /* array element */
	    if (PL_op->op_flags & OPf_SPECIAL) {
		AV * const av = MUTABLE_AV(osv);
		while (++MARK <= SP) {
		    I32 idx = SvIV(*MARK);
		    SV *sv = NULL;
		    bool preeminent = TRUE;
		    if (can_preserve)
			preeminent = av_exists(av, idx);
		    if (tied) {
			SV **svp = av_fetch(av, idx, 1);
			if (svp)
			    sv = *svp;
			else
			    preeminent = FALSE;
		    }
		    else {
			sv = av_delete(av, idx, 0);
		        SvREFCNT_inc_simple_void(sv); /* De-mortalize */
		    }
		    if (preeminent) {
		        save_aelem_flags(av, idx, &sv, SAVEf_KEEPOLDELEM);
			if (tied) {
			    *MARK = sv_mortalcopy(sv);
			    mg_clear(sv);
			} else
			    *MARK = sv;
		    }
		    else {
		        SAVEADELETE(av, idx);
		        *MARK = &PL_sv_undef;
		    }
		}
	    }
	}
	else
	    DIE(aTHX_ "Not a HASH reference");
	if (gimme == G_VOID)
	    SP = ORIGMARK;
	else if (gimme == G_SCALAR) {
	    MARK = ORIGMARK;
	    if (SP > MARK)
		*++MARK = *SP;
	    else
		*++MARK = &PL_sv_undef;
	    SP = MARK;
	}
    }
    else {
	SV * const keysv = POPs;
	SV * const osv   = POPs;
	const bool tied = SvRMAGICAL(osv)
			    && mg_find((const SV *)osv, PERL_MAGIC_tied);
	const bool can_preserve = SvCANEXISTDELETE(osv)
				    || mg_find((const SV *)osv, PERL_MAGIC_env);
	const U32 type = SvTYPE(osv);
	SV *sv = NULL;
	if (type == SVt_PVHV) {
	    HV * const hv = MUTABLE_HV(osv);
	    bool preeminent = TRUE;
	    if (can_preserve)
		preeminent = hv_exists_ent(hv, keysv, 0);
	    if (tied) {
		HE *he = hv_fetch_ent(hv, keysv, 1, 0);
		if (he)
		    sv = HeVAL(he);
		else
		    preeminent = FALSE;
	    }
	    else {
		sv = hv_delete_ent(hv, keysv, 0, 0);
		SvREFCNT_inc_simple_void(sv); /* De-mortalize */
	    }
	    if (preeminent) {
		save_helem_flags(hv, keysv, &sv, SAVEf_KEEPOLDELEM);
		if (tied) {
		    SV *nsv = sv_mortalcopy(sv);
		    mg_clear(sv);
		    sv = nsv;
		}
	    }
	    else
		SAVEHDELETE(hv, keysv);
	}
	else if (type == SVt_PVAV) {
	    if (PL_op->op_flags & OPf_SPECIAL) {
		AV * const av = MUTABLE_AV(osv);
		I32 idx = SvIV(keysv);
		bool preeminent = TRUE;
		if (can_preserve)
		    preeminent = av_exists(av, idx);
		if (tied) {
		    SV **svp = av_fetch(av, idx, 1);
		    if (svp)
			sv = *svp;
		    else
			preeminent = FALSE;
		}
		else {
		    sv = av_delete(av, idx, 0);
		    SvREFCNT_inc_simple_void(sv); /* De-mortalize */
		}
		if (preeminent) {
		    save_aelem_flags(av, idx, &sv, SAVEf_KEEPOLDELEM);
		    if (tied) {
			SV *nsv = sv_mortalcopy(sv);
			mg_clear(sv);
			sv = nsv;
		    }
		}
		else
		    SAVEADELETE(av, idx);
	    }
	    else
		DIE(aTHX_ "panic: avhv_delete no longer supported");
	}
	else
	    DIE(aTHX_ "Not a HASH reference");
	if (!sv)
	    sv = &PL_sv_undef;
	if (gimme != G_VOID)
	    PUSHs(sv);
    }

    RETURN;
}

PP(pp_delete)
{
    dVAR;
    dSP;
    I32 gimme;
    I32 discard;

    if (PL_op->op_private & OPpLVAL_INTRO)
	return do_delete_local();

    gimme = GIMME_V;
    discard = (gimme == G_VOID) ? G_DISCARD : 0;

    if (PL_op->op_private & OPpSLICE) {
	dMARK; dORIGMARK;
	HV * const hv = MUTABLE_HV(POPs);
	const U32 hvtype = SvTYPE(hv);
	if (hvtype == SVt_PVHV) {			/* hash element */
	    while (++MARK <= SP) {
		SV * const sv = hv_delete_ent(hv, *MARK, discard, 0);
		*MARK = sv ? sv : &PL_sv_undef;
	    }
	}
	else if (hvtype == SVt_PVAV) {                  /* array element */
            if (PL_op->op_flags & OPf_SPECIAL) {
                while (++MARK <= SP) {
                    SV * const sv = av_delete(MUTABLE_AV(hv), SvIV(*MARK), discard);
                    *MARK = sv ? sv : &PL_sv_undef;
                }
            }
	}
	else
	    DIE(aTHX_ "Not a HASH reference");
	if (discard)
	    SP = ORIGMARK;
	else if (gimme == G_SCALAR) {
	    MARK = ORIGMARK;
	    if (SP > MARK)
		*++MARK = *SP;
	    else
		*++MARK = &PL_sv_undef;
	    SP = MARK;
	}
    }
    else {
	SV *keysv = POPs;
	HV * const hv = MUTABLE_HV(POPs);
	SV *sv = NULL;
	if (SvTYPE(hv) == SVt_PVHV)
	    sv = hv_delete_ent(hv, keysv, discard, 0);
	else if (SvTYPE(hv) == SVt_PVAV) {
	    if (PL_op->op_flags & OPf_SPECIAL)
		sv = av_delete(MUTABLE_AV(hv), SvIV(keysv), discard);
	    else
		DIE(aTHX_ "panic: avhv_delete no longer supported");
	}
	else
	    DIE(aTHX_ "Not a HASH reference");
	if (!sv)
	    sv = &PL_sv_undef;
	if (!discard)
	    PUSHs(sv);
    }
    RETURN;
}

PP(pp_exists)
{
    dVAR;
    dSP;
    SV *tmpsv;
    HV *hv;

    if (PL_op->op_private & OPpEXISTS_SUB) {
	GV *gv;
	SV * const sv = POPs;
	CV * const cv = sv_2cv(sv, &hv, &gv, 0);
	if (cv)
	    RETPUSHYES;
	if (gv && isGV(gv) && GvCV(gv) && !GvCVGEN(gv))
	    RETPUSHYES;
	RETPUSHNO;
    }
    tmpsv = POPs;
    hv = MUTABLE_HV(POPs);
    if (SvTYPE(hv) == SVt_PVHV) {
	if (hv_exists_ent(hv, tmpsv, 0))
	    RETPUSHYES;
    }
    else if (SvTYPE(hv) == SVt_PVAV) {
	if (PL_op->op_flags & OPf_SPECIAL) {		/* array element */
	    if (av_exists(MUTABLE_AV(hv), SvIV(tmpsv)))
		RETPUSHYES;
	}
    }
    else {
	DIE(aTHX_ "Not a HASH reference");
    }
    RETPUSHNO;
}

PP(pp_hslice)
{
    dVAR; dSP; dMARK; dORIGMARK;
    register HV * const hv = MUTABLE_HV(POPs);
    register const I32 lval = (PL_op->op_flags & OPf_MOD || LVRET);
    const bool localizing = PL_op->op_private & OPpLVAL_INTRO;
    bool can_preserve = FALSE;

    if (localizing) {
        MAGIC *mg;
        HV *stash;

	if (SvCANEXISTDELETE(hv) || mg_find((const SV *)hv, PERL_MAGIC_env))
	    can_preserve = TRUE;
    }

    while (++MARK <= SP) {
        SV * const keysv = *MARK;
        SV **svp;
        HE *he;
        bool preeminent = TRUE;

        if (localizing && can_preserve) {
	    /* If we can determine whether the element exist,
             * try to preserve the existenceness of a tied hash
             * element by using EXISTS and DELETE if possible.
             * Fallback to FETCH and STORE otherwise. */
            preeminent = hv_exists_ent(hv, keysv, 0);
        }

        he = hv_fetch_ent(hv, keysv, lval, 0);
        svp = he ? &HeVAL(he) : NULL;

        if (lval) {
            if (!svp || !*svp || *svp == &PL_sv_undef) {
                DIE(aTHX_ PL_no_helem_sv, SVfARG(keysv));
            }
            if (localizing) {
		if (HvNAME_get(hv) && isGV(*svp))
		    save_gp(MUTABLE_GV(*svp), !(PL_op->op_flags & OPf_SPECIAL));
		else if (preeminent)
		    save_helem_flags(hv, keysv, svp,
			 (PL_op->op_flags & OPf_SPECIAL) ? 0 : SAVEf_SETMAGIC);
		else
		    SAVEHDELETE(hv, keysv);
            }
        }
        *MARK = svp && *svp ? *svp : &PL_sv_undef;
    }
    if (GIMME != G_ARRAY) {
	MARK = ORIGMARK;
	*++MARK = SP > ORIGMARK ? *SP : &PL_sv_undef;
	SP = MARK;
    }
    RETURN;
}

/* List operators. */

PP(pp_list)
{
    dVAR; dSP; dMARK;
    if (GIMME != G_ARRAY) {
	if (++MARK <= SP)
	    *MARK = *SP;		/* unwanted list, return last item */
	else
	    *MARK = &PL_sv_undef;
	SP = MARK;
    }
    RETURN;
}

PP(pp_lslice)
{
    dVAR;
    dSP;
    SV ** const lastrelem = PL_stack_sp;
    SV ** const lastlelem = PL_stack_base + POPMARK;
    SV ** const firstlelem = PL_stack_base + POPMARK + 1;
    register SV ** const firstrelem = lastlelem + 1;
    I32 is_something_there = FALSE;

    register const I32 max = lastrelem - lastlelem;
    register SV **lelem;

    if (GIMME != G_ARRAY) {
	I32 ix = SvIV(*lastlelem);
	if (ix < 0)
	    ix += max;
	if (ix < 0 || ix >= max)
	    *firstlelem = &PL_sv_undef;
	else
	    *firstlelem = firstrelem[ix];
	SP = firstlelem;
	RETURN;
    }

    if (max == 0) {
	SP = firstlelem - 1;
	RETURN;
    }

    for (lelem = firstlelem; lelem <= lastlelem; lelem++) {
	I32 ix = SvIV(*lelem);
	if (ix < 0)
	    ix += max;
	if (ix < 0 || ix >= max)
	    *lelem = &PL_sv_undef;
	else {
	    is_something_there = TRUE;
	    if (!(*lelem = firstrelem[ix]))
		*lelem = &PL_sv_undef;
	}
    }
    if (is_something_there)
	SP = lastlelem;
    else
	SP = firstlelem - 1;
    RETURN;
}

PP(pp_anonlist)
{
    dVAR; dSP; dMARK; dORIGMARK;
    const I32 items = SP - MARK;
    SV * const av = MUTABLE_SV(av_make(items, MARK+1));
    SP = ORIGMARK;		/* av_make() might realloc stack_sp */
    mXPUSHs((PL_op->op_flags & OPf_SPECIAL)
	    ? newRV_noinc(av) : av);
    RETURN;
}

PP(pp_anonhash)
{
    dVAR; dSP; dMARK; dORIGMARK;
    HV* const hv = newHV();

    while (MARK < SP) {
	SV * const key = *++MARK;
	SV * const val = newSV(0);
	if (MARK < SP)
	    sv_setsv(val, *++MARK);
	else
	    Perl_ck_warner(aTHX_ packWARN(WARN_MISC), "Odd number of elements in anonymous hash");
	(void)hv_store_ent(hv,key,val,0);
    }
    SP = ORIGMARK;
    mXPUSHs((PL_op->op_flags & OPf_SPECIAL)
	    ? newRV_noinc(MUTABLE_SV(hv)) : MUTABLE_SV(hv));
    RETURN;
}

static AV *
S_deref_plain_array(pTHX_ AV *ary)
{
    if (SvTYPE(ary) == SVt_PVAV) return ary;
    SvGETMAGIC((SV *)ary);
    if (!SvROK(ary) || SvTYPE(SvRV(ary)) != SVt_PVAV)
	Perl_die(aTHX_ "Not an ARRAY reference");
    else if (SvOBJECT(SvRV(ary)))
	Perl_die(aTHX_ "Not an unblessed ARRAY reference");
    return (AV *)SvRV(ary);
}

#if defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
# define DEREF_PLAIN_ARRAY(ary)       \
   ({                                  \
     AV *aRrRay = ary;                  \
     SvTYPE(aRrRay) == SVt_PVAV          \
      ? aRrRay                            \
      : S_deref_plain_array(aTHX_ aRrRay); \
   })
#else
# define DEREF_PLAIN_ARRAY(ary)            \
   (                                        \
     PL_Sv = (SV *)(ary),                    \
     SvTYPE(PL_Sv) == SVt_PVAV                \
      ? (AV *)PL_Sv                            \
      : S_deref_plain_array(aTHX_ (AV *)PL_Sv)  \
   )
#endif

PP(pp_splice)
{
    dVAR; dSP; dMARK; dORIGMARK;
    int num_args = (SP - MARK);
    register AV *ary = DEREF_PLAIN_ARRAY(MUTABLE_AV(*++MARK));
    register SV **src;
    register SV **dst;
    register I32 i;
    register I32 offset;
    register I32 length;
    I32 newlen;
    I32 after;
    I32 diff;
    const MAGIC * const mg = SvTIED_mg((const SV *)ary, PERL_MAGIC_tied);

    if (mg) {
	return Perl_tied_method(aTHX_ "SPLICE", mark - 1, MUTABLE_SV(ary), mg,
				    GIMME_V | TIED_METHOD_ARGUMENTS_ON_STACK,
				    sp - mark);
    }

    SP++;

    if (++MARK < SP) {
	offset = i = SvIV(*MARK);
	if (offset < 0)
	    offset += AvFILLp(ary) + 1;
	if (offset < 0)
	    DIE(aTHX_ PL_no_aelem, i);
	if (++MARK < SP) {
	    length = SvIVx(*MARK++);
	    if (length < 0) {
		length += AvFILLp(ary) - offset + 1;
		if (length < 0)
		    length = 0;
	    }
	}
	else
	    length = AvMAX(ary) + 1;		/* close enough to infinity */
    }
    else {
	offset = 0;
	length = AvMAX(ary) + 1;
    }
    if (offset > AvFILLp(ary) + 1) {
	if (num_args > 2)
	    Perl_ck_warner(aTHX_ packWARN(WARN_MISC), "splice() offset past end of array" );
	offset = AvFILLp(ary) + 1;
    }
    after = AvFILLp(ary) + 1 - (offset + length);
    if (after < 0) {				/* not that much array */
	length += after;			/* offset+length now in array */
	after = 0;
	if (!AvALLOC(ary))
	    av_extend(ary, 0);
    }

    /* At this point, MARK .. SP-1 is our new LIST */

    newlen = SP - MARK;
    diff = newlen - length;
    if (newlen && !AvREAL(ary) && AvREIFY(ary))
	av_reify(ary);

    /* make new elements SVs now: avoid problems if they're from the array */
    for (dst = MARK, i = newlen; i; i--) {
        SV * const h = *dst;
	*dst++ = newSVsv(h);
    }

    if (diff < 0) {				/* shrinking the area */
	SV **tmparyval = NULL;
	if (newlen) {
	    Newx(tmparyval, newlen, SV*);	/* so remember insertion */
	    Copy(MARK, tmparyval, newlen, SV*);
	}

	MARK = ORIGMARK + 1;
	if (GIMME == G_ARRAY) {			/* copy return vals to stack */
	    MEXTEND(MARK, length);
	    Copy(AvARRAY(ary)+offset, MARK, length, SV*);
	    if (AvREAL(ary)) {
		EXTEND_MORTAL(length);
		for (i = length, dst = MARK; i; i--) {
		    sv_2mortal(*dst);	/* free them eventually */
		    dst++;
		}
	    }
	    MARK += length - 1;
	}
	else {
	    *MARK = AvARRAY(ary)[offset+length-1];
	    if (AvREAL(ary)) {
		sv_2mortal(*MARK);
		for (i = length - 1, dst = &AvARRAY(ary)[offset]; i > 0; i--)
		    SvREFCNT_dec(*dst++);	/* free them now */
	    }
	}
	AvFILLp(ary) += diff;

	/* pull up or down? */

	if (offset < after) {			/* easier to pull up */
	    if (offset) {			/* esp. if nothing to pull */
		src = &AvARRAY(ary)[offset-1];
		dst = src - diff;		/* diff is negative */
		for (i = offset; i > 0; i--)	/* can't trust Copy */
		    *dst-- = *src--;
	    }
	    dst = AvARRAY(ary);
	    AvARRAY(ary) = AvARRAY(ary) - diff; /* diff is negative */
	    AvMAX(ary) += diff;
	}
	else {
	    if (after) {			/* anything to pull down? */
		src = AvARRAY(ary) + offset + length;
		dst = src + diff;		/* diff is negative */
		Move(src, dst, after, SV*);
	    }
	    dst = &AvARRAY(ary)[AvFILLp(ary)+1];
						/* avoid later double free */
	}
	i = -diff;
	while (i)
	    dst[--i] = &PL_sv_undef;
	
	if (newlen) {
 	    Copy( tmparyval, AvARRAY(ary) + offset, newlen, SV* );
	    Safefree(tmparyval);
	}
    }
    else {					/* no, expanding (or same) */
	SV** tmparyval = NULL;
	if (length) {
	    Newx(tmparyval, length, SV*);	/* so remember deletion */
	    Copy(AvARRAY(ary)+offset, tmparyval, length, SV*);
	}

	if (diff > 0) {				/* expanding */
	    /* push up or down? */
	    if (offset < after && diff <= AvARRAY(ary) - AvALLOC(ary)) {
		if (offset) {
		    src = AvARRAY(ary);
		    dst = src - diff;
		    Move(src, dst, offset, SV*);
		}
		AvARRAY(ary) = AvARRAY(ary) - diff;/* diff is positive */
		AvMAX(ary) += diff;
		AvFILLp(ary) += diff;
	    }
	    else {
		if (AvFILLp(ary) + diff >= AvMAX(ary))	/* oh, well */
		    av_extend(ary, AvFILLp(ary) + diff);
		AvFILLp(ary) += diff;

		if (after) {
		    dst = AvARRAY(ary) + AvFILLp(ary);
		    src = dst - diff;
		    for (i = after; i; i--) {
			*dst-- = *src--;
		    }
		}
	    }
	}

	if (newlen) {
	    Copy( MARK, AvARRAY(ary) + offset, newlen, SV* );
	}

	MARK = ORIGMARK + 1;
	if (GIMME == G_ARRAY) {			/* copy return vals to stack */
	    if (length) {
		Copy(tmparyval, MARK, length, SV*);
		if (AvREAL(ary)) {
		    EXTEND_MORTAL(length);
		    for (i = length, dst = MARK; i; i--) {
			sv_2mortal(*dst);	/* free them eventually */
			dst++;
		    }
		}
	    }
	    MARK += length - 1;
	}
	else if (length--) {
	    *MARK = tmparyval[length];
	    if (AvREAL(ary)) {
		sv_2mortal(*MARK);
		while (length-- > 0)
		    SvREFCNT_dec(tmparyval[length]);
	    }
	}
	else
	    *MARK = &PL_sv_undef;
	Safefree(tmparyval);
    }

    if (SvMAGICAL(ary))
	mg_set(MUTABLE_SV(ary));

    SP = MARK;
    RETURN;
}

PP(pp_push)
{
    dVAR; dSP; dMARK; dORIGMARK; dTARGET;
    register AV * const ary = DEREF_PLAIN_ARRAY(MUTABLE_AV(*++MARK));
    const MAGIC * const mg = SvTIED_mg((const SV *)ary, PERL_MAGIC_tied);

    if (mg) {
	*MARK-- = SvTIED_obj(MUTABLE_SV(ary), mg);
	PUSHMARK(MARK);
	PUTBACK;
	ENTER_with_name("call_PUSH");
	call_method("PUSH",G_SCALAR|G_DISCARD);
	LEAVE_with_name("call_PUSH");
	SPAGAIN;
    }
    else {
	PL_delaymagic = DM_DELAY;
	for (++MARK; MARK <= SP; MARK++) {
	    SV * const sv = newSV(0);
	    if (*MARK)
		sv_setsv(sv, *MARK);
	    av_store(ary, AvFILLp(ary)+1, sv);
	}
	if (PL_delaymagic & DM_ARRAY_ISA)
	    mg_set(MUTABLE_SV(ary));

	PL_delaymagic = 0;
    }
    SP = ORIGMARK;
    if (OP_GIMME(PL_op, 0) != G_VOID) {
	PUSHi( AvFILL(ary) + 1 );
    }
    RETURN;
}

PP(pp_shift)
{
    dVAR;
    dSP;
    AV * const av = PL_op->op_flags & OPf_SPECIAL
	? MUTABLE_AV(GvAV(PL_defgv)) : DEREF_PLAIN_ARRAY(MUTABLE_AV(POPs));
    SV * const sv = PL_op->op_type == OP_SHIFT ? av_shift(av) : av_pop(av);
    EXTEND(SP, 1);
    assert (sv);
    if (AvREAL(av))
	(void)sv_2mortal(sv);
    PUSHs(sv);
    RETURN;
}

PP(pp_unshift)
{
    dVAR; dSP; dMARK; dORIGMARK; dTARGET;
    register AV *ary = DEREF_PLAIN_ARRAY(MUTABLE_AV(*++MARK));
    const MAGIC * const mg = SvTIED_mg((const SV *)ary, PERL_MAGIC_tied);

    if (mg) {
	*MARK-- = SvTIED_obj(MUTABLE_SV(ary), mg);
	PUSHMARK(MARK);
	PUTBACK;
	ENTER_with_name("call_UNSHIFT");
	call_method("UNSHIFT",G_SCALAR|G_DISCARD);
	LEAVE_with_name("call_UNSHIFT");
	SPAGAIN;
    }
    else {
	register I32 i = 0;
	av_unshift(ary, SP - MARK);
	while (MARK < SP) {
	    SV * const sv = newSVsv(*++MARK);
	    (void)av_store(ary, i++, sv);
	}
    }
    SP = ORIGMARK;
    if (OP_GIMME(PL_op, 0) != G_VOID) {
	PUSHi( AvFILL(ary) + 1 );
    }
    RETURN;
}

PP(pp_reverse)
{
    dVAR; dSP; dMARK;

    if (GIMME == G_ARRAY) {
	if (PL_op->op_private & OPpREVERSE_INPLACE) {
	    AV *av;

	    /* See pp_sort() */
	    assert( MARK+1 == SP && *SP && SvTYPE(*SP) == SVt_PVAV);
	    (void)POPMARK; /* remove mark associated with ex-OP_AASSIGN */
	    av = MUTABLE_AV((*SP));
	    /* In-place reversing only happens in void context for the array
	     * assignment. We don't need to push anything on the stack. */
	    SP = MARK;

	    if (SvMAGICAL(av)) {
		I32 i, j;
		register SV *tmp = sv_newmortal();
		/* For SvCANEXISTDELETE */
		HV *stash;
		const MAGIC *mg;
		bool can_preserve = SvCANEXISTDELETE(av);

		for (i = 0, j = av_len(av); i < j; ++i, --j) {
		    register SV *begin, *end;

		    if (can_preserve) {
			if (!av_exists(av, i)) {
			    if (av_exists(av, j)) {
				register SV *sv = av_delete(av, j, 0);
				begin = *av_fetch(av, i, TRUE);
				sv_setsv_mg(begin, sv);
			    }
			    continue;
			}
			else if (!av_exists(av, j)) {
			    register SV *sv = av_delete(av, i, 0);
			    end = *av_fetch(av, j, TRUE);
			    sv_setsv_mg(end, sv);
			    continue;
			}
		    }

		    begin = *av_fetch(av, i, TRUE);
		    end   = *av_fetch(av, j, TRUE);
		    sv_setsv(tmp,      begin);
		    sv_setsv_mg(begin, end);
		    sv_setsv_mg(end,   tmp);
		}
	    }
	    else {
		SV **begin = AvARRAY(av);

		if (begin) {
		    SV **end   = begin + AvFILLp(av);

		    while (begin < end) {
			register SV * const tmp = *begin;
			*begin++ = *end;
			*end--   = tmp;
		    }
		}
	    }
	}
	else {
	    SV **oldsp = SP;
	    MARK++;
	    while (MARK < SP) {
		register SV * const tmp = *MARK;
		*MARK++ = *SP;
		*SP--   = tmp;
	    }
	    /* safe as long as stack cannot get extended in the above */
	    SP = oldsp;
	}
    }
    else {
	register char *up;
	register char *down;
	register I32 tmp;
	dTARGET;
	STRLEN len;

	SvUTF8_off(TARG);				/* decontaminate */
	if (SP - MARK > 1)
	    do_join(TARG, &PL_sv_no, MARK, SP);
	else {
	    sv_setsv(TARG, SP > MARK ? *SP : find_rundefsv());
	    if (! SvOK(TARG) && ckWARN(WARN_UNINITIALIZED))
		report_uninit(TARG);
	}

	up = SvPV_force(TARG, len);
	if (len > 1) {
	    if (DO_UTF8(TARG)) {	/* first reverse each character */
		U8* s = (U8*)SvPVX(TARG);
		const U8* send = (U8*)(s + len);
		while (s < send) {
		    if (UTF8_IS_INVARIANT(*s)) {
			s++;
			continue;
		    }
		    else {
			if (!utf8_to_uvchr_buf(s, send, 0))
			    break;
			up = (char*)s;
			s += UTF8SKIP(s);
			down = (char*)(s - 1);
			/* reverse this character */
			while (down > up) {
			    tmp = *up;
			    *up++ = *down;
			    *down-- = (char)tmp;
			}
		    }
		}
		up = SvPVX(TARG);
	    }
	    down = SvPVX(TARG) + len - 1;
	    while (down > up) {
		tmp = *up;
		*up++ = *down;
		*down-- = (char)tmp;
	    }
	    (void)SvPOK_only_UTF8(TARG);
	}
	SP = MARK + 1;
	SETTARG;
    }
    RETURN;
}

PP(pp_split)
{
    dVAR; dSP; dTARG;
    AV *ary;
    register IV limit = POPi;			/* note, negative is forever */
    SV * const sv = POPs;
    STRLEN len;
    register const char *s = SvPV_const(sv, len);
    const bool do_utf8 = DO_UTF8(sv);
    const char *strend = s + len;
    register PMOP *pm;
    register REGEXP *rx;
    register SV *dstr;
    register const char *m;
    I32 iters = 0;
    const STRLEN slen = do_utf8 ? utf8_length((U8*)s, (U8*)strend) : (STRLEN)(strend - s);
    I32 maxiters = slen + 10;
    I32 trailing_empty = 0;
    const char *orig;
    const I32 origlimit = limit;
    I32 realarray = 0;
    I32 base;
    const I32 gimme = GIMME_V;
    bool gimme_scalar;
    const I32 oldsave = PL_savestack_ix;
    U32 make_mortal = SVs_TEMP;
    bool multiline = 0;
    MAGIC *mg = NULL;

#ifdef DEBUGGING
    Copy(&LvTARGOFF(POPs), &pm, 1, PMOP*);
#else
    pm = (PMOP*)POPs;
#endif
    if (!pm || !s)
	DIE(aTHX_ "panic: pp_split, pm=%p, s=%p", pm, s);
    rx = PM_GETRE(pm);

    TAINT_IF(get_regex_charset(RX_EXTFLAGS(rx)) == REGEX_LOCALE_CHARSET &&
	     (RX_EXTFLAGS(rx) & (RXf_WHITE | RXf_SKIPWHITE)));

    RX_MATCH_UTF8_set(rx, do_utf8);

#ifdef USE_ITHREADS
    if (pm->op_pmreplrootu.op_pmtargetoff) {
	ary = GvAVn(MUTABLE_GV(PAD_SVl(pm->op_pmreplrootu.op_pmtargetoff)));
    }
#else
    if (pm->op_pmreplrootu.op_pmtargetgv) {
	ary = GvAVn(pm->op_pmreplrootu.op_pmtargetgv);
    }
#endif
    else
	ary = NULL;
    if (ary && (gimme != G_ARRAY || (pm->op_pmflags & PMf_ONCE))) {
	realarray = 1;
	PUTBACK;
	av_extend(ary,0);
	av_clear(ary);
	SPAGAIN;
	if ((mg = SvTIED_mg((const SV *)ary, PERL_MAGIC_tied))) {
	    PUSHMARK(SP);
	    XPUSHs(SvTIED_obj(MUTABLE_SV(ary), mg));
	}
	else {
	    if (!AvREAL(ary)) {
		I32 i;
		AvREAL_on(ary);
		AvREIFY_off(ary);
		for (i = AvFILLp(ary); i >= 0; i--)
		    AvARRAY(ary)[i] = &PL_sv_undef;	/* don't free mere refs */
	    }
	    /* temporarily switch stacks */
	    SAVESWITCHSTACK(PL_curstack, ary);
	    make_mortal = 0;
	}
    }
    base = SP - PL_stack_base;
    orig = s;
    if (RX_EXTFLAGS(rx) & RXf_SKIPWHITE) {
	if (do_utf8) {
	    while (*s == ' ' || is_utf8_space((U8*)s))
		s += UTF8SKIP(s);
	}
	else if (get_regex_charset(RX_EXTFLAGS(rx)) == REGEX_LOCALE_CHARSET) {
	    while (isSPACE_LC(*s))
		s++;
	}
	else {
	    while (isSPACE(*s))
		s++;
	}
    }
    if (RX_EXTFLAGS(rx) & RXf_PMf_MULTILINE) {
	multiline = 1;
    }

    gimme_scalar = gimme == G_SCALAR && !ary;

    if (!limit)
	limit = maxiters + 2;
    if (RX_EXTFLAGS(rx) & RXf_WHITE) {
	while (--limit) {
	    m = s;
	    /* this one uses 'm' and is a negative test */
	    if (do_utf8) {
		while (m < strend && !( *m == ' ' || is_utf8_space((U8*)m) )) {
		    const int t = UTF8SKIP(m);
		    /* is_utf8_space returns FALSE for malform utf8 */
		    if (strend - m < t)
			m = strend;
		    else
			m += t;
		}
	    }
	    else if (get_regex_charset(RX_EXTFLAGS(rx)) == REGEX_LOCALE_CHARSET) {
	        while (m < strend && !isSPACE_LC(*m))
		    ++m;
            } else {
                while (m < strend && !isSPACE(*m))
                    ++m;
            }  
	    if (m >= strend)
		break;

	    if (gimme_scalar) {
		iters++;
		if (m-s == 0)
		    trailing_empty++;
		else
		    trailing_empty = 0;
	    } else {
		dstr = newSVpvn_flags(s, m-s,
				      (do_utf8 ? SVf_UTF8 : 0) | make_mortal);
		XPUSHs(dstr);
	    }

	    /* skip the whitespace found last */
	    if (do_utf8)
		s = m + UTF8SKIP(m);
	    else
		s = m + 1;

	    /* this one uses 's' and is a positive test */
	    if (do_utf8) {
		while (s < strend && ( *s == ' ' || is_utf8_space((U8*)s) ))
	            s +=  UTF8SKIP(s);
	    }
	    else if (get_regex_charset(RX_EXTFLAGS(rx)) == REGEX_LOCALE_CHARSET) {
	        while (s < strend && isSPACE_LC(*s))
		    ++s;
            } else {
                while (s < strend && isSPACE(*s))
                    ++s;
            } 	    
	}
    }
    else if (RX_EXTFLAGS(rx) & RXf_START_ONLY) {
	while (--limit) {
	    for (m = s; m < strend && *m != '\n'; m++)
		;
	    m++;
	    if (m >= strend)
		break;

	    if (gimme_scalar) {
		iters++;
		if (m-s == 0)
		    trailing_empty++;
		else
		    trailing_empty = 0;
	    } else {
		dstr = newSVpvn_flags(s, m-s,
				      (do_utf8 ? SVf_UTF8 : 0) | make_mortal);
		XPUSHs(dstr);
	    }
	    s = m;
	}
    }
    else if (RX_EXTFLAGS(rx) & RXf_NULL && !(s >= strend)) {
        /*
          Pre-extend the stack, either the number of bytes or
          characters in the string or a limited amount, triggered by:

          my ($x, $y) = split //, $str;
            or
          split //, $str, $i;
        */
	if (!gimme_scalar) {
	    const U32 items = limit - 1;
	    if (items < slen)
		EXTEND(SP, items);
	    else
		EXTEND(SP, slen);
	}

        if (do_utf8) {
            while (--limit) {
                /* keep track of how many bytes we skip over */
                m = s;
                s += UTF8SKIP(s);
		if (gimme_scalar) {
		    iters++;
		    if (s-m == 0)
			trailing_empty++;
		    else
			trailing_empty = 0;
		} else {
		    dstr = newSVpvn_flags(m, s-m, SVf_UTF8 | make_mortal);

		    PUSHs(dstr);
		}

                if (s >= strend)
                    break;
            }
        } else {
            while (--limit) {
	        if (gimme_scalar) {
		    iters++;
		} else {
		    dstr = newSVpvn(s, 1);


		    if (make_mortal)
			sv_2mortal(dstr);

		    PUSHs(dstr);
		}

                s++;

                if (s >= strend)
                    break;
            }
        }
    }
    else if (do_utf8 == (RX_UTF8(rx) != 0) &&
	     (RX_EXTFLAGS(rx) & RXf_USE_INTUIT) && !RX_NPARENS(rx)
	     && (RX_EXTFLAGS(rx) & RXf_CHECK_ALL)
	     && !(RX_EXTFLAGS(rx) & RXf_ANCH)) {
	const int tail = (RX_EXTFLAGS(rx) & RXf_INTUIT_TAIL);
	SV * const csv = CALLREG_INTUIT_STRING(rx);

	len = RX_MINLENRET(rx);
	if (len == 1 && !RX_UTF8(rx) && !tail) {
	    const char c = *SvPV_nolen_const(csv);
	    while (--limit) {
		for (m = s; m < strend && *m != c; m++)
		    ;
		if (m >= strend)
		    break;
		if (gimme_scalar) {
		    iters++;
		    if (m-s == 0)
			trailing_empty++;
		    else
			trailing_empty = 0;
		} else {
		    dstr = newSVpvn_flags(s, m-s,
					  (do_utf8 ? SVf_UTF8 : 0) | make_mortal);
		    XPUSHs(dstr);
		}
		/* The rx->minlen is in characters but we want to step
		 * s ahead by bytes. */
 		if (do_utf8)
		    s = (char*)utf8_hop((U8*)m, len);
 		else
		    s = m + len; /* Fake \n at the end */
	    }
	}
	else {
	    while (s < strend && --limit &&
	      (m = fbm_instr((unsigned char*)s, (unsigned char*)strend,
			     csv, multiline ? FBMrf_MULTILINE : 0)) )
	    {
		if (gimme_scalar) {
		    iters++;
		    if (m-s == 0)
			trailing_empty++;
		    else
			trailing_empty = 0;
		} else {
		    dstr = newSVpvn_flags(s, m-s,
					  (do_utf8 ? SVf_UTF8 : 0) | make_mortal);
		    XPUSHs(dstr);
		}
		/* The rx->minlen is in characters but we want to step
		 * s ahead by bytes. */
 		if (do_utf8)
		    s = (char*)utf8_hop((U8*)m, len);
 		else
		    s = m + len; /* Fake \n at the end */
	    }
	}
    }
    else {
	maxiters += slen * RX_NPARENS(rx);
	while (s < strend && --limit)
	{
	    I32 rex_return;
	    PUTBACK;
	    rex_return = CALLREGEXEC(rx, (char*)s, (char*)strend, (char*)orig, 1 ,
				     sv, NULL, SvSCREAM(sv) ? REXEC_SCREAM : 0);
	    SPAGAIN;
	    if (rex_return == 0)
		break;
	    TAINT_IF(RX_MATCH_TAINTED(rx));
	    if (RX_MATCH_COPIED(rx) && RX_SUBBEG(rx) != orig) {
		m = s;
		s = orig;
		orig = RX_SUBBEG(rx);
		s = orig + (m - s);
		strend = s + (strend - m);
	    }
	    m = RX_OFFS(rx)[0].start + orig;

	    if (gimme_scalar) {
		iters++;
		if (m-s == 0)
		    trailing_empty++;
		else
		    trailing_empty = 0;
	    } else {
		dstr = newSVpvn_flags(s, m-s,
				      (do_utf8 ? SVf_UTF8 : 0) | make_mortal);
		XPUSHs(dstr);
	    }
	    if (RX_NPARENS(rx)) {
		I32 i;
		for (i = 1; i <= (I32)RX_NPARENS(rx); i++) {
		    s = RX_OFFS(rx)[i].start + orig;
		    m = RX_OFFS(rx)[i].end + orig;

		    /* japhy (07/27/01) -- the (m && s) test doesn't catch
		       parens that didn't match -- they should be set to
		       undef, not the empty string */
		    if (gimme_scalar) {
			iters++;
			if (m-s == 0)
			    trailing_empty++;
			else
			    trailing_empty = 0;
		    } else {
			if (m >= orig && s >= orig) {
			    dstr = newSVpvn_flags(s, m-s,
						 (do_utf8 ? SVf_UTF8 : 0)
						  | make_mortal);
			}
			else
			    dstr = &PL_sv_undef;  /* undef, not "" */
			XPUSHs(dstr);
		    }

		}
	    }
	    s = RX_OFFS(rx)[0].end + orig;
	}
    }

    if (!gimme_scalar) {
	iters = (SP - PL_stack_base) - base;
    }
    if (iters > maxiters)
	DIE(aTHX_ "Split loop");

    /* keep field after final delim? */
    if (s < strend || (iters && origlimit)) {
	if (!gimme_scalar) {
	    const STRLEN l = strend - s;
	    dstr = newSVpvn_flags(s, l, (do_utf8 ? SVf_UTF8 : 0) | make_mortal);
	    XPUSHs(dstr);
	}
	iters++;
    }
    else if (!origlimit) {
	if (gimme_scalar) {
	    iters -= trailing_empty;
	} else {
	    while (iters > 0 && (!TOPs || !SvANY(TOPs) || SvCUR(TOPs) == 0)) {
		if (TOPs && !make_mortal)
		    sv_2mortal(TOPs);
		*SP-- = &PL_sv_undef;
		iters--;
	    }
	}
    }

    PUTBACK;
    LEAVE_SCOPE(oldsave); /* may undo an earlier SWITCHSTACK */
    SPAGAIN;
    if (realarray) {
	if (!mg) {
	    if (SvSMAGICAL(ary)) {
		PUTBACK;
		mg_set(MUTABLE_SV(ary));
		SPAGAIN;
	    }
	    if (gimme == G_ARRAY) {
		EXTEND(SP, iters);
		Copy(AvARRAY(ary), SP + 1, iters, SV*);
		SP += iters;
		RETURN;
	    }
	}
	else {
	    PUTBACK;
	    ENTER_with_name("call_PUSH");
	    call_method("PUSH",G_SCALAR|G_DISCARD);
	    LEAVE_with_name("call_PUSH");
	    SPAGAIN;
	    if (gimme == G_ARRAY) {
		I32 i;
		/* EXTEND should not be needed - we just popped them */
		EXTEND(SP, iters);
		for (i=0; i < iters; i++) {
		    SV **svp = av_fetch(ary, i, FALSE);
		    PUSHs((svp) ? *svp : &PL_sv_undef);
		}
		RETURN;
	    }
	}
    }
    else {
	if (gimme == G_ARRAY)
	    RETURN;
    }

    GETTARGET;
    PUSHi(iters);
    RETURN;
}

PP(pp_once)
{
    dSP;
    SV *const sv = PAD_SVl(PL_op->op_targ);

    if (SvPADSTALE(sv)) {
	/* First time. */
	SvPADSTALE_off(sv);
	RETURNOP(cLOGOP->op_other);
    }
    RETURNOP(cLOGOP->op_next);
}

PP(pp_lock)
{
    dVAR;
    dSP;
    dTOPss;
    SV *retsv = sv;
    SvLOCK(sv);
    if (SvTYPE(retsv) == SVt_PVAV || SvTYPE(retsv) == SVt_PVHV
     || SvTYPE(retsv) == SVt_PVCV) {
	retsv = refto(retsv);
    }
    SETs(retsv);
    RETURN;
}


PP(unimplemented_op)
{
    dVAR;
    const Optype op_type = PL_op->op_type;
    /* Using OP_NAME() isn't going to be helpful here. Firstly, it doesn't cope
       with out of range op numbers - it only "special" cases op_custom.
       Secondly, as the three ops we "panic" on are padmy, mapstart and custom,
       if we get here for a custom op then that means that the custom op didn't
       have an implementation. Given that OP_NAME() looks up the custom op
       by its pp_addr, likely it will return NULL, unless someone (unhelpfully)
       registers &PL_unimplemented_op as the address of their custom op.
       NULL doesn't generate a useful error message. "custom" does. */
    const char *const name = op_type >= OP_max
	? "[out of range]" : PL_op_name[PL_op->op_type];
    if(OP_IS_SOCKET(op_type))
	DIE(aTHX_ PL_no_sock_func, name);
    DIE(aTHX_ "panic: unimplemented op %s (#%d) called", name,	op_type);
}

PP(pp_boolkeys)
{
    dVAR;
    dSP;
    HV * const hv = (HV*)POPs;
    
    if (SvTYPE(hv) != SVt_PVHV) { XPUSHs(&PL_sv_no); RETURN; }

    if (SvRMAGICAL(hv)) {
	MAGIC * const mg = mg_find((SV*)hv, PERL_MAGIC_tied);
	if (mg) {
            XPUSHs(magic_scalarpack(hv, mg));
	    RETURN;
        }	    
    }

    XPUSHs(boolSV(HvUSEDKEYS(hv) != 0));
    RETURN;
}

/* For sorting out arguments passed to a &CORE:: subroutine */
PP(pp_coreargs)
{
    dSP;
    int opnum = SvIOK(cSVOP_sv) ? (int)SvUV(cSVOP_sv) : 0;
    int defgv = PL_opargs[opnum] & OA_DEFGV, whicharg = 0;
    AV * const at_ = GvAV(PL_defgv);
    SV **svp = AvARRAY(at_);
    I32 minargs = 0, maxargs = 0, numargs = AvFILLp(at_)+1;
    I32 oa = opnum ? PL_opargs[opnum] >> OASHIFT : 0;
    bool seen_question = 0;
    const char *err = NULL;
    const bool pushmark = PL_op->op_private & OPpCOREARGS_PUSHMARK;

    /* Count how many args there are first, to get some idea how far to
       extend the stack. */
    while (oa) {
	if ((oa & 7) == OA_LIST) { maxargs = I32_MAX; break; }
	maxargs++;
	if (oa & OA_OPTIONAL) seen_question = 1;
	if (!seen_question) minargs++;
	oa >>= 4;
    }

    if(numargs < minargs) err = "Not enough";
    else if(numargs > maxargs) err = "Too many";
    if (err)
	/* diag_listed_as: Too many arguments for %s */
	Perl_croak(aTHX_
	  "%s arguments for %s", err,
	   opnum ? OP_DESC(PL_op->op_next) : SvPV_nolen_const(cSVOP_sv)
	);

    /* Reset the stack pointer.  Without this, we end up returning our own
       arguments in list context, in addition to the values we are supposed
       to return.  nextstate usually does this on sub entry, but we need
       to run the next op with the caller's hints, so we cannot have a
       nextstate. */
    SP = PL_stack_base + cxstack[cxstack_ix].blk_oldsp;

    if(!maxargs) RETURN;

    /* We do this here, rather than with a separate pushmark op, as it has
       to come in between two things this function does (stack reset and
       arg pushing).  This seems the easiest way to do it. */
    if (pushmark) {
	PUTBACK;
	(void)Perl_pp_pushmark(aTHX);
    }

    EXTEND(SP, maxargs == I32_MAX ? numargs : maxargs);
    PUTBACK; /* The code below can die in various places. */

    oa = PL_opargs[opnum] >> OASHIFT;
    for (; oa&&(numargs||!pushmark); (void)(numargs&&(++svp,--numargs))) {
	whicharg++;
	switch (oa & 7) {
	case OA_SCALAR:
	    if (!numargs && defgv && whicharg == minargs + 1) {
		PERL_SI * const oldsi = PL_curstackinfo;
		I32 const oldcxix = oldsi->si_cxix;
		CV *caller;
		if (oldcxix) oldsi->si_cxix--;
		else PL_curstackinfo = oldsi->si_prev;
		caller = find_runcv(NULL);
		PL_curstackinfo = oldsi;
		oldsi->si_cxix = oldcxix;
		PUSHs(find_rundefsv2(
		    caller,cxstack[cxstack_ix].blk_oldcop->cop_seq
		));
	    }
	    else PUSHs(numargs ? svp && *svp ? *svp : &PL_sv_undef : NULL);
	    break;
	case OA_LIST:
	    while (numargs--) {
		PUSHs(svp && *svp ? *svp : &PL_sv_undef);
		svp++;
	    }
	    RETURN;
	case OA_HVREF:
	    if (!svp || !*svp || !SvROK(*svp)
	     || SvTYPE(SvRV(*svp)) != SVt_PVHV)
		DIE(aTHX_
		/* diag_listed_as: Type of arg %d to &CORE::%s must be %s*/
		 "Type of arg %d to &CORE::%s must be hash reference",
		  whicharg, OP_DESC(PL_op->op_next)
		);
	    PUSHs(SvRV(*svp));
	    break;
	case OA_FILEREF:
	    if (!numargs) PUSHs(NULL);
	    else if(svp && *svp && SvROK(*svp) && isGV_with_GP(SvRV(*svp)))
		/* no magic here, as the prototype will have added an extra
		   refgen and we just want what was there before that */
		PUSHs(SvRV(*svp));
	    else {
		const bool constr = PL_op->op_private & whicharg;
		PUSHs(S_rv2gv(aTHX_
		    svp && *svp ? *svp : &PL_sv_undef,
		    constr, CopHINTS_get(PL_curcop) & HINT_STRICT_REFS,
		    !constr
		));
	    }
	    break;
	case OA_SCALARREF:
	  {
	    const bool wantscalar =
		PL_op->op_private & OPpCOREARGS_SCALARMOD;
	    if (!svp || !*svp || !SvROK(*svp)
	        /* We have to permit globrefs even for the \$ proto, as
	           *foo is indistinguishable from ${\*foo}, and the proto-
	           type permits the latter. */
	     || SvTYPE(SvRV(*svp)) > (
	             wantscalar       ? SVt_PVLV
	           : opnum == OP_LOCK ? SVt_PVCV
	           :                    SVt_PVHV
	        )
	       )
		DIE(aTHX_
		/* diag_listed_as: Type of arg %d to &CORE::%s must be %s*/
		 "Type of arg %d to &CORE::%s must be %s",
		  whicharg, OP_DESC(PL_op->op_next),
		  wantscalar
		    ? "scalar reference"
		    : opnum == OP_LOCK
		       ? "reference to one of [$@%&*]"
		       : "reference to one of [$@%*]"
		);
	    PUSHs(SvRV(*svp));
	    break;
	  }
	default:
	    DIE(aTHX_ "panic: unknown OA_*: %x", (unsigned)(oa&7));
	}
	oa = oa >> 4;
    }

    RETURN;
}

PP(pp_runcv)
{
    dSP;
    CV *cv;
    if (PL_op->op_private & OPpOFFBYONE) {
	PERL_SI * const oldsi = PL_curstackinfo;
	I32 const oldcxix = oldsi->si_cxix;
	if (oldcxix) oldsi->si_cxix--;
	else PL_curstackinfo = oldsi->si_prev;
	cv = find_runcv(NULL);
	PL_curstackinfo = oldsi;
	oldsi->si_cxix = oldcxix;
    }
    else cv = find_runcv(NULL);
    XPUSHs(CvEVAL(cv) ? &PL_sv_undef : sv_2mortal(newRV((SV *)cv)));
    RETURN;
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
