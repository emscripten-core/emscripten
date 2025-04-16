//*********************************************************************************
//                               Rendition.h
//---------------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------------
// Hugo Mercier <hmercier31[at]gmail.com> (c) 2008
// Carlos Garcia Campos <carlosgc@gnome.org> (c) 2010
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//*********************************************************************************

#ifndef _RENDITION_H_
#define _RENDITION_H_

#include "Object.h"

struct MediaWindowParameters {

  MediaWindowParameters();
  ~MediaWindowParameters();

  // parse from a floating window parameters dictionary
  void parseFWParams(Object* obj);

  enum MediaWindowType {
    windowFloating = 0,
    windowFullscreen,
    windowHidden,
    windowEmbedded
  };

  enum MediaWindowRelativeTo {
    windowRelativeToDocument = 0,
    windowRelativeToApplication,
    windowRelativeToDesktop
  };


                                         // DEFAULT VALUE

  MediaWindowType type;                  // movieWindowEmbedded
  

  int width;                             // -1
  int height;                            // -1
  
  // floating window position
  MediaWindowRelativeTo relativeTo;      // windowRelativeToDocument (or to desktop)
  double XPosition;                      // 0.5
  double YPosition;                      // 0.5

  GBool hasTitleBar;                      // true
  GBool hasCloseButton;                   // true
  GBool isResizeable;                     // true
};


struct MediaParameters {

  MediaParameters();
  ~MediaParameters();

  // parse from a "Media Play Parameters" dictionary
  void parseMediaPlayParameters(Object* playObj);
  // parse from a "Media Screen Parameters" dictionary
  void parseMediaScreenParameters(Object* screenObj);

  enum MediaFittingPolicy {
    fittingMeet = 0,
    fittingSlice,
    fittingFill,
    fittingScroll,
    fittingHidden,
    fittingUndefined
  };

  struct Color {
    double r, g, b;
  };

  int duration;                      // 0

  int volume;                              // 100

  // defined in media play parameters, p 770
  // correspond to 'fit' SMIL's attribute
  MediaFittingPolicy fittingPolicy;        // fittingUndefined

  GBool autoPlay;                          // true

  // repeat count, can be real values, 0 means forever
  double repeatCount;                      // 1.0

  // background color                      // black = (0.0 0.0 0.0)
  Color bgColor;
  
  // opacity in [0.0 1.0]
  double opacity;                          // 1.0
  

  GBool showControls;                      // false

  MediaWindowParameters windowParams;
};

class MediaRendition {
 public:
  MediaRendition(Object *obj);
  ~MediaRendition();

  GBool isOk () { return ok; }

  MediaParameters* getMHParameters() { return &MH; }
  MediaParameters* getBEParameters() { return &BE; }

  GooString* getContentType() { return contentType; }
  GooString* getFileName() { return fileName; }

  GBool getIsEmbedded() { return isEmbedded; }
  Stream* getEmbbededStream() { return embeddedStream; }
  // write embedded stream to file
  void outputToFile(FILE*);

  MediaRendition* copy();

 private:
  GBool ok;

  // "Must Honor" parameters
  MediaParameters MH;
  // "Best Effort" parameters
  MediaParameters BE;

  GBool isEmbedded;

  GooString* contentType;

  // if it's embedded
  Stream* embeddedStream;

  // if it's not embedded
  GooString* fileName;
};

#endif /* _RENDITION_H_ */
