/* -*- Mode: c; c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t; -*- */
/*
 * Copyright © 2009 Mozilla Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Mozilla Corporation not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Mozilla Corporation makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * MOZILLA CORPORATION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
 * SHALL MOZILLA CORPORATION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * Author: Jeff Muizelaar, Mozilla Corp.
 */

/* This implements a box filter that supports non-integer box sizes */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "goo/gmem.h"
#include "CairoRescaleBox.h"

typedef unsigned short int      uint16_t;
typedef unsigned int            uint32_t;

/* we work in fixed point where 1. == 1 << 24 */
#define FIXED_SHIFT 24

static void downsample_row_box_filter (
        int start, int width,
        uint32_t *src, uint32_t *dest,
        int coverage[], int pixel_coverage)
{
    /* we need an array of the pixel contribution of each destination pixel on the boundaries.
     * we invert the value to get the value on the other size of the box */
    /*

       value  = a * contribution * 1/box_size
       value += a * 1/box_size
       value += a * 1/box_size
       value += a * 1/box_size
       value += a * (1 - contribution) * 1/box_size
                a * (1/box_size - contribution * 1/box_size)

        box size is constant


       value = a * contribtion_a * 1/box_size + b * contribution_b * 1/box_size
               contribution_b = (1 - contribution_a)
                              = (1 - contribution_a_next)
    */

    /* box size = ceil(src_width/dest_width) */
    int x = 0;

    /* skip to start */
    /* XXX: it might be possible to do this directly instead of iteratively, however
     * the iterative solution is simple */
    while (x < start)
    {
        int box = 1 << FIXED_SHIFT;
        int start_coverage = coverage[x];
        box -= start_coverage;
        src++;
        while (box >= pixel_coverage)
        {
            src++;
            box -= pixel_coverage;
        }
        x++;
    }

    while (x < start + width)
    {
        uint32_t a = 0;
        uint32_t r = 0;
        uint32_t g = 0;
        uint32_t b = 0;
        int box = 1 << FIXED_SHIFT;
        int start_coverage = coverage[x];

        a = ((*src >> 24) & 0xff) * start_coverage;
        r = ((*src >> 16) & 0xff) * start_coverage;
        g = ((*src >>  8) & 0xff) * start_coverage;
        b = ((*src >>  0) & 0xff) * start_coverage;
        src++;
        x++;
        box -= start_coverage;

        while (box >= pixel_coverage)
        {
            a += ((*src >> 24) & 0xff) * pixel_coverage;
            r += ((*src >> 16) & 0xff) * pixel_coverage;
            g += ((*src >>  8) & 0xff) * pixel_coverage;
            b += ((*src >>  0) & 0xff) * pixel_coverage;
            src++;

            box -= pixel_coverage;
        }

        /* multiply by whatever is leftover
         * this ensures that we don't bias down.
         * i.e. start_coverage + n*pixel_coverage + box == 1 << 24 */
        if (box > 0)
        {
            a += ((*src >> 24) & 0xff) * box;
            r += ((*src >> 16) & 0xff) * box;
            g += ((*src >>  8) & 0xff) * box;
            b += ((*src >>  0) & 0xff) * box;
        }

        a >>= FIXED_SHIFT;
        r >>= FIXED_SHIFT;
        g >>= FIXED_SHIFT;
        b >>= FIXED_SHIFT;

        *dest = (a << 24) | (r << 16) | (g << 8) | b;
        dest++;
    }
}

static void downsample_columns_box_filter (
        int n,
        int start_coverage,
        int pixel_coverage,
        uint32_t *src, uint32_t *dest)
{
    int stride = n;
    while (n--) {
        uint32_t a = 0;
        uint32_t r = 0;
        uint32_t g = 0;
        uint32_t b = 0;
        uint32_t *column_src = src;
        int box = 1 << FIXED_SHIFT;

        a = ((*column_src >> 24) & 0xff) * start_coverage;
        r = ((*column_src >> 16) & 0xff) * start_coverage;
        g = ((*column_src >>  8) & 0xff) * start_coverage;
        b = ((*column_src >>  0) & 0xff) * start_coverage;
        column_src += stride;
        box -= start_coverage;

        while (box >= pixel_coverage)
        {
            a += ((*column_src >> 24) & 0xff) * pixel_coverage;
            r += ((*column_src >> 16) & 0xff) * pixel_coverage;
            g += ((*column_src >>  8) & 0xff) * pixel_coverage;
            b += ((*column_src >>  0) & 0xff) * pixel_coverage;
            column_src += stride;
            box -= pixel_coverage;
        }

        if (box > 0) {
            a += ((*column_src >> 24) & 0xff) * box;
            r += ((*column_src >> 16) & 0xff) * box;
            g += ((*column_src >>  8) & 0xff) * box;
            b += ((*column_src >>  0) & 0xff) * box;
        }

        a >>= FIXED_SHIFT;
        r >>= FIXED_SHIFT;
        g >>= FIXED_SHIFT;
        b >>= FIXED_SHIFT;

        *dest = (a << 24) | (r << 16) | (g << 8) | b;
        dest++;
        src++;
    }
}

static int compute_coverage (int coverage[], int src_length, int dest_length)
{
    int i;
    /* num = src_length/dest_length
       total = sum(pixel) / num

       pixel * 1/num == pixel * dest_length / src_length
    */
    /* the average contribution of each source pixel */
    int ratio = ((1 << 24)*(long long int)dest_length)/src_length;
    /* because ((1 << 24)*(long long int)dest_length) won't always be divisible by src_length
     * we'll need someplace to put the other bits.
     *
     * We want to ensure a + n*ratio < 1<<24
     *
     * 1<<24
     * */

    double scale = (double)src_length/dest_length;

    /* for each destination pixel compute the coverage of the left most pixel included in the box */
    /* I have a proof of this, which this margin is too narrow to contain */
    for (i=0; i<dest_length; i++)
    {
        float left_side = i*scale;
        float right_side = (i+1)*scale;
        float right_fract = right_side - floor (right_side);
        float left_fract = ceil (left_side) - left_side;
        int overage;
        /* find out how many source pixels will be used to fill the box */
        int count = floor (right_side) - ceil (left_side);
        /* what's the maximum value this expression can become?
           floor((i+1)*scale) - ceil(i*scale)

           (i+1)*scale - i*scale == scale

           since floor((i+1)*scale) <= (i+1)*scale
           and   ceil(i*scale)      >= i*scale

           floor((i+1)*scale) - ceil(i*scale) <= scale

           further since: floor((i+1)*scale) - ceil(i*scale) is an integer

           therefore:
           floor((i+1)*scale) - ceil(i*scale) <= floor(scale)
        */

        if (left_fract == 0.)
            count--;

        /* compute how much the right-most pixel contributes */
        overage = ratio*(right_fract);

        /* the remainder is the the amount that the left-most pixel
         * contributes */
        coverage[i] = (1<<24) - (count * ratio + overage);
    }

    return ratio;
}

GBool downscale_box_filter(uint32_t *orig, int orig_stride, unsigned orig_width, unsigned orig_height,
			   signed scaled_width, signed scaled_height,
			   uint16_t start_column, uint16_t start_row,
			   uint16_t width, uint16_t height,
			   uint32_t *dest, int dst_stride)
{
    int pixel_coverage_x, pixel_coverage_y;
    int dest_y;
    int src_y = 0;
    uint32_t *scanline = orig;
    int *x_coverage = NULL;
    int *y_coverage = NULL;
    uint32_t *temp_buf = NULL;
    GBool retval = gFalse;

    x_coverage = (int *)gmallocn3 (orig_width, 1, sizeof(int));
    y_coverage = (int *)gmallocn3 (orig_height, 1, sizeof(int));

    /* we need to allocate enough room for ceil(src_height/dest_height)+1
      Example:
      src_height = 140
      dest_height = 50
      src_height/dest_height = 2.8

      |-------------|       2.8 pixels
      |----|----|----|----| 4 pixels
      need to sample 3 pixels

         |-------------|       2.8 pixels
      |----|----|----|----| 4 pixels
      need to sample 4 pixels
    */

    temp_buf = (uint32_t *)gmallocn3 ((orig_height + scaled_height-1)/scaled_height+1, scaled_width, sizeof(uint32_t));

    if (!x_coverage || !y_coverage || !scanline || !temp_buf)
	goto cleanup;

    pixel_coverage_x = compute_coverage (x_coverage, orig_width, scaled_width);
    pixel_coverage_y = compute_coverage (y_coverage, orig_height, scaled_height);

    assert (width + start_column <= scaled_width);

    /* skip the rows at the beginning */
    for (dest_y = 0; dest_y < start_row; dest_y++)
    {
        int box = 1 << FIXED_SHIFT;
        int start_coverage_y = y_coverage[dest_y];
        box -= start_coverage_y;
        src_y++;
        while (box >= pixel_coverage_y)
        {
            box -= pixel_coverage_y;
            src_y++;
        }
    }

    for (; dest_y < start_row + height; dest_y++)
    {
        int columns = 0;
        int box = 1 << FIXED_SHIFT;
        int start_coverage_y = y_coverage[dest_y];

	scanline = orig + src_y * orig_stride / 4;
        downsample_row_box_filter (start_column, width, scanline, temp_buf + width * columns, x_coverage, pixel_coverage_x);
        columns++;
        src_y++;
        box -= start_coverage_y;

        while (box >= pixel_coverage_y)
        {
	    scanline = orig + src_y * orig_stride / 4;
            downsample_row_box_filter (start_column, width, scanline, temp_buf + width * columns, x_coverage, pixel_coverage_x);
            columns++;
            src_y++;
            box -= pixel_coverage_y;
        }

        /* downsample any leftovers */
        if (box > 0)
        {
	    scanline = orig + src_y * orig_stride / 4;
            downsample_row_box_filter (start_column, width, scanline, temp_buf + width * columns, x_coverage, pixel_coverage_x);
            columns++;
        }

        /* now scale the rows we just downsampled in the y direction */
        downsample_columns_box_filter (width, start_coverage_y, pixel_coverage_y, temp_buf, dest);
        dest += dst_stride / 4;

//        assert(width*columns <= ((orig_height + scaled_height-1)/scaled_height+1) * width);
    }
//    assert (src_y<=orig_height);

    retval = gTrue;

cleanup:
    free (x_coverage);
    free (y_coverage);
    free (temp_buf);

    return gTrue;
}
