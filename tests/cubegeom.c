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

#include "GL/glew.h"

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    SDL_Surface *screen;
    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    screen = SDL_SetVideoMode( 640, 480, 16, SDL_OPENGL );
    if ( !screen ) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }
    
    glClearColor( 0, 0, 0, 0 );
    
#if !EMSCRIPTEN
    glEnable( GL_TEXTURE_2D ); // Need this to display a texture XXX unnecessary in OpenGL ES 2.0/WebGL
#endif

    glViewport( 0, 0, 640, 480 );

    glMatrixMode( GL_PROJECTION );
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
    
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );urface gives us
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, 
                      GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
    } 
    else {
        printf("SDL could not load image.bmp: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }    
    
    if ( surface ) { 
        SDL_FreeSurface( surface );
    }
    
    glClear( GL_COLOR_BUFFER_BIT );

    // BEGIN

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-0.6435469817188064, 0.6435469817188064 ,-0.48266022190470925, 0.48266022190470925 ,0.5400000214576721, 2048);
    glMatrixMode(GL_MODELVIEW0);
    GLfloat matrixData[] = { -1, 0, 0, 0,
                              0, 0,-1, 0,
                              0, 1, 0, 0,
                              0, 0, 0, 1 };
    glLoadMatrixf(matrixData);
    glRotated(0, 0, 1, 0);
    glRotated(0,-1, 0, 0);
    glRotated(0, 0, 0,-1);
    glTranslated(-512,-512,-527);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glClear(GL_DEPTH_BUFFER_BIT);

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glActiveTexture(GL_TEXTURE0);

    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, XXXXXXXXXXXXXXX);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, YYYYYYYYYYYYYYYY);

    // sauer vertex data is apparently 0-12: V3F, 12: N1B, 16-24: T2F, 24-28: T2S, 28-32: C4B
    glVertexPointer(3, GL_FLOAT, 32, 0); // all these apply to the ARRAY_BUFFER that is bound
    glTexCoordPointer(2, GL_FLOAT ,32, 16);
    glClientActiveTexture(GL_TEXTURE1);
    glTexCoordPointer(2, GL_SHORT, 32, 24);
    glClientActiveTexture(GL_TEXTURE0); // likely not needed, it is a cleanup
    glNormalPointer(GL_BYTE, 32, 12);
    glColorPointer(4, GL_UNSIGNED_BYTE, 32, 28);

    glBindTexture(GL_TEXTURE_2D, texture); // diffuse?
    glActiveTexture(GL_TEXTURE0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture); // lightmap? should be different
    glActiveTexture(GL_TEXTURE0);

    GLint ok;

    char *vertexShader = "#pragma CUBE2_fog\n"
                         "uniform vec4 texgenscroll;\n"
                         "void main(void)\n"
                         "{\n"
                         "    gl_Position = ftransform();\n"
                         "    gl_TexCoord[0].xy = gl_MultiTexCoord0.xy + texgenscroll.xy;\n"
                         "    gl_TexCoord[1].xy = gl_MultiTexCoord1.xy * 3.051851e-05;\n"
                         "    #pragma CUBE2_shadowmap\n"
                         "    #pragma CUBE2_dynlight\n"
                         "    #pragma CUBE2_water\n"
                         "}\n";
    char *fragmentShader = "uniform vec4 colorparams;\n"
                           "uniform sampler2D diffusemap, lightmap;\n"
                           "void main(void)\n"
                           "{\n"
                           "    vec4 diffuse = texture2D(diffusemap, gl_TexCoord[0].xy);\n"
                           "    vec4 lm = texture2D(lightmap, gl_TexCoord[1].xy);\n"
                           "    #pragma CUBE2_shadowmap lm\n"
                           "    #pragma CUBE2_dynlight lm\n"
                           "    diffuse *= colorparams;\n"
                           "    gl_FragColor = diffuse * lm;\n"
                           "    #pragma CUBE2_water\n"
                           "}\n";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShader, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    assert(ok);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShader, NULL);
    glCompileShader(fs);
    GLint ok;
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    assert(ok);

    GLuint program = glCreateProgram();

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    assert(ok);

    glUseProgram(program);

    GLint lightmapLocation = glGetUniformLocation(program, "lightmap");
    assert(lightmapLocation >= 0);
    glUniform1i(lightmapLocation, 1); // sampler2D? Is it the texture unit?

    GLint diffusemapLocation = glGetUniformLocation(program, "diffusemap");
    assert(diffusemapLocation >= 0);
    glUniform1i(diffusemapLocation, 0); // sampler2D? Is it the texture unit?

    GLint texgenscrollLocation = glGetUniformLocation(program, "texgenscroll");
    assert(texgenscrollLocation >= 0);

    GLint colorparamsLocation = glGetUniformLocation(program, "colorparams");
    assert(colorparamsLocation >= 0);

    GLfloat texgenscrollData[] = { 0, 0, 0, 0 };
    glUniform4fv(texgenscrollLocation, 1, textgenscrollData);

    GLfloat colorparamsData[] = { 2, 2, 2, 1 };
    glUniform4fv(colorparamsLocation, 1, colorparamsData);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 12);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 24);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 36);

    // END

    SDL_GL_SwapBuffers();
    
#if !EMSCRIPTEN
    // Wait for 3 seconds to give us a chance to see the image
    SDL_Delay(3000);
#endif

    // Now we can delete the OpenGL texture and close down SDL
    glDeleteTextures( 1, &texture );
    
    SDL_Quit();
    
    return 0;
}
