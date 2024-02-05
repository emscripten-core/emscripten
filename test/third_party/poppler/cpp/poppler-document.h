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

#ifndef POPPLER_DOCUMENT_H
#define POPPLER_DOCUMENT_H

#include "poppler-global.h"
#include "poppler-font.h"

namespace poppler
{

class document_private;
class embedded_file;
class page;
class toc;

class POPPLER_CPP_EXPORT document : public poppler::noncopyable
{
public:
    enum page_mode_enum {
        use_none,
        use_outlines,
        use_thumbs,
        fullscreen,
        use_oc,
        use_attach
    };

    enum page_layout_enum {
        no_layout,
        single_page,
        one_column,
        two_column_left,
        two_column_right,
        two_page_left,
        two_page_right
    };

    ~document();

    bool is_locked() const;
    bool unlock(const std::string &owner_password, const std::string &user_password);

    page_mode_enum page_mode() const;
    page_layout_enum page_layout() const;
    void get_pdf_version(int *major, int *minor) const;
    std::vector<std::string> info_keys() const;
    ustring info_key(const std::string &key) const;
    time_type info_date(const std::string &key) const;
    bool is_encrypted() const;
    bool is_linearized() const;
    bool has_permission(permission_enum which) const;
    ustring metadata() const;
    bool get_pdf_id(std::string *permanent_id, std::string *update_id) const;

    int pages() const;
    page* create_page(const ustring &label) const;
    page* create_page(int index) const;

    std::vector<font_info> fonts() const;
    font_iterator* create_font_iterator(int start_page = 0) const;

    toc* create_toc() const;

    bool has_embedded_files() const;
    std::vector<embedded_file *> embedded_files() const;

    static document* load_from_file(const std::string &file_name,
                                    const std::string &owner_password = std::string(),
                                    const std::string &user_password = std::string());
    static document* load_from_data(byte_array *file_data,
                                    const std::string &owner_password = std::string(),
                                    const std::string &user_password = std::string());
    static document* load_from_raw_data(const char *file_data,
                                        int file_data_length,
                                        const std::string &owner_password = std::string(),
                                        const std::string &user_password = std::string());

private:
    document(document_private &dd);

    document_private *d;
    friend class document_private;
};

}

#endif
