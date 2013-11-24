/*
 * Copyright (C) 2009-2011, Pino Toscano <pino@kde.org>
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

#include "poppler-embedded-file.h"

#include "poppler-embedded-file-private.h"
#include "poppler-private.h"

#include "Object.h"
#include "Stream.h"
#include "Catalog.h"
#include "FileSpec.h"

using namespace poppler;

embedded_file_private::embedded_file_private(FileSpec *fs)
    : file_spec(fs)
{
}

embedded_file_private::~embedded_file_private()
{
    delete file_spec;
}

embedded_file* embedded_file_private::create(FileSpec *fs)
{
    return new embedded_file(*new embedded_file_private(fs));
}

/**
 \class poppler::embedded_file poppler-embedded-file.h "poppler/cpp/poppler-embedded-file.h"

 Represents a file embedded in a PDF %document.
 */


embedded_file::embedded_file(embedded_file_private &dd)
    : d(&dd)
{
}

/**
 Destroys the embedded file.
 */
embedded_file::~embedded_file()
{
    delete d;
}

/**
 \returns whether the embedded file is valid
 */
bool embedded_file::is_valid() const
{
    return d->file_spec->isOk();
}

/**
 \returns the name of the embedded file
 */
std::string embedded_file::name() const
{
    GooString *goo = d->file_spec->getFileName();
    return goo ? std::string(goo->getCString()) : std::string();
}

/**
 \returns the description of the embedded file
 */
ustring embedded_file::description() const
{
    GooString *goo = d->file_spec->getDescription();
    return goo ? detail::unicode_GooString_to_ustring(goo) : ustring();
}

/**
 \note this is not always available in the PDF %document, in that case this
       will return \p -1.

 \returns the size of the embedded file, if known
 */
int embedded_file::size() const
{
    return d->file_spec->getEmbeddedFile()->size();
}

/**
 \returns the time_t representing the modification date of the embedded file,
          if available
 */
time_type embedded_file::modification_date() const
{
    GooString *goo = d->file_spec->getEmbeddedFile()->modDate();
    return goo ? detail::convert_date(goo->getCString()) : time_type(-1);
}

/**
 \returns the time_t representing the creation date of the embedded file,
          if available
 */
time_type embedded_file::creation_date() const
{
    GooString *goo = d->file_spec->getEmbeddedFile()->createDate();
    return goo ? detail::convert_date(goo->getCString()) : time_type(-1);
}

/**
 \returns the checksum of the embedded file
 */
byte_array embedded_file::checksum() const
{
    GooString *cs = d->file_spec->getEmbeddedFile()->checksum();
    if (!cs) {
        return byte_array();
    }
    const char *ccs = cs->getCString();
    byte_array data(cs->getLength());
    for (int i = 0; i < cs->getLength(); ++i) {
        data[i] = ccs[i];
    }
    return data;
}

/**
 \returns the MIME type of the embedded file, if available
 */
std::string embedded_file::mime_type() const
{
    GooString *goo = d->file_spec->getEmbeddedFile()->mimeType();
    return goo ? std::string(goo->getCString()) : std::string();
}

/**
 Reads all the data of the embedded file.

 \returns the data of the embedded file
 */
byte_array embedded_file::data() const
{
    if (!is_valid()) {
        return byte_array();
    }
    Stream *stream = d->file_spec->getEmbeddedFile()->stream();
    if (!stream) {
        return byte_array();
    }

    stream->reset();
    byte_array ret(1024);
    size_t data_len = 0;
    int i;
    while ((i = stream->getChar()) != EOF) {
        if (data_len == ret.size()) {
            ret.resize(ret.size() * 2);
        }
        ret[data_len] = (char)i;
        ++data_len;
    }
    ret.resize(data_len);
    return ret;
}
