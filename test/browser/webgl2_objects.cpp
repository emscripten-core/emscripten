// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#define GL_GLEXT_PROTOTYPES
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES3/gl3.h>
#include <stdio.h>
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
  emscripten_set_canvas_element_size( "#canvas", 100, 100 );

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

  // Anisotropy
  //
  GLfloat maxAnisotropy;
  GL_CALL( glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy) );
  printf("max anisotropy: %f\n", maxAnisotropy);
  assert(maxAnisotropy > 1.f);

  // Vertex Arrays
  //
  GLuint vao1;
  GL_CALL( glGenVertexArrays( 1, &vao1 ) );
  GL_CALL( glBindVertexArray( vao1 ) );
  printf( "vao1: %d\n", vao1 );
  assert(vao1 > 0);

  GLint vao2;
  GL_CALL( glGetIntegerv( GL_VERTEX_ARRAY_BINDING, &vao2 ) );
  printf( "vao2: %d\n", vao2 );
  assert(vao2 == vao1);

  // Sampler Objects
  //
  GLuint sampler;
  GL_CALL( glGenSamplers( 1, &sampler ) );
  assert(sampler > 0);

#if 0 // TODO: Disabled due to https://github.com/KhronosGroup/WebGL/issues/2006
  GL_CALL( glSamplerParameteri( sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, (int)maxAnisotropy ) );

  GLfloat maxAnisotropy2;
  GL_CALL( glGetSamplerParameterfv( sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy2 ) );
  printf("max anisotropy: %f\n", maxAnisotropy2);
  assert(maxAnisotropy2 == maxAnisotropy);
#endif

  GL_CALL( glBindSampler( 0, sampler ) );

  GLint sampler2;
  GL_CALL( glGetIntegerv( GL_SAMPLER_BINDING, &sampler2 ) );
  printf( "sampler2: %d\n", sampler2 );
  assert(sampler2 == sampler);

  return 0;
}
