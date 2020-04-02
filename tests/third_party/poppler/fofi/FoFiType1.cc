//========================================================================
//
// FoFiType1.cc
//
// Copyright 1999-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2008, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2010 Jakub Wilk <ubanus@users.sf.net>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <string.h>
#include "goo/gmem.h"
#include "goo/GooLikely.h"
#include "FoFiEncodings.h"
#include "FoFiType1.h"
#include "poppler/Error.h"

//------------------------------------------------------------------------
// FoFiType1
//------------------------------------------------------------------------

FoFiType1 *FoFiType1::make(char *fileA, int lenA) {
  return new FoFiType1(fileA, lenA, gFalse);
}

FoFiType1 *FoFiType1::load(char *fileName) {
  char *fileA;
  int lenA;

  if (!(fileA = FoFiBase::readFile(fileName, &lenA))) {
    return NULL;
  }
  return new FoFiType1(fileA, lenA, gTrue);
}

FoFiType1::FoFiType1(char *fileA, int lenA, GBool freeFileDataA):
  FoFiBase(fileA, lenA, freeFileDataA)
{
  name = NULL;
  encoding = NULL;
  parsed = gFalse;
}

FoFiType1::~FoFiType1() {
  int i;

  if (name) {
    gfree(name);
  }
  if (encoding && encoding != fofiType1StandardEncoding) {
    for (i = 0; i < 256; ++i) {
      gfree(encoding[i]);
    }
    gfree(encoding);
  }
}

char *FoFiType1::getName() {
  if (!parsed) {
    parse();
  }
  return name;
}

char **FoFiType1::getEncoding() {
  if (!parsed) {
    parse();
  }
  return encoding;
}

void FoFiType1::writeEncoded(char **newEncoding,
			     FoFiOutputFunc outputFunc, void *outputStream) {
  char buf[512];
  char *line, *line2, *p;
  int i;

  // copy everything up to the encoding
  for (line = (char *)file;
       line && strncmp(line, "/Encoding", 9);
       line = getNextLine(line)) ;
  if (!line) {
    // no encoding - just copy the whole font file
    (*outputFunc)(outputStream, (char *)file, len);
    return;
  }
  (*outputFunc)(outputStream, (char *)file, line - (char *)file);

  // write the new encoding
  (*outputFunc)(outputStream, "/Encoding 256 array\n", 20);
  (*outputFunc)(outputStream,
		"0 1 255 {1 index exch /.notdef put} for\n", 40);
  for (i = 0; i < 256; ++i) {
    if (newEncoding[i]) {
      sprintf(buf, "dup %d /%s put\n", i, newEncoding[i]);
      (*outputFunc)(outputStream, buf, strlen(buf));
    }
  }
  (*outputFunc)(outputStream, "readonly def\n", 13);
  
  // find the end of the encoding data
  //~ this ought to parse PostScript tokens
  if (!strncmp(line, "/Encoding StandardEncoding def", 30)) {
    line = getNextLine(line);
  } else {
    // skip "/Encoding" + one whitespace char,
    // then look for 'def' preceded by PostScript whitespace
    p = line + 10;
    line = NULL;
    for (; p < (char *)file + len; ++p) {
      if ((*p == ' ' || *p == '\t' || *p == '\x0a' ||
	   *p == '\x0d' || *p == '\x0c' || *p == '\0') &&
	  p + 4 <= (char *)file + len &&
	  !strncmp(p + 1, "def", 3)) {
	line = p + 4;
	break;
      }
    }
  }

  // some fonts have two /Encoding entries in their dictionary, so we
  // check for a second one here
  if (line) {
    for (line2 = line, i = 0;
	 i < 20 && line2 && strncmp(line2, "/Encoding", 9);
	 line2 = getNextLine(line2), ++i) ;
    if (i < 20 && line2) {
      (*outputFunc)(outputStream, line, line2 - line);
      if (!strncmp(line2, "/Encoding StandardEncoding def", 30)) {
	line = getNextLine(line2);
      } else {
	// skip "/Encoding" + one whitespace char,
	// then look for 'def' preceded by PostScript whitespace
	p = line2 + 10;
	line = NULL;
	for (; p < (char *)file + len; ++p) {
	  if ((*p == ' ' || *p == '\t' || *p == '\x0a' ||
	       *p == '\x0d' || *p == '\x0c' || *p == '\0') &&
	      p + 4 <= (char *)file + len &&
	      !strncmp(p + 1, "def", 3)) {
	    line = p + 4;
	    break;
	  }
	}
      }
    }

    // copy everything after the encoding
    if (line) {
      (*outputFunc)(outputStream, line, ((char *)file + len) - line);
    }
  }
}

char *FoFiType1::getNextLine(char *line) {
  while (line < (char *)file + len && *line != '\x0a' && *line != '\x0d') {
    ++line;
  }
  if (line < (char *)file + len && *line == '\x0d') {
    ++line;
  }
  if (line < (char *)file + len && *line == '\x0a') {
    ++line;
  }
  if (line >= (char *)file + len) {
    return NULL;
  }
  return line;
}

void FoFiType1::parse() {
  char *line, *line1, *p, *p2;
  char buf[256];
  char c;
  int n, code, i, j;
  char *tokptr;

  for (i = 1, line = (char *)file;
       i <= 100 && line && (!name || !encoding);
       ++i) {

    // get font name
    if (!name && !strncmp(line, "/FontName", 9)) {
      strncpy(buf, line, 255);
      buf[255] = '\0';
      if ((p = strchr(buf+9, '/')) &&
	  (p = strtok_r(p+1, " \t\n\r", &tokptr))) {
	name = copyString(p);
      }
      line = getNextLine(line);

    // get encoding
    } else if (!encoding &&
	       !strncmp(line, "/Encoding StandardEncoding def", 30)) {
      encoding = fofiType1StandardEncoding;
    } else if (!encoding &&
	       !strncmp(line, "/Encoding 256 array", 19)) {
      encoding = (char **)gmallocn(256, sizeof(char *));
      for (j = 0; j < 256; ++j) {
	encoding[j] = NULL;
      }
      for (j = 0, line = getNextLine(line);
	   j < 300 && line && (line1 = getNextLine(line));
	   ++j, line = line1) {
	if ((n = line1 - line) > 255) {
	  error(-1, "FoFiType1::parse a line has more than 255 characters, we don't support this");
	  n = 255;
	}
	strncpy(buf, line, n);
	buf[n] = '\0';
	for (p = buf; *p == ' ' || *p == '\t'; ++p) ;
	if (!strncmp(p, "dup", 3)) {
	  for (p += 3; *p == ' ' || *p == '\t'; ++p) ;
	  for (p2 = p; *p2 >= '0' && *p2 <= '9'; ++p2) ;
	  if (*p2) {
	    c = *p2; // store it so we can recover it after atoi
	    *p2 = '\0'; // terminate p so atoi works
	    code = atoi(p);
	    *p2 = c;
	    if (code == 8 && *p2 == '#') {
	      code = 0;
	      for (++p2; *p2 >= '0' && *p2 <= '7'; ++p2) {
		code = code * 8 + (*p2 - '0');
	      }
	    }
	    if (likely(code < 256 && code >= 0)) {
	      for (p = p2; *p == ' ' || *p == '\t'; ++p) ;
	      if (*p == '/') {
		++p;
		for (p2 = p; *p2 && *p2 != ' ' && *p2 != '\t'; ++p2) ;
		c = *p2; // store it so we can recover it after copyString
		*p2 = '\0'; // terminate p so copyString works
		encoding[code] = copyString(p);
		*p2 = c;
		p = p2;
		for (; *p == ' ' || *p == '\t'; ++p); // eat spaces between string and put
		if (!strncmp(p, "put", 3)) {
		  // eat put and spaces and newlines after put
		  for (p += 3; *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'; ++p);
		  if (*p)
		  {
		    // there is still something after the definition
		    // there might be another definition in this line
		    // so move line1 to the end of our parsing
		    // so we start in the potential next definition in the next loop
		    line1 = &line[p - buf];
		  }
		} else {
		  error(-1, "FoFiType1::parse no put after dup");
		}
	      }
	    }
	  }
	} else {
	  if (strtok_r(buf, " \t", &tokptr) &&
	      (p = strtok_r(NULL, " \t\n\r", &tokptr)) && !strcmp(p, "def")) {
	    break;
	  }
	}
      }
      //~ check for getinterval/putinterval junk

    } else {
      line = getNextLine(line);
    }
  }

  parsed = gTrue;
}
