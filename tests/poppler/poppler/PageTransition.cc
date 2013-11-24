/* PageTransition.cc
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2010, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
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

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "PageTransition.h"

//------------------------------------------------------------------------
// PageTransition
//------------------------------------------------------------------------

PageTransition::PageTransition (Object *trans) {
  Object obj;
  Dict *dict;

  type = transitionReplace;
  duration = 1;
  alignment = transitionHorizontal;
  direction = transitionInward;
  angle = 0;
  scale = 1.0;
  rectangular = gFalse;
  ok = gTrue;

  if (!trans || !trans->isDict ()) {
    ok = gFalse;
    return;
  }

  dict = trans->getDict();

  // get type
  if (dict->lookup("S", &obj)->isName()) {
    const char *s = obj.getName();
    
    if (strcmp("R", s) == 0)
      type = transitionReplace;
    else if (strcmp("Split", s) == 0)
      type = transitionSplit;
    else if (strcmp("Blinds", s) == 0)
      type = transitionBlinds;
    else if (strcmp("Box", s) == 0)
      type = transitionBox;
    else if (strcmp("Wipe", s) == 0)
      type = transitionWipe;
    else if (strcmp("Dissolve", s) == 0)
      type = transitionDissolve;
    else if (strcmp("Glitter", s) == 0)
      type = transitionGlitter;
    else if (strcmp("Fly", s) == 0)
      type = transitionFly;
    else if (strcmp("Push", s) == 0)
      type = transitionPush;
    else if (strcmp("Cover", s) == 0)
      type = transitionCover;
    else if (strcmp("Uncover", s) == 0)
      type = transitionUncover;
    else if (strcmp("Fade", s) == 0)
      type = transitionFade;
  }
  obj.free();

  // get duration
  if (dict->lookup("D", &obj)->isInt()) {
    duration = obj.getInt();
  }
  obj.free();

  // get alignment
  if (dict->lookup("Dm", &obj)->isName()) {
    const char *dm = obj.getName();
    
    if (strcmp("H", dm) == 0)
      alignment = transitionHorizontal;
    else if (strcmp("V", dm) == 0)
      alignment = transitionVertical;
  }
  obj.free();

  // get direction
  if (dict->lookup("M", &obj)->isName()) {
    const char *m = obj.getName();
    
    if (strcmp("I", m) == 0)
      direction = transitionInward;
    else if (strcmp("O", m) == 0)
      direction = transitionOutward;
  }
  obj.free();

  // get angle
  if (dict->lookup("Di", &obj)->isInt()) {
    angle = obj.getInt();
  }
  obj.free();

  if (dict->lookup("Di", &obj)->isName()) {
    if (strcmp("None", obj.getName()) == 0)
      angle = 0;
  }
  obj.free();

  // get scale
  if (dict->lookup("SS", &obj)->isNum()) {
    scale = obj.getNum();
  }
  obj.free();

  // get rectangular
  if (dict->lookup("B", &obj)->isBool()) {
    rectangular = obj.getBool();
  }
  obj.free();
}

PageTransition::~PageTransition()
{
}

