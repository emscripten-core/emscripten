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
// Copyright (C) 2010 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef _HTML_LINKS
#define _HTML_LINKS

#include <stdlib.h>
#include <string.h>
#include <vector>
#include "goo/GooString.h"

class HtmlLink{

private:  
  double Xmin;
  double Ymin;
  double Xmax;
  double Ymax;
  GooString* dest;

public:
  HtmlLink(const HtmlLink& x);
  HtmlLink(double xmin,double ymin,double xmax,double ymax,GooString *_dest);
  ~HtmlLink();
  GBool isEqualDest(const HtmlLink& x) const;
  GooString *getDest(){return new GooString(dest);}
  double getX1() const {return Xmin;}
  double getX2() const {return Xmax;}
  double getY1() const {return Ymin;}
  double getY2() const {return Ymax;}
  GBool inLink(double xmin,double ymin,double xmax,double ymax) const ;
  //GooString *Link(GooString *content);
  GooString* getLinkStart();
  
};

class HtmlLinks{
private:
 std::vector<HtmlLink> *accu;
public:
 HtmlLinks();
 ~HtmlLinks();
 void AddLink(const HtmlLink& x) {accu->push_back(x);}
 GBool inLink(double xmin,double ymin,double xmax,double ymax,int& p) const;
 HtmlLink* getLink(int i) const;

};

#endif
   
