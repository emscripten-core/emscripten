/*    universal.c
 *
 *    Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
 *    2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * '"The roots of those mountains must be roots indeed; there must be
 *   great secrets buried there which have not been discovered since the
 *   beginning."'                   --Gandalf, relating Gollum's history
 *
 *     [p.54 of _The Lord of the Rings_, I/ii: "The Shadow of the Past"]
 */

/* This file contains the code that implements the functions in Perl's
 * UNIVERSAL package, such as UNIVERSAL->can().
 *
 * It is also used to store XS functions that need to be present in
 * miniperl for a lack of a better place to put them. It might be
 * clever to move them to separate XS files which would then be pulled
 * in by some to-be-written build process.
 */

#include "EXTERN.h"
#define PERL_IN_UNIVERSAL_C
#include "perl.h"

#ifdef USE_PERLIO
#include "perliol.h" /* For the PERLIO_F_XXX */
#endif

/*
 * Contributed by Graham Barr  <Graham.Barr@tiuk.ti.com>
 * The main guts of traverse_isa was actually copied from gv_fetchmeth
 */

STATIC bool
S_isa_lookup(pTHX_ HV *stash, const char * const name, STRLEN len, U32 flags)
{
    dVAR;
    const struct mro_meta *const meta = HvMROMETA(stash);
    HV *isa = meta->isa;
    const HV *our_stash;

    PERL_ARGS_ASSERT_ISA_LOOKUP;

    if (!isa) {
	(void)mro_get_linear_isa(stash);
	isa = meta->isa;
    }

    if (hv_common(isa, NULL, name, len, ( flags & SVf_UTF8 ? HVhek_UTF8 : 0),
		  HV_FETCH_ISEXISTS, NULL, 0)) {
	/* Direct name lookup worked.  */
	return TRUE;
    }

    /* A stash/class can go by many names (ie. User == main::User), so 
       we use the HvENAME in the stash itself, which is canonical, falling
       back to HvNAME if necessary.  */
    our_stash = gv_stashpvn(name, len, flags);

    if (our_stash) {
	HEK *canon_name = HvENAME_HEK(our_stash);
	if (!canon_name) canon_name = HvNAME_HEK(our_stash);

	if (hv_common(isa, NULL, HEK_KEY(canon_name), HEK_LEN(canon_name),
		      HEK_FLAGS(canon_name),
		      HV_FETCH_ISEXISTS, NULL, HEK_HASH(canon_name))) {
	    return TRUE;
	}
    }

    return FALSE;
}

/*
=head1 SV Manipulation Functions

=for apidoc sv_derived_from_pvn

Returns a boolean indicating whether the SV is derived from the specified class
I<at the C level>.  To check derivation at the Perl level, call C<isa()> as a
normal Perl method.

Currently, the only significant value for C<flags> is SVf_UTF8.

=cut

=for apidoc sv_derived_from_sv

Exactly like L</sv_derived_from_pvn>, but takes the name string in the form
of an SV instead of a string/length pair.

=cut

*/

bool
Perl_sv_derived_from_sv(pTHX_ SV *sv, SV *namesv, U32 flags)
{
    char *namepv;
    STRLEN namelen;
    PERL_ARGS_ASSERT_SV_DERIVED_FROM_SV;
    namepv = SvPV(namesv, namelen);
    if (SvUTF8(namesv))
       flags |= SVf_UTF8;
    return sv_derived_from_pvn(sv, namepv, namelen, flags);
}

/*
=for apidoc sv_derived_from

Exactly like L</sv_derived_from_pv>, but doesn't take a C<flags> parameter.

=cut
*/

bool
Perl_sv_derived_from(pTHX_ SV *sv, const char *const name)
{
    PERL_ARGS_ASSERT_SV_DERIVED_FROM;
    return sv_derived_from_pvn(sv, name, strlen(name), 0);
}

/*
=for apidoc sv_derived_from_pv

Exactly like L</sv_derived_from_pvn>, but takes a nul-terminated string 
instead of a string/length pair.

=cut
*/


bool
Perl_sv_derived_from_pv(pTHX_ SV *sv, const char *const name, U32 flags)
{
    PERL_ARGS_ASSERT_SV_DERIVED_FROM_PV;
    return sv_derived_from_pvn(sv, name, strlen(name), flags);
}

bool
Perl_sv_derived_from_pvn(pTHX_ SV *sv, const char *const name, const STRLEN len, U32 flags)
{
    dVAR;
    HV *stash;

    PERL_ARGS_ASSERT_SV_DERIVED_FROM_PVN;

    SvGETMAGIC(sv);

    if (SvROK(sv)) {
	const char *type;
        sv = SvRV(sv);
        type = sv_reftype(sv,0);
	if (type && strEQ(type,name))
	    return TRUE;
	stash = SvOBJECT(sv) ? SvSTASH(sv) : NULL;
    }
    else {
        stash = gv_stashsv(sv, 0);
    }

    return stash ? isa_lookup(stash, name, len, flags) : FALSE;
}

/*
=for apidoc sv_does_sv

Returns a boolean indicating whether the SV performs a specific, named role.
The SV can be a Perl object or the name of a Perl class.

=cut
*/

#include "XSUB.h"

bool
Perl_sv_does_sv(pTHX_ SV *sv, SV *namesv, U32 flags)
{
    SV *classname;
    bool does_it;
    SV *methodname;
    dSP;

    PERL_ARGS_ASSERT_SV_DOES_SV;
    PERL_UNUSED_ARG(flags);

    ENTER;
    SAVETMPS;

    SvGETMAGIC(sv);

    if (!SvOK(sv) || !(SvROK(sv) || (SvPOK(sv) && SvCUR(sv))
	    || (SvGMAGICAL(sv) && SvPOKp(sv) && SvCUR(sv)))) {
	LEAVE;
	return FALSE;
    }

    if (sv_isobject(sv)) {
	classname = sv_ref(NULL,SvRV(sv),TRUE);
    } else {
	classname = sv;
    }

    if (sv_eq(classname, namesv)) {
	LEAVE;
	return TRUE;
    }

    PUSHMARK(SP);
    EXTEND(SP, 2);
    PUSHs(sv);
    PUSHs(namesv);
    PUTBACK;

    methodname = newSVpvs_flags("isa", SVs_TEMP);
    /* ugly hack: use the SvSCREAM flag so S_method_common
     * can figure out we're calling DOES() and not isa(),
     * and report eventual errors correctly. --rgs */
    SvSCREAM_on(methodname);
    call_sv(methodname, G_SCALAR | G_METHOD);
    SPAGAIN;

    does_it = SvTRUE( TOPs );
    FREETMPS;
    LEAVE;

    return does_it;
}

/*
=for apidoc sv_does

Like L</sv_does_pv>, but doesn't take a C<flags> parameter.

=cut
*/

bool
Perl_sv_does(pTHX_ SV *sv, const char *const name)
{
    PERL_ARGS_ASSERT_SV_DOES;
    return sv_does_sv(sv, newSVpvn_flags(name, strlen(name), SVs_TEMP), 0);
}

/*
=for apidoc sv_does_pv

Like L</sv_does_sv>, but takes a nul-terminated string instead of an SV.

=cut
*/


bool
Perl_sv_does_pv(pTHX_ SV *sv, const char *const name, U32 flags)
{
    PERL_ARGS_ASSERT_SV_DOES_PV;
    return sv_does_sv(sv, newSVpvn_flags(name, strlen(name), SVs_TEMP | flags), flags);
}

/*
=for apidoc sv_does_pvn

Like L</sv_does_sv>, but takes a string/length pair instead of an SV.

=cut
*/

bool
Perl_sv_does_pvn(pTHX_ SV *sv, const char *const name, const STRLEN len, U32 flags)
{
    PERL_ARGS_ASSERT_SV_DOES_PVN;

    return sv_does_sv(sv, newSVpvn_flags(name, len, flags | SVs_TEMP), flags);
}

/*
=for apidoc croak_xs_usage

A specialised variant of C<croak()> for emitting the usage message for xsubs

    croak_xs_usage(cv, "eee_yow");

works out the package name and subroutine name from C<cv>, and then calls
C<croak()>. Hence if C<cv> is C<&ouch::awk>, it would call C<croak> as:

    Perl_croak(aTHX_ "Usage: %"SVf"::%"SVf"(%s)", "ouch" "awk", "eee_yow");

=cut
*/

void
Perl_croak_xs_usage(pTHX_ const CV *const cv, const char *const params)
{
    const GV *const gv = CvGV(cv);

    PERL_ARGS_ASSERT_CROAK_XS_USAGE;

    if (gv) {
	const HV *const stash = GvSTASH(gv);

	if (HvNAME_get(stash))
	    Perl_croak(aTHX_ "Usage: %"HEKf"::%"HEKf"(%s)",
                                HEKfARG(HvNAME_HEK(stash)),
                                HEKfARG(GvNAME_HEK(gv)),
                                params);
	else
	    Perl_croak(aTHX_ "Usage: %"HEKf"(%s)",
                                HEKfARG(GvNAME_HEK(gv)), params);
    } else {
	/* Pants. I don't think that it should be possible to get here. */
	Perl_croak(aTHX_ "Usage: CODE(0x%"UVxf")(%s)", PTR2UV(cv), params);
    }
}

XS(XS_UNIVERSAL_isa)
{
    dVAR;
    dXSARGS;

    if (items != 2)
	croak_xs_usage(cv, "reference, kind");
    else {
	SV * const sv = ST(0);

	SvGETMAGIC(sv);

	if (!SvOK(sv) || !(SvROK(sv) || (SvPOK(sv) && SvCUR(sv))
		    || (SvGMAGICAL(sv) && SvPOKp(sv) && SvCUR(sv))))
	    XSRETURN_UNDEF;

	ST(0) = boolSV(sv_derived_from_sv(sv, ST(1), 0));
	XSRETURN(1);
    }
}

XS(XS_UNIVERSAL_can)
{
    dVAR;
    dXSARGS;
    SV   *sv;
    SV   *rv;
    HV   *pkg = NULL;

    if (items != 2)
	croak_xs_usage(cv, "object-ref, method");

    sv = ST(0);

    SvGETMAGIC(sv);

    if (!SvOK(sv) || !(SvROK(sv) || (SvPOK(sv) && SvCUR(sv))
		|| (SvGMAGICAL(sv) && SvPOKp(sv) && SvCUR(sv))))
	XSRETURN_UNDEF;

    rv = &PL_sv_undef;

    if (SvROK(sv)) {
        sv = MUTABLE_SV(SvRV(sv));
        if (SvOBJECT(sv))
            pkg = SvSTASH(sv);
    }
    else {
        pkg = gv_stashsv(sv, 0);
    }

    if (pkg) {
	GV * const gv = gv_fetchmethod_sv_flags(pkg, ST(1), 0);
        if (gv && isGV(gv))
	    rv = sv_2mortal(newRV(MUTABLE_SV(GvCV(gv))));
    }

    ST(0) = rv;
    XSRETURN(1);
}

XS(XS_UNIVERSAL_DOES)
{
    dVAR;
    dXSARGS;
    PERL_UNUSED_ARG(cv);

    if (items != 2)
	Perl_croak(aTHX_ "Usage: invocant->DOES(kind)");
    else {
	SV * const sv = ST(0);
	if (sv_does_sv( sv, ST(1), 0 ))
	    XSRETURN_YES;

	XSRETURN_NO;
    }
}

XS(XS_UNIVERSAL_VERSION)
{
    dVAR;
    dXSARGS;
    HV *pkg;
    GV **gvp;
    GV *gv;
    SV *sv;
    const char *undef;
    PERL_UNUSED_ARG(cv);

    if (SvROK(ST(0))) {
        sv = MUTABLE_SV(SvRV(ST(0)));
        if (!SvOBJECT(sv))
            Perl_croak(aTHX_ "Cannot find version of an unblessed reference");
        pkg = SvSTASH(sv);
    }
    else {
        pkg = gv_stashsv(ST(0), 0);
    }

    gvp = pkg ? (GV**)hv_fetchs(pkg, "VERSION", FALSE) : NULL;

    if (gvp && isGV(gv = *gvp) && (sv = GvSV(gv)) && SvOK(sv)) {
        SV * const nsv = sv_newmortal();
        sv_setsv(nsv, sv);
        sv = nsv;
	if ( !sv_isobject(sv) || !sv_derived_from(sv, "version"))
	    upg_version(sv, FALSE);

        undef = NULL;
    }
    else {
        sv = &PL_sv_undef;
        undef = "(undef)";
    }

    if (items > 1) {
	SV *req = ST(1);

	if (undef) {
	    if (pkg) {
		const HEK * const name = HvNAME_HEK(pkg);
		Perl_croak(aTHX_
			   "%"HEKf" does not define $%"HEKf
			   "::VERSION--version check failed",
			   HEKfARG(name), HEKfARG(name));
	    } else {
		Perl_croak(aTHX_
			     "%"SVf" defines neither package nor VERSION--version check failed",
			     SVfARG(ST(0)) );
	     }
	}

	if ( !sv_isobject(req) || !sv_derived_from(req, "version")) {
	    /* req may very well be R/O, so create a new object */
	    req = sv_2mortal( new_version(req) );
	}

	if ( vcmp( req, sv ) > 0 ) {
	    if ( hv_exists(MUTABLE_HV(SvRV(req)), "qv", 2 ) ) {
		Perl_croak(aTHX_ "%"HEKf" version %"SVf" required--"
		       "this is only version %"SVf"",
                       HEKfARG(HvNAME_HEK(pkg)),
		       SVfARG(sv_2mortal(vnormal(req))),
		       SVfARG(sv_2mortal(vnormal(sv))));
	    } else {
		Perl_croak(aTHX_ "%"HEKf" version %"SVf" required--"
		       "this is only version %"SVf,
                       HEKfARG(HvNAME_HEK(pkg)),
		       SVfARG(sv_2mortal(vstringify(req))),
		       SVfARG(sv_2mortal(vstringify(sv))));
	    }
	}

    }

    if ( SvOK(sv) && sv_derived_from(sv, "version") ) {
	ST(0) = sv_2mortal(vstringify(sv));
    } else {
	ST(0) = sv;
    }

    XSRETURN(1);
}

XS(XS_version_new)
{
    dVAR;
    dXSARGS;
    if (items > 3)
	croak_xs_usage(cv, "class, version");
    SP -= items;
    {
        SV *vs = ST(1);
	SV *rv;
        STRLEN len;
        const char *classname;
        U32 flags;
        if ( sv_isobject(ST(0)) ) { /* get the class if called as an object method */
            const HV * stash = SvSTASH(SvRV(ST(0)));
            classname = HvNAME(stash);
            len       = HvNAMELEN(stash);
            flags     = HvNAMEUTF8(stash) ? SVf_UTF8 : 0;
        }
        else {
	    classname = SvPV(ST(0), len);
            flags     = SvUTF8(ST(0));
        }

	if ( items == 1 || ! SvOK(vs) ) { /* no param or explicit undef */
	    /* create empty object */
	    vs = sv_newmortal();
	    sv_setpvs(vs, "0");
	}
	else if ( items == 3 ) {
	    vs = sv_newmortal();
	    Perl_sv_setpvf(aTHX_ vs,"v%s",SvPV_nolen_const(ST(2)));
	}

	rv = new_version(vs);
	if ( strnNE(classname,"version", len) ) /* inherited new() */
	    sv_bless(rv, gv_stashpvn(classname, len, GV_ADD | flags));

	mPUSHs(rv);
	PUTBACK;
	return;
    }
}

XS(XS_version_stringify)
{
     dVAR;
     dXSARGS;
     if (items < 1)
	 croak_xs_usage(cv, "lobj, ...");
     SP -= items;
     {
	  SV *	lobj = ST(0);

	  if (sv_isobject(lobj) && sv_derived_from(lobj, "version")) {
	       lobj = SvRV(lobj);
	  }
	  else
	       Perl_croak(aTHX_ "lobj is not of type version");

	  mPUSHs(vstringify(lobj));

	  PUTBACK;
	  return;
     }
}

XS(XS_version_numify)
{
     dVAR;
     dXSARGS;
     if (items < 1)
	 croak_xs_usage(cv, "lobj, ...");
     SP -= items;
     {
	  SV *	lobj = ST(0);

	  if (sv_isobject(lobj) && sv_derived_from(lobj, "version")) {
	       lobj = SvRV(lobj);
	  }
	  else
	       Perl_croak(aTHX_ "lobj is not of type version");

	  mPUSHs(vnumify(lobj));

	  PUTBACK;
	  return;
     }
}

XS(XS_version_normal)
{
     dVAR;
     dXSARGS;
     if (items < 1)
	 croak_xs_usage(cv, "lobj, ...");
     SP -= items;
     {
	  SV *	lobj = ST(0);

	  if (sv_isobject(lobj) && sv_derived_from(lobj, "version")) {
	       lobj = SvRV(lobj);
	  }
	  else
	       Perl_croak(aTHX_ "lobj is not of type version");

	  mPUSHs(vnormal(lobj));

	  PUTBACK;
	  return;
     }
}

XS(XS_version_vcmp)
{
     dVAR;
     dXSARGS;
     if (items < 1)
	 croak_xs_usage(cv, "lobj, ...");
     SP -= items;
     {
	  SV *	lobj = ST(0);

	  if (sv_isobject(lobj) && sv_derived_from(lobj, "version")) {
	       lobj = SvRV(lobj);
	  }
	  else
	       Perl_croak(aTHX_ "lobj is not of type version");

	  {
	       SV	*rs;
	       SV	*rvs;
	       SV * robj = ST(1);
	       const IV	 swap = (IV)SvIV(ST(2));

	       if ( !sv_isobject(robj) || !sv_derived_from(robj, "version") )
	       {
		    robj = new_version(SvOK(robj) ? robj : newSVpvs_flags("0", SVs_TEMP));
		    sv_2mortal(robj);
	       }
	       rvs = SvRV(robj);

	       if ( swap )
	       {
		    rs = newSViv(vcmp(rvs,lobj));
	       }
	       else
	       {
		    rs = newSViv(vcmp(lobj,rvs));
	       }

	       mPUSHs(rs);
	  }

	  PUTBACK;
	  return;
     }
}

XS(XS_version_boolean)
{
    dVAR;
    dXSARGS;
    if (items < 1)
	croak_xs_usage(cv, "lobj, ...");
    SP -= items;
    if (sv_isobject(ST(0)) && sv_derived_from(ST(0), "version")) {
	SV * const lobj = SvRV(ST(0));
	SV * const rs =
	    newSViv( vcmp(lobj,
			  sv_2mortal(new_version(
					sv_2mortal(newSVpvs("0"))
				    ))
			 )
		   );
	mPUSHs(rs);
	PUTBACK;
	return;
    }
    else
	Perl_croak(aTHX_ "lobj is not of type version");
}

XS(XS_version_noop)
{
    dVAR;
    dXSARGS;
    if (items < 1)
	croak_xs_usage(cv, "lobj, ...");
    if (sv_isobject(ST(0)) && sv_derived_from(ST(0), "version"))
	Perl_croak(aTHX_ "operation not supported with version object");
    else
	Perl_croak(aTHX_ "lobj is not of type version");
#ifndef HASATTRIBUTE_NORETURN
    XSRETURN_EMPTY;
#endif
}

XS(XS_version_is_alpha)
{
    dVAR;
    dXSARGS;
    if (items != 1)
	croak_xs_usage(cv, "lobj");
    SP -= items;
    if (sv_isobject(ST(0)) && sv_derived_from(ST(0), "version")) {
	SV * const lobj = ST(0);
	if ( hv_exists(MUTABLE_HV(SvRV(lobj)), "alpha", 5 ) )
	    XSRETURN_YES;
	else
	    XSRETURN_NO;
	PUTBACK;
	return;
    }
    else
	Perl_croak(aTHX_ "lobj is not of type version");
}

XS(XS_version_qv)
{
    dVAR;
    dXSARGS;
    PERL_UNUSED_ARG(cv);
    SP -= items;
    {
	SV * ver = ST(0);
	SV * rv;
        STRLEN len = 0;
        const char * classname = "";
        U32 flags = 0;
        if ( items == 2 && SvOK(ST(1)) ) {
            ver = ST(1);
            if ( sv_isobject(ST(0)) ) { /* class called as an object method */
                const HV * stash = SvSTASH(SvRV(ST(0)));
                classname = HvNAME(stash);
                len       = HvNAMELEN(stash);
                flags     = HvNAMEUTF8(stash) ? SVf_UTF8 : 0;
            }
            else {
	       classname = SvPV(ST(0), len);
                flags     = SvUTF8(ST(0));
            }
        }
	if ( !SvVOK(ver) ) { /* not already a v-string */
	    rv = sv_newmortal();
	    sv_setsv(rv,ver); /* make a duplicate */
	    upg_version(rv, TRUE);
	} else {
	    rv = sv_2mortal(new_version(ver));
	}
	if ( items == 2
                && strnNE(classname,"version", len) ) { /* inherited new() */
	    sv_bless(rv, gv_stashpvn(classname, len, GV_ADD | flags));
        }
	PUSHs(rv);
    }
    PUTBACK;
    return;
}

XS(XS_version_is_qv)
{
    dVAR;
    dXSARGS;
    if (items != 1)
	croak_xs_usage(cv, "lobj");
    SP -= items;
    if (sv_isobject(ST(0)) && sv_derived_from(ST(0), "version")) {
	SV * const lobj = ST(0);
	if ( hv_exists(MUTABLE_HV(SvRV(lobj)), "qv", 2 ) )
	    XSRETURN_YES;
	else
	    XSRETURN_NO;
	PUTBACK;
	return;
    }
    else
	Perl_croak(aTHX_ "lobj is not of type version");
}

XS(XS_utf8_is_utf8)
{
     dVAR;
     dXSARGS;
     if (items != 1)
	 croak_xs_usage(cv, "sv");
     else {
	SV * const sv = ST(0);
	SvGETMAGIC(sv);
	    if (SvUTF8(sv))
		XSRETURN_YES;
	    else
		XSRETURN_NO;
     }
     XSRETURN_EMPTY;
}

XS(XS_utf8_valid)
{
     dVAR;
     dXSARGS;
     if (items != 1)
	 croak_xs_usage(cv, "sv");
    else {
	SV * const sv = ST(0);
	STRLEN len;
	const char * const s = SvPV_const(sv,len);
	if (!SvUTF8(sv) || is_utf8_string((const U8*)s,len))
	    XSRETURN_YES;
	else
	    XSRETURN_NO;
    }
     XSRETURN_EMPTY;
}

XS(XS_utf8_encode)
{
    dVAR;
    dXSARGS;
    if (items != 1)
	croak_xs_usage(cv, "sv");
    sv_utf8_encode(ST(0));
    XSRETURN_EMPTY;
}

XS(XS_utf8_decode)
{
    dVAR;
    dXSARGS;
    if (items != 1)
	croak_xs_usage(cv, "sv");
    else {
	SV * const sv = ST(0);
	bool RETVAL;
	SvPV_force_nolen(sv);
	RETVAL = sv_utf8_decode(sv);
	ST(0) = boolSV(RETVAL);
    }
    XSRETURN(1);
}

XS(XS_utf8_upgrade)
{
    dVAR;
    dXSARGS;
    if (items != 1)
	croak_xs_usage(cv, "sv");
    else {
	SV * const sv = ST(0);
	STRLEN	RETVAL;
	dXSTARG;

	RETVAL = sv_utf8_upgrade(sv);
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}

XS(XS_utf8_downgrade)
{
    dVAR;
    dXSARGS;
    if (items < 1 || items > 2)
	croak_xs_usage(cv, "sv, failok=0");
    else {
	SV * const sv = ST(0);
        const bool failok = (items < 2) ? 0 : (int)SvIV(ST(1));
        const bool RETVAL = sv_utf8_downgrade(sv, failok);

	ST(0) = boolSV(RETVAL);
    }
    XSRETURN(1);
}

XS(XS_utf8_native_to_unicode)
{
 dVAR;
 dXSARGS;
 const UV uv = SvUV(ST(0));

 if (items > 1)
     croak_xs_usage(cv, "sv");

 ST(0) = sv_2mortal(newSViv(NATIVE_TO_UNI(uv)));
 XSRETURN(1);
}

XS(XS_utf8_unicode_to_native)
{
 dVAR;
 dXSARGS;
 const UV uv = SvUV(ST(0));

 if (items > 1)
     croak_xs_usage(cv, "sv");

 ST(0) = sv_2mortal(newSViv(UNI_TO_NATIVE(uv)));
 XSRETURN(1);
}

XS(XS_Internals_SvREADONLY)	/* This is dangerous stuff. */
{
    dVAR;
    dXSARGS;
    SV * const svz = ST(0);
    SV * sv;
    PERL_UNUSED_ARG(cv);

    /* [perl #77776] - called as &foo() not foo() */
    if (!SvROK(svz))
        croak_xs_usage(cv, "SCALAR[, ON]");

    sv = SvRV(svz);

    if (items == 1) {
	 if (SvREADONLY(sv) && !SvIsCOW(sv))
	     XSRETURN_YES;
	 else
	     XSRETURN_NO;
    }
    else if (items == 2) {
	if (SvTRUE(ST(1))) {
	    if (SvIsCOW(sv)) sv_force_normal(sv);
	    SvREADONLY_on(sv);
	    XSRETURN_YES;
	}
	else {
	    /* I hope you really know what you are doing. */
	    if (!SvIsCOW(sv)) SvREADONLY_off(sv);
	    XSRETURN_NO;
	}
    }
    XSRETURN_UNDEF; /* Can't happen. */
}

XS(XS_Internals_SvREFCNT)	/* This is dangerous stuff. */
{
    dVAR;
    dXSARGS;
    SV * const svz = ST(0);
    SV * sv;
    PERL_UNUSED_ARG(cv);

    /* [perl #77776] - called as &foo() not foo() */
    if (!SvROK(svz))
        croak_xs_usage(cv, "SCALAR[, REFCOUNT]");

    sv = SvRV(svz);

    if (items == 1)
	 XSRETURN_UV(SvREFCNT(sv) - 1); /* Minus the ref created for us. */
    else if (items == 2) {
         /* I hope you really know what you are doing. */
	 SvREFCNT(sv) = SvUV(ST(1)) + 1; /* we free one ref on exit */
	 XSRETURN_UV(SvREFCNT(sv) - 1);
    }
    XSRETURN_UNDEF; /* Can't happen. */
}

XS(XS_Internals_hv_clear_placehold)
{
    dVAR;
    dXSARGS;

    if (items != 1 || !SvROK(ST(0)))
	croak_xs_usage(cv, "hv");
    else {
	HV * const hv = MUTABLE_HV(SvRV(ST(0)));
	hv_clear_placeholders(hv);
	XSRETURN(0);
    }
}

XS(XS_PerlIO_get_layers)
{
    dVAR;
    dXSARGS;
    if (items < 1 || items % 2 == 0)
	croak_xs_usage(cv, "filehandle[,args]");
#ifdef USE_PERLIO
    {
	SV *	sv;
	GV *	gv;
	IO *	io;
	bool	input = TRUE;
	bool	details = FALSE;

	if (items > 1) {
	     SV * const *svp;
	     for (svp = MARK + 2; svp <= SP; svp += 2) {
		  SV * const * const varp = svp;
		  SV * const * const valp = svp + 1;
		  STRLEN klen;
		  const char * const key = SvPV_const(*varp, klen);

		  switch (*key) {
		  case 'i':
		       if (klen == 5 && memEQ(key, "input", 5)) {
			    input = SvTRUE(*valp);
			    break;
		       }
		       goto fail;
		  case 'o': 
		       if (klen == 6 && memEQ(key, "output", 6)) {
			    input = !SvTRUE(*valp);
			    break;
		       }
		       goto fail;
		  case 'd':
		       if (klen == 7 && memEQ(key, "details", 7)) {
			    details = SvTRUE(*valp);
			    break;
		       }
		       goto fail;
		  default:
		  fail:
		       Perl_croak(aTHX_
				  "get_layers: unknown argument '%s'",
				  key);
		  }
	     }

	     SP -= (items - 1);
	}

	sv = POPs;
	gv = MAYBE_DEREF_GV(sv);

	if (!gv && !SvROK(sv))
	    gv = gv_fetchsv_nomg(sv, 0, SVt_PVIO);

	if (gv && (io = GvIO(gv))) {
	     AV* const av = PerlIO_get_layers(aTHX_ input ?
					IoIFP(io) : IoOFP(io));
	     I32 i;
	     const I32 last = av_len(av);
	     I32 nitem = 0;
	     
	     for (i = last; i >= 0; i -= 3) {
		  SV * const * const namsvp = av_fetch(av, i - 2, FALSE);
		  SV * const * const argsvp = av_fetch(av, i - 1, FALSE);
		  SV * const * const flgsvp = av_fetch(av, i,     FALSE);

		  const bool namok = namsvp && *namsvp && SvPOK(*namsvp);
		  const bool argok = argsvp && *argsvp && SvPOK(*argsvp);
		  const bool flgok = flgsvp && *flgsvp && SvIOK(*flgsvp);

		  if (details) {
		      /* Indents of 5? Yuck.  */
		      /* We know that PerlIO_get_layers creates a new SV for
			 the name and flags, so we can just take a reference
			 and "steal" it when we free the AV below.  */
		       XPUSHs(namok
			      ? sv_2mortal(SvREFCNT_inc_simple_NN(*namsvp))
			      : &PL_sv_undef);
		       XPUSHs(argok
			      ? newSVpvn_flags(SvPVX_const(*argsvp),
					       SvCUR(*argsvp),
					       (SvUTF8(*argsvp) ? SVf_UTF8 : 0)
					       | SVs_TEMP)
			      : &PL_sv_undef);
		       XPUSHs(flgok
			      ? sv_2mortal(SvREFCNT_inc_simple_NN(*flgsvp))
			      : &PL_sv_undef);
		       nitem += 3;
		  }
		  else {
		       if (namok && argok)
			    XPUSHs(sv_2mortal(Perl_newSVpvf(aTHX_ "%"SVf"(%"SVf")",
						 SVfARG(*namsvp),
						 SVfARG(*argsvp))));
		       else if (namok)
			   XPUSHs(sv_2mortal(SvREFCNT_inc_simple_NN(*namsvp)));
		       else
			    XPUSHs(&PL_sv_undef);
		       nitem++;
		       if (flgok) {
			    const IV flags = SvIVX(*flgsvp);

			    if (flags & PERLIO_F_UTF8) {
				 XPUSHs(newSVpvs_flags("utf8", SVs_TEMP));
				 nitem++;
			    }
		       }
		  }
	     }

	     SvREFCNT_dec(av);

	     XSRETURN(nitem);
	}
    }
#endif

    XSRETURN(0);
}

XS(XS_Internals_hash_seed)
{
    dVAR;
    /* Using dXSARGS would also have dITEM and dSP,
     * which define 2 unused local variables.  */
    dAXMARK;
    PERL_UNUSED_ARG(cv);
    PERL_UNUSED_VAR(mark);
    XSRETURN_UV(PERL_HASH_SEED);
}

XS(XS_Internals_rehash_seed)
{
    dVAR;
    /* Using dXSARGS would also have dITEM and dSP,
     * which define 2 unused local variables.  */
    dAXMARK;
    PERL_UNUSED_ARG(cv);
    PERL_UNUSED_VAR(mark);
    XSRETURN_UV(PL_rehash_seed);
}

XS(XS_Internals_HvREHASH)	/* Subject to change  */
{
    dVAR;
    dXSARGS;
    PERL_UNUSED_ARG(cv);
    if (SvROK(ST(0))) {
	const HV * const hv = (const HV *) SvRV(ST(0));
	if (items == 1 && SvTYPE(hv) == SVt_PVHV) {
	    if (HvREHASH(hv))
		XSRETURN_YES;
	    else
		XSRETURN_NO;
	}
    }
    Perl_croak(aTHX_ "Internals::HvREHASH $hashref");
}

XS(XS_re_is_regexp)
{
    dVAR; 
    dXSARGS;
    PERL_UNUSED_VAR(cv);

    if (items != 1)
	croak_xs_usage(cv, "sv");

    if (SvRXOK(ST(0))) {
        XSRETURN_YES;
    } else {
        XSRETURN_NO;
    }
}

XS(XS_re_regnames_count)
{
    REGEXP *rx = PL_curpm ? PM_GETRE(PL_curpm) : NULL;
    SV * ret;
    dVAR; 
    dXSARGS;

    if (items != 0)
	croak_xs_usage(cv, "");

    SP -= items;
    PUTBACK;

    if (!rx)
        XSRETURN_UNDEF;

    ret = CALLREG_NAMED_BUFF_COUNT(rx);

    SPAGAIN;
    PUSHs(ret ? sv_2mortal(ret) : &PL_sv_undef);
    XSRETURN(1);
}

XS(XS_re_regname)
{
    dVAR;
    dXSARGS;
    REGEXP * rx;
    U32 flags;
    SV * ret;

    if (items < 1 || items > 2)
	croak_xs_usage(cv, "name[, all ]");

    SP -= items;
    PUTBACK;

    rx = PL_curpm ? PM_GETRE(PL_curpm) : NULL;

    if (!rx)
        XSRETURN_UNDEF;

    if (items == 2 && SvTRUE(ST(1))) {
        flags = RXapif_ALL;
    } else {
        flags = RXapif_ONE;
    }
    ret = CALLREG_NAMED_BUFF_FETCH(rx, ST(0), (flags | RXapif_REGNAME));

    SPAGAIN;
    PUSHs(ret ? sv_2mortal(ret) : &PL_sv_undef);
    XSRETURN(1);
}


XS(XS_re_regnames)
{
    dVAR;
    dXSARGS;
    REGEXP * rx;
    U32 flags;
    SV *ret;
    AV *av;
    I32 length;
    I32 i;
    SV **entry;

    if (items > 1)
	croak_xs_usage(cv, "[all]");

    rx = PL_curpm ? PM_GETRE(PL_curpm) : NULL;

    if (!rx)
        XSRETURN_UNDEF;

    if (items == 1 && SvTRUE(ST(0))) {
        flags = RXapif_ALL;
    } else {
        flags = RXapif_ONE;
    }

    SP -= items;
    PUTBACK;

    ret = CALLREG_NAMED_BUFF_ALL(rx, (flags | RXapif_REGNAMES));

    SPAGAIN;

    if (!ret)
        XSRETURN_UNDEF;

    av = MUTABLE_AV(SvRV(ret));
    length = av_len(av);

    for (i = 0; i <= length; i++) {
        entry = av_fetch(av, i, FALSE);
        
        if (!entry)
            Perl_croak(aTHX_ "NULL array element in re::regnames()");

        mXPUSHs(SvREFCNT_inc_simple_NN(*entry));
    }

    SvREFCNT_dec(ret);

    PUTBACK;
    return;
}

XS(XS_re_regexp_pattern)
{
    dVAR;
    dXSARGS;
    REGEXP *re;

    if (items != 1)
	croak_xs_usage(cv, "sv");

    SP -= items;

    /*
       Checks if a reference is a regex or not. If the parameter is
       not a ref, or is not the result of a qr// then returns false
       in scalar context and an empty list in list context.
       Otherwise in list context it returns the pattern and the
       modifiers, in scalar context it returns the pattern just as it
       would if the qr// was stringified normally, regardless as
       to the class of the variable and any stringification overloads
       on the object.
    */

    if ((re = SvRX(ST(0)))) /* assign deliberate */
    {
        /* Houston, we have a regex! */
        SV *pattern;

        if ( GIMME_V == G_ARRAY ) {
	    STRLEN left = 0;
	    char reflags[sizeof(INT_PAT_MODS) + MAX_CHARSET_NAME_LENGTH];
            const char *fptr;
            char ch;
            U16 match_flags;

            /*
               we are in list context so stringify
               the modifiers that apply. We ignore "negative
               modifiers" in this scenario, and the default character set
            */

	    if (get_regex_charset(RX_EXTFLAGS(re)) != REGEX_DEPENDS_CHARSET) {
		STRLEN len;
		const char* const name = get_regex_charset_name(RX_EXTFLAGS(re),
								&len);
		Copy(name, reflags + left, len, char);
		left += len;
	    }
            fptr = INT_PAT_MODS;
            match_flags = (U16)((RX_EXTFLAGS(re) & RXf_PMf_COMPILETIME)
                                    >> RXf_PMf_STD_PMMOD_SHIFT);

            while((ch = *fptr++)) {
                if(match_flags & 1) {
                    reflags[left++] = ch;
                }
                match_flags >>= 1;
            }

            pattern = newSVpvn_flags(RX_PRECOMP(re),RX_PRELEN(re),
				     (RX_UTF8(re) ? SVf_UTF8 : 0) | SVs_TEMP);

            /* return the pattern and the modifiers */
            XPUSHs(pattern);
            XPUSHs(newSVpvn_flags(reflags, left, SVs_TEMP));
            XSRETURN(2);
        } else {
            /* Scalar, so use the string that Perl would return */
            /* return the pattern in (?msix:..) format */
#if PERL_VERSION >= 11
            pattern = sv_2mortal(newSVsv(MUTABLE_SV(re)));
#else
            pattern = newSVpvn_flags(RX_WRAPPED(re), RX_WRAPLEN(re),
				     (RX_UTF8(re) ? SVf_UTF8 : 0) | SVs_TEMP);
#endif
            XPUSHs(pattern);
            XSRETURN(1);
        }
    } else {
        /* It ain't a regexp folks */
        if ( GIMME_V == G_ARRAY ) {
            /* return the empty list */
            XSRETURN_UNDEF;
        } else {
            /* Because of the (?:..) wrapping involved in a
               stringified pattern it is impossible to get a
               result for a real regexp that would evaluate to
               false. Therefore we can return PL_sv_no to signify
               that the object is not a regex, this means that one
               can say

                 if (regex($might_be_a_regex) eq '(?:foo)') { }

               and not worry about undefined values.
            */
            XSRETURN_NO;
        }
    }
    /* NOT-REACHED */
}

struct xsub_details {
    const char *name;
    XSUBADDR_t xsub;
    const char *proto;
};

struct xsub_details details[] = {
    {"UNIVERSAL::isa", XS_UNIVERSAL_isa, NULL},
    {"UNIVERSAL::can", XS_UNIVERSAL_can, NULL},
    {"UNIVERSAL::DOES", XS_UNIVERSAL_DOES, NULL},
    {"UNIVERSAL::VERSION", XS_UNIVERSAL_VERSION, NULL},
    {"version::()", XS_version_noop, NULL},
    {"version::new", XS_version_new, NULL},
    {"version::parse", XS_version_new, NULL},
    {"version::(\"\"", XS_version_stringify, NULL},
    {"version::stringify", XS_version_stringify, NULL},
    {"version::(0+", XS_version_numify, NULL},
    {"version::numify", XS_version_numify, NULL},
    {"version::normal", XS_version_normal, NULL},
    {"version::(cmp", XS_version_vcmp, NULL},
    {"version::(<=>", XS_version_vcmp, NULL},
    {"version::vcmp", XS_version_vcmp, NULL},
    {"version::(bool", XS_version_boolean, NULL},
    {"version::boolean", XS_version_boolean, NULL},
    {"version::(+", XS_version_noop, NULL},
    {"version::(-", XS_version_noop, NULL},
    {"version::(*", XS_version_noop, NULL},
    {"version::(/", XS_version_noop, NULL},
    {"version::(+=", XS_version_noop, NULL},
    {"version::(-=", XS_version_noop, NULL},
    {"version::(*=", XS_version_noop, NULL},
    {"version::(/=", XS_version_noop, NULL},
    {"version::(abs", XS_version_noop, NULL},
    {"version::(nomethod", XS_version_noop, NULL},
    {"version::noop", XS_version_noop, NULL},
    {"version::is_alpha", XS_version_is_alpha, NULL},
    {"version::qv", XS_version_qv, NULL},
    {"version::declare", XS_version_qv, NULL},
    {"version::is_qv", XS_version_is_qv, NULL},
    {"utf8::is_utf8", XS_utf8_is_utf8, NULL},
    {"utf8::valid", XS_utf8_valid, NULL},
    {"utf8::encode", XS_utf8_encode, NULL},
    {"utf8::decode", XS_utf8_decode, NULL},
    {"utf8::upgrade", XS_utf8_upgrade, NULL},
    {"utf8::downgrade", XS_utf8_downgrade, NULL},
    {"utf8::native_to_unicode", XS_utf8_native_to_unicode, NULL},
    {"utf8::unicode_to_native", XS_utf8_unicode_to_native, NULL},
    {"Internals::SvREADONLY", XS_Internals_SvREADONLY, "\\[$%@];$"},
    {"Internals::SvREFCNT", XS_Internals_SvREFCNT, "\\[$%@];$"},
    {"Internals::hv_clear_placeholders", XS_Internals_hv_clear_placehold, "\\%"},
    {"PerlIO::get_layers", XS_PerlIO_get_layers, "*;@"},
    {"Internals::hash_seed", XS_Internals_hash_seed, ""},
    {"Internals::rehash_seed", XS_Internals_rehash_seed, ""},
    {"Internals::HvREHASH", XS_Internals_HvREHASH, "\\%"},
    {"re::is_regexp", XS_re_is_regexp, "$"},
    {"re::regname", XS_re_regname, ";$$"},
    {"re::regnames", XS_re_regnames, ";$"},
    {"re::regnames_count", XS_re_regnames_count, ""},
    {"re::regexp_pattern", XS_re_regexp_pattern, "$"},
};

void
Perl_boot_core_UNIVERSAL(pTHX)
{
    dVAR;
    static const char file[] = __FILE__;
    struct xsub_details *xsub = details;
    const struct xsub_details *end
	= details + sizeof(details) / sizeof(details[0]);

    do {
	newXS_flags(xsub->name, xsub->xsub, file, xsub->proto, 0);
    } while (++xsub < end);

    /* register the overloading (type 'A') magic */
    PL_amagic_generation++;

    /* Providing a Regexp::DESTROY fixes #21347. See test in t/op/ref.t  */
    {
	CV * const cv =
	    newCONSTSUB(get_hv("Regexp::", GV_ADD), "DESTROY", NULL);
	Safefree(CvFILE(cv));
	CvFILE(cv) = (char *)file;
	CvDYNFILE_off(cv);
    }
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
