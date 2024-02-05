/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <GL/glew.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* for context creation */
#include <SDL/SDL.h>

int main() {
  assert(SDL_Init(SDL_INIT_VIDEO) == 0);
  assert(SDL_SetVideoMode(640, 480, 16, SDL_OPENGL) != NULL);

  assert(glewInit() == GLEW_OK);
  assert(glewGetString(0) == NULL);
  assert(!strcmp((const char*)glewGetString(1), "1.10.0"));
  assert(!strcmp((const char*)glewGetString(2), "1"));
  assert(!strcmp((const char*)glewGetString(3), "10"));
  assert(!strcmp((const char*)glewGetString(4), "0"));

  for (int i = 0; i < 8; ++i) {
    assert(glewGetErrorString(i) != NULL);
  }

  assert(glewGetExtension("EXT_unexistant") == 0);
  assert(glewIsSupported("EXT_unexistant EXT_foobar") == 0);

  /* we can't be sure about which extension exists, so lets do test on
   * some of the common ones */
  if (GLEW_EXT_texture_filter_anisotropic) {
    assert(glewGetExtension("EXT_texture_filter_anisotropic") == 1);
    assert(glewGetExtension("GL_EXT_texture_filter_anisotropic") == 1);
  }

  if (GLEW_EXT_framebuffer_object) {
    assert(glewGetExtension("EXT_framebuffer_object") == 1);
    assert(glewGetExtension("GL_EXT_framebuffer_object") == 1);
  }

  if (GLEW_EXT_texture_filter_anisotropic &&
    GLEW_EXT_framebuffer_object) {
    assert(glewIsSupported("EXT_texture_filter_anisotropic EXT_framebuffer_object") == 1);
    assert(glewIsSupported("GL_EXT_texture_filter_anisotropic GL_EXT_framebuffer_object") == 1);
  }

#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
  return 0;
}
