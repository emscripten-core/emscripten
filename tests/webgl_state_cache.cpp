#include <GLES3/gl3.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;

extern "C" {

int getActiveAttribs();

}

int main()
{
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  attrs.minorVersion = 0;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context(0, &attrs);
  emscripten_webgl_make_context_current(context);

  // Hit some GL_STATE_CACHE=1 entry points
  glDisableVertexAttribArray(0);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  glDisableVertexAttribArray(2);
  glDrawArrays(0,0,0);

  int result = getActiveAttribs();
  printf("Active attribs: %d\n", result);
#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif
  return 0;
}
