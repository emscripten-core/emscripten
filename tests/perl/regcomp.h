/*    regcomp.h
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
 *    2000, 2001, 2002, 2003, 2005, 2006, 2007, by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */
#include "regcharclass.h"

typedef OP OP_4tree;			/* Will be redefined later. */


/* Convert branch sequences to more efficient trie ops? */
#define PERL_ENABLE_TRIE_OPTIMISATION 1

/* Be really aggressive about optimising patterns with trie sequences? */
#define PERL_ENABLE_EXTENDED_TRIE_OPTIMISATION 1

/* Should the optimiser take positive assertions into account? */
#define PERL_ENABLE_POSITIVE_ASSERTION_STUDY 0

/* Not for production use: */
#define PERL_ENABLE_EXPERIMENTAL_REGEX_OPTIMISATIONS 0

/* Activate offsets code - set to if 1 to enable */
#ifdef DEBUGGING
#define RE_TRACK_PATTERN_OFFSETS
#endif

/* Unless the next line is uncommented it is illegal to combine lazy 
   matching with possessive matching. Frankly it doesn't make much sense 
   to allow it as X*?+ matches nothing, X+?+ matches a single char only, 
   and X{min,max}?+ matches min times only.
 */
/* #define REG_ALLOW_MINMOD_SUSPEND */

/*
 * The "internal use only" fields in regexp.h are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are:
 *
 * regstart	sv that must begin a match; NULL if none obvious
 * reganch	is the match anchored (at beginning-of-line only)?
 * regmust	string (pointer into program) that match must include, or NULL
 *  [regmust changed to SV* for bminstr()--law]
 * regmlen	length of regmust string
 *  [regmlen not used currently]
 *
 * Regstart and reganch permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  Regmust permits fast rejection
 * of lines that cannot possibly match.  The regmust tests are costly enough
 * that pregcomp() supplies a regmust only if the r.e. contains something
 * potentially expensive (at present, the only such thing detected is * or +
 * at the start of the r.e., which can involve a lot of backup).  Regmlen is
 * supplied because the test in pregexec() needs it and pregcomp() is computing
 * it anyway.
 * [regmust is now supplied always.  The tests that use regmust have a
 * heuristic that disables the test if it usually matches.]
 *
 * [In fact, we now use regmust in many cases to locate where the search
 * starts in the string, so if regback is >= 0, the regmust search is never
 * wasted effort.  The regback variable says how many characters back from
 * where regmust matched is the earliest possible start of the match.
 * For instance, /[a-z].foo/ has a regmust of 'foo' and a regback of 2.]
 */

/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
 * all nodes except BRANCH implement concatenation; a "next" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
 * have one of the subtle syntax dependencies:  an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is
 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:  the tail of the branch connects
 * to the thing following the set of BRANCHes.)  The opcodes are defined
 * in regnodes.h which is generated from regcomp.sym by regcomp.pl.
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 *
 * [The "next" pointer is always aligned on an even
 * boundary, and reads the offset directly as a short.  Also, there is no
 * special test to reverse the sign of BACK pointers since the offset is
 * stored negative.]
 */

/* This is the stuff that used to live in regexp.h that was truly
   private to the engine itself. It now lives here. */



 typedef struct regexp_internal {
        int name_list_idx;	/* Optional data index of an array of paren names */
        union {
	    U32 *offsets;           /* offset annotations 20001228 MJD
                                       data about mapping the program to the
                                       string -
                                       offsets[0] is proglen when this is used
                                       */
            U32 proglen;
        } u;

        regnode *regstclass;    /* Optional startclass as identified or constructed
                                   by the optimiser */
        struct reg_data *data;	/* Additional miscellaneous data used by the program.
                                   Used to make it easier to clone and free arbitrary
                                   data that the regops need. Often the ARG field of
                                   a regop is an index into this structure */
	regnode program[1];	/* Unwarranted chumminess with compiler. */
} regexp_internal;

#define RXi_SET(x,y) (x)->pprivate = (void*)(y)   
#define RXi_GET(x)   ((regexp_internal *)((x)->pprivate))
#define RXi_GET_DECL(r,ri) regexp_internal *ri = RXi_GET(r)
/*
 * Flags stored in regexp->intflags
 * These are used only internally to the regexp engine
 *
 * See regexp.h for flags used externally to the regexp engine
 */
#define PREGf_SKIP		0x00000001
#define PREGf_IMPLICIT		0x00000002 /* Converted .* to ^.* */
#define PREGf_NAUGHTY		0x00000004 /* how exponential is this pattern? */
#define PREGf_VERBARG_SEEN	0x00000008
#define PREGf_CUTGROUP_SEEN	0x00000010


/* this is where the old regcomp.h started */

struct regnode_string {
    U8	str_len;
    U8  type;
    U16 next_off;
    char string[1];
};

/* Argument bearing node - workhorse, 
   arg1 is often for the data field */
struct regnode_1 {
    U8	flags;
    U8  type;
    U16 next_off;
    U32 arg1;
};

/* Similar to a regnode_1 but with an extra signed argument */
struct regnode_2L {
    U8	flags;
    U8  type;
    U16 next_off;
    U32 arg1;
    I32 arg2;
};

/* 'Two field' -- Two 16 bit unsigned args */
struct regnode_2 {
    U8	flags;
    U8  type;
    U16 next_off;
    U16 arg1;
    U16 arg2;
};


#define ANYOF_BITMAP_SIZE	32	/* 256 b/(8 b/B) */
#define ANYOF_CLASSBITMAP_SIZE	 4	/* up to 32 (8*4) named classes */

/* also used by trie */
struct regnode_charclass {
    U8	flags;
    U8  type;
    U16 next_off;
    U32 arg1;				/* used as ptr in S_regclass */
    char bitmap[ANYOF_BITMAP_SIZE];	/* only compile-time */
};

/* has runtime (locale) \d, \w, ..., [:posix:] classes */
struct regnode_charclass_class {
    U8	flags;				/* ANYOF_CLASS bit must go here */
    U8  type;
    U16 next_off;
    U32 arg1;					/* used as ptr in S_regclass */
    char bitmap[ANYOF_BITMAP_SIZE];		/* both compile-time */
    char classflags[ANYOF_CLASSBITMAP_SIZE];	/* and run-time */
};

/* XXX fix this description.
   Impose a limit of REG_INFTY on various pattern matching operations
   to limit stack growth and to avoid "infinite" recursions.
*/
/* The default size for REG_INFTY is I16_MAX, which is the same as
   SHORT_MAX (see perl.h).  Unfortunately I16 isn't necessarily 16 bits
   (see handy.h).  On the Cray C90, sizeof(short)==4 and hence I16_MAX is
   ((1<<31)-1), while on the Cray T90, sizeof(short)==8 and I16_MAX is
   ((1<<63)-1).  To limit stack growth to reasonable sizes, supply a
   smaller default.
	--Andy Dougherty  11 June 1998
*/
#if SHORTSIZE > 2
#  ifndef REG_INFTY
#    define REG_INFTY ((1<<15)-1)
#  endif
#endif

#ifndef REG_INFTY
#  define REG_INFTY I16_MAX
#endif

#define ARG_VALUE(arg) (arg)
#define ARG__SET(arg,val) ((arg) = (val))

#undef ARG
#undef ARG1
#undef ARG2

#define ARG(p) ARG_VALUE(ARG_LOC(p))
#define ARG1(p) ARG_VALUE(ARG1_LOC(p))
#define ARG2(p) ARG_VALUE(ARG2_LOC(p))
#define ARG2L(p) ARG_VALUE(ARG2L_LOC(p))

#define ARG_SET(p, val) ARG__SET(ARG_LOC(p), (val))
#define ARG1_SET(p, val) ARG__SET(ARG1_LOC(p), (val))
#define ARG2_SET(p, val) ARG__SET(ARG2_LOC(p), (val))
#define ARG2L_SET(p, val) ARG__SET(ARG2L_LOC(p), (val))

#undef NEXT_OFF
#undef NODE_ALIGN

#define NEXT_OFF(p) ((p)->next_off)
#define NODE_ALIGN(node)
#define NODE_ALIGN_FILL(node) ((node)->flags = 0xde) /* deadbeef */

#define SIZE_ALIGN NODE_ALIGN

#undef OP
#undef OPERAND
#undef MASK
#undef STRING

#define	OP(p)		((p)->type)
#define FLAGS(p)	((p)->flags)	/* Caution: Doesn't apply to all      \
					   regnode types.  For some, it's the \
					   character set of the regnode */
#define	OPERAND(p)	(((struct regnode_string *)p)->string)
#define MASK(p)		((char*)OPERAND(p))
#define	STR_LEN(p)	(((struct regnode_string *)p)->str_len)
#define	STRING(p)	(((struct regnode_string *)p)->string)
#define STR_SZ(l)	((l + sizeof(regnode) - 1) / sizeof(regnode))
#define NODE_SZ_STR(p)	(STR_SZ(STR_LEN(p))+1)

#undef NODE_ALIGN
#undef ARG_LOC
#undef NEXTOPER
#undef PREVOPER

#define	NODE_ALIGN(node)
#define	ARG_LOC(p)	(((struct regnode_1 *)p)->arg1)
#define	ARG1_LOC(p)	(((struct regnode_2 *)p)->arg1)
#define	ARG2_LOC(p)	(((struct regnode_2 *)p)->arg2)
#define ARG2L_LOC(p)	(((struct regnode_2L *)p)->arg2)

#define NODE_STEP_REGNODE	1	/* sizeof(regnode)/sizeof(regnode) */
#define EXTRA_STEP_2ARGS	EXTRA_SIZE(struct regnode_2)

#define NODE_STEP_B	4

#define	NEXTOPER(p)	((p) + NODE_STEP_REGNODE)
#define	PREVOPER(p)	((p) - NODE_STEP_REGNODE)

#define FILL_ADVANCE_NODE(ptr, op) STMT_START { \
    (ptr)->type = op;    (ptr)->next_off = 0;   (ptr)++; } STMT_END
#define FILL_ADVANCE_NODE_ARG(ptr, op, arg) STMT_START { \
    ARG_SET(ptr, arg);  FILL_ADVANCE_NODE(ptr, op); (ptr) += 1; } STMT_END

#define REG_MAGIC 0234

#define SIZE_ONLY (RExC_emit == &PL_regdummy)

/* If the bitmap doesn't fully represent what this ANYOF node can match, the
 * ARG is set to this special value (since 0, 1, ... are legal, but will never
 * reach this high). */
#define ANYOF_NONBITMAP_EMPTY	((U32) -1)

/* The information used to be stored as as combination of the ANYOF_UTF8 and
 * ANYOF_NONBITMAP_NON_UTF8 bits in the flags field, but was moved out of there
 * to free up a bit for other uses.  This tries to hide the change from
 * existing code as much as possible.  Now, the data structure that goes in ARG
 * is not allocated unless it is needed, and that is what is used to determine
 * if there is something outside the bitmap.  The code now assumes that if
 * that structure exists, that any UTF-8 encoded string should be tried against
 * it, but a non-UTF8-encoded string will be tried only if the
 * ANYOF_NONBITMAP_NON_UTF8 bit is also set. */
#define ANYOF_NONBITMAP(node)	(ARG(node) != ANYOF_NONBITMAP_EMPTY)

/* Flags for node->flags of ANYOF.  These are in short supply, so some games
 * are done to share them, as described below.  If necessary, the ANYOF_LOCALE
 * and ANYOF_CLASS bits could be shared with a space penalty for locale nodes,
 * but this isn't quite so easy, as the optimizer also uses ANYOF_CLASS.
 * Another option would be to push them into new nodes.  E.g. there could be an
 * ANYOF_LOCALE node that would be in place of the flag of the same name.
 * Once the planned change to compile all the above-latin1 code points is done,
 * then the UNICODE_ALL bit can be freed up, with a small performance penalty.
 * If flags need to be added that are applicable to the synthetic start class
 * only, with some work, they could be put in the next-node field, or in an
 * unused bit of the classflags field. */

#define ANYOF_LOCALE		 0x01	    /* /l modifier */

/* The fold is calculated and stored in the bitmap where possible at compile
 * time.  However there are two cases where it isn't possible.  These share
 * this bit:  1) under locale, where the actual folding varies depending on
 * what the locale is at the time of execution; and 2) where the folding is
 * specified in a swash, not the bitmap, such as characters which aren't
 * specified in the bitmap, or properties that aren't looked at at compile time
 */
#define ANYOF_LOC_NONBITMAP_FOLD 0x02

#define ANYOF_INVERT		 0x04

/* Set if this is a struct regnode_charclass_class vs a regnode_charclass.  This
 * is used for runtime \d, \w, [:posix:], ..., which are used only in locale
 * and the optimizer's synthetic start class.  Non-locale \d, etc are resolved
 * at compile-time */
#define ANYOF_CLASS	 0x08
#define ANYOF_LARGE      ANYOF_CLASS    /* Same; name retained for back compat */

/* EOS, meaning that it can match an empty string too, is used for the
 * synthetic start class only. */
#define ANYOF_EOS		0x10

/* ? Is this node the synthetic start class (ssc).  This bit is shared with
 * ANYOF_EOS, as the latter is used only for the ssc, and then not used by
 * regexec.c.  And, the code is structured so that if it is set, the ssc is
 * not used, so it is guaranteed to be 0 for the ssc by the time regexec.c
 * gets executed, and 0 for a non-ssc ANYOF node, as it only ever gets set for
 * a potential ssc candidate.  Thus setting it to 1 after it has been
 * determined that the ssc will be used is not ambiguous */
#define ANYOF_IS_SYNTHETIC	ANYOF_EOS

/* Can match something outside the bitmap that isn't in utf8 */
#define ANYOF_NONBITMAP_NON_UTF8 0x20

/* Matches every code point 0x100 and above*/
#define ANYOF_UNICODE_ALL	0x40

/* Match all Latin1 characters that aren't ASCII when the target string is not
 * in utf8. */
#define ANYOF_NON_UTF8_LATIN1_ALL 0x80

#define ANYOF_FLAGS_ALL		0xff

/* These are the flags that ANYOF_INVERT being set or not doesn't affect
 * whether they are operative or not.  e.g., the node still has LOCALE
 * regardless of being inverted; whereas ANYOF_UNICODE_ALL means something
 * different if inverted */
#define INVERSION_UNAFFECTED_FLAGS (ANYOF_LOCALE                        \
	                           |ANYOF_LOC_NONBITMAP_FOLD            \
	                           |ANYOF_CLASS                         \
	                           |ANYOF_EOS                           \
	                           |ANYOF_NONBITMAP_NON_UTF8)

/* Character classes for node->classflags of ANYOF */
/* Should be synchronized with a table in regprop() */
/* 2n should pair with 2n+1 */

#define ANYOF_ALNUM	 0	/* \w, PL_utf8_alnum, utf8::IsWord, ALNUM */
#define ANYOF_NALNUM	 1
#define ANYOF_SPACE	 2	/* \s */
#define ANYOF_NSPACE	 3
#define ANYOF_DIGIT	 4	/* \d */
#define ANYOF_NDIGIT	 5
#define ANYOF_ALNUMC	 6	/* [[:alnum:]] isalnum(3), utf8::IsAlnum, ALNUMC */
#define ANYOF_NALNUMC	 7
#define ANYOF_ALPHA	 8
#define ANYOF_NALPHA	 9
#define ANYOF_ASCII	10
#define ANYOF_NASCII	11
#define ANYOF_CNTRL	12
#define ANYOF_NCNTRL	13
#define ANYOF_GRAPH	14
#define ANYOF_NGRAPH	15
#define ANYOF_LOWER	16
#define ANYOF_NLOWER	17
#define ANYOF_PRINT	18
#define ANYOF_NPRINT	19
#define ANYOF_PUNCT	20
#define ANYOF_NPUNCT	21
#define ANYOF_UPPER	22
#define ANYOF_NUPPER	23
#define ANYOF_XDIGIT	24
#define ANYOF_NXDIGIT	25
#define ANYOF_PSXSPC	26	/* POSIX space: \s plus the vertical tab */
#define ANYOF_NPSXSPC	27
#define ANYOF_BLANK	28	/* GNU extension: space and tab: non-vertical space */
#define ANYOF_NBLANK	29

#define ANYOF_MAX	32

/* pseudo classes, not stored in the class bitmap, but used as flags
   during compilation of char classes */

#define ANYOF_VERTWS	(ANYOF_MAX+1)
#define ANYOF_NVERTWS	(ANYOF_MAX+2)
#define ANYOF_HORIZWS	(ANYOF_MAX+3)
#define ANYOF_NHORIZWS	(ANYOF_MAX+4)

/* Backward source code compatibility. */

#define ANYOF_ALNUML	 ANYOF_ALNUM
#define ANYOF_NALNUML	 ANYOF_NALNUM
#define ANYOF_SPACEL	 ANYOF_SPACE
#define ANYOF_NSPACEL	 ANYOF_NSPACE

/* Utility macros for the bitmap and classes of ANYOF */

#define ANYOF_SIZE		(sizeof(struct regnode_charclass))
#define ANYOF_CLASS_SIZE	(sizeof(struct regnode_charclass_class))

#define ANYOF_FLAGS(p)		((p)->flags)

#define ANYOF_BIT(c)		(1 << ((c) & 7))

#define ANYOF_CLASS_BYTE(p, c)	(((struct regnode_charclass_class*)(p))->classflags[((c) >> 3) & 3])
#define ANYOF_CLASS_SET(p, c)	(ANYOF_CLASS_BYTE(p, c) |=  ANYOF_BIT(c))
#define ANYOF_CLASS_CLEAR(p, c)	(ANYOF_CLASS_BYTE(p, c) &= ~ANYOF_BIT(c))
#define ANYOF_CLASS_TEST(p, c)	(ANYOF_CLASS_BYTE(p, c) &   ANYOF_BIT(c))

#define ANYOF_CLASS_ZERO(ret)	Zero(((struct regnode_charclass_class*)(ret))->classflags, ANYOF_CLASSBITMAP_SIZE, char)
#define ANYOF_CLASS_SETALL(ret)		\
	memset (((struct regnode_charclass_class*)(ret))->classflags, 255, ANYOF_CLASSBITMAP_SIZE)
#define ANYOF_BITMAP_ZERO(ret)	Zero(((struct regnode_charclass*)(ret))->bitmap, ANYOF_BITMAP_SIZE, char)

#define ANYOF_BITMAP(p)		(((struct regnode_charclass*)(p))->bitmap)
#define ANYOF_BITMAP_BYTE(p, c)	(ANYOF_BITMAP(p)[(((U8)(c)) >> 3) & 31])
#define ANYOF_BITMAP_SET(p, c)	(ANYOF_BITMAP_BYTE(p, c) |=  ANYOF_BIT(c))
#define ANYOF_BITMAP_CLEAR(p,c)	(ANYOF_BITMAP_BYTE(p, c) &= ~ANYOF_BIT(c))
#define ANYOF_BITMAP_TEST(p, c)	(ANYOF_BITMAP_BYTE(p, c) &   ANYOF_BIT(c))

#define ANYOF_BITMAP_SETALL(p)		\
	memset (ANYOF_BITMAP(p), 255, ANYOF_BITMAP_SIZE)
#define ANYOF_BITMAP_CLEARALL(p)	\
	Zero (ANYOF_BITMAP(p), ANYOF_BITMAP_SIZE)
/* Check that all 256 bits are all set.  Used in S_cl_is_anything()  */
#define ANYOF_BITMAP_TESTALLSET(p)	/* Assumes sizeof(p) == 32 */     \
	memEQ (ANYOF_BITMAP(p), "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377", ANYOF_BITMAP_SIZE)

#define ANYOF_SKIP		((ANYOF_SIZE - 1)/sizeof(regnode))
#define ANYOF_CLASS_SKIP	((ANYOF_CLASS_SIZE - 1)/sizeof(regnode))

#if ANYOF_CLASSBITMAP_SIZE != 4
#   error ANYOF_CLASSBITMAP_SIZE is expected to be 4
#endif
#define ANYOF_CLASS_TEST_ANY_SET(p) ((ANYOF_FLAGS(p) & ANYOF_CLASS)         \
	&& memNE (((struct regnode_charclass_class*)(p))->classflags,	    \
		    "\0\0\0\0", ANYOF_CLASSBITMAP_SIZE))
/*#define ANYOF_CLASS_ADD_SKIP	(ANYOF_CLASS_SKIP - ANYOF_SKIP)
 * */


/*
 * Utility definitions.
 */
#ifndef CHARMASK
#  define UCHARAT(p)	((int)*(const U8*)(p))
#else
#  define UCHARAT(p)	((int)*(p)&CHARMASK)
#endif

#define EXTRA_SIZE(guy) ((sizeof(guy)-1)/sizeof(struct regnode))

#define REG_SEEN_ZERO_LEN	0x00000001
#define REG_SEEN_LOOKBEHIND	0x00000002
#define REG_SEEN_GPOS		0x00000004
#define REG_SEEN_EVAL		0x00000008
#define REG_SEEN_CANY		0x00000010
#define REG_SEEN_SANY		REG_SEEN_CANY /* src bckwrd cmpt */
#define REG_SEEN_RECURSE        0x00000020
#define REG_TOP_LEVEL_BRANCHES  0x00000040
#define REG_SEEN_VERBARG        0x00000080
#define REG_SEEN_CUTGROUP       0x00000100
#define REG_SEEN_RUN_ON_COMMENT 0x00000200
#define REG_SEEN_EXACTF_SHARP_S 0x00000400

START_EXTERN_C

#ifdef PLUGGABLE_RE_EXTENSION
#include "re_nodes.h"
#else
#include "regnodes.h"
#endif

#ifndef PLUGGABLE_RE_EXTENSION
#ifndef DOINIT
EXTCONST regexp_engine PL_core_reg_engine;
#else /* DOINIT */
EXTCONST regexp_engine PL_core_reg_engine = { 
        Perl_re_compile,
        Perl_regexec_flags,
        Perl_re_intuit_start,
        Perl_re_intuit_string, 
        Perl_regfree_internal,
        Perl_reg_numbered_buff_fetch,
        Perl_reg_numbered_buff_store,
        Perl_reg_numbered_buff_length,
        Perl_reg_named_buff,
        Perl_reg_named_buff_iter,
        Perl_reg_qr_package,
#if defined(USE_ITHREADS)        
        Perl_regdupe_internal
#endif        
};
#endif /* DOINIT */
#endif /* PLUGGABLE_RE_EXTENSION */


END_EXTERN_C


/* .what is a character array with one character for each member of .data
 * The character describes the function of the corresponding .data item:
 *   a - AV for paren_name_list under DEBUGGING
 *   f - start-class data for regstclass optimization
 *   n - Root of op tree for (?{EVAL}) item
 *   o - Start op for (?{EVAL}) item
 *   p - Pad for (?{EVAL}) item
 *   s - swash for Unicode-style character class, and the multicharacter
 *       strings resulting from casefolding the single-character entries
 *       in the character class
 *   t - trie struct
 *   u - trie struct's widecharmap (a HV, so can't share, must dup)
 *       also used for revcharmap and words under DEBUGGING
 *   T - aho-trie struct
 *   S - sv for named capture lookup
 * 20010712 mjd@plover.com
 * (Remember to update re_dup() and pregfree() if you add any items.)
 */
struct reg_data {
    U32 count;
    U8 *what;
    void* data[1];
};

/* Code in S_to_utf8_substr() and S_to_byte_substr() in regexec.c accesses
   anchored* and float* via array indexes 0 and 1.  */
#define anchored_substr substrs->data[0].substr
#define anchored_utf8 substrs->data[0].utf8_substr
#define anchored_offset substrs->data[0].min_offset
#define anchored_end_shift substrs->data[0].end_shift

#define float_substr substrs->data[1].substr
#define float_utf8 substrs->data[1].utf8_substr
#define float_min_offset substrs->data[1].min_offset
#define float_max_offset substrs->data[1].max_offset
#define float_end_shift substrs->data[1].end_shift

#define check_substr substrs->data[2].substr
#define check_utf8 substrs->data[2].utf8_substr
#define check_offset_min substrs->data[2].min_offset
#define check_offset_max substrs->data[2].max_offset
#define check_end_shift substrs->data[2].end_shift

#define RX_ANCHORED_SUBSTR(rx)	(((struct regexp *)SvANY(rx))->anchored_substr)
#define RX_ANCHORED_UTF8(rx)	(((struct regexp *)SvANY(rx))->anchored_utf8)
#define RX_FLOAT_SUBSTR(rx)	(((struct regexp *)SvANY(rx))->float_substr)
#define RX_FLOAT_UTF8(rx)	(((struct regexp *)SvANY(rx))->float_utf8)

/* trie related stuff */

/* a transition record for the state machine. the
   check field determines which state "owns" the
   transition. the char the transition is for is
   determined by offset from the owning states base
   field.  the next field determines which state
   is to be transitioned to if any.
*/
struct _reg_trie_trans {
  U32 next;
  U32 check;
};

/* a transition list element for the list based representation */
struct _reg_trie_trans_list_elem {
    U16 forid;
    U32 newstate;
};
typedef struct _reg_trie_trans_list_elem reg_trie_trans_le;

/* a state for compressed nodes. base is an offset
  into an array of reg_trie_trans array. If wordnum is
  nonzero the state is accepting. if base is zero then
  the state has no children (and will be accepting)
*/
struct _reg_trie_state {
  U16 wordnum;
  union {
    U32                base;
    reg_trie_trans_le* list;
  } trans;
};

/* info per word; indexed by wordnum */
typedef struct {
    U16  prev;	/* previous word in acceptance chain; eg in
		 * zzz|abc|ab/ after matching the chars abc, the
		 * accepted word is #2, and the previous accepted
		 * word is #3 */
    U32 len;	/* how many chars long is this word? */
    U32 accept;	/* accept state for this word */
} reg_trie_wordinfo;


typedef struct _reg_trie_state    reg_trie_state;
typedef struct _reg_trie_trans    reg_trie_trans;


/* anything in here that needs to be freed later
   should be dealt with in pregfree.
   refcount is first in both this and _reg_ac_data to allow a space
   optimisation in Perl_regdupe.  */
struct _reg_trie_data {
    U32             refcount;        /* number of times this trie is referenced */
    U32             lasttrans;       /* last valid transition element */
    U16             *charmap;        /* byte to charid lookup array */
    reg_trie_state  *states;         /* state data */
    reg_trie_trans  *trans;          /* array of transition elements */
    char            *bitmap;         /* stclass bitmap */
    U16 	    *jump;           /* optional 1 indexed array of offsets before tail 
                                        for the node following a given word. */
    reg_trie_wordinfo *wordinfo;     /* array of info per word */
    U16             uniquecharcount; /* unique chars in trie (width of trans table) */
    U32             startstate;      /* initial state - used for common prefix optimisation */
    STRLEN          minlen;          /* minimum length of words in trie - build/opt only? */
    STRLEN          maxlen;          /* maximum length of words in trie - build/opt only? */
    U32             prefixlen;       /* #chars in common prefix */
    U32             statecount;      /* Build only - number of states in the states array 
                                        (including the unused zero state) */
    U32             wordcount;       /* Build only */
#ifdef DEBUGGING
    STRLEN          charcount;       /* Build only */
#endif
};
/* There is one (3 under DEBUGGING) pointers that logically belong in this
   structure, but are held outside as they need duplication on thread cloning,
   whereas the rest of the structure can be read only:
    HV              *widecharmap;    code points > 255 to charid
#ifdef DEBUGGING
    AV              *words;          Array of words contained in trie, for dumping
    AV              *revcharmap;     Map of each charid back to its character representation
#endif
*/

#define TRIE_WORDS_OFFSET 2

typedef struct _reg_trie_data reg_trie_data;

/* refcount is first in both this and _reg_trie_data to allow a space
   optimisation in Perl_regdupe.  */
struct _reg_ac_data {
    U32              refcount;
    U32              trie;
    U32              *fail;
    reg_trie_state   *states;
};
typedef struct _reg_ac_data reg_ac_data;

/* ANY_BIT doesn't use the structure, so we can borrow it here.
   This is simpler than refactoring all of it as wed end up with
   three different sets... */

#define TRIE_BITMAP(p)		(((reg_trie_data *)(p))->bitmap)
#define TRIE_BITMAP_BYTE(p, c)	(TRIE_BITMAP(p)[(((U8)(c)) >> 3) & 31])
#define TRIE_BITMAP_SET(p, c)	(TRIE_BITMAP_BYTE(p, c) |=  ANYOF_BIT((U8)c))
#define TRIE_BITMAP_CLEAR(p,c)	(TRIE_BITMAP_BYTE(p, c) &= ~ANYOF_BIT((U8)c))
#define TRIE_BITMAP_TEST(p, c)	(TRIE_BITMAP_BYTE(p, c) &   ANYOF_BIT((U8)c))

#define IS_ANYOF_TRIE(op) ((op)==TRIEC || (op)==AHOCORASICKC)
#define IS_TRIE_AC(op) ((op)>=AHOCORASICK)


#define BITMAP_BYTE(p, c)	(((U8*)p)[(((U8)(c)) >> 3) & 31])
#define BITMAP_TEST(p, c)	(BITMAP_BYTE(p, c) &   ANYOF_BIT((U8)c))

/* these defines assume uniquecharcount is the correct variable, and state may be evaluated twice */
#define TRIE_NODENUM(state) (((state)-1)/(trie->uniquecharcount)+1)
#define SAFE_TRIE_NODENUM(state) ((state) ? (((state)-1)/(trie->uniquecharcount)+1) : (state))
#define TRIE_NODEIDX(state) ((state) ? (((state)-1)*(trie->uniquecharcount)+1) : (state))

#ifdef DEBUGGING
#define TRIE_CHARCOUNT(trie) ((trie)->charcount)
#else
#define TRIE_CHARCOUNT(trie) (trie_charcount)
#endif

#define RE_TRIE_MAXBUF_INIT 65536
#define RE_TRIE_MAXBUF_NAME "\022E_TRIE_MAXBUF"
#define RE_DEBUG_FLAGS "\022E_DEBUG_FLAGS"

/*

RE_DEBUG_FLAGS is used to control what debug output is emitted
its divided into three groups of options, some of which interact.
The three groups are: Compile, Execute, Extra. There is room for a
further group, as currently only the low three bytes are used.

    Compile Options:
    
    PARSE
    PEEP
    TRIE
    PROGRAM
    OFFSETS

    Execute Options:

    INTUIT
    MATCH
    TRIE

    Extra Options

    TRIE
    OFFSETS

If you modify any of these make sure you make corresponding changes to
re.pm, especially to the documentation.

*/


/* Compile */
#define RE_DEBUG_COMPILE_MASK      0x0000FF
#define RE_DEBUG_COMPILE_PARSE     0x000001
#define RE_DEBUG_COMPILE_OPTIMISE  0x000002
#define RE_DEBUG_COMPILE_TRIE      0x000004
#define RE_DEBUG_COMPILE_DUMP      0x000008
#define RE_DEBUG_COMPILE_FLAGS     0x000010

/* Execute */
#define RE_DEBUG_EXECUTE_MASK      0x00FF00
#define RE_DEBUG_EXECUTE_INTUIT    0x000100
#define RE_DEBUG_EXECUTE_MATCH     0x000200
#define RE_DEBUG_EXECUTE_TRIE      0x000400

/* Extra */
#define RE_DEBUG_EXTRA_MASK        0xFF0000
#define RE_DEBUG_EXTRA_TRIE        0x010000
#define RE_DEBUG_EXTRA_OFFSETS     0x020000
#define RE_DEBUG_EXTRA_OFFDEBUG    0x040000
#define RE_DEBUG_EXTRA_STATE       0x080000
#define RE_DEBUG_EXTRA_OPTIMISE    0x100000
#define RE_DEBUG_EXTRA_BUFFERS     0x400000
#define RE_DEBUG_EXTRA_GPOS        0x800000
/* combined */
#define RE_DEBUG_EXTRA_STACK       0x280000

#define RE_DEBUG_FLAG(x) (re_debug_flags & x)
/* Compile */
#define DEBUG_COMPILE_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_COMPILE_MASK) x  )
#define DEBUG_PARSE_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_COMPILE_PARSE) x  )
#define DEBUG_OPTIMISE_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_COMPILE_OPTIMISE) x  )
#define DEBUG_PARSE_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_COMPILE_PARSE) x  )
#define DEBUG_DUMP_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_COMPILE_DUMP) x  )
#define DEBUG_TRIE_COMPILE_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_COMPILE_TRIE) x )
#define DEBUG_FLAGS_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_COMPILE_FLAGS) x )
/* Execute */
#define DEBUG_EXECUTE_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_EXECUTE_MASK) x  )
#define DEBUG_INTUIT_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_EXECUTE_INTUIT) x  )
#define DEBUG_MATCH_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_EXECUTE_MATCH) x  )
#define DEBUG_TRIE_EXECUTE_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_EXECUTE_TRIE) x )

/* Extra */
#define DEBUG_EXTRA_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_EXTRA_MASK) x  )
#define DEBUG_OFFSETS_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_EXTRA_OFFSETS) x  )
#define DEBUG_STATE_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_EXTRA_STATE) x )
#define DEBUG_STACK_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_EXTRA_STACK) x )
#define DEBUG_BUFFERS_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_EXTRA_BUFFERS) x )

#define DEBUG_OPTIMISE_MORE_r(x) DEBUG_r( \
    if ((RE_DEBUG_EXTRA_OPTIMISE|RE_DEBUG_COMPILE_OPTIMISE) == \
         (re_debug_flags & (RE_DEBUG_EXTRA_OPTIMISE|RE_DEBUG_COMPILE_OPTIMISE)) ) x )
#define MJD_OFFSET_DEBUG(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_EXTRA_OFFDEBUG) \
        Perl_warn_nocontext x )
#define DEBUG_TRIE_COMPILE_MORE_r(x) DEBUG_TRIE_COMPILE_r( \
    if (re_debug_flags & RE_DEBUG_EXTRA_TRIE) x )
#define DEBUG_TRIE_EXECUTE_MORE_r(x) DEBUG_TRIE_EXECUTE_r( \
    if (re_debug_flags & RE_DEBUG_EXTRA_TRIE) x )

#define DEBUG_TRIE_r(x) DEBUG_r( \
    if (re_debug_flags & (RE_DEBUG_COMPILE_TRIE \
        | RE_DEBUG_EXECUTE_TRIE )) x )
#define DEBUG_GPOS_r(x) DEBUG_r( \
    if (re_debug_flags & RE_DEBUG_EXTRA_GPOS) x )

/* initialization */
/* get_sv() can return NULL during global destruction.  re_debug_flags can get
 * clobbered by a longjmp, so must be initialized */
#define GET_RE_DEBUG_FLAGS DEBUG_r({ \
        SV * re_debug_flags_sv = NULL; \
        re_debug_flags = 0;            \
        re_debug_flags_sv = get_sv(RE_DEBUG_FLAGS, 1); \
        if (re_debug_flags_sv) { \
            if (!SvIOK(re_debug_flags_sv)) \
                sv_setuv(re_debug_flags_sv, RE_DEBUG_COMPILE_DUMP | RE_DEBUG_EXECUTE_MASK ); \
            re_debug_flags=SvIV(re_debug_flags_sv); \
        }\
})

#ifdef DEBUGGING

#define GET_RE_DEBUG_FLAGS_DECL VOL IV re_debug_flags \
	PERL_UNUSED_DECL = 0; GET_RE_DEBUG_FLAGS;

#define RE_PV_COLOR_DECL(rpv,rlen,isuni,dsv,pv,l,m,c1,c2) \
    const char * const rpv =                          \
        pv_pretty((dsv), (pv), (l), (m), \
            PL_colors[(c1)],PL_colors[(c2)], \
            PERL_PV_ESCAPE_RE|PERL_PV_ESCAPE_NONASCII |((isuni) ? PERL_PV_ESCAPE_UNI : 0) );         \
    const int rlen = SvCUR(dsv)

#define RE_SV_ESCAPE(rpv,isuni,dsv,sv,m) \
    const char * const rpv =                          \
        pv_pretty((dsv), (SvPV_nolen_const(sv)), (SvCUR(sv)), (m), \
            PL_colors[(c1)],PL_colors[(c2)], \
            PERL_PV_ESCAPE_RE|PERL_PV_ESCAPE_NONASCII |((isuni) ? PERL_PV_ESCAPE_UNI : 0) )

#define RE_PV_QUOTED_DECL(rpv,isuni,dsv,pv,l,m)                    \
    const char * const rpv =                                       \
        pv_pretty((dsv), (pv), (l), (m), \
            PL_colors[0], PL_colors[1], \
            ( PERL_PV_PRETTY_QUOTE | PERL_PV_ESCAPE_RE | PERL_PV_ESCAPE_NONASCII | PERL_PV_PRETTY_ELLIPSES | \
              ((isuni) ? PERL_PV_ESCAPE_UNI : 0))                  \
        )

#define RE_SV_DUMPLEN(ItEm) (SvCUR(ItEm) - (SvTAIL(ItEm)!=0))
#define RE_SV_TAIL(ItEm) (SvTAIL(ItEm) ? "$" : "")
    
#else /* if not DEBUGGING */

#define GET_RE_DEBUG_FLAGS_DECL
#define RE_PV_COLOR_DECL(rpv,rlen,isuni,dsv,pv,l,m,c1,c2)
#define RE_SV_ESCAPE(rpv,isuni,dsv,sv,m)
#define RE_PV_QUOTED_DECL(rpv,isuni,dsv,pv,l,m)
#define RE_SV_DUMPLEN(ItEm)
#define RE_SV_TAIL(ItEm)

#endif /* DEBUG RELATED DEFINES */

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
