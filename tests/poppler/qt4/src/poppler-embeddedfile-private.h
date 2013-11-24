/* poppler-embeddedfile-private.h: Qt4 interface to poppler
 * Copyright (C) 2005, 2008, 2009, 2012, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2005, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2008, 2011, Pino Toscano <pino@kde.org>
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

#ifndef POPPLER_EMBEDDEDFILE_PRIVATE_H
#define POPPLER_EMBEDDEDFILE_PRIVATE_H

class FileSpec;

namespace Poppler
{

class EmbeddedFileData
{
public:
	EmbeddedFileData(FileSpec *fs);
	~EmbeddedFileData();
    
	EmbFile *embFile() const;

	FileSpec *filespec;
};

}

#endif
