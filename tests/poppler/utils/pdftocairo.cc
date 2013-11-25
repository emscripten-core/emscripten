//========================================================================
//
// pdftocairo.cc
//
// Copyright 2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2007 Ilmari Heikkinen <ilmari.heikkinen@gmail.com>
// Copyright (C) 2008 Richard Airlie <richard.airlie@maglabs.net>
// Copyright (C) 2009 Michael K. Johnson <a1237@danlj.org>
// Copyright (C) 2009 Shen Liang <shenzhuxi@gmail.com>
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2009, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010, 2011-2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 Jonathan Liu <net147@gmail.com>
// Copyright (C) 2010 William Bader <williambader@hotmail.com>
// Copyright (C) 2011 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2012 Koji Otani <sho@bbr.jp>
// Copyright (C) 2013 Lu Wang <coolwanglu@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "parseargs.h"
#include "goo/gmem.h"
#include "goo/gtypes.h"
#include "goo/gtypes_p.h"
#include "goo/GooString.h"
#include "goo/ImgWriter.h"
#include "goo/JpegWriter.h"
#include "goo/PNGWriter.h"
#include "goo/TiffWriter.h"
#include "GlobalParams.h"
#include "Object.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "CairoOutputDev.h"
#if USE_CMS
#ifdef USE_LCMS1
#include <lcms.h>
#else
#include <lcms2.h>
#endif
#endif
#include <cairo.h>
#if CAIRO_HAS_PS_SURFACE
#include <cairo-ps.h>
#endif
#if CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif
#if CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>
#endif


static GBool png = gFalse;
static GBool jpeg = gFalse;
static GBool ps = gFalse;
static GBool eps = gFalse;
static GBool pdf = gFalse;
static GBool svg = gFalse;
static GBool tiff = gFalse;

static int firstPage = 1;
static int lastPage = 0;
static GBool printOnlyOdd = gFalse;
static GBool printOnlyEven = gFalse;
static GBool singleFile = gFalse;
static double resolution = 0.0;
static double x_resolution = 150.0;
static double y_resolution = 150.0;
static int scaleTo = 0;
static int x_scaleTo = 0;
static int y_scaleTo = 0;
static int crop_x = 0;
static int crop_y = 0;
static int crop_w = 0;
static int crop_h = 0;
static int sz = 0;
static GBool useCropBox = gFalse;
static GBool mono = gFalse;
static GBool gray = gFalse;
static GBool transp = gFalse;
static GooString icc;

static GBool level2 = gFalse;
static GBool level3 = gFalse;
static GBool doOrigPageSizes = gFalse;
static char paperSize[15] = "";
static int paperWidth = -1;
static int paperHeight = -1;
static GBool noCrop = gFalse;
static GBool expand = gFalse;
static GBool noShrink = gFalse;
static GBool noCenter = gFalse;
static GBool duplex = gFalse;
static char tiffCompressionStr[16] = "";

static char ownerPassword[33] = "";
static char userPassword[33] = "";
static GBool quiet = gFalse;
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static const ArgDesc argDesc[] = {
#if ENABLE_LIBPNG
  {"-png",    argFlag,     &png,           0,
   "generate a PNG file"},
#endif
#if ENABLE_LIBJPEG
  {"-jpeg",   argFlag,     &jpeg,           0,
   "generate a JPEG file"},
#endif
#if ENABLE_LIBTIFF
  {"-tiff",    argFlag,     &tiff,           0,
   "generate a TIFF file"},
  {"-tiffcompression", argString, tiffCompressionStr, sizeof(tiffCompressionStr),
   "set TIFF compression: none, packbits, jpeg, lzw, deflate"},
#endif
#if CAIRO_HAS_PS_SURFACE
  {"-ps",     argFlag,     &ps,            0,
   "generate PostScript file"},
  {"-eps",        argFlag,     &eps,          0,
   "generate Encapsulated PostScript (EPS)"},
#endif
#if CAIRO_HAS_PDF_SURFACE
  {"-pdf",    argFlag,     &pdf,           0,
   "generate a PDF file"},
#endif
#if CAIRO_HAS_SVG_SURFACE
  {"-svg",    argFlag,     &svg,           0,
   "generate a Scalable Vector Graphics (SVG) file"},
#endif

  {"-f",      argInt,      &firstPage,     0,
   "first page to print"},
  {"-l",      argInt,      &lastPage,      0,
   "last page to print"},
  {"-o",      argFlag,      &printOnlyOdd, 0,
   "print only odd pages"},
  {"-e",      argFlag,      &printOnlyEven, 0,
   "print only even pages"},
  {"-singlefile", argFlag,  &singleFile,   0,
   "write only the first page and do not add digits"},

  {"-r",      argFP,       &resolution,    0,
   "resolution, in PPI (default is 150)"},
  {"-rx",      argFP,       &x_resolution,    0,
   "X resolution, in PPI (default is 150)"},
  {"-ry",      argFP,       &y_resolution,    0,
   "Y resolution, in PPI (default is 150)"},
  {"-scale-to",      argInt,       &scaleTo,    0,
   "scales each page to fit within scale-to*scale-to pixel box"},
  {"-scale-to-x",      argInt,       &x_scaleTo,    0,
   "scales each page horizontally to fit in scale-to-x pixels"},
  {"-scale-to-y",      argInt,       &y_scaleTo,    0,
   "scales each page vertically to fit in scale-to-y pixels"},

  {"-x",      argInt,      &crop_x,             0,
   "x-coordinate of the crop area top left corner"},
  {"-y",      argInt,      &crop_y,             0,
   "y-coordinate of the crop area top left corner"},
  {"-W",      argInt,      &crop_w,             0,
   "width of crop area in pixels (default is 0)"},
  {"-H",      argInt,      &crop_h,             0,
   "height of crop area in pixels (default is 0)"},
  {"-sz",     argInt,      &sz,            0,
   "size of crop square in pixels (sets W and H)"},
  {"-cropbox",argFlag,     &useCropBox,    0,
   "use the crop box rather than media box"},

  {"-mono",   argFlag,     &mono,          0,
   "generate a monochrome image file (PNG, JPEG)"},
  {"-gray",   argFlag,     &gray,          0,
   "generate a grayscale image file (PNG, JPEG)"},
  {"-transp",   argFlag,     &transp,          0,
   "use a transparent background instead of white (PNG)"},
#if USE_CMS
  {"-icc",   argGooString,     &icc,          0,
   "ICC color profile to use"},
#endif

  {"-level2",     argFlag,     &level2,         0,
   "generate Level 2 PostScript (PS, EPS)"},
  {"-level3",     argFlag,     &level3,         0,
   "generate Level 3 PostScript (PS, EPS)"},
  {"-origpagesizes",argFlag,   &doOrigPageSizes,0,
   "conserve original page sizes (PS, PDF, SVG)"},
  {"-paper",      argString,   paperSize,       sizeof(paperSize),
   "paper size (letter, legal, A4, A3, match)"},
  {"-paperw",     argInt,      &paperWidth,     0,
   "paper width, in points"},
  {"-paperh",     argInt,      &paperHeight,    0,
   "paper height, in points"},
  {"-nocrop",     argFlag,     &noCrop,         0,
   "don't crop pages to CropBox"},
  {"-expand",     argFlag,     &expand,         0,
   "expand pages smaller than the paper size"},
  {"-noshrink",   argFlag,     &noShrink,       0,
   "don't shrink pages larger than the paper size"},
  {"-nocenter",   argFlag,     &noCenter,       0,
   "don't center pages smaller than the paper size"},
  {"-duplex",     argFlag,     &duplex,         0,
   "enable duplex printing"},

  {"-opw",    argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",    argString,   userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},

  {"-q",      argFlag,     &quiet,         0,
   "don't print any messages or errors"},
  {"-v",      argFlag,     &printVersion,  0,
   "print copyright and version info"},
  {"-h",      argFlag,     &printHelp,     0,
   "print usage information"},
  {"-help",   argFlag,     &printHelp,     0,
   "print usage information"},
  {"--help",  argFlag,     &printHelp,     0,
   "print usage information"},
  {"-?",      argFlag,     &printHelp,     0,
   "print usage information"},
  {NULL}
};


static  cairo_surface_t *surface;
static  GBool printing;
static  FILE *output_file;

#if USE_CMS
static unsigned char *icc_data;
static int icc_data_size;
static cmsHPROFILE profile;
#endif


void writePageImage(GooString *filename)
{
  ImgWriter *writer = 0;
  FILE *file;
  int height, width, stride;
  unsigned char *data;

  if (png) {
#if ENABLE_LIBPNG
    if (transp)
      writer = new PNGWriter(PNGWriter::RGBA);
    else if (gray)
      writer = new PNGWriter(PNGWriter::GRAY);
    else if (mono)
      writer = new PNGWriter(PNGWriter::MONOCHROME);
    else
      writer = new PNGWriter(PNGWriter::RGB);

#if USE_CMS
#ifdef USE_LCMS1
    if (icc_data)
      static_cast<PNGWriter*>(writer)->setICCProfile(cmsTakeProductName(profile), icc_data, icc_data_size);
    else
      static_cast<PNGWriter*>(writer)->setSRGBProfile();
#else
    if (icc_data) {
      cmsUInt8Number profileID[17];
      profileID[16] = '\0';

      cmsGetHeaderProfileID(profile,profileID);
      static_cast<PNGWriter*>(writer)->setICCProfile(reinterpret_cast<char *>(profileID), icc_data, icc_data_size);
    } else {
      static_cast<PNGWriter*>(writer)->setSRGBProfile();
    }
#endif
#endif
#endif

  } else if (jpeg) {
#if ENABLE_LIBJPEG
    if (gray)
      writer = new JpegWriter(JpegWriter::GRAY);
    else
      writer = new JpegWriter(JpegWriter::RGB);
#endif
  } else if (tiff) {
#if ENABLE_LIBTIFF
    if (transp)
      writer = new TiffWriter(TiffWriter::RGBA_PREMULTIPLIED);
    else if (gray)
      writer = new TiffWriter(TiffWriter::GRAY);
    else if (mono)
      writer = new TiffWriter(TiffWriter::MONOCHROME);
    else
      writer = new TiffWriter(TiffWriter::RGB);
    static_cast<TiffWriter*>(writer)->setCompressionString(tiffCompressionStr);
#endif
  }
  if (!writer)
    return;

  if (filename->cmp("fd://0") == 0)
    file = stdout;
  else
    file = fopen(filename->getCString(), "wb");

  if (!file) {
    fprintf(stderr, "Error opening output file %s\n", filename->getCString());
    exit(2);
  }

  height = cairo_image_surface_get_height(surface);
  width = cairo_image_surface_get_width(surface);
  stride = cairo_image_surface_get_stride(surface);
  data = cairo_image_surface_get_data(surface);

  if (!writer->init(file, width, height, x_resolution, y_resolution)) {
    fprintf(stderr, "Error writing %s\n", filename->getCString());
    exit(2);
  }
  unsigned char *row = (unsigned char *) gmallocn(width, 4);

  for (int y = 0; y < height; y++ ) {
    uint32_t *pixel = (uint32_t *) (data + y*stride);
    unsigned char *rowp = row;
    int bit = 7;
    for (int x = 0; x < width; x++, pixel++) {
      if (transp) {
        if (tiff) {
          // RGBA premultipled format
          *rowp++ = (*pixel &   0xff0000) >> 16;
          *rowp++ = (*pixel &   0x00ff00) >>  8;
          *rowp++ = (*pixel &   0x0000ff) >>  0;
          *rowp++ = (*pixel & 0xff000000) >> 24;
        } else {
	// unpremultiply into RGBA format
          uint8_t a;
          a = (*pixel & 0xff000000) >> 24;
          if (a == 0) {
            *rowp++ = 0;
            *rowp++ = 0;
            *rowp++ = 0;
          } else {
            *rowp++ = (((*pixel & 0xff0000) >> 16) * 255 + a / 2) / a;
            *rowp++ = (((*pixel & 0x00ff00) >>  8) * 255 + a / 2) / a;
            *rowp++ = (((*pixel & 0x0000ff) >>  0) * 255 + a / 2) / a;
          }
          *rowp++ = a;
        }
      } else if (gray || mono) {
	// convert to gray
        // The PDF Reference specifies the DeviceRGB to DeviceGray conversion as
	// gray = 0.3*red + 0.59*green + 0.11*blue
	int r = (*pixel & 0x00ff0000) >> 16;
	int g = (*pixel & 0x0000ff00) >>  8;
	int b = (*pixel & 0x000000ff) >>  0;
	// an arbitrary integer approximation of .3*r + .59*g + .11*b
	int y = (r*19661+g*38666+b*7209 + 32829)>>16;
        if (mono) {
          if (bit == 7)
            *rowp = 0;
          if (y > 127)
            *rowp |= (1 << bit);
          bit--;
          if (bit < 0) {
            bit = 7;
            rowp++;
          }
        } else {
          *rowp++ = y;
        }
      } else {
	// copy into RGB format
	*rowp++ = (*pixel & 0x00ff0000) >> 16;
	*rowp++ = (*pixel & 0x0000ff00) >>  8;
	*rowp++ = (*pixel & 0x000000ff) >>  0;
      }
    }
    writer->writeRow(&row);
  }
  gfree(row);
  writer->close();
  delete writer;
  if (file == stdout) fflush(file);
  else fclose(file);
}

static void getCropSize(double page_w, double page_h, double *width, double *height)
{
  int w = crop_w;
  int h = crop_h;

  if (w == 0)
    w = (int)ceil(page_w);

  if (h == 0)
    h = (int)ceil(page_h);

  *width =  (crop_x + w > page_w ? (int)ceil(page_w - crop_x) : w);
  *height = (crop_y + h > page_h ? (int)ceil(page_h - crop_y) : h);
}

static void getOutputSize(double page_w, double page_h, double *width, double *height)
{

  if (printing) {
    if (doOrigPageSizes) {
      *width = page_w;
      *height = page_h;
    } else {
      *width = paperWidth;
      *height = paperHeight;
    }
  } else {
    getCropSize(page_w * (x_resolution / 72.0),
		page_h * (y_resolution / 72.0),
		width, height);
  }
}

static void getFitToPageTransform(double page_w, double page_h,
				  double paper_w, double paper_h,
				  cairo_matrix_t *m)
{
  double x_scale, y_scale, scale;

  x_scale = paper_w / page_w;
  y_scale = paper_h / page_h;
  if (x_scale < y_scale)
    scale = x_scale;
  else
    scale = y_scale;

  cairo_matrix_init_identity (m);
  if (scale > 1.0) {
    // page is smaller than paper
    if (expand) {
      // expand to fit
      cairo_matrix_scale (m, scale, scale);
    } else if (!noCenter) {
      // centre page
      cairo_matrix_translate (m, (paper_w - page_w)/2, (paper_h - page_h)/2);
    } else {
      if (!svg) {
	// move to PostScript origin
	cairo_matrix_translate (m, 0, (paper_h - page_h));
      }
    }
  } else if (scale < 1.0)
    // page is larger than paper
    if (!noShrink) {
      // shrink to fit
      cairo_matrix_scale (m, scale, scale);
    }
}

static cairo_status_t writeStream(void *closure, const unsigned char *data, unsigned int length)
{
  FILE *file = (FILE *)closure;

  if (fwrite(data, length, 1, file) == 1)
    return CAIRO_STATUS_SUCCESS;
  else
    return CAIRO_STATUS_WRITE_ERROR;
}

static void beginDocument(GooString *outputFileName, double w, double h)
{
  if (printing) {
    if (outputFileName->cmp("fd://0") == 0)
      output_file = stdout;
    else
    {
      output_file = fopen(outputFileName->getCString(), "wb");
      if (!output_file) {
        fprintf(stderr, "Error opening output file %s\n", outputFileName->getCString());
        exit(2);
      }
    }

    if (ps || eps) {
#if CAIRO_HAS_PS_SURFACE
      surface = cairo_ps_surface_create_for_stream(writeStream, output_file, w, h);
      if (level2)
	cairo_ps_surface_restrict_to_level (surface, CAIRO_PS_LEVEL_2);
      if (eps)
	cairo_ps_surface_set_eps (surface, 1);
      if (duplex) {
	cairo_ps_surface_dsc_comment(surface, "%%Requirements: duplex");
	cairo_ps_surface_dsc_begin_setup(surface);
	cairo_ps_surface_dsc_comment(surface, "%%IncludeFeature: *Duplex DuplexNoTumble");
      }
      cairo_ps_surface_dsc_begin_page_setup (surface);
#endif
    } else if (pdf) {
#if CAIRO_HAS_PDF_SURFACE
      surface = cairo_pdf_surface_create_for_stream(writeStream, output_file, w, h);
#endif
    } else if (svg) {
#if CAIRO_HAS_SVG_SURFACE
      surface = cairo_svg_surface_create_for_stream(writeStream, output_file, w, h);
      cairo_svg_surface_restrict_to_version (surface, CAIRO_SVG_VERSION_1_2);
#endif
    }
  }
}

static void beginPage(double w, double h)
{
  if (printing) {
    if (ps || eps) {
#if CAIRO_HAS_PS_SURFACE
      if (w > h) {
	cairo_ps_surface_dsc_comment (surface, "%%PageOrientation: Landscape");
	cairo_ps_surface_set_size (surface, h, w);
      } else {
	cairo_ps_surface_dsc_comment (surface, "%%PageOrientation: Portrait");
	cairo_ps_surface_set_size (surface, w, h);
      }
#endif
    }

#if CAIRO_HAS_PDF_SURFACE
    if (pdf)
      cairo_pdf_surface_set_size (surface, w, h);
#endif

    cairo_surface_set_fallback_resolution (surface, x_resolution, y_resolution);

  } else {
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ceil(w), ceil(h));
  }
}

static void renderPage(PDFDoc *doc, CairoOutputDev *cairoOut, int pg,
		       double page_w, double page_h,
		       double output_w, double output_h)
{
  cairo_t *cr;
  cairo_status_t status;
  cairo_matrix_t m;

  cr = cairo_create(surface);
  cairoOut->setCairo(cr);
  cairoOut->setPrinting(printing);

  cairo_save(cr);
  if (ps && output_w > output_h) {
    // rotate 90 deg for landscape
    cairo_translate (cr, 0, output_w);
    cairo_matrix_init (&m, 0, -1, 1, 0, 0, 0);
    cairo_transform (cr, &m);
  }
  cairo_translate (cr, -crop_x, -crop_y);
  if (printing) {
    double cropped_w, cropped_h;
    getCropSize(page_w, page_h, &cropped_w, &cropped_h);
    getFitToPageTransform(cropped_w, cropped_h, output_w, output_h, &m);
    cairo_transform (cr, &m);
    cairo_rectangle(cr, crop_x, crop_y, cropped_w, cropped_h);
    cairo_clip(cr);
  } else {
    cairo_scale (cr, x_resolution/72.0, y_resolution/72.0);
  }
  doc->displayPageSlice(cairoOut,
			pg,
			72.0, 72.0,
			0, /* rotate */
			!useCropBox, /* useMediaBox */
			gFalse, /* Crop */
			printing,
			-1, -1, -1, -1);
  cairo_restore(cr);
  cairoOut->setCairo(NULL);

  // Blend onto white page
  if (!printing && !transp) {
    cairo_save(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    cairo_restore(cr);
  }

  status = cairo_status(cr);
  if (status)
      error(errInternal, -1, "cairo error: {0:s}\n", cairo_status_to_string(status));
  cairo_destroy (cr);
}

static void endPage(GooString *imageFileName)
{
  cairo_status_t status;

  if (printing) {
    cairo_surface_show_page(surface);
  } else {
    writePageImage(imageFileName);
    cairo_surface_finish(surface);
    status = cairo_surface_status(surface);
    if (status)
      error(errInternal, -1, "cairo error: {0:s}\n", cairo_status_to_string(status));
    cairo_surface_destroy(surface);
  }

}

static void endDocument()
{
  cairo_status_t status;

  if (printing) {
    cairo_surface_finish(surface);
    status = cairo_surface_status(surface);
    if (status)
      error(errInternal, -1, "cairo error: {0:s}\n", cairo_status_to_string(status));
    cairo_surface_destroy(surface);
    fclose(output_file);
  }
}

static GBool setPSPaperSize(char *size, int &psPaperWidth, int &psPaperHeight) {
  if (!strcmp(size, "match")) {
    psPaperWidth = psPaperHeight = -1;
  } else if (!strcmp(size, "letter")) {
    psPaperWidth = 612;
    psPaperHeight = 792;
  } else if (!strcmp(size, "legal")) {
    psPaperWidth = 612;
    psPaperHeight = 1008;
  } else if (!strcmp(size, "A4")) {
    psPaperWidth = 595;
    psPaperHeight = 842;
  } else if (!strcmp(size, "A3")) {
    psPaperWidth = 842;
    psPaperHeight = 1190;
  } else {
    return gFalse;
  }
  return gTrue;
}

static int numberOfCharacters(unsigned int n)
{
  int charNum = 0;
  while (n >= 10)
  {
    n = n / 10;
    charNum++;
  }
  charNum++;
  return charNum;
}

static GooString *getImageFileName(GooString *outputFileName, int numDigits, int page)
{
  char buf[10];
  GooString *imageName = new GooString(outputFileName);
  if (!singleFile) {
    snprintf(buf, sizeof(buf), "-%0*d", numDigits, page);
    imageName->appendf(buf);
  }
  if (png)
    imageName->append(".png");
  else if (jpeg)
    imageName->append(".jpg");
  else if (tiff)
    imageName->append(".tif");

  return imageName;
}

// If (printing || singleFile) the output file name includes the
// extension. Otherwise it is the file name base.
static GooString *getOutputFileName(GooString *fileName, GooString *outputName)
{
  GooString *name;
  char *s;
  char *p;

  if (outputName) {
    if (outputName->cmp("-") == 0) {
      if (!printing && !singleFile) {
	fprintf(stderr, "Error: stdout may only be used with the ps, eps, pdf, svg output options or if -singlefile is used.\n");
	exit(99);
      }
      return new GooString("fd://0");
    }
    return new GooString(outputName);
  }

  if (fileName->cmp("fd://0") == 0) {
    fprintf(stderr, "Error: an output filename or '-' must be supplied when the PDF file is stdin.\n");
    exit(99);
  }

  // be careful not to overwrite the input file when the output format is PDF
  if (pdf && fileName->cmpN("http://", 7) != 0 && fileName->cmpN("https://", 8) != 0) {
    fprintf(stderr, "Error: an output filename or '-' must be supplied when the output format is PDF and input PDF file is a local file.\n");
    exit(99);
  }

  // strip everything up to last '/'
  s = fileName->getCString();
  p = strrchr(s, '/');
  if (p) {
    p++;
    if (*p == 0) {
      fprintf(stderr, "Error: invalid output filename.\n");
      exit(99);
    }
    name = new GooString(p);
  } else {
    name = new GooString(s);
  }

  // remove .pdf extension
  p = strrchr(name->getCString(), '.');
  if (p && strcasecmp(p, ".pdf") == 0) {
    GooString *name2 = new GooString(name->getCString(), name->getLength() - 4);
    delete name;
    name = name2;
  }

  // append new extension
  if (ps)
    name->append(".ps");
  else if (eps)
    name->append(".eps");
  else if (pdf)
    name->append(".pdf");
  else if (svg)
    name->append(".svg");

  return name;
}

static void checkInvalidPrintOption(GBool option, const char *option_name)
{
  if (option) {
    fprintf(stderr, "Error: %s may only be used with the -png, -jpeg, or -tiff output options.\n", option_name);
    exit(99);
  }
}

static void checkInvalidImageOption(GBool option, const char *option_name)
{
  if (option) {
    fprintf(stderr, "Error: %s may only be used with the -ps, -eps, -pdf, or -svg output options.\n", option_name);
    exit(99);
  }
}

int main(int argc, char *argv[]) {
  PDFDoc *doc;
  GooString *fileName = NULL;
  GooString *outputName = NULL;
  GooString *outputFileName = NULL;
  GooString *imageFileName = NULL;
  GooString *ownerPW, *userPW;
  CairoOutputDev *cairoOut;
  int pg, pg_num_len;
  double pg_w, pg_h, tmp, output_w, output_h;
  int num_outputs;

  // parse args
  if (!parseArgs(argDesc, &argc, argv))
    exit(99);

  if ( resolution != 0.0 &&
       (x_resolution == 150.0 ||
        y_resolution == 150.0)) {
    x_resolution = resolution;
    y_resolution = resolution;
  }
  if (argc < 2 || argc > 3 || printVersion || printHelp) {
    fprintf(stderr, "pdftocairo version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "%s\n", popplerCopyright);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdftocairo", "<PDF-file> [<output-file>]", argDesc);
    }
    if (printVersion || printHelp)
      exit(0);
    else
      exit(99);
  }

  num_outputs = (png ? 1 : 0) +
                (jpeg ? 1 : 0) +
                (tiff ? 1 : 0) +
                (ps ? 1 : 0) +
                (eps ? 1 : 0) +
                (pdf ? 1 : 0) +
                (svg ? 1 : 0);
  if (num_outputs == 0) {
    fprintf(stderr, "Error: one of the output format options (-png, -jpeg, -ps, -eps, -pdf, -svg) must be used.\n");
    exit(99);
  }
  if (num_outputs > 1) {
    fprintf(stderr, "Error: use only one of the output format options (-png, -jpeg, -ps, -eps, -pdf, -svg).\n");
    exit(99);
  }
  if (png || jpeg || tiff)
    printing = gFalse;
  else
    printing = gTrue;

  if (printing) {
    checkInvalidPrintOption(mono, "-mono");
    checkInvalidPrintOption(gray, "-gray");
    checkInvalidPrintOption(transp, "-transp");
    checkInvalidPrintOption(icc.getCString()[0], "-icc");
    checkInvalidPrintOption(singleFile, "-singlefile");
  } else {
    checkInvalidImageOption(level2, "-level2");
    checkInvalidImageOption(level3, "-level3");
    checkInvalidImageOption(doOrigPageSizes, "-origpagesizes");
    checkInvalidImageOption(paperSize[0], "-paper");
    checkInvalidImageOption(paperWidth > 0, "-paperw");
    checkInvalidImageOption(paperHeight > 0, "-paperh");
    checkInvalidImageOption(noCrop, "-nocrop");
    checkInvalidImageOption(expand, "-expand");
    checkInvalidImageOption(noShrink, "-noshrink");
    checkInvalidImageOption(noCenter, "-nocenter");
    checkInvalidImageOption(duplex, "-duplex");
  }

  if (icc.getCString()[0] && !png) {
    fprintf(stderr, "Error: -icc may only be used with png output.\n");
    exit(99);
  }

  if (transp && !(png || tiff)) {
    fprintf(stderr, "Error: -transp may only be used with png or tiff output.\n");
    exit(99);
  }

  if (mono && gray) {
    fprintf(stderr, "Error: -mono and -gray may not be used together.\n");
    exit(99);
  }

  if (mono && !(png || tiff)) {
    fprintf(stderr, "Error: -mono may only be used with png or tiff output.\n");
    exit(99);
  }

  if (strlen(tiffCompressionStr) > 0 && !tiff) {
    fprintf(stderr, "Error: -tiffcompression may only be used with tiff output.\n");
    exit(99);
  }

  if (level2 && level3) {
    fprintf(stderr, "Error: use only one of the 'level' options.\n");
    exit(99);
  }
  if (!level2 && !level3)
    level3 = gTrue;

  if (eps && (doOrigPageSizes || paperSize[0] || paperWidth > 0 || paperHeight > 0)) {
    fprintf(stderr, "Error: page size options may not be used with eps output.\n");
    exit(99);
  }

  if (paperSize[0]) {
    if (!setPSPaperSize(paperSize, paperWidth, paperHeight)) {
      fprintf(stderr, "Invalid paper size\n");
      exit(99);
    }
  }

  globalParams = new GlobalParams();
  if (quiet) {
    globalParams->setErrQuiet(quiet);
  }

  // open PDF file
  if (ownerPassword[0]) {
    ownerPW = new GooString(ownerPassword);
  } else {
    ownerPW = NULL;
  }
  if (userPassword[0]) {
    userPW = new GooString(userPassword);
  } else {
    userPW = NULL;
  }

  fileName = new GooString(argv[1]);
  if (fileName->cmp("-") == 0) {
    delete fileName;
    fileName = new GooString("fd://0");
  }
  if (argc == 3)
    outputName = new GooString(argv[2]);
  else
    outputName = NULL;

  outputFileName = getOutputFileName(fileName, outputName);

#if USE_CMS
  icc_data = NULL;
  if (icc.getCString()[0]) {
    FILE *file = fopen(icc.getCString(), "rb");
    if (!file) {
      fprintf(stderr, "Error: unable to open icc profile %s\n", icc.getCString());
      exit(4);
    }
    fseek (file, 0, SEEK_END);
    icc_data_size = ftell(file);
    fseek (file, 0, SEEK_SET);
    icc_data = (unsigned char*)gmalloc(icc_data_size);
    if (fread(icc_data, icc_data_size, 1, file) != 1) {
      fprintf(stderr, "Error: unable to read icc profile %s\n", icc.getCString());
      exit(4);
    }
    fclose(file);
    profile = cmsOpenProfileFromMem(icc_data, icc_data_size);
    if (!profile) {
      fprintf(stderr, "Error: lcms error opening profile\n");
      exit(4);
    }
  } else {
    profile = cmsCreate_sRGBProfile();
  }
  GfxColorSpace::setDisplayProfile(profile);
#endif

  doc = PDFDocFactory().createPDFDoc(*fileName, ownerPW, userPW);
  if (!doc->isOk()) {
    fprintf(stderr, "Error opening PDF file.\n");
    exit(1);
  }

#ifdef ENFORCE_PERMISSIONS
  // check for print permission
  if (printing && !doc->okToPrint()) {
    fprintf(stderr, "Printing this document is not allowed.\n");
    exit(3);
  }
#endif

  // get page range
  if (firstPage < 1)
    firstPage = 1;
  if (singleFile && lastPage < 1)
    lastPage = firstPage;
  if (lastPage < 1 || lastPage > doc->getNumPages())
    lastPage = doc->getNumPages();

  if (eps && firstPage != lastPage) {
    fprintf(stderr, "EPS files can only contain one page.\n");
    exit(99);
  }

  if (singleFile && firstPage < lastPage) {
    if (!quiet) {
      fprintf(stderr,
        "Warning: Single file will write only the first of the %d pages.\n",
        lastPage + 1 - firstPage);
    }
    lastPage = firstPage;
  }

  // Make sure firstPage is always used so that beginDocument() is called
  if ((printOnlyEven && firstPage % 2 == 0) || (printOnlyOdd && firstPage % 2 == 1))
    firstPage++;

  cairoOut = new CairoOutputDev();
  cairoOut->startDoc(doc);
  if (sz != 0)
    crop_w = crop_h = sz;
  pg_num_len = numberOfCharacters(doc->getNumPages());
  for (pg = firstPage; pg <= lastPage; ++pg) {
    if (printOnlyEven && pg % 2 == 0) continue;
    if (printOnlyOdd && pg % 2 == 1) continue;
    if (useCropBox) {
      pg_w = doc->getPageCropWidth(pg);
      pg_h = doc->getPageCropHeight(pg);
    } else {
      pg_w = doc->getPageMediaWidth(pg);
      pg_h = doc->getPageMediaHeight(pg);
    }

    if (printing && pg == firstPage) {
      if (paperWidth < 0 || paperHeight < 0) {
	paperWidth = (int)ceil(pg_w);
	paperHeight = (int)ceil(pg_h);
      }
    }

    if (scaleTo != 0) {
      resolution = (72.0 * scaleTo) / (pg_w > pg_h ? pg_w : pg_h);
      x_resolution = y_resolution = resolution;
    } else {
      if (x_scaleTo > 0) {
        x_resolution = (72.0 * x_scaleTo) / pg_w;
        if (y_scaleTo == -1)
          y_resolution = x_resolution;
      }
      if (y_scaleTo > 0) {
        y_resolution = (72.0 * y_scaleTo) / pg_h;
        if (x_scaleTo == -1)
          x_resolution = y_resolution;
      }
    }
    if ((doc->getPageRotate(pg) == 90) || (doc->getPageRotate(pg) == 270)) {
      tmp = pg_w;
      pg_w = pg_h;
      pg_h = tmp;
    }
    if (imageFileName) {
      delete imageFileName;
      imageFileName = NULL;
    }
    if (!printing)
      imageFileName = getImageFileName(outputFileName, pg_num_len, pg);
    getOutputSize(pg_w, pg_h, &output_w, &output_h);

    if (pg == firstPage)
      beginDocument(outputFileName, output_w, output_h);
    beginPage(output_w, output_h);
    renderPage(doc, cairoOut, pg, pg_w, pg_h, output_w, output_h);
    endPage(imageFileName);
  }
  endDocument();

  // clean up
  delete cairoOut;
  delete doc;
  delete globalParams;
  if (fileName)
    delete fileName;
  if (outputName)
    delete outputName;
  if (outputFileName)
    delete outputFileName;
  if (imageFileName)
    delete imageFileName;
  if (ownerPW)
    delete ownerPW;
  if (userPW)
    delete ownerPW;

#if USE_CMS
  cmsCloseProfile(profile);
  if (icc_data)
    gfree(icc_data);
#endif

  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return 0;
}
