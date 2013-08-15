/*    perly.c
 *
 *    Copyright (c) 2004, 2005, 2006, 2007, 2008,
 *    2009, 2010, 2011 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 * 
 *    Note that this file was originally generated as an output from
 *    GNU bison version 1.875, but now the code is statically maintained
 *    and edited; the bits that are dependent on perly.y are now
 *    #included from the files perly.tab and perly.act.
 *
 *    Here is an important copyright statement from the original, generated
 *    file:
 *
 *	As a special exception, when this file is copied by Bison into a
 *	Bison output file, you may use that output file without
 *	restriction.  This special exception was added by the Free
 *	Software Foundation in version 1.24 of Bison.
 *
 * Note that this file is also #included in madly.c, to allow compilation
 * of a second parser, Perl_madparse, that is identical to Perl_yyparse,
 * but which includes extra code for dumping the parse tree.
 * This is controlled by the PERL_IN_MADLY_C define.
 */

#include "EXTERN.h"
#define PERL_IN_PERLY_C
#include "perl.h"

typedef unsigned char yytype_uint8;
typedef signed char yytype_int8;
typedef unsigned short int yytype_uint16;
typedef short int yytype_int16;
typedef signed char yysigned_char;

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#define YYINITDEPTH 200

#ifdef DEBUGGING
#  define YYDEBUG 1
#else
#  define YYDEBUG 0
#endif

/* contains all the parser state tables; auto-generated from perly.y */
#include "perly.tab"

# define YYSIZE_T size_t

#define YYEOF		0
#define YYTERROR	1

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Enable debugging if requested.  */
#ifdef DEBUGGING

#  define yydebug (DEBUG_p_TEST)

#  define YYFPRINTF PerlIO_printf

#  define YYDPRINTF(Args)			\
do {						\
    if (yydebug)				\
	YYFPRINTF Args;				\
} while (0)

#  define YYDSYMPRINTF(Title, Token, Value)			\
do {								\
    if (yydebug) {						\
	YYFPRINTF (Perl_debug_log, "%s ", Title);		\
	yysymprint (aTHX_ Perl_debug_log,  Token, Value);	\
	YYFPRINTF (Perl_debug_log, "\n");			\
    }								\
} while (0)

/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yysymprint(pTHX_ PerlIO * const yyoutput, int yytype, const YYSTYPE * const yyvaluep)
{
    if (yytype < YYNTOKENS) {
	YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
#   ifdef YYPRINT
	YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
#   else
	YYFPRINTF (yyoutput, "0x%"UVxf, (UV)yyvaluep->ival);
#   endif
    }
    else
	YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

    YYFPRINTF (yyoutput, ")");
}


/*  yy_stack_print()
 *  print the top 8 items on the parse stack.
 */

static void
yy_stack_print (pTHX_ const yy_parser *parser)
{
    const yy_stack_frame *ps, *min;

    min = parser->ps - 8 + 1;
    if (min <= parser->stack)
	min = parser->stack + 1;

    PerlIO_printf(Perl_debug_log, "\nindex:");
    for (ps = min; ps <= parser->ps; ps++)
	PerlIO_printf(Perl_debug_log, " %8d", (int)(ps - parser->stack));

    PerlIO_printf(Perl_debug_log, "\nstate:");
    for (ps = min; ps <= parser->ps; ps++)
	PerlIO_printf(Perl_debug_log, " %8d", ps->state);

    PerlIO_printf(Perl_debug_log, "\ntoken:");
    for (ps = min; ps <= parser->ps; ps++)
	PerlIO_printf(Perl_debug_log, " %8.8s", ps->name);

    PerlIO_printf(Perl_debug_log, "\nvalue:");
    for (ps = min; ps <= parser->ps; ps++) {
	switch (yy_type_tab[yystos[ps->state]]) {
	case toketype_opval:
	    PerlIO_printf(Perl_debug_log, " %8.8s",
		  ps->val.opval
		    ? PL_op_name[ps->val.opval->op_type]
		    : "(Nullop)"
	    );
	    break;
#ifndef PERL_IN_MADLY_C
	case toketype_i_tkval:
#endif
	case toketype_ival:
	    PerlIO_printf(Perl_debug_log, " %8"IVdf, (IV)ps->val.ival);
	    break;
	default:
	    PerlIO_printf(Perl_debug_log, " %8"UVxf, (UV)ps->val.ival);
	}
    }
    PerlIO_printf(Perl_debug_log, "\n\n");
}

#  define YY_STACK_PRINT(parser)	\
do {					\
    if (yydebug && DEBUG_v_TEST)	\
	yy_stack_print (aTHX_ parser);	\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (pTHX_ int yyrule)
{
    int yyi;
    const unsigned int yylineno = yyrline[yyrule];
    YYFPRINTF (Perl_debug_log, "Reducing stack by rule %d (line %u), ",
			  yyrule - 1, yylineno);
    /* Print the symbols being reduced, and their result.  */
    for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
	YYFPRINTF (Perl_debug_log, "%s ", yytname [yyrhs[yyi]]);
    YYFPRINTF (Perl_debug_log, "-> %s\n", yytname [yyr1[yyrule]]);
}

#  define YY_REDUCE_PRINT(Rule)		\
do {					\
    if (yydebug)			\
	yy_reduce_print (aTHX_ Rule);		\
} while (0)

#else /* !DEBUGGING */
#  define YYDPRINTF(Args)
#  define YYDSYMPRINTF(Title, Token, Value)
#  define YY_STACK_PRINT(parser)
#  define YY_REDUCE_PRINT(Rule)
#endif /* !DEBUGGING */

/* called during cleanup (via SAVEDESTRUCTOR_X) to free any items on the
 * parse stack, thus avoiding leaks if we die  */

static void
S_clear_yystack(pTHX_  const yy_parser *parser)
{
    yy_stack_frame *ps     = parser->ps;
    int i = 0;

    if (!parser->stack)
	return;

    YYDPRINTF ((Perl_debug_log, "clearing the parse stack\n"));

    /* Freeing ops on the stack, and the op_latefree / op_latefreed /
     * op_attached flags:
     *
     * When we pop tokens off the stack during error recovery, or when
     * we pop all the tokens off the stack after a die during a shift or
     * reduce (i.e. Perl_croak somewhere in yylex() or in one of the
     * newFOO() functions), then it's possible that some of these tokens are
     * of type opval, pointing to an OP. All these ops are orphans; each is
     * its own miniature subtree that has not yet been attached to a
     * larger tree. In this case, we should clearly free the op (making
     * sure, for each op we free that we have PL_comppad pointing to the
     * right place for freeing any SVs attached to the op in threaded
     * builds.
     *
     * However, there is a particular problem if we die in newFOO() called
     * by a reducing action; e.g.
     *
     *    foo : bar baz boz
     *        { $$ = newFOO($1,$2,$3) }
     *
     * where
     *  OP *newFOO { ....; if (...) croak; .... }
     *
     * In this case, when we come to clean bar baz and boz off the stack,
     * we don't know whether newFOO() has already:
     *    * freed them
     *    * left them as is
     *    * attached them to part of a larger tree
     *    * attached them to PL_compcv
     *    * attached them to PL_compcv then freed it (as in BEGIN {die } )
     *
     * To get round this problem, we set the flag op_latefree on every op
     * that gets pushed onto the parser stack. If op_free() sees this
     * flag, it clears the op and frees any children,, but *doesn't* free
     * the op itself; instead it sets the op_latefreed flag. This means
     * that we can safely call op_free() multiple times on each stack op.
     * So, when clearing the stack, we first, for each op that was being
     * reduced, call op_free with op_latefree=1. This ensures that all ops
     * hanging off these op are freed, but the reducing ops themselves are
     * just undefed. Then we set op_latefreed=0 on *all* ops on the stack
     * and free them. A little thought should convince you that this
     * two-part approach to the reducing ops should handle the first three
     * cases above safely.
     *
     * In the case of attaching to PL_compcv (currently just newATTRSUB
     * does this), then  we set the op_attached flag on the op that has
     * been so attached, then avoid doing the final op_free during
     * cleanup, on the assumption that it will happen (or has already
     * happened) when PL_compcv is freed.
     *
     * Note this is fairly fragile mechanism. A more robust approach
     * would be to use two of these flag bits as 2-bit reference count
     * field for each op, indicating whether it is pointed to from:
     *   * a parent op
     *   * the parser stack
     *   * a CV
     * but this would involve reworking all code (core and external) that
     * manipulate op trees.
     *
     * XXX DAPM 17/1/07 I've decided its too fragile for now, and so have
     * disabled it */

#define DISABLE_STACK_FREE


#ifdef DISABLE_STACK_FREE
    for (i=0; i< parser->yylen; i++) {
	SvREFCNT_dec(ps[-i].compcv);
    }
    ps -= parser->yylen;
#else
    /* clear any reducing ops (1st pass) */

    for (i=0; i< parser->yylen; i++) {
	LEAVE_SCOPE(ps[-i].savestack_ix);
	if (yy_type_tab[yystos[ps[-i].state]] == toketype_opval
	    && ps[-i].val.opval) {
	    if ( ! (ps[-i].val.opval->op_attached
		    && !ps[-i].val.opval->op_latefreed))
	    {
		if (ps[-i].compcv != PL_compcv) {
		    PL_compcv = ps[-i].compcv;
		    PAD_SET_CUR_NOSAVE(CvPADLIST(PL_compcv), 1);
		}
		op_free(ps[-i].val.opval);
	    }
	}
    }
#endif

    /* now free whole the stack, including the just-reduced ops */

    while (ps > parser->stack) {
	LEAVE_SCOPE(ps->savestack_ix);
	if (yy_type_tab[yystos[ps->state]] == toketype_opval
	    && ps->val.opval)
	{
	    if (ps->compcv != PL_compcv) {
		PL_compcv = ps->compcv;
		PAD_SET_CUR_NOSAVE(CvPADLIST(PL_compcv), 1);
	    }
	    YYDPRINTF ((Perl_debug_log, "(freeing op)\n"));
#ifndef DISABLE_STACK_FREE
	    ps->val.opval->op_latefree  = 0;
	    if (!(ps->val.opval->op_attached && !ps->val.opval->op_latefreed))
#endif
		op_free(ps->val.opval);
	}
	SvREFCNT_dec(ps->compcv);
	ps--;
    }

    Safefree(parser->stack);
}


/*----------.
| yyparse.  |
`----------*/

int
#ifdef PERL_IN_MADLY_C
Perl_madparse (pTHX_ int gramtype)
#else
Perl_yyparse (pTHX_ int gramtype)
#endif
{
    dVAR;
    register int yystate;
    register int yyn;
    int yyresult;

    /* Lookahead token as an internal (translated) token number.  */
    int yytoken = 0;

    register yy_parser *parser;	    /* the parser object */
    register yy_stack_frame  *ps;   /* current parser stack frame */

#define YYPOPSTACK   parser->ps = --ps
#define YYPUSHSTACK  parser->ps = ++ps

    /* The variable used to return semantic value and location from the
	  action routines: ie $$.  */
    YYSTYPE yyval;

#ifndef PERL_IN_MADLY_C
#  ifdef PERL_MAD
    if (PL_madskills)
	return madparse(gramtype);
#  endif
#endif

    YYDPRINTF ((Perl_debug_log, "Starting parse\n"));

    parser = PL_parser;

    ENTER;  /* force parser state cleanup/restoration before we return */
    SAVEPPTR(parser->yylval.pval);
    SAVEINT(parser->yychar);
    SAVEINT(parser->yyerrstatus);
    SAVEINT(parser->stack_size);
    SAVEINT(parser->yylen);
    SAVEVPTR(parser->stack);
    SAVEVPTR(parser->ps);

    /* initialise state for this parse */
    parser->yychar = gramtype;
    parser->yyerrstatus = 0;
    parser->stack_size = YYINITDEPTH;
    parser->yylen = 0;
    Newx(parser->stack, YYINITDEPTH, yy_stack_frame);
    ps = parser->ps = parser->stack;
    ps->state = 0;
    SAVEDESTRUCTOR_X(S_clear_yystack, parser);

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
  yynewstate:

    yystate = ps->state;

    YYDPRINTF ((Perl_debug_log, "Entering state %d\n", yystate));

#ifndef DISABLE_STACK_FREE
    if (yy_type_tab[yystos[yystate]] == toketype_opval && ps->val.opval) {
	ps->val.opval->op_latefree  = 1;
	ps->val.opval->op_latefreed = 0;
    }
#endif

    parser->yylen = 0;

    {
	size_t size = ps - parser->stack + 1;

	/* grow the stack? We always leave 1 spare slot,
	 * in case of a '' -> 'foo' reduction */

	if (size >= (size_t)parser->stack_size - 1) {
	    /* this will croak on insufficient memory */
	    parser->stack_size *= 2;
	    Renew(parser->stack, parser->stack_size, yy_stack_frame);
	    ps = parser->ps = parser->stack + size -1;

	    YYDPRINTF((Perl_debug_log,
			    "parser stack size increased to %lu frames\n",
			    (unsigned long int)parser->stack_size));
	}
    }

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */

    /* First try to decide what to do without reference to lookahead token.  */

    yyn = yypact[yystate];
    if (yyn == YYPACT_NINF)
	goto yydefault;

    /* Not known => get a lookahead token if don't already have one.  */

    /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
    if (parser->yychar == YYEMPTY) {
	YYDPRINTF ((Perl_debug_log, "Reading a token: "));
#ifdef PERL_IN_MADLY_C
	parser->yychar = PL_madskills ? madlex() : yylex();
#else
	parser->yychar = yylex();
#endif

#  ifdef EBCDIC
	if (parser->yychar >= 0 && parser->yychar < 255) {
	    parser->yychar = NATIVE_TO_ASCII(parser->yychar);
	}
#  endif
    }

    if (parser->yychar <= YYEOF) {
	parser->yychar = yytoken = YYEOF;
	YYDPRINTF ((Perl_debug_log, "Now at end of input.\n"));
    }
    else {
	yytoken = YYTRANSLATE (parser->yychar);
	YYDSYMPRINTF ("Next token is", yytoken, &parser->yylval);
    }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
	  detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
	goto yydefault;
    yyn = yytable[yyn];
    if (yyn <= 0) {
	if (yyn == 0 || yyn == YYTABLE_NINF)
	    goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
    }

    if (yyn == YYFINAL)
	YYACCEPT;

    /* Shift the lookahead token.  */
    YYDPRINTF ((Perl_debug_log, "Shifting token %s, ", yytname[yytoken]));

    /* Discard the token being shifted unless it is eof.  */
    if (parser->yychar != YYEOF)
	parser->yychar = YYEMPTY;

    YYPUSHSTACK;
    ps->state   = yyn;
    ps->val     = parser->yylval;
    ps->compcv  = (CV*)SvREFCNT_inc(PL_compcv);
    ps->savestack_ix = PL_savestack_ix;
#ifdef DEBUGGING
    ps->name    = (const char *)(yytname[yytoken]);
#endif

    /* Count tokens shifted since error; after three, turn off error
	  status.  */
    if (parser->yyerrstatus)
	parser->yyerrstatus--;

    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact[yystate];
    if (yyn == 0)
	goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    /* yyn is the number of a rule to reduce with.  */
    parser->yylen = yyr2[yyn];

    /* If YYLEN is nonzero, implement the default value of the action:
      "$$ = $1".

      Otherwise, the following line sets YYVAL to garbage.
      This behavior is undocumented and Bison
      users should not rely upon it.  Assigning to YYVAL
      unconditionally makes the parser a bit smaller, and it avoids a
      GCC warning that YYVAL may be used uninitialized.  */
    yyval = ps[1-parser->yylen].val;

    YY_STACK_PRINT(parser);
    YY_REDUCE_PRINT (yyn);

    switch (yyn) {


#define dep() deprecate("\"do\" to call subroutines")

#ifdef PERL_IN_MADLY_C
#  define IVAL(i) (i)->tk_lval.ival
#  define PVAL(p) (p)->tk_lval.pval
#  define TOKEN_GETMAD(a,b,c) token_getmad((a),(b),(c))
#  define TOKEN_FREE(a) token_free(a)
#  define OP_GETMAD(a,b,c) op_getmad((a),(b),(c))
#  define IF_MAD(a,b) (a)
#  define DO_MAD(a) a
#  define MAD
#else
#  define IVAL(i) (i)
#  define PVAL(p) (p)
#  define TOKEN_GETMAD(a,b,c)
#  define TOKEN_FREE(a)
#  define OP_GETMAD(a,b,c)
#  define IF_MAD(a,b) (b)
#  define DO_MAD(a)
#  undef MAD
#endif

/* contains all the rule actions; auto-generated from perly.y */
#include "perly.act"

    }

    /* any just-reduced ops with the op_latefreed flag cleared need to be
     * freed; the rest need the flag resetting */
    {
	int i;
	for (i=0; i< parser->yylen; i++) {
#ifndef DISABLE_STACK_FREE
	    if (yy_type_tab[yystos[ps[-i].state]] == toketype_opval
		&& ps[-i].val.opval)
	    {
		ps[-i].val.opval->op_latefree = 0;
		if (ps[-i].val.opval->op_latefreed)
		    op_free(ps[-i].val.opval);
	    }
#endif
	    SvREFCNT_dec(ps[-i].compcv);
	}
    }

    parser->ps = ps -= (parser->yylen-1);

    /* Now shift the result of the reduction.  Determine what state
	  that goes to, based on the state we popped back to and the rule
	  number reduced by.  */

    ps->val     = yyval;
    ps->compcv  = (CV*)SvREFCNT_inc(PL_compcv);
    ps->savestack_ix = PL_savestack_ix;
#ifdef DEBUGGING
    ps->name    = (const char *)(yytname [yyr1[yyn]]);
#endif

    yyn = yyr1[yyn];

    yystate = yypgoto[yyn - YYNTOKENS] + ps[-1].state;
    if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == ps[-1].state)
	yystate = yytable[yystate];
    else
	yystate = yydefgoto[yyn - YYNTOKENS];
    ps->state = yystate;

    goto yynewstate;


  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* If not already recovering from an error, report this error.  */
    if (!parser->yyerrstatus) {
	yyerror ("syntax error");
    }


    if (parser->yyerrstatus == 3) {
	/* If just tried and failed to reuse lookahead token after an
	      error, discard it.  */

	/* Return failure if at end of input.  */
	if (parser->yychar == YYEOF) {
	    /* Pop the error token.  */
	    SvREFCNT_dec(ps->compcv);
	    YYPOPSTACK;
	    /* Pop the rest of the stack.  */
	    while (ps > parser->stack) {
		YYDSYMPRINTF ("Error: popping", yystos[ps->state], &ps->val);
		LEAVE_SCOPE(ps->savestack_ix);
		if (yy_type_tab[yystos[ps->state]] == toketype_opval
			&& ps->val.opval)
		{
		    YYDPRINTF ((Perl_debug_log, "(freeing op)\n"));
		    if (ps->compcv != PL_compcv) {
			PL_compcv = ps->compcv;
			PAD_SET_CUR_NOSAVE(CvPADLIST(PL_compcv), 1);
		    }
		    ps->val.opval->op_latefree  = 0;
		    op_free(ps->val.opval);
		}
		SvREFCNT_dec(ps->compcv);
		YYPOPSTACK;
	    }
	    YYABORT;
	}

	YYDSYMPRINTF ("Error: discarding", yytoken, &parser->yylval);
	if (yy_type_tab[yytoken] == toketype_opval)
	    op_free(parser->yylval.opval);
	parser->yychar = YYEMPTY;

    }

    /* Else will try to reuse lookahead token after shifting the error
	  token.  */
    goto yyerrlab1;


  /*----------------------------------------------------.
  | yyerrlab1 -- error raised explicitly by an action.  |
  `----------------------------------------------------*/
  yyerrlab1:
    parser->yyerrstatus = 3;	/* Each real token shifted decrements this.  */

    for (;;) {
	yyn = yypact[yystate];
	if (yyn != YYPACT_NINF) {
	    yyn += YYTERROR;
	    if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR) {
		yyn = yytable[yyn];
		if (0 < yyn)
		    break;
	    }
	}

	/* Pop the current state because it cannot handle the error token.  */
	if (ps == parser->stack)
	    YYABORT;

	YYDSYMPRINTF ("Error: popping", yystos[ps->state], &ps->val);
	LEAVE_SCOPE(ps->savestack_ix);
	if (yy_type_tab[yystos[ps->state]] == toketype_opval && ps->val.opval) {
	    YYDPRINTF ((Perl_debug_log, "(freeing op)\n"));
	    if (ps->compcv != PL_compcv) {
		PL_compcv = ps->compcv;
		PAD_SET_CUR_NOSAVE(CvPADLIST(PL_compcv), 1);
	    }
	    ps->val.opval->op_latefree  = 0;
	    op_free(ps->val.opval);
	}
	SvREFCNT_dec(ps->compcv);
	YYPOPSTACK;
	yystate = ps->state;

	YY_STACK_PRINT(parser);
    }

    if (yyn == YYFINAL)
	YYACCEPT;

    YYDPRINTF ((Perl_debug_log, "Shifting error token, "));

    YYPUSHSTACK;
    ps->state   = yyn;
    ps->val     = parser->yylval;
    ps->compcv  = (CV*)SvREFCNT_inc(PL_compcv);
    ps->savestack_ix = PL_savestack_ix;
#ifdef DEBUGGING
    ps->name    ="<err>";
#endif

    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    for (ps=parser->ps; ps > parser->stack; ps--) {
	SvREFCNT_dec(ps->compcv);
    }
    parser->ps = parser->stack; /* disable cleanup */
    goto yyreturn;

  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    LEAVE;	/* force parser stack cleanup before we return */
    return yyresult;
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
