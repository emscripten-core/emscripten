//========================================================================
//
// GfxFont.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// Copyright (C) 2005, 2006, 2008-2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005, 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2006 Takashi Iwai <tiwai@suse.de>
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2007 Koji Otani <sho@bbr.jp>
// Copyright (C) 2007 Ed Catmur <ed@catmur.co.uk>
// Copyright (C) 2008 Jonathan Kew <jonathan_kew@sil.org>
// Copyright (C) 2008 Ed Avis <eda@waniasset.com>
// Copyright (C) 2008, 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2009 Peter Kerzum <kerzum@yandex-team.ru>
// Copyright (C) 2009, 2010 David Benjamin <davidben@mit.edu>
// Copyright (C) 2011 Axel Strübing <axel.struebing@freenet.de>
// Copyright (C) 2011 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <algorithm>
#include "goo/gmem.h"
#include "Error.h"
#include "Object.h"
#include "Dict.h"
#include "GlobalParams.h"
#include "CMap.h"
#include "CharCodeToUnicode.h"
#include "FontEncodingTables.h"
#include "BuiltinFontTables.h"
#include <fofi/FoFiType1.h>
#include <fofi/FoFiType1C.h>
#include <fofi/FoFiTrueType.h>
#include "GfxFont.h"

//------------------------------------------------------------------------

struct StdFontMapEntry {
  char *altName;
  char *properName;
};

// Acrobat 4.0 and earlier substituted Base14-compatible fonts without
// providing Widths and a FontDescriptor, so we munge the names into
// the proper Base14 names.  This table is from implementation note 44
// in the PDF 1.4 spec, with some additions based on empirical
// evidence.
static const StdFontMapEntry stdFontMap[] = {
  { "Arial",                        "Helvetica" },
  { "Arial,Bold",                   "Helvetica-Bold" },
  { "Arial,BoldItalic",             "Helvetica-BoldOblique" },
  { "Arial,Italic",                 "Helvetica-Oblique" },
  { "Arial-Bold",                   "Helvetica-Bold" },
  { "Arial-BoldItalic",             "Helvetica-BoldOblique" },
  { "Arial-BoldItalicMT",           "Helvetica-BoldOblique" },
  { "Arial-BoldMT",                 "Helvetica-Bold" },
  { "Arial-Italic",                 "Helvetica-Oblique" },
  { "Arial-ItalicMT",               "Helvetica-Oblique" },
  { "ArialMT",                      "Helvetica" },
  { "Courier,Bold",                 "Courier-Bold" },
  { "Courier,BoldItalic",           "Courier-BoldOblique" },
  { "Courier,Italic",               "Courier-Oblique" },
  { "CourierNew",                   "Courier" },
  { "CourierNew,Bold",              "Courier-Bold" },
  { "CourierNew,BoldItalic",        "Courier-BoldOblique" },
  { "CourierNew,Italic",            "Courier-Oblique" },
  { "CourierNew-Bold",              "Courier-Bold" },
  { "CourierNew-BoldItalic",        "Courier-BoldOblique" },
  { "CourierNew-Italic",            "Courier-Oblique" },
  { "CourierNewPS-BoldItalicMT",    "Courier-BoldOblique" },
  { "CourierNewPS-BoldMT",          "Courier-Bold" },
  { "CourierNewPS-ItalicMT",        "Courier-Oblique" },
  { "CourierNewPSMT",               "Courier" },
  { "Helvetica,Bold",               "Helvetica-Bold" },
  { "Helvetica,BoldItalic",         "Helvetica-BoldOblique" },
  { "Helvetica,Italic",             "Helvetica-Oblique" },
  { "Helvetica-BoldItalic",         "Helvetica-BoldOblique" },
  { "Helvetica-Italic",             "Helvetica-Oblique" },
  { "Symbol,Bold",                  "Symbol" },
  { "Symbol,BoldItalic",            "Symbol" },
  { "Symbol,Italic",                "Symbol" },
  { "TimesNewRoman",                "Times-Roman" },
  { "TimesNewRoman,Bold",           "Times-Bold" },
  { "TimesNewRoman,BoldItalic",     "Times-BoldItalic" },
  { "TimesNewRoman,Italic",         "Times-Italic" },
  { "TimesNewRoman-Bold",           "Times-Bold" },
  { "TimesNewRoman-BoldItalic",     "Times-BoldItalic" },
  { "TimesNewRoman-Italic",         "Times-Italic" },
  { "TimesNewRomanPS",              "Times-Roman" },
  { "TimesNewRomanPS-Bold",         "Times-Bold" },
  { "TimesNewRomanPS-BoldItalic",   "Times-BoldItalic" },
  { "TimesNewRomanPS-BoldItalicMT", "Times-BoldItalic" },
  { "TimesNewRomanPS-BoldMT",       "Times-Bold" },
  { "TimesNewRomanPS-Italic",       "Times-Italic" },
  { "TimesNewRomanPS-ItalicMT",     "Times-Italic" },
  { "TimesNewRomanPSMT",            "Times-Roman" },
  { "TimesNewRomanPSMT,Bold",       "Times-Bold" },
  { "TimesNewRomanPSMT,BoldItalic", "Times-BoldItalic" },
  { "TimesNewRomanPSMT,Italic",     "Times-Italic" }
};

static int parseCharName(char *charName, Unicode *uBuf, int uLen,
			 GBool names, GBool ligatures, 
			 GBool numeric, GBool hex, GBool variants);

//------------------------------------------------------------------------
// GfxFont
//------------------------------------------------------------------------

GfxFont *GfxFont::makeFont(XRef *xref, char *tagA, Ref idA, Dict *fontDict) {
  GooString *nameA;
  GfxFont *font;
  Object obj1;

  // get base font name
  nameA = NULL;
  fontDict->lookup("BaseFont", &obj1);
  if (obj1.isName()) {
    nameA = new GooString(obj1.getName());
  }
  obj1.free();

  // get font type
  font = NULL;
  fontDict->lookup("Subtype", &obj1);
  if (obj1.isName("Type1") || obj1.isName("MMType1")) {
    font = new Gfx8BitFont(xref, tagA, idA, nameA, fontType1, fontDict);
  } else if (obj1.isName("Type1C")) {
    font = new Gfx8BitFont(xref, tagA, idA, nameA, fontType1C, fontDict);
  } else if (obj1.isName("Type3")) {
    font = new Gfx8BitFont(xref, tagA, idA, nameA, fontType3, fontDict);
  } else if (obj1.isName("TrueType")) {
    font = new Gfx8BitFont(xref, tagA, idA, nameA, fontTrueType, fontDict);
  } else if (obj1.isName("Type0")) {
    font = new GfxCIDFont(xref, tagA, idA, nameA, fontDict);
  } else {
    error(-1, "Unknown font type: '%s'",
	  obj1.isName() ? obj1.getName() : "???");
    font = new Gfx8BitFont(xref, tagA, idA, nameA, fontUnknownType, fontDict);
  }
  obj1.free();

  return font;
}

GfxFont::GfxFont(char *tagA, Ref idA, GooString *nameA) {
  ok = gFalse;
  tag = new GooString(tagA);
  id = idA;
  name = nameA;
  origName = nameA;
  embFontName = NULL;
  extFontFile = NULL;
  family = NULL;
  stretch = StretchNotDefined;
  weight = WeightNotDefined;
  refCnt = 1;
  dfp = NULL;
  hasToUnicode = gFalse;
}

GfxFont::~GfxFont() {
  delete tag;
  delete family;
  if (origName && origName != name) {
    delete origName;
  }
  if (name) {
    delete name;
  }
  if (embFontName) {
    delete embFontName;
  }
  if (extFontFile) {
    delete extFontFile;
  }
  delete dfp;
}

void GfxFont::incRefCnt() {
  refCnt++;
}

void GfxFont::decRefCnt() {
  if (--refCnt == 0)
    delete this;
}

void GfxFont::readFontDescriptor(XRef *xref, Dict *fontDict) {
  Object obj1, obj2, obj3, obj4;
  double t;
  int i;

  // assume Times-Roman by default (for substitution purposes)
  flags = fontSerif;

  embFontID.num = -1;
  embFontID.gen = -1;
  missingWidth = 0;

  if (fontDict->lookup("FontDescriptor", &obj1)->isDict()) {

    // get flags
    if (obj1.dictLookup("Flags", &obj2)->isInt()) {
      flags = obj2.getInt();
    }
    obj2.free();

    // get name
    obj1.dictLookup("FontName", &obj2);
    if (obj2.isName()) {
      embFontName = new GooString(obj2.getName());
    }
    obj2.free();
    if (embFontName == NULL) {
      // get name with typo
      obj1.dictLookup("Fontname", &obj2);
      if (obj2.isName()) {
        embFontName = new GooString(obj2.getName());
        error(-1, "The file uses Fontname instead of FontName please notify the creator that the file is broken");
      }
      obj2.free();
    }

    // get family
    obj1.dictLookup("FontFamily", &obj2);
    if (obj2.isString()) family = new GooString(obj2.getString());
    obj2.free();

    // get stretch
    obj1.dictLookup("FontStretch", &obj2);
    if (obj2.isName()) {
      if (strcmp(obj2.getName(), "UltraCondensed") == 0) stretch = UltraCondensed;
      else if (strcmp(obj2.getName(), "ExtraCondensed") == 0) stretch = ExtraCondensed;
      else if (strcmp(obj2.getName(), "Condensed") == 0) stretch = Condensed;
      else if (strcmp(obj2.getName(), "SemiCondensed") == 0) stretch = SemiCondensed;
      else if (strcmp(obj2.getName(), "Normal") == 0) stretch = Normal;
      else if (strcmp(obj2.getName(), "SemiExpanded") == 0) stretch = SemiExpanded;
      else if (strcmp(obj2.getName(), "Expanded") == 0) stretch = Expanded;
      else if (strcmp(obj2.getName(), "ExtraExpanded") == 0) stretch = ExtraExpanded;
      else if (strcmp(obj2.getName(), "UltraExpanded") == 0) stretch = UltraExpanded;
      else error(-1, "Invalid Font Stretch");
    }
    obj2.free();
    
    // get weight
    obj1.dictLookup("FontWeight", &obj2);
    if (obj2.isNum()) {
      if (obj2.getNum() == 100) weight = W100;
      else if (obj2.getNum() == 200) weight = W200;
      else if (obj2.getNum() == 300) weight = W300;
      else if (obj2.getNum() == 400) weight = W400;
      else if (obj2.getNum() == 500) weight = W500;
      else if (obj2.getNum() == 600) weight = W600;
      else if (obj2.getNum() == 700) weight = W700;
      else if (obj2.getNum() == 800) weight = W800;
      else if (obj2.getNum() == 900) weight = W900;
      else error(-1, "Invalid Font Weight");
    }
    obj2.free();

    // look for embedded font file
    if (obj1.dictLookupNF("FontFile", &obj2)->isRef()) {
      embFontID = obj2.getRef();
      if (type != fontType1) {
	error(-1, "Mismatch between font type and embedded font file");
	if (isCIDFont()) {
	  error(-1, "CID font has FontFile attribute; assuming CIDType0");
	  type = fontCIDType0;
	} else {
	  type = fontType1;
	}
      }
    }
    obj2.free();
    if (embFontID.num == -1 &&
	obj1.dictLookupNF("FontFile2", &obj2)->isRef()) {
      embFontID = obj2.getRef();
      if (type != fontTrueType && type != fontCIDType2) {
	error(-1, "Mismatch between font type and embedded font file");
	type = isCIDFont() ? fontCIDType2 : fontTrueType;
      }
    }
    obj2.free();
    if (embFontID.num == -1 &&
	obj1.dictLookupNF("FontFile3", &obj2)->isRef()) {
      if (obj2.fetch(xref, &obj3)->isStream()) {
	obj3.streamGetDict()->lookup("Subtype", &obj4);
	if (obj4.isName("Type1")) {
	  embFontID = obj2.getRef();
	  if (type != fontType1) {
	    error(-1, "Mismatch between font type and embedded font file");
	    if (isCIDFont()) {
	      error(-1, "Embedded CID font has type Type1; assuming CIDType0");
	      type = fontCIDType0;
	    } else {
	      type = fontType1;
	    }
	  }
	} else if (obj4.isName("Type1C")) {
	  embFontID = obj2.getRef();
	  if (type != fontType1 && type != fontType1C) {
	    error(-1, "Mismatch between font type and embedded font file");
	  }
	  if (isCIDFont()) {
	    error(-1, "Embedded CID font has type Type1C; assuming CIDType0C");
	    type = fontCIDType0C;
	  } else {
	    type = fontType1C;
	  }
	} else if (obj4.isName("TrueType")) {
	  embFontID = obj2.getRef();
	  if (type != fontTrueType) {
	    error(-1, "Mismatch between font type and embedded font file");
	    if (isCIDFont()) {
	      error(-1, "Embedded CID font has type TrueType; assuming CIDType2");
	      type = fontCIDType2;
	    } else {
	      type = fontTrueType;
	    }
	  }
	} else if (obj4.isName("CIDFontType0C")) {
	  embFontID = obj2.getRef();
	  if (type != fontCIDType0) {
	    error(-1, "Mismatch between font type and embedded font file");
	  }
	  if (isCIDFont()) {
	    type = fontCIDType0C;
	  } else {
	    error(-1, "Embedded non-CID font has type CIDFontType0c; assuming Type1C");
	    type = fontType1C;
	  }
	} else if (obj4.isName("OpenType")) {
	  embFontID = obj2.getRef();
	  if (type == fontTrueType) {
	    type = fontTrueTypeOT;
	  } else if (type == fontType1) {
	    type = fontType1COT;
	  } else if (type == fontCIDType0) {
	    type = fontCIDType0COT;
	  } else if (type == fontCIDType2) {
	    type = fontCIDType2OT;
	  } else {
	    error(-1, "Mismatch between font type and embedded font file");
	  }
	} else {
	  error(-1, "Unknown embedded font type '%s'",
		obj4.isName() ? obj4.getName() : "???");
	}
	obj4.free();
      }
      obj3.free();
    }
    obj2.free();

    // look for MissingWidth
    obj1.dictLookup("MissingWidth", &obj2);
    if (obj2.isNum()) {
      missingWidth = obj2.getNum();
    }
    obj2.free();

    // get Ascent and Descent
    obj1.dictLookup("Ascent", &obj2);
    if (obj2.isNum()) {
      t = 0.001 * obj2.getNum();
      // some broken font descriptors set ascent and descent to 0
      if (t != 0) {
	ascent = t;
      }
    }
    obj2.free();
    obj1.dictLookup("Descent", &obj2);
    if (obj2.isNum()) {
      t = 0.001 * obj2.getNum();
      // some broken font descriptors set ascent and descent to 0
      if (t != 0) {
	descent = t;
      }
      // some broken font descriptors specify a positive descent
      if (descent > 0) {
	descent = -descent;
      }
    }
    obj2.free();

    // font FontBBox
    if (obj1.dictLookup("FontBBox", &obj2)->isArray()) {
      for (i = 0; i < 4 && i < obj2.arrayGetLength(); ++i) {
	if (obj2.arrayGet(i, &obj3)->isNum()) {
	  fontBBox[i] = 0.001 * obj3.getNum();
	}
	obj3.free();
      }
    }
    obj2.free();

  }
  obj1.free();
}

CharCodeToUnicode *GfxFont::readToUnicodeCMap(Dict *fontDict, int nBits,
					      CharCodeToUnicode *ctu) {
  GooString *buf;
  Object obj1;

  if (!fontDict->lookup("ToUnicode", &obj1)->isStream()) {
    obj1.free();
    return NULL;
  }
  buf = new GooString();
  obj1.getStream()->fillGooString(buf);
  obj1.streamClose();
  obj1.free();
  if (ctu) {
    ctu->mergeCMap(buf, nBits);
  } else {
    ctu = CharCodeToUnicode::parseCMap(buf, nBits);
  }
  hasToUnicode = gTrue;
  delete buf;
  return ctu;
}

void GfxFont::findExtFontFile() {
  static char *type1Exts[] = { ".pfa", ".pfb", ".ps", "", NULL };
  static char *ttExts[] = { ".ttf", ".ttc", NULL };

  if (name) {
    if (type == fontType1) {
      extFontFile = globalParams->findFontFile(name, type1Exts);
    } else if (type == fontTrueType) {
      extFontFile = globalParams->findFontFile(name, ttExts);
    }
  }
}

char *GfxFont::readExtFontFile(int *len) {
  FILE *f;
  char *buf;

  if (!(f = fopen(extFontFile->getCString(), "rb"))) {
    error(-1, "External font file '%s' vanished", extFontFile->getCString());
    return NULL;
  }
  if (fseek(f, 0, SEEK_END) != 0) {
    error(-1, "Cannot seek to end of '%s'", extFontFile->getCString());
    fclose(f);
    return NULL;
  }
  *len = (int)ftell(f);
  if (fseek(f, 0, SEEK_SET) != 0) {
    error(-1, "Cannot seek to start of '%s'", extFontFile->getCString());
    fclose(f);
    return NULL;
  }
  buf = (char *)gmalloc(*len);
  if ((int)fread(buf, 1, *len, f) != *len) {
    error(-1, "Error reading external font file '%s'",
	  extFontFile->getCString());
  }
  fclose(f);
  return buf;
}

char *GfxFont::readEmbFontFile(XRef *xref, int *len) {
  char *buf;
  Object obj1, obj2;
  Stream *str;

  obj1.initRef(embFontID.num, embFontID.gen);
  obj1.fetch(xref, &obj2);
  if (!obj2.isStream()) {
    error(-1, "Embedded font file is not a stream");
    obj2.free();
    obj1.free();
    embFontID.num = -1;
    *len = 0;
    return NULL;
  }
  str = obj2.getStream();

  buf = (char*)str->toUnsignedChars(len);
  str->close();

  obj2.free();
  obj1.free();

  return buf;
}

//------------------------------------------------------------------------
// Gfx8BitFont
//------------------------------------------------------------------------

Gfx8BitFont::Gfx8BitFont(XRef *xref, char *tagA, Ref idA, GooString *nameA,
			 GfxFontType typeA, Dict *fontDict):
  GfxFont(tagA, idA, nameA)
{
  GooString *name2;
  BuiltinFont *builtinFont;
  char **baseEnc;
  GBool baseEncFromFontFile;
  char *buf;
  int len;
  FoFiType1 *ffT1;
  FoFiType1C *ffT1C;
  int code;
  char *charName;
  GBool missing, hex;
  Unicode toUnicode[256];
  CharCodeToUnicode *utu, *ctu2;
  Unicode uBuf[8];
  double mul;
  int firstChar, lastChar;
  Gushort w;
  Object obj1, obj2, obj3;
  int n, i, a, b, m;

  refCnt = 1;
  type = typeA;
  ctu = NULL;

  // do font name substitution for various aliases of the Base 14 font
  // names
  if (name) {
    name2 = name->copy();
    i = 0;
    while (i < name2->getLength()) {
      if (name2->getChar(i) == ' ') {
	name2->del(i);
      } else {
	++i;
      }
    }
    a = 0;
    b = sizeof(stdFontMap) / sizeof(StdFontMapEntry);
    // invariant: stdFontMap[a].altName <= name2 < stdFontMap[b].altName
    while (b - a > 1) {
      m = (a + b) / 2;
      if (name2->cmp(stdFontMap[m].altName) >= 0) {
	a = m;
      } else {
	b = m;
      }
    }
    if (!name2->cmp(stdFontMap[a].altName)) {
      name = new GooString(stdFontMap[a].properName);
    }
    delete name2;
  }

  // is it a built-in font?
  builtinFont = NULL;
  if (name) {
    for (i = 0; i < nBuiltinFonts; ++i) {
      if (!name->cmp(builtinFonts[i].name)) {
	builtinFont = &builtinFonts[i];
	break;
      }
    }
  }

  // default ascent/descent values
  if (builtinFont) {
    ascent = 0.001 * builtinFont->ascent;
    descent = 0.001 * builtinFont->descent;
    fontBBox[0] = 0.001 * builtinFont->bbox[0];
    fontBBox[1] = 0.001 * builtinFont->bbox[1];
    fontBBox[2] = 0.001 * builtinFont->bbox[2];
    fontBBox[3] = 0.001 * builtinFont->bbox[3];
  } else {
    ascent = 0.95;
    descent = -0.35;
    fontBBox[0] = fontBBox[1] = fontBBox[2] = fontBBox[3] = 0;
  }

  // get info from font descriptor
  readFontDescriptor(xref, fontDict);

  // for non-embedded fonts, don't trust the ascent/descent/bbox
  // values from the font descriptor
  if (builtinFont && embFontID.num < 0) {
    ascent = 0.001 * builtinFont->ascent;
    descent = 0.001 * builtinFont->descent;
    fontBBox[0] = 0.001 * builtinFont->bbox[0];
    fontBBox[1] = 0.001 * builtinFont->bbox[1];
    fontBBox[2] = 0.001 * builtinFont->bbox[2];
    fontBBox[3] = 0.001 * builtinFont->bbox[3];
  }

  // look for an external font file
  findExtFontFile();

  // get font matrix
  fontMat[0] = fontMat[3] = 1;
  fontMat[1] = fontMat[2] = fontMat[4] = fontMat[5] = 0;
  if (fontDict->lookup("FontMatrix", &obj1)->isArray()) {
    for (i = 0; i < 6 && i < obj1.arrayGetLength(); ++i) {
      if (obj1.arrayGet(i, &obj2)->isNum()) {
	fontMat[i] = obj2.getNum();
      }
      obj2.free();
    }
  }
  obj1.free();

  // get Type 3 bounding box, font definition, and resources
  if (type == fontType3) {
    if (fontDict->lookup("FontBBox", &obj1)->isArray()) {
      for (i = 0; i < 4 && i < obj1.arrayGetLength(); ++i) {
	if (obj1.arrayGet(i, &obj2)->isNum()) {
	  fontBBox[i] = obj2.getNum();
	}
	obj2.free();
      }
    }
    obj1.free();
    if (!fontDict->lookup("CharProcs", &charProcs)->isDict()) {
      error(-1, "Missing or invalid CharProcs dictionary in Type 3 font");
      charProcs.free();
    }
    if (!fontDict->lookup("Resources", &resources)->isDict()) {
      resources.free();
    }
  }

  //----- build the font encoding -----

  // Encodings start with a base encoding, which can come from
  // (in order of priority):
  //   1. FontDict.Encoding or FontDict.Encoding.BaseEncoding
  //        - MacRoman / MacExpert / WinAnsi / Standard
  //   2. embedded or external font file
  //   3. default:
  //        - builtin --> builtin encoding
  //        - TrueType --> WinAnsiEncoding
  //        - others --> StandardEncoding
  // and then add a list of differences (if any) from
  // FontDict.Encoding.Differences.

  // check FontDict for base encoding
  hasEncoding = gFalse;
  usesMacRomanEnc = gFalse;
  baseEnc = NULL;
  baseEncFromFontFile = gFalse;
  fontDict->lookup("Encoding", &obj1);
  if (obj1.isDict()) {
    obj1.dictLookup("BaseEncoding", &obj2);
    if (obj2.isName("MacRomanEncoding")) {
      hasEncoding = gTrue;
      usesMacRomanEnc = gTrue;
      baseEnc = macRomanEncoding;
    } else if (obj2.isName("MacExpertEncoding")) {
      hasEncoding = gTrue;
      baseEnc = macExpertEncoding;
    } else if (obj2.isName("WinAnsiEncoding")) {
      hasEncoding = gTrue;
      baseEnc = winAnsiEncoding;
    }
    obj2.free();
  } else if (obj1.isName("MacRomanEncoding")) {
    hasEncoding = gTrue;
    usesMacRomanEnc = gTrue;
    baseEnc = macRomanEncoding;
  } else if (obj1.isName("MacExpertEncoding")) {
    hasEncoding = gTrue;
    baseEnc = macExpertEncoding;
  } else if (obj1.isName("WinAnsiEncoding")) {
    hasEncoding = gTrue;
    baseEnc = winAnsiEncoding;
  }

  // check embedded or external font file for base encoding
  // (only for Type 1 fonts - trying to get an encoding out of a
  // TrueType font is a losing proposition)
  ffT1 = NULL;
  ffT1C = NULL;
  buf = NULL;
  if (type == fontType1 && (extFontFile || embFontID.num >= 0)) {
    if (extFontFile) {
      ffT1 = FoFiType1::load(extFontFile->getCString());
    } else {
      buf = readEmbFontFile(xref, &len);
      ffT1 = FoFiType1::make(buf, len);
    }
    if (ffT1) {
      if (ffT1->getName()) {
	if (embFontName) {
	  delete embFontName;
	}
	embFontName = new GooString(ffT1->getName());
      }
      if (!baseEnc) {
	baseEnc = ffT1->getEncoding();
	baseEncFromFontFile = gTrue;
      }
    }
  } else if (type == fontType1C && (extFontFile || embFontID.num >= 0)) {
    if (extFontFile) {
      ffT1C = FoFiType1C::load(extFontFile->getCString());
    } else {
      buf = readEmbFontFile(xref, &len);
      ffT1C = FoFiType1C::make(buf, len);
    }
    if (ffT1C) {
      if (ffT1C->getName()) {
	if (embFontName) {
	  delete embFontName;
	}
	embFontName = new GooString(ffT1C->getName());
      }
      if (!baseEnc) {
	baseEnc = ffT1C->getEncoding();
	baseEncFromFontFile = gTrue;
      }
    }
  }
  if (buf) {
    gfree(buf);
  }

  // get default base encoding
  if (!baseEnc) {
    if (builtinFont && embFontID.num < 0) {
      baseEnc = builtinFont->defaultBaseEnc;
      hasEncoding = gTrue;
    } else if (type == fontTrueType) {
      baseEnc = winAnsiEncoding;
    } else {
      baseEnc = standardEncoding;
    }
  }

  // copy the base encoding
  for (i = 0; i < 256; ++i) {
    enc[i] = baseEnc[i];
    if ((encFree[i] = baseEncFromFontFile) && enc[i]) {
      enc[i] = copyString(baseEnc[i]);
    }
  }

  // some Type 1C font files have empty encodings, which can break the
  // T1C->T1 conversion (since the 'seac' operator depends on having
  // the accents in the encoding), so we fill in any gaps from
  // StandardEncoding
  if (type == fontType1C && (extFontFile || embFontID.num >= 0) &&
      baseEncFromFontFile) {
    for (i = 0; i < 256; ++i) {
      if (!enc[i] && standardEncoding[i]) {
	enc[i] = standardEncoding[i];
	encFree[i] = gFalse;
      }
    }
  }

  // merge differences into encoding
  if (obj1.isDict()) {
    obj1.dictLookup("Differences", &obj2);
    if (obj2.isArray()) {
      hasEncoding = gTrue;
      code = 0;
      for (i = 0; i < obj2.arrayGetLength(); ++i) {
	obj2.arrayGet(i, &obj3);
	if (obj3.isInt()) {
	  code = obj3.getInt();
	} else if (obj3.isName()) {
	  if (code >= 0 && code < 256) {
	    if (encFree[code]) {
	      gfree(enc[code]);
	    }
	    enc[code] = copyString(obj3.getName());
	    encFree[code] = gTrue;
	  }
	  ++code;
	} else {
	  error(-1, "Wrong type in font encoding resource differences (%s)",
		obj3.getTypeName());
	}
	obj3.free();
      }
    }
    obj2.free();
  }
  obj1.free();
  if (ffT1) {
    delete ffT1;
  }
  if (ffT1C) {
    delete ffT1C;
  }

  //----- build the mapping to Unicode -----

  // pass 1: use the name-to-Unicode mapping table
  missing = hex = gFalse;
  for (code = 0; code < 256; ++code) {
    if ((charName = enc[code])) {
      if (!(toUnicode[code] = globalParams->mapNameToUnicode(charName)) &&
	  strcmp(charName, ".notdef")) {
	// if it wasn't in the name-to-Unicode table, check for a
	// name that looks like 'Axx' or 'xx', where 'A' is any letter
	// and 'xx' is two hex digits
	if ((strlen(charName) == 3 &&
	     isalpha(charName[0]) &&
	     isxdigit(charName[1]) && isxdigit(charName[2]) &&
	     ((charName[1] >= 'a' && charName[1] <= 'f') ||
	      (charName[1] >= 'A' && charName[1] <= 'F') ||
	      (charName[2] >= 'a' && charName[2] <= 'f') ||
	      (charName[2] >= 'A' && charName[2] <= 'F'))) ||
	    (strlen(charName) == 2 &&
	     isxdigit(charName[0]) && isxdigit(charName[1]) &&
	     ((charName[0] >= 'a' && charName[0] <= 'f') ||
	      (charName[0] >= 'A' && charName[0] <= 'F') ||
	      (charName[1] >= 'a' && charName[1] <= 'f') ||
	      (charName[1] >= 'A' && charName[1] <= 'F')))) {
	  hex = gTrue;
	}
	missing = gTrue;
      }
    } else {
      toUnicode[code] = 0;
    }
  }

  // construct the char code -> Unicode mapping object
  ctu = CharCodeToUnicode::make8BitToUnicode(toUnicode);

  // pass 2: try to fill in the missing chars, looking for ligatures, numeric
  // references and variants
  if (missing) {
    for (code = 0; code < 256; ++code) {
      if (!toUnicode[code]) {
	if ((charName = enc[code]) && strcmp(charName, ".notdef")
	    && (n = parseCharName(charName, uBuf, sizeof(uBuf)/sizeof(*uBuf), 
				  gFalse, // don't check simple names (pass 1)
				  gTrue, // do check ligatures
				  globalParams->getMapNumericCharNames(),
				  hex,
				  gTrue))) { // do check variants
	  ctu->setMapping((CharCode)code, uBuf, n);
	} else if (globalParams->getMapUnknownCharNames()) {
	  // if the 'mapUnknownCharNames' flag is set, do a simple pass-through
	  // mapping for unknown character names
	  if (charName && charName[0]) {
	    for (n = 0; n < (int)(sizeof(uBuf)/sizeof(*uBuf)); ++n)
	      if (!(uBuf[n] = charName[n]))
		break;
	    ctu->setMapping((CharCode)code, uBuf, n);
	  } else {
	    uBuf[0] = code;
	    ctu->setMapping((CharCode)code, uBuf, 1);
	  }
	}
      }
    }
  }

  // merge in a ToUnicode CMap, if there is one -- this overwrites
  // existing entries in ctu, i.e., the ToUnicode CMap takes
  // precedence, but the other encoding info is allowed to fill in any
  // holes
  readToUnicodeCMap(fontDict, 8, ctu);

  // look for a Unicode-to-Unicode mapping
  if (name && (utu = globalParams->getUnicodeToUnicode(name))) {
    Unicode *uAux;
    for (i = 0; i < 256; ++i) {
      toUnicode[i] = 0;
    }
    ctu2 = CharCodeToUnicode::make8BitToUnicode(toUnicode);
    for (i = 0; i < 256; ++i) {
      n = ctu->mapToUnicode((CharCode)i, &uAux);
      if (n >= 1) {
	n = utu->mapToUnicode((CharCode)uAux[0], &uAux);
	if (n >= 1) {
	  ctu2->setMapping((CharCode)i, uAux, n);
	}
      }
    }
    utu->decRefCnt();
    delete ctu;
    ctu = ctu2;
  }

  //----- get the character widths -----

  // initialize all widths
  for (code = 0; code < 256; ++code) {
    widths[code] = missingWidth * 0.001;
  }

  // use widths from font dict, if present
  fontDict->lookup("FirstChar", &obj1);
  firstChar = obj1.isInt() ? obj1.getInt() : 0;
  obj1.free();
  if (firstChar < 0 || firstChar > 255) {
    firstChar = 0;
  }
  fontDict->lookup("LastChar", &obj1);
  lastChar = obj1.isInt() ? obj1.getInt() : 255;
  obj1.free();
  if (lastChar < 0 || lastChar > 255) {
    lastChar = 255;
  }
  mul = (type == fontType3) ? fontMat[0] : 0.001;
  fontDict->lookup("Widths", &obj1);
  if (obj1.isArray()) {
    flags |= fontFixedWidth;
    if (obj1.arrayGetLength() < lastChar - firstChar + 1) {
      lastChar = firstChar + obj1.arrayGetLength() - 1;
    }
    for (code = firstChar; code <= lastChar; ++code) {
      obj1.arrayGet(code - firstChar, &obj2);
      if (obj2.isNum()) {
	widths[code] = obj2.getNum() * mul;
	if (widths[code] != widths[firstChar]) {
	  flags &= ~fontFixedWidth;
	}
      }
      obj2.free();
    }

  // use widths from built-in font
  } else if (builtinFont) {
    // this is a kludge for broken PDF files that encode char 32
    // as .notdef
    if (builtinFont->widths->getWidth("space", &w)) {
      widths[32] = 0.001 * w;
    }
    for (code = 0; code < 256; ++code) {
      if (enc[code] && builtinFont->widths->getWidth(enc[code], &w)) {
	widths[code] = 0.001 * w;
      }
    }

  // couldn't find widths -- use defaults 
  } else {
    // this is technically an error -- the Widths entry is required
    // for all but the Base-14 fonts -- but certain PDF generators
    // apparently don't include widths for Arial and TimesNewRoman
    if (isFixedWidth()) {
      i = 0;
    } else if (isSerif()) {
      i = 8;
    } else {
      i = 4;
    }
    if (isBold()) {
      i += 2;
    }
    if (isItalic()) {
      i += 1;
    }
    builtinFont = builtinFontSubst[i];
    // this is a kludge for broken PDF files that encode char 32
    // as .notdef
    if (builtinFont->widths->getWidth("space", &w)) {
      widths[32] = 0.001 * w;
    }
    for (code = 0; code < 256; ++code) {
      if (enc[code] && builtinFont->widths->getWidth(enc[code], &w)) {
	widths[code] = 0.001 * w;
      }
    }
  }
  obj1.free();

  ok = gTrue;
}

Gfx8BitFont::~Gfx8BitFont() {
  int i;

  for (i = 0; i < 256; ++i) {
    if (encFree[i] && enc[i]) {
      gfree(enc[i]);
    }
  }
  ctu->decRefCnt();
  if (charProcs.isDict()) {
    charProcs.free();
  }
  if (resources.isDict()) {
    resources.free();
  }
}

// This function is in part a derived work of the Adobe Glyph Mapping
// Convention: http://www.adobe.com/devnet/opentype/archives/glyph.html
// Algorithmic comments are excerpted from that document to aid
// maintainability.
static int parseCharName(char *charName, Unicode *uBuf, int uLen,
			 GBool names, GBool ligatures,
			 GBool numeric, GBool hex, GBool variants)
{
  if (uLen <= 0) {
    error(-1, "Zero-length output buffer (recursion overflow?) in "
	  "parseCharName, component \"%s\"", charName);
    return 0;
  }
  // Step 1: drop all the characters from the glyph name starting with the
  // first occurrence of a period (U+002E FULL STOP), if any.
  if (variants) {
    char *var_part = strchr(charName, '.');
    if (var_part == charName) {
      return 0;	// .notdef or similar
    } else if (var_part != NULL) {
      // parse names of the form 7.oldstyle, P.swash, s.sc, etc.
      char *main_part = gstrndup(charName, var_part - charName);
      GBool namesRecurse = gTrue, variantsRecurse = gFalse;
      int n = parseCharName(main_part, uBuf, uLen, namesRecurse, ligatures,
			    numeric, hex, variantsRecurse);
      gfree(main_part);
      return n;
    }
  }
  // Step 2: split the remaining string into a sequence of components, using
  // underscore (U+005F LOW LINE) as the delimiter.
  if (ligatures && strchr(charName, '_')) {
    // parse names of the form A_a (e.g. f_i, T_h, l_quotesingle)
    char *lig_part, *lig_end, *lig_copy;
    int n = 0, m;
    lig_part = lig_copy = copyString(charName);
    do {
      if ((lig_end = strchr(lig_part, '_')))
	*lig_end = '\0';
      if (lig_part[0] != '\0') {
	GBool namesRecurse = gTrue, ligaturesRecurse = gFalse;
	if ((m = parseCharName(lig_part, uBuf + n, uLen - n, namesRecurse,
			       ligaturesRecurse, numeric, hex, variants)))
	  n += m;
	else
	  error(-1, "Could not parse ligature component \"%s\" of \"%s\" in "
		"parseCharName", lig_part, charName);
      }
      lig_part = lig_end + 1;
    } while (lig_end && n < uLen);
    gfree(lig_copy);
    return n;
  }
  // Step 3: map each component to a character string according to the
  // procedure below, and concatenate those strings; the result is the
  // character string to which the glyph name is mapped.
  // 3.1. if the font is Zapf Dingbats (PostScript FontName ZapfDingbats), and
  // the component is in the ZapfDingbats list, then map it to the
  // corresponding character in that list.
  // 3.2. otherwise, if the component is in the Adobe Glyph List, then map it
  // to the corresponding character in that list.
  if (names && (uBuf[0] = globalParams->mapNameToUnicode(charName))) {
    return 1;
  }
  if (numeric) {
    unsigned int n = strlen(charName);
    // 3.3. otherwise, if the component is of the form "uni" (U+0075 U+006E
    // U+0069) followed by a sequence of uppercase hexadecimal digits (0 .. 9,
    // A .. F, i.e. U+0030 .. U+0039, U+0041 .. U+0046), the length of that
    // sequence is a multiple of four, and each group of four digits represents
    // a number in the set {0x0000 .. 0xD7FF, 0xE000 .. 0xFFFF}, then interpret
    // each such number as a Unicode scalar value and map the component to the
    // string made of those scalar values. Note that the range and digit length
    // restrictions mean that the "uni" prefix can be used only with Unicode
    // values from the Basic Multilingual Plane (BMP).
    if (n >= 7 && (n % 4) == 3 && !strncmp(charName, "uni", 3)) {
      int i;
      unsigned int m;
      for (i = 0, m = 3; i < uLen && m < n; m += 4) {
	if (isxdigit(charName[m]) && isxdigit(charName[m + 1]) && 
	    isxdigit(charName[m + 2]) && isxdigit(charName[m + 3])) {
	  unsigned int u;
	  sscanf(charName + m, "%4x", &u);
	  if (u <= 0xD7FF || (0xE000 <= u && u <= 0xFFFF)) {
	    uBuf[i++] = u;
	  }
	}
      }
      return i;
    }
    // 3.4. otherwise, if the component is of the form "u" (U+0075) followed by
    // a sequence of four to six uppercase hexadecimal digits {0 .. 9, A .. F}
    // (U+0030 .. U+0039, U+0041 .. U+0046), and those digits represent a
    // number in {0x0000 .. 0xD7FF, 0xE000 .. 0x10FFFF}, then interpret this
    // number as a Unicode scalar value and map the component to the string
    // made of this scalar value.
    if (n >= 5 && n <= 7 && charName[0] == 'u' && isxdigit(charName[1]) &&
	isxdigit(charName[2]) && isxdigit(charName[3]) && isxdigit(charName[4])
	&& (n <= 5 || isxdigit(charName[5]))
	&& (n <= 6 || isxdigit(charName[6]))) {
      unsigned int u;
      sscanf(charName + 1, "%x", &u);
      if (u <= 0xD7FF || (0xE000 <= u && u <= 0x10FFFF)) {
	uBuf[0] = u;
	return 1;
      }
    }
    // Not in Adobe Glyph Mapping convention: look for names of the form 'Axx',
    // 'xx', 'Ann', 'ABnn', or 'nn', where 'A' and 'B' are any letters, 'xx' is
    // two hex digits, and 'nn' is 2-4 decimal digits
    if (hex && n == 3 && isalpha(charName[0]) &&
	isxdigit(charName[1]) && isxdigit(charName[2])) {
      sscanf(charName+1, "%x", (unsigned int *)uBuf);
      return 1;
    } else if (hex && n == 2 &&
	       isxdigit(charName[0]) && isxdigit(charName[1])) {
      sscanf(charName, "%x", (unsigned int *)uBuf);
      return 1;
    } else if (!hex && n >= 2 && n <= 4 &&
	       isdigit(charName[0]) && isdigit(charName[1])) {
      uBuf[0] = (Unicode)atoi(charName);
      return 1;
    } else if (n >= 3 && n <= 5 &&
	       isdigit(charName[1]) && isdigit(charName[2])) {
      uBuf[0] = (Unicode)atoi(charName+1);
      return 1;
    } else if (n >= 4 && n <= 6 &&
	       isdigit(charName[2]) && isdigit(charName[3])) {
      uBuf[0] = (Unicode)atoi(charName+2);
      return 1;
    }
  }
  // 3.5. otherwise, map the component to the empty string
  return 0;
}

int Gfx8BitFont::getNextChar(char *s, int len, CharCode *code,
			     Unicode **u, int *uLen,
			     double *dx, double *dy, double *ox, double *oy) {
  CharCode c;

  *code = c = (CharCode)(*s & 0xff);
  *uLen = ctu->mapToUnicode(c, u);
  *dx = widths[c];
  *dy = *ox = *oy = 0;
  return 1;
}

CharCodeToUnicode *Gfx8BitFont::getToUnicode() {
  ctu->incRefCnt();
  return ctu;
}

Gushort *Gfx8BitFont::getCodeToGIDMap(FoFiTrueType *ff) {
  Gushort *map;
  int cmapPlatform, cmapEncoding;
  int unicodeCmap, macRomanCmap, msSymbolCmap, cmap;
  GBool useMacRoman, useUnicode;
  char *charName;
  Unicode u;
  int code, i, n;

  map = (Gushort *)gmallocn(256, sizeof(Gushort));
  for (i = 0; i < 256; ++i) {
    map[i] = 0;
  }

  // To match up with the Adobe-defined behaviour, we choose a cmap
  // like this:
  // 1. If the PDF font has an encoding:
  //    1a. If the PDF font specified MacRomanEncoding and the
  //        TrueType font has a Macintosh Roman cmap, use it, and
  //        reverse map the char names through MacRomanEncoding to
  //        get char codes.
  //    1b. If the TrueType font has a Microsoft Unicode cmap or a
  //        non-Microsoft Unicode cmap, use it, and use the Unicode
  //        indexes, not the char codes.
  //    1c. If the PDF font is symbolic and the TrueType font has a
  //        Microsoft Symbol cmap, use it, and use char codes
  //        directly (possibly with an offset of 0xf000).
  //    1d. If the TrueType font has a Macintosh Roman cmap, use it,
  //        as in case 1a.
  // 2. If the PDF font does not have an encoding or the PDF font is
  //    symbolic:
  //    2a. If the TrueType font has a Macintosh Roman cmap, use it,
  //        and use char codes directly (possibly with an offset of
  //        0xf000).
  //    2b. If the TrueType font has a Microsoft Symbol cmap, use it,
  //        and use char codes directly (possible with an offset of
  //        0xf000).
  // 3. If none of these rules apply, use the first cmap and hope for
  //    the best (this shouldn't happen).
  unicodeCmap = macRomanCmap = msSymbolCmap = -1;
  for (i = 0; i < ff->getNumCmaps(); ++i) {
    cmapPlatform = ff->getCmapPlatform(i);
    cmapEncoding = ff->getCmapEncoding(i);
    if ((cmapPlatform == 3 && cmapEncoding == 1) ||
	cmapPlatform == 0) {
      unicodeCmap = i;
    } else if (cmapPlatform == 1 && cmapEncoding == 0) {
      macRomanCmap = i;
    } else if (cmapPlatform == 3 && cmapEncoding == 0) {
      msSymbolCmap = i;
    }
  }
  cmap = 0;
  useMacRoman = gFalse;
  useUnicode = gFalse;
  if (hasEncoding || type == fontType1) {
    if (usesMacRomanEnc && macRomanCmap >= 0) {
      cmap = macRomanCmap;
      useMacRoman = gTrue;
    } else if (unicodeCmap >= 0) {
      cmap = unicodeCmap;
      useUnicode = gTrue;
    } else if ((flags & fontSymbolic) && msSymbolCmap >= 0) {
      cmap = msSymbolCmap;
    } else if ((flags & fontSymbolic) && macRomanCmap >= 0) {
      cmap = macRomanCmap;
    } else if (macRomanCmap >= 0) {
      cmap = macRomanCmap;
      useMacRoman = gTrue;
    }
  } else {
    if (msSymbolCmap >= 0) {
      cmap = msSymbolCmap;
    } else if (macRomanCmap >= 0) {
      cmap = macRomanCmap;
    }
  }

  // reverse map the char names through MacRomanEncoding, then map the
  // char codes through the cmap
  if (useMacRoman) {
    for (i = 0; i < 256; ++i) {
      if ((charName = enc[i])) {
	if ((code = globalParams->getMacRomanCharCode(charName))) {
	  map[i] = ff->mapCodeToGID(cmap, code);
	}
      }
    }

  // map Unicode through the cmap
  } else if (useUnicode) {
    Unicode *uAux;
    for (i = 0; i < 256; ++i) {
      if (((charName = enc[i]) && (u = globalParams->mapNameToUnicode(charName))))
	map[i] = ff->mapCodeToGID(cmap, u);
      else
      {
	n = ctu->mapToUnicode((CharCode)i, &uAux);
	if (n > 0) map[i] = ff->mapCodeToGID(cmap, uAux[0]);
      }
    }

  // map the char codes through the cmap, possibly with an offset of
  // 0xf000
  } else {
    for (i = 0; i < 256; ++i) {
      if (!(map[i] = ff->mapCodeToGID(cmap, i))) {
	map[i] = ff->mapCodeToGID(cmap, 0xf000 + i);
      }
    }
  }

  // try the TrueType 'post' table to handle any unmapped characters
  for (i = 0; i < 256; ++i) {
    if (!map[i] && (charName = enc[i])) {
      map[i] = (Gushort)(int)ff->mapNameToGID(charName);
    }
  }

  return map;
}

Dict *Gfx8BitFont::getCharProcs() {
  return charProcs.isDict() ? charProcs.getDict() : (Dict *)NULL;
}

Object *Gfx8BitFont::getCharProc(int code, Object *proc) {
  if (enc[code] && charProcs.isDict()) {
    charProcs.dictLookup(enc[code], proc);
  } else {
    proc->initNull();
  }
  return proc;
}

Dict *Gfx8BitFont::getResources() {
  return resources.isDict() ? resources.getDict() : (Dict *)NULL;
}

//------------------------------------------------------------------------
// GfxCIDFont
//------------------------------------------------------------------------

static bool cmpWidthExcep(const GfxFontCIDWidthExcep &w1, const GfxFontCIDWidthExcep &w2) {
  return w1.first < w2.first;
}

static bool cmpWidthExcepV(const GfxFontCIDWidthExcepV &w1, const GfxFontCIDWidthExcepV &w2) {
  return w1.first < w2.first;
}

GfxCIDFont::GfxCIDFont(XRef *xref, char *tagA, Ref idA, GooString *nameA,
		       Dict *fontDict):
  GfxFont(tagA, idA, nameA)
{
  Dict *desFontDict;
  GooString *collection, *cMapName;
  Object desFontDictObj;
  Object obj1, obj2, obj3, obj4, obj5, obj6;
  CharCodeToUnicode *utu;
  CharCode c;
  Unicode *uBuf;
  int c1, c2;
  int excepsSize, i, j, k, n;

  refCnt = 1;
  ascent = 0.95;
  descent = -0.35;
  fontBBox[0] = fontBBox[1] = fontBBox[2] = fontBBox[3] = 0;
  cMap = NULL;
  cMapName = NULL;
  ctu = NULL;
  widths.defWidth = 1.0;
  widths.defHeight = -1.0;
  widths.defVY = 0.880;
  widths.exceps = NULL;
  widths.nExceps = 0;
  widths.excepsV = NULL;
  widths.nExcepsV = 0;
  cidToGID = NULL;
  cidToGIDLen = 0;

  // get the descendant font
  if (!fontDict->lookup("DescendantFonts", &obj1)->isArray()) {
    error(-1, "Missing DescendantFonts entry in Type 0 font");
    obj1.free();
    goto err1;
  }
  if (!obj1.arrayGet(0, &desFontDictObj)->isDict()) {
    error(-1, "Bad descendant font in Type 0 font");
    goto err3;
  }
  obj1.free();
  desFontDict = desFontDictObj.getDict();

  // font type
  if (!desFontDict->lookup("Subtype", &obj1)) {
    error(-1, "Missing Subtype entry in Type 0 descendant font");
    goto err3;
  }
  if (obj1.isName("CIDFontType0")) {
    type = fontCIDType0;
  } else if (obj1.isName("CIDFontType2")) {
    type = fontCIDType2;
  } else {
    error(-1, "Unknown Type 0 descendant font type '%s'",
	  obj1.isName() ? obj1.getName() : "???");
    goto err3;
  }
  obj1.free();

  // get info from font descriptor
  readFontDescriptor(xref, desFontDict);

  // look for an external font file
  findExtFontFile();

  //----- encoding info -----

  // char collection
  if (!desFontDict->lookup("CIDSystemInfo", &obj1)->isDict()) {
    error(-1, "Missing CIDSystemInfo dictionary in Type 0 descendant font");
    goto err3;
  }
  obj1.dictLookup("Registry", &obj2);
  obj1.dictLookup("Ordering", &obj3);
  if (!obj2.isString() || !obj3.isString()) {
    error(-1, "Invalid CIDSystemInfo dictionary in Type 0 descendant font");
    goto err4;
  }
  collection = obj2.getString()->copy()->append('-')->append(obj3.getString());
  obj3.free();
  obj2.free();
  obj1.free();

  // look for a ToUnicode CMap
  if (!(ctu = readToUnicodeCMap(fontDict, 16, NULL))) {

    // the "Adobe-Identity" and "Adobe-UCS" collections don't have
    // cidToUnicode files
    if (collection->cmp("Adobe-Identity") &&
	collection->cmp("Adobe-UCS")) {

      // look for a user-supplied .cidToUnicode file
      if (!(ctu = globalParams->getCIDToUnicode(collection))) {
	// I'm not completely sure that this is the best thing to do
	// but it seems to produce better results when the .cidToUnicode
	// files from the poppler-data package are missing. At least
	// we know that assuming the Identity mapping is definitely wrong.
	//   -- jrmuizel
	static const char * knownCollections [] = {
	  "Adobe-CNS1",
	  "Adobe-GB1",
	  "Adobe-Japan1",
	  "Adobe-Japan2",
	  "Adobe-Korea1",
	};
	for (size_t i = 0; i < sizeof(knownCollections)/sizeof(knownCollections[0]); i++) {
	  if (collection->cmp(knownCollections[i]) == 0) {
	    error(-1, "Missing language pack for '%s' mapping", collection->getCString());
	    delete collection;
	    goto err2;
	  }
	}
	error(-1, "Unknown character collection '%s'",
	      collection->getCString());
	// fall-through, assuming the Identity mapping -- this appears
	// to match Adobe's behavior
      }
    }
  }

  // look for a Unicode-to-Unicode mapping
  if (name && (utu = globalParams->getUnicodeToUnicode(name))) {
    if (ctu) {
      for (c = 0; c < ctu->getLength(); ++c) {
	n = ctu->mapToUnicode(c, &uBuf);
	if (n >= 1) {
	  n = utu->mapToUnicode((CharCode)uBuf[0], &uBuf);
	  if (n >= 1) {
	    ctu->setMapping(c, uBuf, n);
	  }
	}
      }
      utu->decRefCnt();
    } else {
      ctu = utu;
    }
  }

  // encoding (i.e., CMap)
  //~ also need to deal with the UseCMap entry in the stream dict
  if (!fontDict->lookup("Encoding", &obj1)->isName()) {
    GBool success = gFalse;
    if (obj1.isStream()) {
      Object objName;
      Stream *s = obj1.getStream();
      s->getDict()->lookup("CMapName", &objName);
      if (objName.isName())
      {
        cMapName = new GooString(objName.getName());
        cMap = globalParams->getCMap(collection, cMapName, s);
        success = gTrue;
      }
      objName.free();
    }
    
    if (!success) {
      error(-1, "Missing or invalid Encoding entry in Type 0 font");
      delete collection;
      goto err3;
    }
  } else {
    cMapName = new GooString(obj1.getName());
    cMap = globalParams->getCMap(collection, cMapName);
  }
  if (!cMap) {
      error(-1, "Unknown CMap '%s' for character collection '%s'",
	    cMapName->getCString(), collection->getCString());
      delete collection;
      delete cMapName;
      goto err2;
    }
  delete collection;
  delete cMapName;
  obj1.free();

  // CIDToGIDMap (for embedded TrueType fonts)
  if (type == fontCIDType2 || type == fontCIDType2OT) {
    desFontDict->lookup("CIDToGIDMap", &obj1);
    if (obj1.isStream()) {
      cidToGIDLen = 0;
      i = 64;
      cidToGID = (Gushort *)gmallocn(i, sizeof(Gushort));
      obj1.streamReset();
      while ((c1 = obj1.streamGetChar()) != EOF &&
	     (c2 = obj1.streamGetChar()) != EOF) {
	if (cidToGIDLen == i) {
	  i *= 2;
	  cidToGID = (Gushort *)greallocn(cidToGID, i, sizeof(Gushort));
	}
	cidToGID[cidToGIDLen++] = (Gushort)((c1 << 8) + c2);
      }
    } else if (!obj1.isName("Identity") && !obj1.isNull()) {
      error(-1, "Invalid CIDToGIDMap entry in CID font");
    }
    obj1.free();
  }

  //----- character metrics -----

  // default char width
  if (desFontDict->lookup("DW", &obj1)->isInt()) {
    widths.defWidth = obj1.getInt() * 0.001;
  }
  obj1.free();

  // char width exceptions
  if (desFontDict->lookup("W", &obj1)->isArray()) {
    excepsSize = 0;
    i = 0;
    while (i + 1 < obj1.arrayGetLength()) {
      obj1.arrayGet(i, &obj2);
      obj1.arrayGet(i + 1, &obj3);
      if (obj2.isInt() && obj3.isInt() && i + 2 < obj1.arrayGetLength()) {
	if (obj1.arrayGet(i + 2, &obj4)->isNum()) {
	  if (widths.nExceps == excepsSize) {
	    excepsSize += 16;
	    widths.exceps = (GfxFontCIDWidthExcep *)
	      greallocn(widths.exceps,
			excepsSize, sizeof(GfxFontCIDWidthExcep));
	  }
	  widths.exceps[widths.nExceps].first = obj2.getInt();
	  widths.exceps[widths.nExceps].last = obj3.getInt();
	  widths.exceps[widths.nExceps].width = obj4.getNum() * 0.001;
	  ++widths.nExceps;
	} else {
	  error(-1, "Bad widths array in Type 0 font");
	}
	obj4.free();
	i += 3;
      } else if (obj2.isInt() && obj3.isArray()) {
	if (widths.nExceps + obj3.arrayGetLength() > excepsSize) {
	  excepsSize = (widths.nExceps + obj3.arrayGetLength() + 15) & ~15;
	  widths.exceps = (GfxFontCIDWidthExcep *)
	    greallocn(widths.exceps,
		      excepsSize, sizeof(GfxFontCIDWidthExcep));
	}
	j = obj2.getInt();
	for (k = 0; k < obj3.arrayGetLength(); ++k) {
	  if (obj3.arrayGet(k, &obj4)->isNum()) {
	    widths.exceps[widths.nExceps].first = j;
	    widths.exceps[widths.nExceps].last = j;
	    widths.exceps[widths.nExceps].width = obj4.getNum() * 0.001;
	    ++j;
	    ++widths.nExceps;
	  } else {
	    error(-1, "Bad widths array in Type 0 font");
	  }
	  obj4.free();
	}
	i += 2;
      } else {
	error(-1, "Bad widths array in Type 0 font");
	++i;
      }
      obj3.free();
      obj2.free();
    }
    std::sort(widths.exceps, widths.exceps + widths.nExceps, &cmpWidthExcep);
  }
  obj1.free();

  // default metrics for vertical font
  if (desFontDict->lookup("DW2", &obj1)->isArray() &&
      obj1.arrayGetLength() == 2) {
    if (obj1.arrayGet(0, &obj2)->isNum()) {
      widths.defVY = obj2.getNum() * 0.001;
    }
    obj2.free();
    if (obj1.arrayGet(1, &obj2)->isNum()) {
      widths.defHeight = obj2.getNum() * 0.001;
    }
    obj2.free();
  }
  obj1.free();

  // char metric exceptions for vertical font
  if (desFontDict->lookup("W2", &obj1)->isArray()) {
    excepsSize = 0;
    i = 0;
    while (i + 1 < obj1.arrayGetLength()) {
      obj1.arrayGet(i, &obj2);
      obj1.arrayGet(i+ 1, &obj3);
      if (obj2.isInt() && obj3.isInt() && i + 4 < obj1.arrayGetLength()) {
	if (obj1.arrayGet(i + 2, &obj4)->isNum() &&
	    obj1.arrayGet(i + 3, &obj5)->isNum() &&
	    obj1.arrayGet(i + 4, &obj6)->isNum()) {
	  if (widths.nExcepsV == excepsSize) {
	    excepsSize += 16;
	    widths.excepsV = (GfxFontCIDWidthExcepV *)
	      greallocn(widths.excepsV,
			excepsSize, sizeof(GfxFontCIDWidthExcepV));
	  }
	  widths.excepsV[widths.nExcepsV].first = obj2.getInt();
	  widths.excepsV[widths.nExcepsV].last = obj3.getInt();
	  widths.excepsV[widths.nExcepsV].height = obj4.getNum() * 0.001;
	  widths.excepsV[widths.nExcepsV].vx = obj5.getNum() * 0.001;
	  widths.excepsV[widths.nExcepsV].vy = obj6.getNum() * 0.001;
	  ++widths.nExcepsV;
	} else {
	  error(-1, "Bad widths (W2) array in Type 0 font");
	}
	obj6.free();
	obj5.free();
	obj4.free();
	i += 5;
      } else if (obj2.isInt() && obj3.isArray()) {
	if (widths.nExcepsV + obj3.arrayGetLength() / 3 > excepsSize) {
	  excepsSize =
	    (widths.nExcepsV + obj3.arrayGetLength() / 3 + 15) & ~15;
	  widths.excepsV = (GfxFontCIDWidthExcepV *)
	    greallocn(widths.excepsV,
		      excepsSize, sizeof(GfxFontCIDWidthExcepV));
	}
	j = obj2.getInt();
	for (k = 0; k < obj3.arrayGetLength(); k += 3) {
	  if (obj3.arrayGet(k, &obj4)->isNum() &&
	      obj3.arrayGet(k+1, &obj5)->isNum() &&
	      obj3.arrayGet(k+2, &obj6)->isNum()) {
	    widths.excepsV[widths.nExcepsV].first = j;
	    widths.excepsV[widths.nExcepsV].last = j;
	    widths.excepsV[widths.nExcepsV].height = obj4.getNum() * 0.001;
	    widths.excepsV[widths.nExcepsV].vx = obj5.getNum() * 0.001;
	    widths.excepsV[widths.nExcepsV].vy = obj6.getNum() * 0.001;
	    ++j;
	    ++widths.nExcepsV;
	  } else {
	    error(-1, "Bad widths (W2) array in Type 0 font");
	  }
	  obj6.free();
	  obj5.free();
	  obj4.free();
	}
	i += 2;
      } else {
	error(-1, "Bad widths (W2) array in Type 0 font");
	++i;
      }
      obj3.free();
      obj2.free();
    }
    std::sort(widths.excepsV, widths.excepsV + widths.nExcepsV, &cmpWidthExcepV);
  }
  obj1.free();

  desFontDictObj.free();
  ok = gTrue;
  return;

 err4:
  obj3.free();
  obj2.free();
 err3:
  obj1.free();
 err2:
  desFontDictObj.free();
 err1:;
}

GfxCIDFont::~GfxCIDFont() {
  if (cMap) {
    cMap->decRefCnt();
  }
  if (ctu) {
    ctu->decRefCnt();
  }
  gfree(widths.exceps);
  gfree(widths.excepsV);
  if (cidToGID) {
    gfree(cidToGID);
  }
}

int GfxCIDFont::getNextChar(char *s, int len, CharCode *code,
			    Unicode **u, int *uLen,
			    double *dx, double *dy, double *ox, double *oy) {
  CID cid;
  double w, h, vx, vy;
  int n, a, b, m;

  if (!cMap) {
    *code = 0;
    *uLen = 0;
    *dx = *dy = 0;
    return 1;
  }

  *code = (CharCode)(cid = cMap->getCID(s, len, &n));
  if (ctu) {
    if (hasToUnicode) {
      int i = 0, c = 0;
      while (i < n) {
	c = (c << 8 ) + (s[i] & 0xff);
	++i;
      }
      *uLen = ctu->mapToUnicode(c, u);
    } else {
      *uLen = ctu->mapToUnicode(cid, u);
    }
  } else {
    *uLen = 0;
  }

  // horizontal
  if (cMap->getWMode() == 0) {
    w = widths.defWidth;
    h = vx = vy = 0;
    if (widths.nExceps > 0 && cid >= widths.exceps[0].first) {
      a = 0;
      b = widths.nExceps;
      // invariant: widths.exceps[a].first <= cid < widths.exceps[b].first
      while (b - a > 1) {
	m = (a + b) / 2;
	if (widths.exceps[m].first <= cid) {
	  a = m;
	} else {
	  b = m;
	}
      }
      if (cid <= widths.exceps[a].last) {
	w = widths.exceps[a].width;
      }
    }

  // vertical
  } else {
    w = 0;
    h = widths.defHeight;
    vx = widths.defWidth / 2;
    vy = widths.defVY;
    if (widths.nExcepsV > 0 && cid >= widths.excepsV[0].first) {
      a = 0;
      b = widths.nExcepsV;
      // invariant: widths.excepsV[a].first <= cid < widths.excepsV[b].first
      while (b - a > 1) {
	m = (a + b) / 2;
	if (widths.excepsV[m].last <= cid) {
	  a = m;
	} else {
	  b = m;
	}
      }
      if (cid <= widths.excepsV[a].last) {
	h = widths.excepsV[a].height;
	vx = widths.excepsV[a].vx;
	vy = widths.excepsV[a].vy;
      }
    }
  }

  *dx = w;
  *dy = h;
  *ox = vx;
  *oy = vy;

  return n;
}

int GfxCIDFont::getWMode() {
  return cMap ? cMap->getWMode() : 0;
}

CharCodeToUnicode *GfxCIDFont::getToUnicode() {
  if (ctu) {
    ctu->incRefCnt();
  }
  return ctu;
}

GooString *GfxCIDFont::getCollection() {
  return cMap ? cMap->getCollection() : (GooString *)NULL;
}

Gushort GfxCIDFont::mapCodeToGID(FoFiTrueType *ff, int cmapi,
  Unicode unicode, GBool wmode) {
  Gushort gid = ff->mapCodeToGID(cmapi,unicode);
  if (wmode) {
    Gushort vgid = ff->mapToVertGID(gid);
    if (vgid != 0) gid = vgid;
  }
  return gid;
}

Gushort *GfxCIDFont::getCodeToGIDMap(FoFiTrueType *ff, int *mapsizep) {
#define N_UCS_CANDIDATES 2
  /* space characters */
  static const unsigned long spaces[] = { 
    0x2000,0x2001,0x2002,0x2003,0x2004,0x2005,0x2006,0x2007,
    0x2008,0x2009,0x200A,0x00A0,0x200B,0x2060,0x3000,0xFEFF,
    0
  };
  static char *adobe_cns1_cmaps[] = {
    "UniCNS-UTF32-V",
    "UniCNS-UCS2-V",
    "UniCNS-UTF32-H",
    "UniCNS-UCS2-H",
    0
  };
  static char *adobe_gb1_cmaps[] = {
    "UniGB-UTF32-V",
    "UniGB-UCS2-V",
    "UniGB-UTF32-H",
    "UniGB-UCS2-H",
    0
  };
  static char *adobe_japan1_cmaps[] = {
    "UniJIS-UTF32-V",
    "UniJIS-UCS2-V",
    "UniJIS-UTF32-H",
    "UniJIS-UCS2-H",
    0
  };
  static char *adobe_japan2_cmaps[] = {
    "UniHojo-UTF32-V",
    "UniHojo-UCS2-V",
    "UniHojo-UTF32-H",
    "UniHojo-UCS2-H",
    0
  };
  static char *adobe_korea1_cmaps[] = {
    "UniKS-UTF32-V",
    "UniKS-UCS2-V",
    "UniKS-UTF32-H",
    "UniKS-UCS2-H",
    0
  };
  static struct CMapListEntry {
    char *collection;
    char *scriptTag;
    char *toUnicodeMap;
    char **CMaps;
  } CMapList[] = {
    {
      "Adobe-CNS1",
      "kana",
      "Adobe-CNS1-UCS2",
      adobe_cns1_cmaps,
    },
    {
      "Adobe-GB1",
      "kana",
      "Adobe-GB1-UCS2",
      adobe_gb1_cmaps,
    },
    {
      "Adobe-Japan1",
      "kana",
      "Adobe-Japan1-UCS2",
      adobe_japan1_cmaps,
    },
    {
      "Adobe-Japan2",
      "kana",
      "Adobe-Japan2-UCS2",
      adobe_japan2_cmaps,
    },
    {
      "Adobe-Korea1",
      "kana",
      "Adobe-Korea1-UCS2",
      adobe_korea1_cmaps,
    },
    {0, 0, 0, 0}
  };
  Unicode *humap = 0;
  Unicode *vumap = 0;
  Unicode *tumap = 0;
  Gushort *codeToGID = 0;
  unsigned long n;
  int i;
  unsigned long code;
  int wmode;
  char **cmapName;
  CMap *cMap;
  CMapListEntry *lp;
  int cmap;
  int cmapPlatform, cmapEncoding;
  Ref embID;

  *mapsizep = 0;
  if (!ctu) return NULL;
  if (getCollection()->cmp("Adobe-Identity") == 0) return NULL;
  if (getEmbeddedFontID(&embID)) {
   /* if this font is embedded font, 
    * CIDToGIDMap should be embedded in PDF file
    * and already set. So return it.
    */
    *mapsizep = getCIDToGIDLen();
    return getCIDToGID();
  }

  /* we use only unicode cmap */
  cmap = -1;
  for (i = 0; i < ff->getNumCmaps(); ++i) {
    cmapPlatform = ff->getCmapPlatform(i);
    cmapEncoding = ff->getCmapEncoding(i);
    if (cmapPlatform == 3 && cmapEncoding == 10) {
	/* UCS-4 */
	cmap = i;
	/* use UCS-4 cmap */
	break;
    } else if (cmapPlatform == 3 && cmapEncoding == 1) {
	/* Unicode */
	cmap = i;
    } else if (cmapPlatform == 0 && cmap < 0) {
	cmap = i;
    }
  }
  if (cmap < 0)
    return NULL;

  wmode = getWMode();
  for (lp = CMapList;lp->collection != 0;lp++) {
    if (strcmp(lp->collection,getCollection()->getCString()) == 0) {
      break;
    }
  }
  n = 65536;
  tumap = new Unicode[n];
  humap = new Unicode[n*N_UCS_CANDIDATES];
  memset(humap,0,sizeof(Unicode)*n*N_UCS_CANDIDATES);
  if (lp->collection != 0) {
    CharCodeToUnicode *tctu;
    GooString tname(lp->toUnicodeMap);

    if ((tctu = CharCodeToUnicode::parseCMapFromFile(&tname,16)) != 0) {
      CharCode cid;
      for (cid = 0;cid < n ;cid++) {
	int len;
	Unicode *ucodes;

	len = tctu->mapToUnicode(cid,&ucodes);
	if (len == 1) {
	  tumap[cid] = ucodes[0];
	} else {
	  /* if not single character, ignore it */
	  tumap[cid] = 0;
	}
      }
      delete tctu;
    }
    vumap = new Unicode[n];
    memset(vumap,0,sizeof(Unicode)*n);
    for (cmapName = lp->CMaps;*cmapName != 0;cmapName++) {
      GooString cname(*cmapName);

      if ((cMap = globalParams->getCMap(getCollection(),&cname))
	   != 0) {
	    if (cMap->getWMode()) {
		cMap->setReverseMap(vumap,n,1);
	    } else {
		cMap->setReverseMap(humap,n,N_UCS_CANDIDATES);
	    }
	cMap->decRefCnt();
      }
    }
    ff->setupGSUB(lp->scriptTag);
  } else {
    error(-1,"Unknown character collection %s\n",
      getCollection()->getCString());
    if ((ctu = getToUnicode()) != 0) {
      CharCode cid;
      for (cid = 0;cid < n ;cid++) {
	Unicode *ucode;

	if (ctu->mapToUnicode(cid, &ucode))
	  humap[cid*N_UCS_CANDIDATES] = ucode[0];
	else
	  humap[cid*N_UCS_CANDIDATES] = 0;
	for (i = 1;i < N_UCS_CANDIDATES;i++) {
	    humap[cid*N_UCS_CANDIDATES+i] = 0;
	}
      }
      ctu->decRefCnt();
    }
  }
  // map CID -> Unicode -> GID
  codeToGID = (Gushort *)gmallocn(n, sizeof(Gushort));
  for (code = 0; code < n; ++code) {
    Unicode unicode;
    unsigned long gid;

    unicode = 0;
    gid = 0;
    if (humap != 0) {
      for (i = 0;i < N_UCS_CANDIDATES
	&& gid == 0 && (unicode = humap[code*N_UCS_CANDIDATES+i]) != 0;i++) {
	gid = mapCodeToGID(ff,cmap,unicode,gFalse);
      }
    }
    if (gid == 0 && vumap != 0) {
      unicode = vumap[code];
      if (unicode != 0) {
	gid = mapCodeToGID(ff,cmap,unicode,gTrue);
	if (gid == 0 && tumap != 0) {
	  if ((unicode = tumap[code]) != 0) {
	    gid = mapCodeToGID(ff,cmap,unicode,gTrue);
	  }
	}
      }
    }
    if (gid == 0 && tumap != 0) {
      if ((unicode = tumap[code]) != 0) {
	gid = mapCodeToGID(ff,cmap,unicode,gFalse);
      }
    }
    if (gid == 0) {
      /* special handling space characters */
      const unsigned long *p;

      if (humap != 0) unicode = humap[code];
      if (unicode != 0) {
	/* check if code is space character , so map code to 0x0020 */
	for (p = spaces;*p != 0;p++) {
	  if (*p == unicode) {
	    unicode = 0x20;
	    gid = mapCodeToGID(ff,cmap,unicode,wmode);
	    break;
	  }
	}
      }
    }
    codeToGID[code] = gid;
  }
  *mapsizep = n;
  if (humap != 0) delete[] humap;
  if (tumap != 0) delete[] tumap;
  if (vumap != 0) delete[] vumap;
  return codeToGID;
}

double GfxCIDFont::getWidth (char* s, int len) {
  int nUsed;
  double w;
  int a, b, m;

  CID cid = cMap->getCID(s, len, &nUsed);

  w = widths.defWidth;
  if (widths.nExceps > 0 && cid >= widths.exceps[0].first) {
    a = 0;
    b = widths.nExceps;
    // invariant: widths.exceps[a].first <= cid < widths.exceps[b].first
    while (b - a > 1) {
      m = (a + b) / 2;
      if (widths.exceps[m].first <= cid) {
        a = m;
      } else {
        b = m;
      }
    }
    if (cid <= widths.exceps[a].last) {
      w = widths.exceps[a].width;
    }
  }
  return w;
}

//------------------------------------------------------------------------
// GfxFontDict
//------------------------------------------------------------------------

GfxFontDict::GfxFontDict(XRef *xref, Ref *fontDictRef, Dict *fontDict) {
  int i;
  Object obj1, obj2;
  Ref r;

  numFonts = fontDict->getLength();
  fonts = (GfxFont **)gmallocn(numFonts, sizeof(GfxFont *));
  for (i = 0; i < numFonts; ++i) {
    fontDict->getValNF(i, &obj1);
    obj1.fetch(xref, &obj2);
    if (obj2.isDict()) {
      if (obj1.isRef()) {
	r = obj1.getRef();
      } else {
	// no indirect reference for this font, so invent a unique one
	// (legal generation numbers are five digits, so any 6-digit
	// number would be safe)
	r.num = i;
	if (fontDictRef) {
	  r.gen = 100000 + fontDictRef->num;
	} else {
	  r.gen = 999999;
	}
      }
      fonts[i] = GfxFont::makeFont(xref, fontDict->getKey(i),
				   r, obj2.getDict());
      if (fonts[i] && !fonts[i]->isOk()) {
	// XXX: it may be meaningful to distinguish between
	// NULL and !isOk() so that when we do lookups
	// we can tell the difference between a missing font
	// and a font that is just !isOk()
	fonts[i]->decRefCnt();
	fonts[i] = NULL;
      }
    } else {
      error(-1, "font resource is not a dictionary");
      fonts[i] = NULL;
    }
    obj1.free();
    obj2.free();
  }
}

GfxFontDict::~GfxFontDict() {
  int i;

  for (i = 0; i < numFonts; ++i) {
    if (fonts[i]) {
      fonts[i]->decRefCnt();
    }
  }
  gfree(fonts);
}

GfxFont *GfxFontDict::lookup(char *tag) {
  int i;

  for (i = 0; i < numFonts; ++i) {
    if (fonts[i] && fonts[i]->matches(tag)) {
      return fonts[i];
    }
  }
  return NULL;
}
