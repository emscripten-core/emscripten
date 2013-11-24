//========================================================================
//
// Annot.h
//
// Copyright 2000-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Scott Turner <scotty1024@mac.com>
// Copyright (C) 2007, 2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2007-2011, 2013 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2007, 2008 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright (C) 2008 Michael Vrable <mvrable@cs.ucsd.edu>
// Copyright (C) 2008 Hugo Mercier <hmercier31@gmail.com>
// Copyright (C) 2008 Pino Toscano <pino@kde.org>
// Copyright (C) 2008 Tomas Are Haavet <tomasare@gmail.com>
// Copyright (C) 2009-2011, 2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2012 Tobias Koenig <tokoe@kdab.com>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef ANNOT_H
#define ANNOT_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "Object.h"

class XRef;
class Gfx;
class CharCodeToUnicode;
class GfxFont;
class GfxResources;
class Page;
class PDFDoc;
class Form;
class FormWidget;
class FormField;
class FormFieldChoice;
class PDFRectangle;
class Movie;
class LinkAction;
class Sound;

enum AnnotLineEndingStyle {
  annotLineEndingSquare,        // Square
  annotLineEndingCircle,        // Circle
  annotLineEndingDiamond,       // Diamond
  annotLineEndingOpenArrow,     // OpenArrow
  annotLineEndingClosedArrow,   // ClosedArrow
  annotLineEndingNone,          // None
  annotLineEndingButt,          // Butt
  annotLineEndingROpenArrow,    // ROpenArrow
  annotLineEndingRClosedArrow,  // RClosedArrow
  annotLineEndingSlash          // Slash
};

enum AnnotExternalDataType {
  annotExternalDataMarkupUnknown,
  annotExternalDataMarkup3D       // Markup3D
};

//------------------------------------------------------------------------
// AnnotCoord
//------------------------------------------------------------------------

class AnnotCoord {
public:

  AnnotCoord() : x(0), y(0) { }
  AnnotCoord(double _x, double _y) : x(_x), y(_y) { }

  double getX() const { return x; }
  double getY() const { return y; }
  
protected:

  double x, y;
};

//------------------------------------------------------------------------
// AnnotPath
//------------------------------------------------------------------------

class AnnotPath {
public:
  AnnotPath();
  AnnotPath(Array *array);
  AnnotPath(AnnotCoord **coords, int coordLength);
  ~AnnotPath();

  double getX(int coord) const;
  double getY(int coord) const;
  AnnotCoord *getCoord(int coord) const;
  int getCoordsLength() const { return coordsLength; }
protected:
  AnnotCoord **coords;
  int coordsLength;

  void parsePathArray(Array *array);
};
  
//------------------------------------------------------------------------
// AnnotCalloutLine
//------------------------------------------------------------------------

class AnnotCalloutLine {
public:

  AnnotCalloutLine(double x1, double y1, double x2, double y2);
  virtual ~AnnotCalloutLine() { }

  double getX1() const { return coord1.getX(); }
  double getY1() const { return coord1.getY(); }
  double getX2() const { return coord2.getX(); }
  double getY2() const { return coord2.getY(); }
  
protected:

  AnnotCoord coord1, coord2;
};

//------------------------------------------------------------------------
// AnnotCalloutMultiLine
//------------------------------------------------------------------------

class AnnotCalloutMultiLine: public AnnotCalloutLine {
public:

  AnnotCalloutMultiLine(double x1, double y1, double x2, double y2,
    double x3, double y3);

  double getX3() const { return coord3.getX(); }
  double getY3() const { return coord3.getY(); }

protected:

  AnnotCoord coord3;
};

//------------------------------------------------------------------------
// AnnotBorderEffect
//------------------------------------------------------------------------

class AnnotBorderEffect {
public:

  enum AnnotBorderEffectType {
    borderEffectNoEffect, // S
    borderEffectCloudy    // C
  };

  AnnotBorderEffect(Dict *dict);

  AnnotBorderEffectType getEffectType() const { return effectType; }
  double getIntensity() const { return intensity; }

private:

  AnnotBorderEffectType effectType; // S  (Default S)
  double intensity;                 // I  (Default 0)
};

//------------------------------------------------------------------------
// AnnotQuadrilateral
//------------------------------------------------------------------------

class AnnotQuadrilaterals {
public:
  class AnnotQuadrilateral {
  public:
    AnnotQuadrilateral(double x1, double y1, double x2, double y2, double x3,
      double y3, double x4, double y4);

    AnnotCoord coord1, coord2, coord3, coord4;
  };

  AnnotQuadrilaterals(Array *array, PDFRectangle *rect);
  AnnotQuadrilaterals(AnnotQuadrilateral **quads, int quadsLength);
  ~AnnotQuadrilaterals();

  double getX1(int quadrilateral);
  double getY1(int quadrilateral);
  double getX2(int quadrilateral);
  double getY2(int quadrilateral);
  double getX3(int quadrilateral);
  double getY3(int quadrilateral);
  double getX4(int quadrilateral);
  double getY4(int quadrilateral);
  int getQuadrilateralsLength() const { return quadrilateralsLength; }
protected:

  AnnotQuadrilateral** quadrilaterals;
  int quadrilateralsLength;
};

//------------------------------------------------------------------------
// AnnotBorder
//------------------------------------------------------------------------

class AnnotBorder {
public:
  enum AnnotBorderType {
    typeUnknown,
    typeArray,
    typeBS
  };

  enum AnnotBorderStyle {
    borderSolid,      // Solid
    borderDashed,     // Dashed
    borderBeveled,    // Beveled
    borderInset,      // Inset
    borderUnderlined  // Underlined
  };

  AnnotBorder();
  virtual ~AnnotBorder();

  virtual void setWidth(double new_width) { width = new_width; }

  virtual AnnotBorderType getType() const { return type; }
  virtual double getWidth() const { return width; }
  virtual int getDashLength() const { return dashLength; }
  virtual double *getDash() const { return dash; }
  virtual AnnotBorderStyle getStyle() const { return style; }

protected:
  GBool parseDashArray(Object *dashObj);

  AnnotBorderType type;
  double width;
  static const int DASH_LIMIT = 10; // implementation note 82 in Appendix H.
  int dashLength;
  double *dash;
  AnnotBorderStyle style;
};

//------------------------------------------------------------------------
// AnnotBorderArray
//------------------------------------------------------------------------

class AnnotBorderArray: public AnnotBorder {
public:
  AnnotBorderArray();
  AnnotBorderArray(Array *array);

  void writeToObject(XRef *xref, Object *dest) const;

  void setHorizontalCorner(double hc) { horizontalCorner = hc; }
  void setVerticalCorner(double vc) { verticalCorner = vc; }

  double getHorizontalCorner() const { return horizontalCorner; }
  double getVerticalCorner() const { return verticalCorner; }

protected:
  double horizontalCorner;          // (Default 0)
  double verticalCorner;            // (Default 0)
  // double width;                  // (Default 1)  (inherited from AnnotBorder)
};

//------------------------------------------------------------------------
// AnnotBorderBS
//------------------------------------------------------------------------

class AnnotBorderBS: public AnnotBorder {
public:

  AnnotBorderBS();
  AnnotBorderBS(Dict *dict);

private:
  // double width;           // W  (Default 1)   (inherited from AnnotBorder)
  // AnnotBorderStyle style; // S  (Default S)   (inherited from AnnotBorder)
  // double *dash;           // D  (Default [3]) (inherited from AnnotBorder)
};

//------------------------------------------------------------------------
// AnnotColor
//------------------------------------------------------------------------

class AnnotColor {
public:

  enum AnnotColorSpace {
    colorTransparent = 0,
    colorGray        = 1,
    colorRGB         = 3,
    colorCMYK        = 4
  };

  AnnotColor();
  AnnotColor(double gray);
  AnnotColor(double r, double g, double b);
  AnnotColor(double c, double m, double y, double k);
  AnnotColor(Array *array, int adjust = 0);

  void adjustColor(int adjust);

  AnnotColorSpace getSpace() const { return (AnnotColorSpace) length; }
  const double *getValues() const { return values; }

  void writeToObject(XRef *xref, Object *dest) const;

private:

  double values[4];
  int length;
};

//------------------------------------------------------------------------
// AnnotIconFit
//------------------------------------------------------------------------

class AnnotIconFit {
public:

  enum AnnotIconFitScaleWhen {
    scaleAlways,  // A
    scaleBigger,  // B
    scaleSmaller, // S
    scaleNever    // N
  };

  enum AnnotIconFitScale {
    scaleAnamorphic,  // A
    scaleProportional // P
  };

  AnnotIconFit(Dict *dict);

  AnnotIconFitScaleWhen getScaleWhen() { return scaleWhen; }
  AnnotIconFitScale getScale() { return scale; }
  double getLeft() { return left; }
  double getBottom() { return bottom; }
  bool getFullyBounds() { return fullyBounds; }

protected:

  AnnotIconFitScaleWhen scaleWhen;  // SW (Default A)
  AnnotIconFitScale scale;          // S  (Default P)
  double left;                      // A  (Default [0.5 0.5]
  double bottom;                    // Only if scale is P
  bool fullyBounds;                 // FB (Default false)
};

//------------------------------------------------------------------------
// AnnotAppearance
//------------------------------------------------------------------------

class AnnotAppearance {
public:

  enum AnnotAppearanceType {
    appearNormal,
    appearRollover,
    appearDown
  };

  AnnotAppearance(PDFDoc *docA, Object *dict);
  ~AnnotAppearance();

  // State is ignored if no subdictionary is present
  void getAppearanceStream(AnnotAppearanceType type, const char *state, Object *dest);

  // Access keys in normal appearance subdictionary (N)
  GooString * getStateKey(int i);
  int getNumStates();

  // Removes all associated streams in the xref table. Caller is required to
  // reset parent annotation's AP and AS after this call.
  void removeAllStreams();

  // Test if this AnnotAppearance references the specified stream
  GBool referencesStream(Ref targetStreamRef);

private:
  static GBool referencesStream(Object *stateObj, Ref targetStreamRef);
  void removeStream(Ref refToStream);
  void removeStateStreams(Object *state);

protected:
  PDFDoc *doc;
  XRef *xref;                   // the xref table for this PDF file
  Object appearDict;            // Annotation's AP
};

//------------------------------------------------------------------------
// AnnotAppearanceCharacs
//------------------------------------------------------------------------

class AnnotAppearanceCharacs {
public:

  enum AnnotAppearanceCharacsTextPos {
    captionNoIcon,    // 0
    captionNoCaption, // 1
    captionBelow,     // 2
    captionAbove,     // 3
    captionRight,     // 4
    captionLeft,      // 5
    captionOverlaid   // 6
  };

  AnnotAppearanceCharacs(Dict *dict);
  ~AnnotAppearanceCharacs();

  int getRotation() { return rotation; }
  AnnotColor *getBorderColor() { return borderColor; }
  AnnotColor *getBackColor() { return backColor; }
  GooString *getNormalCaption() { return normalCaption; }
  GooString *getRolloverCaption() { return rolloverCaption; }
  GooString *getAlternateCaption() { return alternateCaption; }
  AnnotIconFit *getIconFit() { return iconFit; }
  AnnotAppearanceCharacsTextPos getPosition() { return position; }

protected:

  int rotation;                           // R  (Default 0)
  AnnotColor *borderColor;                // BC
  AnnotColor *backColor;                  // BG
  GooString *normalCaption;               // CA
  GooString *rolloverCaption;             // RC
  GooString *alternateCaption;            // AC
  // I
  // RI
  // IX
  AnnotIconFit *iconFit;                  // IF
  AnnotAppearanceCharacsTextPos position; // TP (Default 0)
};

//------------------------------------------------------------------------
// AnnotAppearanceBBox
//------------------------------------------------------------------------

class AnnotAppearanceBBox
{
public:
  AnnotAppearanceBBox(PDFRectangle *init);

  void setBorderWidth(double w) { borderWidth = w; }

  // The following functions operate on coords relative to [origX origY]
  void extendTo(double x, double y);
  void getBBoxRect(double bbox[4]) const;

  // Get boundaries in page coordinates
  double getPageXMin() const;
  double getPageYMin() const;
  double getPageXMax() const;
  double getPageYMax() const;

private:
  double origX, origY, borderWidth;
  double minX, minY, maxX, maxY;
};

//------------------------------------------------------------------------
// Annot
//------------------------------------------------------------------------

class Annot {
  friend class Annots;
  friend class Page;
public:
  enum AnnotFlag {
    flagUnknown        = 0x0000,
    flagInvisible      = 0x0001,
    flagHidden         = 0x0002,
    flagPrint          = 0x0004,
    flagNoZoom         = 0x0008,
    flagNoRotate       = 0x0010,
    flagNoView         = 0x0020,
    flagReadOnly       = 0x0040,
    flagLocked         = 0x0080,
    flagToggleNoView   = 0x0100,
    flagLockedContents = 0x0200
  };

  enum AnnotSubtype {
    typeUnknown,        //                 0
    typeText,           // Text            1
    typeLink,           // Link            2
    typeFreeText,       // FreeText        3
    typeLine,           // Line            4
    typeSquare,         // Square          5
    typeCircle,         // Circle          6
    typePolygon,        // Polygon         7
    typePolyLine,       // PolyLine        8
    typeHighlight,      // Highlight       9
    typeUnderline,      // Underline      10
    typeSquiggly,       // Squiggly       11
    typeStrikeOut,      // StrikeOut      12
    typeStamp,          // Stamp          13
    typeCaret,          // Caret          14
    typeInk,            // Ink            15
    typePopup,          // Popup          16
    typeFileAttachment, // FileAttachment 17
    typeSound,          // Sound          18
    typeMovie,          // Movie          19
    typeWidget,         // Widget         20
    typeScreen,         // Screen         21
    typePrinterMark,    // PrinterMark    22
    typeTrapNet,        // TrapNet        23
    typeWatermark,      // Watermark      24
    type3D              // 3D             25
  };

  /**
   * Describes the additional actions of a screen or widget annotation.
   */
  enum AdditionalActionsType {
    actionCursorEntering, ///< Performed when the cursor enters the annotation's active area
    actionCursorLeaving,  ///< Performed when the cursor exists the annotation's active area
    actionMousePressed,   ///< Performed when the mouse button is pressed inside the annotation's active area
    actionMouseReleased,  ///< Performed when the mouse button is released inside the annotation's active area
    actionFocusIn,        ///< Performed when the annotation receives the input focus
    actionFocusOut,       ///< Performed when the annotation loses the input focus
    actionPageOpening,    ///< Performed when the page containing the annotation is opened
    actionPageClosing,    ///< Performed when the page containing the annotation is closed
    actionPageVisible,    ///< Performed when the page containing the annotation becomes visible
    actionPageInvisible   ///< Performed when the page containing the annotation becomes invisible
  };

  enum FormAdditionalActionsType {
    actionFieldModified,   ///< Performed when the when the user modifies the field
    actionFormatField,     ///< Performed before the field is formatted to display its value
    actionValidateField,   ///< Performed when the field value changes
    actionCalculateField,  ///< Performed when the field needs to be recalculated
  };

  Annot(PDFDoc *docA, PDFRectangle *rectA);
  Annot(PDFDoc *docA, Dict *dict);
  Annot(PDFDoc *docA, Dict *dict, Object *obj);
  GBool isOk() { return ok; }

  void incRefCnt();
  void decRefCnt();

  virtual void draw(Gfx *gfx, GBool printing);
  // Get the resource dict of the appearance stream
  virtual Object *getAppearanceResDict(Object *dest);

  GBool match(Ref *refA)
    { return ref.num == refA->num && ref.gen == refA->gen; }

  double getXMin();
  double getYMin();
  double getXMax();
  double getYMax();

  double getFontSize() { return fontSize; }

  void setRect(PDFRectangle *rect);
  void setRect(double x1, double y1, double x2, double y2);

  // Sets the annot contents to new_content
  // new_content should never be NULL
  virtual void setContents(GooString *new_content);
  void setName(GooString *new_name);
  void setModified(GooString *new_date);
  void setFlags(Guint new_flags);

  void setBorder(AnnotBorderArray *new_border); // Takes ownership

  // The annotation takes the ownership of
  // new_color. 
  void setColor(AnnotColor *new_color);

  void setAppearanceState(const char *state);

  // getters
  PDFDoc *getDoc() const { return doc; }
  XRef *getXRef() const { return xref; }
  GBool getHasRef() const { return hasRef; }
  Ref getRef() const { return ref; }
  AnnotSubtype getType() const { return type; }
  PDFRectangle *getRect() const { return rect; }
  void getRect(double *x1, double *y1, double *x2, double *y2) const;
  GooString *getContents() const { return contents; }
  int getPageNum() const { return page; }
  GooString *getName() const { return name; }
  GooString *getModified() const { return modified; }
  Guint getFlags() const { return flags; }
  AnnotAppearance *getAppearStreams() const { return appearStreams; }
  GooString *getAppearState() const { return appearState; }
  AnnotBorder *getBorder() const { return border; }
  AnnotColor *getColor() const { return color; }
  int getTreeKey() const { return treeKey; }

  int getId() { return ref.num; }

  // Check if point is inside the annot rectangle.
  GBool inRect(double x, double y) const;

private:
  void readArrayNum(Object *pdfArray, int key, double *value);
  // write vStr[i:j[ in appearBuf

  void initialize (PDFDoc *docA, Dict *dict);
  void setPage (int new_page, GBool updateP); // Called by Page::addAnnot and Annots ctor


protected:
  virtual ~Annot();
  virtual void removeReferencedObjects(); // Called by Page::removeAnnot
  void setColor(AnnotColor *color, GBool fill);
  void drawCircle(double cx, double cy, double r, GBool fill);
  void drawCircleTopLeft(double cx, double cy, double r);
  void drawCircleBottomRight(double cx, double cy, double r);
  void layoutText(GooString *text, GooString *outBuf, int *i, GfxFont *font,
		  double *width, double widthLimit, int *charCount,
		  GBool noReencode);
  void writeString(GooString *str, GooString *appearBuf);
  void createForm(double *bbox, GBool transparencyGroup, Object *resDict, Object *aStream);
  void createResourcesDict(const char *formName, Object *formStream, const char *stateName,
			   double opacity, const char *blendMode, Object *resDict);
  GBool isVisible(GBool printing);
  int getRotation() const;

  // Updates the field key of the annotation dictionary
  // and sets M to the current time
  void update(const char *key, Object *value);

  // Delete appearance streams and reset appearance state
  void invalidateAppearance();

  int refCnt;

  Object annotObj;
  
  // required data
  AnnotSubtype type;                // Annotation type
  PDFRectangle *rect;               // Rect

  // optional data
  GooString *contents;              // Contents
  int       page;                   // P
  GooString *name;                  // NM
  GooString *modified;              // M
  Guint flags;                      // F (must be a 32 bit unsigned int)
  AnnotAppearance *appearStreams;   // AP
  Object appearance;     // a reference to the Form XObject stream
                         //   for the normal appearance
  AnnotAppearanceBBox *appearBBox;  // BBox of generated appearance
  GooString *appearState;           // AS
  int treeKey;                      // Struct Parent;
  Object oc;                        // OC

  PDFDoc *doc;
  XRef *xref;			// the xref table for this PDF file
  Ref ref;                      // object ref identifying this annotation
  GooString *appearBuf;
  AnnotBorder *border;          // Border, BS
  AnnotColor *color;            // C
  double fontSize; 
  GBool ok;

  bool hasRef;
#if MULTITHREADED
  GooMutex mutex;
#endif
};

//------------------------------------------------------------------------
// AnnotPopup
//------------------------------------------------------------------------

class AnnotPopup: public Annot {
public:
  AnnotPopup(PDFDoc *docA, PDFRectangle *rect);
  AnnotPopup(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotPopup();

  Object *getParent(Object *obj) { return parent.fetch (xref, obj); }
  Object *getParentNF(Object *obj) { return &parent; }
  void setParent(Object *parentA);
  void setParent(Annot *parentA);
  GBool getOpen() const { return open; }
  void setOpen(GBool openA);

protected:
  void initialize(PDFDoc *docA, Dict *dict);

  Object parent; // Parent
  GBool open;   // Open
};

//------------------------------------------------------------------------
// AnnotMarkup
//------------------------------------------------------------------------

class AnnotMarkup: public Annot {
public:
  enum  AnnotMarkupReplyType {
    replyTypeR,     // R
    replyTypeGroup  // Group
  };

  AnnotMarkup(PDFDoc *docA, PDFRectangle *rect);
  AnnotMarkup(PDFDoc *docA, Dict *dict, Object *obj);
  virtual ~AnnotMarkup();

  // getters
  GooString *getLabel() const { return label; }
  AnnotPopup *getPopup() const { return popup; }
  double getOpacity() const { return opacity; }
  // getRC
  GooString *getDate() const { return date; }
  int getInReplyToID() const { return inReplyTo.num; }
  GooString *getSubject() const { return subject; }
  AnnotMarkupReplyType getReplyTo() const { return replyTo; }
  AnnotExternalDataType getExData() const { return exData; }

  // The annotation takes the ownership of new_popup
  void setPopup(AnnotPopup *new_popup);
  void setLabel(GooString *new_label);
  void setOpacity(double opacityA);
  void setDate(GooString *new_date);

protected:
  virtual void removeReferencedObjects();

  GooString *label;             // T            (Default autor)
  AnnotPopup *popup;            // Popup
  double opacity;               // CA           (Default 1.0)
  // RC
  GooString *date;              // CreationDate
  Ref inReplyTo;                // IRT
  GooString *subject;           // Subj
  AnnotMarkupReplyType replyTo; // RT           (Default R)
  // this object is overrided by the custom intent fields defined in some
  // annotation types.
  //GooString *intent;          // IT
  AnnotExternalDataType exData; // ExData

private:
  void initialize(PDFDoc *docA, Dict *dict, Object *obj);
};

//------------------------------------------------------------------------
// AnnotText
//------------------------------------------------------------------------

class AnnotText: public AnnotMarkup {
public:
  enum AnnotTextState {
    stateUnknown,
    // Marked state model
    stateMarked,    // Marked
    stateUnmarked,  // Unmarked
    // Review state model
    stateAccepted,  // Accepted
    stateRejected,  // Rejected
    stateCancelled, // Cancelled
    stateCompleted, // Completed
    stateNone       // None
  };

  AnnotText(PDFDoc *docA, PDFRectangle *rect);
  AnnotText(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotText();

  virtual void draw(Gfx *gfx, GBool printing);

  // getters
  GBool getOpen() const { return open; }
  GooString *getIcon() const { return icon; }
  AnnotTextState getState() const { return state; }

  void setOpen(GBool openA);
  void setIcon(GooString *new_icon);

private:

  void initialize(PDFDoc *docA, Dict *dict);

  GBool open;                       // Open       (Default false)
  GooString *icon;                  // Name       (Default Note)
  AnnotTextState state;             // State      (Default Umarked if
                                    //             StateModel Marked
                                    //             None if StareModel Review)
};

//------------------------------------------------------------------------
// AnnotMovie
//------------------------------------------------------------------------



class AnnotMovie: public Annot {
 public:
  AnnotMovie(PDFDoc *docA, PDFRectangle *rect, Movie *movieA);
  AnnotMovie(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotMovie();

  virtual void draw(Gfx *gfx, GBool printing);

  GooString* getTitle() { return title; }
  Movie* getMovie() { return movie; }

 private:
  void initialize(PDFDoc *docA, Dict *dict);

  GooString* title;      // T
  Movie* movie;          // Movie + A
};


//------------------------------------------------------------------------
// AnnotScreen
//------------------------------------------------------------------------

class AnnotScreen: public Annot {
 public:

  AnnotScreen(PDFDoc *docA, PDFRectangle *rect);
  AnnotScreen(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotScreen();

  GooString* getTitle() { return title; }

  AnnotAppearanceCharacs *getAppearCharacs() { return appearCharacs; }
  LinkAction* getAction() { return action; }
  LinkAction *getAdditionalAction(AdditionalActionsType type);

 private:
  void initialize(PDFDoc *docA, Dict *dict);


  GooString* title;                      // T

  AnnotAppearanceCharacs* appearCharacs; // MK

  LinkAction *action;                    // A
  Object additionalActions;              // AA
};

//------------------------------------------------------------------------
// AnnotLink
//------------------------------------------------------------------------

class AnnotLink: public Annot {
public:

  enum AnnotLinkEffect {
    effectNone,     // N
    effectInvert,   // I
    effectOutline,  // O
    effectPush      // P
  };

  AnnotLink(PDFDoc *docA, PDFRectangle *rect);
  AnnotLink(PDFDoc *docA, Dict *dict, Object *obj);
  virtual ~AnnotLink();

  virtual void draw(Gfx *gfx, GBool printing);

  // getters
  LinkAction *getAction() const { return action; }
  AnnotLinkEffect getLinkEffect() const { return linkEffect; }
  Dict *getUriAction() const { return uriAction; }
  AnnotQuadrilaterals *getQuadrilaterals() const { return quadrilaterals; }

protected:

  void initialize(PDFDoc *docA, Dict *dict);

  LinkAction *action;                  // A, Dest
  AnnotLinkEffect linkEffect;          // H          (Default I)
  Dict *uriAction;                     // PA

  AnnotQuadrilaterals *quadrilaterals; // QuadPoints
};

//------------------------------------------------------------------------
// AnnotFreeText
//------------------------------------------------------------------------

class AnnotFreeText: public AnnotMarkup {
public:

  enum AnnotFreeTextQuadding {
    quaddingLeftJustified,  // 0
    quaddingCentered,       // 1
    quaddingRightJustified  // 2
  };

  enum AnnotFreeTextIntent {
    intentFreeText,           // FreeText
    intentFreeTextCallout,    // FreeTextCallout
    intentFreeTextTypeWriter  // FreeTextTypeWriter
  };

  AnnotFreeText(PDFDoc *docA, PDFRectangle *rect, GooString *da);
  AnnotFreeText(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotFreeText();

  virtual void draw(Gfx *gfx, GBool printing);
  virtual Object *getAppearanceResDict(Object *dest);
  virtual void setContents(GooString *new_content);

  void setAppearanceString(GooString *new_string);
  void setQuadding(AnnotFreeTextQuadding new_quadding);
  void setStyleString(GooString *new_string);
  void setCalloutLine(AnnotCalloutLine *line);
  void setIntent(AnnotFreeTextIntent new_intent);

  // getters
  GooString *getAppearanceString() const { return appearanceString; }
  AnnotFreeTextQuadding getQuadding() const { return quadding; }
  // return rc
  GooString *getStyleString() const { return styleString; }
  AnnotCalloutLine *getCalloutLine() const {  return calloutLine; }
  AnnotFreeTextIntent getIntent() const { return intent; }
  AnnotBorderEffect *getBorderEffect() const { return borderEffect; }
  PDFRectangle *getRectangle() const { return rectangle; }
  AnnotLineEndingStyle getEndStyle() const { return endStyle; }

protected:

  void initialize(PDFDoc *docA, Dict *dict);
  static void parseAppearanceString(GooString *da, double &fontsize, AnnotColor* &fontcolor);
  void generateFreeTextAppearance();

  // required
  GooString *appearanceString;      // DA

  // optional
  AnnotFreeTextQuadding quadding;   // Q  (Default 0)
  // RC
  GooString *styleString;           // DS
  AnnotCalloutLine *calloutLine;    // CL
  AnnotFreeTextIntent intent;       // IT
  AnnotBorderEffect *borderEffect;  // BE
  PDFRectangle *rectangle;          // RD
  // inherited  from Annot
  // AnnotBorderBS border;          // BS
  AnnotLineEndingStyle endStyle;    // LE (Default None)
};

//------------------------------------------------------------------------
// AnnotLine
//------------------------------------------------------------------------

class AnnotLine: public AnnotMarkup {
public:

  enum AnnotLineIntent {
    intentLineArrow,    // LineArrow
    intentLineDimension // LineDimension
  };

  enum AnnotLineCaptionPos {
    captionPosInline, // Inline
    captionPosTop     // Top
  };

  AnnotLine(PDFDoc *docA, PDFRectangle *rect);
  AnnotLine(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotLine();

  virtual void draw(Gfx *gfx, GBool printing);
  virtual Object *getAppearanceResDict(Object *dest);
  virtual void setContents(GooString *new_content);

  void setVertices(double x1, double y1, double x2, double y2);
  void setStartEndStyle(AnnotLineEndingStyle start, AnnotLineEndingStyle end);
  void setInteriorColor(AnnotColor *new_color);
  void setLeaderLineLength(double len);
  void setLeaderLineExtension(double len);
  void setCaption(bool new_cap);
  void setIntent(AnnotLineIntent new_intent);

  // getters
  AnnotLineEndingStyle getStartStyle() const { return startStyle; }
  AnnotLineEndingStyle getEndStyle() const { return endStyle; }
  AnnotColor *getInteriorColor() const { return interiorColor; }
  double getLeaderLineLength() const { return leaderLineLength; }
  double getLeaderLineExtension() const { return leaderLineExtension; }
  bool getCaption() const { return caption; }
  AnnotLineIntent getIntent() const { return intent; }
  double  getLeaderLineOffset() const { return leaderLineOffset; }
  AnnotLineCaptionPos getCaptionPos() const { return captionPos; }
  Dict *getMeasure() const { return measure; }
  double getCaptionTextHorizontal() const { return captionTextHorizontal; }
  double getCaptionTextVertical() const { return captionTextVertical; }
  double getX1() const { return coord1->getX(); }
  double getY1() const { return coord1->getY(); }
  double getX2() const { return coord2->getX(); }
  double getY2() const { return coord2->getY(); }

protected:

  void initialize(PDFDoc *docA, Dict *dict);
  void generateLineAppearance();

  // required
  AnnotCoord *coord1, *coord2;
  
  // optional
  // inherited  from Annot
  // AnnotBorderBS border;          // BS
  AnnotLineEndingStyle startStyle;  // LE       (Default [/None /None])
  AnnotLineEndingStyle endStyle;    //
  AnnotColor *interiorColor;        // IC
  double leaderLineLength;          // LL       (Default 0)
  double leaderLineExtension;       // LLE      (Default 0)
  bool caption;                     // Cap      (Default false)
  AnnotLineIntent intent;           // IT
  double leaderLineOffset;          // LLO
  AnnotLineCaptionPos captionPos;   // CP       (Default Inline)
  Dict *measure;                    // Measure
  double captionTextHorizontal;     // CO       (Default [0, 0])
  double captionTextVertical;       //
};

//------------------------------------------------------------------------
// AnnotTextMarkup
//------------------------------------------------------------------------

class AnnotTextMarkup: public AnnotMarkup {
public:

  AnnotTextMarkup(PDFDoc *docA, PDFRectangle *rect, AnnotSubtype subType);
  AnnotTextMarkup(PDFDoc *docA, Dict *dict, Object *obj);
  virtual ~AnnotTextMarkup();

  virtual void draw(Gfx *gfx, GBool printing);

  // typeHighlight, typeUnderline, typeSquiggly or typeStrikeOut
  void setType(AnnotSubtype new_type);

  void setQuadrilaterals(AnnotQuadrilaterals *quadPoints);

  AnnotQuadrilaterals *getQuadrilaterals() const { return quadrilaterals; }

protected:

  void initialize(PDFDoc *docA, Dict *dict);
  
  AnnotQuadrilaterals *quadrilaterals; // QuadPoints
};

//------------------------------------------------------------------------
// AnnotStamp
//------------------------------------------------------------------------

class AnnotStamp: public AnnotMarkup {
public:

  AnnotStamp(PDFDoc *docA, PDFRectangle *rect);
  AnnotStamp(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotStamp();

  void setIcon(GooString *new_icon);

  // getters
  GooString *getIcon() const { return icon; }

private:

  void initialize(PDFDoc *docA, Dict *dict);

  GooString *icon;                  // Name       (Default Draft)
};

//------------------------------------------------------------------------
// AnnotGeometry
//------------------------------------------------------------------------

class AnnotGeometry: public AnnotMarkup {
public:

  AnnotGeometry(PDFDoc *docA, PDFRectangle *rect, AnnotSubtype subType);
  AnnotGeometry(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotGeometry();

  virtual void draw(Gfx *gfx, GBool printing);

  void setType(AnnotSubtype new_type); // typeSquare or typeCircle
  void setInteriorColor(AnnotColor *new_color);

  // getters
  AnnotColor *getInteriorColor() const { return interiorColor; }
  AnnotBorderEffect *getBorderEffect() const { return borderEffect; }
  PDFRectangle *getGeometryRect() const { return geometryRect; }

private:

  void initialize(PDFDoc *docA, Dict *dict);

  AnnotColor *interiorColor;        // IC
  AnnotBorderEffect *borderEffect;  // BE
  PDFRectangle *geometryRect;       // RD (combined with Rect)
};

//------------------------------------------------------------------------
// AnnotPolygon
//------------------------------------------------------------------------

class AnnotPolygon: public AnnotMarkup {
public:

  enum AnnotPolygonIntent {
    polygonCloud,      // PolygonCloud
    polylineDimension, // PolyLineDimension
    polygonDimension   // PolygonDimension
  };

  AnnotPolygon(PDFDoc *docA, PDFRectangle *rect, AnnotSubtype subType);
  AnnotPolygon(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotPolygon();

  virtual void draw(Gfx *gfx, GBool printing);

  void setType(AnnotSubtype new_type); // typePolygon or typePolyLine
  void setVertices(AnnotPath *path);
  void setStartEndStyle(AnnotLineEndingStyle start, AnnotLineEndingStyle end);
  void setInteriorColor(AnnotColor *new_color);
  void setIntent(AnnotPolygonIntent new_intent);

  // getters
  AnnotPath *getVertices() const { return vertices; }
  AnnotLineEndingStyle getStartStyle() const { return startStyle; }
  AnnotLineEndingStyle getEndStyle() const { return endStyle; }
  AnnotColor *getInteriorColor() const { return interiorColor; }
  AnnotBorderEffect *getBorderEffect() const { return borderEffect; }
  AnnotPolygonIntent getIntent() const { return intent; }

private:

  void initialize(PDFDoc *docA, Dict *dict);

  // required
  AnnotPath *vertices;              // Vertices

  // optional
  AnnotLineEndingStyle startStyle;  // LE       (Default [/None /None])
  AnnotLineEndingStyle endStyle;    //
  // inherited  from Annot
  // AnnotBorderBS border;          // BS
  AnnotColor *interiorColor;        // IC
  AnnotBorderEffect *borderEffect;  // BE
  AnnotPolygonIntent intent;        // IT
  // Measure
};

//------------------------------------------------------------------------
// AnnotCaret
//------------------------------------------------------------------------

class AnnotCaret: public AnnotMarkup {
public:

  enum AnnotCaretSymbol {
    symbolNone,     // None
    symbolP         // P
  };

  AnnotCaret(PDFDoc *docA, PDFRectangle *rect);
  AnnotCaret(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotCaret();

  void setSymbol(AnnotCaretSymbol new_symbol);

  // getters
  AnnotCaretSymbol getSymbol() const { return symbol; }
  PDFRectangle *getCaretRect() const { return caretRect; }

private:

  void initialize(PDFDoc *docA, Dict *dict);

  AnnotCaretSymbol symbol;       // Sy         (Default None)
  PDFRectangle *caretRect;       // RD (combined with Rect)
};

//------------------------------------------------------------------------
// AnnotInk
//------------------------------------------------------------------------

class AnnotInk: public AnnotMarkup {
public:

  AnnotInk(PDFDoc *docA, PDFRectangle *rect);
  AnnotInk(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotInk();

  virtual void draw(Gfx *gfx, GBool printing);

  void setInkList(AnnotPath **paths, int n_paths);

  // getters
  AnnotPath **getInkList() const { return inkList; }
  int getInkListLength() const { return inkListLength; }

private:

  void initialize(PDFDoc *docA, Dict *dict);
  void writeInkList(AnnotPath **paths, int n_paths, Array *dest_array);
  void parseInkList(Array *src_array);
  void freeInkList();

  // required
  AnnotPath **inkList;       // InkList
  int inkListLength;

  // optional
  // inherited from Annot
  // AnnotBorderBS border;  // BS
};

//------------------------------------------------------------------------
// AnnotFileAttachment
//------------------------------------------------------------------------

class AnnotFileAttachment: public AnnotMarkup {
public:

  AnnotFileAttachment(PDFDoc *docA, PDFRectangle *rect, GooString *filename);
  AnnotFileAttachment(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotFileAttachment();

  virtual void draw(Gfx *gfx, GBool printing);

  // getters
  Object *getFile() { return &file; }
  GooString *getName() const { return name; }

private:

  void initialize(PDFDoc *docA, Dict *dict);

  // required
  Object file;      // FS

  // optional
  GooString *name;  // Name
};

//------------------------------------------------------------------------
// AnnotSound
//------------------------------------------------------------------------

class AnnotSound: public AnnotMarkup {
public:

  AnnotSound(PDFDoc *docA, PDFRectangle *rect, Sound *soundA);
  AnnotSound(PDFDoc *docA, Dict *dict, Object *obj);
  ~AnnotSound();

  virtual void draw(Gfx *gfx, GBool printing);

  // getters
  Sound *getSound() { return sound; }
  GooString *getName() const { return name; }

private:

  void initialize(PDFDoc *docA, Dict *dict);

  // required
  Sound *sound;                  // Sound

  // optional
  GooString *name;               // Name
};

//------------------------------------------------------------------------
// AnnotWidget
//------------------------------------------------------------------------

class AnnotWidget: public Annot {
public:

  enum AnnotWidgetHighlightMode {
    highlightModeNone,    // N
    highlightModeInvert,  // I
    highlightModeOutline, // O
    highlightModePush     // P,T
  };

  AnnotWidget(PDFDoc *docA, Dict *dict, Object *obj);
  AnnotWidget(PDFDoc *docA, Dict *dict, Object *obj, FormField *fieldA);
  virtual ~AnnotWidget();

  virtual void draw(Gfx *gfx, GBool printing);

  void drawBorder();
  void drawFormFieldButton(GfxResources *resources, GooString *da);
  void drawFormFieldText(GfxResources *resources, GooString *da);
  void drawFormFieldChoice(GfxResources *resources, GooString *da);
  void generateFieldAppearance ();
  void updateAppearanceStream ();

  AnnotWidgetHighlightMode getMode() { return mode; }
  AnnotAppearanceCharacs *getAppearCharacs() { return appearCharacs; }
  LinkAction *getAction() { return action; }
  LinkAction *getAdditionalAction(AdditionalActionsType type);
  LinkAction *getFormAdditionalAction(FormAdditionalActionsType type);
  Dict *getParent() { return parent; }

private:

  void initialize(PDFDoc *docA, Dict *dict);

  void drawText(GooString *text, GooString *da, GfxResources *resources,
		GBool multiline, int comb, int quadding,
		GBool txField, GBool forceZapfDingbats,
		GBool password=false);
  void drawListBox(FormFieldChoice *fieldChoice,
		   GooString *da, GfxResources *resources, int quadding);

  Form *form;
  FormField *field;                       // FormField object for this annotation
  AnnotWidgetHighlightMode mode;          // H  (Default I)
  AnnotAppearanceCharacs *appearCharacs;  // MK
  LinkAction *action;                     // A
  Object additionalActions;               // AA
  // inherited  from Annot
  // AnnotBorderBS border;                // BS
  Dict *parent;                           // Parent
  GBool addDingbatsResource;
  Ref updatedAppearanceStream; // {-1,-1} if updateAppearanceStream has never been called
};

//------------------------------------------------------------------------
// Annot3D
//------------------------------------------------------------------------

class Annot3D: public Annot {
  class Activation {
  public:
    enum ActivationATrigger {
      aTriggerUnknown,
      aTriggerPageOpened,  // PO
      aTriggerPageVisible, // PV
      aTriggerUserAction   // XA
    };

    enum ActivationAState {
      aStateUnknown,
      aStateEnabled, // I
      aStateDisabled // L
    };

    enum ActivationDTrigger {
      dTriggerUnknown,
      dTriggerPageClosed,    // PC
      dTriggerPageInvisible, // PI
      dTriggerUserAction     // XD
    };

    enum ActivationDState {
      dStateUnknown,
      dStateUninstantiaded, // U
      dStateInstantiated,   // I
      dStateLive            // L
    };

    Activation(Dict *dict);
  private:
    
    ActivationATrigger aTrigger;  // A   (Default XA)
    ActivationAState aState;      // AIS (Default L)
    ActivationDTrigger dTrigger;  // D   (Default PI)
    ActivationDState dState;      // DIS (Default U)
    GBool displayToolbar;         // TB  (Default true)
    GBool displayNavigation;      // NP  (Default false);
  };
public:

  Annot3D(PDFDoc *docA, PDFRectangle *rect);
  Annot3D(PDFDoc *docA, Dict *dict, Object *obj);
  ~Annot3D();

  // getters

private:

  void initialize(PDFDoc *docA, Dict *dict);

  Activation *activation;  // 3DA
};

//------------------------------------------------------------------------
// Annots
//------------------------------------------------------------------------

class Annots {
public:

  // Build a list of Annot objects and call setPage on them
  Annots(PDFDoc *docA, int page, Object *annotsObj);

  ~Annots();

  // Iterate through list of annotations.
  int getNumAnnots() { return nAnnots; }
  Annot *getAnnot(int i) { return annots[i]; }
  void appendAnnot(Annot *annot);
  GBool removeAnnot(Annot *annot);

private:
  Annot* createAnnot(Dict* dict, Object *obj);
  Annot *findAnnot(Ref *ref);

  PDFDoc *doc;
  Annot **annots;
  int nAnnots;
  int size;
};

#endif
