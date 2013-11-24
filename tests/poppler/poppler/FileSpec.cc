//========================================================================
//
// FileSpec.cc
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2008-2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2012 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2012 Hib Eris <hib@hiberis.nl>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

//========================================================================
//
// Most of the code from Link.cc and PSOutputDev.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC 
//
//========================================================================

#include <config.h>

#include "FileSpec.h"

EmbFile::EmbFile(Object *efStream)
{
  m_size = -1;
  m_createDate = NULL;
  m_modDate = NULL;
  m_checksum = NULL;
  m_mimetype = NULL;

  efStream->copy(&m_objStr);

  if (efStream->isStream()) {
    // dataDict corresponds to Table 3.41 in the PDF1.6 spec.
    Dict *dataDict = efStream->streamGetDict();

    // subtype is normally the mimetype
    Object subtypeName;
    if (dataDict->lookup("Subtype", &subtypeName)->isName()) {
      m_mimetype = new GooString(subtypeName.getName());
    }
    subtypeName.free();

    // paramDict corresponds to Table 3.42 in the PDF1.6 spec
    Object paramDict;
    if (dataDict->lookup("Params", &paramDict)->isDict()) {
      Object paramObj;
      if (paramDict.dictLookup("ModDate", &paramObj)->isString())
        m_modDate = new GooString(paramObj.getString());
      paramObj.free();

      if (paramDict.dictLookup("CreationDate", &paramObj)->isString())
        m_createDate = new GooString(paramObj.getString());
      paramObj.free();

      if (paramDict.dictLookup("Size", &paramObj)->isInt())
        m_size = paramObj.getInt();
      paramObj.free();

      if (paramDict.dictLookup("CheckSum", &paramObj)->isString())
        m_checksum = new GooString(paramObj.getString());
      paramObj.free();
    }
    paramDict.free();
  }
}

EmbFile::~EmbFile()
{
  delete m_createDate;
  delete m_modDate;
  delete m_checksum;
  delete m_mimetype;
  m_objStr.free();
}

GBool EmbFile::save(const char *path) {
  FILE *f;
  GBool ret;

  if (!(f = fopen(path, "wb"))) {
    return gFalse;
  }
  ret = save2(f);
  fclose(f);
  return ret;
}

GBool EmbFile::save2(FILE *f) {
  int c;

  m_objStr.streamReset();
  while ((c = m_objStr.streamGetChar()) != EOF) {
    fputc(c, f);
  }
  return gTrue;
}

FileSpec::FileSpec(Object *fileSpecA)
{
  ok = gTrue;
  fileName = NULL;
  platformFileName = NULL;
  embFile = NULL;
  desc = NULL;
  fileSpecA->copy(&fileSpec);

  Object obj1;
  if (!getFileSpecName(fileSpecA, &obj1)) {
    ok = gFalse;
    obj1.free();
    error(errSyntaxError, -1, "Invalid FileSpec");
    return;
  }

  fileName = obj1.getString()->copy();
  obj1.free();

  if (fileSpec.isDict()) {
    if (fileSpec.dictLookup("EF", &obj1)->isDict()) {
      if (!obj1.dictLookupNF("F", &fileStream)->isRef()) {
        ok = gFalse;
        fileStream.free();
        error(errSyntaxError, -1, "Invalid FileSpec: Embedded file stream is not an indirect reference");
        obj1.free();
        return;
      }
    }
    obj1.free();
  }

  if (fileSpec.dictLookup("Desc", &obj1)->isString())
    desc = obj1.getString()->copy();
  obj1.free();
}

FileSpec::~FileSpec()
{
  fileSpec.free();
  fileStream.free();
  delete fileName;
  delete platformFileName;
  delete embFile;
  delete desc;
}

EmbFile *FileSpec::getEmbeddedFile()
{
  if(!ok)
    return NULL;

  if (embFile)
    return embFile;

  Object obj1;
  XRef *xref = fileSpec.getDict()->getXRef();
  embFile = new EmbFile(fileStream.fetch(xref, &obj1));
  obj1.free();

  return embFile;
}

GooString *FileSpec::getFileNameForPlatform()
{
  if (platformFileName)
    return platformFileName;

  Object obj1;
  if (getFileSpecNameForPlatform(&fileSpec, &obj1))
    platformFileName = obj1.getString()->copy();
  obj1.free();

  return platformFileName;
}

GBool getFileSpecName (Object *fileSpec, Object *fileName)
{
  if (fileSpec->isString()) {
    fileSpec->copy(fileName);
    return gTrue;
  }
  
  if (fileSpec->isDict()) {
    fileSpec->dictLookup("UF", fileName);
    if (fileName->isString()) {
      return gTrue;
    }
    fileName->free();
    fileSpec->dictLookup("F", fileName);
    if (fileName->isString()) {
      return gTrue;
    }
    fileName->free();
    fileSpec->dictLookup("DOS", fileName);
    if (fileName->isString()) {
      return gTrue;
    }
    fileName->free();
    fileSpec->dictLookup("Mac", fileName);
    if (fileName->isString()) {
      return gTrue;
    }
    fileName->free();
    fileSpec->dictLookup("Unix", fileName);
    if (fileName->isString()) {
      return gTrue;
    }
    fileName->free();
  }
  return gFalse;
}

GBool getFileSpecNameForPlatform (Object *fileSpec, Object *fileName)
{
  if (fileSpec->isString()) {
    fileSpec->copy(fileName);
    return gTrue;
  }

  if (fileSpec->isDict()) {
    if (!fileSpec->dictLookup("UF", fileName)->isString ()) {
      fileName->free();
      if (!fileSpec->dictLookup("F", fileName)->isString ()) {
        fileName->free();
#ifdef _WIN32
	const char *platform = "DOS";
#else
	const char *platform = "Unix";
#endif
	if (!fileSpec->dictLookup(platform, fileName)->isString ()) {
	  fileName->free();
	  error(errSyntaxError, -1, "Illegal file spec");
	  return gFalse;
	}
      }
    }
  } else {
    error(errSyntaxError, -1, "Illegal file spec");
    return gFalse;
  }

  // system-dependent path manipulation
#ifdef _WIN32
  int i, j;
  GooString *name = fileName->getString();
  // "//...."             --> "\...."
  // "/x/...."            --> "x:\...."
  // "/server/share/...." --> "\\server\share\...."
  // convert escaped slashes to slashes and unescaped slashes to backslashes
  i = 0;
  if (name->getChar(0) == '/') {
    if (name->getLength() >= 2 && name->getChar(1) == '/') {
      name->del(0);
      i = 0;
    } else if (name->getLength() >= 2 &&
	       ((name->getChar(1) >= 'a' && name->getChar(1) <= 'z') ||
		(name->getChar(1) >= 'A' && name->getChar(1) <= 'Z')) &&
	       (name->getLength() == 2 || name->getChar(2) == '/')) {
      name->setChar(0, name->getChar(1));
      name->setChar(1, ':');
      i = 2;
    } else {
      for (j = 2; j < name->getLength(); ++j) {
        if (name->getChar(j-1) != '\\' &&
	    name->getChar(j) == '/') {
	  break;
	}
      }
      if (j < name->getLength()) {
        name->setChar(0, '\\');
	name->insert(0, '\\');
	i = 2;
      }
    }
  }
  for (; i < name->getLength(); ++i) {
    if (name->getChar(i) == '/') {
      name->setChar(i, '\\');
    } else if (name->getChar(i) == '\\' &&
	       i+1 < name->getLength() &&
	       name->getChar(i+1) == '/') {
      name->del(i);
    }
  }
#endif /* _WIN32 */

  return gTrue;
}
