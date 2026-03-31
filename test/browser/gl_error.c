/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
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

  // glGetError again should return 0 initially
  assert(glGetError() == 0);

  // pop from empty stack, causing an underflow error
  glPopMatrix();
  GLenum err = glGetError();
  printf("glGetError -> %d\n", err);
  assert(err == GL_STACK_UNDERFLOW);

  // Calling glGetError again should report no error.
  assert(glGetError() == 0);

  return 0;
}
