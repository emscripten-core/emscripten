// Regression test for https://github.com/emscripten-core/emscripten/issues/27945
//
// GL client-side vertex arrays must be re-read at draw time. An app may keep
// the same (planar) arrays bound via gl*Pointer and refill them in place
// between draws; each glDrawArrays must then draw the current contents.
// GLImmediate used to cache the restrided copy of planar arrays and skip the
// re-copy unless a gl*Pointer/client-state call had happened since the last
// draw, so the second draw below re-drew the first draw's geometry.

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

  // Two separate tightly-packed arrays (planar layout): the emulation copies
  // them into one interleaved staging buffer (its slow path).
  GLfloat xy[3][2] = {
    {-1.0, -1.0},
    { 0.0, -1.0},
    {-0.5,  1.0},
  };
  GLubyte rgba[3][4] = {
    {0xFF, 0x00, 0x00, 0xFF},
    {0xFF, 0x00, 0x00, 0xFF},
    {0xFF, 0x00, 0x00, 0xFF},
  };

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, xy);
  glColorPointer(4, GL_UNSIGNED_BYTE, 0, rgba);

  // First draw: a red triangle on the left half.
  glDrawArrays(GL_TRIANGLES, 0, 3);

  // Refill the SAME arrays in place -- no gl*Pointer or client-state call in
  // between, exactly like an app reusing static scratch arrays every flush.
  // Real GL reads client arrays at draw time, so this must be what is drawn.
  for (int i = 0; i < 3; i++) {
    xy[i][0] += 1.0;              // shift the triangle to the right half
    rgba[i][0] = 0x00;            // and turn it green
    rgba[i][1] = 0xFF;
  }

  // Second draw: must be a green triangle on the right half.
  glDrawArrays(GL_TRIANGLES, 0, 3);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  GLubyte left[4], right[4];
  read_pixel(160, 120, left);    // inside the first triangle only
  read_pixel(480, 120, right);   // inside the second triangle only
  printf("left pixel:  %d,%d,%d,%d\n", left[0], left[1], left[2], left[3]);
  printf("right pixel: %d,%d,%d,%d\n", right[0], right[1], right[2], right[3]);

  // The first draw drew the arrays' old contents...
  assert(left[0] > 200 && left[1] < 50 && left[2] < 50);
  // ...and the second draw their new contents.
  assert(right[0] < 50 && right[1] > 200 && right[2] < 50);

  SDL_GL_SwapBuffers();
  SDL_Quit();
  return 0;
}
