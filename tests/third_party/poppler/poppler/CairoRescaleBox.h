#ifndef CAIRO_RESCALE_BOX_H
#define CAIRO_RESCALE_BOX_H

#include "goo/gtypes.h"

GBool downscale_box_filter(unsigned int *orig, int orig_stride, unsigned orig_width, unsigned orig_height,
			   signed scaled_width, signed scaled_height,
			   unsigned short int start_column, unsigned short int start_row,
			   unsigned short int width, unsigned short int height,
			   unsigned int *dest, int dst_stride);

#endif /* CAIRO_RESCALE_BOX_H */
