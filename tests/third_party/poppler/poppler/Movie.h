//*********************************************************************************
//                               Movie.h
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

#ifndef _MOVIE_H_
#define _MOVIE_H_

#include "Object.h"

struct MovieActivationParameters {

  MovieActivationParameters();
  ~MovieActivationParameters();

  // parse from a "Movie Activation" dictionary
  void parseMovieActivation(Object* actObj);

  enum MovieRepeatMode {
    repeatModeOnce,
    repeatModeOpen,
    repeatModeRepeat,
    repeatModePalindrome
  };

  struct MovieTime {
    MovieTime() { units_per_second = 0; }
    Gulong units;
    int units_per_second; // 0 : defined by movie
  };

  MovieTime start;                         // 0
  MovieTime duration;                      // 0

  double rate;                             // 1.0

  int volume;                              // 100

  GBool showControls;                      // false

  GBool synchronousPlay;                   // false
  MovieRepeatMode repeatMode;              // repeatModeOnce

  // floating window position
  GBool floatingWindow;
  double xPosition;                        // 0.5
  double yPosition;                        // 0.5
  int znum;                                // 1
  int zdenum;                              // 1
};

class Movie {
 public:
  Movie(Object *objMovie, Object *objAct);
  Movie(Object *objMovie);
  ~Movie();

  GBool isOk() { return ok; }
  MovieActivationParameters* getActivationParameters() { return &MA; }

  GooString* getFileName() { return fileName; }

  Gushort getRotationAngle() { return rotationAngle; }
  void getAspect (int *widthA, int *heightA) { *widthA = width; *heightA = height; }

  Object *getPoster(Object *obj) { return poster.copy(obj); }
  GBool getShowPoster() { return showPoster; }

  GBool getUseFloatingWindow() { return MA.floatingWindow; }
  void  getFloatingWindowSize(int *width, int *height);

  Movie* copy();

 private:
  void parseMovie (Object *movieDict);

  GBool ok;

  Gushort rotationAngle;                   // 0
  int width;                               // Aspect
  int height;                              // Aspect

  Object poster;
  GBool showPoster;

  GooString* fileName;

  MovieActivationParameters MA;
};

#endif

