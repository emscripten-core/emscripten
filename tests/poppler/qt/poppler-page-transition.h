/* PageTransition.h
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, Brad Hards <bradh@frogmouth.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __PAGETRANSITION_X_H__
#define __PAGETRANSITION_X_H__

namespace Poppler {

class PageTransitionParams;
class PageTransitionData;

/**
   \brief Describes how a PDF file viewer shall perform the transition
   from one page to another

   In PDF files there is a way to specify if the viewer shall use
   certain effects to perform the transition from one page to
   another. This feature can be used, e.g., in a PDF-based beamer
   presentation.

   This utility class represents the transition effect, and can be
   used to extract the information from a PDF object.
*/


class PageTransition {
 public:

  /** \brief transition effect that shall be used
   */
  // if changed remember to keep in sync with PageTransition.h enum
  enum Type {
    Replace = 0,
    Split,
    Blinds,
    Box,
    Wipe,
    Dissolve,
    Glitter,
    Fly,
    Push,
    Cover,
    Uncover,
    Fade
  };
  
  /** \brief alignment of the transition effect that shall be used
   */
  // if changed remember to keep in sync with PageTransition.h enum
  enum Alignment {
    Horizontal = 0,
    Vertical
  };
  
  /** \brief direction of the transition effect that shall be used
   */
  // if changed remember to keep in sync with PageTransition.h enum
  enum Direction {
    Inward = 0,
    Outward
  };
  
  /** \brief Construct a new PageTransition object from a page dictionary.

  Users of the library will rarely need to construct a
  PageTransition object themselves. Instead, the method
  Poppler::Page::transition() can be used to find out if a certain
  transition effect is specified.

  @warning In case or error, this method will print an error message to stderr,
  and construct a default object.

  @param params an object whose dictionary will be read and
   parsed. This must be a valid object, whose dictionaries are
   accessed by the constructor. The object is only accessed by this
   constructor, and may be deleted after the constructor returns.
  */
  PageTransition(const PageTransitionParams &params);

  /** \brief copy constructor */
  PageTransition(const PageTransition &pt);
  
  /**
     Destructor
  */
  ~PageTransition();
  
  /**
     \brief Get type of the transition.
  */
  Type type() const;
  
  /**
     \brief Get duration of the transition in seconds.
  */
  int duration() const;
  
  /**
     \brief Get dimension in which the transition effect occurs.
  */
  Alignment alignment() const;
  
  /**
     \brief Get direction of motion of the transition effect.
  */
  Direction direction() const;
  
  /**
     \brief Get direction in which the transition effect moves.
  */
  int angle() const;
  
  /**
     \brief Get starting or ending scale.
  */
  double scale() const;
  
  /**
     \brief Returns true if the area to be flown is rectangular and
     opaque.
  */
  bool isRectangular() const;
  
 private:
  PageTransitionData *data;
};

}

#endif
