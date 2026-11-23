/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>

GLuint compile_shader(GLenum shaderType, const char *src) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if (!isCompiled)
  {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    char *buf = (char*)malloc(maxLength);
    glGetShaderInfoLog(shader, maxLength, NULL, buf);
    printf("%s\n", buf);
    free(buf);
    return 0;
  }

  return shader;
}

GLuint create_program(GLuint vertexShader, GLuint fragmentShader)
{
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glBindAttribLocation(program, 0, "apos");
  glBindAttribLocation(program, 1, "acolor");
  glLinkProgram(program);
  return program;
}

#ifdef TEST_CLIENT_INDICES
static void
draw_tile(int tile, GLsizei count, GLenum type, const void* indices) {
  glViewport(tile * 8, 0, 8, 8);
  glDrawElements(GL_TRIANGLES, count, type, indices);
}
#endif

int main() {
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
#ifdef TEST_CLIENT_INDICES
  attr.majorVersion = TEST_CLIENT_INDICES;
  attr.antialias = 0;
#endif
#ifdef EXPLICIT_SWAP
  attr.explicitSwapControl = 1;
#endif
#ifdef DRAW_FROM_CLIENT_MEMORY
  // This test verifies that drawing from client-side memory when enableExtensionsByDefault==false works.
  attr.enableExtensionsByDefault = 0;
#endif

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  static const char vertex_shader[] =
    "attribute vec4 apos;"
    "attribute vec4 acolor;"
    "varying vec4 color;"
    "void main() {"
      "color = acolor;"
      "gl_Position = apos;"
    "}";
  GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);

  static const char fragment_shader[] =
    "precision lowp float;"
    "varying vec4 color;"
    "void main() {"
      "gl_FragColor = color;"
    "}";
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

  GLuint program = create_program(vs, fs);
  glUseProgram(program);

  static const float pos_and_color[] = {
  //     x,     y, r, g, b
     -0.6f, -0.6f, 1, 0, 0,
      0.6f, -0.6f, 0, 1, 0,
      0.f,   0.6f, 0, 0, 1,
  };

#ifdef DRAW_FROM_CLIENT_MEMORY
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 20, pos_and_color);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 20, (void*)(pos_and_color+2));
#else
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(pos_and_color), pos_and_color, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 20, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 20, (void*)8);
#endif
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glClearColor(0.3f,0.3f,0.3f,1);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 3);

#ifdef TEST_CLIENT_INDICES
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  // Queue draws into separate tiles. Six and eight bytes use the same buffer.
  GLubyte bytes[] = {0, 0, 0, 0, 1, 2, 0, 0};
  draw_tile(0, 6, GL_UNSIGNED_BYTE, bytes);
  bytes[5] = 1;
  draw_tile(1, 8, GL_UNSIGNED_BYTE, bytes);
  GLushort shorts[] = {2, 1, 0};
  draw_tile(2, 3, GL_UNSIGNED_SHORT, shorts);

  // With an application buffer bound, indices is a byte offset.
  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  bytes[5] = 2;
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bytes), bytes, GL_STATIC_DRAW);
  draw_tile(3, 3, GL_UNSIGNED_BYTE, (const void*)3);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  draw_tile(4, 3, GL_UNSIGNED_SHORT, shorts);

  // WebGL 2 treats a zero length from the Wasm heap as its entire tail.
  EM_ASM({
    GLctx.savedBufferData = GLctx.bufferData;
    GLctx.emptyIndexUploads = 0;
    GLctx.bufferData = function(target, data) {
      if (target == this.ELEMENT_ARRAY_BUFFER) {
        if (data !== 0 && data.byteLength !== 0) throw new Error('expected empty index data');
        ++this.emptyIndexUploads;
      }
      return this.savedBufferData.apply(this, arguments);
    };
  });
  draw_tile(5, 0, GL_UNSIGNED_SHORT, shorts);
  assert(glGetError() == GL_NO_ERROR);
  int uploads = EM_ASM_INT({ return GLctx.emptyIndexUploads; });
  glDrawElements(GL_TRIANGLES, -1, GL_UNSIGNED_SHORT, shorts);
  assert(glGetError() == GL_INVALID_VALUE);
  assert(EM_ASM_INT({ return GLctx.emptyIndexUploads; }) == uploads);
  EM_ASM({ GLctx.bufferData = GLctx.savedBufferData; });

  GLubyte pixels[6 * 8][4];
  glReadPixels(0, 3, 6 * 8, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  const int expected[] = {1, 0, 1, 1, 1, 0};
  for (int i = 0; i < 6; ++i) {
    const GLubyte* pixel = pixels[i * 8 + 4];
    assert((pixel[0] + pixel[1] + pixel[2] > 0) == expected[i]);
  }
#endif

#ifdef EXPLICIT_SWAP
  // Mark the program for deletion, first, as a regression test for the state
  // restoration. https://github.com/emscripten-core/emscripten/issues/23654
  glDeleteProgram(program);

  emscripten_webgl_commit_frame();
#endif

  assert(glGetError() == GL_NO_ERROR);
  return 0;
}
