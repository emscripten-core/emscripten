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

#ifndef POPPLER_EMBEDDED_FILE_H
#define POPPLER_EMBEDDED_FILE_H

#include "poppler-global.h"

#include <vector>

namespace poppler
{

class embedded_file_private;

class POPPLER_CPP_EXPORT embedded_file : public poppler::noncopyable
{
public:
    ~embedded_file();

    bool is_valid() const;
    std::string name() const;
    ustring description() const;
    int size() const;
    time_type modification_date() const;
    time_type creation_date() const;
    byte_array checksum() const;
    std::string mime_type() const;
    byte_array data() const;

private:
    embedded_file(embedded_file_private &dd);

    embedded_file_private *d;
    friend class embedded_file_private;
};

}

#endif
