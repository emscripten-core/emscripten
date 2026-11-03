// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;

int main()
{
  // Hack context creation to fail for WebGL2 at runtime.
  EM_ASM({
    var original = Module.canvas.getContext;
    Module.canvas.getContext = function(name, attrs) {
      out('ask', name, attrs);
      if (name === "webgl2") return null;
      out('provide!');
      return original.call(Module.canvas, name, attrs);
    };
  });

  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;

  // Request WebGL2 in a build compiled to allow 1 or 2. This should fail
  // because we forced it to in that EM_ASM.
  attrs.majorVersion = 2;
  attrs.minorVersion = 0;
  context = emscripten_webgl_create_context("#canvas", &attrs);
  assert(!context);

  // Request WebGL1, which succeeds.
  attrs.majorVersion = 1;
  context = emscripten_webgl_create_context("#canvas", &attrs);
  assert(context);

  // Verify it is indeed WebGL1.
  EmscriptenWebGLContextAttributes outAttrs;
  EMSCRIPTEN_RESULT res = emscripten_webgl_get_context_attributes(context, &outAttrs);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);

  printf("requested: %d actual: %d\n", attrs.majorVersion, outAttrs.majorVersion);
  assert(outAttrs.majorVersion == 1);
  return 0;
}
