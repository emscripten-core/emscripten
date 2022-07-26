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

#include "poppler-font.h"

#include "poppler-document-private.h"

#include "FontInfo.h"

using namespace poppler;

class poppler::font_info_private
{
public:
    font_info_private()
        : type(font_info::unknown)
        , is_embedded(false)
        , is_subset(false)
    {
    }
    font_info_private(FontInfo *fi)
        : type((font_info::type_enum)fi->getType())
        , is_embedded(fi->getEmbedded())
        , is_subset(fi->getSubset())
        , emb_ref(fi->getEmbRef())
    {
        if (fi->getName()) {
            font_name = fi->getName()->getCString();
        }
        if (fi->getFile()) {
            font_file = fi->getFile()->getCString();
        }
    }

    std::string font_name;
    std::string font_file;
    font_info::type_enum type : 5;
    bool is_embedded : 1;
    bool is_subset : 1;
    Ref emb_ref;
};


class poppler::font_iterator_private
{
public:
    font_iterator_private(int start_page, document_private *dd)
        : font_info_scanner(dd->doc, start_page)
        , total_pages(dd->doc->getNumPages())
        , current_page((std::max)(start_page, 0))
    {
    }
    ~font_iterator_private()
    {
    }

    FontInfoScanner font_info_scanner;
    int total_pages;
    int current_page;
};

/**
 \class poppler::font_info poppler-font.h "poppler/cpp/poppler-font.h"

 The information about a font used in a PDF %document.
 */

/**
 \enum poppler::font_info::type_enum

 The various types of fonts available in a PDF %document.
*/


/**
 Constructs an invalid font information.
 */
font_info::font_info()
    : d(new font_info_private())
{
}

font_info::font_info(font_info_private &dd)
    : d(&dd)
{
}

/**
 Copy constructor.
 */
font_info::font_info(const font_info &fi)
    : d(new font_info_private(*fi.d))
{
}

/**
 Destructor.
 */
font_info::~font_info()
{
    delete d;
}

/**
 \returns the name of the font
 */
std::string font_info::name() const
{
    return d->font_name;
}

/**
 \returns the file name of the font, in case the font is not embedded nor subset
 */
std::string font_info::file() const
{
    return d->font_file;
}

/**
 \returns whether the font is totally embedded in the %document
 */
bool font_info::is_embedded() const
{
    return d->is_embedded;
}

/**
 \returns whether there is a subset of the font embedded in the %document
 */
bool font_info::is_subset() const
{
    return d->is_subset;
}

/**
 \returns the type of the font
 */
font_info::type_enum font_info::type() const
{
    return d->type;
}

/**
 Assignment operator.
 */
font_info& font_info::operator=(const font_info &fi)
{
    if (this != &fi) {
        *d = *fi.d;
    }
    return *this;
}

/**
 \class poppler::font_iterator poppler-font.h "poppler/cpp/poppler-font.h"

 Reads the fonts in the PDF %document page by page.

 font_iterator is the way to collect the list of the fonts used in a PDF
 %document, reading them incrementally page by page.

 A typical usage of this might look like:
 \code
poppler::font_iterator *it = doc->create_font_iterator();
while (it->has_next()) {
    std::vector<poppler::font_info> fonts = it->next();
    // do domething with the fonts
}
// after we are done with the iterator, it must be deleted
delete it;
\endcode
 */


font_iterator::font_iterator(int start_page, document_private *dd)
    : d(new font_iterator_private(start_page, dd))
{
}

/**
 Destructor.
 */
font_iterator::~font_iterator()
{
    delete d;
}

/**
 Returns the fonts of the current page and advances to the next one.
 */
std::vector<font_info> font_iterator::next()
{
    if (!has_next()) {
        return std::vector<font_info>();
    }

    ++d->current_page;

    GooList *items = d->font_info_scanner.scan(1);
    if (!items) {
        return std::vector<font_info>();
    }
    std::vector<font_info> fonts(items->getLength());
    for (int i = 0; i < items->getLength(); ++i) {
        fonts[i] = font_info(*new font_info_private((FontInfo *)items->get(i)));
    }
    deleteGooList(items, FontInfo);
    return fonts;
}

/**
 \returns whether the iterator has more pages to advance to
*/
bool font_iterator::has_next() const
{
    return d->current_page < d->total_pages;
}

/**
 \returns the current page
*/
int font_iterator::current_page() const
{
    return d->current_page;
}
