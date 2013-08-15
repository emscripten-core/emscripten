/*    mg.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *  Sam sat on the ground and put his head in his hands.  'I wish I had never
 *  come here, and I don't want to see no more magic,' he said, and fell silent.
 *
 *     [p.363 of _The Lord of the Rings_, II/vii: "The Mirror of Galadriel"]
 */

/*
=head1 Magical Functions

"Magic" is special data attached to SV structures in order to give them
"magical" properties.  When any Perl code tries to read from, or assign to,
an SV marked as magical, it calls the 'get' or 'set' function associated
with that SV's magic. A get is called prior to reading an SV, in order to
give it a chance to update its internal value (get on $. writes the line
number of the last read filehandle into to the SV's IV slot), while
set is called after an SV has been written to, in order to allow it to make
use of its changed value (set on $/ copies the SV's new value to the
PL_rs global variable).

Magic is implemented as a linked list of MAGIC structures attached to the
SV. Each MAGIC struct holds the type of the magic, a pointer to an array
of functions that implement the get(), set(), length() etc functions,
plus space for some flags and pointers. For example, a tied variable has
a MAGIC structure that contains a pointer to the object associated with the
tie.

*/

#include "EXTERN.h"
#define PERL_IN_MG_C
#include "perl.h"

#if defined(HAS_GETGROUPS) || defined(HAS_SETGROUPS)
#  ifdef I_GRP
#    include <grp.h>
#  endif
#endif

#if defined(HAS_SETGROUPS)
#  ifndef NGROUPS
#    define NGROUPS 32
#  endif
#endif

#ifdef __hpux
#  include <sys/pstat.h>
#endif

#ifdef HAS_PRCTL_SET_NAME
#  include <sys/prctl.h>
#endif

#if defined(HAS_SIGACTION) && defined(SA_SIGINFO)
Signal_t Perl_csighandler(int sig, siginfo_t *, void *);
#else
Signal_t Perl_csighandler(int sig);
#endif

#ifdef __Lynx__
/* Missing protos on LynxOS */
void setruid(uid_t id);
void seteuid(uid_t id);
void setrgid(uid_t id);
void setegid(uid_t id);
#endif

/*
 * Use the "DESTRUCTOR" scope cleanup to reinstate magic.
 */

struct magic_state {
    SV* mgs_sv;
    I32 mgs_ss_ix;
    U32 mgs_magical;
    bool mgs_readonly;
    bool mgs_bumped;
};
/* MGS is typedef'ed to struct magic_state in perl.h */

STATIC void
S_save_magic(pTHX_ I32 mgs_ix, SV *sv)
{
    dVAR;
    MGS* mgs;
    bool bumped = FALSE;

    PERL_ARGS_ASSERT_SAVE_MAGIC;

    /* we shouldn't really be called here with RC==0, but it can sometimes
     * happen via mg_clear() (which also shouldn't be called when RC==0,
     * but it can happen). Handle this case gracefully(ish) by not RC++
     * and thus avoiding the resultant double free */
    if (SvREFCNT(sv) > 0) {
    /* guard against sv getting freed midway through the mg clearing,
     * by holding a private reference for the duration. */
	SvREFCNT_inc_simple_void_NN(sv);
	bumped = TRUE;
    }

    assert(SvMAGICAL(sv));
    /* Turning READONLY off for a copy-on-write scalar (including shared
       hash keys) is a bad idea.  */
    if (SvIsCOW(sv))
      sv_force_normal_flags(sv, 0);

    SAVEDESTRUCTOR_X(S_restore_magic, INT2PTR(void*, (IV)mgs_ix));

    mgs = SSPTR(mgs_ix, MGS*);
    mgs->mgs_sv = sv;
    mgs->mgs_magical = SvMAGICAL(sv);
    mgs->mgs_readonly = SvREADONLY(sv) != 0;
    mgs->mgs_ss_ix = PL_savestack_ix;   /* points after the saved destructor */
    mgs->mgs_bumped = bumped;

    SvMAGICAL_off(sv);
    SvREADONLY_off(sv);
    if (!(SvFLAGS(sv) & (SVf_IOK|SVf_NOK|SVf_POK))) {
	/* No public flags are set, so promote any private flags to public.  */
	SvFLAGS(sv) |= (SvFLAGS(sv) & (SVp_IOK|SVp_NOK|SVp_POK)) >> PRIVSHIFT;
    }
}

/*
=for apidoc mg_magical

Turns on the magical status of an SV.  See C<sv_magic>.

=cut
*/

void
Perl_mg_magical(pTHX_ SV *sv)
{
    const MAGIC* mg;
    PERL_ARGS_ASSERT_MG_MAGICAL;
    PERL_UNUSED_CONTEXT;

    SvMAGICAL_off(sv);
    if ((mg = SvMAGIC(sv))) {
	do {
	    const MGVTBL* const vtbl = mg->mg_virtual;
	    if (vtbl) {
		if (vtbl->svt_get && !(mg->mg_flags & MGf_GSKIP))
		    SvGMAGICAL_on(sv);
		if (vtbl->svt_set)
		    SvSMAGICAL_on(sv);
		if (vtbl->svt_clear)
		    SvRMAGICAL_on(sv);
	    }
	} while ((mg = mg->mg_moremagic));
	if (!(SvFLAGS(sv) & (SVs_GMG|SVs_SMG)))
	    SvRMAGICAL_on(sv);
    }
}

/*
=for apidoc mg_get

Do magic before a value is retrieved from the SV.  See C<sv_magic>.

=cut
*/

int
Perl_mg_get(pTHX_ SV *sv)
{
    dVAR;
    const I32 mgs_ix = SSNEW(sizeof(MGS));
    bool have_new = 0;
    MAGIC *newmg, *head, *cur, *mg;

    PERL_ARGS_ASSERT_MG_GET;

    if (PL_localizing == 1 && sv == DEFSV) return 0;

    save_magic(mgs_ix, sv);

    /* We must call svt_get(sv, mg) for each valid entry in the linked
       list of magic. svt_get() may delete the current entry, add new
       magic to the head of the list, or upgrade the SV. AMS 20010810 */

    newmg = cur = head = mg = SvMAGIC(sv);
    while (mg) {
	const MGVTBL * const vtbl = mg->mg_virtual;
	MAGIC * const nextmg = mg->mg_moremagic;	/* it may delete itself */

	if (!(mg->mg_flags & MGf_GSKIP) && vtbl && vtbl->svt_get) {
	    vtbl->svt_get(aTHX_ sv, mg);

	    /* guard against magic having been deleted - eg FETCH calling
	     * untie */
	    if (!SvMAGIC(sv)) {
		(SSPTR(mgs_ix, MGS *))->mgs_magical = 0; /* recalculate flags */
		break;
	    }

	    /* recalculate flags if this entry was deleted. */
	    if (mg->mg_flags & MGf_GSKIP)
		(SSPTR(mgs_ix, MGS *))->mgs_magical = 0;
	}

	mg = nextmg;

	if (have_new) {
	    /* Have we finished with the new entries we saw? Start again
	       where we left off (unless there are more new entries). */
	    if (mg == head) {
		have_new = 0;
		mg   = cur;
		head = newmg;
	    }
	}

	/* Were any new entries added? */
	if (!have_new && (newmg = SvMAGIC(sv)) != head) {
	    have_new = 1;
	    cur = mg;
	    mg  = newmg;
	    (SSPTR(mgs_ix, MGS *))->mgs_magical = 0; /* recalculate flags */
	}
    }

    restore_magic(INT2PTR(void *, (IV)mgs_ix));
    return 0;
}

/*
=for apidoc mg_set

Do magic after a value is assigned to the SV.  See C<sv_magic>.

=cut
*/

int
Perl_mg_set(pTHX_ SV *sv)
{
    dVAR;
    const I32 mgs_ix = SSNEW(sizeof(MGS));
    MAGIC* mg;
    MAGIC* nextmg;

    PERL_ARGS_ASSERT_MG_SET;

    if (PL_localizing == 2 && sv == DEFSV) return 0;

    save_magic(mgs_ix, sv);

    for (mg = SvMAGIC(sv); mg; mg = nextmg) {
        const MGVTBL* vtbl = mg->mg_virtual;
	nextmg = mg->mg_moremagic;	/* it may delete itself */
	if (mg->mg_flags & MGf_GSKIP) {
	    mg->mg_flags &= ~MGf_GSKIP;	/* setting requires another read */
	    (SSPTR(mgs_ix, MGS*))->mgs_magical = 0;
	}
	if (PL_localizing == 2
	    && PERL_MAGIC_TYPE_IS_VALUE_MAGIC(mg->mg_type))
	    continue;
	if (vtbl && vtbl->svt_set)
	    vtbl->svt_set(aTHX_ sv, mg);
    }

    restore_magic(INT2PTR(void*, (IV)mgs_ix));
    return 0;
}

/*
=for apidoc mg_length

Report on the SV's length.  See C<sv_magic>.

=cut
*/

U32
Perl_mg_length(pTHX_ SV *sv)
{
    dVAR;
    MAGIC* mg;
    STRLEN len;

    PERL_ARGS_ASSERT_MG_LENGTH;

    for (mg = SvMAGIC(sv); mg; mg = mg->mg_moremagic) {
        const MGVTBL * const vtbl = mg->mg_virtual;
	if (vtbl && vtbl->svt_len) {
            const I32 mgs_ix = SSNEW(sizeof(MGS));
	    save_magic(mgs_ix, sv);
	    /* omit MGf_GSKIP -- not changed here */
	    len = vtbl->svt_len(aTHX_ sv, mg);
	    restore_magic(INT2PTR(void*, (IV)mgs_ix));
	    return len;
	}
    }

    {
	/* You can't know whether it's UTF-8 until you get the string again...
	 */
        const U8 *s = (U8*)SvPV_const(sv, len);

	if (DO_UTF8(sv)) {
	    len = utf8_length(s, s + len);
	}
    }
    return len;
}

I32
Perl_mg_size(pTHX_ SV *sv)
{
    MAGIC* mg;

    PERL_ARGS_ASSERT_MG_SIZE;

    for (mg = SvMAGIC(sv); mg; mg = mg->mg_moremagic) {
        const MGVTBL* const vtbl = mg->mg_virtual;
	if (vtbl && vtbl->svt_len) {
            const I32 mgs_ix = SSNEW(sizeof(MGS));
            I32 len;
	    save_magic(mgs_ix, sv);
	    /* omit MGf_GSKIP -- not changed here */
	    len = vtbl->svt_len(aTHX_ sv, mg);
	    restore_magic(INT2PTR(void*, (IV)mgs_ix));
	    return len;
	}
    }

    switch(SvTYPE(sv)) {
	case SVt_PVAV:
	    return AvFILLp((const AV *) sv); /* Fallback to non-tied array */
	case SVt_PVHV:
	    /* FIXME */
	default:
	    Perl_croak(aTHX_ "Size magic not implemented");
	    break;
    }
    return 0;
}

/*
=for apidoc mg_clear

Clear something magical that the SV represents.  See C<sv_magic>.

=cut
*/

int
Perl_mg_clear(pTHX_ SV *sv)
{
    const I32 mgs_ix = SSNEW(sizeof(MGS));
    MAGIC* mg;
    MAGIC *nextmg;

    PERL_ARGS_ASSERT_MG_CLEAR;

    save_magic(mgs_ix, sv);

    for (mg = SvMAGIC(sv); mg; mg = nextmg) {
        const MGVTBL* const vtbl = mg->mg_virtual;
	/* omit GSKIP -- never set here */

	nextmg = mg->mg_moremagic; /* it may delete itself */

	if (vtbl && vtbl->svt_clear)
	    vtbl->svt_clear(aTHX_ sv, mg);
    }

    restore_magic(INT2PTR(void*, (IV)mgs_ix));
    return 0;
}

static MAGIC*
S_mg_findext_flags(pTHX_ const SV *sv, int type, const MGVTBL *vtbl, U32 flags)
{
    PERL_UNUSED_CONTEXT;

    assert(flags <= 1);

    if (sv) {
	MAGIC *mg;

	for (mg = SvMAGIC(sv); mg; mg = mg->mg_moremagic) {
	    if (mg->mg_type == type && (!flags || mg->mg_virtual == vtbl)) {
		return mg;
	    }
	}
    }

    return NULL;
}

/*
=for apidoc mg_find

Finds the magic pointer for type matching the SV.  See C<sv_magic>.

=cut
*/

MAGIC*
Perl_mg_find(pTHX_ const SV *sv, int type)
{
    return S_mg_findext_flags(aTHX_ sv, type, NULL, 0);
}

/*
=for apidoc mg_findext

Finds the magic pointer of C<type> with the given C<vtbl> for the C<SV>.  See
C<sv_magicext>.

=cut
*/

MAGIC*
Perl_mg_findext(pTHX_ const SV *sv, int type, const MGVTBL *vtbl)
{
    return S_mg_findext_flags(aTHX_ sv, type, vtbl, 1);
}

/*
=for apidoc mg_copy

Copies the magic from one SV to another.  See C<sv_magic>.

=cut
*/

int
Perl_mg_copy(pTHX_ SV *sv, SV *nsv, const char *key, I32 klen)
{
    int count = 0;
    MAGIC* mg;

    PERL_ARGS_ASSERT_MG_COPY;

    for (mg = SvMAGIC(sv); mg; mg = mg->mg_moremagic) {
        const MGVTBL* const vtbl = mg->mg_virtual;
	if ((mg->mg_flags & MGf_COPY) && vtbl->svt_copy){
	    count += vtbl->svt_copy(aTHX_ sv, mg, nsv, key, klen);
	}
	else {
	    const char type = mg->mg_type;
	    if (isUPPER(type) && type != PERL_MAGIC_uvar) {
		sv_magic(nsv,
		     (type == PERL_MAGIC_tied)
			? SvTIED_obj(sv, mg)
			: (type == PERL_MAGIC_regdata && mg->mg_obj)
			    ? sv
			    : mg->mg_obj,
		     toLOWER(type), key, klen);
		count++;
	    }
	}
    }
    return count;
}

/*
=for apidoc mg_localize

Copy some of the magic from an existing SV to new localized version of that
SV. Container magic (eg %ENV, $1, tie) gets copied, value magic doesn't (eg
taint, pos).

If setmagic is false then no set magic will be called on the new (empty) SV.
This typically means that assignment will soon follow (e.g. 'local $x = $y'),
and that will handle the magic.

=cut
*/

void
Perl_mg_localize(pTHX_ SV *sv, SV *nsv, bool setmagic)
{
    dVAR;
    MAGIC *mg;

    PERL_ARGS_ASSERT_MG_LOCALIZE;

    if (nsv == DEFSV)
	return;

    for (mg = SvMAGIC(sv); mg; mg = mg->mg_moremagic) {
	const MGVTBL* const vtbl = mg->mg_virtual;
	if (PERL_MAGIC_TYPE_IS_VALUE_MAGIC(mg->mg_type))
	    continue;
		
	if ((mg->mg_flags & MGf_LOCAL) && vtbl->svt_local)
	    (void)vtbl->svt_local(aTHX_ nsv, mg);
	else
	    sv_magicext(nsv, mg->mg_obj, mg->mg_type, vtbl,
			    mg->mg_ptr, mg->mg_len);

	/* container types should remain read-only across localization */
	if (!SvIsCOW(sv)) SvFLAGS(nsv) |= SvREADONLY(sv);
    }

    if (SvTYPE(nsv) >= SVt_PVMG && SvMAGIC(nsv)) {
	SvFLAGS(nsv) |= SvMAGICAL(sv);
	if (setmagic) {
	    PL_localizing = 1;
	    SvSETMAGIC(nsv);
	    PL_localizing = 0;
	}
    }	    
}

#define mg_free_struct(sv, mg) S_mg_free_struct(aTHX_ sv, mg)
static void
S_mg_free_struct(pTHX_ SV *sv, MAGIC *mg)
{
    const MGVTBL* const vtbl = mg->mg_virtual;
    if (vtbl && vtbl->svt_free)
	vtbl->svt_free(aTHX_ sv, mg);
    if (mg->mg_ptr && mg->mg_type != PERL_MAGIC_regex_global) {
	if (mg->mg_len > 0 || mg->mg_type == PERL_MAGIC_utf8)
	    Safefree(mg->mg_ptr);
	else if (mg->mg_len == HEf_SVKEY)
	    SvREFCNT_dec(MUTABLE_SV(mg->mg_ptr));
    }
    if (mg->mg_flags & MGf_REFCOUNTED)
	SvREFCNT_dec(mg->mg_obj);
    Safefree(mg);
}

/*
=for apidoc mg_free

Free any magic storage used by the SV.  See C<sv_magic>.

=cut
*/

int
Perl_mg_free(pTHX_ SV *sv)
{
    MAGIC* mg;
    MAGIC* moremagic;

    PERL_ARGS_ASSERT_MG_FREE;

    for (mg = SvMAGIC(sv); mg; mg = moremagic) {
	moremagic = mg->mg_moremagic;
	mg_free_struct(sv, mg);
	SvMAGIC_set(sv, moremagic);
    }
    SvMAGIC_set(sv, NULL);
    SvMAGICAL_off(sv);
    return 0;
}

/*
=for apidoc Am|void|mg_free_type|SV *sv|int how

Remove any magic of type I<how> from the SV I<sv>.  See L</sv_magic>.

=cut
*/

void
Perl_mg_free_type(pTHX_ SV *sv, int how)
{
    MAGIC *mg, *prevmg, *moremg;
    PERL_ARGS_ASSERT_MG_FREE_TYPE;
    for (prevmg = NULL, mg = SvMAGIC(sv); mg; prevmg = mg, mg = moremg) {
	MAGIC *newhead;
	moremg = mg->mg_moremagic;
	if (mg->mg_type == how) {
	    /* temporarily move to the head of the magic chain, in case
	       custom free code relies on this historical aspect of mg_free */
	    if (prevmg) {
		prevmg->mg_moremagic = moremg;
		mg->mg_moremagic = SvMAGIC(sv);
		SvMAGIC_set(sv, mg);
	    }
	    newhead = mg->mg_moremagic;
	    mg_free_struct(sv, mg);
	    SvMAGIC_set(sv, newhead);
	    mg = prevmg;
	}
    }
    mg_magical(sv);
}

#include <signal.h>

U32
Perl_magic_regdata_cnt(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    PERL_UNUSED_ARG(sv);

    PERL_ARGS_ASSERT_MAGIC_REGDATA_CNT;

    if (PL_curpm) {
	register const REGEXP * const rx = PM_GETRE(PL_curpm);
	if (rx) {
	    if (mg->mg_obj) {			/* @+ */
		/* return the number possible */
		return RX_NPARENS(rx);
	    } else {				/* @- */
		I32 paren = RX_LASTPAREN(rx);

		/* return the last filled */
		while ( paren >= 0
			&& (RX_OFFS(rx)[paren].start == -1
			    || RX_OFFS(rx)[paren].end == -1) )
		    paren--;
		return (U32)paren;
	    }
	}
    }

    return (U32)-1;
}

int
Perl_magic_regdatum_get(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;

    PERL_ARGS_ASSERT_MAGIC_REGDATUM_GET;

    if (PL_curpm) {
	register const REGEXP * const rx = PM_GETRE(PL_curpm);
	if (rx) {
	    register const I32 paren = mg->mg_len;
	    register I32 s;
	    register I32 t;
	    if (paren < 0)
		return 0;
	    if (paren <= (I32)RX_NPARENS(rx) &&
		(s = RX_OFFS(rx)[paren].start) != -1 &&
		(t = RX_OFFS(rx)[paren].end) != -1)
		{
		    register I32 i;
		    if (mg->mg_obj)		/* @+ */
			i = t;
		    else			/* @- */
			i = s;

		    if (i > 0 && RX_MATCH_UTF8(rx)) {
			const char * const b = RX_SUBBEG(rx);
			if (b)
			    i = utf8_length((U8*)b, (U8*)(b+i));
		    }

		    sv_setiv(sv, i);
		}
	}
    }
    return 0;
}

int
Perl_magic_regdatum_set(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_REGDATUM_SET;
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(mg);
    Perl_croak_no_modify(aTHX);
    NORETURN_FUNCTION_END;
}

U32
Perl_magic_len(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    register I32 paren;
    register I32 i;
    register const REGEXP * rx;
    const char * const remaining = mg->mg_ptr + 1;

    PERL_ARGS_ASSERT_MAGIC_LEN;

    switch (*mg->mg_ptr) {
    case '\020':		
      if (*remaining == '\0') { /* ^P */
          break;
      } else if (strEQ(remaining, "REMATCH")) { /* $^PREMATCH */
          goto do_prematch;
      } else if (strEQ(remaining, "OSTMATCH")) { /* $^POSTMATCH */
          goto do_postmatch;
      }
      break;
    case '\015': /* $^MATCH */
	if (strEQ(remaining, "ATCH")) {
        goto do_match;
    } else {
        break;
    }
    case '`':
      do_prematch:
      paren = RX_BUFF_IDX_PREMATCH;
      goto maybegetparen;
    case '\'':
      do_postmatch:
      paren = RX_BUFF_IDX_POSTMATCH;
      goto maybegetparen;
    case '&':
      do_match:
      paren = RX_BUFF_IDX_FULLMATCH;
      goto maybegetparen;
    case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      paren = atoi(mg->mg_ptr);
    maybegetparen:
	if (PL_curpm && (rx = PM_GETRE(PL_curpm))) {
      getparen:
        i = CALLREG_NUMBUF_LENGTH((REGEXP * const)rx, sv, paren);

		if (i < 0)
		    Perl_croak(aTHX_ "panic: magic_len: %"IVdf, (IV)i);
		return i;
	} else {
		if (ckWARN(WARN_UNINITIALIZED))
		    report_uninit(sv);
		return 0;
	}
    case '+':
	if (PL_curpm && (rx = PM_GETRE(PL_curpm))) {
	    paren = RX_LASTPAREN(rx);
	    if (paren)
		goto getparen;
	}
	return 0;
    case '\016': /* ^N */
	if (PL_curpm && (rx = PM_GETRE(PL_curpm))) {
	    paren = RX_LASTCLOSEPAREN(rx);
	    if (paren)
		goto getparen;
	}
	return 0;
    }
    magic_get(sv,mg);
    if (!SvPOK(sv) && SvNIOK(sv)) {
	sv_2pv(sv, 0);
    }
    if (SvPOK(sv))
	return SvCUR(sv);
    return 0;
}

#define SvRTRIM(sv) STMT_START { \
    if (SvPOK(sv)) { \
        STRLEN len = SvCUR(sv); \
        char * const p = SvPVX(sv); \
	while (len > 0 && isSPACE(p[len-1])) \
	   --len; \
	SvCUR_set(sv, len); \
	p[len] = '\0'; \
    } \
} STMT_END

void
Perl_emulate_cop_io(pTHX_ const COP *const c, SV *const sv)
{
    PERL_ARGS_ASSERT_EMULATE_COP_IO;

    if (!(CopHINTS_get(c) & (HINT_LEXICAL_IO_IN|HINT_LEXICAL_IO_OUT)))
	sv_setsv(sv, &PL_sv_undef);
    else {
	sv_setpvs(sv, "");
	SvUTF8_off(sv);
	if ((CopHINTS_get(c) & HINT_LEXICAL_IO_IN)) {
	    SV *const value = cop_hints_fetch_pvs(c, "open<", 0);
	    assert(value);
	    sv_catsv(sv, value);
	}
	sv_catpvs(sv, "\0");
	if ((CopHINTS_get(c) & HINT_LEXICAL_IO_OUT)) {
	    SV *const value = cop_hints_fetch_pvs(c, "open>", 0);
	    assert(value);
	    sv_catsv(sv, value);
	}
    }
}

int
Perl_magic_get(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    register I32 paren;
    register const char *s = NULL;
    register REGEXP *rx;
    const char * const remaining = mg->mg_ptr + 1;
    const char nextchar = *remaining;

    PERL_ARGS_ASSERT_MAGIC_GET;

    switch (*mg->mg_ptr) {
    case '\001':		/* ^A */
	sv_setsv(sv, PL_bodytarget);
	if (SvTAINTED(PL_bodytarget))
	    SvTAINTED_on(sv);
	break;
    case '\003':		/* ^C, ^CHILD_ERROR_NATIVE */
	if (nextchar == '\0') {
	    sv_setiv(sv, (IV)PL_minus_c);
	}
	else if (strEQ(remaining, "HILD_ERROR_NATIVE")) {
	    sv_setiv(sv, (IV)STATUS_NATIVE);
        }
	break;

    case '\004':		/* ^D */
	sv_setiv(sv, (IV)(PL_debug & DEBUG_MASK));
	break;
    case '\005':  /* ^E */
	 if (nextchar == '\0') {
#if defined(VMS)
	     {
#	          include <descrip.h>
#	          include <starlet.h>
		  char msg[255];
		  $DESCRIPTOR(msgdsc,msg);
		  sv_setnv(sv,(NV) vaxc$errno);
		  if (sys$getmsg(vaxc$errno,&msgdsc.dsc$w_length,&msgdsc,0,0) & 1)
		       sv_setpvn(sv,msgdsc.dsc$a_pointer,msgdsc.dsc$w_length);
		  else
		       sv_setpvs(sv,"");
	     }
#elif defined(OS2)
	     if (!(_emx_env & 0x200)) {	/* Under DOS */
		  sv_setnv(sv, (NV)errno);
		  sv_setpv(sv, errno ? Strerror(errno) : "");
	     } else {
		  if (errno != errno_isOS2) {
		       const int tmp = _syserrno();
		       if (tmp)	/* 2nd call to _syserrno() makes it 0 */
			    Perl_rc = tmp;
		  }
		  sv_setnv(sv, (NV)Perl_rc);
		  sv_setpv(sv, os2error(Perl_rc));
	     }
#elif defined(WIN32)
	     {
		  const DWORD dwErr = GetLastError();
		  sv_setnv(sv, (NV)dwErr);
		  if (dwErr) {
		       PerlProc_GetOSError(sv, dwErr);
		  }
		  else
		       sv_setpvs(sv, "");
		  SetLastError(dwErr);
	     }
#else
	     {
		 dSAVE_ERRNO;
		 sv_setnv(sv, (NV)errno);
		 sv_setpv(sv, errno ? Strerror(errno) : "");
		 RESTORE_ERRNO;
	     }
#endif
	     SvRTRIM(sv);
	     SvNOK_on(sv);	/* what a wonderful hack! */
	 }
	 else if (strEQ(remaining, "NCODING"))
	      sv_setsv(sv, PL_encoding);
	 break;
    case '\006':		/* ^F */
	sv_setiv(sv, (IV)PL_maxsysfd);
	break;
    case '\007':		/* ^GLOBAL_PHASE */
	if (strEQ(remaining, "LOBAL_PHASE")) {
	    sv_setpvn(sv, PL_phase_names[PL_phase],
		      strlen(PL_phase_names[PL_phase]));
	}
	break;
    case '\010':		/* ^H */
	sv_setiv(sv, (IV)PL_hints);
	break;
    case '\011':		/* ^I */ /* NOT \t in EBCDIC */
	sv_setpv(sv, PL_inplace); /* Will undefine sv if PL_inplace is NULL */
	break;
    case '\017':		/* ^O & ^OPEN */
	if (nextchar == '\0') {
	    sv_setpv(sv, PL_osname);
	    SvTAINTED_off(sv);
	}
	else if (strEQ(remaining, "PEN")) {
	    Perl_emulate_cop_io(aTHX_ &PL_compiling, sv);
	}
	break;
    case '\020':
	if (nextchar == '\0') {       /* ^P */
	    sv_setiv(sv, (IV)PL_perldb);
	} else if (strEQ(remaining, "REMATCH")) { /* $^PREMATCH */
	    goto do_prematch_fetch;
	} else if (strEQ(remaining, "OSTMATCH")) { /* $^POSTMATCH */
	    goto do_postmatch_fetch;
	}
	break;
    case '\023':		/* ^S */
	if (nextchar == '\0') {
	    if (PL_parser && PL_parser->lex_state != LEX_NOTPARSING)
		SvOK_off(sv);
	    else if (PL_in_eval)
 		sv_setiv(sv, PL_in_eval & ~(EVAL_INREQUIRE));
	    else
		sv_setiv(sv, 0);
	}
	break;
    case '\024':		/* ^T */
	if (nextchar == '\0') {
#ifdef BIG_TIME
            sv_setnv(sv, PL_basetime);
#else
            sv_setiv(sv, (IV)PL_basetime);
#endif
        }
	else if (strEQ(remaining, "AINT"))
            sv_setiv(sv, PL_tainting
		    ? (PL_taint_warn || PL_unsafe ? -1 : 1)
		    : 0);
        break;
    case '\025':		/* $^UNICODE, $^UTF8LOCALE, $^UTF8CACHE */
	if (strEQ(remaining, "NICODE"))
	    sv_setuv(sv, (UV) PL_unicode);
	else if (strEQ(remaining, "TF8LOCALE"))
	    sv_setuv(sv, (UV) PL_utf8locale);
	else if (strEQ(remaining, "TF8CACHE"))
	    sv_setiv(sv, (IV) PL_utf8cache);
        break;
    case '\027':		/* ^W  & $^WARNING_BITS */
	if (nextchar == '\0')
	    sv_setiv(sv, (IV)((PL_dowarn & G_WARN_ON) ? TRUE : FALSE));
	else if (strEQ(remaining, "ARNING_BITS")) {
	    if (PL_compiling.cop_warnings == pWARN_NONE) {
	        sv_setpvn(sv, WARN_NONEstring, WARNsize) ;
	    }
	    else if (PL_compiling.cop_warnings == pWARN_STD) {
		sv_setsv(sv, &PL_sv_undef);
		break;
	    }
            else if (PL_compiling.cop_warnings == pWARN_ALL) {
		/* Get the bit mask for $warnings::Bits{all}, because
		 * it could have been extended by warnings::register */
		HV * const bits=get_hv("warnings::Bits", 0);
		if (bits) {
		    SV ** const bits_all = hv_fetchs(bits, "all", FALSE);
		    if (bits_all)
			sv_setsv(sv, *bits_all);
		}
	        else {
		    sv_setpvn(sv, WARN_ALLstring, WARNsize) ;
		}
	    }
            else {
	        sv_setpvn(sv, (char *) (PL_compiling.cop_warnings + 1),
			  *PL_compiling.cop_warnings);
	    }
	    SvPOK_only(sv);
	}
	break;
    case '\015': /* $^MATCH */
	if (strEQ(remaining, "ATCH")) {
    case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': case '&':
	    if (PL_curpm && (rx = PM_GETRE(PL_curpm))) {
		/*
		 * Pre-threads, this was paren = atoi(GvENAME((const GV *)mg->mg_obj));
		 * XXX Does the new way break anything?
		 */
		paren = atoi(mg->mg_ptr); /* $& is in [0] */
		CALLREG_NUMBUF_FETCH(rx,paren,sv);
		break;
	    }
	    sv_setsv(sv,&PL_sv_undef);
	}
	break;
    case '+':
	if (PL_curpm && (rx = PM_GETRE(PL_curpm))) {
	    if (RX_LASTPAREN(rx)) {
	        CALLREG_NUMBUF_FETCH(rx,RX_LASTPAREN(rx),sv);
	        break;
	    }
	}
	sv_setsv(sv,&PL_sv_undef);
	break;
    case '\016':		/* ^N */
	if (PL_curpm && (rx = PM_GETRE(PL_curpm))) {
	    if (RX_LASTCLOSEPAREN(rx)) {
	        CALLREG_NUMBUF_FETCH(rx,RX_LASTCLOSEPAREN(rx),sv);
	        break;
	    }

	}
	sv_setsv(sv,&PL_sv_undef);
	break;
    case '`':
      do_prematch_fetch:
	if (PL_curpm && (rx = PM_GETRE(PL_curpm))) {
	    CALLREG_NUMBUF_FETCH(rx,-2,sv);
	    break;
	}
	sv_setsv(sv,&PL_sv_undef);
	break;
    case '\'':
      do_postmatch_fetch:
	if (PL_curpm && (rx = PM_GETRE(PL_curpm))) {
	    CALLREG_NUMBUF_FETCH(rx,-1,sv);
	    break;
	}
	sv_setsv(sv,&PL_sv_undef);
	break;
    case '.':
	if (GvIO(PL_last_in_gv)) {
	    sv_setiv(sv, (IV)IoLINES(GvIOp(PL_last_in_gv)));
	}
	break;
    case '?':
	{
	    sv_setiv(sv, (IV)STATUS_CURRENT);
#ifdef COMPLEX_STATUS
	    SvUPGRADE(sv, SVt_PVLV);
	    LvTARGOFF(sv) = PL_statusvalue;
	    LvTARGLEN(sv) = PL_statusvalue_vms;
#endif
	}
	break;
    case '^':
	if (GvIOp(PL_defoutgv))
		s = IoTOP_NAME(GvIOp(PL_defoutgv));
	if (s)
	    sv_setpv(sv,s);
	else {
	    sv_setpv(sv,GvENAME(PL_defoutgv));
	    sv_catpvs(sv,"_TOP");
	}
	break;
    case '~':
	if (GvIOp(PL_defoutgv))
	    s = IoFMT_NAME(GvIOp(PL_defoutgv));
	if (!s)
	    s = GvENAME(PL_defoutgv);
	sv_setpv(sv,s);
	break;
    case '=':
	if (GvIO(PL_defoutgv))
	    sv_setiv(sv, (IV)IoPAGE_LEN(GvIOp(PL_defoutgv)));
	break;
    case '-':
	if (GvIO(PL_defoutgv))
	    sv_setiv(sv, (IV)IoLINES_LEFT(GvIOp(PL_defoutgv)));
	break;
    case '%':
	if (GvIO(PL_defoutgv))
	    sv_setiv(sv, (IV)IoPAGE(GvIOp(PL_defoutgv)));
	break;
    case ':':
	break;
    case '/':
	break;
    case '[':
	sv_setiv(sv, 0);
	break;
    case '|':
	if (GvIO(PL_defoutgv))
	    sv_setiv(sv, (IV)(IoFLAGS(GvIOp(PL_defoutgv)) & IOf_FLUSH) != 0 );
	break;
    case '\\':
	if (PL_ors_sv)
	    sv_copypv(sv, PL_ors_sv);
	break;
    case '$': /* $$ */
	{
	    IV const pid = (IV)PerlProc_getpid();
	    if (isGV(mg->mg_obj) || SvIV(mg->mg_obj) != pid) {
		/* never set manually, or at least not since last fork */
		sv_setiv(sv, pid);
		/* never unsafe, even if reading in a tainted expression */
		SvTAINTED_off(sv);
	    }
	    /* else a value has been assigned manually, so do nothing */
	}
	break;

    case '!':
	{
	dSAVE_ERRNO;
#ifdef VMS
	sv_setnv(sv, (NV)((errno == EVMSERR) ? vaxc$errno : errno));
#else
	sv_setnv(sv, (NV)errno);
#endif
#ifdef OS2
	if (errno == errno_isOS2 || errno == errno_isOS2_set)
	    sv_setpv(sv, os2error(Perl_rc));
	else
#endif
	sv_setpv(sv, errno ? Strerror(errno) : "");
	if (SvPOKp(sv))
	    SvPOK_on(sv);    /* may have got removed during taint processing */
	RESTORE_ERRNO;
	}

	SvRTRIM(sv);
	SvNOK_on(sv);	/* what a wonderful hack! */
	break;
    case '<':
	sv_setiv(sv, (IV)PerlProc_getuid());
	break;
    case '>':
	sv_setiv(sv, (IV)PerlProc_geteuid());
	break;
    case '(':
	sv_setiv(sv, (IV)PerlProc_getgid());
	goto add_groups;
    case ')':
	sv_setiv(sv, (IV)PerlProc_getegid());
      add_groups:
#ifdef HAS_GETGROUPS
	{
	    Groups_t *gary = NULL;
	    I32 i, num_groups = getgroups(0, gary);
            Newx(gary, num_groups, Groups_t);
            num_groups = getgroups(num_groups, gary);
	    for (i = 0; i < num_groups; i++)
		Perl_sv_catpvf(aTHX_ sv, " %"IVdf, (IV)gary[i]);
            Safefree(gary);
	}
	(void)SvIOK_on(sv);	/* what a wonderful hack! */
#endif
	break;
    case '0':
	break;
    }
    return 0;
}

int
Perl_magic_getuvar(pTHX_ SV *sv, MAGIC *mg)
{
    struct ufuncs * const uf = (struct ufuncs *)mg->mg_ptr;

    PERL_ARGS_ASSERT_MAGIC_GETUVAR;

    if (uf && uf->uf_val)
	(*uf->uf_val)(aTHX_ uf->uf_index, sv);
    return 0;
}

int
Perl_magic_setenv(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    STRLEN len = 0, klen;
    const char *s = SvOK(sv) ? SvPV_const(sv,len) : "";
    const char * const ptr = MgPV_const(mg,klen);
    my_setenv(ptr, s);

    PERL_ARGS_ASSERT_MAGIC_SETENV;

#ifdef DYNAMIC_ENV_FETCH
     /* We just undefd an environment var.  Is a replacement */
     /* waiting in the wings? */
    if (!len) {
	SV ** const valp = hv_fetch(GvHVn(PL_envgv), ptr, klen, FALSE);
	if (valp)
	    s = SvOK(*valp) ? SvPV_const(*valp, len) : "";
    }
#endif

#if !defined(OS2) && !defined(AMIGAOS) && !defined(WIN32) && !defined(MSDOS)
			    /* And you'll never guess what the dog had */
			    /*   in its mouth... */
    if (PL_tainting) {
	MgTAINTEDDIR_off(mg);
#ifdef VMS
	if (s && klen == 8 && strEQ(ptr, "DCL$PATH")) {
	    char pathbuf[256], eltbuf[256], *cp, *elt;
	    int i = 0, j = 0;

	    my_strlcpy(eltbuf, s, sizeof(eltbuf));
	    elt = eltbuf;
	    do {          /* DCL$PATH may be a search list */
		while (1) {   /* as may dev portion of any element */
		    if ( ((cp = strchr(elt,'[')) || (cp = strchr(elt,'<'))) ) {
			if ( *(cp+1) == '.' || *(cp+1) == '-' ||
			     cando_by_name(S_IWUSR,0,elt) ) {
			    MgTAINTEDDIR_on(mg);
			    return 0;
			}
		    }
		    if ((cp = strchr(elt, ':')) != NULL)
			*cp = '\0';
		    if (my_trnlnm(elt, eltbuf, j++))
			elt = eltbuf;
		    else
			break;
		}
		j = 0;
	    } while (my_trnlnm(s, pathbuf, i++) && (elt = pathbuf));
	}
#endif /* VMS */
	if (s && klen == 4 && strEQ(ptr,"PATH")) {
	    const char * const strend = s + len;

	    while (s < strend) {
		char tmpbuf[256];
		Stat_t st;
		I32 i;
#ifdef VMS  /* Hmm.  How do we get $Config{path_sep} from C? */
		const char path_sep = '|';
#else
		const char path_sep = ':';
#endif
		s = delimcpy(tmpbuf, tmpbuf + sizeof tmpbuf,
			     s, strend, path_sep, &i);
		s++;
		if (i >= (I32)sizeof tmpbuf   /* too long -- assume the worst */
#ifdef VMS
		      || !strchr(tmpbuf, ':') /* no colon thus no device name -- assume relative path */
#else
		      || *tmpbuf != '/'       /* no starting slash -- assume relative path */
#endif
		      || (PerlLIO_stat(tmpbuf, &st) == 0 && (st.st_mode & 2)) ) {
		    MgTAINTEDDIR_on(mg);
		    return 0;
		}
	    }
	}
    }
#endif /* neither OS2 nor AMIGAOS nor WIN32 nor MSDOS */

    return 0;
}

int
Perl_magic_clearenv(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_CLEARENV;
    PERL_UNUSED_ARG(sv);
    my_setenv(MgPV_nolen_const(mg),NULL);
    return 0;
}

int
Perl_magic_set_all_env(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    PERL_ARGS_ASSERT_MAGIC_SET_ALL_ENV;
    PERL_UNUSED_ARG(mg);
#if defined(VMS)
    Perl_die(aTHX_ "Can't make list assignment to %%ENV on this system");
#else
    if (PL_localizing) {
	HE* entry;
	my_clearenv();
	hv_iterinit(MUTABLE_HV(sv));
	while ((entry = hv_iternext(MUTABLE_HV(sv)))) {
	    I32 keylen;
	    my_setenv(hv_iterkey(entry, &keylen),
		      SvPV_nolen_const(hv_iterval(MUTABLE_HV(sv), entry)));
	}
    }
#endif
    return 0;
}

int
Perl_magic_clear_all_env(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    PERL_ARGS_ASSERT_MAGIC_CLEAR_ALL_ENV;
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(mg);
#if defined(VMS)
    Perl_die(aTHX_ "Can't make list assignment to %%ENV on this system");
#else
    my_clearenv();
#endif
    return 0;
}

#ifndef PERL_MICRO
#ifdef HAS_SIGPROCMASK
static void
restore_sigmask(pTHX_ SV *save_sv)
{
    const sigset_t * const ossetp = (const sigset_t *) SvPV_nolen_const( save_sv );
    (void)sigprocmask(SIG_SETMASK, ossetp, NULL);
}
#endif
int
Perl_magic_getsig(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    /* Are we fetching a signal entry? */
    int i = (I16)mg->mg_private;

    PERL_ARGS_ASSERT_MAGIC_GETSIG;

    if (!i) {
        STRLEN siglen;
        const char * sig = MgPV_const(mg, siglen);
        mg->mg_private = i = whichsig_pvn(sig, siglen);
    }

    if (i > 0) {
    	if(PL_psig_ptr[i])
    	    sv_setsv(sv,PL_psig_ptr[i]);
    	else {
	    Sighandler_t sigstate = rsignal_state(i);
#ifdef FAKE_PERSISTENT_SIGNAL_HANDLERS
	    if (PL_sig_handlers_initted && PL_sig_ignoring[i])
		sigstate = SIG_IGN;
#endif
#ifdef FAKE_DEFAULT_SIGNAL_HANDLERS
	    if (PL_sig_handlers_initted && PL_sig_defaulting[i])
		sigstate = SIG_DFL;
#endif
    	    /* cache state so we don't fetch it again */
    	    if(sigstate == (Sighandler_t) SIG_IGN)
    	    	sv_setpvs(sv,"IGNORE");
    	    else
    	    	sv_setsv(sv,&PL_sv_undef);
	    PL_psig_ptr[i] = SvREFCNT_inc_simple_NN(sv);
    	    SvTEMP_off(sv);
    	}
    }
    return 0;
}
int
Perl_magic_clearsig(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_CLEARSIG;

    magic_setsig(NULL, mg);
    return sv_unmagic(sv, mg->mg_type);
}

Signal_t
#if defined(HAS_SIGACTION) && defined(SA_SIGINFO)
Perl_csighandler(int sig, siginfo_t *sip PERL_UNUSED_DECL, void *uap PERL_UNUSED_DECL)
#else
Perl_csighandler(int sig)
#endif
{
#ifdef PERL_GET_SIG_CONTEXT
    dTHXa(PERL_GET_SIG_CONTEXT);
#else
    dTHX;
#endif
#ifdef FAKE_PERSISTENT_SIGNAL_HANDLERS
    (void) rsignal(sig, PL_csighandlerp);
    if (PL_sig_ignoring[sig]) return;
#endif
#ifdef FAKE_DEFAULT_SIGNAL_HANDLERS
    if (PL_sig_defaulting[sig])
#ifdef KILL_BY_SIGPRC
            exit((Perl_sig_to_vmscondition(sig)&STS$M_COND_ID)|STS$K_SEVERE|STS$M_INHIB_MSG);
#else
            exit(1);
#endif
#endif
    if (
#ifdef SIGILL
	   sig == SIGILL ||
#endif
#ifdef SIGBUS
	   sig == SIGBUS ||
#endif
#ifdef SIGSEGV
	   sig == SIGSEGV ||
#endif
	   (PL_signals & PERL_SIGNALS_UNSAFE_FLAG))
	/* Call the perl level handler now--
	 * with risk we may be in malloc() or being destructed etc. */
#if defined(HAS_SIGACTION) && defined(SA_SIGINFO)
	(*PL_sighandlerp)(sig, NULL, NULL);
#else
	(*PL_sighandlerp)(sig);
#endif
    else {
	if (!PL_psig_pend) return;
	/* Set a flag to say this signal is pending, that is awaiting delivery after
	 * the current Perl opcode completes */
	PL_psig_pend[sig]++;

#ifndef SIG_PENDING_DIE_COUNT
#  define SIG_PENDING_DIE_COUNT 120
#endif
	/* Add one to say _a_ signal is pending */
	if (++PL_sig_pending >= SIG_PENDING_DIE_COUNT)
	    Perl_croak(aTHX_ "Maximal count of pending signals (%lu) exceeded",
		       (unsigned long)SIG_PENDING_DIE_COUNT);
    }
}

#if defined(FAKE_PERSISTENT_SIGNAL_HANDLERS) || defined(FAKE_DEFAULT_SIGNAL_HANDLERS)
void
Perl_csighandler_init(void)
{
    int sig;
    if (PL_sig_handlers_initted) return;

    for (sig = 1; sig < SIG_SIZE; sig++) {
#ifdef FAKE_DEFAULT_SIGNAL_HANDLERS
        dTHX;
        PL_sig_defaulting[sig] = 1;
        (void) rsignal(sig, PL_csighandlerp);
#endif
#ifdef FAKE_PERSISTENT_SIGNAL_HANDLERS
        PL_sig_ignoring[sig] = 0;
#endif
    }
    PL_sig_handlers_initted = 1;
}
#endif

#if defined HAS_SIGPROCMASK
static void
unblock_sigmask(pTHX_ void* newset)
{
    sigprocmask(SIG_UNBLOCK, (sigset_t*)newset, NULL);
}
#endif

void
Perl_despatch_signals(pTHX)
{
    dVAR;
    int sig;
    PL_sig_pending = 0;
    for (sig = 1; sig < SIG_SIZE; sig++) {
	if (PL_psig_pend[sig]) {
	    dSAVE_ERRNO;
#ifdef HAS_SIGPROCMASK
	    /* From sigaction(2) (FreeBSD man page):
	     * | Signal routines normally execute with the signal that
	     * | caused their invocation blocked, but other signals may
	     * | yet occur.
	     * Emulation of this behavior (from within Perl) is enabled
	     * using sigprocmask
	     */
	    int was_blocked;
	    sigset_t newset, oldset;

	    sigemptyset(&newset);
	    sigaddset(&newset, sig);
	    sigprocmask(SIG_BLOCK, &newset, &oldset);
	    was_blocked = sigismember(&oldset, sig);
	    if (!was_blocked) {
		SV* save_sv = newSVpvn((char *)(&newset), sizeof(sigset_t));
		ENTER;
		SAVEFREESV(save_sv);
		SAVEDESTRUCTOR_X(unblock_sigmask, SvPV_nolen(save_sv));
	    }
#endif
 	    PL_psig_pend[sig] = 0;
#if defined(HAS_SIGACTION) && defined(SA_SIGINFO)
	    (*PL_sighandlerp)(sig, NULL, NULL);
#else
	    (*PL_sighandlerp)(sig);
#endif
#ifdef HAS_SIGPROCMASK
	    if (!was_blocked)
		LEAVE;
#endif
	    RESTORE_ERRNO;
	}
    }
}

/* sv of NULL signifies that we're acting as magic_clearsig.  */
int
Perl_magic_setsig(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    I32 i;
    SV** svp = NULL;
    /* Need to be careful with SvREFCNT_dec(), because that can have side
     * effects (due to closures). We must make sure that the new disposition
     * is in place before it is called.
     */
    SV* to_dec = NULL;
    STRLEN len;
#ifdef HAS_SIGPROCMASK
    sigset_t set, save;
    SV* save_sv;
#endif
    register const char *s = MgPV_const(mg,len);

    PERL_ARGS_ASSERT_MAGIC_SETSIG;

    if (*s == '_') {
        if (memEQs(s, len, "__DIE__"))
	    svp = &PL_diehook;
	else if (memEQs(s, len, "__WARN__")
		 && (sv ? 1 : PL_warnhook != PERL_WARNHOOK_FATAL)) {
	    /* Merge the existing behaviours, which are as follows:
	       magic_setsig, we always set svp to &PL_warnhook
	       (hence we always change the warnings handler)
	       For magic_clearsig, we don't change the warnings handler if it's
	       set to the &PL_warnhook.  */
	    svp = &PL_warnhook;
        } else if (sv) {
            SV *tmp = sv_newmortal();
            Perl_croak(aTHX_ "No such hook: %s",
                                pv_pretty(tmp, s, len, 0, NULL, NULL, 0));
        }
	i = 0;
	if (svp && *svp) {
	    if (*svp != PERL_WARNHOOK_FATAL)
		to_dec = *svp;
	    *svp = NULL;
	}
    }
    else {
	i = (I16)mg->mg_private;
	if (!i) {
	    i = whichsig_pvn(s, len);   /* ...no, a brick */
	    mg->mg_private = (U16)i;
	}
	if (i <= 0) {
	    if (sv) {
                SV *tmp = sv_newmortal();
		Perl_ck_warner(aTHX_ packWARN(WARN_SIGNAL), "No such signal: SIG%s",
                                            pv_pretty(tmp, s, len, 0, NULL, NULL, 0));
            }
	    return 0;
	}
#ifdef HAS_SIGPROCMASK
	/* Avoid having the signal arrive at a bad time, if possible. */
	sigemptyset(&set);
	sigaddset(&set,i);
	sigprocmask(SIG_BLOCK, &set, &save);
	ENTER;
	save_sv = newSVpvn((char *)(&save), sizeof(sigset_t));
	SAVEFREESV(save_sv);
	SAVEDESTRUCTOR_X(restore_sigmask, save_sv);
#endif
	PERL_ASYNC_CHECK();
#if defined(FAKE_PERSISTENT_SIGNAL_HANDLERS) || defined(FAKE_DEFAULT_SIGNAL_HANDLERS)
	if (!PL_sig_handlers_initted) Perl_csighandler_init();
#endif
#ifdef FAKE_PERSISTENT_SIGNAL_HANDLERS
	PL_sig_ignoring[i] = 0;
#endif
#ifdef FAKE_DEFAULT_SIGNAL_HANDLERS
	PL_sig_defaulting[i] = 0;
#endif
	to_dec = PL_psig_ptr[i];
	if (sv) {
	    PL_psig_ptr[i] = SvREFCNT_inc_simple_NN(sv);
	    SvTEMP_off(sv); /* Make sure it doesn't go away on us */

	    /* Signals don't change name during the program's execution, so once
	       they're cached in the appropriate slot of PL_psig_name, they can
	       stay there.

	       Ideally we'd find some way of making SVs at (C) compile time, or
	       at least, doing most of the work.  */
	    if (!PL_psig_name[i]) {
		PL_psig_name[i] = newSVpvn(s, len);
		SvREADONLY_on(PL_psig_name[i]);
	    }
	} else {
	    SvREFCNT_dec(PL_psig_name[i]);
	    PL_psig_name[i] = NULL;
	    PL_psig_ptr[i] = NULL;
	}
    }
    if (sv && (isGV_with_GP(sv) || SvROK(sv))) {
	if (i) {
	    (void)rsignal(i, PL_csighandlerp);
	}
	else
	    *svp = SvREFCNT_inc_simple_NN(sv);
    } else {
	if (sv && SvOK(sv)) {
	    s = SvPV_force(sv, len);
	} else {
	    sv = NULL;
	}
	if (sv && memEQs(s, len,"IGNORE")) {
	    if (i) {
#ifdef FAKE_PERSISTENT_SIGNAL_HANDLERS
		PL_sig_ignoring[i] = 1;
		(void)rsignal(i, PL_csighandlerp);
#else
		(void)rsignal(i, (Sighandler_t) SIG_IGN);
#endif
	    }
	}
	else if (!sv || memEQs(s, len,"DEFAULT") || !len) {
	    if (i) {
#ifdef FAKE_DEFAULT_SIGNAL_HANDLERS
		PL_sig_defaulting[i] = 1;
		(void)rsignal(i, PL_csighandlerp);
#else
		(void)rsignal(i, (Sighandler_t) SIG_DFL);
#endif
	    }
	}
	else {
	    /*
	     * We should warn if HINT_STRICT_REFS, but without
	     * access to a known hint bit in a known OP, we can't
	     * tell whether HINT_STRICT_REFS is in force or not.
	     */
	    if (!strchr(s,':') && !strchr(s,'\''))
		Perl_sv_insert_flags(aTHX_ sv, 0, 0, STR_WITH_LEN("main::"),
				     SV_GMAGIC);
	    if (i)
		(void)rsignal(i, PL_csighandlerp);
	    else
		*svp = SvREFCNT_inc_simple_NN(sv);
	}
    }

#ifdef HAS_SIGPROCMASK
    if(i)
	LEAVE;
#endif
    SvREFCNT_dec(to_dec);
    return 0;
}
#endif /* !PERL_MICRO */

int
Perl_magic_setisa(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    PERL_ARGS_ASSERT_MAGIC_SETISA;
    PERL_UNUSED_ARG(sv);

    /* Skip _isaelem because _isa will handle it shortly */
    if (PL_delaymagic & DM_ARRAY_ISA && mg->mg_type == PERL_MAGIC_isaelem)
	return 0;

    return magic_clearisa(NULL, mg);
}

/* sv of NULL signifies that we're acting as magic_setisa.  */
int
Perl_magic_clearisa(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    HV* stash;

    PERL_ARGS_ASSERT_MAGIC_CLEARISA;

    /* Bail out if destruction is going on */
    if(PL_phase == PERL_PHASE_DESTRUCT) return 0;

    if (sv)
	av_clear(MUTABLE_AV(sv));

    if (SvTYPE(mg->mg_obj) != SVt_PVGV && SvSMAGICAL(mg->mg_obj))
	/* This occurs with setisa_elem magic, which calls this
	   same function. */
	mg = mg_find(mg->mg_obj, PERL_MAGIC_isa);

    if (SvTYPE(mg->mg_obj) == SVt_PVAV) { /* multiple stashes */
	SV **svp = AvARRAY((AV *)mg->mg_obj);
	I32 items = AvFILLp((AV *)mg->mg_obj) + 1;
	while (items--) {
	    stash = GvSTASH((GV *)*svp++);
	    if (stash && HvENAME(stash)) mro_isa_changed_in(stash);
	}

	return 0;
    }

    stash = GvSTASH(
        (const GV *)mg->mg_obj
    );

    /* The stash may have been detached from the symbol table, so check its
       name before doing anything. */
    if (stash && HvENAME_get(stash))
	mro_isa_changed_in(stash);

    return 0;
}

int
Perl_magic_setamagic(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    PERL_ARGS_ASSERT_MAGIC_SETAMAGIC;
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(mg);
    PL_amagic_generation++;

    return 0;
}

int
Perl_magic_getnkeys(pTHX_ SV *sv, MAGIC *mg)
{
    HV * const hv = MUTABLE_HV(LvTARG(sv));
    I32 i = 0;

    PERL_ARGS_ASSERT_MAGIC_GETNKEYS;
    PERL_UNUSED_ARG(mg);

    if (hv) {
         (void) hv_iterinit(hv);
         if (! SvTIED_mg((const SV *)hv, PERL_MAGIC_tied))
	     i = HvUSEDKEYS(hv);
         else {
	     while (hv_iternext(hv))
	         i++;
         }
    }

    sv_setiv(sv, (IV)i);
    return 0;
}

int
Perl_magic_setnkeys(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_SETNKEYS;
    PERL_UNUSED_ARG(mg);
    if (LvTARG(sv)) {
	hv_ksplit(MUTABLE_HV(LvTARG(sv)), SvIV(sv));
    }
    return 0;
}

/*
=for apidoc magic_methcall

Invoke a magic method (like FETCH).

C<sv> and C<mg> are the tied thingy and the tie magic.

C<meth> is the name of the method to call.

C<argc> is the number of args (in addition to $self) to pass to the method.

The C<flags> can be:

    G_DISCARD     invoke method with G_DISCARD flag and don't
                  return a value
    G_UNDEF_FILL  fill the stack with argc pointers to
                  PL_sv_undef

The arguments themselves are any values following the C<flags> argument.

Returns the SV (if any) returned by the method, or NULL on failure.


=cut
*/

SV*
Perl_magic_methcall(pTHX_ SV *sv, const MAGIC *mg, const char *meth, U32 flags,
		    U32 argc, ...)
{
    dVAR;
    dSP;
    SV* ret = NULL;

    PERL_ARGS_ASSERT_MAGIC_METHCALL;

    ENTER;

    if (flags & G_WRITING_TO_STDERR) {
	SAVETMPS;

	save_re_context();
	SAVESPTR(PL_stderrgv);
	PL_stderrgv = NULL;
    }

    PUSHSTACKi(PERLSI_MAGIC);
    PUSHMARK(SP);

    EXTEND(SP, argc+1);
    PUSHs(SvTIED_obj(sv, mg));
    if (flags & G_UNDEF_FILL) {
	while (argc--) {
	    PUSHs(&PL_sv_undef);
	}
    } else if (argc > 0) {
	va_list args;
	va_start(args, argc);

	do {
	    SV *const sv = va_arg(args, SV *);
	    PUSHs(sv);
	} while (--argc);

	va_end(args);
    }
    PUTBACK;
    if (flags & G_DISCARD) {
	call_method(meth, G_SCALAR|G_DISCARD);
    }
    else {
	if (call_method(meth, G_SCALAR))
	    ret = *PL_stack_sp--;
    }
    POPSTACK;
    if (flags & G_WRITING_TO_STDERR)
	FREETMPS;
    LEAVE;
    return ret;
}


/* wrapper for magic_methcall that creates the first arg */

STATIC SV*
S_magic_methcall1(pTHX_ SV *sv, const MAGIC *mg, const char *meth, U32 flags,
    int n, SV *val)
{
    dVAR;
    SV* arg1 = NULL;

    PERL_ARGS_ASSERT_MAGIC_METHCALL1;

    if (mg->mg_ptr) {
	if (mg->mg_len >= 0) {
	    arg1 = newSVpvn_flags(mg->mg_ptr, mg->mg_len, SVs_TEMP);
	}
	else if (mg->mg_len == HEf_SVKEY)
	    arg1 = MUTABLE_SV(mg->mg_ptr);
    }
    else if (mg->mg_type == PERL_MAGIC_tiedelem) {
	arg1 = newSViv((IV)(mg->mg_len));
	sv_2mortal(arg1);
    }
    if (!arg1) {
	return Perl_magic_methcall(aTHX_ sv, mg, meth, flags, n - 1, val);
    }
    return Perl_magic_methcall(aTHX_ sv, mg, meth, flags, n, arg1, val);
}

STATIC int
S_magic_methpack(pTHX_ SV *sv, const MAGIC *mg, const char *meth)
{
    dVAR;
    SV* ret;

    PERL_ARGS_ASSERT_MAGIC_METHPACK;

    ret = magic_methcall1(sv, mg, meth, 0, 1, NULL);
    if (ret)
	sv_setsv(sv, ret);
    return 0;
}

int
Perl_magic_getpack(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_GETPACK;

    if (mg->mg_type == PERL_MAGIC_tiedelem)
	mg->mg_flags |= MGf_GSKIP;
    magic_methpack(sv,mg,"FETCH");
    return 0;
}

int
Perl_magic_setpack(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    MAGIC *tmg;
    SV    *val;

    PERL_ARGS_ASSERT_MAGIC_SETPACK;

    /* in the code C<$tied{foo} = $val>, the "thing" that gets passed to
     * STORE() is not $val, but rather a PVLV (the sv in this call), whose
     * public flags indicate its value based on copying from $val. Doing
     * mg_set() on the PVLV temporarily does SvMAGICAL_off(), then calls us.
     * So STORE()'s $_[2] arg is a temporarily disarmed PVLV. This goes
     * wrong if $val happened to be tainted, as sv hasn't got magic
     * enabled, even though taint magic is in the chain. In which case,
     * fake up a temporary tainted value (this is easier than temporarily
     * re-enabling magic on sv). */

    if (PL_tainting && (tmg = mg_find(sv, PERL_MAGIC_taint))
	&& (tmg->mg_len & 1))
    {
	val = sv_mortalcopy(sv);
	SvTAINTED_on(val);
    }
    else
	val = sv;

    magic_methcall1(sv, mg, "STORE", G_DISCARD, 2, val);
    return 0;
}

int
Perl_magic_clearpack(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_CLEARPACK;

    if (mg->mg_type == PERL_MAGIC_tiedscalar) return 0;
    return magic_methpack(sv,mg,"DELETE");
}


U32
Perl_magic_sizepack(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    I32 retval = 0;
    SV* retsv;

    PERL_ARGS_ASSERT_MAGIC_SIZEPACK;

    retsv = magic_methcall1(sv, mg, "FETCHSIZE", 0, 1, NULL);
    if (retsv) {
	retval = SvIV(retsv)-1;
	if (retval < -1)
	    Perl_croak(aTHX_ "FETCHSIZE returned a negative value");
    }
    return (U32) retval;
}

int
Perl_magic_wipepack(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;

    PERL_ARGS_ASSERT_MAGIC_WIPEPACK;

    Perl_magic_methcall(aTHX_ sv, mg, "CLEAR", G_DISCARD, 0);
    return 0;
}

int
Perl_magic_nextpack(pTHX_ SV *sv, MAGIC *mg, SV *key)
{
    dVAR;
    SV* ret;

    PERL_ARGS_ASSERT_MAGIC_NEXTPACK;

    ret = SvOK(key) ? Perl_magic_methcall(aTHX_ sv, mg, "NEXTKEY", 0, 1, key)
	: Perl_magic_methcall(aTHX_ sv, mg, "FIRSTKEY", 0, 0);
    if (ret)
	sv_setsv(key,ret);
    return 0;
}

int
Perl_magic_existspack(pTHX_ SV *sv, const MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_EXISTSPACK;

    return magic_methpack(sv,mg,"EXISTS");
}

SV *
Perl_magic_scalarpack(pTHX_ HV *hv, MAGIC *mg)
{
    dVAR;
    SV *retval;
    SV * const tied = SvTIED_obj(MUTABLE_SV(hv), mg);
    HV * const pkg = SvSTASH((const SV *)SvRV(tied));
   
    PERL_ARGS_ASSERT_MAGIC_SCALARPACK;

    if (!gv_fetchmethod_autoload(pkg, "SCALAR", FALSE)) {
        SV *key;
        if (HvEITER_get(hv))
            /* we are in an iteration so the hash cannot be empty */
            return &PL_sv_yes;
        /* no xhv_eiter so now use FIRSTKEY */
        key = sv_newmortal();
        magic_nextpack(MUTABLE_SV(hv), mg, key);
        HvEITER_set(hv, NULL);     /* need to reset iterator */
        return SvOK(key) ? &PL_sv_yes : &PL_sv_no;
    }
   
    /* there is a SCALAR method that we can call */
    retval = Perl_magic_methcall(aTHX_ MUTABLE_SV(hv), mg, "SCALAR", 0, 0);
    if (!retval)
	retval = &PL_sv_undef;
    return retval;
}

int
Perl_magic_setdbline(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    GV * const gv = PL_DBline;
    const I32 i = SvTRUE(sv);
    SV ** const svp = av_fetch(GvAV(gv),
		     atoi(MgPV_nolen_const(mg)), FALSE);

    PERL_ARGS_ASSERT_MAGIC_SETDBLINE;

    if (svp && SvIOKp(*svp)) {
	OP * const o = INT2PTR(OP*,SvIVX(*svp));
	if (o) {
	    /* set or clear breakpoint in the relevant control op */
	    if (i)
		o->op_flags |= OPf_SPECIAL;
	    else
		o->op_flags &= ~OPf_SPECIAL;
	}
    }
    return 0;
}

int
Perl_magic_getarylen(pTHX_ SV *sv, const MAGIC *mg)
{
    dVAR;
    AV * const obj = MUTABLE_AV(mg->mg_obj);

    PERL_ARGS_ASSERT_MAGIC_GETARYLEN;

    if (obj) {
	sv_setiv(sv, AvFILL(obj));
    } else {
	SvOK_off(sv);
    }
    return 0;
}

int
Perl_magic_setarylen(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    AV * const obj = MUTABLE_AV(mg->mg_obj);

    PERL_ARGS_ASSERT_MAGIC_SETARYLEN;

    if (obj) {
	av_fill(obj, SvIV(sv));
    } else {
	Perl_ck_warner(aTHX_ packWARN(WARN_MISC),
		       "Attempt to set length of freed array");
    }
    return 0;
}

int
Perl_magic_freearylen_p(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;

    PERL_ARGS_ASSERT_MAGIC_FREEARYLEN_P;
    PERL_UNUSED_ARG(sv);

    /* during global destruction, mg_obj may already have been freed */
    if (PL_in_clean_all)
	return 0;

    mg = mg_find (mg->mg_obj, PERL_MAGIC_arylen);

    if (mg) {
	/* arylen scalar holds a pointer back to the array, but doesn't own a
	   reference. Hence the we (the array) are about to go away with it
	   still pointing at us. Clear its pointer, else it would be pointing
	   at free memory. See the comment in sv_magic about reference loops,
	   and why it can't own a reference to us.  */
	mg->mg_obj = 0;
    }
    return 0;
}

int
Perl_magic_getpos(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    SV* const lsv = LvTARG(sv);

    PERL_ARGS_ASSERT_MAGIC_GETPOS;
    PERL_UNUSED_ARG(mg);

    if (SvTYPE(lsv) >= SVt_PVMG && SvMAGIC(lsv)) {
	MAGIC * const found = mg_find(lsv, PERL_MAGIC_regex_global);
	if (found && found->mg_len >= 0) {
	    I32 i = found->mg_len;
	    if (DO_UTF8(lsv))
		sv_pos_b2u(lsv, &i);
	    sv_setiv(sv, i);
	    return 0;
	}
    }
    SvOK_off(sv);
    return 0;
}

int
Perl_magic_setpos(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    SV* const lsv = LvTARG(sv);
    SSize_t pos;
    STRLEN len;
    STRLEN ulen = 0;
    MAGIC* found;

    PERL_ARGS_ASSERT_MAGIC_SETPOS;
    PERL_UNUSED_ARG(mg);

    if (SvTYPE(lsv) >= SVt_PVMG && SvMAGIC(lsv))
	found = mg_find(lsv, PERL_MAGIC_regex_global);
    else
	found = NULL;
    if (!found) {
	if (!SvOK(sv))
	    return 0;
#ifdef PERL_OLD_COPY_ON_WRITE
    if (SvIsCOW(lsv))
        sv_force_normal_flags(lsv, 0);
#endif
	found = sv_magicext(lsv, NULL, PERL_MAGIC_regex_global, &PL_vtbl_mglob,
			    NULL, 0);
    }
    else if (!SvOK(sv)) {
	found->mg_len = -1;
	return 0;
    }
    len = SvPOK(lsv) ? SvCUR(lsv) : sv_len(lsv);

    pos = SvIV(sv);

    if (DO_UTF8(lsv)) {
	ulen = sv_len_utf8(lsv);
	if (ulen)
	    len = ulen;
    }

    if (pos < 0) {
	pos += len;
	if (pos < 0)
	    pos = 0;
    }
    else if (pos > (SSize_t)len)
	pos = len;

    if (ulen) {
	I32 p = pos;
	sv_pos_u2b(lsv, &p, 0);
	pos = p;
    }

    found->mg_len = pos;
    found->mg_flags &= ~MGf_MINMATCH;

    return 0;
}

int
Perl_magic_getsubstr(pTHX_ SV *sv, MAGIC *mg)
{
    STRLEN len;
    SV * const lsv = LvTARG(sv);
    const char * const tmps = SvPV_const(lsv,len);
    STRLEN offs = LvTARGOFF(sv);
    STRLEN rem = LvTARGLEN(sv);
    const bool negoff = LvFLAGS(sv) & 1;
    const bool negrem = LvFLAGS(sv) & 2;

    PERL_ARGS_ASSERT_MAGIC_GETSUBSTR;
    PERL_UNUSED_ARG(mg);

    if (!translate_substr_offsets(
	    SvUTF8(lsv) ? sv_len_utf8(lsv) : len,
	    negoff ? -(IV)offs : (IV)offs, !negoff,
	    negrem ? -(IV)rem  : (IV)rem,  !negrem, &offs, &rem
    )) {
	Perl_ck_warner(aTHX_ packWARN(WARN_SUBSTR), "substr outside of string");
	sv_setsv_nomg(sv, &PL_sv_undef);
	return 0;
    }

    if (SvUTF8(lsv))
	offs = sv_pos_u2b_flags(lsv, offs, &rem, SV_CONST_RETURN);
    sv_setpvn(sv, tmps + offs, rem);
    if (SvUTF8(lsv))
        SvUTF8_on(sv);
    return 0;
}

int
Perl_magic_setsubstr(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    STRLEN len, lsv_len, oldtarglen, newtarglen;
    const char * const tmps = SvPV_const(sv, len);
    SV * const lsv = LvTARG(sv);
    STRLEN lvoff = LvTARGOFF(sv);
    STRLEN lvlen = LvTARGLEN(sv);
    const bool negoff = LvFLAGS(sv) & 1;
    const bool neglen = LvFLAGS(sv) & 2;

    PERL_ARGS_ASSERT_MAGIC_SETSUBSTR;
    PERL_UNUSED_ARG(mg);

    SvGETMAGIC(lsv);
    if (SvROK(lsv))
	Perl_ck_warner(aTHX_ packWARN(WARN_SUBSTR),
			    "Attempt to use reference as lvalue in substr"
	);
    if (SvUTF8(lsv)) lsv_len = sv_len_utf8(lsv);
    else (void)SvPV_nomg(lsv,lsv_len);
    if (!translate_substr_offsets(
	    lsv_len,
	    negoff ? -(IV)lvoff : (IV)lvoff, !negoff,
	    neglen ? -(IV)lvlen : (IV)lvlen, !neglen, &lvoff, &lvlen
    ))
	Perl_croak(aTHX_ "substr outside of string");
    oldtarglen = lvlen;
    if (DO_UTF8(sv)) {
	sv_utf8_upgrade(lsv);
	lvoff = sv_pos_u2b_flags(lsv, lvoff, &lvlen, SV_CONST_RETURN);
	sv_insert_flags(lsv, lvoff, lvlen, tmps, len, 0);
	newtarglen = sv_len_utf8(sv);
	SvUTF8_on(lsv);
    }
    else if (lsv && SvUTF8(lsv)) {
	const char *utf8;
	lvoff = sv_pos_u2b_flags(lsv, lvoff, &lvlen, SV_CONST_RETURN);
	newtarglen = len;
	utf8 = (char*)bytes_to_utf8((U8*)tmps, &len);
	sv_insert_flags(lsv, lvoff, lvlen, utf8, len, 0);
	Safefree(utf8);
    }
    else {
	sv_insert_flags(lsv, lvoff, lvlen, tmps, len, 0);
	newtarglen = len;
    }
    if (!neglen) LvTARGLEN(sv) = newtarglen;
    if (negoff)  LvTARGOFF(sv) += newtarglen - oldtarglen;

    return 0;
}

int
Perl_magic_gettaint(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;

    PERL_ARGS_ASSERT_MAGIC_GETTAINT;
    PERL_UNUSED_ARG(sv);

    TAINT_IF((PL_localizing != 1) && (mg->mg_len & 1));
    return 0;
}

int
Perl_magic_settaint(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;

    PERL_ARGS_ASSERT_MAGIC_SETTAINT;
    PERL_UNUSED_ARG(sv);

    /* update taint status */
    if (PL_tainted)
	mg->mg_len |= 1;
    else
	mg->mg_len &= ~1;
    return 0;
}

int
Perl_magic_getvec(pTHX_ SV *sv, MAGIC *mg)
{
    SV * const lsv = LvTARG(sv);

    PERL_ARGS_ASSERT_MAGIC_GETVEC;
    PERL_UNUSED_ARG(mg);

    if (lsv)
	sv_setuv(sv, do_vecget(lsv, LvTARGOFF(sv), LvTARGLEN(sv)));
    else
	SvOK_off(sv);

    return 0;
}

int
Perl_magic_setvec(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_SETVEC;
    PERL_UNUSED_ARG(mg);
    do_vecset(sv);	/* XXX slurp this routine */
    return 0;
}

int
Perl_magic_setvstring(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_SETVSTRING;

    if (SvPOKp(sv)) {
	SV * const vecsv = sv_newmortal();
	scan_vstring(mg->mg_ptr, mg->mg_ptr + mg->mg_len, vecsv);
	if (sv_eq_flags(vecsv, sv, 0 /*nomg*/)) return 0;
    }
    return sv_unmagic(sv, mg->mg_type);
}

int
Perl_magic_getdefelem(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    SV *targ = NULL;

    PERL_ARGS_ASSERT_MAGIC_GETDEFELEM;

    if (LvTARGLEN(sv)) {
	if (mg->mg_obj) {
	    SV * const ahv = LvTARG(sv);
	    HE * const he = hv_fetch_ent(MUTABLE_HV(ahv), mg->mg_obj, FALSE, 0);
            if (he)
                targ = HeVAL(he);
	}
	else {
	    AV *const av = MUTABLE_AV(LvTARG(sv));
	    if ((I32)LvTARGOFF(sv) <= AvFILL(av))
		targ = AvARRAY(av)[LvTARGOFF(sv)];
	}
	if (targ && (targ != &PL_sv_undef)) {
	    /* somebody else defined it for us */
	    SvREFCNT_dec(LvTARG(sv));
	    LvTARG(sv) = SvREFCNT_inc_simple_NN(targ);
	    LvTARGLEN(sv) = 0;
	    SvREFCNT_dec(mg->mg_obj);
	    mg->mg_obj = NULL;
	    mg->mg_flags &= ~MGf_REFCOUNTED;
	}
    }
    else
	targ = LvTARG(sv);
    sv_setsv(sv, targ ? targ : &PL_sv_undef);
    return 0;
}

int
Perl_magic_setdefelem(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_SETDEFELEM;
    PERL_UNUSED_ARG(mg);
    if (LvTARGLEN(sv))
	vivify_defelem(sv);
    if (LvTARG(sv)) {
	sv_setsv(LvTARG(sv), sv);
	SvSETMAGIC(LvTARG(sv));
    }
    return 0;
}

void
Perl_vivify_defelem(pTHX_ SV *sv)
{
    dVAR;
    MAGIC *mg;
    SV *value = NULL;

    PERL_ARGS_ASSERT_VIVIFY_DEFELEM;

    if (!LvTARGLEN(sv) || !(mg = mg_find(sv, PERL_MAGIC_defelem)))
	return;
    if (mg->mg_obj) {
	SV * const ahv = LvTARG(sv);
	HE * const he = hv_fetch_ent(MUTABLE_HV(ahv), mg->mg_obj, TRUE, 0);
        if (he)
            value = HeVAL(he);
	if (!value || value == &PL_sv_undef)
	    Perl_croak(aTHX_ PL_no_helem_sv, SVfARG(mg->mg_obj));
    }
    else {
	AV *const av = MUTABLE_AV(LvTARG(sv));
	if ((I32)LvTARGLEN(sv) < 0 && (I32)LvTARGOFF(sv) > AvFILL(av))
	    LvTARG(sv) = NULL;	/* array can't be extended */
	else {
	    SV* const * const svp = av_fetch(av, LvTARGOFF(sv), TRUE);
	    if (!svp || (value = *svp) == &PL_sv_undef)
		Perl_croak(aTHX_ PL_no_aelem, (I32)LvTARGOFF(sv));
	}
    }
    SvREFCNT_inc_simple_void(value);
    SvREFCNT_dec(LvTARG(sv));
    LvTARG(sv) = value;
    LvTARGLEN(sv) = 0;
    SvREFCNT_dec(mg->mg_obj);
    mg->mg_obj = NULL;
    mg->mg_flags &= ~MGf_REFCOUNTED;
}

int
Perl_magic_killbackrefs(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_KILLBACKREFS;
    Perl_sv_kill_backrefs(aTHX_ sv, MUTABLE_AV(mg->mg_obj));
    return 0;
}

int
Perl_magic_setmglob(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_SETMGLOB;
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(sv);
    mg->mg_len = -1;
    return 0;
}

int
Perl_magic_setuvar(pTHX_ SV *sv, MAGIC *mg)
{
    const struct ufuncs * const uf = (struct ufuncs *)mg->mg_ptr;

    PERL_ARGS_ASSERT_MAGIC_SETUVAR;

    if (uf && uf->uf_set)
	(*uf->uf_set)(aTHX_ uf->uf_index, sv);
    return 0;
}

int
Perl_magic_setregexp(pTHX_ SV *sv, MAGIC *mg)
{
    const char type = mg->mg_type;

    PERL_ARGS_ASSERT_MAGIC_SETREGEXP;

    if (type == PERL_MAGIC_qr) {
    } else if (type == PERL_MAGIC_bm) {
	SvTAIL_off(sv);
	SvVALID_off(sv);
    } else if (type == PERL_MAGIC_study) {
	if (!isGV_with_GP(sv))
	    SvSCREAM_off(sv);
    } else {
	assert(type == PERL_MAGIC_fm);
    }
    return sv_unmagic(sv, type);
}

#ifdef USE_LOCALE_COLLATE
int
Perl_magic_setcollxfrm(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_SETCOLLXFRM;

    /*
     * RenE<eacute> Descartes said "I think not."
     * and vanished with a faint plop.
     */
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(sv);
    if (mg->mg_ptr) {
	Safefree(mg->mg_ptr);
	mg->mg_ptr = NULL;
	mg->mg_len = -1;
    }
    return 0;
}
#endif /* USE_LOCALE_COLLATE */

/* Just clear the UTF-8 cache data. */
int
Perl_magic_setutf8(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_SETUTF8;
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(sv);
    Safefree(mg->mg_ptr);	/* The mg_ptr holds the pos cache. */
    mg->mg_ptr = NULL;
    mg->mg_len = -1;		/* The mg_len holds the len cache. */
    return 0;
}

int
Perl_magic_set(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    register const char *s;
    register I32 paren;
    register const REGEXP * rx;
    const char * const remaining = mg->mg_ptr + 1;
    I32 i;
    STRLEN len;
    MAGIC *tmg;

    PERL_ARGS_ASSERT_MAGIC_SET;

    switch (*mg->mg_ptr) {
    case '\015': /* $^MATCH */
      if (strEQ(remaining, "ATCH"))
          goto do_match;
    case '`': /* ${^PREMATCH} caught below */
      do_prematch:
      paren = RX_BUFF_IDX_PREMATCH;
      goto setparen;
    case '\'': /* ${^POSTMATCH} caught below */
      do_postmatch:
      paren = RX_BUFF_IDX_POSTMATCH;
      goto setparen;
    case '&':
      do_match:
      paren = RX_BUFF_IDX_FULLMATCH;
      goto setparen;
    case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      paren = atoi(mg->mg_ptr);
      setparen:
	if (PL_curpm && (rx = PM_GETRE(PL_curpm))) {
            CALLREG_NUMBUF_STORE((REGEXP * const)rx,paren,sv);
	} else {
            /* Croak with a READONLY error when a numbered match var is
             * set without a previous pattern match. Unless it's C<local $1>
             */
            if (!PL_localizing) {
                Perl_croak_no_modify(aTHX);
            }
        }
        break;
    case '\001':	/* ^A */
	sv_setsv(PL_bodytarget, sv);
	FmLINES(PL_bodytarget) = 0;
	if (SvPOK(PL_bodytarget)) {
	    char *s = SvPVX(PL_bodytarget);
	    while ( ((s = strchr(s, '\n'))) ) {
		FmLINES(PL_bodytarget)++;
		s++;
	    }
	}
	/* mg_set() has temporarily made sv non-magical */
	if (PL_tainting) {
	    if ((tmg = mg_find(sv,PERL_MAGIC_taint)) && tmg->mg_len & 1)
		SvTAINTED_on(PL_bodytarget);
	    else
		SvTAINTED_off(PL_bodytarget);
	}
	break;
    case '\003':	/* ^C */
	PL_minus_c = cBOOL(SvIV(sv));
	break;

    case '\004':	/* ^D */
#ifdef DEBUGGING
	s = SvPV_nolen_const(sv);
	PL_debug = get_debug_opts(&s, 0) | DEBUG_TOP_FLAG;
	if (DEBUG_x_TEST || DEBUG_B_TEST)
	    dump_all_perl(!DEBUG_B_TEST);
#else
	PL_debug = (SvIV(sv)) | DEBUG_TOP_FLAG;
#endif
	break;
    case '\005':  /* ^E */
	if (*(mg->mg_ptr+1) == '\0') {
#ifdef VMS
	    set_vaxc_errno(SvIV(sv));
#else
#  ifdef WIN32
	    SetLastError( SvIV(sv) );
#  else
#    ifdef OS2
	    os2_setsyserrno(SvIV(sv));
#    else
	    /* will anyone ever use this? */
	    SETERRNO(SvIV(sv), 4);
#    endif
#  endif
#endif
	}
	else if (strEQ(mg->mg_ptr+1, "NCODING")) {
	    SvREFCNT_dec(PL_encoding);
	    if (SvOK(sv) || SvGMAGICAL(sv)) {
		PL_encoding = newSVsv(sv);
	    }
	    else {
		PL_encoding = NULL;
	    }
	}
	break;
    case '\006':	/* ^F */
	PL_maxsysfd = SvIV(sv);
	break;
    case '\010':	/* ^H */
	PL_hints = SvIV(sv);
	break;
    case '\011':	/* ^I */ /* NOT \t in EBCDIC */
	Safefree(PL_inplace);
	PL_inplace = SvOK(sv) ? savesvpv(sv) : NULL;
	break;
    case '\017':	/* ^O */
	if (*(mg->mg_ptr+1) == '\0') {
	    Safefree(PL_osname);
	    PL_osname = NULL;
	    if (SvOK(sv)) {
		TAINT_PROPER("assigning to $^O");
		PL_osname = savesvpv(sv);
	    }
	}
	else if (strEQ(mg->mg_ptr, "\017PEN")) {
	    STRLEN len;
	    const char *const start = SvPV(sv, len);
	    const char *out = (const char*)memchr(start, '\0', len);
	    SV *tmp;


	    PL_compiling.cop_hints |= HINT_LEXICAL_IO_IN | HINT_LEXICAL_IO_OUT;
	    PL_hints |= HINT_LEXICAL_IO_IN | HINT_LEXICAL_IO_OUT;

	    /* Opening for input is more common than opening for output, so
	       ensure that hints for input are sooner on linked list.  */
	    tmp = out ? newSVpvn_flags(out + 1, start + len - out - 1,
				       SvUTF8(sv))
		: newSVpvs_flags("", SvUTF8(sv));
	    (void)hv_stores(GvHV(PL_hintgv), "open>", tmp);
	    mg_set(tmp);

	    tmp = newSVpvn_flags(start, out ? (STRLEN)(out - start) : len,
				        SvUTF8(sv));
	    (void)hv_stores(GvHV(PL_hintgv), "open<", tmp);
	    mg_set(tmp);
	}
	break;
    case '\020':	/* ^P */
      if (*remaining == '\0') { /* ^P */
          PL_perldb = SvIV(sv);
          if (PL_perldb && !PL_DBsingle)
              init_debugger();
          break;
      } else if (strEQ(remaining, "REMATCH")) { /* $^PREMATCH */
          goto do_prematch;
      } else if (strEQ(remaining, "OSTMATCH")) { /* $^POSTMATCH */
          goto do_postmatch;
      }
      break;
    case '\024':	/* ^T */
#ifdef BIG_TIME
	PL_basetime = (Time_t)(SvNOK(sv) ? SvNVX(sv) : sv_2nv(sv));
#else
	PL_basetime = (Time_t)SvIV(sv);
#endif
	break;
    case '\025':	/* ^UTF8CACHE */
	 if (strEQ(mg->mg_ptr+1, "TF8CACHE")) {
	     PL_utf8cache = (signed char) sv_2iv(sv);
	 }
	 break;
    case '\027':	/* ^W & $^WARNING_BITS */
	if (*(mg->mg_ptr+1) == '\0') {
	    if ( ! (PL_dowarn & G_WARN_ALL_MASK)) {
	        i = SvIV(sv);
	        PL_dowarn = (PL_dowarn & ~G_WARN_ON)
		    		| (i ? G_WARN_ON : G_WARN_OFF) ;
	    }
	}
	else if (strEQ(mg->mg_ptr+1, "ARNING_BITS")) {
	    if ( ! (PL_dowarn & G_WARN_ALL_MASK)) {
		if (!SvPOK(sv)) {
	            PL_compiling.cop_warnings = pWARN_STD;
		    break;
		}
		{
		    STRLEN len, i;
		    int accumulate = 0 ;
		    int any_fatals = 0 ;
		    const char * const ptr = SvPV_const(sv, len) ;
		    for (i = 0 ; i < len ; ++i) {
		        accumulate |= ptr[i] ;
		        any_fatals |= (ptr[i] & 0xAA) ;
		    }
		    if (!accumulate) {
		        if (!specialWARN(PL_compiling.cop_warnings))
			    PerlMemShared_free(PL_compiling.cop_warnings);
			PL_compiling.cop_warnings = pWARN_NONE;
		    }
		    /* Yuck. I can't see how to abstract this:  */
		    else if (isWARN_on(((STRLEN *)SvPV_nolen_const(sv)) - 1,
				       WARN_ALL) && !any_fatals) {
			if (!specialWARN(PL_compiling.cop_warnings))
			    PerlMemShared_free(PL_compiling.cop_warnings);
	                PL_compiling.cop_warnings = pWARN_ALL;
	                PL_dowarn |= G_WARN_ONCE ;
	            }
                    else {
			STRLEN len;
			const char *const p = SvPV_const(sv, len);

			PL_compiling.cop_warnings
			    = Perl_new_warnings_bitfield(aTHX_ PL_compiling.cop_warnings,
							 p, len);

	                if (isWARN_on(PL_compiling.cop_warnings, WARN_ONCE))
	                    PL_dowarn |= G_WARN_ONCE ;
	            }

		}
	    }
	}
	break;
    case '.':
	if (PL_localizing) {
	    if (PL_localizing == 1)
		SAVESPTR(PL_last_in_gv);
	}
	else if (SvOK(sv) && GvIO(PL_last_in_gv))
	    IoLINES(GvIOp(PL_last_in_gv)) = SvIV(sv);
	break;
    case '^':
	Safefree(IoTOP_NAME(GvIOp(PL_defoutgv)));
	s = IoTOP_NAME(GvIOp(PL_defoutgv)) = savesvpv(sv);
	IoTOP_GV(GvIOp(PL_defoutgv)) =  gv_fetchsv(sv, GV_ADD, SVt_PVIO);
	break;
    case '~':
	Safefree(IoFMT_NAME(GvIOp(PL_defoutgv)));
	s = IoFMT_NAME(GvIOp(PL_defoutgv)) = savesvpv(sv);
	IoFMT_GV(GvIOp(PL_defoutgv)) =  gv_fetchsv(sv, GV_ADD, SVt_PVIO);
	break;
    case '=':
	IoPAGE_LEN(GvIOp(PL_defoutgv)) = (SvIV(sv));
	break;
    case '-':
	IoLINES_LEFT(GvIOp(PL_defoutgv)) = (SvIV(sv));
	if (IoLINES_LEFT(GvIOp(PL_defoutgv)) < 0L)
		IoLINES_LEFT(GvIOp(PL_defoutgv)) = 0L;
	break;
    case '%':
	IoPAGE(GvIOp(PL_defoutgv)) = (SvIV(sv));
	break;
    case '|':
	{
	    IO * const io = GvIO(PL_defoutgv);
	    if(!io)
	      break;
	    if ((SvIV(sv)) == 0)
		IoFLAGS(io) &= ~IOf_FLUSH;
	    else {
		if (!(IoFLAGS(io) & IOf_FLUSH)) {
		    PerlIO *ofp = IoOFP(io);
		    if (ofp)
			(void)PerlIO_flush(ofp);
		    IoFLAGS(io) |= IOf_FLUSH;
		}
	    }
	}
	break;
    case '/':
	SvREFCNT_dec(PL_rs);
	PL_rs = newSVsv(sv);
	break;
    case '\\':
	SvREFCNT_dec(PL_ors_sv);
	if (SvOK(sv) || SvGMAGICAL(sv)) {
	    PL_ors_sv = newSVsv(sv);
	}
	else {
	    PL_ors_sv = NULL;
	}
	break;
    case '[':
	if (SvIV(sv) != 0)
	    Perl_croak(aTHX_ "Assigning non-zero to $[ is no longer possible");
	break;
    case '?':
#ifdef COMPLEX_STATUS
	if (PL_localizing == 2) {
	    SvUPGRADE(sv, SVt_PVLV);
	    PL_statusvalue = LvTARGOFF(sv);
	    PL_statusvalue_vms = LvTARGLEN(sv);
	}
	else
#endif
#ifdef VMSISH_STATUS
	if (VMSISH_STATUS)
	    STATUS_NATIVE_CHILD_SET((U32)SvIV(sv));
	else
#endif
	    STATUS_UNIX_EXIT_SET(SvIV(sv));
	break;
    case '!':
        {
#ifdef VMS
#   define PERL_VMS_BANG vaxc$errno
#else
#   define PERL_VMS_BANG 0
#endif
	SETERRNO(SvIOK(sv) ? SvIVX(sv) : SvOK(sv) ? sv_2iv(sv) : 0,
		 (SvIV(sv) == EVMSERR) ? 4 : PERL_VMS_BANG);
	}
	break;
    case '<':
	{
	const IV new_uid = SvIV(sv);
	PL_delaymagic_uid = new_uid;
	if (PL_delaymagic) {
	    PL_delaymagic |= DM_RUID;
	    break;				/* don't do magic till later */
	}
#ifdef HAS_SETRUID
	(void)setruid((Uid_t)new_uid);
#else
#ifdef HAS_SETREUID
	(void)setreuid((Uid_t)new_uid, (Uid_t)-1);
#else
#ifdef HAS_SETRESUID
      (void)setresuid((Uid_t)new_uid, (Uid_t)-1, (Uid_t)-1);
#else
	if (new_uid == PerlProc_geteuid()) {		/* special case $< = $> */
#ifdef PERL_DARWIN
	    /* workaround for Darwin's setuid peculiarity, cf [perl #24122] */
	    if (new_uid != 0 && PerlProc_getuid() == 0)
		(void)PerlProc_setuid(0);
#endif
	    (void)PerlProc_setuid(new_uid);
	} else {
	    Perl_croak(aTHX_ "setruid() not implemented");
	}
#endif
#endif
#endif
	break;
	}
    case '>':
	{
	const UV new_euid = SvIV(sv);
	PL_delaymagic_euid = new_euid;
	if (PL_delaymagic) {
	    PL_delaymagic |= DM_EUID;
	    break;				/* don't do magic till later */
	}
#ifdef HAS_SETEUID
	(void)seteuid((Uid_t)new_euid);
#else
#ifdef HAS_SETREUID
	(void)setreuid((Uid_t)-1, (Uid_t)new_euid);
#else
#ifdef HAS_SETRESUID
	(void)setresuid((Uid_t)-1, (Uid_t)new_euid, (Uid_t)-1);
#else
	if (new_euid == PerlProc_getuid())		/* special case $> = $< */
	    PerlProc_setuid(new_euid);
	else {
	    Perl_croak(aTHX_ "seteuid() not implemented");
	}
#endif
#endif
#endif
	break;
	}
    case '(':
	{
	const UV new_gid = SvIV(sv);
	PL_delaymagic_gid = new_gid;
	if (PL_delaymagic) {
	    PL_delaymagic |= DM_RGID;
	    break;				/* don't do magic till later */
	}
#ifdef HAS_SETRGID
	(void)setrgid((Gid_t)new_gid);
#else
#ifdef HAS_SETREGID
	(void)setregid((Gid_t)new_gid, (Gid_t)-1);
#else
#ifdef HAS_SETRESGID
      (void)setresgid((Gid_t)new_gid, (Gid_t)-1, (Gid_t) -1);
#else
	if (new_gid == PerlProc_getegid())			/* special case $( = $) */
	    (void)PerlProc_setgid(new_gid);
	else {
	    Perl_croak(aTHX_ "setrgid() not implemented");
	}
#endif
#endif
#endif
	break;
	}
    case ')':
	{
	UV new_egid;
#ifdef HAS_SETGROUPS
	{
	    const char *p = SvPV_const(sv, len);
            Groups_t *gary = NULL;
#ifdef _SC_NGROUPS_MAX
           int maxgrp = sysconf(_SC_NGROUPS_MAX);

           if (maxgrp < 0)
               maxgrp = NGROUPS;
#else
           int maxgrp = NGROUPS;
#endif

            while (isSPACE(*p))
                ++p;
            new_egid = Atol(p);
            for (i = 0; i < maxgrp; ++i) {
                while (*p && !isSPACE(*p))
                    ++p;
                while (isSPACE(*p))
                    ++p;
                if (!*p)
                    break;
                if(!gary)
                    Newx(gary, i + 1, Groups_t);
                else
                    Renew(gary, i + 1, Groups_t);
                gary[i] = Atol(p);
            }
            if (i)
                (void)setgroups(i, gary);
	    Safefree(gary);
	}
#else  /* HAS_SETGROUPS */
	new_egid = SvIV(sv);
#endif /* HAS_SETGROUPS */
	PL_delaymagic_egid = new_egid;
	if (PL_delaymagic) {
	    PL_delaymagic |= DM_EGID;
	    break;				/* don't do magic till later */
	}
#ifdef HAS_SETEGID
	(void)setegid((Gid_t)new_egid);
#else
#ifdef HAS_SETREGID
	(void)setregid((Gid_t)-1, (Gid_t)new_egid);
#else
#ifdef HAS_SETRESGID
	(void)setresgid((Gid_t)-1, (Gid_t)new_egid, (Gid_t)-1);
#else
	if (new_egid == PerlProc_getgid())			/* special case $) = $( */
	    (void)PerlProc_setgid(new_egid);
	else {
	    Perl_croak(aTHX_ "setegid() not implemented");
	}
#endif
#endif
#endif
	break;
	}
    case ':':
	PL_chopset = SvPV_force(sv,len);
	break;
    case '$': /* $$ */
	/* Store the pid in mg->mg_obj so we can tell when a fork has
	   occurred.  mg->mg_obj points to *$ by default, so clear it. */
	if (isGV(mg->mg_obj)) {
	    if (mg->mg_flags & MGf_REFCOUNTED) /* probably never true */
		SvREFCNT_dec(mg->mg_obj);
	    mg->mg_flags |= MGf_REFCOUNTED;
	    mg->mg_obj = newSViv((IV)PerlProc_getpid());
	}
	else sv_setiv(mg->mg_obj, (IV)PerlProc_getpid());
	break;
    case '0':
	LOCK_DOLLARZERO_MUTEX;
#ifdef HAS_SETPROCTITLE
	/* The BSDs don't show the argv[] in ps(1) output, they
	 * show a string from the process struct and provide
	 * the setproctitle() routine to manipulate that. */
	if (PL_origalen != 1) {
	    s = SvPV_const(sv, len);
#   if __FreeBSD_version > 410001
	    /* The leading "-" removes the "perl: " prefix,
	     * but not the "(perl) suffix from the ps(1)
	     * output, because that's what ps(1) shows if the
	     * argv[] is modified. */
	    setproctitle("-%s", s);
#   else	/* old FreeBSDs, NetBSD, OpenBSD, anyBSD */
	    /* This doesn't really work if you assume that
	     * $0 = 'foobar'; will wipe out 'perl' from the $0
	     * because in ps(1) output the result will be like
	     * sprintf("perl: %s (perl)", s)
	     * I guess this is a security feature:
	     * one (a user process) cannot get rid of the original name.
	     * --jhi */
	    setproctitle("%s", s);
#   endif
	}
#elif defined(__hpux) && defined(PSTAT_SETCMD)
	if (PL_origalen != 1) {
	     union pstun un;
	     s = SvPV_const(sv, len);
	     un.pst_command = (char *)s;
	     pstat(PSTAT_SETCMD, un, len, 0, 0);
	}
#else
	if (PL_origalen > 1) {
	    /* PL_origalen is set in perl_parse(). */
	    s = SvPV_force(sv,len);
	    if (len >= (STRLEN)PL_origalen-1) {
		/* Longer than original, will be truncated. We assume that
		 * PL_origalen bytes are available. */
		Copy(s, PL_origargv[0], PL_origalen-1, char);
	    }
	    else {
		/* Shorter than original, will be padded. */
#ifdef PERL_DARWIN
		/* Special case for Mac OS X: see [perl #38868] */
		const int pad = 0;
#else
		/* Is the space counterintuitive?  Yes.
		 * (You were expecting \0?)
		 * Does it work?  Seems to.  (In Linux 2.4.20 at least.)
		 * --jhi */
		const int pad = ' ';
#endif
		Copy(s, PL_origargv[0], len, char);
		PL_origargv[0][len] = 0;
		memset(PL_origargv[0] + len + 1,
		       pad,  PL_origalen - len - 1);
	    }
	    PL_origargv[0][PL_origalen-1] = 0;
	    for (i = 1; i < PL_origargc; i++)
		PL_origargv[i] = 0;
#ifdef HAS_PRCTL_SET_NAME
	    /* Set the legacy process name in addition to the POSIX name on Linux */
	    if (prctl(PR_SET_NAME, (unsigned long)s, 0, 0, 0) != 0) {
		/* diag_listed_as: SKIPME */
		Perl_croak(aTHX_ "Can't set $0 with prctl(): %s", Strerror(errno));
	    }
#endif
	}
#endif
	UNLOCK_DOLLARZERO_MUTEX;
	break;
    }
    return 0;
}

I32
Perl_whichsig_sv(pTHX_ SV *sigsv)
{
    const char *sigpv;
    STRLEN siglen;
    PERL_ARGS_ASSERT_WHICHSIG_SV;
    PERL_UNUSED_CONTEXT;
    sigpv = SvPV_const(sigsv, siglen);
    return whichsig_pvn(sigpv, siglen);
}

I32
Perl_whichsig_pv(pTHX_ const char *sig)
{
    PERL_ARGS_ASSERT_WHICHSIG_PV;
    PERL_UNUSED_CONTEXT;
    return whichsig_pvn(sig, strlen(sig));
}

I32
Perl_whichsig_pvn(pTHX_ const char *sig, STRLEN len)
{
    register char* const* sigv;

    PERL_ARGS_ASSERT_WHICHSIG_PVN;
    PERL_UNUSED_CONTEXT;

    for (sigv = (char* const*)PL_sig_name; *sigv; sigv++)
	if (strlen(*sigv) == len && memEQ(sig,*sigv, len))
	    return PL_sig_num[sigv - (char* const*)PL_sig_name];
#ifdef SIGCLD
    if (memEQs(sig, len, "CHLD"))
	return SIGCLD;
#endif
#ifdef SIGCHLD
    if (memEQs(sig, len, "CLD"))
	return SIGCHLD;
#endif
    return -1;
}

Signal_t
#if defined(HAS_SIGACTION) && defined(SA_SIGINFO)
Perl_sighandler(int sig, siginfo_t *sip, void *uap)
#else
Perl_sighandler(int sig)
#endif
{
#ifdef PERL_GET_SIG_CONTEXT
    dTHXa(PERL_GET_SIG_CONTEXT);
#else
    dTHX;
#endif
    dSP;
    GV *gv = NULL;
    SV *sv = NULL;
    SV * const tSv = PL_Sv;
    CV *cv = NULL;
    OP *myop = PL_op;
    U32 flags = 0;
    XPV * const tXpv = PL_Xpv;
    I32 old_ss_ix = PL_savestack_ix;


    if (!PL_psig_ptr[sig]) {
		PerlIO_printf(Perl_error_log, "Signal SIG%s received, but no signal handler set.\n",
				 PL_sig_name[sig]);
		exit(sig);
	}

    if (PL_signals &  PERL_SIGNALS_UNSAFE_FLAG) {
	/* Max number of items pushed there is 3*n or 4. We cannot fix
	   infinity, so we fix 4 (in fact 5): */
	if (PL_savestack_ix + 15 <= PL_savestack_max) {
	    flags |= 1;
	    PL_savestack_ix += 5;		/* Protect save in progress. */
	    SAVEDESTRUCTOR_X(S_unwind_handler_stack, NULL);
	}
    }
    /* sv_2cv is too complicated, try a simpler variant first: */
    if (!SvROK(PL_psig_ptr[sig]) || !(cv = MUTABLE_CV(SvRV(PL_psig_ptr[sig])))
	|| SvTYPE(cv) != SVt_PVCV) {
	HV *st;
	cv = sv_2cv(PL_psig_ptr[sig], &st, &gv, GV_ADD);
    }

    if (!cv || !CvROOT(cv)) {
	Perl_ck_warner(aTHX_ packWARN(WARN_SIGNAL), "SIG%s handler \"%s\" not defined.\n",
		       PL_sig_name[sig], (gv ? GvENAME(gv)
					  : ((cv && CvGV(cv))
					     ? GvENAME(CvGV(cv))
					     : "__ANON__")));
	goto cleanup;
    }

    sv = PL_psig_name[sig]
	    ? SvREFCNT_inc_NN(PL_psig_name[sig])
	    : newSVpv(PL_sig_name[sig],0);
    flags |= 8;
    SAVEFREESV(sv);

    if (PL_signals &  PERL_SIGNALS_UNSAFE_FLAG) {
	/* make sure our assumption about the size of the SAVEs are correct:
	 * 3 for SAVEDESTRUCTOR_X, 2 for SAVEFREESV */
	assert(old_ss_ix + 2 + ((flags & 1) ? 3+5 : 0)  == PL_savestack_ix);
    }

    PUSHSTACKi(PERLSI_SIGNAL);
    PUSHMARK(SP);
    PUSHs(sv);
#if defined(HAS_SIGACTION) && defined(SA_SIGINFO)
    {
	 struct sigaction oact;

	 if (sigaction(sig, 0, &oact) == 0 && oact.sa_flags & SA_SIGINFO) {
	      if (sip) {
		   HV *sih = newHV();
		   SV *rv  = newRV_noinc(MUTABLE_SV(sih));
		   /* The siginfo fields signo, code, errno, pid, uid,
		    * addr, status, and band are defined by POSIX/SUSv3. */
		   (void)hv_stores(sih, "signo", newSViv(sip->si_signo));
		   (void)hv_stores(sih, "code", newSViv(sip->si_code));
#if 0 /* XXX TODO: Configure scan for the existence of these, but even that does not help if the SA_SIGINFO is not implemented according to the spec. */
		   hv_stores(sih, "errno",      newSViv(sip->si_errno));
		   hv_stores(sih, "status",     newSViv(sip->si_status));
		   hv_stores(sih, "uid",        newSViv(sip->si_uid));
		   hv_stores(sih, "pid",        newSViv(sip->si_pid));
		   hv_stores(sih, "addr",       newSVuv(PTR2UV(sip->si_addr)));
		   hv_stores(sih, "band",       newSViv(sip->si_band));
#endif
		   EXTEND(SP, 2);
		   PUSHs(rv);
		   mPUSHp((char *)sip, sizeof(*sip));
	      }

	 }
    }
#endif
    PUTBACK;

    call_sv(MUTABLE_SV(cv), G_DISCARD|G_EVAL);

    POPSTACK;
    if (SvTRUE(ERRSV)) {
#ifndef PERL_MICRO
	/* Handler "died", for example to get out of a restart-able read().
	 * Before we re-do that on its behalf re-enable the signal which was
	 * blocked by the system when we entered.
	 */
#ifdef HAS_SIGPROCMASK
#if defined(HAS_SIGACTION) && defined(SA_SIGINFO)
       if (sip || uap)
#endif
	{
	    sigset_t set;
	    sigemptyset(&set);
	    sigaddset(&set,sig);
	    sigprocmask(SIG_UNBLOCK, &set, NULL);
	}
#else
	/* Not clear if this will work */
	(void)rsignal(sig, SIG_IGN);
	(void)rsignal(sig, PL_csighandlerp);
#endif
#endif /* !PERL_MICRO */
	die_sv(ERRSV);
    }
cleanup:
    /* pop any of SAVEFREESV, SAVEDESTRUCTOR_X and "save in progress" */
    PL_savestack_ix = old_ss_ix;
    if (flags & 8)
	SvREFCNT_dec(sv);
    PL_op = myop;			/* Apparently not needed... */

    PL_Sv = tSv;			/* Restore global temporaries. */
    PL_Xpv = tXpv;
    return 1;
}


static void
S_restore_magic(pTHX_ const void *p)
{
    dVAR;
    MGS* const mgs = SSPTR(PTR2IV(p), MGS*);
    SV* const sv = mgs->mgs_sv;
    bool bumped;

    if (!sv)
        return;

    if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv))
    {
#ifdef PERL_OLD_COPY_ON_WRITE
	/* While magic was saved (and off) sv_setsv may well have seen
	   this SV as a prime candidate for COW.  */
	if (SvIsCOW(sv))
	    sv_force_normal_flags(sv, 0);
#endif

	if (mgs->mgs_readonly)
	    SvREADONLY_on(sv);
	if (mgs->mgs_magical)
	    SvFLAGS(sv) |= mgs->mgs_magical;
	else
	    mg_magical(sv);
	if (SvGMAGICAL(sv)) {
	    /* downgrade public flags to private,
	       and discard any other private flags */

	    const U32 pubflags = SvFLAGS(sv) & (SVf_IOK|SVf_NOK|SVf_POK);
	    if (pubflags) {
		SvFLAGS(sv) &= ~( pubflags | (SVp_IOK|SVp_NOK|SVp_POK) );
		SvFLAGS(sv) |= ( pubflags << PRIVSHIFT );
	    }
	}
    }

    bumped = mgs->mgs_bumped;
    mgs->mgs_sv = NULL;  /* mark the MGS structure as restored */

    /* If we're still on top of the stack, pop us off.  (That condition
     * will be satisfied if restore_magic was called explicitly, but *not*
     * if it's being called via leave_scope.)
     * The reason for doing this is that otherwise, things like sv_2cv()
     * may leave alloc gunk on the savestack, and some code
     * (e.g. sighandler) doesn't expect that...
     */
    if (PL_savestack_ix == mgs->mgs_ss_ix)
    {
	UV popval = SSPOPUV;
        assert(popval == SAVEt_DESTRUCTOR_X);
        PL_savestack_ix -= 2;
	popval = SSPOPUV;
        assert((popval & SAVE_MASK) == SAVEt_ALLOC);
        PL_savestack_ix -= popval >> SAVE_TIGHT_SHIFT;
    }
    if (bumped) {
	if (SvREFCNT(sv) == 1) {
	    /* We hold the last reference to this SV, which implies that the
	       SV was deleted as a side effect of the routines we called.
	       So artificially keep it alive a bit longer.
	       We avoid turning on the TEMP flag, which can cause the SV's
	       buffer to get stolen (and maybe other stuff). */
	    int was_temp = SvTEMP(sv);
	    sv_2mortal(sv);
	    if (!was_temp) {
		SvTEMP_off(sv);
	    }
	    SvOK_off(sv);
	}
	else
	    SvREFCNT_dec(sv); /* undo the inc in S_save_magic() */
    }
}

/* clean up the mess created by Perl_sighandler().
 * Note that this is only called during an exit in a signal handler;
 * a die is trapped by the call_sv() and the SAVEDESTRUCTOR_X manually
 * skipped over. */

static void
S_unwind_handler_stack(pTHX_ const void *p)
{
    dVAR;
    PERL_UNUSED_ARG(p);

    PL_savestack_ix -= 5; /* Unprotect save in progress. */
}

/*
=for apidoc magic_sethint

Triggered by a store to %^H, records the key/value pair to
C<PL_compiling.cop_hints_hash>.  It is assumed that hints aren't storing
anything that would need a deep copy.  Maybe we should warn if we find a
reference.

=cut
*/
int
Perl_magic_sethint(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;
    SV *key = (mg->mg_len == HEf_SVKEY) ? MUTABLE_SV(mg->mg_ptr)
	: newSVpvn_flags(mg->mg_ptr, mg->mg_len, SVs_TEMP);

    PERL_ARGS_ASSERT_MAGIC_SETHINT;

    /* mg->mg_obj isn't being used.  If needed, it would be possible to store
       an alternative leaf in there, with PL_compiling.cop_hints being used if
       it's NULL. If needed for threads, the alternative could lock a mutex,
       or take other more complex action.  */

    /* Something changed in %^H, so it will need to be restored on scope exit.
       Doing this here saves a lot of doing it manually in perl code (and
       forgetting to do it, and consequent subtle errors.  */
    PL_hints |= HINT_LOCALIZE_HH;
    CopHINTHASH_set(&PL_compiling,
	cophh_store_sv(CopHINTHASH_get(&PL_compiling), key, 0, sv, 0));
    return 0;
}

/*
=for apidoc magic_clearhint

Triggered by a delete from %^H, records the key to
C<PL_compiling.cop_hints_hash>.

=cut
*/
int
Perl_magic_clearhint(pTHX_ SV *sv, MAGIC *mg)
{
    dVAR;

    PERL_ARGS_ASSERT_MAGIC_CLEARHINT;
    PERL_UNUSED_ARG(sv);

    assert(mg->mg_len == HEf_SVKEY);

    PERL_UNUSED_ARG(sv);

    PL_hints |= HINT_LOCALIZE_HH;
    CopHINTHASH_set(&PL_compiling,
	cophh_delete_sv(CopHINTHASH_get(&PL_compiling),
				 MUTABLE_SV(mg->mg_ptr), 0, 0));
    return 0;
}

/*
=for apidoc magic_clearhints

Triggered by clearing %^H, resets C<PL_compiling.cop_hints_hash>.

=cut
*/
int
Perl_magic_clearhints(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_ARGS_ASSERT_MAGIC_CLEARHINTS;
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(mg);
    cophh_free(CopHINTHASH_get(&PL_compiling));
    CopHINTHASH_set(&PL_compiling, cophh_new_empty());
    return 0;
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
