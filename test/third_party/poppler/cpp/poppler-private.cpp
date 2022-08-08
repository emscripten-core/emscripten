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

#include "poppler-private.h"

#include "DateInfo.h"
#include "GooString.h"
#include "Page.h"

#include <ctime>
#include <iostream>
#include <sstream>

using namespace poppler;

void detail::error_function(int pos, char *msg, va_list args)
{
    std::ostringstream oss;
    if (pos >= 0) {
        oss << "poppler/error (" << pos << "): ";
    } else {
        oss << "poppler/error: ";
    }
    char buffer[4096]; // should be big enough
    vsnprintf(buffer, sizeof(buffer) - 1, msg, args);
    oss << buffer;
    std::cerr << oss.str();
}

rectf detail::pdfrectangle_to_rectf(const PDFRectangle &pdfrect)
{
    return rectf(pdfrect.x1, pdfrect.y1, pdfrect.x2 - pdfrect.x1, pdfrect.y2 - pdfrect.y1);
}

ustring detail::unicode_GooString_to_ustring(GooString *str)
{
    const char *data = str->getCString();
    const int len = str->getLength();

    int i = 0;
    bool is_unicode = false;
    if ((data[0] & 0xff) == 0xfe && (len > 1 && (data[1] & 0xff) == 0xff)) {
        is_unicode = true;
        i = 2;
    }
    ustring::size_type ret_len = len - i;
    if (is_unicode) {
        ret_len >>= 1;
    }
    ustring ret(ret_len, 0);
    size_t ret_index = 0;
    ustring::value_type u;
    if (is_unicode) {
        while (i < len) {
            u = ((data[i] & 0xff) << 8) | (data[i + 1] & 0xff);
            i += 2;
            ret[ret_index++] = u;
        }
    } else {
        while (i < len) {
            u = data[i] & 0xff;
            ++i;
            ret[ret_index++] = u;
        }
    }

    return ret;
}

ustring detail::unicode_to_ustring(const Unicode *u, int length)
{
    ustring str(length * 2, 0);
    ustring::iterator it = str.begin();
    const Unicode *uu = u;
    for (int i = 0; i < length; ++i) {
        *it++ = ustring::value_type(*uu++ & 0xffff);
    }
    return str;
}

GooString* detail::ustring_to_unicode_GooString(const ustring &str)
{
    const size_t len = str.size() * 2 + 2;
    const ustring::value_type *me = str.data();
    byte_array ba(len);
    ba[0] = 0xfe;
    ba[1] = 0xff;
    for (size_t i = 0; i < str.size(); ++i, ++me) {
        ba[i * 2 + 2] = ((*me >> 8) & 0xff);
        ba[i * 2 + 3] = (*me & 0xff);
    }
    GooString *goo = new GooString(&ba[0]);
    return goo;
}

time_type detail::convert_date(const char *date)
{
    int year, mon, day, hour, min, sec, tzHours, tzMins;
    char tz;

    if (!parseDateString(date, &year, &mon, &day, &hour, &min, &sec,
                               &tz, &tzHours, &tzMins)) {
        return time_type(-1);
    }

    struct tm time;
    time.tm_sec = sec;
    time.tm_min = min;
    time.tm_hour = hour;
    time.tm_mday = day;
    time.tm_mon = mon - 1;
    time.tm_year = year - 1900;
    time.tm_wday = -1;
    time.tm_yday = -1;
    time.tm_isdst = -1;
    return mktime(&time);
}
