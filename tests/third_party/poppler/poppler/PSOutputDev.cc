//========================================================================
//
// PSOutputDev.cc
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
// Copyright (C) 2005 Martin Kretzschmar <martink@gnome.org>
// Copyright (C) 2005, 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2006-2009 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2007, 2008 Brad Hards <bradh@kde.org>
// Copyright (C) 2008, 2009 Koji Otani <sho@bbr.jp>
// Copyright (C) 2008, 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2009, 2010 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2009 Till Kamppeter <till.kamppeter@gmail.com>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009, 2011 William Bader <williambader@hotmail.com>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2009, 2010 Adrian Johnson <ajohnson@redneon.com>
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
#include <stddef.h>
#include <stdarg.h>
#include <signal.h>
#include <math.h>
#include <limits.h>
#include "goo/GooString.h"
#include "goo/GooList.h"
#include "poppler-config.h"
#include "GlobalParams.h"
#include "Object.h"
#include "Error.h"
#include "Function.h"
#include "Gfx.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "UnicodeMap.h"
#include <fofi/FoFiType1C.h>
#include <fofi/FoFiTrueType.h>
#include "Catalog.h"
#include "Page.h"
#include "Stream.h"
#include "Annot.h"
#include "XRef.h"
#include "PreScanOutputDev.h"
#include "FileSpec.h"
#if HAVE_SPLASH
#  include "splash/Splash.h"
#  include "splash/SplashBitmap.h"
#  include "SplashOutputDev.h"
#endif
#include "PSOutputDev.h"
#include "PDFDoc.h"

#ifdef MACOS
// needed for setting type/creator of MacOS files
#include "ICSupport.h"
#endif

// the MSVC math.h doesn't define this
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//------------------------------------------------------------------------

// Resolution at which pages with transparency will be rasterized.
#define splashDPI 300

//------------------------------------------------------------------------
// PostScript prolog and setup
//------------------------------------------------------------------------

// The '~' escapes mark prolog code that is emitted only in certain
// levels:
//
//   ~[123][sn]
//      ^   ^----- s=psLevel*Sep, n=psLevel*
//      +----- 1=psLevel1*, 2=psLevel2*, 3=psLevel3*

static char *prolog[] = {
  "/xpdf 75 dict def xpdf begin",
  "% PDF special state",
  "/pdfDictSize 15 def",
  "~1sn",
  "/pdfStates 64 array def",
  "  0 1 63 {",
  "    pdfStates exch pdfDictSize dict",
  "    dup /pdfStateIdx 3 index put",
  "    put",
  "  } for",
  "~123sn",
  "/pdfSetup {",
  "  3 1 roll 2 array astore",
  "  /setpagedevice where {",
  "    pop 3 dict begin",
  "      /PageSize exch def",
  "      /ImagingBBox null def",
  "      /Policies 1 dict dup begin /PageSize 3 def end def",
  "      { /Duplex true def } if",
  "    currentdict end setpagedevice",
  "  } {",
  "    pop pop",
  "  } ifelse",
  "} def",
  "~1sn",
  "/pdfOpNames [",
  "  /pdfFill /pdfStroke /pdfLastFill /pdfLastStroke",
  "  /pdfTextMat /pdfFontSize /pdfCharSpacing /pdfTextRender /pdfPatternCS",
  "  /pdfTextRise /pdfWordSpacing /pdfHorizScaling /pdfTextClipPath",
  "] def",
  "~123sn",
  "/pdfStartPage {",
  "~1sn",
  "  pdfStates 0 get begin",
  "~23sn",
  "  pdfDictSize dict begin",
  "~23n",
  "  /pdfFillCS [] def",
  "  /pdfFillXform {} def",
  "  /pdfStrokeCS [] def",
  "  /pdfStrokeXform {} def",
  "~1n",
  "  /pdfFill 0 def",
  "  /pdfStroke 0 def",
  "~1s",
  "  /pdfFill [0 0 0 1] def",
  "  /pdfStroke [0 0 0 1] def",
  "~23sn",
  "  /pdfFill [0] def",
  "  /pdfStroke [0] def",
  "  /pdfFillOP false def",
  "  /pdfStrokeOP false def",
  "~123sn",
  "  /pdfLastFill false def",
  "  /pdfLastStroke false def",
  "  /pdfTextMat [1 0 0 1 0 0] def",
  "  /pdfFontSize 0 def",
  "  /pdfCharSpacing 0 def",
  "  /pdfTextRender 0 def",
  "  /pdfPatternCS false def", 
  "  /pdfTextRise 0 def",
  "  /pdfWordSpacing 0 def",
  "  /pdfHorizScaling 1 def",
  "  /pdfTextClipPath [] def",
  "} def",
  "/pdfEndPage { end } def",
  "~23s",
  "% separation convention operators",
  "/findcmykcustomcolor where {",
  "  pop",
  "}{",
  "  /findcmykcustomcolor { 5 array astore } def",
  "} ifelse",
  "/setcustomcolor where {",
  "  pop",
  "}{",
  "  /setcustomcolor {",
  "    exch",
  "    [ exch /Separation exch dup 4 get exch /DeviceCMYK exch",
  "      0 4 getinterval cvx",
  "      [ exch /dup load exch { mul exch dup } /forall load",
  "        /pop load dup ] cvx",
  "    ] setcolorspace setcolor",
  "  } def",
  "} ifelse",
  "/customcolorimage where {",
  "  pop",
  "}{",
  "  /customcolorimage {",
  "    gsave",
  "    [ exch /Separation exch dup 4 get exch /DeviceCMYK exch",
  "      0 4 getinterval",
  "      [ exch /dup load exch { mul exch dup } /forall load",
  "        /pop load dup ] cvx",
  "    ] setcolorspace",
  "    10 dict begin",
  "      /ImageType 1 def",
  "      /DataSource exch def",
  "      /ImageMatrix exch def",
  "      /BitsPerComponent exch def",
  "      /Height exch def",
  "      /Width exch def",
  "      /Decode [1 0] def",
  "    currentdict end",
  "    image",
  "    grestore",
  "  } def",
  "} ifelse",
  "~123sn",
  "% PDF color state",
  "~1n",
  "/g { dup /pdfFill exch def setgray",
  "     /pdfLastFill true def /pdfLastStroke false def } def",
  "/G { dup /pdfStroke exch def setgray",
  "     /pdfLastStroke true def /pdfLastFill false def } def",
  "/fCol {",
  "  pdfLastFill not {",
  "    pdfFill setgray",
  "    /pdfLastFill true def /pdfLastStroke false def",
  "  } if",
  "} def",
  "/sCol {",
  "  pdfLastStroke not {",
  "    pdfStroke setgray",
  "    /pdfLastStroke true def /pdfLastFill false def",
  "  } if",
  "} def",
  "~1s",
  "/k { 4 copy 4 array astore /pdfFill exch def setcmykcolor",
  "     /pdfLastFill true def /pdfLastStroke false def } def",
  "/K { 4 copy 4 array astore /pdfStroke exch def setcmykcolor",
  "     /pdfLastStroke true def /pdfLastFill false def } def",
  "/fCol {",
  "  pdfLastFill not {",
  "    pdfFill aload pop setcmykcolor",
  "    /pdfLastFill true def /pdfLastStroke false def",
  "  } if",
  "} def",
  "/sCol {",
  "  pdfLastStroke not {",
  "    pdfStroke aload pop setcmykcolor",
  "    /pdfLastStroke true def /pdfLastFill false def",
  "  } if",
  "} def",
  "~23n",
  "/cs { /pdfFillXform exch def dup /pdfFillCS exch def",
  "      setcolorspace } def",
  "/CS { /pdfStrokeXform exch def dup /pdfStrokeCS exch def",
  "      setcolorspace } def",
  "/sc { pdfLastFill not { pdfFillCS setcolorspace } if",
  "      dup /pdfFill exch def aload pop pdfFillXform setcolor",
  "     /pdfLastFill true def /pdfLastStroke false def } def",
  "/SC { pdfLastStroke not { pdfStrokeCS setcolorspace } if",
  "      dup /pdfStroke exch def aload pop pdfStrokeXform setcolor",
  "     /pdfLastStroke true def /pdfLastFill false def } def",
  "/op { /pdfFillOP exch def",
  "      pdfLastFill { pdfFillOP setoverprint } if } def",
  "/OP { /pdfStrokeOP exch def",
  "      pdfLastStroke { pdfStrokeOP setoverprint } if } def",
  "/fCol {",
  "  pdfLastFill not {",
  "    pdfFillCS setcolorspace",
  "    pdfFill aload pop pdfFillXform setcolor",
  "    pdfFillOP setoverprint",
  "    /pdfLastFill true def /pdfLastStroke false def",
  "  } if",
  "} def",
  "/sCol {",
  "  pdfLastStroke not {",
  "    pdfStrokeCS setcolorspace",
  "    pdfStroke aload pop pdfStrokeXform setcolor",
  "    pdfStrokeOP setoverprint",
  "    /pdfLastStroke true def /pdfLastFill false def",
  "  } if",
  "} def",
  "~23s",
  "/k { 4 copy 4 array astore /pdfFill exch def setcmykcolor",
  "     /pdfLastFill true def /pdfLastStroke false def } def",
  "/K { 4 copy 4 array astore /pdfStroke exch def setcmykcolor",
  "     /pdfLastStroke true def /pdfLastFill false def } def",
  "/ck { 6 copy 6 array astore /pdfFill exch def",
  "      findcmykcustomcolor exch setcustomcolor",
  "      /pdfLastFill true def /pdfLastStroke false def } def",
  "/CK { 6 copy 6 array astore /pdfStroke exch def",
  "      findcmykcustomcolor exch setcustomcolor",
  "      /pdfLastStroke true def /pdfLastFill false def } def",
  "/op { /pdfFillOP exch def",
  "      pdfLastFill { pdfFillOP setoverprint } if } def",
  "/OP { /pdfStrokeOP exch def",
  "      pdfLastStroke { pdfStrokeOP setoverprint } if } def",
  "/fCol {",
  "  pdfLastFill not {",
  "    pdfFill aload length 4 eq {",
  "      setcmykcolor",
  "    }{",
  "      findcmykcustomcolor exch setcustomcolor",
  "    } ifelse",
  "    pdfFillOP setoverprint",
  "    /pdfLastFill true def /pdfLastStroke false def",
  "  } if",
  "} def",
  "/sCol {",
  "  pdfLastStroke not {",
  "    pdfStroke aload length 4 eq {",
  "      setcmykcolor",
  "    }{",
  "      findcmykcustomcolor exch setcustomcolor",
  "    } ifelse",
  "    pdfStrokeOP setoverprint",
  "    /pdfLastStroke true def /pdfLastFill false def",
  "  } if",
  "} def",
  "~123sn",
  "% build a font",
  "/pdfMakeFont {",
  "  4 3 roll findfont",
  "  4 2 roll matrix scale makefont",
  "  dup length dict begin",
  "    { 1 index /FID ne { def } { pop pop } ifelse } forall",
  "    /Encoding exch def",
  "    currentdict",
  "  end",
  "  definefont pop",
  "} def",
  "/pdfMakeFont16 {",
  "  exch findfont",
  "  dup length dict begin",
  "    { 1 index /FID ne { def } { pop pop } ifelse } forall",
  "    /WMode exch def",
  "    currentdict",
  "  end",
  "  definefont pop",
  "} def",
  "~3sn",
  "/pdfMakeFont16L3 {",
  "  1 index /CIDFont resourcestatus {",
  "    pop pop 1 index /CIDFont findresource /CIDFontType known",
  "  } {",
  "    false",
  "  } ifelse",
  "  {",
  "    0 eq { /Identity-H } { /Identity-V } ifelse",
  "    exch 1 array astore composefont pop",
  "  } {",
  "    pdfMakeFont16",
  "  } ifelse",
  "} def",
  "~123sn",
  "% graphics state operators",
  "~1sn",
  "/q {",
  "  gsave",
  "  pdfOpNames length 1 sub -1 0 { pdfOpNames exch get load } for",
  "  pdfStates pdfStateIdx 1 add get begin",
  "  pdfOpNames { exch def } forall",
  "} def",
  "/Q { end grestore } def",
  "~23sn",
  "/q { gsave pdfDictSize dict begin } def",
  "/Q {",
  "  end grestore",
  "  /pdfLastFill where {",
  "    pop",
  "    pdfLastFill {",
  "      pdfFillOP setoverprint",
  "    } {",
  "      pdfStrokeOP setoverprint",
  "    } ifelse",
  "  } if",
  "} def",
  "~123sn",
  "/cm { concat } def",
  "/d { setdash } def",
  "/i { setflat } def",
  "/j { setlinejoin } def",
  "/J { setlinecap } def",
  "/M { setmiterlimit } def",
  "/w { setlinewidth } def",
  "% path segment operators",
  "/m { moveto } def",
  "/l { lineto } def",
  "/c { curveto } def",
  "/re { 4 2 roll moveto 1 index 0 rlineto 0 exch rlineto",
  "      neg 0 rlineto closepath } def",
  "/h { closepath } def",
  "% path painting operators",
  "/S { sCol stroke } def",
  "/Sf { fCol stroke } def",
  "/f { fCol fill } def",
  "/f* { fCol eofill } def",
  "% clipping operators",
  "/W { clip newpath } def",
  "/W* { eoclip newpath } def",
  "/Ws { strokepath clip newpath } def",
  "% text state operators",
  "/Tc { /pdfCharSpacing exch def } def",
  "/Tf { dup /pdfFontSize exch def",
  "      dup pdfHorizScaling mul exch matrix scale",
  "      pdfTextMat matrix concatmatrix dup 4 0 put dup 5 0 put",
  "      exch findfont exch makefont setfont } def",
  "/Tr { /pdfTextRender exch def } def",
  "/Tp { /pdfPatternCS exch def } def", 
  "/Ts { /pdfTextRise exch def } def",
  "/Tw { /pdfWordSpacing exch def } def",
  "/Tz { /pdfHorizScaling exch def } def",
  "% text positioning operators",
  "/Td { pdfTextMat transform moveto } def",
  "/Tm { /pdfTextMat exch def } def",
  "% text string operators",
  "/cshow where {",
  "  pop",
  "  /cshow2 {",
  "    dup {",
  "      pop pop",
  "      1 string dup 0 3 index put 3 index exec",
  "    } exch cshow",
  "    pop pop",
  "  } def",
  "}{",
  "  /cshow2 {",
  "    currentfont /FontType get 0 eq {",
  "      0 2 2 index length 1 sub {",
  "        2 copy get exch 1 add 2 index exch get",
  "        2 copy exch 256 mul add",
  "        2 string dup 0 6 5 roll put dup 1 5 4 roll put",
  "        3 index exec",
  "      } for",
  "    } {",
  "      dup {",
  "        1 string dup 0 3 index put 3 index exec",
  "      } forall",
  "    } ifelse",
  "    pop pop",
  "  } def",
  "} ifelse",
  "/awcp {", // awidthcharpath
  "  exch {",
  "    false charpath",
  "    5 index 5 index rmoveto",
  "    6 index eq { 7 index 7 index rmoveto } if",
  "  } exch cshow2",
  "  6 {pop} repeat",
  "} def",
  "/Tj {",
  "  fCol",  // because stringwidth has to draw Type 3 chars
  "  1 index stringwidth pdfTextMat idtransform pop",
  "  sub 1 index length dup 0 ne { div } { pop pop 0 } ifelse",
  "  pdfWordSpacing pdfHorizScaling mul 0 pdfTextMat dtransform 32",
  "  4 3 roll pdfCharSpacing pdfHorizScaling mul add 0",
  "  pdfTextMat dtransform",
  "  6 5 roll Tj1",
  "} def",
  "/Tj16 {",
  "  fCol",  // because stringwidth has to draw Type 3 chars
  "  2 index stringwidth pdfTextMat idtransform pop",
  "  sub exch div",
  "  pdfWordSpacing pdfHorizScaling mul 0 pdfTextMat dtransform 32",
  "  4 3 roll pdfCharSpacing pdfHorizScaling mul add 0",
  "  pdfTextMat dtransform",
  "  6 5 roll Tj1",
  "} def",
  "/Tj16V {",
  "  fCol",  // because stringwidth has to draw Type 3 chars
  "  2 index stringwidth pdfTextMat idtransform exch pop",
  "  sub exch div",
  "  0 pdfWordSpacing pdfTextMat dtransform 32",
  "  4 3 roll pdfCharSpacing add 0 exch",
  "  pdfTextMat dtransform",
  "  6 5 roll Tj1",
  "} def",
  "/Tj1 {",
  "  0 pdfTextRise pdfTextMat dtransform rmoveto",
  "  currentpoint 8 2 roll",
  "  pdfTextRender 1 and 0 eq pdfPatternCS not and {",
  "    6 copy awidthshow",
  "  } if",
  "  pdfTextRender 3 and dup 1 eq exch 2 eq or {",
  "    7 index 7 index moveto",
  "    6 copy",
  "    currentfont /FontType get 3 eq { fCol } { sCol } ifelse",
  "    false awcp currentpoint stroke moveto",
  "  } if",
  "  pdfTextRender 4 and 0 ne pdfPatternCS or {",
  "    8 6 roll moveto",
  "    false awcp",
  "    /pdfTextClipPath [ pdfTextClipPath aload pop",
  "      {/moveto cvx}",
  "      {/lineto cvx}",
  "      {/curveto cvx}",
  "      {/closepath cvx}",
  "    pathforall ] def",
  "    currentpoint newpath moveto",
  "  } {",
  "    8 {pop} repeat",
  "  } ifelse",
  "  0 pdfTextRise neg pdfTextMat dtransform rmoveto",
  "} def",
  "/TJm { pdfFontSize 0.001 mul mul neg 0",
  "       pdfTextMat dtransform rmoveto } def",
  "/TJmV { pdfFontSize 0.001 mul mul neg 0 exch",
  "        pdfTextMat dtransform rmoveto } def",
  "/Tclip { pdfTextClipPath cvx exec clip newpath",
  "         /pdfTextClipPath [] def } def",
  "/Tclip* { pdfTextClipPath cvx exec eoclip newpath",
  "         /pdfTextClipPath [] def } def",
  "~1ns",
  "% Level 1 image operators",
  "~1n",
  "/pdfIm1 {",
  "  /pdfImBuf1 4 index string def",
  "  { currentfile pdfImBuf1 readhexstring pop } image",
  "} def",
  "~1s",
  "/pdfIm1Sep {",
  "  /pdfImBuf1 4 index string def",
  "  /pdfImBuf2 4 index string def",
  "  /pdfImBuf3 4 index string def",
  "  /pdfImBuf4 4 index string def",
  "  { currentfile pdfImBuf1 readhexstring pop }",
  "  { currentfile pdfImBuf2 readhexstring pop }",
  "  { currentfile pdfImBuf3 readhexstring pop }",
  "  { currentfile pdfImBuf4 readhexstring pop }",
  "  true 4 colorimage",
  "} def",
  "~1ns",
  "/pdfImM1 {",
  "  fCol /pdfImBuf1 4 index 7 add 8 idiv string def",
  "  { currentfile pdfImBuf1 readhexstring pop } imagemask",
  "} def",
  "/pdfImM1a {",
  "  { 2 copy get exch 1 add exch } imagemask",
  "  pop pop",
  "} def",
  "~23sn",
  "% Level 2 image operators",
  "/pdfImBuf 100 string def",
  "/pdfIm {",
  "  image",
  "  { currentfile pdfImBuf readline",
  "    not { pop exit } if",
  "    (%-EOD-) eq { exit } if } loop",
  "} def",
  "~23s",
  "/pdfImSep {",
  "  findcmykcustomcolor exch",
  "  dup /Width get /pdfImBuf1 exch string def",
  "  dup /Decode get aload pop 1 index sub /pdfImDecodeRange exch def",
  "  /pdfImDecodeLow exch def",
  "  begin Width Height BitsPerComponent ImageMatrix DataSource end",
  "  /pdfImData exch def",
  "  { pdfImData pdfImBuf1 readstring pop",
  "    0 1 2 index length 1 sub {",
  "      1 index exch 2 copy get",
  "      pdfImDecodeRange mul 255 div pdfImDecodeLow add round cvi",
  "      255 exch sub put",
  "    } for }",
  "  6 5 roll customcolorimage",
  "  { currentfile pdfImBuf readline",
  "    not { pop exit } if",
  "    (%-EOD-) eq { exit } if } loop",
  "} def",
  "~23sn",
  "/pdfImM {",
  "  fCol imagemask",
  "  { currentfile pdfImBuf readline",
  "    not { pop exit } if",
  "    (%-EOD-) eq { exit } if } loop",
  "} def",
  "~123sn",
  "/pr { 2 index 2 index 3 2 roll putinterval 4 add } def",
  "/pdfImClip {",
  "  gsave",
  "  0 2 4 index length 1 sub {",
  "    dup 4 index exch 2 copy",
  "    get 5 index div put",
  "    1 add 3 index exch 2 copy",
  "    get 3 index div put",
  "  } for",
  "  pop pop rectclip",
  "} def",
  "/pdfImClipEnd { grestore } def",
  "~23sn",
  "% shading operators",
  "/colordelta {",
  "  false 0 1 3 index length 1 sub {",
  "    dup 4 index exch get 3 index 3 2 roll get sub abs 0.004 gt {",
  "      pop true",
  "    } if",
  "  } for",
  "  exch pop exch pop",
  "} def",
  "/funcCol { func n array astore } def",
  "/funcSH {",
  "  dup 0 eq {",
  "    true",
  "  } {",
  "    dup 6 eq {",
  "      false",
  "    } {",
  "      4 index 4 index funcCol dup",
  "      6 index 4 index funcCol dup",
  "      3 1 roll colordelta 3 1 roll",
  "      5 index 5 index funcCol dup",
  "      3 1 roll colordelta 3 1 roll",
  "      6 index 8 index funcCol dup",
  "      3 1 roll colordelta 3 1 roll",
  "      colordelta or or or",
  "    } ifelse",
  "  } ifelse",
  "  {",
  "    1 add",
  "    4 index 3 index add 0.5 mul exch 4 index 3 index add 0.5 mul exch",
  "    6 index 6 index 4 index 4 index 4 index funcSH",
  "    2 index 6 index 6 index 4 index 4 index funcSH",
  "    6 index 2 index 4 index 6 index 4 index funcSH",
  "    5 3 roll 3 2 roll funcSH pop pop",
  "  } {",
  "    pop 3 index 2 index add 0.5 mul 3 index  2 index add 0.5 mul",
  "~23n",
  "    funcCol sc",
  "~23s",
  "    funcCol aload pop k",
  "~23sn",
  "    dup 4 index exch mat transform m",
  "    3 index 3 index mat transform l",
  "    1 index 3 index mat transform l",
  "    mat transform l pop pop h f*",
  "  } ifelse",
  "} def",
  "/axialCol {",
  "  dup 0 lt {",
  "    pop t0",
  "  } {",
  "    dup 1 gt {",
  "      pop t1",
  "    } {",
  "      dt mul t0 add",
  "    } ifelse",
  "  } ifelse",
  "  func n array astore",
  "} def",
  "/axialSH {",
  "  dup 0 eq {",
  "    true",
  "  } {",
  "    dup 8 eq {",
  "      false",
  "    } {",
  "      2 index axialCol 2 index axialCol colordelta",
  "    } ifelse",
  "  } ifelse",
  "  {",
  "    1 add 3 1 roll 2 copy add 0.5 mul",
  "    dup 4 3 roll exch 4 index axialSH",
  "    exch 3 2 roll axialSH",
  "  } {",
  "    pop 2 copy add 0.5 mul",
  "~23n",
  "    axialCol sc",
  "~23s",
  "    axialCol aload pop k",
  "~23sn",
  "    exch dup dx mul x0 add exch dy mul y0 add",
  "    3 2 roll dup dx mul x0 add exch dy mul y0 add",
  "    dx abs dy abs ge {",
  "      2 copy yMin sub dy mul dx div add yMin m",
  "      yMax sub dy mul dx div add yMax l",
  "      2 copy yMax sub dy mul dx div add yMax l",
  "      yMin sub dy mul dx div add yMin l",
  "      h f*",
  "    } {",
  "      exch 2 copy xMin sub dx mul dy div add xMin exch m",
  "      xMax sub dx mul dy div add xMax exch l",
  "      exch 2 copy xMax sub dx mul dy div add xMax exch l",
  "      xMin sub dx mul dy div add xMin exch l",
  "      h f*",
  "    } ifelse",
  "  } ifelse",
  "} def",
  "/radialCol {",
  "  dup t0 lt {",
  "    pop t0",
  "  } {",
  "    dup t1 gt {",
  "      pop t1",
  "    } if",
  "  } ifelse",
  "  func n array astore",
  "} def",
  "/radialSH {",
  "  dup 0 eq {",
  "    true",
  "  } {",
  "    dup 8 eq {",
  "      false",
  "    } {",
  "      2 index dt mul t0 add radialCol",
  "      2 index dt mul t0 add radialCol colordelta",
  "    } ifelse",
  "  } ifelse",
  "  {",
  "    1 add 3 1 roll 2 copy add 0.5 mul",
  "    dup 4 3 roll exch 4 index radialSH",
  "    exch 3 2 roll radialSH",
  "  } {",
  "    pop 2 copy add 0.5 mul dt mul t0 add",
  "~23n",
  "    radialCol sc",
  "~23s",
  "    radialCol aload pop k",
  "~23sn",
  "    encl {",
  "      exch dup dx mul x0 add exch dup dy mul y0 add exch dr mul r0 add",
  "      0 360 arc h",
  "      dup dx mul x0 add exch dup dy mul y0 add exch dr mul r0 add",
  "      360 0 arcn h f",
  "    } {",
  "      2 copy",
  "      dup dx mul x0 add exch dup dy mul y0 add exch dr mul r0 add",
  "      a1 a2 arcn",
  "      dup dx mul x0 add exch dup dy mul y0 add exch dr mul r0 add",
  "      a2 a1 arcn h",
  "      dup dx mul x0 add exch dup dy mul y0 add exch dr mul r0 add",
  "      a1 a2 arc",
  "      dup dx mul x0 add exch dup dy mul y0 add exch dr mul r0 add",
  "      a2 a1 arc h f",
  "    } ifelse",
  "  } ifelse",
  "} def",
  "~123sn",
  "end",
  NULL
};

static char *cmapProlog[] = {
  "/CIDInit /ProcSet findresource begin",
  "10 dict begin",
  "  begincmap",
  "  /CMapType 1 def",
  "  /CMapName /Identity-H def",
  "  /CIDSystemInfo 3 dict dup begin",
  "    /Registry (Adobe) def",
  "    /Ordering (Identity) def",
  "    /Supplement 0 def",
  "  end def",
  "  1 begincodespacerange",
  "    <0000> <ffff>",
  "  endcodespacerange",
  "  0 usefont",
  "  1 begincidrange",
  "    <0000> <ffff> 0",
  "  endcidrange",
  "  endcmap",
  "  currentdict CMapName exch /CMap defineresource pop",
  "end",
  "10 dict begin",
  "  begincmap",
  "  /CMapType 1 def",
  "  /CMapName /Identity-V def",
  "  /CIDSystemInfo 3 dict dup begin",
  "    /Registry (Adobe) def",
  "    /Ordering (Identity) def",
  "    /Supplement 0 def",
  "  end def",
  "  /WMode 1 def",
  "  1 begincodespacerange",
  "    <0000> <ffff>",
  "  endcodespacerange",
  "  0 usefont",
  "  1 begincidrange",
  "    <0000> <ffff> 0",
  "  endcidrange",
  "  endcmap",
  "  currentdict CMapName exch /CMap defineresource pop",
  "end",
  "end",
  NULL
};

//------------------------------------------------------------------------
// Fonts
//------------------------------------------------------------------------

struct PSSubstFont {
  char *psName;			// PostScript name
  double mWidth;		// width of 'm' character
};

static const char *psFonts[] = {
  "Courier",
  "Courier-Bold",
  "Courier-Oblique",
  "Courier-BoldOblique",
  "Helvetica",
  "Helvetica-Bold",
  "Helvetica-Oblique",
  "Helvetica-BoldOblique",
  "Symbol",
  "Times-Roman",
  "Times-Bold",
  "Times-Italic",
  "Times-BoldItalic",
  "ZapfDingbats",
  NULL
};

static const PSSubstFont psSubstFonts[] = {
  {"Helvetica",             0.833},
  {"Helvetica-Oblique",     0.833},
  {"Helvetica-Bold",        0.889},
  {"Helvetica-BoldOblique", 0.889},
  {"Times-Roman",           0.788},
  {"Times-Italic",          0.722},
  {"Times-Bold",            0.833},
  {"Times-BoldItalic",      0.778},
  {"Courier",               0.600},
  {"Courier-Oblique",       0.600},
  {"Courier-Bold",          0.600},
  {"Courier-BoldOblique",   0.600}
};

// Info for 8-bit fonts
struct PSFont8Info {
  Ref fontID;
  Gushort *codeToGID;		// code-to-GID mapping for TrueType fonts
};

// Encoding info for substitute 16-bit font
struct PSFont16Enc {
  Ref fontID;
  GooString *enc;
};

//------------------------------------------------------------------------
// process colors
//------------------------------------------------------------------------

#define psProcessCyan     1
#define psProcessMagenta  2
#define psProcessYellow   4
#define psProcessBlack    8
#define psProcessCMYK    15

//------------------------------------------------------------------------
// PSOutCustomColor
//------------------------------------------------------------------------

class PSOutCustomColor {
public:

  PSOutCustomColor(double cA, double mA,
		   double yA, double kA, GooString *nameA);
  ~PSOutCustomColor();

  double c, m, y, k;
  GooString *name;
  PSOutCustomColor *next;
};

PSOutCustomColor::PSOutCustomColor(double cA, double mA,
				   double yA, double kA, GooString *nameA) {
  c = cA;
  m = mA;
  y = yA;
  k = kA;
  name = nameA;
  next = NULL;
}

PSOutCustomColor::~PSOutCustomColor() {
  delete name;
}

//------------------------------------------------------------------------

struct PSOutImgClipRect {
  int x0, x1, y0, y1;
};

//------------------------------------------------------------------------
// DeviceNRecoder
//------------------------------------------------------------------------

class DeviceNRecoder: public FilterStream {
public:

  DeviceNRecoder(Stream *strA, int widthA, int heightA,
		 GfxImageColorMap *colorMapA);
  virtual ~DeviceNRecoder();
  virtual StreamKind getKind() { return strWeird; }
  virtual void reset();
  virtual int getChar()
    { return (bufIdx >= bufSize && !fillBuf()) ? EOF : buf[bufIdx++]; }
  virtual int lookChar()
    { return (bufIdx >= bufSize && !fillBuf()) ? EOF : buf[bufIdx]; }
  virtual GooString *getPSFilter(int psLevel, char *indent) { return NULL; }
  virtual GBool isBinary(GBool last = gTrue) { return gTrue; }
  virtual GBool isEncoder() { return gTrue; }

private:

  GBool fillBuf();

  int width, height;
  GfxImageColorMap *colorMap;
  Function *func;
  ImageStream *imgStr;
  int buf[gfxColorMaxComps];
  int pixelIdx;
  int bufIdx;
  int bufSize;
};

DeviceNRecoder::DeviceNRecoder(Stream *strA, int widthA, int heightA,
			       GfxImageColorMap *colorMapA):
    FilterStream(strA) {
  width = widthA;
  height = heightA;
  colorMap = colorMapA;
  imgStr = NULL;
  pixelIdx = 0;
  bufIdx = gfxColorMaxComps;
  bufSize = ((GfxDeviceNColorSpace *)colorMap->getColorSpace())->
              getAlt()->getNComps();
  func = ((GfxDeviceNColorSpace *)colorMap->getColorSpace())->
           getTintTransformFunc();
}

DeviceNRecoder::~DeviceNRecoder() {
  if (imgStr) {
    delete imgStr;
  }
}

void DeviceNRecoder::reset() {
  imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(),
			   colorMap->getBits());
  imgStr->reset();
}

GBool DeviceNRecoder::fillBuf() {
  Guchar pixBuf[gfxColorMaxComps];
  GfxColor color;
  double x[gfxColorMaxComps], y[gfxColorMaxComps];
  int i;

  if (pixelIdx >= width * height) {
    return gFalse;
  }
  imgStr->getPixel(pixBuf);
  colorMap->getColor(pixBuf, &color);
  for (i = 0;
       i < ((GfxDeviceNColorSpace *)colorMap->getColorSpace())->getNComps();
       ++i) {
    x[i] = colToDbl(color.c[i]);
  }
  func->transform(x, y);
  for (i = 0; i < bufSize; ++i) {
    buf[i] = (int)(y[i] * 255 + 0.5);
  }
  bufIdx = 0;
  ++pixelIdx;
  return gTrue;
}

//------------------------------------------------------------------------
// PSOutputDev
//------------------------------------------------------------------------

extern "C" {
typedef void (*SignalFunc)(int);
}

static void outputToFile(void *stream, char *data, int len) {
  fwrite(data, 1, len, (FILE *)stream);
}

PSOutputDev::PSOutputDev(const char *fileName, PDFDoc *doc, XRef *xrefA, Catalog *catalog,
			 char *psTitle,
			 int firstPage, int lastPage, PSOutMode modeA,
			 int paperWidthA, int paperHeightA, GBool duplexA,
			 int imgLLXA, int imgLLYA, int imgURXA, int imgURYA,
			 GBool forceRasterizeA,
			 GBool manualCtrlA) {
  FILE *f;
  PSFileType fileTypeA;

  underlayCbk = NULL;
  underlayCbkData = NULL;
  overlayCbk = NULL;
  overlayCbkData = NULL;

  fontIDs = NULL;
  fontFileIDs = NULL;
  fontFileNames = NULL;
  font8Info = NULL;
  font16Enc = NULL;
  imgIDs = NULL;
  formIDs = NULL;
  xobjStack = NULL;
  embFontList = NULL;
  customColors = NULL;
  haveTextClip = gFalse;
  haveCSPattern = gFalse;
  t3String = NULL;

  forceRasterize = forceRasterizeA;

  // open file or pipe
  if (!strcmp(fileName, "-")) {
    fileTypeA = psStdout;
    f = stdout;
  } else if (fileName[0] == '|') {
    fileTypeA = psPipe;
#ifdef HAVE_POPEN
#ifndef _WIN32
    signal(SIGPIPE, (SignalFunc)SIG_IGN);
#endif
    if (!(f = popen(fileName + 1, "w"))) {
      error(-1, "Couldn't run print command '%s'", fileName);
      ok = gFalse;
      return;
    }
#else
    error(-1, "Print commands are not supported ('%s')", fileName);
    ok = gFalse;
    return;
#endif
  } else {
    fileTypeA = psFile;
    if (!(f = fopen(fileName, "w"))) {
      error(-1, "Couldn't open PostScript file '%s'", fileName);
      ok = gFalse;
      return;
    }
  }

  init(outputToFile, f, fileTypeA, psTitle,
       doc, xrefA, catalog, firstPage, lastPage, modeA,
       imgLLXA, imgLLYA, imgURXA, imgURYA, manualCtrlA,
       paperWidthA, paperHeightA, duplexA);
}

PSOutputDev::PSOutputDev(PSOutputFunc outputFuncA, void *outputStreamA,
			 char *psTitle,
			 PDFDoc *doc,
			 XRef *xrefA, Catalog *catalog,
			 int firstPage, int lastPage, PSOutMode modeA,
			 int paperWidthA, int paperHeightA, GBool duplexA,
			 int imgLLXA, int imgLLYA, int imgURXA, int imgURYA,
			 GBool forceRasterizeA,
			 GBool manualCtrlA) {
  underlayCbk = NULL;
  underlayCbkData = NULL;
  overlayCbk = NULL;
  overlayCbkData = NULL;

  fontIDs = NULL;
  fontFileIDs = NULL;
  fontFileNames = NULL;
  font8Info = NULL;
  font16Enc = NULL;
  imgIDs = NULL;
  formIDs = NULL;
  xobjStack = NULL;
  embFontList = NULL;
  customColors = NULL;
  haveTextClip = gFalse;
  haveCSPattern = gFalse;
  t3String = NULL;

  forceRasterize = forceRasterizeA;

  init(outputFuncA, outputStreamA, psGeneric, psTitle,
       doc, xrefA, catalog, firstPage, lastPage, modeA,
       imgLLXA, imgLLYA, imgURXA, imgURYA, manualCtrlA,
       paperWidthA, paperHeightA, duplexA);
}

void PSOutputDev::init(PSOutputFunc outputFuncA, void *outputStreamA,
		       PSFileType fileTypeA, char *pstitle, PDFDoc *doc, XRef *xrefA, Catalog *catalog,
		       int firstPage, int lastPage, PSOutMode modeA,
		       int imgLLXA, int imgLLYA, int imgURXA, int imgURYA,
		       GBool manualCtrlA, int paperWidthA, int paperHeightA,
		       GBool duplexA) {
  PDFRectangle *box;

  // initialize
  displayText = gTrue;
  ok = gTrue;
  outputFunc = outputFuncA;
  outputStream = outputStreamA;
  fileType = fileTypeA;
  m_catalog = catalog;
  xref = xrefA;
  level = globalParams->getPSLevel();
  mode = modeA;
  paperWidth = paperWidthA;
  paperHeight = paperHeightA;
  imgLLX = imgLLXA;
  imgLLY = imgLLYA;
  imgURX = imgURXA;
  imgURY = imgURYA;
  if (paperWidth < 0 || paperHeight < 0) {
    Page *page;
    if ((page = doc->getPage(firstPage))) {
      paperWidth = (int)ceil(page->getMediaWidth());
      paperHeight = (int)ceil(page->getMediaHeight());
    } else {
      error(-1, "Invalid page %d", firstPage);
      paperWidth = 1;
      paperHeight = 1;
    }
  }
  substFonts = globalParams->getPSSubstFonts();
  preload = globalParams->getPSPreload();
  if (imgLLX == 0 && imgURX == 0 && imgLLY == 0 && imgURY == 0) {
    imgLLX = imgLLY = 0;
    imgURX = paperWidth;
    imgURY = paperHeight;
  }
  if (imgLLX == 0 && imgURX == 0 && imgLLY == 0 && imgURY == 0) {
    imgLLX = imgLLY = 0;
    imgURX = paperWidth;
    imgURY = paperHeight;
  }
  manualCtrl = manualCtrlA;
  if (mode == psModeForm) {
    lastPage = firstPage;
  }
  processColors = 0;
  inType3Char = gFalse;

#if OPI_SUPPORT
  // initialize OPI nesting levels
  opi13Nest = 0;
  opi20Nest = 0;
#endif

  tx0 = ty0 = -1;
  xScale0 = yScale0 = 0;
  rotate0 = -1;
  clipLLX0 = clipLLY0 = 0;
  clipURX0 = clipURY0 = -1;

  // initialize fontIDs, fontFileIDs, and fontFileNames lists
  fontIDSize = 64;
  fontIDLen = 0;
  fontIDs = (Ref *)gmallocn(fontIDSize, sizeof(Ref));
  fontFileIDSize = 64;
  fontFileIDLen = 0;
  fontFileIDs = (Ref *)gmallocn(fontFileIDSize, sizeof(Ref));
  fontFileNameSize = 64;
  fontFileNameLen = 0;
  fontFileNames = (GooString **)gmallocn(fontFileNameSize, sizeof(GooString *));
  psFileNames = (GooString **)gmallocn(fontFileNameSize, sizeof(GooString *));
  nextTrueTypeNum = 0;
  font8InfoLen = 0;
  font8InfoSize = 0;
  font16EncLen = 0;
  font16EncSize = 0;
  imgIDLen = 0;
  imgIDSize = 0;
  formIDLen = 0;
  formIDSize = 0;

  xobjStack = new GooList();
  numSaves = 0;
  numTilingPatterns = 0;
  nextFunc = 0;

  // initialize embedded font resource comment list
  embFontList = new GooString();

  if (!manualCtrl) {
    Page *page;
    // this check is needed in case the document has zero pages
    if ((page = doc->getPage(firstPage))) {
      writeHeader(firstPage, lastPage,
		  page->getMediaBox(),
		  page->getCropBox(),
		  page->getRotate(),
		  pstitle);
    } else {
      error(-1, "Invalid page %d", firstPage);
      box = new PDFRectangle(0, 0, 1, 1);
      writeHeader(firstPage, lastPage, box, box, 0, pstitle);
      delete box;
    }
    if (mode != psModeForm) {
      writePS("%%BeginProlog\n");
    }
    writeXpdfProcset();
    if (mode != psModeForm) {
      writePS("%%EndProlog\n");
      writePS("%%BeginSetup\n");
    }
    writeDocSetup(doc, catalog, firstPage, lastPage, duplexA);
    if (mode != psModeForm) {
      writePS("%%EndSetup\n");
    }
  }

  // initialize sequential page number
  seqPage = 1;
}

PSOutputDev::~PSOutputDev() {
  PSOutCustomColor *cc;
  int i;

  if (ok) {
    if (!manualCtrl) {
      writePS("%%Trailer\n");
      writeTrailer();
      if (mode != psModeForm) {
	writePS("%%EOF\n");
      }
    }
    if (fileType == psFile) {
#ifdef MACOS
      ICS_MapRefNumAndAssign((short)((FILE *)outputStream)->handle);
#endif
      fclose((FILE *)outputStream);
    }
#ifdef HAVE_POPEN
    else if (fileType == psPipe) {
      pclose((FILE *)outputStream);
#ifndef _WIN32
      signal(SIGPIPE, (SignalFunc)SIG_DFL);
#endif
    }
#endif
  }
  if (embFontList) {
    delete embFontList;
  }
  if (fontIDs) {
    gfree(fontIDs);
  }
  if (fontFileIDs) {
    gfree(fontFileIDs);
  }
  if (fontFileNames) {
    for (i = 0; i < fontFileNameLen; ++i) {
      delete fontFileNames[i];
    }
    gfree(fontFileNames);
  }
  if (font8Info) {
    for (i = 0; i < font8InfoLen; ++i) {
      gfree(font8Info[i].codeToGID);
    }
    gfree(font8Info);
  }
  if (psFileNames) {
    for (i = 0; i < fontFileNameLen; ++i) {
      if (psFileNames[i])
        delete psFileNames[i];
    }
    gfree(psFileNames);
  }
  if (font16Enc) {
    for (i = 0; i < font16EncLen; ++i) {
      delete font16Enc[i].enc;
    }
    gfree(font16Enc);
  }
  gfree(imgIDs);
  gfree(formIDs);
  if (xobjStack) {
    delete xobjStack;
  }
  while (customColors) {
    cc = customColors;
    customColors = cc->next;
    delete cc;
  }
}

void PSOutputDev::writeHeader(int firstPage, int lastPage,
			      PDFRectangle *mediaBox, PDFRectangle *cropBox,
			      int pageRotate, char *psTitle) {
  double x1, y1, x2, y2;
  Object info, obj1;

  switch (mode) {
  case psModePSOrigPageSizes:
  case psModePS:
    writePS("%!PS-Adobe-3.0\n");
    break;
  case psModeEPS:
    writePS("%!PS-Adobe-3.0 EPSF-3.0\n");
    break;
  case psModeForm:
    writePS("%!PS-Adobe-3.0 Resource-Form\n");
    break;
  }
  xref->getDocInfo(&info);
  if (info.isDict() && info.dictLookup("Creator", &obj1)->isString()) {
    writePS("%%Creator: ");
    writePSTextLine(obj1.getString());
  }
  obj1.free();
  info.free();
  if(psTitle) {
    writePSFmt("%%Title: {0:s}\n", psTitle);
  }
  writePSFmt("%%LanguageLevel: {0:d}\n",
	     (level == psLevel1 || level == psLevel1Sep) ? 1 :
	     (level == psLevel2 || level == psLevel2Sep) ? 2 : 3);
  if (level == psLevel1Sep || level == psLevel2Sep || level == psLevel3Sep) {
    writePS("%%DocumentProcessColors: (atend)\n");
    writePS("%%DocumentCustomColors: (atend)\n");
  }
  writePS("%%DocumentSuppliedResources: (atend)\n");

  switch (mode) {
  case psModePSOrigPageSizes:
    prevWidth = 0;
    prevHeight = 0;
  case psModePS:
    writePSFmt("%%DocumentMedia: plain {0:d} {1:d} 0 () ()\n",
	       paperWidth, paperHeight);
    writePSFmt("%%BoundingBox: 0 0 {0:d} {1:d}\n", paperWidth, paperHeight);
    writePSFmt("%%Pages: {0:d}\n", lastPage - firstPage + 1);
    writePS("%%EndComments\n");
    writePS("%%BeginDefaults\n");
    writePS("%%PageMedia: plain\n");
    writePS("%%EndDefaults\n");
    break;
  case psModeEPS:
    epsX1 = cropBox->x1;
    epsY1 = cropBox->y1;
    epsX2 = cropBox->x2;
    epsY2 = cropBox->y2;
    if (pageRotate == 0 || pageRotate == 180) {
      x1 = epsX1;
      y1 = epsY1;
      x2 = epsX2;
      y2 = epsY2;
    } else { // pageRotate == 90 || pageRotate == 270
      x1 = 0;
      y1 = 0;
      x2 = epsY2 - epsY1;
      y2 = epsX2 - epsX1;
    }
    writePSFmt("%%BoundingBox: {0:d} {1:d} {2:d} {3:d}\n",
	       (int)floor(x1), (int)floor(y1), (int)ceil(x2), (int)ceil(y2));
    if (floor(x1) != ceil(x1) || floor(y1) != ceil(y1) ||
	floor(x2) != ceil(x2) || floor(y2) != ceil(y2)) {
      writePSFmt("%%HiResBoundingBox: {0:.6g} {1:.6g} {2:.6g} {3:.6g}\n",
		 x1, y1, x2, y2);
    }
    writePS("%%DocumentSuppliedResources: (atend)\n");
    writePS("%%EndComments\n");
    break;
  case psModeForm:
    writePS("%%EndComments\n");
    writePS("32 dict dup begin\n");
    writePSFmt("/BBox [{0:d} {1:d} {2:d} {3:d}] def\n",
	       (int)floor(mediaBox->x1), (int)floor(mediaBox->y1),
	       (int)ceil(mediaBox->x2), (int)ceil(mediaBox->y2));
    writePS("/FormType 1 def\n");
    writePS("/Matrix [1 0 0 1 0 0] def\n");
    break;
  }
}

void PSOutputDev::writeXpdfProcset() {
  GBool lev1, lev2, lev3, sep, nonSep;
  char **p;
  char *q;

  writePSFmt("%%BeginResource: procset xpdf {0:s} 0\n", "3.00");
  writePSFmt("%%Copyright: {0:s}\n", xpdfCopyright);
  lev1 = lev2 = lev3 = sep = nonSep = gTrue;
  for (p = prolog; *p; ++p) {
    if ((*p)[0] == '~') {
      lev1 = lev2 = lev3 = sep = nonSep = gFalse;
      for (q = *p + 1; *q; ++q) {
	switch (*q) {
	case '1': lev1 = gTrue; break;
	case '2': lev2 = gTrue; break;
	case '3': lev3 = gTrue; break;
	case 's': sep = gTrue; break;
	case 'n': nonSep = gTrue; break;
	}
      }
    } else if ((level == psLevel1 && lev1 && nonSep) ||
	       (level == psLevel1Sep && lev1 && sep) ||
	       (level == psLevel2 && lev2 && nonSep) ||
	       (level == psLevel2Sep && lev2 && sep) ||
	       (level == psLevel3 && lev3 && nonSep) ||
	       (level == psLevel3Sep && lev3 && sep)) {
      writePSFmt("{0:s}\n", *p);
    }
  }
  writePS("%%EndResource\n");

  if (level >= psLevel3) {
    for (p = cmapProlog; *p; ++p) {
      writePSFmt("{0:s}\n", *p);
    }
  }
}

void PSOutputDev::writeDocSetup(PDFDoc *doc, Catalog *catalog,
				int firstPage, int lastPage,
                                GBool duplexA) {
  Page *page;
  Dict *resDict;
  Annots *annots;
  Object obj1, obj2;
  int pg, i;

  if (mode == psModeForm) {
    // swap the form and xpdf dicts
    writePS("xpdf end begin dup begin\n");
  } else {
    writePS("xpdf begin\n");
  }
  for (pg = firstPage; pg <= lastPage; ++pg) {
    page = doc->getPage(pg);
    if (!page) {
      error(-1, "Failed writing resources for page %d", pg);
      continue;
    }
    if ((resDict = page->getResourceDict())) {
      setupResources(resDict);
    }
    annots = new Annots(xref, catalog, page->getAnnots(&obj1));
    obj1.free();
    for (i = 0; i < annots->getNumAnnots(); ++i) {
      if (annots->getAnnot(i)->getAppearance(&obj1)->isStream()) {
	obj1.streamGetDict()->lookup("Resources", &obj2);
	if (obj2.isDict()) {
	  setupResources(obj2.getDict());
	}
	obj2.free();
      }
      obj1.free();
    }
    delete annots;
  }
  if (mode != psModeForm) {
    if (mode != psModeEPS && !manualCtrl) {
      writePSFmt("{0:d} {1:d} {2:s} pdfSetup\n",
		 paperWidth, paperHeight, duplexA ? "true" : "false");
    }
#if OPI_SUPPORT
    if (globalParams->getPSOPI()) {
      writePS("/opiMatrix matrix currentmatrix def\n");
    }
#endif
  }
}

void PSOutputDev::writePageTrailer() {
  if (mode != psModeForm) {
    writePS("pdfEndPage\n");
  }
}

void PSOutputDev::writeTrailer() {
  PSOutCustomColor *cc;

  if (mode == psModeForm) {
    writePS("/Foo exch /Form defineresource pop\n");
  } else {
    writePS("end\n");
    writePS("%%DocumentSuppliedResources:\n");
    writePS(embFontList->getCString());
    if (level == psLevel1Sep || level == psLevel2Sep ||
	level == psLevel3Sep) {
      writePS("%%DocumentProcessColors:");
      if (processColors & psProcessCyan) {
	writePS(" Cyan");
	 }
      if (processColors & psProcessMagenta) {
	writePS(" Magenta");
      }
      if (processColors & psProcessYellow) {
	writePS(" Yellow");
      }
      if (processColors & psProcessBlack) {
	writePS(" Black");
      }
      writePS("\n");
      writePS("%%DocumentCustomColors:");
      for (cc = customColors; cc; cc = cc->next) {
	writePSFmt(" ({0:s})", cc->name->getCString());
      }
      writePS("\n");
      writePS("%%CMYKCustomColor:\n");
      for (cc = customColors; cc; cc = cc->next) {
	writePSFmt("%%+ {0:.4g} {1:.4g} {2:.4g} {3:.4g} ({4:t})\n",
		   cc->c, cc->m, cc->y, cc->k, cc->name);
      }
    }
  }
}

void PSOutputDev::setupResources(Dict *resDict) {
  Object xObjDict, xObjRef, xObj, patDict, patRef, pat, resObj;
  Ref ref0, ref1;
  GBool skip;
  int i, j;

  setupFonts(resDict);
  setupImages(resDict);
  setupForms(resDict);

  //----- recursively scan XObjects
  resDict->lookup("XObject", &xObjDict);
  if (xObjDict.isDict()) {
    for (i = 0; i < xObjDict.dictGetLength(); ++i) {

      // avoid infinite recursion on XObjects
      skip = gFalse;
      if ((xObjDict.dictGetValNF(i, &xObjRef)->isRef())) {
	ref0 = xObjRef.getRef();
	for (j = 0; j < xobjStack->getLength(); ++j) {
	  ref1 = *(Ref *)xobjStack->get(j);
	  if (ref1.num == ref0.num && ref1.gen == ref0.gen) {
	    skip = gTrue;
	    break;
	  }
	}
	if (!skip) {
	  xobjStack->append(&ref0);
	}
      }
      if (!skip) {

	// process the XObject's resource dictionary
	xObjDict.dictGetVal(i, &xObj);
	if (xObj.isStream()) {
	  xObj.streamGetDict()->lookup("Resources", &resObj);
	  if (resObj.isDict()) {
	    setupResources(resObj.getDict());
	  }
	  resObj.free();
	}
	xObj.free();
      }

      if (xObjRef.isRef() && !skip) {
	xobjStack->del(xobjStack->getLength() - 1);
      }
      xObjRef.free();
    }
  }
  xObjDict.free();

  //----- recursively scan Patterns
  resDict->lookup("Pattern", &patDict);
  if (patDict.isDict()) {
    inType3Char = gTrue;
    for (i = 0; i < patDict.dictGetLength(); ++i) {

      // avoid infinite recursion on Patterns
      skip = gFalse;
      if ((patDict.dictGetValNF(i, &patRef)->isRef())) {
	ref0 = patRef.getRef();
	for (j = 0; j < xobjStack->getLength(); ++j) {
	  ref1 = *(Ref *)xobjStack->get(j);
	  if (ref1.num == ref0.num && ref1.gen == ref0.gen) {
	    skip = gTrue;
	    break;
	  }
	}
	if (!skip) {
	  xobjStack->append(&ref0);
	}
      }
      if (!skip) {

	// process the Pattern's resource dictionary
	patDict.dictGetVal(i, &pat);
	if (pat.isStream()) {
	  pat.streamGetDict()->lookup("Resources", &resObj);
	  if (resObj.isDict()) {
	    setupResources(resObj.getDict());
	  }
	  resObj.free();
	}
	pat.free();
      }

      if (patRef.isRef() && !skip) {
	xobjStack->del(xobjStack->getLength() - 1);
      }
      patRef.free();
    }
    inType3Char = gFalse;
  }
  patDict.free();
}

void PSOutputDev::setupFonts(Dict *resDict) {
  Object obj1, obj2;
  Ref r;
  GfxFontDict *gfxFontDict;
  GfxFont *font;
  int i;

  if (forceRasterize) return;

  gfxFontDict = NULL;
  resDict->lookupNF("Font", &obj1);
  if (obj1.isRef()) {
    obj1.fetch(xref, &obj2);
    if (obj2.isDict()) {
      r = obj1.getRef();
      gfxFontDict = new GfxFontDict(xref, &r, obj2.getDict());
    }
    obj2.free();
  } else if (obj1.isDict()) {
    gfxFontDict = new GfxFontDict(xref, NULL, obj1.getDict());
  }
  if (gfxFontDict) {
    for (i = 0; i < gfxFontDict->getNumFonts(); ++i) {
      if ((font = gfxFontDict->getFont(i))) {
	setupFont(font, resDict);
      }
    }
    delete gfxFontDict;
  }
  obj1.free();
}

void PSOutputDev::setupFont(GfxFont *font, Dict *parentResDict) {
  Ref fontFileID;
  GooString *name;
  PSFontParam *fontParam;
  GooString *psName;
  char buf[16];
  GBool subst;
  UnicodeMap *uMap;
  char *charName;
  double xs, ys;
  int code;
  double w1, w2;
  double *fm;
  int i, j;
  DisplayFontParam *dfp;

  // check if font is already set up
  for (i = 0; i < fontIDLen; ++i) {
    if (fontIDs[i].num == font->getID()->num &&
	fontIDs[i].gen == font->getID()->gen) {
      return;
    }
  }

  // add entry to fontIDs list
  if (fontIDLen >= fontIDSize) {
    fontIDSize += 64;
    fontIDs = (Ref *)greallocn(fontIDs, fontIDSize, sizeof(Ref));
  }
  fontIDs[fontIDLen++] = *font->getID();

  xs = ys = 1;
  subst = gFalse;

  // check for resident 8-bit font
  if (font->getName() &&
      (fontParam = globalParams->getPSFont(font->getName()))) {
    psName = new GooString(fontParam->psFontName->getCString());

  // check for embedded Type 1 font
  } else if (globalParams->getPSEmbedType1() &&
	     font->getType() == fontType1 &&
	     font->getEmbeddedFontID(&fontFileID) &&
	     font->getEmbeddedFontName()) {
    psName = font->getEmbeddedFontName()->sanitizedName(gTrue /* ps mode */);
    setupEmbeddedType1Font(&fontFileID, psName);

  // check for embedded Type 1C font
  } else if (globalParams->getPSEmbedType1() &&
	     font->getType() == fontType1C &&
	     font->getEmbeddedFontID(&fontFileID) &&
	     font->getOrigName()) {
    // use the PDF font name because the embedded font name might
    // not include the subset prefix
    psName = font->getOrigName()->sanitizedName(gTrue /* ps mode */);
    setupEmbeddedType1CFont(font, &fontFileID, psName);

  // check for embedded OpenType - Type 1C font
  } else if (globalParams->getPSEmbedType1() &&
	     font->getType() == fontType1COT &&
	     font->getEmbeddedFontID(&fontFileID) &&
	     font->getOrigName()) {
    // use the PDF font name because the embedded font name might
    // not include the subset prefix
    psName = font->getOrigName()->sanitizedName(gTrue /* ps mode */);
    setupEmbeddedOpenTypeT1CFont(font, &fontFileID, psName);

  // check for external Type 1 font file
  } else if (globalParams->getPSEmbedType1() &&
	     font->getType() == fontType1 &&
	     font->getExtFontFile() &&
	     font->getName()) {
    // this assumes that the PS font name matches the PDF font name
    psName = font->getName()->copy();
    setupExternalType1Font(font->getExtFontFile(), psName);

  // check for embedded TrueType font
  } else if (globalParams->getPSEmbedTrueType() &&
	     (font->getType() == fontTrueType ||
	      font->getType() == fontTrueTypeOT) &&
	     font->getEmbeddedFontID(&fontFileID) &&
	     font->getEmbeddedFontName()) {
    psName = font->getEmbeddedFontName()->sanitizedName(gTrue /* ps mode */);
    setupEmbeddedTrueTypeFont(font, &fontFileID, psName);

  // check for external TrueType font file
  } else if (globalParams->getPSEmbedTrueType() &&
	     font->getType() == fontTrueType &&
	     font->getExtFontFile()) {
    psName = setupExternalTrueTypeFont(font);

  // check for embedded CID PostScript font
  } else if (globalParams->getPSEmbedCIDPostScript() &&
	     font->getType() == fontCIDType0C &&
	     font->getEmbeddedFontID(&fontFileID) &&
	     font->getEmbeddedFontName()) {
    psName = font->getEmbeddedFontName()->sanitizedName(gTrue /* ps mode */);
    setupEmbeddedCIDType0Font(font, &fontFileID, psName);

  // check for embedded CID TrueType font
  } else if (globalParams->getPSEmbedCIDTrueType() &&
	     (font->getType() == fontCIDType2 ||
	      font->getType() == fontCIDType2OT) &&
	     font->getEmbeddedFontID(&fontFileID) &&
	     font->getEmbeddedFontName()) {
    psName = font->getEmbeddedFontName()->sanitizedName(gTrue /* ps mode */);
    setupEmbeddedCIDTrueTypeFont(font, &fontFileID, psName, gTrue);

  // check for embedded OpenType - CID CFF font
  } else if (globalParams->getPSEmbedCIDPostScript() &&
	     font->getType() == fontCIDType0COT &&
	     font->getEmbeddedFontID(&fontFileID) &&
	     font->getEmbeddedFontName()) {
    psName = font->getEmbeddedFontName()->sanitizedName(gTrue /* ps mode */);
    setupEmbeddedOpenTypeCFFFont(font, &fontFileID, psName);

  // check for Type 3 font
  } else if (font->getType() == fontType3) {
    psName = GooString::format("T3_{0:d}_{1:d}",
			     font->getID()->num, font->getID()->gen);
    setupType3Font(font, psName, parentResDict);

  // check for external CID TrueType font file
  } else if (globalParams->getPSEmbedCIDTrueType() &&
	     font->getType() == fontCIDType2 &&
	     font->getExtFontFile()) {
    psName = setupExternalCIDTrueTypeFont(font, font->getExtFontFile());

  // do 8-bit font substitution
  } else if (!font->isCIDFont()) {
    subst = gTrue;
    name = font->getName();
    psName = NULL;
    if (name) {
      for (i = 0; psFonts[i]; ++i) {
	if (name->cmp(psFonts[i]) == 0) {
	  psName = new GooString(psFonts[i]);
	  break;
	}
      }
    }
    if (!psName) {
      if (substFonts) {
	if (font->isFixedWidth()) {
	  i = 8;
	} else if (font->isSerif()) {
	  i = 4;
	} else {
	  i = 0;
	}
	if (font->isBold()) {
	  i += 2;
	}
	if (font->isItalic()) {
	  i += 1;
	}
	psName = new GooString(psSubstFonts[i].psName);
	for (code = 0; code < 256; ++code) {
	  if ((charName = ((Gfx8BitFont *)font)->getCharName(code)) &&
	      charName[0] == 'm' && charName[1] == '\0') {
	    break;
	  }
	}
	if (code < 256) {
	  w1 = ((Gfx8BitFont *)font)->getWidth(code);
	} else {
	  w1 = 0;
	}
	w2 = psSubstFonts[i].mWidth;
	xs = w1 / w2;
	if (xs < 0.1) {
	  xs = 1;
	}
      } else {
	psName = new GooString(name);
	xs = 1;
      }
      if (font->getType() == fontType3) {
	// This is a hack which makes it possible to substitute for some
	// Type 3 fonts.  The problem is that it's impossible to know what
	// the base coordinate system used in the font is without actually
	// rendering the font.
	ys = xs;
	fm = font->getFontMatrix();
	if (fm[0] != 0) {
	  ys *= fm[3] / fm[0];
	}
      } else {
	ys = 1;
      }
    }

  // do 16-bit font substitution
  } else if ((fontParam = globalParams->
	        getPSFont16(font->getName(),
			    ((GfxCIDFont *)font)->getCollection(),
			    font->getWMode()))) {
    subst = gTrue;
    psName = fontParam->psFontName->copy();
    if (font16EncLen >= font16EncSize) {
      font16EncSize += 16;
      font16Enc = (PSFont16Enc *)greallocn(font16Enc,
					   font16EncSize, sizeof(PSFont16Enc));
    }
    font16Enc[font16EncLen].fontID = *font->getID();
    font16Enc[font16EncLen].enc = fontParam->encoding->copy();
    if ((uMap = globalParams->getUnicodeMap(font16Enc[font16EncLen].enc))) {
      uMap->decRefCnt();
      ++font16EncLen;
    } else {
      error(-1, "Couldn't find Unicode map for 16-bit font encoding '%s'",
	    font16Enc[font16EncLen].enc->getCString());
    }

    // try the display font for embedding
  } else if (globalParams->getPSEmbedCIDTrueType() &&
	     ((GfxCIDFont *)font)->getCollection() &&
	     (dfp = globalParams->
	      getDisplayFont(font)) &&
	     dfp->kind == displayFontTT) {
    psName = setupExternalCIDTrueTypeFont(font, dfp->tt.fileName, dfp->tt.faceIndex);

  // give up - can't do anything with this font
  } else {
    error(-1, "Couldn't find a font to substitute for '%s' ('%s' character collection)",
	  font->getName() ? font->getName()->getCString() : "(unnamed)",
	  ((GfxCIDFont *)font)->getCollection()
	    ? ((GfxCIDFont *)font)->getCollection()->getCString()
	    : "(unknown)");
    return;
  }

  // generate PostScript code to set up the font
  if (font->isCIDFont()) {
    if (level == psLevel3 || level == psLevel3Sep) {
      writePSFmt("/F{0:d}_{1:d} /{2:t} {3:d} pdfMakeFont16L3\n",
		 font->getID()->num, font->getID()->gen, psName,
		 font->getWMode());
    } else {
      writePSFmt("/F{0:d}_{1:d} /{2:t} {3:d} pdfMakeFont16\n",
		 font->getID()->num, font->getID()->gen, psName,
		 font->getWMode());
    }
  } else {
    writePSFmt("/F{0:d}_{1:d} /{2:t} {3:.6g} {4:.6g}\n",
	       font->getID()->num, font->getID()->gen, psName, xs, ys);
    for (i = 0; i < 256; i += 8) {
      writePS((char *)((i == 0) ? "[ " : "  "));
      for (j = 0; j < 8; ++j) {
	if (font->getType() == fontTrueType &&
	    !subst &&
	    !((Gfx8BitFont *)font)->getHasEncoding()) {
	  sprintf(buf, "c%02x", i+j);
	  charName = buf;
	} else {
	  charName = ((Gfx8BitFont *)font)->getCharName(i+j);
	  // this is a kludge for broken PDF files that encode char 32
	  // as .notdef
	  if (i+j == 32 && charName && !strcmp(charName, ".notdef")) {
	    charName = "space";
	  }
	}
	writePS("/");
	writePSName(charName ? charName : (char *)".notdef");
	// the empty name is legal in PDF and PostScript, but PostScript
	// uses a double-slash (//...) for "immediately evaluated names",
	// so we need to add a space character here
	if (charName && !charName[0]) {
	  writePS(" ");
	}
      }
      writePS((i == 256-8) ? (char *)"]\n" : (char *)"\n");
    }
    writePS("pdfMakeFont\n");
  }

  delete psName;
}

void PSOutputDev::setupEmbeddedType1Font(Ref *id, GooString *psName) {
  static const char hexChar[17] = "0123456789abcdef";
  Object refObj, strObj, obj1, obj2, obj3;
  Dict *dict;
  int length1, length2, length3;
  int c;
  int start[4];
  GBool binMode;
  GBool writePadding = gTrue;
  int i;

  // check if font is already embedded
  for (i = 0; i < fontFileIDLen; ++i) {
    if (fontFileIDs[i].num == id->num &&
	fontFileIDs[i].gen == id->gen)
      return;
  }

  // add entry to fontFileIDs list
  if (fontFileIDLen >= fontFileIDSize) {
    fontFileIDSize += 64;
    fontFileIDs = (Ref *)greallocn(fontFileIDs, fontFileIDSize, sizeof(Ref));
  }
  fontFileIDs[fontFileIDLen++] = *id;

  // get the font stream and info
  refObj.initRef(id->num, id->gen);
  refObj.fetch(xref, &strObj);
  refObj.free();
  if (!strObj.isStream()) {
    error(-1, "Embedded font file object is not a stream");
    goto err1;
  }
  if (!(dict = strObj.streamGetDict())) {
    error(-1, "Embedded font stream is missing its dictionary");
    goto err1;
  }
  dict->lookup("Length1", &obj1);
  dict->lookup("Length2", &obj2);
  dict->lookup("Length3", &obj3);
  if (!obj1.isInt() || !obj2.isInt() || !obj3.isInt()) {
    error(-1, "Missing length fields in embedded font stream dictionary");
    obj1.free();
    obj2.free();
    obj3.free();
    goto err1;
  }
  length1 = obj1.getInt();
  length2 = obj2.getInt();
  length3 = obj3.getInt();
  obj1.free();
  obj2.free();
  obj3.free();

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // copy ASCII portion of font
  strObj.streamReset();
  for (i = 0; i < length1 && (c = strObj.streamGetChar()) != EOF; ++i) {
    writePSChar(c);
  }

  // figure out if encrypted portion is binary or ASCII
  binMode = gFalse;
  for (i = 0; i < 4; ++i) {
    start[i] = strObj.streamGetChar();
    if (start[i] == EOF) {
      error(-1, "Unexpected end of file in embedded font stream");
      goto err1;
    }
    if (!((start[i] >= '0' && start[i] <= '9') ||
	  (start[i] >= 'A' && start[i] <= 'F') ||
	  (start[i] >= 'a' && start[i] <= 'f')))
      binMode = gTrue;
  }

  if (length2 == 0)
  {
    // length2 == 0 is an error
    // trying to solve it by just piping all
    // the stream data
    error(-1, "Font has length2 as 0, trying to overcome the problem reading the stream until the end");
    length2 = INT_MAX;
    writePadding = gFalse;
  }


  // convert binary data to ASCII
  if (binMode) {
    for (i = 0; i < 4; ++i) {
      writePSChar(hexChar[(start[i] >> 4) & 0x0f]);
      writePSChar(hexChar[start[i] & 0x0f]);
    }
#if 0 // this causes trouble for various PostScript printers
    // if Length2 is incorrect (too small), font data gets chopped, so
    // we take a few extra characters from the trailer just in case
    length2 += length3 >= 8 ? 8 : length3;
#endif
    while (i < length2) {
      if ((c = strObj.streamGetChar()) == EOF) {
	break;
      }
      writePSChar(hexChar[(c >> 4) & 0x0f]);
      writePSChar(hexChar[c & 0x0f]);
      if (++i % 32 == 0) {
	writePSChar('\n');
      }
    }
    if (i % 32 > 0) {
      writePSChar('\n');
    }

  // already in ASCII format -- just copy it
  } else {
    for (i = 0; i < 4; ++i) {
      writePSChar(start[i]);
    }
    for (i = 4; i < length2; ++i) {
      if ((c = strObj.streamGetChar()) == EOF) {
	break;
      }
      writePSChar(c);
    }
  }

  if (writePadding)
  {
    if (length3 > 0) {
      // write fixed-content portion
      while ((c = strObj.streamGetChar()) != EOF) {
	writePSChar(c);
      }
    } else {
      // write padding and "cleartomark"
      for (i = 0; i < 8; ++i) {
	writePS("00000000000000000000000000000000"
		"00000000000000000000000000000000\n");
      }
      writePS("cleartomark\n");
    }
  }

  // ending comment
  writePS("%%EndResource\n");

 err1:
  strObj.streamClose();
  strObj.free();
}

//~ This doesn't handle .pfb files or binary eexec data (which only
//~ happens in pfb files?).
void PSOutputDev::setupExternalType1Font(GooString *fileName, GooString *psName) {
  FILE *fontFile;
  int c;
  int i;

  // check if font is already embedded
  for (i = 0; i < fontFileNameLen; ++i) {
    if (!fontFileNames[i]->cmp(fileName)) {
      return;
    }
  }

  // add entry to fontFileNames list
  if (fontFileNameLen >= fontFileNameSize) {
    fontFileNameSize += 64;
    fontFileNames = (GooString **)greallocn(fontFileNames,
					  fontFileNameSize, sizeof(GooString *));
    psFileNames = (GooString **)greallocn(psFileNames,
				       fontFileNameSize, sizeof(GooString *));
  }
  fontFileNames[fontFileNameLen] = fileName->copy();
  psFileNames[fontFileNameLen] = psName->copy();
  fontFileNameLen++;

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // copy the font file
  if (!(fontFile = fopen(fileName->getCString(), "rb"))) {
    error(-1, "Couldn't open external font file");
    return;
  }
  while ((c = fgetc(fontFile)) != EOF) {
    writePSChar(c);
  }
  fclose(fontFile);

  // ending comment
  writePS("%%EndResource\n");
}

void PSOutputDev::setupEmbeddedType1CFont(GfxFont *font, Ref *id,
					  GooString *psName) {
  char *fontBuf;
  int fontLen;
  FoFiType1C *ffT1C;
  int i;

  // check if font is already embedded
  for (i = 0; i < fontFileIDLen; ++i) {
    if (fontFileIDs[i].num == id->num &&
	fontFileIDs[i].gen == id->gen)
      return;
  }

  // add entry to fontFileIDs list
  if (fontFileIDLen >= fontFileIDSize) {
    fontFileIDSize += 64;
    fontFileIDs = (Ref *)greallocn(fontFileIDs, fontFileIDSize, sizeof(Ref));
  }
  fontFileIDs[fontFileIDLen++] = *id;

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 1 font
  fontBuf = font->readEmbFontFile(xref, &fontLen);
  if ((ffT1C = FoFiType1C::make(fontBuf, fontLen))) {
    ffT1C->convertToType1(psName->getCString(), NULL, gTrue,
			  outputFunc, outputStream);
    delete ffT1C;
  }
  gfree(fontBuf);

  // ending comment
  writePS("%%EndResource\n");
}

void PSOutputDev::setupEmbeddedOpenTypeT1CFont(GfxFont *font, Ref *id,
					       GooString *psName) {
  char *fontBuf;
  int fontLen;
  FoFiTrueType *ffTT;
  int i;

  // check if font is already embedded
  for (i = 0; i < fontFileIDLen; ++i) {
    if (fontFileIDs[i].num == id->num &&
	fontFileIDs[i].gen == id->gen)
      return;
  }

  // add entry to fontFileIDs list
  if (fontFileIDLen >= fontFileIDSize) {
    fontFileIDSize += 64;
    fontFileIDs = (Ref *)greallocn(fontFileIDs, fontFileIDSize, sizeof(Ref));
  }
  fontFileIDs[fontFileIDLen++] = *id;

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 1 font
  fontBuf = font->readEmbFontFile(xref, &fontLen);
  if ((ffTT = FoFiTrueType::make(fontBuf, fontLen))) {
    if (ffTT->isOpenTypeCFF()) {
      ffTT->convertToType1(psName->getCString(), NULL, gTrue,
			   outputFunc, outputStream);
    }
    delete ffTT;
  }
  gfree(fontBuf);

  // ending comment
  writePS("%%EndResource\n");
}

void PSOutputDev::setupEmbeddedTrueTypeFont(GfxFont *font, Ref *id,
					    GooString *psName) {
  char *fontBuf;
  int fontLen;
  FoFiTrueType *ffTT;
  Gushort *codeToGID;
  int i;

  // check if font is already embedded
  for (i = 0; i < fontFileIDLen; ++i) {
    if (fontFileIDs[i].num == id->num &&
	fontFileIDs[i].gen == id->gen) {
      psName->appendf("_{0:d}", nextTrueTypeNum++);
      break;
    }
  }

  // add entry to fontFileIDs list
  if (i == fontFileIDLen) {
    if (fontFileIDLen >= fontFileIDSize) {
      fontFileIDSize += 64;
      fontFileIDs = (Ref *)greallocn(fontFileIDs, fontFileIDSize, sizeof(Ref));
    }
    fontFileIDs[fontFileIDLen++] = *id;
  }

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 42 font
  fontBuf = font->readEmbFontFile(xref, &fontLen);
  if ((ffTT = FoFiTrueType::make(fontBuf, fontLen))) {
    codeToGID = ((Gfx8BitFont *)font)->getCodeToGIDMap(ffTT);
    ffTT->convertToType42(psName->getCString(),
			  ((Gfx8BitFont *)font)->getHasEncoding()
			    ? ((Gfx8BitFont *)font)->getEncoding()
			    : (char **)NULL,
			  codeToGID, outputFunc, outputStream);
    if (codeToGID) {
      if (font8InfoLen >= font8InfoSize) {
	font8InfoSize += 16;
	font8Info = (PSFont8Info *)greallocn(font8Info,
					     font8InfoSize,
					     sizeof(PSFont8Info));
      }
      font8Info[font8InfoLen].fontID = *font->getID();
      font8Info[font8InfoLen].codeToGID = codeToGID;
      ++font8InfoLen;
    }
    delete ffTT;
  }
  gfree(fontBuf);

  // ending comment
  writePS("%%EndResource\n");
}

GooString *PSOutputDev::setupExternalTrueTypeFont(GfxFont *font) {
  GooString *fileName;
  char *fontBuf;
  int fontLen;
  FoFiTrueType *ffTT;
  Gushort *codeToGID;
  GooString *psName;
  int i;

  // check if font is already embedded
  fileName = font->getExtFontFile();
  for (i = 0; i < fontFileNameLen; ++i) {
    if (!fontFileNames[i]->cmp(fileName)) {
      return psFileNames[i]->copy();
    }
  }

  psName = font->getName()->sanitizedName(gTrue /* ps mode */);
  // add entry to fontFileNames list
  if (i == fontFileNameLen) {
    if (fontFileNameLen >= fontFileNameSize) {
      fontFileNameSize += 64;
      fontFileNames =
	(GooString **)greallocn(fontFileNames,
			      fontFileNameSize, sizeof(GooString *));
      psFileNames =
	(GooString **)greallocn(psFileNames,
			     fontFileNameSize, sizeof(GooString *));
    }
    fontFileNames[fontFileNameLen] = fileName->copy();
    psFileNames[fontFileNameLen] = psName->copy();
    fontFileNameLen++;
  }

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 42 font
  fontBuf = font->readExtFontFile(&fontLen);
  if ((ffTT = FoFiTrueType::make(fontBuf, fontLen))) {
    codeToGID = ((Gfx8BitFont *)font)->getCodeToGIDMap(ffTT);
    ffTT->convertToType42(psName->getCString(),
			  ((Gfx8BitFont *)font)->getHasEncoding()
			    ? ((Gfx8BitFont *)font)->getEncoding()
			    : (char **)NULL,
			  codeToGID, outputFunc, outputStream);
    if (codeToGID) {
      if (font8InfoLen >= font8InfoSize) {
	font8InfoSize += 16;
	font8Info = (PSFont8Info *)greallocn(font8Info,
					     font8InfoSize,
					     sizeof(PSFont8Info));
      }
      font8Info[font8InfoLen].fontID = *font->getID();
      font8Info[font8InfoLen].codeToGID = codeToGID;
      ++font8InfoLen;
    }
    delete ffTT;
  }
  gfree(fontBuf);

  // ending comment
  writePS("%%EndResource\n");
  return psName;
}

GooString *PSOutputDev::setupExternalCIDTrueTypeFont(GfxFont *font, GooString *fileName, int faceIndex) {
  FoFiTrueType *ffTT;
  Gushort *codeToGID;
  GooString *psName;
  int i;
  GooString *myFileName;

  myFileName = fileName->copy();
  if (faceIndex > 0) {
    char tmp[32];
    sprintf(tmp, ",%d", faceIndex);
    myFileName->append(tmp);
  }
  // check if font is already embedded
  for (i = 0; i < fontFileNameLen; ++i) {
    if (!fontFileNames[i]->cmp(myFileName)) {
      delete myFileName;
      return psFileNames[i]->copy();
    }
  }

  psName = font->getName()->sanitizedName(gTrue /* ps mode */);
  // add entry to fontFileNames list
  if (i == fontFileNameLen) {
    if (fontFileNameLen >= fontFileNameSize) {
      fontFileNameSize += 64;
      fontFileNames =
	(GooString **)grealloc(fontFileNames,
			     fontFileNameSize * sizeof(GooString *));
      psFileNames =
	(GooString **)grealloc(psFileNames,
			     fontFileNameSize * sizeof(GooString *));
    }
  }
  fontFileNames[fontFileNameLen] = myFileName;
  psFileNames[fontFileNameLen] = psName->copy();
  fontFileNameLen++;

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a CID type2 font
  if ((ffTT = FoFiTrueType::load(fileName->getCString(), faceIndex))) {
      int n = ((GfxCIDFont *)font)->getCIDToGIDLen();
      if (n) {
	codeToGID = (Gushort *)gmallocn(n, sizeof(Gushort));
	memcpy(codeToGID, ((GfxCIDFont *)font)->getCIDToGID(), n * sizeof(Gushort));
      } else {
	codeToGID = ((GfxCIDFont *)font)->getCodeToGIDMap(ffTT, &n);
      }
      if (globalParams->getPSLevel() >= psLevel3) {
	// Level 3: use a CID font
	ffTT->convertToCIDType2(psName->getCString(),
				codeToGID, n, gTrue,
				outputFunc, outputStream);
      } else {
	// otherwise: use a non-CID composite font
	ffTT->convertToType0(psName->getCString(),
			     codeToGID, n, gTrue,
			     outputFunc, outputStream);
      }
      gfree(codeToGID);
      delete ffTT;
  }

  // ending comment
  writePS("%%EndResource\n");
  return psName;
}

void PSOutputDev::setupEmbeddedCIDType0Font(GfxFont *font, Ref *id,
					    GooString *psName) {
  char *fontBuf;
  int fontLen;
  FoFiType1C *ffT1C;
  int i;

  // check if font is already embedded
  for (i = 0; i < fontFileIDLen; ++i) {
    if (fontFileIDs[i].num == id->num &&
	fontFileIDs[i].gen == id->gen)
      return;
  }

  // add entry to fontFileIDs list
  if (fontFileIDLen >= fontFileIDSize) {
    fontFileIDSize += 64;
    fontFileIDs = (Ref *)greallocn(fontFileIDs, fontFileIDSize, sizeof(Ref));
  }
  fontFileIDs[fontFileIDLen++] = *id;

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 0 font
  fontBuf = font->readEmbFontFile(xref, &fontLen);
  if ((ffT1C = FoFiType1C::make(fontBuf, fontLen))) {
    if (globalParams->getPSLevel() >= psLevel3) {
      // Level 3: use a CID font
      ffT1C->convertToCIDType0(psName->getCString(), outputFunc, outputStream);
    } else {
      // otherwise: use a non-CID composite font
      ffT1C->convertToType0(psName->getCString(), outputFunc, outputStream);
    }
    delete ffT1C;
  }
  gfree(fontBuf);

  // ending comment
  writePS("%%EndResource\n");
}

void PSOutputDev::setupEmbeddedCIDTrueTypeFont(GfxFont *font, Ref *id,
					       GooString *psName,
					       GBool needVerticalMetrics) {
  char *fontBuf;
  int fontLen;
  FoFiTrueType *ffTT;
  int i;

  // check if font is already embedded
  for (i = 0; i < fontFileIDLen; ++i) {
    if (fontFileIDs[i].num == id->num &&
	fontFileIDs[i].gen == id->gen) {
      psName->appendf("_{0:d}", nextTrueTypeNum++);
      break;
    }
  }

  // add entry to fontFileIDs list
  if (fontFileIDLen >= fontFileIDSize) {
    fontFileIDSize += 64;
    fontFileIDs = (Ref *)greallocn(fontFileIDs, fontFileIDSize, sizeof(Ref));
  }
  fontFileIDs[fontFileIDLen++] = *id;

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 0 font
  fontBuf = font->readEmbFontFile(xref, &fontLen);
  if ((ffTT = FoFiTrueType::make(fontBuf, fontLen))) {
    if (globalParams->getPSLevel() >= psLevel3) {
      // Level 3: use a CID font
      ffTT->convertToCIDType2(psName->getCString(),
			      ((GfxCIDFont *)font)->getCIDToGID(),
			      ((GfxCIDFont *)font)->getCIDToGIDLen(),
			      needVerticalMetrics,
			      outputFunc, outputStream);
    } else {
      // otherwise: use a non-CID composite font
      ffTT->convertToType0(psName->getCString(),
			   ((GfxCIDFont *)font)->getCIDToGID(),
			   ((GfxCIDFont *)font)->getCIDToGIDLen(),
			   needVerticalMetrics,
			   outputFunc, outputStream);
    }
    delete ffTT;
  }
  gfree(fontBuf);

  // ending comment
  writePS("%%EndResource\n");
}

void PSOutputDev::setupEmbeddedOpenTypeCFFFont(GfxFont *font, Ref *id,
					       GooString *psName) {
  char *fontBuf;
  int fontLen;
  FoFiTrueType *ffTT;
  int i;

  // check if font is already embedded
  for (i = 0; i < fontFileIDLen; ++i) {
    if (fontFileIDs[i].num == id->num &&
	fontFileIDs[i].gen == id->gen)
      return;
  }

  // add entry to fontFileIDs list
  if (fontFileIDLen >= fontFileIDSize) {
    fontFileIDSize += 64;
    fontFileIDs = (Ref *)greallocn(fontFileIDs, fontFileIDSize, sizeof(Ref));
  }
  fontFileIDs[fontFileIDLen++] = *id;

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 0 font
  fontBuf = font->readEmbFontFile(xref, &fontLen);
  if ((ffTT = FoFiTrueType::make(fontBuf, fontLen))) {
    if (ffTT->isOpenTypeCFF()) {
      if (globalParams->getPSLevel() >= psLevel3) {
	// Level 3: use a CID font
	ffTT->convertToCIDType0(psName->getCString(),
				outputFunc, outputStream);
      } else {
	// otherwise: use a non-CID composite font
	ffTT->convertToType0(psName->getCString(), outputFunc, outputStream);
      }
    }
    delete ffTT;
  }
  gfree(fontBuf);

  // ending comment
  writePS("%%EndResource\n");
}

void PSOutputDev::setupType3Font(GfxFont *font, GooString *psName,
				 Dict *parentResDict) {
  Dict *resDict;
  Dict *charProcs;
  Object charProc;
  Gfx *gfx;
  PDFRectangle box;
  double *m;
  GooString *buf;
  int i;

  // set up resources used by font
  if ((resDict = ((Gfx8BitFont *)font)->getResources())) {
    inType3Char = gTrue;
    setupResources(resDict);
    inType3Char = gFalse;
  } else {
    resDict = parentResDict;
  }

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // font dictionary
  writePS("8 dict begin\n");
  writePS("/FontType 3 def\n");
  m = font->getFontMatrix();
  writePSFmt("/FontMatrix [{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] def\n",
	     m[0], m[1], m[2], m[3], m[4], m[5]);
  m = font->getFontBBox();
  writePSFmt("/FontBBox [{0:.6g} {1:.6g} {2:.6g} {3:.6g}] def\n",
	     m[0], m[1], m[2], m[3]);
  writePS("/Encoding 256 array def\n");
  writePS("  0 1 255 { Encoding exch /.notdef put } for\n");
  writePS("/BuildGlyph {\n");
  writePS("  exch /CharProcs get exch\n");
  writePS("  2 copy known not { pop /.notdef } if\n");
  writePS("  get exec\n");
  writePS("} bind def\n");
  writePS("/BuildChar {\n");
  writePS("  1 index /Encoding get exch get\n");
  writePS("  1 index /BuildGlyph get exec\n");
  writePS("} bind def\n");
  if ((charProcs = ((Gfx8BitFont *)font)->getCharProcs())) {
    writePSFmt("/CharProcs {0:d} dict def\n", charProcs->getLength());
    writePS("CharProcs begin\n");
    box.x1 = m[0];
    box.y1 = m[1];
    box.x2 = m[2];
    box.y2 = m[3];
    gfx = new Gfx(xref, this, resDict, m_catalog, &box, NULL);
    inType3Char = gTrue;
    for (i = 0; i < charProcs->getLength(); ++i) {
      t3Cacheable = gFalse;
      t3NeedsRestore = gFalse;
      writePS("/");
      writePSName(charProcs->getKey(i));
      writePS(" {\n");
      gfx->display(charProcs->getVal(i, &charProc));
      charProc.free();
      if (t3String) {
	if (t3Cacheable) {
	  buf = GooString::format("{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g} setcachedevice\n",
				t3WX, t3WY, t3LLX, t3LLY, t3URX, t3URY);
	} else {
	  buf = GooString::format("{0:.6g} {1:.6g} setcharwidth\n", t3WX, t3WY);
	}
	(*outputFunc)(outputStream, buf->getCString(), buf->getLength());
	delete buf;
	(*outputFunc)(outputStream, t3String->getCString(),
		      t3String->getLength());
	delete t3String;
	t3String = NULL;
      }
      if (t3NeedsRestore) {
	(*outputFunc)(outputStream, "Q\n", 2);
      }
      writePS("} def\n");
    }
    inType3Char = gFalse;
    delete gfx;
    writePS("end\n");
  }
  writePS("currentdict end\n");
  writePSFmt("/{0:t} exch definefont pop\n", psName);

  // ending comment
  writePS("%%EndResource\n");
}

void PSOutputDev::setupImages(Dict *resDict) {
  Object xObjDict, xObj, xObjRef, subtypeObj;
  int i;

  if (!(mode == psModeForm || inType3Char || preload)) {
    return;
  }

  //----- recursively scan XObjects
  resDict->lookup("XObject", &xObjDict);
  if (xObjDict.isDict()) {
    for (i = 0; i < xObjDict.dictGetLength(); ++i) {
      xObjDict.dictGetValNF(i, &xObjRef);
      xObjDict.dictGetVal(i, &xObj);
      if (xObj.isStream()) {
	xObj.streamGetDict()->lookup("Subtype", &subtypeObj);
	if (subtypeObj.isName("Image")) {
	  if (xObjRef.isRef()) {
	    setupImage(xObjRef.getRef(), xObj.getStream());
	  } else {
	    error(-1, "Image in resource dict is not an indirect reference");
	  }
	}
	subtypeObj.free();
      }
      xObj.free();
      xObjRef.free();
    }
  }
  xObjDict.free();
}

void PSOutputDev::setupImage(Ref id, Stream *str) {
  GBool useRLE, useCompressed, useASCIIHex;
  GooString *s;
  int c;
  int size, line, col, i;
  int outerSize, outer;

  // check if image is already setup
  for (i = 0; i < imgIDLen; ++i) {
    if (imgIDs[i].num == id.num && imgIDs[i].gen == id.gen) {
      return;
    }
  }

  // add entry to imgIDs list
  if (imgIDLen >= imgIDSize) {
    if (imgIDSize == 0) {
      imgIDSize = 64;
    } else {
      imgIDSize *= 2;
    }
    imgIDs = (Ref *)greallocn(imgIDs, imgIDSize, sizeof(Ref));
  }
  imgIDs[imgIDLen++] = id;

  // filters
  //~ this does not correctly handle the DeviceN color space
  //~   -- need to use DeviceNRecoder
  if (level < psLevel2) {
    useRLE = gFalse;
    useCompressed = gFalse;
    useASCIIHex = gTrue;
  } else {
    s = str->getPSFilter(level < psLevel3 ? 2 : 3, "");
    if (s) {
      useRLE = gFalse;
      useCompressed = gTrue;
      delete s;
    } else {
      useRLE = gTrue;
      useCompressed = gFalse;
    }
    useASCIIHex = level == psLevel1 || level == psLevel1Sep ||
                  globalParams->getPSASCIIHex();
  }
  if (useCompressed) {
    str = str->getUndecodedStream();
  }
  if (useRLE) {
    str = new RunLengthEncoder(str);
  }
  if (useASCIIHex) {
    str = new ASCIIHexEncoder(str);
  } else {
    str = new ASCII85Encoder(str);
  }

  // compute image data size
  str->reset();
  col = size = 0;
  do {
    do {
      c = str->getChar();
    } while (c == '\n' || c == '\r');
    if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
      break;
    }
    if (c == 'z') {
      ++col;
    } else {
      ++col;
      for (i = 1; i <= (useASCIIHex ? 1 : 4); ++i) {
	do {
	  c = str->getChar();
	} while (c == '\n' || c == '\r');
	if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	  break;
	}
	++col;
      }
      if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	break;
      }
    }
    if (col > 225) {
      ++size;
      col = 0;
    }
  } while (c != (useASCIIHex ? '>' : '~') && c != EOF);
  // add one entry for the final line of data; add another entry
  // because the RunLengthDecode filter may read past the end
  ++size;
  if (useRLE) {
    ++size;
  }
  outerSize = size/65535 + 1;

  writePSFmt("{0:d} array dup /ImData_{1:d}_{2:d} exch def\n",
	     outerSize, id.num, id.gen);
  str->close();

  // write the data into the array
  str->reset();
  for (outer = 0;outer < outerSize;outer++) {
    int innerSize = size > 65535 ? 65535 : size;

    // put the inner array into the outer array
    writePSFmt("{0:d} array 1 index {1:d} 2 index put\n",
	       innerSize, outer);
    line = col = 0;
    writePS((char *)(useASCIIHex ? "dup 0 <" : "dup 0 <~"));
    for (;;) {
      do {
	c = str->getChar();
      } while (c == '\n' || c == '\r');
      if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	break;
      }
      if (c == 'z') {
	writePSChar(c);
	++col;
      } else {
	writePSChar(c);
	++col;
	for (i = 1; i <= (useASCIIHex ? 1 : 4); ++i) {
	  do {
	    c = str->getChar();
	  } while (c == '\n' || c == '\r');
	  if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	    break;
	  }
	  writePSChar(c);
	  ++col;
	}
      }
      if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	break;
      }
      // each line is: "dup nnnnn <~...data...~> put<eol>"
      // so max data length = 255 - 20 = 235
      // chunks are 1 or 4 bytes each, so we have to stop at 232
      // but make it 225 just to be safe
      if (col > 225) {
	writePS((char *)(useASCIIHex ? "> put\n" : "~> put\n"));
	++line;
	if (line >= innerSize) break;
	writePSFmt((char *)(useASCIIHex ? "dup {0:d} <" : "dup {0:d} <~"), line);
	col = 0;
      }
    }
    if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
      writePS((char *)(useASCIIHex ? "> put\n" : "~> put\n"));
      if (useRLE) {
	++line;
	writePSFmt("{0:d} <> put\n", line);
      } else {
	writePS("pop\n");
      }
      break;
    }
    writePS("pop\n");
    size -= innerSize;
  }
  writePS("pop\n");
  str->close();

  delete str;
}

void PSOutputDev::setupForms(Dict *resDict) {
  Object xObjDict, xObj, xObjRef, subtypeObj;
  int i;

  if (!preload) {
    return;
  }

  resDict->lookup("XObject", &xObjDict);
  if (xObjDict.isDict()) {
    for (i = 0; i < xObjDict.dictGetLength(); ++i) {
      xObjDict.dictGetValNF(i, &xObjRef);
      xObjDict.dictGetVal(i, &xObj);
      if (xObj.isStream()) {
	xObj.streamGetDict()->lookup("Subtype", &subtypeObj);
	if (subtypeObj.isName("Form")) {
	  if (xObjRef.isRef()) {
	    setupForm(xObjRef.getRef(), &xObj);
	  } else {
	    error(-1, "Form in resource dict is not an indirect reference");
	  }
	}
	subtypeObj.free();
      }
      xObj.free();
      xObjRef.free();
    }
  }
  xObjDict.free();
}

void PSOutputDev::setupForm(Ref id, Object *strObj) {
  Dict *dict, *resDict;
  Object matrixObj, bboxObj, resObj, obj1;
  double m[6], bbox[4];
  PDFRectangle box;
  Gfx *gfx;
  int i;

  // check if form is already defined
  for (i = 0; i < formIDLen; ++i) {
    if (formIDs[i].num == id.num && formIDs[i].gen == id.gen) {
      return;
    }
  }

  // add entry to formIDs list
  if (formIDLen >= formIDSize) {
    if (formIDSize == 0) {
      formIDSize = 64;
    } else {
      formIDSize *= 2;
    }
    formIDs = (Ref *)greallocn(formIDs, formIDSize, sizeof(Ref));
  }
  formIDs[formIDLen++] = id;

  dict = strObj->streamGetDict();

  // get bounding box
  dict->lookup("BBox", &bboxObj);
  if (!bboxObj.isArray()) {
    bboxObj.free();
    error(-1, "Bad form bounding box");
    return;
  }
  for (i = 0; i < 4; ++i) {
    bboxObj.arrayGet(i, &obj1);
    bbox[i] = obj1.getNum();
    obj1.free();
  }
  bboxObj.free();

  // get matrix
  dict->lookup("Matrix", &matrixObj);
  if (matrixObj.isArray()) {
    for (i = 0; i < 6; ++i) {
      matrixObj.arrayGet(i, &obj1);
      m[i] = obj1.getNum();
      obj1.free();
    }
  } else {
    m[0] = 1; m[1] = 0;
    m[2] = 0; m[3] = 1;
    m[4] = 0; m[5] = 0;
  }
  matrixObj.free();

  // get resources
  dict->lookup("Resources", &resObj);
  resDict = resObj.isDict() ? resObj.getDict() : (Dict *)NULL;

  writePSFmt("/f_{0:d}_{1:d} {{\n", id.num, id.gen);
  writePS("q\n");
  writePSFmt("[{0:.6gs} {1:.6gs} {2:.6gs} {3:.6gs} {4:.6gs} {5:.6gs}] cm\n",
	     m[0], m[1], m[2], m[3], m[4], m[5]);

  box.x1 = bbox[0];
  box.y1 = bbox[1];
  box.x2 = bbox[2];
  box.y2 = bbox[3];
  gfx = new Gfx(xref, this, resDict, m_catalog, &box, &box);
  gfx->display(strObj);
  delete gfx;

  writePS("Q\n");
  writePS("} def\n");

  resObj.free();
}

GBool PSOutputDev::checkPageSlice(Page *page, double /*hDPI*/, double /*vDPI*/,
				  int rotateA, GBool useMediaBox, GBool crop,
				  int sliceX, int sliceY,
				  int sliceW, int sliceH,
				  GBool printing, Catalog *catalog,
				  GBool (*abortCheckCbk)(void *data),
				  void *abortCheckCbkData) {
#if HAVE_SPLASH
  PreScanOutputDev *scan;
  GBool rasterize;
  SplashOutputDev *splashOut;
  SplashColor paperColor;
  PDFRectangle box;
  GfxState *state;
  SplashBitmap *bitmap;
  Stream *str0, *str;
  Object obj;
  Guchar *p;
  Guchar col[4];
  double m0, m1, m2, m3, m4, m5;
  int c, w, h, x, y, comp, i;
  char hexBuf[32*2 + 2];	// 32 values X 2 chars/value + line ending + null
  Guchar digit;

  if (!forceRasterize) {
    scan = new PreScanOutputDev();
    page->displaySlice(scan, 72, 72, rotateA, useMediaBox, crop,
                     sliceX, sliceY, sliceW, sliceH,
                     printing, catalog, abortCheckCbk, abortCheckCbkData);
    rasterize = scan->usesTransparency() || scan->hasLevel1PSBug();
    delete scan;
  } else {
    rasterize = gTrue;
  }
  if (!rasterize) {
    return gTrue;
  }

  // rasterize the page
  if (level == psLevel1) {
    paperColor[0] = 0xff;
    splashOut = new SplashOutputDev(splashModeMono8, 1, gFalse,
				    paperColor, gTrue, gFalse);
#if SPLASH_CMYK
  } else if (level == psLevel1Sep) {
    paperColor[0] = paperColor[1] = paperColor[2] = paperColor[3] = 0;
    splashOut = new SplashOutputDev(splashModeCMYK8, 1, gFalse,
				    paperColor, gTrue, gFalse);
#else
  } else if (level == psLevel1Sep) {
    error(-1, "pdftops was built without CMYK support, level1sep needs it to work in this file");
    return gFalse;
#endif
  } else {
    paperColor[0] = paperColor[1] = paperColor[2] = 0xff;
    splashOut = new SplashOutputDev(splashModeRGB8, 1, gFalse,
				    paperColor, gTrue, gFalse);
  }
  splashOut->startDoc(xref);
  page->displaySlice(splashOut, splashDPI, splashDPI, rotateA,
		     useMediaBox, crop,
		     sliceX, sliceY, sliceW, sliceH,
		     printing, catalog, abortCheckCbk, abortCheckCbkData);

  // start the PS page
  page->makeBox(splashDPI, splashDPI, rotateA, useMediaBox, gFalse,
		sliceX, sliceY, sliceW, sliceH, &box, &crop);
  rotateA += page->getRotate();
  if (rotateA >= 360) {
    rotateA -= 360;
  } else if (rotateA < 0) {
    rotateA += 360;
  }
  state = new GfxState(splashDPI, splashDPI, &box, rotateA, gFalse);
  startPage(page->getNum(), state);
  delete state;
  switch (rotateA) {
  case 0:
  default:  // this should never happen
    m0 = box.x2 - box.x1;
    m1 = 0;
    m2 = 0;
    m3 = box.y2 - box.y1;
    m4 = box.x1;
    m5 = box.y1;
    break;
  case 90:
    m0 = 0;
    m1 = box.y2 - box.y1;
    m2 = -(box.x2 - box.x1);
    m3 = 0;
    m4 = box.x2;
    m5 = box.y1;
    break;
  case 180:
    m0 = -(box.x2 - box.x1);
    m1 = 0;
    m2 = 0;
    m3 = -(box.y2 - box.y1);
    m4 = box.x2;
    m5 = box.y2;
    break;
  case 270:
    m0 = 0;
    m1 = -(box.y2 - box.y1);
    m2 = box.x2 - box.x1;
    m3 = 0;
    m4 = box.x1;
    m5 = box.y2;
    break;
  }

  //~ need to add the process colors

  // draw the rasterized image
  bitmap = splashOut->getBitmap();
  w = bitmap->getWidth();
  h = bitmap->getHeight();
  writePS("gsave\n");
  writePSFmt("[{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] concat\n",
	     m0, m1, m2, m3, m4, m5);
  switch (level) {
  case psLevel1:
    writePSFmt("{0:d} {1:d} 8 [{2:d} 0 0 {3:d} 0 {4:d}] pdfIm1\n",
	       w, h, w, -h, h);
    p = bitmap->getDataPtr();
    i = 0;
    for (y = 0; y < h; ++y) {
      for (x = 0; x < w; ++x) {
	digit = *p / 16;
	hexBuf[i++] = digit + ((digit >= 10)? 'a' - 10: '0');
	digit = *p++ % 16;
	hexBuf[i++] = digit + ((digit >= 10)? 'a' - 10: '0');
	if (i >= 64) {
	  hexBuf[i++] = '\n';
	  writePSBuf(hexBuf, i);
	  i = 0;
	}
      }
    }
    if (i != 0) {
      hexBuf[i++] = '\n';
      writePSBuf(hexBuf, i);
    }
    break;
  case psLevel1Sep:
    writePSFmt("{0:d} {1:d} 8 [{2:d} 0 0 {3:d} 0 {4:d}] pdfIm1Sep\n",
	       w, h, w, -h, h);
    p = bitmap->getDataPtr();
    i = 0;
    col[0] = col[1] = col[2] = col[3] = 0;
    if (((psProcessCyan | psProcessMagenta | psProcessYellow | psProcessBlack) & ~processColors) != 0) {
      for (y = 0; y < h; ++y) {
        for (comp = 0; comp < 4; ++comp) {
	  for (x = 0; x < w; ++x) {
	    col[comp] |= p[4*x + comp];
	    digit = p[4*x + comp] / 16;
	    hexBuf[i++] = digit + ((digit >= 10)? 'a' - 10: '0');
	    digit = p[4*x + comp] % 16;
	    hexBuf[i++] = digit + ((digit >= 10)? 'a' - 10: '0');
	    if (i >= 64) {
	      hexBuf[i++] = '\n';
	      writePSBuf(hexBuf, i);
	      i = 0;
	    }
	  }
        }
        p += bitmap->getRowSize();
      }
    } else {
      for (y = 0; y < h; ++y) {
        for (comp = 0; comp < 4; ++comp) {
	  for (x = 0; x < w; ++x) {
	    digit = p[4*x + comp] / 16;
	    hexBuf[i++] = digit + ((digit >= 10)? 'a' - 10: '0');
	    digit = p[4*x + comp] % 16;
	    hexBuf[i++] = digit + ((digit >= 10)? 'a' - 10: '0');
	    if (i >= 64) {
	      hexBuf[i++] = '\n';
	      writePSBuf(hexBuf, i);
	      i = 0;
	    }
	  }
        }
        p += bitmap->getRowSize();
      }
    }
    if (i != 0) {
      hexBuf[i++] = '\n';
      writePSBuf(hexBuf, i);
    }
    if (col[0]) {
      processColors |= psProcessCyan;
    }
    if (col[1]) {
      processColors |= psProcessMagenta;
    }
    if (col[2]) {
      processColors |= psProcessYellow;
    }
    if (col[3]) {
      processColors |= psProcessBlack;
    }
    break;
  case psLevel2:
  case psLevel2Sep:
  case psLevel3:
  case psLevel3Sep:
    writePS("/DeviceRGB setcolorspace\n");
    writePS("<<\n  /ImageType 1\n");
    writePSFmt("  /Width {0:d}\n", bitmap->getWidth());
    writePSFmt("  /Height {0:d}\n", bitmap->getHeight());
    writePSFmt("  /ImageMatrix [{0:d} 0 0 {1:d} 0 {2:d}]\n", w, -h, h);
    writePS("  /BitsPerComponent 8\n");
    writePS("  /Decode [0 1 0 1 0 1]\n");
    writePS("  /DataSource currentfile\n");
    if (globalParams->getPSASCIIHex()) {
      writePS("    /ASCIIHexDecode filter\n");
    } else {
      writePS("    /ASCII85Decode filter\n");
    }
    writePS("    /RunLengthDecode filter\n");
    writePS(">>\n");
    writePS("image\n");
    obj.initNull();
    str0 = new MemStream((char *)bitmap->getDataPtr(), 0, w * h * 3, &obj);
    str = new RunLengthEncoder(str0);
    if (globalParams->getPSASCIIHex()) {
      str = new ASCIIHexEncoder(str);
    } else {
      str = new ASCII85Encoder(str);
    }
    str->reset();
    while ((c = str->getChar()) != EOF) {
      writePSChar(c);
    }
    str->close();
    delete str;
    delete str0;
    processColors |= psProcessCMYK;
    break;
  }
  delete splashOut;
  writePS("grestore\n");

  // finish the PS page
  endPage();

  return gFalse;
#else
  return gTrue;
#endif
}

void PSOutputDev::startPage(int pageNum, GfxState *state) {
  int x1, y1, x2, y2, width, height;
  int imgWidth, imgHeight, imgWidth2, imgHeight2;
  GBool landscape;


  if (mode == psModePS || mode == psModePSOrigPageSizes) {
    GooString pageLabel;
    const GBool gotLabel = m_catalog->indexToLabel(pageNum -1, &pageLabel);
    if (gotLabel) {
      // See bug13338 for why we try to avoid parentheses...
      GBool needParens;
      GooString *filteredString = filterPSLabel(&pageLabel, &needParens);
      if (needParens) {
	writePSFmt("%%Page: ({0:t}) {1:d}\n", filteredString, seqPage);
      } else {
	writePSFmt("%%Page: {0:t} {1:d}\n", filteredString, seqPage);
      }
      delete filteredString;
    } else {
      writePSFmt("%%Page: {0:d} {1:d}\n", pageNum, seqPage);
    }
    if (mode != psModePSOrigPageSizes)
      writePS("%%BeginPageSetup\n");
  }

  // underlays
  if (underlayCbk) {
    (*underlayCbk)(this, underlayCbkData);
  }
  if (overlayCbk) {
    saveState(NULL);
  }

  switch (mode) {

  case psModePSOrigPageSizes:
    x1 = (int)floor(state->getX1());
    y1 = (int)floor(state->getY1());
    x2 = (int)ceil(state->getX2());
    y2 = (int)ceil(state->getY2());
    width = x2 - x1;
    height = y2 - y1;
    if (width > height) {
      landscape = gTrue;
    } else {
      landscape = gFalse;
    }
    writePSFmt("%%PageBoundingBox: {0:d} {1:d} {2:d} {3:d}\n", x1, y1, x2 - x1, y2 - y1);
    writePS("%%BeginPageSetup\n");
    writePSFmt("%%PageOrientation: {0:s}\n",
	       landscape ? "Landscape" : "Portrait");
    if ((width != prevWidth) || (height != prevHeight)) {
      // Set page size only when it actually changes, as otherwise Duplex
      // printing does not work
      writePSFmt("<</PageSize [{0:d} {1:d}]>> setpagedevice\n", width, height);
      prevWidth = width;
      prevHeight = height;
    }
    writePS("pdfStartPage\n");
    writePSFmt("{0:d} {1:d} {2:d} {3:d} re W\n", x1, y1, x2 - x1, y2 - y1);
    writePS("%%EndPageSetup\n");
    ++seqPage;
    break;

  case psModePS:
    // rotate, translate, and scale page
    imgWidth = imgURX - imgLLX;
    imgHeight = imgURY - imgLLY;
    x1 = (int)floor(state->getX1());
    y1 = (int)floor(state->getY1());
    x2 = (int)ceil(state->getX2());
    y2 = (int)ceil(state->getY2());
    width = x2 - x1;
    height = y2 - y1;
    tx = ty = 0;
    // rotation and portrait/landscape mode
    if (rotate0 >= 0) {
      rotate = (360 - rotate0) % 360;
      landscape = gFalse;
    } else {
      rotate = (360 - state->getRotate()) % 360;
      if (rotate == 0 || rotate == 180) {
	if (width > height && width > imgWidth) {
	  rotate += 90;
	  landscape = gTrue;
	} else {
	  landscape = gFalse;
	}
      } else { // rotate == 90 || rotate == 270
	if (height > width && height > imgWidth) {
	  rotate = 270 - rotate;
	  landscape = gTrue;
	} else {
	  landscape = gFalse;
	}
      }
    }
    writePSFmt("%%PageOrientation: {0:s}\n",
	       landscape ? "Landscape" : "Portrait");
    writePS("pdfStartPage\n");
    if (rotate == 0) {
      imgWidth2 = imgWidth;
      imgHeight2 = imgHeight;
    } else if (rotate == 90) {
      writePS("90 rotate\n");
      ty = -imgWidth;
      imgWidth2 = imgHeight;
      imgHeight2 = imgWidth;
    } else if (rotate == 180) {
      writePS("180 rotate\n");
      imgWidth2 = imgWidth;
      imgHeight2 = imgHeight;
      tx = -imgWidth;
      ty = -imgHeight;
    } else { // rotate == 270
      writePS("270 rotate\n");
      tx = -imgHeight;
      imgWidth2 = imgHeight;
      imgHeight2 = imgWidth;
    }
    // shrink or expand
    if (xScale0 > 0 && yScale0 > 0) {
      xScale = xScale0;
      yScale = yScale0;
    } else if ((globalParams->getPSShrinkLarger() &&
	 (width > imgWidth2 || height > imgHeight2)) ||
	(globalParams->getPSExpandSmaller() &&
	 (width < imgWidth2 && height < imgHeight2))) {
      xScale = (double)imgWidth2 / (double)width;
      yScale = (double)imgHeight2 / (double)height;
      if (yScale < xScale) {
	xScale = yScale;
      } else {
	yScale = xScale;
      }
    } else {
      xScale = yScale = 1;
    }
    // deal with odd bounding boxes or clipping
    if (clipLLX0 < clipURX0 && clipLLY0 < clipURY0) {
      tx -= xScale * clipLLX0;
      ty -= yScale * clipLLY0;
    } else {
      tx -= xScale * x1;
      ty -= yScale * y1;
    }
    // center
    if (tx0 >= 0 && ty0 >= 0) {
      tx += rotate == 0 ? tx0 : ty0;
      ty += rotate == 0 ? ty0 : -tx0;
    } else if (globalParams->getPSCenter()) {
      if (clipLLX0 < clipURX0 && clipLLY0 < clipURY0) {
	tx += (imgWidth2 - xScale * (clipURX0 - clipLLX0)) / 2;
	ty += (imgHeight2 - yScale * (clipURY0 - clipLLY0)) / 2;
      } else {
	tx += (imgWidth2 - xScale * width) / 2;
	ty += (imgHeight2 - yScale * height) / 2;
      }
    }
    tx += rotate == 0 ? imgLLX : imgLLY;
    ty += rotate == 0 ? imgLLY : -imgLLX;
    if (tx != 0 || ty != 0) {
      writePSFmt("{0:.6g} {1:.6g} translate\n", tx, ty);
    }
    if (xScale != 1 || yScale != 1) {
      writePSFmt("{0:.6f} {1:.6f} scale\n", xScale, yScale);
    }
    if (clipLLX0 < clipURX0 && clipLLY0 < clipURY0) {
      writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} re W\n",
		 clipLLX0, clipLLY0, clipURX0 - clipLLX0, clipURY0 - clipLLY0);
    } else {
      writePSFmt("{0:d} {1:d} {2:d} {3:d} re W\n", x1, y1, x2 - x1, y2 - y1);
    }

    writePS("%%EndPageSetup\n");
    ++seqPage;
    break;

  case psModeEPS:
    writePS("pdfStartPage\n");
    tx = ty = 0;
    rotate = (360 - state->getRotate()) % 360;
    if (rotate == 0) {
    } else if (rotate == 90) {
      writePS("90 rotate\n");
      tx = -epsX1;
      ty = -epsY2;
    } else if (rotate == 180) {
      writePS("180 rotate\n");
      tx = -(epsX1 + epsX2);
      ty = -(epsY1 + epsY2);
    } else { // rotate == 270
      writePS("270 rotate\n");
      tx = -epsX2;
      ty = -epsY1;
    }
    if (tx != 0 || ty != 0) {
      writePSFmt("{0:.6g} {1:.6g} translate\n", tx, ty);
    }
    xScale = yScale = 1;
    break;

  case psModeForm:
    writePS("/PaintProc {\n");
    writePS("begin xpdf begin\n");
    writePS("pdfStartPage\n");
    tx = ty = 0;
    xScale = yScale = 1;
    rotate = 0;
    break;
  }
}

void PSOutputDev::endPage() {
  if (overlayCbk) {
    restoreState(NULL);
    (*overlayCbk)(this, overlayCbkData);
  }


  if (mode == psModeForm) {
    writePS("pdfEndPage\n");
    writePS("end end\n");
    writePS("} def\n");
    writePS("end end\n");
  } else {
    if (!manualCtrl) {
      writePS("showpage\n");
    }
      writePS("%%PageTrailer\n");
      writePageTrailer();
    }
}

void PSOutputDev::saveState(GfxState *state) {
  writePS("q\n");
  ++numSaves;
}

void PSOutputDev::restoreState(GfxState *state) {
  writePS("Q\n");
  --numSaves;
}

void PSOutputDev::updateCTM(GfxState *state, double m11, double m12,
			    double m21, double m22, double m31, double m32) {
  writePSFmt("[{0:.6gs} {1:.6gs} {2:.6gs} {3:.6gs} {4:.6gs} {5:.6gs}] cm\n",
	     m11, m12, m21, m22, m31, m32);
}

void PSOutputDev::updateLineDash(GfxState *state) {
  double *dash;
  double start;
  int length, i;

  state->getLineDash(&dash, &length, &start);
  writePS("[");
  for (i = 0; i < length; ++i) {
    writePSFmt("{0:.6g}{1:w}",
	       dash[i] < 0 ? 0 : dash[i],
	       (i == length-1) ? 0 : 1);
  }
  writePSFmt("] {0:.6g} d\n", start);
}

void PSOutputDev::updateFlatness(GfxState *state) {
  writePSFmt("{0:d} i\n", state->getFlatness());
}

void PSOutputDev::updateLineJoin(GfxState *state) {
  writePSFmt("{0:d} j\n", state->getLineJoin());
}

void PSOutputDev::updateLineCap(GfxState *state) {
  writePSFmt("{0:d} J\n", state->getLineCap());
}

void PSOutputDev::updateMiterLimit(GfxState *state) {
  writePSFmt("{0:.6g} M\n", state->getMiterLimit());
}

void PSOutputDev::updateLineWidth(GfxState *state) {
  writePSFmt("{0:.6g} w\n", state->getLineWidth());
}

void PSOutputDev::updateFillColorSpace(GfxState *state) {
  switch (level) {
  case psLevel1:
  case psLevel1Sep:
    break;
  case psLevel2:
  case psLevel3:
    if (state->getFillColorSpace()->getMode() != csPattern) {
      dumpColorSpaceL2(state->getFillColorSpace(), gTrue, gFalse, gFalse);
      writePS(" cs\n");
    }
    break;
  case psLevel2Sep:
  case psLevel3Sep:
    break;
  }
}

void PSOutputDev::updateStrokeColorSpace(GfxState *state) {
  switch (level) {
  case psLevel1:
  case psLevel1Sep:
    break;
  case psLevel2:
  case psLevel3:
    if (state->getStrokeColorSpace()->getMode() != csPattern) {
      dumpColorSpaceL2(state->getStrokeColorSpace(), gTrue, gFalse, gFalse);
      writePS(" CS\n");
    }
    break;
  case psLevel2Sep:
  case psLevel3Sep:
    break;
  }
}

void PSOutputDev::updateFillColor(GfxState *state) {
  GfxColor color;
  GfxColor *colorPtr;
  GfxGray gray;
  GfxCMYK cmyk;
  GfxSeparationColorSpace *sepCS;
  double c, m, y, k;
  int i;

  switch (level) {
  case psLevel1:
    state->getFillGray(&gray);
    writePSFmt("{0:.4g} g\n", colToDbl(gray));
    break;
  case psLevel1Sep:
    state->getFillCMYK(&cmyk);
    c = colToDbl(cmyk.c);
    m = colToDbl(cmyk.m);
    y = colToDbl(cmyk.y);
    k = colToDbl(cmyk.k);
    writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} k\n", c, m, y, k);
    addProcessColor(c, m, y, k);
    break;
  case psLevel2:
  case psLevel3:
    if (state->getFillColorSpace()->getMode() != csPattern) {
      colorPtr = state->getFillColor();
      writePS("[");
      for (i = 0; i < state->getFillColorSpace()->getNComps(); ++i) {
	if (i > 0) {
	  writePS(" ");
      }
	writePSFmt("{0:.4g}", colToDbl(colorPtr->c[i]));
      }
      writePS("] sc\n");
    }
    break;
  case psLevel2Sep:
  case psLevel3Sep:
    if (state->getFillColorSpace()->getMode() == csSeparation) {
      sepCS = (GfxSeparationColorSpace *)state->getFillColorSpace();
      color.c[0] = gfxColorComp1;
      sepCS->getCMYK(&color, &cmyk);
      writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} {4:.4g} ({5:t}) ck\n",
		 colToDbl(state->getFillColor()->c[0]),
		 colToDbl(cmyk.c), colToDbl(cmyk.m),
		 colToDbl(cmyk.y), colToDbl(cmyk.k),
		 sepCS->getName());
      addCustomColor(sepCS);
    } else {
      state->getFillCMYK(&cmyk);
      c = colToDbl(cmyk.c);
      m = colToDbl(cmyk.m);
      y = colToDbl(cmyk.y);
      k = colToDbl(cmyk.k);
      writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} k\n", c, m, y, k);
      addProcessColor(c, m, y, k);
    }
    break;
  }
  t3Cacheable = gFalse;
}

void PSOutputDev::updateStrokeColor(GfxState *state) {
  GfxColor color;
  GfxColor *colorPtr;
  GfxGray gray;
  GfxCMYK cmyk;
  GfxSeparationColorSpace *sepCS;
  double c, m, y, k;
  int i;

  switch (level) {
  case psLevel1:
    state->getStrokeGray(&gray);
    writePSFmt("{0:.4g} G\n", colToDbl(gray));
    break;
  case psLevel1Sep:
    state->getStrokeCMYK(&cmyk);
    c = colToDbl(cmyk.c);
    m = colToDbl(cmyk.m);
    y = colToDbl(cmyk.y);
    k = colToDbl(cmyk.k);
    writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} K\n", c, m, y, k);
    addProcessColor(c, m, y, k);
    break;
  case psLevel2:
  case psLevel3:
    if (state->getStrokeColorSpace()->getMode() != csPattern) {
      colorPtr = state->getStrokeColor();
      writePS("[");
      for (i = 0; i < state->getStrokeColorSpace()->getNComps(); ++i) {
	if (i > 0) {
	  writePS(" ");
	}
	writePSFmt("{0:.4g}", colToDbl(colorPtr->c[i]));
      }
      writePS("] SC\n");
    }
    break;
  case psLevel2Sep:
  case psLevel3Sep:
    if (state->getStrokeColorSpace()->getMode() == csSeparation) {
      sepCS = (GfxSeparationColorSpace *)state->getStrokeColorSpace();
      color.c[0] = gfxColorComp1;
      sepCS->getCMYK(&color, &cmyk);
      writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} {4:.4g} ({5:t}) CK\n",
		 colToDbl(state->getStrokeColor()->c[0]),
		 colToDbl(cmyk.c), colToDbl(cmyk.m),
		 colToDbl(cmyk.y), colToDbl(cmyk.k),
		 sepCS->getName());
      addCustomColor(sepCS);
    } else {
      state->getStrokeCMYK(&cmyk);
      c = colToDbl(cmyk.c);
      m = colToDbl(cmyk.m);
      y = colToDbl(cmyk.y);
      k = colToDbl(cmyk.k);
      writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} K\n", c, m, y, k);
      addProcessColor(c, m, y, k);
    }
    break;
  }
  t3Cacheable = gFalse;
}

void PSOutputDev::addProcessColor(double c, double m, double y, double k) {
  if (c > 0) {
    processColors |= psProcessCyan;
  }
  if (m > 0) {
    processColors |= psProcessMagenta;
  }
  if (y > 0) {
    processColors |= psProcessYellow;
  }
  if (k > 0) {
    processColors |= psProcessBlack;
  }
}

void PSOutputDev::addCustomColor(GfxSeparationColorSpace *sepCS) {
  PSOutCustomColor *cc;
  GfxColor color;
  GfxCMYK cmyk;

  for (cc = customColors; cc; cc = cc->next) {
    if (!cc->name->cmp(sepCS->getName())) {
      return;
    }
  }
  color.c[0] = gfxColorComp1;
  sepCS->getCMYK(&color, &cmyk);
  cc = new PSOutCustomColor(colToDbl(cmyk.c), colToDbl(cmyk.m),
			    colToDbl(cmyk.y), colToDbl(cmyk.k),
			    sepCS->getName()->copy());
  cc->next = customColors;
  customColors = cc;
}

void PSOutputDev::updateFillOverprint(GfxState *state) {
  if (level >= psLevel2) {
    writePSFmt("{0:s} op\n", state->getFillOverprint() ? "true" : "false");
  }
}

void PSOutputDev::updateStrokeOverprint(GfxState *state) {
  if (level >= psLevel2) {
    writePSFmt("{0:s} OP\n", state->getStrokeOverprint() ? "true" : "false");
  }
}

void PSOutputDev::updateTransfer(GfxState *state) {
  Function **funcs;
  int i;

  funcs = state->getTransfer();
  if (funcs[0] && funcs[1] && funcs[2] && funcs[3]) {
    if (level >= psLevel2) {
      for (i = 0; i < 4; ++i) {
	cvtFunction(funcs[i]);
      }
      writePS("setcolortransfer\n");
    } else {
      cvtFunction(funcs[3]);
      writePS("settransfer\n");
    }
  } else if (funcs[0]) {
    cvtFunction(funcs[0]);
    writePS("settransfer\n");
  } else {
    writePS("{} settransfer\n");
  }
}

void PSOutputDev::updateFont(GfxState *state) {
  if (state->getFont()) {
    writePSFmt("/F{0:d}_{1:d} {2:.6g} Tf\n",
	       state->getFont()->getID()->num, state->getFont()->getID()->gen,
	       fabs(state->getFontSize()) < 0.00001 ? 0.00001
	                                            : state->getFontSize());
  }
}

void PSOutputDev::updateTextMat(GfxState *state) {
  double *mat;

  mat = state->getTextMat();
  if (fabs(mat[0] * mat[3] - mat[1] * mat[2]) < 0.00001) {
    // avoid a singular (or close-to-singular) matrix
    writePSFmt("[0.00001 0 0 0.00001 {0:.6g} {1:.6g}] Tm\n", mat[4], mat[5]);
  } else {
    writePSFmt("[{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] Tm\n",
	       mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
  }
}

void PSOutputDev::updateCharSpace(GfxState *state) {
  writePSFmt("{0:.6g} Tc\n", state->getCharSpace());
}

void PSOutputDev::updateRender(GfxState *state) {
  int rm;

  rm = state->getRender();
  writePSFmt("{0:d} Tr\n", rm);
  rm &= 3;
  if (rm != 0 && rm != 3) {
    t3Cacheable = gFalse;
  }
}

void PSOutputDev::updateRise(GfxState *state) {
  writePSFmt("{0:.6g} Ts\n", state->getRise());
}

void PSOutputDev::updateWordSpace(GfxState *state) {
  writePSFmt("{0:.6g} Tw\n", state->getWordSpace());
}

void PSOutputDev::updateHorizScaling(GfxState *state) {
  double h;

  h = state->getHorizScaling();
  if (fabs(h) < 0.01) {
    h = 0.01;
  }
  writePSFmt("{0:.6g} Tz\n", h);
}

void PSOutputDev::updateTextPos(GfxState *state) {
  writePSFmt("{0:.6g} {1:.6g} Td\n", state->getLineX(), state->getLineY());
}

void PSOutputDev::updateTextShift(GfxState *state, double shift) {
  if (state->getFont()->getWMode()) {
    writePSFmt("{0:.6g} TJmV\n", shift);
  } else {
    writePSFmt("{0:.6g} TJm\n", shift);
  }
}

void PSOutputDev::stroke(GfxState *state) {
  doPath(state->getPath());
  if (t3String) {
    // if we're construct a cacheable Type 3 glyph, we need to do
    // everything in the fill color
    writePS("Sf\n");
  } else {
    writePS("S\n");
  }
}

void PSOutputDev::fill(GfxState *state) {
  doPath(state->getPath());
  writePS("f\n");
}

void PSOutputDev::eoFill(GfxState *state) {
  doPath(state->getPath());
  writePS("f*\n");
}

GBool PSOutputDev::tilingPatternFill(GfxState *state, Object *str,
				     int paintType, Dict *resDict,
				     double *mat, double *bbox,
				     int x0, int y0, int x1, int y1,
				     double xStep, double yStep) {
  PDFRectangle box;
  Gfx *gfx;

  // define a Type 3 font
  writePS("8 dict begin\n");
  writePS("/FontType 3 def\n");
  writePS("/FontMatrix [1 0 0 1 0 0] def\n");
  writePSFmt("/FontBBox [{0:.6g} {1:.6g} {2:.6g} {3:.6g}] def\n",
	     bbox[0], bbox[1], bbox[2], bbox[3]);
  writePS("/Encoding 256 array def\n");
  writePS("  0 1 255 { Encoding exch /.notdef put } for\n");
  writePS("  Encoding 120 /x put\n");
  writePS("/BuildGlyph {\n");
  writePS("  exch /CharProcs get exch\n");
  writePS("  2 copy known not { pop /.notdef } if\n");
  writePS("  get exec\n");
  writePS("} bind def\n");
  writePS("/BuildChar {\n");
  writePS("  1 index /Encoding get exch get\n");
  writePS("  1 index /BuildGlyph get exec\n");
  writePS("} bind def\n");
  writePS("/CharProcs 1 dict def\n");
  writePS("CharProcs begin\n");
  box.x1 = bbox[0];
  box.y1 = bbox[1];
  box.x2 = bbox[2];
  box.y2 = bbox[3];
  gfx = new Gfx(xref, this, resDict, m_catalog, &box, NULL);
  writePS("/x {\n");
  if (paintType == 2) {
    writePSFmt("{0:.6g} 0 {1:.6g} {2:.6g} {3:.6g} {4:.6g} setcachedevice\n",
	       xStep, bbox[0], bbox[1], bbox[2], bbox[3]);
  } else
  {
    if (x1 - 1 <= x0) {
      writePS("1 0 setcharwidth\n");
    } else {
      writePSFmt("{0:.6g} 0 setcharwidth\n", xStep);
    }
  }
  inType3Char = gTrue;
  ++numTilingPatterns;
  gfx->display(str);
  --numTilingPatterns;
  inType3Char = gFalse;
  writePS("} def\n");
  delete gfx;
  writePS("end\n");
  writePS("currentdict end\n");
  writePSFmt("/xpdfTile{0:d} exch definefont pop\n", numTilingPatterns);

  // draw the tiles
  writePSFmt("/xpdfTile{0:d} findfont setfont\n", numTilingPatterns);
  writePSFmt("gsave [{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] concat\n",
	     mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
  writePSFmt("{0:d} 1 {1:d} {{ {2:.6g} exch {3:.6g} mul m {4:d} 1 {5:d} {{ pop (x) show }} for }} for\n",
	     y0, y1 - 1, x0 * xStep, yStep, x0, x1 - 1);
  writePS("grestore\n");

  return gTrue;
}

GBool PSOutputDev::functionShadedFill(GfxState *state,
				     GfxFunctionShading *shading) {
  double x0, y0, x1, y1;
  double *mat;
  int i;

  if (level == psLevel2Sep || level == psLevel3Sep) {
    if (shading->getColorSpace()->getMode() != csDeviceCMYK) {
      return gFalse;
    }
    processColors |= psProcessCMYK;
  }

  shading->getDomain(&x0, &y0, &x1, &y1);
  mat = shading->getMatrix();
  writePSFmt("/mat [{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] def\n",
	     mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
  writePSFmt("/n {0:d} def\n", shading->getColorSpace()->getNComps());
  if (shading->getNFuncs() == 1) {
    writePS("/func ");
    cvtFunction(shading->getFunc(0));
    writePS("def\n");
  } else {
    writePS("/func {\n");
    for (i = 0; i < shading->getNFuncs(); ++i) {
      if (i < shading->getNFuncs() - 1) {
	writePS("2 copy\n");
      }
      cvtFunction(shading->getFunc(i));
      writePS("exec\n");
      if (i < shading->getNFuncs() - 1) {
	writePS("3 1 roll\n");
      }
    }
    writePS("} def\n");
  }
  writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} 0 funcSH\n", x0, y0, x1, y1);

  return gTrue;
}

GBool PSOutputDev::axialShadedFill(GfxState *state, GfxAxialShading *shading, double /*tMin*/, double /*tMax*/) {
  double xMin, yMin, xMax, yMax;
  double x0, y0, x1, y1, dx, dy, mul;
  double tMin, tMax, t, t0, t1;
  int i;

  if (level == psLevel2Sep || level == psLevel3Sep) {
    if (shading->getColorSpace()->getMode() != csDeviceCMYK) {
      return gFalse;
    }
    processColors |= psProcessCMYK;
  }

  // get the clip region bbox
  state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);

  // compute min and max t values, based on the four corners of the
  // clip region bbox
  shading->getCoords(&x0, &y0, &x1, &y1);
  dx = x1 - x0;
  dy = y1 - y0;
  if (fabs(dx) < 0.01 && fabs(dy) < 0.01) {
    return gTrue;
  } else {
    mul = 1 / (dx * dx + dy * dy);
    tMin = tMax = ((xMin - x0) * dx + (yMin - y0) * dy) * mul;
    t = ((xMin - x0) * dx + (yMax - y0) * dy) * mul;
    if (t < tMin) {
      tMin = t;
    } else if (t > tMax) {
      tMax = t;
    }
    t = ((xMax - x0) * dx + (yMin - y0) * dy) * mul;
    if (t < tMin) {
      tMin = t;
    } else if (t > tMax) {
      tMax = t;
    }
    t = ((xMax - x0) * dx + (yMax - y0) * dy) * mul;
    if (t < tMin) {
      tMin = t;
    } else if (t > tMax) {
      tMax = t;
    }
    if (tMin < 0 && !shading->getExtend0()) {
      tMin = 0;
    }
    if (tMax > 1 && !shading->getExtend1()) {
      tMax = 1;
    }
  }

  // get the function domain
  t0 = shading->getDomain0();
  t1 = shading->getDomain1();

  // generate the PS code
  writePSFmt("/t0 {0:.6g} def\n", t0);
  writePSFmt("/t1 {0:.6g} def\n", t1);
  writePSFmt("/dt {0:.6g} def\n", t1 - t0);
  writePSFmt("/x0 {0:.6g} def\n", x0);
  writePSFmt("/y0 {0:.6g} def\n", y0);
  writePSFmt("/dx {0:.6g} def\n", x1 - x0);
  writePSFmt("/x1 {0:.6g} def\n", x1);
  writePSFmt("/y1 {0:.6g} def\n", y1);
  writePSFmt("/dy {0:.6g} def\n", y1 - y0);
  writePSFmt("/xMin {0:.6g} def\n", xMin);
  writePSFmt("/yMin {0:.6g} def\n", yMin);
  writePSFmt("/xMax {0:.6g} def\n", xMax);
  writePSFmt("/yMax {0:.6g} def\n", yMax);
  writePSFmt("/n {0:d} def\n", shading->getColorSpace()->getNComps());
  if (shading->getNFuncs() == 1) {
    writePS("/func ");
    cvtFunction(shading->getFunc(0));
    writePS("def\n");
  } else {
    writePS("/func {\n");
    for (i = 0; i < shading->getNFuncs(); ++i) {
      if (i < shading->getNFuncs() - 1) {
	writePS("dup\n");
      }
      cvtFunction(shading->getFunc(i));
      writePS("exec\n");
      if (i < shading->getNFuncs() - 1) {
	writePS("exch\n");
      }
    }
    writePS("} def\n");
  }
  writePSFmt("{0:.6g} {1:.6g} 0 axialSH\n", tMin, tMax);

  return gTrue;
}

GBool PSOutputDev::radialShadedFill(GfxState *state, GfxRadialShading *shading, double /*sMin*/, double /*sMax*/) {
  double xMin, yMin, xMax, yMax;
  double x0, y0, r0, x1, y1, r1, t0, t1;
  double xa, ya, ra;
  double sz, xz, yz, sMin, sMax, sa, ta;
  double theta, alpha, a1, a2;
  GBool enclosed;
  int i;

  if (level == psLevel2Sep || level == psLevel3Sep) {
    if (shading->getColorSpace()->getMode() != csDeviceCMYK) {
      return gFalse;
    }
    processColors |= psProcessCMYK;
  }

  // get the shading info
  shading->getCoords(&x0, &y0, &r0, &x1, &y1, &r1);
  t0 = shading->getDomain0();
  t1 = shading->getDomain1();

  // Compute the point at which r(s) = 0; check for the enclosed
  // circles case; and compute the angles for the tangent lines.
  if (r0 == r1) {
    enclosed = x0 == x1 && y0 == y1;
    theta = 0;
    sz = 0; // make gcc happy
  } else {
    sz = -r0 / (r1 - r0);
    xz = x0 + sz * (x1 - x0);
    yz = y0 + sz * (y1 - y0);
    enclosed = (xz - x0) * (xz - x0) + (yz - y0) * (yz - y0) <= r0 * r0;
    theta = asin(r0 / sqrt((x0 - xz) * (x0 - xz) + (y0 - yz) * (y0 - yz)));
    if (r0 > r1) {
      theta = -theta;
    }
  }
  if (enclosed) {
    a1 = 0;
    a2 = 360;
  } else {
    alpha = atan2(y1 - y0, x1 - x0);
    a1 = (180 / M_PI) * (alpha + theta) + 90;
    a2 = (180 / M_PI) * (alpha - theta) - 90;
    while (a2 < a1) {
      a2 += 360;
    }
  }

  // compute the (possibly extended) s range
  state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
  if (enclosed) {
    sMin = 0;
    sMax = 1;
  } else {
    sMin = 1;
    sMax = 0;
    // solve for x(s) + r(s) = xMin
    if ((x1 + r1) - (x0 + r0) != 0) {
      sa = (xMin - (x0 + r0)) / ((x1 + r1) - (x0 + r0));
      if (sa < sMin) {
	sMin = sa;
      } else if (sa > sMax) {
	sMax = sa;
      }
    }
    // solve for x(s) - r(s) = xMax
    if ((x1 - r1) - (x0 - r0) != 0) {
      sa = (xMax - (x0 - r0)) / ((x1 - r1) - (x0 - r0));
      if (sa < sMin) {
	sMin = sa;
      } else if (sa > sMax) {
	sMax = sa;
      }
    }
    // solve for y(s) + r(s) = yMin
    if ((y1 + r1) - (y0 + r0) != 0) {
      sa = (yMin - (y0 + r0)) / ((y1 + r1) - (y0 + r0));
      if (sa < sMin) {
	sMin = sa;
      } else if (sa > sMax) {
	sMax = sa;
      }
    }
    // solve for y(s) - r(s) = yMax
    if ((y1 - r1) - (y0 - r0) != 0) {
      sa = (yMax - (y0 - r0)) / ((y1 - r1) - (y0 - r0));
      if (sa < sMin) {
	sMin = sa;
      } else if (sa > sMax) {
	sMax = sa;
      }
    }
    // check against sz
    if (r0 < r1) {
      if (sMin < sz) {
	sMin = sz;
      }
    } else if (r0 > r1) {
      if (sMax > sz) {
	sMax = sz;
      }
    }
    // check the 'extend' flags
    if (!shading->getExtend0() && sMin < 0) {
      sMin = 0;
    }
    if (!shading->getExtend1() && sMax > 1) {
      sMax = 1;
    }
  }

  // generate the PS code
  writePSFmt("/x0 {0:.6g} def\n", x0);
  writePSFmt("/x1 {0:.6g} def\n", x1);
  writePSFmt("/dx {0:.6g} def\n", x1 - x0);
  writePSFmt("/y0 {0:.6g} def\n", y0);
  writePSFmt("/y1 {0:.6g} def\n", y1);
  writePSFmt("/dy {0:.6g} def\n", y1 - y0);
  writePSFmt("/r0 {0:.6g} def\n", r0);
  writePSFmt("/r1 {0:.6g} def\n", r1);
  writePSFmt("/dr {0:.6g} def\n", r1 - r0);
  writePSFmt("/t0 {0:.6g} def\n", t0);
  writePSFmt("/t1 {0:.6g} def\n", t1);
  writePSFmt("/dt {0:.6g} def\n", t1 - t0);
  writePSFmt("/n {0:d} def\n", shading->getColorSpace()->getNComps());
  writePSFmt("/encl {0:s} def\n", enclosed ? "true" : "false");
  writePSFmt("/a1 {0:.6g} def\n", a1);
  writePSFmt("/a2 {0:.6g} def\n", a2);
  if (shading->getNFuncs() == 1) {
    writePS("/func ");
    cvtFunction(shading->getFunc(0));
    writePS("def\n");
  } else {
    writePS("/func {\n");
    for (i = 0; i < shading->getNFuncs(); ++i) {
      if (i < shading->getNFuncs() - 1) {
	writePS("dup\n");
      }
      cvtFunction(shading->getFunc(i));
      writePS("exec\n");
      if (i < shading->getNFuncs() - 1) {
	writePS("exch\n");
      }
    }
    writePS("} def\n");
  }
  writePSFmt("{0:.6g} {1:.6g} 0 radialSH\n", sMin, sMax);

  // extend the 'enclosed' case
  if (enclosed) {
    // extend the smaller circle
    if ((shading->getExtend0() && r0 <= r1) ||
	(shading->getExtend1() && r1 < r0)) {
      if (r0 <= r1) {
	ta = t0;
	ra = r0;
	xa = x0;
	ya = y0;
      } else {
	ta = t1;
	ra = r1;
	xa = x1;
	ya = y1;
      }
      if (level == psLevel2Sep || level == psLevel3Sep) {
	writePSFmt("{0:.6g} radialCol aload pop k\n", ta);
      } else {
	writePSFmt("{0:.6g} radialCol sc\n", ta);
      }
      writePSFmt("{0:.6g} {1:.6g} {2:.6g} 0 360 arc h f*\n", xa, ya, ra);
    }

    // extend the larger circle
    if ((shading->getExtend0() && r0 > r1) ||
	(shading->getExtend1() && r1 >= r0)) {
      if (r0 > r1) {
	ta = t0;
	ra = r0;
	xa = x0;
	ya = y0;
      } else {
	ta = t1;
	ra = r1;
	xa = x1;
	ya = y1;
      }
      if (level == psLevel2Sep || level == psLevel3Sep) {
	writePSFmt("{0:.6g} radialCol aload pop k\n", ta);
      } else {
	writePSFmt("{0:.6g} radialCol sc\n", ta);
      }
      writePSFmt("{0:.6g} {1:.6g} {2:.6g} 0 360 arc h\n", xa, ya, ra);
      writePSFmt("{0:.6g} {1:.6g} m {2:.6g} {3:.6g} l {4:.6g} {5:.6g} l {6:.6g} {7:.6g} l h f*\n",
		 xMin, yMin, xMin, yMax, xMax, yMax, xMax, yMin);
    }
  }

  return gTrue;
}

void PSOutputDev::clip(GfxState *state) {
  doPath(state->getPath());
  writePS("W\n");
}

void PSOutputDev::eoClip(GfxState *state) {
  doPath(state->getPath());
  writePS("W*\n");
}

void PSOutputDev::clipToStrokePath(GfxState *state) {
  doPath(state->getPath());
  writePS("Ws\n");
}

void PSOutputDev::doPath(GfxPath *path) {
  GfxSubpath *subpath;
  double x0, y0, x1, y1, x2, y2, x3, y3, x4, y4;
  int n, m, i, j;

  n = path->getNumSubpaths();

  if (n == 1 && path->getSubpath(0)->getNumPoints() == 5) {
    subpath = path->getSubpath(0);
    x0 = subpath->getX(0);
    y0 = subpath->getY(0);
    x4 = subpath->getX(4);
    y4 = subpath->getY(4);
    if (x4 == x0 && y4 == y0) {
      x1 = subpath->getX(1);
      y1 = subpath->getY(1);
      x2 = subpath->getX(2);
      y2 = subpath->getY(2);
      x3 = subpath->getX(3);
      y3 = subpath->getY(3);
      if (x0 == x1 && x2 == x3 && y0 == y3 && y1 == y2) {
	writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} re\n",
		   x0 < x2 ? x0 : x2, y0 < y1 ? y0 : y1,
		   fabs(x2 - x0), fabs(y1 - y0));
	return;
      } else if (x0 == x3 && x1 == x2 && y0 == y1 && y2 == y3) {
	writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} re\n",
		   x0 < x1 ? x0 : x1, y0 < y2 ? y0 : y2,
		   fabs(x1 - x0), fabs(y2 - y0));
	return;
      }
    }
  }

  for (i = 0; i < n; ++i) {
    subpath = path->getSubpath(i);
    m = subpath->getNumPoints();
    writePSFmt("{0:.6g} {1:.6g} m\n", subpath->getX(0), subpath->getY(0));
    j = 1;
    while (j < m) {
      if (subpath->getCurve(j)) {
	writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g} c\n",
		   subpath->getX(j), subpath->getY(j),
		   subpath->getX(j+1), subpath->getY(j+1),
		   subpath->getX(j+2), subpath->getY(j+2));
	j += 3;
      } else {
	writePSFmt("{0:.6g} {1:.6g} l\n", subpath->getX(j), subpath->getY(j));
	++j;
      }
    }
    if (subpath->isClosed()) {
      writePS("h\n");
    }
  }
}

void PSOutputDev::drawString(GfxState *state, GooString *s) {
  GfxFont *font;
  int wMode;
  Gushort *codeToGID;
  GooString *s2;
  double dx, dy, dx2, dy2, originX, originY;
  char *p;
  UnicodeMap *uMap;
  CharCode code;
  Unicode *u;
  char buf[8];
  int len, nChars, uLen, n, m, i, j;

  // for pdftohtml, output PS without text
  if( displayText == gFalse )
    return;

  // check for invisible text -- this is used by Acrobat Capture
  if (state->getRender() == 3) {
    return;
  }

  // ignore empty strings
  if (s->getLength() == 0) {
    return;
  }

  // get the font
  if (!(font = state->getFont())) {
    return;
  }
  wMode = font->getWMode();

  // check for a subtitute 16-bit font
  uMap = NULL;
  codeToGID = NULL;
  if (font->isCIDFont()) {
    for (i = 0; i < font16EncLen; ++i) {
      if (font->getID()->num == font16Enc[i].fontID.num &&
	  font->getID()->gen == font16Enc[i].fontID.gen) {
	uMap = globalParams->getUnicodeMap(font16Enc[i].enc);
	break;
      }
    }

  // check for a code-to-GID map
  } else {
    for (i = 0; i < font8InfoLen; ++i) {
      if (font->getID()->num == font8Info[i].fontID.num &&
	  font->getID()->gen == font8Info[i].fontID.gen) {
	codeToGID = font8Info[i].codeToGID;
	break;
      }
    }
  }

  // compute width of chars in string, ignoring char spacing and word
  // spacing -- the Tj operator will adjust for the metrics of the
  // font that's actually used
  dx = dy = 0;
  nChars = 0;
  p = s->getCString();
  len = s->getLength();
  s2 = new GooString();
  while (len > 0) {
    n = font->getNextChar(p, len, &code,
			  &u, &uLen,
			  &dx2, &dy2, &originX, &originY);
    if (font->isCIDFont()) {
      if (uMap) {
	for (i = 0; i < uLen; ++i) {
	  m = uMap->mapUnicode(u[i], buf, (int)sizeof(buf));
	  for (j = 0; j < m; ++j) {
	    s2->append(buf[j]);
	  }
	}
	//~ this really needs to get the number of chars in the target
	//~ encoding - which may be more than the number of Unicode
	//~ chars
	nChars += uLen;
      } else {
	s2->append((char)((code >> 8) & 0xff));
	s2->append((char)(code & 0xff));
	++nChars;
      }
    } else {
      if (!codeToGID || codeToGID[code]) {
	s2->append((char)code);
      }
    }
    dx += dx2;
    dy += dy2;
    p += n;
    len -= n;
  }
  dx *= state->getFontSize() * state->getHorizScaling();
  dy *= state->getFontSize();
  if (uMap) {
    uMap->decRefCnt();
  }

  if (s2->getLength() > 0) {
    writePSString(s2);
    if (font->isCIDFont()) {
      if (wMode) {
	writePSFmt(" {0:d} {1:.6g} Tj16V\n", nChars, dy);
      } else {
	writePSFmt(" {0:d} {1:.6g} Tj16\n", nChars, dx);
      }
    } else {
      writePSFmt(" {0:.6g} Tj\n", dx);
    }
  }
  delete s2;

  if (state->getRender() & 4 || haveCSPattern) {
    haveTextClip = gTrue;
  }
}

void PSOutputDev::beginTextObject(GfxState *state) {
  if (state->getFillColorSpace()->getMode() == csPattern) {
    saveState(state);
    haveCSPattern = gTrue;
    writePS("true Tp\n");
  }
}

void PSOutputDev::endTextObject(GfxState *state) {
  if (haveCSPattern) {
    if (haveTextClip) {
      writePS("Tclip*\n");
      haveTextClip = gFalse;
      if (state->getFillColorSpace()->getMode() != csPattern) {
        double cxMin, cyMin, cxMax, cyMax;
        state->getClipBBox(&cxMin, &cyMin, &cxMax, &cyMax);
        writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} re\n",
                   cxMin, cyMin,
                   cxMax, cyMax);
        writePS("f*\n");
        restoreState(state);
        updateFillColor(state);
      }
    }
    haveCSPattern = gFalse;
  } else if (haveTextClip) {
    writePS("Tclip\n");
    haveTextClip = gFalse;
  }
}

void PSOutputDev::endMaskClip(GfxState * state) {
  writePS("pdfImClipEnd\n");
}

void PSOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
				int width, int height, GBool invert,
				GBool interpolate, GBool inlineImg) {
  int len;

  len = height * ((width + 7) / 8);
  if (state->getFillColorSpace()->getMode() == csPattern && (level != psLevel1 && level != psLevel1Sep)) {
    maskToClippingPath(str, width, height, invert);
  } else {
    switch (level) {
      case psLevel1:
      case psLevel1Sep:
        doImageL1(ref, NULL, invert, inlineImg, str, width, height, len,
                  NULL, NULL, 0, 0, gFalse);
      break;
      case psLevel2:
      case psLevel2Sep:
        doImageL2(ref, NULL, invert, inlineImg, str, width, height, len,
                  NULL, NULL, 0, 0, gFalse);
      break;
      case psLevel3:
      case psLevel3Sep:
        doImageL3(ref, NULL, invert, inlineImg, str, width, height, len,
                  NULL, NULL, 0, 0, gFalse);
      break;
    }
  }
}

void PSOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
			    int width, int height, GfxImageColorMap *colorMap,
			    GBool interpolate, int *maskColors, GBool inlineImg) {
  int len;

  len = height * ((width * colorMap->getNumPixelComps() *
		   colorMap->getBits() + 7) / 8);
  switch (level) {
  case psLevel1:
    doImageL1(ref, colorMap, gFalse, inlineImg, str,
	     width, height, len, maskColors, NULL, 0, 0, gFalse);
    break;
  case psLevel1Sep:
    //~ handle indexed, separation, ... color spaces
    doImageL1Sep(ref, colorMap, gFalse, inlineImg, str,
	     width, height, len, maskColors, NULL, 0, 0, gFalse);
    break;
  case psLevel2:
  case psLevel2Sep:
    doImageL2(ref, colorMap, gFalse, inlineImg, str,
	      width, height, len, maskColors, NULL, 0, 0, gFalse);
    break;
  case psLevel3:
  case psLevel3Sep:
    doImageL3(ref, colorMap, gFalse, inlineImg, str,
	      width, height, len, maskColors, NULL, 0, 0, gFalse);
    break;
  }
  t3Cacheable = gFalse;
}

void PSOutputDev::drawMaskedImage(GfxState *state, Object *ref, Stream *str,
				  int width, int height,
				  GfxImageColorMap *colorMap,
				  GBool interpolate,
				  Stream *maskStr,
				  int maskWidth, int maskHeight,
				  GBool maskInvert, GBool maskInterpolate) {
  int len;

  len = height * ((width * colorMap->getNumPixelComps() *
		   colorMap->getBits() + 7) / 8);
  switch (level) {
  case psLevel1:
    doImageL1(ref, colorMap, gFalse, gFalse, str, width, height, len,
	      NULL, maskStr, maskWidth, maskHeight, maskInvert);    
    break;
  case psLevel1Sep:
    //~ handle indexed, separation, ... color spaces
    doImageL1Sep(ref, colorMap, gFalse, gFalse, str, width, height, len,
	      NULL, maskStr, maskWidth, maskHeight, maskInvert);    
    break;
  case psLevel2:
  case psLevel2Sep:
    doImageL2(ref, colorMap, gFalse, gFalse, str, width, height, len,
	      NULL, maskStr, maskWidth, maskHeight, maskInvert);
    break;
  case psLevel3:
  case psLevel3Sep:
    doImageL3(ref, colorMap, gFalse, gFalse, str, width, height, len,
	      NULL, maskStr, maskWidth, maskHeight, maskInvert);
    break;
  }
  t3Cacheable = gFalse;
}

void PSOutputDev::doImageL1(Object *ref, GfxImageColorMap *colorMap,
			    GBool invert, GBool inlineImg,
			    Stream *str, int width, int height, int len,
			    int *maskColors, Stream *maskStr,
			    int maskWidth, int maskHeight, GBool maskInvert) {
  ImageStream *imgStr;
  Guchar pixBuf[gfxColorMaxComps];
  GfxGray gray;
  int col, x, y, c, i;
  char hexBuf[32*2 + 2];	// 32 values X 2 chars/value + line ending + null
  Guchar digit, grayValue;

  // explicit masking
  if (maskStr && !(maskColors && colorMap)) {
    maskToClippingPath(maskStr, maskWidth, maskHeight, maskInvert);
  }

  if ((inType3Char || preload) && !colorMap) {
    if (inlineImg) {
      // create an array
      str = new FixedLengthEncoder(str, len);
      str = new ASCIIHexEncoder(str);
      str->reset();
      col = 0;
      writePS("[<");
      do {
	do {
	  c = str->getChar();
	} while (c == '\n' || c == '\r');
	if (c == '>' || c == EOF) {
	  break;
	}
	writePSChar(c);
	++col;
	// each line is: "<...data...><eol>"
	// so max data length = 255 - 4 = 251
	// but make it 240 just to be safe
	// chunks are 2 bytes each, so we need to stop on an even col number
	if (col == 240) {
	  writePS(">\n<");
	  col = 0;
	}
      } while (c != '>' && c != EOF);
      writePS(">]\n");
      writePS("0\n");
      str->close();
      delete str;
    } else {
      // make sure the image is setup, it sometimes is not like on bug #17645
      setupImage(ref->getRef(), str);
      // set up to use the array already created by setupImages()
      writePSFmt("ImData_{0:d}_{1:d} 0 0\n", ref->getRefNum(), ref->getRefGen());
    }
  }

  // image/imagemask command
  if ((inType3Char || preload) && !colorMap) {
    writePSFmt("{0:d} {1:d} {2:s} [{3:d} 0 0 {4:d} 0 {5:d}] pdfImM1a\n",
	       width, height, invert ? "true" : "false",
	       width, -height, height);
  } else if (colorMap) {
    writePSFmt("{0:d} {1:d} 8 [{2:d} 0 0 {3:d} 0 {4:d}] pdfIm1\n",
	       width, height,
	       width, -height, height);
  } else {
    writePSFmt("{0:d} {1:d} {2:s} [{3:d} 0 0 {4:d} 0 {5:d}] pdfImM1\n",
	       width, height, invert ? "true" : "false",
	       width, -height, height);
  }

  // image data
  if (!((inType3Char || preload) && !colorMap)) {

    if (colorMap) {

      // set up to process the data stream
      imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(),
			       colorMap->getBits());
      imgStr->reset();

      // process the data stream
      i = 0;
      for (y = 0; y < height; ++y) {

	// write the line
	for (x = 0; x < width; ++x) {
	  imgStr->getPixel(pixBuf);
	  colorMap->getGray(pixBuf, &gray);
	  grayValue = colToByte(gray);
	  digit = grayValue / 16;
	  hexBuf[i++] = digit + ((digit >= 10)? 'a' - 10: '0');
	  digit = grayValue % 16;
	  hexBuf[i++] = digit + ((digit >= 10)? 'a' - 10: '0');
	  if (i >= 64) {
	    hexBuf[i++] = '\n';
	    writePSBuf(hexBuf, i);
	    i = 0;
	  }
	}
      }
      if (i != 0) {
	hexBuf[i++] = '\n';
	writePSBuf(hexBuf, i);
      }
      str->close();
      delete imgStr;

    // imagemask
    } else {
      str->reset();
      i = 0;
      for (y = 0; y < height; ++y) {
	for (x = 0; x < width; x += 8) {
	  grayValue = str->getChar();
	  digit = grayValue / 16;
	  hexBuf[i++] = digit + ((digit >= 10)? 'a' - 10: '0');
	  digit = grayValue % 16;
	  hexBuf[i++] = digit + ((digit >= 10)? 'a' - 10: '0');
	  if (i >= 64) {
	    hexBuf[i++] = '\n';
	    writePSBuf(hexBuf, i);
	    i = 0;
	  }
	}
      }
      if (i != 0) {
	hexBuf[i++] = '\n';
	writePSBuf(hexBuf, i);
      }
      str->close();
    }
  }

  if (maskStr && !(maskColors && colorMap)) {
    writePS("pdfImClipEnd\n");
  }
}

void PSOutputDev::doImageL1Sep(Object *ref, GfxImageColorMap *colorMap,
			       GBool invert, GBool inlineImg,
			       Stream *str, int width, int height, int len,
			       int *maskColors, Stream *maskStr,
			       int maskWidth, int maskHeight, GBool maskInvert) {
  ImageStream *imgStr;
  Guchar *lineBuf;
  Guchar pixBuf[gfxColorMaxComps];
  GfxCMYK cmyk;
  int x, y, i, comp;
  GBool checkProcessColor;
  char hexBuf[32*2 + 2];	// 32 values X 2 chars/value + line ending + null
  Guchar digit;

  // explicit masking
  if (maskStr && !(maskColors && colorMap)) {
    maskToClippingPath(maskStr, maskWidth, maskHeight, maskInvert);
  }

  // width, height, matrix, bits per component
  writePSFmt("{0:d} {1:d} 8 [{2:d} 0 0 {3:d} 0 {4:d}] pdfIm1Sep\n",
	     width, height,
	     width, -height, height);

  // allocate a line buffer
  lineBuf = (Guchar *)gmallocn(width, 4);

  // set up to process the data stream
  imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(),
			   colorMap->getBits());
  imgStr->reset();

  // process the data stream
  checkProcessColor = gTrue;
  i = 0;
  for (y = 0; y < height; ++y) {

    // read the line
    if (checkProcessColor) {
      checkProcessColor = (((psProcessCyan | psProcessMagenta | psProcessYellow | psProcessBlack) & ~processColors) != 0);
    }
    if (checkProcessColor) {
      for (x = 0; x < width; ++x) {
        imgStr->getPixel(pixBuf);
        colorMap->getCMYK(pixBuf, &cmyk);
        lineBuf[4*x+0] = colToByte(cmyk.c);
        lineBuf[4*x+1] = colToByte(cmyk.m);
        lineBuf[4*x+2] = colToByte(cmyk.y);
        lineBuf[4*x+3] = colToByte(cmyk.k);
        addProcessColor(colToDbl(cmyk.c), colToDbl(cmyk.m),
		        colToDbl(cmyk.y), colToDbl(cmyk.k));
      }
    } else {
      for (x = 0; x < width; ++x) {
        imgStr->getPixel(pixBuf);
        colorMap->getCMYK(pixBuf, &cmyk);
        lineBuf[4*x+0] = colToByte(cmyk.c);
        lineBuf[4*x+1] = colToByte(cmyk.m);
        lineBuf[4*x+2] = colToByte(cmyk.y);
        lineBuf[4*x+3] = colToByte(cmyk.k);
      }
    }

    // write one line of each color component
    for (comp = 0; comp < 4; ++comp) {
      for (x = 0; x < width; ++x) {
	digit = lineBuf[4*x + comp] / 16;
	hexBuf[i++] = digit + ((digit >= 10)? 'a'-10: '0');
	digit = lineBuf[4*x + comp] % 16;
	hexBuf[i++] = digit + ((digit >= 10)? 'a'-10: '0');
	if (i >= 64) {
	  hexBuf[i++] = '\n';
	  writePSBuf(hexBuf, i);
	  i = 0;
	}
      }
    }
  }

  if (i != 0) {
    hexBuf[i++] = '\n';
    writePSBuf(hexBuf, i);
  }

  str->close();
  delete imgStr;
  gfree(lineBuf);

  if (maskStr && !(maskColors && colorMap)) {
    writePS("pdfImClipEnd\n");
  }
}

void PSOutputDev::maskToClippingPath(Stream *maskStr, int maskWidth, int maskHeight, GBool maskInvert) {
  ImageStream *imgStr;
  Guchar *line;
  PSOutImgClipRect *rects0, *rects1, *rectsTmp, *rectsOut;
  int rects0Len, rects1Len, rectsSize, rectsOutLen, rectsOutSize;
  GBool emitRect, addRect, extendRect;
  int i, x0, x1, y, maskXor;

  imgStr = new ImageStream(maskStr, maskWidth, 1, 1);
  imgStr->reset();
  rects0Len = rects1Len = rectsOutLen = 0;
  rectsSize = rectsOutSize = 64;
  rects0 = (PSOutImgClipRect *)gmallocn(rectsSize, sizeof(PSOutImgClipRect));
  rects1 = (PSOutImgClipRect *)gmallocn(rectsSize, sizeof(PSOutImgClipRect));
  rectsOut = (PSOutImgClipRect *)gmallocn(rectsOutSize, sizeof(PSOutImgClipRect));
  maskXor = maskInvert ? 1 : 0;
  for (y = 0; y < maskHeight; ++y) {
    if (!(line = imgStr->getLine())) {
      break;
    }
    i = 0;
    rects1Len = 0;
    for (x0 = 0; x0 < maskWidth && (line[x0] ^ maskXor); ++x0) ;
    for (x1 = x0; x1 < maskWidth && !(line[x1] ^ maskXor); ++x1) ;
    while (x0 < maskWidth || i < rects0Len) {
      emitRect = addRect = extendRect = gFalse;
      if (x0 >= maskWidth) {
        emitRect = gTrue;
      } else if (i >= rects0Len) {
        addRect = gTrue;
      } else if (rects0[i].x0 < x0) {
        emitRect = gTrue;
      } else if (x0 < rects0[i].x0) {
        addRect = gTrue;
      } else if (rects0[i].x1 == x1) {
        extendRect = gTrue;
      } else {
        emitRect = addRect = gTrue;
      }
      if (emitRect) {
        if (rectsOutLen == rectsOutSize) {
          rectsOutSize *= 2;
          rectsOut = (PSOutImgClipRect *)greallocn(rectsOut, rectsOutSize, sizeof(PSOutImgClipRect));
        }
        rectsOut[rectsOutLen].x0 = rects0[i].x0;
        rectsOut[rectsOutLen].x1 = rects0[i].x1;
        rectsOut[rectsOutLen].y0 = maskHeight - y - 1;
        rectsOut[rectsOutLen].y1 = maskHeight - rects0[i].y0 - 1;
        ++rectsOutLen;
        ++i;
      }
      if (addRect || extendRect) {
        if (rects1Len == rectsSize) {
          rectsSize *= 2;
          rects0 = (PSOutImgClipRect *)greallocn(rects0, rectsSize, sizeof(PSOutImgClipRect));
          rects1 = (PSOutImgClipRect *)greallocn(rects1, rectsSize, sizeof(PSOutImgClipRect));
        }
        rects1[rects1Len].x0 = x0;
        rects1[rects1Len].x1 = x1;
        if (addRect) {
          rects1[rects1Len].y0 = y;
        }
        if (extendRect) {
          rects1[rects1Len].y0 = rects0[i].y0;
          ++i;
        }
        ++rects1Len;
        for (x0 = x1; x0 < maskWidth && (line[x0] ^ maskXor); ++x0) ;
        for (x1 = x0; x1 < maskWidth && !(line[x1] ^ maskXor); ++x1) ;
      }
    }
    rectsTmp = rects0;
    rects0 = rects1;
    rects1 = rectsTmp;
    i = rects0Len;
    rects0Len = rects1Len;
    rects1Len = i;
  }
  for (i = 0; i < rects0Len; ++i) {
    if (rectsOutLen == rectsOutSize) {
      rectsOutSize *= 2;
      rectsOut = (PSOutImgClipRect *)greallocn(rectsOut, rectsOutSize, sizeof(PSOutImgClipRect));
    }
    rectsOut[rectsOutLen].x0 = rects0[i].x0;
    rectsOut[rectsOutLen].x1 = rects0[i].x1;
    rectsOut[rectsOutLen].y0 = maskHeight - y - 1;
    rectsOut[rectsOutLen].y1 = maskHeight - rects0[i].y0 - 1;
    ++rectsOutLen;
  }
  if (rectsOutLen < 65536/4) {
    writePSFmt("{0:d} array 0\n", rectsOutLen * 4);
    for (i = 0; i < rectsOutLen; ++i) {
      writePSFmt("[{0:d} {1:d} {2:d} {3:d}] pr\n",
		 rectsOut[i].x0, rectsOut[i].y0,
		 rectsOut[i].x1 - rectsOut[i].x0,
		 rectsOut[i].y1 - rectsOut[i].y0);
    }
    writePSFmt("pop {0:d} {1:d} pdfImClip\n", maskWidth, maskHeight);
  } else {
    //  would be over the limit of array size.
    //  make each rectangle path and clip.
    writePS("gsave newpath\n");
    for (i = 0; i < rectsOutLen; ++i) {
      writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} re\n",
		 ((double)rectsOut[i].x0)/maskWidth,
		 ((double)rectsOut[i].y0)/maskHeight,
		 ((double)(rectsOut[i].x1 - rectsOut[i].x0))/maskWidth,
		 ((double)(rectsOut[i].y1 - rectsOut[i].y0))/maskHeight);
    }
    writePS("clip\n");
  }
  gfree(rectsOut);
  gfree(rects0);
  gfree(rects1);
  delete imgStr;
  maskStr->close();
}

void PSOutputDev::doImageL2(Object *ref, GfxImageColorMap *colorMap,
			    GBool invert, GBool inlineImg,
			    Stream *str, int width, int height, int len,
			    int *maskColors, Stream *maskStr,
			    int maskWidth, int maskHeight, GBool maskInvert) {
  Stream *str2;
  ImageStream *imgStr;
  Guchar *line;
  PSOutImgClipRect *rects0, *rects1, *rectsTmp, *rectsOut;
  int rects0Len, rects1Len, rectsSize, rectsOutLen, rectsOutSize;
  GBool emitRect, addRect, extendRect;
  GooString *s;
  int n, numComps;
  GBool useRLE, useASCII, useASCIIHex, useCompressed;
  GfxSeparationColorSpace *sepCS;
  GfxColor color;
  GfxCMYK cmyk;
  int c;
  int col, i, j, x0, x1, y;
  char dataBuf[4096];
  
  rectsOutLen = 0;

  // color key masking
  if (maskColors && colorMap && !inlineImg) {
    // can't read the stream twice for inline images -- but masking
    // isn't allowed with inline images anyway
    numComps = colorMap->getNumPixelComps();
    imgStr = new ImageStream(str, width, numComps, colorMap->getBits());
    imgStr->reset();
    rects0Len = rects1Len = 0;
    rectsSize = rectsOutSize = 64;
    rects0 = (PSOutImgClipRect *)gmallocn(rectsSize, sizeof(PSOutImgClipRect));
    rects1 = (PSOutImgClipRect *)gmallocn(rectsSize, sizeof(PSOutImgClipRect));
    rectsOut = (PSOutImgClipRect *)gmallocn(rectsOutSize,
					    sizeof(PSOutImgClipRect));
    for (y = 0; y < height; ++y) {
      if (!(line = imgStr->getLine())) {
	break;
      }
      i = 0;
      rects1Len = 0;
      for (x0 = 0; x0 < width; ++x0) {
	for (j = 0; j < numComps; ++j) {
	  if (line[x0*numComps+j] < maskColors[2*j] ||
	      line[x0*numComps+j] > maskColors[2*j+1]) {
	    break;
	  }
	}
	if (j < numComps) {
	  break;
	}
      }
      for (x1 = x0; x1 < width; ++x1) {
	for (j = 0; j < numComps; ++j) {
	  if (line[x1*numComps+j] < maskColors[2*j] ||
	      line[x1*numComps+j] > maskColors[2*j+1]) {
	    break;
	  }
	}
	if (j == numComps) {
	  break;
	}
      }
      while (x0 < width || i < rects0Len) {
	emitRect = addRect = extendRect = gFalse;
	if (x0 >= width) {
	  emitRect = gTrue;
	} else if (i >= rects0Len) {
	  addRect = gTrue;
	} else if (rects0[i].x0 < x0) {
	  emitRect = gTrue;
	} else if (x0 < rects0[i].x0) {
	  addRect = gTrue;
	} else if (rects0[i].x1 == x1) {
	  extendRect = gTrue;
	} else {
	  emitRect = addRect = gTrue;
	}
	if (emitRect) {
	  if (rectsOutLen == rectsOutSize) {
	    rectsOutSize *= 2;
	    rectsOut = (PSOutImgClipRect *)greallocn(rectsOut, rectsOutSize,
						     sizeof(PSOutImgClipRect));
	  }
	  rectsOut[rectsOutLen].x0 = rects0[i].x0;
	  rectsOut[rectsOutLen].x1 = rects0[i].x1;
	  rectsOut[rectsOutLen].y0 = height - y - 1;
	  rectsOut[rectsOutLen].y1 = height - rects0[i].y0 - 1;
	  ++rectsOutLen;
	  ++i;
	}
	if (addRect || extendRect) {
	  if (rects1Len == rectsSize) {
	    rectsSize *= 2;
	    rects0 = (PSOutImgClipRect *)greallocn(rects0, rectsSize,
						   sizeof(PSOutImgClipRect));
	    rects1 = (PSOutImgClipRect *)greallocn(rects1, rectsSize,
						   sizeof(PSOutImgClipRect));
	  }
	  rects1[rects1Len].x0 = x0;
	  rects1[rects1Len].x1 = x1;
	  if (addRect) {
	    rects1[rects1Len].y0 = y;
	  }
	  if (extendRect) {
	    rects1[rects1Len].y0 = rects0[i].y0;
	    ++i;
	  }
	  ++rects1Len;
	  for (x0 = x1; x0 < width; ++x0) {
	    for (j = 0; j < numComps; ++j) {
	      if (line[x0*numComps+j] < maskColors[2*j] ||
		  line[x0*numComps+j] > maskColors[2*j+1]) {
		break;
	      }
	    }
	    if (j < numComps) {
	      break;
	    }
	  }
	  for (x1 = x0; x1 < width; ++x1) {
	    for (j = 0; j < numComps; ++j) {
	      if (line[x1*numComps+j] < maskColors[2*j] ||
		  line[x1*numComps+j] > maskColors[2*j+1]) {
		break;
	      }
	    }
	    if (j == numComps) {
	      break;
	    }
	  }
	}
      }
      rectsTmp = rects0;
      rects0 = rects1;
      rects1 = rectsTmp;
      i = rects0Len;
      rects0Len = rects1Len;
      rects1Len = i;
    }
    for (i = 0; i < rects0Len; ++i) {
      if (rectsOutLen == rectsOutSize) {
	rectsOutSize *= 2;
	rectsOut = (PSOutImgClipRect *)greallocn(rectsOut, rectsOutSize,
						 sizeof(PSOutImgClipRect));
      }
      rectsOut[rectsOutLen].x0 = rects0[i].x0;
      rectsOut[rectsOutLen].x1 = rects0[i].x1;
      rectsOut[rectsOutLen].y0 = height - y - 1;
      rectsOut[rectsOutLen].y1 = height - rects0[i].y0 - 1;
      ++rectsOutLen;
    }
    if (rectsOutLen < 65536/4) {
      writePSFmt("{0:d} array 0\n", rectsOutLen * 4);
      for (i = 0; i < rectsOutLen; ++i) {
	writePSFmt("[{0:d} {1:d} {2:d} {3:d}] pr\n",
		   rectsOut[i].x0, rectsOut[i].y0,
		   rectsOut[i].x1 - rectsOut[i].x0,
		   rectsOut[i].y1 - rectsOut[i].y0);
      }
      writePSFmt("pop {0:d} {1:d} pdfImClip\n", width, height);
    } else {
      //  would be over the limit of array size.
      //  make each rectangle path and clip.
      writePS("gsave newpath\n");
      for (i = 0; i < rectsOutLen; ++i) {
	writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} re\n",
		   ((double)rectsOut[i].x0)/width,
		   ((double)rectsOut[i].y0)/height,
		   ((double)(rectsOut[i].x1 - rectsOut[i].x0))/width,
		   ((double)(rectsOut[i].y1 - rectsOut[i].y0))/height);
      }
      writePS("clip\n");
    }
    gfree(rectsOut);
    gfree(rects0);
    gfree(rects1);
    delete imgStr;
    str->close();

  // explicit masking
  } else if (maskStr) {
    maskToClippingPath(maskStr, maskWidth, maskHeight, maskInvert);
  }

  // color space
  if (colorMap) {
    dumpColorSpaceL2(colorMap->getColorSpace(), gFalse, gTrue, gFalse);
    writePS(" setcolorspace\n");
  }

  useASCIIHex = globalParams->getPSASCIIHex();

  // set up the image data
  if (mode == psModeForm || inType3Char || preload) {
    if (inlineImg) {
      // create an array
      str2 = new FixedLengthEncoder(str, len);
      str2 = new RunLengthEncoder(str2);
      if (useASCIIHex) {
	str2 = new ASCIIHexEncoder(str2);
      } else {
	str2 = new ASCII85Encoder(str2);
      }
      str2->reset();
      col = 0;
      writePS((char *)(useASCIIHex ? "[<" : "[<~"));
      do {
	do {
	  c = str2->getChar();
	} while (c == '\n' || c == '\r');
	if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	  break;
	}
	if (c == 'z') {
	  writePSChar(c);
	  ++col;
	} else {
	  writePSChar(c);
	  ++col;
	  for (i = 1; i <= (useASCIIHex ? 1 : 4); ++i) {
	    do {
	      c = str2->getChar();
	    } while (c == '\n' || c == '\r');
	    if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	      break;
	    }
	    writePSChar(c);
	    ++col;
	  }
	}
	// each line is: "<~...data...~><eol>"
	// so max data length = 255 - 6 = 249
	// chunks are 1 or 5 bytes each, so we have to stop at 245
	// but make it 240 just to be safe
	if (col > 240) {
	  writePS((char *)(useASCIIHex ? ">\n<" : "~>\n<~"));
	  col = 0;
	}
      } while (c != (useASCIIHex ? '>' : '~') && c != EOF);
      writePS((char *)(useASCIIHex ? ">\n" : "~>\n"));
      // add an extra entry because the RunLengthDecode filter may
      // read past the end
      writePS("<>]\n");
      writePS("0\n");
      str2->close();
      delete str2;
    } else {
      // make sure the image is setup, it sometimes is not like on bug #17645
      setupImage(ref->getRef(), str);
      // set up to use the array already created by setupImages()
      writePSFmt("ImData_{0:d}_{1:d} 0 0\n",ref->getRefNum(), ref->getRefGen());
    }
  }

  // image dictionary
  writePS("<<\n  /ImageType 1\n");

  // width, height, matrix, bits per component
  writePSFmt("  /Width {0:d}\n", width);
  writePSFmt("  /Height {0:d}\n", height);
  writePSFmt("  /ImageMatrix [{0:d} 0 0 {1:d} 0 {2:d}]\n",
	     width, -height, height);
  if (colorMap && colorMap->getColorSpace()->getMode() == csDeviceN) {
    writePS("  /BitsPerComponent 8\n");
  } else {
    writePSFmt("  /BitsPerComponent {0:d}\n",
	       colorMap ? colorMap->getBits() : 1);
  }

  // decode 
  if (colorMap) {
    writePS("  /Decode [");
    if ((level == psLevel2Sep || level == psLevel3Sep) &&
	colorMap->getColorSpace()->getMode() == csSeparation) {
      // this matches up with the code in the pdfImSep operator
      n = (1 << colorMap->getBits()) - 1;
      writePSFmt("{0:.4g} {1:.4g}", colorMap->getDecodeLow(0) * n,
		 colorMap->getDecodeHigh(0) * n);
    } else if (colorMap->getColorSpace()->getMode() == csDeviceN) {
      numComps = ((GfxDeviceNColorSpace *)colorMap->getColorSpace())->
	           getAlt()->getNComps();
      for (i = 0; i < numComps; ++i) {
	if (i > 0) {
	  writePS(" ");
	}
	writePS("0 1");
      }
    } else {
      numComps = colorMap->getNumPixelComps();
      for (i = 0; i < numComps; ++i) {
	if (i > 0) {
	  writePS(" ");
	}
	writePSFmt("{0:.4g} {1:.4g}",
		   colorMap->getDecodeLow(i), colorMap->getDecodeHigh(i));
      }
    }
    writePS("]\n");
  } else {
    writePSFmt("  /Decode [{0:d} {1:d}]\n", invert ? 1 : 0, invert ? 0 : 1);
  }

  // data source
  if (mode == psModeForm || inType3Char || preload) {
    if (inlineImg) {
      writePS("  /DataSource { 2 copy get exch 1 add exch }\n");
    } else {
      writePS("  /DataSource { dup 65535 ge { pop 1 add 0 } if 2 index 2"
	" index get 1 index get exch 1 add exch }\n");
    }
  } else {
    writePS("  /DataSource currentfile\n");
  }

  // filters
  s = str->getPSFilter(level < psLevel2 ? 1 : level < psLevel3 ? 2 : 3,
		       "    ");
  if ((colorMap && colorMap->getColorSpace()->getMode() == csDeviceN) ||
      inlineImg || !s) {
    useRLE = gTrue;
    useASCII = !(mode == psModeForm || inType3Char || preload);
    useCompressed = gFalse;
  } else {
    useRLE = gFalse;
    useASCII = str->isBinary() &&
               !(mode == psModeForm || inType3Char || preload);
    useCompressed = gTrue;
  }
  if (useASCII) {
    writePSFmt("    /ASCII{0:s}Decode filter\n",
	       useASCIIHex ? "Hex" : "85");
  }
  if (useRLE) {
    writePS("    /RunLengthDecode filter\n");
  }
  if (useCompressed) {
    writePS(s->getCString());
  }
  if (s) {
    delete s;
  }

  if (mode == psModeForm || inType3Char || preload) {

    // end of image dictionary
    writePSFmt(">>\n{0:s}\n", colorMap ? "image" : "imagemask");

    // get rid of the array and index
    if (!inlineImg) writePS("pop ");
    writePS("pop pop\n");

  } else {

    // cut off inline image streams at appropriate length
    if (inlineImg) {
      str = new FixedLengthEncoder(str, len);
    } else if (useCompressed) {
      str = str->getUndecodedStream();
    }

    // recode DeviceN data
    if (colorMap && colorMap->getColorSpace()->getMode() == csDeviceN) {
      str = new DeviceNRecoder(str, width, height, colorMap);
    }

    // add RunLengthEncode and ASCIIHex/85 encode filters
    if (useRLE) {
      str = new RunLengthEncoder(str);
    }
    if (useASCII) {
      if (useASCIIHex) {
	str = new ASCIIHexEncoder(str);
      } else {
	str = new ASCII85Encoder(str);
      }
    }

    // end of image dictionary
    writePS(">>\n");
#if OPI_SUPPORT
    if (opi13Nest) {
      if (inlineImg) {
	// this can't happen -- OPI dictionaries are in XObjects
	error(-1, "Internal: OPI in inline image");
	n = 0;
      } else {
	// need to read the stream to count characters -- the length
	// is data-dependent (because of ASCII and RLE filters)
	str->reset();
	n = 0;
	while ((c = str->getChar()) != EOF) {
	  ++n;
	}
	str->close();
      }
      // +6/7 for "pdfIm\n" / "pdfImM\n"
      // +8 for newline + trailer
      n += colorMap ? 14 : 15;
      writePSFmt("%%BeginData: {0:d} Hex Bytes\n", n);
    }
#endif
    if ((level == psLevel2Sep || level == psLevel3Sep) && colorMap &&
	colorMap->getColorSpace()->getMode() == csSeparation) {
      color.c[0] = gfxColorComp1;
      sepCS = (GfxSeparationColorSpace *)colorMap->getColorSpace();
      sepCS->getCMYK(&color, &cmyk);
      writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} ({4:t}) pdfImSep\n",
		 colToDbl(cmyk.c), colToDbl(cmyk.m),
		 colToDbl(cmyk.y), colToDbl(cmyk.k),
		 sepCS->getName());
    } else {
      writePSFmt("{0:s}\n", colorMap ? "pdfIm" : "pdfImM");
    }

    // copy the stream data
    str->reset();
    i = 0;
    while ((c = str->getChar()) != EOF) {
      dataBuf[i++] = c;
      if (i >= (int)sizeof(dataBuf)) {
	writePSBuf(dataBuf, i);
	i = 0;
      }
    }
    if (i > 0) {
      writePSBuf(dataBuf, i);
    }
    str->close();

    // add newline and trailer to the end
    writePSChar('\n');
    writePS("%-EOD-\n");
#if OPI_SUPPORT
    if (opi13Nest) {
      writePS("%%EndData\n");
    }
#endif

    // delete encoders
    if (useRLE || useASCII || inlineImg) {
      delete str;
    }
  }

  if ((maskColors && colorMap && !inlineImg) || maskStr) {
    if (rectsOutLen < 65536/4) {
	writePS("pdfImClipEnd\n");
    } else {
	writePS("grestore\n");
    }
  }
}

//~ this doesn't currently support OPI
void PSOutputDev::doImageL3(Object *ref, GfxImageColorMap *colorMap,
			    GBool invert, GBool inlineImg,
			    Stream *str, int width, int height, int len,
			    int *maskColors, Stream *maskStr,
			    int maskWidth, int maskHeight, GBool maskInvert) {
  Stream *str2;
  GooString *s;
  int n, numComps;
  GBool useRLE, useASCII, useASCIIHex, useCompressed;
  GBool maskUseRLE, maskUseASCII, maskUseCompressed;
  GfxSeparationColorSpace *sepCS;
  GfxColor color;
  GfxCMYK cmyk;
  int c;
  int col, i;

  useASCIIHex = globalParams->getPSASCIIHex();
  useRLE = useASCII = useCompressed = gFalse; // make gcc happy
  maskUseRLE = maskUseASCII = maskUseCompressed = gFalse; // make gcc happy

  // color space
  if (colorMap) {
    dumpColorSpaceL2(colorMap->getColorSpace(), gFalse, gTrue, gFalse);
    writePS(" setcolorspace\n");
  }

  // set up the image data
  if (mode == psModeForm || inType3Char || preload) {
    if (inlineImg) {
      // create an array
      str2 = new FixedLengthEncoder(str, len);
      str2 = new RunLengthEncoder(str2);
      if (useASCIIHex) {
	str2 = new ASCIIHexEncoder(str2);
      } else {
	str2 = new ASCII85Encoder(str2);
      }
      str2->reset();
      col = 0;
      writePS((char *)(useASCIIHex ? "[<" : "[<~"));
      do {
	do {
	  c = str2->getChar();
	} while (c == '\n' || c == '\r');
	if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	  break;
	}
	if (c == 'z') {
	  writePSChar(c);
	  ++col;
	} else {
	  writePSChar(c);
	  ++col;
	  for (i = 1; i <= (useASCIIHex ? 1 : 4); ++i) {
	    do {
	      c = str2->getChar();
	    } while (c == '\n' || c == '\r');
	    if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	      break;
	    }
	    writePSChar(c);
	    ++col;
	  }
	}
	// each line is: "<~...data...~><eol>"
	// so max data length = 255 - 6 = 249
	// chunks are 1 or 5 bytes each, so we have to stop at 245
	// but make it 240 just to be safe
	if (col > 240) {
	  writePS((char *)(useASCIIHex ? ">\n<" : "~>\n<~"));
	  col = 0;
	}
      } while (c != (useASCIIHex ? '>' : '~') && c != EOF);
      writePS((char *)(useASCIIHex ? ">\n" : "~>\n"));
      // add an extra entry because the RunLengthDecode filter may
      // read past the end
      writePS("<>]\n");
      writePS("0\n");
      str2->close();
      delete str2;
    } else {
      // make sure the image is setup, it sometimes is not like on bug #17645
      setupImage(ref->getRef(), str);
      // set up to use the array already created by setupImages()
      writePSFmt("ImData_{0:d}_{1:d} 0 0\n", ref->getRefNum(), ref->getRefGen());
    }
  }

  // explicit masking
  if (maskStr) {
    writePS("<<\n  /ImageType 3\n");
    writePS("  /InterleaveType 3\n");
    writePS("  /DataDict\n");
  }

  // image (data) dictionary
  writePSFmt("<<\n  /ImageType {0:d}\n", (maskColors && colorMap) ? 4 : 1);

  // color key masking
  if (maskColors && colorMap) {
    writePS("  /MaskColor [\n");
    numComps = colorMap->getNumPixelComps();
    for (i = 0; i < 2 * numComps; i += 2) {
      writePSFmt("    {0:d} {1:d}\n", maskColors[i], maskColors[i+1]);
    }
    writePS("  ]\n");
  }

  // width, height, matrix, bits per component
  writePSFmt("  /Width {0:d}\n", width);
  writePSFmt("  /Height {0:d}\n", height);
  writePSFmt("  /ImageMatrix [{0:d} 0 0 {1:d} 0 {2:d}]\n",
	     width, -height, height);
  if (colorMap && colorMap->getColorSpace()->getMode() == csDeviceN) {
    writePS("  /BitsPerComponent 8\n");
  } else {
    writePSFmt("  /BitsPerComponent {0:d}\n",
	       colorMap ? colorMap->getBits() : 1);
  }

  // decode 
  if (colorMap) {
    writePS("  /Decode [");
    if ((level == psLevel2Sep || level == psLevel3Sep) &&
	colorMap->getColorSpace()->getMode() == csSeparation) {
      // this matches up with the code in the pdfImSep operator
      n = (1 << colorMap->getBits()) - 1;
      writePSFmt("{0:.4g} {1:.4g}", colorMap->getDecodeLow(0) * n,
		 colorMap->getDecodeHigh(0) * n);
    } else if (colorMap->getColorSpace()->getMode() == csDeviceN) {
      numComps = ((GfxDeviceNColorSpace *)colorMap->getColorSpace())->
	           getAlt()->getNComps();
      for (i = 0; i < numComps; ++i) {
	if (i > 0) {
	  writePS(" ");
	}
	writePS("0 1");
      }
    } else {
      numComps = colorMap->getNumPixelComps();
      for (i = 0; i < numComps; ++i) {
	if (i > 0) {
	  writePS(" ");
	}
	writePSFmt("{0:.4g} {1:.4g}", colorMap->getDecodeLow(i),
		   colorMap->getDecodeHigh(i));
      }
    }
    writePS("]\n");
  } else {
    writePSFmt("  /Decode [{0:d} {1:d}]\n", invert ? 1 : 0, invert ? 0 : 1);
  }

  // data source
  if (mode == psModeForm || inType3Char || preload) {
    if (inlineImg) {
	writePS("  /DataSource { 2 copy get exch 1 add exch }\n");
    } else {
	writePS("  /DataSource { dup 65535 ge { pop 1 add 0 } if 2 index 2"
	  " index get 1 index get exch 1 add exch }\n");
    }
  } else {
    writePS("  /DataSource currentfile\n");
  }

  // filters
  s = str->getPSFilter(level < psLevel2 ? 1 : level < psLevel3 ? 2 : 3,
		       "    ");
  if ((colorMap && colorMap->getColorSpace()->getMode() == csDeviceN) ||
      inlineImg || !s) {
    useRLE = gTrue;
    useASCII = !(mode == psModeForm || inType3Char || preload);
    useCompressed = gFalse;
  } else {
    useRLE = gFalse;
    useASCII = str->isBinary() &&
               !(mode == psModeForm || inType3Char || preload);
    useCompressed = gTrue;
  }
  if (useASCII) {
    writePSFmt("    /ASCII{0:s}Decode filter\n",
	       useASCIIHex ? "Hex" : "85");
  }
  if (useRLE) {
    writePS("    /RunLengthDecode filter\n");
  }
  if (useCompressed) {
    writePS(s->getCString());
  }
  if (s) {
    delete s;
  }

  // end of image (data) dictionary
  writePS(">>\n");

  // explicit masking
  if (maskStr) {
    writePS("  /MaskDict\n");
    writePS("<<\n");
    writePS("  /ImageType 1\n");
    writePSFmt("  /Width {0:d}\n", maskWidth);
    writePSFmt("  /Height {0:d}\n", maskHeight);
    writePSFmt("  /ImageMatrix [{0:d} 0 0 {1:d} 0 {2:d}]\n",
	       maskWidth, -maskHeight, maskHeight);
    writePS("  /BitsPerComponent 1\n");
    writePSFmt("  /Decode [{0:d} {1:d}]\n",
	       maskInvert ? 1 : 0, maskInvert ? 0 : 1);

    // mask data source
    writePS("  /DataSource currentfile\n");
    s = maskStr->getPSFilter(3, "    ");
    if (!s) {
      maskUseRLE = gTrue;
      maskUseASCII = gTrue;
      maskUseCompressed = gFalse;
    } else {
      maskUseRLE = gFalse;
      maskUseASCII = maskStr->isBinary();
      maskUseCompressed = gTrue;
    }
    if (maskUseASCII) {
      writePSFmt("    /ASCII{0:s}Decode filter\n",
		 useASCIIHex ? "Hex" : "85");
    }
    if (maskUseRLE) {
      writePS("    /RunLengthDecode filter\n");
    }
    if (maskUseCompressed) {
      writePS(s->getCString());
    }
    if (s) {
      delete s;
    }

    writePS(">>\n");
    writePS(">>\n");
  }

  if (mode == psModeForm || inType3Char || preload) {

    // image command
    writePSFmt("{0:s}\n", colorMap ? "image" : "imagemask");

  } else {

    if ((level == psLevel2Sep || level == psLevel3Sep) && colorMap &&
	colorMap->getColorSpace()->getMode() == csSeparation) {
      color.c[0] = gfxColorComp1;
      sepCS = (GfxSeparationColorSpace *)colorMap->getColorSpace();
      sepCS->getCMYK(&color, &cmyk);
      writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} ({4:t}) pdfImSep\n",
		 colToDbl(cmyk.c), colToDbl(cmyk.m),
		 colToDbl(cmyk.y), colToDbl(cmyk.k),
		 sepCS->getName());
    } else {
      writePSFmt("{0:s}\n", colorMap ? "pdfIm" : "pdfImM");
    }

  }

  // explicit masking
  if (maskStr) {

    if (maskUseCompressed) {
      maskStr = maskStr->getUndecodedStream();
    }

    // add RunLengthEncode and ASCIIHex/85 encode filters
    if (maskUseRLE) {
      maskStr = new RunLengthEncoder(maskStr);
    }
    if (maskUseASCII) {
      if (useASCIIHex) {
	maskStr = new ASCIIHexEncoder(maskStr);
      } else {
	maskStr = new ASCII85Encoder(maskStr);
      }
    }

    // copy the stream data
    maskStr->reset();
    while ((c = maskStr->getChar()) != EOF) {
      writePSChar(c);
    }
    maskStr->close();
    writePSChar('\n');

    // delete encoders
    if (maskUseRLE || maskUseASCII) {
      delete maskStr;
    }
  }

  // get rid of the array and index
  if (mode == psModeForm || inType3Char || preload) {
    if (!inlineImg) writePS("pop ");
    writePS("pop pop\n");

  // image data
  } else {

    // cut off inline image streams at appropriate length
    if (inlineImg) {
      str = new FixedLengthEncoder(str, len);
    } else if (useCompressed) {
      str = str->getUndecodedStream();
    }

    // recode DeviceN data
    if (colorMap && colorMap->getColorSpace()->getMode() == csDeviceN) {
      str = new DeviceNRecoder(str, width, height, colorMap);
    }

    // add RunLengthEncode and ASCIIHex/85 encode filters
    if (useRLE) {
      str = new RunLengthEncoder(str);
    }
    if (useASCII) {
      if (useASCIIHex) {
	str = new ASCIIHexEncoder(str);
      } else {
	str = new ASCII85Encoder(str);
      }
    }

    // copy the stream data
    str->reset();
    while ((c = str->getChar()) != EOF) {
      writePSChar(c);
    }
    str->close();

    // add newline and trailer to the end
    writePSChar('\n');
    writePS("%-EOD-\n");

    // delete encoders
    if (useRLE || useASCII || inlineImg) {
      delete str;
    }
  }
}

void PSOutputDev::dumpColorSpaceL2(GfxColorSpace *colorSpace,
				   GBool genXform, GBool updateColors,
				   GBool map01) {
  GfxCalGrayColorSpace *calGrayCS;
  GfxCalRGBColorSpace *calRGBCS;
  GfxLabColorSpace *labCS;
  GfxIndexedColorSpace *indexedCS;
  GfxSeparationColorSpace *separationCS;
  GfxDeviceNColorSpace *deviceNCS;
  GfxColorSpace *baseCS;
  Guchar *lookup, *p;
  double x[gfxColorMaxComps], y[gfxColorMaxComps];
  double low[gfxColorMaxComps], range[gfxColorMaxComps];
  GfxColor color;
  GfxCMYK cmyk;
  Function *func;
  int n, numComps, numAltComps;
  int byte;
  int i, j, k;

  switch (colorSpace->getMode()) {

  case csDeviceGray:
    writePS("/DeviceGray");
    if (genXform) {
      writePS(" {}");
    }
    if (updateColors) {
      processColors |= psProcessBlack;
    }
    break;

  case csCalGray:
    calGrayCS = (GfxCalGrayColorSpace *)colorSpace;
    writePS("[/CIEBasedA <<\n");
    writePSFmt(" /DecodeA {{{0:.4g} exp}} bind\n", calGrayCS->getGamma());
    writePSFmt(" /MatrixA [{0:.4g} {1:.4g} {2:.4g}]\n",
	       calGrayCS->getWhiteX(), calGrayCS->getWhiteY(),
	       calGrayCS->getWhiteZ());
    writePSFmt(" /WhitePoint [{0:.4g} {1:.4g} {2:.4g}]\n",
	       calGrayCS->getWhiteX(), calGrayCS->getWhiteY(),
	       calGrayCS->getWhiteZ());
    writePSFmt(" /BlackPoint [{0:.4g} {1:.4g} {2:.4g}]\n",
	       calGrayCS->getBlackX(), calGrayCS->getBlackY(),
	       calGrayCS->getBlackZ());
    writePS(">>]");
    if (genXform) {
      writePS(" {}");
    }
    if (updateColors) {
      processColors |= psProcessBlack;
    }
    break;

  case csDeviceRGB:
    writePS("/DeviceRGB");
    if (genXform) {
      writePS(" {}");
    }
    if (updateColors) {
      processColors |= psProcessCMYK;
    }
    break;

  case csCalRGB:
    calRGBCS = (GfxCalRGBColorSpace *)colorSpace;
    writePS("[/CIEBasedABC <<\n");
    writePSFmt(" /DecodeABC [{{{0:.4g} exp}} bind {{{1:.4g} exp}} bind {{{2:.4g} exp}} bind]\n",
	       calRGBCS->getGammaR(), calRGBCS->getGammaG(),
	       calRGBCS->getGammaB());
    writePSFmt(" /MatrixABC [{0:.4g} {1:.4g} {2:.4g} {3:.4g} {4:.4g} {5:.4g} {6:.4g} {7:.4g} {8:.4g}]\n",
	       calRGBCS->getMatrix()[0], calRGBCS->getMatrix()[1],
	       calRGBCS->getMatrix()[2], calRGBCS->getMatrix()[3],
	       calRGBCS->getMatrix()[4], calRGBCS->getMatrix()[5],
	       calRGBCS->getMatrix()[6], calRGBCS->getMatrix()[7],
	       calRGBCS->getMatrix()[8]);
    writePSFmt(" /WhitePoint [{0:.4g} {1:.4g} {2:.4g}]\n",
	       calRGBCS->getWhiteX(), calRGBCS->getWhiteY(),
	       calRGBCS->getWhiteZ());
    writePSFmt(" /BlackPoint [{0:.4g} {1:.4g} {2:.4g}]\n",
	       calRGBCS->getBlackX(), calRGBCS->getBlackY(),
	       calRGBCS->getBlackZ());
    writePS(">>]");
    if (genXform) {
      writePS(" {}");
    }
    if (updateColors) {
      processColors |= psProcessCMYK;
    }
    break;

  case csDeviceCMYK:
    writePS("/DeviceCMYK");
    if (genXform) {
      writePS(" {}");
    }
    if (updateColors) {
      processColors |= psProcessCMYK;
    }
    break;

  case csLab:
    labCS = (GfxLabColorSpace *)colorSpace;
    writePS("[/CIEBasedABC <<\n");
    if (map01) {
      writePS(" /RangeABC [0 1 0 1 0 1]\n");
      writePSFmt(" /DecodeABC [{{100 mul 16 add 116 div}} bind {{{0:.4g} mul {1:.4g} add}} bind {{{2:.4g} mul {3:.4g} add}} bind]\n",
		 (labCS->getAMax() - labCS->getAMin()) / 500.0,
		 labCS->getAMin() / 500.0,
		 (labCS->getBMax() - labCS->getBMin()) / 200.0,
		 labCS->getBMin() / 200.0);
    } else {
      writePSFmt(" /RangeABC [0 100 {0:.4g} {1:.4g} {2:.4g} {3:.4g}]\n",
		 labCS->getAMin(), labCS->getAMax(),
		 labCS->getBMin(), labCS->getBMax());
      writePS(" /DecodeABC [{16 add 116 div} bind {500 div} bind {200 div} bind]\n");
    }
    writePS(" /MatrixABC [1 1 1 1 0 0 0 0 -1]\n");
    writePS(" /DecodeLMN\n");
    writePS("   [{dup 6 29 div ge {dup dup mul mul}\n");
    writePSFmt("     {{4 29 div sub 108 841 div mul }} ifelse {0:.4g} mul}} bind\n",
	       labCS->getWhiteX());
    writePS("    {dup 6 29 div ge {dup dup mul mul}\n");
    writePSFmt("     {{4 29 div sub 108 841 div mul }} ifelse {0:.4g} mul}} bind\n",
	       labCS->getWhiteY());
    writePS("    {dup 6 29 div ge {dup dup mul mul}\n");
    writePSFmt("     {{4 29 div sub 108 841 div mul }} ifelse {0:.4g} mul}} bind]\n",
	       labCS->getWhiteZ());
    writePSFmt(" /WhitePoint [{0:.4g} {1:.4g} {2:.4g}]\n",
	       labCS->getWhiteX(), labCS->getWhiteY(), labCS->getWhiteZ());
    writePSFmt(" /BlackPoint [{0:.4g} {1:.4g} {2:.4g}]\n",
	       labCS->getBlackX(), labCS->getBlackY(), labCS->getBlackZ());
    writePS(">>]");
    if (genXform) {
      writePS(" {}");
    }
    if (updateColors) {
      processColors |= psProcessCMYK;
    }
    break;

  case csICCBased:
    // there is no transform function to the alternate color space, so
    // we can use it directly
    dumpColorSpaceL2(((GfxICCBasedColorSpace *)colorSpace)->getAlt(),
		     genXform, updateColors, gFalse);
    break;

  case csIndexed:
    indexedCS = (GfxIndexedColorSpace *)colorSpace;
    baseCS = indexedCS->getBase();
    writePS("[/Indexed ");
    dumpColorSpaceL2(baseCS, gFalse, gFalse, gTrue);
    n = indexedCS->getIndexHigh();
    numComps = baseCS->getNComps();
    lookup = indexedCS->getLookup();
    writePSFmt(" {0:d} <\n", n);
    if (baseCS->getMode() == csDeviceN) {
      func = ((GfxDeviceNColorSpace *)baseCS)->getTintTransformFunc();
      baseCS->getDefaultRanges(low, range, indexedCS->getIndexHigh());
      if (((GfxDeviceNColorSpace *)baseCS)->getAlt()->getMode() == csLab) {
	labCS = (GfxLabColorSpace *)((GfxDeviceNColorSpace *)baseCS)->getAlt();
      } else {
	labCS = NULL;
      }
      numAltComps = ((GfxDeviceNColorSpace *)baseCS)->getAlt()->getNComps();
      p = lookup;
      for (i = 0; i <= n; i += 8) {
	writePS("  ");
	for (j = i; j < i+8 && j <= n; ++j) {
	  for (k = 0; k < numComps; ++k) {
	    x[k] = low[k] + (*p++ / 255.0) * range[k];
	  }
	  func->transform(x, y);
	  if (labCS) {
	    y[0] /= 100.0;
	    y[1] = (y[1] - labCS->getAMin()) /
	           (labCS->getAMax() - labCS->getAMin());
	    y[2] = (y[2] - labCS->getBMin()) /
	           (labCS->getBMax() - labCS->getBMin());
	  }
	  for (k = 0; k < numAltComps; ++k) {
	    byte = (int)(y[k] * 255 + 0.5);
	    if (byte < 0) {
	      byte = 0;
	    } else if (byte > 255) {
	      byte = 255;
	    }
	    writePSFmt("{0:02x}", byte);
	  }
	  if (updateColors) {
	    color.c[0] = dblToCol(j);
	    indexedCS->getCMYK(&color, &cmyk);
	    addProcessColor(colToDbl(cmyk.c), colToDbl(cmyk.m),
			    colToDbl(cmyk.y), colToDbl(cmyk.k));
	  }
	}
	writePS("\n");
      }
    } else {
      for (i = 0; i <= n; i += 8) {
	writePS("  ");
	for (j = i; j < i+8 && j <= n; ++j) {
	  for (k = 0; k < numComps; ++k) {
	    writePSFmt("{0:02x}", lookup[j * numComps + k]);
	  }
	  if (updateColors) {
	    color.c[0] = dblToCol(j);
	    indexedCS->getCMYK(&color, &cmyk);
	    addProcessColor(colToDbl(cmyk.c), colToDbl(cmyk.m),
			    colToDbl(cmyk.y), colToDbl(cmyk.k));
	  }
	}
	writePS("\n");
      }
    }
    writePS(">]");
    if (genXform) {
      writePS(" {}");
    }
    break;

  case csSeparation:
    separationCS = (GfxSeparationColorSpace *)colorSpace;
    writePS("[/Separation ");
    writePSString(separationCS->getName());
    writePS(" ");
    dumpColorSpaceL2(separationCS->getAlt(), gFalse, gFalse, gFalse);
    writePS("\n");
    cvtFunction(separationCS->getFunc());
    writePS("]");
    if (genXform) {
      writePS(" {}");
    }
    if (updateColors) {
      addCustomColor(separationCS);
    }
    break;

  case csDeviceN:
    // DeviceN color spaces are a Level 3 PostScript feature.
    deviceNCS = (GfxDeviceNColorSpace *)colorSpace;
    dumpColorSpaceL2(deviceNCS->getAlt(), gFalse, updateColors, map01);
    if (genXform) {
      writePS(" ");
      cvtFunction(deviceNCS->getTintTransformFunc());
    }
    break;

  case csPattern:
    //~ unimplemented
    break;
  }
}

#if OPI_SUPPORT
void PSOutputDev::opiBegin(GfxState *state, Dict *opiDict) {
  Object dict;

  if (globalParams->getPSOPI()) {
    opiDict->lookup("2.0", &dict);
    if (dict.isDict()) {
      opiBegin20(state, dict.getDict());
      dict.free();
    } else {
      dict.free();
      opiDict->lookup("1.3", &dict);
      if (dict.isDict()) {
	opiBegin13(state, dict.getDict());
      }
      dict.free();
    }
  }
}

void PSOutputDev::opiBegin20(GfxState *state, Dict *dict) {
  Object obj1, obj2, obj3, obj4;
  double width, height, left, right, top, bottom;
  int w, h;
  int i;

  writePS("%%BeginOPI: 2.0\n");
  writePS("%%Distilled\n");

  dict->lookup("F", &obj1);
  if (getFileSpecName(&obj1, &obj2)) {
    writePSFmt("%%ImageFileName: {0:t}\n", obj2.getString());
    obj2.free();
  }
  obj1.free();

  dict->lookup("MainImage", &obj1);
  if (obj1.isString()) {
    writePSFmt("%%MainImage: {0:t}\n", obj1.getString());
  }
  obj1.free();

  //~ ignoring 'Tags' entry
  //~ need to use writePSString() and deal with >255-char lines

  dict->lookup("Size", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 2) {
    obj1.arrayGet(0, &obj2);
    width = obj2.getNum();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    height = obj2.getNum();
    obj2.free();
    writePSFmt("%%ImageDimensions: {0:.6g} {1:.6g}\n", width, height);
  }
  obj1.free();

  dict->lookup("CropRect", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 4) {
    obj1.arrayGet(0, &obj2);
    left = obj2.getNum();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    top = obj2.getNum();
    obj2.free();
    obj1.arrayGet(2, &obj2);
    right = obj2.getNum();
    obj2.free();
    obj1.arrayGet(3, &obj2);
    bottom = obj2.getNum();
    obj2.free();
    writePSFmt("%%ImageCropRect: {0:.6g} {1:.6g} {2:.6g} {3:.6g}\n",
	       left, top, right, bottom);
  }
  obj1.free();

  dict->lookup("Overprint", &obj1);
  if (obj1.isBool()) {
    writePSFmt("%%ImageOverprint: {0:s}\n", obj1.getBool() ? "true" : "false");
  }
  obj1.free();

  dict->lookup("Inks", &obj1);
  if (obj1.isName()) {
    writePSFmt("%%ImageInks: {0:s}\n", obj1.getName());
  } else if (obj1.isArray() && obj1.arrayGetLength() >= 1) {
    obj1.arrayGet(0, &obj2);
    if (obj2.isName()) {
      writePSFmt("%%ImageInks: {0:s} {1:d}",
		 obj2.getName(), (obj1.arrayGetLength() - 1) / 2);
      for (i = 1; i+1 < obj1.arrayGetLength(); i += 2) {
	obj1.arrayGet(i, &obj3);
	obj1.arrayGet(i+1, &obj4);
	if (obj3.isString() && obj4.isNum()) {
	  writePS(" ");
	  writePSString(obj3.getString());
	  writePSFmt(" {0:.6g}", obj4.getNum());
	}
	obj3.free();
	obj4.free();
      }
      writePS("\n");
    }
    obj2.free();
  }
  obj1.free();

  writePS("gsave\n");

  writePS("%%BeginIncludedImage\n");

  dict->lookup("IncludedImageDimensions", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 2) {
    obj1.arrayGet(0, &obj2);
    w = obj2.getInt();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    h = obj2.getInt();
    obj2.free();
    writePSFmt("%%IncludedImageDimensions: {0:d} {1:d}\n", w, h);
  }
  obj1.free();

  dict->lookup("IncludedImageQuality", &obj1);
  if (obj1.isNum()) {
    writePSFmt("%%IncludedImageQuality: {0:.6g}\n", obj1.getNum());
  }
  obj1.free();

  ++opi20Nest;
}

void PSOutputDev::opiBegin13(GfxState *state, Dict *dict) {
  Object obj1, obj2;
  int left, right, top, bottom, samples, bits, width, height;
  double c, m, y, k;
  double llx, lly, ulx, uly, urx, ury, lrx, lry;
  double tllx, tlly, tulx, tuly, turx, tury, tlrx, tlry;
  double horiz, vert;
  int i, j;

  writePS("save\n");
  writePS("/opiMatrix2 matrix currentmatrix def\n");
  writePS("opiMatrix setmatrix\n");

  dict->lookup("F", &obj1);
  if (getFileSpecName(&obj1, &obj2)) {
    writePSFmt("%ALDImageFileName: {0:t}\n", obj2.getString());
    obj2.free();
  }
  obj1.free();

  dict->lookup("CropRect", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 4) {
    obj1.arrayGet(0, &obj2);
    left = obj2.getInt();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    top = obj2.getInt();
    obj2.free();
    obj1.arrayGet(2, &obj2);
    right = obj2.getInt();
    obj2.free();
    obj1.arrayGet(3, &obj2);
    bottom = obj2.getInt();
    obj2.free();
    writePSFmt("%ALDImageCropRect: {0:d} {1:d} {2:d} {3:d}\n",
	       left, top, right, bottom);
  }
  obj1.free();

  dict->lookup("Color", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 5) {
    obj1.arrayGet(0, &obj2);
    c = obj2.getNum();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    m = obj2.getNum();
    obj2.free();
    obj1.arrayGet(2, &obj2);
    y = obj2.getNum();
    obj2.free();
    obj1.arrayGet(3, &obj2);
    k = obj2.getNum();
    obj2.free();
    obj1.arrayGet(4, &obj2);
    if (obj2.isString()) {
      writePSFmt("%ALDImageColor: {0:.4g} {1:.4g} {2:.4g} {3:.4g} ",
		 c, m, y, k);
      writePSString(obj2.getString());
      writePS("\n");
    }
    obj2.free();
  }
  obj1.free();

  dict->lookup("ColorType", &obj1);
  if (obj1.isName()) {
    writePSFmt("%ALDImageColorType: {0:s}\n", obj1.getName());
  }
  obj1.free();

  //~ ignores 'Comments' entry
  //~ need to handle multiple lines

  dict->lookup("CropFixed", &obj1);
  if (obj1.isArray()) {
    obj1.arrayGet(0, &obj2);
    ulx = obj2.getNum();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    uly = obj2.getNum();
    obj2.free();
    obj1.arrayGet(2, &obj2);
    lrx = obj2.getNum();
    obj2.free();
    obj1.arrayGet(3, &obj2);
    lry = obj2.getNum();
    obj2.free();
    writePSFmt("%ALDImageCropFixed: {0:.6g} {1:.6g} {2:.6g} {3:.6g}\n",
	       ulx, uly, lrx, lry);
  }
  obj1.free();

  dict->lookup("GrayMap", &obj1);
  if (obj1.isArray()) {
    writePS("%ALDImageGrayMap:");
    for (i = 0; i < obj1.arrayGetLength(); i += 16) {
      if (i > 0) {
	writePS("\n%%+");
      }
      for (j = 0; j < 16 && i+j < obj1.arrayGetLength(); ++j) {
	obj1.arrayGet(i+j, &obj2);
	writePSFmt(" {0:d}", obj2.getInt());
	obj2.free();
      }
    }
    writePS("\n");
  }
  obj1.free();

  dict->lookup("ID", &obj1);
  if (obj1.isString()) {
    writePSFmt("%ALDImageID: {0:t}\n", obj1.getString());
  }
  obj1.free();

  dict->lookup("ImageType", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 2) {
    obj1.arrayGet(0, &obj2);
    samples = obj2.getInt();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    bits = obj2.getInt();
    obj2.free();
    writePSFmt("%ALDImageType: {0:d} {1:d}\n", samples, bits);
  }
  obj1.free();

  dict->lookup("Overprint", &obj1);
  if (obj1.isBool()) {
    writePSFmt("%ALDImageOverprint: {0:s}\n",
	       obj1.getBool() ? "true" : "false");
  }
  obj1.free();

  dict->lookup("Position", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 8) {
    obj1.arrayGet(0, &obj2);
    llx = obj2.getNum();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    lly = obj2.getNum();
    obj2.free();
    obj1.arrayGet(2, &obj2);
    ulx = obj2.getNum();
    obj2.free();
    obj1.arrayGet(3, &obj2);
    uly = obj2.getNum();
    obj2.free();
    obj1.arrayGet(4, &obj2);
    urx = obj2.getNum();
    obj2.free();
    obj1.arrayGet(5, &obj2);
    ury = obj2.getNum();
    obj2.free();
    obj1.arrayGet(6, &obj2);
    lrx = obj2.getNum();
    obj2.free();
    obj1.arrayGet(7, &obj2);
    lry = obj2.getNum();
    obj2.free();
    opiTransform(state, llx, lly, &tllx, &tlly);
    opiTransform(state, ulx, uly, &tulx, &tuly);
    opiTransform(state, urx, ury, &turx, &tury);
    opiTransform(state, lrx, lry, &tlrx, &tlry);
    writePSFmt("%ALDImagePosition: {0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g} {6:.6g} {7:.6g}\n",
	       tllx, tlly, tulx, tuly, turx, tury, tlrx, tlry);
    obj2.free();
  }
  obj1.free();

  dict->lookup("Resolution", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 2) {
    obj1.arrayGet(0, &obj2);
    horiz = obj2.getNum();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    vert = obj2.getNum();
    obj2.free();
    writePSFmt("%ALDImageResoution: {0:.6g} {1:.6g}\n", horiz, vert);
    obj2.free();
  }
  obj1.free();

  dict->lookup("Size", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 2) {
    obj1.arrayGet(0, &obj2);
    width = obj2.getInt();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    height = obj2.getInt();
    obj2.free();
    writePSFmt("%ALDImageDimensions: {0:d} {1:d}\n", width, height);
  }
  obj1.free();

  //~ ignoring 'Tags' entry
  //~ need to use writePSString() and deal with >255-char lines

  dict->lookup("Tint", &obj1);
  if (obj1.isNum()) {
    writePSFmt("%ALDImageTint: {0:.6g}\n", obj1.getNum());
  }
  obj1.free();

  dict->lookup("Transparency", &obj1);
  if (obj1.isBool()) {
    writePSFmt("%ALDImageTransparency: {0:s}\n",
	       obj1.getBool() ? "true" : "false");
  }
  obj1.free();

  writePS("%%BeginObject: image\n");
  writePS("opiMatrix2 setmatrix\n");
  ++opi13Nest;
}

// Convert PDF user space coordinates to PostScript default user space
// coordinates.  This has to account for both the PDF CTM and the
// PSOutputDev page-fitting transform.
void PSOutputDev::opiTransform(GfxState *state, double x0, double y0,
			       double *x1, double *y1) {
  double t;

  state->transform(x0, y0, x1, y1);
  *x1 += tx;
  *y1 += ty;
  if (rotate == 90) {
    t = *x1;
    *x1 = -*y1;
    *y1 = t;
  } else if (rotate == 180) {
    *x1 = -*x1;
    *y1 = -*y1;
  } else if (rotate == 270) {
    t = *x1;
    *x1 = *y1;
    *y1 = -t;
  }
  *x1 *= xScale;
  *y1 *= yScale;
}

void PSOutputDev::opiEnd(GfxState *state, Dict *opiDict) {
  Object dict;

  if (globalParams->getPSOPI()) {
    opiDict->lookup("2.0", &dict);
    if (dict.isDict()) {
      writePS("%%EndIncludedImage\n");
      writePS("%%EndOPI\n");
      writePS("grestore\n");
      --opi20Nest;
      dict.free();
    } else {
      dict.free();
      opiDict->lookup("1.3", &dict);
      if (dict.isDict()) {
	writePS("%%EndObject\n");
	writePS("restore\n");
	--opi13Nest;
      }
      dict.free();
    }
  }
}
#endif // OPI_SUPPORT

void PSOutputDev::type3D0(GfxState *state, double wx, double wy) {
  writePSFmt("{0:.6g} {1:.6g} setcharwidth\n", wx, wy);
  writePS("q\n");
  t3NeedsRestore = gTrue;
}

void PSOutputDev::type3D1(GfxState *state, double wx, double wy,
			  double llx, double lly, double urx, double ury) {
  t3WX = wx;
  t3WY = wy;
  t3LLX = llx;
  t3LLY = lly;
  t3URX = urx;
  t3URY = ury;
  t3String = new GooString();
  writePS("q\n");
  t3Cacheable = gTrue;
  t3NeedsRestore = gTrue;
}

void PSOutputDev::drawForm(Ref id) {
  writePSFmt("f_{0:d}_{1:d}\n", id.num, id.gen);
}

void PSOutputDev::psXObject(Stream *psStream, Stream *level1Stream) {
  Stream *str;
  int c;

  if ((level == psLevel1 || level == psLevel1Sep) && level1Stream) {
    str = level1Stream;
  } else {
    str = psStream;
  }
  str->reset();
  while ((c = str->getChar()) != EOF) {
    writePSChar(c);
  }
  str->close();
}

//~ can nextFunc be reset to 0 -- maybe at the start of each page?
//~   or maybe at the start of each color space / pattern?
void PSOutputDev::cvtFunction(Function *func) {
  SampledFunction *func0;
  ExponentialFunction *func2;
  StitchingFunction *func3;
  PostScriptFunction *func4;
  int thisFunc, m, n, nSamples, i, j, k;

  switch (func->getType()) {

  case -1:			// identity
    writePS("{}\n");
    break;

  case 0:			// sampled
    func0 = (SampledFunction *)func;
    thisFunc = nextFunc++;
    m = func0->getInputSize();
    n = func0->getOutputSize();
    nSamples = n;
    for (i = 0; i < m; ++i) {
      nSamples *= func0->getSampleSize(i);
    }
    writePSFmt("/xpdfSamples{0:d} [\n", thisFunc);
    for (i = 0; i < nSamples; ++i) {
      writePSFmt("{0:.6g}\n", func0->getSamples()[i]);
    }
    writePS("] def\n");
    writePSFmt("{{ {0:d} array {1:d} array {2:d} 2 roll\n", 2*m, m, m+2);
    // [e01] [efrac] x0 x1 ... xm-1
    for (i = m-1; i >= 0; --i) {
      // [e01] [efrac] x0 x1 ... xi
      writePSFmt("{0:.6g} sub {1:.6g} mul {2:.6g} add\n",
	      func0->getDomainMin(i),
	      (func0->getEncodeMax(i) - func0->getEncodeMin(i)) /
	        (func0->getDomainMax(i) - func0->getDomainMin(i)),
	      func0->getEncodeMin(i));
      // [e01] [efrac] x0 x1 ... xi-1 xi'
      writePSFmt("dup 0 lt {{ pop 0 }} {{ dup {0:d} gt {{ pop {1:d} }} if }} ifelse\n",
		 func0->getSampleSize(i) - 1, func0->getSampleSize(i) - 1);
      // [e01] [efrac] x0 x1 ... xi-1 xi'
      writePS("dup floor cvi exch dup ceiling cvi exch 2 index sub\n");
      // [e01] [efrac] x0 x1 ... xi-1 floor(xi') ceiling(xi') xi'-floor(xi')
      writePSFmt("{0:d} index {1:d} 3 2 roll put\n", i+3, i);
      // [e01] [efrac] x0 x1 ... xi-1 floor(xi') ceiling(xi')
      writePSFmt("{0:d} index {1:d} 3 2 roll put\n", i+3, 2*i+1);
      // [e01] [efrac] x0 x1 ... xi-1 floor(xi')
      writePSFmt("{0:d} index {1:d} 3 2 roll put\n", i+2, 2*i);
      // [e01] [efrac] x0 x1 ... xi-1
    }
    // [e01] [efrac]
    for (i = 0; i < n; ++i) {
      // [e01] [efrac] y(0) ... y(i-1)
      for (j = 0; j < (1<<m); ++j) {
	// [e01] [efrac] y(0) ... y(i-1) s(0) s(1) ... s(j-1)
	writePSFmt("xpdfSamples{0:d}\n", thisFunc);
	k = m - 1;
	writePSFmt("{0:d} index {1:d} get\n", i+j+2, 2 * k + ((j >> k) & 1));
	for (k = m - 2; k >= 0; --k) {
	  writePSFmt("{0:d} mul {1:d} index {2:d} get add\n",
		     func0->getSampleSize(k),
		     i + j + 3,
		     2 * k + ((j >> k) & 1));
	}
	if (n > 1) {
	  writePSFmt("{0:d} mul {1:d} add ", n, i);
	}
	writePS("get\n");
      }
      // [e01] [efrac] y(0) ... y(i-1) s(0) s(1) ... s(2^m-1)
      for (j = 0; j < m; ++j) {
	// [e01] [efrac] y(0) ... y(i-1) s(0) s(1) ... s(2^(m-j)-1)
	for (k = 0; k < (1 << (m - j)); k += 2) {
	  // [e01] [efrac] y(0) ... y(i-1) <k/2 s' values> <2^(m-j)-k s values>
	  writePSFmt("{0:d} index {1:d} get dup\n",
		     i + k/2 + (1 << (m-j)) - k, j);
	  writePS("3 2 roll mul exch 1 exch sub 3 2 roll mul add\n");
	  writePSFmt("{0:d} 1 roll\n", k/2 + (1 << (m-j)) - k - 1);
	}
	// [e01] [efrac] s'(0) s'(1) ... s(2^(m-j-1)-1)
      }
      // [e01] [efrac] y(0) ... y(i-1) s
      writePSFmt("{0:.6g} mul {1:.6g} add\n",
		 func0->getDecodeMax(i) - func0->getDecodeMin(i),
		 func0->getDecodeMin(i));
      writePSFmt("dup {0:.6g} lt {{ pop {1:.6g} }} {{ dup {2:.6g} gt {{ pop {3:.6g} }} if }} ifelse\n",
		 func0->getRangeMin(i), func0->getRangeMin(i),
		 func0->getRangeMax(i), func0->getRangeMax(i));
      // [e01] [efrac] y(0) ... y(i-1) y(i)
    }
    // [e01] [efrac] y(0) ... y(n-1)
    writePSFmt("{0:d} {1:d} roll pop pop }}\n", n+2, n);
    break;

  case 2:			// exponential
    func2 = (ExponentialFunction *)func;
    n = func2->getOutputSize();
    writePSFmt("{{ dup {0:.6g} lt {{ pop {1:.6g} }} {{ dup {2:.6g} gt {{ pop {3:.6g} }} if }} ifelse\n",
	       func2->getDomainMin(0), func2->getDomainMin(0),
	       func2->getDomainMax(0), func2->getDomainMax(0));
    // x
    for (i = 0; i < n; ++i) {
      // x y(0) .. y(i-1)
      writePSFmt("{0:d} index {1:.6g} exp {2:.6g} mul {3:.6g} add\n",
		 i, func2->getE(), func2->getC1()[i] - func2->getC0()[i],
		 func2->getC0()[i]);
      if (func2->getHasRange()) {
	writePSFmt("dup {0:.6g} lt {{ pop {1:.6g} }} {{ dup {2:.6g} gt {{ pop {3:.6g} }} if }} ifelse\n",
		   func2->getRangeMin(i), func2->getRangeMin(i),
		   func2->getRangeMax(i), func2->getRangeMax(i));
      }
    }
    // x y(0) .. y(n-1)
    writePSFmt("{0:d} {1:d} roll pop }}\n", n+1, n);
    break;

  case 3:			// stitching
    func3 = (StitchingFunction *)func;
    thisFunc = nextFunc++;
    for (i = 0; i < func3->getNumFuncs(); ++i) {
      cvtFunction(func3->getFunc(i));
      writePSFmt("/xpdfFunc{0:d}_{1:d} exch def\n", thisFunc, i);
    }
    writePSFmt("{{ dup {0:.6g} lt {{ pop {1:.6g} }} {{ dup {2:.6g} gt {{ pop {3:.6g} }} if }} ifelse\n",
	       func3->getDomainMin(0), func3->getDomainMin(0),
	       func3->getDomainMax(0), func3->getDomainMax(0));
    for (i = 0; i < func3->getNumFuncs() - 1; ++i) {
      writePSFmt("dup {0:.6g} lt {{ {1:.6g} sub {2:.6g} mul {3:.6g} add xpdfFunc{4:d}_{5:d} }} {{\n",
		 func3->getBounds()[i+1],
		 func3->getBounds()[i],
		 func3->getScale()[i],
		 func3->getEncode()[2*i],
		 thisFunc, i);
    }
    writePSFmt("{0:.6g} sub {1:.6g} mul {2:.6g} add xpdfFunc{3:d}_{4:d}\n",
	       func3->getBounds()[i],
	       func3->getScale()[i],
	       func3->getEncode()[2*i],
	       thisFunc, i);
    for (i = 0; i < func3->getNumFuncs() - 1; ++i) {
      writePS("} ifelse\n");
    }
    writePS("}\n");
    break;

  case 4:			// PostScript
    func4 = (PostScriptFunction *)func;
    writePS(func4->getCodeString()->getCString());
    writePS("\n");
    break;
  }
}

void PSOutputDev::writePSChar(char c) {
  if (t3String) {
    t3String->append(c);
  } else {
    (*outputFunc)(outputStream, &c, 1);
  }
}

void PSOutputDev::writePS(char *s) {
  if (t3String) {
    t3String->append(s);
  } else {
    (*outputFunc)(outputStream, s, strlen(s));
  }
}

void PSOutputDev::writePSBuf(char *s, int len) {
  if (t3String) {
    for (int i = 0; i < len; i++) {
      t3String->append(s[i]);
    }
  } else {
    (*outputFunc)(outputStream, s, len);
  }
}

void PSOutputDev::writePSFmt(const char *fmt, ...) {
  va_list args;
  GooString *buf;

  va_start(args, fmt);
  if (t3String) {
    t3String->appendfv((char *)fmt, args);
  } else {
    buf = GooString::formatv((char *)fmt, args);
    (*outputFunc)(outputStream, buf->getCString(), buf->getLength());
    delete buf;
  }
  va_end(args);
}

void PSOutputDev::writePSString(GooString *s) {
  Guchar *p;
  int n, line;
  char buf[8];

  writePSChar('(');
  line = 1;
  for (p = (Guchar *)s->getCString(), n = s->getLength(); n; ++p, --n) {
    if (line >= 64) {
      writePSChar('\\');
      writePSChar('\n');
      line = 0;
    }
    if (*p == '(' || *p == ')' || *p == '\\') {
      writePSChar('\\');
      writePSChar((char)*p);
      line += 2;
    } else if (*p < 0x20 || *p >= 0x80) {
      sprintf(buf, "\\%03o", *p);
      writePS(buf);
      line += 4;
    } else {
      writePSChar((char)*p);
      ++line;
    }
  }
  writePSChar(')');
}

void PSOutputDev::writePSName(char *s) {
  char *p;
  char c;

  p = s;
  while ((c = *p++)) {
    if (c <= (char)0x20 || c >= (char)0x7f ||
	c == '(' || c == ')' || c == '<' || c == '>' ||
	c == '[' || c == ']' || c == '{' || c == '}' ||
	c == '/' || c == '%' || c == '\\') {
      writePSFmt("#{0:02x}", c & 0xff);
    } else {
      writePSChar(c);
    }
  }
}

// Convert GooString to GooString, with appropriate escaping
// of things that can't appear in a label
// This is heavily based on the writePSTextLine() method
GooString* PSOutputDev::filterPSLabel(GooString *label, GBool *needParens) {
  int i, step;
  GBool isNumeric;

  // - DSC comments must be printable ASCII; control chars and
  //   backslashes have to be escaped (we do cheap UCS2-to-ASCII
  //   conversion by simply ignoring the high byte)
  // - parentheses are escaped. this isn't strictly necessary for matched
  //   parentheses, but shouldn't be a problem
  // - lines are limited to 255 chars (we limit to 200 here to allow
  //   for the keyword, which was emitted by the caller)

  GooString *label2 = new GooString();
  int labelLength = label->getLength();

  if (labelLength == 0) {
    isNumeric = false;
  } else {
    // this gets changed later if we find a non-numeric character
    isNumeric = true;
  }

  if ( (labelLength >= 2) &&
       ( (label->getChar(0) & 0xff) == 0xfe) &&
       ( (label->getChar(1) & 0xff) == 0xff) ) {
    // UCS2 mode
    i = 3;
    step = 2;
    if ( (label->getChar(labelLength-1) == 0) ) {
      // prune the trailing null (0x000 for UCS2)
      labelLength -= 2;
    }
  } else {
    i = 0;
    step = 1;
  }
  for (int j = 0; i < labelLength && j < 200; i += step) {
    char c = label->getChar(i);
    if ( (c < '0') || (c > '9') ) {
      isNumeric = false;
    }
    if (c == '\\') {
      label2->append("\\\\");
      j += 2;
    } else if (c == ')') {
      label2->append("\\)");
    } else if (c == '(') {
      label2->append("\\(");
    } else if (c < 0x20 || c > 0x7e) {
      label2->append(GooString::format("\\{0:03o}", c));
      j += 4;
    } else {
      label2->append(c);
      ++j;
    }
  }
  if (needParens) {
    *needParens = !(isNumeric);
  }
  return label2;
}

// Write a DSC-compliant <textline>.
void PSOutputDev::writePSTextLine(GooString *s) {
  int i, j, step;
  int c;

  // - DSC comments must be printable ASCII; control chars and
  //   backslashes have to be escaped (we do cheap Unicode-to-ASCII
  //   conversion by simply ignoring the high byte)
  // - lines are limited to 255 chars (we limit to 200 here to allow
  //   for the keyword, which was emitted by the caller)
  // - lines that start with a left paren are treated as <text>
  //   instead of <textline>, so we escape a leading paren
  if (s->getLength() >= 2 &&
      (s->getChar(0) & 0xff) == 0xfe &&
      (s->getChar(1) & 0xff) == 0xff) {
    i = 3;
    step = 2;
  } else {
    i = 0;
    step = 1;
  }
  for (j = 0; i < s->getLength() && j < 200; i += step) {
    c = s->getChar(i) & 0xff;
    if (c == '\\') {
      writePS("\\\\");
      j += 2;
    } else if (c < 0x20 || c > 0x7e || (j == 0 && c == '(')) {
      writePSFmt("\\{0:03o}", c);
      j += 4;
    } else {
      writePSChar(c);
      ++j;
    }
  }
  writePS("\n");
}
