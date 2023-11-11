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
#include <webgl/webgl1_ext.h>

GLuint CompileShader(GLenum type, const char *src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);
  assert(glGetError() == GL_NO_ERROR && "Shader compilation failed!");
  return shader;
}

GLuint CreateProgram(GLuint vertexShader, GLuint fragmentShader) {
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glBindAttribLocation(program, 0, "apos");
  glLinkProgram(program);
  return program;
}

int main(int argc, char *argv[]) {
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.majorVersion = 2;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  assert(ctx);
  emscripten_webgl_make_context_current(ctx);

  GLint maxUniformLocations = 0;
  glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &maxUniformLocations);
  assert(maxUniformLocations == 1048576);

  GLuint vs = CompileShader(GL_VERTEX_SHADER,
    "#version 300 es\n"
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n" // GL_FRAGMENT_PRECISION_HIGH is a predefined variable
    "layout(location = 42) uniform mat4 world;\n"
    "#endif\n"
    "#if GL_FRAGMENT_PRECISION_HIGH\n"
    "layout(location = 0) uniform mat4 view;\n"
    "#endif\n"
    " // layout(location = -1) uniform mat4 proj; // Invalid usage, check this is preprocessed away\n"
    " /* layout(location = 100000000) uniform mat4 proj; Invalid usage, check this is preprocessed away */\n"
    "layout(location = 0) in vec4 pos; // Make sure attribute layout locations don't get removed by preprocessor\n"
    "void main() { gl_Position = view*world*pos; }");

  GLuint ps = CompileShader(GL_FRAGMENT_SHADER,
    "#version 300 es\n"
    "precision lowp float;\n"
    "#define LOCATION(x) layout(location=x)\n"
    "LOCATION(8) uniform highp vec3 color;\n"
    "LOCATION(11) uniform mediump vec4 color2;\n"
    "layout(location = 18) uniform lowp vec3 colors[3];\n"
    "layout(location = 24) uniform vec3 colors2[3];\n"
    "LOCATION(0) out highp vec4 SV_TARGET0; // Make sure MRT output locations don't get removed by preprocessor\n"
    "void main() { SV_TARGET0 = vec4(color,1) + color2 + vec4(colors[0].r, colors[1].g, colors[2].b, 1) + vec4(colors2[0].r, colors2[1].g, colors2[2].b, 1); }");

  GLuint program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, ps);
  glLinkProgram(program);
  assert(glGetError() == GL_NO_ERROR && "Shader program link failed");
  glUseProgram(program);

  // Test that we can call glGetUniformfv() on a prebound location.
  float val[4];
  memset(val, -1, sizeof(val));
  glGetUniformfv(program, 11, val);
  assert(val[0] == 0 && val[1] == 0 && val[2] == 0 && val[3] == 0);

  // Test that we can call glGetUniformfv() on an array location.
  memset(val, -1, sizeof(val));
  glGetUniformfv(program, 19, val);
  assert(val[0] == 0 && val[1] == 0 && val[2] == 0);

  assert(glGetUniformLocation(program, "world") == 42);
  assert(glGetUniformLocation(program, "view") == 0);
  assert(glGetUniformLocation(program, "color") == 8);
  assert(glGetUniformLocation(program, "colors[2]") == 20);
  assert(glGetUniformLocation(program, "colors") == 18);
  assert(glGetUniformLocation(program, "colors[]") == 18);
  assert(glGetUniformLocation(program, "colors[0]") == 18);
  assert(glGetUniformLocation(program, "colors[1]") == 19);

  float world[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
  glUniformMatrix4fv(42, 1, GL_FALSE, world);

  float view[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
  glUniformMatrix4fv(0, 1, GL_FALSE, view);

  static const float vb[] = {
     -0.6f, -0.6f,
      0.6f, -0.6f,
      0.f,   0.6f,
  };

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vb), vb, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, 0);
  glEnableVertexAttribArray(0);

  // Test submitting a non-array uniform
  glUniform3f(8/*color*/, 0.1f, 0.2f, 0.1f);
  // Test submitting a non-array uniform that has never had glGetUniformLocation() called on it
  glUniform4f(11/*color2*/, 0.2f, 0.2f, 0.3f, 1.f);
  // Test submitting an array uniform
  float colors[9] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.1f, 0.2f, 0.1f, 0.0f, 0.3f };
  glUniform3fv(18, 3, colors);
    // Test submitting an array uniform that has never had glGetUniformLocation() called on it
  float colors2[9] = { 0.2f, 0.3f, 0.4f, 0.5f, 0.2f, 0.1f, 0.0f, 0.2f, 0.1f };
  glUniform3fv(24, 3, colors2);

  glClearColor(0.3f,0.3f,0.3f,1);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  uint8_t data[4];
  glReadPixels(150, 75, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
  printf("output triangle color: %u, %u, %u, %u\n", data[0], data[1], data[2], data[3]);
  assert(data[0] == 153);
  assert(data[1] == 178);
  assert(data[2] == 204);
  assert(data[3] == 255);

  // Test that setting program zero is allowed
  glUseProgram(0);
  assert(!glGetError());

  // Test that calling glGetUniformLocation() without an active program should report a GL_INVALID_VALUE.
  glGetUniformLocation(0, "colors[0]");
  assert(glGetError() == GL_INVALID_VALUE);

  // Test that calling glUniform*() without an active program should report a GL_INVALID_OPERATION.
  assert(!glGetError());
  glUniform4f(11/*color2*/, 0.2f, 0.2f, 0.3f, 1.f);
  assert(glGetError() == GL_INVALID_OPERATION);

  // Test that explicit and implicit uniform location numberings do not collide
  GLuint program2 = glCreateProgram();
  glAttachShader(program2, CompileShader(GL_VERTEX_SHADER,
    "#version 300 es\n"
    "uniform mat4 world;\n"
    "layout(location = 1) uniform mat4 view;\n" // Should get an automatically assigned location that starts numbering after the highest location that is used explicitly (at 2)
    "layout(location = 0) in vec4 pos;\n"
    "void main() { gl_Position = view*world*pos; }"));
  glAttachShader(program2, CompileShader(GL_FRAGMENT_SHADER,
    "#version 300 es\n"
    "out highp vec4 outColor;\n"
    "void main() { outColor = vec4(0,0,0,1); }"));
  glLinkProgram(program2);
  assert(glGetError() == GL_NO_ERROR && "Shader program link failed");

  assert(glGetUniformLocation(program2, "world") == 2);
  assert(glGetUniformLocation(program2, "view") == 1);

  printf("Test passed!\n");
  return 0;
}
