// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#define GL_GLEXT_PROTOTYPES
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <assert.h>
#include <string.h>

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
  emscripten_set_canvas_element_size( "#canvas", 100, 100 );

  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context( "#canvas", &attrs );
  if (!context)
  {
    printf("Skipped: WebGL is not supported.\n");
    return 0;
  }
  emscripten_webgl_make_context_current(context);

  GLuint shader;
  GL_CALL( shader = glCreateShader(GL_VERTEX_SHADER) );

  GLint value = -97631;
  GL_CALL( glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &value) );
  assert(value == 0);

  const GLchar* tempSource = (const GLchar*)"";
  GL_CALL( glShaderSource(shader, 1, &tempSource, NULL) );

  value = -97631;
  GL_CALL( glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &value) );
  assert(value == 0);

  tempSource = (const GLchar*)"void main() { gl_Position = vec4(0); }";
  GL_CALL( glShaderSource(shader, 1, &tempSource, NULL) );

  value = -97631;
  GL_CALL( glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &value) );
  assert(value == strlen(tempSource) + 1);

  EMSCRIPTEN_RESULT res = emscripten_webgl_destroy_context(context);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);

  return 0;
}
