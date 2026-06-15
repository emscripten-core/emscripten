/*******************************************************************
 *                                                                 *
 *                        Using SDL With OpenGL                    *
 *                                                                 *
 *                    Tutorial by Kyle Foley (sdw)                 *
 *                                                                 *
 * http://gpwiki.org/index.php/SDL:Tutorials:Using_SDL_with_OpenGL *
 *                                                                 *
 *******************************************************************/

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
#include "SDL/SDL_image.h"
#if !USE_GLEW
#include "SDL/SDL_opengl.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void shaders() {
#if USE_GLEW
  glewInit();
#endif

  GLint ok;

  const char *vertexShader = "void main(void) \n"
                             "{ \n"
                             "    gl_Position = ftransform(); \n"
                             "    gl_TexCoord[0] = gl_MultiTexCoord0; \n"
                             "    gl_FrontColor = gl_Color; \n"
                             "} \n";
  const char *fragmentShader = "uniform sampler2D tex0; \n"
                               "void main(void) \n"
                               "{ \n"
                               "    gl_FragColor = gl_Color * texture2D(tex0, gl_TexCoord[0].xy); \n"
                               "} \n";

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertexShader, NULL);
  glCompileShader(vs);
  glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
  assert(ok);

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragmentShader, NULL);
  glCompileShader(fs);
  glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
  assert(ok);

  GLuint program = glCreateProgram();

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &ok);
  assert(ok);

  glUseProgram(program);

  {
    // Also, check getting the error log
    const char *fakeVertexShader = "atbute ve4 blarg; ### AAA\n";
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &fakeVertexShader, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    assert(!ok);
    GLint infoLen = 0;
    glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLen);
    assert(infoLen > 1);
  }
}

int main(int argc, char *argv[])
{
    SDL_Surface *screen;

    // Slightly different SDL initialization
    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); // *new*

    screen = SDL_SetVideoMode( 640, 480, 16, SDL_OPENGL ); // *changed*
    if ( !screen ) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }
    
    // Set the OpenGL state after creating the context with SDL_SetVideoMode

    glClearColor( 0, 0, 0, 0 );
    
#ifndef __EMSCRIPTEN__
    glEnable( GL_TEXTURE_2D ); // Need this to display a texture XXX unnecessary in OpenGL ES 2.0/WebGL
#endif

    glViewport( 0, 0, 640, 480 );

    glMatrixMode( GL_PROJECTION );
    GLfloat matrixData[] = { 2.0/640,        0,  0,  0,
                                   0, -2.0/480,  0,  0,
                                   0,        0, -1,  0,
                                  -1,        1,  0,  1 };
    glLoadMatrixf(matrixData); // test loadmatrix

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    
    // Load the OpenGL texture

    GLuint texture; // Texture object handle
  
    // Have OpenGL generate a texture object handle for us
    glGenTextures( 1, &texture );

    // Bind the texture object
    glBindTexture( GL_TEXTURE_2D, texture );
    
    // Set the texture's stretching properties
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    int w = 256, h = 256;
    GLushort *pixels = (GLushort*)malloc(w*h*2);
    for (int x = 0; x < w; x++)
      for (int y = 0; y < h; y++)
        pixels[w*y + x] = ((x*32)/w) | ((((w-x)*(h-y)*64)/(w*h)) << 5) | (((y*32)/h) << 11);

    // Edit the texture object's image data using the information SDL_Surface gives us
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, 
                  GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixels );
    
    // Clear the screen before drawing
    glClear( GL_COLOR_BUFFER_BIT );
    
    shaders();

    // Bind the texture to which subsequent calls refer to
    glBindTexture( GL_TEXTURE_2D, texture );

    // Use clientside vertex pointers to render two items
    GLfloat vertexData[] = { 0, 0, 10, 10, // texture2, position2
                             1, 0, 300, 10,
                             1, 1, 300, 128,
                             0, 1, 10, 128,
                             0, 0.5, 410, 10,
                             1, 0.5, 600, 10,
                             1, 1, 630, 200,
                             0.5, 1, 310, 250,
                             0, 0, 100, 300,
                             1, 0, 300, 300,
                             1, 1, 300, 400,
                             0, 1, 100, 400 };

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 4*4, &vertexData[0]);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 4*4, &vertexData[2]);

    glDrawArrays(GL_QUADS, 0, 12);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    SDL_GL_SwapBuffers();
    
#ifndef __EMSCRIPTEN__
    // Wait for 3 seconds to give us a chance to see the image
    SDL_Delay(3000);
#endif

    // Now we can delete the OpenGL texture and close down SDL
    glDeleteTextures( 1, &texture );
    
    SDL_Quit();
    
    return 0;
}

