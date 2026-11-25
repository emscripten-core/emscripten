// Regression test for https://github.com/emscripten-core/emscripten/issues/27947
//
// GL divides the s and t texture coordinates by q. The fixed-function shaders
// that LEGACY_GL_EMULATION generates used to sample with texture2D(coord.xy)
// and ignore q, which is wrong whenever q is not 1: with four-component
// texture coordinates, or with a texture matrix whose last row is not
// (0, 0, 0, 1).

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include <assert.h>
#include <stdio.h>

void read_pixel(int x, int y, GLubyte rgba[4]) {
  glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
}

const GLubyte RED[3] = {0xFF, 0x00, 0x00};
const GLubyte GREEN[3] = {0x00, 0xFF, 0x00};
const GLubyte BLUE[3] = {0x00, 0x00, 0xFF};
const GLubyte WHITE[3] = {0xFF, 0xFF, 0xFF};

// Check the middle of each quarter of a horizontal band of the screen.
void check_band(const char *name, int y, const GLubyte *expected[4]) {
  for (int i = 0; i < 4; i++) {
    GLubyte px[4];
    read_pixel(80 + 160 * i, y, px);
    printf("%s quarter %d pixel: %d,%d,%d,%d\n", name, i, px[0], px[1], px[2], px[3]);
    for (int c = 0; c < 3; c++) {
      assert(expected[i][c] ? px[c] > 200 : px[c] < 50);
    }
  }
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

  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);

  // Bottom and middle bands: two-component texture coordinates in one
  // interleaved array, s from 0 to 1 on the bottom band and 0 to 2 on the
  // middle band.
  typedef struct {
    GLfloat x, y;
    GLfloat s, t;
  } Vertex2;
  Vertex2 bands[8] = {
    {-1.0, -1.0,     0.0, 0.0},
    { 1.0, -1.0,     1.0, 0.0},
    { 1.0, -1.0/3.0, 1.0, 1.0},
    {-1.0, -1.0/3.0, 0.0, 1.0},
    {-1.0, -1.0/3.0, 0.0, 0.0},
    { 1.0, -1.0/3.0, 2.0, 0.0},
    { 1.0,  1.0/3.0, 2.0, 2.0},
    {-1.0,  1.0/3.0, 0.0, 2.0},
  };
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(2, GL_FLOAT, sizeof(Vertex2), &bands[0].x);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex2), &bands[0].s);

  // Bottom band: q is 1, a plain lookup.
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  // Middle band: the texture matrix doubles q, which halves s. Only the
  // matrix changes between the two draws.
  GLfloat double_q[16] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 2.0,
  };
  glMatrixMode(GL_TEXTURE);
  glLoadMatrixf(double_q);
  glMatrixMode(GL_MODELVIEW);
  glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);

  // Top band: four-component (s*q, t*q, 0, q) texture coordinates with q
  // running from 1 on the left to 3 on the right, as perspective-correct
  // texturing in screen space produces them.
  typedef struct {
    GLfloat x, y;
    GLfloat s, t, r, q;
  } Vertex4;
  Vertex4 top[4] = {
    {-1.0, 1.0/3.0, 0.0, 0.0, 0.0, 1.0},
    { 1.0, 1.0/3.0, 3.0, 0.0, 0.0, 3.0},
    { 1.0, 1.0,     3.0, 3.0, 0.0, 3.0},
    {-1.0, 1.0,     0.0, 1.0, 0.0, 1.0},
  };
  glVertexPointer(2, GL_FLOAT, sizeof(Vertex4), &top[0].x);
  glTexCoordPointer(4, GL_FLOAT, sizeof(Vertex4), &top[0].s);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  // s at the middle of each quarter: 1/8, 3/8, 5/8, 7/8.
  const GLubyte *plain[4] = {RED, GREEN, BLUE, WHITE};
  check_band("bottom", 80, plain);
  // (2/8)/2 ... (14/8)/2: the same; ignoring q would give green, white,
  // green, white.
  check_band("middle", 240, plain);
  // At screen fraction f, s = 3f / (1 + 2f): 0.30, 0.64, 0.83, 0.95.
  // Ignoring q would give green, red, white, blue.
  const GLubyte *perspective[4] = {GREEN, BLUE, WHITE, WHITE};
  check_band("top", 400, perspective);

  SDL_GL_SwapBuffers();
  SDL_Quit();
  return 0;
}
