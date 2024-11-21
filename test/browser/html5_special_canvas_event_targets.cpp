// Copyright 2024 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <assert.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

const char *TARGETS[]={"!foovas", "#canvas", "canvas"};
const char *target = TARGETS[WHICH_TARGET];

int main()
{
  EM_ASM({
    specialHTMLTargets["!foovas"] = Module.canvas;
  });
  int w=0, h=0;
  EMSCRIPTEN_RESULT res = emscripten_get_canvas_element_size(target, &w, &h);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context(target, &attrs);
  assert(context > 0);
  res = emscripten_webgl_make_context_current(context);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  assert(emscripten_webgl_get_current_context() == context);
  res = emscripten_get_canvas_element_size(target, &w, &h);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  res = emscripten_webgl_destroy_context(context);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  return 0;
}
