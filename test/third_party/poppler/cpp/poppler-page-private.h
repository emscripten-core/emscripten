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

#ifndef POPPLER_PAGE_PRIVATE_H
#define POPPLER_PAGE_PRIVATE_H

#include "poppler-page.h"

class Page;

namespace poppler
{

class document_private;
class page_transition;

class page_private
{
public:
    page_private(document_private *doc, int index);
    ~page_private();

    document_private *doc;
    Page *page;
    int index;
    page_transition *transition;

    static inline page_private* get(const poppler::page *p)
    { return const_cast<poppler::page *>(p)->d; }
};

}

#endif
