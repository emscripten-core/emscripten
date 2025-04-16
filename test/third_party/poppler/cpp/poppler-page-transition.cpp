/*
 * Copyright (C) 2009-2010, Pino Toscano <pino@kde.org>
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

#include "poppler-page-transition.h"

#include "PageTransition.h"

using namespace poppler;

class poppler::page_transition_private
{
public:
    page_transition_private(Object *trans)
        : pt(trans)
    {
    }

    PageTransition pt;
};

/**
 \class poppler::page_transition poppler-page-transition.h "poppler/cpp/poppler-page-transition.h"

 A transition between two pages in a PDF %document.

 Usually shown in a presentation mode of a PDF viewer.
 */

/**
 \enum poppler::page_transition::type_enum

 The possibe types of a %page transition.
*/

/**
 \enum poppler::page_transition::alignment_enum

 The alignment of a %page transition.
*/

/**
 \enum poppler::page_transition::direction_enum

 The direction of an animation in a %page transition.
*/


page_transition::page_transition(Object *params)
    : d(new page_transition_private(params))
{
}

/**
 Copy constructor.
 */
page_transition::page_transition(const page_transition &pt)
    : d(new page_transition_private(*pt.d))
{
}

/**
 Destructor.
 */
page_transition::~page_transition()
{
    delete d;
}

page_transition::type_enum page_transition::type() const
{
    return (page_transition::type_enum)d->pt.getType();
}

int page_transition::duration() const
{
    return d->pt.getDuration();
}

page_transition::alignment_enum page_transition::alignment() const
{
    return (page_transition::alignment_enum)d->pt.getAlignment();
}

page_transition::direction_enum page_transition::direction() const
{
    return (page_transition::direction_enum)d->pt.getDirection();
}

int page_transition::angle() const
{
    return d->pt.getAngle();
}

double page_transition::scale() const
{
    return d->pt.getScale();
}

bool page_transition::is_rectangular() const
{
    return d->pt.isRectangular();
}

page_transition& page_transition::operator=(const page_transition &pt)
{
    if (&pt != this) {
        page_transition_private *new_d = new page_transition_private(*pt.d);
        delete d;
        new_d = d;
    }
    return *this;
}
