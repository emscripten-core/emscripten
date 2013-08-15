/*    cop.h
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * Control ops (cops) are one of the two ops OP_NEXTSTATE and OP_DBSTATE,
 * that (loosely speaking) are separate statements.
 * They hold information important for lexical state and error reporting.
 * At run time, PL_curcop is set to point to the most recently executed cop,
 * and thus can be used to determine our current state.
 */

/* A jmpenv packages the state required to perform a proper non-local jump.
 * Note that there is a PL_start_env initialized when perl starts, and
 * PL_top_env points to this initially, so PL_top_env should always be
 * non-null.
 *
 * Existence of a non-null PL_top_env->je_prev implies it is valid to call
 * longjmp() at that runlevel (we make sure PL_start_env.je_prev is always
 * null to ensure this).
 *
 * je_mustcatch, when set at any runlevel to TRUE, means eval ops must
 * establish a local jmpenv to handle exception traps.  Care must be taken
 * to restore the previous value of je_mustcatch before exiting the
 * stack frame iff JMPENV_PUSH was not called in that stack frame.
 * GSAR 97-03-27
 */

struct jmpenv {
    struct jmpenv *	je_prev;
    Sigjmp_buf		je_buf;		/* only for use if !je_throw */
    int			je_ret;		/* last exception thrown */
    bool		je_mustcatch;	/* need to call longjmp()? */
};

typedef struct jmpenv JMPENV;

#ifdef OP_IN_REGISTER
#define OP_REG_TO_MEM	PL_opsave = op
#define OP_MEM_TO_REG	op = PL_opsave
#else
#define OP_REG_TO_MEM	NOOP
#define OP_MEM_TO_REG	NOOP
#endif

/*
 * How to build the first jmpenv.
 *
 * top_env needs to be non-zero. It points to an area
 * in which longjmp() stuff is stored, as C callstack
 * info there at least is thread specific this has to
 * be per-thread. Otherwise a 'die' in a thread gives
 * that thread the C stack of last thread to do an eval {}!
 */

#define JMPENV_BOOTSTRAP \
    STMT_START {				\
	Zero(&PL_start_env, 1, JMPENV);		\
	PL_start_env.je_ret = -1;		\
	PL_start_env.je_mustcatch = TRUE;	\
	PL_top_env = &PL_start_env;		\
    } STMT_END

/*
 *   PERL_FLEXIBLE_EXCEPTIONS
 * 
 * All the flexible exceptions code has been removed.
 * See the following threads for details:
 *
 *   http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/2004-07/msg00378.html
 * 
 * Joshua's original patches (which weren't applied) and discussion:
 * 
 *   http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1998-02/msg01396.html
 *   http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1998-02/msg01489.html
 *   http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1998-02/msg01491.html
 *   http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1998-02/msg01608.html
 *   http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1998-02/msg02144.html
 *   http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1998-02/msg02998.html
 * 
 * Chip's reworked patch and discussion:
 * 
 *   http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1999-03/msg00520.html
 * 
 * The flaw in these patches (which went unnoticed at the time) was
 * that they moved some code that could potentially die() out of the
 * region protected by the setjmp()s.  This caused exceptions within
 * END blocks and such to not be handled by the correct setjmp().
 * 
 * The original patches that introduces flexible exceptions were:
 *
 * http://perl5.git.perl.org/perl.git/commit/312caa8e97f1c7ee342a9895c2f0e749625b4929
 * http://perl5.git.perl.org/perl.git/commit/14dd3ad8c9bf82cf09798a22cc89a9862dfd6d1a                                        
 *  
 */

#define dJMPENV		JMPENV cur_env

#define JMPENV_PUSH(v) \
    STMT_START {							\
	DEBUG_l({							\
	    int i = 0; JMPENV *p = PL_top_env;				\
	    while (p) { i++; p = p->je_prev; }				\
	    Perl_deb(aTHX_ "JUMPENV_PUSH level=%d at %s:%d\n",		\
		         i,  __FILE__, __LINE__);})			\
	cur_env.je_prev = PL_top_env;					\
	OP_REG_TO_MEM;							\
	cur_env.je_ret = PerlProc_setjmp(cur_env.je_buf, SCOPE_SAVES_SIGNAL_MASK);		\
	OP_MEM_TO_REG;							\
	PL_top_env = &cur_env;						\
	cur_env.je_mustcatch = FALSE;					\
	(v) = cur_env.je_ret;						\
    } STMT_END

#define JMPENV_POP \
    STMT_START {							\
	DEBUG_l({							\
	    int i = -1; JMPENV *p = PL_top_env;				\
	    while (p) { i++; p = p->je_prev; }				\
	    Perl_deb(aTHX_ "JUMPENV_POP level=%d at %s:%d\n",		\
		         i, __FILE__, __LINE__);})			\
	assert(PL_top_env == &cur_env);					\
	PL_top_env = cur_env.je_prev;					\
    } STMT_END

#define JMPENV_JUMP(v) \
    STMT_START {						\
	DEBUG_l({						\
	    int i = -1; JMPENV *p = PL_top_env;			\
	    while (p) { i++; p = p->je_prev; }			\
	    Perl_deb(aTHX_ "JUMPENV_JUMP(%d) level=%d at %s:%d\n", \
		         (int)v, i, __FILE__, __LINE__);})	\
	OP_REG_TO_MEM;						\
	if (PL_top_env->je_prev)				\
	    PerlProc_longjmp(PL_top_env->je_buf, (v));		\
	if ((v) == 2)						\
	    PerlProc_exit(STATUS_EXIT);		                \
	PerlIO_printf(PerlIO_stderr(), "panic: top_env, v=%d\n", (int)v); \
	PerlProc_exit(1);					\
    } STMT_END

#define CATCH_GET		(PL_top_env->je_mustcatch)
#define CATCH_SET(v) \
    STMT_START {							\
	DEBUG_l(							\
	    Perl_deb(aTHX_						\
		"JUMPLEVEL set catch %d => %d (for %p) at %s:%d\n",	\
		 PL_top_env->je_mustcatch, v, (void*)PL_top_env,	\
		 __FILE__, __LINE__);)					\
	PL_top_env->je_mustcatch = (v);					\
    } STMT_END

/*
=head1 COP Hint Hashes
*/

typedef struct refcounted_he COPHH;

#define COPHH_KEY_UTF8 REFCOUNTED_HE_KEY_UTF8

/*
=for apidoc Amx|SV *|cophh_fetch_pvn|const COPHH *cophh|const char *keypv|STRLEN keylen|U32 hash|U32 flags

Look up the entry in the cop hints hash I<cophh> with the key specified by
I<keypv> and I<keylen>.  If I<flags> has the C<COPHH_KEY_UTF8> bit set,
the key octets are interpreted as UTF-8, otherwise they are interpreted
as Latin-1.  I<hash> is a precomputed hash of the key string, or zero if
it has not been precomputed.  Returns a mortal scalar copy of the value
associated with the key, or C<&PL_sv_placeholder> if there is no value
associated with the key.

=cut
*/

#define cophh_fetch_pvn(cophh, keypv, keylen, hash, flags) \
    Perl_refcounted_he_fetch_pvn(aTHX_ cophh, keypv, keylen, hash, flags)

/*
=for apidoc Amx|SV *|cophh_fetch_pvs|const COPHH *cophh|const char *key|U32 flags

Like L</cophh_fetch_pvn>, but takes a literal string instead of a
string/length pair, and no precomputed hash.

=cut
*/

#define cophh_fetch_pvs(cophh, key, flags) \
    Perl_refcounted_he_fetch_pvn(aTHX_ cophh, STR_WITH_LEN(key), 0, flags)

/*
=for apidoc Amx|SV *|cophh_fetch_pv|const COPHH *cophh|const char *key|U32 hash|U32 flags

Like L</cophh_fetch_pvn>, but takes a nul-terminated string instead of
a string/length pair.

=cut
*/

#define cophh_fetch_pv(cophh, key, hash, flags) \
    Perl_refcounted_he_fetch_pv(aTHX_ cophh, key, hash, flags)

/*
=for apidoc Amx|SV *|cophh_fetch_sv|const COPHH *cophh|SV *key|U32 hash|U32 flags

Like L</cophh_fetch_pvn>, but takes a Perl scalar instead of a
string/length pair.

=cut
*/

#define cophh_fetch_sv(cophh, key, hash, flags) \
    Perl_refcounted_he_fetch_sv(aTHX_ cophh, key, hash, flags)

/*
=for apidoc Amx|HV *|cophh_2hv|const COPHH *cophh|U32 flags

Generates and returns a standard Perl hash representing the full set of
key/value pairs in the cop hints hash I<cophh>.  I<flags> is currently
unused and must be zero.

=cut
*/

#define cophh_2hv(cophh, flags) \
    Perl_refcounted_he_chain_2hv(aTHX_ cophh, flags)

/*
=for apidoc Amx|COPHH *|cophh_copy|COPHH *cophh

Make and return a complete copy of the cop hints hash I<cophh>.

=cut
*/

#define cophh_copy(cophh) Perl_refcounted_he_inc(aTHX_ cophh)

/*
=for apidoc Amx|void|cophh_free|COPHH *cophh

Discard the cop hints hash I<cophh>, freeing all resources associated
with it.

=cut
*/

#define cophh_free(cophh) Perl_refcounted_he_free(aTHX_ cophh)

/*
=for apidoc Amx|COPHH *|cophh_new_empty

Generate and return a fresh cop hints hash containing no entries.

=cut
*/

#define cophh_new_empty() ((COPHH *)NULL)

/*
=for apidoc Amx|COPHH *|cophh_store_pvn|COPHH *cophh|const char *keypv|STRLEN keylen|U32 hash|SV *value|U32 flags

Stores a value, associated with a key, in the cop hints hash I<cophh>,
and returns the modified hash.  The returned hash pointer is in general
not the same as the hash pointer that was passed in.  The input hash is
consumed by the function, and the pointer to it must not be subsequently
used.  Use L</cophh_copy> if you need both hashes.

The key is specified by I<keypv> and I<keylen>.  If I<flags> has the
C<COPHH_KEY_UTF8> bit set, the key octets are interpreted as UTF-8,
otherwise they are interpreted as Latin-1.  I<hash> is a precomputed
hash of the key string, or zero if it has not been precomputed.

I<value> is the scalar value to store for this key.  I<value> is copied
by this function, which thus does not take ownership of any reference
to it, and later changes to the scalar will not be reflected in the
value visible in the cop hints hash.  Complex types of scalar will not
be stored with referential integrity, but will be coerced to strings.

=cut
*/

#define cophh_store_pvn(cophh, keypv, keylen, hash, value, flags) \
    Perl_refcounted_he_new_pvn(aTHX_ cophh, keypv, keylen, hash, value, flags)

/*
=for apidoc Amx|COPHH *|cophh_store_pvs|const COPHH *cophh|const char *key|SV *value|U32 flags

Like L</cophh_store_pvn>, but takes a literal string instead of a
string/length pair, and no precomputed hash.

=cut
*/

#define cophh_store_pvs(cophh, key, value, flags) \
    Perl_refcounted_he_new_pvn(aTHX_ cophh, STR_WITH_LEN(key), 0, value, flags)

/*
=for apidoc Amx|COPHH *|cophh_store_pv|const COPHH *cophh|const char *key|U32 hash|SV *value|U32 flags

Like L</cophh_store_pvn>, but takes a nul-terminated string instead of
a string/length pair.

=cut
*/

#define cophh_store_pv(cophh, key, hash, value, flags) \
    Perl_refcounted_he_new_pv(aTHX_ cophh, key, hash, value, flags)

/*
=for apidoc Amx|COPHH *|cophh_store_sv|const COPHH *cophh|SV *key|U32 hash|SV *value|U32 flags

Like L</cophh_store_pvn>, but takes a Perl scalar instead of a
string/length pair.

=cut
*/

#define cophh_store_sv(cophh, key, hash, value, flags) \
    Perl_refcounted_he_new_sv(aTHX_ cophh, key, hash, value, flags)

/*
=for apidoc Amx|COPHH *|cophh_delete_pvn|COPHH *cophh|const char *keypv|STRLEN keylen|U32 hash|U32 flags

Delete a key and its associated value from the cop hints hash I<cophh>,
and returns the modified hash.  The returned hash pointer is in general
not the same as the hash pointer that was passed in.  The input hash is
consumed by the function, and the pointer to it must not be subsequently
used.  Use L</cophh_copy> if you need both hashes.

The key is specified by I<keypv> and I<keylen>.  If I<flags> has the
C<COPHH_KEY_UTF8> bit set, the key octets are interpreted as UTF-8,
otherwise they are interpreted as Latin-1.  I<hash> is a precomputed
hash of the key string, or zero if it has not been precomputed.

=cut
*/

#define cophh_delete_pvn(cophh, keypv, keylen, hash, flags) \
    Perl_refcounted_he_new_pvn(aTHX_ cophh, keypv, keylen, hash, \
	(SV *)NULL, flags)

/*
=for apidoc Amx|COPHH *|cophh_delete_pvs|const COPHH *cophh|const char *key|U32 flags

Like L</cophh_delete_pvn>, but takes a literal string instead of a
string/length pair, and no precomputed hash.

=cut
*/

#define cophh_delete_pvs(cophh, key, flags) \
    Perl_refcounted_he_new_pvn(aTHX_ cophh, STR_WITH_LEN(key), 0, \
	(SV *)NULL, flags)

/*
=for apidoc Amx|COPHH *|cophh_delete_pv|const COPHH *cophh|const char *key|U32 hash|U32 flags

Like L</cophh_delete_pvn>, but takes a nul-terminated string instead of
a string/length pair.

=cut
*/

#define cophh_delete_pv(cophh, key, hash, flags) \
    Perl_refcounted_he_new_pv(aTHX_ cophh, key, hash, (SV *)NULL, flags)

/*
=for apidoc Amx|COPHH *|cophh_delete_sv|const COPHH *cophh|SV *key|U32 hash|U32 flags

Like L</cophh_delete_pvn>, but takes a Perl scalar instead of a
string/length pair.

=cut
*/

#define cophh_delete_sv(cophh, key, hash, flags) \
    Perl_refcounted_he_new_sv(aTHX_ cophh, key, hash, (SV *)NULL, flags)

#include "mydtrace.h"

struct cop {
    BASEOP
    /* On LP64 putting this here takes advantage of the fact that BASEOP isn't
       an exact multiple of 8 bytes to save structure padding.  */
    line_t      cop_line;       /* line # of this command */
    /* label for this construct is now stored in cop_hints_hash */
#ifdef USE_ITHREADS
    char *	cop_stashpv;	/* package line was compiled in */
    char *	cop_file;	/* file name the following line # is from */
    I32         cop_stashlen;	/* negative for UTF8 */
#else
    HV *	cop_stash;	/* package line was compiled in */
    GV *	cop_filegv;	/* file the following line # is from */
#endif
    U32		cop_hints;	/* hints bits from pragmata */
    U32		cop_seq;	/* parse sequence number */
    /* Beware. mg.c and warnings.pl assume the type of this is STRLEN *:  */
    STRLEN *	cop_warnings;	/* lexical warnings bitmask */
    /* compile time state of %^H.  See the comment in op.c for how this is
       used to recreate a hash to return from caller.  */
    COPHH *	cop_hints_hash;
};

#ifdef USE_ITHREADS
#  define CopFILE(c)		((c)->cop_file)
#  define CopFILEGV(c)		(CopFILE(c) \
				 ? gv_fetchfile(CopFILE(c)) : NULL)
				 
#  ifdef NETWARE
#    define CopFILE_set(c,pv)	((c)->cop_file = savepv(pv))
#    define CopFILE_setn(c,pv,l)  ((c)->cop_file = savepv((pv),(l)))
#  else
#    define CopFILE_set(c,pv)	((c)->cop_file = savesharedpv(pv))
#    define CopFILE_setn(c,pv,l)  ((c)->cop_file = savesharedpvn((pv),(l)))
#  endif

#  define CopFILESV(c)		(CopFILE(c) \
				 ? GvSV(gv_fetchfile(CopFILE(c))) : NULL)
#  define CopFILEAV(c)		(CopFILE(c) \
				 ? GvAV(gv_fetchfile(CopFILE(c))) : NULL)
#  ifdef DEBUGGING
#    define CopFILEAVx(c)	(assert(CopFILE(c)), \
				   GvAV(gv_fetchfile(CopFILE(c))))
#  else
#    define CopFILEAVx(c)	(GvAV(gv_fetchfile(CopFILE(c))))
#  endif
#  define CopSTASHPV(c)		((c)->cop_stashpv)

#  ifdef NETWARE
#    define CopSTASHPV_set(c,pv,n)	((c)->cop_stashpv = \
					   ((pv) ? savepvn(pv,n) : NULL))
#  else
#    define CopSTASHPV_set(c,pv,n)	((c)->cop_stashpv = (pv) \
					    ? savesharedpvn(pv,n) : NULL)
#  endif

#  define CopSTASH_len_set(c,n)	((c)->cop_stashlen = (n))
#  define CopSTASH_len(c)	((c)->cop_stashlen)

#  define CopSTASH(c)          (CopSTASHPV(c)                                 \
                                ? gv_stashpvn(CopSTASHPV(c),		  \
				    CopSTASH_len(c) < 0			  \
					? -CopSTASH_len(c)		  \
					:  CopSTASH_len(c),		  \
                                    GV_ADD|SVf_UTF8*(CopSTASH_len(c) < 0) \
                                  )					  \
                                 : NULL)
#  define CopSTASH_set(c,hv)   (CopSTASHPV_set(c,			\
				    (hv) ? HvNAME_get(hv) : NULL,	\
				    (hv) ? HvNAMELEN(hv)  : 0),		\
				CopSTASH_len_set(c,			\
				    (hv) ? HvNAMEUTF8(hv)		\
					    ? -HvNAMELEN(hv)		\
					    :  HvNAMELEN(hv)		\
					 : 0))
#  define CopSTASH_eq(c,hv)	((hv) && stashpv_hvname_match(c,hv))
#  ifdef NETWARE
#    define CopSTASH_free(c) SAVECOPSTASH_FREE(c)
#    define CopFILE_free(c) SAVECOPFILE_FREE(c)
#  else
#    define CopSTASH_free(c)	PerlMemShared_free(CopSTASHPV(c))
#    define CopFILE_free(c)	(PerlMemShared_free(CopFILE(c)),(CopFILE(c) = NULL))
#  endif
#else
#  define CopFILEGV(c)		((c)->cop_filegv)
#  define CopFILEGV_set(c,gv)	((c)->cop_filegv = (GV*)SvREFCNT_inc(gv))
#  define CopFILE_set(c,pv)	CopFILEGV_set((c), gv_fetchfile(pv))
#  define CopFILE_setn(c,pv,l)	CopFILEGV_set((c), gv_fetchfile_flags((pv),(l),0))
#  define CopFILESV(c)		(CopFILEGV(c) ? GvSV(CopFILEGV(c)) : NULL)
#  define CopFILEAV(c)		(CopFILEGV(c) ? GvAV(CopFILEGV(c)) : NULL)
#  ifdef DEBUGGING
#    define CopFILEAVx(c)	(assert(CopFILEGV(c)), GvAV(CopFILEGV(c)))
#  else
#    define CopFILEAVx(c)	(GvAV(CopFILEGV(c)))
# endif
#  define CopFILE(c)		(CopFILEGV(c) && GvSV(CopFILEGV(c)) \
				    ? SvPVX(GvSV(CopFILEGV(c))) : NULL)
#  define CopSTASH(c)		((c)->cop_stash)
#  define CopSTASH_set(c,hv)	((c)->cop_stash = (hv))
#  define CopSTASHPV(c)		(CopSTASH(c) ? HvNAME_get(CopSTASH(c)) : NULL)
   /* cop_stash is not refcounted */
#  define CopSTASHPV_set(c,pv)	CopSTASH_set((c), gv_stashpv(pv,GV_ADD))
#  define CopSTASH_eq(c,hv)	(CopSTASH(c) == (hv))
#  define CopSTASH_free(c)	
#  define CopFILE_free(c)	(SvREFCNT_dec(CopFILEGV(c)),(CopFILEGV(c) = NULL))

#endif /* USE_ITHREADS */

#define CopHINTHASH_get(c)	((COPHH*)((c)->cop_hints_hash))
#define CopHINTHASH_set(c,h)	((c)->cop_hints_hash = (h))

/*
=head1 COP Hint Reading
*/

/*
=for apidoc Am|SV *|cop_hints_fetch_pvn|const COP *cop|const char *keypv|STRLEN keylen|U32 hash|U32 flags

Look up the hint entry in the cop I<cop> with the key specified by
I<keypv> and I<keylen>.  If I<flags> has the C<COPHH_KEY_UTF8> bit set,
the key octets are interpreted as UTF-8, otherwise they are interpreted
as Latin-1.  I<hash> is a precomputed hash of the key string, or zero if
it has not been precomputed.  Returns a mortal scalar copy of the value
associated with the key, or C<&PL_sv_placeholder> if there is no value
associated with the key.

=cut
*/

#define cop_hints_fetch_pvn(cop, keypv, keylen, hash, flags) \
    cophh_fetch_pvn(CopHINTHASH_get(cop), keypv, keylen, hash, flags)

/*
=for apidoc Am|SV *|cop_hints_fetch_pvs|const COP *cop|const char *key|U32 flags

Like L</cop_hints_fetch_pvn>, but takes a literal string instead of a
string/length pair, and no precomputed hash.

=cut
*/

#define cop_hints_fetch_pvs(cop, key, flags) \
    cophh_fetch_pvs(CopHINTHASH_get(cop), key, flags)

/*
=for apidoc Am|SV *|cop_hints_fetch_pv|const COP *cop|const char *key|U32 hash|U32 flags

Like L</cop_hints_fetch_pvn>, but takes a nul-terminated string instead
of a string/length pair.

=cut
*/

#define cop_hints_fetch_pv(cop, key, hash, flags) \
    cophh_fetch_pv(CopHINTHASH_get(cop), key, hash, flags)

/*
=for apidoc Am|SV *|cop_hints_fetch_sv|const COP *cop|SV *key|U32 hash|U32 flags

Like L</cop_hints_fetch_pvn>, but takes a Perl scalar instead of a
string/length pair.

=cut
*/

#define cop_hints_fetch_sv(cop, key, hash, flags) \
    cophh_fetch_sv(CopHINTHASH_get(cop), key, hash, flags)

/*
=for apidoc Am|HV *|cop_hints_2hv|const COP *cop|U32 flags

Generates and returns a standard Perl hash representing the full set of
hint entries in the cop I<cop>.  I<flags> is currently unused and must
be zero.

=cut
*/

#define cop_hints_2hv(cop, flags) \
    cophh_2hv(CopHINTHASH_get(cop), flags)

#define CopLABEL(c)  Perl_cop_fetch_label(aTHX_ (c), NULL, NULL)
#define CopLABEL_len(c,len)  Perl_cop_fetch_label(aTHX_ (c), len, NULL)
#define CopLABEL_len_flags(c,len,flags)  Perl_cop_fetch_label(aTHX_ (c), len, flags)
#define CopLABEL_alloc(pv)	((pv)?savepv(pv):NULL)

#define CopSTASH_ne(c,hv)	(!CopSTASH_eq(c,hv))
#define CopLINE(c)		((c)->cop_line)
#define CopLINE_inc(c)		(++CopLINE(c))
#define CopLINE_dec(c)		(--CopLINE(c))
#define CopLINE_set(c,l)	(CopLINE(c) = (l))

/* OutCopFILE() is CopFILE for output (caller, die, warn, etc.) */
#define OutCopFILE(c) CopFILE(c)

/* FIXME NATIVE_HINTS if this is changed from op_private (see perl.h)  */
#define CopHINTS_get(c)		((c)->cop_hints + 0)
#define CopHINTS_set(c, h)	STMT_START {				\
				    (c)->cop_hints = (h);		\
				} STMT_END

/*
 * Here we have some enormously heavy (or at least ponderous) wizardry.
 */

/* subroutine context */
struct block_sub {
    OP *	retop;	/* op to execute on exit from sub */
    /* Above here is the same for sub, format and eval.  */
    CV *	cv;
    /* Above here is the same for sub and format.  */
    AV *	savearray;
    AV *	argarray;
    I32		olddepth;
    PAD		*oldcomppad;
};


/* format context */
struct block_format {
    OP *	retop;	/* op to execute on exit from sub */
    /* Above here is the same for sub, format and eval.  */
    CV *	cv;
    /* Above here is the same for sub and format.  */
    GV *	gv;
    GV *	dfoutgv;
};

/* base for the next two macros. Don't use directly.
 * Note that the refcnt of the cv is incremented twice;  The CX one is
 * decremented by LEAVESUB, the other by LEAVE. */

#define PUSHSUB_BASE(cx)						\
	ENTRY_PROBE(GvENAME(CvGV(cv)),		       			\
		CopFILE((const COP *)CvSTART(cv)),			\
		CopLINE((const COP *)CvSTART(cv)),			\
		CopSTASHPV((const COP *)CvSTART(cv)));			\
									\
	cx->blk_sub.cv = cv;						\
	cx->blk_sub.olddepth = CvDEPTH(cv);				\
	cx->cx_type |= (hasargs) ? CXp_HASARGS : 0;			\
	cx->blk_sub.retop = NULL;					\
	if (!CvDEPTH(cv)) {						\
	    SvREFCNT_inc_simple_void_NN(cv);				\
	    SvREFCNT_inc_simple_void_NN(cv);				\
	    SAVEFREESV(cv);						\
	}


#define PUSHSUB(cx)							\
    {									\
	/* If the context is indeterminate, then only the lvalue */	\
	/* flags that the caller also has are applicable.        */	\
	U8 phlags =							\
	   (PL_op->op_flags & OPf_WANT)					\
	       ? OPpENTERSUB_LVAL_MASK					\
	       : !(PL_op->op_private & OPpENTERSUB_LVAL_MASK)		\
	           ? 0 : Perl_was_lvalue_sub(aTHX);			\
	PUSHSUB_BASE(cx)						\
	cx->blk_u16 = PL_op->op_private &				\
	                  (phlags|OPpDEREF);				\
    }

/* variant for use by OP_DBSTATE, where op_private holds hint bits */
#define PUSHSUB_DB(cx)							\
	PUSHSUB_BASE(cx)						\
	cx->blk_u16 = 0;


#define PUSHFORMAT(cx, retop)						\
	cx->blk_format.cv = cv;						\
	cx->blk_format.gv = gv;						\
	cx->blk_format.retop = (retop);					\
	cx->blk_format.dfoutgv = PL_defoutgv;				\
	SvREFCNT_inc_void(cx->blk_format.dfoutgv)

#define POP_SAVEARRAY()						\
    STMT_START {							\
	SvREFCNT_dec(GvAV(PL_defgv));					\
	GvAV(PL_defgv) = cx->blk_sub.savearray;				\
    } STMT_END

/* junk in @_ spells trouble when cloning CVs and in pp_caller(), so don't
 * leave any (a fast av_clear(ary), basically) */
#define CLEAR_ARGARRAY(ary) \
    STMT_START {							\
	AvMAX(ary) += AvARRAY(ary) - AvALLOC(ary);			\
	AvARRAY(ary) = AvALLOC(ary);					\
	AvFILLp(ary) = -1;						\
    } STMT_END

#define POPSUB(cx,sv)							\
    STMT_START {							\
	RETURN_PROBE(GvENAME(CvGV((const CV*)cx->blk_sub.cv)),		\
		CopFILE((COP*)CvSTART((const CV*)cx->blk_sub.cv)),	\
		CopLINE((COP*)CvSTART((const CV*)cx->blk_sub.cv)),	\
		CopSTASHPV((COP*)CvSTART((const CV*)cx->blk_sub.cv)));	\
									\
	if (CxHASARGS(cx)) {						\
	    POP_SAVEARRAY();						\
	    /* abandon @_ if it got reified */				\
	    if (AvREAL(cx->blk_sub.argarray)) {				\
		const SSize_t fill = AvFILLp(cx->blk_sub.argarray);	\
		SvREFCNT_dec(cx->blk_sub.argarray);			\
		cx->blk_sub.argarray = newAV();				\
		av_extend(cx->blk_sub.argarray, fill);			\
		AvREIFY_only(cx->blk_sub.argarray);			\
		CX_CURPAD_SV(cx->blk_sub, 0) = MUTABLE_SV(cx->blk_sub.argarray); \
	    }								\
	    else {							\
		CLEAR_ARGARRAY(cx->blk_sub.argarray);			\
	    }								\
	}								\
	sv = MUTABLE_SV(cx->blk_sub.cv);				\
	if (sv && (CvDEPTH((const CV*)sv) = cx->blk_sub.olddepth))	\
	    sv = NULL;						\
    } STMT_END

#define LEAVESUB(sv)							\
    STMT_START {							\
	if (sv)								\
	    SvREFCNT_dec(sv);						\
    } STMT_END

#define POPFORMAT(cx)							\
	setdefout(cx->blk_format.dfoutgv);				\
	SvREFCNT_dec(cx->blk_format.dfoutgv);

/* eval context */
struct block_eval {
    OP *	retop;	/* op to execute on exit from eval */
    /* Above here is the same for sub, format and eval.  */
    SV *	old_namesv;
    OP *	old_eval_root;
    SV *	cur_text;
    CV *	cv;
    JMPENV *	cur_top_env; /* value of PL_top_env when eval CX created */
};

/* If we ever need more than 512 op types, change the shift from 7.
   blku_gimme is actually also only 2 bits, so could be merged with something.
*/

#define CxOLD_IN_EVAL(cx)	(((cx)->blk_u16) & 0x7F)
#define CxOLD_OP_TYPE(cx)	(((cx)->blk_u16) >> 7)

#define PUSHEVAL(cx,n)							\
    STMT_START {							\
	assert(!(PL_in_eval & ~0x7F));					\
	assert(!(PL_op->op_type & ~0x1FF));				\
	cx->blk_u16 = (PL_in_eval & 0x7F) | ((U16)PL_op->op_type << 7);	\
	cx->blk_eval.old_namesv = (n ? newSVpv(n,0) : NULL);		\
	cx->blk_eval.old_eval_root = PL_eval_root;			\
	cx->blk_eval.cur_text = PL_parser ? PL_parser->linestr : NULL;	\
	cx->blk_eval.cv = NULL; /* set by doeval(), as applicable */	\
	cx->blk_eval.retop = NULL;					\
	cx->blk_eval.cur_top_env = PL_top_env; 				\
    } STMT_END

#define POPEVAL(cx)							\
    STMT_START {							\
	PL_in_eval = CxOLD_IN_EVAL(cx);					\
	optype = CxOLD_OP_TYPE(cx);					\
	PL_eval_root = cx->blk_eval.old_eval_root;			\
	if (cx->blk_eval.old_namesv)					\
	    sv_2mortal(cx->blk_eval.old_namesv);			\
    } STMT_END

/* loop context */
struct block_loop {
    I32		resetsp;
    LOOP *	my_op;	/* My op, that contains redo, next and last ops.  */
    union {	/* different ways of locating the iteration variable */
	SV      **svp;
	GV      *gv;
	PAD     *oldcomppad; /* only used in ITHREADS */
    } itervar_u;
    union {
	struct { /* valid if type is LOOP_FOR or LOOP_PLAIN (but {NULL,0})*/
	    AV * ary; /* use the stack if this is NULL */
	    IV ix;
	} ary;
	struct { /* valid if type is LOOP_LAZYIV */
	    IV cur;
	    IV end;
	} lazyiv;
	struct { /* valid if type if LOOP_LAZYSV */
	    SV * cur;
	    SV * end; /* maxiumum value (or minimum in reverse) */
	} lazysv;
    } state_u;
};

#ifdef USE_ITHREADS
#  define CxITERVAR_PADSV(c) \
	&CX_CURPAD_SV( (c)->blk_loop.itervar_u, (c)->blk_loop.my_op->op_targ)
#else
#  define CxITERVAR_PADSV(c) ((c)->blk_loop.itervar_u.svp)
#endif

#define CxITERVAR(c)							\
	((c)->blk_loop.itervar_u.oldcomppad				\
	 ? (CxPADLOOP(c) 						\
	    ? CxITERVAR_PADSV(c)					\
	    : &GvSV((c)->blk_loop.itervar_u.gv))			\
	 : (SV**)NULL)

#define CxLABEL(c)	(0 + CopLABEL((c)->blk_oldcop))
#define CxLABEL_len(c,len)	(0 + CopLABEL_len((c)->blk_oldcop, len))
#define CxLABEL_len_flags(c,len,flags)	(0 + CopLABEL_len_flags((c)->blk_oldcop, len, flags))
#define CxHASARGS(c)	(((c)->cx_type & CXp_HASARGS) == CXp_HASARGS)
#define CxLVAL(c)	(0 + (c)->blk_u16)

#define PUSHLOOP_PLAIN(cx, s)						\
	cx->blk_loop.resetsp = s - PL_stack_base;			\
	cx->blk_loop.my_op = cLOOP;					\
	cx->blk_loop.state_u.ary.ary = NULL;				\
	cx->blk_loop.state_u.ary.ix = 0;				\
	cx->blk_loop.itervar_u.svp = NULL;

#define PUSHLOOP_FOR(cx, ivar, s)					\
	cx->blk_loop.resetsp = s - PL_stack_base;			\
	cx->blk_loop.my_op = cLOOP;					\
	cx->blk_loop.state_u.ary.ary = NULL;				\
	cx->blk_loop.state_u.ary.ix = 0;				\
	cx->blk_loop.itervar_u.svp = (SV**)(ivar);

#define POPLOOP(cx)							\
	if (CxTYPE(cx) == CXt_LOOP_LAZYSV) {				\
	    SvREFCNT_dec(cx->blk_loop.state_u.lazysv.cur);		\
	    SvREFCNT_dec(cx->blk_loop.state_u.lazysv.end);		\
	}								\
	if (CxTYPE(cx) == CXt_LOOP_FOR)					\
	    SvREFCNT_dec(cx->blk_loop.state_u.ary.ary);

/* given/when context */
struct block_givwhen {
	OP *leave_op;
};

#define PUSHGIVEN(cx)							\
	cx->blk_givwhen.leave_op = cLOGOP->op_other;

#define PUSHWHEN PUSHGIVEN

/* context common to subroutines, evals and loops */
struct block {
    U8		blku_type;	/* what kind of context this is */
    U8		blku_gimme;	/* is this block running in list context? */
    U16		blku_u16;	/* used by block_sub and block_eval (so far) */
    I32		blku_oldsp;	/* stack pointer to copy stuff down to */
    COP *	blku_oldcop;	/* old curcop pointer */
    I32		blku_oldmarksp;	/* mark stack index */
    I32		blku_oldscopesp;	/* scope stack index */
    PMOP *	blku_oldpm;	/* values of pattern match vars */

    union {
	struct block_sub	blku_sub;
	struct block_format	blku_format;
	struct block_eval	blku_eval;
	struct block_loop	blku_loop;
	struct block_givwhen	blku_givwhen;
    } blk_u;
};
#define blk_oldsp	cx_u.cx_blk.blku_oldsp
#define blk_oldcop	cx_u.cx_blk.blku_oldcop
#define blk_oldmarksp	cx_u.cx_blk.blku_oldmarksp
#define blk_oldscopesp	cx_u.cx_blk.blku_oldscopesp
#define blk_oldpm	cx_u.cx_blk.blku_oldpm
#define blk_gimme	cx_u.cx_blk.blku_gimme
#define blk_u16		cx_u.cx_blk.blku_u16
#define blk_sub		cx_u.cx_blk.blk_u.blku_sub
#define blk_format	cx_u.cx_blk.blk_u.blku_format
#define blk_eval	cx_u.cx_blk.blk_u.blku_eval
#define blk_loop	cx_u.cx_blk.blk_u.blku_loop
#define blk_givwhen	cx_u.cx_blk.blk_u.blku_givwhen

#define DEBUG_CX(action)						\
    DEBUG_l(								\
	Perl_deb(aTHX_ "CX %ld %s %s (scope %ld,%ld) at %s:%d\n",	\
		    (long)cxstack_ix,					\
		    action,						\
		    PL_block_type[CxTYPE(&cxstack[cxstack_ix])],	\
		    (long)PL_scopestack_ix,				\
		    (long)(cxstack[cxstack_ix].blk_oldscopesp),		\
		    __FILE__, __LINE__));

/* Enter a block. */
#define PUSHBLOCK(cx,t,sp) CXINC, cx = &cxstack[cxstack_ix],		\
	cx->cx_type		= t,					\
	cx->blk_oldsp		= sp - PL_stack_base,			\
	cx->blk_oldcop		= PL_curcop,				\
	cx->blk_oldmarksp	= PL_markstack_ptr - PL_markstack,	\
	cx->blk_oldscopesp	= PL_scopestack_ix,			\
	cx->blk_oldpm		= PL_curpm,				\
	cx->blk_gimme		= (U8)gimme;				\
	DEBUG_CX("PUSH");

/* Exit a block (RETURN and LAST). */
#define POPBLOCK(cx,pm)							\
	DEBUG_CX("POP");						\
	cx = &cxstack[cxstack_ix--],					\
	newsp		 = PL_stack_base + cx->blk_oldsp,		\
	PL_curcop	 = cx->blk_oldcop,				\
	PL_markstack_ptr = PL_markstack + cx->blk_oldmarksp,		\
	PL_scopestack_ix = cx->blk_oldscopesp,				\
	pm		 = cx->blk_oldpm,				\
	gimme		 = cx->blk_gimme;

/* Continue a block elsewhere (NEXT and REDO). */
#define TOPBLOCK(cx)							\
	DEBUG_CX("TOP");						\
	cx  = &cxstack[cxstack_ix],					\
	PL_stack_sp	 = PL_stack_base + cx->blk_oldsp,		\
	PL_markstack_ptr = PL_markstack + cx->blk_oldmarksp,		\
	PL_scopestack_ix = cx->blk_oldscopesp,				\
	PL_curpm         = cx->blk_oldpm;

/* substitution context */
struct subst {
    U8		sbu_type;	/* what kind of context this is */
    U8		sbu_rflags;
    U16		sbu_rxtainted;	/* matches struct block */
    I32		sbu_iters;
    I32		sbu_maxiters;
    I32		sbu_oldsave;
    char *	sbu_orig;
    SV *	sbu_dstr;
    SV *	sbu_targ;
    char *	sbu_s;
    char *	sbu_m;
    char *	sbu_strend;
    void *	sbu_rxres;
    REGEXP *	sbu_rx;
};
#define sb_iters	cx_u.cx_subst.sbu_iters
#define sb_maxiters	cx_u.cx_subst.sbu_maxiters
#define sb_rflags	cx_u.cx_subst.sbu_rflags
#define sb_oldsave	cx_u.cx_subst.sbu_oldsave
#define sb_rxtainted	cx_u.cx_subst.sbu_rxtainted
#define sb_orig		cx_u.cx_subst.sbu_orig
#define sb_dstr		cx_u.cx_subst.sbu_dstr
#define sb_targ		cx_u.cx_subst.sbu_targ
#define sb_s		cx_u.cx_subst.sbu_s
#define sb_m		cx_u.cx_subst.sbu_m
#define sb_strend	cx_u.cx_subst.sbu_strend
#define sb_rxres	cx_u.cx_subst.sbu_rxres
#define sb_rx		cx_u.cx_subst.sbu_rx

#ifdef PERL_CORE
#  define PUSHSUBST(cx) CXINC, cx = &cxstack[cxstack_ix],		\
	cx->sb_iters		= iters,				\
	cx->sb_maxiters		= maxiters,				\
	cx->sb_rflags		= r_flags,				\
	cx->sb_oldsave		= oldsave,				\
	cx->sb_rxtainted	= rxtainted,				\
	cx->sb_orig		= orig,					\
	cx->sb_dstr		= dstr,					\
	cx->sb_targ		= targ,					\
	cx->sb_s		= s,					\
	cx->sb_m		= m,					\
	cx->sb_strend		= strend,				\
	cx->sb_rxres		= NULL,					\
	cx->sb_rx		= rx,					\
	cx->cx_type		= CXt_SUBST | (once ? CXp_ONCE : 0);	\
	rxres_save(&cx->sb_rxres, rx);					\
	(void)ReREFCNT_inc(rx)

#  define POPSUBST(cx) cx = &cxstack[cxstack_ix--];			\
	rxres_free(&cx->sb_rxres);					\
	ReREFCNT_dec(cx->sb_rx)
#endif

#define CxONCE(cx)		((cx)->cx_type & CXp_ONCE)

struct context {
    union {
	struct block	cx_blk;
	struct subst	cx_subst;
    } cx_u;
};
#define cx_type cx_u.cx_subst.sbu_type

/* If you re-order these, there is also an array of uppercase names in perl.h
   and a static array of context names in pp_ctl.c  */
#define CXTYPEMASK	0xf
#define CXt_NULL	0
#define CXt_WHEN	1
#define CXt_BLOCK	2
/* When micro-optimising :-) keep GIVEN next to the LOOPs, as these 5 share a
   jump table in pp_ctl.c
   The first 4 don't have a 'case' in at least one switch statement in pp_ctl.c
*/
#define CXt_GIVEN	3
/* This is first so that CXt_LOOP_FOR|CXt_LOOP_LAZYIV is CXt_LOOP_LAZYIV */
#define CXt_LOOP_FOR	4
#define CXt_LOOP_PLAIN	5
#define CXt_LOOP_LAZYSV	6
#define CXt_LOOP_LAZYIV	7
#define CXt_SUB		8
#define CXt_FORMAT      9
#define CXt_EVAL       10
#define CXt_SUBST      11
/* SUBST doesn't feature in all switch statements.  */

/* private flags for CXt_SUB and CXt_NULL
   However, this is checked in many places which do not check the type, so
   this bit needs to be kept clear for most everything else. For reasons I
   haven't investigated, it can coexist with CXp_FOR_DEF */
#define CXp_MULTICALL	0x10	/* part of a multicall (so don't
				   tear down context on exit). */ 

/* private flags for CXt_SUB and CXt_FORMAT */
#define CXp_HASARGS	0x20

/* private flags for CXt_EVAL */
#define CXp_REAL	0x20	/* truly eval'', not a lookalike */
#define CXp_TRYBLOCK	0x40	/* eval{}, not eval'' or similar */

/* private flags for CXt_LOOP */
#define CXp_FOR_DEF	0x10	/* foreach using $_ */
#define CxPADLOOP(c)	((c)->blk_loop.my_op->op_targ)

/* private flags for CXt_SUBST */
#define CXp_ONCE	0x10	/* What was sbu_once in struct subst */

#define CxTYPE(c)	((c)->cx_type & CXTYPEMASK)
#define CxTYPE_is_LOOP(c)	(((c)->cx_type & 0xC) == 0x4)
#define CxMULTICALL(c)	(((c)->cx_type & CXp_MULTICALL)			\
			 == CXp_MULTICALL)
#define CxREALEVAL(c)	(((c)->cx_type & (CXTYPEMASK|CXp_REAL))		\
			 == (CXt_EVAL|CXp_REAL))
#define CxTRYBLOCK(c)	(((c)->cx_type & (CXTYPEMASK|CXp_TRYBLOCK))	\
			 == (CXt_EVAL|CXp_TRYBLOCK))
#define CxFOREACH(c)	(CxTYPE_is_LOOP(c) && CxTYPE(c) != CXt_LOOP_PLAIN)
#define CxFOREACHDEF(c)	((CxTYPE_is_LOOP(c) && CxTYPE(c) != CXt_LOOP_PLAIN) \
			 && ((c)->cx_type & CXp_FOR_DEF))

#define CXINC (cxstack_ix < cxstack_max ? ++cxstack_ix : (cxstack_ix = cxinc()))

/* 
=head1 "Gimme" Values
*/

/*
=for apidoc AmU||G_SCALAR
Used to indicate scalar context.  See C<GIMME_V>, C<GIMME>, and
L<perlcall>.

=for apidoc AmU||G_ARRAY
Used to indicate list context.  See C<GIMME_V>, C<GIMME> and
L<perlcall>.

=for apidoc AmU||G_VOID
Used to indicate void context.  See C<GIMME_V> and L<perlcall>.

=for apidoc AmU||G_DISCARD
Indicates that arguments returned from a callback should be discarded.  See
L<perlcall>.

=for apidoc AmU||G_EVAL

Used to force a Perl C<eval> wrapper around a callback.  See
L<perlcall>.

=for apidoc AmU||G_NOARGS

Indicates that no arguments are being sent to a callback.  See
L<perlcall>.

=cut
*/

#define G_SCALAR	2
#define G_ARRAY		3
#define G_VOID		1
#define G_WANT		3

/* extra flags for Perl_call_* routines */
#define G_DISCARD	4	/* Call FREETMPS.
				   Don't change this without consulting the
				   hash actions codes defined in hv.h */
#define G_EVAL		8	/* Assume eval {} around subroutine call. */
#define G_NOARGS       16	/* Don't construct a @_ array. */
#define G_KEEPERR      32	/* Warn for errors, don't overwrite $@ */
#define G_NODEBUG      64	/* Disable debugging at toplevel.  */
#define G_METHOD      128       /* Calling method. */
#define G_FAKINGEVAL  256	/* Faking an eval context for call_sv or
				   fold_constants. */
#define G_UNDEF_FILL  512	/* Fill the stack with &PL_sv_undef
				   A special case for UNSHIFT in
				   Perl_magic_methcall().  */
#define G_WRITING_TO_STDERR 1024 /* Perl_write_to_stderr() is calling
				    Perl_magic_methcall().  */

/* flag bits for PL_in_eval */
#define EVAL_NULL	0	/* not in an eval */
#define EVAL_INEVAL	1	/* some enclosing scope is an eval */
#define EVAL_WARNONLY	2	/* used by yywarn() when calling yyerror() */
#define EVAL_KEEPERR	4	/* set by Perl_call_sv if G_KEEPERR */
#define EVAL_INREQUIRE	8	/* The code is being required. */

/* Support for switching (stack and block) contexts.
 * This ensures magic doesn't invalidate local stack and cx pointers.
 */

#define PERLSI_UNKNOWN		-1
#define PERLSI_UNDEF		0
#define PERLSI_MAIN		1
#define PERLSI_MAGIC		2
#define PERLSI_SORT		3
#define PERLSI_SIGNAL		4
#define PERLSI_OVERLOAD		5
#define PERLSI_DESTROY		6
#define PERLSI_WARNHOOK		7
#define PERLSI_DIEHOOK		8
#define PERLSI_REQUIRE		9

struct stackinfo {
    AV *		si_stack;	/* stack for current runlevel */
    PERL_CONTEXT *	si_cxstack;	/* context stack for runlevel */
    struct stackinfo *	si_prev;
    struct stackinfo *	si_next;
    I32			si_cxix;	/* current context index */
    I32			si_cxmax;	/* maximum allocated index */
    I32			si_type;	/* type of runlevel */
    I32			si_markoff;	/* offset where markstack begins for us.
					 * currently used only with DEBUGGING,
					 * but not #ifdef-ed for bincompat */
};

typedef struct stackinfo PERL_SI;

#define cxstack		(PL_curstackinfo->si_cxstack)
#define cxstack_ix	(PL_curstackinfo->si_cxix)
#define cxstack_max	(PL_curstackinfo->si_cxmax)

#ifdef DEBUGGING
#  define	SET_MARK_OFFSET \
    PL_curstackinfo->si_markoff = PL_markstack_ptr - PL_markstack
#else
#  define	SET_MARK_OFFSET NOOP
#endif

#define PUSHSTACKi(type) \
    STMT_START {							\
	PERL_SI *next = PL_curstackinfo->si_next;			\
	DEBUG_l({							\
	    int i = 0; PERL_SI *p = PL_curstackinfo;			\
	    while (p) { i++; p = p->si_prev; }				\
	    Perl_deb(aTHX_ "push STACKINFO %d at %s:%d\n",		\
		         i, __FILE__, __LINE__);})			\
	if (!next) {							\
	    next = new_stackinfo(32, 2048/sizeof(PERL_CONTEXT) - 1);	\
	    next->si_prev = PL_curstackinfo;				\
	    PL_curstackinfo->si_next = next;				\
	}								\
	next->si_type = type;						\
	next->si_cxix = -1;						\
	AvFILLp(next->si_stack) = 0;					\
	SWITCHSTACK(PL_curstack,next->si_stack);			\
	PL_curstackinfo = next;						\
	SET_MARK_OFFSET;						\
    } STMT_END

#define PUSHSTACK PUSHSTACKi(PERLSI_UNKNOWN)

/* POPSTACK works with PL_stack_sp, so it may need to be bracketed by
 * PUTBACK/SPAGAIN to flush/refresh any local SP that may be active */
#define POPSTACK \
    STMT_START {							\
	dSP;								\
	PERL_SI * const prev = PL_curstackinfo->si_prev;		\
	DEBUG_l({							\
	    int i = -1; PERL_SI *p = PL_curstackinfo;			\
	    while (p) { i++; p = p->si_prev; }				\
	    Perl_deb(aTHX_ "pop  STACKINFO %d at %s:%d\n",		\
		         i, __FILE__, __LINE__);})			\
	if (!prev) {							\
	    PerlIO_printf(Perl_error_log, "panic: POPSTACK\n");		\
	    my_exit(1);							\
	}								\
	SWITCHSTACK(PL_curstack,prev->si_stack);			\
	/* don't free prev here, free them all at the END{} */		\
	PL_curstackinfo = prev;						\
    } STMT_END

#define POPSTACK_TO(s) \
    STMT_START {							\
	while (PL_curstack != s) {					\
	    dounwind(-1);						\
	    POPSTACK;							\
	}								\
    } STMT_END

#define IN_PERL_COMPILETIME	(PL_curcop == &PL_compiling)
#define IN_PERL_RUNTIME		(PL_curcop != &PL_compiling)

/*
=head1 Multicall Functions

=for apidoc Ams||dMULTICALL
Declare local variables for a multicall. See L<perlcall/LIGHTWEIGHT CALLBACKS>.

=for apidoc Ams||PUSH_MULTICALL
Opening bracket for a lightweight callback.
See L<perlcall/LIGHTWEIGHT CALLBACKS>.

=for apidoc Ams||MULTICALL
Make a lightweight callback. See L<perlcall/LIGHTWEIGHT CALLBACKS>.

=for apidoc Ams||POP_MULTICALL
Closing bracket for a lightweight callback.
See L<perlcall/LIGHTWEIGHT CALLBACKS>.

=cut
*/

#define dMULTICALL \
    SV **newsp;			/* set by POPBLOCK */			\
    PERL_CONTEXT *cx;							\
    CV *multicall_cv;							\
    OP *multicall_cop;							\
    bool multicall_oldcatch; 						\
    U8 hasargs = 0		/* used by PUSHSUB */

#define PUSH_MULTICALL(the_cv) \
    STMT_START {							\
	CV * const _nOnclAshIngNamE_ = the_cv;				\
	CV * const cv = _nOnclAshIngNamE_;				\
	AV * const padlist = CvPADLIST(cv);				\
	ENTER;								\
 	multicall_oldcatch = CATCH_GET;					\
	SAVETMPS; SAVEVPTR(PL_op);					\
	CATCH_SET(TRUE);						\
	PUSHSTACKi(PERLSI_SORT);					\
	PUSHBLOCK(cx, CXt_SUB|CXp_MULTICALL, PL_stack_sp);		\
	PUSHSUB(cx);							\
	if (++CvDEPTH(cv) >= 2) {					\
	    PERL_STACK_OVERFLOW_CHECK();				\
	    Perl_pad_push(aTHX_ padlist, CvDEPTH(cv));			\
	}								\
	SAVECOMPPAD();							\
	PAD_SET_CUR_NOSAVE(padlist, CvDEPTH(cv));			\
	multicall_cv = cv;						\
	multicall_cop = CvSTART(cv);					\
    } STMT_END

#define MULTICALL \
    STMT_START {							\
	PL_op = multicall_cop;						\
	CALLRUNOPS(aTHX);						\
    } STMT_END

#define POP_MULTICALL \
    STMT_START {							\
	if (! --CvDEPTH(multicall_cv))					\
	    LEAVESUB(multicall_cv);					\
	POPBLOCK(cx,PL_curpm);						\
	POPSTACK;							\
	CATCH_SET(multicall_oldcatch);					\
	LEAVE;								\
	SPAGAIN;							\
    } STMT_END

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
