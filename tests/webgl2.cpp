#include <GLES3/gl3.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>
#include <html5.h>

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;

int main()
{
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  attrs.minorVersion = 0;

  int result = 0;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context(0, &attrs);
  if (context)
  {
    EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(context);
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
    if (!hasGLExtension)
      result = 1;
  }

#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif
  return 0;
}
