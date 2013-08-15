/*    pp_ctl.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *      Now far ahead the Road has gone,
 *          And I must follow, if I can,
 *      Pursuing it with eager feet,
 *          Until it joins some larger way
 *      Where many paths and errands meet.
 *          And whither then?  I cannot say.
 *
 *     [Bilbo on p.35 of _The Lord of the Rings_, I/i: "A Long-Expected Party"]
 */

/* This file contains control-oriented pp ("push/pop") functions that
 * execute the opcodes that make up a perl program. A typical pp function
 * expects to find its arguments on the stack, and usually pushes its
 * results onto the stack, hence the 'pp' terminology. Each OP structure
 * contains a pointer to the relevant pp_foo() function.
 *
 * Control-oriented means things like pp_enteriter() and pp_next(), which
 * alter the flow of control of the program.
 */


#include "EXTERN.h"
#define PERL_IN_PP_CTL_C
#include "perl.h"

#define DOCATCH(o) ((CATCH_GET == TRUE) ? docatch(o) : (o))

#define dopoptosub(plop)	dopoptosub_at(cxstack, (plop))

PP(pp_wantarray)
{
    dVAR;
    dSP;
    I32 cxix;
    const PERL_CONTEXT *cx;
    EXTEND(SP, 1);

    if (PL_op->op_private & OPpOFFBYONE) {
	if (!(cx = caller_cx(1,NULL))) RETPUSHUNDEF;
    }
    else {
      cxix = dopoptosub(cxstack_ix);
      if (cxix < 0)
	RETPUSHUNDEF;
      cx = &cxstack[cxix];
    }

    switch (cx->blk_gimme) {
    case G_ARRAY:
	RETPUSHYES;
    case G_SCALAR:
	RETPUSHNO;
    default:
	RETPUSHUNDEF;
    }
}

PP(pp_regcreset)
{
    dVAR;
    /* XXXX Should store the old value to allow for tie/overload - and
       restore in regcomp, where marked with XXXX. */
    PL_reginterp_cnt = 0;
    TAINT_NOT;
    return NORMAL;
}

PP(pp_regcomp)
{
    dVAR;
    dSP;
    register PMOP *pm = (PMOP*)cLOGOP->op_other;
    SV *tmpstr;
    REGEXP *re = NULL;

    /* prevent recompiling under /o and ithreads. */
#if defined(USE_ITHREADS)
    if (pm->op_pmflags & PMf_KEEP && PM_GETRE(pm)) {
	if (PL_op->op_flags & OPf_STACKED) {
	    dMARK;
	    SP = MARK;
	}
	else
	    (void)POPs;
	RETURN;
    }
#endif

#define tryAMAGICregexp(rx)			\
    STMT_START {				\
	SvGETMAGIC(rx);				\
	if (SvROK(rx) && SvAMAGIC(rx)) {	\
	    SV *sv = AMG_CALLunary(rx, regexp_amg); \
	    if (sv) {				\
		if (SvROK(sv))			\
		    sv = SvRV(sv);		\
		if (SvTYPE(sv) != SVt_REGEXP)	\
		    Perl_croak(aTHX_ "Overloaded qr did not return a REGEXP"); \
		rx = sv;			\
	    }					\
	}					\
    } STMT_END
	    

    if (PL_op->op_flags & OPf_STACKED) {
	/* multiple args; concatenate them */
	dMARK; dORIGMARK;
	tmpstr = PAD_SV(ARGTARG);
	sv_setpvs(tmpstr, "");
	while (++MARK <= SP) {
	    SV *msv = *MARK;
	    SV *sv;

	    tryAMAGICregexp(msv);

	    if ((SvAMAGIC(tmpstr) || SvAMAGIC(msv)) &&
		(sv = amagic_call(tmpstr, msv, concat_amg, AMGf_assign)))
	    {
	       sv_setsv(tmpstr, sv);
	       continue;
	    }

	    if (SvROK(msv) && SvTYPE(SvRV(msv)) == SVt_REGEXP) {
		msv = SvRV(msv);
		PL_reginterp_cnt +=
		    RX_SEEN_EVALS((REGEXP *)MUTABLE_PTR(msv));
	    }

	    sv_catsv_nomg(tmpstr, msv);
	}
    	SvSETMAGIC(tmpstr);
	SP = ORIGMARK;
    }
    else {
	tmpstr = POPs;
	tryAMAGICregexp(tmpstr);
    }

#undef tryAMAGICregexp

    if (SvROK(tmpstr)) {
	SV * const sv = SvRV(tmpstr);
	if (SvTYPE(sv) == SVt_REGEXP)
	    re = (REGEXP*) sv;
    }
    else if (SvTYPE(tmpstr) == SVt_REGEXP)
	re = (REGEXP*) tmpstr;

    if (re) {
	/* The match's LHS's get-magic might need to access this op's reg-
	   exp (as is sometimes the case with $';  see bug 70764).  So we
	   must call get-magic now before we replace the regexp. Hopeful-
	   ly this hack can be replaced with the approach described at
	   http://www.nntp.perl.org/group/perl.perl5.porters/2007/03
	   /msg122415.html some day. */
	if(pm->op_type == OP_MATCH) {
	 SV *lhs;
	 const bool was_tainted = PL_tainted;
	 if (pm->op_flags & OPf_STACKED)
	    lhs = TOPs;
	 else if (pm->op_private & OPpTARGET_MY)
	    lhs = PAD_SV(pm->op_targ);
	 else lhs = DEFSV;
	 SvGETMAGIC(lhs);
	 /* Restore the previous value of PL_tainted (which may have been
	    modified by get-magic), to avoid incorrectly setting the
	    RXf_TAINTED flag further down. */
	 PL_tainted = was_tainted;
	}

	re = reg_temp_copy(NULL, re);
	ReREFCNT_dec(PM_GETRE(pm));
	PM_SETRE(pm, re);
    }
    else {
	STRLEN len = 0;
	const char *t = SvOK(tmpstr) ? SvPV_nomg_const(tmpstr, len) : "";

	re = PM_GETRE(pm);
	assert (re != (REGEXP*) &PL_sv_undef);

	/* Check against the last compiled regexp. */
	if (!re || !RX_PRECOMP(re) || RX_PRELEN(re) != len ||
	    memNE(RX_PRECOMP(re), t, len))
	{
	    const regexp_engine *eng = re ? RX_ENGINE(re) : NULL;
            U32 pm_flags = pm->op_pmflags & RXf_PMf_COMPILETIME;
	    if (re) {
	        ReREFCNT_dec(re);
#ifdef USE_ITHREADS
		PM_SETRE(pm, (REGEXP*) &PL_sv_undef);
#else
		PM_SETRE(pm, NULL);	/* crucial if regcomp aborts */
#endif
	    } else if (PL_curcop->cop_hints_hash) {
	        SV *ptr = cop_hints_fetch_pvs(PL_curcop, "regcomp", 0);
                if (ptr && SvIOK(ptr) && SvIV(ptr))
                    eng = INT2PTR(regexp_engine*,SvIV(ptr));
	    }

	    if (PL_op->op_flags & OPf_SPECIAL)
		PL_reginterp_cnt = I32_MAX; /* Mark as safe.  */

	    if (!DO_UTF8(tmpstr) && SvUTF8(tmpstr)) {
		/* Not doing UTF-8, despite what the SV says. Is this only if
		   we're trapped in use 'bytes'?  */
		/* Make a copy of the octet sequence, but without the flag on,
		   as the compiler now honours the SvUTF8 flag on tmpstr.  */
		STRLEN len;
		const char *const p = SvPV(tmpstr, len);
		tmpstr = newSVpvn_flags(p, len, SVs_TEMP);
	    }
	    else if (SvAMAGIC(tmpstr) || SvGMAGICAL(tmpstr)) {
		/* make a copy to avoid extra stringifies */
		tmpstr = newSVpvn_flags(t, len, SVs_TEMP | SvUTF8(tmpstr));
	    }

	    if (eng)
	        PM_SETRE(pm, CALLREGCOMP_ENG(eng, tmpstr, pm_flags));
	    else
	        PM_SETRE(pm, CALLREGCOMP(tmpstr, pm_flags));

	    PL_reginterp_cnt = 0;	/* XXXX Be extra paranoid - needed
					   inside tie/overload accessors.  */
	}
    }
    
    re = PM_GETRE(pm);

#ifndef INCOMPLETE_TAINTS
    if (PL_tainting) {
	if (PL_tainted) {
	    SvTAINTED_on((SV*)re);
	    RX_EXTFLAGS(re) |= RXf_TAINTED;
	}
    }
#endif

    if (!RX_PRELEN(PM_GETRE(pm)) && PL_curpm)
	pm = PL_curpm;


#if !defined(USE_ITHREADS)
    /* can't change the optree at runtime either */
    /* PMf_KEEP is handled differently under threads to avoid these problems */
    if (pm->op_pmflags & PMf_KEEP) {
	pm->op_private &= ~OPpRUNTIME;	/* no point compiling again */
	cLOGOP->op_first->op_next = PL_op->op_next;
    }
#endif
    RETURN;
}

PP(pp_substcont)
{
    dVAR;
    dSP;
    register PERL_CONTEXT *cx = &cxstack[cxstack_ix];
    register PMOP * const pm = (PMOP*) cLOGOP->op_other;
    register SV * const dstr = cx->sb_dstr;
    register char *s = cx->sb_s;
    register char *m = cx->sb_m;
    char *orig = cx->sb_orig;
    register REGEXP * const rx = cx->sb_rx;
    SV *nsv = NULL;
    REGEXP *old = PM_GETRE(pm);

    PERL_ASYNC_CHECK();

    if(old != rx) {
	if(old)
	    ReREFCNT_dec(old);
	PM_SETRE(pm,ReREFCNT_inc(rx));
    }

    rxres_restore(&cx->sb_rxres, rx);
    RX_MATCH_UTF8_set(rx, DO_UTF8(cx->sb_targ));

    if (cx->sb_iters++) {
	const I32 saviters = cx->sb_iters;
	if (cx->sb_iters > cx->sb_maxiters)
	    DIE(aTHX_ "Substitution loop");

	SvGETMAGIC(TOPs); /* possibly clear taint on $1 etc: #67962 */

    	/* See "how taint works" above pp_subst() */
	if (SvTAINTED(TOPs))
	    cx->sb_rxtainted |= SUBST_TAINT_REPL;
	sv_catsv_nomg(dstr, POPs);
	/* XXX: adjust for positive offsets of \G for instance s/(.)\G//g with positive pos() */
	s -= RX_GOFS(rx);

	/* Are we done */
	/* I believe that we can't set REXEC_SCREAM here if
	   SvSCREAM(cx->sb_targ) is true because SvPVX(cx->sb_targ) isn't always
	   equal to s.  [See the comment before Perl_re_intuit_start(), which is
	   called from Perl_regexec_flags(), which says that it should be when
	   SvSCREAM() is true.]  s, cx->sb_strend and orig will be consistent
	   with SvPVX(cx->sb_targ), as substconst doesn't modify cx->sb_targ
	   during the match.  */
	if (CxONCE(cx) || s < orig ||
		!CALLREGEXEC(rx, s, cx->sb_strend, orig,
			     (s == m) + RX_GOFS(rx), cx->sb_targ, NULL,
			     ((cx->sb_rflags & REXEC_COPY_STR)
			      ? (REXEC_IGNOREPOS|REXEC_NOT_FIRST)
			      : (REXEC_COPY_STR|REXEC_IGNOREPOS|REXEC_NOT_FIRST))))
	{
	    SV *targ = cx->sb_targ;

	    assert(cx->sb_strend >= s);
	    if(cx->sb_strend > s) {
		 if (DO_UTF8(dstr) && !SvUTF8(targ))
		      sv_catpvn_utf8_upgrade(dstr, s, cx->sb_strend - s, nsv);
		 else
		      sv_catpvn(dstr, s, cx->sb_strend - s);
	    }
	    if (RX_MATCH_TAINTED(rx)) /* run time pattern taint, eg locale */
		cx->sb_rxtainted |= SUBST_TAINT_PAT;

	    if (pm->op_pmflags & PMf_NONDESTRUCT) {
		PUSHs(dstr);
		/* From here on down we're using the copy, and leaving the
		   original untouched.  */
		targ = dstr;
	    }
	    else {
		if (SvIsCOW(targ)) {
		    sv_force_normal_flags(targ, SV_COW_DROP_PV);
		} else
		{
		    SvPV_free(targ);
		}
		SvPV_set(targ, SvPVX(dstr));
		SvCUR_set(targ, SvCUR(dstr));
		SvLEN_set(targ, SvLEN(dstr));
		if (DO_UTF8(dstr))
		    SvUTF8_on(targ);
		SvPV_set(dstr, NULL);

		mPUSHi(saviters - 1);

		(void)SvPOK_only_UTF8(targ);
	    }

	    /* update the taint state of various various variables in
	     * preparation for final exit.
	     * See "how taint works" above pp_subst() */
	    if (PL_tainting) {
		if ((cx->sb_rxtainted & SUBST_TAINT_PAT) ||
		    ((cx->sb_rxtainted & (SUBST_TAINT_STR|SUBST_TAINT_RETAINT))
				    == (SUBST_TAINT_STR|SUBST_TAINT_RETAINT))
		)
		    (RX_MATCH_TAINTED_on(rx)); /* taint $1 et al */

		if (!(cx->sb_rxtainted & SUBST_TAINT_BOOLRET)
		    && (cx->sb_rxtainted & (SUBST_TAINT_STR|SUBST_TAINT_PAT))
		)
		    SvTAINTED_on(TOPs);  /* taint return value */
		/* needed for mg_set below */
		PL_tainted = cBOOL(cx->sb_rxtainted &
			    (SUBST_TAINT_STR|SUBST_TAINT_PAT|SUBST_TAINT_REPL));
		SvTAINT(TARG);
	    }
	    /* PL_tainted must be correctly set for this mg_set */
	    SvSETMAGIC(TARG);
	    TAINT_NOT;
	    LEAVE_SCOPE(cx->sb_oldsave);
	    POPSUBST(cx);
	    RETURNOP(pm->op_next);
	    /* NOTREACHED */
	}
	cx->sb_iters = saviters;
    }
    if (RX_MATCH_COPIED(rx) && RX_SUBBEG(rx) != orig) {
	m = s;
	s = orig;
	cx->sb_orig = orig = RX_SUBBEG(rx);
	s = orig + (m - s);
	cx->sb_strend = s + (cx->sb_strend - m);
    }
    cx->sb_m = m = RX_OFFS(rx)[0].start + orig;
    if (m > s) {
	if (DO_UTF8(dstr) && !SvUTF8(cx->sb_targ))
	    sv_catpvn_utf8_upgrade(dstr, s, m - s, nsv);
	else
	    sv_catpvn(dstr, s, m-s);
    }
    cx->sb_s = RX_OFFS(rx)[0].end + orig;
    { /* Update the pos() information. */
	SV * const sv
	    = (pm->op_pmflags & PMf_NONDESTRUCT) ? cx->sb_dstr : cx->sb_targ;
	MAGIC *mg;
	SvUPGRADE(sv, SVt_PVMG);
	if (!(mg = mg_find(sv, PERL_MAGIC_regex_global))) {
#ifdef PERL_OLD_COPY_ON_WRITE
	    if (SvIsCOW(sv))
		sv_force_normal_flags(sv, 0);
#endif
	    mg = sv_magicext(sv, NULL, PERL_MAGIC_regex_global, &PL_vtbl_mglob,
			     NULL, 0);
	}
	mg->mg_len = m - orig;
    }
    if (old != rx)
	(void)ReREFCNT_inc(rx);
    /* update the taint state of various various variables in preparation
     * for calling the code block.
     * See "how taint works" above pp_subst() */
    if (PL_tainting) {
	if (RX_MATCH_TAINTED(rx)) /* run time pattern taint, eg locale */
	    cx->sb_rxtainted |= SUBST_TAINT_PAT;

	if ((cx->sb_rxtainted & SUBST_TAINT_PAT) ||
	    ((cx->sb_rxtainted & (SUBST_TAINT_STR|SUBST_TAINT_RETAINT))
			    == (SUBST_TAINT_STR|SUBST_TAINT_RETAINT))
	)
	    (RX_MATCH_TAINTED_on(rx)); /* taint $1 et al */

	if (cx->sb_iters > 1 && (cx->sb_rxtainted & 
			(SUBST_TAINT_STR|SUBST_TAINT_PAT|SUBST_TAINT_REPL)))
	    SvTAINTED_on((pm->op_pmflags & PMf_NONDESTRUCT)
			 ? cx->sb_dstr : cx->sb_targ);
	TAINT_NOT;
    }
    rxres_save(&cx->sb_rxres, rx);
    PL_curpm = pm;
    RETURNOP(pm->op_pmstashstartu.op_pmreplstart);
}

void
Perl_rxres_save(pTHX_ void **rsp, REGEXP *rx)
{
    UV *p = (UV*)*rsp;
    U32 i;

    PERL_ARGS_ASSERT_RXRES_SAVE;
    PERL_UNUSED_CONTEXT;

    if (!p || p[1] < RX_NPARENS(rx)) {
#ifdef PERL_OLD_COPY_ON_WRITE
	i = 7 + RX_NPARENS(rx) * 2;
#else
	i = 6 + RX_NPARENS(rx) * 2;
#endif
	if (!p)
	    Newx(p, i, UV);
	else
	    Renew(p, i, UV);
	*rsp = (void*)p;
    }

    *p++ = PTR2UV(RX_MATCH_COPIED(rx) ? RX_SUBBEG(rx) : NULL);
    RX_MATCH_COPIED_off(rx);

#ifdef PERL_OLD_COPY_ON_WRITE
    *p++ = PTR2UV(RX_SAVED_COPY(rx));
    RX_SAVED_COPY(rx) = NULL;
#endif

    *p++ = RX_NPARENS(rx);

    *p++ = PTR2UV(RX_SUBBEG(rx));
    *p++ = (UV)RX_SUBLEN(rx);
    for (i = 0; i <= RX_NPARENS(rx); ++i) {
	*p++ = (UV)RX_OFFS(rx)[i].start;
	*p++ = (UV)RX_OFFS(rx)[i].end;
    }
}

static void
S_rxres_restore(pTHX_ void **rsp, REGEXP *rx)
{
    UV *p = (UV*)*rsp;
    U32 i;

    PERL_ARGS_ASSERT_RXRES_RESTORE;
    PERL_UNUSED_CONTEXT;

    RX_MATCH_COPY_FREE(rx);
    RX_MATCH_COPIED_set(rx, *p);
    *p++ = 0;

#ifdef PERL_OLD_COPY_ON_WRITE
    if (RX_SAVED_COPY(rx))
	SvREFCNT_dec (RX_SAVED_COPY(rx));
    RX_SAVED_COPY(rx) = INT2PTR(SV*,*p);
    *p++ = 0;
#endif

    RX_NPARENS(rx) = *p++;

    RX_SUBBEG(rx) = INT2PTR(char*,*p++);
    RX_SUBLEN(rx) = (I32)(*p++);
    for (i = 0; i <= RX_NPARENS(rx); ++i) {
	RX_OFFS(rx)[i].start = (I32)(*p++);
	RX_OFFS(rx)[i].end = (I32)(*p++);
    }
}

static void
S_rxres_free(pTHX_ void **rsp)
{
    UV * const p = (UV*)*rsp;

    PERL_ARGS_ASSERT_RXRES_FREE;
    PERL_UNUSED_CONTEXT;

    if (p) {
#ifdef PERL_POISON
	void *tmp = INT2PTR(char*,*p);
	Safefree(tmp);
	if (*p)
	    PoisonFree(*p, 1, sizeof(*p));
#else
	Safefree(INT2PTR(char*,*p));
#endif
#ifdef PERL_OLD_COPY_ON_WRITE
	if (p[1]) {
	    SvREFCNT_dec (INT2PTR(SV*,p[1]));
	}
#endif
	Safefree(p);
	*rsp = NULL;
    }
}

#define FORM_NUM_BLANK (1<<30)
#define FORM_NUM_POINT (1<<29)

PP(pp_formline)
{
    dVAR; dSP; dMARK; dORIGMARK;
    register SV * const tmpForm = *++MARK;
    SV *formsv;		    /* contains text of original format */
    register U32 *fpc;	    /* format ops program counter */
    register char *t;	    /* current append position in target string */
    const char *f;	    /* current position in format string */
    register I32 arg;
    register SV *sv = NULL; /* current item */
    const char *item = NULL;/* string value of current item */
    I32 itemsize  = 0;	    /* length of current item, possibly truncated */
    I32 fieldsize = 0;	    /* width of current field */
    I32 lines = 0;	    /* number of lines that have been output */
    bool chopspace = (strchr(PL_chopset, ' ') != NULL); /* does $: have space */
    const char *chophere = NULL; /* where to chop current item */
    STRLEN linemark = 0;    /* pos of start of line in output */
    NV value;
    bool gotsome = FALSE;   /* seen at least one non-blank item on this line */
    STRLEN len;
    STRLEN linemax;	    /* estimate of output size in bytes */
    bool item_is_utf8 = FALSE;
    bool targ_is_utf8 = FALSE;
    const char *fmt;
    MAGIC *mg = NULL;
    U8 *source;		    /* source of bytes to append */
    STRLEN to_copy;	    /* how may bytes to append */
    char trans;		    /* what chars to translate */

    mg = doparseform(tmpForm);

    fpc = (U32*)mg->mg_ptr;
    /* the actual string the format was compiled from.
     * with overload etc, this may not match tmpForm */
    formsv = mg->mg_obj;


    SvPV_force(PL_formtarget, len);
    if (SvTAINTED(tmpForm) || SvTAINTED(formsv))
	SvTAINTED_on(PL_formtarget);
    if (DO_UTF8(PL_formtarget))
	targ_is_utf8 = TRUE;
    linemax = (SvCUR(formsv) * (IN_BYTES ? 1 : 3) + 1);
    t = SvGROW(PL_formtarget, len + linemax + 1);
    /* XXX from now onwards, SvCUR(PL_formtarget) is invalid */
    t += len;
    f = SvPV_const(formsv, len);

    for (;;) {
	DEBUG_f( {
	    const char *name = "???";
	    arg = -1;
	    switch (*fpc) {
	    case FF_LITERAL:	arg = fpc[1]; name = "LITERAL";	break;
	    case FF_BLANK:	arg = fpc[1]; name = "BLANK";	break;
	    case FF_SKIP:	arg = fpc[1]; name = "SKIP";	break;
	    case FF_FETCH:	arg = fpc[1]; name = "FETCH";	break;
	    case FF_DECIMAL:	arg = fpc[1]; name = "DECIMAL";	break;

	    case FF_CHECKNL:	name = "CHECKNL";	break;
	    case FF_CHECKCHOP:	name = "CHECKCHOP";	break;
	    case FF_SPACE:	name = "SPACE";		break;
	    case FF_HALFSPACE:	name = "HALFSPACE";	break;
	    case FF_ITEM:	name = "ITEM";		break;
	    case FF_CHOP:	name = "CHOP";		break;
	    case FF_LINEGLOB:	name = "LINEGLOB";	break;
	    case FF_NEWLINE:	name = "NEWLINE";	break;
	    case FF_MORE:	name = "MORE";		break;
	    case FF_LINEMARK:	name = "LINEMARK";	break;
	    case FF_END:	name = "END";		break;
	    case FF_0DECIMAL:	name = "0DECIMAL";	break;
	    case FF_LINESNGL:	name = "LINESNGL";	break;
	    }
	    if (arg >= 0)
		PerlIO_printf(Perl_debug_log, "%-16s%ld\n", name, (long) arg);
	    else
		PerlIO_printf(Perl_debug_log, "%-16s\n", name);
	} );
	switch (*fpc++) {
	case FF_LINEMARK:
	    linemark = t - SvPVX(PL_formtarget);
	    lines++;
	    gotsome = FALSE;
	    break;

	case FF_LITERAL:
	    to_copy = *fpc++;
	    source = (U8 *)f;
	    f += to_copy;
	    trans = '~';
	    item_is_utf8 = targ_is_utf8 ? !!DO_UTF8(formsv) : !!SvUTF8(formsv);
	    goto append;

	case FF_SKIP:
	    f += *fpc++;
	    break;

	case FF_FETCH:
	    arg = *fpc++;
	    f += arg;
	    fieldsize = arg;

	    if (MARK < SP)
		sv = *++MARK;
	    else {
		sv = &PL_sv_no;
		Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX), "Not enough format arguments");
	    }
	    if (SvTAINTED(sv))
		SvTAINTED_on(PL_formtarget);
	    break;

	case FF_CHECKNL:
	    {
		const char *send;
		const char *s = item = SvPV_const(sv, len);
		itemsize = len;
		if (DO_UTF8(sv)) {
		    itemsize = sv_len_utf8(sv);
		    if (itemsize != (I32)len) {
			I32 itembytes;
			if (itemsize > fieldsize) {
			    itemsize = fieldsize;
			    itembytes = itemsize;
			    sv_pos_u2b(sv, &itembytes, 0);
			}
			else
			    itembytes = len;
			send = chophere = s + itembytes;
			while (s < send) {
			    if (*s & ~31)
				gotsome = TRUE;
			    else if (*s == '\n')
				break;
			    s++;
			}
			item_is_utf8 = TRUE;
			itemsize = s - item;
			sv_pos_b2u(sv, &itemsize);
			break;
		    }
		}
		item_is_utf8 = FALSE;
		if (itemsize > fieldsize)
		    itemsize = fieldsize;
		send = chophere = s + itemsize;
		while (s < send) {
		    if (*s & ~31)
			gotsome = TRUE;
		    else if (*s == '\n')
			break;
		    s++;
		}
		itemsize = s - item;
		break;
	    }

	case FF_CHECKCHOP:
	    {
		const char *s = item = SvPV_const(sv, len);
		itemsize = len;
		if (DO_UTF8(sv)) {
		    itemsize = sv_len_utf8(sv);
		    if (itemsize != (I32)len) {
			I32 itembytes;
			if (itemsize <= fieldsize) {
			    const char *send = chophere = s + itemsize;
			    while (s < send) {
				if (*s == '\r') {
				    itemsize = s - item;
				    chophere = s;
				    break;
				}
				if (*s++ & ~31)
				    gotsome = TRUE;
			    }
			}
			else {
			    const char *send;
			    itemsize = fieldsize;
			    itembytes = itemsize;
			    sv_pos_u2b(sv, &itembytes, 0);
			    send = chophere = s + itembytes;
			    while (s < send || (s == send && isSPACE(*s))) {
				if (isSPACE(*s)) {
				    if (chopspace)
					chophere = s;
				    if (*s == '\r')
					break;
				}
				else {
				    if (*s & ~31)
					gotsome = TRUE;
				    if (strchr(PL_chopset, *s))
					chophere = s + 1;
				}
				s++;
			    }
			    itemsize = chophere - item;
			    sv_pos_b2u(sv, &itemsize);
			}
			item_is_utf8 = TRUE;
			break;
		    }
		}
		item_is_utf8 = FALSE;
		if (itemsize <= fieldsize) {
		    const char *const send = chophere = s + itemsize;
		    while (s < send) {
			if (*s == '\r') {
			    itemsize = s - item;
			    chophere = s;
			    break;
			}
			if (*s++ & ~31)
			    gotsome = TRUE;
		    }
		}
		else {
		    const char *send;
		    itemsize = fieldsize;
		    send = chophere = s + itemsize;
		    while (s < send || (s == send && isSPACE(*s))) {
			if (isSPACE(*s)) {
			    if (chopspace)
				chophere = s;
			    if (*s == '\r')
				break;
			}
			else {
			    if (*s & ~31)
				gotsome = TRUE;
			    if (strchr(PL_chopset, *s))
				chophere = s + 1;
			}
			s++;
		    }
		    itemsize = chophere - item;
		}
		break;
	    }

	case FF_SPACE:
	    arg = fieldsize - itemsize;
	    if (arg) {
		fieldsize -= arg;
		while (arg-- > 0)
		    *t++ = ' ';
	    }
	    break;

	case FF_HALFSPACE:
	    arg = fieldsize - itemsize;
	    if (arg) {
		arg /= 2;
		fieldsize -= arg;
		while (arg-- > 0)
		    *t++ = ' ';
	    }
	    break;

	case FF_ITEM:
	    to_copy = itemsize;
	    source = (U8 *)item;
	    trans = 1;
	    if (item_is_utf8) {
		/* convert to_copy from chars to bytes */
		U8 *s = source;
		while (to_copy--)
		   s += UTF8SKIP(s);
		to_copy = s - source;
	    }
	    goto append;

	case FF_CHOP:
	    {
		const char *s = chophere;
		if (chopspace) {
		    while (isSPACE(*s))
			s++;
		}
		sv_chop(sv,s);
		SvSETMAGIC(sv);
		break;
	    }

	case FF_LINESNGL:
	    chopspace = 0;
	case FF_LINEGLOB:
	    {
		const bool oneline = fpc[-1] == FF_LINESNGL;
		const char *s = item = SvPV_const(sv, len);
		const char *const send = s + len;

		item_is_utf8 = DO_UTF8(sv);
		if (!len)
		    break;
		trans = 0;
		gotsome = TRUE;
		chophere = s + len;
		source = (U8 *) s;
		to_copy = len;
		while (s < send) {
		    if (*s++ == '\n') {
			if (oneline) {
			    to_copy = s - SvPVX_const(sv) - 1;
			    chophere = s;
			    break;
			} else {
			    if (s == send) {
				to_copy--;
			    } else
				lines++;
			}
		    }
		}
	    }

	append:
	    /* append to_copy bytes from source to PL_formstring.
	     * item_is_utf8 implies source is utf8.
	     * if trans, translate certain characters during the copy */
	    {
		U8 *tmp = NULL;
		STRLEN grow = 0;

		SvCUR_set(PL_formtarget,
			  t - SvPVX_const(PL_formtarget));

		if (targ_is_utf8 && !item_is_utf8) {
		    source = tmp = bytes_to_utf8(source, &to_copy);
		} else {
		    if (item_is_utf8 && !targ_is_utf8) {
			U8 *s;
			/* Upgrade targ to UTF8, and then we reduce it to
			   a problem we have a simple solution for.
			   Don't need get magic.  */
			sv_utf8_upgrade_nomg(PL_formtarget);
			targ_is_utf8 = TRUE;
			/* re-calculate linemark */
			s = (U8*)SvPVX(PL_formtarget);
			/* the bytes we initially allocated to append the
			 * whole line may have been gobbled up during the
			 * upgrade, so allocate a whole new line's worth
			 * for safety */
			grow = linemax;
			while (linemark--)
			    s += UTF8SKIP(s);
			linemark = s - (U8*)SvPVX(PL_formtarget);
		    }
		    /* Easy. They agree.  */
		    assert (item_is_utf8 == targ_is_utf8);
		}
		if (!trans)
		    /* @* and ^* are the only things that can exceed
		     * the linemax, so grow by the output size, plus
		     * a whole new form's worth in case of any further
		     * output */
		    grow = linemax + to_copy;
		if (grow)
		    SvGROW(PL_formtarget, SvCUR(PL_formtarget) + grow + 1);
		t = SvPVX(PL_formtarget) + SvCUR(PL_formtarget);

		Copy(source, t, to_copy, char);
		if (trans) {
		    /* blank out ~ or control chars, depending on trans.
		     * works on bytes not chars, so relies on not
		     * matching utf8 continuation bytes */
		    U8 *s = (U8*)t;
		    U8 *send = s + to_copy;
		    while (s < send) {
			const int ch = *s;
			if (trans == '~' ? (ch == '~') :
#ifdef EBCDIC
			       iscntrl(ch)
#else
			       (!(ch & ~31))
#endif
			)
			    *s = ' ';
			s++;
		    }
		}

		t += to_copy;
		SvCUR_set(PL_formtarget, SvCUR(PL_formtarget) + to_copy);
		if (tmp)
		    Safefree(tmp);
		break;
	    }

	case FF_0DECIMAL:
	    arg = *fpc++;
#if defined(USE_LONG_DOUBLE)
	    fmt = (const char *)
		((arg & FORM_NUM_POINT) ?
		 "%#0*.*" PERL_PRIfldbl : "%0*.*" PERL_PRIfldbl);
#else
	    fmt = (const char *)
		((arg & FORM_NUM_POINT) ?
		 "%#0*.*f"              : "%0*.*f");
#endif
	    goto ff_dec;
	case FF_DECIMAL:
	    arg = *fpc++;
#if defined(USE_LONG_DOUBLE)
 	    fmt = (const char *)
		((arg & FORM_NUM_POINT) ? "%#*.*" PERL_PRIfldbl : "%*.*" PERL_PRIfldbl);
#else
            fmt = (const char *)
		((arg & FORM_NUM_POINT) ? "%#*.*f"              : "%*.*f");
#endif
	ff_dec:
	    /* If the field is marked with ^ and the value is undefined,
	       blank it out. */
	    if ((arg & FORM_NUM_BLANK) && !SvOK(sv)) {
		arg = fieldsize;
		while (arg--)
		    *t++ = ' ';
		break;
	    }
	    gotsome = TRUE;
	    value = SvNV(sv);
	    /* overflow evidence */
	    if (num_overflow(value, fieldsize, arg)) {
	        arg = fieldsize;
		while (arg--)
		    *t++ = '#';
		break;
	    }
	    /* Formats aren't yet marked for locales, so assume "yes". */
	    {
		STORE_NUMERIC_STANDARD_SET_LOCAL();
		arg &= ~(FORM_NUM_POINT|FORM_NUM_BLANK);
		my_snprintf(t, SvLEN(PL_formtarget) - (t - SvPVX(PL_formtarget)), fmt, (int) fieldsize, (int) arg, value);
		RESTORE_NUMERIC_STANDARD();
	    }
	    t += fieldsize;
	    break;

	case FF_NEWLINE:
	    f++;
	    while (t-- > (SvPVX(PL_formtarget) + linemark) && *t == ' ') ;
	    t++;
	    *t++ = '\n';
	    break;

	case FF_BLANK:
	    arg = *fpc++;
	    if (gotsome) {
		if (arg) {		/* repeat until fields exhausted? */
		    fpc--;
		    goto end;
		}
	    }
	    else {
		t = SvPVX(PL_formtarget) + linemark;
		lines--;
	    }
	    break;

	case FF_MORE:
	    {
		const char *s = chophere;
		const char *send = item + len;
		if (chopspace) {
		    while (isSPACE(*s) && (s < send))
			s++;
		}
		if (s < send) {
		    char *s1;
		    arg = fieldsize - itemsize;
		    if (arg) {
			fieldsize -= arg;
			while (arg-- > 0)
			    *t++ = ' ';
		    }
		    s1 = t - 3;
		    if (strnEQ(s1,"   ",3)) {
			while (s1 > SvPVX_const(PL_formtarget) && isSPACE(s1[-1]))
			    s1--;
		    }
		    *s1++ = '.';
		    *s1++ = '.';
		    *s1++ = '.';
		}
		break;
	    }
	case FF_END:
	end:
	    assert(t < SvPVX_const(PL_formtarget) + SvLEN(PL_formtarget));
	    *t = '\0';
	    SvCUR_set(PL_formtarget, t - SvPVX_const(PL_formtarget));
	    if (targ_is_utf8)
		SvUTF8_on(PL_formtarget);
	    FmLINES(PL_formtarget) += lines;
	    SP = ORIGMARK;
	    if (fpc[-1] == FF_BLANK)
		RETURNOP(cLISTOP->op_first);
	    else
		RETPUSHYES;
	}
    }
}

PP(pp_grepstart)
{
    dVAR; dSP;
    SV *src;

    if (PL_stack_base + *PL_markstack_ptr == SP) {
	(void)POPMARK;
	if (GIMME_V == G_SCALAR)
	    mXPUSHi(0);
	RETURNOP(PL_op->op_next->op_next);
    }
    PL_stack_sp = PL_stack_base + *PL_markstack_ptr + 1;
    Perl_pp_pushmark(aTHX);				/* push dst */
    Perl_pp_pushmark(aTHX);				/* push src */
    ENTER_with_name("grep");					/* enter outer scope */

    SAVETMPS;
    if (PL_op->op_private & OPpGREP_LEX)
	SAVESPTR(PAD_SVl(PL_op->op_targ));
    else
	SAVE_DEFSV;
    ENTER_with_name("grep_item");					/* enter inner scope */
    SAVEVPTR(PL_curpm);

    src = PL_stack_base[*PL_markstack_ptr];
    SvTEMP_off(src);
    if (PL_op->op_private & OPpGREP_LEX)
	PAD_SVl(PL_op->op_targ) = src;
    else
	DEFSV_set(src);

    PUTBACK;
    if (PL_op->op_type == OP_MAPSTART)
	Perl_pp_pushmark(aTHX);			/* push top */
    return ((LOGOP*)PL_op->op_next)->op_other;
}

PP(pp_mapwhile)
{
    dVAR; dSP;
    const I32 gimme = GIMME_V;
    I32 items = (SP - PL_stack_base) - *PL_markstack_ptr; /* how many new items */
    I32 count;
    I32 shift;
    SV** src;
    SV** dst;

    /* first, move source pointer to the next item in the source list */
    ++PL_markstack_ptr[-1];

    /* if there are new items, push them into the destination list */
    if (items && gimme != G_VOID) {
	/* might need to make room back there first */
	if (items > PL_markstack_ptr[-1] - PL_markstack_ptr[-2]) {
	    /* XXX this implementation is very pessimal because the stack
	     * is repeatedly extended for every set of items.  Is possible
	     * to do this without any stack extension or copying at all
	     * by maintaining a separate list over which the map iterates
	     * (like foreach does). --gsar */

	    /* everything in the stack after the destination list moves
	     * towards the end the stack by the amount of room needed */
	    shift = items - (PL_markstack_ptr[-1] - PL_markstack_ptr[-2]);

	    /* items to shift up (accounting for the moved source pointer) */
	    count = (SP - PL_stack_base) - (PL_markstack_ptr[-1] - 1);

	    /* This optimization is by Ben Tilly and it does
	     * things differently from what Sarathy (gsar)
	     * is describing.  The downside of this optimization is
	     * that leaves "holes" (uninitialized and hopefully unused areas)
	     * to the Perl stack, but on the other hand this
	     * shouldn't be a problem.  If Sarathy's idea gets
	     * implemented, this optimization should become
	     * irrelevant.  --jhi */
            if (shift < count)
                shift = count; /* Avoid shifting too often --Ben Tilly */

	    EXTEND(SP,shift);
	    src = SP;
	    dst = (SP += shift);
	    PL_markstack_ptr[-1] += shift;
	    *PL_markstack_ptr += shift;
	    while (count--)
		*dst-- = *src--;
	}
	/* copy the new items down to the destination list */
	dst = PL_stack_base + (PL_markstack_ptr[-2] += items) - 1;
	if (gimme == G_ARRAY) {
	    /* add returned items to the collection (making mortal copies
	     * if necessary), then clear the current temps stack frame
	     * *except* for those items. We do this splicing the items
	     * into the start of the tmps frame (so some items may be on
	     * the tmps stack twice), then moving PL_tmps_floor above
	     * them, then freeing the frame. That way, the only tmps that
	     * accumulate over iterations are the return values for map.
	     * We have to do to this way so that everything gets correctly
	     * freed if we die during the map.
	     */
	    I32 tmpsbase;
	    I32 i = items;
	    /* make space for the slice */
	    EXTEND_MORTAL(items);
	    tmpsbase = PL_tmps_floor + 1;
	    Move(PL_tmps_stack + tmpsbase,
		 PL_tmps_stack + tmpsbase + items,
		 PL_tmps_ix - PL_tmps_floor,
		 SV*);
	    PL_tmps_ix += items;

	    while (i-- > 0) {
		SV *sv = POPs;
		if (!SvTEMP(sv))
		    sv = sv_mortalcopy(sv);
		*dst-- = sv;
		PL_tmps_stack[tmpsbase++] = SvREFCNT_inc_simple(sv);
	    }
	    /* clear the stack frame except for the items */
	    PL_tmps_floor += items;
	    FREETMPS;
	    /* FREETMPS may have cleared the TEMP flag on some of the items */
	    i = items;
	    while (i-- > 0)
		SvTEMP_on(PL_tmps_stack[--tmpsbase]);
	}
	else {
	    /* scalar context: we don't care about which values map returns
	     * (we use undef here). And so we certainly don't want to do mortal
	     * copies of meaningless values. */
	    while (items-- > 0) {
		(void)POPs;
		*dst-- = &PL_sv_undef;
	    }
	    FREETMPS;
	}
    }
    else {
	FREETMPS;
    }
    LEAVE_with_name("grep_item");					/* exit inner scope */

    /* All done yet? */
    if (PL_markstack_ptr[-1] > *PL_markstack_ptr) {

	(void)POPMARK;				/* pop top */
	LEAVE_with_name("grep");					/* exit outer scope */
	(void)POPMARK;				/* pop src */
	items = --*PL_markstack_ptr - PL_markstack_ptr[-1];
	(void)POPMARK;				/* pop dst */
	SP = PL_stack_base + POPMARK;		/* pop original mark */
	if (gimme == G_SCALAR) {
	    if (PL_op->op_private & OPpGREP_LEX) {
		SV* sv = sv_newmortal();
		sv_setiv(sv, items);
		PUSHs(sv);
	    }
	    else {
		dTARGET;
		XPUSHi(items);
	    }
	}
	else if (gimme == G_ARRAY)
	    SP += items;
	RETURN;
    }
    else {
	SV *src;

	ENTER_with_name("grep_item");					/* enter inner scope */
	SAVEVPTR(PL_curpm);

	/* set $_ to the new source item */
	src = PL_stack_base[PL_markstack_ptr[-1]];
	SvTEMP_off(src);
	if (PL_op->op_private & OPpGREP_LEX)
	    PAD_SVl(PL_op->op_targ) = src;
	else
	    DEFSV_set(src);

	RETURNOP(cLOGOP->op_other);
    }
}

/* Range stuff. */

PP(pp_range)
{
    dVAR;
    if (GIMME == G_ARRAY)
	return NORMAL;
    if (SvTRUEx(PAD_SV(PL_op->op_targ)))
	return cLOGOP->op_other;
    else
	return NORMAL;
}

PP(pp_flip)
{
    dVAR;
    dSP;

    if (GIMME == G_ARRAY) {
	RETURNOP(((LOGOP*)cUNOP->op_first)->op_other);
    }
    else {
	dTOPss;
	SV * const targ = PAD_SV(PL_op->op_targ);
	int flip = 0;

	if (PL_op->op_private & OPpFLIP_LINENUM) {
	    if (GvIO(PL_last_in_gv)) {
		flip = SvIV(sv) == (IV)IoLINES(GvIOp(PL_last_in_gv));
	    }
	    else {
		GV * const gv = gv_fetchpvs(".", GV_ADD|GV_NOTQUAL, SVt_PV);
		if (gv && GvSV(gv))
		    flip = SvIV(sv) == SvIV(GvSV(gv));
	    }
	} else {
	    flip = SvTRUE(sv);
	}
	if (flip) {
	    sv_setiv(PAD_SV(cUNOP->op_first->op_targ), 1);
	    if (PL_op->op_flags & OPf_SPECIAL) {
		sv_setiv(targ, 1);
		SETs(targ);
		RETURN;
	    }
	    else {
		sv_setiv(targ, 0);
		SP--;
		RETURNOP(((LOGOP*)cUNOP->op_first)->op_other);
	    }
	}
	sv_setpvs(TARG, "");
	SETs(targ);
	RETURN;
    }
}

/* This code tries to decide if "$left .. $right" should use the
   magical string increment, or if the range is numeric (we make
   an exception for .."0" [#18165]). AMS 20021031. */

#define RANGE_IS_NUMERIC(left,right) ( \
	SvNIOKp(left)  || (SvOK(left)  && !SvPOKp(left))  || \
	SvNIOKp(right) || (SvOK(right) && !SvPOKp(right)) || \
	(((!SvOK(left) && SvOK(right)) || ((!SvOK(left) || \
          looks_like_number(left)) && SvPOKp(left) && *SvPVX_const(left) != '0')) \
         && (!SvOK(right) || looks_like_number(right))))

PP(pp_flop)
{
    dVAR; dSP;

    if (GIMME == G_ARRAY) {
	dPOPPOPssrl;

	SvGETMAGIC(left);
	SvGETMAGIC(right);

	if (RANGE_IS_NUMERIC(left,right)) {
	    register IV i, j;
	    IV max;
	    if ((SvOK(left) && SvNV_nomg(left) < IV_MIN) ||
		(SvOK(right) && SvNV_nomg(right) > IV_MAX))
		DIE(aTHX_ "Range iterator outside integer range");
	    i = SvIV_nomg(left);
	    max = SvIV_nomg(right);
	    if (max >= i) {
		j = max - i + 1;
		EXTEND_MORTAL(j);
		EXTEND(SP, j);
	    }
	    else
		j = 0;
	    while (j--) {
		SV * const sv = sv_2mortal(newSViv(i++));
		PUSHs(sv);
	    }
	}
	else {
	    STRLEN len, llen;
	    const char * const lpv = SvPV_nomg_const(left, llen);
	    const char * const tmps = SvPV_nomg_const(right, len);

	    SV *sv = newSVpvn_flags(lpv, llen, SvUTF8(left)|SVs_TEMP);
	    while (!SvNIOKp(sv) && SvCUR(sv) <= len) {
		XPUSHs(sv);
	        if (strEQ(SvPVX_const(sv),tmps))
	            break;
		sv = sv_2mortal(newSVsv(sv));
		sv_inc(sv);
	    }
	}
    }
    else {
	dTOPss;
	SV * const targ = PAD_SV(cUNOP->op_first->op_targ);
	int flop = 0;
	sv_inc(targ);

	if (PL_op->op_private & OPpFLIP_LINENUM) {
	    if (GvIO(PL_last_in_gv)) {
		flop = SvIV(sv) == (IV)IoLINES(GvIOp(PL_last_in_gv));
	    }
	    else {
		GV * const gv = gv_fetchpvs(".", GV_ADD|GV_NOTQUAL, SVt_PV);
		if (gv && GvSV(gv)) flop = SvIV(sv) == SvIV(GvSV(gv));
	    }
	}
	else {
	    flop = SvTRUE(sv);
	}

	if (flop) {
	    sv_setiv(PAD_SV(((UNOP*)cUNOP->op_first)->op_first->op_targ), 0);
	    sv_catpvs(targ, "E0");
	}
	SETs(targ);
    }

    RETURN;
}

/* Control. */

static const char * const context_name[] = {
    "pseudo-block",
    NULL, /* CXt_WHEN never actually needs "block" */
    NULL, /* CXt_BLOCK never actually needs "block" */
    NULL, /* CXt_GIVEN never actually needs "block" */
    NULL, /* CXt_LOOP_FOR never actually needs "loop" */
    NULL, /* CXt_LOOP_PLAIN never actually needs "loop" */
    NULL, /* CXt_LOOP_LAZYSV never actually needs "loop" */
    NULL, /* CXt_LOOP_LAZYIV never actually needs "loop" */
    "subroutine",
    "format",
    "eval",
    "substitution",
};

STATIC I32
S_dopoptolabel(pTHX_ const char *label, STRLEN len, U32 flags)
{
    dVAR;
    register I32 i;

    PERL_ARGS_ASSERT_DOPOPTOLABEL;

    for (i = cxstack_ix; i >= 0; i--) {
	register const PERL_CONTEXT * const cx = &cxstack[i];
	switch (CxTYPE(cx)) {
	case CXt_SUBST:
	case CXt_SUB:
	case CXt_FORMAT:
	case CXt_EVAL:
	case CXt_NULL:
	    /* diag_listed_as: Exiting subroutine via %s */
	    Perl_ck_warner(aTHX_ packWARN(WARN_EXITING), "Exiting %s via %s",
			   context_name[CxTYPE(cx)], OP_NAME(PL_op));
	    if (CxTYPE(cx) == CXt_NULL)
		return -1;
	    break;
	case CXt_LOOP_LAZYIV:
	case CXt_LOOP_LAZYSV:
	case CXt_LOOP_FOR:
	case CXt_LOOP_PLAIN:
	  {
            STRLEN cx_label_len = 0;
            U32 cx_label_flags = 0;
	    const char *cx_label = CxLABEL_len_flags(cx, &cx_label_len, &cx_label_flags);
	    if (!cx_label || !(
                    ( (cx_label_flags & SVf_UTF8) != (flags & SVf_UTF8) ) ?
                        (flags & SVf_UTF8)
                            ? (bytes_cmp_utf8(
                                        (const U8*)cx_label, cx_label_len,
                                        (const U8*)label, len) == 0)
                            : (bytes_cmp_utf8(
                                        (const U8*)label, len,
                                        (const U8*)cx_label, cx_label_len) == 0)
                    : (len == cx_label_len && ((cx_label == label)
                                    || memEQ(cx_label, label, len))) )) {
		DEBUG_l(Perl_deb(aTHX_ "(poptolabel(): skipping label at cx=%ld %s)\n",
			(long)i, cx_label));
		continue;
	    }
	    DEBUG_l( Perl_deb(aTHX_ "(poptolabel(): found label at cx=%ld %s)\n", (long)i, label));
	    return i;
	  }
	}
    }
    return i;
}



I32
Perl_dowantarray(pTHX)
{
    dVAR;
    const I32 gimme = block_gimme();
    return (gimme == G_VOID) ? G_SCALAR : gimme;
}

I32
Perl_block_gimme(pTHX)
{
    dVAR;
    const I32 cxix = dopoptosub(cxstack_ix);
    if (cxix < 0)
	return G_VOID;

    switch (cxstack[cxix].blk_gimme) {
    case G_VOID:
	return G_VOID;
    case G_SCALAR:
	return G_SCALAR;
    case G_ARRAY:
	return G_ARRAY;
    default:
	Perl_croak(aTHX_ "panic: bad gimme: %d\n", cxstack[cxix].blk_gimme);
	/* NOTREACHED */
	return 0;
    }
}

I32
Perl_is_lvalue_sub(pTHX)
{
    dVAR;
    const I32 cxix = dopoptosub(cxstack_ix);
    assert(cxix >= 0);  /* We should only be called from inside subs */

    if (CxLVAL(cxstack + cxix) && CvLVALUE(cxstack[cxix].blk_sub.cv))
	return CxLVAL(cxstack + cxix);
    else
	return 0;
}

/* only used by PUSHSUB */
I32
Perl_was_lvalue_sub(pTHX)
{
    dVAR;
    const I32 cxix = dopoptosub(cxstack_ix-1);
    assert(cxix >= 0);  /* We should only be called from inside subs */

    if (CxLVAL(cxstack + cxix) && CvLVALUE(cxstack[cxix].blk_sub.cv))
	return CxLVAL(cxstack + cxix);
    else
	return 0;
}

STATIC I32
S_dopoptosub_at(pTHX_ const PERL_CONTEXT *cxstk, I32 startingblock)
{
    dVAR;
    I32 i;

    PERL_ARGS_ASSERT_DOPOPTOSUB_AT;

    for (i = startingblock; i >= 0; i--) {
	register const PERL_CONTEXT * const cx = &cxstk[i];
	switch (CxTYPE(cx)) {
	default:
	    continue;
	case CXt_EVAL:
	case CXt_SUB:
	case CXt_FORMAT:
	    DEBUG_l( Perl_deb(aTHX_ "(dopoptosub_at(): found sub at cx=%ld)\n", (long)i));
	    return i;
	}
    }
    return i;
}

STATIC I32
S_dopoptoeval(pTHX_ I32 startingblock)
{
    dVAR;
    I32 i;
    for (i = startingblock; i >= 0; i--) {
	register const PERL_CONTEXT *cx = &cxstack[i];
	switch (CxTYPE(cx)) {
	default:
	    continue;
	case CXt_EVAL:
	    DEBUG_l( Perl_deb(aTHX_ "(dopoptoeval(): found eval at cx=%ld)\n", (long)i));
	    return i;
	}
    }
    return i;
}

STATIC I32
S_dopoptoloop(pTHX_ I32 startingblock)
{
    dVAR;
    I32 i;
    for (i = startingblock; i >= 0; i--) {
	register const PERL_CONTEXT * const cx = &cxstack[i];
	switch (CxTYPE(cx)) {
	case CXt_SUBST:
	case CXt_SUB:
	case CXt_FORMAT:
	case CXt_EVAL:
	case CXt_NULL:
	    /* diag_listed_as: Exiting subroutine via %s */
	    Perl_ck_warner(aTHX_ packWARN(WARN_EXITING), "Exiting %s via %s",
			   context_name[CxTYPE(cx)], OP_NAME(PL_op));
	    if ((CxTYPE(cx)) == CXt_NULL)
		return -1;
	    break;
	case CXt_LOOP_LAZYIV:
	case CXt_LOOP_LAZYSV:
	case CXt_LOOP_FOR:
	case CXt_LOOP_PLAIN:
	    DEBUG_l( Perl_deb(aTHX_ "(dopoptoloop(): found loop at cx=%ld)\n", (long)i));
	    return i;
	}
    }
    return i;
}

STATIC I32
S_dopoptogiven(pTHX_ I32 startingblock)
{
    dVAR;
    I32 i;
    for (i = startingblock; i >= 0; i--) {
	register const PERL_CONTEXT *cx = &cxstack[i];
	switch (CxTYPE(cx)) {
	default:
	    continue;
	case CXt_GIVEN:
	    DEBUG_l( Perl_deb(aTHX_ "(dopoptogiven(): found given at cx=%ld)\n", (long)i));
	    return i;
	case CXt_LOOP_PLAIN:
	    assert(!CxFOREACHDEF(cx));
	    break;
	case CXt_LOOP_LAZYIV:
	case CXt_LOOP_LAZYSV:
	case CXt_LOOP_FOR:
	    if (CxFOREACHDEF(cx)) {
		DEBUG_l( Perl_deb(aTHX_ "(dopoptogiven(): found foreach at cx=%ld)\n", (long)i));
		return i;
	    }
	}
    }
    return i;
}

STATIC I32
S_dopoptowhen(pTHX_ I32 startingblock)
{
    dVAR;
    I32 i;
    for (i = startingblock; i >= 0; i--) {
	register const PERL_CONTEXT *cx = &cxstack[i];
	switch (CxTYPE(cx)) {
	default:
	    continue;
	case CXt_WHEN:
	    DEBUG_l( Perl_deb(aTHX_ "(dopoptowhen(): found when at cx=%ld)\n", (long)i));
	    return i;
	}
    }
    return i;
}

void
Perl_dounwind(pTHX_ I32 cxix)
{
    dVAR;
    I32 optype;

    if (!PL_curstackinfo) /* can happen if die during thread cloning */
	return;

    while (cxstack_ix > cxix) {
	SV *sv;
        register PERL_CONTEXT *cx = &cxstack[cxstack_ix];
	DEBUG_CX("UNWIND");						\
	/* Note: we don't need to restore the base context info till the end. */
	switch (CxTYPE(cx)) {
	case CXt_SUBST:
	    POPSUBST(cx);
	    continue;  /* not break */
	case CXt_SUB:
	    POPSUB(cx,sv);
	    LEAVESUB(sv);
	    break;
	case CXt_EVAL:
	    POPEVAL(cx);
	    break;
	case CXt_LOOP_LAZYIV:
	case CXt_LOOP_LAZYSV:
	case CXt_LOOP_FOR:
	case CXt_LOOP_PLAIN:
	    POPLOOP(cx);
	    break;
	case CXt_NULL:
	    break;
	case CXt_FORMAT:
	    POPFORMAT(cx);
	    break;
	}
	cxstack_ix--;
    }
    PERL_UNUSED_VAR(optype);
}

void
Perl_qerror(pTHX_ SV *err)
{
    dVAR;

    PERL_ARGS_ASSERT_QERROR;

    if (PL_in_eval) {
	if (PL_in_eval & EVAL_KEEPERR) {
		Perl_ck_warner(aTHX_ packWARN(WARN_MISC), "\t(in cleanup) %"SVf,
                                                    SVfARG(err));
	}
	else
	    sv_catsv(ERRSV, err);
    }
    else if (PL_errors)
	sv_catsv(PL_errors, err);
    else
	Perl_warn(aTHX_ "%"SVf, SVfARG(err));
    if (PL_parser)
	++PL_parser->error_count;
}

void
Perl_die_unwind(pTHX_ SV *msv)
{
    dVAR;
    SV *exceptsv = sv_mortalcopy(msv);
    U8 in_eval = PL_in_eval;
    PERL_ARGS_ASSERT_DIE_UNWIND;

    if (in_eval) {
	I32 cxix;
	I32 gimme;

	/*
	 * Historically, perl used to set ERRSV ($@) early in the die
	 * process and rely on it not getting clobbered during unwinding.
	 * That sucked, because it was liable to get clobbered, so the
	 * setting of ERRSV used to emit the exception from eval{} has
	 * been moved to much later, after unwinding (see just before
	 * JMPENV_JUMP below).	However, some modules were relying on the
	 * early setting, by examining $@ during unwinding to use it as
	 * a flag indicating whether the current unwinding was caused by
	 * an exception.  It was never a reliable flag for that purpose,
	 * being totally open to false positives even without actual
	 * clobberage, but was useful enough for production code to
	 * semantically rely on it.
	 *
	 * We'd like to have a proper introspective interface that
	 * explicitly describes the reason for whatever unwinding
	 * operations are currently in progress, so that those modules
	 * work reliably and $@ isn't further overloaded.  But we don't
	 * have one yet.  In its absence, as a stopgap measure, ERRSV is
	 * now *additionally* set here, before unwinding, to serve as the
	 * (unreliable) flag that it used to.
	 *
	 * This behaviour is temporary, and should be removed when a
	 * proper way to detect exceptional unwinding has been developed.
	 * As of 2010-12, the authors of modules relying on the hack
	 * are aware of the issue, because the modules failed on
	 * perls 5.13.{1..7} which had late setting of $@ without this
	 * early-setting hack.
	 */
	if (!(in_eval & EVAL_KEEPERR)) {
	    SvTEMP_off(exceptsv);
	    sv_setsv(ERRSV, exceptsv);
	}

	while ((cxix = dopoptoeval(cxstack_ix)) < 0
	       && PL_curstackinfo->si_prev)
	{
	    dounwind(-1);
	    POPSTACK;
	}

	if (cxix >= 0) {
	    I32 optype;
	    SV *namesv;
	    register PERL_CONTEXT *cx;
	    SV **newsp;
	    COP *oldcop;
	    JMPENV *restartjmpenv;
	    OP *restartop;

	    if (cxix < cxstack_ix)
		dounwind(cxix);

	    POPBLOCK(cx,PL_curpm);
	    if (CxTYPE(cx) != CXt_EVAL) {
		STRLEN msglen;
		const char* message = SvPVx_const(exceptsv, msglen);
		PerlIO_write(Perl_error_log, (const char *)"panic: die ", 11);
		PerlIO_write(Perl_error_log, message, msglen);
		my_exit(1);
	    }
	    POPEVAL(cx);
	    namesv = cx->blk_eval.old_namesv;
	    oldcop = cx->blk_oldcop;
	    restartjmpenv = cx->blk_eval.cur_top_env;
	    restartop = cx->blk_eval.retop;

	    if (gimme == G_SCALAR)
		*++newsp = &PL_sv_undef;
	    PL_stack_sp = newsp;

	    LEAVE;

	    /* LEAVE could clobber PL_curcop (see save_re_context())
	     * XXX it might be better to find a way to avoid messing with
	     * PL_curcop in save_re_context() instead, but this is a more
	     * minimal fix --GSAR */
	    PL_curcop = oldcop;

	    if (optype == OP_REQUIRE) {
                (void)hv_store(GvHVn(PL_incgv),
                               SvPVX_const(namesv),
                               SvUTF8(namesv) ? -(I32)SvCUR(namesv) : (I32)SvCUR(namesv),
                               &PL_sv_undef, 0);
		/* note that unlike pp_entereval, pp_require isn't
		 * supposed to trap errors. So now that we've popped the
		 * EVAL that pp_require pushed, and processed the error
		 * message, rethrow the error */
		Perl_croak(aTHX_ "%"SVf"Compilation failed in require",
			   SVfARG(exceptsv ? exceptsv : newSVpvs_flags("Unknown error\n",
                                                                    SVs_TEMP)));
	    }
	    if (in_eval & EVAL_KEEPERR) {
		Perl_ck_warner(aTHX_ packWARN(WARN_MISC), "\t(in cleanup) %"SVf,
			       SVfARG(exceptsv));
	    }
	    else {
		sv_setsv(ERRSV, exceptsv);
	    }
	    PL_restartjmpenv = restartjmpenv;
	    PL_restartop = restartop;
	    JMPENV_JUMP(3);
	    /* NOTREACHED */
	}
    }

    write_to_stderr(exceptsv);
    my_failure_exit();
    /* NOTREACHED */
}

PP(pp_xor)
{
    dVAR; dSP; dPOPTOPssrl;
    if (SvTRUE(left) != SvTRUE(right))
	RETSETYES;
    else
	RETSETNO;
}

/*
=for apidoc caller_cx

The XSUB-writer's equivalent of L<caller()|perlfunc/caller>. The
returned C<PERL_CONTEXT> structure can be interrogated to find all the
information returned to Perl by C<caller>. Note that XSUBs don't get a
stack frame, so C<caller_cx(0, NULL)> will return information for the
immediately-surrounding Perl code.

This function skips over the automatic calls to C<&DB::sub> made on the
behalf of the debugger. If the stack frame requested was a sub called by
C<DB::sub>, the return value will be the frame for the call to
C<DB::sub>, since that has the correct line number/etc. for the call
site. If I<dbcxp> is non-C<NULL>, it will be set to a pointer to the
frame for the sub call itself.

=cut
*/

const PERL_CONTEXT *
Perl_caller_cx(pTHX_ I32 count, const PERL_CONTEXT **dbcxp)
{
    register I32 cxix = dopoptosub(cxstack_ix);
    register const PERL_CONTEXT *cx;
    register const PERL_CONTEXT *ccstack = cxstack;
    const PERL_SI *top_si = PL_curstackinfo;

    for (;;) {
	/* we may be in a higher stacklevel, so dig down deeper */
	while (cxix < 0 && top_si->si_type != PERLSI_MAIN) {
	    top_si = top_si->si_prev;
	    ccstack = top_si->si_cxstack;
	    cxix = dopoptosub_at(ccstack, top_si->si_cxix);
	}
	if (cxix < 0)
	    return NULL;
	/* caller() should not report the automatic calls to &DB::sub */
	if (PL_DBsub && GvCV(PL_DBsub) && cxix >= 0 &&
		ccstack[cxix].blk_sub.cv == GvCV(PL_DBsub))
	    count++;
	if (!count--)
	    break;
	cxix = dopoptosub_at(ccstack, cxix - 1);
    }

    cx = &ccstack[cxix];
    if (dbcxp) *dbcxp = cx;

    if (CxTYPE(cx) == CXt_SUB || CxTYPE(cx) == CXt_FORMAT) {
        const I32 dbcxix = dopoptosub_at(ccstack, cxix - 1);
	/* We expect that ccstack[dbcxix] is CXt_SUB, anyway, the
	   field below is defined for any cx. */
	/* caller() should not report the automatic calls to &DB::sub */
	if (PL_DBsub && GvCV(PL_DBsub) && dbcxix >= 0 && ccstack[dbcxix].blk_sub.cv == GvCV(PL_DBsub))
	    cx = &ccstack[dbcxix];
    }

    return cx;
}

PP(pp_caller)
{
    dVAR;
    dSP;
    register const PERL_CONTEXT *cx;
    const PERL_CONTEXT *dbcx;
    I32 gimme;
    const HEK *stash_hek;
    I32 count = 0;
    bool has_arg = MAXARG && TOPs;

    if (MAXARG) {
      if (has_arg)
	count = POPi;
      else (void)POPs;
    }

    cx = caller_cx(count + !!(PL_op->op_private & OPpOFFBYONE), &dbcx);
    if (!cx) {
	if (GIMME != G_ARRAY) {
	    EXTEND(SP, 1);
	    RETPUSHUNDEF;
	}
	RETURN;
    }

    stash_hek = HvNAME_HEK((HV*)CopSTASH(cx->blk_oldcop));
    if (GIMME != G_ARRAY) {
        EXTEND(SP, 1);
	if (!stash_hek)
	    PUSHs(&PL_sv_undef);
	else {
	    dTARGET;
	    sv_sethek(TARG, stash_hek);
	    PUSHs(TARG);
	}
	RETURN;
    }

    EXTEND(SP, 11);

    if (!stash_hek)
	PUSHs(&PL_sv_undef);
    else {
	dTARGET;
	sv_sethek(TARG, stash_hek);
	PUSHTARG;
    }
    mPUSHs(newSVpv(OutCopFILE(cx->blk_oldcop), 0));
    mPUSHi((I32)CopLINE(cx->blk_oldcop));
    if (!has_arg)
	RETURN;
    if (CxTYPE(cx) == CXt_SUB || CxTYPE(cx) == CXt_FORMAT) {
	GV * const cvgv = CvGV(dbcx->blk_sub.cv);
	/* So is ccstack[dbcxix]. */
	if (isGV(cvgv)) {
	    SV * const sv = newSV(0);
	    gv_efullname3(sv, cvgv, NULL);
	    mPUSHs(sv);
	    PUSHs(boolSV(CxHASARGS(cx)));
	}
	else {
	    PUSHs(newSVpvs_flags("(unknown)", SVs_TEMP));
	    PUSHs(boolSV(CxHASARGS(cx)));
	}
    }
    else {
	PUSHs(newSVpvs_flags("(eval)", SVs_TEMP));
	mPUSHi(0);
    }
    gimme = (I32)cx->blk_gimme;
    if (gimme == G_VOID)
	PUSHs(&PL_sv_undef);
    else
	PUSHs(boolSV((gimme & G_WANT) == G_ARRAY));
    if (CxTYPE(cx) == CXt_EVAL) {
	/* eval STRING */
	if (CxOLD_OP_TYPE(cx) == OP_ENTEREVAL) {
	    PUSHs(cx->blk_eval.cur_text);
	    PUSHs(&PL_sv_no);
	}
	/* require */
	else if (cx->blk_eval.old_namesv) {
	    mPUSHs(newSVsv(cx->blk_eval.old_namesv));
	    PUSHs(&PL_sv_yes);
	}
	/* eval BLOCK (try blocks have old_namesv == 0) */
	else {
	    PUSHs(&PL_sv_undef);
	    PUSHs(&PL_sv_undef);
	}
    }
    else {
	PUSHs(&PL_sv_undef);
	PUSHs(&PL_sv_undef);
    }
    if (CxTYPE(cx) == CXt_SUB && CxHASARGS(cx)
	&& CopSTASH_eq(PL_curcop, PL_debstash))
    {
	AV * const ary = cx->blk_sub.argarray;
	const int off = AvARRAY(ary) - AvALLOC(ary);

	Perl_init_dbargs(aTHX);

	if (AvMAX(PL_dbargs) < AvFILLp(ary) + off)
	    av_extend(PL_dbargs, AvFILLp(ary) + off);
	Copy(AvALLOC(ary), AvARRAY(PL_dbargs), AvFILLp(ary) + 1 + off, SV*);
	AvFILLp(PL_dbargs) = AvFILLp(ary) + off;
    }
    /* XXX only hints propagated via op_private are currently
     * visible (others are not easily accessible, since they
     * use the global PL_hints) */
    mPUSHi(CopHINTS_get(cx->blk_oldcop));
    {
	SV * mask ;
	STRLEN * const old_warnings = cx->blk_oldcop->cop_warnings ;

	if  (old_warnings == pWARN_NONE ||
		(old_warnings == pWARN_STD && (PL_dowarn & G_WARN_ON) == 0))
            mask = newSVpvn(WARN_NONEstring, WARNsize) ;
        else if (old_warnings == pWARN_ALL ||
		  (old_warnings == pWARN_STD && PL_dowarn & G_WARN_ON)) {
	    /* Get the bit mask for $warnings::Bits{all}, because
	     * it could have been extended by warnings::register */
	    SV **bits_all;
	    HV * const bits = get_hv("warnings::Bits", 0);
	    if (bits && (bits_all=hv_fetchs(bits, "all", FALSE))) {
		mask = newSVsv(*bits_all);
	    }
	    else {
		mask = newSVpvn(WARN_ALLstring, WARNsize) ;
	    }
	}
        else
            mask = newSVpvn((char *) (old_warnings + 1), old_warnings[0]);
        mPUSHs(mask);
    }

    PUSHs(cx->blk_oldcop->cop_hints_hash ?
	  sv_2mortal(newRV_noinc(MUTABLE_SV(cop_hints_2hv(cx->blk_oldcop, 0))))
	  : &PL_sv_undef);
    RETURN;
}

PP(pp_reset)
{
    dVAR;
    dSP;
    const char * const tmps =
	(MAXARG < 1 || (!TOPs && !POPs)) ? (const char *)"" : POPpconstx;
    sv_reset(tmps, CopSTASH(PL_curcop));
    PUSHs(&PL_sv_yes);
    RETURN;
}

/* like pp_nextstate, but used instead when the debugger is active */

PP(pp_dbstate)
{
    dVAR;
    PL_curcop = (COP*)PL_op;
    TAINT_NOT;		/* Each statement is presumed innocent */
    PL_stack_sp = PL_stack_base + cxstack[cxstack_ix].blk_oldsp;
    FREETMPS;

    PERL_ASYNC_CHECK();

    if (PL_op->op_flags & OPf_SPECIAL /* breakpoint */
	    || SvIV(PL_DBsingle) || SvIV(PL_DBsignal) || SvIV(PL_DBtrace))
    {
	dSP;
	register PERL_CONTEXT *cx;
	const I32 gimme = G_ARRAY;
	U8 hasargs;
	GV * const gv = PL_DBgv;
	register CV * const cv = GvCV(gv);

	if (!cv)
	    DIE(aTHX_ "No DB::DB routine defined");

	if (CvDEPTH(cv) >= 1 && !(PL_debug & DEBUG_DB_RECURSE_FLAG))
	    /* don't do recursive DB::DB call */
	    return NORMAL;

	ENTER;
	SAVETMPS;

	SAVEI32(PL_debug);
	SAVESTACK_POS();
	PL_debug = 0;
	hasargs = 0;
	SPAGAIN;

	if (CvISXSUB(cv)) {
	    CvDEPTH(cv)++;
	    PUSHMARK(SP);
	    (void)(*CvXSUB(cv))(aTHX_ cv);
	    CvDEPTH(cv)--;
	    FREETMPS;
	    LEAVE;
	    return NORMAL;
	}
	else {
	    PUSHBLOCK(cx, CXt_SUB, SP);
	    PUSHSUB_DB(cx);
	    cx->blk_sub.retop = PL_op->op_next;
	    CvDEPTH(cv)++;
	    SAVECOMPPAD();
	    PAD_SET_CUR_NOSAVE(CvPADLIST(cv), 1);
	    RETURNOP(CvSTART(cv));
	}
    }
    else
	return NORMAL;
}

STATIC SV **
S_adjust_stack_on_leave(pTHX_ SV **newsp, SV **sp, SV **mark, I32 gimme, U32 flags)
{
    bool padtmp = 0;
    PERL_ARGS_ASSERT_ADJUST_STACK_ON_LEAVE;

    if (flags & SVs_PADTMP) {
	flags &= ~SVs_PADTMP;
	padtmp = 1;
    }
    if (gimme == G_SCALAR) {
	if (MARK < SP)
	    *++newsp = ((SvFLAGS(*SP) & flags) || (padtmp && SvPADTMP(*SP)))
			    ? *SP : sv_mortalcopy(*SP);
	else {
	    /* MEXTEND() only updates MARK, so reuse it instead of newsp. */
	    MARK = newsp;
	    MEXTEND(MARK, 1);
	    *++MARK = &PL_sv_undef;
	    return MARK;
	}
    }
    else if (gimme == G_ARRAY) {
	/* in case LEAVE wipes old return values */
	while (++MARK <= SP) {
	    if ((SvFLAGS(*MARK) & flags) || (padtmp && SvPADTMP(*MARK)))
		*++newsp = *MARK;
	    else {
		*++newsp = sv_mortalcopy(*MARK);
		TAINT_NOT;	/* Each item is independent */
	    }
	}
	/* When this function was called with MARK == newsp, we reach this
	 * point with SP == newsp. */
    }

    return newsp;
}

PP(pp_enter)
{
    dVAR; dSP;
    register PERL_CONTEXT *cx;
    I32 gimme = GIMME_V;

    ENTER_with_name("block");

    SAVETMPS;
    PUSHBLOCK(cx, CXt_BLOCK, SP);

    RETURN;
}

PP(pp_leave)
{
    dVAR; dSP;
    register PERL_CONTEXT *cx;
    SV **newsp;
    PMOP *newpm;
    I32 gimme;

    if (PL_op->op_flags & OPf_SPECIAL) {
	cx = &cxstack[cxstack_ix];
	cx->blk_oldpm = PL_curpm;	/* fake block should preserve $1 et al */
    }

    POPBLOCK(cx,newpm);

    gimme = OP_GIMME(PL_op, (cxstack_ix >= 0) ? gimme : G_SCALAR);

    TAINT_NOT;
    SP = adjust_stack_on_leave(newsp, SP, newsp, gimme, SVs_PADTMP|SVs_TEMP);
    PL_curpm = newpm;	/* Don't pop $1 et al till now */

    LEAVE_with_name("block");

    RETURN;
}

PP(pp_enteriter)
{
    dVAR; dSP; dMARK;
    register PERL_CONTEXT *cx;
    const I32 gimme = GIMME_V;
    void *itervar; /* location of the iteration variable */
    U8 cxtype = CXt_LOOP_FOR;

    ENTER_with_name("loop1");
    SAVETMPS;

    if (PL_op->op_targ) {			 /* "my" variable */
	if (PL_op->op_private & OPpLVAL_INTRO) {        /* for my $x (...) */
	    SvPADSTALE_off(PAD_SVl(PL_op->op_targ));
	    SAVESETSVFLAGS(PAD_SVl(PL_op->op_targ),
		    SVs_PADSTALE, SVs_PADSTALE);
	}
	SAVEPADSVANDMORTALIZE(PL_op->op_targ);
#ifdef USE_ITHREADS
	itervar = PL_comppad;
#else
	itervar = &PAD_SVl(PL_op->op_targ);
#endif
    }
    else {					/* symbol table variable */
	GV * const gv = MUTABLE_GV(POPs);
	SV** svp = &GvSV(gv);
	save_pushptrptr(gv, SvREFCNT_inc(*svp), SAVEt_GVSV);
	*svp = newSV(0);
	itervar = (void *)gv;
    }

    if (PL_op->op_private & OPpITER_DEF)
	cxtype |= CXp_FOR_DEF;

    ENTER_with_name("loop2");

    PUSHBLOCK(cx, cxtype, SP);
    PUSHLOOP_FOR(cx, itervar, MARK);
    if (PL_op->op_flags & OPf_STACKED) {
	SV *maybe_ary = POPs;
	if (SvTYPE(maybe_ary) != SVt_PVAV) {
	    dPOPss;
	    SV * const right = maybe_ary;
	    SvGETMAGIC(sv);
	    SvGETMAGIC(right);
	    if (RANGE_IS_NUMERIC(sv,right)) {
		cx->cx_type &= ~CXTYPEMASK;
		cx->cx_type |= CXt_LOOP_LAZYIV;
		/* Make sure that no-one re-orders cop.h and breaks our
		   assumptions */
		assert(CxTYPE(cx) == CXt_LOOP_LAZYIV);
#ifdef NV_PRESERVES_UV
		if ((SvOK(sv) && ((SvNV_nomg(sv) < (NV)IV_MIN) ||
				  (SvNV_nomg(sv) > (NV)IV_MAX)))
			||
		    (SvOK(right) && ((SvNV_nomg(right) > (NV)IV_MAX) ||
				     (SvNV_nomg(right) < (NV)IV_MIN))))
#else
		if ((SvOK(sv) && ((SvNV_nomg(sv) <= (NV)IV_MIN)
				  ||
		                  ((SvNV_nomg(sv) > 0) &&
					((SvUV_nomg(sv) > (UV)IV_MAX) ||
					 (SvNV_nomg(sv) > (NV)UV_MAX)))))
			||
		    (SvOK(right) && ((SvNV_nomg(right) <= (NV)IV_MIN)
				     ||
				     ((SvNV_nomg(right) > 0) &&
					((SvUV_nomg(right) > (UV)IV_MAX) ||
					 (SvNV_nomg(right) > (NV)UV_MAX))
				     ))))
#endif
		    DIE(aTHX_ "Range iterator outside integer range");
		cx->blk_loop.state_u.lazyiv.cur = SvIV_nomg(sv);
		cx->blk_loop.state_u.lazyiv.end = SvIV_nomg(right);
#ifdef DEBUGGING
		/* for correct -Dstv display */
		cx->blk_oldsp = sp - PL_stack_base;
#endif
	    }
	    else {
		cx->cx_type &= ~CXTYPEMASK;
		cx->cx_type |= CXt_LOOP_LAZYSV;
		/* Make sure that no-one re-orders cop.h and breaks our
		   assumptions */
		assert(CxTYPE(cx) == CXt_LOOP_LAZYSV);
		cx->blk_loop.state_u.lazysv.cur = newSVsv(sv);
		cx->blk_loop.state_u.lazysv.end = right;
		SvREFCNT_inc(right);
		(void) SvPV_force_nolen(cx->blk_loop.state_u.lazysv.cur);
		/* This will do the upgrade to SVt_PV, and warn if the value
		   is uninitialised.  */
		(void) SvPV_nolen_const(right);
		/* Doing this avoids a check every time in pp_iter in pp_hot.c
		   to replace !SvOK() with a pointer to "".  */
		if (!SvOK(right)) {
		    SvREFCNT_dec(right);
		    cx->blk_loop.state_u.lazysv.end = &PL_sv_no;
		}
	    }
	}
	else /* SvTYPE(maybe_ary) == SVt_PVAV */ {
	    cx->blk_loop.state_u.ary.ary = MUTABLE_AV(maybe_ary);
	    SvREFCNT_inc(maybe_ary);
	    cx->blk_loop.state_u.ary.ix =
		(PL_op->op_private & OPpITER_REVERSED) ?
		AvFILL(cx->blk_loop.state_u.ary.ary) + 1 :
		-1;
	}
    }
    else { /* iterating over items on the stack */
	cx->blk_loop.state_u.ary.ary = NULL; /* means to use the stack */
	if (PL_op->op_private & OPpITER_REVERSED) {
	    cx->blk_loop.state_u.ary.ix = cx->blk_oldsp + 1;
	}
	else {
	    cx->blk_loop.state_u.ary.ix = MARK - PL_stack_base;
	}
    }

    RETURN;
}

PP(pp_enterloop)
{
    dVAR; dSP;
    register PERL_CONTEXT *cx;
    const I32 gimme = GIMME_V;

    ENTER_with_name("loop1");
    SAVETMPS;
    ENTER_with_name("loop2");

    PUSHBLOCK(cx, CXt_LOOP_PLAIN, SP);
    PUSHLOOP_PLAIN(cx, SP);

    RETURN;
}

PP(pp_leaveloop)
{
    dVAR; dSP;
    register PERL_CONTEXT *cx;
    I32 gimme;
    SV **newsp;
    PMOP *newpm;
    SV **mark;

    POPBLOCK(cx,newpm);
    assert(CxTYPE_is_LOOP(cx));
    mark = newsp;
    newsp = PL_stack_base + cx->blk_loop.resetsp;

    TAINT_NOT;
    SP = adjust_stack_on_leave(newsp, SP, MARK, gimme, 0);
    PUTBACK;

    POPLOOP(cx);	/* Stack values are safe: release loop vars ... */
    PL_curpm = newpm;	/* ... and pop $1 et al */

    LEAVE_with_name("loop2");
    LEAVE_with_name("loop1");

    return NORMAL;
}

STATIC void
S_return_lvalues(pTHX_ SV **mark, SV **sp, SV **newsp, I32 gimme,
                       PERL_CONTEXT *cx, PMOP *newpm)
{
    const bool ref = !!(CxLVAL(cx) & OPpENTERSUB_INARGS);
    if (gimme == G_SCALAR) {
	if (CxLVAL(cx) && !ref) {     /* Leave it as it is if we can. */
	    SV *sv;
	    const char *what = NULL;
	    if (MARK < SP) {
		assert(MARK+1 == SP);
		if ((SvPADTMP(TOPs) ||
		     (SvFLAGS(TOPs) & (SVf_READONLY | SVf_FAKE))
		       == SVf_READONLY
		    ) &&
		    !SvSMAGICAL(TOPs)) {
		    what =
			SvREADONLY(TOPs) ? (TOPs == &PL_sv_undef) ? "undef"
			: "a readonly value" : "a temporary";
		}
		else goto copy_sv;
	    }
	    else {
		/* sub:lvalue{} will take us here. */
		what = "undef";
	    }
	    LEAVE;
	    cxstack_ix--;
	    POPSUB(cx,sv);
	    PL_curpm = newpm;
	    LEAVESUB(sv);
	    Perl_croak(aTHX_
	              "Can't return %s from lvalue subroutine", what
	    );
	}
	if (MARK < SP) {
	      copy_sv:
		if (cx->blk_sub.cv && CvDEPTH(cx->blk_sub.cv) > 1) {
		    if (!SvPADTMP(*SP)) {
			*++newsp = SvREFCNT_inc(*SP);
			FREETMPS;
			sv_2mortal(*newsp);
		    }
		    else {
			/* FREETMPS could clobber it */
			SV *sv = SvREFCNT_inc(*SP);
			FREETMPS;
			*++newsp = sv_mortalcopy(sv);
			SvREFCNT_dec(sv);
		    }
		}
		else
		    *++newsp =
		      SvPADTMP(*SP)
		       ? sv_mortalcopy(*SP)
		       : !SvTEMP(*SP)
		          ? sv_2mortal(SvREFCNT_inc_simple_NN(*SP))
		          : *SP;
	}
	else {
	    EXTEND(newsp,1);
	    *++newsp = &PL_sv_undef;
	}
	if (CxLVAL(cx) & OPpDEREF) {
	    SvGETMAGIC(TOPs);
	    if (!SvOK(TOPs)) {
		TOPs = vivify_ref(TOPs, CxLVAL(cx) & OPpDEREF);
	    }
	}
    }
    else if (gimme == G_ARRAY) {
	assert (!(CxLVAL(cx) & OPpDEREF));
	if (ref || !CxLVAL(cx))
	    while (++MARK <= SP)
		*++newsp =
		       SvFLAGS(*MARK) & SVs_PADTMP
		           ? sv_mortalcopy(*MARK)
		     : SvTEMP(*MARK)
		           ? *MARK
		           : sv_2mortal(SvREFCNT_inc_simple_NN(*MARK));
	else while (++MARK <= SP) {
	    if (*MARK != &PL_sv_undef
		    && (SvPADTMP(*MARK)
		       || (SvFLAGS(*MARK) & (SVf_READONLY|SVf_FAKE))
		             == SVf_READONLY
		       )
	    ) {
		    SV *sv;
		    /* Might be flattened array after $#array =  */
		    PUTBACK;
		    LEAVE;
		    cxstack_ix--;
		    POPSUB(cx,sv);
		    PL_curpm = newpm;
		    LEAVESUB(sv);
	       /* diag_listed_as: Can't return %s from lvalue subroutine */
		    Perl_croak(aTHX_
			"Can't return a %s from lvalue subroutine",
			SvREADONLY(TOPs) ? "readonly value" : "temporary");
	    }
	    else
		*++newsp =
		    SvTEMP(*MARK)
		       ? *MARK
		       : sv_2mortal(SvREFCNT_inc_simple_NN(*MARK));
	}
    }
    PL_stack_sp = newsp;
}

PP(pp_return)
{
    dVAR; dSP; dMARK;
    register PERL_CONTEXT *cx;
    bool popsub2 = FALSE;
    bool clear_errsv = FALSE;
    bool lval = FALSE;
    I32 gimme;
    SV **newsp;
    PMOP *newpm;
    I32 optype = 0;
    SV *namesv;
    SV *sv;
    OP *retop = NULL;

    const I32 cxix = dopoptosub(cxstack_ix);

    if (cxix < 0) {
	if (CxMULTICALL(cxstack)) { /* In this case we must be in a
				     * sort block, which is a CXt_NULL
				     * not a CXt_SUB */
	    dounwind(0);
	    PL_stack_base[1] = *PL_stack_sp;
	    PL_stack_sp = PL_stack_base + 1;
	    return 0;
	}
	else
	    DIE(aTHX_ "Can't return outside a subroutine");
    }
    if (cxix < cxstack_ix)
	dounwind(cxix);

    if (CxMULTICALL(&cxstack[cxix])) {
	gimme = cxstack[cxix].blk_gimme;
	if (gimme == G_VOID)
	    PL_stack_sp = PL_stack_base;
	else if (gimme == G_SCALAR) {
	    PL_stack_base[1] = *PL_stack_sp;
	    PL_stack_sp = PL_stack_base + 1;
	}
	return 0;
    }

    POPBLOCK(cx,newpm);
    switch (CxTYPE(cx)) {
    case CXt_SUB:
	popsub2 = TRUE;
	lval = !!CvLVALUE(cx->blk_sub.cv);
	retop = cx->blk_sub.retop;
	cxstack_ix++; /* preserve cx entry on stack for use by POPSUB */
	break;
    case CXt_EVAL:
	if (!(PL_in_eval & EVAL_KEEPERR))
	    clear_errsv = TRUE;
	POPEVAL(cx);
	namesv = cx->blk_eval.old_namesv;
	retop = cx->blk_eval.retop;
	if (CxTRYBLOCK(cx))
	    break;
	if (optype == OP_REQUIRE &&
	    (MARK == SP || (gimme == G_SCALAR && !SvTRUE(*SP))) )
	{
	    /* Unassume the success we assumed earlier. */
	    (void)hv_delete(GvHVn(PL_incgv),
			    SvPVX_const(namesv),
                            SvUTF8(namesv) ? -(I32)SvCUR(namesv) : (I32)SvCUR(namesv),
			    G_DISCARD);
	    DIE(aTHX_ "%"SVf" did not return a true value", SVfARG(namesv));
	}
	break;
    case CXt_FORMAT:
	POPFORMAT(cx);
	retop = cx->blk_sub.retop;
	break;
    default:
	DIE(aTHX_ "panic: return, type=%u", (unsigned) CxTYPE(cx));
    }

    TAINT_NOT;
    if (lval) S_return_lvalues(aTHX_ MARK, SP, newsp, gimme, cx, newpm);
    else {
      if (gimme == G_SCALAR) {
	if (MARK < SP) {
	    if (popsub2) {
		if (cx->blk_sub.cv && CvDEPTH(cx->blk_sub.cv) > 1) {
		    if (SvTEMP(TOPs) && SvREFCNT(TOPs) == 1
			 && !SvMAGICAL(TOPs)) {
			*++newsp = SvREFCNT_inc(*SP);
			FREETMPS;
			sv_2mortal(*newsp);
		    }
		    else {
			sv = SvREFCNT_inc(*SP);	/* FREETMPS could clobber it */
			FREETMPS;
			*++newsp = sv_mortalcopy(sv);
			SvREFCNT_dec(sv);
		    }
		}
		else if (SvTEMP(*SP) && SvREFCNT(*SP) == 1
			  && !SvMAGICAL(*SP)) {
		    *++newsp = *SP;
		}
		else
		    *++newsp = sv_mortalcopy(*SP);
	    }
	    else
		*++newsp = sv_mortalcopy(*SP);
	}
	else
	    *++newsp = &PL_sv_undef;
      }
      else if (gimme == G_ARRAY) {
	while (++MARK <= SP) {
	    *++newsp = popsub2 && SvTEMP(*MARK) && SvREFCNT(*MARK) == 1
			       && !SvGMAGICAL(*MARK)
			? *MARK : sv_mortalcopy(*MARK);
	    TAINT_NOT;		/* Each item is independent */
	}
      }
      PL_stack_sp = newsp;
    }

    LEAVE;
    /* Stack values are safe: */
    if (popsub2) {
	cxstack_ix--;
	POPSUB(cx,sv);	/* release CV and @_ ... */
    }
    else
	sv = NULL;
    PL_curpm = newpm;	/* ... and pop $1 et al */

    LEAVESUB(sv);
    if (clear_errsv) {
	CLEAR_ERRSV();
    }
    return retop;
}

/* This duplicates parts of pp_leavesub, so that it can share code with
 * pp_return */
PP(pp_leavesublv)
{
    dVAR; dSP;
    SV **newsp;
    PMOP *newpm;
    I32 gimme;
    register PERL_CONTEXT *cx;
    SV *sv;

    if (CxMULTICALL(&cxstack[cxstack_ix]))
	return 0;

    POPBLOCK(cx,newpm);
    cxstack_ix++; /* temporarily protect top context */

    TAINT_NOT;

    S_return_lvalues(aTHX_ newsp, SP, newsp, gimme, cx, newpm);

    LEAVE;
    cxstack_ix--;
    POPSUB(cx,sv);	/* Stack values are safe: release CV and @_ ... */
    PL_curpm = newpm;	/* ... and pop $1 et al */

    LEAVESUB(sv);
    return cx->blk_sub.retop;
}

PP(pp_last)
{
    dVAR; dSP;
    I32 cxix;
    register PERL_CONTEXT *cx;
    I32 pop2 = 0;
    I32 gimme;
    I32 optype;
    OP *nextop = NULL;
    SV **newsp;
    PMOP *newpm;
    SV **mark;
    SV *sv = NULL;


    if (PL_op->op_flags & OPf_SPECIAL) {
	cxix = dopoptoloop(cxstack_ix);
	if (cxix < 0)
	    DIE(aTHX_ "Can't \"last\" outside a loop block");
    }
    else {
        cxix = dopoptolabel(cPVOP->op_pv, strlen(cPVOP->op_pv),
                           (cPVOP->op_private & OPpPV_IS_UTF8) ? SVf_UTF8 : 0);
	if (cxix < 0)
	    DIE(aTHX_ "Label not found for \"last %"SVf"\"",
                                        SVfARG(newSVpvn_flags(cPVOP->op_pv,
                                                    strlen(cPVOP->op_pv),
                                                    ((cPVOP->op_private & OPpPV_IS_UTF8)
                                                    ? SVf_UTF8 : 0) | SVs_TEMP)));
    }
    if (cxix < cxstack_ix)
	dounwind(cxix);

    POPBLOCK(cx,newpm);
    cxstack_ix++; /* temporarily protect top context */
    mark = newsp;
    switch (CxTYPE(cx)) {
    case CXt_LOOP_LAZYIV:
    case CXt_LOOP_LAZYSV:
    case CXt_LOOP_FOR:
    case CXt_LOOP_PLAIN:
	pop2 = CxTYPE(cx);
	newsp = PL_stack_base + cx->blk_loop.resetsp;
	nextop = cx->blk_loop.my_op->op_lastop->op_next;
	break;
    case CXt_SUB:
	pop2 = CXt_SUB;
	nextop = cx->blk_sub.retop;
	break;
    case CXt_EVAL:
	POPEVAL(cx);
	nextop = cx->blk_eval.retop;
	break;
    case CXt_FORMAT:
	POPFORMAT(cx);
	nextop = cx->blk_sub.retop;
	break;
    default:
	DIE(aTHX_ "panic: last, type=%u", (unsigned) CxTYPE(cx));
    }

    TAINT_NOT;
    SP = adjust_stack_on_leave(newsp, SP, MARK, gimme,
				pop2 == CXt_SUB ? SVs_TEMP : 0);
    PUTBACK;

    LEAVE;
    cxstack_ix--;
    /* Stack values are safe: */
    switch (pop2) {
    case CXt_LOOP_LAZYIV:
    case CXt_LOOP_PLAIN:
    case CXt_LOOP_LAZYSV:
    case CXt_LOOP_FOR:
	POPLOOP(cx);	/* release loop vars ... */
	LEAVE;
	break;
    case CXt_SUB:
	POPSUB(cx,sv);	/* release CV and @_ ... */
	break;
    }
    PL_curpm = newpm;	/* ... and pop $1 et al */

    LEAVESUB(sv);
    PERL_UNUSED_VAR(optype);
    PERL_UNUSED_VAR(gimme);
    return nextop;
}

PP(pp_next)
{
    dVAR;
    I32 cxix;
    register PERL_CONTEXT *cx;
    I32 inner;

    if (PL_op->op_flags & OPf_SPECIAL) {
	cxix = dopoptoloop(cxstack_ix);
	if (cxix < 0)
	    DIE(aTHX_ "Can't \"next\" outside a loop block");
    }
    else {
	cxix = dopoptolabel(cPVOP->op_pv, strlen(cPVOP->op_pv),
                           (cPVOP->op_private & OPpPV_IS_UTF8) ? SVf_UTF8 : 0);
 	if (cxix < 0)
	    DIE(aTHX_ "Label not found for \"next %"SVf"\"",
                                        SVfARG(newSVpvn_flags(cPVOP->op_pv, 
                                                    strlen(cPVOP->op_pv),
                                                    ((cPVOP->op_private & OPpPV_IS_UTF8)
                                                    ? SVf_UTF8 : 0) | SVs_TEMP)));
    }
    if (cxix < cxstack_ix)
	dounwind(cxix);

    /* clear off anything above the scope we're re-entering, but
     * save the rest until after a possible continue block */
    inner = PL_scopestack_ix;
    TOPBLOCK(cx);
    if (PL_scopestack_ix < inner)
	leave_scope(PL_scopestack[PL_scopestack_ix]);
    PL_curcop = cx->blk_oldcop;
    return (cx)->blk_loop.my_op->op_nextop;
}

PP(pp_redo)
{
    dVAR;
    I32 cxix;
    register PERL_CONTEXT *cx;
    I32 oldsave;
    OP* redo_op;

    if (PL_op->op_flags & OPf_SPECIAL) {
	cxix = dopoptoloop(cxstack_ix);
	if (cxix < 0)
	    DIE(aTHX_ "Can't \"redo\" outside a loop block");
    }
    else {
	cxix = dopoptolabel(cPVOP->op_pv, strlen(cPVOP->op_pv),
                           (cPVOP->op_private & OPpPV_IS_UTF8) ? SVf_UTF8 : 0);
 	if (cxix < 0)
	    DIE(aTHX_ "Label not found for \"redo %"SVf"\"",
                                        SVfARG(newSVpvn_flags(cPVOP->op_pv,
                                                    strlen(cPVOP->op_pv),
                                                    ((cPVOP->op_private & OPpPV_IS_UTF8)
                                                    ? SVf_UTF8 : 0) | SVs_TEMP)));
    }
    if (cxix < cxstack_ix)
	dounwind(cxix);

    redo_op = cxstack[cxix].blk_loop.my_op->op_redoop;
    if (redo_op->op_type == OP_ENTER) {
	/* pop one less context to avoid $x being freed in while (my $x..) */
	cxstack_ix++;
	assert(CxTYPE(&cxstack[cxstack_ix]) == CXt_BLOCK);
	redo_op = redo_op->op_next;
    }

    TOPBLOCK(cx);
    oldsave = PL_scopestack[PL_scopestack_ix - 1];
    LEAVE_SCOPE(oldsave);
    FREETMPS;
    PL_curcop = cx->blk_oldcop;
    return redo_op;
}

STATIC OP *
S_dofindlabel(pTHX_ OP *o, const char *label, STRLEN len, U32 flags, OP **opstack, OP **oplimit)
{
    dVAR;
    OP **ops = opstack;
    static const char too_deep[] = "Target of goto is too deeply nested";

    PERL_ARGS_ASSERT_DOFINDLABEL;

    if (ops >= oplimit)
	Perl_croak(aTHX_ too_deep);
    if (o->op_type == OP_LEAVE ||
	o->op_type == OP_SCOPE ||
	o->op_type == OP_LEAVELOOP ||
	o->op_type == OP_LEAVESUB ||
	o->op_type == OP_LEAVETRY)
    {
	*ops++ = cUNOPo->op_first;
	if (ops >= oplimit)
	    Perl_croak(aTHX_ too_deep);
    }
    *ops = 0;
    if (o->op_flags & OPf_KIDS) {
	OP *kid;
	/* First try all the kids at this level, since that's likeliest. */
	for (kid = cUNOPo->op_first; kid; kid = kid->op_sibling) {
	    if (kid->op_type == OP_NEXTSTATE || kid->op_type == OP_DBSTATE) {
                STRLEN kid_label_len;
                U32 kid_label_flags;
		const char *kid_label = CopLABEL_len_flags(kCOP,
                                                    &kid_label_len, &kid_label_flags);
		if (kid_label && (
                    ( (kid_label_flags & SVf_UTF8) != (flags & SVf_UTF8) ) ?
                        (flags & SVf_UTF8)
                            ? (bytes_cmp_utf8(
                                        (const U8*)kid_label, kid_label_len,
                                        (const U8*)label, len) == 0)
                            : (bytes_cmp_utf8(
                                        (const U8*)label, len,
                                        (const U8*)kid_label, kid_label_len) == 0)
                    : ( len == kid_label_len && ((kid_label == label)
                                    || memEQ(kid_label, label, len)))))
		    return kid;
	    }
	}
	for (kid = cUNOPo->op_first; kid; kid = kid->op_sibling) {
	    if (kid == PL_lastgotoprobe)
		continue;
	    if (kid->op_type == OP_NEXTSTATE || kid->op_type == OP_DBSTATE) {
	        if (ops == opstack)
		    *ops++ = kid;
		else if (ops[-1]->op_type == OP_NEXTSTATE ||
		         ops[-1]->op_type == OP_DBSTATE)
		    ops[-1] = kid;
		else
		    *ops++ = kid;
	    }
	    if ((o = dofindlabel(kid, label, len, flags, ops, oplimit)))
		return o;
	}
    }
    *ops = 0;
    return 0;
}

PP(pp_goto)
{
    dVAR; dSP;
    OP *retop = NULL;
    I32 ix;
    register PERL_CONTEXT *cx;
#define GOTO_DEPTH 64
    OP *enterops[GOTO_DEPTH];
    const char *label = NULL;
    STRLEN label_len = 0;
    U32 label_flags = 0;
    const bool do_dump = (PL_op->op_type == OP_DUMP);
    static const char must_have_label[] = "goto must have label";

    if (PL_op->op_flags & OPf_STACKED) {
	SV * const sv = POPs;

	/* This egregious kludge implements goto &subroutine */
	if (SvROK(sv) && SvTYPE(SvRV(sv)) == SVt_PVCV) {
	    I32 cxix;
	    register PERL_CONTEXT *cx;
	    CV *cv = MUTABLE_CV(SvRV(sv));
	    SV** mark;
	    I32 items = 0;
	    I32 oldsave;
	    bool reified = 0;

	retry:
	    if (!CvROOT(cv) && !CvXSUB(cv)) {
		const GV * const gv = CvGV(cv);
		if (gv) {
		    GV *autogv;
		    SV *tmpstr;
		    /* autoloaded stub? */
		    if (cv != GvCV(gv) && (cv = GvCV(gv)))
			goto retry;
		    autogv = gv_autoload_pvn(GvSTASH(gv), GvNAME(gv),
					  GvNAMELEN(gv),
                                          GvNAMEUTF8(gv) ? SVf_UTF8 : 0);
		    if (autogv && (cv = GvCV(autogv)))
			goto retry;
		    tmpstr = sv_newmortal();
		    gv_efullname3(tmpstr, gv, NULL);
		    DIE(aTHX_ "Goto undefined subroutine &%"SVf"", SVfARG(tmpstr));
		}
		DIE(aTHX_ "Goto undefined subroutine");
	    }

	    /* First do some returnish stuff. */
	    SvREFCNT_inc_simple_void(cv); /* avoid premature free during unwind */
	    FREETMPS;
	    cxix = dopoptosub(cxstack_ix);
	    if (cxix < 0)
		DIE(aTHX_ "Can't goto subroutine outside a subroutine");
	    if (cxix < cxstack_ix)
		dounwind(cxix);
	    TOPBLOCK(cx);
	    SPAGAIN;
	    /* ban goto in eval: see <20050521150056.GC20213@iabyn.com> */
	    if (CxTYPE(cx) == CXt_EVAL) {
		if (CxREALEVAL(cx))
		/* diag_listed_as: Can't goto subroutine from an eval-%s */
		    DIE(aTHX_ "Can't goto subroutine from an eval-string");
		else
		/* diag_listed_as: Can't goto subroutine from an eval-%s */
		    DIE(aTHX_ "Can't goto subroutine from an eval-block");
	    }
	    else if (CxMULTICALL(cx))
		DIE(aTHX_ "Can't goto subroutine from a sort sub (or similar callback)");
	    if (CxTYPE(cx) == CXt_SUB && CxHASARGS(cx)) {
		/* put @_ back onto stack */
		AV* av = cx->blk_sub.argarray;

		items = AvFILLp(av) + 1;
		EXTEND(SP, items+1); /* @_ could have been extended. */
		Copy(AvARRAY(av), SP + 1, items, SV*);
		SvREFCNT_dec(GvAV(PL_defgv));
		GvAV(PL_defgv) = cx->blk_sub.savearray;
		CLEAR_ARGARRAY(av);
		/* abandon @_ if it got reified */
		if (AvREAL(av)) {
		    reified = 1;
		    SvREFCNT_dec(av);
		    av = newAV();
		    av_extend(av, items-1);
		    AvREIFY_only(av);
		    PAD_SVl(0) = MUTABLE_SV(cx->blk_sub.argarray = av);
		}
	    }
	    else if (CvISXSUB(cv)) {	/* put GvAV(defgv) back onto stack */
		AV* const av = GvAV(PL_defgv);
		items = AvFILLp(av) + 1;
		EXTEND(SP, items+1); /* @_ could have been extended. */
		Copy(AvARRAY(av), SP + 1, items, SV*);
	    }
	    mark = SP;
	    SP += items;
	    if (CxTYPE(cx) == CXt_SUB &&
		!(CvDEPTH(cx->blk_sub.cv) = cx->blk_sub.olddepth))
		SvREFCNT_dec(cx->blk_sub.cv);
	    oldsave = PL_scopestack[PL_scopestack_ix - 1];
	    LEAVE_SCOPE(oldsave);

	    /* A destructor called during LEAVE_SCOPE could have undefined
	     * our precious cv.  See bug #99850. */
	    if (!CvROOT(cv) && !CvXSUB(cv)) {
		const GV * const gv = CvGV(cv);
		if (gv) {
		    SV * const tmpstr = sv_newmortal();
		    gv_efullname3(tmpstr, gv, NULL);
		    DIE(aTHX_ "Goto undefined subroutine &%"SVf"",
			       SVfARG(tmpstr));
		}
		DIE(aTHX_ "Goto undefined subroutine");
	    }

	    /* Now do some callish stuff. */
	    SAVETMPS;
	    SAVEFREESV(cv); /* later, undo the 'avoid premature free' hack */
	    if (CvISXSUB(cv)) {
		OP* const retop = cx->blk_sub.retop;
		SV **newsp PERL_UNUSED_DECL;
		I32 gimme PERL_UNUSED_DECL;
		if (reified) {
		    I32 index;
		    for (index=0; index<items; index++)
			sv_2mortal(SP[-index]);
		}

		/* XS subs don't have a CxSUB, so pop it */
		POPBLOCK(cx, PL_curpm);
		/* Push a mark for the start of arglist */
		PUSHMARK(mark);
		PUTBACK;
		(void)(*CvXSUB(cv))(aTHX_ cv);
		LEAVE;
		return retop;
	    }
	    else {
		AV* const padlist = CvPADLIST(cv);
		if (CxTYPE(cx) == CXt_EVAL) {
		    PL_in_eval = CxOLD_IN_EVAL(cx);
		    PL_eval_root = cx->blk_eval.old_eval_root;
		    cx->cx_type = CXt_SUB;
		}
		cx->blk_sub.cv = cv;
		cx->blk_sub.olddepth = CvDEPTH(cv);

		CvDEPTH(cv)++;
		if (CvDEPTH(cv) < 2)
		    SvREFCNT_inc_simple_void_NN(cv);
		else {
		    if (CvDEPTH(cv) == PERL_SUB_DEPTH_WARN && ckWARN(WARN_RECURSION))
			sub_crush_depth(cv);
		    pad_push(padlist, CvDEPTH(cv));
		}
		PL_curcop = cx->blk_oldcop;
		SAVECOMPPAD();
		PAD_SET_CUR_NOSAVE(padlist, CvDEPTH(cv));
		if (CxHASARGS(cx))
		{
		    AV *const av = MUTABLE_AV(PAD_SVl(0));

		    cx->blk_sub.savearray = GvAV(PL_defgv);
		    GvAV(PL_defgv) = MUTABLE_AV(SvREFCNT_inc_simple(av));
		    CX_CURPAD_SAVE(cx->blk_sub);
		    cx->blk_sub.argarray = av;

		    if (items >= AvMAX(av) + 1) {
			SV **ary = AvALLOC(av);
			if (AvARRAY(av) != ary) {
			    AvMAX(av) += AvARRAY(av) - AvALLOC(av);
			    AvARRAY(av) = ary;
			}
			if (items >= AvMAX(av) + 1) {
			    AvMAX(av) = items - 1;
			    Renew(ary,items+1,SV*);
			    AvALLOC(av) = ary;
			    AvARRAY(av) = ary;
			}
		    }
		    ++mark;
		    Copy(mark,AvARRAY(av),items,SV*);
		    AvFILLp(av) = items - 1;
		    assert(!AvREAL(av));
		    if (reified) {
			/* transfer 'ownership' of refcnts to new @_ */
			AvREAL_on(av);
			AvREIFY_off(av);
		    }
		    while (items--) {
			if (*mark)
			    SvTEMP_off(*mark);
			mark++;
		    }
		}
		if (PERLDB_SUB) {	/* Checking curstash breaks DProf. */
		    Perl_get_db_sub(aTHX_ NULL, cv);
		    if (PERLDB_GOTO) {
			CV * const gotocv = get_cvs("DB::goto", 0);
			if (gotocv) {
			    PUSHMARK( PL_stack_sp );
			    call_sv(MUTABLE_SV(gotocv), G_SCALAR | G_NODEBUG);
			    PL_stack_sp--;
			}
		    }
		}
		RETURNOP(CvSTART(cv));
	    }
	}
	else {
	    label       = SvPV_const(sv, label_len);
            label_flags = SvUTF8(sv);
	    if (!(do_dump || *label))
		DIE(aTHX_ must_have_label);
	}
    }
    else if (PL_op->op_flags & OPf_SPECIAL) {
	if (! do_dump)
	    DIE(aTHX_ must_have_label);
    }
    else {
 	label       = cPVOP->op_pv;
        label_flags = (cPVOP->op_private & OPpPV_IS_UTF8) ? SVf_UTF8 : 0;
        label_len   = strlen(label);
    }

    PERL_ASYNC_CHECK();

    if (label && *label) {
	OP *gotoprobe = NULL;
	bool leaving_eval = FALSE;
	bool in_block = FALSE;
	PERL_CONTEXT *last_eval_cx = NULL;

	/* find label */

	PL_lastgotoprobe = NULL;
	*enterops = 0;
	for (ix = cxstack_ix; ix >= 0; ix--) {
	    cx = &cxstack[ix];
	    switch (CxTYPE(cx)) {
	    case CXt_EVAL:
		leaving_eval = TRUE;
                if (!CxTRYBLOCK(cx)) {
		    gotoprobe = (last_eval_cx ?
				last_eval_cx->blk_eval.old_eval_root :
				PL_eval_root);
		    last_eval_cx = cx;
		    break;
                }
                /* else fall through */
	    case CXt_LOOP_LAZYIV:
	    case CXt_LOOP_LAZYSV:
	    case CXt_LOOP_FOR:
	    case CXt_LOOP_PLAIN:
	    case CXt_GIVEN:
	    case CXt_WHEN:
		gotoprobe = cx->blk_oldcop->op_sibling;
		break;
	    case CXt_SUBST:
		continue;
	    case CXt_BLOCK:
		if (ix) {
		    gotoprobe = cx->blk_oldcop->op_sibling;
		    in_block = TRUE;
		} else
		    gotoprobe = PL_main_root;
		break;
	    case CXt_SUB:
		if (CvDEPTH(cx->blk_sub.cv) && !CxMULTICALL(cx)) {
		    gotoprobe = CvROOT(cx->blk_sub.cv);
		    break;
		}
		/* FALL THROUGH */
	    case CXt_FORMAT:
	    case CXt_NULL:
		DIE(aTHX_ "Can't \"goto\" out of a pseudo block");
	    default:
		if (ix)
		    DIE(aTHX_ "panic: goto, type=%u, ix=%ld",
			CxTYPE(cx), (long) ix);
		gotoprobe = PL_main_root;
		break;
	    }
	    if (gotoprobe) {
		retop = dofindlabel(gotoprobe, label, label_len, label_flags,
				    enterops, enterops + GOTO_DEPTH);
		if (retop)
		    break;
		if (gotoprobe->op_sibling &&
			gotoprobe->op_sibling->op_type == OP_UNSTACK &&
			gotoprobe->op_sibling->op_sibling) {
		    retop = dofindlabel(gotoprobe->op_sibling->op_sibling,
					label, label_len, label_flags, enterops,
					enterops + GOTO_DEPTH);
		    if (retop)
			break;
		}
	    }
	    PL_lastgotoprobe = gotoprobe;
	}
	if (!retop)
	    DIE(aTHX_ "Can't find label %"SVf,
                            SVfARG(newSVpvn_flags(label, label_len,
                                        SVs_TEMP | label_flags)));

	/* if we're leaving an eval, check before we pop any frames
           that we're not going to punt, otherwise the error
	   won't be caught */

	if (leaving_eval && *enterops && enterops[1]) {
	    I32 i;
            for (i = 1; enterops[i]; i++)
                if (enterops[i]->op_type == OP_ENTERITER)
                    DIE(aTHX_ "Can't \"goto\" into the middle of a foreach loop");
	}

	if (*enterops && enterops[1]) {
	    I32 i = enterops[1]->op_type == OP_ENTER && in_block ? 2 : 1;
	    if (enterops[i])
		deprecate("\"goto\" to jump into a construct");
	}

	/* pop unwanted frames */

	if (ix < cxstack_ix) {
	    I32 oldsave;

	    if (ix < 0)
		ix = 0;
	    dounwind(ix);
	    TOPBLOCK(cx);
	    oldsave = PL_scopestack[PL_scopestack_ix];
	    LEAVE_SCOPE(oldsave);
	}

	/* push wanted frames */

	if (*enterops && enterops[1]) {
	    OP * const oldop = PL_op;
	    ix = enterops[1]->op_type == OP_ENTER && in_block ? 2 : 1;
	    for (; enterops[ix]; ix++) {
		PL_op = enterops[ix];
		/* Eventually we may want to stack the needed arguments
		 * for each op.  For now, we punt on the hard ones. */
		if (PL_op->op_type == OP_ENTERITER)
		    DIE(aTHX_ "Can't \"goto\" into the middle of a foreach loop");
		PL_op->op_ppaddr(aTHX);
	    }
	    PL_op = oldop;
	}
    }

    if (do_dump) {
#ifdef VMS
	if (!retop) retop = PL_main_start;
#endif
	PL_restartop = retop;
	PL_do_undump = TRUE;

	my_unexec();

	PL_restartop = 0;		/* hmm, must be GNU unexec().. */
	PL_do_undump = FALSE;
    }

    RETURNOP(retop);
}

PP(pp_exit)
{
    dVAR;
    dSP;
    I32 anum;

    if (MAXARG < 1)
	anum = 0;
    else if (!TOPs) {
	anum = 0; (void)POPs;
    }
    else {
	anum = SvIVx(POPs);
#ifdef VMS
        if (anum == 1 && (PL_op->op_private & OPpEXIT_VMSISH))
	    anum = 0;
        VMSISH_HUSHED  = VMSISH_HUSHED || (PL_op->op_private & OPpHUSH_VMSISH);
#endif
    }
    PL_exit_flags |= PERL_EXIT_EXPECTED;
#ifdef PERL_MAD
    /* KLUDGE: disable exit 0 in BEGIN blocks when we're just compiling */
    if (anum || !(PL_minus_c && PL_madskills))
	my_exit(anum);
#else
    my_exit(anum);
#endif
    PUSHs(&PL_sv_undef);
    RETURN;
}

/* Eval. */

STATIC void
S_save_lines(pTHX_ AV *array, SV *sv)
{
    const char *s = SvPVX_const(sv);
    const char * const send = SvPVX_const(sv) + SvCUR(sv);
    I32 line = 1;

    PERL_ARGS_ASSERT_SAVE_LINES;

    while (s && s < send) {
	const char *t;
	SV * const tmpstr = newSV_type(SVt_PVMG);

	t = (const char *)memchr(s, '\n', send - s);
	if (t)
	    t++;
	else
	    t = send;

	sv_setpvn(tmpstr, s, t - s);
	av_store(array, line++, tmpstr);
	s = t;
    }
}

/*
=for apidoc docatch

Check for the cases 0 or 3 of cur_env.je_ret, only used inside an eval context.

0 is used as continue inside eval,

3 is used for a die caught by an inner eval - continue inner loop

See cop.h: je_mustcatch, when set at any runlevel to TRUE, means eval ops must
establish a local jmpenv to handle exception traps.

=cut
*/
STATIC OP *
S_docatch(pTHX_ OP *o)
{
    dVAR;
    int ret;
    OP * const oldop = PL_op;
    dJMPENV;

#ifdef DEBUGGING
    assert(CATCH_GET == TRUE);
#endif
    PL_op = o;

    JMPENV_PUSH(ret);
    switch (ret) {
    case 0:
	assert(cxstack_ix >= 0);
	assert(CxTYPE(&cxstack[cxstack_ix]) == CXt_EVAL);
	cxstack[cxstack_ix].blk_eval.cur_top_env = PL_top_env;
 redo_body:
	CALLRUNOPS(aTHX);
	break;
    case 3:
	/* die caught by an inner eval - continue inner loop */
	if (PL_restartop && PL_restartjmpenv == PL_top_env) {
	    PL_restartjmpenv = NULL;
	    PL_op = PL_restartop;
	    PL_restartop = 0;
	    goto redo_body;
	}
	/* FALL THROUGH */
    default:
	JMPENV_POP;
	PL_op = oldop;
	JMPENV_JUMP(ret);
	/* NOTREACHED */
    }
    JMPENV_POP;
    PL_op = oldop;
    return NULL;
}

/* James Bond: Do you expect me to talk?
   Auric Goldfinger: No, Mr. Bond. I expect you to die.

   This code is an ugly hack, doesn't work with lexicals in subroutines that are
   called more than once, and is only used by regcomp.c, for (?{}) blocks.

   Currently it is not used outside the core code. Best if it stays that way.

   Hence it's now deprecated, and will be removed.
*/
OP *
Perl_sv_compile_2op(pTHX_ SV *sv, OP** startop, const char *code, PAD** padp)
/* sv Text to convert to OP tree. */
/* startop op_free() this to undo. */
/* code Short string id of the caller. */
{
    PERL_ARGS_ASSERT_SV_COMPILE_2OP;
    return Perl_sv_compile_2op_is_broken(aTHX_ sv, startop, code, padp);
}

/* Don't use this. It will go away without warning once the regexp engine is
   refactored not to use it.  */
OP *
Perl_sv_compile_2op_is_broken(pTHX_ SV *sv, OP **startop, const char *code,
			      PAD **padp)
{
    dVAR; dSP;				/* Make POPBLOCK work. */
    PERL_CONTEXT *cx;
    SV **newsp;
    I32 gimme = G_VOID;
    I32 optype;
    OP dummy;
    char tbuf[TYPE_DIGITS(long) + 12 + 10];
    char *tmpbuf = tbuf;
    char *safestr;
    int runtime;
    CV* runcv = NULL;	/* initialise to avoid compiler warnings */
    STRLEN len;
    bool need_catch;

    PERL_ARGS_ASSERT_SV_COMPILE_2OP_IS_BROKEN;

    ENTER_with_name("eval");
    lex_start(sv, NULL, LEX_START_SAME_FILTER);
    SAVETMPS;
    /* switch to eval mode */

    if (IN_PERL_COMPILETIME) {
	SAVECOPSTASH_FREE(&PL_compiling);
	CopSTASH_set(&PL_compiling, PL_curstash);
    }
    if (PERLDB_NAMEEVAL && CopLINE(PL_curcop)) {
	SV * const sv = sv_newmortal();
	Perl_sv_setpvf(aTHX_ sv, "_<(%.10seval %lu)[%s:%"IVdf"]",
		       code, (unsigned long)++PL_evalseq,
		       CopFILE(PL_curcop), (IV)CopLINE(PL_curcop));
	tmpbuf = SvPVX(sv);
	len = SvCUR(sv);
    }
    else
	len = my_snprintf(tmpbuf, sizeof(tbuf), "_<(%.10s_eval %lu)", code,
			  (unsigned long)++PL_evalseq);
    SAVECOPFILE_FREE(&PL_compiling);
    CopFILE_set(&PL_compiling, tmpbuf+2);
    SAVECOPLINE(&PL_compiling);
    CopLINE_set(&PL_compiling, 1);
    /* XXX For C<eval "...">s within BEGIN {} blocks, this ends up
       deleting the eval's FILEGV from the stash before gv_check() runs
       (i.e. before run-time proper). To work around the coredump that
       ensues, we always turn GvMULTI_on for any globals that were
       introduced within evals. See force_ident(). GSAR 96-10-12 */
    safestr = savepvn(tmpbuf, len);
    SAVEDELETE(PL_defstash, safestr, len);
    SAVEHINTS();
#ifdef OP_IN_REGISTER
    PL_opsave = op;
#else
    SAVEVPTR(PL_op);
#endif

    /* we get here either during compilation, or via pp_regcomp at runtime */
    runtime = IN_PERL_RUNTIME;
    if (runtime)
    {
	runcv = find_runcv(NULL);

	/* At run time, we have to fetch the hints from PL_curcop. */
	PL_hints = PL_curcop->cop_hints;
	if (PL_hints & HINT_LOCALIZE_HH) {
	    /* SAVEHINTS created a new HV in PL_hintgv, which we
	       need to GC */
	    SvREFCNT_dec(GvHV(PL_hintgv));
	    GvHV(PL_hintgv) =
	     refcounted_he_chain_2hv(PL_curcop->cop_hints_hash, 0);
	    hv_magic(GvHV(PL_hintgv), NULL, PERL_MAGIC_hints);
	}
	SAVECOMPILEWARNINGS();
	PL_compiling.cop_warnings = DUP_WARNINGS(PL_curcop->cop_warnings);
	cophh_free(CopHINTHASH_get(&PL_compiling));
	/* XXX Does this need to avoid copying a label? */
	PL_compiling.cop_hints_hash
	 = cophh_copy(PL_curcop->cop_hints_hash);
    }

    PL_op = &dummy;
    PL_op->op_type = OP_ENTEREVAL;
    PL_op->op_flags = 0;			/* Avoid uninit warning. */
    PUSHBLOCK(cx, CXt_EVAL|(IN_PERL_COMPILETIME ? 0 : CXp_REAL), SP);
    PUSHEVAL(cx, 0);
    need_catch = CATCH_GET;
    CATCH_SET(TRUE);

    if (runtime)
	(void) doeval(G_SCALAR, startop, runcv, PL_curcop->cop_seq, NULL);
    else
	(void) doeval(G_SCALAR, startop, PL_compcv, PL_cop_seqmax, NULL);
    CATCH_SET(need_catch);
    POPBLOCK(cx,PL_curpm);
    POPEVAL(cx);

    (*startop)->op_type = OP_NULL;
    (*startop)->op_ppaddr = PL_ppaddr[OP_NULL];
    /* XXX DAPM do this properly one year */
    *padp = MUTABLE_AV(SvREFCNT_inc_simple(PL_comppad));
    LEAVE_with_name("eval");
    if (IN_PERL_COMPILETIME)
	CopHINTS_set(&PL_compiling, PL_hints);
#ifdef OP_IN_REGISTER
    op = PL_opsave;
#endif
    PERL_UNUSED_VAR(newsp);
    PERL_UNUSED_VAR(optype);

    return PL_eval_start;
}


/*
=for apidoc find_runcv

Locate the CV corresponding to the currently executing sub or eval.
If db_seqp is non_null, skip CVs that are in the DB package and populate
*db_seqp with the cop sequence number at the point that the DB:: code was
entered. (allows debuggers to eval in the scope of the breakpoint rather
than in the scope of the debugger itself).

=cut
*/

CV*
Perl_find_runcv(pTHX_ U32 *db_seqp)
{
    dVAR;
    PERL_SI	 *si;

    if (db_seqp)
	*db_seqp = PL_curcop->cop_seq;
    for (si = PL_curstackinfo; si; si = si->si_prev) {
        I32 ix;
	for (ix = si->si_cxix; ix >= 0; ix--) {
	    const PERL_CONTEXT *cx = &(si->si_cxstack[ix]);
	    if (CxTYPE(cx) == CXt_SUB || CxTYPE(cx) == CXt_FORMAT) {
		CV * const cv = cx->blk_sub.cv;
		/* skip DB:: code */
		if (db_seqp && PL_debstash && CvSTASH(cv) == PL_debstash) {
		    *db_seqp = cx->blk_oldcop->cop_seq;
		    continue;
		}
		return cv;
	    }
	    else if (CxTYPE(cx) == CXt_EVAL && !CxTRYBLOCK(cx))
		return cx->blk_eval.cv;
	}
    }
    return PL_main_cv;
}


/* Run yyparse() in a setjmp wrapper. Returns:
 *   0: yyparse() successful
 *   1: yyparse() failed
 *   3: yyparse() died
 */
STATIC int
S_try_yyparse(pTHX_ int gramtype)
{
    int ret;
    dJMPENV;

    assert(CxTYPE(&cxstack[cxstack_ix]) == CXt_EVAL);
    JMPENV_PUSH(ret);
    switch (ret) {
    case 0:
	ret = yyparse(gramtype) ? 1 : 0;
	break;
    case 3:
	break;
    default:
	JMPENV_POP;
	JMPENV_JUMP(ret);
	/* NOTREACHED */
    }
    JMPENV_POP;
    return ret;
}


/* Compile a require/do, an eval '', or a /(?{...})/.
 * In the last case, startop is non-null, and contains the address of
 * a pointer that should be set to the just-compiled code.
 * outside is the lexically enclosing CV (if any) that invoked us.
 * Returns a bool indicating whether the compile was successful; if so,
 * PL_eval_start contains the first op of the compiled ocde; otherwise,
 * pushes undef (also croaks if startop != NULL).
 */

/* This function is called from three places, sv_compile_2op, pp_require
 * and pp_entereval.  These can be distinguished as follows:
 *    sv_compile_2op - startop is non-null
 *    pp_require     - startop is null; saveop is not entereval
 *    pp_entereval   - startop is null; saveop is entereval
 */

STATIC bool
S_doeval(pTHX_ int gimme, OP** startop, CV* outside, U32 seq, HV *hh)
{
    dVAR; dSP;
    OP * const saveop = PL_op;
    COP * const oldcurcop = PL_curcop;
    bool in_require = (saveop && saveop->op_type == OP_REQUIRE);
    int yystatus;
    CV *evalcv;

    PL_in_eval = (in_require
		  ? (EVAL_INREQUIRE | (PL_in_eval & EVAL_INEVAL))
		  : EVAL_INEVAL);

    PUSHMARK(SP);

    evalcv = MUTABLE_CV(newSV_type(SVt_PVCV));
    CvEVAL_on(evalcv);
    assert(CxTYPE(&cxstack[cxstack_ix]) == CXt_EVAL);
    cxstack[cxstack_ix].blk_eval.cv = evalcv;
    cxstack[cxstack_ix].blk_gimme = gimme;

    CvOUTSIDE_SEQ(evalcv) = seq;
    CvOUTSIDE(evalcv) = MUTABLE_CV(SvREFCNT_inc_simple(outside));

    /* set up a scratch pad */

    CvPADLIST(evalcv) = pad_new(padnew_SAVE);
    PL_op = NULL; /* avoid PL_op and PL_curpad referring to different CVs */


    if (!PL_madskills)
	SAVEMORTALIZESV(evalcv);	/* must remain until end of current statement */

    /* make sure we compile in the right package */

    if (CopSTASH_ne(PL_curcop, PL_curstash)) {
	SAVEGENERICSV(PL_curstash);
	PL_curstash = (HV *)SvREFCNT_inc_simple(CopSTASH(PL_curcop));
    }
    /* XXX:ajgo do we really need to alloc an AV for begin/checkunit */
    SAVESPTR(PL_beginav);
    PL_beginav = newAV();
    SAVEFREESV(PL_beginav);
    SAVESPTR(PL_unitcheckav);
    PL_unitcheckav = newAV();
    SAVEFREESV(PL_unitcheckav);

#ifdef PERL_MAD
    SAVEBOOL(PL_madskills);
    PL_madskills = 0;
#endif

    if (!startop) ENTER_with_name("evalcomp");
    SAVESPTR(PL_compcv);
    PL_compcv = evalcv;

    /* try to compile it */

    PL_eval_root = NULL;
    PL_curcop = &PL_compiling;
    if (saveop && (saveop->op_type != OP_REQUIRE) && (saveop->op_flags & OPf_SPECIAL))
	PL_in_eval |= EVAL_KEEPERR;
    else
	CLEAR_ERRSV();

    if (!startop) {
	bool clear_hints = saveop->op_type != OP_ENTEREVAL;
	SAVEHINTS();
	if (clear_hints) {
	    PL_hints = 0;
	    hv_clear(GvHV(PL_hintgv));
	}
	else {
	    PL_hints = saveop->op_private & OPpEVAL_COPHH
			 ? oldcurcop->cop_hints : saveop->op_targ;
	    if (hh) {
		/* SAVEHINTS created a new HV in PL_hintgv, which we need to GC */
		SvREFCNT_dec(GvHV(PL_hintgv));
		GvHV(PL_hintgv) = hh;
	    }
	}
	SAVECOMPILEWARNINGS();
	if (clear_hints) {
	    if (PL_dowarn & G_WARN_ALL_ON)
	        PL_compiling.cop_warnings = pWARN_ALL ;
	    else if (PL_dowarn & G_WARN_ALL_OFF)
	        PL_compiling.cop_warnings = pWARN_NONE ;
	    else
	        PL_compiling.cop_warnings = pWARN_STD ;
	}
	else {
	    PL_compiling.cop_warnings =
		DUP_WARNINGS(oldcurcop->cop_warnings);
	    cophh_free(CopHINTHASH_get(&PL_compiling));
	    if (Perl_cop_fetch_label(aTHX_ oldcurcop, NULL, NULL)) {
		/* The label, if present, is the first entry on the chain. So rather
		   than writing a blank label in front of it (which involves an
		   allocation), just use the next entry in the chain.  */
		PL_compiling.cop_hints_hash
		    = cophh_copy(oldcurcop->cop_hints_hash->refcounted_he_next);
		/* Check the assumption that this removed the label.  */
		assert(Perl_cop_fetch_label(aTHX_ &PL_compiling, NULL, NULL) == NULL);
	    }
	    else
		PL_compiling.cop_hints_hash = cophh_copy(oldcurcop->cop_hints_hash);
	}
    }

    CALL_BLOCK_HOOKS(bhk_eval, saveop);

    /* note that yyparse() may raise an exception, e.g. C<BEGIN{die}>,
     * so honour CATCH_GET and trap it here if necessary */

    yystatus = (!in_require && CATCH_GET) ? S_try_yyparse(aTHX_ GRAMPROG) : yyparse(GRAMPROG);

    if (yystatus || PL_parser->error_count || !PL_eval_root) {
	SV **newsp;			/* Used by POPBLOCK. */
	PERL_CONTEXT *cx;
	I32 optype;			/* Used by POPEVAL. */
	SV *namesv;

	cx = NULL;
	namesv = NULL;
	PERL_UNUSED_VAR(newsp);
	PERL_UNUSED_VAR(optype);

	/* note that if yystatus == 3, then the EVAL CX block has already
	 * been popped, and various vars restored */
	PL_op = saveop;
	if (yystatus != 3) {
	    if (PL_eval_root) {
		op_free(PL_eval_root);
		PL_eval_root = NULL;
	    }
	    SP = PL_stack_base + POPMARK;	/* pop original mark */
	    if (!startop) {
		POPBLOCK(cx,PL_curpm);
		POPEVAL(cx);
		namesv = cx->blk_eval.old_namesv;
	    }
	    /* POPBLOCK renders LEAVE_with_name("evalcomp") unnecessary. */
	    LEAVE_with_name("eval"); /* pp_entereval knows about this LEAVE.  */
	}

	if (in_require) {
	    if (!cx) {
		/* If cx is still NULL, it means that we didn't go in the
		 * POPEVAL branch. */
		cx = &cxstack[cxstack_ix];
		assert(CxTYPE(cx) == CXt_EVAL);
		namesv = cx->blk_eval.old_namesv;
	    }
	    (void)hv_store(GvHVn(PL_incgv),
			   SvPVX_const(namesv),
                           SvUTF8(namesv) ? -(I32)SvCUR(namesv) : (I32)SvCUR(namesv),
			   &PL_sv_undef, 0);
	    Perl_croak(aTHX_ "%"SVf"Compilation failed in require",
		       SVfARG(ERRSV
                                ? ERRSV
                                : newSVpvs_flags("Unknown error\n", SVs_TEMP)));
	}
	else if (startop) {
	    if (yystatus != 3) {
		POPBLOCK(cx,PL_curpm);
		POPEVAL(cx);
	    }
	    Perl_croak(aTHX_ "%"SVf"Compilation failed in regexp",
		       SVfARG(ERRSV
                                ? ERRSV
                                : newSVpvs_flags("Unknown error\n", SVs_TEMP)));
	}
	else {
	    if (!*(SvPVx_nolen_const(ERRSV))) {
	        sv_setpvs(ERRSV, "Compilation error");
	    }
	}
	if (gimme != G_ARRAY) PUSHs(&PL_sv_undef);
	PUTBACK;
	return FALSE;
    }
    else if (!startop) LEAVE_with_name("evalcomp");
    CopLINE_set(&PL_compiling, 0);
    if (startop) {
	*startop = PL_eval_root;
    } else
	SAVEFREEOP(PL_eval_root);

    DEBUG_x(dump_eval());

    /* Register with debugger: */
    if (PERLDB_INTER && saveop && saveop->op_type == OP_REQUIRE) {
	CV * const cv = get_cvs("DB::postponed", 0);
	if (cv) {
	    dSP;
	    PUSHMARK(SP);
	    XPUSHs(MUTABLE_SV(CopFILEGV(&PL_compiling)));
	    PUTBACK;
	    call_sv(MUTABLE_SV(cv), G_DISCARD);
	}
    }

    if (PL_unitcheckav) {
	OP *es = PL_eval_start;
	call_list(PL_scopestack_ix, PL_unitcheckav);
	PL_eval_start = es;
    }

    /* compiled okay, so do it */

    CvDEPTH(evalcv) = 1;
    SP = PL_stack_base + POPMARK;		/* pop original mark */
    PL_op = saveop;			/* The caller may need it. */
    PL_parser->lex_state = LEX_NOTPARSING;	/* $^S needs this. */

    PUTBACK;
    return TRUE;
}

STATIC PerlIO *
S_check_type_and_open(pTHX_ SV *name)
{
    Stat_t st;
    const char *p = SvPV_nolen_const(name);
    const int st_rc = PerlLIO_stat(p, &st);

    PERL_ARGS_ASSERT_CHECK_TYPE_AND_OPEN;

    if (st_rc < 0 || S_ISDIR(st.st_mode) || S_ISBLK(st.st_mode)) {
	return NULL;
    }

#if !defined(PERLIO_IS_STDIO) && !defined(USE_SFIO)
    return PerlIO_openn(aTHX_ ":", PERL_SCRIPT_MODE, -1, 0, 0, NULL, 1, &name);
#else
    return PerlIO_open(p, PERL_SCRIPT_MODE);
#endif
}

#ifndef PERL_DISABLE_PMC
STATIC PerlIO *
S_doopen_pm(pTHX_ SV *name)
{
    STRLEN namelen;
    const char *p = SvPV_const(name, namelen);

    PERL_ARGS_ASSERT_DOOPEN_PM;

    if (namelen > 3 && memEQs(p + namelen - 3, 3, ".pm")) {
	SV *const pmcsv = sv_newmortal();
	Stat_t pmcstat;

	SvSetSV_nosteal(pmcsv,name);
	sv_catpvn(pmcsv, "c", 1);

	if (PerlLIO_stat(SvPV_nolen_const(pmcsv), &pmcstat) >= 0)
	    return check_type_and_open(pmcsv);
    }
    return check_type_and_open(name);
}
#else
#  define doopen_pm(name) check_type_and_open(name)
#endif /* !PERL_DISABLE_PMC */

PP(pp_require)
{
    dVAR; dSP;
    register PERL_CONTEXT *cx;
    SV *sv;
    const char *name;
    STRLEN len;
    char * unixname;
    STRLEN unixlen;
#ifdef VMS
    int vms_unixname = 0;
#endif
    const char *tryname = NULL;
    SV *namesv = NULL;
    const I32 gimme = GIMME_V;
    int filter_has_file = 0;
    PerlIO *tryrsfp = NULL;
    SV *filter_cache = NULL;
    SV *filter_state = NULL;
    SV *filter_sub = NULL;
    SV *hook_sv = NULL;
    SV *encoding;
    OP *op;

    sv = POPs;
    if ( (SvNIOKp(sv) || SvVOK(sv)) && PL_op->op_type != OP_DOFILE) {
	sv = sv_2mortal(new_version(sv));
	if (!sv_derived_from(PL_patchlevel, "version"))
	    upg_version(PL_patchlevel, TRUE);
	if (cUNOP->op_first->op_type == OP_CONST && cUNOP->op_first->op_private & OPpCONST_NOVER) {
	    if ( vcmp(sv,PL_patchlevel) <= 0 )
		DIE(aTHX_ "Perls since %"SVf" too modern--this is %"SVf", stopped",
		    SVfARG(sv_2mortal(vnormal(sv))),
		    SVfARG(sv_2mortal(vnormal(PL_patchlevel)))
		);
	}
	else {
	    if ( vcmp(sv,PL_patchlevel) > 0 ) {
		I32 first = 0;
		AV *lav;
		SV * const req = SvRV(sv);
		SV * const pv = *hv_fetchs(MUTABLE_HV(req), "original", FALSE);

		/* get the left hand term */
		lav = MUTABLE_AV(SvRV(*hv_fetchs(MUTABLE_HV(req), "version", FALSE)));

		first  = SvIV(*av_fetch(lav,0,0));
		if (   first > (int)PERL_REVISION    /* probably 'use 6.0' */
		    || hv_exists(MUTABLE_HV(req), "qv", 2 ) /* qv style */
		    || av_len(lav) > 1               /* FP with > 3 digits */
		    || strstr(SvPVX(pv),".0")        /* FP with leading 0 */
		   ) {
		    DIE(aTHX_ "Perl %"SVf" required--this is only "
		    	"%"SVf", stopped",
			SVfARG(sv_2mortal(vnormal(req))),
			SVfARG(sv_2mortal(vnormal(PL_patchlevel)))
		    );
		}
		else { /* probably 'use 5.10' or 'use 5.8' */
		    SV *hintsv;
		    I32 second = 0;

		    if (av_len(lav)>=1) 
			second = SvIV(*av_fetch(lav,1,0));

		    second /= second >= 600  ? 100 : 10;
		    hintsv = Perl_newSVpvf(aTHX_ "v%d.%d.0",
					   (int)first, (int)second);
		    upg_version(hintsv, TRUE);

		    DIE(aTHX_ "Perl %"SVf" required (did you mean %"SVf"?)"
		    	"--this is only %"SVf", stopped",
			SVfARG(sv_2mortal(vnormal(req))),
			SVfARG(sv_2mortal(vnormal(sv_2mortal(hintsv)))),
			SVfARG(sv_2mortal(vnormal(PL_patchlevel)))
		    );
		}
	    }
	}

	RETPUSHYES;
    }
    name = SvPV_const(sv, len);
    if (!(name && len > 0 && *name))
	DIE(aTHX_ "Null filename used");
    TAINT_PROPER("require");


#ifdef VMS
    /* The key in the %ENV hash is in the syntax of file passed as the argument
     * usually this is in UNIX format, but sometimes in VMS format, which
     * can result in a module being pulled in more than once.
     * To prevent this, the key must be stored in UNIX format if the VMS
     * name can be translated to UNIX.
     */
    if ((unixname = tounixspec(name, NULL)) != NULL) {
	unixlen = strlen(unixname);
	vms_unixname = 1;
    }
    else
#endif
    {
        /* if not VMS or VMS name can not be translated to UNIX, pass it
	 * through.
	 */
	unixname = (char *) name;
	unixlen = len;
    }
    if (PL_op->op_type == OP_REQUIRE) {
	SV * const * const svp = hv_fetch(GvHVn(PL_incgv),
					  unixname, unixlen, 0);
	if ( svp ) {
	    if (*svp != &PL_sv_undef)
		RETPUSHYES;
	    else
		DIE(aTHX_ "Attempt to reload %s aborted.\n"
			    "Compilation failed in require", unixname);
	}
    }

    /* prepare to compile file */

    if (path_is_absolute(name)) {
	/* At this point, name is SvPVX(sv)  */
	tryname = name;
	tryrsfp = doopen_pm(sv);
    }
    if (!tryrsfp) {
	AV * const ar = GvAVn(PL_incgv);
	I32 i;
#ifdef VMS
	if (vms_unixname)
#endif
	{
	    namesv = newSV_type(SVt_PV);
	    for (i = 0; i <= AvFILL(ar); i++) {
		SV * const dirsv = *av_fetch(ar, i, TRUE);

		if (SvTIED_mg((const SV *)ar, PERL_MAGIC_tied))
		    mg_get(dirsv);
		if (SvROK(dirsv)) {
		    int count;
		    SV **svp;
		    SV *loader = dirsv;

		    if (SvTYPE(SvRV(loader)) == SVt_PVAV
			&& !sv_isobject(loader))
		    {
			loader = *av_fetch(MUTABLE_AV(SvRV(loader)), 0, TRUE);
		    }

		    Perl_sv_setpvf(aTHX_ namesv, "/loader/0x%"UVxf"/%s",
				   PTR2UV(SvRV(dirsv)), name);
		    tryname = SvPVX_const(namesv);
		    tryrsfp = NULL;

		    ENTER_with_name("call_INC");
		    SAVETMPS;
		    EXTEND(SP, 2);

		    PUSHMARK(SP);
		    PUSHs(dirsv);
		    PUSHs(sv);
		    PUTBACK;
		    if (sv_isobject(loader))
			count = call_method("INC", G_ARRAY);
		    else
			count = call_sv(loader, G_ARRAY);
		    SPAGAIN;

		    if (count > 0) {
			int i = 0;
			SV *arg;

			SP -= count - 1;
			arg = SP[i++];

			if (SvROK(arg) && (SvTYPE(SvRV(arg)) <= SVt_PVLV)
			    && !isGV_with_GP(SvRV(arg))) {
			    filter_cache = SvRV(arg);
			    SvREFCNT_inc_simple_void_NN(filter_cache);

			    if (i < count) {
				arg = SP[i++];
			    }
			}

			if (SvROK(arg) && isGV_with_GP(SvRV(arg))) {
			    arg = SvRV(arg);
			}

			if (isGV_with_GP(arg)) {
			    IO * const io = GvIO((const GV *)arg);

			    ++filter_has_file;

			    if (io) {
				tryrsfp = IoIFP(io);
				if (IoOFP(io) && IoOFP(io) != IoIFP(io)) {
				    PerlIO_close(IoOFP(io));
				}
				IoIFP(io) = NULL;
				IoOFP(io) = NULL;
			    }

			    if (i < count) {
				arg = SP[i++];
			    }
			}

			if (SvROK(arg) && SvTYPE(SvRV(arg)) == SVt_PVCV) {
			    filter_sub = arg;
			    SvREFCNT_inc_simple_void_NN(filter_sub);

			    if (i < count) {
				filter_state = SP[i];
				SvREFCNT_inc_simple_void(filter_state);
			    }
			}

			if (!tryrsfp && (filter_cache || filter_sub)) {
			    tryrsfp = PerlIO_open(BIT_BUCKET,
						  PERL_SCRIPT_MODE);
			}
			SP--;
		    }

		    PUTBACK;
		    FREETMPS;
		    LEAVE_with_name("call_INC");

		    /* Adjust file name if the hook has set an %INC entry.
		       This needs to happen after the FREETMPS above.  */
		    svp = hv_fetch(GvHVn(PL_incgv), name, len, 0);
		    if (svp)
			tryname = SvPV_nolen_const(*svp);

		    if (tryrsfp) {
			hook_sv = dirsv;
			break;
		    }

		    filter_has_file = 0;
		    if (filter_cache) {
			SvREFCNT_dec(filter_cache);
			filter_cache = NULL;
		    }
		    if (filter_state) {
			SvREFCNT_dec(filter_state);
			filter_state = NULL;
		    }
		    if (filter_sub) {
			SvREFCNT_dec(filter_sub);
			filter_sub = NULL;
		    }
		}
		else {
		  if (!path_is_absolute(name)
		  ) {
		    const char *dir;
		    STRLEN dirlen;

		    if (SvOK(dirsv)) {
			dir = SvPV_const(dirsv, dirlen);
		    } else {
			dir = "";
			dirlen = 0;
		    }

#ifdef VMS
		    char *unixdir;
		    if ((unixdir = tounixpath(dir, NULL)) == NULL)
			continue;
		    sv_setpv(namesv, unixdir);
		    sv_catpv(namesv, unixname);
#else
#  ifdef __SYMBIAN32__
		    if (PL_origfilename[0] &&
			PL_origfilename[1] == ':' &&
			!(dir[0] && dir[1] == ':'))
		        Perl_sv_setpvf(aTHX_ namesv,
				       "%c:%s\\%s",
				       PL_origfilename[0],
				       dir, name);
		    else
		        Perl_sv_setpvf(aTHX_ namesv,
				       "%s\\%s",
				       dir, name);
#  else
		    /* The equivalent of		    
		       Perl_sv_setpvf(aTHX_ namesv, "%s/%s", dir, name);
		       but without the need to parse the format string, or
		       call strlen on either pointer, and with the correct
		       allocation up front.  */
		    {
			char *tmp = SvGROW(namesv, dirlen + len + 2);

			memcpy(tmp, dir, dirlen);
			tmp +=dirlen;
			*tmp++ = '/';
			/* name came from an SV, so it will have a '\0' at the
			   end that we can copy as part of this memcpy().  */
			memcpy(tmp, name, len + 1);

			SvCUR_set(namesv, dirlen + len + 1);
			SvPOK_on(namesv);
		    }
#  endif
#endif
		    TAINT_PROPER("require");
		    tryname = SvPVX_const(namesv);
		    tryrsfp = doopen_pm(namesv);
		    if (tryrsfp) {
			if (tryname[0] == '.' && tryname[1] == '/') {
			    ++tryname;
			    while (*++tryname == '/');
			}
			break;
		    }
		    else if (errno == EMFILE)
			/* no point in trying other paths if out of handles */
			break;
		  }
		}
	    }
	}
    }
    sv_2mortal(namesv);
    if (!tryrsfp) {
	if (PL_op->op_type == OP_REQUIRE) {
	    if(errno == EMFILE) {
		/* diag_listed_as: Can't locate %s */
		DIE(aTHX_ "Can't locate %s:   %s", name, Strerror(errno));
	    } else {
	        if (namesv) {			/* did we lookup @INC? */
		    AV * const ar = GvAVn(PL_incgv);
		    I32 i;
		    SV *const inc = newSVpvs_flags("", SVs_TEMP);
		    for (i = 0; i <= AvFILL(ar); i++) {
			sv_catpvs(inc, " ");
			sv_catsv(inc, *av_fetch(ar, i, TRUE));
		    }

		    /* diag_listed_as: Can't locate %s */
		    DIE(aTHX_
			"Can't locate %s in @INC%s%s (@INC contains:%" SVf ")",
			name,
			(memEQ(name + len - 2, ".h", 3)
			 ? " (change .h to .ph maybe?) (did you run h2ph?)" : ""),
			(memEQ(name + len - 3, ".ph", 4)
			 ? " (did you run h2ph?)" : ""),
			inc
			);
		}
	    }
	    DIE(aTHX_ "Can't locate %s", name);
	}

	RETPUSHUNDEF;
    }
    else
	SETERRNO(0, SS_NORMAL);

    /* Assume success here to prevent recursive requirement. */
    /* name is never assigned to again, so len is still strlen(name)  */
    /* Check whether a hook in @INC has already filled %INC */
    if (!hook_sv) {
	(void)hv_store(GvHVn(PL_incgv),
		       unixname, unixlen, newSVpv(tryname,0),0);
    } else {
	SV** const svp = hv_fetch(GvHVn(PL_incgv), unixname, unixlen, 0);
	if (!svp)
	    (void)hv_store(GvHVn(PL_incgv),
			   unixname, unixlen, SvREFCNT_inc_simple(hook_sv), 0 );
    }

    ENTER_with_name("eval");
    SAVETMPS;
    SAVECOPFILE_FREE(&PL_compiling);
    CopFILE_set(&PL_compiling, tryname);
    lex_start(NULL, tryrsfp, 0);

    if (filter_sub || filter_cache) {
	/* We can use the SvPV of the filter PVIO itself as our cache, rather
	   than hanging another SV from it. In turn, filter_add() optionally
	   takes the SV to use as the filter (or creates a new SV if passed
	   NULL), so simply pass in whatever value filter_cache has.  */
	SV * const datasv = filter_add(S_run_user_filter, filter_cache);
	IoLINES(datasv) = filter_has_file;
	IoTOP_GV(datasv) = MUTABLE_GV(filter_state);
	IoBOTTOM_GV(datasv) = MUTABLE_GV(filter_sub);
    }

    /* switch to eval mode */
    PUSHBLOCK(cx, CXt_EVAL, SP);
    PUSHEVAL(cx, name);
    cx->blk_eval.retop = PL_op->op_next;

    SAVECOPLINE(&PL_compiling);
    CopLINE_set(&PL_compiling, 0);

    PUTBACK;

    /* Store and reset encoding. */
    encoding = PL_encoding;
    PL_encoding = NULL;

    if (doeval(gimme, NULL, NULL, PL_curcop->cop_seq, NULL))
	op = DOCATCH(PL_eval_start);
    else
	op = PL_op->op_next;

    /* Restore encoding. */
    PL_encoding = encoding;

    return op;
}

/* This is a op added to hold the hints hash for
   pp_entereval. The hash can be modified by the code
   being eval'ed, so we return a copy instead. */

PP(pp_hintseval)
{
    dVAR;
    dSP;
    mXPUSHs(MUTABLE_SV(hv_copy_hints_hv(MUTABLE_HV(cSVOP_sv))));
    RETURN;
}


PP(pp_entereval)
{
    dVAR; dSP;
    register PERL_CONTEXT *cx;
    SV *sv;
    const I32 gimme = GIMME_V;
    const U32 was = PL_breakable_sub_gen;
    char tbuf[TYPE_DIGITS(long) + 12];
    bool saved_delete = FALSE;
    char *tmpbuf = tbuf;
    STRLEN len;
    CV* runcv;
    U32 seq, lex_flags = 0;
    HV *saved_hh = NULL;
    const bool bytes = PL_op->op_private & OPpEVAL_BYTES;

    if (PL_op->op_private & OPpEVAL_HAS_HH) {
	saved_hh = MUTABLE_HV(SvREFCNT_inc(POPs));
    }
    else if (PL_hints & HINT_LOCALIZE_HH || (
	        PL_op->op_private & OPpEVAL_COPHH
	     && PL_curcop->cop_hints & HINT_LOCALIZE_HH
	    )) {
	saved_hh = cop_hints_2hv(PL_curcop, 0);
	hv_magic(saved_hh, NULL, PERL_MAGIC_hints);
    }
    sv = POPs;
    if (!SvPOK(sv)) {
	/* make sure we've got a plain PV (no overload etc) before testing
	 * for taint. Making a copy here is probably overkill, but better
	 * safe than sorry */
	STRLEN len;
	const char * const p = SvPV_const(sv, len);

	sv = newSVpvn_flags(p, len, SVs_TEMP | SvUTF8(sv));
	lex_flags |= LEX_START_COPIED;

	if (bytes && SvUTF8(sv))
	    SvPVbyte_force(sv, len);
    }
    else if (bytes && SvUTF8(sv)) {
	/* Don't modify someone else's scalar */
	STRLEN len;
	sv = newSVsv(sv);
	(void)sv_2mortal(sv);
	SvPVbyte_force(sv,len);
	lex_flags |= LEX_START_COPIED;
    }

    TAINT_IF(SvTAINTED(sv));
    TAINT_PROPER("eval");

    ENTER_with_name("eval");
    lex_start(sv, NULL, lex_flags | (PL_op->op_private & OPpEVAL_UNICODE
			   ? LEX_IGNORE_UTF8_HINTS
			   : bytes ? LEX_EVALBYTES : LEX_START_SAME_FILTER
			)
	     );
    SAVETMPS;

    /* switch to eval mode */

    if (PERLDB_NAMEEVAL && CopLINE(PL_curcop)) {
	SV * const temp_sv = sv_newmortal();
	Perl_sv_setpvf(aTHX_ temp_sv, "_<(eval %lu)[%s:%"IVdf"]",
		       (unsigned long)++PL_evalseq,
		       CopFILE(PL_curcop), (IV)CopLINE(PL_curcop));
	tmpbuf = SvPVX(temp_sv);
	len = SvCUR(temp_sv);
    }
    else
	len = my_snprintf(tmpbuf, sizeof(tbuf), "_<(eval %lu)", (unsigned long)++PL_evalseq);
    SAVECOPFILE_FREE(&PL_compiling);
    CopFILE_set(&PL_compiling, tmpbuf+2);
    SAVECOPLINE(&PL_compiling);
    CopLINE_set(&PL_compiling, 1);
    /* special case: an eval '' executed within the DB package gets lexically
     * placed in the first non-DB CV rather than the current CV - this
     * allows the debugger to execute code, find lexicals etc, in the
     * scope of the code being debugged. Passing &seq gets find_runcv
     * to do the dirty work for us */
    runcv = find_runcv(&seq);

    PUSHBLOCK(cx, (CXt_EVAL|CXp_REAL), SP);
    PUSHEVAL(cx, 0);
    cx->blk_eval.retop = PL_op->op_next;

    /* prepare to compile string */

    if ((PERLDB_LINE || PERLDB_SAVESRC) && PL_curstash != PL_debstash)
	save_lines(CopFILEAV(&PL_compiling), PL_parser->linestr);
    else {
	/* XXX For C<eval "...">s within BEGIN {} blocks, this ends up
	   deleting the eval's FILEGV from the stash before gv_check() runs
	   (i.e. before run-time proper). To work around the coredump that
	   ensues, we always turn GvMULTI_on for any globals that were
	   introduced within evals. See force_ident(). GSAR 96-10-12 */
	char *const safestr = savepvn(tmpbuf, len);
	SAVEDELETE(PL_defstash, safestr, len);
	saved_delete = TRUE;
    }
    
    PUTBACK;

    if (doeval(gimme, NULL, runcv, seq, saved_hh)) {
	if (was != PL_breakable_sub_gen /* Some subs defined here. */
	    ? (PERLDB_LINE || PERLDB_SAVESRC)
	    :  PERLDB_SAVESRC_NOSUBS) {
	    /* Retain the filegv we created.  */
	} else if (!saved_delete) {
	    char *const safestr = savepvn(tmpbuf, len);
	    SAVEDELETE(PL_defstash, safestr, len);
	}
	return DOCATCH(PL_eval_start);
    } else {
	/* We have already left the scope set up earlier thanks to the LEAVE
	   in doeval().  */
	if (was != PL_breakable_sub_gen /* Some subs defined here. */
	    ? (PERLDB_LINE || PERLDB_SAVESRC)
	    :  PERLDB_SAVESRC_INVALID) {
	    /* Retain the filegv we created.  */
	} else if (!saved_delete) {
	    (void)hv_delete(PL_defstash, tmpbuf, len, G_DISCARD);
	}
	return PL_op->op_next;
    }
}

PP(pp_leaveeval)
{
    dVAR; dSP;
    SV **newsp;
    PMOP *newpm;
    I32 gimme;
    register PERL_CONTEXT *cx;
    OP *retop;
    const U8 save_flags = PL_op -> op_flags;
    I32 optype;
    SV *namesv;
    CV *evalcv;

    PERL_ASYNC_CHECK();
    POPBLOCK(cx,newpm);
    POPEVAL(cx);
    namesv = cx->blk_eval.old_namesv;
    retop = cx->blk_eval.retop;
    evalcv = cx->blk_eval.cv;

    TAINT_NOT;
    SP = adjust_stack_on_leave((gimme == G_VOID) ? SP : newsp, SP, newsp,
				gimme, SVs_TEMP);
    PL_curpm = newpm;	/* Don't pop $1 et al till now */

#ifdef DEBUGGING
    assert(CvDEPTH(evalcv) == 1);
#endif
    CvDEPTH(evalcv) = 0;

    if (optype == OP_REQUIRE &&
	!(gimme == G_SCALAR ? SvTRUE(*SP) : SP > newsp))
    {
	/* Unassume the success we assumed earlier. */
	(void)hv_delete(GvHVn(PL_incgv),
			SvPVX_const(namesv),
                        SvUTF8(namesv) ? -(I32)SvCUR(namesv) : (I32)SvCUR(namesv),
			G_DISCARD);
	retop = Perl_die(aTHX_ "%"SVf" did not return a true value",
			       SVfARG(namesv));
	/* die_unwind() did LEAVE, or we won't be here */
    }
    else {
	LEAVE_with_name("eval");
	if (!(save_flags & OPf_SPECIAL)) {
	    CLEAR_ERRSV();
	}
    }

    RETURNOP(retop);
}

/* Common code for Perl_call_sv and Perl_fold_constants, put here to keep it
   close to the related Perl_create_eval_scope.  */
void
Perl_delete_eval_scope(pTHX)
{
    SV **newsp;
    PMOP *newpm;
    I32 gimme;
    register PERL_CONTEXT *cx;
    I32 optype;
	
    POPBLOCK(cx,newpm);
    POPEVAL(cx);
    PL_curpm = newpm;
    LEAVE_with_name("eval_scope");
    PERL_UNUSED_VAR(newsp);
    PERL_UNUSED_VAR(gimme);
    PERL_UNUSED_VAR(optype);
}

/* Common-ish code salvaged from Perl_call_sv and pp_entertry, because it was
   also needed by Perl_fold_constants.  */
PERL_CONTEXT *
Perl_create_eval_scope(pTHX_ U32 flags)
{
    PERL_CONTEXT *cx;
    const I32 gimme = GIMME_V;
	
    ENTER_with_name("eval_scope");
    SAVETMPS;

    PUSHBLOCK(cx, (CXt_EVAL|CXp_TRYBLOCK), PL_stack_sp);
    PUSHEVAL(cx, 0);

    PL_in_eval = EVAL_INEVAL;
    if (flags & G_KEEPERR)
	PL_in_eval |= EVAL_KEEPERR;
    else
	CLEAR_ERRSV();
    if (flags & G_FAKINGEVAL) {
	PL_eval_root = PL_op; /* Only needed so that goto works right. */
    }
    return cx;
}
    
PP(pp_entertry)
{
    dVAR;
    PERL_CONTEXT * const cx = create_eval_scope(0);
    cx->blk_eval.retop = cLOGOP->op_other->op_next;
    return DOCATCH(PL_op->op_next);
}

PP(pp_leavetry)
{
    dVAR; dSP;
    SV **newsp;
    PMOP *newpm;
    I32 gimme;
    register PERL_CONTEXT *cx;
    I32 optype;

    PERL_ASYNC_CHECK();
    POPBLOCK(cx,newpm);
    POPEVAL(cx);
    PERL_UNUSED_VAR(optype);

    TAINT_NOT;
    SP = adjust_stack_on_leave(newsp, SP, newsp, gimme, SVs_PADTMP|SVs_TEMP);
    PL_curpm = newpm;	/* Don't pop $1 et al till now */

    LEAVE_with_name("eval_scope");
    CLEAR_ERRSV();
    RETURN;
}

PP(pp_entergiven)
{
    dVAR; dSP;
    register PERL_CONTEXT *cx;
    const I32 gimme = GIMME_V;
    
    ENTER_with_name("given");
    SAVETMPS;

    SAVECLEARSV(PAD_SVl(PL_op->op_targ));
    sv_setsv_mg(PAD_SV(PL_op->op_targ), POPs);

    PUSHBLOCK(cx, CXt_GIVEN, SP);
    PUSHGIVEN(cx);

    RETURN;
}

PP(pp_leavegiven)
{
    dVAR; dSP;
    register PERL_CONTEXT *cx;
    I32 gimme;
    SV **newsp;
    PMOP *newpm;
    PERL_UNUSED_CONTEXT;

    POPBLOCK(cx,newpm);
    assert(CxTYPE(cx) == CXt_GIVEN);

    TAINT_NOT;
    SP = adjust_stack_on_leave(newsp, SP, newsp, gimme, SVs_PADTMP|SVs_TEMP);
    PL_curpm = newpm;	/* Don't pop $1 et al till now */

    LEAVE_with_name("given");
    RETURN;
}

/* Helper routines used by pp_smartmatch */
STATIC PMOP *
S_make_matcher(pTHX_ REGEXP *re)
{
    dVAR;
    PMOP *matcher = (PMOP *) newPMOP(OP_MATCH, OPf_WANT_SCALAR | OPf_STACKED);

    PERL_ARGS_ASSERT_MAKE_MATCHER;

    PM_SETRE(matcher, ReREFCNT_inc(re));

    SAVEFREEOP((OP *) matcher);
    ENTER_with_name("matcher"); SAVETMPS;
    SAVEOP();
    return matcher;
}

STATIC bool
S_matcher_matches_sv(pTHX_ PMOP *matcher, SV *sv)
{
    dVAR;
    dSP;

    PERL_ARGS_ASSERT_MATCHER_MATCHES_SV;
    
    PL_op = (OP *) matcher;
    XPUSHs(sv);
    PUTBACK;
    (void) Perl_pp_match(aTHX);
    SPAGAIN;
    return (SvTRUEx(POPs));
}

STATIC void
S_destroy_matcher(pTHX_ PMOP *matcher)
{
    dVAR;

    PERL_ARGS_ASSERT_DESTROY_MATCHER;
    PERL_UNUSED_ARG(matcher);

    FREETMPS;
    LEAVE_with_name("matcher");
}

/* Do a smart match */
PP(pp_smartmatch)
{
    DEBUG_M(Perl_deb(aTHX_ "Starting smart match resolution\n"));
    return do_smartmatch(NULL, NULL, 0);
}

/* This version of do_smartmatch() implements the
 * table of smart matches that is found in perlsyn.
 */
STATIC OP *
S_do_smartmatch(pTHX_ HV *seen_this, HV *seen_other, const bool copied)
{
    dVAR;
    dSP;
    
    bool object_on_left = FALSE;
    SV *e = TOPs;	/* e is for 'expression' */
    SV *d = TOPm1s;	/* d is for 'default', as in PL_defgv */

    /* Take care only to invoke mg_get() once for each argument.
     * Currently we do this by copying the SV if it's magical. */
    if (d) {
	if (!copied && SvGMAGICAL(d))
	    d = sv_mortalcopy(d);
    }
    else
	d = &PL_sv_undef;

    assert(e);
    if (SvGMAGICAL(e))
	e = sv_mortalcopy(e);

    /* First of all, handle overload magic of the rightmost argument */
    if (SvAMAGIC(e)) {
	SV * tmpsv;
	DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Object\n"));
	DEBUG_M(Perl_deb(aTHX_ "        attempting overload\n"));

	tmpsv = amagic_call(d, e, smart_amg, AMGf_noleft);
	if (tmpsv) {
	    SPAGAIN;
	    (void)POPs;
	    SETs(tmpsv);
	    RETURN;
	}
	DEBUG_M(Perl_deb(aTHX_ "        failed to run overload method; continuing...\n"));
    }

    SP -= 2;	/* Pop the values */


    /* ~~ undef */
    if (!SvOK(e)) {
	DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-undef\n"));
	if (SvOK(d))
	    RETPUSHNO;
	else
	    RETPUSHYES;
    }

    if (sv_isobject(e) && (SvTYPE(SvRV(e)) != SVt_REGEXP)) {
	DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Object\n"));
	Perl_croak(aTHX_ "Smart matching a non-overloaded object breaks encapsulation");
    }
    if (sv_isobject(d) && (SvTYPE(SvRV(d)) != SVt_REGEXP))
	object_on_left = TRUE;

    /* ~~ sub */
    if (SvROK(e) && SvTYPE(SvRV(e)) == SVt_PVCV) {
	I32 c;
	if (object_on_left) {
	    goto sm_any_sub; /* Treat objects like scalars */
	}
	else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVHV) {
	    /* Test sub truth for each key */
	    HE *he;
	    bool andedresults = TRUE;
	    HV *hv = (HV*) SvRV(d);
	    I32 numkeys = hv_iterinit(hv);
	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Hash-CodeRef\n"));
	    if (numkeys == 0)
		RETPUSHYES;
	    while ( (he = hv_iternext(hv)) ) {
		DEBUG_M(Perl_deb(aTHX_ "        testing hash key...\n"));
		ENTER_with_name("smartmatch_hash_key_test");
		SAVETMPS;
		PUSHMARK(SP);
		PUSHs(hv_iterkeysv(he));
		PUTBACK;
		c = call_sv(e, G_SCALAR);
		SPAGAIN;
		if (c == 0)
		    andedresults = FALSE;
		else
		    andedresults = SvTRUEx(POPs) && andedresults;
		FREETMPS;
		LEAVE_with_name("smartmatch_hash_key_test");
	    }
	    if (andedresults)
		RETPUSHYES;
	    else
		RETPUSHNO;
	}
	else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVAV) {
	    /* Test sub truth for each element */
	    I32 i;
	    bool andedresults = TRUE;
	    AV *av = (AV*) SvRV(d);
	    const I32 len = av_len(av);
	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Array-CodeRef\n"));
	    if (len == -1)
		RETPUSHYES;
	    for (i = 0; i <= len; ++i) {
		SV * const * const svp = av_fetch(av, i, FALSE);
		DEBUG_M(Perl_deb(aTHX_ "        testing array element...\n"));
		ENTER_with_name("smartmatch_array_elem_test");
		SAVETMPS;
		PUSHMARK(SP);
		if (svp)
		    PUSHs(*svp);
		PUTBACK;
		c = call_sv(e, G_SCALAR);
		SPAGAIN;
		if (c == 0)
		    andedresults = FALSE;
		else
		    andedresults = SvTRUEx(POPs) && andedresults;
		FREETMPS;
		LEAVE_with_name("smartmatch_array_elem_test");
	    }
	    if (andedresults)
		RETPUSHYES;
	    else
		RETPUSHNO;
	}
	else {
	  sm_any_sub:
	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-CodeRef\n"));
	    ENTER_with_name("smartmatch_coderef");
	    SAVETMPS;
	    PUSHMARK(SP);
	    PUSHs(d);
	    PUTBACK;
	    c = call_sv(e, G_SCALAR);
	    SPAGAIN;
	    if (c == 0)
		PUSHs(&PL_sv_no);
	    else if (SvTEMP(TOPs))
		SvREFCNT_inc_void(TOPs);
	    FREETMPS;
	    LEAVE_with_name("smartmatch_coderef");
	    RETURN;
	}
    }
    /* ~~ %hash */
    else if (SvROK(e) && SvTYPE(SvRV(e)) == SVt_PVHV) {
	if (object_on_left) {
	    goto sm_any_hash; /* Treat objects like scalars */
	}
	else if (!SvOK(d)) {
	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Hash ($a undef)\n"));
	    RETPUSHNO;
	}
	else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVHV) {
	    /* Check that the key-sets are identical */
	    HE *he;
	    HV *other_hv = MUTABLE_HV(SvRV(d));
	    bool tied = FALSE;
	    bool other_tied = FALSE;
	    U32 this_key_count  = 0,
	        other_key_count = 0;
	    HV *hv = MUTABLE_HV(SvRV(e));

	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Hash-Hash\n"));
	    /* Tied hashes don't know how many keys they have. */
	    if (SvTIED_mg((SV*)hv, PERL_MAGIC_tied)) {
		tied = TRUE;
	    }
	    else if (SvTIED_mg((const SV *)other_hv, PERL_MAGIC_tied)) {
		HV * const temp = other_hv;
		other_hv = hv;
		hv = temp;
		tied = TRUE;
	    }
	    if (SvTIED_mg((const SV *)other_hv, PERL_MAGIC_tied))
		other_tied = TRUE;
	    
	    if (!tied && HvUSEDKEYS((const HV *) hv) != HvUSEDKEYS(other_hv))
	    	RETPUSHNO;

	    /* The hashes have the same number of keys, so it suffices
	       to check that one is a subset of the other. */
	    (void) hv_iterinit(hv);
	    while ( (he = hv_iternext(hv)) ) {
		SV *key = hv_iterkeysv(he);

		DEBUG_M(Perl_deb(aTHX_ "        comparing hash key...\n"));
	    	++ this_key_count;
	    	
	    	if(!hv_exists_ent(other_hv, key, 0)) {
	    	    (void) hv_iterinit(hv);	/* reset iterator */
		    RETPUSHNO;
	    	}
	    }
	    
	    if (other_tied) {
		(void) hv_iterinit(other_hv);
		while ( hv_iternext(other_hv) )
		    ++other_key_count;
	    }
	    else
		other_key_count = HvUSEDKEYS(other_hv);
	    
	    if (this_key_count != other_key_count)
		RETPUSHNO;
	    else
		RETPUSHYES;
	}
	else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVAV) {
	    AV * const other_av = MUTABLE_AV(SvRV(d));
	    const I32 other_len = av_len(other_av) + 1;
	    I32 i;
	    HV *hv = MUTABLE_HV(SvRV(e));

	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Array-Hash\n"));
	    for (i = 0; i < other_len; ++i) {
		SV ** const svp = av_fetch(other_av, i, FALSE);
		DEBUG_M(Perl_deb(aTHX_ "        checking for key existence...\n"));
		if (svp) {	/* ??? When can this not happen? */
		    if (hv_exists_ent(hv, *svp, 0))
		        RETPUSHYES;
		}
	    }
	    RETPUSHNO;
	}
	else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_REGEXP) {
	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Regex-Hash\n"));
	  sm_regex_hash:
	    {
		PMOP * const matcher = make_matcher((REGEXP*) SvRV(d));
		HE *he;
		HV *hv = MUTABLE_HV(SvRV(e));

		(void) hv_iterinit(hv);
		while ( (he = hv_iternext(hv)) ) {
		    DEBUG_M(Perl_deb(aTHX_ "        testing key against pattern...\n"));
		    if (matcher_matches_sv(matcher, hv_iterkeysv(he))) {
			(void) hv_iterinit(hv);
			destroy_matcher(matcher);
			RETPUSHYES;
		    }
		}
		destroy_matcher(matcher);
		RETPUSHNO;
	    }
	}
	else {
	  sm_any_hash:
	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Hash\n"));
	    if (hv_exists_ent(MUTABLE_HV(SvRV(e)), d, 0))
		RETPUSHYES;
	    else
		RETPUSHNO;
	}
    }
    /* ~~ @array */
    else if (SvROK(e) && SvTYPE(SvRV(e)) == SVt_PVAV) {
	if (object_on_left) {
	    goto sm_any_array; /* Treat objects like scalars */
	}
	else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVHV) {
	    AV * const other_av = MUTABLE_AV(SvRV(e));
	    const I32 other_len = av_len(other_av) + 1;
	    I32 i;

	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Hash-Array\n"));
	    for (i = 0; i < other_len; ++i) {
		SV ** const svp = av_fetch(other_av, i, FALSE);

		DEBUG_M(Perl_deb(aTHX_ "        testing for key existence...\n"));
		if (svp) {	/* ??? When can this not happen? */
		    if (hv_exists_ent(MUTABLE_HV(SvRV(d)), *svp, 0))
		        RETPUSHYES;
		}
	    }
	    RETPUSHNO;
	}
	if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVAV) {
	    AV *other_av = MUTABLE_AV(SvRV(d));
	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Array-Array\n"));
	    if (av_len(MUTABLE_AV(SvRV(e))) != av_len(other_av))
		RETPUSHNO;
	    else {
	    	I32 i;
	    	const I32 other_len = av_len(other_av);

		if (NULL == seen_this) {
		    seen_this = newHV();
		    (void) sv_2mortal(MUTABLE_SV(seen_this));
		}
		if (NULL == seen_other) {
		    seen_other = newHV();
		    (void) sv_2mortal(MUTABLE_SV(seen_other));
		}
		for(i = 0; i <= other_len; ++i) {
		    SV * const * const this_elem = av_fetch(MUTABLE_AV(SvRV(e)), i, FALSE);
		    SV * const * const other_elem = av_fetch(other_av, i, FALSE);

		    if (!this_elem || !other_elem) {
			if ((this_elem && SvOK(*this_elem))
				|| (other_elem && SvOK(*other_elem)))
			    RETPUSHNO;
		    }
		    else if (hv_exists_ent(seen_this,
				sv_2mortal(newSViv(PTR2IV(*this_elem))), 0) ||
			    hv_exists_ent(seen_other,
				sv_2mortal(newSViv(PTR2IV(*other_elem))), 0))
		    {
			if (*this_elem != *other_elem)
			    RETPUSHNO;
		    }
		    else {
			(void)hv_store_ent(seen_this,
				sv_2mortal(newSViv(PTR2IV(*this_elem))),
				&PL_sv_undef, 0);
			(void)hv_store_ent(seen_other,
				sv_2mortal(newSViv(PTR2IV(*other_elem))),
				&PL_sv_undef, 0);
			PUSHs(*other_elem);
			PUSHs(*this_elem);
			
			PUTBACK;
			DEBUG_M(Perl_deb(aTHX_ "        recursively comparing array element...\n"));
			(void) do_smartmatch(seen_this, seen_other, 0);
			SPAGAIN;
			DEBUG_M(Perl_deb(aTHX_ "        recursion finished\n"));
			
			if (!SvTRUEx(POPs))
			    RETPUSHNO;
		    }
		}
		RETPUSHYES;
	    }
	}
	else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_REGEXP) {
	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Regex-Array\n"));
	  sm_regex_array:
	    {
		PMOP * const matcher = make_matcher((REGEXP*) SvRV(d));
		const I32 this_len = av_len(MUTABLE_AV(SvRV(e)));
		I32 i;

		for(i = 0; i <= this_len; ++i) {
		    SV * const * const svp = av_fetch(MUTABLE_AV(SvRV(e)), i, FALSE);
		    DEBUG_M(Perl_deb(aTHX_ "        testing element against pattern...\n"));
		    if (svp && matcher_matches_sv(matcher, *svp)) {
			destroy_matcher(matcher);
			RETPUSHYES;
		    }
		}
		destroy_matcher(matcher);
		RETPUSHNO;
	    }
	}
	else if (!SvOK(d)) {
	    /* undef ~~ array */
	    const I32 this_len = av_len(MUTABLE_AV(SvRV(e)));
	    I32 i;

	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Undef-Array\n"));
	    for (i = 0; i <= this_len; ++i) {
		SV * const * const svp = av_fetch(MUTABLE_AV(SvRV(e)), i, FALSE);
		DEBUG_M(Perl_deb(aTHX_ "        testing for undef element...\n"));
		if (!svp || !SvOK(*svp))
		    RETPUSHYES;
	    }
	    RETPUSHNO;
	}
	else {
	  sm_any_array:
	    {
		I32 i;
		const I32 this_len = av_len(MUTABLE_AV(SvRV(e)));

		DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Array\n"));
		for (i = 0; i <= this_len; ++i) {
		    SV * const * const svp = av_fetch(MUTABLE_AV(SvRV(e)), i, FALSE);
		    if (!svp)
			continue;

		    PUSHs(d);
		    PUSHs(*svp);
		    PUTBACK;
		    /* infinite recursion isn't supposed to happen here */
		    DEBUG_M(Perl_deb(aTHX_ "        recursively testing array element...\n"));
		    (void) do_smartmatch(NULL, NULL, 1);
		    SPAGAIN;
		    DEBUG_M(Perl_deb(aTHX_ "        recursion finished\n"));
		    if (SvTRUEx(POPs))
			RETPUSHYES;
		}
		RETPUSHNO;
	    }
	}
    }
    /* ~~ qr// */
    else if (SvROK(e) && SvTYPE(SvRV(e)) == SVt_REGEXP) {
	if (!object_on_left && SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVHV) {
	    SV *t = d; d = e; e = t;
	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Hash-Regex\n"));
	    goto sm_regex_hash;
	}
	else if (!object_on_left && SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVAV) {
	    SV *t = d; d = e; e = t;
	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Array-Regex\n"));
	    goto sm_regex_array;
	}
	else {
	    PMOP * const matcher = make_matcher((REGEXP*) SvRV(e));

	    DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Regex\n"));
	    PUTBACK;
	    PUSHs(matcher_matches_sv(matcher, d)
		    ? &PL_sv_yes
		    : &PL_sv_no);
	    destroy_matcher(matcher);
	    RETURN;
	}
    }
    /* ~~ scalar */
    /* See if there is overload magic on left */
    else if (object_on_left && SvAMAGIC(d)) {
	SV *tmpsv;
	DEBUG_M(Perl_deb(aTHX_ "    applying rule Object-Any\n"));
	DEBUG_M(Perl_deb(aTHX_ "        attempting overload\n"));
	PUSHs(d); PUSHs(e);
	PUTBACK;
	tmpsv = amagic_call(d, e, smart_amg, AMGf_noright);
	if (tmpsv) {
	    SPAGAIN;
	    (void)POPs;
	    SETs(tmpsv);
	    RETURN;
	}
	SP -= 2;
	DEBUG_M(Perl_deb(aTHX_ "        failed to run overload method; falling back...\n"));
	goto sm_any_scalar;
    }
    else if (!SvOK(d)) {
	/* undef ~~ scalar ; we already know that the scalar is SvOK */
	DEBUG_M(Perl_deb(aTHX_ "    applying rule undef-Any\n"));
	RETPUSHNO;
    }
    else
  sm_any_scalar:
    if (SvNIOK(e) || (SvPOK(e) && looks_like_number(e) && SvNIOK(d))) {
	DEBUG_M(if (SvNIOK(e))
		    Perl_deb(aTHX_ "    applying rule Any-Num\n");
		else
		    Perl_deb(aTHX_ "    applying rule Num-numish\n");
	);
	/* numeric comparison */
	PUSHs(d); PUSHs(e);
	PUTBACK;
	if (CopHINTS_get(PL_curcop) & HINT_INTEGER)
	    (void) Perl_pp_i_eq(aTHX);
	else
	    (void) Perl_pp_eq(aTHX);
	SPAGAIN;
	if (SvTRUEx(POPs))
	    RETPUSHYES;
	else
	    RETPUSHNO;
    }
    
    /* As a last resort, use string comparison */
    DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Any\n"));
    PUSHs(d); PUSHs(e);
    PUTBACK;
    return Perl_pp_seq(aTHX);
}

PP(pp_enterwhen)
{
    dVAR; dSP;
    register PERL_CONTEXT *cx;
    const I32 gimme = GIMME_V;

    /* This is essentially an optimization: if the match
       fails, we don't want to push a context and then
       pop it again right away, so we skip straight
       to the op that follows the leavewhen.
       RETURNOP calls PUTBACK which restores the stack pointer after the POPs.
    */
    if ((0 == (PL_op->op_flags & OPf_SPECIAL)) && !SvTRUEx(POPs))
	RETURNOP(cLOGOP->op_other->op_next);

    ENTER_with_name("when");
    SAVETMPS;

    PUSHBLOCK(cx, CXt_WHEN, SP);
    PUSHWHEN(cx);

    RETURN;
}

PP(pp_leavewhen)
{
    dVAR; dSP;
    I32 cxix;
    register PERL_CONTEXT *cx;
    I32 gimme;
    SV **newsp;
    PMOP *newpm;

    cxix = dopoptogiven(cxstack_ix);
    if (cxix < 0)
	/* diag_listed_as: Can't "when" outside a topicalizer */
	DIE(aTHX_ "Can't \"%s\" outside a topicalizer",
	           PL_op->op_flags & OPf_SPECIAL ? "default" : "when");

    POPBLOCK(cx,newpm);
    assert(CxTYPE(cx) == CXt_WHEN);

    TAINT_NOT;
    SP = adjust_stack_on_leave(newsp, SP, newsp, gimme, SVs_PADTMP|SVs_TEMP);
    PL_curpm = newpm;   /* pop $1 et al */

    LEAVE_with_name("when");

    if (cxix < cxstack_ix)
        dounwind(cxix);

    cx = &cxstack[cxix];

    if (CxFOREACH(cx)) {
	/* clear off anything above the scope we're re-entering */
	I32 inner = PL_scopestack_ix;

	TOPBLOCK(cx);
	if (PL_scopestack_ix < inner)
	    leave_scope(PL_scopestack[PL_scopestack_ix]);
	PL_curcop = cx->blk_oldcop;

	return cx->blk_loop.my_op->op_nextop;
    }
    else
	RETURNOP(cx->blk_givwhen.leave_op);
}

PP(pp_continue)
{
    dVAR; dSP;
    I32 cxix;
    register PERL_CONTEXT *cx;
    I32 gimme;
    SV **newsp;
    PMOP *newpm;

    PERL_UNUSED_VAR(gimme);
    
    cxix = dopoptowhen(cxstack_ix); 
    if (cxix < 0)   
	DIE(aTHX_ "Can't \"continue\" outside a when block");

    if (cxix < cxstack_ix)
        dounwind(cxix);
    
    POPBLOCK(cx,newpm);
    assert(CxTYPE(cx) == CXt_WHEN);

    SP = newsp;
    PL_curpm = newpm;   /* pop $1 et al */

    LEAVE_with_name("when");
    RETURNOP(cx->blk_givwhen.leave_op->op_next);
}

PP(pp_break)
{
    dVAR;   
    I32 cxix;
    register PERL_CONTEXT *cx;

    cxix = dopoptogiven(cxstack_ix); 
    if (cxix < 0)
	DIE(aTHX_ "Can't \"break\" outside a given block");

    cx = &cxstack[cxix];
    if (CxFOREACH(cx))
	DIE(aTHX_ "Can't \"break\" in a loop topicalizer");

    if (cxix < cxstack_ix)
        dounwind(cxix);

    /* Restore the sp at the time we entered the given block */
    TOPBLOCK(cx);

    return cx->blk_givwhen.leave_op;
}

static MAGIC *
S_doparseform(pTHX_ SV *sv)
{
    STRLEN len;
    register char *s = SvPV(sv, len);
    register char *send;
    register char *base = NULL; /* start of current field */
    register I32 skipspaces = 0; /* number of contiguous spaces seen */
    bool noblank   = FALSE; /* ~ or ~~ seen on this line */
    bool repeat    = FALSE; /* ~~ seen on this line */
    bool postspace = FALSE; /* a text field may need right padding */
    U32 *fops;
    register U32 *fpc;
    U32 *linepc = NULL;	    /* position of last FF_LINEMARK */
    register I32 arg;
    bool ischop;	    /* it's a ^ rather than a @ */
    bool unchopnum = FALSE; /* at least one @ (i.e. non-chop) num field seen */
    int maxops = 12; /* FF_LINEMARK + FF_END + 10 (\0 without preceding \n) */
    MAGIC *mg = NULL;
    SV *sv_copy;

    PERL_ARGS_ASSERT_DOPARSEFORM;

    if (len == 0)
	Perl_croak(aTHX_ "Null picture in formline");

    if (SvTYPE(sv) >= SVt_PVMG) {
	/* This might, of course, still return NULL.  */
	mg = mg_find(sv, PERL_MAGIC_fm);
    } else {
	sv_upgrade(sv, SVt_PVMG);
    }

    if (mg) {
	/* still the same as previously-compiled string? */
	SV *old = mg->mg_obj;
	if ( !(!!SvUTF8(old) ^ !!SvUTF8(sv))
	      && len == SvCUR(old)
	      && strnEQ(SvPVX(old), SvPVX(sv), len)
	) {
	    DEBUG_f(PerlIO_printf(Perl_debug_log,"Re-using compiled format\n"));
	    return mg;
	}

	DEBUG_f(PerlIO_printf(Perl_debug_log, "Re-compiling format\n"));
	Safefree(mg->mg_ptr);
	mg->mg_ptr = NULL;
	SvREFCNT_dec(old);
	mg->mg_obj = NULL;
    }
    else {
	DEBUG_f(PerlIO_printf(Perl_debug_log, "Compiling format\n"));
	mg = sv_magicext(sv, NULL, PERL_MAGIC_fm, &PL_vtbl_fm, NULL, 0);
    }

    sv_copy = newSVpvn_utf8(s, len, SvUTF8(sv));
    s = SvPV(sv_copy, len); /* work on the copy, not the original */
    send = s + len;


    /* estimate the buffer size needed */
    for (base = s; s <= send; s++) {
	if (*s == '\n' || *s == '@' || *s == '^')
	    maxops += 10;
    }
    s = base;
    base = NULL;

    Newx(fops, maxops, U32);
    fpc = fops;

    if (s < send) {
	linepc = fpc;
	*fpc++ = FF_LINEMARK;
	noblank = repeat = FALSE;
	base = s;
    }

    while (s <= send) {
	switch (*s++) {
	default:
	    skipspaces = 0;
	    continue;

	case '~':
	    if (*s == '~') {
		repeat = TRUE;
		skipspaces++;
		s++;
	    }
	    noblank = TRUE;
	    /* FALL THROUGH */
	case ' ': case '\t':
	    skipspaces++;
	    continue;
        case 0:
	    if (s < send) {
	        skipspaces = 0;
                continue;
            } /* else FALL THROUGH */
	case '\n':
	    arg = s - base;
	    skipspaces++;
	    arg -= skipspaces;
	    if (arg) {
		if (postspace)
		    *fpc++ = FF_SPACE;
		*fpc++ = FF_LITERAL;
		*fpc++ = (U32)arg;
	    }
	    postspace = FALSE;
	    if (s <= send)
		skipspaces--;
	    if (skipspaces) {
		*fpc++ = FF_SKIP;
		*fpc++ = (U32)skipspaces;
	    }
	    skipspaces = 0;
	    if (s <= send)
		*fpc++ = FF_NEWLINE;
	    if (noblank) {
		*fpc++ = FF_BLANK;
		if (repeat)
		    arg = fpc - linepc + 1;
		else
		    arg = 0;
		*fpc++ = (U32)arg;
	    }
	    if (s < send) {
		linepc = fpc;
		*fpc++ = FF_LINEMARK;
		noblank = repeat = FALSE;
		base = s;
	    }
	    else
		s++;
	    continue;

	case '@':
	case '^':
	    ischop = s[-1] == '^';

	    if (postspace) {
		*fpc++ = FF_SPACE;
		postspace = FALSE;
	    }
	    arg = (s - base) - 1;
	    if (arg) {
		*fpc++ = FF_LITERAL;
		*fpc++ = (U32)arg;
	    }

	    base = s - 1;
	    *fpc++ = FF_FETCH;
	    if (*s == '*') { /*  @* or ^*  */
		s++;
		*fpc++ = 2;  /* skip the @* or ^* */
		if (ischop) {
		    *fpc++ = FF_LINESNGL;
		    *fpc++ = FF_CHOP;
		} else
		    *fpc++ = FF_LINEGLOB;
	    }
	    else if (*s == '#' || (*s == '.' && s[1] == '#')) { /* @###, ^### */
		arg = ischop ? FORM_NUM_BLANK : 0;
		base = s - 1;
		while (*s == '#')
		    s++;
		if (*s == '.') {
                    const char * const f = ++s;
		    while (*s == '#')
			s++;
		    arg |= FORM_NUM_POINT + (s - f);
		}
		*fpc++ = s - base;		/* fieldsize for FETCH */
		*fpc++ = FF_DECIMAL;
                *fpc++ = (U32)arg;
                unchopnum |= ! ischop;
            }
            else if (*s == '0' && s[1] == '#') {  /* Zero padded decimals */
                arg = ischop ? FORM_NUM_BLANK : 0;
		base = s - 1;
                s++;                                /* skip the '0' first */
                while (*s == '#')
                    s++;
                if (*s == '.') {
                    const char * const f = ++s;
                    while (*s == '#')
                        s++;
                    arg |= FORM_NUM_POINT + (s - f);
                }
                *fpc++ = s - base;                /* fieldsize for FETCH */
                *fpc++ = FF_0DECIMAL;
		*fpc++ = (U32)arg;
                unchopnum |= ! ischop;
	    }
	    else {				/* text field */
		I32 prespace = 0;
		bool ismore = FALSE;

		if (*s == '>') {
		    while (*++s == '>') ;
		    prespace = FF_SPACE;
		}
		else if (*s == '|') {
		    while (*++s == '|') ;
		    prespace = FF_HALFSPACE;
		    postspace = TRUE;
		}
		else {
		    if (*s == '<')
			while (*++s == '<') ;
		    postspace = TRUE;
		}
		if (*s == '.' && s[1] == '.' && s[2] == '.') {
		    s += 3;
		    ismore = TRUE;
		}
		*fpc++ = s - base;		/* fieldsize for FETCH */

		*fpc++ = ischop ? FF_CHECKCHOP : FF_CHECKNL;

		if (prespace)
		    *fpc++ = (U32)prespace; /* add SPACE or HALFSPACE */
		*fpc++ = FF_ITEM;
		if (ismore)
		    *fpc++ = FF_MORE;
		if (ischop)
		    *fpc++ = FF_CHOP;
	    }
	    base = s;
	    skipspaces = 0;
	    continue;
	}
    }
    *fpc++ = FF_END;

    assert (fpc <= fops + maxops); /* ensure our buffer estimate was valid */
    arg = fpc - fops;

    mg->mg_ptr = (char *) fops;
    mg->mg_len = arg * sizeof(U32);
    mg->mg_obj = sv_copy;
    mg->mg_flags |= MGf_REFCOUNTED;

    if (unchopnum && repeat)
        Perl_die(aTHX_ "Repeated format line will never terminate (~~ and @#)");

    return mg;
}


STATIC bool
S_num_overflow(NV value, I32 fldsize, I32 frcsize)
{
    /* Can value be printed in fldsize chars, using %*.*f ? */
    NV pwr = 1;
    NV eps = 0.5;
    bool res = FALSE;
    int intsize = fldsize - (value < 0 ? 1 : 0);

    if (frcsize & FORM_NUM_POINT)
        intsize--;
    frcsize &= ~(FORM_NUM_POINT|FORM_NUM_BLANK);
    intsize -= frcsize;

    while (intsize--) pwr *= 10.0;
    while (frcsize--) eps /= 10.0;

    if( value >= 0 ){
        if (value + eps >= pwr)
	    res = TRUE;
    } else {
        if (value - eps <= -pwr)
	    res = TRUE;
    }
    return res;
}

static I32
S_run_user_filter(pTHX_ int idx, SV *buf_sv, int maxlen)
{
    dVAR;
    SV * const datasv = FILTER_DATA(idx);
    const int filter_has_file = IoLINES(datasv);
    SV * const filter_state = MUTABLE_SV(IoTOP_GV(datasv));
    SV * const filter_sub = MUTABLE_SV(IoBOTTOM_GV(datasv));
    int status = 0;
    SV *upstream;
    STRLEN got_len;
    char *got_p = NULL;
    char *prune_from = NULL;
    bool read_from_cache = FALSE;
    STRLEN umaxlen;

    PERL_ARGS_ASSERT_RUN_USER_FILTER;

    assert(maxlen >= 0);
    umaxlen = maxlen;

    /* I was having segfault trouble under Linux 2.2.5 after a
       parse error occured.  (Had to hack around it with a test
       for PL_parser->error_count == 0.)  Solaris doesn't segfault --
       not sure where the trouble is yet.  XXX */

    {
	SV *const cache = datasv;
	if (SvOK(cache)) {
	    STRLEN cache_len;
	    const char *cache_p = SvPV(cache, cache_len);
	    STRLEN take = 0;

	    if (umaxlen) {
		/* Running in block mode and we have some cached data already.
		 */
		if (cache_len >= umaxlen) {
		    /* In fact, so much data we don't even need to call
		       filter_read.  */
		    take = umaxlen;
		}
	    } else {
		const char *const first_nl =
		    (const char *)memchr(cache_p, '\n', cache_len);
		if (first_nl) {
		    take = first_nl + 1 - cache_p;
		}
	    }
	    if (take) {
		sv_catpvn(buf_sv, cache_p, take);
		sv_chop(cache, cache_p + take);
		/* Definitely not EOF  */
		return 1;
	    }

	    sv_catsv(buf_sv, cache);
	    if (umaxlen) {
		umaxlen -= cache_len;
	    }
	    SvOK_off(cache);
	    read_from_cache = TRUE;
	}
    }

    /* Filter API says that the filter appends to the contents of the buffer.
       Usually the buffer is "", so the details don't matter. But if it's not,
       then clearly what it contains is already filtered by this filter, so we
       don't want to pass it in a second time.
       I'm going to use a mortal in case the upstream filter croaks.  */
    upstream = ((SvOK(buf_sv) && sv_len(buf_sv)) || SvGMAGICAL(buf_sv))
	? sv_newmortal() : buf_sv;
    SvUPGRADE(upstream, SVt_PV);
	
    if (filter_has_file) {
	status = FILTER_READ(idx+1, upstream, 0);
    }

    if (filter_sub && status >= 0) {
	dSP;
	int count;

	ENTER_with_name("call_filter_sub");
	SAVE_DEFSV;
	SAVETMPS;
	EXTEND(SP, 2);

	DEFSV_set(upstream);
	PUSHMARK(SP);
	mPUSHi(0);
	if (filter_state) {
	    PUSHs(filter_state);
	}
	PUTBACK;
	count = call_sv(filter_sub, G_SCALAR);
	SPAGAIN;

	if (count > 0) {
	    SV *out = POPs;
	    if (SvOK(out)) {
		status = SvIV(out);
	    }
	}

	PUTBACK;
	FREETMPS;
	LEAVE_with_name("call_filter_sub");
    }

    if(SvOK(upstream)) {
	got_p = SvPV(upstream, got_len);
	if (umaxlen) {
	    if (got_len > umaxlen) {
		prune_from = got_p + umaxlen;
	    }
	} else {
	    char *const first_nl = (char *)memchr(got_p, '\n', got_len);
	    if (first_nl && first_nl + 1 < got_p + got_len) {
		/* There's a second line here... */
		prune_from = first_nl + 1;
	    }
	}
    }
    if (prune_from) {
	/* Oh. Too long. Stuff some in our cache.  */
	STRLEN cached_len = got_p + got_len - prune_from;
	SV *const cache = datasv;

	if (SvOK(cache)) {
	    /* Cache should be empty.  */
	    assert(!SvCUR(cache));
	}

	sv_setpvn(cache, prune_from, cached_len);
	/* If you ask for block mode, you may well split UTF-8 characters.
	   "If it breaks, you get to keep both parts"
	   (Your code is broken if you  don't put them back together again
	   before something notices.) */
	if (SvUTF8(upstream)) {
	    SvUTF8_on(cache);
	}
	SvCUR_set(upstream, got_len - cached_len);
	*prune_from = 0;
	/* Can't yet be EOF  */
	if (status == 0)
	    status = 1;
    }

    /* If they are at EOF but buf_sv has something in it, then they may never
       have touched the SV upstream, so it may be undefined.  If we naively
       concatenate it then we get a warning about use of uninitialised value.
    */
    if (upstream != buf_sv && (SvOK(upstream) || SvGMAGICAL(upstream))) {
	sv_catsv(buf_sv, upstream);
    }

    if (status <= 0) {
	IoLINES(datasv) = 0;
	if (filter_state) {
	    SvREFCNT_dec(filter_state);
	    IoTOP_GV(datasv) = NULL;
	}
	if (filter_sub) {
	    SvREFCNT_dec(filter_sub);
	    IoBOTTOM_GV(datasv) = NULL;
	}
	filter_del(S_run_user_filter);
    }
    if (status == 0 && read_from_cache) {
	/* If we read some data from the cache (and by getting here it implies
	   that we emptied the cache) then we aren't yet at EOF, and mustn't
	   report that to our caller.  */
	return 1;
    }
    return status;
}

/* perhaps someone can come up with a better name for
   this?  it is not really "absolute", per se ... */
static bool
S_path_is_absolute(const char *name)
{
    PERL_ARGS_ASSERT_PATH_IS_ABSOLUTE;

    if (PERL_FILE_IS_ABSOLUTE(name)
#ifdef WIN32
	|| (*name == '.' && ((name[1] == '/' ||
			     (name[1] == '.' && name[2] == '/'))
			 || (name[1] == '\\' ||
			     ( name[1] == '.' && name[2] == '\\')))
	    )
#else
	|| (*name == '.' && (name[1] == '/' ||
			     (name[1] == '.' && name[2] == '/')))
#endif
	 )
    {
	return TRUE;
    }
    else
    	return FALSE;
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
