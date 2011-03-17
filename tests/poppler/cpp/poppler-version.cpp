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

#include "poppler-version.h"

using namespace poppler;

/**
 \returns the version string of the current poppler-cpp library
 */
std::string poppler::version_string()
{
    return std::string(POPPLER_VERSION);
}

/**
 \returns the "major" number of the version of the current poppler-cpp library
 */
unsigned int poppler::version_major()
{
    return POPPLER_VERSION_MAJOR;
}

/**
 \returns the "minor" number of the version of the current poppler-cpp library
 */
unsigned int poppler::version_minor()
{
    return POPPLER_VERSION_MINOR;
}

/**
 \returns the "micro" number of the version of the current poppler-cpp library
 */
unsigned int poppler::version_micro()
{
    return POPPLER_VERSION_MICRO;
}
