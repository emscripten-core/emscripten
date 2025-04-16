/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GL/gl.h>
#include <stdio.h>
#include <string.h>
#include "SDL/SDL.h"

int main() {
  SDL_Surface *screen;

  assert(SDL_Init(SDL_INIT_VIDEO) == 0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  screen = SDL_SetVideoMode( 256, 256, 16, SDL_OPENGL );
  assert(screen);

  // pop from empty stack
  glPopMatrix();
  assert(glGetError() == GL_STACK_UNDERFLOW);

  REPORT_RESULT(1);
  return 0;
}
