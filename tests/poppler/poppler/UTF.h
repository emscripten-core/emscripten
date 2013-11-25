//========================================================================
//
// UTF.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2012 Adrian Johnson <ajohnson@redneon.com>
//
//========================================================================

#ifndef UTF_H
#define UTF_H

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "goo/GooString.h"
#include "CharTypes.h"

// Convert a UTF-16 string to a UCS-4
//   utf16      - utf16 bytes
//   utf16_len  - number of UTF-16 characters
//   ucs4_out   - if not NULL, allocates and returns UCS-4 string. Free with gfree.
//   returns number of UCS-4 characters
int UTF16toUCS4(const Unicode *utf16, int utf16_len, Unicode **ucs4_out);

// Convert a PDF Text String to UCS-4
//   s          - PDF text string
//   ucs4       - if the number of UCS-4 characters is > 0, allocates and
//                returns UCS-4 string. Free with gfree.
//   returns number of UCS-4 characters
int TextStringToUCS4(GooString *textStr, Unicode **ucs4);

// check if UCS-4 character is valid
bool UnicodeIsValid(Unicode ucs4);


#endif
