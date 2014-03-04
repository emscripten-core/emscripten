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

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_opengl.h"

#include <stdio.h>
#include <string.h>

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

    glEnable( GL_TEXTURE_2D ); // Needed when we're using the fixed-function pipeline.

    glViewport( 0, 0, 640, 480 );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix(); // just for testing
    glLoadIdentity();

    glOrtho( 0, 640, 480, 0, -1, 1 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // Load the OpenGL texture

    GLuint texture; // Texture object handle
    SDL_Surface *surface; // Gives us the information to make the texture

    if ( (surface = IMG_Load("screenshot.png")) ) {

        // Check that the image's width is a power of 2
        if ( (surface->w & (surface->w - 1)) != 0 ) {
            printf("warning: image.bmp's width is not a power of 2\n");
        }

        // Also check if the height is a power of 2
        if ( (surface->h & (surface->h - 1)) != 0 ) {
            printf("warning: image.bmp's height is not a power of 2\n");
        }

        // Have OpenGL generate a texture object handle for us
        glGenTextures( 1, &texture );

        // Bind the texture object
        glBindTexture( GL_TEXTURE_2D, texture );

        // Set the texture's stretching properties
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        //SDL_LockSurface(surface);

        // Add some greyness
        memset(surface->pixels, 0x66, surface->w*surface->h);

        // Edit the texture object's image data using the information SDL_Surface gives us
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );

        //SDL_UnlockSurface(surface);
    }
    else {
        printf("SDL could not load image.bmp: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Free the SDL_Surface only if it was successfully created
    if ( surface ) {
        SDL_FreeSurface( surface );
    }

    // Clear the screen before drawing
    glClear( GL_COLOR_BUFFER_BIT );

    // Bind the texture to which subsequent calls refer to
    glBindTexture( GL_TEXTURE_2D, texture );

    glBegin( GL_QUADS );
        glTexCoord2i( 0, 0 ); glVertex3f( 10, 10, 0 );
        glTexCoord2i( 1, 0 ); glVertex3f( 300, 10, 0 );
        glTexCoord2i( 1, 1 ); glVertex3f( 300, 128, 0 );
        glTexCoord2i( 0, 1 ); glVertex3f( 10, 128, 0 );

        glTexCoord2f( 0, 0.5 ); glVertex3f( 410, 10, 0 );
        glTexCoord2f( 1, 0.5 ); glVertex3f( 600, 10, 0 );
        glTexCoord2f( 1, 1   ); glVertex3f( 630, 200, 0 );
        glTexCoord2f( 0.5, 1 ); glVertex3f( 310, 250, 0 );
    glEnd();

    glBegin( GL_TRIANGLE_STRIP );
        glTexCoord2i( 0, 0 ); glVertex3f( 100, 300, 0 );
        glTexCoord2i( 1, 0 ); glVertex3f( 300, 300, 0 );
        glTexCoord2i( 1, 1 ); glVertex3f( 300, 400, 0 );
        glTexCoord2i( 0, 1 ); glVertex3f( 500, 410, 0 );
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glColor3ub(90, 255, 255);
    glBegin( GL_QUADS );
        glVertex3f( 10, 410, 0 );
        glVertex3f( 300, 410, 0 );
        glVertex3f( 300, 480, 0 );
        glVertex3f( 10, 470, 0 );
    glEnd();

    glBegin( GL_QUADS );
        glColor3f(1.0, 0, 1.0);   glVertex3f( 410, 410, 0 );
        glColor3f(0, 1.0, 0);     glVertex3f( 600, 410, 0 );
        glColor3f(0, 0, 1.0);     glVertex3f( 600, 480, 0 );
        glColor3f(1.0, 1.0, 1.0); glVertex3f( 410, 470, 0 );
    glEnd();

    SDL_GL_SwapBuffers();

#ifndef __EMSCRIPTEN__
    // Wait for 3 seconds to give us a chance to see the image
    SDL_Delay(3000);
#endif

    // Now we can delete the OpenGL texture and close down SDL
    glDeleteTextures( 1, &texture );

    SDL_Quit();

    return 0;
}
