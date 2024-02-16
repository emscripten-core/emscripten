// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdlib.h>
#include <cassert>
#include <cstdio>
#include <emscripten.h>
#include <emscripten/html5.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES3/gl3.h>

int result = 0;
GLuint sampleQuery = 0;

#define GL_CALL( x ) \
    { \
        x; \
        GLenum error = glGetError(); \
        if( error != GL_NO_ERROR ) { \
            printf( "GL ERROR: %d,  %s\n", (int)error, #x ); \
            result = 1; \
        } \
    } \

void getQueryResult()
{
  /* Get the result. It should be nonzero. */
  GLuint any;
  GL_CALL(glGetQueryObjectuiv(sampleQuery, GL_QUERY_RESULT, &any));

  if (!any) return;

  printf("queried any samples passed: %u\n", any);
  emscripten_cancel_main_loop();

  GL_CALL(glDeleteQueries(1, &sampleQuery));

  // result == 0 signals success
  exit(result);
}

GLuint compile_shader(GLenum shaderType, const char *src)
{
  GLuint shader = glCreateShader(shaderType);
  GL_CALL(glShaderSource(shader, 1, &src, NULL));
  GL_CALL(glCompileShader(shader));

  GLint isCompiled = 0;
  GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled));
  assert(isCompiled);
  return shader;
}

GLuint create_program(GLuint vert, GLuint frag)
{
   GLuint program = glCreateProgram();
   GL_CALL(glAttachShader(program, vert));
   GL_CALL(glAttachShader(program, frag));
   GL_CALL(glBindAttribLocation(program, 0, "position"));
   GL_CALL(glLinkProgram(program));
   return program;
}

int main() {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);

  attrs.enableExtensionsByDefault = 1;
  attrs.majorVersion = 2;
  attrs.minorVersion = 0;

  /* Skip WebGL 2 tests if not supported */
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
  if (!context) {
    printf("Skipped: WebGL 2 is not supported.\n");
    return 0;
  }
  emscripten_webgl_make_context_current(context);

  /* Generate a query */
  GL_CALL(glGenQueries(2, &sampleQuery));
  assert(sampleQuery);

  /* Begin it */
  GL_CALL(glBeginQuery(GL_ANY_SAMPLES_PASSED, sampleQuery));

  /* Now, `sampleQuery` should be a query */
  bool isQuery = false;
  GL_CALL(isQuery = glIsQuery(sampleQuery));
  assert(isQuery);

  /* Do a draw to actually have something in the queue */
  GLuint vert = compile_shader(GL_VERTEX_SHADER,
    "#version 300 es\n"
    "in vec4 position;\n"
    "void main() {\n"
    "  gl_Position = position;\n"
    "}\n");
  GLuint frag = compile_shader(GL_FRAGMENT_SHADER,
    "#version 300 es\n"
    "precision lowp float;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "  color = vec4(1.0, 0.3, 0.1, 1.0);\n"
    "}\n");
  GLuint program = create_program(vert, frag);
  GL_CALL(glUseProgram(program));
  const float positions[]{
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
  };
  GLuint vertices;
  GL_CALL(glGenBuffers(1, &vertices));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vertices));
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW));
  GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, 0));
  GL_CALL(glEnableVertexAttribArray(0));
  GL_CALL(glClearColor(0.5f, 0.7f, 0.9f, 1.0f));
  GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
  GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));

  /* End the query after the draw */
  GL_CALL(glEndQuery(GL_ANY_SAMPLES_PASSED));

  /* In WebGL, query results are available only in the next event loop
     iteration, so this should return false */
  GLuint isAvailable;
  GL_CALL(glGetQueryObjectuiv(sampleQuery, GL_QUERY_RESULT_AVAILABLE, &isAvailable));
  assert(!isAvailable);

  /* Run the main loop to get the result */
  emscripten_set_main_loop(getQueryResult, 0, 0);

  return 99;
}
