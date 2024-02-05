/* poppler-features.h: glib interface to poppler
 * Copyright (C) 2006, Red Hat, Inc.
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

#ifndef __POPPLER_FEATURES_H__
#define __POPPLER_FEATURES_H__

@CAIRO_FEATURE@
@GDK_FEATURE@

#define POPPLER_MAJOR_VERSION (@POPPLER_MAJOR_VERSION@)
#define POPPLER_MINOR_VERSION (@POPPLER_MINOR_VERSION@)
#define POPPLER_MICRO_VERSION (@POPPLER_MICRO_VERSION@)

#define POPPLER_CHECK_VERSION(major,minor,micro) \
  (POPPLER_MAJOR_VERSION > (major) || \
   (POPPLER_MAJOR_VERSION == (major) && POPPLER_MINOR_VERSION > (minor)) || \
   (POPPLER_MAJOR_VERSION == (major) && POPPLER_MINOR_VERSION == (minor) && POPPLER_MICRO_VERSION >= (micro)))

#endif /* __POPPLER_FEATURES_H__ */
