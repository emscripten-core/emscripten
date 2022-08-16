//*********************************************************************************
//                               Movie.cc
//---------------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------------
// Hugo Mercier <hmercier31[at]gmail.com> (c) 2008
// Pino Toscano <pino@kde.org> (c) 2008
// Carlos Garcia Campos <carlosgc@gnome.org> (c) 2010
// Albert Astals Cid <aacid@kde.org> (c) 2010
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

#include <math.h>
#include "Movie.h"
#include "FileSpec.h"

MovieActivationParameters::MovieActivationParameters() {
  // default values
  floatingWindow = gFalse;
  xPosition = 0.5;
  yPosition = 0.5;
  rate = 1.0;
  volume = 100;
  showControls = gFalse;
  synchronousPlay = gFalse;
  repeatMode = repeatModeOnce;
  start.units = 0;
  duration.units = 0;
  znum = 1;
  zdenum = 1;
}

MovieActivationParameters::~MovieActivationParameters() {
}

void MovieActivationParameters::parseMovieActivation(Object* aDict) {
  Object obj1;

  if (!aDict->dictLookup("Start", &obj1)->isNull()) {
    if (obj1.isInt()) {
      // If it is representable as an integer (subject to the implementation limit for
      // integers, as described in Appendix C), it should be specified as such.

      start.units = obj1.getInt();
    } else if (obj1.isString()) {
      // If it is not representable as an integer, it should be specified as an 8-byte
      // string representing a 64-bit twos-complement integer, most significant
      // byte first.

      // UNSUPPORTED
    } else if (obj1.isArray()) {
      Array* a = obj1.getArray();

      Object tmp;
      a->get(0, &tmp);
      if (tmp.isInt()) {
        start.units = tmp.getInt();
      }
      if (tmp.isString()) {
        // UNSUPPORTED
      }
      tmp.free();

      a->get(1, &tmp);
      if (tmp.isInt()) {
        start.units_per_second = tmp.getInt();
      }
      tmp.free();
    }
  }
  obj1.free();

  if (!aDict->dictLookup("Duration", &obj1)->isNull()) {
    if (obj1.isInt()) {
      duration.units = obj1.getInt();
    } else if (obj1.isString()) {
      // UNSUPPORTED
    } else if (obj1.isArray()) {
      Array* a = obj1.getArray();

      Object tmp;
      a->get(0, &tmp);
      if (tmp.isInt()) {
        duration.units = tmp.getInt();
      }
      if (tmp.isString()) {
        // UNSUPPORTED
      }
      tmp.free();

      a->get(1, &tmp);
      if (tmp.isInt()) {
        duration.units_per_second = tmp.getInt();
      }
      tmp.free();
    }
  }
  obj1.free();

  if (aDict->dictLookup("Rate", &obj1)->isNum()) {
    rate = obj1.getNum();
  }
  obj1.free();

  if (aDict->dictLookup("Volume", &obj1)->isNum()) {
    // convert volume to [0 100]
    volume = int((obj1.getNum() + 1.0) * 50);
  }
  obj1.free();

  if (aDict->dictLookup("ShowControls", &obj1)->isBool()) {
    showControls = obj1.getBool();
  }
  obj1.free();

  if (aDict->dictLookup("Synchronous", &obj1)->isBool()) {
    synchronousPlay = obj1.getBool();
  }
  obj1.free();

  if (aDict->dictLookup("Mode", &obj1)->isName()) {
    char* name = obj1.getName();
    if (!strcmp(name, "Once")) {
      repeatMode = repeatModeOnce;
    } else if (!strcmp(name, "Open")) {
      repeatMode = repeatModeOpen;
    } else if (!strcmp(name, "Repeat")) {
      repeatMode = repeatModeRepeat;
    } else if (!strcmp(name,"Palindrome")) {
      repeatMode = repeatModePalindrome;
    }
  }
  obj1.free();

  if (aDict->dictLookup("FWScale", &obj1)->isArray()) {
    // the presence of that entry implies that the movie is to be played
    // in a floating window
    floatingWindow = gTrue;

    Array* scale = obj1.getArray();
    if (scale->getLength() >= 2) {
      Object tmp;
      if (scale->get(0, &tmp)->isInt()) {
        znum = tmp.getInt();
      }
      tmp.free();
      if (scale->get(1, &tmp)->isInt()) {
        zdenum = tmp.getInt();
      }
      tmp.free();
    }
  }
  obj1.free();

  if (aDict->dictLookup("FWPosition", &obj1)->isArray()) {
    Array* pos = obj1.getArray();
    if (pos->getLength() >= 2) {
      Object tmp;
      if (pos->get(0, &tmp)->isNum()) {
        xPosition = tmp.getNum();
      }
      tmp.free();
      if (pos->get(1, &tmp)->isNum()) {
        yPosition = tmp.getNum();
      }
      tmp.free();
    }
  }
  obj1.free();
}

void Movie::parseMovie (Object *movieDict) {
  fileName = NULL;
  rotationAngle = 0;
  width = -1;
  height = -1;
  showPoster = gFalse;

  Object obj1, obj2;
  if (getFileSpecNameForPlatform(movieDict->dictLookup("F", &obj1), &obj2)) {
    fileName = obj2.getString()->copy();
    obj2.free();
  } else {
    error (-1, "Invalid Movie");
    ok = gFalse;
    obj1.free();
    return;
  }
  obj1.free();

  if (movieDict->dictLookup("Aspect", &obj1)->isArray()) {
    Array* aspect = obj1.getArray();
    if (aspect->getLength() >= 2) {
      Object tmp;
      if( aspect->get(0, &tmp)->isNum() ) {
        width = (int)floor( aspect->get(0, &tmp)->getNum() + 0.5 );
      }
      tmp.free();
      if( aspect->get(1, &tmp)->isNum() ) {
        height = (int)floor( aspect->get(1, &tmp)->getNum() + 0.5 );
      }
      tmp.free();
    }
  }
  obj1.free();

  if (movieDict->dictLookup("Rotate", &obj1)->isInt()) {
    // round up to 90°
    rotationAngle = (((obj1.getInt() + 360) % 360) % 90) * 90;
  }
  obj1.free();

  //
  // movie poster
  //
  if (!movieDict->dictLookupNF("Poster", &poster)->isNull()) {
    if (poster.isRef() || poster.isStream()) {
      showPoster = gTrue;
    } else if (poster.isBool()) {
      showPoster = poster.getBool();
      poster.free();
    } else {
      poster.free();
    }
  }
}

Movie::~Movie() {
  if (fileName)
    delete fileName;
  poster.free();
}

Movie::Movie(Object *movieDict) {
  ok = gTrue;

  if (movieDict->isDict())
    parseMovie(movieDict);
  else
    ok = gFalse;
}

Movie::Movie(Object *movieDict, Object *aDict) {
  ok = gTrue;

  if (movieDict->isDict()) {
    parseMovie(movieDict);
    if (aDict->isDict())
      MA.parseMovieActivation(aDict);
  } else {
    ok = gFalse;
  }
}

void Movie::getFloatingWindowSize(int *widthA, int *heightA)
{
  *widthA = int(width * double(MA.znum) / MA.zdenum);
  *heightA = int(height * double(MA.znum) / MA.zdenum);
}

Movie* Movie::copy() {

  // call default copy constructor
  Movie* new_movie = new Movie(*this);

  if (fileName)
    new_movie->fileName = fileName->copy();

  poster.copy(&new_movie->poster);

  return new_movie;
}
