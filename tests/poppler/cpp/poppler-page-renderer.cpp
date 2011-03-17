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

#include "poppler-page-renderer.h"

#include "poppler-document-private.h"
#include "poppler-page-private.h"

#include <config.h>

#include "PDFDoc.h"
#if defined(HAVE_SPLASH)
#include "SplashOutputDev.h"
#include "splash/SplashBitmap.h"
#endif

using namespace poppler;

class poppler::page_renderer_private
{
public:
    page_renderer_private()
        : paper_color(0xffffffff)
        , hints(0)
    {
    }

    argb paper_color;
    unsigned int hints;
};


/**
 \class poppler::page_renderer poppler-page-renderer.h "poppler/cpp/poppler-renderer.h"

 Simple way to render a page of a PDF %document.

 \since 0.16
 */

/**
 \enum poppler::page_renderer::render_hint

 A flag of an option taken into account when rendering
*/


/**
 Constructs a new %page renderer.
 */
page_renderer::page_renderer()
    : d(new page_renderer_private())
{
}

/**
 Destructor.
 */
page_renderer::~page_renderer()
{
    delete d;
}

/**
 The color used for the "paper" of the pages.

 The default color is opaque solid white (0xffffffff).

 \returns the paper color
 */
argb page_renderer::paper_color() const
{
    return d->paper_color;
}

/**
 Set a new color for the "paper".

 \param c the new color
 */
void page_renderer::set_paper_color(argb c)
{
    d->paper_color = c;
}

/**
 The hints used when rendering.

 By default no hint is set.

 \returns the render hints set
 */
unsigned int page_renderer::render_hints() const
{
    return d->hints;
}

/**
 Enable or disable a single render %hint.

 \param hint the hint to modify
 \param on whether enable it or not
 */
void page_renderer::set_render_hint(page_renderer::render_hint hint, bool on)
{
    if (on) {
        d->hints |= hint;
    } else {
        d->hints &= ~(int)hint;
    }
}

/**
 Set new render %hints at once.

 \param hints the new set of render hints
 */
void page_renderer::set_render_hints(unsigned int hints)
{
    d->hints = hints;
}

/**
 Render the specified page.

 This functions renders the specified page on an image following the specified
 parameters, returning it.

 \param p the page to render
 \param xres the X resolution, in dot per inch (DPI)
 \param yres the Y resolution, in dot per inch (DPI)
 \param x the X top-right coordinate, in pixels
 \param y the Y top-right coordinate, in pixels
 \param w the width in pixels of the area to render
 \param h the height in pixels of the area to render
 \param rotate the rotation to apply when rendering the page

 \returns the rendered image, or a null one in case of errors

 \see can_render
 */
image page_renderer::render_page(const page *p,
                                 double xres, double yres,
                                 int x, int y, int w, int h,
                                 rotation_enum rotate) const
{
    if (!p) {
        return image();
    }

#if defined(HAVE_SPLASH)
    page_private *pp = page_private::get(p);
    PDFDoc *pdfdoc = pp->doc->doc;

    SplashColor bgColor;
    bgColor[0] = d->paper_color & 0xff;
    bgColor[1] = (d->paper_color >> 8) & 0xff;
    bgColor[2] = (d->paper_color >> 16) & 0xff;
    const GBool text_AA = d->hints & text_antialiasing ? gTrue : gFalse;
    SplashOutputDev splashOutputDev(splashModeXBGR8, 4, gFalse, bgColor, gTrue, text_AA);
    splashOutputDev.setVectorAntialias(d->hints & antialiasing ? gTrue : gFalse);
    splashOutputDev.setFreeTypeHinting(d->hints & text_hinting ? gTrue : gFalse);
    splashOutputDev.startDoc(pdfdoc->getXRef());
    pdfdoc->displayPageSlice(&splashOutputDev, pp->index + 1,
                             xres, yres, int(rotate) * 90,
                             gFalse, gTrue, gFalse,
                             x, y, w, h);

    SplashBitmap *bitmap = splashOutputDev.getBitmap();
    const int bw = bitmap->getWidth();
    const int bh = bitmap->getHeight();

    SplashColorPtr data_ptr = bitmap->getDataPtr();

    const image img(reinterpret_cast<char *>(data_ptr), bw, bh, image::format_argb32);
    return img.copy();
#else
    return image();
#endif
}

/**
 Rendering capability test.

 page_renderer can render only if a render backend ('Splash') is compiled in
 Poppler.

 \returns whether page_renderer can render
 */
bool page_renderer::can_render()
{
#if defined(HAVE_SPLASH)
    return true;
#else
    return false;
#endif
}
