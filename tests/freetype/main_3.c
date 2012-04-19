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

unsigned char pixelData[32 * 32];

/* Replace this function with something useful. */
void
draw_bitmap( FT_Bitmap*  bitmap,
             FT_Int      x,
             FT_Int      y)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;
  int xbyte;
  int xbit;
  unsigned char* pcur;

  unsigned char* src = bitmap->buffer;
  unsigned char* dest = pixelData;

  // Note: FT_RENDER_MONO_MODE render characater's one pixel by a single bit,
  // translate the single bit to a single char for displaying image.
  for(int _y = 0; _y < bitmap->rows; ++_y)
  {
	  for(int _x = 0; _x < bitmap->width; ++_x)
	  {	
		  xbyte = _x / 8;
		  xbit = _x - xbyte * 8;
		  pcur = dest + _x;

		  // test if the pixel bit be set
		  if(src[xbyte] & (0x80 >> xbit))
		  {
			  *pcur = 0xFF;
		  }
		  else
		  {
			  *pcur = 0;
		  }
	  }
	  src += bitmap->pitch;
	  dest += bitmap->width;
  }

  // display the character to ref txt file
  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
      if ( i < 0      || j < 0       ||
           i >= WIDTH || j >= HEIGHT )
        continue;
		
      image[j*WIDTH + i] |= pixelData[q * bitmap->width + p];
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
                                                           : '*');
    }
    putchar('\n');
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
  FT_Vector     pen;                    /* untransformed origin  */
  
  char*         filename;
  char*         text;

  double        angle;
  int           target_height;
  int           n, num_chars;
  FT_UInt		glyphIndex;

  if ( argc != 6 )
  {
    fprintf ( stderr, "usage: %s font sample-text width height angle\n", argv[0] );
    exit( 1 );
  }

  filename      = argv[1];                           /* first argument     */
  text          = argv[2];          /* second argument    */
  num_chars     = strlen( text );
  WIDTH         = atoi(argv[3]);
  HEIGHT        = atoi(argv[4]);
  angle         = ( ((float)atoi(argv[5])) / 360 ) * 3.14159 * 2;      /* use 25 degrees     */
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
  error = FT_Set_Char_Size( face, 32 * 64, 0,
                            0, 0 );                /* set character size */
  if (error) printf("Set_Char_Size Error! %d\n", error);

  slot = face->glyph;
  pen.x = 0;
  pen.y = 0;
  for ( n = 0; n < num_chars; n++ )
  {
	  /* set transformation */
	  FT_Set_Transform( face, 0, &pen );

	  /* load glyph image into the slot (erase previous one) */
	  glyphIndex = FT_Get_Char_Index(face, text[n]);
	  
	  /* load glyph image into the slot (erase previous one) */
	  error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
	  if(error) printf("FT_Load_Glyph Error! %d\n", error);
	  
	  error = FT_Render_Glyph(slot, FT_RENDER_MODE_MONO);
	  if(error) printf("FT_Render_Glyph Error! %d\n", error);

	  /* now, draw to our target surface (convert position) */
	  draw_bitmap(&slot->bitmap,
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

/* EOF */
