//========================================================================
//
// TextOutputDev.cc
//
// Copyright 1997-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// Copyright (C) 2005-2007 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005 Nickolay V. Shmyrev <nshmyrev@yandex.ru>
// Copyright (C) 2006-2008 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2006, 2007 Ed Catmur <ed@catmur.co.uk>
// Copyright (C) 2006 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2007, 2008 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2008 Koji Otani <sho@bbr.jp>
// Copyright (C) 2008, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008 Pino Toscano <pino@kde.org>
// Copyright (C) 2008, 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2009 Ross Moore <ross@maths.mq.edu.au>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2010 Brian Ewins <brian.ewins@gmail.com>
// Copyright (C) 2010 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#ifdef _WIN32
#include <fcntl.h> // for O_BINARY
#include <io.h>    // for setmode
#endif
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "goo/GooList.h"
#include "poppler-config.h"
#include "Error.h"
#include "GlobalParams.h"
#include "UnicodeMap.h"
#include "UnicodeTypeTable.h"
#include "Link.h"
#include "TextOutputDev.h"
#include "Page.h"
#include "PDFDocEncoding.h"

#ifdef MACOS
// needed for setting type/creator of MacOS files
#include "ICSupport.h"
#endif

//------------------------------------------------------------------------
// parameters
//------------------------------------------------------------------------

// Each bucket in a text pool includes baselines within a range of
// this many points.
#define textPoolStep 4

// Inter-character space width which will cause addChar to start a new
// word.
#define minWordBreakSpace 0.1

// Negative inter-character space width, i.e., overlap, which will
// cause addChar to start a new word.
#define minDupBreakOverlap 0.2

// Max distance between baselines of two lines within a block, as a
// fraction of the font size.
#define maxLineSpacingDelta 1.5

// Max difference in primary font sizes on two lines in the same
// block.  Delta1 is used when examining new lines above and below the
// current block; delta2 is used when examining text that overlaps the
// current block; delta3 is used when examining text to the left and
// right of the current block.
#define maxBlockFontSizeDelta1 0.05
#define maxBlockFontSizeDelta2 0.6
#define maxBlockFontSizeDelta3 0.2

// Max difference in font sizes inside a word.
#define maxWordFontSizeDelta 0.05

// Maximum distance between baselines of two words on the same line,
// e.g., distance between subscript or superscript and the primary
// baseline, as a fraction of the font size.
#define maxIntraLineDelta 0.5

// Minimum inter-word spacing, as a fraction of the font size.  (Only
// used for raw ordering.)
#define minWordSpacing 0.15

// Maximum inter-word spacing, as a fraction of the font size.
#define maxWordSpacing 1.5

// Maximum horizontal spacing which will allow a word to be pulled
// into a block.
#define minColSpacing1 0.3

// Minimum spacing between columns, as a fraction of the font size.
#define minColSpacing2 1.0

// Maximum vertical spacing between blocks within a flow, as a
// multiple of the font size.
#define maxBlockSpacing 2.5

// Minimum spacing between characters within a word, as a fraction of
// the font size.
#define minCharSpacing -0.2

// Maximum spacing between characters within a word, as a fraction of
// the font size, when there is no obvious extra-wide character
// spacing.
#define maxCharSpacing 0.03

// When extra-wide character spacing is detected, the inter-character
// space threshold is set to the minimum inter-character space
// multiplied by this constant.
#define maxWideCharSpacingMul 1.3

// Upper limit on spacing between characters in a word.
#define maxWideCharSpacing 0.4

// Max difference in primary,secondary coordinates (as a fraction of
// the font size) allowed for duplicated text (fake boldface, drop
// shadows) which is to be discarded.
#define dupMaxPriDelta 0.1
#define dupMaxSecDelta 0.2

// Max width of underlines (in points).
#define maxUnderlineWidth 3

// Min distance between baseline and underline (in points).
//~ this should be font-size-dependent
#define minUnderlineGap -2

// Max distance between baseline and underline (in points).
//~ this should be font-size-dependent
#define maxUnderlineGap 4

// Max horizontal distance between edge of word and start of underline
// (in points).
//~ this should be font-size-dependent
#define underlineSlack 1

// Max distance between edge of text and edge of link border
#define hyperlinkSlack 2

//------------------------------------------------------------------------
// TextUnderline
//------------------------------------------------------------------------

class TextUnderline {
public:

  TextUnderline(double x0A, double y0A, double x1A, double y1A)
    { x0 = x0A; y0 = y0A; x1 = x1A; y1 = y1A; horiz = y0 == y1; }
  ~TextUnderline() {}

  double x0, y0, x1, y1;
  GBool horiz;
};

//------------------------------------------------------------------------
// TextLink
//------------------------------------------------------------------------

class TextLink {
public:

  TextLink(int xMinA, int yMinA, int xMaxA, int yMaxA, Link *linkA)
    { xMin = xMinA; yMin = yMinA; xMax = xMaxA; yMax = yMaxA; link = linkA; }
  ~TextLink() {}

  int xMin, yMin, xMax, yMax;
  Link *link;
};

//------------------------------------------------------------------------
// TextFontInfo
//------------------------------------------------------------------------

TextFontInfo::TextFontInfo(GfxState *state) {
  gfxFont = state->getFont();
  if (gfxFont)
    gfxFont->incRefCnt();
#if TEXTOUT_WORD_LIST
  fontName = (gfxFont && gfxFont->getOrigName())
                 ? gfxFont->getOrigName()->copy()
                 : (GooString *)NULL;
  flags = gfxFont ? gfxFont->getFlags() : 0;
#endif
}

TextFontInfo::~TextFontInfo() {
  if (gfxFont)
    gfxFont->decRefCnt();
#if TEXTOUT_WORD_LIST
  if (fontName) {
    delete fontName;
  }
#endif
}

GBool TextFontInfo::matches(GfxState *state) {
  return state->getFont() == gfxFont;
}

//------------------------------------------------------------------------
// TextWord
//------------------------------------------------------------------------

TextWord::TextWord(GfxState *state, int rotA, double x0, double y0,
		   int charPosA, TextFontInfo *fontA, double fontSizeA) {
  GfxFont *gfxFont;
  double x, y, ascent, descent;

  rot = rotA;
  charPos = charPosA;
  charLen = 0;
  font = fontA;
  fontSize = fontSizeA;
  state->transform(x0, y0, &x, &y);
  if ((gfxFont = font->gfxFont)) {
    ascent = gfxFont->getAscent() * fontSize;
    descent = gfxFont->getDescent() * fontSize;
  } else {
    // this means that the PDF file draws text without a current font,
    // which should never happen
    ascent = 0.95 * fontSize;
    descent = -0.35 * fontSize;
  }
  switch (rot) {
  case 0:
    yMin = y - ascent;
    yMax = y - descent;
    if (yMin == yMax) {
      // this is a sanity check for a case that shouldn't happen -- but
      // if it does happen, we want to avoid dividing by zero later
      yMin = y;
      yMax = y + 1;
    }
    base = y;
    break;
  case 1:
    xMin = x + descent;
    xMax = x + ascent;
    if (xMin == xMax) {
      // this is a sanity check for a case that shouldn't happen -- but
      // if it does happen, we want to avoid dividing by zero later
      xMin = x;
      xMax = x + 1;
    }
    base = x;
    break;
  case 2:
    yMin = y + descent;
    yMax = y + ascent;
    if (yMin == yMax) {
      // this is a sanity check for a case that shouldn't happen -- but
      // if it does happen, we want to avoid dividing by zero later
      yMin = y;
      yMax = y + 1;
    }
    base = y;
    break;
  case 3:
    xMin = x - ascent;
    xMax = x - descent;
    if (xMin == xMax) {
      // this is a sanity check for a case that shouldn't happen -- but
      // if it does happen, we want to avoid dividing by zero later
      xMin = x;
      xMax = x + 1;
    }
    base = x;
    break;
  }
  text = NULL;
  charcode = NULL;
  edge = NULL;
  len = size = 0;
  spaceAfter = gFalse;
  next = NULL;

#if TEXTOUT_WORD_LIST
  GfxRGB rgb;

  if ((state->getRender() & 3) == 1) {
    state->getStrokeRGB(&rgb);
  } else {
    state->getFillRGB(&rgb);
  }
  colorR = colToDbl(rgb.r);
  colorG = colToDbl(rgb.g);
  colorB = colToDbl(rgb.b);
#endif

  underlined = gFalse;
  link = NULL;
}

TextWord::~TextWord() {
  gfree(text);
  gfree(charcode);
  gfree(edge);
}

void TextWord::addChar(GfxState *state, double x, double y,
		       double dx, double dy, CharCode c, Unicode u) {
  if (len == size) {
    size += 16;
    text = (Unicode *)greallocn(text, size, sizeof(Unicode));
    charcode = (Unicode *)greallocn(charcode, size, sizeof(CharCode));
    edge = (double *)greallocn(edge, (size + 1), sizeof(double));
  }
  text[len] = u;
  charcode[len] = c;
  switch (rot) {
  case 0:
    if (len == 0) {
      xMin = x;
    }
    edge[len] = x;
    xMax = edge[len+1] = x + dx;
    break;
  case 1:
    if (len == 0) {
      yMin = y;
    }
    edge[len] = y;
    yMax = edge[len+1] = y + dy;
    break;
  case 2:
    if (len == 0) {
      xMax = x;
    }
    edge[len] = x;
    xMin = edge[len+1] = x + dx;
    break;
  case 3:
    if (len == 0) {
      yMax = y;
    }
    edge[len] = y;
    yMin = edge[len+1] = y + dy;
    break;
  }
  ++len;
}

void TextWord::merge(TextWord *word) {
  int i;

  if (word->xMin < xMin) {
    xMin = word->xMin;
  }
  if (word->yMin < yMin) {
    yMin = word->yMin;
  }
  if (word->xMax > xMax) {
    xMax = word->xMax;
  }
  if (word->yMax > yMax) {
    yMax = word->yMax;
  }
  if (len + word->len > size) {
    size = len + word->len;
    text = (Unicode *)greallocn(text, size, sizeof(Unicode));
    charcode = (CharCode *)greallocn(charcode, (size + 1), sizeof(CharCode));
    edge = (double *)greallocn(edge, (size + 1), sizeof(double));
  }
  for (i = 0; i < word->len; ++i) {
    text[len + i] = word->text[i];
    charcode[len + i] = word->charcode[i];
    edge[len + i] = word->edge[i];
  }
  edge[len + word->len] = word->edge[word->len];
  len += word->len;
  charLen += word->charLen;
}

inline int TextWord::primaryCmp(TextWord *word) {
  double cmp;

  cmp = 0; // make gcc happy
  switch (rot) {
  case 0:
    cmp = xMin - word->xMin;
    break;
  case 1:
    cmp = yMin - word->yMin;
    break;
  case 2:
    cmp = word->xMax - xMax;
    break;
  case 3:
    cmp = word->yMax - yMax;
    break;
  }
  return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

double TextWord::primaryDelta(TextWord *word) {
  double delta;

  delta = 0; // make gcc happy
  switch (rot) {
  case 0:
    delta = word->xMin - xMax;
    break;
  case 1:
    delta = word->yMin - yMax;
    break;
  case 2:
    delta = xMin - word->xMax;
    break;
  case 3:
    delta = yMin - word->yMax;
    break;
  }
  return delta;
}

int TextWord::cmpYX(const void *p1, const void *p2) {
  TextWord *word1 = *(TextWord **)p1;
  TextWord *word2 = *(TextWord **)p2;
  double cmp;

  cmp = word1->yMin - word2->yMin;
  if (cmp == 0) {
    cmp = word1->xMin - word2->xMin;
  }
  return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

#if TEXTOUT_WORD_LIST

GooString *TextWord::getText() {
  GooString *s;
  UnicodeMap *uMap;
  char buf[8];
  int n, i;

  s = new GooString();
  if (!(uMap = globalParams->getTextEncoding())) {
    return s;
  }
  for (i = 0; i < len; ++i) {
    n = uMap->mapUnicode(text[i], buf, sizeof(buf));
    s->append(buf, n);
  }
  uMap->decRefCnt();
  return s;
}

void TextWord::getCharBBox(int charIdx, double *xMinA, double *yMinA,
			   double *xMaxA, double *yMaxA) {
  if (charIdx < 0 || charIdx >= len) {
    return;
  }
  switch (rot) {
  case 0:
    *xMinA = edge[charIdx];
    *xMaxA = edge[charIdx + 1];
    *yMinA = yMin;
    *yMaxA = yMax;
    break;
  case 1:
    *xMinA = xMin;
    *xMaxA = xMax;
    *yMinA = edge[charIdx];
    *yMaxA = edge[charIdx + 1];
    break;
  case 2:
    *xMinA = edge[charIdx + 1];
    *xMaxA = edge[charIdx];
    *yMinA = yMin;
    *yMaxA = yMax;
    break;
  case 3:
    *xMinA = xMin;
    *xMaxA = xMax;
    *yMinA = edge[charIdx + 1];
    *yMaxA = edge[charIdx];
    break;
  }
}

#endif // TEXTOUT_WORD_LIST

//------------------------------------------------------------------------
// TextPool
//------------------------------------------------------------------------

TextPool::TextPool() {
  minBaseIdx = 0;
  maxBaseIdx = -1;
  pool = NULL;
  cursor = NULL;
  cursorBaseIdx = -1;
}

TextPool::~TextPool() {
  int baseIdx;
  TextWord *word, *word2;

  for (baseIdx = minBaseIdx; baseIdx <= maxBaseIdx; ++baseIdx) {
    for (word = pool[baseIdx - minBaseIdx]; word; word = word2) {
      word2 = word->next;
      delete word;
    }
  }
  gfree(pool);
}

int TextPool::getBaseIdx(double base) {
  int baseIdx;

  baseIdx = (int)(base / textPoolStep);
  if (baseIdx < minBaseIdx) {
    return minBaseIdx;
  }
  if (baseIdx > maxBaseIdx) {
    return maxBaseIdx;
  }
  return baseIdx;
}

void TextPool::addWord(TextWord *word) {
  TextWord **newPool;
  int wordBaseIdx, newMinBaseIdx, newMaxBaseIdx, baseIdx;
  TextWord *w0, *w1;

  // expand the array if needed
  wordBaseIdx = (int)(word->base / textPoolStep);
  if (minBaseIdx > maxBaseIdx) {
    minBaseIdx = wordBaseIdx - 128;
    maxBaseIdx = wordBaseIdx + 128;
    pool = (TextWord **)gmallocn(maxBaseIdx - minBaseIdx + 1,
				 sizeof(TextWord *));
    for (baseIdx = minBaseIdx; baseIdx <= maxBaseIdx; ++baseIdx) {
      pool[baseIdx - minBaseIdx] = NULL;
    }
  } else if (wordBaseIdx < minBaseIdx) {
    newMinBaseIdx = wordBaseIdx - 128;
    newPool = (TextWord **)gmallocn(maxBaseIdx - newMinBaseIdx + 1,
				    sizeof(TextWord *));
    for (baseIdx = newMinBaseIdx; baseIdx < minBaseIdx; ++baseIdx) {
      newPool[baseIdx - newMinBaseIdx] = NULL;
    }
    memcpy(&newPool[minBaseIdx - newMinBaseIdx], pool,
	   (maxBaseIdx - minBaseIdx + 1) * sizeof(TextWord *));
    gfree(pool);
    pool = newPool;
    minBaseIdx = newMinBaseIdx;
  } else if (wordBaseIdx > maxBaseIdx) {
    newMaxBaseIdx = wordBaseIdx + 128;
    pool = (TextWord **)greallocn(pool, newMaxBaseIdx - minBaseIdx + 1,
				  sizeof(TextWord *));
    for (baseIdx = maxBaseIdx + 1; baseIdx <= newMaxBaseIdx; ++baseIdx) {
      pool[baseIdx - minBaseIdx] = NULL;
    }
    maxBaseIdx = newMaxBaseIdx;
  }

  // insert the new word
  if (cursor && wordBaseIdx == cursorBaseIdx &&
      word->primaryCmp(cursor) > 0) {
    w0 = cursor;
    w1 = cursor->next;
  } else {
    w0 = NULL;
    w1 = pool[wordBaseIdx - minBaseIdx];
  }
  for (; w1 && word->primaryCmp(w1) > 0; w0 = w1, w1 = w1->next) ;
  word->next = w1;
  if (w0) {
    w0->next = word;
  } else {
    pool[wordBaseIdx - minBaseIdx] = word;
  }
  cursor = word;
  cursorBaseIdx = wordBaseIdx;
}

//------------------------------------------------------------------------
// TextLine
//------------------------------------------------------------------------

TextLine::TextLine(TextBlock *blkA, int rotA, double baseA) {
  blk = blkA;
  rot = rotA;
  base = baseA;
  words = lastWord = NULL;
  text = NULL;
  edge = NULL;
  col = NULL;
  len = 0;
  convertedLen = 0;
  hyphenated = gFalse;
  next = NULL;
  xMin = yMin = 0;
  xMax = yMax = -1;
  normalized = NULL;
  normalized_len = 0;
  normalized_idx = NULL;
}

TextLine::~TextLine() {
  TextWord *word;

  while (words) {
    word = words;
    words = words->next;
    delete word;
  }
  gfree(text);
  gfree(edge);
  gfree(col);
  if (normalized) {
    gfree(normalized);
    gfree(normalized_idx);
  }
}

void TextLine::addWord(TextWord *word) {
  if (lastWord) {
    lastWord->next = word;
  } else {
    words = word;
  }
  lastWord = word;

  if (xMin > xMax) {
    xMin = word->xMin;
    xMax = word->xMax;
    yMin = word->yMin;
    yMax = word->yMax;
  } else {
    if (word->xMin < xMin) {
      xMin = word->xMin;
    }
    if (word->xMax > xMax) {
      xMax = word->xMax;
    }
    if (word->yMin < yMin) {
      yMin = word->yMin;
    }
    if (word->yMax > yMax) {
      yMax = word->yMax;
    }
  }
}

double TextLine::primaryDelta(TextLine *line) {
  double delta;

  delta = 0; // make gcc happy
  switch (rot) {
  case 0:
    delta = line->xMin - xMax;
    break;
  case 1:
    delta = line->yMin - yMax;
    break;
  case 2:
    delta = xMin - line->xMax;
    break;
  case 3:
    delta = yMin - line->yMax;
    break;
  }
  return delta;
}

int TextLine::primaryCmp(TextLine *line) {
  double cmp;

  cmp = 0; // make gcc happy
  switch (rot) {
  case 0:
    cmp = xMin - line->xMin;
    break;
  case 1:
    cmp = yMin - line->yMin;
    break;
  case 2:
    cmp = line->xMax - xMax;
    break;
  case 3:
    cmp = line->yMax - yMax;
    break;
  }
  return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int TextLine::secondaryCmp(TextLine *line) {
  double cmp;

  cmp = (rot == 0 || rot == 3) ? base - line->base : line->base - base;
  return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int TextLine::cmpYX(TextLine *line) {
  int cmp;

  if ((cmp = secondaryCmp(line))) {
    return cmp;
  }
  return primaryCmp(line);
}

int TextLine::cmpXY(const void *p1, const void *p2) {
  TextLine *line1 = *(TextLine **)p1;
  TextLine *line2 = *(TextLine **)p2;
  int cmp;

  if ((cmp = line1->primaryCmp(line2))) {
    return cmp;
  }
  return line1->secondaryCmp(line2);
}

void TextLine::coalesce(UnicodeMap *uMap) {
  TextWord *word0, *word1;
  double space, delta, minSpace;
  GBool isUnicode;
  char buf[8];
  int i, j;

  if (words->next) {

    // compute the inter-word space threshold
    if (words->len > 1 || words->next->len > 1) {
      minSpace = 0;
    } else {
      minSpace = words->primaryDelta(words->next);
      for (word0 = words->next, word1 = word0->next;
	   word1 && minSpace > 0;
	   word0 = word1, word1 = word0->next) {
	if (word1->len > 1) {
	  minSpace = 0;
	}
	delta = word0->primaryDelta(word1);
	if (delta < minSpace) {
	  minSpace = delta;
	}
      }
    }
    if (minSpace <= 0) {
      space = maxCharSpacing * words->fontSize;
    } else {
      space = maxWideCharSpacingMul * minSpace;
      if (space > maxWideCharSpacing * words->fontSize) {
	space = maxWideCharSpacing * words->fontSize;
      }
    }

    // merge words
    word0 = words;
    word1 = words->next;
    while (word1) {
      if (word0->primaryDelta(word1) >= space) {
	word0->spaceAfter = gTrue;
	word0 = word1;
	word1 = word1->next;
      } else if (word0->font == word1->font &&
		 word0->underlined == word1->underlined &&
		 fabs(word0->fontSize - word1->fontSize) <
		   maxWordFontSizeDelta * words->fontSize &&
		 word1->charPos == word0->charPos + word0->charLen) {
	word0->merge(word1);
	word0->next = word1->next;
	delete word1;
	word1 = word0->next;
      } else {
	word0 = word1;
	word1 = word1->next;
      }
    }
  }

  // build the line text
  isUnicode = uMap ? uMap->isUnicode() : gFalse;
  len = 0;
  for (word1 = words; word1; word1 = word1->next) {
    len += word1->len;
    if (word1->spaceAfter) {
      ++len;
    }
  }
  text = (Unicode *)gmallocn(len, sizeof(Unicode));
  edge = (double *)gmallocn(len + 1, sizeof(double));
  i = 0;
  for (word1 = words; word1; word1 = word1->next) {
    for (j = 0; j < word1->len; ++j) {
      text[i] = word1->text[j];
      edge[i] = word1->edge[j];
      ++i;
    }
    edge[i] = word1->edge[word1->len];
    if (word1->spaceAfter) {
      text[i] = (Unicode)0x0020;
      ++i;
    }
  }

  // compute convertedLen and set up the col array
  col = (int *)gmallocn(len + 1, sizeof(int));
  convertedLen = 0;
  for (i = 0; i < len; ++i) {
    col[i] = convertedLen;
    if (isUnicode) {
      ++convertedLen;
    } else if (uMap) {
      convertedLen += uMap->mapUnicode(text[i], buf, sizeof(buf));
    }
  }
  col[len] = convertedLen;

  // check for hyphen at end of line
  //~ need to check for other chars used as hyphens
  hyphenated = text[len - 1] == (Unicode)'-';
}

//------------------------------------------------------------------------
// TextLineFrag
//------------------------------------------------------------------------

class TextLineFrag {
public:

  TextLine *line;		// the line object
  int start, len;		// offset and length of this fragment
				//   (in Unicode chars)
  double xMin, xMax;		// bounding box coordinates
  double yMin, yMax;
  double base;			// baseline virtual coordinate
  int col;			// first column

  void init(TextLine *lineA, int startA, int lenA);
  void computeCoords(GBool oneRot);

  static int cmpYXPrimaryRot(const void *p1, const void *p2);
  static int cmpYXLineRot(const void *p1, const void *p2);
  static int cmpXYLineRot(const void *p1, const void *p2);
  static int cmpXYColumnPrimaryRot(const void *p1, const void *p2);
  static int cmpXYColumnLineRot(const void *p1, const void *p2);
};

void TextLineFrag::init(TextLine *lineA, int startA, int lenA) {
  line = lineA;
  start = startA;
  len = lenA;
  col = line->col[start];
}

void TextLineFrag::computeCoords(GBool oneRot) {
  TextBlock *blk;
  double d0, d1, d2, d3, d4;

  if (oneRot) {

    switch (line->rot) {
    case 0:
      xMin = line->edge[start];
      xMax = line->edge[start + len];
      yMin = line->yMin;
      yMax = line->yMax;
      break;
    case 1:
      xMin = line->xMin;
      xMax = line->xMax;
      yMin = line->edge[start];
      yMax = line->edge[start + len];
      break;
    case 2:
      xMin = line->edge[start + len];
      xMax = line->edge[start];
      yMin = line->yMin;
      yMax = line->yMax;
      break;
    case 3:
      xMin = line->xMin;
      xMax = line->xMax;
      yMin = line->edge[start + len];
      yMax = line->edge[start];
      break;
    }
    base = line->base;

  } else {

    if (line->rot == 0 && line->blk->page->primaryRot == 0) {

      xMin = line->edge[start];
      xMax = line->edge[start + len];
      yMin = line->yMin;
      yMax = line->yMax;
      base = line->base;

    } else {

      blk = line->blk;
      d0 = line->edge[start];
      d1 = line->edge[start + len];
      d2 = d3 = d4 = 0; // make gcc happy

      switch (line->rot) {
      case 0:
	d2 = line->yMin;
	d3 = line->yMax;
	d4 = line->base;
	d0 = (d0 - blk->xMin) / (blk->xMax - blk->xMin);
	d1 = (d1 - blk->xMin) / (blk->xMax - blk->xMin);
	d2 = (d2 - blk->yMin) / (blk->yMax - blk->yMin);
	d3 = (d3 - blk->yMin) / (blk->yMax - blk->yMin);
	d4 = (d4 - blk->yMin) / (blk->yMax - blk->yMin);
	break;
      case 1:
	d2 = line->xMax;
	d3 = line->xMin;
	d4 = line->base;
	d0 = (d0 - blk->yMin) / (blk->yMax - blk->yMin);
	d1 = (d1 - blk->yMin) / (blk->yMax - blk->yMin);
	d2 = (blk->xMax - d2) / (blk->xMax - blk->xMin);
	d3 = (blk->xMax - d3) / (blk->xMax - blk->xMin);
	d4 = (blk->xMax - d4) / (blk->xMax - blk->xMin);
	break;
      case 2:
	d2 = line->yMax;
	d3 = line->yMin;
	d4 = line->base;
	d0 = (blk->xMax - d0) / (blk->xMax - blk->xMin);
	d1 = (blk->xMax - d1) / (blk->xMax - blk->xMin);
	d2 = (blk->yMax - d2) / (blk->yMax - blk->yMin);
	d3 = (blk->yMax - d3) / (blk->yMax - blk->yMin);
	d4 = (blk->yMax - d4) / (blk->yMax - blk->yMin);
	break;
      case 3:
	d2 = line->xMin;
	d3 = line->xMax;
	d4 = line->base;
	d0 = (blk->yMax - d0) / (blk->yMax - blk->yMin);
	d1 = (blk->yMax - d1) / (blk->yMax - blk->yMin);
	d2 = (d2 - blk->xMin) / (blk->xMax - blk->xMin);
	d3 = (d3 - blk->xMin) / (blk->xMax - blk->xMin);
	d4 = (d4 - blk->xMin) / (blk->xMax - blk->xMin);
	break;
      }

      switch (line->blk->page->primaryRot) {
      case 0:
	xMin = blk->xMin + d0 * (blk->xMax - blk->xMin);
	xMax = blk->xMin + d1 * (blk->xMax - blk->xMin);
	yMin = blk->yMin + d2 * (blk->yMax - blk->yMin);
	yMax = blk->yMin + d3 * (blk->yMax - blk->yMin);
	base = blk->yMin + base * (blk->yMax - blk->yMin);
	break;
      case 1:
	xMin = blk->xMax - d3 * (blk->xMax - blk->xMin);
	xMax = blk->xMax - d2 * (blk->xMax - blk->xMin);
	yMin = blk->yMin + d0 * (blk->yMax - blk->yMin);
	yMax = blk->yMin + d1 * (blk->yMax - blk->yMin);
	base = blk->xMax - d4 * (blk->xMax - blk->xMin);
	break;
      case 2:
	xMin = blk->xMax - d1 * (blk->xMax - blk->xMin);
	xMax = blk->xMax - d0 * (blk->xMax - blk->xMin);
	yMin = blk->yMax - d3 * (blk->yMax - blk->yMin);
	yMax = blk->yMax - d2 * (blk->yMax - blk->yMin);
	base = blk->yMax - d4 * (blk->yMax - blk->yMin);
	break;
      case 3:
	xMin = blk->xMin + d2 * (blk->xMax - blk->xMin);
	xMax = blk->xMin + d3 * (blk->xMax - blk->xMin);
	yMin = blk->yMax - d1 * (blk->yMax - blk->yMin);
	yMax = blk->yMax - d0 * (blk->yMax - blk->yMin);
	base = blk->xMin + d4 * (blk->xMax - blk->xMin);
	break;
      }

    }
  }
}

int TextLineFrag::cmpYXPrimaryRot(const void *p1, const void *p2) {
  TextLineFrag *frag1 = (TextLineFrag *)p1;
  TextLineFrag *frag2 = (TextLineFrag *)p2;
  double cmp;

  cmp = 0; // make gcc happy
  switch (frag1->line->blk->page->primaryRot) {
  case 0:
    if (fabs(cmp = frag1->yMin - frag2->yMin) < 0.01) {
      cmp = frag1->xMin - frag2->xMin;
    }
    break;
  case 1:
    if (fabs(cmp = frag2->xMax - frag1->xMax) < 0.01) {
      cmp = frag1->yMin - frag2->yMin;
    }
    break;
  case 2:
    if (fabs(cmp = frag2->yMin - frag1->yMin) < 0.01) {
      cmp = frag2->xMax - frag1->xMax;
    }
    break;
  case 3:
    if (fabs(cmp = frag1->xMax - frag2->xMax) < 0.01) {
      cmp = frag2->yMax - frag1->yMax;
    }
    break;
  }
  return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int TextLineFrag::cmpYXLineRot(const void *p1, const void *p2) {
  TextLineFrag *frag1 = (TextLineFrag *)p1;
  TextLineFrag *frag2 = (TextLineFrag *)p2;
  double cmp;

  cmp = 0; // make gcc happy
  switch (frag1->line->rot) {
  case 0:
    if ((cmp = frag1->yMin - frag2->yMin) == 0) {
      cmp = frag1->xMin - frag2->xMin;
    }
    break;
  case 1:
    if ((cmp = frag2->xMax - frag1->xMax) == 0) {
      cmp = frag1->yMin - frag2->yMin;
    }
    break;
  case 2:
    if ((cmp = frag2->yMin - frag1->yMin) == 0) {
      cmp = frag2->xMax - frag1->xMax;
    }
    break;
  case 3:
    if ((cmp = frag1->xMax - frag2->xMax) == 0) {
      cmp = frag2->yMax - frag1->yMax;
    }
    break;
  }
  return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int TextLineFrag::cmpXYLineRot(const void *p1, const void *p2) {
  TextLineFrag *frag1 = (TextLineFrag *)p1;
  TextLineFrag *frag2 = (TextLineFrag *)p2;
  double cmp;

  cmp = 0; // make gcc happy
  switch (frag1->line->rot) {
  case 0:
    if ((cmp = frag1->xMin - frag2->xMin) == 0) {
      cmp = frag1->yMin - frag2->yMin;
    }
    break;
  case 1:
    if ((cmp = frag1->yMin - frag2->yMin) == 0) {
      cmp = frag2->xMax - frag1->xMax;
    }
    break;
  case 2:
    if ((cmp = frag2->xMax - frag1->xMax) == 0) {
      cmp = frag2->yMin - frag1->yMin;
    }
    break;
  case 3:
    if ((cmp = frag2->yMax - frag1->yMax) == 0) {
      cmp = frag1->xMax - frag2->xMax;
    }
    break;
  }
  return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int TextLineFrag::cmpXYColumnPrimaryRot(const void *p1, const void *p2) {
  TextLineFrag *frag1 = (TextLineFrag *)p1;
  TextLineFrag *frag2 = (TextLineFrag *)p2;
  double cmp;

  // if columns overlap, compare y values
  if (frag1->col < frag2->col + (frag2->line->col[frag2->start + frag2->len] -
				 frag2->line->col[frag2->start]) &&
      frag2->col < frag1->col + (frag1->line->col[frag1->start + frag1->len] -
				 frag1->line->col[frag1->start])) {
    cmp = 0; // make gcc happy
    switch (frag1->line->blk->page->primaryRot) {
    case 0: cmp = frag1->yMin - frag2->yMin; break;
    case 1: cmp = frag2->xMax - frag1->xMax; break;
    case 2: cmp = frag2->yMin - frag1->yMin; break;
    case 3: cmp = frag1->xMax - frag2->xMax; break;
    }
    return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
  }

  // otherwise, compare starting column
  return frag1->col - frag2->col;
}

int TextLineFrag::cmpXYColumnLineRot(const void *p1, const void *p2) {
  TextLineFrag *frag1 = (TextLineFrag *)p1;
  TextLineFrag *frag2 = (TextLineFrag *)p2;
  double cmp;

  // if columns overlap, compare y values
  if (frag1->col < frag2->col + (frag2->line->col[frag2->start + frag2->len] -
				 frag2->line->col[frag2->start]) &&
      frag2->col < frag1->col + (frag1->line->col[frag1->start + frag1->len] -
				 frag1->line->col[frag1->start])) {
    cmp = 0; // make gcc happy
    switch (frag1->line->rot) {
    case 0: cmp = frag1->yMin - frag2->yMin; break;
    case 1: cmp = frag2->xMax - frag1->xMax; break;
    case 2: cmp = frag2->yMin - frag1->yMin; break;
    case 3: cmp = frag1->xMax - frag2->xMax; break;
    }
    return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
  }

  // otherwise, compare starting column
  return frag1->col - frag2->col;
}

//------------------------------------------------------------------------
// TextBlock
//------------------------------------------------------------------------

TextBlock::TextBlock(TextPage *pageA, int rotA) {
  page = pageA;
  rot = rotA;
  xMin = yMin = 0;
  xMax = yMax = -1;
  priMin = 0;
  priMax = page->pageWidth;
  pool = new TextPool();
  lines = NULL;
  curLine = NULL;
  next = NULL;
  stackNext = NULL;
  tableId = -1;
  tableEnd = gFalse;
}

TextBlock::~TextBlock() {
  TextLine *line;

  delete pool;
  while (lines) {
    line = lines;
    lines = lines->next;
    delete line;
  }
}

void TextBlock::addWord(TextWord *word) {
  pool->addWord(word);
  if (xMin > xMax) {
    xMin = word->xMin;
    xMax = word->xMax;
    yMin = word->yMin;
    yMax = word->yMax;
  } else {
    if (word->xMin < xMin) {
      xMin = word->xMin;
    }
    if (word->xMax > xMax) {
      xMax = word->xMax;
    }
    if (word->yMin < yMin) {
      yMin = word->yMin;
    }
    if (word->yMax > yMax) {
      yMax = word->yMax;
    }
  }
}

void TextBlock::coalesce(UnicodeMap *uMap) {
  TextWord *word0, *word1, *word2, *bestWord0, *bestWord1, *lastWord;
  TextLine *line, *line0, *line1;
  int poolMinBaseIdx, startBaseIdx, minBaseIdx, maxBaseIdx;
  int baseIdx, bestWordBaseIdx, idx0, idx1;
  double minBase, maxBase;
  double fontSize, delta, priDelta, secDelta;
  TextLine **lineArray;
  GBool found;
  int col1, col2;
  int i, j, k;

  // discard duplicated text (fake boldface, drop shadows)
  for (idx0 = pool->minBaseIdx; idx0 <= pool->maxBaseIdx; ++idx0) {
    word0 = pool->getPool(idx0);
    while (word0) {
      priDelta = dupMaxPriDelta * word0->fontSize;
      secDelta = dupMaxSecDelta * word0->fontSize;
      if (rot == 0 || rot == 3) {
	maxBaseIdx = pool->getBaseIdx(word0->base + secDelta);
      } else {
	maxBaseIdx = pool->getBaseIdx(word0->base - secDelta);
      }
      found = gFalse;
      word1 = word2 = NULL; // make gcc happy
      for (idx1 = idx0; idx1 <= maxBaseIdx; ++idx1) {
	if (idx1 == idx0) {
	  word1 = word0;
	  word2 = word0->next;
	} else {
	  word1 = NULL;
	  word2 = pool->getPool(idx1);
	}
	for (; word2; word1 = word2, word2 = word2->next) {
	  if (word2->len == word0->len &&
	      !memcmp(word2->text, word0->text,
		      word0->len * sizeof(Unicode))) {
	    switch (rot) {
	    case 0:
	    case 2:
	      found = fabs(word0->xMin - word2->xMin) < priDelta &&
		      fabs(word0->xMax - word2->xMax) < priDelta &&
		      fabs(word0->yMin - word2->yMin) < secDelta &&
		      fabs(word0->yMax - word2->yMax) < secDelta;
	      break;
	    case 1:
	    case 3:
	      found = fabs(word0->xMin - word2->xMin) < secDelta &&
		      fabs(word0->xMax - word2->xMax) < secDelta &&
		      fabs(word0->yMin - word2->yMin) < priDelta &&
		      fabs(word0->yMax - word2->yMax) < priDelta;
	      break;
	    }
	  }
	  if (found) {
	    break;
	  }
	}
	if (found) {
	  break;
	}
      }
      if (found) {
	if (word1) {
	  word1->next = word2->next;
	} else {
	  pool->setPool(idx1, word2->next);
	}
	delete word2;
      } else {
	word0 = word0->next;
      }
    }
  }

  // build the lines
  curLine = NULL;
  poolMinBaseIdx = pool->minBaseIdx;
  charCount = 0;
  nLines = 0;
  while (1) {

    // find the first non-empty line in the pool
    for (;
	 poolMinBaseIdx <= pool->maxBaseIdx && !pool->getPool(poolMinBaseIdx);
	 ++poolMinBaseIdx) ;
    if (poolMinBaseIdx > pool->maxBaseIdx) {
      break;
    }

    // look for the left-most word in the first four lines of the
    // pool -- this avoids starting with a superscript word
    startBaseIdx = poolMinBaseIdx;
    for (baseIdx = poolMinBaseIdx + 1;
	 baseIdx < poolMinBaseIdx + 4 && baseIdx <= pool->maxBaseIdx;
	 ++baseIdx) {
      if (!pool->getPool(baseIdx)) {
	continue;
      }
      if (pool->getPool(baseIdx)->primaryCmp(pool->getPool(startBaseIdx))
	  < 0) {
	startBaseIdx = baseIdx;
      }
    }

    // create a new line
    word0 = pool->getPool(startBaseIdx);
    pool->setPool(startBaseIdx, word0->next);
    word0->next = NULL;
    line = new TextLine(this, word0->rot, word0->base);
    line->addWord(word0);
    lastWord = word0;

    // compute the search range
    fontSize = word0->fontSize;
    minBase = word0->base - maxIntraLineDelta * fontSize;
    maxBase = word0->base + maxIntraLineDelta * fontSize;
    minBaseIdx = pool->getBaseIdx(minBase);
    maxBaseIdx = pool->getBaseIdx(maxBase);

    // find the rest of the words in this line
    while (1) {

      // find the left-most word whose baseline is in the range for
      // this line
      bestWordBaseIdx = 0;
      bestWord0 = bestWord1 = NULL;
      for (baseIdx = minBaseIdx; baseIdx <= maxBaseIdx; ++baseIdx) {
	for (word0 = NULL, word1 = pool->getPool(baseIdx);
	     word1;
	     word0 = word1, word1 = word1->next) {
	  if (word1->base >= minBase &&
	      word1->base <= maxBase &&
	      (delta = lastWord->primaryDelta(word1)) >=
	        minCharSpacing * fontSize) {
	    if (delta < maxWordSpacing * fontSize &&
		(!bestWord1 || word1->primaryCmp(bestWord1) < 0)) {
	      bestWordBaseIdx = baseIdx;
	      bestWord0 = word0;
	      bestWord1 = word1;
	    }
	    break;
	  }
	}
      }
      if (!bestWord1) {
	break;
      }

      // remove it from the pool, and add it to the line
      if (bestWord0) {
	bestWord0->next = bestWord1->next;
      } else {
	pool->setPool(bestWordBaseIdx, bestWord1->next);
      }
      bestWord1->next = NULL;
      line->addWord(bestWord1);
      lastWord = bestWord1;
    }

    // add the line
    if (curLine && line->cmpYX(curLine) > 0) {
      line0 = curLine;
      line1 = curLine->next;
    } else {
      line0 = NULL;
      line1 = lines;
    }
    for (;
	 line1 && line->cmpYX(line1) > 0;
	 line0 = line1, line1 = line1->next) ;
    if (line0) {
      line0->next = line;
    } else {
      lines = line;
    }
    line->next = line1;
    curLine = line;
    line->coalesce(uMap);
    charCount += line->len;
    ++nLines;
  }

  // sort lines into xy order for column assignment
  lineArray = (TextLine **)gmallocn(nLines, sizeof(TextLine *));
  for (line = lines, i = 0; line; line = line->next, ++i) {
    lineArray[i] = line;
  }
  qsort(lineArray, nLines, sizeof(TextLine *), &TextLine::cmpXY);

  // column assignment
  nColumns = 0;
  for (i = 0; i < nLines; ++i) {
    line0 = lineArray[i];
    col1 = 0;
    for (j = 0; j < i; ++j) {
      line1 = lineArray[j];
      if (line1->primaryDelta(line0) >= 0) {
	col2 = line1->col[line1->len] + 1;
      } else {
	k = 0; // make gcc happy
	switch (rot) {
	case 0:
	  for (k = 0;
	       k < line1->len &&
		 line0->xMin >= 0.5 * (line1->edge[k] + line1->edge[k+1]);
	       ++k) ;
	  break;
	case 1:
	  for (k = 0;
	       k < line1->len &&
		 line0->yMin >= 0.5 * (line1->edge[k] + line1->edge[k+1]);
	       ++k) ;
	  break;
	case 2:
	  for (k = 0;
	       k < line1->len &&
		 line0->xMax <= 0.5 * (line1->edge[k] + line1->edge[k+1]);
	       ++k) ;
	  break;
	case 3:
	  for (k = 0;
	       k < line1->len &&
		 line0->yMax <= 0.5 * (line1->edge[k] + line1->edge[k+1]);
	       ++k) ;
	  break;
	}
	col2 = line1->col[k];
      }
      if (col2 > col1) {
	col1 = col2;
      }
    }
    for (k = 0; k <= line0->len; ++k) {
      line0->col[k] += col1;
    }
    if (line0->col[line0->len] > nColumns) {
      nColumns = line0->col[line0->len];
    }
  }
  gfree(lineArray);
}

void TextBlock::updatePriMinMax(TextBlock *blk) {
  double newPriMin, newPriMax;
  GBool gotPriMin, gotPriMax;

  gotPriMin = gotPriMax = gFalse;
  newPriMin = newPriMax = 0; // make gcc happy
  switch (page->primaryRot) {
  case 0:
  case 2:
    if (blk->yMin < yMax && blk->yMax > yMin) {
      if (blk->xMin < xMin) {
	newPriMin = blk->xMax;
	gotPriMin = gTrue;
      }
      if (blk->xMax > xMax) {
	newPriMax = blk->xMin;
	gotPriMax = gTrue;
      }
    }
    break;
  case 1:
  case 3:
    if (blk->xMin < xMax && blk->xMax > xMin) {
      if (blk->yMin < yMin) {
	newPriMin = blk->yMax;
	gotPriMin = gTrue;
      }
      if (blk->yMax > yMax) {
	newPriMax = blk->yMin;
	gotPriMax = gTrue;
      }
    }
    break;
  }
  if (gotPriMin) {
    if (newPriMin > xMin) {
      newPriMin = xMin;
    }
    if (newPriMin > priMin) {
      priMin = newPriMin;
    }
  }
  if (gotPriMax) {
    if (newPriMax < xMax) {
      newPriMax = xMax;
    }
    if (newPriMax < priMax) {
      priMax = newPriMax;
    }
  }
}

int TextBlock::cmpXYPrimaryRot(const void *p1, const void *p2) {
  TextBlock *blk1 = *(TextBlock **)p1;
  TextBlock *blk2 = *(TextBlock **)p2;
  double cmp;

  cmp = 0; // make gcc happy
  switch (blk1->page->primaryRot) {
  case 0:
    if ((cmp = blk1->xMin - blk2->xMin) == 0) {
      cmp = blk1->yMin - blk2->yMin;
    }
    break;
  case 1:
    if ((cmp = blk1->yMin - blk2->yMin) == 0) {
      cmp = blk2->xMax - blk1->xMax;
    }
    break;
  case 2:
    if ((cmp = blk2->xMax - blk1->xMax) == 0) {
      cmp = blk2->yMin - blk1->yMin;
    }
    break;
  case 3:
    if ((cmp = blk2->yMax - blk1->yMax) == 0) {
      cmp = blk1->xMax - blk2->xMax;
    }
    break;
  }
  return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int TextBlock::cmpYXPrimaryRot(const void *p1, const void *p2) {
  TextBlock *blk1 = *(TextBlock **)p1;
  TextBlock *blk2 = *(TextBlock **)p2;
  double cmp;

  cmp = 0; // make gcc happy
  switch (blk1->page->primaryRot) {
  case 0:
    if ((cmp = blk1->yMin - blk2->yMin) == 0) {
      cmp = blk1->xMin - blk2->xMin;
    }
    break;
  case 1:
    if ((cmp = blk2->xMax - blk1->xMax) == 0) {
      cmp = blk1->yMin - blk2->yMin;
    }
    break;
  case 2:
    if ((cmp = blk2->yMin - blk1->yMin) == 0) {
      cmp = blk2->xMax - blk1->xMax;
    }
    break;
  case 3:
    if ((cmp = blk1->xMax - blk2->xMax) == 0) {
      cmp = blk2->yMax - blk1->yMax;
    }
    break;
  }
  return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

int TextBlock::primaryCmp(TextBlock *blk) {
  double cmp;

  cmp = 0; // make gcc happy
  switch (rot) {
  case 0:
    cmp = xMin - blk->xMin;
    break;
  case 1:
    cmp = yMin - blk->yMin;
    break;
  case 2:
    cmp = blk->xMax - xMax;
    break;
  case 3:
    cmp = blk->yMax - yMax;
    break;
  }
  return cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
}

double TextBlock::secondaryDelta(TextBlock *blk) {
  double delta;

  delta = 0; // make gcc happy
  switch (rot) {
  case 0:
    delta = blk->yMin - yMax;
    break;
  case 1:
    delta = xMin - blk->xMax;
    break;
  case 2:
    delta = yMin - blk->yMax;
    break;
  case 3:
    delta = blk->xMin - xMax;
    break;
  }
  return delta;
}

GBool TextBlock::isBelow(TextBlock *blk) {
  GBool below;

  below = gFalse; // make gcc happy
  switch (page->primaryRot) {
  case 0:
    below = xMin >= blk->priMin && xMax <= blk->priMax &&
            yMin > blk->yMin;
    break;
  case 1:
    below = yMin >= blk->priMin && yMax <= blk->priMax &&
            xMax < blk->xMax;
    break;
  case 2:
    below = xMin >= blk->priMin && xMax <= blk->priMax &&
            yMax < blk->yMax;
    break;
  case 3:
    below = yMin >= blk->priMin && yMax <= blk->priMax &&
            xMin > blk->xMin;
    break;
  }

  return below;
}

GBool TextBlock::isBeforeByRule1(TextBlock *blk1) {
  GBool before = gFalse;
  GBool overlap = gFalse;

  switch (this->page->primaryRot) {
  case 0:
  case 2:
    overlap = ((this->ExMin <= blk1->ExMin) &&
	       (blk1->ExMin <= this->ExMax)) ||
      ((blk1->ExMin <= this->ExMin) &&
       (this->ExMin <= blk1->ExMax));
    break;
  case 1:
  case 3:
    overlap = ((this->EyMin <= blk1->EyMin) &&
	       (blk1->EyMin <= this->EyMax)) ||
      ((blk1->EyMin <= this->EyMin) &&
       (this->EyMin <= blk1->EyMax));
    break;
  }
  switch (this->page->primaryRot) {
  case 0:
    before = overlap && this->EyMin < blk1->EyMin;
    break;
  case 1:
    before = overlap && this->ExMax > blk1->ExMax;
    break;
  case 2:
    before = overlap && this->EyMax > blk1->EyMax;
    break;
  case 3:
    before = overlap && this->ExMin < blk1->ExMin;
    break;
  }
  return before;
}

GBool TextBlock::isBeforeByRule2(TextBlock *blk1) {
  double cmp = 0;
  int rotLR = rot;

  if (!page->primaryLR) {
    rotLR = (rotLR + 2) % 4;
  }

  switch (rotLR) {
  case 0:
    cmp = ExMax - blk1->ExMin;
    break;
  case 1:
    cmp = EyMin - blk1->EyMax;
    break;
  case 2:
    cmp = blk1->ExMax - ExMin;
    break;
  case 3:
    cmp = blk1->EyMin - EyMax;
    break;
  }
  return cmp <= 0;
}

// Sort into reading order by performing a topological sort using the rules
// given in "High Performance Document Layout Analysis", T.M. Breuel, 2003.
// See http://pubs.iupr.org/#2003-breuel-sdiut
// Topological sort is done by depth first search, see
// http://en.wikipedia.org/wiki/Topological_sorting
int TextBlock::visitDepthFirst(TextBlock *blkList, int pos1,
			       TextBlock **sorted, int sortPos,
			       GBool* visited) {
  int pos2;
  TextBlock *blk1, *blk2, *blk3;
  GBool before;

  if (visited[pos1]) {
    return sortPos;
  }

  blk1 = this;

#if 0 // for debugging
  printf("visited: %d %.2f..%.2f %.2f..%.2f\n",
	 sortPos, blk1->ExMin, blk1->ExMax, blk1->EyMin, blk1->EyMax);
#endif
  visited[pos1] = gTrue;
  pos2 = -1;
  for (blk2 = blkList; blk2; blk2 = blk2->next) {
    pos2++;
    if (visited[pos2]) {
      // skip visited nodes
      continue;
    }
    before = gFalse;

    // is blk2 before blk1? (for table entries)
    if (blk1->tableId >= 0 && blk1->tableId == blk2->tableId) {
      if (page->primaryLR) {
        if (blk2->xMax <= blk1->xMin &&
            blk2->yMin <= blk1->yMax &&
            blk2->yMax >= blk1->yMin)
          before = gTrue;
      } else {
        if (blk2->xMin >= blk1->xMax &&
            blk2->yMin <= blk1->yMax &&
            blk2->yMax >= blk1->yMin)
          before = gTrue;
      }

      if (blk2->yMax <= blk1->yMin)
        before = gTrue;
    } else {
      if (blk2->isBeforeByRule1(blk1)) {
        // Rule (1) blk1 and blk2 overlap, and blk2 is above blk1.
        before = gTrue;
#if 0   // for debugging
        printf("rule1: %.2f..%.2f %.2f..%.2f %.2f..%.2f %.2f..%.2f\n",
	       blk2->ExMin, blk2->ExMax, blk2->EyMin, blk2->EyMax,
	       blk1->ExMin, blk1->ExMax, blk1->EyMin, blk1->EyMax);
#endif
      } else if (blk2->isBeforeByRule2(blk1)) {
        // Rule (2) blk2 left of blk1, and no intervening blk3
        //          such that blk1 is before blk3 by rule 1,
        //          and blk3 is before blk2 by rule 1.
        before = gTrue;
        for (blk3 = blkList; blk3; blk3 = blk3->next) {
	  if (blk3 == blk2 || blk3 == blk1) {
	    continue;
	  }
	  if (blk1->isBeforeByRule1(blk3) &&
	      blk3->isBeforeByRule1(blk2)) {
	    before = gFalse;
	    break;
	  }
        }
#if 0 // for debugging
        if (before) {
	  printf("rule2: %.2f..%.2f %.2f..%.2f %.2f..%.2f %.2f..%.2f\n",
	         blk1->ExMin, blk1->ExMax, blk1->EyMin, blk1->EyMax,
	         blk2->ExMin, blk2->ExMax, blk2->EyMin, blk2->EyMax);
        }
#endif
      }
    }
    if (before) {
      // blk2 is before blk1, so it needs to be visited
      // before we can add blk1 to the sorted list.
      sortPos = blk2->visitDepthFirst(blkList, pos2, sorted, sortPos, visited);
    }
  }
#if 0 // for debugging
  printf("sorted: %d %.2f..%.2f %.2f..%.2f\n",
	 sortPos, blk1->ExMin, blk1->ExMax, blk1->EyMin, blk1->EyMax);
#endif
  sorted[sortPos++] = blk1;
  return sortPos;
}

//------------------------------------------------------------------------
// TextFlow
//------------------------------------------------------------------------

TextFlow::TextFlow(TextPage *pageA, TextBlock *blk) {
  page = pageA;
  xMin = blk->xMin;
  xMax = blk->xMax;
  yMin = blk->yMin;
  yMax = blk->yMax;
  priMin = blk->priMin;
  priMax = blk->priMax;
  blocks = lastBlk = blk;
  next = NULL;
}

TextFlow::~TextFlow() {
  TextBlock *blk;

  while (blocks) {
    blk = blocks;
    blocks = blocks->next;
    delete blk;
  }
}

void TextFlow::addBlock(TextBlock *blk) {
  if (lastBlk) {
    lastBlk->next = blk;
  } else {
    blocks = blk;
  }
  lastBlk = blk;
  if (blk->xMin < xMin) {
    xMin = blk->xMin;
  }
  if (blk->xMax > xMax) {
    xMax = blk->xMax;
  }
  if (blk->yMin < yMin) {
    yMin = blk->yMin;
  }
  if (blk->yMax > yMax) {
    yMax = blk->yMax;
  }
}

GBool TextFlow::blockFits(TextBlock *blk, TextBlock *prevBlk) {
  GBool fits;

  // lower blocks must use smaller fonts
  if (blk->lines->words->fontSize > lastBlk->lines->words->fontSize) {
    return gFalse;
  }

  fits = gFalse; // make gcc happy
  switch (page->primaryRot) {
  case 0:
    fits = blk->xMin >= priMin && blk->xMax <= priMax;
    break;
  case 1:
    fits = blk->yMin >= priMin && blk->yMax <= priMax;
    break;
  case 2:
    fits = blk->xMin >= priMin && blk->xMax <= priMax;
    break;
  case 3:
    fits = blk->yMin >= priMin && blk->yMax <= priMax;
    break;
  }
  return fits;
}

#if TEXTOUT_WORD_LIST

//------------------------------------------------------------------------
// TextWordList
//------------------------------------------------------------------------

TextWordList::TextWordList(TextPage *text, GBool physLayout) {
  TextFlow *flow;
  TextBlock *blk;
  TextLine *line;
  TextWord *word;
  TextWord **wordArray;
  int nWords, i;

  words = new GooList();

  if (text->rawOrder) {
    for (word = text->rawWords; word; word = word->next) {
      words->append(word);
    }

  } else if (physLayout) {
    // this is inefficient, but it's also the least useful of these
    // three cases
    nWords = 0;
    for (flow = text->flows; flow; flow = flow->next) {
      for (blk = flow->blocks; blk; blk = blk->next) {
	for (line = blk->lines; line; line = line->next) {
	  for (word = line->words; word; word = word->next) {
	    ++nWords;
	  }
	}
      }
    }
    wordArray = (TextWord **)gmallocn(nWords, sizeof(TextWord *));
    i = 0;
    for (flow = text->flows; flow; flow = flow->next) {
      for (blk = flow->blocks; blk; blk = blk->next) {
	for (line = blk->lines; line; line = line->next) {
	  for (word = line->words; word; word = word->next) {
	    wordArray[i++] = word;
	  }
	}
      }
    }
    qsort(wordArray, nWords, sizeof(TextWord *), &TextWord::cmpYX);
    for (i = 0; i < nWords; ++i) {
      words->append(wordArray[i]);
    }
    gfree(wordArray);

  } else {
    for (flow = text->flows; flow; flow = flow->next) {
      for (blk = flow->blocks; blk; blk = blk->next) {
	for (line = blk->lines; line; line = line->next) {
	  for (word = line->words; word; word = word->next) {
	    words->append(word);
	  }
	}
      }
    }
  }
}

TextWordList::~TextWordList() {
  delete words;
}

int TextWordList::getLength() {
  return words->getLength();
}

TextWord *TextWordList::get(int idx) {
  if (idx < 0 || idx >= words->getLength()) {
    return NULL;
  }
  return (TextWord *)words->get(idx);
}

#endif // TEXTOUT_WORD_LIST

//------------------------------------------------------------------------
// TextPage
//------------------------------------------------------------------------

TextPage::TextPage(GBool rawOrderA) {
  int rot;

  refCnt = 1;
  rawOrder = rawOrderA;
  curWord = NULL;
  charPos = 0;
  curFont = NULL;
  curFontSize = 0;
  nest = 0;
  nTinyChars = 0;
  lastCharOverlap = gFalse;
  if (!rawOrder) {
    for (rot = 0; rot < 4; ++rot) {
      pools[rot] = new TextPool();
    }
  }
  flows = NULL;
  blocks = NULL;
  rawWords = NULL;
  rawLastWord = NULL;
  fonts = new GooList();
  lastFindXMin = lastFindYMin = 0;
  haveLastFind = gFalse;
  underlines = new GooList();
  links = new GooList();
}

TextPage::~TextPage() {
  int rot;

  clear();
  if (!rawOrder) {
    for (rot = 0; rot < 4; ++rot) {
      delete pools[rot];
    }
  }
  delete fonts;
  deleteGooList(underlines, TextUnderline);
  deleteGooList(links, TextLink);
}

void TextPage::incRefCnt() {
  refCnt++;
}

void TextPage::decRefCnt() {
  if (--refCnt == 0)
    delete this;
}

void TextPage::startPage(GfxState *state) {
  clear();
  if (state) {
    pageWidth = state->getPageWidth();
    pageHeight = state->getPageHeight();
  } else {
    pageWidth = pageHeight = 0;
  }
}

void TextPage::endPage() {
  if (curWord) {
    endWord();
  }
}

void TextPage::clear() {
  int rot;
  TextFlow *flow;
  TextWord *word;

  if (curWord) {
    delete curWord;
    curWord = NULL;
  }
  if (rawOrder) {
    while (rawWords) {
      word = rawWords;
      rawWords = rawWords->next;
      delete word;
    }
  } else {
    for (rot = 0; rot < 4; ++rot) {
      delete pools[rot];
    }
    while (flows) {
      flow = flows;
      flows = flows->next;
      delete flow;
    }
    gfree(blocks);
  }
  deleteGooList(fonts, TextFontInfo);

  curWord = NULL;
  charPos = 0;
  curFont = NULL;
  curFontSize = 0;
  nest = 0;
  nTinyChars = 0;
  if (!rawOrder) {
    for (rot = 0; rot < 4; ++rot) {
      pools[rot] = new TextPool();
    }
  }
  flows = NULL;
  blocks = NULL;
  rawWords = NULL;
  rawLastWord = NULL;
  fonts = new GooList();
}

void TextPage::updateFont(GfxState *state) {
  GfxFont *gfxFont;
  double *fm;
  char *name;
  int code, mCode, letterCode, anyCode;
  double w;
  int i;

  // get the font info object
  curFont = NULL;
  for (i = 0; i < fonts->getLength(); ++i) {
    curFont = (TextFontInfo *)fonts->get(i);
    if (curFont->matches(state)) {
      break;
    }
    curFont = NULL;
  }
  if (!curFont) {
    curFont = new TextFontInfo(state);
    fonts->append(curFont);
  }

  // adjust the font size
  gfxFont = state->getFont();
  curFontSize = state->getTransformedFontSize();
  if (gfxFont && gfxFont->getType() == fontType3) {
    // This is a hack which makes it possible to deal with some Type 3
    // fonts.  The problem is that it's impossible to know what the
    // base coordinate system used in the font is without actually
    // rendering the font.  This code tries to guess by looking at the
    // width of the character 'm' (which breaks if the font is a
    // subset that doesn't contain 'm').
    mCode = letterCode = anyCode = -1;
    for (code = 0; code < 256; ++code) {
      name = ((Gfx8BitFont *)gfxFont)->getCharName(code);
      if (name && name[0] == 'm' && name[1] == '\0') {
	mCode = code;
      }
      if (letterCode < 0 && name && name[1] == '\0' &&
	  ((name[0] >= 'A' && name[0] <= 'Z') ||
	   (name[0] >= 'a' && name[0] <= 'z'))) {
	letterCode = code;
      }
      if (anyCode < 0 && name &&
	  ((Gfx8BitFont *)gfxFont)->getWidth(code) > 0) {
	anyCode = code;
      }
    }
    if (mCode >= 0 &&
	(w = ((Gfx8BitFont *)gfxFont)->getWidth(mCode)) > 0) {
      // 0.6 is a generic average 'm' width -- yes, this is a hack
      curFontSize *= w / 0.6;
    } else if (letterCode >= 0 &&
	       (w = ((Gfx8BitFont *)gfxFont)->getWidth(letterCode)) > 0) {
      // even more of a hack: 0.5 is a generic letter width
      curFontSize *= w / 0.5;
    } else if (anyCode >= 0 &&
	       (w = ((Gfx8BitFont *)gfxFont)->getWidth(anyCode)) > 0) {
      // better than nothing: 0.5 is a generic character width
      curFontSize *= w / 0.5;
    }
    fm = gfxFont->getFontMatrix();
    if (fm[0] != 0) {
      curFontSize *= fabs(fm[3] / fm[0]);
    }
  }
}

void TextPage::beginWord(GfxState *state, double x0, double y0) {
  GfxFont *gfxFont;
  double *fontm;
  double m[4], m2[4];
  int rot;

  // This check is needed because Type 3 characters can contain
  // text-drawing operations (when TextPage is being used via
  // {X,Win}SplashOutputDev rather than TextOutputDev).
  if (curWord) {
    ++nest;
    return;
  }

  // compute the rotation
  state->getFontTransMat(&m[0], &m[1], &m[2], &m[3]);
  gfxFont = state->getFont();
  if (gfxFont && gfxFont->getType() == fontType3) {
    fontm = state->getFont()->getFontMatrix();
    m2[0] = fontm[0] * m[0] + fontm[1] * m[2];
    m2[1] = fontm[0] * m[1] + fontm[1] * m[3];
    m2[2] = fontm[2] * m[0] + fontm[3] * m[2];
    m2[3] = fontm[2] * m[1] + fontm[3] * m[3];
    m[0] = m2[0];
    m[1] = m2[1];
    m[2] = m2[2];
    m[3] = m2[3];
  }
  if (fabs(m[0] * m[3]) > fabs(m[1] * m[2])) {
    rot = (m[3] < 0) ? 0 : 2;
  } else {
    rot = (m[2] > 0) ? 1 : 3;
  }

  curWord = new TextWord(state, rot, x0, y0, charPos, curFont, curFontSize);
}

void TextPage::addChar(GfxState *state, double x, double y,
		       double dx, double dy,
		       CharCode c, int nBytes, Unicode *u, int uLen) {
  double x1, y1, w1, h1, dx2, dy2, base, sp, delta;
  GBool overlap;
  int i;

  // subtract char and word spacing from the dx,dy values
  sp = state->getCharSpace();
  if (c == (CharCode)0x20) {
    sp += state->getWordSpace();
  }
  state->textTransformDelta(sp * state->getHorizScaling(), 0, &dx2, &dy2);
  dx -= dx2;
  dy -= dy2;
  state->transformDelta(dx, dy, &w1, &h1);

  // throw away chars that aren't inside the page bounds
  // (and also do a sanity check on the character size)
  state->transform(x, y, &x1, &y1);
  if (x1 + w1 < 0 || x1 > pageWidth ||
      y1 + h1 < 0 || y1 > pageHeight ||
      w1 > pageWidth || h1 > pageHeight) {
    charPos += nBytes;
    return;
  }

  // check the tiny chars limit
  if (!globalParams->getTextKeepTinyChars() &&
      fabs(w1) < 3 && fabs(h1) < 3) {
    if (++nTinyChars > 50000) {
      charPos += nBytes;
      return;
    }
  }

  // break words at space character
  if (uLen == 1 && u[0] == (Unicode)0x20) {
    if (curWord) {
      ++curWord->charLen;
    }
    charPos += nBytes;
    endWord();
    return;
  }

  // start a new word if:
  // (1) this character doesn't fall in the right place relative to
  //     the end of the previous word (this places upper and lower
  //     constraints on the position deltas along both the primary
  //     and secondary axes), or
  // (2) this character overlaps the previous one (duplicated text), or
  // (3) the previous character was an overlap (we want each duplicated
  //     character to be in a word by itself at this stage),
  // (4) the font size has changed
  if (curWord && curWord->len > 0) {
    base = sp = delta = 0; // make gcc happy
    switch (curWord->rot) {
    case 0:
      base = y1;
      sp = x1 - curWord->xMax;
      delta = x1 - curWord->edge[curWord->len - 1];
      break;
    case 1:
      base = x1;
      sp = y1 - curWord->yMax;
      delta = y1 - curWord->edge[curWord->len - 1];
      break;
    case 2:
      base = y1;
      sp = curWord->xMin - x1;
      delta = curWord->edge[curWord->len - 1] - x1;
      break;
    case 3:
      base = x1;
      sp = curWord->yMin - y1;
      delta = curWord->edge[curWord->len - 1] - y1;
      break;
    }
    overlap = fabs(delta) < dupMaxPriDelta * curWord->fontSize &&
              fabs(base - curWord->base) < dupMaxSecDelta * curWord->fontSize;
    if (overlap || lastCharOverlap ||
	sp < -minDupBreakOverlap * curWord->fontSize ||
	sp > minWordBreakSpace * curWord->fontSize ||
	fabs(base - curWord->base) > 0.5 ||
	curFontSize != curWord->fontSize) {
      endWord();
    }
    lastCharOverlap = overlap;
  } else {
    lastCharOverlap = gFalse;
  }

  if (uLen != 0) {
    // start a new word if needed
    if (!curWord) {
      beginWord(state, x, y);
    }

    // page rotation and/or transform matrices can cause text to be
    // drawn in reverse order -- in this case, swap the begin/end
    // coordinates and break text into individual chars
    if ((curWord->rot == 0 && w1 < 0) ||
        (curWord->rot == 1 && h1 < 0) ||
        (curWord->rot == 2 && w1 > 0) ||
        (curWord->rot == 3 && h1 > 0)) {
      endWord();
      beginWord(state, x + dx, y + dy);
      x1 += w1;
      y1 += h1;
      w1 = -w1;
      h1 = -h1;
    }

    // add the characters to the current word
    w1 /= uLen;
    h1 /= uLen;
    for (i = 0; i < uLen; ++i) {
      if (u[i] >= 0xd800 && u[i] < 0xdc00) { /* surrogate pair */
	if (i + 1 < uLen && u[i+1] >= 0xdc00 && u[i+1] < 0xe000) {
	  /* next code is a low surrogate */
	  Unicode uu = (((u[i] & 0x3ff) << 10) | (u[i+1] & 0x3ff)) + 0x10000;
	  i++;
	  curWord->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, c, uu);
	} else {
	    /* missing low surrogate
	     replace it with REPLACEMENT CHARACTER (U+FFFD) */
	  curWord->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, c, 0xfffd);
	}
      } else if (u[i] >= 0xdc00 && u[i] < 0xe000) {
	  /* invalid low surrogate
	   replace it with REPLACEMENT CHARACTER (U+FFFD) */
	curWord->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, c, 0xfffd);
      } else {
	curWord->addChar(state, x1 + i*w1, y1 + i*h1, w1, h1, c, u[i]);
      }
    }
  }
  if (curWord) {
    curWord->charLen += nBytes;
  }
  charPos += nBytes;
}

void TextPage::endWord() {
  // This check is needed because Type 3 characters can contain
  // text-drawing operations (when TextPage is being used via
  // {X,Win}SplashOutputDev rather than TextOutputDev).
  if (nest > 0) {
    --nest;
    return;
  }

  if (curWord) {
    addWord(curWord);
    curWord = NULL;
  }
}

void TextPage::addWord(TextWord *word) {
  // throw away zero-length words -- they don't have valid xMin/xMax
  // values, and they're useless anyway
  if (word->len == 0) {
    delete word;
    return;
  }

  if (rawOrder) {
    if (rawLastWord) {
      rawLastWord->next = word;
    } else {
      rawWords = word;
    }
    rawLastWord = word;
  } else {
    pools[word->rot]->addWord(word);
  }
}

void TextPage::addUnderline(double x0, double y0, double x1, double y1) {
  underlines->append(new TextUnderline(x0, y0, x1, y1));
}

void TextPage::addLink(int xMin, int yMin, int xMax, int yMax, Link *link) {
  links->append(new TextLink(xMin, yMin, xMax, yMax, link));
}

void TextPage::coalesce(GBool physLayout, GBool doHTML) {
  UnicodeMap *uMap;
  TextPool *pool;
  TextWord *word0, *word1, *word2;
  TextLine *line;
  TextBlock *blkList, *blk, *lastBlk, *blk0, *blk1, *blk2;
  TextFlow *flow, *lastFlow;
  TextUnderline *underline;
  TextLink *link;
  int rot, poolMinBaseIdx, baseIdx, startBaseIdx, endBaseIdx;
  double minBase, maxBase, newMinBase, newMaxBase;
  double fontSize, colSpace1, colSpace2, lineSpace, intraLineSpace, blkSpace;
  GBool found;
  int count[4];
  int lrCount;
  int col1, col2;
  int i, j, n;

  if (rawOrder) {
    primaryRot = 0;
    primaryLR = gTrue;
    return;
  }

  uMap = globalParams->getTextEncoding();
  blkList = NULL;
  lastBlk = NULL;
  nBlocks = 0;
  primaryRot = -1;

#if 0 // for debugging
  printf("*** initial words ***\n");
  for (rot = 0; rot < 4; ++rot) {
    pool = pools[rot];
    for (baseIdx = pool->minBaseIdx; baseIdx <= pool->maxBaseIdx; ++baseIdx) {
      for (word0 = pool->getPool(baseIdx); word0; word0 = word0->next) {
	printf("    word: x=%.2f..%.2f y=%.2f..%.2f base=%.2f fontSize=%.2f rot=%d link=%p '",
	       word0->xMin, word0->xMax, word0->yMin, word0->yMax,
	       word0->base, word0->fontSize, rot*90, word0->link);
	for (i = 0; i < word0->len; ++i) {
	  fputc(word0->text[i] & 0xff, stdout);
	}
	printf("'\n");
      }
    }
  }
  printf("\n");
#endif

#if 0 //~ for debugging
  for (i = 0; i < underlines->getLength(); ++i) {
    underline = (TextUnderline *)underlines->get(i);
    printf("underline: x=%g..%g y=%g..%g horiz=%d\n",
	   underline->x0, underline->x1, underline->y0, underline->y1,
	   underline->horiz);
  }
#endif

  if (doHTML) {

    //----- handle underlining
    for (i = 0; i < underlines->getLength(); ++i) {
      underline = (TextUnderline *)underlines->get(i);
      if (underline->horiz) {
	// rot = 0
	if (pools[0]->minBaseIdx <= pools[0]->maxBaseIdx) {
	  startBaseIdx = pools[0]->getBaseIdx(underline->y0 + minUnderlineGap);
	  endBaseIdx = pools[0]->getBaseIdx(underline->y0 + maxUnderlineGap);
	  for (j = startBaseIdx; j <= endBaseIdx; ++j) {
	    for (word0 = pools[0]->getPool(j); word0; word0 = word0->next) {
	      //~ need to check the y value against the word baseline
	      if (underline->x0 < word0->xMin + underlineSlack &&
		  word0->xMax - underlineSlack < underline->x1) {
		word0->underlined = gTrue;
	      }
	    }
	  }
	}

	// rot = 2
	if (pools[2]->minBaseIdx <= pools[2]->maxBaseIdx) {
	  startBaseIdx = pools[2]->getBaseIdx(underline->y0 - maxUnderlineGap);
	  endBaseIdx = pools[2]->getBaseIdx(underline->y0 - minUnderlineGap);
	  for (j = startBaseIdx; j <= endBaseIdx; ++j) {
	    for (word0 = pools[2]->getPool(j); word0; word0 = word0->next) {
	      if (underline->x0 < word0->xMin + underlineSlack &&
		  word0->xMax - underlineSlack < underline->x1) {
		word0->underlined = gTrue;
	      }
	    }
	  }
	}
      } else {
	// rot = 1
	if (pools[1]->minBaseIdx <= pools[1]->maxBaseIdx) {
	  startBaseIdx = pools[1]->getBaseIdx(underline->x0 - maxUnderlineGap);
	  endBaseIdx = pools[1]->getBaseIdx(underline->x0 - minUnderlineGap);
	  for (j = startBaseIdx; j <= endBaseIdx; ++j) {
	    for (word0 = pools[1]->getPool(j); word0; word0 = word0->next) {
	      if (underline->y0 < word0->yMin + underlineSlack &&
		  word0->yMax - underlineSlack < underline->y1) {
		word0->underlined = gTrue;
	      }
	    }
	  }
	}

	// rot = 3
	if (pools[3]->minBaseIdx <= pools[3]->maxBaseIdx) {
	  startBaseIdx = pools[3]->getBaseIdx(underline->x0 + minUnderlineGap);
	  endBaseIdx = pools[3]->getBaseIdx(underline->x0 + maxUnderlineGap);
	  for (j = startBaseIdx; j <= endBaseIdx; ++j) {
	    for (word0 = pools[3]->getPool(j); word0; word0 = word0->next) {
	      if (underline->y0 < word0->yMin + underlineSlack &&
		  word0->yMax - underlineSlack < underline->y1) {
		word0->underlined = gTrue;
	      }
	    }
	  }
	}
      }
    }

    //----- handle links
    for (i = 0; i < links->getLength(); ++i) {
      link = (TextLink *)links->get(i);

      // rot = 0
      if (pools[0]->minBaseIdx <= pools[0]->maxBaseIdx) {
	startBaseIdx = pools[0]->getBaseIdx(link->yMin);
	endBaseIdx = pools[0]->getBaseIdx(link->yMax);
	for (j = startBaseIdx; j <= endBaseIdx; ++j) {
	  for (word0 = pools[0]->getPool(j); word0; word0 = word0->next) {
	    if (link->xMin < word0->xMin + hyperlinkSlack &&
		word0->xMax - hyperlinkSlack < link->xMax &&
		link->yMin < word0->yMin + hyperlinkSlack &&
		word0->yMax - hyperlinkSlack < link->yMax) {
	      word0->link = link->link;
	    }
	  }
	}
      }

      // rot = 2
      if (pools[2]->minBaseIdx <= pools[2]->maxBaseIdx) {
	startBaseIdx = pools[2]->getBaseIdx(link->yMin);
	endBaseIdx = pools[2]->getBaseIdx(link->yMax);
	for (j = startBaseIdx; j <= endBaseIdx; ++j) {
	  for (word0 = pools[2]->getPool(j); word0; word0 = word0->next) {
	    if (link->xMin < word0->xMin + hyperlinkSlack &&
		word0->xMax - hyperlinkSlack < link->xMax &&
		link->yMin < word0->yMin + hyperlinkSlack &&
		word0->yMax - hyperlinkSlack < link->yMax) {
	      word0->link = link->link;
	    }
	  }
	}
      }

      // rot = 1
      if (pools[1]->minBaseIdx <= pools[1]->maxBaseIdx) {
	startBaseIdx = pools[1]->getBaseIdx(link->xMin);
	endBaseIdx = pools[1]->getBaseIdx(link->xMax);
	for (j = startBaseIdx; j <= endBaseIdx; ++j) {
	  for (word0 = pools[1]->getPool(j); word0; word0 = word0->next) {
	    if (link->yMin < word0->yMin + hyperlinkSlack &&
		word0->yMax - hyperlinkSlack < link->yMax &&
		link->xMin < word0->xMin + hyperlinkSlack &&
		word0->xMax - hyperlinkSlack < link->xMax) {
	      word0->link = link->link;
	    }
	  }
	}
      }

      // rot = 3
      if (pools[3]->minBaseIdx <= pools[3]->maxBaseIdx) {
	startBaseIdx = pools[3]->getBaseIdx(link->xMin);
	endBaseIdx = pools[3]->getBaseIdx(link->xMax);
	for (j = startBaseIdx; j <= endBaseIdx; ++j) {
	  for (word0 = pools[3]->getPool(j); word0; word0 = word0->next) {
	    if (link->yMin < word0->yMin + hyperlinkSlack &&
		word0->yMax - hyperlinkSlack < link->yMax &&
		link->xMin < word0->xMin + hyperlinkSlack &&
		word0->xMax - hyperlinkSlack < link->xMax) {
	      word0->link = link->link;
	    }
	  }
	}
      }
    }
  }

  //----- assemble the blocks

  //~ add an outer loop for writing mode (vertical text)

  // build blocks for each rotation value
  for (rot = 0; rot < 4; ++rot) {
    pool = pools[rot];
    poolMinBaseIdx = pool->minBaseIdx;
    count[rot] = 0;

    // add blocks until no more words are left
    while (1) {

      // find the first non-empty line in the pool
      for (;
	   poolMinBaseIdx <= pool->maxBaseIdx &&
	     !pool->getPool(poolMinBaseIdx);
	   ++poolMinBaseIdx) ;
      if (poolMinBaseIdx > pool->maxBaseIdx) {
	break;
      }

      // look for the left-most word in the first four lines of the
      // pool -- this avoids starting with a superscript word
      startBaseIdx = poolMinBaseIdx;
      for (baseIdx = poolMinBaseIdx + 1;
	   baseIdx < poolMinBaseIdx + 4 && baseIdx <= pool->maxBaseIdx;
	   ++baseIdx) {
	if (!pool->getPool(baseIdx)) {
	  continue;
	}
	if (pool->getPool(baseIdx)->primaryCmp(pool->getPool(startBaseIdx))
	    < 0) {
	  startBaseIdx = baseIdx;
	}
      }

      // create a new block
      word0 = pool->getPool(startBaseIdx);
      pool->setPool(startBaseIdx, word0->next);
      word0->next = NULL;
      blk = new TextBlock(this, rot);
      blk->addWord(word0);

      fontSize = word0->fontSize;
      minBase = maxBase = word0->base;
      colSpace1 = minColSpacing1 * fontSize;
      colSpace2 = minColSpacing2 * fontSize;
      lineSpace = maxLineSpacingDelta * fontSize;
      intraLineSpace = maxIntraLineDelta * fontSize;

      // add words to the block
      do {
	found = gFalse;

	// look for words on the line above the current top edge of
	// the block
	newMinBase = minBase;
	for (baseIdx = pool->getBaseIdx(minBase);
	     baseIdx >= pool->getBaseIdx(minBase - lineSpace);
	     --baseIdx) {
	  word0 = NULL;
	  word1 = pool->getPool(baseIdx);
	  while (word1) {
	    if (word1->base < minBase &&
		word1->base >= minBase - lineSpace &&
		((rot == 0 || rot == 2)
		 ? (word1->xMin < blk->xMax && word1->xMax > blk->xMin)
		 : (word1->yMin < blk->yMax && word1->yMax > blk->yMin)) &&
		fabs(word1->fontSize - fontSize) <
		  maxBlockFontSizeDelta1 * fontSize) {
	      word2 = word1;
	      if (word0) {
		word0->next = word1->next;
	      } else {
		pool->setPool(baseIdx, word1->next);
	      }
	      word1 = word1->next;
	      word2->next = NULL;
	      blk->addWord(word2);
	      found = gTrue;
	      newMinBase = word2->base;
	    } else {
	      word0 = word1;
	      word1 = word1->next;
	    }
	  }
	}
	minBase = newMinBase;

	// look for words on the line below the current bottom edge of
	// the block
	newMaxBase = maxBase;
	for (baseIdx = pool->getBaseIdx(maxBase);
	     baseIdx <= pool->getBaseIdx(maxBase + lineSpace);
	     ++baseIdx) {
	  word0 = NULL;
	  word1 = pool->getPool(baseIdx);
	  while (word1) {
	    if (word1->base > maxBase &&
		word1->base <= maxBase + lineSpace &&
		((rot == 0 || rot == 2)
		 ? (word1->xMin < blk->xMax && word1->xMax > blk->xMin)
		 : (word1->yMin < blk->yMax && word1->yMax > blk->yMin)) &&
		fabs(word1->fontSize - fontSize) <
		  maxBlockFontSizeDelta1 * fontSize) {
	      word2 = word1;
	      if (word0) {
		word0->next = word1->next;
	      } else {
		pool->setPool(baseIdx, word1->next);
	      }
	      word1 = word1->next;
	      word2->next = NULL;
	      blk->addWord(word2);
	      found = gTrue;
	      newMaxBase = word2->base;
	    } else {
	      word0 = word1;
	      word1 = word1->next;
	    }
	  }
	}
	maxBase = newMaxBase;

	// look for words that are on lines already in the block, and
	// that overlap the block horizontally
	for (baseIdx = pool->getBaseIdx(minBase - intraLineSpace);
	     baseIdx <= pool->getBaseIdx(maxBase + intraLineSpace);
	     ++baseIdx) {
	  word0 = NULL;
	  word1 = pool->getPool(baseIdx);
	  while (word1) {
	    if (word1->base >= minBase - intraLineSpace &&
		word1->base <= maxBase + intraLineSpace &&
		((rot == 0 || rot == 2)
		 ? (word1->xMin < blk->xMax + colSpace1 &&
		    word1->xMax > blk->xMin - colSpace1)
		 : (word1->yMin < blk->yMax + colSpace1 &&
		    word1->yMax > blk->yMin - colSpace1)) &&
		fabs(word1->fontSize - fontSize) <
		  maxBlockFontSizeDelta2 * fontSize) {
	      word2 = word1;
	      if (word0) {
		word0->next = word1->next;
	      } else {
		pool->setPool(baseIdx, word1->next);
	      }
	      word1 = word1->next;
	      word2->next = NULL;
	      blk->addWord(word2);
	      found = gTrue;
	    } else {
	      word0 = word1;
	      word1 = word1->next;
	    }
	  }
	}

	// only check for outlying words (the next two chunks of code)
	// if we didn't find anything else
	if (found) {
	  continue;
	}

	// scan down the left side of the block, looking for words
	// that are near (but not overlapping) the block; if there are
	// three or fewer, add them to the block
	n = 0;
	for (baseIdx = pool->getBaseIdx(minBase - intraLineSpace);
	     baseIdx <= pool->getBaseIdx(maxBase + intraLineSpace);
	     ++baseIdx) {
	  word1 = pool->getPool(baseIdx);
	  while (word1) {
	    if (word1->base >= minBase - intraLineSpace &&
		word1->base <= maxBase + intraLineSpace &&
		((rot == 0 || rot == 2)
		 ? (word1->xMax <= blk->xMin &&
		    word1->xMax > blk->xMin - colSpace2)
		 : (word1->yMax <= blk->yMin &&
		    word1->yMax > blk->yMin - colSpace2)) &&
		fabs(word1->fontSize - fontSize) <
		  maxBlockFontSizeDelta3 * fontSize) {
	      ++n;
	      break;
	    }
	    word1 = word1->next;
	  }
	}
	if (n > 0 && n <= 3) {
	  for (baseIdx = pool->getBaseIdx(minBase - intraLineSpace);
	       baseIdx <= pool->getBaseIdx(maxBase + intraLineSpace);
	       ++baseIdx) {
	    word0 = NULL;
	    word1 = pool->getPool(baseIdx);
	    while (word1) {
	      if (word1->base >= minBase - intraLineSpace &&
		  word1->base <= maxBase + intraLineSpace &&
		  ((rot == 0 || rot == 2)
		   ? (word1->xMax <= blk->xMin &&
		      word1->xMax > blk->xMin - colSpace2)
		   : (word1->yMax <= blk->yMin &&
		      word1->yMax > blk->yMin - colSpace2)) &&
		  fabs(word1->fontSize - fontSize) <
		    maxBlockFontSizeDelta3 * fontSize) {
		word2 = word1;
		if (word0) {
		  word0->next = word1->next;
		} else {
		  pool->setPool(baseIdx, word1->next);
		}
		word1 = word1->next;
		word2->next = NULL;
		blk->addWord(word2);
		if (word2->base < minBase) {
		  minBase = word2->base;
		} else if (word2->base > maxBase) {
		  maxBase = word2->base;
		}
		found = gTrue;
		break;
	      } else {
		word0 = word1;
		word1 = word1->next;
	      }
	    }
	  }
	}

	// scan down the right side of the block, looking for words
	// that are near (but not overlapping) the block; if there are
	// three or fewer, add them to the block
	n = 0;
	for (baseIdx = pool->getBaseIdx(minBase - intraLineSpace);
	     baseIdx <= pool->getBaseIdx(maxBase + intraLineSpace);
	     ++baseIdx) {
	  word1 = pool->getPool(baseIdx);
	  while (word1) {
	    if (word1->base >= minBase - intraLineSpace &&
		word1->base <= maxBase + intraLineSpace &&
		((rot == 0 || rot == 2)
		 ? (word1->xMin >= blk->xMax &&
		    word1->xMin < blk->xMax + colSpace2)
		 : (word1->yMin >= blk->yMax &&
		    word1->yMin < blk->yMax + colSpace2)) &&
		fabs(word1->fontSize - fontSize) <
		  maxBlockFontSizeDelta3 * fontSize) {
	      ++n;
	      break;
	    }
	    word1 = word1->next;
	  }
	}
	if (n > 0 && n <= 3) {
	  for (baseIdx = pool->getBaseIdx(minBase - intraLineSpace);
	       baseIdx <= pool->getBaseIdx(maxBase + intraLineSpace);
	       ++baseIdx) {
	    word0 = NULL;
	    word1 = pool->getPool(baseIdx);
	    while (word1) {
	      if (word1->base >= minBase - intraLineSpace &&
		  word1->base <= maxBase + intraLineSpace &&
		  ((rot == 0 || rot == 2)
		   ? (word1->xMin >= blk->xMax &&
		      word1->xMin < blk->xMax + colSpace2)
		   : (word1->yMin >= blk->yMax &&
		      word1->yMin < blk->yMax + colSpace2)) &&
		  fabs(word1->fontSize - fontSize) <
		    maxBlockFontSizeDelta3 * fontSize) {
		word2 = word1;
		if (word0) {
		  word0->next = word1->next;
		} else {
		  pool->setPool(baseIdx, word1->next);
		}
		word1 = word1->next;
		word2->next = NULL;
		blk->addWord(word2);
		if (word2->base < minBase) {
		  minBase = word2->base;
		} else if (word2->base > maxBase) {
		  maxBase = word2->base;
		}
		found = gTrue;
		break;
	      } else {
		word0 = word1;
		word1 = word1->next;
	      }
	    }
	  }
	}

      } while (found);

      //~ need to compute the primary writing mode (horiz/vert) in
      //~ addition to primary rotation

      // coalesce the block, and add it to the list
      blk->coalesce(uMap);
      if (lastBlk) {
	lastBlk->next = blk;
      } else {
	blkList = blk;
      }
      lastBlk = blk;
      count[rot] += blk->charCount;
      if (primaryRot < 0 || count[rot] > count[primaryRot]) {
	primaryRot = rot;
      }
      ++nBlocks;
    }
  }

#if 0 // for debugging
  printf("*** rotation ***\n");
  for (rot = 0; rot < 4; ++rot) {
    printf("  %d: %6d\n", rot, count[rot]);
  }
  printf("  primary rot = %d\n", primaryRot);
  printf("\n");
#endif

#if 0 // for debugging
  printf("*** blocks ***\n");
  for (blk = blkList; blk; blk = blk->next) {
    printf("block: rot=%d x=%.2f..%.2f y=%.2f..%.2f\n",
	   blk->rot, blk->xMin, blk->xMax, blk->yMin, blk->yMax);
    for (line = blk->lines; line; line = line->next) {
      printf("  line: x=%.2f..%.2f y=%.2f..%.2f base=%.2f\n",
	     line->xMin, line->xMax, line->yMin, line->yMax, line->base);
      for (word0 = line->words; word0; word0 = word0->next) {
	printf("    word: x=%.2f..%.2f y=%.2f..%.2f base=%.2f fontSize=%.2f space=%d: '",
	       word0->xMin, word0->xMax, word0->yMin, word0->yMax,
	       word0->base, word0->fontSize, word0->spaceAfter);
	for (i = 0; i < word0->len; ++i) {
	  fputc(word0->text[i] & 0xff, stdout);
	}
	printf("'\n");
      }
    }
  }
  printf("\n");
#endif

  // determine the primary direction
  lrCount = 0;
  for (blk = blkList; blk; blk = blk->next) {
    for (line = blk->lines; line; line = line->next) {
      for (word0 = line->words; word0; word0 = word0->next) {
	for (i = 0; i < word0->len; ++i) {
	  if (unicodeTypeL(word0->text[i])) {
	    ++lrCount;
	  } else if (unicodeTypeR(word0->text[i])) {
	    --lrCount;
	  }
	}
      }
    }
  }
  primaryLR = lrCount >= 0;

#if 0 // for debugging
  printf("*** direction ***\n");
  printf("lrCount = %d\n", lrCount);
  printf("primaryLR = %d\n", primaryLR);
#endif

  //----- column assignment

  // sort blocks into xy order for column assignment
  if (blocks)
    gfree (blocks);
  blocks = (TextBlock **)gmallocn(nBlocks, sizeof(TextBlock *));
  for (blk = blkList, i = 0; blk; blk = blk->next, ++i) {
    blocks[i] = blk;
  }
  qsort(blocks, nBlocks, sizeof(TextBlock *), &TextBlock::cmpXYPrimaryRot);

  // column assignment
  for (i = 0; i < nBlocks; ++i) {
    blk0 = blocks[i];
    col1 = 0;
    for (j = 0; j < i; ++j) {
      blk1 = blocks[j];
      col2 = 0; // make gcc happy
      switch (primaryRot) {
      case 0:
	if (blk0->xMin > blk1->xMax) {
	  col2 = blk1->col + blk1->nColumns + 3;
	} else if (blk1->xMax == blk1->xMin) {
	  col2 = blk1->col;
	} else {
	  col2 = blk1->col + (int)(((blk0->xMin - blk1->xMin) /
				    (blk1->xMax - blk1->xMin)) *
				   blk1->nColumns);
	}
	break;
      case 1:
	if (blk0->yMin > blk1->yMax) {
	  col2 = blk1->col + blk1->nColumns + 3;
	} else if (blk1->yMax == blk1->yMin) {
	  col2 = blk1->col;
	} else {
	  col2 = blk1->col + (int)(((blk0->yMin - blk1->yMin) /
				    (blk1->yMax - blk1->yMin)) *
				   blk1->nColumns);
	}
	break;
      case 2:
	if (blk0->xMax < blk1->xMin) {
	  col2 = blk1->col + blk1->nColumns + 3;
	} else if (blk1->xMin == blk1->xMax) {
	  col2 = blk1->col;
	} else {
	  col2 = blk1->col + (int)(((blk0->xMax - blk1->xMax) /
				    (blk1->xMin - blk1->xMax)) *
				   blk1->nColumns);
	}
	break;
      case 3:
	if (blk0->yMax < blk1->yMin) {
	  col2 = blk1->col + blk1->nColumns + 3;
	} else if (blk1->yMin == blk1->yMax) {
	  col2 = blk1->col;
	} else {
	  col2 = blk1->col + (int)(((blk0->yMax - blk1->yMax) /
				    (blk1->yMin - blk1->yMax)) *
				   blk1->nColumns);
	}
	break;
      }
      if (col2 > col1) {
	col1 = col2;
      }
    }
    blk0->col = col1;
    for (line = blk0->lines; line; line = line->next) {
      for (j = 0; j <= line->len; ++j) {
	line->col[j] += col1;
      }
    }
  }

#if 0 // for debugging
  printf("*** blocks, after column assignment ***\n");
  for (blk = blkList; blk; blk = blk->next) {
    printf("block: rot=%d x=%.2f..%.2f y=%.2f..%.2f col=%d nCols=%d\n",
	   blk->rot, blk->xMin, blk->xMax, blk->yMin, blk->yMax, blk->col,
	   blk->nColumns);
    for (line = blk->lines; line; line = line->next) {
      printf("  line:\n");
      for (word0 = line->words; word0; word0 = word0->next) {
	printf("    word: x=%.2f..%.2f y=%.2f..%.2f base=%.2f fontSize=%.2f space=%d: '",
	       word0->xMin, word0->xMax, word0->yMin, word0->yMax,
	       word0->base, word0->fontSize, word0->spaceAfter);
	for (i = 0; i < word0->len; ++i) {
	  fputc(word0->text[i] & 0xff, stdout);
	}
	printf("'\n");
      }
    }
  }
  printf("\n");
#endif

  //----- reading order sort

  // compute space on left and right sides of each block
  for (i = 0; i < nBlocks; ++i) {
    blk0 = blocks[i];
    for (j = 0; j < nBlocks; ++j) {
      blk1 = blocks[j];
      if (blk1 != blk0) {
	blk0->updatePriMinMax(blk1);
      }
    }
  }

#if 0 // for debugging
  printf("PAGE\n");
#endif

  int sortPos = 0;
  GBool *visited = (GBool *)gmallocn(nBlocks, sizeof(GBool));
  for (i = 0; i < nBlocks; i++) {
    visited[i] = gFalse;
  }

  double bxMin0, byMin0, bxMin1, byMin1;
  int numTables = 0;
  int tableId = -1;
  int correspondenceX, correspondenceY;
  double xCentre1, yCentre1, xCentre2, yCentre2;
  double xCentre3, yCentre3, xCentre4, yCentre4;
  double deltaX, deltaY;
  TextBlock *fblk2 = NULL, *fblk3 = NULL, *fblk4 = NULL;

  for (blk1 = blkList; blk1; blk1 = blk1->next) {
    blk1->ExMin = blk1->xMin;
    blk1->ExMax = blk1->xMax;
    blk1->EyMin = blk1->yMin;
    blk1->EyMax = blk1->yMax;

    bxMin0 = DBL_MAX;
    byMin0 = DBL_MAX;
    bxMin1 = DBL_MAX;
    byMin1 = DBL_MAX;

    fblk2 = NULL;
    fblk3 = NULL;
    fblk4 = NULL;

    /*  find fblk2, fblk3 and fblk4 so that
     *  fblk2 is on the right of blk1 and overlap with blk1 in y axis
     *  fblk3 is under blk1 and overlap with blk1 in x axis
     *  fblk4 is under blk1 and on the right of blk1
     *  and they are closest to blk1
     */
    for (blk2 = blkList; blk2; blk2 = blk2->next) {
      if (blk2 != blk1) {
        if (blk2->yMin <= blk1->yMax &&
            blk2->yMax >= blk1->yMin &&
            blk2->xMin > blk1->xMax &&
            blk2->xMin < bxMin0) {
          bxMin0 = blk2->xMin;
          fblk2 = blk2;
        } else if (blk2->xMin <= blk1->xMax &&
                   blk2->xMax >= blk1->xMin &&
                   blk2->yMin > blk1->yMax &&
                   blk2->yMin < byMin0) {
          byMin0 = blk2->yMin;
          fblk3 = blk2;
        } else if (blk2->xMin > blk1->xMax &&
                   blk2->xMin < bxMin1 &&
                   blk2->yMin > blk1->yMax &&
                   blk2->yMin < byMin1) {
          bxMin1 = blk2->xMin;
          byMin1 = blk2->yMin;
          fblk4 = blk2;
        }
      }
    }

    /*  fblk4 can not overlap with fblk3 in x and with fblk2 in y
     *  fblk2 can not overlap with fblk3 in x and y
     *  fblk4 has to overlap with fblk3 in y and with fblk2 in x
     */
    if (fblk2 != NULL &&
        fblk3 != NULL &&
        fblk4 != NULL) {
      if (((fblk3->xMin <= fblk4->xMax && fblk3->xMax >= fblk4->xMin) ||
           (fblk2->yMin <= fblk4->yMax && fblk2->yMax >= fblk4->yMin) ||
           (fblk2->xMin <= fblk3->xMax && fblk2->xMax >= fblk3->xMin) ||
           (fblk2->yMin <= fblk3->yMax && fblk2->yMax >= fblk3->yMin)) ||
          !(fblk4->xMin <= fblk2->xMax && fblk4->xMax >= fblk2->xMin &&
            fblk4->yMin <= fblk3->yMax && fblk4->yMax >= fblk3->yMin)) {
        fblk2 = NULL;
        fblk3 = NULL;
        fblk4 = NULL;
      }
    }

    // if we found any then look whether they form a table
    if (fblk2 != NULL &&
        fblk3 != NULL &&
        fblk4 != NULL) {
      tableId = -1;
      correspondenceX = 0;
      correspondenceY = 0;
      deltaX = 0.0;
      deltaY = 0.0;

      if (blk1->lines && blk1->lines->words)
        deltaX = blk1->lines->words->getFontSize();
      if (fblk2->lines && fblk2->lines->words)
        deltaX = deltaX < fblk2->lines->words->getFontSize() ?
                   deltaX : fblk2->lines->words->getFontSize();
      if (fblk3->lines && fblk3->lines->words)
        deltaX = deltaX < fblk3->lines->words->getFontSize() ?
                   deltaX : fblk3->lines->words->getFontSize();
      if (fblk4->lines && fblk4->lines->words)
        deltaX = deltaX < fblk4->lines->words->getFontSize() ?
                   deltaX : fblk4->lines->words->getFontSize();

      deltaY = deltaX;

      deltaX *= minColSpacing1;
      deltaY *= maxIntraLineDelta;

      xCentre1 = (blk1->xMax + blk1->xMin) / 2.0;
      yCentre1 = (blk1->yMax + blk1->yMin) / 2.0;
      xCentre2 = (fblk2->xMax + fblk2->xMin) / 2.0;
      yCentre2 = (fblk2->yMax + fblk2->yMin) / 2.0;
      xCentre3 = (fblk3->xMax + fblk3->xMin) / 2.0;
      yCentre3 = (fblk3->yMax + fblk3->yMin) / 2.0;
      xCentre4 = (fblk4->xMax + fblk4->xMin) / 2.0;
      yCentre4 = (fblk4->yMax + fblk4->yMin) / 2.0;

      // are blocks centrally aligned in x ?
      if (fabs (xCentre1 - xCentre3) <= deltaX &&
          fabs (xCentre2 - xCentre4) <= deltaX)
        correspondenceX++;

      // are blocks centrally aligned in y ?
      if (fabs (yCentre1 - yCentre2) <= deltaY &&
          fabs (yCentre3 - yCentre4) <= deltaY)
        correspondenceY++;

      // are blocks aligned to the left ?
      if (fabs (blk1->xMin - fblk3->xMin) <= deltaX &&
          fabs (fblk2->xMin - fblk4->xMin) <= deltaX)
        correspondenceX++;

      // are blocks aligned to the right ?
      if (fabs (blk1->xMax - fblk3->xMax) <= deltaX &&
          fabs (fblk2->xMax - fblk4->xMax) <= deltaX)
        correspondenceX++;

      // are blocks aligned to the top ?
      if (fabs (blk1->yMin - fblk2->yMin) <= deltaY &&
          fabs (fblk3->yMin - fblk4->yMin) <= deltaY)
        correspondenceY++;

      // are blocks aligned to the bottom ?
      if (fabs (blk1->yMax - fblk2->yMax) <= deltaY &&
          fabs (fblk3->yMax - fblk4->yMax) <= deltaY)
        correspondenceY++;

      // are blocks aligned in x and y ?
      if (correspondenceX > 0 &&
          correspondenceY > 0) {

        // find maximal tableId
        tableId = tableId < fblk4->tableId ? fblk4->tableId : tableId;
        tableId = tableId < fblk3->tableId ? fblk3->tableId : tableId;
        tableId = tableId < fblk2->tableId ? fblk2->tableId : tableId;
        tableId = tableId < blk1->tableId ? blk1->tableId : tableId;

        // if the tableId is -1, then we found new table
        if (tableId < 0) {
          tableId = numTables;
          numTables++;
        }

        blk1->tableId = tableId;
        fblk2->tableId = tableId;
        fblk3->tableId = tableId;
        fblk4->tableId = tableId;
      }
    }
  }

  /*  set extended bounding boxes of all table entries
   *  so that they contain whole table
   *  (we need to process whole table size when comparing it
   *   with regular text blocks)
   */
  PDFRectangle *envelopes = new PDFRectangle [numTables];
  TextBlock **ending_blocks = new TextBlock* [numTables];

  for (i = 0; i < numTables; i++) {
    envelopes[i].x1 = DBL_MAX;
    envelopes[i].x2 = DBL_MIN;
    envelopes[i].y1 = DBL_MAX;
    envelopes[i].y2 = DBL_MIN;
  }

  for (blk1 = blkList; blk1; blk1 = blk1->next) {
    if (blk1->tableId >= 0) {
      if (blk1->ExMin < envelopes[blk1->tableId].x1) {
        envelopes[blk1->tableId].x1 = blk1->ExMin;
        if (!blk1->page->primaryLR)
          ending_blocks[blk1->tableId] = blk1;
      }

      if (blk1->ExMax > envelopes[blk1->tableId].x2) {
        envelopes[blk1->tableId].x2 = blk1->ExMax;
        if (blk1->page->primaryLR)
          ending_blocks[blk1->tableId] = blk1;
      }

      envelopes[blk1->tableId].y1 = blk1->EyMin < envelopes[blk1->tableId].y1 ?
                                      blk1->EyMin : envelopes[blk1->tableId].y1;
      envelopes[blk1->tableId].y2 = blk1->EyMax > envelopes[blk1->tableId].y2 ?
                                      blk1->EyMax : envelopes[blk1->tableId].y2;
    }
  }

  for (blk1 = blkList; blk1; blk1 = blk1->next) {
    if (blk1->tableId >= 0 &&
        blk1->xMin <= ending_blocks[blk1->tableId]->xMax &&
        blk1->xMax >= ending_blocks[blk1->tableId]->xMin) {
      blk1->tableEnd = gTrue;
    }
  }

  for (blk1 = blkList; blk1; blk1 = blk1->next) {
    if (blk1->tableId >= 0) {
      blk1->ExMin = envelopes[blk1->tableId].x1;
      blk1->ExMax = envelopes[blk1->tableId].x2;
      blk1->EyMin = envelopes[blk1->tableId].y1;
      blk1->EyMax = envelopes[blk1->tableId].y2;
    }
  }
  delete[] envelopes;
  delete[] ending_blocks;


  /*  set extended bounding boxes of all other blocks
   *  so that they extend in x without hitting neighbours
   */
  for (blk1 = blkList; blk1; blk1 = blk1->next) {
    if (!blk1->tableId >= 0) {
      double xMax = DBL_MAX;
      double xMin = DBL_MIN;

      for (blk2 = blkList; blk2; blk2 = blk2->next) {
        if (blk2 == blk1)
           continue;

        if (blk1->yMin <= blk2->yMax && blk1->yMax >= blk2->yMin) {
          if (blk2->xMin < xMax && blk2->xMin > blk1->xMax)
            xMax = blk2->xMin;

          if (blk2->xMax > xMin && blk2->xMax < blk1->xMin)
            xMin = blk2->xMax;
        }
      }

      for (blk2 = blkList; blk2; blk2 = blk2->next) {
        if (blk2 == blk1)
           continue;

        if (blk2->xMax > blk1->ExMax &&
            blk2->xMax <= xMax &&
            blk2->yMin >= blk1->yMax) {
          blk1->ExMax = blk2->xMax;
        }

        if (blk2->xMin < blk1->ExMin &&
            blk2->xMin >= xMin &&
            blk2->yMin >= blk1->yMax)
          blk1->ExMin = blk2->xMin;
      }
    }
  }

  i = -1;
  for (blk1 = blkList; blk1; blk1 = blk1->next) {
    i++;
    sortPos = blk1->visitDepthFirst(blkList, i, blocks, sortPos, visited);
  }
  if (visited) {
    gfree(visited);
  }

#if 0 // for debugging
  printf("*** blocks, after ro sort ***\n");
  for (i = 0; i < nBlocks; ++i) {
    blk = blocks[i];
    printf("block: rot=%d x=%.2f..%.2f y=%.2f..%.2f space=%.2f..%.2f\n",
	   blk->rot, blk->xMin, blk->xMax, blk->yMin, blk->yMax,
	   blk->priMin, blk->priMax);
    for (line = blk->lines; line; line = line->next) {
      printf("  line:\n");
      for (word0 = line->words; word0; word0 = word0->next) {
	printf("    word: x=%.2f..%.2f y=%.2f..%.2f base=%.2f fontSize=%.2f space=%d: '",
	       word0->xMin, word0->xMax, word0->yMin, word0->yMax,
	       word0->base, word0->fontSize, word0->spaceAfter);
	for (j = 0; j < word0->len; ++j) {
	  fputc(word0->text[j] & 0xff, stdout);
	}
	printf("'\n");
      }
    }
  }
  printf("\n");
  fflush(stdout);
#endif

  // build the flows
  //~ this needs to be adjusted for writing mode (vertical text)
  //~ this also needs to account for right-to-left column ordering
  flow = NULL;
  while (flows) {
    flow = flows;
    flows = flows->next;
    delete flow;
  }
  flows = lastFlow = NULL;
  // assume blocks are already in reading order,
  // and construct flows accordingly.
  for (i = 0; i < nBlocks; i++) {
    blk = blocks[i];
    blk->next = NULL;
    if (flow) {
      blk1 = blocks[i - 1];
      blkSpace = maxBlockSpacing * blk1->lines->words->fontSize;
      if (blk1->secondaryDelta(blk) <= blkSpace &&
	  blk->isBelow(blk1) &&
	  flow->blockFits(blk, blk1)) {
	flow->addBlock(blk);
	continue;
      }
    }
    flow = new TextFlow(this, blk);
    if (lastFlow) {
      lastFlow->next = flow;
    } else {
      flows = flow;
    }
    lastFlow = flow;
  }

#if 0 // for debugging
  printf("*** flows ***\n");
  for (flow = flows; flow; flow = flow->next) {
    printf("flow: x=%.2f..%.2f y=%.2f..%.2f pri:%.2f..%.2f\n",
	   flow->xMin, flow->xMax, flow->yMin, flow->yMax,
	   flow->priMin, flow->priMax);
    for (blk = flow->blocks; blk; blk = blk->next) {
      printf("  block: rot=%d x=%.2f..%.2f y=%.2f..%.2f pri=%.2f..%.2f\n",
	     blk->rot, blk->ExMin, blk->ExMax, blk->EyMin, blk->EyMax,
	     blk->priMin, blk->priMax);
      for (line = blk->lines; line; line = line->next) {
	printf("    line:\n");
	for (word0 = line->words; word0; word0 = word0->next) {
	  printf("      word: x=%.2f..%.2f y=%.2f..%.2f base=%.2f fontSize=%.2f space=%d: '",
		 word0->xMin, word0->xMax, word0->yMin, word0->yMax,
		 word0->base, word0->fontSize, word0->spaceAfter);
	  for (i = 0; i < word0->len; ++i) {
	    fputc(word0->text[i] & 0xff, stdout);
	  }
	  printf("'\n");
	}
      }
    }
  }
  printf("\n");
#endif

  if (uMap) {
    uMap->decRefCnt();
  }
}

GBool TextPage::findText(Unicode *s, int len,
			 GBool startAtTop, GBool stopAtBottom,
			 GBool startAtLast, GBool stopAtLast,
			 GBool caseSensitive, GBool backward,
			 double *xMin, double *yMin,
			 double *xMax, double *yMax) {
  TextBlock *blk;
  TextLine *line;
  Unicode *s2, *txt;
  Unicode *p;
  int txtSize, m, i, j, k;
  double xStart, yStart, xStop, yStop;
  double xMin0, yMin0, xMax0, yMax0;
  double xMin1, yMin1, xMax1, yMax1;
  GBool found;

  //~ needs to handle right-to-left text

  if (rawOrder) {
    return gFalse;
  }

  // convert the search string to uppercase
  if (!caseSensitive) {
    s2 = unicodeNormalizeNFKC(s, len, &len, NULL);
    for (i = 0; i < len; ++i) {
      s2[i] = unicodeToUpper(s2[i]);
    }
  } else {
    s2 = unicodeNormalizeNFKC(s, len, &len, NULL);
  }

  txt = NULL;
  txtSize = 0;

  xStart = yStart = xStop = yStop = 0;
  if (startAtLast && haveLastFind) {
    xStart = lastFindXMin;
    yStart = lastFindYMin;
  } else if (!startAtTop) {
    xStart = *xMin;
    yStart = *yMin;
  }
  if (stopAtLast && haveLastFind) {
    xStop = lastFindXMin;
    yStop = lastFindYMin;
  } else if (!stopAtBottom) {
    xStop = *xMax;
    yStop = *yMax;
  }

  found = gFalse;
  xMin0 = xMax0 = yMin0 = yMax0 = 0; // make gcc happy
  xMin1 = xMax1 = yMin1 = yMax1 = 0; // make gcc happy

  for (i = backward ? nBlocks - 1 : 0;
       backward ? i >= 0 : i < nBlocks;
       i += backward ? -1 : 1) {
    blk = blocks[i];

    // check: is the block above the top limit?
    if (!startAtTop && (backward ? blk->yMin > yStart : blk->yMax < yStart)) {
      continue;
    }

    // check: is the block below the bottom limit?
    if (!stopAtBottom && (backward ? blk->yMax < yStop : blk->yMin > yStop)) {
      break;
    }

    for (line = blk->lines; line; line = line->next) {

      // check: is the line above the top limit?
      if (!startAtTop &&
	  (backward ? line->yMin > yStart : line->yMin < yStart)) {
	continue;
      }

      // check: is the line below the bottom limit?
      if (!stopAtBottom &&
	  (backward ? line->yMin < yStop : line->yMin > yStop)) {
	continue;
      }

      if (!line->normalized)
	line->normalized = unicodeNormalizeNFKC(line->text, line->len, 
						&line->normalized_len, 
						&line->normalized_idx);
      // convert the line to uppercase
      m = line->normalized_len;
      if (!caseSensitive) {
	if (m > txtSize) {
	  txt = (Unicode *)greallocn(txt, m, sizeof(Unicode));
	  txtSize = m;
	}
	for (k = 0; k < m; ++k) {
	  txt[k] = unicodeToUpper(line->normalized[k]);
	  }
	  } else {
	txt = line->normalized;
	  }

      // search each position in this line
      j = backward ? m - len : 0;
      p = txt + j;
      while (backward ? j >= 0 : j <= m - len) {

	// compare the strings
	for (k = 0; k < len; ++k) {
	  if (p[k] != s2[k]) {
	    break;
	  }
	}

	// found it
	if (k == len) {
	  // where s2 matches a subsequence of a compatibility equivalence
	  // decomposition, highlight the entire glyph, since we don't know
	  // the internal layout of subglyph components
	  int normStart = line->normalized_idx[j];
	  int normAfterEnd = line->normalized_idx[j + len - 1] + 1;
	  switch (line->rot) {
	  case 0:
	    xMin1 = line->edge[normStart];
	    xMax1 = line->edge[normAfterEnd];
	    yMin1 = line->yMin;
	    yMax1 = line->yMax;
	    break;
	  case 1:
	    xMin1 = line->xMin;
	    xMax1 = line->xMax;
	    yMin1 = line->edge[normStart];
	    yMax1 = line->edge[normAfterEnd];
	    break;
	  case 2:
	    xMin1 = line->edge[normAfterEnd];
	    xMax1 = line->edge[normStart];
	    yMin1 = line->yMin;
	    yMax1 = line->yMax;
	    break;
	  case 3:
	    xMin1 = line->xMin;
	    xMax1 = line->xMax;
	    yMin1 = line->edge[normAfterEnd];
	    yMax1 = line->edge[normStart];
	    break;
	  }
	  if (backward) {
	    if ((startAtTop ||
		 yMin1 < yStart || (yMin1 == yStart && xMin1 < xStart)) &&
		(stopAtBottom ||
		 yMin1 > yStop || (yMin1 == yStop && xMin1 > xStop))) {
	      if (!found ||
		  yMin1 > yMin0 || (yMin1 == yMin0 && xMin1 > xMin0)) {
		xMin0 = xMin1;
		xMax0 = xMax1;
		yMin0 = yMin1;
		yMax0 = yMax1;
		found = gTrue;
	      }
	    }
	  } else {
	    if ((startAtTop ||
		 yMin1 > yStart || (yMin1 == yStart && xMin1 > xStart)) &&
		(stopAtBottom ||
		 yMin1 < yStop || (yMin1 == yStop && xMin1 < xStop))) {
	      if (!found ||
		  yMin1 < yMin0 || (yMin1 == yMin0 && xMin1 < xMin0)) {
		xMin0 = xMin1;
		xMax0 = xMax1;
		yMin0 = yMin1;
		yMax0 = yMax1;
		found = gTrue;
	      }
	    }
	  }
	}
	if (backward) {
	  --j;
	  --p;
	} else {
	  ++j;
	  ++p;
	}
      }
    }
    }

  gfree(s2);
  if (!caseSensitive) {
    gfree(txt);
  }

  if (found) {
    *xMin = xMin0;
    *xMax = xMax0;
    *yMin = yMin0;
    *yMax = yMax0;
    lastFindXMin = xMin0;
    lastFindYMin = yMin0;
    haveLastFind = gTrue;
    return gTrue;
  }

  return gFalse;
}

GooString *TextPage::getText(double xMin, double yMin,
			   double xMax, double yMax) {
  GooString *s;
  UnicodeMap *uMap;
  GBool isUnicode;
  TextBlock *blk;
  TextLine *line;
  TextLineFrag *frags;
  int nFrags, fragsSize;
  TextLineFrag *frag;
  char space[8], eol[16];
  int spaceLen, eolLen;
  int lastRot;
  double x, y, delta;
  int col, idx0, idx1, i, j;
  GBool multiLine, oneRot;

  s = new GooString();

  // get the output encoding
  if (!(uMap = globalParams->getTextEncoding())) {
    return s;
  }

  if (rawOrder) {
    TextWord*  word;
    char mbc[16];
    int  mbc_len;

    for (word = rawWords; word && word <= rawLastWord; word = word->next) {
      for (j = 0; j < word->getLength(); ++j) {
        double gXMin, gXMax, gYMin, gYMax;
        word->getCharBBox(j, &gXMin, &gYMin, &gXMax, &gYMax);
        if (xMin <= gXMin && gXMax <= xMax && yMin <= gYMin && gYMax <= yMax)
        {
          mbc_len = uMap->mapUnicode( *(word->getChar(j)), mbc, sizeof(mbc) );
          s->append(mbc, mbc_len);
        }
      }
    }
    return s;
  }

  isUnicode = uMap->isUnicode();
  spaceLen = uMap->mapUnicode(0x20, space, sizeof(space));
  eolLen = 0; // make gcc happy
  switch (globalParams->getTextEOL()) {
  case eolUnix:
    eolLen = uMap->mapUnicode(0x0a, eol, sizeof(eol));
    break;
  case eolDOS:
    eolLen = uMap->mapUnicode(0x0d, eol, sizeof(eol));
    eolLen += uMap->mapUnicode(0x0a, eol + eolLen, sizeof(eol) - eolLen);
    break;
  case eolMac:
    eolLen = uMap->mapUnicode(0x0d, eol, sizeof(eol));
    break;
  }

  //~ writing mode (horiz/vert)

  // collect the line fragments that are in the rectangle
  fragsSize = 256;
  frags = (TextLineFrag *)gmallocn(fragsSize, sizeof(TextLineFrag));
  nFrags = 0;
  lastRot = -1;
  oneRot = gTrue;
  for (i = 0; i < nBlocks; ++i) {
    blk = blocks[i];
    if (xMin < blk->xMax && blk->xMin < xMax &&
	yMin < blk->yMax && blk->yMin < yMax) {
      for (line = blk->lines; line; line = line->next) {
	if (xMin < line->xMax && line->xMin < xMax &&
	    yMin < line->yMax && line->yMin < yMax) {
	  idx0 = idx1 = -1;
	  switch (line->rot) {
	  case 0:
	    y = 0.5 * (line->yMin + line->yMax);
	    if (yMin < y && y < yMax) {
	      j = 0;
	      while (j < line->len) {
		if (0.5 * (line->edge[j] + line->edge[j+1]) > xMin) {
		  idx0 = j;
		  break;
		}
		++j;
	      }
	      j = line->len - 1;
	      while (j >= 0) {
		if (0.5 * (line->edge[j] + line->edge[j+1]) < xMax) {
		  idx1 = j;
		  break;
		}
		--j;
	      }
	    }
	    break;
	  case 1:
	    x = 0.5 * (line->xMin + line->xMax);
	    if (xMin < x && x < xMax) {
	      j = 0;
	      while (j < line->len) {
		if (0.5 * (line->edge[j] + line->edge[j+1]) > yMin) {
		  idx0 = j;
		  break;
		}
		++j;
	      }
	      j = line->len - 1;
	      while (j >= 0) {
		if (0.5 * (line->edge[j] + line->edge[j+1]) < yMax) {
		  idx1 = j;
		  break;
		}
		--j;
	      }
	    }
	    break;
	  case 2:
	    y = 0.5 * (line->yMin + line->yMax);
	    if (yMin < y && y < yMax) {
	      j = 0;
	      while (j < line->len) {
		if (0.5 * (line->edge[j] + line->edge[j+1]) < xMax) {
		  idx0 = j;
		  break;
		}
		++j;
	      }
	      j = line->len - 1;
	      while (j >= 0) {
		if (0.5 * (line->edge[j] + line->edge[j+1]) > xMin) {
		  idx1 = j;
		  break;
		}
		--j;
	      }
	    }
	    break;
	  case 3:
	    x = 0.5 * (line->xMin + line->xMax);
	    if (xMin < x && x < xMax) {
	      j = 0;
	      while (j < line->len) {
		if (0.5 * (line->edge[j] + line->edge[j+1]) < yMax) {
		  idx0 = j;
		  break;
		}
		++j;
	      }
	      j = line->len - 1;
	      while (j >= 0) {
		if (0.5 * (line->edge[j] + line->edge[j+1]) > yMin) {
		  idx1 = j;
		  break;
		}
		--j;
	      }
	    }
	    break;
	  }
	  if (idx0 >= 0 && idx1 >= 0) {
	    if (nFrags == fragsSize) {
	      fragsSize *= 2;
	      frags = (TextLineFrag *)
		          greallocn(frags, fragsSize, sizeof(TextLineFrag));
	    }
	    frags[nFrags].init(line, idx0, idx1 - idx0 + 1);
	    ++nFrags;
	    if (lastRot >= 0 && line->rot != lastRot) {
	      oneRot = gFalse;
	    }
	    lastRot = line->rot;
	  }
	}
      }
    }
  }

  // sort the fragments and generate the string
  if (nFrags > 0) {

    for (i = 0; i < nFrags; ++i) {
      frags[i].computeCoords(oneRot);
    }
    assignColumns(frags, nFrags, oneRot);

    // if all lines in the region have the same rotation, use it;
    // otherwise, use the page's primary rotation
    if (oneRot) {
      qsort(frags, nFrags, sizeof(TextLineFrag),
	    &TextLineFrag::cmpYXLineRot);
    } else {
      qsort(frags, nFrags, sizeof(TextLineFrag),
	    &TextLineFrag::cmpYXPrimaryRot);
    }
    i = 0;
    while (i < nFrags) {
      delta = maxIntraLineDelta * frags[i].line->words->fontSize;
      for (j = i+1;
	   j < nFrags && fabs(frags[j].base - frags[i].base) < delta;
	   ++j) ;
      qsort(frags + i, j - i, sizeof(TextLineFrag),
	    oneRot ? &TextLineFrag::cmpXYColumnLineRot
	           : &TextLineFrag::cmpXYColumnPrimaryRot);
      i = j;
    }

    col = 0;
    multiLine = gFalse;
    for (i = 0; i < nFrags; ++i) {
      frag = &frags[i];

      // insert a return
      if (frag->col < col ||
	  (i > 0 && fabs(frag->base - frags[i-1].base) >
	              maxIntraLineDelta * frags[i-1].line->words->fontSize)) {
	s->append(eol, eolLen);
	col = 0;
	multiLine = gTrue;
      }

      // column alignment
      for (; col < frag->col; ++col) {
	s->append(space, spaceLen);
      }

      // get the fragment text
      col += dumpFragment(frag->line->text + frag->start, frag->len, uMap, s);
    }

    if (multiLine) {
      s->append(eol, eolLen);
    }
  }

  gfree(frags);
  uMap->decRefCnt();

  return s;
}

class TextSelectionVisitor {
public:
  TextSelectionVisitor (TextPage *page);
  virtual ~TextSelectionVisitor () { }
  virtual void visitBlock (TextBlock *block,
			   TextLine *begin,
			   TextLine *end,
			   PDFRectangle *selection) = 0;
  virtual void visitLine (TextLine *line, 
			  TextWord *begin,
			  TextWord *end,
			  int edge_begin,
			  int edge_end,
			  PDFRectangle *selection) = 0;
  virtual void visitWord (TextWord *word, int begin, int end,
			  PDFRectangle *selection) = 0;

protected:
  TextPage *page;
};

TextSelectionVisitor::TextSelectionVisitor (TextPage *page)
  : page(page)
{
}


class TextSelectionDumper : public TextSelectionVisitor {
public:
  TextSelectionDumper(TextPage *page);
  virtual ~TextSelectionDumper();

  virtual void visitBlock (TextBlock *block, 
			   TextLine *begin,
			   TextLine *end,
			   PDFRectangle *selection) { };
  virtual void visitLine (TextLine *line,
			  TextWord *begin,
			  TextWord *end,
			  int edge_begin,
			  int edge_end,
			  PDFRectangle *selection);
  virtual void visitWord (TextWord *word, int begin, int end,
			  PDFRectangle *selection) { };

  GooString *getText(void);

private:
  TextLineFrag *frags;
  int nFrags, fragsSize;
};

TextSelectionDumper::TextSelectionDumper(TextPage *page)
    : TextSelectionVisitor(page)
{
  fragsSize = 256;
  frags = (TextLineFrag *)gmallocn(fragsSize, sizeof(TextLineFrag));
  nFrags = 0;
}

TextSelectionDumper::~TextSelectionDumper()
{
  gfree(frags);
}

void TextSelectionDumper::visitLine (TextLine *line,
				     TextWord *begin,
				     TextWord *end,
				     int edge_begin,
				     int edge_end,
				     PDFRectangle *selection)
{
  if (nFrags == fragsSize) {
    fragsSize *= 2;
    frags = (TextLineFrag *) grealloc(frags, fragsSize * sizeof(TextLineFrag));
  }

  frags[nFrags].init(line, edge_begin, edge_end - edge_begin);
  ++nFrags;

}

GooString *TextSelectionDumper::getText (void)
{
  GooString *s;
  TextLineFrag *frag;
  int i, j;
  GBool multiLine;
  UnicodeMap *uMap;
  char space[8], eol[16];
  int spaceLen, eolLen;
  GooList *strings = NULL;
  int actual_table = -1;
  int actual_line = -1;
  int last_length = 0;
  TextBlock *actual_block = NULL;

  s = new GooString();

  uMap = globalParams->getTextEncoding();

  if (uMap == NULL)
      return s;

  spaceLen = uMap->mapUnicode(0x20, space, sizeof(space));
  eolLen = uMap->mapUnicode(0x0a, eol, sizeof(eol));

  if (nFrags > 0) {
    multiLine = gFalse;
    for (i = 0; i < nFrags; ++i) {
      frag = &frags[i];

      if (actual_table >= 0 && frag->line->blk->tableId < 0) {
        for (j = 0; j < strings->getLength (); j++) {
          s->append ((GooString*) strings->get (j));
          s->append (eol, eolLen);
          delete ((GooString*) strings->get (j));
        }
        delete strings;
        strings = NULL;
        actual_table = -1;
        actual_line = -1;
        actual_block = NULL;
      }

      // a table
      if (frag->line->blk->tableId >= 0) {
        if (actual_table == -1) {
          strings = new GooList();
          actual_table = frag->line->blk->tableId;
          actual_block = frag->line->blk;
          actual_line = -1;
        }

        // the same block
        if (actual_block == frag->line->blk) {
          actual_line++;
          if (actual_line >= strings->getLength ()) {
            GooString *t = new GooString ();
            // add some spaces to have this block correctly aligned
            if (actual_line > 0)
              for (j = 0; j < ((GooString*) (strings->get (actual_line - 1)))->getLength() - last_length - 1; j++)
                t->append (space, spaceLen);
            strings->append (t);
          }
        }
        // another block
        else {
          // previous block ended its row
          if (actual_block->tableEnd) {
            for (j = 0; j < strings->getLength (); j++) {
              s->append ((GooString*) strings->get (j));
              s->append (eol, eolLen);
              delete ((GooString*) strings->get (j));
            }
            delete strings;

            strings = new GooList();
            GooString *t = new GooString ();
            strings->append (t);
          }
          actual_block = frag->line->blk;
          actual_line = 0;
        }

        page->dumpFragment(frag->line->text + frag->start, frag->len, uMap, ((GooString*) strings->get (actual_line)));
        last_length = frag->len;

        if (!frag->line->blk->tableEnd) {
          ((GooString*) strings->get (actual_line))->append (space, spaceLen);
        }
      }
      // not a table
      else {
        page->dumpFragment (frag->line->text + frag->start, frag->len, uMap, s);
        s->append (eol, eolLen);
      }
    }

    if (strings != NULL) {
      for (j = 0; j < strings->getLength (); j++) {
        s->append((GooString*) strings->get (j));
        s->append(eol, eolLen);
        delete ((GooString*) strings->get (j));
      }
      delete strings;
      strings = NULL;
      actual_table = -1;
      actual_line = -1;
      actual_block = NULL;
    }
  }

  uMap->decRefCnt();

  return s;
}

class TextSelectionSizer : public TextSelectionVisitor {
public:
  TextSelectionSizer(TextPage *page, double scale);
  ~TextSelectionSizer() { }

  virtual void visitBlock (TextBlock *block,
			   TextLine *begin,
			   TextLine *end,
			   PDFRectangle *selection) { };
  virtual void visitLine (TextLine *line, 
			  TextWord *begin,
			  TextWord *end,
			  int edge_begin,
			  int edge_end,
			  PDFRectangle *selection);
  virtual void visitWord (TextWord *word, int begin, int end,
			  PDFRectangle *selection) { };

  GooList *getRegion () { return list; }

private:
  GooList *list;
  double scale;
};

TextSelectionSizer::TextSelectionSizer(TextPage *page, double scale)
  : TextSelectionVisitor(page),
    scale(scale)
{
  list = new GooList();
}

void TextSelectionSizer::visitLine (TextLine *line, 
				    TextWord *begin,
				    TextWord *end,
				    int edge_begin,
				    int edge_end,
				    PDFRectangle *selection)
{
  PDFRectangle *rect;
  double x1, y1, x2, y2, margin;

  margin = (line->yMax - line->yMin) / 8;
  x1 = line->edge[edge_begin];
  y1 = line->yMin - margin;
  x2 = line->edge[edge_end];
  y2 = line->yMax + margin;

  rect = new PDFRectangle (floor (x1 * scale), 
			   floor (y1 * scale),
			   ceil (x2 * scale),
			   ceil (y2 * scale));
  list->append (rect);
}


class TextSelectionPainter : public TextSelectionVisitor {
public:
  TextSelectionPainter(TextPage *page,
		       double scale,
		       int rotation,
		       OutputDev *out,
		       GfxColor *box_color,
		       GfxColor *glyph_color);
  ~TextSelectionPainter();

  virtual void visitBlock (TextBlock *block,
			   TextLine *begin,
			   TextLine *end,
			   PDFRectangle *selection) { };
  virtual void visitLine (TextLine *line, 
			  TextWord *begin,
			  TextWord *end,
			  int edge_begin,
			  int edge_end,
			  PDFRectangle *selection);
  virtual void visitWord (TextWord *word, int begin, int end,
			  PDFRectangle *selection);

private:
  OutputDev *out;
  GfxColor *box_color, *glyph_color;
  GfxState *state;
};

TextSelectionPainter::TextSelectionPainter(TextPage *page,
					   double scale,
					   int rotation,
					   OutputDev *out,
					   GfxColor *box_color,
					   GfxColor *glyph_color)
  : TextSelectionVisitor(page),
    out(out),
    box_color(box_color),
    glyph_color(glyph_color)
{
  PDFRectangle box(0, 0, page->pageWidth, page->pageHeight);

  state = new GfxState(72 * scale, 72 * scale, &box, rotation, gFalse);

  out->startPage (0, state);
  out->setDefaultCTM (state->getCTM());

  state->setTextMat(1, 0, 0, -1, 0, 0);
  state->setFillColorSpace(new GfxDeviceRGBColorSpace());

}

TextSelectionPainter::~TextSelectionPainter()
{
  out->endPage ();

  delete state;
}

void TextSelectionPainter::visitLine (TextLine *line,
				      TextWord *begin,
				      TextWord *end,
				      int edge_begin,
				      int edge_end,
				      PDFRectangle *selection)
{
  double x1, y1, x2, y2, margin;
  Matrix ctm, ictm;

  state->setFillColor(box_color);
  out->updateFillColor(state);

  margin = (line->yMax - line->yMin) / 8;
  x1 = floor (line->edge[edge_begin]);
  y1 = floor (line->yMin - margin);
  x2 = ceil (line->edge[edge_end]);
  y2 = ceil (line->yMax + margin);

  state->getCTM (&ctm);
  ctm.transform(line->edge[edge_begin], line->yMin - margin, &x1, &y1);
  ctm.transform(line->edge[edge_end], line->yMax + margin, &x2, &y2);

  x1 = floor (x1);
  y1 = floor (y1);
  x2 = ceil (x2);
  y2 = ceil (y2);

  ctm.invertTo (&ictm);
  ictm.transform(x1, y1, &x1, &y1);
  ictm.transform(x2, y2, &x2, &y2);

  state->moveTo(x1, y1);
  state->lineTo(x2, y1);
  state->lineTo(x2, y2);
  state->lineTo(x1, y2);
  state->closePath();

  out->fill(state);
  state->clearPath();
}

void TextSelectionPainter::visitWord (TextWord *word, int begin, int end,
				      PDFRectangle *selection)
{
  GooString *string;
  int i;

  state->setFillColor(glyph_color);
  out->updateFillColor(state);
  word->font->gfxFont->incRefCnt();
  state->setFont(word->font->gfxFont, word->fontSize);
  out->updateFont(state);

  /* The only purpose of this string is to let the output device query
   * it's length.  Might want to change this interface later. */

  string = new GooString ((char *) word->charcode, end - begin);

  out->beginString(state, string);

  for (i = begin; i < end; i++)
    out->drawChar(state, word->edge[i], word->base, 0, 0, 0, 0,
		  word->charcode[i], 1, NULL, 0);
  
  out->endString(state);

  delete string;
}

void TextWord::visitSelection(TextSelectionVisitor *visitor,
			      PDFRectangle *selection,
			      SelectionStyle style)
{
  int i, begin, end;
  double mid;

  begin = len;
  end = 0;
  for (i = 0; i < len; i++) {
    mid = (edge[i] + edge[i + 1]) / 2;
    if (selection->x1 < mid || selection->x2 < mid)
      if (i < begin)
	begin = i;
    if (mid < selection->x1 || mid < selection->x2)
      end = i + 1;
  }

  /* Skip empty selection. */
  if (end <= begin)
    return;

  visitor->visitWord (this, begin, end, selection);
}

void TextLine::visitSelection(TextSelectionVisitor *visitor,
			      PDFRectangle *selection,
			      SelectionStyle style) {
  TextWord *p, *begin, *end, *current;
  int i, edge_begin, edge_end;
  PDFRectangle child_selection;

  begin = NULL;
  end = NULL;
  current = NULL;
  for (p = words; p != NULL; p = p->next) {
    if (blk->page->primaryLR) {
      if ((selection->x1 < p->xMax) ||
	  (selection->x2 < p->xMax))
        if (begin == NULL) 
	  begin = p;

      if (((selection->x1 > p->xMin) ||
	   (selection->x2 > p->xMin)) && (begin != NULL)) {
        end = p->next;
        current = p;
      }
    } else {
      if ((selection->x1 > p->xMin) ||
	  (selection->x2 > p->xMin))
        if (begin == NULL) 
	  begin = p;

      if (((selection->x1 < p->xMax) ||
	   (selection->x2 < p->xMax)) && (begin != NULL)) {
        end = p->next;
        current = p;
      }
    }
  }

  if (!current)
    current = begin;
  
  child_selection = *selection;
  if (style == selectionStyleWord) {
    child_selection.x1 = begin ? begin->xMin : xMin;
    if (end && end->xMax != -1) {
      child_selection.x2 = current->xMax;
    } else {
      child_selection.x2 = xMax;
    }
  }

  edge_begin = len;
  edge_end = 0;
  for (i = 0; i < len; i++) {
    double mid = (edge[i] + edge[i + 1]) /  2;
    if (child_selection.x1 < mid || child_selection.x2 < mid)
      if (i < edge_begin)
	edge_begin = i;
    if (mid < child_selection.x2 || mid < child_selection.x1)
      edge_end = i + 1;
  }

  /* Skip empty selection. */
  if (edge_end <= edge_begin)
    return;

  visitor->visitLine (this, begin, end, edge_begin, edge_end,
		      &child_selection);

  for (p = begin; p != end; p = p->next)
    p->visitSelection (visitor, &child_selection, style);
}

void TextBlock::visitSelection(TextSelectionVisitor *visitor,
			       PDFRectangle *selection,
			       SelectionStyle style) {
  PDFRectangle child_selection;
  double x[2], y[2], d, best_d[2];
  TextLine *p, *best_line[2];
  int i, count = 0, best_count[2], start, stop;
  GBool all[2];

  x[0] = selection->x1;
  y[0] = selection->y1;
  x[1] = selection->x2;
  y[1] = selection->y2;

  for (i = 0; i < 2; i++) {
    // the first/last lines are often not nearest
    // the corners, so we have to force them to be
    // selected when the selection runs outside this
    // block.
    if (page->primaryLR) {
      all[i] = x[i] >= this->xMax && y[i] >= this->yMax;
      if (x[i] <= this->xMin && y[i] <= this->yMin) {
	best_line[i] = this->lines;
	best_count[i] = 1;
      } else {
	best_line[i] = NULL;
	best_count[i] = 0;
      }
    } else {
      all[i] = x[i] <= this->xMin && y[i] >= this->yMax;
      if (x[i] >= this->xMax && y[i] <= this->yMin) {
	best_line[i] = this->lines;
	best_count[i] = 1;
      } else {
	best_line[i] = NULL;
	best_count[i] = 0;
      }
    }
    best_d[i] = 0;
  }

  // find the nearest line to the selection points
  // using the manhattan distance.
  for (p = this->lines; p; p = p->next) {
    count++;
    for (i = 0; i < 2; i++) {
      d = fmax(p->xMin - x[i], 0.0) +
	fmax(x[i] - p->xMax, 0.0) +
	fmax(p->yMin - y[i], 0.0) +
	fmax(y[i] - p->yMax, 0.0);
      if (!best_line[i] || all[i] ||
	  d < best_d[i]) {
	best_line[i] = p;
	best_count[i] = count;
	best_d[i] = d;
      }
    }
  }
  // assert: best is always set.
  if (!best_line[0] || !best_line[1]) {
    return;
  }

  // Now decide which point was first.
  if (best_count[0] < best_count[1] ||
      (best_count[0] == best_count[1] &&
       y[0] < y[1])) {
    start = 0;
    stop = 1;
  } else {
    start = 1;
    stop = 0;
  }

  visitor->visitBlock(this, best_line[start], best_line[stop], selection);

  for (p = best_line[start]; p; p = p->next) {
    if (page->primaryLR) {
      child_selection.x1 = p->xMin;
      child_selection.x2 = p->xMax;
    } else {
      child_selection.x1 = p->xMax;
      child_selection.x2 = p->xMin;
    }
    child_selection.y1 = p->yMin;
    child_selection.y2 = p->yMax;
    if (style == selectionStyleLine) {
      if (p == best_line[start]) {
	child_selection.x1 = 0;
	child_selection.y1 = 0;
      }
      if (p == best_line[stop]) {
	child_selection.x2 = page->pageWidth;
	child_selection.y2 = page->pageHeight;
      }
    } else {
      if (p == best_line[start]) {
	child_selection.x1 = fmax(p->xMin, fmin(p->xMax, x[start]));
	child_selection.y1 = fmax(p->yMin, fmin(p->yMax, y[start]));
      }
      if (p == best_line[stop]) {
	child_selection.x2 = fmax(p->xMin, fmin(p->xMax, x[stop]));
	child_selection.y2 = fmax(p->yMin, fmin(p->yMax, y[stop]));
      }
    }
    p->visitSelection(visitor, &child_selection, style);
    if (p == best_line[stop]) {
      return;
    }
  }
}

void TextPage::visitSelection(TextSelectionVisitor *visitor,
			      PDFRectangle *selection,
			      SelectionStyle style)
{
  PDFRectangle child_selection;
  double x[2], y[2], d, best_d[2];
  double xMin, yMin, xMax, yMax;
  TextFlow *flow, *best_flow[2];
  TextBlock *blk, *best_block[2];
  int i, count = 0, best_count[2], start, stop;

  if (!flows)
    return;

  x[0] = selection->x1;
  y[0] = selection->y1;
  x[1] = selection->x2;
  y[1] = selection->y2;

  xMin = pageWidth;
  yMin = pageHeight;
  xMax = 0.0;
  yMax = 0.0;

  for (i = 0; i < 2; i++) {
    best_block[i] = NULL;
    best_flow[i] = NULL;
    best_count[i] = 0;
    best_d[i] = 0;
  }

  // find the nearest blocks to the selection points
  // using the manhattan distance.
  for (flow = flows; flow; flow = flow->next) {
    for (blk = flow->blocks; blk; blk = blk->next) {
      count++;
      // the first/last blocks in reading order are
      // often not the closest to the page corners;
      // track the corners, force those blocks to
      // be selected if the selection runs across
      // multiple pages.
      xMin = fmin(xMin, blk->xMin);
      yMin = fmin(yMin, blk->yMin);
      xMax = fmax(xMax, blk->xMax);
      yMax = fmax(yMax, blk->yMax);
      for (i = 0; i < 2; i++) {
	d = fmax(blk->xMin - x[i], 0.0) +
	  fmax(x[i] - blk->xMax, 0.0) +
	  fmax(blk->yMin - y[i], 0.0) +
	  fmax(y[i] - blk->yMax, 0.0);
	if (!best_block[i] ||
	    d < best_d[i] ||
	    (!blk->next && !flow->next &&
	     x[i] > xMax && y[i] > yMax)) {
	  best_block[i] = blk;
	  best_flow[i] = flow;
	  best_count[i] = count;
	  best_d[i] = d;
	}
      }
    }
  }
  for (i = 0; i < 2; i++) {
    if (primaryLR) {
      if (x[i] < xMin && y[i] < yMin) {
	best_block[i] = flows->blocks;
	best_flow[i] = flows;
	best_count[i] = 1;
      }
    } else {
      if (x[i] > xMax && y[i] < yMin) {
	best_block[i] = flows->blocks;
	best_flow[i] = flows;
	best_count[i] = 1;
      }
    }
  }
  // assert: best is always set.
  if (!best_block[0] || !best_block[1]) {
    return;
  }

  // Now decide which point was first.
  if (best_count[0] < best_count[1] ||
      (best_count[0] == best_count[1] && y[0] < y[1])) {
    start = 0;
    stop = 1;
  } else {
    start = 1;
    stop = 0;
  }

  for (flow = best_flow[start]; flow; flow = flow->next) {
    if (flow == best_flow[start]) {
      blk = best_block[start];
    } else {
      blk = flow->blocks;
    }
    for (; blk; blk = blk->next) {
      if (primaryLR) {
	child_selection.x1 = blk->xMin;
	child_selection.x2 = blk->xMax;
      } else {
	child_selection.x1 = blk->xMax;
	child_selection.x2 = blk->xMin;
      }
      child_selection.y1 = blk->yMin;
      child_selection.y2 = blk->yMax;
      if (blk == best_block[start]) {
	child_selection.x1 = fmax(blk->xMin, fmin(blk->xMax, x[start]));
	child_selection.y1 = fmax(blk->yMin, fmin(blk->yMax, y[start]));
      }
      if (blk == best_block[stop]) {
	child_selection.x2 = fmax(blk->xMin, fmin(blk->xMax, x[stop]));
	child_selection.y2 = fmax(blk->yMin, fmin(blk->yMax, y[stop]));
	blk->visitSelection(visitor, &child_selection, style);
	return;
      }
      blk->visitSelection(visitor, &child_selection, style);
    }
  }
}

void TextPage::drawSelection(OutputDev *out,
			     double scale,
			     int rotation,
			     PDFRectangle *selection,
			     SelectionStyle style,
			     GfxColor *glyph_color, GfxColor *box_color)
{
  TextSelectionPainter painter(this, scale, rotation, 
			       out, box_color, glyph_color);

  visitSelection(&painter, selection, style);
}

GooList *TextPage::getSelectionRegion(PDFRectangle *selection,
				      SelectionStyle style,
				      double scale) {
  TextSelectionSizer sizer(this, scale);

  visitSelection(&sizer, selection, style);

  return sizer.getRegion();
}

GooString *TextPage::getSelectionText(PDFRectangle *selection,
				      SelectionStyle style)
{
  TextSelectionDumper dumper(this);

  visitSelection(&dumper, selection, style);

  return dumper.getText();
}

GBool TextPage::findCharRange(int pos, int length,
			      double *xMin, double *yMin,
			      double *xMax, double *yMax) {
  TextBlock *blk;
  TextLine *line;
  TextWord *word;
  double xMin0, xMax0, yMin0, yMax0;
  double xMin1, xMax1, yMin1, yMax1;
  GBool first;
  int i, j0, j1;

  if (rawOrder) {
    return gFalse;
  }

  //~ this doesn't correctly handle:
  //~ - ranges split across multiple lines (the highlighted region
  //~   is the bounding box of all the parts of the range)
  //~ - cases where characters don't convert one-to-one into Unicode
  first = gTrue;
  xMin0 = xMax0 = yMin0 = yMax0 = 0; // make gcc happy
  xMin1 = xMax1 = yMin1 = yMax1 = 0; // make gcc happy
  for (i = 0; i < nBlocks; ++i) {
    blk = blocks[i];
    for (line = blk->lines; line; line = line->next) {
      for (word = line->words; word; word = word->next) {
	if (pos < word->charPos + word->charLen &&
	    word->charPos < pos + length) {
	  j0 = pos - word->charPos;
	  if (j0 < 0) {
	    j0 = 0;
	  }
	  j1 = pos + length - 1 - word->charPos;
	  if (j1 >= word->len) {
	    j1 = word->len - 1;
	  }
	  switch (line->rot) {
	  case 0:
	    xMin1 = word->edge[j0];
	    xMax1 = word->edge[j1 + 1];
	    yMin1 = word->yMin;
	    yMax1 = word->yMax;
	    break;
	  case 1:
	    xMin1 = word->xMin;
	    xMax1 = word->xMax;
	    yMin1 = word->edge[j0];
	    yMax1 = word->edge[j1 + 1];
	    break;
	  case 2:
	    xMin1 = word->edge[j1 + 1];
	    xMax1 = word->edge[j0];
	    yMin1 = word->yMin;
	    yMax1 = word->yMax;
	    break;
	  case 3:
	    xMin1 = word->xMin;
	    xMax1 = word->xMax;
	    yMin1 = word->edge[j1 + 1];
	    yMax1 = word->edge[j0];
	    break;
	  }
	  if (first || xMin1 < xMin0) {
	    xMin0 = xMin1;
	  }
	  if (first || xMax1 > xMax0) {
	    xMax0 = xMax1;
	  }
	  if (first || yMin1 < yMin0) {
	    yMin0 = yMin1;
	  }
	  if (first || yMax1 > yMax0) {
	    yMax0 = yMax1;
	  }
	  first = gFalse;
	}
      }
    }
  }
  if (!first) {
    *xMin = xMin0;
    *xMax = xMax0;
    *yMin = yMin0;
    *yMax = yMax0;
    return gTrue;
  }
  return gFalse;
}

void TextPage::dump(void *outputStream, TextOutputFunc outputFunc,
		    GBool physLayout) {
  UnicodeMap *uMap;
  TextFlow *flow;
  TextBlock *blk;
  TextLine *line;
  TextLineFrag *frags;
  TextWord *word;
  int nFrags, fragsSize;
  TextLineFrag *frag;
  char space[8], eol[16], eop[8];
  int spaceLen, eolLen, eopLen;
  GBool pageBreaks;
  GooString *s;
  double delta;
  int col, i, j, d, n;

  // get the output encoding
  if (!(uMap = globalParams->getTextEncoding())) {
    return;
  }
  spaceLen = uMap->mapUnicode(0x20, space, sizeof(space));
  eolLen = 0; // make gcc happy
  switch (globalParams->getTextEOL()) {
  case eolUnix:
    eolLen = uMap->mapUnicode(0x0a, eol, sizeof(eol));
    break;
  case eolDOS:
    eolLen = uMap->mapUnicode(0x0d, eol, sizeof(eol));
    eolLen += uMap->mapUnicode(0x0a, eol + eolLen, sizeof(eol) - eolLen);
    break;
  case eolMac:
    eolLen = uMap->mapUnicode(0x0d, eol, sizeof(eol));
    break;
  }
  eopLen = uMap->mapUnicode(0x0c, eop, sizeof(eop));
  pageBreaks = globalParams->getTextPageBreaks();

  //~ writing mode (horiz/vert)

  // output the page in raw (content stream) order
  if (rawOrder) {

    for (word = rawWords; word; word = word->next) {
      s = new GooString();
      dumpFragment(word->text, word->len, uMap, s);
      (*outputFunc)(outputStream, s->getCString(), s->getLength());
      delete s;
      if (word->next &&
	  fabs(word->next->base - word->base) <
	    maxIntraLineDelta * word->fontSize) {
	if (word->next->xMin > word->xMax + minWordSpacing * word->fontSize) {
	  (*outputFunc)(outputStream, space, spaceLen);
	}
      } else {
	(*outputFunc)(outputStream, eol, eolLen);
      }
    }

  // output the page, maintaining the original physical layout
  } else if (physLayout) {

    // collect the line fragments for the page and sort them
    fragsSize = 256;
    frags = (TextLineFrag *)gmallocn(fragsSize, sizeof(TextLineFrag));
    nFrags = 0;
    for (i = 0; i < nBlocks; ++i) {
      blk = blocks[i];
      for (line = blk->lines; line; line = line->next) {
	if (nFrags == fragsSize) {
	  fragsSize *= 2;
	  frags = (TextLineFrag *)greallocn(frags,
					    fragsSize, sizeof(TextLineFrag));
	}
	frags[nFrags].init(line, 0, line->len);
	frags[nFrags].computeCoords(gTrue);
	++nFrags;
      }
    }
    qsort(frags, nFrags, sizeof(TextLineFrag), &TextLineFrag::cmpYXPrimaryRot);
    i = 0;
    while (i < nFrags) {
      delta = maxIntraLineDelta * frags[i].line->words->fontSize;
      for (j = i+1;
	   j < nFrags && fabs(frags[j].base - frags[i].base) < delta;
	   ++j) ;
      qsort(frags + i, j - i, sizeof(TextLineFrag),
	    &TextLineFrag::cmpXYColumnPrimaryRot);
      i = j;
    }

#if 0 // for debugging
    printf("*** line fragments ***\n");
    for (i = 0; i < nFrags; ++i) {
      frag = &frags[i];
      printf("frag: x=%.2f..%.2f y=%.2f..%.2f base=%.2f '",
	     frag->xMin, frag->xMax, frag->yMin, frag->yMax, frag->base);
      for (n = 0; n < frag->len; ++n) {
	fputc(frag->line->text[frag->start + n] & 0xff, stdout);
      }
      printf("'\n");
    }
    printf("\n");
#endif

    // generate output
    col = 0;
    for (i = 0; i < nFrags; ++i) {
      frag = &frags[i];

      // column alignment
      for (; col < frag->col; ++col) {
	(*outputFunc)(outputStream, space, spaceLen);
      }

      // print the line
      s = new GooString();
      col += dumpFragment(frag->line->text + frag->start, frag->len, uMap, s);
      (*outputFunc)(outputStream, s->getCString(), s->getLength());
      delete s;

      // print one or more returns if necessary
      if (i == nFrags - 1 ||
	  frags[i+1].col < col ||
	  fabs(frags[i+1].base - frag->base) >
	    maxIntraLineDelta * frag->line->words->fontSize) {
	if (i < nFrags - 1) {
	  d = (int)((frags[i+1].base - frag->base) /
		    frag->line->words->fontSize);
	  if (d < 1) {
	    d = 1;
	  } else if (d > 5) {
	    d = 5;
	  }
	} else {
	  d = 1;
	}
	for (; d > 0; --d) {
	  (*outputFunc)(outputStream, eol, eolLen);
	}
	col = 0;
      }
    }

    gfree(frags);

  // output the page, "undoing" the layout
  } else {
    for (flow = flows; flow; flow = flow->next) {
      for (blk = flow->blocks; blk; blk = blk->next) {
	for (line = blk->lines; line; line = line->next) {
	  n = line->len;
	  if (line->hyphenated && (line->next || blk->next)) {
	    --n;
	  }
	  s = new GooString();
	  dumpFragment(line->text, n, uMap, s);
	  (*outputFunc)(outputStream, s->getCString(), s->getLength());
	  delete s;
	  // output a newline when a hyphen is not suppressed
	  if (n == line->len) {
	    (*outputFunc)(outputStream, eol, eolLen);
	  }
	}
      }
      (*outputFunc)(outputStream, eol, eolLen);
    }
  }

  // end of page
  if (pageBreaks) {
    (*outputFunc)(outputStream, eop, eopLen);
  }

  uMap->decRefCnt();
}

void TextPage::assignColumns(TextLineFrag *frags, int nFrags, GBool oneRot) {
  TextLineFrag *frag0, *frag1;
  int rot, col1, col2, i, j, k;

  // all text in the region has the same rotation -- recompute the
  // column numbers based only on the text in the region
  if (oneRot) {
    qsort(frags, nFrags, sizeof(TextLineFrag), &TextLineFrag::cmpXYLineRot);
    rot = frags[0].line->rot;
    for (i = 0; i < nFrags; ++i) {
      frag0 = &frags[i];
      col1 = 0;
      for (j = 0; j < i; ++j) {
	frag1 = &frags[j];
	col2 = 0; // make gcc happy
	switch (rot) {
	case 0:
	  if (frag0->xMin >= frag1->xMax) {
	    col2 = frag1->col + (frag1->line->col[frag1->start + frag1->len] -
				 frag1->line->col[frag1->start]) + 1;
	  } else {
	    for (k = frag1->start;
		 k < frag1->start + frag1->len &&
		   frag0->xMin >= 0.5 * (frag1->line->edge[k] +
					 frag1->line->edge[k+1]);
		 ++k) ;
	    col2 = frag1->col +
	           frag1->line->col[k] - frag1->line->col[frag1->start];
	  }
	  break;
	case 1:
	  if (frag0->yMin >= frag1->yMax) {
	    col2 = frag1->col + (frag1->line->col[frag1->start + frag1->len] -
				 frag1->line->col[frag1->start]) + 1;
	  } else {
	    for (k = frag1->start;
		 k < frag1->start + frag1->len &&
		   frag0->yMin >= 0.5 * (frag1->line->edge[k] +
					 frag1->line->edge[k+1]);
		 ++k) ;
	    col2 = frag1->col +
	           frag1->line->col[k] - frag1->line->col[frag1->start];
	  }
	  break;
	case 2:
	  if (frag0->xMax <= frag1->xMin) {
	    col2 = frag1->col + (frag1->line->col[frag1->start + frag1->len] -
				 frag1->line->col[frag1->start]) + 1;
	  } else {
	    for (k = frag1->start;
		 k < frag1->start + frag1->len &&
		   frag0->xMax <= 0.5 * (frag1->line->edge[k] +
					 frag1->line->edge[k+1]);
		 ++k) ;
	    col2 = frag1->col +
	           frag1->line->col[k] - frag1->line->col[frag1->start];
	  }
	  break;
	case 3:
	  if (frag0->yMax <= frag1->yMin) {
	    col2 = frag1->col + (frag1->line->col[frag1->start + frag1->len] -
				 frag1->line->col[frag1->start]) + 1;
	  } else {
	    for (k = frag1->start;
		 k < frag1->start + frag1->len &&
		   frag0->yMax <= 0.5 * (frag1->line->edge[k] +
					 frag1->line->edge[k+1]);
		 ++k) ;
	    col2 = frag1->col +
	           frag1->line->col[k] - frag1->line->col[frag1->start];
	  }
	  break;
	}
	if (col2 > col1) {
	  col1 = col2;
	}
      }
      frag0->col = col1;
    }

  // the region includes text at different rotations -- use the
  // globally assigned column numbers, offset by the minimum column
  // number (i.e., shift everything over to column 0)
  } else {
    col1 = frags[0].col;
    for (i = 1; i < nFrags; ++i) {
      if (frags[i].col < col1) {
	col1 = frags[i].col;
      }
    }
    for (i = 0; i < nFrags; ++i) {
      frags[i].col -= col1;
    }
  }
}

int TextPage::dumpFragment(Unicode *text, int len, UnicodeMap *uMap,
			   GooString *s) {
  char lre[8], rle[8], popdf[8], buf[8];
  int lreLen, rleLen, popdfLen, n;
  int nCols, i, j, k;

  nCols = 0;

  if (uMap->isUnicode()) {

    lreLen = uMap->mapUnicode(0x202a, lre, sizeof(lre));
    rleLen = uMap->mapUnicode(0x202b, rle, sizeof(rle));
    popdfLen = uMap->mapUnicode(0x202c, popdf, sizeof(popdf));

    if (primaryLR) {

      i = 0;
      while (i < len) {
	// output a left-to-right section
	for (j = i; j < len && !unicodeTypeR(text[j]); ++j) ;
	for (k = i; k < j; ++k) {
	  n = uMap->mapUnicode(text[k], buf, sizeof(buf));
	  s->append(buf, n);
	  ++nCols;
	}
	i = j;
	// output a right-to-left section
	for (j = i; j < len && !unicodeTypeL(text[j]); ++j) ;
	if (j > i) {
	  s->append(rle, rleLen);
	  for (k = j - 1; k >= i; --k) {
	    n = uMap->mapUnicode(text[k], buf, sizeof(buf));
	    s->append(buf, n);
	    ++nCols;
	  }
	  s->append(popdf, popdfLen);
	  i = j;
	}
      }

    } else {

      s->append(rle, rleLen);
      i = len - 1;
      while (i >= 0) {
	// output a right-to-left section
	for (j = i; j >= 0 && !unicodeTypeL(text[j]); --j) ;
	for (k = i; k > j; --k) {
	  n = uMap->mapUnicode(text[k], buf, sizeof(buf));
	  s->append(buf, n);
	  ++nCols;
	}
	i = j;
	// output a left-to-right section
	for (j = i; j >= 0 && !unicodeTypeR(text[j]); --j) ;
	if (j < i) {
	  s->append(lre, lreLen);
	  for (k = j + 1; k <= i; ++k) {
	    n = uMap->mapUnicode(text[k], buf, sizeof(buf));
	    s->append(buf, n);
	    ++nCols;
	  }
	  s->append(popdf, popdfLen);
	  i = j;
	}
      }
      s->append(popdf, popdfLen);

    }

  } else {
    for (i = 0; i < len; ++i) {
      n = uMap->mapUnicode(text[i], buf, sizeof(buf));
      s->append(buf, n);
      nCols += n;
    }
  }

  return nCols;
}

#if TEXTOUT_WORD_LIST
TextWordList *TextPage::makeWordList(GBool physLayout) {
  return new TextWordList(this, physLayout);
}
#endif

//------------------------------------------------------------------------
// ActualText
//------------------------------------------------------------------------
ActualText::ActualText(TextPage *out) {
  out->incRefCnt();
  text = out;
  actualText = NULL;
  actualTextBMCLevel = 0;
}

ActualText::~ActualText() {
  if (actualText)
    delete actualText;
  text->decRefCnt();
}

void ActualText::addChar(GfxState *state, double x, double y,
			 double dx, double dy,
			 CharCode c, int nBytes, Unicode *u, int uLen) {
  if (actualTextBMCLevel == 0) {
    text->addChar(state, x, y, dx, dy, c, nBytes, u, uLen);
  } else {
    // Inside ActualText span.
    if (newActualTextSpan) {
      actualText_x = x;
      actualText_y = y;
      actualText_dx = dx;
      actualText_dy = dy;
      newActualTextSpan = gFalse;
    } else {
      if (x < actualText_x)
	actualText_x = x;
      if (y < actualText_y)
	actualText_y = y;
      if (x + dx > actualText_x + actualText_dx)
	actualText_dx = x + dx - actualText_x;
      if (y + dy > actualText_y + actualText_dy)
	actualText_dy = y + dy - actualText_y;
    }
  }
}

void ActualText::beginMC(Dict *properties) {
  if (actualTextBMCLevel > 0) {
    // Already inside a ActualText span.
    actualTextBMCLevel++;
    return;
  }

  Object obj;
  if (properties && properties->lookup("ActualText", &obj)) {
    if (obj.isString()) {
      actualText = obj.getString();
      actualTextBMCLevel = 1;
      newActualTextSpan = gTrue;
    }
  }
}

void ActualText::endMC(GfxState *state) {
  char *uniString = NULL;
  Unicode *uni;
  int length, i;

  if (actualTextBMCLevel > 0) {
    actualTextBMCLevel--;
    if (actualTextBMCLevel == 0) {
      // ActualText span closed. Output the span text and the
      // extents of all the glyphs inside the span

      if (newActualTextSpan) {
	// No content inside span.
	actualText_x = state->getCurX();
	actualText_y = state->getCurY();
	actualText_dx = 0;
	actualText_dy = 0;
      }

      if (!actualText->hasUnicodeMarker()) {
	if (actualText->getLength() > 0) {
	  //non-unicode string -- assume pdfDocEncoding and
	  //try to convert to UTF16BE
	  uniString = pdfDocEncodingToUTF16(actualText, &length);
	} else {
	  length = 0;
	}
      } else {
	uniString = actualText->getCString();
	length = actualText->getLength();
      }

      if (length < 3)
	length = 0;
      else
	length = length/2 - 1;
      uni = new Unicode[length];
      for (i = 0 ; i < length; i++)
	uni[i] = ((uniString[2 + i*2] & 0xff)<<8)|(uniString[3 + i*2] & 0xff);

      text->addChar(state,
		    actualText_x, actualText_y,
		    actualText_dx, actualText_dy,
		    0, 1, uni, length);

      delete [] uni;
      if (!actualText->hasUnicodeMarker())
	delete [] uniString;
      delete actualText;
      actualText = NULL;
    }
  }
}

//------------------------------------------------------------------------
// TextOutputDev
//------------------------------------------------------------------------

static void TextOutputDev_outputToFile(void *stream, char *text, int len) {
  fwrite(text, 1, len, (FILE *)stream);
}

TextOutputDev::TextOutputDev(char *fileName, GBool physLayoutA,
			     GBool rawOrderA, GBool append) {
  text = NULL;
  physLayout = physLayoutA;
  rawOrder = rawOrderA;
  doHTML = gFalse;
  ok = gTrue;

  // open file
  needClose = gFalse;
  if (fileName) {
    if (!strcmp(fileName, "-")) {
      outputStream = stdout;
#ifdef _WIN32
      // keep DOS from munging the end-of-line characters
      setmode(fileno(stdout), O_BINARY);
#endif
    } else if ((outputStream = fopen(fileName, append ? "ab" : "wb"))) {
      needClose = gTrue;
    } else {
      error(-1, "Couldn't open text file '%s'", fileName);
      ok = gFalse;
      actualText = NULL;
      return;
    }
    outputFunc = &TextOutputDev_outputToFile;
  } else {
    outputStream = NULL;
  }

  // set up text object
  text = new TextPage(rawOrderA);
  actualText = new ActualText(text);
}

TextOutputDev::TextOutputDev(TextOutputFunc func, void *stream,
			     GBool physLayoutA, GBool rawOrderA) {
  outputFunc = func;
  outputStream = stream;
  needClose = gFalse;
  physLayout = physLayoutA;
  rawOrder = rawOrderA;
  doHTML = gFalse;
  text = new TextPage(rawOrderA);
  actualText = new ActualText(text);
  ok = gTrue;
}

TextOutputDev::~TextOutputDev() {
  if (needClose) {
#ifdef MACOS
    ICS_MapRefNumAndAssign((short)((FILE *)outputStream)->handle);
#endif
    fclose((FILE *)outputStream);
  }
  if (text) {
    text->decRefCnt();
  }
  delete actualText;
}

void TextOutputDev::startPage(int pageNum, GfxState *state) {
  text->startPage(state);
}

void TextOutputDev::endPage() {
  text->endPage();
  text->coalesce(physLayout, doHTML);
  if (outputStream) {
    text->dump(outputStream, outputFunc, physLayout);
  }
}

void TextOutputDev::updateFont(GfxState *state) {
  text->updateFont(state);
}

void TextOutputDev::beginString(GfxState *state, GooString *s) {
}

void TextOutputDev::endString(GfxState *state) {
}

void TextOutputDev::drawChar(GfxState *state, double x, double y,
			     double dx, double dy,
			     double originX, double originY,
			     CharCode c, int nBytes, Unicode *u, int uLen) {
  actualText->addChar(state, x, y, dx, dy, c, nBytes, u, uLen);
}

void TextOutputDev::beginMarkedContent(char *name, Dict *properties)
{
  actualText->beginMC(properties);
}

void TextOutputDev::endMarkedContent(GfxState *state)
{
  actualText->endMC(state);
}

void TextOutputDev::stroke(GfxState *state) {
  GfxPath *path;
  GfxSubpath *subpath;
  double x[2], y[2];

  if (!doHTML) {
    return;
  }
  path = state->getPath();
  if (path->getNumSubpaths() != 1) {
    return;
  }
  subpath = path->getSubpath(0);
  if (subpath->getNumPoints() != 2) {
    return;
  }
  state->transform(subpath->getX(0), subpath->getY(0), &x[0], &y[0]);
  state->transform(subpath->getX(1), subpath->getY(1), &x[1], &y[1]);

  // look for a vertical or horizontal line
  if (x[0] == x[1] || y[0] == y[1]) {
    text->addUnderline(x[0], y[0], x[1], y[1]);
  }
}

void TextOutputDev::fill(GfxState *state) {
  GfxPath *path;
  GfxSubpath *subpath;
  double x[5], y[5];
  double rx0, ry0, rx1, ry1, t;
  int i;

  if (!doHTML) {
    return;
  }
  path = state->getPath();
  if (path->getNumSubpaths() != 1) {
    return;
  }
  subpath = path->getSubpath(0);
  if (subpath->getNumPoints() != 5) {
    return;
  }
  for (i = 0; i < 5; ++i) {
    if (subpath->getCurve(i)) {
      return;
    }
    state->transform(subpath->getX(i), subpath->getY(i), &x[i], &y[i]);
  }

  // look for a rectangle
  if (x[0] == x[1] && y[1] == y[2] && x[2] == x[3] && y[3] == y[4] &&
      x[0] == x[4] && y[0] == y[4]) {
    rx0 = x[0];
    ry0 = y[0];
    rx1 = x[2];
    ry1 = y[1];
  } else if (y[0] == y[1] && x[1] == x[2] && y[2] == y[3] && x[3] == x[4] &&
	     x[0] == x[4] && y[0] == y[4]) {
    rx0 = x[0];
    ry0 = y[0];
    rx1 = x[1];
    ry1 = y[2];
  } else {
    return;
  }
  if (rx1 < rx0) {
    t = rx0;
    rx0 = rx1;
    rx1 = t;
  }
  if (ry1 < ry0) {
    t = ry0;
    ry0 = ry1;
    ry1 = t;
  }

  // skinny horizontal rectangle
  if (ry1 - ry0 < rx1 - rx0) {
    if (ry1 - ry0 < maxUnderlineWidth) {
      ry0 = 0.5 * (ry0 + ry1);
      text->addUnderline(rx0, ry0, rx1, ry0);
    }

  // skinny vertical rectangle
  } else {
    if (rx1 - rx0 < maxUnderlineWidth) {
      rx0 = 0.5 * (rx0 + rx1);
      text->addUnderline(rx0, ry0, rx0, ry1);
    }
  }
}

void TextOutputDev::eoFill(GfxState *state) {
  if (!doHTML) {
    return;
  }
  fill(state);
}

void TextOutputDev::processLink(Link *link, Catalog * /*catalog*/) {
  double x1, y1, x2, y2;
  int xMin, yMin, xMax, yMax, x, y;

  if (!doHTML) {
    return;
  }
  link->getRect(&x1, &y1, &x2, &y2);
  cvtUserToDev(x1, y1, &x, &y);
  xMin = xMax = x;
  yMin = yMax = y;
  cvtUserToDev(x1, y2, &x, &y);
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
  cvtUserToDev(x2, y1, &x, &y);
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
  cvtUserToDev(x2, y2, &x, &y);
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
  text->addLink(xMin, yMin, xMax, yMax, link);
}

GBool TextOutputDev::findText(Unicode *s, int len,
			      GBool startAtTop, GBool stopAtBottom,
			      GBool startAtLast, GBool stopAtLast,
			      GBool caseSensitive, GBool backward,
			      double *xMin, double *yMin,
			      double *xMax, double *yMax) {
  return text->findText(s, len, startAtTop, stopAtBottom,
			startAtLast, stopAtLast, caseSensitive, backward,
			xMin, yMin, xMax, yMax);
}

GooString *TextOutputDev::getText(double xMin, double yMin,
				double xMax, double yMax) {
  return text->getText(xMin, yMin, xMax, yMax);
}

void TextOutputDev::drawSelection(OutputDev *out,
				  double scale,
				  int rotation,
				  PDFRectangle *selection,
				  SelectionStyle style,
				  GfxColor *glyph_color, GfxColor *box_color) {
  text->drawSelection(out, scale, rotation, selection, style, glyph_color, box_color);
}

GooList *TextOutputDev::getSelectionRegion(PDFRectangle *selection,
					   SelectionStyle style,
					   double scale) {
  return text->getSelectionRegion(selection, style, scale);
}

GooString *TextOutputDev::getSelectionText(PDFRectangle *selection,
					   SelectionStyle style)
{
  return text->getSelectionText(selection, style);
}

GBool TextOutputDev::findCharRange(int pos, int length,
				   double *xMin, double *yMin,
				   double *xMax, double *yMax) {
  return text->findCharRange(pos, length, xMin, yMin, xMax, yMax);
}

#if TEXTOUT_WORD_LIST
TextWordList *TextOutputDev::makeWordList() {
  return text->makeWordList(physLayout);
}
#endif

TextPage *TextOutputDev::takeText() {
  TextPage *ret;

  ret = text;
  text = new TextPage(rawOrder);
  return ret;
}
