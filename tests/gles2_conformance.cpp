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
    // The spec is somewhat vague here, equally as good could be to return GL_INVALID_ENUM here.
    i = 123;
    glGetIntegerv(GL_SHADER_BINARY_FORMATS, &i);
    assert(glGetError() == GL_NO_ERROR);
    assert(i == 0);

    // Spec does not say what to report on the following, but since GL_SHADER_BINARY_FORMATS is supposed
    // to return a a pointer to an array representing a list, the pointer can't be converted to bool or float,
    // so report a GL_INVALID_ENUM.
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
    REPORT_RESULT();
#endif
}
