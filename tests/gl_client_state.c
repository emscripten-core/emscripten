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

#if !EMSCRIPTEN
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

#define MAX_TEXTURE_UNITS		2


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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_READ);
    
    //    unbind from it
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    

    // DRAW
    
    // Clear the screen before drawing
    glClear( GL_COLOR_BUFFER_BIT );
    
    // TEST
    
    //    bind to it again
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    // This test ensures 
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), 0);

    
    for (unsigned int i = 0; i < MAX_TEXTURE_UNITS; i++)
    {
        glActiveTexture(GL_TEXTURE0 + (i));
        glClientActiveTexture(GL_TEXTURE0 + (i));
		
		// COLORS
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), (GLvoid*)((GLvoid*)&vertices2[0].color - (GLvoid*)&vertices2[0]));
		
        glActiveTexture(GL_TEXTURE0 + (i));
        glClientActiveTexture(GL_TEXTURE0 + (i));
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        
        GLboolean v = GL_FALSE;

        // XXX This should not assert.
        // http://www.opengl.org/sdk/docs/man/xhtml/glGet.xml
        //
        // "NOTES
        // The following parameters return the associated value for the active texture
        // unit: GL_TEXTURE_1D, GL_TEXTURE_BINDING_1D, GL_TEXTURE_2D,
        // GL_TEXTURE_BINDING_2D, GL_TEXTURE_3D and GL_TEXTURE_BINDING_3D."

        glGetBooleanv(GL_TEXTURE_2D, &v);
        
        // XXX This should not assert.
        // because of the call to glEnable(GL_TEXTURE_2D), we should read a 
        // positive bool from this.
        assert(v);
        
        glTexCoordPointer(0, 0, 0, 0);
    }
    
    // back to texture zero.
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);

//    gldrawarrays first with a low number of vertices, then with a high number
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    glDrawArrays(GL_TRIANGLE_STRIP, 10, 3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    SDL_GL_SwapBuffers();
    
#if !EMSCRIPTEN
    // Wait for 3 seconds to give us a chance to see the image
    SDL_Delay(6000);
#endif
    
    SDL_Quit();
    
    return 0;
}
