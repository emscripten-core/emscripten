//========================================================================
//
// GfxState.cc
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
// Copyright (C) 2005 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2006, 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2006, 2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2006-2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009, 2012 Koji Otani <sho@bbr.jp>
// Copyright (C) 2009, 2011-2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2009 Christian Persch <chpe@gnome.org>
// Copyright (C) 2010 Paweł Wiejacha <pawel.wiejacha@gmail.com>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2011 Andrea Canciani <ranma42@gmail.com>
// Copyright (C) 2012 William Bader <williambader@hotmail.com>
// Copyright (C) 2013 Lu Wang <coolwanglu@gmail.com>
// Copyright (C) 2013 Hib Eris <hib@hiberis.nl>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <algorithm>
#include <stddef.h>
#include <math.h>
#include <string.h>
#include "goo/gmem.h"
#include "Error.h"
#include "Object.h"
#include "Array.h"
#include "Page.h"
#include "Gfx.h"
#include "GfxState.h"
#include "GfxState_helpers.h"
#include "GfxFont.h"
#include "GlobalParams.h"
#include "PopplerCache.h"
#include "OutputDev.h"
#include "splash/SplashTypes.h"

//------------------------------------------------------------------------

// Max depth of nested color spaces.  This is used to catch infinite
// loops in the color space object structure.
#define colorSpaceRecursionLimit 8

//------------------------------------------------------------------------

GBool Matrix::invertTo(Matrix *other) const
{
  double det;

  det = 1 / determinant();
  other->m[0] = m[3] * det;
  other->m[1] = -m[1] * det;
  other->m[2] = -m[2] * det;
  other->m[3] = m[0] * det;
  other->m[4] = (m[2] * m[5] - m[3] * m[4]) * det;
  other->m[5] = (m[1] * m[4] - m[0] * m[5]) * det;

  return gTrue;
}

void Matrix::transform(double x, double y, double *tx, double *ty) const
{
  double temp_x, temp_y;

  temp_x = m[0] * x + m[2] * y + m[4];
  temp_y = m[1] * x + m[3] * y + m[5];

  *tx = temp_x;
  *ty = temp_y;
}

// Matrix norm, taken from _cairo_matrix_transformed_circle_major_axis
double Matrix::norm() const
{
  double f, g, h, i, j;

  i = m[0]*m[0] + m[1]*m[1];
  j = m[2]*m[2] + m[3]*m[3];

  f = 0.5 * (i + j);
  g = 0.5 * (i - j);
  h = m[0]*m[2] + m[1]*m[3];

  return sqrt (f + hypot (g, h));
}

//------------------------------------------------------------------------

struct GfxBlendModeInfo {
  const char *name;
  GfxBlendMode mode;
};

static const GfxBlendModeInfo gfxBlendModeNames[] = {
  { "Normal",     gfxBlendNormal },
  { "Compatible", gfxBlendNormal },
  { "Multiply",   gfxBlendMultiply },
  { "Screen",     gfxBlendScreen },
  { "Overlay",    gfxBlendOverlay },
  { "Darken",     gfxBlendDarken },
  { "Lighten",    gfxBlendLighten },
  { "ColorDodge", gfxBlendColorDodge },
  { "ColorBurn",  gfxBlendColorBurn },
  { "HardLight",  gfxBlendHardLight },
  { "SoftLight",  gfxBlendSoftLight },
  { "Difference", gfxBlendDifference },
  { "Exclusion",  gfxBlendExclusion },
  { "Hue",        gfxBlendHue },
  { "Saturation", gfxBlendSaturation },
  { "Color",      gfxBlendColor },
  { "Luminosity", gfxBlendLuminosity }
};

#define nGfxBlendModeNames \
          ((int)((sizeof(gfxBlendModeNames) / sizeof(GfxBlendModeInfo))))
	 
//------------------------------------------------------------------------
// 
// NB: This must match the GfxColorSpaceMode enum defined in
// GfxState.h
static const char *gfxColorSpaceModeNames[] = {
  "DeviceGray",
  "CalGray",
  "DeviceRGB",
  "CalRGB",
  "DeviceCMYK",
  "Lab",
  "ICCBased",
  "Indexed",
  "Separation",
  "DeviceN",
  "Pattern"
};

#define nGfxColorSpaceModes ((sizeof(gfxColorSpaceModeNames) / sizeof(char *)))

#ifdef USE_CMS

static const std::map<unsigned int, unsigned int>::size_type CMSCACHE_LIMIT = 2048;

#ifdef USE_LCMS1
#include <lcms.h>
#define cmsColorSpaceSignature icColorSpaceSignature
#define cmsSetLogErrorHandler cmsSetErrorHandler
#define cmsSigXYZData icSigXYZData
#define cmsSigLuvData icSigLuvData
#define cmsSigLabData icSigLabData
#define cmsSigYCbCrData icSigYCbCrData
#define cmsSigYxyData icSigYxyData
#define cmsSigRgbData icSigRgbData
#define cmsSigHsvData icSigHsvData
#define cmsSigHlsData icSigHlsData
#define cmsSigCmyData icSigCmyData
#define cmsSig3colorData icSig3colorData
#define cmsSigGrayData icSigGrayData
#define cmsSigCmykData icSigCmykData
#define cmsSig4colorData icSig4colorData
#define cmsSig2colorData icSig2colorData
#define cmsSig5colorData icSig5colorData
#define cmsSig6colorData icSig6colorData
#define cmsSig7colorData icSig7colorData
#define cmsSig8colorData icSig8colorData
#define cmsSig9colorData icSig9colorData
#define cmsSig10colorData icSig10colorData
#define cmsSig11colorData icSig11colorData
#define cmsSig12colorData icSig12colorData
#define cmsSig13colorData icSig13colorData
#define cmsSig14colorData icSig14colorData
#define cmsSig15colorData icSig15colorData
#define LCMS_FLAGS 0
#else
#include <lcms2.h>
#define LCMS_FLAGS cmsFLAGS_NOOPTIMIZE
#endif

#define COLOR_PROFILE_DIR "/ColorProfiles/"
#define GLOBAL_COLOR_PROFILE_DIR POPPLER_DATADIR COLOR_PROFILE_DIR

void GfxColorTransform::doTransform(void *in, void *out, unsigned int size) {
  cmsDoTransform(transform, in, out, size);
}

// transformA should be a cmsHTRANSFORM
GfxColorTransform::GfxColorTransform(void *transformA, int cmsIntentA, unsigned int transformPixelTypeA) {
  transform = transformA;
  refCount = 1;
  cmsIntent = cmsIntentA;
  transformPixelType = transformPixelTypeA;
}

GfxColorTransform::~GfxColorTransform() {
  cmsDeleteTransform(transform);
}

void GfxColorTransform::ref() {
  refCount++;
}

unsigned int GfxColorTransform::unref() {
  return --refCount;
}

static cmsHPROFILE RGBProfile = NULL;
static GooString *displayProfileName = NULL; // display profile file Name
static cmsHPROFILE displayProfile = NULL; // display profile
static unsigned int displayPixelType = 0;
static GfxColorTransform *XYZ2DisplayTransform = NULL;

// convert color space signature to cmsColor type 
static unsigned int getCMSColorSpaceType(cmsColorSpaceSignature cs);
static unsigned int getCMSNChannels(cmsColorSpaceSignature cs);
static cmsHPROFILE loadColorProfile(const char *fileName);

void GfxColorSpace::setDisplayProfile(void *displayProfileA) {
  displayProfile = displayProfileA;
  if (displayProfile != NULL) {
    cmsHTRANSFORM transform;
    unsigned int nChannels;

    displayPixelType = getCMSColorSpaceType(cmsGetColorSpace(displayProfile));
    nChannels = getCMSNChannels(cmsGetColorSpace(displayProfile));
    // create transform from XYZ
    cmsHPROFILE XYZProfile = cmsCreateXYZProfile();
    if ((transform = cmsCreateTransform(XYZProfile, TYPE_XYZ_DBL,
	   displayProfile,
	   COLORSPACE_SH(displayPixelType) |
	     CHANNELS_SH(nChannels) | BYTES_SH(1),
	  INTENT_RELATIVE_COLORIMETRIC,LCMS_FLAGS)) == 0) {
      error(errSyntaxWarning, -1, "Can't create Lab transform");
    } else {
      XYZ2DisplayTransform = new GfxColorTransform(transform, INTENT_RELATIVE_COLORIMETRIC, displayPixelType);
    }
    cmsCloseProfile(XYZProfile);
  }
}

void GfxColorSpace::setDisplayProfileName(GooString *name) {
  displayProfileName = name->copy();
}

cmsHPROFILE GfxColorSpace::getRGBProfile() {
  return RGBProfile;
}

cmsHPROFILE GfxColorSpace::getDisplayProfile() {
  return displayProfile;
}

#endif

//------------------------------------------------------------------------
// GfxColorSpace
//------------------------------------------------------------------------

GfxColorSpace::GfxColorSpace() {
  overprintMask = 0x0f;
  mapping = NULL;
}

GfxColorSpace::~GfxColorSpace() {
}

GfxColorSpace *GfxColorSpace::parse(Object *csObj, OutputDev *out, GfxState *state, int recursion) {
  GfxColorSpace *cs;
  Object obj1;

  if (recursion > colorSpaceRecursionLimit) {
    error(errSyntaxError, -1, "Loop detected in color space objects");
    return NULL;
  }

  cs = NULL;
  if (csObj->isName()) {
    if (csObj->isName("DeviceGray") || csObj->isName("G")) {
      cs = new GfxDeviceGrayColorSpace();
    } else if (csObj->isName("DeviceRGB") || csObj->isName("RGB")) {
      cs = new GfxDeviceRGBColorSpace();
    } else if (csObj->isName("DeviceCMYK") || csObj->isName("CMYK")) {
      cs = new GfxDeviceCMYKColorSpace();
    } else if (csObj->isName("Pattern")) {
      cs = new GfxPatternColorSpace(NULL);
    } else {
      error(errSyntaxWarning, -1, "Bad color space '{0:s}'", csObj->getName());
    }
  } else if (csObj->isArray() && csObj->arrayGetLength() > 0) {
    csObj->arrayGet(0, &obj1);
    if (obj1.isName("DeviceGray") || obj1.isName("G")) {
      cs = new GfxDeviceGrayColorSpace();
    } else if (obj1.isName("DeviceRGB") || obj1.isName("RGB")) {
      cs = new GfxDeviceRGBColorSpace();
    } else if (obj1.isName("DeviceCMYK") || obj1.isName("CMYK")) {
      cs = new GfxDeviceCMYKColorSpace();
    } else if (obj1.isName("CalGray")) {
      cs = GfxCalGrayColorSpace::parse(csObj->getArray(), state);
    } else if (obj1.isName("CalRGB")) {
      cs = GfxCalRGBColorSpace::parse(csObj->getArray(), state);
    } else if (obj1.isName("Lab")) {
      cs = GfxLabColorSpace::parse(csObj->getArray(), state);
    } else if (obj1.isName("ICCBased")) {
      cs = GfxICCBasedColorSpace::parse(csObj->getArray(), out, state, recursion);
    } else if (obj1.isName("Indexed") || obj1.isName("I")) {
      cs = GfxIndexedColorSpace::parse(csObj->getArray(), out, state, recursion);
    } else if (obj1.isName("Separation")) {
      cs = GfxSeparationColorSpace::parse(csObj->getArray(), out, state, recursion);
    } else if (obj1.isName("DeviceN")) {
      cs = GfxDeviceNColorSpace::parse(csObj->getArray(), out, state, recursion);
    } else if (obj1.isName("Pattern")) {
      cs = GfxPatternColorSpace::parse(csObj->getArray(), out, state, recursion);
    } else {
      error(errSyntaxWarning, -1, "Bad color space");
    }
    obj1.free();
  } else if (csObj->isDict()) {
    csObj->dictLookup("ColorSpace", &obj1);
    if (obj1.isName("DeviceGray")) {
      cs = new GfxDeviceGrayColorSpace();
    } else if (obj1.isName("DeviceRGB")) {
      cs = new GfxDeviceRGBColorSpace();
    } else if (obj1.isName("DeviceCMYK")) {
      cs = new GfxDeviceCMYKColorSpace();
    } else {
      error(errSyntaxWarning, -1, "Bad color space dict'");
    }
    obj1.free();
  } else {
    error(errSyntaxWarning, -1, "Bad color space - expected name or array or dict");
  }
  return cs;
}

void GfxColorSpace::createMapping(GooList *separationList, int maxSepComps) {
  return;
}

void GfxColorSpace::getDefaultRanges(double *decodeLow, double *decodeRange,
				     int maxImgPixel) {
  int i;

  for (i = 0; i < getNComps(); ++i) {
    decodeLow[i] = 0;
    decodeRange[i] = 1;
  }
}

int GfxColorSpace::getNumColorSpaceModes() {
  return nGfxColorSpaceModes;
}

const char *GfxColorSpace::getColorSpaceModeName(int idx) {
  return gfxColorSpaceModeNames[idx];
}

#ifdef USE_CMS
cmsHPROFILE loadColorProfile(const char *fileName)
{
  cmsHPROFILE hp = NULL;
  FILE *fp;

  if (fileName[0] == '/') {
    // full path
    // check if open the file
    if ((fp = fopen(fileName,"r")) != NULL) {
      fclose(fp);
      hp = cmsOpenProfileFromFile(fileName,"r");
    }
    return hp;
  }
  // try to load from global directory
  GooString *path = new GooString(GLOBAL_COLOR_PROFILE_DIR);
  path->append(fileName);
  // check if open the file
  if ((fp = fopen(path->getCString(),"r")) != NULL) {
    fclose(fp);
    hp = cmsOpenProfileFromFile(path->getCString(),"r");
  }
  delete path;
  return hp;
}

#ifdef USE_LCMS1
static int CMSError(int ecode, const char *msg)
{
    error(errSyntaxWarning, -1, "{0:s}", msg);
    return 1;
}
#else
static void CMSError(cmsContext /*contextId*/, cmsUInt32Number /*ecode*/, const char *text)
{
    error(errSyntaxWarning, -1, "{0:s}", text);
}
#endif

int GfxColorSpace::setupColorProfiles()
{
  static GBool initialized = gFalse;
  cmsHTRANSFORM transform;
  unsigned int nChannels;

  // do only once
  if (initialized) return 0;
  initialized = gTrue;

  // set error handlor
  cmsSetLogErrorHandler(CMSError);

  if (displayProfile == NULL) {
    // load display profile if it was not already loaded.
    if (displayProfileName == NULL) {
      displayProfile = loadColorProfile("display.icc");
    } else if (displayProfileName->getLength() > 0) {
      displayProfile = loadColorProfile(displayProfileName->getCString());
    }
  }
  // load RGB profile
  RGBProfile = loadColorProfile("RGB.icc");
  if (RGBProfile == NULL) {
    /* use built in sRGB profile */
    RGBProfile = cmsCreate_sRGBProfile();
  }
  // create transforms
  if (displayProfile != NULL) {
    displayPixelType = getCMSColorSpaceType(cmsGetColorSpace(displayProfile));
    nChannels = getCMSNChannels(cmsGetColorSpace(displayProfile));
    // create transform from XYZ
    cmsHPROFILE XYZProfile = cmsCreateXYZProfile();
    if ((transform = cmsCreateTransform(XYZProfile, TYPE_XYZ_DBL,
	   displayProfile,
	   COLORSPACE_SH(displayPixelType) |
	     CHANNELS_SH(nChannels) | BYTES_SH(1),
	  INTENT_RELATIVE_COLORIMETRIC,LCMS_FLAGS)) == 0) {
      error(errSyntaxWarning, -1, "Can't create Lab transform");
    } else {
      XYZ2DisplayTransform = new GfxColorTransform(transform, INTENT_RELATIVE_COLORIMETRIC, displayPixelType);
    }
    cmsCloseProfile(XYZProfile);
  }
  return 0;
}

unsigned int getCMSColorSpaceType(cmsColorSpaceSignature cs)
{
    switch (cs) {
    case cmsSigXYZData:
      return PT_XYZ;
      break;
    case cmsSigLabData:
      return PT_Lab;
      break;
    case cmsSigLuvData:
      return PT_YUV;
      break;
    case cmsSigYCbCrData:
      return PT_YCbCr;
      break;
    case cmsSigYxyData:
      return PT_Yxy;
      break;
    case cmsSigRgbData:
      return PT_RGB;
      break;
    case cmsSigGrayData:
      return PT_GRAY;
      break;
    case cmsSigHsvData:
      return PT_HSV;
      break;
    case cmsSigHlsData:
      return PT_HLS;
      break;
    case cmsSigCmykData:
      return PT_CMYK;
      break;
    case cmsSigCmyData:
      return PT_CMY;
      break;
    case cmsSig2colorData:
    case cmsSig3colorData:
    case cmsSig4colorData:
    case cmsSig5colorData:
    case cmsSig6colorData:
    case cmsSig7colorData:
    case cmsSig8colorData:
    case cmsSig9colorData:
    case cmsSig10colorData:
    case cmsSig11colorData:
    case cmsSig12colorData:
    case cmsSig13colorData:
    case cmsSig14colorData:
    case cmsSig15colorData:
    default:
      break;
    }
    return PT_RGB;
}

unsigned int getCMSNChannels(cmsColorSpaceSignature cs)
{
    switch (cs) {
    case cmsSigXYZData:
    case cmsSigLuvData:
    case cmsSigLabData:
    case cmsSigYCbCrData:
    case cmsSigYxyData:
    case cmsSigRgbData:
    case cmsSigHsvData:
    case cmsSigHlsData:
    case cmsSigCmyData:
    case cmsSig3colorData:
      return 3;
      break;
    case cmsSigGrayData:
      return 1;
      break;
    case cmsSigCmykData:
    case cmsSig4colorData:
      return 4;
      break;
    case cmsSig2colorData:
      return 2;
      break;
    case cmsSig5colorData:
      return 5;
      break;
    case cmsSig6colorData:
      return 6;
      break;
    case cmsSig7colorData:
      return 7;
      break;
    case cmsSig8colorData:
      return 8;
      break;
    case cmsSig9colorData:
      return 9;
      break;
    case cmsSig10colorData:
      return 10;
      break;
    case cmsSig11colorData:
      return 11;
      break;
    case cmsSig12colorData:
      return 12;
      break;
    case cmsSig13colorData:
      return 13;
      break;
    case cmsSig14colorData:
      return 14;
      break;
    case cmsSig15colorData:
      return 15;
    default:
      break;
    }
    return 3;
}
#endif

//------------------------------------------------------------------------
// GfxDeviceGrayColorSpace
//------------------------------------------------------------------------

GfxDeviceGrayColorSpace::GfxDeviceGrayColorSpace() {
}

GfxDeviceGrayColorSpace::~GfxDeviceGrayColorSpace() {
}

GfxColorSpace *GfxDeviceGrayColorSpace::copy() {
  return new GfxDeviceGrayColorSpace();
}

void GfxDeviceGrayColorSpace::getGray(GfxColor *color, GfxGray *gray) {
  *gray = clip01(color->c[0]);
}

void GfxDeviceGrayColorSpace::getGrayLine(Guchar *in, Guchar *out, int length) {
  memcpy (out, in, length);
}

void GfxDeviceGrayColorSpace::getRGB(GfxColor *color, GfxRGB *rgb) {
  rgb->r = rgb->g = rgb->b = clip01(color->c[0]);
}

void GfxDeviceGrayColorSpace::getRGBLine(Guchar *in, unsigned int *out,
					 int length) {
  int i;

  for (i = 0; i < length; i++)
    out[i] = (in[i] << 16) | (in[i] << 8) | (in[i] << 0);
}

void GfxDeviceGrayColorSpace::getRGBLine(Guchar *in, Guchar *out, int length) {
  for (int i = 0; i < length; i++) {
    *out++ = in[i];
    *out++ = in[i];
    *out++ = in[i];
  }
}

void GfxDeviceGrayColorSpace::getRGBXLine(Guchar *in, Guchar *out, int length) {
  for (int i = 0; i < length; i++) {
    *out++ = in[i];
    *out++ = in[i];
    *out++ = in[i];
    *out++ = 255;
  }
}

void GfxDeviceGrayColorSpace::getCMYKLine(Guchar *in, Guchar *out, int length) {
  for (int i = 0; i < length; i++) {
    *out++ = 0;
    *out++ = 0;
    *out++ = 0;
    *out++ = in[i];
  }
}

void GfxDeviceGrayColorSpace::getDeviceNLine(Guchar *in, Guchar *out, int length) {
  for (int i = 0; i < length; i++) {
    for (int j = 0; j < SPOT_NCOMPS+4; j++)
      out[j] = 0;
    out[4] = in[i];
    out += (SPOT_NCOMPS+4);
  }
}

void GfxDeviceGrayColorSpace::getCMYK(GfxColor *color, GfxCMYK *cmyk) {
  cmyk->c = cmyk->m = cmyk->y = 0;
  cmyk->k = clip01(gfxColorComp1 - color->c[0]);
}

void GfxDeviceGrayColorSpace::getDeviceN(GfxColor *color, GfxColor *deviceN) {
  for (int i = 0; i < gfxColorMaxComps; i++)
    deviceN->c[i] = 0;
  deviceN->c[3] = clip01(gfxColorComp1 - color->c[0]);
}

void GfxDeviceGrayColorSpace::getDefaultColor(GfxColor *color) {
  color->c[0] = 0;
}

//------------------------------------------------------------------------
// GfxCalGrayColorSpace
//------------------------------------------------------------------------

GfxCalGrayColorSpace::GfxCalGrayColorSpace() {
  whiteX = whiteY = whiteZ = 1;
  blackX = blackY = blackZ = 0;
  gamma = 1;
}

GfxCalGrayColorSpace::~GfxCalGrayColorSpace() {
#ifdef USE_CMS
  if (transform != NULL) {
    if (transform->unref() == 0) delete transform;
  }
#endif
}

GfxColorSpace *GfxCalGrayColorSpace::copy() {
  GfxCalGrayColorSpace *cs;

  cs = new GfxCalGrayColorSpace();
  cs->whiteX = whiteX;
  cs->whiteY = whiteY;
  cs->whiteZ = whiteZ;
  cs->blackX = blackX;
  cs->blackY = blackY;
  cs->blackZ = blackZ;
  cs->gamma = gamma;
#ifdef USE_CMS
  cs->transform = transform;
  if (transform != NULL) transform->ref();
#endif
  return cs;
}

// This is the inverse of MatrixLMN in Example 4.10 from the PostScript
// Language Reference, Third Edition.
static const double xyzrgb[3][3] = {
  {  3.240449, -1.537136, -0.498531 },
  { -0.969265,  1.876011,  0.041556 },
  {  0.055643, -0.204026,  1.057229 }
};

GfxColorSpace *GfxCalGrayColorSpace::parse(Array *arr, GfxState *state) {
  GfxCalGrayColorSpace *cs;
  Object obj1, obj2, obj3;

  arr->get(1, &obj1);
  if (!obj1.isDict()) {
    error(errSyntaxWarning, -1, "Bad CalGray color space");
    obj1.free();
    return NULL;
  }
  cs = new GfxCalGrayColorSpace();
  if (obj1.dictLookup("WhitePoint", &obj2)->isArray() &&
      obj2.arrayGetLength() == 3) {
    obj2.arrayGet(0, &obj3);
    if (likely(obj3.isNum()))
      cs->whiteX = obj3.getNum();
    obj3.free();
    obj2.arrayGet(1, &obj3);
    if (likely(obj3.isNum()))
      cs->whiteY = obj3.getNum();
    obj3.free();
    obj2.arrayGet(2, &obj3);
    if (likely(obj3.isNum()))
      cs->whiteZ = obj3.getNum();
    obj3.free();
  }
  obj2.free();
  if (obj1.dictLookup("BlackPoint", &obj2)->isArray() &&
      obj2.arrayGetLength() == 3) {
    obj2.arrayGet(0, &obj3);
    if (likely(obj3.isNum()))
      cs->blackX = obj3.getNum();
    obj3.free();
    obj2.arrayGet(1, &obj3);
    if (likely(obj3.isNum()))
      cs->blackY = obj3.getNum();
    obj3.free();
    obj2.arrayGet(2, &obj3);
    if (likely(obj3.isNum()))
      cs->blackZ = obj3.getNum();
    obj3.free();
  }
  obj2.free();
  if (obj1.dictLookup("Gamma", &obj2)->isNum()) {
    cs->gamma = obj2.getNum();
  }
  obj2.free();
  obj1.free();

  cs->kr = 1 / (xyzrgb[0][0] * cs->whiteX +
		xyzrgb[0][1] * cs->whiteY +
		xyzrgb[0][2] * cs->whiteZ);
  cs->kg = 1 / (xyzrgb[1][0] * cs->whiteX +
		xyzrgb[1][1] * cs->whiteY +
		xyzrgb[1][2] * cs->whiteZ);
  cs->kb = 1 / (xyzrgb[2][0] * cs->whiteX +
		xyzrgb[2][1] * cs->whiteY +
		xyzrgb[2][2] * cs->whiteZ);
#ifdef USE_CMS
  cs->transform = (state != NULL) ? state->getXYZ2DisplayTransform() : XYZ2DisplayTransform;
  if (cs->transform != NULL) cs->transform->ref();
#endif
  return cs;
}

// convert CalGray to media XYZ color space
// (not multiply by the white point)
void GfxCalGrayColorSpace::getXYZ(GfxColor *color, 
  double *pX, double *pY, double *pZ) {
  const double A = colToDbl(color->c[0]);
  const double xyzColor = pow(A,gamma);
  *pX = xyzColor;
  *pY = xyzColor;
  *pZ = xyzColor;
}

void GfxCalGrayColorSpace::getGray(GfxColor *color, GfxGray *gray) {
  GfxRGB rgb;

#ifdef USE_CMS
  if (transform != NULL && transform->getTransformPixelType() == PT_GRAY) {
    Guchar out[gfxColorMaxComps];
    double in[gfxColorMaxComps];
    double X, Y, Z;
    
    getXYZ(color,&X,&Y,&Z);
    in[0] = clip01(X);
    in[1] = clip01(Y);
    in[2] = clip01(Z);
    transform->doTransform(in,out,1);
    *gray = byteToCol(out[0]);
    return;
  }
#endif
  getRGB(color, &rgb);
  *gray = clip01((GfxColorComp)(0.299 * rgb.r +
				0.587 * rgb.g +
				0.114 * rgb.b + 0.5));
}

void GfxCalGrayColorSpace::getRGB(GfxColor *color, GfxRGB *rgb) {
  double X, Y, Z;
  double r, g, b;

  getXYZ(color,&X,&Y,&Z);
#ifdef USE_CMS
  if (transform != NULL && transform->getTransformPixelType() == PT_RGB) {
    Guchar out[gfxColorMaxComps];
    double in[gfxColorMaxComps];
    
    in[0] = clip01(X);
    in[1] = clip01(Y);
    in[2] = clip01(Z);
    transform->doTransform(in,out,1);
    rgb->r = byteToCol(out[0]);
    rgb->g = byteToCol(out[1]);
    rgb->b = byteToCol(out[2]);
    return;
  }
#endif
  X *= whiteX;
  Y *= whiteY;
  Z *= whiteZ;
  // convert XYZ to RGB, including gamut mapping and gamma correction
  r = xyzrgb[0][0] * X + xyzrgb[0][1] * Y + xyzrgb[0][2] * Z;
  g = xyzrgb[1][0] * X + xyzrgb[1][1] * Y + xyzrgb[1][2] * Z;
  b = xyzrgb[2][0] * X + xyzrgb[2][1] * Y + xyzrgb[2][2] * Z;
  rgb->r = dblToCol(sqrt(clip01(r * kr)));
  rgb->g = dblToCol(sqrt(clip01(g * kg)));
  rgb->b = dblToCol(sqrt(clip01(b * kb)));
}

void GfxCalGrayColorSpace::getCMYK(GfxColor *color, GfxCMYK *cmyk) {
  GfxRGB rgb;
  GfxColorComp c, m, y, k;

#ifdef USE_CMS
  if (transform != NULL && transform->getTransformPixelType() == PT_CMYK) {
    double in[gfxColorMaxComps];
    Guchar out[gfxColorMaxComps];
    double X, Y, Z;
    
    getXYZ(color,&X,&Y,&Z);
    in[0] = clip01(X);
    in[1] = clip01(Y);
    in[2] = clip01(Z);
    
    transform->doTransform(in,out,1);
    cmyk->c = byteToCol(out[0]);
    cmyk->m = byteToCol(out[1]);
    cmyk->y = byteToCol(out[2]);
    cmyk->k = byteToCol(out[3]);
    return;
  }
#endif
  getRGB(color, &rgb);
  c = clip01(gfxColorComp1 - rgb.r);
  m = clip01(gfxColorComp1 - rgb.g);
  y = clip01(gfxColorComp1 - rgb.b);
  k = c;
  if (m < k) {
    k = m;
  }
  if (y < k) {
    k = y;
  }
  cmyk->c = c - k;
  cmyk->m = m - k;
  cmyk->y = y - k;
  cmyk->k = k;
}

void GfxCalGrayColorSpace::getDeviceN(GfxColor *color, GfxColor *deviceN) {
  GfxCMYK cmyk;
  for (int i = 0; i < gfxColorMaxComps; i++)
    deviceN->c[i] = 0;
  getCMYK(color, &cmyk);
  deviceN->c[0] = cmyk.c;
  deviceN->c[1] = cmyk.m;
  deviceN->c[2] = cmyk.y;
  deviceN->c[3] = cmyk.k;
}

void GfxCalGrayColorSpace::getDefaultColor(GfxColor *color) {
  color->c[0] = 0;
}

//------------------------------------------------------------------------
// GfxDeviceRGBColorSpace
//------------------------------------------------------------------------

GfxDeviceRGBColorSpace::GfxDeviceRGBColorSpace() {
}

GfxDeviceRGBColorSpace::~GfxDeviceRGBColorSpace() {
}

GfxColorSpace *GfxDeviceRGBColorSpace::copy() {
  return new GfxDeviceRGBColorSpace();
}

void GfxDeviceRGBColorSpace::getGray(GfxColor *color, GfxGray *gray) {
  *gray = clip01((GfxColorComp)(0.3 * color->c[0] +
		 0.59 * color->c[1] +
		 0.11 * color->c[2] + 0.5));
}

void GfxDeviceRGBColorSpace::getGrayLine(Guchar *in, Guchar *out, int length) {
  int i;

  for (i = 0; i < length; i++) {
    out[i] = 
      (in[i * 3 + 0] * 19595 + 
       in[i * 3 + 1] * 38469 + 
       in[i * 3 + 2] * 7472) / 65536;
  }
}

void GfxDeviceRGBColorSpace::getRGB(GfxColor *color, GfxRGB *rgb) {
  rgb->r = clip01(color->c[0]);
  rgb->g = clip01(color->c[1]);
  rgb->b = clip01(color->c[2]);
}

void GfxDeviceRGBColorSpace::getRGBLine(Guchar *in, unsigned int *out,
					int length) {
  Guchar *p;
  int i;

  for (i = 0, p = in; i < length; i++, p += 3)
    out[i] = (p[0] << 16) | (p[1] << 8) | (p[2] << 0);
}

void GfxDeviceRGBColorSpace::getRGBLine(Guchar *in, Guchar *out, int length) {
  for (int i = 0; i < length; i++) {
    *out++ = *in++;
    *out++ = *in++;
    *out++ = *in++;
  }
}

void GfxDeviceRGBColorSpace::getRGBXLine(Guchar *in, Guchar *out, int length) {
  for (int i = 0; i < length; i++) {
    *out++ = *in++;
    *out++ = *in++;
    *out++ = *in++;
    *out++ = 255;
  }
}

void GfxDeviceRGBColorSpace::getCMYKLine(Guchar *in, Guchar *out, int length) {
  GfxColorComp c, m, y, k;

  for (int i = 0; i < length; i++) {
    c = byteToCol(255 - *in++);
    m = byteToCol(255 - *in++);
    y = byteToCol(255 - *in++);
    k = c;
    if (m < k) {
      k = m;
    }
    if (y < k) {
      k = y;
    }
    *out++ = colToByte(c - k);
    *out++ = colToByte(m - k);
    *out++ = colToByte(y - k);
    *out++ = colToByte(k);
  }
}

void GfxDeviceRGBColorSpace::getDeviceNLine(Guchar *in, Guchar *out, int length) {
  GfxColorComp c, m, y, k;

  for (int i = 0; i < length; i++) {
    for (int j = 0; j < SPOT_NCOMPS+4; j++) 
      out[j] = 0;
    c = byteToCol(255 - *in++);
    m = byteToCol(255 - *in++);
    y = byteToCol(255 - *in++);
    k = c;
    if (m < k) {
      k = m;
    }
    if (y < k) {
      k = y;
    }
    out[0] = colToByte(c - k);
    out[1] = colToByte(m - k);
    out[2] = colToByte(y - k);
    out[3] = colToByte(k);
    out += (SPOT_NCOMPS+4);
  }
}

void GfxDeviceRGBColorSpace::getCMYK(GfxColor *color, GfxCMYK *cmyk) {
  GfxColorComp c, m, y, k;

  c = clip01(gfxColorComp1 - color->c[0]);
  m = clip01(gfxColorComp1 - color->c[1]);
  y = clip01(gfxColorComp1 - color->c[2]);
  k = c;
  if (m < k) {
    k = m;
  }
  if (y < k) {
    k = y;
  }
  cmyk->c = c - k;
  cmyk->m = m - k;
  cmyk->y = y - k;
  cmyk->k = k;
}

void GfxDeviceRGBColorSpace::getDeviceN(GfxColor *color, GfxColor *deviceN) {
  GfxCMYK cmyk;
  for (int i = 0; i < gfxColorMaxComps; i++)
    deviceN->c[i] = 0;
  getCMYK(color, &cmyk);
  deviceN->c[0] = cmyk.c;
  deviceN->c[1] = cmyk.m;
  deviceN->c[2] = cmyk.y;
  deviceN->c[3] = cmyk.k;
}

void GfxDeviceRGBColorSpace::getDefaultColor(GfxColor *color) {
  color->c[0] = 0;
  color->c[1] = 0;
  color->c[2] = 0;
}

//------------------------------------------------------------------------
// GfxCalRGBColorSpace
//------------------------------------------------------------------------

GfxCalRGBColorSpace::GfxCalRGBColorSpace() {
  whiteX = whiteY = whiteZ = 1;
  blackX = blackY = blackZ = 0;
  gammaR = gammaG = gammaB = 1;
  mat[0] = 1; mat[1] = 0; mat[2] = 0;
  mat[3] = 0; mat[4] = 1; mat[5] = 0;
  mat[6] = 0; mat[7] = 0; mat[8] = 1;
}

GfxCalRGBColorSpace::~GfxCalRGBColorSpace() {
#ifdef USE_CMS
  if (transform != NULL) {
    if (transform->unref() == 0) delete transform;
  }
#endif
}

GfxColorSpace *GfxCalRGBColorSpace::copy() {
  GfxCalRGBColorSpace *cs;
  int i;

  cs = new GfxCalRGBColorSpace();
  cs->whiteX = whiteX;
  cs->whiteY = whiteY;
  cs->whiteZ = whiteZ;
  cs->blackX = blackX;
  cs->blackY = blackY;
  cs->blackZ = blackZ;
  cs->gammaR = gammaR;
  cs->gammaG = gammaG;
  cs->gammaB = gammaB;
  for (i = 0; i < 9; ++i) {
    cs->mat[i] = mat[i];
  }
#ifdef USE_CMS
  cs->transform = transform;
  if (transform != NULL) transform->ref();
#endif
  return cs;
}

GfxColorSpace *GfxCalRGBColorSpace::parse(Array *arr, GfxState *state) {
  GfxCalRGBColorSpace *cs;
  Object obj1, obj2, obj3;
  int i;

  arr->get(1, &obj1);
  if (!obj1.isDict()) {
    error(errSyntaxWarning, -1, "Bad CalRGB color space");
    obj1.free();
    return NULL;
  }
  cs = new GfxCalRGBColorSpace();
  if (obj1.dictLookup("WhitePoint", &obj2)->isArray() &&
      obj2.arrayGetLength() == 3) {
    obj2.arrayGet(0, &obj3);
    if (likely(obj3.isNum()))
      cs->whiteX = obj3.getNum();
    obj3.free();
    obj2.arrayGet(1, &obj3);
    if (likely(obj3.isNum()))
      cs->whiteY = obj3.getNum();
    obj3.free();
    obj2.arrayGet(2, &obj3);
    if (likely(obj3.isNum()))
      cs->whiteZ = obj3.getNum();
    obj3.free();
  }
  obj2.free();
  if (obj1.dictLookup("BlackPoint", &obj2)->isArray() &&
      obj2.arrayGetLength() == 3) {
    obj2.arrayGet(0, &obj3);
    if (likely(obj3.isNum()))
      cs->blackX = obj3.getNum();
    obj3.free();
    obj2.arrayGet(1, &obj3);
    if (likely(obj3.isNum()))
      cs->blackY = obj3.getNum();
    obj3.free();
    obj2.arrayGet(2, &obj3);
    if (likely(obj3.isNum()))
      cs->blackZ = obj3.getNum();
    obj3.free();
  }
  obj2.free();
  if (obj1.dictLookup("Gamma", &obj2)->isArray() &&
      obj2.arrayGetLength() == 3) {
    obj2.arrayGet(0, &obj3);
    if (likely(obj3.isNum()))
      cs->gammaR = obj3.getNum();
    obj3.free();
    obj2.arrayGet(1, &obj3);
    if (likely(obj3.isNum()))
      cs->gammaG = obj3.getNum();
    obj3.free();
    obj2.arrayGet(2, &obj3);
    if (likely(obj3.isNum()))
      cs->gammaB = obj3.getNum();
    obj3.free();
  }
  obj2.free();
  if (obj1.dictLookup("Matrix", &obj2)->isArray() &&
      obj2.arrayGetLength() == 9) {
    for (i = 0; i < 9; ++i) {
      obj2.arrayGet(i, &obj3);
      if (likely(obj3.isNum()))
        cs->mat[i] = obj3.getNum();
      obj3.free();
    }
  }
  obj2.free();
  obj1.free();

  cs->kr = 1 / (xyzrgb[0][0] * cs->whiteX +
		xyzrgb[0][1] * cs->whiteY +
		xyzrgb[0][2] * cs->whiteZ);
  cs->kg = 1 / (xyzrgb[1][0] * cs->whiteX +
		xyzrgb[1][1] * cs->whiteY +
		xyzrgb[1][2] * cs->whiteZ);
  cs->kb = 1 / (xyzrgb[2][0] * cs->whiteX +
		xyzrgb[2][1] * cs->whiteY +
		xyzrgb[2][2] * cs->whiteZ);

#ifdef USE_CMS
  cs->transform = (state != NULL) ? state->getXYZ2DisplayTransform() : XYZ2DisplayTransform;
  if (cs->transform != NULL) cs->transform->ref();
#endif
  return cs;
}

// convert CalRGB to XYZ color space
void GfxCalRGBColorSpace::getXYZ(GfxColor *color, 
  double *pX, double *pY, double *pZ) {
  double A, B, C;

  A = pow(colToDbl(color->c[0]), gammaR);
  B = pow(colToDbl(color->c[1]), gammaG);
  C = pow(colToDbl(color->c[2]), gammaB);
  *pX = mat[0] * A + mat[3] * B + mat[6] * C;
  *pY = mat[1] * A + mat[4] * B + mat[7] * C;
  *pZ = mat[2] * A + mat[5] * B + mat[8] * C;
}

void GfxCalRGBColorSpace::getGray(GfxColor *color, GfxGray *gray) {
  GfxRGB rgb;

#ifdef USE_CMS
  if (transform != NULL && transform->getTransformPixelType() == PT_GRAY) {
    Guchar out[gfxColorMaxComps];
    double in[gfxColorMaxComps];
    double X, Y, Z;
    
    getXYZ(color,&X,&Y,&Z);
    in[0] = clip01(X);
    in[1] = clip01(Y);
    in[2] = clip01(Z);
    transform->doTransform(in,out,1);
    *gray = byteToCol(out[0]);
    return;
  }
#endif
  getRGB(color, &rgb);
  *gray = clip01((GfxColorComp)(0.299 * rgb.r +
				0.587 * rgb.g +
				0.114 * rgb.b + 0.5));
}

void GfxCalRGBColorSpace::getRGB(GfxColor *color, GfxRGB *rgb) {
  double X, Y, Z;
  double r, g, b;

  getXYZ(color,&X,&Y,&Z);
#ifdef USE_CMS
  if (transform != NULL && transform->getTransformPixelType() == PT_RGB) {
    Guchar out[gfxColorMaxComps];
    double in[gfxColorMaxComps];
    
    in[0] = clip01(X/whiteX);
    in[1] = clip01(Y/whiteY);
    in[2] = clip01(Z/whiteZ);
    transform->doTransform(in,out,1);
    rgb->r = byteToCol(out[0]);
    rgb->g = byteToCol(out[1]);
    rgb->b = byteToCol(out[2]);
    return;
  }
#endif
  // convert XYZ to RGB, including gamut mapping and gamma correction
  r = xyzrgb[0][0] * X + xyzrgb[0][1] * Y + xyzrgb[0][2] * Z;
  g = xyzrgb[1][0] * X + xyzrgb[1][1] * Y + xyzrgb[1][2] * Z;
  b = xyzrgb[2][0] * X + xyzrgb[2][1] * Y + xyzrgb[2][2] * Z;
  rgb->r = dblToCol(sqrt(clip01(r)));
  rgb->g = dblToCol(sqrt(clip01(g)));
  rgb->b = dblToCol(sqrt(clip01(b)));
}

void GfxCalRGBColorSpace::getCMYK(GfxColor *color, GfxCMYK *cmyk) {
  GfxRGB rgb;
  GfxColorComp c, m, y, k;

#ifdef USE_CMS
  if (transform != NULL && transform->getTransformPixelType() == PT_CMYK) {
    double in[gfxColorMaxComps];
    Guchar out[gfxColorMaxComps];
    double X, Y, Z;
    
    getXYZ(color,&X,&Y,&Z);
    in[0] = clip01(X);
    in[1] = clip01(Y);
    in[2] = clip01(Z);
    transform->doTransform(in,out,1);
    cmyk->c = byteToCol(out[0]);
    cmyk->m = byteToCol(out[1]);
    cmyk->y = byteToCol(out[2]);
    cmyk->k = byteToCol(out[3]);
    return;
  }
#endif
  getRGB(color, &rgb);
  c = clip01(gfxColorComp1 - rgb.r);
  m = clip01(gfxColorComp1 - rgb.g);
  y = clip01(gfxColorComp1 - rgb.b);
  k = c;
  if (m < k) {
    k = m;
  }
  if (y < k) {
    k = y;
  }
  cmyk->c = c - k;
  cmyk->m = m - k;
  cmyk->y = y - k;
  cmyk->k = k;
}

void GfxCalRGBColorSpace::getDeviceN(GfxColor *color, GfxColor *deviceN) {
  GfxCMYK cmyk;
  for (int i = 0; i < gfxColorMaxComps; i++)
    deviceN->c[i] = 0;
  getCMYK(color, &cmyk);
  deviceN->c[0] = cmyk.c;
  deviceN->c[1] = cmyk.m;
  deviceN->c[2] = cmyk.y;
  deviceN->c[3] = cmyk.k;
}

void GfxCalRGBColorSpace::getDefaultColor(GfxColor *color) {
  color->c[0] = 0;
  color->c[1] = 0;
  color->c[2] = 0;
}

//------------------------------------------------------------------------
// GfxDeviceCMYKColorSpace
//------------------------------------------------------------------------

GfxDeviceCMYKColorSpace::GfxDeviceCMYKColorSpace() {
}

GfxDeviceCMYKColorSpace::~GfxDeviceCMYKColorSpace() {
}

GfxColorSpace *GfxDeviceCMYKColorSpace::copy() {
  return new GfxDeviceCMYKColorSpace();
}

void GfxDeviceCMYKColorSpace::getGray(GfxColor *color, GfxGray *gray) {
  *gray = clip01((GfxColorComp)(gfxColorComp1 - color->c[3]
				- 0.3  * color->c[0]
				- 0.59 * color->c[1]
				- 0.11 * color->c[2] + 0.5));
}

void GfxDeviceCMYKColorSpace::getRGB(GfxColor *color, GfxRGB *rgb) {
  double c, m, y, k, c1, m1, y1, k1, r, g, b;
    
  c = colToDbl(color->c[0]);
  m = colToDbl(color->c[1]);
  y = colToDbl(color->c[2]);
  k = colToDbl(color->c[3]);
  c1 = 1 - c;
  m1 = 1 - m;
  y1 = 1 - y;
  k1 = 1 - k;
  cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
  rgb->r = clip01(dblToCol(r));
  rgb->g = clip01(dblToCol(g));
  rgb->b = clip01(dblToCol(b));
}

static inline void GfxDeviceCMYKColorSpacegetRGBLineHelper(Guchar *&in, double &r, double &g, double &b)
{
  double c, m, y, k, c1, m1, y1, k1;
  
  c = byteToDbl(*in++);
  m = byteToDbl(*in++);
  y = byteToDbl(*in++);
  k = byteToDbl(*in++);
  c1 = 1 - c;
  m1 = 1 - m;
  y1 = 1 - y;
  k1 = 1 - k;
  cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
}

void GfxDeviceCMYKColorSpace::getRGBLine(Guchar *in, unsigned int *out, int length)
{
  double r, g, b;
  for (int i = 0; i < length; i++) {
    GfxDeviceCMYKColorSpacegetRGBLineHelper(in, r, g, b);
    *out++ = (dblToByte(clip01(r)) << 16) | (dblToByte(clip01(g)) << 8) | dblToByte(clip01(b));
  }
}

void GfxDeviceCMYKColorSpace::getRGBLine(Guchar *in, Guchar *out, int length)
{
  double r, g, b;
  
  for (int i = 0; i < length; i++) {
    GfxDeviceCMYKColorSpacegetRGBLineHelper(in, r, g, b);
    *out++ = dblToByte(clip01(r));
    *out++ = dblToByte(clip01(g));
    *out++ = dblToByte(clip01(b));
  }
}

void GfxDeviceCMYKColorSpace::getRGBXLine(Guchar *in, Guchar *out, int length)
{
  double r, g, b;
  
  for (int i = 0; i < length; i++) {
    GfxDeviceCMYKColorSpacegetRGBLineHelper(in, r, g, b);
    *out++ = dblToByte(clip01(r));
    *out++ = dblToByte(clip01(g));
    *out++ = dblToByte(clip01(b));
    *out++ = 255;
  }
}

void GfxDeviceCMYKColorSpace::getCMYKLine(Guchar *in, Guchar *out, int length)
{
  for (int i = 0; i < length; i++) {
    *out++ = *in++;
    *out++ = *in++;
    *out++ = *in++;
    *out++ = *in++;
  }
}

void GfxDeviceCMYKColorSpace::getDeviceNLine(Guchar *in, Guchar *out, int length)
{
  for (int i = 0; i < length; i++) {
    for (int j = 0; j < SPOT_NCOMPS+4; j++)
      out[j] = 0;
    out[0] = *in++;
    out[1] = *in++;
    out[2] = *in++;
    out[3] = *in++;
    out += (SPOT_NCOMPS+4);
  }
}

void GfxDeviceCMYKColorSpace::getCMYK(GfxColor *color, GfxCMYK *cmyk) {
  cmyk->c = clip01(color->c[0]);
  cmyk->m = clip01(color->c[1]);
  cmyk->y = clip01(color->c[2]);
  cmyk->k = clip01(color->c[3]);
}

void GfxDeviceCMYKColorSpace::getDeviceN(GfxColor *color, GfxColor *deviceN) {
  for (int i = 0; i < gfxColorMaxComps; i++)
    deviceN->c[i] = 0;
  deviceN->c[0] = clip01(color->c[0]);
  deviceN->c[1] = clip01(color->c[1]);
  deviceN->c[2] = clip01(color->c[2]);
  deviceN->c[3] = clip01(color->c[3]);
}

void GfxDeviceCMYKColorSpace::getDefaultColor(GfxColor *color) {
  color->c[0] = 0;
  color->c[1] = 0;
  color->c[2] = 0;
  color->c[3] = gfxColorComp1;
}

//------------------------------------------------------------------------
// GfxLabColorSpace
//------------------------------------------------------------------------

GfxLabColorSpace::GfxLabColorSpace() {
  whiteX = whiteY = whiteZ = 1;
  blackX = blackY = blackZ = 0;
  aMin = bMin = -100;
  aMax = bMax = 100;
}

GfxLabColorSpace::~GfxLabColorSpace() {
#ifdef USE_CMS
  if (transform != NULL) {
    if (transform->unref() == 0) delete transform;
  }
#endif
}

GfxColorSpace *GfxLabColorSpace::copy() {
  GfxLabColorSpace *cs;

  cs = new GfxLabColorSpace();
  cs->whiteX = whiteX;
  cs->whiteY = whiteY;
  cs->whiteZ = whiteZ;
  cs->blackX = blackX;
  cs->blackY = blackY;
  cs->blackZ = blackZ;
  cs->aMin = aMin;
  cs->aMax = aMax;
  cs->bMin = bMin;
  cs->bMax = bMax;
  cs->kr = kr;
  cs->kg = kg;
  cs->kb = kb;
#ifdef USE_CMS
  cs->transform = transform;
  if (transform != NULL) transform->ref();
#endif
  return cs;
}

GfxColorSpace *GfxLabColorSpace::parse(Array *arr, GfxState *state) {
  GfxLabColorSpace *cs;
  Object obj1, obj2, obj3;

  arr->get(1, &obj1);
  if (!obj1.isDict()) {
    error(errSyntaxWarning, -1, "Bad Lab color space");
    obj1.free();
    return NULL;
  }
  cs = new GfxLabColorSpace();
  if (obj1.dictLookup("WhitePoint", &obj2)->isArray() &&
      obj2.arrayGetLength() == 3) {
    obj2.arrayGet(0, &obj3);
    cs->whiteX = obj3.getNum();
    obj3.free();
    obj2.arrayGet(1, &obj3);
    cs->whiteY = obj3.getNum();
    obj3.free();
    obj2.arrayGet(2, &obj3);
    cs->whiteZ = obj3.getNum();
    obj3.free();
  }
  obj2.free();
  if (obj1.dictLookup("BlackPoint", &obj2)->isArray() &&
      obj2.arrayGetLength() == 3) {
    obj2.arrayGet(0, &obj3);
    cs->blackX = obj3.getNum();
    obj3.free();
    obj2.arrayGet(1, &obj3);
    cs->blackY = obj3.getNum();
    obj3.free();
    obj2.arrayGet(2, &obj3);
    cs->blackZ = obj3.getNum();
    obj3.free();
  }
  obj2.free();
  if (obj1.dictLookup("Range", &obj2)->isArray() &&
      obj2.arrayGetLength() == 4) {
    obj2.arrayGet(0, &obj3);
    cs->aMin = obj3.getNum();
    obj3.free();
    obj2.arrayGet(1, &obj3);
    cs->aMax = obj3.getNum();
    obj3.free();
    obj2.arrayGet(2, &obj3);
    cs->bMin = obj3.getNum();
    obj3.free();
    obj2.arrayGet(3, &obj3);
    cs->bMax = obj3.getNum();
    obj3.free();
  }
  obj2.free();
  obj1.free();

  cs->kr = 1 / (xyzrgb[0][0] * cs->whiteX +
		xyzrgb[0][1] * cs->whiteY +
		xyzrgb[0][2] * cs->whiteZ);
  cs->kg = 1 / (xyzrgb[1][0] * cs->whiteX +
		xyzrgb[1][1] * cs->whiteY +
		xyzrgb[1][2] * cs->whiteZ);
  cs->kb = 1 / (xyzrgb[2][0] * cs->whiteX +
		xyzrgb[2][1] * cs->whiteY +
		xyzrgb[2][2] * cs->whiteZ);

#ifdef USE_CMS
  cs->transform = (state != NULL) ? state->getXYZ2DisplayTransform() : XYZ2DisplayTransform;
  if (cs->transform != NULL) cs->transform->ref();
#endif
  return cs;
}

void GfxLabColorSpace::getGray(GfxColor *color, GfxGray *gray) {
  GfxRGB rgb;

#ifdef USE_CMS
  if (transform != NULL && transform->getTransformPixelType() == PT_GRAY) {
    Guchar out[gfxColorMaxComps];
    double in[gfxColorMaxComps];
    
    getXYZ(color, &in[0], &in[1], &in[2]);
    transform->doTransform(in,out,1);
    *gray = byteToCol(out[0]);
    return;
  }
#endif
  getRGB(color, &rgb);
  *gray = clip01((GfxColorComp)(0.299 * rgb.r +
				0.587 * rgb.g +
				0.114 * rgb.b + 0.5));
}

// convert L*a*b* to media XYZ color space
// (not multiply by the white point)
void GfxLabColorSpace::getXYZ(GfxColor *color, 
  double *pX, double *pY, double *pZ) {
  double X, Y, Z;
  double t1, t2;

  t1 = (colToDbl(color->c[0]) + 16) / 116;
  t2 = t1 + colToDbl(color->c[1]) / 500;
  if (t2 >= (6.0 / 29.0)) {
    X = t2 * t2 * t2;
  } else {
    X = (108.0 / 841.0) * (t2 - (4.0 / 29.0));
  }
  if (t1 >= (6.0 / 29.0)) {
    Y = t1 * t1 * t1;
  } else {
    Y = (108.0 / 841.0) * (t1 - (4.0 / 29.0));
  }
  t2 = t1 - colToDbl(color->c[2]) / 200;
  if (t2 >= (6.0 / 29.0)) {
    Z = t2 * t2 * t2;
  } else {
    Z = (108.0 / 841.0) * (t2 - (4.0 / 29.0));
  }
  *pX = X;
  *pY = Y;
  *pZ = Z;
}

void GfxLabColorSpace::getRGB(GfxColor *color, GfxRGB *rgb) {
  double X, Y, Z;
  double r, g, b;

  getXYZ(color, &X, &Y, &Z);
#ifdef USE_CMS
  if (transform != NULL && transform->getTransformPixelType() == PT_RGB) {
    Guchar out[gfxColorMaxComps];
    double in[gfxColorMaxComps];
    
    in[0] = clip01(X);
    in[1] = clip01(Y);
    in[2] = clip01(Z);
    transform->doTransform(in,out,1);
    rgb->r = byteToCol(out[0]);
    rgb->g = byteToCol(out[1]);
    rgb->b = byteToCol(out[2]);
    return;
  } else if (transform != NULL && transform->getTransformPixelType() == PT_CMYK) {
    Guchar out[gfxColorMaxComps];
    double in[gfxColorMaxComps];
    double c, m, y, k, c1, m1, y1, k1, r, g, b;

    in[0] = clip01(X);
    in[1] = clip01(Y);
    in[2] = clip01(Z);
    transform->doTransform(in,out,1);
    c = byteToDbl(out[0]);
    m = byteToDbl(out[1]);
    y = byteToDbl(out[2]);
    k = byteToDbl(out[3]);
    c1 = 1 - c;
    m1 = 1 - m;
    y1 = 1 - y;
    k1 = 1 - k;
    cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
    rgb->r = clip01(dblToCol(r));
    rgb->g = clip01(dblToCol(g));
    rgb->b = clip01(dblToCol(b));
    return;
  }
#endif
  X *= whiteX;
  Y *= whiteY;
  Z *= whiteZ;
  // convert XYZ to RGB, including gamut mapping and gamma correction
  r = xyzrgb[0][0] * X + xyzrgb[0][1] * Y + xyzrgb[0][2] * Z;
  g = xyzrgb[1][0] * X + xyzrgb[1][1] * Y + xyzrgb[1][2] * Z;
  b = xyzrgb[2][0] * X + xyzrgb[2][1] * Y + xyzrgb[2][2] * Z;
  rgb->r = dblToCol(sqrt(clip01(r * kr)));
  rgb->g = dblToCol(sqrt(clip01(g * kg)));
  rgb->b = dblToCol(sqrt(clip01(b * kb)));
}

void GfxLabColorSpace::getCMYK(GfxColor *color, GfxCMYK *cmyk) {
  GfxRGB rgb;
  GfxColorComp c, m, y, k;

#ifdef USE_CMS
  if (transform != NULL && transform->getTransformPixelType() == PT_CMYK) {
    double in[gfxColorMaxComps];
    Guchar out[gfxColorMaxComps];
    
    getXYZ(color, &in[0], &in[1], &in[2]);
    transform->doTransform(in,out,1);
    cmyk->c = byteToCol(out[0]);
    cmyk->m = byteToCol(out[1]);
    cmyk->y = byteToCol(out[2]);
    cmyk->k = byteToCol(out[3]);
    return;
  }
#endif
  getRGB(color, &rgb);
  c = clip01(gfxColorComp1 - rgb.r);
  m = clip01(gfxColorComp1 - rgb.g);
  y = clip01(gfxColorComp1 - rgb.b);
  k = c;
  if (m < k) {
    k = m;
  }
  if (y < k) {
    k = y;
  }
  cmyk->c = c - k;
  cmyk->m = m - k;
  cmyk->y = y - k;
  cmyk->k = k;
}

void GfxLabColorSpace::getDeviceN(GfxColor *color, GfxColor *deviceN) {
  GfxCMYK cmyk;
  for (int i = 0; i < gfxColorMaxComps; i++)
    deviceN->c[i] = 0;
  getCMYK(color, &cmyk);
  deviceN->c[0] = cmyk.c;
  deviceN->c[1] = cmyk.m;
  deviceN->c[2] = cmyk.y;
  deviceN->c[3] = cmyk.k;
}

void GfxLabColorSpace::getDefaultColor(GfxColor *color) {
  color->c[0] = 0;
  if (aMin > 0) {
    color->c[1] = dblToCol(aMin);
  } else if (aMax < 0) {
    color->c[1] = dblToCol(aMax);
  } else {
    color->c[1] = 0;
  }
  if (bMin > 0) {
    color->c[2] = dblToCol(bMin);
  } else if (bMax < 0) {
    color->c[2] = dblToCol(bMax);
  } else {
    color->c[2] = 0;
  }
}

void GfxLabColorSpace::getDefaultRanges(double *decodeLow, double *decodeRange,
					int maxImgPixel) {
  decodeLow[0] = 0;
  decodeRange[0] = 100;
  decodeLow[1] = aMin;
  decodeRange[1] = aMax - aMin;
  decodeLow[2] = bMin;
  decodeRange[2] = bMax - bMin;
}

//------------------------------------------------------------------------
// GfxICCBasedColorSpace
//------------------------------------------------------------------------

class GfxICCBasedColorSpaceKey : public PopplerCacheKey
{
  public:
    GfxICCBasedColorSpaceKey(int numA, int genA) : num(numA), gen(genA)
    {
    }
    
    bool operator==(const PopplerCacheKey &key) const
    {
      const GfxICCBasedColorSpaceKey *k = static_cast<const GfxICCBasedColorSpaceKey*>(&key);
      return k->num == num && k->gen == gen;
    }
    
    int num, gen;
};

class GfxICCBasedColorSpaceItem : public PopplerCacheItem
{
  public:
    GfxICCBasedColorSpaceItem(GfxICCBasedColorSpace *csA)
    {
      cs = static_cast<GfxICCBasedColorSpace*>(csA->copy());
    }
    
    ~GfxICCBasedColorSpaceItem()
    {
      delete cs;
    }
    
    GfxICCBasedColorSpace *cs;
};

GfxICCBasedColorSpace::GfxICCBasedColorSpace(int nCompsA, GfxColorSpace *altA,
					     Ref *iccProfileStreamA) {
  nComps = nCompsA;
  alt = altA;
  iccProfileStream = *iccProfileStreamA;
  rangeMin[0] = rangeMin[1] = rangeMin[2] = rangeMin[3] = 0;
  rangeMax[0] = rangeMax[1] = rangeMax[2] = rangeMax[3] = 1;
#ifdef USE_CMS
  transform = NULL;
  lineTransform = NULL;
#endif
}

GfxICCBasedColorSpace::~GfxICCBasedColorSpace() {
  delete alt;
#ifdef USE_CMS
  if (transform != NULL) {
    if (transform->unref() == 0) delete transform;
  }
  if (lineTransform != NULL) {
    if (lineTransform->unref() == 0) delete lineTransform;
  }
#endif
}

GfxColorSpace *GfxICCBasedColorSpace::copy() {
  GfxICCBasedColorSpace *cs;
  int i;

  cs = new GfxICCBasedColorSpace(nComps, alt->copy(), &iccProfileStream);
  for (i = 0; i < 4; ++i) {
    cs->rangeMin[i] = rangeMin[i];
    cs->rangeMax[i] = rangeMax[i];
  }
#ifdef USE_CMS
  cs->transform = transform;
  if (transform != NULL) transform->ref();
  cs->lineTransform = lineTransform;
  if (lineTransform != NULL) lineTransform->ref();
#endif
  return cs;
}

GfxColorSpace *GfxICCBasedColorSpace::parse(Array *arr, OutputDev *out, GfxState *state, int recursion) {
  GfxICCBasedColorSpace *cs;
  Ref iccProfileStreamA;
  int nCompsA;
  GfxColorSpace *altA;
  Dict *dict;
  Object obj1, obj2, obj3;
  int i;

  if (arr->getLength() < 2) {
    error(errSyntaxError, -1, "Bad ICCBased color space");
    return NULL;
  }
  arr->getNF(1, &obj1);
  if (obj1.isRef()) {
    iccProfileStreamA = obj1.getRef();
  } else {
    iccProfileStreamA.num = 0;
    iccProfileStreamA.gen = 0;
  }
  obj1.free();
#ifdef USE_CMS
  // check cache
  if (out && iccProfileStreamA.num > 0) {
    GfxICCBasedColorSpaceKey k(iccProfileStreamA.num, iccProfileStreamA.gen);
    GfxICCBasedColorSpaceItem *item = static_cast<GfxICCBasedColorSpaceItem *>(out->getIccColorSpaceCache()->lookup(k));
    if (item != NULL)
    {
      cs = static_cast<GfxICCBasedColorSpace*>(item->cs->copy());
      int transformIntent = cs->getIntent();
      int cmsIntent = INTENT_RELATIVE_COLORIMETRIC;
      if (state != NULL) {
        const char *intent = state->getRenderingIntent();
        if (intent != NULL) {
          if (strcmp(intent, "AbsoluteColorimetric") == 0) {
            cmsIntent = INTENT_ABSOLUTE_COLORIMETRIC;
          } else if (strcmp(intent, "Saturation") == 0) {
            cmsIntent = INTENT_SATURATION;
          } else if (strcmp(intent, "Perceptual") == 0) {
            cmsIntent = INTENT_PERCEPTUAL;
          }
        }
      }
      if (transformIntent == cmsIntent) {
        return cs;
      }
      delete cs;
    }
  }
#endif
  arr->get(1, &obj1);
  if (!obj1.isStream()) {
    error(errSyntaxWarning, -1, "Bad ICCBased color space (stream)");
    obj1.free();
    return NULL;
  }
  dict = obj1.streamGetDict();
  if (!dict->lookup("N", &obj2)->isInt()) {
    error(errSyntaxWarning, -1, "Bad ICCBased color space (N)");
    obj2.free();
    obj1.free();
    return NULL;
  }
  nCompsA = obj2.getInt();
  obj2.free();
  if (nCompsA > 4) {
    error(errSyntaxError, -1,
	  "ICCBased color space with too many ({0:d} > 4) components",
	  nCompsA);
    nCompsA = 4;
  }
  if (dict->lookup("Alternate", &obj2)->isNull() ||
      !(altA = GfxColorSpace::parse(&obj2, out, state, recursion + 1))) {
    switch (nCompsA) {
    case 1:
      altA = new GfxDeviceGrayColorSpace();
      break;
    case 3:
      altA = new GfxDeviceRGBColorSpace();
      break;
    case 4:
      altA = new GfxDeviceCMYKColorSpace();
      break;
    default:
      error(errSyntaxWarning, -1, "Bad ICCBased color space - invalid N");
      obj2.free();
      obj1.free();
      return NULL;
    }
  }
  obj2.free();
  cs = new GfxICCBasedColorSpace(nCompsA, altA, &iccProfileStreamA);
  if (dict->lookup("Range", &obj2)->isArray() &&
      obj2.arrayGetLength() == 2 * nCompsA) {
    Object obj4;
    for (i = 0; i < nCompsA; ++i) {
      obj2.arrayGet(2*i, &obj3);
      obj2.arrayGet(2*i+1, &obj4);
      if (obj3.isNum() && obj4.isNum()) {
        cs->rangeMin[i] = obj3.getNum();
        cs->rangeMax[i] = obj4.getNum();
      }
      obj3.free();
      obj4.free();
    }
  }
  obj2.free();
  obj1.free();

#ifdef USE_CMS
  arr->get(1, &obj1);
  dict = obj1.streamGetDict();
  Guchar *profBuf;
  Stream *iccStream = obj1.getStream();
  int length = 0;

  profBuf = iccStream->toUnsignedChars(&length, 65536, 65536);
  cmsHPROFILE hp = cmsOpenProfileFromMem(profBuf,length);
  gfree(profBuf);
  if (hp == 0) {
    error(errSyntaxWarning, -1, "read ICCBased color space profile error");
  } else {
    cmsHPROFILE dhp = (state != NULL && state->getDisplayProfile() != NULL) ? state->getDisplayProfile() : displayProfile;
    if (dhp == NULL) dhp = RGBProfile;
    unsigned int cst = getCMSColorSpaceType(cmsGetColorSpace(hp));
    unsigned int dNChannels = getCMSNChannels(cmsGetColorSpace(dhp));
    unsigned int dcst = getCMSColorSpaceType(cmsGetColorSpace(dhp));
    cmsHTRANSFORM transform;

    int cmsIntent = INTENT_RELATIVE_COLORIMETRIC;
    if (state != NULL) {
      const char *intent = state->getRenderingIntent();
      if (intent != NULL) {
        if (strcmp(intent, "AbsoluteColorimetric") == 0) {
          cmsIntent = INTENT_ABSOLUTE_COLORIMETRIC;
        } else if (strcmp(intent, "Saturation") == 0) {
          cmsIntent = INTENT_SATURATION;
        } else if (strcmp(intent, "Perceptual") == 0) {
          cmsIntent = INTENT_PERCEPTUAL;
        }
      }
    }
    if ((transform = cmsCreateTransform(hp,
	   COLORSPACE_SH(cst) |CHANNELS_SH(nCompsA) | BYTES_SH(1),
	   dhp,
	   COLORSPACE_SH(dcst) |
	     CHANNELS_SH(dNChannels) | BYTES_SH(1),
	   cmsIntent, LCMS_FLAGS)) == 0) {
      error(errSyntaxWarning, -1, "Can't create transform");
      cs->transform = NULL;
    } else {
      cs->transform = new GfxColorTransform(transform, cmsIntent, dcst);
    }
    if (dcst == PT_RGB || dcst == PT_CMYK) {
       // create line transform only when the display is RGB type color space 
      if ((transform = cmsCreateTransform(hp,
	    CHANNELS_SH(nCompsA) | BYTES_SH(1),dhp,
	    (dcst == PT_RGB) ? TYPE_RGB_8 : TYPE_CMYK_8, cmsIntent, LCMS_FLAGS)) == 0) {
	error(errSyntaxWarning, -1, "Can't create transform");
	cs->lineTransform = NULL;
      } else {
	cs->lineTransform = new GfxColorTransform(transform, cmsIntent, dcst);
      }
    }
    cmsCloseProfile(hp);
  }
  obj1.free();
  // put this colorSpace into cache
  if (out && iccProfileStreamA.num > 0) {
    GfxICCBasedColorSpaceKey *k = new GfxICCBasedColorSpaceKey(iccProfileStreamA.num, iccProfileStreamA.gen);
    GfxICCBasedColorSpaceItem *item = new GfxICCBasedColorSpaceItem(cs);
    out->getIccColorSpaceCache()->put(k, item);
  }
#endif
  return cs;
}

void GfxICCBasedColorSpace::getGray(GfxColor *color, GfxGray *gray) {
#ifdef USE_CMS
  if (transform != 0 && transform->getTransformPixelType() == PT_GRAY) {
    Guchar in[gfxColorMaxComps];
    Guchar out[gfxColorMaxComps];
    
    for (int i = 0;i < nComps;i++) {
	in[i] = colToByte(color->c[i]);
    }
    if (nComps <= 4) {
      unsigned int key = 0;
      for (int j = 0; j < nComps; j++) {
        key = (key << 8) + in[j];
      }
      std::map<unsigned int, unsigned int>::iterator it = cmsCache.find(key);
      if (it != cmsCache.end()) {
        unsigned int value = it->second;
        *gray = byteToCol(value & 0xff);
        return;
      }
    }    
    transform->doTransform(in,out,1);
    *gray = byteToCol(out[0]);
    if (nComps <= 4 && cmsCache.size() <= CMSCACHE_LIMIT) {
      unsigned int key = 0;
      for (int j = 0; j < nComps; j++) {
        key = (key << 8) + in[j];
      }
      unsigned int value = out[0];
      cmsCache.insert(std::pair<unsigned int, unsigned int>(key, value));
    }
  } else {
    GfxRGB rgb;
    getRGB(color,&rgb);
    *gray = clip01((GfxColorComp)(0.3 * rgb.r +
		   0.59 * rgb.g +
		   0.11 * rgb.b + 0.5));
  }
#else
  alt->getGray(color, gray);
#endif
}

void GfxICCBasedColorSpace::getRGB(GfxColor *color, GfxRGB *rgb) {
#ifdef USE_CMS
  if (transform != 0 && transform->getTransformPixelType() == PT_RGB) {
    Guchar in[gfxColorMaxComps];
    Guchar out[gfxColorMaxComps];
    
    for (int i = 0;i < nComps;i++) {
	in[i] = colToByte(color->c[i]);
    }
    if (nComps <= 4) {
      unsigned int key = 0;
      for (int j = 0; j < nComps; j++) {
        key = (key << 8) + in[j];
      }
      std::map<unsigned int, unsigned int>::iterator it = cmsCache.find(key);
      if (it != cmsCache.end()) {
        unsigned int value = it->second;
        rgb->r = byteToCol(value >> 16);
        rgb->g = byteToCol((value >> 8) & 0xff);
        rgb->b = byteToCol(value & 0xff);
        return;
      }
    }    
    transform->doTransform(in,out,1);
    rgb->r = byteToCol(out[0]);
    rgb->g = byteToCol(out[1]);
    rgb->b = byteToCol(out[2]);
    if (nComps <= 4 && cmsCache.size() <= CMSCACHE_LIMIT) {
      unsigned int key = 0;
      for (int j = 0; j < nComps; j++) {
        key = (key << 8) + in[j];
      }
      unsigned int value = (out[0] << 16) + (out[1] << 8) + out[2];
      cmsCache.insert(std::pair<unsigned int, unsigned int>(key, value));
    }
  } else if (transform != NULL && transform->getTransformPixelType() == PT_CMYK) {
    Guchar in[gfxColorMaxComps];
    Guchar out[gfxColorMaxComps];
    double c, m, y, k, c1, m1, y1, k1, r, g, b;

    for (int i = 0;i < nComps;i++) {
	in[i] = colToByte(color->c[i]);
    }
    if (nComps <= 4) {
      unsigned int key = 0;
      for (int j = 0; j < nComps; j++) {
        key = (key << 8) + in[j];
      }
      std::map<unsigned int, unsigned int>::iterator it = cmsCache.find(key);
      if (it != cmsCache.end()) {
        unsigned int value = it->second;
        rgb->r = byteToCol(value >> 16);
        rgb->g = byteToCol((value >> 8) & 0xff);
        rgb->b = byteToCol(value & 0xff);
        return;
      }
    }    
    transform->doTransform(in,out,1);
    c = byteToDbl(out[0]);
    m = byteToDbl(out[1]);
    y = byteToDbl(out[2]);
    k = byteToDbl(out[3]);
    c1 = 1 - c;
    m1 = 1 - m;
    y1 = 1 - y;
    k1 = 1 - k;
    cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
    rgb->r = clip01(dblToCol(r));
    rgb->g = clip01(dblToCol(g));
    rgb->b = clip01(dblToCol(b));
    if (nComps <= 4 && cmsCache.size() <= CMSCACHE_LIMIT) {
      unsigned int key = 0;
      for (int j = 0; j < nComps; j++) {
        key = (key << 8) + in[j];
      }
      unsigned int value = (dblToByte(r) << 16) + (dblToByte(g) << 8) + dblToByte(b);
      cmsCache.insert(std::pair<unsigned int, unsigned int>(key, value));
    }
  } else {
    alt->getRGB(color, rgb);
  }
#else
  alt->getRGB(color, rgb);
#endif
}

void GfxICCBasedColorSpace::getRGBLine(Guchar *in, unsigned int *out,
				       int length) {
#ifdef USE_CMS
  if (lineTransform != 0 && lineTransform->getTransformPixelType() == PT_RGB) {
    Guchar* tmp = (Guchar *)gmallocn(3 * length, sizeof(Guchar));
    lineTransform->doTransform(in, tmp, length);
    for (int i = 0; i < length; ++i) {
        Guchar *current = tmp + (i * 3);
        out[i] = (current[0] << 16) | (current[1] << 8) | current[2];
    }
    gfree(tmp);
  } else {
    alt->getRGBLine(in, out, length);
  }
#else
  alt->getRGBLine(in, out, length);
#endif
}

void GfxICCBasedColorSpace::getRGBLine(Guchar *in, Guchar *out, int length) {
#ifdef USE_CMS
  if (lineTransform != 0 && lineTransform->getTransformPixelType() == PT_RGB) {
    Guchar* tmp = (Guchar *)gmallocn(3 * length, sizeof(Guchar));
    lineTransform->doTransform(in, tmp, length);
    Guchar *current = tmp;
    for (int i = 0; i < length; ++i) {
        *out++ = *current++;
        *out++ = *current++;
        *out++ = *current++;
    }
    gfree(tmp);
  } else if (lineTransform != NULL && lineTransform->getTransformPixelType() == PT_CMYK) {
    Guchar* tmp = (Guchar *)gmallocn(4 * length, sizeof(Guchar));
    lineTransform->doTransform(in, tmp, length);
    Guchar *current = tmp;
    double c, m, y, k, c1, m1, y1, k1, r, g, b;
    for (int i = 0; i < length; ++i) {
      c = byteToDbl(*current++);
      m = byteToDbl(*current++);
      y = byteToDbl(*current++);
      k = byteToDbl(*current++);
      c1 = 1 - c;
      m1 = 1 - m;
      y1 = 1 - y;
      k1 = 1 - k;
      cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
      *out++ = dblToByte(r);
      *out++ = dblToByte(g);
      *out++ = dblToByte(b);
    }
    gfree(tmp);
  } else {
    alt->getRGBLine(in, out, length);
  }
#else
  alt->getRGBLine(in, out, length);
#endif
}

void GfxICCBasedColorSpace::getRGBXLine(Guchar *in, Guchar *out, int length) {
#ifdef USE_CMS
  if (lineTransform != 0 && lineTransform->getTransformPixelType() == PT_RGB) {
    Guchar* tmp = (Guchar *)gmallocn(3 * length, sizeof(Guchar));
    lineTransform->doTransform(in, tmp, length);
    Guchar *current = tmp;
    for (int i = 0; i < length; ++i) {
        *out++ = *current++;
        *out++ = *current++;
        *out++ = *current++;
        *out++ = 255;
    }
    gfree(tmp);
  } else {
    alt->getRGBXLine(in, out, length);
  }
#else
  alt->getRGBXLine(in, out, length);
#endif
}

void GfxICCBasedColorSpace::getCMYKLine(Guchar *in, Guchar *out, int length) {
#ifdef USE_CMS
  if (lineTransform != NULL && lineTransform->getTransformPixelType() == PT_CMYK) {
    transform->doTransform(in,out,length);
  } else if (lineTransform != NULL) {
    GfxColorComp c, m, y, k;
    Guchar* tmp = (Guchar *)gmallocn(3 * length, sizeof(Guchar));
    getRGBLine(in, tmp, length);
    Guchar *p = tmp;
    for (int i = 0; i < length; i++) {
      c = byteToCol(255 - *p++);
      m = byteToCol(255 - *p++);
      y = byteToCol(255 - *p++);
      k = c;
      if (m < k) {
        k = m;
      }
      if (y < k) {
        k = y;
      }
      *out++ = colToByte(c - k);
      *out++ = colToByte(m - k);
      *out++ = colToByte(y - k);
      *out++ = colToByte(k);
    }
    gfree(tmp);
  } else {
    alt->getCMYKLine(in, out, length);
  }
#else
  alt->getCMYKLine(in, out, length);
#endif
}

void GfxICCBasedColorSpace::getDeviceNLine(Guchar *in, Guchar *out, int length) {
#ifdef USE_CMS
  if (lineTransform != NULL && lineTransform->getTransformPixelType() == PT_CMYK) {
    Guchar* tmp = (Guchar *)gmallocn(4 * length, sizeof(Guchar));
    transform->doTransform(in,tmp,length);
    Guchar *p = tmp;
    for (int i = 0; i < length; i++) {
      for (int j = 0; j < 4; j++)
        *out++ = *p++;
      for (int j = 4; j < SPOT_NCOMPS+4; j++)
        *out++ = 0;
    }
    gfree(tmp);
  } else if (lineTransform != NULL) {
    GfxColorComp c, m, y, k;
    Guchar* tmp = (Guchar *)gmallocn(3 * length, sizeof(Guchar));
    getRGBLine(in, tmp, length);
    Guchar *p = tmp;
    for (int i = 0; i < length; i++) {
      for (int j = 0; j < SPOT_NCOMPS+4; j++) 
        out[j] = 0;
      c = byteToCol(255 - *p++);
      m = byteToCol(255 - *p++);
      y = byteToCol(255 - *p++);
      k = c;
      if (m < k) {
        k = m;
      }
      if (y < k) {
        k = y;
      }
      out[0] = colToByte(c - k);
      out[1] = colToByte(m - k);
      out[2] = colToByte(y - k);
      out[3] = colToByte(k);
      out += (SPOT_NCOMPS+4);
    }
    gfree(tmp);
  } else {
    alt->getDeviceNLine(in, out, length);
  }
#else
  alt->getDeviceNLine(in, out, length);
#endif
}

void GfxICCBasedColorSpace::getCMYK(GfxColor *color, GfxCMYK *cmyk) {
#ifdef USE_CMS
  if (transform != NULL && transform->getTransformPixelType() == PT_CMYK) {
    Guchar in[gfxColorMaxComps];
    Guchar out[gfxColorMaxComps];
    
    for (int i = 0;i < nComps;i++) {
	in[i] = colToByte(color->c[i]);
    }
    if (nComps <= 4) {
      unsigned int key = 0;
      for (int j = 0; j < nComps; j++) {
        key = (key << 8) + in[j];
      }
      std::map<unsigned int, unsigned int>::iterator it = cmsCache.find(key);
      if (it != cmsCache.end()) {
        unsigned int value = it->second;
        cmyk->c = byteToCol(value >> 24);
        cmyk->m = byteToCol((value >> 16) & 0xff);
        cmyk->y = byteToCol((value >> 8) & 0xff);
        cmyk->k = byteToCol(value & 0xff);
        return;
      }
    }    
    transform->doTransform(in,out,1);
    cmyk->c = byteToCol(out[0]);
    cmyk->m = byteToCol(out[1]);
    cmyk->y = byteToCol(out[2]);
    cmyk->k = byteToCol(out[3]);
    if (nComps <= 4 && cmsCache.size() <= CMSCACHE_LIMIT) {
      unsigned int key = 0;
      for (int j = 0; j < nComps; j++) {
        key = (key << 8) + in[j];
      }
      unsigned int value = (out[0] << 24) + (out[1] << 16) + (out[2] << 8) + out[3];
      cmsCache.insert(std::pair<unsigned int, unsigned int>(key, value));
    }
  } else {
    GfxRGB rgb;
    GfxColorComp c, m, y, k;

    getRGB(color,&rgb);
    c = clip01(gfxColorComp1 - rgb.r);
    m = clip01(gfxColorComp1 - rgb.g);
    y = clip01(gfxColorComp1 - rgb.b);
    k = c;
    if (m < k) {
      k = m;
    }
    if (y < k) {
      k = y;
    }
    cmyk->c = c - k;
    cmyk->m = m - k;
    cmyk->y = y - k;
    cmyk->k = k;
  }
#else
  alt->getCMYK(color, cmyk);
#endif
}

GBool GfxICCBasedColorSpace::useGetRGBLine() {
#ifdef USE_CMS
  return lineTransform != NULL || alt->useGetRGBLine();
#else
  return alt->useGetRGBLine();
#endif
}

GBool GfxICCBasedColorSpace::useGetCMYKLine() {
#ifdef USE_CMS
  return lineTransform != NULL || alt->useGetCMYKLine();
#else
  return alt->useGetCMYKLine();
#endif
}

GBool GfxICCBasedColorSpace::useGetDeviceNLine() {
#ifdef USE_CMS
  return lineTransform != NULL || alt->useGetDeviceNLine();
#else
  return alt->useGetDeviceNLine();
#endif
}

void GfxICCBasedColorSpace::getDeviceN(GfxColor *color, GfxColor *deviceN) {
  GfxCMYK cmyk;
  for (int i = 0; i < gfxColorMaxComps; i++)
    deviceN->c[i] = 0;
  getCMYK(color, &cmyk);
  deviceN->c[0] = cmyk.c;
  deviceN->c[1] = cmyk.m;
  deviceN->c[2] = cmyk.y;
  deviceN->c[3] = cmyk.k;
}

void GfxICCBasedColorSpace::getDefaultColor(GfxColor *color) {
  int i;

  for (i = 0; i < nComps; ++i) {
    if (rangeMin[i] > 0) {
      color->c[i] = dblToCol(rangeMin[i]);
    } else if (rangeMax[i] < 0) {
      color->c[i] = dblToCol(rangeMax[i]);
    } else {
      color->c[i] = 0;
    }
  }
}

void GfxICCBasedColorSpace::getDefaultRanges(double *decodeLow,
					     double *decodeRange,
					     int maxImgPixel) {
  alt->getDefaultRanges(decodeLow, decodeRange, maxImgPixel);

#if 0
  // this is nominally correct, but some PDF files don't set the
  // correct ranges in the ICCBased dict
  int i;

  for (i = 0; i < nComps; ++i) {
    decodeLow[i] = rangeMin[i];
    decodeRange[i] = rangeMax[i] - rangeMin[i];
  }
#endif
}

//------------------------------------------------------------------------
// GfxIndexedColorSpace
//------------------------------------------------------------------------

GfxIndexedColorSpace::GfxIndexedColorSpace(GfxColorSpace *baseA,
					   int indexHighA) {
  base = baseA;
  indexHigh = indexHighA;
  lookup = (Guchar *)gmallocn((indexHigh + 1) * base->getNComps(),
			      sizeof(Guchar));
  overprintMask = base->getOverprintMask();
}

GfxIndexedColorSpace::~GfxIndexedColorSpace() {
  delete base;
  gfree(lookup);
}

GfxColorSpace *GfxIndexedColorSpace::copy() {
  GfxIndexedColorSpace *cs;

  cs = new GfxIndexedColorSpace(base->copy(), indexHigh);
  memcpy(cs->lookup, lookup,
	 (indexHigh + 1) * base->getNComps() * sizeof(Guchar));
  return cs;
}

GfxColorSpace *GfxIndexedColorSpace::parse(Array *arr, OutputDev *out, GfxState *state, int recursion) {
  GfxIndexedColorSpace *cs;
  GfxColorSpace *baseA;
  int indexHighA;
  Object obj1;
  char *s;
  int n, i, j;

  if (arr->getLength() != 4) {
    error(errSyntaxWarning, -1, "Bad Indexed color space");
    goto err1;
  }
  arr->get(1, &obj1);
  if (!(baseA = GfxColorSpace::parse(&obj1, out, state, recursion + 1))) {
    error(errSyntaxWarning, -1, "Bad Indexed color space (base color space)");
    goto err2;
  }
  obj1.free();
  if (!arr->get(2, &obj1)->isInt()) {
    error(errSyntaxWarning, -1, "Bad Indexed color space (hival)");
    delete baseA;
    goto err2;
  }
  indexHighA = obj1.getInt();
  if (indexHighA < 0 || indexHighA > 255) {
    // the PDF spec requires indexHigh to be in [0,255] -- allowing
    // values larger than 255 creates a security hole: if nComps *
    // indexHigh is greater than 2^31, the loop below may overwrite
    // past the end of the array
    int previousValue = indexHighA;
    if (indexHighA < 0) indexHighA = 0;
    else indexHighA = 255;
    error(errSyntaxWarning, -1, "Bad Indexed color space (invalid indexHigh value, was {0:d} using {1:d} to try to recover)", previousValue, indexHighA);
  }
  obj1.free();
  cs = new GfxIndexedColorSpace(baseA, indexHighA);
  arr->get(3, &obj1);
  n = baseA->getNComps();
  if (obj1.isStream()) {
    obj1.streamReset();
    for (i = 0; i <= indexHighA; ++i) {
      const int readChars = obj1.streamGetChars(n, &cs->lookup[i*n]);
      for (j = readChars; j < n; ++j) {
        error(errSyntaxWarning, -1, "Bad Indexed color space (lookup table stream too short) padding with zeroes");
        cs->lookup[i*n + j] = 0;
      }
    }
    obj1.streamClose();
  } else if (obj1.isString()) {
    if (obj1.getString()->getLength() < (indexHighA + 1) * n) {
      error(errSyntaxWarning, -1, "Bad Indexed color space (lookup table string too short)");
      goto err3;
    }
    s = obj1.getString()->getCString();
    for (i = 0; i <= indexHighA; ++i) {
      for (j = 0; j < n; ++j) {
	cs->lookup[i*n + j] = (Guchar)*s++;
      }
    }
  } else {
    error(errSyntaxWarning, -1, "Bad Indexed color space (lookup table)");
    goto err3;
  }
  obj1.free();
  return cs;

 err3:
  delete cs;
 err2:
  obj1.free();
 err1:
  return NULL;
}

GfxColor *GfxIndexedColorSpace::mapColorToBase(GfxColor *color,
					       GfxColor *baseColor) {
  Guchar *p;
  double low[gfxColorMaxComps], range[gfxColorMaxComps];
  int n, i;

  n = base->getNComps();
  base->getDefaultRanges(low, range, indexHigh);
  const int idx = (int)(colToDbl(color->c[0]) + 0.5) * n;
  if (likely(idx + n < (indexHigh + 1) * base->getNComps())) {
    p = &lookup[idx];
    for (i = 0; i < n; ++i) {
      baseColor->c[i] = dblToCol(low[i] + (p[i] / 255.0) * range[i]);
    }
  } else {
    for (i = 0; i < n; ++i) {
      baseColor->c[i] = 0;
    }
  }
  return baseColor;
}

void GfxIndexedColorSpace::getGray(GfxColor *color, GfxGray *gray) {
  GfxColor color2;

  base->getGray(mapColorToBase(color, &color2), gray);
}

void GfxIndexedColorSpace::getRGB(GfxColor *color, GfxRGB *rgb) {
  GfxColor color2;

  base->getRGB(mapColorToBase(color, &color2), rgb);
}

void GfxIndexedColorSpace::getRGBLine(Guchar *in, unsigned int *out, int length) {
  Guchar *line;
  int i, j, n;

  n = base->getNComps();
  line = (Guchar *) gmallocn (length, n);
  for (i = 0; i < length; i++)
    for (j = 0; j < n; j++)
      line[i * n + j] = lookup[in[i] * n + j];

  base->getRGBLine(line, out, length);

  gfree (line);
}

void GfxIndexedColorSpace::getRGBLine(Guchar *in, Guchar *out, int length)
{
  Guchar *line;
  int i, j, n;

  n = base->getNComps();
  line = (Guchar *) gmallocn (length, n);
  for (i = 0; i < length; i++)
    for (j = 0; j < n; j++)
      line[i * n + j] = lookup[in[i] * n + j];

  base->getRGBLine(line, out, length);

  gfree (line);
}

void GfxIndexedColorSpace::getRGBXLine(Guchar *in, Guchar *out, int length)
{
  Guchar *line;
  int i, j, n;

  n = base->getNComps();
  line = (Guchar *) gmallocn (length, n);
  for (i = 0; i < length; i++)
    for (j = 0; j < n; j++)
      line[i * n + j] = lookup[in[i] * n + j];

  base->getRGBXLine(line, out, length);

  gfree (line);
}

void GfxIndexedColorSpace::getCMYKLine(Guchar *in, Guchar *out, int length) {
  Guchar *line;
  int i, j, n;

  n = base->getNComps();
  line = (Guchar *) gmallocn (length, n);
  for (i = 0; i < length; i++)
    for (j = 0; j < n; j++)
      line[i * n + j] = lookup[in[i] * n + j];

  base->getCMYKLine(line, out, length);

  gfree (line);
}

void GfxIndexedColorSpace::getDeviceNLine(Guchar *in, Guchar *out, int length) {
  Guchar *line;
  int i, j, n;

  n = base->getNComps();
  line = (Guchar *) gmallocn (length, n);
  for (i = 0; i < length; i++)
    for (j = 0; j < n; j++)
      line[i * n + j] = lookup[in[i] * n + j];

  base->getDeviceNLine(line, out, length);

  gfree (line);
}

void GfxIndexedColorSpace::getCMYK(GfxColor *color, GfxCMYK *cmyk) {
  GfxColor color2;

  base->getCMYK(mapColorToBase(color, &color2), cmyk);
}

void GfxIndexedColorSpace::getDeviceN(GfxColor *color, GfxColor *deviceN) {
  GfxColor color2;

  base->getDeviceN(mapColorToBase(color, &color2), deviceN);
}

void GfxIndexedColorSpace::getDefaultColor(GfxColor *color) {
  color->c[0] = 0;
}

void GfxIndexedColorSpace::getDefaultRanges(double *decodeLow,
					    double *decodeRange,
					    int maxImgPixel) {
  decodeLow[0] = 0;
  decodeRange[0] = maxImgPixel;
}

//------------------------------------------------------------------------
// GfxSeparationColorSpace
//------------------------------------------------------------------------

GfxSeparationColorSpace::GfxSeparationColorSpace(GooString *nameA,
						 GfxColorSpace *altA,
						 Function *funcA) {
  name = nameA;
  alt = altA;
  func = funcA;
  nonMarking = !name->cmp("None");
  if (!name->cmp("Cyan")) {
    overprintMask = 0x01;
  } else if (!name->cmp("Magenta")) {
    overprintMask = 0x02;
  } else if (!name->cmp("Yellow")) {
    overprintMask = 0x04;
  } else if (!name->cmp("Black")) {
    overprintMask = 0x08;
  } else if (!name->cmp("All")) {
    overprintMask = 0xffffffff;
  }
}

GfxSeparationColorSpace::GfxSeparationColorSpace(GooString *nameA,
						 GfxColorSpace *altA,
						 Function *funcA,
						 GBool nonMarkingA,
						 Guint overprintMaskA,
						 int *mappingA) {
  name = nameA;
  alt = altA;
  func = funcA;
  nonMarking = nonMarkingA;
  overprintMask = overprintMaskA;
  mapping = mappingA;
}

GfxSeparationColorSpace::~GfxSeparationColorSpace() {
  delete name;
  delete alt;
  delete func;
  if (mapping != NULL)
    gfree(mapping);
}

GfxColorSpace *GfxSeparationColorSpace::copy() {
  int *mappingA = NULL;
  if (mapping != NULL) {
    mappingA = (int *) gmalloc(sizeof(int));
    *mappingA = *mapping;
  }
  return new GfxSeparationColorSpace(name->copy(), alt->copy(), func->copy(),
				     nonMarking, overprintMask, mappingA);
}

//~ handle the 'All' and 'None' colorants
GfxColorSpace *GfxSeparationColorSpace::parse(Array *arr, OutputDev *out, GfxState *state, int recursion) {
  GfxSeparationColorSpace *cs;
  GooString *nameA;
  GfxColorSpace *altA;
  Function *funcA;
  Object obj1;

  if (arr->getLength() != 4) {
    error(errSyntaxWarning, -1, "Bad Separation color space");
    goto err1;
  }
  if (!arr->get(1, &obj1)->isName()) {
    error(errSyntaxWarning, -1, "Bad Separation color space (name)");
    goto err2;
  }
  nameA = new GooString(obj1.getName());
  obj1.free();
  arr->get(2, &obj1);
  if (!(altA = GfxColorSpace::parse(&obj1, out, state, recursion + 1))) {
    error(errSyntaxWarning, -1, "Bad Separation color space (alternate color space)");
    goto err3;
  }
  obj1.free();
  arr->get(3, &obj1);
  if (!(funcA = Function::parse(&obj1))) {
    goto err4;
  }
  if (funcA->getInputSize() != 1) {
    error(errSyntaxWarning, -1, "Bad SeparationColorSpace function");
    goto err5;
  }
  obj1.free();
  cs = new GfxSeparationColorSpace(nameA, altA, funcA);
  return cs;

 err5:
  delete funcA;
 err4:
  delete altA;
 err3:
  delete nameA;
 err2:
  obj1.free();
 err1:
  return NULL;
}

void GfxSeparationColorSpace::getGray(GfxColor *color, GfxGray *gray) {
  double x;
  double c[gfxColorMaxComps];
  GfxColor color2;
  int i;

  x = colToDbl(color->c[0]);
  func->transform(&x, c);
  for (i = 0; i < alt->getNComps(); ++i) {
    color2.c[i] = dblToCol(c[i]);
  }
  alt->getGray(&color2, gray);
}

void GfxSeparationColorSpace::getRGB(GfxColor *color, GfxRGB *rgb) {
  double x;
  double c[gfxColorMaxComps];
  GfxColor color2;
  int i;

  x = colToDbl(color->c[0]);
  func->transform(&x, c);
  for (i = 0; i < alt->getNComps(); ++i) {
    color2.c[i] = dblToCol(c[i]);
  }
  alt->getRGB(&color2, rgb);
}

void GfxSeparationColorSpace::getCMYK(GfxColor *color, GfxCMYK *cmyk) {
  double x;
  double c[gfxColorMaxComps];
  GfxColor color2;
  int i;

  x = colToDbl(color->c[0]);
  func->transform(&x, c);
  for (i = 0; i < alt->getNComps(); ++i) {
    color2.c[i] = dblToCol(c[i]);
  }
  alt->getCMYK(&color2, cmyk);
}

void GfxSeparationColorSpace::getDeviceN(GfxColor *color, GfxColor *deviceN) {
  for (int i = 0; i < gfxColorMaxComps; i++)
    deviceN->c[i] = 0;
  if (mapping == NULL || mapping[0] == -1) {
    GfxCMYK cmyk;

    getCMYK(color, &cmyk);
    deviceN->c[0] = cmyk.c;
    deviceN->c[1] = cmyk.m;
    deviceN->c[2] = cmyk.y;
    deviceN->c[3] = cmyk.k;
  } else {
    deviceN->c[mapping[0]] = color->c[0];
  }
}

void GfxSeparationColorSpace::getDefaultColor(GfxColor *color) {
  color->c[0] = gfxColorComp1;
}

void GfxSeparationColorSpace::createMapping(GooList *separationList, int maxSepComps) {
  if (nonMarking)
    return;
  mapping = (int *)gmalloc(sizeof(int));
  switch (overprintMask) {
    case 0x01:
      *mapping = 0;
      break;
    case 0x02:
      *mapping = 1;
      break;
    case 0x04:
      *mapping = 2;
      break;
    case 0x08:
      *mapping = 3;
      break;
    default:
      Guint newOverprintMask = 0x10;
      for (int i = 0; i < separationList->getLength(); i++) {
        GfxSeparationColorSpace *sepCS = (GfxSeparationColorSpace *)separationList->get(i);
        if (!sepCS->getName()->cmp(name)) {
          if (sepCS->getFunc()->hasDifferentResultSet(func)) {
            error(errSyntaxWarning, -1,
              "Different functions found for '{0:s}', convert immediately", name);
            gfree(mapping);
            mapping = NULL;
            return;
          }
          *mapping = i+4;
          overprintMask = newOverprintMask;
          return;
        }
        newOverprintMask <<=1;
      }
      if (separationList->getLength() == maxSepComps) {
        error(errSyntaxWarning, -1,
	        "Too many ({0:d}) spots, convert '{1:s}' immediately", maxSepComps, name);
        gfree(mapping);
        mapping = NULL;
        return;
      }
      *mapping = separationList->getLength() + 4;
      separationList->append(copy());
      overprintMask = newOverprintMask;
      break;
  }
}

//------------------------------------------------------------------------
// GfxDeviceNColorSpace
//------------------------------------------------------------------------

GfxDeviceNColorSpace::GfxDeviceNColorSpace(int nCompsA,
					   GooString **namesA,
					   GfxColorSpace *altA,
					   Function *funcA,
					   GooList *sepsCSA) {
  int i;

  nComps = nCompsA;
  alt = altA;
  func = funcA;
  sepsCS = sepsCSA;
  nonMarking = gTrue;
  overprintMask = 0;
  mapping = NULL;
  for (i = 0; i < nComps; ++i) {
    names[i] = namesA[i];
    if (names[i]->cmp("None")) {
      nonMarking = gFalse;
    }
    if (!names[i]->cmp("Cyan")) {
      overprintMask |= 0x01;
    } else if (!names[i]->cmp("Magenta")) {
      overprintMask |= 0x02;
    } else if (!names[i]->cmp("Yellow")) {
      overprintMask |= 0x04;
    } else if (!names[i]->cmp("Black")) {
      overprintMask |= 0x08;
    } else if (!names[i]->cmp("All")) {
      overprintMask = 0xffffffff;
    } else {
      overprintMask = 0x0f;
    }
  }
}

GfxDeviceNColorSpace::GfxDeviceNColorSpace(int nCompsA,
					   GooString **namesA,
					   GfxColorSpace *altA,
					   Function *funcA,
					   GooList *sepsCSA,
					   int *mappingA,
					   GBool nonMarkingA,
					   Guint overprintMaskA) {
  int i;

  nComps = nCompsA;
  alt = altA;
  func = funcA;
  sepsCS = sepsCSA;
  mapping = mappingA;
  nonMarking = nonMarkingA;
  overprintMask = overprintMaskA;
  for (i = 0; i < nComps; ++i) {
    names[i] = namesA[i]->copy();
  }
}

GfxDeviceNColorSpace::~GfxDeviceNColorSpace() {
  int i;

  for (i = 0; i < nComps; ++i) {
    delete names[i];
  }
  delete alt;
  delete func;
  deleteGooList(sepsCS, GfxSeparationColorSpace);
  if (mapping != NULL)
    gfree(mapping);
}

GfxColorSpace *GfxDeviceNColorSpace::copy() {
  int i;
  int *mappingA = NULL;

  GooList *sepsCSA = new GooList(sepsCS->getLength());
  for (i = 0; i < sepsCS->getLength(); i++)
    sepsCSA->append(((GfxSeparationColorSpace *) sepsCS->get(i))->copy());
  if (mapping != NULL) {
    mappingA = (int *)gmalloc(sizeof(int) * nComps);
    for (i = 0; i < nComps; i++)
      mappingA[i] = mapping[i];
  }
  return new GfxDeviceNColorSpace(nComps, names, alt->copy(), func->copy(),
				  sepsCSA, mappingA, nonMarking, overprintMask);
}

//~ handle the 'None' colorant
GfxColorSpace *GfxDeviceNColorSpace::parse(Array *arr, OutputDev *out, GfxState *state, int recursion) {
  GfxDeviceNColorSpace *cs;
  int nCompsA;
  GooString *namesA[gfxColorMaxComps];
  GfxColorSpace *altA;
  Function *funcA;
  Object obj1, obj2;
  int i;
  GooList *separationList = new GooList();

  if (arr->getLength() != 4 && arr->getLength() != 5) {
    error(errSyntaxWarning, -1, "Bad DeviceN color space");
    goto err1;
  }
  if (!arr->get(1, &obj1)->isArray()) {
    error(errSyntaxWarning, -1, "Bad DeviceN color space (names)");
    goto err2;
  }
  nCompsA = obj1.arrayGetLength();
  if (nCompsA > gfxColorMaxComps) {
    error(errSyntaxWarning, -1, "DeviceN color space with too many ({0:d} > {1:d}) components",
	  nCompsA, gfxColorMaxComps);
    nCompsA = gfxColorMaxComps;
  }
  for (i = 0; i < nCompsA; ++i) {
    if (!obj1.arrayGet(i, &obj2)->isName()) {
      error(errSyntaxWarning, -1, "Bad DeviceN color space (names)");
      obj2.free();
      goto err2;
    }
    namesA[i] = new GooString(obj2.getName());
    obj2.free();
  }
  obj1.free();
  arr->get(2, &obj1);
  if (!(altA = GfxColorSpace::parse(&obj1, out, state, recursion + 1))) {
    error(errSyntaxWarning, -1, "Bad DeviceN color space (alternate color space)");
    goto err3;
  }
  obj1.free();
  arr->get(3, &obj1);
  if (!(funcA = Function::parse(&obj1))) {
    goto err4;
  }
  obj1.free();
  if (arr->getLength() == 5) {
    if (!arr->get(4, &obj1)->isDict()) {
      error(errSyntaxWarning, -1, "Bad DeviceN color space (attributes)");
      goto err4;
    }
    Dict *attribs = obj1.getDict();
    attribs->lookup("Colorants", &obj2);
    if (obj2.isDict()) {
      Dict *colorants = obj2.getDict();
      for (i = 0; i < colorants->getLength(); i++) {
        Object obj3;
        colorants->getVal(i, &obj3);
        if (obj3.isArray()) {
          separationList->append(GfxSeparationColorSpace::parse(obj3.getArray(), out, state, recursion));
        } else {
          obj3.free();
          obj2.free();
          error(errSyntaxWarning, -1, "Bad DeviceN color space (colorant value entry is not an Array)");
          goto err4;
        }
        obj3.free();
      }
    }
    obj2.free();
    obj1.free();
  }
  cs = new GfxDeviceNColorSpace(nCompsA, namesA, altA, funcA, separationList);
  return cs;

 err4:
  delete altA;
 err3:
  for (i = 0; i < nCompsA; ++i) {
    delete namesA[i];
  }
 err2:
  obj1.free();
 err1:
  delete separationList;
  return NULL;
}

void GfxDeviceNColorSpace::getGray(GfxColor *color, GfxGray *gray) {
  double x[gfxColorMaxComps], c[gfxColorMaxComps];
  GfxColor color2;
  int i;

  for (i = 0; i < nComps; ++i) {
    x[i] = colToDbl(color->c[i]);
  }
  func->transform(x, c);
  for (i = 0; i < alt->getNComps(); ++i) {
    color2.c[i] = dblToCol(c[i]);
  }
  alt->getGray(&color2, gray);
}

void GfxDeviceNColorSpace::getRGB(GfxColor *color, GfxRGB *rgb) {
  double x[gfxColorMaxComps], c[gfxColorMaxComps];
  GfxColor color2;
  int i;

  for (i = 0; i < nComps; ++i) {
    x[i] = colToDbl(color->c[i]);
  }
  func->transform(x, c);
  for (i = 0; i < alt->getNComps(); ++i) {
    color2.c[i] = dblToCol(c[i]);
  }
  alt->getRGB(&color2, rgb);
}

void GfxDeviceNColorSpace::getCMYK(GfxColor *color, GfxCMYK *cmyk) {
  double x[gfxColorMaxComps], c[gfxColorMaxComps];
  GfxColor color2;
  int i;

  for (i = 0; i < nComps; ++i) {
    x[i] = colToDbl(color->c[i]);
  }
  func->transform(x, c);
  for (i = 0; i < alt->getNComps(); ++i) {
    color2.c[i] = dblToCol(c[i]);
  }
  alt->getCMYK(&color2, cmyk);
}

void GfxDeviceNColorSpace::getDeviceN(GfxColor *color, GfxColor *deviceN) {
  for (int i = 0; i < gfxColorMaxComps; i++)
    deviceN->c[i] = 0;
  if (mapping == NULL) {
    GfxCMYK cmyk;

    getCMYK(color, &cmyk);
    deviceN->c[0] = cmyk.c;
    deviceN->c[1] = cmyk.m;
    deviceN->c[2] = cmyk.y;
    deviceN->c[3] = cmyk.k;
  } else {
    for (int j = 0; j < nComps; j++)
      if (mapping[j] != -1)
        deviceN->c[mapping[j]] = color->c[j];
  }
}

void GfxDeviceNColorSpace::getDefaultColor(GfxColor *color) {
  int i;

  for (i = 0; i < nComps; ++i) {
    color->c[i] = gfxColorComp1;
  }
}

void GfxDeviceNColorSpace::createMapping(GooList *separationList, int maxSepComps) {
  if (nonMarking)               // None
    return;
  mapping = (int *)gmalloc(sizeof(int) * nComps);
  Guint newOverprintMask = 0;
  for (int i = 0; i < nComps; i++) {
    if (!names[i]->cmp("None")) {
      mapping[i] = -1;
    } else if (!names[i]->cmp("Cyan")) {
      newOverprintMask |= 0x01;
      mapping[i] = 0;
    } else if (!names[i]->cmp("Magenta")) {
      newOverprintMask |= 0x02;
      mapping[i] = 1;
    } else if (!names[i]->cmp("Yellow")) {
      newOverprintMask |= 0x04;
      mapping[i] = 2;
    } else if (!names[i]->cmp("Black")) {
      newOverprintMask |= 0x08;
      mapping[i] = 3;
    } else {
      Guint startOverprintMask = 0x10;
      GBool found = gFalse;
      Function *sepFunc = NULL;
      if (nComps == 1)
        sepFunc = func;
      else {
        for (int k = 0; k < sepsCS->getLength(); k++) {
          GfxSeparationColorSpace *sepCS = (GfxSeparationColorSpace *)sepsCS->get(k);
          if (!sepCS->getName()->cmp(names[i])) {
            sepFunc = sepCS->getFunc();
            break;
          }
        }
      }
      for (int j = 0; j < separationList->getLength(); j++) {
        GfxSeparationColorSpace *sepCS = (GfxSeparationColorSpace *)separationList->get(j);
        if (!sepCS->getName()->cmp(names[i])) {
          if (sepFunc != NULL && sepCS->getFunc()->hasDifferentResultSet(sepFunc)) {
            error(errSyntaxWarning, -1,
              "Different functions found for '{0:s}', convert immediately", names[i]);
            gfree(mapping);
            mapping = NULL;
            overprintMask = 0xffffffff;
            return;
          }
          mapping[i] = j+4;
          newOverprintMask |= startOverprintMask;
          found = gTrue;
          break;
        }
        startOverprintMask <<=1;
      }
      if (!found) {
        if (separationList->getLength() == maxSepComps) {
          error(errSyntaxWarning, -1,
            "Too many ({0:d}) spots, convert '{1:s}' immediately", maxSepComps, names[i]);
          gfree(mapping);
          mapping = NULL;
          overprintMask = 0xffffffff;
          return;
        }
        mapping[i] = separationList->getLength() + 4;
        newOverprintMask |= startOverprintMask;
        if (nComps == 1)
          separationList->append(new GfxSeparationColorSpace(names[i]->copy(),alt->copy(), func->copy()));
        else {
          for (int k = 0; k < sepsCS->getLength(); k++) {
            GfxSeparationColorSpace *sepCS = (GfxSeparationColorSpace *)sepsCS->get(k);
            if (!sepCS->getName()->cmp(names[i])) {
              found = gTrue;
              separationList->append(sepCS->copy());
              break;
            }
          }
          if(!found) {
            error(errSyntaxWarning, -1, "DeviceN has no suitable colorant");
            gfree(mapping);
            mapping = NULL;
            overprintMask = 0xffffffff;
            return;
          }
        }
      }
    }
  }
  overprintMask = newOverprintMask;
}

//------------------------------------------------------------------------
// GfxPatternColorSpace
//------------------------------------------------------------------------

GfxPatternColorSpace::GfxPatternColorSpace(GfxColorSpace *underA) {
  under = underA;
}

GfxPatternColorSpace::~GfxPatternColorSpace() {
  if (under) {
    delete under;
  }
}

GfxColorSpace *GfxPatternColorSpace::copy() {
  return new GfxPatternColorSpace(under ? under->copy() :
				          (GfxColorSpace *)NULL);
}

GfxColorSpace *GfxPatternColorSpace::parse(Array *arr, OutputDev *out, GfxState *state, int recursion) {
  GfxPatternColorSpace *cs;
  GfxColorSpace *underA;
  Object obj1;

  if (arr->getLength() != 1 && arr->getLength() != 2) {
    error(errSyntaxWarning, -1, "Bad Pattern color space");
    return NULL;
  }
  underA = NULL;
  if (arr->getLength() == 2) {
    arr->get(1, &obj1);
    if (!(underA = GfxColorSpace::parse(&obj1, out, state, recursion + 1))) {
      error(errSyntaxWarning, -1, "Bad Pattern color space (underlying color space)");
      obj1.free();
      return NULL;
    }
    obj1.free();
  }
  cs = new GfxPatternColorSpace(underA);
  return cs;
}

void GfxPatternColorSpace::getGray(GfxColor *color, GfxGray *gray) {
  *gray = 0;
}

void GfxPatternColorSpace::getRGB(GfxColor *color, GfxRGB *rgb) {
  rgb->r = rgb->g = rgb->b = 0;
}

void GfxPatternColorSpace::getCMYK(GfxColor *color, GfxCMYK *cmyk) {
  cmyk->c = cmyk->m = cmyk->y = 0;
  cmyk->k = 1;
}

void GfxPatternColorSpace::getDeviceN(GfxColor *color, GfxColor *deviceN) {
  for (int i = 0; i < gfxColorMaxComps; i++)
    deviceN->c[i] = 0;
  deviceN->c[3] = 1;
}

void GfxPatternColorSpace::getDefaultColor(GfxColor *color) {
  color->c[0]=0;
}

//------------------------------------------------------------------------
// Pattern
//------------------------------------------------------------------------

GfxPattern::GfxPattern(int typeA) {
  type = typeA;
}

GfxPattern::~GfxPattern() {
}

GfxPattern *GfxPattern::parse(Object *obj, OutputDev *out, GfxState *state) {
  GfxPattern *pattern;
  Object obj1;

  if (obj->isDict()) {
    obj->dictLookup("PatternType", &obj1);
  } else if (obj->isStream()) {
    obj->streamGetDict()->lookup("PatternType", &obj1);
  } else {
    return NULL;
  }
  pattern = NULL;
  if (obj1.isInt() && obj1.getInt() == 1) {
    pattern = GfxTilingPattern::parse(obj);
  } else if (obj1.isInt() && obj1.getInt() == 2) {
    pattern = GfxShadingPattern::parse(obj, out, state);
  }
  obj1.free();
  return pattern;
}

//------------------------------------------------------------------------
// GfxTilingPattern
//------------------------------------------------------------------------

GfxTilingPattern *GfxTilingPattern::parse(Object *patObj) {
  GfxTilingPattern *pat;
  Dict *dict;
  int paintTypeA, tilingTypeA;
  double bboxA[4], matrixA[6];
  double xStepA, yStepA;
  Object resDictA;
  Object obj1, obj2;
  int i;

  if (!patObj->isStream()) {
    return NULL;
  }
  dict = patObj->streamGetDict();

  if (dict->lookup("PaintType", &obj1)->isInt()) {
    paintTypeA = obj1.getInt();
  } else {
    paintTypeA = 1;
    error(errSyntaxWarning, -1, "Invalid or missing PaintType in pattern");
  }
  obj1.free();
  if (dict->lookup("TilingType", &obj1)->isInt()) {
    tilingTypeA = obj1.getInt();
  } else {
    tilingTypeA = 1;
    error(errSyntaxWarning, -1, "Invalid or missing TilingType in pattern");
  }
  obj1.free();
  bboxA[0] = bboxA[1] = 0;
  bboxA[2] = bboxA[3] = 1;
  if (dict->lookup("BBox", &obj1)->isArray() &&
      obj1.arrayGetLength() == 4) {
    for (i = 0; i < 4; ++i) {
      if (obj1.arrayGet(i, &obj2)->isNum()) {
	bboxA[i] = obj2.getNum();
      }
      obj2.free();
    }
  } else {
    error(errSyntaxWarning, -1, "Invalid or missing BBox in pattern");
  }
  obj1.free();
  if (dict->lookup("XStep", &obj1)->isNum()) {
    xStepA = obj1.getNum();
  } else {
    xStepA = 1;
    error(errSyntaxWarning, -1, "Invalid or missing XStep in pattern");
  }
  obj1.free();
  if (dict->lookup("YStep", &obj1)->isNum()) {
    yStepA = obj1.getNum();
  } else {
    yStepA = 1;
    error(errSyntaxWarning, -1, "Invalid or missing YStep in pattern");
  }
  obj1.free();
  if (!dict->lookup("Resources", &resDictA)->isDict()) {
    resDictA.free();
    resDictA.initNull();
    error(errSyntaxWarning, -1, "Invalid or missing Resources in pattern");
  }
  matrixA[0] = 1; matrixA[1] = 0;
  matrixA[2] = 0; matrixA[3] = 1;
  matrixA[4] = 0; matrixA[5] = 0;
  if (dict->lookup("Matrix", &obj1)->isArray() &&
      obj1.arrayGetLength() == 6) {
    for (i = 0; i < 6; ++i) {
      if (obj1.arrayGet(i, &obj2)->isNum()) {
	matrixA[i] = obj2.getNum();
      }
      obj2.free();
    }
  }
  obj1.free();

  pat = new GfxTilingPattern(paintTypeA, tilingTypeA, bboxA, xStepA, yStepA,
			     &resDictA, matrixA, patObj);
  resDictA.free();
  return pat;
}

GfxTilingPattern::GfxTilingPattern(int paintTypeA, int tilingTypeA,
				   double *bboxA, double xStepA, double yStepA,
				   Object *resDictA, double *matrixA,
				   Object *contentStreamA):
  GfxPattern(1)
{
  int i;

  paintType = paintTypeA;
  tilingType = tilingTypeA;
  for (i = 0; i < 4; ++i) {
    bbox[i] = bboxA[i];
  }
  xStep = xStepA;
  yStep = yStepA;
  resDictA->copy(&resDict);
  for (i = 0; i < 6; ++i) {
    matrix[i] = matrixA[i];
  }
  contentStreamA->copy(&contentStream);
}

GfxTilingPattern::~GfxTilingPattern() {
  resDict.free();
  contentStream.free();
}

GfxPattern *GfxTilingPattern::copy() {
  return new GfxTilingPattern(paintType, tilingType, bbox, xStep, yStep,
			      &resDict, matrix, &contentStream);
}

//------------------------------------------------------------------------
// GfxShadingPattern
//------------------------------------------------------------------------

GfxShadingPattern *GfxShadingPattern::parse(Object *patObj, OutputDev *out, GfxState *state) {
  Dict *dict;
  GfxShading *shadingA;
  double matrixA[6];
  Object obj1, obj2;
  int i;

  if (!patObj->isDict()) {
    return NULL;
  }
  dict = patObj->getDict();

  dict->lookup("Shading", &obj1);
  shadingA = GfxShading::parse(&obj1, out, state);
  obj1.free();
  if (!shadingA) {
    return NULL;
  }

  matrixA[0] = 1; matrixA[1] = 0;
  matrixA[2] = 0; matrixA[3] = 1;
  matrixA[4] = 0; matrixA[5] = 0;
  if (dict->lookup("Matrix", &obj1)->isArray() &&
      obj1.arrayGetLength() == 6) {
    for (i = 0; i < 6; ++i) {
      if (obj1.arrayGet(i, &obj2)->isNum()) {
	matrixA[i] = obj2.getNum();
      }
      obj2.free();
    }
  }
  obj1.free();

  return new GfxShadingPattern(shadingA, matrixA);
}

GfxShadingPattern::GfxShadingPattern(GfxShading *shadingA, double *matrixA):
  GfxPattern(2)
{
  int i;

  shading = shadingA;
  for (i = 0; i < 6; ++i) {
    matrix[i] = matrixA[i];
  }
}

GfxShadingPattern::~GfxShadingPattern() {
  delete shading;
}

GfxPattern *GfxShadingPattern::copy() {
  return new GfxShadingPattern(shading->copy(), matrix);
}

//------------------------------------------------------------------------
// GfxShading
//------------------------------------------------------------------------

GfxShading::GfxShading(int typeA) {
  type = typeA;
  colorSpace = NULL;
}

GfxShading::GfxShading(GfxShading *shading) {
  int i;

  type = shading->type;
  colorSpace = shading->colorSpace->copy();
  for (i = 0; i < gfxColorMaxComps; ++i) {
    background.c[i] = shading->background.c[i];
  }
  hasBackground = shading->hasBackground;
  xMin = shading->xMin;
  yMin = shading->yMin;
  xMax = shading->xMax;
  yMax = shading->yMax;
  hasBBox = shading->hasBBox;
}

GfxShading::~GfxShading() {
  if (colorSpace) {
    delete colorSpace;
  }
}

GfxShading *GfxShading::parse(Object *obj, OutputDev *out, GfxState *state) {
  GfxShading *shading;
  Dict *dict;
  int typeA;
  Object obj1;

  if (obj->isDict()) {
    dict = obj->getDict();
  } else if (obj->isStream()) {
    dict = obj->streamGetDict();
  } else {
    return NULL;
  }

  if (!dict->lookup("ShadingType", &obj1)->isInt()) {
    error(errSyntaxWarning, -1, "Invalid ShadingType in shading dictionary");
    obj1.free();
    return NULL;
  }
  typeA = obj1.getInt();
  obj1.free();

  switch (typeA) {
  case 1:
    shading = GfxFunctionShading::parse(dict, out, state);
    break;
  case 2:
    shading = GfxAxialShading::parse(dict, out, state);
    break;
  case 3:
    shading = GfxRadialShading::parse(dict, out, state);
    break;
  case 4:
    if (obj->isStream()) {
      shading = GfxGouraudTriangleShading::parse(4, dict, obj->getStream(), out, state);
    } else {
      error(errSyntaxWarning, -1, "Invalid Type 4 shading object");
      goto err1;
    }
    break;
  case 5:
    if (obj->isStream()) {
      shading = GfxGouraudTriangleShading::parse(5, dict, obj->getStream(), out, state);
    } else {
      error(errSyntaxWarning, -1, "Invalid Type 5 shading object");
      goto err1;
    }
    break;
  case 6:
    if (obj->isStream()) {
      shading = GfxPatchMeshShading::parse(6, dict, obj->getStream(), out, state);
    } else {
      error(errSyntaxWarning, -1, "Invalid Type 6 shading object");
      goto err1;
    }
    break;
  case 7:
    if (obj->isStream()) {
      shading = GfxPatchMeshShading::parse(7, dict, obj->getStream(), out, state);
    } else {
      error(errSyntaxWarning, -1, "Invalid Type 7 shading object");
      goto err1;
    }
    break;
  default:
    error(errSyntaxWarning, -1, "Unimplemented shading type {0:d}", typeA);
    goto err1;
  }

  return shading;

 err1:
  return NULL;
}

GBool GfxShading::init(Dict *dict, OutputDev *out, GfxState *state) {
  Object obj1, obj2;
  int i;

  dict->lookup("ColorSpace", &obj1);
  if (!(colorSpace = GfxColorSpace::parse(&obj1, out, state))) {
    error(errSyntaxWarning, -1, "Bad color space in shading dictionary");
    obj1.free();
    return gFalse;
  }
  obj1.free();

  for (i = 0; i < gfxColorMaxComps; ++i) {
    background.c[i] = 0;
  }
  hasBackground = gFalse;
  if (dict->lookup("Background", &obj1)->isArray()) {
    if (obj1.arrayGetLength() == colorSpace->getNComps()) {
      hasBackground = gTrue;
      for (i = 0; i < colorSpace->getNComps(); ++i) {
	background.c[i] = dblToCol(obj1.arrayGet(i, &obj2)->getNum());
	obj2.free();
      }
    } else {
      error(errSyntaxWarning, -1, "Bad Background in shading dictionary");
    }
  }
  obj1.free();

  xMin = yMin = xMax = yMax = 0;
  hasBBox = gFalse;
  if (dict->lookup("BBox", &obj1)->isArray()) {
    if (obj1.arrayGetLength() == 4) {
      Object obj3, obj4, obj5;
      obj1.arrayGet(0, &obj2);
      obj1.arrayGet(1, &obj3);
      obj1.arrayGet(2, &obj4);
      obj1.arrayGet(3, &obj5);
      if (obj2.isNum() && obj3.isNum() && obj4.isNum() && obj5.isNum())
      {
        hasBBox = gTrue;
        xMin = obj2.getNum();
        yMin = obj3.getNum();
        xMax = obj4.getNum();
        yMax = obj5.getNum();
      } else {
        error(errSyntaxWarning, -1, "Bad BBox in shading dictionary (Values not numbers)");
      }
      obj2.free();
      obj3.free();
      obj4.free();
      obj5.free();
    } else {
      error(errSyntaxWarning, -1, "Bad BBox in shading dictionary");
    }
  }
  obj1.free();

  return gTrue;
}

//------------------------------------------------------------------------
// GfxFunctionShading
//------------------------------------------------------------------------

GfxFunctionShading::GfxFunctionShading(double x0A, double y0A,
				       double x1A, double y1A,
				       double *matrixA,
				       Function **funcsA, int nFuncsA):
  GfxShading(1)
{
  int i;

  x0 = x0A;
  y0 = y0A;
  x1 = x1A;
  y1 = y1A;
  for (i = 0; i < 6; ++i) {
    matrix[i] = matrixA[i];
  }
  nFuncs = nFuncsA;
  for (i = 0; i < nFuncs; ++i) {
    funcs[i] = funcsA[i];
  }
}

GfxFunctionShading::GfxFunctionShading(GfxFunctionShading *shading):
  GfxShading(shading)
{
  int i;

  x0 = shading->x0;
  y0 = shading->y0;
  x1 = shading->x1;
  y1 = shading->y1;
  for (i = 0; i < 6; ++i) {
    matrix[i] = shading->matrix[i];
  }
  nFuncs = shading->nFuncs;
  for (i = 0; i < nFuncs; ++i) {
    funcs[i] = shading->funcs[i]->copy();
  }
}

GfxFunctionShading::~GfxFunctionShading() {
  int i;

  for (i = 0; i < nFuncs; ++i) {
    delete funcs[i];
  }
}

GfxFunctionShading *GfxFunctionShading::parse(Dict *dict, OutputDev *out, GfxState *state) {
  GfxFunctionShading *shading;
  double x0A, y0A, x1A, y1A;
  double matrixA[6];
  Function *funcsA[gfxColorMaxComps];
  int nFuncsA;
  Object obj1, obj2;
  int i;

  x0A = y0A = 0;
  x1A = y1A = 1;
  if (dict->lookup("Domain", &obj1)->isArray() &&
      obj1.arrayGetLength() == 4) {
    x0A = obj1.arrayGet(0, &obj2)->getNum();
    obj2.free();
    x1A = obj1.arrayGet(1, &obj2)->getNum();
    obj2.free();
    y0A = obj1.arrayGet(2, &obj2)->getNum();
    obj2.free();
    y1A = obj1.arrayGet(3, &obj2)->getNum();
    obj2.free();
  }
  obj1.free();

  matrixA[0] = 1; matrixA[1] = 0;
  matrixA[2] = 0; matrixA[3] = 1;
  matrixA[4] = 0; matrixA[5] = 0;
  if (dict->lookup("Matrix", &obj1)->isArray() &&
      obj1.arrayGetLength() == 6) {
    matrixA[0] = obj1.arrayGet(0, &obj2)->getNum();
    obj2.free();
    matrixA[1] = obj1.arrayGet(1, &obj2)->getNum();
    obj2.free();
    matrixA[2] = obj1.arrayGet(2, &obj2)->getNum();
    obj2.free();
    matrixA[3] = obj1.arrayGet(3, &obj2)->getNum();
    obj2.free();
    matrixA[4] = obj1.arrayGet(4, &obj2)->getNum();
    obj2.free();
    matrixA[5] = obj1.arrayGet(5, &obj2)->getNum();
    obj2.free();
  }
  obj1.free();

  dict->lookup("Function", &obj1);
  if (obj1.isArray()) {
    nFuncsA = obj1.arrayGetLength();
    if (nFuncsA > gfxColorMaxComps) {
      error(errSyntaxWarning, -1, "Invalid Function array in shading dictionary");
      goto err1;
    }
    for (i = 0; i < nFuncsA; ++i) {
      obj1.arrayGet(i, &obj2);
      if (!(funcsA[i] = Function::parse(&obj2))) {
	goto err2;
      }
      obj2.free();
    }
  } else {
    nFuncsA = 1;
    if (!(funcsA[0] = Function::parse(&obj1))) {
      goto err1;
    }
  }
  obj1.free();

  shading = new GfxFunctionShading(x0A, y0A, x1A, y1A, matrixA,
				   funcsA, nFuncsA);
  if (!shading->init(dict, out, state)) {
    delete shading;
    return NULL;
  }
  return shading;

 err2:
  obj2.free();
 err1:
  obj1.free();
  return NULL;
}

GfxShading *GfxFunctionShading::copy() {
  return new GfxFunctionShading(this);
}

void GfxFunctionShading::getColor(double x, double y, GfxColor *color) {
  double in[2], out[gfxColorMaxComps];
  int i;

  // NB: there can be one function with n outputs or n functions with
  // one output each (where n = number of color components)
  for (i = 0; i < gfxColorMaxComps; ++i) {
    out[i] = 0;
  }
  in[0] = x;
  in[1] = y;
  for (i = 0; i < nFuncs; ++i) {
    funcs[i]->transform(in, &out[i]);
  }
  for (i = 0; i < gfxColorMaxComps; ++i) {
    color->c[i] = dblToCol(out[i]);
  }
}

//------------------------------------------------------------------------
// GfxUnivariateShading
//------------------------------------------------------------------------

GfxUnivariateShading::GfxUnivariateShading(int typeA,
					   double t0A, double t1A,
					   Function **funcsA, int nFuncsA,
					   GBool extend0A, GBool extend1A):
  GfxShading(typeA)
{
  int i;

  t0 = t0A;
  t1 = t1A;
  nFuncs = nFuncsA;
  for (i = 0; i < nFuncs; ++i) {
    funcs[i] = funcsA[i];
  }
  extend0 = extend0A;
  extend1 = extend1A;

  cacheSize = 0;
  lastMatch = 0;
  cacheBounds = NULL;
  cacheCoeff = NULL;
  cacheValues = NULL;
}

GfxUnivariateShading::GfxUnivariateShading(GfxUnivariateShading *shading):
  GfxShading(shading)
{
  int i;

  t0 = shading->t0;
  t1 = shading->t1;
  nFuncs = shading->nFuncs;
  for (i = 0; i < nFuncs; ++i) {
    funcs[i] = shading->funcs[i]->copy();
  }
  extend0 = shading->extend0;
  extend1 = shading->extend1;

  cacheSize = 0;
  lastMatch = 0;
  cacheBounds = NULL;
  cacheCoeff = NULL;
  cacheValues = NULL;
}

GfxUnivariateShading::~GfxUnivariateShading() {
  int i;

  for (i = 0; i < nFuncs; ++i) {
    delete funcs[i];
  }

  gfree (cacheBounds);
}

void GfxUnivariateShading::getColor(double t, GfxColor *color) {
  double out[gfxColorMaxComps];
  int i, nComps;

  // NB: there can be one function with n outputs or n functions with
  // one output each (where n = number of color components)
  nComps = nFuncs * funcs[0]->getOutputSize();

  if (cacheSize > 0) {
    double x, ix, *l, *u, *upper;

    if (cacheBounds[lastMatch - 1] >= t) {
      upper = std::lower_bound (cacheBounds, cacheBounds + lastMatch - 1, t);
      lastMatch = upper - cacheBounds;
      lastMatch = std::min<int>(std::max<int>(1, lastMatch), cacheSize - 1);
    } else if (cacheBounds[lastMatch] < t) {
      upper = std::lower_bound (cacheBounds + lastMatch + 1, cacheBounds + cacheSize, t);
      lastMatch = upper - cacheBounds;
      lastMatch = std::min<int>(std::max<int>(1, lastMatch), cacheSize - 1);
    }

    x = (t - cacheBounds[lastMatch-1]) * cacheCoeff[lastMatch];
    ix = 1.0 - x;
    u = cacheValues + lastMatch * nComps;
    l = u - nComps;

    for (i = 0; i < nComps; ++i) {
      out[i] = ix * l[i] + x * u[i];
    }
  } else {
    for (i = 0; i < nComps; ++i) {
      out[i] = 0;
    }
    for (i = 0; i < nFuncs; ++i) {
      if (funcs[i]->getInputSize() != 1) {
        error(errSyntaxWarning, -1, "Invalid shading function (input != 1)");
        break;
      }
      funcs[i]->transform(&t, &out[i]);
    }
  }

  for (i = 0; i < nComps; ++i) {
    color->c[i] = dblToCol(out[i]);
  }
}

void GfxUnivariateShading::setupCache(const Matrix *ctm,
				      double xMin, double yMin,
				      double xMax, double yMax) {
  double sMin, sMax, tMin, tMax, upperBound;
  int i, j, nComps, maxSize;

  gfree (cacheBounds);
  cacheBounds = NULL;
  cacheSize = 0;

  // NB: there can be one function with n outputs or n functions with
  // one output each (where n = number of color components)
  nComps = nFuncs * funcs[0]->getOutputSize();

  getParameterRange(&sMin, &sMax, xMin, yMin, xMax, yMax);
  upperBound = ctm->norm() * getDistance(sMin, sMax);
  maxSize = ceil(upperBound);
  maxSize = std::max<int>(maxSize, 2);

  {
    double x[4], y[4];

    ctm->transform(xMin, yMin, &x[0], &y[0]);
    ctm->transform(xMax, yMin, &x[1], &y[1]);
    ctm->transform(xMin, yMax, &x[2], &y[2]);
    ctm->transform(xMax, yMax, &x[3], &y[3]);

    xMin = xMax = x[0];
    yMin = yMax = y[0];
    for (i = 1; i < 4; i++) {
      xMin = std::min<double>(xMin, x[i]);
      yMin = std::min<double>(yMin, y[i]);
      xMax = std::max<double>(xMax, x[i]);
      yMax = std::max<double>(yMax, y[i]);
    }
  }

  if (maxSize > (xMax-xMin) * (yMax-yMin)) {
    return;
  }

  if (t0 < t1) {
    tMin = t0 + sMin * (t1 - t0);
    tMax = t0 + sMax * (t1 - t0);
  } else {
    tMin = t0 + sMax * (t1 - t0);
    tMax = t0 + sMin * (t1 - t0);
  }

  cacheBounds = (double *)gmallocn(maxSize, sizeof(double) * (nComps + 2));
  cacheCoeff = cacheBounds + maxSize;
  cacheValues = cacheCoeff + maxSize;

  if (cacheSize != 0) {
    for (j = 0; j < cacheSize; ++j) {
      cacheCoeff[j] = 1 / (cacheBounds[j+1] - cacheBounds[j]);
    }
  } else if (tMax != tMin) {
    double step = (tMax - tMin) / (maxSize - 1);
    double coeff = (maxSize - 1) / (tMax - tMin);

    cacheSize = maxSize;

    for (j = 0; j < cacheSize; ++j) {
      cacheBounds[j] = tMin + j * step;
      cacheCoeff[j] = coeff;

      for (i = 0; i < nComps; ++i) {
	cacheValues[j*nComps + i] = 0;
      }
      for (i = 0; i < nFuncs; ++i) {
	funcs[i]->transform(&cacheBounds[j], &cacheValues[j*nComps + i]);
      }
    }
  }

  lastMatch = 1;
}


//------------------------------------------------------------------------
// GfxAxialShading
//------------------------------------------------------------------------

GfxAxialShading::GfxAxialShading(double x0A, double y0A,
				 double x1A, double y1A,
				 double t0A, double t1A,
				 Function **funcsA, int nFuncsA,
				 GBool extend0A, GBool extend1A):
  GfxUnivariateShading(2, t0A, t1A, funcsA, nFuncsA, extend0A, extend1A)
{
  x0 = x0A;
  y0 = y0A;
  x1 = x1A;
  y1 = y1A;
}

GfxAxialShading::GfxAxialShading(GfxAxialShading *shading):
  GfxUnivariateShading(shading)
{
  x0 = shading->x0;
  y0 = shading->y0;
  x1 = shading->x1;
  y1 = shading->y1;
}

GfxAxialShading::~GfxAxialShading() {
}

GfxAxialShading *GfxAxialShading::parse(Dict *dict, OutputDev *out, GfxState *state) {
  GfxAxialShading *shading;
  double x0A, y0A, x1A, y1A;
  double t0A, t1A;
  Function *funcsA[gfxColorMaxComps];
  int nFuncsA;
  GBool extend0A, extend1A;
  Object obj1, obj2;
  int i;

  x0A = y0A = x1A = y1A = 0;
  if (dict->lookup("Coords", &obj1)->isArray() &&
      obj1.arrayGetLength() == 4) {
    Object obj3, obj4, obj5;
    obj1.arrayGet(0, &obj2);
    obj1.arrayGet(1, &obj3);
    obj1.arrayGet(2, &obj4);
    obj1.arrayGet(3, &obj5);
    if (obj2.isNum() && obj3.isNum() && obj4.isNum() && obj5.isNum()) {
      x0A = obj2.getNum();
      y0A = obj3.getNum();
      x1A = obj4.getNum();
      y1A = obj5.getNum();
    }
    obj2.free();
    obj3.free();
    obj4.free();
    obj5.free();
  } else {
    error(errSyntaxWarning, -1, "Missing or invalid Coords in shading dictionary");
    goto err1;
  }
  obj1.free();

  t0A = 0;
  t1A = 1;
  if (dict->lookup("Domain", &obj1)->isArray() &&
      obj1.arrayGetLength() == 2) {
    Object obj3;
    obj1.arrayGet(0, &obj2);
    obj1.arrayGet(1, &obj3);
    if (obj2.isNum() && obj3.isNum()) {
      t0A = obj2.getNum();
      t1A = obj3.getNum();
    }
    obj2.free();
    obj3.free();
  }
  obj1.free();

  dict->lookup("Function", &obj1);
  if (obj1.isArray()) {
    nFuncsA = obj1.arrayGetLength();
    if (nFuncsA > gfxColorMaxComps || nFuncsA == 0) {
      error(errSyntaxWarning, -1, "Invalid Function array in shading dictionary");
      goto err1;
    }
    for (i = 0; i < nFuncsA; ++i) {
      obj1.arrayGet(i, &obj2);
      if (!(funcsA[i] = Function::parse(&obj2))) {
	obj1.free();
	obj2.free();
	goto err1;
      }
      obj2.free();
    }
  } else {
    nFuncsA = 1;
    if (!(funcsA[0] = Function::parse(&obj1))) {
      obj1.free();
      goto err1;
    }
  }
  obj1.free();

  extend0A = extend1A = gFalse;
  if (dict->lookup("Extend", &obj1)->isArray() &&
      obj1.arrayGetLength() == 2) {
    obj1.arrayGet(0, &obj2);
    if (obj2.isBool()) {
      extend0A = obj2.getBool();
    } else {
      error(errSyntaxWarning, -1, "Invalid axial shading extend (0)");
    }
    obj2.free();
    obj1.arrayGet(1, &obj2);
    if (obj2.isBool()) {
      extend1A = obj2.getBool();
    } else {
      error(errSyntaxWarning, -1, "Invalid axial shading extend (1)");
    }
    obj2.free();
  }
  obj1.free();

  shading = new GfxAxialShading(x0A, y0A, x1A, y1A, t0A, t1A,
				funcsA, nFuncsA, extend0A, extend1A);
  if (!shading->init(dict, out, state)) {
    delete shading;
    return NULL;
  }
  return shading;

 err1:
  return NULL;
}

GfxShading *GfxAxialShading::copy() {
  return new GfxAxialShading(this);
}

double GfxAxialShading::getDistance(double sMin, double sMax) {
  double xMin, yMin, xMax, yMax;

  xMin = x0 + sMin * (x1 - x0);
  yMin = y0 + sMin * (y1 - y0);
  xMax = x0 + sMax * (x1 - x0);
  yMax = y0 + sMax * (y1 - y0);

  return hypot(xMax-xMin, yMax-yMin);
}

void GfxAxialShading::getParameterRange(double *lower, double *upper,
					double xMin, double yMin,
					double xMax, double yMax) {
  double pdx, pdy, invsqnorm, tdx, tdy, t, range[2];

  // Linear gradients are orthogonal to the line passing through their
  // extremes. Because of convexity, the parameter range can be
  // computed as the convex hull (one the real line) of the parameter
  // values of the 4 corners of the box.
  //
  // The parameter value t for a point (x,y) can be computed as:
  //
  //   t = (p2 - p1) . (x,y) / |p2 - p1|^2
  //
  // t0  is the t value for the top left corner
  // tdx is the difference between left and right corners
  // tdy is the difference between top and bottom corners

  pdx = x1 - x0;
  pdy = y1 - y0;
  invsqnorm = 1.0 / (pdx * pdx + pdy * pdy);
  pdx *= invsqnorm;
  pdy *= invsqnorm;

  t = (xMin - x0) * pdx + (yMin - y0) * pdy;
  tdx = (xMax - xMin) * pdx;
  tdy = (yMax - yMin) * pdy;

  // Because of the linearity of the t value, tdx can simply be added
  // the t0 to move along the top edge. After this, *lower and *upper
  // represent the parameter range for the top edge, so extending it
  // to include the whole box simply requires adding tdy to the
  // correct extreme.

  range[0] = range[1] = t;
  if (tdx < 0)
    range[0] += tdx;
  else
    range[1] += tdx;

  if (tdy < 0)
    range[0] += tdy;
  else
    range[1] += tdy;

  *lower = std::max<double>(0., std::min<double>(1., range[0]));
  *upper = std::max<double>(0., std::min<double>(1., range[1]));
}					

//------------------------------------------------------------------------
// GfxRadialShading
//------------------------------------------------------------------------

#ifndef RADIAL_EPSILON
#define RADIAL_EPSILON	(1. / 1024 / 1024)
#endif

GfxRadialShading::GfxRadialShading(double x0A, double y0A, double r0A,
				   double x1A, double y1A, double r1A,
				   double t0A, double t1A,
				   Function **funcsA, int nFuncsA,
				   GBool extend0A, GBool extend1A):
  GfxUnivariateShading(3, t0A, t1A, funcsA, nFuncsA, extend0A, extend1A)
{
  x0 = x0A;
  y0 = y0A;
  r0 = r0A;
  x1 = x1A;
  y1 = y1A;
  r1 = r1A;
}

GfxRadialShading::GfxRadialShading(GfxRadialShading *shading):
  GfxUnivariateShading(shading)
{
  x0 = shading->x0;
  y0 = shading->y0;
  r0 = shading->r0;
  x1 = shading->x1;
  y1 = shading->y1;
  r1 = shading->r1;
}

GfxRadialShading::~GfxRadialShading() {
}

GfxRadialShading *GfxRadialShading::parse(Dict *dict, OutputDev *out, GfxState *state) {
  GfxRadialShading *shading;
  double x0A, y0A, r0A, x1A, y1A, r1A;
  double t0A, t1A;
  Function *funcsA[gfxColorMaxComps];
  int nFuncsA;
  GBool extend0A, extend1A;
  Object obj1, obj2;
  int i;

  x0A = y0A = r0A = x1A = y1A = r1A = 0;
  if (dict->lookup("Coords", &obj1)->isArray() &&
      obj1.arrayGetLength() == 6) {
    x0A = obj1.arrayGet(0, &obj2)->getNum();
    obj2.free();
    y0A = obj1.arrayGet(1, &obj2)->getNum();
    obj2.free();
    r0A = obj1.arrayGet(2, &obj2)->getNum();
    obj2.free();
    x1A = obj1.arrayGet(3, &obj2)->getNum();
    obj2.free();
    y1A = obj1.arrayGet(4, &obj2)->getNum();
    obj2.free();
    r1A = obj1.arrayGet(5, &obj2)->getNum();
    obj2.free();
  } else {
    error(errSyntaxWarning, -1, "Missing or invalid Coords in shading dictionary");
    goto err1;
  }
  obj1.free();

  t0A = 0;
  t1A = 1;
  if (dict->lookup("Domain", &obj1)->isArray() &&
      obj1.arrayGetLength() == 2) {
    t0A = obj1.arrayGet(0, &obj2)->getNum();
    obj2.free();
    t1A = obj1.arrayGet(1, &obj2)->getNum();
    obj2.free();
  }
  obj1.free();

  dict->lookup("Function", &obj1);
  if (obj1.isArray()) {
    nFuncsA = obj1.arrayGetLength();
    if (nFuncsA > gfxColorMaxComps) {
      error(errSyntaxWarning, -1, "Invalid Function array in shading dictionary");
      goto err1;
    }
    for (i = 0; i < nFuncsA; ++i) {
      obj1.arrayGet(i, &obj2);
      if (!(funcsA[i] = Function::parse(&obj2))) {
	obj1.free();
	obj2.free();
	goto err1;
      }
      obj2.free();
    }
  } else {
    nFuncsA = 1;
    if (!(funcsA[0] = Function::parse(&obj1))) {
      obj1.free();
      goto err1;
    }
  }
  obj1.free();

  extend0A = extend1A = gFalse;
  if (dict->lookup("Extend", &obj1)->isArray() &&
      obj1.arrayGetLength() == 2) {
    extend0A = obj1.arrayGet(0, &obj2)->getBool();
    obj2.free();
    extend1A = obj1.arrayGet(1, &obj2)->getBool();
    obj2.free();
  }
  obj1.free();

  shading = new GfxRadialShading(x0A, y0A, r0A, x1A, y1A, r1A, t0A, t1A,
				 funcsA, nFuncsA, extend0A, extend1A);
  if (!shading->init(dict, out, state)) {
    delete shading;
    return NULL;
  }
  return shading;

 err1:
  return NULL;
}

GfxShading *GfxRadialShading::copy() {
  return new GfxRadialShading(this);
}

double GfxRadialShading::getDistance(double sMin, double sMax) {
  double xMin, yMin, rMin, xMax, yMax, rMax;

  xMin = x0 + sMin * (x1 - x0);
  yMin = y0 + sMin * (y1 - y0);
  rMin = r0 + sMin * (r1 - r0);

  xMax = x0 + sMax * (x1 - x0);
  yMax = y0 + sMax * (y1 - y0);
  rMax = r0 + sMax * (r1 - r0);

  return hypot(xMax-xMin, yMax-yMin) + fabs(rMax-rMin);
}

// extend range, adapted from cairo, radialExtendRange
static GBool
radialExtendRange (double range[2], double value, GBool valid)
{
  if (!valid)
    range[0] = range[1] = value;
  else if (value < range[0])
    range[0] = value;
  else if (value > range[1])
    range[1] = value;

  return gTrue;
}

inline void radialEdge(double num, double den, double delta, double lower, double upper,
					   double dr, double mindr, GBool &valid, double *range)
{ 
  if (fabs (den) >= RADIAL_EPSILON) {					
    double t_edge, v;							    									
    t_edge = (num) / (den);						
    v = t_edge * (delta);						
    if (t_edge * dr >= mindr && (lower) <= v && v <= (upper))		
      valid = radialExtendRange (range, t_edge, valid);			
  }
}

inline void radialCorner1(double x, double y, double &b, double dx, double dy, double cr, 
					   double dr, double mindr, GBool &valid, double *range)
{
    b = (x) * dx + (y) * dy + cr * dr;				
    if (fabs (b) >= RADIAL_EPSILON) {				
      double t_corner;						
      double x2 = (x) * (x);					
      double y2 = (y) * (y);					
      double cr2 = (cr) * (cr);					
      double c = x2 + y2 - cr2;					
								
      t_corner = 0.5 * c / b;					
      if (t_corner * dr >= mindr)				
	valid = radialExtendRange (range, t_corner, valid);	
    }
}

inline void radialCorner2(double x, double y, double a, double &b, double &c, double &d, double dx, double dy, double cr,
					   double inva, double dr, double mindr, GBool &valid, double *range)
{
    b = (x) * dx + (y) * dy + cr * dr;				
    c = (x) * (x) + (y) * (y) - cr * cr;			
    d = b * b - a * c;						
    if (d >= 0) {						
      double t_corner;						
								
      d = sqrt (d);						
      t_corner = (b + d) * inva;				
      if (t_corner * dr >= mindr)				
	valid = radialExtendRange (range, t_corner, valid);	
      t_corner = (b - d) * inva;				
      if (t_corner * dr >= mindr)				
	valid = radialExtendRange (range, t_corner, valid);	
    }
}
void GfxRadialShading::getParameterRange(double *lower, double *upper,
					 double xMin, double yMin,
					 double xMax, double yMax) {
  double cx, cy, cr, dx, dy, dr;
  double a, x_focus, y_focus;
  double mindr, minx, miny, maxx, maxy;
  double range[2];
  GBool valid;

  // A radial pattern is considered degenerate if it can be
  // represented as a solid or clear pattern.  This corresponds to one
  // of the two cases:
  //
  // 1) The radii are both very small:
  //      |dr| < FLT_EPSILON && min (r0, r1) < FLT_EPSILON
  //
  // 2) The two circles have about the same radius and are very
  //    close to each other (approximately a cylinder gradient that
  //    doesn't move with the parameter):
  //      |dr| < FLT_EPSILON && max (|dx|, |dy|) < 2 * FLT_EPSILON

  if (xMin >= xMax || yMin >=yMax || 
      (fabs (r0 - r1) < RADIAL_EPSILON &&
       (std::min<double>(r0, r1) < RADIAL_EPSILON ||
	std::max<double>(fabs (x0 - x1), fabs (y0 - y1)) < 2 * RADIAL_EPSILON))) {
    *lower = *upper = 0;
    return;
  }

  range[0] = range[1] = 0;
  valid = gFalse;

  x_focus = y_focus = 0; // silence gcc

  cx = x0;
  cy = y0;
  cr = r0;
  dx = x1 - cx;
  dy = y1 - cy;
  dr = r1 - cr;

  // translate by -(cx, cy) to simplify computations
  xMin -= cx;
  yMin -= cy;
  xMax -= cx;
  yMax -= cy;

  // enlarge boundaries slightly to avoid rounding problems in the
  // parameter range computation
  xMin -= RADIAL_EPSILON;
  yMin -= RADIAL_EPSILON;
  xMax += RADIAL_EPSILON;
  yMax += RADIAL_EPSILON;

  // enlarge boundaries even more to avoid rounding problems when
  // testing if a point belongs to the box
  minx = xMin - RADIAL_EPSILON;
  miny = yMin - RADIAL_EPSILON;
  maxx = xMax + RADIAL_EPSILON;
  maxy = yMax + RADIAL_EPSILON;

  // we dont' allow negative radiuses, so we will be checking that
  // t*dr >= mindr to consider t valid
  mindr = -(cr + RADIAL_EPSILON);

  // After the previous transformations, the start circle is centered
  // in the origin and has radius cr. A 1-unit change in the t
  // parameter corresponds to dx,dy,dr changes in the x,y,r of the
  // circle (center coordinates, radius).
  //
  // To compute the minimum range needed to correctly draw the
  // pattern, we start with an empty range and extend it to include
  // the circles touching the bounding box or within it.
    
  // Focus, the point where the circle has radius == 0.
  //
  // r = cr + t * dr = 0
  // t = -cr / dr
  //
  // If the radius is constant (dr == 0) there is no focus (the
  // gradient represents a cylinder instead of a cone).
  if (fabs (dr) >= RADIAL_EPSILON) {
    double t_focus;

    t_focus = -cr / dr;
    x_focus = t_focus * dx;
    y_focus = t_focus * dy;
    if (minx <= x_focus && x_focus <= maxx &&
	miny <= y_focus && y_focus <= maxy)
    {
      valid = radialExtendRange (range, t_focus, valid);
    }
  }

  // Circles externally tangent to box edges.
  //
  // All circles have center in (dx, dy) * t
  //
  // If the circle is tangent to the line defined by the edge of the
  // box, then at least one of the following holds true:
  //
  //   (dx*t) + (cr + dr*t) == x0 (left   edge)
  //   (dx*t) - (cr + dr*t) == x1 (right  edge)
  //   (dy*t) + (cr + dr*t) == y0 (top    edge)
  //   (dy*t) - (cr + dr*t) == y1 (bottom edge)
  //
  // The solution is only valid if the tangent point is actually on
  // the edge, i.e. if its y coordinate is in [y0,y1] for left/right
  // edges and if its x coordinate is in [x0,x1] for top/bottom edges.
  //
  // For the first equation:
  //
  //   (dx + dr) * t = x0 - cr
  //   t = (x0 - cr) / (dx + dr)
  //   y = dy * t
  //
  // in the code this becomes:
  //
  //   t_edge = (num) / (den)
  //   v = (delta) * t_edge
  //
  // If the denominator in t is 0, the pattern is tangent to a line
  // parallel to the edge under examination. The corner-case where the
  // boundary line is the same as the edge is handled by the focus
  // point case and/or by the a==0 case.
  
  // circles tangent (externally) to left/right/top/bottom edge
  radialEdge(xMin - cr, dx + dr, dy, miny, maxy, dr, mindr, valid, range);
  radialEdge(xMax + cr, dx - dr, dy, miny, maxy, dr, mindr, valid, range);
  radialEdge(yMin - cr, dy + dr, dx, minx, maxx, dr, mindr, valid, range);
  radialEdge(yMax + cr, dy - dr, dx, minx, maxx, dr, mindr, valid, range);

  // Circles passing through a corner.
  //
  // A circle passing through the point (x,y) satisfies:
  //
  // (x-t*dx)^2 + (y-t*dy)^2 == (cr + t*dr)^2
  //
  // If we set:
  //   a = dx^2 + dy^2 - dr^2
  //   b = x*dx + y*dy + cr*dr
  //   c = x^2 + y^2 - cr^2
  // we have:
  //   a*t^2 - 2*b*t + c == 0
    
  a = dx * dx + dy * dy - dr * dr;
  if (fabs (a) < RADIAL_EPSILON * RADIAL_EPSILON) {
    double b;

    // Ensure that gradients with both a and dr small are
    // considered degenerate.
    // The floating point version of the degeneracy test implemented
    // in _radial_pattern_is_degenerate() is:
    //
    //  1) The circles are practically the same size:
    //     |dr| < RADIAL_EPSILON
    //  AND
    //  2a) The circles are both very small:
    //      min (r0, r1) < RADIAL_EPSILON
    //   OR
    //  2b) The circles are very close to each other:
    //      max (|dx|, |dy|) < 2 * RADIAL_EPSILON
    //
    // Assuming that the gradient is not degenerate, we want to
    // show that |a| < RADIAL_EPSILON^2 implies |dr| >= RADIAL_EPSILON.
    //
    // If the gradient is not degenerate yet it has |dr| <
    // RADIAL_EPSILON, (2b) is false, thus:
    //
    //   max (|dx|, |dy|) >= 2*RADIAL_EPSILON
    // which implies:
    //   4*RADIAL_EPSILON^2 <= max (|dx|, |dy|)^2 <= dx^2 + dy^2
    //
    // From the definition of a, we get:
    //   a = dx^2 + dy^2 - dr^2 < RADIAL_EPSILON^2
    //   dx^2 + dy^2 - RADIAL_EPSILON^2 < dr^2
    //   3*RADIAL_EPSILON^2 < dr^2
    //
    // which is inconsistent with the hypotheses, thus |dr| <
    // RADIAL_EPSILON is false or the gradient is degenerate.
	
    assert (fabs (dr) >= RADIAL_EPSILON);

    // If a == 0, all the circles are tangent to a line in the
    // focus point. If this line is within the box extents, we
    // should add the circle with infinite radius, but this would
    // make the range unbounded. We will be limiting the range to
    // [0,1] anyway, so we simply add the biggest legitimate
    // circle (it happens for 0 or for 1).
    if (dr < 0) {
      valid = radialExtendRange (range, 0, valid);
    } else {
      valid = radialExtendRange (range, 1, valid);
    }

    // Nondegenerate, nonlimit circles passing through the corners.
    //
    // a == 0 && a*t^2 - 2*b*t + c == 0
    //
    // t = c / (2*b)
    //
    // The b == 0 case has just been handled, so we only have to
    // compute this if b != 0.

	// circles touching each corner
	radialCorner1(xMin, yMin, b, dx, dy, cr, dr, mindr, valid, range);
	radialCorner1(xMin, yMax, b, dx, dy, cr, dr, mindr, valid, range);
	radialCorner1(xMax, yMin, b, dx, dy, cr, dr, mindr, valid, range);
	radialCorner1(xMax, yMax, b, dx, dy, cr, dr, mindr, valid, range);
  } else {
    double inva, b, c, d;

    inva = 1 / a;

    // Nondegenerate, nonlimit circles passing through the corners.
    //
    // a != 0 && a*t^2 - 2*b*t + c == 0
    //
    // t = (b +- sqrt (b*b - a*c)) / a
    //
    // If the argument of sqrt() is negative, then no circle
    // passes through the corner.

    // circles touching each corner
	radialCorner2(xMin, yMin, a, b, c, d, dx, dy, cr, inva, dr, mindr, valid, range);
	radialCorner2(xMin, yMax, a, b, c, d, dx, dy, cr, inva, dr, mindr, valid, range);
	radialCorner2(xMax, yMin, a, b, c, d, dx, dy, cr, inva, dr, mindr, valid, range);
	radialCorner2(xMax, yMax, a, b, c, d, dx, dy, cr, inva, dr, mindr, valid, range);
  }

  *lower = std::max<double>(0., std::min<double>(1., range[0]));
  *upper = std::max<double>(0., std::min<double>(1., range[1]));
}

//------------------------------------------------------------------------
// GfxShadingBitBuf
//------------------------------------------------------------------------

class GfxShadingBitBuf {
public:

  GfxShadingBitBuf(Stream *strA);
  ~GfxShadingBitBuf();
  GBool getBits(int n, Guint *val);
  void flushBits();

private:

  Stream *str;
  int bitBuf;
  int nBits;
};

GfxShadingBitBuf::GfxShadingBitBuf(Stream *strA) {
  str = strA;
  str->reset();
  bitBuf = 0;
  nBits = 0;
}

GfxShadingBitBuf::~GfxShadingBitBuf() {
  str->close();
}

GBool GfxShadingBitBuf::getBits(int n, Guint *val) {
  int x;

  if (nBits >= n) {
    x = (bitBuf >> (nBits - n)) & ((1 << n) - 1);
    nBits -= n;
  } else {
    x = 0;
    if (nBits > 0) {
      x = bitBuf & ((1 << nBits) - 1);
      n -= nBits;
      nBits = 0;
    }
    while (n > 0) {
      if ((bitBuf = str->getChar()) == EOF) {
	nBits = 0;
	return gFalse;
      }
      if (n >= 8) {
	x = (x << 8) | bitBuf;
	n -= 8;
      } else {
	x = (x << n) | (bitBuf >> (8 - n));
	nBits = 8 - n;
	n = 0;
      }
    }
  }
  *val = x;
  return gTrue;
}

void GfxShadingBitBuf::flushBits() {
  bitBuf = 0;
  nBits = 0;
}

//------------------------------------------------------------------------
// GfxGouraudTriangleShading
//------------------------------------------------------------------------

GfxGouraudTriangleShading::GfxGouraudTriangleShading(
			       int typeA,
			       GfxGouraudVertex *verticesA, int nVerticesA,
			       int (*trianglesA)[3], int nTrianglesA,
			       Function **funcsA, int nFuncsA):
  GfxShading(typeA)
{
  int i;

  vertices = verticesA;
  nVertices = nVerticesA;
  triangles = trianglesA;
  nTriangles = nTrianglesA;
  nFuncs = nFuncsA;
  for (i = 0; i < nFuncs; ++i) {
    funcs[i] = funcsA[i];
  }
}

GfxGouraudTriangleShading::GfxGouraudTriangleShading(
			       GfxGouraudTriangleShading *shading):
  GfxShading(shading)
{
  int i;

  nVertices = shading->nVertices;
  vertices = (GfxGouraudVertex *)gmallocn(nVertices, sizeof(GfxGouraudVertex));
  memcpy(vertices, shading->vertices, nVertices * sizeof(GfxGouraudVertex));
  nTriangles = shading->nTriangles;
  triangles = (int (*)[3])gmallocn(nTriangles * 3, sizeof(int));
  memcpy(triangles, shading->triangles, nTriangles * 3 * sizeof(int));
  nFuncs = shading->nFuncs;
  for (i = 0; i < nFuncs; ++i) {
    funcs[i] = shading->funcs[i]->copy();
  }
}

GfxGouraudTriangleShading::~GfxGouraudTriangleShading() {
  int i;

  gfree(vertices);
  gfree(triangles);
  for (i = 0; i < nFuncs; ++i) {
    delete funcs[i];
  }
}

GfxGouraudTriangleShading *GfxGouraudTriangleShading::parse(int typeA,
							    Dict *dict,
							    Stream *str,
							    OutputDev *out, GfxState *gfxState) {
  GfxGouraudTriangleShading *shading;
  Function *funcsA[gfxColorMaxComps];
  int nFuncsA;
  int coordBits, compBits, flagBits, vertsPerRow, nRows;
  double xMin, xMax, yMin, yMax;
  double cMin[gfxColorMaxComps], cMax[gfxColorMaxComps];
  double xMul, yMul;
  double cMul[gfxColorMaxComps];
  GfxGouraudVertex *verticesA;
  int (*trianglesA)[3];
  int nComps, nVerticesA, nTrianglesA, vertSize, triSize;
  Guint x, y, flag;
  Guint c[gfxColorMaxComps];
  GfxShadingBitBuf *bitBuf;
  Object obj1, obj2;
  int i, j, k, state;

  if (dict->lookup("BitsPerCoordinate", &obj1)->isInt()) {
    coordBits = obj1.getInt();
  } else {
    error(errSyntaxWarning, -1, "Missing or invalid BitsPerCoordinate in shading dictionary");
    goto err2;
  }
  obj1.free();
  if (dict->lookup("BitsPerComponent", &obj1)->isInt()) {
    compBits = obj1.getInt();
  } else {
    error(errSyntaxWarning, -1, "Missing or invalid BitsPerComponent in shading dictionary");
    goto err2;
  }
  obj1.free();
  flagBits = vertsPerRow = 0; // make gcc happy
  if (typeA == 4) {
    if (dict->lookup("BitsPerFlag", &obj1)->isInt()) {
      flagBits = obj1.getInt();
    } else {
      error(errSyntaxWarning, -1, "Missing or invalid BitsPerFlag in shading dictionary");
      goto err2;
    }
    obj1.free();
  } else {
    if (dict->lookup("VerticesPerRow", &obj1)->isInt()) {
      vertsPerRow = obj1.getInt();
    } else {
      error(errSyntaxWarning, -1, "Missing or invalid VerticesPerRow in shading dictionary");
      goto err2;
    }
    obj1.free();
  }
  if (dict->lookup("Decode", &obj1)->isArray() &&
      obj1.arrayGetLength() >= 6) {
    xMin = obj1.arrayGet(0, &obj2)->getNum();
    obj2.free();
    xMax = obj1.arrayGet(1, &obj2)->getNum();
    obj2.free();
    xMul = (xMax - xMin) / (pow(2.0, coordBits) - 1);
    yMin = obj1.arrayGet(2, &obj2)->getNum();
    obj2.free();
    yMax = obj1.arrayGet(3, &obj2)->getNum();
    obj2.free();
    yMul = (yMax - yMin) / (pow(2.0, coordBits) - 1);
    for (i = 0; 5 + 2*i < obj1.arrayGetLength() && i < gfxColorMaxComps; ++i) {
      cMin[i] = obj1.arrayGet(4 + 2*i, &obj2)->getNum();
      obj2.free();
      cMax[i] = obj1.arrayGet(5 + 2*i, &obj2)->getNum();
      obj2.free();
      cMul[i] = (cMax[i] - cMin[i]) / (double)((1 << compBits) - 1);
    }
    nComps = i;
  } else {
    error(errSyntaxWarning, -1, "Missing or invalid Decode array in shading dictionary");
    goto err2;
  }
  obj1.free();

  if (!dict->lookup("Function", &obj1)->isNull()) {
    if (obj1.isArray()) {
      nFuncsA = obj1.arrayGetLength();
      if (nFuncsA > gfxColorMaxComps) {
	error(errSyntaxWarning, -1, "Invalid Function array in shading dictionary");
	goto err1;
      }
      for (i = 0; i < nFuncsA; ++i) {
	obj1.arrayGet(i, &obj2);
	if (!(funcsA[i] = Function::parse(&obj2))) {
	  obj1.free();
	  obj2.free();
	  goto err1;
	}
	obj2.free();
      }
    } else {
      nFuncsA = 1;
      if (!(funcsA[0] = Function::parse(&obj1))) {
	obj1.free();
	goto err1;
      }
    }
  } else {
    nFuncsA = 0;
  }
  obj1.free();

  nVerticesA = nTrianglesA = 0;
  verticesA = NULL;
  trianglesA = NULL;
  vertSize = triSize = 0;
  state = 0;
  flag = 0; // make gcc happy
  bitBuf = new GfxShadingBitBuf(str);
  while (1) {
    if (typeA == 4) {
      if (!bitBuf->getBits(flagBits, &flag)) {
	break;
      }
    }
    if (!bitBuf->getBits(coordBits, &x) ||
	!bitBuf->getBits(coordBits, &y)) {
      break;
    }
    for (i = 0; i < nComps; ++i) {
      if (!bitBuf->getBits(compBits, &c[i])) {
	break;
      }
    }
    if (i < nComps) {
      break;
    }
    if (nVerticesA == vertSize) {
      int oldVertSize = vertSize;
      vertSize = (vertSize == 0) ? 16 : 2 * vertSize;
      verticesA = (GfxGouraudVertex *)
	              greallocn(verticesA, vertSize, sizeof(GfxGouraudVertex));
      memset(verticesA + oldVertSize, 0, (vertSize - oldVertSize) * sizeof(GfxGouraudVertex));
    }
    verticesA[nVerticesA].x = xMin + xMul * (double)x;
    verticesA[nVerticesA].y = yMin + yMul * (double)y;
    for (i = 0; i < nComps; ++i) {
      verticesA[nVerticesA].color.c[i] =
	  dblToCol(cMin[i] + cMul[i] * (double)c[i]);
    }
    ++nVerticesA;
    bitBuf->flushBits();
    if (typeA == 4) {
      if (state == 0 || state == 1) {
	++state;
      } else if (state == 2 || flag > 0) {
	if (nTrianglesA == triSize) {
	  triSize = (triSize == 0) ? 16 : 2 * triSize;
	  trianglesA = (int (*)[3])
	                   greallocn(trianglesA, triSize * 3, sizeof(int));
	}
	if (state == 2) {
	  trianglesA[nTrianglesA][0] = nVerticesA - 3;
	  trianglesA[nTrianglesA][1] = nVerticesA - 2;
	  trianglesA[nTrianglesA][2] = nVerticesA - 1;
	  ++state;
	} else if (flag == 1) {
	  trianglesA[nTrianglesA][0] = trianglesA[nTrianglesA - 1][1];
	  trianglesA[nTrianglesA][1] = trianglesA[nTrianglesA - 1][2];
	  trianglesA[nTrianglesA][2] = nVerticesA - 1;
	} else { // flag == 2
	  trianglesA[nTrianglesA][0] = trianglesA[nTrianglesA - 1][0];
	  trianglesA[nTrianglesA][1] = trianglesA[nTrianglesA - 1][2];
	  trianglesA[nTrianglesA][2] = nVerticesA - 1;
	}
	++nTrianglesA;
      } else { // state == 3 && flag == 0
	state = 1;
      }
    }
  }
  delete bitBuf;
  if (typeA == 5) {
    nRows = nVerticesA / vertsPerRow;
    nTrianglesA = (nRows - 1) * 2 * (vertsPerRow - 1);
    trianglesA = (int (*)[3])gmallocn(nTrianglesA * 3, sizeof(int));
    k = 0;
    for (i = 0; i < nRows - 1; ++i) {
      for (j = 0; j < vertsPerRow - 1; ++j) {
	trianglesA[k][0] = i * vertsPerRow + j;
	trianglesA[k][1] = i * vertsPerRow + j+1;
	trianglesA[k][2] = (i+1) * vertsPerRow + j;
	++k;
	trianglesA[k][0] = i * vertsPerRow + j+1;
	trianglesA[k][1] = (i+1) * vertsPerRow + j;
	trianglesA[k][2] = (i+1) * vertsPerRow + j+1;
	++k;
      }
    }
  }

  shading = new GfxGouraudTriangleShading(typeA, verticesA, nVerticesA,
					  trianglesA, nTrianglesA,
					  funcsA, nFuncsA);
  if (!shading->init(dict, out, gfxState)) {
    delete shading;
    return NULL;
  }
  return shading;

 err2:
  obj1.free();
 err1:
  return NULL;
}

GfxShading *GfxGouraudTriangleShading::copy() {
  return new GfxGouraudTriangleShading(this);
}

void GfxGouraudTriangleShading::getTriangle(
				    int i,
				    double *x0, double *y0, GfxColor *color0,
				    double *x1, double *y1, GfxColor *color1,
				    double *x2, double *y2, GfxColor *color2) {
  double in;
  double out[gfxColorMaxComps];
  int v, j;

  assert(!isParameterized()); 

  v = triangles[i][0];
  *x0 = vertices[v].x;
  *y0 = vertices[v].y;
  if (nFuncs > 0) {
    in = colToDbl(vertices[v].color.c[0]);
    for (j = 0; j < nFuncs; ++j) {
      funcs[j]->transform(&in, &out[j]);
    }
    for (j = 0; j < gfxColorMaxComps; ++j) {
      color0->c[j] = dblToCol(out[j]);
    }
  } else {
    *color0 = vertices[v].color;
  }
  v = triangles[i][1];
  *x1 = vertices[v].x;
  *y1 = vertices[v].y;
  if (nFuncs > 0) {
    in = colToDbl(vertices[v].color.c[0]);
    for (j = 0; j < nFuncs; ++j) {
      funcs[j]->transform(&in, &out[j]);
    }
    for (j = 0; j < gfxColorMaxComps; ++j) {
      color1->c[j] = dblToCol(out[j]);
    }
  } else {
    *color1 = vertices[v].color;
  }
  v = triangles[i][2];
  *x2 = vertices[v].x;
  *y2 = vertices[v].y;
  if (nFuncs > 0) {
    in = colToDbl(vertices[v].color.c[0]);
    for (j = 0; j < nFuncs; ++j) {
      funcs[j]->transform(&in, &out[j]);
    }
    for (j = 0; j < gfxColorMaxComps; ++j) {
      color2->c[j] = dblToCol(out[j]);
    }
  } else {
    *color2 = vertices[v].color;
  }
}

void GfxGouraudTriangleShading::getParameterizedColor(double t, GfxColor *color) {
  double out[gfxColorMaxComps];

  for (int j = 0; j < nFuncs; ++j) {
    funcs[j]->transform(&t, &out[j]);
  }
  for (int j = 0; j < gfxColorMaxComps; ++j) {
    color->c[j] = dblToCol(out[j]);
  }
}

void GfxGouraudTriangleShading::getTriangle(int i,
                                            double *x0, double *y0, double *color0,
                                            double *x1, double *y1, double *color1,
                                            double *x2, double *y2, double *color2) {
  int v;

  assert(isParameterized()); 

  v = triangles[i][0];
  *x0 = vertices[v].x;
  *y0 = vertices[v].y;
  *color0 = colToDbl(vertices[v].color.c[0]);
  v = triangles[i][1];
  *x1 = vertices[v].x;
  *y1 = vertices[v].y;
  *color1 = colToDbl(vertices[v].color.c[0]);
  v = triangles[i][2];
  *x2 = vertices[v].x;
  *y2 = vertices[v].y;
  *color2 = colToDbl(vertices[v].color.c[0]);
}

//------------------------------------------------------------------------
// GfxPatchMeshShading
//------------------------------------------------------------------------

GfxPatchMeshShading::GfxPatchMeshShading(int typeA,
					 GfxPatch *patchesA, int nPatchesA,
					 Function **funcsA, int nFuncsA):
  GfxShading(typeA)
{
  int i;

  patches = patchesA;
  nPatches = nPatchesA;
  nFuncs = nFuncsA;
  for (i = 0; i < nFuncs; ++i) {
    funcs[i] = funcsA[i];
  }
}

GfxPatchMeshShading::GfxPatchMeshShading(GfxPatchMeshShading *shading):
  GfxShading(shading)
{
  int i;

  nPatches = shading->nPatches;
  patches = (GfxPatch *)gmallocn(nPatches, sizeof(GfxPatch));
  memcpy(patches, shading->patches, nPatches * sizeof(GfxPatch));
  nFuncs = shading->nFuncs;
  for (i = 0; i < nFuncs; ++i) {
    funcs[i] = shading->funcs[i]->copy();
  }
}

GfxPatchMeshShading::~GfxPatchMeshShading() {
  int i;

  gfree(patches);
  for (i = 0; i < nFuncs; ++i) {
    delete funcs[i];
  }
}

GfxPatchMeshShading *GfxPatchMeshShading::parse(int typeA, Dict *dict,
						Stream *str, OutputDev *out, GfxState *state) {
  GfxPatchMeshShading *shading;
  Function *funcsA[gfxColorMaxComps];
  int nFuncsA;
  int coordBits, compBits, flagBits;
  double xMin, xMax, yMin, yMax;
  double cMin[gfxColorMaxComps], cMax[gfxColorMaxComps];
  double xMul, yMul;
  double cMul[gfxColorMaxComps];
  GfxPatch *patchesA, *p;
  int nComps, nPatchesA, patchesSize, nPts, nColors;
  Guint flag;
  double x[16], y[16];
  Guint xi, yi;
  double c[4][gfxColorMaxComps];
  Guint ci;
  GfxShadingBitBuf *bitBuf;
  Object obj1, obj2;
  int i, j;

  if (dict->lookup("BitsPerCoordinate", &obj1)->isInt()) {
    coordBits = obj1.getInt();
  } else {
    error(errSyntaxWarning, -1, "Missing or invalid BitsPerCoordinate in shading dictionary");
    goto err2;
  }
  obj1.free();
  if (dict->lookup("BitsPerComponent", &obj1)->isInt()) {
    compBits = obj1.getInt();
  } else {
    error(errSyntaxWarning, -1, "Missing or invalid BitsPerComponent in shading dictionary");
    goto err2;
  }
  obj1.free();
  if (dict->lookup("BitsPerFlag", &obj1)->isInt()) {
    flagBits = obj1.getInt();
  } else {
    error(errSyntaxWarning, -1, "Missing or invalid BitsPerFlag in shading dictionary");
    goto err2;
  }
  obj1.free();
  if (dict->lookup("Decode", &obj1)->isArray() &&
      obj1.arrayGetLength() >= 6) {
    xMin = obj1.arrayGet(0, &obj2)->getNum();
    obj2.free();
    xMax = obj1.arrayGet(1, &obj2)->getNum();
    obj2.free();
    xMul = (xMax - xMin) / (pow(2.0, coordBits) - 1);
    yMin = obj1.arrayGet(2, &obj2)->getNum();
    obj2.free();
    yMax = obj1.arrayGet(3, &obj2)->getNum();
    obj2.free();
    yMul = (yMax - yMin) / (pow(2.0, coordBits) - 1);
    for (i = 0; 5 + 2*i < obj1.arrayGetLength() && i < gfxColorMaxComps; ++i) {
      cMin[i] = obj1.arrayGet(4 + 2*i, &obj2)->getNum();
      obj2.free();
      cMax[i] = obj1.arrayGet(5 + 2*i, &obj2)->getNum();
      obj2.free();
      cMul[i] = (cMax[i] - cMin[i]) / (double)((1 << compBits) - 1);
    }
    nComps = i;
  } else {
    error(errSyntaxWarning, -1, "Missing or invalid Decode array in shading dictionary");
    goto err2;
  }
  obj1.free();

  if (!dict->lookup("Function", &obj1)->isNull()) {
    if (obj1.isArray()) {
      nFuncsA = obj1.arrayGetLength();
      if (nFuncsA > gfxColorMaxComps) {
	error(errSyntaxWarning, -1, "Invalid Function array in shading dictionary");
	goto err1;
      }
      for (i = 0; i < nFuncsA; ++i) {
	obj1.arrayGet(i, &obj2);
	if (!(funcsA[i] = Function::parse(&obj2))) {
	  obj1.free();
	  obj2.free();
	  goto err1;
	}
	obj2.free();
      }
    } else {
      nFuncsA = 1;
      if (!(funcsA[0] = Function::parse(&obj1))) {
	obj1.free();
	goto err1;
      }
    }
  } else {
    nFuncsA = 0;
  }
  obj1.free();

  nPatchesA = 0;
  patchesA = NULL;
  patchesSize = 0;
  bitBuf = new GfxShadingBitBuf(str);
  while (1) {
    if (!bitBuf->getBits(flagBits, &flag)) {
      break;
    }
    if (typeA == 6) {
      switch (flag) {
      case 0: nPts = 12; nColors = 4; break;
      case 1:
      case 2:
      case 3:
      default: nPts =  8; nColors = 2; break;
      }
    } else {
      switch (flag) {
      case 0: nPts = 16; nColors = 4; break;
      case 1:
      case 2:
      case 3:
      default: nPts = 12; nColors = 2; break;
      }
    }
    for (i = 0; i < nPts; ++i) {
      if (!bitBuf->getBits(coordBits, &xi) ||
	  !bitBuf->getBits(coordBits, &yi)) {
	break;
      }
      x[i] = xMin + xMul * (double)xi;
      y[i] = yMin + yMul * (double)yi;
    }
    if (i < nPts) {
      break;
    }
    for (i = 0; i < nColors; ++i) {
      for (j = 0; j < nComps; ++j) {
	if (!bitBuf->getBits(compBits, &ci)) {
	  break;
	}
	c[i][j] = cMin[j] + cMul[j] * (double)ci;
	if( nFuncsA == 0 ) {
	  // ... and colorspace values can also be stored into doubles.
	  // They will be casted later.
	  c[i][j] = dblToCol(c[i][j]);
	}
      }
      if (j < nComps) {
	break;
      }
    }
    if (i < nColors) {
      break;
    }
    if (nPatchesA == patchesSize) {
      int oldPatchesSize = patchesSize;
      patchesSize = (patchesSize == 0) ? 16 : 2 * patchesSize;
      patchesA = (GfxPatch *)greallocn(patchesA,
				       patchesSize, sizeof(GfxPatch));
      memset(patchesA + oldPatchesSize, 0, (patchesSize - oldPatchesSize) * sizeof(GfxPatch));
    }
    p = &patchesA[nPatchesA];
    if (typeA == 6) {
      switch (flag) {
      case 0:
	p->x[0][0] = x[0];
	p->y[0][0] = y[0];
	p->x[0][1] = x[1];
	p->y[0][1] = y[1];
	p->x[0][2] = x[2];
	p->y[0][2] = y[2];
	p->x[0][3] = x[3];
	p->y[0][3] = y[3];
	p->x[1][3] = x[4];
	p->y[1][3] = y[4];
	p->x[2][3] = x[5];
	p->y[2][3] = y[5];
	p->x[3][3] = x[6];
	p->y[3][3] = y[6];
	p->x[3][2] = x[7];
	p->y[3][2] = y[7];
	p->x[3][1] = x[8];
	p->y[3][1] = y[8];
	p->x[3][0] = x[9];
	p->y[3][0] = y[9];
	p->x[2][0] = x[10];
	p->y[2][0] = y[10];
	p->x[1][0] = x[11];
	p->y[1][0] = y[11];
	for (j = 0; j < nComps; ++j) {
	  p->color[0][0].c[j] = c[0][j];
	  p->color[0][1].c[j] = c[1][j];
	  p->color[1][1].c[j] = c[2][j];
	  p->color[1][0].c[j] = c[3][j];
	}
	break;
      case 1:
	if (nPatchesA == 0) {
	  goto err1;
	}
	p->x[0][0] = patchesA[nPatchesA-1].x[0][3];
	p->y[0][0] = patchesA[nPatchesA-1].y[0][3];
	p->x[0][1] = patchesA[nPatchesA-1].x[1][3];
	p->y[0][1] = patchesA[nPatchesA-1].y[1][3];
	p->x[0][2] = patchesA[nPatchesA-1].x[2][3];
	p->y[0][2] = patchesA[nPatchesA-1].y[2][3];
	p->x[0][3] = patchesA[nPatchesA-1].x[3][3];
	p->y[0][3] = patchesA[nPatchesA-1].y[3][3];
	p->x[1][3] = x[0];
	p->y[1][3] = y[0];
	p->x[2][3] = x[1];
	p->y[2][3] = y[1];
	p->x[3][3] = x[2];
	p->y[3][3] = y[2];
	p->x[3][2] = x[3];
	p->y[3][2] = y[3];
	p->x[3][1] = x[4];
	p->y[3][1] = y[4];
	p->x[3][0] = x[5];
	p->y[3][0] = y[5];
	p->x[2][0] = x[6];
	p->y[2][0] = y[6];
	p->x[1][0] = x[7];
	p->y[1][0] = y[7];
	for (j = 0; j < nComps; ++j) {
	  p->color[0][0].c[j] = patchesA[nPatchesA-1].color[0][1].c[j];
	  p->color[0][1].c[j] = patchesA[nPatchesA-1].color[1][1].c[j];
	  p->color[1][1].c[j] = c[0][j];
	  p->color[1][0].c[j] = c[1][j];
	}
	break;
      case 2:
	if (nPatchesA == 0) {
	  goto err1;
	}
	p->x[0][0] = patchesA[nPatchesA-1].x[3][3];
	p->y[0][0] = patchesA[nPatchesA-1].y[3][3];
	p->x[0][1] = patchesA[nPatchesA-1].x[3][2];
	p->y[0][1] = patchesA[nPatchesA-1].y[3][2];
	p->x[0][2] = patchesA[nPatchesA-1].x[3][1];
	p->y[0][2] = patchesA[nPatchesA-1].y[3][1];
	p->x[0][3] = patchesA[nPatchesA-1].x[3][0];
	p->y[0][3] = patchesA[nPatchesA-1].y[3][0];
	p->x[1][3] = x[0];
	p->y[1][3] = y[0];
	p->x[2][3] = x[1];
	p->y[2][3] = y[1];
	p->x[3][3] = x[2];
	p->y[3][3] = y[2];
	p->x[3][2] = x[3];
	p->y[3][2] = y[3];
	p->x[3][1] = x[4];
	p->y[3][1] = y[4];
	p->x[3][0] = x[5];
	p->y[3][0] = y[5];
	p->x[2][0] = x[6];
	p->y[2][0] = y[6];
	p->x[1][0] = x[7];
	p->y[1][0] = y[7];
	for (j = 0; j < nComps; ++j) {
	  p->color[0][0].c[j] = patchesA[nPatchesA-1].color[1][1].c[j];
	  p->color[0][1].c[j] = patchesA[nPatchesA-1].color[1][0].c[j];
	  p->color[1][1].c[j] = c[0][j];
	  p->color[1][0].c[j] = c[1][j];
	}
	break;
      case 3:
	if (nPatchesA == 0) {
	  goto err1;
	}
	p->x[0][0] = patchesA[nPatchesA-1].x[3][0];
	p->y[0][0] = patchesA[nPatchesA-1].y[3][0];
	p->x[0][1] = patchesA[nPatchesA-1].x[2][0];
	p->y[0][1] = patchesA[nPatchesA-1].y[2][0];
	p->x[0][2] = patchesA[nPatchesA-1].x[1][0];
	p->y[0][2] = patchesA[nPatchesA-1].y[1][0];
	p->x[0][3] = patchesA[nPatchesA-1].x[0][0];
	p->y[0][3] = patchesA[nPatchesA-1].y[0][0];
	p->x[1][3] = x[0];
	p->y[1][3] = y[0];
	p->x[2][3] = x[1];
	p->y[2][3] = y[1];
	p->x[3][3] = x[2];
	p->y[3][3] = y[2];
	p->x[3][2] = x[3];
	p->y[3][2] = y[3];
	p->x[3][1] = x[4];
	p->y[3][1] = y[4];
	p->x[3][0] = x[5];
	p->y[3][0] = y[5];
	p->x[2][0] = x[6];
	p->y[2][0] = y[6];
	p->x[1][0] = x[7];
	p->y[1][0] = y[7];
	for (j = 0; j < nComps; ++j) {
	  p->color[0][1].c[j] = patchesA[nPatchesA-1].color[1][0].c[j];
	  p->color[0][1].c[j] = patchesA[nPatchesA-1].color[0][0].c[j];
	  p->color[1][1].c[j] = c[0][j];
	  p->color[1][0].c[j] = c[1][j];
	}
	break;
  }
    } else {
      switch (flag) {
      case 0:
	p->x[0][0] = x[0];
	p->y[0][0] = y[0];
	p->x[0][1] = x[1];
	p->y[0][1] = y[1];
	p->x[0][2] = x[2];
	p->y[0][2] = y[2];
	p->x[0][3] = x[3];
	p->y[0][3] = y[3];
	p->x[1][3] = x[4];
	p->y[1][3] = y[4];
	p->x[2][3] = x[5];
	p->y[2][3] = y[5];
	p->x[3][3] = x[6];
	p->y[3][3] = y[6];
	p->x[3][2] = x[7];
	p->y[3][2] = y[7];
	p->x[3][1] = x[8];
	p->y[3][1] = y[8];
	p->x[3][0] = x[9];
	p->y[3][0] = y[9];
	p->x[2][0] = x[10];
	p->y[2][0] = y[10];
	p->x[1][0] = x[11];
	p->y[1][0] = y[11];
	p->x[1][1] = x[12];
	p->y[1][1] = y[12];
	p->x[1][2] = x[13];
	p->y[1][2] = y[13];
	p->x[2][2] = x[14];
	p->y[2][2] = y[14];
	p->x[2][1] = x[15];
	p->y[2][1] = y[15];
	for (j = 0; j < nComps; ++j) {
	  p->color[0][0].c[j] = c[0][j];
	  p->color[0][1].c[j] = c[1][j];
	  p->color[1][1].c[j] = c[2][j];
	  p->color[1][0].c[j] = c[3][j];
	}
	break;
      case 1:
	if (nPatchesA == 0) {
	  goto err1;
	}
	p->x[0][0] = patchesA[nPatchesA-1].x[0][3];
	p->y[0][0] = patchesA[nPatchesA-1].y[0][3];
	p->x[0][1] = patchesA[nPatchesA-1].x[1][3];
	p->y[0][1] = patchesA[nPatchesA-1].y[1][3];
	p->x[0][2] = patchesA[nPatchesA-1].x[2][3];
	p->y[0][2] = patchesA[nPatchesA-1].y[2][3];
	p->x[0][3] = patchesA[nPatchesA-1].x[3][3];
	p->y[0][3] = patchesA[nPatchesA-1].y[3][3];
	p->x[1][3] = x[0];
	p->y[1][3] = y[0];
	p->x[2][3] = x[1];
	p->y[2][3] = y[1];
	p->x[3][3] = x[2];
	p->y[3][3] = y[2];
	p->x[3][2] = x[3];
	p->y[3][2] = y[3];
	p->x[3][1] = x[4];
	p->y[3][1] = y[4];
	p->x[3][0] = x[5];
	p->y[3][0] = y[5];
	p->x[2][0] = x[6];
	p->y[2][0] = y[6];
	p->x[1][0] = x[7];
	p->y[1][0] = y[7];
	p->x[1][1] = x[8];
	p->y[1][1] = y[8];
	p->x[1][2] = x[9];
	p->y[1][2] = y[9];
	p->x[2][2] = x[10];
	p->y[2][2] = y[10];
	p->x[2][1] = x[11];
	p->y[2][1] = y[11];
	for (j = 0; j < nComps; ++j) {
	  p->color[0][0].c[j] = patchesA[nPatchesA-1].color[0][1].c[j];
	  p->color[0][1].c[j] = patchesA[nPatchesA-1].color[1][1].c[j];
	  p->color[1][1].c[j] = c[0][j];
	  p->color[1][0].c[j] = c[1][j];
	}
	break;
      case 2:
	if (nPatchesA == 0) {
	  goto err1;
	}
	p->x[0][0] = patchesA[nPatchesA-1].x[3][3];
	p->y[0][0] = patchesA[nPatchesA-1].y[3][3];
	p->x[0][1] = patchesA[nPatchesA-1].x[3][2];
	p->y[0][1] = patchesA[nPatchesA-1].y[3][2];
	p->x[0][2] = patchesA[nPatchesA-1].x[3][1];
	p->y[0][2] = patchesA[nPatchesA-1].y[3][1];
	p->x[0][3] = patchesA[nPatchesA-1].x[3][0];
	p->y[0][3] = patchesA[nPatchesA-1].y[3][0];
	p->x[1][3] = x[0];
	p->y[1][3] = y[0];
	p->x[2][3] = x[1];
	p->y[2][3] = y[1];
	p->x[3][3] = x[2];
	p->y[3][3] = y[2];
	p->x[3][2] = x[3];
	p->y[3][2] = y[3];
	p->x[3][1] = x[4];
	p->y[3][1] = y[4];
	p->x[3][0] = x[5];
	p->y[3][0] = y[5];
	p->x[2][0] = x[6];
	p->y[2][0] = y[6];
	p->x[1][0] = x[7];
	p->y[1][0] = y[7];
	p->x[1][1] = x[8];
	p->y[1][1] = y[8];
	p->x[1][2] = x[9];
	p->y[1][2] = y[9];
	p->x[2][2] = x[10];
	p->y[2][2] = y[10];
	p->x[2][1] = x[11];
	p->y[2][1] = y[11];
	for (j = 0; j < nComps; ++j) {
	  p->color[0][0].c[j] = patchesA[nPatchesA-1].color[1][1].c[j];
	  p->color[0][1].c[j] = patchesA[nPatchesA-1].color[1][0].c[j];
	  p->color[1][1].c[j] = c[0][j];
	  p->color[1][0].c[j] = c[1][j];
	}
	break;
      case 3:
	if (nPatchesA == 0) {
	  goto err1;
	}
	p->x[0][0] = patchesA[nPatchesA-1].x[3][0];
	p->y[0][0] = patchesA[nPatchesA-1].y[3][0];
	p->x[0][1] = patchesA[nPatchesA-1].x[2][0];
	p->y[0][1] = patchesA[nPatchesA-1].y[2][0];
	p->x[0][2] = patchesA[nPatchesA-1].x[1][0];
	p->y[0][2] = patchesA[nPatchesA-1].y[1][0];
	p->x[0][3] = patchesA[nPatchesA-1].x[0][0];
	p->y[0][3] = patchesA[nPatchesA-1].y[0][0];
	p->x[1][3] = x[0];
	p->y[1][3] = y[0];
	p->x[2][3] = x[1];
	p->y[2][3] = y[1];
	p->x[3][3] = x[2];
	p->y[3][3] = y[2];
	p->x[3][2] = x[3];
	p->y[3][2] = y[3];
	p->x[3][1] = x[4];
	p->y[3][1] = y[4];
	p->x[3][0] = x[5];
	p->y[3][0] = y[5];
	p->x[2][0] = x[6];
	p->y[2][0] = y[6];
	p->x[1][0] = x[7];
	p->y[1][0] = y[7];
	p->x[1][1] = x[8];
	p->y[1][1] = y[8];
	p->x[1][2] = x[9];
	p->y[1][2] = y[9];
	p->x[2][2] = x[10];
	p->y[2][2] = y[10];
	p->x[2][1] = x[11];
	p->y[2][1] = y[11];
	for (j = 0; j < nComps; ++j) {
	  p->color[0][0].c[j] = patchesA[nPatchesA-1].color[1][0].c[j];
	  p->color[0][1].c[j] = patchesA[nPatchesA-1].color[0][0].c[j];
	  p->color[1][1].c[j] = c[0][j];
	  p->color[1][0].c[j] = c[1][j];
	}
	break;
      }
    }
    ++nPatchesA;
    bitBuf->flushBits();
  }
  delete bitBuf;

  if (typeA == 6) {
    for (i = 0; i < nPatchesA; ++i) {
      p = &patchesA[i];
      p->x[1][1] = (-4 * p->x[0][0]
		    +6 * (p->x[0][1] + p->x[1][0])
		    -2 * (p->x[0][3] + p->x[3][0])
		    +3 * (p->x[3][1] + p->x[1][3])
		    - p->x[3][3]) / 9;
      p->y[1][1] = (-4 * p->y[0][0]
		    +6 * (p->y[0][1] + p->y[1][0])
		    -2 * (p->y[0][3] + p->y[3][0])
		    +3 * (p->y[3][1] + p->y[1][3])
		    - p->y[3][3]) / 9;
      p->x[1][2] = (-4 * p->x[0][3]
		    +6 * (p->x[0][2] + p->x[1][3])
		    -2 * (p->x[0][0] + p->x[3][3])
		    +3 * (p->x[3][2] + p->x[1][0])
		    - p->x[3][0]) / 9;
      p->y[1][2] = (-4 * p->y[0][3]
		    +6 * (p->y[0][2] + p->y[1][3])
		    -2 * (p->y[0][0] + p->y[3][3])
		    +3 * (p->y[3][2] + p->y[1][0])
		    - p->y[3][0]) / 9;
      p->x[2][1] = (-4 * p->x[3][0]
		    +6 * (p->x[3][1] + p->x[2][0])
		    -2 * (p->x[3][3] + p->x[0][0])
		    +3 * (p->x[0][1] + p->x[2][3])
		    - p->x[0][3]) / 9;
      p->y[2][1] = (-4 * p->y[3][0]
		    +6 * (p->y[3][1] + p->y[2][0])
		    -2 * (p->y[3][3] + p->y[0][0])
		    +3 * (p->y[0][1] + p->y[2][3])
		    - p->y[0][3]) / 9;
      p->x[2][2] = (-4 * p->x[3][3]
		    +6 * (p->x[3][2] + p->x[2][3])
		    -2 * (p->x[3][0] + p->x[0][3])
		    +3 * (p->x[0][2] + p->x[2][0])
		    - p->x[0][0]) / 9;
      p->y[2][2] = (-4 * p->y[3][3]
		    +6 * (p->y[3][2] + p->y[2][3])
		    -2 * (p->y[3][0] + p->y[0][3])
		    +3 * (p->y[0][2] + p->y[2][0])
		    - p->y[0][0]) / 9;
    }
  }

  shading = new GfxPatchMeshShading(typeA, patchesA, nPatchesA,
				    funcsA, nFuncsA);
  if (!shading->init(dict, out, state)) {
    delete shading;
    return NULL;
  }
  return shading;

 err2:
  obj1.free();
 err1:
  return NULL;
}

void GfxPatchMeshShading::getParameterizedColor(double t, GfxColor *color) {
  double out[gfxColorMaxComps];

  for (int j = 0; j < nFuncs; ++j) {
    funcs[j]->transform(&t, &out[j]);
  }
  for (int j = 0; j < gfxColorMaxComps; ++j) {
    color->c[j] = dblToCol(out[j]);
  }
}

GfxShading *GfxPatchMeshShading::copy() {
  return new GfxPatchMeshShading(this);
}

//------------------------------------------------------------------------
// GfxImageColorMap
//------------------------------------------------------------------------

GfxImageColorMap::GfxImageColorMap(int bitsA, Object *decode,
				   GfxColorSpace *colorSpaceA) {
  GfxIndexedColorSpace *indexedCS;
  GfxSeparationColorSpace *sepCS;
  int maxPixel, indexHigh;
  Guchar *indexedLookup;
  Function *sepFunc;
  Object obj;
  double x[gfxColorMaxComps];
  double y[gfxColorMaxComps];
  int i, j, k;
  double mapped;
  GBool useByteLookup;

  ok = gTrue;

  // bits per component and color space
  bits = bitsA;
  maxPixel = (1 << bits) - 1;
  colorSpace = colorSpaceA;

  // this is a hack to support 16 bits images, everywhere
  // we assume a component fits in 8 bits, with this hack
  // we treat 16 bit images as 8 bit ones until it's fixed correctly.
  // The hack has another part on ImageStream::getLine
  if (maxPixel > 255) maxPixel = 255;

  // initialize
  for (k = 0; k < gfxColorMaxComps; ++k) {
    lookup[k] = NULL;
    lookup2[k] = NULL;
  }
  byte_lookup = NULL;

  // get decode map
  if (decode->isNull()) {
    nComps = colorSpace->getNComps();
    colorSpace->getDefaultRanges(decodeLow, decodeRange, maxPixel);
  } else if (decode->isArray()) {
    nComps = decode->arrayGetLength() / 2;
    if (nComps < colorSpace->getNComps()) {
      goto err1;
    }
    if (nComps > colorSpace->getNComps()) {
      error(errSyntaxWarning, -1, "Too many elements in Decode array");
      nComps = colorSpace->getNComps();
    }
    for (i = 0; i < nComps; ++i) {
      decode->arrayGet(2*i, &obj);
      if (!obj.isNum()) {
	goto err2;
      }
      decodeLow[i] = obj.getNum();
      obj.free();
      decode->arrayGet(2*i+1, &obj);
      if (!obj.isNum()) {
	goto err2;
      }
      decodeRange[i] = obj.getNum() - decodeLow[i];
      obj.free();
    }
  } else {
    goto err1;
  }

  // Construct a lookup table -- this stores pre-computed decoded
  // values for each component, i.e., the result of applying the
  // decode mapping to each possible image pixel component value.
  for (k = 0; k < nComps; ++k) {
    lookup[k] = (GfxColorComp *)gmallocn(maxPixel + 1,
					 sizeof(GfxColorComp));
    for (i = 0; i <= maxPixel; ++i) {
      lookup[k][i] = dblToCol(decodeLow[k] +
			      (i * decodeRange[k]) / maxPixel);
    }
  }

  // Optimization: for Indexed and Separation color spaces (which have
  // only one component), we pre-compute a second lookup table with
  // color values
  colorSpace2 = NULL;
  nComps2 = 0;
  useByteLookup = gFalse;
  switch (colorSpace->getMode()) {
  case csIndexed:
    // Note that indexHigh may not be the same as maxPixel --
    // Distiller will remove unused palette entries, resulting in
    // indexHigh < maxPixel.
    indexedCS = (GfxIndexedColorSpace *)colorSpace;
    colorSpace2 = indexedCS->getBase();
    indexHigh = indexedCS->getIndexHigh();
    nComps2 = colorSpace2->getNComps();
    indexedLookup = indexedCS->getLookup();
    colorSpace2->getDefaultRanges(x, y, indexHigh);
    if (colorSpace2->useGetGrayLine() || colorSpace2->useGetRGBLine() || colorSpace2->useGetCMYKLine() || colorSpace2->useGetDeviceNLine()) {
      byte_lookup = (Guchar *)gmallocn ((maxPixel + 1), nComps2);
      useByteLookup = gTrue;
    }
    for (k = 0; k < nComps2; ++k) {
      lookup2[k] = (GfxColorComp *)gmallocn(maxPixel + 1,
					   sizeof(GfxColorComp));
      for (i = 0; i <= maxPixel; ++i) {
	j = (int)(decodeLow[0] + (i * decodeRange[0]) / maxPixel + 0.5);
	if (j < 0) {
	  j = 0;
	} else if (j > indexHigh) {
	  j = indexHigh;
	}

	mapped = x[k] + (indexedLookup[j*nComps2 + k] / 255.0) * y[k];
	lookup2[k][i] = dblToCol(mapped);
	if (useByteLookup)
	  byte_lookup[i * nComps2 + k] = (Guchar) (mapped * 255);
      }
    }
    break;
  case csSeparation:
    sepCS = (GfxSeparationColorSpace *)colorSpace;
    colorSpace2 = sepCS->getAlt();
    nComps2 = colorSpace2->getNComps();
    sepFunc = sepCS->getFunc();
    if (colorSpace2->useGetGrayLine() || colorSpace2->useGetRGBLine() || colorSpace2->useGetCMYKLine() || colorSpace2->useGetDeviceNLine()) {
      byte_lookup = (Guchar *)gmallocn ((maxPixel + 1), nComps2);
      useByteLookup = gTrue;
    }
    for (k = 0; k < nComps2; ++k) {
      lookup2[k] = (GfxColorComp *)gmallocn(maxPixel + 1,
					   sizeof(GfxColorComp));
      for (i = 0; i <= maxPixel; ++i) {
	x[0] = decodeLow[0] + (i * decodeRange[0]) / maxPixel;
	sepFunc->transform(x, y);
	lookup2[k][i] = dblToCol(y[k]);
	if (useByteLookup)
	  byte_lookup[i*nComps2 + k] = (Guchar) (y[k] * 255);
      }
    }
    break;
  default:
    if (colorSpace->useGetGrayLine() || colorSpace->useGetRGBLine() || colorSpace->useGetCMYKLine() || colorSpace->useGetDeviceNLine()) {
      byte_lookup = (Guchar *)gmallocn ((maxPixel + 1), nComps);
      useByteLookup = gTrue;
    }
    for (k = 0; k < nComps; ++k) {
      lookup2[k] = (GfxColorComp *)gmallocn(maxPixel + 1,
					   sizeof(GfxColorComp));
      for (i = 0; i <= maxPixel; ++i) {
	mapped = decodeLow[k] + (i * decodeRange[k]) / maxPixel;
	lookup2[k][i] = dblToCol(mapped);
	if (useByteLookup) {
	  int byte;

	  byte = (int) (mapped * 255.0 + 0.5);
	  if (byte < 0)
	    byte = 0;
	  else if (byte > 255)
	    byte = 255;
	  byte_lookup[i * nComps + k] = byte;
	}
      }
    }
  }

  return;

 err2:
  obj.free();
 err1:
  ok = gFalse;
}

GfxImageColorMap::GfxImageColorMap(GfxImageColorMap *colorMap) {
  int n, i, k;

  colorSpace = colorMap->colorSpace->copy();
  bits = colorMap->bits;
  nComps = colorMap->nComps;
  nComps2 = colorMap->nComps2;
  colorSpace2 = NULL;
  for (k = 0; k < gfxColorMaxComps; ++k) {
    lookup[k] = NULL;
  }
  n = 1 << bits;
  if (colorSpace->getMode() == csIndexed) {
    colorSpace2 = ((GfxIndexedColorSpace *)colorSpace)->getBase();
    for (k = 0; k < nComps2; ++k) {
      lookup[k] = (GfxColorComp *)gmallocn(n, sizeof(GfxColorComp));
      memcpy(lookup[k], colorMap->lookup[k], n * sizeof(GfxColorComp));
    }
  } else if (colorSpace->getMode() == csSeparation) {
    colorSpace2 = ((GfxSeparationColorSpace *)colorSpace)->getAlt();
    for (k = 0; k < nComps2; ++k) {
      lookup[k] = (GfxColorComp *)gmallocn(n, sizeof(GfxColorComp));
      memcpy(lookup[k], colorMap->lookup[k], n * sizeof(GfxColorComp));
    }
  } else {
    for (k = 0; k < nComps; ++k) {
      lookup[k] = (GfxColorComp *)gmallocn(n, sizeof(GfxColorComp));
      memcpy(lookup[k], colorMap->lookup[k], n * sizeof(GfxColorComp));
    }
  }
  if (colorMap->byte_lookup) {
    int nc = colorSpace2 ? nComps2 : nComps;

    byte_lookup = (Guchar *)gmallocn (n, nc);
    memcpy(byte_lookup, colorMap->byte_lookup, n * nc);
  }
  for (i = 0; i < nComps; ++i) {
    decodeLow[i] = colorMap->decodeLow[i];
    decodeRange[i] = colorMap->decodeRange[i];
  }
  ok = gTrue;
}

GfxImageColorMap::~GfxImageColorMap() {
  int i;

  delete colorSpace;
  for (i = 0; i < gfxColorMaxComps; ++i) {
    gfree(lookup[i]);
    gfree(lookup2[i]);
  }
  gfree(byte_lookup);
}

void GfxImageColorMap::getGray(Guchar *x, GfxGray *gray) {
  GfxColor color;
  int i;

  if (colorSpace2) {
    for (i = 0; i < nComps2; ++i) {
      color.c[i] = lookup2[i][x[0]];
    }
    colorSpace2->getGray(&color, gray);
  } else {
    for (i = 0; i < nComps; ++i) {
      color.c[i] = lookup2[i][x[i]];
    }
    colorSpace->getGray(&color, gray);
  }
}

void GfxImageColorMap::getRGB(Guchar *x, GfxRGB *rgb) {
  GfxColor color;
  int i;

  if (colorSpace2) {
    for (i = 0; i < nComps2; ++i) {
      color.c[i] = lookup2[i][x[0]];
    }
    colorSpace2->getRGB(&color, rgb);
  } else {
    for (i = 0; i < nComps; ++i) {
      color.c[i] = lookup2[i][x[i]];
    }
    colorSpace->getRGB(&color, rgb);
  }
}

void GfxImageColorMap::getGrayLine(Guchar *in, Guchar *out, int length) {
  int i, j;
  Guchar *inp, *tmp_line;

  if ((colorSpace2 && !colorSpace2->useGetGrayLine ()) ||
      (!colorSpace2 && !colorSpace->useGetGrayLine ())) {
    GfxGray gray;

    inp = in;
    for (i = 0; i < length; i++) {
      getGray (inp, &gray);
      out[i] = colToByte(gray);
      inp += nComps;
    }
    return;
  }

  switch (colorSpace->getMode()) {
  case csIndexed:
  case csSeparation:
    tmp_line = (Guchar *) gmallocn (length, nComps2);
    for (i = 0; i < length; i++) {
      for (j = 0; j < nComps2; j++) {
	tmp_line[i * nComps2 + j] = byte_lookup[in[i] * nComps2 + j];
      }
    }
    colorSpace2->getGrayLine(tmp_line, out, length);
    gfree (tmp_line);
    break;

  default:
    inp = in;
    for (j = 0; j < length; j++)
      for (i = 0; i < nComps; i++) {
	*inp = byte_lookup[*inp * nComps + i];
	inp++;
      }
    colorSpace->getGrayLine(in, out, length);
    break;
  }

}

void GfxImageColorMap::getRGBLine(Guchar *in, unsigned int *out, int length) {
  int i, j;
  Guchar *inp, *tmp_line;

  if (!useRGBLine()) {
    GfxRGB rgb;

    inp = in;
    for (i = 0; i < length; i++) {
      getRGB (inp, &rgb);
      out[i] =
          ((int) colToByte(rgb.r) << 16) |
          ((int) colToByte(rgb.g) << 8) |
	  ((int) colToByte(rgb.b) << 0);
      inp += nComps;
    }
    return;
  }

  switch (colorSpace->getMode()) {
  case csIndexed:
  case csSeparation:
    tmp_line = (Guchar *) gmallocn (length, nComps2);
    for (i = 0; i < length; i++) {
      for (j = 0; j < nComps2; j++) {
	tmp_line[i * nComps2 + j] = byte_lookup[in[i] * nComps2 + j];
      }
    }
    colorSpace2->getRGBLine(tmp_line, out, length);
    gfree (tmp_line);
    break;

  default:
    inp = in;
    for (j = 0; j < length; j++)
      for (i = 0; i < nComps; i++) {
	*inp = byte_lookup[*inp * nComps + i];
	inp++;
      }
    colorSpace->getRGBLine(in, out, length);
    break;
  }

}

void GfxImageColorMap::getRGBLine(Guchar *in, Guchar *out, int length) {
  int i, j;
  Guchar *inp, *tmp_line;

  if (!useRGBLine()) {
    GfxRGB rgb;

    inp = in;
    for (i = 0; i < length; i++) {
      getRGB (inp, &rgb);
      *out++ = colToByte(rgb.r);
      *out++ = colToByte(rgb.g);
      *out++ = colToByte(rgb.b);
      inp += nComps;
    }
    return;
  }

  switch (colorSpace->getMode()) {
  case csIndexed:
  case csSeparation:
    tmp_line = (Guchar *) gmallocn (length, nComps2);
    for (i = 0; i < length; i++) {
      for (j = 0; j < nComps2; j++) {
	tmp_line[i * nComps2 + j] = byte_lookup[in[i] * nComps2 + j];
      }
    }
    colorSpace2->getRGBLine(tmp_line, out, length);
    gfree (tmp_line);
    break;

  default:
    inp = in;
    for (j = 0; j < length; j++)
      for (i = 0; i < nComps; i++) {
	*inp = byte_lookup[*inp * nComps + i];
	inp++;
      }
    colorSpace->getRGBLine(in, out, length);
    break;
  }

}

void GfxImageColorMap::getRGBXLine(Guchar *in, Guchar *out, int length) {
  int i, j;
  Guchar *inp, *tmp_line;

  if (!useRGBLine()) {
    GfxRGB rgb;

    inp = in;
    for (i = 0; i < length; i++) {
      getRGB (inp, &rgb);
      *out++ = colToByte(rgb.r);
      *out++ = colToByte(rgb.g);
      *out++ = colToByte(rgb.b);
      *out++ = 255;
      inp += nComps;
    }
    return;
  }

  switch (colorSpace->getMode()) {
  case csIndexed:
  case csSeparation:
    tmp_line = (Guchar *) gmallocn (length, nComps2);
    for (i = 0; i < length; i++) {
      for (j = 0; j < nComps2; j++) {
	tmp_line[i * nComps2 + j] = byte_lookup[in[i] * nComps2 + j];
      }
    }
    colorSpace2->getRGBXLine(tmp_line, out, length);
    gfree (tmp_line);
    break;

  default:
    inp = in;
    for (j = 0; j < length; j++)
      for (i = 0; i < nComps; i++) {
	*inp = byte_lookup[*inp * nComps + i];
	inp++;
      }
    colorSpace->getRGBXLine(in, out, length);
    break;
  }

}

void GfxImageColorMap::getCMYKLine(Guchar *in, Guchar *out, int length) {
  int i, j;
  Guchar *inp, *tmp_line;

  if (!useCMYKLine()) {
    GfxCMYK cmyk;

    inp = in;
    for (i = 0; i < length; i++) {
      getCMYK (inp, &cmyk);
      *out++ = colToByte(cmyk.c);
      *out++ = colToByte(cmyk.m);
      *out++ = colToByte(cmyk.y);
      *out++ = colToByte(cmyk.k);
      inp += nComps;
    }
    return;
  }

  switch (colorSpace->getMode()) {
  case csIndexed:
  case csSeparation:
    tmp_line = (Guchar *) gmallocn (length, nComps2);
    for (i = 0; i < length; i++) {
      for (j = 0; j < nComps2; j++) {
	tmp_line[i * nComps2 + j] = byte_lookup[in[i] * nComps2 + j];
      }
    }
    colorSpace2->getCMYKLine(tmp_line, out, length);
    gfree (tmp_line);
    break;

  default:
    inp = in;
    for (j = 0; j < length; j++)
      for (i = 0; i < nComps; i++) {
	*inp = byte_lookup[*inp * nComps + i];
	inp++;
      }
    colorSpace->getCMYKLine(in, out, length);
    break;
  }

}

void GfxImageColorMap::getDeviceNLine(Guchar *in, Guchar *out, int length) {
  int i, j;
  Guchar *inp, *tmp_line;

  if (!useDeviceNLine()) {
    GfxColor deviceN;

    inp = in;
    for (i = 0; i < length; i++) {
      getDeviceN (inp, &deviceN);
      for (int j = 0; j < SPOT_NCOMPS+4; j++)
        *out++ = deviceN.c[j];
      inp += nComps;
    }
    return;
  }

  switch (colorSpace->getMode()) {
  case csIndexed:
  case csSeparation:
    tmp_line = (Guchar *) gmallocn (length, nComps2);
    for (i = 0; i < length; i++) {
      for (j = 0; j < nComps2; j++) {
	tmp_line[i * nComps2 + j] = byte_lookup[in[i] * nComps2 + j];
      }
    }
    colorSpace2->getDeviceNLine(tmp_line, out, length);
    gfree (tmp_line);
    break;

  default:
    inp = in;
    for (j = 0; j < length; j++)
      for (i = 0; i < nComps; i++) {
	*inp = byte_lookup[*inp * nComps + i];
	inp++;
      }
    colorSpace->getDeviceNLine(in, out, length);
    break;
  }

}

void GfxImageColorMap::getCMYK(Guchar *x, GfxCMYK *cmyk) {
  GfxColor color;
  int i;

  if (colorSpace2) {
    for (i = 0; i < nComps2; ++i) {
      color.c[i] = lookup2[i][x[0]];
    }
    colorSpace2->getCMYK(&color, cmyk);
  } else {
    for (i = 0; i < nComps; ++i) {
      color.c[i] = lookup[i][x[i]];
    }
    colorSpace->getCMYK(&color, cmyk);
  }
}

void GfxImageColorMap::getDeviceN(Guchar *x, GfxColor *deviceN) {
  GfxColor color;
  int i;

  if (colorSpace2) {
    for (i = 0; i < nComps2; ++i) {
      color.c[i] = lookup2[i][x[0]];
    }
    colorSpace2->getDeviceN(&color, deviceN);
  } else {
    for (i = 0; i < nComps; ++i) {
      color.c[i] = lookup[i][x[i]];
    }
    colorSpace->getDeviceN(&color, deviceN);
  }
}

void GfxImageColorMap::getColor(Guchar *x, GfxColor *color) {
  int maxPixel, i;

  maxPixel = (1 << bits) - 1;
  for (i = 0; i < nComps; ++i) {
    color->c[i] = dblToCol(decodeLow[i] + (x[i] * decodeRange[i]) / maxPixel);
  }
}

//------------------------------------------------------------------------
// GfxSubpath and GfxPath
//------------------------------------------------------------------------

GfxSubpath::GfxSubpath(double x1, double y1) {
  size = 16;
  x = (double *)gmallocn(size, sizeof(double));
  y = (double *)gmallocn(size, sizeof(double));
  curve = (GBool *)gmallocn(size, sizeof(GBool));
  n = 1;
  x[0] = x1;
  y[0] = y1;
  curve[0] = gFalse;
  closed = gFalse;
}

GfxSubpath::~GfxSubpath() {
  gfree(x);
  gfree(y);
  gfree(curve);
}

// Used for copy().
GfxSubpath::GfxSubpath(GfxSubpath *subpath) {
  size = subpath->size;
  n = subpath->n;
  x = (double *)gmallocn(size, sizeof(double));
  y = (double *)gmallocn(size, sizeof(double));
  curve = (GBool *)gmallocn(size, sizeof(GBool));
  memcpy(x, subpath->x, n * sizeof(double));
  memcpy(y, subpath->y, n * sizeof(double));
  memcpy(curve, subpath->curve, n * sizeof(GBool));
  closed = subpath->closed;
}

void GfxSubpath::lineTo(double x1, double y1) {
  if (n >= size) {
    size *= 2;
    x = (double *)greallocn(x, size, sizeof(double));
    y = (double *)greallocn(y, size, sizeof(double));
    curve = (GBool *)greallocn(curve, size, sizeof(GBool));
  }
  x[n] = x1;
  y[n] = y1;
  curve[n] = gFalse;
  ++n;
}

void GfxSubpath::curveTo(double x1, double y1, double x2, double y2,
			 double x3, double y3) {
  if (n+3 > size) {
    size *= 2;
    x = (double *)greallocn(x, size, sizeof(double));
    y = (double *)greallocn(y, size, sizeof(double));
    curve = (GBool *)greallocn(curve, size, sizeof(GBool));
  }
  x[n] = x1;
  y[n] = y1;
  x[n+1] = x2;
  y[n+1] = y2;
  x[n+2] = x3;
  y[n+2] = y3;
  curve[n] = curve[n+1] = gTrue;
  curve[n+2] = gFalse;
  n += 3;
}

void GfxSubpath::close() {
  if (x[n-1] != x[0] || y[n-1] != y[0]) {
    lineTo(x[0], y[0]);
  }
  closed = gTrue;
}

void GfxSubpath::offset(double dx, double dy) {
  int i;

  for (i = 0; i < n; ++i) {
    x[i] += dx;
    y[i] += dy;
  }
}

GfxPath::GfxPath() {
  justMoved = gFalse;
  size = 16;
  n = 0;
  firstX = firstY = 0;
  subpaths = (GfxSubpath **)gmallocn(size, sizeof(GfxSubpath *));
}

GfxPath::~GfxPath() {
  int i;

  for (i = 0; i < n; ++i)
    delete subpaths[i];
  gfree(subpaths);
}

// Used for copy().
GfxPath::GfxPath(GBool justMoved1, double firstX1, double firstY1,
		 GfxSubpath **subpaths1, int n1, int size1) {
  int i;

  justMoved = justMoved1;
  firstX = firstX1;
  firstY = firstY1;
  size = size1;
  n = n1;
  subpaths = (GfxSubpath **)gmallocn(size, sizeof(GfxSubpath *));
  for (i = 0; i < n; ++i)
    subpaths[i] = subpaths1[i]->copy();
}

void GfxPath::moveTo(double x, double y) {
  justMoved = gTrue;
  firstX = x;
  firstY = y;
}

void GfxPath::lineTo(double x, double y) {
  if (justMoved || (n > 0 && subpaths[n-1]->isClosed())) {
    if (n >= size) {
      size *= 2;
      subpaths = (GfxSubpath **)
	           greallocn(subpaths, size, sizeof(GfxSubpath *));
    }
    if (justMoved) {
      subpaths[n] = new GfxSubpath(firstX, firstY);
    } else {
      subpaths[n] = new GfxSubpath(subpaths[n-1]->getLastX(),
				   subpaths[n-1]->getLastY());
    }
    ++n;
    justMoved = gFalse;
  }
  subpaths[n-1]->lineTo(x, y);
}

void GfxPath::curveTo(double x1, double y1, double x2, double y2,
	     double x3, double y3) {
  if (justMoved || (n > 0 && subpaths[n-1]->isClosed())) {
    if (n >= size) {
      size *= 2;
      subpaths = (GfxSubpath **) 
 	         greallocn(subpaths, size, sizeof(GfxSubpath *));
    }
    if (justMoved) {
      subpaths[n] = new GfxSubpath(firstX, firstY);
    } else {
      subpaths[n] = new GfxSubpath(subpaths[n-1]->getLastX(),
				   subpaths[n-1]->getLastY());
    }
    ++n;
    justMoved = gFalse;
  }
  subpaths[n-1]->curveTo(x1, y1, x2, y2, x3, y3);
}

void GfxPath::close() {
  // this is necessary to handle the pathological case of
  // moveto/closepath/clip, which defines an empty clipping region
  if (justMoved) {
    if (n >= size) {
      size *= 2;
      subpaths = (GfxSubpath **)
	greallocn(subpaths, size, sizeof(GfxSubpath *));
    }
    subpaths[n] = new GfxSubpath(firstX, firstY);
    ++n;
    justMoved = gFalse;
  }
  subpaths[n-1]->close();
}

void GfxPath::append(GfxPath *path) {
  int i;

  if (n + path->n > size) {
    size = n + path->n;
    subpaths = (GfxSubpath **)
                 greallocn(subpaths, size, sizeof(GfxSubpath *));
  }
  for (i = 0; i < path->n; ++i) {
    subpaths[n++] = path->subpaths[i]->copy();
  }
  justMoved = gFalse;
}

void GfxPath::offset(double dx, double dy) {
  int i;

  for (i = 0; i < n; ++i) {
    subpaths[i]->offset(dx, dy);
  }
}

//------------------------------------------------------------------------
// GfxState
//------------------------------------------------------------------------
GfxState::ReusablePathIterator::ReusablePathIterator(GfxPath *path)
 : path(path),
   subPathOff(0),
   coordOff(0),
   numCoords(0),
   curSubPath(NULL)
{
  if( path->getNumSubpaths() ) {
    curSubPath = path->getSubpath(subPathOff);
    numCoords = curSubPath->getNumPoints();
  }
}

bool GfxState::ReusablePathIterator::isEnd() const {
   return coordOff >= numCoords;
}

void GfxState::ReusablePathIterator::next() {
  ++coordOff;
  if (coordOff == numCoords) {
    ++subPathOff;
    if (subPathOff < path->getNumSubpaths()) {
      coordOff = 0;
      curSubPath = path->getSubpath(subPathOff);
      numCoords = curSubPath->getNumPoints();
    }
  }
}

void GfxState::ReusablePathIterator::setCoord(double x, double y) {
  curSubPath->setX(coordOff, x);
  curSubPath->setY(coordOff, y);
}

void GfxState::ReusablePathIterator::reset() {
  coordOff = 0;
  subPathOff = 0;
  curSubPath = path->getSubpath(0);
  numCoords = curSubPath->getNumPoints();
}

GfxState::GfxState(double hDPIA, double vDPIA, PDFRectangle *pageBox,
		   int rotateA, GBool upsideDown) {
  double kx, ky;

  hDPI = hDPIA;
  vDPI = vDPIA;
  rotate = rotateA;
  px1 = pageBox->x1;
  py1 = pageBox->y1;
  px2 = pageBox->x2;
  py2 = pageBox->y2;
  kx = hDPI / 72.0;
  ky = vDPI / 72.0;
  if (rotate == 90) {
    ctm[0] = 0;
    ctm[1] = upsideDown ? ky : -ky;
    ctm[2] = kx;
    ctm[3] = 0;
    ctm[4] = -kx * py1;
    ctm[5] = ky * (upsideDown ? -px1 : px2);
    pageWidth = kx * (py2 - py1);
    pageHeight = ky * (px2 - px1);
  } else if (rotate == 180) {
    ctm[0] = -kx;
    ctm[1] = 0;
    ctm[2] = 0;
    ctm[3] = upsideDown ? ky : -ky;
    ctm[4] = kx * px2;
    ctm[5] = ky * (upsideDown ? -py1 : py2);
    pageWidth = kx * (px2 - px1);
    pageHeight = ky * (py2 - py1);
  } else if (rotate == 270) {
    ctm[0] = 0;
    ctm[1] = upsideDown ? -ky : ky;
    ctm[2] = -kx;
    ctm[3] = 0;
    ctm[4] = kx * py2;
    ctm[5] = ky * (upsideDown ? px2 : -px1);
    pageWidth = kx * (py2 - py1);
    pageHeight = ky * (px2 - px1);
  } else {
    ctm[0] = kx;
    ctm[1] = 0;
    ctm[2] = 0;
    ctm[3] = upsideDown ? -ky : ky;
    ctm[4] = -kx * px1;
    ctm[5] = ky * (upsideDown ? py2 : -py1);
    pageWidth = kx * (px2 - px1);
    pageHeight = ky * (py2 - py1);
  }

  fillColorSpace = new GfxDeviceGrayColorSpace();
  strokeColorSpace = new GfxDeviceGrayColorSpace();
  fillColor.c[0] = 0;
  strokeColor.c[0] = 0;
  fillPattern = NULL;
  strokePattern = NULL;
  blendMode = gfxBlendNormal;
  fillOpacity = 1;
  strokeOpacity = 1;
  fillOverprint = gFalse;
  strokeOverprint = gFalse;
  overprintMode = 0;
  transfer[0] = transfer[1] = transfer[2] = transfer[3] = NULL;

  lineWidth = 1;
  lineDash = NULL;
  lineDashLength = 0;
  lineDashStart = 0;
  flatness = 1;
  lineJoin = 0;
  lineCap = 0;
  miterLimit = 10;
  strokeAdjust = gFalse;
  alphaIsShape = gFalse;
  textKnockout = gFalse;

  font = NULL;
  fontSize = 0;
  textMat[0] = 1; textMat[1] = 0;
  textMat[2] = 0; textMat[3] = 1;
  textMat[4] = 0; textMat[5] = 0;
  charSpace = 0;
  wordSpace = 0;
  horizScaling = 1;
  leading = 0;
  rise = 0;
  render = 0;

  path = new GfxPath();
  curX = curY = 0;
  lineX = lineY = 0;

  clipXMin = 0;
  clipYMin = 0;
  clipXMax = pageWidth;
  clipYMax = pageHeight;

  renderingIntent[0] = 0;

  saved = NULL;
#ifdef USE_CMS
  GfxColorSpace::setupColorProfiles();
  XYZ2DisplayTransformRelCol = NULL;
  XYZ2DisplayTransformAbsCol = NULL;
  XYZ2DisplayTransformSat = NULL;
  XYZ2DisplayTransformPerc = NULL;
  localDisplayProfile = NULL;
  displayProfileRef = 0;
#endif
}

GfxState::~GfxState() {
  int i;

  if (fillColorSpace) {
    delete fillColorSpace;
  }
  if (strokeColorSpace) {
    delete strokeColorSpace;
  }
  if (fillPattern) {
    delete fillPattern;
  }
  if (strokePattern) {
    delete strokePattern;
  }
  for (i = 0; i < 4; ++i) {
    if (transfer[i]) {
      delete transfer[i];
    }
  }
  gfree(lineDash);
  if (path) {
    // this gets set to NULL by restore()
    delete path;
  }
  if (font) {
    font->decRefCnt();
  }
#ifdef USE_CMS
  if (XYZ2DisplayTransformRelCol) {
    if (XYZ2DisplayTransformRelCol->unref() == 0)
      delete XYZ2DisplayTransformRelCol;
  }
  if (XYZ2DisplayTransformAbsCol) {
    if (XYZ2DisplayTransformAbsCol->unref() == 0)
      delete XYZ2DisplayTransformAbsCol;
  }
  if (XYZ2DisplayTransformSat) {
    if (XYZ2DisplayTransformSat->unref() == 0)
      delete XYZ2DisplayTransformSat;
  }
  if (XYZ2DisplayTransformPerc) {
    if (XYZ2DisplayTransformPerc->unref() == 0)
      delete XYZ2DisplayTransformPerc;
  }
  if (--displayProfileRef == 0 && localDisplayProfile != NULL) {
    cmsCloseProfile(localDisplayProfile);
  }
#endif
}

// Used for copy();
GfxState::GfxState(GfxState *state, GBool copyPath) {
  int i;

  memcpy(this, state, sizeof(GfxState));
  if (fillColorSpace) {
    fillColorSpace = state->fillColorSpace->copy();
  }
  if (strokeColorSpace) {
    strokeColorSpace = state->strokeColorSpace->copy();
  }
  if (fillPattern) {
    fillPattern = state->fillPattern->copy();
  }
  if (strokePattern) {
    strokePattern = state->strokePattern->copy();
  }
  for (i = 0; i < 4; ++i) {
    if (transfer[i]) {
      transfer[i] = state->transfer[i]->copy();
    }
  }
  if (lineDashLength > 0) {
    lineDash = (double *)gmallocn(lineDashLength, sizeof(double));
    memcpy(lineDash, state->lineDash, lineDashLength * sizeof(double));
  }
  if (font)
    font->incRefCnt();

  if (copyPath) {
    path = state->path->copy();
  }
  saved = NULL;
#ifdef USE_CMS
  if (XYZ2DisplayTransformRelCol) {
    XYZ2DisplayTransformRelCol->ref();
  }
  if (XYZ2DisplayTransformAbsCol) {
    XYZ2DisplayTransformAbsCol->ref();
  }
  if (XYZ2DisplayTransformSat) {
    XYZ2DisplayTransformSat->ref();
  }
  if (XYZ2DisplayTransformPerc) {
    XYZ2DisplayTransformPerc->ref();
  }
  if (localDisplayProfile) {
    displayProfileRef++;
  }
#endif
}

#ifdef USE_CMS
void GfxState::setDisplayProfile(cmsHPROFILE localDisplayProfileA) {
  if (localDisplayProfile != NULL) {
    cmsCloseProfile(localDisplayProfile);
  }
  localDisplayProfile = localDisplayProfileA;
  if (localDisplayProfileA != NULL) {
    cmsHTRANSFORM transform;
    unsigned int nChannels;
    unsigned int localDisplayPixelType;

    localDisplayPixelType = getCMSColorSpaceType(cmsGetColorSpace(localDisplayProfile));
    nChannels = getCMSNChannels(cmsGetColorSpace(localDisplayProfile));
    displayProfileRef = 1;
    // create transform from XYZ
    cmsHPROFILE XYZProfile = cmsCreateXYZProfile();
    if ((transform = cmsCreateTransform(XYZProfile, TYPE_XYZ_DBL,
	   localDisplayProfile,
	   COLORSPACE_SH(localDisplayPixelType) |
	     CHANNELS_SH(nChannels) | BYTES_SH(1),
	  INTENT_RELATIVE_COLORIMETRIC,LCMS_FLAGS)) == 0) {
      error(errSyntaxWarning, -1, "Can't create Lab transform");
    } else {
      XYZ2DisplayTransformRelCol = new GfxColorTransform(transform, INTENT_RELATIVE_COLORIMETRIC, localDisplayPixelType);
    }
    if ((transform = cmsCreateTransform(XYZProfile, TYPE_XYZ_DBL,
	   localDisplayProfile,
	   COLORSPACE_SH(localDisplayPixelType) |
	     CHANNELS_SH(nChannels) | BYTES_SH(1),
	  INTENT_ABSOLUTE_COLORIMETRIC,LCMS_FLAGS)) == 0) {
      error(errSyntaxWarning, -1, "Can't create Lab transform");
    } else {
      XYZ2DisplayTransformAbsCol = new GfxColorTransform(transform, INTENT_ABSOLUTE_COLORIMETRIC, localDisplayPixelType);
    }
    if ((transform = cmsCreateTransform(XYZProfile, TYPE_XYZ_DBL,
	   localDisplayProfile,
	   COLORSPACE_SH(localDisplayPixelType) |
	     CHANNELS_SH(nChannels) | BYTES_SH(1),
	  INTENT_SATURATION,LCMS_FLAGS)) == 0) {
      error(errSyntaxWarning, -1, "Can't create Lab transform");
    } else {
      XYZ2DisplayTransformSat = new GfxColorTransform(transform, INTENT_SATURATION, localDisplayPixelType);
    }
    if ((transform = cmsCreateTransform(XYZProfile, TYPE_XYZ_DBL,
	   localDisplayProfile,
	   COLORSPACE_SH(localDisplayPixelType) |
	     CHANNELS_SH(nChannels) | BYTES_SH(1),
	  INTENT_PERCEPTUAL,LCMS_FLAGS)) == 0) {
      error(errSyntaxWarning, -1, "Can't create Lab transform");
    } else {
      XYZ2DisplayTransformPerc = new GfxColorTransform(transform, INTENT_PERCEPTUAL, localDisplayPixelType);
    }
    cmsCloseProfile(XYZProfile);
  }
}

GfxColorTransform *GfxState::getXYZ2DisplayTransform() {
  GfxColorTransform *transform;

  transform = XYZ2DisplayTransformRelCol;
  if (renderingIntent != NULL) {
    if (strcmp(renderingIntent, "AbsoluteColorimetric") == 0) {
      transform = XYZ2DisplayTransformAbsCol;
    } else if (strcmp(renderingIntent, "Saturation") == 0) {
      transform = XYZ2DisplayTransformSat;
    } else if (strcmp(renderingIntent, "Perceptual") == 0) {
      transform = XYZ2DisplayTransformPerc;
    }
  }
  if (transform == NULL) {
    transform = XYZ2DisplayTransform;
  }
  return transform;
}

#endif

void GfxState::setPath(GfxPath *pathA) {
  delete path;
  path = pathA;
}

void GfxState::getUserClipBBox(double *xMin, double *yMin,
			       double *xMax, double *yMax) {
  double ictm[6];
  double xMin1, yMin1, xMax1, yMax1, det, tx, ty;

  // invert the CTM
  det = 1 / (ctm[0] * ctm[3] - ctm[1] * ctm[2]);
  ictm[0] = ctm[3] * det;
  ictm[1] = -ctm[1] * det;
  ictm[2] = -ctm[2] * det;
  ictm[3] = ctm[0] * det;
  ictm[4] = (ctm[2] * ctm[5] - ctm[3] * ctm[4]) * det;
  ictm[5] = (ctm[1] * ctm[4] - ctm[0] * ctm[5]) * det;

  // transform all four corners of the clip bbox; find the min and max
  // x and y values
  xMin1 = xMax1 = clipXMin * ictm[0] + clipYMin * ictm[2] + ictm[4];
  yMin1 = yMax1 = clipXMin * ictm[1] + clipYMin * ictm[3] + ictm[5];
  tx = clipXMin * ictm[0] + clipYMax * ictm[2] + ictm[4];
  ty = clipXMin * ictm[1] + clipYMax * ictm[3] + ictm[5];
  if (tx < xMin1) {
    xMin1 = tx;
  } else if (tx > xMax1) {
    xMax1 = tx;
  }
  if (ty < yMin1) {
    yMin1 = ty;
  } else if (ty > yMax1) {
    yMax1 = ty;
  }
  tx = clipXMax * ictm[0] + clipYMin * ictm[2] + ictm[4];
  ty = clipXMax * ictm[1] + clipYMin * ictm[3] + ictm[5];
  if (tx < xMin1) {
    xMin1 = tx;
  } else if (tx > xMax1) {
    xMax1 = tx;
  }
  if (ty < yMin1) {
    yMin1 = ty;
  } else if (ty > yMax1) {
    yMax1 = ty;
  }
  tx = clipXMax * ictm[0] + clipYMax * ictm[2] + ictm[4];
  ty = clipXMax * ictm[1] + clipYMax * ictm[3] + ictm[5];
  if (tx < xMin1) {
    xMin1 = tx;
  } else if (tx > xMax1) {
    xMax1 = tx;
  }
  if (ty < yMin1) {
    yMin1 = ty;
  } else if (ty > yMax1) {
    yMax1 = ty;
  }

  *xMin = xMin1;
  *yMin = yMin1;
  *xMax = xMax1;
  *yMax = yMax1;
}

double GfxState::transformWidth(double w) {
  double x, y;

  x = ctm[0] + ctm[2];
  y = ctm[1] + ctm[3];
  return w * sqrt(0.5 * (x * x + y * y));
}

double GfxState::getTransformedFontSize() {
  double x1, y1, x2, y2;

  x1 = textMat[2] * fontSize;
  y1 = textMat[3] * fontSize;
  x2 = ctm[0] * x1 + ctm[2] * y1;
  y2 = ctm[1] * x1 + ctm[3] * y1;
  return sqrt(x2 * x2 + y2 * y2);
}

void GfxState::getFontTransMat(double *m11, double *m12,
			       double *m21, double *m22) {
  *m11 = (textMat[0] * ctm[0] + textMat[1] * ctm[2]) * fontSize;
  *m12 = (textMat[0] * ctm[1] + textMat[1] * ctm[3]) * fontSize;
  *m21 = (textMat[2] * ctm[0] + textMat[3] * ctm[2]) * fontSize;
  *m22 = (textMat[2] * ctm[1] + textMat[3] * ctm[3]) * fontSize;
}

void GfxState::setCTM(double a, double b, double c,
		      double d, double e, double f) {
  ctm[0] = a;
  ctm[1] = b;
  ctm[2] = c;
  ctm[3] = d;
  ctm[4] = e;
  ctm[5] = f;
}

void GfxState::concatCTM(double a, double b, double c,
			 double d, double e, double f) {
  double a1 = ctm[0];
  double b1 = ctm[1];
  double c1 = ctm[2];
  double d1 = ctm[3];

  ctm[0] = a * a1 + b * c1;
  ctm[1] = a * b1 + b * d1;
  ctm[2] = c * a1 + d * c1;
  ctm[3] = c * b1 + d * d1;
  ctm[4] = e * a1 + f * c1 + ctm[4];
  ctm[5] = e * b1 + f * d1 + ctm[5];
}

void GfxState::shiftCTMAndClip(double tx, double ty) {
  ctm[4] += tx;
  ctm[5] += ty;
  clipXMin += tx;
  clipYMin += ty;
  clipXMax += tx;
  clipYMax += ty;
}

void GfxState::setFillColorSpace(GfxColorSpace *colorSpace) {
  if (fillColorSpace) {
    delete fillColorSpace;
  }
  fillColorSpace = colorSpace;
}

void GfxState::setStrokeColorSpace(GfxColorSpace *colorSpace) {
  if (strokeColorSpace) {
    delete strokeColorSpace;
  }
  strokeColorSpace = colorSpace;
}

void GfxState::setFillPattern(GfxPattern *pattern) {
  if (fillPattern) {
    delete fillPattern;
  }
  fillPattern = pattern;
}

void GfxState::setStrokePattern(GfxPattern *pattern) {
  if (strokePattern) {
    delete strokePattern;
  }
  strokePattern = pattern;
}

void GfxState::setFont(GfxFont *fontA, double fontSizeA) {
  if (font)
    font->decRefCnt();

  font = fontA;
  fontSize = fontSizeA;
}

void GfxState::setTransfer(Function **funcs) {
  int i;

  for (i = 0; i < 4; ++i) {
    if (transfer[i]) {
      delete transfer[i];
    }
    transfer[i] = funcs[i];
  }
}

void GfxState::setLineDash(double *dash, int length, double start) {
  if (lineDash)
    gfree(lineDash);
  lineDash = dash;
  lineDashLength = length;
  lineDashStart = start;
}

void GfxState::clearPath() {
  delete path;
  path = new GfxPath();
}

void GfxState::clip() {
  double xMin, yMin, xMax, yMax, x, y;
  GfxSubpath *subpath;
  int i, j;

  xMin = xMax = yMin = yMax = 0; // make gcc happy
  for (i = 0; i < path->getNumSubpaths(); ++i) {
    subpath = path->getSubpath(i);
    for (j = 0; j < subpath->getNumPoints(); ++j) {
      transform(subpath->getX(j), subpath->getY(j), &x, &y);
      if (i == 0 && j == 0) {
	xMin = xMax = x;
	yMin = yMax = y;
      } else {
	if (x < xMin) {
	  xMin = x;
	} else if (x > xMax) {
	  xMax = x;
	}
	if (y < yMin) {
	  yMin = y;
	} else if (y > yMax) {
	  yMax = y;
	}
      }
    }
  }
  if (xMin > clipXMin) {
    clipXMin = xMin;
  }
  if (yMin > clipYMin) {
    clipYMin = yMin;
  }
  if (xMax < clipXMax) {
    clipXMax = xMax;
  }
  if (yMax < clipYMax) {
    clipYMax = yMax;
  }
}

void GfxState::clipToStrokePath() {
  double xMin, yMin, xMax, yMax, x, y, t0, t1;
  GfxSubpath *subpath;
  int i, j;

  xMin = xMax = yMin = yMax = 0; // make gcc happy
  for (i = 0; i < path->getNumSubpaths(); ++i) {
    subpath = path->getSubpath(i);
    for (j = 0; j < subpath->getNumPoints(); ++j) {
      transform(subpath->getX(j), subpath->getY(j), &x, &y);
      if (i == 0 && j == 0) {
	xMin = xMax = x;
	yMin = yMax = y;
      } else {
	if (x < xMin) {
	  xMin = x;
	} else if (x > xMax) {
	  xMax = x;
	}
	if (y < yMin) {
	  yMin = y;
	} else if (y > yMax) {
	  yMax = y;
	}
      }
    }
  }

  // allow for the line width
  //~ miter joins can extend farther than this
  t0 = fabs(ctm[0]);
  t1 = fabs(ctm[2]);
  if (t0 > t1) {
    xMin -= 0.5 * lineWidth * t0;
    xMax += 0.5 * lineWidth * t0;
  } else {
    xMin -= 0.5 * lineWidth * t1;
    xMax += 0.5 * lineWidth * t1;
  }
  t0 = fabs(ctm[0]);
  t1 = fabs(ctm[3]);
  if (t0 > t1) {
    yMin -= 0.5 * lineWidth * t0;
    yMax += 0.5 * lineWidth * t0;
  } else {
    yMin -= 0.5 * lineWidth * t1;
    yMax += 0.5 * lineWidth * t1;
  }

  if (xMin > clipXMin) {
    clipXMin = xMin;
  }
  if (yMin > clipYMin) {
    clipYMin = yMin;
  }
  if (xMax < clipXMax) {
    clipXMax = xMax;
  }
  if (yMax < clipYMax) {
    clipYMax = yMax;
  }
}

void GfxState::clipToRect(double xMin, double yMin, double xMax, double yMax) {
  double x, y, xMin1, yMin1, xMax1, yMax1;

  transform(xMin, yMin, &x, &y);
  xMin1 = xMax1 = x;
  yMin1 = yMax1 = y;
  transform(xMax, yMin, &x, &y);
  if (x < xMin1) {
    xMin1 = x;
  } else if (x > xMax1) {
    xMax1 = x;
  }
  if (y < yMin1) {
    yMin1 = y;
  } else if (y > yMax1) {
    yMax1 = y;
  }
  transform(xMax, yMax, &x, &y);
  if (x < xMin1) {
    xMin1 = x;
  } else if (x > xMax1) {
    xMax1 = x;
  }
  if (y < yMin1) {
    yMin1 = y;
  } else if (y > yMax1) {
    yMax1 = y;
  }
  transform(xMin, yMax, &x, &y);
  if (x < xMin1) {
    xMin1 = x;
  } else if (x > xMax1) {
    xMax1 = x;
  }
  if (y < yMin1) {
    yMin1 = y;
  } else if (y > yMax1) {
    yMax1 = y;
  }

  if (xMin1 > clipXMin) {
    clipXMin = xMin1;
  }
  if (yMin1 > clipYMin) {
    clipYMin = yMin1;
  }
  if (xMax1 < clipXMax) {
    clipXMax = xMax1;
  }
  if (yMax1 < clipYMax) {
    clipYMax = yMax1;
  }
}

void GfxState::textShift(double tx, double ty) {
  double dx, dy;

  textTransformDelta(tx, ty, &dx, &dy);
  curX += dx;
  curY += dy;
}

void GfxState::shift(double dx, double dy) {
  curX += dx;
  curY += dy;
}

GfxState *GfxState::save() {
  GfxState *newState;

  newState = copy();
  newState->saved = this;
  return newState;
}

GfxState *GfxState::restore() {
  GfxState *oldState;

  if (saved) {
    oldState = saved;

    // these attributes aren't saved/restored by the q/Q operators
    oldState->path = path;
    oldState->curX = curX;
    oldState->curY = curY;
    oldState->lineX = lineX;
    oldState->lineY = lineY;

    path = NULL;
    saved = NULL;
    delete this;

  } else {
    oldState = this;
  }

  return oldState;
}

GBool GfxState::parseBlendMode(Object *obj, GfxBlendMode *mode) {
  Object obj2;
  int i, j;

  if (obj->isName()) {
    for (i = 0; i < nGfxBlendModeNames; ++i) {
      if (!strcmp(obj->getName(), gfxBlendModeNames[i].name)) {
	*mode = gfxBlendModeNames[i].mode;
	return gTrue;
      }
    }
    return gFalse;
  } else if (obj->isArray()) {
    for (i = 0; i < obj->arrayGetLength(); ++i) {
      obj->arrayGet(i, &obj2);
      if (!obj2.isName()) {
	obj2.free();
	return gFalse;
      }
      for (j = 0; j < nGfxBlendModeNames; ++j) {
	if (!strcmp(obj2.getName(), gfxBlendModeNames[j].name)) {
	  obj2.free();
	  *mode = gfxBlendModeNames[j].mode;
	  return gTrue;
	}
      }
      obj2.free();
    }
    *mode = gfxBlendNormal;
    return gTrue;
  } else {
    return gFalse;
  }
}
