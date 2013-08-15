/*    utf8.h
 *
 *    Copyright (C) 2000, 2001, 2002, 2005, 2006, 2007, 2009,
 *    2010, 2011 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/* Use UTF-8 as the default script encoding?
 * Turning this on will break scripts having non-UTF-8 binary
 * data (such as Latin-1) in string literals. */
#ifdef USE_UTF8_SCRIPTS
#    define USE_UTF8_IN_NAMES (!IN_BYTES)
#else
#    define USE_UTF8_IN_NAMES (PL_hints & HINT_UTF8)
#endif

/* For to_utf8_fold_flags, q.v. */
#define FOLD_FLAGS_LOCALE 0x1
#define FOLD_FLAGS_FULL   0x2

#define to_uni_fold(c, p, lenp) _to_uni_fold_flags(c, p, lenp, 1)
#define to_utf8_fold(c, p, lenp) _to_utf8_fold_flags(c, p, lenp, \
	             FOLD_FLAGS_FULL, NULL)
#define to_utf8_lower(a,b,c) _to_utf8_lower_flags(a,b,c,0, NULL)
#define to_utf8_upper(a,b,c) _to_utf8_upper_flags(a,b,c,0, NULL)
#define to_utf8_title(a,b,c) _to_utf8_title_flags(a,b,c,0, NULL)

/* Source backward compatibility. */
#define uvuni_to_utf8(d, uv)		uvuni_to_utf8_flags(d, uv, 0)
#define is_utf8_string_loc(s, len, ep)	is_utf8_string_loclen(s, len, ep, 0)

#define foldEQ_utf8(s1, pe1, l1, u1, s2, pe2, l2, u2) \
		    foldEQ_utf8_flags(s1, pe1, l1, u1, s2, pe2, l2, u2, 0)
#define FOLDEQ_UTF8_NOMIX_ASCII (1 << 0)
#define FOLDEQ_UTF8_LOCALE      (1 << 1)
#define FOLDEQ_S1_ALREADY_FOLDED  (1 << 2)
#define FOLDEQ_S2_ALREADY_FOLDED  (1 << 3)

/*
=for apidoc ibcmp_utf8

This is a synonym for (! foldEQ_utf8())

=cut
*/
#define ibcmp_utf8(s1, pe1, l1, u1, s2, pe2, l2, u2) \
		    cBOOL(! foldEQ_utf8(s1, pe1, l1, u1, s2, pe2, l2, u2))

#ifdef EBCDIC
/* The equivalent of these macros but implementing UTF-EBCDIC
   are in the following header file:
 */

#include "utfebcdic.h"

#else	/* ! EBCDIC */
START_EXTERN_C

#ifdef DOINIT
EXTCONST unsigned char PL_utf8skip[] = {
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ascii */
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ascii */
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ascii */
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ascii */
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* bogus */
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* bogus */
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* scripts */
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,	 /* cjk etc. */
7,13, /* Perl extended (not official UTF-8).  Up to 72bit allowed (64-bit +
	 reserved). */
};
#else
EXTCONST unsigned char PL_utf8skip[];
#endif

END_EXTERN_C

/* Native character to iso-8859-1 */
#define NATIVE_TO_ASCII(ch)      (ch)
#define ASCII_TO_NATIVE(ch)      (ch)
/* Transform after encoding */
#define NATIVE_TO_UTF(ch)        (ch)
#define UTF_TO_NATIVE(ch)        (ch)
/* Transforms in wide UV chars */
#define UNI_TO_NATIVE(ch)        (ch)
#define NATIVE_TO_UNI(ch)        (ch)
/* Transforms in invariant space */
#define NATIVE_TO_NEED(enc,ch)   (ch)
#define ASCII_TO_NEED(enc,ch)    (ch)

/* As there are no translations, avoid the function wrapper */
#define utf8n_to_uvchr utf8n_to_uvuni
#define uvchr_to_utf8  uvuni_to_utf8

/*

 The following table is from Unicode 3.2.

 Code Points		1st Byte  2nd Byte  3rd Byte  4th Byte

   U+0000..U+007F	00..7F
   U+0080..U+07FF     * C2..DF    80..BF
   U+0800..U+0FFF	E0      * A0..BF    80..BF
   U+1000..U+CFFF       E1..EC    80..BF    80..BF
   U+D000..U+D7FF       ED        80..9F    80..BF
   U+D800..U+DFFF       +++++++ utf16 surrogates, not legal utf8 +++++++
   U+E000..U+FFFF       EE..EF    80..BF    80..BF
  U+10000..U+3FFFF	F0      * 90..BF    80..BF    80..BF
  U+40000..U+FFFFF	F1..F3    80..BF    80..BF    80..BF
 U+100000..U+10FFFF	F4        80..8F    80..BF    80..BF

Note the gaps before several of the byte entries above marked by '*'.  These are
caused by legal UTF-8 avoiding non-shortest encodings: it is technically
possible to UTF-8-encode a single code point in different ways, but that is
explicitly forbidden, and the shortest possible encoding should always be used
(and that is what Perl does).

 */

/*
 Another way to look at it, as bits:

 Code Points                    1st Byte   2nd Byte  3rd Byte  4th Byte

                    0aaaaaaa     0aaaaaaa
            00000bbbbbaaaaaa     110bbbbb  10aaaaaa
            ccccbbbbbbaaaaaa     1110cccc  10bbbbbb  10aaaaaa
  00000dddccccccbbbbbbaaaaaa     11110ddd  10cccccc  10bbbbbb  10aaaaaa

As you can see, the continuation bytes all begin with C<10>, and the
leading bits of the start byte tell how many bytes there are in the
encoded character.

Perl's extended UTF-8 means we can have start bytes up to FF.

*/

#define UNI_IS_INVARIANT(c)		(((UV)c) <  0x80)
#define UTF8_IS_START(c)		(((U8)c) >= 0xc2)
#define UTF8_IS_CONTINUATION(c)		(((U8)c) >= 0x80 && (((U8)c) <= 0xbf))
#define UTF8_IS_CONTINUED(c) 		(((U8)c) &  0x80)

/* Masking with 0xfe allows low bit to be 0 or 1; thus this matches 0xc[23] */
#define UTF8_IS_DOWNGRADEABLE_START(c)	(((U8)c & 0xfe) == 0xc2)

#define UTF_START_MARK(len) (((len) >  7) ? 0xFF : (0xFE << (7-(len))))
#define UTF_START_MASK(len) (((len) >= 7) ? 0x00 : (0x1F >> ((len)-2)))

#define UTF_CONTINUATION_MARK		0x80
#define UTF_ACCUMULATION_SHIFT		6
#define UTF_CONTINUATION_MASK		((U8)0x3f)

/* This sets the UTF_CONTINUATION_MASK in the upper bits of a word.  If a value
 * is anded with it, and the result is non-zero, then using the original value
 * in UTF8_ACCUMULATE will overflow, shifting bits off the left */
#define UTF_ACCUMULATION_OVERFLOW_MASK					\
    (((UV) UTF_CONTINUATION_MASK) << ((sizeof(UV) * CHARBITS) - UTF_ACCUMULATION_SHIFT))

#ifdef HAS_QUAD
#define UNISKIP(uv) ( (uv) < 0x80           ? 1 : \
		      (uv) < 0x800          ? 2 : \
		      (uv) < 0x10000        ? 3 : \
		      (uv) < 0x200000       ? 4 : \
		      (uv) < 0x4000000      ? 5 : \
		      (uv) < 0x80000000     ? 6 : \
                      (uv) < UTF8_QUAD_MAX ? 7 : 13 )
#else
/* No, I'm not even going to *TRY* putting #ifdef inside a #define */
#define UNISKIP(uv) ( (uv) < 0x80           ? 1 : \
		      (uv) < 0x800          ? 2 : \
		      (uv) < 0x10000        ? 3 : \
		      (uv) < 0x200000       ? 4 : \
		      (uv) < 0x4000000      ? 5 : \
		      (uv) < 0x80000000     ? 6 : 7 )
#endif

#endif /* EBCDIC vs ASCII */

/* Rest of these are attributes of Unicode and perl's internals rather than the
 * encoding, or happen to be the same in both ASCII and EBCDIC (at least at
 * this level; the macros that some of these call may have different
 * definitions in the two encodings */

#define NATIVE8_TO_UNI(ch)     NATIVE_TO_ASCII(ch)	/* a clearer synonym */

#define UTF8_ACCUMULATE(old, new)	(((old) << UTF_ACCUMULATION_SHIFT) | (((U8)new) & UTF_CONTINUATION_MASK))

/* Convert a two (not one) byte utf8 character to a unicode code point value.
 * Needs just one iteration of accumulate.  Should not be used unless it is
 * known that the two bytes are legal: 1) two-byte start, and 2) continuation.
 * Note that the result can be larger than 255 if the input character is not
 * downgradable */
#define TWO_BYTE_UTF8_TO_UNI(HI, LO) \
		    UTF8_ACCUMULATE((NATIVE_TO_UTF(HI) & UTF_START_MASK(2)), \
				     NATIVE_TO_UTF(LO))

#define UTF8SKIP(s) PL_utf8skip[*(const U8*)(s)]

#define UTF8_IS_INVARIANT(c)		UNI_IS_INVARIANT(NATIVE_TO_UTF(c))
#define NATIVE_IS_INVARIANT(c)		UNI_IS_INVARIANT(NATIVE8_TO_UNI(c))

#define MAX_PORTABLE_UTF8_TWO_BYTE 0x3FF    /* constrained by EBCDIC */

/* The macros in the next sets are used to generate the two utf8 or utfebcdic
 * bytes from an ordinal that is known to fit into two bytes; it must be less
 * than 0x3FF to work across both encodings. */
/* Nocast allows these to be used in the case label of a switch statement */
#define UTF8_TWO_BYTE_HI_nocast(c)	UTF_TO_NATIVE(((c) >> UTF_ACCUMULATION_SHIFT) | (0xFF & UTF_START_MARK(2)))
#define UTF8_TWO_BYTE_LO_nocast(c)	UTF_TO_NATIVE(((c) & UTF_CONTINUATION_MASK) | UTF_CONTINUATION_MARK)

#define UTF8_TWO_BYTE_HI(c)	((U8) (UTF8_TWO_BYTE_HI_nocast(c)))
#define UTF8_TWO_BYTE_LO(c)	((U8) (UTF8_TWO_BYTE_LO_nocast(c)))

/* This name is used when the source is a single byte */
#define UTF8_EIGHT_BIT_HI(c)	UTF8_TWO_BYTE_HI((U8)(c))
#define UTF8_EIGHT_BIT_LO(c)	UTF8_TWO_BYTE_LO((U8)(c))

/*
 * 'UTF' is whether or not p is encoded in UTF8.  The names 'foo_lazy_if' stem
 * from an earlier version of these macros in which they didn't call the
 * foo_utf8() macros (i.e. were 'lazy') unless they decided that *p is the
 * beginning of a utf8 character.  Now that foo_utf8() determines that itself,
 * no need to do it again here
 */
#define isIDFIRST_lazy_if(p,UTF) ((IN_BYTES || !UTF ) \
				 ? isIDFIRST(*(p)) \
				 : isIDFIRST_utf8((const U8*)p))
#define isALNUM_lazy_if(p,UTF)   ((IN_BYTES || (!UTF )) \
				 ? isALNUM(*(p)) \
				 : isALNUM_utf8((const U8*)p))

#define isIDFIRST_lazy(p)	isIDFIRST_lazy_if(p,1)
#define isALNUM_lazy(p)		isALNUM_lazy_if(p,1)

#define UTF8_MAXBYTES 13
/* How wide can a single UTF-8 encoded character become in bytes.
 * NOTE: Strictly speaking Perl's UTF-8 should not be called UTF-8
 * since UTF-8 is an encoding of Unicode and given Unicode's current
 * upper limit only four bytes is possible.  Perl thinks of UTF-8
 * as a way to encode non-negative integers in a binary format. */
#define UTF8_MAXLEN UTF8_MAXBYTES

/* The maximum number of UTF-8 bytes a single Unicode character can
 * uppercase/lowercase/fold into; this number depends on the Unicode
 * version.  An example of maximal expansion is the U+03B0 which
 * uppercases to U+03C5 U+0308 U+0301.  The Unicode databases that
 * tell these things are UnicodeData.txt, CaseFolding.txt, and
 * SpecialCasing.txt.  The value is 6 for strict Unicode characters, but it has
 * to be as big as Perl allows for a single character */
#define UTF8_MAXBYTES_CASE	UTF8_MAXBYTES

/* A Unicode character can fold to up to 3 characters */
#define UTF8_MAX_FOLD_CHAR_EXPAND 3

#define IN_BYTES (CopHINTS_get(PL_curcop) & HINT_BYTES)
#define DO_UTF8(sv) (SvUTF8(sv) && !IN_BYTES)
#define IN_UNI_8_BIT \
	    (CopHINTS_get(PL_curcop) & (HINT_UNI_8_BIT|HINT_LOCALE_NOT_CHARS) \
	     && ! IN_LOCALE_RUNTIME && ! IN_BYTES)


#define UTF8_ALLOW_EMPTY		0x0001	/* Allow a zero length string */

/* Allow first byte to be a continuation byte */
#define UTF8_ALLOW_CONTINUATION		0x0002

/* Allow second... bytes to be non-continuation bytes */
#define UTF8_ALLOW_NON_CONTINUATION	0x0004

/* expecting more bytes than were available in the string */
#define UTF8_ALLOW_SHORT		0x0008

/* Overlong sequence; i.e., the code point can be specified in fewer bytes. */
#define UTF8_ALLOW_LONG                 0x0010

#define UTF8_DISALLOW_SURROGATE		0x0020	/* Unicode surrogates */
#define UTF8_WARN_SURROGATE		0x0040

#define UTF8_DISALLOW_NONCHAR           0x0080	/* Unicode non-character */
#define UTF8_WARN_NONCHAR               0x0100	/*  code points */

#define UTF8_DISALLOW_SUPER		0x0200	/* Super-set of Unicode: code */
#define UTF8_WARN_SUPER		        0x0400	/* points above the legal max */

/* Code points which never were part of the original UTF-8 standard, the first
 * byte of which is a FE or FF on ASCII platforms. */
#define UTF8_DISALLOW_FE_FF		0x0800
#define UTF8_WARN_FE_FF		        0x1000

#define UTF8_CHECK_ONLY			0x2000

/* For backwards source compatibility.  They do nothing, as the default now
 * includes what they used to mean.  The first one's meaning was to allow the
 * just the single non-character 0xFFFF */
#define UTF8_ALLOW_FFFF 0
#define UTF8_ALLOW_SURROGATE 0

#define UTF8_DISALLOW_ILLEGAL_INTERCHANGE (UTF8_DISALLOW_SUPER|UTF8_DISALLOW_NONCHAR|UTF8_DISALLOW_SURROGATE|UTF8_DISALLOW_FE_FF)
#define UTF8_WARN_ILLEGAL_INTERCHANGE \
	(UTF8_WARN_SUPER|UTF8_WARN_NONCHAR|UTF8_WARN_SURROGATE|UTF8_WARN_FE_FF)
#define UTF8_ALLOW_ANY \
	    (~(UTF8_DISALLOW_ILLEGAL_INTERCHANGE|UTF8_WARN_ILLEGAL_INTERCHANGE))
#define UTF8_ALLOW_ANYUV                                                        \
         (UTF8_ALLOW_EMPTY                                                      \
	  & ~(UTF8_DISALLOW_ILLEGAL_INTERCHANGE|UTF8_WARN_ILLEGAL_INTERCHANGE))
#define UTF8_ALLOW_DEFAULT		(ckWARN(WARN_UTF8) ? 0 : \
					 UTF8_ALLOW_ANYUV)

/* Surrogates, non-character code points and above-Unicode code points are
 * problematic in some contexts.  This allows code that needs to check for
 * those to to quickly exclude the vast majority of code points it will
 * encounter */
#ifdef EBCDIC
#   define UTF8_FIRST_PROBLEMATIC_CODE_POINT_FIRST_BYTE UTF_TO_NATIVE(0xF1)
#else
#   define UTF8_FIRST_PROBLEMATIC_CODE_POINT_FIRST_BYTE 0xED
#endif

/*		ASCII		   EBCDIC I8
 * U+D7FF:   \xED\x9F\xBF	\xF1\xB5\xBF\xBF    last before surrogates
 * U+D800:   \xED\xA0\x80	\xF1\xB6\xA0\xA0    1st surrogate
 * U+DFFF:   \xED\xBF\xBF	\xF1\xB7\xBF\xBF    final surrogate
 * U+E000:   \xEE\x80\x80	\xF1\xB8\xA0\xA0    next after surrogates
 */
#ifdef EBCDIC /* Both versions assume well-formed UTF8 */
#   define UTF8_IS_SURROGATE(s)  (*(s) == UTF_TO_NATIVE(0xF1)                   \
    && ((*((s) +1) == UTF_TO_NATIVE(0xB6)) || *((s) + 1) == UTF_TO_NATIVE(0xB7)))
#else
#   define UTF8_IS_SURROGATE(s) (*(s) == 0xED && *((s) + 1) >= 0xA0)
#endif

/*		  ASCII		     EBCDIC I8
 * U+10FFFF: \xF4\x8F\xBF\xBF	\xF9\xA1\xBF\xBF\xBF	max legal Unicode
 * U+110000: \xF4\x90\x80\x80	\xF9\xA2\xA0\xA0\xA0
 * U+110001: \xF4\x90\x80\x81	\xF9\xA2\xA0\xA0\xA1
 */
#ifdef EBCDIC /* Both versions assume well-formed UTF8 */
#   define UTF8_IS_SUPER(s)  (*(s) >= UTF_TO_NATIVE(0xF9)                       \
      && (*(s) > UTF_TO_NATIVE(0xF9) || (*((s) + 1) >= UTF_TO_NATIVE(0xA2))))
#else
#   define UTF8_IS_SUPER(s)  (*(s) >= 0xF4                                      \
					&& (*(s) > 0xF4 || (*((s) + 1) >= 0x90)))
#endif

/*	   ASCII		     EBCDIC I8
 * U+FDCF: \xEF\xB7\x8F		\xF1\xBF\xAE\xAF	last before non-char block
 * U+FDD0: \xEF\xB7\x90		\xF1\xBF\xAE\xB0	first non-char in block
 * U+FDEF: \xEF\xB7\xAF		\xF1\xBF\xAF\xAF	last non-char in block
 * U+FDF0: \xEF\xB7\xB0		\xF1\xBF\xAF\xB0	first after non-char block
 * U+FFFF: \xEF\xBF\xBF		\xF1\xBF\xBF\xBF
 * U+1FFFF: \xF0\x9F\xBF\xBF	\xF3\xBF\xBF\xBF
 * U+2FFFF: \xF0\xAF\xBF\xBF	\xF5\xBF\xBF\xBF
 * U+3FFFF: \xF0\xBF\xBF\xBF	\xF7\xBF\xBF\xBF
 * U+4FFFF: \xF1\x8F\xBF\xBF	\xF8\xA9\xBF\xBF\xBF
 * U+5FFFF: \xF1\x9F\xBF\xBF	\xF8\xAB\xBF\xBF\xBF
 * U+6FFFF: \xF1\xAF\xBF\xBF	\xF8\xAD\xBF\xBF\xBF
 * U+7FFFF: \xF1\xBF\xBF\xBF	\xF8\xAF\xBF\xBF\xBF
 * U+8FFFF: \xF2\x8F\xBF\xBF	\xF8\xB1\xBF\xBF\xBF
 * U+9FFFF: \xF2\x9F\xBF\xBF	\xF8\xB3\xBF\xBF\xBF
 * U+AFFFF: \xF2\xAF\xBF\xBF	\xF8\xB5\xBF\xBF\xBF
 * U+BFFFF: \xF2\xBF\xBF\xBF	\xF8\xB7\xBF\xBF\xBF
 * U+CFFFF: \xF3\x8F\xBF\xBF	\xF8\xB9\xBF\xBF\xBF
 * U+DFFFF: \xF3\x9F\xBF\xBF	\xF8\xBB\xBF\xBF\xBF
 * U+EFFFF: \xF3\xAF\xBF\xBF	\xF8\xBD\xBF\xBF\xBF
 * U+FFFFF: \xF3\xBF\xBF\xBF	\xF8\xBF\xBF\xBF\xBF
 * U+10FFFF: \xF4\x8F\xBF\xBF	\xF9\xA1\xBF\xBF\xBF
 */
#define UTF8_IS_NONCHAR_(s) (                                                   \
    *(s) >= UTF8_FIRST_PROBLEMATIC_CODE_POINT_FIRST_BYTE                        \
    && ! UTF8_IS_SUPER(s)                                                       \
    && UTF8_IS_NONCHAR_GIVEN_THAT_NON_SUPER_AND_GE_FIRST_PROBLEMATIC(s)         \

#ifdef EBCDIC /* Both versions assume well-formed UTF8 */
#   define UTF8_IS_NONCHAR_GIVEN_THAT_NON_SUPER_AND_GE_PROBLEMATIC(s)           \
    ((*(s) == UTF_TO_NATIVE(0xF1)                                               \
       && (*((s) + 1) == UTF_TO_NATIVE(0xBF)                                    \
       &&    ((*((s) + 2) == UTF_TO_NATIVE(0xAE)                                \
	    && *((s) + 3) >= UTF_TO_NATIVE(0xB0))                               \
	  || (*((s) + 2) == UTF_TO_NATIVE(0xAF)                                 \
	    && *((s) + 3) <= UTF_TO_NATIVE(0xAF)))))                            \
    || (UTF8SKIP(*(s)) > 3                                                      \
	/* (These were all derived by inspection and experimentation with an */ \
	/* editor)  The next line checks the next to final byte in the char */  \
	&& *((s) + UTF8SKIP(*(s)) - 2) == UTF_TO_NATIVE(0xBF)                   \
	&& *((s) + UTF8SKIP(*(s)) - 3) == UTF_TO_NATIVE(0xBF)                   \
        && (NATIVE_TO_UTF(*((s) + UTF8SKIP(*(s)) - 4)) & 0x81) == 0x81          \
        && (NATIVE_TO_UTF(*((s) + UTF8SKIP(*(s)) - 1)) & 0xBE) == 0XBE))
#else
#   define UTF8_IS_NONCHAR_GIVEN_THAT_NON_SUPER_AND_GE_PROBLEMATIC(s)           \
    ((*(s) == 0xEF                                                              \
	&& ((*((s) + 1) == 0xB7 && (*((s) + 2) >= 0x90 && (*((s) + 2) <= 0xAF)))\
		/* Gets U+FFF[EF] */                                            \
	    || (*((s) + 1) == 0xBF && ((*((s) + 2) & 0xBE) == 0xBE))))          \
 || ((*((s) + 2) == 0xBF                                                        \
	 && (*((s) + 3) & 0xBE) == 0xBE                                         \
	    /* Excludes things like U+10FFE = \xF0\x90\xBF\xBE */               \
	 && (*((s) + 1) & 0x8F) == 0x8F)))
#endif

#define UNICODE_SURROGATE_FIRST		0xD800
#define UNICODE_SURROGATE_LAST		0xDFFF
#define UNICODE_REPLACEMENT		0xFFFD
#define UNICODE_BYTE_ORDER_MARK		0xFEFF

/* Though our UTF-8 encoding can go beyond this,
 * let's be conservative and do as Unicode says. */
#define PERL_UNICODE_MAX	0x10FFFF

#define UNICODE_WARN_SURROGATE     0x0001	/* UTF-16 surrogates */
#define UNICODE_WARN_NONCHAR       0x0002	/* Non-char code points */
#define UNICODE_WARN_SUPER         0x0004	/* Above 0x10FFFF */
#define UNICODE_WARN_FE_FF         0x0008	/* Above 0x10FFFF */
#define UNICODE_DISALLOW_SURROGATE 0x0010
#define UNICODE_DISALLOW_NONCHAR   0x0020
#define UNICODE_DISALLOW_SUPER     0x0040
#define UNICODE_DISALLOW_FE_FF     0x0080
#define UNICODE_WARN_ILLEGAL_INTERCHANGE \
    (UNICODE_WARN_SURROGATE|UNICODE_WARN_NONCHAR|UNICODE_WARN_SUPER)
#define UNICODE_DISALLOW_ILLEGAL_INTERCHANGE \
    (UNICODE_DISALLOW_SURROGATE|UNICODE_DISALLOW_NONCHAR|UNICODE_DISALLOW_SUPER)

/* For backward source compatibility, as are now the default */
#define UNICODE_ALLOW_SURROGATE 0
#define UNICODE_ALLOW_SUPER	0
#define UNICODE_ALLOW_ANY	0

#define UNICODE_IS_SURROGATE(c)		((c) >= UNICODE_SURROGATE_FIRST && \
					 (c) <= UNICODE_SURROGATE_LAST)
#define UNICODE_IS_REPLACEMENT(c)	((c) == UNICODE_REPLACEMENT)
#define UNICODE_IS_BYTE_ORDER_MARK(c)	((c) == UNICODE_BYTE_ORDER_MARK)
#define UNICODE_IS_NONCHAR(c)		((c >= 0xFDD0 && c <= 0xFDEF) \
			/* The other noncharacters end in FFFE or FFFF, which  \
			 * the mask below catches both of, but beyond the last \
			 * official unicode code point, they aren't            \
			 * noncharacters, since those aren't Unicode           \
			 * characters at all */                                \
			|| ((((c & 0xFFFE) == 0xFFFE)) && ! UNICODE_IS_SUPER(c)))
#define UNICODE_IS_SUPER(c)		((c) > PERL_UNICODE_MAX)
#define UNICODE_IS_FE_FF(c)		((c) > 0x7FFFFFFF)

#ifdef HAS_QUAD
#    define UTF8_QUAD_MAX	UINT64_C(0x1000000000)
#endif

#define UNICODE_GREEK_CAPITAL_LETTER_SIGMA	0x03A3
#define UNICODE_GREEK_SMALL_LETTER_FINAL_SIGMA	0x03C2
#define UNICODE_GREEK_SMALL_LETTER_SIGMA	0x03C3
#define GREEK_SMALL_LETTER_MU                   0x03BC
#define GREEK_CAPITAL_LETTER_MU 0x039C	/* Upper and title case of MICRON */
#define LATIN_CAPITAL_LETTER_Y_WITH_DIAERESIS 0x0178	/* Also is title case */
#define LATIN_CAPITAL_LETTER_SHARP_S	0x1E9E

#define UNI_DISPLAY_ISPRINT	0x0001
#define UNI_DISPLAY_BACKSLASH	0x0002
#define UNI_DISPLAY_QQ		(UNI_DISPLAY_ISPRINT|UNI_DISPLAY_BACKSLASH)
#define UNI_DISPLAY_REGEX	(UNI_DISPLAY_ISPRINT|UNI_DISPLAY_BACKSLASH)

#ifndef EBCDIC
#   define LATIN_SMALL_LETTER_SHARP_S	0x00DF
#   define LATIN_SMALL_LETTER_Y_WITH_DIAERESIS 0x00FF
#   define MICRO_SIGN 0x00B5
#   define LATIN_CAPITAL_LETTER_A_WITH_RING_ABOVE 0x00C5
#   define LATIN_SMALL_LETTER_A_WITH_RING_ABOVE 0x00E5
#endif

#define ANYOF_FOLD_SHARP_S(node, input, end)	\
	(ANYOF_BITMAP_TEST(node, LATIN_SMALL_LETTER_SHARP_S) && \
	 (ANYOF_NONBITMAP(node)) && \
	 (ANYOF_FLAGS(node) & ANYOF_LOC_NONBITMAP_FOLD) && \
	 ((end) > (input) + 1) && \
	 toLOWER((input)[0]) == 's' && \
	 toLOWER((input)[1]) == 's')
#define SHARP_S_SKIP 2

#ifndef EBCDIC
#   define IS_UTF8_CHAR_1(p)	\
	((p)[0] <= 0x7F)
#   define IS_UTF8_CHAR_2(p)	\
	((p)[0] >= 0xC2 && (p)[0] <= 0xDF && \
	 (p)[1] >= 0x80 && (p)[1] <= 0xBF)
#   define IS_UTF8_CHAR_3a(p)	\
	((p)[0] == 0xE0 && \
	 (p)[1] >= 0xA0 && (p)[1] <= 0xBF && \
	 (p)[2] >= 0x80 && (p)[2] <= 0xBF)
#   define IS_UTF8_CHAR_3b(p)	\
	((p)[0] >= 0xE1 && (p)[0] <= 0xEC && \
	 (p)[1] >= 0x80 && (p)[1] <= 0xBF && \
	 (p)[2] >= 0x80 && (p)[2] <= 0xBF)
#   define IS_UTF8_CHAR_3c(p)	\
	((p)[0] == 0xED && \
	 (p)[1] >= 0x80 && (p)[1] <= 0xBF && \
	 (p)[2] >= 0x80 && (p)[2] <= 0xBF)
    /* In IS_UTF8_CHAR_3c(p) one could use
     * (p)[1] >= 0x80 && (p)[1] <= 0x9F
     * if one wanted to exclude surrogates. */
#   define IS_UTF8_CHAR_3d(p)	\
	((p)[0] >= 0xEE && (p)[0] <= 0xEF && \
	 (p)[1] >= 0x80 && (p)[1] <= 0xBF && \
	 (p)[2] >= 0x80 && (p)[2] <= 0xBF)
#   define IS_UTF8_CHAR_4a(p)	\
	((p)[0] == 0xF0 && \
	 (p)[1] >= 0x90 && (p)[1] <= 0xBF && \
	 (p)[2] >= 0x80 && (p)[2] <= 0xBF && \
	 (p)[3] >= 0x80 && (p)[3] <= 0xBF)
#   define IS_UTF8_CHAR_4b(p)	\
	((p)[0] >= 0xF1 && (p)[0] <= 0xF3 && \
	 (p)[1] >= 0x80 && (p)[1] <= 0xBF && \
	 (p)[2] >= 0x80 && (p)[2] <= 0xBF && \
	 (p)[3] >= 0x80 && (p)[3] <= 0xBF)
/* In IS_UTF8_CHAR_4c(p) one could use
 * (p)[0] == 0xF4
 * if one wanted to stop at the Unicode limit U+10FFFF.
 * The 0xF7 allows us to go to 0x1fffff (0x200000 would
 * require five bytes).  Not doing any further code points
 * since that is not needed (and that would not be strict
 * UTF-8, anyway).  The "slow path" in Perl_is_utf8_char()
 * will take care of the "extended UTF-8". */
#   define IS_UTF8_CHAR_4c(p)	\
	((p)[0] >= 0xF4 && (p)[0] <= 0xF7 && \
	 (p)[1] >= 0x80 && (p)[1] <= 0xBF && \
	 (p)[2] >= 0x80 && (p)[2] <= 0xBF && \
	 (p)[3] >= 0x80 && (p)[3] <= 0xBF)

#   define IS_UTF8_CHAR_3(p)	\
	(IS_UTF8_CHAR_3a(p) || \
	 IS_UTF8_CHAR_3b(p) || \
	 IS_UTF8_CHAR_3c(p) || \
	 IS_UTF8_CHAR_3d(p))
#   define IS_UTF8_CHAR_4(p)	\
	(IS_UTF8_CHAR_4a(p) || \
	 IS_UTF8_CHAR_4b(p) || \
	 IS_UTF8_CHAR_4c(p))

/* IS_UTF8_CHAR(p) is strictly speaking wrong (not UTF-8) because it
 * (1) allows UTF-8 encoded UTF-16 surrogates
 * (2) it allows code points past U+10FFFF.
 * The Perl_is_utf8_char() full "slow" code will handle the Perl
 * "extended UTF-8". */
#   define IS_UTF8_CHAR(p, n)	\
	((n) == 1 ? IS_UTF8_CHAR_1(p) : \
 	 (n) == 2 ? IS_UTF8_CHAR_2(p) : \
	 (n) == 3 ? IS_UTF8_CHAR_3(p) : \
	 (n) == 4 ? IS_UTF8_CHAR_4(p) : 0)

#   define IS_UTF8_CHAR_FAST(n) ((n) <= 4)

#else	/* EBCDIC */

/* This is an attempt to port IS_UTF8_CHAR to EBCDIC based on eyeballing.
 * untested.  If want to exclude surrogates and above-Unicode, see the
 * definitions for UTF8_IS_SURROGATE  and UTF8_IS_SUPER */
#   define IS_UTF8_CHAR_1(p)	\
	(NATIVE_TO_ASCII((p)[0]) <= 0x9F)
#   define IS_UTF8_CHAR_2(p)	\
	(NATIVE_TO_I8((p)[0]) >= 0xC5 && NATIVE_TO_I8((p)[0]) <= 0xDF && \
	 NATIVE_TO_I8((p)[1]) >= 0xA0 && NATIVE_TO_I8((p)[1]) <= 0xBF)
#   define IS_UTF8_CHAR_3(p)	\
	(NATIVE_TO_I8((p)[0]) == 0xE1 && NATIVE_TO_I8((p)[1]) <= 0xEF && \
	 NATIVE_TO_I8((p)[1]) >= 0xA0 && NATIVE_TO_I8((p)[1]) <= 0xBF && \
	 NATIVE_TO_I8((p)[2]) >= 0xA0 && NATIVE_TO_I8((p)[2]) <= 0xBF)
#   define IS_UTF8_CHAR_4a(p)	\
	(NATIVE_TO_I8((p)[0]) == 0xF0 && \
	 NATIVE_TO_I8((p)[1]) >= 0xB0 && NATIVE_TO_I8((p)[1]) <= 0xBF && \
	 NATIVE_TO_I8((p)[2]) >= 0xA0 && NATIVE_TO_I8((p)[2]) <= 0xBF && \
	 NATIVE_TO_I8((p)[3]) >= 0xA0 && NATIVE_TO_I8((p)[3]) <= 0xBF)
#   define IS_UTF8_CHAR_4b(p)	\
	(NATIVE_TO_I8((p)[0]) >= 0xF1 && NATIVE_TO_I8((p)[0]) <= 0xF7 && \
	 NATIVE_TO_I8((p)[1]) >= 0xA0 && NATIVE_TO_I8((p)[1]) <= 0xBF && \
	 NATIVE_TO_I8((p)[2]) >= 0xA0 && NATIVE_TO_I8((p)[2]) <= 0xBF && \
	 NATIVE_TO_I8((p)[3]) >= 0xA0 && NATIVE_TO_I8((p)[3]) <= 0xBF)
#   define IS_UTF8_CHAR_5a(p)	\
	(NATIVE_TO_I8((p)[0]) == 0xF8 && \
	 NATIVE_TO_I8((p)[1]) >= 0xA8 && NATIVE_TO_I8((p)[1]) <= 0xBF && \
	 NATIVE_TO_I8((p)[1]) >= 0xA0 && NATIVE_TO_I8((p)[1]) <= 0xBF && \
	 NATIVE_TO_I8((p)[2]) >= 0xA0 && NATIVE_TO_I8((p)[2]) <= 0xBF && \
	 NATIVE_TO_I8((p)[3]) >= 0xA0 && NATIVE_TO_I8((p)[3]) <= 0xBF)
#   define IS_UTF8_CHAR_5b(p)	\
	 (NATIVE_TO_I8((p)[0]) >= 0xF9 && NATIVE_TO_I8((p)[1]) <= 0xFB && \
	 NATIVE_TO_I8((p)[1]) >= 0xA0 && NATIVE_TO_I8((p)[1]) <= 0xBF && \
	 NATIVE_TO_I8((p)[1]) >= 0xA0 && NATIVE_TO_I8((p)[1]) <= 0xBF && \
	 NATIVE_TO_I8((p)[2]) >= 0xA0 && NATIVE_TO_I8((p)[2]) <= 0xBF && \
	 NATIVE_TO_I8((p)[3]) >= 0xA0 && NATIVE_TO_I8((p)[3]) <= 0xBF)

#   define IS_UTF8_CHAR_4(p)	\
	(IS_UTF8_CHAR_4a(p) || \
	 IS_UTF8_CHAR_4b(p))
#   define IS_UTF8_CHAR_5(p)	\
	(IS_UTF8_CHAR_5a(p) || \
	 IS_UTF8_CHAR_5b(p))
#   define IS_UTF8_CHAR(p, n)	\
	((n) == 1 ? IS_UTF8_CHAR_1(p) : \
	 (n) == 2 ? IS_UTF8_CHAR_2(p) : \
	 (n) == 3 ? IS_UTF8_CHAR_3(p) : \
	 (n) == 4 ? IS_UTF8_CHAR_4(p) : \
	 (n) == 5 ? IS_UTF8_CHAR_5(p) : 0)

#   define IS_UTF8_CHAR_FAST(n) ((n) <= 5)

#endif /* IS_UTF8_CHAR() for UTF-8 */

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 noet:
 */
