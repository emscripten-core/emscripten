// Built from glbook/hello triange and sdl_ogl, see details there

#include "SDL2/SDL.h"
#include "SDL2/SDL_opengles2.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

GLuint programObject;
int width = 512;
int height = 256;

GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   shader = glCreateShader ( type );
   if ( shader == 0 )
   	return 0;

   glShaderSource ( shader, 1, &shaderSrc, NULL );
   glCompileShader ( shader );
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );
   if ( !compiled ) 
   {
      GLint infoLen = 0;
      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      if ( infoLen > 1 )
      {
         char* infoLog = malloc (sizeof(char) * infoLen );
         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         printf ( "Error compiling shader:\n%s\n", infoLog );            
         free ( infoLog );
      }
      glDeleteShader ( shader );
      return 0;
   }
   return shader;
}

int Init ()
{
   GLbyte vShaderStr[] =  
      "attribute vec4 vPosition;    \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "}                            \n";
   
   GLbyte fShaderStr[] =  
      "precision mediump float;\n"\
      "void main()                                  \n"
      "{                                            \n"
      "  gl_FragColor = vec4 ( 0.0, 0.0, 1.0, 1.0 );\n"
      "}                                            \n";

   GLuint vertexShader;
   GLuint fragmentShader;
   GLint linked;

   vertexShader = LoadShader ( GL_VERTEX_SHADER, vShaderStr );
   fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fShaderStr );

   programObject = glCreateProgram ( );
   if ( programObject == 0 )
      return 0;

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );
   glBindAttribLocation ( programObject, 0, "vPosition" );
   glLinkProgram ( programObject );
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );
   if ( !linked ) 
   {
      GLint infoLen = 0;
      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      if ( infoLen > 1 )
      {
         char* infoLog = malloc (sizeof(char) * infoLen );
         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         printf ( "Error linking program:\n%s\n", infoLog );            
         free ( infoLog );
      }
      glDeleteProgram ( programObject );
      return GL_FALSE;
   }

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ()
{
   GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f, 
                           -0.5f, -0.5f, 0.0f,
                            0.5f, -0.5f, 0.0f };

   // No clientside arrays, so do this in a webgl-friendly manner
   GLuint vertexPosObject;
   glGenBuffers(1, &vertexPosObject);
   glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
   glBufferData(GL_ARRAY_BUFFER, 9*4, vVertices, GL_STATIC_DRAW);
   
   glViewport ( 0, 0, width, height );
   glClear ( GL_COLOR_BUFFER_BIT );
   glUseProgram ( programObject );

   glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
   glVertexAttribPointer(0 /* ? */, 3, GL_FLOAT, 0, 0, 0);
   glEnableVertexAttribArray(0);

   glDrawArrays ( GL_TRIANGLES, 0, 3 );
}

void Verify() {
  unsigned char *data = malloc(width*height*4 + 16);
  int *last = (int*)(data + width*height*4 - 4);
  int *after = (int*)(data + width*height*4);
  *last = 0xdeadbeef;
  *after = 0x12345678;
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
  assert(*last != 0xdeadbeef); // should overwrite the buffer to the end
  assert(*after == 0x12345678); // nothing should be written afterwards!
  // Should see some blue, and nothing else
  int seen = 0;
  int ok = 1;
  for (int x = 0; x < width*height; x++) {
    seen = seen || data[x*4+2] != 0;
    ok = ok && (data[x*4+0] == 0);
    ok = ok && (data[x*4+1] == 0);
  }
  int result = seen && ok;
  REPORT_RESULT();
}

int main(int argc, char *argv[])
{
  SDL_Window *window;
  SDL_GLContext context;

  if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow("sdl_gl_read", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);

  if (!window) {
    printf("Unable to create window: %s\n", SDL_GetError());
    return 1;
  }

  context = SDL_GL_CreateContext(window);

  Init();
  Draw();
  Verify();

  return 0;
}

