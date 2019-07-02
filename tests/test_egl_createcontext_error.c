/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <EGL/egl.h>
#include <emscripten.h>

int result = 1; // Success
#define assert(x) do { \
  if (!(x)) { \
    result = 0; \
    printf("Assertion failure: %s in %s:%d!\n", #x, __FILE__, __LINE__); \
  } \
} while(0)

int main(int argc, char *argv[]) {
  EM_ASM({
    Module['canvas'] = document.createElement('canvas');
    Module['canvas'].getContext = function() {
      return null;
    };
  });

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  assert(display != NULL);
  assert(eglInitialize(display, NULL, NULL));

  EGLConfig config;
  EGLint count;
  assert(eglChooseConfig(display, NULL, &config, 1, &count));
  assert(count == 1);

  EGLint attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  assert(eglCreateContext(display, config, NULL, attribs) == NULL);
  assert(eglGetError() == EGL_BAD_MATCH);

#ifdef REPORT_RESULT
  REPORT_RESULT(result);
#endif
}
