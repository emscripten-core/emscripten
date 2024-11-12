#include <emscripten/html5.h>
#include <string.h>
#include <assert.h>
#include <GLES2/gl2.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>

int main()
{
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.majorVersion = 1;
  attr.enableExtensionsByDefault = 0;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  bool hasVaos = emscripten_webgl_enable_extension(ctx, "OES_vertex_array_object");
  if (hasVaos)
  {
    GLuint vao = 0;
    glGenVertexArraysOES(1, &vao);
    assert(vao != 0);
  }

  return 0;
}
