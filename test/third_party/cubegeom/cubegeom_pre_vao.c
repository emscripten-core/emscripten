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
#define USE_GLEW 1
#endif

#if USE_GLEW
#include "GL/glew.h"
#endif

#include "SDL/SDL.h"
#if !USE_GLEW && !USE_REGAL
#include "SDL/SDL_opengl.h"
#endif

#if USE_REGAL
#include "GL/Regal.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    SDL_Surface *screen;
    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    screen = SDL_SetVideoMode( 640, 480, 24, SDL_OPENGL );
    if ( !screen ) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }

#if USE_REGAL
    RegalMakeCurrent((void*)1);
#endif
    
    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT );

    // Create a texture

    GLuint texture;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    GLubyte textureData[16*16*4];
    for (int x = 0; x < 16; x++) {
      for (int y = 0; y < 16; y++) {
        *((int*)&textureData[(x*16 + y) * 4]) = x*16 + ((y*16) << 8);
      }
    }
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, 
                  GL_RGBA, GL_UNSIGNED_BYTE, textureData );

    // Create a second texture

    GLuint texture2;
    glGenTextures( 1, &texture2 );
    glBindTexture( GL_TEXTURE_2D, texture2 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    GLubyte texture2Data[] = { 0xff,    0,    0, 0xff,
                                  0, 0xff,    0, 0xaa,
                                  0,    0, 0xff, 0x55,
                               0x80, 0x90, 0x70,    0 };
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, texture2Data );
    
    // BEGIN

#if USE_GLEW
    glewInit();
#endif

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // original: glFrustum(-0.6435469817188064, 0.6435469817188064 ,-0.48266022190470925, 0.48266022190470925 ,0.5400000214576721, 2048);
    //glFrustum(-0.6435469817188064, 0.1435469817188064 ,-0.48266022190470925, 0.88266022190470925 ,0.5400000214576721, 2048);
    GLfloat pm[] = { 1.372136116027832, 0, 0, 0, 0, 0.7910231351852417, 0, 0, -0.6352481842041016, 0.29297152161598206, -1.0005275011062622, -1, 0, 0, -1.080284833908081, 0 };
    glLoadMatrixf(pm);

    glMatrixMode(GL_MODELVIEW);
    GLfloat matrixData[] = { -1, 0, 0, 0,
                              0, 0,-1, 0,
                              0, 1, 0, 0,
                              0, 0, 0, 1 };
    glLoadMatrixf(matrixData);

    glActiveTexture(GL_TEXTURE0);

    GLuint arrayBuffer, elementBuffer;
    glGenBuffers(1, &arrayBuffer);
    glGenBuffers(1, &elementBuffer);

    GLubyte arrayData[] = {
/*
[0, 0,   0, 67] ==>  128 float
[0, 0, 128, 67] ==>  256 float
[0, 0,   0, 68] ==>  512 float
[0, 0, 128, 68] ==> 1024 float

[vertex x        ] [vertex y         ] [vertex z         ] [nr]                [texture u        ] [texture v        ] [lm u   ] [lm v   ] [color r,g,b,a    ] */
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128, //  0
  0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,  67,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128, //  1
  0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,  67,   0,   0,   0,  67,   0,   0,   0,   0, 128, 128, 128, 128, //  2
  0,   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  67,   0,   0,   0,   0, 128, 128, 128, 128, //  3
  0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,  67,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128, //  4
  0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0, 128,  67,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128, //  5
  0,   0, 128,  68,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0, 128,  67,   0,   0,   0,  67,   0,   0,   0,   0, 128, 128, 128, 128, //  6
  0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,  67,   0,   0,   0,  67,   0,   0,   0,   0, 128, 128, 128, 128, //  7
  0,   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  67,   0,   0,   0,   0, 128, 128, 128, 128, //  8
  0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,  67,   0,   0,   0,  67,   0,   0,   0,   0, 128, 128, 128, 128, //  9
  0,   0,   0,  68,   0,   0, 128,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,  67,   0,   0, 128,  67,   0,   0,   0,   0, 128, 128, 128, 128, // 10
  0,   0,   0,   0,   0,   0, 128,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128,  67,   0,   0,   0,   0, 128, 128, 128, 128, // 11
  0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,  67,   0,   0,   0,  67,   0,   0,   0,   0, 128, 128, 128, 128, // 12
  0,   0, 128,  68,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0, 128,  67,   0,   0,   0,  67,   0,   0,   0,   0, 128, 128, 128, 128, // 13
  0,   0, 128,  68,   0,   0, 128,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0, 128,  67,   0,   0, 128,  67,   0,   0,   0,   0, 128, 128, 128, 128, // 14
  0,   0,   0,  68,   0,   0, 128,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,  67,   0,   0, 128,  67,   0,   0,   0,   0, 128, 128, 128, 128, // 15

  0,   0,   0,  68,   0,   0,   0,  68,   0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0,   0,   0,   0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,   0,   0,   0,   0,  68,   0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0,   0,  68,   0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0,   0,   0,   0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0, 128,  68,   0,   0,   0,  68,   0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0, 128,  68,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0, 128,  68,   0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0,   0,  68,   0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0, 128,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,   0,   0,   0,   0,  68,   0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,   0,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,   0,   0,   0, 128,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0, 128,  68,   0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0, 128,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0,   0,  68,   0,   0,   0,  68,   0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0, 128,  68,   0,   0,   0,  68,   0,   0, 128,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0, 128,  68,   0,   0,   0,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128,
  0,   0, 128,  68,   0,   0, 128,  68,   0,   0,   0,  68,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128
    };

    // Generate a VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    assert(sizeof(arrayData) == 1408);
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrayData), arrayData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLushort elementData[] = { 1, 2, 0, 2, 3, 0, 5, 6, 4, 6, 7, 4, 9, 10, 8, 10, 11, 8, 13, 14, 12, 14, 15, 12 };
    assert(sizeof(elementData) == 48);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementData), elementData, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

    glBindTexture(GL_TEXTURE_2D, texture); // diffuse?
    glActiveTexture(GL_TEXTURE0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2); // lightmap?
    glActiveTexture(GL_TEXTURE0);

    GLint ok;

    const char *vertexShader = "attribute vec4 a_position;\n"
                               "attribute vec4 a_texCoord0;\n"
                               "uniform mat4 u_modelView;\n"
                               "uniform mat4 u_projection;\n"
                               "varying vec4 v_texCoord0;\n"
                               "void main(void)\n"
                               "{\n"
                               "    gl_Position = (u_projection * u_modelView * a_position) + vec4(200, 0, 0, 0);\n"
                               "    v_texCoord0.xy = a_texCoord0.xy/20.0;\n" // added /20 here
                               "}\n";
    const char *fragmentShader = "uniform sampler2D diffusemap;\n"
                                 "varying vec4 v_texCoord0;\n"
                                 "void main(void)\n"
                                 "{\n"
                                 "    vec4 diffuse = texture2D(diffusemap, v_texCoord0.xy);\n"
                                 "    gl_FragColor = diffuse;\n"
                                 "}\n";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShader, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
      printf("Shader compilation error with vertex\n");
      GLint infoLen = 0;
      glGetShaderiv (vs, GL_INFO_LOG_LENGTH, &infoLen);
      if (infoLen > 1)
      {
         char* infoLog = (char *)malloc(sizeof(char) * infoLen+1);
         glGetShaderInfoLog(vs, infoLen, NULL, infoLog);
         printf("Error compiling shader:\n%s\n", infoLog);            
      }
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShader, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
      printf("Shader compilation error with fragment\n");
      GLint infoLen = 0;
      glGetShaderiv (fs, GL_INFO_LOG_LENGTH, &infoLen);
      if (infoLen > 1)
      {
         char* infoLog = (char *)malloc(sizeof(char) * infoLen+1);
         glGetShaderInfoLog(fs, infoLen, NULL, infoLog);
         printf("Error compiling shader:\n%s\n", infoLog);            
      }
    }

    GLuint program = glCreateProgram();

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    assert(ok);

    glUseProgram(program);

    GLint diffusemapLocation = glGetUniformLocation(program, "diffusemap");
    assert(diffusemapLocation >= 0);
    glUniform1i(diffusemapLocation, 0);

    {
      GLfloat data[16];
      glGetFloatv(GL_MODELVIEW_MATRIX, data);
      printf("Modelview: ");
      for (int i = 0; i < 16; i++) printf("%.3f, ", data[i]);
      printf("\n");
      GLint modelViewLocation = glGetUniformLocation(program, "u_modelView");
      assert(modelViewLocation >= 0);
      glUniformMatrix4fv(modelViewLocation, 1, GL_FALSE, data);
    }
    {
      GLfloat data[16];
      glGetFloatv(GL_PROJECTION_MATRIX, data);
      printf("Projection: ");
      for (int i = 0; i < 16; i++) printf("%.3f, ", data[i]);
      printf("\n");
      GLint projectionLocation = glGetUniformLocation(program, "u_projection");
      assert(projectionLocation >= 0);
      glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, data);
    }

    glBindAttribLocation(program, 0, "a_position");
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
    glEnableVertexAttribArray(0);

    glBindAttribLocation(program, 1, "a_texCoord0");
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (void*)16);
    glEnableVertexAttribArray(1);

    // stop recording in the VAO

    glBindVertexArray(0);

    // unbind all the stuff the VAO would save for us, so this is a valid test

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // draw with VAO

    glBindVertexArray(vao);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)12);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*) 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)24);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)36);

    glBindVertexArray(0);

    glDeleteVertexArrays(1, &vao);

    // END

    SDL_GL_SwapBuffers();

#ifndef __EMSCRIPTEN__
    SDL_Delay(1500);
#endif

    SDL_Quit();
    
    return 0;
}
