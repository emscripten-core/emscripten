// Tests that glReadPixels() works when GL_PACK_ROW_LENGTH > read width.
#include <GLES3/gl3.h>
#include <emscripten/html5.h>
#include <assert.h>
#include <string.h>

int main() {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attrs);
  emscripten_webgl_make_context_current(ctx);

  // Clear framebuffer to red.
  glClearColor(1, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  // Read 8 pixels wide into a buffer with row length 32.
  glPixelStorei(GL_PACK_ROW_LENGTH, 32);
  GLubyte buf[32 * 2 * 4];
  memset(buf, 0, sizeof(buf));
  glReadPixels(0, 0, 8, 2, GL_RGBA, GL_UNSIGNED_BYTE, buf);

  // Verify pixel at (0,0) is red.
  assert(buf[0] == 255 && buf[1] == 0 && buf[2] == 0 && buf[3] == 255);

  // Verify that pixel at (0,1) was read to address at stride 32 (pixels), so is red as well.
  assert(buf[32*4] == 255 && buf[32*4+1] == 0 && buf[32*4+2] == 0 && buf[32*4+3] == 255);

  return 0;
}
