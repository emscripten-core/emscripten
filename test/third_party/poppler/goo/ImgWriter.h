//========================================================================
//
// ImgWriter.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2009 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Brian Cameron <brian.cameron@oracle.com>
//
//========================================================================

#ifndef IMGWRITER_H
#define IMGWRITER_H

#include <config.h>
#include <stdio.h>
	
class ImgWriter
{
	public:
		virtual ~ImgWriter();
		virtual bool init(FILE *f, int width, int height, int hDPI, int vDPI) = 0;
		
		virtual bool writePointers(unsigned char **rowPointers, int rowCount) = 0;
		virtual bool writeRow(unsigned char **row) = 0;
		
		virtual bool close() = 0;
};

#endif
