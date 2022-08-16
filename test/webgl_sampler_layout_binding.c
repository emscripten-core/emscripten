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
#include <stdlib.h>

GLuint CompileShader(GLenum type, const char *src)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);
  assert(glGetError() == GL_NO_ERROR && "Shader compilation failed!");

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
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);

  GLuint vs = CompileShader(GL_VERTEX_SHADER,
    "attribute vec4 pos;\n"
    "varying vec2 pUv;\n"
    "void main() { pUv = pos.xy; gl_Position = pos; }");

  GLuint ps = CompileShader(GL_FRAGMENT_SHADER,
    "precision lowp float;\n"
    "layout(binding = 4) uniform sampler2D tex;\n"
    "varying vec2 pUv;\n"
    "void main() { gl_FragColor = texture2D(tex, pUv); }");

  GLuint program = CreateProgram(vs, ps);
  glUseProgram(program);

  GLuint tex[2];
  glGenTextures(2, tex);

  glActiveTexture(GL_TEXTURE0 + 4);
  glBindTexture(GL_TEXTURE_2D, tex[0]);
  uint32_t color[4] = {
    0xFF0000FF,
    0xFF00FF00,
    0xFFFF0000,
    0xFFFFFFFF,
  };
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, color);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glActiveTexture(GL_TEXTURE0 + 5);
  glBindTexture(GL_TEXTURE_2D, tex[1]);
  uint32_t color2[4] = {
    0xFFFF0000,
    0xFFFF00FF,
    0xFF00FF00,
    0xFF000000,
  };
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, color2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

  glClearColor(0.3f,0.3f,0.3f,1);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  uint8_t data[4];
  glReadPixels(160, 85, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
  printf("output color: %u, %u, %u, %u\n", data[0], data[1], data[2], data[3]);
  assert(data[0] == 255);
  assert(data[1] == 0);
  assert(data[2] == 0);
  assert(data[3] == 255);

  GLuint ps2 = CompileShader(GL_FRAGMENT_SHADER,
    "precision lowp float;\n"
    "layout(binding = 4) uniform sampler2D tex[2];\n"
    "varying vec2 pUv;\n"
    "void main() { gl_FragColor = texture2D(tex[0], pUv) + texture2D(tex[1], pUv); }");

  GLuint program2 = CreateProgram(vs, ps2);
  glUseProgram(program2);

  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glReadPixels(160, 85, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
  printf("output 2 color: %u, %u, %u, %u\n", data[0], data[1], data[2], data[3]);
  assert(data[0] == 255);
  assert(data[1] == 0);
  assert(data[2] == 255);
  assert(data[3] == 255);

  printf("Test passed!\n");
  return 0;
}
