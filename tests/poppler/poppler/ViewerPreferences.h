//========================================================================
//
// ViewerPreferences.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2011 Pino Toscano <pino@kde.org>
//
//========================================================================

#ifndef VIEWERPREFERENCES_H
#define VIEWERPREFERENCES_H

#include "goo/gtypes.h"

class Dict;

//------------------------------------------------------------------------
// ViewerPreferences
//------------------------------------------------------------------------

class ViewerPreferences {
public:

  enum NonFullScreenPageMode {
    nfpmUseNone,
    nfpmUseOutlines,
    nfpmUseThumbs,
    nfpmUseOC
  };
  enum Direction {
    directionL2R,
    directionR2L
  };
  enum PrintScaling {
    printScalingNone,
    printScalingAppDefault
  };
  enum Duplex {
    duplexNone,
    duplexSimplex,
    duplexDuplexFlipShortEdge,
    duplexDuplexFlipLongEdge
  };

  ViewerPreferences(Dict *prefDict);
  ~ViewerPreferences();

  GBool getHideToolbar() const { return hideToolbar; }
  GBool getHideMenubar() const { return hideMenubar; }
  GBool getHideWindowUI() const { return hideWindowUI; }
  GBool getFitWindow() const { return fitWindow; }
  GBool getCenterWindow() const { return centerWindow; }
  GBool getDisplayDocTitle() const { return displayDocTitle; }
  NonFullScreenPageMode getNonFullScreenPageMode() const { return nonFullScreenPageMode; }
  Direction getDirection() const { return direction; }
  PrintScaling getPrintScaling() const { return printScaling; }
  Duplex getDuplex() const { return duplex; }

private:

  void init();

  GBool hideToolbar;
  GBool hideMenubar;
  GBool hideWindowUI;
  GBool fitWindow;
  GBool centerWindow;
  GBool displayDocTitle;
  NonFullScreenPageMode nonFullScreenPageMode;
  Direction direction;
  PrintScaling printScaling;
  Duplex duplex;
};

#endif
