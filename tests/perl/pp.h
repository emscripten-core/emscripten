/*    pp.h
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1998, 1999, 2000, 2001,
 *    2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#define PP(s) OP * Perl_##s(pTHX)

/*
=head1 Stack Manipulation Macros

=for apidoc AmU||SP
Stack pointer.  This is usually handled by C<xsubpp>.  See C<dSP> and
C<SPAGAIN>.

=for apidoc AmU||MARK
Stack marker variable for the XSUB.  See C<dMARK>.

=for apidoc Am|void|PUSHMARK|SP
Opening bracket for arguments on a callback.  See C<PUTBACK> and
L<perlcall>.

=for apidoc Ams||dSP
Declares a local copy of perl's stack pointer for the XSUB, available via
the C<SP> macro.  See C<SP>.

=for apidoc ms||djSP

Declare Just C<SP>. This is actually identical to C<dSP>, and declares
a local copy of perl's stack pointer, available via the C<SP> macro.
See C<SP>.  (Available for backward source code compatibility with the
old (Perl 5.005) thread model.)

=for apidoc Ams||dMARK
Declare a stack marker variable, C<mark>, for the XSUB.  See C<MARK> and
C<dORIGMARK>.

=for apidoc Ams||dORIGMARK
Saves the original stack mark for the XSUB.  See C<ORIGMARK>.

=for apidoc AmU||ORIGMARK
The original stack mark for the XSUB.  See C<dORIGMARK>.

=for apidoc Ams||SPAGAIN
Refetch the stack pointer.  Used after a callback.  See L<perlcall>.

=cut */

#undef SP /* Solaris 2.7 i386 has this in /usr/include/sys/reg.h */
#define SP sp
#define MARK mark
#define TARG targ

#define PUSHMARK(p)	\
	STMT_START {					\
	    if (++PL_markstack_ptr == PL_markstack_max)	\
	    markstack_grow();				\
	    *PL_markstack_ptr = (I32)((p) - PL_stack_base);\
	} STMT_END

#define TOPMARK		(*PL_markstack_ptr)
#define POPMARK		(*PL_markstack_ptr--)

#define dSP		SV **sp = PL_stack_sp
#define djSP		dSP
#define dMARK		register SV **mark = PL_stack_base + POPMARK
#define dORIGMARK	const I32 origmark = (I32)(mark - PL_stack_base)
#define ORIGMARK	(PL_stack_base + origmark)

#define SPAGAIN		sp = PL_stack_sp
#define MSPAGAIN	STMT_START { sp = PL_stack_sp; mark = ORIGMARK; } STMT_END

#define GETTARGETSTACKED targ = (PL_op->op_flags & OPf_STACKED ? POPs : PAD_SV(PL_op->op_targ))
#define dTARGETSTACKED SV * GETTARGETSTACKED

#define GETTARGET targ = PAD_SV(PL_op->op_targ)
#define dTARGET SV * GETTARGET

#define GETATARGET targ = (PL_op->op_flags & OPf_STACKED ? sp[-1] : PAD_SV(PL_op->op_targ))
#define dATARGET SV * GETATARGET

#define dTARG SV *targ

#define NORMAL PL_op->op_next
#define DIE return Perl_die

/*
=for apidoc Ams||PUTBACK
Closing bracket for XSUB arguments.  This is usually handled by C<xsubpp>.
See C<PUSHMARK> and L<perlcall> for other uses.

=for apidoc Amn|SV*|POPs
Pops an SV off the stack.

=for apidoc Amn|char*|POPp
Pops a string off the stack. Deprecated. New code should use POPpx.

=for apidoc Amn|char*|POPpx
Pops a string off the stack.

=for apidoc Amn|char*|POPpbytex
Pops a string off the stack which must consist of bytes i.e. characters < 256.

=for apidoc Amn|NV|POPn
Pops a double off the stack.

=for apidoc Amn|IV|POPi
Pops an integer off the stack.

=for apidoc Amn|long|POPl
Pops a long off the stack.

=cut
*/

#define PUTBACK		PL_stack_sp = sp
#define RETURN		return (PUTBACK, NORMAL)
#define RETURNOP(o)	return (PUTBACK, o)
#define RETURNX(x)	return (x, PUTBACK, NORMAL)

#define POPs		(*sp--)
#define POPp		(SvPVx(POPs, PL_na))		/* deprecated */
#define POPpx		(SvPVx_nolen(POPs))
#define POPpconstx	(SvPVx_nolen_const(POPs))
#define POPpbytex	(SvPVbytex_nolen(POPs))
#define POPn		(SvNVx(POPs))
#define POPi		((IV)SvIVx(POPs))
#define POPu		((UV)SvUVx(POPs))
#define POPl		((long)SvIVx(POPs))
#define POPul		((unsigned long)SvIVx(POPs))
#ifdef HAS_QUAD
#define POPq		((Quad_t)SvIVx(POPs))
#define POPuq		((Uquad_t)SvUVx(POPs))
#endif

#define TOPs		(*sp)
#define TOPm1s		(*(sp-1))
#define TOPp1s		(*(sp+1))
#define TOPp		(SvPV(TOPs, PL_na))		/* deprecated */
#define TOPpx		(SvPV_nolen(TOPs))
#define TOPn		(SvNV(TOPs))
#define TOPi		((IV)SvIV(TOPs))
#define TOPu		((UV)SvUV(TOPs))
#define TOPl		((long)SvIV(TOPs))
#define TOPul		((unsigned long)SvUV(TOPs))
#ifdef HAS_QUAD
#define TOPq		((Quad_t)SvIV(TOPs))
#define TOPuq		((Uquad_t)SvUV(TOPs))
#endif

/* Go to some pains in the rare event that we must extend the stack. */

/*
=for apidoc Am|void|EXTEND|SP|int nitems
Used to extend the argument stack for an XSUB's return values. Once
used, guarantees that there is room for at least C<nitems> to be pushed
onto the stack.

=for apidoc Am|void|PUSHs|SV* sv
Push an SV onto the stack.  The stack must have room for this element.
Does not handle 'set' magic.  Does not use C<TARG>.  See also C<PUSHmortal>,
C<XPUSHs> and C<XPUSHmortal>.

=for apidoc Am|void|PUSHp|char* str|STRLEN len
Push a string onto the stack.  The stack must have room for this element.
The C<len> indicates the length of the string.  Handles 'set' magic.  Uses
C<TARG>, so C<dTARGET> or C<dXSTARG> should be called to declare it.  Do not
call multiple C<TARG>-oriented macros to return lists from XSUB's - see
C<mPUSHp> instead.  See also C<XPUSHp> and C<mXPUSHp>.

=for apidoc Am|void|PUSHn|NV nv
Push a double onto the stack.  The stack must have room for this element.
Handles 'set' magic.  Uses C<TARG>, so C<dTARGET> or C<dXSTARG> should be
called to declare it.  Do not call multiple C<TARG>-oriented macros to
return lists from XSUB's - see C<mPUSHn> instead.  See also C<XPUSHn> and
C<mXPUSHn>.

=for apidoc Am|void|PUSHi|IV iv
Push an integer onto the stack.  The stack must have room for this element.
Handles 'set' magic.  Uses C<TARG>, so C<dTARGET> or C<dXSTARG> should be
called to declare it.  Do not call multiple C<TARG>-oriented macros to 
return lists from XSUB's - see C<mPUSHi> instead.  See also C<XPUSHi> and
C<mXPUSHi>.

=for apidoc Am|void|PUSHu|UV uv
Push an unsigned integer onto the stack.  The stack must have room for this
element.  Handles 'set' magic.  Uses C<TARG>, so C<dTARGET> or C<dXSTARG>
should be called to declare it.  Do not call multiple C<TARG>-oriented
macros to return lists from XSUB's - see C<mPUSHu> instead.  See also
C<XPUSHu> and C<mXPUSHu>.

=for apidoc Am|void|XPUSHs|SV* sv
Push an SV onto the stack, extending the stack if necessary.  Does not
handle 'set' magic.  Does not use C<TARG>.  See also C<XPUSHmortal>,
C<PUSHs> and C<PUSHmortal>.

=for apidoc Am|void|XPUSHp|char* str|STRLEN len
Push a string onto the stack, extending the stack if necessary.  The C<len>
indicates the length of the string.  Handles 'set' magic.  Uses C<TARG>, so
C<dTARGET> or C<dXSTARG> should be called to declare it.  Do not call
multiple C<TARG>-oriented macros to return lists from XSUB's - see
C<mXPUSHp> instead.  See also C<PUSHp> and C<mPUSHp>.

=for apidoc Am|void|XPUSHn|NV nv
Push a double onto the stack, extending the stack if necessary.  Handles
'set' magic.  Uses C<TARG>, so C<dTARGET> or C<dXSTARG> should be called to
declare it.  Do not call multiple C<TARG>-oriented macros to return lists
from XSUB's - see C<mXPUSHn> instead.  See also C<PUSHn> and C<mPUSHn>.

=for apidoc Am|void|XPUSHi|IV iv
Push an integer onto the stack, extending the stack if necessary.  Handles
'set' magic.  Uses C<TARG>, so C<dTARGET> or C<dXSTARG> should be called to
declare it.  Do not call multiple C<TARG>-oriented macros to return lists
from XSUB's - see C<mXPUSHi> instead.  See also C<PUSHi> and C<mPUSHi>.

=for apidoc Am|void|XPUSHu|UV uv
Push an unsigned integer onto the stack, extending the stack if necessary.
Handles 'set' magic.  Uses C<TARG>, so C<dTARGET> or C<dXSTARG> should be
called to declare it.  Do not call multiple C<TARG>-oriented macros to
return lists from XSUB's - see C<mXPUSHu> instead.  See also C<PUSHu> and
C<mPUSHu>.

=for apidoc Am|void|mPUSHs|SV* sv
Push an SV onto the stack and mortalizes the SV.  The stack must have room
for this element.  Does not use C<TARG>.  See also C<PUSHs> and C<mXPUSHs>.

=for apidoc Am|void|PUSHmortal
Push a new mortal SV onto the stack.  The stack must have room for this
element.  Does not use C<TARG>.  See also C<PUSHs>, C<XPUSHmortal> and C<XPUSHs>.

=for apidoc Am|void|mPUSHp|char* str|STRLEN len
Push a string onto the stack.  The stack must have room for this element.
The C<len> indicates the length of the string.  Does not use C<TARG>.
See also C<PUSHp>, C<mXPUSHp> and C<XPUSHp>.

=for apidoc Am|void|mPUSHn|NV nv
Push a double onto the stack.  The stack must have room for this element.
Does not use C<TARG>.  See also C<PUSHn>, C<mXPUSHn> and C<XPUSHn>.

=for apidoc Am|void|mPUSHi|IV iv
Push an integer onto the stack.  The stack must have room for this element.
Does not use C<TARG>.  See also C<PUSHi>, C<mXPUSHi> and C<XPUSHi>.

=for apidoc Am|void|mPUSHu|UV uv
Push an unsigned integer onto the stack.  The stack must have room for this
element.  Does not use C<TARG>.  See also C<PUSHu>, C<mXPUSHu> and C<XPUSHu>.

=for apidoc Am|void|mXPUSHs|SV* sv
Push an SV onto the stack, extending the stack if necessary and mortalizes
the SV.  Does not use C<TARG>.  See also C<XPUSHs> and C<mPUSHs>.

=for apidoc Am|void|XPUSHmortal
Push a new mortal SV onto the stack, extending the stack if necessary.
Does not use C<TARG>.  See also C<XPUSHs>, C<PUSHmortal> and C<PUSHs>.

=for apidoc Am|void|mXPUSHp|char* str|STRLEN len
Push a string onto the stack, extending the stack if necessary.  The C<len>
indicates the length of the string.  Does not use C<TARG>.  See also C<XPUSHp>,
C<mPUSHp> and C<PUSHp>.

=for apidoc Am|void|mXPUSHn|NV nv
Push a double onto the stack, extending the stack if necessary.
Does not use C<TARG>.  See also C<XPUSHn>, C<mPUSHn> and C<PUSHn>.

=for apidoc Am|void|mXPUSHi|IV iv
Push an integer onto the stack, extending the stack if necessary.
Does not use C<TARG>.  See also C<XPUSHi>, C<mPUSHi> and C<PUSHi>.

=for apidoc Am|void|mXPUSHu|UV uv
Push an unsigned integer onto the stack, extending the stack if necessary.
Does not use C<TARG>.  See also C<XPUSHu>, C<mPUSHu> and C<PUSHu>.

=cut
*/

#define EXTEND(p,n)	STMT_START { if (PL_stack_max - p < (int)(n)) {		\
			    sp = stack_grow(sp,p, (int) (n));		\
			} } STMT_END

/* Same thing, but update mark register too. */
#define MEXTEND(p,n)	STMT_START {if (PL_stack_max - p < (int)(n)) {	\
			    const int markoff = mark - PL_stack_base;	\
			    sp = stack_grow(sp,p,(int) (n));		\
			    mark = PL_stack_base + markoff;		\
			} } STMT_END

#define PUSHs(s)	(*++sp = (s))
#define PUSHTARG	STMT_START { SvSETMAGIC(TARG); PUSHs(TARG); } STMT_END
#define PUSHp(p,l)	STMT_START { sv_setpvn(TARG, (p), (l)); PUSHTARG; } STMT_END
#define PUSHn(n)	STMT_START { sv_setnv(TARG, (NV)(n)); PUSHTARG; } STMT_END
#define PUSHi(i)	STMT_START { sv_setiv(TARG, (IV)(i)); PUSHTARG; } STMT_END
#define PUSHu(u)	STMT_START { sv_setuv(TARG, (UV)(u)); PUSHTARG; } STMT_END

#define XPUSHs(s)	STMT_START { EXTEND(sp,1); (*++sp = (s)); } STMT_END
#define XPUSHTARG	STMT_START { SvSETMAGIC(TARG); XPUSHs(TARG); } STMT_END
#define XPUSHp(p,l)	STMT_START { sv_setpvn(TARG, (p), (l)); XPUSHTARG; } STMT_END
#define XPUSHn(n)	STMT_START { sv_setnv(TARG, (NV)(n)); XPUSHTARG; } STMT_END
#define XPUSHi(i)	STMT_START { sv_setiv(TARG, (IV)(i)); XPUSHTARG; } STMT_END
#define XPUSHu(u)	STMT_START { sv_setuv(TARG, (UV)(u)); XPUSHTARG; } STMT_END
#define XPUSHundef	STMT_START { SvOK_off(TARG); XPUSHs(TARG); } STMT_END

#define mPUSHs(s)	PUSHs(sv_2mortal(s))
#define PUSHmortal	PUSHs(sv_newmortal())
#define mPUSHp(p,l)	PUSHs(newSVpvn_flags((p), (l), SVs_TEMP))
#define mPUSHn(n)	sv_setnv(PUSHmortal, (NV)(n))
#define mPUSHi(i)	sv_setiv(PUSHmortal, (IV)(i))
#define mPUSHu(u)	sv_setuv(PUSHmortal, (UV)(u))

#define mXPUSHs(s)	XPUSHs(sv_2mortal(s))
#define XPUSHmortal	XPUSHs(sv_newmortal())
#define mXPUSHp(p,l)	STMT_START { EXTEND(sp,1); mPUSHp((p), (l)); } STMT_END
#define mXPUSHn(n)	STMT_START { EXTEND(sp,1); sv_setnv(PUSHmortal, (NV)(n)); } STMT_END
#define mXPUSHi(i)	STMT_START { EXTEND(sp,1); sv_setiv(PUSHmortal, (IV)(i)); } STMT_END
#define mXPUSHu(u)	STMT_START { EXTEND(sp,1); sv_setuv(PUSHmortal, (UV)(u)); } STMT_END

#define SETs(s)		(*sp = s)
#define SETTARG		STMT_START { SvSETMAGIC(TARG); SETs(TARG); } STMT_END
#define SETp(p,l)	STMT_START { sv_setpvn(TARG, (p), (l)); SETTARG; } STMT_END
#define SETn(n)		STMT_START { sv_setnv(TARG, (NV)(n)); SETTARG; } STMT_END
#define SETi(i)		STMT_START { sv_setiv(TARG, (IV)(i)); SETTARG; } STMT_END
#define SETu(u)		STMT_START { sv_setuv(TARG, (UV)(u)); SETTARG; } STMT_END

#define dTOPss		SV *sv = TOPs
#define dPOPss		SV *sv = POPs
#define dTOPnv		NV value = TOPn
#define dPOPnv		NV value = POPn
#define dPOPnv_nomg	NV value = (sp--, SvNV_nomg(TOPp1s))
#define dTOPiv		IV value = TOPi
#define dPOPiv		IV value = POPi
#define dTOPuv		UV value = TOPu
#define dPOPuv		UV value = POPu
#ifdef HAS_QUAD
#define dTOPqv		Quad_t value = TOPu
#define dPOPqv		Quad_t value = POPu
#define dTOPuqv		Uquad_t value = TOPuq
#define dPOPuqv		Uquad_t value = POPuq
#endif

#define dPOPXssrl(X)	SV *right = POPs; SV *left = CAT2(X,s)
#define dPOPXnnrl(X)	NV right = POPn; NV left = CAT2(X,n)
#define dPOPXiirl(X)	IV right = POPi; IV left = CAT2(X,i)

#define USE_LEFT(sv) \
	(SvOK(sv) || !(PL_op->op_flags & OPf_STACKED))
#define dPOPXiirl_ul_nomg(X) \
    IV right = (sp--, SvIV_nomg(TOPp1s));		\
    SV *leftsv = CAT2(X,s);				\
    IV left = USE_LEFT(leftsv) ? SvIV_nomg(leftsv) : 0

#define dPOPPOPssrl	dPOPXssrl(POP)
#define dPOPPOPnnrl	dPOPXnnrl(POP)
#define dPOPPOPiirl	dPOPXiirl(POP)

#define dPOPTOPssrl	dPOPXssrl(TOP)
#define dPOPTOPnnrl	dPOPXnnrl(TOP)
#define dPOPTOPnnrl_nomg \
    NV right = SvNV_nomg(TOPs); NV left = (sp--, SvNV_nomg(TOPs))
#define dPOPTOPiirl	dPOPXiirl(TOP)
#define dPOPTOPiirl_ul_nomg dPOPXiirl_ul_nomg(TOP)
#define dPOPTOPiirl_nomg \
    IV right = SvIV_nomg(TOPs); IV left = (sp--, SvIV_nomg(TOPs))

#define RETPUSHYES	RETURNX(PUSHs(&PL_sv_yes))
#define RETPUSHNO	RETURNX(PUSHs(&PL_sv_no))
#define RETPUSHUNDEF	RETURNX(PUSHs(&PL_sv_undef))

#define RETSETYES	RETURNX(SETs(&PL_sv_yes))
#define RETSETNO	RETURNX(SETs(&PL_sv_no))
#define RETSETUNDEF	RETURNX(SETs(&PL_sv_undef))

#define ARGTARG		PL_op->op_targ

    /* See OPpTARGET_MY: */
#define MAXARG		(PL_op->op_private & 15)

#define SWITCHSTACK(f,t) \
    STMT_START {							\
	AvFILLp(f) = sp - PL_stack_base;				\
	PL_stack_base = AvARRAY(t);					\
	PL_stack_max = PL_stack_base + AvMAX(t);			\
	sp = PL_stack_sp = PL_stack_base + AvFILLp(t);			\
	PL_curstack = t;						\
    } STMT_END

#define EXTEND_MORTAL(n) \
    STMT_START {							\
	if (PL_tmps_ix + (n) >= PL_tmps_max)				\
	    tmps_grow(n);						\
    } STMT_END

#define AMGf_noright	1
#define AMGf_noleft	2
#define AMGf_assign	4
#define AMGf_unary	8
#define AMGf_numeric	0x10	/* for Perl_try_amagic_bin */
#define AMGf_set	0x20	/* for Perl_try_amagic_bin */


/* do SvGETMAGIC on the stack args before checking for overload */

#define tryAMAGICun_MG(method, flags) STMT_START { \
	if ( (SvFLAGS(TOPs) & (SVf_ROK|SVs_GMG)) \
		&& Perl_try_amagic_un(aTHX_ method, flags)) \
	    return NORMAL; \
    } STMT_END
#define tryAMAGICbin_MG(method, flags) STMT_START { \
	if ( ((SvFLAGS(TOPm1s)|SvFLAGS(TOPs)) & (SVf_ROK|SVs_GMG)) \
		&& Perl_try_amagic_bin(aTHX_ method, flags)) \
	    return NORMAL; \
    } STMT_END

#define AMG_CALLunary(sv,meth) \
    amagic_call(sv,&PL_sv_undef, meth, AMGf_noright | AMGf_unary)

/* No longer used in core. Use AMG_CALLunary instead */
#define AMG_CALLun(sv,meth) AMG_CALLunary(sv, CAT2(meth,_amg))

#define tryAMAGICunTARGET(meth, shift, jump)			\
    STMT_START {						\
	dATARGET;						\
	dSP;							\
	SV *tmpsv;						\
	SV *arg= sp[shift];					\
	if (SvAMAGIC(arg) &&					\
	    (tmpsv = amagic_call(arg, &PL_sv_undef, meth,	\
				 AMGf_noright | AMGf_unary))) {	\
	    SPAGAIN;						\
	    sp += shift;					\
	    sv_setsv(TARG, tmpsv);				\
	    if (opASSIGN)					\
		sp--;						\
	    SETTARG;						\
	    PUTBACK;						\
	    if (jump) {						\
	        OP *jump_o = NORMAL->op_next;                   \
		while (jump_o->op_type == OP_NULL)		\
		    jump_o = jump_o->op_next;			\
		assert(jump_o->op_type == OP_ENTERSUB);		\
		PL_markstack_ptr--;				\
		return jump_o->op_next;				\
	    }							\
	    return NORMAL;					\
	}							\
    } STMT_END

/* This is no longer used anywhere in the core. You might wish to consider
   calling amagic_deref_call() directly, as it has a cleaner interface.  */
#define tryAMAGICunDEREF(meth)						\
    STMT_START {							\
	sv = amagic_deref_call(*sp, CAT2(meth,_amg));			\
	SPAGAIN;							\
    } STMT_END


#define opASSIGN (PL_op->op_flags & OPf_STACKED)
#define SETsv(sv)	STMT_START {					\
		if (opASSIGN || (SvFLAGS(TARG) & SVs_PADMY))		\
		   { sv_setsv(TARG, (sv)); SETTARG; }			\
		else SETs(sv); } STMT_END

#define SETsvUN(sv)	STMT_START {					\
		if (SvFLAGS(TARG) & SVs_PADMY)		\
		   { sv_setsv(TARG, (sv)); SETTARG; }			\
		else SETs(sv); } STMT_END

/*
=for apidoc mU||LVRET
True if this op will be the return value of an lvalue subroutine

=cut */
#define LVRET ((PL_op->op_private & OPpMAYBE_LVSUB) && is_lvalue_sub())

#define SvCANEXISTDELETE(sv) \
 (!SvRMAGICAL(sv)            \
  || ((mg = mg_find((const SV *) sv, PERL_MAGIC_tied))           \
      && (stash = SvSTASH(SvRV(SvTIED_obj(MUTABLE_SV(sv), mg)))) \
      && gv_fetchmethod_autoload(stash, "EXISTS", TRUE)          \
      && gv_fetchmethod_autoload(stash, "DELETE", TRUE)          \
     )                       \
  )

#ifdef PERL_CORE

/* These are just for Perl_tied_method(), which is not part of the public API.
   Use 0x04 rather than the next available bit, to help the compiler if the
   architecture can generate more efficient instructions.  */
#  define TIED_METHOD_MORTALIZE_NOT_NEEDED	0x04
#  define TIED_METHOD_ARGUMENTS_ON_STACK	0x08
#  define TIED_METHOD_SAY			0x10

/* Used in various places that need to dereference a glob or globref */
#  define MAYBE_DEREF_GV_flags(sv,phlags)                          \
    (                                                               \
	(void)(phlags & SV_GMAGIC && (SvGETMAGIC(sv),0)),            \
	isGV_with_GP(sv)                                              \
	  ? (GV *)sv                                                   \
	  : SvROK(sv) && SvTYPE(SvRV(sv)) <= SVt_PVLV &&               \
	    (SvGETMAGIC(SvRV(sv)), isGV_with_GP(SvRV(sv)))              \
	     ? (GV *)SvRV(sv)                                            \
	     : NULL                                                       \
    )
#  define MAYBE_DEREF_GV(sv)      MAYBE_DEREF_GV_flags(sv,SV_GMAGIC)
#  define MAYBE_DEREF_GV_nomg(sv) MAYBE_DEREF_GV_flags(sv,0)

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
