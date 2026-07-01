// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#define GL_GLEXT_PROTOTYPES
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES3/gl3.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <assert.h>

#define GL_CALL( x ) \
    { \
        x; \
        GLenum error = glGetError(); \
        if( error != GL_NO_ERROR ) { \
            printf( "GL ERROR: %d,  %s\n", (int)error, #x ); \
            assert(false); \
        } \
    } \


int main()
{
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);

  attrs.enableExtensionsByDefault = 1;
  attrs.majorVersion = 2;
  attrs.minorVersion = 0;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context( "#canvas", &attrs );
  if (!context)
  {
    printf("Skipped: WebGL 2 is not supported.\n");
    return 0;
  }
  emscripten_webgl_make_context_current(context);

  const char *vertexShader =
    "#version 300 es\n"
    "uniform mat4 a;\n"
    "uniform Block1a {\n"
    "  uniform mat4 var1;\n"
    "  uniform vec4 variable2;\n"
    "} block1bb;\n"
    "uniform Block2ccc {\n"
    "  uniform mat4 var1;\n"
    "  uniform vec4 variable2;\n"
    "} block2dddd;\n"
    "void main() {\n"
    "  gl_Position = a * block1bb.var1*block1bb.variable2 + block2dddd.var1*block2dddd.variable2;\n"
    "}\n";

    const char *fragmentShader =
    "#version 300 es\n"
    "precision lowp float;\n"
    "  uniform Block3eeeee {\n"
    "  uniform vec4 var1;\n"
    "  uniform vec4 variable2;\n"
    "} block3ffffff;\n"   // Append characters of different lengths to test name string lengths.
    "out vec4 outColor;\n"
    "void main() {\n"
    "  outColor = block3ffffff.var1 + block3ffffff.variable2;\n"
    "}\n";

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertexShader, NULL);
  glCompileShader(vs);
  int ok = 0;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    printf("Shader compilation error with vertex\n");
    GLint infoLen = 0;
    glGetShaderiv (vs, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1)
    {
       char* infoLog = (char *)malloc(sizeof(char) * infoLen+1);
       glGetShaderInfoLog(vs, infoLen, NULL, infoLog);
       printf("Error compiling shader:\n%s\n", infoLog);
    }
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragmentShader, NULL);
  glCompileShader(fs);
  glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    printf("Shader compilation error with fragment\n");
    GLint infoLen = 0;
    glGetShaderiv (fs, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1)
    {
       char* infoLog = (char *)malloc(sizeof(char) * infoLen+1);
       glGetShaderInfoLog(fs, infoLen, NULL, infoLog);
       printf("Error compiling shader:\n%s\n", infoLog);
    }
  }

  GLuint program = glCreateProgram();

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &ok);
  assert(ok);

  int maxLength = 0;
  glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxLength);
  printf("GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH: %d\n", maxLength);
  assert(maxLength == 12);

  GLint numActiveUniforms = -1;
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
  printf("GL_ACTIVE_UNIFORMS: %d\n", numActiveUniforms);
  assert(numActiveUniforms == 7);

  for(int i = 0; i < numActiveUniforms; ++i)
  {
    char str[256] = {};
    GLsizei length = -1;
    GLint size = -1;
    GLenum type = -1;
    glGetActiveUniform(program, i, 255, &length, &size, &type, str);

    GLint loc = glGetUniformLocation(program, str);

    GLint indx = -1;
    glGetActiveUniformsiv(program, 1, (GLuint*)&i, GL_UNIFORM_BLOCK_INDEX, &indx);

    printf("Active uniform at index %d: %s\n", i, str);
    printf("glGetUniformLocation = %d \t GL_UNIFORM_BLOCK_INDEX = %d \t size = %d \t type = %d\n", loc, indx, size, type);

    assert((loc == -1) != (indx == -1)); // one of them must be true
  }

  GLint numActiveUniformBlocks = -1;
  glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &numActiveUniformBlocks);
  assert(numActiveUniformBlocks == 3);

  // Dump all active uniform buffer blocks of the current program.
  for(int i = 0; i < numActiveUniformBlocks; ++i)
  {
    char str[256] = {};
    GLsizei length = -1;
    glGetActiveUniformBlockName(program, i, 255, &length, str);
    assert(length > 0);
    printf("Active uniform block at index %d: %s\n", i, str);

    GLint param = -1;
#define DUMPUNIFORMBLOCKSTATUS(stat) param = -1; glGetActiveUniformBlockiv(program, i, stat, &param); printf("%s: %d\n", #stat, param); assert(param != -1);
    DUMPUNIFORMBLOCKSTATUS(GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER);
    DUMPUNIFORMBLOCKSTATUS(GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER);
    DUMPUNIFORMBLOCKSTATUS(GL_UNIFORM_BLOCK_BINDING);
    DUMPUNIFORMBLOCKSTATUS(GL_UNIFORM_BLOCK_DATA_SIZE);
    DUMPUNIFORMBLOCKSTATUS(GL_UNIFORM_BLOCK_NAME_LENGTH);
    DUMPUNIFORMBLOCKSTATUS(GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS);
    GLint indices[16] = {};
    glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices);
    for(GLint i = 0; i < param; ++i)
      printf("offset for index %d: %d\n", i, indices[i]);
  }

  return 0;
}
