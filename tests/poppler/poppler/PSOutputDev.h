//========================================================================
//
// PSOutputDev.h
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
// Copyright (C) 2005 Martin Kretzschmar <martink@gnome.org>
// Copyright (C) 2005 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2006-2008, 2012, 2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007 Brad Hards <bradh@kde.org>
// Copyright (C) 2009-2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2009 Till Kamppeter <till.kamppeter@gmail.com>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009, 2011 William Bader <williambader@hotmail.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2011 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef PSOUTPUTDEV_H
#define PSOUTPUTDEV_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "poppler-config.h"
#include <stddef.h>
#include "Object.h"
#include "GfxState.h"
#include "GlobalParams.h"
#include "OutputDev.h"
#include <set>

class GHooash;
class PDFDoc;
class XRef;
class Function;
class GfxPath;
class GfxFont;
class GfxColorSpace;
class GfxSeparationColorSpace;
class PDFRectangle;
struct PST1FontName;
struct PSFont8Info;
struct PSFont16Enc;
class PSOutCustomColor;
class PSOutputDev;

//------------------------------------------------------------------------
// PSOutputDev
//------------------------------------------------------------------------

enum PSOutMode {
  psModePS,
  psModeEPS,
  psModeForm,
  psModePSOrigPageSizes
};

enum PSFileType {
  psFile,			// write to file
  psPipe,			// write to pipe
  psStdout,			// write to stdout
  psGeneric			// write to a generic stream
};

enum PSOutCustomCodeLocation {
  psOutCustomDocSetup,
  psOutCustomPageSetup
};

typedef void (*PSOutputFunc)(void *stream, const char *data, int len);

typedef GooString *(*PSOutCustomCodeCbk)(PSOutputDev *psOut,
				       PSOutCustomCodeLocation loc, int n, 
				       void *data);

class PSOutputDev: public OutputDev {
public:

  // Open a PostScript output file, and write the prolog.
  PSOutputDev(const char *fileName, PDFDoc *docA,
	      char *psTitle,
	      int firstPage, int lastPage, PSOutMode modeA,
	      int paperWidthA = -1, int paperHeightA = -1,
	      GBool duplexA = gTrue,
	      int imgLLXA = 0, int imgLLYA = 0,
	      int imgURXA = 0, int imgURYA = 0,
	      GBool forceRasterizeA = gFalse,
	      GBool manualCtrlA = gFalse,
	      PSOutCustomCodeCbk customCodeCbkA = NULL,
	      void *customCodeCbkDataA = NULL);

  // Open a PSOutputDev that will write to a generic stream.
  PSOutputDev(PSOutputFunc outputFuncA, void *outputStreamA,
	      char *psTitle,
	      PDFDoc *docA,
	      int firstPage, int lastPage, PSOutMode modeA,
	      int paperWidthA = -1, int paperHeightA = -1,
	      GBool duplexA = gTrue,
	      int imgLLXA = 0, int imgLLYA = 0,
	      int imgURXA = 0, int imgURYA = 0,
	      GBool forceRasterizeA = gFalse,
	      GBool manualCtrlA = gFalse,
	      PSOutCustomCodeCbk customCodeCbkA = NULL,
	      void *customCodeCbkDataA = NULL);

  // Destructor -- writes the trailer and closes the file.
  virtual ~PSOutputDev();

  // Check if file was successfully created.
  virtual GBool isOk() { return ok; }

  //---- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gFalse; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() { return gFalse; }

  // Does this device use tilingPatternFill()?  If this returns false,
  // tiling pattern fills will be reduced to a series of other drawing
  // operations.
  virtual GBool useTilingPatternFill() { return gTrue; }

  // Does this device use functionShadedFill(), axialShadedFill(), and
  // radialShadedFill()?  If this returns false, these shaded fills
  // will be reduced to a series of other drawing operations.
  virtual GBool useShadedFills(int type)
    { return type < 4 && level >= psLevel2; }

  // Does this device use drawForm()?  If this returns false,
  // form-type XObjects will be interpreted (i.e., unrolled).
  virtual GBool useDrawForm() { return preload; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars() { return gFalse; }
  
  virtual GBool needClipToCropBox() { return mode == psModeEPS; }

  //----- header/trailer (used only if manualCtrl is true)

  // Write the document-level header.
  void writeHeader(int firstPage, int lastPage,
		   PDFRectangle *mediaBox, PDFRectangle *cropBox,
		   int pageRotate, char *pstitle);

  // Write the Xpdf procset.
  void writeXpdfProcset();

  // Write the trailer for the current page.
  void writePageTrailer();

  // Write the document trailer.
  void writeTrailer();

  //----- initialization and control

  // Check to see if a page slice should be displayed.  If this
  // returns false, the page display is aborted.  Typically, an
  // OutputDev will use some alternate means to display the page
  // before returning false.
  virtual GBool checkPageSlice(Page *page, double hDPI, double vDPI,
			       int rotate, GBool useMediaBox, GBool crop,
			       int sliceX, int sliceY, int sliceW, int sliceH,
			       GBool printing,
			       GBool (*abortCheckCbk)(void *data) = NULL,
			       void *abortCheckCbkData = NULL,
			       GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data) = NULL,
			       void *annotDisplayDecideCbkData = NULL);

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state, XRef *xref);

  // End a page.
  virtual void endPage();

  //----- save/restore graphics state
  virtual void saveState(GfxState *state);
  virtual void restoreState(GfxState *state);

  //----- update graphics state
  virtual void updateCTM(GfxState *state, double m11, double m12,
			 double m21, double m22, double m31, double m32);
  virtual void updateLineDash(GfxState *state);
  virtual void updateFlatness(GfxState *state);
  virtual void updateLineJoin(GfxState *state);
  virtual void updateLineCap(GfxState *state);
  virtual void updateMiterLimit(GfxState *state);
  virtual void updateLineWidth(GfxState *state);
  virtual void updateFillColorSpace(GfxState *state);
  virtual void updateStrokeColorSpace(GfxState *state);
  virtual void updateFillColor(GfxState *state);
  virtual void updateStrokeColor(GfxState *state);
  virtual void updateFillOverprint(GfxState *state);
  virtual void updateStrokeOverprint(GfxState *state);
  virtual void updateOverprintMode(GfxState *state);
  virtual void updateTransfer(GfxState *state);

  //----- update text state
  virtual void updateFont(GfxState *state);
  virtual void updateTextMat(GfxState *state);
  virtual void updateCharSpace(GfxState *state);
  virtual void updateRender(GfxState *state);
  virtual void updateRise(GfxState *state);
  virtual void updateWordSpace(GfxState *state);
  virtual void updateHorizScaling(GfxState *state);
  virtual void updateTextPos(GfxState *state);
  virtual void updateTextShift(GfxState *state, double shift);
  virtual void saveTextPos(GfxState *state);
  virtual void restoreTextPos(GfxState *state);

  //----- path painting
  virtual void stroke(GfxState *state);
  virtual void fill(GfxState *state);
  virtual void eoFill(GfxState *state);
  virtual GBool tilingPatternFill(GfxState *state, Gfx *gfx, Catalog *cat, Object *str,
				  double *pmat, int paintType, int tilingType, Dict *resDict,
				  double *mat, double *bbox,
				  int x0, int y0, int x1, int y1,
				  double xStep, double yStep);
  virtual GBool functionShadedFill(GfxState *state,
				   GfxFunctionShading *shading);
  virtual GBool axialShadedFill(GfxState *state, GfxAxialShading *shading, double /*tMin*/, double /*tMax*/);
  virtual GBool radialShadedFill(GfxState *state, GfxRadialShading *shading, double /*sMin*/, double /*sMax*/);

  //----- path clipping
  virtual void clip(GfxState *state);
  virtual void eoClip(GfxState *state);
  virtual void clipToStrokePath(GfxState *state);

  //----- text drawing
  virtual void drawString(GfxState *state, GooString *s);
  virtual void beginTextObject(GfxState *state);
  virtual void endTextObject(GfxState *state);

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
			     int width, int height, GBool invert,
			     GBool interpolate, GBool inlineImg);
  virtual void setSoftMaskFromImageMask(GfxState *state,
					Object *ref, Stream *str,
					int width, int height, GBool invert,
					GBool inlineImg, double *baseMatrix);
  virtual void unsetSoftMaskFromImageMask(GfxState *state, double *baseMatrix);
  virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			 int width, int height, GfxImageColorMap *colorMap,
			 GBool interpolate, int *maskColors, GBool inlineImg);
  virtual void drawMaskedImage(GfxState *state, Object *ref, Stream *str,
			       int width, int height,
			       GfxImageColorMap *colorMap,
			       GBool interpolate,
			       Stream *maskStr, int maskWidth, int maskHeight,
			       GBool maskInvert, GBool maskInterpolate);

#if OPI_SUPPORT
  //----- OPI functions
  virtual void opiBegin(GfxState *state, Dict *opiDict);
  virtual void opiEnd(GfxState *state, Dict *opiDict);
#endif

  //----- Type 3 font operators
  virtual void type3D0(GfxState *state, double wx, double wy);
  virtual void type3D1(GfxState *state, double wx, double wy,
		       double llx, double lly, double urx, double ury);

  //----- form XObjects
  virtual void drawForm(Ref ref);

  //----- PostScript XObjects
  virtual void psXObject(Stream *psStream, Stream *level1Stream);

  //----- miscellaneous
  void setOffset(double x, double y)
    { tx0 = x; ty0 = y; }
  void setScale(double x, double y)
    { xScale0 = x; yScale0 = y; }
  void setRotate(int rotateA)
    { rotate0 = rotateA; }
  void setClip(double llx, double lly, double urx, double ury)
    { clipLLX0 = llx; clipLLY0 = lly; clipURX0 = urx; clipURY0 = ury; }
  void setUnderlayCbk(void (*cbk)(PSOutputDev *psOut, void *data),
		      void *data)
    { underlayCbk = cbk; underlayCbkData = data; }
  void setOverlayCbk(void (*cbk)(PSOutputDev *psOut, void *data),
		     void *data)
    { overlayCbk = cbk; overlayCbkData = data; }
  void setDisplayText(GBool display) { displayText = display; }

private:

  void init(PSOutputFunc outputFuncA, void *outputStreamA,
	    PSFileType fileTypeA, char *pstitle, PDFDoc *doc,
	    int firstPage, int lastPage, PSOutMode modeA,
	    int imgLLXA, int imgLLYA, int imgURXA, int imgURYA,
	    GBool manualCtrlA, int paperWidthA, int paperHeightA,
            GBool duplexA);
  void setupResources(Dict *resDict);
  void setupFonts(Dict *resDict);
  void setupFont(GfxFont *font, Dict *parentResDict);
  void setupEmbeddedType1Font(Ref *id, GooString *psName);
  void setupExternalType1Font(GooString *fileName, GooString *psName);
  void setupEmbeddedType1CFont(GfxFont *font, Ref *id, GooString *psName);
  void setupEmbeddedOpenTypeT1CFont(GfxFont *font, Ref *id, GooString *psName);
  void setupEmbeddedTrueTypeFont(GfxFont *font, Ref *id, GooString *psName);
  void setupExternalTrueTypeFont(GfxFont *font, GooString *fileName,
				 GooString *psName);
  void setupEmbeddedCIDType0Font(GfxFont *font, Ref *id, GooString *psName);
  void setupEmbeddedCIDTrueTypeFont(GfxFont *font, Ref *id, GooString *psName,
				    GBool needVerticalMetrics);
  void setupExternalCIDTrueTypeFont(GfxFont *font,
				    GooString *fileName,
				    GooString *psName,
				    GBool needVerticalMetrics);
  void setupEmbeddedOpenTypeCFFFont(GfxFont *font, Ref *id, GooString *psName);
  void setupType3Font(GfxFont *font, GooString *psName, Dict *parentResDict);
  GooString *makePSFontName(GfxFont *font, Ref *id);
  void setupImages(Dict *resDict);
  void setupImage(Ref id, Stream *str, GBool mask);
  void setupForms(Dict *resDict);
  void setupForm(Ref id, Object *strObj);
  void addProcessColor(double c, double m, double y, double k);
  void addCustomColor(GfxSeparationColorSpace *sepCS);
  void doPath(GfxPath *path);
  void maskToClippingPath(Stream *maskStr, int maskWidth, int maskHeight, GBool maskInvert);
  void doImageL1(Object *ref, GfxImageColorMap *colorMap,
		 GBool invert, GBool inlineImg,
		 Stream *str, int width, int height, int len,
		 int *maskColors, Stream *maskStr,
		 int maskWidth, int maskHeight, GBool maskInvert);
  void doImageL1Sep(Object *ref, GfxImageColorMap *colorMap,
		    GBool invert, GBool inlineImg,
		    Stream *str, int width, int height, int len,
		    int *maskColors, Stream *maskStr,
		    int maskWidth, int maskHeight, GBool maskInvert);
  void doImageL2(Object *ref, GfxImageColorMap *colorMap,
		 GBool invert, GBool inlineImg,
		 Stream *str, int width, int height, int len,
		 int *maskColors, Stream *maskStr,
		 int maskWidth, int maskHeight, GBool maskInvert);
  void doImageL3(Object *ref, GfxImageColorMap *colorMap,
		 GBool invert, GBool inlineImg,
		 Stream *str, int width, int height, int len,
		 int *maskColors, Stream *maskStr,
		 int maskWidth, int maskHeight, GBool maskInvert);
  void dumpColorSpaceL2(GfxColorSpace *colorSpace,
			GBool genXform, GBool updateColors,
			GBool map01);
  GBool tilingPatternFillL1(GfxState *state, Catalog *cat, Object *str,
			    double *pmat, int paintType, int tilingType, Dict *resDict,
			    double *mat, double *bbox,
			    int x0, int y0, int x1, int y1,
			    double xStep, double yStep);
  GBool tilingPatternFillL2(GfxState *state, Catalog *cat, Object *str,
			    double *pmat, int paintType, int tilingType, Dict *resDict,
			    double *mat, double *bbox,
			    int x0, int y0, int x1, int y1,
			    double xStep, double yStep);

#if OPI_SUPPORT
  void opiBegin20(GfxState *state, Dict *dict);
  void opiBegin13(GfxState *state, Dict *dict);
  void opiTransform(GfxState *state, double x0, double y0,
		    double *x1, double *y1);
#endif
  void cvtFunction(Function *func, GBool invertPSFunction = gFalse);
  GooString *filterPSName(GooString *name);

  // Write the document-level setup.
  void writeDocSetup(PDFDoc *doc, Catalog *catalog, int firstPage, int lastPage, GBool duplexA);

  void writePSChar(char c);
  void writePS(const char *s);
  void writePSBuf(const char *s, int len);
  void writePSFmt(const char *fmt, ...);
  void writePSString(GooString *s);
  void writePSName(const char *s);
  GooString *filterPSLabel(GooString *label, GBool *needParens=0);
  void writePSTextLine(GooString *s);

  PSLevel level;		// PostScript level (1, 2, separation)
  PSOutMode mode;		// PostScript mode (PS, EPS, form)
  int paperWidth;		// width of paper, in pts
  int paperHeight;		// height of paper, in pts
  GBool paperMatch;		// true if paper size is set to match each page
  int prevWidth;		// width of previous page
                                // (only psModePSOrigPageSizes output mode)
  int prevHeight;		// height of previous page
                                // (only psModePSOrigPageSizes output mode)
  int imgLLX, imgLLY,		// imageable area, in pts
      imgURX, imgURY;
  GBool preload;		// load all images into memory, and
				//   predefine forms

  PSOutputFunc outputFunc;
  void *outputStream;
  PSFileType fileType;		// file / pipe / stdout
  GBool manualCtrl;
  int seqPage;			// current sequential page number
  void (*underlayCbk)(PSOutputDev *psOut, void *data);
  void *underlayCbkData;
  void (*overlayCbk)(PSOutputDev *psOut, void *data);
  void *overlayCbkData;
  GooString *(*customCodeCbk)(PSOutputDev *psOut,
			    PSOutCustomCodeLocation loc, int n, 
			    void *data);
  void *customCodeCbkData;

  PDFDoc *doc;
  XRef *xref;			// the xref table for this PDF file

  Ref *fontIDs;			// list of object IDs of all used fonts
  int fontIDLen;		// number of entries in fontIDs array
  int fontIDSize;		// size of fontIDs array
  std::set<int> resourceIDs;	// list of object IDs of objects containing Resources we've already set up
  GooHash *fontNames;		// all used font names
  PST1FontName *t1FontNames;	// font names for Type 1/1C fonts
  int t1FontNameLen;		// number of entries in t1FontNames array
  int t1FontNameSize;		// size of t1FontNames array
  PSFont8Info *font8Info;	// info for 8-bit fonts
  int font8InfoLen;		// number of entries in font8Info array
  int font8InfoSize;		// size of font8Info array
  PSFont16Enc *font16Enc;	// encodings for substitute 16-bit fonts
  int font16EncLen;		// number of entries in font16Enc array
  int font16EncSize;		// size of font16Enc array
  Ref *imgIDs;			// list of image IDs for in-memory images
  int imgIDLen;			// number of entries in imgIDs array
  int imgIDSize;		// size of imgIDs array
  Ref *formIDs;			// list of IDs for predefined forms
  int formIDLen;		// number of entries in formIDs array
  int formIDSize;		// size of formIDs array
  int numSaves;			// current number of gsaves
  int numTilingPatterns;	// current number of nested tiling patterns
  int nextFunc;			// next unique number to use for a function

  GooList *paperSizes;		// list of used paper sizes, if paperMatch
				//   is true [PSOutPaperSize]
  double tx0, ty0;		// global translation
  double xScale0, yScale0;	// global scaling
  int rotate0;			// rotation angle (0, 90, 180, 270)
  double clipLLX0, clipLLY0,
         clipURX0, clipURY0;
  double tx, ty;		// global translation for current page
  double xScale, yScale;	// global scaling for current page
  int rotate;			// rotation angle for current page
  double epsX1, epsY1,		// EPS bounding box (unrotated)
         epsX2, epsY2;

  GooString *embFontList;		// resource comments for embedded fonts

  int processColors;		// used process colors
  PSOutCustomColor		// used custom colors
    *customColors;

  GBool haveTextClip;		// set if text has been drawn with a
				//   clipping render mode

  GBool inType3Char;		// inside a Type 3 CharProc
  GBool inUncoloredPattern;     // inside a uncolored pattern (PaintType = 2)
  GooString *t3String;		// Type 3 content string
  double t3WX, t3WY,		// Type 3 character parameters
         t3LLX, t3LLY, t3URX, t3URY;
  GBool t3FillColorOnly;	// operators should only use the fill color
  GBool t3Cacheable;		// cleared if char is not cacheable
  GBool t3NeedsRestore;		// set if a 'q' operator was issued
  GBool forceRasterize;		// forces the page to be rasterized into a image before printing
  GBool displayText;		// displayText

#if OPI_SUPPORT
  int opi13Nest;		// nesting level of OPI 1.3 objects
  int opi20Nest;		// nesting level of OPI 2.0 objects
#endif

  GBool ok;			// set up ok?

  friend class WinPDFPrinter;
};

#endif
