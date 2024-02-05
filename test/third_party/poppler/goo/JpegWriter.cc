//========================================================================
//
// JpegWriter.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2010 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Harry Roberts <harry.roberts@midnight-labs.org>
//
//========================================================================

#include "JpegWriter.h"

#ifdef ENABLE_LIBJPEG

#include "poppler/Error.h"

void outputMessage(j_common_ptr cinfo)
{
	char buffer[JMSG_LENGTH_MAX];

	// Create the message
	(*cinfo->err->format_message) (cinfo, buffer);

	// Send it to poppler's error handler
	error(-1, "%s", buffer);
}

JpegWriter::JpegWriter(int q, bool p)
: progressive(p), quality(q)
{
}

JpegWriter::JpegWriter()
: progressive(false), quality(-1)
{
}

JpegWriter::~JpegWriter()
{
	// cleanup
	jpeg_destroy_compress(&cinfo);
}

bool JpegWriter::init(FILE *f, int width, int height, int hDPI, int vDPI)
{
	// Setup error handler
	cinfo.err = jpeg_std_error(&jerr);
	jerr.output_message = &outputMessage;

	// Initialize libjpeg
	jpeg_create_compress(&cinfo);
	
	// Set destination file
	jpeg_stdio_dest(&cinfo, f);
	
	// Set libjpeg configuration
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.density_unit = 1; // dots per inch
	cinfo.X_density = hDPI;
	cinfo.Y_density = vDPI;
	cinfo.input_components = 3;     /* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; /* colorspace of input image */
	jpeg_set_defaults(&cinfo);
	
	// Set quality
	if( quality >= 0 && quality <= 100 ) { 
		jpeg_set_quality(&cinfo, quality, true);
	}
	
	// Use progressive mode
	if( progressive) {
		jpeg_simple_progression(&cinfo);
	}
	
	// Get ready for data
	jpeg_start_compress(&cinfo, TRUE);
	
	return true;
}

bool JpegWriter::writePointers(unsigned char **rowPointers, int rowCount)
{
	// Write all rows to the file
	jpeg_write_scanlines(&cinfo, rowPointers, rowCount);
	
	return true;
}

bool JpegWriter::writeRow(unsigned char **row)
{
	// Write the row to the file
	jpeg_write_scanlines(&cinfo, row, 1);
	
	return true;
}

bool JpegWriter::close()
{
	jpeg_finish_compress(&cinfo);
	
	return true;
}

#endif
