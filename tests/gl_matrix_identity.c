/*
THIS WORK, INCLUDING THE SOURCE CODE, DOCUMENTATION
AND RELATED MEDIA AND DATA, IS PLACED INTO THE PUBLIC DOMAIN.

THE ORIGINAL AUTHOR IS KYLE FOLEY.

THIS SOFTWARE IS PROVIDED AS-IS WITHOUT WARRANTY
OF ANY KIND, NOT EVEN THE IMPLIED WARRANTY OF
MERCHANTABILITY. THE AUTHOR OF THIS SOFTWARE,
ASSUMES _NO_ RESPONSIBILITY FOR ANY CONSEQUENCE
RESULTING FROM THE USE, MODIFICATION, OR
REDISTRIBUTION OF THIS SOFTWARE.
*/

#ifndef __EMSCRIPTEN__
#define USE_GLEW 1
#endif

#if USE_GLEW
#include "GL/glew.h"
#endif

#include "SDL/SDL.h"
#if !USE_GLEW
#include "SDL/SDL_opengl.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void verify() {
  int width = 640, height = 480;
  unsigned char *data = (unsigned char*)malloc(width*height*4);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
  int sum = 0;
  for (int x = 0; x < width*height*4; x++) {
    if (x % 4 != 3) sum += x * data[x];
  }
#ifdef __EMSCRIPTEN__
  int result = sum;
  REPORT_RESULT();
#endif
}

int main(int argc, char *argv[])
{
    SDL_Surface *screen;
    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    screen = SDL_SetVideoMode( 640, 480, 24, SDL_OPENGL );
    if ( !screen ) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }

    // Create a texture

    GLuint texture;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    GLubyte textureData[]  = { 0x7f,    0,    0,
                                  0, 0xff,    0,
                               0x7f,    0,    0,
                                  0, 0xff,    0};
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0,
                  GL_RGB, GL_UNSIGNED_BYTE, textureData );


    // BEGIN

#if USE_GLEW
    glewInit();
#endif

    glClearColor( 0, 0, 0.5, 1.0 );
    glClear( GL_COLOR_BUFFER_BIT );

    glColor4f(0.8, 0.8, 0.8, 1);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    const int kRowSize = 20;
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    float fbuf[] = {0, 1, 0, 0, 1,
                    1, 1, 0, 1, 1,
                    1, 0, 0, 1, 0,
                    0, 1, 0, 0, 1,
                    1, 0, 0, 1, 0,
                    0, 0, 0, 0, 0};
    glBufferData(GL_ARRAY_BUFFER, sizeof(fbuf) * sizeof(float), fbuf, GL_STATIC_DRAW);

    glTexCoordPointer(2, GL_FLOAT, kRowSize, (GLvoid*)(3*4));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, kRowSize, 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    // END

    SDL_GL_SwapBuffers();

    verify();

#ifndef __EMSCRIPTEN__
    SDL_Delay(1500);
#endif

    SDL_Quit();

    return 0;
}
