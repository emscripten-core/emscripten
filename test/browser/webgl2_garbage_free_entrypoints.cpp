// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>

GLuint CompileShader(GLenum type, const char *src)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);
  assert(glGetError() == GL_NO_ERROR && "Shader compilation failed!");
  return shader;
}

int main(int argc, char *argv[])
{
  emscripten_set_canvas_element_size("#canvas", 256, 256);
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.alpha = attr.depth = attr.stencil = attr.antialias = attr.preserveDrawingBuffer = attr.failIfMajorPerformanceCaveat = 0;
  attr.enableExtensionsByDefault = 1;
  attr.premultipliedAlpha = 0;
#ifdef TEST_WEBGL2
  attr.majorVersion = 2;
#else
  attr.majorVersion = 1;
#endif
  attr.minorVersion = 0;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  GLuint vs = CompileShader(GL_VERTEX_SHADER,
    "#version 100\n"
    "attribute vec4 pos; void main() { gl_Position = pos; }");

  GLuint ps = CompileShader(GL_FRAGMENT_SHADER,
    "#version 100\n"
    "precision lowp float;\n"
    "uniform vec3 color;\n"
    "uniform vec3 colors[3];\n"
    "void main() { gl_FragColor = vec4(color,1) + vec4(colors[0].r, colors[1].g, colors[2].b, 1); }");

  GLuint program = 0;
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, ps);
  glBindAttribLocation(program, 0, "pos");
  glLinkProgram(program);
  assert(glGetError() == GL_NO_ERROR && "Shader program link failed");

  int color_loc = glGetUniformLocation(program, "color");
  assert(glGetError() == GL_NO_ERROR);
  assert(color_loc != -1);

  glUseProgram(program);
  assert(glGetError() == GL_NO_ERROR);
  float col[3] = { 0.2f, 0.2f, 0.2f };
  glUniform3fv(color_loc, 1, col);
  assert(glGetError() == GL_NO_ERROR);

  int loc2 = glGetUniformLocation(program, "colors[2]");
  int loc = glGetUniformLocation(program, "colors");
  assert(glGetUniformLocation(program, "colors[2]") == loc+2);
  assert(loc2 == loc+2);
  assert(glGetUniformLocation(program, "colors[0]") == loc);
  assert(glGetUniformLocation(program, "colors[3]") == -1);
  assert(glGetUniformLocation(program, "colors[1]") == loc+1);
  assert(glGetUniformLocation(program, "colors[]") == loc);
  assert(glGetUniformLocation(program, "colors[-1]") == -1);
  assert(glGetUniformLocation(program, "colors[-100]") == -1);

  float colors[4*3] = { 1,0,0, 0,0.5,0, 0,0,0.2, 1,1,1 };

  // Pass the actual colors (testing a nonzero location offset), but do a
  // mistake by setting one index too many. Spec says this should be gracefully
  // handled, and that excess elements are ignored.
  glUniform3fv(loc+1, 3, colors+3);
  assert(glGetError() == GL_NO_ERROR);
  glUniform3fv(loc, 1, colors); // Set the first index as well.
  assert(glGetError() == GL_NO_ERROR);

  glUniform3fv(loc, 4, colors); // Just directly set the full array.
  assert(glGetError() == GL_NO_ERROR);

  GLuint vbo = 0;
  const GLfloat v[] = { -1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1 };
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  unsigned char pixel[4];
  glReadPixels(1, 1, 1, 1, GL_RGBA,GL_UNSIGNED_BYTE, pixel);
  printf("%d,%d,%d,%d\n", pixel[0], pixel[1], pixel[2], pixel[3]);
  assert(pixel[0] == 255);
  assert(pixel[1] == 178);
  assert(pixel[2] == 102);
  assert(pixel[3] == 255);

  printf("Test passed!\n");
  return 0;
}
