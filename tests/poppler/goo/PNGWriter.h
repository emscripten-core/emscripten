//========================================================================
//
// PNGWriter.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2009 Warren Toomey <wkt@tuhs.org>
// Copyright (C) 2009 Shen Liang <shenzhuxi@gmail.com>
// Copyright (C) 2009 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2010 Adrian Johnson <ajohnson@redneon.com>
//
//========================================================================

#ifndef PNGWRITER_H
#define PNGWRITER_H

#include <config.h>

#ifdef ENABLE_LIBPNG

#include <cstdio>
#include <png.h>
#include "ImgWriter.h"

class PNGWriter : public ImgWriter
{
	public:
		PNGWriter();
		~PNGWriter();
		
		bool init(FILE *f, int width, int height, int hDPI, int vDPI);
		
		bool writePointers(unsigned char **rowPointers, int rowCount);
		bool writeRow(unsigned char **row);
		
		bool close();
	
	private:
		png_structp png_ptr;
		png_infop info_ptr;
};

#endif

#endif
