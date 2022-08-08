/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

/* example1.c                                                      */
/*                                                                 */
/* This small program shows how to print a rotated string with the */
/* FreeType 2 library.                                             */


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

int WIDTH = 0;
int HEIGHT = 0;

/* origin is the upper left corner */
unsigned char *image;


/* Replace this function with something useful. */

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
    for ( j = 0; j < WIDTH; j++ ) 
	{
      if (image[i*WIDTH + j]) count++;
      putchar(image[i*WIDTH + j] == 0? ' '
                                : image[i*WIDTH + j] < 128 ? '+'
                                                           : '*' );
    }
    putchar( '\n' );
  }
  printf("Non-0s: %d\n", count);
}


int
main( int     argc,
      char**  argv )
{
  FT_Library    library;
  FT_Face       face;

  FT_GlyphSlot  slot;
  FT_Error      error;

  FT_UInt glyphIndex;

  char*         filename;
  char*         text;

  int           target_height;

  if ( argc != 6 )
  {
    fprintf ( stderr, "usage: %s font sample-text width height angle\n", argv[0] );
    exit( 1 );
  }

  // Only test the character 'w'
  text = (char*)"w";
  
  filename      = argv[1];                           /* first argument     */
  WIDTH         = atoi(argv[3]);
  HEIGHT        = atoi(argv[4]);
  target_height = HEIGHT;
	
  image = (unsigned char*)malloc(WIDTH*HEIGHT);
  for (int x = 0; x < WIDTH; x++)
    for (int y = 0; y < HEIGHT; y++)
      image[y*WIDTH + x] = 0;

  error = FT_Init_FreeType( &library );              /* initialize library */
  if (error) printf("Init Error! %d\n", error);

  error = FT_New_Face( library, argv[1], 0, &face ); /* create face object */
  if (error) printf("New_Face Error! %d\n", error);

  /* use 50pt at 100dpi */
  error = FT_Set_Char_Size( face, 0, 32 * 64, 0, 0 );                /* set character size */
  if (error) printf("Set_Cshar_Size Error! %d\n", error);
  	
  slot = face->glyph;

  glyphIndex = FT_Get_Char_Index(face, text[0]);
	
  /* load glyph image into the slot (erase previous one) */
  error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_BITMAP);
  if(error) printf("FT_Load_Glyph Error! %d\n", error);
  	
  error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
  if(error) printf("FT_Render_Glyph Error! %d\n", error);
  	
  /* now, draw to our target surface (convert position) */
  draw_bitmap(&slot->bitmap, slot->bitmap_left, target_height - slot->bitmap_top);
  	
  show_image();
  	
  FT_Done_Face(face);
  FT_Done_FreeType(library);
  return 0;
}

/* EOF */
