// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>

#ifndef GL_MAX_UNIFORM_LOCATIONS
#define GL_MAX_UNIFORM_LOCATIONS          0x826E
#endif

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
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  GLint maxUniformLocations = 0;
  glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &maxUniformLocations);
  assert(maxUniformLocations == 1048576);

  GLuint vs = CompileShader(GL_VERTEX_SHADER,
    "#version 100\n"
    "layout(location = 42) uniform mat4 world;\n"
    "layout(location = 0) uniform mat4 view;\n"
    " // layout(location = -1) uniform mat4 proj; // Invalid usage, check this is preprocessed away\n"
    " /* layout(location = 100000000) uniform mat4 proj; Invalid usage, check this is preprocessed away */\n"
    "attribute vec4 pos; void main() { gl_Position = view*world*pos; }");

  GLuint ps = CompileShader(GL_FRAGMENT_SHADER,
    "#version 100\n"
    "precision lowp float;\n"
    "#define LOCATION(x) layout(location=x)\n"
    "LOCATION(8) uniform vec3 color;\n"
    "layout(location = 18) uniform vec3 colors[3];\n"
    "void main() { gl_FragColor = vec4(color,1) + vec4(colors[0].r, colors[1].g, colors[2].b, 1); }");

  GLuint program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, ps);
  glLinkProgram(program);
  assert(glGetError() == GL_NO_ERROR && "Shader program link failed");

  assert(glGetUniformLocation(program, "world") == 42);
  assert(glGetUniformLocation(program, "view") == 0);
  assert(glGetUniformLocation(program, "color") == 8);
  assert(glGetUniformLocation(program, "colors") == 18);
  assert(glGetUniformLocation(program, "colors[]") == 18);
  assert(glGetUniformLocation(program, "colors[0]") == 18);
  assert(glGetUniformLocation(program, "colors[1]") == 19);
  assert(glGetUniformLocation(program, "colors[2]") == 20);

  printf("Test passed!\n");
#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
}
