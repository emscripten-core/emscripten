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
// Copyright (C) 2011, 2012 Adrian Johnson <ajohnson@redneon.com>
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
struct GfxFontCIDWidths;
struct Base14FontMapEntry;

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
// GfxFontLoc
//------------------------------------------------------------------------

enum GfxFontLocType {
  gfxFontLocEmbedded,		// font embedded in PDF file
  gfxFontLocExternal,		// external font file
  gfxFontLocResident		// font resident in PS printer
};

class GfxFontLoc {
public:

  GfxFontLoc();
  ~GfxFontLoc();

  GfxFontLocType locType;
  GfxFontType fontType;
  Ref embFontID;		// embedded stream obj ID
				//   (if locType == gfxFontLocEmbedded)
  GooString *path;		// font file path
				//   (if locType == gfxFontLocExternal)
				// PS font name
				//   (if locType == gfxFontLocResident)
  int fontNum;			// for TrueType collections
				//   (if locType == gfxFontLocExternal)
  GooString *encoding;		// PS font encoding, only for 16-bit fonts
				//   (if locType == gfxFontLocResident)
  int wMode;			// writing mode, only for 16-bit fonts
				//   (if locType == gfxFontLocResident)
  int substIdx;			// substitute font index
				//   (if locType == gfxFontLocExternal,
				//   and a Base-14 substitution was made)
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
  static GfxFont *makeFont(XRef *xref, const char *tagA, Ref idA, Dict *fontDict);

  GfxFont(const char *tagA, Ref idA, GooString *nameA,
	  GfxFontType typeA, Ref embFontIDA);

  GBool isOk() { return ok; }

  void incRefCnt();
  void decRefCnt();

  // Get font tag.
  GooString *getTag() { return tag; }

  // Get font dictionary ID.
  Ref *getID() { return &id; }

  // Does this font match the tag?
  GBool matches(char *tagA) { return !tag->cmp(tagA); }

  // Get font family name.
  GooString *getFamily() { return family; }
  
  // Get font stretch.
  Stretch getStretch() { return stretch; }
  
  // Get font weight.
  Weight getWeight() { return weight; }

  // Get the original font name (ignornig any munging that might have
  // been done to map to a canonical Base-14 font name).
  GooString *getName() { return name; }

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

  // Locate the font file for this font.  If <ps> is true, includes PS
  // printer-resident fonts.  Returns NULL on failure.
  GfxFontLoc *locateFont(XRef *xref, GBool ps);

  // Locate a Base-14 font file for a specified font name.
  static GfxFontLoc *locateBase14Font(GooString *base14Name);

  // Read an external or embedded font file into a buffer.
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

  // Does this font have a toUnicode map?
  GBool hasToUnicodeCMap() { return hasToUnicode; }

  // Return the name of the encoding
  GooString *getEncodingName() { return encodingName; }

protected:

  virtual ~GfxFont();

  static GfxFontType getFontType(XRef *xref, Dict *fontDict, Ref *embID);
  void readFontDescriptor(XRef *xref, Dict *fontDict);
  CharCodeToUnicode *readToUnicodeCMap(Dict *fontDict, int nBits,
				       CharCodeToUnicode *ctu);
  static GfxFontLoc *getExternalFont(GooString *path, GBool cid);

  GooString *tag;			// PDF font tag
  Ref id;			// reference (used as unique ID)
  GooString *name;		// font name
  GooString *family;		// font family
  Stretch stretch;			// font stretch
  Weight weight;			// font weight
  GfxFontType type;		// type of font
  int flags;			// font descriptor flags
  GooString *embFontName;		// name of embedded font
  Ref embFontID;		// ref to embedded font file stream
  double fontMat[6];		// font matrix (Type 3 only)
  double fontBBox[4];		// font bounding box (Type 3 only)
  double missingWidth;		// "default" width
  double ascent;		// max height above baseline
  double descent;		// max depth below baseline
  int refCnt;
  GBool ok;
  GBool hasToUnicode;
  GooString *encodingName;
};

//------------------------------------------------------------------------
// Gfx8BitFont
//------------------------------------------------------------------------

class Gfx8BitFont: public GfxFont {
public:

  Gfx8BitFont(XRef *xref, const char *tagA, Ref idA, GooString *nameA,
	      GfxFontType typeA, Ref embFontIDA, Dict *fontDict);

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
  int *getCodeToGIDMap(FoFiTrueType *ff);

  // Return the Type 3 CharProc dictionary, or NULL if none.
  Dict *getCharProcs();

  // Return the Type 3 CharProc for the character associated with <code>.
  Object *getCharProc(int code, Object *proc);

  // Return the Type 3 Resources dictionary, or NULL if none.
  Dict *getResources();

private:
  virtual ~Gfx8BitFont();

  const Base14FontMapEntry *base14;	// for Base-14 fonts only; NULL otherwise
  char *enc[256];		// char code --> char name
  char encFree[256];		// boolean for each char name: if set,
				//   the string is malloc'ed
  CharCodeToUnicode *ctu;	// char code --> Unicode
  GBool hasEncoding;
  GBool usesMacRomanEnc;
  double widths[256];		// character widths
  Object charProcs;		// Type 3 CharProcs dictionary
  Object resources;		// Type 3 Resources dictionary

  friend class GfxFont;
};

//------------------------------------------------------------------------
// GfxCIDFont
//------------------------------------------------------------------------

class GfxCIDFont: public GfxFont {
public:

  GfxCIDFont(XRef *xref, const char *tagA, Ref idA, GooString *nameA,
	     GfxFontType typeA, Ref embFontIDA, Dict *fontDict);

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
  int *getCIDToGID() { return cidToGID; }
  int getCIDToGIDLen() { return cidToGIDLen; }

  int *getCodeToGIDMap(FoFiTrueType *ff, int *length);

  double getWidth(char* s, int len);

private:
  virtual ~GfxCIDFont();

  int mapCodeToGID(FoFiTrueType *ff, int cmapi,
    Unicode unicode, GBool wmode);

  GooString *collection;		// collection name
  CMap *cMap;			// char code --> CID
  CharCodeToUnicode *ctu;	// CID --> Unicode
  GBool ctuUsesCharCode;	// true: ctu maps char code to Unicode;
				//   false: ctu maps CID to Unicode
  GfxFontCIDWidths widths;	// character widths
  int *cidToGID;		// CID --> GID mapping (for embedded
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
