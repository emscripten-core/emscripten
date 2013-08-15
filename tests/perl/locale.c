/*    locale.c
 *
 *    Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001,
 *    2002, 2003, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *      A Elbereth Gilthoniel,
 *      silivren penna míriel
 *      o menel aglar elenath!
 *      Na-chaered palan-díriel
 *      o galadhremmin ennorath,
 *      Fanuilos, le linnathon
 *      nef aear, si nef aearon!
 *
 *     [p.238 of _The Lord of the Rings_, II/i: "Many Meetings"]
 */

/* utility functions for handling locale-specific stuff like what
 * character represents the decimal point.
 */

#include "EXTERN.h"
#define PERL_IN_LOCALE_C
#include "perl.h"

#ifdef I_LANGINFO
#   include <langinfo.h>
#endif

#include "reentr.h"

#if defined(USE_LOCALE_NUMERIC) || defined(USE_LOCALE_COLLATE)
/*
 * Standardize the locale name from a string returned by 'setlocale'.
 *
 * The standard return value of setlocale() is either
 * (1) "xx_YY" if the first argument of setlocale() is not LC_ALL
 * (2) "xa_YY xb_YY ..." if the first argument of setlocale() is LC_ALL
 *     (the space-separated values represent the various sublocales,
 *      in some unspecified order)
 *
 * In some platforms it has a form like "LC_SOMETHING=Lang_Country.866\n",
 * which is harmful for further use of the string in setlocale().
 *
 */
STATIC char *
S_stdize_locale(pTHX_ char *locs)
{
    const char * const s = strchr(locs, '=');
    bool okay = TRUE;

    PERL_ARGS_ASSERT_STDIZE_LOCALE;

    if (s) {
	const char * const t = strchr(s, '.');
	okay = FALSE;
	if (t) {
	    const char * const u = strchr(t, '\n');
	    if (u && (u[1] == 0)) {
		const STRLEN len = u - s;
		Move(s + 1, locs, len, char);
		locs[len] = 0;
		okay = TRUE;
	    }
	}
    }

    if (!okay)
	Perl_croak(aTHX_ "Can't fix broken locale name \"%s\"", locs);

    return locs;
}
#endif

void
Perl_set_numeric_radix(pTHX)
{
#ifdef USE_LOCALE_NUMERIC
    dVAR;
# ifdef HAS_LOCALECONV
    const struct lconv* const lc = localeconv();

    if (lc && lc->decimal_point) {
	if (lc->decimal_point[0] == '.' && lc->decimal_point[1] == 0) {
	    SvREFCNT_dec(PL_numeric_radix_sv);
	    PL_numeric_radix_sv = NULL;
	}
	else {
	    if (PL_numeric_radix_sv)
		sv_setpv(PL_numeric_radix_sv, lc->decimal_point);
	    else
		PL_numeric_radix_sv = newSVpv(lc->decimal_point, 0);
	}
    }
    else
	PL_numeric_radix_sv = NULL;
# endif /* HAS_LOCALECONV */
#endif /* USE_LOCALE_NUMERIC */
}

/*
 * Set up for a new numeric locale.
 */
void
Perl_new_numeric(pTHX_ const char *newnum)
{
#ifdef USE_LOCALE_NUMERIC
    dVAR;

    if (! newnum) {
	Safefree(PL_numeric_name);
	PL_numeric_name = NULL;
	PL_numeric_standard = TRUE;
	PL_numeric_local = TRUE;
	return;
    }

    if (! PL_numeric_name || strNE(PL_numeric_name, newnum)) {
	Safefree(PL_numeric_name);
	PL_numeric_name = stdize_locale(savepv(newnum));
	PL_numeric_standard = ((*newnum == 'C' && newnum[1] == '\0')
			       || strEQ(newnum, "POSIX"));
	PL_numeric_local = TRUE;
	set_numeric_radix();
    }

#endif /* USE_LOCALE_NUMERIC */
}

void
Perl_set_numeric_standard(pTHX)
{
#ifdef USE_LOCALE_NUMERIC
    dVAR;

    if (! PL_numeric_standard) {
	setlocale(LC_NUMERIC, "C");
	PL_numeric_standard = TRUE;
	PL_numeric_local = FALSE;
	set_numeric_radix();
    }

#endif /* USE_LOCALE_NUMERIC */
}

void
Perl_set_numeric_local(pTHX)
{
#ifdef USE_LOCALE_NUMERIC
    dVAR;

    if (! PL_numeric_local) {
	setlocale(LC_NUMERIC, PL_numeric_name);
	PL_numeric_standard = FALSE;
	PL_numeric_local = TRUE;
	set_numeric_radix();
    }

#endif /* USE_LOCALE_NUMERIC */
}

/*
 * Set up for a new ctype locale.
 */
void
Perl_new_ctype(pTHX_ const char *newctype)
{
#ifdef USE_LOCALE_CTYPE
    dVAR;
    int i;

    PERL_ARGS_ASSERT_NEW_CTYPE;

    for (i = 0; i < 256; i++) {
	if (isUPPER_LC(i))
	    PL_fold_locale[i] = toLOWER_LC(i);
	else if (isLOWER_LC(i))
	    PL_fold_locale[i] = toUPPER_LC(i);
	else
	    PL_fold_locale[i] = i;
    }

#endif /* USE_LOCALE_CTYPE */
    PERL_ARGS_ASSERT_NEW_CTYPE;
    PERL_UNUSED_ARG(newctype);
    PERL_UNUSED_CONTEXT;
}

/*
 * Set up for a new collation locale.
 */
void
Perl_new_collate(pTHX_ const char *newcoll)
{
#ifdef USE_LOCALE_COLLATE
    dVAR;

    if (! newcoll) {
	if (PL_collation_name) {
	    ++PL_collation_ix;
	    Safefree(PL_collation_name);
	    PL_collation_name = NULL;
	}
	PL_collation_standard = TRUE;
	PL_collxfrm_base = 0;
	PL_collxfrm_mult = 2;
	return;
    }

    if (! PL_collation_name || strNE(PL_collation_name, newcoll)) {
	++PL_collation_ix;
	Safefree(PL_collation_name);
	PL_collation_name = stdize_locale(savepv(newcoll));
	PL_collation_standard = ((*newcoll == 'C' && newcoll[1] == '\0')
				 || strEQ(newcoll, "POSIX"));

	{
	  /*  2: at most so many chars ('a', 'b'). */
	  /* 50: surely no system expands a char more. */
#define XFRMBUFSIZE  (2 * 50)
	  char xbuf[XFRMBUFSIZE];
	  const Size_t fa = strxfrm(xbuf, "a",  XFRMBUFSIZE);
	  const Size_t fb = strxfrm(xbuf, "ab", XFRMBUFSIZE);
	  const SSize_t mult = fb - fa;
	  if (mult < 1 && !(fa == 0 && fb == 0))
	      Perl_croak(aTHX_ "panic: strxfrm() gets absurd - a => %"UVuf", ab => %"UVuf,
			 (UV) fa, (UV) fb);
	  PL_collxfrm_base = (fa > (Size_t)mult) ? (fa - mult) : 0;
	  PL_collxfrm_mult = mult;
	}
    }

#endif /* USE_LOCALE_COLLATE */
}

/*
 * Initialize locale awareness.
 */
int
Perl_init_i18nl10n(pTHX_ int printwarn)
{
    int ok = 1;
    /* returns
     *    1 = set ok or not applicable,
     *    0 = fallback to C locale,
     *   -1 = fallback to C locale failed
     */

#if defined(USE_LOCALE)
    dVAR;

#ifdef USE_LOCALE_CTYPE
    char *curctype   = NULL;
#endif /* USE_LOCALE_CTYPE */
#ifdef USE_LOCALE_COLLATE
    char *curcoll    = NULL;
#endif /* USE_LOCALE_COLLATE */
#ifdef USE_LOCALE_NUMERIC
    char *curnum     = NULL;
#endif /* USE_LOCALE_NUMERIC */
#ifdef __GLIBC__
    char * const language   = PerlEnv_getenv("LANGUAGE");
#endif
    char * const lc_all     = PerlEnv_getenv("LC_ALL");
    char * const lang       = PerlEnv_getenv("LANG");
    bool setlocale_failure = FALSE;

#ifdef LOCALE_ENVIRON_REQUIRED

    /*
     * Ultrix setlocale(..., "") fails if there are no environment
     * variables from which to get a locale name.
     */

    bool done = FALSE;

#ifdef LC_ALL
    if (lang) {
	if (setlocale(LC_ALL, ""))
	    done = TRUE;
	else
	    setlocale_failure = TRUE;
    }
    if (!setlocale_failure) {
#ifdef USE_LOCALE_CTYPE
	Safefree(curctype);
	if (! (curctype =
	       setlocale(LC_CTYPE,
			 (!done && (lang || PerlEnv_getenv("LC_CTYPE")))
				    ? "" : NULL)))
	    setlocale_failure = TRUE;
	else
	    curctype = savepv(curctype);
#endif /* USE_LOCALE_CTYPE */
#ifdef USE_LOCALE_COLLATE
	Safefree(curcoll);
	if (! (curcoll =
	       setlocale(LC_COLLATE,
			 (!done && (lang || PerlEnv_getenv("LC_COLLATE")))
				   ? "" : NULL)))
	    setlocale_failure = TRUE;
	else
	    curcoll = savepv(curcoll);
#endif /* USE_LOCALE_COLLATE */
#ifdef USE_LOCALE_NUMERIC
	Safefree(curnum);
	if (! (curnum =
	       setlocale(LC_NUMERIC,
			 (!done && (lang || PerlEnv_getenv("LC_NUMERIC")))
				  ? "" : NULL)))
	    setlocale_failure = TRUE;
	else
	    curnum = savepv(curnum);
#endif /* USE_LOCALE_NUMERIC */
    }

#endif /* LC_ALL */

#endif /* !LOCALE_ENVIRON_REQUIRED */

#ifdef LC_ALL
    if (! setlocale(LC_ALL, ""))
	setlocale_failure = TRUE;
#endif /* LC_ALL */

    if (!setlocale_failure) {
#ifdef USE_LOCALE_CTYPE
	Safefree(curctype);
	if (! (curctype = setlocale(LC_CTYPE, "")))
	    setlocale_failure = TRUE;
	else
	    curctype = savepv(curctype);
#endif /* USE_LOCALE_CTYPE */
#ifdef USE_LOCALE_COLLATE
	Safefree(curcoll);
	if (! (curcoll = setlocale(LC_COLLATE, "")))
	    setlocale_failure = TRUE;
	else
	    curcoll = savepv(curcoll);
#endif /* USE_LOCALE_COLLATE */
#ifdef USE_LOCALE_NUMERIC
	Safefree(curnum);
	if (! (curnum = setlocale(LC_NUMERIC, "")))
	    setlocale_failure = TRUE;
	else
	    curnum = savepv(curnum);
#endif /* USE_LOCALE_NUMERIC */
    }

    if (setlocale_failure) {
	char *p;
	const bool locwarn = (printwarn > 1 ||
			(printwarn &&
			 (!(p = PerlEnv_getenv("PERL_BADLANG")) || atoi(p))));

	if (locwarn) {
#ifdef LC_ALL

	    PerlIO_printf(Perl_error_log,
	       "perl: warning: Setting locale failed.\n");

#else /* !LC_ALL */

	    PerlIO_printf(Perl_error_log,
	       "perl: warning: Setting locale failed for the categories:\n\t");
#ifdef USE_LOCALE_CTYPE
	    if (! curctype)
		PerlIO_printf(Perl_error_log, "LC_CTYPE ");
#endif /* USE_LOCALE_CTYPE */
#ifdef USE_LOCALE_COLLATE
	    if (! curcoll)
		PerlIO_printf(Perl_error_log, "LC_COLLATE ");
#endif /* USE_LOCALE_COLLATE */
#ifdef USE_LOCALE_NUMERIC
	    if (! curnum)
		PerlIO_printf(Perl_error_log, "LC_NUMERIC ");
#endif /* USE_LOCALE_NUMERIC */
	    PerlIO_printf(Perl_error_log, "\n");

#endif /* LC_ALL */

	    PerlIO_printf(Perl_error_log,
		"perl: warning: Please check that your locale settings:\n");

#ifdef __GLIBC__
	    PerlIO_printf(Perl_error_log,
			  "\tLANGUAGE = %c%s%c,\n",
			  language ? '"' : '(',
			  language ? language : "unset",
			  language ? '"' : ')');
#endif

	    PerlIO_printf(Perl_error_log,
			  "\tLC_ALL = %c%s%c,\n",
			  lc_all ? '"' : '(',
			  lc_all ? lc_all : "unset",
			  lc_all ? '"' : ')');

#if defined(USE_ENVIRON_ARRAY)
	    {
	      char **e;
	      for (e = environ; *e; e++) {
		  if (strnEQ(*e, "LC_", 3)
			&& strnNE(*e, "LC_ALL=", 7)
			&& (p = strchr(*e, '=')))
		      PerlIO_printf(Perl_error_log, "\t%.*s = \"%s\",\n",
				    (int)(p - *e), *e, p + 1);
	      }
	    }
#else
	    PerlIO_printf(Perl_error_log,
			  "\t(possibly more locale environment variables)\n");
#endif

	    PerlIO_printf(Perl_error_log,
			  "\tLANG = %c%s%c\n",
			  lang ? '"' : '(',
			  lang ? lang : "unset",
			  lang ? '"' : ')');

	    PerlIO_printf(Perl_error_log,
			  "    are supported and installed on your system.\n");
	}

#ifdef LC_ALL

	if (setlocale(LC_ALL, "C")) {
	    if (locwarn)
		PerlIO_printf(Perl_error_log,
      "perl: warning: Falling back to the standard locale (\"C\").\n");
	    ok = 0;
	}
	else {
	    if (locwarn)
		PerlIO_printf(Perl_error_log,
      "perl: warning: Failed to fall back to the standard locale (\"C\").\n");
	    ok = -1;
	}

#else /* ! LC_ALL */

	if (0
#ifdef USE_LOCALE_CTYPE
	    || !(curctype || setlocale(LC_CTYPE, "C"))
#endif /* USE_LOCALE_CTYPE */
#ifdef USE_LOCALE_COLLATE
	    || !(curcoll || setlocale(LC_COLLATE, "C"))
#endif /* USE_LOCALE_COLLATE */
#ifdef USE_LOCALE_NUMERIC
	    || !(curnum || setlocale(LC_NUMERIC, "C"))
#endif /* USE_LOCALE_NUMERIC */
	    )
	{
	    if (locwarn)
		PerlIO_printf(Perl_error_log,
      "perl: warning: Cannot fall back to the standard locale (\"C\").\n");
	    ok = -1;
	}

#endif /* ! LC_ALL */

#ifdef USE_LOCALE_CTYPE
	Safefree(curctype);
	curctype = savepv(setlocale(LC_CTYPE, NULL));
#endif /* USE_LOCALE_CTYPE */
#ifdef USE_LOCALE_COLLATE
	Safefree(curcoll);
	curcoll = savepv(setlocale(LC_COLLATE, NULL));
#endif /* USE_LOCALE_COLLATE */
#ifdef USE_LOCALE_NUMERIC
	Safefree(curnum);
	curnum = savepv(setlocale(LC_NUMERIC, NULL));
#endif /* USE_LOCALE_NUMERIC */
    }
    else {

#ifdef USE_LOCALE_CTYPE
    new_ctype(curctype);
#endif /* USE_LOCALE_CTYPE */

#ifdef USE_LOCALE_COLLATE
    new_collate(curcoll);
#endif /* USE_LOCALE_COLLATE */

#ifdef USE_LOCALE_NUMERIC
    new_numeric(curnum);
#endif /* USE_LOCALE_NUMERIC */

    }

#endif /* USE_LOCALE */

#ifdef USE_PERLIO
    {
      /* Set PL_utf8locale to TRUE if using PerlIO _and_
	 any of the following are true:
	 - nl_langinfo(CODESET) contains /^utf-?8/i
	 - $ENV{LC_ALL}   contains /^utf-?8/i
	 - $ENV{LC_CTYPE} contains /^utf-?8/i
	 - $ENV{LANG}     contains /^utf-?8/i
	 The LC_ALL, LC_CTYPE, LANG obey the usual override
	 hierarchy of locale environment variables.  (LANGUAGE
	 affects only LC_MESSAGES only under glibc.) (If present,
	 it overrides LC_MESSAGES for GNU gettext, and it also
	 can have more than one locale, separated by spaces,
	 in case you need to know.)
	 If PL_utf8locale and PL_unicode (set by -C or by $ENV{PERL_UNICODE})
         are true, perl.c:S_parse_body() will turn on the PerlIO :utf8 layer
	 on STDIN, STDOUT, STDERR, _and_ the default open discipline.
      */
	 bool utf8locale = FALSE;
	 char *codeset = NULL;
#if defined(HAS_NL_LANGINFO) && defined(CODESET)
	 codeset = nl_langinfo(CODESET);
#endif
	 if (codeset)
	      utf8locale = (foldEQ(codeset, STR_WITH_LEN("UTF-8"))
			    || foldEQ(codeset, STR_WITH_LEN("UTF8") ));
#if defined(USE_LOCALE)
	 else { /* nl_langinfo(CODESET) is supposed to correctly
		 * interpret the locale environment variables,
		 * but just in case it fails, let's do this manually. */ 
	      if (lang)
		   utf8locale = (foldEQ(lang, STR_WITH_LEN("UTF-8"))
				 || foldEQ(lang, STR_WITH_LEN("UTF8") ));
#ifdef USE_LOCALE_CTYPE
	      if (curctype)
		   utf8locale = (foldEQ(curctype, STR_WITH_LEN("UTF-8"))
				 || foldEQ(curctype, STR_WITH_LEN("UTF8") ));
#endif
	      if (lc_all)
		   utf8locale = (foldEQ(lc_all, STR_WITH_LEN("UTF-8"))
				 || foldEQ(lc_all, STR_WITH_LEN("UTF8") ));
	 }
#endif /* USE_LOCALE */
	 if (utf8locale)
	      PL_utf8locale = TRUE;
    }
    /* Set PL_unicode to $ENV{PERL_UNICODE} if using PerlIO.
       This is an alternative to using the -C command line switch
       (the -C if present will override this). */
    {
	 const char *p = PerlEnv_getenv("PERL_UNICODE");
	 PL_unicode = p ? parse_unicode_opts(&p) : 0;
	 if (PL_unicode & PERL_UNICODE_UTF8CACHEASSERT_FLAG)
	     PL_utf8cache = -1;
    }
#endif

#ifdef USE_LOCALE_CTYPE
    Safefree(curctype);
#endif /* USE_LOCALE_CTYPE */
#ifdef USE_LOCALE_COLLATE
    Safefree(curcoll);
#endif /* USE_LOCALE_COLLATE */
#ifdef USE_LOCALE_NUMERIC
    Safefree(curnum);
#endif /* USE_LOCALE_NUMERIC */
    return ok;
}

#ifdef USE_LOCALE_COLLATE

/*
 * mem_collxfrm() is a bit like strxfrm() but with two important
 * differences. First, it handles embedded NULs. Second, it allocates
 * a bit more memory than needed for the transformed data itself.
 * The real transformed data begins at offset sizeof(collationix).
 * Please see sv_collxfrm() to see how this is used.
 */

char *
Perl_mem_collxfrm(pTHX_ const char *s, STRLEN len, STRLEN *xlen)
{
    dVAR;
    char *xbuf;
    STRLEN xAlloc, xin, xout; /* xalloc is a reserved word in VC */

    PERL_ARGS_ASSERT_MEM_COLLXFRM;

    /* the first sizeof(collationix) bytes are used by sv_collxfrm(). */
    /* the +1 is for the terminating NUL. */

    xAlloc = sizeof(PL_collation_ix) + PL_collxfrm_base + (PL_collxfrm_mult * len) + 1;
    Newx(xbuf, xAlloc, char);
    if (! xbuf)
	goto bad;

    *(U32*)xbuf = PL_collation_ix;
    xout = sizeof(PL_collation_ix);
    for (xin = 0; xin < len; ) {
	Size_t xused;

	for (;;) {
	    xused = strxfrm(xbuf + xout, s + xin, xAlloc - xout);
	    if (xused >= PERL_INT_MAX)
		goto bad;
	    if ((STRLEN)xused < xAlloc - xout)
		break;
	    xAlloc = (2 * xAlloc) + 1;
	    Renew(xbuf, xAlloc, char);
	    if (! xbuf)
		goto bad;
	}

	xin += strlen(s + xin) + 1;
	xout += xused;

	/* Embedded NULs are understood but silently skipped
	 * because they make no sense in locale collation. */
    }

    xbuf[xout] = '\0';
    *xlen = xout - sizeof(PL_collation_ix);
    return xbuf;

  bad:
    Safefree(xbuf);
    *xlen = 0;
    return NULL;
}

#endif /* USE_LOCALE_COLLATE */

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
