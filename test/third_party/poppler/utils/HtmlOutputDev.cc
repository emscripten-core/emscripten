//========================================================================
//
// HtmlOutputDev.cc
//
// Copyright 1997-2002 Glyph & Cog, LLC
//
// Changed 1999-2000 by G.Ovtcharov
//
// Changed 2002 by Mikhail Kruk
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005-2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008 Kjartan Maraas <kmaraas@gnome.org>
// Copyright (C) 2008 Boris Toloknov <tlknv@yandex.ru>
// Copyright (C) 2008 Haruyuki Kawabe <Haruyuki.Kawabe@unisys.co.jp>
// Copyright (C) 2008 Tomas Are Haavet <tomasare@gmail.com>
// Copyright (C) 2009 Warren Toomey <wkt@tuhs.org>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 Reece Dunn <msclrhd@gmail.com>
// Copyright (C) 2010 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 OSSD CDAC Mumbai by Leena Chourey (leenac@cdacmumbai.in) and Onkar Potdar (onkar@cdacmumbai.in)
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <math.h>
#include "goo/GooString.h"
#include "goo/GooList.h"
#include "UnicodeMap.h"
#include "goo/gmem.h"
#include "Error.h"
#include "GfxState.h"
#include "Page.h"
#include "PNGWriter.h"
#ifdef ENABLE_LIBJPEG
#include "DCTStream.h"
#endif
#include "GlobalParams.h"
#include "HtmlOutputDev.h"
#include "HtmlFonts.h"

int HtmlPage::pgNum=0;
int HtmlOutputDev::imgNum=1;
GooList *HtmlOutputDev::imgList=new GooList();

extern GBool complexMode;
extern GBool singleHtml;
extern GBool ignore;
extern GBool printCommands;
extern GBool printHtml;
extern GBool noframes;
extern GBool stout;
extern GBool xml;
extern GBool showHidden;
extern GBool noMerge;

static GooString* basename(GooString* str){
  
  char *p=str->getCString();
  int len=str->getLength();
  for (int i=len-1;i>=0;i--)
    if (*(p+i)==SLASH) 
      return new GooString((p+i+1),len-i-1);
  return new GooString(str);
}

#if 0
static GooString* Dirname(GooString* str){
  
  char *p=str->getCString();
  int len=str->getLength();
  for (int i=len-1;i>=0;i--)
    if (*(p+i)==SLASH) 
      return new GooString(p,i+1);
  return new GooString();
} 
#endif

//------------------------------------------------------------------------
// HtmlString
//------------------------------------------------------------------------

HtmlString::HtmlString(GfxState *state, double fontSize, HtmlFontAccu* fonts) {
  GfxFont *font;
  double x, y;

  state->transform(state->getCurX(), state->getCurY(), &x, &y);
  if ((font = state->getFont())) {
    double ascent = font->getAscent();
    double descent = font->getDescent();
    if( ascent > 1.05 ){
        //printf( "ascent=%.15g is too high, descent=%.15g\n", ascent, descent );
        ascent = 1.05;
    }
    if( descent < -0.4 ){
        //printf( "descent %.15g is too low, ascent=%.15g\n", descent, ascent );
        descent = -0.4;
    }
    yMin = y - ascent * fontSize;
    yMax = y - descent * fontSize;
    GfxRGB rgb;
    state->getFillRGB(&rgb);
    GooString *name = state->getFont()->getName();
    if (!name) name = HtmlFont::getDefaultFont(); //new GooString("default");
    HtmlFont hfont=HtmlFont(name, static_cast<int>(fontSize-1), rgb);
    fontpos = fonts->AddFont(hfont);
  } else {
    // this means that the PDF file draws text without a current font,
    // which should never happen
    yMin = y - 0.95 * fontSize;
    yMax = y + 0.35 * fontSize;
    fontpos=0;
  }
  if (yMin == yMax) {
    // this is a sanity check for a case that shouldn't happen -- but
    // if it does happen, we want to avoid dividing by zero later
    yMin = y;
    yMax = y + 1;
  }
  col = 0;
  text = NULL;
  xRight = NULL;
  link = NULL;
  len = size = 0;
  yxNext = NULL;
  xyNext = NULL;
  htext=new GooString();
  dir = textDirUnknown;
}


HtmlString::~HtmlString() {
  gfree(text);
  delete htext;
  gfree(xRight);
}

void HtmlString::addChar(GfxState *state, double x, double y,
			 double dx, double dy, Unicode u) {
  if (dir == textDirUnknown) {
    //dir = UnicodeMap::getDirection(u);
    dir = textDirLeftRight;
  } 

  if (len == size) {
    size += 16;
    text = (Unicode *)grealloc(text, size * sizeof(Unicode));
    xRight = (double *)grealloc(xRight, size * sizeof(double));
  }
  text[len] = u;
  if (len == 0) {
    xMin = x;
  }
  xMax = xRight[len] = x + dx;
//printf("added char: %f %f xright = %f\n", x, dx, x+dx);
  ++len;
}

void HtmlString::endString()
{
  if( dir == textDirRightLeft && len > 1 )
  {
    //printf("will reverse!\n");
    for (int i = 0; i < len / 2; i++)
    {
      Unicode ch = text[i];
      text[i] = text[len - i - 1];
      text[len - i - 1] = ch;
    }
  }
}

//------------------------------------------------------------------------
// HtmlPage
//------------------------------------------------------------------------

HtmlPage::HtmlPage(GBool rawOrder, char *imgExtVal) {
  this->rawOrder = rawOrder;
  curStr = NULL;
  yxStrings = NULL;
  xyStrings = NULL;
  yxCur1 = yxCur2 = NULL;
  fonts=new HtmlFontAccu();
  links=new HtmlLinks();
  pageWidth=0;
  pageHeight=0;
  fontsPageMarker = 0;
  DocName=NULL;
  firstPage = -1;
  imgExt = new GooString(imgExtVal);
}

HtmlPage::~HtmlPage() {
  clear();
  if (DocName) delete DocName;
  if (fonts) delete fonts;
  if (links) delete links;
  if (imgExt) delete imgExt;  
}

void HtmlPage::updateFont(GfxState *state) {
  GfxFont *font;
  double *fm;
  char *name;
  int code;
  double w;
  
  // adjust the font size
  fontSize = state->getTransformedFontSize();
  if ((font = state->getFont()) && font->getType() == fontType3) {
    // This is a hack which makes it possible to deal with some Type 3
    // fonts.  The problem is that it's impossible to know what the
    // base coordinate system used in the font is without actually
    // rendering the font.  This code tries to guess by looking at the
    // width of the character 'm' (which breaks if the font is a
    // subset that doesn't contain 'm').
    for (code = 0; code < 256; ++code) {
      if ((name = ((Gfx8BitFont *)font)->getCharName(code)) &&
	  name[0] == 'm' && name[1] == '\0') {
	break;
      }
    }
    if (code < 256) {
      w = ((Gfx8BitFont *)font)->getWidth(code);
      if (w != 0) {
	// 600 is a generic average 'm' width -- yes, this is a hack
	fontSize *= w / 0.6;
      }
    }
    fm = font->getFontMatrix();
    if (fm[0] != 0) {
      fontSize *= fabs(fm[3] / fm[0]);
    }
  }
}

void HtmlPage::beginString(GfxState *state, GooString *s) {
  curStr = new HtmlString(state, fontSize, fonts);
}


void HtmlPage::conv(){
  HtmlString *tmp;

  int linkIndex = 0;
  HtmlFont* h;
  for(tmp=yxStrings;tmp;tmp=tmp->yxNext){
     int pos=tmp->fontpos;
     //  printf("%d\n",pos);
     h=fonts->Get(pos);

     if (tmp->htext) delete tmp->htext; 
     tmp->htext=HtmlFont::simple(h,tmp->text,tmp->len);

     if (links->inLink(tmp->xMin,tmp->yMin,tmp->xMax,tmp->yMax, linkIndex)){
       tmp->link = links->getLink(linkIndex);
       /*GooString *t=tmp->htext;
       tmp->htext=links->getLink(k)->Link(tmp->htext);
       delete t;*/
     }
  }

}


void HtmlPage::addChar(GfxState *state, double x, double y,
		       double dx, double dy, 
			double ox, double oy, Unicode *u, int uLen) {
  double x1, y1, w1, h1, dx2, dy2;
  int n, i;
  state->transform(x, y, &x1, &y1);
  n = curStr->len;
 
  // check that new character is in the same direction as current string
  // and is not too far away from it before adding 
  //if ((UnicodeMap::getDirection(u[0]) != curStr->dir) || 
  // XXX
  if (
     (n > 0 && 
      fabs(x1 - curStr->xRight[n-1]) > 0.1 * (curStr->yMax - curStr->yMin))) {
    endString();
    beginString(state, NULL);
  }
  state->textTransformDelta(state->getCharSpace() * state->getHorizScaling(),
			    0, &dx2, &dy2);
  dx -= dx2;
  dy -= dy2;
  state->transformDelta(dx, dy, &w1, &h1);
  if (uLen != 0) {
    w1 /= uLen;
    h1 /= uLen;
  }
  for (i = 0; i < uLen; ++i) {
    curStr->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, u[i]);
  }
}

void HtmlPage::endString() {
  HtmlString *p1, *p2;
  double h, y1, y2;

  // throw away zero-length strings -- they don't have valid xMin/xMax
  // values, and they're useless anyway
  if (curStr->len == 0) {
    delete curStr;
    curStr = NULL;
    return;
  }

  curStr->endString();

#if 0 //~tmp
  if (curStr->yMax - curStr->yMin > 20) {
    delete curStr;
    curStr = NULL;
    return;
  }
#endif

  // insert string in y-major list
  h = curStr->yMax - curStr->yMin;
  y1 = curStr->yMin + 0.5 * h;
  y2 = curStr->yMin + 0.8 * h;
  if (rawOrder) {
    p1 = yxCur1;
    p2 = NULL;
  } else if ((!yxCur1 ||
              (y1 >= yxCur1->yMin &&
               (y2 >= yxCur1->yMax || curStr->xMax >= yxCur1->xMin))) &&
             (!yxCur2 ||
              (y1 < yxCur2->yMin ||
               (y2 < yxCur2->yMax && curStr->xMax < yxCur2->xMin)))) {
    p1 = yxCur1;
    p2 = yxCur2;
  } else {
    for (p1 = NULL, p2 = yxStrings; p2; p1 = p2, p2 = p2->yxNext) {
      if (y1 < p2->yMin || (y2 < p2->yMax && curStr->xMax < p2->xMin))
        break;
    }
    yxCur2 = p2;
  }
  yxCur1 = curStr;
  if (p1)
    p1->yxNext = curStr;
  else
    yxStrings = curStr;
  curStr->yxNext = p2;
  curStr = NULL;
}

static const char *strrstr( const char *s, const char *ss )
{
  const char *p = strstr( s, ss );
  for( const char *pp = p; pp != NULL; pp = strstr( p+1, ss ) ){
    p = pp;
  }
  return p;
}

static void CloseTags( GooString *htext, GBool &finish_a, GBool &finish_italic, GBool &finish_bold )
{
  const char *last_italic = finish_italic && ( finish_bold   || finish_a    ) ? strrstr( htext->getCString(), "<i>" ) : NULL;
  const char *last_bold   = finish_bold   && ( finish_italic || finish_a    ) ? strrstr( htext->getCString(), "<b>" ) : NULL;
  const char *last_a      = finish_a      && ( finish_italic || finish_bold ) ? strrstr( htext->getCString(), "<a " ) : NULL;
  if( finish_a && ( finish_italic || finish_bold ) && last_a > ( last_italic > last_bold ? last_italic : last_bold ) ){
    htext->append("</a>", 4);
    finish_a = false;
  }
  if( finish_italic && finish_bold && last_italic > last_bold ){
    htext->append("</i>", 4);
    finish_italic = false;
  }
  if( finish_bold )
    htext->append("</b>", 4);
  if( finish_italic )
    htext->append("</i>", 4);
  if( finish_a )
    htext->append("</a>");
}

void HtmlPage::coalesce() {
  HtmlString *str1, *str2;
  HtmlFont *hfont1, *hfont2;
  double space, horSpace, vertSpace, vertOverlap;
  GBool addSpace, addLineBreak;
  int n, i;
  double curX, curY;

#if 0 //~ for debugging
  for (str1 = yxStrings; str1; str1 = str1->yxNext) {
    printf("x=%f..%f  y=%f..%f  size=%2d '",
	   str1->xMin, str1->xMax, str1->yMin, str1->yMax,
	   (int)(str1->yMax - str1->yMin));
    for (i = 0; i < str1->len; ++i) {
      fputc(str1->text[i] & 0xff, stdout);
    }
    printf("'\n");
  }
  printf("\n------------------------------------------------------------\n\n");
#endif
  str1 = yxStrings;

  if( !str1 ) return;

  //----- discard duplicated text (fake boldface, drop shadows)
  if( !complexMode )
  {	/* if not in complex mode get rid of duplicate strings */
	HtmlString *str3;
	GBool found;
  	while (str1)
	{
		double size = str1->yMax - str1->yMin;
		double xLimit = str1->xMin + size * 0.2;
		found = gFalse;
		for (str2 = str1, str3 = str1->yxNext;
			str3 && str3->xMin < xLimit;
			str2 = str3, str3 = str2->yxNext)
		{
			if (str3->len == str1->len &&
				!memcmp(str3->text, str1->text, str1->len * sizeof(Unicode)) &&
				fabs(str3->yMin - str1->yMin) < size * 0.2 &&
				fabs(str3->yMax - str1->yMax) < size * 0.2 &&
				fabs(str3->xMax - str1->xMax) < size * 0.2)
			{
				found = gTrue;
				//printf("found duplicate!\n");
				break;
			}
		}
		if (found)
		{
			str2->xyNext = str3->xyNext;
			str2->yxNext = str3->yxNext;
			delete str3;
		}
		else
		{
			str1 = str1->yxNext;
		}
	}		
  }	/*- !complexMode */
  
  str1 = yxStrings;
  
  hfont1 = getFont(str1);
  if( hfont1->isBold() )
    str1->htext->insert(0,"<b>",3);
  if( hfont1->isItalic() )
    str1->htext->insert(0,"<i>",3);
  if( str1->getLink() != NULL ) {
    GooString *ls = str1->getLink()->getLinkStart();
    str1->htext->insert(0, ls);
    delete ls;
  }
  curX = str1->xMin; curY = str1->yMin;

  while (str1 && (str2 = str1->yxNext)) {
    hfont2 = getFont(str2);
    space = str1->yMax - str1->yMin;
    horSpace = str2->xMin - str1->xMax;
    addLineBreak = !noMerge && (fabs(str1->xMin - str2->xMin) < 0.4);
    vertSpace = str2->yMin - str1->yMax;

//printf("coalesce %d %d %f? ", str1->dir, str2->dir, d);

    if (str2->yMin >= str1->yMin && str2->yMin <= str1->yMax)
    {
	vertOverlap = str1->yMax - str2->yMin;
    } else
    if (str2->yMax >= str1->yMin && str2->yMax <= str1->yMax)
    {
	vertOverlap = str2->yMax - str1->yMin;
    } else
    {
    	vertOverlap = 0;
    } 
    
    if (
	(
	 (
	  (
	   (rawOrder && vertOverlap > 0.5 * space) 
	   ||
	   (!rawOrder && str2->yMin < str1->yMax)
	  ) &&
	  (horSpace > -0.5 * space && horSpace < space)
	 ) ||
       	 (vertSpace >= 0 && vertSpace < 0.5 * space && addLineBreak)
	) &&
	(!complexMode || (hfont1->isEqualIgnoreBold(*hfont2))) && // in complex mode fonts must be the same, in other modes fonts do not metter
	str1->dir == str2->dir // text direction the same
       ) 
    {
//      printf("yes\n");
      n = str1->len + str2->len;
      if ((addSpace = horSpace > 0.1 * space)) {
        ++n;
      }
      if (addLineBreak) {
        ++n;
      }
  
      str1->size = (n + 15) & ~15;
      str1->text = (Unicode *)grealloc(str1->text,
				       str1->size * sizeof(Unicode));
      str1->xRight = (double *)grealloc(str1->xRight,
					str1->size * sizeof(double));
      if (addSpace) {
		  str1->text[str1->len] = 0x20;
		  str1->htext->append(xml?" ":"&nbsp;");
		  str1->xRight[str1->len] = str2->xMin;
		  ++str1->len;
      }
      if (addLineBreak) {
	  str1->text[str1->len] = '\n';
	  str1->htext->append("<br>");
	  str1->xRight[str1->len] = str2->xMin;
	  ++str1->len;
	  str1->yMin = str2->yMin;
	  str1->yMax = str2->yMax;
	  str1->xMax = str2->xMax;
	  int fontLineSize = hfont1->getLineSize();
	  int curLineSize = (int)(vertSpace + space); 
	  if( curLineSize != fontLineSize )
	  {
	      HtmlFont *newfnt = new HtmlFont(*hfont1);
	      newfnt->setLineSize(curLineSize);
	      str1->fontpos = fonts->AddFont(*newfnt);
	      delete newfnt;
	      hfont1 = getFont(str1);
	      // we have to reget hfont2 because it's location could have
	      // changed on resize
	      hfont2 = getFont(str2); 
	  }
      }
      for (i = 0; i < str2->len; ++i) {
	str1->text[str1->len] = str2->text[i];
	str1->xRight[str1->len] = str2->xRight[i];
	++str1->len;
      }

      /* fix <i>, <b> if str1 and str2 differ and handle switch of links */
      HtmlLink *hlink1 = str1->getLink();
      HtmlLink *hlink2 = str2->getLink();
      bool switch_links = !hlink1 || !hlink2 || !hlink1->isEqualDest(*hlink2);
      GBool finish_a = switch_links && hlink1 != NULL;
      GBool finish_italic = hfont1->isItalic() && ( !hfont2->isItalic() || finish_a );
      GBool finish_bold   = hfont1->isBold()   && ( !hfont2->isBold()   || finish_a || finish_italic );
      CloseTags( str1->htext, finish_a, finish_italic, finish_bold );
      if( switch_links && hlink2 != NULL ) {
        GooString *ls = hlink2->getLinkStart();
        str1->htext->append(ls);
        delete ls;
      }
      if( ( !hfont1->isItalic() || finish_italic ) && hfont2->isItalic() )
	    str1->htext->append("<i>", 3);
      if( ( !hfont1->isBold() || finish_bold ) && hfont2->isBold() )
	    str1->htext->append("<b>", 3);


      str1->htext->append(str2->htext);
      // str1 now contains href for link of str2 (if it is defined)
      str1->link = str2->link; 
      hfont1 = hfont2;
      if (str2->xMax > str1->xMax) {
	str1->xMax = str2->xMax;
      }
      if (str2->yMax > str1->yMax) {
	str1->yMax = str2->yMax;
      }
      str1->yxNext = str2->yxNext;
      delete str2;
    } else { // keep strings separate
//      printf("no\n"); 
      GBool finish_a = str1->getLink() != NULL;
      GBool finish_bold   = hfont1->isBold();
      GBool finish_italic = hfont1->isItalic();
      CloseTags( str1->htext, finish_a, finish_italic, finish_bold );
     
      str1->xMin = curX; str1->yMin = curY; 
      str1 = str2;
      curX = str1->xMin; curY = str1->yMin;
      hfont1 = hfont2;
      if( hfont1->isBold() )
	str1->htext->insert(0,"<b>",3);
      if( hfont1->isItalic() )
	str1->htext->insert(0,"<i>",3);
      if( str1->getLink() != NULL ) {
	GooString *ls = str1->getLink()->getLinkStart();
	str1->htext->insert(0, ls);
	delete ls;
      }
    }
  }
  str1->xMin = curX; str1->yMin = curY;

  GBool finish_bold   = hfont1->isBold();
  GBool finish_italic = hfont1->isItalic();
  GBool finish_a = str1->getLink() != NULL;
  CloseTags( str1->htext, finish_a, finish_italic, finish_bold );

#if 0 //~ for debugging
  for (str1 = yxStrings; str1; str1 = str1->yxNext) {
    printf("x=%3d..%3d  y=%3d..%3d  size=%2d ",
	   (int)str1->xMin, (int)str1->xMax, (int)str1->yMin, (int)str1->yMax,
	   (int)(str1->yMax - str1->yMin));
    printf("'%s'\n", str1->htext->getCString());  
  }
  printf("\n------------------------------------------------------------\n\n");
#endif

}

void HtmlPage::dumpAsXML(FILE* f,int page){  
  fprintf(f, "<page number=\"%d\" position=\"absolute\"", page);
  fprintf(f," top=\"0\" left=\"0\" height=\"%d\" width=\"%d\">\n", pageHeight,pageWidth);
    
  for(int i=fontsPageMarker;i < fonts->size();i++) {
    GooString *fontCSStyle = fonts->CSStyle(i);
    fprintf(f,"\t%s\n",fontCSStyle->getCString());
    delete fontCSStyle;
  }
  
  GooString *str, *str1 = NULL;
  for(HtmlString *tmp=yxStrings;tmp;tmp=tmp->yxNext){
    if (tmp->htext){
      str=new GooString(tmp->htext);
      fprintf(f,"<text top=\"%d\" left=\"%d\" ",xoutRound(tmp->yMin),xoutRound(tmp->xMin));
      fprintf(f,"width=\"%d\" height=\"%d\" ",xoutRound(tmp->xMax-tmp->xMin),xoutRound(tmp->yMax-tmp->yMin));
      fprintf(f,"font=\"%d\">", tmp->fontpos);
      str1=fonts->getCSStyle(tmp->fontpos, str);
      fputs(str1->getCString(),f);
      delete str;
      delete str1;
      fputs("</text>\n",f);
    }
  }
  fputs("</page>\n",f);
}


void HtmlPage::dumpComplex(FILE *file, int page){
  FILE* pageFile;
  GooString* tmp;
  char* htmlEncoding;

  if( firstPage == -1 ) firstPage = page; 
  
  if( !noframes )
  {
      GooString* pgNum=GooString::fromInt(page);
      tmp = new GooString(DocName);
      if (!singleHtml){
            tmp->append('-')->append(pgNum)->append(".html");
            pageFile = fopen(tmp->getCString(), "w");
      } else {
            tmp->append("-html")->append(".html");
            pageFile = fopen(tmp->getCString(), "a");
      }
      delete pgNum;
      if (!pageFile) {
	  error(-1, "Couldn't open html file '%s'", tmp->getCString());
	  delete tmp;
	  return;
      } 

      if (!singleHtml)
          fprintf(pageFile,"%s\n<HTML>\n<HEAD>\n<TITLE>Page %d</TITLE>\n\n", DOCTYPE, page);
      else
          fprintf(pageFile,"%s\n<HTML>\n<HEAD>\n<TITLE>%s</TITLE>\n\n", DOCTYPE, tmp->getCString());

      delete tmp;

      htmlEncoding = HtmlOutputDev::mapEncodingToHtml
	  (globalParams->getTextEncodingName());
      if (!singleHtml)
          fprintf(pageFile, "<META http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">\n", htmlEncoding);
      else
          fprintf(pageFile, "<META http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">\n <br>\n", htmlEncoding);
  }
  else 
  {
      pageFile = file;
      fprintf(pageFile,"<!-- Page %d -->\n", page);
      fprintf(pageFile,"<a name=\"%d\"></a>\n", page);
  } 
  
  fprintf(pageFile,"<DIV style=\"position:relative;width:%d;height:%d;\">\n",
	pageWidth, pageHeight);

  tmp=basename(DocName);
   
  fputs("<STYLE type=\"text/css\">\n<!--\n",pageFile);
  for(int i=fontsPageMarker;i!=fonts->size();i++) {
    GooString *fontCSStyle;
    if (!singleHtml)
         fontCSStyle = fonts->CSStyle(i);
    else
         fontCSStyle = fonts->CSStyle(i,page);
    fprintf(pageFile,"\t%s\n",fontCSStyle->getCString());
    delete fontCSStyle;
  }
 
  fputs("-->\n</STYLE>\n",pageFile);
  
  if( !noframes )
  {  
      fputs("</HEAD>\n<BODY bgcolor=\"#A0A0A0\" vlink=\"blue\" link=\"blue\">\n",pageFile); 
  }
  
  if( !ignore ) 
  {
    fprintf(pageFile,
	    "<IMG width=\"%d\" height=\"%d\" src=\"%s%03d.%s\" alt=\"background image\">\n",
	    pageWidth, pageHeight, tmp->getCString(), 
		(page-firstPage+1), imgExt->getCString());
  }
  
  delete tmp;
  
  GooString *str, *str1 = NULL;
  for(HtmlString *tmp1=yxStrings;tmp1;tmp1=tmp1->yxNext){
    if (tmp1->htext){
      str=new GooString(tmp1->htext);
      fprintf(pageFile,
	      "<DIV style=\"position:absolute;top:%d;left:%d\">",
	      xoutRound(tmp1->yMin),
	      xoutRound(tmp1->xMin));
      fputs("<nobr>",pageFile); 
      if (!singleHtml)
          str1=fonts->getCSStyle(tmp1->fontpos, str);
      else
          str1=fonts->getCSStyle(tmp1->fontpos, str, page);
      fputs(str1->getCString(),pageFile);
      delete str;      
      delete str1;
      fputs("</nobr></DIV>\n",pageFile);
    }
  }

  fputs("</DIV>\n", pageFile);
  
  if( !noframes )
  {
      fputs("</BODY>\n</HTML>\n",pageFile);
      fclose(pageFile);
  }
}


void HtmlPage::dump(FILE *f, int pageNum) 
{
  if (complexMode || singleHtml)
  {
    if (xml) dumpAsXML(f, pageNum);
    if (!xml) dumpComplex(f, pageNum);  
  }
  else
  {
    fprintf(f,"<A name=%d></a>",pageNum);
    // Loop over the list of image names on this page
    int listlen=HtmlOutputDev::imgList->getLength();
    for (int i = 0; i < listlen; i++) {
      GooString *fName= (GooString *)HtmlOutputDev::imgList->del(0);
      fprintf(f,"<IMG src=\"%s\"><br>\n",fName->getCString());
      delete fName;
    }
    HtmlOutputDev::imgNum=1;

    GooString* str;
    for(HtmlString *tmp=yxStrings;tmp;tmp=tmp->yxNext){
      if (tmp->htext){
		str=new GooString(tmp->htext); 
		fputs(str->getCString(),f);
		delete str;      
		fputs("<br>\n",f);  
      }
    }
	fputs("<hr>\n",f);  
  }
}



void HtmlPage::clear() {
  HtmlString *p1, *p2;

  if (curStr) {
    delete curStr;
    curStr = NULL;
  }
  for (p1 = yxStrings; p1; p1 = p2) {
    p2 = p1->yxNext;
    delete p1;
  }
  yxStrings = NULL;
  xyStrings = NULL;
  yxCur1 = yxCur2 = NULL;

  if( !noframes )
  {
      delete fonts;
      fonts=new HtmlFontAccu();
      fontsPageMarker = 0;
  }
  else
  {
      fontsPageMarker = fonts->size();
  }

  delete links;
  links=new HtmlLinks();
 

}

void HtmlPage::setDocName(char *fname){
  DocName=new GooString(fname);
}

//------------------------------------------------------------------------
// HtmlMetaVar
//------------------------------------------------------------------------

HtmlMetaVar::HtmlMetaVar(char *_name, char *_content)
{
    name = new GooString(_name);
    content = new GooString(_content);
}

HtmlMetaVar::~HtmlMetaVar()
{
   delete name;
   delete content;
} 
    
GooString* HtmlMetaVar::toString()	
{
    GooString *result = new GooString("<META name=\"");
    result->append(name);
    result->append("\" content=\"");
    result->append(content);
    result->append("\">"); 
    return result;
}

//------------------------------------------------------------------------
// HtmlOutputDev
//------------------------------------------------------------------------

static char* HtmlEncodings[][2] = {
    {"Latin1", "ISO-8859-1"},
    {NULL, NULL}
};


char* HtmlOutputDev::mapEncodingToHtml(GooString* encoding)
{
    char* enc = encoding->getCString();
    for(int i = 0; HtmlEncodings[i][0] != NULL; i++)
    {
	if( strcmp(enc, HtmlEncodings[i][0]) == 0 )
	{
	    return HtmlEncodings[i][1];
	}
    }
    return enc; 
}

void HtmlOutputDev::doFrame(int firstPage){
  GooString* fName=new GooString(Docname);
  char* htmlEncoding;
  fName->append(".html");

  if (!(fContentsFrame = fopen(fName->getCString(), "w"))){
    error(-1, "Couldn't open html file '%s'", fName->getCString());
    delete fName;
    return;
  }
  
  delete fName;
    
  fName=basename(Docname);
  fputs(DOCTYPE_FRAMES, fContentsFrame);
  fputs("\n<HTML>",fContentsFrame);
  fputs("\n<HEAD>",fContentsFrame);
  fprintf(fContentsFrame,"\n<TITLE>%s</TITLE>",docTitle->getCString());
  htmlEncoding = mapEncodingToHtml(globalParams->getTextEncodingName());
  fprintf(fContentsFrame, "\n<META http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">\n", htmlEncoding);
  dumpMetaVars(fContentsFrame);
  fprintf(fContentsFrame, "</HEAD>\n");
  fputs("<FRAMESET cols=\"100,*\">\n",fContentsFrame);
  fprintf(fContentsFrame,"<FRAME name=\"links\" src=\"%s_ind.html\">\n",fName->getCString());
  fputs("<FRAME name=\"contents\" src=",fContentsFrame); 
  if (complexMode) 
      fprintf(fContentsFrame,"\"%s-%d.html\"",fName->getCString(), firstPage);
  else
      fprintf(fContentsFrame,"\"%ss.html\"",fName->getCString());
  
  fputs(">\n</FRAMESET>\n</HTML>\n",fContentsFrame);
 
  delete fName;
  fclose(fContentsFrame);  
}

HtmlOutputDev::HtmlOutputDev(char *fileName, char *title, 
	char *author, char *keywords, char *subject, char *date,
	char *extension,
	GBool rawOrder, int firstPage, GBool outline) 
{
  char *htmlEncoding;
  
  fContentsFrame = NULL;
  docTitle = new GooString(title);
  pages = NULL;
  dumpJPEG=gTrue;
  //write = gTrue;
  this->rawOrder = rawOrder;
  this->doOutline = outline;
  ok = gFalse;
  imgNum=1;
  //this->firstPage = firstPage;
  //pageNum=firstPage;
  // open file
  needClose = gFalse;
  pages = new HtmlPage(rawOrder, extension);
  
  glMetaVars = new GooList();
  glMetaVars->append(new HtmlMetaVar("generator", "pdftohtml 0.36"));  
  if( author ) glMetaVars->append(new HtmlMetaVar("author", author));  
  if( keywords ) glMetaVars->append(new HtmlMetaVar("keywords", keywords));  
  if( date ) glMetaVars->append(new HtmlMetaVar("date", date));  
  if( subject ) glMetaVars->append(new HtmlMetaVar("subject", subject));
 
  maxPageWidth = 0;
  maxPageHeight = 0;

  pages->setDocName(fileName);
  Docname=new GooString (fileName);

  // for non-xml output (complex or simple) with frames generate the left frame
  if(!xml && !noframes)
  {
     if (!singleHtml)
     {
         GooString* left=new GooString(fileName);
         left->append("_ind.html");

         doFrame(firstPage);

         if (!(fContentsFrame = fopen(left->getCString(), "w")))
         {
             error(-1, "Couldn't open html file '%s'", left->getCString());
             delete left;
             return;
         }
         delete left;
         fputs(DOCTYPE, fContentsFrame);
         fputs("<HTML>\n<HEAD>\n<TITLE></TITLE>\n</HEAD>\n<BODY>\n",fContentsFrame);

         if (doOutline)
         {
             GooString *str = basename(Docname);
             fprintf(fContentsFrame, "<A href=\"%s%s\" target=\"contents\">Outline</a><br>", str->getCString(), complexMode ? "-outline.html" : "s.html#outline");
             delete str;
         }
     }
	if (!complexMode)
	{	/* not in complex mode */
		
       GooString* right=new GooString(fileName);
       right->append("s.html");

       if (!(page=fopen(right->getCString(),"w"))){
        error(-1, "Couldn't open html file '%s'", right->getCString());
        delete right;
		return;
       }
       delete right;
       fputs(DOCTYPE, page);
       fputs("<HTML>\n<HEAD>\n<TITLE></TITLE>\n</HEAD>\n<BODY>\n",page);
     }
  }

  if (noframes) {
    if (stout) page=stdout;
    else {
      GooString* right=new GooString(fileName);
      if (!xml) right->append(".html");
      if (xml) right->append(".xml");
      if (!(page=fopen(right->getCString(),"w"))){
	error(-1, "Couldn't open html file '%s'", right->getCString());
	delete right;
	return;
      }  
      delete right;
    }

    htmlEncoding = mapEncodingToHtml(globalParams->getTextEncodingName()); 
    if (xml) 
    {
      fprintf(page, "<?xml version=\"1.0\" encoding=\"%s\"?>\n", htmlEncoding);
      fputs("<!DOCTYPE pdf2xml SYSTEM \"pdf2xml.dtd\">\n\n", page);
      fputs("<pdf2xml>\n",page);
    } 
    else 
    {
      fprintf(page,"%s\n<HTML>\n<HEAD>\n<TITLE>%s</TITLE>\n",
	      DOCTYPE, docTitle->getCString());
      
      fprintf(page, "<META http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">\n", htmlEncoding);
      
      dumpMetaVars(page);
      fprintf(page,"</HEAD>\n");
      fprintf(page,"<BODY bgcolor=\"#A0A0A0\" vlink=\"blue\" link=\"blue\">\n");
    }
  }
  ok = gTrue; 
}

HtmlOutputDev::~HtmlOutputDev() {
    HtmlFont::clear(); 
    
    delete Docname;
    delete docTitle;

    deleteGooList(glMetaVars, HtmlMetaVar);

    if (fContentsFrame){
      fputs("</BODY>\n</HTML>\n",fContentsFrame);  
      fclose(fContentsFrame);
    }
    if (xml) {
      fputs("</pdf2xml>\n",page);  
      fclose(page);
    } else
    if ( !complexMode || xml || noframes )
    { 
      fputs("</BODY>\n</HTML>\n",page);  
      fclose(page);
    }
    if (pages)
      delete pages;
}

void HtmlOutputDev::startPage(int pageNum, GfxState *state) {
#if 0
  if (mode&&!xml){
    if (write){
      write=gFalse;
      GooString* fname=Dirname(Docname);
      fname->append("image.log");
      if((tin=fopen(getFileNameFromPath(fname->getCString(),fname->getLength()),"w"))==NULL){
	printf("Error : can not open %s",fname);
	exit(1);
      }
      delete fname;
    // if(state->getRotation()!=0) 
    //  fprintf(tin,"ROTATE=%d rotate %d neg %d neg translate\n",state->getRotation(),state->getX1(),-state->getY1());
    // else 
      fprintf(tin,"ROTATE=%d neg %d neg translate\n",state->getX1(),state->getY1());  
    }
  }
#endif

  this->pageNum = pageNum;
  GooString *str=basename(Docname);
  pages->clear(); 
  if(!noframes)
  {
    if (fContentsFrame)
	{
      if (complexMode)
		fprintf(fContentsFrame,"<A href=\"%s-%d.html\"",str->getCString(),pageNum);
      else 
		fprintf(fContentsFrame,"<A href=\"%ss.html#%d\"",str->getCString(),pageNum);
      fprintf(fContentsFrame," target=\"contents\" >Page %d</a><br>\n",pageNum);
    }
  }

  pages->pageWidth=static_cast<int>(state->getPageWidth());
  pages->pageHeight=static_cast<int>(state->getPageHeight());

  delete str;
} 


void HtmlOutputDev::endPage() {
  Links *linksList = docPage->getLinks(catalog);
  for (int i = 0; i < linksList->getNumLinks(); ++i)
  {
      doProcessLink(linksList->getLink(i));
  }
  delete linksList;

  pages->conv();
  pages->coalesce();
  pages->dump(page, pageNum);
  
  // I don't yet know what to do in the case when there are pages of different
  // sizes and we want complex output: running ghostscript many times 
  // seems very inefficient. So for now I'll just use last page's size
  maxPageWidth = pages->pageWidth;
  maxPageHeight = pages->pageHeight;
  
  //if(!noframes&&!xml) fputs("<br>\n", fContentsFrame);
  if(!stout && !globalParams->getErrQuiet()) printf("Page-%d\n",(pageNum));
}

void HtmlOutputDev::updateFont(GfxState *state) {
  pages->updateFont(state);
}

void HtmlOutputDev::beginString(GfxState *state, GooString *s) {
  pages->beginString(state, s);
}

void HtmlOutputDev::endString(GfxState *state) {
  pages->endString();
}

void HtmlOutputDev::drawChar(GfxState *state, double x, double y,
	      double dx, double dy,
	      double originX, double originY,
	      CharCode code, int /*nBytes*/, Unicode *u, int uLen) 
{
  if ( !showHidden && (state->getRender() & 3) == 3) {
    return;
  }
  pages->addChar(state, x, y, dx, dy, originX, originY, u, uLen);
}

void HtmlOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
				  int width, int height, GBool invert,
				  GBool interpolate, GBool inlineImg) {

  if (ignore||complexMode) {
    OutputDev::drawImageMask(state, ref, str, width, height, invert, interpolate, inlineImg);
    return;
  }
  
  FILE *f1;
  int c;
  
  int x0, y0;			// top left corner of image
  int w0, h0, w1, h1;		// size of image
  double xt, yt, wt, ht;
  GBool rotate, xFlip, yFlip;
 
  // get image position and size
  state->transform(0, 0, &xt, &yt);
  state->transformDelta(1, 1, &wt, &ht);
  if (wt > 0) {
    x0 = xoutRound(xt);
    w0 = xoutRound(wt);
  } else {
    x0 = xoutRound(xt + wt);
    w0 = xoutRound(-wt);
  }
  if (ht > 0) {
    y0 = xoutRound(yt);
    h0 = xoutRound(ht);
  } else {
    y0 = xoutRound(yt + ht);
    h0 = xoutRound(-ht);
  }
  state->transformDelta(1, 0, &xt, &yt);
  rotate = fabs(xt) < fabs(yt);
  if (rotate) {
    w1 = h0;
    h1 = w0;
    xFlip = ht < 0;
    yFlip = wt > 0;
  } else {
    w1 = w0;
    h1 = h0;
    xFlip = wt < 0;
    yFlip = ht > 0;
  }

  // dump JPEG file
  if (dumpJPEG  && str->getKind() == strDCT) {
    GooString *fName=new GooString(Docname);
    fName->append("-");
    GooString *pgNum=GooString::fromInt(pageNum);
    GooString *imgnum=GooString::fromInt(imgNum);
    // open the image file
    fName->append(pgNum)->append("_")->append(imgnum)->append(".jpg");
    delete pgNum;
    delete imgnum;

    ++imgNum;
    if (!(f1 = fopen(fName->getCString(), "wb"))) {
      error(-1, "Couldn't open image file '%s'", fName->getCString());
      delete fName;
      return;
    }

    // initialize stream
    str = ((DCTStream *)str)->getRawStream();
    str->reset();

    // copy the stream
    while ((c = str->getChar()) != EOF)
      fputc(c, f1);

    fclose(f1);
   
  if (fName) imgList->append(fName);
  }
  else {
    OutputDev::drawImageMask(state, ref, str, width, height, invert, interpolate, inlineImg);
  }
}

void HtmlOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
			      int width, int height, GfxImageColorMap *colorMap,
			      GBool interpolate, int *maskColors, GBool inlineImg) {

  if (ignore||complexMode) {
    OutputDev::drawImage(state, ref, str, width, height, colorMap, interpolate,
			 maskColors, inlineImg);
    return;
  }

  FILE *f1;
  int c;
  
  int x0, y0;			// top left corner of image
  int w0, h0, w1, h1;		// size of image
  double xt, yt, wt, ht;
  GBool rotate, xFlip, yFlip;
 
  // get image position and size
  state->transform(0, 0, &xt, &yt);
  state->transformDelta(1, 1, &wt, &ht);
  if (wt > 0) {
    x0 = xoutRound(xt);
    w0 = xoutRound(wt);
  } else {
    x0 = xoutRound(xt + wt);
    w0 = xoutRound(-wt);
  }
  if (ht > 0) {
    y0 = xoutRound(yt);
    h0 = xoutRound(ht);
  } else {
    y0 = xoutRound(yt + ht);
    h0 = xoutRound(-ht);
  }
  state->transformDelta(1, 0, &xt, &yt);
  rotate = fabs(xt) < fabs(yt);
  if (rotate) {
    w1 = h0;
    h1 = w0;
    xFlip = ht < 0;
    yFlip = wt > 0;
  } else {
    w1 = w0;
    h1 = h0;
    xFlip = wt < 0;
    yFlip = ht > 0;
  }

   
  /*if( !globalParams->getErrQuiet() )
    printf("image stream of kind %d\n", str->getKind());*/
  // dump JPEG file
  if (dumpJPEG && str->getKind() == strDCT) {
    GooString *fName=new GooString(Docname);
    fName->append("-");
    GooString *pgNum= GooString::fromInt(pageNum);
    GooString *imgnum= GooString::fromInt(imgNum);  
    
    // open the image file
    fName->append(pgNum)->append("_")->append(imgnum)->append(".jpg");
    delete pgNum;
    delete imgnum;

    ++imgNum;
    
    if (!(f1 = fopen(fName->getCString(), "wb"))) {
      error(-1, "Couldn't open image file '%s'", fName->getCString());
      delete fName;
      return;
    }

    // initialize stream
    str = ((DCTStream *)str)->getRawStream();
    str->reset();

    // copy the stream
    while ((c = str->getChar()) != EOF)
      fputc(c, f1);
    
    fclose(f1);
  
    if (fName) imgList->append(fName);
  }
  else {
#ifdef ENABLE_LIBPNG
    // Dump the image as a PNG file. Much of the PNG code
    // comes from an example by Guillaume Cottenceau.
    Guchar *p;
    GfxRGB rgb;
    png_byte *row = (png_byte *) malloc(3 * width);   // 3 bytes/pixel: RGB
    png_bytep *row_pointer= &row;

    // Create the image filename
    GooString *fName=new GooString(Docname);
    fName->append("-");
    GooString *pgNum= GooString::fromInt(pageNum);
    GooString *imgnum= GooString::fromInt(imgNum);  
    fName->append(pgNum)->append("_")->append(imgnum)->append(".png");
    delete pgNum;
    delete imgnum;

    // Open the image file
    if (!(f1 = fopen(fName->getCString(), "wb"))) {
      error(-1, "Couldn't open image file '%s'", fName->getCString());
      delete fName;
      return;
    }

    PNGWriter *writer = new PNGWriter();
    // TODO can we calculate the resolution of the image?
    if (!writer->init(f1, width, height, 72, 72)) {
        delete writer;
        fclose(f1);
        return;
    }

    // Initialize the image stream
    ImageStream *imgStr = new ImageStream(str, width,
                        colorMap->getNumPixelComps(), colorMap->getBits());
    imgStr->reset();

    // For each line...
    for (int y = 0; y < height; y++) {

      // Convert into a PNG row
      p = imgStr->getLine();
      for (int x = 0; x < width; x++) {
        colorMap->getRGB(p, &rgb);
	// Write the RGB pixels into the row
	row[3*x]= colToByte(rgb.r);
	row[3*x+1]= colToByte(rgb.g);
	row[3*x+2]= colToByte(rgb.b);
         p += colorMap->getNumPixelComps();
      }

      if (!writer->writeRow(row_pointer)) {
        delete writer;
        fclose(f1);
        return;
      }
    }

    writer->close();
    delete writer;
    fclose(f1);

    free(row);
    imgList->append(fName);
    ++imgNum;
    imgStr->close();
    delete imgStr;
#else
    OutputDev::drawImage(state, ref, str, width, height, colorMap, interpolate,
			 maskColors, inlineImg);
#endif
  }
}



void HtmlOutputDev::doProcessLink(Link* link){
  double _x1,_y1,_x2,_y2;
  int x1,y1,x2,y2;
  
  link->getRect(&_x1,&_y1,&_x2,&_y2);
  cvtUserToDev(_x1,_y1,&x1,&y1);
  
  cvtUserToDev(_x2,_y2,&x2,&y2); 


  GooString* _dest=getLinkDest(link,catalog);
  HtmlLink t((double) x1,(double) y2,(double) x2,(double) y1,_dest);
  pages->AddLink(t);
  delete _dest;
}

GooString* HtmlOutputDev::getLinkDest(Link *link,Catalog* catalog){
  char *p;
  switch(link->getAction()->getKind()) 
  {
      case actionGoTo:
	  { 
	  GooString* file=basename(Docname);
	  int page=1;
	  LinkGoTo *ha=(LinkGoTo *)link->getAction();
	  LinkDest *dest=NULL;
	  if (ha->getDest()!=NULL)
	      dest=ha->getDest()->copy();
	  else if (ha->getNamedDest()!=NULL)
	      dest=catalog->findDest(ha->getNamedDest());
	      
	  if (dest){ 
	      if (dest->isPageRef()){
		  Ref pageref=dest->getPageRef();
		  page=catalog->findPage(pageref.num,pageref.gen);
	      }
	      else {
		  page=dest->getPageNum();
	      }

	      delete dest;

	      GooString *str=GooString::fromInt(page);
	      /* 		complex 	simple
	       	frames		file-4.html	files.html#4
		noframes	file.html#4	file.html#4
	       */
	      if (noframes)
	      {
		  file->append(".html#");
		  file->append(str);
	      }
	      else
	      {
	      	if( complexMode ) 
		{
		    file->append("-");
		    file->append(str);
		    file->append(".html");
		}
		else
		{
		    file->append("s.html#");
		    file->append(str);
		}
	      }

	      if (printCommands) printf(" link to page %d ",page);
	      delete str;
	      return file;
	  }
	  else 
	  {
	      return new GooString();
	  }
	  }
      case actionGoToR:
	  {
	  LinkGoToR *ha=(LinkGoToR *) link->getAction();
	  LinkDest *dest=NULL;
	  int page=1;
	  GooString *file=new GooString();
	  if (ha->getFileName()){
	      delete file;
	      file=new GooString(ha->getFileName()->getCString());
	  }
	  if (ha->getDest()!=NULL)  dest=ha->getDest()->copy();
	  if (dest&&file){
	      if (!(dest->isPageRef()))  page=dest->getPageNum();
	      delete dest;

	      if (printCommands) printf(" link to page %d ",page);
	      if (printHtml){
		  p=file->getCString()+file->getLength()-4;
		  if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")){
		      file->del(file->getLength()-4,4);
		      file->append(".html");
		  }
		  file->append('#');
		  file->append(GooString::fromInt(page));
	      }
	  }
	  if (printCommands && file) printf("filename %s\n",file->getCString());
	  return file;
	  }
      case actionURI:
	  { 
	  LinkURI *ha=(LinkURI *) link->getAction();
	  GooString* file=new GooString(ha->getURI()->getCString());
	  // printf("uri : %s\n",file->getCString());
	  return file;
	  }
      case actionLaunch:
	  {
	  LinkLaunch *ha=(LinkLaunch *) link->getAction();
	  GooString* file=new GooString(ha->getFileName()->getCString());
	  if (printHtml) { 
	      p=file->getCString()+file->getLength()-4;
	      if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")){
		  file->del(file->getLength()-4,4);
		  file->append(".html");
	      }
	      if (printCommands) printf("filename %s",file->getCString());
    
	      return file;      
  
	  }
	  }
      default:
	  return new GooString();
  }
}

void HtmlOutputDev::dumpMetaVars(FILE *file)
{
  GooString *var;

  for(int i = 0; i < glMetaVars->getLength(); i++)
  {
     HtmlMetaVar *t = (HtmlMetaVar*)glMetaVars->get(i); 
     var = t->toString(); 
     fprintf(file, "%s\n", var->getCString());
     delete var;
  }
}

GBool HtmlOutputDev::dumpDocOutline(Catalog* catalog)
{ 
	FILE * output = NULL;
	GBool bClose = gFalse;

	if (!ok || xml)
    	return gFalse;
  
	Object *outlines = catalog->getOutline();
  	if (!outlines->isDict())
    	return gFalse;
  
	if (!complexMode && !xml)
  	{
		output = page;
  	}
  	else if (complexMode && !xml)
	{
		if (noframes)
		{
			output = page; 
			fputs("<hr>\n", output);
		}
		else
		{
			GooString *str = Docname->copy();
			str->append("-outline.html");
			output = fopen(str->getCString(), "w");
			if (output == NULL)
				return gFalse;
			delete str;
			bClose = gTrue;
     		fputs("<HTML>\n<HEAD>\n<TITLE>Document Outline</TITLE>\n</HEAD>\n<BODY>\n", output);
		}
	}
 
  	GBool done = newOutlineLevel(output, outlines, catalog);
  	if (done && !complexMode)
    	fputs("<hr>\n", output);
	
	if (bClose)
	{
		fputs("</BODY>\n</HTML>\n", output);
		fclose(output);
	}
  	return done;
}

GBool HtmlOutputDev::newOutlineLevel(FILE *output, Object *node, Catalog* catalog, int level)
{
  Object curr, next;
  GBool atLeastOne = gFalse;
  
  if (node->dictLookup("First", &curr)->isDict()) {
    if (level == 1)
	{
		fputs("<A name=\"outline\"></a>", output);
		fputs("<h1>Document Outline</h1>\n", output);
	}
    fputs("<ul>",output);
    do {
      // get title, give up if not found
      Object title;
      if (curr.dictLookup("Title", &title)->isNull()) {
		title.free();
		break;
      }
      GooString *titleStr = new GooString(title.getString());
      title.free();

      // get corresponding link
      // Note: some code duplicated from HtmlOutputDev::getLinkDest().
      GooString *linkName = NULL;;
      Object dest;
      if (!curr.dictLookup("Dest", &dest)->isNull()) {
		LinkGoTo *link = new LinkGoTo(&dest);
		LinkDest *linkdest=NULL;
		if (link->getDest()!=NULL)
			linkdest=link->getDest()->copy();
		else if (link->getNamedDest()!=NULL)
			linkdest=catalog->findDest(link->getNamedDest());
			
		delete link;
		if (linkdest) { 
	  		int page;
	  		if (linkdest->isPageRef()) {
	    		Ref pageref=linkdest->getPageRef();
	    		page=catalog->findPage(pageref.num,pageref.gen);
	  		} else {
	    		page=linkdest->getPageNum();
	  		}
	  		delete linkdest;

			/* 			complex 	simple
			frames		file-4.html	files.html#4
			noframes	file.html#4	file.html#4
	   		*/
	  		linkName=basename(Docname);
	  		GooString *str=GooString::fromInt(page);
	  		if (noframes) {
	    		linkName->append(".html#");
				linkName->append(str);
	  		} else {
    			if( complexMode ) {
	   		   		linkName->append("-");
	      			linkName->append(str);
	      			linkName->append(".html");
	    		} else {
	      			linkName->append("s.html#");
	      			linkName->append(str);
	    		}
	  		}
			delete str;
		}
      }
      dest.free();

      fputs("<li>",output);
      if (linkName)
		fprintf(output,"<A href=\"%s\">", linkName->getCString());
      fputs(titleStr->getCString(),output);
      if (linkName) {
		fputs("</A>",output);
		delete linkName;
      }
      fputs("\n",output);
      delete titleStr;
      atLeastOne = gTrue;

      newOutlineLevel(output, &curr, catalog, level+1);
      curr.dictLookup("Next", &next);
      curr.free();
      curr = next;
    } while(curr.isDict());
    fputs("</ul>",output);
  }
  curr.free();

  return atLeastOne;
}
