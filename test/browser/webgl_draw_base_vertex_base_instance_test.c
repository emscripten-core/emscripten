/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <webgl/webgl2_ext.h>

#include <GLES3/gl3.h>

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

GLuint create_program(GLuint vertexShader, GLuint fragmentShader) {
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glBindAttribLocation(program, 0, "apos");
  glBindAttribLocation(program, 1, "acolor");
  glLinkProgram(program);
  return program;
}

#ifndef WEBGL_CONTEXT_VERSION
#define WEBGL_CONTEXT_VERSION 2
#endif

int main() {
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
  EMSCRIPTEN_RESULT res;
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = WEBGL_CONTEXT_VERSION;
  ctx = emscripten_webgl_create_context("#canvas", &attrs);
  assert(ctx > 0); // Must have received a valid context.
  res = emscripten_webgl_make_context_current(ctx);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);

#ifdef EXPLICIT_SWAP
  attrs.explicitSwapControl = 1;
#endif

  GLboolean extAvailable = emscripten_webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(ctx);
  extAvailable &= emscripten_webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(ctx);

  if (!extAvailable) {
    EM_ASM({
      fetch('http://localhost:8888/report_result?skipped:%20WEBGL_draw_instanced_base_vertex_base_instance%20is%20not%20supported!').then(() => window.close());
    });
    return 0;
  }

  static const char vertex_shader[] =
    "#version 300 es\n"
    "layout(location=0) in vec4 apos;"
    "layout(location=1) in vec4 acolor;"
    "layout(location=2) in float icolor;"
    "out vec4 color;"
    "void main() {"
      "color = acolor;"
      "color.b = icolor;"
      "gl_Position = apos;"
    "}";
  GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);

  static const char fragment_shader[] =
    "#version 300 es\n"
    "precision lowp float;"
    "in vec4 color;"
    "out vec4 o_color;"
    "void main() {"
      "o_color = color;"
    "}";
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

  GLuint program = create_program(vs, fs);
  glUseProgram(program);

  static const float pos_and_color[] = {
  //     x,     y, r, g, b
     -0.5f, -0.5f, 1, 1, 0,
      0.5f, -0.5f, 1, 0, 0,
     -0.5f,   0.5f, 0, 1, 0,
      0.5f, 0.5f, 0, 0, 0,
     -0.5f,   0.5f, 0, 1, 0,
      0.5f, -0.5f, 1, 0, 0
  };

  // 2 -- 3
  // | \  |
  // |  \ |
  // 0 -- 1
  static const GLushort indices[] = {
    0, 1, 2,
    2, 1, 0
  };

  static const float instance_color[] = {
    0, 1
  };

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(pos_and_color), pos_and_color, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 20, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 20, (void*)8);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  GLuint ibo;
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ARRAY_BUFFER, ibo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(instance_color), instance_color, GL_STATIC_DRAW);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glVertexAttribDivisor(2, 1);
  glEnableVertexAttribArray(2);

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
  GLint baseVertices[] = {0, 1};
  GLuint baseInstances[] = {0, 1};
  GLsizei drawcount = 2;

  glClear(GL_COLOR_BUFFER_BIT);

#if MULTI_DRAW
#if DRAW_ELEMENTS
  glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL(GL_TRIANGLES, counts, GL_UNSIGNED_SHORT, offsets, instanceCounts, baseVertices, baseInstances, drawcount);
#else
  glMultiDrawArraysInstancedBaseInstanceWEBGL(GL_TRIANGLES, firsts, counts, instanceCounts, baseInstances, drawcount);
#endif
#else
#if DRAW_ELEMENTS
  for (GLsizei i = 0; i < drawcount; i++) {
    glDrawElementsInstancedBaseVertexBaseInstanceWEBGL(GL_TRIANGLES, counts[i], GL_UNSIGNED_SHORT, offsets[i], instanceCounts[i], baseVertices[i], baseInstances[i]);
  }
#else
  for (GLsizei i = 0; i < drawcount; i++) {
    glDrawArraysInstancedBaseInstanceWEBGL(GL_TRIANGLES, firsts[i], counts[i], instanceCounts[i], baseInstances[i]);
  }
#endif
#endif

#ifdef EXPLICIT_SWAP
  emscripten_webgl_commit_frame();
#endif
}
