// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Tests glClearDepth() and glDepthRange(). These exist only in desktop OpenGL,
// and not in OpenGL ES or WebGL, so they need -sLEGACY_GL_EMULATION.

#include <assert.h>
#include <GL/gl.h>
#include <SDL/SDL.h>

void check(GLfloat clear_depth, GLfloat near_val, GLfloat far_val) {
  GLfloat v[2];
  glGetFloatv(GL_DEPTH_CLEAR_VALUE, v);
  assert(v[0] == clear_depth);
  glGetFloatv(GL_DEPTH_RANGE, v);
  assert(v[0] == near_val && v[1] == far_val);
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_SetVideoMode(64, 64, 32, SDL_OPENGL);

  // Called directly.
  glClearDepth(0.25);
  glDepthRange(0.25, 0.5);
  check(0.25f, 0.25f, 0.5f);

  // Called via GetProcAddress.
  void (*clear_depth)(GLclampd) = SDL_GL_GetProcAddress("glClearDepth");
  void (*depth_range)(GLclampd, GLclampd) = SDL_GL_GetProcAddress("glDepthRange");
  assert(clear_depth && depth_range);
  clear_depth(0.75);
  depth_range(0.5, 0.75);
  check(0.75f, 0.5f, 0.75f);
  return 0;
}
