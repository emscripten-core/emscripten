// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <GLES3/gl3.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;

int main()
{
  EM_ASM({
    Array.prototype.someExtensionFromThirdParty = {};
    Array.prototype.someExtensionFromThirdParty.length = 42;
    Array.prototype.someExtensionFromThirdParty.something = function() { return "Surprise!"; };
  });

  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  attrs.minorVersion = 0;
  attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
  if (context)
  {
    memset(&attrs, -1, sizeof(attrs));
    EMSCRIPTEN_RESULT res = emscripten_webgl_get_context_attributes(context, &attrs);
    assert(res == EMSCRIPTEN_RESULT_SUCCESS);
    assert(attrs.majorVersion == 2);
    assert(attrs.powerPreference == EM_WEBGL_POWER_PREFERENCE_DEFAULT || attrs.powerPreference == EM_WEBGL_POWER_PREFERENCE_LOW_POWER || attrs.powerPreference == EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE);
    res = emscripten_webgl_make_context_current(context);
    assert(res == EMSCRIPTEN_RESULT_SUCCESS);
    assert(emscripten_webgl_get_current_context() == context);

    int numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    bool hasGLExtension = false;
    for(int i = 0; i < numExtensions; ++i)
    {
      const char *ext = (const char *)glGetStringi(GL_EXTENSIONS, i);
      printf("extension %d: %s\n", i, ext);
      if (strstr(ext, "GL_") == 0)
        hasGLExtension = true;
    }
    assert(hasGLExtension);
  }

  return 0;
}
