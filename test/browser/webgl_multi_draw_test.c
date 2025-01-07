/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <webgl/webgl1_ext.h>
#include <webgl/webgl2_ext.h>

#include <GLES2/gl2.h>

GLuint compile_shader(GLenum shaderType, const char *src) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if (!isCompiled) {
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

GLuint create_program(GLuint vertexShader, GLuint fragmentShader) {
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
#ifdef EXPLICIT_SWAP
  attr.explicitSwapControl = 1;
#endif

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  GLboolean extAvailable = emscripten_webgl_enable_WEBGL_multi_draw(ctx);

  if (!extAvailable) {
    EM_ASM({
      fetch("http://localhost:8888/report_result?skipped:%20WEBGL_multi_draw%20is%20not%20supported!").then(() => windows.close());
    });
    return 0;
  }

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
     -0.5f, -0.5f, 1, 0, 0,
      0.5f, -0.5f, 0, 1, 0,
     -0.5f,   0.5f, 0, 0, 1,
      0.5f, 0.5f, 1, 1, 1,
     -0.5f,   0.5f, 0, 0, 1,
      0.5f, -0.5f, 0, 1, 0,
  };

  // 2 -- 3
  // | \  |
  // |  \ |
  // 0 -- 1
  static const GLushort indices[] = {
    0, 1, 2,
    3, 2, 1
  };

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(pos_and_color), pos_and_color, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 20, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 20, (void*)8);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  // use element array buffer
  GLuint elementBuffer;
  glGenBuffers(1, &elementBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glClearColor(0.3f,0.3f,0.3f,1);

  GLint firsts[] = {0, 3};
  GLsizei counts[] = {3, 3};
  GLsizei instanceCounts[] = {1, 1};
  const GLvoid* const offsets[] = {(GLvoid*)0, (GLvoid*)(3 * sizeof(GLushort))};
  GLsizei drawcount = 2;

#ifdef MULTI_DRAW_ARRAYS
  glClear(GL_COLOR_BUFFER_BIT);
  glMultiDrawArraysWEBGL(GL_TRIANGLES, firsts, counts, drawcount);
#endif

#ifdef MULTI_DRAW_ARRAYS_INSTANCED
  glClear(GL_COLOR_BUFFER_BIT);
  glMultiDrawArraysInstancedWEBGL(GL_TRIANGLES, firsts, counts, instanceCounts, drawcount);
#endif

#ifdef MULTI_DRAW_ELEMENTS
  glClear(GL_COLOR_BUFFER_BIT);
  glMultiDrawElementsWEBGL(GL_TRIANGLES, counts, GL_UNSIGNED_SHORT, offsets, drawcount);
#endif

#ifdef MULTI_DRAW_ELEMENTS_INSTANCED
  glClear(GL_COLOR_BUFFER_BIT);
  glMultiDrawElementsInstancedWEBGL(GL_TRIANGLES, counts, GL_UNSIGNED_SHORT, offsets, instanceCounts, drawcount);
#endif

#ifdef EXPLICIT_SWAP
  emscripten_webgl_commit_frame();
#endif

}
