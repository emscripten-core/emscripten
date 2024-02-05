//========================================================================
//
// SecurityHandler.cc
//
// Copyright 2004 Glyph & Cog, LLC
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

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "GooString.h"
#include "PDFDoc.h"
#include "Decrypt.h"
#include "Error.h"
#include "GlobalParams.h"
#if HAVE_XPDFCORE
#  include "XPDFCore.h"
#elif HAVE_WINPDFCORE
#  include "WinPDFCore.h"
#endif
#ifdef ENABLE_PLUGINS
#  include "XpdfPluginAPI.h"
#endif
#include "SecurityHandler.h"

#include <limits.h>

//------------------------------------------------------------------------
// SecurityHandler
//------------------------------------------------------------------------

SecurityHandler *SecurityHandler::make(PDFDoc *docA, Object *encryptDictA) {
  Object filterObj;
  SecurityHandler *secHdlr;
#ifdef ENABLE_PLUGINS
  XpdfSecurityHandler *xsh;
#endif

  encryptDictA->dictLookup("Filter", &filterObj);
  if (filterObj.isName("Standard")) {
    secHdlr = new StandardSecurityHandler(docA, encryptDictA);
  } else if (filterObj.isName()) {
#ifdef ENABLE_PLUGINS
    if ((xsh = globalParams->getSecurityHandler(filterObj.getName()))) {
      secHdlr = new ExternalSecurityHandler(docA, encryptDictA, xsh);
    } else {
#endif
      error(-1, "Couldn't find the '%s' security handler",
	    filterObj.getName());
      secHdlr = NULL;
#ifdef ENABLE_PLUGINS
    }
#endif
  } else {
    error(-1, "Missing or invalid 'Filter' entry in encryption dictionary");
    secHdlr = NULL;
  }
  filterObj.free();
  return secHdlr;
}

SecurityHandler::SecurityHandler(PDFDoc *docA) {
  doc = docA;
}

SecurityHandler::~SecurityHandler() {
}

GBool SecurityHandler::checkEncryption(GooString *ownerPassword,
				       GooString *userPassword) {
  void *authData;
  GBool ok;
  int i;

  if (ownerPassword || userPassword) {
    authData = makeAuthData(ownerPassword, userPassword);
  } else {
    authData = NULL;
  }
  ok = authorize(authData);
  if (authData) {
    freeAuthData(authData);
  }
  for (i = 0; !ok && i < 3; ++i) {
    if (!(authData = getAuthData())) {
      break;
    }
    ok = authorize(authData);
    if (authData) {
      freeAuthData(authData);
    }
  }
  if (!ok) {
    error(-1, "Incorrect password");
  }
  return ok;
}

//------------------------------------------------------------------------
// StandardSecurityHandler
//------------------------------------------------------------------------

class StandardAuthData {
public:

  StandardAuthData(GooString *ownerPasswordA, GooString *userPasswordA) {
    ownerPassword = ownerPasswordA;
    userPassword = userPasswordA;
  }

  ~StandardAuthData() {
    if (ownerPassword) {
      delete ownerPassword;
    }
    if (userPassword) {
      delete userPassword;
    }
  }

  GooString *ownerPassword;
  GooString *userPassword;
};

StandardSecurityHandler::StandardSecurityHandler(PDFDoc *docA,
						 Object *encryptDictA):
  SecurityHandler(docA)
{
  Object versionObj, revisionObj, lengthObj;
  Object ownerKeyObj, userKeyObj, permObj, fileIDObj;
  Object fileIDObj1;
  Object cryptFiltersObj, streamFilterObj, stringFilterObj;
  Object cryptFilterObj, cfmObj, cfLengthObj;
  Object encryptMetadataObj;

  ok = gFalse;
  fileID = NULL;
  ownerKey = NULL;
  userKey = NULL;

  encryptDictA->dictLookup("V", &versionObj);
  encryptDictA->dictLookup("R", &revisionObj);
  encryptDictA->dictLookup("Length", &lengthObj);
  encryptDictA->dictLookup("O", &ownerKeyObj);
  encryptDictA->dictLookup("U", &userKeyObj);
  encryptDictA->dictLookup("P", &permObj);
  if (permObj.isUint()) {
      unsigned int permUint = permObj.getUint();
      int perms = permUint - UINT_MAX - 1;
      permObj.free();
      permObj.initInt(perms);
  }
  doc->getXRef()->getTrailerDict()->dictLookup("ID", &fileIDObj);
  if (versionObj.isInt() &&
      revisionObj.isInt() &&
      ownerKeyObj.isString() && ownerKeyObj.getString()->getLength() == 32 &&
      userKeyObj.isString() && userKeyObj.getString()->getLength() == 32 &&
      permObj.isInt()) {
    encVersion = versionObj.getInt();
    encRevision = revisionObj.getInt();
    encAlgorithm = cryptRC4;
    // revision 2 forces a 40-bit key - some buggy PDF generators
    // set the Length value incorrectly
    if (encRevision == 2 || !lengthObj.isInt()) {
      fileKeyLength = 5;
    } else {
      fileKeyLength = lengthObj.getInt() / 8;
    }
    encryptMetadata = gTrue;
    //~ this currently only handles a subset of crypt filter functionality
    if (encVersion == 4 && encRevision == 4) {
      encryptDictA->dictLookup("CF", &cryptFiltersObj);
      encryptDictA->dictLookup("StmF", &streamFilterObj);
      encryptDictA->dictLookup("StrF", &stringFilterObj);
      if (cryptFiltersObj.isDict() &&
	  streamFilterObj.isName() &&
	  stringFilterObj.isName() &&
	  !strcmp(streamFilterObj.getName(), stringFilterObj.getName())) {
	if (cryptFiltersObj.dictLookup(streamFilterObj.getName(),
				       &cryptFilterObj)->isDict()) {
	  cryptFilterObj.dictLookup("CFM", &cfmObj);
	  if (cfmObj.isName("V2")) {
	    encVersion = 2;
	    encRevision = 3;
	    if (cryptFilterObj.dictLookup("Length", &cfLengthObj)->isInt()) {
	      //~ according to the spec, this should be cfLengthObj / 8
	      fileKeyLength = cfLengthObj.getInt();
	    }
	    cfLengthObj.free();
	  } else if (cfmObj.isName("AESV2")) {
	    encVersion = 2;
	    encRevision = 3;
	    encAlgorithm = cryptAES;
	    if (cryptFilterObj.dictLookup("Length", &cfLengthObj)->isInt()) {
	      //~ according to the spec, this should be cfLengthObj / 8
	      fileKeyLength = cfLengthObj.getInt();
	    }
	    cfLengthObj.free();
	  }
	  cfmObj.free();
	}
	cryptFilterObj.free();
      }
      stringFilterObj.free();
      streamFilterObj.free();
      cryptFiltersObj.free();
      if (encryptDictA->dictLookup("EncryptMetadata",
				   &encryptMetadataObj)->isBool()) {
	encryptMetadata = encryptMetadataObj.getBool();
      }
      encryptMetadataObj.free();
    }
    permFlags = permObj.getInt();
    ownerKey = ownerKeyObj.getString()->copy();
    userKey = userKeyObj.getString()->copy();
    if (encVersion >= 1 && encVersion <= 2 &&
	encRevision >= 2 && encRevision <= 3) {
      if (fileIDObj.isArray()) {
	if (fileIDObj.arrayGet(0, &fileIDObj1)->isString()) {
	  fileID = fileIDObj1.getString()->copy();
	} else {
	  fileID = new GooString();
	}
	fileIDObj1.free();
      } else {
	fileID = new GooString();
      }
      ok = gTrue;
    } else {
      error(-1, "Unsupported version/revision (%d/%d) of Standard security handler",
	    encVersion, encRevision);
    }
  } else {
    error(-1, "Weird encryption info");
  }
  if (fileKeyLength > 16) {
    fileKeyLength = 16;
  }
  fileIDObj.free();
  permObj.free();
  userKeyObj.free();
  ownerKeyObj.free();
  lengthObj.free();
  revisionObj.free();
  versionObj.free();
}

StandardSecurityHandler::~StandardSecurityHandler() {
  if (fileID) {
    delete fileID;
  }
  if (ownerKey) {
    delete ownerKey;
  }
  if (userKey) {
    delete userKey;
  }
}

void *StandardSecurityHandler::makeAuthData(GooString *ownerPassword,
					    GooString *userPassword) {
  return new StandardAuthData(ownerPassword ? ownerPassword->copy()
			                    : (GooString *)NULL,
			      userPassword ? userPassword->copy()
			                   : (GooString *)NULL);
}

void *StandardSecurityHandler::getAuthData() {
#if HAVE_XPDFCORE
  XPDFCore *core;
  GooString *password;

  if (!(core = (XPDFCore *)doc->getGUIData()) ||
      !(password = core->getPassword())) {
    return NULL;
  }
  return new StandardAuthData(password, password->copy());
#elif HAVE_WINPDFCORE
  WinPDFCore *core;
  GooString *password;

  if (!(core = (WinPDFCore *)doc->getGUIData()) ||
      !(password = core->getPassword())) {
    return NULL;
  }
  return new StandardAuthData(password, password->copy());
#else
  return NULL;
#endif
}

void StandardSecurityHandler::freeAuthData(void *authData) {
  delete (StandardAuthData *)authData;
}

GBool StandardSecurityHandler::authorize(void *authData) {
  GooString *ownerPassword, *userPassword;

  if (!ok) {
    return gFalse;
  }
  if (authData) {
    ownerPassword = ((StandardAuthData *)authData)->ownerPassword;
    userPassword = ((StandardAuthData *)authData)->userPassword;
  } else {
    ownerPassword = NULL;
    userPassword = NULL;
  }
  if (!Decrypt::makeFileKey(encVersion, encRevision, fileKeyLength,
			    ownerKey, userKey, permFlags, fileID,
			    ownerPassword, userPassword, fileKey,
			    encryptMetadata, &ownerPasswordOk)) {
    return gFalse;
  }
  return gTrue;
}

#ifdef ENABLE_PLUGINS

//------------------------------------------------------------------------
// ExternalSecurityHandler
//------------------------------------------------------------------------

ExternalSecurityHandler::ExternalSecurityHandler(PDFDoc *docA,
						 Object *encryptDictA,
						 XpdfSecurityHandler *xshA):
  SecurityHandler(docA)
{
  encryptDictA->copy(&encryptDict);
  xsh = xshA;
  encAlgorithm = cryptRC4; //~ this should be obtained via getKey
  ok = gFalse;

  if (!(*xsh->newDoc)(xsh->handlerData, (XpdfDoc)docA,
		      (XpdfObject)encryptDictA, &docData)) {
    return;
  }

  ok = gTrue;
}

ExternalSecurityHandler::~ExternalSecurityHandler() {
  (*xsh->freeDoc)(xsh->handlerData, docData);
  encryptDict.free();
}

void *ExternalSecurityHandler::makeAuthData(GooString *ownerPassword,
					    GooString *userPassword) {
  char *opw, *upw;
  void *authData;

  opw = ownerPassword ? ownerPassword->getCString() : (char *)NULL;
  upw = userPassword ? userPassword->getCString() : (char *)NULL;
  if (!(*xsh->makeAuthData)(xsh->handlerData, docData, opw, upw, &authData)) {
    return NULL;
  }
  return authData;
}

void *ExternalSecurityHandler::getAuthData() {
  void *authData;

  if (!(*xsh->getAuthData)(xsh->handlerData, docData, &authData)) {
    return NULL;
  }
  return authData;
}

void ExternalSecurityHandler::freeAuthData(void *authData) {
  (*xsh->freeAuthData)(xsh->handlerData, docData, authData);
}

GBool ExternalSecurityHandler::authorize(void *authData) {
  char *key;
  int length;

  if (!ok) {
    return gFalse;
  }
  permFlags = (*xsh->authorize)(xsh->handlerData, docData, authData);
  if (!(permFlags & xpdfPermissionOpen)) {
    return gFalse;
  }
  if (!(*xsh->getKey)(xsh->handlerData, docData, &key, &length, &encVersion)) {
    return gFalse;
  }
  if ((fileKeyLength = length) > 16) {
    fileKeyLength = 16;
  }
  memcpy(fileKey, key, fileKeyLength);
  (*xsh->freeKey)(xsh->handlerData, docData, key, length);
  return gTrue;
}

#endif // ENABLE_PLUGINS
