// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Regression test for two bugs in $emscriptenWebGLGetUniform (libwebgl.js):
//
//   1. glGetUniform*v resolved its uniform location against
//      `GLctx.currentProgram` instead of the explicit `program` parameter, so
//      calling glGetUniform*v without a prior glUseProgram(program) threw
//      "parameter 2 is not of type 'WebGLUniformLocation'" from
//      GLctx.getUniform. Per the GLES spec, glGetUniform*v takes the program
//      explicitly and does not require it to be bound.
//
//   2. Under -sGL_ASSERTIONS, the location validation ran before the integer
//      ID -> WebGLProgram lookup, dereferencing a property of a number and
//      throwing "Cannot read properties of undefined (reading '<location>')".

#include <assert.h>
#include <stdio.h>
#include <GLES3/gl3.h>
#include <emscripten/html5.h>

int main() {
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.majorVersion = 2;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  assert(ctx);
  emscripten_webgl_make_context_current(ctx);

  const char* vs = "#version 300 es\nvoid main(){gl_Position=vec4(0);}";
  const char* fs = "#version 300 es\n"
                   "precision mediump float;\n"
                   "uniform float u;\n"
                   "out vec4 o;\n"
                   "void main(){ o = vec4(u); }";

  GLuint v = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(v, 1, &vs, NULL);
  glCompileShader(v);

  GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(f, 1, &fs, NULL);
  glCompileShader(f);

  GLuint p = glCreateProgram();
  glAttachShader(p, v);
  glAttachShader(p, f);
  glLinkProgram(p);

  GLint linked = 0;
  glGetProgramiv(p, GL_LINK_STATUS, &linked);
  assert(linked && "Program link failed");

  GLint loc = glGetUniformLocation(p, "u");
  assert(loc >= 0);

  glUseProgram(p);
  glUniform1f(loc, 1.5f);
  assert(glGetError() == GL_NO_ERROR);

  // Case A: program currently bound. Without the fix, this crashes under
  // -sGL_ASSERTIONS=1 because validateGLObjectID(program.uniformLocsById, ...)
  // runs before the integer -> WebGLProgram swap.
  float outA = -1.f;
  glGetUniformfv(p, loc, &outA);
  assert(glGetError() == GL_NO_ERROR);
  assert(outA == 1.5f);

  // Case B: no program currently bound. Without the fix, glGetUniform*v
  // resolves `location` against GLctx.currentProgram (null), the inner GL
  // call receives `undefined` for the WebGLUniformLocation, and WebGL throws
  // a TypeError.
  glUseProgram(0);
  float outB = -1.f;
  glGetUniformfv(p, loc, &outB);
  assert(glGetError() == GL_NO_ERROR);
  assert(outB == 1.5f);

  printf("Test passed!\n");
  return 0;
}
