/*    toke.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *  'It all comes from here, the stench and the peril.'    --Frodo
 *
 *     [p.719 of _The Lord of the Rings_, IV/ix: "Shelob's Lair"]
 */

/*
 * This file is the lexer for Perl.  It's closely linked to the
 * parser, perly.y.
 *
 * The main routine is yylex(), which returns the next token.
 */

/*
=head1 Lexer interface

This is the lower layer of the Perl parser, managing characters and tokens.

=for apidoc AmU|yy_parser *|PL_parser

Pointer to a structure encapsulating the state of the parsing operation
currently in progress.  The pointer can be locally changed to perform
a nested parse without interfering with the state of an outer parse.
Individual members of C<PL_parser> have their own documentation.

=cut
*/

#include "EXTERN.h"
#define PERL_IN_TOKE_C
#include "perl.h"
#include "dquote_static.c"

#define new_constant(a,b,c,d,e,f,g)	\
	S_new_constant(aTHX_ a,b,STR_WITH_LEN(c),d,e,f, g)

#define pl_yylval	(PL_parser->yylval)

/* XXX temporary backwards compatibility */
#define PL_lex_brackets		(PL_parser->lex_brackets)
#define PL_lex_allbrackets	(PL_parser->lex_allbrackets)
#define PL_lex_fakeeof		(PL_parser->lex_fakeeof)
#define PL_lex_brackstack	(PL_parser->lex_brackstack)
#define PL_lex_casemods		(PL_parser->lex_casemods)
#define PL_lex_casestack        (PL_parser->lex_casestack)
#define PL_lex_defer		(PL_parser->lex_defer)
#define PL_lex_dojoin		(PL_parser->lex_dojoin)
#define PL_lex_expect		(PL_parser->lex_expect)
#define PL_lex_formbrack        (PL_parser->lex_formbrack)
#define PL_lex_inpat		(PL_parser->lex_inpat)
#define PL_lex_inwhat		(PL_parser->lex_inwhat)
#define PL_lex_op		(PL_parser->lex_op)
#define PL_lex_repl		(PL_parser->lex_repl)
#define PL_lex_starts		(PL_parser->lex_starts)
#define PL_lex_stuff		(PL_parser->lex_stuff)
#define PL_multi_start		(PL_parser->multi_start)
#define PL_multi_open		(PL_parser->multi_open)
#define PL_multi_close		(PL_parser->multi_close)
#define PL_pending_ident        (PL_parser->pending_ident)
#define PL_preambled		(PL_parser->preambled)
#define PL_sublex_info		(PL_parser->sublex_info)
#define PL_linestr		(PL_parser->linestr)
#define PL_expect		(PL_parser->expect)
#define PL_copline		(PL_parser->copline)
#define PL_bufptr		(PL_parser->bufptr)
#define PL_oldbufptr		(PL_parser->oldbufptr)
#define PL_oldoldbufptr		(PL_parser->oldoldbufptr)
#define PL_linestart		(PL_parser->linestart)
#define PL_bufend		(PL_parser->bufend)
#define PL_last_uni		(PL_parser->last_uni)
#define PL_last_lop		(PL_parser->last_lop)
#define PL_last_lop_op		(PL_parser->last_lop_op)
#define PL_lex_state		(PL_parser->lex_state)
#define PL_rsfp			(PL_parser->rsfp)
#define PL_rsfp_filters		(PL_parser->rsfp_filters)
#define PL_in_my		(PL_parser->in_my)
#define PL_in_my_stash		(PL_parser->in_my_stash)
#define PL_tokenbuf		(PL_parser->tokenbuf)
#define PL_multi_end		(PL_parser->multi_end)
#define PL_error_count		(PL_parser->error_count)

#ifdef PERL_MAD
#  define PL_endwhite		(PL_parser->endwhite)
#  define PL_faketokens		(PL_parser->faketokens)
#  define PL_lasttoke		(PL_parser->lasttoke)
#  define PL_nextwhite		(PL_parser->nextwhite)
#  define PL_realtokenstart	(PL_parser->realtokenstart)
#  define PL_skipwhite		(PL_parser->skipwhite)
#  define PL_thisclose		(PL_parser->thisclose)
#  define PL_thismad		(PL_parser->thismad)
#  define PL_thisopen		(PL_parser->thisopen)
#  define PL_thisstuff		(PL_parser->thisstuff)
#  define PL_thistoken		(PL_parser->thistoken)
#  define PL_thiswhite		(PL_parser->thiswhite)
#  define PL_thiswhite		(PL_parser->thiswhite)
#  define PL_nexttoke		(PL_parser->nexttoke)
#  define PL_curforce		(PL_parser->curforce)
#else
#  define PL_nexttoke		(PL_parser->nexttoke)
#  define PL_nexttype		(PL_parser->nexttype)
#  define PL_nextval		(PL_parser->nextval)
#endif

/* This can't be done with embed.fnc, because struct yy_parser contains a
   member named pending_ident, which clashes with the generated #define  */
static int
S_pending_ident(pTHX);

static const char ident_too_long[] = "Identifier too long";

#ifdef PERL_MAD
#  define CURMAD(slot,sv) if (PL_madskills) { curmad(slot,sv); sv = 0; }
#  define NEXTVAL_NEXTTOKE PL_nexttoke[PL_curforce].next_val
#else
#  define CURMAD(slot,sv)
#  define NEXTVAL_NEXTTOKE PL_nextval[PL_nexttoke]
#endif

#define XENUMMASK  0x3f
#define XFAKEEOF   0x40
#define XFAKEBRACK 0x80

#ifdef USE_UTF8_SCRIPTS
#   define UTF (!IN_BYTES)
#else
#   define UTF ((PL_linestr && DO_UTF8(PL_linestr)) || ( !(PL_parser->lex_flags & LEX_IGNORE_UTF8_HINTS) && (PL_hints & HINT_UTF8)))
#endif

/* The maximum number of characters preceding the unrecognized one to display */
#define UNRECOGNIZED_PRECEDE_COUNT 10

/* In variables named $^X, these are the legal values for X.
 * 1999-02-27 mjd-perl-patch@plover.com */
#define isCONTROLVAR(x) (isUPPER(x) || strchr("[\\]^_?", (x)))

#define SPACE_OR_TAB(c) ((c)==' '||(c)=='\t')

/* LEX_* are values for PL_lex_state, the state of the lexer.
 * They are arranged oddly so that the guard on the switch statement
 * can get by with a single comparison (if the compiler is smart enough).
 */

/* #define LEX_NOTPARSING		11 is done in perl.h. */

#define LEX_NORMAL		10 /* normal code (ie not within "...")     */
#define LEX_INTERPNORMAL	 9 /* code within a string, eg "$foo[$x+1]" */
#define LEX_INTERPCASEMOD	 8 /* expecting a \U, \Q or \E etc          */
#define LEX_INTERPPUSH		 7 /* starting a new sublex parse level     */
#define LEX_INTERPSTART		 6 /* expecting the start of a $var         */

				   /* at end of code, eg "$x" followed by:  */
#define LEX_INTERPEND		 5 /* ... eg not one of [, { or ->          */
#define LEX_INTERPENDMAYBE	 4 /* ... eg one of [, { or ->              */

#define LEX_INTERPCONCAT	 3 /* expecting anything, eg at start of
				        string or after \E, $foo, etc       */
#define LEX_INTERPCONST		 2 /* NOT USED */
#define LEX_FORMLINE		 1 /* expecting a format line               */
#define LEX_KNOWNEXT		 0 /* next token known; just return it      */


#ifdef DEBUGGING
static const char* const lex_state_names[] = {
    "KNOWNEXT",
    "FORMLINE",
    "INTERPCONST",
    "INTERPCONCAT",
    "INTERPENDMAYBE",
    "INTERPEND",
    "INTERPSTART",
    "INTERPPUSH",
    "INTERPCASEMOD",
    "INTERPNORMAL",
    "NORMAL"
};
#endif

#ifdef ff_next
#undef ff_next
#endif

#include "keywords.h"

/* CLINE is a macro that ensures PL_copline has a sane value */

#ifdef CLINE
#undef CLINE
#endif
#define CLINE (PL_copline = (CopLINE(PL_curcop) < PL_copline ? CopLINE(PL_curcop) : PL_copline))

#ifdef PERL_MAD
#  define SKIPSPACE0(s) skipspace0(s)
#  define SKIPSPACE1(s) skipspace1(s)
#  define SKIPSPACE2(s,tsv) skipspace2(s,&tsv)
#  define PEEKSPACE(s) skipspace2(s,0)
#else
#  define SKIPSPACE0(s) skipspace(s)
#  define SKIPSPACE1(s) skipspace(s)
#  define SKIPSPACE2(s,tsv) skipspace(s)
#  define PEEKSPACE(s) skipspace(s)
#endif

/*
 * Convenience functions to return different tokens and prime the
 * lexer for the next token.  They all take an argument.
 *
 * TOKEN        : generic token (used for '(', DOLSHARP, etc)
 * OPERATOR     : generic operator
 * AOPERATOR    : assignment operator
 * PREBLOCK     : beginning the block after an if, while, foreach, ...
 * PRETERMBLOCK : beginning a non-code-defining {} block (eg, hash ref)
 * PREREF       : *EXPR where EXPR is not a simple identifier
 * TERM         : expression term
 * LOOPX        : loop exiting command (goto, last, dump, etc)
 * FTST         : file test operator
 * FUN0         : zero-argument function
 * FUN0OP       : zero-argument function, with its op created in this file
 * FUN1         : not used, except for not, which isn't a UNIOP
 * BOop         : bitwise or or xor
 * BAop         : bitwise and
 * SHop         : shift operator
 * PWop         : power operator
 * PMop         : pattern-matching operator
 * Aop          : addition-level operator
 * Mop          : multiplication-level operator
 * Eop          : equality-testing operator
 * Rop          : relational operator <= != gt
 *
 * Also see LOP and lop() below.
 */

#ifdef DEBUGGING /* Serve -DT. */
#   define REPORT(retval) tokereport((I32)retval, &pl_yylval)
#else
#   define REPORT(retval) (retval)
#endif

#define TOKEN(retval) return ( PL_bufptr = s, REPORT(retval))
#define OPERATOR(retval) return (PL_expect = XTERM, PL_bufptr = s, REPORT(retval))
#define AOPERATOR(retval) return ao((PL_expect = XTERM, PL_bufptr = s, REPORT(retval)))
#define PREBLOCK(retval) return (PL_expect = XBLOCK,PL_bufptr = s, REPORT(retval))
#define PRETERMBLOCK(retval) return (PL_expect = XTERMBLOCK,PL_bufptr = s, REPORT(retval))
#define PREREF(retval) return (PL_expect = XREF,PL_bufptr = s, REPORT(retval))
#define TERM(retval) return (CLINE, PL_expect = XOPERATOR, PL_bufptr = s, REPORT(retval))
#define LOOPX(f) return (pl_yylval.ival=f, PL_expect=XTERM, PL_bufptr=s, REPORT((int)LOOPEX))
#define FTST(f)  return (pl_yylval.ival=f, PL_expect=XTERMORDORDOR, PL_bufptr=s, REPORT((int)UNIOP))
#define FUN0(f)  return (pl_yylval.ival=f, PL_expect=XOPERATOR, PL_bufptr=s, REPORT((int)FUNC0))
#define FUN0OP(f)  return (pl_yylval.opval=f, CLINE, PL_expect=XOPERATOR, PL_bufptr=s, REPORT((int)FUNC0OP))
#define FUN1(f)  return (pl_yylval.ival=f, PL_expect=XOPERATOR, PL_bufptr=s, REPORT((int)FUNC1))
#define BOop(f)  return ao((pl_yylval.ival=f, PL_expect=XTERM, PL_bufptr=s, REPORT((int)BITOROP)))
#define BAop(f)  return ao((pl_yylval.ival=f, PL_expect=XTERM, PL_bufptr=s, REPORT((int)BITANDOP)))
#define SHop(f)  return ao((pl_yylval.ival=f, PL_expect=XTERM, PL_bufptr=s, REPORT((int)SHIFTOP)))
#define PWop(f)  return ao((pl_yylval.ival=f, PL_expect=XTERM, PL_bufptr=s, REPORT((int)POWOP)))
#define PMop(f)  return(pl_yylval.ival=f, PL_expect=XTERM, PL_bufptr=s, REPORT((int)MATCHOP))
#define Aop(f)   return ao((pl_yylval.ival=f, PL_expect=XTERM, PL_bufptr=s, REPORT((int)ADDOP)))
#define Mop(f)   return ao((pl_yylval.ival=f, PL_expect=XTERM, PL_bufptr=s, REPORT((int)MULOP)))
#define Eop(f)   return (pl_yylval.ival=f, PL_expect=XTERM, PL_bufptr=s, REPORT((int)EQOP))
#define Rop(f)   return (pl_yylval.ival=f, PL_expect=XTERM, PL_bufptr=s, REPORT((int)RELOP))

/* This bit of chicanery makes a unary function followed by
 * a parenthesis into a function with one argument, highest precedence.
 * The UNIDOR macro is for unary functions that can be followed by the //
 * operator (such as C<shift // 0>).
 */
#define UNI2(f,x) { \
	pl_yylval.ival = f; \
	PL_expect = x; \
	PL_bufptr = s; \
	PL_last_uni = PL_oldbufptr; \
	PL_last_lop_op = f; \
	if (*s == '(') \
	    return REPORT( (int)FUNC1 ); \
	s = PEEKSPACE(s); \
	return REPORT( *s=='(' ? (int)FUNC1 : (int)UNIOP ); \
	}
#define UNI(f)    UNI2(f,XTERM)
#define UNIDOR(f) UNI2(f,XTERMORDORDOR)
#define UNIPROTO(f,optional) { \
	if (optional) PL_last_uni = PL_oldbufptr; \
	OPERATOR(f); \
	}

#define UNIBRACK(f) { \
	pl_yylval.ival = f; \
	PL_bufptr = s; \
	PL_last_uni = PL_oldbufptr; \
	if (*s == '(') \
	    return REPORT( (int)FUNC1 ); \
	s = PEEKSPACE(s); \
	return REPORT( (*s == '(') ? (int)FUNC1 : (int)UNIOP ); \
	}

/* grandfather return to old style */
#define OLDLOP(f) \
	do { \
	    if (!PL_lex_allbrackets && PL_lex_fakeeof > LEX_FAKEEOF_LOWLOGIC) \
		PL_lex_fakeeof = LEX_FAKEEOF_LOWLOGIC; \
	    pl_yylval.ival = (f); \
	    PL_expect = XTERM; \
	    PL_bufptr = s; \
	    return (int)LSTOP; \
	} while(0)

#ifdef DEBUGGING

/* how to interpret the pl_yylval associated with the token */
enum token_type {
    TOKENTYPE_NONE,
    TOKENTYPE_IVAL,
    TOKENTYPE_OPNUM, /* pl_yylval.ival contains an opcode number */
    TOKENTYPE_PVAL,
    TOKENTYPE_OPVAL,
    TOKENTYPE_GVVAL
};

static struct debug_tokens {
    const int token;
    enum token_type type;
    const char *name;
} const debug_tokens[] =
{
    { ADDOP,		TOKENTYPE_OPNUM,	"ADDOP" },
    { ANDAND,		TOKENTYPE_NONE,		"ANDAND" },
    { ANDOP,		TOKENTYPE_NONE,		"ANDOP" },
    { ANONSUB,		TOKENTYPE_IVAL,		"ANONSUB" },
    { ARROW,		TOKENTYPE_NONE,		"ARROW" },
    { ASSIGNOP,		TOKENTYPE_OPNUM,	"ASSIGNOP" },
    { BITANDOP,		TOKENTYPE_OPNUM,	"BITANDOP" },
    { BITOROP,		TOKENTYPE_OPNUM,	"BITOROP" },
    { COLONATTR,	TOKENTYPE_NONE,		"COLONATTR" },
    { CONTINUE,		TOKENTYPE_NONE,		"CONTINUE" },
    { DEFAULT,		TOKENTYPE_NONE,		"DEFAULT" },
    { DO,		TOKENTYPE_NONE,		"DO" },
    { DOLSHARP,		TOKENTYPE_NONE,		"DOLSHARP" },
    { DORDOR,		TOKENTYPE_NONE,		"DORDOR" },
    { DOROP,		TOKENTYPE_OPNUM,	"DOROP" },
    { DOTDOT,		TOKENTYPE_IVAL,		"DOTDOT" },
    { ELSE,		TOKENTYPE_NONE,		"ELSE" },
    { ELSIF,		TOKENTYPE_IVAL,		"ELSIF" },
    { EQOP,		TOKENTYPE_OPNUM,	"EQOP" },
    { FOR,		TOKENTYPE_IVAL,		"FOR" },
    { FORMAT,		TOKENTYPE_NONE,		"FORMAT" },
    { FUNC,		TOKENTYPE_OPNUM,	"FUNC" },
    { FUNC0,		TOKENTYPE_OPNUM,	"FUNC0" },
    { FUNC0OP,		TOKENTYPE_OPVAL,	"FUNC0OP" },
    { FUNC0SUB,		TOKENTYPE_OPVAL,	"FUNC0SUB" },
    { FUNC1,		TOKENTYPE_OPNUM,	"FUNC1" },
    { FUNCMETH,		TOKENTYPE_OPVAL,	"FUNCMETH" },
    { GIVEN,		TOKENTYPE_IVAL,		"GIVEN" },
    { HASHBRACK,	TOKENTYPE_NONE,		"HASHBRACK" },
    { IF,		TOKENTYPE_IVAL,		"IF" },
    { LABEL,		TOKENTYPE_OPVAL,	"LABEL" },
    { LOCAL,		TOKENTYPE_IVAL,		"LOCAL" },
    { LOOPEX,		TOKENTYPE_OPNUM,	"LOOPEX" },
    { LSTOP,		TOKENTYPE_OPNUM,	"LSTOP" },
    { LSTOPSUB,		TOKENTYPE_OPVAL,	"LSTOPSUB" },
    { MATCHOP,		TOKENTYPE_OPNUM,	"MATCHOP" },
    { METHOD,		TOKENTYPE_OPVAL,	"METHOD" },
    { MULOP,		TOKENTYPE_OPNUM,	"MULOP" },
    { MY,		TOKENTYPE_IVAL,		"MY" },
    { MYSUB,		TOKENTYPE_NONE,		"MYSUB" },
    { NOAMP,		TOKENTYPE_NONE,		"NOAMP" },
    { NOTOP,		TOKENTYPE_NONE,		"NOTOP" },
    { OROP,		TOKENTYPE_IVAL,		"OROP" },
    { OROR,		TOKENTYPE_NONE,		"OROR" },
    { PACKAGE,		TOKENTYPE_NONE,		"PACKAGE" },
    { PLUGEXPR,		TOKENTYPE_OPVAL,	"PLUGEXPR" },
    { PLUGSTMT,		TOKENTYPE_OPVAL,	"PLUGSTMT" },
    { PMFUNC,		TOKENTYPE_OPVAL,	"PMFUNC" },
    { POSTDEC,		TOKENTYPE_NONE,		"POSTDEC" },
    { POSTINC,		TOKENTYPE_NONE,		"POSTINC" },
    { POWOP,		TOKENTYPE_OPNUM,	"POWOP" },
    { PREDEC,		TOKENTYPE_NONE,		"PREDEC" },
    { PREINC,		TOKENTYPE_NONE,		"PREINC" },
    { PRIVATEREF,	TOKENTYPE_OPVAL,	"PRIVATEREF" },
    { REFGEN,		TOKENTYPE_NONE,		"REFGEN" },
    { RELOP,		TOKENTYPE_OPNUM,	"RELOP" },
    { SHIFTOP,		TOKENTYPE_OPNUM,	"SHIFTOP" },
    { SUB,		TOKENTYPE_NONE,		"SUB" },
    { THING,		TOKENTYPE_OPVAL,	"THING" },
    { UMINUS,		TOKENTYPE_NONE,		"UMINUS" },
    { UNIOP,		TOKENTYPE_OPNUM,	"UNIOP" },
    { UNIOPSUB,		TOKENTYPE_OPVAL,	"UNIOPSUB" },
    { UNLESS,		TOKENTYPE_IVAL,		"UNLESS" },
    { UNTIL,		TOKENTYPE_IVAL,		"UNTIL" },
    { USE,		TOKENTYPE_IVAL,		"USE" },
    { WHEN,		TOKENTYPE_IVAL,		"WHEN" },
    { WHILE,		TOKENTYPE_IVAL,		"WHILE" },
    { WORD,		TOKENTYPE_OPVAL,	"WORD" },
    { YADAYADA,		TOKENTYPE_IVAL,		"YADAYADA" },
    { 0,		TOKENTYPE_NONE,		NULL }
};

/* dump the returned token in rv, plus any optional arg in pl_yylval */

STATIC int
S_tokereport(pTHX_ I32 rv, const YYSTYPE* lvalp)
{
    dVAR;

    PERL_ARGS_ASSERT_TOKEREPORT;

    if (DEBUG_T_TEST) {
	const char *name = NULL;
	enum token_type type = TOKENTYPE_NONE;
	const struct debug_tokens *p;
	SV* const report = newSVpvs("<== ");

	for (p = debug_tokens; p->token; p++) {
	    if (p->token == (int)rv) {
		name = p->name;
		type = p->type;
		break;
	    }
	}
	if (name)
	    Perl_sv_catpv(aTHX_ report, name);
	else if ((char)rv > ' ' && (char)rv < '~')
	    Perl_sv_catpvf(aTHX_ report, "'%c'", (char)rv);
	else if (!rv)
	    sv_catpvs(report, "EOF");
	else
	    Perl_sv_catpvf(aTHX_ report, "?? %"IVdf, (IV)rv);
	switch (type) {
	case TOKENTYPE_NONE:
	case TOKENTYPE_GVVAL: /* doesn't appear to be used */
	    break;
	case TOKENTYPE_IVAL:
	    Perl_sv_catpvf(aTHX_ report, "(ival=%"IVdf")", (IV)lvalp->ival);
	    break;
	case TOKENTYPE_OPNUM:
	    Perl_sv_catpvf(aTHX_ report, "(ival=op_%s)",
				    PL_op_name[lvalp->ival]);
	    break;
	case TOKENTYPE_PVAL:
	    Perl_sv_catpvf(aTHX_ report, "(pval=\"%s\")", lvalp->pval);
	    break;
	case TOKENTYPE_OPVAL:
	    if (lvalp->opval) {
		Perl_sv_catpvf(aTHX_ report, "(opval=op_%s)",
				    PL_op_name[lvalp->opval->op_type]);
		if (lvalp->opval->op_type == OP_CONST) {
		    Perl_sv_catpvf(aTHX_ report, " %s",
			SvPEEK(cSVOPx_sv(lvalp->opval)));
		}

	    }
	    else
		sv_catpvs(report, "(opval=null)");
	    break;
	}
        PerlIO_printf(Perl_debug_log, "### %s\n\n", SvPV_nolen_const(report));
    };
    return (int)rv;
}


/* print the buffer with suitable escapes */

STATIC void
S_printbuf(pTHX_ const char *const fmt, const char *const s)
{
    SV* const tmp = newSVpvs("");

    PERL_ARGS_ASSERT_PRINTBUF;

    PerlIO_printf(Perl_debug_log, fmt, pv_display(tmp, s, strlen(s), 0, 60));
    SvREFCNT_dec(tmp);
}

#endif

static int
S_deprecate_commaless_var_list(pTHX) {
    PL_expect = XTERM;
    deprecate("comma-less variable list");
    return REPORT(','); /* grandfather non-comma-format format */
}

/*
 * S_ao
 *
 * This subroutine detects &&=, ||=, and //= and turns an ANDAND, OROR or DORDOR
 * into an OP_ANDASSIGN, OP_ORASSIGN, or OP_DORASSIGN
 */

STATIC int
S_ao(pTHX_ int toketype)
{
    dVAR;
    if (*PL_bufptr == '=') {
	PL_bufptr++;
	if (toketype == ANDAND)
	    pl_yylval.ival = OP_ANDASSIGN;
	else if (toketype == OROR)
	    pl_yylval.ival = OP_ORASSIGN;
	else if (toketype == DORDOR)
	    pl_yylval.ival = OP_DORASSIGN;
	toketype = ASSIGNOP;
    }
    return toketype;
}

/*
 * S_no_op
 * When Perl expects an operator and finds something else, no_op
 * prints the warning.  It always prints "<something> found where
 * operator expected.  It prints "Missing semicolon on previous line?"
 * if the surprise occurs at the start of the line.  "do you need to
 * predeclare ..." is printed out for code like "sub bar; foo bar $x"
 * where the compiler doesn't know if foo is a method call or a function.
 * It prints "Missing operator before end of line" if there's nothing
 * after the missing operator, or "... before <...>" if there is something
 * after the missing operator.
 */

STATIC void
S_no_op(pTHX_ const char *const what, char *s)
{
    dVAR;
    char * const oldbp = PL_bufptr;
    const bool is_first = (PL_oldbufptr == PL_linestart);

    PERL_ARGS_ASSERT_NO_OP;

    if (!s)
	s = oldbp;
    else
	PL_bufptr = s;
    yywarn(Perl_form(aTHX_ "%s found where operator expected", what), UTF ? SVf_UTF8 : 0);
    if (ckWARN_d(WARN_SYNTAX)) {
	if (is_first)
	    Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
		    "\t(Missing semicolon on previous line?)\n");
	else if (PL_oldoldbufptr && isIDFIRST_lazy_if(PL_oldoldbufptr,UTF)) {
	    const char *t;
	    for (t = PL_oldoldbufptr; (isALNUM_lazy_if(t,UTF) || *t == ':');
                                                            t += UTF ? UTF8SKIP(t) : 1)
		NOOP;
	    if (t < PL_bufptr && isSPACE(*t))
		Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
			"\t(Do you need to predeclare %"SVf"?)\n",
		    SVfARG(newSVpvn_flags(PL_oldoldbufptr, (STRLEN)(t - PL_oldoldbufptr),
                                   SVs_TEMP | (UTF ? SVf_UTF8 : 0))));
	}
	else {
	    assert(s >= oldbp);
	    Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
		    "\t(Missing operator before %"SVf"?)\n",
                    SVfARG(newSVpvn_flags(oldbp, (STRLEN)(s - oldbp),
                                    SVs_TEMP | (UTF ? SVf_UTF8 : 0))));
	}
    }
    PL_bufptr = oldbp;
}

/*
 * S_missingterm
 * Complain about missing quote/regexp/heredoc terminator.
 * If it's called with NULL then it cauterizes the line buffer.
 * If we're in a delimited string and the delimiter is a control
 * character, it's reformatted into a two-char sequence like ^C.
 * This is fatal.
 */

STATIC void
S_missingterm(pTHX_ char *s)
{
    dVAR;
    char tmpbuf[3];
    char q;
    if (s) {
	char * const nl = strrchr(s,'\n');
	if (nl)
	    *nl = '\0';
    }
    else if (isCNTRL(PL_multi_close)) {
	*tmpbuf = '^';
	tmpbuf[1] = (char)toCTRL(PL_multi_close);
	tmpbuf[2] = '\0';
	s = tmpbuf;
    }
    else {
	*tmpbuf = (char)PL_multi_close;
	tmpbuf[1] = '\0';
	s = tmpbuf;
    }
    q = strchr(s,'"') ? '\'' : '"';
    Perl_croak(aTHX_ "Can't find string terminator %c%s%c anywhere before EOF",q,s,q);
}

#include "feature.h"

/*
 * Check whether the named feature is enabled.
 */
bool
Perl_feature_is_enabled(pTHX_ const char *const name, STRLEN namelen)
{
    dVAR;
    char he_name[8 + MAX_FEATURE_LEN] = "feature_";

    PERL_ARGS_ASSERT_FEATURE_IS_ENABLED;

    assert(CURRENT_FEATURE_BUNDLE == FEATURE_BUNDLE_CUSTOM);

    if (namelen > MAX_FEATURE_LEN)
	return FALSE;
    memcpy(&he_name[8], name, namelen);

    return cBOOL(cop_hints_fetch_pvn(PL_curcop, he_name, 8 + namelen, 0,
				     REFCOUNTED_HE_EXISTS));
}

/*
 * experimental text filters for win32 carriage-returns, utf16-to-utf8 and
 * utf16-to-utf8-reversed.
 */

#ifdef PERL_CR_FILTER
static void
strip_return(SV *sv)
{
    register const char *s = SvPVX_const(sv);
    register const char * const e = s + SvCUR(sv);

    PERL_ARGS_ASSERT_STRIP_RETURN;

    /* outer loop optimized to do nothing if there are no CR-LFs */
    while (s < e) {
	if (*s++ == '\r' && *s == '\n') {
	    /* hit a CR-LF, need to copy the rest */
	    register char *d = s - 1;
	    *d++ = *s++;
	    while (s < e) {
		if (*s == '\r' && s[1] == '\n')
		    s++;
		*d++ = *s++;
	    }
	    SvCUR(sv) -= s - d;
	    return;
	}
    }
}

STATIC I32
S_cr_textfilter(pTHX_ int idx, SV *sv, int maxlen)
{
    const I32 count = FILTER_READ(idx+1, sv, maxlen);
    if (count > 0 && !maxlen)
	strip_return(sv);
    return count;
}
#endif

/*
=for apidoc Amx|void|lex_start|SV *line|PerlIO *rsfp|U32 flags

Creates and initialises a new lexer/parser state object, supplying
a context in which to lex and parse from a new source of Perl code.
A pointer to the new state object is placed in L</PL_parser>.  An entry
is made on the save stack so that upon unwinding the new state object
will be destroyed and the former value of L</PL_parser> will be restored.
Nothing else need be done to clean up the parsing context.

The code to be parsed comes from I<line> and I<rsfp>.  I<line>, if
non-null, provides a string (in SV form) containing code to be parsed.
A copy of the string is made, so subsequent modification of I<line>
does not affect parsing.  I<rsfp>, if non-null, provides an input stream
from which code will be read to be parsed.  If both are non-null, the
code in I<line> comes first and must consist of complete lines of input,
and I<rsfp> supplies the remainder of the source.

The I<flags> parameter is reserved for future use.  Currently it is only
used by perl internally, so extensions should always pass zero.

=cut
*/

/* LEX_START_SAME_FILTER indicates that this is not a new file, so it
   can share filters with the current parser.
   LEX_START_DONT_CLOSE indicates that the file handle wasn't opened by the
   caller, hence isn't owned by the parser, so shouldn't be closed on parser
   destruction. This is used to handle the case of defaulting to reading the
   script from the standard input because no filename was given on the command
   line (without getting confused by situation where STDIN has been closed, so
   the script handle is opened on fd 0)  */

void
Perl_lex_start(pTHX_ SV *line, PerlIO *rsfp, U32 flags)
{
    dVAR;
    const char *s = NULL;
    yy_parser *parser, *oparser;
    if (flags && flags & ~LEX_START_FLAGS)
	Perl_croak(aTHX_ "Lexing code internal error (%s)", "lex_start");

    /* create and initialise a parser */

    Newxz(parser, 1, yy_parser);
    parser->old_parser = oparser = PL_parser;
    PL_parser = parser;

    parser->stack = NULL;
    parser->ps = NULL;
    parser->stack_size = 0;

    /* on scope exit, free this parser and restore any outer one */
    SAVEPARSER(parser);
    parser->saved_curcop = PL_curcop;

    /* initialise lexer state */

#ifdef PERL_MAD
    parser->curforce = -1;
#else
    parser->nexttoke = 0;
#endif
    parser->error_count = oparser ? oparser->error_count : 0;
    parser->copline = NOLINE;
    parser->lex_state = LEX_NORMAL;
    parser->expect = XSTATE;
    parser->rsfp = rsfp;
    parser->rsfp_filters =
      !(flags & LEX_START_SAME_FILTER) || !oparser
        ? NULL
        : MUTABLE_AV(SvREFCNT_inc(
            oparser->rsfp_filters
             ? oparser->rsfp_filters
             : (oparser->rsfp_filters = newAV())
          ));

    Newx(parser->lex_brackstack, 120, char);
    Newx(parser->lex_casestack, 12, char);
    *parser->lex_casestack = '\0';

    if (line) {
	STRLEN len;
	s = SvPV_const(line, len);
	parser->linestr = flags & LEX_START_COPIED
			    ? SvREFCNT_inc_simple_NN(line)
			    : newSVpvn_flags(s, len, SvUTF8(line));
	if (!len || s[len-1] != ';')
	    sv_catpvs(parser->linestr, "\n;");
    } else {
	parser->linestr = newSVpvs("\n;");
    }
    parser->oldoldbufptr =
	parser->oldbufptr =
	parser->bufptr =
	parser->linestart = SvPVX(parser->linestr);
    parser->bufend = parser->bufptr + SvCUR(parser->linestr);
    parser->last_lop = parser->last_uni = NULL;
    parser->lex_flags = flags & (LEX_IGNORE_UTF8_HINTS|LEX_EVALBYTES
				 |LEX_DONT_CLOSE_RSFP);

    parser->in_pod = parser->filtered = 0;
}


/* delete a parser object */

void
Perl_parser_free(pTHX_  const yy_parser *parser)
{
    PERL_ARGS_ASSERT_PARSER_FREE;

    PL_curcop = parser->saved_curcop;
    SvREFCNT_dec(parser->linestr);

    if (PL_parser->lex_flags & LEX_DONT_CLOSE_RSFP)
	PerlIO_clearerr(parser->rsfp);
    else if (parser->rsfp && (!parser->old_parser ||
		(parser->old_parser && parser->rsfp != parser->old_parser->rsfp)))
	PerlIO_close(parser->rsfp);
    SvREFCNT_dec(parser->rsfp_filters);

    Safefree(parser->lex_brackstack);
    Safefree(parser->lex_casestack);
    PL_parser = parser->old_parser;
    Safefree(parser);
}


/*
=for apidoc AmxU|SV *|PL_parser-E<gt>linestr

Buffer scalar containing the chunk currently under consideration of the
text currently being lexed.  This is always a plain string scalar (for
which C<SvPOK> is true).  It is not intended to be used as a scalar by
normal scalar means; instead refer to the buffer directly by the pointer
variables described below.

The lexer maintains various C<char*> pointers to things in the
C<PL_parser-E<gt>linestr> buffer.  If C<PL_parser-E<gt>linestr> is ever
reallocated, all of these pointers must be updated.  Don't attempt to
do this manually, but rather use L</lex_grow_linestr> if you need to
reallocate the buffer.

The content of the text chunk in the buffer is commonly exactly one
complete line of input, up to and including a newline terminator,
but there are situations where it is otherwise.  The octets of the
buffer may be intended to be interpreted as either UTF-8 or Latin-1.
The function L</lex_bufutf8> tells you which.  Do not use the C<SvUTF8>
flag on this scalar, which may disagree with it.

For direct examination of the buffer, the variable
L</PL_parser-E<gt>bufend> points to the end of the buffer.  The current
lexing position is pointed to by L</PL_parser-E<gt>bufptr>.  Direct use
of these pointers is usually preferable to examination of the scalar
through normal scalar means.

=for apidoc AmxU|char *|PL_parser-E<gt>bufend

Direct pointer to the end of the chunk of text currently being lexed, the
end of the lexer buffer.  This is equal to C<SvPVX(PL_parser-E<gt>linestr)
+ SvCUR(PL_parser-E<gt>linestr)>.  A NUL character (zero octet) is
always located at the end of the buffer, and does not count as part of
the buffer's contents.

=for apidoc AmxU|char *|PL_parser-E<gt>bufptr

Points to the current position of lexing inside the lexer buffer.
Characters around this point may be freely examined, within
the range delimited by C<SvPVX(L</PL_parser-E<gt>linestr>)> and
L</PL_parser-E<gt>bufend>.  The octets of the buffer may be intended to be
interpreted as either UTF-8 or Latin-1, as indicated by L</lex_bufutf8>.

Lexing code (whether in the Perl core or not) moves this pointer past
the characters that it consumes.  It is also expected to perform some
bookkeeping whenever a newline character is consumed.  This movement
can be more conveniently performed by the function L</lex_read_to>,
which handles newlines appropriately.

Interpretation of the buffer's octets can be abstracted out by
using the slightly higher-level functions L</lex_peek_unichar> and
L</lex_read_unichar>.

=for apidoc AmxU|char *|PL_parser-E<gt>linestart

Points to the start of the current line inside the lexer buffer.
This is useful for indicating at which column an error occurred, and
not much else.  This must be updated by any lexing code that consumes
a newline; the function L</lex_read_to> handles this detail.

=cut
*/

/*
=for apidoc Amx|bool|lex_bufutf8

Indicates whether the octets in the lexer buffer
(L</PL_parser-E<gt>linestr>) should be interpreted as the UTF-8 encoding
of Unicode characters.  If not, they should be interpreted as Latin-1
characters.  This is analogous to the C<SvUTF8> flag for scalars.

In UTF-8 mode, it is not guaranteed that the lexer buffer actually
contains valid UTF-8.  Lexing code must be robust in the face of invalid
encoding.

The actual C<SvUTF8> flag of the L</PL_parser-E<gt>linestr> scalar
is significant, but not the whole story regarding the input character
encoding.  Normally, when a file is being read, the scalar contains octets
and its C<SvUTF8> flag is off, but the octets should be interpreted as
UTF-8 if the C<use utf8> pragma is in effect.  During a string eval,
however, the scalar may have the C<SvUTF8> flag on, and in this case its
octets should be interpreted as UTF-8 unless the C<use bytes> pragma
is in effect.  This logic may change in the future; use this function
instead of implementing the logic yourself.

=cut
*/

bool
Perl_lex_bufutf8(pTHX)
{
    return UTF;
}

/*
=for apidoc Amx|char *|lex_grow_linestr|STRLEN len

Reallocates the lexer buffer (L</PL_parser-E<gt>linestr>) to accommodate
at least I<len> octets (including terminating NUL).  Returns a
pointer to the reallocated buffer.  This is necessary before making
any direct modification of the buffer that would increase its length.
L</lex_stuff_pvn> provides a more convenient way to insert text into
the buffer.

Do not use C<SvGROW> or C<sv_grow> directly on C<PL_parser-E<gt>linestr>;
this function updates all of the lexer's variables that point directly
into the buffer.

=cut
*/

char *
Perl_lex_grow_linestr(pTHX_ STRLEN len)
{
    SV *linestr;
    char *buf;
    STRLEN bufend_pos, bufptr_pos, oldbufptr_pos, oldoldbufptr_pos;
    STRLEN linestart_pos, last_uni_pos, last_lop_pos;
    linestr = PL_parser->linestr;
    buf = SvPVX(linestr);
    if (len <= SvLEN(linestr))
	return buf;
    bufend_pos = PL_parser->bufend - buf;
    bufptr_pos = PL_parser->bufptr - buf;
    oldbufptr_pos = PL_parser->oldbufptr - buf;
    oldoldbufptr_pos = PL_parser->oldoldbufptr - buf;
    linestart_pos = PL_parser->linestart - buf;
    last_uni_pos = PL_parser->last_uni ? PL_parser->last_uni - buf : 0;
    last_lop_pos = PL_parser->last_lop ? PL_parser->last_lop - buf : 0;
    buf = sv_grow(linestr, len);
    PL_parser->bufend = buf + bufend_pos;
    PL_parser->bufptr = buf + bufptr_pos;
    PL_parser->oldbufptr = buf + oldbufptr_pos;
    PL_parser->oldoldbufptr = buf + oldoldbufptr_pos;
    PL_parser->linestart = buf + linestart_pos;
    if (PL_parser->last_uni)
	PL_parser->last_uni = buf + last_uni_pos;
    if (PL_parser->last_lop)
	PL_parser->last_lop = buf + last_lop_pos;
    return buf;
}

/*
=for apidoc Amx|void|lex_stuff_pvn|const char *pv|STRLEN len|U32 flags

Insert characters into the lexer buffer (L</PL_parser-E<gt>linestr>),
immediately after the current lexing point (L</PL_parser-E<gt>bufptr>),
reallocating the buffer if necessary.  This means that lexing code that
runs later will see the characters as if they had appeared in the input.
It is not recommended to do this as part of normal parsing, and most
uses of this facility run the risk of the inserted characters being
interpreted in an unintended manner.

The string to be inserted is represented by I<len> octets starting
at I<pv>.  These octets are interpreted as either UTF-8 or Latin-1,
according to whether the C<LEX_STUFF_UTF8> flag is set in I<flags>.
The characters are recoded for the lexer buffer, according to how the
buffer is currently being interpreted (L</lex_bufutf8>).  If a string
to be inserted is available as a Perl scalar, the L</lex_stuff_sv>
function is more convenient.

=cut
*/

void
Perl_lex_stuff_pvn(pTHX_ const char *pv, STRLEN len, U32 flags)
{
    dVAR;
    char *bufptr;
    PERL_ARGS_ASSERT_LEX_STUFF_PVN;
    if (flags & ~(LEX_STUFF_UTF8))
	Perl_croak(aTHX_ "Lexing code internal error (%s)", "lex_stuff_pvn");
    if (UTF) {
	if (flags & LEX_STUFF_UTF8) {
	    goto plain_copy;
	} else {
	    STRLEN highhalf = 0;
	    const char *p, *e = pv+len;
	    for (p = pv; p != e; p++)
		highhalf += !!(((U8)*p) & 0x80);
	    if (!highhalf)
		goto plain_copy;
	    lex_grow_linestr(SvCUR(PL_parser->linestr)+1+len+highhalf);
	    bufptr = PL_parser->bufptr;
	    Move(bufptr, bufptr+len+highhalf, PL_parser->bufend+1-bufptr, char);
	    SvCUR_set(PL_parser->linestr,
	    	SvCUR(PL_parser->linestr) + len+highhalf);
	    PL_parser->bufend += len+highhalf;
	    for (p = pv; p != e; p++) {
		U8 c = (U8)*p;
		if (c & 0x80) {
		    *bufptr++ = (char)(0xc0 | (c >> 6));
		    *bufptr++ = (char)(0x80 | (c & 0x3f));
		} else {
		    *bufptr++ = (char)c;
		}
	    }
	}
    } else {
	if (flags & LEX_STUFF_UTF8) {
	    STRLEN highhalf = 0;
	    const char *p, *e = pv+len;
	    for (p = pv; p != e; p++) {
		U8 c = (U8)*p;
		if (c >= 0xc4) {
		    Perl_croak(aTHX_ "Lexing code attempted to stuff "
				"non-Latin-1 character into Latin-1 input");
		} else if (c >= 0xc2 && p+1 != e &&
			    (((U8)p[1]) & 0xc0) == 0x80) {
		    p++;
		    highhalf++;
		} else if (c >= 0x80) {
		    /* malformed UTF-8 */
		    ENTER;
		    SAVESPTR(PL_warnhook);
		    PL_warnhook = PERL_WARNHOOK_FATAL;
		    utf8n_to_uvuni((U8*)p, e-p, NULL, 0);
		    LEAVE;
		}
	    }
	    if (!highhalf)
		goto plain_copy;
	    lex_grow_linestr(SvCUR(PL_parser->linestr)+1+len-highhalf);
	    bufptr = PL_parser->bufptr;
	    Move(bufptr, bufptr+len-highhalf, PL_parser->bufend+1-bufptr, char);
	    SvCUR_set(PL_parser->linestr,
	    	SvCUR(PL_parser->linestr) + len-highhalf);
	    PL_parser->bufend += len-highhalf;
	    for (p = pv; p != e; p++) {
		U8 c = (U8)*p;
		if (c & 0x80) {
		    *bufptr++ = (char)(((c & 0x3) << 6) | (p[1] & 0x3f));
		    p++;
		} else {
		    *bufptr++ = (char)c;
		}
	    }
	} else {
	    plain_copy:
	    lex_grow_linestr(SvCUR(PL_parser->linestr)+1+len);
	    bufptr = PL_parser->bufptr;
	    Move(bufptr, bufptr+len, PL_parser->bufend+1-bufptr, char);
	    SvCUR_set(PL_parser->linestr, SvCUR(PL_parser->linestr) + len);
	    PL_parser->bufend += len;
	    Copy(pv, bufptr, len, char);
	}
    }
}

/*
=for apidoc Amx|void|lex_stuff_pv|const char *pv|U32 flags

Insert characters into the lexer buffer (L</PL_parser-E<gt>linestr>),
immediately after the current lexing point (L</PL_parser-E<gt>bufptr>),
reallocating the buffer if necessary.  This means that lexing code that
runs later will see the characters as if they had appeared in the input.
It is not recommended to do this as part of normal parsing, and most
uses of this facility run the risk of the inserted characters being
interpreted in an unintended manner.

The string to be inserted is represented by octets starting at I<pv>
and continuing to the first nul.  These octets are interpreted as either
UTF-8 or Latin-1, according to whether the C<LEX_STUFF_UTF8> flag is set
in I<flags>.  The characters are recoded for the lexer buffer, according
to how the buffer is currently being interpreted (L</lex_bufutf8>).
If it is not convenient to nul-terminate a string to be inserted, the
L</lex_stuff_pvn> function is more appropriate.

=cut
*/

void
Perl_lex_stuff_pv(pTHX_ const char *pv, U32 flags)
{
    PERL_ARGS_ASSERT_LEX_STUFF_PV;
    lex_stuff_pvn(pv, strlen(pv), flags);
}

/*
=for apidoc Amx|void|lex_stuff_sv|SV *sv|U32 flags

Insert characters into the lexer buffer (L</PL_parser-E<gt>linestr>),
immediately after the current lexing point (L</PL_parser-E<gt>bufptr>),
reallocating the buffer if necessary.  This means that lexing code that
runs later will see the characters as if they had appeared in the input.
It is not recommended to do this as part of normal parsing, and most
uses of this facility run the risk of the inserted characters being
interpreted in an unintended manner.

The string to be inserted is the string value of I<sv>.  The characters
are recoded for the lexer buffer, according to how the buffer is currently
being interpreted (L</lex_bufutf8>).  If a string to be inserted is
not already a Perl scalar, the L</lex_stuff_pvn> function avoids the
need to construct a scalar.

=cut
*/

void
Perl_lex_stuff_sv(pTHX_ SV *sv, U32 flags)
{
    char *pv;
    STRLEN len;
    PERL_ARGS_ASSERT_LEX_STUFF_SV;
    if (flags)
	Perl_croak(aTHX_ "Lexing code internal error (%s)", "lex_stuff_sv");
    pv = SvPV(sv, len);
    lex_stuff_pvn(pv, len, flags | (SvUTF8(sv) ? LEX_STUFF_UTF8 : 0));
}

/*
=for apidoc Amx|void|lex_unstuff|char *ptr

Discards text about to be lexed, from L</PL_parser-E<gt>bufptr> up to
I<ptr>.  Text following I<ptr> will be moved, and the buffer shortened.
This hides the discarded text from any lexing code that runs later,
as if the text had never appeared.

This is not the normal way to consume lexed text.  For that, use
L</lex_read_to>.

=cut
*/

void
Perl_lex_unstuff(pTHX_ char *ptr)
{
    char *buf, *bufend;
    STRLEN unstuff_len;
    PERL_ARGS_ASSERT_LEX_UNSTUFF;
    buf = PL_parser->bufptr;
    if (ptr < buf)
	Perl_croak(aTHX_ "Lexing code internal error (%s)", "lex_unstuff");
    if (ptr == buf)
	return;
    bufend = PL_parser->bufend;
    if (ptr > bufend)
	Perl_croak(aTHX_ "Lexing code internal error (%s)", "lex_unstuff");
    unstuff_len = ptr - buf;
    Move(ptr, buf, bufend+1-ptr, char);
    SvCUR_set(PL_parser->linestr, SvCUR(PL_parser->linestr) - unstuff_len);
    PL_parser->bufend = bufend - unstuff_len;
}

/*
=for apidoc Amx|void|lex_read_to|char *ptr

Consume text in the lexer buffer, from L</PL_parser-E<gt>bufptr> up
to I<ptr>.  This advances L</PL_parser-E<gt>bufptr> to match I<ptr>,
performing the correct bookkeeping whenever a newline character is passed.
This is the normal way to consume lexed text.

Interpretation of the buffer's octets can be abstracted out by
using the slightly higher-level functions L</lex_peek_unichar> and
L</lex_read_unichar>.

=cut
*/

void
Perl_lex_read_to(pTHX_ char *ptr)
{
    char *s;
    PERL_ARGS_ASSERT_LEX_READ_TO;
    s = PL_parser->bufptr;
    if (ptr < s || ptr > PL_parser->bufend)
	Perl_croak(aTHX_ "Lexing code internal error (%s)", "lex_read_to");
    for (; s != ptr; s++)
	if (*s == '\n') {
	    CopLINE_inc(PL_curcop);
	    PL_parser->linestart = s+1;
	}
    PL_parser->bufptr = ptr;
}

/*
=for apidoc Amx|void|lex_discard_to|char *ptr

Discards the first part of the L</PL_parser-E<gt>linestr> buffer,
up to I<ptr>.  The remaining content of the buffer will be moved, and
all pointers into the buffer updated appropriately.  I<ptr> must not
be later in the buffer than the position of L</PL_parser-E<gt>bufptr>:
it is not permitted to discard text that has yet to be lexed.

Normally it is not necessarily to do this directly, because it suffices to
use the implicit discarding behaviour of L</lex_next_chunk> and things
based on it.  However, if a token stretches across multiple lines,
and the lexing code has kept multiple lines of text in the buffer for
that purpose, then after completion of the token it would be wise to
explicitly discard the now-unneeded earlier lines, to avoid future
multi-line tokens growing the buffer without bound.

=cut
*/

void
Perl_lex_discard_to(pTHX_ char *ptr)
{
    char *buf;
    STRLEN discard_len;
    PERL_ARGS_ASSERT_LEX_DISCARD_TO;
    buf = SvPVX(PL_parser->linestr);
    if (ptr < buf)
	Perl_croak(aTHX_ "Lexing code internal error (%s)", "lex_discard_to");
    if (ptr == buf)
	return;
    if (ptr > PL_parser->bufptr)
	Perl_croak(aTHX_ "Lexing code internal error (%s)", "lex_discard_to");
    discard_len = ptr - buf;
    if (PL_parser->oldbufptr < ptr)
	PL_parser->oldbufptr = ptr;
    if (PL_parser->oldoldbufptr < ptr)
	PL_parser->oldoldbufptr = ptr;
    if (PL_parser->last_uni && PL_parser->last_uni < ptr)
	PL_parser->last_uni = NULL;
    if (PL_parser->last_lop && PL_parser->last_lop < ptr)
	PL_parser->last_lop = NULL;
    Move(ptr, buf, PL_parser->bufend+1-ptr, char);
    SvCUR_set(PL_parser->linestr, SvCUR(PL_parser->linestr) - discard_len);
    PL_parser->bufend -= discard_len;
    PL_parser->bufptr -= discard_len;
    PL_parser->oldbufptr -= discard_len;
    PL_parser->oldoldbufptr -= discard_len;
    if (PL_parser->last_uni)
	PL_parser->last_uni -= discard_len;
    if (PL_parser->last_lop)
	PL_parser->last_lop -= discard_len;
}

/*
=for apidoc Amx|bool|lex_next_chunk|U32 flags

Reads in the next chunk of text to be lexed, appending it to
L</PL_parser-E<gt>linestr>.  This should be called when lexing code has
looked to the end of the current chunk and wants to know more.  It is
usual, but not necessary, for lexing to have consumed the entirety of
the current chunk at this time.

If L</PL_parser-E<gt>bufptr> is pointing to the very end of the current
chunk (i.e., the current chunk has been entirely consumed), normally the
current chunk will be discarded at the same time that the new chunk is
read in.  If I<flags> includes C<LEX_KEEP_PREVIOUS>, the current chunk
will not be discarded.  If the current chunk has not been entirely
consumed, then it will not be discarded regardless of the flag.

Returns true if some new text was added to the buffer, or false if the
buffer has reached the end of the input text.

=cut
*/

#define LEX_FAKE_EOF 0x80000000

bool
Perl_lex_next_chunk(pTHX_ U32 flags)
{
    SV *linestr;
    char *buf;
    STRLEN old_bufend_pos, new_bufend_pos;
    STRLEN bufptr_pos, oldbufptr_pos, oldoldbufptr_pos;
    STRLEN linestart_pos, last_uni_pos, last_lop_pos;
    bool got_some_for_debugger = 0;
    bool got_some;
    if (flags & ~(LEX_KEEP_PREVIOUS|LEX_FAKE_EOF))
	Perl_croak(aTHX_ "Lexing code internal error (%s)", "lex_next_chunk");
    linestr = PL_parser->linestr;
    buf = SvPVX(linestr);
    if (!(flags & LEX_KEEP_PREVIOUS) &&
	    PL_parser->bufptr == PL_parser->bufend) {
	old_bufend_pos = bufptr_pos = oldbufptr_pos = oldoldbufptr_pos = 0;
	linestart_pos = 0;
	if (PL_parser->last_uni != PL_parser->bufend)
	    PL_parser->last_uni = NULL;
	if (PL_parser->last_lop != PL_parser->bufend)
	    PL_parser->last_lop = NULL;
	last_uni_pos = last_lop_pos = 0;
	*buf = 0;
	SvCUR(linestr) = 0;
    } else {
	old_bufend_pos = PL_parser->bufend - buf;
	bufptr_pos = PL_parser->bufptr - buf;
	oldbufptr_pos = PL_parser->oldbufptr - buf;
	oldoldbufptr_pos = PL_parser->oldoldbufptr - buf;
	linestart_pos = PL_parser->linestart - buf;
	last_uni_pos = PL_parser->last_uni ? PL_parser->last_uni - buf : 0;
	last_lop_pos = PL_parser->last_lop ? PL_parser->last_lop - buf : 0;
    }
    if (flags & LEX_FAKE_EOF) {
	goto eof;
    } else if (!PL_parser->rsfp && !PL_parser->filtered) {
	got_some = 0;
    } else if (filter_gets(linestr, old_bufend_pos)) {
	got_some = 1;
	got_some_for_debugger = 1;
    } else {
	if (!SvPOK(linestr))   /* can get undefined by filter_gets */
	    sv_setpvs(linestr, "");
	eof:
	/* End of real input.  Close filehandle (unless it was STDIN),
	 * then add implicit termination.
	 */
	if (PL_parser->lex_flags & LEX_DONT_CLOSE_RSFP)
	    PerlIO_clearerr(PL_parser->rsfp);
	else if (PL_parser->rsfp)
	    (void)PerlIO_close(PL_parser->rsfp);
	PL_parser->rsfp = NULL;
	PL_parser->in_pod = PL_parser->filtered = 0;
#ifdef PERL_MAD
	if (PL_madskills && !PL_in_eval && (PL_minus_p || PL_minus_n))
	    PL_faketokens = 1;
#endif
	if (!PL_in_eval && PL_minus_p) {
	    sv_catpvs(linestr,
		/*{*/";}continue{print or die qq(-p destination: $!\\n);}");
	    PL_minus_n = PL_minus_p = 0;
	} else if (!PL_in_eval && PL_minus_n) {
	    sv_catpvs(linestr, /*{*/";}");
	    PL_minus_n = 0;
	} else
	    sv_catpvs(linestr, ";");
	got_some = 1;
    }
    buf = SvPVX(linestr);
    new_bufend_pos = SvCUR(linestr);
    PL_parser->bufend = buf + new_bufend_pos;
    PL_parser->bufptr = buf + bufptr_pos;
    PL_parser->oldbufptr = buf + oldbufptr_pos;
    PL_parser->oldoldbufptr = buf + oldoldbufptr_pos;
    PL_parser->linestart = buf + linestart_pos;
    if (PL_parser->last_uni)
	PL_parser->last_uni = buf + last_uni_pos;
    if (PL_parser->last_lop)
	PL_parser->last_lop = buf + last_lop_pos;
    if (got_some_for_debugger && (PERLDB_LINE || PERLDB_SAVESRC) &&
	    PL_curstash != PL_debstash) {
	/* debugger active and we're not compiling the debugger code,
	 * so store the line into the debugger's array of lines
	 */
	update_debugger_info(NULL, buf+old_bufend_pos,
	    new_bufend_pos-old_bufend_pos);
    }
    return got_some;
}

/*
=for apidoc Amx|I32|lex_peek_unichar|U32 flags

Looks ahead one (Unicode) character in the text currently being lexed.
Returns the codepoint (unsigned integer value) of the next character,
or -1 if lexing has reached the end of the input text.  To consume the
peeked character, use L</lex_read_unichar>.

If the next character is in (or extends into) the next chunk of input
text, the next chunk will be read in.  Normally the current chunk will be
discarded at the same time, but if I<flags> includes C<LEX_KEEP_PREVIOUS>
then the current chunk will not be discarded.

If the input is being interpreted as UTF-8 and a UTF-8 encoding error
is encountered, an exception is generated.

=cut
*/

I32
Perl_lex_peek_unichar(pTHX_ U32 flags)
{
    dVAR;
    char *s, *bufend;
    if (flags & ~(LEX_KEEP_PREVIOUS))
	Perl_croak(aTHX_ "Lexing code internal error (%s)", "lex_peek_unichar");
    s = PL_parser->bufptr;
    bufend = PL_parser->bufend;
    if (UTF) {
	U8 head;
	I32 unichar;
	STRLEN len, retlen;
	if (s == bufend) {
	    if (!lex_next_chunk(flags))
		return -1;
	    s = PL_parser->bufptr;
	    bufend = PL_parser->bufend;
	}
	head = (U8)*s;
	if (!(head & 0x80))
	    return head;
	if (head & 0x40) {
	    len = PL_utf8skip[head];
	    while ((STRLEN)(bufend-s) < len) {
		if (!lex_next_chunk(flags | LEX_KEEP_PREVIOUS))
		    break;
		s = PL_parser->bufptr;
		bufend = PL_parser->bufend;
	    }
	}
	unichar = utf8n_to_uvuni((U8*)s, bufend-s, &retlen, UTF8_CHECK_ONLY);
	if (retlen == (STRLEN)-1) {
	    /* malformed UTF-8 */
	    ENTER;
	    SAVESPTR(PL_warnhook);
	    PL_warnhook = PERL_WARNHOOK_FATAL;
	    utf8n_to_uvuni((U8*)s, bufend-s, NULL, 0);
	    LEAVE;
	}
	return unichar;
    } else {
	if (s == bufend) {
	    if (!lex_next_chunk(flags))
		return -1;
	    s = PL_parser->bufptr;
	}
	return (U8)*s;
    }
}

/*
=for apidoc Amx|I32|lex_read_unichar|U32 flags

Reads the next (Unicode) character in the text currently being lexed.
Returns the codepoint (unsigned integer value) of the character read,
and moves L</PL_parser-E<gt>bufptr> past the character, or returns -1
if lexing has reached the end of the input text.  To non-destructively
examine the next character, use L</lex_peek_unichar> instead.

If the next character is in (or extends into) the next chunk of input
text, the next chunk will be read in.  Normally the current chunk will be
discarded at the same time, but if I<flags> includes C<LEX_KEEP_PREVIOUS>
then the current chunk will not be discarded.

If the input is being interpreted as UTF-8 and a UTF-8 encoding error
is encountered, an exception is generated.

=cut
*/

I32
Perl_lex_read_unichar(pTHX_ U32 flags)
{
    I32 c;
    if (flags & ~(LEX_KEEP_PREVIOUS))
	Perl_croak(aTHX_ "Lexing code internal error (%s)", "lex_read_unichar");
    c = lex_peek_unichar(flags);
    if (c != -1) {
	if (c == '\n')
	    CopLINE_inc(PL_curcop);
	if (UTF)
	    PL_parser->bufptr += UTF8SKIP(PL_parser->bufptr);
	else
	    ++(PL_parser->bufptr);
    }
    return c;
}

/*
=for apidoc Amx|void|lex_read_space|U32 flags

Reads optional spaces, in Perl style, in the text currently being
lexed.  The spaces may include ordinary whitespace characters and
Perl-style comments.  C<#line> directives are processed if encountered.
L</PL_parser-E<gt>bufptr> is moved past the spaces, so that it points
at a non-space character (or the end of the input text).

If spaces extend into the next chunk of input text, the next chunk will
be read in.  Normally the current chunk will be discarded at the same
time, but if I<flags> includes C<LEX_KEEP_PREVIOUS> then the current
chunk will not be discarded.

=cut
*/

#define LEX_NO_NEXT_CHUNK 0x80000000

void
Perl_lex_read_space(pTHX_ U32 flags)
{
    char *s, *bufend;
    bool need_incline = 0;
    if (flags & ~(LEX_KEEP_PREVIOUS|LEX_NO_NEXT_CHUNK))
	Perl_croak(aTHX_ "Lexing code internal error (%s)", "lex_read_space");
#ifdef PERL_MAD
    if (PL_skipwhite) {
	sv_free(PL_skipwhite);
	PL_skipwhite = NULL;
    }
    if (PL_madskills)
	PL_skipwhite = newSVpvs("");
#endif /* PERL_MAD */
    s = PL_parser->bufptr;
    bufend = PL_parser->bufend;
    while (1) {
	char c = *s;
	if (c == '#') {
	    do {
		c = *++s;
	    } while (!(c == '\n' || (c == 0 && s == bufend)));
	} else if (c == '\n') {
	    s++;
	    PL_parser->linestart = s;
	    if (s == bufend)
		need_incline = 1;
	    else
		incline(s);
	} else if (isSPACE(c)) {
	    s++;
	} else if (c == 0 && s == bufend) {
	    bool got_more;
#ifdef PERL_MAD
	    if (PL_madskills)
		sv_catpvn(PL_skipwhite, PL_parser->bufptr, s-PL_parser->bufptr);
#endif /* PERL_MAD */
	    if (flags & LEX_NO_NEXT_CHUNK)
		break;
	    PL_parser->bufptr = s;
	    CopLINE_inc(PL_curcop);
	    got_more = lex_next_chunk(flags);
	    CopLINE_dec(PL_curcop);
	    s = PL_parser->bufptr;
	    bufend = PL_parser->bufend;
	    if (!got_more)
		break;
	    if (need_incline && PL_parser->rsfp) {
		incline(s);
		need_incline = 0;
	    }
	} else {
	    break;
	}
    }
#ifdef PERL_MAD
    if (PL_madskills)
	sv_catpvn(PL_skipwhite, PL_parser->bufptr, s-PL_parser->bufptr);
#endif /* PERL_MAD */
    PL_parser->bufptr = s;
}

/*
 * S_incline
 * This subroutine has nothing to do with tilting, whether at windmills
 * or pinball tables.  Its name is short for "increment line".  It
 * increments the current line number in CopLINE(PL_curcop) and checks
 * to see whether the line starts with a comment of the form
 *    # line 500 "foo.pm"
 * If so, it sets the current line number and file to the values in the comment.
 */

STATIC void
S_incline(pTHX_ const char *s)
{
    dVAR;
    const char *t;
    const char *n;
    const char *e;
    line_t line_num;

    PERL_ARGS_ASSERT_INCLINE;

    CopLINE_inc(PL_curcop);
    if (*s++ != '#')
	return;
    while (SPACE_OR_TAB(*s))
	s++;
    if (strnEQ(s, "line", 4))
	s += 4;
    else
	return;
    if (SPACE_OR_TAB(*s))
	s++;
    else
	return;
    while (SPACE_OR_TAB(*s))
	s++;
    if (!isDIGIT(*s))
	return;

    n = s;
    while (isDIGIT(*s))
	s++;
    if (!SPACE_OR_TAB(*s) && *s != '\r' && *s != '\n' && *s != '\0')
	return;
    while (SPACE_OR_TAB(*s))
	s++;
    if (*s == '"' && (t = strchr(s+1, '"'))) {
	s++;
	e = t + 1;
    }
    else {
	t = s;
	while (!isSPACE(*t))
	    t++;
	e = t;
    }
    while (SPACE_OR_TAB(*e) || *e == '\r' || *e == '\f')
	e++;
    if (*e != '\n' && *e != '\0')
	return;		/* false alarm */

    line_num = atoi(n)-1;

    if (t - s > 0) {
	const STRLEN len = t - s;
	SV *const temp_sv = CopFILESV(PL_curcop);
	const char *cf;
	STRLEN tmplen;

	if (temp_sv) {
	    cf = SvPVX(temp_sv);
	    tmplen = SvCUR(temp_sv);
	} else {
	    cf = NULL;
	    tmplen = 0;
	}

	if (!PL_rsfp && !PL_parser->filtered) {
	    /* must copy *{"::_<(eval N)[oldfilename:L]"}
	     * to *{"::_<newfilename"} */
	    /* However, the long form of evals is only turned on by the
	       debugger - usually they're "(eval %lu)" */
	    char smallbuf[128];
	    char *tmpbuf;
	    GV **gvp;
	    STRLEN tmplen2 = len;
	    if (tmplen + 2 <= sizeof smallbuf)
		tmpbuf = smallbuf;
	    else
		Newx(tmpbuf, tmplen + 2, char);
	    tmpbuf[0] = '_';
	    tmpbuf[1] = '<';
	    memcpy(tmpbuf + 2, cf, tmplen);
	    tmplen += 2;
	    gvp = (GV**)hv_fetch(PL_defstash, tmpbuf, tmplen, FALSE);
	    if (gvp) {
		char *tmpbuf2;
		GV *gv2;

		if (tmplen2 + 2 <= sizeof smallbuf)
		    tmpbuf2 = smallbuf;
		else
		    Newx(tmpbuf2, tmplen2 + 2, char);

		if (tmpbuf2 != smallbuf || tmpbuf != smallbuf) {
		    /* Either they malloc'd it, or we malloc'd it,
		       so no prefix is present in ours.  */
		    tmpbuf2[0] = '_';
		    tmpbuf2[1] = '<';
		}

		memcpy(tmpbuf2 + 2, s, tmplen2);
		tmplen2 += 2;

		gv2 = *(GV**)hv_fetch(PL_defstash, tmpbuf2, tmplen2, TRUE);
		if (!isGV(gv2)) {
		    gv_init(gv2, PL_defstash, tmpbuf2, tmplen2, FALSE);
		    /* adjust ${"::_<newfilename"} to store the new file name */
		    GvSV(gv2) = newSVpvn(tmpbuf2 + 2, tmplen2 - 2);
		    /* The line number may differ. If that is the case,
		       alias the saved lines that are in the array.
		       Otherwise alias the whole array. */
		    if (CopLINE(PL_curcop) == line_num) {
			GvHV(gv2) = MUTABLE_HV(SvREFCNT_inc(GvHV(*gvp)));
			GvAV(gv2) = MUTABLE_AV(SvREFCNT_inc(GvAV(*gvp)));
		    }
		    else if (GvAV(*gvp)) {
			AV * const av = GvAV(*gvp);
			const I32 start = CopLINE(PL_curcop)+1;
			I32 items = AvFILLp(av) - start;
			if (items > 0) {
			    AV * const av2 = GvAVn(gv2);
			    SV **svp = AvARRAY(av) + start;
			    I32 l = (I32)line_num+1;
			    while (items--)
				av_store(av2, l++, SvREFCNT_inc(*svp++));
			}
		    }
		}

		if (tmpbuf2 != smallbuf) Safefree(tmpbuf2);
	    }
	    if (tmpbuf != smallbuf) Safefree(tmpbuf);
	}
	CopFILE_free(PL_curcop);
	CopFILE_setn(PL_curcop, s, len);
    }
    CopLINE_set(PL_curcop, line_num);
}

#ifdef PERL_MAD
/* skip space before PL_thistoken */

STATIC char *
S_skipspace0(pTHX_ register char *s)
{
    PERL_ARGS_ASSERT_SKIPSPACE0;

    s = skipspace(s);
    if (!PL_madskills)
	return s;
    if (PL_skipwhite) {
	if (!PL_thiswhite)
	    PL_thiswhite = newSVpvs("");
	sv_catsv(PL_thiswhite, PL_skipwhite);
	sv_free(PL_skipwhite);
	PL_skipwhite = 0;
    }
    PL_realtokenstart = s - SvPVX(PL_linestr);
    return s;
}

/* skip space after PL_thistoken */

STATIC char *
S_skipspace1(pTHX_ register char *s)
{
    const char *start = s;
    I32 startoff = start - SvPVX(PL_linestr);

    PERL_ARGS_ASSERT_SKIPSPACE1;

    s = skipspace(s);
    if (!PL_madskills)
	return s;
    start = SvPVX(PL_linestr) + startoff;
    if (!PL_thistoken && PL_realtokenstart >= 0) {
	const char * const tstart = SvPVX(PL_linestr) + PL_realtokenstart;
	PL_thistoken = newSVpvn(tstart, start - tstart);
    }
    PL_realtokenstart = -1;
    if (PL_skipwhite) {
	if (!PL_nextwhite)
	    PL_nextwhite = newSVpvs("");
	sv_catsv(PL_nextwhite, PL_skipwhite);
	sv_free(PL_skipwhite);
	PL_skipwhite = 0;
    }
    return s;
}

STATIC char *
S_skipspace2(pTHX_ register char *s, SV **svp)
{
    char *start;
    const I32 bufptroff = PL_bufptr - SvPVX(PL_linestr);
    const I32 startoff = s - SvPVX(PL_linestr);

    PERL_ARGS_ASSERT_SKIPSPACE2;

    s = skipspace(s);
    PL_bufptr = SvPVX(PL_linestr) + bufptroff;
    if (!PL_madskills || !svp)
	return s;
    start = SvPVX(PL_linestr) + startoff;
    if (!PL_thistoken && PL_realtokenstart >= 0) {
	char * const tstart = SvPVX(PL_linestr) + PL_realtokenstart;
	PL_thistoken = newSVpvn(tstart, start - tstart);
	PL_realtokenstart = -1;
    }
    if (PL_skipwhite) {
	if (!*svp)
	    *svp = newSVpvs("");
	sv_setsv(*svp, PL_skipwhite);
	sv_free(PL_skipwhite);
	PL_skipwhite = 0;
    }
    
    return s;
}
#endif

STATIC void
S_update_debugger_info(pTHX_ SV *orig_sv, const char *const buf, STRLEN len)
{
    AV *av = CopFILEAVx(PL_curcop);
    if (av) {
	SV * const sv = newSV_type(SVt_PVMG);
	if (orig_sv)
	    sv_setsv(sv, orig_sv);
	else
	    sv_setpvn(sv, buf, len);
	(void)SvIOK_on(sv);
	SvIV_set(sv, 0);
	av_store(av, (I32)CopLINE(PL_curcop), sv);
    }
}

/*
 * S_skipspace
 * Called to gobble the appropriate amount and type of whitespace.
 * Skips comments as well.
 */

STATIC char *
S_skipspace(pTHX_ register char *s)
{
#ifdef PERL_MAD
    char *start = s;
#endif /* PERL_MAD */
    PERL_ARGS_ASSERT_SKIPSPACE;
#ifdef PERL_MAD
    if (PL_skipwhite) {
	sv_free(PL_skipwhite);
	PL_skipwhite = NULL;
    }
#endif /* PERL_MAD */
    if (PL_lex_formbrack && PL_lex_brackets <= PL_lex_formbrack) {
	while (s < PL_bufend && SPACE_OR_TAB(*s))
	    s++;
    } else {
	STRLEN bufptr_pos = PL_bufptr - SvPVX(PL_linestr);
	PL_bufptr = s;
	lex_read_space(LEX_KEEP_PREVIOUS |
		(PL_sublex_info.sub_inwhat || PL_lex_state == LEX_FORMLINE ?
		    LEX_NO_NEXT_CHUNK : 0));
	s = PL_bufptr;
	PL_bufptr = SvPVX(PL_linestr) + bufptr_pos;
	if (PL_linestart > PL_bufptr)
	    PL_bufptr = PL_linestart;
	return s;
    }
#ifdef PERL_MAD
    if (PL_madskills)
	PL_skipwhite = newSVpvn(start, s-start);
#endif /* PERL_MAD */
    return s;
}

/*
 * S_check_uni
 * Check the unary operators to ensure there's no ambiguity in how they're
 * used.  An ambiguous piece of code would be:
 *     rand + 5
 * This doesn't mean rand() + 5.  Because rand() is a unary operator,
 * the +5 is its argument.
 */

STATIC void
S_check_uni(pTHX)
{
    dVAR;
    const char *s;
    const char *t;

    if (PL_oldoldbufptr != PL_last_uni)
	return;
    while (isSPACE(*PL_last_uni))
	PL_last_uni++;
    s = PL_last_uni;
    while (isALNUM_lazy_if(s,UTF) || *s == '-')
	s++;
    if ((t = strchr(s, '(')) && t < PL_bufptr)
	return;

    Perl_ck_warner_d(aTHX_ packWARN(WARN_AMBIGUOUS),
		     "Warning: Use of \"%.*s\" without parentheses is ambiguous",
		     (int)(s - PL_last_uni), PL_last_uni);
}

/*
 * LOP : macro to build a list operator.  Its behaviour has been replaced
 * with a subroutine, S_lop() for which LOP is just another name.
 */

#define LOP(f,x) return lop(f,x,s)

/*
 * S_lop
 * Build a list operator (or something that might be one).  The rules:
 *  - if we have a next token, then it's a list operator [why?]
 *  - if the next thing is an opening paren, then it's a function
 *  - else it's a list operator
 */

STATIC I32
S_lop(pTHX_ I32 f, int x, char *s)
{
    dVAR;

    PERL_ARGS_ASSERT_LOP;

    pl_yylval.ival = f;
    CLINE;
    PL_expect = x;
    PL_bufptr = s;
    PL_last_lop = PL_oldbufptr;
    PL_last_lop_op = (OPCODE)f;
#ifdef PERL_MAD
    if (PL_lasttoke)
	goto lstop;
#else
    if (PL_nexttoke)
	goto lstop;
#endif
    if (*s == '(')
	return REPORT(FUNC);
    s = PEEKSPACE(s);
    if (*s == '(')
	return REPORT(FUNC);
    else {
	lstop:
	if (!PL_lex_allbrackets && PL_lex_fakeeof > LEX_FAKEEOF_LOWLOGIC)
	    PL_lex_fakeeof = LEX_FAKEEOF_LOWLOGIC;
	return REPORT(LSTOP);
    }
}

#ifdef PERL_MAD
 /*
 * S_start_force
 * Sets up for an eventual force_next().  start_force(0) basically does
 * an unshift, while start_force(-1) does a push.  yylex removes items
 * on the "pop" end.
 */

STATIC void
S_start_force(pTHX_ int where)
{
    int i;

    if (where < 0)	/* so people can duplicate start_force(PL_curforce) */
	where = PL_lasttoke;
    assert(PL_curforce < 0 || PL_curforce == where);
    if (PL_curforce != where) {
	for (i = PL_lasttoke; i > where; --i) {
	    PL_nexttoke[i] = PL_nexttoke[i-1];
	}
	PL_lasttoke++;
    }
    if (PL_curforce < 0)	/* in case of duplicate start_force() */
	Zero(&PL_nexttoke[where], 1, NEXTTOKE);
    PL_curforce = where;
    if (PL_nextwhite) {
	if (PL_madskills)
	    curmad('^', newSVpvs(""));
	CURMAD('_', PL_nextwhite);
    }
}

STATIC void
S_curmad(pTHX_ char slot, SV *sv)
{
    MADPROP **where;

    if (!sv)
	return;
    if (PL_curforce < 0)
	where = &PL_thismad;
    else
	where = &PL_nexttoke[PL_curforce].next_mad;

    if (PL_faketokens)
	sv_setpvs(sv, "");
    else {
	if (!IN_BYTES) {
	    if (UTF && is_utf8_string((U8*)SvPVX(sv), SvCUR(sv)))
		SvUTF8_on(sv);
	    else if (PL_encoding) {
		sv_recode_to_utf8(sv, PL_encoding);
	    }
	}
    }

    /* keep a slot open for the head of the list? */
    if (slot != '_' && *where && (*where)->mad_key == '^') {
	(*where)->mad_key = slot;
	sv_free(MUTABLE_SV(((*where)->mad_val)));
	(*where)->mad_val = (void*)sv;
    }
    else
	addmad(newMADsv(slot, sv), where, 0);
}
#else
#  define start_force(where)    NOOP
#  define curmad(slot, sv)      NOOP
#endif

/*
 * S_force_next
 * When the lexer realizes it knows the next token (for instance,
 * it is reordering tokens for the parser) then it can call S_force_next
 * to know what token to return the next time the lexer is called.  Caller
 * will need to set PL_nextval[] (or PL_nexttoke[].next_val with PERL_MAD),
 * and possibly PL_expect to ensure the lexer handles the token correctly.
 */

STATIC void
S_force_next(pTHX_ I32 type)
{
    dVAR;
#ifdef DEBUGGING
    if (DEBUG_T_TEST) {
        PerlIO_printf(Perl_debug_log, "### forced token:\n");
	tokereport(type, &NEXTVAL_NEXTTOKE);
    }
#endif
#ifdef PERL_MAD
    if (PL_curforce < 0)
	start_force(PL_lasttoke);
    PL_nexttoke[PL_curforce].next_type = type;
    if (PL_lex_state != LEX_KNOWNEXT)
 	PL_lex_defer = PL_lex_state;
    PL_lex_state = LEX_KNOWNEXT;
    PL_lex_expect = PL_expect;
    PL_curforce = -1;
#else
    PL_nexttype[PL_nexttoke] = type;
    PL_nexttoke++;
    if (PL_lex_state != LEX_KNOWNEXT) {
	PL_lex_defer = PL_lex_state;
	PL_lex_expect = PL_expect;
	PL_lex_state = LEX_KNOWNEXT;
    }
#endif
}

void
Perl_yyunlex(pTHX)
{
    int yyc = PL_parser->yychar;
    if (yyc != YYEMPTY) {
	if (yyc) {
	    start_force(-1);
	    NEXTVAL_NEXTTOKE = PL_parser->yylval;
	    if (yyc == '{'/*}*/ || yyc == HASHBRACK || yyc == '['/*]*/) {
		PL_lex_allbrackets--;
		PL_lex_brackets--;
		yyc |= (3<<24) | (PL_lex_brackstack[PL_lex_brackets] << 16);
	    } else if (yyc == '('/*)*/) {
		PL_lex_allbrackets--;
		yyc |= (2<<24);
	    }
	    force_next(yyc);
	}
	PL_parser->yychar = YYEMPTY;
    }
}

STATIC SV *
S_newSV_maybe_utf8(pTHX_ const char *const start, STRLEN len)
{
    dVAR;
    SV * const sv = newSVpvn_utf8(start, len,
				  !IN_BYTES
				  && UTF
				  && !is_ascii_string((const U8*)start, len)
				  && is_utf8_string((const U8*)start, len));
    return sv;
}

/*
 * S_force_word
 * When the lexer knows the next thing is a word (for instance, it has
 * just seen -> and it knows that the next char is a word char, then
 * it calls S_force_word to stick the next word into the PL_nexttoke/val
 * lookahead.
 *
 * Arguments:
 *   char *start : buffer position (must be within PL_linestr)
 *   int token   : PL_next* will be this type of bare word (e.g., METHOD,WORD)
 *   int check_keyword : if true, Perl checks to make sure the word isn't
 *       a keyword (do this if the word is a label, e.g. goto FOO)
 *   int allow_pack : if true, : characters will also be allowed (require,
 *       use, etc. do this)
 *   int allow_initial_tick : used by the "sub" lexer only.
 */

STATIC char *
S_force_word(pTHX_ register char *start, int token, int check_keyword, int allow_pack, int allow_initial_tick)
{
    dVAR;
    register char *s;
    STRLEN len;

    PERL_ARGS_ASSERT_FORCE_WORD;

    start = SKIPSPACE1(start);
    s = start;
    if (isIDFIRST_lazy_if(s,UTF) ||
	(allow_pack && *s == ':') ||
	(allow_initial_tick && *s == '\'') )
    {
	s = scan_word(s, PL_tokenbuf, sizeof PL_tokenbuf, allow_pack, &len);
	if (check_keyword && keyword(PL_tokenbuf, len, 0))
	    return start;
	start_force(PL_curforce);
	if (PL_madskills)
	    curmad('X', newSVpvn(start,s-start));
	if (token == METHOD) {
	    s = SKIPSPACE1(s);
	    if (*s == '(')
		PL_expect = XTERM;
	    else {
		PL_expect = XOPERATOR;
	    }
	}
	if (PL_madskills)
	    curmad('g', newSVpvs( "forced" ));
	NEXTVAL_NEXTTOKE.opval
	    = (OP*)newSVOP(OP_CONST,0,
			   S_newSV_maybe_utf8(aTHX_ PL_tokenbuf, len));
	NEXTVAL_NEXTTOKE.opval->op_private |= OPpCONST_BARE;
	force_next(token);
    }
    return s;
}

/*
 * S_force_ident
 * Called when the lexer wants $foo *foo &foo etc, but the program
 * text only contains the "foo" portion.  The first argument is a pointer
 * to the "foo", and the second argument is the type symbol to prefix.
 * Forces the next token to be a "WORD".
 * Creates the symbol if it didn't already exist (via gv_fetchpv()).
 */

STATIC void
S_force_ident(pTHX_ register const char *s, int kind)
{
    dVAR;

    PERL_ARGS_ASSERT_FORCE_IDENT;

    if (*s) {
	const STRLEN len = strlen(s);
	OP* const o = (OP*)newSVOP(OP_CONST, 0, newSVpvn_flags(s, len,
                                                                UTF ? SVf_UTF8 : 0));
	start_force(PL_curforce);
	NEXTVAL_NEXTTOKE.opval = o;
	force_next(WORD);
	if (kind) {
	    o->op_private = OPpCONST_ENTERED;
	    /* XXX see note in pp_entereval() for why we forgo typo
	       warnings if the symbol must be introduced in an eval.
	       GSAR 96-10-12 */
	    gv_fetchpvn_flags(s, len,
			      (PL_in_eval ? (GV_ADDMULTI | GV_ADDINEVAL)
			      : GV_ADD) | ( UTF ? SVf_UTF8 : 0 ),
			      kind == '$' ? SVt_PV :
			      kind == '@' ? SVt_PVAV :
			      kind == '%' ? SVt_PVHV :
			      SVt_PVGV
			      );
	}
    }
}

NV
Perl_str_to_version(pTHX_ SV *sv)
{
    NV retval = 0.0;
    NV nshift = 1.0;
    STRLEN len;
    const char *start = SvPV_const(sv,len);
    const char * const end = start + len;
    const bool utf = SvUTF8(sv) ? TRUE : FALSE;

    PERL_ARGS_ASSERT_STR_TO_VERSION;

    while (start < end) {
	STRLEN skip;
	UV n;
	if (utf)
	    n = utf8n_to_uvchr((U8*)start, len, &skip, 0);
	else {
	    n = *(U8*)start;
	    skip = 1;
	}
	retval += ((NV)n)/nshift;
	start += skip;
	nshift *= 1000;
    }
    return retval;
}

/*
 * S_force_version
 * Forces the next token to be a version number.
 * If the next token appears to be an invalid version number, (e.g. "v2b"),
 * and if "guessing" is TRUE, then no new token is created (and the caller
 * must use an alternative parsing method).
 */

STATIC char *
S_force_version(pTHX_ char *s, int guessing)
{
    dVAR;
    OP *version = NULL;
    char *d;
#ifdef PERL_MAD
    I32 startoff = s - SvPVX(PL_linestr);
#endif

    PERL_ARGS_ASSERT_FORCE_VERSION;

    s = SKIPSPACE1(s);

    d = s;
    if (*d == 'v')
	d++;
    if (isDIGIT(*d)) {
	while (isDIGIT(*d) || *d == '_' || *d == '.')
	    d++;
#ifdef PERL_MAD
	if (PL_madskills) {
	    start_force(PL_curforce);
	    curmad('X', newSVpvn(s,d-s));
	}
#endif
        if (*d == ';' || isSPACE(*d) || *d == '{' || *d == '}' || !*d) {
	    SV *ver;
#ifdef USE_LOCALE_NUMERIC
	    char *loc = savepv(setlocale(LC_NUMERIC, NULL));
	    setlocale(LC_NUMERIC, "C");
#endif
            s = scan_num(s, &pl_yylval);
#ifdef USE_LOCALE_NUMERIC
	    setlocale(LC_NUMERIC, loc);
	    Safefree(loc);
#endif
            version = pl_yylval.opval;
	    ver = cSVOPx(version)->op_sv;
	    if (SvPOK(ver) && !SvNIOK(ver)) {
		SvUPGRADE(ver, SVt_PVNV);
		SvNV_set(ver, str_to_version(ver));
		SvNOK_on(ver);		/* hint that it is a version */
	    }
        }
	else if (guessing) {
#ifdef PERL_MAD
	    if (PL_madskills) {
		sv_free(PL_nextwhite);	/* let next token collect whitespace */
		PL_nextwhite = 0;
		s = SvPVX(PL_linestr) + startoff;
	    }
#endif
	    return s;
	}
    }

#ifdef PERL_MAD
    if (PL_madskills && !version) {
	sv_free(PL_nextwhite);	/* let next token collect whitespace */
	PL_nextwhite = 0;
	s = SvPVX(PL_linestr) + startoff;
    }
#endif
    /* NOTE: The parser sees the package name and the VERSION swapped */
    start_force(PL_curforce);
    NEXTVAL_NEXTTOKE.opval = version;
    force_next(WORD);

    return s;
}

/*
 * S_force_strict_version
 * Forces the next token to be a version number using strict syntax rules.
 */

STATIC char *
S_force_strict_version(pTHX_ char *s)
{
    dVAR;
    OP *version = NULL;
#ifdef PERL_MAD
    I32 startoff = s - SvPVX(PL_linestr);
#endif
    const char *errstr = NULL;

    PERL_ARGS_ASSERT_FORCE_STRICT_VERSION;

    while (isSPACE(*s)) /* leading whitespace */
	s++;

    if (is_STRICT_VERSION(s,&errstr)) {
	SV *ver = newSV(0);
	s = (char *)scan_version(s, ver, 0);
	version = newSVOP(OP_CONST, 0, ver);
    }
    else if ( (*s != ';' && *s != '{' && *s != '}' ) &&
	    (s = SKIPSPACE1(s), (*s != ';' && *s != '{' && *s != '}' )))
    {
	PL_bufptr = s;
	if (errstr)
	    yyerror(errstr); /* version required */
	return s;
    }

#ifdef PERL_MAD
    if (PL_madskills && !version) {
	sv_free(PL_nextwhite);	/* let next token collect whitespace */
	PL_nextwhite = 0;
	s = SvPVX(PL_linestr) + startoff;
    }
#endif
    /* NOTE: The parser sees the package name and the VERSION swapped */
    start_force(PL_curforce);
    NEXTVAL_NEXTTOKE.opval = version;
    force_next(WORD);

    return s;
}

/*
 * S_tokeq
 * Tokenize a quoted string passed in as an SV.  It finds the next
 * chunk, up to end of string or a backslash.  It may make a new
 * SV containing that chunk (if HINT_NEW_STRING is on).  It also
 * turns \\ into \.
 */

STATIC SV *
S_tokeq(pTHX_ SV *sv)
{
    dVAR;
    register char *s;
    register char *send;
    register char *d;
    STRLEN len = 0;
    SV *pv = sv;

    PERL_ARGS_ASSERT_TOKEQ;

    if (!SvLEN(sv))
	goto finish;

    s = SvPV_force(sv, len);
    if (SvTYPE(sv) >= SVt_PVIV && SvIVX(sv) == -1)
	goto finish;
    send = s + len;
    /* This is relying on the SV being "well formed" with a trailing '\0'  */
    while (s < send && !(*s == '\\' && s[1] == '\\'))
	s++;
    if (s == send)
	goto finish;
    d = s;
    if ( PL_hints & HINT_NEW_STRING ) {
	pv = newSVpvn_flags(SvPVX_const(pv), len, SVs_TEMP | SvUTF8(sv));
    }
    while (s < send) {
	if (*s == '\\') {
	    if (s + 1 < send && (s[1] == '\\'))
		s++;		/* all that, just for this */
	}
	*d++ = *s++;
    }
    *d = '\0';
    SvCUR_set(sv, d - SvPVX_const(sv));
  finish:
    if ( PL_hints & HINT_NEW_STRING )
       return new_constant(NULL, 0, "q", sv, pv, "q", 1);
    return sv;
}

/*
 * Now come three functions related to double-quote context,
 * S_sublex_start, S_sublex_push, and S_sublex_done.  They're used when
 * converting things like "\u\Lgnat" into ucfirst(lc("gnat")).  They
 * interact with PL_lex_state, and create fake ( ... ) argument lists
 * to handle functions and concatenation.
 * They assume that whoever calls them will be setting up a fake
 * join call, because each subthing puts a ',' after it.  This lets
 *   "lower \luPpEr"
 * become
 *  join($, , 'lower ', lcfirst( 'uPpEr', ) ,)
 *
 * (I'm not sure whether the spurious commas at the end of lcfirst's
 * arguments and join's arguments are created or not).
 */

/*
 * S_sublex_start
 * Assumes that pl_yylval.ival is the op we're creating (e.g. OP_LCFIRST).
 *
 * Pattern matching will set PL_lex_op to the pattern-matching op to
 * make (we return THING if pl_yylval.ival is OP_NULL, PMFUNC otherwise).
 *
 * OP_CONST and OP_READLINE are easy--just make the new op and return.
 *
 * Everything else becomes a FUNC.
 *
 * Sets PL_lex_state to LEX_INTERPPUSH unless (ival was OP_NULL or we
 * had an OP_CONST or OP_READLINE).  This just sets us up for a
 * call to S_sublex_push().
 */

STATIC I32
S_sublex_start(pTHX)
{
    dVAR;
    register const I32 op_type = pl_yylval.ival;

    if (op_type == OP_NULL) {
	pl_yylval.opval = PL_lex_op;
	PL_lex_op = NULL;
	return THING;
    }
    if (op_type == OP_CONST || op_type == OP_READLINE) {
	SV *sv = tokeq(PL_lex_stuff);

	if (SvTYPE(sv) == SVt_PVIV) {
	    /* Overloaded constants, nothing fancy: Convert to SVt_PV: */
	    STRLEN len;
	    const char * const p = SvPV_const(sv, len);
	    SV * const nsv = newSVpvn_flags(p, len, SvUTF8(sv));
	    SvREFCNT_dec(sv);
	    sv = nsv;
	}
	pl_yylval.opval = (OP*)newSVOP(op_type, 0, sv);
	PL_lex_stuff = NULL;
	/* Allow <FH> // "foo" */
	if (op_type == OP_READLINE)
	    PL_expect = XTERMORDORDOR;
	return THING;
    }
    else if (op_type == OP_BACKTICK && PL_lex_op) {
	/* readpipe() vas overriden */
	cSVOPx(cLISTOPx(cUNOPx(PL_lex_op)->op_first)->op_first->op_sibling)->op_sv = tokeq(PL_lex_stuff);
	pl_yylval.opval = PL_lex_op;
	PL_lex_op = NULL;
	PL_lex_stuff = NULL;
	return THING;
    }

    PL_sublex_info.super_state = PL_lex_state;
    PL_sublex_info.sub_inwhat = (U16)op_type;
    PL_sublex_info.sub_op = PL_lex_op;
    PL_lex_state = LEX_INTERPPUSH;

    PL_expect = XTERM;
    if (PL_lex_op) {
	pl_yylval.opval = PL_lex_op;
	PL_lex_op = NULL;
	return PMFUNC;
    }
    else
	return FUNC;
}

/*
 * S_sublex_push
 * Create a new scope to save the lexing state.  The scope will be
 * ended in S_sublex_done.  Returns a '(', starting the function arguments
 * to the uc, lc, etc. found before.
 * Sets PL_lex_state to LEX_INTERPCONCAT.
 */

STATIC I32
S_sublex_push(pTHX)
{
    dVAR;
    ENTER;

    PL_lex_state = PL_sublex_info.super_state;
    SAVEBOOL(PL_lex_dojoin);
    SAVEI32(PL_lex_brackets);
    SAVEI32(PL_lex_allbrackets);
    SAVEI8(PL_lex_fakeeof);
    SAVEI32(PL_lex_casemods);
    SAVEI32(PL_lex_starts);
    SAVEI8(PL_lex_state);
    SAVEVPTR(PL_lex_inpat);
    SAVEI16(PL_lex_inwhat);
    SAVECOPLINE(PL_curcop);
    SAVEPPTR(PL_bufptr);
    SAVEPPTR(PL_bufend);
    SAVEPPTR(PL_oldbufptr);
    SAVEPPTR(PL_oldoldbufptr);
    SAVEPPTR(PL_last_lop);
    SAVEPPTR(PL_last_uni);
    SAVEPPTR(PL_linestart);
    SAVESPTR(PL_linestr);
    SAVEGENERICPV(PL_lex_brackstack);
    SAVEGENERICPV(PL_lex_casestack);

    PL_linestr = PL_lex_stuff;
    PL_lex_stuff = NULL;

    PL_bufend = PL_bufptr = PL_oldbufptr = PL_oldoldbufptr = PL_linestart
	= SvPVX(PL_linestr);
    PL_bufend += SvCUR(PL_linestr);
    PL_last_lop = PL_last_uni = NULL;
    SAVEFREESV(PL_linestr);

    PL_lex_dojoin = FALSE;
    PL_lex_brackets = 0;
    PL_lex_allbrackets = 0;
    PL_lex_fakeeof = LEX_FAKEEOF_NEVER;
    Newx(PL_lex_brackstack, 120, char);
    Newx(PL_lex_casestack, 12, char);
    PL_lex_casemods = 0;
    *PL_lex_casestack = '\0';
    PL_lex_starts = 0;
    PL_lex_state = LEX_INTERPCONCAT;
    CopLINE_set(PL_curcop, (line_t)PL_multi_start);

    PL_lex_inwhat = PL_sublex_info.sub_inwhat;
    if (PL_lex_inwhat == OP_TRANSR) PL_lex_inwhat = OP_TRANS;
    if (PL_lex_inwhat == OP_MATCH || PL_lex_inwhat == OP_QR || PL_lex_inwhat == OP_SUBST)
	PL_lex_inpat = PL_sublex_info.sub_op;
    else
	PL_lex_inpat = NULL;

    return '(';
}

/*
 * S_sublex_done
 * Restores lexer state after a S_sublex_push.
 */

STATIC I32
S_sublex_done(pTHX)
{
    dVAR;
    if (!PL_lex_starts++) {
	SV * const sv = newSVpvs("");
	if (SvUTF8(PL_linestr))
	    SvUTF8_on(sv);
	PL_expect = XOPERATOR;
	pl_yylval.opval = (OP*)newSVOP(OP_CONST, 0, sv);
	return THING;
    }

    if (PL_lex_casemods) {		/* oops, we've got some unbalanced parens */
	PL_lex_state = LEX_INTERPCASEMOD;
	return yylex();
    }

    /* Is there a right-hand side to take care of? (s//RHS/ or tr//RHS/) */
    assert(PL_lex_inwhat != OP_TRANSR);
    if (PL_lex_repl && (PL_lex_inwhat == OP_SUBST || PL_lex_inwhat == OP_TRANS)) {
	PL_linestr = PL_lex_repl;
	PL_lex_inpat = 0;
	PL_bufend = PL_bufptr = PL_oldbufptr = PL_oldoldbufptr = PL_linestart = SvPVX(PL_linestr);
	PL_bufend += SvCUR(PL_linestr);
	PL_last_lop = PL_last_uni = NULL;
	SAVEFREESV(PL_linestr);
	PL_lex_dojoin = FALSE;
	PL_lex_brackets = 0;
	PL_lex_allbrackets = 0;
	PL_lex_fakeeof = LEX_FAKEEOF_NEVER;
	PL_lex_casemods = 0;
	*PL_lex_casestack = '\0';
	PL_lex_starts = 0;
	if (SvEVALED(PL_lex_repl)) {
	    PL_lex_state = LEX_INTERPNORMAL;
	    PL_lex_starts++;
	    /*	we don't clear PL_lex_repl here, so that we can check later
		whether this is an evalled subst; that means we rely on the
		logic to ensure sublex_done() is called again only via the
		branch (in yylex()) that clears PL_lex_repl, else we'll loop */
	}
	else {
	    PL_lex_state = LEX_INTERPCONCAT;
	    PL_lex_repl = NULL;
	}
	return ',';
    }
    else {
#ifdef PERL_MAD
	if (PL_madskills) {
	    if (PL_thiswhite) {
		if (!PL_endwhite)
		    PL_endwhite = newSVpvs("");
		sv_catsv(PL_endwhite, PL_thiswhite);
		PL_thiswhite = 0;
	    }
	    if (PL_thistoken)
		sv_setpvs(PL_thistoken,"");
	    else
		PL_realtokenstart = -1;
	}
#endif
	LEAVE;
	PL_bufend = SvPVX(PL_linestr);
	PL_bufend += SvCUR(PL_linestr);
	PL_expect = XOPERATOR;
	PL_sublex_info.sub_inwhat = 0;
	return ')';
    }
}

/*
  scan_const

  Extracts a pattern, double-quoted string, or transliteration.  This
  is terrifying code.

  It looks at PL_lex_inwhat and PL_lex_inpat to find out whether it's
  processing a pattern (PL_lex_inpat is true), a transliteration
  (PL_lex_inwhat == OP_TRANS is true), or a double-quoted string.

  Returns a pointer to the character scanned up to. If this is
  advanced from the start pointer supplied (i.e. if anything was
  successfully parsed), will leave an OP for the substring scanned
  in pl_yylval. Caller must intuit reason for not parsing further
  by looking at the next characters herself.

  In patterns:
    backslashes:
      constants: \N{NAME} only
      case and quoting: \U \Q \E
    stops on @ and $, but not for $ as tail anchor

  In transliterations:
    characters are VERY literal, except for - not at the start or end
    of the string, which indicates a range. If the range is in bytes,
    scan_const expands the range to the full set of intermediate
    characters. If the range is in utf8, the hyphen is replaced with
    a certain range mark which will be handled by pmtrans() in op.c.

  In double-quoted strings:
    backslashes:
      double-quoted style: \r and \n
      constants: \x31, etc.
      deprecated backrefs: \1 (in substitution replacements)
      case and quoting: \U \Q \E
    stops on @ and $

  scan_const does *not* construct ops to handle interpolated strings.
  It stops processing as soon as it finds an embedded $ or @ variable
  and leaves it to the caller to work out what's going on.

  embedded arrays (whether in pattern or not) could be:
      @foo, @::foo, @'foo, @{foo}, @$foo, @+, @-.

  $ in double-quoted strings must be the symbol of an embedded scalar.

  $ in pattern could be $foo or could be tail anchor.  Assumption:
  it's a tail anchor if $ is the last thing in the string, or if it's
  followed by one of "()| \r\n\t"

  \1 (backreferences) are turned into $1

  The structure of the code is
      while (there's a character to process) {
	  handle transliteration ranges
	  skip regexp comments /(?#comment)/ and codes /(?{code})/
	  skip #-initiated comments in //x patterns
	  check for embedded arrays
	  check for embedded scalars
	  if (backslash) {
	      deprecate \1 in substitution replacements
	      handle string-changing backslashes \l \U \Q \E, etc.
	      switch (what was escaped) {
		  handle \- in a transliteration (becomes a literal -)
		  if a pattern and not \N{, go treat as regular character
		  handle \132 (octal characters)
		  handle \x15 and \x{1234} (hex characters)
		  handle \N{name} (named characters, also \N{3,5} in a pattern)
		  handle \cV (control characters)
		  handle printf-style backslashes (\f, \r, \n, etc)
	      } (end switch)
	      continue
	  } (end if backslash)
          handle regular character
    } (end while character to read)
		
*/

STATIC char *
S_scan_const(pTHX_ char *start)
{
    dVAR;
    register char *send = PL_bufend;		/* end of the constant */
    SV *sv = newSV(send - start);		/* sv for the constant.  See
						   note below on sizing. */
    register char *s = start;			/* start of the constant */
    register char *d = SvPVX(sv);		/* destination for copies */
    bool dorange = FALSE;			/* are we in a translit range? */
    bool didrange = FALSE;		        /* did we just finish a range? */
    bool has_utf8 = FALSE;			/* Output constant is UTF8 */
    bool  this_utf8 = cBOOL(UTF);		/* Is the source string assumed
						   to be UTF8?  But, this can
						   show as true when the source
						   isn't utf8, as for example
						   when it is entirely composed
						   of hex constants */

    /* Note on sizing:  The scanned constant is placed into sv, which is
     * initialized by newSV() assuming one byte of output for every byte of
     * input.  This routine expects newSV() to allocate an extra byte for a
     * trailing NUL, which this routine will append if it gets to the end of
     * the input.  There may be more bytes of input than output (eg., \N{LATIN
     * CAPITAL LETTER A}), or more output than input if the constant ends up
     * recoded to utf8, but each time a construct is found that might increase
     * the needed size, SvGROW() is called.  Its size parameter each time is
     * based on the best guess estimate at the time, namely the length used so
     * far, plus the length the current construct will occupy, plus room for
     * the trailing NUL, plus one byte for every input byte still unscanned */ 

    UV uv;
#ifdef EBCDIC
    UV literal_endpoint = 0;
    bool native_range = TRUE; /* turned to FALSE if the first endpoint is Unicode. */
#endif

    PERL_ARGS_ASSERT_SCAN_CONST;

    assert(PL_lex_inwhat != OP_TRANSR);
    if (PL_lex_inwhat == OP_TRANS && PL_sublex_info.sub_op) {
	/* If we are doing a trans and we know we want UTF8 set expectation */
	has_utf8   = PL_sublex_info.sub_op->op_private & (OPpTRANS_FROM_UTF|OPpTRANS_TO_UTF);
	this_utf8  = PL_sublex_info.sub_op->op_private & (PL_lex_repl ? OPpTRANS_FROM_UTF : OPpTRANS_TO_UTF);
    }


    while (s < send || dorange) {

        /* get transliterations out of the way (they're most literal) */
	if (PL_lex_inwhat == OP_TRANS) {
	    /* expand a range A-Z to the full set of characters.  AIE! */
	    if (dorange) {
		I32 i;				/* current expanded character */
		I32 min;			/* first character in range */
		I32 max;			/* last character in range */

#ifdef EBCDIC
		UV uvmax = 0;
#endif

		if (has_utf8
#ifdef EBCDIC
		    && !native_range
#endif
		    ) {
		    char * const c = (char*)utf8_hop((U8*)d, -1);
		    char *e = d++;
		    while (e-- > c)
			*(e + 1) = *e;
		    *c = (char)UTF_TO_NATIVE(0xff);
		    /* mark the range as done, and continue */
		    dorange = FALSE;
		    didrange = TRUE;
		    continue;
		}

		i = d - SvPVX_const(sv);		/* remember current offset */
#ifdef EBCDIC
                SvGROW(sv,
		       SvLEN(sv) + (has_utf8 ?
				    (512 - UTF_CONTINUATION_MARK +
				     UNISKIP(0x100))
				    : 256));
                /* How many two-byte within 0..255: 128 in UTF-8,
		 * 96 in UTF-8-mod. */
#else
		SvGROW(sv, SvLEN(sv) + 256);	/* never more than 256 chars in a range */
#endif
		d = SvPVX(sv) + i;		/* refresh d after realloc */
#ifdef EBCDIC
                if (has_utf8) {
                    int j;
                    for (j = 0; j <= 1; j++) {
                        char * const c = (char*)utf8_hop((U8*)d, -1);
                        const UV uv    = utf8n_to_uvchr((U8*)c, d - c, NULL, 0);
                        if (j)
                            min = (U8)uv;
                        else if (uv < 256)
                            max = (U8)uv;
                        else {
                            max = (U8)0xff; /* only to \xff */
                            uvmax = uv; /* \x{100} to uvmax */
                        }
                        d = c; /* eat endpoint chars */
                     }
                }
               else {
#endif
		   d -= 2;		/* eat the first char and the - */
		   min = (U8)*d;	/* first char in range */
		   max = (U8)d[1];	/* last char in range  */
#ifdef EBCDIC
	       }
#endif

                if (min > max) {
		    Perl_croak(aTHX_
			       "Invalid range \"%c-%c\" in transliteration operator",
			       (char)min, (char)max);
                }

#ifdef EBCDIC
		if (literal_endpoint == 2 &&
		    ((isLOWER(min) && isLOWER(max)) ||
		     (isUPPER(min) && isUPPER(max)))) {
		    if (isLOWER(min)) {
			for (i = min; i <= max; i++)
			    if (isLOWER(i))
				*d++ = NATIVE_TO_NEED(has_utf8,i);
		    } else {
			for (i = min; i <= max; i++)
			    if (isUPPER(i))
				*d++ = NATIVE_TO_NEED(has_utf8,i);
		    }
		}
		else
#endif
		    for (i = min; i <= max; i++)
#ifdef EBCDIC
                        if (has_utf8) {
                            const U8 ch = (U8)NATIVE_TO_UTF(i);
                            if (UNI_IS_INVARIANT(ch))
                                *d++ = (U8)i;
                            else {
                                *d++ = (U8)UTF8_EIGHT_BIT_HI(ch);
                                *d++ = (U8)UTF8_EIGHT_BIT_LO(ch);
                            }
                        }
                        else
#endif
                            *d++ = (char)i;
 
#ifdef EBCDIC
                if (uvmax) {
                    d = (char*)uvchr_to_utf8((U8*)d, 0x100);
                    if (uvmax > 0x101)
                        *d++ = (char)UTF_TO_NATIVE(0xff);
                    if (uvmax > 0x100)
                        d = (char*)uvchr_to_utf8((U8*)d, uvmax);
                }
#endif

		/* mark the range as done, and continue */
		dorange = FALSE;
		didrange = TRUE;
#ifdef EBCDIC
		literal_endpoint = 0;
#endif
		continue;
	    }

	    /* range begins (ignore - as first or last char) */
	    else if (*s == '-' && s+1 < send  && s != start) {
		if (didrange) {
		    Perl_croak(aTHX_ "Ambiguous range in transliteration operator");
		}
		if (has_utf8
#ifdef EBCDIC
		    && !native_range
#endif
		    ) {
		    *d++ = (char)UTF_TO_NATIVE(0xff);	/* use illegal utf8 byte--see pmtrans */
		    s++;
		    continue;
		}
		dorange = TRUE;
		s++;
	    }
	    else {
		didrange = FALSE;
#ifdef EBCDIC
		literal_endpoint = 0;
		native_range = TRUE;
#endif
	    }
	}

	/* if we get here, we're not doing a transliteration */

	/* skip for regexp comments /(?#comment)/ and code /(?{code})/,
	   except for the last char, which will be done separately. */
	else if (*s == '(' && PL_lex_inpat && s[1] == '?') {
	    if (s[2] == '#') {
		while (s+1 < send && *s != ')')
		    *d++ = NATIVE_TO_NEED(has_utf8,*s++);
	    }
	    else if (s[2] == '{' /* This should match regcomp.c */
		    || (s[2] == '?' && s[3] == '{'))
	    {
		I32 count = 1;
		char *regparse = s + (s[2] == '{' ? 3 : 4);
		char c;

		while (count && (c = *regparse)) {
		    if (c == '\\' && regparse[1])
			regparse++;
		    else if (c == '{')
			count++;
		    else if (c == '}')
			count--;
		    regparse++;
		}
		if (*regparse != ')')
		    regparse--;		/* Leave one char for continuation. */
		while (s < regparse)
		    *d++ = NATIVE_TO_NEED(has_utf8,*s++);
	    }
	}

	/* likewise skip #-initiated comments in //x patterns */
	else if (*s == '#' && PL_lex_inpat &&
	  ((PMOP*)PL_lex_inpat)->op_pmflags & RXf_PMf_EXTENDED) {
	    while (s+1 < send && *s != '\n')
		*d++ = NATIVE_TO_NEED(has_utf8,*s++);
	}

	/* check for embedded arrays
	   (@foo, @::foo, @'foo, @{foo}, @$foo, @+, @-)
	   */
	else if (*s == '@' && s[1]) {
	    if (isALNUM_lazy_if(s+1,UTF))
		break;
	    if (strchr(":'{$", s[1]))
		break;
	    if (!PL_lex_inpat && (s[1] == '+' || s[1] == '-'))
		break; /* in regexp, neither @+ nor @- are interpolated */
	}

	/* check for embedded scalars.  only stop if we're sure it's a
	   variable.
        */
	else if (*s == '$') {
	    if (!PL_lex_inpat)	/* not a regexp, so $ must be var */
		break;
	    if (s + 1 < send && !strchr("()| \r\n\t", s[1])) {
		if (s[1] == '\\') {
		    Perl_ck_warner(aTHX_ packWARN(WARN_AMBIGUOUS),
				   "Possible unintended interpolation of $\\ in regex");
		}
		break;		/* in regexp, $ might be tail anchor */
            }
	}

	/* End of else if chain - OP_TRANS rejoin rest */

	/* backslashes */
	if (*s == '\\' && s+1 < send) {
	    char* e;	/* Can be used for ending '}', etc. */

	    s++;

	    /* warn on \1 - \9 in substitution replacements, but note that \11
	     * is an octal; and \19 is \1 followed by '9' */
	    if (PL_lex_inwhat == OP_SUBST && !PL_lex_inpat &&
		isDIGIT(*s) && *s != '0' && !isDIGIT(s[1]))
	    {
		Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX), "\\%c better written as $%c", *s, *s);
		*--s = '$';
		break;
	    }

	    /* string-change backslash escapes */
	    if (PL_lex_inwhat != OP_TRANS && *s && strchr("lLuUEQF", *s)) {
		--s;
		break;
	    }
	    /* In a pattern, process \N, but skip any other backslash escapes.
	     * This is because we don't want to translate an escape sequence
	     * into a meta symbol and have the regex compiler use the meta
	     * symbol meaning, e.g. \x{2E} would be confused with a dot.  But
	     * in spite of this, we do have to process \N here while the proper
	     * charnames handler is in scope.  See bugs #56444 and #62056.
	     * There is a complication because \N in a pattern may also stand
	     * for 'match a non-nl', and not mean a charname, in which case its
	     * processing should be deferred to the regex compiler.  To be a
	     * charname it must be followed immediately by a '{', and not look
	     * like \N followed by a curly quantifier, i.e., not something like
	     * \N{3,}.  regcurly returns a boolean indicating if it is a legal
	     * quantifier */
	    else if (PL_lex_inpat
		    && (*s != 'N'
			|| s[1] != '{'
			|| regcurly(s + 1)))
	    {
		*d++ = NATIVE_TO_NEED(has_utf8,'\\');
		goto default_action;
	    }

	    switch (*s) {

	    /* quoted - in transliterations */
	    case '-':
		if (PL_lex_inwhat == OP_TRANS) {
		    *d++ = *s++;
		    continue;
		}
		/* FALL THROUGH */
	    default:
	        {
		    if ((isALPHA(*s) || isDIGIT(*s)))
			Perl_ck_warner(aTHX_ packWARN(WARN_MISC),
				       "Unrecognized escape \\%c passed through",
				       *s);
		    /* default action is to copy the quoted character */
		    goto default_action;
		}

	    /* eg. \132 indicates the octal constant 0132 */
	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
		{
                    I32 flags = 0;
                    STRLEN len = 3;
		    uv = NATIVE_TO_UNI(grok_oct(s, &len, &flags, NULL));
		    s += len;
		}
		goto NUM_ESCAPE_INSERT;

	    /* eg. \o{24} indicates the octal constant \024 */
	    case 'o':
		{
		    STRLEN len;
		    const char* error;

		    bool valid = grok_bslash_o(s, &uv, &len, &error, 1);
		    s += len;
		    if (! valid) {
			yyerror(error);
			continue;
		    }
		    goto NUM_ESCAPE_INSERT;
		}

	    /* eg. \x24 indicates the hex constant 0x24 */
	    case 'x':
		++s;
		if (*s == '{') {
		    char* const e = strchr(s, '}');
                    I32 flags = PERL_SCAN_ALLOW_UNDERSCORES |
                      PERL_SCAN_DISALLOW_PREFIX;
		    STRLEN len;

                    ++s;
		    if (!e) {
			yyerror("Missing right brace on \\x{}");
			continue;
		    }
                    len = e - s;
		    uv = NATIVE_TO_UNI(grok_hex(s, &len, &flags, NULL));
		    s = e + 1;
		}
		else {
		    {
			STRLEN len = 2;
                        I32 flags = PERL_SCAN_DISALLOW_PREFIX;
			uv = NATIVE_TO_UNI(grok_hex(s, &len, &flags, NULL));
			s += len;
		    }
		}

	      NUM_ESCAPE_INSERT:
		/* Insert oct or hex escaped character.  There will always be
		 * enough room in sv since such escapes will be longer than any
		 * UTF-8 sequence they can end up as, except if they force us
		 * to recode the rest of the string into utf8 */
		
		/* Here uv is the ordinal of the next character being added in
		 * unicode (converted from native). */
		if (!UNI_IS_INVARIANT(uv)) {
		    if (!has_utf8 && uv > 255) {
			/* Might need to recode whatever we have accumulated so
			 * far if it contains any chars variant in utf8 or
			 * utf-ebcdic. */
			  
			SvCUR_set(sv, d - SvPVX_const(sv));
			SvPOK_on(sv);
			*d = '\0';
			/* See Note on sizing above.  */
			sv_utf8_upgrade_flags_grow(sv,
					SV_GMAGIC|SV_FORCE_UTF8_UPGRADE,
					UNISKIP(uv) + (STRLEN)(send - s) + 1);
			d = SvPVX(sv) + SvCUR(sv);
			has_utf8 = TRUE;
                    }

                    if (has_utf8) {
		        d = (char*)uvuni_to_utf8((U8*)d, uv);
			if (PL_lex_inwhat == OP_TRANS &&
			    PL_sublex_info.sub_op) {
			    PL_sublex_info.sub_op->op_private |=
				(PL_lex_repl ? OPpTRANS_FROM_UTF
					     : OPpTRANS_TO_UTF);
			}
#ifdef EBCDIC
			if (uv > 255 && !dorange)
			    native_range = FALSE;
#endif
                    }
		    else {
		        *d++ = (char)uv;
		    }
		}
		else {
		    *d++ = (char) uv;
		}
		continue;

 	    case 'N':
		/* In a non-pattern \N must be a named character, like \N{LATIN
		 * SMALL LETTER A} or \N{U+0041}.  For patterns, it also can
		 * mean to match a non-newline.  For non-patterns, named
		 * characters are converted to their string equivalents. In
		 * patterns, named characters are not converted to their
		 * ultimate forms for the same reasons that other escapes
		 * aren't.  Instead, they are converted to the \N{U+...} form
		 * to get the value from the charnames that is in effect right
		 * now, while preserving the fact that it was a named character
		 * so that the regex compiler knows this */

		/* This section of code doesn't generally use the
		 * NATIVE_TO_NEED() macro to transform the input.  I (khw) did
		 * a close examination of this macro and determined it is a
		 * no-op except on utfebcdic variant characters.  Every
		 * character generated by this that would normally need to be
		 * enclosed by this macro is invariant, so the macro is not
		 * needed, and would complicate use of copy().  XXX There are
		 * other parts of this file where the macro is used
		 * inconsistently, but are saved by it being a no-op */

		/* The structure of this section of code (besides checking for
		 * errors and upgrading to utf8) is:
		 *  Further disambiguate between the two meanings of \N, and if
		 *	not a charname, go process it elsewhere
		 *  If of form \N{U+...}, pass it through if a pattern;
		 *	otherwise convert to utf8
		 *  Otherwise must be \N{NAME}: convert to \N{U+c1.c2...} if a
		 *  pattern; otherwise convert to utf8 */

		/* Here, s points to the 'N'; the test below is guaranteed to
		 * succeed if we are being called on a pattern as we already
		 * know from a test above that the next character is a '{'.
		 * On a non-pattern \N must mean 'named sequence, which
		 * requires braces */
		s++;
		if (*s != '{') {
		    yyerror("Missing braces on \\N{}"); 
		    continue;
		}
		s++;

		/* If there is no matching '}', it is an error. */
		if (! (e = strchr(s, '}'))) {
		    if (! PL_lex_inpat) {
			yyerror("Missing right brace on \\N{}");
		    } else {
			yyerror("Missing right brace on \\N{} or unescaped left brace after \\N.");
		    }
		    continue;
		}

		/* Here it looks like a named character */

		if (PL_lex_inpat) {

		    /* XXX This block is temporary code.  \N{} implies that the
		     * pattern is to have Unicode semantics, and therefore
		     * currently has to be encoded in utf8.  By putting it in
		     * utf8 now, we save a whole pass in the regular expression
		     * compiler.  Once that code is changed so Unicode
		     * semantics doesn't necessarily have to be in utf8, this
		     * block should be removed.  However, the code that parses
		     * the output of this would have to be changed to not
		     * necessarily expect utf8 */
		    if (!has_utf8) {
			SvCUR_set(sv, d - SvPVX_const(sv));
			SvPOK_on(sv);
			*d = '\0';
			/* See Note on sizing above.  */
			sv_utf8_upgrade_flags_grow(sv,
					SV_GMAGIC|SV_FORCE_UTF8_UPGRADE,
					/* 5 = '\N{' + cur char + NUL */
					(STRLEN)(send - s) + 5);
			d = SvPVX(sv) + SvCUR(sv);
			has_utf8 = TRUE;
		    }
		}

		if (*s == 'U' && s[1] == '+') { /* \N{U+...} */
		    I32 flags = PERL_SCAN_ALLOW_UNDERSCORES
				| PERL_SCAN_DISALLOW_PREFIX;
		    STRLEN len;

		    /* For \N{U+...}, the '...' is a unicode value even on
		     * EBCDIC machines */
		    s += 2;	    /* Skip to next char after the 'U+' */
		    len = e - s;
		    uv = grok_hex(s, &len, &flags, NULL);
		    if (len == 0 || len != (STRLEN)(e - s)) {
			yyerror("Invalid hexadecimal number in \\N{U+...}");
			s = e + 1;
			continue;
		    }

		    if (PL_lex_inpat) {

			/* On non-EBCDIC platforms, pass through to the regex
			 * compiler unchanged.  The reason we evaluated the
			 * number above is to make sure there wasn't a syntax
			 * error.  But on EBCDIC we convert to native so
			 * downstream code can continue to assume it's native
			 */
			s -= 5;	    /* Include the '\N{U+' */
#ifdef EBCDIC
			d += my_snprintf(d, e - s + 1 + 1,  /* includes the }
							       and the \0 */
				    "\\N{U+%X}",
				    (unsigned int) UNI_TO_NATIVE(uv));
#else
			Copy(s, d, e - s + 1, char);	/* 1 = include the } */
			d += e - s + 1;
#endif
		    }
		    else {  /* Not a pattern: convert the hex to string */

			 /* If destination is not in utf8, unconditionally
			  * recode it to be so.  This is because \N{} implies
			  * Unicode semantics, and scalars have to be in utf8
			  * to guarantee those semantics */
			if (! has_utf8) {
			    SvCUR_set(sv, d - SvPVX_const(sv));
			    SvPOK_on(sv);
			    *d = '\0';
			    /* See Note on sizing above.  */
			    sv_utf8_upgrade_flags_grow(
					sv,
					SV_GMAGIC|SV_FORCE_UTF8_UPGRADE,
					UNISKIP(uv) + (STRLEN)(send - e) + 1);
			    d = SvPVX(sv) + SvCUR(sv);
			    has_utf8 = TRUE;
			}

			/* Add the string to the output */
			if (UNI_IS_INVARIANT(uv)) {
			    *d++ = (char) uv;
			}
			else d = (char*)uvuni_to_utf8((U8*)d, uv);
		    }
		}
		else { /* Here is \N{NAME} but not \N{U+...}. */

		    SV *res;		/* result from charnames */
		    const char *str;    /* the string in 'res' */
		    STRLEN len;		/* its length */

		    /* Get the value for NAME */
		    res = newSVpvn(s, e - s);
		    res = new_constant( NULL, 0, "charnames",
					/* includes all of: \N{...} */
					res, NULL, s - 3, e - s + 4 );

		    /* Most likely res will be in utf8 already since the
		     * standard charnames uses pack U, but a custom translator
		     * can leave it otherwise, so make sure.  XXX This can be
		     * revisited to not have charnames use utf8 for characters
		     * that don't need it when regexes don't have to be in utf8
		     * for Unicode semantics.  If doing so, remember EBCDIC */
		    sv_utf8_upgrade(res);
		    str = SvPV_const(res, len);

		    /* Don't accept malformed input */
		    if (! is_utf8_string((U8 *) str, len)) {
			yyerror("Malformed UTF-8 returned by \\N");
		    }
		    else if (PL_lex_inpat) {

			if (! len) { /* The name resolved to an empty string */
			    Copy("\\N{}", d, 4, char);
			    d += 4;
			}
			else {
			    /* In order to not lose information for the regex
			    * compiler, pass the result in the specially made
			    * syntax: \N{U+c1.c2.c3...}, where c1 etc. are
			    * the code points in hex of each character
			    * returned by charnames */

			    const char *str_end = str + len;
			    STRLEN char_length;	    /* cur char's byte length */
			    STRLEN output_length;   /* and the number of bytes
						       after this is translated
						       into hex digits */
			    const STRLEN off = d - SvPVX_const(sv);

			    /* 2 hex per byte; 2 chars for '\N'; 2 chars for
			     * max('U+', '.'); and 1 for NUL */
			    char hex_string[2 * UTF8_MAXBYTES + 5];

			    /* Get the first character of the result. */
			    U32 uv = utf8n_to_uvuni((U8 *) str,
						    len,
						    &char_length,
						    UTF8_ALLOW_ANYUV);

			    /* The call to is_utf8_string() above hopefully
			     * guarantees that there won't be an error.  But
			     * it's easy here to make sure.  The function just
			     * above warns and returns 0 if invalid utf8, but
			     * it can also return 0 if the input is validly a
			     * NUL. Disambiguate */
			    if (uv == 0 && NATIVE_TO_ASCII(*str) != '\0') {
				uv = UNICODE_REPLACEMENT;
			    }

			    /* Convert first code point to hex, including the
			     * boiler plate before it.  For all these, we
			     * convert to native format so that downstream code
			     * can continue to assume the input is native */
			    output_length =
				my_snprintf(hex_string, sizeof(hex_string),
					    "\\N{U+%X",
					    (unsigned int) UNI_TO_NATIVE(uv));

			    /* Make sure there is enough space to hold it */
			    d = off + SvGROW(sv, off
						 + output_length
						 + (STRLEN)(send - e)
						 + 2);	/* '}' + NUL */
			    /* And output it */
			    Copy(hex_string, d, output_length, char);
			    d += output_length;

			    /* For each subsequent character, append dot and
			     * its ordinal in hex */
			    while ((str += char_length) < str_end) {
				const STRLEN off = d - SvPVX_const(sv);
				U32 uv = utf8n_to_uvuni((U8 *) str,
							str_end - str,
							&char_length,
							UTF8_ALLOW_ANYUV);
				if (uv == 0 && NATIVE_TO_ASCII(*str) != '\0') {
				    uv = UNICODE_REPLACEMENT;
				}

				output_length =
				    my_snprintf(hex_string, sizeof(hex_string),
					    ".%X",
					    (unsigned int) UNI_TO_NATIVE(uv));

				d = off + SvGROW(sv, off
						     + output_length
						     + (STRLEN)(send - e)
						     + 2);	/* '}' +  NUL */
				Copy(hex_string, d, output_length, char);
				d += output_length;
			    }

			    *d++ = '}';	/* Done.  Add the trailing brace */
			}
		    }
		    else { /* Here, not in a pattern.  Convert the name to a
			    * string. */

			 /* If destination is not in utf8, unconditionally
			  * recode it to be so.  This is because \N{} implies
			  * Unicode semantics, and scalars have to be in utf8
			  * to guarantee those semantics */
			if (! has_utf8) {
			    SvCUR_set(sv, d - SvPVX_const(sv));
			    SvPOK_on(sv);
			    *d = '\0';
			    /* See Note on sizing above.  */
			    sv_utf8_upgrade_flags_grow(sv,
						SV_GMAGIC|SV_FORCE_UTF8_UPGRADE,
						len + (STRLEN)(send - s) + 1);
			    d = SvPVX(sv) + SvCUR(sv);
			    has_utf8 = TRUE;
			} else if (len > (STRLEN)(e - s + 4)) { /* I _guess_ 4 is \N{} --jhi */

			    /* See Note on sizing above.  (NOTE: SvCUR() is not
			     * set correctly here). */
			    const STRLEN off = d - SvPVX_const(sv);
			    d = off + SvGROW(sv, off + len + (STRLEN)(send - s) + 1);
			}
			Copy(str, d, len, char);
			d += len;
		    }
		    SvREFCNT_dec(res);

		    /* Deprecate non-approved name syntax */
		    if (ckWARN_d(WARN_DEPRECATED)) {
			bool problematic = FALSE;
			char* i = s;

			/* For non-ut8 input, look to see that the first
			 * character is an alpha, then loop through the rest
			 * checking that each is a continuation */
			if (! this_utf8) {
			    if (! isALPHAU(*i)) problematic = TRUE;
			    else for (i = s + 1; i < e; i++) {
				if (isCHARNAME_CONT(*i)) continue;
				problematic = TRUE;
				break;
			    }
			}
			else {
			    /* Similarly for utf8.  For invariants can check
			     * directly.  We accept anything above the latin1
			     * range because it is immaterial to Perl if it is
			     * correct or not, and is expensive to check.  But
			     * it is fairly easy in the latin1 range to convert
			     * the variants into a single character and check
			     * those */
			    if (UTF8_IS_INVARIANT(*i)) {
				if (! isALPHAU(*i)) problematic = TRUE;
			    } else if (UTF8_IS_DOWNGRADEABLE_START(*i)) {
				if (! isALPHAU(UNI_TO_NATIVE(TWO_BYTE_UTF8_TO_UNI(*i,
									    *(i+1)))))
				{
				    problematic = TRUE;
				}
			    }
			    if (! problematic) for (i = s + UTF8SKIP(s);
						    i < e;
						    i+= UTF8SKIP(i))
			    {
				if (UTF8_IS_INVARIANT(*i)) {
				    if (isCHARNAME_CONT(*i)) continue;
				} else if (! UTF8_IS_DOWNGRADEABLE_START(*i)) {
				    continue;
				} else if (isCHARNAME_CONT(
					    UNI_TO_NATIVE(
					    TWO_BYTE_UTF8_TO_UNI(*i, *(i+1)))))
				{
				    continue;
				}
				problematic = TRUE;
				break;
			    }
			}
			if (problematic) {
			    /* The e-i passed to the final %.*s makes sure that
			     * should the trailing NUL be missing that this
			     * print won't run off the end of the string */
			    Perl_warner(aTHX_ packWARN(WARN_DEPRECATED),
					"Deprecated character in \\N{...}; marked by <-- HERE  in \\N{%.*s<-- HERE %.*s",
					(int)(i - s + 1), s, (int)(e - i), i + 1);
			}
		    }
		} /* End \N{NAME} */
#ifdef EBCDIC
		if (!dorange) 
		    native_range = FALSE; /* \N{} is defined to be Unicode */
#endif
		s = e + 1;  /* Point to just after the '}' */
		continue;

	    /* \c is a control character */
	    case 'c':
		s++;
		if (s < send) {
		    *d++ = grok_bslash_c(*s++, has_utf8, 1);
		}
		else {
		    yyerror("Missing control char name in \\c");
		}
		continue;

	    /* printf-style backslashes, formfeeds, newlines, etc */
	    case 'b':
		*d++ = NATIVE_TO_NEED(has_utf8,'\b');
		break;
	    case 'n':
		*d++ = NATIVE_TO_NEED(has_utf8,'\n');
		break;
	    case 'r':
		*d++ = NATIVE_TO_NEED(has_utf8,'\r');
		break;
	    case 'f':
		*d++ = NATIVE_TO_NEED(has_utf8,'\f');
		break;
	    case 't':
		*d++ = NATIVE_TO_NEED(has_utf8,'\t');
		break;
	    case 'e':
		*d++ = ASCII_TO_NEED(has_utf8,'\033');
		break;
	    case 'a':
		*d++ = ASCII_TO_NEED(has_utf8,'\007');
		break;
	    } /* end switch */

	    s++;
	    continue;
	} /* end if (backslash) */
#ifdef EBCDIC
	else
	    literal_endpoint++;
#endif

    default_action:
	/* If we started with encoded form, or already know we want it,
	   then encode the next character */
	if (! NATIVE_IS_INVARIANT((U8)(*s)) && (this_utf8 || has_utf8)) {
	    STRLEN len  = 1;


	    /* One might think that it is wasted effort in the case of the
	     * source being utf8 (this_utf8 == TRUE) to take the next character
	     * in the source, convert it to an unsigned value, and then convert
	     * it back again.  But the source has not been validated here.  The
	     * routine that does the conversion checks for errors like
	     * malformed utf8 */

	    const UV nextuv   = (this_utf8) ? utf8n_to_uvchr((U8*)s, send - s, &len, 0) : (UV) ((U8) *s);
	    const STRLEN need = UNISKIP(NATIVE_TO_UNI(nextuv));
	    if (!has_utf8) {
		SvCUR_set(sv, d - SvPVX_const(sv));
		SvPOK_on(sv);
		*d = '\0';
		/* See Note on sizing above.  */
		sv_utf8_upgrade_flags_grow(sv,
					SV_GMAGIC|SV_FORCE_UTF8_UPGRADE,
					need + (STRLEN)(send - s) + 1);
		d = SvPVX(sv) + SvCUR(sv);
		has_utf8 = TRUE;
	    } else if (need > len) {
		/* encoded value larger than old, may need extra space (NOTE:
		 * SvCUR() is not set correctly here).   See Note on sizing
		 * above.  */
		const STRLEN off = d - SvPVX_const(sv);
		d = SvGROW(sv, off + need + (STRLEN)(send - s) + 1) + off;
	    }
	    s += len;

	    d = (char*)uvchr_to_utf8((U8*)d, nextuv);
#ifdef EBCDIC
	    if (uv > 255 && !dorange)
		native_range = FALSE;
#endif
	}
	else {
	    *d++ = NATIVE_TO_NEED(has_utf8,*s++);
	}
    } /* while loop to process each character */

    /* terminate the string and set up the sv */
    *d = '\0';
    SvCUR_set(sv, d - SvPVX_const(sv));
    if (SvCUR(sv) >= SvLEN(sv))
	Perl_croak(aTHX_ "panic: constant overflowed allocated space, %"UVuf
		   " >= %"UVuf, (UV)SvCUR(sv), (UV)SvLEN(sv));

    SvPOK_on(sv);
    if (PL_encoding && !has_utf8) {
	sv_recode_to_utf8(sv, PL_encoding);
	if (SvUTF8(sv))
	    has_utf8 = TRUE;
    }
    if (has_utf8) {
	SvUTF8_on(sv);
	if (PL_lex_inwhat == OP_TRANS && PL_sublex_info.sub_op) {
	    PL_sublex_info.sub_op->op_private |=
		    (PL_lex_repl ? OPpTRANS_FROM_UTF : OPpTRANS_TO_UTF);
	}
    }

    /* shrink the sv if we allocated more than we used */
    if (SvCUR(sv) + 5 < SvLEN(sv)) {
	SvPV_shrink_to_cur(sv);
    }

    /* return the substring (via pl_yylval) only if we parsed anything */
    if (s > PL_bufptr) {
	if ( PL_hints & ( PL_lex_inpat ? HINT_NEW_RE : HINT_NEW_STRING ) ) {
	    const char *const key = PL_lex_inpat ? "qr" : "q";
	    const STRLEN keylen = PL_lex_inpat ? 2 : 1;
	    const char *type;
	    STRLEN typelen;

	    if (PL_lex_inwhat == OP_TRANS) {
		type = "tr";
		typelen = 2;
	    } else if (PL_lex_inwhat == OP_SUBST && !PL_lex_inpat) {
		type = "s";
		typelen = 1;
	    } else  {
		type = "qq";
		typelen = 2;
	    }

	    sv = S_new_constant(aTHX_ start, s - start, key, keylen, sv, NULL,
				type, typelen);
	}
	pl_yylval.opval = (OP*)newSVOP(OP_CONST, 0, sv);
    } else
	SvREFCNT_dec(sv);
    return s;
}

/* S_intuit_more
 * Returns TRUE if there's more to the expression (e.g., a subscript),
 * FALSE otherwise.
 *
 * It deals with "$foo[3]" and /$foo[3]/ and /$foo[0123456789$]+/
 *
 * ->[ and ->{ return TRUE
 * { and [ outside a pattern are always subscripts, so return TRUE
 * if we're outside a pattern and it's not { or [, then return FALSE
 * if we're in a pattern and the first char is a {
 *   {4,5} (any digits around the comma) returns FALSE
 * if we're in a pattern and the first char is a [
 *   [] returns FALSE
 *   [SOMETHING] has a funky algorithm to decide whether it's a
 *      character class or not.  It has to deal with things like
 *      /$foo[-3]/ and /$foo[$bar]/ as well as /$foo[$\d]+/
 * anything else returns TRUE
 */

/* This is the one truly awful dwimmer necessary to conflate C and sed. */

STATIC int
S_intuit_more(pTHX_ register char *s)
{
    dVAR;

    PERL_ARGS_ASSERT_INTUIT_MORE;

    if (PL_lex_brackets)
	return TRUE;
    if (*s == '-' && s[1] == '>' && (s[2] == '[' || s[2] == '{'))
	return TRUE;
    if (*s != '{' && *s != '[')
	return FALSE;
    if (!PL_lex_inpat)
	return TRUE;

    /* In a pattern, so maybe we have {n,m}. */
    if (*s == '{') {
	if (regcurly(s)) {
	    return FALSE;
	}
	return TRUE;
    }

    /* On the other hand, maybe we have a character class */

    s++;
    if (*s == ']' || *s == '^')
	return FALSE;
    else {
        /* this is terrifying, and it works */
	int weight = 2;		/* let's weigh the evidence */
	char seen[256];
	unsigned char un_char = 255, last_un_char;
	const char * const send = strchr(s,']');
	char tmpbuf[sizeof PL_tokenbuf * 4];

	if (!send)		/* has to be an expression */
	    return TRUE;

	Zero(seen,256,char);
	if (*s == '$')
	    weight -= 3;
	else if (isDIGIT(*s)) {
	    if (s[1] != ']') {
		if (isDIGIT(s[1]) && s[2] == ']')
		    weight -= 10;
	    }
	    else
		weight -= 100;
	}
	for (; s < send; s++) {
	    last_un_char = un_char;
	    un_char = (unsigned char)*s;
	    switch (*s) {
	    case '@':
	    case '&':
	    case '$':
		weight -= seen[un_char] * 10;
		if (isALNUM_lazy_if(s+1,UTF)) {
		    int len;
		    scan_ident(s, send, tmpbuf, sizeof tmpbuf, FALSE);
		    len = (int)strlen(tmpbuf);
		    if (len > 1 && gv_fetchpvn_flags(tmpbuf, len,
                                                    UTF ? SVf_UTF8 : 0, SVt_PV))
			weight -= 100;
		    else
			weight -= 10;
		}
		else if (*s == '$' && s[1] &&
		  strchr("[#!%*<>()-=",s[1])) {
		    if (/*{*/ strchr("])} =",s[2]))
			weight -= 10;
		    else
			weight -= 1;
		}
		break;
	    case '\\':
		un_char = 254;
		if (s[1]) {
		    if (strchr("wds]",s[1]))
			weight += 100;
		    else if (seen[(U8)'\''] || seen[(U8)'"'])
			weight += 1;
		    else if (strchr("rnftbxcav",s[1]))
			weight += 40;
		    else if (isDIGIT(s[1])) {
			weight += 40;
			while (s[1] && isDIGIT(s[1]))
			    s++;
		    }
		}
		else
		    weight += 100;
		break;
	    case '-':
		if (s[1] == '\\')
		    weight += 50;
		if (strchr("aA01! ",last_un_char))
		    weight += 30;
		if (strchr("zZ79~",s[1]))
		    weight += 30;
		if (last_un_char == 255 && (isDIGIT(s[1]) || s[1] == '$'))
		    weight -= 5;	/* cope with negative subscript */
		break;
	    default:
		if (!isALNUM(last_un_char)
		    && !(last_un_char == '$' || last_un_char == '@'
			 || last_un_char == '&')
		    && isALPHA(*s) && s[1] && isALPHA(s[1])) {
		    char *d = tmpbuf;
		    while (isALPHA(*s))
			*d++ = *s++;
		    *d = '\0';
		    if (keyword(tmpbuf, d - tmpbuf, 0))
			weight -= 150;
		}
		if (un_char == last_un_char + 1)
		    weight += 5;
		weight -= seen[un_char];
		break;
	    }
	    seen[un_char]++;
	}
	if (weight >= 0)	/* probably a character class */
	    return FALSE;
    }

    return TRUE;
}

/*
 * S_intuit_method
 *
 * Does all the checking to disambiguate
 *   foo bar
 * between foo(bar) and bar->foo.  Returns 0 if not a method, otherwise
 * FUNCMETH (bar->foo(args)) or METHOD (bar->foo args).
 *
 * First argument is the stuff after the first token, e.g. "bar".
 *
 * Not a method if bar is a filehandle.
 * Not a method if foo is a subroutine prototyped to take a filehandle.
 * Not a method if it's really "Foo $bar"
 * Method if it's "foo $bar"
 * Not a method if it's really "print foo $bar"
 * Method if it's really "foo package::" (interpreted as package->foo)
 * Not a method if bar is known to be a subroutine ("sub bar; foo bar")
 * Not a method if bar is a filehandle or package, but is quoted with
 *   =>
 */

STATIC int
S_intuit_method(pTHX_ char *start, GV *gv, CV *cv)
{
    dVAR;
    char *s = start + (*start == '$');
    char tmpbuf[sizeof PL_tokenbuf];
    STRLEN len;
    GV* indirgv;
#ifdef PERL_MAD
    int soff;
#endif

    PERL_ARGS_ASSERT_INTUIT_METHOD;

    if (gv) {
	if (SvTYPE(gv) == SVt_PVGV && GvIO(gv))
	    return 0;
	if (cv) {
	    if (SvPOK(cv)) {
		const char *proto = CvPROTO(cv);
		if (proto) {
		    if (*proto == ';')
			proto++;
		    if (*proto == '*')
			return 0;
		}
	    }
	} else
	    gv = NULL;
    }
    s = scan_word(s, tmpbuf, sizeof tmpbuf, TRUE, &len);
    /* start is the beginning of the possible filehandle/object,
     * and s is the end of it
     * tmpbuf is a copy of it
     */

    if (*start == '$') {
	if (gv || PL_last_lop_op == OP_PRINT || PL_last_lop_op == OP_SAY ||
		isUPPER(*PL_tokenbuf))
	    return 0;
#ifdef PERL_MAD
	len = start - SvPVX(PL_linestr);
#endif
	s = PEEKSPACE(s);
#ifdef PERL_MAD
	start = SvPVX(PL_linestr) + len;
#endif
	PL_bufptr = start;
	PL_expect = XREF;
	return *s == '(' ? FUNCMETH : METHOD;
    }
    if (!keyword(tmpbuf, len, 0)) {
	if (len > 2 && tmpbuf[len - 2] == ':' && tmpbuf[len - 1] == ':') {
	    len -= 2;
	    tmpbuf[len] = '\0';
#ifdef PERL_MAD
	    soff = s - SvPVX(PL_linestr);
#endif
	    goto bare_package;
	}
	indirgv = gv_fetchpvn_flags(tmpbuf, len, ( UTF ? SVf_UTF8 : 0 ), SVt_PVCV);
	if (indirgv && GvCVu(indirgv))
	    return 0;
	/* filehandle or package name makes it a method */
	if (!gv || GvIO(indirgv) || gv_stashpvn(tmpbuf, len, UTF ? SVf_UTF8 : 0)) {
#ifdef PERL_MAD
	    soff = s - SvPVX(PL_linestr);
#endif
	    s = PEEKSPACE(s);
	    if ((PL_bufend - s) >= 2 && *s == '=' && *(s+1) == '>')
		return 0;	/* no assumptions -- "=>" quotes bareword */
      bare_package:
	    start_force(PL_curforce);
	    NEXTVAL_NEXTTOKE.opval = (OP*)newSVOP(OP_CONST, 0,
						  S_newSV_maybe_utf8(aTHX_ tmpbuf, len));
	    NEXTVAL_NEXTTOKE.opval->op_private = OPpCONST_BARE;
	    if (PL_madskills)
		curmad('X', newSVpvn_flags(start,SvPVX(PL_linestr) + soff - start,
                                                            ( UTF ? SVf_UTF8 : 0 )));
	    PL_expect = XTERM;
	    force_next(WORD);
	    PL_bufptr = s;
#ifdef PERL_MAD
	    PL_bufptr = SvPVX(PL_linestr) + soff; /* restart before space */
#endif
	    return *s == '(' ? FUNCMETH : METHOD;
	}
    }
    return 0;
}

/* Encoded script support. filter_add() effectively inserts a
 * 'pre-processing' function into the current source input stream.
 * Note that the filter function only applies to the current source file
 * (e.g., it will not affect files 'require'd or 'use'd by this one).
 *
 * The datasv parameter (which may be NULL) can be used to pass
 * private data to this instance of the filter. The filter function
 * can recover the SV using the FILTER_DATA macro and use it to
 * store private buffers and state information.
 *
 * The supplied datasv parameter is upgraded to a PVIO type
 * and the IoDIRP/IoANY field is used to store the function pointer,
 * and IOf_FAKE_DIRP is enabled on datasv to mark this as such.
 * Note that IoTOP_NAME, IoFMT_NAME, IoBOTTOM_NAME, if set for
 * private use must be set using malloc'd pointers.
 */

SV *
Perl_filter_add(pTHX_ filter_t funcp, SV *datasv)
{
    dVAR;
    if (!funcp)
	return NULL;

    if (!PL_parser)
	return NULL;

    if (PL_parser->lex_flags & LEX_IGNORE_UTF8_HINTS)
	Perl_croak(aTHX_ "Source filters apply only to byte streams");

    if (!PL_rsfp_filters)
	PL_rsfp_filters = newAV();
    if (!datasv)
	datasv = newSV(0);
    SvUPGRADE(datasv, SVt_PVIO);
    IoANY(datasv) = FPTR2DPTR(void *, funcp); /* stash funcp into spare field */
    IoFLAGS(datasv) |= IOf_FAKE_DIRP;
    DEBUG_P(PerlIO_printf(Perl_debug_log, "filter_add func %p (%s)\n",
			  FPTR2DPTR(void *, IoANY(datasv)),
			  SvPV_nolen(datasv)));
    av_unshift(PL_rsfp_filters, 1);
    av_store(PL_rsfp_filters, 0, datasv) ;
    if (
	!PL_parser->filtered
     && PL_parser->lex_flags & LEX_EVALBYTES
     && PL_bufptr < PL_bufend
    ) {
	const char *s = PL_bufptr;
	while (s < PL_bufend) {
	    if (*s == '\n') {
		SV *linestr = PL_parser->linestr;
		char *buf = SvPVX(linestr);
		STRLEN const bufptr_pos = PL_parser->bufptr - buf;
		STRLEN const oldbufptr_pos = PL_parser->oldbufptr - buf;
		STRLEN const oldoldbufptr_pos=PL_parser->oldoldbufptr-buf;
		STRLEN const linestart_pos = PL_parser->linestart - buf;
		STRLEN const last_uni_pos =
		    PL_parser->last_uni ? PL_parser->last_uni - buf : 0;
		STRLEN const last_lop_pos =
		    PL_parser->last_lop ? PL_parser->last_lop - buf : 0;
		av_push(PL_rsfp_filters, linestr);
		PL_parser->linestr = 
		    newSVpvn(SvPVX(linestr), ++s-SvPVX(linestr));
		buf = SvPVX(PL_parser->linestr);
		PL_parser->bufend = buf + SvCUR(PL_parser->linestr);
		PL_parser->bufptr = buf + bufptr_pos;
		PL_parser->oldbufptr = buf + oldbufptr_pos;
		PL_parser->oldoldbufptr = buf + oldoldbufptr_pos;
		PL_parser->linestart = buf + linestart_pos;
		if (PL_parser->last_uni)
		    PL_parser->last_uni = buf + last_uni_pos;
		if (PL_parser->last_lop)
		    PL_parser->last_lop = buf + last_lop_pos;
		SvLEN(linestr) = SvCUR(linestr);
		SvCUR(linestr) = s-SvPVX(linestr);
		PL_parser->filtered = 1;
		break;
	    }
	    s++;
	}
    }
    return(datasv);
}


/* Delete most recently added instance of this filter function.	*/
void
Perl_filter_del(pTHX_ filter_t funcp)
{
    dVAR;
    SV *datasv;

    PERL_ARGS_ASSERT_FILTER_DEL;

#ifdef DEBUGGING
    DEBUG_P(PerlIO_printf(Perl_debug_log, "filter_del func %p",
			  FPTR2DPTR(void*, funcp)));
#endif
    if (!PL_parser || !PL_rsfp_filters || AvFILLp(PL_rsfp_filters)<0)
	return;
    /* if filter is on top of stack (usual case) just pop it off */
    datasv = FILTER_DATA(AvFILLp(PL_rsfp_filters));
    if (IoANY(datasv) == FPTR2DPTR(void *, funcp)) {
	sv_free(av_pop(PL_rsfp_filters));

        return;
    }
    /* we need to search for the correct entry and clear it	*/
    Perl_die(aTHX_ "filter_del can only delete in reverse order (currently)");
}


/* Invoke the idxth filter function for the current rsfp.	 */
/* maxlen 0 = read one text line */
I32
Perl_filter_read(pTHX_ int idx, SV *buf_sv, int maxlen)
{
    dVAR;
    filter_t funcp;
    SV *datasv = NULL;
    /* This API is bad. It should have been using unsigned int for maxlen.
       Not sure if we want to change the API, but if not we should sanity
       check the value here.  */
    unsigned int correct_length
	= maxlen < 0 ?
#ifdef PERL_MICRO
	0x7FFFFFFF
#else
	INT_MAX
#endif
	: maxlen;

    PERL_ARGS_ASSERT_FILTER_READ;

    if (!PL_parser || !PL_rsfp_filters)
	return -1;
    if (idx > AvFILLp(PL_rsfp_filters)) {       /* Any more filters?	*/
	/* Provide a default input filter to make life easy.	*/
	/* Note that we append to the line. This is handy.	*/
	DEBUG_P(PerlIO_printf(Perl_debug_log,
			      "filter_read %d: from rsfp\n", idx));
	if (correct_length) {
 	    /* Want a block */
	    int len ;
	    const int old_len = SvCUR(buf_sv);

	    /* ensure buf_sv is large enough */
	    SvGROW(buf_sv, (STRLEN)(old_len + correct_length + 1)) ;
	    if ((len = PerlIO_read(PL_rsfp, SvPVX(buf_sv) + old_len,
				   correct_length)) <= 0) {
		if (PerlIO_error(PL_rsfp))
	            return -1;		/* error */
	        else
		    return 0 ;		/* end of file */
	    }
	    SvCUR_set(buf_sv, old_len + len) ;
	    SvPVX(buf_sv)[old_len + len] = '\0';
	} else {
	    /* Want a line */
            if (sv_gets(buf_sv, PL_rsfp, SvCUR(buf_sv)) == NULL) {
		if (PerlIO_error(PL_rsfp))
	            return -1;		/* error */
	        else
		    return 0 ;		/* end of file */
	    }
	}
	return SvCUR(buf_sv);
    }
    /* Skip this filter slot if filter has been deleted	*/
    if ( (datasv = FILTER_DATA(idx)) == &PL_sv_undef) {
	DEBUG_P(PerlIO_printf(Perl_debug_log,
			      "filter_read %d: skipped (filter deleted)\n",
			      idx));
	return FILTER_READ(idx+1, buf_sv, correct_length); /* recurse */
    }
    if (SvTYPE(datasv) != SVt_PVIO) {
	if (correct_length) {
 	    /* Want a block */
	    const STRLEN remainder = SvLEN(datasv) - SvCUR(datasv);
	    if (!remainder) return 0; /* eof */
	    if (correct_length > remainder) correct_length = remainder;
	    sv_catpvn(buf_sv, SvEND(datasv), correct_length);
	    SvCUR_set(datasv, SvCUR(datasv) + correct_length);
	} else {
	    /* Want a line */
	    const char *s = SvEND(datasv);
	    const char *send = SvPVX(datasv) + SvLEN(datasv);
	    while (s < send) {
		if (*s == '\n') {
		    s++;
		    break;
		}
		s++;
	    }
	    if (s == send) return 0; /* eof */
	    sv_catpvn(buf_sv, SvEND(datasv), s-SvEND(datasv));
	    SvCUR_set(datasv, s-SvPVX(datasv));
	}
	return SvCUR(buf_sv);
    }
    /* Get function pointer hidden within datasv	*/
    funcp = DPTR2FPTR(filter_t, IoANY(datasv));
    DEBUG_P(PerlIO_printf(Perl_debug_log,
			  "filter_read %d: via function %p (%s)\n",
			  idx, (void*)datasv, SvPV_nolen_const(datasv)));
    /* Call function. The function is expected to 	*/
    /* call "FILTER_READ(idx+1, buf_sv)" first.		*/
    /* Return: <0:error, =0:eof, >0:not eof 		*/
    return (*funcp)(aTHX_ idx, buf_sv, correct_length);
}

STATIC char *
S_filter_gets(pTHX_ register SV *sv, STRLEN append)
{
    dVAR;

    PERL_ARGS_ASSERT_FILTER_GETS;

#ifdef PERL_CR_FILTER
    if (!PL_rsfp_filters) {
	filter_add(S_cr_textfilter,NULL);
    }
#endif
    if (PL_rsfp_filters) {
	if (!append)
            SvCUR_set(sv, 0);	/* start with empty line	*/
        if (FILTER_READ(0, sv, 0) > 0)
            return ( SvPVX(sv) ) ;
        else
	    return NULL ;
    }
    else
        return (sv_gets(sv, PL_rsfp, append));
}

STATIC HV *
S_find_in_my_stash(pTHX_ const char *pkgname, STRLEN len)
{
    dVAR;
    GV *gv;

    PERL_ARGS_ASSERT_FIND_IN_MY_STASH;

    if (len == 11 && *pkgname == '_' && strEQ(pkgname, "__PACKAGE__"))
        return PL_curstash;

    if (len > 2 &&
        (pkgname[len - 2] == ':' && pkgname[len - 1] == ':') &&
        (gv = gv_fetchpvn_flags(pkgname, len, ( UTF ? SVf_UTF8 : 0 ), SVt_PVHV)))
    {
        return GvHV(gv);			/* Foo:: */
    }

    /* use constant CLASS => 'MyClass' */
    gv = gv_fetchpvn_flags(pkgname, len, UTF ? SVf_UTF8 : 0, SVt_PVCV);
    if (gv && GvCV(gv)) {
	SV * const sv = cv_const_sv(GvCV(gv));
	if (sv)
            pkgname = SvPV_const(sv, len);
    }

    return gv_stashpvn(pkgname, len, UTF ? SVf_UTF8 : 0);
}

/*
 * S_readpipe_override
 * Check whether readpipe() is overridden, and generates the appropriate
 * optree, provided sublex_start() is called afterwards.
 */
STATIC void
S_readpipe_override(pTHX)
{
    GV **gvp;
    GV *gv_readpipe = gv_fetchpvs("readpipe", GV_NOTQUAL, SVt_PVCV);
    pl_yylval.ival = OP_BACKTICK;
    if ((gv_readpipe
		&& GvCVu(gv_readpipe) && GvIMPORTED_CV(gv_readpipe))
	    ||
	    ((gvp = (GV**)hv_fetchs(PL_globalstash, "readpipe", FALSE))
	     && (gv_readpipe = *gvp) && isGV_with_GP(gv_readpipe)
	     && GvCVu(gv_readpipe) && GvIMPORTED_CV(gv_readpipe)))
    {
	PL_lex_op = (OP*)newUNOP(OP_ENTERSUB, OPf_STACKED,
	    op_append_elem(OP_LIST,
		newSVOP(OP_CONST, 0, &PL_sv_undef), /* value will be read later */
		newCVREF(0, newGVOP(OP_GV, 0, gv_readpipe))));
    }
}

#ifdef PERL_MAD 
 /*
 * Perl_madlex
 * The intent of this yylex wrapper is to minimize the changes to the
 * tokener when we aren't interested in collecting madprops.  It remains
 * to be seen how successful this strategy will be...
 */

int
Perl_madlex(pTHX)
{
    int optype;
    char *s = PL_bufptr;

    /* make sure PL_thiswhite is initialized */
    PL_thiswhite = 0;
    PL_thismad = 0;

    /* just do what yylex would do on pending identifier; leave PL_thiswhite alone */
    if (PL_lex_state != LEX_KNOWNEXT && PL_pending_ident)
        return S_pending_ident(aTHX);

    /* previous token ate up our whitespace? */
    if (!PL_lasttoke && PL_nextwhite) {
	PL_thiswhite = PL_nextwhite;
	PL_nextwhite = 0;
    }

    /* isolate the token, and figure out where it is without whitespace */
    PL_realtokenstart = -1;
    PL_thistoken = 0;
    optype = yylex();
    s = PL_bufptr;
    assert(PL_curforce < 0);

    if (!PL_thismad || PL_thismad->mad_key == '^') {	/* not forced already? */
	if (!PL_thistoken) {
	    if (PL_realtokenstart < 0 || !CopLINE(PL_curcop))
		PL_thistoken = newSVpvs("");
	    else {
		char * const tstart = SvPVX(PL_linestr) + PL_realtokenstart;
		PL_thistoken = newSVpvn(tstart, s - tstart);
	    }
	}
	if (PL_thismad)	/* install head */
	    CURMAD('X', PL_thistoken);
    }

    /* last whitespace of a sublex? */
    if (optype == ')' && PL_endwhite) {
	CURMAD('X', PL_endwhite);
    }

    if (!PL_thismad) {

	/* if no whitespace and we're at EOF, bail.  Otherwise fake EOF below. */
	if (!PL_thiswhite && !PL_endwhite && !optype) {
	    sv_free(PL_thistoken);
	    PL_thistoken = 0;
	    return 0;
	}

	/* put off final whitespace till peg */
	if (optype == ';' && !PL_rsfp && !PL_parser->filtered) {
	    PL_nextwhite = PL_thiswhite;
	    PL_thiswhite = 0;
	}
	else if (PL_thisopen) {
	    CURMAD('q', PL_thisopen);
	    if (PL_thistoken)
		sv_free(PL_thistoken);
	    PL_thistoken = 0;
	}
	else {
	    /* Store actual token text as madprop X */
	    CURMAD('X', PL_thistoken);
	}

	if (PL_thiswhite) {
	    /* add preceding whitespace as madprop _ */
	    CURMAD('_', PL_thiswhite);
	}

	if (PL_thisstuff) {
	    /* add quoted material as madprop = */
	    CURMAD('=', PL_thisstuff);
	}

	if (PL_thisclose) {
	    /* add terminating quote as madprop Q */
	    CURMAD('Q', PL_thisclose);
	}
    }

    /* special processing based on optype */

    switch (optype) {

    /* opval doesn't need a TOKEN since it can already store mp */
    case WORD:
    case METHOD:
    case FUNCMETH:
    case THING:
    case PMFUNC:
    case PRIVATEREF:
    case FUNC0SUB:
    case UNIOPSUB:
    case LSTOPSUB:
    case LABEL:
	if (pl_yylval.opval)
	    append_madprops(PL_thismad, pl_yylval.opval, 0);
	PL_thismad = 0;
	return optype;

    /* fake EOF */
    case 0:
	optype = PEG;
	if (PL_endwhite) {
	    addmad(newMADsv('p', PL_endwhite), &PL_thismad, 0);
	    PL_endwhite = 0;
	}
	break;

    case ']':
    case '}':
	if (PL_faketokens)
	    break;
	/* remember any fake bracket that lexer is about to discard */ 
	if (PL_lex_brackets == 1 &&
	    ((expectation)PL_lex_brackstack[0] & XFAKEBRACK))
	{
	    s = PL_bufptr;
	    while (s < PL_bufend && (*s == ' ' || *s == '\t'))
		s++;
	    if (*s == '}') {
		PL_thiswhite = newSVpvn(PL_bufptr, ++s - PL_bufptr);
		addmad(newMADsv('#', PL_thiswhite), &PL_thismad, 0);
		PL_thiswhite = 0;
		PL_bufptr = s - 1;
		break;	/* don't bother looking for trailing comment */
	    }
	    else
		s = PL_bufptr;
	}
	if (optype == ']')
	    break;
	/* FALLTHROUGH */

    /* attach a trailing comment to its statement instead of next token */
    case ';':
	if (PL_faketokens)
	    break;
	if (PL_bufptr > PL_oldbufptr && PL_bufptr[-1] == optype) {
	    s = PL_bufptr;
	    while (s < PL_bufend && (*s == ' ' || *s == '\t'))
		s++;
	    if (*s == '\n' || *s == '#') {
		while (s < PL_bufend && *s != '\n')
		    s++;
		if (s < PL_bufend)
		    s++;
		PL_thiswhite = newSVpvn(PL_bufptr, s - PL_bufptr);
		addmad(newMADsv('#', PL_thiswhite), &PL_thismad, 0);
		PL_thiswhite = 0;
		PL_bufptr = s;
	    }
	}
	break;

    /* ival */
    default:
	break;

    }

    /* Create new token struct.  Note: opvals return early above. */
    pl_yylval.tkval = newTOKEN(optype, pl_yylval, PL_thismad);
    PL_thismad = 0;
    return optype;
}
#endif

STATIC char *
S_tokenize_use(pTHX_ int is_use, char *s) {
    dVAR;

    PERL_ARGS_ASSERT_TOKENIZE_USE;

    if (PL_expect != XSTATE)
	yyerror(Perl_form(aTHX_ "\"%s\" not allowed in expression",
		    is_use ? "use" : "no"));
    s = SKIPSPACE1(s);
    if (isDIGIT(*s) || (*s == 'v' && isDIGIT(s[1]))) {
	s = force_version(s, TRUE);
	if (*s == ';' || *s == '}'
		|| (s = SKIPSPACE1(s), (*s == ';' || *s == '}'))) {
	    start_force(PL_curforce);
	    NEXTVAL_NEXTTOKE.opval = NULL;
	    force_next(WORD);
	}
	else if (*s == 'v') {
	    s = force_word(s,WORD,FALSE,TRUE,FALSE);
	    s = force_version(s, FALSE);
	}
    }
    else {
	s = force_word(s,WORD,FALSE,TRUE,FALSE);
	s = force_version(s, FALSE);
    }
    pl_yylval.ival = is_use;
    return s;
}
#ifdef DEBUGGING
    static const char* const exp_name[] =
	{ "OPERATOR", "TERM", "REF", "STATE", "BLOCK", "ATTRBLOCK",
	  "ATTRTERM", "TERMBLOCK", "TERMORDORDOR"
	};
#endif

#define word_takes_any_delimeter(p,l) S_word_takes_any_delimeter(p,l)
STATIC bool
S_word_takes_any_delimeter(char *p, STRLEN len)
{
    return (len == 1 && strchr("msyq", p[0])) ||
	   (len == 2 && (
	    (p[0] == 't' && p[1] == 'r') ||
	    (p[0] == 'q' && strchr("qwxr", p[1]))));
}

/*
  yylex

  Works out what to call the token just pulled out of the input
  stream.  The yacc parser takes care of taking the ops we return and
  stitching them into a tree.

  Returns:
    PRIVATEREF

  Structure:
      if read an identifier
          if we're in a my declaration
	      croak if they tried to say my($foo::bar)
	      build the ops for a my() declaration
	  if it's an access to a my() variable
	      are we in a sort block?
	          croak if my($a); $a <=> $b
	      build ops for access to a my() variable
	  if in a dq string, and they've said @foo and we can't find @foo
	      croak
	  build ops for a bareword
      if we already built the token before, use it.
*/


#ifdef __SC__
#pragma segment Perl_yylex
#endif
int
Perl_yylex(pTHX)
{
    dVAR;
    register char *s = PL_bufptr;
    register char *d;
    STRLEN len;
    bool bof = FALSE;
    U32 fake_eof = 0;

    /* orig_keyword, gvp, and gv are initialized here because
     * jump to the label just_a_word_zero can bypass their
     * initialization later. */
    I32 orig_keyword = 0;
    GV *gv = NULL;
    GV **gvp = NULL;

    DEBUG_T( {
	SV* tmp = newSVpvs("");
	PerlIO_printf(Perl_debug_log, "### %"IVdf":LEX_%s/X%s %s\n",
	    (IV)CopLINE(PL_curcop),
	    lex_state_names[PL_lex_state],
	    exp_name[PL_expect],
	    pv_display(tmp, s, strlen(s), 0, 60));
	SvREFCNT_dec(tmp);
    } );
    /* check if there's an identifier for us to look at */
    if (PL_lex_state != LEX_KNOWNEXT && PL_pending_ident)
        return REPORT(S_pending_ident(aTHX));

    /* no identifier pending identification */

    switch (PL_lex_state) {
#ifdef COMMENTARY
    case LEX_NORMAL:		/* Some compilers will produce faster */
    case LEX_INTERPNORMAL:	/* code if we comment these out. */
	break;
#endif

    /* when we've already built the next token, just pull it out of the queue */
    case LEX_KNOWNEXT:
#ifdef PERL_MAD
	PL_lasttoke--;
	pl_yylval = PL_nexttoke[PL_lasttoke].next_val;
	if (PL_madskills) {
	    PL_thismad = PL_nexttoke[PL_lasttoke].next_mad;
	    PL_nexttoke[PL_lasttoke].next_mad = 0;
	    if (PL_thismad && PL_thismad->mad_key == '_') {
		PL_thiswhite = MUTABLE_SV(PL_thismad->mad_val);
		PL_thismad->mad_val = 0;
		mad_free(PL_thismad);
		PL_thismad = 0;
	    }
	}
	if (!PL_lasttoke) {
	    PL_lex_state = PL_lex_defer;
  	    PL_expect = PL_lex_expect;
  	    PL_lex_defer = LEX_NORMAL;
	    if (!PL_nexttoke[PL_lasttoke].next_type)
		return yylex();
  	}
#else
	PL_nexttoke--;
	pl_yylval = PL_nextval[PL_nexttoke];
	if (!PL_nexttoke) {
	    PL_lex_state = PL_lex_defer;
	    PL_expect = PL_lex_expect;
	    PL_lex_defer = LEX_NORMAL;
	}
#endif
	{
	    I32 next_type;
#ifdef PERL_MAD
	    next_type = PL_nexttoke[PL_lasttoke].next_type;
#else
	    next_type = PL_nexttype[PL_nexttoke];
#endif
	    if (next_type & (7<<24)) {
		if (next_type & (1<<24)) {
		    if (PL_lex_brackets > 100)
			Renew(PL_lex_brackstack, PL_lex_brackets + 10, char);
		    PL_lex_brackstack[PL_lex_brackets++] =
			(char) ((next_type >> 16) & 0xff);
		}
		if (next_type & (2<<24))
		    PL_lex_allbrackets++;
		if (next_type & (4<<24))
		    PL_lex_allbrackets--;
		next_type &= 0xffff;
	    }
#ifdef PERL_MAD
	    /* FIXME - can these be merged?  */
	    return next_type;
#else
	    return REPORT(next_type);
#endif
	}

    /* interpolated case modifiers like \L \U, including \Q and \E.
       when we get here, PL_bufptr is at the \
    */
    case LEX_INTERPCASEMOD:
#ifdef DEBUGGING
	if (PL_bufptr != PL_bufend && *PL_bufptr != '\\')
	    Perl_croak(aTHX_
		       "panic: INTERPCASEMOD bufptr=%p, bufend=%p, *bufptr=%u",
		       PL_bufptr, PL_bufend, *PL_bufptr);
#endif
	/* handle \E or end of string */
       	if (PL_bufptr == PL_bufend || PL_bufptr[1] == 'E') {
	    /* if at a \E */
	    if (PL_lex_casemods) {
		const char oldmod = PL_lex_casestack[--PL_lex_casemods];
		PL_lex_casestack[PL_lex_casemods] = '\0';

		if (PL_bufptr != PL_bufend
		    && (oldmod == 'L' || oldmod == 'U' || oldmod == 'Q'
                        || oldmod == 'F')) {
		    PL_bufptr += 2;
		    PL_lex_state = LEX_INTERPCONCAT;
#ifdef PERL_MAD
		    if (PL_madskills)
			PL_thistoken = newSVpvs("\\E");
#endif
		}
		PL_lex_allbrackets--;
		return REPORT(')');
	    }
            else if ( PL_bufptr != PL_bufend && PL_bufptr[1] == 'E' ) {
               /* Got an unpaired \E */
               Perl_ck_warner(aTHX_ packWARN(WARN_MISC),
                        "Useless use of \\E");
            }
#ifdef PERL_MAD
	    while (PL_bufptr != PL_bufend &&
	      PL_bufptr[0] == '\\' && PL_bufptr[1] == 'E') {
		if (!PL_thiswhite)
		    PL_thiswhite = newSVpvs("");
		sv_catpvn(PL_thiswhite, PL_bufptr, 2);
		PL_bufptr += 2;
	    }
#else
	    if (PL_bufptr != PL_bufend)
		PL_bufptr += 2;
#endif
	    PL_lex_state = LEX_INTERPCONCAT;
	    return yylex();
	}
	else {
	    DEBUG_T({ PerlIO_printf(Perl_debug_log,
              "### Saw case modifier\n"); });
	    s = PL_bufptr + 1;
	    if (s[1] == '\\' && s[2] == 'E') {
#ifdef PERL_MAD
		if (!PL_thiswhite)
		    PL_thiswhite = newSVpvs("");
		sv_catpvn(PL_thiswhite, PL_bufptr, 4);
#endif
	        PL_bufptr = s + 3;
		PL_lex_state = LEX_INTERPCONCAT;
		return yylex();
	    }
	    else {
		I32 tmp;
		if (!PL_madskills) /* when just compiling don't need correct */
		    if (strnEQ(s, "L\\u", 3) || strnEQ(s, "U\\l", 3))
			tmp = *s, *s = s[2], s[2] = (char)tmp;	/* misordered... */
		if ((*s == 'L' || *s == 'U' || *s == 'F') &&
		    (strchr(PL_lex_casestack, 'L')
                        || strchr(PL_lex_casestack, 'U')
                        || strchr(PL_lex_casestack, 'F'))) {
		    PL_lex_casestack[--PL_lex_casemods] = '\0';
		    PL_lex_allbrackets--;
		    return REPORT(')');
		}
		if (PL_lex_casemods > 10)
		    Renew(PL_lex_casestack, PL_lex_casemods + 2, char);
		PL_lex_casestack[PL_lex_casemods++] = *s;
		PL_lex_casestack[PL_lex_casemods] = '\0';
		PL_lex_state = LEX_INTERPCONCAT;
		start_force(PL_curforce);
		NEXTVAL_NEXTTOKE.ival = 0;
		force_next((2<<24)|'(');
		start_force(PL_curforce);
		if (*s == 'l')
		    NEXTVAL_NEXTTOKE.ival = OP_LCFIRST;
		else if (*s == 'u')
		    NEXTVAL_NEXTTOKE.ival = OP_UCFIRST;
		else if (*s == 'L')
		    NEXTVAL_NEXTTOKE.ival = OP_LC;
		else if (*s == 'U')
		    NEXTVAL_NEXTTOKE.ival = OP_UC;
		else if (*s == 'Q')
		    NEXTVAL_NEXTTOKE.ival = OP_QUOTEMETA;
                else if (*s == 'F')
		    NEXTVAL_NEXTTOKE.ival = OP_FC;
		else
		    Perl_croak(aTHX_ "panic: yylex, *s=%u", *s);
		if (PL_madskills) {
		    SV* const tmpsv = newSVpvs("\\ ");
		    /* replace the space with the character we want to escape
		     */
		    SvPVX(tmpsv)[1] = *s;
		    curmad('_', tmpsv);
		}
		PL_bufptr = s + 1;
	    }
	    force_next(FUNC);
	    if (PL_lex_starts) {
		s = PL_bufptr;
		PL_lex_starts = 0;
#ifdef PERL_MAD
		if (PL_madskills) {
		    if (PL_thistoken)
			sv_free(PL_thistoken);
		    PL_thistoken = newSVpvs("");
		}
#endif
		/* commas only at base level: /$a\Ub$c/ => ($a,uc(b.$c)) */
		if (PL_lex_casemods == 1 && PL_lex_inpat)
		    OPERATOR(',');
		else
		    Aop(OP_CONCAT);
	    }
	    else
		return yylex();
	}

    case LEX_INTERPPUSH:
        return REPORT(sublex_push());

    case LEX_INTERPSTART:
	if (PL_bufptr == PL_bufend)
	    return REPORT(sublex_done());
	DEBUG_T({ PerlIO_printf(Perl_debug_log,
              "### Interpolated variable\n"); });
	PL_expect = XTERM;
	PL_lex_dojoin = (*PL_bufptr == '@');
	PL_lex_state = LEX_INTERPNORMAL;
	if (PL_lex_dojoin) {
	    start_force(PL_curforce);
	    NEXTVAL_NEXTTOKE.ival = 0;
	    force_next(',');
	    start_force(PL_curforce);
	    force_ident("\"", '$');
	    start_force(PL_curforce);
	    NEXTVAL_NEXTTOKE.ival = 0;
	    force_next('$');
	    start_force(PL_curforce);
	    NEXTVAL_NEXTTOKE.ival = 0;
	    force_next((2<<24)|'(');
	    start_force(PL_curforce);
	    NEXTVAL_NEXTTOKE.ival = OP_JOIN;	/* emulate join($", ...) */
	    force_next(FUNC);
	}
	if (PL_lex_starts++) {
	    s = PL_bufptr;
#ifdef PERL_MAD
	    if (PL_madskills) {
		if (PL_thistoken)
		    sv_free(PL_thistoken);
		PL_thistoken = newSVpvs("");
	    }
#endif
	    /* commas only at base level: /$a\Ub$c/ => ($a,uc(b.$c)) */
	    if (!PL_lex_casemods && PL_lex_inpat)
		OPERATOR(',');
	    else
		Aop(OP_CONCAT);
	}
	return yylex();

    case LEX_INTERPENDMAYBE:
	if (intuit_more(PL_bufptr)) {
	    PL_lex_state = LEX_INTERPNORMAL;	/* false alarm, more expr */
	    break;
	}
	/* FALL THROUGH */

    case LEX_INTERPEND:
	if (PL_lex_dojoin) {
	    PL_lex_dojoin = FALSE;
	    PL_lex_state = LEX_INTERPCONCAT;
#ifdef PERL_MAD
	    if (PL_madskills) {
		if (PL_thistoken)
		    sv_free(PL_thistoken);
		PL_thistoken = newSVpvs("");
	    }
#endif
	    PL_lex_allbrackets--;
	    return REPORT(')');
	}
	if (PL_lex_inwhat == OP_SUBST && PL_linestr == PL_lex_repl
	    && SvEVALED(PL_lex_repl))
	{
	    if (PL_bufptr != PL_bufend)
		Perl_croak(aTHX_ "Bad evalled substitution pattern");
	    PL_lex_repl = NULL;
	}
	/* FALLTHROUGH */
    case LEX_INTERPCONCAT:
#ifdef DEBUGGING
	if (PL_lex_brackets)
	    Perl_croak(aTHX_ "panic: INTERPCONCAT, lex_brackets=%ld",
		       (long) PL_lex_brackets);
#endif
	if (PL_bufptr == PL_bufend)
	    return REPORT(sublex_done());

	if (SvIVX(PL_linestr) == '\'') {
	    SV *sv = newSVsv(PL_linestr);
	    if (!PL_lex_inpat)
		sv = tokeq(sv);
	    else if ( PL_hints & HINT_NEW_RE )
		sv = new_constant(NULL, 0, "qr", sv, sv, "q", 1);
	    pl_yylval.opval = (OP*)newSVOP(OP_CONST, 0, sv);
	    s = PL_bufend;
	}
	else {
	    s = scan_const(PL_bufptr);
	    if (*s == '\\')
		PL_lex_state = LEX_INTERPCASEMOD;
	    else
		PL_lex_state = LEX_INTERPSTART;
	}

	if (s != PL_bufptr) {
	    start_force(PL_curforce);
	    if (PL_madskills) {
		curmad('X', newSVpvn(PL_bufptr,s-PL_bufptr));
	    }
	    NEXTVAL_NEXTTOKE = pl_yylval;
	    PL_expect = XTERM;
	    force_next(THING);
	    if (PL_lex_starts++) {
#ifdef PERL_MAD
		if (PL_madskills) {
		    if (PL_thistoken)
			sv_free(PL_thistoken);
		    PL_thistoken = newSVpvs("");
		}
#endif
		/* commas only at base level: /$a\Ub$c/ => ($a,uc(b.$c)) */
		if (!PL_lex_casemods && PL_lex_inpat)
		    OPERATOR(',');
		else
		    Aop(OP_CONCAT);
	    }
	    else {
		PL_bufptr = s;
		return yylex();
	    }
	}

	return yylex();
    case LEX_FORMLINE:
	PL_lex_state = LEX_NORMAL;
	s = scan_formline(PL_bufptr);
	if (!PL_lex_formbrack)
	    goto rightbracket;
	OPERATOR(';');
    }

    s = PL_bufptr;
    PL_oldoldbufptr = PL_oldbufptr;
    PL_oldbufptr = s;

  retry:
#ifdef PERL_MAD
    if (PL_thistoken) {
	sv_free(PL_thistoken);
	PL_thistoken = 0;
    }
    PL_realtokenstart = s - SvPVX(PL_linestr);	/* assume but undo on ws */
#endif
    switch (*s) {
    default:
	if (isIDFIRST_lazy_if(s,UTF))
	    goto keylookup;
	{
        SV *dsv = newSVpvs_flags("", SVs_TEMP);
        const char *c = UTF ? savepv(sv_uni_display(dsv, newSVpvn_flags(s,
                                                    UTF8SKIP(s),
                                                    SVs_TEMP | SVf_UTF8),
                                            10, UNI_DISPLAY_ISPRINT))
                            : Perl_form(aTHX_ "\\x%02X", (unsigned char)*s);
        len = UTF ? Perl_utf8_length(aTHX_ (U8 *) PL_linestart, (U8 *) s) : (STRLEN) (s - PL_linestart);
        if (len > UNRECOGNIZED_PRECEDE_COUNT) {
            d = UTF ? (char *) Perl_utf8_hop(aTHX_ (U8 *) s, -UNRECOGNIZED_PRECEDE_COUNT) : s - UNRECOGNIZED_PRECEDE_COUNT;
        } else {
            d = PL_linestart;
        }	
        *s = '\0';
        sv_setpv(dsv, d);
        if (UTF)
            SvUTF8_on(dsv);
        Perl_croak(aTHX_  "Unrecognized character %s; marked by <-- HERE after %"SVf"<-- HERE near column %d", c, SVfARG(dsv), (int) len + 1);
    }
    case 4:
    case 26:
	goto fake_eof;			/* emulate EOF on ^D or ^Z */
    case 0:
#ifdef PERL_MAD
	if (PL_madskills)
	    PL_faketokens = 0;
#endif
	if (!PL_rsfp && (!PL_parser->filtered || s+1 < PL_bufend)) {
	    PL_last_uni = 0;
	    PL_last_lop = 0;
	    if (PL_lex_brackets &&
		    PL_lex_brackstack[PL_lex_brackets-1] != XFAKEEOF) {
		yyerror((const char *)
			(PL_lex_formbrack
			 ? "Format not terminated"
			 : "Missing right curly or square bracket"));
	    }
            DEBUG_T( { PerlIO_printf(Perl_debug_log,
                        "### Tokener got EOF\n");
            } );
	    TOKEN(0);
	}
	if (s++ < PL_bufend)
	    goto retry;			/* ignore stray nulls */
	PL_last_uni = 0;
	PL_last_lop = 0;
	if (!PL_in_eval && !PL_preambled) {
	    PL_preambled = TRUE;
#ifdef PERL_MAD
	    if (PL_madskills)
		PL_faketokens = 1;
#endif
	    if (PL_perldb) {
		/* Generate a string of Perl code to load the debugger.
		 * If PERL5DB is set, it will return the contents of that,
		 * otherwise a compile-time require of perl5db.pl.  */

		const char * const pdb = PerlEnv_getenv("PERL5DB");

		if (pdb) {
		    sv_setpv(PL_linestr, pdb);
		    sv_catpvs(PL_linestr,";");
		} else {
		    SETERRNO(0,SS_NORMAL);
		    sv_setpvs(PL_linestr, "BEGIN { require 'perl5db.pl' };");
		}
	    } else
		sv_setpvs(PL_linestr,"");
	    if (PL_preambleav) {
		SV **svp = AvARRAY(PL_preambleav);
		SV **const end = svp + AvFILLp(PL_preambleav);
		while(svp <= end) {
		    sv_catsv(PL_linestr, *svp);
		    ++svp;
		    sv_catpvs(PL_linestr, ";");
		}
		sv_free(MUTABLE_SV(PL_preambleav));
		PL_preambleav = NULL;
	    }
	    if (PL_minus_E)
		sv_catpvs(PL_linestr,
			  "use feature ':5." STRINGIFY(PERL_VERSION) "';");
	    if (PL_minus_n || PL_minus_p) {
		sv_catpvs(PL_linestr, "LINE: while (<>) {"/*}*/);
		if (PL_minus_l)
		    sv_catpvs(PL_linestr,"chomp;");
		if (PL_minus_a) {
		    if (PL_minus_F) {
			if ((*PL_splitstr == '/' || *PL_splitstr == '\''
			     || *PL_splitstr == '"')
			      && strchr(PL_splitstr + 1, *PL_splitstr))
			    Perl_sv_catpvf(aTHX_ PL_linestr, "our @F=split(%s);", PL_splitstr);
			else {
			    /* "q\0${splitstr}\0" is legal perl. Yes, even NUL
			       bytes can be used as quoting characters.  :-) */
			    const char *splits = PL_splitstr;
			    sv_catpvs(PL_linestr, "our @F=split(q\0");
			    do {
				/* Need to \ \s  */
				if (*splits == '\\')
				    sv_catpvn(PL_linestr, splits, 1);
				sv_catpvn(PL_linestr, splits, 1);
			    } while (*splits++);
			    /* This loop will embed the trailing NUL of
			       PL_linestr as the last thing it does before
			       terminating.  */
			    sv_catpvs(PL_linestr, ");");
			}
		    }
		    else
		        sv_catpvs(PL_linestr,"our @F=split(' ');");
		}
	    }
	    sv_catpvs(PL_linestr, "\n");
	    PL_oldoldbufptr = PL_oldbufptr = s = PL_linestart = SvPVX(PL_linestr);
	    PL_bufend = SvPVX(PL_linestr) + SvCUR(PL_linestr);
	    PL_last_lop = PL_last_uni = NULL;
	    if ((PERLDB_LINE || PERLDB_SAVESRC) && PL_curstash != PL_debstash)
		update_debugger_info(PL_linestr, NULL, 0);
	    goto retry;
	}
	do {
	    fake_eof = 0;
	    bof = PL_rsfp ? TRUE : FALSE;
	    if (0) {
	      fake_eof:
		fake_eof = LEX_FAKE_EOF;
	    }
	    PL_bufptr = PL_bufend;
	    CopLINE_inc(PL_curcop);
	    if (!lex_next_chunk(fake_eof)) {
		CopLINE_dec(PL_curcop);
		s = PL_bufptr;
		TOKEN(';');	/* not infinite loop because rsfp is NULL now */
	    }
	    CopLINE_dec(PL_curcop);
#ifdef PERL_MAD
	    if (!PL_rsfp)
		PL_realtokenstart = -1;
#endif
	    s = PL_bufptr;
	    /* If it looks like the start of a BOM or raw UTF-16,
	     * check if it in fact is. */
	    if (bof && PL_rsfp &&
		     (*s == 0 ||
		      *(U8*)s == 0xEF ||
		      *(U8*)s >= 0xFE ||
		      s[1] == 0)) {
		Off_t offset = (IV)PerlIO_tell(PL_rsfp);
		bof = (offset == (Off_t)SvCUR(PL_linestr));
#if defined(PERLIO_USING_CRLF) && defined(PERL_TEXTMODE_SCRIPTS)
		/* offset may include swallowed CR */
		if (!bof)
		    bof = (offset == (Off_t)SvCUR(PL_linestr)+1);
#endif
		if (bof) {
		    PL_bufend = SvPVX(PL_linestr) + SvCUR(PL_linestr);
		    s = swallow_bom((U8*)s);
		}
	    }
	    if (PL_parser->in_pod) {
		/* Incest with pod. */
#ifdef PERL_MAD
		if (PL_madskills)
		    sv_catsv(PL_thiswhite, PL_linestr);
#endif
		if (*s == '=' && strnEQ(s, "=cut", 4) && !isALPHA(s[4])) {
		    sv_setpvs(PL_linestr, "");
		    PL_oldoldbufptr = PL_oldbufptr = s = PL_linestart = SvPVX(PL_linestr);
		    PL_bufend = SvPVX(PL_linestr) + SvCUR(PL_linestr);
		    PL_last_lop = PL_last_uni = NULL;
		    PL_parser->in_pod = 0;
		}
	    }
	    if (PL_rsfp || PL_parser->filtered)
		incline(s);
	} while (PL_parser->in_pod);
	PL_oldoldbufptr = PL_oldbufptr = PL_bufptr = PL_linestart = s;
	PL_bufend = SvPVX(PL_linestr) + SvCUR(PL_linestr);
	PL_last_lop = PL_last_uni = NULL;
	if (CopLINE(PL_curcop) == 1) {
	    while (s < PL_bufend && isSPACE(*s))
		s++;
	    if (*s == ':' && s[1] != ':') /* for csh execing sh scripts */
		s++;
#ifdef PERL_MAD
	    if (PL_madskills)
		PL_thiswhite = newSVpvn(PL_linestart, s - PL_linestart);
#endif
	    d = NULL;
	    if (!PL_in_eval) {
		if (*s == '#' && *(s+1) == '!')
		    d = s + 2;
#ifdef ALTERNATE_SHEBANG
		else {
		    static char const as[] = ALTERNATE_SHEBANG;
		    if (*s == as[0] && strnEQ(s, as, sizeof(as) - 1))
			d = s + (sizeof(as) - 1);
		}
#endif /* ALTERNATE_SHEBANG */
	    }
	    if (d) {
		char *ipath;
		char *ipathend;

		while (isSPACE(*d))
		    d++;
		ipath = d;
		while (*d && !isSPACE(*d))
		    d++;
		ipathend = d;

#ifdef ARG_ZERO_IS_SCRIPT
		if (ipathend > ipath) {
		    /*
		     * HP-UX (at least) sets argv[0] to the script name,
		     * which makes $^X incorrect.  And Digital UNIX and Linux,
		     * at least, set argv[0] to the basename of the Perl
		     * interpreter. So, having found "#!", we'll set it right.
		     */
		    SV * const x = GvSV(gv_fetchpvs("\030", GV_ADD|GV_NOTQUAL,
						    SVt_PV)); /* $^X */
		    assert(SvPOK(x) || SvGMAGICAL(x));
		    if (sv_eq(x, CopFILESV(PL_curcop))) {
			sv_setpvn(x, ipath, ipathend - ipath);
			SvSETMAGIC(x);
		    }
		    else {
			STRLEN blen;
			STRLEN llen;
			const char *bstart = SvPV_const(CopFILESV(PL_curcop),blen);
			const char * const lstart = SvPV_const(x,llen);
			if (llen < blen) {
			    bstart += blen - llen;
			    if (strnEQ(bstart, lstart, llen) &&	bstart[-1] == '/') {
				sv_setpvn(x, ipath, ipathend - ipath);
				SvSETMAGIC(x);
			    }
			}
		    }
		    TAINT_NOT;	/* $^X is always tainted, but that's OK */
		}
#endif /* ARG_ZERO_IS_SCRIPT */

		/*
		 * Look for options.
		 */
		d = instr(s,"perl -");
		if (!d) {
		    d = instr(s,"perl");
#if defined(DOSISH)
		    /* avoid getting into infinite loops when shebang
		     * line contains "Perl" rather than "perl" */
		    if (!d) {
			for (d = ipathend-4; d >= ipath; --d) {
			    if ((*d == 'p' || *d == 'P')
				&& !ibcmp(d, "perl", 4))
			    {
				break;
			    }
			}
			if (d < ipath)
			    d = NULL;
		    }
#endif
		}
#ifdef ALTERNATE_SHEBANG
		/*
		 * If the ALTERNATE_SHEBANG on this system starts with a
		 * character that can be part of a Perl expression, then if
		 * we see it but not "perl", we're probably looking at the
		 * start of Perl code, not a request to hand off to some
		 * other interpreter.  Similarly, if "perl" is there, but
		 * not in the first 'word' of the line, we assume the line
		 * contains the start of the Perl program.
		 */
		if (d && *s != '#') {
		    const char *c = ipath;
		    while (*c && !strchr("; \t\r\n\f\v#", *c))
			c++;
		    if (c < d)
			d = NULL;	/* "perl" not in first word; ignore */
		    else
			*s = '#';	/* Don't try to parse shebang line */
		}
#endif /* ALTERNATE_SHEBANG */
		if (!d &&
		    *s == '#' &&
		    ipathend > ipath &&
		    !PL_minus_c &&
		    !instr(s,"indir") &&
		    instr(PL_origargv[0],"perl"))
		{
		    dVAR;
		    char **newargv;

		    *ipathend = '\0';
		    s = ipathend + 1;
		    while (s < PL_bufend && isSPACE(*s))
			s++;
		    if (s < PL_bufend) {
			Newx(newargv,PL_origargc+3,char*);
			newargv[1] = s;
			while (s < PL_bufend && !isSPACE(*s))
			    s++;
			*s = '\0';
			Copy(PL_origargv+1, newargv+2, PL_origargc+1, char*);
		    }
		    else
			newargv = PL_origargv;
		    newargv[0] = ipath;
		    PERL_FPU_PRE_EXEC
		    PerlProc_execv(ipath, EXEC_ARGV_CAST(newargv));
		    PERL_FPU_POST_EXEC
		    Perl_croak(aTHX_ "Can't exec %s", ipath);
		}
		if (d) {
		    while (*d && !isSPACE(*d))
			d++;
		    while (SPACE_OR_TAB(*d))
			d++;

		    if (*d++ == '-') {
			const bool switches_done = PL_doswitches;
			const U32 oldpdb = PL_perldb;
			const bool oldn = PL_minus_n;
			const bool oldp = PL_minus_p;
			const char *d1 = d;

			do {
			    bool baduni = FALSE;
			    if (*d1 == 'C') {
				const char *d2 = d1 + 1;
				if (parse_unicode_opts((const char **)&d2)
				    != PL_unicode)
				    baduni = TRUE;
			    }
			    if (baduni || *d1 == 'M' || *d1 == 'm') {
				const char * const m = d1;
				while (*d1 && !isSPACE(*d1))
				    d1++;
				Perl_croak(aTHX_ "Too late for \"-%.*s\" option",
				      (int)(d1 - m), m);
			    }
			    d1 = moreswitches(d1);
			} while (d1);
			if (PL_doswitches && !switches_done) {
			    int argc = PL_origargc;
			    char **argv = PL_origargv;
			    do {
				argc--,argv++;
			    } while (argc && argv[0][0] == '-' && argv[0][1]);
			    init_argv_symbols(argc,argv);
			}
			if (((PERLDB_LINE || PERLDB_SAVESRC) && !oldpdb) ||
			    ((PL_minus_n || PL_minus_p) && !(oldn || oldp)))
			      /* if we have already added "LINE: while (<>) {",
			         we must not do it again */
			{
			    sv_setpvs(PL_linestr, "");
			    PL_oldoldbufptr = PL_oldbufptr = s = PL_linestart = SvPVX(PL_linestr);
			    PL_bufend = SvPVX(PL_linestr) + SvCUR(PL_linestr);
			    PL_last_lop = PL_last_uni = NULL;
			    PL_preambled = FALSE;
			    if (PERLDB_LINE || PERLDB_SAVESRC)
				(void)gv_fetchfile(PL_origfilename);
			    goto retry;
			}
		    }
		}
	    }
	}
	if (PL_lex_formbrack && PL_lex_brackets <= PL_lex_formbrack) {
	    PL_bufptr = s;
	    PL_lex_state = LEX_FORMLINE;
	    return yylex();
	}
	goto retry;
    case '\r':
#ifdef PERL_STRICT_CR
	Perl_warn(aTHX_ "Illegal character \\%03o (carriage return)", '\r');
	Perl_croak(aTHX_
      "\t(Maybe you didn't strip carriage returns after a network transfer?)\n");
#endif
    case ' ': case '\t': case '\f': case 013:
#ifdef PERL_MAD
	PL_realtokenstart = -1;
	if (!PL_thiswhite)
	    PL_thiswhite = newSVpvs("");
	sv_catpvn(PL_thiswhite, s, 1);
#endif
	s++;
	goto retry;
    case '#':
    case '\n':
#ifdef PERL_MAD
	PL_realtokenstart = -1;
	if (PL_madskills)
	    PL_faketokens = 0;
#endif
	if (PL_lex_state != LEX_NORMAL ||
	     (PL_in_eval && !PL_rsfp && !PL_parser->filtered)) {
	    if (*s == '#' && s == PL_linestart && PL_in_eval
	     && !PL_rsfp && !PL_parser->filtered) {
		/* handle eval qq[#line 1 "foo"\n ...] */
		CopLINE_dec(PL_curcop);
		incline(s);
	    }
	    if (PL_madskills && !PL_lex_formbrack && !PL_in_eval) {
		s = SKIPSPACE0(s);
		if (!PL_in_eval || PL_rsfp || PL_parser->filtered)
		    incline(s);
	    }
	    else {
		d = s;
		while (d < PL_bufend && *d != '\n')
		    d++;
		if (d < PL_bufend)
		    d++;
		else if (d > PL_bufend) /* Found by Ilya: feed random input to Perl. */
		    Perl_croak(aTHX_ "panic: input overflow, %p > %p",
			       d, PL_bufend);
#ifdef PERL_MAD
		if (PL_madskills)
		    PL_thiswhite = newSVpvn(s, d - s);
#endif
		s = d;
		incline(s);
	    }
	    if (PL_lex_formbrack && PL_lex_brackets <= PL_lex_formbrack) {
		PL_bufptr = s;
		PL_lex_state = LEX_FORMLINE;
		return yylex();
	    }
	}
	else {
#ifdef PERL_MAD
	    if (PL_madskills && CopLINE(PL_curcop) >= 1 && !PL_lex_formbrack) {
		if (CopLINE(PL_curcop) == 1 && s[0] == '#' && s[1] == '!') {
		    PL_faketokens = 0;
		    s = SKIPSPACE0(s);
		    TOKEN(PEG);	/* make sure any #! line is accessible */
		}
		s = SKIPSPACE0(s);
	    }
	    else {
/*		if (PL_madskills && PL_lex_formbrack) { */
		    d = s;
		    while (d < PL_bufend && *d != '\n')
			d++;
		    if (d < PL_bufend)
			d++;
		    else if (d > PL_bufend) /* Found by Ilya: feed random input to Perl. */
		      Perl_croak(aTHX_ "panic: input overflow");
		    if (PL_madskills && CopLINE(PL_curcop) >= 1) {
			if (!PL_thiswhite)
			    PL_thiswhite = newSVpvs("");
			if (CopLINE(PL_curcop) == 1) {
			    sv_setpvs(PL_thiswhite, "");
			    PL_faketokens = 0;
			}
			sv_catpvn(PL_thiswhite, s, d - s);
		    }
		    s = d;
/*		}
		*s = '\0';
		PL_bufend = s; */
	    }
#else
	    *s = '\0';
	    PL_bufend = s;
#endif
	}
	goto retry;
    case '-':
	if (s[1] && isALPHA(s[1]) && !isALNUM(s[2])) {
	    I32 ftst = 0;
	    char tmp;

	    s++;
	    PL_bufptr = s;
	    tmp = *s++;

	    while (s < PL_bufend && SPACE_OR_TAB(*s))
		s++;

	    if (strnEQ(s,"=>",2)) {
		s = force_word(PL_bufptr,WORD,FALSE,FALSE,FALSE);
		DEBUG_T( { printbuf("### Saw unary minus before =>, forcing word %s\n", s); } );
		OPERATOR('-');		/* unary minus */
	    }
	    PL_last_uni = PL_oldbufptr;
	    switch (tmp) {
	    case 'r': ftst = OP_FTEREAD;	break;
	    case 'w': ftst = OP_FTEWRITE;	break;
	    case 'x': ftst = OP_FTEEXEC;	break;
	    case 'o': ftst = OP_FTEOWNED;	break;
	    case 'R': ftst = OP_FTRREAD;	break;
	    case 'W': ftst = OP_FTRWRITE;	break;
	    case 'X': ftst = OP_FTREXEC;	break;
	    case 'O': ftst = OP_FTROWNED;	break;
	    case 'e': ftst = OP_FTIS;		break;
	    case 'z': ftst = OP_FTZERO;		break;
	    case 's': ftst = OP_FTSIZE;		break;
	    case 'f': ftst = OP_FTFILE;		break;
	    case 'd': ftst = OP_FTDIR;		break;
	    case 'l': ftst = OP_FTLINK;		break;
	    case 'p': ftst = OP_FTPIPE;		break;
	    case 'S': ftst = OP_FTSOCK;		break;
	    case 'u': ftst = OP_FTSUID;		break;
	    case 'g': ftst = OP_FTSGID;		break;
	    case 'k': ftst = OP_FTSVTX;		break;
	    case 'b': ftst = OP_FTBLK;		break;
	    case 'c': ftst = OP_FTCHR;		break;
	    case 't': ftst = OP_FTTTY;		break;
	    case 'T': ftst = OP_FTTEXT;		break;
	    case 'B': ftst = OP_FTBINARY;	break;
	    case 'M': case 'A': case 'C':
		gv_fetchpvs("\024", GV_ADD|GV_NOTQUAL, SVt_PV);
		switch (tmp) {
		case 'M': ftst = OP_FTMTIME;	break;
		case 'A': ftst = OP_FTATIME;	break;
		case 'C': ftst = OP_FTCTIME;	break;
		default:			break;
		}
		break;
	    default:
		break;
	    }
	    if (ftst) {
		PL_last_lop_op = (OPCODE)ftst;
		DEBUG_T( { PerlIO_printf(Perl_debug_log,
                        "### Saw file test %c\n", (int)tmp);
		} );
		FTST(ftst);
	    }
	    else {
		/* Assume it was a minus followed by a one-letter named
		 * subroutine call (or a -bareword), then. */
		DEBUG_T( { PerlIO_printf(Perl_debug_log,
			"### '-%c' looked like a file test but was not\n",
			(int) tmp);
		} );
		s = --PL_bufptr;
	    }
	}
	{
	    const char tmp = *s++;
	    if (*s == tmp) {
		s++;
		if (PL_expect == XOPERATOR)
		    TERM(POSTDEC);
		else
		    OPERATOR(PREDEC);
	    }
	    else if (*s == '>') {
		s++;
		s = SKIPSPACE1(s);
		if (isIDFIRST_lazy_if(s,UTF)) {
		    s = force_word(s,METHOD,FALSE,TRUE,FALSE);
		    TOKEN(ARROW);
		}
		else if (*s == '$')
		    OPERATOR(ARROW);
		else
		    TERM(ARROW);
	    }
	    if (PL_expect == XOPERATOR) {
		if (*s == '=' && !PL_lex_allbrackets &&
			PL_lex_fakeeof >= LEX_FAKEEOF_ASSIGN) {
		    s--;
		    TOKEN(0);
		}
		Aop(OP_SUBTRACT);
	    }
	    else {
		if (isSPACE(*s) || !isSPACE(*PL_bufptr))
		    check_uni();
		OPERATOR('-');		/* unary minus */
	    }
	}

    case '+':
	{
	    const char tmp = *s++;
	    if (*s == tmp) {
		s++;
		if (PL_expect == XOPERATOR)
		    TERM(POSTINC);
		else
		    OPERATOR(PREINC);
	    }
	    if (PL_expect == XOPERATOR) {
		if (*s == '=' && !PL_lex_allbrackets &&
			PL_lex_fakeeof >= LEX_FAKEEOF_ASSIGN) {
		    s--;
		    TOKEN(0);
		}
		Aop(OP_ADD);
	    }
	    else {
		if (isSPACE(*s) || !isSPACE(*PL_bufptr))
		    check_uni();
		OPERATOR('+');
	    }
	}

    case '*':
	if (PL_expect != XOPERATOR) {
	    s = scan_ident(s, PL_bufend, PL_tokenbuf, sizeof PL_tokenbuf, TRUE);
	    PL_expect = XOPERATOR;
	    force_ident(PL_tokenbuf, '*');
	    if (!*PL_tokenbuf)
		PREREF('*');
	    TERM('*');
	}
	s++;
	if (*s == '*') {
	    s++;
	    if (*s == '=' && !PL_lex_allbrackets &&
		    PL_lex_fakeeof >= LEX_FAKEEOF_ASSIGN) {
		s -= 2;
		TOKEN(0);
	    }
	    PWop(OP_POW);
	}
	if (*s == '=' && !PL_lex_allbrackets &&
		PL_lex_fakeeof >= LEX_FAKEEOF_ASSIGN) {
	    s--;
	    TOKEN(0);
	}
	Mop(OP_MULTIPLY);

    case '%':
	if (PL_expect == XOPERATOR) {
	    if (s[1] == '=' && !PL_lex_allbrackets &&
		    PL_lex_fakeeof >= LEX_FAKEEOF_ASSIGN)
		TOKEN(0);
	    ++s;
	    Mop(OP_MODULO);
	}
	PL_tokenbuf[0] = '%';
	s = scan_ident(s, PL_bufend, PL_tokenbuf + 1,
		sizeof PL_tokenbuf - 1, FALSE);
	if (!PL_tokenbuf[1]) {
	    PREREF('%');
	}
	PL_pending_ident = '%';
	TERM('%');

    case '^':
	if (!PL_lex_allbrackets && PL_lex_fakeeof >=
		(s[1] == '=' ? LEX_FAKEEOF_ASSIGN : LEX_FAKEEOF_BITWISE))
	    TOKEN(0);
	s++;
	BOop(OP_BIT_XOR);
    case '[':
	if (PL_lex_brackets > 100)
	    Renew(PL_lex_brackstack, PL_lex_brackets + 10, char);
	PL_lex_brackstack[PL_lex_brackets++] = 0;
	PL_lex_allbrackets++;
	{
	    const char tmp = *s++;
	    OPERATOR(tmp);
	}
    case '~':
	if (s[1] == '~'
	    && (PL_expect == XOPERATOR || PL_expect == XTERMORDORDOR))
	{
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE)
		TOKEN(0);
	    s += 2;
	    Eop(OP_SMARTMATCH);
	}
	s++;
	OPERATOR('~');
    case ',':
	if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_COMMA)
	    TOKEN(0);
	s++;
	OPERATOR(',');
    case ':':
	if (s[1] == ':') {
	    len = 0;
	    goto just_a_word_zero_gv;
	}
	s++;
	switch (PL_expect) {
	    OP *attrs;
#ifdef PERL_MAD
	    I32 stuffstart;
#endif
	case XOPERATOR:
	    if (!PL_in_my || PL_lex_state != LEX_NORMAL)
		break;
	    PL_bufptr = s;	/* update in case we back off */
	    if (*s == '=') {
		Perl_croak(aTHX_
			   "Use of := for an empty attribute list is not allowed");
	    }
	    goto grabattrs;
	case XATTRBLOCK:
	    PL_expect = XBLOCK;
	    goto grabattrs;
	case XATTRTERM:
	    PL_expect = XTERMBLOCK;
	 grabattrs:
#ifdef PERL_MAD
	    stuffstart = s - SvPVX(PL_linestr) - 1;
#endif
	    s = PEEKSPACE(s);
	    attrs = NULL;
	    while (isIDFIRST_lazy_if(s,UTF)) {
		I32 tmp;
		SV *sv;
		d = scan_word(s, PL_tokenbuf, sizeof PL_tokenbuf, FALSE, &len);
		if (isLOWER(*s) && (tmp = keyword(PL_tokenbuf, len, 0))) {
		    if (tmp < 0) tmp = -tmp;
		    switch (tmp) {
		    case KEY_or:
		    case KEY_and:
		    case KEY_for:
		    case KEY_foreach:
		    case KEY_unless:
		    case KEY_if:
		    case KEY_while:
		    case KEY_until:
			goto got_attrs;
		    default:
			break;
		    }
		}
		sv = newSVpvn_flags(s, len, UTF ? SVf_UTF8 : 0);
		if (*d == '(') {
		    d = scan_str(d,TRUE,TRUE);
		    if (!d) {
			/* MUST advance bufptr here to avoid bogus
			   "at end of line" context messages from yyerror().
			 */
			PL_bufptr = s + len;
			yyerror("Unterminated attribute parameter in attribute list");
			if (attrs)
			    op_free(attrs);
			sv_free(sv);
			return REPORT(0);	/* EOF indicator */
		    }
		}
		if (PL_lex_stuff) {
		    sv_catsv(sv, PL_lex_stuff);
		    attrs = op_append_elem(OP_LIST, attrs,
					newSVOP(OP_CONST, 0, sv));
		    SvREFCNT_dec(PL_lex_stuff);
		    PL_lex_stuff = NULL;
		}
		else {
		    if (len == 6 && strnEQ(SvPVX(sv), "unique", len)) {
			sv_free(sv);
			if (PL_in_my == KEY_our) {
			    deprecate(":unique");
			}
			else
			    Perl_croak(aTHX_ "The 'unique' attribute may only be applied to 'our' variables");
		    }

		    /* NOTE: any CV attrs applied here need to be part of
		       the CVf_BUILTIN_ATTRS define in cv.h! */
		    else if (!PL_in_my && len == 6 && strnEQ(SvPVX(sv), "lvalue", len)) {
			sv_free(sv);
			CvLVALUE_on(PL_compcv);
		    }
		    else if (!PL_in_my && len == 6 && strnEQ(SvPVX(sv), "locked", len)) {
			sv_free(sv);
			deprecate(":locked");
		    }
		    else if (!PL_in_my && len == 6 && strnEQ(SvPVX(sv), "method", len)) {
			sv_free(sv);
			CvMETHOD_on(PL_compcv);
		    }
		    /* After we've set the flags, it could be argued that
		       we don't need to do the attributes.pm-based setting
		       process, and shouldn't bother appending recognized
		       flags.  To experiment with that, uncomment the
		       following "else".  (Note that's already been
		       uncommented.  That keeps the above-applied built-in
		       attributes from being intercepted (and possibly
		       rejected) by a package's attribute routines, but is
		       justified by the performance win for the common case
		       of applying only built-in attributes.) */
		    else
		        attrs = op_append_elem(OP_LIST, attrs,
					    newSVOP(OP_CONST, 0,
					      	    sv));
		}
		s = PEEKSPACE(d);
		if (*s == ':' && s[1] != ':')
		    s = PEEKSPACE(s+1);
		else if (s == d)
		    break;	/* require real whitespace or :'s */
		/* XXX losing whitespace on sequential attributes here */
	    }
	    {
		const char tmp
		    = (PL_expect == XOPERATOR ? '=' : '{'); /*'}(' for vi */
		if (*s != ';' && *s != '}' && *s != tmp
		    && (tmp != '=' || *s != ')')) {
		    const char q = ((*s == '\'') ? '"' : '\'');
		    /* If here for an expression, and parsed no attrs, back
		       off. */
		    if (tmp == '=' && !attrs) {
			s = PL_bufptr;
			break;
		    }
		    /* MUST advance bufptr here to avoid bogus "at end of line"
		       context messages from yyerror().
		    */
		    PL_bufptr = s;
		    yyerror( (const char *)
			     (*s
			      ? Perl_form(aTHX_ "Invalid separator character "
					  "%c%c%c in attribute list", q, *s, q)
			      : "Unterminated attribute list" ) );
		    if (attrs)
			op_free(attrs);
		    OPERATOR(':');
		}
	    }
	got_attrs:
	    if (attrs) {
		start_force(PL_curforce);
		NEXTVAL_NEXTTOKE.opval = attrs;
		CURMAD('_', PL_nextwhite);
		force_next(THING);
	    }
#ifdef PERL_MAD
	    if (PL_madskills) {
		PL_thistoken = newSVpvn(SvPVX(PL_linestr) + stuffstart,
				     (s - SvPVX(PL_linestr)) - stuffstart);
	    }
#endif
	    TOKEN(COLONATTR);
	}
	if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_CLOSING) {
	    s--;
	    TOKEN(0);
	}
	PL_lex_allbrackets--;
	OPERATOR(':');
    case '(':
	s++;
	if (PL_last_lop == PL_oldoldbufptr || PL_last_uni == PL_oldoldbufptr)
	    PL_oldbufptr = PL_oldoldbufptr;		/* allow print(STDOUT 123) */
	else
	    PL_expect = XTERM;
	s = SKIPSPACE1(s);
	PL_lex_allbrackets++;
	TOKEN('(');
    case ';':
	if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_NONEXPR)
	    TOKEN(0);
	CLINE;
	s++;
	OPERATOR(';');
    case ')':
	if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_CLOSING)
	    TOKEN(0);
	s++;
	PL_lex_allbrackets--;
	s = SKIPSPACE1(s);
	if (*s == '{')
	    PREBLOCK(')');
	TERM(')');
    case ']':
	if (PL_lex_brackets && PL_lex_brackstack[PL_lex_brackets-1] == XFAKEEOF)
	    TOKEN(0);
	s++;
	if (PL_lex_brackets <= 0)
	    yyerror("Unmatched right square bracket");
	else
	    --PL_lex_brackets;
	PL_lex_allbrackets--;
	if (PL_lex_state == LEX_INTERPNORMAL) {
	    if (PL_lex_brackets == 0) {
		if (*s == '-' && s[1] == '>')
		    PL_lex_state = LEX_INTERPENDMAYBE;
		else if (*s != '[' && *s != '{')
		    PL_lex_state = LEX_INTERPEND;
	    }
	}
	TERM(']');
    case '{':
      leftbracket:
	s++;
	if (PL_lex_brackets > 100) {
	    Renew(PL_lex_brackstack, PL_lex_brackets + 10, char);
	}
	switch (PL_expect) {
	case XTERM:
	    if (PL_lex_formbrack) {
		s--;
		PRETERMBLOCK(DO);
	    }
	    if (PL_oldoldbufptr == PL_last_lop)
		PL_lex_brackstack[PL_lex_brackets++] = XTERM;
	    else
		PL_lex_brackstack[PL_lex_brackets++] = XOPERATOR;
	    PL_lex_allbrackets++;
	    OPERATOR(HASHBRACK);
	case XOPERATOR:
	    while (s < PL_bufend && SPACE_OR_TAB(*s))
		s++;
	    d = s;
	    PL_tokenbuf[0] = '\0';
	    if (d < PL_bufend && *d == '-') {
		PL_tokenbuf[0] = '-';
		d++;
		while (d < PL_bufend && SPACE_OR_TAB(*d))
		    d++;
	    }
	    if (d < PL_bufend && isIDFIRST_lazy_if(d,UTF)) {
		d = scan_word(d, PL_tokenbuf + 1, sizeof PL_tokenbuf - 1,
			      FALSE, &len);
		while (d < PL_bufend && SPACE_OR_TAB(*d))
		    d++;
		if (*d == '}') {
		    const char minus = (PL_tokenbuf[0] == '-');
		    s = force_word(s + minus, WORD, FALSE, TRUE, FALSE);
		    if (minus)
			force_next('-');
		}
	    }
	    /* FALL THROUGH */
	case XATTRBLOCK:
	case XBLOCK:
	    PL_lex_brackstack[PL_lex_brackets++] = XSTATE;
	    PL_lex_allbrackets++;
	    PL_expect = XSTATE;
	    break;
	case XATTRTERM:
	case XTERMBLOCK:
	    PL_lex_brackstack[PL_lex_brackets++] = XOPERATOR;
	    PL_lex_allbrackets++;
	    PL_expect = XSTATE;
	    break;
	default: {
		const char *t;
		if (PL_oldoldbufptr == PL_last_lop)
		    PL_lex_brackstack[PL_lex_brackets++] = XTERM;
		else
		    PL_lex_brackstack[PL_lex_brackets++] = XOPERATOR;
		PL_lex_allbrackets++;
		s = SKIPSPACE1(s);
		if (*s == '}') {
		    if (PL_expect == XREF && PL_lex_state == LEX_INTERPNORMAL) {
			PL_expect = XTERM;
			/* This hack is to get the ${} in the message. */
			PL_bufptr = s+1;
			yyerror("syntax error");
			break;
		    }
		    OPERATOR(HASHBRACK);
		}
		/* This hack serves to disambiguate a pair of curlies
		 * as being a block or an anon hash.  Normally, expectation
		 * determines that, but in cases where we're not in a
		 * position to expect anything in particular (like inside
		 * eval"") we have to resolve the ambiguity.  This code
		 * covers the case where the first term in the curlies is a
		 * quoted string.  Most other cases need to be explicitly
		 * disambiguated by prepending a "+" before the opening
		 * curly in order to force resolution as an anon hash.
		 *
		 * XXX should probably propagate the outer expectation
		 * into eval"" to rely less on this hack, but that could
		 * potentially break current behavior of eval"".
		 * GSAR 97-07-21
		 */
		t = s;
		if (*s == '\'' || *s == '"' || *s == '`') {
		    /* common case: get past first string, handling escapes */
		    for (t++; t < PL_bufend && *t != *s;)
			if (*t++ == '\\' && (*t == '\\' || *t == *s))
			    t++;
		    t++;
		}
		else if (*s == 'q') {
		    if (++t < PL_bufend
			&& (!isALNUM(*t)
			    || ((*t == 'q' || *t == 'x') && ++t < PL_bufend
				&& !isALNUM(*t))))
		    {
			/* skip q//-like construct */
			const char *tmps;
			char open, close, term;
			I32 brackets = 1;

			while (t < PL_bufend && isSPACE(*t))
			    t++;
			/* check for q => */
			if (t+1 < PL_bufend && t[0] == '=' && t[1] == '>') {
			    OPERATOR(HASHBRACK);
			}
			term = *t;
			open = term;
			if (term && (tmps = strchr("([{< )]}> )]}>",term)))
			    term = tmps[5];
			close = term;
			if (open == close)
			    for (t++; t < PL_bufend; t++) {
				if (*t == '\\' && t+1 < PL_bufend && open != '\\')
				    t++;
				else if (*t == open)
				    break;
			    }
			else {
			    for (t++; t < PL_bufend; t++) {
				if (*t == '\\' && t+1 < PL_bufend)
				    t++;
				else if (*t == close && --brackets <= 0)
				    break;
				else if (*t == open)
				    brackets++;
			    }
			}
			t++;
		    }
		    else
			/* skip plain q word */
			while (t < PL_bufend && isALNUM_lazy_if(t,UTF))
			     t += UTF8SKIP(t);
		}
		else if (isALNUM_lazy_if(t,UTF)) {
		    t += UTF8SKIP(t);
		    while (t < PL_bufend && isALNUM_lazy_if(t,UTF))
			 t += UTF8SKIP(t);
		}
		while (t < PL_bufend && isSPACE(*t))
		    t++;
		/* if comma follows first term, call it an anon hash */
		/* XXX it could be a comma expression with loop modifiers */
		if (t < PL_bufend && ((*t == ',' && (*s == 'q' || !isLOWER(*s)))
				   || (*t == '=' && t[1] == '>')))
		    OPERATOR(HASHBRACK);
		if (PL_expect == XREF)
		    PL_expect = XTERM;
		else {
		    PL_lex_brackstack[PL_lex_brackets-1] = XSTATE;
		    PL_expect = XSTATE;
		}
	    }
	    break;
	}
	pl_yylval.ival = CopLINE(PL_curcop);
	if (isSPACE(*s) || *s == '#')
	    PL_copline = NOLINE;   /* invalidate current command line number */
	TOKEN('{');
    case '}':
	if (PL_lex_brackets && PL_lex_brackstack[PL_lex_brackets-1] == XFAKEEOF)
	    TOKEN(0);
      rightbracket:
	s++;
	if (PL_lex_brackets <= 0)
	    yyerror("Unmatched right curly bracket");
	else
	    PL_expect = (expectation)PL_lex_brackstack[--PL_lex_brackets];
	PL_lex_allbrackets--;
	if (PL_lex_brackets < PL_lex_formbrack && PL_lex_state != LEX_INTERPNORMAL)
	    PL_lex_formbrack = 0;
	if (PL_lex_state == LEX_INTERPNORMAL) {
	    if (PL_lex_brackets == 0) {
		if (PL_expect & XFAKEBRACK) {
		    PL_expect &= XENUMMASK;
		    PL_lex_state = LEX_INTERPEND;
		    PL_bufptr = s;
#if 0
		    if (PL_madskills) {
			if (!PL_thiswhite)
			    PL_thiswhite = newSVpvs("");
			sv_catpvs(PL_thiswhite,"}");
		    }
#endif
		    return yylex();	/* ignore fake brackets */
		}
		if (*s == '-' && s[1] == '>')
		    PL_lex_state = LEX_INTERPENDMAYBE;
		else if (*s != '[' && *s != '{')
		    PL_lex_state = LEX_INTERPEND;
	    }
	}
	if (PL_expect & XFAKEBRACK) {
	    PL_expect &= XENUMMASK;
	    PL_bufptr = s;
	    return yylex();		/* ignore fake brackets */
	}
	start_force(PL_curforce);
	if (PL_madskills) {
	    curmad('X', newSVpvn(s-1,1));
	    CURMAD('_', PL_thiswhite);
	}
	force_next('}');
#ifdef PERL_MAD
	if (!PL_thistoken)
	    PL_thistoken = newSVpvs("");
#endif
	TOKEN(';');
    case '&':
	s++;
	if (*s++ == '&') {
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >=
		    (*s == '=' ? LEX_FAKEEOF_ASSIGN : LEX_FAKEEOF_LOGIC)) {
		s -= 2;
		TOKEN(0);
	    }
	    AOPERATOR(ANDAND);
	}
	s--;
	if (PL_expect == XOPERATOR) {
	    if (PL_bufptr == PL_linestart && ckWARN(WARN_SEMICOLON)
		&& isIDFIRST_lazy_if(s,UTF))
	    {
		CopLINE_dec(PL_curcop);
		Perl_warner(aTHX_ packWARN(WARN_SEMICOLON), "%s", PL_warn_nosemi);
		CopLINE_inc(PL_curcop);
	    }
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >=
		    (*s == '=' ? LEX_FAKEEOF_ASSIGN : LEX_FAKEEOF_BITWISE)) {
		s--;
		TOKEN(0);
	    }
	    BAop(OP_BIT_AND);
	}

	s = scan_ident(s - 1, PL_bufend, PL_tokenbuf, sizeof PL_tokenbuf, TRUE);
	if (*PL_tokenbuf) {
	    PL_expect = XOPERATOR;
	    force_ident(PL_tokenbuf, '&');
	}
	else
	    PREREF('&');
	pl_yylval.ival = (OPpENTERSUB_AMPER<<8);
	TERM('&');

    case '|':
	s++;
	if (*s++ == '|') {
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >=
		    (*s == '=' ? LEX_FAKEEOF_ASSIGN : LEX_FAKEEOF_LOGIC)) {
		s -= 2;
		TOKEN(0);
	    }
	    AOPERATOR(OROR);
	}
	s--;
	if (!PL_lex_allbrackets && PL_lex_fakeeof >=
		(*s == '=' ? LEX_FAKEEOF_ASSIGN : LEX_FAKEEOF_BITWISE)) {
	    s--;
	    TOKEN(0);
	}
	BOop(OP_BIT_OR);
    case '=':
	s++;
	{
	    const char tmp = *s++;
	    if (tmp == '=') {
		if (!PL_lex_allbrackets &&
			PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE) {
		    s -= 2;
		    TOKEN(0);
		}
		Eop(OP_EQ);
	    }
	    if (tmp == '>') {
		if (!PL_lex_allbrackets &&
			PL_lex_fakeeof >= LEX_FAKEEOF_COMMA) {
		    s -= 2;
		    TOKEN(0);
		}
		OPERATOR(',');
	    }
	    if (tmp == '~')
		PMop(OP_MATCH);
	    if (tmp && isSPACE(*s) && ckWARN(WARN_SYNTAX)
		&& strchr("+-*/%.^&|<",tmp))
		Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
			    "Reversed %c= operator",(int)tmp);
	    s--;
	    if (PL_expect == XSTATE && isALPHA(tmp) &&
		(s == PL_linestart+1 || s[-2] == '\n') )
		{
		    if (PL_in_eval && !PL_rsfp && !PL_parser->filtered) {
			d = PL_bufend;
			while (s < d) {
			    if (*s++ == '\n') {
				incline(s);
				if (strnEQ(s,"=cut",4)) {
				    s = strchr(s,'\n');
				    if (s)
					s++;
				    else
					s = d;
				    incline(s);
				    goto retry;
				}
			    }
			}
			goto retry;
		    }
#ifdef PERL_MAD
		    if (PL_madskills) {
			if (!PL_thiswhite)
			    PL_thiswhite = newSVpvs("");
			sv_catpvn(PL_thiswhite, PL_linestart,
				  PL_bufend - PL_linestart);
		    }
#endif
		    s = PL_bufend;
		    PL_parser->in_pod = 1;
		    goto retry;
		}
	}
	if (PL_lex_brackets < PL_lex_formbrack) {
	    const char *t = s;
#ifdef PERL_STRICT_CR
	    while (SPACE_OR_TAB(*t))
#else
	    while (SPACE_OR_TAB(*t) || *t == '\r')
#endif
		t++;
	    if (*t == '\n' || *t == '#') {
		s--;
		PL_expect = XBLOCK;
		goto leftbracket;
	    }
	}
	if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_ASSIGN) {
	    s--;
	    TOKEN(0);
	}
	pl_yylval.ival = 0;
	OPERATOR(ASSIGNOP);
    case '!':
	s++;
	{
	    const char tmp = *s++;
	    if (tmp == '=') {
		/* was this !=~ where !~ was meant?
		 * warn on m:!=~\s+([/?]|[msy]\W|tr\W): */

		if (*s == '~' && ckWARN(WARN_SYNTAX)) {
		    const char *t = s+1;

		    while (t < PL_bufend && isSPACE(*t))
			++t;

		    if (*t == '/' || *t == '?' ||
			((*t == 'm' || *t == 's' || *t == 'y')
			 && !isALNUM(t[1])) ||
			(*t == 't' && t[1] == 'r' && !isALNUM(t[2])))
			Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
				    "!=~ should be !~");
		}
		if (!PL_lex_allbrackets &&
			PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE) {
		    s -= 2;
		    TOKEN(0);
		}
		Eop(OP_NE);
	    }
	    if (tmp == '~')
		PMop(OP_NOT);
	}
	s--;
	OPERATOR('!');
    case '<':
	if (PL_expect != XOPERATOR) {
	    if (s[1] != '<' && !strchr(s,'>'))
		check_uni();
	    if (s[1] == '<')
		s = scan_heredoc(s);
	    else
		s = scan_inputsymbol(s);
	    TERM(sublex_start());
	}
	s++;
	{
	    char tmp = *s++;
	    if (tmp == '<') {
		if (*s == '=' && !PL_lex_allbrackets &&
			PL_lex_fakeeof >= LEX_FAKEEOF_ASSIGN) {
		    s -= 2;
		    TOKEN(0);
		}
		SHop(OP_LEFT_SHIFT);
	    }
	    if (tmp == '=') {
		tmp = *s++;
		if (tmp == '>') {
		    if (!PL_lex_allbrackets &&
			    PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE) {
			s -= 3;
			TOKEN(0);
		    }
		    Eop(OP_NCMP);
		}
		s--;
		if (!PL_lex_allbrackets &&
			PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE) {
		    s -= 2;
		    TOKEN(0);
		}
		Rop(OP_LE);
	    }
	}
	s--;
	if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE) {
	    s--;
	    TOKEN(0);
	}
	Rop(OP_LT);
    case '>':
	s++;
	{
	    const char tmp = *s++;
	    if (tmp == '>') {
		if (*s == '=' && !PL_lex_allbrackets &&
			PL_lex_fakeeof >= LEX_FAKEEOF_ASSIGN) {
		    s -= 2;
		    TOKEN(0);
		}
		SHop(OP_RIGHT_SHIFT);
	    }
	    else if (tmp == '=') {
		if (!PL_lex_allbrackets &&
			PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE) {
		    s -= 2;
		    TOKEN(0);
		}
		Rop(OP_GE);
	    }
	}
	s--;
	if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE) {
	    s--;
	    TOKEN(0);
	}
	Rop(OP_GT);

    case '$':
	CLINE;

	if (PL_expect == XOPERATOR) {
	    if (PL_lex_formbrack && PL_lex_brackets == PL_lex_formbrack) {
		return deprecate_commaless_var_list();
	    }
	}

	if (s[1] == '#' && (isIDFIRST_lazy_if(s+2,UTF) || strchr("{$:+-@", s[2]))) {
	    PL_tokenbuf[0] = '@';
	    s = scan_ident(s + 1, PL_bufend, PL_tokenbuf + 1,
			   sizeof PL_tokenbuf - 1, FALSE);
	    if (PL_expect == XOPERATOR)
		no_op("Array length", s);
	    if (!PL_tokenbuf[1])
		PREREF(DOLSHARP);
	    PL_expect = XOPERATOR;
	    PL_pending_ident = '#';
	    TOKEN(DOLSHARP);
	}

	PL_tokenbuf[0] = '$';
	s = scan_ident(s, PL_bufend, PL_tokenbuf + 1,
		       sizeof PL_tokenbuf - 1, FALSE);
	if (PL_expect == XOPERATOR)
	    no_op("Scalar", s);
	if (!PL_tokenbuf[1]) {
	    if (s == PL_bufend)
		yyerror("Final $ should be \\$ or $name");
	    PREREF('$');
	}

	d = s;
	{
	    const char tmp = *s;
	    if (PL_lex_state == LEX_NORMAL || PL_lex_brackets)
		s = SKIPSPACE1(s);

	    if ((PL_expect != XREF || PL_oldoldbufptr == PL_last_lop)
		&& intuit_more(s)) {
		if (*s == '[') {
		    PL_tokenbuf[0] = '@';
		    if (ckWARN(WARN_SYNTAX)) {
			char *t = s+1;

			while (isSPACE(*t) || isALNUM_lazy_if(t,UTF) || *t == '$')
			    t++;
			if (*t++ == ',') {
			    PL_bufptr = PEEKSPACE(PL_bufptr); /* XXX can realloc */
			    while (t < PL_bufend && *t != ']')
				t++;
			    Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
					"Multidimensional syntax %.*s not supported",
				    (int)((t - PL_bufptr) + 1), PL_bufptr);
			}
		    }
		}
		else if (*s == '{') {
		    char *t;
		    PL_tokenbuf[0] = '%';
		    if (strEQ(PL_tokenbuf+1, "SIG")  && ckWARN(WARN_SYNTAX)
			&& (t = strchr(s, '}')) && (t = strchr(t, '=')))
			{
			    char tmpbuf[sizeof PL_tokenbuf];
			    do {
				t++;
			    } while (isSPACE(*t));
			    if (isIDFIRST_lazy_if(t,UTF)) {
				STRLEN len;
				t = scan_word(t, tmpbuf, sizeof tmpbuf, TRUE,
					      &len);
				while (isSPACE(*t))
				    t++;
				if (*t == ';'
                                       && get_cvn_flags(tmpbuf, len, UTF ? SVf_UTF8 : 0))
				    Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
						"You need to quote \"%"SVf"\"",
						  SVfARG(newSVpvn_flags(tmpbuf, len, 
                                                    SVs_TEMP | (UTF ? SVf_UTF8 : 0))));
			    }
			}
		}
	    }

	    PL_expect = XOPERATOR;
	    if (PL_lex_state == LEX_NORMAL && isSPACE((char)tmp)) {
		const bool islop = (PL_last_lop == PL_oldoldbufptr);
		if (!islop || PL_last_lop_op == OP_GREPSTART)
		    PL_expect = XOPERATOR;
		else if (strchr("$@\"'`q", *s))
		    PL_expect = XTERM;		/* e.g. print $fh "foo" */
		else if (strchr("&*<%", *s) && isIDFIRST_lazy_if(s+1,UTF))
		    PL_expect = XTERM;		/* e.g. print $fh &sub */
		else if (isIDFIRST_lazy_if(s,UTF)) {
		    char tmpbuf[sizeof PL_tokenbuf];
		    int t2;
		    scan_word(s, tmpbuf, sizeof tmpbuf, TRUE, &len);
		    if ((t2 = keyword(tmpbuf, len, 0))) {
			/* binary operators exclude handle interpretations */
			switch (t2) {
			case -KEY_x:
			case -KEY_eq:
			case -KEY_ne:
			case -KEY_gt:
			case -KEY_lt:
			case -KEY_ge:
			case -KEY_le:
			case -KEY_cmp:
			    break;
			default:
			    PL_expect = XTERM;	/* e.g. print $fh length() */
			    break;
			}
		    }
		    else {
			PL_expect = XTERM;	/* e.g. print $fh subr() */
		    }
		}
		else if (isDIGIT(*s))
		    PL_expect = XTERM;		/* e.g. print $fh 3 */
		else if (*s == '.' && isDIGIT(s[1]))
		    PL_expect = XTERM;		/* e.g. print $fh .3 */
		else if ((*s == '?' || *s == '-' || *s == '+')
			 && !isSPACE(s[1]) && s[1] != '=')
		    PL_expect = XTERM;		/* e.g. print $fh -1 */
		else if (*s == '/' && !isSPACE(s[1]) && s[1] != '='
			 && s[1] != '/')
		    PL_expect = XTERM;		/* e.g. print $fh /.../
						   XXX except DORDOR operator
						*/
		else if (*s == '<' && s[1] == '<' && !isSPACE(s[2])
			 && s[2] != '=')
		    PL_expect = XTERM;		/* print $fh <<"EOF" */
	    }
	}
	PL_pending_ident = '$';
	TOKEN('$');

    case '@':
	if (PL_expect == XOPERATOR)
	    no_op("Array", s);
	PL_tokenbuf[0] = '@';
	s = scan_ident(s, PL_bufend, PL_tokenbuf + 1, sizeof PL_tokenbuf - 1, FALSE);
	if (!PL_tokenbuf[1]) {
	    PREREF('@');
	}
	if (PL_lex_state == LEX_NORMAL)
	    s = SKIPSPACE1(s);
	if ((PL_expect != XREF || PL_oldoldbufptr == PL_last_lop) && intuit_more(s)) {
	    if (*s == '{')
		PL_tokenbuf[0] = '%';

	    /* Warn about @ where they meant $. */
	    if (*s == '[' || *s == '{') {
		if (ckWARN(WARN_SYNTAX)) {
		    const char *t = s + 1;
		    while (*t && (isALNUM_lazy_if(t,UTF) || strchr(" \t$#+-'\"", *t)))
			t += UTF ? UTF8SKIP(t) : 1;
		    if (*t == '}' || *t == ']') {
			t++;
			PL_bufptr = PEEKSPACE(PL_bufptr); /* XXX can realloc */
       /* diag_listed_as: Scalar value @%s[%s] better written as $%s[%s] */
			Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
			    "Scalar value %"SVf" better written as $%"SVf,
			    SVfARG(newSVpvn_flags(PL_bufptr, (STRLEN)(t-PL_bufptr),
                                                SVs_TEMP | (UTF ? SVf_UTF8 : 0 ))),
                            SVfARG(newSVpvn_flags(PL_bufptr+1, (STRLEN)(t-PL_bufptr-1),
                                                SVs_TEMP | (UTF ? SVf_UTF8 : 0 ))));
		    }
		}
	    }
	}
	PL_pending_ident = '@';
	TERM('@');

     case '/':			/* may be division, defined-or, or pattern */
	if (PL_expect == XTERMORDORDOR && s[1] == '/') {
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >=
		    (s[2] == '=' ? LEX_FAKEEOF_ASSIGN : LEX_FAKEEOF_LOGIC))
		TOKEN(0);
	    s += 2;
	    AOPERATOR(DORDOR);
	}
     case '?':			/* may either be conditional or pattern */
	if (PL_expect == XOPERATOR) {
	     char tmp = *s++;
	     if(tmp == '?') {
		if (!PL_lex_allbrackets &&
			PL_lex_fakeeof >= LEX_FAKEEOF_IFELSE) {
		    s--;
		    TOKEN(0);
		}
		PL_lex_allbrackets++;
		OPERATOR('?');
	     }
             else {
	         tmp = *s++;
	         if(tmp == '/') {
	             /* A // operator. */
		    if (!PL_lex_allbrackets && PL_lex_fakeeof >=
			    (*s == '=' ? LEX_FAKEEOF_ASSIGN :
					    LEX_FAKEEOF_LOGIC)) {
			s -= 2;
			TOKEN(0);
		    }
	            AOPERATOR(DORDOR);
	         }
	         else {
	             s--;
		     if (*s == '=' && !PL_lex_allbrackets &&
			     PL_lex_fakeeof >= LEX_FAKEEOF_ASSIGN) {
			 s--;
			 TOKEN(0);
		     }
	             Mop(OP_DIVIDE);
	         }
	     }
	 }
	 else {
	     /* Disable warning on "study /blah/" */
	     if (PL_oldoldbufptr == PL_last_uni
	      && (*PL_last_uni != 's' || s - PL_last_uni < 5
	          || memNE(PL_last_uni, "study", 5)
	          || isALNUM_lazy_if(PL_last_uni+5,UTF)
	      ))
	         check_uni();
	     if (*s == '?')
		 deprecate("?PATTERN? without explicit operator");
	     s = scan_pat(s,OP_MATCH);
	     TERM(sublex_start());
	 }

    case '.':
	if (PL_lex_formbrack && PL_lex_brackets == PL_lex_formbrack
#ifdef PERL_STRICT_CR
	    && s[1] == '\n'
#else
	    && (s[1] == '\n' || (s[1] == '\r' && s[2] == '\n'))
#endif
	    && (s == PL_linestart || s[-1] == '\n') )
	{
	    PL_lex_formbrack = 0;
	    PL_expect = XSTATE;
	    goto rightbracket;
	}
	if (PL_expect == XSTATE && s[1] == '.' && s[2] == '.') {
	    s += 3;
	    OPERATOR(YADAYADA);
	}
	if (PL_expect == XOPERATOR || !isDIGIT(s[1])) {
	    char tmp = *s++;
	    if (*s == tmp) {
		if (!PL_lex_allbrackets &&
			PL_lex_fakeeof >= LEX_FAKEEOF_RANGE) {
		    s--;
		    TOKEN(0);
		}
		s++;
		if (*s == tmp) {
		    s++;
		    pl_yylval.ival = OPf_SPECIAL;
		}
		else
		    pl_yylval.ival = 0;
		OPERATOR(DOTDOT);
	    }
	    if (*s == '=' && !PL_lex_allbrackets &&
		    PL_lex_fakeeof >= LEX_FAKEEOF_ASSIGN) {
		s--;
		TOKEN(0);
	    }
	    Aop(OP_CONCAT);
	}
	/* FALL THROUGH */
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
	s = scan_num(s, &pl_yylval);
	DEBUG_T( { printbuf("### Saw number in %s\n", s); } );
	if (PL_expect == XOPERATOR)
	    no_op("Number",s);
	TERM(THING);

    case '\'':
	s = scan_str(s,!!PL_madskills,FALSE);
	DEBUG_T( { printbuf("### Saw string before %s\n", s); } );
	if (PL_expect == XOPERATOR) {
	    if (PL_lex_formbrack && PL_lex_brackets == PL_lex_formbrack) {
		return deprecate_commaless_var_list();
	    }
	    else
		no_op("String",s);
	}
	if (!s)
	    missingterm(NULL);
	pl_yylval.ival = OP_CONST;
	TERM(sublex_start());

    case '"':
	s = scan_str(s,!!PL_madskills,FALSE);
	DEBUG_T( { printbuf("### Saw string before %s\n", s); } );
	if (PL_expect == XOPERATOR) {
	    if (PL_lex_formbrack && PL_lex_brackets == PL_lex_formbrack) {
		return deprecate_commaless_var_list();
	    }
	    else
		no_op("String",s);
	}
	if (!s)
	    missingterm(NULL);
	pl_yylval.ival = OP_CONST;
	/* FIXME. I think that this can be const if char *d is replaced by
	   more localised variables.  */
	for (d = SvPV(PL_lex_stuff, len); len; len--, d++) {
	    if (*d == '$' || *d == '@' || *d == '\\' || !UTF8_IS_INVARIANT((U8)*d)) {
		pl_yylval.ival = OP_STRINGIFY;
		break;
	    }
	}
	TERM(sublex_start());

    case '`':
	s = scan_str(s,!!PL_madskills,FALSE);
	DEBUG_T( { printbuf("### Saw backtick string before %s\n", s); } );
	if (PL_expect == XOPERATOR)
	    no_op("Backticks",s);
	if (!s)
	    missingterm(NULL);
	readpipe_override();
	TERM(sublex_start());

    case '\\':
	s++;
	if (PL_lex_inwhat && isDIGIT(*s))
	    Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),"Can't use \\%c to mean $%c in expression",
			   *s, *s);
	if (PL_expect == XOPERATOR)
	    no_op("Backslash",s);
	OPERATOR(REFGEN);

    case 'v':
	if (isDIGIT(s[1]) && PL_expect != XOPERATOR) {
	    char *start = s + 2;
	    while (isDIGIT(*start) || *start == '_')
		start++;
	    if (*start == '.' && isDIGIT(start[1])) {
		s = scan_num(s, &pl_yylval);
		TERM(THING);
	    }
	    /* avoid v123abc() or $h{v1}, allow C<print v10;> */
	    else if (!isALPHA(*start) && (PL_expect == XTERM
			|| PL_expect == XREF || PL_expect == XSTATE
			|| PL_expect == XTERMORDORDOR)) {
		GV *const gv = gv_fetchpvn_flags(s, start - s,
                                                    UTF ? SVf_UTF8 : 0, SVt_PVCV);
		if (!gv) {
		    s = scan_num(s, &pl_yylval);
		    TERM(THING);
		}
	    }
	}
	goto keylookup;
    case 'x':
	if (isDIGIT(s[1]) && PL_expect == XOPERATOR) {
	    s++;
	    Mop(OP_REPEAT);
	}
	goto keylookup;

    case '_':
    case 'a': case 'A':
    case 'b': case 'B':
    case 'c': case 'C':
    case 'd': case 'D':
    case 'e': case 'E':
    case 'f': case 'F':
    case 'g': case 'G':
    case 'h': case 'H':
    case 'i': case 'I':
    case 'j': case 'J':
    case 'k': case 'K':
    case 'l': case 'L':
    case 'm': case 'M':
    case 'n': case 'N':
    case 'o': case 'O':
    case 'p': case 'P':
    case 'q': case 'Q':
    case 'r': case 'R':
    case 's': case 'S':
    case 't': case 'T':
    case 'u': case 'U':
	      case 'V':
    case 'w': case 'W':
	      case 'X':
    case 'y': case 'Y':
    case 'z': case 'Z':

      keylookup: {
	bool anydelim;
	I32 tmp;

	orig_keyword = 0;
	gv = NULL;
	gvp = NULL;

	PL_bufptr = s;
	s = scan_word(s, PL_tokenbuf, sizeof PL_tokenbuf, FALSE, &len);

	/* Some keywords can be followed by any delimiter, including ':' */
	anydelim = word_takes_any_delimeter(PL_tokenbuf, len);

	/* x::* is just a word, unless x is "CORE" */
	if (!anydelim && *s == ':' && s[1] == ':' && strNE(PL_tokenbuf, "CORE"))
	    goto just_a_word;

	d = s;
	while (d < PL_bufend && isSPACE(*d))
		d++;	/* no comments skipped here, or s### is misparsed */

	/* Is this a word before a => operator? */
	if (*d == '=' && d[1] == '>') {
	    CLINE;
	    pl_yylval.opval
		= (OP*)newSVOP(OP_CONST, 0,
			       S_newSV_maybe_utf8(aTHX_ PL_tokenbuf, len));
	    pl_yylval.opval->op_private = OPpCONST_BARE;
	    TERM(WORD);
	}

	/* Check for plugged-in keyword */
	{
	    OP *o;
	    int result;
	    char *saved_bufptr = PL_bufptr;
	    PL_bufptr = s;
	    result = PL_keyword_plugin(aTHX_ PL_tokenbuf, len, &o);
	    s = PL_bufptr;
	    if (result == KEYWORD_PLUGIN_DECLINE) {
		/* not a plugged-in keyword */
		PL_bufptr = saved_bufptr;
	    } else if (result == KEYWORD_PLUGIN_STMT) {
		pl_yylval.opval = o;
		CLINE;
		PL_expect = XSTATE;
		return REPORT(PLUGSTMT);
	    } else if (result == KEYWORD_PLUGIN_EXPR) {
		pl_yylval.opval = o;
		CLINE;
		PL_expect = XOPERATOR;
		return REPORT(PLUGEXPR);
	    } else {
		Perl_croak(aTHX_ "Bad plugin affecting keyword '%s'",
					PL_tokenbuf);
	    }
	}

	/* Check for built-in keyword */
	tmp = keyword(PL_tokenbuf, len, 0);

	/* Is this a label? */
	if (!anydelim && PL_expect == XSTATE
	      && d < PL_bufend && *d == ':' && *(d + 1) != ':') {
	    s = d + 1;
	    pl_yylval.opval = (OP*)newSVOP(OP_CONST, 0,
                                            newSVpvn_flags(PL_tokenbuf,
                                                        len, UTF ? SVf_UTF8 : 0));
	    CLINE;
	    TOKEN(LABEL);
	}

	if (tmp < 0) {			/* second-class keyword? */
	    GV *ogv = NULL;	/* override (winner) */
	    GV *hgv = NULL;	/* hidden (loser) */
	    if (PL_expect != XOPERATOR && (*s != ':' || s[1] != ':')) {
		CV *cv;
		if ((gv = gv_fetchpvn_flags(PL_tokenbuf, len,
                                            UTF ? SVf_UTF8 : 0, SVt_PVCV)) &&
		    (cv = GvCVu(gv)))
		{
		    if (GvIMPORTED_CV(gv))
			ogv = gv;
		    else if (! CvMETHOD(cv))
			hgv = gv;
		}
		if (!ogv &&
		    (gvp = (GV**)hv_fetch(PL_globalstash, PL_tokenbuf,
                                            UTF ? -(I32)len : (I32)len, FALSE)) &&
		    (gv = *gvp) && isGV_with_GP(gv) &&
		    GvCVu(gv) && GvIMPORTED_CV(gv))
		{
		    ogv = gv;
		}
	    }
	    if (ogv) {
		orig_keyword = tmp;
		tmp = 0;		/* overridden by import or by GLOBAL */
	    }
	    else if (gv && !gvp
		     && -tmp==KEY_lock	/* XXX generalizable kludge */
		     && GvCVu(gv))
	    {
		tmp = 0;		/* any sub overrides "weak" keyword */
	    }
	    else {			/* no override */
		tmp = -tmp;
		if (tmp == KEY_dump) {
		    Perl_ck_warner(aTHX_ packWARN(WARN_MISC),
				   "dump() better written as CORE::dump()");
		}
		gv = NULL;
		gvp = 0;
		if (hgv && tmp != KEY_x && tmp != KEY_CORE)	/* never ambiguous */
		    Perl_ck_warner(aTHX_ packWARN(WARN_AMBIGUOUS),
				   "Ambiguous call resolved as CORE::%s(), "
				   "qualify as such or use &",
				   GvENAME(hgv));
	    }
	}

      reserved_word:
	switch (tmp) {

	default:			/* not a keyword */
	    /* Trade off - by using this evil construction we can pull the
	       variable gv into the block labelled keylookup. If not, then
	       we have to give it function scope so that the goto from the
	       earlier ':' case doesn't bypass the initialisation.  */
	    if (0) {
	    just_a_word_zero_gv:
		gv = NULL;
		gvp = NULL;
		orig_keyword = 0;
	    }
	  just_a_word: {
		SV *sv;
		int pkgname = 0;
		const char lastchar = (PL_bufptr == PL_oldoldbufptr ? 0 : PL_bufptr[-1]);
		OP *rv2cv_op;
		CV *cv;
#ifdef PERL_MAD
		SV *nextPL_nextwhite = 0;
#endif


		/* Get the rest if it looks like a package qualifier */

		if (*s == '\'' || (*s == ':' && s[1] == ':')) {
		    STRLEN morelen;
		    s = scan_word(s, PL_tokenbuf + len, sizeof PL_tokenbuf - len,
				  TRUE, &morelen);
		    if (!morelen)
			Perl_croak(aTHX_ "Bad name after %"SVf"%s",
                                        SVfARG(newSVpvn_flags(PL_tokenbuf, len,
                                            (UTF ? SVf_UTF8 : 0) | SVs_TEMP )),
				*s == '\'' ? "'" : "::");
		    len += morelen;
		    pkgname = 1;
		}

		if (PL_expect == XOPERATOR) {
		    if (PL_bufptr == PL_linestart) {
			CopLINE_dec(PL_curcop);
			Perl_warner(aTHX_ packWARN(WARN_SEMICOLON), "%s", PL_warn_nosemi);
			CopLINE_inc(PL_curcop);
		    }
		    else
			no_op("Bareword",s);
		}

		/* Look for a subroutine with this name in current package,
		   unless name is "Foo::", in which case Foo is a bareword
		   (and a package name). */

		if (len > 2 && !PL_madskills &&
		    PL_tokenbuf[len - 2] == ':' && PL_tokenbuf[len - 1] == ':')
		{
		    if (ckWARN(WARN_BAREWORD)
			&& ! gv_fetchpvn_flags(PL_tokenbuf, len, UTF ? SVf_UTF8 : 0, SVt_PVHV))
			Perl_warner(aTHX_ packWARN(WARN_BAREWORD),
		  	    "Bareword \"%"SVf"\" refers to nonexistent package",
			     SVfARG(newSVpvn_flags(PL_tokenbuf, len,
                                        (UTF ? SVf_UTF8 : 0) | SVs_TEMP)));
		    len -= 2;
		    PL_tokenbuf[len] = '\0';
		    gv = NULL;
		    gvp = 0;
		}
		else {
		    if (!gv) {
			/* Mustn't actually add anything to a symbol table.
			   But also don't want to "initialise" any placeholder
			   constants that might already be there into full
			   blown PVGVs with attached PVCV.  */
			gv = gv_fetchpvn_flags(PL_tokenbuf, len,
					       GV_NOADD_NOINIT | ( UTF ? SVf_UTF8 : 0 ),
					       SVt_PVCV);
		    }
		    len = 0;
		}

		/* if we saw a global override before, get the right name */

		sv = S_newSV_maybe_utf8(aTHX_ PL_tokenbuf,
		    len ? len : strlen(PL_tokenbuf));
		if (gvp) {
		    SV * const tmp_sv = sv;
		    sv = newSVpvs("CORE::GLOBAL::");
		    sv_catsv(sv, tmp_sv);
		    SvREFCNT_dec(tmp_sv);
		}

#ifdef PERL_MAD
		if (PL_madskills && !PL_thistoken) {
		    char *start = SvPVX(PL_linestr) + PL_realtokenstart;
		    PL_thistoken = newSVpvn(start,s - start);
		    PL_realtokenstart = s - SvPVX(PL_linestr);
		}
#endif

		/* Presume this is going to be a bareword of some sort. */
		CLINE;
		pl_yylval.opval = (OP*)newSVOP(OP_CONST, 0, sv);
		pl_yylval.opval->op_private = OPpCONST_BARE;

		/* And if "Foo::", then that's what it certainly is. */
		if (len)
		    goto safe_bareword;

		{
		    OP *const_op = newSVOP(OP_CONST, 0, SvREFCNT_inc_NN(sv));
		    const_op->op_private = OPpCONST_BARE;
		    rv2cv_op = newCVREF(0, const_op);
		}
		cv = rv2cv_op_cv(rv2cv_op, 0);

		/* See if it's the indirect object for a list operator. */

		if (PL_oldoldbufptr &&
		    PL_oldoldbufptr < PL_bufptr &&
		    (PL_oldoldbufptr == PL_last_lop
		     || PL_oldoldbufptr == PL_last_uni) &&
		    /* NO SKIPSPACE BEFORE HERE! */
		    (PL_expect == XREF ||
		     ((PL_opargs[PL_last_lop_op] >> OASHIFT)& 7) == OA_FILEREF))
		{
		    bool immediate_paren = *s == '(';

		    /* (Now we can afford to cross potential line boundary.) */
		    s = SKIPSPACE2(s,nextPL_nextwhite);
#ifdef PERL_MAD
		    PL_nextwhite = nextPL_nextwhite;	/* assume no & deception */
#endif

		    /* Two barewords in a row may indicate method call. */

		    if ((isIDFIRST_lazy_if(s,UTF) || *s == '$') &&
			(tmp = intuit_method(s, gv, cv))) {
			op_free(rv2cv_op);
			if (tmp == METHOD && !PL_lex_allbrackets &&
				PL_lex_fakeeof > LEX_FAKEEOF_LOWLOGIC)
			    PL_lex_fakeeof = LEX_FAKEEOF_LOWLOGIC;
			return REPORT(tmp);
		    }

		    /* If not a declared subroutine, it's an indirect object. */
		    /* (But it's an indir obj regardless for sort.) */
		    /* Also, if "_" follows a filetest operator, it's a bareword */

		    if (
			( !immediate_paren && (PL_last_lop_op == OP_SORT ||
                         (!cv &&
                        (PL_last_lop_op != OP_MAPSTART &&
			 PL_last_lop_op != OP_GREPSTART))))
		       || (PL_tokenbuf[0] == '_' && PL_tokenbuf[1] == '\0'
			    && ((PL_opargs[PL_last_lop_op] & OA_CLASS_MASK) == OA_FILESTATOP))
		       )
		    {
			PL_expect = (PL_last_lop == PL_oldoldbufptr) ? XTERM : XOPERATOR;
			goto bareword;
		    }
		}

		PL_expect = XOPERATOR;
#ifdef PERL_MAD
		if (isSPACE(*s))
		    s = SKIPSPACE2(s,nextPL_nextwhite);
		PL_nextwhite = nextPL_nextwhite;
#else
		s = skipspace(s);
#endif

		/* Is this a word before a => operator? */
		if (*s == '=' && s[1] == '>' && !pkgname) {
		    op_free(rv2cv_op);
		    CLINE;
		    sv_setpv(((SVOP*)pl_yylval.opval)->op_sv, PL_tokenbuf);
		    if (UTF && !IN_BYTES && is_utf8_string((U8*)PL_tokenbuf, len))
		      SvUTF8_on(((SVOP*)pl_yylval.opval)->op_sv);
		    TERM(WORD);
		}

		/* If followed by a paren, it's certainly a subroutine. */
		if (*s == '(') {
		    CLINE;
		    if (cv) {
			d = s + 1;
			while (SPACE_OR_TAB(*d))
			    d++;
			if (*d == ')' && (sv = cv_const_sv(cv))) {
			    s = d + 1;
			    goto its_constant;
			}
		    }
#ifdef PERL_MAD
		    if (PL_madskills) {
			PL_nextwhite = PL_thiswhite;
			PL_thiswhite = 0;
		    }
		    start_force(PL_curforce);
#endif
		    NEXTVAL_NEXTTOKE.opval = pl_yylval.opval;
		    PL_expect = XOPERATOR;
#ifdef PERL_MAD
		    if (PL_madskills) {
			PL_nextwhite = nextPL_nextwhite;
			curmad('X', PL_thistoken);
			PL_thistoken = newSVpvs("");
		    }
#endif
		    op_free(rv2cv_op);
		    force_next(WORD);
		    pl_yylval.ival = 0;
		    TOKEN('&');
		}

		/* If followed by var or block, call it a method (unless sub) */

		if ((*s == '$' || *s == '{') && !cv) {
		    op_free(rv2cv_op);
		    PL_last_lop = PL_oldbufptr;
		    PL_last_lop_op = OP_METHOD;
		    if (!PL_lex_allbrackets &&
			    PL_lex_fakeeof > LEX_FAKEEOF_LOWLOGIC)
			PL_lex_fakeeof = LEX_FAKEEOF_LOWLOGIC;
		    PREBLOCK(METHOD);
		}

		/* If followed by a bareword, see if it looks like indir obj. */

		if (!orig_keyword
			&& (isIDFIRST_lazy_if(s,UTF) || *s == '$')
			&& (tmp = intuit_method(s, gv, cv))) {
		    op_free(rv2cv_op);
		    if (tmp == METHOD && !PL_lex_allbrackets &&
			    PL_lex_fakeeof > LEX_FAKEEOF_LOWLOGIC)
			PL_lex_fakeeof = LEX_FAKEEOF_LOWLOGIC;
		    return REPORT(tmp);
		}

		/* Not a method, so call it a subroutine (if defined) */

		if (cv) {
		    if (lastchar == '-') {
                        const SV *tmpsv = newSVpvn_flags( PL_tokenbuf, len ? len : strlen(PL_tokenbuf), (UTF ? SVf_UTF8 : 0) | SVs_TEMP );
 			Perl_ck_warner_d(aTHX_ packWARN(WARN_AMBIGUOUS),
				"Ambiguous use of -%"SVf" resolved as -&%"SVf"()",
				SVfARG(tmpsv), SVfARG(tmpsv));
                    }
		    /* Check for a constant sub */
		    if ((sv = cv_const_sv(cv))) {
		  its_constant:
			op_free(rv2cv_op);
			SvREFCNT_dec(((SVOP*)pl_yylval.opval)->op_sv);
			((SVOP*)pl_yylval.opval)->op_sv = SvREFCNT_inc_simple(sv);
			pl_yylval.opval->op_private = 0;
			pl_yylval.opval->op_flags |= OPf_SPECIAL;
			TOKEN(WORD);
		    }

		    op_free(pl_yylval.opval);
		    pl_yylval.opval = rv2cv_op;
		    pl_yylval.opval->op_private |= OPpENTERSUB_NOPAREN;
		    PL_last_lop = PL_oldbufptr;
		    PL_last_lop_op = OP_ENTERSUB;
		    /* Is there a prototype? */
		    if (
#ifdef PERL_MAD
			cv &&
#endif
			SvPOK(cv))
		    {
			STRLEN protolen = CvPROTOLEN(cv);
			const char *proto = CvPROTO(cv);
			bool optional;
			if (!protolen)
			    TERM(FUNC0SUB);
			if ((optional = *proto == ';'))
			  do
			    proto++;
			  while (*proto == ';');
			if (
			    (
			        (
			            *proto == '$' || *proto == '_'
			         || *proto == '*' || *proto == '+'
			        )
			     && proto[1] == '\0'
			    )
			 || (
			     *proto == '\\' && proto[1] && proto[2] == '\0'
			    )
			)
			    UNIPROTO(UNIOPSUB,optional);
			if (*proto == '\\' && proto[1] == '[') {
			    const char *p = proto + 2;
			    while(*p && *p != ']')
				++p;
			    if(*p == ']' && !p[1])
				UNIPROTO(UNIOPSUB,optional);
			}
			if (*proto == '&' && *s == '{') {
			    if (PL_curstash)
				sv_setpvs(PL_subname, "__ANON__");
			    else
				sv_setpvs(PL_subname, "__ANON__::__ANON__");
			    if (!PL_lex_allbrackets &&
				    PL_lex_fakeeof > LEX_FAKEEOF_LOWLOGIC)
				PL_lex_fakeeof = LEX_FAKEEOF_LOWLOGIC;
			    PREBLOCK(LSTOPSUB);
			}
		    }
#ifdef PERL_MAD
		    {
			if (PL_madskills) {
			    PL_nextwhite = PL_thiswhite;
			    PL_thiswhite = 0;
			}
			start_force(PL_curforce);
			NEXTVAL_NEXTTOKE.opval = pl_yylval.opval;
			PL_expect = XTERM;
			if (PL_madskills) {
			    PL_nextwhite = nextPL_nextwhite;
			    curmad('X', PL_thistoken);
			    PL_thistoken = newSVpvs("");
			}
			force_next(WORD);
			if (!PL_lex_allbrackets &&
				PL_lex_fakeeof > LEX_FAKEEOF_LOWLOGIC)
			    PL_lex_fakeeof = LEX_FAKEEOF_LOWLOGIC;
			TOKEN(NOAMP);
		    }
		}

		/* Guess harder when madskills require "best effort". */
		if (PL_madskills && (!gv || !GvCVu(gv))) {
		    int probable_sub = 0;
		    if (strchr("\"'`$@%0123456789!*+{[<", *s))
			probable_sub = 1;
		    else if (isALPHA(*s)) {
			char tmpbuf[1024];
			STRLEN tmplen;
			d = s;
			d = scan_word(d, tmpbuf, sizeof tmpbuf, TRUE, &tmplen);
			if (!keyword(tmpbuf, tmplen, 0))
			    probable_sub = 1;
			else {
			    while (d < PL_bufend && isSPACE(*d))
				d++;
			    if (*d == '=' && d[1] == '>')
				probable_sub = 1;
			}
		    }
		    if (probable_sub) {
			gv = gv_fetchpv(PL_tokenbuf, GV_ADD | ( UTF ? SVf_UTF8 : 0 ),
                                        SVt_PVCV);
			op_free(pl_yylval.opval);
			pl_yylval.opval = rv2cv_op;
			pl_yylval.opval->op_private |= OPpENTERSUB_NOPAREN;
			PL_last_lop = PL_oldbufptr;
			PL_last_lop_op = OP_ENTERSUB;
			PL_nextwhite = PL_thiswhite;
			PL_thiswhite = 0;
			start_force(PL_curforce);
			NEXTVAL_NEXTTOKE.opval = pl_yylval.opval;
			PL_expect = XTERM;
			PL_nextwhite = nextPL_nextwhite;
			curmad('X', PL_thistoken);
			PL_thistoken = newSVpvs("");
			force_next(WORD);
			if (!PL_lex_allbrackets &&
				PL_lex_fakeeof > LEX_FAKEEOF_LOWLOGIC)
			    PL_lex_fakeeof = LEX_FAKEEOF_LOWLOGIC;
			TOKEN(NOAMP);
		    }
#else
		    NEXTVAL_NEXTTOKE.opval = pl_yylval.opval;
		    PL_expect = XTERM;
		    force_next(WORD);
		    if (!PL_lex_allbrackets &&
			    PL_lex_fakeeof > LEX_FAKEEOF_LOWLOGIC)
			PL_lex_fakeeof = LEX_FAKEEOF_LOWLOGIC;
		    TOKEN(NOAMP);
#endif
		}

		/* Call it a bare word */

		if (PL_hints & HINT_STRICT_SUBS)
		    pl_yylval.opval->op_private |= OPpCONST_STRICT;
		else {
		bareword:
		    /* after "print" and similar functions (corresponding to
		     * "F? L" in opcode.pl), whatever wasn't already parsed as
		     * a filehandle should be subject to "strict subs".
		     * Likewise for the optional indirect-object argument to system
		     * or exec, which can't be a bareword */
		    if ((PL_last_lop_op == OP_PRINT
			    || PL_last_lop_op == OP_PRTF
			    || PL_last_lop_op == OP_SAY
			    || PL_last_lop_op == OP_SYSTEM
			    || PL_last_lop_op == OP_EXEC)
			    && (PL_hints & HINT_STRICT_SUBS))
			pl_yylval.opval->op_private |= OPpCONST_STRICT;
		    if (lastchar != '-') {
			if (ckWARN(WARN_RESERVED)) {
			    d = PL_tokenbuf;
			    while (isLOWER(*d))
				d++;
			    if (!*d && !gv_stashpv(PL_tokenbuf, UTF ? SVf_UTF8 : 0))
				Perl_warner(aTHX_ packWARN(WARN_RESERVED), PL_warn_reserved,
				       PL_tokenbuf);
			}
		    }
		}
		op_free(rv2cv_op);

	    safe_bareword:
		if ((lastchar == '*' || lastchar == '%' || lastchar == '&')) {
		    Perl_ck_warner_d(aTHX_ packWARN(WARN_AMBIGUOUS),
				     "Operator or semicolon missing before %c%"SVf,
				     lastchar, SVfARG(newSVpvn_flags(PL_tokenbuf,
                                                    strlen(PL_tokenbuf),
                                                    SVs_TEMP | (UTF ? SVf_UTF8 : 0))));
		    Perl_ck_warner_d(aTHX_ packWARN(WARN_AMBIGUOUS),
				     "Ambiguous use of %c resolved as operator %c",
				     lastchar, lastchar);
		}
		TOKEN(WORD);
	    }

	case KEY___FILE__:
	    FUN0OP(
		(OP*)newSVOP(OP_CONST, 0, newSVpv(CopFILE(PL_curcop),0))
	    );

	case KEY___LINE__:
	    FUN0OP(
        	(OP*)newSVOP(OP_CONST, 0,
		    Perl_newSVpvf(aTHX_ "%"IVdf, (IV)CopLINE(PL_curcop)))
	    );

	case KEY___PACKAGE__:
	    FUN0OP(
		(OP*)newSVOP(OP_CONST, 0,
					(PL_curstash
					 ? newSVhek(HvNAME_HEK(PL_curstash))
					 : &PL_sv_undef))
	    );

	case KEY___DATA__:
	case KEY___END__: {
	    GV *gv;
	    if (PL_rsfp && (!PL_in_eval || PL_tokenbuf[2] == 'D')) {
		const char *pname = "main";
		STRLEN plen = 4;
		U32 putf8 = 0;
		if (PL_tokenbuf[2] == 'D')
		{
		    HV * const stash =
			PL_curstash ? PL_curstash : PL_defstash;
		    pname = HvNAME_get(stash);
		    plen  = HvNAMELEN (stash);
		    if(HvNAMEUTF8(stash)) putf8 = SVf_UTF8;
		}
		gv = gv_fetchpvn_flags(
			Perl_form(aTHX_ "%*s::DATA", (int)plen, pname),
			plen+6, GV_ADD|putf8, SVt_PVIO
		);
		GvMULTI_on(gv);
		if (!GvIO(gv))
		    GvIOp(gv) = newIO();
		IoIFP(GvIOp(gv)) = PL_rsfp;
#if defined(HAS_FCNTL) && defined(F_SETFD)
		{
		    const int fd = PerlIO_fileno(PL_rsfp);
		    fcntl(fd,F_SETFD,fd >= 3);
		}
#endif
		/* Mark this internal pseudo-handle as clean */
		IoFLAGS(GvIOp(gv)) |= IOf_UNTAINT;
		if ((PerlIO*)PL_rsfp == PerlIO_stdin())
		    IoTYPE(GvIOp(gv)) = IoTYPE_STD;
		else
		    IoTYPE(GvIOp(gv)) = IoTYPE_RDONLY;
#if defined(WIN32) && !defined(PERL_TEXTMODE_SCRIPTS)
		/* if the script was opened in binmode, we need to revert
		 * it to text mode for compatibility; but only iff it has CRs
		 * XXX this is a questionable hack at best. */
		if (PL_bufend-PL_bufptr > 2
		    && PL_bufend[-1] == '\n' && PL_bufend[-2] == '\r')
		{
		    Off_t loc = 0;
		    if (IoTYPE(GvIOp(gv)) == IoTYPE_RDONLY) {
			loc = PerlIO_tell(PL_rsfp);
			(void)PerlIO_seek(PL_rsfp, 0L, 0);
		    }
#ifdef NETWARE
			if (PerlLIO_setmode(PL_rsfp, O_TEXT) != -1) {
#else
		    if (PerlLIO_setmode(PerlIO_fileno(PL_rsfp), O_TEXT) != -1) {
#endif	/* NETWARE */
			if (loc > 0)
			    PerlIO_seek(PL_rsfp, loc, 0);
		    }
		}
#endif
#ifdef PERLIO_LAYERS
		if (!IN_BYTES) {
		    if (UTF)
			PerlIO_apply_layers(aTHX_ PL_rsfp, NULL, ":utf8");
		    else if (PL_encoding) {
			SV *name;
			dSP;
			ENTER;
			SAVETMPS;
			PUSHMARK(sp);
			EXTEND(SP, 1);
			XPUSHs(PL_encoding);
			PUTBACK;
			call_method("name", G_SCALAR);
			SPAGAIN;
			name = POPs;
			PUTBACK;
			PerlIO_apply_layers(aTHX_ PL_rsfp, NULL,
					    Perl_form(aTHX_ ":encoding(%"SVf")",
						      SVfARG(name)));
			FREETMPS;
			LEAVE;
		    }
		}
#endif
#ifdef PERL_MAD
		if (PL_madskills) {
		    if (PL_realtokenstart >= 0) {
			char *tstart = SvPVX(PL_linestr) + PL_realtokenstart;
			if (!PL_endwhite)
			    PL_endwhite = newSVpvs("");
			sv_catsv(PL_endwhite, PL_thiswhite);
			PL_thiswhite = 0;
			sv_catpvn(PL_endwhite, tstart, PL_bufend - tstart);
			PL_realtokenstart = -1;
		    }
		    while ((s = filter_gets(PL_endwhite, SvCUR(PL_endwhite)))
			   != NULL) ;
		}
#endif
		PL_rsfp = NULL;
	    }
	    goto fake_eof;
	}

	case KEY___SUB__:
	    FUN0OP(newPVOP(OP_RUNCV,0,NULL));

	case KEY_AUTOLOAD:
	case KEY_DESTROY:
	case KEY_BEGIN:
	case KEY_UNITCHECK:
	case KEY_CHECK:
	case KEY_INIT:
	case KEY_END:
	    if (PL_expect == XSTATE) {
		s = PL_bufptr;
		goto really_sub;
	    }
	    goto just_a_word;

	case KEY_CORE:
	    if (*s == ':' && s[1] == ':') {
		s += 2;
		d = s;
		s = scan_word(s, PL_tokenbuf, sizeof PL_tokenbuf, FALSE, &len);
		if (!(tmp = keyword(PL_tokenbuf, len, 1)))
		    Perl_croak(aTHX_ "CORE::%"SVf" is not a keyword",
                                    SVfARG(newSVpvn_flags(PL_tokenbuf, len,
                                                (UTF ? SVf_UTF8 : 0) | SVs_TEMP)));
		if (tmp < 0)
		    tmp = -tmp;
		else if (tmp == KEY_require || tmp == KEY_do
		      || tmp == KEY_glob)
		    /* that's a way to remember we saw "CORE::" */
		    orig_keyword = tmp;
		goto reserved_word;
	    }
	    goto just_a_word;

	case KEY_abs:
	    UNI(OP_ABS);

	case KEY_alarm:
	    UNI(OP_ALARM);

	case KEY_accept:
	    LOP(OP_ACCEPT,XTERM);

	case KEY_and:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_LOWLOGIC)
		return REPORT(0);
	    OPERATOR(ANDOP);

	case KEY_atan2:
	    LOP(OP_ATAN2,XTERM);

	case KEY_bind:
	    LOP(OP_BIND,XTERM);

	case KEY_binmode:
	    LOP(OP_BINMODE,XTERM);

	case KEY_bless:
	    LOP(OP_BLESS,XTERM);

	case KEY_break:
	    FUN0(OP_BREAK);

	case KEY_chop:
	    UNI(OP_CHOP);

	case KEY_continue:
		    /* We have to disambiguate the two senses of
		      "continue". If the next token is a '{' then
		      treat it as the start of a continue block;
		      otherwise treat it as a control operator.
		     */
		    s = skipspace(s);
		    if (*s == '{')
	    PREBLOCK(CONTINUE);
		    else
			FUN0(OP_CONTINUE);

	case KEY_chdir:
	    /* may use HOME */
	    (void)gv_fetchpvs("ENV", GV_ADD|GV_NOTQUAL, SVt_PVHV);
	    UNI(OP_CHDIR);

	case KEY_close:
	    UNI(OP_CLOSE);

	case KEY_closedir:
	    UNI(OP_CLOSEDIR);

	case KEY_cmp:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE)
		return REPORT(0);
	    Eop(OP_SCMP);

	case KEY_caller:
	    UNI(OP_CALLER);

	case KEY_crypt:
#ifdef FCRYPT
	    if (!PL_cryptseen) {
		PL_cryptseen = TRUE;
		init_des();
	    }
#endif
	    LOP(OP_CRYPT,XTERM);

	case KEY_chmod:
	    LOP(OP_CHMOD,XTERM);

	case KEY_chown:
	    LOP(OP_CHOWN,XTERM);

	case KEY_connect:
	    LOP(OP_CONNECT,XTERM);

	case KEY_chr:
	    UNI(OP_CHR);

	case KEY_cos:
	    UNI(OP_COS);

	case KEY_chroot:
	    UNI(OP_CHROOT);

	case KEY_default:
	    PREBLOCK(DEFAULT);

	case KEY_do:
	    s = SKIPSPACE1(s);
	    if (*s == '{')
		PRETERMBLOCK(DO);
	    if (*s != '\'')
		s = force_word(s,WORD,TRUE,TRUE,FALSE);
	    if (orig_keyword == KEY_do) {
		orig_keyword = 0;
		pl_yylval.ival = 1;
	    }
	    else
		pl_yylval.ival = 0;
	    OPERATOR(DO);

	case KEY_die:
	    PL_hints |= HINT_BLOCK_SCOPE;
	    LOP(OP_DIE,XTERM);

	case KEY_defined:
	    UNI(OP_DEFINED);

	case KEY_delete:
	    UNI(OP_DELETE);

	case KEY_dbmopen:
	    Perl_populate_isa(aTHX_ STR_WITH_LEN("AnyDBM_File::ISA"),
			      STR_WITH_LEN("NDBM_File::"),
			      STR_WITH_LEN("DB_File::"),
			      STR_WITH_LEN("GDBM_File::"),
			      STR_WITH_LEN("SDBM_File::"),
			      STR_WITH_LEN("ODBM_File::"),
			      NULL);
	    LOP(OP_DBMOPEN,XTERM);

	case KEY_dbmclose:
	    UNI(OP_DBMCLOSE);

	case KEY_dump:
	    s = force_word(s,WORD,TRUE,FALSE,FALSE);
	    LOOPX(OP_DUMP);

	case KEY_else:
	    PREBLOCK(ELSE);

	case KEY_elsif:
	    pl_yylval.ival = CopLINE(PL_curcop);
	    OPERATOR(ELSIF);

	case KEY_eq:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE)
		return REPORT(0);
	    Eop(OP_SEQ);

	case KEY_exists:
	    UNI(OP_EXISTS);
	
	case KEY_exit:
	    if (PL_madskills)
		UNI(OP_INT);
	    UNI(OP_EXIT);

	case KEY_eval:
	    s = SKIPSPACE1(s);
	    if (*s == '{') { /* block eval */
		PL_expect = XTERMBLOCK;
		UNIBRACK(OP_ENTERTRY);
	    }
	    else { /* string eval */
		PL_expect = XTERM;
		UNIBRACK(OP_ENTEREVAL);
	    }

	case KEY_evalbytes:
	    PL_expect = XTERM;
	    UNIBRACK(-OP_ENTEREVAL);

	case KEY_eof:
	    UNI(OP_EOF);

	case KEY_exp:
	    UNI(OP_EXP);

	case KEY_each:
	    UNI(OP_EACH);

	case KEY_exec:
	    LOP(OP_EXEC,XREF);

	case KEY_endhostent:
	    FUN0(OP_EHOSTENT);

	case KEY_endnetent:
	    FUN0(OP_ENETENT);

	case KEY_endservent:
	    FUN0(OP_ESERVENT);

	case KEY_endprotoent:
	    FUN0(OP_EPROTOENT);

	case KEY_endpwent:
	    FUN0(OP_EPWENT);

	case KEY_endgrent:
	    FUN0(OP_EGRENT);

	case KEY_for:
	case KEY_foreach:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_NONEXPR)
		return REPORT(0);
	    pl_yylval.ival = CopLINE(PL_curcop);
	    s = SKIPSPACE1(s);
	    if (PL_expect == XSTATE && isIDFIRST_lazy_if(s,UTF)) {
		char *p = s;
#ifdef PERL_MAD
		int soff = s - SvPVX(PL_linestr); /* for skipspace realloc */
#endif

		if ((PL_bufend - p) >= 3 &&
		    strnEQ(p, "my", 2) && isSPACE(*(p + 2)))
		    p += 2;
		else if ((PL_bufend - p) >= 4 &&
		    strnEQ(p, "our", 3) && isSPACE(*(p + 3)))
		    p += 3;
		p = PEEKSPACE(p);
		if (isIDFIRST_lazy_if(p,UTF)) {
		    p = scan_ident(p, PL_bufend,
			PL_tokenbuf, sizeof PL_tokenbuf, TRUE);
		    p = PEEKSPACE(p);
		}
		if (*p != '$')
		    Perl_croak(aTHX_ "Missing $ on loop variable");
#ifdef PERL_MAD
		s = SvPVX(PL_linestr) + soff;
#endif
	    }
	    OPERATOR(FOR);

	case KEY_formline:
	    LOP(OP_FORMLINE,XTERM);

	case KEY_fork:
	    FUN0(OP_FORK);

	case KEY_fc:
	    UNI(OP_FC);

	case KEY_fcntl:
	    LOP(OP_FCNTL,XTERM);

	case KEY_fileno:
	    UNI(OP_FILENO);

	case KEY_flock:
	    LOP(OP_FLOCK,XTERM);

	case KEY_gt:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE)
		return REPORT(0);
	    Rop(OP_SGT);

	case KEY_ge:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE)
		return REPORT(0);
	    Rop(OP_SGE);

	case KEY_grep:
	    LOP(OP_GREPSTART, XREF);

	case KEY_goto:
	    s = force_word(s,WORD,TRUE,FALSE,FALSE);
	    LOOPX(OP_GOTO);

	case KEY_gmtime:
	    UNI(OP_GMTIME);

	case KEY_getc:
	    UNIDOR(OP_GETC);

	case KEY_getppid:
	    FUN0(OP_GETPPID);

	case KEY_getpgrp:
	    UNI(OP_GETPGRP);

	case KEY_getpriority:
	    LOP(OP_GETPRIORITY,XTERM);

	case KEY_getprotobyname:
	    UNI(OP_GPBYNAME);

	case KEY_getprotobynumber:
	    LOP(OP_GPBYNUMBER,XTERM);

	case KEY_getprotoent:
	    FUN0(OP_GPROTOENT);

	case KEY_getpwent:
	    FUN0(OP_GPWENT);

	case KEY_getpwnam:
	    UNI(OP_GPWNAM);

	case KEY_getpwuid:
	    UNI(OP_GPWUID);

	case KEY_getpeername:
	    UNI(OP_GETPEERNAME);

	case KEY_gethostbyname:
	    UNI(OP_GHBYNAME);

	case KEY_gethostbyaddr:
	    LOP(OP_GHBYADDR,XTERM);

	case KEY_gethostent:
	    FUN0(OP_GHOSTENT);

	case KEY_getnetbyname:
	    UNI(OP_GNBYNAME);

	case KEY_getnetbyaddr:
	    LOP(OP_GNBYADDR,XTERM);

	case KEY_getnetent:
	    FUN0(OP_GNETENT);

	case KEY_getservbyname:
	    LOP(OP_GSBYNAME,XTERM);

	case KEY_getservbyport:
	    LOP(OP_GSBYPORT,XTERM);

	case KEY_getservent:
	    FUN0(OP_GSERVENT);

	case KEY_getsockname:
	    UNI(OP_GETSOCKNAME);

	case KEY_getsockopt:
	    LOP(OP_GSOCKOPT,XTERM);

	case KEY_getgrent:
	    FUN0(OP_GGRENT);

	case KEY_getgrnam:
	    UNI(OP_GGRNAM);

	case KEY_getgrgid:
	    UNI(OP_GGRGID);

	case KEY_getlogin:
	    FUN0(OP_GETLOGIN);

	case KEY_given:
	    pl_yylval.ival = CopLINE(PL_curcop);
	    OPERATOR(GIVEN);

	case KEY_glob:
	    LOP(
	     orig_keyword==KEY_glob ? (orig_keyword=0, -OP_GLOB) : OP_GLOB,
	     XTERM
	    );

	case KEY_hex:
	    UNI(OP_HEX);

	case KEY_if:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_NONEXPR)
		return REPORT(0);
	    pl_yylval.ival = CopLINE(PL_curcop);
	    OPERATOR(IF);

	case KEY_index:
	    LOP(OP_INDEX,XTERM);

	case KEY_int:
	    UNI(OP_INT);

	case KEY_ioctl:
	    LOP(OP_IOCTL,XTERM);

	case KEY_join:
	    LOP(OP_JOIN,XTERM);

	case KEY_keys:
	    UNI(OP_KEYS);

	case KEY_kill:
	    LOP(OP_KILL,XTERM);

	case KEY_last:
	    s = force_word(s,WORD,TRUE,FALSE,FALSE);
	    LOOPX(OP_LAST);
	
	case KEY_lc:
	    UNI(OP_LC);

	case KEY_lcfirst:
	    UNI(OP_LCFIRST);

	case KEY_local:
	    pl_yylval.ival = 0;
	    OPERATOR(LOCAL);

	case KEY_length:
	    UNI(OP_LENGTH);

	case KEY_lt:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE)
		return REPORT(0);
	    Rop(OP_SLT);

	case KEY_le:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE)
		return REPORT(0);
	    Rop(OP_SLE);

	case KEY_localtime:
	    UNI(OP_LOCALTIME);

	case KEY_log:
	    UNI(OP_LOG);

	case KEY_link:
	    LOP(OP_LINK,XTERM);

	case KEY_listen:
	    LOP(OP_LISTEN,XTERM);

	case KEY_lock:
	    UNI(OP_LOCK);

	case KEY_lstat:
	    UNI(OP_LSTAT);

	case KEY_m:
	    s = scan_pat(s,OP_MATCH);
	    TERM(sublex_start());

	case KEY_map:
	    LOP(OP_MAPSTART, XREF);

	case KEY_mkdir:
	    LOP(OP_MKDIR,XTERM);

	case KEY_msgctl:
	    LOP(OP_MSGCTL,XTERM);

	case KEY_msgget:
	    LOP(OP_MSGGET,XTERM);

	case KEY_msgrcv:
	    LOP(OP_MSGRCV,XTERM);

	case KEY_msgsnd:
	    LOP(OP_MSGSND,XTERM);

	case KEY_our:
	case KEY_my:
	case KEY_state:
	    PL_in_my = (U16)tmp;
	    s = SKIPSPACE1(s);
	    if (isIDFIRST_lazy_if(s,UTF)) {
#ifdef PERL_MAD
		char* start = s;
#endif
		s = scan_word(s, PL_tokenbuf, sizeof PL_tokenbuf, TRUE, &len);
		if (len == 3 && strnEQ(PL_tokenbuf, "sub", 3))
		    goto really_sub;
		PL_in_my_stash = find_in_my_stash(PL_tokenbuf, len);
		if (!PL_in_my_stash) {
		    char tmpbuf[1024];
		    PL_bufptr = s;
		    my_snprintf(tmpbuf, sizeof(tmpbuf), "No such class %.1000s", PL_tokenbuf);
		    yyerror_pv(tmpbuf, UTF ? SVf_UTF8 : 0);
		}
#ifdef PERL_MAD
		if (PL_madskills) {	/* just add type to declarator token */
		    sv_catsv(PL_thistoken, PL_nextwhite);
		    PL_nextwhite = 0;
		    sv_catpvn(PL_thistoken, start, s - start);
		}
#endif
	    }
	    pl_yylval.ival = 1;
	    OPERATOR(MY);

	case KEY_next:
	    s = force_word(s,WORD,TRUE,FALSE,FALSE);
	    LOOPX(OP_NEXT);

	case KEY_ne:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_COMPARE)
		return REPORT(0);
	    Eop(OP_SNE);

	case KEY_no:
	    s = tokenize_use(0, s);
	    OPERATOR(USE);

	case KEY_not:
	    if (*s == '(' || (s = SKIPSPACE1(s), *s == '('))
		FUN1(OP_NOT);
	    else {
		if (!PL_lex_allbrackets &&
			PL_lex_fakeeof > LEX_FAKEEOF_LOWLOGIC)
		    PL_lex_fakeeof = LEX_FAKEEOF_LOWLOGIC;
		OPERATOR(NOTOP);
	    }

	case KEY_open:
	    s = SKIPSPACE1(s);
	    if (isIDFIRST_lazy_if(s,UTF)) {
		const char *t;
		for (d = s; isALNUM_lazy_if(d,UTF);) {
		    d += UTF ? UTF8SKIP(d) : 1;
                    if (UTF) {
                        while (UTF8_IS_CONTINUED(*d) && is_utf8_mark((U8*)d)) {
                            d += UTF ? UTF8SKIP(d) : 1;
                        }
                    }
                }
		for (t=d; isSPACE(*t);)
		    t++;
		if ( *t && strchr("|&*+-=!?:.", *t) && ckWARN_d(WARN_PRECEDENCE)
		    /* [perl #16184] */
		    && !(t[0] == '=' && t[1] == '>')
		    && !(t[0] == ':' && t[1] == ':')
		    && !keyword(s, d-s, 0)
		) {
		    SV *tmpsv = newSVpvn_flags(s, (STRLEN)(d-s),
                                                SVs_TEMP | (UTF ? SVf_UTF8 : 0));
		    Perl_warner(aTHX_ packWARN(WARN_PRECEDENCE),
			   "Precedence problem: open %"SVf" should be open(%"SVf")",
			    SVfARG(tmpsv), SVfARG(tmpsv));
		}
	    }
	    LOP(OP_OPEN,XTERM);

	case KEY_or:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_LOWLOGIC)
		return REPORT(0);
	    pl_yylval.ival = OP_OR;
	    OPERATOR(OROP);

	case KEY_ord:
	    UNI(OP_ORD);

	case KEY_oct:
	    UNI(OP_OCT);

	case KEY_opendir:
	    LOP(OP_OPEN_DIR,XTERM);

	case KEY_print:
	    checkcomma(s,PL_tokenbuf,"filehandle");
	    LOP(OP_PRINT,XREF);

	case KEY_printf:
	    checkcomma(s,PL_tokenbuf,"filehandle");
	    LOP(OP_PRTF,XREF);

	case KEY_prototype:
	    UNI(OP_PROTOTYPE);

	case KEY_push:
	    LOP(OP_PUSH,XTERM);

	case KEY_pop:
	    UNIDOR(OP_POP);

	case KEY_pos:
	    UNIDOR(OP_POS);
	
	case KEY_pack:
	    LOP(OP_PACK,XTERM);

	case KEY_package:
	    s = force_word(s,WORD,FALSE,TRUE,FALSE);
	    s = SKIPSPACE1(s);
	    s = force_strict_version(s);
	    PL_lex_expect = XBLOCK;
	    OPERATOR(PACKAGE);

	case KEY_pipe:
	    LOP(OP_PIPE_OP,XTERM);

	case KEY_q:
	    s = scan_str(s,!!PL_madskills,FALSE);
	    if (!s)
		missingterm(NULL);
	    pl_yylval.ival = OP_CONST;
	    TERM(sublex_start());

	case KEY_quotemeta:
	    UNI(OP_QUOTEMETA);

	case KEY_qw: {
	    OP *words = NULL;
	    s = scan_str(s,!!PL_madskills,FALSE);
	    if (!s)
		missingterm(NULL);
	    PL_expect = XOPERATOR;
	    if (SvCUR(PL_lex_stuff)) {
		int warned_comma = !ckWARN(WARN_QW);
		int warned_comment = warned_comma;
		d = SvPV_force(PL_lex_stuff, len);
		while (len) {
		    for (; isSPACE(*d) && len; --len, ++d)
			/**/;
		    if (len) {
			SV *sv;
			const char *b = d;
			if (!warned_comma || !warned_comment) {
			    for (; !isSPACE(*d) && len; --len, ++d) {
				if (!warned_comma && *d == ',') {
				    Perl_warner(aTHX_ packWARN(WARN_QW),
					"Possible attempt to separate words with commas");
				    ++warned_comma;
				}
				else if (!warned_comment && *d == '#') {
				    Perl_warner(aTHX_ packWARN(WARN_QW),
					"Possible attempt to put comments in qw() list");
				    ++warned_comment;
				}
			    }
			}
			else {
			    for (; !isSPACE(*d) && len; --len, ++d)
				/**/;
			}
			sv = newSVpvn_utf8(b, d-b, DO_UTF8(PL_lex_stuff));
			words = op_append_elem(OP_LIST, words,
					    newSVOP(OP_CONST, 0, tokeq(sv)));
		    }
		}
	    }
	    if (!words)
		words = newNULLLIST();
	    if (PL_lex_stuff) {
		SvREFCNT_dec(PL_lex_stuff);
		PL_lex_stuff = NULL;
	    }
	    PL_expect = XOPERATOR;
	    pl_yylval.opval = sawparens(words);
	    TOKEN(QWLIST);
	}

	case KEY_qq:
	    s = scan_str(s,!!PL_madskills,FALSE);
	    if (!s)
		missingterm(NULL);
	    pl_yylval.ival = OP_STRINGIFY;
	    if (SvIVX(PL_lex_stuff) == '\'')
		SvIV_set(PL_lex_stuff, 0);	/* qq'$foo' should interpolate */
	    TERM(sublex_start());

	case KEY_qr:
	    s = scan_pat(s,OP_QR);
	    TERM(sublex_start());

	case KEY_qx:
	    s = scan_str(s,!!PL_madskills,FALSE);
	    if (!s)
		missingterm(NULL);
	    readpipe_override();
	    TERM(sublex_start());

	case KEY_return:
	    OLDLOP(OP_RETURN);

	case KEY_require:
	    s = SKIPSPACE1(s);
	    if (isDIGIT(*s)) {
		s = force_version(s, FALSE);
	    }
	    else if (*s != 'v' || !isDIGIT(s[1])
		    || (s = force_version(s, TRUE), *s == 'v'))
	    {
		*PL_tokenbuf = '\0';
		s = force_word(s,WORD,TRUE,TRUE,FALSE);
		if (isIDFIRST_lazy_if(PL_tokenbuf,UTF))
		    gv_stashpvn(PL_tokenbuf, strlen(PL_tokenbuf),
                                GV_ADD | (UTF ? SVf_UTF8 : 0));
		else if (*s == '<')
		    yyerror("<> should be quotes");
	    }
	    if (orig_keyword == KEY_require) {
		orig_keyword = 0;
		pl_yylval.ival = 1;
	    }
	    else 
		pl_yylval.ival = 0;
	    PL_expect = XTERM;
	    PL_bufptr = s;
	    PL_last_uni = PL_oldbufptr;
	    PL_last_lop_op = OP_REQUIRE;
	    s = skipspace(s);
	    return REPORT( (int)REQUIRE );

	case KEY_reset:
	    UNI(OP_RESET);

	case KEY_redo:
	    s = force_word(s,WORD,TRUE,FALSE,FALSE);
	    LOOPX(OP_REDO);

	case KEY_rename:
	    LOP(OP_RENAME,XTERM);

	case KEY_rand:
	    UNI(OP_RAND);

	case KEY_rmdir:
	    UNI(OP_RMDIR);

	case KEY_rindex:
	    LOP(OP_RINDEX,XTERM);

	case KEY_read:
	    LOP(OP_READ,XTERM);

	case KEY_readdir:
	    UNI(OP_READDIR);

	case KEY_readline:
	    UNIDOR(OP_READLINE);

	case KEY_readpipe:
	    UNIDOR(OP_BACKTICK);

	case KEY_rewinddir:
	    UNI(OP_REWINDDIR);

	case KEY_recv:
	    LOP(OP_RECV,XTERM);

	case KEY_reverse:
	    LOP(OP_REVERSE,XTERM);

	case KEY_readlink:
	    UNIDOR(OP_READLINK);

	case KEY_ref:
	    UNI(OP_REF);

	case KEY_s:
	    s = scan_subst(s);
	    if (pl_yylval.opval)
		TERM(sublex_start());
	    else
		TOKEN(1);	/* force error */

	case KEY_say:
	    checkcomma(s,PL_tokenbuf,"filehandle");
	    LOP(OP_SAY,XREF);

	case KEY_chomp:
	    UNI(OP_CHOMP);
	
	case KEY_scalar:
	    UNI(OP_SCALAR);

	case KEY_select:
	    LOP(OP_SELECT,XTERM);

	case KEY_seek:
	    LOP(OP_SEEK,XTERM);

	case KEY_semctl:
	    LOP(OP_SEMCTL,XTERM);

	case KEY_semget:
	    LOP(OP_SEMGET,XTERM);

	case KEY_semop:
	    LOP(OP_SEMOP,XTERM);

	case KEY_send:
	    LOP(OP_SEND,XTERM);

	case KEY_setpgrp:
	    LOP(OP_SETPGRP,XTERM);

	case KEY_setpriority:
	    LOP(OP_SETPRIORITY,XTERM);

	case KEY_sethostent:
	    UNI(OP_SHOSTENT);

	case KEY_setnetent:
	    UNI(OP_SNETENT);

	case KEY_setservent:
	    UNI(OP_SSERVENT);

	case KEY_setprotoent:
	    UNI(OP_SPROTOENT);

	case KEY_setpwent:
	    FUN0(OP_SPWENT);

	case KEY_setgrent:
	    FUN0(OP_SGRENT);

	case KEY_seekdir:
	    LOP(OP_SEEKDIR,XTERM);

	case KEY_setsockopt:
	    LOP(OP_SSOCKOPT,XTERM);

	case KEY_shift:
	    UNIDOR(OP_SHIFT);

	case KEY_shmctl:
	    LOP(OP_SHMCTL,XTERM);

	case KEY_shmget:
	    LOP(OP_SHMGET,XTERM);

	case KEY_shmread:
	    LOP(OP_SHMREAD,XTERM);

	case KEY_shmwrite:
	    LOP(OP_SHMWRITE,XTERM);

	case KEY_shutdown:
	    LOP(OP_SHUTDOWN,XTERM);

	case KEY_sin:
	    UNI(OP_SIN);

	case KEY_sleep:
	    UNI(OP_SLEEP);

	case KEY_socket:
	    LOP(OP_SOCKET,XTERM);

	case KEY_socketpair:
	    LOP(OP_SOCKPAIR,XTERM);

	case KEY_sort:
	    checkcomma(s,PL_tokenbuf,"subroutine name");
	    s = SKIPSPACE1(s);
	    PL_expect = XTERM;
	    s = force_word(s,WORD,TRUE,TRUE,FALSE);
	    LOP(OP_SORT,XREF);

	case KEY_split:
	    LOP(OP_SPLIT,XTERM);

	case KEY_sprintf:
	    LOP(OP_SPRINTF,XTERM);

	case KEY_splice:
	    LOP(OP_SPLICE,XTERM);

	case KEY_sqrt:
	    UNI(OP_SQRT);

	case KEY_srand:
	    UNI(OP_SRAND);

	case KEY_stat:
	    UNI(OP_STAT);

	case KEY_study:
	    UNI(OP_STUDY);

	case KEY_substr:
	    LOP(OP_SUBSTR,XTERM);

	case KEY_format:
	case KEY_sub:
	  really_sub:
	    {
		char tmpbuf[sizeof PL_tokenbuf];
		SSize_t tboffset = 0;
		expectation attrful;
		bool have_name, have_proto;
		const int key = tmp;

#ifdef PERL_MAD
		SV *tmpwhite = 0;

		char *tstart = SvPVX(PL_linestr) + PL_realtokenstart;
		SV *subtoken = newSVpvn_flags(tstart, s - tstart, SvUTF8(PL_linestr));
		PL_thistoken = 0;

		d = s;
		s = SKIPSPACE2(s,tmpwhite);
#else
		s = skipspace(s);
#endif

		if (isIDFIRST_lazy_if(s,UTF) || *s == '\'' ||
		    (*s == ':' && s[1] == ':'))
		{
#ifdef PERL_MAD
		    SV *nametoke = NULL;
#endif

		    PL_expect = XBLOCK;
		    attrful = XATTRBLOCK;
		    /* remember buffer pos'n for later force_word */
		    tboffset = s - PL_oldbufptr;
		    d = scan_word(s, tmpbuf, sizeof tmpbuf, TRUE, &len);
#ifdef PERL_MAD
		    if (PL_madskills)
			nametoke = newSVpvn_flags(s, d - s, SvUTF8(PL_linestr));
#endif
		    if (memchr(tmpbuf, ':', len))
			sv_setpvn(PL_subname, tmpbuf, len);
		    else {
			sv_setsv(PL_subname,PL_curstname);
			sv_catpvs(PL_subname,"::");
			sv_catpvn(PL_subname,tmpbuf,len);
		    }
                    if (SvUTF8(PL_linestr))
                        SvUTF8_on(PL_subname);
		    have_name = TRUE;

#ifdef PERL_MAD

		    start_force(0);
		    CURMAD('X', nametoke);
		    CURMAD('_', tmpwhite);
		    (void) force_word(PL_oldbufptr + tboffset, WORD,
				      FALSE, TRUE, TRUE);

		    s = SKIPSPACE2(d,tmpwhite);
#else
		    s = skipspace(d);
#endif
		}
		else {
		    if (key == KEY_my)
			Perl_croak(aTHX_ "Missing name in \"my sub\"");
		    PL_expect = XTERMBLOCK;
		    attrful = XATTRTERM;
		    sv_setpvs(PL_subname,"?");
		    have_name = FALSE;
		}

		if (key == KEY_format) {
		    if (*s == '=')
			PL_lex_formbrack = PL_lex_brackets + 1;
#ifdef PERL_MAD
		    PL_thistoken = subtoken;
		    s = d;
#else
		    if (have_name)
			(void) force_word(PL_oldbufptr + tboffset, WORD,
					  FALSE, TRUE, TRUE);
#endif
		    OPERATOR(FORMAT);
		}

		/* Look for a prototype */
		if (*s == '(') {
		    char *p;
		    bool bad_proto = FALSE;
		    bool in_brackets = FALSE;
		    char greedy_proto = ' ';
		    bool proto_after_greedy_proto = FALSE;
		    bool must_be_last = FALSE;
		    bool underscore = FALSE;
		    bool seen_underscore = FALSE;
		    const bool warnillegalproto = ckWARN(WARN_ILLEGALPROTO);
                    STRLEN tmplen;

		    s = scan_str(s,!!PL_madskills,FALSE);
		    if (!s)
			Perl_croak(aTHX_ "Prototype not terminated");
		    /* strip spaces and check for bad characters */
		    d = SvPV(PL_lex_stuff, tmplen);
		    tmp = 0;
		    for (p = d; tmplen; tmplen--, ++p) {
			if (!isSPACE(*p)) {
                            d[tmp++] = *p;

			    if (warnillegalproto) {
				if (must_be_last)
				    proto_after_greedy_proto = TRUE;
				if (!strchr("$@%*;[]&\\_+", *p) || *p == '\0') {
				    bad_proto = TRUE;
				}
				else {
				    if ( underscore ) {
					if ( !strchr(";@%", *p) )
					    bad_proto = TRUE;
					underscore = FALSE;
				    }
				    if ( *p == '[' ) {
					in_brackets = TRUE;
				    }
				    else if ( *p == ']' ) {
					in_brackets = FALSE;
				    }
				    else if ( (*p == '@' || *p == '%') &&
					 ( tmp < 2 || d[tmp-2] != '\\' ) &&
					 !in_brackets ) {
					must_be_last = TRUE;
					greedy_proto = *p;
				    }
				    else if ( *p == '_' ) {
					underscore = seen_underscore = TRUE;
				    }
				}
			    }
			}
		    }
                    d[tmp] = '\0';
		    if (proto_after_greedy_proto)
			Perl_warner(aTHX_ packWARN(WARN_ILLEGALPROTO),
				    "Prototype after '%c' for %"SVf" : %s",
				    greedy_proto, SVfARG(PL_subname), d);
		    if (bad_proto) {
                        SV *dsv = newSVpvs_flags("", SVs_TEMP);
			Perl_warner(aTHX_ packWARN(WARN_ILLEGALPROTO),
				    "Illegal character %sin prototype for %"SVf" : %s",
				    seen_underscore ? "after '_' " : "",
				    SVfARG(PL_subname),
                                    SvUTF8(PL_lex_stuff)
                                        ? sv_uni_display(dsv,
                                            newSVpvn_flags(d, tmp, SVs_TEMP | SVf_UTF8),
                                            tmp,
                                            UNI_DISPLAY_ISPRINT)
                                        : pv_pretty(dsv, d, tmp, 60, NULL, NULL,
                                            PERL_PV_ESCAPE_NONASCII));
                    }
                    SvCUR_set(PL_lex_stuff, tmp);
		    have_proto = TRUE;

#ifdef PERL_MAD
		    start_force(0);
		    CURMAD('q', PL_thisopen);
		    CURMAD('_', tmpwhite);
		    CURMAD('=', PL_thisstuff);
		    CURMAD('Q', PL_thisclose);
		    NEXTVAL_NEXTTOKE.opval =
			(OP*)newSVOP(OP_CONST, 0, PL_lex_stuff);
		    PL_lex_stuff = NULL;
		    force_next(THING);

		    s = SKIPSPACE2(s,tmpwhite);
#else
		    s = skipspace(s);
#endif
		}
		else
		    have_proto = FALSE;

		if (*s == ':' && s[1] != ':')
		    PL_expect = attrful;
		else if (*s != '{' && key == KEY_sub) {
		    if (!have_name)
			Perl_croak(aTHX_ "Illegal declaration of anonymous subroutine");
		    else if (*s != ';' && *s != '}')
			Perl_croak(aTHX_ "Illegal declaration of subroutine %"SVf, SVfARG(PL_subname));
		}

#ifdef PERL_MAD
		start_force(0);
		if (tmpwhite) {
		    if (PL_madskills)
			curmad('^', newSVpvs(""));
		    CURMAD('_', tmpwhite);
		}
		force_next(0);

		PL_thistoken = subtoken;
#else
		if (have_proto) {
		    NEXTVAL_NEXTTOKE.opval =
			(OP*)newSVOP(OP_CONST, 0, PL_lex_stuff);
		    PL_lex_stuff = NULL;
		    force_next(THING);
		}
#endif
		if (!have_name) {
		    if (PL_curstash)
			sv_setpvs(PL_subname, "__ANON__");
		    else
			sv_setpvs(PL_subname, "__ANON__::__ANON__");
		    TOKEN(ANONSUB);
		}
#ifndef PERL_MAD
		(void) force_word(PL_oldbufptr + tboffset, WORD,
				  FALSE, TRUE, TRUE);
#endif
		if (key == KEY_my)
		    TOKEN(MYSUB);
		TOKEN(SUB);
	    }

	case KEY_system:
	    LOP(OP_SYSTEM,XREF);

	case KEY_symlink:
	    LOP(OP_SYMLINK,XTERM);

	case KEY_syscall:
	    LOP(OP_SYSCALL,XTERM);

	case KEY_sysopen:
	    LOP(OP_SYSOPEN,XTERM);

	case KEY_sysseek:
	    LOP(OP_SYSSEEK,XTERM);

	case KEY_sysread:
	    LOP(OP_SYSREAD,XTERM);

	case KEY_syswrite:
	    LOP(OP_SYSWRITE,XTERM);

	case KEY_tr:
	    s = scan_trans(s);
	    TERM(sublex_start());

	case KEY_tell:
	    UNI(OP_TELL);

	case KEY_telldir:
	    UNI(OP_TELLDIR);

	case KEY_tie:
	    LOP(OP_TIE,XTERM);

	case KEY_tied:
	    UNI(OP_TIED);

	case KEY_time:
	    FUN0(OP_TIME);

	case KEY_times:
	    FUN0(OP_TMS);

	case KEY_truncate:
	    LOP(OP_TRUNCATE,XTERM);

	case KEY_uc:
	    UNI(OP_UC);

	case KEY_ucfirst:
	    UNI(OP_UCFIRST);

	case KEY_untie:
	    UNI(OP_UNTIE);

	case KEY_until:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_NONEXPR)
		return REPORT(0);
	    pl_yylval.ival = CopLINE(PL_curcop);
	    OPERATOR(UNTIL);

	case KEY_unless:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_NONEXPR)
		return REPORT(0);
	    pl_yylval.ival = CopLINE(PL_curcop);
	    OPERATOR(UNLESS);

	case KEY_unlink:
	    LOP(OP_UNLINK,XTERM);

	case KEY_undef:
	    UNIDOR(OP_UNDEF);

	case KEY_unpack:
	    LOP(OP_UNPACK,XTERM);

	case KEY_utime:
	    LOP(OP_UTIME,XTERM);

	case KEY_umask:
	    UNIDOR(OP_UMASK);

	case KEY_unshift:
	    LOP(OP_UNSHIFT,XTERM);

	case KEY_use:
	    s = tokenize_use(1, s);
	    OPERATOR(USE);

	case KEY_values:
	    UNI(OP_VALUES);

	case KEY_vec:
	    LOP(OP_VEC,XTERM);

	case KEY_when:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_NONEXPR)
		return REPORT(0);
	    pl_yylval.ival = CopLINE(PL_curcop);
	    OPERATOR(WHEN);

	case KEY_while:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_NONEXPR)
		return REPORT(0);
	    pl_yylval.ival = CopLINE(PL_curcop);
	    OPERATOR(WHILE);

	case KEY_warn:
	    PL_hints |= HINT_BLOCK_SCOPE;
	    LOP(OP_WARN,XTERM);

	case KEY_wait:
	    FUN0(OP_WAIT);

	case KEY_waitpid:
	    LOP(OP_WAITPID,XTERM);

	case KEY_wantarray:
	    FUN0(OP_WANTARRAY);

	case KEY_write:
#ifdef EBCDIC
	{
	    char ctl_l[2];
	    ctl_l[0] = toCTRL('L');
	    ctl_l[1] = '\0';
	    gv_fetchpvn_flags(ctl_l, 1, GV_ADD|GV_NOTQUAL, SVt_PV);
	}
#else
	    /* Make sure $^L is defined */
	    gv_fetchpvs("\f", GV_ADD|GV_NOTQUAL, SVt_PV);
#endif
	    UNI(OP_ENTERWRITE);

	case KEY_x:
	    if (PL_expect == XOPERATOR) {
		if (*s == '=' && !PL_lex_allbrackets &&
			PL_lex_fakeeof >= LEX_FAKEEOF_ASSIGN)
		    return REPORT(0);
		Mop(OP_REPEAT);
	    }
	    check_uni();
	    goto just_a_word;

	case KEY_xor:
	    if (!PL_lex_allbrackets && PL_lex_fakeeof >= LEX_FAKEEOF_LOWLOGIC)
		return REPORT(0);
	    pl_yylval.ival = OP_XOR;
	    OPERATOR(OROP);

	case KEY_y:
	    s = scan_trans(s);
	    TERM(sublex_start());
	}
    }}
}
#ifdef __SC__
#pragma segment Main
#endif

static int
S_pending_ident(pTHX)
{
    dVAR;
    register char *d;
    PADOFFSET tmp = 0;
    /* pit holds the identifier we read and pending_ident is reset */
    char pit = PL_pending_ident;
    const STRLEN tokenbuf_len = strlen(PL_tokenbuf);
    /* All routes through this function want to know if there is a colon.  */
    const char *const has_colon = (const char*) memchr (PL_tokenbuf, ':', tokenbuf_len);
    PL_pending_ident = 0;

    /* PL_realtokenstart = realtokenend = PL_bufptr - SvPVX(PL_linestr); */
    DEBUG_T({ PerlIO_printf(Perl_debug_log,
          "### Pending identifier '%s'\n", PL_tokenbuf); });

    /* if we're in a my(), we can't allow dynamics here.
       $foo'bar has already been turned into $foo::bar, so
       just check for colons.

       if it's a legal name, the OP is a PADANY.
    */
    if (PL_in_my) {
        if (PL_in_my == KEY_our) {	/* "our" is merely analogous to "my" */
            if (has_colon)
                yyerror_pv(Perl_form(aTHX_ "No package name allowed for "
                                  "variable %s in \"our\"",
                                  PL_tokenbuf), UTF ? SVf_UTF8 : 0);
            tmp = allocmy(PL_tokenbuf, tokenbuf_len, UTF ? SVf_UTF8 : 0);
        }
        else {
            if (has_colon)
                yyerror_pv(Perl_form(aTHX_ PL_no_myglob,
			    PL_in_my == KEY_my ? "my" : "state", PL_tokenbuf),
                            UTF ? SVf_UTF8 : 0);

            pl_yylval.opval = newOP(OP_PADANY, 0);
            pl_yylval.opval->op_targ = allocmy(PL_tokenbuf, tokenbuf_len,
                                                        UTF ? SVf_UTF8 : 0);
            return PRIVATEREF;
        }
    }

    /*
       build the ops for accesses to a my() variable.

       Deny my($a) or my($b) in a sort block, *if* $a or $b is
       then used in a comparison.  This catches most, but not
       all cases.  For instance, it catches
           sort { my($a); $a <=> $b }
       but not
           sort { my($a); $a < $b ? -1 : $a == $b ? 0 : 1; }
       (although why you'd do that is anyone's guess).
    */

    if (!has_colon) {
	if (!PL_in_my)
	    tmp = pad_findmy_pvn(PL_tokenbuf, tokenbuf_len,
                                    UTF ? SVf_UTF8 : 0);
        if (tmp != NOT_IN_PAD) {
            /* might be an "our" variable" */
            if (PAD_COMPNAME_FLAGS_isOUR(tmp)) {
                /* build ops for a bareword */
		HV *  const stash = PAD_COMPNAME_OURSTASH(tmp);
		HEK * const stashname = HvNAME_HEK(stash);
		SV *  const sym = newSVhek(stashname);
                sv_catpvs(sym, "::");
                sv_catpvn_flags(sym, PL_tokenbuf+1, tokenbuf_len - 1, (UTF ? SV_CATUTF8 : SV_CATBYTES ));
                pl_yylval.opval = (OP*)newSVOP(OP_CONST, 0, sym);
                pl_yylval.opval->op_private = OPpCONST_ENTERED;
                gv_fetchsv(sym,
                    (PL_in_eval
                        ? (GV_ADDMULTI | GV_ADDINEVAL)
                        : GV_ADDMULTI
                    ),
                    ((PL_tokenbuf[0] == '$') ? SVt_PV
                     : (PL_tokenbuf[0] == '@') ? SVt_PVAV
                     : SVt_PVHV));
                return WORD;
            }

            /* if it's a sort block and they're naming $a or $b */
            if (PL_last_lop_op == OP_SORT &&
                PL_tokenbuf[0] == '$' &&
                (PL_tokenbuf[1] == 'a' || PL_tokenbuf[1] == 'b')
                && !PL_tokenbuf[2])
            {
                for (d = PL_in_eval ? PL_oldoldbufptr : PL_linestart;
                     d < PL_bufend && *d != '\n';
                     d++)
                {
                    if (strnEQ(d,"<=>",3) || strnEQ(d,"cmp",3)) {
                        Perl_croak(aTHX_ "Can't use \"my %s\" in sort comparison",
                              PL_tokenbuf);
                    }
                }
            }

            pl_yylval.opval = newOP(OP_PADANY, 0);
            pl_yylval.opval->op_targ = tmp;
            return PRIVATEREF;
        }
    }

    /*
       Whine if they've said @foo in a doublequoted string,
       and @foo isn't a variable we can find in the symbol
       table.
    */
    if (ckWARN(WARN_AMBIGUOUS) &&
	pit == '@' && PL_lex_state != LEX_NORMAL && !PL_lex_brackets) {
        GV *const gv = gv_fetchpvn_flags(PL_tokenbuf + 1, tokenbuf_len - 1,
                                        ( UTF ? SVf_UTF8 : 0 ), SVt_PVAV);
        if ((!gv || ((PL_tokenbuf[0] == '@') ? !GvAV(gv) : !GvHV(gv)))
		/* DO NOT warn for @- and @+ */
		&& !( PL_tokenbuf[2] == '\0' &&
		    ( PL_tokenbuf[1] == '-' || PL_tokenbuf[1] == '+' ))
	   )
        {
            /* Downgraded from fatal to warning 20000522 mjd */
            Perl_warner(aTHX_ packWARN(WARN_AMBIGUOUS),
			"Possible unintended interpolation of %"SVf" in string",
			SVfARG(newSVpvn_flags(PL_tokenbuf, tokenbuf_len,
                                        SVs_TEMP | ( UTF ? SVf_UTF8 : 0 ))));
        }
    }

    /* build ops for a bareword */
    pl_yylval.opval = (OP*)newSVOP(OP_CONST, 0, newSVpvn_flags(PL_tokenbuf + 1,
						      tokenbuf_len - 1,
                                                      UTF ? SVf_UTF8 : 0 ));
    pl_yylval.opval->op_private = OPpCONST_ENTERED;
    gv_fetchpvn_flags(PL_tokenbuf+1, tokenbuf_len - 1,
		     (PL_in_eval ? (GV_ADDMULTI | GV_ADDINEVAL) : GV_ADD)
                     | ( UTF ? SVf_UTF8 : 0 ),
		     ((PL_tokenbuf[0] == '$') ? SVt_PV
		      : (PL_tokenbuf[0] == '@') ? SVt_PVAV
		      : SVt_PVHV));
    return WORD;
}

STATIC void
S_checkcomma(pTHX_ const char *s, const char *name, const char *what)
{
    dVAR;

    PERL_ARGS_ASSERT_CHECKCOMMA;

    if (*s == ' ' && s[1] == '(') {	/* XXX gotta be a better way */
	if (ckWARN(WARN_SYNTAX)) {
	    int level = 1;
	    const char *w;
	    for (w = s+2; *w && level; w++) {
		if (*w == '(')
		    ++level;
		else if (*w == ')')
		    --level;
	    }
	    while (isSPACE(*w))
		++w;
	    /* the list of chars below is for end of statements or
	     * block / parens, boolean operators (&&, ||, //) and branch
	     * constructs (or, and, if, until, unless, while, err, for).
	     * Not a very solid hack... */
	    if (!*w || !strchr(";&/|})]oaiuwef!=", *w))
		Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
			    "%s (...) interpreted as function",name);
	}
    }
    while (s < PL_bufend && isSPACE(*s))
	s++;
    if (*s == '(')
	s++;
    while (s < PL_bufend && isSPACE(*s))
	s++;
    if (isIDFIRST_lazy_if(s,UTF)) {
	const char * const w = s;
        s += UTF ? UTF8SKIP(s) : 1;
	while (isALNUM_lazy_if(s,UTF))
	    s += UTF ? UTF8SKIP(s) : 1;
	while (s < PL_bufend && isSPACE(*s))
	    s++;
	if (*s == ',') {
	    GV* gv;
	    if (keyword(w, s - w, 0))
		return;

	    gv = gv_fetchpvn_flags(w, s - w, ( UTF ? SVf_UTF8 : 0 ), SVt_PVCV);
	    if (gv && GvCVu(gv))
		return;
	    Perl_croak(aTHX_ "No comma allowed after %s", what);
	}
    }
}

/* Either returns sv, or mortalizes sv and returns a new SV*.
   Best used as sv=new_constant(..., sv, ...).
   If s, pv are NULL, calls subroutine with one argument,
   and type is used with error messages only. */

STATIC SV *
S_new_constant(pTHX_ const char *s, STRLEN len, const char *key, STRLEN keylen,
	       SV *sv, SV *pv, const char *type, STRLEN typelen)
{
    dVAR; dSP;
    HV * table = GvHV(PL_hintgv);		 /* ^H */
    SV *res;
    SV **cvp;
    SV *cv, *typesv;
    const char *why1 = "", *why2 = "", *why3 = "";

    PERL_ARGS_ASSERT_NEW_CONSTANT;

    /* charnames doesn't work well if there have been errors found */
    if (PL_error_count > 0 && strEQ(key,"charnames"))
	return &PL_sv_undef;

    if (!table
	|| ! (PL_hints & HINT_LOCALIZE_HH)
	|| ! (cvp = hv_fetch(table, key, keylen, FALSE))
	|| ! SvOK(*cvp))
    {
	SV *msg;
	
	/* Here haven't found what we're looking for.  If it is charnames,
	 * perhaps it needs to be loaded.  Try doing that before giving up */
	if (strEQ(key,"charnames")) {
	    Perl_load_module(aTHX_
		            0,
			    newSVpvs("_charnames"),
			     /* version parameter; no need to specify it, as if
			      * we get too early a version, will fail anyway,
			      * not being able to find '_charnames' */
			    NULL,
			    newSVpvs(":full"),
			    newSVpvs(":short"),
			    NULL);
	    SPAGAIN;
	    table = GvHV(PL_hintgv);
	    if (table
		&& (PL_hints & HINT_LOCALIZE_HH)
		&& (cvp = hv_fetch(table, key, keylen, FALSE))
		&& SvOK(*cvp))
	    {
		goto now_ok;
	    }
	}
	if (!table || !(PL_hints & HINT_LOCALIZE_HH)) {
	    msg = Perl_newSVpvf(aTHX_
			    "Constant(%s) unknown", (type ? type: "undef"));
	}
	else {
	why1 = "$^H{";
	why2 = key;
	why3 = "} is not defined";
    report:
	msg = Perl_newSVpvf(aTHX_ "Constant(%s): %s%s%s",
			    (type ? type: "undef"), why1, why2, why3);
	}
	yyerror(SvPVX_const(msg));
 	SvREFCNT_dec(msg);
  	return sv;
    }
now_ok:
    sv_2mortal(sv);			/* Parent created it permanently */
    cv = *cvp;
    if (!pv && s)
  	pv = newSVpvn_flags(s, len, SVs_TEMP);
    if (type && pv)
  	typesv = newSVpvn_flags(type, typelen, SVs_TEMP);
    else
  	typesv = &PL_sv_undef;

    PUSHSTACKi(PERLSI_OVERLOAD);
    ENTER ;
    SAVETMPS;

    PUSHMARK(SP) ;
    EXTEND(sp, 3);
    if (pv)
 	PUSHs(pv);
    PUSHs(sv);
    if (pv)
 	PUSHs(typesv);
    PUTBACK;
    call_sv(cv, G_SCALAR | ( PL_in_eval ? 0 : G_EVAL));

    SPAGAIN ;

    /* Check the eval first */
    if (!PL_in_eval && SvTRUE(ERRSV)) {
 	sv_catpvs(ERRSV, "Propagated");
	yyerror(SvPV_nolen_const(ERRSV)); /* Duplicates the message inside eval */
	(void)POPs;
	res = SvREFCNT_inc_simple(sv);
    }
    else {
 	res = POPs;
	SvREFCNT_inc_simple_void(res);
    }

    PUTBACK ;
    FREETMPS ;
    LEAVE ;
    POPSTACK;

    if (!SvOK(res)) {
 	why1 = "Call to &{$^H{";
 	why2 = key;
 	why3 = "}} did not return a defined value";
 	sv = res;
 	goto report;
    }

    return res;
}

/* Returns a NUL terminated string, with the length of the string written to
   *slp
   */
STATIC char *
S_scan_word(pTHX_ register char *s, char *dest, STRLEN destlen, int allow_package, STRLEN *slp)
{
    dVAR;
    register char *d = dest;
    register char * const e = d + destlen - 3;  /* two-character token, ending NUL */

    PERL_ARGS_ASSERT_SCAN_WORD;

    for (;;) {
	if (d >= e)
	    Perl_croak(aTHX_ ident_too_long);
	if (isALNUM(*s) || (!UTF && isALNUMC_L1(*s)))	/* UTF handled below */
	    *d++ = *s++;
	else if (allow_package && (*s == '\'') && isIDFIRST_lazy_if(s+1,UTF)) {
	    *d++ = ':';
	    *d++ = ':';
	    s++;
	}
	else if (allow_package && (s[0] == ':') && (s[1] == ':') && (s[2] != '$')) {
	    *d++ = *s++;
	    *d++ = *s++;
	}
	else if (UTF && UTF8_IS_START(*s) && isALNUM_utf8((U8*)s)) {
	    char *t = s + UTF8SKIP(s);
	    size_t len;
	    while (UTF8_IS_CONTINUED(*t) && is_utf8_mark((U8*)t))
		t += UTF8SKIP(t);
	    len = t - s;
	    if (d + len > e)
		Perl_croak(aTHX_ ident_too_long);
	    Copy(s, d, len, char);
	    d += len;
	    s = t;
	}
	else {
	    *d = '\0';
	    *slp = d - dest;
	    return s;
	}
    }
}

STATIC char *
S_scan_ident(pTHX_ register char *s, register const char *send, char *dest, STRLEN destlen, I32 ck_uni)
{
    dVAR;
    char *bracket = NULL;
    char funny = *s++;
    register char *d = dest;
    register char * const e = d + destlen - 3;    /* two-character token, ending NUL */

    PERL_ARGS_ASSERT_SCAN_IDENT;

    if (isSPACE(*s))
	s = PEEKSPACE(s);
    if (isDIGIT(*s)) {
	while (isDIGIT(*s)) {
	    if (d >= e)
		Perl_croak(aTHX_ ident_too_long);
	    *d++ = *s++;
	}
    }
    else {
	for (;;) {
	    if (d >= e)
		Perl_croak(aTHX_ ident_too_long);
	    if (isALNUM(*s))	/* UTF handled below */
		*d++ = *s++;
	    else if (*s == '\'' && isIDFIRST_lazy_if(s+1,UTF)) {
		*d++ = ':';
		*d++ = ':';
		s++;
	    }
	    else if (*s == ':' && s[1] == ':') {
		*d++ = *s++;
		*d++ = *s++;
	    }
	    else if (UTF && UTF8_IS_START(*s) && isALNUM_utf8((U8*)s)) {
		char *t = s + UTF8SKIP(s);
		while (UTF8_IS_CONTINUED(*t) && is_utf8_mark((U8*)t))
		    t += UTF8SKIP(t);
		if (d + (t - s) > e)
		    Perl_croak(aTHX_ ident_too_long);
		Copy(s, d, t - s, char);
		d += t - s;
		s = t;
	    }
	    else
		break;
	}
    }
    *d = '\0';
    d = dest;
    if (*d) {
	if (PL_lex_state != LEX_NORMAL)
	    PL_lex_state = LEX_INTERPENDMAYBE;
	return s;
    }
    if (*s == '$' && s[1] &&
	(isALNUM_lazy_if(s+1,UTF) || s[1] == '$' || s[1] == '{' || strnEQ(s+1,"::",2)) )
    {
	return s;
    }
    if (*s == '{') {
	bracket = s;
	s++;
    }
    else if (ck_uni)
	check_uni();
    if (s < send) {
        if (UTF) {
            const STRLEN skip = UTF8SKIP(s);
            STRLEN i;
            d[skip] = '\0';
            for ( i = 0; i < skip; i++ )
                d[i] = *s++;
        }
        else {
            *d = *s++;
            d[1] = '\0';
        }
    }
    if (*d == '^' && *s && isCONTROLVAR(*s)) {
	*d = toCTRL(*s);
	s++;
    }
    if (bracket) {
	if (isSPACE(s[-1])) {
	    while (s < send) {
		const char ch = *s++;
		if (!SPACE_OR_TAB(ch)) {
		    *d = ch;
		    break;
		}
	    }
	}
	if (isIDFIRST_lazy_if(d,UTF)) {
	    d += UTF8SKIP(d);
	    if (UTF) {
		char *end = s;
		while ((end < send && isALNUM_lazy_if(end,UTF)) || *end == ':') {
		    end += UTF8SKIP(end);
		    while (end < send && UTF8_IS_CONTINUED(*end) && is_utf8_mark((U8*)end))
			end += UTF8SKIP(end);
		}
		Copy(s, d, end - s, char);
		d += end - s;
		s = end;
	    }
	    else {
		while ((isALNUM(*s) || *s == ':') && d < e)
		    *d++ = *s++;
		if (d >= e)
		    Perl_croak(aTHX_ ident_too_long);
	    }
	    *d = '\0';
	    while (s < send && SPACE_OR_TAB(*s))
		s++;
	    if ((*s == '[' || (*s == '{' && strNE(dest, "sub")))) {
		if (ckWARN(WARN_AMBIGUOUS) && keyword(dest, d - dest, 0)) {
		    const char * const brack =
			(const char *)
			((*s == '[') ? "[...]" : "{...}");
   /* diag_listed_as: Ambiguous use of %c{%s[...]} resolved to %c%s[...] */
		    Perl_warner(aTHX_ packWARN(WARN_AMBIGUOUS),
			"Ambiguous use of %c{%s%s} resolved to %c%s%s",
			funny, dest, brack, funny, dest, brack);
		}
		bracket++;
		PL_lex_brackstack[PL_lex_brackets++] = (char)(XOPERATOR | XFAKEBRACK);
		PL_lex_allbrackets++;
		return s;
	    }
	}
	/* Handle extended ${^Foo} variables
	 * 1999-02-27 mjd-perl-patch@plover.com */
	else if (!isALNUM(*d) && !isPRINT(*d) /* isCTRL(d) */
		 && isALNUM(*s))
	{
	    d++;
	    while (isALNUM(*s) && d < e) {
		*d++ = *s++;
	    }
	    if (d >= e)
		Perl_croak(aTHX_ ident_too_long);
	    *d = '\0';
	}
	if (*s == '}') {
	    s++;
	    if (PL_lex_state == LEX_INTERPNORMAL && !PL_lex_brackets) {
		PL_lex_state = LEX_INTERPEND;
		PL_expect = XREF;
	    }
	    if (PL_lex_state == LEX_NORMAL) {
		if (ckWARN(WARN_AMBIGUOUS) &&
		    (keyword(dest, d - dest, 0)
		     || get_cvn_flags(dest, d - dest, UTF ? SVf_UTF8 : 0)))
		{
                    SV *tmp = newSVpvn_flags( dest, d - dest,
                                            SVs_TEMP | (UTF ? SVf_UTF8 : 0) );
		    if (funny == '#')
			funny = '@';
		    Perl_warner(aTHX_ packWARN(WARN_AMBIGUOUS),
			"Ambiguous use of %c{%"SVf"} resolved to %c%"SVf,
			funny, tmp, funny, tmp);
		}
	    }
	}
	else {
	    s = bracket;		/* let the parser handle it */
	    *dest = '\0';
	}
    }
    else if (PL_lex_state == LEX_INTERPNORMAL && !PL_lex_brackets && !intuit_more(s))
	PL_lex_state = LEX_INTERPEND;
    return s;
}

static bool
S_pmflag(pTHX_ const char* const valid_flags, U32 * pmfl, char** s, char* charset) {

    /* Adds, subtracts to/from 'pmfl' based on regex modifier flags found in
     * the parse starting at 's', based on the subset that are valid in this
     * context input to this routine in 'valid_flags'. Advances s.  Returns
     * TRUE if the input was a valid flag, so the next char may be as well;
     * otherwise FALSE. 'charset' should point to a NUL upon first call on the
     * current regex.  This routine will set it to any charset modifier found.
     * The caller shouldn't change it.  This way, another charset modifier
     * encountered in the parse can be detected as an error, as we have decided
     * allow only one */

    const char c = **s;

    if (! strchr(valid_flags, c)) {
        if (isALNUM(c)) {
	    goto deprecate;
        }
        return FALSE;
    }

    switch (c) {

        CASE_STD_PMMOD_FLAGS_PARSE_SET(pmfl);
        case GLOBAL_PAT_MOD:      *pmfl |= PMf_GLOBAL; break;
        case CONTINUE_PAT_MOD:    *pmfl |= PMf_CONTINUE; break;
        case ONCE_PAT_MOD:        *pmfl |= PMf_KEEP; break;
        case KEEPCOPY_PAT_MOD:    *pmfl |= RXf_PMf_KEEPCOPY; break;
        case NONDESTRUCT_PAT_MOD: *pmfl |= PMf_NONDESTRUCT; break;
	case LOCALE_PAT_MOD:

	    /* In 5.14, qr//lt is legal but deprecated; the 't' means they
	     * can't be regex modifiers.
	     * In 5.14, s///le is legal and ambiguous.  Try to disambiguate as
	     * much as easily done.  s///lei, for example, has to mean regex
	     * modifiers if it's not an error (as does any word character
	     * following the 'e').  Otherwise, we resolve to the backwards-
	     * compatible, but less likely 's/// le ...', i.e. as meaning
	     * less-than-or-equal.  The reason it's not likely is that s//
	     * returns a number for code in the field (/r returns a string, but
	     * that wasn't added until the 5.13 series), and so '<=' should be
	     * used for comparing, not 'le'. */
	    if (*((*s) + 1) == 't') {
		goto deprecate;
	    }
	    else if (*((*s) + 1) == 'e' && ! isALNUM(*((*s) + 2))) {

		/* 'e' is valid only for substitutes, s///e.  If it is not
		 * valid in the current context, then 'm//le' must mean the
		 * comparison operator, so use the regular deprecation message.
		 */
		if (! strchr(valid_flags, 'e')) {
		    goto deprecate;
		}
		Perl_ck_warner_d(aTHX_ packWARN(WARN_AMBIGUOUS),
		    "Ambiguous use of 's//le...' resolved as 's// le...'; Rewrite as 's//el' if you meant 'use locale rules and evaluate rhs as an expression'.  In Perl 5.18, it will be resolved the other way");
		return FALSE;
	    }
	    if (*charset) {
		goto multiple_charsets;
	    }
	    set_regex_charset(pmfl, REGEX_LOCALE_CHARSET);
	    *charset = c;
	    break;
	case UNICODE_PAT_MOD:
	    /* In 5.14, qr//unless and qr//until are legal but deprecated; the
	     * 'n' means they can't be regex modifiers */
	    if (*((*s) + 1) == 'n') {
		goto deprecate;
	    }
	    if (*charset) {
		goto multiple_charsets;
	    }
	    set_regex_charset(pmfl, REGEX_UNICODE_CHARSET);
	    *charset = c;
	    break;
	case ASCII_RESTRICT_PAT_MOD:
	    /* In 5.14, qr//and is legal but deprecated; the 'n' means they
	     * can't be regex modifiers */
	    if (*((*s) + 1) == 'n') {
		goto deprecate;
	    }

	    if (! *charset) {
		set_regex_charset(pmfl, REGEX_ASCII_RESTRICTED_CHARSET);
	    }
	    else {

		/* Error if previous modifier wasn't an 'a', but if it was, see
		 * if, and accept, a second occurrence (only) */
		if (*charset != 'a'
		    || get_regex_charset(*pmfl)
			!= REGEX_ASCII_RESTRICTED_CHARSET)
		{
			goto multiple_charsets;
		}
		set_regex_charset(pmfl, REGEX_ASCII_MORE_RESTRICTED_CHARSET);
	    }
	    *charset = c;
	    break;
	case DEPENDS_PAT_MOD:
	    if (*charset) {
		goto multiple_charsets;
	    }
	    set_regex_charset(pmfl, REGEX_DEPENDS_CHARSET);
	    *charset = c;
	    break;
    }

    (*s)++;
    return TRUE;

    deprecate:
	Perl_ck_warner_d(aTHX_ packWARN(WARN_SYNTAX),
	    "Having no space between pattern and following word is deprecated");
        return FALSE;

    multiple_charsets:
	if (*charset != c) {
	    yyerror(Perl_form(aTHX_ "Regexp modifiers \"/%c\" and \"/%c\" are mutually exclusive", *charset, c));
	}
	else if (c == 'a') {
	    yyerror("Regexp modifier \"/a\" may appear a maximum of twice");
	}
	else {
	    yyerror(Perl_form(aTHX_ "Regexp modifier \"/%c\" may not appear twice", c));
	}

	/* Pretend that it worked, so will continue processing before dieing */
	(*s)++;
	return TRUE;
}

STATIC char *
S_scan_pat(pTHX_ char *start, I32 type)
{
    dVAR;
    PMOP *pm;
    char *s = scan_str(start,!!PL_madskills,FALSE);
    const char * const valid_flags =
	(const char *)((type == OP_QR) ? QR_PAT_MODS : M_PAT_MODS);
    char charset = '\0';    /* character set modifier */
#ifdef PERL_MAD
    char *modstart;
#endif

    PERL_ARGS_ASSERT_SCAN_PAT;

    if (!s) {
	const char * const delimiter = skipspace(start);
	Perl_croak(aTHX_
		   (const char *)
		   (*delimiter == '?'
		    ? "Search pattern not terminated or ternary operator parsed as search pattern"
		    : "Search pattern not terminated" ));
    }

    pm = (PMOP*)newPMOP(type, 0);
    if (PL_multi_open == '?') {
	/* This is the only point in the code that sets PMf_ONCE:  */
	pm->op_pmflags |= PMf_ONCE;

	/* Hence it's safe to do this bit of PMOP book-keeping here, which
	   allows us to restrict the list needed by reset to just the ??
	   matches.  */
	assert(type != OP_TRANS);
	if (PL_curstash) {
	    MAGIC *mg = mg_find((const SV *)PL_curstash, PERL_MAGIC_symtab);
	    U32 elements;
	    if (!mg) {
		mg = sv_magicext(MUTABLE_SV(PL_curstash), 0, PERL_MAGIC_symtab, 0, 0,
				 0);
	    }
	    elements = mg->mg_len / sizeof(PMOP**);
	    Renewc(mg->mg_ptr, elements + 1, PMOP*, char);
	    ((PMOP**)mg->mg_ptr) [elements++] = pm;
	    mg->mg_len = elements * sizeof(PMOP**);
	    PmopSTASH_set(pm,PL_curstash);
	}
    }
#ifdef PERL_MAD
    modstart = s;
#endif
    while (*s && S_pmflag(aTHX_ valid_flags, &(pm->op_pmflags), &s, &charset)) {};
#ifdef PERL_MAD
    if (PL_madskills && modstart != s) {
	SV* tmptoken = newSVpvn(modstart, s - modstart);
	append_madprops(newMADPROP('m', MAD_SV, tmptoken, 0), (OP*)pm, 0);
    }
#endif
    /* issue a warning if /c is specified,but /g is not */
    if ((pm->op_pmflags & PMf_CONTINUE) && !(pm->op_pmflags & PMf_GLOBAL))
    {
        Perl_ck_warner(aTHX_ packWARN(WARN_REGEXP), 
		       "Use of /c modifier is meaningless without /g" );
    }

    PL_lex_op = (OP*)pm;
    pl_yylval.ival = OP_MATCH;
    return s;
}

STATIC char *
S_scan_subst(pTHX_ char *start)
{
    dVAR;
    char *s;
    register PMOP *pm;
    I32 first_start;
    I32 es = 0;
    char charset = '\0';    /* character set modifier */
#ifdef PERL_MAD
    char *modstart;
#endif

    PERL_ARGS_ASSERT_SCAN_SUBST;

    pl_yylval.ival = OP_NULL;

    s = scan_str(start,!!PL_madskills,FALSE);

    if (!s)
	Perl_croak(aTHX_ "Substitution pattern not terminated");

    if (s[-1] == PL_multi_open)
	s--;
#ifdef PERL_MAD
    if (PL_madskills) {
	CURMAD('q', PL_thisopen);
	CURMAD('_', PL_thiswhite);
	CURMAD('E', PL_thisstuff);
	CURMAD('Q', PL_thisclose);
	PL_realtokenstart = s - SvPVX(PL_linestr);
    }
#endif

    first_start = PL_multi_start;
    s = scan_str(s,!!PL_madskills,FALSE);
    if (!s) {
	if (PL_lex_stuff) {
	    SvREFCNT_dec(PL_lex_stuff);
	    PL_lex_stuff = NULL;
	}
	Perl_croak(aTHX_ "Substitution replacement not terminated");
    }
    PL_multi_start = first_start;	/* so whole substitution is taken together */

    pm = (PMOP*)newPMOP(OP_SUBST, 0);

#ifdef PERL_MAD
    if (PL_madskills) {
	CURMAD('z', PL_thisopen);
	CURMAD('R', PL_thisstuff);
	CURMAD('Z', PL_thisclose);
    }
    modstart = s;
#endif

    while (*s) {
	if (*s == EXEC_PAT_MOD) {
	    s++;
	    es++;
	}
	else if (! S_pmflag(aTHX_ S_PAT_MODS, &(pm->op_pmflags), &s, &charset))
	{
	    break;
	}
    }

#ifdef PERL_MAD
    if (PL_madskills) {
	if (modstart != s)
	    curmad('m', newSVpvn(modstart, s - modstart));
	append_madprops(PL_thismad, (OP*)pm, 0);
	PL_thismad = 0;
    }
#endif
    if ((pm->op_pmflags & PMf_CONTINUE)) {
        Perl_ck_warner(aTHX_ packWARN(WARN_REGEXP), "Use of /c modifier is meaningless in s///" );
    }

    if (es) {
	SV * const repl = newSVpvs("");

	PL_sublex_info.super_bufptr = s;
	PL_sublex_info.super_bufend = PL_bufend;
	PL_multi_end = 0;
	pm->op_pmflags |= PMf_EVAL;
	while (es-- > 0) {
	    if (es)
		sv_catpvs(repl, "eval ");
	    else
		sv_catpvs(repl, "do ");
	}
	sv_catpvs(repl, "{");
	sv_catsv(repl, PL_lex_repl);
	if (strchr(SvPVX(PL_lex_repl), '#'))
	    sv_catpvs(repl, "\n");
	sv_catpvs(repl, "}");
	SvEVALED_on(repl);
	SvREFCNT_dec(PL_lex_repl);
	PL_lex_repl = repl;
    }

    PL_lex_op = (OP*)pm;
    pl_yylval.ival = OP_SUBST;
    return s;
}

STATIC char *
S_scan_trans(pTHX_ char *start)
{
    dVAR;
    register char* s;
    OP *o;
    U8 squash;
    U8 del;
    U8 complement;
    bool nondestruct = 0;
#ifdef PERL_MAD
    char *modstart;
#endif

    PERL_ARGS_ASSERT_SCAN_TRANS;

    pl_yylval.ival = OP_NULL;

    s = scan_str(start,!!PL_madskills,FALSE);
    if (!s)
	Perl_croak(aTHX_ "Transliteration pattern not terminated");

    if (s[-1] == PL_multi_open)
	s--;
#ifdef PERL_MAD
    if (PL_madskills) {
	CURMAD('q', PL_thisopen);
	CURMAD('_', PL_thiswhite);
	CURMAD('E', PL_thisstuff);
	CURMAD('Q', PL_thisclose);
	PL_realtokenstart = s - SvPVX(PL_linestr);
    }
#endif

    s = scan_str(s,!!PL_madskills,FALSE);
    if (!s) {
	if (PL_lex_stuff) {
	    SvREFCNT_dec(PL_lex_stuff);
	    PL_lex_stuff = NULL;
	}
	Perl_croak(aTHX_ "Transliteration replacement not terminated");
    }
    if (PL_madskills) {
	CURMAD('z', PL_thisopen);
	CURMAD('R', PL_thisstuff);
	CURMAD('Z', PL_thisclose);
    }

    complement = del = squash = 0;
#ifdef PERL_MAD
    modstart = s;
#endif
    while (1) {
	switch (*s) {
	case 'c':
	    complement = OPpTRANS_COMPLEMENT;
	    break;
	case 'd':
	    del = OPpTRANS_DELETE;
	    break;
	case 's':
	    squash = OPpTRANS_SQUASH;
	    break;
	case 'r':
	    nondestruct = 1;
	    break;
	default:
	    goto no_more;
	}
	s++;
    }
  no_more:

    o = newPVOP(nondestruct ? OP_TRANSR : OP_TRANS, 0, (char*)NULL);
    o->op_private &= ~OPpTRANS_ALL;
    o->op_private |= del|squash|complement|
      (DO_UTF8(PL_lex_stuff)? OPpTRANS_FROM_UTF : 0)|
      (DO_UTF8(PL_lex_repl) ? OPpTRANS_TO_UTF   : 0);

    PL_lex_op = o;
    pl_yylval.ival = nondestruct ? OP_TRANSR : OP_TRANS;

#ifdef PERL_MAD
    if (PL_madskills) {
	if (modstart != s)
	    curmad('m', newSVpvn(modstart, s - modstart));
	append_madprops(PL_thismad, o, 0);
	PL_thismad = 0;
    }
#endif

    return s;
}

STATIC char *
S_scan_heredoc(pTHX_ register char *s)
{
    dVAR;
    SV *herewas;
    I32 op_type = OP_SCALAR;
    I32 len;
    SV *tmpstr;
    char term;
    const char *found_newline;
    register char *d;
    register char *e;
    char *peek;
    const int outer = (PL_rsfp || PL_parser->filtered)
		   && !(PL_lex_inwhat == OP_SCALAR);
#ifdef PERL_MAD
    I32 stuffstart = s - SvPVX(PL_linestr);
    char *tstart;
 
    PL_realtokenstart = -1;
#endif

    PERL_ARGS_ASSERT_SCAN_HEREDOC;

    s += 2;
    d = PL_tokenbuf;
    e = PL_tokenbuf + sizeof PL_tokenbuf - 1;
    if (!outer)
	*d++ = '\n';
    peek = s;
    while (SPACE_OR_TAB(*peek))
	peek++;
    if (*peek == '`' || *peek == '\'' || *peek =='"') {
	s = peek;
	term = *s++;
	s = delimcpy(d, e, s, PL_bufend, term, &len);
	d += len;
	if (s < PL_bufend)
	    s++;
    }
    else {
	if (*s == '\\')
	    s++, term = '\'';
	else
	    term = '"';
	if (!isALNUM_lazy_if(s,UTF))
	    deprecate("bare << to mean <<\"\"");
	for (; isALNUM_lazy_if(s,UTF); s++) {
	    if (d < e)
		*d++ = *s;
	}
    }
    if (d >= PL_tokenbuf + sizeof PL_tokenbuf - 1)
	Perl_croak(aTHX_ "Delimiter for here document is too long");
    *d++ = '\n';
    *d = '\0';
    len = d - PL_tokenbuf;

#ifdef PERL_MAD
    if (PL_madskills) {
	tstart = PL_tokenbuf + !outer;
	PL_thisclose = newSVpvn(tstart, len - !outer);
	tstart = SvPVX(PL_linestr) + stuffstart;
	PL_thisopen = newSVpvn(tstart, s - tstart);
	stuffstart = s - SvPVX(PL_linestr);
    }
#endif
#ifndef PERL_STRICT_CR
    d = strchr(s, '\r');
    if (d) {
	char * const olds = s;
	s = d;
	while (s < PL_bufend) {
	    if (*s == '\r') {
		*d++ = '\n';
		if (*++s == '\n')
		    s++;
	    }
	    else if (*s == '\n' && s[1] == '\r') {	/* \015\013 on a mac? */
		*d++ = *s++;
		s++;
	    }
	    else
		*d++ = *s++;
	}
	*d = '\0';
	PL_bufend = d;
	SvCUR_set(PL_linestr, PL_bufend - SvPVX_const(PL_linestr));
	s = olds;
    }
#endif
#ifdef PERL_MAD
    found_newline = 0;
#endif
    if ( outer || !(found_newline = (char*)memchr((void*)s, '\n', PL_bufend - s)) ) {
        herewas = newSVpvn(s,PL_bufend-s);
    }
    else {
#ifdef PERL_MAD
        herewas = newSVpvn(s-1,found_newline-s+1);
#else
        s--;
        herewas = newSVpvn(s,found_newline-s);
#endif
    }
#ifdef PERL_MAD
    if (PL_madskills) {
	tstart = SvPVX(PL_linestr) + stuffstart;
	if (PL_thisstuff)
	    sv_catpvn(PL_thisstuff, tstart, s - tstart);
	else
	    PL_thisstuff = newSVpvn(tstart, s - tstart);
    }
#endif
    s += SvCUR(herewas);

#ifdef PERL_MAD
    stuffstart = s - SvPVX(PL_linestr);

    if (found_newline)
	s--;
#endif

    tmpstr = newSV_type(SVt_PVIV);
    SvGROW(tmpstr, 80);
    if (term == '\'') {
	op_type = OP_CONST;
	SvIV_set(tmpstr, -1);
    }
    else if (term == '`') {
	op_type = OP_BACKTICK;
	SvIV_set(tmpstr, '\\');
    }

    CLINE;
    PL_multi_start = CopLINE(PL_curcop);
    PL_multi_open = PL_multi_close = '<';
    term = *PL_tokenbuf;
    if (PL_lex_inwhat == OP_SUBST && PL_in_eval && !PL_rsfp
     && !PL_parser->filtered) {
	char * const bufptr = PL_sublex_info.super_bufptr;
	char * const bufend = PL_sublex_info.super_bufend;
	char * const olds = s - SvCUR(herewas);
	s = strchr(bufptr, '\n');
	if (!s)
	    s = bufend;
	d = s;
	while (s < bufend &&
	  (*s != term || memNE(s,PL_tokenbuf,len)) ) {
	    if (*s++ == '\n')
		CopLINE_inc(PL_curcop);
	}
	if (s >= bufend) {
	    CopLINE_set(PL_curcop, (line_t)PL_multi_start);
	    missingterm(PL_tokenbuf);
	}
	sv_setpvn(herewas,bufptr,d-bufptr+1);
	sv_setpvn(tmpstr,d+1,s-d);
	s += len - 1;
	sv_catpvn(herewas,s,bufend-s);
	Copy(SvPVX_const(herewas),bufptr,SvCUR(herewas) + 1,char);

	s = olds;
	goto retval;
    }
    else if (!outer) {
	d = s;
	while (s < PL_bufend &&
	  (*s != term || memNE(s,PL_tokenbuf,len)) ) {
	    if (*s++ == '\n')
		CopLINE_inc(PL_curcop);
	}
	if (s >= PL_bufend) {
	    CopLINE_set(PL_curcop, (line_t)PL_multi_start);
	    missingterm(PL_tokenbuf);
	}
	sv_setpvn(tmpstr,d+1,s-d);
#ifdef PERL_MAD
	if (PL_madskills) {
	    if (PL_thisstuff)
		sv_catpvn(PL_thisstuff, d + 1, s - d);
	    else
		PL_thisstuff = newSVpvn(d + 1, s - d);
	    stuffstart = s - SvPVX(PL_linestr);
	}
#endif
	s += len - 1;
	CopLINE_inc(PL_curcop);	/* the preceding stmt passes a newline */

	sv_catpvn(herewas,s,PL_bufend-s);
	sv_setsv(PL_linestr,herewas);
	PL_oldoldbufptr = PL_oldbufptr = PL_bufptr = s = PL_linestart = SvPVX(PL_linestr);
	PL_bufend = SvPVX(PL_linestr) + SvCUR(PL_linestr);
	PL_last_lop = PL_last_uni = NULL;
    }
    else
	sv_setpvs(tmpstr,"");   /* avoid "uninitialized" warning */
    while (s >= PL_bufend) {	/* multiple line string? */
#ifdef PERL_MAD
	if (PL_madskills) {
	    tstart = SvPVX(PL_linestr) + stuffstart;
	    if (PL_thisstuff)
		sv_catpvn(PL_thisstuff, tstart, PL_bufend - tstart);
	    else
		PL_thisstuff = newSVpvn(tstart, PL_bufend - tstart);
	}
#endif
	PL_bufptr = s;
	CopLINE_inc(PL_curcop);
	if (!outer || !lex_next_chunk(0)) {
	    CopLINE_set(PL_curcop, (line_t)PL_multi_start);
	    missingterm(PL_tokenbuf);
	}
	CopLINE_dec(PL_curcop);
	s = PL_bufptr;
#ifdef PERL_MAD
	stuffstart = s - SvPVX(PL_linestr);
#endif
	CopLINE_inc(PL_curcop);
	PL_bufend = SvPVX(PL_linestr) + SvCUR(PL_linestr);
	PL_last_lop = PL_last_uni = NULL;
#ifndef PERL_STRICT_CR
	if (PL_bufend - PL_linestart >= 2) {
	    if ((PL_bufend[-2] == '\r' && PL_bufend[-1] == '\n') ||
		(PL_bufend[-2] == '\n' && PL_bufend[-1] == '\r'))
	    {
		PL_bufend[-2] = '\n';
		PL_bufend--;
		SvCUR_set(PL_linestr, PL_bufend - SvPVX_const(PL_linestr));
	    }
	    else if (PL_bufend[-1] == '\r')
		PL_bufend[-1] = '\n';
	}
	else if (PL_bufend - PL_linestart == 1 && PL_bufend[-1] == '\r')
	    PL_bufend[-1] = '\n';
#endif
	if (*s == term && memEQ(s,PL_tokenbuf,len)) {
	    STRLEN off = PL_bufend - 1 - SvPVX_const(PL_linestr);
	    *(SvPVX(PL_linestr) + off ) = ' ';
	    lex_grow_linestr(SvCUR(PL_linestr) + SvCUR(herewas) + 1);
	    sv_catsv(PL_linestr,herewas);
	    PL_bufend = SvPVX(PL_linestr) + SvCUR(PL_linestr);
	    s = SvPVX(PL_linestr) + off; /* In case PV of PL_linestr moved. */
	}
	else {
	    s = PL_bufend;
	    sv_catsv(tmpstr,PL_linestr);
	}
    }
    s++;
retval:
    PL_multi_end = CopLINE(PL_curcop);
    if (SvCUR(tmpstr) + 5 < SvLEN(tmpstr)) {
	SvPV_shrink_to_cur(tmpstr);
    }
    SvREFCNT_dec(herewas);
    if (!IN_BYTES) {
	if (UTF && is_utf8_string((U8*)SvPVX_const(tmpstr), SvCUR(tmpstr)))
	    SvUTF8_on(tmpstr);
	else if (PL_encoding)
	    sv_recode_to_utf8(tmpstr, PL_encoding);
    }
    PL_lex_stuff = tmpstr;
    pl_yylval.ival = op_type;
    return s;
}

/* scan_inputsymbol
   takes: current position in input buffer
   returns: new position in input buffer
   side-effects: pl_yylval and lex_op are set.

   This code handles:

   <>		read from ARGV
   <FH> 	read from filehandle
   <pkg::FH>	read from package qualified filehandle
   <pkg'FH>	read from package qualified filehandle
   <$fh>	read from filehandle in $fh
   <*.h>	filename glob

*/

STATIC char *
S_scan_inputsymbol(pTHX_ char *start)
{
    dVAR;
    register char *s = start;		/* current position in buffer */
    char *end;
    I32 len;
    char *d = PL_tokenbuf;					/* start of temp holding space */
    const char * const e = PL_tokenbuf + sizeof PL_tokenbuf;	/* end of temp holding space */

    PERL_ARGS_ASSERT_SCAN_INPUTSYMBOL;

    end = strchr(s, '\n');
    if (!end)
	end = PL_bufend;
    s = delimcpy(d, e, s + 1, end, '>', &len);	/* extract until > */

    /* die if we didn't have space for the contents of the <>,
       or if it didn't end, or if we see a newline
    */

    if (len >= (I32)sizeof PL_tokenbuf)
	Perl_croak(aTHX_ "Excessively long <> operator");
    if (s >= end)
	Perl_croak(aTHX_ "Unterminated <> operator");

    s++;

    /* check for <$fh>
       Remember, only scalar variables are interpreted as filehandles by
       this code.  Anything more complex (e.g., <$fh{$num}>) will be
       treated as a glob() call.
       This code makes use of the fact that except for the $ at the front,
       a scalar variable and a filehandle look the same.
    */
    if (*d == '$' && d[1]) d++;

    /* allow <Pkg'VALUE> or <Pkg::VALUE> */
    while (*d && (isALNUM_lazy_if(d,UTF) || *d == '\'' || *d == ':'))
	d += UTF ? UTF8SKIP(d) : 1;

    /* If we've tried to read what we allow filehandles to look like, and
       there's still text left, then it must be a glob() and not a getline.
       Use scan_str to pull out the stuff between the <> and treat it
       as nothing more than a string.
    */

    if (d - PL_tokenbuf != len) {
	pl_yylval.ival = OP_GLOB;
	s = scan_str(start,!!PL_madskills,FALSE);
	if (!s)
	   Perl_croak(aTHX_ "Glob not terminated");
	return s;
    }
    else {
	bool readline_overriden = FALSE;
	GV *gv_readline;
	GV **gvp;
    	/* we're in a filehandle read situation */
	d = PL_tokenbuf;

	/* turn <> into <ARGV> */
	if (!len)
	    Copy("ARGV",d,5,char);

	/* Check whether readline() is overriden */
	gv_readline = gv_fetchpvs("readline", GV_NOTQUAL, SVt_PVCV);
	if ((gv_readline
		&& GvCVu(gv_readline) && GvIMPORTED_CV(gv_readline))
		||
		((gvp = (GV**)hv_fetchs(PL_globalstash, "readline", FALSE))
		 && (gv_readline = *gvp) && isGV_with_GP(gv_readline)
		&& GvCVu(gv_readline) && GvIMPORTED_CV(gv_readline)))
	    readline_overriden = TRUE;

	/* if <$fh>, create the ops to turn the variable into a
	   filehandle
	*/
	if (*d == '$') {
	    /* try to find it in the pad for this block, otherwise find
	       add symbol table ops
	    */
	    const PADOFFSET tmp = pad_findmy_pvn(d, len, UTF ? SVf_UTF8 : 0);
	    if (tmp != NOT_IN_PAD) {
		if (PAD_COMPNAME_FLAGS_isOUR(tmp)) {
		    HV * const stash = PAD_COMPNAME_OURSTASH(tmp);
		    HEK * const stashname = HvNAME_HEK(stash);
		    SV * const sym = sv_2mortal(newSVhek(stashname));
		    sv_catpvs(sym, "::");
		    sv_catpv(sym, d+1);
		    d = SvPVX(sym);
		    goto intro_sym;
		}
		else {
		    OP * const o = newOP(OP_PADSV, 0);
		    o->op_targ = tmp;
		    PL_lex_op = readline_overriden
			? (OP*)newUNOP(OP_ENTERSUB, OPf_STACKED,
				op_append_elem(OP_LIST, o,
				    newCVREF(0, newGVOP(OP_GV,0,gv_readline))))
			: (OP*)newUNOP(OP_READLINE, 0, o);
		}
	    }
	    else {
		GV *gv;
		++d;
intro_sym:
		gv = gv_fetchpv(d,
				(PL_in_eval
				 ? (GV_ADDMULTI | GV_ADDINEVAL)
				 : GV_ADDMULTI) | ( UTF ? SVf_UTF8 : 0 ),
				SVt_PV);
		PL_lex_op = readline_overriden
		    ? (OP*)newUNOP(OP_ENTERSUB, OPf_STACKED,
			    op_append_elem(OP_LIST,
				newUNOP(OP_RV2SV, 0, newGVOP(OP_GV, 0, gv)),
				newCVREF(0, newGVOP(OP_GV, 0, gv_readline))))
		    : (OP*)newUNOP(OP_READLINE, 0,
			    newUNOP(OP_RV2SV, 0,
				newGVOP(OP_GV, 0, gv)));
	    }
	    if (!readline_overriden)
		PL_lex_op->op_flags |= OPf_SPECIAL;
	    /* we created the ops in PL_lex_op, so make pl_yylval.ival a null op */
	    pl_yylval.ival = OP_NULL;
	}

	/* If it's none of the above, it must be a literal filehandle
	   (<Foo::BAR> or <FOO>) so build a simple readline OP */
	else {
	    GV * const gv = gv_fetchpv(d, GV_ADD | ( UTF ? SVf_UTF8 : 0 ), SVt_PVIO);
	    PL_lex_op = readline_overriden
		? (OP*)newUNOP(OP_ENTERSUB, OPf_STACKED,
			op_append_elem(OP_LIST,
			    newGVOP(OP_GV, 0, gv),
			    newCVREF(0, newGVOP(OP_GV, 0, gv_readline))))
		: (OP*)newUNOP(OP_READLINE, 0, newGVOP(OP_GV, 0, gv));
	    pl_yylval.ival = OP_NULL;
	}
    }

    return s;
}


/* scan_str
   takes: start position in buffer
	  keep_quoted preserve \ on the embedded delimiter(s)
	  keep_delims preserve the delimiters around the string
   returns: position to continue reading from buffer
   side-effects: multi_start, multi_close, lex_repl or lex_stuff, and
   	updates the read buffer.

   This subroutine pulls a string out of the input.  It is called for:
   	q		single quotes		q(literal text)
	'		single quotes		'literal text'
	qq		double quotes		qq(interpolate $here please)
	"		double quotes		"interpolate $here please"
	qx		backticks		qx(/bin/ls -l)
	`		backticks		`/bin/ls -l`
	qw		quote words		@EXPORT_OK = qw( func() $spam )
	m//		regexp match		m/this/
	s///		regexp substitute	s/this/that/
	tr///		string transliterate	tr/this/that/
	y///		string transliterate	y/this/that/
	($*@)		sub prototypes		sub foo ($)
	(stuff)		sub attr parameters	sub foo : attr(stuff)
	<>		readline or globs	<FOO>, <>, <$fh>, or <*.c>
	
   In most of these cases (all but <>, patterns and transliterate)
   yylex() calls scan_str().  m// makes yylex() call scan_pat() which
   calls scan_str().  s/// makes yylex() call scan_subst() which calls
   scan_str().  tr/// and y/// make yylex() call scan_trans() which
   calls scan_str().

   It skips whitespace before the string starts, and treats the first
   character as the delimiter.  If the delimiter is one of ([{< then
   the corresponding "close" character )]}> is used as the closing
   delimiter.  It allows quoting of delimiters, and if the string has
   balanced delimiters ([{<>}]) it allows nesting.

   On success, the SV with the resulting string is put into lex_stuff or,
   if that is already non-NULL, into lex_repl. The second case occurs only
   when parsing the RHS of the special constructs s/// and tr/// (y///).
   For convenience, the terminating delimiter character is stuffed into
   SvIVX of the SV.
*/

STATIC char *
S_scan_str(pTHX_ char *start, int keep_quoted, int keep_delims)
{
    dVAR;
    SV *sv;				/* scalar value: string */
    const char *tmps;			/* temp string, used for delimiter matching */
    register char *s = start;		/* current position in the buffer */
    register char term;			/* terminating character */
    register char *to;			/* current position in the sv's data */
    I32 brackets = 1;			/* bracket nesting level */
    bool has_utf8 = FALSE;		/* is there any utf8 content? */
    I32 termcode;			/* terminating char. code */
    U8 termstr[UTF8_MAXBYTES];		/* terminating string */
    STRLEN termlen;			/* length of terminating string */
    int last_off = 0;			/* last position for nesting bracket */
#ifdef PERL_MAD
    int stuffstart;
    char *tstart;
#endif

    PERL_ARGS_ASSERT_SCAN_STR;

    /* skip space before the delimiter */
    if (isSPACE(*s)) {
	s = PEEKSPACE(s);
    }

#ifdef PERL_MAD
    if (PL_realtokenstart >= 0) {
	stuffstart = PL_realtokenstart;
	PL_realtokenstart = -1;
    }
    else
	stuffstart = start - SvPVX(PL_linestr);
#endif
    /* mark where we are, in case we need to report errors */
    CLINE;

    /* after skipping whitespace, the next character is the terminator */
    term = *s;
    if (!UTF) {
	termcode = termstr[0] = term;
	termlen = 1;
    }
    else {
	termcode = utf8_to_uvchr_buf((U8*)s, (U8*)PL_bufend, &termlen);
	Copy(s, termstr, termlen, U8);
	if (!UTF8_IS_INVARIANT(term))
	    has_utf8 = TRUE;
    }

    /* mark where we are */
    PL_multi_start = CopLINE(PL_curcop);
    PL_multi_open = term;

    /* find corresponding closing delimiter */
    if (term && (tmps = strchr("([{< )]}> )]}>",term)))
	termcode = termstr[0] = term = tmps[5];

    PL_multi_close = term;

    /* create a new SV to hold the contents.  79 is the SV's initial length.
       What a random number. */
    sv = newSV_type(SVt_PVIV);
    SvGROW(sv, 80);
    SvIV_set(sv, termcode);
    (void)SvPOK_only(sv);		/* validate pointer */

    /* move past delimiter and try to read a complete string */
    if (keep_delims)
	sv_catpvn(sv, s, termlen);
    s += termlen;
#ifdef PERL_MAD
    tstart = SvPVX(PL_linestr) + stuffstart;
    if (!PL_thisopen && !keep_delims) {
	PL_thisopen = newSVpvn(tstart, s - tstart);
	stuffstart = s - SvPVX(PL_linestr);
    }
#endif
    for (;;) {
	if (PL_encoding && !UTF) {
	    bool cont = TRUE;

	    while (cont) {
		int offset = s - SvPVX_const(PL_linestr);
		const bool found = sv_cat_decode(sv, PL_encoding, PL_linestr,
					   &offset, (char*)termstr, termlen);
		const char * const ns = SvPVX_const(PL_linestr) + offset;
		char * const svlast = SvEND(sv) - 1;

		for (; s < ns; s++) {
		    if (*s == '\n' && !PL_rsfp && !PL_parser->filtered)
			CopLINE_inc(PL_curcop);
		}
		if (!found)
		    goto read_more_line;
		else {
		    /* handle quoted delimiters */
		    if (SvCUR(sv) > 1 && *(svlast-1) == '\\') {
			const char *t;
			for (t = svlast-2; t >= SvPVX_const(sv) && *t == '\\';)
			    t--;
			if ((svlast-1 - t) % 2) {
			    if (!keep_quoted) {
				*(svlast-1) = term;
				*svlast = '\0';
				SvCUR_set(sv, SvCUR(sv) - 1);
			    }
			    continue;
			}
		    }
		    if (PL_multi_open == PL_multi_close) {
			cont = FALSE;
		    }
		    else {
			const char *t;
			char *w;
			for (t = w = SvPVX(sv)+last_off; t < svlast; w++, t++) {
			    /* At here, all closes are "was quoted" one,
			       so we don't check PL_multi_close. */
			    if (*t == '\\') {
				if (!keep_quoted && *(t+1) == PL_multi_open)
				    t++;
				else
				    *w++ = *t++;
			    }
			    else if (*t == PL_multi_open)
				brackets++;

			    *w = *t;
			}
			if (w < t) {
			    *w++ = term;
			    *w = '\0';
			    SvCUR_set(sv, w - SvPVX_const(sv));
			}
			last_off = w - SvPVX(sv);
			if (--brackets <= 0)
			    cont = FALSE;
		    }
		}
	    }
	    if (!keep_delims) {
		SvCUR_set(sv, SvCUR(sv) - 1);
		*SvEND(sv) = '\0';
	    }
	    break;
	}

    	/* extend sv if need be */
	SvGROW(sv, SvCUR(sv) + (PL_bufend - s) + 1);
	/* set 'to' to the next character in the sv's string */
	to = SvPVX(sv)+SvCUR(sv);

	/* if open delimiter is the close delimiter read unbridle */
	if (PL_multi_open == PL_multi_close) {
	    for (; s < PL_bufend; s++,to++) {
	    	/* embedded newlines increment the current line number */
		if (*s == '\n' && !PL_rsfp && !PL_parser->filtered)
		    CopLINE_inc(PL_curcop);
		/* handle quoted delimiters */
		if (*s == '\\' && s+1 < PL_bufend && term != '\\') {
		    if (!keep_quoted && s[1] == term)
			s++;
		/* any other quotes are simply copied straight through */
		    else
			*to++ = *s++;
		}
		/* terminate when run out of buffer (the for() condition), or
		   have found the terminator */
		else if (*s == term) {
		    if (termlen == 1)
			break;
		    if (s+termlen <= PL_bufend && memEQ(s, (char*)termstr, termlen))
			break;
		}
		else if (!has_utf8 && !UTF8_IS_INVARIANT((U8)*s) && UTF)
		    has_utf8 = TRUE;
		*to = *s;
	    }
	}
	
	/* if the terminator isn't the same as the start character (e.g.,
	   matched brackets), we have to allow more in the quoting, and
	   be prepared for nested brackets.
	*/
	else {
	    /* read until we run out of string, or we find the terminator */
	    for (; s < PL_bufend; s++,to++) {
	    	/* embedded newlines increment the line count */
		if (*s == '\n' && !PL_rsfp && !PL_parser->filtered)
		    CopLINE_inc(PL_curcop);
		/* backslashes can escape the open or closing characters */
		if (*s == '\\' && s+1 < PL_bufend) {
		    if (!keep_quoted &&
			((s[1] == PL_multi_open) || (s[1] == PL_multi_close)))
			s++;
		    else
			*to++ = *s++;
		}
		/* allow nested opens and closes */
		else if (*s == PL_multi_close && --brackets <= 0)
		    break;
		else if (*s == PL_multi_open)
		    brackets++;
		else if (!has_utf8 && !UTF8_IS_INVARIANT((U8)*s) && UTF)
		    has_utf8 = TRUE;
		*to = *s;
	    }
	}
	/* terminate the copied string and update the sv's end-of-string */
	*to = '\0';
	SvCUR_set(sv, to - SvPVX_const(sv));

	/*
	 * this next chunk reads more into the buffer if we're not done yet
	 */

  	if (s < PL_bufend)
	    break;		/* handle case where we are done yet :-) */

#ifndef PERL_STRICT_CR
	if (to - SvPVX_const(sv) >= 2) {
	    if ((to[-2] == '\r' && to[-1] == '\n') ||
		(to[-2] == '\n' && to[-1] == '\r'))
	    {
		to[-2] = '\n';
		to--;
		SvCUR_set(sv, to - SvPVX_const(sv));
	    }
	    else if (to[-1] == '\r')
		to[-1] = '\n';
	}
	else if (to - SvPVX_const(sv) == 1 && to[-1] == '\r')
	    to[-1] = '\n';
#endif
	
     read_more_line:
	/* if we're out of file, or a read fails, bail and reset the current
	   line marker so we can report where the unterminated string began
	*/
#ifdef PERL_MAD
	if (PL_madskills) {
	    char * const tstart = SvPVX(PL_linestr) + stuffstart;
	    if (PL_thisstuff)
		sv_catpvn(PL_thisstuff, tstart, PL_bufend - tstart);
	    else
		PL_thisstuff = newSVpvn(tstart, PL_bufend - tstart);
	}
#endif
	CopLINE_inc(PL_curcop);
	PL_bufptr = PL_bufend;
	if (!lex_next_chunk(0)) {
	    sv_free(sv);
	    CopLINE_set(PL_curcop, (line_t)PL_multi_start);
	    return NULL;
	}
	s = PL_bufptr;
#ifdef PERL_MAD
	stuffstart = 0;
#endif
    }

    /* at this point, we have successfully read the delimited string */

    if (!PL_encoding || UTF) {
#ifdef PERL_MAD
	if (PL_madskills) {
	    char * const tstart = SvPVX(PL_linestr) + stuffstart;
	    const int len = s - tstart;
	    if (PL_thisstuff)
		sv_catpvn(PL_thisstuff, tstart, len);
	    else
		PL_thisstuff = newSVpvn(tstart, len);
	    if (!PL_thisclose && !keep_delims)
		PL_thisclose = newSVpvn(s,termlen);
	}
#endif

	if (keep_delims)
	    sv_catpvn(sv, s, termlen);
	s += termlen;
    }
#ifdef PERL_MAD
    else {
	if (PL_madskills) {
	    char * const tstart = SvPVX(PL_linestr) + stuffstart;
	    const int len = s - tstart - termlen;
	    if (PL_thisstuff)
		sv_catpvn(PL_thisstuff, tstart, len);
	    else
		PL_thisstuff = newSVpvn(tstart, len);
	    if (!PL_thisclose && !keep_delims)
		PL_thisclose = newSVpvn(s - termlen,termlen);
	}
    }
#endif
    if (has_utf8 || PL_encoding)
	SvUTF8_on(sv);

    PL_multi_end = CopLINE(PL_curcop);

    /* if we allocated too much space, give some back */
    if (SvCUR(sv) + 5 < SvLEN(sv)) {
	SvLEN_set(sv, SvCUR(sv) + 1);
	SvPV_renew(sv, SvLEN(sv));
    }

    /* decide whether this is the first or second quoted string we've read
       for this op
    */

    if (PL_lex_stuff)
	PL_lex_repl = sv;
    else
	PL_lex_stuff = sv;
    return s;
}

/*
  scan_num
  takes: pointer to position in buffer
  returns: pointer to new position in buffer
  side-effects: builds ops for the constant in pl_yylval.op

  Read a number in any of the formats that Perl accepts:

  \d(_?\d)*(\.(\d(_?\d)*)?)?[Ee][\+\-]?(\d(_?\d)*)	12 12.34 12.
  \.\d(_?\d)*[Ee][\+\-]?(\d(_?\d)*)			.34
  0b[01](_?[01])*
  0[0-7](_?[0-7])*
  0x[0-9A-Fa-f](_?[0-9A-Fa-f])*

  Like most scan_ routines, it uses the PL_tokenbuf buffer to hold the
  thing it reads.

  If it reads a number without a decimal point or an exponent, it will
  try converting the number to an integer and see if it can do so
  without loss of precision.
*/

char *
Perl_scan_num(pTHX_ const char *start, YYSTYPE* lvalp)
{
    dVAR;
    register const char *s = start;	/* current position in buffer */
    register char *d;			/* destination in temp buffer */
    register char *e;			/* end of temp buffer */
    NV nv;				/* number read, as a double */
    SV *sv = NULL;			/* place to put the converted number */
    bool floatit;			/* boolean: int or float? */
    const char *lastub = NULL;		/* position of last underbar */
    static char const number_too_long[] = "Number too long";

    PERL_ARGS_ASSERT_SCAN_NUM;

    /* We use the first character to decide what type of number this is */

    switch (*s) {
    default:
	Perl_croak(aTHX_ "panic: scan_num, *s=%d", *s);

    /* if it starts with a 0, it could be an octal number, a decimal in
       0.13 disguise, or a hexadecimal number, or a binary number. */
    case '0':
	{
	  /* variables:
	     u		holds the "number so far"
	     shift	the power of 2 of the base
			(hex == 4, octal == 3, binary == 1)
	     overflowed	was the number more than we can hold?

	     Shift is used when we add a digit.  It also serves as an "are
	     we in octal/hex/binary?" indicator to disallow hex characters
	     when in octal mode.
	   */
	    NV n = 0.0;
	    UV u = 0;
	    I32 shift;
	    bool overflowed = FALSE;
	    bool just_zero  = TRUE;	/* just plain 0 or binary number? */
	    static const NV nvshift[5] = { 1.0, 2.0, 4.0, 8.0, 16.0 };
	    static const char* const bases[5] =
	      { "", "binary", "", "octal", "hexadecimal" };
	    static const char* const Bases[5] =
	      { "", "Binary", "", "Octal", "Hexadecimal" };
	    static const char* const maxima[5] =
	      { "",
		"0b11111111111111111111111111111111",
		"",
		"037777777777",
		"0xffffffff" };
	    const char *base, *Base, *max;

	    /* check for hex */
	    if (s[1] == 'x' || s[1] == 'X') {
		shift = 4;
		s += 2;
		just_zero = FALSE;
	    } else if (s[1] == 'b' || s[1] == 'B') {
		shift = 1;
		s += 2;
		just_zero = FALSE;
	    }
	    /* check for a decimal in disguise */
	    else if (s[1] == '.' || s[1] == 'e' || s[1] == 'E')
		goto decimal;
	    /* so it must be octal */
	    else {
		shift = 3;
		s++;
	    }

	    if (*s == '_') {
		Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
			       "Misplaced _ in number");
	       lastub = s++;
	    }

	    base = bases[shift];
	    Base = Bases[shift];
	    max  = maxima[shift];

	    /* read the rest of the number */
	    for (;;) {
		/* x is used in the overflow test,
		   b is the digit we're adding on. */
		UV x, b;

		switch (*s) {

		/* if we don't mention it, we're done */
		default:
		    goto out;

		/* _ are ignored -- but warned about if consecutive */
		case '_':
		    if (lastub && s == lastub + 1)
		        Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
				       "Misplaced _ in number");
		    lastub = s++;
		    break;

		/* 8 and 9 are not octal */
		case '8': case '9':
		    if (shift == 3)
			yyerror(Perl_form(aTHX_ "Illegal octal digit '%c'", *s));
		    /* FALL THROUGH */

	        /* octal digits */
		case '2': case '3': case '4':
		case '5': case '6': case '7':
		    if (shift == 1)
			yyerror(Perl_form(aTHX_ "Illegal binary digit '%c'", *s));
		    /* FALL THROUGH */

		case '0': case '1':
		    b = *s++ & 15;		/* ASCII digit -> value of digit */
		    goto digit;

	        /* hex digits */
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		    /* make sure they said 0x */
		    if (shift != 4)
			goto out;
		    b = (*s++ & 7) + 9;

		    /* Prepare to put the digit we have onto the end
		       of the number so far.  We check for overflows.
		    */

		  digit:
		    just_zero = FALSE;
		    if (!overflowed) {
			x = u << shift;	/* make room for the digit */

			if ((x >> shift) != u
			    && !(PL_hints & HINT_NEW_BINARY)) {
			    overflowed = TRUE;
			    n = (NV) u;
			    Perl_ck_warner_d(aTHX_ packWARN(WARN_OVERFLOW),
					     "Integer overflow in %s number",
					     base);
			} else
			    u = x | b;		/* add the digit to the end */
		    }
		    if (overflowed) {
			n *= nvshift[shift];
			/* If an NV has not enough bits in its
			 * mantissa to represent an UV this summing of
			 * small low-order numbers is a waste of time
			 * (because the NV cannot preserve the
			 * low-order bits anyway): we could just
			 * remember when did we overflow and in the
			 * end just multiply n by the right
			 * amount. */
			n += (NV) b;
		    }
		    break;
		}
	    }

	  /* if we get here, we had success: make a scalar value from
	     the number.
	  */
	  out:

	    /* final misplaced underbar check */
	    if (s[-1] == '_') {
		Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX), "Misplaced _ in number");
	    }

	    if (overflowed) {
		if (n > 4294967295.0)
		    Perl_ck_warner(aTHX_ packWARN(WARN_PORTABLE),
				   "%s number > %s non-portable",
				   Base, max);
		sv = newSVnv(n);
	    }
	    else {
#if UVSIZE > 4
		if (u > 0xffffffff)
		    Perl_ck_warner(aTHX_ packWARN(WARN_PORTABLE),
				   "%s number > %s non-portable",
				   Base, max);
#endif
		sv = newSVuv(u);
	    }
	    if (just_zero && (PL_hints & HINT_NEW_INTEGER))
		sv = new_constant(start, s - start, "integer",
				  sv, NULL, NULL, 0);
	    else if (PL_hints & HINT_NEW_BINARY)
		sv = new_constant(start, s - start, "binary", sv, NULL, NULL, 0);
	}
	break;

    /*
      handle decimal numbers.
      we're also sent here when we read a 0 as the first digit
    */
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9': case '.':
      decimal:
	d = PL_tokenbuf;
	e = PL_tokenbuf + sizeof PL_tokenbuf - 6; /* room for various punctuation */
	floatit = FALSE;

	/* read next group of digits and _ and copy into d */
	while (isDIGIT(*s) || *s == '_') {
	    /* skip underscores, checking for misplaced ones
	       if -w is on
	    */
	    if (*s == '_') {
		if (lastub && s == lastub + 1)
		    Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
				   "Misplaced _ in number");
		lastub = s++;
	    }
	    else {
	        /* check for end of fixed-length buffer */
		if (d >= e)
		    Perl_croak(aTHX_ number_too_long);
		/* if we're ok, copy the character */
		*d++ = *s++;
	    }
	}

	/* final misplaced underbar check */
	if (lastub && s == lastub + 1) {
	    Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX), "Misplaced _ in number");
	}

	/* read a decimal portion if there is one.  avoid
	   3..5 being interpreted as the number 3. followed
	   by .5
	*/
	if (*s == '.' && s[1] != '.') {
	    floatit = TRUE;
	    *d++ = *s++;

	    if (*s == '_') {
		Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
			       "Misplaced _ in number");
		lastub = s;
	    }

	    /* copy, ignoring underbars, until we run out of digits.
	    */
	    for (; isDIGIT(*s) || *s == '_'; s++) {
	        /* fixed length buffer check */
		if (d >= e)
		    Perl_croak(aTHX_ number_too_long);
		if (*s == '_') {
		   if (lastub && s == lastub + 1)
		       Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
				      "Misplaced _ in number");
		   lastub = s;
		}
		else
		    *d++ = *s;
	    }
	    /* fractional part ending in underbar? */
	    if (s[-1] == '_') {
		Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
			       "Misplaced _ in number");
	    }
	    if (*s == '.' && isDIGIT(s[1])) {
		/* oops, it's really a v-string, but without the "v" */
		s = start;
		goto vstring;
	    }
	}

	/* read exponent part, if present */
	if ((*s == 'e' || *s == 'E') && strchr("+-0123456789_", s[1])) {
	    floatit = TRUE;
	    s++;

	    /* regardless of whether user said 3E5 or 3e5, use lower 'e' */
	    *d++ = 'e';		/* At least some Mach atof()s don't grok 'E' */

	    /* stray preinitial _ */
	    if (*s == '_') {
		Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
			       "Misplaced _ in number");
	        lastub = s++;
	    }

	    /* allow positive or negative exponent */
	    if (*s == '+' || *s == '-')
		*d++ = *s++;

	    /* stray initial _ */
	    if (*s == '_') {
		Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
			       "Misplaced _ in number");
	        lastub = s++;
	    }

	    /* read digits of exponent */
	    while (isDIGIT(*s) || *s == '_') {
	        if (isDIGIT(*s)) {
		    if (d >= e)
		        Perl_croak(aTHX_ number_too_long);
		    *d++ = *s++;
		}
		else {
		   if (((lastub && s == lastub + 1) ||
			(!isDIGIT(s[1]) && s[1] != '_')))
		       Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
				      "Misplaced _ in number");
		   lastub = s++;
		}
	    }
	}


	/*
           We try to do an integer conversion first if no characters
           indicating "float" have been found.
	 */

	if (!floatit) {
    	    UV uv;
	    const int flags = grok_number (PL_tokenbuf, d - PL_tokenbuf, &uv);

            if (flags == IS_NUMBER_IN_UV) {
              if (uv <= IV_MAX)
		sv = newSViv(uv); /* Prefer IVs over UVs. */
              else
	    	sv = newSVuv(uv);
            } else if (flags == (IS_NUMBER_IN_UV | IS_NUMBER_NEG)) {
              if (uv <= (UV) IV_MIN)
                sv = newSViv(-(IV)uv);
              else
	    	floatit = TRUE;
            } else
              floatit = TRUE;
        }
	if (floatit) {
	    /* terminate the string */
	    *d = '\0';
	    nv = Atof(PL_tokenbuf);
	    sv = newSVnv(nv);
	}

	if ( floatit
	     ? (PL_hints & HINT_NEW_FLOAT) : (PL_hints & HINT_NEW_INTEGER) ) {
	    const char *const key = floatit ? "float" : "integer";
	    const STRLEN keylen = floatit ? 5 : 7;
	    sv = S_new_constant(aTHX_ PL_tokenbuf, d - PL_tokenbuf,
				key, keylen, sv, NULL, NULL, 0);
	}
	break;

    /* if it starts with a v, it could be a v-string */
    case 'v':
vstring:
		sv = newSV(5); /* preallocate storage space */
		s = scan_vstring(s, PL_bufend, sv);
	break;
    }

    /* make the op for the constant and return */

    if (sv)
	lvalp->opval = newSVOP(OP_CONST, 0, sv);
    else
	lvalp->opval = NULL;

    return (char *)s;
}

STATIC char *
S_scan_formline(pTHX_ register char *s)
{
    dVAR;
    register char *eol;
    register char *t;
    SV * const stuff = newSVpvs("");
    bool needargs = FALSE;
    bool eofmt = FALSE;
#ifdef PERL_MAD
    char *tokenstart = s;
    SV* savewhite = NULL;

    if (PL_madskills) {
	savewhite = PL_thiswhite;
	PL_thiswhite = 0;
    }
#endif

    PERL_ARGS_ASSERT_SCAN_FORMLINE;

    while (!needargs) {
	if (*s == '.') {
	    t = s+1;
#ifdef PERL_STRICT_CR
	    while (SPACE_OR_TAB(*t))
		t++;
#else
	    while (SPACE_OR_TAB(*t) || *t == '\r')
		t++;
#endif
	    if (*t == '\n' || t == PL_bufend) {
	        eofmt = TRUE;
		break;
            }
	}
	if (PL_in_eval && !PL_rsfp && !PL_parser->filtered) {
	    eol = (char *) memchr(s,'\n',PL_bufend-s);
	    if (!eol++)
		eol = PL_bufend;
	}
	else
	    eol = PL_bufend = SvPVX(PL_linestr) + SvCUR(PL_linestr);
	if (*s != '#') {
	    for (t = s; t < eol; t++) {
		if (*t == '~' && t[1] == '~' && SvCUR(stuff)) {
		    needargs = FALSE;
		    goto enough;	/* ~~ must be first line in formline */
		}
		if (*t == '@' || *t == '^')
		    needargs = TRUE;
	    }
	    if (eol > s) {
	        sv_catpvn(stuff, s, eol-s);
#ifndef PERL_STRICT_CR
		if (eol-s > 1 && eol[-2] == '\r' && eol[-1] == '\n') {
		    char *end = SvPVX(stuff) + SvCUR(stuff);
		    end[-2] = '\n';
		    end[-1] = '\0';
		    SvCUR_set(stuff, SvCUR(stuff) - 1);
		}
#endif
	    }
	    else
	      break;
	}
	s = (char*)eol;
	if (PL_rsfp || PL_parser->filtered) {
	    bool got_some;
#ifdef PERL_MAD
	    if (PL_madskills) {
		if (PL_thistoken)
		    sv_catpvn(PL_thistoken, tokenstart, PL_bufend - tokenstart);
		else
		    PL_thistoken = newSVpvn(tokenstart, PL_bufend - tokenstart);
	    }
#endif
	    PL_bufptr = PL_bufend;
	    CopLINE_inc(PL_curcop);
	    got_some = lex_next_chunk(0);
	    CopLINE_dec(PL_curcop);
	    s = PL_bufptr;
#ifdef PERL_MAD
	    tokenstart = PL_bufptr;
#endif
	    if (!got_some)
		break;
	}
	incline(s);
    }
  enough:
    if (SvCUR(stuff)) {
	PL_expect = XTERM;
	if (needargs) {
	    PL_lex_state = LEX_NORMAL;
	    start_force(PL_curforce);
	    NEXTVAL_NEXTTOKE.ival = 0;
	    force_next(',');
	}
	else
	    PL_lex_state = LEX_FORMLINE;
	if (!IN_BYTES) {
	    if (UTF && is_utf8_string((U8*)SvPVX_const(stuff), SvCUR(stuff)))
		SvUTF8_on(stuff);
	    else if (PL_encoding)
		sv_recode_to_utf8(stuff, PL_encoding);
	}
	start_force(PL_curforce);
	NEXTVAL_NEXTTOKE.opval = (OP*)newSVOP(OP_CONST, 0, stuff);
	force_next(THING);
	start_force(PL_curforce);
	NEXTVAL_NEXTTOKE.ival = OP_FORMLINE;
	force_next(LSTOP);
    }
    else {
	SvREFCNT_dec(stuff);
	if (eofmt)
	    PL_lex_formbrack = 0;
	PL_bufptr = s;
    }
#ifdef PERL_MAD
    if (PL_madskills) {
	if (PL_thistoken)
	    sv_catpvn(PL_thistoken, tokenstart, s - tokenstart);
	else
	    PL_thistoken = newSVpvn(tokenstart, s - tokenstart);
	PL_thiswhite = savewhite;
    }
#endif
    return s;
}

I32
Perl_start_subparse(pTHX_ I32 is_format, U32 flags)
{
    dVAR;
    const I32 oldsavestack_ix = PL_savestack_ix;
    CV* const outsidecv = PL_compcv;

    if (PL_compcv) {
	assert(SvTYPE(PL_compcv) == SVt_PVCV);
    }
    SAVEI32(PL_subline);
    save_item(PL_subname);
    SAVESPTR(PL_compcv);

    PL_compcv = MUTABLE_CV(newSV_type(is_format ? SVt_PVFM : SVt_PVCV));
    CvFLAGS(PL_compcv) |= flags;

    PL_subline = CopLINE(PL_curcop);
    CvPADLIST(PL_compcv) = pad_new(padnew_SAVE|padnew_SAVESUB);
    CvOUTSIDE(PL_compcv) = MUTABLE_CV(SvREFCNT_inc_simple(outsidecv));
    CvOUTSIDE_SEQ(PL_compcv) = PL_cop_seqmax;

    return oldsavestack_ix;
}

#ifdef __SC__
#pragma segment Perl_yylex
#endif
static int
S_yywarn(pTHX_ const char *const s, U32 flags)
{
    dVAR;

    PERL_ARGS_ASSERT_YYWARN;

    PL_in_eval |= EVAL_WARNONLY;
    yyerror_pv(s, flags);
    PL_in_eval &= ~EVAL_WARNONLY;
    return 0;
}

int
Perl_yyerror(pTHX_ const char *const s)
{
    PERL_ARGS_ASSERT_YYERROR;
    return yyerror_pvn(s, strlen(s), 0);
}

int
Perl_yyerror_pv(pTHX_ const char *const s, U32 flags)
{
    PERL_ARGS_ASSERT_YYERROR_PV;
    return yyerror_pvn(s, strlen(s), flags);
}

int
Perl_yyerror_pvn(pTHX_ const char *const s, STRLEN len, U32 flags)
{
    dVAR;
    const char *context = NULL;
    int contlen = -1;
    SV *msg;
    SV * const where_sv = newSVpvs_flags("", SVs_TEMP);
    int yychar  = PL_parser->yychar;
    U32 is_utf8 = flags & SVf_UTF8;

    PERL_ARGS_ASSERT_YYERROR_PVN;

    if (!yychar || (yychar == ';' && !PL_rsfp))
	sv_catpvs(where_sv, "at EOF");
    else if (PL_oldoldbufptr && PL_bufptr > PL_oldoldbufptr &&
      PL_bufptr - PL_oldoldbufptr < 200 && PL_oldoldbufptr != PL_oldbufptr &&
      PL_oldbufptr != PL_bufptr) {
	/*
		Only for NetWare:
		The code below is removed for NetWare because it abends/crashes on NetWare
		when the script has error such as not having the closing quotes like:
		    if ($var eq "value)
		Checking of white spaces is anyway done in NetWare code.
	*/
#ifndef NETWARE
	while (isSPACE(*PL_oldoldbufptr))
	    PL_oldoldbufptr++;
#endif
	context = PL_oldoldbufptr;
	contlen = PL_bufptr - PL_oldoldbufptr;
    }
    else if (PL_oldbufptr && PL_bufptr > PL_oldbufptr &&
      PL_bufptr - PL_oldbufptr < 200 && PL_oldbufptr != PL_bufptr) {
	/*
		Only for NetWare:
		The code below is removed for NetWare because it abends/crashes on NetWare
		when the script has error such as not having the closing quotes like:
		    if ($var eq "value)
		Checking of white spaces is anyway done in NetWare code.
	*/
#ifndef NETWARE
	while (isSPACE(*PL_oldbufptr))
	    PL_oldbufptr++;
#endif
	context = PL_oldbufptr;
	contlen = PL_bufptr - PL_oldbufptr;
    }
    else if (yychar > 255)
	sv_catpvs(where_sv, "next token ???");
    else if (yychar == -2) { /* YYEMPTY */
	if (PL_lex_state == LEX_NORMAL ||
	   (PL_lex_state == LEX_KNOWNEXT && PL_lex_defer == LEX_NORMAL))
	    sv_catpvs(where_sv, "at end of line");
	else if (PL_lex_inpat)
	    sv_catpvs(where_sv, "within pattern");
	else
	    sv_catpvs(where_sv, "within string");
    }
    else {
	sv_catpvs(where_sv, "next char ");
	if (yychar < 32)
	    Perl_sv_catpvf(aTHX_ where_sv, "^%c", toCTRL(yychar));
	else if (isPRINT_LC(yychar)) {
	    const char string = yychar;
	    sv_catpvn(where_sv, &string, 1);
	}
	else
	    Perl_sv_catpvf(aTHX_ where_sv, "\\%03o", yychar & 255);
    }
    msg = sv_2mortal(newSVpvn_flags(s, len, is_utf8));
    Perl_sv_catpvf(aTHX_ msg, " at %s line %"IVdf", ",
        OutCopFILE(PL_curcop), (IV)CopLINE(PL_curcop));
    if (context)
	Perl_sv_catpvf(aTHX_ msg, "near \"%"SVf"\"\n",
                            SVfARG(newSVpvn_flags(context, contlen,
                                        SVs_TEMP | (UTF ? SVf_UTF8 : 0))));
    else
	Perl_sv_catpvf(aTHX_ msg, "%"SVf"\n", SVfARG(where_sv));
    if (PL_multi_start < PL_multi_end && (U32)(CopLINE(PL_curcop) - PL_multi_end) <= 1) {
        Perl_sv_catpvf(aTHX_ msg,
        "  (Might be a runaway multi-line %c%c string starting on line %"IVdf")\n",
                (int)PL_multi_open,(int)PL_multi_close,(IV)PL_multi_start);
        PL_multi_end = 0;
    }
    if (PL_in_eval & EVAL_WARNONLY) {
	Perl_ck_warner_d(aTHX_ packWARN(WARN_SYNTAX), "%"SVf, SVfARG(msg));
    }
    else
	qerror(msg);
    if (PL_error_count >= 10) {
	if (PL_in_eval && SvCUR(ERRSV))
	    Perl_croak(aTHX_ "%"SVf"%s has too many errors.\n",
		       SVfARG(ERRSV), OutCopFILE(PL_curcop));
	else
	    Perl_croak(aTHX_ "%s has too many errors.\n",
            OutCopFILE(PL_curcop));
    }
    PL_in_my = 0;
    PL_in_my_stash = NULL;
    return 0;
}
#ifdef __SC__
#pragma segment Main
#endif

STATIC char*
S_swallow_bom(pTHX_ U8 *s)
{
    dVAR;
    const STRLEN slen = SvCUR(PL_linestr);

    PERL_ARGS_ASSERT_SWALLOW_BOM;

    switch (s[0]) {
    case 0xFF:
	if (s[1] == 0xFE) {
	    /* UTF-16 little-endian? (or UTF-32LE?) */
	    if (s[2] == 0 && s[3] == 0)  /* UTF-32 little-endian */
		/* diag_listed_as: Unsupported script encoding %s */
		Perl_croak(aTHX_ "Unsupported script encoding UTF-32LE");
#ifndef PERL_NO_UTF16_FILTER
	    if (DEBUG_p_TEST || DEBUG_T_TEST) PerlIO_printf(Perl_debug_log, "UTF-16LE script encoding (BOM)\n");
	    s += 2;
	    if (PL_bufend > (char*)s) {
		s = add_utf16_textfilter(s, TRUE);
	    }
#else
	    /* diag_listed_as: Unsupported script encoding %s */
	    Perl_croak(aTHX_ "Unsupported script encoding UTF-16LE");
#endif
	}
	break;
    case 0xFE:
	if (s[1] == 0xFF) {   /* UTF-16 big-endian? */
#ifndef PERL_NO_UTF16_FILTER
	    if (DEBUG_p_TEST || DEBUG_T_TEST) PerlIO_printf(Perl_debug_log, "UTF-16BE script encoding (BOM)\n");
	    s += 2;
	    if (PL_bufend > (char *)s) {
		s = add_utf16_textfilter(s, FALSE);
	    }
#else
	    /* diag_listed_as: Unsupported script encoding %s */
	    Perl_croak(aTHX_ "Unsupported script encoding UTF-16BE");
#endif
	}
	break;
    case 0xEF:
	if (slen > 2 && s[1] == 0xBB && s[2] == 0xBF) {
	    if (DEBUG_p_TEST || DEBUG_T_TEST) PerlIO_printf(Perl_debug_log, "UTF-8 script encoding (BOM)\n");
	    s += 3;                      /* UTF-8 */
	}
	break;
    case 0:
	if (slen > 3) {
	     if (s[1] == 0) {
		  if (s[2] == 0xFE && s[3] == 0xFF) {
		       /* UTF-32 big-endian */
		       /* diag_listed_as: Unsupported script encoding %s */
		       Perl_croak(aTHX_ "Unsupported script encoding UTF-32BE");
		  }
	     }
	     else if (s[2] == 0 && s[3] != 0) {
		  /* Leading bytes
		   * 00 xx 00 xx
		   * are a good indicator of UTF-16BE. */
#ifndef PERL_NO_UTF16_FILTER
		  if (DEBUG_p_TEST || DEBUG_T_TEST) PerlIO_printf(Perl_debug_log, "UTF-16BE script encoding (no BOM)\n");
		  s = add_utf16_textfilter(s, FALSE);
#else
		  /* diag_listed_as: Unsupported script encoding %s */
		  Perl_croak(aTHX_ "Unsupported script encoding UTF-16BE");
#endif
	     }
	}
#ifdef EBCDIC
    case 0xDD:
        if (slen > 3 && s[1] == 0x73 && s[2] == 0x66 && s[3] == 0x73) {
            if (DEBUG_p_TEST || DEBUG_T_TEST) PerlIO_printf(Perl_debug_log, "UTF-8 script encoding (BOM)\n");
            s += 4;                      /* UTF-8 */
        }
        break;
#endif

    default:
	 if (slen > 3 && s[1] == 0 && s[2] != 0 && s[3] == 0) {
		  /* Leading bytes
		   * xx 00 xx 00
		   * are a good indicator of UTF-16LE. */
#ifndef PERL_NO_UTF16_FILTER
	      if (DEBUG_p_TEST || DEBUG_T_TEST) PerlIO_printf(Perl_debug_log, "UTF-16LE script encoding (no BOM)\n");
	      s = add_utf16_textfilter(s, TRUE);
#else
	      /* diag_listed_as: Unsupported script encoding %s */
	      Perl_croak(aTHX_ "Unsupported script encoding UTF-16LE");
#endif
	 }
    }
    return (char*)s;
}


#ifndef PERL_NO_UTF16_FILTER
static I32
S_utf16_textfilter(pTHX_ int idx, SV *sv, int maxlen)
{
    dVAR;
    SV *const filter = FILTER_DATA(idx);
    /* We re-use this each time round, throwing the contents away before we
       return.  */
    SV *const utf16_buffer = MUTABLE_SV(IoTOP_GV(filter));
    SV *const utf8_buffer = filter;
    IV status = IoPAGE(filter);
    const bool reverse = cBOOL(IoLINES(filter));
    I32 retval;

    PERL_ARGS_ASSERT_UTF16_TEXTFILTER;

    /* As we're automatically added, at the lowest level, and hence only called
       from this file, we can be sure that we're not called in block mode. Hence
       don't bother writing code to deal with block mode.  */
    if (maxlen) {
	Perl_croak(aTHX_ "panic: utf16_textfilter called in block mode (for %d characters)", maxlen);
    }
    if (status < 0) {
	Perl_croak(aTHX_ "panic: utf16_textfilter called after error (status=%"IVdf")", status);
    }
    DEBUG_P(PerlIO_printf(Perl_debug_log,
			  "utf16_textfilter(%p,%ce): idx=%d maxlen=%d status=%"IVdf" utf16=%"UVuf" utf8=%"UVuf"\n",
			  FPTR2DPTR(void *, S_utf16_textfilter),
			  reverse ? 'l' : 'b', idx, maxlen, status,
			  (UV)SvCUR(utf16_buffer), (UV)SvCUR(utf8_buffer)));

    while (1) {
	STRLEN chars;
	STRLEN have;
	I32 newlen;
	U8 *end;
	/* First, look in our buffer of existing UTF-8 data:  */
	char *nl = (char *)memchr(SvPVX(utf8_buffer), '\n', SvCUR(utf8_buffer));

	if (nl) {
	    ++nl;
	} else if (status == 0) {
	    /* EOF */
	    IoPAGE(filter) = 0;
	    nl = SvEND(utf8_buffer);
	}
	if (nl) {
	    STRLEN got = nl - SvPVX(utf8_buffer);
	    /* Did we have anything to append?  */
	    retval = got != 0;
	    sv_catpvn(sv, SvPVX(utf8_buffer), got);
	    /* Everything else in this code works just fine if SVp_POK isn't
	       set.  This, however, needs it, and we need it to work, else
	       we loop infinitely because the buffer is never consumed.  */
	    sv_chop(utf8_buffer, nl);
	    break;
	}

	/* OK, not a complete line there, so need to read some more UTF-16.
	   Read an extra octect if the buffer currently has an odd number. */
	while (1) {
	    if (status <= 0)
		break;
	    if (SvCUR(utf16_buffer) >= 2) {
		/* Location of the high octet of the last complete code point.
		   Gosh, UTF-16 is a pain. All the benefits of variable length,
		   *coupled* with all the benefits of partial reads and
		   endianness.  */
		const U8 *const last_hi = (U8*)SvPVX(utf16_buffer)
		    + ((SvCUR(utf16_buffer) & ~1) - (reverse ? 1 : 2));

		if (*last_hi < 0xd8 || *last_hi > 0xdb) {
		    break;
		}

		/* We have the first half of a surrogate. Read more.  */
		DEBUG_P(PerlIO_printf(Perl_debug_log, "utf16_textfilter partial surrogate detected at %p\n", last_hi));
	    }

	    status = FILTER_READ(idx + 1, utf16_buffer,
				 160 + (SvCUR(utf16_buffer) & 1));
	    DEBUG_P(PerlIO_printf(Perl_debug_log, "utf16_textfilter status=%"IVdf" SvCUR(sv)=%"UVuf"\n", status, (UV)SvCUR(utf16_buffer)));
	    DEBUG_P({ sv_dump(utf16_buffer); sv_dump(utf8_buffer);});
	    if (status < 0) {
		/* Error */
		IoPAGE(filter) = status;
		return status;
	    }
	}

	chars = SvCUR(utf16_buffer) >> 1;
	have = SvCUR(utf8_buffer);
	SvGROW(utf8_buffer, have + chars * 3 + 1);

	if (reverse) {
	    end = utf16_to_utf8_reversed((U8*)SvPVX(utf16_buffer),
					 (U8*)SvPVX_const(utf8_buffer) + have,
					 chars * 2, &newlen);
	} else {
	    end = utf16_to_utf8((U8*)SvPVX(utf16_buffer),
				(U8*)SvPVX_const(utf8_buffer) + have,
				chars * 2, &newlen);
	}
	SvCUR_set(utf8_buffer, have + newlen);
	*end = '\0';

	/* No need to keep this SV "well-formed" with a '\0' after the end, as
	   it's private to us, and utf16_to_utf8{,reversed} take a
	   (pointer,length) pair, rather than a NUL-terminated string.  */
	if(SvCUR(utf16_buffer) & 1) {
	    *SvPVX(utf16_buffer) = SvEND(utf16_buffer)[-1];
	    SvCUR_set(utf16_buffer, 1);
	} else {
	    SvCUR_set(utf16_buffer, 0);
	}
    }
    DEBUG_P(PerlIO_printf(Perl_debug_log,
			  "utf16_textfilter: returns, status=%"IVdf" utf16=%"UVuf" utf8=%"UVuf"\n",
			  status,
			  (UV)SvCUR(utf16_buffer), (UV)SvCUR(utf8_buffer)));
    DEBUG_P({ sv_dump(utf8_buffer); sv_dump(sv);});
    return retval;
}

static U8 *
S_add_utf16_textfilter(pTHX_ U8 *const s, bool reversed)
{
    SV *filter = filter_add(S_utf16_textfilter, NULL);

    PERL_ARGS_ASSERT_ADD_UTF16_TEXTFILTER;

    IoTOP_GV(filter) = MUTABLE_GV(newSVpvn((char *)s, PL_bufend - (char*)s));
    sv_setpvs(filter, "");
    IoLINES(filter) = reversed;
    IoPAGE(filter) = 1; /* Not EOF */

    /* Sadly, we have to return a valid pointer, come what may, so we have to
       ignore any error return from this.  */
    SvCUR_set(PL_linestr, 0);
    if (FILTER_READ(0, PL_linestr, 0)) {
	SvUTF8_on(PL_linestr);
    } else {
	SvUTF8_on(PL_linestr);
    }
    PL_bufend = SvEND(PL_linestr);
    return (U8*)SvPVX(PL_linestr);
}
#endif

/*
Returns a pointer to the next character after the parsed
vstring, as well as updating the passed in sv.

Function must be called like

	sv = newSV(5);
	s = scan_vstring(s,e,sv);

where s and e are the start and end of the string.
The sv should already be large enough to store the vstring
passed in, for performance reasons.

*/

char *
Perl_scan_vstring(pTHX_ const char *s, const char *const e, SV *sv)
{
    dVAR;
    const char *pos = s;
    const char *start = s;

    PERL_ARGS_ASSERT_SCAN_VSTRING;

    if (*pos == 'v') pos++;  /* get past 'v' */
    while (pos < e && (isDIGIT(*pos) || *pos == '_'))
	pos++;
    if ( *pos != '.') {
	/* this may not be a v-string if followed by => */
	const char *next = pos;
	while (next < e && isSPACE(*next))
	    ++next;
	if ((e - next) >= 2 && *next == '=' && next[1] == '>' ) {
	    /* return string not v-string */
	    sv_setpvn(sv,(char *)s,pos-s);
	    return (char *)pos;
	}
    }

    if (!isALPHA(*pos)) {
	U8 tmpbuf[UTF8_MAXBYTES+1];

	if (*s == 'v')
	    s++;  /* get past 'v' */

	sv_setpvs(sv, "");

	for (;;) {
	    /* this is atoi() that tolerates underscores */
	    U8 *tmpend;
	    UV rev = 0;
	    const char *end = pos;
	    UV mult = 1;
	    while (--end >= s) {
		if (*end != '_') {
		    const UV orev = rev;
		    rev += (*end - '0') * mult;
		    mult *= 10;
		    if (orev > rev)
			/* diag_listed_as: Integer overflow in %s number */
			Perl_ck_warner_d(aTHX_ packWARN(WARN_OVERFLOW),
					 "Integer overflow in decimal number");
		}
	    }
#ifdef EBCDIC
	    if (rev > 0x7FFFFFFF)
		 Perl_croak(aTHX_ "In EBCDIC the v-string components cannot exceed 2147483647");
#endif
	    /* Append native character for the rev point */
	    tmpend = uvchr_to_utf8(tmpbuf, rev);
	    sv_catpvn(sv, (const char*)tmpbuf, tmpend - tmpbuf);
	    if (!UNI_IS_INVARIANT(NATIVE_TO_UNI(rev)))
		 SvUTF8_on(sv);
	    if (pos + 1 < e && *pos == '.' && isDIGIT(pos[1]))
		 s = ++pos;
	    else {
		 s = pos;
		 break;
	    }
	    while (pos < e && (isDIGIT(*pos) || *pos == '_'))
		 pos++;
	}
	SvPOK_on(sv);
	sv_magic(sv,NULL,PERL_MAGIC_vstring,(const char*)start, pos-start);
	SvRMAGICAL_on(sv);
    }
    return (char *)s;
}

int
Perl_keyword_plugin_standard(pTHX_
	char *keyword_ptr, STRLEN keyword_len, OP **op_ptr)
{
    PERL_ARGS_ASSERT_KEYWORD_PLUGIN_STANDARD;
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(keyword_ptr);
    PERL_UNUSED_ARG(keyword_len);
    PERL_UNUSED_ARG(op_ptr);
    return KEYWORD_PLUGIN_DECLINE;
}

#define parse_recdescent(g,p) S_parse_recdescent(aTHX_ g,p)
static void
S_parse_recdescent(pTHX_ int gramtype, I32 fakeeof)
{
    SAVEI32(PL_lex_brackets);
    if (PL_lex_brackets > 100)
	Renew(PL_lex_brackstack, PL_lex_brackets + 10, char);
    PL_lex_brackstack[PL_lex_brackets++] = XFAKEEOF;
    SAVEI32(PL_lex_allbrackets);
    PL_lex_allbrackets = 0;
    SAVEI8(PL_lex_fakeeof);
    PL_lex_fakeeof = (U8)fakeeof;
    if(yyparse(gramtype) && !PL_parser->error_count)
	qerror(Perl_mess(aTHX_ "Parse error"));
}

#define parse_recdescent_for_op(g,p) S_parse_recdescent_for_op(aTHX_ g,p)
static OP *
S_parse_recdescent_for_op(pTHX_ int gramtype, I32 fakeeof)
{
    OP *o;
    ENTER;
    SAVEVPTR(PL_eval_root);
    PL_eval_root = NULL;
    parse_recdescent(gramtype, fakeeof);
    o = PL_eval_root;
    LEAVE;
    return o;
}

#define parse_expr(p,f) S_parse_expr(aTHX_ p,f)
static OP *
S_parse_expr(pTHX_ I32 fakeeof, U32 flags)
{
    OP *exprop;
    if (flags & ~PARSE_OPTIONAL)
	Perl_croak(aTHX_ "Parsing code internal error (%s)", "parse_expr");
    exprop = parse_recdescent_for_op(GRAMEXPR, fakeeof);
    if (!exprop && !(flags & PARSE_OPTIONAL)) {
	if (!PL_parser->error_count)
	    qerror(Perl_mess(aTHX_ "Parse error"));
	exprop = newOP(OP_NULL, 0);
    }
    return exprop;
}

/*
=for apidoc Amx|OP *|parse_arithexpr|U32 flags

Parse a Perl arithmetic expression.  This may contain operators of precedence
down to the bit shift operators.  The expression must be followed (and thus
terminated) either by a comparison or lower-precedence operator or by
something that would normally terminate an expression such as semicolon.
If I<flags> includes C<PARSE_OPTIONAL> then the expression is optional,
otherwise it is mandatory.  It is up to the caller to ensure that the
dynamic parser state (L</PL_parser> et al) is correctly set to reflect
the source of the code to be parsed and the lexical context for the
expression.

The op tree representing the expression is returned.  If an optional
expression is absent, a null pointer is returned, otherwise the pointer
will be non-null.

If an error occurs in parsing or compilation, in most cases a valid op
tree is returned anyway.  The error is reflected in the parser state,
normally resulting in a single exception at the top level of parsing
which covers all the compilation errors that occurred.  Some compilation
errors, however, will throw an exception immediately.

=cut
*/

OP *
Perl_parse_arithexpr(pTHX_ U32 flags)
{
    return parse_expr(LEX_FAKEEOF_COMPARE, flags);
}

/*
=for apidoc Amx|OP *|parse_termexpr|U32 flags

Parse a Perl term expression.  This may contain operators of precedence
down to the assignment operators.  The expression must be followed (and thus
terminated) either by a comma or lower-precedence operator or by
something that would normally terminate an expression such as semicolon.
If I<flags> includes C<PARSE_OPTIONAL> then the expression is optional,
otherwise it is mandatory.  It is up to the caller to ensure that the
dynamic parser state (L</PL_parser> et al) is correctly set to reflect
the source of the code to be parsed and the lexical context for the
expression.

The op tree representing the expression is returned.  If an optional
expression is absent, a null pointer is returned, otherwise the pointer
will be non-null.

If an error occurs in parsing or compilation, in most cases a valid op
tree is returned anyway.  The error is reflected in the parser state,
normally resulting in a single exception at the top level of parsing
which covers all the compilation errors that occurred.  Some compilation
errors, however, will throw an exception immediately.

=cut
*/

OP *
Perl_parse_termexpr(pTHX_ U32 flags)
{
    return parse_expr(LEX_FAKEEOF_COMMA, flags);
}

/*
=for apidoc Amx|OP *|parse_listexpr|U32 flags

Parse a Perl list expression.  This may contain operators of precedence
down to the comma operator.  The expression must be followed (and thus
terminated) either by a low-precedence logic operator such as C<or> or by
something that would normally terminate an expression such as semicolon.
If I<flags> includes C<PARSE_OPTIONAL> then the expression is optional,
otherwise it is mandatory.  It is up to the caller to ensure that the
dynamic parser state (L</PL_parser> et al) is correctly set to reflect
the source of the code to be parsed and the lexical context for the
expression.

The op tree representing the expression is returned.  If an optional
expression is absent, a null pointer is returned, otherwise the pointer
will be non-null.

If an error occurs in parsing or compilation, in most cases a valid op
tree is returned anyway.  The error is reflected in the parser state,
normally resulting in a single exception at the top level of parsing
which covers all the compilation errors that occurred.  Some compilation
errors, however, will throw an exception immediately.

=cut
*/

OP *
Perl_parse_listexpr(pTHX_ U32 flags)
{
    return parse_expr(LEX_FAKEEOF_LOWLOGIC, flags);
}

/*
=for apidoc Amx|OP *|parse_fullexpr|U32 flags

Parse a single complete Perl expression.  This allows the full
expression grammar, including the lowest-precedence operators such
as C<or>.  The expression must be followed (and thus terminated) by a
token that an expression would normally be terminated by: end-of-file,
closing bracketing punctuation, semicolon, or one of the keywords that
signals a postfix expression-statement modifier.  If I<flags> includes
C<PARSE_OPTIONAL> then the expression is optional, otherwise it is
mandatory.  It is up to the caller to ensure that the dynamic parser
state (L</PL_parser> et al) is correctly set to reflect the source of
the code to be parsed and the lexical context for the expression.

The op tree representing the expression is returned.  If an optional
expression is absent, a null pointer is returned, otherwise the pointer
will be non-null.

If an error occurs in parsing or compilation, in most cases a valid op
tree is returned anyway.  The error is reflected in the parser state,
normally resulting in a single exception at the top level of parsing
which covers all the compilation errors that occurred.  Some compilation
errors, however, will throw an exception immediately.

=cut
*/

OP *
Perl_parse_fullexpr(pTHX_ U32 flags)
{
    return parse_expr(LEX_FAKEEOF_NONEXPR, flags);
}

/*
=for apidoc Amx|OP *|parse_block|U32 flags

Parse a single complete Perl code block.  This consists of an opening
brace, a sequence of statements, and a closing brace.  The block
constitutes a lexical scope, so C<my> variables and various compile-time
effects can be contained within it.  It is up to the caller to ensure
that the dynamic parser state (L</PL_parser> et al) is correctly set to
reflect the source of the code to be parsed and the lexical context for
the statement.

The op tree representing the code block is returned.  This is always a
real op, never a null pointer.  It will normally be a C<lineseq> list,
including C<nextstate> or equivalent ops.  No ops to construct any kind
of runtime scope are included by virtue of it being a block.

If an error occurs in parsing or compilation, in most cases a valid op
tree (most likely null) is returned anyway.  The error is reflected in
the parser state, normally resulting in a single exception at the top
level of parsing which covers all the compilation errors that occurred.
Some compilation errors, however, will throw an exception immediately.

The I<flags> parameter is reserved for future use, and must always
be zero.

=cut
*/

OP *
Perl_parse_block(pTHX_ U32 flags)
{
    if (flags)
	Perl_croak(aTHX_ "Parsing code internal error (%s)", "parse_block");
    return parse_recdescent_for_op(GRAMBLOCK, LEX_FAKEEOF_NEVER);
}

/*
=for apidoc Amx|OP *|parse_barestmt|U32 flags

Parse a single unadorned Perl statement.  This may be a normal imperative
statement or a declaration that has compile-time effect.  It does not
include any label or other affixture.  It is up to the caller to ensure
that the dynamic parser state (L</PL_parser> et al) is correctly set to
reflect the source of the code to be parsed and the lexical context for
the statement.

The op tree representing the statement is returned.  This may be a
null pointer if the statement is null, for example if it was actually
a subroutine definition (which has compile-time side effects).  If not
null, it will be ops directly implementing the statement, suitable to
pass to L</newSTATEOP>.  It will not normally include a C<nextstate> or
equivalent op (except for those embedded in a scope contained entirely
within the statement).

If an error occurs in parsing or compilation, in most cases a valid op
tree (most likely null) is returned anyway.  The error is reflected in
the parser state, normally resulting in a single exception at the top
level of parsing which covers all the compilation errors that occurred.
Some compilation errors, however, will throw an exception immediately.

The I<flags> parameter is reserved for future use, and must always
be zero.

=cut
*/

OP *
Perl_parse_barestmt(pTHX_ U32 flags)
{
    if (flags)
	Perl_croak(aTHX_ "Parsing code internal error (%s)", "parse_barestmt");
    return parse_recdescent_for_op(GRAMBARESTMT, LEX_FAKEEOF_NEVER);
}

/*
=for apidoc Amx|SV *|parse_label|U32 flags

Parse a single label, possibly optional, of the type that may prefix a
Perl statement.  It is up to the caller to ensure that the dynamic parser
state (L</PL_parser> et al) is correctly set to reflect the source of
the code to be parsed.  If I<flags> includes C<PARSE_OPTIONAL> then the
label is optional, otherwise it is mandatory.

The name of the label is returned in the form of a fresh scalar.  If an
optional label is absent, a null pointer is returned.

If an error occurs in parsing, which can only occur if the label is
mandatory, a valid label is returned anyway.  The error is reflected in
the parser state, normally resulting in a single exception at the top
level of parsing which covers all the compilation errors that occurred.

=cut
*/

SV *
Perl_parse_label(pTHX_ U32 flags)
{
    if (flags & ~PARSE_OPTIONAL)
	Perl_croak(aTHX_ "Parsing code internal error (%s)", "parse_label");
    if (PL_lex_state == LEX_KNOWNEXT) {
	PL_parser->yychar = yylex();
	if (PL_parser->yychar == LABEL) {
	    SV *lsv;
	    PL_parser->yychar = YYEMPTY;
	    lsv = newSV_type(SVt_PV);
	    sv_copypv(lsv, cSVOPx(pl_yylval.opval)->op_sv);
	    return lsv;
	} else {
	    yyunlex();
	    goto no_label;
	}
    } else {
	char *s, *t;
	STRLEN wlen, bufptr_pos;
	lex_read_space(0);
	t = s = PL_bufptr;
        if (!isIDFIRST_lazy_if(s, UTF))
	    goto no_label;
	t = scan_word(s, PL_tokenbuf, sizeof PL_tokenbuf, FALSE, &wlen);
	if (word_takes_any_delimeter(s, wlen))
	    goto no_label;
	bufptr_pos = s - SvPVX(PL_linestr);
	PL_bufptr = t;
	lex_read_space(LEX_KEEP_PREVIOUS);
	t = PL_bufptr;
	s = SvPVX(PL_linestr) + bufptr_pos;
	if (t[0] == ':' && t[1] != ':') {
	    PL_oldoldbufptr = PL_oldbufptr;
	    PL_oldbufptr = s;
	    PL_bufptr = t+1;
	    return newSVpvn_flags(s, wlen, UTF ? SVf_UTF8 : 0);
	} else {
	    PL_bufptr = s;
	    no_label:
	    if (flags & PARSE_OPTIONAL) {
		return NULL;
	    } else {
		qerror(Perl_mess(aTHX_ "Parse error"));
		return newSVpvs("x");
	    }
	}
    }
}

/*
=for apidoc Amx|OP *|parse_fullstmt|U32 flags

Parse a single complete Perl statement.  This may be a normal imperative
statement or a declaration that has compile-time effect, and may include
optional labels.  It is up to the caller to ensure that the dynamic
parser state (L</PL_parser> et al) is correctly set to reflect the source
of the code to be parsed and the lexical context for the statement.

The op tree representing the statement is returned.  This may be a
null pointer if the statement is null, for example if it was actually
a subroutine definition (which has compile-time side effects).  If not
null, it will be the result of a L</newSTATEOP> call, normally including
a C<nextstate> or equivalent op.

If an error occurs in parsing or compilation, in most cases a valid op
tree (most likely null) is returned anyway.  The error is reflected in
the parser state, normally resulting in a single exception at the top
level of parsing which covers all the compilation errors that occurred.
Some compilation errors, however, will throw an exception immediately.

The I<flags> parameter is reserved for future use, and must always
be zero.

=cut
*/

OP *
Perl_parse_fullstmt(pTHX_ U32 flags)
{
    if (flags)
	Perl_croak(aTHX_ "Parsing code internal error (%s)", "parse_fullstmt");
    return parse_recdescent_for_op(GRAMFULLSTMT, LEX_FAKEEOF_NEVER);
}

/*
=for apidoc Amx|OP *|parse_stmtseq|U32 flags

Parse a sequence of zero or more Perl statements.  These may be normal
imperative statements, including optional labels, or declarations
that have compile-time effect, or any mixture thereof.  The statement
sequence ends when a closing brace or end-of-file is encountered in a
place where a new statement could have validly started.  It is up to
the caller to ensure that the dynamic parser state (L</PL_parser> et al)
is correctly set to reflect the source of the code to be parsed and the
lexical context for the statements.

The op tree representing the statement sequence is returned.  This may
be a null pointer if the statements were all null, for example if there
were no statements or if there were only subroutine definitions (which
have compile-time side effects).  If not null, it will be a C<lineseq>
list, normally including C<nextstate> or equivalent ops.

If an error occurs in parsing or compilation, in most cases a valid op
tree is returned anyway.  The error is reflected in the parser state,
normally resulting in a single exception at the top level of parsing
which covers all the compilation errors that occurred.  Some compilation
errors, however, will throw an exception immediately.

The I<flags> parameter is reserved for future use, and must always
be zero.

=cut
*/

OP *
Perl_parse_stmtseq(pTHX_ U32 flags)
{
    OP *stmtseqop;
    I32 c;
    if (flags)
	Perl_croak(aTHX_ "Parsing code internal error (%s)", "parse_stmtseq");
    stmtseqop = parse_recdescent_for_op(GRAMSTMTSEQ, LEX_FAKEEOF_CLOSING);
    c = lex_peek_unichar(0);
    if (c != -1 && c != /*{*/'}')
	qerror(Perl_mess(aTHX_ "Parse error"));
    return stmtseqop;
}

void
Perl_munge_qwlist_to_paren_list(pTHX_ OP *qwlist)
{
    PERL_ARGS_ASSERT_MUNGE_QWLIST_TO_PAREN_LIST;
    deprecate("qw(...) as parentheses");
    force_next((4<<24)|')');
    if (qwlist->op_type == OP_STUB) {
	op_free(qwlist);
    }
    else {
	start_force(PL_curforce);
	NEXTVAL_NEXTTOKE.opval = qwlist;
	force_next(THING);
    }
    force_next((2<<24)|'(');
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
