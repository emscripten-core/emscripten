/*    handy.h
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1999, 2000,
 *    2001, 2002, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#if !defined(__STDC__)
#ifdef NULL
#undef NULL
#endif
#ifndef I286
#  define NULL 0
#else
#  define NULL 0L
#endif
#endif

#ifndef PERL_CORE
#  define Null(type) ((type)NULL)

/*
=head1 Handy Values

=for apidoc AmU||Nullch
Null character pointer. (No longer available when C<PERL_CORE> is defined.)

=for apidoc AmU||Nullsv
Null SV pointer. (No longer available when C<PERL_CORE> is defined.)

=cut
*/

#  define Nullch Null(char*)
#  define Nullfp Null(PerlIO*)
#  define Nullsv Null(SV*)
#endif

#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
#define TRUE (1)
#define FALSE (0)

/* The MUTABLE_*() macros cast pointers to the types shown, in such a way
 * (compiler permitting) that casting away const-ness will give a warning;
 * e.g.:
 *
 * const SV *sv = ...;
 * AV *av1 = (AV*)sv;        <== BAD:  the const has been silently cast away
 * AV *av2 = MUTABLE_AV(sv); <== GOOD: it may warn
 */

#if defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN)
#  define MUTABLE_PTR(p) ({ void *_p = (p); _p; })
#else
#  define MUTABLE_PTR(p) ((void *) (p))
#endif

#define MUTABLE_AV(p)	((AV *)MUTABLE_PTR(p))
#define MUTABLE_CV(p)	((CV *)MUTABLE_PTR(p))
#define MUTABLE_GV(p)	((GV *)MUTABLE_PTR(p))
#define MUTABLE_HV(p)	((HV *)MUTABLE_PTR(p))
#define MUTABLE_IO(p)	((IO *)MUTABLE_PTR(p))
#define MUTABLE_SV(p)	((SV *)MUTABLE_PTR(p))

#ifdef I_STDBOOL
#  include <stdbool.h>
#  ifndef HAS_BOOL
#    define HAS_BOOL 1
#  endif
#endif

/* bool is built-in for g++-2.6.3 and later, which might be used
   for extensions.  <_G_config.h> defines _G_HAVE_BOOL, but we can't
   be sure _G_config.h will be included before this file.  _G_config.h
   also defines _G_HAVE_BOOL for both gcc and g++, but only g++
   actually has bool.  Hence, _G_HAVE_BOOL is pretty useless for us.
   g++ can be identified by __GNUG__.
   Andy Dougherty	February 2000
*/
#ifdef __GNUG__		/* GNU g++ has bool built-in */
#  ifndef HAS_BOOL
#    define HAS_BOOL 1
#  endif
#endif

/* The NeXT dynamic loader headers will not build with the bool macro
   So declare them now to clear confusion.
*/
#if defined(NeXT) || defined(__NeXT__)
# undef FALSE
# undef TRUE
  typedef enum bool { FALSE = 0, TRUE = 1 } bool;
# define ENUM_BOOL 1
# ifndef HAS_BOOL
#  define HAS_BOOL 1
# endif /* !HAS_BOOL */
#endif /* NeXT || __NeXT__ */

#ifndef HAS_BOOL
# if defined(UTS) || defined(VMS)
#  define bool int
# else
#  define bool char
# endif
# define HAS_BOOL 1
#endif

/* a simple (bool) cast may not do the right thing: if bool is defined
 * as char for example, then the cast from int is implementation-defined
 * (bool)!!(cbool) in a ternary triggers a bug in xlc on AIX
 */

#define cBOOL(cbool) ((cbool) ? (bool)1 : (bool)0)

/* Try to figure out __func__ or __FUNCTION__ equivalent, if any.
 * XXX Should really be a Configure probe, with HAS__FUNCTION__
 *     and FUNCTION__ as results.
 * XXX Similarly, a Configure probe for __FILE__ and __LINE__ is needed. */
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || (defined(__SUNPRO_C)) /* C99 or close enough. */
#  define FUNCTION__ __func__
#else
#  if (defined(_MSC_VER) && _MSC_VER < 1300) || /* Pre-MSVC 7.0 has neither __func__ nor __FUNCTION and no good workarounds, either. */ \
      (defined(__DECC_VER)) /* Tru64 or VMS, and strict C89 being used, but not modern enough cc (in Tur64, -c99 not known, only -std1). */
#    define FUNCTION__ ""
#  else
#    define FUNCTION__ __FUNCTION__ /* Common extension. */
#  endif
#endif

/* XXX A note on the perl source internal type system.  The
   original intent was that I32 be *exactly* 32 bits.

   Currently, we only guarantee that I32 is *at least* 32 bits.
   Specifically, if int is 64 bits, then so is I32.  (This is the case
   for the Cray.)  This has the advantage of meshing nicely with
   standard library calls (where we pass an I32 and the library is
   expecting an int), but the disadvantage that an I32 is not 32 bits.
   Andy Dougherty	August 1996

   There is no guarantee that there is *any* integral type with
   exactly 32 bits.  It is perfectly legal for a system to have
   sizeof(short) == sizeof(int) == sizeof(long) == 8.

   Similarly, there is no guarantee that I16 and U16 have exactly 16
   bits.

   For dealing with issues that may arise from various 32/64-bit
   systems, we will ask Configure to check out

	SHORTSIZE == sizeof(short)
	INTSIZE == sizeof(int)
	LONGSIZE == sizeof(long)
	LONGLONGSIZE == sizeof(long long) (if HAS_LONG_LONG)
	PTRSIZE == sizeof(void *)
	DOUBLESIZE == sizeof(double)
	LONG_DOUBLESIZE == sizeof(long double) (if HAS_LONG_DOUBLE).

*/

#ifdef I_INTTYPES /* e.g. Linux has int64_t without <inttypes.h> */
#   include <inttypes.h>
#   ifdef INT32_MIN_BROKEN
#       undef  INT32_MIN
#       define INT32_MIN (-2147483647-1)
#   endif
#   ifdef INT64_MIN_BROKEN
#       undef  INT64_MIN
#       define INT64_MIN (-9223372036854775807LL-1)
#   endif
#endif

typedef I8TYPE I8;
typedef U8TYPE U8;
typedef I16TYPE I16;
typedef U16TYPE U16;
typedef I32TYPE I32;
typedef U32TYPE U32;
#ifdef PERL_CORE
#   ifdef HAS_QUAD
typedef I64TYPE I64;
typedef U64TYPE U64;
#   endif
#endif /* PERL_CORE */

#if defined(HAS_QUAD) && defined(USE_64_BIT_INT)
#   ifndef UINT64_C /* usually from <inttypes.h> */
#       if defined(HAS_LONG_LONG) && QUADKIND == QUAD_IS_LONG_LONG
#           define INT64_C(c)	CAT2(c,LL)
#           define UINT64_C(c)	CAT2(c,ULL)
#       else
#           if LONGSIZE == 8 && QUADKIND == QUAD_IS_LONG
#               define INT64_C(c)	CAT2(c,L)
#               define UINT64_C(c)	CAT2(c,UL)
#           else
#               if defined(_WIN64) && defined(_MSC_VER)
#                   define INT64_C(c)	CAT2(c,I64)
#                   define UINT64_C(c)	CAT2(c,UI64)
#               else
#                   define INT64_C(c)	((I64TYPE)(c))
#                   define UINT64_C(c)	((U64TYPE)(c))
#               endif
#           endif
#       endif
#   endif
#endif

#if defined(UINT8_MAX) && defined(INT16_MAX) && defined(INT32_MAX)

/* I8_MAX and I8_MIN constants are not defined, as I8 is an ambiguous type.
   Please search CHAR_MAX in perl.h for further details. */
#define U8_MAX UINT8_MAX
#define U8_MIN UINT8_MIN

#define I16_MAX INT16_MAX
#define I16_MIN INT16_MIN
#define U16_MAX UINT16_MAX
#define U16_MIN UINT16_MIN

#define I32_MAX INT32_MAX
#define I32_MIN INT32_MIN
#ifndef UINT32_MAX_BROKEN /* e.g. HP-UX with gcc messes this up */
#  define U32_MAX UINT32_MAX
#else
#  define U32_MAX 4294967295U
#endif
#define U32_MIN UINT32_MIN

#else

/* I8_MAX and I8_MIN constants are not defined, as I8 is an ambiguous type.
   Please search CHAR_MAX in perl.h for further details. */
#define U8_MAX PERL_UCHAR_MAX
#define U8_MIN PERL_UCHAR_MIN

#define I16_MAX PERL_SHORT_MAX
#define I16_MIN PERL_SHORT_MIN
#define U16_MAX PERL_USHORT_MAX
#define U16_MIN PERL_USHORT_MIN

#if LONGSIZE > 4
# define I32_MAX PERL_INT_MAX
# define I32_MIN PERL_INT_MIN
# define U32_MAX PERL_UINT_MAX
# define U32_MIN PERL_UINT_MIN
#else
# define I32_MAX PERL_LONG_MAX
# define I32_MIN PERL_LONG_MIN
# define U32_MAX PERL_ULONG_MAX
# define U32_MIN PERL_ULONG_MIN
#endif

#endif

/* log(2) is pretty close to  0.30103, just in case anyone is grepping for it */
#define BIT_DIGITS(N)   (((N)*146)/485 + 1)  /* log2(10) =~ 146/485 */
#define TYPE_DIGITS(T)  BIT_DIGITS(sizeof(T) * 8)
#define TYPE_CHARS(T)   (TYPE_DIGITS(T) + 2) /* sign, NUL */

#define Ctl(ch) ((ch) & 037)

/*
=head1 SV-Body Allocation

=for apidoc Ama|SV*|newSVpvs|const char* s
Like C<newSVpvn>, but takes a literal string instead of a string/length pair.

=for apidoc Ama|SV*|newSVpvs_flags|const char* s|U32 flags
Like C<newSVpvn_flags>, but takes a literal string instead of a string/length
pair.

=for apidoc Ama|SV*|newSVpvs_share|const char* s
Like C<newSVpvn_share>, but takes a literal string instead of a string/length
pair and omits the hash parameter.

=for apidoc Am|void|sv_catpvs_flags|SV* sv|const char* s|I32 flags
Like C<sv_catpvn_flags>, but takes a literal string instead of a
string/length pair.

=for apidoc Am|void|sv_catpvs_nomg|SV* sv|const char* s
Like C<sv_catpvn_nomg>, but takes a literal string instead of a
string/length pair.

=for apidoc Am|void|sv_catpvs|SV* sv|const char* s
Like C<sv_catpvn>, but takes a literal string instead of a string/length pair.

=for apidoc Am|void|sv_catpvs_mg|SV* sv|const char* s
Like C<sv_catpvn_mg>, but takes a literal string instead of a
string/length pair.

=for apidoc Am|void|sv_setpvs|SV* sv|const char* s
Like C<sv_setpvn>, but takes a literal string instead of a string/length pair.

=for apidoc Am|void|sv_setpvs_mg|SV* sv|const char* s
Like C<sv_setpvn_mg>, but takes a literal string instead of a
string/length pair.

=for apidoc Am|SV *|sv_setref_pvs|const char* s
Like C<sv_setref_pvn>, but takes a literal string instead of a
string/length pair.

=head1 Memory Management

=for apidoc Ama|char*|savepvs|const char* s
Like C<savepvn>, but takes a literal string instead of a string/length pair.

=for apidoc Ama|char*|savesharedpvs|const char* s
A version of C<savepvs()> which allocates the duplicate string in memory
which is shared between threads.

=head1 GV Functions

=for apidoc Am|HV*|gv_stashpvs|const char* name|I32 create
Like C<gv_stashpvn>, but takes a literal string instead of a string/length pair.

=head1 Hash Manipulation Functions

=for apidoc Am|SV**|hv_fetchs|HV* tb|const char* key|I32 lval
Like C<hv_fetch>, but takes a literal string instead of a string/length pair.

=for apidoc Am|SV**|hv_stores|HV* tb|const char* key|NULLOK SV* val
Like C<hv_store>, but takes a literal string instead of a string/length pair
and omits the hash parameter.

=head1 Lexer interface

=for apidoc Amx|void|lex_stuff_pvs|const char *pv|U32 flags

Like L</lex_stuff_pvn>, but takes a literal string instead of a
string/length pair.

=cut
*/

/* concatenating with "" ensures that only literal strings are accepted as argument */
#define STR_WITH_LEN(s)  ("" s ""), (sizeof(s)-1)

/* note that STR_WITH_LEN() can't be used as argument to macros or functions that
 * under some configurations might be macros, which means that it requires the full
 * Perl_xxx(aTHX_ ...) form for any API calls where it's used.
 */

/* STR_WITH_LEN() shortcuts */
#define newSVpvs(str) Perl_newSVpvn(aTHX_ STR_WITH_LEN(str))
#define newSVpvs_flags(str,flags)	\
    Perl_newSVpvn_flags(aTHX_ STR_WITH_LEN(str), flags)
#define newSVpvs_share(str) Perl_newSVpvn_share(aTHX_ STR_WITH_LEN(str), 0)
#define sv_catpvs_flags(sv, str, flags) \
    Perl_sv_catpvn_flags(aTHX_ sv, STR_WITH_LEN(str), flags)
#define sv_catpvs_nomg(sv, str) \
    Perl_sv_catpvn_flags(aTHX_ sv, STR_WITH_LEN(str), 0)
#define sv_catpvs(sv, str) \
    Perl_sv_catpvn_flags(aTHX_ sv, STR_WITH_LEN(str), SV_GMAGIC)
#define sv_catpvs_mg(sv, str) \
    Perl_sv_catpvn_flags(aTHX_ sv, STR_WITH_LEN(str), SV_GMAGIC|SV_SMAGIC)
#define sv_setpvs(sv, str) Perl_sv_setpvn(aTHX_ sv, STR_WITH_LEN(str))
#define sv_setpvs_mg(sv, str) Perl_sv_setpvn_mg(aTHX_ sv, STR_WITH_LEN(str))
#define sv_setref_pvs(rv, classname, str) \
    Perl_sv_setref_pvn(aTHX_ rv, classname, STR_WITH_LEN(str))
#define savepvs(str) Perl_savepvn(aTHX_ STR_WITH_LEN(str))
#define savesharedpvs(str) Perl_savesharedpvn(aTHX_ STR_WITH_LEN(str))
#define gv_stashpvs(str, create) \
    Perl_gv_stashpvn(aTHX_ STR_WITH_LEN(str), create)
#define gv_fetchpvs(namebeg, add, sv_type) \
    Perl_gv_fetchpvn_flags(aTHX_ STR_WITH_LEN(namebeg), add, sv_type)
#define gv_fetchpvn(namebeg, len, add, sv_type) \
    Perl_gv_fetchpvn_flags(aTHX_ namebeg, len, add, sv_type)
#define sv_catxmlpvs(dsv, str, utf8) \
    Perl_sv_catxmlpvn(aTHX_ dsv, STR_WITH_LEN(str), utf8)
#define hv_fetchs(hv,key,lval)						\
  ((SV **)Perl_hv_common(aTHX_ (hv), NULL, STR_WITH_LEN(key), 0,	\
			 (lval) ? (HV_FETCH_JUST_SV | HV_FETCH_LVALUE)	\
			 : HV_FETCH_JUST_SV, NULL, 0))

#define hv_stores(hv,key,val)						\
  ((SV **)Perl_hv_common(aTHX_ (hv), NULL, STR_WITH_LEN(key), 0,	\
			 (HV_FETCH_ISSTORE|HV_FETCH_JUST_SV), (val), 0))

#define lex_stuff_pvs(pv,flags) Perl_lex_stuff_pvn(aTHX_ STR_WITH_LEN(pv), flags)

#define get_cvs(str, flags)					\
	Perl_get_cvn_flags(aTHX_ STR_WITH_LEN(str), (flags))

/*
=head1 Miscellaneous Functions

=for apidoc Am|bool|strNE|char* s1|char* s2
Test two strings to see if they are different.  Returns true or
false.

=for apidoc Am|bool|strEQ|char* s1|char* s2
Test two strings to see if they are equal.  Returns true or false.

=for apidoc Am|bool|strLT|char* s1|char* s2
Test two strings to see if the first, C<s1>, is less than the second,
C<s2>.  Returns true or false.

=for apidoc Am|bool|strLE|char* s1|char* s2
Test two strings to see if the first, C<s1>, is less than or equal to the
second, C<s2>.  Returns true or false.

=for apidoc Am|bool|strGT|char* s1|char* s2
Test two strings to see if the first, C<s1>, is greater than the second,
C<s2>.  Returns true or false.

=for apidoc Am|bool|strGE|char* s1|char* s2
Test two strings to see if the first, C<s1>, is greater than or equal to
the second, C<s2>.  Returns true or false.

=for apidoc Am|bool|strnNE|char* s1|char* s2|STRLEN len
Test two strings to see if they are different.  The C<len> parameter
indicates the number of bytes to compare.  Returns true or false. (A
wrapper for C<strncmp>).

=for apidoc Am|bool|strnEQ|char* s1|char* s2|STRLEN len
Test two strings to see if they are equal.  The C<len> parameter indicates
the number of bytes to compare.  Returns true or false. (A wrapper for
C<strncmp>).

=cut
*/

#define strNE(s1,s2) (strcmp(s1,s2))
#define strEQ(s1,s2) (!strcmp(s1,s2))
#define strLT(s1,s2) (strcmp(s1,s2) < 0)
#define strLE(s1,s2) (strcmp(s1,s2) <= 0)
#define strGT(s1,s2) (strcmp(s1,s2) > 0)
#define strGE(s1,s2) (strcmp(s1,s2) >= 0)
#define strnNE(s1,s2,l) (strncmp(s1,s2,l))
#define strnEQ(s1,s2,l) (!strncmp(s1,s2,l))

#ifdef HAS_MEMCMP
#  define memNE(s1,s2,l) (memcmp(s1,s2,l))
#  define memEQ(s1,s2,l) (!memcmp(s1,s2,l))
#else
#  define memNE(s1,s2,l) (bcmp(s1,s2,l))
#  define memEQ(s1,s2,l) (!bcmp(s1,s2,l))
#endif

#define memEQs(s1, l, s2) \
	(sizeof(s2)-1 == l && memEQ(s1, ("" s2 ""), (sizeof(s2)-1)))
#define memNEs(s1, l, s2) !memEQs(s1, l, s2)

/*
 * Character classes.
 *
 * Unfortunately, the introduction of locales means that we
 * can't trust isupper(), etc. to tell the truth.  And when
 * it comes to /\w+/ with tainting enabled, we *must* be able
 * to trust our character classes.
 *
 * Therefore, the default tests in the text of Perl will be
 * independent of locale.  Any code that wants to depend on
 * the current locale will use the tests that begin with "lc".
 */

#ifdef HAS_SETLOCALE  /* XXX Is there a better test for this? */
#  ifndef CTYPE256
#    define CTYPE256
#  endif
#endif

/*

=head1 Character classes
There are three variants for all the functions in this section.  The base ones
operate using the character set of the platform Perl is running on.  The ones
with an C<_A> suffix operate on the ASCII character set, and the ones with an
C<_L1> suffix operate on the full Latin1 character set.  All are unaffected by
locale and by C<use bytes>.

For ASCII platforms, the base function with no suffix and the one with the
C<_A> suffix are identical.  The function with the C<_L1> suffix imposes the
Latin-1 character set onto the platform.  That is, the code points that are
ASCII are unaffected, since ASCII is a subset of Latin-1.  But the non-ASCII
code points are treated as if they are Latin-1 characters.  For example,
C<isSPACE_L1()> will return true when called with the code point 0xA0, which is
the Latin-1 NO-BREAK SPACE.

For EBCDIC platforms, the base function with no suffix and the one with the
C<_L1> suffix should be identical, since, as of this writing, the EBCDIC code
pages that Perl knows about all are equivalent to Latin-1.  The function that
ends in an C<_A> suffix will not return true unless the specified character also
has an ASCII equivalent.

=for apidoc Am|bool|isALPHA|char ch
Returns a boolean indicating whether the specified character is an
alphabetic character in the platform's native character set.
See the L<top of this section|/Character classes> for an explanation of variants
C<isALPHA_A> and C<isALPHA_L1>.

=for apidoc Am|bool|isASCII|char ch
Returns a boolean indicating whether the specified character is one of the 128
characters in the ASCII character set.  On non-ASCII platforms, it is if this
character corresponds to an ASCII character.  Variants C<isASCII_A()> and
C<isASCII_L1()> are identical to C<isASCII()>.

=for apidoc Am|bool|isDIGIT|char ch
Returns a boolean indicating whether the specified character is a
digit in the platform's native character set.
Variants C<isDIGIT_A> and C<isDIGIT_L1> are identical to C<isDIGIT>.

=for apidoc Am|bool|isLOWER|char ch
Returns a boolean indicating whether the specified character is a
lowercase character in the platform's native character set.
See the L<top of this section|/Character classes> for an explanation of variants
C<isLOWER_A> and C<isLOWER_L1>.

=for apidoc Am|bool|isOCTAL|char ch
Returns a boolean indicating whether the specified character is an
octal digit, [0-7] in the platform's native character set.
Variants C<isOCTAL_A> and C<isOCTAL_L1> are identical to C<isOCTAL>.

=for apidoc Am|bool|isSPACE|char ch
Returns a boolean indicating whether the specified character is a
whitespace character in the platform's native character set.  This is the same
as what C<\s> matches in a regular expression.
See the L<top of this section|/Character classes> for an explanation of variants
C<isSPACE_A> and C<isSPACE_L1>.

=for apidoc Am|bool|isUPPER|char ch
Returns a boolean indicating whether the specified character is an
uppercase character in the platform's native character set.
See the L<top of this section|/Character classes> for an explanation of variants
C<isUPPER_A> and C<isUPPER_L1>.

=for apidoc Am|bool|isWORDCHAR|char ch
Returns a boolean indicating whether the specified character is a
character that is any of: alphabetic, numeric, or an underscore.  This is the
same as what C<\w> matches in a regular expression.
C<isALNUM()> is a synonym provided for backward compatibility.  Note that it
does not have the standard C language meaning of alphanumeric, since it matches
an underscore and the standard meaning does not.
See the L<top of this section|/Character classes> for an explanation of variants
C<isWORDCHAR_A> and C<isWORDCHAR_L1>.

=for apidoc Am|bool|isXDIGIT|char ch
Returns a boolean indicating whether the specified character is a hexadecimal
digit, [0-9A-Fa-f].  Variants C<isXDIGIT_A()> and C<isXDIGIT_L1()> are
identical to C<isXDIGIT()>.

=head1 Character case changing

=for apidoc Am|char|toUPPER|char ch
Converts the specified character to uppercase in the platform's native
character set, if possible; otherwise returns the input character itself.

=for apidoc Am|char|toLOWER|char ch
Converts the specified character to lowercase in the platform's native
character set, if possible; otherwise returns the input character itself.

=cut

Note that these macros are repeated in Devel::PPPort, so should also be
patched there.  The file as of this writing is cpan/Devel-PPPort/parts/inc/misc

*/

/* Specify the widest unsigned type on the platform.  Use U64TYPE because U64
 * is known only in the perl core, and this macro can be called from outside
 * that */
#ifdef HAS_QUAD
#   define WIDEST_UTYPE U64TYPE
#else
#   define WIDEST_UTYPE U32
#endif

/* FITS_IN_8_BITS(c) returns true if c doesn't have  a bit set other than in
 * the lower 8.  It is designed to be hopefully bomb-proof, making sure that no
 * bits of information are lost even on a 64-bit machine, but to get the
 * compiler to optimize it out if possible.  This is because Configure makes
 * sure that the machine has an 8-bit byte, so if c is stored in a byte, the
 * sizeof() guarantees that this evaluates to a constant true at compile time.
 */
#define FITS_IN_8_BITS(c) ((sizeof(c) == 1) || !(((WIDEST_UTYPE)(c)) & ~0xFF))

#ifdef EBCDIC
#   define isASCII(c)    (FITS_IN_8_BITS(c) && (NATIVE_TO_UNI((U8) (c)) < 128))
#else
#   define isASCII(c)    ((WIDEST_UTYPE)(c) < 128)
#endif

#define isASCII_A(c)  isASCII(c)
#define isASCII_L1(c)  isASCII(c)

/* ASCII range only */
#ifdef H_PERL       /* If have access to perl.h, lookup in its table */
/* Bits for PL_charclass[].  These use names used in l1_char_class_tab.h but
 * their actual definitions are here.  If that has a name not used here, it
 * won't compile. */
#  define _CC_ALNUMC_A         (1<<0)
#  define _CC_ALNUMC_L1        (1<<1)
#  define _CC_ALPHA_A          (1<<2)
#  define _CC_ALPHA_L1         (1<<3)
#  define _CC_BLANK_A          (1<<4)
#  define _CC_BLANK_L1         (1<<5)
#  define _CC_CHARNAME_CONT    (1<<6)
#  define _CC_CNTRL_A          (1<<7)
#  define _CC_CNTRL_L1         (1<<8)
#  define _CC_DIGIT_A          (1<<9)
#  define _CC_GRAPH_A          (1<<10)
#  define _CC_GRAPH_L1         (1<<11)
#  define _CC_IDFIRST_A        (1<<12)
#  define _CC_IDFIRST_L1       (1<<13)
#  define _CC_LOWER_A          (1<<14)
#  define _CC_LOWER_L1         (1<<15)
#  define _CC_OCTAL_A          (1<<16)
#  define _CC_PRINT_A          (1<<17)
#  define _CC_PRINT_L1         (1<<18)
#  define _CC_PSXSPC_A         (1<<19)
#  define _CC_PSXSPC_L1        (1<<20)
#  define _CC_PUNCT_A          (1<<21)
#  define _CC_PUNCT_L1         (1<<22)
#  define _CC_SPACE_A          (1<<23)
#  define _CC_SPACE_L1         (1<<24)
#  define _CC_UPPER_A          (1<<25)
#  define _CC_UPPER_L1         (1<<26)
#  define _CC_WORDCHAR_A       (1<<27)
#  define _CC_WORDCHAR_L1      (1<<28)
#  define _CC_XDIGIT_A         (1<<29)
#  define _CC_NONLATIN1_FOLD   (1<<30)
#  define _CC_QUOTEMETA        (1U<<31)	/* 1U keeps Solaris from griping */
/* Unused: None
 * If more are needed, can give up some of the above.  The first ones to go
 * would be those that require just two tests to verify, either there are two
 * code points, like BLANK_A, or occupy a single range like OCTAL_A, DIGIT_A,
 * UPPER_A, and LOWER_A.
 */

#  ifdef DOINIT
EXTCONST  U32 PL_charclass[] = {
#    include "l1_char_class_tab.h"
};

#  else /* ! DOINIT */
EXTCONST U32 PL_charclass[];
#  endif

#   define isALNUMC_A(c) cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_ALNUMC_A))
#   define isALPHA_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_ALPHA_A))
#   define isBLANK_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_BLANK_A))
#   define isCNTRL_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_CNTRL_A))
#   define isDIGIT_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_DIGIT_A))
#   define isGRAPH_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_GRAPH_A))
#   define isIDFIRST_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_IDFIRST_A))
#   define isLOWER_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_LOWER_A))
#   define isOCTAL_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_OCTAL_A))
#   define isPRINT_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_PRINT_A))
#   define isPSXSPC_A(c) cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_PSXSPC_A))
#   define isPUNCT_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_PUNCT_A))
#   define isSPACE_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_SPACE_A))
#   define isUPPER_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_UPPER_A))
#   define isWORDCHAR_A(c) cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_WORDCHAR_A))
#   define isXDIGIT_A(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_XDIGIT_A))
    /* Either participates in a fold with a character above 255, or is a
     * multi-char fold */
#   define _HAS_NONLATIN1_FOLD_CLOSURE_ONLY_FOR_USE_BY_REGCOMP_DOT_C_AND_REGEXEC_DOT_C(c) ((! cBOOL(FITS_IN_8_BITS(c))) || (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_NONLATIN1_FOLD))
#   define _isQUOTEMETA(c) cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_QUOTEMETA))
#else   /* No perl.h. */
#   define isOCTAL_A(c)  ((c) <= '7' && (c) >= '0')
#   ifdef EBCDIC
#       define isALNUMC_A(c)   (isASCII(c) && isALNUMC(c))
#       define isALPHA_A(c)    (isASCII(c) && isALPHA(c))
#       define isBLANK_A(c)    (isASCII(c) && isBLANK(c))
#       define isCNTRL_A(c)    (isASCII(c) && isCNTRL(c))
#       define isDIGIT_A(c)    (isASCII(c) && isDIGIT(c))
#       define isGRAPH_A(c)    (isASCII(c) && isGRAPH(c))
#       define isIDFIRST_A(c)  (isASCII(c) && isIDFIRST(c))
#       define isLOWER_A(c)    (isASCII(c) && isLOWER(c))
#       define isPRINT_A(c)    (isASCII(c) && isPRINT(c))
#       define isPSXSPC_A(c)   (isASCII(c) && isPSXSPC(c))
#       define isPUNCT_A(c)    (isASCII(c) && isPUNCT(c))
#       define isSPACE_A(c)    (isASCII(c) && isSPACE(c))
#       define isUPPER_A(c)    (isASCII(c) && isUPPER(c))
#       define isWORDCHAR_A(c) (isASCII(c) && isWORDCHAR(c))
#       define isXDIGIT_A(c)   (isASCII(c) && isXDIGIT(c))
#   else   /* ASCII platform, no perl.h */
#       define isALNUMC_A(c) (isALPHA_A(c) || isDIGIT_A(c))
#       define isALPHA_A(c)  (isUPPER_A(c) || isLOWER_A(c))
#       define isBLANK_A(c)  ((c) == ' ' || (c) == '\t')
#       define isCNTRL_A(c)  (FITS_IN_8_BITS(c) && ((U8) (c) < ' ' || (c) == 127))
#       define isDIGIT_A(c)  ((c) <= '9' && (c) >= '0')
#       define isGRAPH_A(c)  (isWORDCHAR_A(c) || isPUNCT_A(c))
#       define isIDFIRST_A(c) (isALPHA_A(c) || (c) == '_')
#       define isLOWER_A(c)  ((c) >= 'a' && (c) <= 'z')
#       define isPRINT_A(c)  (((c) >= 32 && (c) < 127))
#       define isPSXSPC_A(c) (isSPACE_A(c) || (c) == '\v')
#       define isPUNCT_A(c)  (((c) >= 33 && (c) <= 47) || ((c) >= 58 && (c) <= 64)  || ((c) >= 91 && (c) <= 96) || ((c) >= 123 && (c) <= 126))
#       define isSPACE_A(c)  ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) =='\r' || (c) == '\f')
#       define isUPPER_A(c)  ((c) <= 'Z' && (c) >= 'A')
#       define isWORDCHAR_A(c) (isALPHA_A(c) || isDIGIT_A(c) || (c) == '_')
#       define isXDIGIT_A(c)   (isDIGIT_A(c) || ((c) >= 'a' && (c) <= 'f') || ((c) <= 'F' && (c) >= 'A'))
#   endif
#endif  /* ASCII range definitions */

/* Latin1 definitions */
#ifdef H_PERL
#   define isALNUMC_L1(c) cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_ALNUMC_L1))
#   define isALPHA_L1(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_ALPHA_L1))
#   define isBLANK_L1(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_BLANK_L1))
/*  continuation character for legal NAME in \N{NAME} */
#   define isCHARNAME_CONT(c) cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_CHARNAME_CONT))
#   define isCNTRL_L1(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_CNTRL_L1))
#   define isGRAPH_L1(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_GRAPH_L1))
#   define isIDFIRST_L1(c) cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_IDFIRST_L1))
#   define isLOWER_L1(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_LOWER_L1))
#   define isPRINT_L1(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_PRINT_L1))
#   define isPSXSPC_L1(c) cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_PSXSPC_L1))
#   define isPUNCT_L1(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_PUNCT_L1))
#   define isSPACE_L1(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_SPACE_L1))
#   define isUPPER_L1(c)  cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_UPPER_L1))
#   define isWORDCHAR_L1(c) cBOOL(FITS_IN_8_BITS(c) && (PL_charclass[(U8) NATIVE_TO_UNI(c)] & _CC_WORDCHAR_L1))
#else /* No access to perl.h.  Only a few provided here, just in case needed
       * for backwards compatibility */
    /* ALPHAU includes Unicode semantics for latin1 characters.  It has an extra
     * >= AA test to speed up ASCII-only tests at the expense of the others */
#   define isALPHA_L1(c) (isALPHA(c) || (NATIVE_TO_UNI((U8) c) >= 0xAA \
	&& ((NATIVE_TO_UNI((U8) c) >= 0xC0 \
		&& NATIVE_TO_UNI((U8) c) != 0xD7 && NATIVE_TO_UNI((U8) c) != 0xF7) \
	    || NATIVE_TO_UNI((U8) c) == 0xAA \
	    || NATIVE_TO_UNI((U8) c) == 0xB5 \
	    || NATIVE_TO_UNI((U8) c) == 0xBA)))
#   define isCHARNAME_CONT(c) (isALNUM_L1(c) || (c) == ' ' || (c) == '-' || (c) == '(' || (c) == ')' || (c) == ':' || NATIVE_TO_UNI((U8) c) == 0xA0)
#endif

/* Macros for backwards compatibility and for completeness when the ASCII and
 * Latin1 values are identical */
#define isALNUM(c)      isWORDCHAR(c)
#define isALNUMU(c)     isWORDCHAR_L1(c)
#define isALPHAU(c)     isALPHA_L1(c)
#define isDIGIT_L1(c)   isDIGIT_A(c)
#define isOCTAL(c)      isOCTAL_A(c)
#define isOCTAL_L1(c)   isOCTAL_A(c)
#define isXDIGIT_L1(c)  isXDIGIT_A(c)

/* Macros that differ between EBCDIC and ASCII.  Where C89 defines a function,
 * that is used in the EBCDIC form, because in EBCDIC we do not do locales:
 * therefore can use native functions.  For those where C89 doesn't define a
 * function, use our function, assuming that the EBCDIC code page is isomorphic
 * with Latin1, which the three currently recognized by Perl are.  Some libc's
 * have an isblank(), but it's not guaranteed. */
#ifdef EBCDIC
#   define isALNUMC(c)	isalnum(c)
#   define isALPHA(c)	isalpha(c)
#   define isBLANK(c)	((c) == ' ' || (c) == '\t' || NATIVE_TO_UNI(c) == 0xA0)
#   define isCNTRL(c)	iscntrl(c)
#   define isDIGIT(c)	isdigit(c)
#   define isGRAPH(c)	isgraph(c)
#   define isIDFIRST(c) (isALPHA(c) || (c) == '_')
#   define isLOWER(c)	islower(c)
#   define isPRINT(c)	isprint(c)
#   define isPSXSPC(c)	isspace(c)
#   define isPUNCT(c)	ispunct(c)
#   define isSPACE(c)   (isPSXSPC(c) && (c) != '\v')
#   define isUPPER(c)	isupper(c)
#   define isXDIGIT(c)	isxdigit(c)
#   define isWORDCHAR(c) (isalnum(c) || (c) == '_')
#   define toLOWER(c)	tolower(c)
#   define toUPPER(c)	toupper(c)
#else /* Not EBCDIC: ASCII-only matching */
#   define isALNUMC(c)  isALNUMC_A(c)
#   define isALPHA(c)   isALPHA_A(c)
#   define isBLANK(c)   isBLANK_A(c)
#   define isCNTRL(c)   isCNTRL_A(c)
#   define isDIGIT(c)   isDIGIT_A(c)
#   define isGRAPH(c)   isGRAPH_A(c)
#   define isIDFIRST(c) isIDFIRST_A(c)
#   define isLOWER(c)   isLOWER_A(c)
#   define isPRINT(c)   isPRINT_A(c)
#   define isPSXSPC(c)	isPSXSPC_A(c)
#   define isPUNCT(c)   isPUNCT_A(c)
#   define isSPACE(c)   isSPACE_A(c)
#   define isUPPER(c)   isUPPER_A(c)
#   define isWORDCHAR(c) isWORDCHAR_A(c)
#   define isXDIGIT(c)  isXDIGIT_A(c)

    /* ASCII casing.  These could also be written as
	#define toLOWER(c) (isASCII(c) ? toLOWER_LATIN1(c) : (c))
	#define toUPPER(c) (isASCII(c) ? toUPPER_LATIN1_MOD(c) : (c))
       which uses table lookup and mask instead of subtraction.  (This would
       work because the _MOD does not apply in the ASCII range) */
#   define toLOWER(c)	(isUPPER(c) ? (c) + ('a' - 'A') : (c))
#   define toUPPER(c)	(isLOWER(c) ? (c) - ('a' - 'A') : (c))
#endif


/* Use table lookup for speed; return error character for input
 * out-of-range */
#define toLOWER_LATIN1(c)    (FITS_IN_8_BITS(c)                            \
                             ? UNI_TO_NATIVE(PL_latin1_lc[                 \
                                               NATIVE_TO_UNI( (U8) (c)) ]) \
                             : UNICODE_REPLACEMENT)
/* Modified uc.  Is correct uc except for three non-ascii chars which are
 * all mapped to one of them, and these need special handling; error
 * character for input out-of-range */
#define toUPPER_LATIN1_MOD(c) (FITS_IN_8_BITS(c)                           \
                              ? UNI_TO_NATIVE(PL_mod_latin1_uc[            \
                                               NATIVE_TO_UNI( (U8) (c)) ]) \
                              : UNICODE_REPLACEMENT)

#ifdef USE_NEXT_CTYPE

#  define isALNUM_LC(c) \
	(NXIsAlNum((unsigned int)(c)) || (char)(c) == '_')
#  define isIDFIRST_LC(c) \
	(NXIsAlpha((unsigned int)(c)) || (char)(c) == '_')
#  define isALPHA_LC(c)		NXIsAlpha((unsigned int)(c))
#  define isASCII_LC(c)		isASCII((unsigned int)(c))
#  define isBLANK_LC(c)		isBLANK((unsigned int)(c))
#  define isSPACE_LC(c)		NXIsSpace((unsigned int)(c))
#  define isDIGIT_LC(c)		NXIsDigit((unsigned int)(c))
#  define isUPPER_LC(c)		NXIsUpper((unsigned int)(c))
#  define isLOWER_LC(c)		NXIsLower((unsigned int)(c))
#  define isALNUMC_LC(c)	NXIsAlNum((unsigned int)(c))
#  define isCNTRL_LC(c)		NXIsCntrl((unsigned int)(c))
#  define isGRAPH_LC(c)		NXIsGraph((unsigned int)(c))
#  define isPRINT_LC(c)		NXIsPrint((unsigned int)(c))
#  define isPUNCT_LC(c)		NXIsPunct((unsigned int)(c))
#  define toUPPER_LC(c)		NXToUpper((unsigned int)(c))
#  define toLOWER_LC(c)		NXToLower((unsigned int)(c))

#else /* !USE_NEXT_CTYPE */

#  if defined(CTYPE256) || (!defined(isascii) && !defined(HAS_ISASCII))

/* Note that the foo_LC() macros in this case generally are defined only on
 * code points 0-256, and give undefined, unwarned results if called with
 * values outside that range */

#    define isALNUM_LC(c)   (isalnum((unsigned char)(c)) || (char)(c) == '_')
#    define isIDFIRST_LC(c) (isalpha((unsigned char)(c)) || (char)(c) == '_')
#    define isALPHA_LC(c)	isalpha((unsigned char)(c))
#    ifdef HAS_ISASCII
#	define isASCII_LC(c)	isascii((unsigned char)(c))
#    else
#	define isASCII_LC(c)	isASCII((unsigned char)(c))
#    endif
#    ifdef HAS_ISBLANK
#	define isBLANK_LC(c)	isblank((unsigned char)(c))
#    else
#	define isBLANK_LC(c)	isBLANK((unsigned char)(c))
#    endif
#    define isSPACE_LC(c)	isspace((unsigned char)(c))
#    define isDIGIT_LC(c)	isdigit((unsigned char)(c))
#    define isUPPER_LC(c)	isupper((unsigned char)(c))
#    define isLOWER_LC(c)	islower((unsigned char)(c))
#    define isALNUMC_LC(c)	isalnum((unsigned char)(c))
#    define isCNTRL_LC(c)	iscntrl((unsigned char)(c))
#    define isGRAPH_LC(c)	isgraph((unsigned char)(c))
#    define isPRINT_LC(c)	isprint((unsigned char)(c))
#    define isPUNCT_LC(c)	ispunct((unsigned char)(c))
#    define toUPPER_LC(c)	toupper((unsigned char)(c))
#    define toLOWER_LC(c)	tolower((unsigned char)(c))

#  else

#    define isALNUM_LC(c)	(isascii(c) && (isalnum(c) || (c) == '_'))
#    define isIDFIRST_LC(c)	(isascii(c) && (isalpha(c) || (c) == '_'))
#    define isALPHA_LC(c)	(isascii(c) && isalpha(c))
#    define isASCII_LC(c)	isascii(c)
#    ifdef HAS_ISBLANK
#	define isBLANK_LC(c)	(isascii(c) && isblank(c))
#    else
#	define isBLANK_LC(c)	isBLANK(c)
#    endif
#    define isSPACE_LC(c)	(isascii(c) && isspace(c))
#    define isDIGIT_LC(c)	(isascii(c) && isdigit(c))
#    define isUPPER_LC(c)	(isascii(c) && isupper(c))
#    define isLOWER_LC(c)	(isascii(c) && islower(c))
#    define isALNUMC_LC(c)	(isascii(c) && isalnum(c))
#    define isCNTRL_LC(c)	(isascii(c) && iscntrl(c))
#    define isGRAPH_LC(c)	(isascii(c) && isgraph(c))
#    define isPRINT_LC(c)	(isascii(c) && isprint(c))
#    define isPUNCT_LC(c)	(isascii(c) && ispunct(c))
#    define toUPPER_LC(c)	toupper(c)
#    define toLOWER_LC(c)	tolower(c)

#  endif
#endif /* USE_NEXT_CTYPE */

#define isPSXSPC_LC(c)		(isSPACE_LC(c) || (c) == '\v')

/* For use in the macros just below.  If the input is Latin1, use the Latin1
 * (_L1) version of the macro; otherwise use the function.  Won't compile if
 * 'c' isn't unsigned, as won't match function prototype. The macros do bounds
 * checking, so have duplicate checks here, so could create versions of the
 * macros that don't, but experiments show that gcc optimizes them out anyway.
 */
#define generic_uni(macro, function, c) ((c) < 256               \
                                         ? CAT2(macro, _L1)(c)   \
                                         : function(c))
/* Note that all ignore 'use bytes' */

#define isALNUM_uni(c)		generic_uni(isWORDCHAR, is_uni_alnum, c)
#define isIDFIRST_uni(c)        generic_uni(isIDFIRST, is_uni_idfirst, c)
#define isALPHA_uni(c)		generic_uni(isALPHA, is_uni_alpha, c)
#define isSPACE_uni(c)		generic_uni(isSPACE, is_uni_space, c)
#define isDIGIT_uni(c)		generic_uni(isDIGIT, is_uni_digit, c)
#define isUPPER_uni(c)		generic_uni(isUPPER, is_uni_upper, c)
#define isLOWER_uni(c)		generic_uni(isLOWER, is_uni_lower, c)
#define isASCII_uni(c)		isASCII(c)
/* All controls are in Latin1 */
#define isCNTRL_uni(c)		((c) < 256 && isCNTRL_L1(c))
#define isGRAPH_uni(c)		generic_uni(isGRAPH, is_uni_graph, c)
#define isPRINT_uni(c)		generic_uni(isPRINT, is_uni_print, c)
#define isPUNCT_uni(c)		generic_uni(isPUNCT, is_uni_punct, c)
#define isXDIGIT_uni(c)		generic_uni(isXDIGIT, is_uni_xdigit, c)
#define toUPPER_uni(c,s,l)	to_uni_upper(c,s,l)
#define toTITLE_uni(c,s,l)	to_uni_title(c,s,l)
#define toLOWER_uni(c,s,l)	to_uni_lower(c,s,l)
#define toFOLD_uni(c,s,l)	to_uni_fold(c,s,l)

/* Posix and regular space differ only in U+000B, which is in Latin1 */
#define isPSXSPC_uni(c)		((c) < 256 ? isPSXSPC_L1(c) : isSPACE_uni(c))
#define isBLANK_uni(c)		isBLANK(c) /* could be wrong */

#define isALNUM_LC_uvchr(c)	(c < 256 ? isALNUM_LC(c) : is_uni_alnum_lc(c))
#define isIDFIRST_LC_uvchr(c)	(c < 256 ? isIDFIRST_LC(c) : is_uni_idfirst_lc(c))
#define isALPHA_LC_uvchr(c)	(c < 256 ? isALPHA_LC(c) : is_uni_alpha_lc(c))
#define isSPACE_LC_uvchr(c)	(c < 256 ? isSPACE_LC(c) : is_uni_space_lc(c))
#define isDIGIT_LC_uvchr(c)	(c < 256 ? isDIGIT_LC(c) : is_uni_digit_lc(c))
#define isUPPER_LC_uvchr(c)	(c < 256 ? isUPPER_LC(c) : is_uni_upper_lc(c))
#define isLOWER_LC_uvchr(c)	(c < 256 ? isLOWER_LC(c) : is_uni_lower_lc(c))
#define isCNTRL_LC_uvchr(c)	(c < 256 ? isCNTRL_LC(c) : is_uni_cntrl_lc(c))
#define isGRAPH_LC_uvchr(c)	(c < 256 ? isGRAPH_LC(c) : is_uni_graph_lc(c))
#define isPRINT_LC_uvchr(c)	(c < 256 ? isPRINT_LC(c) : is_uni_print_lc(c))
#define isPUNCT_LC_uvchr(c)	(c < 256 ? isPUNCT_LC(c) : is_uni_punct_lc(c))

#define isPSXSPC_LC_uni(c)	(isSPACE_LC_uni(c) ||(c) == '\f')
#define isBLANK_LC_uni(c)	isBLANK(c) /* could be wrong */

/* For use in the macros just below.  If the input is ASCII, use the ASCII (_A)
 * version of the macro; if the input is in the upper Latin1 range, use the
 * Latin1 (_L1) version of the macro, after converting from utf8; otherwise use
 * the function.  This relies on the fact that ASCII characters have the same
 * representation whether utf8 or not */
#define generic_utf8(macro, function, p) (isASCII(*(p))                        \
                                         ? CAT2(CAT2(macro,_),A)(*(p))               \
                                         : (UTF8_IS_DOWNGRADEABLE_START(*(p))) \
                                           ? CAT2(macro, _L1)                  \
                                             (TWO_BYTE_UTF8_TO_UNI(*(p),       \
                                                                   *((p)+1)))  \
                                           : function(p))

/* Note that all assume that the utf8 has been validated, and ignore 'use
 * bytes' */

#define isALNUM_utf8(p)		generic_utf8(isWORDCHAR, is_utf8_alnum, p)
/* To prevent S_scan_word in toke.c from hanging, we have to make sure that
 * IDFIRST is an alnum.  See
 * http://rt.perl.org/rt3/Ticket/Display.html?id=74022 for more detail than you
 * ever wanted to know about.  XXX It is unclear if this should extend to
 * isIDFIRST_uni() which it hasn't so far.  (In the ASCII range, there isn't a
 * difference.) This used to be not the XID version, but we decided to go with
 * the more modern Unicode definition */
#define isIDFIRST_utf8(p)       (isASCII(*(p))                                  \
                                ? isIDFIRST_A(*(p))                             \
                                : (UTF8_IS_DOWNGRADEABLE_START(*(p)))           \
                                  ? isIDFIRST_L1(TWO_BYTE_UTF8_TO_UNI(*(p),     \
                                                                      *((p)+1)))\
                                  : Perl__is_utf8__perl_idstart(aTHX_ p))
#define isIDCONT_utf8(p)	generic_utf8(isWORDCHAR, is_utf8_xidcont, p)
#define isALPHA_utf8(p)		generic_utf8(isALPHA, is_utf8_alpha, p)
#define isSPACE_utf8(p)		generic_utf8(isSPACE, is_utf8_space, p)
#define isDIGIT_utf8(p)		generic_utf8(isDIGIT, is_utf8_digit, p)
#define isUPPER_utf8(p)		generic_utf8(isUPPER, is_utf8_upper, p)
#define isLOWER_utf8(p)		generic_utf8(isLOWER, is_utf8_lower, p)
/* Because ASCII is invariant under utf8, the non-utf8 macro works */
#define isASCII_utf8(p)		isASCII(p)
#define isCNTRL_utf8(p)		generic_utf8(isCNTRL, is_utf8_cntrl, p)
#define isGRAPH_utf8(p)		generic_utf8(isGRAPH, is_utf8_graph, p)
#define isPRINT_utf8(p)		generic_utf8(isPRINT, is_utf8_print, p)
#define isPUNCT_utf8(p)		generic_utf8(isPUNCT, is_utf8_punct, p)
#define isXDIGIT_utf8(p)	generic_utf8(isXDIGIT, is_utf8_xdigit, p)
#define toUPPER_utf8(p,s,l)	to_utf8_upper(p,s,l)
#define toTITLE_utf8(p,s,l)	to_utf8_title(p,s,l)
#define toLOWER_utf8(p,s,l)	to_utf8_lower(p,s,l)

/* Posix and regular space differ only in U+000B, which is in ASCII (and hence
 * Latin1 */
#define isPSXSPC_utf8(p)	((isASCII(*(p)))                               \
                                ? isPSXSPC_A(*(p))                             \
                                : (UTF8_IS_DOWNGRADEABLE_START(*(p))           \
				  ? isPSXSPC_L1(TWO_BYTE_UTF8_TO_UNI(*(p),     \
                                                                     *((p)+1)))\
                                  : isSPACE_utf8(p)))
#define isBLANK_utf8(c)		isBLANK(c) /* could be wrong */

#define isALNUM_LC_utf8(p)	isALNUM_LC_uvchr(valid_utf8_to_uvchr(p,  0))
#define isIDFIRST_LC_utf8(p)	isIDFIRST_LC_uvchr(valid_utf8_to_uvchr(p,  0))
#define isALPHA_LC_utf8(p)	isALPHA_LC_uvchr(valid_utf8_to_uvchr(p,  0))
#define isSPACE_LC_utf8(p)	isSPACE_LC_uvchr(valid_utf8_to_uvchr(p,  0))
#define isDIGIT_LC_utf8(p)	isDIGIT_LC_uvchr(valid_utf8_to_uvchr(p,  0))
#define isUPPER_LC_utf8(p)	isUPPER_LC_uvchr(valid_utf8_to_uvchr(p,  0))
#define isLOWER_LC_utf8(p)	isLOWER_LC_uvchr(valid_utf8_to_uvchr(p,  0))
#define isALNUMC_LC_utf8(p)	isALNUMC_LC_uvchr(valid_utf8_to_uvchr(p,  0))
#define isCNTRL_LC_utf8(p)	isCNTRL_LC_uvchr(valid_utf8_to_uvchr(p,  0))
#define isGRAPH_LC_utf8(p)	isGRAPH_LC_uvchr(valid_utf8_to_uvchr(p,  0))
#define isPRINT_LC_utf8(p)	isPRINT_LC_uvchr(valid_utf8_to_uvchr(p,  0))
#define isPUNCT_LC_utf8(p)	isPUNCT_LC_uvchr(valid_utf8_to_uvchr(p,  0))

#define isPSXSPC_LC_utf8(c)	(isSPACE_LC_utf8(c) ||(c) == '\f')
#define isBLANK_LC_utf8(c)	isBLANK(c) /* could be wrong */

/* This conversion works both ways, strangely enough. On EBCDIC platforms,
 * CTRL-@ is 0, CTRL-A is 1, etc, just like on ASCII */
#  define toCTRL(c)    (toUPPER(NATIVE_TO_UNI(c)) ^ 64)

/* Line numbers are unsigned, 32 bits. */
typedef U32 line_t;
#define NOLINE ((line_t) 4294967295UL)

/* Helpful alias for version prescan */
#define is_LAX_VERSION(a,b) \
	(a != Perl_prescan_version(aTHX_ a, FALSE, b, NULL, NULL, NULL, NULL))

#define is_STRICT_VERSION(a,b) \
	(a != Perl_prescan_version(aTHX_ a, TRUE, b, NULL, NULL, NULL, NULL))

#define BADVERSION(a,b,c) \
	if (b) { \
	    *b = c; \
	} \
	return a;

/*
=head1 Memory Management

=for apidoc Am|void|Newx|void* ptr|int nitems|type
The XSUB-writer's interface to the C C<malloc> function.

In 5.9.3, Newx() and friends replace the older New() API, and drops
the first parameter, I<x>, a debug aid which allowed callers to identify
themselves.  This aid has been superseded by a new build option,
PERL_MEM_LOG (see L<perlhacktips/PERL_MEM_LOG>).  The older API is still
there for use in XS modules supporting older perls.

=for apidoc Am|void|Newxc|void* ptr|int nitems|type|cast
The XSUB-writer's interface to the C C<malloc> function, with
cast.  See also C<Newx>.

=for apidoc Am|void|Newxz|void* ptr|int nitems|type
The XSUB-writer's interface to the C C<malloc> function.  The allocated
memory is zeroed with C<memzero>.  See also C<Newx>.

=for apidoc Am|void|Renew|void* ptr|int nitems|type
The XSUB-writer's interface to the C C<realloc> function.

=for apidoc Am|void|Renewc|void* ptr|int nitems|type|cast
The XSUB-writer's interface to the C C<realloc> function, with
cast.

=for apidoc Am|void|Safefree|void* ptr
The XSUB-writer's interface to the C C<free> function.

=for apidoc Am|void|Move|void* src|void* dest|int nitems|type
The XSUB-writer's interface to the C C<memmove> function.  The C<src> is the
source, C<dest> is the destination, C<nitems> is the number of items, and C<type> is
the type.  Can do overlapping moves.  See also C<Copy>.

=for apidoc Am|void *|MoveD|void* src|void* dest|int nitems|type
Like C<Move> but returns dest. Useful for encouraging compilers to tail-call
optimise.

=for apidoc Am|void|Copy|void* src|void* dest|int nitems|type
The XSUB-writer's interface to the C C<memcpy> function.  The C<src> is the
source, C<dest> is the destination, C<nitems> is the number of items, and C<type> is
the type.  May fail on overlapping copies.  See also C<Move>.

=for apidoc Am|void *|CopyD|void* src|void* dest|int nitems|type

Like C<Copy> but returns dest. Useful for encouraging compilers to tail-call
optimise.

=for apidoc Am|void|Zero|void* dest|int nitems|type

The XSUB-writer's interface to the C C<memzero> function.  The C<dest> is the
destination, C<nitems> is the number of items, and C<type> is the type.

=for apidoc Am|void *|ZeroD|void* dest|int nitems|type

Like C<Zero> but returns dest. Useful for encouraging compilers to tail-call
optimise.

=for apidoc Am|void|StructCopy|type src|type dest|type
This is an architecture-independent macro to copy one structure to another.

=for apidoc Am|void|PoisonWith|void* dest|int nitems|type|U8 byte

Fill up memory with a byte pattern (a byte repeated over and over
again) that hopefully catches attempts to access uninitialized memory.

=for apidoc Am|void|PoisonNew|void* dest|int nitems|type

PoisonWith(0xAB) for catching access to allocated but uninitialized memory.

=for apidoc Am|void|PoisonFree|void* dest|int nitems|type

PoisonWith(0xEF) for catching access to freed memory.

=for apidoc Am|void|Poison|void* dest|int nitems|type

PoisonWith(0xEF) for catching access to freed memory.

=cut */

/* Maintained for backwards-compatibility only. Use newSV() instead. */
#ifndef PERL_CORE
#define NEWSV(x,len)	newSV(len)
#endif

#define MEM_SIZE_MAX ((MEM_SIZE)~0)

/* The +0.0 in MEM_WRAP_CHECK_ is an attempt to foil
 * overly eager compilers that will bleat about e.g.
 * (U16)n > (size_t)~0/sizeof(U16) always being false. */
#ifdef PERL_MALLOC_WRAP
#define MEM_WRAP_CHECK(n,t) MEM_WRAP_CHECK_1(n,t,PL_memory_wrap)
#define MEM_WRAP_CHECK_1(n,t,a) \
	(void)(sizeof(t) > 1 && ((MEM_SIZE)(n)+0.0) > MEM_SIZE_MAX/sizeof(t) && (Perl_croak_nocontext("%s",(a)),0))
#define MEM_WRAP_CHECK_(n,t) MEM_WRAP_CHECK(n,t),

#define PERL_STRLEN_ROUNDUP(n) ((void)(((n) > MEM_SIZE_MAX - 2 * PERL_STRLEN_ROUNDUP_QUANTUM) ? (Perl_croak_nocontext("%s",PL_memory_wrap),0):0),((n-1+PERL_STRLEN_ROUNDUP_QUANTUM)&~((MEM_SIZE)PERL_STRLEN_ROUNDUP_QUANTUM-1)))

#else

#define MEM_WRAP_CHECK(n,t)
#define MEM_WRAP_CHECK_1(n,t,a)
#define MEM_WRAP_CHECK_2(n,t,a,b)
#define MEM_WRAP_CHECK_(n,t)

#define PERL_STRLEN_ROUNDUP(n) (((n-1+PERL_STRLEN_ROUNDUP_QUANTUM)&~((MEM_SIZE)PERL_STRLEN_ROUNDUP_QUANTUM-1)))

#endif

#ifdef PERL_MEM_LOG
/*
 * If PERL_MEM_LOG is defined, all Newx()s, Renew()s, and Safefree()s
 * go through functions, which are handy for debugging breakpoints, but
 * which more importantly get the immediate calling environment (file and
 * line number, and C function name if available) passed in.  This info can
 * then be used for logging the calls, for which one gets a sample
 * implementation unless -DPERL_MEM_LOG_NOIMPL is also defined.
 *
 * Known problems:
 * - not all memory allocs get logged, only those
 *   that go through Newx() and derivatives (while all
 *   Safefrees do get logged)
 * - __FILE__ and __LINE__ do not work everywhere
 * - __func__ or __FUNCTION__ even less so
 * - I think more goes on after the perlio frees but
 *   the thing is that STDERR gets closed (as do all
 *   the file descriptors)
 * - no deeper calling stack than the caller of the Newx()
 *   or the kind, but do I look like a C reflection/introspection
 *   utility to you?
 * - the function prototypes for the logging functions
 *   probably should maybe be somewhere else than handy.h
 * - one could consider inlining (macrofying) the logging
 *   for speed, but I am too lazy
 * - one could imagine recording the allocations in a hash,
 *   (keyed by the allocation address?), and maintain that
 *   through reallocs and frees, but how to do that without
 *   any News() happening...?
 * - lots of -Ddefines to get useful/controllable output
 * - lots of ENV reads
 */

PERL_EXPORT_C Malloc_t Perl_mem_log_alloc(const UV n, const UV typesize, const char *type_name, Malloc_t newalloc, const char *filename, const int linenumber, const char *funcname);

PERL_EXPORT_C Malloc_t Perl_mem_log_realloc(const UV n, const UV typesize, const char *type_name, Malloc_t oldalloc, Malloc_t newalloc, const char *filename, const int linenumber, const char *funcname);

PERL_EXPORT_C Malloc_t Perl_mem_log_free(Malloc_t oldalloc, const char *filename, const int linenumber, const char *funcname);

# ifdef PERL_CORE
#  ifndef PERL_MEM_LOG_NOIMPL
enum mem_log_type {
  MLT_ALLOC,
  MLT_REALLOC,
  MLT_FREE,
  MLT_NEW_SV,
  MLT_DEL_SV
};
#  endif
#  if defined(PERL_IN_SV_C)  /* those are only used in sv.c */
void Perl_mem_log_new_sv(const SV *sv, const char *filename, const int linenumber, const char *funcname);
void Perl_mem_log_del_sv(const SV *sv, const char *filename, const int linenumber, const char *funcname);
#  endif
# endif

#endif

#ifdef PERL_MEM_LOG
#define MEM_LOG_ALLOC(n,t,a)     Perl_mem_log_alloc(n,sizeof(t),STRINGIFY(t),a,__FILE__,__LINE__,FUNCTION__)
#define MEM_LOG_REALLOC(n,t,v,a) Perl_mem_log_realloc(n,sizeof(t),STRINGIFY(t),v,a,__FILE__,__LINE__,FUNCTION__)
#define MEM_LOG_FREE(a)          Perl_mem_log_free(a,__FILE__,__LINE__,FUNCTION__)
#endif

#ifndef MEM_LOG_ALLOC
#define MEM_LOG_ALLOC(n,t,a)     (a)
#endif
#ifndef MEM_LOG_REALLOC
#define MEM_LOG_REALLOC(n,t,v,a) (a)
#endif
#ifndef MEM_LOG_FREE
#define MEM_LOG_FREE(a)          (a)
#endif

#define Newx(v,n,t)	(v = (MEM_WRAP_CHECK_(n,t) (t*)MEM_LOG_ALLOC(n,t,safemalloc((MEM_SIZE)((n)*sizeof(t))))))
#define Newxc(v,n,t,c)	(v = (MEM_WRAP_CHECK_(n,t) (c*)MEM_LOG_ALLOC(n,t,safemalloc((MEM_SIZE)((n)*sizeof(t))))))
#define Newxz(v,n,t)	(v = (MEM_WRAP_CHECK_(n,t) (t*)MEM_LOG_ALLOC(n,t,safecalloc((n),sizeof(t)))))

#ifndef PERL_CORE
/* pre 5.9.x compatibility */
#define New(x,v,n,t)	Newx(v,n,t)
#define Newc(x,v,n,t,c)	Newxc(v,n,t,c)
#define Newz(x,v,n,t)	Newxz(v,n,t)
#endif

#define Renew(v,n,t) \
	  (v = (MEM_WRAP_CHECK_(n,t) (t*)MEM_LOG_REALLOC(n,t,v,saferealloc((Malloc_t)(v),(MEM_SIZE)((n)*sizeof(t))))))
#define Renewc(v,n,t,c) \
	  (v = (MEM_WRAP_CHECK_(n,t) (c*)MEM_LOG_REALLOC(n,t,v,saferealloc((Malloc_t)(v),(MEM_SIZE)((n)*sizeof(t))))))

#ifdef PERL_POISON
#define Safefree(d) \
  ((d) ? (void)(safefree(MEM_LOG_FREE((Malloc_t)(d))), Poison(&(d), 1, Malloc_t)) : (void) 0)
#else
#define Safefree(d)	safefree(MEM_LOG_FREE((Malloc_t)(d)))
#endif

#define Move(s,d,n,t)	(MEM_WRAP_CHECK_(n,t) (void)memmove((char*)(d),(const char*)(s), (n) * sizeof(t)))
#define Copy(s,d,n,t)	(MEM_WRAP_CHECK_(n,t) (void)memcpy((char*)(d),(const char*)(s), (n) * sizeof(t)))
#define Zero(d,n,t)	(MEM_WRAP_CHECK_(n,t) (void)memzero((char*)(d), (n) * sizeof(t)))

#define MoveD(s,d,n,t)	(MEM_WRAP_CHECK_(n,t) memmove((char*)(d),(const char*)(s), (n) * sizeof(t)))
#define CopyD(s,d,n,t)	(MEM_WRAP_CHECK_(n,t) memcpy((char*)(d),(const char*)(s), (n) * sizeof(t)))
#ifdef HAS_MEMSET
#define ZeroD(d,n,t)	(MEM_WRAP_CHECK_(n,t) memzero((char*)(d), (n) * sizeof(t)))
#else
/* Using bzero(), which returns void.  */
#define ZeroD(d,n,t)	(MEM_WRAP_CHECK_(n,t) memzero((char*)(d), (n) * sizeof(t)),d)
#endif

#define PoisonWith(d,n,t,b)	(MEM_WRAP_CHECK_(n,t) (void)memset((char*)(d), (U8)(b), (n) * sizeof(t)))
#define PoisonNew(d,n,t)	PoisonWith(d,n,t,0xAB)
#define PoisonFree(d,n,t)	PoisonWith(d,n,t,0xEF)
#define Poison(d,n,t)		PoisonFree(d,n,t)

#ifdef USE_STRUCT_COPY
#define StructCopy(s,d,t) (*((t*)(d)) = *((t*)(s)))
#else
#define StructCopy(s,d,t) Copy(s,d,1,t)
#endif

#define C_ARRAY_LENGTH(a)	(sizeof(a)/sizeof((a)[0]))
#define C_ARRAY_END(a)		(a) + (sizeof(a)/sizeof((a)[0]))

#ifdef NEED_VA_COPY
# ifdef va_copy
#  define Perl_va_copy(s, d) va_copy(d, s)
# else
#  if defined(__va_copy)
#   define Perl_va_copy(s, d) __va_copy(d, s)
#  else
#   define Perl_va_copy(s, d) Copy(s, d, 1, va_list)
#  endif
# endif
#endif

/* convenience debug macros */
#ifdef USE_ITHREADS
#define pTHX_FORMAT  "Perl interpreter: 0x%p"
#define pTHX__FORMAT ", Perl interpreter: 0x%p"
#define pTHX_VALUE_   (void *)my_perl,
#define pTHX_VALUE    (void *)my_perl
#define pTHX__VALUE_ ,(void *)my_perl,
#define pTHX__VALUE  ,(void *)my_perl
#else
#define pTHX_FORMAT
#define pTHX__FORMAT
#define pTHX_VALUE_
#define pTHX_VALUE
#define pTHX__VALUE_
#define pTHX__VALUE
#endif /* USE_ITHREADS */

/* Perl_deprecate was not part of the public API, and did not have a deprecate()
   shortcut macro defined without -DPERL_CORE. Neither codesearch.google.com nor
   CPAN::Unpack show any users outside the core.  */
#ifdef PERL_CORE
#  define deprecate(s) Perl_ck_warner_d(aTHX_ packWARN(WARN_DEPRECATED), "Use of " s " is deprecated")
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
