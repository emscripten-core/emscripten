//========================================================================
//
// Link.cc
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
// Copyright (C) 2006, 2008 Pino Toscano <pino@kde.org>
// Copyright (C) 2007,2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2008 Hugo Mercier <hmercier31@gmail.com>
// Copyright (C) 2008-2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2009 Ilya Gorenbein <igorenbein@finjan.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stddef.h>
#include <string.h>
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "goo/GooList.h"
#include "Error.h"
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Link.h"
#include "Sound.h"
#include "FileSpec.h"
#include "Rendition.h"

//------------------------------------------------------------------------
// LinkAction
//------------------------------------------------------------------------

LinkAction *LinkAction::parseDest(Object *obj) {
  LinkAction *action;

  action = new LinkGoTo(obj);
  if (!action->isOk()) {
    delete action;
    return NULL;
  }
  return action;
}

LinkAction *LinkAction::parseAction(Object *obj, GooString *baseURI) {
  LinkAction *action;
  Object obj2, obj3, obj4;

  if (!obj->isDict()) {
      error(-1, "parseAction: Bad annotation action for URI '%s'",
            baseURI ? baseURI->getCString() : "NULL");
      return NULL;
  }

  obj->dictLookup("S", &obj2);

  // GoTo action
  if (obj2.isName("GoTo")) {
    obj->dictLookup("D", &obj3);
    action = new LinkGoTo(&obj3);
    obj3.free();

  // GoToR action
  } else if (obj2.isName("GoToR")) {
    obj->dictLookup("F", &obj3);
    obj->dictLookup("D", &obj4);
    action = new LinkGoToR(&obj3, &obj4);
    obj3.free();
    obj4.free();

  // Launch action
  } else if (obj2.isName("Launch")) {
    action = new LinkLaunch(obj);

  // URI action
  } else if (obj2.isName("URI")) {
    obj->dictLookup("URI", &obj3);
    action = new LinkURI(&obj3, baseURI);
    obj3.free();

  // Named action
  } else if (obj2.isName("Named")) {
    obj->dictLookup("N", &obj3);
    action = new LinkNamed(&obj3);
    obj3.free();

  // Movie action
  } else if (obj2.isName("Movie")) {
    action = new LinkMovie(obj);

  // Rendition action
  } else if (obj2.isName("Rendition")) {
    action = new LinkRendition(obj);

  // Sound action
  } else if (obj2.isName("Sound")) {
    action = new LinkSound(obj);

  // JavaScript action
  } else if (obj2.isName("JavaScript")) {
    obj->dictLookup("JS", &obj3);
    action = new LinkJavaScript(&obj3);
    obj3.free();

  // Set-OCG-State action
  } else if (obj2.isName("SetOCGState")) {
    action = new LinkOCGState(obj);

  // unknown action
  } else if (obj2.isName()) {
    action = new LinkUnknown(obj2.getName());

  // action is missing or wrong type
  } else {
    error(-1, "parseAction: Unknown annotation action object: URI = '%s'",
          baseURI ? baseURI->getCString() : "NULL");
    action = NULL;
  }

  obj2.free();

  if (action && !action->isOk()) {
    delete action;
    return NULL;
  }
  return action;
}

//------------------------------------------------------------------------
// LinkDest
//------------------------------------------------------------------------

LinkDest::LinkDest(Array *a) {
  Object obj1, obj2;

  // initialize fields
  left = bottom = right = top = zoom = 0;
  changeLeft = changeTop = changeZoom = gFalse;
  ok = gFalse;

  // get page
  if (a->getLength() < 2) {
    error(-1, "Annotation destination array is too short");
    return;
  }
  a->getNF(0, &obj1);
  if (obj1.isInt()) {
    pageNum = obj1.getInt() + 1;
    pageIsRef = gFalse;
  } else if (obj1.isRef()) {
    pageRef.num = obj1.getRefNum();
    pageRef.gen = obj1.getRefGen();
    pageIsRef = gTrue;
  } else {
    error(-1, "Bad annotation destination");
    goto err2;
  }
  obj1.free();

  // get destination type
  a->get(1, &obj1);

  // XYZ link
  if (obj1.isName("XYZ")) {
    kind = destXYZ;
    if (a->getLength() < 3) {
      changeLeft = gFalse;
    } else {
      a->get(2, &obj2);
      if (obj2.isNull()) {
	changeLeft = gFalse;
      } else if (obj2.isNum()) {
	changeLeft = gTrue;
	left = obj2.getNum();
      } else {
	error(-1, "Bad annotation destination position");
	goto err1;
      }
      obj2.free();
    }
    if (a->getLength() < 4) {
      changeTop = gFalse;
    } else {
      a->get(3, &obj2);
      if (obj2.isNull()) {
	changeTop = gFalse;
      } else if (obj2.isNum()) {
	changeTop = gTrue;
	top = obj2.getNum();
      } else {
	error(-1, "Bad annotation destination position");
	goto err1;
      }
      obj2.free();
    }
    if (a->getLength() < 5) {
      changeZoom = gFalse;
    } else {
      a->get(4, &obj2);
      if (obj2.isNull()) {
	changeZoom = gFalse;
      } else if (obj2.isNum()) {
	changeZoom = gTrue;
	zoom = obj2.getNum();
      } else {
	error(-1, "Bad annotation destination position");
	goto err1;
      }
      obj2.free();
    }

  // Fit link
  } else if (obj1.isName("Fit")) {
    if (a->getLength() < 2) {
      error(-1, "Annotation destination array is too short");
      goto err2;
    }
    kind = destFit;

  // FitH link
  } else if (obj1.isName("FitH")) {
    if (a->getLength() < 3) {
      error(-1, "Annotation destination array is too short");
      goto err2;
    }
    kind = destFitH;
    a->get(2, &obj2);
    if (obj2.isNull()) {
      changeTop = gFalse;
    } else if (obj2.isNum()) {
      changeTop = gTrue;
      top = obj2.getNum();
    } else {
      error(-1, "Bad annotation destination position");
      kind = destFit;
    }
    obj2.free();

  // FitV link
  } else if (obj1.isName("FitV")) {
    if (a->getLength() < 3) {
      error(-1, "Annotation destination array is too short");
      goto err2;
    }
    kind = destFitV;
    a->get(2, &obj2);
    if (obj2.isNull()) {
      changeLeft = gFalse;
    } else if (obj2.isNum()) {
      changeLeft = gTrue;
      left = obj2.getNum();
    } else {
      error(-1, "Bad annotation destination position");
      kind = destFit;
    }
    obj2.free();

  // FitR link
  } else if (obj1.isName("FitR")) {
    if (a->getLength() < 6) {
      error(-1, "Annotation destination array is too short");
      goto err2;
    }
    kind = destFitR;
    if (!a->get(2, &obj2)->isNum()) {
      error(-1, "Bad annotation destination position");
      kind = destFit;
    }
    left = obj2.getNum();
    obj2.free();
    if (!a->get(3, &obj2)->isNum()) {
      error(-1, "Bad annotation destination position");
      kind = destFit;
    }
    bottom = obj2.getNum();
    obj2.free();
    if (!a->get(4, &obj2)->isNum()) {
      error(-1, "Bad annotation destination position");
      kind = destFit;
    }
    right = obj2.getNum();
    obj2.free();
    if (!a->get(5, &obj2)->isNum()) {
      error(-1, "Bad annotation destination position");
      kind = destFit;
    }
    top = obj2.getNum();
    obj2.free();

  // FitB link
  } else if (obj1.isName("FitB")) {
    if (a->getLength() < 2) {
      error(-1, "Annotation destination array is too short");
      goto err2;
    }
    kind = destFitB;

  // FitBH link
  } else if (obj1.isName("FitBH")) {
    if (a->getLength() < 3) {
      error(-1, "Annotation destination array is too short");
      goto err2;
    }
    kind = destFitBH;
    a->get(2, &obj2);
    if (obj2.isNull()) {
      changeTop = gFalse;
    } else if (obj2.isNum()) {
      changeTop = gTrue;
      top = obj2.getNum();
    } else {
      error(-1, "Bad annotation destination position");
      kind = destFit;
    }
    obj2.free();

  // FitBV link
  } else if (obj1.isName("FitBV")) {
    if (a->getLength() < 3) {
      error(-1, "Annotation destination array is too short");
      goto err2;
    }
    kind = destFitBV;
    a->get(2, &obj2);
    if (obj2.isNull()) {
      changeLeft = gFalse;
    } else if (obj2.isNum()) {
      changeLeft = gTrue;
      left = obj2.getNum();
    } else {
      error(-1, "Bad annotation destination position");
      kind = destFit;
    }
    obj2.free();

  // unknown link kind
  } else {
    error(-1, "Unknown annotation destination type");
    goto err2;
  }

  obj1.free();
  ok = gTrue;
  return;

 err1:
  obj2.free();
 err2:
  obj1.free();
}

LinkDest::LinkDest(LinkDest *dest) {
  kind = dest->kind;
  pageIsRef = dest->pageIsRef;
  if (pageIsRef)
    pageRef = dest->pageRef;
  else
    pageNum = dest->pageNum;
  left = dest->left;
  bottom = dest->bottom;
  right = dest->right;
  top = dest->top;
  zoom = dest->zoom;
  changeLeft = dest->changeLeft;
  changeTop = dest->changeTop;
  changeZoom = dest->changeZoom;
  ok = gTrue;
}

//------------------------------------------------------------------------
// LinkGoTo
//------------------------------------------------------------------------

LinkGoTo::LinkGoTo(Object *destObj) {
  dest = NULL;
  namedDest = NULL;

  // named destination
  if (destObj->isName()) {
    namedDest = new GooString(destObj->getName());
  } else if (destObj->isString()) {
    namedDest = destObj->getString()->copy();

  // destination dictionary
  } else if (destObj->isArray()) {
    dest = new LinkDest(destObj->getArray());
    if (!dest->isOk()) {
      delete dest;
      dest = NULL;
    }

  // error
  } else {
    error(-1, "Illegal annotation destination");
  }
}

LinkGoTo::~LinkGoTo() {
  if (dest)
    delete dest;
  if (namedDest)
    delete namedDest;
}

//------------------------------------------------------------------------
// LinkGoToR
//------------------------------------------------------------------------

LinkGoToR::LinkGoToR(Object *fileSpecObj, Object *destObj) {
  fileName = NULL;
  dest = NULL;
  namedDest = NULL;

  // get file name
  Object obj1;
  if (getFileSpecNameForPlatform (fileSpecObj, &obj1)) {
    fileName = obj1.getString()->copy();
    obj1.free();
  }

  // named destination
  if (destObj->isName()) {
    namedDest = new GooString(destObj->getName());
  } else if (destObj->isString()) {
    namedDest = destObj->getString()->copy();

  // destination dictionary
  } else if (destObj->isArray()) {
    dest = new LinkDest(destObj->getArray());
    if (!dest->isOk()) {
      delete dest;
      dest = NULL;
    }

  // error
  } else {
    error(-1, "Illegal annotation destination");
  }
}

LinkGoToR::~LinkGoToR() {
  if (fileName)
    delete fileName;
  if (dest)
    delete dest;
  if (namedDest)
    delete namedDest;
}


//------------------------------------------------------------------------
// LinkLaunch
//------------------------------------------------------------------------

LinkLaunch::LinkLaunch(Object *actionObj) {
  Object obj1, obj2, obj3;

  fileName = NULL;
  params = NULL;

  if (actionObj->isDict()) {
    if (!actionObj->dictLookup("F", &obj1)->isNull()) {
      if (getFileSpecNameForPlatform (&obj1, &obj3)) {
	fileName = obj3.getString()->copy();
	obj3.free();
      }
    } else {
      obj1.free();
#ifdef _WIN32
      if (actionObj->dictLookup("Win", &obj1)->isDict()) {
	obj1.dictLookup("F", &obj2);
	if (getFileSpecNameForPlatform (&obj2, &obj3)) {
	  fileName = obj3.getString()->copy();
	  obj3.free();
	}
	obj2.free();
	if (obj1.dictLookup("P", &obj2)->isString()) {
	  params = obj2.getString()->copy();
	}
	obj2.free();
      } else {
	error(-1, "Bad launch-type link action");
      }
#else
      //~ This hasn't been defined by Adobe yet, so assume it looks
      //~ just like the Win dictionary until they say otherwise.
      if (actionObj->dictLookup("Unix", &obj1)->isDict()) {
	obj1.dictLookup("F", &obj2);
	if (getFileSpecNameForPlatform (&obj2, &obj3)) {
	  fileName = obj3.getString()->copy();
	  obj3.free();
	}
	obj2.free();
	if (obj1.dictLookup("P", &obj2)->isString()) {
	  params = obj2.getString()->copy();
	}
	obj2.free();
      } else {
	error(-1, "Bad launch-type link action");
      }
#endif
    }
    obj1.free();
  }
}

LinkLaunch::~LinkLaunch() {
  if (fileName)
    delete fileName;
  if (params)
    delete params;
}

//------------------------------------------------------------------------
// LinkURI
//------------------------------------------------------------------------

LinkURI::LinkURI(Object *uriObj, GooString *baseURI) {
  GooString *uri2;
  int n;
  char c;

  uri = NULL;
  if (uriObj->isString()) {
    uri2 = uriObj->getString()->copy();
    if (baseURI && baseURI->getLength() > 0) {
      n = strcspn(uri2->getCString(), "/:");
      if (n == uri2->getLength() || uri2->getChar(n) == '/') {
	uri = baseURI->copy();
	c = uri->getChar(uri->getLength() - 1);
	if (c == '/' || c == '?') {
	  if (uri2->getChar(0) == '/') {
	    uri2->del(0);
	  }
	} else {
	  if (uri2->getChar(0) != '/') {
	    uri->append('/');
	  }
	}
	uri->append(uri2);
	delete uri2;
      } else {
	uri = uri2;
      }
    } else {
      uri = uri2;
    }
  } else {
    error(-1, "Illegal URI-type link");
  }
}

LinkURI::~LinkURI() {
  if (uri)
    delete uri;
}

//------------------------------------------------------------------------
// LinkNamed
//------------------------------------------------------------------------

LinkNamed::LinkNamed(Object *nameObj) {
  name = NULL;
  if (nameObj->isName()) {
    name = new GooString(nameObj->getName());
  }
}

LinkNamed::~LinkNamed() {
  if (name) {
    delete name;
  }
}

//------------------------------------------------------------------------
// LinkMovie
//------------------------------------------------------------------------

LinkMovie::LinkMovie(Object *obj) {
  annotRef.num = -1;
  annotTitle = NULL;

  Object tmp;
  if (obj->dictLookupNF("Annotation", &tmp)->isRef()) {
    annotRef = tmp.getRef();
  }
  tmp.free();

  if (obj->dictLookup("T", &tmp)->isString()) {
    annotTitle = tmp.getString()->copy();
  }
  tmp.free();

  if ((annotTitle == NULL) && (annotRef.num == -1)) {
    error(-1, "Movie action is missing both the Annot and T keys");
  }

  if (obj->dictLookup("Operation", &tmp)->isName()) {
    char *name = tmp.getName();
    
    if (!strcmp(name, "Play")) {
      operation = operationTypePlay;
    }
    else if (!strcmp(name, "Stop")) {
      operation = operationTypeStop;
    }
    else if (!strcmp(name, "Pause")) {
      operation = operationTypePause;
    }
    else if (!strcmp(name, "Resume")) {
      operation = operationTypeResume;
    }
  }
  tmp.free();
}

LinkMovie::~LinkMovie() {
  if (annotTitle) {
    delete annotTitle;
  }
}

//------------------------------------------------------------------------
// LinkSound
//------------------------------------------------------------------------

LinkSound::LinkSound(Object *soundObj) {
  volume = 1.0;
  sync = gFalse;
  repeat = gFalse;
  mix = gFalse;
  sound = NULL;
  if (soundObj->isDict())
  {
    Object tmp;
    // volume
    soundObj->dictLookup("Volume", &tmp);
    if (tmp.isNum()) {
      volume = tmp.getNum();
    }
    tmp.free();
    // sync
    soundObj->dictLookup("Synchronous", &tmp);
    if (tmp.isBool()) {
      sync = tmp.getBool();
    }
    tmp.free();
    // repeat
    soundObj->dictLookup("Repeat", &tmp);
    if (tmp.isBool()) {
      repeat = tmp.getBool();
    }
    tmp.free();
    // mix
    soundObj->dictLookup("Mix", &tmp);
    if (tmp.isBool()) {
      mix = tmp.getBool();
    }
    tmp.free();
    // 'Sound' object
    soundObj->dictLookup("Sound", &tmp);
    sound = Sound::parseSound(&tmp);
    tmp.free();
  }
}

LinkSound::~LinkSound() {
  delete sound;
}

//------------------------------------------------------------------------
// LinkRendition
//------------------------------------------------------------------------

LinkRendition::LinkRendition(Object *obj) {
  operation = -1;
  media = NULL;
  js = NULL;

  if (obj->isDict()) {
    Object tmp;

    if (!obj->dictLookup("JS", &tmp)->isNull()) {
      if (tmp.isString()) {
        js = new GooString(tmp.getString());
      } else if (tmp.isStream()) {
        Stream *stream = tmp.getStream();
	js = new GooString();
	stream->fillGooString(js);
      } else {
        error(-1, "Invalid Rendition Action: JS not string or stream");
      }
    }
    tmp.free();

    if (obj->dictLookup("OP", &tmp)->isInt()) {
      operation = tmp.getInt();
      if (!js && (operation < 0 || operation > 4)) {
        error (-1, "Invalid Rendition Action: unrecognized operation valued: %d", operation);
      } else {
        Object obj1;

        // retrieve rendition object
        if (obj->dictLookup("R", &renditionObj)->isDict()) {
          media = new MediaRendition(&renditionObj);
	} else if (operation == 0 || operation == 4) {
          error (-1, "Invalid Rendition Action: no R field with op = %d", operation);
	  renditionObj.free();
	}

	if (!obj->dictLookupNF("AN", &screenRef)->isRef() && operation >= 0 && operation <= 4) {
	  error (-1, "Invalid Rendition Action: no AN field with op = %d", operation);
	  screenRef.free();
	}
      }
    } else if (!js) {
      error(-1, "Invalid Rendition action: no OP or JS field defined");
    }
    tmp.free();
  }
}

LinkRendition::~LinkRendition() {
  renditionObj.free();
  screenRef.free();

  if (js)
    delete js;
  if (media)
    delete media;
}


//------------------------------------------------------------------------
// LinkJavaScript
//------------------------------------------------------------------------

LinkJavaScript::LinkJavaScript(Object *jsObj) {
  js = NULL;

  if (jsObj->isString()) {
    js = new GooString(jsObj->getString());
  }
  else if (jsObj->isStream()) {
    Stream *stream = jsObj->getStream();
    js = new GooString();
    stream->fillGooString(js);
  }
}

LinkJavaScript::~LinkJavaScript() {
  if (js) {
    delete js;
  }
}

//------------------------------------------------------------------------
// LinkOCGState
//------------------------------------------------------------------------
LinkOCGState::LinkOCGState(Object *obj) {
  Object obj1;

  stateList = new GooList();
  preserveRB = gTrue;

  if (obj->dictLookup("State", &obj1)->isArray()) {
    StateList *stList = NULL;

    for (int i = 0; i < obj1.arrayGetLength(); ++i) {
      Object obj2;

      obj1.arrayGetNF(i, &obj2);
      if (obj2.isName()) {
        if (stList)
	  stateList->append(stList);

	char *name = obj2.getName();
	stList = new StateList();
	stList->list = new GooList();
	if (!strcmp (name, "ON")) {
	  stList->st = On;
	} else if (!strcmp (name, "OFF")) {
	  stList->st = Off;
	} else if (!strcmp (name, "Toggle")) {
	  stList->st = Toggle;
	} else {
	  error (-1, "Invalid name '%s' in OCG Action state array", name);
	  delete stList;
	  stList = NULL;
	}
      } else if (obj2.isRef()) {
        if (stList) {
	  Ref ocgRef = obj2.getRef();
	  Ref *item = new Ref();
	  item->num = ocgRef.num;
	  item->gen = ocgRef.gen;
	  stList->list->append(item);
	} else {
	  error (-1, "Invalid OCG Action State array, expected name instead of ref");
	}
      } else {
        error (-1, "Invalid item in OCG Action State array");
      }
      obj2.free();
    }
    // Add the last group
    if (stList)
      stateList->append(stList);
  } else {
    error (-1, "Invalid OCGState action");
    delete stateList;
    stateList = NULL;
  }
  obj1.free();

  if (obj->dictLookup("PreserveRB", &obj1)->isBool()) {
    preserveRB = obj1.getBool();
  }
  obj1.free();
}

LinkOCGState::~LinkOCGState() {
  if (stateList)
    deleteGooList(stateList, StateList);
}

LinkOCGState::StateList::~StateList() {
  if (list)
    deleteGooList(list, Ref);
}

//------------------------------------------------------------------------
// LinkUnknown
//------------------------------------------------------------------------

LinkUnknown::LinkUnknown(char *actionA) {
  action = new GooString(actionA);
}

LinkUnknown::~LinkUnknown() {
  delete action;
}

//------------------------------------------------------------------------
// Link
//------------------------------------------------------------------------

Link::Link(Dict *dict, GooString *baseURI) {
  Object obj1, obj2;
  double t;

  action = NULL;
  ok = gFalse;

  // get rectangle
  if (!dict->lookup("Rect", &obj1)->isArray()) {
    error(-1, "Annotation rectangle is wrong type");
    goto err2;
  }
  if (!obj1.arrayGet(0, &obj2)->isNum()) {
    error(-1, "Bad annotation rectangle");
    goto err1;
  }
  x1 = obj2.getNum();
  obj2.free();
  if (!obj1.arrayGet(1, &obj2)->isNum()) {
    error(-1, "Bad annotation rectangle");
    goto err1;
  }
  y1 = obj2.getNum();
  obj2.free();
  if (!obj1.arrayGet(2, &obj2)->isNum()) {
    error(-1, "Bad annotation rectangle");
    goto err1;
  }
  x2 = obj2.getNum();
  obj2.free();
  if (!obj1.arrayGet(3, &obj2)->isNum()) {
    error(-1, "Bad annotation rectangle");
    goto err1;
  }
  y2 = obj2.getNum();
  obj2.free();
  obj1.free();
  if (x1 > x2) {
    t = x1;
    x1 = x2;
    x2 = t;
  }
  if (y1 > y2) {
    t = y1;
    y1 = y2;
    y2 = t;
  }

  // look for destination
  if (!dict->lookup("Dest", &obj1)->isNull()) {
    action = LinkAction::parseDest(&obj1);

  // look for action
  } else {
    obj1.free();
    if (dict->lookup("A", &obj1)->isDict()) {
      action = LinkAction::parseAction(&obj1, baseURI);
    }
  }
  obj1.free();

  // check for bad action
  if (action) {
    ok = gTrue;
  }

  return;

 err1:
  obj2.free();
 err2:
  obj1.free();
}

Link::~Link() {
  if (action) {
    delete action;
  }
}

//------------------------------------------------------------------------
// Links
//------------------------------------------------------------------------

Links::Links(Object *annots, GooString *baseURI) {
  Link *link;
  Object obj1, obj2;
  int size;
  int i;

  links = NULL;
  size = 0;
  numLinks = 0;

  if (annots->isArray()) {
    for (i = 0; i < annots->arrayGetLength(); ++i) {
      if (annots->arrayGet(i, &obj1)->isDict()) {
	if (obj1.dictLookup("Subtype", &obj2)->isName("Link")) {
	  link = new Link(obj1.getDict(), baseURI);
	  if (link->isOk()) {
	    if (numLinks >= size) {
	      size += 16;
	      links = (Link **)greallocn(links, size, sizeof(Link *));
	    }
	    links[numLinks++] = link;
	  } else {
	    delete link;
	  }
	}
	obj2.free();
      }
      obj1.free();
    }
  }
}

Links::~Links() {
  int i;

  for (i = 0; i < numLinks; ++i)
    delete links[i];
  gfree(links);
}

LinkAction *Links::find(double x, double y) const {
  int i;

  for (i = numLinks - 1; i >= 0; --i) {
    if (links[i]->inRect(x, y)) {
      return links[i]->getAction();
    }
  }
  return NULL;
}

GBool Links::onLink(double x, double y) const {
  int i;

  for (i = 0; i < numLinks; ++i) {
    if (links[i]->inRect(x, y))
      return gTrue;
  }
  return gFalse;
}
