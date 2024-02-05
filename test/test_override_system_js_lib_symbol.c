#include <emscripten.h>
#include <emscripten/html5.h>
#include <assert.h>
#include <GLES3/gl3.h>
#include <stdio.h>

GLuint what_got_created(void);

int main() {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  emscripten_webgl_make_context_current(emscripten_webgl_create_context("canvas", &attrs));

  GLuint createType = GL_UNSIGNED_BYTE;
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 1, 1, 1, 0, GL_RGBA, createType, 0);
  GLuint whatGotCreated = what_got_created();
  printf("Created texture of type 0x%x\n", whatGotCreated);
  assert(createType == whatGotCreated);
  return 0;
}
