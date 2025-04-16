//========================================================================
//
// GfxFont.h
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
// Copyright (C) 2005, 2008 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006 Takashi Iwai <tiwai@suse.de>
// Copyright (C) 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2007 Koji Otani <sho@bbr.jp>
// Copyright (C) 2011 Axel Strübing <axel.struebing@freenet.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef GFXFONT_H
#define GFXFONT_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "goo/GooString.h"
#include "Object.h"
#include "CharTypes.h"

class Dict;
class CMap;
class CharCodeToUnicode;
class FoFiTrueType;
class DisplayFontParam;
struct GfxFontCIDWidths;

//------------------------------------------------------------------------
// GfxFontType
//------------------------------------------------------------------------

enum GfxFontType {
  //----- Gfx8BitFont
  fontUnknownType,
  fontType1,
  fontType1C,
  fontType1COT,
  fontType3,
  fontTrueType,
  fontTrueTypeOT,
  //----- GfxCIDFont
  fontCIDType0,
  fontCIDType0C,
  fontCIDType0COT,
  fontCIDType2,
  fontCIDType2OT
};

//------------------------------------------------------------------------
// GfxFontCIDWidths
//------------------------------------------------------------------------

struct GfxFontCIDWidthExcep {
  CID first;			// this record applies to
  CID last;			//   CIDs <first>..<last>
  double width;			// char width
};

struct GfxFontCIDWidthExcepV {
  CID first;			// this record applies to
  CID last;			//   CIDs <first>..<last>
  double height;		// char height
  double vx, vy;		// origin position
};

struct GfxFontCIDWidths {
  double defWidth;		// default char width
  double defHeight;		// default char height
  double defVY;			// default origin position
  GfxFontCIDWidthExcep *exceps;	// exceptions
  int nExceps;			// number of valid entries in exceps
  GfxFontCIDWidthExcepV *	// exceptions for vertical font
    excepsV;
  int nExcepsV;			// number of valid entries in excepsV
};

//------------------------------------------------------------------------
// GfxFont
//------------------------------------------------------------------------

#define fontFixedWidth (1 << 0)
#define fontSerif      (1 << 1)
#define fontSymbolic   (1 << 2)
#define fontItalic     (1 << 6)
#define fontBold       (1 << 18)

class GfxFont {
public:

  enum Stretch { 
	StretchNotDefined, 
	UltraCondensed, 
	ExtraCondensed, 
	Condensed, 
	SemiCondensed, 
	Normal, 
	SemiExpanded, 
	Expanded, 
	ExtraExpanded, 
	UltraExpanded };

  enum Weight { 
	WeightNotDefined, 
	W100, 
	W200, 
	W300, 
	W400, // Normal
	W500, 
	W600, 
	W700, // Bold
	W800, 
	W900 };

  // Build a GfxFont object.
  static GfxFont *makeFont(XRef *xref, char *tagA, Ref idA, Dict *fontDict);

  GfxFont(char *tagA, Ref idA, GooString *nameA);

  GBool isOk() { return ok; }

  void incRefCnt();
  void decRefCnt();

  // Get font tag.
  GooString *getTag() { return tag; }

  // Get font dictionary ID.
  Ref *getID() { return &id; }

  // Does this font match the tag?
  GBool matches(char *tagA) { return !tag->cmp(tagA); }

  // Get base font name.
  GooString *getName() { return name; }
  
  // Get font family name.
  GooString *getFamily() { return family; }
  
  // Get font stretch.
  Stretch getStretch() { return stretch; }
  
  // Get font weight.
  Weight getWeight() { return weight; }

  // Get the original font name (ignornig any munging that might have
  // been done to map to a canonical Base-14 font name).
  GooString *getOrigName() { return origName; }

  // Get font type.
  GfxFontType getType() { return type; }
  virtual GBool isCIDFont() { return gFalse; }

  // Get embedded font ID, i.e., a ref for the font file stream.
  // Returns false if there is no embedded font.
  GBool getEmbeddedFontID(Ref *embID)
    { *embID = embFontID; return embFontID.num >= 0; }

  // Get the PostScript font name for the embedded font.  Returns
  // NULL if there is no embedded font.
  GooString *getEmbeddedFontName() { return embFontName; }

  // Get the name of the external font file.  Returns NULL if there
  // is no external font file.
  GooString *getExtFontFile() { return extFontFile; }

  // Get font descriptor flags.
  int getFlags() { return flags; }
  GBool isFixedWidth() { return flags & fontFixedWidth; }
  GBool isSerif() { return flags & fontSerif; }
  GBool isSymbolic() { return flags & fontSymbolic; }
  GBool isItalic() { return flags & fontItalic; }
  GBool isBold() { return flags & fontBold; }

  // Return the Unicode map.
  virtual CharCodeToUnicode *getToUnicode() = 0;

  // Return the font matrix.
  double *getFontMatrix() { return fontMat; }

  // Return the font bounding box.
  double *getFontBBox() { return fontBBox; }

  // Return the ascent and descent values.
  double getAscent() { return ascent; }
  double getDescent() { return descent; }

  // Return the writing mode (0=horizontal, 1=vertical).
  virtual int getWMode() { return 0; }

  // Read an external or embedded font file into a buffer.
  char *readExtFontFile(int *len);
  char *readEmbFontFile(XRef *xref, int *len);

  // Get the next char from a string <s> of <len> bytes, returning the
  // char <code>, its Unicode mapping <u>, its displacement vector
  // (<dx>, <dy>), and its origin offset vector (<ox>, <oy>).  <uSize>
  // is the number of entries available in <u>, and <uLen> is set to
  // the number actually used.  Returns the number of bytes used by
  // the char code.
  virtual int getNextChar(char *s, int len, CharCode *code,
			  Unicode **u, int *uLen,
			  double *dx, double *dy, double *ox, double *oy) = 0;

  /* XXX: dfp shouldn't be public, however the font finding code is currently in
   * GlobalParams. Instead it should be inside the GfxFont class. However,
   * getDisplayFont currently uses FCcfg so moving it is not as simple. */
  /* XXX: related to this is the fact that all of the extFontFile stuff is dead */
  DisplayFontParam *dfp;
protected:

  virtual ~GfxFont();

  void readFontDescriptor(XRef *xref, Dict *fontDict);
  CharCodeToUnicode *readToUnicodeCMap(Dict *fontDict, int nBits,
				       CharCodeToUnicode *ctu);
  void findExtFontFile();

  GooString *tag;			// PDF font tag
  Ref id;			// reference (used as unique ID)
  GooString *name;		// font name
  GooString *family;		// font family
  Stretch stretch;			// font stretch
  Weight weight;			// font weight
  GooString *origName;		// original font name
  GfxFontType type;		// type of font
  int flags;			// font descriptor flags
  GooString *embFontName;		// name of embedded font
  Ref embFontID;		// ref to embedded font file stream
  GooString *extFontFile;		// external font file name
  double fontMat[6];		// font matrix (Type 3 only)
  double fontBBox[4];		// font bounding box (Type 3 only)
  double missingWidth;		// "default" width
  double ascent;		// max height above baseline
  double descent;		// max depth below baseline
  int refCnt;
  GBool ok;
  GBool hasToUnicode;
};

//------------------------------------------------------------------------
// Gfx8BitFont
//------------------------------------------------------------------------

class Gfx8BitFont: public GfxFont {
public:

  Gfx8BitFont(XRef *xref, char *tagA, Ref idA, GooString *nameA,
	      GfxFontType typeA, Dict *fontDict);

  virtual int getNextChar(char *s, int len, CharCode *code,
			  Unicode **u, int *uLen,
			  double *dx, double *dy, double *ox, double *oy);

  // Return the encoding.
  char **getEncoding() { return enc; }

  // Return the Unicode map.
  CharCodeToUnicode *getToUnicode();

  // Return the character name associated with <code>.
  char *getCharName(int code) { return enc[code]; }

  // Returns true if the PDF font specified an encoding.
  GBool getHasEncoding() { return hasEncoding; }

  // Returns true if the PDF font specified MacRomanEncoding.
  GBool getUsesMacRomanEnc() { return usesMacRomanEnc; }

  // Get width of a character.
  double getWidth(Guchar c) { return widths[c]; }

  // Return a char code-to-GID mapping for the provided font file.
  // (This is only useful for TrueType fonts.)
  Gushort *getCodeToGIDMap(FoFiTrueType *ff);

  // Return the Type 3 CharProc dictionary, or NULL if none.
  Dict *getCharProcs();

  // Return the Type 3 CharProc for the character associated with <code>.
  Object *getCharProc(int code, Object *proc);

  // Return the Type 3 Resources dictionary, or NULL if none.
  Dict *getResources();

private:
  virtual ~Gfx8BitFont();

  char *enc[256];		// char code --> char name
  char encFree[256];		// boolean for each char name: if set,
				//   the string is malloc'ed
  CharCodeToUnicode *ctu;	// char code --> Unicode
  GBool hasEncoding;
  GBool usesMacRomanEnc;
  double widths[256];		// character widths
  Object charProcs;		// Type 3 CharProcs dictionary
  Object resources;		// Type 3 Resources dictionary
};

//------------------------------------------------------------------------
// GfxCIDFont
//------------------------------------------------------------------------

class GfxCIDFont: public GfxFont {
public:

  GfxCIDFont(XRef *xref, char *tagA, Ref idA, GooString *nameA,
	     Dict *fontDict);

  virtual GBool isCIDFont() { return gTrue; }

  virtual int getNextChar(char *s, int len, CharCode *code,
			  Unicode **u, int *uLen,
			  double *dx, double *dy, double *ox, double *oy);

  // Return the writing mode (0=horizontal, 1=vertical).
  virtual int getWMode();

  // Return the Unicode map.
  CharCodeToUnicode *getToUnicode();

  // Get the collection name (<registry>-<ordering>).
  GooString *getCollection();

  // Return the CID-to-GID mapping table.  These should only be called
  // if type is fontCIDType2.
  Gushort *getCIDToGID() { return cidToGID; }
  int getCIDToGIDLen() { return cidToGIDLen; }

  Gushort *getCodeToGIDMap(FoFiTrueType *ff, int *length);

  double getWidth(char* s, int len);

private:
  virtual ~GfxCIDFont();

  Gushort mapCodeToGID(FoFiTrueType *ff, int cmapi,
    Unicode unicode, GBool wmode);

  CMap *cMap;			// char code --> CID
  CharCodeToUnicode *ctu;	// CID --> Unicode
  GfxFontCIDWidths widths;	// character widths
  Gushort *cidToGID;		// CID --> GID mapping (for embedded
				//   TrueType fonts)
  int cidToGIDLen;
};

//------------------------------------------------------------------------
// GfxFontDict
//------------------------------------------------------------------------

class GfxFontDict {
public:

  // Build the font dictionary, given the PDF font dictionary.
  GfxFontDict(XRef *xref, Ref *fontDictRef, Dict *fontDict);

  // Destructor.
  ~GfxFontDict();

  // Get the specified font.
  GfxFont *lookup(char *tag);

  // Iterative access.
  int getNumFonts() { return numFonts; }
  GfxFont *getFont(int i) { return fonts[i]; }

private:

  GfxFont **fonts;		// list of fonts
  int numFonts;			// number of fonts
};

#endif
