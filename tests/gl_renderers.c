/*******************************************************************
 *                                                                 *
 *                        Using SDL With OpenGL                    *
 *                                                                 *
 *                    Tutorial by Kyle Foley (sdw)                 *
 *                                                                 *
 * http://gpwiki.org/index.php/SDL:Tutorials:Using_SDL_with_OpenGL *
 *                                                                 *
 *******************************************************************/

/*
THIS WORK, INCLUDING THE SOURCE CODE, DOCUMENTATION
AND RELATED MEDIA AND DATA, IS PLACED INTO THE PUBLIC DOMAIN.

THE ORIGINAL AUTHOR IS KYLE FOLEY.

THIS SOFTWARE IS PROVIDED AS-IS WITHOUT WARRANTY
OF ANY KIND, NOT EVEN THE IMPLIED WARRANTY OF
MERCHANTABILITY. THE AUTHOR OF THIS SOFTWARE,
ASSUMES _NO_ RESPONSIBILITY FOR ANY CONSEQUENCE
RESULTING FROM THE USE, MODIFICATION, OR
REDISTRIBUTION OF THIS SOFTWARE.
*/

#ifndef __EMSCRIPTEN__
#define USE_GLEW 0
#endif

#if USE_GLEW
#include "GL/glew.h"
#endif

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#if !USE_GLEW
#include "SDL/SDL_opengl.h"
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    SDL_Surface *screen;

    // Slightly different SDL initialization
    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); // *new*

    screen = SDL_SetVideoMode( 640, 480, 16, SDL_OPENGL ); // *changed*
    if ( !screen ) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }

    // Set the OpenGL state after creating the context with SDL_SetVideoMode

    glClearColor( 0, 0, 0, 0 );

#ifndef __EMSCRIPTEN__
    glEnable( GL_TEXTURE_2D ); // Need this to display a texture XXX unnecessary in OpenGL ES 2.0/WebGL
#endif

    glViewport( 0, 0, 640, 480 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // Clear the screen before drawing
    glClear( GL_COLOR_BUFFER_BIT );

    typedef struct Vertex {
        GLfloat x;
        GLfloat y;
    } Vertex;

    typedef struct Color {
        GLubyte r;
        GLubyte g;
        GLubyte b;
        GLubyte a;
    } Color;

    typedef struct Type1 {
        Vertex location;
        Color  color;
    } Type1;

    typedef struct Type2 {
        GLuint unused1;
        Vertex location;
        GLfloat unused2;
        Color  color;
    } Type2;

    Type1 first[3] = {
        {{-1.0, 0.0}, {0xFF, 0x00, 0x00, 0xFF}},
        {{ 0.0, 1.0}, {0x00, 0xFF, 0x00, 0xFF}},
        {{ 1.0, 0.0}, {0x00, 0x00, 0xFF, 0xFF}}
    };

    Type2 second[3] = {
        {0.0, {-1.0,  0.0}, 0.0, {0xFF, 0x00, 0x00, 0xFF}},
        {0.0, { 1.0,  0.0}, 0.0, {0x00, 0x00, 0xFF, 0xFF}},
        {0.0, { 0.0, -1.0}, 0.0, {0x00, 0xFF, 0x00, 0xFF}}};

    // make two vbo objects
    GLuint vbo[2];
    glGenBuffers(2, &vbo[0]);

    // load the first into the context
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

    // allocate enough space for 100 vertices
    glBufferData(GL_ARRAY_BUFFER, sizeof(Type1)*100, NULL, GL_DYNAMIC_DRAW);

    // load the second into the context
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

    // allocate enough space for 100 vertices
    glBufferData(GL_ARRAY_BUFFER, sizeof(Type2)*100, NULL, GL_DYNAMIC_DRAW);

    // DRAW

    GLbyte * pointer;

    // Clear the screen before drawing
    glClear( GL_COLOR_BUFFER_BIT );

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // FIRST
    // load the first into the context
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

    // Load actual data in
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Type1)*3, &first[0]);

    // point to the buffer's location data
    glVertexPointer(2, GL_FLOAT, sizeof(Type1), NULL);

    pointer = (GLbyte*)(((GLbyte*)&first[0].color) - ((GLbyte*)&first[0].location));

    printf("location = %p\n", pointer);
    // point to the buffer's color data
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Type1), pointer);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

    // SECOND

    // load the first into the context
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

    // Load actual data in
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Type2)*3, &second[0]);

    pointer = (GLbyte*)((GLbyte*)&second[0].location - (GLbyte*)&second[0].unused1);

    // point to the buffer's location data
    printf("location = %p\n", pointer);
    glVertexPointer(2, GL_FLOAT, sizeof(Type2), pointer);

    pointer = (GLbyte*)((GLbyte*)&second[0].color - (GLbyte*)&second[0].unused1);

    // point to the buffer's location data
    printf("location = %p\n", pointer);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Type2), pointer);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    for(int i = 0; i <= 2; ++ i)
    {
        SDL_GL_SetSwapInterval(i);
        assert(SDL_GL_GetSwapInterval() == i);
    }

    SDL_GL_SwapBuffers();

#ifndef __EMSCRIPTEN__
    // Wait for 3 seconds to give us a chance to see the image
    SDL_Delay(3000);
#endif

    SDL_Quit();

    return 0;
}
