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
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2006, 2008-2010, 2012 Albert Astals Cid <aacid@kde.org>
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
// Copyright (C) 2011, 2012 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Yi Yang <ahyangyi@gmail.com>
// Copyright (C) 2012 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
// Copyright (C) 2012 Thomas Freitag <Thomas.Freitag@alfa.de>
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
#include <math.h>
#include <limits.h>
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
#include "UnicodeTypeTable.h"
#include <fofi/FoFiIdentifier.h>
#include <fofi/FoFiType1.h>
#include <fofi/FoFiType1C.h>
#include <fofi/FoFiTrueType.h>
#include "GfxFont.h"

//------------------------------------------------------------------------

struct Base14FontMapEntry {
  const char *altName;
  const char *base14Name;
};

static const Base14FontMapEntry base14FontMap[] = {
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
  { "Courier",                      "Courier" },
  { "Courier,Bold",                 "Courier-Bold" },
  { "Courier,BoldItalic",           "Courier-BoldOblique" },
  { "Courier,Italic",               "Courier-Oblique" },
  { "Courier-Bold",                 "Courier-Bold" },
  { "Courier-BoldOblique",          "Courier-BoldOblique" },
  { "Courier-Oblique",              "Courier-Oblique" },
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
  { "Helvetica",                    "Helvetica" },
  { "Helvetica,Bold",               "Helvetica-Bold" },
  { "Helvetica,BoldItalic",         "Helvetica-BoldOblique" },
  { "Helvetica,Italic",             "Helvetica-Oblique" },
  { "Helvetica-Bold",               "Helvetica-Bold" },
  { "Helvetica-BoldItalic",         "Helvetica-BoldOblique" },
  { "Helvetica-BoldOblique",        "Helvetica-BoldOblique" },
  { "Helvetica-Italic",             "Helvetica-Oblique" },
  { "Helvetica-Oblique",            "Helvetica-Oblique" },
  { "Symbol",                       "Symbol" },
  { "Symbol,Bold",                  "Symbol" },
  { "Symbol,BoldItalic",            "Symbol" },
  { "Symbol,Italic",                "Symbol" },
  { "Times-Bold",                   "Times-Bold" },
  { "Times-BoldItalic",             "Times-BoldItalic" },
  { "Times-Italic",                 "Times-Italic" },
  { "Times-Roman",                  "Times-Roman" },
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
  { "TimesNewRomanPSMT,Italic",     "Times-Italic" },
  { "ZapfDingbats",                 "ZapfDingbats" }
};

//------------------------------------------------------------------------

// index: {fixed:0, sans-serif:4, serif:8} + bold*2 + italic
// NB: must be in same order as psSubstFonts in PSOutputDev.cc
static const char *base14SubstFonts[14] = {
  "Courier",
  "Courier-Oblique",
  "Courier-Bold",
  "Courier-BoldOblique",
  "Helvetica",
  "Helvetica-Oblique",
  "Helvetica-Bold",
  "Helvetica-BoldOblique",
  "Times-Roman",
  "Times-Italic",
  "Times-Bold",
  "Times-BoldItalic",
  // the last two are never used for substitution
  "Symbol",
  "ZapfDingbats"
};

//------------------------------------------------------------------------

static int parseCharName(char *charName, Unicode *uBuf, int uLen,
			 GBool names, GBool ligatures, 
			 GBool numeric, GBool hex, GBool variants);

//------------------------------------------------------------------------

static int readFromStream(void *data) {
  return ((Stream *)data)->getChar();
}

//------------------------------------------------------------------------
// GfxFontLoc
//------------------------------------------------------------------------

GfxFontLoc::GfxFontLoc() {
  path = NULL;
  fontNum = 0;
  encoding = NULL;
  substIdx = -1;
}

GfxFontLoc::~GfxFontLoc() {
  if (path) {
    delete path;
  }
  if (encoding) {
    delete encoding;
  }
}

//------------------------------------------------------------------------
// GfxFont
//------------------------------------------------------------------------

GfxFont *GfxFont::makeFont(XRef *xref, const char *tagA, Ref idA, Dict *fontDict) {
  GooString *nameA;
  Ref embFontIDA;
  GfxFontType typeA;
  GfxFont *font;
  Object obj1;

  // get base font name
  nameA = NULL;
  fontDict->lookup("BaseFont", &obj1);
  if (obj1.isName()) {
    nameA = new GooString(obj1.getName());
  }
  obj1.free();

  // get embedded font ID and font type
  typeA = getFontType(xref, fontDict, &embFontIDA);

  // create the font object
  font = NULL;
  if (typeA < fontCIDType0) {
    font = new Gfx8BitFont(xref, tagA, idA, nameA, typeA, embFontIDA,
			   fontDict);
  } else {
    font = new GfxCIDFont(xref, tagA, idA, nameA, typeA, embFontIDA,
			  fontDict);
  }

  return font;
}

GfxFont::GfxFont(const char *tagA, Ref idA, GooString *nameA,
		 GfxFontType typeA, Ref embFontIDA) {
  ok = gFalse;
  tag = new GooString(tagA);
  id = idA;
  name = nameA;
  type = typeA;
  embFontID = embFontIDA;
  embFontName = NULL;
  family = NULL;
  stretch = StretchNotDefined;
  weight = WeightNotDefined;
  refCnt = 1;
  encodingName = new GooString("");
  hasToUnicode = gFalse;
}

GfxFont::~GfxFont() {
  delete tag;
  delete family;
  if (name) {
    delete name;
  }
  if (embFontName) {
    delete embFontName;
  }
  if (encodingName) {
    delete encodingName;
  }
}

void GfxFont::incRefCnt() {
  refCnt++;
}

void GfxFont::decRefCnt() {
  if (--refCnt == 0)
    delete this;
}

// This function extracts three pieces of information:
// 1. the "expected" font type, i.e., the font type implied by
//    Font.Subtype, DescendantFont.Subtype, and
//    FontDescriptor.FontFile3.Subtype
// 2. the embedded font object ID
// 3. the actual font type - determined by examining the embedded font
//    if there is one, otherwise equal to the expected font type
// If the expected and actual font types don't match, a warning
// message is printed.  The expected font type is not used for
// anything else.
GfxFontType GfxFont::getFontType(XRef *xref, Dict *fontDict, Ref *embID) {
  GfxFontType t, expectedType;
  FoFiIdentifierType fft;
  Dict *fontDict2;
  Object subtype, fontDesc, obj1, obj2, obj3, obj4;
  GBool isType0, err;

  t = fontUnknownType;
  embID->num = embID->gen = -1;
  err = gFalse;

  fontDict->lookup("Subtype", &subtype);
  expectedType = fontUnknownType;
  isType0 = gFalse;
  if (subtype.isName("Type1") || subtype.isName("MMType1")) {
    expectedType = fontType1;
  } else if (subtype.isName("Type1C")) {
    expectedType = fontType1C;
  } else if (subtype.isName("Type3")) {
    expectedType = fontType3;
  } else if (subtype.isName("TrueType")) {
    expectedType = fontTrueType;
  } else if (subtype.isName("Type0")) {
    isType0 = gTrue;
  } else {
    error(errSyntaxWarning, -1, "Unknown font type: '{0:s}'",
	  subtype.isName() ? subtype.getName() : "???");
  }
  subtype.free();

  fontDict2 = fontDict;
  if (fontDict->lookup("DescendantFonts", &obj1)->isArray()) {
    if (obj1.arrayGetLength() == 0) {
      error(errSyntaxWarning, -1, "Empty DescendantFonts array in font");
      obj2.initNull();
    } else if (obj1.arrayGet(0, &obj2)->isDict()) {
      if (!isType0) {
	error(errSyntaxWarning, -1, "Non-CID font with DescendantFonts array");
      }
      fontDict2 = obj2.getDict();
      fontDict2->lookup("Subtype", &subtype);
      if (subtype.isName("CIDFontType0")) {
	if (isType0) {
	  expectedType = fontCIDType0;
	}
      } else if (subtype.isName("CIDFontType2")) {
	if (isType0) {
	  expectedType = fontCIDType2;
	}
      }
      subtype.free();
    }
  } else {
    obj2.initNull();
  }

  if (fontDict2->lookup("FontDescriptor", &fontDesc)->isDict()) {
    if (fontDesc.dictLookupNF("FontFile", &obj3)->isRef()) {
      *embID = obj3.getRef();
      if (expectedType != fontType1) {
	err = gTrue;
      }
    }
    obj3.free();
    if (embID->num == -1 &&
	fontDesc.dictLookupNF("FontFile2", &obj3)->isRef()) {
      *embID = obj3.getRef();
      if (isType0) {
	expectedType = fontCIDType2;
      } else if (expectedType != fontTrueType) {
	err = gTrue;
      }
    }
    obj3.free();
    if (embID->num == -1 &&
	fontDesc.dictLookupNF("FontFile3", &obj3)->isRef()) {
      *embID = obj3.getRef();
      if (obj3.fetch(xref, &obj4)->isStream()) {
	obj4.streamGetDict()->lookup("Subtype", &subtype);
	if (subtype.isName("Type1")) {
	  if (expectedType != fontType1) {
	    err = gTrue;
	    expectedType = isType0 ? fontCIDType0 : fontType1;
	  }
	} else if (subtype.isName("Type1C")) {
	  if (expectedType == fontType1) {
	    expectedType = fontType1C;
	  } else if (expectedType != fontType1C) {
	    err = gTrue;
	    expectedType = isType0 ? fontCIDType0C : fontType1C;
	  }
	} else if (subtype.isName("TrueType")) {
	  if (expectedType != fontTrueType) {
	    err = gTrue;
	    expectedType = isType0 ? fontCIDType2 : fontTrueType;
	  }
	} else if (subtype.isName("CIDFontType0C")) {
	  if (expectedType == fontCIDType0) {
	    expectedType = fontCIDType0C;
	  } else {
	    err = gTrue;
	    expectedType = isType0 ? fontCIDType0C : fontType1C;
	  }
	} else if (subtype.isName("OpenType")) {
	  if (expectedType == fontTrueType) {
	    expectedType = fontTrueTypeOT;
	  } else if (expectedType == fontType1) {
	    expectedType = fontType1COT;
	  } else if (expectedType == fontCIDType0) {
	    expectedType = fontCIDType0COT;
	  } else if (expectedType == fontCIDType2) {
	    expectedType = fontCIDType2OT;
	  } else {
	    err = gTrue;
	  }
	} else {
	  error(errSyntaxError, -1, "Unknown font type '{0:s}'",
		subtype.isName() ? subtype.getName() : "???");
	}
	subtype.free();
      }
      obj4.free();
    }
    obj3.free();
  }
  fontDesc.free();

  t = fontUnknownType;
  if (embID->num >= 0) {
    obj3.initRef(embID->num, embID->gen);
    obj3.fetch(xref, &obj4);
    if (obj4.isStream()) {
      obj4.streamReset();
      fft = FoFiIdentifier::identifyStream(&readFromStream, obj4.getStream());
      obj4.streamClose();
      switch (fft) {
      case fofiIdType1PFA:
      case fofiIdType1PFB:
	t = fontType1;
	break;
      case fofiIdCFF8Bit:
	t = isType0 ? fontCIDType0C : fontType1C;
	break;
      case fofiIdCFFCID:
	t = fontCIDType0C;
	break;
      case fofiIdTrueType:
      case fofiIdTrueTypeCollection:
	t = isType0 ? fontCIDType2 : fontTrueType;
	break;
      case fofiIdOpenTypeCFF8Bit:
	t = expectedType; // hack: open type always == expected type? s. bug-poppler20605.pdf
	break;
      case fofiIdOpenTypeCFFCID:
	t = fontCIDType0COT;
	break;
      default:
	error(errSyntaxError, -1, "Embedded font file may be invalid");
	break;
      }
    }
    obj4.free();
    obj3.free();
  }

  if (t == fontUnknownType) {
    t = expectedType;
  }

  if (t != expectedType) {
    err = gTrue;
  }

  if (err) {
    error(errSyntaxWarning, -1,
	  "Mismatch between font type and embedded font file");
  }

  obj2.free();
  obj1.free();

  return t;
}

void GfxFont::readFontDescriptor(XRef *xref, Dict *fontDict) {
  Object obj1, obj2, obj3, obj4;
  double t;
  int i;

  // assume Times-Roman by default (for substitution purposes)
  flags = fontSerif;

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
        error(errSyntaxWarning, -1, "The file uses Fontname instead of FontName please notify the creator that the file is broken");
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
      else error(errSyntaxWarning, -1, "Invalid Font Stretch");
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
      else error(errSyntaxWarning, -1, "Invalid Font Weight");
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
      // some broken font descriptors specify a negative ascent
      if (t < 0) {
	t = -t;
      }
      // some broken font descriptors set ascent and descent to 0;
      // others set it to ridiculous values (e.g., 32768)
      if (t != 0 && t < 3) {
	ascent = t;
      }
    }
    obj2.free();
    obj1.dictLookup("Descent", &obj2);
    if (obj2.isNum()) {
      t = 0.001 * obj2.getNum();
      // some broken font descriptors specify a positive descent
      if (t > 0) {
	t = -t;
      }
      // some broken font descriptors set ascent and descent to 0
      if (t != 0 && t > -3) {
	descent = t;
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

GfxFontLoc *GfxFont::locateFont(XRef *xref, GBool ps) {
  GfxFontLoc *fontLoc;
  SysFontType sysFontType;
  GooString *path, *base14Name, *substName;
  PSFontParam16 *psFont16;
  Object refObj, embFontObj;
  int substIdx, fontNum;
  GBool embed;

  if (type == fontType3) {
    return NULL;
  }

  //----- embedded font
  if (embFontID.num >= 0) {
    embed = gTrue;
    refObj.initRef(embFontID.num, embFontID.gen);
    refObj.fetch(xref, &embFontObj);
    if (!embFontObj.isStream()) {
      error(errSyntaxError, -1, "Embedded font object is wrong type");
      embed = gFalse;
    }
    embFontObj.free();
    refObj.free();
    if (embed) {
      if (ps) {
	switch (type) {
	case fontType1:
	case fontType1C:
	case fontType1COT:
	  embed = globalParams->getPSEmbedType1();
	  break;
	case fontTrueType:
	case fontTrueTypeOT:
	  embed = globalParams->getPSEmbedTrueType();
	  break;
	case fontCIDType0C:
	case fontCIDType0COT:
	  embed = globalParams->getPSEmbedCIDPostScript();
	  break;
	case fontCIDType2:
	case fontCIDType2OT:
	  embed = globalParams->getPSEmbedCIDTrueType();
	  break;
	default:
	  break;
	}
      }
      if (embed) {
	fontLoc = new GfxFontLoc();
	fontLoc->locType = gfxFontLocEmbedded;
	fontLoc->fontType = type;
	fontLoc->embFontID = embFontID;
	return fontLoc;
      }
    }
  }

  //----- PS passthrough
  if (ps && !isCIDFont() && globalParams->getPSFontPassthrough()) {
    fontLoc = new GfxFontLoc();
    fontLoc->locType = gfxFontLocResident;
    fontLoc->fontType = fontType1;
    fontLoc->path = name->copy();
    return fontLoc;
  }

  //----- PS resident Base-14 font
  if (ps && !isCIDFont() && ((Gfx8BitFont *)this)->base14) {
    fontLoc = new GfxFontLoc();
    fontLoc->locType = gfxFontLocResident;
    fontLoc->fontType = fontType1;
    fontLoc->path = new GooString(((Gfx8BitFont *)this)->base14->base14Name);
    return fontLoc;
  }

  //----- external font file (fontFile, fontDir)
  if (name && (path = globalParams->findFontFile(name))) {
    if ((fontLoc = getExternalFont(path, isCIDFont()))) {
      return fontLoc;
    }
  }

  //----- external font file for Base-14 font
  if (!ps && !isCIDFont() && ((Gfx8BitFont *)this)->base14) {
    base14Name = new GooString(((Gfx8BitFont *)this)->base14->base14Name);
    if ((path = globalParams->findBase14FontFile(base14Name, this))) {
      if ((fontLoc = getExternalFont(path, gFalse))) {
	delete base14Name;
	return fontLoc;
      }
    }
    delete base14Name;
  }

  //----- system font
  if ((path = globalParams->findSystemFontFile(this, &sysFontType,
					       &fontNum))) {
    if (isCIDFont()) {
      if (sysFontType == sysFontTTF || sysFontType == sysFontTTC) {
	fontLoc = new GfxFontLoc();
	fontLoc->locType = gfxFontLocExternal;
	fontLoc->fontType = fontCIDType2;
	fontLoc->path = path;
	fontLoc->fontNum = fontNum;
	return fontLoc;
      }
    } else {
      if (sysFontType == sysFontTTF || sysFontType == sysFontTTC) {
	fontLoc = new GfxFontLoc();
	fontLoc->locType = gfxFontLocExternal;
	fontLoc->fontType = fontTrueType;
	fontLoc->path = path;
	return fontLoc;
      } else if (sysFontType == sysFontPFA || sysFontType == sysFontPFB) {
	fontLoc = new GfxFontLoc();
	fontLoc->locType = gfxFontLocExternal;
	fontLoc->fontType = fontType1;
	fontLoc->path = path;
	fontLoc->fontNum = fontNum;
	return fontLoc;
      }
    }
    delete path;
  }

  if (!isCIDFont()) {

    //----- 8-bit PS resident font
    if (name && ps) {
      if ((path = globalParams->getPSResidentFont(name))) {
	fontLoc = new GfxFontLoc();
	fontLoc->locType = gfxFontLocResident;
	fontLoc->fontType = fontType1;
	fontLoc->path = path;
	return fontLoc;
      }
    }

    //----- 8-bit font substitution
    if (flags & fontFixedWidth) {
      substIdx = 0;
    } else if (flags & fontSerif) {
      substIdx = 8;
    } else {
      substIdx = 4;
    }
    if (isBold()) {
      substIdx += 2;
    }
    if (isItalic()) {
      substIdx += 1;
    }
    substName = new GooString(base14SubstFonts[substIdx]);
    if (ps) {
      error(errSyntaxWarning, -1, "Substituting font '{0:s}' for '{1:t}'",
	    base14SubstFonts[substIdx], name ? name : new GooString("null"));
      fontLoc = new GfxFontLoc();
      fontLoc->locType = gfxFontLocResident;
      fontLoc->fontType = fontType1;
      fontLoc->path = substName;
      fontLoc->substIdx = substIdx;
      return fontLoc;
    } else {
      path = globalParams->findFontFile(substName);
      delete substName;
      if (path) {
	if ((fontLoc = getExternalFont(path, gFalse))) {
	  error(errSyntaxWarning, -1, "Substituting font '{0:s}' for '{1:t}'",
		  base14SubstFonts[substIdx], (name == NULL) ? new GooString("") : name);
	  name = new GooString(base14SubstFonts[substIdx]);
	  fontLoc->substIdx = substIdx;
	  return fontLoc;
	}
      }
    }

    // failed to find a substitute font
    return NULL;
  }

  //----- 16-bit PS resident font
  if (ps && ((psFont16 = globalParams->getPSResidentFont16(
					 name,
					 ((GfxCIDFont *)this)->getWMode())))) {
    fontLoc = new GfxFontLoc();
    fontLoc->locType = gfxFontLocResident;
    fontLoc->fontType = fontCIDType0; // this is not used
    fontLoc->path = psFont16->psFontName->copy();
    fontLoc->encoding = psFont16->encoding->copy();
    fontLoc->wMode = psFont16->wMode;
    return fontLoc;
  }
  if (ps && ((psFont16 = globalParams->getPSResidentFontCC(
				 ((GfxCIDFont *)this)->getCollection(),
				 ((GfxCIDFont *)this)->getWMode())))) {
    error(errSyntaxWarning, -1, "Substituting font '{0:t}' for '{1:t}'",
	  psFont16->psFontName, name);
    fontLoc = new GfxFontLoc();
    fontLoc->locType = gfxFontLocResident;
    fontLoc->fontType = fontCIDType0; // this is not used
    fontLoc->path = psFont16->psFontName->copy();
    fontLoc->encoding = psFont16->encoding->copy();
    fontLoc->wMode = psFont16->wMode;
    return fontLoc;
  }

  //----- CID font substitution
  if ((path = globalParams->findCCFontFile(
				((GfxCIDFont *)this)->getCollection()))) {
    if ((fontLoc = getExternalFont(path, gTrue))) {
      error(errSyntaxWarning, -1, "Substituting font '{0:t}' for '{1:t}'",
	    fontLoc->path, name);
      return fontLoc;
    }
  }

  // failed to find a substitute font
  return NULL;
}

GfxFontLoc *GfxFont::locateBase14Font(GooString *base14Name) {
  GooString *path;

  path = globalParams->findFontFile(base14Name);
  if (!path) {
    return NULL;
  }
  return getExternalFont(path, gFalse);
}

GfxFontLoc *GfxFont::getExternalFont(GooString *path, GBool cid) {
  FoFiIdentifierType fft;
  GfxFontType fontType;
  GfxFontLoc *fontLoc;

  fft = FoFiIdentifier::identifyFile(path->getCString());
  switch (fft) {
  case fofiIdType1PFA:
  case fofiIdType1PFB:
    fontType = fontType1;
    break;
  case fofiIdCFF8Bit:
    fontType = fontType1C;
    break;
  case fofiIdCFFCID:
    fontType = fontCIDType0C;
    break;
  case fofiIdTrueType:
  case fofiIdTrueTypeCollection:
    fontType = cid ? fontCIDType2 : fontTrueType;
    break;
  case fofiIdOpenTypeCFF8Bit:
    fontType = fontType1COT;
    break;
  case fofiIdOpenTypeCFFCID:
    fontType = fontCIDType0COT;
    break;
  case fofiIdUnknown:
  case fofiIdError:
  default:
    fontType = fontUnknownType;
    break;
  }
  if (fontType == fontUnknownType ||
      (cid ? (fontType < fontCIDType0)
           : (fontType >= fontCIDType0))) {
    delete path;
    return NULL;
  }
  fontLoc = new GfxFontLoc();
  fontLoc->locType = gfxFontLocExternal;
  fontLoc->fontType = fontType;
  fontLoc->path = path;
  return fontLoc;
}

char *GfxFont::readEmbFontFile(XRef *xref, int *len) {
  char *buf;
  Object obj1, obj2;
  Stream *str;

  obj1.initRef(embFontID.num, embFontID.gen);
  obj1.fetch(xref, &obj2);
  if (!obj2.isStream()) {
    error(errSyntaxError, -1, "Embedded font file is not a stream");
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

Gfx8BitFont::Gfx8BitFont(XRef *xref, const char *tagA, Ref idA, GooString *nameA,
			 GfxFontType typeA, Ref embFontIDA, Dict *fontDict):
  GfxFont(tagA, idA, nameA, typeA, embFontIDA) {
  GooString *name2;
  BuiltinFont *builtinFont;
  const char **baseEnc;
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
  ctu = NULL;

  // do font name substitution for various aliases of the Base 14 font
  // names
  base14 = NULL;
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
    b = sizeof(base14FontMap) / sizeof(Base14FontMapEntry);
    // invariant: base14FontMap[a].altName <= name2 < base14FontMap[b].altName
    while (b - a > 1) {
      m = (a + b) / 2;
      if (name2->cmp(base14FontMap[m].altName) >= 0) {
	a = m;
      } else {
	b = m;
      }
    }
    if (!name2->cmp(base14FontMap[a].altName)) {
      base14 = &base14FontMap[a];
    }
    delete name2;
  }

  // is it a built-in font?
  builtinFont = NULL;
  if (base14) {
    for (i = 0; i < nBuiltinFonts; ++i) {
      if (!strcmp(base14->base14Name, builtinFonts[i].name)) {
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
      error(errSyntaxError, -1,
	    "Missing or invalid CharProcs dictionary in Type 3 font");
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

  // check embedded font file for base encoding
  // (only for Type 1 fonts - trying to get an encoding out of a
  // TrueType font is a losing proposition)
  ffT1 = NULL;
  ffT1C = NULL;
  buf = NULL;
  if (type == fontType1 && embFontID.num >= 0) {
    if ((buf = readEmbFontFile(xref, &len))) {
      if ((ffT1 = FoFiType1::make(buf, len))) {
	if (ffT1->getName()) {
	  if (embFontName) {
	    delete embFontName;
	  }
	  embFontName = new GooString(ffT1->getName());
	}
	if (!baseEnc) {
	  baseEnc = (const char **)ffT1->getEncoding();
	  baseEncFromFontFile = gTrue;
	}
      }
      gfree(buf);
    }
  } else if (type == fontType1C && embFontID.num >= 0) {
    if ((buf = readEmbFontFile(xref, &len))) {
      if ((ffT1C = FoFiType1C::make(buf, len))) {
	if (ffT1C->getName()) {
	  if (embFontName) {
	    delete embFontName;
	  }
	  embFontName = new GooString(ffT1C->getName());
	}
	if (!baseEnc) {
	  baseEnc = (const char **)ffT1C->getEncoding();
	  baseEncFromFontFile = gTrue;
	}
      }
      gfree(buf);
    }
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

  if (baseEncFromFontFile) {
    encodingName->Set("Builtin");
  } else if (baseEnc == winAnsiEncoding) {
    encodingName->Set("WinAnsi");
  } else if (baseEnc == macRomanEncoding) {
    encodingName->Set("MacRoman");
  } else if (baseEnc == macExpertEncoding) {
    encodingName->Set("MacExpert");
  } else if (baseEnc == symbolEncoding) {
    encodingName->Set("Symbol");
  } else if (baseEnc == zapfDingbatsEncoding) {
    encodingName->Set("ZapfDingbats");
  } else {
    encodingName->Set("Standard");
  }

  // copy the base encoding
  for (i = 0; i < 256; ++i) {
    enc[i] = (char *)baseEnc[i];
    if ((encFree[i] = baseEncFromFontFile) && enc[i]) {
      enc[i] = copyString(baseEnc[i]);
    }
  }

  // some Type 1C font files have empty encodings, which can break the
  // T1C->T1 conversion (since the 'seac' operator depends on having
  // the accents in the encoding), so we fill in any gaps from
  // StandardEncoding
  if (type == fontType1C && embFontID.num >= 0 && baseEncFromFontFile) {
    for (i = 0; i < 256; ++i) {
      if (!enc[i] && standardEncoding[i]) {
	enc[i] = (char *)standardEncoding[i];
	encFree[i] = gFalse;
      }
    }
  }

  // merge differences into encoding
  if (obj1.isDict()) {
    obj1.dictLookup("Differences", &obj2);
    if (obj2.isArray()) {
      encodingName->Set("Custom");
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
	  error(errSyntaxError, -1,
		"Wrong type in font encoding resource differences ({0:s})",
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

  // pass 1a: Expand ligatures in the Alphabetic Presentation Form
  // block (eg "fi", "ffi") to normal form
  for (code = 0; code < 256; ++code) {
    if (unicodeIsAlphabeticPresentationForm(toUnicode[code])) {
      Unicode *normalized = unicodeNormalizeNFKC(&toUnicode[code], 1, &len, NULL);
      if (len > 1)
        ctu->setMapping((CharCode)code, normalized, len);
      gfree(normalized);
    }
  }

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
	if (fabs(widths[code] - widths[firstChar]) > 0.00001) {
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
    error(errInternal, -1, "Zero-length output buffer (recursion overflow?) in "
	  "parseCharName, component \"{0:s}\"", charName);
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
	  error(errSyntaxWarning, -1, "Could not parse ligature component \"{0:s}\" of \"{1:s}\" in "
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

int *Gfx8BitFont::getCodeToGIDMap(FoFiTrueType *ff) {
  int *map;
  int cmapPlatform, cmapEncoding;
  int unicodeCmap, macRomanCmap, msSymbolCmap, cmap;
  GBool useMacRoman, useUnicode;
  char *charName;
  Unicode u;
  int code, i, n;

  map = (int *)gmallocn(256, sizeof(int));
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
  //    1b. If the PDF font is not symbolic or the PDF font is not
  //        embedded, and the TrueType font has a Microsoft Unicode
  //        cmap or a non-Microsoft Unicode cmap, use it, and use the
  //        Unicode indexes, not the char codes.
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
    } else if ((!(flags & fontSymbolic) || embFontID.num < 0) &&
	       unicodeCmap >= 0) {
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
      } else {
	map[i] = -1;
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
	else map[i] = -1;
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
    if (map[i] <= 0 && (charName = enc[i])) {
      map[i] = ff->mapNameToGID(charName);
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

struct cmpWidthExcepFunctor {
  bool operator()(const GfxFontCIDWidthExcep &w1,
		  const GfxFontCIDWidthExcep &w2) {
    return w1.first < w2.first;
  }
};

struct cmpWidthExcepVFunctor {
  bool operator()(const GfxFontCIDWidthExcepV &w1,
		  const GfxFontCIDWidthExcepV &w2) {
    return w1.first < w2.first;
  }
};

GfxCIDFont::GfxCIDFont(XRef *xref, const char *tagA, Ref idA, GooString *nameA,
		       GfxFontType typeA, Ref embFontIDA, Dict *fontDict):
  GfxFont(tagA, idA, nameA, typeA, embFontIDA)
{
  Dict *desFontDict;
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
  collection = NULL;
  cMap = NULL;
  ctu = NULL;
  ctuUsesCharCode = gTrue;
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
  if (!fontDict->lookup("DescendantFonts", &obj1)->isArray() ||
      obj1.arrayGetLength() == 0) {
    error(errSyntaxError, -1, "Missing or empty DescendantFonts entry in Type 0 font");
    obj1.free();
    goto err1;
  }
  if (!obj1.arrayGet(0, &desFontDictObj)->isDict()) {
    error(errSyntaxError, -1, "Bad descendant font in Type 0 font");
    goto err3;
  }
  obj1.free();
  desFontDict = desFontDictObj.getDict();

  // get info from font descriptor
  readFontDescriptor(xref, desFontDict);

  //----- encoding info -----

  // char collection
  if (!desFontDict->lookup("CIDSystemInfo", &obj1)->isDict()) {
    error(errSyntaxError, -1, "Missing CIDSystemInfo dictionary in Type 0 descendant font");
    goto err3;
  }
  obj1.dictLookup("Registry", &obj2);
  obj1.dictLookup("Ordering", &obj3);
  if (!obj2.isString() || !obj3.isString()) {
    error(errSyntaxError, -1, "Invalid CIDSystemInfo dictionary in Type 0 descendant font");
    goto err3;
  }
  collection = obj2.getString()->copy()->append('-')->append(obj3.getString());
  obj3.free();
  obj2.free();
  obj1.free();

  // look for a ToUnicode CMap
  if (!(ctu = readToUnicodeCMap(fontDict, 16, NULL))) {
    ctuUsesCharCode = gFalse;

    // use an identity mapping for the "Adobe-Identity" and
    // "Adobe-UCS" collections
    if (!collection->cmp("Adobe-Identity") ||
	!collection->cmp("Adobe-UCS")) {
      ctu = CharCodeToUnicode::makeIdentityMapping();
    } else {
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
	    error(errSyntaxError, -1, "Missing language pack for '{0:t}' mapping", collection);
	    goto err2;
	  }
	}
	error(errSyntaxError, -1, "Unknown character collection '{0:t}'",
	      collection);
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
  if (fontDict->lookup("Encoding", &obj1)->isNull()) {
    error(errSyntaxError, -1, "Missing Encoding entry in Type 0 font");
    goto err2;
  }
  if (!(cMap = CMap::parse(NULL, collection, &obj1))) {
    goto err2;
  }
  obj1.free();
  if (cMap->getCMapName()) {
    encodingName->Set(cMap->getCMapName()->getCString());
  } else {
    encodingName->Set("Custom");
  }

  // CIDToGIDMap (for embedded TrueType fonts)
  if (type == fontCIDType2 || type == fontCIDType2OT) {
    desFontDict->lookup("CIDToGIDMap", &obj1);
    if (obj1.isStream()) {
      cidToGIDLen = 0;
      i = 64;
      cidToGID = (int *)gmallocn(i, sizeof(int));
      obj1.streamReset();
      while ((c1 = obj1.streamGetChar()) != EOF &&
	     (c2 = obj1.streamGetChar()) != EOF) {
	if (cidToGIDLen == i) {
	  i *= 2;
	  cidToGID = (int *)greallocn(cidToGID, i, sizeof(int));
	}
	cidToGID[cidToGIDLen++] = (c1 << 8) + c2;
      }
    } else if (!obj1.isName("Identity") && !obj1.isNull()) {
      error(errSyntaxError, -1, "Invalid CIDToGIDMap entry in CID font");
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
	  error(errSyntaxError, -1, "Bad widths array in Type 0 font");
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
	    error(errSyntaxError, -1, "Bad widths array in Type 0 font");
	  }
	  obj4.free();
	}
	i += 2;
      } else {
	error(errSyntaxError, -1, "Bad widths array in Type 0 font");
	++i;
      }
      obj3.free();
      obj2.free();
    }
    std::sort(widths.exceps, widths.exceps + widths.nExceps,
	      cmpWidthExcepFunctor());
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
	  error(errSyntaxError, -1, "Bad widths (W2) array in Type 0 font");
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
	    error(errSyntaxError, -1, "Bad widths (W2) array in Type 0 font");
	  }
	  obj6.free();
	  obj5.free();
	  obj4.free();
	}
	i += 2;
      } else {
	error(errSyntaxError, -1, "Bad widths (W2) array in Type 0 font");
	++i;
      }
      obj3.free();
      obj2.free();
    }
    std::sort(widths.excepsV, widths.excepsV + widths.nExcepsV,
	      cmpWidthExcepVFunctor());
  }
  obj1.free();

  desFontDictObj.free();
  ok = gTrue;
  return;

 err3:
  obj3.free();
  obj2.free();
 err2:
  obj1.free();
  desFontDictObj.free();
 err1:;
}

GfxCIDFont::~GfxCIDFont() {
  if (collection) {
    delete collection;
  }
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
  CharCode c;
  double w, h, vx, vy;
  int n, a, b, m;

  if (!cMap) {
    *code = 0;
    *uLen = 0;
    *dx = *dy = 0;
    return 1;
  }

  *code = (CharCode)(cid = cMap->getCID(s, len, &c, &n));
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

int GfxCIDFont::mapCodeToGID(FoFiTrueType *ff, int cmapi,
  Unicode unicode, GBool wmode) {
  Gushort gid = ff->mapCodeToGID(cmapi,unicode);
  if (wmode) {
    Gushort vgid = ff->mapToVertGID(gid);
    if (vgid != 0) gid = vgid;
  }
  return gid;
}

int *GfxCIDFont::getCodeToGIDMap(FoFiTrueType *ff, int *mapsizep) {
#define N_UCS_CANDIDATES 2
  /* space characters */
  static const unsigned long spaces[] = { 
    0x2000,0x2001,0x2002,0x2003,0x2004,0x2005,0x2006,0x2007,
    0x2008,0x2009,0x200A,0x00A0,0x200B,0x2060,0x3000,0xFEFF,
    0
  };
  static const char *adobe_cns1_cmaps[] = {
    "UniCNS-UTF32-V",
    "UniCNS-UCS2-V",
    "UniCNS-UTF32-H",
    "UniCNS-UCS2-H",
    0
  };
  static const char *adobe_gb1_cmaps[] = {
    "UniGB-UTF32-V",
    "UniGB-UCS2-V",
    "UniGB-UTF32-H",
    "UniGB-UCS2-H",
    0
  };
  static const char *adobe_japan1_cmaps[] = {
    "UniJIS-UTF32-V",
    "UniJIS-UCS2-V",
    "UniJIS-UTF32-H",
    "UniJIS-UCS2-H",
    0
  };
  static const char *adobe_japan2_cmaps[] = {
    "UniHojo-UTF32-V",
    "UniHojo-UCS2-V",
    "UniHojo-UTF32-H",
    "UniHojo-UCS2-H",
    0
  };
  static const char *adobe_korea1_cmaps[] = {
    "UniKS-UTF32-V",
    "UniKS-UCS2-V",
    "UniKS-UTF32-H",
    "UniKS-UCS2-H",
    0
  };
  static struct CMapListEntry {
    const char *collection;
    const char *scriptTag;
    const char *languageTag;
    const char *toUnicodeMap;
    const char **CMaps;
  } CMapList[] = {
    {
      "Adobe-CNS1",
      "hani", "CHN ",
      "Adobe-CNS1-UCS2",
      adobe_cns1_cmaps,
    },
    {
      "Adobe-GB1",
      "hani", "CHN ",
      "Adobe-GB1-UCS2",
      adobe_gb1_cmaps,
    },
    {
      "Adobe-Japan1",
      "kana", "JAN ",
      "Adobe-Japan1-UCS2",
      adobe_japan1_cmaps,
    },
    {
      "Adobe-Japan2",
      "kana", "JAN ",
      "Adobe-Japan2-UCS2",
      adobe_japan2_cmaps,
    },
    {
      "Adobe-Korea1",
      "hang", "KOR ",
      "Adobe-Korea1-UCS2",
      adobe_korea1_cmaps,
    },
    {0, 0, 0, 0}
  };
  Unicode *humap = 0;
  Unicode *vumap = 0;
  Unicode *tumap = 0;
  int *codeToGID = 0;
  unsigned long n;
  int i;
  unsigned long code;
  int wmode;
  const char **cmapName;
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
    ff->setupGSUB(lp->scriptTag, lp->languageTag);
  } else {
    error(errSyntaxError, -1, "Unknown character collection {0:t}\n",
      getCollection());
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
  codeToGID = (int *)gmallocn(n, sizeof(int));
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
  CharCode c;

  CID cid = cMap->getCID(s, len, &c, &nUsed);

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
      error(errSyntaxError, -1, "font resource is not a dictionary");
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
