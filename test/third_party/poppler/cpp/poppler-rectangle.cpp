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

#include "poppler-rectangle.h"

#include <iostream>

using namespace poppler;

/**
 \class poppler::rectangle poppler-rectangle.h "poppler/cpp/poppler-rectangle.h"

 A rectangle.
 */

/**
 \typedef poppler::rect

 A rectangle with int dimensions and coordinates.
 */

/**
 \typedef poppler::rectf

 A rectangle with float (double) dimensions and coordinates.
 */


std::ostream& poppler::operator<<(std::ostream& stream, const rect &r)
{
    stream << "[" << r.x() << "," << r.y() << " " << r.width() << "+" << r.height() << "]";
    return stream;
}

std::ostream& poppler::operator<<(std::ostream& stream, const rectf &r)
{
    stream << "[" << r.x() << "," << r.y() << " " << r.width() << "+" << r.height() << "]";
    return stream;
}
