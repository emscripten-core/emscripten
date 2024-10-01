/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

void printError(FT_Error error) {
  printf("FT_Error (%d) : %s\n", FT_Errors[error].code, FT_Errors[error].message);
}

int WIDTH = 0;
int HEIGHT = 0;

/* origin is the upper left corner */
unsigned char *image;

void
draw_bitmap( FT_Bitmap*  bitmap,
             FT_Int      x,
             FT_Int      y)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;


  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
      if ( i < 0      || j < 0       ||
           i >= WIDTH || j >= HEIGHT )
        continue;

      image[j*WIDTH + i] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
}


void
show_image( void )
{
  int  i, j;
  int count = 0;

  for ( i = 0; i < HEIGHT; i++ )
  {
    for ( j = 0; j < WIDTH; j++ ) {
      if (image[i*WIDTH + j]) count++;
      putchar( image[i*WIDTH + j] == 0 ? ' '
                                : image[i*WIDTH + j] < 128 ? '+'
                                                           : '*' );
    }
    putchar( '\n' );
  }
}


int
main( int     argc,
      char**  argv )
{
  FT_Library    library;
  FT_Face       face;

  FT_GlyphSlot  slot;
  FT_Vector     pen;
  FT_Error      error;

  char*         filename;
  char*         text;

  double        angle;
  int           target_height;
  int           n, num_chars;

  filename      = "LiberationSansBold.ttf";
  text          = "w";
  num_chars     = strlen( text );
  WIDTH         = 16;
  HEIGHT        = 16;
  target_height = HEIGHT;

  image = (unsigned char*)malloc(WIDTH*HEIGHT);
  for (int x = 0; x < WIDTH; x++)
    for (int y = 0; y < HEIGHT; y++)
      image[y*WIDTH + x] = 0;

  error = FT_Init_FreeType( &library ); /* initialize library */
  if (error) printError(error);

  error = FT_New_Face( library, filename, 0, &face ); /* create face object */
  if (error) printError(error);

  error = FT_Set_Char_Size( face, 16 * 64, 0, 100, 0 ); /* set character size */
  if (error) printError(error);

  slot = face->glyph;

  pen.x = 0;
  pen.y = 0;

  for ( n = 0; n < num_chars; n++ )
  {

    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Char( face, text[n], FT_LOAD_RENDER );
    if ( error )
      continue;                 /* ignore errors */

    /* now, draw to our target surface (convert position) */
    draw_bitmap( &slot->bitmap,
                 slot->bitmap_left,
                 target_height - slot->bitmap_top );

    /* increment pen position */
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;
  }

  show_image();

  FT_Done_Face    ( face );
  FT_Done_FreeType( library );

  return 0;
}
