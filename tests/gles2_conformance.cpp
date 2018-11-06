// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "SDL/SDL.h"

#include <GLES2/gl2.h>

#include <stdio.h>
#include <string.h>

int result = 1; // Success
#define assert(x) do { if (!(x)) {result = 0; printf("Assertion failure: %s in %s:%d!\n", #x, __FILE__, __LINE__); } } while(0)

int main(int argc, char *argv[])
{
    SDL_Surface *screen;

    // Slightly different SDL initialization
    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    screen = SDL_SetVideoMode( 640, 480, 16, SDL_OPENGL ); // *changed*
    if ( !screen ) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }

    // Test that code containing functions related to GLES2 binary shader API will successfully compile ad run
    // (will be nonfunctional no-ops since WebGL doesn't have binary shaders)
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vs, 0, 0, 0);
    assert(glGetError() != GL_NO_ERROR);

    // Calling any of glGet() with null pointer should be detected and not crash.
    // Note that native code can crash when passed a null pointer, and the GL spec does not say anything
    // about this, so we spec that Emscripten GLES2 code should generate GL_INVALID_VALUE.
    glGetBooleanv(GL_ACTIVE_TEXTURE, 0);
    assert(glGetError() == GL_INVALID_VALUE);
    glGetIntegerv(GL_ACTIVE_TEXTURE, 0);
    assert(glGetError() == GL_INVALID_VALUE);
    glGetFloatv(GL_ACTIVE_TEXTURE, 0);
    assert(glGetError() == GL_INVALID_VALUE);

    GLboolean b = GL_TRUE;
    GLint i = -1;
    GLfloat f = -1.f;
    glGetBooleanv(GL_NUM_SHADER_BINARY_FORMATS, &b);
    assert(glGetError() == GL_NO_ERROR);
    assert(b == GL_FALSE);
    glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &i);
    assert(glGetError() == GL_NO_ERROR);
    assert(i == 0);
    glGetFloatv(GL_NUM_SHADER_BINARY_FORMATS, &f);
    assert(glGetError() == GL_NO_ERROR);
    assert(f == 0.f);

    // Currently testing that glGetIntegerv(GL_SHADER_BINARY_FORMATS) should be a no-op.
    int formats[10] = { 123 };
    glGetIntegerv(GL_SHADER_BINARY_FORMATS, formats);
    assert(glGetError() == GL_NO_ERROR);
    assert(formats[0] == 123);

    // Converting enums to booleans or floats would be odd, so test that the following report a GL_INVALID_ENUM.
    glGetBooleanv(GL_SHADER_BINARY_FORMATS, &b);
    assert(glGetError() == GL_INVALID_ENUM);    
    glGetFloatv(GL_SHADER_BINARY_FORMATS, &f);
    assert(glGetError() == GL_INVALID_ENUM);

    // Test that we can query for shader compiler support.
    glGetIntegerv(GL_SHADER_COMPILER, &i);
    assert(glGetError() == GL_NO_ERROR);
    assert(i != 0);
    glGetBooleanv(GL_SHADER_COMPILER, &b);
    assert(glGetError() == GL_NO_ERROR);
    assert(b == GL_TRUE);
    glGetFloatv(GL_SHADER_COMPILER, &f);
    assert(glGetError() == GL_NO_ERROR);
    assert(f == 1.f);
    
#ifdef REPORT_RESULT
    REPORT_RESULT(result);
#endif
}
