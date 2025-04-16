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
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int hasext(const char *exts, const char *ext) // from cube2, zlib licensed
{
    int len = strlen(ext);
    if(len) for(const char *cur = exts; (cur = strstr(cur, ext)); cur += len)
    {
        if((cur == exts || cur[-1] == ' ') && (cur[len] == ' ' || !cur[len])) return 1;
    }
    return 0;
}

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

    // Check extensions

    const char *exts = (const char *)glGetString(GL_EXTENSIONS);
    assert(hasext(exts, "GL_ARB_texture_compression"));
    assert(hasext(exts, "GL_EXT_texture_compression_s3tc"));

    // Set the OpenGL state after creating the context with SDL_SetVideoMode

    glClearColor( 0, 0, 0, 0 );

    glEnable( GL_TEXTURE_2D ); // Needed when we're using the fixed-function pipeline.

    glViewport( 0, 0, 640, 480 );

    glMatrixMode( GL_PROJECTION );
    GLfloat matrixData[] = { 2.0/640,        0,  0,  0,
                                   0, -2.0/480,  0,  0,
                                   0,        0, -1,  0,
                                  -1,        1,  0,  1 };
    glLoadMatrixf(matrixData); // test loadmatrix

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();


    // Load the OpenGL texture

    GLuint texture;

    #define DDS_SIZE 262272
    FILE *dds = fopen("screenshot.dds", "rb");
    char *ddsdata = (char*)malloc(512*512*4);//DDS_SIZE);
    assert(fread(ddsdata, 1, DDS_SIZE, dds) == DDS_SIZE);
    fclose(dds);

    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );

    assert(!glGetError());
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 512, 512, 0, DDS_SIZE-128, ddsdata+128);
    assert(!glGetError());

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );


    // Prepare and Render

    // Clear the screen before drawing
    glClear( GL_COLOR_BUFFER_BIT );

    // Bind the texture to which subsequent calls refer to
    glBindTexture( GL_TEXTURE_2D, texture );

    // Use clientside vertex pointers to render two items
    GLfloat vertexData[] = { 0, 0, 10, 10, // texture2, position2
                             1, 0, 300, 10,
                             1, 1, 300, 128,
                             0, 1, 10, 128,
                             0, 0.5, 410, 10,
                             1, 0.5, 600, 10,
                             1, 1, 630, 200,
                             0.5, 1, 310, 250 };

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 4*4, &vertexData[0]);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 4*4, &vertexData[2]);

    glDrawArrays(GL_QUADS, 0, 8);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    // Render the last item using oldschool glBegin etc
    glBegin( GL_TRIANGLE_STRIP );
        glTexCoord2i( 0, 0 ); glVertex3f( 100, 300, 0 );
        glTexCoord2i( 1, 0 ); glVertex3f( 300, 300, 0 );
        glTexCoord2i( 1, 1 ); glVertex3f( 300, 400, 0 );
        glTexCoord2i( 0, 1 ); glVertex3f( 500, 410, 0 );
    glEnd();

    SDL_GL_SwapBuffers();

#ifndef __EMSCRIPTEN__
    // Wait for 3 seconds to give us a chance to see the image
    SDL_Delay(1500);
#endif

    // Now we can delete the OpenGL texture and close down SDL
    glDeleteTextures( 1, &texture );

    SDL_Quit();

    return 0;
}
