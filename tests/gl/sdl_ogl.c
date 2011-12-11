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
  Combined with opengl.org tutorial #2,
      http://www.opengl.org/wiki/Tutorial2:_VAOs,_VBOs,_Vertex_and_Fragment_Shaders_%28C_/_SDL%29

  Build with

    gcc sdl_ogl.c -lSDL -lGL

  g++ will fail!

  Or, to JS:

    ~/Dev/emscripten/tools/emmaken.py sdl_ogl.c -o sdl_ogl.o
    ~/Dev/emscripten/emscripten.py sdl_ogl.o > sdl_ogl.js
*/

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include <stdio.h>
#include <stdlib.h>

char* filetobuf(char *file)
{
  FILE *fptr;
  long length;
  char *buf;

  fptr = fopen(file, "r"); /* Open file for reading */
  if (!fptr) /* Return NULL on failure */
      return NULL;
  fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
  length = ftell(fptr); /* Find out how many bytes into the file we are */
  buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
  fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
  fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
  fclose(fptr); /* Close the file */
  buf[length] = 0; /* Null terminator */

  return buf; /* Return the buffer */
}

void drawscene()
{
  int i; /* Simple iterator */
  GLuint vao, vbo[2]; /* Create handles for our Vertex Array Object and two Vertex Buffer Objects */

  /* We're going to create a simple diamond made from lines */
  const GLfloat diamond[4][2] = {
  {  0.0,  1.0  }, /* Top point */
  {  1.0,  0.0  }, /* Right point */
  {  0.0, -1.0  }, /* Bottom point */
  { -1.0,  0.0  } }; /* Left point */

  const GLfloat colors[4][3] = {
  {  1.0,  0.0,  0.0  }, /* Red */
  {  0.0,  1.0,  0.0  }, /* Green */
  {  0.0,  0.0,  1.0  }, /* Blue */
  {  1.0,  1.0,  1.0  } }; /* White */

  /* These pointers will receive the contents of our shader source code files */
  GLchar *vertexsource, *fragmentsource;

  /* These are handles used to reference the shaders */
  GLuint vertexshader, fragmentshader;

  /* This is a handle to the shader program */
  GLuint shaderprogram;

  /* Allocate and assign a Vertex Array Object to our handle */
  glGenVertexArrays(1, &vao);

  /* Bind our Vertex Array Object as the current used object */
  glBindVertexArray(vao);

  /* Allocate and assign two Vertex Buffer Objects to our handle */
  glGenBuffers(2, vbo);

  /* Bind our first VBO as being the active buffer and storing vertex attributes (coordinates) */
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

  /* Copy the vertex data from diamond to our buffer */
  /* 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values */
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), diamond, GL_STATIC_DRAW);

  /* Specify that our coordinate data is going into attribute index 0, and contains two floats per vertex */
  glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  /* Enable attribute index 0 as being used */
  glEnableVertexAttribArray(0);

  /* Bind our second VBO as being the active buffer and storing vertex attributes (colors) */
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

  /* Copy the color data from colors to our buffer */
  /* 12 * sizeof(GLfloat) is the size of the colors array, since it contains 12 GLfloat values */
  glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), colors, GL_STATIC_DRAW);

  /* Specify that our color data is going into attribute index 1, and contains three floats per vertex */
  glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  /* Enable attribute index 1 as being used */
  glEnableVertexAttribArray(1);

  /* Read our shaders into the appropriate buffers */
  vertexsource = filetobuf("tutorial2.vert");
  fragmentsource = filetobuf("tutorial2.frag");

  /* Assign our handles a "name" to new shader objects */
  vertexshader = glCreateShader(GL_VERTEX_SHADER);
  fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);

  /* Associate the source code buffers with each handle */
  glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
  glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);

  /* Compile our shader objects */
  glCompileShader(vertexshader);
  glCompileShader(fragmentshader);

  /* Assign our program handle a "name" */
  shaderprogram = glCreateProgram();

  /* Attach our shaders to our program */
  glAttachShader(shaderprogram, vertexshader);
  glAttachShader(shaderprogram, fragmentshader);

  /* Bind attribute index 0 (coordinates) to in_Position and attribute index 1 (color) to in_Color */
  glBindAttribLocation(shaderprogram, 0, "in_Position");
  glBindAttribLocation(shaderprogram, 1, "in_Color");

  /* Link our program, and set it as being actively used */
  glLinkProgram(shaderprogram);
  glUseProgram(shaderprogram);

  /* Loop our display increasing the number of shown vertexes each time.
   * Start with 2 vertexes (a line) and increase to 3 (a triangle) and 4 (a diamond) */
  for (i=4; i <= 4; i++)
  {
    /* Make our background black */
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Invoke glDrawArrays telling that our data is a line loop and we want to draw 2-4 vertexes */
    glDrawArrays(GL_LINE_LOOP, 0, i);

    /* Swap our buffers to make our changes visible */
    SDL_GL_SwapBuffers();

    /* Sleep for 2 seconds */
    SDL_Delay(2000);
  }

  /* Cleanup all the things we bound and allocated */
  glUseProgram(0);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDetachShader(shaderprogram, vertexshader);
  glDetachShader(shaderprogram, fragmentshader);
  glDeleteProgram(shaderprogram);
  glDeleteShader(vertexshader);
  glDeleteShader(fragmentshader);
  glDeleteBuffers(2, vbo);
  glDeleteVertexArrays(1, &vao);
  free(vertexsource);
  free(fragmentsource);
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

  screen = SDL_SetVideoMode( 512, 384, 32, SDL_OPENGL); // *changed*
  if ( !screen ) {
    printf("Unable to set video mode: %s\n", SDL_GetError());
    return 1;
  }
  
  drawscene();
   
  SDL_Quit();
    
  return 0;
}
