//========================================================================
//
// This file comes from pdftohtml project
// http://pdftohtml.sourceforge.net
//
// Copyright from:
// Gueorgui Ovtcharov
// Rainer Dorsch <http://www.ra.informatik.uni-stuttgart.de/~rainer/>
// Mikhail Kruk <meshko@cs.brandeis.edu>
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2010 OSSD CDAC Mumbai by Leena Chourey (leenac@cdacmumbai.in) and Onkar Potdar (onkar@cdacmumbai.in)
// Copyright (C) 2010, 2012 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2011 Steven Murdoch <Steven.Murdoch@cl.cam.ac.uk>
// Copyright (C) 2011 Joshua Richardson <jric@chegg.com>
// Copyright (C) 2012 Igor Slepchin <igor.slepchin@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef _HTML_FONTS_H
#define _HTML_FONTS_H
#include "goo/GooString.h"
#include "GfxState.h"
#include "CharTypes.h"
#include <vector>

class HtmlFontColor{
 private:
   unsigned int r;
   unsigned int g;
   unsigned int b;
   GBool Ok(unsigned int xcol){ return xcol<=255;}
   GooString *convtoX(unsigned  int xcol) const;
 public:
   HtmlFontColor():r(0),g(0),b(0){}
   HtmlFontColor(GfxRGB rgb);
   HtmlFontColor(const HtmlFontColor& x){r=x.r;g=x.g;b=x.b;}
   HtmlFontColor& operator=(const HtmlFontColor &x){
     r=x.r;g=x.g;b=x.b;
     return *this;
   }
   ~HtmlFontColor(){};
   GooString* toString() const;
   GBool isEqual(const HtmlFontColor& col) const{
     return ((r==col.r)&&(g==col.g)&&(b==col.b));
   }
} ;  


class HtmlFont{
 private:
   unsigned int size;
   int lineSize;
   GBool italic;
   GBool bold;
   GBool rotOrSkewed;
   int pos; // position of the font name in the fonts array
   static GooString *DefaultFont;
   GooString *FontName;
   HtmlFontColor color;
   double rotSkewMat[4]; // only four values needed for rotation and skew
public:  

   HtmlFont(){FontName=NULL; rotOrSkewed = gFalse;}
   HtmlFont(GfxFont *font,int _size, GfxRGB rgb);
   HtmlFont(const HtmlFont& x);
   HtmlFont& operator=(const HtmlFont& x);
   HtmlFontColor getColor() const {return color;}
   ~HtmlFont();
   static void clear();
   GooString* getFullName();
   GBool isItalic() const {return italic;}
   GBool isBold() const {return bold;}
   GBool isRotOrSkewed() const { return rotOrSkewed; }
   unsigned int getSize() const {return size;}
   int getLineSize() const {return lineSize;}
   void setLineSize(int _lineSize) { lineSize = _lineSize; }
   void setRotMat(const double * const mat)
   { rotOrSkewed = gTrue; memcpy(rotSkewMat, mat, sizeof(rotSkewMat)); }
   const double *getRotMat() const { return rotSkewMat; }
   GooString* getFontName();
   static GooString* getDefaultFont();
   static void setDefaultFont(GooString* defaultFont);
   static GooString* HtmlFilter(Unicode* u, int uLen); //char* s);
   GBool isEqual(const HtmlFont& x) const;
   GBool isEqualIgnoreBold(const HtmlFont& x) const;
   static GooString* simple(HtmlFont *font, Unicode *content, int uLen);
   void print() const {printf("font: %s %d %s%spos: %d\n", FontName->getCString(), size, bold ? "bold " : "", italic ? "italic " : "", pos);};
};

class HtmlFontAccu{
private:
  std::vector<HtmlFont> *accu;
  
public:
  HtmlFontAccu();
  ~HtmlFontAccu();
  int AddFont(const HtmlFont& font);
  HtmlFont *Get(int i){
    return &(*accu)[i];
  } 
  GooString* CSStyle(int i, int j = 0);
  int size() const {return accu->size();}
  
};  
#endif
