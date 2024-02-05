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

#include "poppler-document.h"
#include "poppler-embedded-file.h"
#include "poppler-page.h"
#include "poppler-toc.h"

#include "poppler-document-private.h"
#include "poppler-embedded-file-private.h"
#include "poppler-private.h"
#include "poppler-toc-private.h"

#include "Catalog.h"
#include "ErrorCodes.h"
#include "GlobalParams.h"
#include "Outline.h"

#include <algorithm>
#include <iterator>
#include <memory>

using namespace poppler;

unsigned int poppler::document_private::count = 0U;

document_private::document_private(GooString *file_path, const std::string &owner_password,
                                   const std::string &user_password)
    : doc(0)
    , raw_doc_data(0)
    , raw_doc_data_length(0)
    , is_locked(false)
{
    GooString goo_owner_password(owner_password.c_str());
    GooString goo_user_password(user_password.c_str());
    doc = new PDFDoc(file_path, &goo_owner_password, &goo_user_password);
    init();
}

document_private::document_private(byte_array *file_data,
                                   const std::string &owner_password,
                                   const std::string &user_password)
    : doc(0)
    , raw_doc_data(0)
    , raw_doc_data_length(0)
    , is_locked(false)
{
    Object obj;
    obj.initNull();
    file_data->swap(doc_data);
    MemStream *memstr = new MemStream(&doc_data[0], 0, doc_data.size(), &obj);
    GooString goo_owner_password(owner_password.c_str());
    GooString goo_user_password(user_password.c_str());
    doc = new PDFDoc(memstr, &goo_owner_password, &goo_user_password);
    init();
}

document_private::document_private(const char *file_data, int file_data_length,
                                   const std::string &owner_password,
                                   const std::string &user_password)
    : doc(0)
    , raw_doc_data(file_data)
    , raw_doc_data_length(file_data_length)
    , is_locked(false)
{
    Object obj;
    obj.initNull();
    MemStream *memstr = new MemStream(const_cast<char *>(raw_doc_data), 0, raw_doc_data_length, &obj);
    GooString goo_owner_password(owner_password.c_str());
    GooString goo_user_password(user_password.c_str());
    doc = new PDFDoc(memstr, &goo_owner_password, &goo_user_password);
    init();
}

document_private::~document_private()
{
    delete_all(embedded_files);

    delete doc;

    if (count > 0) {
        --count;
        if (!count) {
            delete globalParams;
            globalParams = 0;
        }
    }
}

void document_private::init()
{
    if (!count) {
        globalParams = new GlobalParams();
        setErrorFunction(detail::error_function);
    }
    count++;
}

document* document_private::check_document(document_private *doc, byte_array *file_data)
{
    if (doc->doc->isOk() || doc->doc->getErrorCode() == errEncrypted) {
        if (doc->doc->getErrorCode() == errEncrypted) {
            doc->is_locked = true;
        }
        return new document(*doc);
    } else {
        // put back the document data where it was before
        if (file_data) {
            file_data->swap(doc->doc_data);
        }
        delete doc;
    }
    return 0;
}

/**
 \class poppler::document poppler-document.h "poppler/cpp/poppler-document.h"

 Represents a PDF %document.
 */

/**
 \enum poppler::document::page_mode_enum

 The various page modes available in a PDF %document.
*/
/**
 \var poppler::document::page_mode_enum poppler::document::use_none

 The %document specifies no particular page mode.
*/
/**
 \var poppler::document::page_mode_enum poppler::document::use_outlines

 The %document specifies its TOC (table of contents) should be open.
*/
/**
 \var poppler::document::page_mode_enum poppler::document::use_thumbs

 The %document specifies that should be open a view of the thumbnails of its
 pages.
*/
/**
 \var poppler::document::page_mode_enum poppler::document::fullscreen

 The %document specifies it wants to be open in a fullscreen mode.
*/
/**
 \var poppler::document::page_mode_enum poppler::document::use_oc

 The %document specifies that should be open a view of its Optional Content
 (also known as layers).
*/
/**
 \var poppler::document::page_mode_enum poppler::document::use_attach

 The %document specifies that should be open a view of its %document-level
 attachments.
 */


document::document(document_private &dd)
    : d(&dd)
{
}

document::~document()
{
    delete d;
}

/**
 \returns whether the current %document is locked
 */
bool document::is_locked() const
{
    return d->is_locked;
}

/**
 Unlocks the current doocument, if locked.

 \returns the new locking status of the document
 */
bool document::unlock(const std::string &owner_password, const std::string &user_password)
{
    if (d->is_locked) {
        document_private *newdoc = 0;
        if (d->doc_data.size() > 0) {
            newdoc = new document_private(&d->doc_data,
                                          owner_password, user_password);
        } else if (d->raw_doc_data) {
            newdoc = new document_private(d->raw_doc_data, d->raw_doc_data_length,
                                          owner_password, user_password);
        } else {
            newdoc = new document_private(new GooString(d->doc->getFileName()),
                                          owner_password, user_password);
        }
        if (!newdoc->doc->isOk()) {
            d->doc_data.swap(newdoc->doc_data);
            delete newdoc;
        } else {
            delete d;
            d = newdoc;
            d->is_locked = false;
        }
    }
    return d->is_locked;
}

/**
 \returns the eventual page mode specified by the current PDF %document
 */
document::page_mode_enum document::page_mode() const
{
    switch (d->doc->getCatalog()->getPageMode()) {
    case Catalog::pageModeNone:
        return use_none;
    case Catalog::pageModeOutlines:
        return use_outlines;
    case Catalog::pageModeThumbs:
        return use_thumbs;
    case Catalog::pageModeFullScreen:
        return fullscreen;
    case Catalog::pageModeOC:
        return use_oc;
    case Catalog::pageModeAttach:
        return use_attach;
    default:
        return use_none;
    }
}

/**
 \returns the eventual page layout specified by the current PDF %document
 */
document::page_layout_enum document::page_layout() const
{
    switch (d->doc->getCatalog()->getPageLayout()) {
    case Catalog::pageLayoutNone:
        return no_layout;
    case Catalog::pageLayoutSinglePage:
        return single_page;
    case Catalog::pageLayoutOneColumn:
        return one_column;
    case Catalog::pageLayoutTwoColumnLeft:
        return two_column_left;
    case Catalog::pageLayoutTwoColumnRight:
        return two_column_right;
    case Catalog::pageLayoutTwoPageLeft:
        return two_page_left;
    case Catalog::pageLayoutTwoPageRight:
        return two_page_right;
    default:
        return no_layout;
    }
}

/**
 Gets the version of the current PDF %document.

 Example:
 \code
 poppler::document *doc = ...;
 // for example, if the document is PDF 1.6:
 int major = 0, minor = 0;
 doc->get_pdf_version(&major, &minor);
 // major == 1
 // minor == 6
 \endcode

 \param major if not NULL, will be set to the "major" number of the version
 \param minor if not NULL, will be set to the "minor" number of the version
 */
void document::get_pdf_version(int *major, int *minor) const
{
    if (major) {
        *major = d->doc->getPDFMajorVersion();
    }
    if (minor) {
        *minor = d->doc->getPDFMinorVersion();
    }
}

/**
 \returns all the information keys available in the %document
 \see info_key, info_date
 */
std::vector<std::string> document::info_keys() const
{
    if (d->is_locked) {
        return std::vector<std::string>();
    }

    Object info;
    if (!d->doc->getDocInfo(&info)->isDict()) {
        info.free();
        return std::vector<std::string>();
    }

    Dict *info_dict = info.getDict();
    std::vector<std::string> keys(info_dict->getLength());
    for (int i = 0; i < info_dict->getLength(); ++i) {
        keys[i] = std::string(info_dict->getKey(i));
    }

    info.free();
    return keys;
}

/**
 Gets the value of the specified \p key of the document information.

 \returns the value for the \p key, or an empty string if not available
 \see info_keys, info_date
 */
ustring document::info_key(const std::string &key) const
{
    if (d->is_locked) {
        return ustring();
    }

    Object info;
    if (!d->doc->getDocInfo(&info)->isDict()) {
        info.free();
        return ustring();
    }

    Dict *info_dict = info.getDict();
    Object obj;
    ustring result;
    if (info_dict->lookup(PSTR(key.c_str()), &obj)->isString()) {
        result = detail::unicode_GooString_to_ustring(obj.getString());
    }
    obj.free();
    info.free();
    return result;
}

/**
 Gets the time_t value value of the specified \p key of the document
 information.

 \returns the time_t value for the \p key
 \see info_keys, info_date
 */
time_type document::info_date(const std::string &key) const
{
    if (d->is_locked) {
        return time_type(-1);
    }

    Object info;
    if (!d->doc->getDocInfo(&info)->isDict()) {
        info.free();
        return time_type(-1);
    }

    Dict *info_dict = info.getDict();
    Object obj;
    time_type result = time_type(-1);
    if (info_dict->lookup(PSTR(key.c_str()), &obj)->isString()) {
        result = detail::convert_date(obj.getString()->getCString());
    }
    obj.free();
    info.free();
    return result;
}

/**
 \returns whether the document is encrypted
 */
bool document::is_encrypted() const
{
    return d->doc->isEncrypted();
}

/**
 \returns whether the document is linearized
 */
bool document::is_linearized() const
{
    return d->doc->isLinearized();
}

/**
 Check for available "document permission".

 \returns whether the specified permission is allowed
 */
bool document::has_permission(permission_enum which) const
{
    switch (which) {
    case perm_print:
        return d->doc->okToPrint();
    case perm_change:
        return d->doc->okToChange();
    case perm_copy:
        return d->doc->okToCopy();
    case perm_add_notes:
        return d->doc->okToAddNotes();
    case perm_fill_forms:
        return d->doc->okToFillForm();
    case perm_accessibility:
        return d->doc->okToAccessibility();
    case perm_assemble:
        return d->doc->okToAssemble();
    case perm_print_high_resolution:
        return d->doc->okToPrintHighRes();
    }
    return true;
}

/**
 Reads the %document metadata string.

 \return the %document metadata string
 */
ustring document::metadata() const
{
    std::auto_ptr<GooString> md(d->doc->getCatalog()->readMetadata());
    if (md.get()) {
        return detail::unicode_GooString_to_ustring(md.get());
    }
    return ustring();
}

/**
 Gets the IDs of the current PDF %document, if available.

 \param permanent_id if not NULL, will be set to the permanent ID of the %document
 \param update_id if not NULL, will be set to the update ID of the %document

 \returns whether the document has the IDs

 \since 0.16
 */
bool document::get_pdf_id(std::string *permanent_id, std::string *update_id) const
{
    GooString goo_permanent_id;
    GooString goo_update_id;

    if (!d->doc->getID(permanent_id ? &goo_permanent_id : 0, update_id ? &goo_update_id : 0)) {
        return false;
    }

    if (permanent_id) {
        *permanent_id = goo_permanent_id.getCString();
    }
    if (update_id) {
        *update_id = goo_update_id.getCString();
    }

    return true;
}

/**
 Document page count.

 \returns the number of pages of the document
 */
int document::pages() const
{
    return d->doc->getNumPages();
}

/**
 Document page by label reading.

 This creates a new page representing the %document %page whose label is the
 specified \p label. If there is no page with that \p label, NULL is returned.

 \returns a new page object or NULL
 */
page* document::create_page(const ustring &label) const
{
    std::auto_ptr<GooString> goolabel(detail::ustring_to_unicode_GooString(label));
    int index = 0;

    if (!d->doc->getCatalog()->labelToIndex(goolabel.get(), &index)) {
        return 0;
    }
    return create_page(index);
}

/**
 Document page by index reading.

 This creates a new page representing the \p index -th %page of the %document.
 \note the page indexes are in the range [0, pages()[.

 \returns a new page object or NULL
 */
page* document::create_page(int index) const
{
    return index >= 0 && index < d->doc->getNumPages() ? new page(d, index) : 0;
}

/**
 Reads all the font information of the %document.

 \note this can be slow for big documents; prefer the use of a font_iterator
 to read incrementally page by page
 \see create_font_iterator
 */
std::vector<font_info> document::fonts() const
{
    std::vector<font_info> result;
    font_iterator it(0, d);
    while (it.has_next()) {
        const std::vector<font_info> l = it.next();
        std::copy(l.begin(), l.end(), std::back_inserter(result));
    }
    return result;
}

/**
 Creates a new font iterator.

 This creates a new font iterator for reading the font information of the
 %document page by page, starting at the specified \p start_page (0 if not
 specified).

 \returns a new font iterator
 */
font_iterator* document::create_font_iterator(int start_page) const
{
    return new font_iterator(start_page, d);
}

/**
 Reads the TOC (table of contents) of the %document.

 \returns a new toc object if a TOC is available, NULL otherwise
 */
toc* document::create_toc() const
{
    return toc_private::load_from_outline(d->doc->getOutline());
}

/**
 Reads whether the current document has %document-level embedded files
 (attachments).

 This is a very fast way to know whether there are embedded files (also known
 as "attachments") at the %document-level. Note this does not take into account
 files embedded in other ways (e.g. to annotations).

 \returns whether the document has embedded files
 */
bool document::has_embedded_files() const
{
    return d->doc->getCatalog()->numEmbeddedFiles() > 0;
}

/**
 Reads all the %document-level embedded files of the %document.

 \returns the %document-level embedded files
 */
std::vector<embedded_file *> document::embedded_files() const
{
    if (d->is_locked) {
        return std::vector<embedded_file *>();
    }

    if (d->embedded_files.empty() && d->doc->getCatalog()->numEmbeddedFiles() > 0) {
        const int num = d->doc->getCatalog()->numEmbeddedFiles();
        d->embedded_files.resize(num);
        for (int i = 0; i < num; ++i) {
            EmbFile *ef = d->doc->getCatalog()->embeddedFile(i);
            d->embedded_files[i] = embedded_file_private::create(ef);
        }
    }
    return d->embedded_files;
}

/**
 Tries to load a PDF %document from the specified file.

 \param file_name the file to open
 \returns a new document if the load succeeded (even if the document is locked),
          NULL otherwise
 */
document* document::load_from_file(const std::string &file_name,
                                   const std::string &owner_password,
                                   const std::string &user_password)
{
    document_private *doc = new document_private(
                                new GooString(file_name.c_str()),
                                owner_password, user_password);
    return document_private::check_document(doc, 0);
}

/**
 Tries to load a PDF %document from the specified data.

 \note if the loading succeeds, the document takes ownership of the
       \p file_data (swap()ing it)

 \param file_data the data representing a document to open
 \returns a new document if the load succeeded (even if the document is locked),
          NULL otherwise
 */
document* document::load_from_data(byte_array *file_data,
                                   const std::string &owner_password,
                                   const std::string &user_password)
{
    if (!file_data || file_data->size() < 10) {
        return 0;
    }

    document_private *doc = new document_private(
                                file_data, owner_password, user_password);
    return document_private::check_document(doc, file_data);
}

/**
 Tries to load a PDF %document from the specified data buffer.

 \note the buffer must remain valid for the whole lifetime of the returned
       document

 \param file_data the data buffer representing a document to open
 \param file_data_length the length of the data buffer

 \returns a new document if the load succeeded (even if the document is locked),
          NULL otherwise

 \since 0.16
 */
document* document::load_from_raw_data(const char *file_data,
                                       int file_data_length,
                                       const std::string &owner_password,
                                       const std::string &user_password)
{
    if (!file_data || file_data_length < 10) {
        return 0;
    }

    document_private *doc = new document_private(
                                file_data, file_data_length,
                                owner_password, user_password);
    return document_private::check_document(doc, 0);
}
