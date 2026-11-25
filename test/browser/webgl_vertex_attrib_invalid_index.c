// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Tests that calling the vertex attribute array functions with an attribute
// index that is out of range generates GL_INVALID_VALUE, like it does in
// native OpenGL ES, instead of throwing a JavaScript exception.
// See https://github.com/emscripten-core/emscripten/issues/14589
//
// A common way to end up with such an index is to pass on the -1 that
// glGetAttribLocation() returns for an attribute that the shader declares, but
// does not use.
//
// In -sFULL_ES2 and -sFULL_ES3 builds the Emscripten GL library tracks the
// state of each vertex attribute to emulate client-side vertex arrays, so
// there the index needs to be validated by the GL library itself.

#include <assert.h>
#include <stdio.h>
#include <emscripten/html5.h>

#ifdef TEST_WEBGL2
#include <GLES3/gl3.h>
#define VERSION "#version 300 es\n"
#define ATTRIBUTE "in"
#else
#include <GLES2/gl2.h>
#define VERSION
#define ATTRIBUTE "attribute"
#endif

static GLuint compile_shader(GLenum type, const char *src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);
  GLint compiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  assert(compiled && "Shader compile failed");
  return shader;
}

// Calls each of the vertex attribute array functions with the given invalid
// attribute index.
static void test_invalid_index(GLuint index) {
  printf("Testing invalid vertex attribute index %d\n", (int)index);

  glEnableVertexAttribArray(index);
  assert(glGetError() == GL_INVALID_VALUE);
  // glGetError() clears the error.
  assert(glGetError() == GL_NO_ERROR);

  glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, 0, 0);
  assert(glGetError() == GL_INVALID_VALUE);

#ifdef TEST_WEBGL2
  glVertexAttribIPointer(index, 2, GL_INT, 0, 0);
  assert(glGetError() == GL_INVALID_VALUE);
#endif

  glDisableVertexAttribArray(index);
  assert(glGetError() == GL_INVALID_VALUE);
}

int main() {
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
#ifdef TEST_WEBGL2
  attr.majorVersion = 2;
#endif
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  assert(ctx > 0);
  emscripten_webgl_make_context_current(ctx);

  // The attribute 'unused' is declared, but not used, so it is not an active
  // attribute of the linked program.
  GLuint vs = compile_shader(GL_VERTEX_SHADER,
    VERSION
    ATTRIBUTE " vec2 pos;\n"
    ATTRIBUTE " vec2 unused;\n"
    "void main() { gl_Position = vec4(pos, 0.0, 1.0); }\n");

  GLuint fs = compile_shader(GL_FRAGMENT_SHADER,
#ifdef TEST_WEBGL2
    VERSION
    "precision mediump float;\n"
    "out vec4 color;\n"
    "void main() { color = vec4(1.0); }\n");
#else
    "precision mediump float;\n"
    "void main() { gl_FragColor = vec4(1.0); }\n");
#endif

  GLuint program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  GLint linked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  assert(linked && "Program link failed");

  glUseProgram(program);

  GLint loc_pos = glGetAttribLocation(program, "pos");
  GLint loc_unused = glGetAttribLocation(program, "unused");
  assert(loc_pos != -1);
  assert(loc_unused == -1);

  GLint max_vertex_attribs = 0;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);
  assert(max_vertex_attribs >= 8);
  assert(glGetError() == GL_NO_ERROR);

  static const float vertices[] = { -1.f, -1.f, 1.f, -1.f, 0.f, 1.f };
  GLuint vbo;
  glGenBuffers(1, &vbo);

  // Test both when specifying the vertex data from a buffer object, and when
  // no buffer is bound (which is the client-side vertex array code path in
  // -sFULL_ES2 builds).
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  assert(glGetError() == GL_NO_ERROR);
  test_invalid_index(loc_unused);
  test_invalid_index(max_vertex_attribs);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  test_invalid_index(loc_unused);
  test_invalid_index(max_vertex_attribs);

  // The failed calls above did not disturb the state of the valid vertex
  // attributes, so rendering with them still works.
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glEnableVertexAttribArray(loc_pos);
  glVertexAttribPointer(loc_pos, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  assert(glGetError() == GL_NO_ERROR);

  // The highest valid attribute index is still accepted.
  glEnableVertexAttribArray(max_vertex_attribs - 1);
  glDisableVertexAttribArray(max_vertex_attribs - 1);
  assert(glGetError() == GL_NO_ERROR);

  printf("Test passed!\n");
  return 0;
}
