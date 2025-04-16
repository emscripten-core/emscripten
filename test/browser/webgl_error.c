#include <emscripten/html5.h>
#include <assert.h>
#include <GLES2/gl2.h>

int main() {
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  assert(ctx);
  emscripten_webgl_make_context_current(ctx);
  assert(!glGetError());
  char p[4];
  glReadPixels(0, 0, 1, 1, 0/*invalid*/, 0/*invalid*/, p);
  assert(glGetError());
  assert(!glGetError());
  return 0;
}
