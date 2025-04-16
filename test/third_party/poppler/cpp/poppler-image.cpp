/*
 * Copyright (C) 2010, Pino Toscano <pino@kde.org>
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

#include "poppler-image.h"

#include "poppler-image-private.h"

#include <config.h>
#include "ImgWriter.h"
#if defined(ENABLE_LIBPNG)
#include "PNGWriter.h"
#endif
#if defined(ENABLE_LIBJPEG)
#include "JpegWriter.h"
#endif

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <memory>
#include <vector>

namespace {

struct FileCloser {
    inline FileCloser(FILE *ff)
        : f(ff) {}
    inline ~FileCloser()
    { (void)close(); }
    inline bool close()
    { if (f) { const int c = fclose(f); f = 0; return c == 0; } return true; }

    FILE *f;
};

int calc_bytes_per_row(int width, poppler::image::format_enum format)
{
    switch (format) {
    case poppler::image::format_invalid:
        return 0;
    case poppler::image::format_mono:
        return (width + 7) >> 3;
    case poppler::image::format_rgb24:
        return width * 3;
    case poppler::image::format_argb32:
        return width * 4;
    }
    return 0;
}

}

using namespace poppler;

image_private::image_private(int iwidth, int iheight, image::format_enum iformat)
    : ref(1)
    , data(0)
    , width(iwidth)
    , height(iheight)
    , bytes_per_row(0)
    , bytes_num(0)
    , format(iformat)
    , own_data(true)
{
}

image_private::~image_private()
{
    if (own_data) {
        std::free(data);
    }
}

image_private *image_private::create_data(int width, int height, image::format_enum format)
{
    if (width <= 0 || height <= 0) {
        return 0;
    }

    int bpr = calc_bytes_per_row(width, format);
    if (bpr <= 0) {
        return 0;
    }

    std::auto_ptr<image_private> d(new image_private(width, height, format));
    d->bytes_num = bpr * height;
    d->data = reinterpret_cast<char *>(std::malloc(d->bytes_num));
    if (!d->data) {
        return 0;
    }
    d->own_data = true;
    d->bytes_per_row = bpr;

    return d.release();
}

image_private *image_private::create_data(char *data, int width, int height, image::format_enum format)
{
    if (width <= 0 || height <= 0 || !data) {
        return 0;
    }

    int bpr = calc_bytes_per_row(width, format);
    if (bpr <= 0) {
        return 0;
    }

    std::auto_ptr<image_private> d(new image_private(width, height, format));
    d->bytes_num = bpr * height;
    d->data = data;
    d->own_data = false;
    d->bytes_per_row = bpr;

    return d.release();
}

/**
 \class poppler::image poppler-image.h "poppler/cpp/poppler-image.h"

 A simple representation of image, with direct access to the data.

 This class uses implicit sharing for the internal data, so it can be used as
 value class. This also means any non-const operation will make sure that the
 data used by current instance is not shared with other instances (ie
 \em detaching), copying the shared data.

 \since 0.16
 */

/**
 \enum poppler::image::format_enum

 The possible formats for an image.
*/


/**
 Construct an invalid image.
 */
image::image()
    : d(0)
{
}

/**
 Construct a new image.

 It allocates the storage needed for the image data; if the allocation fails,
 the image is an invalid one.

 \param iwidth the width for the image
 \param iheight the height for the image
 \param iformat the format for the bits of the image
 */
image::image(int iwidth, int iheight, image::format_enum iformat)
    : d(image_private::create_data(iwidth, iheight, iformat))
{
}

/**
 Construct a new image.

 It uses the provide data buffer for the image, so you \b must ensure it
 remains valid for the whole lifetime of the image.

 \param idata the buffer to use for the image
 \param iwidth the width for the image
 \param iheight the height for the image
 \param iformat the format for the bits of the image
 */
image::image(char *idata, int iwidth, int iheight, image::format_enum iformat)
    : d(image_private::create_data(idata, iwidth, iheight, iformat))
{
}

/**
 Copy constructor.
 */
image::image(const image &pt)
    : d(pt.d)
{
    if (d) {
        ++d->ref;
    }
}

/**
 Destructor.
 */
image::~image()
{
    if (d && !--d->ref) {
        delete d;
    }
}

/**
 Image validity check.

 \returns whether the image is valid.
 */
bool image::is_valid() const
{
    return d && d->format != format_invalid;
}

/**
 \returns the format of the image
 */
image::format_enum image::format() const
{
    return d ? d->format : format_invalid;
}

/**
 \returns whether the width of the image
 */
int image::width() const
{
    return d ? d->width : 0;
}

/**
 \returns whether the height of the image
 */
int image::height() const
{
    return d ? d->height : 0;
}

/**
 \returns the number of bytes in each row of the image
 */
int image::bytes_per_row() const
{
    return d ? d->bytes_per_row : 0;
}

/**
 Access to the image bits.

 This function will detach and copy the shared data.

 \returns the pointer to the first pixel
 */
char *image::data()
{
    if (!d) {
        return 0;
    }

    detach();
    return d->data;
}

/**
 Access to the image bits.

 This function provides const access to the data.

 \returns the pointer to the first pixel
 */
const char *image::const_data() const
{
    return d ? d->data : 0;
}

/**
 Copy of a slice of the image.

 \param r the sub-area of this image to copy; if empty, the whole image is
          copied

 \returns a new image representing the specified part of the current image
 */
image image::copy(const rect &r) const
{
    if (r.is_empty()) {
        image img(*this);
        img.detach();
        return img;
    }

    // ### FIXME
    return *this;
}

/**
 Saves the current image to file.

 Using this function it is possible to save the image to the specified
 \p file_name, in the specified \p out_format and with a resolution of the
 specified \p dpi.

 Image formats commonly supported are:
 \li PNG: \c png
 \li JPEG: \c jpeg, \c jpg

 If an image format is not supported (check the result of
 supported_image_formats()), the saving fails.

 \returns whether the saving succeeded
 */
bool image::save(const std::string &file_name, const std::string &out_format, int dpi) const
{
    if (!is_valid() || file_name.empty() || out_format.empty()) {
        return false;
    }

    std::string fmt = out_format;
    std::transform(fmt.begin(), fmt.end(), fmt.begin(), tolower);

    std::auto_ptr<ImgWriter> w;
    const int actual_dpi = dpi == -1 ? 75 : dpi;
    if (false) {
    }
#if defined(ENABLE_LIBPNG)
    else if (fmt == "png") {
        w.reset(new PNGWriter());
    }
#endif
#if defined(ENABLE_LIBJPEG)
    else if (fmt == "jpeg" || fmt == "jpg") {
        w.reset(new JpegWriter());
    }
#endif
    if (!w.get()) {
        return false;
    }
    FILE *f = fopen(file_name.c_str(), "w");
    if (!f) {
        return false;
    }
    const FileCloser fc(f);
    if (!w->init(f, d->width, d->height, actual_dpi, actual_dpi)) {
        return false;
    }
    switch (d->format) {
    case format_invalid:
        return false;
    case format_mono:
        return false;
    case format_rgb24: {
        char *hptr = d->data;
        for (int y = 0; y < d->height; ++y) {
            if (!w->writeRow(reinterpret_cast<unsigned char **>(&hptr))) {
                return false;
            }
            hptr += d->bytes_per_row;
        }
        break;
    }
    case format_argb32: {
        std::vector<unsigned char> row(3 * d->width);
        char *hptr = d->data;
        for (int y = 0; y < d->height; ++y) {
            unsigned char *rowptr = &row[0];
            for (int x = 0; x < d->width; ++x, rowptr += 3) {
                const unsigned int pixel = *reinterpret_cast<unsigned int *>(hptr + x * 4);
                rowptr[0] = (pixel >> 16) & 0xff;
                rowptr[1] = (pixel >> 8) & 0xff;
                rowptr[2] = pixel & 0xff;
            }
            rowptr = &row[0];
            if (!w->writeRow(&rowptr)) {
                return false;
            }
            hptr += d->bytes_per_row;
        }
        break;
    }
    }

    if (!w->close()) {
        return false;
    }

    return true;
}

/**
 \returns a list of the supported image formats
 */
std::vector<std::string> image::supported_image_formats()
{
    std::vector<std::string> formats;
#if defined(ENABLE_LIBPNG)
    formats.push_back("png");
#endif
#if defined(ENABLE_LIBJPEG)
    formats.push_back("jpeg");
    formats.push_back("jpg");
#endif
    return formats;
}

/**
 Assignment operator.
 */
image& image::operator=(const image &pt)
{
    if (pt.d) {
        ++pt.d->ref;
    }
    image_private *old_d = d;
    d = pt.d;
    if (old_d && !--old_d->ref) {
        delete old_d;
    }
    return *this;
}

void image::detach()
{
    if (d->ref == 1) {
        return;
    }

    image_private *old_d = d;
    d = image_private::create_data(old_d->width, old_d->height, old_d->format);
    if (d) {
        std::memcpy(d->data, old_d->data, old_d->bytes_num);
        --old_d->ref;
    } else {
        d = old_d;
    }
}
