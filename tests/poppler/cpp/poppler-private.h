/*
 * Copyright (C) 2009, Pino Toscano <pino@kde.org>
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

#ifndef POPPLER_PRIVATE_H
#define POPPLER_PRIVATE_H

#include "poppler-global.h"
#include "poppler-rectangle.h"

#include "CharTypes.h"

#include <stdarg.h>

class GooString;
class PDFRectangle;

#define PSTR(str) const_cast<char *>(str)

namespace poppler
{

namespace detail
{

void error_function(int pos, char *msg, va_list args);

rectf pdfrectangle_to_rectf(const PDFRectangle &pdfrect);

ustring unicode_GooString_to_ustring(GooString *str);
ustring unicode_to_ustring(const Unicode *u, int length);
GooString* ustring_to_unicode_GooString(const ustring &str);

time_type convert_date(const char *date);

}

template <typename ConstIterator>
void delete_all(ConstIterator it, ConstIterator end)
{
    while (it != end) {
        delete *it++;
    }
}

template <typename Collection>
void delete_all(const Collection &c)
{
    delete_all(c.begin(), c.end());
}

}

#endif
