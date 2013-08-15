/*    regcomp.c
 */

/*
 * 'A fair jaw-cracker dwarf-language must be.'            --Samwise Gamgee
 *
 *     [p.285 of _The Lord of the Rings_, II/iii: "The Ring Goes South"]
 */

/* This file contains functions for compiling a regular expression.  See
 * also regexec.c which funnily enough, contains functions for executing
 * a regular expression.
 *
 * This file is also copied at build time to ext/re/re_comp.c, where
 * it's built with -DPERL_EXT_RE_BUILD -DPERL_EXT_RE_DEBUG -DPERL_EXT.
 * This causes the main functions to be compiled under new names and with
 * debugging support added, which makes "use re 'debug'" work.
 */

/* NOTE: this is derived from Henry Spencer's regexp code, and should not
 * confused with the original package (see point 3 below).  Thanks, Henry!
 */

/* Additional note: this code is very heavily munged from Henry's version
 * in places.  In some spots I've traded clarity for efficiency, so don't
 * blame Henry for some of the lack of readability.
 */

/* The names of the functions have been changed from regcomp and
 * regexec to pregcomp and pregexec in order to avoid conflicts
 * with the POSIX routines of the same names.
*/

#ifdef PERL_EXT_RE_BUILD
#include "re_top.h"
#endif

/*
 * pregcomp and pregexec -- regsub and regerror are not used in perl
 *
 *	Copyright (c) 1986 by University of Toronto.
 *	Written by Henry Spencer.  Not derived from licensed software.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to redistribute it freely,
 *	subject to the following restrictions:
 *
 *	1. The author is not responsible for the consequences of use of
 *		this software, no matter how awful, even if they arise
 *		from defects in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *		by explicit claim or by omission.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *		be misrepresented as being the original software.
 *
 *
 ****    Alterations to Henry's code are...
 ****
 ****    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
 ****    2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
 ****    by Larry Wall and others
 ****
 ****    You may distribute under the terms of either the GNU General Public
 ****    License or the Artistic License, as specified in the README file.

 *
 * Beware that some of this code is subtly aware of the way operator
 * precedence is structured in regular expressions.  Serious changes in
 * regular-expression syntax might require a total rethink.
 */
#include "EXTERN.h"
#define PERL_IN_REGCOMP_C
#include "perl.h"

#ifndef PERL_IN_XSUB_RE
#  include "INTERN.h"
#endif

#define REG_COMP_C
#ifdef PERL_IN_XSUB_RE
#  include "re_comp.h"
#else
#  include "regcomp.h"
#endif

#include "dquote_static.c"
#ifndef PERL_IN_XSUB_RE
#  include "charclass_invlists.h"
#endif

#ifdef op
#undef op
#endif /* op */

#ifdef MSDOS
#  if defined(BUGGY_MSC6)
 /* MSC 6.00A breaks on op/regexp.t test 85 unless we turn this off */
#    pragma optimize("a",off)
 /* But MSC 6.00A is happy with 'w', for aliases only across function calls*/
#    pragma optimize("w",on )
#  endif /* BUGGY_MSC6 */
#endif /* MSDOS */

#ifndef STATIC
#define	STATIC	static
#endif

typedef struct RExC_state_t {
    U32		flags;			/* are we folding, multilining? */
    char	*precomp;		/* uncompiled string. */
    REGEXP	*rx_sv;			/* The SV that is the regexp. */
    regexp	*rx;                    /* perl core regexp structure */
    regexp_internal	*rxi;           /* internal data for regexp object pprivate field */        
    char	*start;			/* Start of input for compile */
    char	*end;			/* End of input for compile */
    char	*parse;			/* Input-scan pointer. */
    I32		whilem_seen;		/* number of WHILEM in this expr */
    regnode	*emit_start;		/* Start of emitted-code area */
    regnode	*emit_bound;		/* First regnode outside of the allocated space */
    regnode	*emit;			/* Code-emit pointer; &regdummy = don't = compiling */
    I32		naughty;		/* How bad is this pattern? */
    I32		sawback;		/* Did we see \1, ...? */
    U32		seen;
    I32		size;			/* Code size. */
    I32		npar;			/* Capture buffer count, (OPEN). */
    I32		cpar;			/* Capture buffer count, (CLOSE). */
    I32		nestroot;		/* root parens we are in - used by accept */
    I32		extralen;
    I32		seen_zerolen;
    I32		seen_evals;
    regnode	**open_parens;		/* pointers to open parens */
    regnode	**close_parens;		/* pointers to close parens */
    regnode	*opend;			/* END node in program */
    I32		utf8;		/* whether the pattern is utf8 or not */
    I32		orig_utf8;	/* whether the pattern was originally in utf8 */
				/* XXX use this for future optimisation of case
				 * where pattern must be upgraded to utf8. */
    I32		uni_semantics;	/* If a d charset modifier should use unicode
				   rules, even if the pattern is not in
				   utf8 */
    HV		*paren_names;		/* Paren names */
    
    regnode	**recurse;		/* Recurse regops */
    I32		recurse_count;		/* Number of recurse regops */
    I32		in_lookbehind;
    I32		contains_locale;
    I32		override_recoding;
#if ADD_TO_REGEXEC
    char 	*starttry;		/* -Dr: where regtry was called. */
#define RExC_starttry	(pRExC_state->starttry)
#endif
#ifdef DEBUGGING
    const char  *lastparse;
    I32         lastnum;
    AV          *paren_name_list;       /* idx -> name */
#define RExC_lastparse	(pRExC_state->lastparse)
#define RExC_lastnum	(pRExC_state->lastnum)
#define RExC_paren_name_list    (pRExC_state->paren_name_list)
#endif
} RExC_state_t;

#define RExC_flags	(pRExC_state->flags)
#define RExC_precomp	(pRExC_state->precomp)
#define RExC_rx_sv	(pRExC_state->rx_sv)
#define RExC_rx		(pRExC_state->rx)
#define RExC_rxi	(pRExC_state->rxi)
#define RExC_start	(pRExC_state->start)
#define RExC_end	(pRExC_state->end)
#define RExC_parse	(pRExC_state->parse)
#define RExC_whilem_seen	(pRExC_state->whilem_seen)
#ifdef RE_TRACK_PATTERN_OFFSETS
#define RExC_offsets	(pRExC_state->rxi->u.offsets) /* I am not like the others */
#endif
#define RExC_emit	(pRExC_state->emit)
#define RExC_emit_start	(pRExC_state->emit_start)
#define RExC_emit_bound	(pRExC_state->emit_bound)
#define RExC_naughty	(pRExC_state->naughty)
#define RExC_sawback	(pRExC_state->sawback)
#define RExC_seen	(pRExC_state->seen)
#define RExC_size	(pRExC_state->size)
#define RExC_npar	(pRExC_state->npar)
#define RExC_nestroot   (pRExC_state->nestroot)
#define RExC_extralen	(pRExC_state->extralen)
#define RExC_seen_zerolen	(pRExC_state->seen_zerolen)
#define RExC_seen_evals	(pRExC_state->seen_evals)
#define RExC_utf8	(pRExC_state->utf8)
#define RExC_uni_semantics	(pRExC_state->uni_semantics)
#define RExC_orig_utf8	(pRExC_state->orig_utf8)
#define RExC_open_parens	(pRExC_state->open_parens)
#define RExC_close_parens	(pRExC_state->close_parens)
#define RExC_opend	(pRExC_state->opend)
#define RExC_paren_names	(pRExC_state->paren_names)
#define RExC_recurse	(pRExC_state->recurse)
#define RExC_recurse_count	(pRExC_state->recurse_count)
#define RExC_in_lookbehind	(pRExC_state->in_lookbehind)
#define RExC_contains_locale	(pRExC_state->contains_locale)
#define RExC_override_recoding	(pRExC_state->override_recoding)


#define	ISMULT1(c)	((c) == '*' || (c) == '+' || (c) == '?')
#define	ISMULT2(s)	((*s) == '*' || (*s) == '+' || (*s) == '?' || \
	((*s) == '{' && regcurly(s)))

#ifdef SPSTART
#undef SPSTART		/* dratted cpp namespace... */
#endif
/*
 * Flags to be passed up and down.
 */
#define	WORST		0	/* Worst case. */
#define	HASWIDTH	0x01	/* Known to match non-null strings. */

/* Simple enough to be STAR/PLUS operand, in an EXACT node must be a single
 * character, and if utf8, must be invariant.  Note that this is not the same thing as REGNODE_SIMPLE */
#define	SIMPLE		0x02
#define	SPSTART		0x04	/* Starts with * or +. */
#define TRYAGAIN	0x08	/* Weeded out a declaration. */
#define POSTPONED	0x10    /* (?1),(?&name), (??{...}) or similar */

#define REG_NODE_NUM(x) ((x) ? (int)((x)-RExC_emit_start) : -1)

/* whether trie related optimizations are enabled */
#if PERL_ENABLE_EXTENDED_TRIE_OPTIMISATION
#define TRIE_STUDY_OPT
#define FULL_TRIE_STUDY
#define TRIE_STCLASS
#endif



#define PBYTE(u8str,paren) ((U8*)(u8str))[(paren) >> 3]
#define PBITVAL(paren) (1 << ((paren) & 7))
#define PAREN_TEST(u8str,paren) ( PBYTE(u8str,paren) & PBITVAL(paren))
#define PAREN_SET(u8str,paren) PBYTE(u8str,paren) |= PBITVAL(paren)
#define PAREN_UNSET(u8str,paren) PBYTE(u8str,paren) &= (~PBITVAL(paren))

/* If not already in utf8, do a longjmp back to the beginning */
#define UTF8_LONGJMP 42 /* Choose a value not likely to ever conflict */
#define REQUIRE_UTF8	STMT_START {                                       \
                                     if (! UTF) JMPENV_JUMP(UTF8_LONGJMP); \
                        } STMT_END

/* About scan_data_t.

  During optimisation we recurse through the regexp program performing
  various inplace (keyhole style) optimisations. In addition study_chunk
  and scan_commit populate this data structure with information about
  what strings MUST appear in the pattern. We look for the longest 
  string that must appear at a fixed location, and we look for the
  longest string that may appear at a floating location. So for instance
  in the pattern:
  
    /FOO[xX]A.*B[xX]BAR/
    
  Both 'FOO' and 'A' are fixed strings. Both 'B' and 'BAR' are floating
  strings (because they follow a .* construct). study_chunk will identify
  both FOO and BAR as being the longest fixed and floating strings respectively.
  
  The strings can be composites, for instance
  
     /(f)(o)(o)/
     
  will result in a composite fixed substring 'foo'.
  
  For each string some basic information is maintained:
  
  - offset or min_offset
    This is the position the string must appear at, or not before.
    It also implicitly (when combined with minlenp) tells us how many
    characters must match before the string we are searching for.
    Likewise when combined with minlenp and the length of the string it
    tells us how many characters must appear after the string we have 
    found.
  
  - max_offset
    Only used for floating strings. This is the rightmost point that
    the string can appear at. If set to I32 max it indicates that the
    string can occur infinitely far to the right.
  
  - minlenp
    A pointer to the minimum length of the pattern that the string 
    was found inside. This is important as in the case of positive 
    lookahead or positive lookbehind we can have multiple patterns 
    involved. Consider
    
    /(?=FOO).*F/
    
    The minimum length of the pattern overall is 3, the minimum length
    of the lookahead part is 3, but the minimum length of the part that
    will actually match is 1. So 'FOO's minimum length is 3, but the 
    minimum length for the F is 1. This is important as the minimum length
    is used to determine offsets in front of and behind the string being 
    looked for.  Since strings can be composites this is the length of the
    pattern at the time it was committed with a scan_commit. Note that
    the length is calculated by study_chunk, so that the minimum lengths
    are not known until the full pattern has been compiled, thus the 
    pointer to the value.
  
  - lookbehind
  
    In the case of lookbehind the string being searched for can be
    offset past the start point of the final matching string. 
    If this value was just blithely removed from the min_offset it would
    invalidate some of the calculations for how many chars must match
    before or after (as they are derived from min_offset and minlen and
    the length of the string being searched for). 
    When the final pattern is compiled and the data is moved from the
    scan_data_t structure into the regexp structure the information
    about lookbehind is factored in, with the information that would 
    have been lost precalculated in the end_shift field for the 
    associated string.

  The fields pos_min and pos_delta are used to store the minimum offset
  and the delta to the maximum offset at the current point in the pattern.    

*/

typedef struct scan_data_t {
    /*I32 len_min;      unused */
    /*I32 len_delta;    unused */
    I32 pos_min;
    I32 pos_delta;
    SV *last_found;
    I32 last_end;	    /* min value, <0 unless valid. */
    I32 last_start_min;
    I32 last_start_max;
    SV **longest;	    /* Either &l_fixed, or &l_float. */
    SV *longest_fixed;      /* longest fixed string found in pattern */
    I32 offset_fixed;       /* offset where it starts */
    I32 *minlen_fixed;      /* pointer to the minlen relevant to the string */
    I32 lookbehind_fixed;   /* is the position of the string modfied by LB */
    SV *longest_float;      /* longest floating string found in pattern */
    I32 offset_float_min;   /* earliest point in string it can appear */
    I32 offset_float_max;   /* latest point in string it can appear */
    I32 *minlen_float;      /* pointer to the minlen relevant to the string */
    I32 lookbehind_float;   /* is the position of the string modified by LB */
    I32 flags;
    I32 whilem_c;
    I32 *last_closep;
    struct regnode_charclass_class *start_class;
} scan_data_t;

/*
 * Forward declarations for pregcomp()'s friends.
 */

static const scan_data_t zero_scan_data =
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0};

#define SF_BEFORE_EOL		(SF_BEFORE_SEOL|SF_BEFORE_MEOL)
#define SF_BEFORE_SEOL		0x0001
#define SF_BEFORE_MEOL		0x0002
#define SF_FIX_BEFORE_EOL	(SF_FIX_BEFORE_SEOL|SF_FIX_BEFORE_MEOL)
#define SF_FL_BEFORE_EOL	(SF_FL_BEFORE_SEOL|SF_FL_BEFORE_MEOL)

#ifdef NO_UNARY_PLUS
#  define SF_FIX_SHIFT_EOL	(0+2)
#  define SF_FL_SHIFT_EOL		(0+4)
#else
#  define SF_FIX_SHIFT_EOL	(+2)
#  define SF_FL_SHIFT_EOL		(+4)
#endif

#define SF_FIX_BEFORE_SEOL	(SF_BEFORE_SEOL << SF_FIX_SHIFT_EOL)
#define SF_FIX_BEFORE_MEOL	(SF_BEFORE_MEOL << SF_FIX_SHIFT_EOL)

#define SF_FL_BEFORE_SEOL	(SF_BEFORE_SEOL << SF_FL_SHIFT_EOL)
#define SF_FL_BEFORE_MEOL	(SF_BEFORE_MEOL << SF_FL_SHIFT_EOL) /* 0x20 */
#define SF_IS_INF		0x0040
#define SF_HAS_PAR		0x0080
#define SF_IN_PAR		0x0100
#define SF_HAS_EVAL		0x0200
#define SCF_DO_SUBSTR		0x0400
#define SCF_DO_STCLASS_AND	0x0800
#define SCF_DO_STCLASS_OR	0x1000
#define SCF_DO_STCLASS		(SCF_DO_STCLASS_AND|SCF_DO_STCLASS_OR)
#define SCF_WHILEM_VISITED_POS	0x2000

#define SCF_TRIE_RESTUDY        0x4000 /* Do restudy? */
#define SCF_SEEN_ACCEPT         0x8000 

#define UTF cBOOL(RExC_utf8)

/* The enums for all these are ordered so things work out correctly */
#define LOC (get_regex_charset(RExC_flags) == REGEX_LOCALE_CHARSET)
#define DEPENDS_SEMANTICS (get_regex_charset(RExC_flags) == REGEX_DEPENDS_CHARSET)
#define UNI_SEMANTICS (get_regex_charset(RExC_flags) == REGEX_UNICODE_CHARSET)
#define AT_LEAST_UNI_SEMANTICS (get_regex_charset(RExC_flags) >= REGEX_UNICODE_CHARSET)
#define ASCII_RESTRICTED (get_regex_charset(RExC_flags) == REGEX_ASCII_RESTRICTED_CHARSET)
#define MORE_ASCII_RESTRICTED (get_regex_charset(RExC_flags) == REGEX_ASCII_MORE_RESTRICTED_CHARSET)
#define AT_LEAST_ASCII_RESTRICTED (get_regex_charset(RExC_flags) >= REGEX_ASCII_RESTRICTED_CHARSET)

#define FOLD cBOOL(RExC_flags & RXf_PMf_FOLD)

#define OOB_UNICODE		12345678
#define OOB_NAMEDCLASS		-1

#define CHR_SVLEN(sv) (UTF ? sv_len_utf8(sv) : SvCUR(sv))
#define CHR_DIST(a,b) (UTF ? utf8_distance(a,b) : a - b)


/* length of regex to show in messages that don't mark a position within */
#define RegexLengthToShowInErrorMessages 127

/*
 * If MARKER[12] are adjusted, be sure to adjust the constants at the top
 * of t/op/regmesg.t, the tests in t/op/re_tests, and those in
 * op/pragma/warn/regcomp.
 */
#define MARKER1 "<-- HERE"    /* marker as it appears in the description */
#define MARKER2 " <-- HERE "  /* marker as it appears within the regex */

#define REPORT_LOCATION " in regex; marked by " MARKER1 " in m/%.*s" MARKER2 "%s/"

/*
 * Calls SAVEDESTRUCTOR_X if needed, then calls Perl_croak with the given
 * arg. Show regex, up to a maximum length. If it's too long, chop and add
 * "...".
 */
#define _FAIL(code) STMT_START {					\
    const char *ellipses = "";						\
    IV len = RExC_end - RExC_precomp;					\
									\
    if (!SIZE_ONLY)							\
	SAVEDESTRUCTOR_X(clear_re,(void*)RExC_rx_sv);			\
    if (len > RegexLengthToShowInErrorMessages) {			\
	/* chop 10 shorter than the max, to ensure meaning of "..." */	\
	len = RegexLengthToShowInErrorMessages - 10;			\
	ellipses = "...";						\
    }									\
    code;                                                               \
} STMT_END

#define	FAIL(msg) _FAIL(			    \
    Perl_croak(aTHX_ "%s in regex m/%.*s%s/",	    \
	    msg, (int)len, RExC_precomp, ellipses))

#define	FAIL2(msg,arg) _FAIL(			    \
    Perl_croak(aTHX_ msg " in regex m/%.*s%s/",	    \
	    arg, (int)len, RExC_precomp, ellipses))

/*
 * Simple_vFAIL -- like FAIL, but marks the current location in the scan
 */
#define	Simple_vFAIL(m) STMT_START {					\
    const IV offset = RExC_parse - RExC_precomp;			\
    Perl_croak(aTHX_ "%s" REPORT_LOCATION,				\
	    m, (int)offset, RExC_precomp, RExC_precomp + offset);	\
} STMT_END

/*
 * Calls SAVEDESTRUCTOR_X if needed, then Simple_vFAIL()
 */
#define	vFAIL(m) STMT_START {				\
    if (!SIZE_ONLY)					\
	SAVEDESTRUCTOR_X(clear_re,(void*)RExC_rx_sv);	\
    Simple_vFAIL(m);					\
} STMT_END

/*
 * Like Simple_vFAIL(), but accepts two arguments.
 */
#define	Simple_vFAIL2(m,a1) STMT_START {			\
    const IV offset = RExC_parse - RExC_precomp;			\
    S_re_croak2(aTHX_ m, REPORT_LOCATION, a1,			\
	    (int)offset, RExC_precomp, RExC_precomp + offset);	\
} STMT_END

/*
 * Calls SAVEDESTRUCTOR_X if needed, then Simple_vFAIL2().
 */
#define	vFAIL2(m,a1) STMT_START {			\
    if (!SIZE_ONLY)					\
	SAVEDESTRUCTOR_X(clear_re,(void*)RExC_rx_sv);	\
    Simple_vFAIL2(m, a1);				\
} STMT_END


/*
 * Like Simple_vFAIL(), but accepts three arguments.
 */
#define	Simple_vFAIL3(m, a1, a2) STMT_START {			\
    const IV offset = RExC_parse - RExC_precomp;		\
    S_re_croak2(aTHX_ m, REPORT_LOCATION, a1, a2,		\
	    (int)offset, RExC_precomp, RExC_precomp + offset);	\
} STMT_END

/*
 * Calls SAVEDESTRUCTOR_X if needed, then Simple_vFAIL3().
 */
#define	vFAIL3(m,a1,a2) STMT_START {			\
    if (!SIZE_ONLY)					\
	SAVEDESTRUCTOR_X(clear_re,(void*)RExC_rx_sv);	\
    Simple_vFAIL3(m, a1, a2);				\
} STMT_END

/*
 * Like Simple_vFAIL(), but accepts four arguments.
 */
#define	Simple_vFAIL4(m, a1, a2, a3) STMT_START {		\
    const IV offset = RExC_parse - RExC_precomp;		\
    S_re_croak2(aTHX_ m, REPORT_LOCATION, a1, a2, a3,		\
	    (int)offset, RExC_precomp, RExC_precomp + offset);	\
} STMT_END

#define	ckWARNreg(loc,m) STMT_START {					\
    const IV offset = loc - RExC_precomp;				\
    Perl_ck_warner(aTHX_ packWARN(WARN_REGEXP), m REPORT_LOCATION,	\
	    (int)offset, RExC_precomp, RExC_precomp + offset);		\
} STMT_END

#define	ckWARNregdep(loc,m) STMT_START {				\
    const IV offset = loc - RExC_precomp;				\
    Perl_ck_warner_d(aTHX_ packWARN2(WARN_DEPRECATED, WARN_REGEXP),	\
	    m REPORT_LOCATION,						\
	    (int)offset, RExC_precomp, RExC_precomp + offset);		\
} STMT_END

#define	ckWARN2regdep(loc,m, a1) STMT_START {				\
    const IV offset = loc - RExC_precomp;				\
    Perl_ck_warner_d(aTHX_ packWARN2(WARN_DEPRECATED, WARN_REGEXP),	\
	    m REPORT_LOCATION,						\
	    a1, (int)offset, RExC_precomp, RExC_precomp + offset);	\
} STMT_END

#define	ckWARN2reg(loc, m, a1) STMT_START {				\
    const IV offset = loc - RExC_precomp;				\
    Perl_ck_warner(aTHX_ packWARN(WARN_REGEXP), m REPORT_LOCATION,	\
	    a1, (int)offset, RExC_precomp, RExC_precomp + offset);	\
} STMT_END

#define	vWARN3(loc, m, a1, a2) STMT_START {				\
    const IV offset = loc - RExC_precomp;				\
    Perl_warner(aTHX_ packWARN(WARN_REGEXP), m REPORT_LOCATION,		\
	    a1, a2, (int)offset, RExC_precomp, RExC_precomp + offset);	\
} STMT_END

#define	ckWARN3reg(loc, m, a1, a2) STMT_START {				\
    const IV offset = loc - RExC_precomp;				\
    Perl_ck_warner(aTHX_ packWARN(WARN_REGEXP), m REPORT_LOCATION,	\
	    a1, a2, (int)offset, RExC_precomp, RExC_precomp + offset);	\
} STMT_END

#define	vWARN4(loc, m, a1, a2, a3) STMT_START {				\
    const IV offset = loc - RExC_precomp;				\
    Perl_warner(aTHX_ packWARN(WARN_REGEXP), m REPORT_LOCATION,		\
	    a1, a2, a3, (int)offset, RExC_precomp, RExC_precomp + offset); \
} STMT_END

#define	ckWARN4reg(loc, m, a1, a2, a3) STMT_START {			\
    const IV offset = loc - RExC_precomp;				\
    Perl_ck_warner(aTHX_ packWARN(WARN_REGEXP), m REPORT_LOCATION,	\
	    a1, a2, a3, (int)offset, RExC_precomp, RExC_precomp + offset); \
} STMT_END

#define	vWARN5(loc, m, a1, a2, a3, a4) STMT_START {			\
    const IV offset = loc - RExC_precomp;				\
    Perl_warner(aTHX_ packWARN(WARN_REGEXP), m REPORT_LOCATION,		\
	    a1, a2, a3, a4, (int)offset, RExC_precomp, RExC_precomp + offset); \
} STMT_END


/* Allow for side effects in s */
#define REGC(c,s) STMT_START {			\
    if (!SIZE_ONLY) *(s) = (c); else (void)(s);	\
} STMT_END

/* Macros for recording node offsets.   20001227 mjd@plover.com 
 * Nodes are numbered 1, 2, 3, 4.  Node #n's position is recorded in
 * element 2*n-1 of the array.  Element #2n holds the byte length node #n.
 * Element 0 holds the number n.
 * Position is 1 indexed.
 */
#ifndef RE_TRACK_PATTERN_OFFSETS
#define Set_Node_Offset_To_R(node,byte)
#define Set_Node_Offset(node,byte)
#define Set_Cur_Node_Offset
#define Set_Node_Length_To_R(node,len)
#define Set_Node_Length(node,len)
#define Set_Node_Cur_Length(node)
#define Node_Offset(n) 
#define Node_Length(n) 
#define Set_Node_Offset_Length(node,offset,len)
#define ProgLen(ri) ri->u.proglen
#define SetProgLen(ri,x) ri->u.proglen = x
#else
#define ProgLen(ri) ri->u.offsets[0]
#define SetProgLen(ri,x) ri->u.offsets[0] = x
#define Set_Node_Offset_To_R(node,byte) STMT_START {			\
    if (! SIZE_ONLY) {							\
	MJD_OFFSET_DEBUG(("** (%d) offset of node %d is %d.\n",		\
		    __LINE__, (int)(node), (int)(byte)));		\
	if((node) < 0) {						\
	    Perl_croak(aTHX_ "value of node is %d in Offset macro", (int)(node)); \
	} else {							\
	    RExC_offsets[2*(node)-1] = (byte);				\
	}								\
    }									\
} STMT_END

#define Set_Node_Offset(node,byte) \
    Set_Node_Offset_To_R((node)-RExC_emit_start, (byte)-RExC_start)
#define Set_Cur_Node_Offset Set_Node_Offset(RExC_emit, RExC_parse)

#define Set_Node_Length_To_R(node,len) STMT_START {			\
    if (! SIZE_ONLY) {							\
	MJD_OFFSET_DEBUG(("** (%d) size of node %d is %d.\n",		\
		__LINE__, (int)(node), (int)(len)));			\
	if((node) < 0) {						\
	    Perl_croak(aTHX_ "value of node is %d in Length macro", (int)(node)); \
	} else {							\
	    RExC_offsets[2*(node)] = (len);				\
	}								\
    }									\
} STMT_END

#define Set_Node_Length(node,len) \
    Set_Node_Length_To_R((node)-RExC_emit_start, len)
#define Set_Cur_Node_Length(len) Set_Node_Length(RExC_emit, len)
#define Set_Node_Cur_Length(node) \
    Set_Node_Length(node, RExC_parse - parse_start)

/* Get offsets and lengths */
#define Node_Offset(n) (RExC_offsets[2*((n)-RExC_emit_start)-1])
#define Node_Length(n) (RExC_offsets[2*((n)-RExC_emit_start)])

#define Set_Node_Offset_Length(node,offset,len) STMT_START {	\
    Set_Node_Offset_To_R((node)-RExC_emit_start, (offset));	\
    Set_Node_Length_To_R((node)-RExC_emit_start, (len));	\
} STMT_END
#endif

#if PERL_ENABLE_EXPERIMENTAL_REGEX_OPTIMISATIONS
#define EXPERIMENTAL_INPLACESCAN
#endif /*PERL_ENABLE_EXPERIMENTAL_REGEX_OPTIMISATIONS*/

#define DEBUG_STUDYDATA(str,data,depth)                              \
DEBUG_OPTIMISE_MORE_r(if(data){                                      \
    PerlIO_printf(Perl_debug_log,                                    \
        "%*s" str "Pos:%"IVdf"/%"IVdf                                \
        " Flags: 0x%"UVXf" Whilem_c: %"IVdf" Lcp: %"IVdf" %s",       \
        (int)(depth)*2, "",                                          \
        (IV)((data)->pos_min),                                       \
        (IV)((data)->pos_delta),                                     \
        (UV)((data)->flags),                                         \
        (IV)((data)->whilem_c),                                      \
        (IV)((data)->last_closep ? *((data)->last_closep) : -1),     \
        is_inf ? "INF " : ""                                         \
    );                                                               \
    if ((data)->last_found)                                          \
        PerlIO_printf(Perl_debug_log,                                \
            "Last:'%s' %"IVdf":%"IVdf"/%"IVdf" %sFixed:'%s' @ %"IVdf \
            " %sFloat: '%s' @ %"IVdf"/%"IVdf"",                      \
            SvPVX_const((data)->last_found),                         \
            (IV)((data)->last_end),                                  \
            (IV)((data)->last_start_min),                            \
            (IV)((data)->last_start_max),                            \
            ((data)->longest &&                                      \
             (data)->longest==&((data)->longest_fixed)) ? "*" : "",  \
            SvPVX_const((data)->longest_fixed),                      \
            (IV)((data)->offset_fixed),                              \
            ((data)->longest &&                                      \
             (data)->longest==&((data)->longest_float)) ? "*" : "",  \
            SvPVX_const((data)->longest_float),                      \
            (IV)((data)->offset_float_min),                          \
            (IV)((data)->offset_float_max)                           \
        );                                                           \
    PerlIO_printf(Perl_debug_log,"\n");                              \
});

static void clear_re(pTHX_ void *r);

/* Mark that we cannot extend a found fixed substring at this point.
   Update the longest found anchored substring and the longest found
   floating substrings if needed. */

STATIC void
S_scan_commit(pTHX_ const RExC_state_t *pRExC_state, scan_data_t *data, I32 *minlenp, int is_inf)
{
    const STRLEN l = CHR_SVLEN(data->last_found);
    const STRLEN old_l = CHR_SVLEN(*data->longest);
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_SCAN_COMMIT;

    if ((l >= old_l) && ((l > old_l) || (data->flags & SF_BEFORE_EOL))) {
	SvSetMagicSV(*data->longest, data->last_found);
	if (*data->longest == data->longest_fixed) {
	    data->offset_fixed = l ? data->last_start_min : data->pos_min;
	    if (data->flags & SF_BEFORE_EOL)
		data->flags
		    |= ((data->flags & SF_BEFORE_EOL) << SF_FIX_SHIFT_EOL);
	    else
		data->flags &= ~SF_FIX_BEFORE_EOL;
	    data->minlen_fixed=minlenp;
	    data->lookbehind_fixed=0;
	}
	else { /* *data->longest == data->longest_float */
	    data->offset_float_min = l ? data->last_start_min : data->pos_min;
	    data->offset_float_max = (l
				      ? data->last_start_max
				      : data->pos_min + data->pos_delta);
	    if (is_inf || (U32)data->offset_float_max > (U32)I32_MAX)
		data->offset_float_max = I32_MAX;
	    if (data->flags & SF_BEFORE_EOL)
		data->flags
		    |= ((data->flags & SF_BEFORE_EOL) << SF_FL_SHIFT_EOL);
	    else
		data->flags &= ~SF_FL_BEFORE_EOL;
            data->minlen_float=minlenp;
            data->lookbehind_float=0;
	}
    }
    SvCUR_set(data->last_found, 0);
    {
	SV * const sv = data->last_found;
	if (SvUTF8(sv) && SvMAGICAL(sv)) {
	    MAGIC * const mg = mg_find(sv, PERL_MAGIC_utf8);
	    if (mg)
		mg->mg_len = 0;
	}
    }
    data->last_end = -1;
    data->flags &= ~SF_BEFORE_EOL;
    DEBUG_STUDYDATA("commit: ",data,0);
}

/* Can match anything (initialization) */
STATIC void
S_cl_anything(const RExC_state_t *pRExC_state, struct regnode_charclass_class *cl)
{
    PERL_ARGS_ASSERT_CL_ANYTHING;

    ANYOF_BITMAP_SETALL(cl);
    cl->flags = ANYOF_CLASS|ANYOF_EOS|ANYOF_UNICODE_ALL
		|ANYOF_LOC_NONBITMAP_FOLD|ANYOF_NON_UTF8_LATIN1_ALL;

    /* If any portion of the regex is to operate under locale rules,
     * initialization includes it.  The reason this isn't done for all regexes
     * is that the optimizer was written under the assumption that locale was
     * all-or-nothing.  Given the complexity and lack of documentation in the
     * optimizer, and that there are inadequate test cases for locale, so many
     * parts of it may not work properly, it is safest to avoid locale unless
     * necessary. */
    if (RExC_contains_locale) {
	ANYOF_CLASS_SETALL(cl);	    /* /l uses class */
	cl->flags |= ANYOF_LOCALE;
    }
    else {
	ANYOF_CLASS_ZERO(cl);	    /* Only /l uses class now */
    }
}

/* Can match anything (initialization) */
STATIC int
S_cl_is_anything(const struct regnode_charclass_class *cl)
{
    int value;

    PERL_ARGS_ASSERT_CL_IS_ANYTHING;

    for (value = 0; value <= ANYOF_MAX; value += 2)
	if (ANYOF_CLASS_TEST(cl, value) && ANYOF_CLASS_TEST(cl, value + 1))
	    return 1;
    if (!(cl->flags & ANYOF_UNICODE_ALL))
	return 0;
    if (!ANYOF_BITMAP_TESTALLSET((const void*)cl))
	return 0;
    return 1;
}

/* Can match anything (initialization) */
STATIC void
S_cl_init(const RExC_state_t *pRExC_state, struct regnode_charclass_class *cl)
{
    PERL_ARGS_ASSERT_CL_INIT;

    Zero(cl, 1, struct regnode_charclass_class);
    cl->type = ANYOF;
    cl_anything(pRExC_state, cl);
    ARG_SET(cl, ANYOF_NONBITMAP_EMPTY);
}

/* These two functions currently do the exact same thing */
#define cl_init_zero		S_cl_init

/* 'AND' a given class with another one.  Can create false positives.  'cl'
 * should not be inverted.  'and_with->flags & ANYOF_CLASS' should be 0 if
 * 'and_with' is a regnode_charclass instead of a regnode_charclass_class. */
STATIC void
S_cl_and(struct regnode_charclass_class *cl,
	const struct regnode_charclass_class *and_with)
{
    PERL_ARGS_ASSERT_CL_AND;

    assert(and_with->type == ANYOF);

    /* I (khw) am not sure all these restrictions are necessary XXX */
    if (!(ANYOF_CLASS_TEST_ANY_SET(and_with))
	&& !(ANYOF_CLASS_TEST_ANY_SET(cl))
	&& (and_with->flags & ANYOF_LOCALE) == (cl->flags & ANYOF_LOCALE)
	&& !(and_with->flags & ANYOF_LOC_NONBITMAP_FOLD)
	&& !(cl->flags & ANYOF_LOC_NONBITMAP_FOLD)) {
	int i;

	if (and_with->flags & ANYOF_INVERT)
	    for (i = 0; i < ANYOF_BITMAP_SIZE; i++)
		cl->bitmap[i] &= ~and_with->bitmap[i];
	else
	    for (i = 0; i < ANYOF_BITMAP_SIZE; i++)
		cl->bitmap[i] &= and_with->bitmap[i];
    } /* XXXX: logic is complicated otherwise, leave it along for a moment. */

    if (and_with->flags & ANYOF_INVERT) {

        /* Here, the and'ed node is inverted.  Get the AND of the flags that
         * aren't affected by the inversion.  Those that are affected are
         * handled individually below */
	U8 affected_flags = cl->flags & ~INVERSION_UNAFFECTED_FLAGS;
	cl->flags &= (and_with->flags & INVERSION_UNAFFECTED_FLAGS);
	cl->flags |= affected_flags;

        /* We currently don't know how to deal with things that aren't in the
         * bitmap, but we know that the intersection is no greater than what
         * is already in cl, so let there be false positives that get sorted
         * out after the synthetic start class succeeds, and the node is
         * matched for real. */

        /* The inversion of these two flags indicate that the resulting
         * intersection doesn't have them */
	if (and_with->flags & ANYOF_UNICODE_ALL) {
	    cl->flags &= ~ANYOF_UNICODE_ALL;
	}
	if (and_with->flags & ANYOF_NON_UTF8_LATIN1_ALL) {
	    cl->flags &= ~ANYOF_NON_UTF8_LATIN1_ALL;
	}
    }
    else {   /* and'd node is not inverted */
	U8 outside_bitmap_but_not_utf8; /* Temp variable */

	if (! ANYOF_NONBITMAP(and_with)) {

            /* Here 'and_with' doesn't match anything outside the bitmap
             * (except possibly ANYOF_UNICODE_ALL), which means the
             * intersection can't either, except for ANYOF_UNICODE_ALL, in
             * which case we don't know what the intersection is, but it's no
             * greater than what cl already has, so can just leave it alone,
             * with possible false positives */
            if (! (and_with->flags & ANYOF_UNICODE_ALL)) {
                ARG_SET(cl, ANYOF_NONBITMAP_EMPTY);
		cl->flags &= ~ANYOF_NONBITMAP_NON_UTF8;
            }
	}
	else if (! ANYOF_NONBITMAP(cl)) {

	    /* Here, 'and_with' does match something outside the bitmap, and cl
	     * doesn't have a list of things to match outside the bitmap.  If
             * cl can match all code points above 255, the intersection will
             * be those above-255 code points that 'and_with' matches.  If cl
             * can't match all Unicode code points, it means that it can't
             * match anything outside the bitmap (since the 'if' that got us
             * into this block tested for that), so we leave the bitmap empty.
             */
	    if (cl->flags & ANYOF_UNICODE_ALL) {
		ARG_SET(cl, ARG(and_with));

                /* and_with's ARG may match things that don't require UTF8.
                 * And now cl's will too, in spite of this being an 'and'.  See
                 * the comments below about the kludge */
		cl->flags |= and_with->flags & ANYOF_NONBITMAP_NON_UTF8;
	    }
	}
	else {
            /* Here, both 'and_with' and cl match something outside the
             * bitmap.  Currently we do not do the intersection, so just match
             * whatever cl had at the beginning.  */
	}


        /* Take the intersection of the two sets of flags.  However, the
         * ANYOF_NONBITMAP_NON_UTF8 flag is treated as an 'or'.  This is a
         * kludge around the fact that this flag is not treated like the others
         * which are initialized in cl_anything().  The way the optimizer works
         * is that the synthetic start class (SSC) is initialized to match
         * anything, and then the first time a real node is encountered, its
         * values are AND'd with the SSC's with the result being the values of
         * the real node.  However, there are paths through the optimizer where
         * the AND never gets called, so those initialized bits are set
         * inappropriately, which is not usually a big deal, as they just cause
         * false positives in the SSC, which will just mean a probably
         * imperceptible slow down in execution.  However this bit has a
         * higher false positive consequence in that it can cause utf8.pm,
         * utf8_heavy.pl ... to be loaded when not necessary, which is a much
         * bigger slowdown and also causes significant extra memory to be used.
         * In order to prevent this, the code now takes a different tack.  The
         * bit isn't set unless some part of the regular expression needs it,
         * but once set it won't get cleared.  This means that these extra
         * modules won't get loaded unless there was some path through the
         * pattern that would have required them anyway, and  so any false
         * positives that occur by not ANDing them out when they could be
         * aren't as severe as they would be if we treated this bit like all
         * the others */
        outside_bitmap_but_not_utf8 = (cl->flags | and_with->flags)
                                      & ANYOF_NONBITMAP_NON_UTF8;
	cl->flags &= and_with->flags;
	cl->flags |= outside_bitmap_but_not_utf8;
    }
}

/* 'OR' a given class with another one.  Can create false positives.  'cl'
 * should not be inverted.  'or_with->flags & ANYOF_CLASS' should be 0 if
 * 'or_with' is a regnode_charclass instead of a regnode_charclass_class. */
STATIC void
S_cl_or(const RExC_state_t *pRExC_state, struct regnode_charclass_class *cl, const struct regnode_charclass_class *or_with)
{
    PERL_ARGS_ASSERT_CL_OR;

    if (or_with->flags & ANYOF_INVERT) {

        /* Here, the or'd node is to be inverted.  This means we take the
         * complement of everything not in the bitmap, but currently we don't
         * know what that is, so give up and match anything */
	if (ANYOF_NONBITMAP(or_with)) {
	    cl_anything(pRExC_state, cl);
	}
	/* We do not use
	 * (B1 | CL1) | (!B2 & !CL2) = (B1 | !B2 & !CL2) | (CL1 | (!B2 & !CL2))
	 *   <= (B1 | !B2) | (CL1 | !CL2)
	 * which is wasteful if CL2 is small, but we ignore CL2:
	 *   (B1 | CL1) | (!B2 & !CL2) <= (B1 | CL1) | !B2 = (B1 | !B2) | CL1
	 * XXXX Can we handle case-fold?  Unclear:
	 *   (OK1(i) | OK1(i')) | !(OK1(i) | OK1(i')) =
	 *   (OK1(i) | OK1(i')) | (!OK1(i) & !OK1(i'))
	 */
	else if ( (or_with->flags & ANYOF_LOCALE) == (cl->flags & ANYOF_LOCALE)
	     && !(or_with->flags & ANYOF_LOC_NONBITMAP_FOLD)
	     && !(cl->flags & ANYOF_LOC_NONBITMAP_FOLD) ) {
	    int i;

	    for (i = 0; i < ANYOF_BITMAP_SIZE; i++)
		cl->bitmap[i] |= ~or_with->bitmap[i];
	} /* XXXX: logic is complicated otherwise */
	else {
	    cl_anything(pRExC_state, cl);
	}

        /* And, we can just take the union of the flags that aren't affected
         * by the inversion */
	cl->flags |= or_with->flags & INVERSION_UNAFFECTED_FLAGS;

        /* For the remaining flags:
            ANYOF_UNICODE_ALL and inverted means to not match anything above
                    255, which means that the union with cl should just be
                    what cl has in it, so can ignore this flag
            ANYOF_NON_UTF8_LATIN1_ALL and inverted means if not utf8 and ord
                    is 127-255 to match them, but then invert that, so the
                    union with cl should just be what cl has in it, so can
                    ignore this flag
         */
    } else {    /* 'or_with' is not inverted */
	/* (B1 | CL1) | (B2 | CL2) = (B1 | B2) | (CL1 | CL2)) */
	if ( (or_with->flags & ANYOF_LOCALE) == (cl->flags & ANYOF_LOCALE)
	     && (!(or_with->flags & ANYOF_LOC_NONBITMAP_FOLD)
		 || (cl->flags & ANYOF_LOC_NONBITMAP_FOLD)) ) {
	    int i;

	    /* OR char bitmap and class bitmap separately */
	    for (i = 0; i < ANYOF_BITMAP_SIZE; i++)
		cl->bitmap[i] |= or_with->bitmap[i];
	    if (ANYOF_CLASS_TEST_ANY_SET(or_with)) {
		for (i = 0; i < ANYOF_CLASSBITMAP_SIZE; i++)
		    cl->classflags[i] |= or_with->classflags[i];
		cl->flags |= ANYOF_CLASS;
	    }
	}
	else { /* XXXX: logic is complicated, leave it along for a moment. */
	    cl_anything(pRExC_state, cl);
	}

	if (ANYOF_NONBITMAP(or_with)) {

	    /* Use the added node's outside-the-bit-map match if there isn't a
	     * conflict.  If there is a conflict (both nodes match something
	     * outside the bitmap, but what they match outside is not the same
	     * pointer, and hence not easily compared until XXX we extend
	     * inversion lists this far), give up and allow the start class to
	     * match everything outside the bitmap.  If that stuff is all above
	     * 255, can just set UNICODE_ALL, otherwise caould be anything. */
	    if (! ANYOF_NONBITMAP(cl)) {
		ARG_SET(cl, ARG(or_with));
	    }
	    else if (ARG(cl) != ARG(or_with)) {

		if ((or_with->flags & ANYOF_NONBITMAP_NON_UTF8)) {
		    cl_anything(pRExC_state, cl);
		}
		else {
		    cl->flags |= ANYOF_UNICODE_ALL;
		}
	    }
	}

        /* Take the union */
	cl->flags |= or_with->flags;
    }
}

#define TRIE_LIST_ITEM(state,idx) (trie->states[state].trans.list)[ idx ]
#define TRIE_LIST_CUR(state)  ( TRIE_LIST_ITEM( state, 0 ).forid )
#define TRIE_LIST_LEN(state) ( TRIE_LIST_ITEM( state, 0 ).newstate )
#define TRIE_LIST_USED(idx)  ( trie->states[state].trans.list ? (TRIE_LIST_CUR( idx ) - 1) : 0 )


#ifdef DEBUGGING
/*
   dump_trie(trie,widecharmap,revcharmap)
   dump_trie_interim_list(trie,widecharmap,revcharmap,next_alloc)
   dump_trie_interim_table(trie,widecharmap,revcharmap,next_alloc)

   These routines dump out a trie in a somewhat readable format.
   The _interim_ variants are used for debugging the interim
   tables that are used to generate the final compressed
   representation which is what dump_trie expects.

   Part of the reason for their existence is to provide a form
   of documentation as to how the different representations function.

*/

/*
  Dumps the final compressed table form of the trie to Perl_debug_log.
  Used for debugging make_trie().
*/

STATIC void
S_dump_trie(pTHX_ const struct _reg_trie_data *trie, HV *widecharmap,
	    AV *revcharmap, U32 depth)
{
    U32 state;
    SV *sv=sv_newmortal();
    int colwidth= widecharmap ? 6 : 4;
    U16 word;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_DUMP_TRIE;

    PerlIO_printf( Perl_debug_log, "%*sChar : %-6s%-6s%-4s ",
        (int)depth * 2 + 2,"",
        "Match","Base","Ofs" );

    for( state = 0 ; state < trie->uniquecharcount ; state++ ) {
	SV ** const tmp = av_fetch( revcharmap, state, 0);
        if ( tmp ) {
            PerlIO_printf( Perl_debug_log, "%*s", 
                colwidth,
                pv_pretty(sv, SvPV_nolen_const(*tmp), SvCUR(*tmp), colwidth, 
	                    PL_colors[0], PL_colors[1],
	                    (SvUTF8(*tmp) ? PERL_PV_ESCAPE_UNI : 0) |
	                    PERL_PV_ESCAPE_FIRSTCHAR 
                ) 
            );
        }
    }
    PerlIO_printf( Perl_debug_log, "\n%*sState|-----------------------",
        (int)depth * 2 + 2,"");

    for( state = 0 ; state < trie->uniquecharcount ; state++ )
        PerlIO_printf( Perl_debug_log, "%.*s", colwidth, "--------");
    PerlIO_printf( Perl_debug_log, "\n");

    for( state = 1 ; state < trie->statecount ; state++ ) {
	const U32 base = trie->states[ state ].trans.base;

        PerlIO_printf( Perl_debug_log, "%*s#%4"UVXf"|", (int)depth * 2 + 2,"", (UV)state);

        if ( trie->states[ state ].wordnum ) {
            PerlIO_printf( Perl_debug_log, " W%4X", trie->states[ state ].wordnum );
        } else {
            PerlIO_printf( Perl_debug_log, "%6s", "" );
        }

        PerlIO_printf( Perl_debug_log, " @%4"UVXf" ", (UV)base );

        if ( base ) {
            U32 ofs = 0;

            while( ( base + ofs  < trie->uniquecharcount ) ||
                   ( base + ofs - trie->uniquecharcount < trie->lasttrans
                     && trie->trans[ base + ofs - trie->uniquecharcount ].check != state))
                    ofs++;

            PerlIO_printf( Perl_debug_log, "+%2"UVXf"[ ", (UV)ofs);

            for ( ofs = 0 ; ofs < trie->uniquecharcount ; ofs++ ) {
                if ( ( base + ofs >= trie->uniquecharcount ) &&
                     ( base + ofs - trie->uniquecharcount < trie->lasttrans ) &&
                     trie->trans[ base + ofs - trie->uniquecharcount ].check == state )
                {
                   PerlIO_printf( Perl_debug_log, "%*"UVXf,
                    colwidth,
                    (UV)trie->trans[ base + ofs - trie->uniquecharcount ].next );
                } else {
                    PerlIO_printf( Perl_debug_log, "%*s",colwidth,"   ." );
                }
            }

            PerlIO_printf( Perl_debug_log, "]");

        }
        PerlIO_printf( Perl_debug_log, "\n" );
    }
    PerlIO_printf(Perl_debug_log, "%*sword_info N:(prev,len)=", (int)depth*2, "");
    for (word=1; word <= trie->wordcount; word++) {
	PerlIO_printf(Perl_debug_log, " %d:(%d,%d)",
	    (int)word, (int)(trie->wordinfo[word].prev),
	    (int)(trie->wordinfo[word].len));
    }
    PerlIO_printf(Perl_debug_log, "\n" );
}    
/*
  Dumps a fully constructed but uncompressed trie in list form.
  List tries normally only are used for construction when the number of 
  possible chars (trie->uniquecharcount) is very high.
  Used for debugging make_trie().
*/
STATIC void
S_dump_trie_interim_list(pTHX_ const struct _reg_trie_data *trie,
			 HV *widecharmap, AV *revcharmap, U32 next_alloc,
			 U32 depth)
{
    U32 state;
    SV *sv=sv_newmortal();
    int colwidth= widecharmap ? 6 : 4;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_DUMP_TRIE_INTERIM_LIST;

    /* print out the table precompression.  */
    PerlIO_printf( Perl_debug_log, "%*sState :Word | Transition Data\n%*s%s",
        (int)depth * 2 + 2,"", (int)depth * 2 + 2,"",
        "------:-----+-----------------\n" );
    
    for( state=1 ; state < next_alloc ; state ++ ) {
        U16 charid;
    
        PerlIO_printf( Perl_debug_log, "%*s %4"UVXf" :",
            (int)depth * 2 + 2,"", (UV)state  );
        if ( ! trie->states[ state ].wordnum ) {
            PerlIO_printf( Perl_debug_log, "%5s| ","");
        } else {
            PerlIO_printf( Perl_debug_log, "W%4x| ",
                trie->states[ state ].wordnum
            );
        }
        for( charid = 1 ; charid <= TRIE_LIST_USED( state ) ; charid++ ) {
	    SV ** const tmp = av_fetch( revcharmap, TRIE_LIST_ITEM(state,charid).forid, 0);
	    if ( tmp ) {
                PerlIO_printf( Perl_debug_log, "%*s:%3X=%4"UVXf" | ",
                    colwidth,
                    pv_pretty(sv, SvPV_nolen_const(*tmp), SvCUR(*tmp), colwidth, 
	                    PL_colors[0], PL_colors[1],
	                    (SvUTF8(*tmp) ? PERL_PV_ESCAPE_UNI : 0) |
	                    PERL_PV_ESCAPE_FIRSTCHAR 
                    ) ,
                    TRIE_LIST_ITEM(state,charid).forid,
                    (UV)TRIE_LIST_ITEM(state,charid).newstate
                );
                if (!(charid % 10)) 
                    PerlIO_printf(Perl_debug_log, "\n%*s| ",
                        (int)((depth * 2) + 14), "");
            }
        }
        PerlIO_printf( Perl_debug_log, "\n");
    }
}    

/*
  Dumps a fully constructed but uncompressed trie in table form.
  This is the normal DFA style state transition table, with a few 
  twists to facilitate compression later. 
  Used for debugging make_trie().
*/
STATIC void
S_dump_trie_interim_table(pTHX_ const struct _reg_trie_data *trie,
			  HV *widecharmap, AV *revcharmap, U32 next_alloc,
			  U32 depth)
{
    U32 state;
    U16 charid;
    SV *sv=sv_newmortal();
    int colwidth= widecharmap ? 6 : 4;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_DUMP_TRIE_INTERIM_TABLE;
    
    /*
       print out the table precompression so that we can do a visual check
       that they are identical.
     */
    
    PerlIO_printf( Perl_debug_log, "%*sChar : ",(int)depth * 2 + 2,"" );

    for( charid = 0 ; charid < trie->uniquecharcount ; charid++ ) {
	SV ** const tmp = av_fetch( revcharmap, charid, 0);
        if ( tmp ) {
            PerlIO_printf( Perl_debug_log, "%*s", 
                colwidth,
                pv_pretty(sv, SvPV_nolen_const(*tmp), SvCUR(*tmp), colwidth, 
	                    PL_colors[0], PL_colors[1],
	                    (SvUTF8(*tmp) ? PERL_PV_ESCAPE_UNI : 0) |
	                    PERL_PV_ESCAPE_FIRSTCHAR 
                ) 
            );
        }
    }

    PerlIO_printf( Perl_debug_log, "\n%*sState+-",(int)depth * 2 + 2,"" );

    for( charid=0 ; charid < trie->uniquecharcount ; charid++ ) {
        PerlIO_printf( Perl_debug_log, "%.*s", colwidth,"--------");
    }

    PerlIO_printf( Perl_debug_log, "\n" );

    for( state=1 ; state < next_alloc ; state += trie->uniquecharcount ) {

        PerlIO_printf( Perl_debug_log, "%*s%4"UVXf" : ", 
            (int)depth * 2 + 2,"",
            (UV)TRIE_NODENUM( state ) );

        for( charid = 0 ; charid < trie->uniquecharcount ; charid++ ) {
            UV v=(UV)SAFE_TRIE_NODENUM( trie->trans[ state + charid ].next );
            if (v)
                PerlIO_printf( Perl_debug_log, "%*"UVXf, colwidth, v );
            else
                PerlIO_printf( Perl_debug_log, "%*s", colwidth, "." );
        }
        if ( ! trie->states[ TRIE_NODENUM( state ) ].wordnum ) {
            PerlIO_printf( Perl_debug_log, " (%4"UVXf")\n", (UV)trie->trans[ state ].check );
        } else {
            PerlIO_printf( Perl_debug_log, " (%4"UVXf") W%4X\n", (UV)trie->trans[ state ].check,
            trie->states[ TRIE_NODENUM( state ) ].wordnum );
        }
    }
}

#endif


/* make_trie(startbranch,first,last,tail,word_count,flags,depth)
  startbranch: the first branch in the whole branch sequence
  first      : start branch of sequence of branch-exact nodes.
	       May be the same as startbranch
  last       : Thing following the last branch.
	       May be the same as tail.
  tail       : item following the branch sequence
  count      : words in the sequence
  flags      : currently the OP() type we will be building one of /EXACT(|F|Fl)/
  depth      : indent depth

Inplace optimizes a sequence of 2 or more Branch-Exact nodes into a TRIE node.

A trie is an N'ary tree where the branches are determined by digital
decomposition of the key. IE, at the root node you look up the 1st character and
follow that branch repeat until you find the end of the branches. Nodes can be
marked as "accepting" meaning they represent a complete word. Eg:

  /he|she|his|hers/

would convert into the following structure. Numbers represent states, letters
following numbers represent valid transitions on the letter from that state, if
the number is in square brackets it represents an accepting state, otherwise it
will be in parenthesis.

      +-h->+-e->[3]-+-r->(8)-+-s->[9]
      |    |
      |   (2)
      |    |
     (1)   +-i->(6)-+-s->[7]
      |
      +-s->(3)-+-h->(4)-+-e->[5]

      Accept Word Mapping: 3=>1 (he),5=>2 (she), 7=>3 (his), 9=>4 (hers)

This shows that when matching against the string 'hers' we will begin at state 1
read 'h' and move to state 2, read 'e' and move to state 3 which is accepting,
then read 'r' and go to state 8 followed by 's' which takes us to state 9 which
is also accepting. Thus we know that we can match both 'he' and 'hers' with a
single traverse. We store a mapping from accepting to state to which word was
matched, and then when we have multiple possibilities we try to complete the
rest of the regex in the order in which they occured in the alternation.

The only prior NFA like behaviour that would be changed by the TRIE support is
the silent ignoring of duplicate alternations which are of the form:

 / (DUPE|DUPE) X? (?{ ... }) Y /x

Thus EVAL blocks following a trie may be called a different number of times with
and without the optimisation. With the optimisations dupes will be silently
ignored. This inconsistent behaviour of EVAL type nodes is well established as
the following demonstrates:

 'words'=~/(word|word|word)(?{ print $1 })[xyz]/

which prints out 'word' three times, but

 'words'=~/(word|word|word)(?{ print $1 })S/

which doesnt print it out at all. This is due to other optimisations kicking in.

Example of what happens on a structural level:

The regexp /(ac|ad|ab)+/ will produce the following debug output:

   1: CURLYM[1] {1,32767}(18)
   5:   BRANCH(8)
   6:     EXACT <ac>(16)
   8:   BRANCH(11)
   9:     EXACT <ad>(16)
  11:   BRANCH(14)
  12:     EXACT <ab>(16)
  16:   SUCCEED(0)
  17:   NOTHING(18)
  18: END(0)

This would be optimizable with startbranch=5, first=5, last=16, tail=16
and should turn into:

   1: CURLYM[1] {1,32767}(18)
   5:   TRIE(16)
	[Words:3 Chars Stored:6 Unique Chars:4 States:5 NCP:1]
	  <ac>
	  <ad>
	  <ab>
  16:   SUCCEED(0)
  17:   NOTHING(18)
  18: END(0)

Cases where tail != last would be like /(?foo|bar)baz/:

   1: BRANCH(4)
   2:   EXACT <foo>(8)
   4: BRANCH(7)
   5:   EXACT <bar>(8)
   7: TAIL(8)
   8: EXACT <baz>(10)
  10: END(0)

which would be optimizable with startbranch=1, first=1, last=7, tail=8
and would end up looking like:

    1: TRIE(8)
      [Words:2 Chars Stored:6 Unique Chars:5 States:7 NCP:1]
	<foo>
	<bar>
   7: TAIL(8)
   8: EXACT <baz>(10)
  10: END(0)

    d = uvuni_to_utf8_flags(d, uv, 0);

is the recommended Unicode-aware way of saying

    *(d++) = uv;
*/

#define TRIE_STORE_REVCHAR(val)                                            \
    STMT_START {                                                           \
	if (UTF) {							   \
            SV *zlopp = newSV(7); /* XXX: optimize me */                   \
	    unsigned char *flrbbbbb = (unsigned char *) SvPVX(zlopp);	   \
            unsigned const char *const kapow = uvuni_to_utf8(flrbbbbb, val); \
	    SvCUR_set(zlopp, kapow - flrbbbbb);				   \
	    SvPOK_on(zlopp);						   \
	    SvUTF8_on(zlopp);						   \
	    av_push(revcharmap, zlopp);					   \
	} else {							   \
            char ooooff = (char)val;                                           \
	    av_push(revcharmap, newSVpvn(&ooooff, 1));			   \
	}								   \
        } STMT_END

#define TRIE_READ_CHAR STMT_START {                                                     \
    wordlen++;                                                                          \
    if ( UTF ) {                                                                        \
        /* if it is UTF then it is either already folded, or does not need folding */   \
        uvc = utf8n_to_uvuni( (const U8*) uc, UTF8_MAXLEN, &len, uniflags);             \
    }                                                                                   \
    else if (folder == PL_fold_latin1) {                                                \
        /* if we use this folder we have to obey unicode rules on latin-1 data */       \
        if ( foldlen > 0 ) {                                                            \
           uvc = utf8n_to_uvuni( (const U8*) scan, UTF8_MAXLEN, &len, uniflags );       \
           foldlen -= len;                                                              \
           scan += len;                                                                 \
           len = 0;                                                                     \
        } else {                                                                        \
            len = 1;                                                                    \
            uvc = _to_fold_latin1( (U8) *uc, foldbuf, &foldlen, 1);                     \
            skiplen = UNISKIP(uvc);                                                     \
            foldlen -= skiplen;                                                         \
            scan = foldbuf + skiplen;                                                   \
        }                                                                               \
    } else {                                                                            \
        /* raw data, will be folded later if needed */                                  \
        uvc = (U32)*uc;                                                                 \
        len = 1;                                                                        \
    }                                                                                   \
} STMT_END



#define TRIE_LIST_PUSH(state,fid,ns) STMT_START {               \
    if ( TRIE_LIST_CUR( state ) >=TRIE_LIST_LEN( state ) ) {    \
	U32 ging = TRIE_LIST_LEN( state ) *= 2;                 \
	Renew( trie->states[ state ].trans.list, ging, reg_trie_trans_le ); \
    }                                                           \
    TRIE_LIST_ITEM( state, TRIE_LIST_CUR( state ) ).forid = fid;     \
    TRIE_LIST_ITEM( state, TRIE_LIST_CUR( state ) ).newstate = ns;   \
    TRIE_LIST_CUR( state )++;                                   \
} STMT_END

#define TRIE_LIST_NEW(state) STMT_START {                       \
    Newxz( trie->states[ state ].trans.list,               \
	4, reg_trie_trans_le );                                 \
     TRIE_LIST_CUR( state ) = 1;                                \
     TRIE_LIST_LEN( state ) = 4;                                \
} STMT_END

#define TRIE_HANDLE_WORD(state) STMT_START {                    \
    U16 dupe= trie->states[ state ].wordnum;                    \
    regnode * const noper_next = regnext( noper );              \
                                                                \
    DEBUG_r({                                                   \
        /* store the word for dumping */                        \
        SV* tmp;                                                \
        if (OP(noper) != NOTHING)                               \
            tmp = newSVpvn_utf8(STRING(noper), STR_LEN(noper), UTF);	\
        else                                                    \
            tmp = newSVpvn_utf8( "", 0, UTF );			\
        av_push( trie_words, tmp );                             \
    });                                                         \
                                                                \
    curword++;                                                  \
    trie->wordinfo[curword].prev   = 0;                         \
    trie->wordinfo[curword].len    = wordlen;                   \
    trie->wordinfo[curword].accept = state;                     \
                                                                \
    if ( noper_next < tail ) {                                  \
        if (!trie->jump)                                        \
            trie->jump = (U16 *) PerlMemShared_calloc( word_count + 1, sizeof(U16) ); \
        trie->jump[curword] = (U16)(noper_next - convert);      \
        if (!jumper)                                            \
            jumper = noper_next;                                \
        if (!nextbranch)                                        \
            nextbranch= regnext(cur);                           \
    }                                                           \
                                                                \
    if ( dupe ) {                                               \
        /* It's a dupe. Pre-insert into the wordinfo[].prev   */\
        /* chain, so that when the bits of chain are later    */\
        /* linked together, the dups appear in the chain      */\
	trie->wordinfo[curword].prev = trie->wordinfo[dupe].prev; \
	trie->wordinfo[dupe].prev = curword;                    \
    } else {                                                    \
        /* we haven't inserted this word yet.                */ \
        trie->states[ state ].wordnum = curword;                \
    }                                                           \
} STMT_END


#define TRIE_TRANS_STATE(state,base,ucharcount,charid,special)		\
     ( ( base + charid >=  ucharcount					\
         && base + charid < ubound					\
         && state == trie->trans[ base - ucharcount + charid ].check	\
         && trie->trans[ base - ucharcount + charid ].next )		\
           ? trie->trans[ base - ucharcount + charid ].next		\
           : ( state==1 ? special : 0 )					\
      )

#define MADE_TRIE       1
#define MADE_JUMP_TRIE  2
#define MADE_EXACT_TRIE 4

STATIC I32
S_make_trie(pTHX_ RExC_state_t *pRExC_state, regnode *startbranch, regnode *first, regnode *last, regnode *tail, U32 word_count, U32 flags, U32 depth)
{
    dVAR;
    /* first pass, loop through and scan words */
    reg_trie_data *trie;
    HV *widecharmap = NULL;
    AV *revcharmap = newAV();
    regnode *cur;
    const U32 uniflags = UTF8_ALLOW_DEFAULT;
    STRLEN len = 0;
    UV uvc = 0;
    U16 curword = 0;
    U32 next_alloc = 0;
    regnode *jumper = NULL;
    regnode *nextbranch = NULL;
    regnode *convert = NULL;
    U32 *prev_states; /* temp array mapping each state to previous one */
    /* we just use folder as a flag in utf8 */
    const U8 * folder = NULL;

#ifdef DEBUGGING
    const U32 data_slot = add_data( pRExC_state, 4, "tuuu" );
    AV *trie_words = NULL;
    /* along with revcharmap, this only used during construction but both are
     * useful during debugging so we store them in the struct when debugging.
     */
#else
    const U32 data_slot = add_data( pRExC_state, 2, "tu" );
    STRLEN trie_charcount=0;
#endif
    SV *re_trie_maxbuff;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_MAKE_TRIE;
#ifndef DEBUGGING
    PERL_UNUSED_ARG(depth);
#endif

    switch (flags) {
	case EXACT: break;
	case EXACTFA:
        case EXACTFU_SS:
        case EXACTFU_TRICKYFOLD:
	case EXACTFU: folder = PL_fold_latin1; break;
	case EXACTF:  folder = PL_fold; break;
	case EXACTFL: folder = PL_fold_locale; break;
        default: Perl_croak( aTHX_ "panic! In trie construction, unknown node type %u %s", (unsigned) flags, PL_reg_name[flags] );
    }

    trie = (reg_trie_data *) PerlMemShared_calloc( 1, sizeof(reg_trie_data) );
    trie->refcount = 1;
    trie->startstate = 1;
    trie->wordcount = word_count;
    RExC_rxi->data->data[ data_slot ] = (void*)trie;
    trie->charmap = (U16 *) PerlMemShared_calloc( 256, sizeof(U16) );
    if (flags == EXACT)
	trie->bitmap = (char *) PerlMemShared_calloc( ANYOF_BITMAP_SIZE, 1 );
    trie->wordinfo = (reg_trie_wordinfo *) PerlMemShared_calloc(
                       trie->wordcount+1, sizeof(reg_trie_wordinfo));

    DEBUG_r({
        trie_words = newAV();
    });

    re_trie_maxbuff = get_sv(RE_TRIE_MAXBUF_NAME, 1);
    if (!SvIOK(re_trie_maxbuff)) {
        sv_setiv(re_trie_maxbuff, RE_TRIE_MAXBUF_INIT);
    }
    DEBUG_OPTIMISE_r({
                PerlIO_printf( Perl_debug_log,
                  "%*smake_trie start==%d, first==%d, last==%d, tail==%d depth=%d\n",
                  (int)depth * 2 + 2, "", 
                  REG_NODE_NUM(startbranch),REG_NODE_NUM(first), 
                  REG_NODE_NUM(last), REG_NODE_NUM(tail),
                  (int)depth);
    });
   
   /* Find the node we are going to overwrite */
    if ( first == startbranch && OP( last ) != BRANCH ) {
        /* whole branch chain */
        convert = first;
    } else {
        /* branch sub-chain */
        convert = NEXTOPER( first );
    }
        
    /*  -- First loop and Setup --

       We first traverse the branches and scan each word to determine if it
       contains widechars, and how many unique chars there are, this is
       important as we have to build a table with at least as many columns as we
       have unique chars.

       We use an array of integers to represent the character codes 0..255
       (trie->charmap) and we use a an HV* to store Unicode characters. We use the
       native representation of the character value as the key and IV's for the
       coded index.

       *TODO* If we keep track of how many times each character is used we can
       remap the columns so that the table compression later on is more
       efficient in terms of memory by ensuring the most common value is in the
       middle and the least common are on the outside.  IMO this would be better
       than a most to least common mapping as theres a decent chance the most
       common letter will share a node with the least common, meaning the node
       will not be compressible. With a middle is most common approach the worst
       case is when we have the least common nodes twice.

     */

    for ( cur = first ; cur < last ; cur = regnext( cur ) ) {
        regnode * const noper = NEXTOPER( cur );
        const U8 *uc = (U8*)STRING( noper );
        const U8 * const e  = uc + STR_LEN( noper );
        STRLEN foldlen = 0;
        U8 foldbuf[ UTF8_MAXBYTES_CASE + 1 ];
        STRLEN skiplen = 0;
        const U8 *scan = (U8*)NULL;
        U32 wordlen      = 0;         /* required init */
        STRLEN chars = 0;
        bool set_bit = trie->bitmap ? 1 : 0; /*store the first char in the bitmap?*/

        if (OP(noper) == NOTHING) {
            trie->minlen= 0;
            continue;
        }
        if ( set_bit ) { /* bitmap only alloced when !(UTF&&Folding) */
            TRIE_BITMAP_SET(trie,*uc); /* store the raw first byte
                                          regardless of encoding */
            if (OP( noper ) == EXACTFU_SS) {
                /* false positives are ok, so just set this */
                TRIE_BITMAP_SET(trie,0xDF);
            }
        }
        for ( ; uc < e ; uc += len ) {
            TRIE_CHARCOUNT(trie)++;
            TRIE_READ_CHAR;
            chars++;
            if ( uvc < 256 ) {
                if ( folder ) {
                    U8 folded= folder[ (U8) uvc ];
                    if ( !trie->charmap[ folded ] ) {
                        trie->charmap[ folded ]=( ++trie->uniquecharcount );
                        TRIE_STORE_REVCHAR( folded );
                    }
                }
                if ( !trie->charmap[ uvc ] ) {
                    trie->charmap[ uvc ]=( ++trie->uniquecharcount );
                    TRIE_STORE_REVCHAR( uvc );
                }
                if ( set_bit ) {
		    /* store the codepoint in the bitmap, and its folded
		     * equivalent. */
                    TRIE_BITMAP_SET(trie, uvc);

		    /* store the folded codepoint */
                    if ( folder ) TRIE_BITMAP_SET(trie, folder[(U8) uvc ]);

		    if ( !UTF ) {
			/* store first byte of utf8 representation of
			   variant codepoints */
			if (! UNI_IS_INVARIANT(uvc)) {
			    TRIE_BITMAP_SET(trie, UTF8_TWO_BYTE_HI(uvc));
			}
		    }
                    set_bit = 0; /* We've done our bit :-) */
                }
            } else {
                SV** svpp;
                if ( !widecharmap )
                    widecharmap = newHV();

                svpp = hv_fetch( widecharmap, (char*)&uvc, sizeof( UV ), 1 );

                if ( !svpp )
                    Perl_croak( aTHX_ "error creating/fetching widecharmap entry for 0x%"UVXf, uvc );

                if ( !SvTRUE( *svpp ) ) {
                    sv_setiv( *svpp, ++trie->uniquecharcount );
                    TRIE_STORE_REVCHAR(uvc);
                }
            }
        }
        if( cur == first ) {
            trie->minlen = chars;
            trie->maxlen = chars;
        } else if (chars < trie->minlen) {
            trie->minlen = chars;
        } else if (chars > trie->maxlen) {
            trie->maxlen = chars;
        }
        if (OP( noper ) == EXACTFU_SS) {
            /* XXX: workaround - 'ss' could match "\x{DF}" so minlen could be 1 and not 2*/
	    if (trie->minlen > 1)
                trie->minlen= 1;
        }
	if (OP( noper ) == EXACTFU_TRICKYFOLD) {
	    /* XXX: workround - things like "\x{1FBE}\x{0308}\x{0301}" can match "\x{0390}" 
	     *		      - We assume that any such sequence might match a 2 byte string */
            if (trie->minlen > 2 )
                trie->minlen= 2;
        }

    } /* end first pass */
    DEBUG_TRIE_COMPILE_r(
        PerlIO_printf( Perl_debug_log, "%*sTRIE(%s): W:%d C:%d Uq:%d Min:%d Max:%d\n",
                (int)depth * 2 + 2,"",
                ( widecharmap ? "UTF8" : "NATIVE" ), (int)word_count,
		(int)TRIE_CHARCOUNT(trie), trie->uniquecharcount,
		(int)trie->minlen, (int)trie->maxlen )
    );

    /*
        We now know what we are dealing with in terms of unique chars and
        string sizes so we can calculate how much memory a naive
        representation using a flat table  will take. If it's over a reasonable
        limit (as specified by ${^RE_TRIE_MAXBUF}) we use a more memory
        conservative but potentially much slower representation using an array
        of lists.

        At the end we convert both representations into the same compressed
        form that will be used in regexec.c for matching with. The latter
        is a form that cannot be used to construct with but has memory
        properties similar to the list form and access properties similar
        to the table form making it both suitable for fast searches and
        small enough that its feasable to store for the duration of a program.

        See the comment in the code where the compressed table is produced
        inplace from the flat tabe representation for an explanation of how
        the compression works.

    */


    Newx(prev_states, TRIE_CHARCOUNT(trie) + 2, U32);
    prev_states[1] = 0;

    if ( (IV)( ( TRIE_CHARCOUNT(trie) + 1 ) * trie->uniquecharcount + 1) > SvIV(re_trie_maxbuff) ) {
        /*
            Second Pass -- Array Of Lists Representation

            Each state will be represented by a list of charid:state records
            (reg_trie_trans_le) the first such element holds the CUR and LEN
            points of the allocated array. (See defines above).

            We build the initial structure using the lists, and then convert
            it into the compressed table form which allows faster lookups
            (but cant be modified once converted).
        */

        STRLEN transcount = 1;

        DEBUG_TRIE_COMPILE_MORE_r( PerlIO_printf( Perl_debug_log, 
            "%*sCompiling trie using list compiler\n",
            (int)depth * 2 + 2, ""));

	trie->states = (reg_trie_state *)
	    PerlMemShared_calloc( TRIE_CHARCOUNT(trie) + 2,
				  sizeof(reg_trie_state) );
        TRIE_LIST_NEW(1);
        next_alloc = 2;

        for ( cur = first ; cur < last ; cur = regnext( cur ) ) {

	    regnode * const noper = NEXTOPER( cur );
	    U8 *uc           = (U8*)STRING( noper );
	    const U8 * const e = uc + STR_LEN( noper );
	    U32 state        = 1;         /* required init */
	    U16 charid       = 0;         /* sanity init */
	    U8 *scan         = (U8*)NULL; /* sanity init */
	    STRLEN foldlen   = 0;         /* required init */
            U32 wordlen      = 0;         /* required init */
	    U8 foldbuf[ UTF8_MAXBYTES_CASE + 1 ];
            STRLEN skiplen   = 0;

            if (OP(noper) != NOTHING) {
                for ( ; uc < e ; uc += len ) {

                    TRIE_READ_CHAR;

                    if ( uvc < 256 ) {
                        charid = trie->charmap[ uvc ];
		    } else {
                        SV** const svpp = hv_fetch( widecharmap, (char*)&uvc, sizeof( UV ), 0);
                        if ( !svpp ) {
                            charid = 0;
                        } else {
                            charid=(U16)SvIV( *svpp );
                        }
		    }
                    /* charid is now 0 if we dont know the char read, or nonzero if we do */
                    if ( charid ) {

                        U16 check;
                        U32 newstate = 0;

                        charid--;
                        if ( !trie->states[ state ].trans.list ) {
                            TRIE_LIST_NEW( state );
			}
                        for ( check = 1; check <= TRIE_LIST_USED( state ); check++ ) {
                            if ( TRIE_LIST_ITEM( state, check ).forid == charid ) {
                                newstate = TRIE_LIST_ITEM( state, check ).newstate;
                                break;
                            }
                        }
                        if ( ! newstate ) {
                            newstate = next_alloc++;
			    prev_states[newstate] = state;
                            TRIE_LIST_PUSH( state, charid, newstate );
                            transcount++;
                        }
                        state = newstate;
                    } else {
                        Perl_croak( aTHX_ "panic! In trie construction, no char mapping for %"IVdf, uvc );
		    }
		}
	    }
            TRIE_HANDLE_WORD(state);

        } /* end second pass */

        /* next alloc is the NEXT state to be allocated */
        trie->statecount = next_alloc; 
        trie->states = (reg_trie_state *)
	    PerlMemShared_realloc( trie->states,
				   next_alloc
				   * sizeof(reg_trie_state) );

        /* and now dump it out before we compress it */
        DEBUG_TRIE_COMPILE_MORE_r(dump_trie_interim_list(trie, widecharmap,
							 revcharmap, next_alloc,
							 depth+1)
        );

        trie->trans = (reg_trie_trans *)
	    PerlMemShared_calloc( transcount, sizeof(reg_trie_trans) );
        {
            U32 state;
            U32 tp = 0;
            U32 zp = 0;


            for( state=1 ; state < next_alloc ; state ++ ) {
                U32 base=0;

                /*
                DEBUG_TRIE_COMPILE_MORE_r(
                    PerlIO_printf( Perl_debug_log, "tp: %d zp: %d ",tp,zp)
                );
                */

                if (trie->states[state].trans.list) {
                    U16 minid=TRIE_LIST_ITEM( state, 1).forid;
                    U16 maxid=minid;
		    U16 idx;

                    for( idx = 2 ; idx <= TRIE_LIST_USED( state ) ; idx++ ) {
			const U16 forid = TRIE_LIST_ITEM( state, idx).forid;
			if ( forid < minid ) {
			    minid=forid;
			} else if ( forid > maxid ) {
			    maxid=forid;
			}
                    }
                    if ( transcount < tp + maxid - minid + 1) {
                        transcount *= 2;
			trie->trans = (reg_trie_trans *)
			    PerlMemShared_realloc( trie->trans,
						     transcount
						     * sizeof(reg_trie_trans) );
                        Zero( trie->trans + (transcount / 2), transcount / 2 , reg_trie_trans );
                    }
                    base = trie->uniquecharcount + tp - minid;
                    if ( maxid == minid ) {
                        U32 set = 0;
                        for ( ; zp < tp ; zp++ ) {
                            if ( ! trie->trans[ zp ].next ) {
                                base = trie->uniquecharcount + zp - minid;
                                trie->trans[ zp ].next = TRIE_LIST_ITEM( state, 1).newstate;
                                trie->trans[ zp ].check = state;
                                set = 1;
                                break;
                            }
                        }
                        if ( !set ) {
                            trie->trans[ tp ].next = TRIE_LIST_ITEM( state, 1).newstate;
                            trie->trans[ tp ].check = state;
                            tp++;
                            zp = tp;
                        }
                    } else {
                        for ( idx=1; idx <= TRIE_LIST_USED( state ) ; idx++ ) {
                            const U32 tid = base -  trie->uniquecharcount + TRIE_LIST_ITEM( state, idx ).forid;
                            trie->trans[ tid ].next = TRIE_LIST_ITEM( state, idx ).newstate;
                            trie->trans[ tid ].check = state;
                        }
                        tp += ( maxid - minid + 1 );
                    }
                    Safefree(trie->states[ state ].trans.list);
                }
                /*
                DEBUG_TRIE_COMPILE_MORE_r(
                    PerlIO_printf( Perl_debug_log, " base: %d\n",base);
                );
                */
                trie->states[ state ].trans.base=base;
            }
            trie->lasttrans = tp + 1;
        }
    } else {
        /*
           Second Pass -- Flat Table Representation.

           we dont use the 0 slot of either trans[] or states[] so we add 1 to each.
           We know that we will need Charcount+1 trans at most to store the data
           (one row per char at worst case) So we preallocate both structures
           assuming worst case.

           We then construct the trie using only the .next slots of the entry
           structs.

           We use the .check field of the first entry of the node temporarily to
           make compression both faster and easier by keeping track of how many non
           zero fields are in the node.

           Since trans are numbered from 1 any 0 pointer in the table is a FAIL
           transition.

           There are two terms at use here: state as a TRIE_NODEIDX() which is a
           number representing the first entry of the node, and state as a
           TRIE_NODENUM() which is the trans number. state 1 is TRIE_NODEIDX(1) and
           TRIE_NODENUM(1), state 2 is TRIE_NODEIDX(2) and TRIE_NODENUM(3) if there
           are 2 entrys per node. eg:

             A B       A B
          1. 2 4    1. 3 7
          2. 0 3    3. 0 5
          3. 0 0    5. 0 0
          4. 0 0    7. 0 0

           The table is internally in the right hand, idx form. However as we also
           have to deal with the states array which is indexed by nodenum we have to
           use TRIE_NODENUM() to convert.

        */
        DEBUG_TRIE_COMPILE_MORE_r( PerlIO_printf( Perl_debug_log, 
            "%*sCompiling trie using table compiler\n",
            (int)depth * 2 + 2, ""));

	trie->trans = (reg_trie_trans *)
	    PerlMemShared_calloc( ( TRIE_CHARCOUNT(trie) + 1 )
				  * trie->uniquecharcount + 1,
				  sizeof(reg_trie_trans) );
        trie->states = (reg_trie_state *)
	    PerlMemShared_calloc( TRIE_CHARCOUNT(trie) + 2,
				  sizeof(reg_trie_state) );
        next_alloc = trie->uniquecharcount + 1;


        for ( cur = first ; cur < last ; cur = regnext( cur ) ) {

	    regnode * const noper   = NEXTOPER( cur );
	    const U8 *uc     = (U8*)STRING( noper );
	    const U8 * const e = uc + STR_LEN( noper );

            U32 state        = 1;         /* required init */

            U16 charid       = 0;         /* sanity init */
            U32 accept_state = 0;         /* sanity init */
            U8 *scan         = (U8*)NULL; /* sanity init */

            STRLEN foldlen   = 0;         /* required init */
            U32 wordlen      = 0;         /* required init */
            STRLEN skiplen   = 0;
            U8 foldbuf[ UTF8_MAXBYTES_CASE + 1 ];


            if ( OP(noper) != NOTHING ) {
                for ( ; uc < e ; uc += len ) {

                    TRIE_READ_CHAR;

                    if ( uvc < 256 ) {
                        charid = trie->charmap[ uvc ];
                    } else {
                        SV* const * const svpp = hv_fetch( widecharmap, (char*)&uvc, sizeof( UV ), 0);
                        charid = svpp ? (U16)SvIV(*svpp) : 0;
                    }
                    if ( charid ) {
                        charid--;
                        if ( !trie->trans[ state + charid ].next ) {
                            trie->trans[ state + charid ].next = next_alloc;
                            trie->trans[ state ].check++;
			    prev_states[TRIE_NODENUM(next_alloc)]
				    = TRIE_NODENUM(state);
                            next_alloc += trie->uniquecharcount;
                        }
                        state = trie->trans[ state + charid ].next;
                    } else {
                        Perl_croak( aTHX_ "panic! In trie construction, no char mapping for %"IVdf, uvc );
                    }
                    /* charid is now 0 if we dont know the char read, or nonzero if we do */
                }
            }
            accept_state = TRIE_NODENUM( state );
            TRIE_HANDLE_WORD(accept_state);

        } /* end second pass */

        /* and now dump it out before we compress it */
        DEBUG_TRIE_COMPILE_MORE_r(dump_trie_interim_table(trie, widecharmap,
							  revcharmap,
							  next_alloc, depth+1));

        {
        /*
           * Inplace compress the table.*

           For sparse data sets the table constructed by the trie algorithm will
           be mostly 0/FAIL transitions or to put it another way mostly empty.
           (Note that leaf nodes will not contain any transitions.)

           This algorithm compresses the tables by eliminating most such
           transitions, at the cost of a modest bit of extra work during lookup:

           - Each states[] entry contains a .base field which indicates the
           index in the state[] array wheres its transition data is stored.

           - If .base is 0 there are no valid transitions from that node.

           - If .base is nonzero then charid is added to it to find an entry in
           the trans array.

           -If trans[states[state].base+charid].check!=state then the
           transition is taken to be a 0/Fail transition. Thus if there are fail
           transitions at the front of the node then the .base offset will point
           somewhere inside the previous nodes data (or maybe even into a node
           even earlier), but the .check field determines if the transition is
           valid.

           XXX - wrong maybe?
           The following process inplace converts the table to the compressed
           table: We first do not compress the root node 1,and mark all its
           .check pointers as 1 and set its .base pointer as 1 as well. This
           allows us to do a DFA construction from the compressed table later,
           and ensures that any .base pointers we calculate later are greater
           than 0.

           - We set 'pos' to indicate the first entry of the second node.

           - We then iterate over the columns of the node, finding the first and
           last used entry at l and m. We then copy l..m into pos..(pos+m-l),
           and set the .check pointers accordingly, and advance pos
           appropriately and repreat for the next node. Note that when we copy
           the next pointers we have to convert them from the original
           NODEIDX form to NODENUM form as the former is not valid post
           compression.

           - If a node has no transitions used we mark its base as 0 and do not
           advance the pos pointer.

           - If a node only has one transition we use a second pointer into the
           structure to fill in allocated fail transitions from other states.
           This pointer is independent of the main pointer and scans forward
           looking for null transitions that are allocated to a state. When it
           finds one it writes the single transition into the "hole".  If the
           pointer doesnt find one the single transition is appended as normal.

           - Once compressed we can Renew/realloc the structures to release the
           excess space.

           See "Table-Compression Methods" in sec 3.9 of the Red Dragon,
           specifically Fig 3.47 and the associated pseudocode.

           demq
        */
        const U32 laststate = TRIE_NODENUM( next_alloc );
	U32 state, charid;
        U32 pos = 0, zp=0;
        trie->statecount = laststate;

        for ( state = 1 ; state < laststate ; state++ ) {
            U8 flag = 0;
	    const U32 stateidx = TRIE_NODEIDX( state );
	    const U32 o_used = trie->trans[ stateidx ].check;
	    U32 used = trie->trans[ stateidx ].check;
            trie->trans[ stateidx ].check = 0;

            for ( charid = 0 ; used && charid < trie->uniquecharcount ; charid++ ) {
                if ( flag || trie->trans[ stateidx + charid ].next ) {
                    if ( trie->trans[ stateidx + charid ].next ) {
                        if (o_used == 1) {
                            for ( ; zp < pos ; zp++ ) {
                                if ( ! trie->trans[ zp ].next ) {
                                    break;
                                }
                            }
                            trie->states[ state ].trans.base = zp + trie->uniquecharcount - charid ;
                            trie->trans[ zp ].next = SAFE_TRIE_NODENUM( trie->trans[ stateidx + charid ].next );
                            trie->trans[ zp ].check = state;
                            if ( ++zp > pos ) pos = zp;
                            break;
                        }
                        used--;
                    }
                    if ( !flag ) {
                        flag = 1;
                        trie->states[ state ].trans.base = pos + trie->uniquecharcount - charid ;
                    }
                    trie->trans[ pos ].next = SAFE_TRIE_NODENUM( trie->trans[ stateidx + charid ].next );
                    trie->trans[ pos ].check = state;
                    pos++;
                }
            }
        }
        trie->lasttrans = pos + 1;
        trie->states = (reg_trie_state *)
	    PerlMemShared_realloc( trie->states, laststate
				   * sizeof(reg_trie_state) );
        DEBUG_TRIE_COMPILE_MORE_r(
                PerlIO_printf( Perl_debug_log,
		    "%*sAlloc: %d Orig: %"IVdf" elements, Final:%"IVdf". Savings of %%%5.2f\n",
		    (int)depth * 2 + 2,"",
                    (int)( ( TRIE_CHARCOUNT(trie) + 1 ) * trie->uniquecharcount + 1 ),
		    (IV)next_alloc,
		    (IV)pos,
                    ( ( next_alloc - pos ) * 100 ) / (double)next_alloc );
            );

        } /* end table compress */
    }
    DEBUG_TRIE_COMPILE_MORE_r(
            PerlIO_printf(Perl_debug_log, "%*sStatecount:%"UVxf" Lasttrans:%"UVxf"\n",
                (int)depth * 2 + 2, "",
                (UV)trie->statecount,
                (UV)trie->lasttrans)
    );
    /* resize the trans array to remove unused space */
    trie->trans = (reg_trie_trans *)
	PerlMemShared_realloc( trie->trans, trie->lasttrans
			       * sizeof(reg_trie_trans) );

    {   /* Modify the program and insert the new TRIE node */ 
        U8 nodetype =(U8)(flags & 0xFF);
        char *str=NULL;
        
#ifdef DEBUGGING
        regnode *optimize = NULL;
#ifdef RE_TRACK_PATTERN_OFFSETS

        U32 mjd_offset = 0;
        U32 mjd_nodelen = 0;
#endif /* RE_TRACK_PATTERN_OFFSETS */
#endif /* DEBUGGING */
        /*
           This means we convert either the first branch or the first Exact,
           depending on whether the thing following (in 'last') is a branch
           or not and whther first is the startbranch (ie is it a sub part of
           the alternation or is it the whole thing.)
           Assuming its a sub part we convert the EXACT otherwise we convert
           the whole branch sequence, including the first.
         */
        /* Find the node we are going to overwrite */
        if ( first != startbranch || OP( last ) == BRANCH ) {
            /* branch sub-chain */
            NEXT_OFF( first ) = (U16)(last - first);
#ifdef RE_TRACK_PATTERN_OFFSETS
            DEBUG_r({
                mjd_offset= Node_Offset((convert));
                mjd_nodelen= Node_Length((convert));
            });
#endif
            /* whole branch chain */
        }
#ifdef RE_TRACK_PATTERN_OFFSETS
        else {
            DEBUG_r({
                const  regnode *nop = NEXTOPER( convert );
                mjd_offset= Node_Offset((nop));
                mjd_nodelen= Node_Length((nop));
            });
        }
        DEBUG_OPTIMISE_r(
            PerlIO_printf(Perl_debug_log, "%*sMJD offset:%"UVuf" MJD length:%"UVuf"\n",
                (int)depth * 2 + 2, "",
                (UV)mjd_offset, (UV)mjd_nodelen)
        );
#endif
        /* But first we check to see if there is a common prefix we can 
           split out as an EXACT and put in front of the TRIE node.  */
        trie->startstate= 1;
        if ( trie->bitmap && !widecharmap && !trie->jump  ) {
            U32 state;
            for ( state = 1 ; state < trie->statecount-1 ; state++ ) {
                U32 ofs = 0;
                I32 idx = -1;
                U32 count = 0;
                const U32 base = trie->states[ state ].trans.base;

                if ( trie->states[state].wordnum )
                        count = 1;

                for ( ofs = 0 ; ofs < trie->uniquecharcount ; ofs++ ) {
                    if ( ( base + ofs >= trie->uniquecharcount ) &&
                         ( base + ofs - trie->uniquecharcount < trie->lasttrans ) &&
                         trie->trans[ base + ofs - trie->uniquecharcount ].check == state )
                    {
                        if ( ++count > 1 ) {
                            SV **tmp = av_fetch( revcharmap, ofs, 0);
			    const U8 *ch = (U8*)SvPV_nolen_const( *tmp );
                            if ( state == 1 ) break;
                            if ( count == 2 ) {
                                Zero(trie->bitmap, ANYOF_BITMAP_SIZE, char);
                                DEBUG_OPTIMISE_r(
                                    PerlIO_printf(Perl_debug_log,
					"%*sNew Start State=%"UVuf" Class: [",
                                        (int)depth * 2 + 2, "",
                                        (UV)state));
				if (idx >= 0) {
				    SV ** const tmp = av_fetch( revcharmap, idx, 0);
				    const U8 * const ch = (U8*)SvPV_nolen_const( *tmp );

                                    TRIE_BITMAP_SET(trie,*ch);
                                    if ( folder )
                                        TRIE_BITMAP_SET(trie, folder[ *ch ]);
                                    DEBUG_OPTIMISE_r(
                                        PerlIO_printf(Perl_debug_log, "%s", (char*)ch)
                                    );
				}
			    }
			    TRIE_BITMAP_SET(trie,*ch);
			    if ( folder )
				TRIE_BITMAP_SET(trie,folder[ *ch ]);
			    DEBUG_OPTIMISE_r(PerlIO_printf( Perl_debug_log,"%s", ch));
			}
                        idx = ofs;
		    }
                }
                if ( count == 1 ) {
                    SV **tmp = av_fetch( revcharmap, idx, 0);
                    STRLEN len;
                    char *ch = SvPV( *tmp, len );
                    DEBUG_OPTIMISE_r({
                        SV *sv=sv_newmortal();
                        PerlIO_printf( Perl_debug_log,
			    "%*sPrefix State: %"UVuf" Idx:%"UVuf" Char='%s'\n",
                            (int)depth * 2 + 2, "",
                            (UV)state, (UV)idx, 
                            pv_pretty(sv, SvPV_nolen_const(*tmp), SvCUR(*tmp), 6, 
	                        PL_colors[0], PL_colors[1],
	                        (SvUTF8(*tmp) ? PERL_PV_ESCAPE_UNI : 0) |
	                        PERL_PV_ESCAPE_FIRSTCHAR 
                            )
                        );
                    });
                    if ( state==1 ) {
                        OP( convert ) = nodetype;
                        str=STRING(convert);
                        STR_LEN(convert)=0;
                    }
                    STR_LEN(convert) += len;
                    while (len--)
                        *str++ = *ch++;
		} else {
#ifdef DEBUGGING	    
		    if (state>1)
			DEBUG_OPTIMISE_r(PerlIO_printf( Perl_debug_log,"]\n"));
#endif
		    break;
		}
	    }
	    trie->prefixlen = (state-1);
            if (str) {
                regnode *n = convert+NODE_SZ_STR(convert);
                NEXT_OFF(convert) = NODE_SZ_STR(convert);
                trie->startstate = state;
                trie->minlen -= (state - 1);
                trie->maxlen -= (state - 1);
#ifdef DEBUGGING
               /* At least the UNICOS C compiler choked on this
                * being argument to DEBUG_r(), so let's just have
                * it right here. */
               if (
#ifdef PERL_EXT_RE_BUILD
                   1
#else
                   DEBUG_r_TEST
#endif
                   ) {
                   regnode *fix = convert;
                   U32 word = trie->wordcount;
                   mjd_nodelen++;
                   Set_Node_Offset_Length(convert, mjd_offset, state - 1);
                   while( ++fix < n ) {
                       Set_Node_Offset_Length(fix, 0, 0);
                   }
                   while (word--) {
                       SV ** const tmp = av_fetch( trie_words, word, 0 );
                       if (tmp) {
                           if ( STR_LEN(convert) <= SvCUR(*tmp) )
                               sv_chop(*tmp, SvPV_nolen(*tmp) + STR_LEN(convert));
                           else
                               sv_chop(*tmp, SvPV_nolen(*tmp) + SvCUR(*tmp));
                       }
                   }
               }
#endif
                if (trie->maxlen) {
                    convert = n;
		} else {
                    NEXT_OFF(convert) = (U16)(tail - convert);
                    DEBUG_r(optimize= n);
                }
            }
        }
        if (!jumper) 
            jumper = last; 
        if ( trie->maxlen ) {
	    NEXT_OFF( convert ) = (U16)(tail - convert);
	    ARG_SET( convert, data_slot );
	    /* Store the offset to the first unabsorbed branch in 
	       jump[0], which is otherwise unused by the jump logic. 
	       We use this when dumping a trie and during optimisation. */
	    if (trie->jump) 
	        trie->jump[0] = (U16)(nextbranch - convert);
            
            /* If the start state is not accepting (meaning there is no empty string/NOTHING)
	     *   and there is a bitmap
	     *   and the first "jump target" node we found leaves enough room
	     * then convert the TRIE node into a TRIEC node, with the bitmap
	     * embedded inline in the opcode - this is hypothetically faster.
	     */
            if ( !trie->states[trie->startstate].wordnum
		 && trie->bitmap
		 && ( (char *)jumper - (char *)convert) >= (int)sizeof(struct regnode_charclass) )
            {
                OP( convert ) = TRIEC;
                Copy(trie->bitmap, ((struct regnode_charclass *)convert)->bitmap, ANYOF_BITMAP_SIZE, char);
                PerlMemShared_free(trie->bitmap);
                trie->bitmap= NULL;
            } else 
                OP( convert ) = TRIE;

            /* store the type in the flags */
            convert->flags = nodetype;
            DEBUG_r({
            optimize = convert 
                      + NODE_STEP_REGNODE 
                      + regarglen[ OP( convert ) ];
            });
            /* XXX We really should free up the resource in trie now, 
                   as we won't use them - (which resources?) dmq */
        }
        /* needed for dumping*/
        DEBUG_r(if (optimize) {
            regnode *opt = convert;

            while ( ++opt < optimize) {
                Set_Node_Offset_Length(opt,0,0);
            }
            /* 
                Try to clean up some of the debris left after the 
                optimisation.
             */
            while( optimize < jumper ) {
                mjd_nodelen += Node_Length((optimize));
                OP( optimize ) = OPTIMIZED;
                Set_Node_Offset_Length(optimize,0,0);
                optimize++;
            }
            Set_Node_Offset_Length(convert,mjd_offset,mjd_nodelen);
        });
    } /* end node insert */

    /*  Finish populating the prev field of the wordinfo array.  Walk back
     *  from each accept state until we find another accept state, and if
     *  so, point the first word's .prev field at the second word. If the
     *  second already has a .prev field set, stop now. This will be the
     *  case either if we've already processed that word's accept state,
     *  or that state had multiple words, and the overspill words were
     *  already linked up earlier.
     */
    {
	U16 word;
	U32 state;
	U16 prev;

	for (word=1; word <= trie->wordcount; word++) {
	    prev = 0;
	    if (trie->wordinfo[word].prev)
		continue;
	    state = trie->wordinfo[word].accept;
	    while (state) {
		state = prev_states[state];
		if (!state)
		    break;
		prev = trie->states[state].wordnum;
		if (prev)
		    break;
	    }
	    trie->wordinfo[word].prev = prev;
	}
	Safefree(prev_states);
    }


    /* and now dump out the compressed format */
    DEBUG_TRIE_COMPILE_r(dump_trie(trie, widecharmap, revcharmap, depth+1));

    RExC_rxi->data->data[ data_slot + 1 ] = (void*)widecharmap;
#ifdef DEBUGGING
    RExC_rxi->data->data[ data_slot + TRIE_WORDS_OFFSET ] = (void*)trie_words;
    RExC_rxi->data->data[ data_slot + 3 ] = (void*)revcharmap;
#else
    SvREFCNT_dec(revcharmap);
#endif
    return trie->jump 
           ? MADE_JUMP_TRIE 
           : trie->startstate>1 
             ? MADE_EXACT_TRIE 
             : MADE_TRIE;
}

STATIC void
S_make_trie_failtable(pTHX_ RExC_state_t *pRExC_state, regnode *source,  regnode *stclass, U32 depth)
{
/* The Trie is constructed and compressed now so we can build a fail array if it's needed

   This is basically the Aho-Corasick algorithm. Its from exercise 3.31 and 3.32 in the
   "Red Dragon" -- Compilers, principles, techniques, and tools. Aho, Sethi, Ullman 1985/88
   ISBN 0-201-10088-6

   We find the fail state for each state in the trie, this state is the longest proper
   suffix of the current state's 'word' that is also a proper prefix of another word in our
   trie. State 1 represents the word '' and is thus the default fail state. This allows
   the DFA not to have to restart after its tried and failed a word at a given point, it
   simply continues as though it had been matching the other word in the first place.
   Consider
      'abcdgu'=~/abcdefg|cdgu/
   When we get to 'd' we are still matching the first word, we would encounter 'g' which would
   fail, which would bring us to the state representing 'd' in the second word where we would
   try 'g' and succeed, proceeding to match 'cdgu'.
 */
 /* add a fail transition */
    const U32 trie_offset = ARG(source);
    reg_trie_data *trie=(reg_trie_data *)RExC_rxi->data->data[trie_offset];
    U32 *q;
    const U32 ucharcount = trie->uniquecharcount;
    const U32 numstates = trie->statecount;
    const U32 ubound = trie->lasttrans + ucharcount;
    U32 q_read = 0;
    U32 q_write = 0;
    U32 charid;
    U32 base = trie->states[ 1 ].trans.base;
    U32 *fail;
    reg_ac_data *aho;
    const U32 data_slot = add_data( pRExC_state, 1, "T" );
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_MAKE_TRIE_FAILTABLE;
#ifndef DEBUGGING
    PERL_UNUSED_ARG(depth);
#endif


    ARG_SET( stclass, data_slot );
    aho = (reg_ac_data *) PerlMemShared_calloc( 1, sizeof(reg_ac_data) );
    RExC_rxi->data->data[ data_slot ] = (void*)aho;
    aho->trie=trie_offset;
    aho->states=(reg_trie_state *)PerlMemShared_malloc( numstates * sizeof(reg_trie_state) );
    Copy( trie->states, aho->states, numstates, reg_trie_state );
    Newxz( q, numstates, U32);
    aho->fail = (U32 *) PerlMemShared_calloc( numstates, sizeof(U32) );
    aho->refcount = 1;
    fail = aho->fail;
    /* initialize fail[0..1] to be 1 so that we always have
       a valid final fail state */
    fail[ 0 ] = fail[ 1 ] = 1;

    for ( charid = 0; charid < ucharcount ; charid++ ) {
	const U32 newstate = TRIE_TRANS_STATE( 1, base, ucharcount, charid, 0 );
	if ( newstate ) {
            q[ q_write ] = newstate;
            /* set to point at the root */
            fail[ q[ q_write++ ] ]=1;
        }
    }
    while ( q_read < q_write) {
	const U32 cur = q[ q_read++ % numstates ];
        base = trie->states[ cur ].trans.base;

        for ( charid = 0 ; charid < ucharcount ; charid++ ) {
	    const U32 ch_state = TRIE_TRANS_STATE( cur, base, ucharcount, charid, 1 );
	    if (ch_state) {
                U32 fail_state = cur;
                U32 fail_base;
                do {
                    fail_state = fail[ fail_state ];
                    fail_base = aho->states[ fail_state ].trans.base;
                } while ( !TRIE_TRANS_STATE( fail_state, fail_base, ucharcount, charid, 1 ) );

                fail_state = TRIE_TRANS_STATE( fail_state, fail_base, ucharcount, charid, 1 );
                fail[ ch_state ] = fail_state;
                if ( !aho->states[ ch_state ].wordnum && aho->states[ fail_state ].wordnum )
                {
                        aho->states[ ch_state ].wordnum =  aho->states[ fail_state ].wordnum;
                }
                q[ q_write++ % numstates] = ch_state;
            }
        }
    }
    /* restore fail[0..1] to 0 so that we "fall out" of the AC loop
       when we fail in state 1, this allows us to use the
       charclass scan to find a valid start char. This is based on the principle
       that theres a good chance the string being searched contains lots of stuff
       that cant be a start char.
     */
    fail[ 0 ] = fail[ 1 ] = 0;
    DEBUG_TRIE_COMPILE_r({
        PerlIO_printf(Perl_debug_log,
		      "%*sStclass Failtable (%"UVuf" states): 0", 
		      (int)(depth * 2), "", (UV)numstates
        );
        for( q_read=1; q_read<numstates; q_read++ ) {
            PerlIO_printf(Perl_debug_log, ", %"UVuf, (UV)fail[q_read]);
        }
        PerlIO_printf(Perl_debug_log, "\n");
    });
    Safefree(q);
    /*RExC_seen |= REG_SEEN_TRIEDFA;*/
}


/*
 * There are strange code-generation bugs caused on sparc64 by gcc-2.95.2.
 * These need to be revisited when a newer toolchain becomes available.
 */
#if defined(__sparc64__) && defined(__GNUC__)
#   if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 96)
#       undef  SPARC64_GCC_WORKAROUND
#       define SPARC64_GCC_WORKAROUND 1
#   endif
#endif

#define DEBUG_PEEP(str,scan,depth) \
    DEBUG_OPTIMISE_r({if (scan){ \
       SV * const mysv=sv_newmortal(); \
       regnode *Next = regnext(scan); \
       regprop(RExC_rx, mysv, scan); \
       PerlIO_printf(Perl_debug_log, "%*s" str ">%3d: %s (%d)\n", \
       (int)depth*2, "", REG_NODE_NUM(scan), SvPV_nolen_const(mysv),\
       Next ? (REG_NODE_NUM(Next)) : 0 ); \
   }});


/* The below joins as many adjacent EXACTish nodes as possible into a single
 * one, and looks for problematic sequences of characters whose folds vs.
 * non-folds have sufficiently different lengths, that the optimizer would be
 * fooled into rejecting legitimate matches of them, and the trie construction
 * code can't cope with them.  The joining is only done if:
 * 1) there is room in the current conglomerated node to entirely contain the
 *    next one.
 * 2) they are the exact same node type
 *
 * The adjacent nodes actually may be separated by NOTHING kind nodes, and
 * these get optimized out
 *
 * If there are problematic code sequences, *min_subtract is set to the delta
 * that the minimum size of the node can be less than its actual size.  And,
 * the node type of the result is changed to reflect that it contains these
 * sequences.
 *
 * And *has_exactf_sharp_s is set to indicate whether or not the node is EXACTF
 * and contains LATIN SMALL LETTER SHARP S
 *
 * This is as good a place as any to discuss the design of handling these
 * problematic sequences.  It's been wrong in Perl for a very long time.  There
 * are three code points in Unicode whose folded lengths differ so much from
 * the un-folded lengths that it causes problems for the optimizer and trie
 * construction.  Why only these are problematic, and not others where lengths
 * also differ is something I (khw) do not understand.  New versions of Unicode
 * might add more such code points.  Hopefully the logic in fold_grind.t that
 * figures out what to test (in part by verifying that each size-combination
 * gets tested) will catch any that do come along, so they can be added to the
 * special handling below.  The chances of new ones are actually rather small,
 * as most, if not all, of the world's scripts that have casefolding have
 * already been encoded by Unicode.  Also, a number of Unicode's decisions were
 * made to allow compatibility with pre-existing standards, and almost all of
 * those have already been dealt with.  These would otherwise be the most
 * likely candidates for generating further tricky sequences.  In other words,
 * Unicode by itself is unlikely to add new ones unless it is for compatibility
 * with pre-existing standards, and there aren't many of those left.
 *
 * The previous designs for dealing with these involved assigning a special
 * node for them.  This approach doesn't work, as evidenced by this example:
 *      "\xDFs" =~ /s\xDF/ui    # Used to fail before these patches
 * Both these fold to "sss", but if the pattern is parsed to create a node of
 * that would match just the \xDF, it won't be able to handle the case where a
 * successful match would have to cross the node's boundary.  The new approach
 * that hopefully generally solves the problem generates an EXACTFU_SS node
 * that is "sss".
 *
 * There are a number of components to the approach (a lot of work for just
 * three code points!):
 * 1)   This routine examines each EXACTFish node that could contain the
 *      problematic sequences.  It returns in *min_subtract how much to
 *      subtract from the the actual length of the string to get a real minimum
 *      for one that could match it.  This number is usually 0 except for the
 *      problematic sequences.  This delta is used by the caller to adjust the
 *      min length of the match, and the delta between min and max, so that the
 *      optimizer doesn't reject these possibilities based on size constraints.
 * 2)   These sequences are not currently correctly handled by the trie code
 *      either, so it changes the joined node type to ops that are not handled
 *      by trie's, those new ops being EXACTFU_SS and EXACTFU_TRICKYFOLD.
 * 3)   This is sufficient for the two Greek sequences (described below), but
 *      the one involving the Sharp s (\xDF) needs more.  The node type
 *      EXACTFU_SS is used for an EXACTFU node that contains at least one "ss"
 *      sequence in it.  For non-UTF-8 patterns and strings, this is the only
 *      case where there is a possible fold length change.  That means that a
 *      regular EXACTFU node without UTF-8 involvement doesn't have to concern
 *      itself with length changes, and so can be processed faster.  regexec.c
 *      takes advantage of this.  Generally, an EXACTFish node that is in UTF-8
 *      is pre-folded by regcomp.c.  This saves effort in regex matching.
 *      However, probably mostly for historical reasons, the pre-folding isn't
 *      done for non-UTF8 patterns (and it can't be for EXACTF and EXACTFL
 *      nodes, as what they fold to isn't known until runtime.)  The fold
 *      possibilities for the non-UTF8 patterns are quite simple, except for
 *      the sharp s.  All the ones that don't involve a UTF-8 target string
 *      are members of a fold-pair, and arrays are set up for all of them
 *      that quickly find the other member of the pair.  It might actually
 *      be faster to pre-fold these, but it isn't currently done, except for
 *      the sharp s.  Code elsewhere in this file makes sure that it gets
 *      folded to 'ss', even if the pattern isn't UTF-8.  This avoids the
 *      issues described in the next item.
 * 4)   A problem remains for the sharp s in EXACTF nodes.  Whether it matches
 *      'ss' or not is not knowable at compile time.  It will match iff the
 *      target string is in UTF-8, unlike the EXACTFU nodes, where it always
 *      matches; and the EXACTFL and EXACTFA nodes where it never does.  Thus
 *      it can't be folded to "ss" at compile time, unlike EXACTFU does as
 *      described in item 3).  An assumption that the optimizer part of
 *      regexec.c (probably unwittingly) makes is that a character in the
 *      pattern corresponds to at most a single character in the target string.
 *      (And I do mean character, and not byte here, unlike other parts of the
 *      documentation that have never been updated to account for multibyte
 *      Unicode.)  This assumption is wrong only in this case, as all other
 *      cases are either 1-1 folds when no UTF-8 is involved; or is true by
 *      virtue of having this file pre-fold UTF-8 patterns.   I'm
 *      reluctant to try to change this assumption, so instead the code punts.
 *      This routine examines EXACTF nodes for the sharp s, and returns a
 *      boolean indicating whether or not the node is an EXACTF node that
 *      contains a sharp s.  When it is true, the caller sets a flag that later
 *      causes the optimizer in this file to not set values for the floating
 *      and fixed string lengths, and thus avoids the optimizer code in
 *      regexec.c that makes the invalid assumption.  Thus, there is no
 *      optimization based on string lengths for EXACTF nodes that contain the
 *      sharp s.  This only happens for /id rules (which means the pattern
 *      isn't in UTF-8).
 */

#define JOIN_EXACT(scan,min_subtract,has_exactf_sharp_s, flags) \
    if (PL_regkind[OP(scan)] == EXACT) \
        join_exact(pRExC_state,(scan),(min_subtract),has_exactf_sharp_s, (flags),NULL,depth+1)

STATIC U32
S_join_exact(pTHX_ RExC_state_t *pRExC_state, regnode *scan, UV *min_subtract, bool *has_exactf_sharp_s, U32 flags,regnode *val, U32 depth) {
    /* Merge several consecutive EXACTish nodes into one. */
    regnode *n = regnext(scan);
    U32 stringok = 1;
    regnode *next = scan + NODE_SZ_STR(scan);
    U32 merged = 0;
    U32 stopnow = 0;
#ifdef DEBUGGING
    regnode *stop = scan;
    GET_RE_DEBUG_FLAGS_DECL;
#else
    PERL_UNUSED_ARG(depth);
#endif

    PERL_ARGS_ASSERT_JOIN_EXACT;
#ifndef EXPERIMENTAL_INPLACESCAN
    PERL_UNUSED_ARG(flags);
    PERL_UNUSED_ARG(val);
#endif
    DEBUG_PEEP("join",scan,depth);

    /* Look through the subsequent nodes in the chain.  Skip NOTHING, merge
     * EXACT ones that are mergeable to the current one. */
    while (n
           && (PL_regkind[OP(n)] == NOTHING
               || (stringok && OP(n) == OP(scan)))
           && NEXT_OFF(n)
           && NEXT_OFF(scan) + NEXT_OFF(n) < I16_MAX)
    {
        
        if (OP(n) == TAIL || n > next)
            stringok = 0;
        if (PL_regkind[OP(n)] == NOTHING) {
            DEBUG_PEEP("skip:",n,depth);
            NEXT_OFF(scan) += NEXT_OFF(n);
            next = n + NODE_STEP_REGNODE;
#ifdef DEBUGGING
            if (stringok)
                stop = n;
#endif
            n = regnext(n);
        }
        else if (stringok) {
            const unsigned int oldl = STR_LEN(scan);
            regnode * const nnext = regnext(n);

            if (oldl + STR_LEN(n) > U8_MAX)
                break;
            
            DEBUG_PEEP("merg",n,depth);
            merged++;

            NEXT_OFF(scan) += NEXT_OFF(n);
            STR_LEN(scan) += STR_LEN(n);
            next = n + NODE_SZ_STR(n);
            /* Now we can overwrite *n : */
            Move(STRING(n), STRING(scan) + oldl, STR_LEN(n), char);
#ifdef DEBUGGING
            stop = next - 1;
#endif
            n = nnext;
            if (stopnow) break;
        }

#ifdef EXPERIMENTAL_INPLACESCAN
	if (flags && !NEXT_OFF(n)) {
	    DEBUG_PEEP("atch", val, depth);
	    if (reg_off_by_arg[OP(n)]) {
		ARG_SET(n, val - n);
	    }
	    else {
		NEXT_OFF(n) = val - n;
	    }
	    stopnow = 1;
	}
#endif
    }

    *min_subtract = 0;
    *has_exactf_sharp_s = FALSE;

    /* Here, all the adjacent mergeable EXACTish nodes have been merged.  We
     * can now analyze for sequences of problematic code points.  (Prior to
     * this final joining, sequences could have been split over boundaries, and
     * hence missed).  The sequences only happen in folding, hence for any
     * non-EXACT EXACTish node */
    if (OP(scan) != EXACT) {
        U8 *s;
        U8 * s0 = (U8*) STRING(scan);
        U8 * const s_end = s0 + STR_LEN(scan);

	/* The below is perhaps overboard, but this allows us to save a test
	 * each time through the loop at the expense of a mask.  This is
	 * because on both EBCDIC and ASCII machines, 'S' and 's' differ by a
	 * single bit.  On ASCII they are 32 apart; on EBCDIC, they are 64.
	 * This uses an exclusive 'or' to find that bit and then inverts it to
	 * form a mask, with just a single 0, in the bit position where 'S' and
	 * 's' differ. */
	const U8 S_or_s_mask = (U8) ~ ('S' ^ 's');
	const U8 s_masked = 's' & S_or_s_mask;

	/* One pass is made over the node's string looking for all the
	 * possibilities.  to avoid some tests in the loop, there are two main
	 * cases, for UTF-8 patterns (which can't have EXACTF nodes) and
	 * non-UTF-8 */
	if (UTF) {

	    /* There are two problematic Greek code points in Unicode
	     * casefolding
	     *
	     * U+0390 - GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS
	     * U+03B0 - GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS
	     *
	     * which casefold to
	     *
	     * Unicode                      UTF-8
	     *
	     * U+03B9 U+0308 U+0301         0xCE 0xB9 0xCC 0x88 0xCC 0x81
	     * U+03C5 U+0308 U+0301         0xCF 0x85 0xCC 0x88 0xCC 0x81
             *
	     * This means that in case-insensitive matching (or "loose
	     * matching", as Unicode calls it), an EXACTF of length six (the
	     * UTF-8 encoded byte length of the above casefolded versions) can
	     * match a target string of length two (the byte length of UTF-8
	     * encoded U+0390 or U+03B0).  This would rather mess up the
	     * minimum length computation.  (there are other code points that
	     * also fold to these two sequences, but the delta is smaller)
	     *
	     * If these sequences are found, the minimum length is decreased by
	     * four (six minus two).
	     *
	     * Similarly, 'ss' may match the single char and byte LATIN SMALL
	     * LETTER SHARP S.  We decrease the min length by 1 for each
	     * occurrence of 'ss' found */

#ifdef EBCDIC /* RD tunifold greek 0390 and 03B0 */
#	    define U390_first_byte 0xb4
	    const U8 U390_tail[] = "\x68\xaf\x49\xaf\x42";
#	    define U3B0_first_byte 0xb5
	    const U8 U3B0_tail[] = "\x46\xaf\x49\xaf\x42";
#else
#	    define U390_first_byte 0xce
	    const U8 U390_tail[] = "\xb9\xcc\x88\xcc\x81";
#	    define U3B0_first_byte 0xcf
	    const U8 U3B0_tail[] = "\x85\xcc\x88\xcc\x81";
#endif
	    const U8 len = sizeof(U390_tail); /* (-1 for NUL; +1 for 1st byte;
						 yields a net of 0 */
	    /* Examine the string for one of the problematic sequences */
	    for (s = s0;
		 s < s_end - 1; /* Can stop 1 before the end, as minimum length
				 * sequence we are looking for is 2 */
		 s += UTF8SKIP(s))
	    {

		/* Look for the first byte in each problematic sequence */
		switch (*s) {
		    /* We don't have to worry about other things that fold to
		     * 's' (such as the long s, U+017F), as all above-latin1
		     * code points have been pre-folded */
		    case 's':
		    case 'S':

                        /* Current character is an 's' or 'S'.  If next one is
                         * as well, we have the dreaded sequence */
			if (((*(s+1) & S_or_s_mask) == s_masked)
			    /* These two node types don't have special handling
			     * for 'ss' */
			    && OP(scan) != EXACTFL && OP(scan) != EXACTFA)
			{
			    *min_subtract += 1;
			    OP(scan) = EXACTFU_SS;
			    s++;    /* No need to look at this character again */
			}
			break;

		    case U390_first_byte:
			if (s_end - s >= len

			    /* The 1's are because are skipping comparing the
			     * first byte */
			    && memEQ(s + 1, U390_tail, len - 1))
			{
			    goto greek_sequence;
			}
			break;

		    case U3B0_first_byte:
			if (! (s_end - s >= len
			       && memEQ(s + 1, U3B0_tail, len - 1)))
			{
			    break;
			}
		      greek_sequence:
			*min_subtract += 4;

			/* This can't currently be handled by trie's, so change
			 * the node type to indicate this.  If EXACTFA and
			 * EXACTFL were ever to be handled by trie's, this
			 * would have to be changed.  If this node has already
			 * been changed to EXACTFU_SS in this loop, leave it as
			 * is.  (I (khw) think it doesn't matter in regexec.c
			 * for UTF patterns, but no need to change it */
			if (OP(scan) == EXACTFU) {
			    OP(scan) = EXACTFU_TRICKYFOLD;
			}
			s += 6;	/* We already know what this sequence is.  Skip
				   the rest of it */
			break;
		}
	    }
	}
	else if (OP(scan) != EXACTFL && OP(scan) != EXACTFA) {

	    /* Here, the pattern is not UTF-8.  We need to look only for the
	     * 'ss' sequence, and in the EXACTF case, the sharp s, which can be
	     * in the final position.  Otherwise we can stop looking 1 byte
	     * earlier because have to find both the first and second 's' */
	    const U8* upper = (OP(scan) == EXACTF) ? s_end : s_end -1;

	    for (s = s0; s < upper; s++) {
		switch (*s) {
		    case 'S':
		    case 's':
			if (s_end - s > 1
			    && ((*(s+1) & S_or_s_mask) == s_masked))
			{
			    *min_subtract += 1;

			    /* EXACTF nodes need to know that the minimum
			     * length changed so that a sharp s in the string
			     * can match this ss in the pattern, but they
			     * remain EXACTF nodes, as they are not trie'able,
			     * so don't have to invent a new node type to
			     * exclude them from the trie code */
			    if (OP(scan) != EXACTF) {
				OP(scan) = EXACTFU_SS;
			    }
			    s++;
			}
			break;
		    case LATIN_SMALL_LETTER_SHARP_S:
			if (OP(scan) == EXACTF) {
			    *has_exactf_sharp_s = TRUE;
			}
			break;
		}
	    }
	}
    }

#ifdef DEBUGGING
    /* Allow dumping but overwriting the collection of skipped
     * ops and/or strings with fake optimized ops */
    n = scan + NODE_SZ_STR(scan);
    while (n <= stop) {
	OP(n) = OPTIMIZED;
	FLAGS(n) = 0;
	NEXT_OFF(n) = 0;
        n++;
    }
#endif
    DEBUG_OPTIMISE_r(if (merged){DEBUG_PEEP("finl",scan,depth)});
    return stopnow;
}

/* REx optimizer.  Converts nodes into quicker variants "in place".
   Finds fixed substrings.  */

/* Stops at toplevel WHILEM as well as at "last". At end *scanp is set
   to the position after last scanned or to NULL. */

#define INIT_AND_WITHP \
    assert(!and_withp); \
    Newx(and_withp,1,struct regnode_charclass_class); \
    SAVEFREEPV(and_withp)

/* this is a chain of data about sub patterns we are processing that
   need to be handled separately/specially in study_chunk. Its so
   we can simulate recursion without losing state.  */
struct scan_frame;
typedef struct scan_frame {
    regnode *last;  /* last node to process in this frame */
    regnode *next;  /* next node to process when last is reached */
    struct scan_frame *prev; /*previous frame*/
    I32 stop; /* what stopparen do we use */
} scan_frame;


#define SCAN_COMMIT(s, data, m) scan_commit(s, data, m, is_inf)

#define CASE_SYNST_FNC(nAmE)                                       \
case nAmE:                                                         \
    if (flags & SCF_DO_STCLASS_AND) {                              \
	    for (value = 0; value < 256; value++)                  \
		if (!is_ ## nAmE ## _cp(value))                       \
		    ANYOF_BITMAP_CLEAR(data->start_class, value);  \
    }                                                              \
    else {                                                         \
	    for (value = 0; value < 256; value++)                  \
		if (is_ ## nAmE ## _cp(value))                        \
		    ANYOF_BITMAP_SET(data->start_class, value);	   \
    }                                                              \
    break;                                                         \
case N ## nAmE:                                                    \
    if (flags & SCF_DO_STCLASS_AND) {                              \
	    for (value = 0; value < 256; value++)                   \
		if (is_ ## nAmE ## _cp(value))                         \
		    ANYOF_BITMAP_CLEAR(data->start_class, value);   \
    }                                                               \
    else {                                                          \
	    for (value = 0; value < 256; value++)                   \
		if (!is_ ## nAmE ## _cp(value))                        \
		    ANYOF_BITMAP_SET(data->start_class, value);	    \
    }                                                               \
    break



STATIC I32
S_study_chunk(pTHX_ RExC_state_t *pRExC_state, regnode **scanp,
                        I32 *minlenp, I32 *deltap,
			regnode *last,
			scan_data_t *data,
			I32 stopparen,
			U8* recursed,
			struct regnode_charclass_class *and_withp,
			U32 flags, U32 depth)
			/* scanp: Start here (read-write). */
			/* deltap: Write maxlen-minlen here. */
			/* last: Stop before this one. */
			/* data: string data about the pattern */
			/* stopparen: treat close N as END */
			/* recursed: which subroutines have we recursed into */
			/* and_withp: Valid if flags & SCF_DO_STCLASS_OR */
{
    dVAR;
    I32 min = 0, pars = 0, code;
    regnode *scan = *scanp, *next;
    I32 delta = 0;
    int is_inf = (flags & SCF_DO_SUBSTR) && (data->flags & SF_IS_INF);
    int is_inf_internal = 0;		/* The studied chunk is infinite */
    I32 is_par = OP(scan) == OPEN ? ARG(scan) : 0;
    scan_data_t data_fake;
    SV *re_trie_maxbuff = NULL;
    regnode *first_non_open = scan;
    I32 stopmin = I32_MAX;
    scan_frame *frame = NULL;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_STUDY_CHUNK;

#ifdef DEBUGGING
    StructCopy(&zero_scan_data, &data_fake, scan_data_t);
#endif

    if ( depth == 0 ) {
        while (first_non_open && OP(first_non_open) == OPEN)
            first_non_open=regnext(first_non_open);
    }


  fake_study_recurse:
    while ( scan && OP(scan) != END && scan < last ){
        UV min_subtract = 0;    /* How much to subtract from the minimum node
                                   length to get a real minimum (because the
                                   folded version may be shorter) */
	bool has_exactf_sharp_s = FALSE;
	/* Peephole optimizer: */
	DEBUG_STUDYDATA("Peep:", data,depth);
	DEBUG_PEEP("Peep",scan,depth);

        /* Its not clear to khw or hv why this is done here, and not in the
         * clauses that deal with EXACT nodes.  khw's guess is that it's
         * because of a previous design */
        JOIN_EXACT(scan,&min_subtract, &has_exactf_sharp_s, 0);

	/* Follow the next-chain of the current node and optimize
	   away all the NOTHINGs from it.  */
	if (OP(scan) != CURLYX) {
	    const int max = (reg_off_by_arg[OP(scan)]
		       ? I32_MAX
		       /* I32 may be smaller than U16 on CRAYs! */
		       : (I32_MAX < U16_MAX ? I32_MAX : U16_MAX));
	    int off = (reg_off_by_arg[OP(scan)] ? ARG(scan) : NEXT_OFF(scan));
	    int noff;
	    regnode *n = scan;

	    /* Skip NOTHING and LONGJMP. */
	    while ((n = regnext(n))
		   && ((PL_regkind[OP(n)] == NOTHING && (noff = NEXT_OFF(n)))
		       || ((OP(n) == LONGJMP) && (noff = ARG(n))))
		   && off + noff < max)
		off += noff;
	    if (reg_off_by_arg[OP(scan)])
		ARG(scan) = off;
	    else
		NEXT_OFF(scan) = off;
	}



	/* The principal pseudo-switch.  Cannot be a switch, since we
	   look into several different things.  */
	if (OP(scan) == BRANCH || OP(scan) == BRANCHJ
		   || OP(scan) == IFTHEN) {
	    next = regnext(scan);
	    code = OP(scan);
	    /* demq: the op(next)==code check is to see if we have "branch-branch" AFAICT */

	    if (OP(next) == code || code == IFTHEN) {
	        /* NOTE - There is similar code to this block below for handling
	           TRIE nodes on a re-study.  If you change stuff here check there
	           too. */
		I32 max1 = 0, min1 = I32_MAX, num = 0;
		struct regnode_charclass_class accum;
		regnode * const startbranch=scan;

		if (flags & SCF_DO_SUBSTR)
		    SCAN_COMMIT(pRExC_state, data, minlenp); /* Cannot merge strings after this. */
		if (flags & SCF_DO_STCLASS)
		    cl_init_zero(pRExC_state, &accum);

		while (OP(scan) == code) {
		    I32 deltanext, minnext, f = 0, fake;
		    struct regnode_charclass_class this_class;

		    num++;
		    data_fake.flags = 0;
		    if (data) {
			data_fake.whilem_c = data->whilem_c;
			data_fake.last_closep = data->last_closep;
		    }
		    else
			data_fake.last_closep = &fake;

		    data_fake.pos_delta = delta;
		    next = regnext(scan);
		    scan = NEXTOPER(scan);
		    if (code != BRANCH)
			scan = NEXTOPER(scan);
		    if (flags & SCF_DO_STCLASS) {
			cl_init(pRExC_state, &this_class);
			data_fake.start_class = &this_class;
			f = SCF_DO_STCLASS_AND;
		    }
		    if (flags & SCF_WHILEM_VISITED_POS)
			f |= SCF_WHILEM_VISITED_POS;

		    /* we suppose the run is continuous, last=next...*/
		    minnext = study_chunk(pRExC_state, &scan, minlenp, &deltanext,
					  next, &data_fake,
					  stopparen, recursed, NULL, f,depth+1);
		    if (min1 > minnext)
			min1 = minnext;
		    if (max1 < minnext + deltanext)
			max1 = minnext + deltanext;
		    if (deltanext == I32_MAX)
			is_inf = is_inf_internal = 1;
		    scan = next;
		    if (data_fake.flags & (SF_HAS_PAR|SF_IN_PAR))
			pars++;
	            if (data_fake.flags & SCF_SEEN_ACCEPT) {
	                if ( stopmin > minnext) 
	                    stopmin = min + min1;
	                flags &= ~SCF_DO_SUBSTR;
	                if (data)
	                    data->flags |= SCF_SEEN_ACCEPT;
	            }
		    if (data) {
			if (data_fake.flags & SF_HAS_EVAL)
			    data->flags |= SF_HAS_EVAL;
			data->whilem_c = data_fake.whilem_c;
		    }
		    if (flags & SCF_DO_STCLASS)
			cl_or(pRExC_state, &accum, &this_class);
		}
		if (code == IFTHEN && num < 2) /* Empty ELSE branch */
		    min1 = 0;
		if (flags & SCF_DO_SUBSTR) {
		    data->pos_min += min1;
		    data->pos_delta += max1 - min1;
		    if (max1 != min1 || is_inf)
			data->longest = &(data->longest_float);
		}
		min += min1;
		delta += max1 - min1;
		if (flags & SCF_DO_STCLASS_OR) {
		    cl_or(pRExC_state, data->start_class, &accum);
		    if (min1) {
			cl_and(data->start_class, and_withp);
			flags &= ~SCF_DO_STCLASS;
		    }
		}
		else if (flags & SCF_DO_STCLASS_AND) {
		    if (min1) {
			cl_and(data->start_class, &accum);
			flags &= ~SCF_DO_STCLASS;
		    }
		    else {
			/* Switch to OR mode: cache the old value of
			 * data->start_class */
			INIT_AND_WITHP;
			StructCopy(data->start_class, and_withp,
				   struct regnode_charclass_class);
			flags &= ~SCF_DO_STCLASS_AND;
			StructCopy(&accum, data->start_class,
				   struct regnode_charclass_class);
			flags |= SCF_DO_STCLASS_OR;
			data->start_class->flags |= ANYOF_EOS;
		    }
		}

                if (PERL_ENABLE_TRIE_OPTIMISATION && OP( startbranch ) == BRANCH ) {
		/* demq.

		   Assuming this was/is a branch we are dealing with: 'scan' now
		   points at the item that follows the branch sequence, whatever
		   it is. We now start at the beginning of the sequence and look
		   for subsequences of

		   BRANCH->EXACT=>x1
		   BRANCH->EXACT=>x2
		   tail

		   which would be constructed from a pattern like /A|LIST|OF|WORDS/

		   If we can find such a subsequence we need to turn the first
		   element into a trie and then add the subsequent branch exact
		   strings to the trie.

		   We have two cases

		     1. patterns where the whole set of branches can be converted. 

		     2. patterns where only a subset can be converted.

		   In case 1 we can replace the whole set with a single regop
		   for the trie. In case 2 we need to keep the start and end
		   branches so

		     'BRANCH EXACT; BRANCH EXACT; BRANCH X'
		     becomes BRANCH TRIE; BRANCH X;

		  There is an additional case, that being where there is a 
		  common prefix, which gets split out into an EXACT like node
		  preceding the TRIE node.

		  If x(1..n)==tail then we can do a simple trie, if not we make
		  a "jump" trie, such that when we match the appropriate word
		  we "jump" to the appropriate tail node. Essentially we turn
		  a nested if into a case structure of sorts.

		*/

		    int made=0;
		    if (!re_trie_maxbuff) {
			re_trie_maxbuff = get_sv(RE_TRIE_MAXBUF_NAME, 1);
			if (!SvIOK(re_trie_maxbuff))
			    sv_setiv(re_trie_maxbuff, RE_TRIE_MAXBUF_INIT);
		    }
                    if ( SvIV(re_trie_maxbuff)>=0  ) {
                        regnode *cur;
                        regnode *first = (regnode *)NULL;
                        regnode *last = (regnode *)NULL;
                        regnode *tail = scan;
                        U8 trietype = 0;
                        U32 count=0;

#ifdef DEBUGGING
                        SV * const mysv = sv_newmortal();       /* for dumping */
#endif
                        /* var tail is used because there may be a TAIL
                           regop in the way. Ie, the exacts will point to the
                           thing following the TAIL, but the last branch will
                           point at the TAIL. So we advance tail. If we
                           have nested (?:) we may have to move through several
                           tails.
                         */

                        while ( OP( tail ) == TAIL ) {
                            /* this is the TAIL generated by (?:) */
                            tail = regnext( tail );
                        }

                        
                        DEBUG_OPTIMISE_r({
                            regprop(RExC_rx, mysv, tail );
                            PerlIO_printf( Perl_debug_log, "%*s%s%s\n",
                                (int)depth * 2 + 2, "", 
                                "Looking for TRIE'able sequences. Tail node is: ", 
                                SvPV_nolen_const( mysv )
                            );
                        });
                        
                        /*

                            Step through the branches
                                cur represents each branch,
                                noper is the first thing to be matched as part of that branch
                                noper_next is the regnext() of that node.

                            We normally handle a case like this /FOO[xyz]|BAR[pqr]/
                            via a "jump trie" but we also support building with NOJUMPTRIE,
                            which restricts the trie logic to structures like /FOO|BAR/.

                            If noper is a trieable nodetype then the branch is a possible optimization
                            target. If we are building under NOJUMPTRIE then we require that noper_next
                            is the same as scan (our current position in the regex program).

                            Once we have two or more consecutive such branches we can create a
                            trie of the EXACT's contents and stitch it in place into the program.

                            If the sequence represents all of the branches in the alternation we
                            replace the entire thing with a single TRIE node.

                            Otherwise when it is a subsequence we need to stitch it in place and
                            replace only the relevant branches. This means the first branch has
                            to remain as it is used by the alternation logic, and its next pointer,
                            and needs to be repointed at the item on the branch chain following
                            the last branch we have optimized away.

                            This could be either a BRANCH, in which case the subsequence is internal,
                            or it could be the item following the branch sequence in which case the
                            subsequence is at the end (which does not necessarily mean the first node
                            is the start of the alternation).

                            TRIE_TYPE(X) is a define which maps the optype to a trietype.

                                optype          |  trietype
                                ----------------+-----------
                                NOTHING         | NOTHING
                                EXACT           | EXACT
                                EXACTFU         | EXACTFU
                                EXACTFU_SS      | EXACTFU
                                EXACTFU_TRICKYFOLD | EXACTFU
                                EXACTFA         | 0


                        */
#define TRIE_TYPE(X) ( ( NOTHING == (X) ) ? NOTHING :   \
                       ( EXACT == (X) )   ? EXACT :        \
                       ( EXACTFU == (X) || EXACTFU_SS == (X) || EXACTFU_TRICKYFOLD == (X) ) ? EXACTFU :        \
                       0 )

                        /* dont use tail as the end marker for this traverse */
                        for ( cur = startbranch ; cur != scan ; cur = regnext( cur ) ) {
                            regnode * const noper = NEXTOPER( cur );
                            U8 noper_type = OP( noper );
                            U8 noper_trietype = TRIE_TYPE( noper_type );
#if defined(DEBUGGING) || defined(NOJUMPTRIE)
                            regnode * const noper_next = regnext( noper );
#endif

                            DEBUG_OPTIMISE_r({
                                regprop(RExC_rx, mysv, cur);
                                PerlIO_printf( Perl_debug_log, "%*s- %s (%d)",
                                   (int)depth * 2 + 2,"", SvPV_nolen_const( mysv ), REG_NODE_NUM(cur) );

                                regprop(RExC_rx, mysv, noper);
                                PerlIO_printf( Perl_debug_log, " -> %s",
                                    SvPV_nolen_const(mysv));

                                if ( noper_next ) {
                                  regprop(RExC_rx, mysv, noper_next );
                                  PerlIO_printf( Perl_debug_log,"\t=> %s\t",
                                    SvPV_nolen_const(mysv));
                                }
                                PerlIO_printf( Perl_debug_log, "(First==%d,Last==%d,Cur==%d)\n",
                                   REG_NODE_NUM(first), REG_NODE_NUM(last), REG_NODE_NUM(cur) );
                            });

                            /* Is noper a trieable nodetype that can be merged with the
                             * current trie (if there is one)? */
                            if ( noper_trietype
                                  &&
                                  (
                                        /* XXX: Currently we cannot allow a NOTHING node to be the first element
                                         * of a TRIEABLE sequence, Otherwise we will overwrite the regop following
                                         * the NOTHING with the TRIE regop later on. This is because a NOTHING node
                                         * is only one regnode wide, and a TRIE is two regnodes. An example of a
                                         * problematic pattern is: "x" =~ /\A(?>(?:(?:)A|B|C?x))\z/
                                         * At a later point of time we can somewhat workaround this by handling
                                         * NOTHING -> EXACT sequences as generated by /(?:)A|(?:)B/ type patterns,
                                         * as we can effectively ignore the NOTHING regop in that case.
                                         * This clause, which allows NOTHING to start a sequence is left commented
                                         * out as a reference.
                                         * - Yves

                                           ( noper_trietype == NOTHING)
                                           || ( trietype == NOTHING )
                                        */
                                        ( noper_trietype == NOTHING && trietype )
                                        || ( trietype == noper_trietype )
                                  )
#ifdef NOJUMPTRIE
                                  && noper_next == tail
#endif
                                  && count < U16_MAX)
                            {
                                /* Handle mergable triable node
                                 * Either we are the first node in a new trieable sequence,
                                 * in which case we do some bookkeeping, otherwise we update
                                 * the end pointer. */
                                count++;
                                if ( !first ) {
                                    first = cur;
                                    trietype = noper_trietype;
                                } else {
                                    if ( trietype == NOTHING )
                                        trietype = noper_trietype;
                                    last = cur;
                                }
                            } /* end handle mergable triable node */
                            else {
                                /* handle unmergable node -
                                 * noper may either be a triable node which can not be tried
                                 * together with the current trie, or a non triable node */
                                if ( last ) {
                                    /* If last is set and trietype is not NOTHING then we have found
                                     * at least two triable branch sequences in a row of a similar
                                     * trietype so we can turn them into a trie. If/when we
                                     * allow NOTHING to start a trie sequence this condition will be
                                     * required, and it isn't expensive so we leave it in for now. */
                                    if ( trietype != NOTHING )
                                        make_trie( pRExC_state,
                                                startbranch, first, cur, tail, count,
                                                trietype, depth+1 );
                                    last = NULL; /* note: we clear/update first, trietype etc below, so we dont do it here */
                                }
                                if ( noper_trietype
#ifdef NOJUMPTRIE
                                     && noper_next == tail
#endif
                                ){
                                    /* noper is triable, so we can start a new trie sequence */
                                    count = 1;
                                    first = cur;
                                    trietype = noper_trietype;
                                } else if (first) {
                                    /* if we already saw a first but the current node is not triable then we have
                                     * to reset the first information. */
                                    count = 0;
                                    first = NULL;
                                    trietype = 0;
                                }
                            } /* end handle unmergable node */
                        } /* loop over branches */
                        DEBUG_OPTIMISE_r({
                            regprop(RExC_rx, mysv, cur);
                            PerlIO_printf( Perl_debug_log,
                              "%*s- %s (%d) <SCAN FINISHED>\n", (int)depth * 2 + 2,
                              "", SvPV_nolen_const( mysv ),REG_NODE_NUM(cur));

                        });
                        if ( last && trietype != NOTHING ) {
                            /* the last branch of the sequence was part of a trie,
                             * so we have to construct it here outside of the loop
                             */
                            made= make_trie( pRExC_state, startbranch, first, scan, tail, count, trietype, depth+1 );
#ifdef TRIE_STUDY_OPT
                            if ( ((made == MADE_EXACT_TRIE && 
                                 startbranch == first) 
                                 || ( first_non_open == first )) && 
                                 depth==0 ) {
                                flags |= SCF_TRIE_RESTUDY;
                                if ( startbranch == first 
                                     && scan == tail ) 
                                {
                                    RExC_seen &=~REG_TOP_LEVEL_BRANCHES;
                                }
                            }
#endif
                        } /* end if ( last) */
                    } /* TRIE_MAXBUF is non zero */
                    
                } /* do trie */
                
	    }
	    else if ( code == BRANCHJ ) {  /* single branch is optimized. */
		scan = NEXTOPER(NEXTOPER(scan));
	    } else			/* single branch is optimized. */
		scan = NEXTOPER(scan);
	    continue;
	} else if (OP(scan) == SUSPEND || OP(scan) == GOSUB || OP(scan) == GOSTART) {
	    scan_frame *newframe = NULL;
	    I32 paren;
	    regnode *start;
	    regnode *end;

	    if (OP(scan) != SUSPEND) {
	    /* set the pointer */
	        if (OP(scan) == GOSUB) {
	            paren = ARG(scan);
	            RExC_recurse[ARG2L(scan)] = scan;
                    start = RExC_open_parens[paren-1];
                    end   = RExC_close_parens[paren-1];
                } else {
                    paren = 0;
                    start = RExC_rxi->program + 1;
                    end   = RExC_opend;
                }
                if (!recursed) {
                    Newxz(recursed, (((RExC_npar)>>3) +1), U8);
                    SAVEFREEPV(recursed);
                }
                if (!PAREN_TEST(recursed,paren+1)) {
		    PAREN_SET(recursed,paren+1);
                    Newx(newframe,1,scan_frame);
                } else {
                    if (flags & SCF_DO_SUBSTR) {
                        SCAN_COMMIT(pRExC_state,data,minlenp);
                        data->longest = &(data->longest_float);
                    }
                    is_inf = is_inf_internal = 1;
                    if (flags & SCF_DO_STCLASS_OR) /* Allow everything */
                        cl_anything(pRExC_state, data->start_class);
                    flags &= ~SCF_DO_STCLASS;
	        }
            } else {
	        Newx(newframe,1,scan_frame);
	        paren = stopparen;
	        start = scan+2;
	        end = regnext(scan);
	    }
	    if (newframe) {
                assert(start);
                assert(end);
	        SAVEFREEPV(newframe);
	        newframe->next = regnext(scan);
	        newframe->last = last;
	        newframe->stop = stopparen;
	        newframe->prev = frame;

	        frame = newframe;
	        scan =  start;
	        stopparen = paren;
	        last = end;

	        continue;
	    }
	}
	else if (OP(scan) == EXACT) {
	    I32 l = STR_LEN(scan);
	    UV uc;
	    if (UTF) {
		const U8 * const s = (U8*)STRING(scan);
		uc = utf8_to_uvchr_buf(s, s + l, NULL);
		l = utf8_length(s, s + l);
	    } else {
		uc = *((U8*)STRING(scan));
	    }
	    min += l;
	    if (flags & SCF_DO_SUBSTR) { /* Update longest substr. */
		/* The code below prefers earlier match for fixed
		   offset, later match for variable offset.  */
		if (data->last_end == -1) { /* Update the start info. */
		    data->last_start_min = data->pos_min;
 		    data->last_start_max = is_inf
 			? I32_MAX : data->pos_min + data->pos_delta;
		}
		sv_catpvn(data->last_found, STRING(scan), STR_LEN(scan));
		if (UTF)
		    SvUTF8_on(data->last_found);
		{
		    SV * const sv = data->last_found;
		    MAGIC * const mg = SvUTF8(sv) && SvMAGICAL(sv) ?
			mg_find(sv, PERL_MAGIC_utf8) : NULL;
		    if (mg && mg->mg_len >= 0)
			mg->mg_len += utf8_length((U8*)STRING(scan),
						  (U8*)STRING(scan)+STR_LEN(scan));
		}
		data->last_end = data->pos_min + l;
		data->pos_min += l; /* As in the first entry. */
		data->flags &= ~SF_BEFORE_EOL;
	    }
	    if (flags & SCF_DO_STCLASS_AND) {
		/* Check whether it is compatible with what we know already! */
		int compat = 1;


		/* If compatible, we or it in below.  It is compatible if is
		 * in the bitmp and either 1) its bit or its fold is set, or 2)
		 * it's for a locale.  Even if there isn't unicode semantics
		 * here, at runtime there may be because of matching against a
		 * utf8 string, so accept a possible false positive for
		 * latin1-range folds */
		if (uc >= 0x100 ||
		    (!(data->start_class->flags & (ANYOF_CLASS | ANYOF_LOCALE))
		    && !ANYOF_BITMAP_TEST(data->start_class, uc)
		    && (!(data->start_class->flags & ANYOF_LOC_NONBITMAP_FOLD)
			|| !ANYOF_BITMAP_TEST(data->start_class, PL_fold_latin1[uc])))
                    )
		{
		    compat = 0;
		}
		ANYOF_CLASS_ZERO(data->start_class);
		ANYOF_BITMAP_ZERO(data->start_class);
		if (compat)
		    ANYOF_BITMAP_SET(data->start_class, uc);
		else if (uc >= 0x100) {
		    int i;

		    /* Some Unicode code points fold to the Latin1 range; as
		     * XXX temporary code, instead of figuring out if this is
		     * one, just assume it is and set all the start class bits
		     * that could be some such above 255 code point's fold
		     * which will generate fals positives.  As the code
		     * elsewhere that does compute the fold settles down, it
		     * can be extracted out and re-used here */
		    for (i = 0; i < 256; i++){
			if (_HAS_NONLATIN1_FOLD_CLOSURE_ONLY_FOR_USE_BY_REGCOMP_DOT_C_AND_REGEXEC_DOT_C(i)) {
			    ANYOF_BITMAP_SET(data->start_class, i);
			}
		    }
		}
		data->start_class->flags &= ~ANYOF_EOS;
		if (uc < 0x100)
		  data->start_class->flags &= ~ANYOF_UNICODE_ALL;
	    }
	    else if (flags & SCF_DO_STCLASS_OR) {
		/* false positive possible if the class is case-folded */
		if (uc < 0x100)
		    ANYOF_BITMAP_SET(data->start_class, uc);
		else
		    data->start_class->flags |= ANYOF_UNICODE_ALL;
		data->start_class->flags &= ~ANYOF_EOS;
		cl_and(data->start_class, and_withp);
	    }
	    flags &= ~SCF_DO_STCLASS;
	}
	else if (PL_regkind[OP(scan)] == EXACT) { /* But OP != EXACT! */
	    I32 l = STR_LEN(scan);
	    UV uc = *((U8*)STRING(scan));

	    /* Search for fixed substrings supports EXACT only. */
	    if (flags & SCF_DO_SUBSTR) {
		assert(data);
		SCAN_COMMIT(pRExC_state, data, minlenp);
	    }
	    if (UTF) {
		const U8 * const s = (U8 *)STRING(scan);
		uc = utf8_to_uvchr_buf(s, s + l, NULL);
		l = utf8_length(s, s + l);
	    }
	    else if (has_exactf_sharp_s) {
		RExC_seen |= REG_SEEN_EXACTF_SHARP_S;
	    }
	    min += l - min_subtract;
            if (min < 0) {
                min = 0;
            }
            delta += min_subtract;
	    if (flags & SCF_DO_SUBSTR) {
		data->pos_min += l - min_subtract;
		if (data->pos_min < 0) {
                    data->pos_min = 0;
                }
                data->pos_delta += min_subtract;
		if (min_subtract) {
		    data->longest = &(data->longest_float);
		}
	    }
	    if (flags & SCF_DO_STCLASS_AND) {
		/* Check whether it is compatible with what we know already! */
		int compat = 1;
		if (uc >= 0x100 ||
		 (!(data->start_class->flags & (ANYOF_CLASS | ANYOF_LOCALE))
		  && !ANYOF_BITMAP_TEST(data->start_class, uc)
		  && !ANYOF_BITMAP_TEST(data->start_class, PL_fold_latin1[uc])))
		{
		    compat = 0;
		}
		ANYOF_CLASS_ZERO(data->start_class);
		ANYOF_BITMAP_ZERO(data->start_class);
		if (compat) {
		    ANYOF_BITMAP_SET(data->start_class, uc);
		    data->start_class->flags &= ~ANYOF_EOS;
		    data->start_class->flags |= ANYOF_LOC_NONBITMAP_FOLD;
		    if (OP(scan) == EXACTFL) {
			/* XXX This set is probably no longer necessary, and
			 * probably wrong as LOCALE now is on in the initial
			 * state */
			data->start_class->flags |= ANYOF_LOCALE;
		    }
		    else {

			/* Also set the other member of the fold pair.  In case
			 * that unicode semantics is called for at runtime, use
			 * the full latin1 fold.  (Can't do this for locale,
			 * because not known until runtime) */
			ANYOF_BITMAP_SET(data->start_class, PL_fold_latin1[uc]);

                        /* All other (EXACTFL handled above) folds except under
                         * /iaa that include s, S, and sharp_s also may include
                         * the others */
			if (OP(scan) != EXACTFA) {
			    if (uc == 's' || uc == 'S') {
				ANYOF_BITMAP_SET(data->start_class,
					         LATIN_SMALL_LETTER_SHARP_S);
			    }
			    else if (uc == LATIN_SMALL_LETTER_SHARP_S) {
				ANYOF_BITMAP_SET(data->start_class, 's');
				ANYOF_BITMAP_SET(data->start_class, 'S');
			    }
			}
		    }
		}
		else if (uc >= 0x100) {
		    int i;
		    for (i = 0; i < 256; i++){
			if (_HAS_NONLATIN1_FOLD_CLOSURE_ONLY_FOR_USE_BY_REGCOMP_DOT_C_AND_REGEXEC_DOT_C(i)) {
			    ANYOF_BITMAP_SET(data->start_class, i);
			}
		    }
		}
	    }
	    else if (flags & SCF_DO_STCLASS_OR) {
		if (data->start_class->flags & ANYOF_LOC_NONBITMAP_FOLD) {
		    /* false positive possible if the class is case-folded.
		       Assume that the locale settings are the same... */
		    if (uc < 0x100) {
			ANYOF_BITMAP_SET(data->start_class, uc);
                        if (OP(scan) != EXACTFL) {

                            /* And set the other member of the fold pair, but
                             * can't do that in locale because not known until
                             * run-time */
                            ANYOF_BITMAP_SET(data->start_class,
					     PL_fold_latin1[uc]);

			    /* All folds except under /iaa that include s, S,
			     * and sharp_s also may include the others */
			    if (OP(scan) != EXACTFA) {
				if (uc == 's' || uc == 'S') {
				    ANYOF_BITMAP_SET(data->start_class,
					           LATIN_SMALL_LETTER_SHARP_S);
				}
				else if (uc == LATIN_SMALL_LETTER_SHARP_S) {
				    ANYOF_BITMAP_SET(data->start_class, 's');
				    ANYOF_BITMAP_SET(data->start_class, 'S');
				}
			    }
                        }
		    }
		    data->start_class->flags &= ~ANYOF_EOS;
		}
		cl_and(data->start_class, and_withp);
	    }
	    flags &= ~SCF_DO_STCLASS;
	}
	else if (REGNODE_VARIES(OP(scan))) {
	    I32 mincount, maxcount, minnext, deltanext, fl = 0;
	    I32 f = flags, pos_before = 0;
	    regnode * const oscan = scan;
	    struct regnode_charclass_class this_class;
	    struct regnode_charclass_class *oclass = NULL;
	    I32 next_is_eval = 0;

	    switch (PL_regkind[OP(scan)]) {
	    case WHILEM:		/* End of (?:...)* . */
		scan = NEXTOPER(scan);
		goto finish;
	    case PLUS:
		if (flags & (SCF_DO_SUBSTR | SCF_DO_STCLASS)) {
		    next = NEXTOPER(scan);
		    if (OP(next) == EXACT || (flags & SCF_DO_STCLASS)) {
			mincount = 1;
			maxcount = REG_INFTY;
			next = regnext(scan);
			scan = NEXTOPER(scan);
			goto do_curly;
		    }
		}
		if (flags & SCF_DO_SUBSTR)
		    data->pos_min++;
		min++;
		/* Fall through. */
	    case STAR:
		if (flags & SCF_DO_STCLASS) {
		    mincount = 0;
		    maxcount = REG_INFTY;
		    next = regnext(scan);
		    scan = NEXTOPER(scan);
		    goto do_curly;
		}
		is_inf = is_inf_internal = 1;
		scan = regnext(scan);
		if (flags & SCF_DO_SUBSTR) {
		    SCAN_COMMIT(pRExC_state, data, minlenp); /* Cannot extend fixed substrings */
		    data->longest = &(data->longest_float);
		}
		goto optimize_curly_tail;
	    case CURLY:
	        if (stopparen>0 && (OP(scan)==CURLYN || OP(scan)==CURLYM)
	            && (scan->flags == stopparen))
		{
		    mincount = 1;
		    maxcount = 1;
		} else {
		    mincount = ARG1(scan);
		    maxcount = ARG2(scan);
		}
		next = regnext(scan);
		if (OP(scan) == CURLYX) {
		    I32 lp = (data ? *(data->last_closep) : 0);
		    scan->flags = ((lp <= (I32)U8_MAX) ? (U8)lp : U8_MAX);
		}
		scan = NEXTOPER(scan) + EXTRA_STEP_2ARGS;
		next_is_eval = (OP(scan) == EVAL);
	      do_curly:
		if (flags & SCF_DO_SUBSTR) {
		    if (mincount == 0) SCAN_COMMIT(pRExC_state,data,minlenp); /* Cannot extend fixed substrings */
		    pos_before = data->pos_min;
		}
		if (data) {
		    fl = data->flags;
		    data->flags &= ~(SF_HAS_PAR|SF_IN_PAR|SF_HAS_EVAL);
		    if (is_inf)
			data->flags |= SF_IS_INF;
		}
		if (flags & SCF_DO_STCLASS) {
		    cl_init(pRExC_state, &this_class);
		    oclass = data->start_class;
		    data->start_class = &this_class;
		    f |= SCF_DO_STCLASS_AND;
		    f &= ~SCF_DO_STCLASS_OR;
		}
	        /* Exclude from super-linear cache processing any {n,m}
		   regops for which the combination of input pos and regex
		   pos is not enough information to determine if a match
		   will be possible.

		   For example, in the regex /foo(bar\s*){4,8}baz/ with the
		   regex pos at the \s*, the prospects for a match depend not
		   only on the input position but also on how many (bar\s*)
		   repeats into the {4,8} we are. */
               if ((mincount > 1) || (maxcount > 1 && maxcount != REG_INFTY))
		    f &= ~SCF_WHILEM_VISITED_POS;

		/* This will finish on WHILEM, setting scan, or on NULL: */
		minnext = study_chunk(pRExC_state, &scan, minlenp, &deltanext, 
		                      last, data, stopparen, recursed, NULL,
				      (mincount == 0
					? (f & ~SCF_DO_SUBSTR) : f),depth+1);

		if (flags & SCF_DO_STCLASS)
		    data->start_class = oclass;
		if (mincount == 0 || minnext == 0) {
		    if (flags & SCF_DO_STCLASS_OR) {
			cl_or(pRExC_state, data->start_class, &this_class);
		    }
		    else if (flags & SCF_DO_STCLASS_AND) {
			/* Switch to OR mode: cache the old value of
			 * data->start_class */
			INIT_AND_WITHP;
			StructCopy(data->start_class, and_withp,
				   struct regnode_charclass_class);
			flags &= ~SCF_DO_STCLASS_AND;
			StructCopy(&this_class, data->start_class,
				   struct regnode_charclass_class);
			flags |= SCF_DO_STCLASS_OR;
			data->start_class->flags |= ANYOF_EOS;
		    }
		} else {		/* Non-zero len */
		    if (flags & SCF_DO_STCLASS_OR) {
			cl_or(pRExC_state, data->start_class, &this_class);
			cl_and(data->start_class, and_withp);
		    }
		    else if (flags & SCF_DO_STCLASS_AND)
			cl_and(data->start_class, &this_class);
		    flags &= ~SCF_DO_STCLASS;
		}
		if (!scan) 		/* It was not CURLYX, but CURLY. */
		    scan = next;
		if ( /* ? quantifier ok, except for (?{ ... }) */
		    (next_is_eval || !(mincount == 0 && maxcount == 1))
		    && (minnext == 0) && (deltanext == 0)
		    && data && !(data->flags & (SF_HAS_PAR|SF_IN_PAR))
		    && maxcount <= REG_INFTY/3) /* Complement check for big count */
		{
		    ckWARNreg(RExC_parse,
			      "Quantifier unexpected on zero-length expression");
		}

		min += minnext * mincount;
		is_inf_internal |= ((maxcount == REG_INFTY
				     && (minnext + deltanext) > 0)
				    || deltanext == I32_MAX);
		is_inf |= is_inf_internal;
		delta += (minnext + deltanext) * maxcount - minnext * mincount;

		/* Try powerful optimization CURLYX => CURLYN. */
		if (  OP(oscan) == CURLYX && data
		      && data->flags & SF_IN_PAR
		      && !(data->flags & SF_HAS_EVAL)
		      && !deltanext && minnext == 1 ) {
		    /* Try to optimize to CURLYN.  */
		    regnode *nxt = NEXTOPER(oscan) + EXTRA_STEP_2ARGS;
		    regnode * const nxt1 = nxt;
#ifdef DEBUGGING
		    regnode *nxt2;
#endif

		    /* Skip open. */
		    nxt = regnext(nxt);
		    if (!REGNODE_SIMPLE(OP(nxt))
			&& !(PL_regkind[OP(nxt)] == EXACT
			     && STR_LEN(nxt) == 1))
			goto nogo;
#ifdef DEBUGGING
		    nxt2 = nxt;
#endif
		    nxt = regnext(nxt);
		    if (OP(nxt) != CLOSE)
			goto nogo;
		    if (RExC_open_parens) {
			RExC_open_parens[ARG(nxt1)-1]=oscan; /*open->CURLYM*/
			RExC_close_parens[ARG(nxt1)-1]=nxt+2; /*close->while*/
		    }
		    /* Now we know that nxt2 is the only contents: */
		    oscan->flags = (U8)ARG(nxt);
		    OP(oscan) = CURLYN;
		    OP(nxt1) = NOTHING;	/* was OPEN. */

#ifdef DEBUGGING
		    OP(nxt1 + 1) = OPTIMIZED; /* was count. */
		    NEXT_OFF(nxt1+ 1) = 0; /* just for consistency. */
		    NEXT_OFF(nxt2) = 0;	/* just for consistency with CURLY. */
		    OP(nxt) = OPTIMIZED;	/* was CLOSE. */
		    OP(nxt + 1) = OPTIMIZED; /* was count. */
		    NEXT_OFF(nxt+ 1) = 0; /* just for consistency. */
#endif
		}
	      nogo:

		/* Try optimization CURLYX => CURLYM. */
		if (  OP(oscan) == CURLYX && data
		      && !(data->flags & SF_HAS_PAR)
		      && !(data->flags & SF_HAS_EVAL)
		      && !deltanext	/* atom is fixed width */
		      && minnext != 0	/* CURLYM can't handle zero width */
		) {
		    /* XXXX How to optimize if data == 0? */
		    /* Optimize to a simpler form.  */
		    regnode *nxt = NEXTOPER(oscan) + EXTRA_STEP_2ARGS; /* OPEN */
		    regnode *nxt2;

		    OP(oscan) = CURLYM;
		    while ( (nxt2 = regnext(nxt)) /* skip over embedded stuff*/
			    && (OP(nxt2) != WHILEM))
			nxt = nxt2;
		    OP(nxt2)  = SUCCEED; /* Whas WHILEM */
		    /* Need to optimize away parenths. */
		    if ((data->flags & SF_IN_PAR) && OP(nxt) == CLOSE) {
			/* Set the parenth number.  */
			regnode *nxt1 = NEXTOPER(oscan) + EXTRA_STEP_2ARGS; /* OPEN*/

			oscan->flags = (U8)ARG(nxt);
			if (RExC_open_parens) {
			    RExC_open_parens[ARG(nxt1)-1]=oscan; /*open->CURLYM*/
			    RExC_close_parens[ARG(nxt1)-1]=nxt2+1; /*close->NOTHING*/
			}
			OP(nxt1) = OPTIMIZED;	/* was OPEN. */
			OP(nxt) = OPTIMIZED;	/* was CLOSE. */

#ifdef DEBUGGING
			OP(nxt1 + 1) = OPTIMIZED; /* was count. */
			OP(nxt + 1) = OPTIMIZED; /* was count. */
			NEXT_OFF(nxt1 + 1) = 0; /* just for consistency. */
			NEXT_OFF(nxt + 1) = 0; /* just for consistency. */
#endif
#if 0
			while ( nxt1 && (OP(nxt1) != WHILEM)) {
			    regnode *nnxt = regnext(nxt1);
			    if (nnxt == nxt) {
				if (reg_off_by_arg[OP(nxt1)])
				    ARG_SET(nxt1, nxt2 - nxt1);
				else if (nxt2 - nxt1 < U16_MAX)
				    NEXT_OFF(nxt1) = nxt2 - nxt1;
				else
				    OP(nxt) = NOTHING;	/* Cannot beautify */
			    }
			    nxt1 = nnxt;
			}
#endif
			/* Optimize again: */
			study_chunk(pRExC_state, &nxt1, minlenp, &deltanext, nxt,
				    NULL, stopparen, recursed, NULL, 0,depth+1);
		    }
		    else
			oscan->flags = 0;
		}
		else if ((OP(oscan) == CURLYX)
			 && (flags & SCF_WHILEM_VISITED_POS)
			 /* See the comment on a similar expression above.
			    However, this time it's not a subexpression
			    we care about, but the expression itself. */
			 && (maxcount == REG_INFTY)
			 && data && ++data->whilem_c < 16) {
		    /* This stays as CURLYX, we can put the count/of pair. */
		    /* Find WHILEM (as in regexec.c) */
		    regnode *nxt = oscan + NEXT_OFF(oscan);

		    if (OP(PREVOPER(nxt)) == NOTHING) /* LONGJMP */
			nxt += ARG(nxt);
		    PREVOPER(nxt)->flags = (U8)(data->whilem_c
			| (RExC_whilem_seen << 4)); /* On WHILEM */
		}
		if (data && fl & (SF_HAS_PAR|SF_IN_PAR))
		    pars++;
		if (flags & SCF_DO_SUBSTR) {
		    SV *last_str = NULL;
		    int counted = mincount != 0;

		    if (data->last_end > 0 && mincount != 0) { /* Ends with a string. */
#if defined(SPARC64_GCC_WORKAROUND)
			I32 b = 0;
			STRLEN l = 0;
			const char *s = NULL;
			I32 old = 0;

			if (pos_before >= data->last_start_min)
			    b = pos_before;
			else
			    b = data->last_start_min;

			l = 0;
			s = SvPV_const(data->last_found, l);
			old = b - data->last_start_min;

#else
			I32 b = pos_before >= data->last_start_min
			    ? pos_before : data->last_start_min;
			STRLEN l;
			const char * const s = SvPV_const(data->last_found, l);
			I32 old = b - data->last_start_min;
#endif

			if (UTF)
			    old = utf8_hop((U8*)s, old) - (U8*)s;
			l -= old;
			/* Get the added string: */
			last_str = newSVpvn_utf8(s  + old, l, UTF);
			if (deltanext == 0 && pos_before == b) {
			    /* What was added is a constant string */
			    if (mincount > 1) {
				SvGROW(last_str, (mincount * l) + 1);
				repeatcpy(SvPVX(last_str) + l,
					  SvPVX_const(last_str), l, mincount - 1);
				SvCUR_set(last_str, SvCUR(last_str) * mincount);
				/* Add additional parts. */
				SvCUR_set(data->last_found,
					  SvCUR(data->last_found) - l);
				sv_catsv(data->last_found, last_str);
				{
				    SV * sv = data->last_found;
				    MAGIC *mg =
					SvUTF8(sv) && SvMAGICAL(sv) ?
					mg_find(sv, PERL_MAGIC_utf8) : NULL;
				    if (mg && mg->mg_len >= 0)
					mg->mg_len += CHR_SVLEN(last_str) - l;
				}
				data->last_end += l * (mincount - 1);
			    }
			} else {
			    /* start offset must point into the last copy */
			    data->last_start_min += minnext * (mincount - 1);
			    data->last_start_max += is_inf ? I32_MAX
				: (maxcount - 1) * (minnext + data->pos_delta);
			}
		    }
		    /* It is counted once already... */
		    data->pos_min += minnext * (mincount - counted);
		    data->pos_delta += - counted * deltanext +
			(minnext + deltanext) * maxcount - minnext * mincount;
		    if (mincount != maxcount) {
			 /* Cannot extend fixed substrings found inside
			    the group.  */
			SCAN_COMMIT(pRExC_state,data,minlenp);
			if (mincount && last_str) {
			    SV * const sv = data->last_found;
			    MAGIC * const mg = SvUTF8(sv) && SvMAGICAL(sv) ?
				mg_find(sv, PERL_MAGIC_utf8) : NULL;

			    if (mg)
				mg->mg_len = -1;
			    sv_setsv(sv, last_str);
			    data->last_end = data->pos_min;
			    data->last_start_min =
				data->pos_min - CHR_SVLEN(last_str);
			    data->last_start_max = is_inf
				? I32_MAX
				: data->pos_min + data->pos_delta
				- CHR_SVLEN(last_str);
			}
			data->longest = &(data->longest_float);
		    }
		    SvREFCNT_dec(last_str);
		}
		if (data && (fl & SF_HAS_EVAL))
		    data->flags |= SF_HAS_EVAL;
	      optimize_curly_tail:
		if (OP(oscan) != CURLYX) {
		    while (PL_regkind[OP(next = regnext(oscan))] == NOTHING
			   && NEXT_OFF(next))
			NEXT_OFF(oscan) += NEXT_OFF(next);
		}
		continue;
	    default:			/* REF, ANYOFV, and CLUMP only? */
		if (flags & SCF_DO_SUBSTR) {
		    SCAN_COMMIT(pRExC_state,data,minlenp);	/* Cannot expect anything... */
		    data->longest = &(data->longest_float);
		}
		is_inf = is_inf_internal = 1;
		if (flags & SCF_DO_STCLASS_OR)
		    cl_anything(pRExC_state, data->start_class);
		flags &= ~SCF_DO_STCLASS;
		break;
	    }
	}
	else if (OP(scan) == LNBREAK) {
	    if (flags & SCF_DO_STCLASS) {
		int value = 0;
		data->start_class->flags &= ~ANYOF_EOS;	/* No match on empty */
    	        if (flags & SCF_DO_STCLASS_AND) {
                    for (value = 0; value < 256; value++)
                        if (!is_VERTWS_cp(value))
                            ANYOF_BITMAP_CLEAR(data->start_class, value);
                }
                else {
                    for (value = 0; value < 256; value++)
                        if (is_VERTWS_cp(value))
                            ANYOF_BITMAP_SET(data->start_class, value);
                }
                if (flags & SCF_DO_STCLASS_OR)
		    cl_and(data->start_class, and_withp);
		flags &= ~SCF_DO_STCLASS;
            }
	    min += 1;
	    delta += 1;
            if (flags & SCF_DO_SUBSTR) {
    	        SCAN_COMMIT(pRExC_state,data,minlenp);	/* Cannot expect anything... */
    	        data->pos_min += 1;
	        data->pos_delta += 1;
		data->longest = &(data->longest_float);
    	    }
	}
	else if (REGNODE_SIMPLE(OP(scan))) {
	    int value = 0;

	    if (flags & SCF_DO_SUBSTR) {
		SCAN_COMMIT(pRExC_state,data,minlenp);
		data->pos_min++;
	    }
	    min++;
	    if (flags & SCF_DO_STCLASS) {
		data->start_class->flags &= ~ANYOF_EOS;	/* No match on empty */

		/* Some of the logic below assumes that switching
		   locale on will only add false positives. */
		switch (PL_regkind[OP(scan)]) {
		case SANY:
		default:
		  do_default:
		    /* Perl_croak(aTHX_ "panic: unexpected simple REx opcode %d", OP(scan)); */
		    if (flags & SCF_DO_STCLASS_OR) /* Allow everything */
			cl_anything(pRExC_state, data->start_class);
		    break;
		case REG_ANY:
		    if (OP(scan) == SANY)
			goto do_default;
		    if (flags & SCF_DO_STCLASS_OR) { /* Everything but \n */
			value = (ANYOF_BITMAP_TEST(data->start_class,'\n')
				 || ANYOF_CLASS_TEST_ANY_SET(data->start_class));
			cl_anything(pRExC_state, data->start_class);
		    }
		    if (flags & SCF_DO_STCLASS_AND || !value)
			ANYOF_BITMAP_CLEAR(data->start_class,'\n');
		    break;
		case ANYOF:
		    if (flags & SCF_DO_STCLASS_AND)
			cl_and(data->start_class,
			       (struct regnode_charclass_class*)scan);
		    else
			cl_or(pRExC_state, data->start_class,
			      (struct regnode_charclass_class*)scan);
		    break;
		case ALNUM:
		    if (flags & SCF_DO_STCLASS_AND) {
			if (!(data->start_class->flags & ANYOF_LOCALE)) {
			    ANYOF_CLASS_CLEAR(data->start_class,ANYOF_NALNUM);
                            if (OP(scan) == ALNUMU) {
                                for (value = 0; value < 256; value++) {
                                    if (!isWORDCHAR_L1(value)) {
                                        ANYOF_BITMAP_CLEAR(data->start_class, value);
                                    }
                                }
                            } else {
                                for (value = 0; value < 256; value++) {
                                    if (!isALNUM(value)) {
                                        ANYOF_BITMAP_CLEAR(data->start_class, value);
                                    }
                                }
                            }
			}
		    }
		    else {
			if (data->start_class->flags & ANYOF_LOCALE)
			    ANYOF_CLASS_SET(data->start_class,ANYOF_ALNUM);

			/* Even if under locale, set the bits for non-locale
			 * in case it isn't a true locale-node.  This will
			 * create false positives if it truly is locale */
                        if (OP(scan) == ALNUMU) {
                            for (value = 0; value < 256; value++) {
                                if (isWORDCHAR_L1(value)) {
                                    ANYOF_BITMAP_SET(data->start_class, value);
                                }
                            }
                        } else {
                            for (value = 0; value < 256; value++) {
                                if (isALNUM(value)) {
                                    ANYOF_BITMAP_SET(data->start_class, value);
                                }
                            }
                        }
		    }
		    break;
		case NALNUM:
		    if (flags & SCF_DO_STCLASS_AND) {
			if (!(data->start_class->flags & ANYOF_LOCALE)) {
			    ANYOF_CLASS_CLEAR(data->start_class,ANYOF_ALNUM);
                            if (OP(scan) == NALNUMU) {
                                for (value = 0; value < 256; value++) {
                                    if (isWORDCHAR_L1(value)) {
                                        ANYOF_BITMAP_CLEAR(data->start_class, value);
                                    }
                                }
                            } else {
                                for (value = 0; value < 256; value++) {
                                    if (isALNUM(value)) {
                                        ANYOF_BITMAP_CLEAR(data->start_class, value);
                                    }
                                }
			    }
			}
		    }
		    else {
			if (data->start_class->flags & ANYOF_LOCALE)
			    ANYOF_CLASS_SET(data->start_class,ANYOF_NALNUM);

			/* Even if under locale, set the bits for non-locale in
			 * case it isn't a true locale-node.  This will create
			 * false positives if it truly is locale */
			if (OP(scan) == NALNUMU) {
			    for (value = 0; value < 256; value++) {
				if (! isWORDCHAR_L1(value)) {
				    ANYOF_BITMAP_SET(data->start_class, value);
				}
			    }
			} else {
			    for (value = 0; value < 256; value++) {
				if (! isALNUM(value)) {
				    ANYOF_BITMAP_SET(data->start_class, value);
				}
			    }
			}
		    }
		    break;
		case SPACE:
		    if (flags & SCF_DO_STCLASS_AND) {
			if (!(data->start_class->flags & ANYOF_LOCALE)) {
			    ANYOF_CLASS_CLEAR(data->start_class,ANYOF_NSPACE);
			    if (OP(scan) == SPACEU) {
                                for (value = 0; value < 256; value++) {
                                    if (!isSPACE_L1(value)) {
                                        ANYOF_BITMAP_CLEAR(data->start_class, value);
                                    }
                                }
                            } else {
                                for (value = 0; value < 256; value++) {
                                    if (!isSPACE(value)) {
                                        ANYOF_BITMAP_CLEAR(data->start_class, value);
                                    }
                                }
                            }
			}
		    }
		    else {
                        if (data->start_class->flags & ANYOF_LOCALE) {
			    ANYOF_CLASS_SET(data->start_class,ANYOF_SPACE);
                        }
                        if (OP(scan) == SPACEU) {
                            for (value = 0; value < 256; value++) {
                                if (isSPACE_L1(value)) {
                                    ANYOF_BITMAP_SET(data->start_class, value);
                                }
                            }
                        } else {
                            for (value = 0; value < 256; value++) {
                                if (isSPACE(value)) {
                                    ANYOF_BITMAP_SET(data->start_class, value);
                                }
                            }
			}
		    }
		    break;
		case NSPACE:
		    if (flags & SCF_DO_STCLASS_AND) {
			if (!(data->start_class->flags & ANYOF_LOCALE)) {
			    ANYOF_CLASS_CLEAR(data->start_class,ANYOF_SPACE);
                            if (OP(scan) == NSPACEU) {
                                for (value = 0; value < 256; value++) {
                                    if (isSPACE_L1(value)) {
                                        ANYOF_BITMAP_CLEAR(data->start_class, value);
                                    }
                                }
                            } else {
                                for (value = 0; value < 256; value++) {
                                    if (isSPACE(value)) {
                                        ANYOF_BITMAP_CLEAR(data->start_class, value);
                                    }
                                }
                            }
			}
		    }
		    else {
			if (data->start_class->flags & ANYOF_LOCALE)
			    ANYOF_CLASS_SET(data->start_class,ANYOF_NSPACE);
                        if (OP(scan) == NSPACEU) {
                            for (value = 0; value < 256; value++) {
                                if (!isSPACE_L1(value)) {
                                    ANYOF_BITMAP_SET(data->start_class, value);
                                }
                            }
                        }
                        else {
                            for (value = 0; value < 256; value++) {
                                if (!isSPACE(value)) {
                                    ANYOF_BITMAP_SET(data->start_class, value);
                                }
                            }
                        }
		    }
		    break;
		case DIGIT:
		    if (flags & SCF_DO_STCLASS_AND) {
			if (!(data->start_class->flags & ANYOF_LOCALE)) {
                            ANYOF_CLASS_CLEAR(data->start_class,ANYOF_NDIGIT);
			    for (value = 0; value < 256; value++)
				if (!isDIGIT(value))
				    ANYOF_BITMAP_CLEAR(data->start_class, value);
			}
		    }
		    else {
			if (data->start_class->flags & ANYOF_LOCALE)
			    ANYOF_CLASS_SET(data->start_class,ANYOF_DIGIT);
			for (value = 0; value < 256; value++)
			    if (isDIGIT(value))
				ANYOF_BITMAP_SET(data->start_class, value);
		    }
		    break;
		case NDIGIT:
		    if (flags & SCF_DO_STCLASS_AND) {
			if (!(data->start_class->flags & ANYOF_LOCALE))
                            ANYOF_CLASS_CLEAR(data->start_class,ANYOF_DIGIT);
			for (value = 0; value < 256; value++)
			    if (isDIGIT(value))
				ANYOF_BITMAP_CLEAR(data->start_class, value);
		    }
		    else {
			if (data->start_class->flags & ANYOF_LOCALE)
			    ANYOF_CLASS_SET(data->start_class,ANYOF_NDIGIT);
			for (value = 0; value < 256; value++)
			    if (!isDIGIT(value))
				ANYOF_BITMAP_SET(data->start_class, value);
		    }
		    break;
		CASE_SYNST_FNC(VERTWS);
		CASE_SYNST_FNC(HORIZWS);

		}
		if (flags & SCF_DO_STCLASS_OR)
		    cl_and(data->start_class, and_withp);
		flags &= ~SCF_DO_STCLASS;
	    }
	}
	else if (PL_regkind[OP(scan)] == EOL && flags & SCF_DO_SUBSTR) {
	    data->flags |= (OP(scan) == MEOL
			    ? SF_BEFORE_MEOL
			    : SF_BEFORE_SEOL);
	}
	else if (  PL_regkind[OP(scan)] == BRANCHJ
		 /* Lookbehind, or need to calculate parens/evals/stclass: */
		   && (scan->flags || data || (flags & SCF_DO_STCLASS))
		   && (OP(scan) == IFMATCH || OP(scan) == UNLESSM)) {
            if ( !PERL_ENABLE_POSITIVE_ASSERTION_STUDY 
                || OP(scan) == UNLESSM )
            {
                /* Negative Lookahead/lookbehind
                   In this case we can't do fixed string optimisation.
                */

                I32 deltanext, minnext, fake = 0;
                regnode *nscan;
                struct regnode_charclass_class intrnl;
                int f = 0;

                data_fake.flags = 0;
                if (data) {
                    data_fake.whilem_c = data->whilem_c;
                    data_fake.last_closep = data->last_closep;
		}
                else
                    data_fake.last_closep = &fake;
		data_fake.pos_delta = delta;
                if ( flags & SCF_DO_STCLASS && !scan->flags
                     && OP(scan) == IFMATCH ) { /* Lookahead */
                    cl_init(pRExC_state, &intrnl);
                    data_fake.start_class = &intrnl;
                    f |= SCF_DO_STCLASS_AND;
		}
                if (flags & SCF_WHILEM_VISITED_POS)
                    f |= SCF_WHILEM_VISITED_POS;
                next = regnext(scan);
                nscan = NEXTOPER(NEXTOPER(scan));
                minnext = study_chunk(pRExC_state, &nscan, minlenp, &deltanext, 
                    last, &data_fake, stopparen, recursed, NULL, f, depth+1);
                if (scan->flags) {
                    if (deltanext) {
			FAIL("Variable length lookbehind not implemented");
                    }
                    else if (minnext > (I32)U8_MAX) {
			FAIL2("Lookbehind longer than %"UVuf" not implemented", (UV)U8_MAX);
                    }
                    scan->flags = (U8)minnext;
                }
                if (data) {
                    if (data_fake.flags & (SF_HAS_PAR|SF_IN_PAR))
                        pars++;
                    if (data_fake.flags & SF_HAS_EVAL)
                        data->flags |= SF_HAS_EVAL;
                    data->whilem_c = data_fake.whilem_c;
                }
                if (f & SCF_DO_STCLASS_AND) {
		    if (flags & SCF_DO_STCLASS_OR) {
			/* OR before, AND after: ideally we would recurse with
			 * data_fake to get the AND applied by study of the
			 * remainder of the pattern, and then derecurse;
			 * *** HACK *** for now just treat as "no information".
			 * See [perl #56690].
			 */
			cl_init(pRExC_state, data->start_class);
		    }  else {
			/* AND before and after: combine and continue */
			const int was = (data->start_class->flags & ANYOF_EOS);

			cl_and(data->start_class, &intrnl);
			if (was)
			    data->start_class->flags |= ANYOF_EOS;
		    }
                }
	    }
#if PERL_ENABLE_POSITIVE_ASSERTION_STUDY
            else {
                /* Positive Lookahead/lookbehind
                   In this case we can do fixed string optimisation,
                   but we must be careful about it. Note in the case of
                   lookbehind the positions will be offset by the minimum
                   length of the pattern, something we won't know about
                   until after the recurse.
                */
                I32 deltanext, fake = 0;
                regnode *nscan;
                struct regnode_charclass_class intrnl;
                int f = 0;
                /* We use SAVEFREEPV so that when the full compile 
                    is finished perl will clean up the allocated 
                    minlens when it's all done. This way we don't
                    have to worry about freeing them when we know
                    they wont be used, which would be a pain.
                 */
                I32 *minnextp;
                Newx( minnextp, 1, I32 );
                SAVEFREEPV(minnextp);

                if (data) {
                    StructCopy(data, &data_fake, scan_data_t);
                    if ((flags & SCF_DO_SUBSTR) && data->last_found) {
                        f |= SCF_DO_SUBSTR;
                        if (scan->flags) 
                            SCAN_COMMIT(pRExC_state, &data_fake,minlenp);
                        data_fake.last_found=newSVsv(data->last_found);
                    }
                }
                else
                    data_fake.last_closep = &fake;
                data_fake.flags = 0;
		data_fake.pos_delta = delta;
                if (is_inf)
	            data_fake.flags |= SF_IS_INF;
                if ( flags & SCF_DO_STCLASS && !scan->flags
                     && OP(scan) == IFMATCH ) { /* Lookahead */
                    cl_init(pRExC_state, &intrnl);
                    data_fake.start_class = &intrnl;
                    f |= SCF_DO_STCLASS_AND;
                }
                if (flags & SCF_WHILEM_VISITED_POS)
                    f |= SCF_WHILEM_VISITED_POS;
                next = regnext(scan);
                nscan = NEXTOPER(NEXTOPER(scan));

                *minnextp = study_chunk(pRExC_state, &nscan, minnextp, &deltanext, 
                    last, &data_fake, stopparen, recursed, NULL, f,depth+1);
                if (scan->flags) {
                    if (deltanext) {
			FAIL("Variable length lookbehind not implemented");
                    }
                    else if (*minnextp > (I32)U8_MAX) {
			FAIL2("Lookbehind longer than %"UVuf" not implemented", (UV)U8_MAX);
                    }
                    scan->flags = (U8)*minnextp;
                }

                *minnextp += min;

                if (f & SCF_DO_STCLASS_AND) {
                    const int was = (data->start_class->flags & ANYOF_EOS);

                    cl_and(data->start_class, &intrnl);
                    if (was)
                        data->start_class->flags |= ANYOF_EOS;
                }
                if (data) {
                    if (data_fake.flags & (SF_HAS_PAR|SF_IN_PAR))
                        pars++;
                    if (data_fake.flags & SF_HAS_EVAL)
                        data->flags |= SF_HAS_EVAL;
                    data->whilem_c = data_fake.whilem_c;
                    if ((flags & SCF_DO_SUBSTR) && data_fake.last_found) {
                        if (RExC_rx->minlen<*minnextp)
                            RExC_rx->minlen=*minnextp;
                        SCAN_COMMIT(pRExC_state, &data_fake, minnextp);
                        SvREFCNT_dec(data_fake.last_found);
                        
                        if ( data_fake.minlen_fixed != minlenp ) 
                        {
                            data->offset_fixed= data_fake.offset_fixed;
                            data->minlen_fixed= data_fake.minlen_fixed;
                            data->lookbehind_fixed+= scan->flags;
                        }
                        if ( data_fake.minlen_float != minlenp )
                        {
                            data->minlen_float= data_fake.minlen_float;
                            data->offset_float_min=data_fake.offset_float_min;
                            data->offset_float_max=data_fake.offset_float_max;
                            data->lookbehind_float+= scan->flags;
                        }
                    }
                }


	    }
#endif
	}
	else if (OP(scan) == OPEN) {
	    if (stopparen != (I32)ARG(scan))
	        pars++;
	}
	else if (OP(scan) == CLOSE) {
	    if (stopparen == (I32)ARG(scan)) {
	        break;
	    }
	    if ((I32)ARG(scan) == is_par) {
		next = regnext(scan);

		if ( next && (OP(next) != WHILEM) && next < last)
		    is_par = 0;		/* Disable optimization */
	    }
	    if (data)
		*(data->last_closep) = ARG(scan);
	}
	else if (OP(scan) == EVAL) {
		if (data)
		    data->flags |= SF_HAS_EVAL;
	}
	else if ( PL_regkind[OP(scan)] == ENDLIKE ) {
	    if (flags & SCF_DO_SUBSTR) {
		SCAN_COMMIT(pRExC_state,data,minlenp);
		flags &= ~SCF_DO_SUBSTR;
	    }
	    if (data && OP(scan)==ACCEPT) {
	        data->flags |= SCF_SEEN_ACCEPT;
	        if (stopmin > min)
	            stopmin = min;
	    }
	}
	else if (OP(scan) == LOGICAL && scan->flags == 2) /* Embedded follows */
	{
		if (flags & SCF_DO_SUBSTR) {
		    SCAN_COMMIT(pRExC_state,data,minlenp);
		    data->longest = &(data->longest_float);
		}
		is_inf = is_inf_internal = 1;
		if (flags & SCF_DO_STCLASS_OR) /* Allow everything */
		    cl_anything(pRExC_state, data->start_class);
		flags &= ~SCF_DO_STCLASS;
	}
	else if (OP(scan) == GPOS) {
	    if (!(RExC_rx->extflags & RXf_GPOS_FLOAT) &&
	        !(delta || is_inf || (data && data->pos_delta))) 
	    {
	        if (!(RExC_rx->extflags & RXf_ANCH) && (flags & SCF_DO_SUBSTR))
		    RExC_rx->extflags |= RXf_ANCH_GPOS;
	        if (RExC_rx->gofs < (U32)min)
		    RExC_rx->gofs = min;
            } else {
                RExC_rx->extflags |= RXf_GPOS_FLOAT;
                RExC_rx->gofs = 0;
            }	    
	}
#ifdef TRIE_STUDY_OPT
#ifdef FULL_TRIE_STUDY
        else if (PL_regkind[OP(scan)] == TRIE) {
            /* NOTE - There is similar code to this block above for handling
               BRANCH nodes on the initial study.  If you change stuff here
               check there too. */
            regnode *trie_node= scan;
            regnode *tail= regnext(scan);
            reg_trie_data *trie = (reg_trie_data*)RExC_rxi->data->data[ ARG(scan) ];
            I32 max1 = 0, min1 = I32_MAX;
            struct regnode_charclass_class accum;

            if (flags & SCF_DO_SUBSTR) /* XXXX Add !SUSPEND? */
                SCAN_COMMIT(pRExC_state, data,minlenp); /* Cannot merge strings after this. */
            if (flags & SCF_DO_STCLASS)
                cl_init_zero(pRExC_state, &accum);
                
            if (!trie->jump) {
                min1= trie->minlen;
                max1= trie->maxlen;
            } else {
                const regnode *nextbranch= NULL;
                U32 word;
                
                for ( word=1 ; word <= trie->wordcount ; word++) 
                {
                    I32 deltanext=0, minnext=0, f = 0, fake;
                    struct regnode_charclass_class this_class;
                    
                    data_fake.flags = 0;
                    if (data) {
                        data_fake.whilem_c = data->whilem_c;
                        data_fake.last_closep = data->last_closep;
                    }
                    else
                        data_fake.last_closep = &fake;
		    data_fake.pos_delta = delta;
                    if (flags & SCF_DO_STCLASS) {
                        cl_init(pRExC_state, &this_class);
                        data_fake.start_class = &this_class;
                        f = SCF_DO_STCLASS_AND;
                    }
                    if (flags & SCF_WHILEM_VISITED_POS)
                        f |= SCF_WHILEM_VISITED_POS;
    
                    if (trie->jump[word]) {
                        if (!nextbranch)
                            nextbranch = trie_node + trie->jump[0];
                        scan= trie_node + trie->jump[word];
                        /* We go from the jump point to the branch that follows
                           it. Note this means we need the vestigal unused branches
                           even though they arent otherwise used.
                         */
                        minnext = study_chunk(pRExC_state, &scan, minlenp, 
                            &deltanext, (regnode *)nextbranch, &data_fake, 
                            stopparen, recursed, NULL, f,depth+1);
                    }
                    if (nextbranch && PL_regkind[OP(nextbranch)]==BRANCH)
                        nextbranch= regnext((regnode*)nextbranch);
                    
                    if (min1 > (I32)(minnext + trie->minlen))
                        min1 = minnext + trie->minlen;
                    if (max1 < (I32)(minnext + deltanext + trie->maxlen))
                        max1 = minnext + deltanext + trie->maxlen;
                    if (deltanext == I32_MAX)
                        is_inf = is_inf_internal = 1;
                    
                    if (data_fake.flags & (SF_HAS_PAR|SF_IN_PAR))
                        pars++;
                    if (data_fake.flags & SCF_SEEN_ACCEPT) {
                        if ( stopmin > min + min1) 
	                    stopmin = min + min1;
	                flags &= ~SCF_DO_SUBSTR;
	                if (data)
	                    data->flags |= SCF_SEEN_ACCEPT;
	            }
                    if (data) {
                        if (data_fake.flags & SF_HAS_EVAL)
                            data->flags |= SF_HAS_EVAL;
                        data->whilem_c = data_fake.whilem_c;
                    }
                    if (flags & SCF_DO_STCLASS)
                        cl_or(pRExC_state, &accum, &this_class);
                }
            }
            if (flags & SCF_DO_SUBSTR) {
                data->pos_min += min1;
                data->pos_delta += max1 - min1;
                if (max1 != min1 || is_inf)
                    data->longest = &(data->longest_float);
            }
            min += min1;
            delta += max1 - min1;
            if (flags & SCF_DO_STCLASS_OR) {
                cl_or(pRExC_state, data->start_class, &accum);
                if (min1) {
                    cl_and(data->start_class, and_withp);
                    flags &= ~SCF_DO_STCLASS;
                }
            }
            else if (flags & SCF_DO_STCLASS_AND) {
                if (min1) {
                    cl_and(data->start_class, &accum);
                    flags &= ~SCF_DO_STCLASS;
                }
                else {
                    /* Switch to OR mode: cache the old value of
                     * data->start_class */
		    INIT_AND_WITHP;
                    StructCopy(data->start_class, and_withp,
                               struct regnode_charclass_class);
                    flags &= ~SCF_DO_STCLASS_AND;
                    StructCopy(&accum, data->start_class,
                               struct regnode_charclass_class);
                    flags |= SCF_DO_STCLASS_OR;
                    data->start_class->flags |= ANYOF_EOS;
                }
            }
            scan= tail;
            continue;
        }
#else
	else if (PL_regkind[OP(scan)] == TRIE) {
	    reg_trie_data *trie = (reg_trie_data*)RExC_rxi->data->data[ ARG(scan) ];
	    U8*bang=NULL;
	    
	    min += trie->minlen;
	    delta += (trie->maxlen - trie->minlen);
	    flags &= ~SCF_DO_STCLASS; /* xxx */
            if (flags & SCF_DO_SUBSTR) {
    	        SCAN_COMMIT(pRExC_state,data,minlenp);	/* Cannot expect anything... */
    	        data->pos_min += trie->minlen;
    	        data->pos_delta += (trie->maxlen - trie->minlen);
		if (trie->maxlen != trie->minlen)
		    data->longest = &(data->longest_float);
    	    }
    	    if (trie->jump) /* no more substrings -- for now /grr*/
    	        flags &= ~SCF_DO_SUBSTR; 
	}
#endif /* old or new */
#endif /* TRIE_STUDY_OPT */

	/* Else: zero-length, ignore. */
	scan = regnext(scan);
    }
    if (frame) {
        last = frame->last;
        scan = frame->next;
        stopparen = frame->stop;
        frame = frame->prev;
        goto fake_study_recurse;
    }

  finish:
    assert(!frame);
    DEBUG_STUDYDATA("pre-fin:",data,depth);

    *scanp = scan;
    *deltap = is_inf_internal ? I32_MAX : delta;
    if (flags & SCF_DO_SUBSTR && is_inf)
	data->pos_delta = I32_MAX - data->pos_min;
    if (is_par > (I32)U8_MAX)
	is_par = 0;
    if (is_par && pars==1 && data) {
	data->flags |= SF_IN_PAR;
	data->flags &= ~SF_HAS_PAR;
    }
    else if (pars && data) {
	data->flags |= SF_HAS_PAR;
	data->flags &= ~SF_IN_PAR;
    }
    if (flags & SCF_DO_STCLASS_OR)
	cl_and(data->start_class, and_withp);
    if (flags & SCF_TRIE_RESTUDY)
        data->flags |= 	SCF_TRIE_RESTUDY;
    
    DEBUG_STUDYDATA("post-fin:",data,depth);
    
    return min < stopmin ? min : stopmin;
}

STATIC U32
S_add_data(RExC_state_t *pRExC_state, U32 n, const char *s)
{
    U32 count = RExC_rxi->data ? RExC_rxi->data->count : 0;

    PERL_ARGS_ASSERT_ADD_DATA;

    Renewc(RExC_rxi->data,
	   sizeof(*RExC_rxi->data) + sizeof(void*) * (count + n - 1),
	   char, struct reg_data);
    if(count)
	Renew(RExC_rxi->data->what, count + n, U8);
    else
	Newx(RExC_rxi->data->what, n, U8);
    RExC_rxi->data->count = count + n;
    Copy(s, RExC_rxi->data->what + count, n, U8);
    return count;
}

/*XXX: todo make this not included in a non debugging perl */
#ifndef PERL_IN_XSUB_RE
void
Perl_reginitcolors(pTHX)
{
    dVAR;
    const char * const s = PerlEnv_getenv("PERL_RE_COLORS");
    if (s) {
	char *t = savepv(s);
	int i = 0;
	PL_colors[0] = t;
	while (++i < 6) {
	    t = strchr(t, '\t');
	    if (t) {
		*t = '\0';
		PL_colors[i] = ++t;
	    }
	    else
		PL_colors[i] = t = (char *)"";
	}
    } else {
	int i = 0;
	while (i < 6)
	    PL_colors[i++] = (char *)"";
    }
    PL_colorset = 1;
}
#endif


#ifdef TRIE_STUDY_OPT
#define CHECK_RESTUDY_GOTO                                  \
        if (                                                \
              (data.flags & SCF_TRIE_RESTUDY)               \
              && ! restudied++                              \
        )     goto reStudy
#else
#define CHECK_RESTUDY_GOTO
#endif        

/*
 - pregcomp - compile a regular expression into internal code
 *
 * We can't allocate space until we know how big the compiled form will be,
 * but we can't compile it (and thus know how big it is) until we've got a
 * place to put the code.  So we cheat:  we compile it twice, once with code
 * generation turned off and size counting turned on, and once "for real".
 * This also means that we don't allocate space until we are sure that the
 * thing really will compile successfully, and we never have to move the
 * code and thus invalidate pointers into it.  (Note that it has to be in
 * one piece because free() must be able to free it all.) [NB: not true in perl]
 *
 * Beware that the optimization-preparation code in here knows about some
 * of the structure of the compiled regexp.  [I'll say.]
 */



#ifndef PERL_IN_XSUB_RE
#define RE_ENGINE_PTR &PL_core_reg_engine
#else
extern const struct regexp_engine my_reg_engine;
#define RE_ENGINE_PTR &my_reg_engine
#endif

#ifndef PERL_IN_XSUB_RE 
REGEXP *
Perl_pregcomp(pTHX_ SV * const pattern, const U32 flags)
{
    dVAR;
    HV * const table = GvHV(PL_hintgv);

    PERL_ARGS_ASSERT_PREGCOMP;

    /* Dispatch a request to compile a regexp to correct 
       regexp engine. */
    if (table) {
        SV **ptr= hv_fetchs(table, "regcomp", FALSE);
        GET_RE_DEBUG_FLAGS_DECL;
        if (ptr && SvIOK(*ptr) && SvIV(*ptr)) {
            const regexp_engine *eng=INT2PTR(regexp_engine*,SvIV(*ptr));
            DEBUG_COMPILE_r({
                PerlIO_printf(Perl_debug_log, "Using engine %"UVxf"\n",
                    SvIV(*ptr));
            });            
            return CALLREGCOMP_ENG(eng, pattern, flags);
        } 
    }
    return Perl_re_compile(aTHX_ pattern, flags);
}
#endif

REGEXP *
Perl_re_compile(pTHX_ SV * const pattern, U32 orig_pm_flags)
{
    dVAR;
    REGEXP *rx;
    struct regexp *r;
    register regexp_internal *ri;
    STRLEN plen;
    char* VOL exp;
    char* xend;
    regnode *scan;
    I32 flags;
    I32 minlen = 0;
    U32 pm_flags;

    /* these are all flags - maybe they should be turned
     * into a single int with different bit masks */
    I32 sawlookahead = 0;
    I32 sawplus = 0;
    I32 sawopen = 0;
    bool used_setjump = FALSE;
    regex_charset initial_charset = get_regex_charset(orig_pm_flags);

    U8 jump_ret = 0;
    dJMPENV;
    scan_data_t data;
    RExC_state_t RExC_state;
    RExC_state_t * const pRExC_state = &RExC_state;
#ifdef TRIE_STUDY_OPT    
    int restudied;
    RExC_state_t copyRExC_state;
#endif    
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_RE_COMPILE;

    DEBUG_r(if (!PL_colorset) reginitcolors());

#ifndef PERL_IN_XSUB_RE
    /* Initialize these here instead of as-needed, as is quick and avoids
     * having to test them each time otherwise */
    if (! PL_AboveLatin1) {
	PL_AboveLatin1 = _new_invlist_C_array(AboveLatin1_invlist);
	PL_ASCII = _new_invlist_C_array(ASCII_invlist);
	PL_Latin1 = _new_invlist_C_array(Latin1_invlist);

	PL_L1PosixAlnum = _new_invlist_C_array(L1PosixAlnum_invlist);
	PL_PosixAlnum = _new_invlist_C_array(PosixAlnum_invlist);

	PL_L1PosixAlpha = _new_invlist_C_array(L1PosixAlpha_invlist);
	PL_PosixAlpha = _new_invlist_C_array(PosixAlpha_invlist);

	PL_PosixBlank = _new_invlist_C_array(PosixBlank_invlist);
	PL_XPosixBlank = _new_invlist_C_array(XPosixBlank_invlist);

	PL_L1Cased = _new_invlist_C_array(L1Cased_invlist);

	PL_PosixCntrl = _new_invlist_C_array(PosixCntrl_invlist);
	PL_XPosixCntrl = _new_invlist_C_array(XPosixCntrl_invlist);

	PL_PosixDigit = _new_invlist_C_array(PosixDigit_invlist);

	PL_L1PosixGraph = _new_invlist_C_array(L1PosixGraph_invlist);
	PL_PosixGraph = _new_invlist_C_array(PosixGraph_invlist);

	PL_L1PosixAlnum = _new_invlist_C_array(L1PosixAlnum_invlist);
	PL_PosixAlnum = _new_invlist_C_array(PosixAlnum_invlist);

	PL_L1PosixLower = _new_invlist_C_array(L1PosixLower_invlist);
	PL_PosixLower = _new_invlist_C_array(PosixLower_invlist);

	PL_L1PosixPrint = _new_invlist_C_array(L1PosixPrint_invlist);
	PL_PosixPrint = _new_invlist_C_array(PosixPrint_invlist);

	PL_L1PosixPunct = _new_invlist_C_array(L1PosixPunct_invlist);
	PL_PosixPunct = _new_invlist_C_array(PosixPunct_invlist);

	PL_PerlSpace = _new_invlist_C_array(PerlSpace_invlist);
	PL_XPerlSpace = _new_invlist_C_array(XPerlSpace_invlist);

	PL_PosixSpace = _new_invlist_C_array(PosixSpace_invlist);
	PL_XPosixSpace = _new_invlist_C_array(XPosixSpace_invlist);

	PL_L1PosixUpper = _new_invlist_C_array(L1PosixUpper_invlist);
	PL_PosixUpper = _new_invlist_C_array(PosixUpper_invlist);

	PL_VertSpace = _new_invlist_C_array(VertSpace_invlist);

	PL_PosixWord = _new_invlist_C_array(PosixWord_invlist);
	PL_L1PosixWord = _new_invlist_C_array(L1PosixWord_invlist);

	PL_PosixXDigit = _new_invlist_C_array(PosixXDigit_invlist);
	PL_XPosixXDigit = _new_invlist_C_array(XPosixXDigit_invlist);
    }
#endif

    exp = SvPV(pattern, plen);

    if (plen == 0) { /* ignore the utf8ness if the pattern is 0 length */
	RExC_utf8 = RExC_orig_utf8 = 0;
    }
    else {
	RExC_utf8 = RExC_orig_utf8 = SvUTF8(pattern);
    }
    RExC_uni_semantics = 0;
    RExC_contains_locale = 0;

    /****************** LONG JUMP TARGET HERE***********************/
    /* Longjmp back to here if have to switch in midstream to utf8 */
    if (! RExC_orig_utf8) {
	JMPENV_PUSH(jump_ret);
	used_setjump = TRUE;
    }

    if (jump_ret == 0) {    /* First time through */
	xend = exp + plen;

        DEBUG_COMPILE_r({
            SV *dsv= sv_newmortal();
            RE_PV_QUOTED_DECL(s, RExC_utf8,
                dsv, exp, plen, 60);
            PerlIO_printf(Perl_debug_log, "%sCompiling REx%s %s\n",
                           PL_colors[4],PL_colors[5],s);
        });
    }
    else {  /* longjumped back */
        STRLEN len = plen;

        /* If the cause for the longjmp was other than changing to utf8, pop
         * our own setjmp, and longjmp to the correct handler */
	if (jump_ret != UTF8_LONGJMP) {
	    JMPENV_POP;
	    JMPENV_JUMP(jump_ret);
	}

	GET_RE_DEBUG_FLAGS;

        /* It's possible to write a regexp in ascii that represents Unicode
        codepoints outside of the byte range, such as via \x{100}. If we
        detect such a sequence we have to convert the entire pattern to utf8
        and then recompile, as our sizing calculation will have been based
        on 1 byte == 1 character, but we will need to use utf8 to encode
        at least some part of the pattern, and therefore must convert the whole
        thing.
        -- dmq */
        DEBUG_PARSE_r(PerlIO_printf(Perl_debug_log,
	    "UTF8 mismatch! Converting to utf8 for resizing and compile\n"));
        exp = (char*)Perl_bytes_to_utf8(aTHX_
				        (U8*)SvPV_nomg(pattern, plen),
					&len);
        xend = exp + len;
        RExC_orig_utf8 = RExC_utf8 = 1;
        SAVEFREEPV(exp);
    }

#ifdef TRIE_STUDY_OPT
    restudied = 0;
#endif

    pm_flags = orig_pm_flags;

    if (initial_charset == REGEX_LOCALE_CHARSET) {
	RExC_contains_locale = 1;
    }
    else if (RExC_utf8 && initial_charset == REGEX_DEPENDS_CHARSET) {

	/* Set to use unicode semantics if the pattern is in utf8 and has the
	 * 'depends' charset specified, as it means unicode when utf8  */
	set_regex_charset(&pm_flags, REGEX_UNICODE_CHARSET);
    }

    RExC_precomp = exp;
    RExC_flags = pm_flags;
    RExC_sawback = 0;

    RExC_seen = 0;
    RExC_in_lookbehind = 0;
    RExC_seen_zerolen = *exp == '^' ? -1 : 0;
    RExC_seen_evals = 0;
    RExC_extralen = 0;
    RExC_override_recoding = 0;

    /* First pass: determine size, legality. */
    RExC_parse = exp;
    RExC_start = exp;
    RExC_end = xend;
    RExC_naughty = 0;
    RExC_npar = 1;
    RExC_nestroot = 0;
    RExC_size = 0L;
    RExC_emit = &PL_regdummy;
    RExC_whilem_seen = 0;
    RExC_open_parens = NULL;
    RExC_close_parens = NULL;
    RExC_opend = NULL;
    RExC_paren_names = NULL;
#ifdef DEBUGGING
    RExC_paren_name_list = NULL;
#endif
    RExC_recurse = NULL;
    RExC_recurse_count = 0;

#if 0 /* REGC() is (currently) a NOP at the first pass.
       * Clever compilers notice this and complain. --jhi */
    REGC((U8)REG_MAGIC, (char*)RExC_emit);
#endif
    DEBUG_PARSE_r(
	PerlIO_printf(Perl_debug_log, "Starting first pass (sizing)\n");
        RExC_lastnum=0;
        RExC_lastparse=NULL;
    );
    if (reg(pRExC_state, 0, &flags,1) == NULL) {
	RExC_precomp = NULL;
	return(NULL);
    }

    /* Here, finished first pass.  Get rid of any added setjmp */
    if (used_setjump) {
	JMPENV_POP;
    }

    DEBUG_PARSE_r({
        PerlIO_printf(Perl_debug_log, 
            "Required size %"IVdf" nodes\n"
            "Starting second pass (creation)\n", 
            (IV)RExC_size);
        RExC_lastnum=0; 
        RExC_lastparse=NULL; 
    });

    /* The first pass could have found things that force Unicode semantics */
    if ((RExC_utf8 || RExC_uni_semantics)
	 && get_regex_charset(pm_flags) == REGEX_DEPENDS_CHARSET)
    {
	set_regex_charset(&pm_flags, REGEX_UNICODE_CHARSET);
    }

    /* Small enough for pointer-storage convention?
       If extralen==0, this means that we will not need long jumps. */
    if (RExC_size >= 0x10000L && RExC_extralen)
        RExC_size += RExC_extralen;
    else
	RExC_extralen = 0;
    if (RExC_whilem_seen > 15)
	RExC_whilem_seen = 15;

    /* Allocate space and zero-initialize. Note, the two step process 
       of zeroing when in debug mode, thus anything assigned has to 
       happen after that */
    rx = (REGEXP*) newSV_type(SVt_REGEXP);
    r = (struct regexp*)SvANY(rx);
    Newxc(ri, sizeof(regexp_internal) + (unsigned)RExC_size * sizeof(regnode),
	 char, regexp_internal);
    if ( r == NULL || ri == NULL )
	FAIL("Regexp out of space");
#ifdef DEBUGGING
    /* avoid reading uninitialized memory in DEBUGGING code in study_chunk() */
    Zero(ri, sizeof(regexp_internal) + (unsigned)RExC_size * sizeof(regnode), char);
#else 
    /* bulk initialize base fields with 0. */
    Zero(ri, sizeof(regexp_internal), char);        
#endif

    /* non-zero initialization begins here */
    RXi_SET( r, ri );
    r->engine= RE_ENGINE_PTR;
    r->extflags = pm_flags;
    {
        bool has_p     = ((r->extflags & RXf_PMf_KEEPCOPY) == RXf_PMf_KEEPCOPY);
        bool has_charset = (get_regex_charset(r->extflags) != REGEX_DEPENDS_CHARSET);

        /* The caret is output if there are any defaults: if not all the STD
         * flags are set, or if no character set specifier is needed */
        bool has_default =
                    (((r->extflags & RXf_PMf_STD_PMMOD) != RXf_PMf_STD_PMMOD)
                    || ! has_charset);
	bool has_runon = ((RExC_seen & REG_SEEN_RUN_ON_COMMENT)==REG_SEEN_RUN_ON_COMMENT);
	U16 reganch = (U16)((r->extflags & RXf_PMf_STD_PMMOD)
			    >> RXf_PMf_STD_PMMOD_SHIFT);
	const char *fptr = STD_PAT_MODS;        /*"msix"*/
	char *p;
        /* Allocate for the worst case, which is all the std flags are turned
         * on.  If more precision is desired, we could do a population count of
         * the flags set.  This could be done with a small lookup table, or by
         * shifting, masking and adding, or even, when available, assembly
         * language for a machine-language population count.
         * We never output a minus, as all those are defaults, so are
         * covered by the caret */
	const STRLEN wraplen = plen + has_p + has_runon
            + has_default       /* If needs a caret */

		/* If needs a character set specifier */
	    + ((has_charset) ? MAX_CHARSET_NAME_LENGTH : 0)
            + (sizeof(STD_PAT_MODS) - 1)
            + (sizeof("(?:)") - 1);

        p = sv_grow(MUTABLE_SV(rx), wraplen + 1); /* +1 for the ending NUL */
	SvPOK_on(rx);
	SvFLAGS(rx) |= SvUTF8(pattern);
        *p++='('; *p++='?';

        /* If a default, cover it using the caret */
        if (has_default) {
            *p++= DEFAULT_PAT_MOD;
        }
        if (has_charset) {
	    STRLEN len;
	    const char* const name = get_regex_charset_name(r->extflags, &len);
	    Copy(name, p, len, char);
	    p += len;
        }
        if (has_p)
            *p++ = KEEPCOPY_PAT_MOD; /*'p'*/
        {
            char ch;
            while((ch = *fptr++)) {
                if(reganch & 1)
                    *p++ = ch;
                reganch >>= 1;
            }
        }

        *p++ = ':';
        Copy(RExC_precomp, p, plen, char);
	assert ((RX_WRAPPED(rx) - p) < 16);
	r->pre_prefix = p - RX_WRAPPED(rx);
        p += plen;
        if (has_runon)
            *p++ = '\n';
        *p++ = ')';
        *p = 0;
	SvCUR_set(rx, p - SvPVX_const(rx));
    }

    r->intflags = 0;
    r->nparens = RExC_npar - 1;	/* set early to validate backrefs */
    
    if (RExC_seen & REG_SEEN_RECURSE) {
        Newxz(RExC_open_parens, RExC_npar,regnode *);
        SAVEFREEPV(RExC_open_parens);
        Newxz(RExC_close_parens,RExC_npar,regnode *);
        SAVEFREEPV(RExC_close_parens);
    }

    /* Useful during FAIL. */
#ifdef RE_TRACK_PATTERN_OFFSETS
    Newxz(ri->u.offsets, 2*RExC_size+1, U32); /* MJD 20001228 */
    DEBUG_OFFSETS_r(PerlIO_printf(Perl_debug_log,
                          "%s %"UVuf" bytes for offset annotations.\n",
                          ri->u.offsets ? "Got" : "Couldn't get",
                          (UV)((2*RExC_size+1) * sizeof(U32))));
#endif
    SetProgLen(ri,RExC_size);
    RExC_rx_sv = rx;
    RExC_rx = r;
    RExC_rxi = ri;

    /* Second pass: emit code. */
    RExC_flags = pm_flags;	/* don't let top level (?i) bleed */
    RExC_parse = exp;
    RExC_end = xend;
    RExC_naughty = 0;
    RExC_npar = 1;
    RExC_emit_start = ri->program;
    RExC_emit = ri->program;
    RExC_emit_bound = ri->program + RExC_size + 1;

    /* Store the count of eval-groups for security checks: */
    RExC_rx->seen_evals = RExC_seen_evals;
    REGC((U8)REG_MAGIC, (char*) RExC_emit++);
    if (reg(pRExC_state, 0, &flags,1) == NULL) {
	ReREFCNT_dec(rx);   
	return(NULL);
    }
    /* XXXX To minimize changes to RE engine we always allocate
       3-units-long substrs field. */
    Newx(r->substrs, 1, struct reg_substr_data);
    if (RExC_recurse_count) {
        Newxz(RExC_recurse,RExC_recurse_count,regnode *);
        SAVEFREEPV(RExC_recurse);
    }

reStudy:
    r->minlen = minlen = sawlookahead = sawplus = sawopen = 0;
    Zero(r->substrs, 1, struct reg_substr_data);

#ifdef TRIE_STUDY_OPT
    if (!restudied) {
        StructCopy(&zero_scan_data, &data, scan_data_t);
        copyRExC_state = RExC_state;
    } else {
        U32 seen=RExC_seen;
        DEBUG_OPTIMISE_r(PerlIO_printf(Perl_debug_log,"Restudying\n"));
        
        RExC_state = copyRExC_state;
        if (seen & REG_TOP_LEVEL_BRANCHES) 
            RExC_seen |= REG_TOP_LEVEL_BRANCHES;
        else
            RExC_seen &= ~REG_TOP_LEVEL_BRANCHES;
        if (data.last_found) {
            SvREFCNT_dec(data.longest_fixed);
	    SvREFCNT_dec(data.longest_float);
	    SvREFCNT_dec(data.last_found);
	}
	StructCopy(&zero_scan_data, &data, scan_data_t);
    }
#else
    StructCopy(&zero_scan_data, &data, scan_data_t);
#endif    

    /* Dig out information for optimizations. */
    r->extflags = RExC_flags; /* was pm_op */
    /*dmq: removed as part of de-PMOP: pm->op_pmflags = RExC_flags; */
 
    if (UTF)
	SvUTF8_on(rx);	/* Unicode in it? */
    ri->regstclass = NULL;
    if (RExC_naughty >= 10)	/* Probably an expensive pattern. */
	r->intflags |= PREGf_NAUGHTY;
    scan = ri->program + 1;		/* First BRANCH. */

    /* testing for BRANCH here tells us whether there is "must appear"
       data in the pattern. If there is then we can use it for optimisations */
    if (!(RExC_seen & REG_TOP_LEVEL_BRANCHES)) { /*  Only one top-level choice. */
	I32 fake;
	STRLEN longest_float_length, longest_fixed_length;
	struct regnode_charclass_class ch_class; /* pointed to by data */
	int stclass_flag;
	I32 last_close = 0; /* pointed to by data */
        regnode *first= scan;
        regnode *first_next= regnext(first);
	/*
	 * Skip introductions and multiplicators >= 1
	 * so that we can extract the 'meat' of the pattern that must 
	 * match in the large if() sequence following.
	 * NOTE that EXACT is NOT covered here, as it is normally
	 * picked up by the optimiser separately. 
	 *
	 * This is unfortunate as the optimiser isnt handling lookahead
	 * properly currently.
	 *
	 */
	while ((OP(first) == OPEN && (sawopen = 1)) ||
	       /* An OR of *one* alternative - should not happen now. */
	    (OP(first) == BRANCH && OP(first_next) != BRANCH) ||
	    /* for now we can't handle lookbehind IFMATCH*/
	    (OP(first) == IFMATCH && !first->flags && (sawlookahead = 1)) ||
	    (OP(first) == PLUS) ||
	    (OP(first) == MINMOD) ||
	       /* An {n,m} with n>0 */
	    (PL_regkind[OP(first)] == CURLY && ARG1(first) > 0) ||
	    (OP(first) == NOTHING && PL_regkind[OP(first_next)] != END ))
	{
		/* 
		 * the only op that could be a regnode is PLUS, all the rest
		 * will be regnode_1 or regnode_2.
		 *
		 */
		if (OP(first) == PLUS)
		    sawplus = 1;
		else
		    first += regarglen[OP(first)];

		first = NEXTOPER(first);
		first_next= regnext(first);
	}

	/* Starting-point info. */
      again:
        DEBUG_PEEP("first:",first,0);
        /* Ignore EXACT as we deal with it later. */
	if (PL_regkind[OP(first)] == EXACT) {
	    if (OP(first) == EXACT)
		NOOP;	/* Empty, get anchored substr later. */
	    else
		ri->regstclass = first;
	}
#ifdef TRIE_STCLASS
	else if (PL_regkind[OP(first)] == TRIE &&
	        ((reg_trie_data *)ri->data->data[ ARG(first) ])->minlen>0) 
	{
	    regnode *trie_op;
	    /* this can happen only on restudy */
	    if ( OP(first) == TRIE ) {
                struct regnode_1 *trieop = (struct regnode_1 *)
		    PerlMemShared_calloc(1, sizeof(struct regnode_1));
                StructCopy(first,trieop,struct regnode_1);
                trie_op=(regnode *)trieop;
            } else {
                struct regnode_charclass *trieop = (struct regnode_charclass *)
		    PerlMemShared_calloc(1, sizeof(struct regnode_charclass));
                StructCopy(first,trieop,struct regnode_charclass);
                trie_op=(regnode *)trieop;
            }
            OP(trie_op)+=2;
            make_trie_failtable(pRExC_state, (regnode *)first, trie_op, 0);
	    ri->regstclass = trie_op;
	}
#endif
	else if (REGNODE_SIMPLE(OP(first)))
	    ri->regstclass = first;
	else if (PL_regkind[OP(first)] == BOUND ||
		 PL_regkind[OP(first)] == NBOUND)
	    ri->regstclass = first;
	else if (PL_regkind[OP(first)] == BOL) {
	    r->extflags |= (OP(first) == MBOL
			   ? RXf_ANCH_MBOL
			   : (OP(first) == SBOL
			      ? RXf_ANCH_SBOL
			      : RXf_ANCH_BOL));
	    first = NEXTOPER(first);
	    goto again;
	}
	else if (OP(first) == GPOS) {
	    r->extflags |= RXf_ANCH_GPOS;
	    first = NEXTOPER(first);
	    goto again;
	}
	else if ((!sawopen || !RExC_sawback) &&
	    (OP(first) == STAR &&
	    PL_regkind[OP(NEXTOPER(first))] == REG_ANY) &&
	    !(r->extflags & RXf_ANCH) && !(RExC_seen & REG_SEEN_EVAL))
	{
	    /* turn .* into ^.* with an implied $*=1 */
	    const int type =
		(OP(NEXTOPER(first)) == REG_ANY)
		    ? RXf_ANCH_MBOL
		    : RXf_ANCH_SBOL;
	    r->extflags |= type;
	    r->intflags |= PREGf_IMPLICIT;
	    first = NEXTOPER(first);
	    goto again;
	}
	if (sawplus && !sawlookahead && (!sawopen || !RExC_sawback)
	    && !(RExC_seen & REG_SEEN_EVAL)) /* May examine pos and $& */
	    /* x+ must match at the 1st pos of run of x's */
	    r->intflags |= PREGf_SKIP;

	/* Scan is after the zeroth branch, first is atomic matcher. */
#ifdef TRIE_STUDY_OPT
	DEBUG_PARSE_r(
	    if (!restudied)
	        PerlIO_printf(Perl_debug_log, "first at %"IVdf"\n",
			      (IV)(first - scan + 1))
        );
#else
	DEBUG_PARSE_r(
	    PerlIO_printf(Perl_debug_log, "first at %"IVdf"\n",
	        (IV)(first - scan + 1))
        );
#endif


	/*
	* If there's something expensive in the r.e., find the
	* longest literal string that must appear and make it the
	* regmust.  Resolve ties in favor of later strings, since
	* the regstart check works with the beginning of the r.e.
	* and avoiding duplication strengthens checking.  Not a
	* strong reason, but sufficient in the absence of others.
	* [Now we resolve ties in favor of the earlier string if
	* it happens that c_offset_min has been invalidated, since the
	* earlier string may buy us something the later one won't.]
	*/

	data.longest_fixed = newSVpvs("");
	data.longest_float = newSVpvs("");
	data.last_found = newSVpvs("");
	data.longest = &(data.longest_fixed);
	first = scan;
	if (!ri->regstclass) {
	    cl_init(pRExC_state, &ch_class);
	    data.start_class = &ch_class;
	    stclass_flag = SCF_DO_STCLASS_AND;
	} else				/* XXXX Check for BOUND? */
	    stclass_flag = 0;
	data.last_closep = &last_close;
        
	minlen = study_chunk(pRExC_state, &first, &minlen, &fake, scan + RExC_size, /* Up to end */
            &data, -1, NULL, NULL,
            SCF_DO_SUBSTR | SCF_WHILEM_VISITED_POS | stclass_flag,0);


        CHECK_RESTUDY_GOTO;


	if ( RExC_npar == 1 && data.longest == &(data.longest_fixed)
	     && data.last_start_min == 0 && data.last_end > 0
	     && !RExC_seen_zerolen
	     && !(RExC_seen & REG_SEEN_VERBARG)
	     && (!(RExC_seen & REG_SEEN_GPOS) || (r->extflags & RXf_ANCH_GPOS)))
	    r->extflags |= RXf_CHECK_ALL;
	scan_commit(pRExC_state, &data,&minlen,0);
	SvREFCNT_dec(data.last_found);

        /* Note that code very similar to this but for anchored string 
           follows immediately below, changes may need to be made to both. 
           Be careful. 
         */
	longest_float_length = CHR_SVLEN(data.longest_float);
	if (longest_float_length
	    || (data.flags & SF_FL_BEFORE_EOL
		&& (!(data.flags & SF_FL_BEFORE_MEOL)
		    || (RExC_flags & RXf_PMf_MULTILINE)))) 
        {
            I32 t,ml;

            /* See comments for join_exact for why REG_SEEN_EXACTF_SHARP_S */
	    if ((RExC_seen & REG_SEEN_EXACTF_SHARP_S)
		|| (SvCUR(data.longest_fixed)  /* ok to leave SvCUR */
		    && data.offset_fixed == data.offset_float_min
		    && SvCUR(data.longest_fixed) == SvCUR(data.longest_float)))
		    goto remove_float;		/* As in (a)+. */

            /* copy the information about the longest float from the reg_scan_data
               over to the program. */
	    if (SvUTF8(data.longest_float)) {
		r->float_utf8 = data.longest_float;
		r->float_substr = NULL;
	    } else {
		r->float_substr = data.longest_float;
		r->float_utf8 = NULL;
	    }
	    /* float_end_shift is how many chars that must be matched that 
	       follow this item. We calculate it ahead of time as once the
	       lookbehind offset is added in we lose the ability to correctly
	       calculate it.*/
	    ml = data.minlen_float ? *(data.minlen_float) 
	                           : (I32)longest_float_length;
	    r->float_end_shift = ml - data.offset_float_min
	        - longest_float_length + (SvTAIL(data.longest_float) != 0)
	        + data.lookbehind_float;
	    r->float_min_offset = data.offset_float_min - data.lookbehind_float;
	    r->float_max_offset = data.offset_float_max;
	    if (data.offset_float_max < I32_MAX) /* Don't offset infinity */
	        r->float_max_offset -= data.lookbehind_float;
	    
	    t = (data.flags & SF_FL_BEFORE_EOL /* Can't have SEOL and MULTI */
		       && (!(data.flags & SF_FL_BEFORE_MEOL)
			   || (RExC_flags & RXf_PMf_MULTILINE)));
	    fbm_compile(data.longest_float, t ? FBMcf_TAIL : 0);
	}
	else {
	  remove_float:
	    r->float_substr = r->float_utf8 = NULL;
	    SvREFCNT_dec(data.longest_float);
	    longest_float_length = 0;
	}

        /* Note that code very similar to this but for floating string 
           is immediately above, changes may need to be made to both. 
           Be careful. 
         */
	longest_fixed_length = CHR_SVLEN(data.longest_fixed);

        /* See comments for join_exact for why REG_SEEN_EXACTF_SHARP_S */
	if (! (RExC_seen & REG_SEEN_EXACTF_SHARP_S)
	    && (longest_fixed_length
	        || (data.flags & SF_FIX_BEFORE_EOL /* Cannot have SEOL and MULTI */
		    && (!(data.flags & SF_FIX_BEFORE_MEOL)
		        || (RExC_flags & RXf_PMf_MULTILINE)))) )
        {
            I32 t,ml;

            /* copy the information about the longest fixed 
               from the reg_scan_data over to the program. */
	    if (SvUTF8(data.longest_fixed)) {
		r->anchored_utf8 = data.longest_fixed;
		r->anchored_substr = NULL;
	    } else {
		r->anchored_substr = data.longest_fixed;
		r->anchored_utf8 = NULL;
	    }
	    /* fixed_end_shift is how many chars that must be matched that 
	       follow this item. We calculate it ahead of time as once the
	       lookbehind offset is added in we lose the ability to correctly
	       calculate it.*/
            ml = data.minlen_fixed ? *(data.minlen_fixed) 
                                   : (I32)longest_fixed_length;
            r->anchored_end_shift = ml - data.offset_fixed
	        - longest_fixed_length + (SvTAIL(data.longest_fixed) != 0)
	        + data.lookbehind_fixed;
	    r->anchored_offset = data.offset_fixed - data.lookbehind_fixed;

	    t = (data.flags & SF_FIX_BEFORE_EOL /* Can't have SEOL and MULTI */
		 && (!(data.flags & SF_FIX_BEFORE_MEOL)
		     || (RExC_flags & RXf_PMf_MULTILINE)));
	    fbm_compile(data.longest_fixed, t ? FBMcf_TAIL : 0);
	}
	else {
	    r->anchored_substr = r->anchored_utf8 = NULL;
	    SvREFCNT_dec(data.longest_fixed);
	    longest_fixed_length = 0;
	}
	if (ri->regstclass
	    && (OP(ri->regstclass) == REG_ANY || OP(ri->regstclass) == SANY))
	    ri->regstclass = NULL;

	if ((!(r->anchored_substr || r->anchored_utf8) || r->anchored_offset)
	    && stclass_flag
	    && !(data.start_class->flags & ANYOF_EOS)
	    && !cl_is_anything(data.start_class))
	{
	    const U32 n = add_data(pRExC_state, 1, "f");
	    data.start_class->flags |= ANYOF_IS_SYNTHETIC;

	    Newx(RExC_rxi->data->data[n], 1,
		struct regnode_charclass_class);
	    StructCopy(data.start_class,
		       (struct regnode_charclass_class*)RExC_rxi->data->data[n],
		       struct regnode_charclass_class);
	    ri->regstclass = (regnode*)RExC_rxi->data->data[n];
	    r->intflags &= ~PREGf_SKIP;	/* Used in find_byclass(). */
	    DEBUG_COMPILE_r({ SV *sv = sv_newmortal();
	              regprop(r, sv, (regnode*)data.start_class);
		      PerlIO_printf(Perl_debug_log,
				    "synthetic stclass \"%s\".\n",
				    SvPVX_const(sv));});
	}

	/* A temporary algorithm prefers floated substr to fixed one to dig more info. */
	if (longest_fixed_length > longest_float_length) {
	    r->check_end_shift = r->anchored_end_shift;
	    r->check_substr = r->anchored_substr;
	    r->check_utf8 = r->anchored_utf8;
	    r->check_offset_min = r->check_offset_max = r->anchored_offset;
	    if (r->extflags & RXf_ANCH_SINGLE)
		r->extflags |= RXf_NOSCAN;
	}
	else {
	    r->check_end_shift = r->float_end_shift;
	    r->check_substr = r->float_substr;
	    r->check_utf8 = r->float_utf8;
	    r->check_offset_min = r->float_min_offset;
	    r->check_offset_max = r->float_max_offset;
	}
	/* XXXX Currently intuiting is not compatible with ANCH_GPOS.
	   This should be changed ASAP!  */
	if ((r->check_substr || r->check_utf8) && !(r->extflags & RXf_ANCH_GPOS)) {
	    r->extflags |= RXf_USE_INTUIT;
	    if (SvTAIL(r->check_substr ? r->check_substr : r->check_utf8))
		r->extflags |= RXf_INTUIT_TAIL;
	}
	/* XXX Unneeded? dmq (shouldn't as this is handled elsewhere)
	if ( (STRLEN)minlen < longest_float_length )
            minlen= longest_float_length;
        if ( (STRLEN)minlen < longest_fixed_length )
            minlen= longest_fixed_length;     
        */
    }
    else {
	/* Several toplevels. Best we can is to set minlen. */
	I32 fake;
	struct regnode_charclass_class ch_class;
	I32 last_close = 0;

	DEBUG_PARSE_r(PerlIO_printf(Perl_debug_log, "\nMulti Top Level\n"));

	scan = ri->program + 1;
	cl_init(pRExC_state, &ch_class);
	data.start_class = &ch_class;
	data.last_closep = &last_close;

        
	minlen = study_chunk(pRExC_state, &scan, &minlen, &fake, scan + RExC_size,
	    &data, -1, NULL, NULL, SCF_DO_STCLASS_AND|SCF_WHILEM_VISITED_POS,0);
        
        CHECK_RESTUDY_GOTO;

	r->check_substr = r->check_utf8 = r->anchored_substr = r->anchored_utf8
		= r->float_substr = r->float_utf8 = NULL;

	if (!(data.start_class->flags & ANYOF_EOS)
	    && !cl_is_anything(data.start_class))
	{
	    const U32 n = add_data(pRExC_state, 1, "f");
	    data.start_class->flags |= ANYOF_IS_SYNTHETIC;

	    Newx(RExC_rxi->data->data[n], 1,
		struct regnode_charclass_class);
	    StructCopy(data.start_class,
		       (struct regnode_charclass_class*)RExC_rxi->data->data[n],
		       struct regnode_charclass_class);
	    ri->regstclass = (regnode*)RExC_rxi->data->data[n];
	    r->intflags &= ~PREGf_SKIP;	/* Used in find_byclass(). */
	    DEBUG_COMPILE_r({ SV* sv = sv_newmortal();
	              regprop(r, sv, (regnode*)data.start_class);
		      PerlIO_printf(Perl_debug_log,
				    "synthetic stclass \"%s\".\n",
				    SvPVX_const(sv));});
	}
    }

    /* Guard against an embedded (?=) or (?<=) with a longer minlen than
       the "real" pattern. */
    DEBUG_OPTIMISE_r({
	PerlIO_printf(Perl_debug_log,"minlen: %"IVdf" r->minlen:%"IVdf"\n",
		      (IV)minlen, (IV)r->minlen);
    });
    r->minlenret = minlen;
    if (r->minlen < minlen) 
        r->minlen = minlen;
    
    if (RExC_seen & REG_SEEN_GPOS)
	r->extflags |= RXf_GPOS_SEEN;
    if (RExC_seen & REG_SEEN_LOOKBEHIND)
	r->extflags |= RXf_LOOKBEHIND_SEEN;
    if (RExC_seen & REG_SEEN_EVAL)
	r->extflags |= RXf_EVAL_SEEN;
    if (RExC_seen & REG_SEEN_CANY)
	r->extflags |= RXf_CANY_SEEN;
    if (RExC_seen & REG_SEEN_VERBARG)
	r->intflags |= PREGf_VERBARG_SEEN;
    if (RExC_seen & REG_SEEN_CUTGROUP)
	r->intflags |= PREGf_CUTGROUP_SEEN;
    if (RExC_paren_names)
        RXp_PAREN_NAMES(r) = MUTABLE_HV(SvREFCNT_inc(RExC_paren_names));
    else
        RXp_PAREN_NAMES(r) = NULL;

#ifdef STUPID_PATTERN_CHECKS            
    if (RX_PRELEN(rx) == 0)
        r->extflags |= RXf_NULL;
    if (r->extflags & RXf_SPLIT && RX_PRELEN(rx) == 1 && RX_PRECOMP(rx)[0] == ' ')
        /* XXX: this should happen BEFORE we compile */
        r->extflags |= (RXf_SKIPWHITE|RXf_WHITE); 
    else if (RX_PRELEN(rx) == 3 && memEQ("\\s+", RX_PRECOMP(rx), 3))
        r->extflags |= RXf_WHITE;
    else if (RX_PRELEN(rx) == 1 && RXp_PRECOMP(rx)[0] == '^')
        r->extflags |= RXf_START_ONLY;
#else
    if (r->extflags & RXf_SPLIT && RX_PRELEN(rx) == 1 && RX_PRECOMP(rx)[0] == ' ')
            /* XXX: this should happen BEFORE we compile */
            r->extflags |= (RXf_SKIPWHITE|RXf_WHITE); 
    else {
        regnode *first = ri->program + 1;
        U8 fop = OP(first);

        if (PL_regkind[fop] == NOTHING && OP(NEXTOPER(first)) == END)
            r->extflags |= RXf_NULL;
        else if (PL_regkind[fop] == BOL && OP(NEXTOPER(first)) == END)
            r->extflags |= RXf_START_ONLY;
        else if (fop == PLUS && OP(NEXTOPER(first)) == SPACE
			     && OP(regnext(first)) == END)
            r->extflags |= RXf_WHITE;    
    }
#endif
#ifdef DEBUGGING
    if (RExC_paren_names) {
        ri->name_list_idx = add_data( pRExC_state, 1, "a" );
        ri->data->data[ri->name_list_idx] = (void*)SvREFCNT_inc(RExC_paren_name_list);
    } else
#endif
        ri->name_list_idx = 0;

    if (RExC_recurse_count) {
        for ( ; RExC_recurse_count ; RExC_recurse_count-- ) {
            const regnode *scan = RExC_recurse[RExC_recurse_count-1];
            ARG2L_SET( scan, RExC_open_parens[ARG(scan)-1] - scan );
        }
    }
    Newxz(r->offs, RExC_npar, regexp_paren_pair);
    /* assume we don't need to swap parens around before we match */

    DEBUG_DUMP_r({
        PerlIO_printf(Perl_debug_log,"Final program:\n");
        regdump(r);
    });
#ifdef RE_TRACK_PATTERN_OFFSETS
    DEBUG_OFFSETS_r(if (ri->u.offsets) {
        const U32 len = ri->u.offsets[0];
        U32 i;
        GET_RE_DEBUG_FLAGS_DECL;
        PerlIO_printf(Perl_debug_log, "Offsets: [%"UVuf"]\n\t", (UV)ri->u.offsets[0]);
        for (i = 1; i <= len; i++) {
            if (ri->u.offsets[i*2-1] || ri->u.offsets[i*2])
                PerlIO_printf(Perl_debug_log, "%"UVuf":%"UVuf"[%"UVuf"] ",
                (UV)i, (UV)ri->u.offsets[i*2-1], (UV)ri->u.offsets[i*2]);
            }
        PerlIO_printf(Perl_debug_log, "\n");
    });
#endif
    return rx;
}

#undef RE_ENGINE_PTR


SV*
Perl_reg_named_buff(pTHX_ REGEXP * const rx, SV * const key, SV * const value,
                    const U32 flags)
{
    PERL_ARGS_ASSERT_REG_NAMED_BUFF;

    PERL_UNUSED_ARG(value);

    if (flags & RXapif_FETCH) {
        return reg_named_buff_fetch(rx, key, flags);
    } else if (flags & (RXapif_STORE | RXapif_DELETE | RXapif_CLEAR)) {
        Perl_croak_no_modify(aTHX);
        return NULL;
    } else if (flags & RXapif_EXISTS) {
        return reg_named_buff_exists(rx, key, flags)
            ? &PL_sv_yes
            : &PL_sv_no;
    } else if (flags & RXapif_REGNAMES) {
        return reg_named_buff_all(rx, flags);
    } else if (flags & (RXapif_SCALAR | RXapif_REGNAMES_COUNT)) {
        return reg_named_buff_scalar(rx, flags);
    } else {
        Perl_croak(aTHX_ "panic: Unknown flags %d in named_buff", (int)flags);
        return NULL;
    }
}

SV*
Perl_reg_named_buff_iter(pTHX_ REGEXP * const rx, const SV * const lastkey,
                         const U32 flags)
{
    PERL_ARGS_ASSERT_REG_NAMED_BUFF_ITER;
    PERL_UNUSED_ARG(lastkey);

    if (flags & RXapif_FIRSTKEY)
        return reg_named_buff_firstkey(rx, flags);
    else if (flags & RXapif_NEXTKEY)
        return reg_named_buff_nextkey(rx, flags);
    else {
        Perl_croak(aTHX_ "panic: Unknown flags %d in named_buff_iter", (int)flags);
        return NULL;
    }
}

SV*
Perl_reg_named_buff_fetch(pTHX_ REGEXP * const r, SV * const namesv,
			  const U32 flags)
{
    AV *retarray = NULL;
    SV *ret;
    struct regexp *const rx = (struct regexp *)SvANY(r);

    PERL_ARGS_ASSERT_REG_NAMED_BUFF_FETCH;

    if (flags & RXapif_ALL)
        retarray=newAV();

    if (rx && RXp_PAREN_NAMES(rx)) {
        HE *he_str = hv_fetch_ent( RXp_PAREN_NAMES(rx), namesv, 0, 0 );
        if (he_str) {
            IV i;
            SV* sv_dat=HeVAL(he_str);
            I32 *nums=(I32*)SvPVX(sv_dat);
            for ( i=0; i<SvIVX(sv_dat); i++ ) {
                if ((I32)(rx->nparens) >= nums[i]
                    && rx->offs[nums[i]].start != -1
                    && rx->offs[nums[i]].end != -1)
                {
                    ret = newSVpvs("");
                    CALLREG_NUMBUF_FETCH(r,nums[i],ret);
                    if (!retarray)
                        return ret;
                } else {
                    if (retarray)
                        ret = newSVsv(&PL_sv_undef);
                }
                if (retarray)
                    av_push(retarray, ret);
            }
            if (retarray)
                return newRV_noinc(MUTABLE_SV(retarray));
        }
    }
    return NULL;
}

bool
Perl_reg_named_buff_exists(pTHX_ REGEXP * const r, SV * const key,
                           const U32 flags)
{
    struct regexp *const rx = (struct regexp *)SvANY(r);

    PERL_ARGS_ASSERT_REG_NAMED_BUFF_EXISTS;

    if (rx && RXp_PAREN_NAMES(rx)) {
        if (flags & RXapif_ALL) {
            return hv_exists_ent(RXp_PAREN_NAMES(rx), key, 0);
        } else {
	    SV *sv = CALLREG_NAMED_BUFF_FETCH(r, key, flags);
            if (sv) {
		SvREFCNT_dec(sv);
                return TRUE;
            } else {
                return FALSE;
            }
        }
    } else {
        return FALSE;
    }
}

SV*
Perl_reg_named_buff_firstkey(pTHX_ REGEXP * const r, const U32 flags)
{
    struct regexp *const rx = (struct regexp *)SvANY(r);

    PERL_ARGS_ASSERT_REG_NAMED_BUFF_FIRSTKEY;

    if ( rx && RXp_PAREN_NAMES(rx) ) {
	(void)hv_iterinit(RXp_PAREN_NAMES(rx));

	return CALLREG_NAMED_BUFF_NEXTKEY(r, NULL, flags & ~RXapif_FIRSTKEY);
    } else {
	return FALSE;
    }
}

SV*
Perl_reg_named_buff_nextkey(pTHX_ REGEXP * const r, const U32 flags)
{
    struct regexp *const rx = (struct regexp *)SvANY(r);
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REG_NAMED_BUFF_NEXTKEY;

    if (rx && RXp_PAREN_NAMES(rx)) {
        HV *hv = RXp_PAREN_NAMES(rx);
        HE *temphe;
        while ( (temphe = hv_iternext_flags(hv,0)) ) {
            IV i;
            IV parno = 0;
            SV* sv_dat = HeVAL(temphe);
            I32 *nums = (I32*)SvPVX(sv_dat);
            for ( i = 0; i < SvIVX(sv_dat); i++ ) {
                if ((I32)(rx->lastparen) >= nums[i] &&
                    rx->offs[nums[i]].start != -1 &&
                    rx->offs[nums[i]].end != -1)
                {
                    parno = nums[i];
                    break;
                }
            }
            if (parno || flags & RXapif_ALL) {
		return newSVhek(HeKEY_hek(temphe));
            }
        }
    }
    return NULL;
}

SV*
Perl_reg_named_buff_scalar(pTHX_ REGEXP * const r, const U32 flags)
{
    SV *ret;
    AV *av;
    I32 length;
    struct regexp *const rx = (struct regexp *)SvANY(r);

    PERL_ARGS_ASSERT_REG_NAMED_BUFF_SCALAR;

    if (rx && RXp_PAREN_NAMES(rx)) {
        if (flags & (RXapif_ALL | RXapif_REGNAMES_COUNT)) {
            return newSViv(HvTOTALKEYS(RXp_PAREN_NAMES(rx)));
        } else if (flags & RXapif_ONE) {
            ret = CALLREG_NAMED_BUFF_ALL(r, (flags | RXapif_REGNAMES));
            av = MUTABLE_AV(SvRV(ret));
            length = av_len(av);
	    SvREFCNT_dec(ret);
            return newSViv(length + 1);
        } else {
            Perl_croak(aTHX_ "panic: Unknown flags %d in named_buff_scalar", (int)flags);
            return NULL;
        }
    }
    return &PL_sv_undef;
}

SV*
Perl_reg_named_buff_all(pTHX_ REGEXP * const r, const U32 flags)
{
    struct regexp *const rx = (struct regexp *)SvANY(r);
    AV *av = newAV();

    PERL_ARGS_ASSERT_REG_NAMED_BUFF_ALL;

    if (rx && RXp_PAREN_NAMES(rx)) {
        HV *hv= RXp_PAREN_NAMES(rx);
        HE *temphe;
        (void)hv_iterinit(hv);
        while ( (temphe = hv_iternext_flags(hv,0)) ) {
            IV i;
            IV parno = 0;
            SV* sv_dat = HeVAL(temphe);
            I32 *nums = (I32*)SvPVX(sv_dat);
            for ( i = 0; i < SvIVX(sv_dat); i++ ) {
                if ((I32)(rx->lastparen) >= nums[i] &&
                    rx->offs[nums[i]].start != -1 &&
                    rx->offs[nums[i]].end != -1)
                {
                    parno = nums[i];
                    break;
                }
            }
            if (parno || flags & RXapif_ALL) {
                av_push(av, newSVhek(HeKEY_hek(temphe)));
            }
        }
    }

    return newRV_noinc(MUTABLE_SV(av));
}

void
Perl_reg_numbered_buff_fetch(pTHX_ REGEXP * const r, const I32 paren,
			     SV * const sv)
{
    struct regexp *const rx = (struct regexp *)SvANY(r);
    char *s = NULL;
    I32 i = 0;
    I32 s1, t1;

    PERL_ARGS_ASSERT_REG_NUMBERED_BUFF_FETCH;
        
    if (!rx->subbeg) {
        sv_setsv(sv,&PL_sv_undef);
        return;
    } 
    else               
    if (paren == RX_BUFF_IDX_PREMATCH && rx->offs[0].start != -1) {
        /* $` */
	i = rx->offs[0].start;
	s = rx->subbeg;
    }
    else 
    if (paren == RX_BUFF_IDX_POSTMATCH && rx->offs[0].end != -1) {
        /* $' */
	s = rx->subbeg + rx->offs[0].end;
	i = rx->sublen - rx->offs[0].end;
    } 
    else
    if ( 0 <= paren && paren <= (I32)rx->nparens &&
        (s1 = rx->offs[paren].start) != -1 &&
        (t1 = rx->offs[paren].end) != -1)
    {
        /* $& $1 ... */
        i = t1 - s1;
        s = rx->subbeg + s1;
    } else {
        sv_setsv(sv,&PL_sv_undef);
        return;
    }          
    assert(rx->sublen >= (s - rx->subbeg) + i );
    if (i >= 0) {
        const int oldtainted = PL_tainted;
        TAINT_NOT;
        sv_setpvn(sv, s, i);
        PL_tainted = oldtainted;
        if ( (rx->extflags & RXf_CANY_SEEN)
            ? (RXp_MATCH_UTF8(rx)
                        && (!i || is_utf8_string((U8*)s, i)))
            : (RXp_MATCH_UTF8(rx)) )
        {
            SvUTF8_on(sv);
        }
        else
            SvUTF8_off(sv);
        if (PL_tainting) {
            if (RXp_MATCH_TAINTED(rx)) {
                if (SvTYPE(sv) >= SVt_PVMG) {
                    MAGIC* const mg = SvMAGIC(sv);
                    MAGIC* mgt;
                    PL_tainted = 1;
                    SvMAGIC_set(sv, mg->mg_moremagic);
                    SvTAINT(sv);
                    if ((mgt = SvMAGIC(sv))) {
                        mg->mg_moremagic = mgt;
                        SvMAGIC_set(sv, mg);
                    }
                } else {
                    PL_tainted = 1;
                    SvTAINT(sv);
                }
            } else 
                SvTAINTED_off(sv);
        }
    } else {
        sv_setsv(sv,&PL_sv_undef);
        return;
    }
}

void
Perl_reg_numbered_buff_store(pTHX_ REGEXP * const rx, const I32 paren,
							 SV const * const value)
{
    PERL_ARGS_ASSERT_REG_NUMBERED_BUFF_STORE;

    PERL_UNUSED_ARG(rx);
    PERL_UNUSED_ARG(paren);
    PERL_UNUSED_ARG(value);

    if (!PL_localizing)
        Perl_croak_no_modify(aTHX);
}

I32
Perl_reg_numbered_buff_length(pTHX_ REGEXP * const r, const SV * const sv,
                              const I32 paren)
{
    struct regexp *const rx = (struct regexp *)SvANY(r);
    I32 i;
    I32 s1, t1;

    PERL_ARGS_ASSERT_REG_NUMBERED_BUFF_LENGTH;

    /* Some of this code was originally in C<Perl_magic_len> in F<mg.c> */
	switch (paren) {
      /* $` / ${^PREMATCH} */
      case RX_BUFF_IDX_PREMATCH:
        if (rx->offs[0].start != -1) {
			i = rx->offs[0].start;
			if (i > 0) {
				s1 = 0;
				t1 = i;
				goto getlen;
			}
	    }
        return 0;
      /* $' / ${^POSTMATCH} */
      case RX_BUFF_IDX_POSTMATCH:
	    if (rx->offs[0].end != -1) {
			i = rx->sublen - rx->offs[0].end;
			if (i > 0) {
				s1 = rx->offs[0].end;
				t1 = rx->sublen;
				goto getlen;
			}
	    }
        return 0;
      /* $& / ${^MATCH}, $1, $2, ... */
      default:
	    if (paren <= (I32)rx->nparens &&
            (s1 = rx->offs[paren].start) != -1 &&
            (t1 = rx->offs[paren].end) != -1)
	    {
            i = t1 - s1;
            goto getlen;
        } else {
            if (ckWARN(WARN_UNINITIALIZED))
                report_uninit((const SV *)sv);
            return 0;
        }
    }
  getlen:
    if (i > 0 && RXp_MATCH_UTF8(rx)) {
        const char * const s = rx->subbeg + s1;
        const U8 *ep;
        STRLEN el;

        i = t1 - s1;
        if (is_utf8_string_loclen((U8*)s, i, &ep, &el))
			i = el;
    }
    return i;
}

SV*
Perl_reg_qr_package(pTHX_ REGEXP * const rx)
{
    PERL_ARGS_ASSERT_REG_QR_PACKAGE;
	PERL_UNUSED_ARG(rx);
	if (0)
	    return NULL;
	else
	    return newSVpvs("Regexp");
}

/* Scans the name of a named buffer from the pattern.
 * If flags is REG_RSN_RETURN_NULL returns null.
 * If flags is REG_RSN_RETURN_NAME returns an SV* containing the name
 * If flags is REG_RSN_RETURN_DATA returns the data SV* corresponding
 * to the parsed name as looked up in the RExC_paren_names hash.
 * If there is an error throws a vFAIL().. type exception.
 */

#define REG_RSN_RETURN_NULL    0
#define REG_RSN_RETURN_NAME    1
#define REG_RSN_RETURN_DATA    2

STATIC SV*
S_reg_scan_name(pTHX_ RExC_state_t *pRExC_state, U32 flags)
{
    char *name_start = RExC_parse;

    PERL_ARGS_ASSERT_REG_SCAN_NAME;

    if (isIDFIRST_lazy_if(RExC_parse, UTF)) {
	 /* skip IDFIRST by using do...while */
	if (UTF)
	    do {
		RExC_parse += UTF8SKIP(RExC_parse);
	    } while (isALNUM_utf8((U8*)RExC_parse));
	else
	    do {
		RExC_parse++;
	    } while (isALNUM(*RExC_parse));
    }

    if ( flags ) {
        SV* sv_name
	    = newSVpvn_flags(name_start, (int)(RExC_parse - name_start),
			     SVs_TEMP | (UTF ? SVf_UTF8 : 0));
        if ( flags == REG_RSN_RETURN_NAME)
            return sv_name;
        else if (flags==REG_RSN_RETURN_DATA) {
            HE *he_str = NULL;
            SV *sv_dat = NULL;
            if ( ! sv_name )      /* should not happen*/
                Perl_croak(aTHX_ "panic: no svname in reg_scan_name");
            if (RExC_paren_names)
                he_str = hv_fetch_ent( RExC_paren_names, sv_name, 0, 0 );
            if ( he_str )
                sv_dat = HeVAL(he_str);
            if ( ! sv_dat )
                vFAIL("Reference to nonexistent named group");
            return sv_dat;
        }
        else {
            Perl_croak(aTHX_ "panic: bad flag %lx in reg_scan_name",
		       (unsigned long) flags);
        }
        /* NOT REACHED */
    }
    return NULL;
}

#define DEBUG_PARSE_MSG(funcname)     DEBUG_PARSE_r({           \
    int rem=(int)(RExC_end - RExC_parse);                       \
    int cut;                                                    \
    int num;                                                    \
    int iscut=0;                                                \
    if (rem>10) {                                               \
        rem=10;                                                 \
        iscut=1;                                                \
    }                                                           \
    cut=10-rem;                                                 \
    if (RExC_lastparse!=RExC_parse)                             \
        PerlIO_printf(Perl_debug_log," >%.*s%-*s",              \
            rem, RExC_parse,                                    \
            cut + 4,                                            \
            iscut ? "..." : "<"                                 \
        );                                                      \
    else                                                        \
        PerlIO_printf(Perl_debug_log,"%16s","");                \
                                                                \
    if (SIZE_ONLY)                                              \
       num = RExC_size + 1;                                     \
    else                                                        \
       num=REG_NODE_NUM(RExC_emit);                             \
    if (RExC_lastnum!=num)                                      \
       PerlIO_printf(Perl_debug_log,"|%4d",num);                \
    else                                                        \
       PerlIO_printf(Perl_debug_log,"|%4s","");                 \
    PerlIO_printf(Perl_debug_log,"|%*s%-4s",                    \
        (int)((depth*2)), "",                                   \
        (funcname)                                              \
    );                                                          \
    RExC_lastnum=num;                                           \
    RExC_lastparse=RExC_parse;                                  \
})



#define DEBUG_PARSE(funcname)     DEBUG_PARSE_r({           \
    DEBUG_PARSE_MSG((funcname));                            \
    PerlIO_printf(Perl_debug_log,"%4s","\n");               \
})
#define DEBUG_PARSE_FMT(funcname,fmt,args)     DEBUG_PARSE_r({           \
    DEBUG_PARSE_MSG((funcname));                            \
    PerlIO_printf(Perl_debug_log,fmt "\n",args);               \
})

/* This section of code defines the inversion list object and its methods.  The
 * interfaces are highly subject to change, so as much as possible is static to
 * this file.  An inversion list is here implemented as a malloc'd C UV array
 * with some added info that is placed as UVs at the beginning in a header
 * portion.  An inversion list for Unicode is an array of code points, sorted
 * by ordinal number.  The zeroth element is the first code point in the list.
 * The 1th element is the first element beyond that not in the list.  In other
 * words, the first range is
 *  invlist[0]..(invlist[1]-1)
 * The other ranges follow.  Thus every element whose index is divisible by two
 * marks the beginning of a range that is in the list, and every element not
 * divisible by two marks the beginning of a range not in the list.  A single
 * element inversion list that contains the single code point N generally
 * consists of two elements
 *  invlist[0] == N
 *  invlist[1] == N+1
 * (The exception is when N is the highest representable value on the
 * machine, in which case the list containing just it would be a single
 * element, itself.  By extension, if the last range in the list extends to
 * infinity, then the first element of that range will be in the inversion list
 * at a position that is divisible by two, and is the final element in the
 * list.)
 * Taking the complement (inverting) an inversion list is quite simple, if the
 * first element is 0, remove it; otherwise add a 0 element at the beginning.
 * This implementation reserves an element at the beginning of each inversion list
 * to contain 0 when the list contains 0, and contains 1 otherwise.  The actual
 * beginning of the list is either that element if 0, or the next one if 1.
 *
 * More about inversion lists can be found in "Unicode Demystified"
 * Chapter 13 by Richard Gillam, published by Addison-Wesley.
 * More will be coming when functionality is added later.
 *
 * The inversion list data structure is currently implemented as an SV pointing
 * to an array of UVs that the SV thinks are bytes.  This allows us to have an
 * array of UV whose memory management is automatically handled by the existing
 * facilities for SV's.
 *
 * Some of the methods should always be private to the implementation, and some
 * should eventually be made public */

#define INVLIST_LEN_OFFSET 0	/* Number of elements in the inversion list */
#define INVLIST_ITER_OFFSET 1	/* Current iteration position */

/* This is a combination of a version and data structure type, so that one
 * being passed in can be validated to be an inversion list of the correct
 * vintage.  When the structure of the header is changed, a new random number
 * in the range 2**31-1 should be generated and the new() method changed to
 * insert that at this location.  Then, if an auxiliary program doesn't change
 * correspondingly, it will be discovered immediately */
#define INVLIST_VERSION_ID_OFFSET 2
#define INVLIST_VERSION_ID 1064334010

/* For safety, when adding new elements, remember to #undef them at the end of
 * the inversion list code section */

#define INVLIST_ZERO_OFFSET 3	/* 0 or 1; must be last element in header */
/* The UV at position ZERO contains either 0 or 1.  If 0, the inversion list
 * contains the code point U+00000, and begins here.  If 1, the inversion list
 * doesn't contain U+0000, and it begins at the next UV in the array.
 * Inverting an inversion list consists of adding or removing the 0 at the
 * beginning of it.  By reserving a space for that 0, inversion can be made
 * very fast */

#define HEADER_LENGTH (INVLIST_ZERO_OFFSET + 1)

/* Internally things are UVs */
#define TO_INTERNAL_SIZE(x) ((x + HEADER_LENGTH) * sizeof(UV))
#define FROM_INTERNAL_SIZE(x) ((x / sizeof(UV)) - HEADER_LENGTH)

#define INVLIST_INITIAL_LEN 10

PERL_STATIC_INLINE UV*
S__invlist_array_init(pTHX_ SV* const invlist, const bool will_have_0)
{
    /* Returns a pointer to the first element in the inversion list's array.
     * This is called upon initialization of an inversion list.  Where the
     * array begins depends on whether the list has the code point U+0000
     * in it or not.  The other parameter tells it whether the code that
     * follows this call is about to put a 0 in the inversion list or not.
     * The first element is either the element with 0, if 0, or the next one,
     * if 1 */

    UV* zero = get_invlist_zero_addr(invlist);

    PERL_ARGS_ASSERT__INVLIST_ARRAY_INIT;

    /* Must be empty */
    assert(! *get_invlist_len_addr(invlist));

    /* 1^1 = 0; 1^0 = 1 */
    *zero = 1 ^ will_have_0;
    return zero + *zero;
}

PERL_STATIC_INLINE UV*
S_invlist_array(pTHX_ SV* const invlist)
{
    /* Returns the pointer to the inversion list's array.  Every time the
     * length changes, this needs to be called in case malloc or realloc moved
     * it */

    PERL_ARGS_ASSERT_INVLIST_ARRAY;

    /* Must not be empty.  If these fail, you probably didn't check for <len>
     * being non-zero before trying to get the array */
    assert(*get_invlist_len_addr(invlist));
    assert(*get_invlist_zero_addr(invlist) == 0
	   || *get_invlist_zero_addr(invlist) == 1);

    /* The array begins either at the element reserved for zero if the
     * list contains 0 (that element will be set to 0), or otherwise the next
     * element (in which case the reserved element will be set to 1). */
    return (UV *) (get_invlist_zero_addr(invlist)
		   + *get_invlist_zero_addr(invlist));
}

PERL_STATIC_INLINE UV*
S_get_invlist_len_addr(pTHX_ SV* invlist)
{
    /* Return the address of the UV that contains the current number
     * of used elements in the inversion list */

    PERL_ARGS_ASSERT_GET_INVLIST_LEN_ADDR;

    return (UV *) (SvPVX(invlist) + (INVLIST_LEN_OFFSET * sizeof (UV)));
}

PERL_STATIC_INLINE UV
S_invlist_len(pTHX_ SV* const invlist)
{
    /* Returns the current number of elements stored in the inversion list's
     * array */

    PERL_ARGS_ASSERT_INVLIST_LEN;

    return *get_invlist_len_addr(invlist);
}

PERL_STATIC_INLINE void
S_invlist_set_len(pTHX_ SV* const invlist, const UV len)
{
    /* Sets the current number of elements stored in the inversion list */

    PERL_ARGS_ASSERT_INVLIST_SET_LEN;

    *get_invlist_len_addr(invlist) = len;

    assert(len <= SvLEN(invlist));

    SvCUR_set(invlist, TO_INTERNAL_SIZE(len));
    /* If the list contains U+0000, that element is part of the header,
     * and should not be counted as part of the array.  It will contain
     * 0 in that case, and 1 otherwise.  So we could flop 0=>1, 1=>0 and
     * subtract:
     *	SvCUR_set(invlist,
     *		  TO_INTERNAL_SIZE(len
     *				   - (*get_invlist_zero_addr(inv_list) ^ 1)));
     * But, this is only valid if len is not 0.  The consequences of not doing
     * this is that the memory allocation code may think that 1 more UV is
     * being used than actually is, and so might do an unnecessary grow.  That
     * seems worth not bothering to make this the precise amount.
     *
     * Note that when inverting, SvCUR shouldn't change */
}

PERL_STATIC_INLINE UV
S_invlist_max(pTHX_ SV* const invlist)
{
    /* Returns the maximum number of elements storable in the inversion list's
     * array, without having to realloc() */

    PERL_ARGS_ASSERT_INVLIST_MAX;

    return FROM_INTERNAL_SIZE(SvLEN(invlist));
}

PERL_STATIC_INLINE UV*
S_get_invlist_zero_addr(pTHX_ SV* invlist)
{
    /* Return the address of the UV that is reserved to hold 0 if the inversion
     * list contains 0.  This has to be the last element of the heading, as the
     * list proper starts with either it if 0, or the next element if not.
     * (But we force it to contain either 0 or 1) */

    PERL_ARGS_ASSERT_GET_INVLIST_ZERO_ADDR;

    return (UV *) (SvPVX(invlist) + (INVLIST_ZERO_OFFSET * sizeof (UV)));
}

#ifndef PERL_IN_XSUB_RE
SV*
Perl__new_invlist(pTHX_ IV initial_size)
{

    /* Return a pointer to a newly constructed inversion list, with enough
     * space to store 'initial_size' elements.  If that number is negative, a
     * system default is used instead */

    SV* new_list;

    if (initial_size < 0) {
	initial_size = INVLIST_INITIAL_LEN;
    }

    /* Allocate the initial space */
    new_list = newSV(TO_INTERNAL_SIZE(initial_size));
    invlist_set_len(new_list, 0);

    /* Force iterinit() to be used to get iteration to work */
    *get_invlist_iter_addr(new_list) = UV_MAX;

    /* This should force a segfault if a method doesn't initialize this
     * properly */
    *get_invlist_zero_addr(new_list) = UV_MAX;

    *get_invlist_version_id_addr(new_list) = INVLIST_VERSION_ID;
#if HEADER_LENGTH != 4
#   error Need to regenerate VERSION_ID by running perl -E 'say int(rand 2**31-1)', and then changing the #if to the new length
#endif

    return new_list;
}
#endif

STATIC SV*
S__new_invlist_C_array(pTHX_ UV* list)
{
    /* Return a pointer to a newly constructed inversion list, initialized to
     * point to <list>, which has to be in the exact correct inversion list
     * form, including internal fields.  Thus this is a dangerous routine that
     * should not be used in the wrong hands */

    SV* invlist = newSV_type(SVt_PV);

    PERL_ARGS_ASSERT__NEW_INVLIST_C_ARRAY;

    SvPV_set(invlist, (char *) list);
    SvLEN_set(invlist, 0);  /* Means we own the contents, and the system
			       shouldn't touch it */
    SvCUR_set(invlist, TO_INTERNAL_SIZE(invlist_len(invlist)));

    if (*get_invlist_version_id_addr(invlist) != INVLIST_VERSION_ID) {
        Perl_croak(aTHX_ "panic: Incorrect version for previously generated inversion list");
    }

    return invlist;
}

STATIC void
S_invlist_extend(pTHX_ SV* const invlist, const UV new_max)
{
    /* Grow the maximum size of an inversion list */

    PERL_ARGS_ASSERT_INVLIST_EXTEND;

    SvGROW((SV *)invlist, TO_INTERNAL_SIZE(new_max));
}

PERL_STATIC_INLINE void
S_invlist_trim(pTHX_ SV* const invlist)
{
    PERL_ARGS_ASSERT_INVLIST_TRIM;

    /* Change the length of the inversion list to how many entries it currently
     * has */

    SvPV_shrink_to_cur((SV *) invlist);
}

/* An element is in an inversion list iff its index is even numbered: 0, 2, 4,
 * etc */
#define ELEMENT_RANGE_MATCHES_INVLIST(i) (! ((i) & 1))
#define PREV_RANGE_MATCHES_INVLIST(i) (! ELEMENT_RANGE_MATCHES_INVLIST(i))

#define _invlist_union_complement_2nd(a, b, output) _invlist_union_maybe_complement_2nd(a, b, TRUE, output)

STATIC void
S__append_range_to_invlist(pTHX_ SV* const invlist, const UV start, const UV end)
{
   /* Subject to change or removal.  Append the range from 'start' to 'end' at
    * the end of the inversion list.  The range must be above any existing
    * ones. */

    UV* array;
    UV max = invlist_max(invlist);
    UV len = invlist_len(invlist);

    PERL_ARGS_ASSERT__APPEND_RANGE_TO_INVLIST;

    if (len == 0) { /* Empty lists must be initialized */
        array = _invlist_array_init(invlist, start == 0);
    }
    else {
	/* Here, the existing list is non-empty. The current max entry in the
	 * list is generally the first value not in the set, except when the
	 * set extends to the end of permissible values, in which case it is
	 * the first entry in that final set, and so this call is an attempt to
	 * append out-of-order */

	UV final_element = len - 1;
	array = invlist_array(invlist);
	if (array[final_element] > start
	    || ELEMENT_RANGE_MATCHES_INVLIST(final_element))
	{
	    Perl_croak(aTHX_ "panic: attempting to append to an inversion list, but wasn't at the end of the list, final=%"UVuf", start=%"UVuf", match=%c",
		       array[final_element], start,
		       ELEMENT_RANGE_MATCHES_INVLIST(final_element) ? 't' : 'f');
	}

	/* Here, it is a legal append.  If the new range begins with the first
	 * value not in the set, it is extending the set, so the new first
	 * value not in the set is one greater than the newly extended range.
	 * */
	if (array[final_element] == start) {
	    if (end != UV_MAX) {
		array[final_element] = end + 1;
	    }
	    else {
		/* But if the end is the maximum representable on the machine,
		 * just let the range that this would extend to have no end */
		invlist_set_len(invlist, len - 1);
	    }
	    return;
	}
    }

    /* Here the new range doesn't extend any existing set.  Add it */

    len += 2;	/* Includes an element each for the start and end of range */

    /* If overflows the existing space, extend, which may cause the array to be
     * moved */
    if (max < len) {
	invlist_extend(invlist, len);
	invlist_set_len(invlist, len);	/* Have to set len here to avoid assert
					   failure in invlist_array() */
	array = invlist_array(invlist);
    }
    else {
	invlist_set_len(invlist, len);
    }

    /* The next item on the list starts the range, the one after that is
     * one past the new range.  */
    array[len - 2] = start;
    if (end != UV_MAX) {
	array[len - 1] = end + 1;
    }
    else {
	/* But if the end is the maximum representable on the machine, just let
	 * the range have no end */
	invlist_set_len(invlist, len - 1);
    }
}

#ifndef PERL_IN_XSUB_RE

STATIC IV
S_invlist_search(pTHX_ SV* const invlist, const UV cp)
{
    /* Searches the inversion list for the entry that contains the input code
     * point <cp>.  If <cp> is not in the list, -1 is returned.  Otherwise, the
     * return value is the index into the list's array of the range that
     * contains <cp> */

    IV low = 0;
    IV high = invlist_len(invlist);
    const UV * const array = invlist_array(invlist);

    PERL_ARGS_ASSERT_INVLIST_SEARCH;

    /* If list is empty or the code point is before the first element, return
     * failure. */
    if (high == 0 || cp < array[0]) {
	return -1;
    }

    /* Binary search.  What we are looking for is <i> such that
     *	array[i] <= cp < array[i+1]
     * The loop below converges on the i+1. */
    while (low < high) {
	IV mid = (low + high) / 2;
	if (array[mid] <= cp) {
	    low = mid + 1;

	    /* We could do this extra test to exit the loop early.
	    if (cp < array[low]) {
		return mid;
	    }
	    */
	}
	else { /* cp < array[mid] */
	    high = mid;
	}
    }

    return high - 1;
}

void
Perl__invlist_populate_swatch(pTHX_ SV* const invlist, const UV start, const UV end, U8* swatch)
{
    /* populates a swatch of a swash the same way swatch_get() does in utf8.c,
     * but is used when the swash has an inversion list.  This makes this much
     * faster, as it uses a binary search instead of a linear one.  This is
     * intimately tied to that function, and perhaps should be in utf8.c,
     * except it is intimately tied to inversion lists as well.  It assumes
     * that <swatch> is all 0's on input */

    UV current = start;
    const IV len = invlist_len(invlist);
    IV i;
    const UV * array;

    PERL_ARGS_ASSERT__INVLIST_POPULATE_SWATCH;

    if (len == 0) { /* Empty inversion list */
        return;
    }

    array = invlist_array(invlist);

    /* Find which element it is */
    i = invlist_search(invlist, start);

    /* We populate from <start> to <end> */
    while (current < end) {
        UV upper;

	/* The inversion list gives the results for every possible code point
	 * after the first one in the list.  Only those ranges whose index is
	 * even are ones that the inversion list matches.  For the odd ones,
	 * and if the initial code point is not in the list, we have to skip
	 * forward to the next element */
        if (i == -1 || ! ELEMENT_RANGE_MATCHES_INVLIST(i)) {
            i++;
            if (i >= len) { /* Finished if beyond the end of the array */
                return;
            }
            current = array[i];
	    if (current >= end) {   /* Finished if beyond the end of what we
				       are populating */
                return;
            }
        }
        assert(current >= start);

	/* The current range ends one below the next one, except don't go past
	 * <end> */
        i++;
        upper = (i < len && array[i] < end) ? array[i] : end;

	/* Here we are in a range that matches.  Populate a bit in the 3-bit U8
	 * for each code point in it */
        for (; current < upper; current++) {
            const STRLEN offset = (STRLEN)(current - start);
            swatch[offset >> 3] |= 1 << (offset & 7);
        }

	/* Quit if at the end of the list */
        if (i >= len) {

	    /* But first, have to deal with the highest possible code point on
	     * the platform.  The previous code assumes that <end> is one
	     * beyond where we want to populate, but that is impossible at the
	     * platform's infinity, so have to handle it specially */
            if (UNLIKELY(end == UV_MAX && ELEMENT_RANGE_MATCHES_INVLIST(len-1)))
	    {
                const STRLEN offset = (STRLEN)(end - start);
                swatch[offset >> 3] |= 1 << (offset & 7);
            }
            return;
        }

	/* Advance to the next range, which will be for code points not in the
	 * inversion list */
        current = array[i];
    }

    return;
}


void
Perl__invlist_union_maybe_complement_2nd(pTHX_ SV* const a, SV* const b, bool complement_b, SV** output)
{
    /* Take the union of two inversion lists and point <output> to it.  *output
     * should be defined upon input, and if it points to one of the two lists,
     * the reference count to that list will be decremented.  The first list,
     * <a>, may be NULL, in which case a copy of the second list is returned.
     * If <complement_b> is TRUE, the union is taken of the complement
     * (inversion) of <b> instead of b itself.
     *
     * The basis for this comes from "Unicode Demystified" Chapter 13 by
     * Richard Gillam, published by Addison-Wesley, and explained at some
     * length there.  The preface says to incorporate its examples into your
     * code at your own risk.
     *
     * The algorithm is like a merge sort.
     *
     * XXX A potential performance improvement is to keep track as we go along
     * if only one of the inputs contributes to the result, meaning the other
     * is a subset of that one.  In that case, we can skip the final copy and
     * return the larger of the input lists, but then outside code might need
     * to keep track of whether to free the input list or not */

    UV* array_a;    /* a's array */
    UV* array_b;
    UV len_a;	    /* length of a's array */
    UV len_b;

    SV* u;			/* the resulting union */
    UV* array_u;
    UV len_u;

    UV i_a = 0;		    /* current index into a's array */
    UV i_b = 0;
    UV i_u = 0;

    /* running count, as explained in the algorithm source book; items are
     * stopped accumulating and are output when the count changes to/from 0.
     * The count is incremented when we start a range that's in the set, and
     * decremented when we start a range that's not in the set.  So its range
     * is 0 to 2.  Only when the count is zero is something not in the set.
     */
    UV count = 0;

    PERL_ARGS_ASSERT__INVLIST_UNION_MAYBE_COMPLEMENT_2ND;
    assert(a != b);

    /* If either one is empty, the union is the other one */
    if (a == NULL || ((len_a = invlist_len(a)) == 0)) {
	if (*output == a) {
            if (a != NULL) {
                SvREFCNT_dec(a);
            }
	}
	if (*output != b) {
	    *output = invlist_clone(b);
            if (complement_b) {
                _invlist_invert(*output);
            }
	} /* else *output already = b; */
	return;
    }
    else if ((len_b = invlist_len(b)) == 0) {
	if (*output == b) {
	    SvREFCNT_dec(b);
	}

        /* The complement of an empty list is a list that has everything in it,
         * so the union with <a> includes everything too */
        if (complement_b) {
            if (a == *output) {
                SvREFCNT_dec(a);
            }
            *output = _new_invlist(1);
            _append_range_to_invlist(*output, 0, UV_MAX);
        }
        else if (*output != a) {
            *output = invlist_clone(a);
        }
        /* else *output already = a; */
	return;
    }

    /* Here both lists exist and are non-empty */
    array_a = invlist_array(a);
    array_b = invlist_array(b);

    /* If are to take the union of 'a' with the complement of b, set it
     * up so are looking at b's complement. */
    if (complement_b) {

	/* To complement, we invert: if the first element is 0, remove it.  To
	 * do this, we just pretend the array starts one later, and clear the
	 * flag as we don't have to do anything else later */
        if (array_b[0] == 0) {
            array_b++;
            len_b--;
            complement_b = FALSE;
        }
        else {

            /* But if the first element is not zero, we unshift a 0 before the
             * array.  The data structure reserves a space for that 0 (which
             * should be a '1' right now), so physical shifting is unneeded,
             * but temporarily change that element to 0.  Before exiting the
             * routine, we must restore the element to '1' */
            array_b--;
            len_b++;
            array_b[0] = 0;
        }
    }

    /* Size the union for the worst case: that the sets are completely
     * disjoint */
    u = _new_invlist(len_a + len_b);

    /* Will contain U+0000 if either component does */
    array_u = _invlist_array_init(u, (len_a > 0 && array_a[0] == 0)
				      || (len_b > 0 && array_b[0] == 0));

    /* Go through each list item by item, stopping when exhausted one of
     * them */
    while (i_a < len_a && i_b < len_b) {
	UV cp;	    /* The element to potentially add to the union's array */
	bool cp_in_set;   /* is it in the the input list's set or not */

	/* We need to take one or the other of the two inputs for the union.
	 * Since we are merging two sorted lists, we take the smaller of the
	 * next items.  In case of a tie, we take the one that is in its set
	 * first.  If we took one not in the set first, it would decrement the
	 * count, possibly to 0 which would cause it to be output as ending the
	 * range, and the next time through we would take the same number, and
	 * output it again as beginning the next range.  By doing it the
	 * opposite way, there is no possibility that the count will be
	 * momentarily decremented to 0, and thus the two adjoining ranges will
	 * be seamlessly merged.  (In a tie and both are in the set or both not
	 * in the set, it doesn't matter which we take first.) */
	if (array_a[i_a] < array_b[i_b]
	    || (array_a[i_a] == array_b[i_b]
		&& ELEMENT_RANGE_MATCHES_INVLIST(i_a)))
	{
	    cp_in_set = ELEMENT_RANGE_MATCHES_INVLIST(i_a);
	    cp= array_a[i_a++];
	}
	else {
	    cp_in_set = ELEMENT_RANGE_MATCHES_INVLIST(i_b);
	    cp= array_b[i_b++];
	}

	/* Here, have chosen which of the two inputs to look at.  Only output
	 * if the running count changes to/from 0, which marks the
	 * beginning/end of a range in that's in the set */
	if (cp_in_set) {
	    if (count == 0) {
		array_u[i_u++] = cp;
	    }
	    count++;
	}
	else {
	    count--;
	    if (count == 0) {
		array_u[i_u++] = cp;
	    }
	}
    }

    /* Here, we are finished going through at least one of the lists, which
     * means there is something remaining in at most one.  We check if the list
     * that hasn't been exhausted is positioned such that we are in the middle
     * of a range in its set or not.  (i_a and i_b point to the element beyond
     * the one we care about.) If in the set, we decrement 'count'; if 0, there
     * is potentially more to output.
     * There are four cases:
     *	1) Both weren't in their sets, count is 0, and remains 0.  What's left
     *	   in the union is entirely from the non-exhausted set.
     *	2) Both were in their sets, count is 2.  Nothing further should
     *	   be output, as everything that remains will be in the exhausted
     *	   list's set, hence in the union; decrementing to 1 but not 0 insures
     *	   that
     *	3) the exhausted was in its set, non-exhausted isn't, count is 1.
     *	   Nothing further should be output because the union includes
     *	   everything from the exhausted set.  Not decrementing ensures that.
     *	4) the exhausted wasn't in its set, non-exhausted is, count is 1;
     *	   decrementing to 0 insures that we look at the remainder of the
     *	   non-exhausted set */
    if ((i_a != len_a && PREV_RANGE_MATCHES_INVLIST(i_a))
	|| (i_b != len_b && PREV_RANGE_MATCHES_INVLIST(i_b)))
    {
	count--;
    }

    /* The final length is what we've output so far, plus what else is about to
     * be output.  (If 'count' is non-zero, then the input list we exhausted
     * has everything remaining up to the machine's limit in its set, and hence
     * in the union, so there will be no further output. */
    len_u = i_u;
    if (count == 0) {
	/* At most one of the subexpressions will be non-zero */
	len_u += (len_a - i_a) + (len_b - i_b);
    }

    /* Set result to final length, which can change the pointer to array_u, so
     * re-find it */
    if (len_u != invlist_len(u)) {
	invlist_set_len(u, len_u);
	invlist_trim(u);
	array_u = invlist_array(u);
    }

    /* When 'count' is 0, the list that was exhausted (if one was shorter than
     * the other) ended with everything above it not in its set.  That means
     * that the remaining part of the union is precisely the same as the
     * non-exhausted list, so can just copy it unchanged.  (If both list were
     * exhausted at the same time, then the operations below will be both 0.)
     */
    if (count == 0) {
	IV copy_count; /* At most one will have a non-zero copy count */
	if ((copy_count = len_a - i_a) > 0) {
	    Copy(array_a + i_a, array_u + i_u, copy_count, UV);
	}
	else if ((copy_count = len_b - i_b) > 0) {
	    Copy(array_b + i_b, array_u + i_u, copy_count, UV);
	}
    }

    /*  We may be removing a reference to one of the inputs */
    if (a == *output || b == *output) {
	SvREFCNT_dec(*output);
    }

    /* If we've changed b, restore it */
    if (complement_b) {
        array_b[0] = 1;
    }

    *output = u;
    return;
}

void
Perl__invlist_intersection_maybe_complement_2nd(pTHX_ SV* const a, SV* const b, bool complement_b, SV** i)
{
    /* Take the intersection of two inversion lists and point <i> to it.  *i
     * should be defined upon input, and if it points to one of the two lists,
     * the reference count to that list will be decremented.
     * If <complement_b> is TRUE, the result will be the intersection of <a>
     * and the complement (or inversion) of <b> instead of <b> directly.
     *
     * The basis for this comes from "Unicode Demystified" Chapter 13 by
     * Richard Gillam, published by Addison-Wesley, and explained at some
     * length there.  The preface says to incorporate its examples into your
     * code at your own risk.  In fact, it had bugs
     *
     * The algorithm is like a merge sort, and is essentially the same as the
     * union above
     */

    UV* array_a;		/* a's array */
    UV* array_b;
    UV len_a;	/* length of a's array */
    UV len_b;

    SV* r;		     /* the resulting intersection */
    UV* array_r;
    UV len_r;

    UV i_a = 0;		    /* current index into a's array */
    UV i_b = 0;
    UV i_r = 0;

    /* running count, as explained in the algorithm source book; items are
     * stopped accumulating and are output when the count changes to/from 2.
     * The count is incremented when we start a range that's in the set, and
     * decremented when we start a range that's not in the set.  So its range
     * is 0 to 2.  Only when the count is 2 is something in the intersection.
     */
    UV count = 0;

    PERL_ARGS_ASSERT__INVLIST_INTERSECTION_MAYBE_COMPLEMENT_2ND;
    assert(a != b);

    /* Special case if either one is empty */
    len_a = invlist_len(a);
    if ((len_a == 0) || ((len_b = invlist_len(b)) == 0)) {

        if (len_a != 0 && complement_b) {

            /* Here, 'a' is not empty, therefore from the above 'if', 'b' must
             * be empty.  Here, also we are using 'b's complement, which hence
             * must be every possible code point.  Thus the intersection is
             * simply 'a'. */
            if (*i != a) {
                *i = invlist_clone(a);

                if (*i == b) {
                    SvREFCNT_dec(b);
                }
            }
            /* else *i is already 'a' */
            return;
        }

        /* Here, 'a' or 'b' is empty and not using the complement of 'b'.  The
         * intersection must be empty */
	if (*i == a) {
	    SvREFCNT_dec(a);
	}
	else if (*i == b) {
	    SvREFCNT_dec(b);
	}
	*i = _new_invlist(0);
	return;
    }

    /* Here both lists exist and are non-empty */
    array_a = invlist_array(a);
    array_b = invlist_array(b);

    /* If are to take the intersection of 'a' with the complement of b, set it
     * up so are looking at b's complement. */
    if (complement_b) {

	/* To complement, we invert: if the first element is 0, remove it.  To
	 * do this, we just pretend the array starts one later, and clear the
	 * flag as we don't have to do anything else later */
        if (array_b[0] == 0) {
            array_b++;
            len_b--;
            complement_b = FALSE;
        }
        else {

            /* But if the first element is not zero, we unshift a 0 before the
             * array.  The data structure reserves a space for that 0 (which
             * should be a '1' right now), so physical shifting is unneeded,
             * but temporarily change that element to 0.  Before exiting the
             * routine, we must restore the element to '1' */
            array_b--;
            len_b++;
            array_b[0] = 0;
        }
    }

    /* Size the intersection for the worst case: that the intersection ends up
     * fragmenting everything to be completely disjoint */
    r= _new_invlist(len_a + len_b);

    /* Will contain U+0000 iff both components do */
    array_r = _invlist_array_init(r, len_a > 0 && array_a[0] == 0
				     && len_b > 0 && array_b[0] == 0);

    /* Go through each list item by item, stopping when exhausted one of
     * them */
    while (i_a < len_a && i_b < len_b) {
	UV cp;	    /* The element to potentially add to the intersection's
		       array */
	bool cp_in_set;	/* Is it in the input list's set or not */

	/* We need to take one or the other of the two inputs for the
	 * intersection.  Since we are merging two sorted lists, we take the
	 * smaller of the next items.  In case of a tie, we take the one that
	 * is not in its set first (a difference from the union algorithm).  If
	 * we took one in the set first, it would increment the count, possibly
	 * to 2 which would cause it to be output as starting a range in the
	 * intersection, and the next time through we would take that same
	 * number, and output it again as ending the set.  By doing it the
	 * opposite of this, there is no possibility that the count will be
	 * momentarily incremented to 2.  (In a tie and both are in the set or
	 * both not in the set, it doesn't matter which we take first.) */
	if (array_a[i_a] < array_b[i_b]
	    || (array_a[i_a] == array_b[i_b]
		&& ! ELEMENT_RANGE_MATCHES_INVLIST(i_a)))
	{
	    cp_in_set = ELEMENT_RANGE_MATCHES_INVLIST(i_a);
	    cp= array_a[i_a++];
	}
	else {
	    cp_in_set = ELEMENT_RANGE_MATCHES_INVLIST(i_b);
	    cp= array_b[i_b++];
	}

	/* Here, have chosen which of the two inputs to look at.  Only output
	 * if the running count changes to/from 2, which marks the
	 * beginning/end of a range that's in the intersection */
	if (cp_in_set) {
	    count++;
	    if (count == 2) {
		array_r[i_r++] = cp;
	    }
	}
	else {
	    if (count == 2) {
		array_r[i_r++] = cp;
	    }
	    count--;
	}
    }

    /* Here, we are finished going through at least one of the lists, which
     * means there is something remaining in at most one.  We check if the list
     * that has been exhausted is positioned such that we are in the middle
     * of a range in its set or not.  (i_a and i_b point to elements 1 beyond
     * the ones we care about.)  There are four cases:
     *	1) Both weren't in their sets, count is 0, and remains 0.  There's
     *	   nothing left in the intersection.
     *	2) Both were in their sets, count is 2 and perhaps is incremented to
     *	   above 2.  What should be output is exactly that which is in the
     *	   non-exhausted set, as everything it has is also in the intersection
     *	   set, and everything it doesn't have can't be in the intersection
     *	3) The exhausted was in its set, non-exhausted isn't, count is 1, and
     *	   gets incremented to 2.  Like the previous case, the intersection is
     *	   everything that remains in the non-exhausted set.
     *	4) the exhausted wasn't in its set, non-exhausted is, count is 1, and
     *	   remains 1.  And the intersection has nothing more. */
    if ((i_a == len_a && PREV_RANGE_MATCHES_INVLIST(i_a))
	|| (i_b == len_b && PREV_RANGE_MATCHES_INVLIST(i_b)))
    {
	count++;
    }

    /* The final length is what we've output so far plus what else is in the
     * intersection.  At most one of the subexpressions below will be non-zero */
    len_r = i_r;
    if (count >= 2) {
	len_r += (len_a - i_a) + (len_b - i_b);
    }

    /* Set result to final length, which can change the pointer to array_r, so
     * re-find it */
    if (len_r != invlist_len(r)) {
	invlist_set_len(r, len_r);
	invlist_trim(r);
	array_r = invlist_array(r);
    }

    /* Finish outputting any remaining */
    if (count >= 2) { /* At most one will have a non-zero copy count */
	IV copy_count;
	if ((copy_count = len_a - i_a) > 0) {
	    Copy(array_a + i_a, array_r + i_r, copy_count, UV);
	}
	else if ((copy_count = len_b - i_b) > 0) {
	    Copy(array_b + i_b, array_r + i_r, copy_count, UV);
	}
    }

    /*  We may be removing a reference to one of the inputs */
    if (a == *i || b == *i) {
	SvREFCNT_dec(*i);
    }

    /* If we've changed b, restore it */
    if (complement_b) {
        array_b[0] = 1;
    }

    *i = r;
    return;
}

SV*
Perl__add_range_to_invlist(pTHX_ SV* invlist, const UV start, const UV end)
{
    /* Add the range from 'start' to 'end' inclusive to the inversion list's
     * set.  A pointer to the inversion list is returned.  This may actually be
     * a new list, in which case the passed in one has been destroyed.  The
     * passed in inversion list can be NULL, in which case a new one is created
     * with just the one range in it */

    SV* range_invlist;
    UV len;

    if (invlist == NULL) {
	invlist = _new_invlist(2);
	len = 0;
    }
    else {
	len = invlist_len(invlist);
    }

    /* If comes after the final entry, can just append it to the end */
    if (len == 0
	|| start >= invlist_array(invlist)
				    [invlist_len(invlist) - 1])
    {
	_append_range_to_invlist(invlist, start, end);
	return invlist;
    }

    /* Here, can't just append things, create and return a new inversion list
     * which is the union of this range and the existing inversion list */
    range_invlist = _new_invlist(2);
    _append_range_to_invlist(range_invlist, start, end);

    _invlist_union(invlist, range_invlist, &invlist);

    /* The temporary can be freed */
    SvREFCNT_dec(range_invlist);

    return invlist;
}

#endif

PERL_STATIC_INLINE SV*
S_add_cp_to_invlist(pTHX_ SV* invlist, const UV cp) {
    return _add_range_to_invlist(invlist, cp, cp);
}

#ifndef PERL_IN_XSUB_RE
void
Perl__invlist_invert(pTHX_ SV* const invlist)
{
    /* Complement the input inversion list.  This adds a 0 if the list didn't
     * have a zero; removes it otherwise.  As described above, the data
     * structure is set up so that this is very efficient */

    UV* len_pos = get_invlist_len_addr(invlist);

    PERL_ARGS_ASSERT__INVLIST_INVERT;

    /* The inverse of matching nothing is matching everything */
    if (*len_pos == 0) {
	_append_range_to_invlist(invlist, 0, UV_MAX);
	return;
    }

    /* The exclusive or complents 0 to 1; and 1 to 0.  If the result is 1, the
     * zero element was a 0, so it is being removed, so the length decrements
     * by 1; and vice-versa.  SvCUR is unaffected */
    if (*get_invlist_zero_addr(invlist) ^= 1) {
	(*len_pos)--;
    }
    else {
	(*len_pos)++;
    }
}

void
Perl__invlist_invert_prop(pTHX_ SV* const invlist)
{
    /* Complement the input inversion list (which must be a Unicode property,
     * all of which don't match above the Unicode maximum code point.)  And
     * Perl has chosen to not have the inversion match above that either.  This
     * adds a 0x110000 if the list didn't end with it, and removes it if it did
     */

    UV len;
    UV* array;

    PERL_ARGS_ASSERT__INVLIST_INVERT_PROP;

    _invlist_invert(invlist);

    len = invlist_len(invlist);

    if (len != 0) { /* If empty do nothing */
	array = invlist_array(invlist);
	if (array[len - 1] != PERL_UNICODE_MAX + 1) {
	    /* Add 0x110000.  First, grow if necessary */
	    len++;
	    if (invlist_max(invlist) < len) {
		invlist_extend(invlist, len);
		array = invlist_array(invlist);
	    }
	    invlist_set_len(invlist, len);
	    array[len - 1] = PERL_UNICODE_MAX + 1;
	}
	else {  /* Remove the 0x110000 */
	    invlist_set_len(invlist, len - 1);
	}
    }

    return;
}
#endif

PERL_STATIC_INLINE SV*
S_invlist_clone(pTHX_ SV* const invlist)
{

    /* Return a new inversion list that is a copy of the input one, which is
     * unchanged */

    /* Need to allocate extra space to accommodate Perl's addition of a
     * trailing NUL to SvPV's, since it thinks they are always strings */
    SV* new_invlist = _new_invlist(invlist_len(invlist) + 1);
    STRLEN length = SvCUR(invlist);

    PERL_ARGS_ASSERT_INVLIST_CLONE;

    SvCUR_set(new_invlist, length); /* This isn't done automatically */
    Copy(SvPVX(invlist), SvPVX(new_invlist), length, char);

    return new_invlist;
}

PERL_STATIC_INLINE UV*
S_get_invlist_iter_addr(pTHX_ SV* invlist)
{
    /* Return the address of the UV that contains the current iteration
     * position */

    PERL_ARGS_ASSERT_GET_INVLIST_ITER_ADDR;

    return (UV *) (SvPVX(invlist) + (INVLIST_ITER_OFFSET * sizeof (UV)));
}

PERL_STATIC_INLINE UV*
S_get_invlist_version_id_addr(pTHX_ SV* invlist)
{
    /* Return the address of the UV that contains the version id. */

    PERL_ARGS_ASSERT_GET_INVLIST_VERSION_ID_ADDR;

    return (UV *) (SvPVX(invlist) + (INVLIST_VERSION_ID_OFFSET * sizeof (UV)));
}

PERL_STATIC_INLINE void
S_invlist_iterinit(pTHX_ SV* invlist)	/* Initialize iterator for invlist */
{
    PERL_ARGS_ASSERT_INVLIST_ITERINIT;

    *get_invlist_iter_addr(invlist) = 0;
}

STATIC bool
S_invlist_iternext(pTHX_ SV* invlist, UV* start, UV* end)
{
    /* An C<invlist_iterinit> call on <invlist> must be used to set this up.
     * This call sets in <*start> and <*end>, the next range in <invlist>.
     * Returns <TRUE> if successful and the next call will return the next
     * range; <FALSE> if was already at the end of the list.  If the latter,
     * <*start> and <*end> are unchanged, and the next call to this function
     * will start over at the beginning of the list */

    UV* pos = get_invlist_iter_addr(invlist);
    UV len = invlist_len(invlist);
    UV *array;

    PERL_ARGS_ASSERT_INVLIST_ITERNEXT;

    if (*pos >= len) {
	*pos = UV_MAX;	/* Force iternit() to be required next time */
	return FALSE;
    }

    array = invlist_array(invlist);

    *start = array[(*pos)++];

    if (*pos >= len) {
	*end = UV_MAX;
    }
    else {
	*end = array[(*pos)++] - 1;
    }

    return TRUE;
}

#ifndef PERL_IN_XSUB_RE
SV *
Perl__invlist_contents(pTHX_ SV* const invlist)
{
    /* Get the contents of an inversion list into a string SV so that they can
     * be printed out.  It uses the format traditionally done for debug tracing
     */

    UV start, end;
    SV* output = newSVpvs("\n");

    PERL_ARGS_ASSERT__INVLIST_CONTENTS;

    invlist_iterinit(invlist);
    while (invlist_iternext(invlist, &start, &end)) {
	if (end == UV_MAX) {
	    Perl_sv_catpvf(aTHX_ output, "%04"UVXf"\tINFINITY\n", start);
	}
	else if (end != start) {
	    Perl_sv_catpvf(aTHX_ output, "%04"UVXf"\t%04"UVXf"\n",
		    start,       end);
	}
	else {
	    Perl_sv_catpvf(aTHX_ output, "%04"UVXf"\n", start);
	}
    }

    return output;
}
#endif

#if 0
void
S_invlist_dump(pTHX_ SV* const invlist, const char * const header)
{
    /* Dumps out the ranges in an inversion list.  The string 'header'
     * if present is output on a line before the first range */

    UV start, end;

    if (header && strlen(header)) {
	PerlIO_printf(Perl_debug_log, "%s\n", header);
    }
    invlist_iterinit(invlist);
    while (invlist_iternext(invlist, &start, &end)) {
	if (end == UV_MAX) {
	    PerlIO_printf(Perl_debug_log, "0x%04"UVXf" .. INFINITY\n", start);
	}
	else {
	    PerlIO_printf(Perl_debug_log, "0x%04"UVXf" .. 0x%04"UVXf"\n", start, end);
	}
    }
}
#endif

#undef HEADER_LENGTH
#undef INVLIST_INITIAL_LENGTH
#undef TO_INTERNAL_SIZE
#undef FROM_INTERNAL_SIZE
#undef INVLIST_LEN_OFFSET
#undef INVLIST_ZERO_OFFSET
#undef INVLIST_ITER_OFFSET
#undef INVLIST_VERSION_ID

/* End of inversion list object */

/*
 - reg - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid.
 */
#define REGTAIL(x,y,z) regtail((x),(y),(z),depth+1)
#ifdef DEBUGGING
#define REGTAIL_STUDY(x,y,z) regtail_study((x),(y),(z),depth+1)
#else
#define REGTAIL_STUDY(x,y,z) regtail((x),(y),(z),depth+1)
#endif

STATIC regnode *
S_reg(pTHX_ RExC_state_t *pRExC_state, I32 paren, I32 *flagp,U32 depth)
    /* paren: Parenthesized? 0=top, 1=(, inside: changed to letter. */
{
    dVAR;
    register regnode *ret;		/* Will be the head of the group. */
    register regnode *br;
    register regnode *lastbr;
    register regnode *ender = NULL;
    register I32 parno = 0;
    I32 flags;
    U32 oregflags = RExC_flags;
    bool have_branch = 0;
    bool is_open = 0;
    I32 freeze_paren = 0;
    I32 after_freeze = 0;

    /* for (?g), (?gc), and (?o) warnings; warning
       about (?c) will warn about (?g) -- japhy    */

#define WASTED_O  0x01
#define WASTED_G  0x02
#define WASTED_C  0x04
#define WASTED_GC (0x02|0x04)
    I32 wastedflags = 0x00;

    char * parse_start = RExC_parse; /* MJD */
    char * const oregcomp_parse = RExC_parse;

    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REG;
    DEBUG_PARSE("reg ");

    *flagp = 0;				/* Tentatively. */


    /* Make an OPEN node, if parenthesized. */
    if (paren) {
        if ( *RExC_parse == '*') { /* (*VERB:ARG) */
	    char *start_verb = RExC_parse;
	    STRLEN verb_len = 0;
	    char *start_arg = NULL;
	    unsigned char op = 0;
	    int argok = 1;
	    int internal_argval = 0; /* internal_argval is only useful if !argok */
	    while ( *RExC_parse && *RExC_parse != ')' ) {
	        if ( *RExC_parse == ':' ) {
	            start_arg = RExC_parse + 1;
	            break;
	        }
	        RExC_parse++;
	    }
	    ++start_verb;
	    verb_len = RExC_parse - start_verb;
	    if ( start_arg ) {
	        RExC_parse++;
	        while ( *RExC_parse && *RExC_parse != ')' ) 
	            RExC_parse++;
	        if ( *RExC_parse != ')' ) 
	            vFAIL("Unterminated verb pattern argument");
	        if ( RExC_parse == start_arg )
	            start_arg = NULL;
	    } else {
	        if ( *RExC_parse != ')' )
	            vFAIL("Unterminated verb pattern");
	    }
	    
	    switch ( *start_verb ) {
            case 'A':  /* (*ACCEPT) */
                if ( memEQs(start_verb,verb_len,"ACCEPT") ) {
		    op = ACCEPT;
		    internal_argval = RExC_nestroot;
		}
		break;
            case 'C':  /* (*COMMIT) */
                if ( memEQs(start_verb,verb_len,"COMMIT") )
                    op = COMMIT;
                break;
            case 'F':  /* (*FAIL) */
                if ( verb_len==1 || memEQs(start_verb,verb_len,"FAIL") ) {
		    op = OPFAIL;
		    argok = 0;
		}
		break;
            case ':':  /* (*:NAME) */
	    case 'M':  /* (*MARK:NAME) */
	        if ( verb_len==0 || memEQs(start_verb,verb_len,"MARK") ) {
                    op = MARKPOINT;
                    argok = -1;
                }
                break;
            case 'P':  /* (*PRUNE) */
                if ( memEQs(start_verb,verb_len,"PRUNE") )
                    op = PRUNE;
                break;
            case 'S':   /* (*SKIP) */  
                if ( memEQs(start_verb,verb_len,"SKIP") ) 
                    op = SKIP;
                break;
            case 'T':  /* (*THEN) */
                /* [19:06] <TimToady> :: is then */
                if ( memEQs(start_verb,verb_len,"THEN") ) {
                    op = CUTGROUP;
                    RExC_seen |= REG_SEEN_CUTGROUP;
                }
                break;
	    }
	    if ( ! op ) {
	        RExC_parse++;
	        vFAIL3("Unknown verb pattern '%.*s'",
	            verb_len, start_verb);
	    }
	    if ( argok ) {
                if ( start_arg && internal_argval ) {
	            vFAIL3("Verb pattern '%.*s' may not have an argument",
	                verb_len, start_verb); 
	        } else if ( argok < 0 && !start_arg ) {
                    vFAIL3("Verb pattern '%.*s' has a mandatory argument",
	                verb_len, start_verb);    
	        } else {
	            ret = reganode(pRExC_state, op, internal_argval);
	            if ( ! internal_argval && ! SIZE_ONLY ) {
                        if (start_arg) {
                            SV *sv = newSVpvn( start_arg, RExC_parse - start_arg);
                            ARG(ret) = add_data( pRExC_state, 1, "S" );
                            RExC_rxi->data->data[ARG(ret)]=(void*)sv;
                            ret->flags = 0;
                        } else {
                            ret->flags = 1; 
                        }
                    }	            
	        }
	        if (!internal_argval)
	            RExC_seen |= REG_SEEN_VERBARG;
	    } else if ( start_arg ) {
	        vFAIL3("Verb pattern '%.*s' may not have an argument",
	                verb_len, start_verb);    
	    } else {
	        ret = reg_node(pRExC_state, op);
	    }
	    nextchar(pRExC_state);
	    return ret;
        } else 
	if (*RExC_parse == '?') { /* (?...) */
	    bool is_logical = 0;
	    const char * const seqstart = RExC_parse;
            bool has_use_defaults = FALSE;

	    RExC_parse++;
	    paren = *RExC_parse++;
	    ret = NULL;			/* For look-ahead/behind. */
	    switch (paren) {

	    case 'P':	/* (?P...) variants for those used to PCRE/Python */
	        paren = *RExC_parse++;
		if ( paren == '<')         /* (?P<...>) named capture */
		    goto named_capture;
                else if (paren == '>') {   /* (?P>name) named recursion */
                    goto named_recursion;
                }
                else if (paren == '=') {   /* (?P=...)  named backref */
                    /* this pretty much dupes the code for \k<NAME> in regatom(), if
                       you change this make sure you change that */
                    char* name_start = RExC_parse;
		    U32 num = 0;
                    SV *sv_dat = reg_scan_name(pRExC_state,
                        SIZE_ONLY ? REG_RSN_RETURN_NULL : REG_RSN_RETURN_DATA);
                    if (RExC_parse == name_start || *RExC_parse != ')')
                        vFAIL2("Sequence %.3s... not terminated",parse_start);

                    if (!SIZE_ONLY) {
                        num = add_data( pRExC_state, 1, "S" );
                        RExC_rxi->data->data[num]=(void*)sv_dat;
                        SvREFCNT_inc_simple_void(sv_dat);
                    }
                    RExC_sawback = 1;
		    ret = reganode(pRExC_state,
				   ((! FOLD)
				     ? NREF
				     : (MORE_ASCII_RESTRICTED)
				       ? NREFFA
                                       : (AT_LEAST_UNI_SEMANTICS)
                                         ? NREFFU
                                         : (LOC)
                                           ? NREFFL
                                           : NREFF),
				    num);
                    *flagp |= HASWIDTH;

                    Set_Node_Offset(ret, parse_start+1);
                    Set_Node_Cur_Length(ret); /* MJD */

                    nextchar(pRExC_state);
                    return ret;
                }
                RExC_parse++;
		vFAIL3("Sequence (%.*s...) not recognized", RExC_parse-seqstart, seqstart);
		/*NOTREACHED*/
            case '<':           /* (?<...) */
		if (*RExC_parse == '!')
		    paren = ',';
		else if (*RExC_parse != '=') 
              named_capture:
		{               /* (?<...>) */
		    char *name_start;
		    SV *svname;
		    paren= '>';
            case '\'':          /* (?'...') */
    		    name_start= RExC_parse;
    		    svname = reg_scan_name(pRExC_state,
    		        SIZE_ONLY ?  /* reverse test from the others */
    		        REG_RSN_RETURN_NAME : 
    		        REG_RSN_RETURN_NULL);
		    if (RExC_parse == name_start) {
		        RExC_parse++;
		        vFAIL3("Sequence (%.*s...) not recognized", RExC_parse-seqstart, seqstart);
		        /*NOTREACHED*/
                    }
		    if (*RExC_parse != paren)
		        vFAIL2("Sequence (?%c... not terminated",
		            paren=='>' ? '<' : paren);
		    if (SIZE_ONLY) {
			HE *he_str;
			SV *sv_dat = NULL;
                        if (!svname) /* shouldn't happen */
                            Perl_croak(aTHX_
                                "panic: reg_scan_name returned NULL");
                        if (!RExC_paren_names) {
                            RExC_paren_names= newHV();
                            sv_2mortal(MUTABLE_SV(RExC_paren_names));
#ifdef DEBUGGING
                            RExC_paren_name_list= newAV();
                            sv_2mortal(MUTABLE_SV(RExC_paren_name_list));
#endif
                        }
                        he_str = hv_fetch_ent( RExC_paren_names, svname, 1, 0 );
                        if ( he_str )
                            sv_dat = HeVAL(he_str);
                        if ( ! sv_dat ) {
                            /* croak baby croak */
                            Perl_croak(aTHX_
                                "panic: paren_name hash element allocation failed");
                        } else if ( SvPOK(sv_dat) ) {
                            /* (?|...) can mean we have dupes so scan to check
                               its already been stored. Maybe a flag indicating
                               we are inside such a construct would be useful,
                               but the arrays are likely to be quite small, so
                               for now we punt -- dmq */
                            IV count = SvIV(sv_dat);
                            I32 *pv = (I32*)SvPVX(sv_dat);
                            IV i;
                            for ( i = 0 ; i < count ; i++ ) {
                                if ( pv[i] == RExC_npar ) {
                                    count = 0;
                                    break;
                                }
                            }
                            if ( count ) {
                                pv = (I32*)SvGROW(sv_dat, SvCUR(sv_dat) + sizeof(I32)+1);
                                SvCUR_set(sv_dat, SvCUR(sv_dat) + sizeof(I32));
                                pv[count] = RExC_npar;
                                SvIV_set(sv_dat, SvIVX(sv_dat) + 1);
                            }
                        } else {
                            (void)SvUPGRADE(sv_dat,SVt_PVNV);
                            sv_setpvn(sv_dat, (char *)&(RExC_npar), sizeof(I32));
                            SvIOK_on(sv_dat);
                            SvIV_set(sv_dat, 1);
                        }
#ifdef DEBUGGING
			/* Yes this does cause a memory leak in debugging Perls */
                        if (!av_store(RExC_paren_name_list, RExC_npar, SvREFCNT_inc(svname)))
                            SvREFCNT_dec(svname);
#endif

                        /*sv_dump(sv_dat);*/
                    }
                    nextchar(pRExC_state);
		    paren = 1;
		    goto capturing_parens;
		}
                RExC_seen |= REG_SEEN_LOOKBEHIND;
		RExC_in_lookbehind++;
		RExC_parse++;
	    case '=':           /* (?=...) */
		RExC_seen_zerolen++;
                break;
	    case '!':           /* (?!...) */
		RExC_seen_zerolen++;
	        if (*RExC_parse == ')') {
	            ret=reg_node(pRExC_state, OPFAIL);
	            nextchar(pRExC_state);
	            return ret;
	        }
	        break;
	    case '|':           /* (?|...) */
	        /* branch reset, behave like a (?:...) except that
	           buffers in alternations share the same numbers */
	        paren = ':'; 
	        after_freeze = freeze_paren = RExC_npar;
	        break;
	    case ':':           /* (?:...) */
	    case '>':           /* (?>...) */
		break;
	    case '$':           /* (?$...) */
	    case '@':           /* (?@...) */
		vFAIL2("Sequence (?%c...) not implemented", (int)paren);
		break;
	    case '#':           /* (?#...) */
		while (*RExC_parse && *RExC_parse != ')')
		    RExC_parse++;
		if (*RExC_parse != ')')
		    FAIL("Sequence (?#... not terminated");
		nextchar(pRExC_state);
		*flagp = TRYAGAIN;
		return NULL;
	    case '0' :           /* (?0) */
	    case 'R' :           /* (?R) */
		if (*RExC_parse != ')')
		    FAIL("Sequence (?R) not terminated");
		ret = reg_node(pRExC_state, GOSTART);
		*flagp |= POSTPONED;
		nextchar(pRExC_state);
		return ret;
		/*notreached*/
            { /* named and numeric backreferences */
                I32 num;
            case '&':            /* (?&NAME) */
                parse_start = RExC_parse - 1;
              named_recursion:
                {
    		    SV *sv_dat = reg_scan_name(pRExC_state,
    		        SIZE_ONLY ? REG_RSN_RETURN_NULL : REG_RSN_RETURN_DATA);
    		     num = sv_dat ? *((I32 *)SvPVX(sv_dat)) : 0;
                }
                goto gen_recurse_regop;
                /* NOT REACHED */
            case '+':
                if (!(RExC_parse[0] >= '1' && RExC_parse[0] <= '9')) {
                    RExC_parse++;
                    vFAIL("Illegal pattern");
                }
                goto parse_recursion;
                /* NOT REACHED*/
            case '-': /* (?-1) */
                if (!(RExC_parse[0] >= '1' && RExC_parse[0] <= '9')) {
                    RExC_parse--; /* rewind to let it be handled later */
                    goto parse_flags;
                } 
                /*FALLTHROUGH */
            case '1': case '2': case '3': case '4': /* (?1) */
	    case '5': case '6': case '7': case '8': case '9':
	        RExC_parse--;
              parse_recursion:
		num = atoi(RExC_parse);
  	        parse_start = RExC_parse - 1; /* MJD */
	        if (*RExC_parse == '-')
	            RExC_parse++;
		while (isDIGIT(*RExC_parse))
			RExC_parse++;
	        if (*RExC_parse!=')') 
	            vFAIL("Expecting close bracket");

              gen_recurse_regop:
                if ( paren == '-' ) {
                    /*
                    Diagram of capture buffer numbering.
                    Top line is the normal capture buffer numbers
                    Bottom line is the negative indexing as from
                    the X (the (?-2))

                    +   1 2    3 4 5 X          6 7
                       /(a(x)y)(a(b(c(?-2)d)e)f)(g(h))/
                    -   5 4    3 2 1 X          x x

                    */
                    num = RExC_npar + num;
                    if (num < 1)  {
                        RExC_parse++;
                        vFAIL("Reference to nonexistent group");
                    }
                } else if ( paren == '+' ) {
                    num = RExC_npar + num - 1;
                }

                ret = reganode(pRExC_state, GOSUB, num);
                if (!SIZE_ONLY) {
		    if (num > (I32)RExC_rx->nparens) {
			RExC_parse++;
			vFAIL("Reference to nonexistent group");
	            }
	            ARG2L_SET( ret, RExC_recurse_count++);
                    RExC_emit++;
		    DEBUG_OPTIMISE_MORE_r(PerlIO_printf(Perl_debug_log,
			"Recurse #%"UVuf" to %"IVdf"\n", (UV)ARG(ret), (IV)ARG2L(ret)));
		} else {
		    RExC_size++;
    		}
    		RExC_seen |= REG_SEEN_RECURSE;
                Set_Node_Length(ret, 1 + regarglen[OP(ret)]); /* MJD */
		Set_Node_Offset(ret, parse_start); /* MJD */

                *flagp |= POSTPONED;
                nextchar(pRExC_state);
                return ret;
            } /* named and numeric backreferences */
            /* NOT REACHED */

	    case '?':           /* (??...) */
		is_logical = 1;
		if (*RExC_parse != '{') {
		    RExC_parse++;
		    vFAIL3("Sequence (%.*s...) not recognized", RExC_parse-seqstart, seqstart);
		    /*NOTREACHED*/
		}
		*flagp |= POSTPONED;
		paren = *RExC_parse++;
		/* FALL THROUGH */
	    case '{':           /* (?{...}) */
	    {
		I32 count = 1;
		U32 n = 0;
		char c;
		char *s = RExC_parse;

		RExC_seen_zerolen++;
		RExC_seen |= REG_SEEN_EVAL;
		while (count && (c = *RExC_parse)) {
		    if (c == '\\') {
			if (RExC_parse[1])
			    RExC_parse++;
		    }
		    else if (c == '{')
			count++;
		    else if (c == '}')
			count--;
		    RExC_parse++;
		}
		if (*RExC_parse != ')') {
		    RExC_parse = s;
		    vFAIL("Sequence (?{...}) not terminated or not {}-balanced");
		}
		if (!SIZE_ONLY) {
		    PAD *pad;
		    OP_4tree *sop, *rop;
		    SV * const sv = newSVpvn(s, RExC_parse - 1 - s);

		    ENTER;
		    Perl_save_re_context(aTHX);
		    rop = Perl_sv_compile_2op_is_broken(aTHX_ sv, &sop, "re", &pad);
		    sop->op_private |= OPpREFCOUNTED;
		    /* re_dup will OpREFCNT_inc */
		    OpREFCNT_set(sop, 1);
		    LEAVE;

		    n = add_data(pRExC_state, 3, "nop");
		    RExC_rxi->data->data[n] = (void*)rop;
		    RExC_rxi->data->data[n+1] = (void*)sop;
		    RExC_rxi->data->data[n+2] = (void*)pad;
		    SvREFCNT_dec(sv);
		}
		else {						/* First pass */
		    if (PL_reginterp_cnt < ++RExC_seen_evals
			&& IN_PERL_RUNTIME)
			/* No compiled RE interpolated, has runtime
			   components ===> unsafe.  */
			FAIL("Eval-group not allowed at runtime, use re 'eval'");
		    if (PL_tainting && PL_tainted)
			FAIL("Eval-group in insecure regular expression");
#if PERL_VERSION > 8
		    if (IN_PERL_COMPILETIME)
			PL_cv_has_eval = 1;
#endif
		}

		nextchar(pRExC_state);
		if (is_logical) {
		    ret = reg_node(pRExC_state, LOGICAL);
		    if (!SIZE_ONLY)
			ret->flags = 2;
                    REGTAIL(pRExC_state, ret, reganode(pRExC_state, EVAL, n));
                    /* deal with the length of this later - MJD */
		    return ret;
		}
		ret = reganode(pRExC_state, EVAL, n);
		Set_Node_Length(ret, RExC_parse - parse_start + 1);
		Set_Node_Offset(ret, parse_start);
		return ret;
	    }
	    case '(':           /* (?(?{...})...) and (?(?=...)...) */
	    {
	        int is_define= 0;
		if (RExC_parse[0] == '?') {        /* (?(?...)) */
		    if (RExC_parse[1] == '=' || RExC_parse[1] == '!'
			|| RExC_parse[1] == '<'
			|| RExC_parse[1] == '{') { /* Lookahead or eval. */
			I32 flag;

			ret = reg_node(pRExC_state, LOGICAL);
			if (!SIZE_ONLY)
			    ret->flags = 1;
                        REGTAIL(pRExC_state, ret, reg(pRExC_state, 1, &flag,depth+1));
			goto insert_if;
		    }
		}
		else if ( RExC_parse[0] == '<'     /* (?(<NAME>)...) */
		         || RExC_parse[0] == '\'' ) /* (?('NAME')...) */
	        {
	            char ch = RExC_parse[0] == '<' ? '>' : '\'';
	            char *name_start= RExC_parse++;
	            U32 num = 0;
	            SV *sv_dat=reg_scan_name(pRExC_state,
	                SIZE_ONLY ? REG_RSN_RETURN_NULL : REG_RSN_RETURN_DATA);
	            if (RExC_parse == name_start || *RExC_parse != ch)
                        vFAIL2("Sequence (?(%c... not terminated",
                            (ch == '>' ? '<' : ch));
                    RExC_parse++;
	            if (!SIZE_ONLY) {
                        num = add_data( pRExC_state, 1, "S" );
                        RExC_rxi->data->data[num]=(void*)sv_dat;
                        SvREFCNT_inc_simple_void(sv_dat);
                    }
                    ret = reganode(pRExC_state,NGROUPP,num);
                    goto insert_if_check_paren;
		}
		else if (RExC_parse[0] == 'D' &&
		         RExC_parse[1] == 'E' &&
		         RExC_parse[2] == 'F' &&
		         RExC_parse[3] == 'I' &&
		         RExC_parse[4] == 'N' &&
		         RExC_parse[5] == 'E')
		{
		    ret = reganode(pRExC_state,DEFINEP,0);
		    RExC_parse +=6 ;
		    is_define = 1;
		    goto insert_if_check_paren;
		}
		else if (RExC_parse[0] == 'R') {
		    RExC_parse++;
		    parno = 0;
		    if (RExC_parse[0] >= '1' && RExC_parse[0] <= '9' ) {
		        parno = atoi(RExC_parse++);
		        while (isDIGIT(*RExC_parse))
			    RExC_parse++;
		    } else if (RExC_parse[0] == '&') {
		        SV *sv_dat;
		        RExC_parse++;
		        sv_dat = reg_scan_name(pRExC_state,
    		            SIZE_ONLY ? REG_RSN_RETURN_NULL : REG_RSN_RETURN_DATA);
    		        parno = sv_dat ? *((I32 *)SvPVX(sv_dat)) : 0;
		    }
		    ret = reganode(pRExC_state,INSUBP,parno); 
		    goto insert_if_check_paren;
		}
		else if (RExC_parse[0] >= '1' && RExC_parse[0] <= '9' ) {
                    /* (?(1)...) */
		    char c;
		    parno = atoi(RExC_parse++);

		    while (isDIGIT(*RExC_parse))
			RExC_parse++;
                    ret = reganode(pRExC_state, GROUPP, parno);

                 insert_if_check_paren:
		    if ((c = *nextchar(pRExC_state)) != ')')
			vFAIL("Switch condition not recognized");
		  insert_if:
                    REGTAIL(pRExC_state, ret, reganode(pRExC_state, IFTHEN, 0));
                    br = regbranch(pRExC_state, &flags, 1,depth+1);
		    if (br == NULL)
			br = reganode(pRExC_state, LONGJMP, 0);
		    else
                        REGTAIL(pRExC_state, br, reganode(pRExC_state, LONGJMP, 0));
		    c = *nextchar(pRExC_state);
		    if (flags&HASWIDTH)
			*flagp |= HASWIDTH;
		    if (c == '|') {
		        if (is_define) 
		            vFAIL("(?(DEFINE)....) does not allow branches");
			lastbr = reganode(pRExC_state, IFTHEN, 0); /* Fake one for optimizer. */
                        regbranch(pRExC_state, &flags, 1,depth+1);
                        REGTAIL(pRExC_state, ret, lastbr);
		 	if (flags&HASWIDTH)
			    *flagp |= HASWIDTH;
			c = *nextchar(pRExC_state);
		    }
		    else
			lastbr = NULL;
		    if (c != ')')
			vFAIL("Switch (?(condition)... contains too many branches");
		    ender = reg_node(pRExC_state, TAIL);
                    REGTAIL(pRExC_state, br, ender);
		    if (lastbr) {
                        REGTAIL(pRExC_state, lastbr, ender);
                        REGTAIL(pRExC_state, NEXTOPER(NEXTOPER(lastbr)), ender);
		    }
		    else
                        REGTAIL(pRExC_state, ret, ender);
                    RExC_size++; /* XXX WHY do we need this?!!
                                    For large programs it seems to be required
                                    but I can't figure out why. -- dmq*/
		    return ret;
		}
		else {
		    vFAIL2("Unknown switch condition (?(%.2s", RExC_parse);
		}
	    }
            case 0:
		RExC_parse--; /* for vFAIL to print correctly */
                vFAIL("Sequence (? incomplete");
                break;
            case DEFAULT_PAT_MOD:   /* Use default flags with the exceptions
				       that follow */
                has_use_defaults = TRUE;
                STD_PMMOD_FLAGS_CLEAR(&RExC_flags);
		set_regex_charset(&RExC_flags, (RExC_utf8 || RExC_uni_semantics)
						? REGEX_UNICODE_CHARSET
						: REGEX_DEPENDS_CHARSET);
                goto parse_flags;
	    default:
	        --RExC_parse;
	        parse_flags:      /* (?i) */  
	    {
                U32 posflags = 0, negflags = 0;
	        U32 *flagsp = &posflags;
                char has_charset_modifier = '\0';
		regex_charset cs = get_regex_charset(RExC_flags);
		if (cs == REGEX_DEPENDS_CHARSET
		    && (RExC_utf8 || RExC_uni_semantics))
		{
		    cs = REGEX_UNICODE_CHARSET;
		}

		while (*RExC_parse) {
		    /* && strchr("iogcmsx", *RExC_parse) */
		    /* (?g), (?gc) and (?o) are useless here
		       and must be globally applied -- japhy */
                    switch (*RExC_parse) {
	            CASE_STD_PMMOD_FLAGS_PARSE_SET(flagsp);
                    case LOCALE_PAT_MOD:
                        if (has_charset_modifier) {
			    goto excess_modifier;
			}
			else if (flagsp == &negflags) {
                            goto neg_modifier;
                        }
			cs = REGEX_LOCALE_CHARSET;
                        has_charset_modifier = LOCALE_PAT_MOD;
			RExC_contains_locale = 1;
                        break;
                    case UNICODE_PAT_MOD:
                        if (has_charset_modifier) {
			    goto excess_modifier;
			}
			else if (flagsp == &negflags) {
                            goto neg_modifier;
                        }
			cs = REGEX_UNICODE_CHARSET;
                        has_charset_modifier = UNICODE_PAT_MOD;
                        break;
                    case ASCII_RESTRICT_PAT_MOD:
                        if (flagsp == &negflags) {
                            goto neg_modifier;
                        }
                        if (has_charset_modifier) {
                            if (cs != REGEX_ASCII_RESTRICTED_CHARSET) {
                                goto excess_modifier;
                            }
			    /* Doubled modifier implies more restricted */
                            cs = REGEX_ASCII_MORE_RESTRICTED_CHARSET;
                        }
			else {
			    cs = REGEX_ASCII_RESTRICTED_CHARSET;
			}
                        has_charset_modifier = ASCII_RESTRICT_PAT_MOD;
                        break;
                    case DEPENDS_PAT_MOD:
                        if (has_use_defaults) {
                            goto fail_modifiers;
			}
			else if (flagsp == &negflags) {
                            goto neg_modifier;
			}
			else if (has_charset_modifier) {
			    goto excess_modifier;
                        }

			/* The dual charset means unicode semantics if the
			 * pattern (or target, not known until runtime) are
			 * utf8, or something in the pattern indicates unicode
			 * semantics */
			cs = (RExC_utf8 || RExC_uni_semantics)
			     ? REGEX_UNICODE_CHARSET
			     : REGEX_DEPENDS_CHARSET;
                        has_charset_modifier = DEPENDS_PAT_MOD;
                        break;
		    excess_modifier:
			RExC_parse++;
			if (has_charset_modifier == ASCII_RESTRICT_PAT_MOD) {
			    vFAIL2("Regexp modifier \"%c\" may appear a maximum of twice", ASCII_RESTRICT_PAT_MOD);
			}
			else if (has_charset_modifier == *(RExC_parse - 1)) {
			    vFAIL2("Regexp modifier \"%c\" may not appear twice", *(RExC_parse - 1));
			}
			else {
			    vFAIL3("Regexp modifiers \"%c\" and \"%c\" are mutually exclusive", has_charset_modifier, *(RExC_parse - 1));
			}
			/*NOTREACHED*/
		    neg_modifier:
			RExC_parse++;
			vFAIL2("Regexp modifier \"%c\" may not appear after the \"-\"", *(RExC_parse - 1));
			/*NOTREACHED*/
                    case ONCE_PAT_MOD: /* 'o' */
                    case GLOBAL_PAT_MOD: /* 'g' */
			if (SIZE_ONLY && ckWARN(WARN_REGEXP)) {
			    const I32 wflagbit = *RExC_parse == 'o' ? WASTED_O : WASTED_G;
			    if (! (wastedflags & wflagbit) ) {
				wastedflags |= wflagbit;
				vWARN5(
				    RExC_parse + 1,
				    "Useless (%s%c) - %suse /%c modifier",
				    flagsp == &negflags ? "?-" : "?",
				    *RExC_parse,
				    flagsp == &negflags ? "don't " : "",
				    *RExC_parse
				);
			    }
			}
			break;
		        
		    case CONTINUE_PAT_MOD: /* 'c' */
			if (SIZE_ONLY && ckWARN(WARN_REGEXP)) {
			    if (! (wastedflags & WASTED_C) ) {
				wastedflags |= WASTED_GC;
				vWARN3(
				    RExC_parse + 1,
				    "Useless (%sc) - %suse /gc modifier",
				    flagsp == &negflags ? "?-" : "?",
				    flagsp == &negflags ? "don't " : ""
				);
			    }
			}
			break;
	            case KEEPCOPY_PAT_MOD: /* 'p' */
                        if (flagsp == &negflags) {
                            if (SIZE_ONLY)
                                ckWARNreg(RExC_parse + 1,"Useless use of (?-p)");
                        } else {
                            *flagsp |= RXf_PMf_KEEPCOPY;
                        }
	                break;
                    case '-':
                        /* A flag is a default iff it is following a minus, so
                         * if there is a minus, it means will be trying to
                         * re-specify a default which is an error */
                        if (has_use_defaults || flagsp == &negflags) {
            fail_modifiers:
                            RExC_parse++;
		            vFAIL3("Sequence (%.*s...) not recognized", RExC_parse-seqstart, seqstart);
		            /*NOTREACHED*/
		        }
			flagsp = &negflags;
		        wastedflags = 0;  /* reset so (?g-c) warns twice */
		        break;
                    case ':':
		        paren = ':';
		        /*FALLTHROUGH*/
                    case ')':
                        RExC_flags |= posflags;
                        RExC_flags &= ~negflags;
			set_regex_charset(&RExC_flags, cs);
                        if (paren != ':') {
                            oregflags |= posflags;
                            oregflags &= ~negflags;
			    set_regex_charset(&oregflags, cs);
                        }
                        nextchar(pRExC_state);
		        if (paren != ':') {
		            *flagp = TRYAGAIN;
		            return NULL;
		        } else {
                            ret = NULL;
		            goto parse_rest;
		        }
		        /*NOTREACHED*/
                    default:
		        RExC_parse++;
		        vFAIL3("Sequence (%.*s...) not recognized", RExC_parse-seqstart, seqstart);
		        /*NOTREACHED*/
                    }                           
		    ++RExC_parse;
		}
	    }} /* one for the default block, one for the switch */
	}
	else {                  /* (...) */
	  capturing_parens:
	    parno = RExC_npar;
	    RExC_npar++;
	    
	    ret = reganode(pRExC_state, OPEN, parno);
	    if (!SIZE_ONLY ){
	        if (!RExC_nestroot) 
	            RExC_nestroot = parno;
	        if (RExC_seen & REG_SEEN_RECURSE
	            && !RExC_open_parens[parno-1])
	        {
		    DEBUG_OPTIMISE_MORE_r(PerlIO_printf(Perl_debug_log,
			"Setting open paren #%"IVdf" to %d\n", 
			(IV)parno, REG_NODE_NUM(ret)));
	            RExC_open_parens[parno-1]= ret;
	        }
	    }
            Set_Node_Length(ret, 1); /* MJD */
            Set_Node_Offset(ret, RExC_parse); /* MJD */
	    is_open = 1;
	}
    }
    else                        /* ! paren */
	ret = NULL;
   
   parse_rest:
    /* Pick up the branches, linking them together. */
    parse_start = RExC_parse;   /* MJD */
    br = regbranch(pRExC_state, &flags, 1,depth+1);

    /*     branch_len = (paren != 0); */

    if (br == NULL)
	return(NULL);
    if (*RExC_parse == '|') {
	if (!SIZE_ONLY && RExC_extralen) {
	    reginsert(pRExC_state, BRANCHJ, br, depth+1);
	}
	else {                  /* MJD */
	    reginsert(pRExC_state, BRANCH, br, depth+1);
            Set_Node_Length(br, paren != 0);
            Set_Node_Offset_To_R(br-RExC_emit_start, parse_start-RExC_start);
        }
	have_branch = 1;
	if (SIZE_ONLY)
	    RExC_extralen += 1;		/* For BRANCHJ-BRANCH. */
    }
    else if (paren == ':') {
	*flagp |= flags&SIMPLE;
    }
    if (is_open) {				/* Starts with OPEN. */
        REGTAIL(pRExC_state, ret, br);          /* OPEN -> first. */
    }
    else if (paren != '?')		/* Not Conditional */
	ret = br;
    *flagp |= flags & (SPSTART | HASWIDTH | POSTPONED);
    lastbr = br;
    while (*RExC_parse == '|') {
	if (!SIZE_ONLY && RExC_extralen) {
	    ender = reganode(pRExC_state, LONGJMP,0);
            REGTAIL(pRExC_state, NEXTOPER(NEXTOPER(lastbr)), ender); /* Append to the previous. */
	}
	if (SIZE_ONLY)
	    RExC_extralen += 2;		/* Account for LONGJMP. */
	nextchar(pRExC_state);
	if (freeze_paren) {
	    if (RExC_npar > after_freeze)
	        after_freeze = RExC_npar;
            RExC_npar = freeze_paren;	    
        }
        br = regbranch(pRExC_state, &flags, 0, depth+1);

	if (br == NULL)
	    return(NULL);
        REGTAIL(pRExC_state, lastbr, br);               /* BRANCH -> BRANCH. */
	lastbr = br;
	*flagp |= flags & (SPSTART | HASWIDTH | POSTPONED);
    }

    if (have_branch || paren != ':') {
	/* Make a closing node, and hook it on the end. */
	switch (paren) {
	case ':':
	    ender = reg_node(pRExC_state, TAIL);
	    break;
	case 1:
	    ender = reganode(pRExC_state, CLOSE, parno);
	    if (!SIZE_ONLY && RExC_seen & REG_SEEN_RECURSE) {
		DEBUG_OPTIMISE_MORE_r(PerlIO_printf(Perl_debug_log,
			"Setting close paren #%"IVdf" to %d\n", 
			(IV)parno, REG_NODE_NUM(ender)));
	        RExC_close_parens[parno-1]= ender;
	        if (RExC_nestroot == parno) 
	            RExC_nestroot = 0;
	    }	    
            Set_Node_Offset(ender,RExC_parse+1); /* MJD */
            Set_Node_Length(ender,1); /* MJD */
	    break;
	case '<':
	case ',':
	case '=':
	case '!':
	    *flagp &= ~HASWIDTH;
	    /* FALL THROUGH */
	case '>':
	    ender = reg_node(pRExC_state, SUCCEED);
	    break;
	case 0:
	    ender = reg_node(pRExC_state, END);
	    if (!SIZE_ONLY) {
                assert(!RExC_opend); /* there can only be one! */
                RExC_opend = ender;
            }
	    break;
	}
        REGTAIL(pRExC_state, lastbr, ender);

	if (have_branch && !SIZE_ONLY) {
	    if (depth==1)
	        RExC_seen |= REG_TOP_LEVEL_BRANCHES;

	    /* Hook the tails of the branches to the closing node. */
	    for (br = ret; br; br = regnext(br)) {
		const U8 op = PL_regkind[OP(br)];
		if (op == BRANCH) {
                    REGTAIL_STUDY(pRExC_state, NEXTOPER(br), ender);
		}
		else if (op == BRANCHJ) {
                    REGTAIL_STUDY(pRExC_state, NEXTOPER(NEXTOPER(br)), ender);
		}
	    }
	}
    }

    {
        const char *p;
        static const char parens[] = "=!<,>";

	if (paren && (p = strchr(parens, paren))) {
	    U8 node = ((p - parens) % 2) ? UNLESSM : IFMATCH;
	    int flag = (p - parens) > 1;

	    if (paren == '>')
		node = SUSPEND, flag = 0;
	    reginsert(pRExC_state, node,ret, depth+1);
	    Set_Node_Cur_Length(ret);
	    Set_Node_Offset(ret, parse_start + 1);
	    ret->flags = flag;
            REGTAIL_STUDY(pRExC_state, ret, reg_node(pRExC_state, TAIL));
	}
    }

    /* Check for proper termination. */
    if (paren) {
	RExC_flags = oregflags;
	if (RExC_parse >= RExC_end || *nextchar(pRExC_state) != ')') {
	    RExC_parse = oregcomp_parse;
	    vFAIL("Unmatched (");
	}
    }
    else if (!paren && RExC_parse < RExC_end) {
	if (*RExC_parse == ')') {
	    RExC_parse++;
	    vFAIL("Unmatched )");
	}
	else
	    FAIL("Junk on end of regexp");	/* "Can't happen". */
	/* NOTREACHED */
    }

    if (RExC_in_lookbehind) {
	RExC_in_lookbehind--;
    }
    if (after_freeze > RExC_npar)
        RExC_npar = after_freeze;
    return(ret);
}

/*
 - regbranch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 */
STATIC regnode *
S_regbranch(pTHX_ RExC_state_t *pRExC_state, I32 *flagp, I32 first, U32 depth)
{
    dVAR;
    register regnode *ret;
    register regnode *chain = NULL;
    register regnode *latest;
    I32 flags = 0, c = 0;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGBRANCH;

    DEBUG_PARSE("brnc");

    if (first)
	ret = NULL;
    else {
	if (!SIZE_ONLY && RExC_extralen)
	    ret = reganode(pRExC_state, BRANCHJ,0);
	else {
	    ret = reg_node(pRExC_state, BRANCH);
            Set_Node_Length(ret, 1);
        }
    }

    if (!first && SIZE_ONLY)
	RExC_extralen += 1;			/* BRANCHJ */

    *flagp = WORST;			/* Tentatively. */

    RExC_parse--;
    nextchar(pRExC_state);
    while (RExC_parse < RExC_end && *RExC_parse != '|' && *RExC_parse != ')') {
	flags &= ~TRYAGAIN;
        latest = regpiece(pRExC_state, &flags,depth+1);
	if (latest == NULL) {
	    if (flags & TRYAGAIN)
		continue;
	    return(NULL);
	}
	else if (ret == NULL)
	    ret = latest;
	*flagp |= flags&(HASWIDTH|POSTPONED);
	if (chain == NULL) 	/* First piece. */
	    *flagp |= flags&SPSTART;
	else {
	    RExC_naughty++;
            REGTAIL(pRExC_state, chain, latest);
	}
	chain = latest;
	c++;
    }
    if (chain == NULL) {	/* Loop ran zero times. */
	chain = reg_node(pRExC_state, NOTHING);
	if (ret == NULL)
	    ret = chain;
    }
    if (c == 1) {
	*flagp |= flags&SIMPLE;
    }

    return ret;
}

/*
 - regpiece - something followed by possible [*+?]
 *
 * Note that the branching code sequences used for ? and the general cases
 * of * and + are somewhat optimized:  they use the same NOTHING node as
 * both the endmarker for their branch list and the body of the last branch.
 * It might seem that this node could be dispensed with entirely, but the
 * endmarker role is not redundant.
 */
STATIC regnode *
S_regpiece(pTHX_ RExC_state_t *pRExC_state, I32 *flagp, U32 depth)
{
    dVAR;
    register regnode *ret;
    register char op;
    register char *next;
    I32 flags;
    const char * const origparse = RExC_parse;
    I32 min;
    I32 max = REG_INFTY;
#ifdef RE_TRACK_PATTERN_OFFSETS
    char *parse_start;
#endif
    const char *maxpos = NULL;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGPIECE;

    DEBUG_PARSE("piec");

    ret = regatom(pRExC_state, &flags,depth+1);
    if (ret == NULL) {
	if (flags & TRYAGAIN)
	    *flagp |= TRYAGAIN;
	return(NULL);
    }

    op = *RExC_parse;

    if (op == '{' && regcurly(RExC_parse)) {
	maxpos = NULL;
#ifdef RE_TRACK_PATTERN_OFFSETS
        parse_start = RExC_parse; /* MJD */
#endif
	next = RExC_parse + 1;
	while (isDIGIT(*next) || *next == ',') {
	    if (*next == ',') {
		if (maxpos)
		    break;
		else
		    maxpos = next;
	    }
	    next++;
	}
	if (*next == '}') {		/* got one */
	    if (!maxpos)
		maxpos = next;
	    RExC_parse++;
	    min = atoi(RExC_parse);
	    if (*maxpos == ',')
		maxpos++;
	    else
		maxpos = RExC_parse;
	    max = atoi(maxpos);
	    if (!max && *maxpos != '0')
		max = REG_INFTY;		/* meaning "infinity" */
	    else if (max >= REG_INFTY)
		vFAIL2("Quantifier in {,} bigger than %d", REG_INFTY - 1);
	    RExC_parse = next;
	    nextchar(pRExC_state);

	do_curly:
	    if ((flags&SIMPLE)) {
		RExC_naughty += 2 + RExC_naughty / 2;
		reginsert(pRExC_state, CURLY, ret, depth+1);
                Set_Node_Offset(ret, parse_start+1); /* MJD */
                Set_Node_Cur_Length(ret);
	    }
	    else {
		regnode * const w = reg_node(pRExC_state, WHILEM);

		w->flags = 0;
                REGTAIL(pRExC_state, ret, w);
		if (!SIZE_ONLY && RExC_extralen) {
		    reginsert(pRExC_state, LONGJMP,ret, depth+1);
		    reginsert(pRExC_state, NOTHING,ret, depth+1);
		    NEXT_OFF(ret) = 3;	/* Go over LONGJMP. */
		}
		reginsert(pRExC_state, CURLYX,ret, depth+1);
                                /* MJD hk */
                Set_Node_Offset(ret, parse_start+1);
                Set_Node_Length(ret,
                                op == '{' ? (RExC_parse - parse_start) : 1);

		if (!SIZE_ONLY && RExC_extralen)
		    NEXT_OFF(ret) = 3;	/* Go over NOTHING to LONGJMP. */
                REGTAIL(pRExC_state, ret, reg_node(pRExC_state, NOTHING));
		if (SIZE_ONLY)
		    RExC_whilem_seen++, RExC_extralen += 3;
		RExC_naughty += 4 + RExC_naughty;	/* compound interest */
	    }
	    ret->flags = 0;

	    if (min > 0)
		*flagp = WORST;
	    if (max > 0)
		*flagp |= HASWIDTH;
	    if (max < min)
		vFAIL("Can't do {n,m} with n > m");
	    if (!SIZE_ONLY) {
		ARG1_SET(ret, (U16)min);
		ARG2_SET(ret, (U16)max);
	    }

	    goto nest_check;
	}
    }

    if (!ISMULT1(op)) {
	*flagp = flags;
	return(ret);
    }

#if 0				/* Now runtime fix should be reliable. */

    /* if this is reinstated, don't forget to put this back into perldiag:

	    =item Regexp *+ operand could be empty at {#} in regex m/%s/

	   (F) The part of the regexp subject to either the * or + quantifier
           could match an empty string. The {#} shows in the regular
           expression about where the problem was discovered.

    */

    if (!(flags&HASWIDTH) && op != '?')
      vFAIL("Regexp *+ operand could be empty");
#endif

#ifdef RE_TRACK_PATTERN_OFFSETS
    parse_start = RExC_parse;
#endif
    nextchar(pRExC_state);

    *flagp = (op != '+') ? (WORST|SPSTART|HASWIDTH) : (WORST|HASWIDTH);

    if (op == '*' && (flags&SIMPLE)) {
	reginsert(pRExC_state, STAR, ret, depth+1);
	ret->flags = 0;
	RExC_naughty += 4;
    }
    else if (op == '*') {
	min = 0;
	goto do_curly;
    }
    else if (op == '+' && (flags&SIMPLE)) {
	reginsert(pRExC_state, PLUS, ret, depth+1);
	ret->flags = 0;
	RExC_naughty += 3;
    }
    else if (op == '+') {
	min = 1;
	goto do_curly;
    }
    else if (op == '?') {
	min = 0; max = 1;
	goto do_curly;
    }
  nest_check:
    if (!SIZE_ONLY && !(flags&(HASWIDTH|POSTPONED)) && max > REG_INFTY/3) {
	ckWARN3reg(RExC_parse,
		   "%.*s matches null string many times",
		   (int)(RExC_parse >= origparse ? RExC_parse - origparse : 0),
		   origparse);
    }

    if (RExC_parse < RExC_end && *RExC_parse == '?') {
	nextchar(pRExC_state);
	reginsert(pRExC_state, MINMOD, ret, depth+1);
        REGTAIL(pRExC_state, ret, ret + NODE_STEP_REGNODE);
    }
#ifndef REG_ALLOW_MINMOD_SUSPEND
    else
#endif
    if (RExC_parse < RExC_end && *RExC_parse == '+') {
        regnode *ender;
        nextchar(pRExC_state);
        ender = reg_node(pRExC_state, SUCCEED);
        REGTAIL(pRExC_state, ret, ender);
        reginsert(pRExC_state, SUSPEND, ret, depth+1);
        ret->flags = 0;
        ender = reg_node(pRExC_state, TAIL);
        REGTAIL(pRExC_state, ret, ender);
        /*ret= ender;*/
    }

    if (RExC_parse < RExC_end && ISMULT2(RExC_parse)) {
	RExC_parse++;
	vFAIL("Nested quantifiers");
    }

    return(ret);
}


/* reg_namedseq(pRExC_state,UVp, UV depth)
   
   This is expected to be called by a parser routine that has 
   recognized '\N' and needs to handle the rest. RExC_parse is
   expected to point at the first char following the N at the time
   of the call.

   The \N may be inside (indicated by valuep not being NULL) or outside a
   character class.

   \N may begin either a named sequence, or if outside a character class, mean
   to match a non-newline.  For non single-quoted regexes, the tokenizer has
   attempted to decide which, and in the case of a named sequence converted it
   into one of the forms: \N{} (if the sequence is null), or \N{U+c1.c2...},
   where c1... are the characters in the sequence.  For single-quoted regexes,
   the tokenizer passes the \N sequence through unchanged; this code will not
   attempt to determine this nor expand those.  The net effect is that if the
   beginning of the passed-in pattern isn't '{U+' or there is no '}', it
   signals that this \N occurrence means to match a non-newline.
   
   Only the \N{U+...} form should occur in a character class, for the same
   reason that '.' inside a character class means to just match a period: it
   just doesn't make sense.
   
   If valuep is non-null then it is assumed that we are parsing inside 
   of a charclass definition and the first codepoint in the resolved
   string is returned via *valuep and the routine will return NULL. 
   In this mode if a multichar string is returned from the charnames 
   handler, a warning will be issued, and only the first char in the 
   sequence will be examined. If the string returned is zero length
   then the value of *valuep is undefined and NON-NULL will 
   be returned to indicate failure. (This will NOT be a valid pointer 
   to a regnode.)
   
   If valuep is null then it is assumed that we are parsing normal text and a
   new EXACT node is inserted into the program containing the resolved string,
   and a pointer to the new node is returned.  But if the string is zero length
   a NOTHING node is emitted instead.

   On success RExC_parse is set to the char following the endbrace.
   Parsing failures will generate a fatal error via vFAIL(...)
 */
STATIC regnode *
S_reg_namedseq(pTHX_ RExC_state_t *pRExC_state, UV *valuep, I32 *flagp, U32 depth)
{
    char * endbrace;    /* '}' following the name */
    regnode *ret = NULL;
    char* p;

    GET_RE_DEBUG_FLAGS_DECL;
 
    PERL_ARGS_ASSERT_REG_NAMEDSEQ;

    GET_RE_DEBUG_FLAGS;

    /* The [^\n] meaning of \N ignores spaces and comments under the /x
     * modifier.  The other meaning does not */
    p = (RExC_flags & RXf_PMf_EXTENDED)
	? regwhite( pRExC_state, RExC_parse )
	: RExC_parse;
   
    /* Disambiguate between \N meaning a named character versus \N meaning
     * [^\n].  The former is assumed when it can't be the latter. */
    if (*p != '{' || regcurly(p)) {
	RExC_parse = p;
	if (valuep) {
	    /* no bare \N in a charclass */
	    vFAIL("\\N in a character class must be a named character: \\N{...}");
	}
	nextchar(pRExC_state);
	ret = reg_node(pRExC_state, REG_ANY);
	*flagp |= HASWIDTH|SIMPLE;
	RExC_naughty++;
	RExC_parse--;
        Set_Node_Length(ret, 1); /* MJD */
	return ret;
    }

    /* Here, we have decided it should be a named sequence */

    /* The test above made sure that the next real character is a '{', but
     * under the /x modifier, it could be separated by space (or a comment and
     * \n) and this is not allowed (for consistency with \x{...} and the
     * tokenizer handling of \N{NAME}). */
    if (*RExC_parse != '{') {
	vFAIL("Missing braces on \\N{}");
    }

    RExC_parse++;	/* Skip past the '{' */

    if (! (endbrace = strchr(RExC_parse, '}')) /* no trailing brace */
	|| ! (endbrace == RExC_parse		/* nothing between the {} */
	      || (endbrace - RExC_parse >= 2	/* U+ (bad hex is checked below */
		  && strnEQ(RExC_parse, "U+", 2)))) /* for a better error msg) */
    {
	if (endbrace) RExC_parse = endbrace;	/* position msg's '<--HERE' */
	vFAIL("\\N{NAME} must be resolved by the lexer");
    }

    if (endbrace == RExC_parse) {   /* empty: \N{} */
	if (! valuep) {
	    RExC_parse = endbrace + 1;  
	    return reg_node(pRExC_state,NOTHING);
	}

	if (SIZE_ONLY) {
	    ckWARNreg(RExC_parse,
		    "Ignoring zero length \\N{} in character class"
	    );
	    RExC_parse = endbrace + 1;  
	}
	*valuep = 0;
	return (regnode *) &RExC_parse; /* Invalid regnode pointer */
    }

    REQUIRE_UTF8;	/* named sequences imply Unicode semantics */
    RExC_parse += 2;	/* Skip past the 'U+' */

    if (valuep) {   /* In a bracketed char class */
	/* We only pay attention to the first char of 
	multichar strings being returned. I kinda wonder
	if this makes sense as it does change the behaviour
	from earlier versions, OTOH that behaviour was broken
	as well. XXX Solution is to recharacterize as
	[rest-of-class]|multi1|multi2... */

	STRLEN length_of_hex;
	I32 flags = PERL_SCAN_ALLOW_UNDERSCORES
	    | PERL_SCAN_DISALLOW_PREFIX
	    | (SIZE_ONLY ? PERL_SCAN_SILENT_ILLDIGIT : 0);
    
	char * endchar = RExC_parse + strcspn(RExC_parse, ".}");
	if (endchar < endbrace) {
	    ckWARNreg(endchar, "Using just the first character returned by \\N{} in character class");
	}

	length_of_hex = (STRLEN)(endchar - RExC_parse);
	*valuep = grok_hex(RExC_parse, &length_of_hex, &flags, NULL);

	/* The tokenizer should have guaranteed validity, but it's possible to
	 * bypass it by using single quoting, so check */
	if (length_of_hex == 0
	    || length_of_hex != (STRLEN)(endchar - RExC_parse) )
	{
	    RExC_parse += length_of_hex;	/* Includes all the valid */
	    RExC_parse += (RExC_orig_utf8)	/* point to after 1st invalid */
			    ? UTF8SKIP(RExC_parse)
			    : 1;
	    /* Guard against malformed utf8 */
	    if (RExC_parse >= endchar) RExC_parse = endchar;
	    vFAIL("Invalid hexadecimal number in \\N{U+...}");
	}    

	RExC_parse = endbrace + 1;
	if (endchar == endbrace) return NULL;

        ret = (regnode *) &RExC_parse;	/* Invalid regnode pointer */
    }
    else {	/* Not a char class */

	/* What is done here is to convert this to a sub-pattern of the form
	 * (?:\x{char1}\x{char2}...)
	 * and then call reg recursively.  That way, it retains its atomicness,
	 * while not having to worry about special handling that some code
	 * points may have.  toke.c has converted the original Unicode values
	 * to native, so that we can just pass on the hex values unchanged.  We
	 * do have to set a flag to keep recoding from happening in the
	 * recursion */

	SV * substitute_parse = newSVpvn_flags("?:", 2, SVf_UTF8|SVs_TEMP);
	STRLEN len;
	char *endchar;	    /* Points to '.' or '}' ending cur char in the input
			       stream */
	char *orig_end = RExC_end;

	while (RExC_parse < endbrace) {

	    /* Code points are separated by dots.  If none, there is only one
	     * code point, and is terminated by the brace */
	    endchar = RExC_parse + strcspn(RExC_parse, ".}");

	    /* Convert to notation the rest of the code understands */
	    sv_catpv(substitute_parse, "\\x{");
	    sv_catpvn(substitute_parse, RExC_parse, endchar - RExC_parse);
	    sv_catpv(substitute_parse, "}");

	    /* Point to the beginning of the next character in the sequence. */
	    RExC_parse = endchar + 1;
	}
	sv_catpv(substitute_parse, ")");

	RExC_parse = SvPV(substitute_parse, len);

	/* Don't allow empty number */
	if (len < 8) {
	    vFAIL("Invalid hexadecimal number in \\N{U+...}");
	}
	RExC_end = RExC_parse + len;

	/* The values are Unicode, and therefore not subject to recoding */
	RExC_override_recoding = 1;

	ret = reg(pRExC_state, 1, flagp, depth+1);

	RExC_parse = endbrace;
	RExC_end = orig_end;
	RExC_override_recoding = 0;

	nextchar(pRExC_state);
    }

    return ret;
}


/*
 * reg_recode
 *
 * It returns the code point in utf8 for the value in *encp.
 *    value: a code value in the source encoding
 *    encp:  a pointer to an Encode object
 *
 * If the result from Encode is not a single character,
 * it returns U+FFFD (Replacement character) and sets *encp to NULL.
 */
STATIC UV
S_reg_recode(pTHX_ const char value, SV **encp)
{
    STRLEN numlen = 1;
    SV * const sv = newSVpvn_flags(&value, numlen, SVs_TEMP);
    const char * const s = *encp ? sv_recode_to_utf8(sv, *encp) : SvPVX(sv);
    const STRLEN newlen = SvCUR(sv);
    UV uv = UNICODE_REPLACEMENT;

    PERL_ARGS_ASSERT_REG_RECODE;

    if (newlen)
	uv = SvUTF8(sv)
	     ? utf8n_to_uvchr((U8*)s, newlen, &numlen, UTF8_ALLOW_DEFAULT)
	     : *(U8*)s;

    if (!newlen || numlen != newlen) {
	uv = UNICODE_REPLACEMENT;
	*encp = NULL;
    }
    return uv;
}


/*
 - regatom - the lowest level

   Try to identify anything special at the start of the pattern. If there
   is, then handle it as required. This may involve generating a single regop,
   such as for an assertion; or it may involve recursing, such as to
   handle a () structure.

   If the string doesn't start with something special then we gobble up
   as much literal text as we can.

   Once we have been able to handle whatever type of thing started the
   sequence, we return.

   Note: we have to be careful with escapes, as they can be both literal
   and special, and in the case of \10 and friends can either, depending
   on context. Specifically there are two separate switches for handling
   escape sequences, with the one for handling literal escapes requiring
   a dummy entry for all of the special escapes that are actually handled
   by the other.
*/

STATIC regnode *
S_regatom(pTHX_ RExC_state_t *pRExC_state, I32 *flagp, U32 depth)
{
    dVAR;
    register regnode *ret = NULL;
    I32 flags;
    char *parse_start = RExC_parse;
    U8 op;
    GET_RE_DEBUG_FLAGS_DECL;
    DEBUG_PARSE("atom");
    *flagp = WORST;		/* Tentatively. */

    PERL_ARGS_ASSERT_REGATOM;

tryagain:
    switch ((U8)*RExC_parse) {
    case '^':
	RExC_seen_zerolen++;
	nextchar(pRExC_state);
	if (RExC_flags & RXf_PMf_MULTILINE)
	    ret = reg_node(pRExC_state, MBOL);
	else if (RExC_flags & RXf_PMf_SINGLELINE)
	    ret = reg_node(pRExC_state, SBOL);
	else
	    ret = reg_node(pRExC_state, BOL);
        Set_Node_Length(ret, 1); /* MJD */
	break;
    case '$':
	nextchar(pRExC_state);
	if (*RExC_parse)
	    RExC_seen_zerolen++;
	if (RExC_flags & RXf_PMf_MULTILINE)
	    ret = reg_node(pRExC_state, MEOL);
	else if (RExC_flags & RXf_PMf_SINGLELINE)
	    ret = reg_node(pRExC_state, SEOL);
	else
	    ret = reg_node(pRExC_state, EOL);
        Set_Node_Length(ret, 1); /* MJD */
	break;
    case '.':
	nextchar(pRExC_state);
	if (RExC_flags & RXf_PMf_SINGLELINE)
	    ret = reg_node(pRExC_state, SANY);
	else
	    ret = reg_node(pRExC_state, REG_ANY);
	*flagp |= HASWIDTH|SIMPLE;
	RExC_naughty++;
        Set_Node_Length(ret, 1); /* MJD */
	break;
    case '[':
    {
	char * const oregcomp_parse = ++RExC_parse;
        ret = regclass(pRExC_state,depth+1);
	if (*RExC_parse != ']') {
	    RExC_parse = oregcomp_parse;
	    vFAIL("Unmatched [");
	}
	nextchar(pRExC_state);
	*flagp |= HASWIDTH|SIMPLE;
        Set_Node_Length(ret, RExC_parse - oregcomp_parse + 1); /* MJD */
	break;
    }
    case '(':
	nextchar(pRExC_state);
        ret = reg(pRExC_state, 1, &flags,depth+1);
	if (ret == NULL) {
		if (flags & TRYAGAIN) {
		    if (RExC_parse == RExC_end) {
			 /* Make parent create an empty node if needed. */
			*flagp |= TRYAGAIN;
			return(NULL);
		    }
		    goto tryagain;
		}
		return(NULL);
	}
	*flagp |= flags&(HASWIDTH|SPSTART|SIMPLE|POSTPONED);
	break;
    case '|':
    case ')':
	if (flags & TRYAGAIN) {
	    *flagp |= TRYAGAIN;
	    return NULL;
	}
	vFAIL("Internal urp");
				/* Supposed to be caught earlier. */
	break;
    case '{':
	if (!regcurly(RExC_parse)) {
	    RExC_parse++;
	    goto defchar;
	}
	/* FALL THROUGH */
    case '?':
    case '+':
    case '*':
	RExC_parse++;
	vFAIL("Quantifier follows nothing");
	break;
    case '\\':
	/* Special Escapes

	   This switch handles escape sequences that resolve to some kind
	   of special regop and not to literal text. Escape sequnces that
	   resolve to literal text are handled below in the switch marked
	   "Literal Escapes".

	   Every entry in this switch *must* have a corresponding entry
	   in the literal escape switch. However, the opposite is not
	   required, as the default for this switch is to jump to the
	   literal text handling code.
	*/
	switch ((U8)*++RExC_parse) {
	/* Special Escapes */
	case 'A':
	    RExC_seen_zerolen++;
	    ret = reg_node(pRExC_state, SBOL);
	    *flagp |= SIMPLE;
	    goto finish_meta_pat;
	case 'G':
	    ret = reg_node(pRExC_state, GPOS);
	    RExC_seen |= REG_SEEN_GPOS;
	    *flagp |= SIMPLE;
	    goto finish_meta_pat;
	case 'K':
	    RExC_seen_zerolen++;
	    ret = reg_node(pRExC_state, KEEPS);
	    *flagp |= SIMPLE;
	    /* XXX:dmq : disabling in-place substitution seems to
	     * be necessary here to avoid cases of memory corruption, as
	     * with: C<$_="x" x 80; s/x\K/y/> -- rgs
	     */
	    RExC_seen |= REG_SEEN_LOOKBEHIND;
	    goto finish_meta_pat;
	case 'Z':
	    ret = reg_node(pRExC_state, SEOL);
	    *flagp |= SIMPLE;
	    RExC_seen_zerolen++;		/* Do not optimize RE away */
	    goto finish_meta_pat;
	case 'z':
	    ret = reg_node(pRExC_state, EOS);
	    *flagp |= SIMPLE;
	    RExC_seen_zerolen++;		/* Do not optimize RE away */
	    goto finish_meta_pat;
	case 'C':
	    ret = reg_node(pRExC_state, CANY);
	    RExC_seen |= REG_SEEN_CANY;
	    *flagp |= HASWIDTH|SIMPLE;
	    goto finish_meta_pat;
	case 'X':
	    ret = reg_node(pRExC_state, CLUMP);
	    *flagp |= HASWIDTH;
	    goto finish_meta_pat;
	case 'w':
	    switch (get_regex_charset(RExC_flags)) {
		case REGEX_LOCALE_CHARSET:
		    op = ALNUML;
		    break;
		case REGEX_UNICODE_CHARSET:
		    op = ALNUMU;
		    break;
		case REGEX_ASCII_RESTRICTED_CHARSET:
		case REGEX_ASCII_MORE_RESTRICTED_CHARSET:
		    op = ALNUMA;
		    break;
		case REGEX_DEPENDS_CHARSET:
		    op = ALNUM;
		    break;
		default:
		    goto bad_charset;
            }
	    ret = reg_node(pRExC_state, op);
	    *flagp |= HASWIDTH|SIMPLE;
	    goto finish_meta_pat;
	case 'W':
	    switch (get_regex_charset(RExC_flags)) {
		case REGEX_LOCALE_CHARSET:
		    op = NALNUML;
		    break;
		case REGEX_UNICODE_CHARSET:
		    op = NALNUMU;
		    break;
		case REGEX_ASCII_RESTRICTED_CHARSET:
		case REGEX_ASCII_MORE_RESTRICTED_CHARSET:
		    op = NALNUMA;
		    break;
		case REGEX_DEPENDS_CHARSET:
		    op = NALNUM;
		    break;
		default:
		    goto bad_charset;
            }
	    ret = reg_node(pRExC_state, op);
	    *flagp |= HASWIDTH|SIMPLE;
	    goto finish_meta_pat;
	case 'b':
	    RExC_seen_zerolen++;
	    RExC_seen |= REG_SEEN_LOOKBEHIND;
	    switch (get_regex_charset(RExC_flags)) {
		case REGEX_LOCALE_CHARSET:
		    op = BOUNDL;
		    break;
		case REGEX_UNICODE_CHARSET:
		    op = BOUNDU;
		    break;
		case REGEX_ASCII_RESTRICTED_CHARSET:
		case REGEX_ASCII_MORE_RESTRICTED_CHARSET:
		    op = BOUNDA;
		    break;
		case REGEX_DEPENDS_CHARSET:
		    op = BOUND;
		    break;
		default:
		    goto bad_charset;
            }
	    ret = reg_node(pRExC_state, op);
	    FLAGS(ret) = get_regex_charset(RExC_flags);
	    *flagp |= SIMPLE;
	    if (! SIZE_ONLY && (U8) *(RExC_parse + 1) == '{') {
		ckWARNregdep(RExC_parse, "\"\\b{\" is deprecated; use \"\\b\\{\" instead");
	    }
	    goto finish_meta_pat;
	case 'B':
	    RExC_seen_zerolen++;
	    RExC_seen |= REG_SEEN_LOOKBEHIND;
	    switch (get_regex_charset(RExC_flags)) {
		case REGEX_LOCALE_CHARSET:
		    op = NBOUNDL;
		    break;
		case REGEX_UNICODE_CHARSET:
		    op = NBOUNDU;
		    break;
		case REGEX_ASCII_RESTRICTED_CHARSET:
		case REGEX_ASCII_MORE_RESTRICTED_CHARSET:
		    op = NBOUNDA;
		    break;
		case REGEX_DEPENDS_CHARSET:
		    op = NBOUND;
		    break;
		default:
		    goto bad_charset;
            }
	    ret = reg_node(pRExC_state, op);
	    FLAGS(ret) = get_regex_charset(RExC_flags);
	    *flagp |= SIMPLE;
	    if (! SIZE_ONLY && (U8) *(RExC_parse + 1) == '{') {
		ckWARNregdep(RExC_parse, "\"\\B{\" is deprecated; use \"\\B\\{\" instead");
	    }
	    goto finish_meta_pat;
	case 's':
	    switch (get_regex_charset(RExC_flags)) {
		case REGEX_LOCALE_CHARSET:
		    op = SPACEL;
		    break;
		case REGEX_UNICODE_CHARSET:
		    op = SPACEU;
		    break;
		case REGEX_ASCII_RESTRICTED_CHARSET:
		case REGEX_ASCII_MORE_RESTRICTED_CHARSET:
		    op = SPACEA;
		    break;
		case REGEX_DEPENDS_CHARSET:
		    op = SPACE;
		    break;
		default:
		    goto bad_charset;
            }
	    ret = reg_node(pRExC_state, op);
	    *flagp |= HASWIDTH|SIMPLE;
	    goto finish_meta_pat;
	case 'S':
	    switch (get_regex_charset(RExC_flags)) {
		case REGEX_LOCALE_CHARSET:
		    op = NSPACEL;
		    break;
		case REGEX_UNICODE_CHARSET:
		    op = NSPACEU;
		    break;
		case REGEX_ASCII_RESTRICTED_CHARSET:
		case REGEX_ASCII_MORE_RESTRICTED_CHARSET:
		    op = NSPACEA;
		    break;
		case REGEX_DEPENDS_CHARSET:
		    op = NSPACE;
		    break;
		default:
		    goto bad_charset;
            }
	    ret = reg_node(pRExC_state, op);
	    *flagp |= HASWIDTH|SIMPLE;
	    goto finish_meta_pat;
	case 'd':
	    switch (get_regex_charset(RExC_flags)) {
		case REGEX_LOCALE_CHARSET:
		    op = DIGITL;
		    break;
		case REGEX_ASCII_RESTRICTED_CHARSET:
		case REGEX_ASCII_MORE_RESTRICTED_CHARSET:
		    op = DIGITA;
		    break;
		case REGEX_DEPENDS_CHARSET: /* No difference between these */
		case REGEX_UNICODE_CHARSET:
		    op = DIGIT;
		    break;
		default:
		    goto bad_charset;
            }
	    ret = reg_node(pRExC_state, op);
	    *flagp |= HASWIDTH|SIMPLE;
	    goto finish_meta_pat;
	case 'D':
	    switch (get_regex_charset(RExC_flags)) {
		case REGEX_LOCALE_CHARSET:
		    op = NDIGITL;
		    break;
		case REGEX_ASCII_RESTRICTED_CHARSET:
		case REGEX_ASCII_MORE_RESTRICTED_CHARSET:
		    op = NDIGITA;
		    break;
		case REGEX_DEPENDS_CHARSET: /* No difference between these */
		case REGEX_UNICODE_CHARSET:
		    op = NDIGIT;
		    break;
		default:
		    goto bad_charset;
            }
	    ret = reg_node(pRExC_state, op);
	    *flagp |= HASWIDTH|SIMPLE;
	    goto finish_meta_pat;
	case 'R':
	    ret = reg_node(pRExC_state, LNBREAK);
	    *flagp |= HASWIDTH|SIMPLE;
	    goto finish_meta_pat;
	case 'h':
	    ret = reg_node(pRExC_state, HORIZWS);
	    *flagp |= HASWIDTH|SIMPLE;
	    goto finish_meta_pat;
	case 'H':
	    ret = reg_node(pRExC_state, NHORIZWS);
	    *flagp |= HASWIDTH|SIMPLE;
	    goto finish_meta_pat;
	case 'v':
	    ret = reg_node(pRExC_state, VERTWS);
	    *flagp |= HASWIDTH|SIMPLE;
	    goto finish_meta_pat;
	case 'V':
	    ret = reg_node(pRExC_state, NVERTWS);
	    *flagp |= HASWIDTH|SIMPLE;
         finish_meta_pat:	    
	    nextchar(pRExC_state);
            Set_Node_Length(ret, 2); /* MJD */
	    break;	    
	case 'p':
	case 'P':
	    {
		char* const oldregxend = RExC_end;
#ifdef DEBUGGING
		char* parse_start = RExC_parse - 2;
#endif

		if (RExC_parse[1] == '{') {
		  /* a lovely hack--pretend we saw [\pX] instead */
		    RExC_end = strchr(RExC_parse, '}');
		    if (!RExC_end) {
		        const U8 c = (U8)*RExC_parse;
			RExC_parse += 2;
			RExC_end = oldregxend;
			vFAIL2("Missing right brace on \\%c{}", c);
		    }
		    RExC_end++;
		}
		else {
		    RExC_end = RExC_parse + 2;
		    if (RExC_end > oldregxend)
			RExC_end = oldregxend;
		}
		RExC_parse--;

                ret = regclass(pRExC_state,depth+1);

		RExC_end = oldregxend;
		RExC_parse--;

		Set_Node_Offset(ret, parse_start + 2);
		Set_Node_Cur_Length(ret);
		nextchar(pRExC_state);
		*flagp |= HASWIDTH|SIMPLE;
	    }
	    break;
        case 'N': 
            /* Handle \N and \N{NAME} here and not below because it can be
            multicharacter. join_exact() will join them up later on. 
            Also this makes sure that things like /\N{BLAH}+/ and 
            \N{BLAH} being multi char Just Happen. dmq*/
            ++RExC_parse;
            ret= reg_namedseq(pRExC_state, NULL, flagp, depth);
            break;
	case 'k':    /* Handle \k<NAME> and \k'NAME' */
	parse_named_seq:
        {   
            char ch= RExC_parse[1];	    
	    if (ch != '<' && ch != '\'' && ch != '{') {
	        RExC_parse++;
	        vFAIL2("Sequence %.2s... not terminated",parse_start);
	    } else {
	        /* this pretty much dupes the code for (?P=...) in reg(), if
                   you change this make sure you change that */
		char* name_start = (RExC_parse += 2);
		U32 num = 0;
                SV *sv_dat = reg_scan_name(pRExC_state,
                    SIZE_ONLY ? REG_RSN_RETURN_NULL : REG_RSN_RETURN_DATA);
                ch= (ch == '<') ? '>' : (ch == '{') ? '}' : '\'';
                if (RExC_parse == name_start || *RExC_parse != ch)
                    vFAIL2("Sequence %.3s... not terminated",parse_start);

                if (!SIZE_ONLY) {
                    num = add_data( pRExC_state, 1, "S" );
                    RExC_rxi->data->data[num]=(void*)sv_dat;
                    SvREFCNT_inc_simple_void(sv_dat);
                }

                RExC_sawback = 1;
                ret = reganode(pRExC_state,
                               ((! FOLD)
                                 ? NREF
				 : (MORE_ASCII_RESTRICTED)
				   ? NREFFA
                                   : (AT_LEAST_UNI_SEMANTICS)
                                     ? NREFFU
                                     : (LOC)
                                       ? NREFFL
                                       : NREFF),
                                num);
                *flagp |= HASWIDTH;

                /* override incorrect value set in reganode MJD */
                Set_Node_Offset(ret, parse_start+1);
                Set_Node_Cur_Length(ret); /* MJD */
                nextchar(pRExC_state);

            }
            break;
	}
	case 'g': 
	case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    {
		I32 num;
		bool isg = *RExC_parse == 'g';
		bool isrel = 0; 
		bool hasbrace = 0;
		if (isg) {
		    RExC_parse++;
		    if (*RExC_parse == '{') {
		        RExC_parse++;
		        hasbrace = 1;
		    }
		    if (*RExC_parse == '-') {
		        RExC_parse++;
		        isrel = 1;
		    }
		    if (hasbrace && !isDIGIT(*RExC_parse)) {
		        if (isrel) RExC_parse--;
                        RExC_parse -= 2;		            
		        goto parse_named_seq;
		}   }
		num = atoi(RExC_parse);
		if (isg && num == 0)
		    vFAIL("Reference to invalid group 0");
                if (isrel) {
                    num = RExC_npar - num;
                    if (num < 1)
                        vFAIL("Reference to nonexistent or unclosed group");
                }
		if (!isg && num > 9 && num >= RExC_npar)
		    goto defchar;
		else {
		    char * const parse_start = RExC_parse - 1; /* MJD */
		    while (isDIGIT(*RExC_parse))
			RExC_parse++;
	            if (parse_start == RExC_parse - 1) 
	                vFAIL("Unterminated \\g... pattern");
                    if (hasbrace) {
                        if (*RExC_parse != '}') 
                            vFAIL("Unterminated \\g{...} pattern");
                        RExC_parse++;
                    }    
		    if (!SIZE_ONLY) {
		        if (num > (I32)RExC_rx->nparens)
			    vFAIL("Reference to nonexistent group");
		    }
		    RExC_sawback = 1;
		    ret = reganode(pRExC_state,
				   ((! FOLD)
				     ? REF
				     : (MORE_ASCII_RESTRICTED)
				       ? REFFA
                                       : (AT_LEAST_UNI_SEMANTICS)
                                         ? REFFU
                                         : (LOC)
                                           ? REFFL
                                           : REFF),
				    num);
		    *flagp |= HASWIDTH;

                    /* override incorrect value set in reganode MJD */
                    Set_Node_Offset(ret, parse_start+1);
                    Set_Node_Cur_Length(ret); /* MJD */
		    RExC_parse--;
		    nextchar(pRExC_state);
		}
	    }
	    break;
	case '\0':
	    if (RExC_parse >= RExC_end)
		FAIL("Trailing \\");
	    /* FALL THROUGH */
	default:
	    /* Do not generate "unrecognized" warnings here, we fall
	       back into the quick-grab loop below */
	    parse_start--;
	    goto defchar;
	}
	break;

    case '#':
	if (RExC_flags & RXf_PMf_EXTENDED) {
	    if ( reg_skipcomment( pRExC_state ) )
		goto tryagain;
	}
	/* FALL THROUGH */

    default:

            parse_start = RExC_parse - 1;

	    RExC_parse++;

	defchar: {
	    register STRLEN len;
	    register UV ender;
	    register char *p;
	    char *s;
	    STRLEN foldlen;
	    U8 tmpbuf[UTF8_MAXBYTES_CASE+1], *foldbuf;
            U8 node_type;

	    /* Is this a LATIN LOWER CASE SHARP S in an EXACTFU node?  If so,
	     * it is folded to 'ss' even if not utf8 */
	    bool is_exactfu_sharp_s;

	    ender = 0;
            node_type = ((! FOLD) ? EXACT
		        : (LOC)
			  ? EXACTFL
			  : (MORE_ASCII_RESTRICTED)
			    ? EXACTFA
			    : (AT_LEAST_UNI_SEMANTICS)
			      ? EXACTFU
			      : EXACTF);
	    ret = reg_node(pRExC_state, node_type);
	    s = STRING(ret);

	    /* XXX The node can hold up to 255 bytes, yet this only goes to
             * 127.  I (khw) do not know why.  Keeping it somewhat less than
             * 255 allows us to not have to worry about overflow due to
             * converting to utf8 and fold expansion, but that value is
             * 255-UTF8_MAXBYTES_CASE.  join_exact() may join adjacent nodes
             * split up by this limit into a single one using the real max of
             * 255.  Even at 127, this breaks under rare circumstances.  If
             * folding, we do not want to split a node at a character that is a
             * non-final in a multi-char fold, as an input string could just
             * happen to want to match across the node boundary.  The join
             * would solve that problem if the join actually happens.  But a
             * series of more than two nodes in a row each of 127 would cause
             * the first join to succeed to get to 254, but then there wouldn't
             * be room for the next one, which could at be one of those split
             * multi-char folds.  I don't know of any fool-proof solution.  One
             * could back off to end with only a code point that isn't such a
             * non-final, but it is possible for there not to be any in the
             * entire node. */
	    for (len = 0, p = RExC_parse - 1;
	         len < 127 && p < RExC_end;
	         len++)
	    {
		char * const oldp = p;

		if (RExC_flags & RXf_PMf_EXTENDED)
		    p = regwhite( pRExC_state, p );
		switch ((U8)*p) {
		case '^':
		case '$':
		case '.':
		case '[':
		case '(':
		case ')':
		case '|':
		    goto loopdone;
		case '\\':
		    /* Literal Escapes Switch

		       This switch is meant to handle escape sequences that
		       resolve to a literal character.

		       Every escape sequence that represents something
		       else, like an assertion or a char class, is handled
		       in the switch marked 'Special Escapes' above in this
		       routine, but also has an entry here as anything that
		       isn't explicitly mentioned here will be treated as
		       an unescaped equivalent literal.
		    */

		    switch ((U8)*++p) {
		    /* These are all the special escapes. */
		    case 'A':             /* Start assertion */
		    case 'b': case 'B':   /* Word-boundary assertion*/
		    case 'C':             /* Single char !DANGEROUS! */
		    case 'd': case 'D':   /* digit class */
		    case 'g': case 'G':   /* generic-backref, pos assertion */
		    case 'h': case 'H':   /* HORIZWS */
		    case 'k': case 'K':   /* named backref, keep marker */
		    case 'N':             /* named char sequence */
		    case 'p': case 'P':   /* Unicode property */
		              case 'R':   /* LNBREAK */
		    case 's': case 'S':   /* space class */
		    case 'v': case 'V':   /* VERTWS */
		    case 'w': case 'W':   /* word class */
		    case 'X':             /* eXtended Unicode "combining character sequence" */
		    case 'z': case 'Z':   /* End of line/string assertion */
			--p;
			goto loopdone;

	            /* Anything after here is an escape that resolves to a
	               literal. (Except digits, which may or may not)
	             */
		    case 'n':
			ender = '\n';
			p++;
			break;
		    case 'r':
			ender = '\r';
			p++;
			break;
		    case 't':
			ender = '\t';
			p++;
			break;
		    case 'f':
			ender = '\f';
			p++;
			break;
		    case 'e':
			  ender = ASCII_TO_NATIVE('\033');
			p++;
			break;
		    case 'a':
			  ender = ASCII_TO_NATIVE('\007');
			p++;
			break;
		    case 'o':
			{
			    STRLEN brace_len = len;
			    UV result;
			    const char* error_msg;

			    bool valid = grok_bslash_o(p,
						       &result,
						       &brace_len,
						       &error_msg,
						       1);
			    p += brace_len;
			    if (! valid) {
				RExC_parse = p;	/* going to die anyway; point
						   to exact spot of failure */
				vFAIL(error_msg);
			    }
			    else
			    {
				ender = result;
			    }
			    if (PL_encoding && ender < 0x100) {
				goto recode_encoding;
			    }
			    if (ender > 0xff) {
				REQUIRE_UTF8;
			    }
			    break;
			}
		    case 'x':
			if (*++p == '{') {
			    char* const e = strchr(p, '}');

			    if (!e) {
				RExC_parse = p + 1;
				vFAIL("Missing right brace on \\x{}");
			    }
			    else {
                                I32 flags = PERL_SCAN_ALLOW_UNDERSCORES
                                    | PERL_SCAN_DISALLOW_PREFIX;
                                STRLEN numlen = e - p - 1;
				ender = grok_hex(p + 1, &numlen, &flags, NULL);
				if (ender > 0xff)
				    REQUIRE_UTF8;
				p = e + 1;
			    }
			}
			else {
                            I32 flags = PERL_SCAN_DISALLOW_PREFIX;
			    STRLEN numlen = 2;
			    ender = grok_hex(p, &numlen, &flags, NULL);
			    p += numlen;
			}
			if (PL_encoding && ender < 0x100)
			    goto recode_encoding;
			break;
		    case 'c':
			p++;
			ender = grok_bslash_c(*p++, UTF, SIZE_ONLY);
			break;
		    case '0': case '1': case '2': case '3':case '4':
		    case '5': case '6': case '7': case '8':case '9':
			if (*p == '0' ||
			    (isDIGIT(p[1]) && atoi(p) >= RExC_npar))
			{
			    I32 flags = PERL_SCAN_SILENT_ILLDIGIT;
			    STRLEN numlen = 3;
			    ender = grok_oct(p, &numlen, &flags, NULL);
			    if (ender > 0xff) {
				REQUIRE_UTF8;
			    }
			    p += numlen;
			}
			else {
			    --p;
			    goto loopdone;
			}
			if (PL_encoding && ender < 0x100)
			    goto recode_encoding;
			break;
		    recode_encoding:
			if (! RExC_override_recoding) {
			    SV* enc = PL_encoding;
			    ender = reg_recode((const char)(U8)ender, &enc);
			    if (!enc && SIZE_ONLY)
				ckWARNreg(p, "Invalid escape in the specified encoding");
			    REQUIRE_UTF8;
			}
			break;
		    case '\0':
			if (p >= RExC_end)
			    FAIL("Trailing \\");
			/* FALL THROUGH */
		    default:
			if (!SIZE_ONLY&& isALPHA(*p)) {
			    /* Include any { following the alpha to emphasize
			     * that it could be part of an escape at some point
			     * in the future */
			    int len = (*(p + 1) == '{') ? 2 : 1;
			    ckWARN3reg(p + len, "Unrecognized escape \\%.*s passed through", len, p);
			}
			goto normal_default;
		    }
		    break;
		default:
		  normal_default:
		    if (UTF8_IS_START(*p) && UTF) {
			STRLEN numlen;
			ender = utf8n_to_uvchr((U8*)p, RExC_end - p,
					       &numlen, UTF8_ALLOW_DEFAULT);
			p += numlen;
		    }
		    else
			ender = (U8) *p++;
		    break;
		} /* End of switch on the literal */

                is_exactfu_sharp_s = (node_type == EXACTFU
			              && ender == LATIN_SMALL_LETTER_SHARP_S);
		if ( RExC_flags & RXf_PMf_EXTENDED)
		    p = regwhite( pRExC_state, p );
		if ((UTF && FOLD) || is_exactfu_sharp_s) {
		    /* Prime the casefolded buffer.  Locale rules, which apply
		     * only to code points < 256, aren't known until execution,
		     * so for them, just output the original character using
                     * utf8.  If we start to fold non-UTF patterns, be sure to
                     * update join_exact() */
		    if (LOC && ender < 256) {
			if (UNI_IS_INVARIANT(ender)) {
			    *tmpbuf = (U8) ender;
			    foldlen = 1;
			} else {
			    *tmpbuf = UTF8_TWO_BYTE_HI(ender);
			    *(tmpbuf + 1) = UTF8_TWO_BYTE_LO(ender);
			    foldlen = 2;
			}
		    }
		    else if (isASCII(ender)) {	/* Note: Here can't also be LOC
						 */
			ender = toLOWER(ender);
			*tmpbuf = (U8) ender;
			foldlen = 1;
		    }
		    else if (! MORE_ASCII_RESTRICTED && ! LOC) {

			/* Locale and /aa require more selectivity about the
			 * fold, so are handled below.  Otherwise, here, just
			 * use the fold */
			ender = toFOLD_uni(ender, tmpbuf, &foldlen);
		    }
		    else {
			/* Under locale rules or /aa we are not to mix,
			 * respectively, ords < 256 or ASCII with non-.  So
			 * reject folds that mix them, using only the
			 * non-folded code point.  So do the fold to a
			 * temporary, and inspect each character in it. */
			U8 trialbuf[UTF8_MAXBYTES_CASE+1];
			U8* s = trialbuf;
			UV tmpender = toFOLD_uni(ender, trialbuf, &foldlen);
			U8* e = s + foldlen;
			bool fold_ok = TRUE;

			while (s < e) {
			    if (isASCII(*s)
				|| (LOC && (UTF8_IS_INVARIANT(*s)
					   || UTF8_IS_DOWNGRADEABLE_START(*s))))
			    {
				fold_ok = FALSE;
				break;
			    }
			    s += UTF8SKIP(s);
			}
			if (fold_ok) {
			    Copy(trialbuf, tmpbuf, foldlen, U8);
			    ender = tmpender;
			}
			else {
			    uvuni_to_utf8(tmpbuf, ender);
			    foldlen = UNISKIP(ender);
			}
		    }
		}
		if (p < RExC_end && ISMULT2(p)) { /* Back off on ?+*. */
		    if (len)
			p = oldp;
		    else if (UTF || is_exactfu_sharp_s) {
			 if (FOLD) {
			      /* Emit all the Unicode characters. */
			      STRLEN numlen;
			      for (foldbuf = tmpbuf;
				   foldlen;
				   foldlen -= numlen) {

				   /* tmpbuf has been constructed by us, so we
				    * know it is valid utf8 */
				   ender = valid_utf8_to_uvchr(foldbuf, &numlen);
				   if (numlen > 0) {
					const STRLEN unilen = reguni(pRExC_state, ender, s);
					s       += unilen;
					len     += unilen;
					/* In EBCDIC the numlen
					 * and unilen can differ. */
					foldbuf += numlen;
					if (numlen >= foldlen)
					     break;
				   }
				   else
					break; /* "Can't happen." */
			      }
			 }
			 else {
			      const STRLEN unilen = reguni(pRExC_state, ender, s);
			      if (unilen > 0) {
				   s   += unilen;
				   len += unilen;
			      }
			 }
		    }
		    else {
			len++;
			REGC((char)ender, s++);
		    }
		    break;
		}
                if (UTF || is_exactfu_sharp_s) {
		     if (FOLD) {
		          /* Emit all the Unicode characters. */
			  STRLEN numlen;
			  for (foldbuf = tmpbuf;
			       foldlen;
			       foldlen -= numlen) {
			       ender = valid_utf8_to_uvchr(foldbuf, &numlen);
			       if (numlen > 0) {
				    const STRLEN unilen = reguni(pRExC_state, ender, s);
				    len     += unilen;
				    s       += unilen;
				    /* In EBCDIC the numlen
				     * and unilen can differ. */
				    foldbuf += numlen;
				    if (numlen >= foldlen)
					 break;
			       }
			       else
				    break;
			  }
		     }
		     else {
			  const STRLEN unilen = reguni(pRExC_state, ender, s);
			  if (unilen > 0) {
			       s   += unilen;
			       len += unilen;
			  }
		     }
		     len--;
		}
		else {
		    REGC((char)ender, s++);
		}
	    }
	loopdone:   /* Jumped to when encounters something that shouldn't be in
		       the node */
	    RExC_parse = p - 1;
            Set_Node_Cur_Length(ret); /* MJD */
	    nextchar(pRExC_state);
	    {
		/* len is STRLEN which is unsigned, need to copy to signed */
		IV iv = len;
		if (iv < 0)
		    vFAIL("Internal disaster");
	    }
	    if (len > 0)
		*flagp |= HASWIDTH;
	    if (len == 1 && UNI_IS_INVARIANT(ender))
		*flagp |= SIMPLE;

	    if (SIZE_ONLY)
		RExC_size += STR_SZ(len);
	    else {
		STR_LEN(ret) = len;
		RExC_emit += STR_SZ(len);
            }
	}
	break;
    }

    return(ret);

/* Jumped to when an unrecognized character set is encountered */
bad_charset:
    Perl_croak(aTHX_ "panic: Unknown regex character set encoding: %u", get_regex_charset(RExC_flags));
    return(NULL);
}

STATIC char *
S_regwhite( RExC_state_t *pRExC_state, char *p )
{
    const char *e = RExC_end;

    PERL_ARGS_ASSERT_REGWHITE;

    while (p < e) {
	if (isSPACE(*p))
	    ++p;
	else if (*p == '#') {
            bool ended = 0;
	    do {
		if (*p++ == '\n') {
		    ended = 1;
		    break;
		}
	    } while (p < e);
	    if (!ended)
	        RExC_seen |= REG_SEEN_RUN_ON_COMMENT;
	}
	else
	    break;
    }
    return p;
}

/* Parse POSIX character classes: [[:foo:]], [[=foo=]], [[.foo.]].
   Character classes ([:foo:]) can also be negated ([:^foo:]).
   Returns a named class id (ANYOF_XXX) if successful, -1 otherwise.
   Equivalence classes ([=foo=]) and composites ([.foo.]) are parsed,
   but trigger failures because they are currently unimplemented. */

#define POSIXCC_DONE(c)   ((c) == ':')
#define POSIXCC_NOTYET(c) ((c) == '=' || (c) == '.')
#define POSIXCC(c) (POSIXCC_DONE(c) || POSIXCC_NOTYET(c))

STATIC I32
S_regpposixcc(pTHX_ RExC_state_t *pRExC_state, I32 value)
{
    dVAR;
    I32 namedclass = OOB_NAMEDCLASS;

    PERL_ARGS_ASSERT_REGPPOSIXCC;

    if (value == '[' && RExC_parse + 1 < RExC_end &&
	/* I smell either [: or [= or [. -- POSIX has been here, right? */
	POSIXCC(UCHARAT(RExC_parse))) {
	const char c = UCHARAT(RExC_parse);
	char* const s = RExC_parse++;

	while (RExC_parse < RExC_end && UCHARAT(RExC_parse) != c)
	    RExC_parse++;
	if (RExC_parse == RExC_end)
	    /* Grandfather lone [:, [=, [. */
	    RExC_parse = s;
	else {
	    const char* const t = RExC_parse++; /* skip over the c */
	    assert(*t == c);

  	    if (UCHARAT(RExC_parse) == ']') {
		const char *posixcc = s + 1;
  		RExC_parse++; /* skip over the ending ] */

		if (*s == ':') {
		    const I32 complement = *posixcc == '^' ? *posixcc++ : 0;
		    const I32 skip = t - posixcc;

		    /* Initially switch on the length of the name.  */
		    switch (skip) {
		    case 4:
			if (memEQ(posixcc, "word", 4)) /* this is not POSIX, this is the Perl \w */
			    namedclass = complement ? ANYOF_NALNUM : ANYOF_ALNUM;
			break;
		    case 5:
			/* Names all of length 5.  */
			/* alnum alpha ascii blank cntrl digit graph lower
			   print punct space upper  */
			/* Offset 4 gives the best switch position.  */
			switch (posixcc[4]) {
			case 'a':
			    if (memEQ(posixcc, "alph", 4)) /* alpha */
				namedclass = complement ? ANYOF_NALPHA : ANYOF_ALPHA;
			    break;
			case 'e':
			    if (memEQ(posixcc, "spac", 4)) /* space */
				namedclass = complement ? ANYOF_NPSXSPC : ANYOF_PSXSPC;
			    break;
			case 'h':
			    if (memEQ(posixcc, "grap", 4)) /* graph */
				namedclass = complement ? ANYOF_NGRAPH : ANYOF_GRAPH;
			    break;
			case 'i':
			    if (memEQ(posixcc, "asci", 4)) /* ascii */
				namedclass = complement ? ANYOF_NASCII : ANYOF_ASCII;
			    break;
			case 'k':
			    if (memEQ(posixcc, "blan", 4)) /* blank */
				namedclass = complement ? ANYOF_NBLANK : ANYOF_BLANK;
			    break;
			case 'l':
			    if (memEQ(posixcc, "cntr", 4)) /* cntrl */
				namedclass = complement ? ANYOF_NCNTRL : ANYOF_CNTRL;
			    break;
			case 'm':
			    if (memEQ(posixcc, "alnu", 4)) /* alnum */
				namedclass = complement ? ANYOF_NALNUMC : ANYOF_ALNUMC;
			    break;
			case 'r':
			    if (memEQ(posixcc, "lowe", 4)) /* lower */
				namedclass = complement ? ANYOF_NLOWER : ANYOF_LOWER;
			    else if (memEQ(posixcc, "uppe", 4)) /* upper */
				namedclass = complement ? ANYOF_NUPPER : ANYOF_UPPER;
			    break;
			case 't':
			    if (memEQ(posixcc, "digi", 4)) /* digit */
				namedclass = complement ? ANYOF_NDIGIT : ANYOF_DIGIT;
			    else if (memEQ(posixcc, "prin", 4)) /* print */
				namedclass = complement ? ANYOF_NPRINT : ANYOF_PRINT;
			    else if (memEQ(posixcc, "punc", 4)) /* punct */
				namedclass = complement ? ANYOF_NPUNCT : ANYOF_PUNCT;
			    break;
			}
			break;
		    case 6:
			if (memEQ(posixcc, "xdigit", 6))
			    namedclass = complement ? ANYOF_NXDIGIT : ANYOF_XDIGIT;
			break;
		    }

		    if (namedclass == OOB_NAMEDCLASS)
			Simple_vFAIL3("POSIX class [:%.*s:] unknown",
				      t - s - 1, s + 1);
		    assert (posixcc[skip] == ':');
		    assert (posixcc[skip+1] == ']');
		} else if (!SIZE_ONLY) {
		    /* [[=foo=]] and [[.foo.]] are still future. */

		    /* adjust RExC_parse so the warning shows after
		       the class closes */
		    while (UCHARAT(RExC_parse) && UCHARAT(RExC_parse) != ']')
			RExC_parse++;
		    Simple_vFAIL3("POSIX syntax [%c %c] is reserved for future extensions", c, c);
		}
	    } else {
		/* Maternal grandfather:
		 * "[:" ending in ":" but not in ":]" */
		RExC_parse = s;
	    }
	}
    }

    return namedclass;
}

STATIC void
S_checkposixcc(pTHX_ RExC_state_t *pRExC_state)
{
    dVAR;

    PERL_ARGS_ASSERT_CHECKPOSIXCC;

    if (POSIXCC(UCHARAT(RExC_parse))) {
	const char *s = RExC_parse;
	const char  c = *s++;

	while (isALNUM(*s))
	    s++;
	if (*s && c == *s && s[1] == ']') {
	    ckWARN3reg(s+2,
		       "POSIX syntax [%c %c] belongs inside character classes",
		       c, c);

	    /* [[=foo=]] and [[.foo.]] are still future. */
	    if (POSIXCC_NOTYET(c)) {
		/* adjust RExC_parse so the error shows after
		   the class closes */
		while (UCHARAT(RExC_parse) && UCHARAT(RExC_parse++) != ']')
		    NOOP;
		Simple_vFAIL3("POSIX syntax [%c %c] is reserved for future extensions", c, c);
	    }
	}
    }
}

/* Generate the code to add a full posix character <class> to the bracketed
 * character class given by <node>.  (<node> is needed only under locale rules)
 * destlist     is the inversion list for non-locale rules that this class is
 *              to be added to
 * sourcelist   is the ASCII-range inversion list to add under /a rules
 * Xsourcelist  is the full Unicode range list to use otherwise. */
#define DO_POSIX(node, class, destlist, sourcelist, Xsourcelist)           \
    if (LOC) {                                                             \
	SV* scratch_list = NULL;                                           \
                                                                           \
        /* Set this class in the node for runtime matching */              \
        ANYOF_CLASS_SET(node, class);                                      \
                                                                           \
        /* For above Latin1 code points, we use the full Unicode range */  \
        _invlist_intersection(PL_AboveLatin1,                              \
                              Xsourcelist,                                 \
                              &scratch_list);                              \
        /* And set the output to it, adding instead if there already is an \
	 * output.  Checking if <destlist> is NULL first saves an extra    \
	 * clone.  Its reference count will be decremented at the next     \
	 * union, etc, or if this is the only instance, at the end of the  \
	 * routine */                                                      \
        if (! destlist) {                                                  \
            destlist = scratch_list;                                       \
        }                                                                  \
        else {                                                             \
            _invlist_union(destlist, scratch_list, &destlist);             \
            SvREFCNT_dec(scratch_list);                                    \
        }                                                                  \
    }                                                                      \
    else {                                                                 \
        /* For non-locale, just add it to any existing list */             \
        _invlist_union(destlist,                                           \
                       (AT_LEAST_ASCII_RESTRICTED)                         \
                           ? sourcelist                                    \
                           : Xsourcelist,                                  \
                       &destlist);                                         \
    }

/* Like DO_POSIX, but matches the complement of <sourcelist> and <Xsourcelist>.
 */
#define DO_N_POSIX(node, class, destlist, sourcelist, Xsourcelist)         \
    if (LOC) {                                                             \
        SV* scratch_list = NULL;                                           \
        ANYOF_CLASS_SET(node, class);					   \
        _invlist_subtract(PL_AboveLatin1, Xsourcelist, &scratch_list);	   \
        if (! destlist) {					           \
            destlist = scratch_list;					   \
        }                                                                  \
        else {                                                             \
            _invlist_union(destlist, scratch_list, &destlist);             \
            SvREFCNT_dec(scratch_list);                                    \
        }                                                                  \
    }                                                                      \
    else {                                                                 \
        _invlist_union_complement_2nd(destlist,                            \
                                    (AT_LEAST_ASCII_RESTRICTED)            \
                                        ? sourcelist                       \
                                        : Xsourcelist,                     \
                                    &destlist);                            \
        /* Under /d, everything in the upper half of the Latin1 range      \
         * matches this complement */                                      \
        if (DEPENDS_SEMANTICS) {                                           \
            ANYOF_FLAGS(node) |= ANYOF_NON_UTF8_LATIN1_ALL;                \
        }                                                                  \
    }

/* Generate the code to add a posix character <class> to the bracketed
 * character class given by <node>.  (<node> is needed only under locale rules)
 * destlist       is the inversion list for non-locale rules that this class is
 *                to be added to
 * sourcelist     is the ASCII-range inversion list to add under /a rules
 * l1_sourcelist  is the Latin1 range list to use otherwise.
 * Xpropertyname  is the name to add to <run_time_list> of the property to
 *                specify the code points above Latin1 that will have to be
 *                determined at run-time
 * run_time_list  is a SV* that contains text names of properties that are to
 *                be computed at run time.  This concatenates <Xpropertyname>
 *                to it, apppropriately
 * This is essentially DO_POSIX, but we know only the Latin1 values at compile
 * time */
#define DO_POSIX_LATIN1_ONLY_KNOWN(node, class, destlist, sourcelist,      \
                              l1_sourcelist, Xpropertyname, run_time_list) \
    /* If not /a matching, there are going to be code points we will have  \
     * to defer to runtime to look-up */                                   \
    if (! AT_LEAST_ASCII_RESTRICTED) {                                     \
        Perl_sv_catpvf(aTHX_ run_time_list, "+utf8::%s\n", Xpropertyname); \
    }                                                                      \
    if (LOC) {                                                             \
        ANYOF_CLASS_SET(node, class);                                      \
    }                                                                      \
    else {                                                                 \
        _invlist_union(destlist,                                           \
                       (AT_LEAST_ASCII_RESTRICTED)                         \
                           ? sourcelist                                    \
                           : l1_sourcelist,                                \
                       &destlist);                                         \
    }

/* Like DO_POSIX_LATIN1_ONLY_KNOWN, but for the complement.  A combination of
 * this and DO_N_POSIX */
#define DO_N_POSIX_LATIN1_ONLY_KNOWN(node, class, destlist, sourcelist,    \
                              l1_sourcelist, Xpropertyname, run_time_list) \
    if (AT_LEAST_ASCII_RESTRICTED) {                                       \
        _invlist_union_complement_2nd(destlist, sourcelist, &destlist);    \
    }                                                                      \
    else {                                                                 \
        Perl_sv_catpvf(aTHX_ run_time_list, "!utf8::%s\n", Xpropertyname); \
	if (LOC) {                                                         \
	    ANYOF_CLASS_SET(node, namedclass);				   \
	}                                                                  \
	else {                                                             \
            SV* scratch_list = NULL;                                       \
	    _invlist_subtract(PL_Latin1, l1_sourcelist, &scratch_list);    \
	    if (! destlist) {                                              \
		destlist = scratch_list;                                   \
	    }                                                              \
	    else {                                                         \
		_invlist_union(destlist, scratch_list, &destlist);         \
		SvREFCNT_dec(scratch_list);                                \
	    }                                                              \
	    if (DEPENDS_SEMANTICS) {                                       \
		ANYOF_FLAGS(node) |= ANYOF_NON_UTF8_LATIN1_ALL;            \
	    }                                                              \
	}                                                                  \
    }

STATIC U8
S_set_regclass_bit_fold(pTHX_ RExC_state_t *pRExC_state, regnode* node, const U8 value, SV** invlist_ptr, AV** alternate_ptr)
{

    /* Handle the setting of folds in the bitmap for non-locale ANYOF nodes.
     * Locale folding is done at run-time, so this function should not be
     * called for nodes that are for locales.
     *
     * This function sets the bit corresponding to the fold of the input
     * 'value', if not already set.  The fold of 'f' is 'F', and the fold of
     * 'F' is 'f'.
     *
     * It also knows about the characters that are in the bitmap that have
     * folds that are matchable only outside it, and sets the appropriate lists
     * and flags.
     *
     * It returns the number of bits that actually changed from 0 to 1 */

    U8 stored = 0;
    U8 fold;

    PERL_ARGS_ASSERT_SET_REGCLASS_BIT_FOLD;

    fold = (AT_LEAST_UNI_SEMANTICS) ? PL_fold_latin1[value]
                                    : PL_fold[value];

    /* It assumes the bit for 'value' has already been set */
    if (fold != value && ! ANYOF_BITMAP_TEST(node, fold)) {
        ANYOF_BITMAP_SET(node, fold);
        stored++;
    }
    if (_HAS_NONLATIN1_FOLD_CLOSURE_ONLY_FOR_USE_BY_REGCOMP_DOT_C_AND_REGEXEC_DOT_C(value) && (! isASCII(value) || ! MORE_ASCII_RESTRICTED)) {
	/* Certain Latin1 characters have matches outside the bitmap.  To get
	 * here, 'value' is one of those characters.   None of these matches is
	 * valid for ASCII characters under /aa, which have been excluded by
	 * the 'if' above.  The matches fall into three categories:
	 * 1) They are singly folded-to or -from an above 255 character, as
	 *    LATIN SMALL LETTER Y WITH DIAERESIS and LATIN CAPITAL LETTER Y
	 *    WITH DIAERESIS;
	 * 2) They are part of a multi-char fold with another character in the
	 *    bitmap, only LATIN SMALL LETTER SHARP S => "ss" fits that bill;
	 * 3) They are part of a multi-char fold with a character not in the
	 *    bitmap, such as various ligatures.
	 * We aren't dealing fully with multi-char folds, except we do deal
	 * with the pattern containing a character that has a multi-char fold
	 * (not so much the inverse).
	 * For types 1) and 3), the matches only happen when the target string
	 * is utf8; that's not true for 2), and we set a flag for it.
	 *
	 * The code below adds to the passed in inversion list the single fold
	 * closures for 'value'.  The values are hard-coded here so that an
	 * innocent-looking character class, like /[ks]/i won't have to go out
	 * to disk to find the possible matches.  XXX It would be better to
	 * generate these via regen, in case a new version of the Unicode
	 * standard adds new mappings, though that is not really likely. */
	switch (value) {
	    case 'k':
	    case 'K':
		/* KELVIN SIGN */
		*invlist_ptr = add_cp_to_invlist(*invlist_ptr, 0x212A);
		break;
	    case 's':
	    case 'S':
		/* LATIN SMALL LETTER LONG S */
		*invlist_ptr = add_cp_to_invlist(*invlist_ptr, 0x017F);
		break;
	    case MICRO_SIGN:
		*invlist_ptr = add_cp_to_invlist(*invlist_ptr,
						 GREEK_SMALL_LETTER_MU);
		*invlist_ptr = add_cp_to_invlist(*invlist_ptr,
						 GREEK_CAPITAL_LETTER_MU);
		break;
	    case LATIN_CAPITAL_LETTER_A_WITH_RING_ABOVE:
	    case LATIN_SMALL_LETTER_A_WITH_RING_ABOVE:
		/* ANGSTROM SIGN */
		*invlist_ptr = add_cp_to_invlist(*invlist_ptr, 0x212B);
		if (DEPENDS_SEMANTICS) {    /* See DEPENDS comment below */
		    *invlist_ptr = add_cp_to_invlist(*invlist_ptr,
						     PL_fold_latin1[value]);
		}
		break;
	    case LATIN_SMALL_LETTER_Y_WITH_DIAERESIS:
		*invlist_ptr = add_cp_to_invlist(*invlist_ptr,
					LATIN_CAPITAL_LETTER_Y_WITH_DIAERESIS);
		break;
	    case LATIN_SMALL_LETTER_SHARP_S:
		*invlist_ptr = add_cp_to_invlist(*invlist_ptr,
					LATIN_CAPITAL_LETTER_SHARP_S);

		/* Under /a, /d, and /u, this can match the two chars "ss" */
		if (! MORE_ASCII_RESTRICTED) {
		    add_alternate(alternate_ptr, (U8 *) "ss", 2);

		    /* And under /u or /a, it can match even if the target is
		     * not utf8 */
		    if (AT_LEAST_UNI_SEMANTICS) {
			ANYOF_FLAGS(node) |= ANYOF_NONBITMAP_NON_UTF8;
		    }
		}
		break;
	    case 'F': case 'f':
	    case 'I': case 'i':
	    case 'L': case 'l':
	    case 'T': case 't':
	    case 'A': case 'a':
	    case 'H': case 'h':
	    case 'J': case 'j':
	    case 'N': case 'n':
	    case 'W': case 'w':
	    case 'Y': case 'y':
                /* These all are targets of multi-character folds from code
                 * points that require UTF8 to express, so they can't match
                 * unless the target string is in UTF-8, so no action here is
                 * necessary, as regexec.c properly handles the general case
                 * for UTF-8 matching */
		break;
	    default:
		/* Use deprecated warning to increase the chances of this
		 * being output */
		ckWARN2regdep(RExC_parse, "Perl folding rules are not up-to-date for 0x%x; please use the perlbug utility to report;", value);
		break;
	}
    }
    else if (DEPENDS_SEMANTICS
	    && ! isASCII(value)
	    && PL_fold_latin1[value] != value)
    {
	   /* Under DEPENDS rules, non-ASCII Latin1 characters match their
	    * folds only when the target string is in UTF-8.  We add the fold
	    * here to the list of things to match outside the bitmap, which
	    * won't be looked at unless it is UTF8 (or else if something else
	    * says to look even if not utf8, but those things better not happen
	    * under DEPENDS semantics. */
	*invlist_ptr = add_cp_to_invlist(*invlist_ptr, PL_fold_latin1[value]);
    }

    return stored;
}


PERL_STATIC_INLINE U8
S_set_regclass_bit(pTHX_ RExC_state_t *pRExC_state, regnode* node, const U8 value, SV** invlist_ptr, AV** alternate_ptr)
{
    /* This inline function sets a bit in the bitmap if not already set, and if
     * appropriate, its fold, returning the number of bits that actually
     * changed from 0 to 1 */

    U8 stored;

    PERL_ARGS_ASSERT_SET_REGCLASS_BIT;

    if (ANYOF_BITMAP_TEST(node, value)) {   /* Already set */
	return 0;
    }

    ANYOF_BITMAP_SET(node, value);
    stored = 1;

    if (FOLD && ! LOC) {	/* Locale folds aren't known until runtime */
	stored += set_regclass_bit_fold(pRExC_state, node, value, invlist_ptr, alternate_ptr);
    }

    return stored;
}

STATIC void
S_add_alternate(pTHX_ AV** alternate_ptr, U8* string, STRLEN len)
{
    /* Adds input 'string' with length 'len' to the ANYOF node's unicode
     * alternate list, pointed to by 'alternate_ptr'.  This is an array of
     * the multi-character folds of characters in the node */
    SV *sv;

    PERL_ARGS_ASSERT_ADD_ALTERNATE;

    if (! *alternate_ptr) {
	*alternate_ptr = newAV();
    }
    sv = newSVpvn_utf8((char*)string, len, TRUE);
    av_push(*alternate_ptr, sv);
    return;
}

/*
   parse a class specification and produce either an ANYOF node that
   matches the pattern or perhaps will be optimized into an EXACTish node
   instead. The node contains a bit map for the first 256 characters, with the
   corresponding bit set if that character is in the list.  For characters
   above 255, a range list is used */

STATIC regnode *
S_regclass(pTHX_ RExC_state_t *pRExC_state, U32 depth)
{
    dVAR;
    register UV nextvalue;
    register IV prevvalue = OOB_UNICODE;
    register IV range = 0;
    UV value = 0; /* XXX:dmq: needs to be referenceable (unfortunately) */
    register regnode *ret;
    STRLEN numlen;
    IV namedclass;
    char *rangebegin = NULL;
    bool need_class = 0;
    bool allow_full_fold = TRUE;   /* Assume wants multi-char folding */
    SV *listsv = NULL;
    STRLEN initial_listsv_len = 0; /* Kind of a kludge to see if it is more
				      than just initialized.  */
    SV* properties = NULL;    /* Code points that match \p{} \P{} */
    UV element_count = 0;   /* Number of distinct elements in the class.
			       Optimizations may be possible if this is tiny */
    UV n;

    /* Unicode properties are stored in a swash; this holds the current one
     * being parsed.  If this swash is the only above-latin1 component of the
     * character class, an optimization is to pass it directly on to the
     * execution engine.  Otherwise, it is set to NULL to indicate that there
     * are other things in the class that have to be dealt with at execution
     * time */
    SV* swash = NULL;		/* Code points that match \p{} \P{} */

    /* Set if a component of this character class is user-defined; just passed
     * on to the engine */
    UV has_user_defined_property = 0;

    /* code points this node matches that can't be stored in the bitmap */
    SV* nonbitmap = NULL;

    /* The items that are to match that aren't stored in the bitmap, but are a
     * result of things that are stored there.  This is the fold closure of
     * such a character, either because it has DEPENDS semantics and shouldn't
     * be matched unless the target string is utf8, or is a code point that is
     * too large for the bit map, as for example, the fold of the MICRO SIGN is
     * above 255.  This all is solely for performance reasons.  By having this
     * code know the outside-the-bitmap folds that the bitmapped characters are
     * involved with, we don't have to go out to disk to find the list of
     * matches, unless the character class includes code points that aren't
     * storable in the bit map.  That means that a character class with an 's'
     * in it, for example, doesn't need to go out to disk to find everything
     * that matches.  A 2nd list is used so that the 'nonbitmap' list is kept
     * empty unless there is something whose fold we don't know about, and will
     * have to go out to the disk to find. */
    SV* l1_fold_invlist = NULL;

    /* List of multi-character folds that are matched by this node */
    AV* unicode_alternate  = NULL;
#ifdef EBCDIC
    UV literal_endpoint = 0;
#endif
    UV stored = 0;  /* how many chars stored in the bitmap */

    regnode * const orig_emit = RExC_emit; /* Save the original RExC_emit in
        case we need to change the emitted regop to an EXACT. */
    const char * orig_parse = RExC_parse;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGCLASS;
#ifndef DEBUGGING
    PERL_UNUSED_ARG(depth);
#endif

    DEBUG_PARSE("clas");

    /* Assume we are going to generate an ANYOF node. */
    ret = reganode(pRExC_state, ANYOF, 0);


    if (!SIZE_ONLY) {
	ANYOF_FLAGS(ret) = 0;
    }

    if (UCHARAT(RExC_parse) == '^') {	/* Complement of range. */
	RExC_naughty++;
	RExC_parse++;
	if (!SIZE_ONLY)
	    ANYOF_FLAGS(ret) |= ANYOF_INVERT;

        /* We have decided to not allow multi-char folds in inverted character
	 * classes, due to the confusion that can happen, especially with
	 * classes that are designed for a non-Unicode world:  You have the
	 * peculiar case that:
            "s s" =~ /^[^\xDF]+$/i => Y
            "ss"  =~ /^[^\xDF]+$/i => N
         *
         * See [perl #89750] */
        allow_full_fold = FALSE;
    }

    if (SIZE_ONLY) {
	RExC_size += ANYOF_SKIP;
	listsv = &PL_sv_undef; /* For code scanners: listsv always non-NULL. */
    }
    else {
 	RExC_emit += ANYOF_SKIP;
	if (LOC) {
	    ANYOF_FLAGS(ret) |= ANYOF_LOCALE;
	}
	ANYOF_BITMAP_ZERO(ret);
	listsv = newSVpvs("# comment\n");
	initial_listsv_len = SvCUR(listsv);
    }

    nextvalue = RExC_parse < RExC_end ? UCHARAT(RExC_parse) : 0;

    if (!SIZE_ONLY && POSIXCC(nextvalue))
	checkposixcc(pRExC_state);

    /* allow 1st char to be ] (allowing it to be - is dealt with later) */
    if (UCHARAT(RExC_parse) == ']')
	goto charclassloop;

parseit:
    while (RExC_parse < RExC_end && UCHARAT(RExC_parse) != ']') {

    charclassloop:

	namedclass = OOB_NAMEDCLASS; /* initialize as illegal */

	if (!range) {
	    rangebegin = RExC_parse;
	    element_count++;
	}
	if (UTF) {
	    value = utf8n_to_uvchr((U8*)RExC_parse,
				   RExC_end - RExC_parse,
				   &numlen, UTF8_ALLOW_DEFAULT);
	    RExC_parse += numlen;
	}
	else
	    value = UCHARAT(RExC_parse++);

	nextvalue = RExC_parse < RExC_end ? UCHARAT(RExC_parse) : 0;
	if (value == '[' && POSIXCC(nextvalue))
	    namedclass = regpposixcc(pRExC_state, value);
	else if (value == '\\') {
	    if (UTF) {
		value = utf8n_to_uvchr((U8*)RExC_parse,
				   RExC_end - RExC_parse,
				   &numlen, UTF8_ALLOW_DEFAULT);
		RExC_parse += numlen;
	    }
	    else
		value = UCHARAT(RExC_parse++);
	    /* Some compilers cannot handle switching on 64-bit integer
	     * values, therefore value cannot be an UV.  Yes, this will
	     * be a problem later if we want switch on Unicode.
	     * A similar issue a little bit later when switching on
	     * namedclass. --jhi */
	    switch ((I32)value) {
	    case 'w':	namedclass = ANYOF_ALNUM;	break;
	    case 'W':	namedclass = ANYOF_NALNUM;	break;
	    case 's':	namedclass = ANYOF_SPACE;	break;
	    case 'S':	namedclass = ANYOF_NSPACE;	break;
	    case 'd':	namedclass = ANYOF_DIGIT;	break;
	    case 'D':	namedclass = ANYOF_NDIGIT;	break;
	    case 'v':	namedclass = ANYOF_VERTWS;	break;
	    case 'V':	namedclass = ANYOF_NVERTWS;	break;
	    case 'h':	namedclass = ANYOF_HORIZWS;	break;
	    case 'H':	namedclass = ANYOF_NHORIZWS;	break;
            case 'N':  /* Handle \N{NAME} in class */
                {
                    /* We only pay attention to the first char of 
                    multichar strings being returned. I kinda wonder
                    if this makes sense as it does change the behaviour
                    from earlier versions, OTOH that behaviour was broken
                    as well. */
                    UV v; /* value is register so we cant & it /grrr */
                    if (reg_namedseq(pRExC_state, &v, NULL, depth)) {
                        goto parseit;
                    }
                    value= v; 
                }
                break;
	    case 'p':
	    case 'P':
		{
		char *e;
		if (RExC_parse >= RExC_end)
		    vFAIL2("Empty \\%c{}", (U8)value);
		if (*RExC_parse == '{') {
		    const U8 c = (U8)value;
		    e = strchr(RExC_parse++, '}');
                    if (!e)
                        vFAIL2("Missing right brace on \\%c{}", c);
		    while (isSPACE(UCHARAT(RExC_parse)))
		        RExC_parse++;
                    if (e == RExC_parse)
                        vFAIL2("Empty \\%c{}", c);
		    n = e - RExC_parse;
		    while (isSPACE(UCHARAT(RExC_parse + n - 1)))
		        n--;
		}
		else {
		    e = RExC_parse;
		    n = 1;
		}
		if (!SIZE_ONLY) {
                    SV** invlistsvp;
                    SV* invlist;
                    char* name;
		    if (UCHARAT(RExC_parse) == '^') {
			 RExC_parse++;
			 n--;
			 value = value == 'p' ? 'P' : 'p'; /* toggle */
			 while (isSPACE(UCHARAT(RExC_parse))) {
			      RExC_parse++;
			      n--;
			 }
		    }
                    /* Try to get the definition of the property into
                     * <invlist>.  If /i is in effect, the effective property
                     * will have its name be <__NAME_i>.  The design is
                     * discussed in commit
                     * 2f833f5208e26b208886e51e09e2c072b5eabb46 */
                    Newx(name, n + sizeof("_i__\n"), char);

                    sprintf(name, "%s%.*s%s\n",
                                    (FOLD) ? "__" : "",
                                    (int)n,
                                    RExC_parse,
                                    (FOLD) ? "_i" : ""
                    );

                    /* Look up the property name, and get its swash and
                     * inversion list, if the property is found  */
                    if (swash) {
                        SvREFCNT_dec(swash);
                    }
                    swash = _core_swash_init("utf8", name, &PL_sv_undef,
                                             1, /* binary */
                                             0, /* not tr/// */
                                             TRUE, /* this routine will handle
                                                      undefined properties */
                                             NULL, FALSE /* No inversion list */
                                            );
                    if (   ! swash
                        || ! SvROK(swash)
                        || ! SvTYPE(SvRV(swash)) == SVt_PVHV
                        || ! (invlistsvp =
				hv_fetchs(MUTABLE_HV(SvRV(swash)),
                                "INVLIST", FALSE))
                        || ! (invlist = *invlistsvp))
		    {
                        if (swash) {
                            SvREFCNT_dec(swash);
                            swash = NULL;
                        }

                        /* Here didn't find it.  It could be a user-defined
                         * property that will be available at run-time.  Add it
                         * to the list to look up then */
                        Perl_sv_catpvf(aTHX_ listsv, "%cutf8::%s\n",
                                        (value == 'p' ? '+' : '!'),
                                        name);
                        has_user_defined_property = 1;

                        /* We don't know yet, so have to assume that the
                         * property could match something in the Latin1 range,
                         * hence something that isn't utf8 */
                        ANYOF_FLAGS(ret) |= ANYOF_NONBITMAP_NON_UTF8;
                    }
                    else {

                        /* Here, did get the swash and its inversion list.  If
                         * the swash is from a user-defined property, then this
                         * whole character class should be regarded as such */
                        SV** user_defined_svp =
                                            hv_fetchs(MUTABLE_HV(SvRV(swash)),
                                                        "USER_DEFINED", FALSE);
                        if (user_defined_svp) {
                            has_user_defined_property
                                                    |= SvUV(*user_defined_svp);
                        }

                        /* Invert if asking for the complement */
                        if (value == 'P') {
			    _invlist_union_complement_2nd(properties, invlist, &properties);

                            /* The swash can't be used as-is, because we've
			     * inverted things; delay removing it to here after
			     * have copied its invlist above */
                            SvREFCNT_dec(swash);
                            swash = NULL;
                        }
                        else {
                            _invlist_union(properties, invlist, &properties);
			}
		    }
		    Safefree(name);
		}
		RExC_parse = e + 1;
		namedclass = ANYOF_MAX;  /* no official name, but it's named */

		/* \p means they want Unicode semantics */
		RExC_uni_semantics = 1;
		}
		break;
	    case 'n':	value = '\n';			break;
	    case 'r':	value = '\r';			break;
	    case 't':	value = '\t';			break;
	    case 'f':	value = '\f';			break;
	    case 'b':	value = '\b';			break;
	    case 'e':	value = ASCII_TO_NATIVE('\033');break;
	    case 'a':	value = ASCII_TO_NATIVE('\007');break;
	    case 'o':
		RExC_parse--;	/* function expects to be pointed at the 'o' */
		{
		    const char* error_msg;
		    bool valid = grok_bslash_o(RExC_parse,
					       &value,
					       &numlen,
					       &error_msg,
					       SIZE_ONLY);
		    RExC_parse += numlen;
		    if (! valid) {
			vFAIL(error_msg);
		    }
		}
		if (PL_encoding && value < 0x100) {
		    goto recode_encoding;
		}
		break;
	    case 'x':
		if (*RExC_parse == '{') {
                    I32 flags = PERL_SCAN_ALLOW_UNDERSCORES
                        | PERL_SCAN_DISALLOW_PREFIX;
		    char * const e = strchr(RExC_parse++, '}');
                    if (!e)
                        vFAIL("Missing right brace on \\x{}");

		    numlen = e - RExC_parse;
		    value = grok_hex(RExC_parse, &numlen, &flags, NULL);
		    RExC_parse = e + 1;
		}
		else {
                    I32 flags = PERL_SCAN_DISALLOW_PREFIX;
		    numlen = 2;
		    value = grok_hex(RExC_parse, &numlen, &flags, NULL);
		    RExC_parse += numlen;
		}
		if (PL_encoding && value < 0x100)
		    goto recode_encoding;
		break;
	    case 'c':
		value = grok_bslash_c(*RExC_parse++, UTF, SIZE_ONLY);
		break;
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7':
		{
		    /* Take 1-3 octal digits */
		    I32 flags = PERL_SCAN_SILENT_ILLDIGIT;
		    numlen = 3;
		    value = grok_oct(--RExC_parse, &numlen, &flags, NULL);
		    RExC_parse += numlen;
		    if (PL_encoding && value < 0x100)
			goto recode_encoding;
		    break;
		}
	    recode_encoding:
		if (! RExC_override_recoding) {
		    SV* enc = PL_encoding;
		    value = reg_recode((const char)(U8)value, &enc);
		    if (!enc && SIZE_ONLY)
			ckWARNreg(RExC_parse,
				  "Invalid escape in the specified encoding");
		    break;
		}
	    default:
		/* Allow \_ to not give an error */
		if (!SIZE_ONLY && isALNUM(value) && value != '_') {
		    ckWARN2reg(RExC_parse,
			       "Unrecognized escape \\%c in character class passed through",
			       (int)value);
		}
		break;
	    }
	} /* end of \blah */
#ifdef EBCDIC
	else
	    literal_endpoint++;
#endif

	if (namedclass > OOB_NAMEDCLASS) { /* this is a named class \blah */

	    /* What matches in a locale is not known until runtime, so need to
	     * (one time per class) allocate extra space to pass to regexec.
	     * The space will contain a bit for each named class that is to be
	     * matched against.  This isn't needed for \p{} and pseudo-classes,
	     * as they are not affected by locale, and hence are dealt with
	     * separately */
	    if (LOC && namedclass < ANYOF_MAX && ! need_class) {
		need_class = 1;
		if (SIZE_ONLY) {
		    RExC_size += ANYOF_CLASS_SKIP - ANYOF_SKIP;
		}
		else {
		    RExC_emit += ANYOF_CLASS_SKIP - ANYOF_SKIP;
		    ANYOF_CLASS_ZERO(ret);
		}
		ANYOF_FLAGS(ret) |= ANYOF_CLASS;
	    }

	    /* a bad range like a-\d, a-[:digit:].  The '-' is taken as a
	     * literal, as is the character that began the false range, i.e.
	     * the 'a' in the examples */
	    if (range) {
		if (!SIZE_ONLY) {
		    const int w =
			RExC_parse >= rangebegin ?
			RExC_parse - rangebegin : 0;
		    ckWARN4reg(RExC_parse,
			       "False [] range \"%*.*s\"",
			       w, w, rangebegin);

		    stored +=
                         set_regclass_bit(pRExC_state, ret, '-', &l1_fold_invlist, &unicode_alternate);
		    if (prevvalue < 256) {
			stored +=
                         set_regclass_bit(pRExC_state, ret, (U8) prevvalue, &l1_fold_invlist, &unicode_alternate);
		    }
		    else {
			nonbitmap = add_cp_to_invlist(nonbitmap, prevvalue);
		    }
		}

		range = 0; /* this was not a true range */
	    }

	    if (!SIZE_ONLY) {

		/* Possible truncation here but in some 64-bit environments
		 * the compiler gets heartburn about switch on 64-bit values.
		 * A similar issue a little earlier when switching on value.
		 * --jhi */
		switch ((I32)namedclass) {
		    int i;  /* loop counter */

		case ANYOF_ALNUMC: /* C's alnum, in contrast to \w */
		    DO_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                        PL_PosixAlnum, PL_L1PosixAlnum, "XPosixAlnum", listsv);
		    break;
		case ANYOF_NALNUMC:
		    DO_N_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                        PL_PosixAlnum, PL_L1PosixAlnum, "XPosixAlnum", listsv);
		    break;
		case ANYOF_ALPHA:
		    DO_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                        PL_PosixAlpha, PL_L1PosixAlpha, "XPosixAlpha", listsv);
		    break;
		case ANYOF_NALPHA:
		    DO_N_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                        PL_PosixAlpha, PL_L1PosixAlpha, "XPosixAlpha", listsv);
		    break;
		case ANYOF_ASCII:
		    if (LOC) {
			ANYOF_CLASS_SET(ret, namedclass);
		    }
                    else {
                        _invlist_union(properties, PL_ASCII, &properties);
                    }
		    break;
		case ANYOF_NASCII:
		    if (LOC) {
			ANYOF_CLASS_SET(ret, namedclass);
		    }
                    else {
                        _invlist_union_complement_2nd(properties,
                                                    PL_ASCII, &properties);
                        if (DEPENDS_SEMANTICS) {
                            ANYOF_FLAGS(ret) |= ANYOF_NON_UTF8_LATIN1_ALL;
                        }
                    }
		    break;
		case ANYOF_BLANK:
                    DO_POSIX(ret, namedclass, properties,
                                            PL_PosixBlank, PL_XPosixBlank);
		    break;
		case ANYOF_NBLANK:
                    DO_N_POSIX(ret, namedclass, properties,
                                            PL_PosixBlank, PL_XPosixBlank);
		    break;
		case ANYOF_CNTRL:
                    DO_POSIX(ret, namedclass, properties,
                                            PL_PosixCntrl, PL_XPosixCntrl);
		    break;
		case ANYOF_NCNTRL:
                    DO_N_POSIX(ret, namedclass, properties,
                                            PL_PosixCntrl, PL_XPosixCntrl);
		    break;
		case ANYOF_DIGIT:
		    /* Ignore the compiler warning for this macro, planned to
		     * be eliminated later */
		    DO_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                        PL_PosixDigit, PL_PosixDigit, "XPosixDigit", listsv);
		    break;
		case ANYOF_NDIGIT:
		    DO_N_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                        PL_PosixDigit, PL_PosixDigit, "XPosixDigit", listsv);
		    break;
		case ANYOF_GRAPH:
		    DO_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                        PL_PosixGraph, PL_L1PosixGraph, "XPosixGraph", listsv);
		    break;
		case ANYOF_NGRAPH:
		    DO_N_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                        PL_PosixGraph, PL_L1PosixGraph, "XPosixGraph", listsv);
		    break;
		case ANYOF_HORIZWS:
		    /* NBSP matches this, and needs to be added unconditionally
		     * to the bit map as it matches even under /d, unlike all
		     * the rest of the Posix-like classes (\v doesn't have any
		     * matches in the Latin1 range, so it is unaffected.) which
		     * Otherwise, we use the nonbitmap, as /d doesn't make a
		     * difference in what these match.  It turns out that \h is
		     * just a synonym for XPosixBlank */
		    _invlist_union(nonbitmap, PL_XPosixBlank, &nonbitmap);
		    stored += set_regclass_bit(pRExC_state, ret,
					       UNI_TO_NATIVE(0xA0),
					       &l1_fold_invlist,
					       &unicode_alternate);

		    break;
		case ANYOF_NHORIZWS:
                    _invlist_union_complement_2nd(nonbitmap,
                                                 PL_XPosixBlank, &nonbitmap);
		    for (i = 128; i < 256; i++) {
			if (i == 0xA0) {
			    continue;
			}
			stored += set_regclass_bit(pRExC_state, ret,
						   UNI_TO_NATIVE(i),
						   &l1_fold_invlist,
						   &unicode_alternate);
		    }
		    break;
		case ANYOF_LOWER:
		case ANYOF_NLOWER:
                {   /* These require special handling, as they differ under
		       folding, matching Cased there (which in the ASCII range
		       is the same as Alpha */

		    SV* ascii_source;
		    SV* l1_source;
		    const char *Xname;

		    if (FOLD && ! LOC) {
			ascii_source = PL_PosixAlpha;
			l1_source = PL_L1Cased;
			Xname = "Cased";
		    }
		    else {
			ascii_source = PL_PosixLower;
			l1_source = PL_L1PosixLower;
			Xname = "XPosixLower";
		    }
		    if (namedclass == ANYOF_LOWER) {
			DO_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                                    ascii_source, l1_source, Xname, listsv);
		    }
		    else {
			DO_N_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass,
                            properties, ascii_source, l1_source, Xname, listsv);
		    }
		    break;
		}
		case ANYOF_PRINT:
		    DO_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                        PL_PosixPrint, PL_L1PosixPrint, "XPosixPrint", listsv);
		    break;
		case ANYOF_NPRINT:
		    DO_N_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                        PL_PosixPrint, PL_L1PosixPrint, "XPosixPrint", listsv);
		    break;
		case ANYOF_PUNCT:
		    DO_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                        PL_PosixPunct, PL_L1PosixPunct, "XPosixPunct", listsv);
		    break;
		case ANYOF_NPUNCT:
		    DO_N_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                        PL_PosixPunct, PL_L1PosixPunct, "XPosixPunct", listsv);
		    break;
		case ANYOF_PSXSPC:
                    DO_POSIX(ret, namedclass, properties,
                                            PL_PosixSpace, PL_XPosixSpace);
		    break;
		case ANYOF_NPSXSPC:
                    DO_N_POSIX(ret, namedclass, properties,
                                            PL_PosixSpace, PL_XPosixSpace);
		    break;
		case ANYOF_SPACE:
                    DO_POSIX(ret, namedclass, properties,
                                            PL_PerlSpace, PL_XPerlSpace);
		    break;
		case ANYOF_NSPACE:
                    DO_N_POSIX(ret, namedclass, properties,
                                            PL_PerlSpace, PL_XPerlSpace);
		    break;
		case ANYOF_UPPER:   /* Same as LOWER, above */
		case ANYOF_NUPPER:
		{
		    SV* ascii_source;
		    SV* l1_source;
		    const char *Xname;

		    if (FOLD && ! LOC) {
			ascii_source = PL_PosixAlpha;
			l1_source = PL_L1Cased;
			Xname = "Cased";
		    }
		    else {
			ascii_source = PL_PosixUpper;
			l1_source = PL_L1PosixUpper;
			Xname = "XPosixUpper";
		    }
		    if (namedclass == ANYOF_UPPER) {
			DO_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                                    ascii_source, l1_source, Xname, listsv);
		    }
		    else {
			DO_N_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass,
                        properties, ascii_source, l1_source, Xname, listsv);
		    }
		    break;
		}
		case ANYOF_ALNUM:   /* Really is 'Word' */
		    DO_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                            PL_PosixWord, PL_L1PosixWord, "XPosixWord", listsv);
		    break;
		case ANYOF_NALNUM:
		    DO_N_POSIX_LATIN1_ONLY_KNOWN(ret, namedclass, properties,
                            PL_PosixWord, PL_L1PosixWord, "XPosixWord", listsv);
		    break;
		case ANYOF_VERTWS:
		    /* For these, we use the nonbitmap, as /d doesn't make a
		     * difference in what these match.  There would be problems
		     * if these characters had folds other than themselves, as
		     * nonbitmap is subject to folding */
		    _invlist_union(nonbitmap, PL_VertSpace, &nonbitmap);
		    break;
		case ANYOF_NVERTWS:
                    _invlist_union_complement_2nd(nonbitmap,
                                                    PL_VertSpace, &nonbitmap);
		    break;
		case ANYOF_XDIGIT:
                    DO_POSIX(ret, namedclass, properties,
                                            PL_PosixXDigit, PL_XPosixXDigit);
		    break;
		case ANYOF_NXDIGIT:
                    DO_N_POSIX(ret, namedclass, properties,
                                            PL_PosixXDigit, PL_XPosixXDigit);
		    break;
		case ANYOF_MAX:
		    /* this is to handle \p and \P */
		    break;
		default:
		    vFAIL("Invalid [::] class");
		    break;
		}

		continue;
	    }
	} /* end of namedclass \blah */

	if (range) {
	    if (prevvalue > (IV)value) /* b-a */ {
		const int w = RExC_parse - rangebegin;
		Simple_vFAIL4("Invalid [] range \"%*.*s\"", w, w, rangebegin);
		range = 0; /* not a valid range */
	    }
	}
	else {
	    prevvalue = value; /* save the beginning of the range */
	    if (RExC_parse+1 < RExC_end
		&& *RExC_parse == '-'
		&& RExC_parse[1] != ']')
	    {
		RExC_parse++;

		/* a bad range like \w-, [:word:]- ? */
		if (namedclass > OOB_NAMEDCLASS) {
		    if (ckWARN(WARN_REGEXP)) {
			const int w =
			    RExC_parse >= rangebegin ?
			    RExC_parse - rangebegin : 0;
			vWARN4(RExC_parse,
			       "False [] range \"%*.*s\"",
			       w, w, rangebegin);
		    }
		    if (!SIZE_ONLY)
			stored +=
                            set_regclass_bit(pRExC_state, ret, '-', &l1_fold_invlist, &unicode_alternate);
		} else
		    range = 1;	/* yeah, it's a range! */
		continue;	/* but do it the next time */
	    }
	}

	/* non-Latin1 code point implies unicode semantics.  Must be set in
	 * pass1 so is there for the whole of pass 2 */
	if (value > 255) {
	    RExC_uni_semantics = 1;
	}

	/* now is the next time */
	if (!SIZE_ONLY) {
	    if (prevvalue < 256) {
	        const IV ceilvalue = value < 256 ? value : 255;
		IV i;
#ifdef EBCDIC
		/* In EBCDIC [\x89-\x91] should include
		 * the \x8e but [i-j] should not. */
		if (literal_endpoint == 2 &&
		    ((isLOWER(prevvalue) && isLOWER(ceilvalue)) ||
		     (isUPPER(prevvalue) && isUPPER(ceilvalue))))
		{
		    if (isLOWER(prevvalue)) {
			for (i = prevvalue; i <= ceilvalue; i++)
			    if (isLOWER(i) && !ANYOF_BITMAP_TEST(ret,i)) {
				stored +=
                                  set_regclass_bit(pRExC_state, ret, (U8) i, &l1_fold_invlist, &unicode_alternate);
			    }
		    } else {
			for (i = prevvalue; i <= ceilvalue; i++)
			    if (isUPPER(i) && !ANYOF_BITMAP_TEST(ret,i)) {
				stored +=
                                  set_regclass_bit(pRExC_state, ret, (U8) i, &l1_fold_invlist, &unicode_alternate);
			    }
		    }
		}
		else
#endif
		      for (i = prevvalue; i <= ceilvalue; i++) {
			stored += set_regclass_bit(pRExC_state, ret, (U8) i, &l1_fold_invlist, &unicode_alternate);
	              }
	  }
	  if (value > 255) {
	    const UV prevnatvalue  = NATIVE_TO_UNI(prevvalue);
	    const UV natvalue      = NATIVE_TO_UNI(value);
	    nonbitmap = _add_range_to_invlist(nonbitmap, prevnatvalue, natvalue);
	}
#ifdef EBCDIC
	    literal_endpoint = 0;
#endif
        }

	range = 0; /* this range (if it was one) is done now */
    }



    if (SIZE_ONLY)
        return ret;
    /****** !SIZE_ONLY AFTER HERE *********/

    /* If folding and there are code points above 255, we calculate all
     * characters that could fold to or from the ones already on the list */
    if (FOLD && nonbitmap) {
	UV start, end;	/* End points of code point ranges */

	SV* fold_intersection = NULL;

	/* This is a list of all the characters that participate in folds
	    * (except marks, etc in multi-char folds */
	if (! PL_utf8_foldable) {
	    SV* swash = swash_init("utf8", "Cased", &PL_sv_undef, 1, 0);
	    PL_utf8_foldable = _swash_to_invlist(swash);
            SvREFCNT_dec(swash);
	}

	/* This is a hash that for a particular fold gives all characters
	    * that are involved in it */
	if (! PL_utf8_foldclosures) {

	    /* If we were unable to find any folds, then we likely won't be
	     * able to find the closures.  So just create an empty list.
	     * Folding will effectively be restricted to the non-Unicode rules
	     * hard-coded into Perl.  (This case happens legitimately during
	     * compilation of Perl itself before the Unicode tables are
	     * generated) */
	    if (invlist_len(PL_utf8_foldable) == 0) {
		PL_utf8_foldclosures = newHV();
	    } else {
		/* If the folds haven't been read in, call a fold function
		    * to force that */
		if (! PL_utf8_tofold) {
		    U8 dummy[UTF8_MAXBYTES+1];
		    STRLEN dummy_len;

		    /* This particular string is above \xff in both UTF-8 and
		     * UTFEBCDIC */
		    to_utf8_fold((U8*) "\xC8\x80", dummy, &dummy_len);
		    assert(PL_utf8_tofold); /* Verify that worked */
		}
		PL_utf8_foldclosures = _swash_inversion_hash(PL_utf8_tofold);
	    }
	}

	/* Only the characters in this class that participate in folds need be
	 * checked.  Get the intersection of this class and all the possible
	 * characters that are foldable.  This can quickly narrow down a large
	 * class */
	_invlist_intersection(PL_utf8_foldable, nonbitmap, &fold_intersection);

	/* Now look at the foldable characters in this class individually */
	invlist_iterinit(fold_intersection);
	while (invlist_iternext(fold_intersection, &start, &end)) {
	    UV j;

	    /* Look at every character in the range */
	    for (j = start; j <= end; j++) {

		/* Get its fold */
		U8 foldbuf[UTF8_MAXBYTES_CASE+1];
		STRLEN foldlen;
		const UV f =
                    _to_uni_fold_flags(j, foldbuf, &foldlen, allow_full_fold);

		if (foldlen > (STRLEN)UNISKIP(f)) {

		    /* Any multicharacter foldings (disallowed in lookbehind
		     * patterns) require the following transform: [ABCDEF] ->
		     * (?:[ABCabcDEFd]|pq|rst) where E folds into "pq" and F
		     * folds into "rst", all other characters fold to single
		     * characters.  We save away these multicharacter foldings,
		     * to be later saved as part of the additional "s" data. */
		    if (! RExC_in_lookbehind) {
			U8* loc = foldbuf;
			U8* e = foldbuf + foldlen;

			/* If any of the folded characters of this are in the
			 * Latin1 range, tell the regex engine that this can
			 * match a non-utf8 target string.  The only multi-byte
			 * fold whose source is in the Latin1 range (U+00DF)
			 * applies only when the target string is utf8, or
			 * under unicode rules */
			if (j > 255 || AT_LEAST_UNI_SEMANTICS) {
			    while (loc < e) {

				/* Can't mix ascii with non- under /aa */
				if (MORE_ASCII_RESTRICTED
				    && (isASCII(*loc) != isASCII(j)))
				{
				    goto end_multi_fold;
				}
				if (UTF8_IS_INVARIANT(*loc)
				    || UTF8_IS_DOWNGRADEABLE_START(*loc))
				{
                                    /* Can't mix above and below 256 under LOC
                                     */
				    if (LOC) {
					goto end_multi_fold;
				    }
				    ANYOF_FLAGS(ret)
					    |= ANYOF_NONBITMAP_NON_UTF8;
				    break;
				}
				loc += UTF8SKIP(loc);
			    }
			}

			add_alternate(&unicode_alternate, foldbuf, foldlen);
		    end_multi_fold: ;
		    }

		    /* This is special-cased, as it is the only letter which
		     * has both a multi-fold and single-fold in Latin1.  All
		     * the other chars that have single and multi-folds are
		     * always in utf8, and the utf8 folding algorithm catches
		     * them */
		    if (! LOC && j == LATIN_CAPITAL_LETTER_SHARP_S) {
			stored += set_regclass_bit(pRExC_state,
					ret,
					LATIN_SMALL_LETTER_SHARP_S,
					&l1_fold_invlist, &unicode_alternate);
		    }
		}
		else {
		    /* Single character fold.  Add everything in its fold
		     * closure to the list that this node should match */
		    SV** listp;

		    /* The fold closures data structure is a hash with the keys
		     * being every character that is folded to, like 'k', and
		     * the values each an array of everything that folds to its
		     * key.  e.g. [ 'k', 'K', KELVIN_SIGN ] */
		    if ((listp = hv_fetch(PL_utf8_foldclosures,
				    (char *) foldbuf, foldlen, FALSE)))
		    {
			AV* list = (AV*) *listp;
			IV k;
			for (k = 0; k <= av_len(list); k++) {
			    SV** c_p = av_fetch(list, k, FALSE);
			    UV c;
			    if (c_p == NULL) {
				Perl_croak(aTHX_ "panic: invalid PL_utf8_foldclosures structure");
			    }
			    c = SvUV(*c_p);

			    /* /aa doesn't allow folds between ASCII and non-;
			     * /l doesn't allow them between above and below
			     * 256 */
			    if ((MORE_ASCII_RESTRICTED
				 && (isASCII(c) != isASCII(j)))
				    || (LOC && ((c < 256) != (j < 256))))
			    {
				continue;
			    }

			    if (c < 256 && AT_LEAST_UNI_SEMANTICS) {
				stored += set_regclass_bit(pRExC_state,
					ret,
					(U8) c,
					&l1_fold_invlist, &unicode_alternate);
			    }
				/* It may be that the code point is already in
				 * this range or already in the bitmap, in
				 * which case we need do nothing */
			    else if ((c < start || c > end)
					&& (c > 255
					    || ! ANYOF_BITMAP_TEST(ret, c)))
			    {
				nonbitmap = add_cp_to_invlist(nonbitmap, c);
			    }
			}
		    }
		}
	    }
	}
	SvREFCNT_dec(fold_intersection);
    }

    /* Combine the two lists into one. */
    if (l1_fold_invlist) {
	if (nonbitmap) {
	    _invlist_union(nonbitmap, l1_fold_invlist, &nonbitmap);
	    SvREFCNT_dec(l1_fold_invlist);
	}
	else {
	    nonbitmap = l1_fold_invlist;
	}
    }

    /* And combine the result (if any) with any inversion list from properties.
     * The lists are kept separate up to now because we don't want to fold the
     * properties */
    if (properties) {
	if (nonbitmap) {
	    _invlist_union(nonbitmap, properties, &nonbitmap);
	    SvREFCNT_dec(properties);
	}
	else {
	    nonbitmap = properties;
	}
    }

    /* Here, <nonbitmap> contains all the code points we can determine at
     * compile time that we haven't put into the bitmap.  Go through it, and
     * for things that belong in the bitmap, put them there, and delete from
     * <nonbitmap> */
    if (nonbitmap) {

	/* Above-ASCII code points in /d have to stay in <nonbitmap>, as they
	 * possibly only should match when the target string is UTF-8 */
	UV max_cp_to_set = (DEPENDS_SEMANTICS) ? 127 : 255;

	/* This gets set if we actually need to modify things */
	bool change_invlist = FALSE;

	UV start, end;

	/* Start looking through <nonbitmap> */
	invlist_iterinit(nonbitmap);
	while (invlist_iternext(nonbitmap, &start, &end)) {
	    UV high;
	    int i;

	    /* Quit if are above what we should change */
	    if (start > max_cp_to_set) {
		break;
	    }

	    change_invlist = TRUE;

	    /* Set all the bits in the range, up to the max that we are doing */
	    high = (end < max_cp_to_set) ? end : max_cp_to_set;
	    for (i = start; i <= (int) high; i++) {
		if (! ANYOF_BITMAP_TEST(ret, i)) {
		    ANYOF_BITMAP_SET(ret, i);
		    stored++;
		    prevvalue = value;
		    value = i;
		}
	    }
	}

        /* Done with loop; remove any code points that are in the bitmap from
         * <nonbitmap> */
	if (change_invlist) {
	    _invlist_subtract(nonbitmap,
		              (DEPENDS_SEMANTICS)
			        ? PL_ASCII
			        : PL_Latin1,
                              &nonbitmap);
	}

	/* If have completely emptied it, remove it completely */
	if (invlist_len(nonbitmap) == 0) {
	    SvREFCNT_dec(nonbitmap);
	    nonbitmap = NULL;
	}
    }

    /* Here, we have calculated what code points should be in the character
     * class.  <nonbitmap> does not overlap the bitmap except possibly in the
     * case of DEPENDS rules.
     *
     * Now we can see about various optimizations.  Fold calculation (which we
     * did above) needs to take place before inversion.  Otherwise /[^k]/i
     * would invert to include K, which under /i would match k, which it
     * shouldn't. */

    /* Optimize inverted simple patterns (e.g. [^a-z]).  Note that we haven't
     * set the FOLD flag yet, so this does optimize those.  It doesn't
     * optimize locale.  Doing so perhaps could be done as long as there is
     * nothing like \w in it; some thought also would have to be given to the
     * interaction with above 0x100 chars */
    if ((ANYOF_FLAGS(ret) & ANYOF_INVERT)
        && ! LOC
	&& ! unicode_alternate
	/* In case of /d, there are some things that should match only when in
	 * not in the bitmap, i.e., they require UTF8 to match.  These are
	 * listed in nonbitmap, but if ANYOF_NONBITMAP_NON_UTF8 is set in this
	 * case, they don't require UTF8, so can invert here */
	&& (! nonbitmap
	    || ! DEPENDS_SEMANTICS
	    || (ANYOF_FLAGS(ret) & ANYOF_NONBITMAP_NON_UTF8))
	&& SvCUR(listsv) == initial_listsv_len)
    {
	int i;
	if (! nonbitmap) {
	    for (i = 0; i < 256; ++i) {
		if (ANYOF_BITMAP_TEST(ret, i)) {
		    ANYOF_BITMAP_CLEAR(ret, i);
		}
		else {
		    ANYOF_BITMAP_SET(ret, i);
		    prevvalue = value;
		    value = i;
		}
	    }
	    /* The inversion means that everything above 255 is matched */
	    ANYOF_FLAGS(ret) |= ANYOF_UNICODE_ALL;
	}
	else {
	    /* Here, also has things outside the bitmap that may overlap with
	     * the bitmap.  We have to sync them up, so that they get inverted
	     * in both places.  Earlier, we removed all overlaps except in the
	     * case of /d rules, so no syncing is needed except for this case
	     */
	    SV *remove_list = NULL;

	    if (DEPENDS_SEMANTICS) {
		UV start, end;

		/* Set the bits that correspond to the ones that aren't in the
		 * bitmap.  Otherwise, when we invert, we'll miss these.
		 * Earlier, we removed from the nonbitmap all code points
		 * < 128, so there is no extra work here */
		invlist_iterinit(nonbitmap);
		while (invlist_iternext(nonbitmap, &start, &end)) {
		    if (start > 255) {  /* The bit map goes to 255 */
			break;
		    }
		    if (end > 255) {
			end = 255;
		    }
		    for (i = start; i <= (int) end; ++i) {
			ANYOF_BITMAP_SET(ret, i);
			prevvalue = value;
			value = i;
		    }
		}
	    }

	    /* Now invert both the bitmap and the nonbitmap.  Anything in the
	     * bitmap has to also be removed from the non-bitmap, but again,
	     * there should not be overlap unless is /d rules. */
	    _invlist_invert(nonbitmap);

	    /* Any swash can't be used as-is, because we've inverted things */
	    if (swash) {
		SvREFCNT_dec(swash);
		swash = NULL;
	    }

	    for (i = 0; i < 256; ++i) {
		if (ANYOF_BITMAP_TEST(ret, i)) {
		    ANYOF_BITMAP_CLEAR(ret, i);
		    if (DEPENDS_SEMANTICS) {
			if (! remove_list) {
			    remove_list = _new_invlist(2);
			}
			remove_list = add_cp_to_invlist(remove_list, i);
		    }
		}
		else {
		    ANYOF_BITMAP_SET(ret, i);
		    prevvalue = value;
		    value = i;
		}
	    }

	    /* And do the removal */
	    if (DEPENDS_SEMANTICS) {
		if (remove_list) {
		    _invlist_subtract(nonbitmap, remove_list, &nonbitmap);
		    SvREFCNT_dec(remove_list);
		}
	    }
	    else {
		/* There is no overlap for non-/d, so just delete anything
		 * below 256 */
		_invlist_intersection(nonbitmap, PL_AboveLatin1, &nonbitmap);
	    }
	}

	stored = 256 - stored;

	/* Clear the invert flag since have just done it here */
	ANYOF_FLAGS(ret) &= ~ANYOF_INVERT;
    }

    /* Folding in the bitmap is taken care of above, but not for locale (for
     * which we have to wait to see what folding is in effect at runtime), and
     * for some things not in the bitmap (only the upper latin folds in this
     * case, as all other single-char folding has been set above).  Set
     * run-time fold flag for these */
    if (FOLD && (LOC
		|| (DEPENDS_SEMANTICS
		    && nonbitmap
		    && ! (ANYOF_FLAGS(ret) & ANYOF_NONBITMAP_NON_UTF8))
		|| unicode_alternate))
    {
	ANYOF_FLAGS(ret) |= ANYOF_LOC_NONBITMAP_FOLD;
    }

    /* A single character class can be "optimized" into an EXACTish node.
     * Note that since we don't currently count how many characters there are
     * outside the bitmap, we are XXX missing optimization possibilities for
     * them.  This optimization can't happen unless this is a truly single
     * character class, which means that it can't be an inversion into a
     * many-character class, and there must be no possibility of there being
     * things outside the bitmap.  'stored' (only) for locales doesn't include
     * \w, etc, so have to make a special test that they aren't present
     *
     * Similarly A 2-character class of the very special form like [bB] can be
     * optimized into an EXACTFish node, but only for non-locales, and for
     * characters which only have the two folds; so things like 'fF' and 'Ii'
     * wouldn't work because they are part of the fold of 'LATIN SMALL LIGATURE
     * FI'. */
    if (! nonbitmap
	&& ! unicode_alternate
	&& SvCUR(listsv) == initial_listsv_len
	&& ! (ANYOF_FLAGS(ret) & (ANYOF_INVERT|ANYOF_UNICODE_ALL))
        && (((stored == 1 && ((! (ANYOF_FLAGS(ret) & ANYOF_LOCALE))
                              || (! ANYOF_CLASS_TEST_ANY_SET(ret)))))
	    || (stored == 2 && ((! (ANYOF_FLAGS(ret) & ANYOF_LOCALE))
				 && (! _HAS_NONLATIN1_FOLD_CLOSURE_ONLY_FOR_USE_BY_REGCOMP_DOT_C_AND_REGEXEC_DOT_C(value))
				 /* If the latest code point has a fold whose
				  * bit is set, it must be the only other one */
				&& ((prevvalue = PL_fold_latin1[value]) != (IV)value)
				 && ANYOF_BITMAP_TEST(ret, prevvalue)))))
    {
        /* Note that the information needed to decide to do this optimization
         * is not currently available until the 2nd pass, and that the actually
	 * used EXACTish node takes less space than the calculated ANYOF node,
	 * and hence the amount of space calculated in the first pass is larger
         * than actually used, so this optimization doesn't gain us any space.
	 * But an EXACT node is faster than an ANYOF node, and can be combined
	 * with any adjacent EXACT nodes later by the optimizer for further
	 * gains.  The speed of executing an EXACTF is similar to an ANYOF
	 * node, so the optimization advantage comes from the ability to join
	 * it to adjacent EXACT nodes */

        const char * cur_parse= RExC_parse;
	U8 op;
        RExC_emit = (regnode *)orig_emit;
        RExC_parse = (char *)orig_parse;

	if (stored == 1) {

	    /* A locale node with one point can be folded; all the other cases
	     * with folding will have two points, since we calculate them above
	     */
	    if (ANYOF_FLAGS(ret) & ANYOF_LOC_NONBITMAP_FOLD) {
		 op = EXACTFL;
	    }
	    else {
		op = EXACT;
	    }
	}
	else {   /* else 2 chars in the bit map: the folds of each other */

	    /* Use the folded value, which for the cases where we get here,
	     * is just the lower case of the current one (which may resolve to
	     * itself, or to the other one */
	    value = toLOWER_LATIN1(value);

	    /* To join adjacent nodes, they must be the exact EXACTish type.
	     * Try to use the most likely type, by using EXACTFA if possible,
	     * then EXACTFU if the regex calls for it, or is required because
	     * the character is non-ASCII.  (If <value> is ASCII, its fold is
	     * also ASCII for the cases where we get here.) */
	    if (MORE_ASCII_RESTRICTED && isASCII(value)) {
		op = EXACTFA;
	    }
	    else if (AT_LEAST_UNI_SEMANTICS || !isASCII(value)) {
		op = EXACTFU;
	    }
	    else {    /* Otherwise, more likely to be EXACTF type */
		op = EXACTF;
	    }
	}

	ret = reg_node(pRExC_state, op);
        RExC_parse = (char *)cur_parse;
	if (UTF && ! NATIVE_IS_INVARIANT(value)) {
	    *STRING(ret)= UTF8_EIGHT_BIT_HI((U8) value);
	    *(STRING(ret) + 1)= UTF8_EIGHT_BIT_LO((U8) value);
	    STR_LEN(ret)= 2;
	    RExC_emit += STR_SZ(2);
	}
	else {
	    *STRING(ret)= (char)value;
	    STR_LEN(ret)= 1;
	    RExC_emit += STR_SZ(1);
	}
	SvREFCNT_dec(listsv);
        return ret;
    }

    /* If there is a swash and more than one element, we can't use the swash in
     * the optimization below. */
    if (swash && element_count > 1) {
	SvREFCNT_dec(swash);
	swash = NULL;
    }
    if (! nonbitmap
	&& SvCUR(listsv) == initial_listsv_len
	&& ! unicode_alternate)
    {
	ARG_SET(ret, ANYOF_NONBITMAP_EMPTY);
	SvREFCNT_dec(listsv);
	SvREFCNT_dec(unicode_alternate);
    }
    else {
	/* av[0] stores the character class description in its textual form:
	 *       used later (regexec.c:Perl_regclass_swash()) to initialize the
	 *       appropriate swash, and is also useful for dumping the regnode.
	 * av[1] if NULL, is a placeholder to later contain the swash computed
	 *       from av[0].  But if no further computation need be done, the
	 *       swash is stored there now.
	 * av[2] stores the multicharacter foldings, used later in
	 *       regexec.c:S_reginclass().
	 * av[3] stores the nonbitmap inversion list for use in addition or
	 *       instead of av[0]; not used if av[1] isn't NULL
	 * av[4] is set if any component of the class is from a user-defined
	 *       property; not used if av[1] isn't NULL */
	AV * const av = newAV();
	SV *rv;

	av_store(av, 0, (SvCUR(listsv) == initial_listsv_len)
			? &PL_sv_undef
			: listsv);
	if (swash) {
	    av_store(av, 1, swash);
	    SvREFCNT_dec(nonbitmap);
	}
	else {
	    av_store(av, 1, NULL);
	    if (nonbitmap) {
		av_store(av, 3, nonbitmap);
		av_store(av, 4, newSVuv(has_user_defined_property));
	    }
	}

        /* Store any computed multi-char folds only if we are allowing
         * them */
        if (allow_full_fold) {
            av_store(av, 2, MUTABLE_SV(unicode_alternate));
            if (unicode_alternate) { /* This node is variable length */
                OP(ret) = ANYOFV;
            }
        }
        else {
            av_store(av, 2, NULL);
        }
	rv = newRV_noinc(MUTABLE_SV(av));
	n = add_data(pRExC_state, 1, "s");
	RExC_rxi->data->data[n] = (void*)rv;
	ARG_SET(ret, n);
    }
    return ret;
}


/* reg_skipcomment()

   Absorbs an /x style # comments from the input stream.
   Returns true if there is more text remaining in the stream.
   Will set the REG_SEEN_RUN_ON_COMMENT flag if the comment
   terminates the pattern without including a newline.

   Note its the callers responsibility to ensure that we are
   actually in /x mode

*/

STATIC bool
S_reg_skipcomment(pTHX_ RExC_state_t *pRExC_state)
{
    bool ended = 0;

    PERL_ARGS_ASSERT_REG_SKIPCOMMENT;

    while (RExC_parse < RExC_end)
        if (*RExC_parse++ == '\n') {
            ended = 1;
            break;
        }
    if (!ended) {
        /* we ran off the end of the pattern without ending
           the comment, so we have to add an \n when wrapping */
        RExC_seen |= REG_SEEN_RUN_ON_COMMENT;
        return 0;
    } else
        return 1;
}

/* nextchar()

   Advances the parse position, and optionally absorbs
   "whitespace" from the inputstream.

   Without /x "whitespace" means (?#...) style comments only,
   with /x this means (?#...) and # comments and whitespace proper.

   Returns the RExC_parse point from BEFORE the scan occurs.

   This is the /x friendly way of saying RExC_parse++.
*/

STATIC char*
S_nextchar(pTHX_ RExC_state_t *pRExC_state)
{
    char* const retval = RExC_parse++;

    PERL_ARGS_ASSERT_NEXTCHAR;

    for (;;) {
	if (RExC_end - RExC_parse >= 3
	    && *RExC_parse == '('
	    && RExC_parse[1] == '?'
	    && RExC_parse[2] == '#')
	{
	    while (*RExC_parse != ')') {
		if (RExC_parse == RExC_end)
		    FAIL("Sequence (?#... not terminated");
		RExC_parse++;
	    }
	    RExC_parse++;
	    continue;
	}
	if (RExC_flags & RXf_PMf_EXTENDED) {
	    if (isSPACE(*RExC_parse)) {
		RExC_parse++;
		continue;
	    }
	    else if (*RExC_parse == '#') {
	        if ( reg_skipcomment( pRExC_state ) )
	            continue;
	    }
	}
	return retval;
    }
}

/*
- reg_node - emit a node
*/
STATIC regnode *			/* Location. */
S_reg_node(pTHX_ RExC_state_t *pRExC_state, U8 op)
{
    dVAR;
    register regnode *ptr;
    regnode * const ret = RExC_emit;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REG_NODE;

    if (SIZE_ONLY) {
	SIZE_ALIGN(RExC_size);
	RExC_size += 1;
	return(ret);
    }
    if (RExC_emit >= RExC_emit_bound)
        Perl_croak(aTHX_ "panic: reg_node overrun trying to emit %d, %p>=%p",
		   op, RExC_emit, RExC_emit_bound);

    NODE_ALIGN_FILL(ret);
    ptr = ret;
    FILL_ADVANCE_NODE(ptr, op);
#ifdef RE_TRACK_PATTERN_OFFSETS
    if (RExC_offsets) {         /* MJD */
	MJD_OFFSET_DEBUG(("%s:%d: (op %s) %s %"UVuf" (len %"UVuf") (max %"UVuf").\n", 
              "reg_node", __LINE__, 
              PL_reg_name[op],
              (UV)(RExC_emit - RExC_emit_start) > RExC_offsets[0] 
		? "Overwriting end of array!\n" : "OK",
              (UV)(RExC_emit - RExC_emit_start),
              (UV)(RExC_parse - RExC_start),
              (UV)RExC_offsets[0])); 
	Set_Node_Offset(RExC_emit, RExC_parse + (op == END));
    }
#endif
    RExC_emit = ptr;
    return(ret);
}

/*
- reganode - emit a node with an argument
*/
STATIC regnode *			/* Location. */
S_reganode(pTHX_ RExC_state_t *pRExC_state, U8 op, U32 arg)
{
    dVAR;
    register regnode *ptr;
    regnode * const ret = RExC_emit;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGANODE;

    if (SIZE_ONLY) {
	SIZE_ALIGN(RExC_size);
	RExC_size += 2;
	/* 
	   We can't do this:
	   
	   assert(2==regarglen[op]+1); 

	   Anything larger than this has to allocate the extra amount.
	   If we changed this to be:
	   
	   RExC_size += (1 + regarglen[op]);
	   
	   then it wouldn't matter. Its not clear what side effect
	   might come from that so its not done so far.
	   -- dmq
	*/
	return(ret);
    }
    if (RExC_emit >= RExC_emit_bound)
        Perl_croak(aTHX_ "panic: reg_node overrun trying to emit %d, %p>=%p",
		   op, RExC_emit, RExC_emit_bound);

    NODE_ALIGN_FILL(ret);
    ptr = ret;
    FILL_ADVANCE_NODE_ARG(ptr, op, arg);
#ifdef RE_TRACK_PATTERN_OFFSETS
    if (RExC_offsets) {         /* MJD */
	MJD_OFFSET_DEBUG(("%s(%d): (op %s) %s %"UVuf" <- %"UVuf" (max %"UVuf").\n", 
              "reganode",
	      __LINE__,
	      PL_reg_name[op],
              (UV)(RExC_emit - RExC_emit_start) > RExC_offsets[0] ? 
              "Overwriting end of array!\n" : "OK",
              (UV)(RExC_emit - RExC_emit_start),
              (UV)(RExC_parse - RExC_start),
              (UV)RExC_offsets[0])); 
	Set_Cur_Node_Offset;
    }
#endif            
    RExC_emit = ptr;
    return(ret);
}

/*
- reguni - emit (if appropriate) a Unicode character
*/
STATIC STRLEN
S_reguni(pTHX_ const RExC_state_t *pRExC_state, UV uv, char* s)
{
    dVAR;

    PERL_ARGS_ASSERT_REGUNI;

    return SIZE_ONLY ? UNISKIP(uv) : (uvchr_to_utf8((U8*)s, uv) - (U8*)s);
}

/*
- reginsert - insert an operator in front of already-emitted operand
*
* Means relocating the operand.
*/
STATIC void
S_reginsert(pTHX_ RExC_state_t *pRExC_state, U8 op, regnode *opnd, U32 depth)
{
    dVAR;
    register regnode *src;
    register regnode *dst;
    register regnode *place;
    const int offset = regarglen[(U8)op];
    const int size = NODE_STEP_REGNODE + offset;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGINSERT;
    PERL_UNUSED_ARG(depth);
/* (PL_regkind[(U8)op] == CURLY ? EXTRA_STEP_2ARGS : 0); */
    DEBUG_PARSE_FMT("inst"," - %s",PL_reg_name[op]);
    if (SIZE_ONLY) {
	RExC_size += size;
	return;
    }

    src = RExC_emit;
    RExC_emit += size;
    dst = RExC_emit;
    if (RExC_open_parens) {
        int paren;
        /*DEBUG_PARSE_FMT("inst"," - %"IVdf, (IV)RExC_npar);*/
        for ( paren=0 ; paren < RExC_npar ; paren++ ) {
            if ( RExC_open_parens[paren] >= opnd ) {
                /*DEBUG_PARSE_FMT("open"," - %d",size);*/
                RExC_open_parens[paren] += size;
            } else {
                /*DEBUG_PARSE_FMT("open"," - %s","ok");*/
            }
            if ( RExC_close_parens[paren] >= opnd ) {
                /*DEBUG_PARSE_FMT("close"," - %d",size);*/
                RExC_close_parens[paren] += size;
            } else {
                /*DEBUG_PARSE_FMT("close"," - %s","ok");*/
            }
        }
    }

    while (src > opnd) {
	StructCopy(--src, --dst, regnode);
#ifdef RE_TRACK_PATTERN_OFFSETS
        if (RExC_offsets) {     /* MJD 20010112 */
	    MJD_OFFSET_DEBUG(("%s(%d): (op %s) %s copy %"UVuf" -> %"UVuf" (max %"UVuf").\n",
                  "reg_insert",
		  __LINE__,
		  PL_reg_name[op],
                  (UV)(dst - RExC_emit_start) > RExC_offsets[0] 
		    ? "Overwriting end of array!\n" : "OK",
                  (UV)(src - RExC_emit_start),
                  (UV)(dst - RExC_emit_start),
                  (UV)RExC_offsets[0])); 
	    Set_Node_Offset_To_R(dst-RExC_emit_start, Node_Offset(src));
	    Set_Node_Length_To_R(dst-RExC_emit_start, Node_Length(src));
        }
#endif
    }
    

    place = opnd;		/* Op node, where operand used to be. */
#ifdef RE_TRACK_PATTERN_OFFSETS
    if (RExC_offsets) {         /* MJD */
	MJD_OFFSET_DEBUG(("%s(%d): (op %s) %s %"UVuf" <- %"UVuf" (max %"UVuf").\n", 
              "reginsert",
	      __LINE__,
	      PL_reg_name[op],
              (UV)(place - RExC_emit_start) > RExC_offsets[0] 
              ? "Overwriting end of array!\n" : "OK",
              (UV)(place - RExC_emit_start),
              (UV)(RExC_parse - RExC_start),
              (UV)RExC_offsets[0]));
	Set_Node_Offset(place, RExC_parse);
	Set_Node_Length(place, 1);
    }
#endif    
    src = NEXTOPER(place);
    FILL_ADVANCE_NODE(place, op);
    Zero(src, offset, regnode);
}

/*
- regtail - set the next-pointer at the end of a node chain of p to val.
- SEE ALSO: regtail_study
*/
/* TODO: All three parms should be const */
STATIC void
S_regtail(pTHX_ RExC_state_t *pRExC_state, regnode *p, const regnode *val,U32 depth)
{
    dVAR;
    register regnode *scan;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGTAIL;
#ifndef DEBUGGING
    PERL_UNUSED_ARG(depth);
#endif

    if (SIZE_ONLY)
	return;

    /* Find last node. */
    scan = p;
    for (;;) {
	regnode * const temp = regnext(scan);
        DEBUG_PARSE_r({
            SV * const mysv=sv_newmortal();
            DEBUG_PARSE_MSG((scan==p ? "tail" : ""));
            regprop(RExC_rx, mysv, scan);
            PerlIO_printf(Perl_debug_log, "~ %s (%d) %s %s\n",
                SvPV_nolen_const(mysv), REG_NODE_NUM(scan),
                    (temp == NULL ? "->" : ""),
                    (temp == NULL ? PL_reg_name[OP(val)] : "")
            );
        });
        if (temp == NULL)
            break;
        scan = temp;
    }

    if (reg_off_by_arg[OP(scan)]) {
        ARG_SET(scan, val - scan);
    }
    else {
        NEXT_OFF(scan) = val - scan;
    }
}

#ifdef DEBUGGING
/*
- regtail_study - set the next-pointer at the end of a node chain of p to val.
- Look for optimizable sequences at the same time.
- currently only looks for EXACT chains.

This is experimental code. The idea is to use this routine to perform 
in place optimizations on branches and groups as they are constructed,
with the long term intention of removing optimization from study_chunk so
that it is purely analytical.

Currently only used when in DEBUG mode. The macro REGTAIL_STUDY() is used
to control which is which.

*/
/* TODO: All four parms should be const */

STATIC U8
S_regtail_study(pTHX_ RExC_state_t *pRExC_state, regnode *p, const regnode *val,U32 depth)
{
    dVAR;
    register regnode *scan;
    U8 exact = PSEUDO;
#ifdef EXPERIMENTAL_INPLACESCAN
    I32 min = 0;
#endif
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGTAIL_STUDY;


    if (SIZE_ONLY)
        return exact;

    /* Find last node. */

    scan = p;
    for (;;) {
        regnode * const temp = regnext(scan);
#ifdef EXPERIMENTAL_INPLACESCAN
        if (PL_regkind[OP(scan)] == EXACT) {
	    bool has_exactf_sharp_s;	/* Unexamined in this routine */
            if (join_exact(pRExC_state,scan,&min, &has_exactf_sharp_s, 1,val,depth+1))
                return EXACT;
	}
#endif
        if ( exact ) {
            switch (OP(scan)) {
                case EXACT:
                case EXACTF:
                case EXACTFA:
                case EXACTFU:
                case EXACTFU_SS:
                case EXACTFU_TRICKYFOLD:
                case EXACTFL:
                        if( exact == PSEUDO )
                            exact= OP(scan);
                        else if ( exact != OP(scan) )
                            exact= 0;
                case NOTHING:
                    break;
                default:
                    exact= 0;
            }
        }
        DEBUG_PARSE_r({
            SV * const mysv=sv_newmortal();
            DEBUG_PARSE_MSG((scan==p ? "tsdy" : ""));
            regprop(RExC_rx, mysv, scan);
            PerlIO_printf(Perl_debug_log, "~ %s (%d) -> %s\n",
                SvPV_nolen_const(mysv),
                REG_NODE_NUM(scan),
                PL_reg_name[exact]);
        });
	if (temp == NULL)
	    break;
	scan = temp;
    }
    DEBUG_PARSE_r({
        SV * const mysv_val=sv_newmortal();
        DEBUG_PARSE_MSG("");
        regprop(RExC_rx, mysv_val, val);
        PerlIO_printf(Perl_debug_log, "~ attach to %s (%"IVdf") offset to %"IVdf"\n",
		      SvPV_nolen_const(mysv_val),
		      (IV)REG_NODE_NUM(val),
		      (IV)(val - scan)
        );
    });
    if (reg_off_by_arg[OP(scan)]) {
	ARG_SET(scan, val - scan);
    }
    else {
	NEXT_OFF(scan) = val - scan;
    }

    return exact;
}
#endif

/*
 - regdump - dump a regexp onto Perl_debug_log in vaguely comprehensible form
 */
#ifdef DEBUGGING
static void 
S_regdump_extflags(pTHX_ const char *lead, const U32 flags)
{
    int bit;
    int set=0;
    regex_charset cs;

    for (bit=0; bit<32; bit++) {
        if (flags & (1<<bit)) {
	    if ((1<<bit) & RXf_PMf_CHARSET) {	/* Output separately, below */
		continue;
	    }
            if (!set++ && lead) 
                PerlIO_printf(Perl_debug_log, "%s",lead);
            PerlIO_printf(Perl_debug_log, "%s ",PL_reg_extflags_name[bit]);
        }	        
    }	   
    if ((cs = get_regex_charset(flags)) != REGEX_DEPENDS_CHARSET) {
            if (!set++ && lead) {
                PerlIO_printf(Perl_debug_log, "%s",lead);
            }
            switch (cs) {
                case REGEX_UNICODE_CHARSET:
                    PerlIO_printf(Perl_debug_log, "UNICODE");
                    break;
                case REGEX_LOCALE_CHARSET:
                    PerlIO_printf(Perl_debug_log, "LOCALE");
                    break;
                case REGEX_ASCII_RESTRICTED_CHARSET:
                    PerlIO_printf(Perl_debug_log, "ASCII-RESTRICTED");
                    break;
                case REGEX_ASCII_MORE_RESTRICTED_CHARSET:
                    PerlIO_printf(Perl_debug_log, "ASCII-MORE_RESTRICTED");
                    break;
                default:
                    PerlIO_printf(Perl_debug_log, "UNKNOWN CHARACTER SET");
                    break;
            }
    }
    if (lead)  {
        if (set) 
            PerlIO_printf(Perl_debug_log, "\n");
        else 
            PerlIO_printf(Perl_debug_log, "%s[none-set]\n",lead);
    }            
}   
#endif

void
Perl_regdump(pTHX_ const regexp *r)
{
#ifdef DEBUGGING
    dVAR;
    SV * const sv = sv_newmortal();
    SV *dsv= sv_newmortal();
    RXi_GET_DECL(r,ri);
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGDUMP;

    (void)dumpuntil(r, ri->program, ri->program + 1, NULL, NULL, sv, 0, 0);

    /* Header fields of interest. */
    if (r->anchored_substr) {
	RE_PV_QUOTED_DECL(s, 0, dsv, SvPVX_const(r->anchored_substr), 
	    RE_SV_DUMPLEN(r->anchored_substr), 30);
	PerlIO_printf(Perl_debug_log,
		      "anchored %s%s at %"IVdf" ",
		      s, RE_SV_TAIL(r->anchored_substr),
		      (IV)r->anchored_offset);
    } else if (r->anchored_utf8) {
	RE_PV_QUOTED_DECL(s, 1, dsv, SvPVX_const(r->anchored_utf8), 
	    RE_SV_DUMPLEN(r->anchored_utf8), 30);
	PerlIO_printf(Perl_debug_log,
		      "anchored utf8 %s%s at %"IVdf" ",
		      s, RE_SV_TAIL(r->anchored_utf8),
		      (IV)r->anchored_offset);
    }		      
    if (r->float_substr) {
	RE_PV_QUOTED_DECL(s, 0, dsv, SvPVX_const(r->float_substr), 
	    RE_SV_DUMPLEN(r->float_substr), 30);
	PerlIO_printf(Perl_debug_log,
		      "floating %s%s at %"IVdf"..%"UVuf" ",
		      s, RE_SV_TAIL(r->float_substr),
		      (IV)r->float_min_offset, (UV)r->float_max_offset);
    } else if (r->float_utf8) {
	RE_PV_QUOTED_DECL(s, 1, dsv, SvPVX_const(r->float_utf8), 
	    RE_SV_DUMPLEN(r->float_utf8), 30);
	PerlIO_printf(Perl_debug_log,
		      "floating utf8 %s%s at %"IVdf"..%"UVuf" ",
		      s, RE_SV_TAIL(r->float_utf8),
		      (IV)r->float_min_offset, (UV)r->float_max_offset);
    }
    if (r->check_substr || r->check_utf8)
	PerlIO_printf(Perl_debug_log,
		      (const char *)
		      (r->check_substr == r->float_substr
		       && r->check_utf8 == r->float_utf8
		       ? "(checking floating" : "(checking anchored"));
    if (r->extflags & RXf_NOSCAN)
	PerlIO_printf(Perl_debug_log, " noscan");
    if (r->extflags & RXf_CHECK_ALL)
	PerlIO_printf(Perl_debug_log, " isall");
    if (r->check_substr || r->check_utf8)
	PerlIO_printf(Perl_debug_log, ") ");

    if (ri->regstclass) {
	regprop(r, sv, ri->regstclass);
	PerlIO_printf(Perl_debug_log, "stclass %s ", SvPVX_const(sv));
    }
    if (r->extflags & RXf_ANCH) {
	PerlIO_printf(Perl_debug_log, "anchored");
	if (r->extflags & RXf_ANCH_BOL)
	    PerlIO_printf(Perl_debug_log, "(BOL)");
	if (r->extflags & RXf_ANCH_MBOL)
	    PerlIO_printf(Perl_debug_log, "(MBOL)");
	if (r->extflags & RXf_ANCH_SBOL)
	    PerlIO_printf(Perl_debug_log, "(SBOL)");
	if (r->extflags & RXf_ANCH_GPOS)
	    PerlIO_printf(Perl_debug_log, "(GPOS)");
	PerlIO_putc(Perl_debug_log, ' ');
    }
    if (r->extflags & RXf_GPOS_SEEN)
	PerlIO_printf(Perl_debug_log, "GPOS:%"UVuf" ", (UV)r->gofs);
    if (r->intflags & PREGf_SKIP)
	PerlIO_printf(Perl_debug_log, "plus ");
    if (r->intflags & PREGf_IMPLICIT)
	PerlIO_printf(Perl_debug_log, "implicit ");
    PerlIO_printf(Perl_debug_log, "minlen %"IVdf" ", (IV)r->minlen);
    if (r->extflags & RXf_EVAL_SEEN)
	PerlIO_printf(Perl_debug_log, "with eval ");
    PerlIO_printf(Perl_debug_log, "\n");
    DEBUG_FLAGS_r(regdump_extflags("r->extflags: ",r->extflags));            
#else
    PERL_ARGS_ASSERT_REGDUMP;
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(r);
#endif	/* DEBUGGING */
}

/*
- regprop - printable representation of opcode
*/
#define EMIT_ANYOF_TEST_SEPARATOR(do_sep,sv,flags) \
STMT_START { \
        if (do_sep) {                           \
            Perl_sv_catpvf(aTHX_ sv,"%s][%s",PL_colors[1],PL_colors[0]); \
            if (flags & ANYOF_INVERT)           \
                /*make sure the invert info is in each */ \
                sv_catpvs(sv, "^");             \
            do_sep = 0;                         \
        }                                       \
} STMT_END

void
Perl_regprop(pTHX_ const regexp *prog, SV *sv, const regnode *o)
{
#ifdef DEBUGGING
    dVAR;
    register int k;
    RXi_GET_DECL(prog,progi);
    GET_RE_DEBUG_FLAGS_DECL;
    
    PERL_ARGS_ASSERT_REGPROP;

    sv_setpvs(sv, "");

    if (OP(o) > REGNODE_MAX)		/* regnode.type is unsigned */
	/* It would be nice to FAIL() here, but this may be called from
	   regexec.c, and it would be hard to supply pRExC_state. */
	Perl_croak(aTHX_ "Corrupted regexp opcode %d > %d", (int)OP(o), (int)REGNODE_MAX);
    sv_catpv(sv, PL_reg_name[OP(o)]); /* Take off const! */

    k = PL_regkind[OP(o)];

    if (k == EXACT) {
	sv_catpvs(sv, " ");
	/* Using is_utf8_string() (via PERL_PV_UNI_DETECT) 
	 * is a crude hack but it may be the best for now since 
	 * we have no flag "this EXACTish node was UTF-8" 
	 * --jhi */
	pv_pretty(sv, STRING(o), STR_LEN(o), 60, PL_colors[0], PL_colors[1],
		  PERL_PV_ESCAPE_UNI_DETECT |
		  PERL_PV_ESCAPE_NONASCII   |
		  PERL_PV_PRETTY_ELLIPSES   |
		  PERL_PV_PRETTY_LTGT       |
		  PERL_PV_PRETTY_NOCLEAR
		  );
    } else if (k == TRIE) {
	/* print the details of the trie in dumpuntil instead, as
	 * progi->data isn't available here */
        const char op = OP(o);
        const U32 n = ARG(o);
        const reg_ac_data * const ac = IS_TRIE_AC(op) ?
               (reg_ac_data *)progi->data->data[n] :
               NULL;
        const reg_trie_data * const trie
	    = (reg_trie_data*)progi->data->data[!IS_TRIE_AC(op) ? n : ac->trie];
        
        Perl_sv_catpvf(aTHX_ sv, "-%s",PL_reg_name[o->flags]);
        DEBUG_TRIE_COMPILE_r(
            Perl_sv_catpvf(aTHX_ sv,
                "<S:%"UVuf"/%"IVdf" W:%"UVuf" L:%"UVuf"/%"UVuf" C:%"UVuf"/%"UVuf">",
                (UV)trie->startstate,
                (IV)trie->statecount-1, /* -1 because of the unused 0 element */
                (UV)trie->wordcount,
                (UV)trie->minlen,
                (UV)trie->maxlen,
                (UV)TRIE_CHARCOUNT(trie),
                (UV)trie->uniquecharcount
            )
        );
        if ( IS_ANYOF_TRIE(op) || trie->bitmap ) {
            int i;
            int rangestart = -1;
            U8* bitmap = IS_ANYOF_TRIE(op) ? (U8*)ANYOF_BITMAP(o) : (U8*)TRIE_BITMAP(trie);
            sv_catpvs(sv, "[");
            for (i = 0; i <= 256; i++) {
                if (i < 256 && BITMAP_TEST(bitmap,i)) {
                    if (rangestart == -1)
                        rangestart = i;
                } else if (rangestart != -1) {
                    if (i <= rangestart + 3)
                        for (; rangestart < i; rangestart++)
                            put_byte(sv, rangestart);
                    else {
                        put_byte(sv, rangestart);
                        sv_catpvs(sv, "-");
                        put_byte(sv, i - 1);
                    }
                    rangestart = -1;
                }
            }
            sv_catpvs(sv, "]");
        } 
	 
    } else if (k == CURLY) {
	if (OP(o) == CURLYM || OP(o) == CURLYN || OP(o) == CURLYX)
	    Perl_sv_catpvf(aTHX_ sv, "[%d]", o->flags); /* Parenth number */
	Perl_sv_catpvf(aTHX_ sv, " {%d,%d}", ARG1(o), ARG2(o));
    }
    else if (k == WHILEM && o->flags)			/* Ordinal/of */
	Perl_sv_catpvf(aTHX_ sv, "[%d/%d]", o->flags & 0xf, o->flags>>4);
    else if (k == REF || k == OPEN || k == CLOSE || k == GROUPP || OP(o)==ACCEPT) {
	Perl_sv_catpvf(aTHX_ sv, "%d", (int)ARG(o));	/* Parenth number */
	if ( RXp_PAREN_NAMES(prog) ) {
            if ( k != REF || (OP(o) < NREF)) {
	        AV *list= MUTABLE_AV(progi->data->data[progi->name_list_idx]);
	        SV **name= av_fetch(list, ARG(o), 0 );
	        if (name)
	            Perl_sv_catpvf(aTHX_ sv, " '%"SVf"'", SVfARG(*name));
            }	    
            else {
                AV *list= MUTABLE_AV(progi->data->data[ progi->name_list_idx ]);
                SV *sv_dat= MUTABLE_SV(progi->data->data[ ARG( o ) ]);
                I32 *nums=(I32*)SvPVX(sv_dat);
                SV **name= av_fetch(list, nums[0], 0 );
                I32 n;
                if (name) {
                    for ( n=0; n<SvIVX(sv_dat); n++ ) {
                        Perl_sv_catpvf(aTHX_ sv, "%s%"IVdf,
			   	    (n ? "," : ""), (IV)nums[n]);
                    }
                    Perl_sv_catpvf(aTHX_ sv, " '%"SVf"'", SVfARG(*name));
                }
            }
        }            
    } else if (k == GOSUB) 
	Perl_sv_catpvf(aTHX_ sv, "%d[%+d]", (int)ARG(o),(int)ARG2L(o));	/* Paren and offset */
    else if (k == VERB) {
        if (!o->flags) 
            Perl_sv_catpvf(aTHX_ sv, ":%"SVf, 
			   SVfARG((MUTABLE_SV(progi->data->data[ ARG( o ) ]))));
    } else if (k == LOGICAL)
	Perl_sv_catpvf(aTHX_ sv, "[%d]", o->flags);	/* 2: embedded, otherwise 1 */
    else if (k == ANYOF) {
	int i, rangestart = -1;
	const U8 flags = ANYOF_FLAGS(o);
	int do_sep = 0;

	/* Should be synchronized with * ANYOF_ #xdefines in regcomp.h */
	static const char * const anyofs[] = {
	    "\\w",
	    "\\W",
	    "\\s",
	    "\\S",
	    "\\d",
	    "\\D",
	    "[:alnum:]",
	    "[:^alnum:]",
	    "[:alpha:]",
	    "[:^alpha:]",
	    "[:ascii:]",
	    "[:^ascii:]",
	    "[:cntrl:]",
	    "[:^cntrl:]",
	    "[:graph:]",
	    "[:^graph:]",
	    "[:lower:]",
	    "[:^lower:]",
	    "[:print:]",
	    "[:^print:]",
	    "[:punct:]",
	    "[:^punct:]",
	    "[:upper:]",
	    "[:^upper:]",
	    "[:xdigit:]",
	    "[:^xdigit:]",
	    "[:space:]",
	    "[:^space:]",
	    "[:blank:]",
	    "[:^blank:]"
	};

	if (flags & ANYOF_LOCALE)
	    sv_catpvs(sv, "{loc}");
	if (flags & ANYOF_LOC_NONBITMAP_FOLD)
	    sv_catpvs(sv, "{i}");
	Perl_sv_catpvf(aTHX_ sv, "[%s", PL_colors[0]);
	if (flags & ANYOF_INVERT)
	    sv_catpvs(sv, "^");

	/* output what the standard cp 0-255 bitmap matches */
	for (i = 0; i <= 256; i++) {
	    if (i < 256 && ANYOF_BITMAP_TEST(o,i)) {
		if (rangestart == -1)
		    rangestart = i;
	    } else if (rangestart != -1) {
		if (i <= rangestart + 3)
		    for (; rangestart < i; rangestart++)
			put_byte(sv, rangestart);
		else {
		    put_byte(sv, rangestart);
		    sv_catpvs(sv, "-");
		    put_byte(sv, i - 1);
		}
		do_sep = 1;
		rangestart = -1;
	    }
	}
        
        EMIT_ANYOF_TEST_SEPARATOR(do_sep,sv,flags);
        /* output any special charclass tests (used entirely under use locale) */
	if (ANYOF_CLASS_TEST_ANY_SET(o))
	    for (i = 0; i < (int)(sizeof(anyofs)/sizeof(char*)); i++)
		if (ANYOF_CLASS_TEST(o,i)) {
		    sv_catpv(sv, anyofs[i]);
		    do_sep = 1;
		}
        
        EMIT_ANYOF_TEST_SEPARATOR(do_sep,sv,flags);
        
	if (flags & ANYOF_NON_UTF8_LATIN1_ALL) {
	    sv_catpvs(sv, "{non-utf8-latin1-all}");
	}

        /* output information about the unicode matching */
	if (flags & ANYOF_UNICODE_ALL)
	    sv_catpvs(sv, "{unicode_all}");
	else if (ANYOF_NONBITMAP(o))
	    sv_catpvs(sv, "{unicode}");
	if (flags & ANYOF_NONBITMAP_NON_UTF8)
	    sv_catpvs(sv, "{outside bitmap}");

	if (ANYOF_NONBITMAP(o)) {
	    SV *lv; /* Set if there is something outside the bit map */
	    SV * const sw = regclass_swash(prog, o, FALSE, &lv, 0);
            bool byte_output = FALSE;   /* If something in the bitmap has been
                                           output */

	    if (lv && lv != &PL_sv_undef) {
		if (sw) {
		    U8 s[UTF8_MAXBYTES_CASE+1];

		    for (i = 0; i <= 256; i++) { /* Look at chars in bitmap */
			uvchr_to_utf8(s, i);

			if (i < 256
                            && ! ANYOF_BITMAP_TEST(o, i)    /* Don't duplicate
                                                               things already
                                                               output as part
                                                               of the bitmap */
                            && swash_fetch(sw, s, TRUE))
                        {
			    if (rangestart == -1)
				rangestart = i;
			} else if (rangestart != -1) {
                            byte_output = TRUE;
			    if (i <= rangestart + 3)
				for (; rangestart < i; rangestart++) {
				    put_byte(sv, rangestart);
				}
			    else {
				put_byte(sv, rangestart);
				sv_catpvs(sv, "-");
				put_byte(sv, i-1);
			    }
			    rangestart = -1;
			}
		    }
		}

		{
		    char *s = savesvpv(lv);
		    char * const origs = s;

		    while (*s && *s != '\n')
			s++;

		    if (*s == '\n') {
			const char * const t = ++s;

                        if (byte_output) {
                            sv_catpvs(sv, " ");
                        }

			while (*s) {
			    if (*s == '\n') {

                                /* Truncate very long output */
				if (s - origs > 256) {
				    Perl_sv_catpvf(aTHX_ sv,
						   "%.*s...",
					           (int) (s - origs - 1),
						   t);
				    goto out_dump;
				}
				*s = ' ';
			    }
			    else if (*s == '\t') {
				*s = '-';
			    }
			    s++;
			}
			if (s[-1] == ' ')
			    s[-1] = 0;

			sv_catpv(sv, t);
		    }

		out_dump:

		    Safefree(origs);
		}
		SvREFCNT_dec(lv);
	    }
	}

	Perl_sv_catpvf(aTHX_ sv, "%s]", PL_colors[1]);
    }
    else if (k == BRANCHJ && (OP(o) == UNLESSM || OP(o) == IFMATCH))
	Perl_sv_catpvf(aTHX_ sv, "[%d]", -(o->flags));
#else
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(o);
    PERL_UNUSED_ARG(prog);
#endif	/* DEBUGGING */
}

SV *
Perl_re_intuit_string(pTHX_ REGEXP * const r)
{				/* Assume that RE_INTUIT is set */
    dVAR;
    struct regexp *const prog = (struct regexp *)SvANY(r);
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_RE_INTUIT_STRING;
    PERL_UNUSED_CONTEXT;

    DEBUG_COMPILE_r(
	{
	    const char * const s = SvPV_nolen_const(prog->check_substr
		      ? prog->check_substr : prog->check_utf8);

	    if (!PL_colorset) reginitcolors();
	    PerlIO_printf(Perl_debug_log,
		      "%sUsing REx %ssubstr:%s \"%s%.60s%s%s\"\n",
		      PL_colors[4],
		      prog->check_substr ? "" : "utf8 ",
		      PL_colors[5],PL_colors[0],
		      s,
		      PL_colors[1],
		      (strlen(s) > 60 ? "..." : ""));
	} );

    return prog->check_substr ? prog->check_substr : prog->check_utf8;
}

/* 
   pregfree() 
   
   handles refcounting and freeing the perl core regexp structure. When 
   it is necessary to actually free the structure the first thing it 
   does is call the 'free' method of the regexp_engine associated to
   the regexp, allowing the handling of the void *pprivate; member 
   first. (This routine is not overridable by extensions, which is why 
   the extensions free is called first.)
   
   See regdupe and regdupe_internal if you change anything here. 
*/
#ifndef PERL_IN_XSUB_RE
void
Perl_pregfree(pTHX_ REGEXP *r)
{
    SvREFCNT_dec(r);
}

void
Perl_pregfree2(pTHX_ REGEXP *rx)
{
    dVAR;
    struct regexp *const r = (struct regexp *)SvANY(rx);
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_PREGFREE2;

    if (r->mother_re) {
        ReREFCNT_dec(r->mother_re);
    } else {
        CALLREGFREE_PVT(rx); /* free the private data */
        SvREFCNT_dec(RXp_PAREN_NAMES(r));
    }        
    if (r->substrs) {
        SvREFCNT_dec(r->anchored_substr);
        SvREFCNT_dec(r->anchored_utf8);
        SvREFCNT_dec(r->float_substr);
        SvREFCNT_dec(r->float_utf8);
	Safefree(r->substrs);
    }
    RX_MATCH_COPY_FREE(rx);
#ifdef PERL_OLD_COPY_ON_WRITE
    SvREFCNT_dec(r->saved_copy);
#endif
    Safefree(r->offs);
}

/*  reg_temp_copy()
    
    This is a hacky workaround to the structural issue of match results
    being stored in the regexp structure which is in turn stored in
    PL_curpm/PL_reg_curpm. The problem is that due to qr// the pattern
    could be PL_curpm in multiple contexts, and could require multiple
    result sets being associated with the pattern simultaneously, such
    as when doing a recursive match with (??{$qr})
    
    The solution is to make a lightweight copy of the regexp structure 
    when a qr// is returned from the code executed by (??{$qr}) this
    lightweight copy doesn't actually own any of its data except for
    the starp/end and the actual regexp structure itself. 
    
*/    
    
    
REGEXP *
Perl_reg_temp_copy (pTHX_ REGEXP *ret_x, REGEXP *rx)
{
    struct regexp *ret;
    struct regexp *const r = (struct regexp *)SvANY(rx);
    register const I32 npar = r->nparens+1;

    PERL_ARGS_ASSERT_REG_TEMP_COPY;

    if (!ret_x)
	ret_x = (REGEXP*) newSV_type(SVt_REGEXP);
    ret = (struct regexp *)SvANY(ret_x);
    
    (void)ReREFCNT_inc(rx);
    /* We can take advantage of the existing "copied buffer" mechanism in SVs
       by pointing directly at the buffer, but flagging that the allocated
       space in the copy is zero. As we've just done a struct copy, it's now
       a case of zero-ing that, rather than copying the current length.  */
    SvPV_set(ret_x, RX_WRAPPED(rx));
    SvFLAGS(ret_x) |= SvFLAGS(rx) & (SVf_POK|SVp_POK|SVf_UTF8);
    memcpy(&(ret->xpv_cur), &(r->xpv_cur),
	   sizeof(regexp) - STRUCT_OFFSET(regexp, xpv_cur));
    SvLEN_set(ret_x, 0);
    SvSTASH_set(ret_x, NULL);
    SvMAGIC_set(ret_x, NULL);
    Newx(ret->offs, npar, regexp_paren_pair);
    Copy(r->offs, ret->offs, npar, regexp_paren_pair);
    if (r->substrs) {
        Newx(ret->substrs, 1, struct reg_substr_data);
	StructCopy(r->substrs, ret->substrs, struct reg_substr_data);

	SvREFCNT_inc_void(ret->anchored_substr);
	SvREFCNT_inc_void(ret->anchored_utf8);
	SvREFCNT_inc_void(ret->float_substr);
	SvREFCNT_inc_void(ret->float_utf8);

	/* check_substr and check_utf8, if non-NULL, point to either their
	   anchored or float namesakes, and don't hold a second reference.  */
    }
    RX_MATCH_COPIED_off(ret_x);
#ifdef PERL_OLD_COPY_ON_WRITE
    ret->saved_copy = NULL;
#endif
    ret->mother_re = rx;
    
    return ret_x;
}
#endif

/* regfree_internal() 

   Free the private data in a regexp. This is overloadable by 
   extensions. Perl takes care of the regexp structure in pregfree(), 
   this covers the *pprivate pointer which technically perl doesn't 
   know about, however of course we have to handle the 
   regexp_internal structure when no extension is in use. 
   
   Note this is called before freeing anything in the regexp 
   structure. 
 */
 
void
Perl_regfree_internal(pTHX_ REGEXP * const rx)
{
    dVAR;
    struct regexp *const r = (struct regexp *)SvANY(rx);
    RXi_GET_DECL(r,ri);
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGFREE_INTERNAL;

    DEBUG_COMPILE_r({
	if (!PL_colorset)
	    reginitcolors();
	{
	    SV *dsv= sv_newmortal();
            RE_PV_QUOTED_DECL(s, RX_UTF8(rx),
                dsv, RX_PRECOMP(rx), RX_PRELEN(rx), 60);
            PerlIO_printf(Perl_debug_log,"%sFreeing REx:%s %s\n", 
                PL_colors[4],PL_colors[5],s);
        }
    });
#ifdef RE_TRACK_PATTERN_OFFSETS
    if (ri->u.offsets)
        Safefree(ri->u.offsets);             /* 20010421 MJD */
#endif
    if (ri->data) {
	int n = ri->data->count;
	PAD* new_comppad = NULL;
	PAD* old_comppad;
	PADOFFSET refcnt;

	while (--n >= 0) {
          /* If you add a ->what type here, update the comment in regcomp.h */
	    switch (ri->data->what[n]) {
	    case 'a':
	    case 's':
	    case 'S':
	    case 'u':
		SvREFCNT_dec(MUTABLE_SV(ri->data->data[n]));
		break;
	    case 'f':
		Safefree(ri->data->data[n]);
		break;
	    case 'p':
		new_comppad = MUTABLE_AV(ri->data->data[n]);
		break;
	    case 'o':
		if (new_comppad == NULL)
		    Perl_croak(aTHX_ "panic: pregfree comppad");
		PAD_SAVE_LOCAL(old_comppad,
		    /* Watch out for global destruction's random ordering. */
		    (SvTYPE(new_comppad) == SVt_PVAV) ? new_comppad : NULL
		);
		OP_REFCNT_LOCK;
		refcnt = OpREFCNT_dec((OP_4tree*)ri->data->data[n]);
		OP_REFCNT_UNLOCK;
		if (!refcnt)
                    op_free((OP_4tree*)ri->data->data[n]);

		PAD_RESTORE_LOCAL(old_comppad);
		SvREFCNT_dec(MUTABLE_SV(new_comppad));
		new_comppad = NULL;
		break;
	    case 'n':
	        break;
            case 'T':	        
                { /* Aho Corasick add-on structure for a trie node.
                     Used in stclass optimization only */
                    U32 refcount;
                    reg_ac_data *aho=(reg_ac_data*)ri->data->data[n];
                    OP_REFCNT_LOCK;
                    refcount = --aho->refcount;
                    OP_REFCNT_UNLOCK;
                    if ( !refcount ) {
                        PerlMemShared_free(aho->states);
                        PerlMemShared_free(aho->fail);
			 /* do this last!!!! */
                        PerlMemShared_free(ri->data->data[n]);
                        PerlMemShared_free(ri->regstclass);
                    }
                }
                break;
	    case 't':
	        {
	            /* trie structure. */
	            U32 refcount;
	            reg_trie_data *trie=(reg_trie_data*)ri->data->data[n];
                    OP_REFCNT_LOCK;
                    refcount = --trie->refcount;
                    OP_REFCNT_UNLOCK;
                    if ( !refcount ) {
                        PerlMemShared_free(trie->charmap);
                        PerlMemShared_free(trie->states);
                        PerlMemShared_free(trie->trans);
                        if (trie->bitmap)
                            PerlMemShared_free(trie->bitmap);
                        if (trie->jump)
                            PerlMemShared_free(trie->jump);
			PerlMemShared_free(trie->wordinfo);
                        /* do this last!!!! */
                        PerlMemShared_free(ri->data->data[n]);
		    }
		}
		break;
	    default:
		Perl_croak(aTHX_ "panic: regfree data code '%c'", ri->data->what[n]);
	    }
	}
	Safefree(ri->data->what);
	Safefree(ri->data);
    }

    Safefree(ri);
}

#define av_dup_inc(s,t)	MUTABLE_AV(sv_dup_inc((const SV *)s,t))
#define hv_dup_inc(s,t)	MUTABLE_HV(sv_dup_inc((const SV *)s,t))
#define SAVEPVN(p,n)	((p) ? savepvn(p,n) : NULL)

/* 
   re_dup - duplicate a regexp. 
   
   This routine is expected to clone a given regexp structure. It is only
   compiled under USE_ITHREADS.

   After all of the core data stored in struct regexp is duplicated
   the regexp_engine.dupe method is used to copy any private data
   stored in the *pprivate pointer. This allows extensions to handle
   any duplication it needs to do.

   See pregfree() and regfree_internal() if you change anything here. 
*/
#if defined(USE_ITHREADS)
#ifndef PERL_IN_XSUB_RE
void
Perl_re_dup_guts(pTHX_ const REGEXP *sstr, REGEXP *dstr, CLONE_PARAMS *param)
{
    dVAR;
    I32 npar;
    const struct regexp *r = (const struct regexp *)SvANY(sstr);
    struct regexp *ret = (struct regexp *)SvANY(dstr);
    
    PERL_ARGS_ASSERT_RE_DUP_GUTS;

    npar = r->nparens+1;
    Newx(ret->offs, npar, regexp_paren_pair);
    Copy(r->offs, ret->offs, npar, regexp_paren_pair);
    if(ret->swap) {
        /* no need to copy these */
        Newx(ret->swap, npar, regexp_paren_pair);
    }

    if (ret->substrs) {
	/* Do it this way to avoid reading from *r after the StructCopy().
	   That way, if any of the sv_dup_inc()s dislodge *r from the L1
	   cache, it doesn't matter.  */
	const bool anchored = r->check_substr
	    ? r->check_substr == r->anchored_substr
	    : r->check_utf8 == r->anchored_utf8;
        Newx(ret->substrs, 1, struct reg_substr_data);
	StructCopy(r->substrs, ret->substrs, struct reg_substr_data);

	ret->anchored_substr = sv_dup_inc(ret->anchored_substr, param);
	ret->anchored_utf8 = sv_dup_inc(ret->anchored_utf8, param);
	ret->float_substr = sv_dup_inc(ret->float_substr, param);
	ret->float_utf8 = sv_dup_inc(ret->float_utf8, param);

	/* check_substr and check_utf8, if non-NULL, point to either their
	   anchored or float namesakes, and don't hold a second reference.  */

	if (ret->check_substr) {
	    if (anchored) {
		assert(r->check_utf8 == r->anchored_utf8);
		ret->check_substr = ret->anchored_substr;
		ret->check_utf8 = ret->anchored_utf8;
	    } else {
		assert(r->check_substr == r->float_substr);
		assert(r->check_utf8 == r->float_utf8);
		ret->check_substr = ret->float_substr;
		ret->check_utf8 = ret->float_utf8;
	    }
	} else if (ret->check_utf8) {
	    if (anchored) {
		ret->check_utf8 = ret->anchored_utf8;
	    } else {
		ret->check_utf8 = ret->float_utf8;
	    }
	}
    }

    RXp_PAREN_NAMES(ret) = hv_dup_inc(RXp_PAREN_NAMES(ret), param);

    if (ret->pprivate)
	RXi_SET(ret,CALLREGDUPE_PVT(dstr,param));

    if (RX_MATCH_COPIED(dstr))
	ret->subbeg  = SAVEPVN(ret->subbeg, ret->sublen);
    else
	ret->subbeg = NULL;
#ifdef PERL_OLD_COPY_ON_WRITE
    ret->saved_copy = NULL;
#endif

    if (ret->mother_re) {
	if (SvPVX_const(dstr) == SvPVX_const(ret->mother_re)) {
	    /* Our storage points directly to our mother regexp, but that's
	       1: a buffer in a different thread
	       2: something we no longer hold a reference on
	       so we need to copy it locally.  */
	    /* Note we need to use SvCUR(), rather than
	       SvLEN(), on our mother_re, because it, in
	       turn, may well be pointing to its own mother_re.  */
	    SvPV_set(dstr, SAVEPVN(SvPVX_const(ret->mother_re),
				   SvCUR(ret->mother_re)+1));
	    SvLEN_set(dstr, SvCUR(ret->mother_re)+1);
	}
	ret->mother_re      = NULL;
    }
    ret->gofs = 0;
}
#endif /* PERL_IN_XSUB_RE */

/*
   regdupe_internal()
   
   This is the internal complement to regdupe() which is used to copy
   the structure pointed to by the *pprivate pointer in the regexp.
   This is the core version of the extension overridable cloning hook.
   The regexp structure being duplicated will be copied by perl prior
   to this and will be provided as the regexp *r argument, however 
   with the /old/ structures pprivate pointer value. Thus this routine
   may override any copying normally done by perl.
   
   It returns a pointer to the new regexp_internal structure.
*/

void *
Perl_regdupe_internal(pTHX_ REGEXP * const rx, CLONE_PARAMS *param)
{
    dVAR;
    struct regexp *const r = (struct regexp *)SvANY(rx);
    regexp_internal *reti;
    int len;
    RXi_GET_DECL(r,ri);

    PERL_ARGS_ASSERT_REGDUPE_INTERNAL;
    
    len = ProgLen(ri);
    
    Newxc(reti, sizeof(regexp_internal) + len*sizeof(regnode), char, regexp_internal);
    Copy(ri->program, reti->program, len+1, regnode);
    

    reti->regstclass = NULL;

    if (ri->data) {
	struct reg_data *d;
        const int count = ri->data->count;
	int i;

	Newxc(d, sizeof(struct reg_data) + count*sizeof(void *),
		char, struct reg_data);
	Newx(d->what, count, U8);

	d->count = count;
	for (i = 0; i < count; i++) {
	    d->what[i] = ri->data->what[i];
	    switch (d->what[i]) {
	        /* legal options are one of: sSfpontTua
	           see also regcomp.h and pregfree() */
	    case 'a': /* actually an AV, but the dup function is identical.  */
	    case 's':
	    case 'S':
	    case 'p': /* actually an AV, but the dup function is identical.  */
	    case 'u': /* actually an HV, but the dup function is identical.  */
		d->data[i] = sv_dup_inc((const SV *)ri->data->data[i], param);
		break;
	    case 'f':
		/* This is cheating. */
		Newx(d->data[i], 1, struct regnode_charclass_class);
		StructCopy(ri->data->data[i], d->data[i],
			    struct regnode_charclass_class);
		reti->regstclass = (regnode*)d->data[i];
		break;
	    case 'o':
		/* Compiled op trees are readonly and in shared memory,
		   and can thus be shared without duplication. */
		OP_REFCNT_LOCK;
		d->data[i] = (void*)OpREFCNT_inc((OP*)ri->data->data[i]);
		OP_REFCNT_UNLOCK;
		break;
	    case 'T':
		/* Trie stclasses are readonly and can thus be shared
		 * without duplication. We free the stclass in pregfree
		 * when the corresponding reg_ac_data struct is freed.
		 */
		reti->regstclass= ri->regstclass;
		/* Fall through */
	    case 't':
		OP_REFCNT_LOCK;
		((reg_trie_data*)ri->data->data[i])->refcount++;
		OP_REFCNT_UNLOCK;
		/* Fall through */
	    case 'n':
		d->data[i] = ri->data->data[i];
		break;
            default:
		Perl_croak(aTHX_ "panic: re_dup unknown data code '%c'", ri->data->what[i]);
	    }
	}

	reti->data = d;
    }
    else
	reti->data = NULL;

    reti->name_list_idx = ri->name_list_idx;

#ifdef RE_TRACK_PATTERN_OFFSETS
    if (ri->u.offsets) {
        Newx(reti->u.offsets, 2*len+1, U32);
        Copy(ri->u.offsets, reti->u.offsets, 2*len+1, U32);
    }
#else
    SetProgLen(reti,len);
#endif

    return (void*)reti;
}

#endif    /* USE_ITHREADS */

#ifndef PERL_IN_XSUB_RE

/*
 - regnext - dig the "next" pointer out of a node
 */
regnode *
Perl_regnext(pTHX_ register regnode *p)
{
    dVAR;
    register I32 offset;

    if (!p)
	return(NULL);

    if (OP(p) > REGNODE_MAX) {		/* regnode.type is unsigned */
	Perl_croak(aTHX_ "Corrupted regexp opcode %d > %d", (int)OP(p), (int)REGNODE_MAX);
    }

    offset = (reg_off_by_arg[OP(p)] ? ARG(p) : NEXT_OFF(p));
    if (offset == 0)
	return(NULL);

    return(p+offset);
}
#endif

STATIC void
S_re_croak2(pTHX_ const char* pat1,const char* pat2,...)
{
    va_list args;
    STRLEN l1 = strlen(pat1);
    STRLEN l2 = strlen(pat2);
    char buf[512];
    SV *msv;
    const char *message;

    PERL_ARGS_ASSERT_RE_CROAK2;

    if (l1 > 510)
	l1 = 510;
    if (l1 + l2 > 510)
	l2 = 510 - l1;
    Copy(pat1, buf, l1 , char);
    Copy(pat2, buf + l1, l2 , char);
    buf[l1 + l2] = '\n';
    buf[l1 + l2 + 1] = '\0';
#ifdef I_STDARG
    /* ANSI variant takes additional second argument */
    va_start(args, pat2);
#else
    va_start(args);
#endif
    msv = vmess(buf, &args);
    va_end(args);
    message = SvPV_const(msv,l1);
    if (l1 > 512)
	l1 = 512;
    Copy(message, buf, l1 , char);
    buf[l1-1] = '\0';			/* Overwrite \n */
    Perl_croak(aTHX_ "%s", buf);
}

/* XXX Here's a total kludge.  But we need to re-enter for swash routines. */

#ifndef PERL_IN_XSUB_RE
void
Perl_save_re_context(pTHX)
{
    dVAR;

    struct re_save_state *state;

    SAVEVPTR(PL_curcop);
    SSGROW(SAVESTACK_ALLOC_FOR_RE_SAVE_STATE + 1);

    state = (struct re_save_state *)(PL_savestack + PL_savestack_ix);
    PL_savestack_ix += SAVESTACK_ALLOC_FOR_RE_SAVE_STATE;
    SSPUSHUV(SAVEt_RE_STATE);

    Copy(&PL_reg_state, state, 1, struct re_save_state);

    PL_reg_start_tmp = 0;
    PL_reg_start_tmpl = 0;
    PL_reg_oldsaved = NULL;
    PL_reg_oldsavedlen = 0;
    PL_reg_maxiter = 0;
    PL_reg_leftiter = 0;
    PL_reg_poscache = NULL;
    PL_reg_poscache_size = 0;
#ifdef PERL_OLD_COPY_ON_WRITE
    PL_nrs = NULL;
#endif

    /* Save $1..$n (#18107: UTF-8 s/(\w+)/uc($1)/e); AMS 20021106. */
    if (PL_curpm) {
	const REGEXP * const rx = PM_GETRE(PL_curpm);
	if (rx) {
	    U32 i;
	    for (i = 1; i <= RX_NPARENS(rx); i++) {
		char digits[TYPE_CHARS(long)];
		const STRLEN len = my_snprintf(digits, sizeof(digits), "%lu", (long)i);
		GV *const *const gvp
		    = (GV**)hv_fetch(PL_defstash, digits, len, 0);

		if (gvp) {
		    GV * const gv = *gvp;
		    if (SvTYPE(gv) == SVt_PVGV && GvSV(gv))
			save_scalar(gv);
		}
	    }
	}
    }
}
#endif

static void
clear_re(pTHX_ void *r)
{
    dVAR;
    ReREFCNT_dec((REGEXP *)r);
}

#ifdef DEBUGGING

STATIC void
S_put_byte(pTHX_ SV *sv, int c)
{
    PERL_ARGS_ASSERT_PUT_BYTE;

    /* Our definition of isPRINT() ignores locales, so only bytes that are
       not part of UTF-8 are considered printable. I assume that the same
       holds for UTF-EBCDIC.
       Also, code point 255 is not printable in either (it's E0 in EBCDIC,
       which Wikipedia says:

       EO, or Eight Ones, is an 8-bit EBCDIC character code represented as all
       ones (binary 1111 1111, hexadecimal FF). It is similar, but not
       identical, to the ASCII delete (DEL) or rubout control character.
       ) So the old condition can be simplified to !isPRINT(c)  */
    if (!isPRINT(c)) {
	if (c < 256) {
	    Perl_sv_catpvf(aTHX_ sv, "\\x%02x", c);
	}
	else {
	    Perl_sv_catpvf(aTHX_ sv, "\\x{%x}", c);
	}
    }
    else {
	const char string = c;
	if (c == '-' || c == ']' || c == '\\' || c == '^')
	    sv_catpvs(sv, "\\");
	sv_catpvn(sv, &string, 1);
    }
}


#define CLEAR_OPTSTART \
    if (optstart) STMT_START { \
	    DEBUG_OPTIMISE_r(PerlIO_printf(Perl_debug_log, " (%"IVdf" nodes)\n", (IV)(node - optstart))); \
	    optstart=NULL; \
    } STMT_END

#define DUMPUNTIL(b,e) CLEAR_OPTSTART; node=dumpuntil(r,start,(b),(e),last,sv,indent+1,depth+1);

STATIC const regnode *
S_dumpuntil(pTHX_ const regexp *r, const regnode *start, const regnode *node,
	    const regnode *last, const regnode *plast, 
	    SV* sv, I32 indent, U32 depth)
{
    dVAR;
    register U8 op = PSEUDO;	/* Arbitrary non-END op. */
    register const regnode *next;
    const regnode *optstart= NULL;
    
    RXi_GET_DECL(r,ri);
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_DUMPUNTIL;

#ifdef DEBUG_DUMPUNTIL
    PerlIO_printf(Perl_debug_log, "--- %d : %d - %d - %d\n",indent,node-start,
        last ? last-start : 0,plast ? plast-start : 0);
#endif
            
    if (plast && plast < last) 
        last= plast;

    while (PL_regkind[op] != END && (!last || node < last)) {
	/* While that wasn't END last time... */
	NODE_ALIGN(node);
	op = OP(node);
	if (op == CLOSE || op == WHILEM)
	    indent--;
	next = regnext((regnode *)node);

	/* Where, what. */
	if (OP(node) == OPTIMIZED) {
	    if (!optstart && RE_DEBUG_FLAG(RE_DEBUG_COMPILE_OPTIMISE))
	        optstart = node;
	    else
		goto after_print;
	} else
	    CLEAR_OPTSTART;

	regprop(r, sv, node);
	PerlIO_printf(Perl_debug_log, "%4"IVdf":%*s%s", (IV)(node - start),
		      (int)(2*indent + 1), "", SvPVX_const(sv));
        
        if (OP(node) != OPTIMIZED) {		      
            if (next == NULL)		/* Next ptr. */
                PerlIO_printf(Perl_debug_log, " (0)");
            else if (PL_regkind[(U8)op] == BRANCH && PL_regkind[OP(next)] != BRANCH )
                PerlIO_printf(Perl_debug_log, " (FAIL)");
            else 
                PerlIO_printf(Perl_debug_log, " (%"IVdf")", (IV)(next - start));
            (void)PerlIO_putc(Perl_debug_log, '\n'); 
        }
        
      after_print:
	if (PL_regkind[(U8)op] == BRANCHJ) {
	    assert(next);
	    {
                register const regnode *nnode = (OP(next) == LONGJMP
					     ? regnext((regnode *)next)
					     : next);
                if (last && nnode > last)
                    nnode = last;
                DUMPUNTIL(NEXTOPER(NEXTOPER(node)), nnode);
	    }
	}
	else if (PL_regkind[(U8)op] == BRANCH) {
	    assert(next);
	    DUMPUNTIL(NEXTOPER(node), next);
	}
	else if ( PL_regkind[(U8)op]  == TRIE ) {
	    const regnode *this_trie = node;
	    const char op = OP(node);
            const U32 n = ARG(node);
	    const reg_ac_data * const ac = op>=AHOCORASICK ?
               (reg_ac_data *)ri->data->data[n] :
               NULL;
	    const reg_trie_data * const trie =
	        (reg_trie_data*)ri->data->data[op<AHOCORASICK ? n : ac->trie];
#ifdef DEBUGGING
	    AV *const trie_words = MUTABLE_AV(ri->data->data[n + TRIE_WORDS_OFFSET]);
#endif
	    const regnode *nextbranch= NULL;
	    I32 word_idx;
            sv_setpvs(sv, "");
	    for (word_idx= 0; word_idx < (I32)trie->wordcount; word_idx++) {
		SV ** const elem_ptr = av_fetch(trie_words,word_idx,0);

                PerlIO_printf(Perl_debug_log, "%*s%s ",
                   (int)(2*(indent+3)), "",
                    elem_ptr ? pv_pretty(sv, SvPV_nolen_const(*elem_ptr), SvCUR(*elem_ptr), 60,
	                    PL_colors[0], PL_colors[1],
	                    (SvUTF8(*elem_ptr) ? PERL_PV_ESCAPE_UNI : 0) |
	                    PERL_PV_PRETTY_ELLIPSES    |
	                    PERL_PV_PRETTY_LTGT
                            )
                            : "???"
                );
                if (trie->jump) {
                    U16 dist= trie->jump[word_idx+1];
		    PerlIO_printf(Perl_debug_log, "(%"UVuf")\n",
				  (UV)((dist ? this_trie + dist : next) - start));
                    if (dist) {
                        if (!nextbranch)
                            nextbranch= this_trie + trie->jump[0];    
			DUMPUNTIL(this_trie + dist, nextbranch);
                    }
                    if (nextbranch && PL_regkind[OP(nextbranch)]==BRANCH)
                        nextbranch= regnext((regnode *)nextbranch);
                } else {
                    PerlIO_printf(Perl_debug_log, "\n");
		}
	    }
	    if (last && next > last)
	        node= last;
	    else
	        node= next;
	}
	else if ( op == CURLY ) {   /* "next" might be very big: optimizer */
	    DUMPUNTIL(NEXTOPER(node) + EXTRA_STEP_2ARGS,
                    NEXTOPER(node) + EXTRA_STEP_2ARGS + 1);
	}
	else if (PL_regkind[(U8)op] == CURLY && op != CURLYX) {
	    assert(next);
	    DUMPUNTIL(NEXTOPER(node) + EXTRA_STEP_2ARGS, next);
	}
	else if ( op == PLUS || op == STAR) {
	    DUMPUNTIL(NEXTOPER(node), NEXTOPER(node) + 1);
	}
	else if (PL_regkind[(U8)op] == ANYOF) {
	    /* arglen 1 + class block */
	    node += 1 + ((ANYOF_FLAGS(node) & ANYOF_CLASS)
		    ? ANYOF_CLASS_SKIP : ANYOF_SKIP);
	    node = NEXTOPER(node);
	}
	else if (PL_regkind[(U8)op] == EXACT) {
            /* Literal string, where present. */
	    node += NODE_SZ_STR(node) - 1;
	    node = NEXTOPER(node);
	}
	else {
	    node = NEXTOPER(node);
	    node += regarglen[(U8)op];
	}
	if (op == CURLYX || op == OPEN)
	    indent++;
    }
    CLEAR_OPTSTART;
#ifdef DEBUG_DUMPUNTIL    
    PerlIO_printf(Perl_debug_log, "--- %d\n", (int)indent);
#endif
    return node;
}

#endif	/* DEBUGGING */

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
