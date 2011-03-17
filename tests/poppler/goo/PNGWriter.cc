//========================================================================
//
// PNGWriter.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2009 Warren Toomey <wkt@tuhs.org>
// Copyright (C) 2009 Shen Liang <shenzhuxi@gmail.com>
// Copyright (C) 2009 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2010 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2011 Thomas Klausner <wiz@danbala.tuwien.ac.at>
//
//========================================================================

#include "PNGWriter.h"

#ifdef ENABLE_LIBPNG

#include <zlib.h>

#include "poppler/Error.h"

PNGWriter::PNGWriter()
{
}

PNGWriter::~PNGWriter()
{
	/* cleanup heap allocation */
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

bool PNGWriter::init(FILE *f, int width, int height, int hDPI, int vDPI)
{
	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		error(-1, "png_create_write_struct failed");
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		error(-1, "png_create_info_struct failed");
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		error(-1, "png_jmpbuf failed");
		return false;
	}

	/* write header */
	png_init_io(png_ptr, f);
	if (setjmp(png_jmpbuf(png_ptr))) {
		error(-1, "Error during writing header");
		return false;
	}
	
	// Set up the type of PNG image and the compression level
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	png_byte bit_depth = 8;
	png_byte color_type = PNG_COLOR_TYPE_RGB;
	png_byte interlace_type = PNG_INTERLACE_NONE;

	png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, interlace_type, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// PNG_RESOLUTION_UNKNOWN means dots per inch
	png_set_pHYs(png_ptr, info_ptr, hDPI, vDPI, PNG_RESOLUTION_UNKNOWN);

	png_write_info(png_ptr, info_ptr);
	if (setjmp(png_jmpbuf(png_ptr))) {
		error(-1, "error during writing png info bytes");
		return false;
	}
	
	return true;
}

bool PNGWriter::writePointers(unsigned char **rowPointers, int rowCount)
{
	png_write_image(png_ptr, rowPointers);
	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr))) {
		error(-1, "Error during writing bytes");
		return false;
	}
	
	return true;
}

bool PNGWriter::writeRow(unsigned char **row)
{
	// Write the row to the file
	png_write_rows(png_ptr, row, 1);
	if (setjmp(png_jmpbuf(png_ptr))) {
		error(-1, "error during png row write");
		return false;
	}
	
	return true;
}

bool PNGWriter::close()
{
	/* end write */
	png_write_end(png_ptr, info_ptr);
	if (setjmp(png_jmpbuf(png_ptr))) {
		error(-1, "Error during end of write");
		return false;
	}
	
	return true;
}

#endif
