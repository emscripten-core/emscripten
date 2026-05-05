// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Regression test for $webglGetUniformLocation silently returning undefined
// when called from a glUniform*() setter on a freshly linked program before
// any glGetUniformLocation() has run.
//
// glLinkProgram resets program.uniformLocsById to 0; only
// $webglPrepareUniformLocationsBeforeFirstUse populates it. That prepare call
// used to live only on the glGetUniformLocation() path, so a program author
// using -sGL_EXPLICIT_UNIFORM_LOCATION and addressing uniforms by their
// layout(location=N) value (i.e. without ever calling glGetUniformLocation)
// would see glUniform*() silently no-op: GLctx.uniform*(undefined, ...) is
// dropped by WebGL with no error.

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

  const char* vs = "#version 300 es\n"
                   "layout(location = 6) uniform vec3 u_val;\n"
                   "void main() { gl_Position = vec4(u_val, 1.0); }\n";

  const char* fs = "#version 300 es\n"
                   "precision mediump float;\n"
                   "out vec4 o;\n"
                   "void main() { o = vec4(1.0); }\n";

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

  glUseProgram(p);

  // Set a uniform using its explicit layout(location=6) WITHOUT first calling
  // glGetUniformLocation. Before the fix, this silently no-ops because
  // $webglGetUniformLocation reads from program.uniformLocsById which is still
  // 0 from glLinkProgram, returns undefined, and GLctx.uniform3fv ignores it.
  const float val[] = {1.0f, 2.0f, 3.0f};
  glUniform3fv(/* location */ 6, 1, val);
  assert(glGetError() == GL_NO_ERROR);

  float rb[3] = {-1.f, -1.f, -1.f};
  glGetUniformfv(p, /* location */ 6, rb);
  assert(glGetError() == GL_NO_ERROR);
  assert(rb[0] == 1.0f && rb[1] == 2.0f && rb[2] == 3.0f);

  // Subsequent set should also work (was already working before the fix,
  // because the readback's prepare populated uniformLocsById as a side effect).
  const float val2[] = {4.0f, 5.0f, 6.0f};
  glUniform3fv(/* location */ 6, 1, val2);
  assert(glGetError() == GL_NO_ERROR);

  float rb2[3] = {-1.f, -1.f, -1.f};
  glGetUniformfv(p, /* location */ 6, rb2);
  assert(glGetError() == GL_NO_ERROR);
  assert(rb2[0] == 4.0f && rb2[1] == 5.0f && rb2[2] == 6.0f);

  printf("Test passed!\n");
  return 0;
}
