//========================================================================
//
// Link.h
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
// Copyright (C) 2008 Hugo Mercier <hmercier31@gmail.com>
// Copyright (C) 2010, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2012 Tobias Koening <tobias.koenig@kdab.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef LINK_H
#define LINK_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "Object.h"

class GooString;
class GooList;
class Array;
class Dict;
class Sound;
class MediaRendition;
class AnnotLink;
class Annots;

//------------------------------------------------------------------------
// LinkAction
//------------------------------------------------------------------------

enum LinkActionKind {
  actionGoTo,			// go to destination
  actionGoToR,			// go to destination in new file
  actionLaunch,			// launch app (or open document)
  actionURI,			// URI
  actionNamed,			// named action
  actionMovie,			// movie action
  actionRendition,		// rendition action
  actionSound,			// sound action
  actionJavaScript,		// JavaScript action
  actionOCGState,               // Set-OCG-State action
  actionUnknown			// anything else
};

class LinkAction {
public:

  // Destructor.
  virtual ~LinkAction() {}

  // Was the LinkAction created successfully?
  virtual GBool isOk() = 0;

  // Check link action type.
  virtual LinkActionKind getKind() = 0;

  // Parse a destination (old-style action) name, string, or array.
  static LinkAction *parseDest(Object *obj);

  // Parse an action dictionary.
  static LinkAction *parseAction(Object *obj, GooString *baseURI = NULL);
};

//------------------------------------------------------------------------
// LinkDest
//------------------------------------------------------------------------

enum LinkDestKind {
  destXYZ,
  destFit,
  destFitH,
  destFitV,
  destFitR,
  destFitB,
  destFitBH,
  destFitBV
};

class LinkDest {
public:

  // Build a LinkDest from the array.
  LinkDest(Array *a);

  // Copy a LinkDest.
  LinkDest *copy() { return new LinkDest(this); }

  // Was the LinkDest created successfully?
  GBool isOk() { return ok; }

  // Accessors.
  LinkDestKind getKind() { return kind; }
  GBool isPageRef() { return pageIsRef; }
  int getPageNum() { return pageNum; }
  Ref getPageRef() { return pageRef; }
  double getLeft() { return left; }
  double getBottom() { return bottom; }
  double getRight() { return right; }
  double getTop() { return top; }
  double getZoom() { return zoom; }
  GBool getChangeLeft() { return changeLeft; }
  GBool getChangeTop() { return changeTop; }
  GBool getChangeZoom() { return changeZoom; }

private:

  LinkDestKind kind;		// destination type
  GBool pageIsRef;		// is the page a reference or number?
  union {
    Ref pageRef;		// reference to page
    int pageNum;		// one-relative page number
  };
  double left, bottom;		// position
  double right, top;
  double zoom;			// zoom factor
  GBool changeLeft, changeTop;	// which position components to change:
  GBool changeZoom;		//   destXYZ uses all three;
				//   destFitH/BH use changeTop;
				//   destFitV/BV use changeLeft
  GBool ok;			// set if created successfully

  LinkDest(LinkDest *dest);
};

//------------------------------------------------------------------------
// LinkGoTo
//------------------------------------------------------------------------

class LinkGoTo: public LinkAction {
public:

  // Build a LinkGoTo from a destination (dictionary, name, or string).
  LinkGoTo(Object *destObj);

  // Destructor.
  virtual ~LinkGoTo();

  // Was the LinkGoTo created successfully?
  virtual GBool isOk() { return dest || namedDest; }

  // Accessors.
  virtual LinkActionKind getKind() { return actionGoTo; }
  LinkDest *getDest() { return dest; }
  GooString *getNamedDest() { return namedDest; }

private:

  LinkDest *dest;		// regular destination (NULL for remote
				//   link with bad destination)
  GooString *namedDest;	// named destination (only one of dest and
				//   and namedDest may be non-NULL)
};

//------------------------------------------------------------------------
// LinkGoToR
//------------------------------------------------------------------------

class LinkGoToR: public LinkAction {
public:

  // Build a LinkGoToR from a file spec (dictionary) and destination
  // (dictionary, name, or string).
  LinkGoToR(Object *fileSpecObj, Object *destObj);

  // Destructor.
  virtual ~LinkGoToR();

  // Was the LinkGoToR created successfully?
  virtual GBool isOk() { return fileName && (dest || namedDest); }

  // Accessors.
  virtual LinkActionKind getKind() { return actionGoToR; }
  GooString *getFileName() { return fileName; }
  LinkDest *getDest() { return dest; }
  GooString *getNamedDest() { return namedDest; }

private:

  GooString *fileName;		// file name
  LinkDest *dest;		// regular destination (NULL for remote
				//   link with bad destination)
  GooString *namedDest;	// named destination (only one of dest and
				//   and namedDest may be non-NULL)
};

//------------------------------------------------------------------------
// LinkLaunch
//------------------------------------------------------------------------

class LinkLaunch: public LinkAction {
public:

  // Build a LinkLaunch from an action dictionary.
  LinkLaunch(Object *actionObj);

  // Destructor.
  virtual ~LinkLaunch();

  // Was the LinkLaunch created successfully?
  virtual GBool isOk() { return fileName != NULL; }

  // Accessors.
  virtual LinkActionKind getKind() { return actionLaunch; }
  GooString *getFileName() { return fileName; }
  GooString *getParams() { return params; }

private:

  GooString *fileName;		// file name
  GooString *params;		// parameters
};

//------------------------------------------------------------------------
// LinkURI
//------------------------------------------------------------------------

class LinkURI: public LinkAction {
public:

  // Build a LinkURI given the URI (string) and base URI.
  LinkURI(Object *uriObj, GooString *baseURI);

  // Destructor.
  virtual ~LinkURI();

  // Was the LinkURI created successfully?
  virtual GBool isOk() { return uri != NULL; }

  // Accessors.
  virtual LinkActionKind getKind() { return actionURI; }
  GooString *getURI() { return uri; }

private:

  GooString *uri;			// the URI
};

//------------------------------------------------------------------------
// LinkNamed
//------------------------------------------------------------------------

class LinkNamed: public LinkAction {
public:

  // Build a LinkNamed given the action name.
  LinkNamed(Object *nameObj);

  virtual ~LinkNamed();

  virtual GBool isOk() { return name != NULL; }

  virtual LinkActionKind getKind() { return actionNamed; }
  GooString *getName() { return name; }

private:

  GooString *name;
};


//------------------------------------------------------------------------
// LinkMovie
//------------------------------------------------------------------------

class LinkMovie: public LinkAction {
public:

  enum OperationType {
    operationTypePlay,
    operationTypePause,
    operationTypeResume,
    operationTypeStop
  };

  LinkMovie(Object *obj);
  virtual ~LinkMovie();

  virtual GBool isOk() { return annotRef.num >= 0 || annotTitle != NULL; }
  virtual LinkActionKind getKind() { return actionMovie; }

  // a movie action stores either an indirect reference to a movie annotation
  // or the movie annotation title

  GBool hasAnnotRef() { return annotRef.num >= 0; }
  GBool hasAnnotTitle() { return annotTitle != NULL; }
  Ref *getAnnotRef() { return &annotRef; }
  GooString *getAnnotTitle() { return annotTitle; }

  OperationType getOperation() { return operation; }

private:

  Ref annotRef;            // Annotation
  GooString *annotTitle;   // T

  OperationType operation; // Operation
};


//------------------------------------------------------------------------
// LinkRendition
//------------------------------------------------------------------------

class LinkRendition: public LinkAction {
public:
  /**
   * Describes the possible rendition operations.
   */
  enum RenditionOperation {
    NoRendition,
    PlayRendition,
    StopRendition,
    PauseRendition,
    ResumeRendition
  };

  LinkRendition(Object *Obj);

  virtual ~LinkRendition();

  virtual GBool isOk() { return true; }

  virtual LinkActionKind getKind() { return actionRendition; }

  GBool hasRenditionObject() { return renditionObj.isDict(); }
  Object* getRenditionObject() { return &renditionObj; }

  GBool hasScreenAnnot() { return screenRef.isRef(); }
  Ref getScreenAnnot() { return screenRef.getRef(); }

  RenditionOperation getOperation() { return operation; }

  MediaRendition* getMedia() { return media; }

  GooString *getScript() { return js; }

private:

  Object screenRef;
  Object renditionObj;
  RenditionOperation operation;

  MediaRendition* media;

  GooString *js;
};

//------------------------------------------------------------------------
// LinkSound
//------------------------------------------------------------------------

class LinkSound: public LinkAction {
public:

  LinkSound(Object *soundObj);

  virtual ~LinkSound();

  virtual GBool isOk() { return sound != NULL; }

  virtual LinkActionKind getKind() { return actionSound; }

  double getVolume() { return volume; }
  GBool getSynchronous() { return sync; }
  GBool getRepeat() { return repeat; }
  GBool getMix() { return mix; }
  Sound *getSound() { return sound; }

private:

  double volume;
  GBool sync;
  GBool repeat;
  GBool mix;
  Sound *sound;
};

//------------------------------------------------------------------------
// LinkJavaScript
//------------------------------------------------------------------------

class LinkJavaScript: public LinkAction {
public:

  // Build a LinkJavaScript given the action name.
  LinkJavaScript(Object *jsObj);

  virtual ~LinkJavaScript();

  virtual GBool isOk() { return js != NULL; }

  virtual LinkActionKind getKind() { return actionJavaScript; }
  GooString *getScript() { return js; }

private:

  GooString *js;
};

//------------------------------------------------------------------------
// LinkOCGState
//------------------------------------------------------------------------
class LinkOCGState: public LinkAction {
public:
  LinkOCGState(Object *obj);

  virtual ~LinkOCGState();

  virtual GBool isOk() { return stateList != NULL; }

  virtual LinkActionKind getKind() { return actionOCGState; }

  enum State { On, Off, Toggle};
  struct StateList {
    StateList() { list = NULL; }
    ~StateList();
    State st;
    GooList *list;
  };

  GooList *getStateList() { return stateList; }
  GBool getPreserveRB() { return preserveRB; }

private:
  GooList *stateList;
  GBool preserveRB;
};

//------------------------------------------------------------------------
// LinkUnknown
//------------------------------------------------------------------------

class LinkUnknown: public LinkAction {
public:

  // Build a LinkUnknown with the specified action type.
  LinkUnknown(char *actionA);

  // Destructor.
  virtual ~LinkUnknown();

  // Was the LinkUnknown create successfully?
  virtual GBool isOk() { return action != NULL; }

  // Accessors.
  virtual LinkActionKind getKind() { return actionUnknown; }
  GooString *getAction() { return action; }

private:

  GooString *action;		// action subtype
};

//------------------------------------------------------------------------
// Links
//------------------------------------------------------------------------

class Links {
public:

  // Extract links from array of annotations.
  Links(Annots *annots);

  // Destructor.
  ~Links();

  // Iterate through list of links.
  int getNumLinks() const { return numLinks; }
  AnnotLink *getLink(int i) const { return links[i]; }

  // If point <x>,<y> is in a link, return the associated action;
  // else return NULL.
  LinkAction *find(double x, double y) const;

  // Return true if <x>,<y> is in a link.
  GBool onLink(double x, double y) const;

private:

  AnnotLink **links;
  int numLinks;
};

#endif
