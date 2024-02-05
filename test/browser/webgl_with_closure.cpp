// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#define GL_GLEXT_PROTOTYPES
#include <assert.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES3/gl3.h>
#include <stdio.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#define GL_CALL( x ) \
    { \
        x; \
        GLenum error = glGetError(); \
        if( error != GL_NO_ERROR ) \
            printf( "GL ERROR: %d,  %s\n", (int)error, #x ); \
    } \


    bool IsFramebufferValid( GLenum target )
    {
        // get framebuffer status from opengl
        GLenum status = glCheckFramebufferStatus( target );
        
        // debug output of the framebuffer status
        switch( status )
        {
            case GL_FRAMEBUFFER_COMPLETE:
                printf("****  Framebuffer COMPLETE\n");
                break;
            case GL_FRAMEBUFFER_UNDEFINED:
                printf("****  Target is the default framebuffer, but the default framebuffer does not exist.\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                printf("****  Any of the framebuffer attachment points are framebuffer incomplete.\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                printf("****  The framebuffer does not have at least one image attached to it.\n");
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                printf("****  the combination of internal formats of the attached images violates an "
                                "implementation-dependent set of restrictions.\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                printf("****  The value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; "
                                "if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or, if "
                                "the attached images are a mix of renderbuffers and textures, the value of "
                                "GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES."
                                "Or the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached "
                                "textures; or, if the attached images are a mix of renderbuffers and textures, the "
                                "value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures.\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
                printf("****  GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS.\n");
                break;
            default:
                printf("****  IsFramebufferValid()  Unknown validation error: %#08x\n", (uint32_t)status );
                break;
        }
        
        return status == GL_FRAMEBUFFER_COMPLETE;
    }


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
        attrs.majorVersion = 1;
        context = emscripten_webgl_create_context( "#canvas", &attrs );
        assert(context);
        // We did not have WebGL 2, but were able to init WebGL 1? In that case,
        // gracefully skip this test with the current browser not supporting
        // this one.
        printf("Skipping test: WebGL 2.0 is not available.\n");
        return 0;
    }
    emscripten_webgl_make_context_current(context);


    // Textures
    //
    int mips = 10;
    int sizeO = 512;

    unsigned short* data = new unsigned short[ 512 * 512 * 4 ];
    //memset( data, 0, 512 * 512 * 4 );

    // Create texture 1
    GLuint tex1;
    GL_CALL( glGenTextures( 1, &tex1 ) );
    GL_CALL( glBindTexture( GL_TEXTURE_CUBE_MAP, tex1 ) );
    for( int i=0; i<6; ++i )
    {
        GL_CALL( glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA16F, sizeO, sizeO, 0, GL_RGBA, GL_HALF_FLOAT, NULL ) );
    }
    GL_CALL( glGenerateMipmap( GL_TEXTURE_CUBE_MAP ) );

    for( int i=0; i<6; ++i )
        GL_CALL( glTexSubImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, 0, 0, sizeO, sizeO, GL_RGBA, GL_HALF_FLOAT, data ) );

    delete [] data;

    // Create texture 2
    GLuint tex2;
    GL_CALL( glGenTextures( 1, &tex2 ) );
    GL_CALL( glBindTexture( GL_TEXTURE_CUBE_MAP, tex2 ) );
    for( int i=0; i<6; ++i )
    {
        GL_CALL( glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA16F, sizeO, sizeO, 0, GL_RGBA, GL_HALF_FLOAT, NULL ) );
    }
    GL_CALL( glGenerateMipmap( GL_TEXTURE_CUBE_MAP ) );



    // FBOs
    //

    // Create FBO 1
    GLuint fbo1;
    GL_CALL( glGenFramebuffers( 1, &fbo1 ) );
    GL_CALL( glBindFramebuffer( GL_FRAMEBUFFER, fbo1 ) );
    for( int level=0; level<mips; ++level )
    {
        for( int i=0; i<6; ++i )
        {
            GL_CALL( glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, tex1, level ) );
        }
    }
    IsFramebufferValid( GL_FRAMEBUFFER );

    // Create FBO 2
    GLuint fbo2;
    GL_CALL( glGenFramebuffers( 1, &fbo2 ) );
    GL_CALL( glBindFramebuffer( GL_FRAMEBUFFER, fbo2 ) );
    for( int level=0; level<mips; ++level )
    {
        for( int i=0; i<6; ++i )
        {
            GL_CALL( glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, tex2, level ) );

            glViewport( 0, 0, sizeO, sizeO );
        }
    }
    IsFramebufferValid( GL_FRAMEBUFFER );


    // Copy FBO 1 to 2
    int nw = sizeO;
    GL_CALL( glBindFramebuffer( GL_READ_FRAMEBUFFER, fbo1 ) );
    GL_CALL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fbo2 ) );
    for( int level=0; level<mips; ++level )
    {
        for( int i=0; i<6; ++i )
        {
            GL_CALL( glFramebufferTexture2D( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, tex1, level ) );
            GL_CALL( glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, tex2, level ) );

            glViewport( 0, 0, nw, nw );

            GL_CALL( glBlitFramebuffer( 0, 0, nw, nw, 0, 0, nw, nw, GL_COLOR_BUFFER_BIT, GL_NEAREST ) );
        }

        nw /= 2;
    }

    GL_CALL( glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 ) );
    GL_CALL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ) );  

  return 0;
}
