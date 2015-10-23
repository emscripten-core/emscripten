//========================================================================
//
// pdftoppm.cc
//
// Copyright 2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2007 Ilmari Heikkinen <ilmari.heikkinen@gmail.com>
// Copyright (C) 2008 Richard Airlie <richard.airlie@maglabs.net>
// Copyright (C) 2009 Michael K. Johnson <a1237@danlj.org>
// Copyright (C) 2009 Shen Liang <shenzhuxi@gmail.com>
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2009, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 Jonathan Liu <net147@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#ifdef _WIN32
#include <fcntl.h> // for O_BINARY
#include <io.h>    // for setmode
#endif
#include <stdio.h>
#include <math.h>
#include "parseargs.h"
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "GlobalParams.h"
#include "Object.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "splash/SplashBitmap.h"
#include "splash/Splash.h"
#include "SplashOutputDev.h"

#define PPM_FILE_SZ 512

static int firstPage = 1;
static int lastPage = 0;
static GBool printOnlyOdd = gFalse;
static GBool printOnlyEven = gFalse;
static double resolution = 0.0;
static double x_resolution = 150.0;
static double y_resolution = 150.0;
static int scaleTo = 0;
static int x_scaleTo = 0;
static int y_scaleTo = 0;
static int x = 0;
static int y = 0;
static int w = 0;
static int h = 0;
static int sz = 0;
static GBool useCropBox = gFalse;
static GBool mono = gFalse;
static GBool gray = gFalse;
static GBool png = gFalse;
static GBool jpeg = gFalse;
static char enableFreeTypeStr[16] = "";
static char antialiasStr[16] = "";
static char vectorAntialiasStr[16] = "";
static char ownerPassword[33] = "";
static char userPassword[33] = "";
static GBool quiet = gFalse;
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static const ArgDesc argDesc[] = {
  {"-f",      argInt,      &firstPage,     0,
   "first page to print"},
  {"-l",      argInt,      &lastPage,      0,
   "last page to print"},
  {"-o",      argFlag,      &printOnlyOdd, 0,
   "print only odd pages"},
  {"-e",      argFlag,      &printOnlyEven, 0,
   "print only even pages"},

  {"-r",      argFP,       &resolution,    0,
   "resolution, in DPI (default is 150)"},
  {"-rx",      argFP,       &x_resolution,    0,
   "X resolution, in DPI (default is 150)"},
  {"-ry",      argFP,       &y_resolution,    0,
   "Y resolution, in DPI (default is 150)"},
  {"-scale-to",      argInt,       &scaleTo,    0,
   "scales each page to fit within scale-to*scale-to pixel box"},
  {"-scale-to-x",      argInt,       &x_scaleTo,    0,
   "scales each page horizontally to fit in scale-to-x pixels"},
  {"-scale-to-y",      argInt,       &y_scaleTo,    0,
   "scales each page vertically to fit in scale-to-y pixels"},

  {"-x",      argInt,      &x,             0,
   "x-coordinate of the crop area top left corner"},
  {"-y",      argInt,      &y,             0,
   "y-coordinate of the crop area top left corner"},
  {"-W",      argInt,      &w,             0,
   "width of crop area in pixels (default is 0)"},
  {"-H",      argInt,      &h,             0,
   "height of crop area in pixels (default is 0)"},
  {"-sz",     argInt,      &sz,            0,
   "size of crop square in pixels (sets W and H)"},
  {"-cropbox",argFlag,     &useCropBox,    0,
   "use the crop box rather than media box"},

  {"-mono",   argFlag,     &mono,          0,
   "generate a monochrome PBM file"},
  {"-gray",   argFlag,     &gray,          0,
   "generate a grayscale PGM file"},
#if ENABLE_LIBPNG
  {"-png",    argFlag,     &png,           0,
   "generate a PNG file"},
#endif
#if ENABLE_LIBJPEG
  {"-jpeg",    argFlag,     &jpeg,           0,
   "generate a JPEG file"},
#endif
#if HAVE_FREETYPE_FREETYPE_H | HAVE_FREETYPE_H
  {"-freetype",   argString,      enableFreeTypeStr, sizeof(enableFreeTypeStr),
   "enable FreeType font rasterizer: yes, no"},
#endif
  
  {"-aa",         argString,      antialiasStr,   sizeof(antialiasStr),
   "enable font anti-aliasing: yes, no"},
  {"-aaVector",   argString,      vectorAntialiasStr, sizeof(vectorAntialiasStr),
   "enable vector anti-aliasing: yes, no"},
  
  {"-opw",    argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",    argString,   userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},
  
  {"-q",      argFlag,     &quiet,         0,
   "don't print any messages or errors"},
  {"-v",      argFlag,     &printVersion,  0,
   "print copyright and version info"},
  {"-h",      argFlag,     &printHelp,     0,
   "print usage information"},
  {"-help",   argFlag,     &printHelp,     0,
   "print usage information"},
  {"--help",  argFlag,     &printHelp,     0,
   "print usage information"},
  {"-?",      argFlag,     &printHelp,     0,
   "print usage information"},
  {NULL}
};

static void savePageSlice(PDFDoc *doc,
                   SplashOutputDev *splashOut, 
                   int pg, int x, int y, int w, int h, 
                   double pg_w, double pg_h, 
                   char *ppmFile) {
  if (w == 0) w = (int)ceil(pg_w);
  if (h == 0) h = (int)ceil(pg_h);
  w = (x+w > pg_w ? (int)ceil(pg_w-x) : w);
  h = (y+h > pg_h ? (int)ceil(pg_h-y) : h);
  doc->displayPageSlice(splashOut, 
    pg, x_resolution, y_resolution, 
    0,
    !useCropBox, gFalse, gFalse,
    x, y, w, h
  );

  SplashBitmap *bitmap = splashOut->getBitmap();
  
  if (ppmFile != NULL) {
    if (png) {
      bitmap->writeImgFile(splashFormatPng, ppmFile, x_resolution, y_resolution);
    } else if (jpeg) {
      bitmap->writeImgFile(splashFormatJpeg, ppmFile, x_resolution, y_resolution);
    } else {
      bitmap->writePNMFile(ppmFile);
    }
  } else {
#ifdef __EMSCRIPTEN__ // XXX EMSCRIPTEN: avoid writing to stdout, better for benchmarking
    printf("avoiding writing to stdout\n");
#else

#ifdef _WIN32
    setmode(fileno(stdout), O_BINARY);
#endif

    if (png) {
      bitmap->writeImgFile(splashFormatPng, stdout, x_resolution, y_resolution);
    } else if (jpeg) {
      bitmap->writeImgFile(splashFormatJpeg, stdout, x_resolution, y_resolution);
    } else {
      bitmap->writePNMFile(stdout);
    }
#endif
  }
}

static int numberOfCharacters(unsigned int n)
{
  int charNum = 0;
  while (n >= 10)
  {
    n = n / 10;
    charNum++;
  }
  charNum++;
  return charNum;
}

int main(int argc, char *argv[]) {
  PDFDoc *doc;
  GooString *fileName = NULL;
  char *ppmRoot = NULL;
  char ppmFile[PPM_FILE_SZ];
  GooString *ownerPW, *userPW;
  SplashColor paperColor;
  SplashOutputDev *splashOut;
  GBool ok;
  int exitCode;
  int pg, pg_num_len;
  double pg_w, pg_h, tmp;

  exitCode = 99;

  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if (mono && gray) {
    ok = gFalse;
  }
  if ( resolution != 0.0 &&
       (x_resolution == 150.0 ||
        y_resolution == 150.0)) {
    x_resolution = resolution;
    y_resolution = resolution;
  }
  if (!ok || argc > 3 || printVersion || printHelp) {
    fprintf(stderr, "pdftoppm version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "%s\n", popplerCopyright);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdftoppm", "[PDF-file [PPM-file-prefix]]", argDesc);
    }
    if (printVersion || printHelp)
      exitCode = 0;
    goto err0;
  }
  if (argc > 1) fileName = new GooString(argv[1]);
  if (argc == 3) ppmRoot = argv[2];

  // read config file
  globalParams = new GlobalParams();
  if (enableFreeTypeStr[0]) {
    if (!globalParams->setEnableFreeType(enableFreeTypeStr)) {
      fprintf(stderr, "Bad '-freetype' value on command line\n");
    }
  }
  if (antialiasStr[0]) {
    if (!globalParams->setAntialias(antialiasStr)) {
      fprintf(stderr, "Bad '-aa' value on command line\n");
    }
  }
  if (vectorAntialiasStr[0]) {
    if (!globalParams->setVectorAntialias(vectorAntialiasStr)) {
      fprintf(stderr, "Bad '-aaVector' value on command line\n");
    }
  }
  if (quiet) {
    globalParams->setErrQuiet(quiet);
  }

  // open PDF file
  if (ownerPassword[0]) {
    ownerPW = new GooString(ownerPassword);
  } else {
    ownerPW = NULL;
  }
  if (userPassword[0]) {
    userPW = new GooString(userPassword);
  } else {
    userPW = NULL;
  }

  if (fileName == NULL) {
    fileName = new GooString("fd://0");
  }
  if (fileName->cmp("-") == 0) {
    delete fileName;
    fileName = new GooString("fd://0");
  }
  doc = PDFDocFactory().createPDFDoc(*fileName, ownerPW, userPW);
  delete fileName;

  if (userPW) {
    delete userPW;
  }
  if (ownerPW) {
    delete ownerPW;
  }
  if (!doc->isOk()) {
    exitCode = 1;
    goto err1;
  }

  // get page range
  if (firstPage < 1)
    firstPage = 1;
  if (lastPage < 1 || lastPage > doc->getNumPages())
    lastPage = doc->getNumPages();

  // write PPM files
  paperColor[0] = 255;
  paperColor[1] = 255;
  paperColor[2] = 255;
  splashOut = new SplashOutputDev(mono ? splashModeMono1 :
				    gray ? splashModeMono8 :
				             splashModeRGB8, 4,
				  gFalse, paperColor);
  splashOut->startDoc(doc->getXRef());
  if (sz != 0) w = h = sz;
  pg_num_len = numberOfCharacters(doc->getNumPages());
  for (pg = firstPage; pg <= lastPage; ++pg) {
    if (printOnlyEven && pg % 2 == 0) continue;
    if (printOnlyOdd && pg % 2 == 1) continue;
    if (useCropBox) {
      pg_w = doc->getPageCropWidth(pg);
      pg_h = doc->getPageCropHeight(pg);
    } else {
      pg_w = doc->getPageMediaWidth(pg);
      pg_h = doc->getPageMediaHeight(pg);
    }

    if (scaleTo != 0) {
      resolution = (72.0 * scaleTo) / (pg_w > pg_h ? pg_w : pg_h);
      x_resolution = y_resolution = resolution;
    } else {
      if (x_scaleTo != 0) {
        x_resolution = (72.0 * x_scaleTo) / pg_w;
      }
      if (y_scaleTo != 0) {
        y_resolution = (72.0 * y_scaleTo) / pg_h;
      }
    }
    pg_w = pg_w * (x_resolution / 72.0);
    pg_h = pg_h * (y_resolution / 72.0);
    if ((doc->getPageRotate(pg) == 90) || (doc->getPageRotate(pg) == 270)) {
      tmp = pg_w;
      pg_w = pg_h;
      pg_h = tmp;
    }
    if (ppmRoot != NULL) {
      snprintf(ppmFile, PPM_FILE_SZ, "%.*s-%0*d.%s",
              PPM_FILE_SZ - 32, ppmRoot, pg_num_len, pg,
              png ? "png" : jpeg ? "jpg" : mono ? "pbm" : gray ? "pgm" : "ppm");
      savePageSlice(doc, splashOut, pg, x, y, w, h, pg_w, pg_h, ppmFile);
    } else {
      savePageSlice(doc, splashOut, pg, x, y, w, h, pg_w, pg_h, NULL);
    }
  }
  delete splashOut;

  exitCode = 0;

  // clean up
 err1:
  delete doc;
  delete globalParams;
 err0:

  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return exitCode;
}
