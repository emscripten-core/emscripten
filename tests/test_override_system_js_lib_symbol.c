#include <emscripten.h>
#include <emscripten/html5.h>
#include <assert.h>
#include <GLES2/gl2.h>
#include <stdio.h>

GLuint what_got_created(void);

int main() {
  GLuint createType = GL_UNSIGNED_BYTE;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, createType, 0);
  GLuint whatGotCreated = what_got_created();
  printf("Created texture of type 0x%x\n", whatGotCreated);
  assert(createType == whatGotCreated);
  return 0;
}
