/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>

GLuint compile_shader(GLenum shaderType, const char *src)
{
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if (!isCompiled)
  {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    char *buf = (char*)malloc(maxLength+1);
    glGetShaderInfoLog(shader, maxLength, &maxLength, buf);
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

int main()
{
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.majorVersion = 2;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  static const char vertex_shader[] =
    "#version 300 es\n"
    "in vec4 apos;\n"
    "in int idx;\n"
    "struct S { vec4 color; };\n"
    "uniform S colors[3];\n"
    "out vec4 color;\n"
    "void main() {\n"
      "color = colors[idx].color;\n"
      "gl_Position = apos;\n"
    "}\n";
  GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);

  static const char fragment_shader[] =
    "#version 300 es\n"
    "precision lowp float;\n"
    "in vec4 color;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
      "fragColor = color;\n"
    "}\n";
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

  GLuint program = create_program(vs, fs);
  glUseProgram(program);

  struct P
  {
    float x, y;
    int idx;
  };

  static const struct P pos_and_idx[] = {
     { -0.6f, -0.6f, 0 },
     {  0.6f, -0.6f, 1 },
     {  0.f,   0.6f, 2 }
  };

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(pos_and_idx), pos_and_idx, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 12, 0);
  glVertexAttribIPointer(1, 1, GL_INT, 12, (void*)8);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  float red[4]   = { 1.0f, 0.f, 0.0f, 1.0f };
  float green[4] = { 0.0f, 1.f, 0.0f, 1.0f };
  float blue[4]  = { 0.0f, 0.f, 1.0f, 1.0f };
  int loc = glGetUniformLocation(program, "colors[0].color");
  assert(loc >= 0);
  glUniform4fv(loc, 1, red);
  loc = glGetUniformLocation(program, "colors[1].color");
  assert(loc >= 0);
  glUniform4fv(loc, 1, green);
  loc = glGetUniformLocation(program, "colors[2].color");
  assert(loc >= 0);
  glUniform4fv(loc, 1, blue);
  glClearColor(0.3f,0.3f,0.3f,1);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}
