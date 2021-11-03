// Copyright 2018 The Emscripten Authors.  All rights reserved.
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

#include <string>

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

  /* Two textures */
  GLuint tex[2];
  glGenTextures(2, tex);

  /* Verify that unpack buffer binding is not used after the buffer gets destroyed */
  glBindTexture(GL_TEXTURE_2D, tex[0]);
  {
    const int pixels[16] = {}; /* 4*1 all-black RGBA pixels */

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 4*4, pixels, GL_STATIC_DRAW);

    /* This should use the unpack buffer */
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 4, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

    glDeleteBuffers(1, &buffer);

    /* This not anymore */
    GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
  }

  /* Verify that unpack buffer is used for compressed image upload as well */
  const std::string exts = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
  if(exts.find("WEBGL_compressed_texture_s3tc") != std::string::npos)
  {
    printf("WEBGL_compressed_texture_s3tc is supported, testing ...\n");

    glBindTexture(GL_TEXTURE_2D, tex[1]);
    const char pixels[8] = {}; /* 4*4 all-black S3TC DXT1 pixels */

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 8, pixels, GL_STATIC_DRAW);

    /* This should all use the unpack buffer */
    GL_CALL(glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 4, 4, 0, 8, nullptr));
    GL_CALL(glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8, nullptr));

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    /* This not anymore */
    GL_CALL(glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8, pixels));

    glDeleteBuffers(1, &buffer);
  }
  else printf("WEBGL_compressed_texture_s3tc is NOT supported\n");

  glDeleteTextures(2, tex);

  return 0;
}
