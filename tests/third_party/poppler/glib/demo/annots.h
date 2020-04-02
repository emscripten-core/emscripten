/*
 * Copyright (C) 2008 Inigo Martinez <inigomartinez@gmail.com>
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

#include <gtk/gtk.h>
#include <poppler.h>

#ifndef _ANNOTS_H_
#define _ANNOTS_H_

G_BEGIN_DECLS

GtkWidget *pgd_annots_create_widget (PopplerDocument *document);

G_END_DECLS

#endif /* _ANNOTS_H_ */
