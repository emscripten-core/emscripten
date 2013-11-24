//========================================================================
//
// SplashTypes.h
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008 Tomas Are Haavet <tomasare@gmail.com>
// Copyright (C) 2009, 2011-2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2010 William Bader <williambader@hotmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHTYPES_H
#define SPLASHTYPES_H

#include "goo/gtypes.h"

//------------------------------------------------------------------------
// coordinates
//------------------------------------------------------------------------

#if USE_FIXEDPOINT
#include "goo/FixedPoint.h"
typedef FixedPoint SplashCoord;
#elif USE_FLOAT
typedef float SplashCoord;
#else
typedef double SplashCoord;
#endif

//------------------------------------------------------------------------
// antialiasing
//------------------------------------------------------------------------

#define splashAASize 4

#ifndef SPOT_NCOMPS
#define SPOT_NCOMPS 4
#endif

//------------------------------------------------------------------------
// colors
//------------------------------------------------------------------------

enum SplashColorMode {
  splashModeMono1,		// 1 bit per component, 8 pixels per byte,
				//   MSbit is on the left
  splashModeMono8,		// 1 byte per component, 1 byte per pixel
  splashModeRGB8,		// 1 byte per component, 3 bytes per pixel:
				//   RGBRGB...
  splashModeBGR8,		// 1 byte per component, 3 bytes per pixel:
				//   BGRBGR...
  splashModeXBGR8		// 1 byte per component, 4 bytes per pixel:
				//   XBGRXBGR...
#if SPLASH_CMYK
  ,
  splashModeCMYK8,	// 1 byte per component, 4 bytes per pixel:
				//   CMYKCMYK...
  splashModeDeviceN8		// 1 byte per component, 
                        // 4 bytes + n bytes spot colors per pixel:
				                // CMYKSSSSCMYKSSSS...
#endif
};

enum SplashThinLineMode {
  splashThinLineDefault,  // if SA on: draw solid if requested line width, transformed into 
                          // device space, is less than half a pixel and a shaped line else
  splashThinLineSolid,     // draw line solid at least with 1 pixel 
  splashThinLineShape     // draw line shaped at least with 1 pixel
};
// number of components in each color mode
// (defined in SplashState.cc)
extern int splashColorModeNComps[];

// max number of components in any SplashColor
#if SPLASH_CMYK
#define splashMaxColorComps SPOT_NCOMPS+4
#else
#define splashMaxColorComps 4
#endif

typedef Guchar SplashColor[splashMaxColorComps];
typedef Guchar *SplashColorPtr;

// RGB8
static inline Guchar splashRGB8R(SplashColorPtr rgb8) { return rgb8[0]; }
static inline Guchar splashRGB8G(SplashColorPtr rgb8) { return rgb8[1]; }
static inline Guchar splashRGB8B(SplashColorPtr rgb8) { return rgb8[2]; }

// BGR8
static inline Guchar splashBGR8R(SplashColorPtr bgr8) { return bgr8[2]; }
static inline Guchar splashBGR8G(SplashColorPtr bgr8) { return bgr8[1]; }
static inline Guchar splashBGR8B(SplashColorPtr bgr8) { return bgr8[0]; }

#if SPLASH_CMYK
// CMYK8
static inline Guchar splashCMYK8C(SplashColorPtr cmyk8) { return cmyk8[0]; }
static inline Guchar splashCMYK8M(SplashColorPtr cmyk8) { return cmyk8[1]; }
static inline Guchar splashCMYK8Y(SplashColorPtr cmyk8) { return cmyk8[2]; }
static inline Guchar splashCMYK8K(SplashColorPtr cmyk8) { return cmyk8[3]; }

// DEVICEN8
static inline Guchar splashDeviceN8C(SplashColorPtr deviceN8) { return deviceN8[0]; }
static inline Guchar splashDeviceN8M(SplashColorPtr deviceN8) { return deviceN8[1]; }
static inline Guchar splashDeviceN8Y(SplashColorPtr deviceN8) { return deviceN8[2]; }
static inline Guchar splashDeviceN8K(SplashColorPtr deviceN8) { return deviceN8[3]; }
static inline Guchar splashDeviceN8S(SplashColorPtr deviceN8, int nSpot) { return deviceN8[4 + nSpot]; }
#endif

static inline void splashClearColor(SplashColorPtr dest) {
  dest[0] = 0;
  dest[1] = 0;
  dest[2] = 0;
#if SPLASH_CMYK
  dest[3] = 0;
  for (int i = SPOT_NCOMPS; i < SPOT_NCOMPS + 4; i++)
    dest[i] = 0;
#endif
}

static inline void splashColorCopy(SplashColorPtr dest, SplashColorPtr src) {
  dest[0] = src[0];
  dest[1] = src[1];
  dest[2] = src[2];
#if SPLASH_CMYK
  dest[3] = src[3];
  for (int i = SPOT_NCOMPS; i < SPOT_NCOMPS + 4; i++)
    dest[i] = src[i];
#endif
}

static inline void splashColorXor(SplashColorPtr dest, SplashColorPtr src) {
  dest[0] ^= src[0];
  dest[1] ^= src[1];
  dest[2] ^= src[2];
#if SPLASH_CMYK
  dest[3] ^= src[3];
  for (int i = SPOT_NCOMPS; i < SPOT_NCOMPS + 4; i++)
    dest[i] ^= src[i];
#endif
}

//------------------------------------------------------------------------
// blend functions
//------------------------------------------------------------------------

typedef void (*SplashBlendFunc)(SplashColorPtr src, SplashColorPtr dest,
				SplashColorPtr blend, SplashColorMode cm);

//------------------------------------------------------------------------
// screen parameters
//------------------------------------------------------------------------

enum SplashScreenType {
  splashScreenDispersed,
  splashScreenClustered,
  splashScreenStochasticClustered
};

struct SplashScreenParams {
  SplashScreenType type;
  int size;
  int dotRadius;
  SplashCoord gamma;
  SplashCoord blackThreshold;
  SplashCoord whiteThreshold;
};

//------------------------------------------------------------------------
// error results
//------------------------------------------------------------------------

typedef int SplashError;


//------------------------------------------------------------------------
// image file formats
//------------------------------------------------------------------------

enum SplashImageFileFormat {
  splashFormatJpeg,
  splashFormatPng,
  splashFormatTiff,
  splashFormatJpegCMYK
};

#endif
