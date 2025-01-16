// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <emscripten/html5.h>


int main() {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = WEBGL_CONTEXT_MAJOR_VERSION; // provided when invoking the test
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
  // turn the error call into abort so that it can be detected by the test
  EM_ASM({
    err = (msg) => { abort(`Expected Error: ${msg}`); }
  });
  context = emscripten_webgl_create_context("#canvas", &attrs);
  assert(context);
  emscripten_webgl_destroy_context(context);
}
