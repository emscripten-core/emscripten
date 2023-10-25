// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>
#include <stdlib.h>

GLuint CompileShader(GLenum type, const char *src)
{
  GLuint shader = glCreateShader(type);
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
  else
  {
    printf("Shader compiled OK!\n");
  }

  return shader;
}

GLuint CreateProgram(GLuint vertexShader, GLuint fragmentShader)
{
   GLuint program = glCreateProgram();
   glAttachShader(program, vertexShader);
   glAttachShader(program, fragmentShader);
   glBindAttribLocation(program, 0, "pos");
   glLinkProgram(program);
   GLint linked = 0;
   glGetProgramiv(program, GL_LINK_STATUS, &linked);
   if (!linked) 
   {
      GLint infoLen = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
      if (infoLen)
      {
        char *infoLog = (char*)malloc(infoLen);
        glGetProgramInfoLog(program, infoLen, NULL, infoLog);
        printf("Error linking program:\n%s\n", infoLog);
      }
      assert(0);
   }
   return program;
}

int main(int argc, char *argv[])
{
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.majorVersion = 2;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  GLuint vs = CompileShader(GL_VERTEX_SHADER,
    "#version 300 es\n"
    "in vec4 pos;\n"
    "void main() { gl_Position = pos; }");

  GLuint ps = CompileShader(GL_FRAGMENT_SHADER,
    "#version 300 es\n"
    "precision lowp float;\n"
    "layout(binding = 4) uniform red { vec4 unused1; float r; };\n"
    "layout(binding = 5, std140) uniform Green { float unused2; float g; } green;\n"
    "layout(std140, binding = 6) uniform Blue { vec3 unused3; float b; } blue[2];\n"
    "out vec4 outColor;\n"
    "void main() { outColor = vec4(r, green.g, blue[0].b + blue[1].b, 1.0); }");

  GLuint program = CreateProgram(vs, ps);
  glUseProgram(program);

  int numBlocks;
  glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);
  for(int i = 0; i < numBlocks; ++i) {
    int size = -1;
    glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
    printf("Uniform block at index %d: size: %d\n", i, size);
  }

  static const float vb[] = {
     -1.0f, -1.0f,
      1.0f, -1.0f,
     -1.0f,  1.0f,
     -1.0f,  1.0f,
      1.0f, -1.0f,
      1.0f,  1.0f,
  };

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vb), vb, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, 0);
  glEnableVertexAttribArray(0);

  struct {
    float unused1[4];
    float r;
    float unused2[3];
  } red = { 0, 0, 0, 0, 0.3f };
  printf("sizeof(red)=%d\n", (int)sizeof(red));

  struct {
    float unused1;
    float g;
    float unused2[2];
  } green = { 0, 0.5f };
  printf("sizeof(green)=%d\n", (int)sizeof(green));

  struct {
    float unused1[3];
    float b;
  } blue[2] = { { 0, 0, 0, 0.3f }, { 0, 0, 0.f, 0.6f } };
  printf("sizeof(blue[0])=%d\n", (int)sizeof(blue[0]));

  GLuint bufs[4];
  glGenBuffers(4, bufs);
  glBindBuffer(GL_UNIFORM_BUFFER, bufs[0]);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(red), &red, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, bufs[1]);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(green), &green, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, bufs[2]);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(blue[0]), blue, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, bufs[3]);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(blue[1]), blue+1, GL_STATIC_DRAW);

  glBindBufferBase(GL_UNIFORM_BUFFER, 4, bufs[0]);
  glBindBufferBase(GL_UNIFORM_BUFFER, 5, bufs[1]);
  glBindBufferBase(GL_UNIFORM_BUFFER, 6, bufs[2]);
  glBindBufferBase(GL_UNIFORM_BUFFER, 7, bufs[3]);
  glClearColor(0.1f,0.1f,0.1f,1);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  uint8_t data[4];
  glReadPixels(160, 85, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
  printf("output color: %u, %u, %u, %u\n", data[0], data[1], data[2], data[3]);
#define DIFF(x, y) ((x)-(y) < 0 ? (y) - (x) : (x) - (y))
  assert(DIFF(data[0], 76) <= 1);
  assert(DIFF(data[1], 127) <= 1);
  assert(DIFF(data[2], 229) <= 1);
  assert(data[3] == 255);

  // Try binding to a negative binding point, should fail:
  assert(!glGetError());
  printf("The following compile should produce an error:\n");
  CompileShader(GL_FRAGMENT_SHADER,
    "#version 300 es\n"
    "precision lowp float;\n"
    "layout(binding = -1) uniform red { float r; };\n"
    "out vec4 outColor;\n"
    "void main() { outColor = vec4(r, r, r, 1.0); }");
  assert(glGetError());

  // Try binding to a too high binding point, should fail:
  GLint bindingPoints = 0;
  glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS, &bindingPoints);
  assert(bindingPoints > 0);
  assert(!glGetError());

  char str[1024];
  sprintf(str, 
    "#version 300 es\n"
    "precision lowp float;\n"
    "layout(binding = %d) uniform red { float r; };\n"
    "out vec4 outColor;\n"
    "void main() { outColor = vec4(r, r, r, 1.0); }", bindingPoints);
  printf("The following compile should produce an error:\n");
  CompileShader(GL_FRAGMENT_SHADER, str);
  assert(glGetError());
  assert(!glGetError());

  // Test that glUseProgram(0) succeeds without errors.
  glUseProgram(0);
  assert(!glGetError());

  printf("Test passed!\n");
  return 0;
}
