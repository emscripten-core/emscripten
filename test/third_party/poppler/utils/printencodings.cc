/*
 * Copyright (C) 2008, Albert Astals Cid <aacid@kde.org>
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

#include "printencodings.h"

#include "GlobalParams.h"
#include "goo/GooList.h"
#include "goo/GooString.h"

void printEncodings()
{
  GooList *encNames = globalParams->getEncodingNames();
  printf("Available encodings are:\n");
  for (int i = 0; i < encNames->getLength(); ++i) {
    GooString *enc = (GooString*)encNames->get(i);
    printf("%s\n", enc->getCString());
  }
  delete encNames;
}
