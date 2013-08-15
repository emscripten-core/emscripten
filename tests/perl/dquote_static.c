/*    dquote_static.c
 *
 * This file contains static functions that are related to
 * parsing double-quotish expressions, but are used in more than
 * one file.
 *
 * It is currently #included by regcomp.c and toke.c.
*/

#define PERL_IN_DQUOTE_STATIC_C
#include "proto.h"
#include "embed.h"

/*
 - regcurly - a little FSA that accepts {\d+,?\d*}
    Pulled from regcomp.c.
 */
PERL_STATIC_INLINE I32
S_regcurly(pTHX_ register const char *s)
{
    PERL_ARGS_ASSERT_REGCURLY;

    if (*s++ != '{')
	return FALSE;
    if (!isDIGIT(*s))
	return FALSE;
    while (isDIGIT(*s))
	s++;
    if (*s == ',') {
	s++;
	while (isDIGIT(*s))
	    s++;
    }
    if (*s != '}')
	return FALSE;
    return TRUE;
}

/* XXX Add documentation after final interface and behavior is decided */
/* May want to show context for error, so would pass Perl_bslash_c(pTHX_ const char* current, const char* start, const bool output_warning)
    U8 source = *current;
*/

STATIC char
S_grok_bslash_c(pTHX_ const char source, const bool utf8, const bool output_warning)
{

    U8 result;

    if (utf8) {
	/* Trying to deprecate non-ASCII usages.  This construct has never
	 * worked for a utf8 variant.  So, even though are accepting non-ASCII
	 * Latin1 in 5.14, no need to make them work under utf8 */
	if (! isASCII(source)) {
	    Perl_croak(aTHX_ "Character following \"\\c\" must be ASCII");
	}
    }

    result = toCTRL(source);
    if (! isASCII(source)) {
	    Perl_ck_warner_d(aTHX_ packWARN2(WARN_DEPRECATED, WARN_SYNTAX),
			    "Character following \"\\c\" must be ASCII");
    }
    else if (! isCNTRL(result) && output_warning) {
	if (source == '{') {
	    Perl_ck_warner_d(aTHX_ packWARN2(WARN_DEPRECATED, WARN_SYNTAX),
			    "\"\\c{\" is deprecated and is more clearly written as \";\"");
	}
	else {
	    U8 clearer[3];
	    U8 i = 0;
	    if (! isALNUM(result)) {
		clearer[i++] = '\\';
	    }
	    clearer[i++] = result;
	    clearer[i++] = '\0';

	    Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
			    "\"\\c%c\" is more clearly written simply as \"%s\"",
			    source,
			    clearer);
	}
    }

    return result;
}

STATIC bool
S_grok_bslash_o(pTHX_ const char *s,
			 UV *uv,
			 STRLEN *len,
			 const char** error_msg,
			 const bool output_warning)
{

/*  Documentation to be supplied when interface nailed down finally
 *  This returns FALSE if there is an error which the caller need not recover
 *  from; , otherwise TRUE.  In either case the caller should look at *len
 *  On input:
 *	s   points to a string that begins with 'o', and the previous character
 *	    was a backslash.
 *	uv  points to a UV that will hold the output value, valid only if the
 *	    return from the function is TRUE
 *	len on success will point to the next character in the string past the
 *		       end of this construct.
 *	    on failure, it will point to the failure
 *      error_msg is a pointer that will be set to an internal buffer giving an
 *	    error message upon failure (the return is FALSE).  Untouched if
 *	    function succeeds
 *	output_warning says whether to output any warning messages, or suppress
 *	    them
 */
    const char* e;
    STRLEN numbers_len;
    I32 flags = PERL_SCAN_ALLOW_UNDERSCORES
		| PERL_SCAN_DISALLOW_PREFIX
		/* XXX Until the message is improved in grok_oct, handle errors
		 * ourselves */
	        | PERL_SCAN_SILENT_ILLDIGIT;

    PERL_ARGS_ASSERT_GROK_BSLASH_O;


    assert(*s == 'o');
    s++;

    if (*s != '{') {
	*len = 1;	/* Move past the o */
	*error_msg = "Missing braces on \\o{}";
	return FALSE;
    }

    e = strchr(s, '}');
    if (!e) {
	*len = 2;	/* Move past the o{ */
	*error_msg = "Missing right brace on \\o{";
	return FALSE;
    }

    /* Return past the '}' no matter what is inside the braces */
    *len = e - s + 2;	/* 2 = 1 for the o + 1 for the '}' */

    s++;    /* Point to first digit */

    numbers_len = e - s;
    if (numbers_len == 0) {
	*error_msg = "Number with no digits";
	return FALSE;
    }

    *uv = NATIVE_TO_UNI(grok_oct(s, &numbers_len, &flags, NULL));
    /* Note that if has non-octal, will ignore everything starting with that up
     * to the '}' */

    if (output_warning && numbers_len != (STRLEN) (e - s)) {
	Perl_ck_warner(aTHX_ packWARN(WARN_DIGIT),
	/* diag_listed_as: Non-octal character '%c'.  Resolved as "%s" */
		       "Non-octal character '%c'.  Resolved as \"\\o{%.*s}\"",
		       *(s + numbers_len),
		       (int) numbers_len,
		       s);
    }

    return TRUE;
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
