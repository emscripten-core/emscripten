#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <assert.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

int main()
{
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context(0, &attrs);
  assert(context > 0); // Must have received a valid context.
  EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(context);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  assert(emscripten_webgl_get_current_context() == context);
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
