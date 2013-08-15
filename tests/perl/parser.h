/*    parser.h
 *
 *    Copyright (c) 2006, 2007, 2009, 2010, 2011 Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 * 
 * This file defines the layout of the parser object used by the parser
 * and lexer (perly.c, toke.c).
 */

#define YYEMPTY		(-2)

typedef struct {
    YYSTYPE val;    /* semantic value */
    short   state;
    I32     savestack_ix;	/* size of savestack at this state */
    CV	    *compcv; /* value of PL_compcv when this value was created */
#ifdef DEBUGGING
    const char  *name; /* token/rule name for -Dpv */
#endif
} yy_stack_frame;

typedef struct yy_parser {

    /* parser state */

    struct yy_parser *old_parser; /* previous value of PL_parser */
    YYSTYPE	    yylval;	/* value of lookahead symbol, set by yylex() */
    int		    yychar;	/* The lookahead symbol.  */

    /* Number of tokens to shift before error messages enabled.  */
    int		    yyerrstatus;

    int		    stack_size;
    int		    yylen;	/* length of active reduction */
    yy_stack_frame  *stack;	/* base of stack */
    yy_stack_frame  *ps;	/* current stack frame */

    /* lexer state */

    I32		lex_brackets;	/* square and curly bracket count */
    I32		lex_casemods;	/* casemod count */
    char	*lex_brackstack;/* what kind of brackets to pop */
    char	*lex_casestack;	/* what kind of case mods in effect */
    U8		lex_defer;	/* state after determined token */
    bool	lex_dojoin;	/* doing an array interpolation */
    U8		lex_expect;	/* expect after determined token */
    U8		expect;		/* how to interpret ambiguous tokens */
    I32		lex_formbrack;	/* bracket count at outer format level */
    OP		*lex_inpat;	/* in pattern $) and $| are special */
    OP		*lex_op;	/* extra info to pass back on op */
    SV		*lex_repl;	/* runtime replacement from s/// */
    U16		lex_inwhat;	/* what kind of quoting are we in */
    OPCODE	last_lop_op;	/* last list operator */
    I32		lex_starts;	/* how many interps done on level */
    SV		*lex_stuff;	/* runtime pattern from m// or s/// */
    I32		multi_start;	/* 1st line of multi-line string */
    I32		multi_end;	/* last line of multi-line string */
    char	multi_open;	/* delimiter of said string */
    char	multi_close;	/* delimiter of said string */
    char	pending_ident;	/* pending identifier lookup */
    bool	preambled;
    I32		lex_allbrackets;/* (), [], {}, ?: bracket count */
    SUBLEXINFO	sublex_info;
    SV		*linestr;	/* current chunk of src text */
    char	*bufptr;	
    char	*oldbufptr;	
    char	*oldoldbufptr;	
    char	*bufend;	
    char	*linestart;	/* beginning of most recently read line */
    char	*last_uni;	/* position of last named-unary op */
    char	*last_lop;	/* position of last list operator */
    line_t	copline;	/* current line number */
    U16		in_my;		/* we're compiling a "my"/"our" declaration */
    U8		lex_state;	/* next token is determined */
    U8		error_count;	/* how many compile errors so far, max 10 */
    HV		*in_my_stash;	/* declared class of this "my" declaration */
    PerlIO	*rsfp;		/* current source file pointer */
    AV		*rsfp_filters;	/* holds chain of active source filters */

#ifdef PERL_MAD
    SV		*endwhite;
    I32		faketokens;
    I32		lasttoke;
    SV		*nextwhite;
    I32		realtokenstart;
    SV		*skipwhite;
    SV		*thisclose;
    MADPROP *	thismad;
    SV		*thisopen;
    SV		*thisstuff;
    SV		*thistoken;
    SV		*thiswhite;

/* What we know when we're in LEX_KNOWNEXT state. */
    NEXTTOKE	nexttoke[5];	/* value of next token, if any */
    I32		curforce;
#else
    YYSTYPE	nextval[5];	/* value of next token, if any */
    I32		nexttype[5];	/* type of next token */
    I32		nexttoke;
#endif

    COP		*saved_curcop;	/* the previous PL_curcop */
    char	tokenbuf[256];

    U8		lex_fakeeof;	/* precedence at which to fake EOF */
    U8		lex_flags;
    PERL_BITFIELD16	in_pod:1;      /* lexer is within a =pod section */
    PERL_BITFIELD16	filtered:1;    /* source filters in evalbytes */
} yy_parser;

/* flags for lexer API */
#define LEX_STUFF_UTF8		0x00000001
#define LEX_KEEP_PREVIOUS	0x00000002

#ifdef PERL_CORE
# define LEX_START_SAME_FILTER	0x00000001
# define LEX_IGNORE_UTF8_HINTS	0x00000002
# define LEX_EVALBYTES		0x00000004
# define LEX_START_COPIED	0x00000008
# define LEX_DONT_CLOSE_RSFP	0x00000010
# define LEX_START_FLAGS \
	(LEX_START_SAME_FILTER|LEX_START_COPIED \
	|LEX_IGNORE_UTF8_HINTS|LEX_EVALBYTES|LEX_DONT_CLOSE_RSFP)
#endif

/* flags for parser API */
#define PARSE_OPTIONAL          0x00000001

/* values for lex_fakeeof */
enum {
    LEX_FAKEEOF_NEVER,      /* don't fake EOF */
    LEX_FAKEEOF_CLOSING,    /* fake EOF at unmatched closing punctuation */
    LEX_FAKEEOF_NONEXPR,    /* ... and at token that can't be in expression */
    LEX_FAKEEOF_LOWLOGIC,   /* ... and at low-precedence logic operator */
    LEX_FAKEEOF_COMMA,      /* ... and at comma */
    LEX_FAKEEOF_ASSIGN,     /* ... and at assignment operator */
    LEX_FAKEEOF_IFELSE,     /* ... and at ?: operator */
    LEX_FAKEEOF_RANGE,      /* ... and at range operator */
    LEX_FAKEEOF_LOGIC,      /* ... and at logic operator */
    LEX_FAKEEOF_BITWISE,    /* ... and at bitwise operator */
    LEX_FAKEEOF_COMPARE,    /* ... and at comparison operator */
    LEX_FAKEEOF_MAX
};

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
