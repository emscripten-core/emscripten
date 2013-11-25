//========================================================================
//
// Gfx.h
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
// Copyright (C) 2007 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright (C) 2008 Brad Hards <bradh@kde.org>
// Copyright (C) 2008, 2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009-2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009, 2010, 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2010 David Benjamin <davidben@mit.edu>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2013 Fabio D'Urso <fabiodurso@hotmail.it>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef GFX_H
#define GFX_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "poppler-config.h"
#include "goo/gtypes.h"
#include "goo/GooList.h"
#include "GfxState.h"
#include "Object.h"
#include "PopplerCache.h"

#include <vector>

class GooString;
class PDFDoc;
class XRef;
class Array;
class Stream;
class Parser;
class Dict;
class Function;
class OutputDev;
class GfxFontDict;
class GfxFont;
class GfxPattern;
class GfxTilingPattern;
class GfxShadingPattern;
class GfxShading;
class GfxFunctionShading;
class GfxAxialShading;
class GfxRadialShading;
class GfxGouraudTriangleShading;
class GfxPatchMeshShading;
struct GfxPatch;
class GfxState;
struct GfxColor;
class GfxColorSpace;
class Gfx;
class PDFRectangle;
class AnnotBorder;
class AnnotColor;
class Catalog;
struct MarkedContentStack;

//------------------------------------------------------------------------

enum GfxClipType {
  clipNone,
  clipNormal,
  clipEO
};

enum TchkType {
  tchkBool,			// boolean
  tchkInt,			// integer
  tchkNum,			// number (integer or real)
  tchkString,			// string
  tchkName,			// name
  tchkArray,			// array
  tchkProps,			// properties (dictionary or name)
  tchkSCN,			// scn/SCN args (number of name)
  tchkNone			// used to avoid empty initializer lists
};

#define maxArgs 33

struct Operator {
  char name[4];
  int numArgs;
  TchkType tchk[maxArgs];
  void (Gfx::*func)(Object args[], int numArgs);
};

//------------------------------------------------------------------------

class GfxResources {
public:

  GfxResources(XRef *xref, Dict *resDict, GfxResources *nextA);
  ~GfxResources();

  GfxFont *lookupFont(char *name);
  GBool lookupXObject(char *name, Object *obj);
  GBool lookupXObjectNF(char *name, Object *obj);
  GBool lookupMarkedContentNF(char *name, Object *obj);
  void lookupColorSpace(const char *name, Object *obj);
  GfxPattern *lookupPattern(char *name, OutputDev *out, GfxState *state);
  GfxShading *lookupShading(char *name, OutputDev *out, GfxState *state);
  GBool lookupGState(char *name, Object *obj);
  GBool lookupGStateNF(char *name, Object *obj);

  GfxResources *getNext() { return next; }

private:

  GfxFontDict *fonts;
  Object xObjDict;
  Object colorSpaceDict;
  Object patternDict;
  Object shadingDict;
  Object gStateDict;
  PopplerObjectCache gStateCache;
  Object propertiesDict;
  GfxResources *next;
};

//------------------------------------------------------------------------
// Gfx
//------------------------------------------------------------------------

class Gfx {
public:

  // Constructor for regular output.
  Gfx(PDFDoc *docA, OutputDev *outA, int pageNum, Dict *resDict,
      double hDPI, double vDPI, PDFRectangle *box,
      PDFRectangle *cropBox, int rotate,
      GBool (*abortCheckCbkA)(void *data) = NULL,
      void *abortCheckCbkDataA = NULL, XRef *xrefA = NULL);

  // Constructor for a sub-page object.
  Gfx(PDFDoc *docA, OutputDev *outA, Dict *resDict,
      PDFRectangle *box, PDFRectangle *cropBox,
      GBool (*abortCheckCbkA)(void *data) = NULL,
      void *abortCheckCbkDataA = NULL, XRef *xrefA = NULL);
#ifdef USE_CMS
  void initDisplayProfile();
#endif
  ~Gfx();

  XRef *getXRef() { return xref; }

  // Interpret a stream or array of streams.
  void display(Object *obj, GBool topLevel = gTrue);

  // Display an annotation, given its appearance (a Form XObject),
  // border style, and bounding box (in default user space).
  void drawAnnot(Object *str, AnnotBorder *border, AnnotColor *aColor,
		 double xMin, double yMin, double xMax, double yMax, int rotate);

  // Save graphics state.
  void saveState();

  // Push a new state guard
  void pushStateGuard();

  // Restore graphics state.
  void restoreState();

  // Pop to state guard and pop guard
  void popStateGuard();

  // Get the current graphics state object.
  GfxState *getState() { return state; }

  GBool checkTransparencyGroup(Dict *resDict);

  void drawForm(Object *str, Dict *resDict, double *matrix, double *bbox,
	       GBool transpGroup = gFalse, GBool softMask = gFalse,
	       GfxColorSpace *blendingColorSpace = NULL,
	       GBool isolated = gFalse, GBool knockout = gFalse,
	       GBool alpha = gFalse, Function *transferFunc = NULL,
	       GfxColor *backdropColor = NULL);

  void pushResources(Dict *resDict);
  void popResources();

private:

  PDFDoc *doc;
  XRef *xref;			// the xref table for this PDF file
  Catalog *catalog;		// the Catalog for this PDF file  
  OutputDev *out;		// output device
  GBool subPage;		// is this a sub-page object?
  GBool printCommands;		// print the drawing commands (for debugging)
  GBool profileCommands;	// profile the drawing commands (for debugging)
  GBool commandAborted;         // did the previous command abort the drawing?
  GfxResources *res;		// resource stack
  int updateLevel;

  GfxState *state;		// current graphics state
  int stackHeight;		// the height of the current graphics stack
  std::vector<int> stateGuards;   // a stack of state limits; to guard against unmatched pops
  GBool fontChanged;		// set if font or text matrix has changed
  GfxClipType clip;		// do a clip?
  int ignoreUndef;		// current BX/EX nesting level
  double baseMatrix[6];		// default matrix for most recent
				//   page/form/pattern
  int formDepth;
  GBool ocState;		// true if drawing is enabled, false if
				//   disabled

  MarkedContentStack *mcStack;	// current BMC/EMC stack

  Parser *parser;		// parser for page content stream(s)
  
  std::set<int> formsDrawing;	// the forms that are being drawn

  GBool				// callback to check for an abort
    (*abortCheckCbk)(void *data);
  void *abortCheckCbkData;

  static Operator opTab[];	// table of operators

  void go(GBool topLevel);
  void execOp(Object *cmd, Object args[], int numArgs);
  Operator *findOp(char *name);
  GBool checkArg(Object *arg, TchkType type);
  Goffset getPos();

  int bottomGuard();

  // graphics state operators
  void opSave(Object args[], int numArgs);
  void opRestore(Object args[], int numArgs);
  void opConcat(Object args[], int numArgs);
  void opSetDash(Object args[], int numArgs);
  void opSetFlat(Object args[], int numArgs);
  void opSetLineJoin(Object args[], int numArgs);
  void opSetLineCap(Object args[], int numArgs);
  void opSetMiterLimit(Object args[], int numArgs);
  void opSetLineWidth(Object args[], int numArgs);
  void opSetExtGState(Object args[], int numArgs);
  void doSoftMask(Object *str, GBool alpha,
		  GfxColorSpace *blendingColorSpace,
		  GBool isolated, GBool knockout,
		  Function *transferFunc, GfxColor *backdropColor);
  void opSetRenderingIntent(Object args[], int numArgs);

  // color operators
  void opSetFillGray(Object args[], int numArgs);
  void opSetStrokeGray(Object args[], int numArgs);
  void opSetFillCMYKColor(Object args[], int numArgs);
  void opSetStrokeCMYKColor(Object args[], int numArgs);
  void opSetFillRGBColor(Object args[], int numArgs);
  void opSetStrokeRGBColor(Object args[], int numArgs);
  void opSetFillColorSpace(Object args[], int numArgs);
  void opSetStrokeColorSpace(Object args[], int numArgs);
  void opSetFillColor(Object args[], int numArgs);
  void opSetStrokeColor(Object args[], int numArgs);
  void opSetFillColorN(Object args[], int numArgs);
  void opSetStrokeColorN(Object args[], int numArgs);

  // path segment operators
  void opMoveTo(Object args[], int numArgs);
  void opLineTo(Object args[], int numArgs);
  void opCurveTo(Object args[], int numArgs);
  void opCurveTo1(Object args[], int numArgs);
  void opCurveTo2(Object args[], int numArgs);
  void opRectangle(Object args[], int numArgs);
  void opClosePath(Object args[], int numArgs);

  // path painting operators
  void opEndPath(Object args[], int numArgs);
  void opStroke(Object args[], int numArgs);
  void opCloseStroke(Object args[], int numArgs);
  void opFill(Object args[], int numArgs);
  void opEOFill(Object args[], int numArgs);
  void opFillStroke(Object args[], int numArgs);
  void opCloseFillStroke(Object args[], int numArgs);
  void opEOFillStroke(Object args[], int numArgs);
  void opCloseEOFillStroke(Object args[], int numArgs);
  void doPatternFill(GBool eoFill);
  void doPatternStroke();
  void doPatternText();
  void doPatternImageMask(Object *ref, Stream *str, int width, int height,
			  GBool invert, GBool inlineImg);
  void doTilingPatternFill(GfxTilingPattern *tPat,
			   GBool stroke, GBool eoFill, GBool text);
  void doShadingPatternFill(GfxShadingPattern *sPat,
			    GBool stroke, GBool eoFill, GBool text);
  void opShFill(Object args[], int numArgs);
  void doFunctionShFill(GfxFunctionShading *shading);
  void doFunctionShFill1(GfxFunctionShading *shading,
			 double x0, double y0,
			 double x1, double y1,
			 GfxColor *colors, int depth);
  void doAxialShFill(GfxAxialShading *shading);
  void doRadialShFill(GfxRadialShading *shading);
  void doGouraudTriangleShFill(GfxGouraudTriangleShading *shading);
  void gouraudFillTriangle(double x0, double y0, GfxColor *color0,
			   double x1, double y1, GfxColor *color1,
			   double x2, double y2, GfxColor *color2,
			   int nComps, int depth, GfxState::ReusablePathIterator *path);
  void gouraudFillTriangle(double x0, double y0, double color0,
			   double x1, double y1, double color1,
			   double x2, double y2, double color2,
			   double refineColorThreshold, int depth, GfxGouraudTriangleShading *shading, GfxState::ReusablePathIterator *path);
  void doPatchMeshShFill(GfxPatchMeshShading *shading);
  void fillPatch(GfxPatch *patch, int colorComps, int patchColorComps, double refineColorThreshold, int depth, GfxPatchMeshShading *shading);
  void doEndPath();

  // path clipping operators
  void opClip(Object args[], int numArgs);
  void opEOClip(Object args[], int numArgs);

  // text object operators
  void opBeginText(Object args[], int numArgs);
  void opEndText(Object args[], int numArgs);

  // text state operators
  void opSetCharSpacing(Object args[], int numArgs);
  void opSetFont(Object args[], int numArgs);
  void opSetTextLeading(Object args[], int numArgs);
  void opSetTextRender(Object args[], int numArgs);
  void opSetTextRise(Object args[], int numArgs);
  void opSetWordSpacing(Object args[], int numArgs);
  void opSetHorizScaling(Object args[], int numArgs);

  // text positioning operators
  void opTextMove(Object args[], int numArgs);
  void opTextMoveSet(Object args[], int numArgs);
  void opSetTextMatrix(Object args[], int numArgs);
  void opTextNextLine(Object args[], int numArgs);

  // text string operators
  void opShowText(Object args[], int numArgs);
  void opMoveShowText(Object args[], int numArgs);
  void opMoveSetShowText(Object args[], int numArgs);
  void opShowSpaceText(Object args[], int numArgs);
  void doShowText(GooString *s);
  void doIncCharCount(GooString *s);

  // XObject operators
  void opXObject(Object args[], int numArgs);
  void doImage(Object *ref, Stream *str, GBool inlineImg);
  void doForm(Object *str);

  // in-line image operators
  void opBeginImage(Object args[], int numArgs);
  Stream *buildImageStream();
  void opImageData(Object args[], int numArgs);
  void opEndImage(Object args[], int numArgs);

  // type 3 font operators
  void opSetCharWidth(Object args[], int numArgs);
  void opSetCacheDevice(Object args[], int numArgs);

  // compatibility operators
  void opBeginIgnoreUndef(Object args[], int numArgs);
  void opEndIgnoreUndef(Object args[], int numArgs);

  // marked content operators
  void opBeginMarkedContent(Object args[], int numArgs);
  void opEndMarkedContent(Object args[], int numArgs);
  void opMarkPoint(Object args[], int numArgs);
  GfxState *saveStateStack();
  void restoreStateStack(GfxState *oldState);
  GBool contentIsHidden();
  void pushMarkedContent();
  void popMarkedContent();
};

#endif
