//========================================================================
//
// JSInfo.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================


#include "config.h"
#include <stdio.h>
#include "Object.h"
#include "Dict.h"
#include "Annot.h"
#include "PDFDoc.h"
#include "JSInfo.h"
#include "Link.h"
#include "Form.h"
#include "UnicodeMap.h"
#include "UTF.h"

JSInfo::JSInfo(PDFDoc *docA, int firstPage) {
  doc = docA;
  currentPage = firstPage + 1;
}

JSInfo::~JSInfo() {
}

void JSInfo::printJS(GooString *js) {
  Unicode *u;
  char buf[8];
  int i, n, len;

  if (!js || !js->getCString())
    return;

  len = TextStringToUCS4(js, &u);
  for (i = 0; i < len; i++) {
    n = uniMap->mapUnicode(u[i], buf, sizeof(buf));
    fwrite(buf, 1, n, file);
  }
}

void JSInfo::scanLinkAction(LinkAction *link, const char *action) {
  if (!link)
    return;

  if (link->getKind() == actionJavaScript) {
    hasJS = gTrue;
    if (print) {
      LinkJavaScript *linkjs = static_cast<LinkJavaScript *>(link);
      GooString *s = linkjs->getScript();
      if (s && s->getCString()) {
	fprintf(file, "%s:\n", action);
	printJS(s);
	fputs("\n\n", file);
      }
    }
  }

  if (link->getKind() == actionRendition) {
    LinkRendition *linkr = static_cast<LinkRendition *>(link);
    if (linkr->getScript()) {
      hasJS = gTrue;
      if (print) {
        GooString *s = linkr->getScript();
        if (s && s->getCString()) {
          fprintf(file, "%s (Rendition):\n", action);
          printJS(s);
          fputs("\n\n", file);
        }
      }
    }
  }
}

void JSInfo::scanJS(int nPages) {
  print = gFalse;
  file = NULL;
  scan(nPages);
}

void JSInfo::scanJS(int nPages, FILE *fout, UnicodeMap *uMap) {
  print = gTrue;
  file = fout;
  uniMap = uMap;
  scan(nPages);
}

void JSInfo::scan(int nPages) {
  Page *page;
  Annots *annots;
  Object obj1, obj2;
  int lastPage;

  hasJS = gFalse;

  // Names
  int numNames = doc->getCatalog()->numJS();
  if (numNames > 0) {
    hasJS = gTrue;
    if (print) {
      for (int i = 0; i < numNames; i++) {
	fprintf(file, "Name Dictionary \"%s\":\n", doc->getCatalog()->getJSName(i)->getCString());
	printJS(doc->getCatalog()->getJS(i));
	fputs("\n\n", file);
      }
    }
  }

  // document actions
  scanLinkAction(doc->getCatalog()->getAdditionalAction(Catalog::actionCloseDocument),
                 "Before Close Document");
  scanLinkAction(doc->getCatalog()->getAdditionalAction(Catalog::actionSaveDocumentStart),
                 "Before Save Document");
  scanLinkAction(doc->getCatalog()->getAdditionalAction(Catalog::actionSaveDocumentFinish),
                 "After Save Document");
  scanLinkAction(doc->getCatalog()->getAdditionalAction(Catalog::actionPrintDocumentStart),
                 "Before Print Document");
  scanLinkAction(doc->getCatalog()->getAdditionalAction(Catalog::actionPrintDocumentFinish),
                 "After Print Document");

  // form field actions
  if (doc->getCatalog()->getFormType() == Catalog::AcroForm) {
    Form *form = doc->getCatalog()->getForm();
    for (int i = 0; i < form->getNumFields(); i++) {
      FormField *field = form->getRootField(i);
      for (int j = 0; j < field->getNumWidgets(); j++) {
	FormWidget *widget = field->getWidget(j);
	scanLinkAction(widget->getActivationAction(),
                       "Field Activated");
	scanLinkAction(widget->getAdditionalAction(Annot::actionFieldModified),
                       "Field Modified");
	scanLinkAction(widget->getAdditionalAction(Annot::actionFormatField),
                       "Format Field");
	scanLinkAction(widget->getAdditionalAction(Annot::actionValidateField),
                       "Validate Field");
	scanLinkAction(widget->getAdditionalAction(Annot::actionCalculateField),
                       "Calculate Field");
      }
    }
  }

  // scan pages

  if (currentPage > doc->getNumPages()) {
    return;
  }

  lastPage = currentPage + nPages;
  if (lastPage > doc->getNumPages() + 1) {
    lastPage = doc->getNumPages() + 1;
  }

  for (int pg = currentPage; pg < lastPage; ++pg) {
    page = doc->getPage(pg);
    if (!page) continue;

    // page actions (open, close)
    scanLinkAction(page->getAdditionalAction(Page::actionOpenPage), "Page Open");
    scanLinkAction(page->getAdditionalAction(Page::actionClosePage), "Page Close");

    // annotation actions (links, screen, widget)
    annots = page->getAnnots();
    for (int i = 0; i < annots->getNumAnnots(); ++i) {
      if (annots->getAnnot(i)->getType() == Annot::typeLink) {
	AnnotLink *annot = static_cast<AnnotLink *>(annots->getAnnot(i));
	scanLinkAction(annot->getAction(), "Link Annotation Activated");
      } else if (annots->getAnnot(i)->getType() == Annot::typeScreen) {
	AnnotScreen *annot = static_cast<AnnotScreen *>(annots->getAnnot(i));
	scanLinkAction(annot->getAction(),
                       "Screen Annotation Activated");
	scanLinkAction(annot->getAdditionalAction(Annot::actionCursorEntering),
                       "Screen Annotation Cursor Enter");
	scanLinkAction(annot->getAdditionalAction(Annot::actionCursorLeaving),
                       "Screen Annotation Cursor Leave");
	scanLinkAction(annot->getAdditionalAction(Annot::actionMousePressed),
                       "Screen Annotation Mouse Pressed");
	scanLinkAction(annot->getAdditionalAction(Annot::actionMouseReleased),
                       "Screen Annotation Mouse Released");
	scanLinkAction(annot->getAdditionalAction(Annot::actionFocusIn),
                       "Screen Annotation Focus In");
	scanLinkAction(annot->getAdditionalAction(Annot::actionFocusOut),
                       "Screen Annotation Focus Out");
	scanLinkAction(annot->getAdditionalAction(Annot::actionPageOpening),
                       "Screen Annotation Page Open");
	scanLinkAction(annot->getAdditionalAction(Annot::actionPageClosing),
                       "Screen Annotation Page Close");
	scanLinkAction(annot->getAdditionalAction(Annot::actionPageVisible),
                       "Screen Annotation Page Visible");
	scanLinkAction(annot->getAdditionalAction(Annot::actionPageInvisible),
                       "Screen Annotation Page Invisible");

      } else if (annots->getAnnot(i)->getType() == Annot::typeWidget) {
	AnnotWidget *annot = static_cast<AnnotWidget *>(annots->getAnnot(i));
	scanLinkAction(annot->getAction(),
                       "Widget Annotation Activated");
	scanLinkAction(annot->getAdditionalAction(Annot::actionCursorEntering),
                       "Widget Annotation Cursor Enter");
	scanLinkAction(annot->getAdditionalAction(Annot::actionCursorLeaving),
                       "Widget Annotation Cursor Leave");
	scanLinkAction(annot->getAdditionalAction(Annot::actionMousePressed),
                       "Widget Annotation Mouse Pressed");
	scanLinkAction(annot->getAdditionalAction(Annot::actionMouseReleased),
                       "Widget Annotation Mouse Released");
	scanLinkAction(annot->getAdditionalAction(Annot::actionFocusIn),
                       "Widget Annotation Focus In");
	scanLinkAction(annot->getAdditionalAction(Annot::actionFocusOut),
                       "Widget Annotation Focus Out");
	scanLinkAction(annot->getAdditionalAction(Annot::actionPageOpening),
                       "Widget Annotation Page Open");
	scanLinkAction(annot->getAdditionalAction(Annot::actionPageClosing),
                       "Widget Annotation Page Close");
	scanLinkAction(annot->getAdditionalAction(Annot::actionPageVisible),
                       "Widget Annotation Page Visible");
	scanLinkAction(annot->getAdditionalAction(Annot::actionPageInvisible),
                       "Widget Annotation Page Invisible");
      }
    }
  }

  currentPage = lastPage;
}

GBool JSInfo::containsJS() {
  return hasJS;
};
