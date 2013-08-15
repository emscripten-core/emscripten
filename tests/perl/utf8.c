/*    utf8.c
 *
 *    Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
 *    by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'What a fix!' said Sam.  'That's the one place in all the lands we've ever
 *  heard of that we don't want to see any closer; and that's the one place
 *  we're trying to get to!  And that's just where we can't get, nohow.'
 *
 *     [p.603 of _The Lord of the Rings_, IV/I: "The Taming of Sméagol"]
 *
 * 'Well do I understand your speech,' he answered in the same language;
 * 'yet few strangers do so.  Why then do you not speak in the Common Tongue,
 *  as is the custom in the West, if you wish to be answered?'
 *                           --Gandalf, addressing Théoden's door wardens
 *
 *     [p.508 of _The Lord of the Rings_, III/vi: "The King of the Golden Hall"]
 *
 * ...the travellers perceived that the floor was paved with stones of many
 * hues; branching runes and strange devices intertwined beneath their feet.
 *
 *     [p.512 of _The Lord of the Rings_, III/vi: "The King of the Golden Hall"]
 */

#include "EXTERN.h"
#define PERL_IN_UTF8_C
#include "perl.h"

#ifndef EBCDIC
/* Separate prototypes needed because in ASCII systems these are
 * usually macros but they still are compiled as code, too. */
PERL_CALLCONV UV	Perl_utf8n_to_uvchr(pTHX_ const U8 *s, STRLEN curlen, STRLEN *retlen, U32 flags);
PERL_CALLCONV U8*	Perl_uvchr_to_utf8(pTHX_ U8 *d, UV uv);
#endif

static const char unees[] =
    "Malformed UTF-8 character (unexpected end of string)";

/*
=head1 Unicode Support

This file contains various utility functions for manipulating UTF8-encoded
strings. For the uninitiated, this is a method of representing arbitrary
Unicode characters as a variable number of bytes, in such a way that
characters in the ASCII range are unmodified, and a zero byte never appears
within non-zero characters.

=cut
*/

/*
=for apidoc is_ascii_string

Returns true if the first C<len> bytes of the string C<s> are the same whether
or not the string is encoded in UTF-8 (or UTF-EBCDIC on EBCDIC machines).  That
is, if they are invariant.  On ASCII-ish machines, only ASCII characters
fit this definition, hence the function's name.

If C<len> is 0, it will be calculated using C<strlen(s)>.

See also L</is_utf8_string>(), L</is_utf8_string_loclen>(), and L</is_utf8_string_loc>().

=cut
*/

bool
Perl_is_ascii_string(const U8 *s, STRLEN len)
{
    const U8* const send = s + (len ? len : strlen((const char *)s));
    const U8* x = s;

    PERL_ARGS_ASSERT_IS_ASCII_STRING;

    for (; x < send; ++x) {
	if (!UTF8_IS_INVARIANT(*x))
	    break;
    }

    return x == send;
}

/*
=for apidoc uvuni_to_utf8_flags

Adds the UTF-8 representation of the code point C<uv> to the end
of the string C<d>; C<d> should have at least C<UTF8_MAXBYTES+1> free
bytes available. The return value is the pointer to the byte after the
end of the new character. In other words,

    d = uvuni_to_utf8_flags(d, uv, flags);

or, in most cases,

    d = uvuni_to_utf8(d, uv);

(which is equivalent to)

    d = uvuni_to_utf8_flags(d, uv, 0);

This is the recommended Unicode-aware way of saying

    *(d++) = uv;

This function will convert to UTF-8 (and not warn) even code points that aren't
legal Unicode or are problematic, unless C<flags> contains one or more of the
following flags:

If C<uv> is a Unicode surrogate code point and UNICODE_WARN_SURROGATE is set,
the function will raise a warning, provided UTF8 warnings are enabled.  If instead
UNICODE_DISALLOW_SURROGATE is set, the function will fail and return NULL.
If both flags are set, the function will both warn and return NULL.

The UNICODE_WARN_NONCHAR and UNICODE_DISALLOW_NONCHAR flags correspondingly
affect how the function handles a Unicode non-character.  And, likewise for the
UNICODE_WARN_SUPER and UNICODE_DISALLOW_SUPER flags, and code points that are
above the Unicode maximum of 0x10FFFF.  Code points above 0x7FFF_FFFF (which are
even less portable) can be warned and/or disallowed even if other above-Unicode
code points are accepted by the UNICODE_WARN_FE_FF and UNICODE_DISALLOW_FE_FF
flags.

And finally, the flag UNICODE_WARN_ILLEGAL_INTERCHANGE selects all four of the
above WARN flags; and UNICODE_DISALLOW_ILLEGAL_INTERCHANGE selects all four
DISALLOW flags.


=cut
*/

U8 *
Perl_uvuni_to_utf8_flags(pTHX_ U8 *d, UV uv, UV flags)
{
    PERL_ARGS_ASSERT_UVUNI_TO_UTF8_FLAGS;

    if (ckWARN4_d(WARN_UTF8, WARN_SURROGATE, WARN_NON_UNICODE, WARN_NONCHAR)) {
	if (UNICODE_IS_SURROGATE(uv)) {
	    if (flags & UNICODE_WARN_SURROGATE) {
		Perl_ck_warner_d(aTHX_ packWARN(WARN_SURROGATE),
					    "UTF-16 surrogate U+%04"UVXf, uv);
	    }
	    if (flags & UNICODE_DISALLOW_SURROGATE) {
		return NULL;
	    }
	}
	else if (UNICODE_IS_SUPER(uv)) {
	    if (flags & UNICODE_WARN_SUPER
		|| (UNICODE_IS_FE_FF(uv) && (flags & UNICODE_WARN_FE_FF)))
	    {
		Perl_ck_warner_d(aTHX_ packWARN(WARN_NON_UNICODE),
			  "Code point 0x%04"UVXf" is not Unicode, may not be portable", uv);
	    }
	    if (flags & UNICODE_DISALLOW_SUPER
		|| (UNICODE_IS_FE_FF(uv) && (flags & UNICODE_DISALLOW_FE_FF)))
	    {
		return NULL;
	    }
	}
	else if (UNICODE_IS_NONCHAR(uv)) {
	    if (flags & UNICODE_WARN_NONCHAR) {
		Perl_ck_warner_d(aTHX_ packWARN(WARN_NONCHAR),
		 "Unicode non-character U+%04"UVXf" is illegal for open interchange",
		 uv);
	    }
	    if (flags & UNICODE_DISALLOW_NONCHAR) {
		return NULL;
	    }
	}
    }
    if (UNI_IS_INVARIANT(uv)) {
	*d++ = (U8)UTF_TO_NATIVE(uv);
	return d;
    }
#if defined(EBCDIC)
    else {
	STRLEN len  = UNISKIP(uv);
	U8 *p = d+len-1;
	while (p > d) {
	    *p-- = (U8)UTF_TO_NATIVE((uv & UTF_CONTINUATION_MASK) | UTF_CONTINUATION_MARK);
	    uv >>= UTF_ACCUMULATION_SHIFT;
	}
	*p = (U8)UTF_TO_NATIVE((uv & UTF_START_MASK(len)) | UTF_START_MARK(len));
	return d+len;
    }
#else /* Non loop style */
    if (uv < 0x800) {
	*d++ = (U8)(( uv >>  6)         | 0xc0);
	*d++ = (U8)(( uv        & 0x3f) | 0x80);
	return d;
    }
    if (uv < 0x10000) {
	*d++ = (U8)(( uv >> 12)         | 0xe0);
	*d++ = (U8)(((uv >>  6) & 0x3f) | 0x80);
	*d++ = (U8)(( uv        & 0x3f) | 0x80);
	return d;
    }
    if (uv < 0x200000) {
	*d++ = (U8)(( uv >> 18)         | 0xf0);
	*d++ = (U8)(((uv >> 12) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >>  6) & 0x3f) | 0x80);
	*d++ = (U8)(( uv        & 0x3f) | 0x80);
	return d;
    }
    if (uv < 0x4000000) {
	*d++ = (U8)(( uv >> 24)         | 0xf8);
	*d++ = (U8)(((uv >> 18) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 12) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >>  6) & 0x3f) | 0x80);
	*d++ = (U8)(( uv        & 0x3f) | 0x80);
	return d;
    }
    if (uv < 0x80000000) {
	*d++ = (U8)(( uv >> 30)         | 0xfc);
	*d++ = (U8)(((uv >> 24) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 18) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 12) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >>  6) & 0x3f) | 0x80);
	*d++ = (U8)(( uv        & 0x3f) | 0x80);
	return d;
    }
#ifdef HAS_QUAD
    if (uv < UTF8_QUAD_MAX)
#endif
    {
	*d++ =                            0xfe;	/* Can't match U+FEFF! */
	*d++ = (U8)(((uv >> 30) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 24) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 18) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 12) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >>  6) & 0x3f) | 0x80);
	*d++ = (U8)(( uv        & 0x3f) | 0x80);
	return d;
    }
#ifdef HAS_QUAD
    {
	*d++ =                            0xff;		/* Can't match U+FFFE! */
	*d++ =                            0x80;		/* 6 Reserved bits */
	*d++ = (U8)(((uv >> 60) & 0x0f) | 0x80);	/* 2 Reserved bits */
	*d++ = (U8)(((uv >> 54) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 48) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 42) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 36) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 30) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 24) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 18) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >> 12) & 0x3f) | 0x80);
	*d++ = (U8)(((uv >>  6) & 0x3f) | 0x80);
	*d++ = (U8)(( uv        & 0x3f) | 0x80);
	return d;
    }
#endif
#endif /* Loop style */
}

/*

Tests if the first C<len> bytes of string C<s> form a valid UTF-8
character.  Note that an INVARIANT (i.e. ASCII) character is a valid
UTF-8 character.  The number of bytes in the UTF-8 character
will be returned if it is valid, otherwise 0.

This is the "slow" version as opposed to the "fast" version which is
the "unrolled" IS_UTF8_CHAR().  E.g. for t/uni/class.t the speed
difference is a factor of 2 to 3.  For lengths (UTF8SKIP(s)) of four
or less you should use the IS_UTF8_CHAR(), for lengths of five or more
you should use the _slow().  In practice this means that the _slow()
will be used very rarely, since the maximum Unicode code point (as of
Unicode 4.1) is U+10FFFF, which encodes in UTF-8 to four bytes.  Only
the "Perl extended UTF-8" (the infamous 'v-strings') will encode into
five bytes or more.

=cut */
STATIC STRLEN
S_is_utf8_char_slow(const U8 *s, const STRLEN len)
{
    dTHX;   /* The function called below requires thread context */

    STRLEN actual_len;

    PERL_ARGS_ASSERT_IS_UTF8_CHAR_SLOW;

    utf8n_to_uvuni(s, len, &actual_len, UTF8_CHECK_ONLY);

    return (actual_len == (STRLEN) -1) ? 0 : actual_len;
}

/*
=for apidoc is_utf8_char_buf

Returns the number of bytes that comprise the first UTF-8 encoded character in
buffer C<buf>.  C<buf_end> should point to one position beyond the end of the
buffer.  0 is returned if C<buf> does not point to a complete, valid UTF-8
encoded character.

Note that an INVARIANT character (i.e. ASCII on non-EBCDIC
machines) is a valid UTF-8 character.

=cut */

STRLEN
Perl_is_utf8_char_buf(const U8 *buf, const U8* buf_end)
{

    STRLEN len;

    PERL_ARGS_ASSERT_IS_UTF8_CHAR_BUF;

    if (buf_end <= buf) {
	return 0;
    }

    len = buf_end - buf;
    if (len > UTF8SKIP(buf)) {
	len = UTF8SKIP(buf);
    }

#ifdef IS_UTF8_CHAR
    if (IS_UTF8_CHAR_FAST(len))
        return IS_UTF8_CHAR(buf, len) ? len : 0;
#endif /* #ifdef IS_UTF8_CHAR */
    return is_utf8_char_slow(buf, len);
}

/*
=for apidoc is_utf8_char

DEPRECATED!

Tests if some arbitrary number of bytes begins in a valid UTF-8
character.  Note that an INVARIANT (i.e. ASCII on non-EBCDIC machines)
character is a valid UTF-8 character.  The actual number of bytes in the UTF-8
character will be returned if it is valid, otherwise 0.

This function is deprecated due to the possibility that malformed input could
cause reading beyond the end of the input buffer.  Use L</is_utf8_char_buf>
instead.

=cut */

STRLEN
Perl_is_utf8_char(const U8 *s)
{
    PERL_ARGS_ASSERT_IS_UTF8_CHAR;

    /* Assumes we have enough space, which is why this is deprecated */
    return is_utf8_char_buf(s, s + UTF8SKIP(s));
}


/*
=for apidoc is_utf8_string

Returns true if the first C<len> bytes of string C<s> form a valid
UTF-8 string, false otherwise.  If C<len> is 0, it will be calculated
using C<strlen(s)> (which means if you use this option, that C<s> has to have a
terminating NUL byte).  Note that all characters being ASCII constitute 'a
valid UTF-8 string'.

See also L</is_ascii_string>(), L</is_utf8_string_loclen>(), and L</is_utf8_string_loc>().

=cut
*/

bool
Perl_is_utf8_string(const U8 *s, STRLEN len)
{
    const U8* const send = s + (len ? len : strlen((const char *)s));
    const U8* x = s;

    PERL_ARGS_ASSERT_IS_UTF8_STRING;

    while (x < send) {
	 /* Inline the easy bits of is_utf8_char() here for speed... */
	 if (UTF8_IS_INVARIANT(*x)) {
	    x++;
	 }
	 else if (!UTF8_IS_START(*x))
	     return FALSE;
	 else {
	      /* ... and call is_utf8_char() only if really needed. */
	     const STRLEN c = UTF8SKIP(x);
	     const U8* const next_char_ptr = x + c;

	     if (next_char_ptr > send) {
		 return FALSE;
	     }

	     if (IS_UTF8_CHAR_FAST(c)) {
	         if (!IS_UTF8_CHAR(x, c))
		     return FALSE;
	     }
	     else if (! is_utf8_char_slow(x, c)) {
		 return FALSE;
	     }
	     x = next_char_ptr;
	 }
    }

    return TRUE;
}

/*
Implemented as a macro in utf8.h

=for apidoc is_utf8_string_loc

Like L</is_utf8_string> but stores the location of the failure (in the
case of "utf8ness failure") or the location C<s>+C<len> (in the case of
"utf8ness success") in the C<ep>.

See also L</is_utf8_string_loclen>() and L</is_utf8_string>().

=for apidoc is_utf8_string_loclen

Like L</is_utf8_string>() but stores the location of the failure (in the
case of "utf8ness failure") or the location C<s>+C<len> (in the case of
"utf8ness success") in the C<ep>, and the number of UTF-8
encoded characters in the C<el>.

See also L</is_utf8_string_loc>() and L</is_utf8_string>().

=cut
*/

bool
Perl_is_utf8_string_loclen(const U8 *s, STRLEN len, const U8 **ep, STRLEN *el)
{
    const U8* const send = s + (len ? len : strlen((const char *)s));
    const U8* x = s;
    STRLEN c;
    STRLEN outlen = 0;

    PERL_ARGS_ASSERT_IS_UTF8_STRING_LOCLEN;

    while (x < send) {
	 const U8* next_char_ptr;

	 /* Inline the easy bits of is_utf8_char() here for speed... */
	 if (UTF8_IS_INVARIANT(*x))
	     next_char_ptr = x + 1;
	 else if (!UTF8_IS_START(*x))
	     goto out;
	 else {
	     /* ... and call is_utf8_char() only if really needed. */
	     c = UTF8SKIP(x);
	     next_char_ptr = c + x;
	     if (next_char_ptr > send) {
		 goto out;
	     }
	     if (IS_UTF8_CHAR_FAST(c)) {
	         if (!IS_UTF8_CHAR(x, c))
		     c = 0;
	     } else
	         c = is_utf8_char_slow(x, c);
	     if (!c)
	         goto out;
	 }
         x = next_char_ptr;
	 outlen++;
    }

 out:
    if (el)
        *el = outlen;

    if (ep)
        *ep = x;
    return (x == send);
}

/*

=for apidoc utf8n_to_uvuni

Bottom level UTF-8 decode routine.
Returns the code point value of the first character in the string C<s>,
which is assumed to be in UTF-8 (or UTF-EBCDIC) encoding, and no longer than
C<curlen> bytes; C<*retlen> (if C<retlen> isn't NULL) will be set to
the length, in bytes, of that character.

The value of C<flags> determines the behavior when C<s> does not point to a
well-formed UTF-8 character.  If C<flags> is 0, when a malformation is found,
zero is returned and C<*retlen> is set so that (S<C<s> + C<*retlen>>) is the
next possible position in C<s> that could begin a non-malformed character.
Also, if UTF-8 warnings haven't been lexically disabled, a warning is raised.

Various ALLOW flags can be set in C<flags> to allow (and not warn on)
individual types of malformations, such as the sequence being overlong (that
is, when there is a shorter sequence that can express the same code point;
overlong sequences are expressly forbidden in the UTF-8 standard due to
potential security issues).  Another malformation example is the first byte of
a character not being a legal first byte.  See F<utf8.h> for the list of such
flags.  For allowed 0 length strings, this function returns 0; for allowed
overlong sequences, the computed code point is returned; for all other allowed
malformations, the Unicode REPLACEMENT CHARACTER is returned, as these have no
determinable reasonable value.

The UTF8_CHECK_ONLY flag overrides the behavior when a non-allowed (by other
flags) malformation is found.  If this flag is set, the routine assumes that
the caller will raise a warning, and this function will silently just set
C<retlen> to C<-1> and return zero.

Certain code points are considered problematic.  These are Unicode surrogates,
Unicode non-characters, and code points above the Unicode maximum of 0x10FFFF.
By default these are considered regular code points, but certain situations
warrant special handling for them.  If C<flags> contains
UTF8_DISALLOW_ILLEGAL_INTERCHANGE, all three classes are treated as
malformations and handled as such.  The flags UTF8_DISALLOW_SURROGATE,
UTF8_DISALLOW_NONCHAR, and UTF8_DISALLOW_SUPER (meaning above the legal Unicode
maximum) can be set to disallow these categories individually.

The flags UTF8_WARN_ILLEGAL_INTERCHANGE, UTF8_WARN_SURROGATE,
UTF8_WARN_NONCHAR, and UTF8_WARN_SUPER will cause warning messages to be raised
for their respective categories, but otherwise the code points are considered
valid (not malformations).  To get a category to both be treated as a
malformation and raise a warning, specify both the WARN and DISALLOW flags.
(But note that warnings are not raised if lexically disabled nor if
UTF8_CHECK_ONLY is also specified.)

Very large code points (above 0x7FFF_FFFF) are considered more problematic than
the others that are above the Unicode legal maximum.  There are several
reasons: they requre at least 32 bits to represent them on ASCII platforms, are
not representable at all on EBCDIC platforms, and the original UTF-8
specification never went above this number (the current 0x10FFFF limit was
imposed later).  (The smaller ones, those that fit into 32 bits, are
representable by a UV on ASCII platforms, but not by an IV, which means that
the number of operations that can be performed on them is quite restricted.)
The UTF-8 encoding on ASCII platforms for these large code points begins with a
byte containing 0xFE or 0xFF.  The UTF8_DISALLOW_FE_FF flag will cause them to
be treated as malformations, while allowing smaller above-Unicode code points.
(Of course UTF8_DISALLOW_SUPER will treat all above-Unicode code points,
including these, as malformations.) Similarly, UTF8_WARN_FE_FF acts just like
the other WARN flags, but applies just to these code points.

All other code points corresponding to Unicode characters, including private
use and those yet to be assigned, are never considered malformed and never
warn.

Most code should use L</utf8_to_uvchr_buf>() rather than call this directly.

=cut
*/

UV
Perl_utf8n_to_uvuni(pTHX_ const U8 *s, STRLEN curlen, STRLEN *retlen, U32 flags)
{
    dVAR;
    const U8 * const s0 = s;
    U8 overflow_byte = '\0';	/* Save byte in case of overflow */
    U8 * send;
    UV uv = *s;
    STRLEN expectlen;
    SV* sv = NULL;
    UV outlier_ret = 0;	/* return value when input is in error or problematic
			 */
    UV pack_warn = 0;	/* Save result of packWARN() for later */
    bool unexpected_non_continuation = FALSE;
    bool overflowed = FALSE;
    bool do_overlong_test = TRUE;   /* May have to skip this test */

    const char* const malformed_text = "Malformed UTF-8 character";

    PERL_ARGS_ASSERT_UTF8N_TO_UVUNI;

    /* The order of malformation tests here is important.  We should consume as
     * few bytes as possible in order to not skip any valid character.  This is
     * required by the Unicode Standard (section 3.9 of Unicode 6.0); see also
     * http://unicode.org/reports/tr36 for more discussion as to why.  For
     * example, once we've done a UTF8SKIP, we can tell the expected number of
     * bytes, and could fail right off the bat if the input parameters indicate
     * that there are too few available.  But it could be that just that first
     * byte is garbled, and the intended character occupies fewer bytes.  If we
     * blindly assumed that the first byte is correct, and skipped based on
     * that number, we could skip over a valid input character.  So instead, we
     * always examine the sequence byte-by-byte.
     *
     * We also should not consume too few bytes, otherwise someone could inject
     * things.  For example, an input could be deliberately designed to
     * overflow, and if this code bailed out immediately upon discovering that,
     * returning to the caller *retlen pointing to the very next byte (one
     * which is actually part of of the overflowing sequence), that could look
     * legitimate to the caller, which could discard the initial partial
     * sequence and process the rest, inappropriately */

    /* Zero length strings, if allowed, of necessity are zero */
    if (curlen == 0) {
	if (retlen) {
	    *retlen = 0;
	}

	if (flags & UTF8_ALLOW_EMPTY) {
	    return 0;
	}
	if (! (flags & UTF8_CHECK_ONLY)) {
	    sv = sv_2mortal(Perl_newSVpvf(aTHX_ "%s (empty string)", malformed_text));
	}
	goto malformed;
    }

    expectlen = UTF8SKIP(s);

    /* A well-formed UTF-8 character, as the vast majority of calls to this
     * function will be for, has this expected length.  For efficiency, set
     * things up here to return it.  It will be overriden only in those rare
     * cases where a malformation is found */
    if (retlen) {
	*retlen = expectlen;
    }

    /* An invariant is trivially well-formed */
    if (UTF8_IS_INVARIANT(uv)) {
	return (UV) (NATIVE_TO_UTF(*s));
    }

    /* A continuation character can't start a valid sequence */
    if (UTF8_IS_CONTINUATION(uv)) {
	if (flags & UTF8_ALLOW_CONTINUATION) {
	    if (retlen) {
		*retlen = 1;
	    }
	    return UNICODE_REPLACEMENT;
	}

	if (! (flags & UTF8_CHECK_ONLY)) {
	    sv = sv_2mortal(Perl_newSVpvf(aTHX_ "%s (unexpected continuation byte 0x%02x, with no preceding start byte)", malformed_text, *s0));
	}
	curlen = 1;
	goto malformed;
    }

#ifdef EBCDIC
    uv = NATIVE_TO_UTF(uv);
#endif

    /* Here is not a continuation byte, nor an invariant.  The only thing left
     * is a start byte (possibly for an overlong) */

    /* Remove the leading bits that indicate the number of bytes in the
     * character's whole UTF-8 sequence, leaving just the bits that are part of
     * the value */
    uv &= UTF_START_MASK(expectlen);

    /* Now, loop through the remaining bytes in the character's sequence,
     * accumulating each into the working value as we go.  Be sure to not look
     * past the end of the input string */
    send =  (U8*) s0 + ((expectlen <= curlen) ? expectlen : curlen);

    for (s = s0 + 1; s < send; s++) {
	if (UTF8_IS_CONTINUATION(*s)) {
#ifndef EBCDIC	/* Can't overflow in EBCDIC */
	    if (uv & UTF_ACCUMULATION_OVERFLOW_MASK) {

		/* The original implementors viewed this malformation as more
		 * serious than the others (though I, khw, don't understand
		 * why, since other malformations also give very very wrong
		 * results), so there is no way to turn off checking for it.
		 * Set a flag, but keep going in the loop, so that we absorb
		 * the rest of the bytes that comprise the character. */
		overflowed = TRUE;
		overflow_byte = *s; /* Save for warning message's use */
	    }
#endif
	    uv = UTF8_ACCUMULATE(uv, *s);
	}
	else {
	    /* Here, found a non-continuation before processing all expected
	     * bytes.  This byte begins a new character, so quit, even if
	     * allowing this malformation. */
	    unexpected_non_continuation = TRUE;
	    break;
	}
    } /* End of loop through the character's bytes */

    /* Save how many bytes were actually in the character */
    curlen = s - s0;

    /* The loop above finds two types of malformations: non-continuation and/or
     * overflow.  The non-continuation malformation is really a too-short
     * malformation, as it means that the current character ended before it was
     * expected to (being terminated prematurely by the beginning of the next
     * character, whereas in the too-short malformation there just are too few
     * bytes available to hold the character.  In both cases, the check below
     * that we have found the expected number of bytes would fail if executed.)
     * Thus the non-continuation malformation is really unnecessary, being a
     * subset of the too-short malformation.  But there may be existing
     * applications that are expecting the non-continuation type, so we retain
     * it, and return it in preference to the too-short malformation.  (If this
     * code were being written from scratch, the two types might be collapsed
     * into one.)  I, khw, am also giving priority to returning the
     * non-continuation and too-short malformations over overflow when multiple
     * ones are present.  I don't know of any real reason to prefer one over
     * the other, except that it seems to me that multiple-byte errors trumps
     * errors from a single byte */
    if (unexpected_non_continuation) {
	if (!(flags & UTF8_ALLOW_NON_CONTINUATION)) {
	    if (! (flags & UTF8_CHECK_ONLY)) {
		if (curlen == 1) {
		    sv = sv_2mortal(Perl_newSVpvf(aTHX_ "%s (unexpected non-continuation byte 0x%02x, immediately after start byte 0x%02x)", malformed_text, *s, *s0));
		}
		else {
		    sv = sv_2mortal(Perl_newSVpvf(aTHX_ "%s (unexpected non-continuation byte 0x%02x, %d bytes after start byte 0x%02x, expected %d bytes)", malformed_text, *s, (int) curlen, *s0, (int)expectlen));
		}
	    }
	    goto malformed;
	}
	uv = UNICODE_REPLACEMENT;

	/* Skip testing for overlongs, as the REPLACEMENT may not be the same
	 * as what the original expectations were. */
	do_overlong_test = FALSE;
	if (retlen) {
	    *retlen = curlen;
	}
    }
    else if (curlen < expectlen) {
	if (! (flags & UTF8_ALLOW_SHORT)) {
	    if (! (flags & UTF8_CHECK_ONLY)) {
		sv = sv_2mortal(Perl_newSVpvf(aTHX_ "%s (%d byte%s, need %d, after start byte 0x%02x)", malformed_text, (int)curlen, curlen == 1 ? "" : "s", (int)expectlen, *s0));
	    }
	    goto malformed;
	}
	uv = UNICODE_REPLACEMENT;
	do_overlong_test = FALSE;
	if (retlen) {
	    *retlen = curlen;
	}
    }

#ifndef EBCDIC	/* EBCDIC allows FE, FF, can't overflow */
    if ((*s0 & 0xFE) == 0xFE	/* matches both FE, FF */
	&& (flags & (UTF8_WARN_FE_FF|UTF8_DISALLOW_FE_FF)))
    {
	/* By adding UTF8_CHECK_ONLY to the test, we avoid unnecessary
	 * generation of the sv, since no warnings are raised under CHECK */
	if ((flags & (UTF8_WARN_FE_FF|UTF8_CHECK_ONLY)) == UTF8_WARN_FE_FF
	    && ckWARN_d(WARN_UTF8))
	{
	    sv = sv_2mortal(Perl_newSVpvf(aTHX_ "%s Code point beginning with byte 0x%02X is not Unicode, and not portable", malformed_text, *s0));
	    pack_warn = packWARN(WARN_UTF8);
	}
	if (flags & UTF8_DISALLOW_FE_FF) {
	    goto malformed;
	}
    }
    if (overflowed) {

	/* If the first byte is FF, it will overflow a 32-bit word.  If the
	 * first byte is FE, it will overflow a signed 32-bit word.  The
	 * above preserves backward compatibility, since its message was used
	 * in earlier versions of this code in preference to overflow */
	sv = sv_2mortal(Perl_newSVpvf(aTHX_ "%s (overflow at byte 0x%02x, after start byte 0x%02x)", malformed_text, overflow_byte, *s0));
	goto malformed;
    }
#endif

    if (do_overlong_test
	&& expectlen > (STRLEN)UNISKIP(uv)
	&& ! (flags & UTF8_ALLOW_LONG))
    {
	/* The overlong malformation has lower precedence than the others.
	 * Note that if this malformation is allowed, we return the actual
	 * value, instead of the replacement character.  This is because this
	 * value is actually well-defined. */
	if (! (flags & UTF8_CHECK_ONLY)) {
	    sv = sv_2mortal(Perl_newSVpvf(aTHX_ "%s (%d byte%s, need %d, after start byte 0x%02x)", malformed_text, (int)expectlen, expectlen == 1 ? "": "s", UNISKIP(uv), *s0));
	}
	goto malformed;
    }

    /* Here, the input is considered to be well-formed , but could be a
     * problematic code point that is not allowed by the input parameters. */
    if (uv >= UNICODE_SURROGATE_FIRST /* isn't problematic if < this */
	&& (flags & (UTF8_DISALLOW_ILLEGAL_INTERCHANGE
		     |UTF8_WARN_ILLEGAL_INTERCHANGE)))
    {
	if (UNICODE_IS_SURROGATE(uv)) {
	    if ((flags & (UTF8_WARN_SURROGATE|UTF8_CHECK_ONLY)) == UTF8_WARN_SURROGATE
		&& ckWARN2_d(WARN_UTF8, WARN_SURROGATE))
	    {
		sv = sv_2mortal(Perl_newSVpvf(aTHX_ "UTF-16 surrogate U+%04"UVXf"", uv));
		pack_warn = packWARN2(WARN_UTF8, WARN_SURROGATE);
	    }
	    if (flags & UTF8_DISALLOW_SURROGATE) {
		goto disallowed;
	    }
	}
	else if (UNICODE_IS_NONCHAR(uv)) {
	    if ((flags & (UTF8_WARN_NONCHAR|UTF8_CHECK_ONLY)) == UTF8_WARN_NONCHAR
		&& ckWARN2_d(WARN_UTF8, WARN_NONCHAR))
	    {
		sv = sv_2mortal(Perl_newSVpvf(aTHX_ "Unicode non-character U+%04"UVXf" is illegal for open interchange", uv));
		pack_warn = packWARN2(WARN_UTF8, WARN_NONCHAR);
	    }
	    if (flags & UTF8_DISALLOW_NONCHAR) {
		goto disallowed;
	    }
	}
	else if ((uv > PERL_UNICODE_MAX)) {
	    if ((flags & (UTF8_WARN_SUPER|UTF8_CHECK_ONLY)) == UTF8_WARN_SUPER
		&& ckWARN2_d(WARN_UTF8, WARN_NON_UNICODE))
	    {
		sv = sv_2mortal(Perl_newSVpvf(aTHX_ "Code point 0x%04"UVXf" is not Unicode, may not be portable", uv));
		pack_warn = packWARN2(WARN_UTF8, WARN_NON_UNICODE);
	    }
	    if (flags & UTF8_DISALLOW_SUPER) {
		goto disallowed;
	    }
	}

	if (sv) {
	    outlier_ret = uv;
	    goto do_warn;
	}

	/* Here, this is not considered a malformed character, so drop through
	 * to return it */
    }

    return uv;

    /* There are three cases which get to beyond this point.  In all 3 cases:
     * <sv>	    if not null points to a string to print as a warning.
     * <curlen>	    is what <*retlen> should be set to if UTF8_CHECK_ONLY isn't
     *		    set.
     * <outlier_ret> is what return value to use if UTF8_CHECK_ONLY isn't set.
     *		    This is done by initializing it to 0, and changing it only
     *		    for case 1).
     * The 3 cases are:
     * 1)   The input is valid but problematic, and to be warned about.  The
     *	    return value is the resultant code point; <*retlen> is set to
     *	    <curlen>, the number of bytes that comprise the code point.
     *	    <pack_warn> contains the result of packWARN() for the warning
     *	    types.  The entry point for this case is the label <do_warn>;
     * 2)   The input is a valid code point but disallowed by the parameters to
     *	    this function.  The return value is 0.  If UTF8_CHECK_ONLY is set,
     *	    <*relen> is -1; otherwise it is <curlen>, the number of bytes that
     *	    comprise the code point.  <pack_warn> contains the result of
     *	    packWARN() for the warning types.  The entry point for this case is
     *	    the label <disallowed>.
     * 3)   The input is malformed.  The return value is 0.  If UTF8_CHECK_ONLY
     *	    is set, <*relen> is -1; otherwise it is <curlen>, the number of
     *	    bytes that comprise the malformation.  All such malformations are
     *	    assumed to be warning type <utf8>.  The entry point for this case
     *	    is the label <malformed>.
     */

malformed:

    if (sv && ckWARN_d(WARN_UTF8)) {
	pack_warn = packWARN(WARN_UTF8);
    }

disallowed:

    if (flags & UTF8_CHECK_ONLY) {
	if (retlen)
	    *retlen = ((STRLEN) -1);
	return 0;
    }

do_warn:

    if (pack_warn) {	/* <pack_warn> was initialized to 0, and changed only
			   if warnings are to be raised. */
	const char * const string = SvPVX_const(sv);

	if (PL_op)
	    Perl_warner(aTHX_ pack_warn, "%s in %s", string,  OP_DESC(PL_op));
	else
	    Perl_warner(aTHX_ pack_warn, "%s", string);
    }

    if (retlen) {
	*retlen = curlen;
    }

    return outlier_ret;
}

/*
=for apidoc utf8_to_uvchr_buf

Returns the native code point of the first character in the string C<s> which
is assumed to be in UTF-8 encoding; C<send> points to 1 beyond the end of C<s>.
C<*retlen> will be set to the length, in bytes, of that character.

If C<s> does not point to a well-formed UTF-8 character and UTF8 warnings are
enabled, zero is returned and C<*retlen> is set (if C<retlen> isn't
NULL) to -1.  If those warnings are off, the computed value if well-defined (or
the Unicode REPLACEMENT CHARACTER, if not) is silently returned, and C<*retlen>
is set (if C<retlen> isn't NULL) so that (S<C<s> + C<*retlen>>) is the
next possible position in C<s> that could begin a non-malformed character.
See L</utf8n_to_uvuni> for details on when the REPLACEMENT CHARACTER is returned.

=cut
*/


UV
Perl_utf8_to_uvchr_buf(pTHX_ const U8 *s, const U8 *send, STRLEN *retlen)
{
    PERL_ARGS_ASSERT_UTF8_TO_UVCHR_BUF;

    assert(s < send);

    return utf8n_to_uvchr(s, send - s, retlen,
			  ckWARN_d(WARN_UTF8) ? 0 : UTF8_ALLOW_ANY);
}

/* Like L</utf8_to_uvchr_buf>(), but should only be called when it is known that
 * there are no malformations in the input UTF-8 string C<s>.  Currently, some
 * malformations are checked for, but this checking likely will be removed in
 * the future */

UV
Perl_valid_utf8_to_uvchr(pTHX_ const U8 *s, STRLEN *retlen)
{
    PERL_ARGS_ASSERT_VALID_UTF8_TO_UVCHR;

    return utf8_to_uvchr_buf(s, s + UTF8_MAXBYTES, retlen);
}

/*
=for apidoc utf8_to_uvchr

DEPRECATED!

Returns the native code point of the first character in the string C<s>
which is assumed to be in UTF-8 encoding; C<retlen> will be set to the
length, in bytes, of that character.

Some, but not all, UTF-8 malformations are detected, and in fact, some
malformed input could cause reading beyond the end of the input buffer, which
is why this function is deprecated.  Use L</utf8_to_uvchr_buf> instead.

If C<s> points to one of the detected malformations, and UTF8 warnings are
enabled, zero is returned and C<*retlen> is set (if C<retlen> isn't
NULL) to -1.  If those warnings are off, the computed value if well-defined (or
the Unicode REPLACEMENT CHARACTER, if not) is silently returned, and C<*retlen>
is set (if C<retlen> isn't NULL) so that (S<C<s> + C<*retlen>>) is the
next possible position in C<s> that could begin a non-malformed character.
See L</utf8n_to_uvuni> for details on when the REPLACEMENT CHARACTER is returned.

=cut
*/

UV
Perl_utf8_to_uvchr(pTHX_ const U8 *s, STRLEN *retlen)
{
    PERL_ARGS_ASSERT_UTF8_TO_UVCHR;

    return valid_utf8_to_uvchr(s, retlen);
}

/*
=for apidoc utf8_to_uvuni_buf

Returns the Unicode code point of the first character in the string C<s> which
is assumed to be in UTF-8 encoding; C<send> points to 1 beyond the end of C<s>.
C<retlen> will be set to the length, in bytes, of that character.

This function should only be used when the returned UV is considered
an index into the Unicode semantic tables (e.g. swashes).

If C<s> does not point to a well-formed UTF-8 character and UTF8 warnings are
enabled, zero is returned and C<*retlen> is set (if C<retlen> isn't
NULL) to -1.  If those warnings are off, the computed value if well-defined (or
the Unicode REPLACEMENT CHARACTER, if not) is silently returned, and C<*retlen>
is set (if C<retlen> isn't NULL) so that (S<C<s> + C<*retlen>>) is the
next possible position in C<s> that could begin a non-malformed character.
See L</utf8n_to_uvuni> for details on when the REPLACEMENT CHARACTER is returned.

=cut
*/

UV
Perl_utf8_to_uvuni_buf(pTHX_ const U8 *s, const U8 *send, STRLEN *retlen)
{
    PERL_ARGS_ASSERT_UTF8_TO_UVUNI_BUF;

    assert(send > s);

    /* Call the low level routine asking for checks */
    return Perl_utf8n_to_uvuni(aTHX_ s, send -s, retlen,
			       ckWARN_d(WARN_UTF8) ? 0 : UTF8_ALLOW_ANY);
}

/* Like L</utf8_to_uvuni_buf>(), but should only be called when it is known that
 * there are no malformations in the input UTF-8 string C<s>.  Currently, some
 * malformations are checked for, but this checking likely will be removed in
 * the future */

UV
Perl_valid_utf8_to_uvuni(pTHX_ const U8 *s, STRLEN *retlen)
{
    PERL_ARGS_ASSERT_VALID_UTF8_TO_UVUNI;

    return utf8_to_uvuni_buf(s, s + UTF8_MAXBYTES, retlen);
}

/*
=for apidoc utf8_to_uvuni

DEPRECATED!

Returns the Unicode code point of the first character in the string C<s>
which is assumed to be in UTF-8 encoding; C<retlen> will be set to the
length, in bytes, of that character.

This function should only be used when the returned UV is considered
an index into the Unicode semantic tables (e.g. swashes).

Some, but not all, UTF-8 malformations are detected, and in fact, some
malformed input could cause reading beyond the end of the input buffer, which
is why this function is deprecated.  Use L</utf8_to_uvuni_buf> instead.

If C<s> points to one of the detected malformations, and UTF8 warnings are
enabled, zero is returned and C<*retlen> is set (if C<retlen> doesn't point to
NULL) to -1.  If those warnings are off, the computed value if well-defined (or
the Unicode REPLACEMENT CHARACTER, if not) is silently returned, and C<*retlen>
is set (if C<retlen> isn't NULL) so that (S<C<s> + C<*retlen>>) is the
next possible position in C<s> that could begin a non-malformed character.
See L</utf8n_to_uvuni> for details on when the REPLACEMENT CHARACTER is returned.

=cut
*/

UV
Perl_utf8_to_uvuni(pTHX_ const U8 *s, STRLEN *retlen)
{
    PERL_ARGS_ASSERT_UTF8_TO_UVUNI;

    return valid_utf8_to_uvuni(s, retlen);
}

/*
=for apidoc utf8_length

Return the length of the UTF-8 char encoded string C<s> in characters.
Stops at C<e> (inclusive).  If C<e E<lt> s> or if the scan would end
up past C<e>, croaks.

=cut
*/

STRLEN
Perl_utf8_length(pTHX_ const U8 *s, const U8 *e)
{
    dVAR;
    STRLEN len = 0;

    PERL_ARGS_ASSERT_UTF8_LENGTH;

    /* Note: cannot use UTF8_IS_...() too eagerly here since e.g.
     * the bitops (especially ~) can create illegal UTF-8.
     * In other words: in Perl UTF-8 is not just for Unicode. */

    if (e < s)
	goto warn_and_return;
    while (s < e) {
	if (!UTF8_IS_INVARIANT(*s))
	    s += UTF8SKIP(s);
	else
	    s++;
	len++;
    }

    if (e != s) {
	len--;
        warn_and_return:
	if (PL_op)
	    Perl_ck_warner_d(aTHX_ packWARN(WARN_UTF8),
			     "%s in %s", unees, OP_DESC(PL_op));
	else
	    Perl_ck_warner_d(aTHX_ packWARN(WARN_UTF8), "%s", unees);
    }

    return len;
}

/*
=for apidoc utf8_distance

Returns the number of UTF-8 characters between the UTF-8 pointers C<a>
and C<b>.

WARNING: use only if you *know* that the pointers point inside the
same UTF-8 buffer.

=cut
*/

IV
Perl_utf8_distance(pTHX_ const U8 *a, const U8 *b)
{
    PERL_ARGS_ASSERT_UTF8_DISTANCE;

    return (a < b) ? -1 * (IV) utf8_length(a, b) : (IV) utf8_length(b, a);
}

/*
=for apidoc utf8_hop

Return the UTF-8 pointer C<s> displaced by C<off> characters, either
forward or backward.

WARNING: do not use the following unless you *know* C<off> is within
the UTF-8 data pointed to by C<s> *and* that on entry C<s> is aligned
on the first byte of character or just after the last byte of a character.

=cut
*/

U8 *
Perl_utf8_hop(pTHX_ const U8 *s, I32 off)
{
    PERL_ARGS_ASSERT_UTF8_HOP;

    PERL_UNUSED_CONTEXT;
    /* Note: cannot use UTF8_IS_...() too eagerly here since e.g
     * the bitops (especially ~) can create illegal UTF-8.
     * In other words: in Perl UTF-8 is not just for Unicode. */

    if (off >= 0) {
	while (off--)
	    s += UTF8SKIP(s);
    }
    else {
	while (off++) {
	    s--;
	    while (UTF8_IS_CONTINUATION(*s))
		s--;
	}
    }
    return (U8 *)s;
}

/*
=for apidoc bytes_cmp_utf8

Compares the sequence of characters (stored as octets) in C<b>, C<blen> with the
sequence of characters (stored as UTF-8) in C<u>, C<ulen>. Returns 0 if they are
equal, -1 or -2 if the first string is less than the second string, +1 or +2
if the first string is greater than the second string.

-1 or +1 is returned if the shorter string was identical to the start of the
longer string. -2 or +2 is returned if the was a difference between characters
within the strings.

=cut
*/

int
Perl_bytes_cmp_utf8(pTHX_ const U8 *b, STRLEN blen, const U8 *u, STRLEN ulen)
{
    const U8 *const bend = b + blen;
    const U8 *const uend = u + ulen;

    PERL_ARGS_ASSERT_BYTES_CMP_UTF8;

    PERL_UNUSED_CONTEXT;

    while (b < bend && u < uend) {
        U8 c = *u++;
	if (!UTF8_IS_INVARIANT(c)) {
	    if (UTF8_IS_DOWNGRADEABLE_START(c)) {
		if (u < uend) {
		    U8 c1 = *u++;
		    if (UTF8_IS_CONTINUATION(c1)) {
			c = UNI_TO_NATIVE(TWO_BYTE_UTF8_TO_UNI(c, c1));
		    } else {
			Perl_ck_warner_d(aTHX_ packWARN(WARN_UTF8),
					 "Malformed UTF-8 character "
					 "(unexpected non-continuation byte 0x%02x"
					 ", immediately after start byte 0x%02x)"
					 /* Dear diag.t, it's in the pod.  */
					 "%s%s", c1, c,
					 PL_op ? " in " : "",
					 PL_op ? OP_DESC(PL_op) : "");
			return -2;
		    }
		} else {
		    if (PL_op)
			Perl_ck_warner_d(aTHX_ packWARN(WARN_UTF8),
					 "%s in %s", unees, OP_DESC(PL_op));
		    else
			Perl_ck_warner_d(aTHX_ packWARN(WARN_UTF8), "%s", unees);
		    return -2; /* Really want to return undef :-)  */
		}
	    } else {
		return -2;
	    }
	}
	if (*b != c) {
	    return *b < c ? -2 : +2;
	}
	++b;
    }

    if (b == bend && u == uend)
	return 0;

    return b < bend ? +1 : -1;
}

/*
=for apidoc utf8_to_bytes

Converts a string C<s> of length C<len> from UTF-8 into native byte encoding.
Unlike L</bytes_to_utf8>, this over-writes the original string, and
updates C<len> to contain the new length.
Returns zero on failure, setting C<len> to -1.

If you need a copy of the string, see L</bytes_from_utf8>.

=cut
*/

U8 *
Perl_utf8_to_bytes(pTHX_ U8 *s, STRLEN *len)
{
    U8 * const save = s;
    U8 * const send = s + *len;
    U8 *d;

    PERL_ARGS_ASSERT_UTF8_TO_BYTES;

    /* ensure valid UTF-8 and chars < 256 before updating string */
    while (s < send) {
        U8 c = *s++;

        if (!UTF8_IS_INVARIANT(c) &&
            (!UTF8_IS_DOWNGRADEABLE_START(c) || (s >= send)
	     || !(c = *s++) || !UTF8_IS_CONTINUATION(c))) {
            *len = ((STRLEN) -1);
            return 0;
        }
    }

    d = s = save;
    while (s < send) {
        STRLEN ulen;
        *d++ = (U8)utf8_to_uvchr_buf(s, send, &ulen);
        s += ulen;
    }
    *d = '\0';
    *len = d - save;
    return save;
}

/*
=for apidoc bytes_from_utf8

Converts a string C<s> of length C<len> from UTF-8 into native byte encoding.
Unlike L</utf8_to_bytes> but like L</bytes_to_utf8>, returns a pointer to
the newly-created string, and updates C<len> to contain the new
length.  Returns the original string if no conversion occurs, C<len>
is unchanged. Do nothing if C<is_utf8> points to 0. Sets C<is_utf8> to
0 if C<s> is converted or consisted entirely of characters that are invariant
in utf8 (i.e., US-ASCII on non-EBCDIC machines).

=cut
*/

U8 *
Perl_bytes_from_utf8(pTHX_ const U8 *s, STRLEN *len, bool *is_utf8)
{
    U8 *d;
    const U8 *start = s;
    const U8 *send;
    I32 count = 0;

    PERL_ARGS_ASSERT_BYTES_FROM_UTF8;

    PERL_UNUSED_CONTEXT;
    if (!*is_utf8)
        return (U8 *)start;

    /* ensure valid UTF-8 and chars < 256 before converting string */
    for (send = s + *len; s < send;) {
        U8 c = *s++;
	if (!UTF8_IS_INVARIANT(c)) {
	    if (UTF8_IS_DOWNGRADEABLE_START(c) && s < send &&
                (c = *s++) && UTF8_IS_CONTINUATION(c))
		count++;
	    else
                return (U8 *)start;
	}
    }

    *is_utf8 = FALSE;

    Newx(d, (*len) - count + 1, U8);
    s = start; start = d;
    while (s < send) {
	U8 c = *s++;
	if (!UTF8_IS_INVARIANT(c)) {
	    /* Then it is two-byte encoded */
	    c = UNI_TO_NATIVE(TWO_BYTE_UTF8_TO_UNI(c, *s++));
	}
	*d++ = c;
    }
    *d = '\0';
    *len = d - start;
    return (U8 *)start;
}

/*
=for apidoc bytes_to_utf8

Converts a string C<s> of length C<len> bytes from the native encoding into
UTF-8.
Returns a pointer to the newly-created string, and sets C<len> to
reflect the new length in bytes.

A NUL character will be written after the end of the string.

If you want to convert to UTF-8 from encodings other than
the native (Latin1 or EBCDIC),
see L</sv_recode_to_utf8>().

=cut
*/

/* This logic is duplicated in sv_catpvn_flags, so any bug fixes will
   likewise need duplication. */

U8*
Perl_bytes_to_utf8(pTHX_ const U8 *s, STRLEN *len)
{
    const U8 * const send = s + (*len);
    U8 *d;
    U8 *dst;

    PERL_ARGS_ASSERT_BYTES_TO_UTF8;
    PERL_UNUSED_CONTEXT;

    Newx(d, (*len) * 2 + 1, U8);
    dst = d;

    while (s < send) {
        const UV uv = NATIVE_TO_ASCII(*s++);
        if (UNI_IS_INVARIANT(uv))
            *d++ = (U8)UTF_TO_NATIVE(uv);
        else {
            *d++ = (U8)UTF8_EIGHT_BIT_HI(uv);
            *d++ = (U8)UTF8_EIGHT_BIT_LO(uv);
        }
    }
    *d = '\0';
    *len = d-dst;
    return dst;
}

/*
 * Convert native (big-endian) or reversed (little-endian) UTF-16 to UTF-8.
 *
 * Destination must be pre-extended to 3/2 source.  Do not use in-place.
 * We optimize for native, for obvious reasons. */

U8*
Perl_utf16_to_utf8(pTHX_ U8* p, U8* d, I32 bytelen, I32 *newlen)
{
    U8* pend;
    U8* dstart = d;

    PERL_ARGS_ASSERT_UTF16_TO_UTF8;

    if (bytelen & 1)
	Perl_croak(aTHX_ "panic: utf16_to_utf8: odd bytelen %"UVuf, (UV)bytelen);

    pend = p + bytelen;

    while (p < pend) {
	UV uv = (p[0] << 8) + p[1]; /* UTF-16BE */
	p += 2;
	if (uv < 0x80) {
#ifdef EBCDIC
	    *d++ = UNI_TO_NATIVE(uv);
#else
	    *d++ = (U8)uv;
#endif
	    continue;
	}
	if (uv < 0x800) {
	    *d++ = (U8)(( uv >>  6)         | 0xc0);
	    *d++ = (U8)(( uv        & 0x3f) | 0x80);
	    continue;
	}
	if (uv >= 0xd800 && uv <= 0xdbff) {	/* surrogates */
	    if (p >= pend) {
		Perl_croak(aTHX_ "Malformed UTF-16 surrogate");
	    } else {
		UV low = (p[0] << 8) + p[1];
		p += 2;
		if (low < 0xdc00 || low > 0xdfff)
		    Perl_croak(aTHX_ "Malformed UTF-16 surrogate");
		uv = ((uv - 0xd800) << 10) + (low - 0xdc00) + 0x10000;
	    }
	} else if (uv >= 0xdc00 && uv <= 0xdfff) {
	    Perl_croak(aTHX_ "Malformed UTF-16 surrogate");
	}
	if (uv < 0x10000) {
	    *d++ = (U8)(( uv >> 12)         | 0xe0);
	    *d++ = (U8)(((uv >>  6) & 0x3f) | 0x80);
	    *d++ = (U8)(( uv        & 0x3f) | 0x80);
	    continue;
	}
	else {
	    *d++ = (U8)(( uv >> 18)         | 0xf0);
	    *d++ = (U8)(((uv >> 12) & 0x3f) | 0x80);
	    *d++ = (U8)(((uv >>  6) & 0x3f) | 0x80);
	    *d++ = (U8)(( uv        & 0x3f) | 0x80);
	    continue;
	}
    }
    *newlen = d - dstart;
    return d;
}

/* Note: this one is slightly destructive of the source. */

U8*
Perl_utf16_to_utf8_reversed(pTHX_ U8* p, U8* d, I32 bytelen, I32 *newlen)
{
    U8* s = (U8*)p;
    U8* const send = s + bytelen;

    PERL_ARGS_ASSERT_UTF16_TO_UTF8_REVERSED;

    if (bytelen & 1)
	Perl_croak(aTHX_ "panic: utf16_to_utf8_reversed: odd bytelen %"UVuf,
		   (UV)bytelen);

    while (s < send) {
	const U8 tmp = s[0];
	s[0] = s[1];
	s[1] = tmp;
	s += 2;
    }
    return utf16_to_utf8(p, d, bytelen, newlen);
}

/* for now these are all defined (inefficiently) in terms of the utf8 versions.
 * Note that the macros in handy.h that call these short-circuit calling them
 * for Latin-1 range inputs */

bool
Perl_is_uni_alnum(pTHX_ UV c)
{
    U8 tmpbuf[UTF8_MAXBYTES+1];
    uvchr_to_utf8(tmpbuf, c);
    return is_utf8_alnum(tmpbuf);
}

bool
Perl_is_uni_idfirst(pTHX_ UV c)
{
    U8 tmpbuf[UTF8_MAXBYTES+1];
    uvchr_to_utf8(tmpbuf, c);
    return is_utf8_idfirst(tmpbuf);
}

bool
Perl_is_uni_alpha(pTHX_ UV c)
{
    U8 tmpbuf[UTF8_MAXBYTES+1];
    uvchr_to_utf8(tmpbuf, c);
    return is_utf8_alpha(tmpbuf);
}

bool
Perl_is_uni_ascii(pTHX_ UV c)
{
    return isASCII(c);
}

bool
Perl_is_uni_space(pTHX_ UV c)
{
    U8 tmpbuf[UTF8_MAXBYTES+1];
    uvchr_to_utf8(tmpbuf, c);
    return is_utf8_space(tmpbuf);
}

bool
Perl_is_uni_digit(pTHX_ UV c)
{
    U8 tmpbuf[UTF8_MAXBYTES+1];
    uvchr_to_utf8(tmpbuf, c);
    return is_utf8_digit(tmpbuf);
}

bool
Perl_is_uni_upper(pTHX_ UV c)
{
    U8 tmpbuf[UTF8_MAXBYTES+1];
    uvchr_to_utf8(tmpbuf, c);
    return is_utf8_upper(tmpbuf);
}

bool
Perl_is_uni_lower(pTHX_ UV c)
{
    U8 tmpbuf[UTF8_MAXBYTES+1];
    uvchr_to_utf8(tmpbuf, c);
    return is_utf8_lower(tmpbuf);
}

bool
Perl_is_uni_cntrl(pTHX_ UV c)
{
    return isCNTRL_L1(c);
}

bool
Perl_is_uni_graph(pTHX_ UV c)
{
    U8 tmpbuf[UTF8_MAXBYTES+1];
    uvchr_to_utf8(tmpbuf, c);
    return is_utf8_graph(tmpbuf);
}

bool
Perl_is_uni_print(pTHX_ UV c)
{
    U8 tmpbuf[UTF8_MAXBYTES+1];
    uvchr_to_utf8(tmpbuf, c);
    return is_utf8_print(tmpbuf);
}

bool
Perl_is_uni_punct(pTHX_ UV c)
{
    U8 tmpbuf[UTF8_MAXBYTES+1];
    uvchr_to_utf8(tmpbuf, c);
    return is_utf8_punct(tmpbuf);
}

bool
Perl_is_uni_xdigit(pTHX_ UV c)
{
    U8 tmpbuf[UTF8_MAXBYTES_CASE+1];
    uvchr_to_utf8(tmpbuf, c);
    return is_utf8_xdigit(tmpbuf);
}

UV
Perl__to_upper_title_latin1(pTHX_ const U8 c, U8* p, STRLEN *lenp, const char S_or_s)
{
    /* We have the latin1-range values compiled into the core, so just use
     * those, converting the result to utf8.  The only difference between upper
     * and title case in this range is that LATIN_SMALL_LETTER_SHARP_S is
     * either "SS" or "Ss".  Which one to use is passed into the routine in
     * 'S_or_s' to avoid a test */

    UV converted = toUPPER_LATIN1_MOD(c);

    PERL_ARGS_ASSERT__TO_UPPER_TITLE_LATIN1;

    assert(S_or_s == 'S' || S_or_s == 's');

    if (UNI_IS_INVARIANT(converted)) { /* No difference between the two for
					  characters in this range */
	*p = (U8) converted;
	*lenp = 1;
	return converted;
    }

    /* toUPPER_LATIN1_MOD gives the correct results except for three outliers,
     * which it maps to one of them, so as to only have to have one check for
     * it in the main case */
    if (UNLIKELY(converted == LATIN_SMALL_LETTER_Y_WITH_DIAERESIS)) {
	switch (c) {
	    case LATIN_SMALL_LETTER_Y_WITH_DIAERESIS:
		converted = LATIN_CAPITAL_LETTER_Y_WITH_DIAERESIS;
		break;
	    case MICRO_SIGN:
		converted = GREEK_CAPITAL_LETTER_MU;
		break;
	    case LATIN_SMALL_LETTER_SHARP_S:
		*(p)++ = 'S';
		*p = S_or_s;
		*lenp = 2;
		return 'S';
	    default:
		Perl_croak(aTHX_ "panic: to_upper_title_latin1 did not expect '%c' to map to '%c'", c, LATIN_SMALL_LETTER_Y_WITH_DIAERESIS);
		/* NOTREACHED */
	}
    }

    *(p)++ = UTF8_TWO_BYTE_HI(converted);
    *p = UTF8_TWO_BYTE_LO(converted);
    *lenp = 2;

    return converted;
}

/* Call the function to convert a UTF-8 encoded character to the specified case.
 * Note that there may be more than one character in the result.
 * INP is a pointer to the first byte of the input character
 * OUTP will be set to the first byte of the string of changed characters.  It
 *	needs to have space for UTF8_MAXBYTES_CASE+1 bytes
 * LENP will be set to the length in bytes of the string of changed characters
 *
 * The functions return the ordinal of the first character in the string of OUTP */
#define CALL_UPPER_CASE(INP, OUTP, LENP) Perl_to_utf8_case(aTHX_ INP, OUTP, LENP, &PL_utf8_toupper, "ToUc", "utf8::ToSpecUc")
#define CALL_TITLE_CASE(INP, OUTP, LENP) Perl_to_utf8_case(aTHX_ INP, OUTP, LENP, &PL_utf8_totitle, "ToTc", "utf8::ToSpecTc")
#define CALL_LOWER_CASE(INP, OUTP, LENP) Perl_to_utf8_case(aTHX_ INP, OUTP, LENP, &PL_utf8_tolower, "ToLc", "utf8::ToSpecLc")

/* This additionally has the input parameter SPECIALS, which if non-zero will
 * cause this to use the SPECIALS hash for folding (meaning get full case
 * folding); otherwise, when zero, this implies a simple case fold */
#define CALL_FOLD_CASE(INP, OUTP, LENP, SPECIALS) Perl_to_utf8_case(aTHX_ INP, OUTP, LENP, &PL_utf8_tofold, "ToCf", (SPECIALS) ? "utf8::ToSpecCf" : NULL)

UV
Perl_to_uni_upper(pTHX_ UV c, U8* p, STRLEN *lenp)
{
    dVAR;

    /* Convert the Unicode character whose ordinal is <c> to its uppercase
     * version and store that in UTF-8 in <p> and its length in bytes in <lenp>.
     * Note that the <p> needs to be at least UTF8_MAXBYTES_CASE+1 bytes since
     * the changed version may be longer than the original character.
     *
     * The ordinal of the first character of the changed version is returned
     * (but note, as explained above, that there may be more.) */

    PERL_ARGS_ASSERT_TO_UNI_UPPER;

    if (c < 256) {
	return _to_upper_title_latin1((U8) c, p, lenp, 'S');
    }

    uvchr_to_utf8(p, c);
    return CALL_UPPER_CASE(p, p, lenp);
}

UV
Perl_to_uni_title(pTHX_ UV c, U8* p, STRLEN *lenp)
{
    dVAR;

    PERL_ARGS_ASSERT_TO_UNI_TITLE;

    if (c < 256) {
	return _to_upper_title_latin1((U8) c, p, lenp, 's');
    }

    uvchr_to_utf8(p, c);
    return CALL_TITLE_CASE(p, p, lenp);
}

STATIC U8
S_to_lower_latin1(pTHX_ const U8 c, U8* p, STRLEN *lenp)
{
    /* We have the latin1-range values compiled into the core, so just use
     * those, converting the result to utf8.  Since the result is always just
     * one character, we allow <p> to be NULL */

    U8 converted = toLOWER_LATIN1(c);

    if (p != NULL) {
	if (UNI_IS_INVARIANT(converted)) {
	    *p = converted;
	    *lenp = 1;
	}
	else {
	    *p = UTF8_TWO_BYTE_HI(converted);
	    *(p+1) = UTF8_TWO_BYTE_LO(converted);
	    *lenp = 2;
	}
    }
    return converted;
}

UV
Perl_to_uni_lower(pTHX_ UV c, U8* p, STRLEN *lenp)
{
    dVAR;

    PERL_ARGS_ASSERT_TO_UNI_LOWER;

    if (c < 256) {
	return to_lower_latin1((U8) c, p, lenp);
    }

    uvchr_to_utf8(p, c);
    return CALL_LOWER_CASE(p, p, lenp);
}

UV
Perl__to_fold_latin1(pTHX_ const U8 c, U8* p, STRLEN *lenp, const bool flags)
{
    /* Corresponds to to_lower_latin1(), <flags> is TRUE if to use full case
     * folding */

    UV converted;

    PERL_ARGS_ASSERT__TO_FOLD_LATIN1;

    if (c == MICRO_SIGN) {
	converted = GREEK_SMALL_LETTER_MU;
    }
    else if (flags && c == LATIN_SMALL_LETTER_SHARP_S) {
	*(p)++ = 's';
	*p = 's';
	*lenp = 2;
	return 's';
    }
    else { /* In this range the fold of all other characters is their lower
              case */
	converted = toLOWER_LATIN1(c);
    }

    if (UNI_IS_INVARIANT(converted)) {
	*p = (U8) converted;
	*lenp = 1;
    }
    else {
	*(p)++ = UTF8_TWO_BYTE_HI(converted);
	*p = UTF8_TWO_BYTE_LO(converted);
	*lenp = 2;
    }

    return converted;
}

UV
Perl__to_uni_fold_flags(pTHX_ UV c, U8* p, STRLEN *lenp, const bool flags)
{

    /* Not currently externally documented, and subject to change, <flags> is
     * TRUE iff full folding is to be used */

    PERL_ARGS_ASSERT__TO_UNI_FOLD_FLAGS;

    if (c < 256) {
	return _to_fold_latin1((U8) c, p, lenp, flags);
    }

    uvchr_to_utf8(p, c);
    return CALL_FOLD_CASE(p, p, lenp, flags);
}

/* for now these all assume no locale info available for Unicode > 255; and
 * the corresponding macros in handy.h (like isALNUM_LC_uvchr) should have been
 * called instead, so that these don't get called for < 255 */

bool
Perl_is_uni_alnum_lc(pTHX_ UV c)
{
    return is_uni_alnum(c);	/* XXX no locale support yet */
}

bool
Perl_is_uni_idfirst_lc(pTHX_ UV c)
{
    return is_uni_idfirst(c);	/* XXX no locale support yet */
}

bool
Perl_is_uni_alpha_lc(pTHX_ UV c)
{
    return is_uni_alpha(c);	/* XXX no locale support yet */
}

bool
Perl_is_uni_ascii_lc(pTHX_ UV c)
{
    return is_uni_ascii(c);	/* XXX no locale support yet */
}

bool
Perl_is_uni_space_lc(pTHX_ UV c)
{
    return is_uni_space(c);	/* XXX no locale support yet */
}

bool
Perl_is_uni_digit_lc(pTHX_ UV c)
{
    return is_uni_digit(c);	/* XXX no locale support yet */
}

bool
Perl_is_uni_upper_lc(pTHX_ UV c)
{
    return is_uni_upper(c);	/* XXX no locale support yet */
}

bool
Perl_is_uni_lower_lc(pTHX_ UV c)
{
    return is_uni_lower(c);	/* XXX no locale support yet */
}

bool
Perl_is_uni_cntrl_lc(pTHX_ UV c)
{
    return is_uni_cntrl(c);	/* XXX no locale support yet */
}

bool
Perl_is_uni_graph_lc(pTHX_ UV c)
{
    return is_uni_graph(c);	/* XXX no locale support yet */
}

bool
Perl_is_uni_print_lc(pTHX_ UV c)
{
    return is_uni_print(c);	/* XXX no locale support yet */
}

bool
Perl_is_uni_punct_lc(pTHX_ UV c)
{
    return is_uni_punct(c);	/* XXX no locale support yet */
}

bool
Perl_is_uni_xdigit_lc(pTHX_ UV c)
{
    return is_uni_xdigit(c);	/* XXX no locale support yet */
}

U32
Perl_to_uni_upper_lc(pTHX_ U32 c)
{
    /* XXX returns only the first character -- do not use XXX */
    /* XXX no locale support yet */
    STRLEN len;
    U8 tmpbuf[UTF8_MAXBYTES_CASE+1];
    return (U32)to_uni_upper(c, tmpbuf, &len);
}

U32
Perl_to_uni_title_lc(pTHX_ U32 c)
{
    /* XXX returns only the first character XXX -- do not use XXX */
    /* XXX no locale support yet */
    STRLEN len;
    U8 tmpbuf[UTF8_MAXBYTES_CASE+1];
    return (U32)to_uni_title(c, tmpbuf, &len);
}

U32
Perl_to_uni_lower_lc(pTHX_ U32 c)
{
    /* XXX returns only the first character -- do not use XXX */
    /* XXX no locale support yet */
    STRLEN len;
    U8 tmpbuf[UTF8_MAXBYTES_CASE+1];
    return (U32)to_uni_lower(c, tmpbuf, &len);
}

static bool
S_is_utf8_common(pTHX_ const U8 *const p, SV **swash,
		 const char *const swashname)
{
    /* returns a boolean giving whether or not the UTF8-encoded character that
     * starts at <p> is in the swash indicated by <swashname>.  <swash>
     * contains a pointer to where the swash indicated by <swashname>
     * is to be stored; which this routine will do, so that future calls will
     * look at <*swash> and only generate a swash if it is not null
     *
     * Note that it is assumed that the buffer length of <p> is enough to
     * contain all the bytes that comprise the character.  Thus, <*p> should
     * have been checked before this call for mal-formedness enough to assure
     * that. */

    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_COMMON;

    /* The API should have included a length for the UTF-8 character in <p>,
     * but it doesn't.  We therefor assume that p has been validated at least
     * as far as there being enough bytes available in it to accommodate the
     * character without reading beyond the end, and pass that number on to the
     * validating routine */
    if (!is_utf8_char_buf(p, p + UTF8SKIP(p)))
	return FALSE;
    if (!*swash)
	*swash = swash_init("utf8", swashname, &PL_sv_undef, 1, 0);
    return swash_fetch(*swash, p, TRUE) != 0;
}

bool
Perl_is_utf8_alnum(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_ALNUM;

    /* NOTE: "IsWord", not "IsAlnum", since Alnum is a true
     * descendant of isalnum(3), in other words, it doesn't
     * contain the '_'. --jhi */
    return is_utf8_common(p, &PL_utf8_alnum, "IsWord");
}

bool
Perl_is_utf8_idfirst(pTHX_ const U8 *p) /* The naming is historical. */
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_IDFIRST;

    if (*p == '_')
	return TRUE;
    /* is_utf8_idstart would be more logical. */
    return is_utf8_common(p, &PL_utf8_idstart, "IdStart");
}

bool
Perl_is_utf8_xidfirst(pTHX_ const U8 *p) /* The naming is historical. */
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_XIDFIRST;

    if (*p == '_')
	return TRUE;
    /* is_utf8_idstart would be more logical. */
    return is_utf8_common(p, &PL_utf8_xidstart, "XIdStart");
}

bool
Perl__is_utf8__perl_idstart(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT__IS_UTF8__PERL_IDSTART;

    return is_utf8_common(p, &PL_utf8_perl_idstart, "_Perl_IDStart");
}

bool
Perl_is_utf8_idcont(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_IDCONT;

    return is_utf8_common(p, &PL_utf8_idcont, "IdContinue");
}

bool
Perl_is_utf8_xidcont(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_XIDCONT;

    return is_utf8_common(p, &PL_utf8_idcont, "XIdContinue");
}

bool
Perl_is_utf8_alpha(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_ALPHA;

    return is_utf8_common(p, &PL_utf8_alpha, "IsAlpha");
}

bool
Perl_is_utf8_ascii(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_ASCII;

    /* ASCII characters are the same whether in utf8 or not.  So the macro
     * works on both utf8 and non-utf8 representations. */
    return isASCII(*p);
}

bool
Perl_is_utf8_space(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_SPACE;

    return is_utf8_common(p, &PL_utf8_space, "IsXPerlSpace");
}

bool
Perl_is_utf8_perl_space(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_PERL_SPACE;

    /* Only true if is an ASCII space-like character, and ASCII is invariant
     * under utf8, so can just use the macro */
    return isSPACE_A(*p);
}

bool
Perl_is_utf8_perl_word(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_PERL_WORD;

    /* Only true if is an ASCII word character, and ASCII is invariant
     * under utf8, so can just use the macro */
    return isWORDCHAR_A(*p);
}

bool
Perl_is_utf8_digit(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_DIGIT;

    return is_utf8_common(p, &PL_utf8_digit, "IsDigit");
}

bool
Perl_is_utf8_posix_digit(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_POSIX_DIGIT;

    /* Only true if is an ASCII digit character, and ASCII is invariant
     * under utf8, so can just use the macro */
    return isDIGIT_A(*p);
}

bool
Perl_is_utf8_upper(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_UPPER;

    return is_utf8_common(p, &PL_utf8_upper, "IsUppercase");
}

bool
Perl_is_utf8_lower(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_LOWER;

    return is_utf8_common(p, &PL_utf8_lower, "IsLowercase");
}

bool
Perl_is_utf8_cntrl(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_CNTRL;

    if (isASCII(*p)) {
	return isCNTRL_A(*p);
    }

    /* All controls are in Latin1 */
    if (! UTF8_IS_DOWNGRADEABLE_START(*p)) {
	return 0;
    }
    return isCNTRL_L1(TWO_BYTE_UTF8_TO_UNI(*p, *(p+1)));
}

bool
Perl_is_utf8_graph(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_GRAPH;

    return is_utf8_common(p, &PL_utf8_graph, "IsGraph");
}

bool
Perl_is_utf8_print(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_PRINT;

    return is_utf8_common(p, &PL_utf8_print, "IsPrint");
}

bool
Perl_is_utf8_punct(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_PUNCT;

    return is_utf8_common(p, &PL_utf8_punct, "IsPunct");
}

bool
Perl_is_utf8_xdigit(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_XDIGIT;

    return is_utf8_common(p, &PL_utf8_xdigit, "IsXDigit");
}

bool
Perl_is_utf8_mark(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_MARK;

    return is_utf8_common(p, &PL_utf8_mark, "IsM");
}

bool
Perl_is_utf8_X_begin(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_X_BEGIN;

    return is_utf8_common(p, &PL_utf8_X_begin, "_X_Begin");
}

bool
Perl_is_utf8_X_extend(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_X_EXTEND;

    return is_utf8_common(p, &PL_utf8_X_extend, "_X_Extend");
}

bool
Perl_is_utf8_X_prepend(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_X_PREPEND;

    return is_utf8_common(p, &PL_utf8_X_prepend, "GCB=Prepend");
}

bool
Perl_is_utf8_X_non_hangul(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_X_NON_HANGUL;

    return is_utf8_common(p, &PL_utf8_X_non_hangul, "HST=Not_Applicable");
}

bool
Perl_is_utf8_X_L(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_X_L;

    return is_utf8_common(p, &PL_utf8_X_L, "GCB=L");
}

bool
Perl_is_utf8_X_LV(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_X_LV;

    return is_utf8_common(p, &PL_utf8_X_LV, "GCB=LV");
}

bool
Perl_is_utf8_X_LVT(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_X_LVT;

    return is_utf8_common(p, &PL_utf8_X_LVT, "GCB=LVT");
}

bool
Perl_is_utf8_X_T(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_X_T;

    return is_utf8_common(p, &PL_utf8_X_T, "GCB=T");
}

bool
Perl_is_utf8_X_V(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_X_V;

    return is_utf8_common(p, &PL_utf8_X_V, "GCB=V");
}

bool
Perl_is_utf8_X_LV_LVT_V(pTHX_ const U8 *p)
{
    dVAR;

    PERL_ARGS_ASSERT_IS_UTF8_X_LV_LVT_V;

    return is_utf8_common(p, &PL_utf8_X_LV_LVT_V, "_X_LV_LVT_V");
}

bool
Perl__is_utf8_quotemeta(pTHX_ const U8 *p)
{
    /* For exclusive use of pp_quotemeta() */

    dVAR;

    PERL_ARGS_ASSERT__IS_UTF8_QUOTEMETA;

    return is_utf8_common(p, &PL_utf8_quotemeta, "_Perl_Quotemeta");
}

/*
=for apidoc to_utf8_case

The C<p> contains the pointer to the UTF-8 string encoding
the character that is being converted.  This routine assumes that the character
at C<p> is well-formed.

The C<ustrp> is a pointer to the character buffer to put the
conversion result to.  The C<lenp> is a pointer to the length
of the result.

The C<swashp> is a pointer to the swash to use.

Both the special and normal mappings are stored in F<lib/unicore/To/Foo.pl>,
and loaded by SWASHNEW, using F<lib/utf8_heavy.pl>.  The C<special> (usually,
but not always, a multicharacter mapping), is tried first.

The C<special> is a string like "utf8::ToSpecLower", which means the
hash %utf8::ToSpecLower.  The access to the hash is through
Perl_to_utf8_case().

The C<normal> is a string like "ToLower" which means the swash
%utf8::ToLower.

=cut */

UV
Perl_to_utf8_case(pTHX_ const U8 *p, U8* ustrp, STRLEN *lenp,
			SV **swashp, const char *normal, const char *special)
{
    dVAR;
    U8 tmpbuf[UTF8_MAXBYTES_CASE+1];
    STRLEN len = 0;
    const UV uv0 = valid_utf8_to_uvchr(p, NULL);
    /* The NATIVE_TO_UNI() and UNI_TO_NATIVE() mappings
     * are necessary in EBCDIC, they are redundant no-ops
     * in ASCII-ish platforms, and hopefully optimized away. */
    const UV uv1 = NATIVE_TO_UNI(uv0);

    PERL_ARGS_ASSERT_TO_UTF8_CASE;

    /* Note that swash_fetch() doesn't output warnings for these because it
     * assumes we will */
    if (uv1 >= UNICODE_SURROGATE_FIRST) {
	if (uv1 <= UNICODE_SURROGATE_LAST) {
	    if (ckWARN_d(WARN_SURROGATE)) {
		const char* desc = (PL_op) ? OP_DESC(PL_op) : normal;
		Perl_warner(aTHX_ packWARN(WARN_SURROGATE),
		    "Operation \"%s\" returns its argument for UTF-16 surrogate U+%04"UVXf"", desc, uv1);
	    }
	}
	else if (UNICODE_IS_SUPER(uv1)) {
	    if (ckWARN_d(WARN_NON_UNICODE)) {
		const char* desc = (PL_op) ? OP_DESC(PL_op) : normal;
		Perl_warner(aTHX_ packWARN(WARN_NON_UNICODE),
		    "Operation \"%s\" returns its argument for non-Unicode code point 0x%04"UVXf"", desc, uv1);
	    }
	}

	/* Note that non-characters are perfectly legal, so no warning should
	 * be given */
    }

    uvuni_to_utf8(tmpbuf, uv1);

    if (!*swashp) /* load on-demand */
         *swashp = swash_init("utf8", normal, &PL_sv_undef, 4, 0);

    if (special) {
         /* It might be "special" (sometimes, but not always,
	  * a multicharacter mapping) */
	 HV * const hv = get_hv(special, 0);
	 SV **svp;

	 if (hv &&
	     (svp = hv_fetch(hv, (const char*)tmpbuf, UNISKIP(uv1), FALSE)) &&
	     (*svp)) {
	     const char *s;

	      s = SvPV_const(*svp, len);
	      if (len == 1)
		   len = uvuni_to_utf8(ustrp, NATIVE_TO_UNI(*(U8*)s)) - ustrp;
	      else {
#ifdef EBCDIC
		   /* If we have EBCDIC we need to remap the characters
		    * since any characters in the low 256 are Unicode
		    * code points, not EBCDIC. */
		   U8 *t = (U8*)s, *tend = t + len, *d;

		   d = tmpbuf;
		   if (SvUTF8(*svp)) {
			STRLEN tlen = 0;

			while (t < tend) {
			     const UV c = utf8_to_uvchr_buf(t, tend, &tlen);
			     if (tlen > 0) {
				  d = uvchr_to_utf8(d, UNI_TO_NATIVE(c));
				  t += tlen;
			     }
			     else
				  break;
			}
		   }
		   else {
			while (t < tend) {
			     d = uvchr_to_utf8(d, UNI_TO_NATIVE(*t));
			     t++;
			}
		   }
		   len = d - tmpbuf;
		   Copy(tmpbuf, ustrp, len, U8);
#else
		   Copy(s, ustrp, len, U8);
#endif
	      }
	 }
    }

    if (!len && *swashp) {
	const UV uv2 = swash_fetch(*swashp, tmpbuf, TRUE);

	 if (uv2) {
	      /* It was "normal" (a single character mapping). */
	      const UV uv3 = UNI_TO_NATIVE(uv2);
	      len = uvchr_to_utf8(ustrp, uv3) - ustrp;
	 }
    }

    if (!len) /* Neither: just copy.  In other words, there was no mapping
		 defined, which means that the code point maps to itself */
	 len = uvchr_to_utf8(ustrp, uv0) - ustrp;

    if (lenp)
	 *lenp = len;

    return len ? valid_utf8_to_uvchr(ustrp, 0) : 0;
}

STATIC UV
S_check_locale_boundary_crossing(pTHX_ const U8* const p, const UV result, U8* const ustrp, STRLEN *lenp)
{
    /* This is called when changing the case of a utf8-encoded character above
     * the Latin1 range, and the operation is in locale.  If the result
     * contains a character that crosses the 255/256 boundary, disallow the
     * change, and return the original code point.  See L<perlfunc/lc> for why;
     *
     * p	points to the original string whose case was changed; assumed
     *          by this routine to be well-formed
     * result	the code point of the first character in the changed-case string
     * ustrp	points to the changed-case string (<result> represents its first char)
     * lenp	points to the length of <ustrp> */

    UV original;    /* To store the first code point of <p> */

    PERL_ARGS_ASSERT_CHECK_LOCALE_BOUNDARY_CROSSING;

    assert(! UTF8_IS_INVARIANT(*p) && ! UTF8_IS_DOWNGRADEABLE_START(*p));

    /* We know immediately if the first character in the string crosses the
     * boundary, so can skip */
    if (result > 255) {

	/* Look at every character in the result; if any cross the
	* boundary, the whole thing is disallowed */
	U8* s = ustrp + UTF8SKIP(ustrp);
	U8* e = ustrp + *lenp;
	while (s < e) {
	    if (UTF8_IS_INVARIANT(*s) || UTF8_IS_DOWNGRADEABLE_START(*s))
	    {
		goto bad_crossing;
	    }
	    s += UTF8SKIP(s);
	}

	/* Here, no characters crossed, result is ok as-is */
	return result;
    }

bad_crossing:

    /* Failed, have to return the original */
    original = valid_utf8_to_uvchr(p, lenp);
    Copy(p, ustrp, *lenp, char);
    return original;
}

/*
=for apidoc to_utf8_upper

Convert the UTF-8 encoded character at C<p> to its uppercase version and
store that in UTF-8 in C<ustrp> and its length in bytes in C<lenp>.  Note
that the ustrp needs to be at least UTF8_MAXBYTES_CASE+1 bytes since
the uppercase version may be longer than the original character.

The first character of the uppercased version is returned
(but note, as explained above, that there may be more.)

The character at C<p> is assumed by this routine to be well-formed.

=cut */

/* Not currently externally documented, and subject to change:
 * <flags> is set iff locale semantics are to be used for code points < 256
 * <tainted_ptr> if non-null, *tainted_ptr will be set TRUE iff locale rules
 *		 were used in the calculation; otherwise unchanged. */

UV
Perl__to_utf8_upper_flags(pTHX_ const U8 *p, U8* ustrp, STRLEN *lenp, const bool flags, bool* tainted_ptr)
{
    dVAR;

    UV result;

    PERL_ARGS_ASSERT__TO_UTF8_UPPER_FLAGS;

    if (UTF8_IS_INVARIANT(*p)) {
	if (flags) {
	    result = toUPPER_LC(*p);
	}
	else {
	    return _to_upper_title_latin1(*p, ustrp, lenp, 'S');
	}
    }
    else if UTF8_IS_DOWNGRADEABLE_START(*p) {
	if (flags) {
	    result = toUPPER_LC(TWO_BYTE_UTF8_TO_UNI(*p, *(p+1)));
	}
	else {
	    return _to_upper_title_latin1(TWO_BYTE_UTF8_TO_UNI(*p, *(p+1)),
				          ustrp, lenp, 'S');
	}
    }
    else {  /* utf8, ord above 255 */
	result = CALL_UPPER_CASE(p, ustrp, lenp);

	if (flags) {
	    result = check_locale_boundary_crossing(p, result, ustrp, lenp);
	}
	return result;
    }

    /* Here, used locale rules.  Convert back to utf8 */
    if (UTF8_IS_INVARIANT(result)) {
	*ustrp = (U8) result;
	*lenp = 1;
    }
    else {
	*ustrp = UTF8_EIGHT_BIT_HI(result);
	*(ustrp + 1) = UTF8_EIGHT_BIT_LO(result);
	*lenp = 2;
    }

    if (tainted_ptr) {
	*tainted_ptr = TRUE;
    }
    return result;
}

/*
=for apidoc to_utf8_title

Convert the UTF-8 encoded character at C<p> to its titlecase version and
store that in UTF-8 in C<ustrp> and its length in bytes in C<lenp>.  Note
that the C<ustrp> needs to be at least UTF8_MAXBYTES_CASE+1 bytes since the
titlecase version may be longer than the original character.

The first character of the titlecased version is returned
(but note, as explained above, that there may be more.)

The character at C<p> is assumed by this routine to be well-formed.

=cut */

/* Not currently externally documented, and subject to change:
 * <flags> is set iff locale semantics are to be used for code points < 256
 *	   Since titlecase is not defined in POSIX, uppercase is used instead
 *	   for these/
 * <tainted_ptr> if non-null, *tainted_ptr will be set TRUE iff locale rules
 *		 were used in the calculation; otherwise unchanged. */

UV
Perl__to_utf8_title_flags(pTHX_ const U8 *p, U8* ustrp, STRLEN *lenp, const bool flags, bool* tainted_ptr)
{
    dVAR;

    UV result;

    PERL_ARGS_ASSERT__TO_UTF8_TITLE_FLAGS;

    if (UTF8_IS_INVARIANT(*p)) {
	if (flags) {
	    result = toUPPER_LC(*p);
	}
	else {
	    return _to_upper_title_latin1(*p, ustrp, lenp, 's');
	}
    }
    else if UTF8_IS_DOWNGRADEABLE_START(*p) {
	if (flags) {
	    result = toUPPER_LC(TWO_BYTE_UTF8_TO_UNI(*p, *(p+1)));
	}
	else {
	    return _to_upper_title_latin1(TWO_BYTE_UTF8_TO_UNI(*p, *(p+1)),
				          ustrp, lenp, 's');
	}
    }
    else {  /* utf8, ord above 255 */
	result = CALL_TITLE_CASE(p, ustrp, lenp);

	if (flags) {
	    result = check_locale_boundary_crossing(p, result, ustrp, lenp);
	}
	return result;
    }

    /* Here, used locale rules.  Convert back to utf8 */
    if (UTF8_IS_INVARIANT(result)) {
	*ustrp = (U8) result;
	*lenp = 1;
    }
    else {
	*ustrp = UTF8_EIGHT_BIT_HI(result);
	*(ustrp + 1) = UTF8_EIGHT_BIT_LO(result);
	*lenp = 2;
    }

    if (tainted_ptr) {
	*tainted_ptr = TRUE;
    }
    return result;
}

/*
=for apidoc to_utf8_lower

Convert the UTF-8 encoded character at C<p> to its lowercase version and
store that in UTF-8 in ustrp and its length in bytes in C<lenp>.  Note
that the C<ustrp> needs to be at least UTF8_MAXBYTES_CASE+1 bytes since the
lowercase version may be longer than the original character.

The first character of the lowercased version is returned
(but note, as explained above, that there may be more.)

The character at C<p> is assumed by this routine to be well-formed.

=cut */

/* Not currently externally documented, and subject to change:
 * <flags> is set iff locale semantics are to be used for code points < 256
 * <tainted_ptr> if non-null, *tainted_ptr will be set TRUE iff locale rules
 *		 were used in the calculation; otherwise unchanged. */

UV
Perl__to_utf8_lower_flags(pTHX_ const U8 *p, U8* ustrp, STRLEN *lenp, const bool flags, bool* tainted_ptr)
{
    UV result;

    dVAR;

    PERL_ARGS_ASSERT__TO_UTF8_LOWER_FLAGS;

    if (UTF8_IS_INVARIANT(*p)) {
	if (flags) {
	    result = toLOWER_LC(*p);
	}
	else {
	    return to_lower_latin1(*p, ustrp, lenp);
	}
    }
    else if UTF8_IS_DOWNGRADEABLE_START(*p) {
	if (flags) {
	    result = toLOWER_LC(TWO_BYTE_UTF8_TO_UNI(*p, *(p+1)));
	}
	else {
	    return to_lower_latin1(TWO_BYTE_UTF8_TO_UNI(*p, *(p+1)),
		                   ustrp, lenp);
	}
    }
    else {  /* utf8, ord above 255 */
	result = CALL_LOWER_CASE(p, ustrp, lenp);

	if (flags) {
	    result = check_locale_boundary_crossing(p, result, ustrp, lenp);
	}

	return result;
    }

    /* Here, used locale rules.  Convert back to utf8 */
    if (UTF8_IS_INVARIANT(result)) {
	*ustrp = (U8) result;
	*lenp = 1;
    }
    else {
	*ustrp = UTF8_EIGHT_BIT_HI(result);
	*(ustrp + 1) = UTF8_EIGHT_BIT_LO(result);
	*lenp = 2;
    }

    if (tainted_ptr) {
	*tainted_ptr = TRUE;
    }
    return result;
}

/*
=for apidoc to_utf8_fold

Convert the UTF-8 encoded character at C<p> to its foldcase version and
store that in UTF-8 in C<ustrp> and its length in bytes in C<lenp>.  Note
that the C<ustrp> needs to be at least UTF8_MAXBYTES_CASE+1 bytes since the
foldcase version may be longer than the original character (up to
three characters).

The first character of the foldcased version is returned
(but note, as explained above, that there may be more.)

The character at C<p> is assumed by this routine to be well-formed.

=cut */

/* Not currently externally documented, and subject to change,
 * in <flags>
 *	bit FOLD_FLAGS_LOCALE is set iff locale semantics are to be used for code
 *			      points < 256.  Since foldcase is not defined in
 *			      POSIX, lowercase is used instead
 *      bit FOLD_FLAGS_FULL   is set iff full case folds are to be used;
 *			      otherwise simple folds
 * <tainted_ptr> if non-null, *tainted_ptr will be set TRUE iff locale rules
 *		 were used in the calculation; otherwise unchanged. */

UV
Perl__to_utf8_fold_flags(pTHX_ const U8 *p, U8* ustrp, STRLEN *lenp, U8 flags, bool* tainted_ptr)
{
    dVAR;

    UV result;

    PERL_ARGS_ASSERT__TO_UTF8_FOLD_FLAGS;

    if (UTF8_IS_INVARIANT(*p)) {
	if (flags & FOLD_FLAGS_LOCALE) {
	    result = toLOWER_LC(*p);
	}
	else {
	    return _to_fold_latin1(*p, ustrp, lenp,
		                   cBOOL(flags & FOLD_FLAGS_FULL));
	}
    }
    else if UTF8_IS_DOWNGRADEABLE_START(*p) {
	if (flags & FOLD_FLAGS_LOCALE) {
	    result = toLOWER_LC(TWO_BYTE_UTF8_TO_UNI(*p, *(p+1)));
	}
	else {
	    return _to_fold_latin1(TWO_BYTE_UTF8_TO_UNI(*p, *(p+1)),
		                   ustrp, lenp, cBOOL(flags & FOLD_FLAGS_FULL));
	}
    }
    else {  /* utf8, ord above 255 */
	result = CALL_FOLD_CASE(p, ustrp, lenp, flags);

	if ((flags & FOLD_FLAGS_LOCALE)) {
	    result = check_locale_boundary_crossing(p, result, ustrp, lenp);
	}

	return result;
    }

    /* Here, used locale rules.  Convert back to utf8 */
    if (UTF8_IS_INVARIANT(result)) {
	*ustrp = (U8) result;
	*lenp = 1;
    }
    else {
	*ustrp = UTF8_EIGHT_BIT_HI(result);
	*(ustrp + 1) = UTF8_EIGHT_BIT_LO(result);
	*lenp = 2;
    }

    if (tainted_ptr) {
	*tainted_ptr = TRUE;
    }
    return result;
}

/* Note:
 * Returns a "swash" which is a hash described in utf8.c:Perl_swash_fetch().
 * C<pkg> is a pointer to a package name for SWASHNEW, should be "utf8".
 * For other parameters, see utf8::SWASHNEW in lib/utf8_heavy.pl.
 */

SV*
Perl_swash_init(pTHX_ const char* pkg, const char* name, SV *listsv, I32 minbits, I32 none)
{
    PERL_ARGS_ASSERT_SWASH_INIT;

    /* Returns a copy of a swash initiated by the called function.  This is the
     * public interface, and returning a copy prevents others from doing
     * mischief on the original */

    return newSVsv(_core_swash_init(pkg, name, listsv, minbits, none, FALSE, NULL, FALSE));
}

SV*
Perl__core_swash_init(pTHX_ const char* pkg, const char* name, SV *listsv, I32 minbits, I32 none, bool return_if_undef, SV* invlist, bool passed_in_invlist_has_user_defined_property)
{
    /* Initialize and return a swash, creating it if necessary.  It does this
     * by calling utf8_heavy.pl in the general case.
     *
     * This interface should only be used by functions that won't destroy or
     * adversely change the swash, as doing so affects all other uses of the
     * swash in the program; the general public should use 'Perl_swash_init'
     * instead.
     *
     * pkg  is the name of the package that <name> should be in.
     * name is the name of the swash to find.  Typically it is a Unicode
     *	    property name, including user-defined ones
     * listsv is a string to initialize the swash with.  It must be of the form
     *	    documented as the subroutine return value in
     *	    L<perlunicode/User-Defined Character Properties>
     * minbits is the number of bits required to represent each data element.
     *	    It is '1' for binary properties.
     * none I (khw) do not understand this one, but it is used only in tr///.
     * return_if_undef is TRUE if the routine shouldn't croak if it can't find
     *	    the requested property
     * invlist is an inversion list to initialize the swash with (or NULL)
     * has_user_defined_property is TRUE if <invlist> has some component that
     *      came from a user-defined property
     *
     * Thus there are three possible inputs to find the swash: <name>,
     * <listsv>, and <invlist>.  At least one must be specified.  The result
     * will be the union of the specified ones, although <listsv>'s various
     * actions can intersect, etc. what <name> gives.
     *
     * <invlist> is only valid for binary properties */

    dVAR;
    SV* retval = &PL_sv_undef;

    assert(listsv != &PL_sv_undef || strNE(name, "") || invlist);
    assert(! invlist || minbits == 1);

    /* If data was passed in to go out to utf8_heavy to find the swash of, do
     * so */
    if (listsv != &PL_sv_undef || strNE(name, "")) {
	dSP;
	const size_t pkg_len = strlen(pkg);
	const size_t name_len = strlen(name);
	HV * const stash = gv_stashpvn(pkg, pkg_len, 0);
	SV* errsv_save;
	GV *method;

	PERL_ARGS_ASSERT__CORE_SWASH_INIT;

	PUSHSTACKi(PERLSI_MAGIC);
	ENTER;
	SAVEHINTS();
	save_re_context();
	if (PL_parser && PL_parser->error_count)
	    SAVEI8(PL_parser->error_count), PL_parser->error_count = 0;
	method = gv_fetchmeth(stash, "SWASHNEW", 8, -1);
	if (!method) {	/* demand load utf8 */
	    ENTER;
	    errsv_save = newSVsv(ERRSV);
	    /* It is assumed that callers of this routine are not passing in
	     * any user derived data.  */
	    /* Need to do this after save_re_context() as it will set
	     * PL_tainted to 1 while saving $1 etc (see the code after getrx:
	     * in Perl_magic_get).  Even line to create errsv_save can turn on
	     * PL_tainted.  */
	    SAVEBOOL(PL_tainted);
	    PL_tainted = 0;
	    Perl_load_module(aTHX_ PERL_LOADMOD_NOIMPORT, newSVpvn(pkg,pkg_len),
			     NULL);
	    if (!SvTRUE(ERRSV))
		sv_setsv(ERRSV, errsv_save);
	    SvREFCNT_dec(errsv_save);
	    LEAVE;
	}
	SPAGAIN;
	PUSHMARK(SP);
	EXTEND(SP,5);
	mPUSHp(pkg, pkg_len);
	mPUSHp(name, name_len);
	PUSHs(listsv);
	mPUSHi(minbits);
	mPUSHi(none);
	PUTBACK;
	errsv_save = newSVsv(ERRSV);
	/* If we already have a pointer to the method, no need to use
	 * call_method() to repeat the lookup.  */
	if (method ? call_sv(MUTABLE_SV(method), G_SCALAR)
	    : call_sv(newSVpvs_flags("SWASHNEW", SVs_TEMP), G_SCALAR | G_METHOD))
	{
	    retval = *PL_stack_sp--;
	    SvREFCNT_inc(retval);
	}
	if (!SvTRUE(ERRSV))
	    sv_setsv(ERRSV, errsv_save);
	SvREFCNT_dec(errsv_save);
	LEAVE;
	POPSTACK;
	if (IN_PERL_COMPILETIME) {
	    CopHINTS_set(PL_curcop, PL_hints);
	}
	if (!SvROK(retval) || SvTYPE(SvRV(retval)) != SVt_PVHV) {
	    if (SvPOK(retval))

		/* If caller wants to handle missing properties, let them */
		if (return_if_undef) {
		    return NULL;
		}
		Perl_croak(aTHX_
			   "Can't find Unicode property definition \"%"SVf"\"",
			   SVfARG(retval));
	    Perl_croak(aTHX_ "SWASHNEW didn't return an HV ref");
	}
    } /* End of calling the module to find the swash */

    /* Make sure there is an inversion list for binary properties */
    if (minbits == 1) {
	SV** swash_invlistsvp = NULL;
	SV* swash_invlist = NULL;
	bool invlist_in_swash_is_valid = FALSE;
	HV* swash_hv = NULL;

        /* If this operation fetched a swash, get its already existing
         * inversion list or create one for it */
	if (retval != &PL_sv_undef) {
	    swash_hv = MUTABLE_HV(SvRV(retval));

	    swash_invlistsvp = hv_fetchs(swash_hv, "INVLIST", FALSE);
	    if (swash_invlistsvp) {
		swash_invlist = *swash_invlistsvp;
		invlist_in_swash_is_valid = TRUE;
	    }
	    else {
		swash_invlist = _swash_to_invlist(retval);
	    }
	}

	/* If an inversion list was passed in, have to include it */
	if (invlist) {

            /* Any fetched swash will by now have an inversion list in it;
             * otherwise <swash_invlist>  will be NULL, indicating that we
             * didn't fetch a swash */
	    if (swash_invlist) {

		/* Add the passed-in inversion list, which invalidates the one
		 * already stored in the swash */
		invlist_in_swash_is_valid = FALSE;
		_invlist_union(invlist, swash_invlist, &swash_invlist);
	    }
	    else {

		/* Here, there is no swash already.  Set up a minimal one */
		swash_hv = newHV();
		retval = newRV_inc(MUTABLE_SV(swash_hv));
		swash_invlist = invlist;
	    }

            if (passed_in_invlist_has_user_defined_property) {
                if (! hv_stores(swash_hv, "USER_DEFINED", newSVuv(1))) {
                    Perl_croak(aTHX_ "panic: hv_store() unexpectedly failed");
                }
            }
	}

        /* Here, we have computed the union of all the passed-in data.  It may
         * be that there was an inversion list in the swash which didn't get
         * touched; otherwise save the one computed one */
	if (! invlist_in_swash_is_valid) {
	    if (! hv_stores(MUTABLE_HV(SvRV(retval)), "INVLIST", swash_invlist))
            {
		Perl_croak(aTHX_ "panic: hv_store() unexpectedly failed");
	    }
	}
    }

    return retval;
}


/* This API is wrong for special case conversions since we may need to
 * return several Unicode characters for a single Unicode character
 * (see lib/unicore/SpecCase.txt) The SWASHGET in lib/utf8_heavy.pl is
 * the lower-level routine, and it is similarly broken for returning
 * multiple values.  --jhi
 * For those, you should use to_utf8_case() instead */
/* Now SWASHGET is recasted into S_swatch_get in this file. */

/* Note:
 * Returns the value of property/mapping C<swash> for the first character
 * of the string C<ptr>. If C<do_utf8> is true, the string C<ptr> is
 * assumed to be in utf8. If C<do_utf8> is false, the string C<ptr> is
 * assumed to be in native 8-bit encoding. Caches the swatch in C<swash>.
 *
 * A "swash" is a hash which contains initially the keys/values set up by
 * SWASHNEW.  The purpose is to be able to completely represent a Unicode
 * property for all possible code points.  Things are stored in a compact form
 * (see utf8_heavy.pl) so that calculation is required to find the actual
 * property value for a given code point.  As code points are looked up, new
 * key/value pairs are added to the hash, so that the calculation doesn't have
 * to ever be re-done.  Further, each calculation is done, not just for the
 * desired one, but for a whole block of code points adjacent to that one.
 * For binary properties on ASCII machines, the block is usually for 64 code
 * points, starting with a code point evenly divisible by 64.  Thus if the
 * property value for code point 257 is requested, the code goes out and
 * calculates the property values for all 64 code points between 256 and 319,
 * and stores these as a single 64-bit long bit vector, called a "swatch",
 * under the key for code point 256.  The key is the UTF-8 encoding for code
 * point 256, minus the final byte.  Thus, if the length of the UTF-8 encoding
 * for a code point is 13 bytes, the key will be 12 bytes long.  If the value
 * for code point 258 is then requested, this code realizes that it would be
 * stored under the key for 256, and would find that value and extract the
 * relevant bit, offset from 256.
 *
 * Non-binary properties are stored in as many bits as necessary to represent
 * their values (32 currently, though the code is more general than that), not
 * as single bits, but the principal is the same: the value for each key is a
 * vector that encompasses the property values for all code points whose UTF-8
 * representations are represented by the key.  That is, for all code points
 * whose UTF-8 representations are length N bytes, and the key is the first N-1
 * bytes of that.
 */
UV
Perl_swash_fetch(pTHX_ SV *swash, const U8 *ptr, bool do_utf8)
{
    dVAR;
    HV *const hv = MUTABLE_HV(SvRV(swash));
    U32 klen;
    U32 off;
    STRLEN slen;
    STRLEN needents;
    const U8 *tmps = NULL;
    U32 bit;
    SV *swatch;
    U8 tmputf8[2];
    const UV c = NATIVE_TO_ASCII(*ptr);

    PERL_ARGS_ASSERT_SWASH_FETCH;

    /* Convert to utf8 if not already */
    if (!do_utf8 && !UNI_IS_INVARIANT(c)) {
	tmputf8[0] = (U8)UTF8_EIGHT_BIT_HI(c);
	tmputf8[1] = (U8)UTF8_EIGHT_BIT_LO(c);
	ptr = tmputf8;
    }
    /* Given a UTF-X encoded char 0xAA..0xYY,0xZZ
     * then the "swatch" is a vec() for all the chars which start
     * with 0xAA..0xYY
     * So the key in the hash (klen) is length of encoded char -1
     */
    klen = UTF8SKIP(ptr) - 1;
    off  = ptr[klen];

    if (klen == 0) {
      /* If char is invariant then swatch is for all the invariant chars
       * In both UTF-8 and UTF-8-MOD that happens to be UTF_CONTINUATION_MARK
       */
	needents = UTF_CONTINUATION_MARK;
	off      = NATIVE_TO_UTF(ptr[klen]);
    }
    else {
      /* If char is encoded then swatch is for the prefix */
	needents = (1 << UTF_ACCUMULATION_SHIFT);
	off      = NATIVE_TO_UTF(ptr[klen]) & UTF_CONTINUATION_MASK;
    }

    /*
     * This single-entry cache saves about 1/3 of the utf8 overhead in test
     * suite.  (That is, only 7-8% overall over just a hash cache.  Still,
     * it's nothing to sniff at.)  Pity we usually come through at least
     * two function calls to get here...
     *
     * NB: this code assumes that swatches are never modified, once generated!
     */

    if (hv   == PL_last_swash_hv &&
	klen == PL_last_swash_klen &&
	(!klen || memEQ((char *)ptr, (char *)PL_last_swash_key, klen)) )
    {
	tmps = PL_last_swash_tmps;
	slen = PL_last_swash_slen;
    }
    else {
	/* Try our second-level swatch cache, kept in a hash. */
	SV** svp = hv_fetch(hv, (const char*)ptr, klen, FALSE);

	/* If not cached, generate it via swatch_get */
	if (!svp || !SvPOK(*svp)
		 || !(tmps = (const U8*)SvPV_const(*svp, slen))) {
	    /* We use utf8n_to_uvuni() as we want an index into
	       Unicode tables, not a native character number.
	     */
	    const UV code_point = utf8n_to_uvuni(ptr, UTF8_MAXBYTES, 0,
					   ckWARN(WARN_UTF8) ?
					   0 : UTF8_ALLOW_ANY);
	    swatch = swatch_get(swash,
		    /* On EBCDIC & ~(0xA0-1) isn't a useful thing to do */
				(klen) ? (code_point & ~((UV)needents - 1)) : 0,
				needents);

	    if (IN_PERL_COMPILETIME)
		CopHINTS_set(PL_curcop, PL_hints);

	    svp = hv_store(hv, (const char *)ptr, klen, swatch, 0);

	    if (!svp || !(tmps = (U8*)SvPV(*svp, slen))
		     || (slen << 3) < needents)
		Perl_croak(aTHX_ "panic: swash_fetch got improper swatch, "
			   "svp=%p, tmps=%p, slen=%"UVuf", needents=%"UVuf,
			   svp, tmps, (UV)slen, (UV)needents);
	}

	PL_last_swash_hv = hv;
	assert(klen <= sizeof(PL_last_swash_key));
	PL_last_swash_klen = (U8)klen;
	/* FIXME change interpvar.h?  */
	PL_last_swash_tmps = (U8 *) tmps;
	PL_last_swash_slen = slen;
	if (klen)
	    Copy(ptr, PL_last_swash_key, klen, U8);
    }

    if (UTF8_IS_SUPER(ptr) && ckWARN_d(WARN_NON_UNICODE)) {
	SV** const bitssvp = hv_fetchs(hv, "BITS", FALSE);

	/* This outputs warnings for binary properties only, assuming that
	 * to_utf8_case() will output any for non-binary.  Also, surrogates
	 * aren't checked for, as that would warn on things like /\p{Gc=Cs}/ */

	if (! bitssvp || SvUV(*bitssvp) == 1) {
	    /* User-defined properties can silently match above-Unicode */
	    SV** const user_defined_svp = hv_fetchs(hv, "USER_DEFINED", FALSE);
	    if (! user_defined_svp || ! SvUV(*user_defined_svp)) {
		const UV code_point = utf8n_to_uvuni(ptr, UTF8_MAXBYTES, 0, 0);
		Perl_warner(aTHX_ packWARN(WARN_NON_UNICODE),
		    "Code point 0x%04"UVXf" is not Unicode, all \\p{} matches fail; all \\P{} matches succeed", code_point);
	    }
	}
    }

    switch ((int)((slen << 3) / needents)) {
    case 1:
	bit = 1 << (off & 7);
	off >>= 3;
	return (tmps[off] & bit) != 0;
    case 8:
	return tmps[off];
    case 16:
	off <<= 1;
	return (tmps[off] << 8) + tmps[off + 1] ;
    case 32:
	off <<= 2;
	return (tmps[off] << 24) + (tmps[off+1] << 16) + (tmps[off+2] << 8) + tmps[off + 3] ;
    }
    Perl_croak(aTHX_ "panic: swash_fetch got swatch of unexpected bit width, "
	       "slen=%"UVuf", needents=%"UVuf, (UV)slen, (UV)needents);
    NORETURN_FUNCTION_END;
}

/* Read a single line of the main body of the swash input text.  These are of
 * the form:
 * 0053	0056	0073
 * where each number is hex.  The first two numbers form the minimum and
 * maximum of a range, and the third is the value associated with the range.
 * Not all swashes should have a third number
 *
 * On input: l	  points to the beginning of the line to be examined; it points
 *		  to somewhere in the string of the whole input text, and is
 *		  terminated by a \n or the null string terminator.
 *	     lend   points to the null terminator of that string
 *	     wants_value    is non-zero if the swash expects a third number
 *	     typestr is the name of the swash's mapping, like 'ToLower'
 * On output: *min, *max, and *val are set to the values read from the line.
 *	      returns a pointer just beyond the line examined.  If there was no
 *	      valid min number on the line, returns lend+1
 */

STATIC U8*
S_swash_scan_list_line(pTHX_ U8* l, U8* const lend, UV* min, UV* max, UV* val,
			     const bool wants_value, const U8* const typestr)
{
    const int  typeto  = typestr[0] == 'T' && typestr[1] == 'o';
    STRLEN numlen;	    /* Length of the number */
    I32 flags = PERL_SCAN_SILENT_ILLDIGIT
		| PERL_SCAN_DISALLOW_PREFIX
		| PERL_SCAN_SILENT_NON_PORTABLE;

    /* nl points to the next \n in the scan */
    U8* const nl = (U8*)memchr(l, '\n', lend - l);

    /* Get the first number on the line: the range minimum */
    numlen = lend - l;
    *min = grok_hex((char *)l, &numlen, &flags, NULL);
    if (numlen)	    /* If found a hex number, position past it */
	l += numlen;
    else if (nl) {	    /* Else, go handle next line, if any */
	return nl + 1;	/* 1 is length of "\n" */
    }
    else {		/* Else, no next line */
	return lend + 1;	/* to LIST's end at which \n is not found */
    }

    /* The max range value follows, separated by a BLANK */
    if (isBLANK(*l)) {
	++l;
	flags = PERL_SCAN_SILENT_ILLDIGIT
		| PERL_SCAN_DISALLOW_PREFIX
		| PERL_SCAN_SILENT_NON_PORTABLE;
	numlen = lend - l;
	*max = grok_hex((char *)l, &numlen, &flags, NULL);
	if (numlen)
	    l += numlen;
	else    /* If no value here, it is a single element range */
	    *max = *min;

	/* Non-binary tables have a third entry: what the first element of the
	 * range maps to */
	if (wants_value) {
	    if (isBLANK(*l)) {
		++l;

		/* The ToLc, etc table mappings are not in hex, and must be
		 * corrected by adding the code point to them */
/*		if (typeto) {
		    char *after_strtol = (char *) lend;
		    *val = Strtol((char *)l, &after_strtol, 10);
		    l = (U8 *) after_strtol;
		}
		else { COMMENT COMMENT Other tables are in hex, and are the correct result
			  without tweaking
*/
		    flags = PERL_SCAN_SILENT_ILLDIGIT
			| PERL_SCAN_DISALLOW_PREFIX
			| PERL_SCAN_SILENT_NON_PORTABLE;
		    numlen = lend - l;
		    *val = grok_hex((char *)l, &numlen, &flags, NULL);
		    if (numlen)
			l += numlen;
		    else
			*val = 0;
		/* } */
	    }
	    else {
		*val = 0;
		if (typeto) {
		    /* diag_listed_as: To%s: illegal mapping '%s' */
		    Perl_croak(aTHX_ "%s: illegal mapping '%s'",
				     typestr, l);
		}
	    }
	}
	else
	    *val = 0; /* bits == 1, then any val should be ignored */
    }
    else { /* Nothing following range min, should be single element with no
	      mapping expected */
	*max = *min;
	if (wants_value) {
	    *val = 0;
	    if (typeto) {
		/* diag_listed_as: To%s: illegal mapping '%s' */
		Perl_croak(aTHX_ "%s: illegal mapping '%s'", typestr, l);
	    }
	}
	else
	    *val = 0; /* bits == 1, then val should be ignored */
    }

    /* Position to next line if any, or EOF */
    if (nl)
	l = nl + 1;
    else
	l = lend;

    return l;
}

/* Note:
 * Returns a swatch (a bit vector string) for a code point sequence
 * that starts from the value C<start> and comprises the number C<span>.
 * A C<swash> must be an object created by SWASHNEW (see lib/utf8_heavy.pl).
 * Should be used via swash_fetch, which will cache the swatch in C<swash>.
 */
STATIC SV*
S_swatch_get(pTHX_ SV* swash, UV start, UV span)
{
    SV *swatch;
    U8 *l, *lend, *x, *xend, *s, *send;
    STRLEN lcur, xcur, scur;
    HV *const hv = MUTABLE_HV(SvRV(swash));
    SV** const invlistsvp = hv_fetchs(hv, "INVLIST", FALSE);

    SV** listsvp = NULL; /* The string containing the main body of the table */
    SV** extssvp = NULL;
    SV** invert_it_svp = NULL;
    U8* typestr = NULL;
    STRLEN bits;
    STRLEN octets; /* if bits == 1, then octets == 0 */
    UV  none;
    UV  end = start + span;

    if (invlistsvp == NULL) {
        SV** const bitssvp = hv_fetchs(hv, "BITS", FALSE);
        SV** const nonesvp = hv_fetchs(hv, "NONE", FALSE);
        SV** const typesvp = hv_fetchs(hv, "TYPE", FALSE);
        extssvp = hv_fetchs(hv, "EXTRAS", FALSE);
        listsvp = hv_fetchs(hv, "LIST", FALSE);
        invert_it_svp = hv_fetchs(hv, "INVERT_IT", FALSE);

	bits  = SvUV(*bitssvp);
	none  = SvUV(*nonesvp);
	typestr = (U8*)SvPV_nolen(*typesvp);
    }
    else {
	bits = 1;
	none = 0;
    }
    octets = bits >> 3; /* if bits == 1, then octets == 0 */

    PERL_ARGS_ASSERT_SWATCH_GET;

    if (bits != 1 && bits != 8 && bits != 16 && bits != 32) {
	Perl_croak(aTHX_ "panic: swatch_get doesn't expect bits %"UVuf,
						 (UV)bits);
    }

    /* If overflowed, use the max possible */
    if (end < start) {
	end = UV_MAX;
	span = end - start;
    }

    /* create and initialize $swatch */
    scur   = octets ? (span * octets) : (span + 7) / 8;
    swatch = newSV(scur);
    SvPOK_on(swatch);
    s = (U8*)SvPVX(swatch);
    if (octets && none) {
	const U8* const e = s + scur;
	while (s < e) {
	    if (bits == 8)
		*s++ = (U8)(none & 0xff);
	    else if (bits == 16) {
		*s++ = (U8)((none >>  8) & 0xff);
		*s++ = (U8)( none        & 0xff);
	    }
	    else if (bits == 32) {
		*s++ = (U8)((none >> 24) & 0xff);
		*s++ = (U8)((none >> 16) & 0xff);
		*s++ = (U8)((none >>  8) & 0xff);
		*s++ = (U8)( none        & 0xff);
	    }
	}
	*s = '\0';
    }
    else {
	(void)memzero((U8*)s, scur + 1);
    }
    SvCUR_set(swatch, scur);
    s = (U8*)SvPVX(swatch);

    if (invlistsvp) {	/* If has an inversion list set up use that */
	_invlist_populate_swatch(*invlistsvp, start, end, s);
        return swatch;
    }

    /* read $swash->{LIST} */
    l = (U8*)SvPV(*listsvp, lcur);
    lend = l + lcur;
    while (l < lend) {
	UV min, max, val, upper;
	l = S_swash_scan_list_line(aTHX_ l, lend, &min, &max, &val,
					 cBOOL(octets), typestr);
	if (l > lend) {
	    break;
	}

	/* If looking for something beyond this range, go try the next one */
	if (max < start)
	    continue;

	/* <end> is generally 1 beyond where we want to set things, but at the
	 * platform's infinity, where we can't go any higher, we want to
	 * include the code point at <end> */
        upper = (max < end)
                ? max
                : (max != UV_MAX || end != UV_MAX)
                  ? end - 1
                  : end;

	if (octets) {
	    UV key;
	    if (min < start) {
		if (!none || val < none) {
		    val += start - min;
		}
		min = start;
	    }
	    for (key = min; key <= upper; key++) {
		STRLEN offset;
		/* offset must be non-negative (start <= min <= key < end) */
		offset = octets * (key - start);
		if (bits == 8)
		    s[offset] = (U8)(val & 0xff);
		else if (bits == 16) {
		    s[offset    ] = (U8)((val >>  8) & 0xff);
		    s[offset + 1] = (U8)( val        & 0xff);
		}
		else if (bits == 32) {
		    s[offset    ] = (U8)((val >> 24) & 0xff);
		    s[offset + 1] = (U8)((val >> 16) & 0xff);
		    s[offset + 2] = (U8)((val >>  8) & 0xff);
		    s[offset + 3] = (U8)( val        & 0xff);
		}

		if (!none || val < none)
		    ++val;
	    }
	}
	else { /* bits == 1, then val should be ignored */
	    UV key;
	    if (min < start)
		min = start;

	    for (key = min; key <= upper; key++) {
		const STRLEN offset = (STRLEN)(key - start);
		s[offset >> 3] |= 1 << (offset & 7);
	    }
	}
    } /* while */

    /* Invert if the data says it should be.  Assumes that bits == 1 */
    if (invert_it_svp && SvUV(*invert_it_svp)) {

	/* Unicode properties should come with all bits above PERL_UNICODE_MAX
	 * be 0, and their inversion should also be 0, as we don't succeed any
	 * Unicode property matches for non-Unicode code points */
	if (start <= PERL_UNICODE_MAX) {

	    /* The code below assumes that we never cross the
	     * Unicode/above-Unicode boundary in a range, as otherwise we would
	     * have to figure out where to stop flipping the bits.  Since this
	     * boundary is divisible by a large power of 2, and swatches comes
	     * in small powers of 2, this should be a valid assumption */
	    assert(start + span - 1 <= PERL_UNICODE_MAX);

	    send = s + scur;
	    while (s < send) {
		*s = ~(*s);
		s++;
	    }
	}
    }

    /* read $swash->{EXTRAS}
     * This code also copied to swash_to_invlist() below */
    x = (U8*)SvPV(*extssvp, xcur);
    xend = x + xcur;
    while (x < xend) {
	STRLEN namelen;
	U8 *namestr;
	SV** othersvp;
	HV* otherhv;
	STRLEN otherbits;
	SV **otherbitssvp, *other;
	U8 *s, *o, *nl;
	STRLEN slen, olen;

	const U8 opc = *x++;
	if (opc == '\n')
	    continue;

	nl = (U8*)memchr(x, '\n', xend - x);

	if (opc != '-' && opc != '+' && opc != '!' && opc != '&') {
	    if (nl) {
		x = nl + 1; /* 1 is length of "\n" */
		continue;
	    }
	    else {
		x = xend; /* to EXTRAS' end at which \n is not found */
		break;
	    }
	}

	namestr = x;
	if (nl) {
	    namelen = nl - namestr;
	    x = nl + 1;
	}
	else {
	    namelen = xend - namestr;
	    x = xend;
	}

	othersvp = hv_fetch(hv, (char *)namestr, namelen, FALSE);
	otherhv = MUTABLE_HV(SvRV(*othersvp));
	otherbitssvp = hv_fetchs(otherhv, "BITS", FALSE);
	otherbits = (STRLEN)SvUV(*otherbitssvp);
	if (bits < otherbits)
	    Perl_croak(aTHX_ "panic: swatch_get found swatch size mismatch, "
		       "bits=%"UVuf", otherbits=%"UVuf, (UV)bits, (UV)otherbits);

	/* The "other" swatch must be destroyed after. */
	other = swatch_get(*othersvp, start, span);
	o = (U8*)SvPV(other, olen);

	if (!olen)
	    Perl_croak(aTHX_ "panic: swatch_get got improper swatch");

	s = (U8*)SvPV(swatch, slen);
	if (bits == 1 && otherbits == 1) {
	    if (slen != olen)
		Perl_croak(aTHX_ "panic: swatch_get found swatch length "
			   "mismatch, slen=%"UVuf", olen=%"UVuf,
			   (UV)slen, (UV)olen);

	    switch (opc) {
	    case '+':
		while (slen--)
		    *s++ |= *o++;
		break;
	    case '!':
		while (slen--)
		    *s++ |= ~*o++;
		break;
	    case '-':
		while (slen--)
		    *s++ &= ~*o++;
		break;
	    case '&':
		while (slen--)
		    *s++ &= *o++;
		break;
	    default:
		break;
	    }
	}
	else {
	    STRLEN otheroctets = otherbits >> 3;
	    STRLEN offset = 0;
	    U8* const send = s + slen;

	    while (s < send) {
		UV otherval = 0;

		if (otherbits == 1) {
		    otherval = (o[offset >> 3] >> (offset & 7)) & 1;
		    ++offset;
		}
		else {
		    STRLEN vlen = otheroctets;
		    otherval = *o++;
		    while (--vlen) {
			otherval <<= 8;
			otherval |= *o++;
		    }
		}

		if (opc == '+' && otherval)
		    NOOP;   /* replace with otherval */
		else if (opc == '!' && !otherval)
		    otherval = 1;
		else if (opc == '-' && otherval)
		    otherval = 0;
		else if (opc == '&' && !otherval)
		    otherval = 0;
		else {
		    s += octets; /* no replacement */
		    continue;
		}

		if (bits == 8)
		    *s++ = (U8)( otherval & 0xff);
		else if (bits == 16) {
		    *s++ = (U8)((otherval >>  8) & 0xff);
		    *s++ = (U8)( otherval        & 0xff);
		}
		else if (bits == 32) {
		    *s++ = (U8)((otherval >> 24) & 0xff);
		    *s++ = (U8)((otherval >> 16) & 0xff);
		    *s++ = (U8)((otherval >>  8) & 0xff);
		    *s++ = (U8)( otherval        & 0xff);
		}
	    }
	}
	sv_free(other); /* through with it! */
    } /* while */
    return swatch;
}

HV*
Perl__swash_inversion_hash(pTHX_ SV* const swash)
{

   /* Subject to change or removal.  For use only in one place in regcomp.c.
    * Can't be used on a property that is subject to user override, as it
    * relies on the value of SPECIALS in the swash which would be set by
    * utf8_heavy.pl to the hash in the non-overriden file, and hence is not set
    * for overridden properties
    *
    * Returns a hash which is the inversion and closure of a swash mapping.
    * For example, consider the input lines:
    * 004B		006B
    * 004C		006C
    * 212A		006B
    *
    * The returned hash would have two keys, the utf8 for 006B and the utf8 for
    * 006C.  The value for each key is an array.  For 006C, the array would
    * have a two elements, the utf8 for itself, and for 004C.  For 006B, there
    * would be three elements in its array, the utf8 for 006B, 004B and 212A.
    *
    * Essentially, for any code point, it gives all the code points that map to
    * it, or the list of 'froms' for that point.
    *
    * Currently it ignores any additions or deletions from other swashes,
    * looking at just the main body of the swash, and if there are SPECIALS
    * in the swash, at that hash
    *
    * The specials hash can be extra code points, and most likely consists of
    * maps from single code points to multiple ones (each expressed as a string
    * of utf8 characters).   This function currently returns only 1-1 mappings.
    * However consider this possible input in the specials hash:
    * "\xEF\xAC\x85" => "\x{0073}\x{0074}",         # U+FB05 => 0073 0074
    * "\xEF\xAC\x86" => "\x{0073}\x{0074}",         # U+FB06 => 0073 0074
    *
    * Both FB05 and FB06 map to the same multi-char sequence, which we don't
    * currently handle.  But it also means that FB05 and FB06 are equivalent in
    * a 1-1 mapping which we should handle, and this relationship may not be in
    * the main table.  Therefore this function examines all the multi-char
    * sequences and adds the 1-1 mappings that come out of that.  */

    U8 *l, *lend;
    STRLEN lcur;
    HV *const hv = MUTABLE_HV(SvRV(swash));

    /* The string containing the main body of the table */
    SV** const listsvp = hv_fetchs(hv, "LIST", FALSE);

    SV** const typesvp = hv_fetchs(hv, "TYPE", FALSE);
    SV** const bitssvp = hv_fetchs(hv, "BITS", FALSE);
    SV** const nonesvp = hv_fetchs(hv, "NONE", FALSE);
    /*SV** const extssvp = hv_fetchs(hv, "EXTRAS", FALSE);*/
    const U8* const typestr = (U8*)SvPV_nolen(*typesvp);
    const STRLEN bits  = SvUV(*bitssvp);
    const STRLEN octets = bits >> 3; /* if bits == 1, then octets == 0 */
    const UV     none  = SvUV(*nonesvp);
    SV **specials_p = hv_fetchs(hv, "SPECIALS", 0);

    HV* ret = newHV();

    PERL_ARGS_ASSERT__SWASH_INVERSION_HASH;

    /* Must have at least 8 bits to get the mappings */
    if (bits != 8 && bits != 16 && bits != 32) {
	Perl_croak(aTHX_ "panic: swash_inversion_hash doesn't expect bits %"UVuf,
						 (UV)bits);
    }

    if (specials_p) { /* It might be "special" (sometimes, but not always, a
			mapping to more than one character */

	/* Construct an inverse mapping hash for the specials */
	HV * const specials_hv = MUTABLE_HV(SvRV(*specials_p));
	HV * specials_inverse = newHV();
	char *char_from; /* the lhs of the map */
	I32 from_len;   /* its byte length */
	char *char_to;  /* the rhs of the map */
	I32 to_len;	/* its byte length */
	SV *sv_to;	/* and in a sv */
	AV* from_list;  /* list of things that map to each 'to' */

	hv_iterinit(specials_hv);

	/* The keys are the characters (in utf8) that map to the corresponding
	 * utf8 string value.  Iterate through the list creating the inverse
	 * list. */
	while ((sv_to = hv_iternextsv(specials_hv, &char_from, &from_len))) {
	    SV** listp;
	    if (! SvPOK(sv_to)) {
		Perl_croak(aTHX_ "panic: value returned from hv_iternextsv() "
			   "unexpectedly is not a string, flags=%lu",
			   (unsigned long)SvFLAGS(sv_to));
	    }
	    /*DEBUG_U(PerlIO_printf(Perl_debug_log, "Found mapping from %"UVXf", First char of to is %"UVXf"\n", valid_utf8_to_uvchr((U8*) char_from, 0), valid_utf8_to_uvchr((U8*) SvPVX(sv_to), 0)));*/

	    /* Each key in the inverse list is a mapped-to value, and the key's
	     * hash value is a list of the strings (each in utf8) that map to
	     * it.  Those strings are all one character long */
	    if ((listp = hv_fetch(specials_inverse,
				    SvPVX(sv_to),
				    SvCUR(sv_to), 0)))
	    {
		from_list = (AV*) *listp;
	    }
	    else { /* No entry yet for it: create one */
		from_list = newAV();
		if (! hv_store(specials_inverse,
				SvPVX(sv_to),
				SvCUR(sv_to),
				(SV*) from_list, 0))
		{
		    Perl_croak(aTHX_ "panic: hv_store() unexpectedly failed");
		}
	    }

	    /* Here have the list associated with this 'to' (perhaps newly
	     * created and empty).  Just add to it.  Note that we ASSUME that
	     * the input is guaranteed to not have duplications, so we don't
	     * check for that.  Duplications just slow down execution time. */
	    av_push(from_list, newSVpvn_utf8(char_from, from_len, TRUE));
	}

	/* Here, 'specials_inverse' contains the inverse mapping.  Go through
	 * it looking for cases like the FB05/FB06 examples above.  There would
	 * be an entry in the hash like
	*	'st' => [ FB05, FB06 ]
	* In this example we will create two lists that get stored in the
	* returned hash, 'ret':
	*	FB05 => [ FB05, FB06 ]
	*	FB06 => [ FB05, FB06 ]
	*
	* Note that there is nothing to do if the array only has one element.
	* (In the normal 1-1 case handled below, we don't have to worry about
	* two lists, as everything gets tied to the single list that is
	* generated for the single character 'to'.  But here, we are omitting
	* that list, ('st' in the example), so must have multiple lists.) */
	while ((from_list = (AV *) hv_iternextsv(specials_inverse,
						 &char_to, &to_len)))
	{
	    if (av_len(from_list) > 0) {
		int i;

		/* We iterate over all combinations of i,j to place each code
		 * point on each list */
		for (i = 0; i <= av_len(from_list); i++) {
		    int j;
		    AV* i_list = newAV();
		    SV** entryp = av_fetch(from_list, i, FALSE);
		    if (entryp == NULL) {
			Perl_croak(aTHX_ "panic: av_fetch() unexpectedly failed");
		    }
		    if (hv_fetch(ret, SvPVX(*entryp), SvCUR(*entryp), FALSE)) {
			Perl_croak(aTHX_ "panic: unexpected entry for %s", SvPVX(*entryp));
		    }
		    if (! hv_store(ret, SvPVX(*entryp), SvCUR(*entryp),
				   (SV*) i_list, FALSE))
		    {
			Perl_croak(aTHX_ "panic: hv_store() unexpectedly failed");
		    }

		    /* For debugging: UV u = valid_utf8_to_uvchr((U8*) SvPVX(*entryp), 0);*/
		    for (j = 0; j <= av_len(from_list); j++) {
			entryp = av_fetch(from_list, j, FALSE);
			if (entryp == NULL) {
			    Perl_croak(aTHX_ "panic: av_fetch() unexpectedly failed");
			}

			/* When i==j this adds itself to the list */
			av_push(i_list, newSVuv(utf8_to_uvchr_buf(
					(U8*) SvPVX(*entryp),
					(U8*) SvPVX(*entryp) + SvCUR(*entryp),
					0)));
			/*DEBUG_U(PerlIO_printf(Perl_debug_log, "Adding %"UVXf" to list for %"UVXf"\n", valid_utf8_to_uvchr((U8*) SvPVX(*entryp), 0), u));*/
		    }
		}
	    }
	}
	SvREFCNT_dec(specials_inverse); /* done with it */
    } /* End of specials */

    /* read $swash->{LIST} */
    l = (U8*)SvPV(*listsvp, lcur);
    lend = l + lcur;

    /* Go through each input line */
    while (l < lend) {
	UV min, max, val;
	UV inverse;
	l = S_swash_scan_list_line(aTHX_ l, lend, &min, &max, &val,
					 cBOOL(octets), typestr);
	if (l > lend) {
	    break;
	}

	/* Each element in the range is to be inverted */
	for (inverse = min; inverse <= max; inverse++) {
	    AV* list;
	    SV** listp;
	    IV i;
	    bool found_key = FALSE;
	    bool found_inverse = FALSE;

	    /* The key is the inverse mapping */
	    char key[UTF8_MAXBYTES+1];
	    char* key_end = (char *) uvuni_to_utf8((U8*) key, val);
	    STRLEN key_len = key_end - key;

	    /* Get the list for the map */
	    if ((listp = hv_fetch(ret, key, key_len, FALSE))) {
		list = (AV*) *listp;
	    }
	    else { /* No entry yet for it: create one */
		list = newAV();
		if (! hv_store(ret, key, key_len, (SV*) list, FALSE)) {
		    Perl_croak(aTHX_ "panic: hv_store() unexpectedly failed");
		}
	    }

	    /* Look through list to see if this inverse mapping already is
	     * listed, or if there is a mapping to itself already */
	    for (i = 0; i <= av_len(list); i++) {
		SV** entryp = av_fetch(list, i, FALSE);
		SV* entry;
		if (entryp == NULL) {
		    Perl_croak(aTHX_ "panic: av_fetch() unexpectedly failed");
		}
		entry = *entryp;
		/*DEBUG_U(PerlIO_printf(Perl_debug_log, "list for %"UVXf" contains %"UVXf"\n", val, SvUV(entry)));*/
		if (SvUV(entry) == val) {
		    found_key = TRUE;
		}
		if (SvUV(entry) == inverse) {
		    found_inverse = TRUE;
		}

		/* No need to continue searching if found everything we are
		 * looking for */
		if (found_key && found_inverse) {
		    break;
		}
	    }

	    /* Make sure there is a mapping to itself on the list */
	    if (! found_key) {
		av_push(list, newSVuv(val));
		/*DEBUG_U(PerlIO_printf(Perl_debug_log, "Adding %"UVXf" to list for %"UVXf"\n", val, val));*/
	    }


	    /* Simply add the value to the list */
	    if (! found_inverse) {
		av_push(list, newSVuv(inverse));
		/*DEBUG_U(PerlIO_printf(Perl_debug_log, "Adding %"UVXf" to list for %"UVXf"\n", inverse, val));*/
	    }

	    /* swatch_get() increments the value of val for each element in the
	     * range.  That makes more compact tables possible.  You can
	     * express the capitalization, for example, of all consecutive
	     * letters with a single line: 0061\t007A\t0041 This maps 0061 to
	     * 0041, 0062 to 0042, etc.  I (khw) have never understood 'none',
	     * and it's not documented; it appears to be used only in
	     * implementing tr//; I copied the semantics from swatch_get(), just
	     * in case */
	    if (!none || val < none) {
		++val;
	    }
	}
    }

    return ret;
}

SV*
Perl__swash_to_invlist(pTHX_ SV* const swash)
{

   /* Subject to change or removal.  For use only in one place in regcomp.c */

    U8 *l, *lend;
    char *loc;
    STRLEN lcur;
    HV *const hv = MUTABLE_HV(SvRV(swash));
    UV elements = 0;    /* Number of elements in the inversion list */
    U8 empty[] = "";

    /* The string containing the main body of the table */
    SV** const listsvp = hv_fetchs(hv, "LIST", FALSE);
    SV** const typesvp = hv_fetchs(hv, "TYPE", FALSE);
    SV** const bitssvp = hv_fetchs(hv, "BITS", FALSE);
    SV** const extssvp = hv_fetchs(hv, "EXTRAS", FALSE);
    SV** const invert_it_svp = hv_fetchs(hv, "INVERT_IT", FALSE);

    const U8* const typestr = (U8*)SvPV_nolen(*typesvp);
    const STRLEN bits  = SvUV(*bitssvp);
    const STRLEN octets = bits >> 3; /* if bits == 1, then octets == 0 */
    U8 *x, *xend;
    STRLEN xcur;

    SV* invlist;

    PERL_ARGS_ASSERT__SWASH_TO_INVLIST;

    /* read $swash->{LIST} */
    if (SvPOK(*listsvp)) {
	l = (U8*)SvPV(*listsvp, lcur);
    }
    else {
	/* LIST legitimately doesn't contain a string during compilation phases
	 * of Perl itself, before the Unicode tables are generated.  In this
	 * case, just fake things up by creating an empty list */
	l = empty;
	lcur = 0;
    }
    loc = (char *) l;
    lend = l + lcur;

    /* Scan the input to count the number of lines to preallocate array size
     * based on worst possible case, which is each line in the input creates 2
     * elements in the inversion list: 1) the beginning of a range in the list;
     * 2) the beginning of a range not in the list.  */
    while ((loc = (strchr(loc, '\n'))) != NULL) {
	elements += 2;
	loc++;
    }

    /* If the ending is somehow corrupt and isn't a new line, add another
     * element for the final range that isn't in the inversion list */
    if (! (*lend == '\n'
	|| (*lend == '\0' && (lcur == 0 || *(lend - 1) == '\n'))))
    {
	elements++;
    }

    invlist = _new_invlist(elements);

    /* Now go through the input again, adding each range to the list */
    while (l < lend) {
	UV start, end;
	UV val;		/* Not used by this function */

	l = S_swash_scan_list_line(aTHX_ l, lend, &start, &end, &val,
					 cBOOL(octets), typestr);

	if (l > lend) {
	    break;
	}

	invlist = _add_range_to_invlist(invlist, start, end);
    }

    /* Invert if the data says it should be */
    if (invert_it_svp && SvUV(*invert_it_svp)) {
	_invlist_invert_prop(invlist);
    }

    /* This code is copied from swatch_get()
     * read $swash->{EXTRAS} */
    x = (U8*)SvPV(*extssvp, xcur);
    xend = x + xcur;
    while (x < xend) {
	STRLEN namelen;
	U8 *namestr;
	SV** othersvp;
	HV* otherhv;
	STRLEN otherbits;
	SV **otherbitssvp, *other;
	U8 *nl;

	const U8 opc = *x++;
	if (opc == '\n')
	    continue;

	nl = (U8*)memchr(x, '\n', xend - x);

	if (opc != '-' && opc != '+' && opc != '!' && opc != '&') {
	    if (nl) {
		x = nl + 1; /* 1 is length of "\n" */
		continue;
	    }
	    else {
		x = xend; /* to EXTRAS' end at which \n is not found */
		break;
	    }
	}

	namestr = x;
	if (nl) {
	    namelen = nl - namestr;
	    x = nl + 1;
	}
	else {
	    namelen = xend - namestr;
	    x = xend;
	}

	othersvp = hv_fetch(hv, (char *)namestr, namelen, FALSE);
	otherhv = MUTABLE_HV(SvRV(*othersvp));
	otherbitssvp = hv_fetchs(otherhv, "BITS", FALSE);
	otherbits = (STRLEN)SvUV(*otherbitssvp);

	if (bits != otherbits || bits != 1) {
	    Perl_croak(aTHX_ "panic: _swash_to_invlist only operates on boolean "
		       "properties, bits=%"UVuf", otherbits=%"UVuf,
		       (UV)bits, (UV)otherbits);
	}

	/* The "other" swatch must be destroyed after. */
	other = _swash_to_invlist((SV *)*othersvp);

	/* End of code copied from swatch_get() */
	switch (opc) {
	case '+':
	    _invlist_union(invlist, other, &invlist);
	    break;
	case '!':
	    _invlist_invert(other);
	    _invlist_union(invlist, other, &invlist);
	    break;
	case '-':
	    _invlist_subtract(invlist, other, &invlist);
	    break;
	case '&':
	    _invlist_intersection(invlist, other, &invlist);
	    break;
	default:
	    break;
	}
	sv_free(other); /* through with it! */
    }

    return invlist;
}

/*
=for apidoc uvchr_to_utf8

Adds the UTF-8 representation of the Native code point C<uv> to the end
of the string C<d>; C<d> should have at least C<UTF8_MAXBYTES+1> free
bytes available. The return value is the pointer to the byte after the
end of the new character. In other words,

    d = uvchr_to_utf8(d, uv);

is the recommended wide native character-aware way of saying

    *(d++) = uv;

=cut
*/

/* On ASCII machines this is normally a macro but we want a
   real function in case XS code wants it
*/
U8 *
Perl_uvchr_to_utf8(pTHX_ U8 *d, UV uv)
{
    PERL_ARGS_ASSERT_UVCHR_TO_UTF8;

    return Perl_uvuni_to_utf8_flags(aTHX_ d, NATIVE_TO_UNI(uv), 0);
}

U8 *
Perl_uvchr_to_utf8_flags(pTHX_ U8 *d, UV uv, UV flags)
{
    PERL_ARGS_ASSERT_UVCHR_TO_UTF8_FLAGS;

    return Perl_uvuni_to_utf8_flags(aTHX_ d, NATIVE_TO_UNI(uv), flags);
}

/*
=for apidoc utf8n_to_uvchr

Returns the native character value of the first character in the string
C<s>
which is assumed to be in UTF-8 encoding; C<retlen> will be set to the
length, in bytes, of that character.

C<length> and C<flags> are the same as L</utf8n_to_uvuni>().

=cut
*/
/* On ASCII machines this is normally a macro but we want
   a real function in case XS code wants it
*/
UV
Perl_utf8n_to_uvchr(pTHX_ const U8 *s, STRLEN curlen, STRLEN *retlen,
U32 flags)
{
    const UV uv = Perl_utf8n_to_uvuni(aTHX_ s, curlen, retlen, flags);

    PERL_ARGS_ASSERT_UTF8N_TO_UVCHR;

    return UNI_TO_NATIVE(uv);
}

bool
Perl_check_utf8_print(pTHX_ register const U8* s, const STRLEN len)
{
    /* May change: warns if surrogates, non-character code points, or
     * non-Unicode code points are in s which has length len bytes.  Returns
     * TRUE if none found; FALSE otherwise.  The only other validity check is
     * to make sure that this won't exceed the string's length */

    const U8* const e = s + len;
    bool ok = TRUE;

    PERL_ARGS_ASSERT_CHECK_UTF8_PRINT;

    while (s < e) {
	if (UTF8SKIP(s) > len) {
	    Perl_ck_warner_d(aTHX_ packWARN(WARN_UTF8),
			   "%s in %s", unees, PL_op ? OP_DESC(PL_op) : "print");
	    return FALSE;
	}
	if (UNLIKELY(*s >= UTF8_FIRST_PROBLEMATIC_CODE_POINT_FIRST_BYTE)) {
	    STRLEN char_len;
	    if (UTF8_IS_SUPER(s)) {
		if (ckWARN_d(WARN_NON_UNICODE)) {
		    UV uv = utf8_to_uvchr_buf(s, e, &char_len);
		    Perl_warner(aTHX_ packWARN(WARN_NON_UNICODE),
			"Code point 0x%04"UVXf" is not Unicode, may not be portable", uv);
		    ok = FALSE;
		}
	    }
	    else if (UTF8_IS_SURROGATE(s)) {
		if (ckWARN_d(WARN_SURROGATE)) {
		    UV uv = utf8_to_uvchr_buf(s, e, &char_len);
		    Perl_warner(aTHX_ packWARN(WARN_SURROGATE),
			"Unicode surrogate U+%04"UVXf" is illegal in UTF-8", uv);
		    ok = FALSE;
		}
	    }
	    else if
		((UTF8_IS_NONCHAR_GIVEN_THAT_NON_SUPER_AND_GE_PROBLEMATIC(s))
		 && (ckWARN_d(WARN_NONCHAR)))
	    {
		UV uv = utf8_to_uvchr_buf(s, e, &char_len);
		Perl_warner(aTHX_ packWARN(WARN_NONCHAR),
		    "Unicode non-character U+%04"UVXf" is illegal for open interchange", uv);
		ok = FALSE;
	    }
	}
	s += UTF8SKIP(s);
    }

    return ok;
}

/*
=for apidoc pv_uni_display

Build to the scalar C<dsv> a displayable version of the string C<spv>,
length C<len>, the displayable version being at most C<pvlim> bytes long
(if longer, the rest is truncated and "..." will be appended).

The C<flags> argument can have UNI_DISPLAY_ISPRINT set to display
isPRINT()able characters as themselves, UNI_DISPLAY_BACKSLASH
to display the \\[nrfta\\] as the backslashed versions (like '\n')
(UNI_DISPLAY_BACKSLASH is preferred over UNI_DISPLAY_ISPRINT for \\).
UNI_DISPLAY_QQ (and its alias UNI_DISPLAY_REGEX) have both
UNI_DISPLAY_BACKSLASH and UNI_DISPLAY_ISPRINT turned on.

The pointer to the PV of the C<dsv> is returned.

=cut */
char *
Perl_pv_uni_display(pTHX_ SV *dsv, const U8 *spv, STRLEN len, STRLEN pvlim, UV flags)
{
    int truncated = 0;
    const char *s, *e;

    PERL_ARGS_ASSERT_PV_UNI_DISPLAY;

    sv_setpvs(dsv, "");
    SvUTF8_off(dsv);
    for (s = (const char *)spv, e = s + len; s < e; s += UTF8SKIP(s)) {
	 UV u;
	  /* This serves double duty as a flag and a character to print after
	     a \ when flags & UNI_DISPLAY_BACKSLASH is true.
	  */
	 char ok = 0;

	 if (pvlim && SvCUR(dsv) >= pvlim) {
	      truncated++;
	      break;
	 }
	 u = utf8_to_uvchr_buf((U8*)s, (U8*)e, 0);
	 if (u < 256) {
	     const unsigned char c = (unsigned char)u & 0xFF;
	     if (flags & UNI_DISPLAY_BACKSLASH) {
	         switch (c) {
		 case '\n':
		     ok = 'n'; break;
		 case '\r':
		     ok = 'r'; break;
		 case '\t':
		     ok = 't'; break;
		 case '\f':
		     ok = 'f'; break;
		 case '\a':
		     ok = 'a'; break;
		 case '\\':
		     ok = '\\'; break;
		 default: break;
		 }
		 if (ok) {
		     const char string = ok;
		     sv_catpvs(dsv, "\\");
		     sv_catpvn(dsv, &string, 1);
		 }
	     }
	     /* isPRINT() is the locale-blind version. */
	     if (!ok && (flags & UNI_DISPLAY_ISPRINT) && isPRINT(c)) {
		 const char string = c;
		 sv_catpvn(dsv, &string, 1);
		 ok = 1;
	     }
	 }
	 if (!ok)
	     Perl_sv_catpvf(aTHX_ dsv, "\\x{%"UVxf"}", u);
    }
    if (truncated)
	 sv_catpvs(dsv, "...");

    return SvPVX(dsv);
}

/*
=for apidoc sv_uni_display

Build to the scalar C<dsv> a displayable version of the scalar C<sv>,
the displayable version being at most C<pvlim> bytes long
(if longer, the rest is truncated and "..." will be appended).

The C<flags> argument is as in L</pv_uni_display>().

The pointer to the PV of the C<dsv> is returned.

=cut
*/
char *
Perl_sv_uni_display(pTHX_ SV *dsv, SV *ssv, STRLEN pvlim, UV flags)
{
    PERL_ARGS_ASSERT_SV_UNI_DISPLAY;

     return Perl_pv_uni_display(aTHX_ dsv, (const U8*)SvPVX_const(ssv),
				SvCUR(ssv), pvlim, flags);
}

/*
=for apidoc foldEQ_utf8

Returns true if the leading portions of the strings C<s1> and C<s2> (either or both
of which may be in UTF-8) are the same case-insensitively; false otherwise.
How far into the strings to compare is determined by other input parameters.

If C<u1> is true, the string C<s1> is assumed to be in UTF-8-encoded Unicode;
otherwise it is assumed to be in native 8-bit encoding.  Correspondingly for C<u2>
with respect to C<s2>.

If the byte length C<l1> is non-zero, it says how far into C<s1> to check for fold
equality.  In other words, C<s1>+C<l1> will be used as a goal to reach.  The
scan will not be considered to be a match unless the goal is reached, and
scanning won't continue past that goal.  Correspondingly for C<l2> with respect to
C<s2>.

If C<pe1> is non-NULL and the pointer it points to is not NULL, that pointer is
considered an end pointer beyond which scanning of C<s1> will not continue under
any circumstances.  This means that if both C<l1> and C<pe1> are specified, and
C<pe1>
is less than C<s1>+C<l1>, the match will never be successful because it can
never
get as far as its goal (and in fact is asserted against).  Correspondingly for
C<pe2> with respect to C<s2>.

At least one of C<s1> and C<s2> must have a goal (at least one of C<l1> and
C<l2> must be non-zero), and if both do, both have to be
reached for a successful match.   Also, if the fold of a character is multiple
characters, all of them must be matched (see tr21 reference below for
'folding').

Upon a successful match, if C<pe1> is non-NULL,
it will be set to point to the beginning of the I<next> character of C<s1>
beyond what was matched.  Correspondingly for C<pe2> and C<s2>.

For case-insensitiveness, the "casefolding" of Unicode is used
instead of upper/lowercasing both the characters, see
L<http://www.unicode.org/unicode/reports/tr21/> (Case Mappings).

=cut */

/* A flags parameter has been added which may change, and hence isn't
 * externally documented.  Currently it is:
 *  0 for as-documented above
 *  FOLDEQ_UTF8_NOMIX_ASCII meaning that if a non-ASCII character folds to an
			    ASCII one, to not match
 *  FOLDEQ_UTF8_LOCALE	    meaning that locale rules are to be used for code
 *			    points below 256; unicode rules for above 255; and
 *			    folds that cross those boundaries are disallowed,
 *			    like the NOMIX_ASCII option
 *  FOLDEQ_S1_ALREADY_FOLDED s1 has already been folded before calling this
 *                           routine.  This allows that step to be skipped.
 *  FOLDEQ_S2_ALREADY_FOLDED   Similarly.
 */
I32
Perl_foldEQ_utf8_flags(pTHX_ const char *s1, char **pe1, register UV l1, bool u1, const char *s2, char **pe2, register UV l2, bool u2, U32 flags)
{
    dVAR;
    register const U8 *p1  = (const U8*)s1; /* Point to current char */
    register const U8 *p2  = (const U8*)s2;
    register const U8 *g1 = NULL;       /* goal for s1 */
    register const U8 *g2 = NULL;
    register const U8 *e1 = NULL;       /* Don't scan s1 past this */
    register U8 *f1 = NULL;             /* Point to current folded */
    register const U8 *e2 = NULL;
    register U8 *f2 = NULL;
    STRLEN n1 = 0, n2 = 0;              /* Number of bytes in current char */
    U8 foldbuf1[UTF8_MAXBYTES_CASE+1];
    U8 foldbuf2[UTF8_MAXBYTES_CASE+1];

    PERL_ARGS_ASSERT_FOLDEQ_UTF8_FLAGS;

    /* The algorithm requires that input with the flags on the first line of
     * the assert not be pre-folded. */
    assert( ! ((flags & (FOLDEQ_UTF8_NOMIX_ASCII | FOLDEQ_UTF8_LOCALE))
	&& (flags & (FOLDEQ_S1_ALREADY_FOLDED | FOLDEQ_S2_ALREADY_FOLDED))));

    if (pe1) {
        e1 = *(U8**)pe1;
    }

    if (l1) {
        g1 = (const U8*)s1 + l1;
    }

    if (pe2) {
        e2 = *(U8**)pe2;
    }

    if (l2) {
        g2 = (const U8*)s2 + l2;
    }

    /* Must have at least one goal */
    assert(g1 || g2);

    if (g1) {

        /* Will never match if goal is out-of-bounds */
        assert(! e1  || e1 >= g1);

        /* Here, there isn't an end pointer, or it is beyond the goal.  We
        * only go as far as the goal */
        e1 = g1;
    }
    else {
	assert(e1);    /* Must have an end for looking at s1 */
    }

    /* Same for goal for s2 */
    if (g2) {
        assert(! e2  || e2 >= g2);
        e2 = g2;
    }
    else {
	assert(e2);
    }

    /* If both operands are already folded, we could just do a memEQ on the
     * whole strings at once, but it would be better if the caller realized
     * this and didn't even call us */

    /* Look through both strings, a character at a time */
    while (p1 < e1 && p2 < e2) {

        /* If at the beginning of a new character in s1, get its fold to use
	 * and the length of the fold.  (exception: locale rules just get the
	 * character to a single byte) */
        if (n1 == 0) {
	    if (flags & FOLDEQ_S1_ALREADY_FOLDED) {
		f1 = (U8 *) p1;
		n1 = UTF8SKIP(f1);
	    }

	    else {
		/* If in locale matching, we use two sets of rules, depending
		 * on if the code point is above or below 255.  Here, we test
		 * for and handle locale rules */
		if ((flags & FOLDEQ_UTF8_LOCALE)
		    && (! u1 || UTF8_IS_INVARIANT(*p1)
			|| UTF8_IS_DOWNGRADEABLE_START(*p1)))
		{
		    /* There is no mixing of code points above and below 255. */
		    if (u2 && (! UTF8_IS_INVARIANT(*p2)
			&& ! UTF8_IS_DOWNGRADEABLE_START(*p2)))
		    {
			return 0;
		    }

		    /* We handle locale rules by converting, if necessary, the
		     * code point to a single byte. */
		    if (! u1 || UTF8_IS_INVARIANT(*p1)) {
			*foldbuf1 = *p1;
		    }
		    else {
			*foldbuf1 = TWO_BYTE_UTF8_TO_UNI(*p1, *(p1 + 1));
		    }
		    n1 = 1;
		}
		else if (isASCII(*p1)) {    /* Note, that here won't be both
					       ASCII and using locale rules */

		    /* If trying to mix non- with ASCII, and not supposed to,
		     * fail */
		    if ((flags & FOLDEQ_UTF8_NOMIX_ASCII) && ! isASCII(*p2)) {
			return 0;
		    }
		    n1 = 1;
		    *foldbuf1 = toLOWER(*p1);   /* Folds in the ASCII range are
						   just lowercased */
		}
		else if (u1) {
		    to_utf8_fold(p1, foldbuf1, &n1);
		}
		else {  /* Not utf8, get utf8 fold */
		    to_uni_fold(NATIVE_TO_UNI(*p1), foldbuf1, &n1);
		}
		f1 = foldbuf1;
	    }
        }

        if (n2 == 0) {    /* Same for s2 */
	    if (flags & FOLDEQ_S2_ALREADY_FOLDED) {
		f2 = (U8 *) p2;
		n2 = UTF8SKIP(f2);
	    }
	    else {
		if ((flags & FOLDEQ_UTF8_LOCALE)
		    && (! u2 || UTF8_IS_INVARIANT(*p2) || UTF8_IS_DOWNGRADEABLE_START(*p2)))
		{
		    /* Here, the next char in s2 is < 256.  We've already
		     * worked on s1, and if it isn't also < 256, can't match */
		    if (u1 && (! UTF8_IS_INVARIANT(*p1)
			&& ! UTF8_IS_DOWNGRADEABLE_START(*p1)))
		    {
			return 0;
		    }
		    if (! u2 || UTF8_IS_INVARIANT(*p2)) {
			*foldbuf2 = *p2;
		    }
		    else {
			*foldbuf2 = TWO_BYTE_UTF8_TO_UNI(*p2, *(p2 + 1));
		    }

		    /* Use another function to handle locale rules.  We've made
		     * sure that both characters to compare are single bytes */
		    if (! foldEQ_locale((char *) f1, (char *) foldbuf2, 1)) {
			return 0;
		    }
		    n1 = n2 = 0;
		}
		else if (isASCII(*p2)) {
		    if ((flags & FOLDEQ_UTF8_NOMIX_ASCII) && ! isASCII(*p1)) {
			return 0;
		    }
		    n2 = 1;
		    *foldbuf2 = toLOWER(*p2);
		}
		else if (u2) {
		    to_utf8_fold(p2, foldbuf2, &n2);
		}
		else {
		    to_uni_fold(NATIVE_TO_UNI(*p2), foldbuf2, &n2);
		}
		f2 = foldbuf2;
	    }
        }

	/* Here f1 and f2 point to the beginning of the strings to compare.
	 * These strings are the folds of the next character from each input
	 * string, stored in utf8. */

        /* While there is more to look for in both folds, see if they
        * continue to match */
        while (n1 && n2) {
            U8 fold_length = UTF8SKIP(f1);
            if (fold_length != UTF8SKIP(f2)
                || (fold_length == 1 && *f1 != *f2) /* Short circuit memNE
                                                       function call for single
                                                       byte */
                || memNE((char*)f1, (char*)f2, fold_length))
            {
                return 0; /* mismatch */
            }

            /* Here, they matched, advance past them */
            n1 -= fold_length;
            f1 += fold_length;
            n2 -= fold_length;
            f2 += fold_length;
        }

        /* When reach the end of any fold, advance the input past it */
        if (n1 == 0) {
            p1 += u1 ? UTF8SKIP(p1) : 1;
        }
        if (n2 == 0) {
            p2 += u2 ? UTF8SKIP(p2) : 1;
        }
    } /* End of loop through both strings */

    /* A match is defined by each scan that specified an explicit length
    * reaching its final goal, and the other not having matched a partial
    * character (which can happen when the fold of a character is more than one
    * character). */
    if (! ((g1 == 0 || p1 == g1) && (g2 == 0 || p2 == g2)) || n1 || n2) {
        return 0;
    }

    /* Successful match.  Set output pointers */
    if (pe1) {
        *pe1 = (char*)p1;
    }
    if (pe2) {
        *pe2 = (char*)p2;
    }
    return 1;
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
