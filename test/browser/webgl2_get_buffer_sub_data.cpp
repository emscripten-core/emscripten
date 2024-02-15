#include <GLES3/gl3.h>
#include <webgl/webgl2.h>
#include <emscripten/html5_webgl.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

int main()
{
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.majorVersion = 2;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  GLuint buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  uint8_t data2[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };

  // Test full buffer read
  glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data2);
  printf("read data: %x %x %x %x %x %x %x %x\n",
         data2[0],
         data2[1],
         data2[2],
         data2[3],
         data2[4],
         data2[5],
         data2[6],
         data2[7]);
  assert(!memcmp(data, data2, sizeof(data)));

  // Test partial buffer read
  memset(data2, 0, 8);
  glGetBufferSubData(GL_ARRAY_BUFFER, 3, 4, data2 + 3);
  assert(!memcmp(data + 3, data2 + 3, 4));
  assert(data2[0] == 0);
  assert(data2[1] == 0);
  assert(data2[2] == 0);
  assert(data2[7] == 0);

  printf("Test passed!\n");
  return 0;
}
