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

#define MAX(x, y) ((x) > (y) ? (x) : (y))

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

    screen = SDL_SetVideoMode( 600, 600, 16, SDL_OPENGL ); // *changed*
    if ( !screen ) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }

    // Check extensions

    const char *exts = (const char *)glGetString(GL_EXTENSIONS);
    assert(hasext(exts, "GL_EXT_texture_filter_anisotropic"));

    const char *vendor = (const char *)glGetString(GL_VENDOR);
    printf("vendor: %s\n", vendor);

    GLint aniso;
    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    printf("Max anisotropy: %d (using that)\n", aniso);
    assert(aniso >= 4);

    // Set the OpenGL state after creating the context with SDL_SetVideoMode

    glClearColor( 0, 0, 0, 0 );

    glEnable( GL_TEXTURE_2D ); // Needed when we're using the fixed-function pipeline.

    glViewport( 0, 0, 600, 600 );

    glMatrixMode( GL_PROJECTION );
    GLfloat matrixData[] = { 2.0/600,        0,        0,  0,
                                   0, -2.0/600,        0,  0,
                                   0,        0, -2.0/600,  0,
                                  -1,        1,        0,  1 };
    glLoadMatrixf(matrixData); // test loadmatrix

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();


    // Load the OpenGL texture

    GLuint texture, texture2;

    const int DDS_SIZE = 43920;
    FILE *dds = fopen("water.dds", "rb");
    assert(dds);
    char *ddsdata = (char*)malloc(DDS_SIZE);
    assert(fread(ddsdata, 1, DDS_SIZE, dds) == DDS_SIZE);
    fclose(dds);

    {
      glGenTextures( 1, &texture );
      glBindTexture( GL_TEXTURE_2D, texture );

      char *curr = ddsdata + 128;
      int level = 0;
      int w = 512;
      int h = 64;
      while (level < 5) {
        printf("uploading level %d: %d, %d\n", level, w, h);
        assert(!glGetError());
        glCompressedTexImage2D(GL_TEXTURE_2D, level, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, w, h, 0, w*h, curr);
        assert(!glGetError());
        curr += MAX(w, 4)*MAX(h, 4);
        w /= 2;
        h /= 2;
        level++;
      }
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    }
    {
      glGenTextures( 1, &texture2 );
      glBindTexture( GL_TEXTURE_2D, texture2 );

      char *curr = ddsdata + 128;
      int level = 0;
      int w = 512;
      int h = 64;
      while (level < 5) {
        printf("uploading level %d: %d, %d\n", level, w, h);
        assert(!glGetError());
        glCompressedTexImage2D(GL_TEXTURE_2D, level, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, w, h, 0, w*h, curr);
        assert(!glGetError());
        curr += MAX(w, 4)*MAX(h, 4);
        w /= 2;
        h /= 2;
        level++;
      }
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
    }
    {
      assert(!glGetError());
      glBindFramebuffer(GL_RENDERBUFFER, 0);
      assert(glGetError());

      GLint out = 321;
      assert(!glGetError());
      glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &out); // invalid, just test output
      assert(out == 0);
    }

    // Prepare and Render

    // Clear the screen before drawing
    glClear( GL_COLOR_BUFFER_BIT );

    // Bind the texture to which subsequent calls refer to
    int w = 10;
    int n = 15;
    glBindTexture( GL_TEXTURE_2D, texture );
    for (int x = 0; x < n; x++) {
      int start = x*w*2;
      glBegin( GL_TRIANGLES );
        glTexCoord2i( 1, 0 ); glVertex2i( start  ,   0 );
        glTexCoord2i( 0, 0 ); glVertex3f( start+w, 300, 0 );
        glTexCoord2i( 1, 1 ); glVertex3f( start-w, 300, 0 );
      glEnd();
    }
    glBindTexture( GL_TEXTURE_2D, texture2 );
    for (int x = 0; x < n; x++) {
      int start = n*w*2 + x*w*2;
      glBegin( GL_TRIANGLES );
        glTexCoord2i( 1, 0 ); glVertex3f( start  ,   0, 0 );
        glTexCoord2i( 0, 0 ); glVertex3f( start+w, 300, 0 );
        glTexCoord2i( 1, 1 ); glVertex3f( start-w, 300, 0 );
      glEnd();
    }
/*
    int w = 8;
    int n = 20;
    for (int x = 0; x < n; x++) {
      for (int y = 0; y < n*2; y++) {
        glBindTexture( GL_TEXTURE_2D, texture );
        glBegin( GL_TRIANGLE_STRIP );
          glTexCoord2i( 0, 0 ); glVertex3f( x*w,           y*(w), 0 );
          glTexCoord2i( 1, 0 ); glVertex3f( (x+1)*(w-2*y/n),     y*(w), 0 );
          glTexCoord2i( 1, 1 ); glVertex3f( (x+1)*(w-2*y/n), (y+1)*(w), 0 );
          glTexCoord2i( 0, 1 ); glVertex3f( x*w,       (y+1)*(w), 0 );
        glEnd();
        glBindTexture( GL_TEXTURE_2D, texture2 );
        glBegin( GL_TRIANGLE_STRIP );
          glTexCoord2i( 0, 0 ); glVertex3f( n*w + x*w,           y*(w), 0 );
          glTexCoord2i( 1, 0 ); glVertex3f( n*w + (x+1)*(w-2*y/n),     y*(w), 0 );
          glTexCoord2i( 1, 1 ); glVertex3f( n*w + (x+1)*(w-2*y/n), (y+1)*(w), 0 );
          glTexCoord2i( 0, 1 ); glVertex3f( n*w + x*w,       (y+1)*(w), 0 );
        glEnd();
      }
    }
*/
    SDL_GL_SwapBuffers();

#ifndef __EMSCRIPTEN__
    // Wait for 3 seconds to give us a chance to see the image
    SDL_Delay(2000);
#endif

    // Now we can delete the OpenGL texture and close down SDL
    glDeleteTextures( 1, &texture );

    SDL_Quit();

    // check for asm compilation bug with aliased functions with different sigs

    glBegin( GL_TRIANGLE_STRIP );
    void (*f)(int, int) = glVertex2i;
    if ((int)f % 16 == 4) f(5, 7);
    void (*g)(int, int) = glVertex3f;
    if ((int)g % 16 == 4) g(5, 7);
    return (int)f + (int)g;
}

