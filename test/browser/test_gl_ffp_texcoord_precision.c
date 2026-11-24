// Regression test for https://github.com/emscripten-core/emscripten/issues/27948
//
// The fixed-function fragment shaders that LEGACY_GL_EMULATION generates used
// to declare `precision mediump float`. Where mediump really is half precision
// (for example on Apple GPUs), texture coordinates not far from zero lose most
// of their fraction: around 1000, fp16 can only represent steps of 0.5. GL 1.x
// requires texture coordinates up to 2^10 in magnitude, computed to about one
// part in 10^5, so the generated shaders must use highp where it is available.
//
// On GPUs where mediump is full precision this test passes either way.

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include <assert.h>
#include <stdio.h>

void read_pixel(int x, int y, GLubyte rgba[4]) {
  glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
}

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_Surface *screen = SDL_SetVideoMode(640, 480, 16, SDL_OPENGL);
  if (!screen) {
    printf("Unable to set video mode: %s\n", SDL_GetError());
    return 1;
  }

  glClearColor(0, 0, 0, 0);
  glViewport(0, 0, 640, 480);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT);

  // A 4x1 texture of red, green, blue and white texels, sampled NEAREST, so
  // each texel covers a quarter of the s range.
  GLubyte texels[4][4] = {
    {0xFF, 0x00, 0x00, 0xFF},
    {0x00, 0xFF, 0x00, 0xFF},
    {0x00, 0x00, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF},
  };
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texels);
  glEnable(GL_TEXTURE_2D);

  // Texture coordinates run from 1000 to 1001 across the screen, and the
  // texture matrix brings them back to 0..1.
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glTranslatef(-1000.0, 0.0, 0.0);
  glMatrixMode(GL_MODELVIEW);

  GLfloat xy[4][2] = {
    {-1.0, -1.0},
    { 1.0, -1.0},
    { 1.0,  1.0},
    {-1.0,  1.0},
  };
  GLfloat st[4][2] = {
    {1000.0, 0.0},
    {1001.0, 0.0},
    {1001.0, 1.0},
    {1000.0, 1.0},
  };

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, xy);
  glTexCoordPointer(2, GL_FLOAT, 0, st);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  // The middle of each quarter of the screen must show its own texel. With
  // half-precision texture coordinates the second quarter (s = 1000.375)
  // rounds to 1000.5 and shows the third texel instead.
  const GLubyte expected[4][3] = {
    {0xFF, 0x00, 0x00},
    {0x00, 0xFF, 0x00},
    {0x00, 0x00, 0xFF},
    {0xFF, 0xFF, 0xFF},
  };
  for (int i = 0; i < 4; i++) {
    GLubyte px[4];
    read_pixel(80 + 160 * i, 240, px);
    printf("quarter %d pixel: %d,%d,%d,%d\n", i, px[0], px[1], px[2], px[3]);
    for (int c = 0; c < 3; c++) {
      assert(expected[i][c] ? px[c] > 200 : px[c] < 50);
    }
  }

  SDL_GL_SwapBuffers();
  SDL_Quit();
  return 0;
}
