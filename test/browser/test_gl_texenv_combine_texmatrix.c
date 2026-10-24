// Regression test for https://github.com/emscripten-core/emscripten/issues/27946
//
// GL_COMBINE texture environments that use the texture more than once, drawn
// while a texture matrix is in use. The generated fragment shader samples the
// texture as `texture2D(u_texUnit0, (u_textureMatrix0 * v_texCoord0).xy)`, and
// the pass that merges repeated texture loads into one variable used to find
// them with a regex that stopped at the first `)` -- the one closing the
// matrix product -- which turned the shader into invalid GLSL, so nothing was
// drawn.

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void read_pixel(int x, int y, GLubyte rgba[4]) {
  glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
}

void check_rgb(const char *name, const GLubyte px[4], int r, int g, int b) {
  printf("%s pixel: %d,%d,%d,%d\n", name, px[0], px[1], px[2], px[3]);
  assert(abs(px[0] - r) <= 8 && abs(px[1] - g) <= 8 && abs(px[2] - b) <= 8);
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

  // A single half-transparent green texel.
  GLubyte texel[4] = {0x00, 0xFF, 0x00, 0x80};
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texel);
  glEnable(GL_TEXTURE_2D);

  // Any texture matrix use puts the matrix into the sample expression.
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glTranslatef(0.25, 0.0, 0.0);
  glMatrixMode(GL_MODELVIEW);

  // One interleaved array (the emulation's fast path): a red quad on each
  // half of the screen, with texture coordinates.
  typedef struct {
    GLfloat x, y;
    GLfloat s, t;
    GLubyte rgba[4];
  } Vertex;
  Vertex quads[8] = {
    {-1.0, -1.0, 0.0, 0.0, {0xFF, 0x00, 0x00, 0xFF}},
    { 0.0, -1.0, 1.0, 0.0, {0xFF, 0x00, 0x00, 0xFF}},
    { 0.0,  1.0, 1.0, 1.0, {0xFF, 0x00, 0x00, 0xFF}},
    {-1.0,  1.0, 0.0, 1.0, {0xFF, 0x00, 0x00, 0xFF}},
    { 0.0, -1.0, 0.0, 0.0, {0xFF, 0x00, 0x00, 0xFF}},
    { 1.0, -1.0, 1.0, 0.0, {0xFF, 0x00, 0x00, 0xFF}},
    { 1.0,  1.0, 1.0, 1.0, {0xFF, 0x00, 0x00, 0xFF}},
    { 0.0,  1.0, 0.0, 1.0, {0xFF, 0x00, 0x00, 0xFF}},
  };

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &quads[0].x);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &quads[0].s);
  glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), quads[0].rgba);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

  // Left half: rgb = texture.rgb * texture.a, two loads on one line.
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_ALPHA);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  // Right half: rgb = mix(primary color, texture, texture.a), the texture
  // loaded on two lines.
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
  glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  GLubyte px[4];
  read_pixel(160, 240, px);
  check_rgb("left", px, 0x00, 0x80, 0x00);  // green * 0.5
  read_pixel(480, 240, px);
  check_rgb("right", px, 0x7F, 0x80, 0x00); // half red, half green

  SDL_GL_SwapBuffers();
  SDL_Quit();
  return 0;
}
