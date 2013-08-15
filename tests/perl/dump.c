/*    dump.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *  'You have talked long in your sleep, Frodo,' said Gandalf gently, 'and
 *   it has not been hard for me to read your mind and memory.'
 *
 *     [p.220 of _The Lord of the Rings_, II/i: "Many Meetings"]
 */

/* This file contains utility routines to dump the contents of SV and OP
 * structures, as used by command-line options like -Dt and -Dx, and
 * by Devel::Peek.
 *
 * It also holds the debugging version of the  runops function.
 */

#include "EXTERN.h"
#define PERL_IN_DUMP_C
#include "perl.h"
#include "regcomp.h"
#include "proto.h"


static const char* const svtypenames[SVt_LAST] = {
    "NULL",
    "BIND",
    "IV",
    "NV",
    "PV",
    "PVIV",
    "PVNV",
    "PVMG",
    "REGEXP",
    "PVGV",
    "PVLV",
    "PVAV",
    "PVHV",
    "PVCV",
    "PVFM",
    "PVIO"
};


static const char* const svshorttypenames[SVt_LAST] = {
    "UNDEF",
    "BIND",
    "IV",
    "NV",
    "PV",
    "PVIV",
    "PVNV",
    "PVMG",
    "REGEXP",
    "GV",
    "PVLV",
    "AV",
    "HV",
    "CV",
    "FM",
    "IO"
};

struct flag_to_name {
    U32 flag;
    const char *name;
};

static void
S_append_flags(pTHX_ SV *sv, U32 flags, const struct flag_to_name *start,
	       const struct flag_to_name *const end)
{
    do {
	if (flags & start->flag)
	    sv_catpv(sv, start->name);
    } while (++start < end);
}

#define append_flags(sv, f, flags) \
    S_append_flags(aTHX_ (sv), (f), (flags), C_ARRAY_END(flags))



void
Perl_dump_indent(pTHX_ I32 level, PerlIO *file, const char* pat, ...)
{
    va_list args;
    PERL_ARGS_ASSERT_DUMP_INDENT;
    va_start(args, pat);
    dump_vindent(level, file, pat, &args);
    va_end(args);
}

void
Perl_dump_vindent(pTHX_ I32 level, PerlIO *file, const char* pat, va_list *args)
{
    dVAR;
    PERL_ARGS_ASSERT_DUMP_VINDENT;
    PerlIO_printf(file, "%*s", (int)(level*PL_dumpindent), "");
    PerlIO_vprintf(file, pat, *args);
}

void
Perl_dump_all(pTHX)
{
    dump_all_perl(FALSE);
}

void
Perl_dump_all_perl(pTHX_ bool justperl)
{

    dVAR;
    PerlIO_setlinebuf(Perl_debug_log);
    if (PL_main_root)
	op_dump(PL_main_root);
    dump_packsubs_perl(PL_defstash, justperl);
}

void
Perl_dump_packsubs(pTHX_ const HV *stash)
{
    PERL_ARGS_ASSERT_DUMP_PACKSUBS;
    dump_packsubs_perl(stash, FALSE);
}

void
Perl_dump_packsubs_perl(pTHX_ const HV *stash, bool justperl)
{
    dVAR;
    I32	i;

    PERL_ARGS_ASSERT_DUMP_PACKSUBS_PERL;

    if (!HvARRAY(stash))
	return;
    for (i = 0; i <= (I32) HvMAX(stash); i++) {
        const HE *entry;
	for (entry = HvARRAY(stash)[i]; entry; entry = HeNEXT(entry)) {
	    const GV * const gv = (const GV *)HeVAL(entry);
	    if (SvTYPE(gv) != SVt_PVGV || !GvGP(gv))
		continue;
	    if (GvCVu(gv))
		dump_sub_perl(gv, justperl);
	    if (GvFORM(gv))
		dump_form(gv);
	    if (HeKEY(entry)[HeKLEN(entry)-1] == ':') {
		const HV * const hv = GvHV(gv);
		if (hv && (hv != PL_defstash))
		    dump_packsubs_perl(hv, justperl); /* nested package */
	    }
	}
    }
}

void
Perl_dump_sub(pTHX_ const GV *gv)
{
    PERL_ARGS_ASSERT_DUMP_SUB;
    dump_sub_perl(gv, FALSE);
}

void
Perl_dump_sub_perl(pTHX_ const GV *gv, bool justperl)
{
    SV * sv;

    PERL_ARGS_ASSERT_DUMP_SUB_PERL;

    if (justperl && (CvISXSUB(GvCV(gv)) || !CvROOT(GvCV(gv))))
	return;

    sv = sv_newmortal();
    gv_fullname3(sv, gv, NULL);
    Perl_dump_indent(aTHX_ 0, Perl_debug_log, "\nSUB %s = ", SvPVX_const(sv));
    if (CvISXSUB(GvCV(gv)))
	Perl_dump_indent(aTHX_ 0, Perl_debug_log, "(xsub 0x%"UVxf" %d)\n",
	    PTR2UV(CvXSUB(GvCV(gv))),
	    (int)CvXSUBANY(GvCV(gv)).any_i32);
    else if (CvROOT(GvCV(gv)))
	op_dump(CvROOT(GvCV(gv)));
    else
	Perl_dump_indent(aTHX_ 0, Perl_debug_log, "<undef>\n");
}

void
Perl_dump_form(pTHX_ const GV *gv)
{
    SV * const sv = sv_newmortal();

    PERL_ARGS_ASSERT_DUMP_FORM;

    gv_fullname3(sv, gv, NULL);
    Perl_dump_indent(aTHX_ 0, Perl_debug_log, "\nFORMAT %s = ", SvPVX_const(sv));
    if (CvROOT(GvFORM(gv)))
	op_dump(CvROOT(GvFORM(gv)));
    else
	Perl_dump_indent(aTHX_ 0, Perl_debug_log, "<undef>\n");
}

void
Perl_dump_eval(pTHX)
{
    dVAR;
    op_dump(PL_eval_root);
}


/*
=for apidoc pv_escape

Escapes at most the first "count" chars of pv and puts the results into
dsv such that the size of the escaped string will not exceed "max" chars
and will not contain any incomplete escape sequences.

If flags contains PERL_PV_ESCAPE_QUOTE then any double quotes in the string
will also be escaped.

Normally the SV will be cleared before the escaped string is prepared,
but when PERL_PV_ESCAPE_NOCLEAR is set this will not occur.

If PERL_PV_ESCAPE_UNI is set then the input string is treated as Unicode,
if PERL_PV_ESCAPE_UNI_DETECT is set then the input string is scanned
using C<is_utf8_string()> to determine if it is Unicode.

If PERL_PV_ESCAPE_ALL is set then all input chars will be output
using C<\x01F1> style escapes, otherwise if PERL_PV_ESCAPE_NONASCII is set, only
chars above 127 will be escaped using this style; otherwise, only chars above
255 will be so escaped; other non printable chars will use octal or
common escaped patterns like C<\n>. Otherwise, if PERL_PV_ESCAPE_NOBACKSLASH
then all chars below 255 will be treated as printable and
will be output as literals.

If PERL_PV_ESCAPE_FIRSTCHAR is set then only the first char of the
string will be escaped, regardless of max. If the output is to be in hex,
then it will be returned as a plain hex
sequence. Thus the output will either be a single char,
an octal escape sequence, a special escape like C<\n> or a hex value.

If PERL_PV_ESCAPE_RE is set then the escape char used will be a '%' and
not a '\\'. This is because regexes very often contain backslashed
sequences, whereas '%' is not a particularly common character in patterns.

Returns a pointer to the escaped text as held by dsv.

=cut
*/
#define PV_ESCAPE_OCTBUFSIZE 32

char *
Perl_pv_escape( pTHX_ SV *dsv, char const * const str, 
                const STRLEN count, const STRLEN max, 
                STRLEN * const escaped, const U32 flags ) 
{
    const char esc = (flags & PERL_PV_ESCAPE_RE) ? '%' : '\\';
    const char dq = (flags & PERL_PV_ESCAPE_QUOTE) ? '"' : esc;
    char octbuf[PV_ESCAPE_OCTBUFSIZE] = "%123456789ABCDF";
    STRLEN wrote = 0;    /* chars written so far */
    STRLEN chsize = 0;   /* size of data to be written */
    STRLEN readsize = 1; /* size of data just read */
    bool isuni= flags & PERL_PV_ESCAPE_UNI ? 1 : 0; /* is this Unicode */
    const char *pv  = str;
    const char * const end = pv + count; /* end of string */
    octbuf[0] = esc;

    PERL_ARGS_ASSERT_PV_ESCAPE;

    if (!(flags & PERL_PV_ESCAPE_NOCLEAR)) {
	    /* This won't alter the UTF-8 flag */
	    sv_setpvs(dsv, "");
    }
    
    if ((flags & PERL_PV_ESCAPE_UNI_DETECT) && is_utf8_string((U8*)pv, count))
        isuni = 1;
    
    for ( ; (pv < end && (!max || (wrote < max))) ; pv += readsize ) {
        const UV u= (isuni) ? utf8_to_uvchr_buf((U8*)pv, (U8*) end, &readsize) : (U8)*pv;
        const U8 c = (U8)u & 0xFF;
        
        if ( ( u > 255 )
	  || (flags & PERL_PV_ESCAPE_ALL)
	  || (( u > 127 ) && (flags & PERL_PV_ESCAPE_NONASCII)))
	{
            if (flags & PERL_PV_ESCAPE_FIRSTCHAR) 
                chsize = my_snprintf( octbuf, PV_ESCAPE_OCTBUFSIZE, 
                                      "%"UVxf, u);
            else
                chsize = my_snprintf( octbuf, PV_ESCAPE_OCTBUFSIZE, 
                                      "%cx{%"UVxf"}", esc, u);
        } else if (flags & PERL_PV_ESCAPE_NOBACKSLASH) {
            chsize = 1;            
        } else {         
            if ( (c == dq) || (c == esc) || !isPRINT(c) ) {
	        chsize = 2;
                switch (c) {
                
		case '\\' : /* fallthrough */
		case '%'  : if ( c == esc )  {
		                octbuf[1] = esc;  
		            } else {
		                chsize = 1;
		            }
		            break;
		case '\v' : octbuf[1] = 'v';  break;
		case '\t' : octbuf[1] = 't';  break;
		case '\r' : octbuf[1] = 'r';  break;
		case '\n' : octbuf[1] = 'n';  break;
		case '\f' : octbuf[1] = 'f';  break;
                case '"'  : 
                        if ( dq == '"' ) 
				octbuf[1] = '"';
                        else 
                            chsize = 1;
                        break;
		default:
                        if ( (pv+readsize < end) && isDIGIT((U8)*(pv+readsize)) )
                            chsize = my_snprintf( octbuf, PV_ESCAPE_OCTBUFSIZE, 
                                                  "%c%03o", esc, c);
			else
                            chsize = my_snprintf( octbuf, PV_ESCAPE_OCTBUFSIZE, 
                                                  "%c%o", esc, c);
                }
            } else {
                chsize = 1;
            }
	}
	if ( max && (wrote + chsize > max) ) {
	    break;
        } else if (chsize > 1) {
            sv_catpvn(dsv, octbuf, chsize);
            wrote += chsize;
	} else {
	    /* If PERL_PV_ESCAPE_NOBACKSLASH is set then bytes in the range
	       128-255 can be appended raw to the dsv. If dsv happens to be
	       UTF-8 then we need catpvf to upgrade them for us.
	       Or add a new API call sv_catpvc(). Think about that name, and
	       how to keep it clear that it's unlike the s of catpvs, which is
	       really an array octets, not a string.  */
            Perl_sv_catpvf( aTHX_ dsv, "%c", c);
	    wrote++;
	}
        if ( flags & PERL_PV_ESCAPE_FIRSTCHAR ) 
            break;
    }
    if (escaped != NULL)
        *escaped= pv - str;
    return SvPVX(dsv);
}
/*
=for apidoc pv_pretty

Converts a string into something presentable, handling escaping via
pv_escape() and supporting quoting and ellipses.

If the PERL_PV_PRETTY_QUOTE flag is set then the result will be 
double quoted with any double quotes in the string escaped. Otherwise
if the PERL_PV_PRETTY_LTGT flag is set then the result be wrapped in
angle brackets. 

If the PERL_PV_PRETTY_ELLIPSES flag is set and not all characters in
string were output then an ellipsis C<...> will be appended to the
string. Note that this happens AFTER it has been quoted.

If start_color is non-null then it will be inserted after the opening
quote (if there is one) but before the escaped text. If end_color
is non-null then it will be inserted after the escaped text but before
any quotes or ellipses.

Returns a pointer to the prettified text as held by dsv.

=cut           
*/

char *
Perl_pv_pretty( pTHX_ SV *dsv, char const * const str, const STRLEN count, 
  const STRLEN max, char const * const start_color, char const * const end_color, 
  const U32 flags ) 
{
    const U8 dq = (flags & PERL_PV_PRETTY_QUOTE) ? '"' : '%';
    STRLEN escaped;
 
    PERL_ARGS_ASSERT_PV_PRETTY;
   
    if (!(flags & PERL_PV_PRETTY_NOCLEAR)) {
	    /* This won't alter the UTF-8 flag */
	    sv_setpvs(dsv, "");
    }

    if ( dq == '"' )
        sv_catpvs(dsv, "\"");
    else if ( flags & PERL_PV_PRETTY_LTGT )
        sv_catpvs(dsv, "<");
        
    if ( start_color != NULL ) 
        sv_catpv(dsv, start_color);
    
    pv_escape( dsv, str, count, max, &escaped, flags | PERL_PV_ESCAPE_NOCLEAR );    
    
    if ( end_color != NULL ) 
        sv_catpv(dsv, end_color);

    if ( dq == '"' ) 
	sv_catpvs( dsv, "\"");
    else if ( flags & PERL_PV_PRETTY_LTGT )
        sv_catpvs(dsv, ">");         
    
    if ( (flags & PERL_PV_PRETTY_ELLIPSES) && ( escaped < count ) )
	    sv_catpvs(dsv, "...");
 
    return SvPVX(dsv);
}

/*
=for apidoc pv_display

Similar to

  pv_escape(dsv,pv,cur,pvlim,PERL_PV_ESCAPE_QUOTE);

except that an additional "\0" will be appended to the string when
len > cur and pv[cur] is "\0".

Note that the final string may be up to 7 chars longer than pvlim.

=cut
*/

char *
Perl_pv_display(pTHX_ SV *dsv, const char *pv, STRLEN cur, STRLEN len, STRLEN pvlim)
{
    PERL_ARGS_ASSERT_PV_DISPLAY;

    pv_pretty( dsv, pv, cur, pvlim, NULL, NULL, PERL_PV_PRETTY_DUMP);
    if (len > cur && pv[cur] == '\0')
            sv_catpvs( dsv, "\\0");
    return SvPVX(dsv);
}

char *
Perl_sv_peek(pTHX_ SV *sv)
{
    dVAR;
    SV * const t = sv_newmortal();
    int unref = 0;
    U32 type;

    sv_setpvs(t, "");
  retry:
    if (!sv) {
	sv_catpv(t, "VOID");
	goto finish;
    }
    else if (sv == (const SV *)0x55555555 || SvTYPE(sv) == 'U') {
	sv_catpv(t, "WILD");
	goto finish;
    }
    else if (sv == &PL_sv_undef || sv == &PL_sv_no || sv == &PL_sv_yes || sv == &PL_sv_placeholder) {
	if (sv == &PL_sv_undef) {
	    sv_catpv(t, "SV_UNDEF");
	    if (!(SvFLAGS(sv) & (SVf_OK|SVf_OOK|SVs_OBJECT|
				 SVs_GMG|SVs_SMG|SVs_RMG)) &&
		SvREADONLY(sv))
		goto finish;
	}
	else if (sv == &PL_sv_no) {
	    sv_catpv(t, "SV_NO");
	    if (!(SvFLAGS(sv) & (SVf_ROK|SVf_OOK|SVs_OBJECT|
				 SVs_GMG|SVs_SMG|SVs_RMG)) &&
		!(~SvFLAGS(sv) & (SVf_POK|SVf_NOK|SVf_READONLY|
				  SVp_POK|SVp_NOK)) &&
		SvCUR(sv) == 0 &&
		SvNVX(sv) == 0.0)
		goto finish;
	}
	else if (sv == &PL_sv_yes) {
	    sv_catpv(t, "SV_YES");
	    if (!(SvFLAGS(sv) & (SVf_ROK|SVf_OOK|SVs_OBJECT|
				 SVs_GMG|SVs_SMG|SVs_RMG)) &&
		!(~SvFLAGS(sv) & (SVf_POK|SVf_NOK|SVf_READONLY|
				  SVp_POK|SVp_NOK)) &&
		SvCUR(sv) == 1 &&
		SvPVX_const(sv) && *SvPVX_const(sv) == '1' &&
		SvNVX(sv) == 1.0)
		goto finish;
	}
	else {
	    sv_catpv(t, "SV_PLACEHOLDER");
	    if (!(SvFLAGS(sv) & (SVf_OK|SVf_OOK|SVs_OBJECT|
				 SVs_GMG|SVs_SMG|SVs_RMG)) &&
		SvREADONLY(sv))
		goto finish;
	}
	sv_catpv(t, ":");
    }
    else if (SvREFCNT(sv) == 0) {
	sv_catpv(t, "(");
	unref++;
    }
    else if (DEBUG_R_TEST_) {
	int is_tmp = 0;
	I32 ix;
	/* is this SV on the tmps stack? */
	for (ix=PL_tmps_ix; ix>=0; ix--) {
	    if (PL_tmps_stack[ix] == sv) {
		is_tmp = 1;
		break;
	    }
	}
	if (SvREFCNT(sv) > 1)
	    Perl_sv_catpvf(aTHX_ t, "<%"UVuf"%s>", (UV)SvREFCNT(sv),
		    is_tmp ? "T" : "");
	else if (is_tmp)
	    sv_catpv(t, "<T>");
    }

    if (SvROK(sv)) {
	sv_catpv(t, "\\");
	if (SvCUR(t) + unref > 10) {
	    SvCUR_set(t, unref + 3);
	    *SvEND(t) = '\0';
	    sv_catpv(t, "...");
	    goto finish;
	}
	sv = SvRV(sv);
	goto retry;
    }
    type = SvTYPE(sv);
    if (type == SVt_PVCV) {
	Perl_sv_catpvf(aTHX_ t, "CV(%s)", CvGV(sv) ? GvNAME(CvGV(sv)) : "");
	goto finish;
    } else if (type < SVt_LAST) {
	sv_catpv(t, svshorttypenames[type]);

	if (type == SVt_NULL)
	    goto finish;
    } else {
	sv_catpv(t, "FREED");
	goto finish;
    }

    if (SvPOKp(sv)) {
	if (!SvPVX_const(sv))
	    sv_catpv(t, "(null)");
	else {
	    SV * const tmp = newSVpvs("");
	    sv_catpv(t, "(");
	    if (SvOOK(sv)) {
		STRLEN delta;
		SvOOK_offset(sv, delta);
		Perl_sv_catpvf(aTHX_ t, "[%s]", pv_display(tmp, SvPVX_const(sv)-delta, delta, 0, 127));
	    }
	    Perl_sv_catpvf(aTHX_ t, "%s)", pv_display(tmp, SvPVX_const(sv), SvCUR(sv), SvLEN(sv), 127));
	    if (SvUTF8(sv))
		Perl_sv_catpvf(aTHX_ t, " [UTF8 \"%s\"]",
			       sv_uni_display(tmp, sv, 6 * SvCUR(sv),
					      UNI_DISPLAY_QQ));
	    SvREFCNT_dec(tmp);
	}
    }
    else if (SvNOKp(sv)) {
	STORE_NUMERIC_LOCAL_SET_STANDARD();
	Perl_sv_catpvf(aTHX_ t, "(%"NVgf")",SvNVX(sv));
	RESTORE_NUMERIC_LOCAL();
    }
    else if (SvIOKp(sv)) {
	if (SvIsUV(sv))
	    Perl_sv_catpvf(aTHX_ t, "(%"UVuf")", (UV)SvUVX(sv));
	else
            Perl_sv_catpvf(aTHX_ t, "(%"IVdf")", (IV)SvIVX(sv));
    }
    else
	sv_catpv(t, "()");

  finish:
    while (unref--)
	sv_catpv(t, ")");
    if (PL_tainting && SvTAINTED(sv))
	sv_catpv(t, " [tainted]");
    return SvPV_nolen(t);
}

void
Perl_do_pmop_dump(pTHX_ I32 level, PerlIO *file, const PMOP *pm)
{
    char ch;

    PERL_ARGS_ASSERT_DO_PMOP_DUMP;

    if (!pm) {
	Perl_dump_indent(aTHX_ level, file, "{}\n");
	return;
    }
    Perl_dump_indent(aTHX_ level, file, "{\n");
    level++;
    if (pm->op_pmflags & PMf_ONCE)
	ch = '?';
    else
	ch = '/';
    if (PM_GETRE(pm))
	Perl_dump_indent(aTHX_ level, file, "PMf_PRE %c%s%c%s\n",
	     ch, RX_PRECOMP(PM_GETRE(pm)), ch,
	     (pm->op_private & OPpRUNTIME) ? " (RUNTIME)" : "");
    else
	Perl_dump_indent(aTHX_ level, file, "PMf_PRE (RUNTIME)\n");
    if (pm->op_type != OP_PUSHRE && pm->op_pmreplrootu.op_pmreplroot) {
	Perl_dump_indent(aTHX_ level, file, "PMf_REPL = ");
	op_dump(pm->op_pmreplrootu.op_pmreplroot);
    }
    if (pm->op_pmflags || (PM_GETRE(pm) && RX_CHECK_SUBSTR(PM_GETRE(pm)))) {
	SV * const tmpsv = pm_description(pm);
	Perl_dump_indent(aTHX_ level, file, "PMFLAGS = (%s)\n", SvCUR(tmpsv) ? SvPVX_const(tmpsv) + 1 : "");
	SvREFCNT_dec(tmpsv);
    }

    Perl_dump_indent(aTHX_ level-1, file, "}\n");
}

const struct flag_to_name pmflags_flags_names[] = {
    {PMf_CONST, ",CONST"},
    {PMf_KEEP, ",KEEP"},
    {PMf_GLOBAL, ",GLOBAL"},
    {PMf_CONTINUE, ",CONTINUE"},
    {PMf_RETAINT, ",RETAINT"},
    {PMf_EVAL, ",EVAL"},
    {PMf_NONDESTRUCT, ",NONDESTRUCT"},
};

static SV *
S_pm_description(pTHX_ const PMOP *pm)
{
    SV * const desc = newSVpvs("");
    const REGEXP * const regex = PM_GETRE(pm);
    const U32 pmflags = pm->op_pmflags;

    PERL_ARGS_ASSERT_PM_DESCRIPTION;

    if (pmflags & PMf_ONCE)
	sv_catpv(desc, ",ONCE");
#ifdef USE_ITHREADS
    if (SvREADONLY(PL_regex_pad[pm->op_pmoffset]))
        sv_catpv(desc, ":USED");
#else
    if (pmflags & PMf_USED)
        sv_catpv(desc, ":USED");
#endif

    if (regex) {
        if (RX_EXTFLAGS(regex) & RXf_TAINTED)
            sv_catpv(desc, ",TAINTED");
        if (RX_CHECK_SUBSTR(regex)) {
            if (!(RX_EXTFLAGS(regex) & RXf_NOSCAN))
                sv_catpv(desc, ",SCANFIRST");
            if (RX_EXTFLAGS(regex) & RXf_CHECK_ALL)
                sv_catpv(desc, ",ALL");
        }
        if (RX_EXTFLAGS(regex) & RXf_SKIPWHITE)
            sv_catpv(desc, ",SKIPWHITE");
    }

    append_flags(desc, pmflags, pmflags_flags_names);
    return desc;
}

void
Perl_pmop_dump(pTHX_ PMOP *pm)
{
    do_pmop_dump(0, Perl_debug_log, pm);
}

/* Return a unique integer to represent the address of op o.
 * If it already exists in PL_op_sequence, just return it;
 * otherwise add it.
 *  *** Note that this isn't thread-safe */

STATIC UV
S_sequence_num(pTHX_ const OP *o)
{
    dVAR;
    SV     *op,
          **seq;
    const char *key;
    STRLEN  len;
    if (!o)
	return 0;
    op = newSVuv(PTR2UV(o));
    sv_2mortal(op);
    key = SvPV_const(op, len);
    if (!PL_op_sequence)
	PL_op_sequence = newHV();
    seq = hv_fetch(PL_op_sequence, key, len, 0);
    if (seq)
	return SvUV(*seq);
    (void)hv_store(PL_op_sequence, key, len, newSVuv(++PL_op_seq), 0);
    return PL_op_seq;
}

const struct flag_to_name op_flags_names[] = {
    {OPf_KIDS, ",KIDS"},
    {OPf_PARENS, ",PARENS"},
    {OPf_REF, ",REF"},
    {OPf_MOD, ",MOD"},
    {OPf_STACKED, ",STACKED"},
    {OPf_SPECIAL, ",SPECIAL"}
};

const struct flag_to_name op_trans_names[] = {
    {OPpTRANS_FROM_UTF, ",FROM_UTF"},
    {OPpTRANS_TO_UTF, ",TO_UTF"},
    {OPpTRANS_IDENTICAL, ",IDENTICAL"},
    {OPpTRANS_SQUASH, ",SQUASH"},
    {OPpTRANS_COMPLEMENT, ",COMPLEMENT"},
    {OPpTRANS_GROWS, ",GROWS"},
    {OPpTRANS_DELETE, ",DELETE"}
};

const struct flag_to_name op_entersub_names[] = {
    {OPpENTERSUB_DB, ",DB"},
    {OPpENTERSUB_HASTARG, ",HASTARG"},
    {OPpENTERSUB_AMPER, ",AMPER"},
    {OPpENTERSUB_NOPAREN, ",NOPAREN"},
    {OPpENTERSUB_INARGS, ",INARGS"}
};

const struct flag_to_name op_const_names[] = {
    {OPpCONST_NOVER, ",NOVER"},
    {OPpCONST_SHORTCIRCUIT, ",SHORTCIRCUIT"},
    {OPpCONST_STRICT, ",STRICT"},
    {OPpCONST_ENTERED, ",ENTERED"},
    {OPpCONST_BARE, ",BARE"},
    {OPpCONST_WARNING, ",WARNING"}
};

const struct flag_to_name op_sort_names[] = {
    {OPpSORT_NUMERIC, ",NUMERIC"},
    {OPpSORT_INTEGER, ",INTEGER"},
    {OPpSORT_REVERSE, ",REVERSE"},
    {OPpSORT_INPLACE, ",INPLACE"},
    {OPpSORT_DESCEND, ",DESCEND"},
    {OPpSORT_QSORT, ",QSORT"},
    {OPpSORT_STABLE, ",STABLE"}
};

const struct flag_to_name op_open_names[] = {
    {OPpOPEN_IN_RAW, ",IN_RAW"},
    {OPpOPEN_IN_CRLF, ",IN_CRLF"},
    {OPpOPEN_OUT_RAW, ",OUT_RAW"},
    {OPpOPEN_OUT_CRLF, ",OUT_CRLF"}
};

const struct flag_to_name op_exit_names[] = {
    {OPpEXIT_VMSISH, ",EXIT_VMSISH"},
    {OPpHUSH_VMSISH, ",HUSH_VMSISH"}
};

#define OP_PRIVATE_ONCE(op, flag, name) \
    const struct flag_to_name CAT2(op, _names)[] = {	\
	{(flag), (name)} \
    }

OP_PRIVATE_ONCE(op_aassign, OPpASSIGN_COMMON, ",COMMON");
OP_PRIVATE_ONCE(op_leavesub, OPpREFCOUNTED, ",REFCOUNTED");
OP_PRIVATE_ONCE(op_sassign, OPpASSIGN_BACKWARDS, ",BACKWARDS");
OP_PRIVATE_ONCE(op_repeat, OPpREPEAT_DOLIST, ",DOLIST");
OP_PRIVATE_ONCE(op_reverse, OPpREVERSE_INPLACE, ",INPLACE");
OP_PRIVATE_ONCE(op_rv2cv, OPpLVAL_INTRO, ",INTRO");
OP_PRIVATE_ONCE(op_flip, OPpFLIP_LINENUM, ",LINENUM");
OP_PRIVATE_ONCE(op_gv, OPpEARLY_CV, ",EARLY_CV");
OP_PRIVATE_ONCE(op_list, OPpLIST_GUESSED, ",GUESSED");
OP_PRIVATE_ONCE(op_delete, OPpSLICE, ",SLICE");
OP_PRIVATE_ONCE(op_exists, OPpEXISTS_SUB, ",EXISTS_SUB");
OP_PRIVATE_ONCE(op_die, OPpHUSH_VMSISH, ",HUSH_VMSISH");

struct op_private_by_op {
    U16 op_type;
    U16 len;
    const struct flag_to_name *start;
};

const struct op_private_by_op op_private_names[] = {
    {OP_LEAVESUB, C_ARRAY_LENGTH(op_leavesub_names), op_leavesub_names },
    {OP_LEAVE, C_ARRAY_LENGTH(op_leavesub_names), op_leavesub_names },
    {OP_LEAVESUBLV, C_ARRAY_LENGTH(op_leavesub_names), op_leavesub_names },
    {OP_LEAVEWRITE, C_ARRAY_LENGTH(op_leavesub_names), op_leavesub_names },
    {OP_AASSIGN, C_ARRAY_LENGTH(op_aassign_names), op_aassign_names },
    {OP_DIE, C_ARRAY_LENGTH(op_die_names), op_die_names },
    {OP_DELETE, C_ARRAY_LENGTH(op_delete_names), op_delete_names },
    {OP_EXISTS, C_ARRAY_LENGTH(op_exists_names), op_exists_names },
    {OP_EXIT, C_ARRAY_LENGTH(op_exit_names), op_exit_names },
    {OP_FLIP, C_ARRAY_LENGTH(op_flip_names), op_flip_names },
    {OP_FLOP, C_ARRAY_LENGTH(op_flip_names), op_flip_names },
    {OP_GV, C_ARRAY_LENGTH(op_gv_names), op_gv_names },
    {OP_LIST, C_ARRAY_LENGTH(op_list_names), op_list_names },
    {OP_SASSIGN, C_ARRAY_LENGTH(op_sassign_names), op_sassign_names },
    {OP_REPEAT, C_ARRAY_LENGTH(op_repeat_names), op_repeat_names },
    {OP_RV2CV, C_ARRAY_LENGTH(op_rv2cv_names), op_rv2cv_names },
    {OP_TRANS, C_ARRAY_LENGTH(op_trans_names), op_trans_names },
    {OP_CONST, C_ARRAY_LENGTH(op_const_names), op_const_names },
    {OP_SORT, C_ARRAY_LENGTH(op_sort_names), op_sort_names },
    {OP_OPEN, C_ARRAY_LENGTH(op_open_names), op_open_names },
    {OP_BACKTICK, C_ARRAY_LENGTH(op_open_names), op_open_names }
};

static bool
S_op_private_to_names(pTHX_ SV *tmpsv, U32 optype, U32 op_private) {
    const struct op_private_by_op *start = op_private_names;
    const struct op_private_by_op *const end
	= op_private_names + C_ARRAY_LENGTH(op_private_names);

    /* This is a linear search, but no worse than the code that it replaced.
       It's debugging code - size is more important than speed.  */
    do {
	if (optype == start->op_type) {
	    S_append_flags(aTHX_ tmpsv, op_private, start->start,
			   start->start + start->len);
	    return TRUE;
	}
    } while (++start < end);
    return FALSE;
}

void
Perl_do_op_dump(pTHX_ I32 level, PerlIO *file, const OP *o)
{
    dVAR;
    UV      seq;
    const OPCODE optype = o->op_type;

    PERL_ARGS_ASSERT_DO_OP_DUMP;

    Perl_dump_indent(aTHX_ level, file, "{\n");
    level++;
    seq = sequence_num(o);
    if (seq)
	PerlIO_printf(file, "%-4"UVuf, seq);
    else
	PerlIO_printf(file, "????");
    PerlIO_printf(file,
		  "%*sTYPE = %s  ===> ",
		  (int)(PL_dumpindent*level-4), "", OP_NAME(o));
    if (o->op_next)
	PerlIO_printf(file,
			o->op_type == OP_NULL ? "(%"UVuf")\n" : "%"UVuf"\n",
				sequence_num(o->op_next));
    else
	PerlIO_printf(file, "NULL\n");
    if (o->op_targ) {
	if (optype == OP_NULL) {
	    Perl_dump_indent(aTHX_ level, file, "  (was %s)\n", PL_op_name[o->op_targ]);
	    if (o->op_targ == OP_NEXTSTATE) {
		if (CopLINE(cCOPo))
		    Perl_dump_indent(aTHX_ level, file, "LINE = %"UVuf"\n",
				     (UV)CopLINE(cCOPo));
		if (CopSTASHPV(cCOPo))
		    Perl_dump_indent(aTHX_ level, file, "PACKAGE = \"%s\"\n",
				     CopSTASHPV(cCOPo));
		if (CopLABEL(cCOPo))
		    Perl_dump_indent(aTHX_ level, file, "LABEL = \"%s\"\n",
				     CopLABEL(cCOPo));
	    }
	}
	else
	    Perl_dump_indent(aTHX_ level, file, "TARG = %ld\n", (long)o->op_targ);
    }
#ifdef DUMPADDR
    Perl_dump_indent(aTHX_ level, file, "ADDR = 0x%"UVxf" => 0x%"UVxf"\n", (UV)o, (UV)o->op_next);
#endif
    if (o->op_flags || o->op_latefree || o->op_latefreed || o->op_attached) {
	SV * const tmpsv = newSVpvs("");
	switch (o->op_flags & OPf_WANT) {
	case OPf_WANT_VOID:
	    sv_catpv(tmpsv, ",VOID");
	    break;
	case OPf_WANT_SCALAR:
	    sv_catpv(tmpsv, ",SCALAR");
	    break;
	case OPf_WANT_LIST:
	    sv_catpv(tmpsv, ",LIST");
	    break;
	default:
	    sv_catpv(tmpsv, ",UNKNOWN");
	    break;
	}
	append_flags(tmpsv, o->op_flags, op_flags_names);
	if (o->op_latefree)
	    sv_catpv(tmpsv, ",LATEFREE");
	if (o->op_latefreed)
	    sv_catpv(tmpsv, ",LATEFREED");
	if (o->op_attached)
	    sv_catpv(tmpsv, ",ATTACHED");
	Perl_dump_indent(aTHX_ level, file, "FLAGS = (%s)\n", SvCUR(tmpsv) ? SvPVX_const(tmpsv) + 1 : "");
	SvREFCNT_dec(tmpsv);
    }
    if (o->op_private) {
	SV * const tmpsv = newSVpvs("");
	if (PL_opargs[optype] & OA_TARGLEX) {
	    if (o->op_private & OPpTARGET_MY)
		sv_catpv(tmpsv, ",TARGET_MY");
	}
	else if (optype == OP_ENTERSUB ||
	    optype == OP_RV2SV ||
	    optype == OP_GVSV ||
	    optype == OP_RV2AV ||
	    optype == OP_RV2HV ||
	    optype == OP_RV2GV ||
	    optype == OP_AELEM ||
	    optype == OP_HELEM )
	{
	    if (optype == OP_ENTERSUB) {
		append_flags(tmpsv, o->op_private, op_entersub_names);
	    }
	    else {
		switch (o->op_private & OPpDEREF) {
		case OPpDEREF_SV:
		    sv_catpv(tmpsv, ",SV");
		    break;
		case OPpDEREF_AV:
		    sv_catpv(tmpsv, ",AV");
		    break;
		case OPpDEREF_HV:
		    sv_catpv(tmpsv, ",HV");
		    break;
		}
		if (o->op_private & OPpMAYBE_LVSUB)
		    sv_catpv(tmpsv, ",MAYBE_LVSUB");
	    }

	    if (optype == OP_AELEM || optype == OP_HELEM) {
		if (o->op_private & OPpLVAL_DEFER)
		    sv_catpv(tmpsv, ",LVAL_DEFER");
	    }
	    else {
		if (o->op_private & HINT_STRICT_REFS)
		    sv_catpv(tmpsv, ",STRICT_REFS");
		if (o->op_private & OPpOUR_INTRO)
		    sv_catpv(tmpsv, ",OUR_INTRO");
	    }
	}
	else if (S_op_private_to_names(aTHX_ tmpsv, optype, o->op_private)) {
	}
	else if (PL_check[optype] != Perl_ck_ftst) {
	    if (OP_IS_FILETEST_ACCESS(o->op_type) && o->op_private & OPpFT_ACCESS)
		sv_catpv(tmpsv, ",FT_ACCESS");
	    if (o->op_private & OPpFT_STACKED)
		sv_catpv(tmpsv, ",FT_STACKED");
	}
	if (o->op_flags & OPf_MOD && o->op_private & OPpLVAL_INTRO)
	    sv_catpv(tmpsv, ",INTRO");
	if (SvCUR(tmpsv))
	    Perl_dump_indent(aTHX_ level, file, "PRIVATE = (%s)\n", SvPVX_const(tmpsv) + 1);
	SvREFCNT_dec(tmpsv);
    }

#ifdef PERL_MAD
    if (PL_madskills && o->op_madprop) {
	SV * const tmpsv = newSVpvs("");
	MADPROP* mp = o->op_madprop;
	Perl_dump_indent(aTHX_ level, file, "MADPROPS = {\n");
	level++;
	while (mp) {
	    const char tmp = mp->mad_key;
	    sv_setpvs(tmpsv,"'");
	    if (tmp)
		sv_catpvn(tmpsv, &tmp, 1);
	    sv_catpv(tmpsv, "'=");
	    switch (mp->mad_type) {
	    case MAD_NULL:
		sv_catpv(tmpsv, "NULL");
		Perl_dump_indent(aTHX_ level, file, "%s\n", SvPVX(tmpsv));
		break;
	    case MAD_PV:
		sv_catpv(tmpsv, "<");
		sv_catpvn(tmpsv, (char*)mp->mad_val, mp->mad_vlen);
		sv_catpv(tmpsv, ">");
		Perl_dump_indent(aTHX_ level, file, "%s\n", SvPVX(tmpsv));
		break;
	    case MAD_OP:
		if ((OP*)mp->mad_val) {
		    Perl_dump_indent(aTHX_ level, file, "%s\n", SvPVX(tmpsv));
		    do_op_dump(level, file, (OP*)mp->mad_val);
		}
		break;
	    default:
		sv_catpv(tmpsv, "(UNK)");
		Perl_dump_indent(aTHX_ level, file, "%s\n", SvPVX(tmpsv));
		break;
	    }
	    mp = mp->mad_next;
	}
	level--;
	Perl_dump_indent(aTHX_ level, file, "}\n");

	SvREFCNT_dec(tmpsv);
    }
#endif

    switch (optype) {
    case OP_AELEMFAST:
    case OP_GVSV:
    case OP_GV:
#ifdef USE_ITHREADS
	Perl_dump_indent(aTHX_ level, file, "PADIX = %" IVdf "\n", (IV)cPADOPo->op_padix);
#else
	if ( ! (o->op_flags & OPf_SPECIAL)) { /* not lexical */
	    if (cSVOPo->op_sv) {
		SV * const tmpsv = newSV(0);
		ENTER;
		SAVEFREESV(tmpsv);
#ifdef PERL_MAD
		/* FIXME - is this making unwarranted assumptions about the
		   UTF-8 cleanliness of the dump file handle?  */
		SvUTF8_on(tmpsv);
#endif
		gv_fullname3(tmpsv, MUTABLE_GV(cSVOPo->op_sv), NULL);
		Perl_dump_indent(aTHX_ level, file, "GV = %s\n",
				 SvPV_nolen_const(tmpsv));
		LEAVE;
	    }
	    else
		Perl_dump_indent(aTHX_ level, file, "GV = NULL\n");
	}
#endif
	break;
    case OP_CONST:
    case OP_HINTSEVAL:
    case OP_METHOD_NAMED:
#ifndef USE_ITHREADS
	/* with ITHREADS, consts are stored in the pad, and the right pad
	 * may not be active here, so skip */
	Perl_dump_indent(aTHX_ level, file, "SV = %s\n", SvPEEK(cSVOPo_sv));
#endif
	break;
    case OP_NEXTSTATE:
    case OP_DBSTATE:
	if (CopLINE(cCOPo))
	    Perl_dump_indent(aTHX_ level, file, "LINE = %"UVuf"\n",
			     (UV)CopLINE(cCOPo));
	if (CopSTASHPV(cCOPo))
	    Perl_dump_indent(aTHX_ level, file, "PACKAGE = \"%s\"\n",
			     CopSTASHPV(cCOPo));
	if (CopLABEL(cCOPo))
	    Perl_dump_indent(aTHX_ level, file, "LABEL = \"%s\"\n",
			     CopLABEL(cCOPo));
	break;
    case OP_ENTERLOOP:
	Perl_dump_indent(aTHX_ level, file, "REDO ===> ");
	if (cLOOPo->op_redoop)
	    PerlIO_printf(file, "%"UVuf"\n", sequence_num(cLOOPo->op_redoop));
	else
	    PerlIO_printf(file, "DONE\n");
	Perl_dump_indent(aTHX_ level, file, "NEXT ===> ");
	if (cLOOPo->op_nextop)
	    PerlIO_printf(file, "%"UVuf"\n", sequence_num(cLOOPo->op_nextop));
	else
	    PerlIO_printf(file, "DONE\n");
	Perl_dump_indent(aTHX_ level, file, "LAST ===> ");
	if (cLOOPo->op_lastop)
	    PerlIO_printf(file, "%"UVuf"\n", sequence_num(cLOOPo->op_lastop));
	else
	    PerlIO_printf(file, "DONE\n");
	break;
    case OP_COND_EXPR:
    case OP_RANGE:
    case OP_MAPWHILE:
    case OP_GREPWHILE:
    case OP_OR:
    case OP_AND:
	Perl_dump_indent(aTHX_ level, file, "OTHER ===> ");
	if (cLOGOPo->op_other)
	    PerlIO_printf(file, "%"UVuf"\n", sequence_num(cLOGOPo->op_other));
	else
	    PerlIO_printf(file, "DONE\n");
	break;
    case OP_PUSHRE:
    case OP_MATCH:
    case OP_QR:
    case OP_SUBST:
	do_pmop_dump(level, file, cPMOPo);
	break;
    case OP_LEAVE:
    case OP_LEAVEEVAL:
    case OP_LEAVESUB:
    case OP_LEAVESUBLV:
    case OP_LEAVEWRITE:
    case OP_SCOPE:
	if (o->op_private & OPpREFCOUNTED)
	    Perl_dump_indent(aTHX_ level, file, "REFCNT = %"UVuf"\n", (UV)o->op_targ);
	break;
    default:
	break;
    }
    if (o->op_flags & OPf_KIDS) {
	OP *kid;
	for (kid = cUNOPo->op_first; kid; kid = kid->op_sibling)
	    do_op_dump(level, file, kid);
    }
    Perl_dump_indent(aTHX_ level-1, file, "}\n");
}

void
Perl_op_dump(pTHX_ const OP *o)
{
    PERL_ARGS_ASSERT_OP_DUMP;
    do_op_dump(0, Perl_debug_log, o);
}

void
Perl_gv_dump(pTHX_ GV *gv)
{
    SV *sv;

    PERL_ARGS_ASSERT_GV_DUMP;

    if (!gv) {
	PerlIO_printf(Perl_debug_log, "{}\n");
	return;
    }
    sv = sv_newmortal();
    PerlIO_printf(Perl_debug_log, "{\n");
    gv_fullname3(sv, gv, NULL);
    Perl_dump_indent(aTHX_ 1, Perl_debug_log, "GV_NAME = %s", SvPVX_const(sv));
    if (gv != GvEGV(gv)) {
	gv_efullname3(sv, GvEGV(gv), NULL);
	Perl_dump_indent(aTHX_ 1, Perl_debug_log, "-> %s", SvPVX_const(sv));
    }
    PerlIO_putc(Perl_debug_log, '\n');
    Perl_dump_indent(aTHX_ 0, Perl_debug_log, "}\n");
}


/* map magic types to the symbolic names
 * (with the PERL_MAGIC_ prefixed stripped)
 */

static const struct { const char type; const char *name; } magic_names[] = {
#include "mg_names.c"
	/* this null string terminates the list */
	{ 0,                         NULL },
};

void
Perl_do_magic_dump(pTHX_ I32 level, PerlIO *file, const MAGIC *mg, I32 nest, I32 maxnest, bool dumpops, STRLEN pvlim)
{
    PERL_ARGS_ASSERT_DO_MAGIC_DUMP;

    for (; mg; mg = mg->mg_moremagic) {
 	Perl_dump_indent(aTHX_ level, file,
			 "  MAGIC = 0x%"UVxf"\n", PTR2UV(mg));
 	if (mg->mg_virtual) {
            const MGVTBL * const v = mg->mg_virtual;
	    if (v >= PL_magic_vtables
		&& v < PL_magic_vtables + magic_vtable_max) {
		const U32 i = v - PL_magic_vtables;
	        Perl_dump_indent(aTHX_ level, file, "    MG_VIRTUAL = &PL_vtbl_%s\n", PL_magic_vtable_names[i]);
	    }
	    else
	        Perl_dump_indent(aTHX_ level, file, "    MG_VIRTUAL = 0x%"UVxf"\n", PTR2UV(v));
        }
	else
	    Perl_dump_indent(aTHX_ level, file, "    MG_VIRTUAL = 0\n");

	if (mg->mg_private)
	    Perl_dump_indent(aTHX_ level, file, "    MG_PRIVATE = %d\n", mg->mg_private);

	{
	    int n;
	    const char *name = NULL;
	    for (n = 0; magic_names[n].name; n++) {
		if (mg->mg_type == magic_names[n].type) {
		    name = magic_names[n].name;
		    break;
		}
	    }
	    if (name)
		Perl_dump_indent(aTHX_ level, file,
				"    MG_TYPE = PERL_MAGIC_%s\n", name);
	    else
		Perl_dump_indent(aTHX_ level, file,
				"    MG_TYPE = UNKNOWN(\\%o)\n", mg->mg_type);
	}

        if (mg->mg_flags) {
            Perl_dump_indent(aTHX_ level, file, "    MG_FLAGS = 0x%02X\n", mg->mg_flags);
	    if (mg->mg_type == PERL_MAGIC_envelem &&
		mg->mg_flags & MGf_TAINTEDDIR)
	        Perl_dump_indent(aTHX_ level, file, "      TAINTEDDIR\n");
	    if (mg->mg_type == PERL_MAGIC_regex_global &&
		mg->mg_flags & MGf_MINMATCH)
	        Perl_dump_indent(aTHX_ level, file, "      MINMATCH\n");
	    if (mg->mg_flags & MGf_REFCOUNTED)
	        Perl_dump_indent(aTHX_ level, file, "      REFCOUNTED\n");
            if (mg->mg_flags & MGf_GSKIP)
	        Perl_dump_indent(aTHX_ level, file, "      GSKIP\n");
	    if (mg->mg_flags & MGf_COPY)
	        Perl_dump_indent(aTHX_ level, file, "      COPY\n");
	    if (mg->mg_flags & MGf_DUP)
	        Perl_dump_indent(aTHX_ level, file, "      DUP\n");
	    if (mg->mg_flags & MGf_LOCAL)
	        Perl_dump_indent(aTHX_ level, file, "      LOCAL\n");
        }
	if (mg->mg_obj) {
	    Perl_dump_indent(aTHX_ level, file, "    MG_OBJ = 0x%"UVxf"\n",
	        PTR2UV(mg->mg_obj));
            if (mg->mg_type == PERL_MAGIC_qr) {
		REGEXP* const re = (REGEXP *)mg->mg_obj;
		SV * const dsv = sv_newmortal();
                const char * const s
		    = pv_pretty(dsv, RX_WRAPPED(re), RX_WRAPLEN(re),
                    60, NULL, NULL,
                    ( PERL_PV_PRETTY_QUOTE | PERL_PV_ESCAPE_RE | PERL_PV_PRETTY_ELLIPSES |
                    (RX_UTF8(re) ? PERL_PV_ESCAPE_UNI : 0))
                );
		Perl_dump_indent(aTHX_ level+1, file, "    PAT = %s\n", s);
		Perl_dump_indent(aTHX_ level+1, file, "    REFCNT = %"IVdf"\n",
			(IV)RX_REFCNT(re));
            }
            if (mg->mg_flags & MGf_REFCOUNTED)
		do_sv_dump(level+2, file, mg->mg_obj, nest+1, maxnest, dumpops, pvlim); /* MG is already +1 */
	}
        if (mg->mg_len)
	    Perl_dump_indent(aTHX_ level, file, "    MG_LEN = %ld\n", (long)mg->mg_len);
        if (mg->mg_ptr) {
	    Perl_dump_indent(aTHX_ level, file, "    MG_PTR = 0x%"UVxf, PTR2UV(mg->mg_ptr));
	    if (mg->mg_len >= 0) {
		if (mg->mg_type != PERL_MAGIC_utf8) {
		    SV * const sv = newSVpvs("");
		    PerlIO_printf(file, " %s", pv_display(sv, mg->mg_ptr, mg->mg_len, 0, pvlim));
		    SvREFCNT_dec(sv);
		}
            }
	    else if (mg->mg_len == HEf_SVKEY) {
		PerlIO_puts(file, " => HEf_SVKEY\n");
		do_sv_dump(level+2, file, MUTABLE_SV(((mg)->mg_ptr)), nest+1,
			   maxnest, dumpops, pvlim); /* MG is already +1 */
		continue;
	    }
	    else if (mg->mg_len == -1 && mg->mg_type == PERL_MAGIC_utf8);
	    else
		PerlIO_puts(
		  file,
		 " ???? - " __FILE__
		 " does not know how to handle this MG_LEN"
		);
            PerlIO_putc(file, '\n');
        }
	if (mg->mg_type == PERL_MAGIC_utf8) {
	    const STRLEN * const cache = (STRLEN *) mg->mg_ptr;
	    if (cache) {
		IV i;
		for (i = 0; i < PERL_MAGIC_UTF8_CACHESIZE; i++)
		    Perl_dump_indent(aTHX_ level, file,
				     "      %2"IVdf": %"UVuf" -> %"UVuf"\n",
				     i,
				     (UV)cache[i * 2],
				     (UV)cache[i * 2 + 1]);
	    }
	}
    }
}

void
Perl_magic_dump(pTHX_ const MAGIC *mg)
{
    do_magic_dump(0, Perl_debug_log, mg, 0, 0, FALSE, 0);
}

void
Perl_do_hv_dump(pTHX_ I32 level, PerlIO *file, const char *name, HV *sv)
{
    const char *hvname;

    PERL_ARGS_ASSERT_DO_HV_DUMP;

    Perl_dump_indent(aTHX_ level, file, "%s = 0x%"UVxf, name, PTR2UV(sv));
    if (sv && (hvname = HvNAME_get(sv)))
    {
	/* we have to use pv_display and HvNAMELEN_get() so that we display the real package
           name which quite legally could contain insane things like tabs, newlines, nulls or
           other scary crap - this should produce sane results - except maybe for unicode package
           names - but we will wait for someone to file a bug on that - demerphq */
        SV * const tmpsv = newSVpvs("");
        PerlIO_printf(file, "\t%s\n", pv_display(tmpsv, hvname, HvNAMELEN_get(sv), 0, 1024));
    }
    else
	PerlIO_putc(file, '\n');
}

void
Perl_do_gv_dump(pTHX_ I32 level, PerlIO *file, const char *name, GV *sv)
{
    PERL_ARGS_ASSERT_DO_GV_DUMP;

    Perl_dump_indent(aTHX_ level, file, "%s = 0x%"UVxf, name, PTR2UV(sv));
    if (sv && GvNAME(sv))
	PerlIO_printf(file, "\t\"%s\"\n", GvNAME(sv));
    else
	PerlIO_putc(file, '\n');
}

void
Perl_do_gvgv_dump(pTHX_ I32 level, PerlIO *file, const char *name, GV *sv)
{
    PERL_ARGS_ASSERT_DO_GVGV_DUMP;

    Perl_dump_indent(aTHX_ level, file, "%s = 0x%"UVxf, name, PTR2UV(sv));
    if (sv && GvNAME(sv)) {
	const char *hvname;
	PerlIO_printf(file, "\t\"");
	if (GvSTASH(sv) && (hvname = HvNAME_get(GvSTASH(sv))))
	    PerlIO_printf(file, "%s\" :: \"", hvname);
	PerlIO_printf(file, "%s\"\n", GvNAME(sv));
    }
    else
	PerlIO_putc(file, '\n');
}

const struct flag_to_name first_sv_flags_names[] = {
    {SVs_TEMP, "TEMP,"},
    {SVs_OBJECT, "OBJECT,"},
    {SVs_GMG, "GMG,"},
    {SVs_SMG, "SMG,"},
    {SVs_RMG, "RMG,"},
    {SVf_IOK, "IOK,"},
    {SVf_NOK, "NOK,"},
    {SVf_POK, "POK,"}
};

const struct flag_to_name second_sv_flags_names[] = {
    {SVf_OOK, "OOK,"},
    {SVf_FAKE, "FAKE,"},
    {SVf_READONLY, "READONLY,"},
    {SVf_BREAK, "BREAK,"},
    {SVf_AMAGIC, "OVERLOAD,"},
    {SVp_IOK, "pIOK,"},
    {SVp_NOK, "pNOK,"},
    {SVp_POK, "pPOK,"}
};

const struct flag_to_name cv_flags_names[] = {
    {CVf_ANON, "ANON,"},
    {CVf_UNIQUE, "UNIQUE,"},
    {CVf_CLONE, "CLONE,"},
    {CVf_CLONED, "CLONED,"},
    {CVf_CONST, "CONST,"},
    {CVf_NODEBUG, "NODEBUG,"},
    {CVf_LVALUE, "LVALUE,"},
    {CVf_METHOD, "METHOD,"},
    {CVf_WEAKOUTSIDE, "WEAKOUTSIDE,"},
    {CVf_CVGV_RC, "CVGV_RC,"},
    {CVf_DYNFILE, "DYNFILE,"},
    {CVf_AUTOLOAD, "AUTOLOAD,"},
    {CVf_ISXSUB, "ISXSUB,"}
};

const struct flag_to_name hv_flags_names[] = {
    {SVphv_SHAREKEYS, "SHAREKEYS,"},
    {SVphv_LAZYDEL, "LAZYDEL,"},
    {SVphv_HASKFLAGS, "HASKFLAGS,"},
    {SVphv_REHASH, "REHASH,"},
    {SVphv_CLONEABLE, "CLONEABLE,"}
};

const struct flag_to_name gp_flags_names[] = {
    {GVf_INTRO, "INTRO,"},
    {GVf_MULTI, "MULTI,"},
    {GVf_ASSUMECV, "ASSUMECV,"},
    {GVf_IN_PAD, "IN_PAD,"}
};

const struct flag_to_name gp_flags_imported_names[] = {
    {GVf_IMPORTED_SV, " SV"},
    {GVf_IMPORTED_AV, " AV"},
    {GVf_IMPORTED_HV, " HV"},
    {GVf_IMPORTED_CV, " CV"},
};

const struct flag_to_name regexp_flags_names[] = {
    {RXf_PMf_MULTILINE,   "PMf_MULTILINE,"},
    {RXf_PMf_SINGLELINE,  "PMf_SINGLELINE,"},
    {RXf_PMf_FOLD,        "PMf_FOLD,"},
    {RXf_PMf_EXTENDED,    "PMf_EXTENDED,"},
    {RXf_PMf_KEEPCOPY,    "PMf_KEEPCOPY,"},
    {RXf_ANCH_BOL,        "ANCH_BOL,"},
    {RXf_ANCH_MBOL,       "ANCH_MBOL,"},
    {RXf_ANCH_SBOL,       "ANCH_SBOL,"},
    {RXf_ANCH_GPOS,       "ANCH_GPOS,"},
    {RXf_GPOS_SEEN,       "GPOS_SEEN,"},
    {RXf_GPOS_FLOAT,      "GPOS_FLOAT,"},
    {RXf_LOOKBEHIND_SEEN, "LOOKBEHIND_SEEN,"},
    {RXf_EVAL_SEEN,       "EVAL_SEEN,"},
    {RXf_CANY_SEEN,       "CANY_SEEN,"},
    {RXf_NOSCAN,          "NOSCAN,"},
    {RXf_CHECK_ALL,       "CHECK_ALL,"},
    {RXf_MATCH_UTF8,      "MATCH_UTF8,"},
    {RXf_USE_INTUIT_NOML, "USE_INTUIT_NOML,"},
    {RXf_USE_INTUIT_ML,   "USE_INTUIT_ML,"},
    {RXf_INTUIT_TAIL,     "INTUIT_TAIL,"},
    {RXf_SPLIT,           "SPLIT,"},
    {RXf_COPY_DONE,       "COPY_DONE,"},
    {RXf_TAINTED_SEEN,    "TAINTED_SEEN,"},
    {RXf_TAINTED,         "TAINTED,"},
    {RXf_START_ONLY,      "START_ONLY,"},
    {RXf_SKIPWHITE,       "SKIPWHITE,"},
    {RXf_WHITE,           "WHITE,"},
    {RXf_NULL,            "NULL,"},
};

void
Perl_do_sv_dump(pTHX_ I32 level, PerlIO *file, SV *sv, I32 nest, I32 maxnest, bool dumpops, STRLEN pvlim)
{
    dVAR;
    SV *d;
    const char *s;
    U32 flags;
    U32 type;

    PERL_ARGS_ASSERT_DO_SV_DUMP;

    if (!sv) {
	Perl_dump_indent(aTHX_ level, file, "SV = 0\n");
	return;
    }

    flags = SvFLAGS(sv);
    type = SvTYPE(sv);

    /* process general SV flags */

    d = Perl_newSVpvf(aTHX_
		   "(0x%"UVxf") at 0x%"UVxf"\n%*s  REFCNT = %"IVdf"\n%*s  FLAGS = (",
		   PTR2UV(SvANY(sv)), PTR2UV(sv),
		   (int)(PL_dumpindent*level), "", (IV)SvREFCNT(sv),
		   (int)(PL_dumpindent*level), "");

    if (!((flags & SVpad_NAME) == SVpad_NAME
	  && (type == SVt_PVMG || type == SVt_PVNV))) {
	if ((flags & SVs_PADMY) && (flags & SVs_PADSTALE))
	    sv_catpv(d, "PADSTALE,");
    }
    if (!((flags & SVpad_NAME) == SVpad_NAME && type == SVt_PVMG)) {
	if (!(flags & SVs_PADMY) && (flags & SVs_PADTMP))
	    sv_catpv(d, "PADTMP,");
	if (flags & SVs_PADMY)	sv_catpv(d, "PADMY,");
    }
    append_flags(d, flags, first_sv_flags_names);
    if (flags & SVf_ROK)  {	
    				sv_catpv(d, "ROK,");
	if (SvWEAKREF(sv))	sv_catpv(d, "WEAKREF,");
    }
    append_flags(d, flags, second_sv_flags_names);
    if (flags & SVp_SCREAM && type != SVt_PVHV && !isGV_with_GP(sv)) {
	if (SvPCS_IMPORTED(sv))
				sv_catpv(d, "PCS_IMPORTED,");
	else
				sv_catpv(d, "SCREAM,");
    }

    /* process type-specific SV flags */

    switch (type) {
    case SVt_PVCV:
    case SVt_PVFM:
	append_flags(d, CvFLAGS(sv), cv_flags_names);
	break;
    case SVt_PVHV:
	append_flags(d, flags, hv_flags_names);
	break;
    case SVt_PVGV:
    case SVt_PVLV:
	if (isGV_with_GP(sv)) {
	    append_flags(d, GvFLAGS(sv), gp_flags_names);
	}
	if (isGV_with_GP(sv) && GvIMPORTED(sv)) {
	    sv_catpv(d, "IMPORT");
	    if (GvIMPORTED(sv) == GVf_IMPORTED)
		sv_catpv(d, "ALL,");
	    else {
		sv_catpv(d, "(");
		append_flags(d, GvFLAGS(sv), gp_flags_imported_names);
		sv_catpv(d, " ),");
	    }
	}
	/* FALL THROUGH */
    default:
    evaled_or_uv:
	if (SvEVALED(sv))	sv_catpv(d, "EVALED,");
	if (SvIsUV(sv) && !(flags & SVf_ROK))	sv_catpv(d, "IsUV,");
	break;
    case SVt_PVMG:
	if (SvTAIL(sv))		sv_catpv(d, "TAIL,");
	if (SvVALID(sv))	sv_catpv(d, "VALID,");
	if (SvPAD_TYPED(sv))	sv_catpv(d, "TYPED,");
	if (SvPAD_OUR(sv))	sv_catpv(d, "OUR,");
	/* FALL THROUGH */
    case SVt_PVNV:
	if (SvPAD_STATE(sv))	sv_catpv(d, "STATE,");
	goto evaled_or_uv;
    case SVt_PVAV:
	break;
    }
    /* SVphv_SHAREKEYS is also 0x20000000 */
    if ((type != SVt_PVHV) && SvUTF8(sv))
        sv_catpv(d, "UTF8");

    if (*(SvEND(d) - 1) == ',') {
        SvCUR_set(d, SvCUR(d) - 1);
	SvPVX(d)[SvCUR(d)] = '\0';
    }
    sv_catpv(d, ")");
    s = SvPVX_const(d);

    /* dump initial SV details */

#ifdef DEBUG_LEAKING_SCALARS
    Perl_dump_indent(aTHX_ level, file,
	"ALLOCATED at %s:%d %s %s (parent 0x%"UVxf"); serial %"UVuf"\n",
	sv->sv_debug_file ? sv->sv_debug_file : "(unknown)",
	sv->sv_debug_line,
	sv->sv_debug_inpad ? "for" : "by",
	sv->sv_debug_optype ? PL_op_name[sv->sv_debug_optype]: "(none)",
	PTR2UV(sv->sv_debug_parent),
	sv->sv_debug_serial
    );
#endif
    Perl_dump_indent(aTHX_ level, file, "SV = ");

    /* Dump SV type */

    if (type < SVt_LAST) {
	PerlIO_printf(file, "%s%s\n", svtypenames[type], s);

	if (type ==  SVt_NULL) {
	    SvREFCNT_dec(d);
	    return;
	}
    } else {
	PerlIO_printf(file, "UNKNOWN(0x%"UVxf") %s\n", (UV)type, s);
	SvREFCNT_dec(d);
	return;
    }

    /* Dump general SV fields */

    if ((type >= SVt_PVIV && type != SVt_PVAV && type != SVt_PVHV
	 && type != SVt_PVCV && type != SVt_PVFM && type != SVt_PVIO
	 && type != SVt_REGEXP && !isGV_with_GP(sv) && !SvVALID(sv))
	|| (type == SVt_IV && !SvROK(sv))) {
	if (SvIsUV(sv)
#ifdef PERL_OLD_COPY_ON_WRITE
	               || SvIsCOW(sv)
#endif
	                             )
	    Perl_dump_indent(aTHX_ level, file, "  UV = %"UVuf, (UV)SvUVX(sv));
	else
	    Perl_dump_indent(aTHX_ level, file, "  IV = %"IVdf, (IV)SvIVX(sv));
#ifdef PERL_OLD_COPY_ON_WRITE
	if (SvIsCOW_shared_hash(sv))
	    PerlIO_printf(file, "  (HASH)");
	else if (SvIsCOW_normal(sv))
	    PerlIO_printf(file, "  (COW from 0x%"UVxf")", (UV)SvUVX(sv));
#endif
	PerlIO_putc(file, '\n');
    }

    if ((type == SVt_PVNV || type == SVt_PVMG)
	&& (SvFLAGS(sv) & SVpad_NAME) == SVpad_NAME) {
	Perl_dump_indent(aTHX_ level, file, "  COP_LOW = %"UVuf"\n",
			 (UV) COP_SEQ_RANGE_LOW(sv));
	Perl_dump_indent(aTHX_ level, file, "  COP_HIGH = %"UVuf"\n",
			 (UV) COP_SEQ_RANGE_HIGH(sv));
    } else if ((type >= SVt_PVNV && type != SVt_PVAV && type != SVt_PVHV
		&& type != SVt_PVCV && type != SVt_PVFM  && type != SVt_REGEXP
		&& type != SVt_PVIO && !isGV_with_GP(sv) && !SvVALID(sv))
	       || type == SVt_NV) {
	STORE_NUMERIC_LOCAL_SET_STANDARD();
	/* %Vg doesn't work? --jhi */
#ifdef USE_LONG_DOUBLE
	Perl_dump_indent(aTHX_ level, file, "  NV = %.*" PERL_PRIgldbl "\n", LDBL_DIG, SvNVX(sv));
#else
	Perl_dump_indent(aTHX_ level, file, "  NV = %.*g\n", DBL_DIG, SvNVX(sv));
#endif
	RESTORE_NUMERIC_LOCAL();
    }

    if (SvROK(sv)) {
	Perl_dump_indent(aTHX_ level, file, "  RV = 0x%"UVxf"\n", PTR2UV(SvRV(sv)));
	if (nest < maxnest)
	    do_sv_dump(level+1, file, SvRV(sv), nest+1, maxnest, dumpops, pvlim);
    }

    if (type < SVt_PV) {
	SvREFCNT_dec(d);
	return;
    }

    if ((type <= SVt_PVLV && !isGV_with_GP(sv)) || type == SVt_PVFM) {
	if (SvPVX_const(sv)) {
	    STRLEN delta;
	    if (SvOOK(sv)) {
		SvOOK_offset(sv, delta);
		Perl_dump_indent(aTHX_ level, file,"  OFFSET = %"UVuf"\n",
				 (UV) delta);
	    } else {
		delta = 0;
	    }
	    Perl_dump_indent(aTHX_ level, file,"  PV = 0x%"UVxf" ", PTR2UV(SvPVX_const(sv)));
	    if (SvOOK(sv)) {
		PerlIO_printf(file, "( %s . ) ",
			      pv_display(d, SvPVX_const(sv) - delta, delta, 0,
					 pvlim));
	    }
	    PerlIO_printf(file, "%s", pv_display(d, SvPVX_const(sv), SvCUR(sv), SvLEN(sv), pvlim));
	    if (SvUTF8(sv)) /* the 6?  \x{....} */
	        PerlIO_printf(file, " [UTF8 \"%s\"]", sv_uni_display(d, sv, 6 * SvCUR(sv), UNI_DISPLAY_QQ));
	    PerlIO_printf(file, "\n");
	    Perl_dump_indent(aTHX_ level, file, "  CUR = %"IVdf"\n", (IV)SvCUR(sv));
	    Perl_dump_indent(aTHX_ level, file, "  LEN = %"IVdf"\n", (IV)SvLEN(sv));
	}
	else
	    Perl_dump_indent(aTHX_ level, file, "  PV = 0\n");
    }

    if (type >= SVt_PVMG) {
	if (type == SVt_PVMG && SvPAD_OUR(sv)) {
	    HV * const ost = SvOURSTASH(sv);
	    if (ost)
		do_hv_dump(level, file, "  OURSTASH", ost);
	} else {
	    if (SvMAGIC(sv))
		do_magic_dump(level, file, SvMAGIC(sv), nest+1, maxnest, dumpops, pvlim);
	}
	if (SvSTASH(sv))
	    do_hv_dump(level, file, "  STASH", SvSTASH(sv));

	if ((type == SVt_PVMG || type == SVt_PVLV) && SvVALID(sv)) {
	    Perl_dump_indent(aTHX_ level, file, "  RARE = %u\n", (U8)BmRARE(sv));
	    Perl_dump_indent(aTHX_ level, file, "  PREVIOUS = %"UVuf"\n", (UV)BmPREVIOUS(sv));
	    Perl_dump_indent(aTHX_ level, file, "  USEFUL = %"IVdf"\n", (IV)BmUSEFUL(sv));
	}
    }

    /* Dump type-specific SV fields */

    switch (type) {
    case SVt_PVAV:
	Perl_dump_indent(aTHX_ level, file, "  ARRAY = 0x%"UVxf, PTR2UV(AvARRAY(sv)));
	if (AvARRAY(sv) != AvALLOC(sv)) {
	    PerlIO_printf(file, " (offset=%"IVdf")\n", (IV)(AvARRAY(sv) - AvALLOC(sv)));
	    Perl_dump_indent(aTHX_ level, file, "  ALLOC = 0x%"UVxf"\n", PTR2UV(AvALLOC(sv)));
	}
	else
	    PerlIO_putc(file, '\n');
	Perl_dump_indent(aTHX_ level, file, "  FILL = %"IVdf"\n", (IV)AvFILLp(sv));
	Perl_dump_indent(aTHX_ level, file, "  MAX = %"IVdf"\n", (IV)AvMAX(sv));
	Perl_dump_indent(aTHX_ level, file, "  ARYLEN = 0x%"UVxf"\n", SvMAGIC(sv) ? PTR2UV(AvARYLEN(sv)) : 0);
	sv_setpvs(d, "");
	if (AvREAL(sv))	sv_catpv(d, ",REAL");
	if (AvREIFY(sv))	sv_catpv(d, ",REIFY");
	Perl_dump_indent(aTHX_ level, file, "  FLAGS = (%s)\n",
			 SvCUR(d) ? SvPVX_const(d) + 1 : "");
	if (nest < maxnest && av_len(MUTABLE_AV(sv)) >= 0) {
	    int count;
	    for (count = 0; count <=  av_len(MUTABLE_AV(sv)) && count < maxnest; count++) {
		SV** const elt = av_fetch(MUTABLE_AV(sv),count,0);

		Perl_dump_indent(aTHX_ level + 1, file, "Elt No. %"IVdf"\n", (IV)count);
		if (elt)
		    do_sv_dump(level+1, file, *elt, nest+1, maxnest, dumpops, pvlim);
	    }
	}
	break;
    case SVt_PVHV:
	Perl_dump_indent(aTHX_ level, file, "  ARRAY = 0x%"UVxf, PTR2UV(HvARRAY(sv)));
	if (HvARRAY(sv) && HvUSEDKEYS(sv)) {
	    /* Show distribution of HEs in the ARRAY */
	    int freq[200];
#define FREQ_MAX ((int)(sizeof freq / sizeof freq[0] - 1))
	    int i;
	    int max = 0;
	    U32 pow2 = 2, keys = HvUSEDKEYS(sv);
	    NV theoret, sum = 0;

	    PerlIO_printf(file, "  (");
	    Zero(freq, FREQ_MAX + 1, int);
	    for (i = 0; (STRLEN)i <= HvMAX(sv); i++) {
		HE* h;
		int count = 0;
                for (h = HvARRAY(sv)[i]; h; h = HeNEXT(h))
		    count++;
		if (count > FREQ_MAX)
		    count = FREQ_MAX;
	        freq[count]++;
	        if (max < count)
		    max = count;
	    }
	    for (i = 0; i <= max; i++) {
		if (freq[i]) {
		    PerlIO_printf(file, "%d%s:%d", i,
				  (i == FREQ_MAX) ? "+" : "",
				  freq[i]);
		    if (i != max)
			PerlIO_printf(file, ", ");
		}
            }
	    PerlIO_putc(file, ')');
	    /* The "quality" of a hash is defined as the total number of
	       comparisons needed to access every element once, relative
	       to the expected number needed for a random hash.

	       The total number of comparisons is equal to the sum of
	       the squares of the number of entries in each bucket.
	       For a random hash of n keys into k buckets, the expected
	       value is
				n + n(n-1)/2k
	    */

	    for (i = max; i > 0; i--) { /* Precision: count down. */
		sum += freq[i] * i * i;
            }
	    while ((keys = keys >> 1))
		pow2 = pow2 << 1;
	    theoret = HvUSEDKEYS(sv);
	    theoret += theoret * (theoret-1)/pow2;
	    PerlIO_putc(file, '\n');
	    Perl_dump_indent(aTHX_ level, file, "  hash quality = %.1"NVff"%%", theoret/sum*100);
	}
	PerlIO_putc(file, '\n');
	Perl_dump_indent(aTHX_ level, file, "  KEYS = %"IVdf"\n", (IV)HvUSEDKEYS(sv));
	Perl_dump_indent(aTHX_ level, file, "  FILL = %"IVdf"\n", (IV)HvFILL(sv));
	Perl_dump_indent(aTHX_ level, file, "  MAX = %"IVdf"\n", (IV)HvMAX(sv));
	Perl_dump_indent(aTHX_ level, file, "  RITER = %"IVdf"\n", (IV)HvRITER_get(sv));
	Perl_dump_indent(aTHX_ level, file, "  EITER = 0x%"UVxf"\n", PTR2UV(HvEITER_get(sv)));
	{
	    MAGIC * const mg = mg_find(sv, PERL_MAGIC_symtab);
	    if (mg && mg->mg_obj) {
		Perl_dump_indent(aTHX_ level, file, "  PMROOT = 0x%"UVxf"\n", PTR2UV(mg->mg_obj));
	    }
	}
	{
	    const char * const hvname = HvNAME_get(sv);
	    if (hvname)
		Perl_dump_indent(aTHX_ level, file, "  NAME = \"%s\"\n", hvname);
	}
	if (SvOOK(sv)) {
	    AV * const backrefs
		= *Perl_hv_backreferences_p(aTHX_ MUTABLE_HV(sv));
	    struct mro_meta * const meta = HvAUX(sv)->xhv_mro_meta;
	    if (HvAUX(sv)->xhv_name_count)
		Perl_dump_indent(aTHX_
		 level, file, "  NAMECOUNT = %"IVdf"\n",
		 (IV)HvAUX(sv)->xhv_name_count
		);
	    if (HvAUX(sv)->xhv_name_u.xhvnameu_name && HvENAME_HEK_NN(sv)) {
		const I32 count = HvAUX(sv)->xhv_name_count;
		if (count) {
		    SV * const names = newSVpvs_flags("", SVs_TEMP);
		    /* The starting point is the first element if count is
		       positive and the second element if count is negative. */
		    HEK *const *hekp = HvAUX(sv)->xhv_name_u.xhvnameu_names
			+ (count < 0 ? 1 : 0);
		    HEK *const *const endp = HvAUX(sv)->xhv_name_u.xhvnameu_names
			+ (count < 0 ? -count : count);
		    while (hekp < endp) {
			if (*hekp) {
			    sv_catpvs(names, ", \"");
			    sv_catpvn(names, HEK_KEY(*hekp), HEK_LEN(*hekp));
			    sv_catpvs(names, "\"");
			} else {
			    /* This should never happen. */
			    sv_catpvs(names, ", (null)");
			}
			++hekp;
		    }
		    Perl_dump_indent(aTHX_
		     level, file, "  ENAME = %s\n", SvPV_nolen(names)+2
		    );
		}
		else
		    Perl_dump_indent(aTHX_
		     level, file, "  ENAME = \"%s\"\n", HvENAME_get(sv)
		    );
	    }
	    if (backrefs) {
		Perl_dump_indent(aTHX_ level, file, "  BACKREFS = 0x%"UVxf"\n",
				 PTR2UV(backrefs));
		do_sv_dump(level+1, file, MUTABLE_SV(backrefs), nest+1, maxnest,
			   dumpops, pvlim);
	    }
	    if (meta) {
		/* FIXME - mro_algs kflags can signal a UTF-8 name.  */
		Perl_dump_indent(aTHX_ level, file, "  MRO_WHICH = \"%.*s\" (0x%"UVxf")\n",
				 (int)meta->mro_which->length,
				 meta->mro_which->name,
				 PTR2UV(meta->mro_which));
		Perl_dump_indent(aTHX_ level, file, "  CACHE_GEN = 0x%"UVxf"\n",
				 (UV)meta->cache_gen);
		Perl_dump_indent(aTHX_ level, file, "  PKG_GEN = 0x%"UVxf"\n",
				 (UV)meta->pkg_gen);
		if (meta->mro_linear_all) {
		    Perl_dump_indent(aTHX_ level, file, "  MRO_LINEAR_ALL = 0x%"UVxf"\n",
				 PTR2UV(meta->mro_linear_all));
		do_sv_dump(level+1, file, MUTABLE_SV(meta->mro_linear_all), nest+1, maxnest,
			   dumpops, pvlim);
		}
		if (meta->mro_linear_current) {
		    Perl_dump_indent(aTHX_ level, file, "  MRO_LINEAR_CURRENT = 0x%"UVxf"\n",
				 PTR2UV(meta->mro_linear_current));
		do_sv_dump(level+1, file, MUTABLE_SV(meta->mro_linear_current), nest+1, maxnest,
			   dumpops, pvlim);
		}
		if (meta->mro_nextmethod) {
		    Perl_dump_indent(aTHX_ level, file, "  MRO_NEXTMETHOD = 0x%"UVxf"\n",
				 PTR2UV(meta->mro_nextmethod));
		do_sv_dump(level+1, file, MUTABLE_SV(meta->mro_nextmethod), nest+1, maxnest,
			   dumpops, pvlim);
		}
		if (meta->isa) {
		    Perl_dump_indent(aTHX_ level, file, "  ISA = 0x%"UVxf"\n",
				 PTR2UV(meta->isa));
		do_sv_dump(level+1, file, MUTABLE_SV(meta->isa), nest+1, maxnest,
			   dumpops, pvlim);
		}
	    }
	}
	if (nest < maxnest) {
	    HV * const hv = MUTABLE_HV(sv);
	    STRLEN i;
	    HE *he;

	    if (HvARRAY(hv)) {
		int count = maxnest - nest;
		for (i=0; i <= HvMAX(hv); i++) {
		    for (he = HvARRAY(hv)[i]; he; he = HeNEXT(he)) {
			U32 hash;
			SV * keysv;
			const char * keypv;
			SV * elt;
		STRLEN len;

			if (count-- <= 0) goto DONEHV;

			hash = HeHASH(he);
			keysv = hv_iterkeysv(he);
			keypv = SvPV_const(keysv, len);
			elt = HeVAL(he);

		Perl_dump_indent(aTHX_ level+1, file, "Elt %s ", pv_display(d, keypv, len, 0, pvlim));
		if (SvUTF8(keysv))
		    PerlIO_printf(file, "[UTF8 \"%s\"] ", sv_uni_display(d, keysv, 6 * SvCUR(keysv), UNI_DISPLAY_QQ));
			if (HvEITER_get(hv) == he)
			    PerlIO_printf(file, "[CURRENT] ");
		if (HeKREHASH(he))
		    PerlIO_printf(file, "[REHASH] ");
			PerlIO_printf(file, "HASH = 0x%"UVxf"\n", (UV) hash);
		do_sv_dump(level+1, file, elt, nest+1, maxnest, dumpops, pvlim);
	    }
		}
	      DONEHV:;
	    }
	}
	break;

    case SVt_PVCV:
	if (CvAUTOLOAD(sv)) {
	    STRLEN len;
	    const char *const name =  SvPV_const(sv, len);
	    Perl_dump_indent(aTHX_ level, file, "  AUTOLOAD = \"%.*s\"\n",
			     (int) len, name);
	}
	if (SvPOK(sv)) {
	    Perl_dump_indent(aTHX_ level, file, "  PROTOTYPE = \"%.*s\"\n",
			     (int) CvPROTOLEN(sv), CvPROTO(sv));
	}
	/* FALL THROUGH */
    case SVt_PVFM:
	do_hv_dump(level, file, "  COMP_STASH", CvSTASH(sv));
	if (!CvISXSUB(sv)) {
	    if (CvSTART(sv)) {
		Perl_dump_indent(aTHX_ level, file,
				 "  START = 0x%"UVxf" ===> %"IVdf"\n",
				 PTR2UV(CvSTART(sv)),
				 (IV)sequence_num(CvSTART(sv)));
	    }
	    Perl_dump_indent(aTHX_ level, file, "  ROOT = 0x%"UVxf"\n",
			     PTR2UV(CvROOT(sv)));
	    if (CvROOT(sv) && dumpops) {
		do_op_dump(level+1, file, CvROOT(sv));
	    }
	} else {
	    SV * const constant = cv_const_sv((const CV *)sv);

	    Perl_dump_indent(aTHX_ level, file, "  XSUB = 0x%"UVxf"\n", PTR2UV(CvXSUB(sv)));

	    if (constant) {
		Perl_dump_indent(aTHX_ level, file, "  XSUBANY = 0x%"UVxf
				 " (CONST SV)\n",
				 PTR2UV(CvXSUBANY(sv).any_ptr));
		do_sv_dump(level+1, file, constant, nest+1, maxnest, dumpops,
			   pvlim);
	    } else {
		Perl_dump_indent(aTHX_ level, file, "  XSUBANY = %"IVdf"\n",
				 (IV)CvXSUBANY(sv).any_i32);
	    }
	}
 	do_gvgv_dump(level, file, "  GVGV::GV", CvGV(sv));
	Perl_dump_indent(aTHX_ level, file, "  FILE = \"%s\"\n", CvFILE(sv));
	if (type == SVt_PVCV)
	    Perl_dump_indent(aTHX_ level, file, "  DEPTH = %"IVdf"\n", (IV)CvDEPTH(sv));
	Perl_dump_indent(aTHX_ level, file, "  FLAGS = 0x%"UVxf"\n", (UV)CvFLAGS(sv));
	Perl_dump_indent(aTHX_ level, file, "  OUTSIDE_SEQ = %"UVuf"\n", (UV)CvOUTSIDE_SEQ(sv));
	if (type == SVt_PVFM)
	    Perl_dump_indent(aTHX_ level, file, "  LINES = %"IVdf"\n", (IV)FmLINES(sv));
	Perl_dump_indent(aTHX_ level, file, "  PADLIST = 0x%"UVxf"\n", PTR2UV(CvPADLIST(sv)));
	if (nest < maxnest) {
	    do_dump_pad(level+1, file, CvPADLIST(sv), 0);
	}
	{
	    const CV * const outside = CvOUTSIDE(sv);
	    Perl_dump_indent(aTHX_ level, file, "  OUTSIDE = 0x%"UVxf" (%s)\n",
			PTR2UV(outside),
			(!outside ? "null"
			 : CvANON(outside) ? "ANON"
			 : (outside == PL_main_cv) ? "MAIN"
			 : CvUNIQUE(outside) ? "UNIQUE"
			 : CvGV(outside) ? GvNAME(CvGV(outside)) : "UNDEFINED"));
	}
	if (nest < maxnest && (CvCLONE(sv) || CvCLONED(sv)))
	    do_sv_dump(level+1, file, MUTABLE_SV(CvOUTSIDE(sv)), nest+1, maxnest, dumpops, pvlim);
	break;

    case SVt_PVGV:
    case SVt_PVLV:
	if (type == SVt_PVLV) {
	    Perl_dump_indent(aTHX_ level, file, "  TYPE = %c\n", LvTYPE(sv));
	    Perl_dump_indent(aTHX_ level, file, "  TARGOFF = %"IVdf"\n", (IV)LvTARGOFF(sv));
	    Perl_dump_indent(aTHX_ level, file, "  TARGLEN = %"IVdf"\n", (IV)LvTARGLEN(sv));
	    Perl_dump_indent(aTHX_ level, file, "  TARG = 0x%"UVxf"\n", PTR2UV(LvTARG(sv)));
	    Perl_dump_indent(aTHX_ level, file, "  FLAGS = %"IVdf"\n", (IV)LvFLAGS(sv));
	    if (LvTYPE(sv) != 't' && LvTYPE(sv) != 'T')
		do_sv_dump(level+1, file, LvTARG(sv), nest+1, maxnest,
		    dumpops, pvlim);
	}
	if (!isGV_with_GP(sv))
	    break;
	Perl_dump_indent(aTHX_ level, file, "  NAME = \"%s\"\n", GvNAME(sv));
	Perl_dump_indent(aTHX_ level, file, "  NAMELEN = %"IVdf"\n", (IV)GvNAMELEN(sv));
	do_hv_dump (level, file, "  GvSTASH", GvSTASH(sv));
	Perl_dump_indent(aTHX_ level, file, "  GP = 0x%"UVxf"\n", PTR2UV(GvGP(sv)));
	if (!GvGP(sv))
	    break;
	Perl_dump_indent(aTHX_ level, file, "    SV = 0x%"UVxf"\n", PTR2UV(GvSV(sv)));
	Perl_dump_indent(aTHX_ level, file, "    REFCNT = %"IVdf"\n", (IV)GvREFCNT(sv));
	Perl_dump_indent(aTHX_ level, file, "    IO = 0x%"UVxf"\n", PTR2UV(GvIOp(sv)));
	Perl_dump_indent(aTHX_ level, file, "    FORM = 0x%"UVxf"  \n", PTR2UV(GvFORM(sv)));
	Perl_dump_indent(aTHX_ level, file, "    AV = 0x%"UVxf"\n", PTR2UV(GvAV(sv)));
	Perl_dump_indent(aTHX_ level, file, "    HV = 0x%"UVxf"\n", PTR2UV(GvHV(sv)));
	Perl_dump_indent(aTHX_ level, file, "    CV = 0x%"UVxf"\n", PTR2UV(GvCV(sv)));
	Perl_dump_indent(aTHX_ level, file, "    CVGEN = 0x%"UVxf"\n", (UV)GvCVGEN(sv));
	Perl_dump_indent(aTHX_ level, file, "    LINE = %"IVdf"\n", (IV)GvLINE(sv));
	Perl_dump_indent(aTHX_ level, file, "    FILE = \"%s\"\n", GvFILE(sv));
	Perl_dump_indent(aTHX_ level, file, "    FLAGS = 0x%"UVxf"\n", (UV)GvFLAGS(sv));
	do_gv_dump (level, file, "    EGV", GvEGV(sv));
	break;
    case SVt_PVIO:
	Perl_dump_indent(aTHX_ level, file, "  IFP = 0x%"UVxf"\n", PTR2UV(IoIFP(sv)));
	Perl_dump_indent(aTHX_ level, file, "  OFP = 0x%"UVxf"\n", PTR2UV(IoOFP(sv)));
	Perl_dump_indent(aTHX_ level, file, "  DIRP = 0x%"UVxf"\n", PTR2UV(IoDIRP(sv)));
	Perl_dump_indent(aTHX_ level, file, "  LINES = %"IVdf"\n", (IV)IoLINES(sv));
	Perl_dump_indent(aTHX_ level, file, "  PAGE = %"IVdf"\n", (IV)IoPAGE(sv));
	Perl_dump_indent(aTHX_ level, file, "  PAGE_LEN = %"IVdf"\n", (IV)IoPAGE_LEN(sv));
	Perl_dump_indent(aTHX_ level, file, "  LINES_LEFT = %"IVdf"\n", (IV)IoLINES_LEFT(sv));
        if (IoTOP_NAME(sv))
            Perl_dump_indent(aTHX_ level, file, "  TOP_NAME = \"%s\"\n", IoTOP_NAME(sv));
	if (!IoTOP_GV(sv) || SvTYPE(IoTOP_GV(sv)) == SVt_PVGV)
	    do_gv_dump (level, file, "  TOP_GV", IoTOP_GV(sv));
	else {
	    Perl_dump_indent(aTHX_ level, file, "  TOP_GV = 0x%"UVxf"\n",
			     PTR2UV(IoTOP_GV(sv)));
	    do_sv_dump (level+1, file, MUTABLE_SV(IoTOP_GV(sv)), nest+1,
			maxnest, dumpops, pvlim);
	}
	/* Source filters hide things that are not GVs in these three, so let's
	   be careful out there.  */
        if (IoFMT_NAME(sv))
            Perl_dump_indent(aTHX_ level, file, "  FMT_NAME = \"%s\"\n", IoFMT_NAME(sv));
	if (!IoFMT_GV(sv) || SvTYPE(IoFMT_GV(sv)) == SVt_PVGV)
	    do_gv_dump (level, file, "  FMT_GV", IoFMT_GV(sv));
	else {
	    Perl_dump_indent(aTHX_ level, file, "  FMT_GV = 0x%"UVxf"\n",
			     PTR2UV(IoFMT_GV(sv)));
	    do_sv_dump (level+1, file, MUTABLE_SV(IoFMT_GV(sv)), nest+1,
			maxnest, dumpops, pvlim);
	}
        if (IoBOTTOM_NAME(sv))
            Perl_dump_indent(aTHX_ level, file, "  BOTTOM_NAME = \"%s\"\n", IoBOTTOM_NAME(sv));
	if (!IoBOTTOM_GV(sv) || SvTYPE(IoBOTTOM_GV(sv)) == SVt_PVGV)
	    do_gv_dump (level, file, "  BOTTOM_GV", IoBOTTOM_GV(sv));
	else {
	    Perl_dump_indent(aTHX_ level, file, "  BOTTOM_GV = 0x%"UVxf"\n",
			     PTR2UV(IoBOTTOM_GV(sv)));
	    do_sv_dump (level+1, file, MUTABLE_SV(IoBOTTOM_GV(sv)), nest+1,
			maxnest, dumpops, pvlim);
	}
	if (isPRINT(IoTYPE(sv)))
            Perl_dump_indent(aTHX_ level, file, "  TYPE = '%c'\n", IoTYPE(sv));
	else
            Perl_dump_indent(aTHX_ level, file, "  TYPE = '\\%o'\n", IoTYPE(sv));
	Perl_dump_indent(aTHX_ level, file, "  FLAGS = 0x%"UVxf"\n", (UV)IoFLAGS(sv));
	break;
    case SVt_REGEXP:
	{
	    struct regexp * const r = (struct regexp *)SvANY(sv);
	    flags = RX_EXTFLAGS((REGEXP*)sv);
	    sv_setpv(d,"");
	    append_flags(d, flags, regexp_flags_names);
	    if (*(SvEND(d) - 1) == ',') {
		SvCUR_set(d, SvCUR(d) - 1);
		SvPVX(d)[SvCUR(d)] = '\0';
	    }
	    Perl_dump_indent(aTHX_ level, file, "  EXTFLAGS = 0x%"UVxf" (%s)\n",
				(UV)flags, SvPVX_const(d));
	    Perl_dump_indent(aTHX_ level, file, "  INTFLAGS = 0x%"UVxf"\n",
				(UV)(r->intflags));
	    Perl_dump_indent(aTHX_ level, file, "  NPARENS = %"UVuf"\n",
				(UV)(r->nparens));
	    Perl_dump_indent(aTHX_ level, file, "  LASTPAREN = %"UVuf"\n",
				(UV)(r->lastparen));
	    Perl_dump_indent(aTHX_ level, file, "  LASTCLOSEPAREN = %"UVuf"\n",
				(UV)(r->lastcloseparen));
	    Perl_dump_indent(aTHX_ level, file, "  MINLEN = %"IVdf"\n",
				(IV)(r->minlen));
	    Perl_dump_indent(aTHX_ level, file, "  MINLENRET = %"IVdf"\n",
				(IV)(r->minlenret));
	    Perl_dump_indent(aTHX_ level, file, "  GOFS = %"UVuf"\n",
				(UV)(r->gofs));
	    Perl_dump_indent(aTHX_ level, file, "  PRE_PREFIX = %"UVuf"\n",
				(UV)(r->pre_prefix));
	    Perl_dump_indent(aTHX_ level, file, "  SEEN_EVALS = %"UVuf"\n",
				(UV)(r->seen_evals));
	    Perl_dump_indent(aTHX_ level, file, "  SUBLEN = %"IVdf"\n",
				(IV)(r->sublen));
	    if (r->subbeg)
		Perl_dump_indent(aTHX_ level, file, "  SUBBEG = 0x%"UVxf" %s\n",
			    PTR2UV(r->subbeg),
			    pv_display(d, r->subbeg, r->sublen, 50, pvlim));
	    else
		Perl_dump_indent(aTHX_ level, file, "  SUBBEG = 0x0\n");
	    Perl_dump_indent(aTHX_ level, file, "  ENGINE = 0x%"UVxf"\n",
				PTR2UV(r->engine));
	    Perl_dump_indent(aTHX_ level, file, "  MOTHER_RE = 0x%"UVxf"\n",
				PTR2UV(r->mother_re));
	    Perl_dump_indent(aTHX_ level, file, "  PAREN_NAMES = 0x%"UVxf"\n",
				PTR2UV(r->paren_names));
	    Perl_dump_indent(aTHX_ level, file, "  SUBSTRS = 0x%"UVxf"\n",
				PTR2UV(r->substrs));
	    Perl_dump_indent(aTHX_ level, file, "  PPRIVATE = 0x%"UVxf"\n",
				PTR2UV(r->pprivate));
	    Perl_dump_indent(aTHX_ level, file, "  OFFS = 0x%"UVxf"\n",
				PTR2UV(r->offs));
#ifdef PERL_OLD_COPY_ON_WRITE
	    Perl_dump_indent(aTHX_ level, file, "  SAVED_COPY = 0x%"UVxf"\n",
				PTR2UV(r->saved_copy));
#endif
	}
	break;
    }
    SvREFCNT_dec(d);
}

void
Perl_sv_dump(pTHX_ SV *sv)
{
    dVAR;

    PERL_ARGS_ASSERT_SV_DUMP;

    if (SvROK(sv))
	do_sv_dump(0, Perl_debug_log, sv, 0, 4, 0, 0);
    else
	do_sv_dump(0, Perl_debug_log, sv, 0, 0, 0, 0);
}

int
Perl_runops_debug(pTHX)
{
    dVAR;
    if (!PL_op) {
	Perl_ck_warner_d(aTHX_ packWARN(WARN_DEBUGGING), "NULL OP IN RUN");
	return 0;
    }

    DEBUG_l(Perl_deb(aTHX_ "Entering new RUNOPS level\n"));
    do {
	if (PL_debug) {
	    if (PL_watchaddr && (*PL_watchaddr != PL_watchok))
		PerlIO_printf(Perl_debug_log,
			      "WARNING: %"UVxf" changed from %"UVxf" to %"UVxf"\n",
			      PTR2UV(PL_watchaddr), PTR2UV(PL_watchok),
			      PTR2UV(*PL_watchaddr));
	    if (DEBUG_s_TEST_) {
		if (DEBUG_v_TEST_) {
		    PerlIO_printf(Perl_debug_log, "\n");
		    deb_stack_all();
		}
		else
		    debstack();
	    }


	    if (DEBUG_t_TEST_) debop(PL_op);
	    if (DEBUG_P_TEST_) debprof(PL_op);
	}
    } while ((PL_op = PL_op->op_ppaddr(aTHX)));
    DEBUG_l(Perl_deb(aTHX_ "leaving RUNOPS level\n"));

    TAINT_NOT;
    return 0;
}

I32
Perl_debop(pTHX_ const OP *o)
{
    dVAR;

    PERL_ARGS_ASSERT_DEBOP;

    if (CopSTASH_eq(PL_curcop, PL_debstash) && !DEBUG_J_TEST_)
	return 0;

    Perl_deb(aTHX_ "%s", OP_NAME(o));
    switch (o->op_type) {
    case OP_CONST:
    case OP_HINTSEVAL:
	/* With ITHREADS, consts are stored in the pad, and the right pad
	 * may not be active here, so check.
	 * Looks like only during compiling the pads are illegal.
	 */
#ifdef USE_ITHREADS
	if ((((SVOP*)o)->op_sv) || !IN_PERL_COMPILETIME)
#endif
	    PerlIO_printf(Perl_debug_log, "(%s)", SvPEEK(cSVOPo_sv));
	break;
    case OP_GVSV:
    case OP_GV:
	if (cGVOPo_gv) {
	    SV * const sv = newSV(0);
#ifdef PERL_MAD
	    /* FIXME - is this making unwarranted assumptions about the
	       UTF-8 cleanliness of the dump file handle?  */
	    SvUTF8_on(sv);
#endif
	    gv_fullname3(sv, cGVOPo_gv, NULL);
	    PerlIO_printf(Perl_debug_log, "(%s)", SvPV_nolen_const(sv));
	    SvREFCNT_dec(sv);
	}
	else
	    PerlIO_printf(Perl_debug_log, "(NULL)");
	break;
    case OP_PADSV:
    case OP_PADAV:
    case OP_PADHV:
	{
	/* print the lexical's name */
	CV * const cv = deb_curcv(cxstack_ix);
	SV *sv;
        if (cv) {
	    AV * const padlist = CvPADLIST(cv);
            AV * const comppad = MUTABLE_AV(*av_fetch(padlist, 0, FALSE));
            sv = *av_fetch(comppad, o->op_targ, FALSE);
        } else
            sv = NULL;
        if (sv)
	    PerlIO_printf(Perl_debug_log, "(%s)", SvPV_nolen_const(sv));
        else
	    PerlIO_printf(Perl_debug_log, "[%"UVuf"]", (UV)o->op_targ);
	}
        break;
    default:
	break;
    }
    PerlIO_printf(Perl_debug_log, "\n");
    return 0;
}

STATIC CV*
S_deb_curcv(pTHX_ const I32 ix)
{
    dVAR;
    const PERL_CONTEXT * const cx = &cxstack[ix];
    if (CxTYPE(cx) == CXt_SUB || CxTYPE(cx) == CXt_FORMAT)
        return cx->blk_sub.cv;
    else if (CxTYPE(cx) == CXt_EVAL && !CxTRYBLOCK(cx))
        return cx->blk_eval.cv;
    else if (ix == 0 && PL_curstackinfo->si_type == PERLSI_MAIN)
        return PL_main_cv;
    else if (ix <= 0)
        return NULL;
    else
        return deb_curcv(ix - 1);
}

void
Perl_watch(pTHX_ char **addr)
{
    dVAR;

    PERL_ARGS_ASSERT_WATCH;

    PL_watchaddr = addr;
    PL_watchok = *addr;
    PerlIO_printf(Perl_debug_log, "WATCHING, %"UVxf" is currently %"UVxf"\n",
	PTR2UV(PL_watchaddr), PTR2UV(PL_watchok));
}

STATIC void
S_debprof(pTHX_ const OP *o)
{
    dVAR;

    PERL_ARGS_ASSERT_DEBPROF;

    if (!DEBUG_J_TEST_ && CopSTASH_eq(PL_curcop, PL_debstash))
	return;
    if (!PL_profiledata)
	Newxz(PL_profiledata, MAXO, U32);
    ++PL_profiledata[o->op_type];
}

void
Perl_debprofdump(pTHX)
{
    dVAR;
    unsigned i;
    if (!PL_profiledata)
	return;
    for (i = 0; i < MAXO; i++) {
	if (PL_profiledata[i])
	    PerlIO_printf(Perl_debug_log,
			  "%5lu %s\n", (unsigned long)PL_profiledata[i],
                                       PL_op_name[i]);
    }
}

#ifdef PERL_MAD
/*
 *    XML variants of most of the above routines
 */

STATIC void
S_xmldump_attr(pTHX_ I32 level, PerlIO *file, const char* pat, ...)
{
    va_list args;

    PERL_ARGS_ASSERT_XMLDUMP_ATTR;

    PerlIO_printf(file, "\n    ");
    va_start(args, pat);
    xmldump_vindent(level, file, pat, &args);
    va_end(args);
}


void
Perl_xmldump_indent(pTHX_ I32 level, PerlIO *file, const char* pat, ...)
{
    va_list args;
    PERL_ARGS_ASSERT_XMLDUMP_INDENT;
    va_start(args, pat);
    xmldump_vindent(level, file, pat, &args);
    va_end(args);
}

void
Perl_xmldump_vindent(pTHX_ I32 level, PerlIO *file, const char* pat, va_list *args)
{
    PERL_ARGS_ASSERT_XMLDUMP_VINDENT;

    PerlIO_printf(file, "%*s", (int)(level*PL_dumpindent), "");
    PerlIO_vprintf(file, pat, *args);
}

void
Perl_xmldump_all(pTHX)
{
    xmldump_all_perl(FALSE);
}

void
Perl_xmldump_all_perl(pTHX_ bool justperl PERL_UNUSED_DECL)
{
    PerlIO_setlinebuf(PL_xmlfp);
    if (PL_main_root)
	op_xmldump(PL_main_root);
    /* someday we might call this, when it outputs XML: */
    /* xmldump_packsubs_perl(PL_defstash, justperl); */
    if (PL_xmlfp != (PerlIO*)PerlIO_stdout())
	PerlIO_close(PL_xmlfp);
    PL_xmlfp = 0;
}

void
Perl_xmldump_packsubs(pTHX_ const HV *stash)
{
    PERL_ARGS_ASSERT_XMLDUMP_PACKSUBS;
    xmldump_packsubs_perl(stash, FALSE);
}

void
Perl_xmldump_packsubs_perl(pTHX_ const HV *stash, bool justperl)
{
    I32	i;
    HE	*entry;

    PERL_ARGS_ASSERT_XMLDUMP_PACKSUBS_PERL;

    if (!HvARRAY(stash))
	return;
    for (i = 0; i <= (I32) HvMAX(stash); i++) {
	for (entry = HvARRAY(stash)[i]; entry; entry = HeNEXT(entry)) {
	    GV *gv = MUTABLE_GV(HeVAL(entry));
	    HV *hv;
	    if (SvTYPE(gv) != SVt_PVGV || !GvGP(gv))
		continue;
	    if (GvCVu(gv))
		xmldump_sub_perl(gv, justperl);
	    if (GvFORM(gv))
		xmldump_form(gv);
	    if (HeKEY(entry)[HeKLEN(entry)-1] == ':'
		&& (hv = GvHV(gv)) && hv != PL_defstash)
		xmldump_packsubs_perl(hv, justperl);	/* nested package */
	}
    }
}

void
Perl_xmldump_sub(pTHX_ const GV *gv)
{
    PERL_ARGS_ASSERT_XMLDUMP_SUB;
    xmldump_sub_perl(gv, FALSE);
}

void
Perl_xmldump_sub_perl(pTHX_ const GV *gv, bool justperl)
{
    SV * sv;

    PERL_ARGS_ASSERT_XMLDUMP_SUB_PERL;

    if (justperl && (CvISXSUB(GvCV(gv)) || !CvROOT(GvCV(gv))))
	return;

    sv = sv_newmortal();
    gv_fullname3(sv, gv, NULL);
    Perl_xmldump_indent(aTHX_ 0, PL_xmlfp, "\nSUB %s = ", SvPVX(sv));
    if (CvXSUB(GvCV(gv)))
	Perl_xmldump_indent(aTHX_ 0, PL_xmlfp, "(xsub 0x%"UVxf" %d)\n",
	    PTR2UV(CvXSUB(GvCV(gv))),
	    (int)CvXSUBANY(GvCV(gv)).any_i32);
    else if (CvROOT(GvCV(gv)))
	op_xmldump(CvROOT(GvCV(gv)));
    else
	Perl_xmldump_indent(aTHX_ 0, PL_xmlfp, "<undef>\n");
}

void
Perl_xmldump_form(pTHX_ const GV *gv)
{
    SV * const sv = sv_newmortal();

    PERL_ARGS_ASSERT_XMLDUMP_FORM;

    gv_fullname3(sv, gv, NULL);
    Perl_xmldump_indent(aTHX_ 0, PL_xmlfp, "\nFORMAT %s = ", SvPVX(sv));
    if (CvROOT(GvFORM(gv)))
	op_xmldump(CvROOT(GvFORM(gv)));
    else
	Perl_xmldump_indent(aTHX_ 0, PL_xmlfp, "<undef>\n");
}

void
Perl_xmldump_eval(pTHX)
{
    op_xmldump(PL_eval_root);
}

char *
Perl_sv_catxmlsv(pTHX_ SV *dsv, SV *ssv)
{
    PERL_ARGS_ASSERT_SV_CATXMLSV;
    return sv_catxmlpvn(dsv, SvPVX(ssv), SvCUR(ssv), SvUTF8(ssv));
}

char *
Perl_sv_catxmlpv(pTHX_ SV *dsv, const char *pv, int utf8)
{
    PERL_ARGS_ASSERT_SV_CATXMLPV;
    return sv_catxmlpvn(dsv, pv, strlen(pv), utf8);
}

char *
Perl_sv_catxmlpvn(pTHX_ SV *dsv, const char *pv, STRLEN len, int utf8)
{
    unsigned int c;
    const char * const e = pv + len;
    const char * const start = pv;
    STRLEN dsvcur;
    STRLEN cl;

    PERL_ARGS_ASSERT_SV_CATXMLPVN;

    sv_catpvs(dsv,"");
    dsvcur = SvCUR(dsv);	/* in case we have to restart */

  retry:
    while (pv < e) {
	if (utf8) {
	    c = utf8_to_uvchr_buf((U8*)pv, (U8*)e, &cl);
	    if (cl == 0) {
		SvCUR(dsv) = dsvcur;
		pv = start;
		utf8 = 0;
		goto retry;
	    }
	}
	else
	    c = (*pv & 255);

	switch (c) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x0b:
	case 0x0c:
	case 0x0e:
	case 0x0f:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b:
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
	case 0x7f:
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x86:
	case 0x87:
	case 0x88:
	case 0x89:
	case 0x90:
	case 0x91:
	case 0x92:
	case 0x93:
	case 0x94:
	case 0x95:
	case 0x96:
	case 0x97:
	case 0x98:
	case 0x99:
	case 0x9a:
	case 0x9b:
	case 0x9c:
	case 0x9d:
	case 0x9e:
	case 0x9f:
	    Perl_sv_catpvf(aTHX_ dsv, "STUPIDXML(#x%X)", c);
	    break;
	case '<':
	    sv_catpvs(dsv, "&lt;");
	    break;
	case '>':
	    sv_catpvs(dsv, "&gt;");
	    break;
	case '&':
	    sv_catpvs(dsv, "&amp;");
	    break;
	case '"':
	    sv_catpvs(dsv, "&#34;");
	    break;
	default:
	    if (c < 0xD800) {
		if (c < 32 || c > 127) {
		    Perl_sv_catpvf(aTHX_ dsv, "&#x%X;", c);
		}
		else {
		    const char string = (char) c;
		    sv_catpvn(dsv, &string, 1);
		}
		break;
	    }
	    if ((c >= 0xD800 && c <= 0xDB7F) ||
		(c >= 0xDC00 && c <= 0xDFFF) ||
		(c >= 0xFFF0 && c <= 0xFFFF) ||
		 c > 0x10ffff)
		Perl_sv_catpvf(aTHX_ dsv, "STUPIDXML(#x%X)", c);
	    else
		Perl_sv_catpvf(aTHX_ dsv, "&#x%X;", c);
	}

	if (utf8)
	    pv += UTF8SKIP(pv);
	else
	    pv++;
    }

    return SvPVX(dsv);
}

char *
Perl_sv_xmlpeek(pTHX_ SV *sv)
{
    SV * const t = sv_newmortal();
    STRLEN n_a;
    int unref = 0;

    PERL_ARGS_ASSERT_SV_XMLPEEK;

    sv_utf8_upgrade(t);
    sv_setpvs(t, "");
    /* retry: */
    if (!sv) {
	sv_catpv(t, "VOID=\"\"");
	goto finish;
    }
    else if (sv == (const SV *)0x55555555 || SvTYPE(sv) == 'U') {
	sv_catpv(t, "WILD=\"\"");
	goto finish;
    }
    else if (sv == &PL_sv_undef || sv == &PL_sv_no || sv == &PL_sv_yes || sv == &PL_sv_placeholder) {
	if (sv == &PL_sv_undef) {
	    sv_catpv(t, "SV_UNDEF=\"1\"");
	    if (!(SvFLAGS(sv) & (SVf_OK|SVf_OOK|SVs_OBJECT|
				 SVs_GMG|SVs_SMG|SVs_RMG)) &&
		SvREADONLY(sv))
		goto finish;
	}
	else if (sv == &PL_sv_no) {
	    sv_catpv(t, "SV_NO=\"1\"");
	    if (!(SvFLAGS(sv) & (SVf_ROK|SVf_OOK|SVs_OBJECT|
				 SVs_GMG|SVs_SMG|SVs_RMG)) &&
		!(~SvFLAGS(sv) & (SVf_POK|SVf_NOK|SVf_READONLY|
				  SVp_POK|SVp_NOK)) &&
		SvCUR(sv) == 0 &&
		SvNVX(sv) == 0.0)
		goto finish;
	}
	else if (sv == &PL_sv_yes) {
	    sv_catpv(t, "SV_YES=\"1\"");
	    if (!(SvFLAGS(sv) & (SVf_ROK|SVf_OOK|SVs_OBJECT|
				 SVs_GMG|SVs_SMG|SVs_RMG)) &&
		!(~SvFLAGS(sv) & (SVf_POK|SVf_NOK|SVf_READONLY|
				  SVp_POK|SVp_NOK)) &&
		SvCUR(sv) == 1 &&
		SvPVX(sv) && *SvPVX(sv) == '1' &&
		SvNVX(sv) == 1.0)
		goto finish;
	}
	else {
	    sv_catpv(t, "SV_PLACEHOLDER=\"1\"");
	    if (!(SvFLAGS(sv) & (SVf_OK|SVf_OOK|SVs_OBJECT|
				 SVs_GMG|SVs_SMG|SVs_RMG)) &&
		SvREADONLY(sv))
		goto finish;
	}
	sv_catpv(t, " XXX=\"\" ");
    }
    else if (SvREFCNT(sv) == 0) {
	sv_catpv(t, " refcnt=\"0\"");
	unref++;
    }
    else if (DEBUG_R_TEST_) {
	int is_tmp = 0;
	I32 ix;
	/* is this SV on the tmps stack? */
	for (ix=PL_tmps_ix; ix>=0; ix--) {
	    if (PL_tmps_stack[ix] == sv) {
		is_tmp = 1;
		break;
	    }
	}
	if (SvREFCNT(sv) > 1)
	    Perl_sv_catpvf(aTHX_ t, " DRT=\"<%"UVuf"%s>\"", (UV)SvREFCNT(sv),
		    is_tmp ? "T" : "");
	else if (is_tmp)
	    sv_catpv(t, " DRT=\"<T>\"");
    }

    if (SvROK(sv)) {
	sv_catpv(t, " ROK=\"\"");
    }
    switch (SvTYPE(sv)) {
    default:
	sv_catpv(t, " FREED=\"1\"");
	goto finish;

    case SVt_NULL:
	sv_catpv(t, " UNDEF=\"1\"");
	goto finish;
    case SVt_IV:
	sv_catpv(t, " IV=\"");
	break;
    case SVt_NV:
	sv_catpv(t, " NV=\"");
	break;
    case SVt_PV:
	sv_catpv(t, " PV=\"");
	break;
    case SVt_PVIV:
	sv_catpv(t, " PVIV=\"");
	break;
    case SVt_PVNV:
	sv_catpv(t, " PVNV=\"");
	break;
    case SVt_PVMG:
	sv_catpv(t, " PVMG=\"");
	break;
    case SVt_PVLV:
	sv_catpv(t, " PVLV=\"");
	break;
    case SVt_PVAV:
	sv_catpv(t, " AV=\"");
	break;
    case SVt_PVHV:
	sv_catpv(t, " HV=\"");
	break;
    case SVt_PVCV:
	if (CvGV(sv))
	    Perl_sv_catpvf(aTHX_ t, " CV=\"(%s)\"", GvNAME(CvGV(sv)));
	else
	    sv_catpv(t, " CV=\"()\"");
	goto finish;
    case SVt_PVGV:
	sv_catpv(t, " GV=\"");
	break;
    case SVt_BIND:
	sv_catpv(t, " BIND=\"");
	break;
    case SVt_REGEXP:
	sv_catpv(t, " REGEXP=\"");
	break;
    case SVt_PVFM:
	sv_catpv(t, " FM=\"");
	break;
    case SVt_PVIO:
	sv_catpv(t, " IO=\"");
	break;
    }

    if (SvPOKp(sv)) {
	if (SvPVX(sv)) {
	    sv_catxmlsv(t, sv);
	}
    }
    else if (SvNOKp(sv)) {
	STORE_NUMERIC_LOCAL_SET_STANDARD();
	Perl_sv_catpvf(aTHX_ t, "%"NVgf"",SvNVX(sv));
	RESTORE_NUMERIC_LOCAL();
    }
    else if (SvIOKp(sv)) {
	if (SvIsUV(sv))
	    Perl_sv_catpvf(aTHX_ t, "%"UVuf"", (UV)SvUVX(sv));
	else
            Perl_sv_catpvf(aTHX_ t, "%"IVdf"", (IV)SvIVX(sv));
    }
    else
	sv_catpv(t, "");
    sv_catpv(t, "\"");

  finish:
    while (unref--)
	sv_catpv(t, ")");
    return SvPV(t, n_a);
}

void
Perl_do_pmop_xmldump(pTHX_ I32 level, PerlIO *file, const PMOP *pm)
{
    PERL_ARGS_ASSERT_DO_PMOP_XMLDUMP;

    if (!pm) {
	Perl_xmldump_indent(aTHX_ level, file, "<pmop/>\n");
	return;
    }
    Perl_xmldump_indent(aTHX_ level, file, "<pmop \n");
    level++;
    if (PM_GETRE(pm)) {
	REGEXP *const r = PM_GETRE(pm);
	SV * const tmpsv = newSVpvn_utf8("", 0, TRUE);
	sv_catxmlsv(tmpsv, MUTABLE_SV(r));
	Perl_xmldump_indent(aTHX_ level, file, "pre=\"%s\"\n",
	     SvPVX(tmpsv));
	SvREFCNT_dec(tmpsv);
	Perl_xmldump_indent(aTHX_ level, file, "when=\"%s\"\n",
	     (pm->op_private & OPpRUNTIME) ? "RUN" : "COMP");
    }
    else
	Perl_xmldump_indent(aTHX_ level, file, "pre=\"\" when=\"RUN\"\n");
    if (pm->op_pmflags || (PM_GETRE(pm) && RX_CHECK_SUBSTR(PM_GETRE(pm)))) {
	SV * const tmpsv = pm_description(pm);
	Perl_xmldump_indent(aTHX_ level, file, "pmflags=\"%s\"\n", SvCUR(tmpsv) ? SvPVX(tmpsv) + 1 : "");
	SvREFCNT_dec(tmpsv);
    }

    level--;
    if (pm->op_type != OP_PUSHRE && pm->op_pmreplrootu.op_pmreplroot) {
	Perl_xmldump_indent(aTHX_ level, file, ">\n");
	Perl_xmldump_indent(aTHX_ level+1, file, "<pm_repl>\n");
	do_op_xmldump(level+2, file, pm->op_pmreplrootu.op_pmreplroot);
	Perl_xmldump_indent(aTHX_ level+1, file, "</pm_repl>\n");
	Perl_xmldump_indent(aTHX_ level, file, "</pmop>\n");
    }
    else
	Perl_xmldump_indent(aTHX_ level, file, "/>\n");
}

void
Perl_pmop_xmldump(pTHX_ const PMOP *pm)
{
    do_pmop_xmldump(0, PL_xmlfp, pm);
}

void
Perl_do_op_xmldump(pTHX_ I32 level, PerlIO *file, const OP *o)
{
    UV      seq;
    int     contents = 0;

    PERL_ARGS_ASSERT_DO_OP_XMLDUMP;

    if (!o)
	return;
    seq = sequence_num(o);
    Perl_xmldump_indent(aTHX_ level, file,
	"<op_%s seq=\"%"UVuf" -> ",
	     OP_NAME(o),
	              seq);
    level++;
    if (o->op_next)
	PerlIO_printf(file, seq ? "%"UVuf"\"" : "(%"UVuf")\"",
		      sequence_num(o->op_next));
    else
	PerlIO_printf(file, "DONE\"");

    if (o->op_targ) {
	if (o->op_type == OP_NULL)
	{
	    PerlIO_printf(file, " was=\"%s\"", PL_op_name[o->op_targ]);
	    if (o->op_targ == OP_NEXTSTATE)
	    {
		if (CopLINE(cCOPo))
		    PerlIO_printf(file, " line=\"%"UVuf"\"",
				     (UV)CopLINE(cCOPo));
		if (CopSTASHPV(cCOPo))
		    PerlIO_printf(file, " package=\"%s\"",
				     CopSTASHPV(cCOPo));
		if (CopLABEL(cCOPo))
		    PerlIO_printf(file, " label=\"%s\"",
				     CopLABEL(cCOPo));
	    }
	}
	else
	    PerlIO_printf(file, " targ=\"%ld\"", (long)o->op_targ);
    }
#ifdef DUMPADDR
    PerlIO_printf(file, " addr=\"0x%"UVxf" => 0x%"UVxf"\"", (UV)o, (UV)o->op_next);
#endif
    if (o->op_flags) {
	SV * const tmpsv = newSVpvs("");
	switch (o->op_flags & OPf_WANT) {
	case OPf_WANT_VOID:
	    sv_catpv(tmpsv, ",VOID");
	    break;
	case OPf_WANT_SCALAR:
	    sv_catpv(tmpsv, ",SCALAR");
	    break;
	case OPf_WANT_LIST:
	    sv_catpv(tmpsv, ",LIST");
	    break;
	default:
	    sv_catpv(tmpsv, ",UNKNOWN");
	    break;
	}
	if (o->op_flags & OPf_KIDS)
	    sv_catpv(tmpsv, ",KIDS");
	if (o->op_flags & OPf_PARENS)
	    sv_catpv(tmpsv, ",PARENS");
	if (o->op_flags & OPf_STACKED)
	    sv_catpv(tmpsv, ",STACKED");
	if (o->op_flags & OPf_REF)
	    sv_catpv(tmpsv, ",REF");
	if (o->op_flags & OPf_MOD)
	    sv_catpv(tmpsv, ",MOD");
	if (o->op_flags & OPf_SPECIAL)
	    sv_catpv(tmpsv, ",SPECIAL");
	PerlIO_printf(file, " flags=\"%s\"", SvCUR(tmpsv) ? SvPVX(tmpsv) + 1 : "");
	SvREFCNT_dec(tmpsv);
    }
    if (o->op_private) {
	SV * const tmpsv = newSVpvs("");
	if (PL_opargs[o->op_type] & OA_TARGLEX) {
	    if (o->op_private & OPpTARGET_MY)
		sv_catpv(tmpsv, ",TARGET_MY");
	}
	else if (o->op_type == OP_LEAVESUB ||
		 o->op_type == OP_LEAVE ||
		 o->op_type == OP_LEAVESUBLV ||
		 o->op_type == OP_LEAVEWRITE) {
	    if (o->op_private & OPpREFCOUNTED)
		sv_catpv(tmpsv, ",REFCOUNTED");
	}
        else if (o->op_type == OP_AASSIGN) {
	    if (o->op_private & OPpASSIGN_COMMON)
		sv_catpv(tmpsv, ",COMMON");
	}
	else if (o->op_type == OP_SASSIGN) {
	    if (o->op_private & OPpASSIGN_BACKWARDS)
		sv_catpv(tmpsv, ",BACKWARDS");
	}
	else if (o->op_type == OP_TRANS) {
	    if (o->op_private & OPpTRANS_SQUASH)
		sv_catpv(tmpsv, ",SQUASH");
	    if (o->op_private & OPpTRANS_DELETE)
		sv_catpv(tmpsv, ",DELETE");
	    if (o->op_private & OPpTRANS_COMPLEMENT)
		sv_catpv(tmpsv, ",COMPLEMENT");
	    if (o->op_private & OPpTRANS_IDENTICAL)
		sv_catpv(tmpsv, ",IDENTICAL");
	    if (o->op_private & OPpTRANS_GROWS)
		sv_catpv(tmpsv, ",GROWS");
	}
	else if (o->op_type == OP_REPEAT) {
	    if (o->op_private & OPpREPEAT_DOLIST)
		sv_catpv(tmpsv, ",DOLIST");
	}
	else if (o->op_type == OP_ENTERSUB ||
		 o->op_type == OP_RV2SV ||
		 o->op_type == OP_GVSV ||
		 o->op_type == OP_RV2AV ||
		 o->op_type == OP_RV2HV ||
		 o->op_type == OP_RV2GV ||
		 o->op_type == OP_AELEM ||
		 o->op_type == OP_HELEM )
	{
	    if (o->op_type == OP_ENTERSUB) {
		if (o->op_private & OPpENTERSUB_AMPER)
		    sv_catpv(tmpsv, ",AMPER");
		if (o->op_private & OPpENTERSUB_DB)
		    sv_catpv(tmpsv, ",DB");
		if (o->op_private & OPpENTERSUB_HASTARG)
		    sv_catpv(tmpsv, ",HASTARG");
		if (o->op_private & OPpENTERSUB_NOPAREN)
		    sv_catpv(tmpsv, ",NOPAREN");
		if (o->op_private & OPpENTERSUB_INARGS)
		    sv_catpv(tmpsv, ",INARGS");
	    }
	    else {
		switch (o->op_private & OPpDEREF) {
	    case OPpDEREF_SV:
		sv_catpv(tmpsv, ",SV");
		break;
	    case OPpDEREF_AV:
		sv_catpv(tmpsv, ",AV");
		break;
	    case OPpDEREF_HV:
		sv_catpv(tmpsv, ",HV");
		break;
	    }
		if (o->op_private & OPpMAYBE_LVSUB)
		    sv_catpv(tmpsv, ",MAYBE_LVSUB");
	    }
	    if (o->op_type == OP_AELEM || o->op_type == OP_HELEM) {
		if (o->op_private & OPpLVAL_DEFER)
		    sv_catpv(tmpsv, ",LVAL_DEFER");
	    }
	    else {
		if (o->op_private & HINT_STRICT_REFS)
		    sv_catpv(tmpsv, ",STRICT_REFS");
		if (o->op_private & OPpOUR_INTRO)
		    sv_catpv(tmpsv, ",OUR_INTRO");
	    }
	}
	else if (o->op_type == OP_CONST) {
	    if (o->op_private & OPpCONST_BARE)
		sv_catpv(tmpsv, ",BARE");
	    if (o->op_private & OPpCONST_STRICT)
		sv_catpv(tmpsv, ",STRICT");
	    if (o->op_private & OPpCONST_WARNING)
		sv_catpv(tmpsv, ",WARNING");
	    if (o->op_private & OPpCONST_ENTERED)
		sv_catpv(tmpsv, ",ENTERED");
	}
	else if (o->op_type == OP_FLIP) {
	    if (o->op_private & OPpFLIP_LINENUM)
		sv_catpv(tmpsv, ",LINENUM");
	}
	else if (o->op_type == OP_FLOP) {
	    if (o->op_private & OPpFLIP_LINENUM)
		sv_catpv(tmpsv, ",LINENUM");
	}
	else if (o->op_type == OP_RV2CV) {
	    if (o->op_private & OPpLVAL_INTRO)
		sv_catpv(tmpsv, ",INTRO");
	}
	else if (o->op_type == OP_GV) {
	    if (o->op_private & OPpEARLY_CV)
		sv_catpv(tmpsv, ",EARLY_CV");
	}
	else if (o->op_type == OP_LIST) {
	    if (o->op_private & OPpLIST_GUESSED)
		sv_catpv(tmpsv, ",GUESSED");
	}
	else if (o->op_type == OP_DELETE) {
	    if (o->op_private & OPpSLICE)
		sv_catpv(tmpsv, ",SLICE");
	}
	else if (o->op_type == OP_EXISTS) {
	    if (o->op_private & OPpEXISTS_SUB)
		sv_catpv(tmpsv, ",EXISTS_SUB");
	}
	else if (o->op_type == OP_SORT) {
	    if (o->op_private & OPpSORT_NUMERIC)
		sv_catpv(tmpsv, ",NUMERIC");
	    if (o->op_private & OPpSORT_INTEGER)
		sv_catpv(tmpsv, ",INTEGER");
	    if (o->op_private & OPpSORT_REVERSE)
		sv_catpv(tmpsv, ",REVERSE");
	}
	else if (o->op_type == OP_OPEN || o->op_type == OP_BACKTICK) {
	    if (o->op_private & OPpOPEN_IN_RAW)
		sv_catpv(tmpsv, ",IN_RAW");
	    if (o->op_private & OPpOPEN_IN_CRLF)
		sv_catpv(tmpsv, ",IN_CRLF");
	    if (o->op_private & OPpOPEN_OUT_RAW)
		sv_catpv(tmpsv, ",OUT_RAW");
	    if (o->op_private & OPpOPEN_OUT_CRLF)
		sv_catpv(tmpsv, ",OUT_CRLF");
	}
	else if (o->op_type == OP_EXIT) {
	    if (o->op_private & OPpEXIT_VMSISH)
		sv_catpv(tmpsv, ",EXIT_VMSISH");
	    if (o->op_private & OPpHUSH_VMSISH)
		sv_catpv(tmpsv, ",HUSH_VMSISH");
	}
	else if (o->op_type == OP_DIE) {
	    if (o->op_private & OPpHUSH_VMSISH)
		sv_catpv(tmpsv, ",HUSH_VMSISH");
	}
	else if (PL_check[o->op_type] != Perl_ck_ftst) {
	    if (OP_IS_FILETEST_ACCESS(o->op_type) && o->op_private & OPpFT_ACCESS)
		sv_catpv(tmpsv, ",FT_ACCESS");
	    if (o->op_private & OPpFT_STACKED)
		sv_catpv(tmpsv, ",FT_STACKED");
	}
	if (o->op_flags & OPf_MOD && o->op_private & OPpLVAL_INTRO)
	    sv_catpv(tmpsv, ",INTRO");
	if (SvCUR(tmpsv))
	    S_xmldump_attr(aTHX_ level, file, "private=\"%s\"", SvPVX(tmpsv) + 1);
	SvREFCNT_dec(tmpsv);
    }

    switch (o->op_type) {
    case OP_AELEMFAST:
	if (o->op_flags & OPf_SPECIAL) {
	    break;
	}
    case OP_GVSV:
    case OP_GV:
#ifdef USE_ITHREADS
	S_xmldump_attr(aTHX_ level, file, "padix=\"%" IVdf "\"", (IV)cPADOPo->op_padix);
#else
	if (cSVOPo->op_sv) {
	    SV * const tmpsv1 = newSVpvn_utf8(NULL, 0, TRUE);
	    SV * const tmpsv2 = newSVpvn_utf8("", 0, TRUE);
	    char *s;
	    STRLEN len;
	    ENTER;
	    SAVEFREESV(tmpsv1);
	    SAVEFREESV(tmpsv2);
	    gv_fullname3(tmpsv1, MUTABLE_GV(cSVOPo->op_sv), NULL);
	    s = SvPV(tmpsv1,len);
	    sv_catxmlpvn(tmpsv2, s, len, 1);
	    S_xmldump_attr(aTHX_ level, file, "gv=\"%s\"", SvPV(tmpsv2, len));
	    LEAVE;
	}
	else
	    S_xmldump_attr(aTHX_ level, file, "gv=\"NULL\"");
#endif
	break;
    case OP_CONST:
    case OP_HINTSEVAL:
    case OP_METHOD_NAMED:
#ifndef USE_ITHREADS
	/* with ITHREADS, consts are stored in the pad, and the right pad
	 * may not be active here, so skip */
	S_xmldump_attr(aTHX_ level, file, "%s", sv_xmlpeek(cSVOPo_sv));
#endif
	break;
    case OP_ANONCODE:
	if (!contents) {
	    contents = 1;
	    PerlIO_printf(file, ">\n");
	}
	do_op_xmldump(level+1, file, CvROOT(cSVOPo_sv));
	break;
    case OP_NEXTSTATE:
    case OP_DBSTATE:
	if (CopLINE(cCOPo))
	    S_xmldump_attr(aTHX_ level, file, "line=\"%"UVuf"\"",
			     (UV)CopLINE(cCOPo));
	if (CopSTASHPV(cCOPo))
	    S_xmldump_attr(aTHX_ level, file, "package=\"%s\"",
			     CopSTASHPV(cCOPo));
	if (CopLABEL(cCOPo))
	    S_xmldump_attr(aTHX_ level, file, "label=\"%s\"",
			     CopLABEL(cCOPo));
	break;
    case OP_ENTERLOOP:
	S_xmldump_attr(aTHX_ level, file, "redo=\"");
	if (cLOOPo->op_redoop)
	    PerlIO_printf(file, "%"UVuf"\"", sequence_num(cLOOPo->op_redoop));
	else
	    PerlIO_printf(file, "DONE\"");
	S_xmldump_attr(aTHX_ level, file, "next=\"");
	if (cLOOPo->op_nextop)
	    PerlIO_printf(file, "%"UVuf"\"", sequence_num(cLOOPo->op_nextop));
	else
	    PerlIO_printf(file, "DONE\"");
	S_xmldump_attr(aTHX_ level, file, "last=\"");
	if (cLOOPo->op_lastop)
	    PerlIO_printf(file, "%"UVuf"\"", sequence_num(cLOOPo->op_lastop));
	else
	    PerlIO_printf(file, "DONE\"");
	break;
    case OP_COND_EXPR:
    case OP_RANGE:
    case OP_MAPWHILE:
    case OP_GREPWHILE:
    case OP_OR:
    case OP_AND:
	S_xmldump_attr(aTHX_ level, file, "other=\"");
	if (cLOGOPo->op_other)
	    PerlIO_printf(file, "%"UVuf"\"", sequence_num(cLOGOPo->op_other));
	else
	    PerlIO_printf(file, "DONE\"");
	break;
    case OP_LEAVE:
    case OP_LEAVEEVAL:
    case OP_LEAVESUB:
    case OP_LEAVESUBLV:
    case OP_LEAVEWRITE:
    case OP_SCOPE:
	if (o->op_private & OPpREFCOUNTED)
	    S_xmldump_attr(aTHX_ level, file, "refcnt=\"%"UVuf"\"", (UV)o->op_targ);
	break;
    default:
	break;
    }

    if (PL_madskills && o->op_madprop) {
	char prevkey = '\0';
	SV * const tmpsv = newSVpvn_utf8("", 0, TRUE);
	const MADPROP* mp = o->op_madprop;

	if (!contents) {
	    contents = 1;
	    PerlIO_printf(file, ">\n");
	}
	Perl_xmldump_indent(aTHX_ level, file, "<madprops>\n");
	level++;
	while (mp) {
	    char tmp = mp->mad_key;
	    sv_setpvs(tmpsv,"\"");
	    if (tmp)
		sv_catxmlpvn(tmpsv, &tmp, 1, 0);
	    if ((tmp == '_') || (tmp == '#')) /* '_' '#' whitespace belong to the previous token. */
		sv_catxmlpvn(tmpsv, &prevkey, 1, 0);
	    else
		prevkey = tmp;
	    sv_catpv(tmpsv, "\"");
	    switch (mp->mad_type) {
	    case MAD_NULL:
		sv_catpv(tmpsv, "NULL");
		Perl_xmldump_indent(aTHX_ level, file, "<mad_null key=%s/>\n", SvPVX(tmpsv));
		break;
	    case MAD_PV:
		sv_catpv(tmpsv, " val=\"");
		sv_catxmlpvn(tmpsv, (char*)mp->mad_val, mp->mad_vlen,1);
		sv_catpv(tmpsv, "\"");
		Perl_xmldump_indent(aTHX_ level, file, "<mad_pv key=%s/>\n", SvPVX(tmpsv));
		break;
	    case MAD_SV:
		sv_catpv(tmpsv, " val=\"");
		sv_catxmlsv(tmpsv, MUTABLE_SV(mp->mad_val));
		sv_catpv(tmpsv, "\"");
		Perl_xmldump_indent(aTHX_ level, file, "<mad_sv key=%s/>\n", SvPVX(tmpsv));
		break;
	    case MAD_OP:
		if ((OP*)mp->mad_val) {
		    Perl_xmldump_indent(aTHX_ level, file, "<mad_op key=%s>\n", SvPVX(tmpsv));
		    do_op_xmldump(level+1, file, (OP*)mp->mad_val);
		    Perl_xmldump_indent(aTHX_ level, file, "</mad_op>\n");
		}
		break;
	    default:
		Perl_xmldump_indent(aTHX_ level, file, "<mad_unk key=%s/>\n", SvPVX(tmpsv));
		break;
	    }
	    mp = mp->mad_next;
	}
	level--;
	Perl_xmldump_indent(aTHX_ level, file, "</madprops>\n");

	SvREFCNT_dec(tmpsv);
    }

    switch (o->op_type) {
    case OP_PUSHRE:
    case OP_MATCH:
    case OP_QR:
    case OP_SUBST:
	if (!contents) {
	    contents = 1;
	    PerlIO_printf(file, ">\n");
	}
	do_pmop_xmldump(level, file, cPMOPo);
	break;
    default:
	break;
    }

    if (o->op_flags & OPf_KIDS) {
	OP *kid;
	if (!contents) {
	    contents = 1;
	    PerlIO_printf(file, ">\n");
	}
	for (kid = cUNOPo->op_first; kid; kid = kid->op_sibling)
	    do_op_xmldump(level, file, kid);
    }

    if (contents)
	Perl_xmldump_indent(aTHX_ level-1, file, "</op_%s>\n", OP_NAME(o));
    else
	PerlIO_printf(file, " />\n");
}

void
Perl_op_xmldump(pTHX_ const OP *o)
{
    PERL_ARGS_ASSERT_OP_XMLDUMP;

    do_op_xmldump(0, PL_xmlfp, o);
}
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
