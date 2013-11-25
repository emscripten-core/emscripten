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
// Copyright (C) 2009-2011 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010, 2012 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 Jonathan Liu <net147@gmail.com>
// Copyright (C) 2010 William Bader <williambader@hotmail.com>
// Copyright (C) 2011-2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Adam Reichold <adamreichold@myopera.com>
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

// Uncomment to build pdftoppm with pthreads
// You may also have to change the buildsystem to
// link pdftoppm to pthread library
// This is here for developer testing not user ready
// #define UTILS_USE_PTHREADS 1

#ifdef UTILS_USE_PTHREADS
#include <errno.h>
#include <pthread.h>
#include <deque>
#endif // UTILS_USE_PTHREADS

static int firstPage = 1;
static int lastPage = 0;
static GBool printOnlyOdd = gFalse;
static GBool printOnlyEven = gFalse;
static GBool singleFile = gFalse;
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
static GBool jpegcmyk = gFalse;
static GBool tiff = gFalse;
#if SPLASH_CMYK
static GBool overprint = gFalse;
#endif
static char enableFreeTypeStr[16] = "";
static char antialiasStr[16] = "";
static char vectorAntialiasStr[16] = "";
static char ownerPassword[33] = "";
static char userPassword[33] = "";
static char TiffCompressionStr[16] = "";
static char thinLineModeStr[8] = "";
static SplashThinLineMode thinLineMode = splashThinLineDefault;
#ifdef UTILS_USE_PTHREADS
static int numberOfJobs = 1;
#endif // UTILS_USE_PTHREADS
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
  {"-singlefile", argFlag,  &singleFile,   0,
   "write only the first page and do not add digits"},

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
  {"-jpeg",   argFlag,     &jpeg,           0,
   "generate a JPEG file"},
#if SPLASH_CMYK
  {"-jpegcmyk",argFlag,    &jpegcmyk,       0,
   "generate a CMYK JPEG file"},
#endif
#endif
#if SPLASH_CMYK
  {"-overprint",argFlag,   &overprint,      0,
   "enable overprint"},
#endif
#if ENABLE_LIBTIFF
  {"-tiff",    argFlag,     &tiff,           0,
   "generate a TIFF file"},
  {"-tiffcompression", argString, TiffCompressionStr, sizeof(TiffCompressionStr),
   "set TIFF compression: none, packbits, jpeg, lzw, deflate"},
#endif
#if HAVE_FREETYPE_FREETYPE_H | HAVE_FREETYPE_H
  {"-freetype",   argString,      enableFreeTypeStr, sizeof(enableFreeTypeStr),
   "enable FreeType font rasterizer: yes, no"},
#endif
  {"-thinlinemode", argString, thinLineModeStr, sizeof(thinLineModeStr),
   "set thin line mode: none, solid, shape. Default: none"},
  
  {"-aa",         argString,      antialiasStr,   sizeof(antialiasStr),
   "enable font anti-aliasing: yes, no"},
  {"-aaVector",   argString,      vectorAntialiasStr, sizeof(vectorAntialiasStr),
   "enable vector anti-aliasing: yes, no"},
  
  {"-opw",    argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",    argString,   userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},
  
#ifdef UTILS_USE_PTHREADS
  {"-j",      argInt,      &numberOfJobs,  0,
   "number of jobs to run concurrently"},
#endif // UTILS_USE_PTHREADS

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
    } else if (jpegcmyk) {
      bitmap->writeImgFile(splashFormatJpegCMYK, ppmFile, x_resolution, y_resolution);
    } else if (tiff) {
      bitmap->writeImgFile(splashFormatTiff, ppmFile, x_resolution, y_resolution, TiffCompressionStr);
    } else {
      bitmap->writePNMFile(ppmFile);
    }
  } else {
#ifdef _WIN32
    setmode(fileno(stdout), O_BINARY);
#endif

    if (png) {
      bitmap->writeImgFile(splashFormatPng, stdout, x_resolution, y_resolution);
    } else if (jpeg) {
      bitmap->writeImgFile(splashFormatJpeg, stdout, x_resolution, y_resolution);
    } else if (tiff) {
      bitmap->writeImgFile(splashFormatTiff, stdout, x_resolution, y_resolution, TiffCompressionStr);
    } else {
      bitmap->writePNMFile(stdout);
    }
  }
}

#ifdef UTILS_USE_PTHREADS

struct PageJob {
  PDFDoc *doc;
  int pg;
  
  double pg_w, pg_h;
  SplashColor* paperColor;
  
  char *ppmFile;
};

static std::deque<PageJob> pageJobQueue;
static pthread_mutex_t pageJobMutex = PTHREAD_MUTEX_INITIALIZER;

static void processPageJobs() {
  while(true) {
    // pop the next job or exit if queue is empty
    pthread_mutex_lock(&pageJobMutex);
    
    if(pageJobQueue.empty()) {
      pthread_mutex_unlock(&pageJobMutex);
      return;
    }
    
    PageJob pageJob = pageJobQueue.front();
    pageJobQueue.pop_front();
    
    pthread_mutex_unlock(&pageJobMutex);
    
    // process the job    
    SplashOutputDev *splashOut = new SplashOutputDev(mono ? splashModeMono1 :
                  gray ? splashModeMono8 :
#if SPLASH_CMYK
        			    (jpegcmyk || overprint) ? splashModeDeviceN8 :
#endif
		              splashModeRGB8, 4, gFalse, *pageJob.paperColor, gTrue, gTrue, thinLineMode);
    splashOut->startDoc(pageJob.doc);
    
    savePageSlice(pageJob.doc, splashOut, pageJob.pg, x, y, w, h, pageJob.pg_w, pageJob.pg_h, pageJob.ppmFile);
    
    delete splashOut;
    delete[] pageJob.ppmFile;
  }
}

#endif // UTILS_USE_PTHREADS

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
  char *ppmFile;
  GooString *ownerPW, *userPW;
  SplashColor paperColor;
#ifndef UTILS_USE_PTHREADS
  SplashOutputDev *splashOut;
#else
  pthread_t* jobs;
#endif // UTILS_USE_PTHREADS
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
  if (thinLineModeStr[0]) {
    if (strcmp(thinLineModeStr, "solid") == 0) {
      thinLineMode = splashThinLineSolid;
    } else if (strcmp(thinLineModeStr, "shape") == 0) {
      thinLineMode = splashThinLineShape;
    } else if (strcmp(thinLineModeStr, "none") != 0) {
      fprintf(stderr, "Bad '-thinlinemode' value on command line\n");
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
  if (singleFile && lastPage < 1)
    lastPage = firstPage;
  if (lastPage < 1 || lastPage > doc->getNumPages())
    lastPage = doc->getNumPages();

  if (singleFile && firstPage < lastPage) {
    if (!quiet) {
      fprintf(stderr,
        "Warning: Single file will write only the first of the %d pages.\n",
        lastPage + 1 - firstPage);
    }
    lastPage = firstPage;
  }

  // write PPM files
#if SPLASH_CMYK
  if (jpegcmyk || overprint) {
    globalParams->setOverprintPreview(gTrue);
    for (int cp = 0; cp < SPOT_NCOMPS+4; cp++)
      paperColor[cp] = 0;
  } else 
#endif
  {
    paperColor[0] = 255;
    paperColor[1] = 255;
    paperColor[2] = 255;
  }
  
#ifndef UTILS_USE_PTHREADS

  splashOut = new SplashOutputDev(mono ? splashModeMono1 :
				    gray ? splashModeMono8 :
#if SPLASH_CMYK
				    (jpegcmyk || overprint) ? splashModeDeviceN8 :
#endif
				             splashModeRGB8, 4,
				  gFalse, paperColor, gTrue, gTrue, thinLineMode);
  splashOut->startDoc(doc);
  
#endif // UTILS_USE_PTHREADS
  
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
      if (x_scaleTo > 0) {
        x_resolution = (72.0 * x_scaleTo) / pg_w;
        if (y_scaleTo == -1)
          y_resolution = x_resolution;
      }
      if (y_scaleTo > 0) {
        y_resolution = (72.0 * y_scaleTo) / pg_h;
        if (x_scaleTo == -1)
          x_resolution = y_resolution;
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
      const char *ext = png ? "png" : (jpeg || jpegcmyk) ? "jpg" : tiff ? "tif" : mono ? "pbm" : gray ? "pgm" : "ppm";
      if (singleFile) {
        ppmFile = new char[strlen(ppmRoot) + 1 + strlen(ext) + 1];
        sprintf(ppmFile, "%s.%s", ppmRoot, ext);
      } else {
        ppmFile = new char[strlen(ppmRoot) + 1 + pg_num_len + 1 + strlen(ext) + 1];
        sprintf(ppmFile, "%s-%0*d.%s", ppmRoot, pg_num_len, pg, ext);
      }
    } else {
      ppmFile = NULL;
    }
#ifndef UTILS_USE_PTHREADS
    // process job in main thread
    savePageSlice(doc, splashOut, pg, x, y, w, h, pg_w, pg_h, ppmFile);
    
    delete[] ppmFile;
#else
    
    // queue job for worker threads
    PageJob pageJob = {
      .doc = doc,
      .pg = pg,
      
      .pg_w = pg_w, .pg_h = pg_h,
      
      .paperColor = &paperColor,
      
      .ppmFile = ppmFile
    };
    
    pageJobQueue.push_back(pageJob);
    
#endif // UTILS_USE_PTHREADS
  }
#ifndef UTILS_USE_PTHREADS
  delete splashOut;
#else
  
  // spawn worker threads and wait on them
  jobs = (pthread_t*)malloc(numberOfJobs * sizeof(pthread_t));

  for(int i=0; i < numberOfJobs; ++i) {
    if(pthread_create(&jobs[i], NULL, (void* (*)(void*))processPageJobs, NULL) != 0) {
	    fprintf(stderr, "pthread_create() failed with errno: %d\n", errno);
	    exit(EXIT_FAILURE);
    }
  }
  
  for(int i=0; i < numberOfJobs; ++i) {
    if(pthread_join(jobs[i], NULL) != 0) {
      fprintf(stderr, "pthread_join() failed with errno: %d\n", errno);
      exit(EXIT_FAILURE);
    }
  }

  free(jobs);
  
#endif // UTILS_USE_PTHREADS

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
