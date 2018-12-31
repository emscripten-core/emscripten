#include <GL/glew.h>
#define NO_SDL_GLEXT
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <emscripten.h>

#include "test_webgl_context_attributes_common.c"

int main(int argc, char *argv[]) {
    
    checkContextAttributesSupport(); 
    
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#ifdef AA_ACTIVATED
    antiAliasingActivated = true;
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
#else
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
#endif
    
#ifdef DEPTH_ACTIVATED
    depthActivated = true;
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
#else
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
#endif
    
#ifdef STENCIL_ACTIVATED
    stencilActivated = true;
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#else
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
#endif

#ifdef ALPHA_ACTIVATED
    alphaActivated = true;
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
#else
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
#endif
    
    SDL_Surface *screen = SDL_SetVideoMode(WINDOWS_SIZE, WINDOWS_SIZE, 32, SDL_OPENGL);
       
    glewInit();
    initGlObjects();
    
    draw();
        
    REPORT_RESULT();
    
    return 0;
}
