/* PageTransition.cc
 * Copyright (C) 2005, Net Integration Technologies, Inc.
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

#include "PageTransition.h"
#include "poppler-page-transition.h"
#include "poppler-page-transition-private.h"

namespace Poppler {

class PageTransitionData
{
  public:
    PageTransitionData(Object *trans)
    {
        pt = new ::PageTransition(trans);
    }

    PageTransitionData(const PageTransitionData &ptd)
    {
        pt = new ::PageTransition(*ptd.pt);
    }

    ~PageTransitionData()
    {
        delete pt;
    }

    ::PageTransition *pt;
};

PageTransition::PageTransition(const PageTransitionParams &params)
{
  data = new PageTransitionData(params.dictObj);
}

PageTransition::PageTransition(const PageTransition &pt)
{
  data = new PageTransitionData(*pt.data);
}

PageTransition::~PageTransition()
{
  delete data;
}

PageTransition::Type PageTransition::type() const
{
  return (Poppler::PageTransition::Type)data->pt->getType();
}

int PageTransition::duration() const
{
  return data->pt->getDuration();
}

PageTransition::Alignment PageTransition::alignment() const
{
  return (Poppler::PageTransition::Alignment)data->pt->getAlignment();
}

PageTransition::Direction PageTransition::direction() const
{
  return (Poppler::PageTransition::Direction)data->pt->getDirection();
}

int PageTransition::angle() const
{
  return data->pt->getAngle();
}

double PageTransition::scale() const
{
  return data->pt->getScale();
}
bool PageTransition::isRectangular() const
{
  return data->pt->isRectangular();
}

}
