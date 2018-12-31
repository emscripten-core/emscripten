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
#if !USE_GLEW
#include "SDL/SDL_opengl.h"
#endif

#include <stdio.h>
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
    
    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT );

    // Create a texture

    GLuint texture;
    assert(!glIsTexture(1)); // not a texture
    glGenTextures( 1, &texture );
    assert(!glIsTexture(texture)); // not a texture until glBindTexture
    glBindTexture( GL_TEXTURE_2D, texture );
    assert(glIsTexture(texture)); // NOW it is a texture
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    GLubyte textureData[16*16*4];
    for (int x = 0; x < 16; x++) {
      for (int y = 0; y < 16; y++) {
        *((int*)&textureData[(x*16 + y) * 4]) = x*16 + ((y*16) << 8) + ((y*16) << 16) + 0xff331177;
      }
    }
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, 
                  GL_RGBA, GL_UNSIGNED_BYTE, textureData );

    // BEGIN

#if USE_GLEW
    glewInit();
#endif

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // original: glFrustum(-0.6435469817188064, 0.6435469817188064 ,-0.48266022190470925, 0.48266022190470925 ,0.5400000214576721, 2048);
    // cubegeom: glFrustum(-0.6435469817188064, 0.1435469817188064, -0.48266022190470925, 0.88266022190470925 ,0.5400000214576721, 2048);
    glFrustum(-0.6435469817188064, 0.6435469817188064, -0.48266022190470925, 0.48266022190470925, 0.5400000214576721, 2048);

    glMatrixMode(GL_MODELVIEW);
    GLfloat matrixData[] = { -1, 0, 0, 0,
                              0, 0,-1, 0,
                              0, 1, 0, 0,
                              0, 0, 0, 1 };
    glLoadMatrixf(matrixData);
    // cubegeom glTranslated(-512,-512,-527); // XXX this should be uncommented, but if it is then nothing is shown
    glRotated(0,  0, 1,  0);
    glRotated(0, -1, 0,  0);
    glRotated(0,  0, 0, -1);
    glTranslated(-512,-512,-527);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glClear(GL_DEPTH_BUFFER_BIT);

    GLint ok;

    const char *vertexShader =
        "uniform vec4 center, animstate;\n"
        "uniform vec4 texgenS, texgenT;\n"
        "void main(void)\n"
        "{\n"
        "  vec4 wobble = vec4(gl_Vertex.xyz*(1.0 + 0.5*abs(fract(dot(gl_Vertex.xyz, center.xyz) + animstate.w*0.002) - 0.5)), gl_Vertex.w);\n"
        "  gl_Position = gl_ModelViewProjectionMatrix * wobble;\n"
        "  gl_FrontColor = gl_Color;\n"
        "  gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;\n"
        "  vec2 texgen = vec2(dot(texgenS, gl_Vertex), dot(texgenT, gl_Vertex)); \n"
        "  gl_TexCoord[1].xy = texgen;\n"
        "  gl_TexCoord[2].xy = texgen - animstate.w*0.0005;\n"
        "}\n";
    const char *fragmentShader =
        "uniform sampler2D tex2;\n"
        "uniform sampler2D tex0, tex1;\n"
        "void main(void)\n"
        "{\n"
        "  vec2 dtc = gl_TexCoord[0].xy + texture2D(tex0, gl_TexCoord[2].xy).xy*0.1; \n"
        "  vec4 diffuse = texture2D(tex0, dtc);\n"
        "  vec4 blend = texture2D(tex1, gl_TexCoord[1].xy); \n"
        "  diffuse *= blend.a*4.0; \n"
        "  diffuse.b += 0.5 - blend.a*0.5; \n"
        "  gl_FragColor = diffuse * gl_Color;\n"
        "}\n";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShader, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    assert(ok);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShader, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    assert(ok);

    GLuint program = glCreateProgram();

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    assert(ok);

    glUseProgram(program);

    // get active uniform data for 10 uniforms. XXX they include our additions from shader rewriting! XXX

    // gen texture, we already has one

    GLuint arrayBuffer;
    glGenBuffers(1, &arrayBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    GLubyte arrayBufferData[] = 
{0,0,0,128,0,0,0,0,0,0,128,63,0,0,0,0,0,0,128,63,0,0,0,128,0,0,0,0,0,0,128,63,171,170,170,61,0,0,128,63,0,0,0,128,0,0,0,0,0,0,128,63,171,170,42,62,0,0,128,63,0,0,0,128,0,0,0,0,0,0,128,63,0,0,128,62,0,0,128,63,0,0,0,128,0,0,0,128,0,0,128,63,171,170,170,62,0,0,128,63,0,0,0,128,0,0,0,128,0,0,128,63,85,85,213,62,0,0,128,63,0,0,0,128,0,0,0,128,0,0,128,63,0,0,0,63,0,0,128,63,0,0,0,0,0,0,0,128,0,0,128,63,85,85,21,63,0,0,128,63,0,0,0,0,0,0,0,128,0,0,128,63,171,170,42,63,0,0,128,63,0,0,0,0,0,0,0,128,0,0,128,63,0,0,64,63,0,0,128,63,0,0,0,0,0,0,0,0,0,0,128,63,85,85,85,63,0,0,128,63,0,0,0,0,0,0,0,0,0,0,128,63,171,170,106,63,0,0,128,63,0,0,0,128,0,0,0,0,0,0,128,63,0,0,128,63,0,0,128,63,0,0,0,128,0,0,0,63,215,179,93,63,0,0,0,0,85,85,85,63,0,0,128,190,215,179,221,62,215,179,93,63,171,170,170,61,85,85,85,63,215,179,221,190,0,0,128,62,215,179,93,63,171,170,42,62,85,85,85,63,0,0,0,191,0,48,13,36,215,179,93,63,0,0,128,62,85,85,85,63,215,179,221,190,0,0,128,190,215,179,93,63,171,170,170,62,85,85,85,63,0,0,128,190,215,179,221,190,215,179,93,63,85,85,213,62,85,85,85,63,0,76,163,165,0,0,0,191,215,179,93,63,0,0,0,63,85,85,85,63,0,0,128,62,215,179,221,190,215,179,93,63,85,85,21,63,85,85,85,63,215,179,221,62,0,0,128,190,215,179,93,63,171,170,42,63,85,85,85,63,0,0,0,63,0,200,211,164,215,179,93,63,0,0,64,63,85,85,85,63,215,179,221,62,0,0,128,62,215,179,93,63,85,85,85,63,85,85,85,63,0,0,128,62,215,179,221,62,215,179,93,63,171,170,106,63,85,85,85,63,0,0,0,128,0,0,0,63,215,179,93,63,0,0,128,63,85,85,85,63,0,0,0,128,215,179,93,63,0,0,0,63,0,0,0,0,171,170,42,63,215,179,221,190,0,0,64,63,0,0,0,63,171,170,170,61,171,170,42,63,0,0,64,191,215,179,221,62,0,0,0,63,171,170,42,62,171,170,42,63,215,179,93,191,63,139,116,36,0,0,0,63,0,0,128,62,171,170,42,63,0,0,64,191,215,179,221,190,0,0,0,63,171,170,170,62,171,170,42,63,215,179,221,190,0,0,64,191,0,0,0,63,85,85,213,62,171,170,42,63,83,107,13,166,215,179,93,191,0,0,0,63,0,0,0,63,171,170,42,63,215,179,221,62,0,0,64,191,0,0,0,63,85,85,21,63,171,170,42,63,0,0,64,63,215,179,221,190,0,0,0,63,171,170,42,63,171,170,42,63,215,179,93,63,111,104,55,165,0,0,0,63,0,0,64,63,171,170,42,63,0,0,64,63,215,179,221,62,0,0,0,63,85,85,85,63,171,170,42,63,215,179,221,62,0,0,64,63,0,0,0,63,171,170,106,63,171,170,42,63,0,0,0,128,215,179,93,63,0,0,0,63,0,0,128,63,171,170,42,63,0,0,0,128,0,0,128,63,0,166,17,38,0,0,0,0,0,0,0,63,0,0,0,191,215,179,93,63,0,166,17,38,171,170,170,61,0,0,0,63,215,179,93,191,0,0,0,63,0,166,17,38,171,170,42,62,0,0,0,63,0,0,128,191,0,48,141,36,0,166,17,38,0,0,128,62,0,0,0,63,215,179,93,191,0,0,0,191,0,166,17,38,171,170,170,62,0,0,0,63,0,0,0,191,215,179,93,191,0,166,17,38,85,85,213,62,0,0,0,63,0,76,35,166,0,0,128,191,0,166,17,38,0,0,0,63,0,0,0,63,0,0,0,63,215,179,93,191,0,166,17,38,85,85,21,63,0,0,0,63,215,179,93,63,0,0,0,191,0,166,17,38,171,170,42,63,0,0,0,63,0,0,128,63,0,200,83,165,0,166,17,38,0,0,64,63,0,0,0,63,215,179,93,63,0,0,0,63,0,166,17,38,85,85,85,63,0,0,0,63,0,0,0,63,215,179,93,63,0,166,17,38,171,170,106,63,0,0,0,63,0,0,0,128,0,0,128,63,0,166,17,38,0,0,128,63,0,0,0,63,0,0,0,128,215,179,93,63,0,0,0,191,0,0,0,0,171,170,170,62,215,179,221,190,0,0,64,63,0,0,0,191,171,170,170,61,171,170,170,62,0,0,64,191,215,179,221,62,0,0,0,191,171,170,42,62,171,170,170,62,215,179,93,191,63,139,116,36,0,0,0,191,0,0,128,62,171,170,170,62,0,0,64,191,215,179,221,190,0,0,0,191,171,170,170,62,171,170,170,62,215,179,221,190,0,0,64,191,0,0,0,191,85,85,213,62,171,170,170,62,83,107,13,166,215,179,93,191,0,0,0,191,0,0,0,63,171,170,170,62,215,179,221,62,0,0,64,191,0,0,0,191,85,85,21,63,171,170,170,62,0,0,64,63,215,179,221,190,0,0,0,191,171,170,42,63,171,170,170,62,215,179,93,63,111,104,55,165,0,0,0,191,0,0,64,63,171,170,170,62,0,0,64,63,215,179,221,62,0,0,0,191,85,85,85,63,171,170,170,62,215,179,221,62,0,0,64,63,0,0,0,191,171,170,106,63,171,170,170,62,0,0,0,128,215,179,93,63,0,0,0,191,0,0,128,63,171,170,170,62,0,0,0,128,0,0,0,63,215,179,93,191,0,0,0,0,171,170,42,62,0,0,128,190,215,179,221,62,215,179,93,191,171,170,170,61,171,170,42,62,215,179,221,190,0,0,128,62,215,179,93,191,171,170,42,62,171,170,42,62,0,0,0,191,0,48,13,36,215,179,93,191,0,0,128,62,171,170,42,62,215,179,221,190,0,0,128,190,215,179,93,191,171,170,170,62,171,170,42,62,0,0,128,190,215,179,221,190,215,179,93,191,85,85,213,62,171,170,42,62,0,76,163,165,0,0,0,191,215,179,93,191,0,0,0,63,171,170,42,62,0,0,128,62,215,179,221,190,215,179,93,191,85,85,21,63,171,170,42,62,215,179,221,62,0,0,128,190,215,179,93,191,171,170,42,63,171,170,42,62,0,0,0,63,0,200,211,164,215,179,93,191,0,0,64,63,171,170,42,62,215,179,221,62,0,0,128,62,215,179,93,191,85,85,85,63,171,170,42,62,0,0,128,62,215,179,221,62,215,179,93,191,171,170,106,63,171,170,42,62,0,0,0,128,0,0,0,63,215,179,93,191,0,0,128,63,171,170,42,62,0,0,0,128,0,166,145,38,0,0,128,191,0,0,0,0,0,0,64,37,0,166,17,166,63,69,124,38,0,0,128,191,171,170,170,61,0,0,64,37,63,69,124,166,0,166,17,38,0,0,128,191,171,170,42,62,0,0,64,37,0,166,145,166,122,167,160,11,0,0,128,191,0,0,128,62,0,0,64,37,63,69,124,166,0,166,17,166,0,0,128,191,171,170,170,62,0,0,64,37,0,166,17,166,63,69,124,166,0,0,128,191,85,85,213,62,0,0,64,37,223,207,57,141,0,166,145,166,0,0,128,191,0,0,0,63,0,0,64,37,0,166,17,38,63,69,124,166,0,0,128,191,85,85,21,63,0,0,64,37,63,69,124,38,0,166,17,166,0,0,128,191,171,170,42,63,0,0,64,37,0,166,145,38,55,251,112,140,0,0,128,191,0,0,64,63,0,0,64,37,63,69,124,38,0,166,17,38,0,0,128,191,85,85,85,63,0,0,64,37,0,166,17,38,63,69,124,38,0,0,128,191,171,170,106,63,0,0,64,37,0,0,0,128,0,166,145,38,0,0,128,191,0,0,128,63,0,0,64,37};
    glBufferData(GL_ARRAY_BUFFER, 1820, arrayBufferData, GL_STATIC_DRAW);

    GLuint elementBuffer;
    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    GLubyte elementBufferData[] = 
{0,0,13,0,1,0,1,0,13,0,14,0,1,0,14,0,2,0,2,0,14,0,15,0,2,0,15,0,3,0,3,0,15,0,16,0,3,0,16,0,4,0,4,0,16,0,17,0,4,0,17,0,5,0,5,0,17,0,18,0,5,0,18,0,6,0,6,0,18,0,19,0,6,0,19,0,7,0,7,0,19,0,20,0,7,0,20,0,8,0,8,0,20,0,21,0,8,0,21,0,9,0,9,0,21,0,22,0,9,0,22,0,10,0,10,0,22,0,23,0,10,0,23,0,11,0,11,0,23,0,24,0,11,0,24,0,12,0,12,0,24,0,25,0,24,0,37,0,25,0,25,0,37,0,38,0,23,0,36,0,24,0,24,0,36,0,37,0,22,0,35,0,23,0,23,0,35,0,36,0,21,0,34,0,22,0,22,0,34,0,35,0,20,0,33,0,21,0,21,0,33,0,34,0,19,0,32,0,20,0,20,0,32,0,33,0,18,0,31,0,19,0,19,0,31,0,32,0,17,0,30,0,18,0,18,0,30,0,31,0,16,0,29,0,17,0,17,0,29,0,30,0,15,0,28,0,16,0,16,0,28,0,29,0,14,0,27,0,15,0,15,0,27,0,28,0,13,0,26,0,14,0,14,0,26,0,27,0,26,0,39,0,27,0,27,0,39,0,40,0,27,0,40,0,28,0,28,0,40,0,41,0,28,0,41,0,29,0,29,0,41,0,42,0,29,0,42,0,30,0,30,0,42,0,43,0,30,0,43,0,31,0,31,0,43,0,44,0,31,0,44,0,32,0,32,0,44,0,45,0,32,0,45,0,33,0,33,0,45,0,46,0,33,0,46,0,34,0,34,0,46,0,47,0,34,0,47,0,35,0,35,0,47,0,48,0,35,0,48,0,36,0,36,0,48,0,49,0,36,0,49,0,37,0,37,0,49,0,50,0,37,0,50,0,38,0,38,0,50,0,51,0,50,0,63,0,51,0,51,0,63,0,64,0,49,0,62,0,50,0,50,0,62,0,63,0,48,0,61,0,49,0,49,0,61,0,62,0,47,0,60,0,48,0,48,0,60,0,61,0,46,0,59,0,47,0,47,0,59,0,60,0,45,0,58,0,46,0,46,0,58,0,59,0,44,0,57,0,45,0,45,0,57,0,58,0,43,0,56,0,44,0,44,0,56,0,57,0,42,0,55,0,43,0,43,0,55,0,56,0,41,0,54,0,42,0,42,0,54,0,55,0,40,0,53,0,41,0,41,0,53,0,54,0,39,0,52,0,40,0,40,0,52,0,53,0,52,0,65,0,53,0,53,0,65,0,66,0,53,0,66,0,54,0,54,0,66,0,67,0,54,0,67,0,55,0,55,0,67,0,68,0,55,0,68,0,56,0,56,0,68,0,69,0,56,0,69,0,57,0,57,0,69,0,70,0,57,0,70,0,58,0,58,0,70,0,71,0,58,0,71,0,59,0,59,0,71,0,72,0,59,0,72,0,60,0,60,0,72,0,73,0,60,0,73,0,61,0,61,0,73,0,74,0,61,0,74,0,62,0,62,0,74,0,75,0,62,0,75,0,63,0,63,0,75,0,76,0,63,0,76,0,64,0,64,0,76,0,77,0,76,0,89,0,77,0,77,0,89,0,90,0,75,0,88,0,76,0,76,0,88,0,89,0,74,0,87,0,75,0,75,0,87,0,88,0,73,0,86,0,74,0,74,0,86,0,87,0,72,0,85,0,73,0,73,0,85,0,86,0,71,0,84,0,72,0,72,0,84,0,85,0,70,0,83,0,71,0,71,0,83,0,84,0,69,0,82,0,70,0,70,0,82,0,83,0,68,0,81,0,69,0,69,0,81,0,82,0,67,0,80,0,68,0,68,0,80,0,81,0,66,0,79,0,67,0,67,0,79,0,80,0,65,0,78,0,66,0,66,0,78,0,79,0};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 864, elementBufferData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

    // try to confuse the client state tracker
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 20, 0);
    glTexCoordPointer(2, GL_FLOAT, 20, (void*)12);

    glPushMatrix();
    glTranslated(484.50579833984375, 589.3919067382812, 528.0055541992188);

    GLint texgenSLocation = glGetUniformLocation(program, "texgenS");
    assert(texgenSLocation >= 0);
    GLfloat texgenSData[4] = { 0.31012535095214844, 0.05928778275847435, 0.38769474625587463, 0.5 };
    glUniform4fv(texgenSLocation, 1, texgenSData);

    GLint texgenTLocation = glGetUniformLocation(program, "texgenT");
    assert(texgenTLocation >= 0 && texgenTLocation != texgenSLocation);
    GLfloat texgenTData[4] = { -0.12697559595108032, -0.4524572193622589, 0.17076200246810913, 0.5 };
    glUniform4fv(texgenTLocation, 1, texgenTData);

    GLint centerLocation = glGetUniformLocation(program, "center");
    if (centerLocation >= 0) {
      GLfloat centerData[4] = { 484.50579833984375, 589.3919067382812, 528.0055541992188, 0 };
      glUniform4fv(centerLocation, 1, centerData);
    }

    GLint animstateLocation = glGetUniformLocation(program, "animstate");
    assert(animstateLocation >= 0);
    GLfloat animstateData[4] = { 1, 55, 51, 10709 };
    glUniform4fv(animstateLocation, 1, animstateData);

    glRotated(1529.857142857143, 0.5773502588272095, 0.5773502588272095, 0.5773502588272095);
    glScaled(-55, 55, -55);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture); // XXX this is after setting Pointers, do we miss it? Also, does it not need clientActiveTexture - should we have updated that?
    glActiveTexture(GL_TEXTURE0);
    glColor4f(1, 0.158823529411764705, 0.058823529411764705, 1);

    glDrawElements(GL_TRIANGLES, 432, GL_UNSIGNED_SHORT, 0);

    // END

    SDL_GL_SwapBuffers();

    assert(glIsTexture(texture)); // still a texture
    glDeleteTextures(1, &texture);
    assert(!glIsTexture(texture)); // but not anymore

#ifndef __EMSCRIPTEN__
    SDL_Delay(1500);
#endif

    SDL_Quit();
    
    return 0;
}
