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

#include <SDL/SDL.h>

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
    
    typedef struct Color {
        GLubyte r;
        GLubyte g;
        GLubyte b;
        GLubyte a;
    } Color;
    
    typedef struct Vertex {
        GLfloat x;
        GLfloat y;
        Color color;
    } Vertex;
    
    Vertex vertices[18] = {
        {-1.00,  0.0, {0xFF, 0x00, 0xFF, 0xFF}},
        {-1.00,  1.0, {0xFF, 0xFF, 0x00, 0xFF}},
        {-0.75,  0.0, {0xFF, 0x00, 0x00, 0xFF}},
        {-0.75,  1.0, {0xFF, 0xFF, 0xFF, 0xFF}},
        {-0.50,  0.0, {0xFF, 0x00, 0x00, 0xFF}},
        {-0.50,  1.0, {0xFF, 0xFF, 0x00, 0xFF}},
        {-0.25,  0.0, {0xFF, 0x00, 0xFF, 0xFF}},
        {-0.25,  1.0, {0xFF, 0xFF, 0x00, 0xFF}},
        {-0.00,  0.0, {0xFF, 0x00, 0x00, 0xFF}},
        {-0.00,  1.0, {0xFF, 0xFF, 0xFF, 0xFF}},
        { 0.25,  0.0, {0xFF, 0x00, 0x00, 0xFF}},
        { 0.25,  1.0, {0xFF, 0xFF, 0x00, 0xFF}},
        { 0.50,  0.0, {0xFF, 0x00, 0xFF, 0xFF}},
        { 0.50,  1.0, {0xFF, 0xFF, 0x00, 0xFF}},
        { 0.75,  0.0, {0xFF, 0x00, 0x00, 0xFF}},
        { 0.75,  1.0, {0xFF, 0xFF, 0xFF, 0xFF}},
        { 1.00,  0.0, {0xFF, 0x00, 0x00, 0xFF}},
        { 1.00,  1.0, {0xFF, 0xFF, 0x00, 0xFF}}
    };
    
    Vertex vertices2[18] = {
        {-1.00,  -1.0, {0xFF, 0x00, 0xFF, 0xFF}},
        {-1.00,   0.0, {0xFF, 0xFF, 0x00, 0xFF}},
        {-0.75,  -1.0, {0xFF, 0x00, 0x00, 0xFF}},
        {-0.75,   0.0, {0xFF, 0xFF, 0xFF, 0xFF}},
        {-0.50,  -1.0, {0xFF, 0x00, 0x00, 0xFF}},
        {-0.50,   0.0, {0xFF, 0xFF, 0x00, 0xFF}},
        {-0.25,  -1.0, {0xFF, 0x00, 0xFF, 0xFF}},
        {-0.25,   0.0, {0xFF, 0xFF, 0x00, 0xFF}},
        {-0.00,  -1.0, {0xFF, 0x00, 0x00, 0xFF}},
        {-0.00,   0.0, {0xFF, 0xFF, 0xFF, 0xFF}},
        { 0.25,  -1.0, {0xFF, 0x00, 0x00, 0xFF}},
        { 0.25,   0.0, {0xFF, 0xFF, 0x00, 0xFF}},
        { 0.50,  -1.0, {0xFF, 0x00, 0xFF, 0xFF}},
        { 0.50,   0.0, {0xFF, 0xFF, 0x00, 0xFF}},
        { 0.75,  -1.0, {0xFF, 0x00, 0x00, 0xFF}},
        { 0.75,   0.0, {0xFF, 0xFF, 0xFF, 0xFF}},
        { 1.00,  -1.0, {0xFF, 0x00, 0x00, 0xFF}},
        { 1.00,   0.0, {0xFF, 0xFF, 0x00, 0xFF}}
    };
    
    //    make a vertex buffer for the second set of vertices
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    
    
    //    bind to it
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //    send it to gl
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_READ); // GL_STATIC_READ is not in WebGL!
    
    //    unbind from it
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    

    // DRAW
    
    // Clear the screen before drawing
    glClear( GL_COLOR_BUFFER_BIT );
    
    // This test ensures that we can use two separate arrays in memory for different
    // attributes, and that they each can have different stride.
    // The first test shows implicit striding (the zero indicates tightly packed)
    // The second test shows explicit striding where the stride is passed in
    //   even though it also is tightly packed
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    // TEST 1 - clientside data
    
    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vertices);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &vertices[0].color);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    glDrawArrays(GL_TRIANGLE_STRIP, 10, 3);
    

    
    // TEST 2 - bind to array buffer, gl*Pointer calls are offsets into the buffer, which was previously uploaded to
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), 0);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), (GLvoid*)((GLvoid*)&vertices2[0].color - (GLvoid*)&vertices2[0]));
    
//    gldrawarrays first with a low number of vertices, then with a high number
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    glDrawArrays(GL_TRIANGLE_STRIP, 10, 3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    SDL_GL_SwapBuffers();
    
#ifndef __EMSCRIPTEN__
    // Wait for 3 seconds to give us a chance to see the image
    SDL_Delay(3000);
#endif
    
    SDL_Quit();
    
    return 0;
}
