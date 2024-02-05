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

#include "SDL.h"
#include "SDL_opengl.h"

#include <stdio.h>
#include <string.h>

#define WIDTH 640
#define HEIGHT 480

int main(int argc, char* argv[]) {
  SDL_Window* window;
  SDL_GLContext context;

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  window = SDL_CreateWindow(
    "OpenGL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
  if (!window) {
    fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
    return 0;
  }

  context = SDL_GL_CreateContext(window);
  if (!context) {
    fprintf(stderr, "Couldn't create context: %s\n", SDL_GetError());
    return 0;
  }

  glMatrixMode(GL_PROJECTION);
  glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);

  glClearColor(0, 0, 0, 1);

  glEnable(GL_TEXTURE_2D);

  GLuint texture; // Texture object handle

  // Have OpenGL generate a texture object handle for us
  glGenTextures(1, &texture);

  // Bind the texture object
  glBindTexture(GL_TEXTURE_2D, texture);

  // Set the texture's stretching properties
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  GLubyte texture_data[] = {
    0xffu, 0x00u, 0x00u, 0xffu, // 0.0 0.0
    0x00u, 0xffu, 0x00u, 0xffu, // 1.0 0.0
    0x00u, 0x00u, 0xffu, 0xffu, // 0.0 1.0
    0xffu, 0xffu, 0x00u, 0xffu  // 1.0 1.0
  };

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

  // Clear the screen before drawing
  glClear(GL_COLOR_BUFFER_BIT);

  // Bind the texture to which subsequent calls refer to
  glBindTexture(GL_TEXTURE_2D, texture);

  const GLenum modes[8] = {
    GL_POINTS, GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP,
    GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS
  };

  const GLfloat vertex_data[4][4] = {
    { 0.25f, 0.75f, 0.0f, 0.0f },
    { 0.25f, 0.25f, 1.0f, 0.0f },
    { 0.75f, 0.75f, 0.0f, 1.0f },
    { 0.75f, 0.25f, 1.0f, 1.0f }
  };

  const GLushort indices[] = {
    // GL_POINTS, GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP, GL_TRIANGLE_STRIP
    0u, 1u, 2u, 3u,
    // GL_TRIANGLES
    0u, 1u, 2u, 2u, 1u, 3u,
    // GL_TRIANGLE_FAN, GL_QUADS
    0u, 1u, 3u, 2u,
  };

  glVertexPointer(2, GL_FLOAT, sizeof (vertex_data[0]), &vertex_data[0][0]);
  glTexCoordPointer(2, GL_FLOAT, sizeof (vertex_data[0]), &vertex_data[0][2]);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  #define ROWS 2
  #define COLS 4
  #define PORT_W (WIDTH / COLS)
  #define PORT_H (HEIGHT / ROWS)

  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      GLenum mode = modes[i * COLS + j];

      if (mode == GL_QUADS)
        continue;

      glViewport(j * PORT_W, i * PORT_H, PORT_W, PORT_H);

      if (mode == GL_TRIANGLE_FAN /* || mode == GL_QUADS*/)
        glDrawElements(mode, 4, GL_UNSIGNED_SHORT, &indices[10]);
      else if (mode == GL_TRIANGLES)
        glDrawElements(mode, 6, GL_UNSIGNED_SHORT, &indices[4]);
      else
        glDrawElements(mode, 4, GL_UNSIGNED_SHORT, &indices[0]);
    }
  }

  SDL_GL_SwapWindow(window);

#ifndef __EMSCRIPTEN__
  SDL_Event e;

  while (SDL_WaitEvent(&e)) {
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F12) {
      void *pixels = malloc(WIDTH * HEIGHT * 3);
      if (pixels) {
        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels, WIDTH, HEIGHT, 32, WIDTH * 3, SDL_PIXELFORMAT_RGB24);
        if (surface) {
          SDL_SaveBMP(surface, "sdl2_gldrawelements.bmp");
        }
      }
    }
    if (e.type == SDL_QUIT)
      break;
  }
#endif

  // Now we can delete the OpenGL texture and close down SDL
  glDeleteTextures(1, &texture);

  // Don't quit - we need to reftest the canvas! SDL_Quit();

  return 0;
}
