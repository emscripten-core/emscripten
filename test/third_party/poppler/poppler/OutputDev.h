//========================================================================
//
// OutputDev.h
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
// Copyright (C) 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright (C) 2006 Thorkild Stray <thorkild@ifi.uio.no>
// Copyright (C) 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2007 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2009, 2010 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef OUTPUTDEV_H
#define OUTPUTDEV_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "poppler-config.h"
#include "goo/gtypes.h"
#include "CharTypes.h"
#include "Object.h"

class Dict;
class GooHash;
class GooString;
class GfxState;
struct GfxColor;
class GfxColorSpace;
class GfxImageColorMap;
class GfxFunctionShading;
class GfxAxialShading;
class GfxGouraudTriangleShading;
class GfxPatchMeshShading;
class GfxRadialShading;
class Stream;
class Links;
class Link;
class Catalog;
class Page;
class Function;

//------------------------------------------------------------------------
// OutputDev
//------------------------------------------------------------------------

class OutputDev {
public:

  // Constructor.
  OutputDev() { profileHash = NULL; }

  // Destructor.
  virtual ~OutputDev() {}

  //----- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() = 0;

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() = 0;

  // Does this device use tilingPatternFill()?  If this returns false,
  // tiling pattern fills will be reduced to a series of other drawing
  // operations.
  virtual GBool useTilingPatternFill() { return gFalse; }

  // Does this device support specific shading types?
  // see gouraudTriangleShadedFill() and patchMeshShadedFill()
  virtual GBool useShadedFills(int type) { return gFalse; }

  // Does this device use FillColorStop()?
  virtual GBool useFillColorStop() { return gFalse; }

  // Does this device use drawForm()?  If this returns false,
  // form-type XObjects will be interpreted (i.e., unrolled).
  virtual GBool useDrawForm() { return gFalse; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars() = 0;

  // Does this device need non-text content?
  virtual GBool needNonText() { return gTrue; }

  // If current colorspace ist pattern,
  // does this device support text in pattern colorspace?
  // Default is false
  virtual GBool supportTextCSPattern(GfxState * /*state*/) { return gFalse; }

  // If current colorspace ist pattern,
  // need this device special handling for masks in pattern colorspace?
  // Default is false
  virtual GBool fillMaskCSPattern(GfxState * /*state*/) { return gFalse; }
  virtual void endMaskClip(GfxState * /*state*/) {}

  //----- initialization and control

  // Set default transform matrix.
  virtual void setDefaultCTM(double *ctm);

  // Check to see if a page slice should be displayed.  If this
  // returns false, the page display is aborted.  Typically, an
  // OutputDev will use some alternate means to display the page
  // before returning false.
  virtual GBool checkPageSlice(Page *page, double hDPI, double vDPI,
			       int rotate, GBool useMediaBox, GBool crop,
			       int sliceX, int sliceY, int sliceW, int sliceH,
			       GBool printing, Catalog * catalog,
			       GBool (* abortCheckCbk)(void *data) = NULL,
			       void * abortCheckCbkData = NULL)
    { return gTrue; }

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state) {}

  // End a page.
  virtual void endPage() {}

  // Dump page contents to display.
  virtual void dump() {}

  //----- coordinate conversion

  // Convert between device and user coordinates.
  virtual void cvtDevToUser(double dx, double dy, double *ux, double *uy);
  virtual void cvtUserToDev(double ux, double uy, int *dx, int *dy);

  double *getDefCTM() { return defCTM; }
  double *getDefICTM() { return defICTM; }

  //----- save/restore graphics state
  virtual void saveState(GfxState * /*state*/) {}
  virtual void restoreState(GfxState * /*state*/) {}

  //----- update graphics state
  virtual void updateAll(GfxState *state);
  virtual void updateCTM(GfxState * /*state*/, double /*m11*/, double /*m12*/,
			 double /*m21*/, double /*m22*/, double /*m31*/, double /*m32*/) {}
  virtual void updateLineDash(GfxState * /*state*/) {}
  virtual void updateFlatness(GfxState * /*state*/) {}
  virtual void updateLineJoin(GfxState * /*state*/) {}
  virtual void updateLineCap(GfxState * /*state*/) {}
  virtual void updateMiterLimit(GfxState * /*state*/) {}
  virtual void updateLineWidth(GfxState * /*state*/) {}
  virtual void updateStrokeAdjust(GfxState * /*state*/) {}
  virtual void updateAlphaIsShape(GfxState * /*state*/) {}
  virtual void updateTextKnockout(GfxState * /*state*/) {}
  virtual void updateFillColorSpace(GfxState * /*state*/) {}
  virtual void updateStrokeColorSpace(GfxState * /*state*/) {}
  virtual void updateFillColor(GfxState * /*state*/) {}
  virtual void updateStrokeColor(GfxState * /*state*/) {}
  virtual void updateBlendMode(GfxState * /*state*/) {}
  virtual void updateFillOpacity(GfxState * /*state*/) {}
  virtual void updateStrokeOpacity(GfxState * /*state*/) {}
  virtual void updateFillOverprint(GfxState * /*state*/) {}
  virtual void updateStrokeOverprint(GfxState * /*state*/) {}
  virtual void updateTransfer(GfxState * /*state*/) {}
  virtual void updateFillColorStop(GfxState * /*state*/, double /*offset*/) {}

  //----- update text state
  virtual void updateFont(GfxState * /*state*/) {}
  virtual void updateTextMat(GfxState * /*state*/) {}
  virtual void updateCharSpace(GfxState * /*state*/) {}
  virtual void updateRender(GfxState * /*state*/) {}
  virtual void updateRise(GfxState * /*state*/) {}
  virtual void updateWordSpace(GfxState * /*state*/) {}
  virtual void updateHorizScaling(GfxState * /*state*/) {}
  virtual void updateTextPos(GfxState * /*state*/) {}
  virtual void updateTextShift(GfxState * /*state*/, double /*shift*/) {}

  //----- path painting
  virtual void stroke(GfxState * /*state*/) {}
  virtual void fill(GfxState * /*state*/) {}
  virtual void eoFill(GfxState * /*state*/) {}
  virtual GBool tilingPatternFill(GfxState * /*state*/, Object * /*str*/,
				  int /*paintType*/, Dict * /*resDict*/,
				  double * /*mat*/, double * /*bbox*/,
				  int /*x0*/, int /*y0*/, int /*x1*/, int /*y1*/,
				  double /*xStep*/, double /*yStep*/)
    { return gFalse; }
  virtual GBool functionShadedFill(GfxState * /*state*/,
				   GfxFunctionShading * /*shading*/)
    { return gFalse; }
  virtual GBool axialShadedFill(GfxState * /*state*/, GfxAxialShading * /*shading*/, double /*tMin*/, double /*tMax*/)
    { return gFalse; }
  virtual GBool axialShadedSupportExtend(GfxState * /*state*/, GfxAxialShading * /*shading*/)
    { return gFalse; }
  virtual GBool radialShadedFill(GfxState * /*state*/, GfxRadialShading * /*shading*/, double /*sMin*/, double /*sMax*/)
    { return gFalse; }
  virtual GBool radialShadedSupportExtend(GfxState * /*state*/, GfxRadialShading * /*shading*/)
    { return gFalse; }
  virtual GBool gouraudTriangleShadedFill(GfxState *state, GfxGouraudTriangleShading *shading)
    { return gFalse; }
  virtual GBool patchMeshShadedFill(GfxState *state, GfxPatchMeshShading *shading)
    { return gFalse; }

  //----- path clipping
  virtual void clip(GfxState * /*state*/) {}
  virtual void eoClip(GfxState * /*state*/) {}
  virtual void clipToStrokePath(GfxState * /*state*/) {}

  //----- text drawing
  virtual void beginStringOp(GfxState * /*state*/) {}
  virtual void endStringOp(GfxState * /*state*/) {}
  virtual void beginString(GfxState * /*state*/, GooString * /*s*/) {}
  virtual void endString(GfxState * /*state*/) {}
  virtual void drawChar(GfxState * /*state*/, double /*x*/, double /*y*/,
			double /*dx*/, double /*dy*/,
			double /*originX*/, double /*originY*/,
			CharCode /*code*/, int /*nBytes*/, Unicode * /*u*/, int /*uLen*/) {}
  virtual void drawString(GfxState * /*state*/, GooString * /*s*/) {}
  virtual GBool beginType3Char(GfxState * /*state*/, double /*x*/, double /*y*/,
			       double /*dx*/, double /*dy*/,
			       CharCode /*code*/, Unicode * /*u*/, int /*uLen*/);
  virtual void endType3Char(GfxState * /*state*/) {}
  virtual void beginTextObject(GfxState * /*state*/) {}
  virtual GBool deviceHasTextClip(GfxState * /*state*/) { return gFalse; }
  virtual void endTextObject(GfxState * /*state*/) {}

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
			     int width, int height, GBool invert, GBool interpolate,
			     GBool inlineImg);
  virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			 int width, int height, GfxImageColorMap *colorMap,
			 GBool interpolate, int *maskColors, GBool inlineImg);
  virtual void drawMaskedImage(GfxState *state, Object *ref, Stream *str,
			       int width, int height,
			       GfxImageColorMap *colorMap, GBool interpolate,
			       Stream *maskStr, int maskWidth, int maskHeight,
			       GBool maskInvert, GBool maskInterpolate);
  virtual void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
				   int width, int height,
				   GfxImageColorMap *colorMap,
				   GBool interpolate,
				   Stream *maskStr,
				   int maskWidth, int maskHeight,
				   GfxImageColorMap *maskColorMap,
				   GBool maskInterpolate);

  //----- grouping operators

  virtual void endMarkedContent(GfxState *state);
  virtual void beginMarkedContent(char *name, Dict *properties);
  virtual void markPoint(char *name);
  virtual void markPoint(char *name, Dict *properties);
  
  

#if OPI_SUPPORT
  //----- OPI functions
  virtual void opiBegin(GfxState *state, Dict *opiDict);
  virtual void opiEnd(GfxState *state, Dict *opiDict);
#endif

  //----- Type 3 font operators
  virtual void type3D0(GfxState * /*state*/, double /*wx*/, double /*wy*/) {}
  virtual void type3D1(GfxState * /*state*/, double /*wx*/, double /*wy*/,
		       double /*llx*/, double /*lly*/, double /*urx*/, double /*ury*/) {}

  //----- form XObjects
  virtual void drawForm(Ref /*id*/) {}

  //----- PostScript XObjects
  virtual void psXObject(Stream * /*psStream*/, Stream * /*level1Stream*/) {}

  //----- Profiling
  virtual void startProfile();
  virtual GooHash *getProfileHash() {return profileHash; }
  virtual GooHash *endProfile();

  //----- transparency groups and soft masks
  virtual void beginTransparencyGroup(GfxState * /*state*/, double * /*bbox*/,
				      GfxColorSpace * /*blendingColorSpace*/,
				      GBool /*isolated*/, GBool /*knockout*/,
				      GBool /*forSoftMask*/) {}
  virtual void endTransparencyGroup(GfxState * /*state*/) {}
  virtual void paintTransparencyGroup(GfxState * /*state*/, double * /*bbox*/) {}
  virtual void setSoftMask(GfxState * /*state*/, double * /*bbox*/, GBool /*alpha*/,
			   Function * /*transferFunc*/, GfxColor * /*backdropColor*/) {}
  virtual void clearSoftMask(GfxState * /*state*/) {}

  //----- links
  virtual void processLink(Link * /*link*/, Catalog * /*catalog*/) {}

#if 1 //~tmp: turn off anti-aliasing temporarily
  virtual GBool getVectorAntialias() { return gFalse; }
  virtual void setVectorAntialias(GBool /*vaa*/) {}
#endif

private:

  double defCTM[6];		// default coordinate transform matrix
  double defICTM[6];		// inverse of default CTM
  GooHash *profileHash;
};

#endif
