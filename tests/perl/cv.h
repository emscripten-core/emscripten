/*    cv.h
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1999, 2000, 2001,
 *    2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/* This structure must match the beginning of XPVFM in sv.h  */

struct xpvcv {
    _XPV_HEAD;
    _XPVCV_COMMON;
    I32	xcv_depth;	/* >= 2 indicates recursive call */
};

/*
=head1 Handy Values

=for apidoc AmU||Nullcv
Null CV pointer.

(deprecated - use C<(CV *)NULL> instead)

=head1 CV Manipulation Functions

This section documents functions to manipulate CVs which are code-values,
or subroutines. For more information, see L<perlguts>.

=for apidoc Am|HV*|CvSTASH|CV* cv
Returns the stash of the CV. A stash is the symbol table hash, containing
the package-scoped variables in the package where the subroutine was defined.
For more information, see L<perlguts>.

This also has a special use with XS AUTOLOAD subs.
See L<perlguts/Autoloading with XSUBs>.

=cut
*/

#ifndef PERL_CORE
#  define Nullcv Null(CV*)
#endif

#define CvSTASH(sv)	(0+((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_stash)
#define CvSTASH_set(cv,st) Perl_cvstash_set(aTHX_ cv, st)
#define CvSTART(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_start_u.xcv_start
#define CvROOT(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_root_u.xcv_root
#define CvXSUB(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_root_u.xcv_xsub
#define CvXSUBANY(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_start_u.xcv_xsubany
#define CvGV(sv)	(0+((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_gv)
#define CvGV_set(cv,gv)	Perl_cvgv_set(aTHX_ cv, gv)
#define CvFILE(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_file
#ifdef USE_ITHREADS
#  define CvFILE_set_from_cop(sv, cop)	\
    (CvFILE(sv) = savepv(CopFILE(cop)), CvDYNFILE_on(sv))
#else
#  define CvFILE_set_from_cop(sv, cop)	\
    (CvFILE(sv) = CopFILE(cop), CvDYNFILE_off(sv))
#endif
#define CvFILEGV(sv)	(gv_fetchfile(CvFILE(sv)))
#if defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
#  define CvDEPTH(sv) (*({const CV *const _cvdepth = (const CV *)sv; \
			  assert(SvTYPE(_cvdepth) == SVt_PVCV);	 \
			  &((XPVCV*)SvANY(_cvdepth))->xcv_depth; \
			}))
#else
#  define CvDEPTH(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_depth
#endif
#define CvPADLIST(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_padlist
#define CvOUTSIDE(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_outside
#define CvFLAGS(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_flags
#define CvOUTSIDE_SEQ(sv) ((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_outside_seq

/* These two are sometimes called on non-CVs */
#define CvPROTO(sv)                               \
	(                                          \
	 SvPOK(sv)                                  \
	  ? SvTYPE(sv) == SVt_PVCV && CvAUTOLOAD(sv) \
	     ? SvEND(sv)+1 : SvPVX_const(sv)          \
	  : NULL                                       \
	)
#define CvPROTOLEN(sv)	                          \
	(                                          \
	 SvPOK(sv)                                  \
	  ? SvTYPE(sv) == SVt_PVCV && CvAUTOLOAD(sv) \
	     ? SvLEN(sv)-SvCUR(sv)-2                  \
	     : SvCUR(sv)                               \
	  : 0                                           \
	)

#define CVf_METHOD	0x0001	/* CV is explicitly marked as a method */
#define CVf_LVALUE	0x0002  /* CV return value can be used as lvalue */
#define CVf_CONST	0x0004  /* inlinable sub */
#define CVf_ISXSUB	0x0008	/* CV is an XSUB, not pure perl.  */

#define CVf_WEAKOUTSIDE	0x0010  /* CvOUTSIDE isn't ref counted */
#define CVf_CLONE	0x0020	/* anon CV uses external lexicals */
#define CVf_CLONED	0x0040	/* a clone of one of those */
#define CVf_ANON	0x0080	/* CV is not pointed to by a GV */
#define CVf_UNIQUE	0x0100	/* sub is only called once (eg PL_main_cv,
				 * require, eval). */
#define CVf_NODEBUG	0x0200	/* no DB::sub indirection for this CV
				   (esp. useful for special XSUBs) */
#define CVf_CVGV_RC	0x0400	/* CvGV is reference counted */
#define CVf_DYNFILE	0x1000	/* The filename isn't static  */
#define CVf_AUTOLOAD	0x2000	/* SvPVX contains AUTOLOADed sub name  */

/* This symbol for optimised communication between toke.c and op.c: */
#define CVf_BUILTIN_ATTRS	(CVf_METHOD|CVf_LVALUE)

#define CvCLONE(cv)		(CvFLAGS(cv) & CVf_CLONE)
#define CvCLONE_on(cv)		(CvFLAGS(cv) |= CVf_CLONE)
#define CvCLONE_off(cv)		(CvFLAGS(cv) &= ~CVf_CLONE)

#define CvCLONED(cv)		(CvFLAGS(cv) & CVf_CLONED)
#define CvCLONED_on(cv)		(CvFLAGS(cv) |= CVf_CLONED)
#define CvCLONED_off(cv)	(CvFLAGS(cv) &= ~CVf_CLONED)

#define CvANON(cv)		(CvFLAGS(cv) & CVf_ANON)
#define CvANON_on(cv)		(CvFLAGS(cv) |= CVf_ANON)
#define CvANON_off(cv)		(CvFLAGS(cv) &= ~CVf_ANON)

#define CvUNIQUE(cv)		(CvFLAGS(cv) & CVf_UNIQUE)
#define CvUNIQUE_on(cv)		(CvFLAGS(cv) |= CVf_UNIQUE)
#define CvUNIQUE_off(cv)	(CvFLAGS(cv) &= ~CVf_UNIQUE)

#define CvNODEBUG(cv)		(CvFLAGS(cv) & CVf_NODEBUG)
#define CvNODEBUG_on(cv)	(CvFLAGS(cv) |= CVf_NODEBUG)
#define CvNODEBUG_off(cv)	(CvFLAGS(cv) &= ~CVf_NODEBUG)

#define CvMETHOD(cv)		(CvFLAGS(cv) & CVf_METHOD)
#define CvMETHOD_on(cv)		(CvFLAGS(cv) |= CVf_METHOD)
#define CvMETHOD_off(cv)	(CvFLAGS(cv) &= ~CVf_METHOD)

#define CvLVALUE(cv)		(CvFLAGS(cv) & CVf_LVALUE)
#define CvLVALUE_on(cv)		(CvFLAGS(cv) |= CVf_LVALUE)
#define CvLVALUE_off(cv)	(CvFLAGS(cv) &= ~CVf_LVALUE)

#define CvEVAL(cv)		(CvUNIQUE(cv) && !SvFAKE(cv))
#define CvEVAL_on(cv)		(CvUNIQUE_on(cv),SvFAKE_off(cv))
#define CvEVAL_off(cv)		CvUNIQUE_off(cv)

/* BEGIN|CHECK|INIT|UNITCHECK|END */
#define CvSPECIAL(cv)		(CvUNIQUE(cv) && SvFAKE(cv))
#define CvSPECIAL_on(cv)	(CvUNIQUE_on(cv),SvFAKE_on(cv))
#define CvSPECIAL_off(cv)	(CvUNIQUE_off(cv),SvFAKE_off(cv))

#define CvCONST(cv)		(CvFLAGS(cv) & CVf_CONST)
#define CvCONST_on(cv)		(CvFLAGS(cv) |= CVf_CONST)
#define CvCONST_off(cv)		(CvFLAGS(cv) &= ~CVf_CONST)

#define CvWEAKOUTSIDE(cv)	(CvFLAGS(cv) & CVf_WEAKOUTSIDE)
#define CvWEAKOUTSIDE_on(cv)	(CvFLAGS(cv) |= CVf_WEAKOUTSIDE)
#define CvWEAKOUTSIDE_off(cv)	(CvFLAGS(cv) &= ~CVf_WEAKOUTSIDE)

#define CvISXSUB(cv)		(CvFLAGS(cv) & CVf_ISXSUB)
#define CvISXSUB_on(cv)		(CvFLAGS(cv) |= CVf_ISXSUB)
#define CvISXSUB_off(cv)	(CvFLAGS(cv) &= ~CVf_ISXSUB)

#define CvCVGV_RC(cv)		(CvFLAGS(cv) & CVf_CVGV_RC)
#define CvCVGV_RC_on(cv)	(CvFLAGS(cv) |= CVf_CVGV_RC)
#define CvCVGV_RC_off(cv)	(CvFLAGS(cv) &= ~CVf_CVGV_RC)

#define CvDYNFILE(cv)		(CvFLAGS(cv) & CVf_DYNFILE)
#define CvDYNFILE_on(cv)	(CvFLAGS(cv) |= CVf_DYNFILE)
#define CvDYNFILE_off(cv)	(CvFLAGS(cv) &= ~CVf_DYNFILE)

#define CvAUTOLOAD(cv)		(CvFLAGS(cv) & CVf_AUTOLOAD)
#define CvAUTOLOAD_on(cv)	(CvFLAGS(cv) |= CVf_AUTOLOAD)
#define CvAUTOLOAD_off(cv)	(CvFLAGS(cv) &= ~CVf_AUTOLOAD)

/* Flags for newXS_flags  */
#define XS_DYNAMIC_FILENAME	0x01	/* The filename isn't static  */

/*
=head1 CV reference counts and CvOUTSIDE

=for apidoc m|bool|CvWEAKOUTSIDE|CV *cv

Each CV has a pointer, C<CvOUTSIDE()>, to its lexically enclosing
CV (if any). Because pointers to anonymous sub prototypes are
stored in C<&> pad slots, it is a possible to get a circular reference,
with the parent pointing to the child and vice-versa. To avoid the
ensuing memory leak, we do not increment the reference count of the CV
pointed to by C<CvOUTSIDE> in the I<one specific instance> that the parent
has a C<&> pad slot pointing back to us. In this case, we set the
C<CvWEAKOUTSIDE> flag in the child. This allows us to determine under what
circumstances we should decrement the refcount of the parent when freeing
the child.

There is a further complication with non-closure anonymous subs (i.e. those
that do not refer to any lexicals outside that sub). In this case, the
anonymous prototype is shared rather than being cloned. This has the
consequence that the parent may be freed while there are still active
children, eg

    BEGIN { $a = sub { eval '$x' } }

In this case, the BEGIN is freed immediately after execution since there
are no active references to it: the anon sub prototype has
C<CvWEAKOUTSIDE> set since it's not a closure, and $a points to the same
CV, so it doesn't contribute to BEGIN's refcount either.  When $a is
executed, the C<eval '$x'> causes the chain of C<CvOUTSIDE>s to be followed,
and the freed BEGIN is accessed.

To avoid this, whenever a CV and its associated pad is freed, any
C<&> entries in the pad are explicitly removed from the pad, and if the
refcount of the pointed-to anon sub is still positive, then that
child's C<CvOUTSIDE> is set to point to its grandparent. This will only
occur in the single specific case of a non-closure anon prototype
having one or more active references (such as C<$a> above).

One other thing to consider is that a CV may be merely undefined
rather than freed, eg C<undef &foo>. In this case, its refcount may
not have reached zero, but we still delete its pad and its C<CvROOT> etc.
Since various children may still have their C<CvOUTSIDE> pointing at this
undefined CV, we keep its own C<CvOUTSIDE> for the time being, so that
the chain of lexical scopes is unbroken. For example, the following
should print 123:

    my $x = 123;
    sub tmp { sub { eval '$x' } }
    my $a = tmp();
    undef &tmp;
    print  $a->();

=cut
*/

typedef OP *(*Perl_call_checker)(pTHX_ OP *, GV *, SV *);

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
