#include <stdio.h>
#include <string.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#include <webgl/webgl1_ext.h>
#include <assert.h>

int main()
{
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.enableExtensionsByDefault = 0;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  assert(!glGetError());

  // This should gracefully return null and record a GL error.
  const char *str = (const char *)glGetString(GL_UNMASKED_VENDOR_WEBGL);
  printf("%s\n", str);
  assert(glGetError());
  assert(!glGetError()); // One error is enough

  bool success = emscripten_webgl_enable_extension(ctx, "WEBGL_debug_renderer_info");
  if (!success)
  {
    // Browser does not have WEBGL_debug_renderer_info, skip remainder and return success.
    return 0;
  }

  assert(!glGetError());

  str = (const char *)glGetString(GL_UNMASKED_VENDOR_WEBGL);
  printf("%s\n", str);
  assert(strlen(str) > 3); // Should get something (dependent on hardware)
  assert(!glGetError());
  return 0;
}
