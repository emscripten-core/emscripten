/*    perly.y
 *
 *    Copyright (c) 1991-2002, 2003, 2004, 2005, 2006 Larry Wall
 *    Copyright (c) 2007, 2008, 2009, 2010, 2011 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'I see,' laughed Strider.  'I look foul and feel fair.  Is that it?
 *  All that is gold does not glitter, not all those who wander are lost.'
 *
 *     [p.171 of _The Lord of the Rings_, I/x: "Strider"]
 */

/*
 * This file holds the grammar for the Perl language. If edited, you need
 * to run regen_perly.pl, which re-creates the files perly.h, perly.tab
 * and perly.act which are derived from this.
 *
 * Note that these derived files are included and compiled twice; once
 * from perly.c, and once from madly.c. The second time, a number of MAD
 * macros are defined, which compile in extra code that allows the parse
 * tree to be accurately dumped. In particular:
 *
 * MAD            defined if compiling madly.c
 * DO_MAD(A)      expands to A  under madly.c, to null otherwise
 * IF_MAD(a,b)    expands to A under madly.c, to B otherwise
 * TOKEN_GETMAD() expands to token_getmad() under madly.c, to null otherwise
 * TOKEN_FREE()   similarly
 * OP_GETMAD()    similarly
 * IVAL(i)        expands to (i)->tk_lval.ival or (i)
 * PVAL(p)        expands to (p)->tk_lval.pval or (p)
 *
 * The main job of of this grammar is to call the various newFOO()
 * functions in op.c to build a syntax tree of OP structs.
 * It relies on the lexer in toke.c to do the tokenizing.
 *
 * Note: due to the way that the cleanup code works WRT to freeing ops on
 * the parse stack, it is dangerous to assign to the $n variables within
 * an action.
 */

/*  Make the parser re-entrant. */

%pure_parser

/* FIXME for MAD - is the new mintro on while and until important?  */

%start grammar

%union {
    I32	ival; /* __DEFAULT__ (marker for regen_perly.pl;
				must always be 1st union member) */
    char *pval;
    OP *opval;
    GV *gvval;
#ifdef PERL_IN_MADLY_C
    TOKEN* p_tkval;
    TOKEN* i_tkval;
#else
    char *p_tkval;
    I32	i_tkval;
#endif
#ifdef PERL_MAD
    TOKEN* tkval;
#endif
}

%token <ival> GRAMPROG GRAMEXPR GRAMBLOCK GRAMBARESTMT GRAMFULLSTMT GRAMSTMTSEQ

%token <i_tkval> '{' '}' '[' ']' '-' '+' '$' '@' '%' '*' '&' ';'

%token <opval> WORD METHOD FUNCMETH THING PMFUNC PRIVATEREF QWLIST
%token <opval> FUNC0OP FUNC0SUB UNIOPSUB LSTOPSUB
%token <opval> PLUGEXPR PLUGSTMT
%token <p_tkval> LABEL
%token <i_tkval> FORMAT SUB ANONSUB PACKAGE USE
%token <i_tkval> WHILE UNTIL IF UNLESS ELSE ELSIF CONTINUE FOR
%token <i_tkval> GIVEN WHEN DEFAULT
%token <i_tkval> LOOPEX DOTDOT YADAYADA
%token <i_tkval> FUNC0 FUNC1 FUNC UNIOP LSTOP
%token <i_tkval> RELOP EQOP MULOP ADDOP
%token <i_tkval> DOLSHARP DO HASHBRACK NOAMP
%token <i_tkval> LOCAL MY MYSUB REQUIRE
%token <i_tkval> COLONATTR

%type <i_tkval> lpar_or_qw

%type <ival> grammar remember mremember
%type <ival>  startsub startanonsub startformsub
/* FIXME for MAD - are these two ival? */
%type <ival> mydefsv mintro

%type <opval> stmtseq fullstmt labfullstmt barestmt block mblock else
%type <opval> expr term subscripted scalar ary hsh arylen star amper sideff
%type <opval> listexpr nexpr texpr iexpr mexpr mnexpr miexpr
%type <opval> optlistexpr optexpr indirob listop method
%type <opval> formname subname proto subbody cont my_scalar
%type <opval> subattrlist myattrlist myattrterm myterm
%type <opval> termbinop termunop anonymous termdo

%nonassoc <i_tkval> PREC_LOW
%nonassoc LOOPEX

%left <i_tkval> OROP DOROP
%left <i_tkval> ANDOP
%right <i_tkval> NOTOP
%nonassoc LSTOP LSTOPSUB
%left <i_tkval> ','
%right <i_tkval> ASSIGNOP
%right <i_tkval> '?' ':'
%nonassoc DOTDOT YADAYADA
%left <i_tkval> OROR DORDOR
%left <i_tkval> ANDAND
%left <i_tkval> BITOROP
%left <i_tkval> BITANDOP
%nonassoc EQOP
%nonassoc RELOP
%nonassoc UNIOP UNIOPSUB
%nonassoc REQUIRE
%left <i_tkval> SHIFTOP
%left ADDOP
%left MULOP
%left <i_tkval> MATCHOP
%right <i_tkval> '!' '~' UMINUS REFGEN
%right <i_tkval> POWOP
%nonassoc <i_tkval> PREINC PREDEC POSTINC POSTDEC
%left <i_tkval> ARROW
%nonassoc <i_tkval> ')'
%left <i_tkval> '('
%left '[' '{'

%token <i_tkval> PEG

%% /* RULES */

/* Top-level choice of what kind of thing yyparse was called to parse */
grammar	:	GRAMPROG
			{
			  PL_parser->expect = XSTATE;
			}
		remember stmtseq
			{
			  newPROG(block_end($3,$4));
			  $$ = 0;
			}
	|	GRAMEXPR
			{
			  parser->expect = XTERM;
			}
		optexpr
			{
			  PL_eval_root = $3;
			  $$ = 0;
			}
	|	GRAMBLOCK
			{
			  parser->expect = XBLOCK;
			}
		block
			{
			  PL_pad_reset_pending = TRUE;
			  PL_eval_root = $3;
			  $$ = 0;
			  yyunlex();
			  parser->yychar = YYEOF;
			}
	|	GRAMBARESTMT
			{
			  parser->expect = XSTATE;
			}
		barestmt
			{
			  PL_pad_reset_pending = TRUE;
			  PL_eval_root = $3;
			  $$ = 0;
			  yyunlex();
			  parser->yychar = YYEOF;
			}
	|	GRAMFULLSTMT
			{
			  parser->expect = XSTATE;
			}
		fullstmt
			{
			  PL_pad_reset_pending = TRUE;
			  PL_eval_root = $3;
			  $$ = 0;
			  yyunlex();
			  parser->yychar = YYEOF;
			}
	|	GRAMSTMTSEQ
			{
			  parser->expect = XSTATE;
			}
		stmtseq
			{
			  PL_eval_root = $3;
			  $$ = 0;
			}
	;

/* An ordinary block */
block	:	'{' remember stmtseq '}'
			{ if (PL_parser->copline > (line_t)IVAL($1))
			      PL_parser->copline = (line_t)IVAL($1);
			  $$ = block_end($2, $3);
			  TOKEN_GETMAD($1,$$,'{');
			  TOKEN_GETMAD($4,$$,'}');
			}
	;

remember:	/* NULL */	/* start a full lexical scope */
			{ $$ = block_start(TRUE); }
	;

mydefsv:	/* NULL */	/* lexicalize $_ */
			{ $$ = (I32) Perl_allocmy(aTHX_ STR_WITH_LEN("$_"), 0); }
	;

mblock	:	'{' mremember stmtseq '}'
			{ if (PL_parser->copline > (line_t)IVAL($1))
			      PL_parser->copline = (line_t)IVAL($1);
			  $$ = block_end($2, $3);
			  TOKEN_GETMAD($1,$$,'{');
			  TOKEN_GETMAD($4,$$,'}');
			}
	;

mremember:	/* NULL */	/* start a partial lexical scope */
			{ $$ = block_start(FALSE); }
	;

/* A sequence of statements in the program */
stmtseq	:	/* NULL */
			{ $$ = (OP*)NULL; }
	|	stmtseq fullstmt
			{   $$ = op_append_list(OP_LINESEQ, $1, $2);
			    PL_pad_reset_pending = TRUE;
			    if ($1 && $2)
				PL_hints |= HINT_BLOCK_SCOPE;
			}
	;

/* A statement in the program, including optional labels */
fullstmt:	barestmt
			{
			  if($1) {
			      $$ = newSTATEOP(0, NULL, $1);
			  } else {
			      $$ = IF_MAD(newOP(OP_NULL, 0), NULL);
			  }
			}
	|	labfullstmt
			{ $$ = $1; }
	;

labfullstmt:	LABEL barestmt
			{
			  $$ = newSTATEOP(SvUTF8(((SVOP*)$1)->op_sv),
                                        savepv(SvPVX(((SVOP*)$1)->op_sv)), $2);
			  TOKEN_GETMAD($1,
			      $2 ? cLISTOPx($$)->op_first : $$, 'L');
			}
	|	LABEL labfullstmt
			{
			  $$ = newSTATEOP(SvUTF8(((SVOP*)$1)->op_sv),
                                        savepv(SvPVX(((SVOP*)$1)->op_sv)), $2);
			  TOKEN_GETMAD($1, cLISTOPx($$)->op_first, 'L');
			}
	;

/* A bare statement, lacking label and other aspects of state op */
barestmt:	PLUGSTMT
			{ $$ = $1; }
	|	PEG
			{
			  $$ = newOP(OP_NULL,0);
			  TOKEN_GETMAD($1,$$,'p');
			}
	|	FORMAT startformsub formname block
			{
			  CV *fmtcv = PL_compcv;
			  SvREFCNT_inc_simple_void(PL_compcv);
#ifdef MAD
			  $$ = newFORM($2, $3, $4);
			  prepend_madprops($1->tk_mad, $$, 'F');
			  $1->tk_mad = 0;
			  token_free($1);
#else
			  newFORM($2, $3, $4);
			  $$ = (OP*)NULL;
#endif
			  if (CvOUTSIDE(fmtcv) && !CvUNIQUE(CvOUTSIDE(fmtcv))) {
			      SvREFCNT_inc_simple_void(fmtcv);
			      pad_add_anon(fmtcv, OP_NULL);
			  }
			}
	|	SUB startsub subname proto subattrlist subbody
			{
			  SvREFCNT_inc_simple_void(PL_compcv);
#ifdef MAD
			  {
			      OP* o = newSVOP(OP_ANONCODE, 0,
				(SV*)newATTRSUB($2, $3, $4, $5, $6));
			      $$ = newOP(OP_NULL,0);
			      op_getmad(o,$$,'&');
			      op_getmad($3,$$,'n');
			      op_getmad($4,$$,'s');
			      op_getmad($5,$$,'a');
			      token_getmad($1,$$,'d');
			      append_madprops($6->op_madprop, $$, 0);
			      $6->op_madprop = 0;
			  }
#else
			  newATTRSUB($2, $3, $4, $5, $6);
			  $$ = (OP*)NULL;
#endif
			}
	|	MYSUB startsub subname proto subattrlist subbody
			{
			  /* Unimplemented "my sub foo { }" */
			  SvREFCNT_inc_simple_void(PL_compcv);
#ifdef MAD
			  $$ = newMYSUB($2, $3, $4, $5, $6);
			  token_getmad($1,$$,'d');
#else
			  newMYSUB($2, $3, $4, $5, $6);
			  $$ = (OP*)NULL;
#endif
			}
	|	PACKAGE WORD WORD ';'
			{
#ifdef MAD
			  $$ = package($3);
			  token_getmad($1,$$,'o');
			  if ($2)
			      package_version($2);
			  token_getmad($4,$$,';');
#else
			  package($3);
			  if ($2)
			      package_version($2);
			  $$ = (OP*)NULL;
#endif
			}
	|	USE startsub
			{ CvSPECIAL_on(PL_compcv); /* It's a BEGIN {} */ }
		WORD WORD optlistexpr ';'
			{
			  SvREFCNT_inc_simple_void(PL_compcv);
#ifdef MAD
			  $$ = utilize(IVAL($1), $2, $4, $5, $6);
			  token_getmad($1,$$,'o');
			  token_getmad($7,$$,';');
			  if (PL_parser->rsfp_filters &&
				      AvFILLp(PL_parser->rsfp_filters) >= 0)
			      append_madprops(newMADPROP('!', MAD_NULL, NULL, 0), $$, 0);
#else
			  utilize(IVAL($1), $2, $4, $5, $6);
			  $$ = (OP*)NULL;
#endif
			}
	|	IF lpar_or_qw remember mexpr ')' mblock else
			{
			  $$ = block_end($3,
			      newCONDOP(0, $4, op_scope($6), $7));
			  TOKEN_GETMAD($1,$$,'I');
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($5,$$,')');
			  PL_parser->copline = (line_t)IVAL($1);
			}
	|	UNLESS lpar_or_qw remember miexpr ')' mblock else
			{
			  $$ = block_end($3,
			      newCONDOP(0, $4, op_scope($6), $7));
			  TOKEN_GETMAD($1,$$,'I');
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($5,$$,')');
			  PL_parser->copline = (line_t)IVAL($1);
			}
	|	GIVEN lpar_or_qw remember mydefsv mexpr ')' mblock
			{
			  $$ = block_end($3,
				  newGIVENOP($5, op_scope($7), (PADOFFSET)$4));
			  PL_parser->copline = (line_t)IVAL($1);
			}
	|	WHEN lpar_or_qw remember mexpr ')' mblock
			{ $$ = block_end($3, newWHENOP($4, op_scope($6))); }
	|	DEFAULT block
			{ $$ = newWHENOP(0, op_scope($2)); }
	|	WHILE lpar_or_qw remember texpr ')' mintro mblock cont
			{
			  $$ = block_end($3,
				  newWHILEOP(0, 1, (LOOP*)(OP*)NULL,
				      $4, $7, $8, $6));
			  TOKEN_GETMAD($1,$$,'W');
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($5,$$,')');
			  PL_parser->copline = (line_t)IVAL($1);
			}
	|	UNTIL lpar_or_qw remember iexpr ')' mintro mblock cont
			{
			  $$ = block_end($3,
				  newWHILEOP(0, 1, (LOOP*)(OP*)NULL,
				      $4, $7, $8, $6));
			  TOKEN_GETMAD($1,$$,'W');
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($5,$$,')');
			  PL_parser->copline = (line_t)IVAL($1);
			}
	|	FOR lpar_or_qw remember mnexpr ';' texpr ';' mintro mnexpr ')'
		mblock
			{
			  OP *initop = IF_MAD($4 ? $4 : newOP(OP_NULL, 0), $4);
			  OP *forop = newWHILEOP(0, 1, (LOOP*)(OP*)NULL,
				      scalar($6), $11, $9, $8);
			  if (initop) {
			      forop = op_prepend_elem(OP_LINESEQ, initop,
				  op_append_elem(OP_LINESEQ,
				      newOP(OP_UNSTACK, OPf_SPECIAL),
				      forop));
			  }
			  DO_MAD({ forop = newUNOP(OP_NULL, 0, forop); })
			  $$ = block_end($3, forop);
			  TOKEN_GETMAD($1,$$,'3');
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($5,$$,'1');
			  TOKEN_GETMAD($7,$$,'2');
			  TOKEN_GETMAD($10,$$,')');
			  PL_parser->copline = (line_t)IVAL($1);
			}
	|	FOR MY remember my_scalar lpar_or_qw mexpr ')' mblock cont
			{
			  $$ = block_end($3, newFOROP(0, $4, $6, $8, $9));
			  TOKEN_GETMAD($1,$$,'W');
			  TOKEN_GETMAD($2,$$,'d');
			  TOKEN_GETMAD($5,$$,'(');
			  TOKEN_GETMAD($7,$$,')');
			  PL_parser->copline = (line_t)IVAL($1);
			}
	|	FOR scalar lpar_or_qw remember mexpr ')' mblock cont
			{
			  $$ = block_end($4, newFOROP(0,
				      op_lvalue($2, OP_ENTERLOOP), $5, $7, $8));
			  TOKEN_GETMAD($1,$$,'W');
			  TOKEN_GETMAD($3,$$,'(');
			  TOKEN_GETMAD($6,$$,')');
			  PL_parser->copline = (line_t)IVAL($1);
			}
	|	FOR lpar_or_qw remember mexpr ')' mblock cont
			{
			  $$ = block_end($3,
				  newFOROP(0, (OP*)NULL, $4, $6, $7));
			  TOKEN_GETMAD($1,$$,'W');
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($5,$$,')');
			  PL_parser->copline = (line_t)IVAL($1);
			}
	|	block cont
			{
			  /* a block is a loop that happens once */
			  $$ = newWHILEOP(0, 1, (LOOP*)(OP*)NULL,
				  (OP*)NULL, $1, $2, 0);
			}
	|	PACKAGE WORD WORD '{' remember
			{
			  int save_3_latefree = $3->op_latefree;
			  $3->op_latefree = 1;
			  package($3);
			  $3->op_latefree = save_3_latefree;
			  if ($2) {
			      int save_2_latefree = $2->op_latefree;
			      $2->op_latefree = 1;
			      package_version($2);
			      $2->op_latefree = save_2_latefree;
			  }
			}
		stmtseq '}'
			{
			  /* a block is a loop that happens once */
			  $$ = newWHILEOP(0, 1, (LOOP*)(OP*)NULL,
				  (OP*)NULL, block_end($5, $7), (OP*)NULL, 0);
			  op_free($3);
			  if ($2)
			      op_free($2);
			  TOKEN_GETMAD($4,$$,'{');
			  TOKEN_GETMAD($8,$$,'}');
			  if (PL_parser->copline > (line_t)IVAL($4))
			      PL_parser->copline = (line_t)IVAL($4);
			}
	|	sideff ';'
			{
			  PL_parser->expect = XSTATE;
			  $$ = $1;
			  TOKEN_GETMAD($2,$$,';');
			}
	|	';'
			{
			  PL_parser->expect = XSTATE;
			  $$ = IF_MAD(newOP(OP_NULL, 0), (OP*)NULL);
			  TOKEN_GETMAD($1,$$,';');
			  PL_parser->copline = NOLINE;
			}
	;

/* An expression which may have a side-effect */
sideff	:	error
			{ $$ = (OP*)NULL; }
	|	expr
			{ $$ = $1; }
	|	expr IF expr
			{ $$ = newLOGOP(OP_AND, 0, $3, $1);
			  TOKEN_GETMAD($2,$$,'i');
			}
	|	expr UNLESS expr
			{ $$ = newLOGOP(OP_OR, 0, $3, $1);
			  TOKEN_GETMAD($2,$$,'i');
			}
	|	expr WHILE expr
			{ $$ = newLOOPOP(OPf_PARENS, 1, scalar($3), $1);
			  TOKEN_GETMAD($2,$$,'w');
			}
	|	expr UNTIL iexpr
			{ $$ = newLOOPOP(OPf_PARENS, 1, $3, $1);
			  TOKEN_GETMAD($2,$$,'w');
			}
	|	expr FOR expr
			{ $$ = newFOROP(0, (OP*)NULL, $3, $1, (OP*)NULL);
			  TOKEN_GETMAD($2,$$,'w');
			  PL_parser->copline = (line_t)IVAL($2);
			}
	|	expr WHEN expr
			{ $$ = newWHENOP($3, op_scope($1)); }
	;

/* else and elsif blocks */
else	:	/* NULL */
			{ $$ = (OP*)NULL; }
	|	ELSE mblock
			{
			  ($2)->op_flags |= OPf_PARENS;
			  $$ = op_scope($2);
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	ELSIF lpar_or_qw mexpr ')' mblock else
			{ PL_parser->copline = (line_t)IVAL($1);
			    $$ = newCONDOP(0,
				newSTATEOP(OPf_SPECIAL,NULL,$3),
				op_scope($5), $6);
			  PL_hints |= HINT_BLOCK_SCOPE;
			  TOKEN_GETMAD($1,$$,'I');
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($4,$$,')');
			}
	;

/* Continue blocks */
cont	:	/* NULL */
			{ $$ = (OP*)NULL; }
	|	CONTINUE block
			{
			  $$ = op_scope($2);
			  TOKEN_GETMAD($1,$$,'o');
			}
	;

/* determine whether there are any new my declarations */
mintro	:	/* NULL */
			{ $$ = (PL_min_intro_pending &&
			    PL_max_intro_pending >=  PL_min_intro_pending);
			  intro_my(); }

/* Normal expression */
nexpr	:	/* NULL */
			{ $$ = (OP*)NULL; }
	|	sideff
	;

/* Boolean expression */
texpr	:	/* NULL means true */
			{ YYSTYPE tmplval;
			  (void)scan_num("1", &tmplval);
			  $$ = tmplval.opval; }
	|	expr
	;

/* Inverted boolean expression */
iexpr	:	expr
			{ $$ = invert(scalar($1)); }
	;

/* Expression with its own lexical scope */
mexpr	:	expr
			{ $$ = $1; intro_my(); }
	;

mnexpr	:	nexpr
			{ $$ = $1; intro_my(); }
	;

miexpr	:	iexpr
			{ $$ = $1; intro_my(); }
	;

formname:	WORD		{ $$ = $1; }
	|	/* NULL */	{ $$ = (OP*)NULL; }
	;

startsub:	/* NULL */	/* start a regular subroutine scope */
			{ $$ = start_subparse(FALSE, 0);
			    SAVEFREESV(PL_compcv); }

	;

startanonsub:	/* NULL */	/* start an anonymous subroutine scope */
			{ $$ = start_subparse(FALSE, CVf_ANON);
			    SAVEFREESV(PL_compcv); }
	;

startformsub:	/* NULL */	/* start a format subroutine scope */
			{ $$ = start_subparse(TRUE, 0);
			    SAVEFREESV(PL_compcv); }
	;

/* Name of a subroutine - must be a bareword, could be special */
subname	:	WORD	{ const char *const name = SvPV_nolen_const(((SVOP*)$1)->op_sv);
			  if (strEQ(name, "BEGIN") || strEQ(name, "END")
			      || strEQ(name, "INIT") || strEQ(name, "CHECK")
			      || strEQ(name, "UNITCHECK"))
			      CvSPECIAL_on(PL_compcv);
			  $$ = $1; }
	;

/* Subroutine prototype */
proto	:	/* NULL */
			{ $$ = (OP*)NULL; }
	|	THING
	;

/* Optional list of subroutine attributes */
subattrlist:	/* NULL */
			{ $$ = (OP*)NULL; }
	|	COLONATTR THING
			{ $$ = $2;
			  TOKEN_GETMAD($1,$$,':');
			}
	|	COLONATTR
			{ $$ = IF_MAD(
				    newOP(OP_NULL, 0),
				    (OP*)NULL
				);
			  TOKEN_GETMAD($1,$$,':');
			}
	;

/* List of attributes for a "my" variable declaration */
myattrlist:	COLONATTR THING
			{ $$ = $2;
			  TOKEN_GETMAD($1,$$,':');
			}
	|	COLONATTR
			{ $$ = IF_MAD(
				    newOP(OP_NULL, 0),
				    (OP*)NULL
				);
			  TOKEN_GETMAD($1,$$,':');
			}
	;

/* Subroutine body - either null or a block */
subbody	:	block	{ $$ = $1; }
	|	';'	{ $$ = IF_MAD(
				    newOP(OP_NULL,0),
				    (OP*)NULL
				);
			  PL_parser->expect = XSTATE;
			  TOKEN_GETMAD($1,$$,';');
			}
	;

/* Ordinary expressions; logical combinations */
expr	:	expr ANDOP expr
			{ $$ = newLOGOP(OP_AND, 0, $1, $3);
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	expr OROP expr
			{ $$ = newLOGOP(IVAL($2), 0, $1, $3);
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	expr DOROP expr
			{ $$ = newLOGOP(OP_DOR, 0, $1, $3);
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	listexpr %prec PREC_LOW
	;

/* Expressions are a list of terms joined by commas */
listexpr:	listexpr ','
			{
#ifdef MAD
			  OP* op = newNULLLIST();
			  token_getmad($2,op,',');
			  $$ = op_append_elem(OP_LIST, $1, op);
#else
			  $$ = $1;
#endif
			}
	|	listexpr ',' term
			{ 
			  OP* term = $3;
			  DO_MAD(
			      term = newUNOP(OP_NULL, 0, term);
			      token_getmad($2,term,',');
			  )
			  $$ = op_append_elem(OP_LIST, $1, term);
			}
	|	term %prec PREC_LOW
	;

/* List operators */
listop	:	LSTOP indirob listexpr /* map {...} @args or print $fh @args */
			{ $$ = convert(IVAL($1), OPf_STACKED,
				op_prepend_elem(OP_LIST, newGVREF(IVAL($1),$2), $3) );
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	FUNC '(' indirob expr ')'      /* print ($fh @args */
			{ $$ = convert(IVAL($1), OPf_STACKED,
				op_prepend_elem(OP_LIST, newGVREF(IVAL($1),$3), $4) );
			  TOKEN_GETMAD($1,$$,'o');
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($5,$$,')');
			}
	|	term ARROW method lpar_or_qw optexpr ')' /* $foo->bar(list) */
			{ $$ = convert(OP_ENTERSUB, OPf_STACKED,
				op_append_elem(OP_LIST,
				    op_prepend_elem(OP_LIST, scalar($1), $5),
				    newUNOP(OP_METHOD, 0, $3)));
			  TOKEN_GETMAD($2,$$,'A');
			  TOKEN_GETMAD($4,$$,'(');
			  TOKEN_GETMAD($6,$$,')');
			}
	|	term ARROW method                     /* $foo->bar */
			{ $$ = convert(OP_ENTERSUB, OPf_STACKED,
				op_append_elem(OP_LIST, scalar($1),
				    newUNOP(OP_METHOD, 0, $3)));
			  TOKEN_GETMAD($2,$$,'A');
			}
	|	METHOD indirob optlistexpr           /* new Class @args */
			{ $$ = convert(OP_ENTERSUB, OPf_STACKED,
				op_append_elem(OP_LIST,
				    op_prepend_elem(OP_LIST, $2, $3),
				    newUNOP(OP_METHOD, 0, $1)));
			}
	|	FUNCMETH indirob '(' optexpr ')'    /* method $object (@args) */
			{ $$ = convert(OP_ENTERSUB, OPf_STACKED,
				op_append_elem(OP_LIST,
				    op_prepend_elem(OP_LIST, $2, $4),
				    newUNOP(OP_METHOD, 0, $1)));
			  TOKEN_GETMAD($3,$$,'(');
			  TOKEN_GETMAD($5,$$,')');
			}
	|	LSTOP optlistexpr                    /* print @args */
			{ $$ = convert(IVAL($1), 0, $2);
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	FUNC '(' optexpr ')'                 /* print (@args) */
			{ $$ = convert(IVAL($1), 0, $3);
			  TOKEN_GETMAD($1,$$,'o');
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($4,$$,')');
			}
	|	LSTOPSUB startanonsub block /* sub f(&@);   f { foo } ... */
			{ SvREFCNT_inc_simple_void(PL_compcv);
			  $<opval>$ = newANONATTRSUB($2, 0, (OP*)NULL, $3); }
		    optlistexpr		%prec LSTOP  /* ... @bar */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				 op_append_elem(OP_LIST,
				   op_prepend_elem(OP_LIST, $<opval>4, $5), $1));
			}
	;

/* Names of methods. May use $object->$methodname */
method	:	METHOD
	|	scalar
	;

/* Some kind of subscripted expression */
subscripted:    star '{' expr ';' '}'        /* *main::{something} */
                        /* In this and all the hash accessors, ';' is
                         * provided by the tokeniser */
			{ $$ = newBINOP(OP_GELEM, 0, $1, scalar($3));
			    PL_parser->expect = XOPERATOR;
			  TOKEN_GETMAD($2,$$,'{');
			  TOKEN_GETMAD($4,$$,';');
			  TOKEN_GETMAD($5,$$,'}');
			}
	|	scalar '[' expr ']'          /* $array[$element] */
			{ $$ = newBINOP(OP_AELEM, 0, oopsAV($1), scalar($3));
			  TOKEN_GETMAD($2,$$,'[');
			  TOKEN_GETMAD($4,$$,']');
			}
	|	term ARROW '[' expr ']'      /* somearef->[$element] */
			{ $$ = newBINOP(OP_AELEM, 0,
					ref(newAVREF($1),OP_RV2AV),
					scalar($4));
			  TOKEN_GETMAD($2,$$,'a');
			  TOKEN_GETMAD($3,$$,'[');
			  TOKEN_GETMAD($5,$$,']');
			}
	|	subscripted '[' expr ']'    /* $foo->[$bar]->[$baz] */
			{ $$ = newBINOP(OP_AELEM, 0,
					ref(newAVREF($1),OP_RV2AV),
					scalar($3));
			  TOKEN_GETMAD($2,$$,'[');
			  TOKEN_GETMAD($4,$$,']');
			}
	|	scalar '{' expr ';' '}'    /* $foo{bar();} */
			{ $$ = newBINOP(OP_HELEM, 0, oopsHV($1), jmaybe($3));
			    PL_parser->expect = XOPERATOR;
			  TOKEN_GETMAD($2,$$,'{');
			  TOKEN_GETMAD($4,$$,';');
			  TOKEN_GETMAD($5,$$,'}');
			}
	|	term ARROW '{' expr ';' '}' /* somehref->{bar();} */
			{ $$ = newBINOP(OP_HELEM, 0,
					ref(newHVREF($1),OP_RV2HV),
					jmaybe($4));
			    PL_parser->expect = XOPERATOR;
			  TOKEN_GETMAD($2,$$,'a');
			  TOKEN_GETMAD($3,$$,'{');
			  TOKEN_GETMAD($5,$$,';');
			  TOKEN_GETMAD($6,$$,'}');
			}
	|	subscripted '{' expr ';' '}' /* $foo->[bar]->{baz;} */
			{ $$ = newBINOP(OP_HELEM, 0,
					ref(newHVREF($1),OP_RV2HV),
					jmaybe($3));
			    PL_parser->expect = XOPERATOR;
			  TOKEN_GETMAD($2,$$,'{');
			  TOKEN_GETMAD($4,$$,';');
			  TOKEN_GETMAD($5,$$,'}');
			}
	|	term ARROW '(' ')'          /* $subref->() */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				   newCVREF(0, scalar($1)));
			  TOKEN_GETMAD($2,$$,'a');
			  TOKEN_GETMAD($3,$$,'(');
			  TOKEN_GETMAD($4,$$,')');
			}
	|	term ARROW '(' expr ')'     /* $subref->(@args) */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				   op_append_elem(OP_LIST, $4,
				       newCVREF(0, scalar($1))));
			  TOKEN_GETMAD($2,$$,'a');
			  TOKEN_GETMAD($3,$$,'(');
			  TOKEN_GETMAD($5,$$,')');
			}

	|	subscripted lpar_or_qw expr ')'   /* $foo->{bar}->(@args) */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				   op_append_elem(OP_LIST, $3,
					       newCVREF(0, scalar($1))));
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($4,$$,')');
			}
	|	subscripted lpar_or_qw ')'        /* $foo->{bar}->() */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				   newCVREF(0, scalar($1)));
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($3,$$,')');
			}
	|	'(' expr ')' '[' expr ']'            /* list slice */
			{ $$ = newSLICEOP(0, $5, $2);
			  TOKEN_GETMAD($1,$$,'(');
			  TOKEN_GETMAD($3,$$,')');
			  TOKEN_GETMAD($4,$$,'[');
			  TOKEN_GETMAD($6,$$,']');
			}
	|	QWLIST '[' expr ']'            /* list literal slice */
			{ $$ = newSLICEOP(0, $3, $1);
			  TOKEN_GETMAD($2,$$,'[');
			  TOKEN_GETMAD($4,$$,']');
			}
	|	'(' ')' '[' expr ']'                 /* empty list slice! */
			{ $$ = newSLICEOP(0, $4, (OP*)NULL);
			  TOKEN_GETMAD($1,$$,'(');
			  TOKEN_GETMAD($2,$$,')');
			  TOKEN_GETMAD($3,$$,'[');
			  TOKEN_GETMAD($5,$$,']');
			}
    ;

/* Binary operators between terms */
termbinop:	term ASSIGNOP term                     /* $x = $y */
			{ $$ = newASSIGNOP(OPf_STACKED, $1, IVAL($2), $3);
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term POWOP term                        /* $x ** $y */
			{ $$ = newBINOP(IVAL($2), 0, scalar($1), scalar($3));
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term MULOP term                        /* $x * $y, $x x $y */
			{   if (IVAL($2) != OP_REPEAT)
				scalar($1);
			    $$ = newBINOP(IVAL($2), 0, $1, scalar($3));
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term ADDOP term                        /* $x + $y */
			{ $$ = newBINOP(IVAL($2), 0, scalar($1), scalar($3));
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term SHIFTOP term                      /* $x >> $y, $x << $y */
			{ $$ = newBINOP(IVAL($2), 0, scalar($1), scalar($3));
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term RELOP term                        /* $x > $y, etc. */
			{ $$ = newBINOP(IVAL($2), 0, scalar($1), scalar($3));
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term EQOP term                         /* $x == $y, $x eq $y */
			{ $$ = newBINOP(IVAL($2), 0, scalar($1), scalar($3));
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term BITANDOP term                     /* $x & $y */
			{ $$ = newBINOP(IVAL($2), 0, scalar($1), scalar($3));
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term BITOROP term                      /* $x | $y */
			{ $$ = newBINOP(IVAL($2), 0, scalar($1), scalar($3));
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term DOTDOT term                       /* $x..$y, $x...$y */
			{
			  $$ = newRANGE(IVAL($2), scalar($1), scalar($3));
			  DO_MAD({
			      UNOP *op;
			      op = (UNOP*)$$;
			      op = (UNOP*)op->op_first;	/* get to flop */
			      op = (UNOP*)op->op_first;	/* get to flip */
			      op = (UNOP*)op->op_first;	/* get to range */
			      token_getmad($2,(OP*)op,'o');
			    })
			}
	|	term ANDAND term                       /* $x && $y */
			{ $$ = newLOGOP(OP_AND, 0, $1, $3);
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term OROR term                         /* $x || $y */
			{ $$ = newLOGOP(OP_OR, 0, $1, $3);
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term DORDOR term                       /* $x // $y */
			{ $$ = newLOGOP(OP_DOR, 0, $1, $3);
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term MATCHOP term                      /* $x =~ /$y/ */
			{ $$ = bind_match(IVAL($2), $1, $3);
			  TOKEN_GETMAD($2,
				($$->op_type == OP_NOT
				    ? ((UNOP*)$$)->op_first : $$),
				'~');
			}
    ;

/* Unary operators and terms */
termunop : '-' term %prec UMINUS                       /* -$x */
			{ $$ = newUNOP(OP_NEGATE, 0, scalar($2));
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	'+' term %prec UMINUS                  /* +$x */
			{ $$ = IF_MAD(
				    newUNOP(OP_NULL, 0, $2),
				    $2
				);
			  TOKEN_GETMAD($1,$$,'+');
			}
	|	'!' term                               /* !$x */
			{ $$ = newUNOP(OP_NOT, 0, scalar($2));
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	'~' term                               /* ~$x */
			{ $$ = newUNOP(OP_COMPLEMENT, 0, scalar($2));
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	term POSTINC                           /* $x++ */
			{ $$ = newUNOP(OP_POSTINC, 0,
					op_lvalue(scalar($1), OP_POSTINC));
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	term POSTDEC                           /* $x-- */
			{ $$ = newUNOP(OP_POSTDEC, 0,
					op_lvalue(scalar($1), OP_POSTDEC));
			  TOKEN_GETMAD($2,$$,'o');
			}
	|	PREINC term                            /* ++$x */
			{ $$ = newUNOP(OP_PREINC, 0,
					op_lvalue(scalar($2), OP_PREINC));
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	PREDEC term                            /* --$x */
			{ $$ = newUNOP(OP_PREDEC, 0,
					op_lvalue(scalar($2), OP_PREDEC));
			  TOKEN_GETMAD($1,$$,'o');
			}

    ;

/* Constructors for anonymous data */
anonymous:	'[' expr ']'
			{ $$ = newANONLIST($2);
			  TOKEN_GETMAD($1,$$,'[');
			  TOKEN_GETMAD($3,$$,']');
			}
	|	'[' ']'
			{ $$ = newANONLIST((OP*)NULL);
			  TOKEN_GETMAD($1,$$,'[');
			  TOKEN_GETMAD($2,$$,']');
			}
	|	HASHBRACK expr ';' '}'	%prec '(' /* { foo => "Bar" } */
			{ $$ = newANONHASH($2);
			  TOKEN_GETMAD($1,$$,'{');
			  TOKEN_GETMAD($3,$$,';');
			  TOKEN_GETMAD($4,$$,'}');
			}
	|	HASHBRACK ';' '}'	%prec '(' /* { } (';' by tokener) */
			{ $$ = newANONHASH((OP*)NULL);
			  TOKEN_GETMAD($1,$$,'{');
			  TOKEN_GETMAD($2,$$,';');
			  TOKEN_GETMAD($3,$$,'}');
			}
	|	ANONSUB startanonsub proto subattrlist block	%prec '('
			{ SvREFCNT_inc_simple_void(PL_compcv);
			  $$ = newANONATTRSUB($2, $3, $4, $5);
			  TOKEN_GETMAD($1,$$,'o');
			  OP_GETMAD($3,$$,'s');
			  OP_GETMAD($4,$$,'a');
			}

    ;

/* Things called with "do" */
termdo	:       DO term	%prec UNIOP                     /* do $filename */
			{ $$ = dofile($2, IVAL($1));
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	DO block	%prec '('               /* do { code */
			{ $$ = newUNOP(OP_NULL, OPf_SPECIAL, op_scope($2));
			  TOKEN_GETMAD($1,$$,'D');
			}
	|	DO WORD lpar_or_qw ')'                  /* do somesub() */
			{ $$ = newUNOP(OP_ENTERSUB,
			    OPf_SPECIAL|OPf_STACKED,
			    op_prepend_elem(OP_LIST,
				scalar(newCVREF(
				    (OPpENTERSUB_AMPER<<8),
				    scalar($2)
				)),(OP*)NULL)); dep();
			  TOKEN_GETMAD($1,$$,'o');
			  TOKEN_GETMAD($3,$$,'(');
			  TOKEN_GETMAD($4,$$,')');
			}
	|	DO WORD lpar_or_qw expr ')'             /* do somesub(@args) */
			{ $$ = newUNOP(OP_ENTERSUB,
			    OPf_SPECIAL|OPf_STACKED,
			    op_append_elem(OP_LIST,
				$4,
				scalar(newCVREF(
				    (OPpENTERSUB_AMPER<<8),
				    scalar($2)
				)))); dep();
			  TOKEN_GETMAD($1,$$,'o');
			  TOKEN_GETMAD($3,$$,'(');
			  TOKEN_GETMAD($5,$$,')');
			}
	|	DO scalar lpar_or_qw ')'                /* do $subref () */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_SPECIAL|OPf_STACKED,
			    op_prepend_elem(OP_LIST,
				scalar(newCVREF(0,scalar($2))), (OP*)NULL)); dep();
			  TOKEN_GETMAD($1,$$,'o');
			  TOKEN_GETMAD($3,$$,'(');
			  TOKEN_GETMAD($4,$$,')');
			}
	|	DO scalar lpar_or_qw expr ')'           /* do $subref (@args) */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_SPECIAL|OPf_STACKED,
			    op_prepend_elem(OP_LIST,
				$4,
				scalar(newCVREF(0,scalar($2))))); dep();
			  TOKEN_GETMAD($1,$$,'o');
			  TOKEN_GETMAD($3,$$,'(');
			  TOKEN_GETMAD($5,$$,')');
			}

        ;

term	:	termbinop
	|	termunop
	|	anonymous
	|	termdo
	|	term '?' term ':' term
			{ $$ = newCONDOP(0, $1, $3, $5);
			  TOKEN_GETMAD($2,$$,'?');
			  TOKEN_GETMAD($4,$$,':');
			}
	|	REFGEN term                          /* \$x, \@y, \%z */
			{ $$ = newUNOP(OP_REFGEN, 0, op_lvalue($2,OP_REFGEN));
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	myattrterm	%prec UNIOP
			{ $$ = $1; }
	|	LOCAL term	%prec UNIOP
			{ $$ = localize($2,IVAL($1));
			  TOKEN_GETMAD($1,$$,'k');
			}
	|	'(' expr ')'
			{ $$ = sawparens(IF_MAD(newUNOP(OP_NULL,0,$2), $2));
			  TOKEN_GETMAD($1,$$,'(');
			  TOKEN_GETMAD($3,$$,')');
			}
	|	QWLIST
			{ $$ = IF_MAD(newUNOP(OP_NULL,0,$1), $1); }
	|	'(' ')'
			{ $$ = sawparens(newNULLLIST());
			  TOKEN_GETMAD($1,$$,'(');
			  TOKEN_GETMAD($2,$$,')');
			}
	|	scalar	%prec '('
			{ $$ = $1; }
	|	star	%prec '('
			{ $$ = $1; }
	|	hsh 	%prec '('
			{ $$ = $1; }
	|	ary 	%prec '('
			{ $$ = $1; }
	|	arylen 	%prec '('                    /* $#x, $#{ something } */
			{ $$ = newUNOP(OP_AV2ARYLEN, 0, ref($1, OP_AV2ARYLEN));}
	|       subscripted
			{ $$ = $1; }
	|	ary '[' expr ']'                     /* array slice */
			{ $$ = op_prepend_elem(OP_ASLICE,
				newOP(OP_PUSHMARK, 0),
				    newLISTOP(OP_ASLICE, 0,
					list($3),
					ref($1, OP_ASLICE)));
			  TOKEN_GETMAD($2,$$,'[');
			  TOKEN_GETMAD($4,$$,']');
			}
	|	ary '{' expr ';' '}'                 /* @hash{@keys} */
			{ $$ = op_prepend_elem(OP_HSLICE,
				newOP(OP_PUSHMARK, 0),
				    newLISTOP(OP_HSLICE, 0,
					list($3),
					ref(oopsHV($1), OP_HSLICE)));
			    PL_parser->expect = XOPERATOR;
			  TOKEN_GETMAD($2,$$,'{');
			  TOKEN_GETMAD($4,$$,';');
			  TOKEN_GETMAD($5,$$,'}');
			}
	|	THING	%prec '('
			{ $$ = $1; }
	|	amper                                /* &foo; */
			{ $$ = newUNOP(OP_ENTERSUB, 0, scalar($1)); }
	|	amper lpar_or_qw ')'                 /* &foo() */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED, scalar($1));
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($3,$$,')');
			}
	|	amper lpar_or_qw expr ')'            /* &foo(@args) */
			{
			  $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				op_append_elem(OP_LIST, $3, scalar($1)));
			  DO_MAD({
			      OP* op = $$;
			      if (op->op_type == OP_CONST) { /* defeat const fold */
				op = (OP*)op->op_madprop->mad_val;
			      }
			      token_getmad($2,op,'(');
			      token_getmad($4,op,')');
			  })
			}
	|	NOAMP WORD optlistexpr               /* foo(@args) */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
			    op_append_elem(OP_LIST, $3, scalar($2)));
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	LOOPEX  /* loop exiting command (goto, last, dump, etc) */
			{ $$ = newOP(IVAL($1), OPf_SPECIAL);
			    PL_hints |= HINT_BLOCK_SCOPE;
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	LOOPEX term
			{ $$ = newLOOPEX(IVAL($1),$2);
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	NOTOP listexpr                       /* not $foo */
			{ $$ = newUNOP(OP_NOT, 0, scalar($2));
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	UNIOP                                /* Unary op, $_ implied */
			{ $$ = newOP(IVAL($1), 0);
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	UNIOP block                          /* eval { foo }* */
			{ $$ = newUNOP(IVAL($1), 0, $2);
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	UNIOP term                           /* Unary op */
			{ $$ = newUNOP(IVAL($1), 0, $2);
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	REQUIRE                              /* require, $_ implied */
			{ $$ = newOP(OP_REQUIRE, $1 ? OPf_SPECIAL : 0);
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	REQUIRE term                         /* require Foo */
			{ $$ = newUNOP(OP_REQUIRE, $1 ? OPf_SPECIAL : 0, $2);
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	UNIOPSUB
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED, scalar($1)); }
	|	UNIOPSUB term                        /* Sub treated as unop */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
			    op_append_elem(OP_LIST, $2, scalar($1))); }
	|	FUNC0                                /* Nullary operator */
			{ $$ = newOP(IVAL($1), 0);
			  TOKEN_GETMAD($1,$$,'o');
			}
	|	FUNC0 '(' ')'
			{ $$ = newOP(IVAL($1), 0);
			  TOKEN_GETMAD($1,$$,'o');
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($3,$$,')');
			}
	|	FUNC0OP       /* Same as above, but op created in toke.c */
			{ $$ = $1; }
	|	FUNC0OP '(' ')'
			{ $$ = $1;
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($3,$$,')');
			}
	|	FUNC0SUB                             /* Sub treated as nullop */
			{ $$ = newUNOP(OP_ENTERSUB, OPf_STACKED,
				scalar($1)); }
	|	FUNC1 '(' ')'                        /* not () */
			{ $$ = (IVAL($1) == OP_NOT)
			    ? newUNOP(IVAL($1), 0, newSVOP(OP_CONST, 0, newSViv(0)))
			    : newOP(IVAL($1), OPf_SPECIAL);

			  TOKEN_GETMAD($1,$$,'o');
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($3,$$,')');
			}
	|	FUNC1 '(' expr ')'                   /* not($foo) */
			{ $$ = newUNOP(IVAL($1), 0, $3);
			  TOKEN_GETMAD($1,$$,'o');
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($4,$$,')');
			}
	|	PMFUNC '(' listexpr ')'		/* m//, s///, tr/// */
			{ $$ = pmruntime($1, $3, 1);
			  TOKEN_GETMAD($2,$$,'(');
			  TOKEN_GETMAD($4,$$,')');
			}
	|	WORD
	|	listop
	|	YADAYADA
			{
			  $$ = newLISTOP(OP_DIE, 0, newOP(OP_PUSHMARK, 0),
				newSVOP(OP_CONST, 0, newSVpvs("Unimplemented")));
			  TOKEN_GETMAD($1,$$,'X');
			}
	|	PLUGEXPR
	;

/* "my" declarations, with optional attributes */
myattrterm:	MY myterm myattrlist
			{ $$ = my_attrs($2,$3);
			  DO_MAD(
			      token_getmad($1,$$,'d');
			      append_madprops($3->op_madprop, $$, 'a');
			      $3->op_madprop = 0;
			  )
			}
	|	MY myterm
			{ $$ = localize($2,IVAL($1));
			  TOKEN_GETMAD($1,$$,'d');
			}
	;

/* Things that can be "my"'d */
myterm	:	'(' expr ')'
			{ $$ = sawparens($2);
			  TOKEN_GETMAD($1,$$,'(');
			  TOKEN_GETMAD($3,$$,')');
			}
	|	'(' ')'
			{ $$ = sawparens(newNULLLIST());
			  TOKEN_GETMAD($1,$$,'(');
			  TOKEN_GETMAD($2,$$,')');
			}
	|	scalar	%prec '('
			{ $$ = $1; }
	|	hsh 	%prec '('
			{ $$ = $1; }
	|	ary 	%prec '('
			{ $$ = $1; }
	;

/* Basic list expressions */
optlistexpr:	/* NULL */ %prec PREC_LOW
			{ $$ = (OP*)NULL; }
	|	listexpr    %prec PREC_LOW
			{ $$ = $1; }
	;

optexpr:	/* NULL */
			{ $$ = (OP*)NULL; }
	|	expr
			{ $$ = $1; }
	;

lpar_or_qw:	'('
			{ $$ = $1; }
	|	QWLIST
			{ munge_qwlist_to_paren_list($1); }
		'('
			{ $$ = $3; }
	;

/* A little bit of trickery to make "for my $foo (@bar)" actually be
   lexical */
my_scalar:	scalar
			{ PL_parser->in_my = 0; $$ = my($1); }
	;

amper	:	'&' indirob
			{ $$ = newCVREF(IVAL($1),$2);
			  TOKEN_GETMAD($1,$$,'&');
			}
	;

scalar	:	'$' indirob
			{ $$ = newSVREF($2);
			  TOKEN_GETMAD($1,$$,'$');
			}
	;

ary	:	'@' indirob
			{ $$ = newAVREF($2);
			  TOKEN_GETMAD($1,$$,'@');
			}
	;

hsh	:	'%' indirob
			{ $$ = newHVREF($2);
			  TOKEN_GETMAD($1,$$,'%');
			}
	;

arylen	:	DOLSHARP indirob
			{ $$ = newAVREF($2);
			  TOKEN_GETMAD($1,$$,'l');
			}
	;

star	:	'*' indirob
			{ $$ = newGVREF(0,$2);
			  TOKEN_GETMAD($1,$$,'*');
			}
	;

/* Indirect objects */
indirob	:	WORD
			{ $$ = scalar($1); }
	|	scalar %prec PREC_LOW
			{ $$ = scalar($1); }
	|	block
			{ $$ = op_scope($1); }

	|	PRIVATEREF
			{ $$ = $1; }
	;
