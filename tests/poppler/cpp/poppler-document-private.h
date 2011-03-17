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

#ifndef POPPLER_DOCUMENT_PRIVATE_H
#define POPPLER_DOCUMENT_PRIVATE_H

#include "poppler-global.h"

#include "poppler-config.h"
#include "GooString.h"
#include "PDFDoc.h"

#include <vector>

namespace poppler
{

class document;
class embedded_file;

class document_private
{
public:
    document_private(GooString *file_path, const std::string &owner_password,
                     const std::string &user_password);
    document_private(byte_array *file_data, const std::string &owner_password,
                     const std::string &user_password);
    document_private(const char *file_data, int file_data_length,
                     const std::string &owner_password,
                     const std::string &user_password);
    ~document_private();

    static document* check_document(document_private *doc, byte_array *file_data);

    PDFDoc *doc;
    byte_array doc_data;
    const char *raw_doc_data;
    int raw_doc_data_length;
    bool is_locked;
    std::vector<embedded_file *> embedded_files;

private:
    void init();

    static unsigned int count;
};

}

#endif
