//========================================================================
//
// ABWOutputDev.h
//
// Copyright 2006-2007 Jauco Noordzij <jauco@jauco.nl>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
//
//========================================================================

#ifndef ABWOUTPUTDEV_H
#define ABWOUTPUTDEV_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>
#include "goo/gtypes.h"
#include "goo/GooList.h"
#include "GfxFont.h"
#include "OutputDev.h"
#include "Catalog.h"
#include "UnicodeMap.h"
#include "PDFDoc.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#ifdef _WIN32
#  define SLASH '\\'
#else
#  define SLASH '/'
#endif

#define xoutRound(x) ((int)(x + 0.5))

class GfxState;
class GooString;

//------------------------------------------------------------------------
// ABWOutputDev
//------------------------------------------------------------------------

class ABWOutputDev: public OutputDev {
public:

  // Open a text output file.  If <fileName> is NULL, no file is written
  // (this is useful, e.g., for searching text).  If <useASCII7> is true,
  // text is converted to 7-bit ASCII; otherwise, text is converted to
  // 8-bit ISO Latin-1.  <useASCII7> should also be set for Japanese
  // (EUC-JP) text.  If <rawOrder> is true, the text is kept in content
  // stream order.
  ABWOutputDev(xmlDocPtr ext_doc);

  // Destructor.
  virtual ~ABWOutputDev();

  // Check if file was successfully created.
  virtual GBool isOk() { return gTrue; }

  //---- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gTrue; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() { return gTrue; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars() { return gFalse; }

  // Does this device need non-text content?
  virtual GBool needNonText() { return gFalse; }

  //----- initialization and control

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state);

  // End a page.
  virtual void endPage();

  //----- update text state
  virtual void updateFont(GfxState *state);

  //----- text drawing
  //new feature    
  virtual int DevType() {return 1234;}

  int getPageWidth() { return maxPageWidth; }
  int getPageHeight() { return maxPageHeight; }
  float getBiggestSeperator(xmlNodePtr N_set, unsigned int direction, float * C1, float * C2);
  void recursiveXYC(xmlNodePtr nodeset);
  void splitNodes(float splitValue, unsigned int direction, xmlNodePtr N_parent, double extravalue);
  virtual void beginString(GfxState *state, GooString *s);
  virtual void endString(GfxState *state);
  virtual void drawChar(GfxState *state, double x, double y,
			double dx, double dy,
			double originX, double originY,
			CharCode code, int nBytes, Unicode *u, int uLen);
  void beginWord(GfxState *state, double x, double y);
  void endWord();
  void beginTextBlock(GfxState *state, double x, double y);
  void endTextBlock();
  void interpretXYTree();
  void ATP_recursive(xmlNodePtr N_cur);
  void cleanUpNode(xmlNodePtr N_parent, bool aggregateInfo);
  void transformPage(xmlNodePtr N_parent);
  void generateParagraphs();
  void addAlignment(xmlNodePtr N_parent);
  void setPDFDoc(PDFDoc *priv_pdfdoc);
  void createABW();

private:
  int maxPageWidth;
  int maxPageHeight;
  int G_pageNum;
  int Style, maxStyle;
  //A lot of values are nice to have around. I think that declaring some 
  //global variables that contain these values is faster & easier than reading
  //them from the xml tree every time.
  double height;
  double wordSpace, charSpace;
  double X1,X2,Y1,Y2,horDist, verDist, curDx, curDy;
  bool mightBreak;
  xmlDocPtr doc;
  /* node pointers */
  xmlNodePtr N_root, N_content, N_page, N_style, N_text, N_styleset, N_Block, N_word, N_column, N_colset;
  xmlNodePtr outputDoc;
  xmlXPathContextPtr xpathCtx;
  static const unsigned int HORIZONTAL = 0;
  static const unsigned int VERTICAL = 1;
  UnicodeMap *uMap;
  PDFDoc *pdfdoc;
  int xmlLsCountNode(xmlNodePtr node);
};
#endif
