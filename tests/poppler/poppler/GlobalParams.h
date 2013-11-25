//========================================================================
//
// GlobalParams.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2007-2010, 2012 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright (C) 2006 Takashi Iwai <tiwai@suse.de>
// Copyright (C) 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2007 Krzysztof Kowalczyk <kkowalczyk@gmail.com>
// Copyright (C) 2009 Jonathan Kew <jonathan_kew@sil.org>
// Copyright (C) 2009 Petr Gajdos <pgajdos@novell.com>
// Copyright (C) 2009, 2011, 2012 William Bader <williambader@hotmail.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2011 Pino Toscano <pino@kde.org>
// Copyright (C) 2012 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Thomas Freitag <Thomas.Freitag@alfa.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef GLOBALPARAMS_H
#define GLOBALPARAMS_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <assert.h>
#include "poppler-config.h"
#include <stdio.h>
#include "goo/gtypes.h"
#include "CharTypes.h"

#if MULTITHREADED
#include "goo/GooMutex.h"
#endif

class GooString;
class GooList;
class GooHash;
class NameToCharCode;
class CharCodeToUnicode;
class CharCodeToUnicodeCache;
class UnicodeMap;
class UnicodeMapCache;
class CMap;
class CMapCache;
struct XpdfSecurityHandler;
class GlobalParams;
class GfxFont;
class Stream;
class SysFontList;

//------------------------------------------------------------------------

// The global parameters object.
extern GlobalParams *globalParams;

//------------------------------------------------------------------------

enum SysFontType {
  sysFontPFA,
  sysFontPFB,
  sysFontTTF,
  sysFontTTC
};

//------------------------------------------------------------------------

class PSFontParam16 {
public:

  GooString *name;		// PDF font name for psResidentFont16;
				//   char collection name for psResidentFontCC
  int wMode;			// writing mode (0=horiz, 1=vert)
  GooString *psFontName;		// PostScript font name
  GooString *encoding;		// encoding

  PSFontParam16(GooString *nameA, int wModeA,
		GooString *psFontNameA, GooString *encodingA);
  ~PSFontParam16();
};

//------------------------------------------------------------------------

enum PSLevel {
  psLevel1,
  psLevel1Sep,
  psLevel2,
  psLevel2Sep,
  psLevel3,
  psLevel3Sep
};

//------------------------------------------------------------------------

enum EndOfLineKind {
  eolUnix,			// LF
  eolDOS,			// CR+LF
  eolMac			// CR
};

//------------------------------------------------------------------------

enum ScreenType {
  screenUnset,
  screenDispersed,
  screenClustered,
  screenStochasticClustered
};

//------------------------------------------------------------------------

class GlobalParams {
public:

  // Initialize the global parameters by attempting to read a config
  // file.
  GlobalParams(const char *customPopplerDataDir = NULL);

  ~GlobalParams();

  void setupBaseFonts(char *dir);

  //----- accessors

  CharCode getMacRomanCharCode(char *charName);

  Unicode mapNameToUnicode(const char *charName);
  UnicodeMap *getResidentUnicodeMap(GooString *encodingName);
  FILE *getUnicodeMapFile(GooString *encodingName);
  FILE *findCMapFile(GooString *collection, GooString *cMapName);
  FILE *findToUnicodeFile(GooString *name);
  GooString *findFontFile(GooString *fontName);
  GooString *findBase14FontFile(GooString *base14Name, GfxFont *font);
  GooString *findSystemFontFile(GfxFont *font, SysFontType *type,
			      int *fontNum, GooString *substituteFontName = NULL, 
		              GooString *base14Name = NULL);
  GooString *findCCFontFile(GooString *collection);
  GBool getPSExpandSmaller();
  GBool getPSShrinkLarger();
  GBool getPSCenter();
  PSLevel getPSLevel();
  GooString *getPSResidentFont(GooString *fontName);
  GooList *getPSResidentFonts();
  PSFontParam16 *getPSResidentFont16(GooString *fontName, int wMode);
  PSFontParam16 *getPSResidentFontCC(GooString *collection, int wMode);
  GBool getPSEmbedType1();
  GBool getPSEmbedTrueType();
  GBool getPSEmbedCIDPostScript();
  GBool getPSEmbedCIDTrueType();
  GBool getPSFontPassthrough();
  GBool getPSPreload();
  GBool getPSOPI();
  GBool getPSASCIIHex();
  GBool getPSBinary();
  GBool getPSUncompressPreloadedImages();
  double getPSRasterResolution();
  GBool getPSRasterMono();
  GooString *getTextEncodingName();
  EndOfLineKind getTextEOL();
  GBool getTextPageBreaks();
  GBool getTextKeepTinyChars();
  GBool getEnableFreeType();
  GBool getAntialias();
  GBool getVectorAntialias();
  GBool getAntialiasPrinting();
  GBool getStrokeAdjust();
  ScreenType getScreenType();
  int getScreenSize();
  int getScreenDotRadius();
  double getScreenGamma();
  double getScreenBlackThreshold();
  double getScreenWhiteThreshold();
  double getMinLineWidth();
  GBool getOverprintPreview() { return overprintPreview; }
  GBool getMapNumericCharNames();
  GBool getMapUnknownCharNames();
  GBool getPrintCommands();
  GBool getProfileCommands();
  GBool getErrQuiet();
  double getSplashResolution();

  CharCodeToUnicode *getCIDToUnicode(GooString *collection);
  CharCodeToUnicode *getUnicodeToUnicode(GooString *fontName);
  UnicodeMap *getUnicodeMap(GooString *encodingName);
  CMap *getCMap(GooString *collection, GooString *cMapName, Stream *stream = NULL);
  UnicodeMap *getTextEncoding();
#ifdef ENABLE_PLUGINS
  GBool loadPlugin(char *type, char *name);
#endif

  GooList *getEncodingNames();

  //----- functions to set parameters
  void addFontFile(GooString *fontName, GooString *path);
  void setPSFile(char *file);
  void setPSExpandSmaller(GBool expand);
  void setPSShrinkLarger(GBool shrink);
  void setPSCenter(GBool center);
  void setPSLevel(PSLevel level);
  void setPSEmbedType1(GBool embed);
  void setPSEmbedTrueType(GBool embed);
  void setPSEmbedCIDPostScript(GBool embed);
  void setPSEmbedCIDTrueType(GBool embed);
  void setPSFontPassthrough(GBool passthrough);
  void setPSPreload(GBool preload);
  void setPSOPI(GBool opi);
  void setPSASCIIHex(GBool hex);
  void setPSBinary(GBool binary);
  void setPSUncompressPreloadedImages(GBool uncomp);
  void setPSRasterResolution(double res);
  void setPSRasterMono(GBool mono);
  void setTextEncoding(char *encodingName);
  GBool setTextEOL(char *s);
  void setTextPageBreaks(GBool pageBreaks);
  void setTextKeepTinyChars(GBool keep);
  GBool setEnableFreeType(char *s);
  GBool setDisableFreeTypeHinting(char *s);
  GBool setAntialias(char *s);
  GBool setVectorAntialias(char *s);
  void setAntialiasPrinting(GBool print);
  void setStrokeAdjust(GBool strokeAdjust);
  void setScreenType(ScreenType st);
  void setScreenSize(int size);
  void setScreenDotRadius(int radius);
  void setScreenGamma(double gamma);
  void setScreenBlackThreshold(double blackThreshold);
  void setScreenWhiteThreshold(double whiteThreshold);
  void setMinLineWidth(double minLineWidth);
  void setOverprintPreview(GBool overprintPreviewA);
  void setMapNumericCharNames(GBool map);
  void setMapUnknownCharNames(GBool map);
  void setPrintCommands(GBool printCommandsA);
  void setProfileCommands(GBool profileCommandsA);
  void setErrQuiet(GBool errQuietA);

  //----- security handlers

  void addSecurityHandler(XpdfSecurityHandler *handler);
  XpdfSecurityHandler *getSecurityHandler(char *name);

private:

  void parseNameToUnicode(GooString *name);
  GBool parseYesNo2(const char *token, GBool *flag);
  UnicodeMap *getUnicodeMap2(GooString *encodingName);

  void scanEncodingDirs();
  void addCIDToUnicode(GooString *collection, GooString *fileName);
  void addUnicodeMap(GooString *encodingName, GooString *fileName);
  void addCMapDir(GooString *collection, GooString *dir);

  //----- static tables

  NameToCharCode *		// mapping from char name to
    macRomanReverseMap;		//   MacRomanEncoding index

  //----- user-modifiable settings

  NameToCharCode *		// mapping from char name to Unicode
    nameToUnicode;
  GooHash *cidToUnicodes;		// files for mappings from char collections
				//   to Unicode, indexed by collection name
				//   [GooString]
  GooHash *unicodeToUnicodes;	// files for Unicode-to-Unicode mappings,
				//   indexed by font name pattern [GooString]
  GooHash *residentUnicodeMaps;	// mappings from Unicode to char codes,
				//   indexed by encoding name [UnicodeMap]
  GooHash *unicodeMaps;		// files for mappings from Unicode to char
				//   codes, indexed by encoding name [GooString]
  GooHash *cMapDirs;		// list of CMap dirs, indexed by collection
				//   name [GooList[GooString]]
  GooList *toUnicodeDirs;		// list of ToUnicode CMap dirs [GooString]
  GBool baseFontsInitialized;
#ifdef _WIN32
  GooHash *substFiles;	// windows font substitutes (for CID fonts)
#endif
  GooHash *fontFiles;		// font files: font name mapped to path
				//   [GString]
  GooList *fontDirs;		// list of font dirs [GString]
  GooHash *ccFontFiles;	// character collection font files:
				//   collection name  mapped to path [GString]
  SysFontList *sysFonts;	// system fonts
  GooString *psFile;		// PostScript file or command (for xpdf)
  GBool psExpandSmaller;	// expand smaller pages to fill paper
  GBool psShrinkLarger;		// shrink larger pages to fit paper
  GBool psCenter;		// center pages on the paper
  PSLevel psLevel;		// PostScript level to generate
  GooHash *psResidentFonts;	// 8-bit fonts resident in printer:
				//   PDF font name mapped to PS font name
				//   [GString]
  GooList *psResidentFonts16;	// 16-bit fonts resident in printer:
				//   PDF font name mapped to font info
				//   [PSFontParam16]
  GooList *psResidentFontsCC;	// 16-bit character collection fonts
				//   resident in printer: collection name
				//   mapped to font info [PSFontParam16]
  GBool psEmbedType1;		// embed Type 1 fonts?
  GBool psEmbedTrueType;	// embed TrueType fonts?
  GBool psEmbedCIDPostScript;	// embed CID PostScript fonts?
  GBool psEmbedCIDTrueType;	// embed CID TrueType fonts?
  GBool psFontPassthrough;	// pass all fonts through as-is?
  GBool psPreload;		// preload PostScript images and forms into
				//   memory
  GBool psOPI;			// generate PostScript OPI comments?
  GBool psASCIIHex;		// use ASCIIHex instead of ASCII85?
  GBool psBinary;		// use binary instead of hex
  GBool psUncompressPreloadedImages;  // uncompress all preloaded images
  double psRasterResolution;	// PostScript rasterization resolution (dpi)
  GBool psRasterMono;		// true to do PostScript rasterization
				//   in monochrome (gray); false to do it
				//   in color (RGB/CMYK)
  GooString *textEncoding;	// encoding (unicodeMap) to use for text
				//   output
  EndOfLineKind textEOL;	// type of EOL marker to use for text
				//   output
  GBool textPageBreaks;		// insert end-of-page markers?
  GBool textKeepTinyChars;	// keep all characters in text output
  GBool enableFreeType;		// FreeType enable flag
  GBool disableFreeTypeHinting;	// FreeType disable hinting flag
  GBool antialias;		// anti-aliasing enable flag
  GBool vectorAntialias;	// vector anti-aliasing enable flag
  GBool antialiasPrinting;	// allow anti-aliasing when printing
  GBool strokeAdjust;		// stroke adjustment enable flag
  ScreenType screenType;	// halftone screen type
  int screenSize;		// screen matrix size
  int screenDotRadius;		// screen dot radius
  double screenGamma;		// screen gamma correction
  double screenBlackThreshold;	// screen black clamping threshold
  double screenWhiteThreshold;	// screen white clamping threshold
  double minLineWidth;		// minimum line width
  GBool overprintPreview;	// enable overprint preview
  GBool mapNumericCharNames;	// map numeric char names (from font subsets)?
  GBool mapUnknownCharNames;	// map unknown char names?
  GBool printCommands;		// print the drawing commands
  GBool profileCommands;	// profile the drawing commands
  GBool errQuiet;		// suppress error messages?
  double splashResolution;	// resolution when rasterizing images

  CharCodeToUnicodeCache *cidToUnicodeCache;
  CharCodeToUnicodeCache *unicodeToUnicodeCache;
  UnicodeMapCache *unicodeMapCache;
  CMapCache *cMapCache;
  
#ifdef ENABLE_PLUGINS
  GooList *plugins;		// list of plugins [Plugin]
  GooList *securityHandlers;	// list of loaded security handlers
				//   [XpdfSecurityHandler]
#endif

#if MULTITHREADED
  GooMutex mutex;
  GooMutex unicodeMapCacheMutex;
  GooMutex cMapCacheMutex;
#endif

  const char *popplerDataDir;
};

#endif
