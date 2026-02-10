// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>
#include <string.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

GLuint program;

#define PIX_C(x, y) ((x)/256.0f + (y)/256.0f)
#define CLAMP(c) ((c) < 0.f ? 0.f : ((c) > 1.f ? 1.f : (c)))
#define PIX(x, y) CLAMP(PIX_C(x, y))

void draw()
{
  int w, h;
  emscripten_get_canvas_element_size("#canvas", &w, &h);
  float xs = (float)h / w;
  float ys = 1.0f;
  float mat[] = { xs, 0, 0, 0, 0, ys, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
  glUniformMatrix4fv(glGetUniformLocation(program, "mat"), 1, 0, mat);
  glClearColor(0,0,1,1);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  unsigned char* imageData = (unsigned char*)malloc(256*256*4*sizeof(unsigned char));
  glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
  for(int y = 0; y < 256; ++y)
    for(int x = 0; x < 256; ++x)
    {
      unsigned char red = imageData[(y*256+x)*4];
      float expectedRed = PIX(x, y);
      unsigned char eRed = (unsigned char)(expectedRed * 255.0f);
      assert(abs((int)eRed - red) <= 2);
    }
  emscripten_cancel_main_loop();
  printf("Test successful!\n");
#if _REENTRANT
  // In PROXY_TO_PTHREAD mode its currently not enough to cancel the main
  // loop and have the application exit:
  // https://github.com/emscripten-core/emscripten/issues/18773
  exit(0);
#endif
}

int main()
{
  emscripten_set_canvas_element_size("#canvas", 256, 256);
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.alpha = attr.depth = attr.stencil = attr.antialias = attr.preserveDrawingBuffer = attr.failIfMajorPerformanceCaveat = 0;
  attr.enableExtensionsByDefault = 1;
  attr.premultipliedAlpha = 0;
  attr.majorVersion = 1;
  attr.minorVersion = 0;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  assert(vs);
  const char *vss = "attribute vec4 vPosition; uniform mat4 mat; varying vec2 texCoord; void main() { gl_Position = vPosition; texCoord = (vPosition.xy + vec2(1.0)) * vec2(0.5); }";
  glShaderSource(vs, 1, &vss, 0);
  glCompileShader(vs);
  GLint isCompiled = 0;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &isCompiled);
  if (!isCompiled)
  {
    GLint maxLength = 0;
    glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);
    char *buf = malloc(maxLength);
    glGetShaderInfoLog(vs, maxLength, &maxLength, buf);
    printf("%s\n", buf);
    return 1;
  }

  GLuint ps = glCreateShader(GL_FRAGMENT_SHADER);
  const char *pss = "precision lowp float; varying vec2 texCoord; uniform vec3 colors[3]; uniform sampler2D tex; void main() { gl_FragColor = texture2D(tex, texCoord); }";
  glShaderSource(ps, 1, &pss, 0);
  glCompileShader(ps);
  glGetShaderiv(ps, GL_COMPILE_STATUS, &isCompiled);
  if (!isCompiled)
  {
    GLint maxLength = 0;
    glGetShaderiv(ps, GL_INFO_LOG_LENGTH, &maxLength);
    char *buf = malloc(maxLength);
    glGetShaderInfoLog(ps, maxLength, &maxLength, buf);
    printf("%s\n", buf);
    return 1;
  }

  program = glCreateProgram();
  assert(program);
  glAttachShader(program, vs);
  glAttachShader(program, ps);
  glBindAttribLocation(program, 0, "vPosition");
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &isCompiled);
  if (!isCompiled)
  {
    GLint maxLength = 0;
    glGetShaderiv(program, GL_INFO_LOG_LENGTH, &maxLength);
    char *buf = malloc(maxLength);
    glGetProgramInfoLog(program, maxLength, &maxLength, buf);
    printf("%s\n", buf);
    return 1;
  }

  glUseProgram(program);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  assert(vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  float verts[] = { -1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1 };
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, 0, sizeof(float)*2, 0);
  glEnableVertexAttribArray(0);
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  float* texData = (float*)malloc(256*256*sizeof(float));
  for(int y = 0; y < 256; ++y)
    for(int x = 0; x < 256; ++x)
    {
      texData[y*256+x] = PIX(x, y);
    }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 256, 0, GL_LUMINANCE, GL_FLOAT, texData);
  emscripten_set_main_loop(draw, 0, 0);
  return 0;
}
