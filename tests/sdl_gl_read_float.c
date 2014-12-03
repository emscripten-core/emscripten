#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_opengl.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int width = 512; 
int height = 512; 

void FloatReadTest() {

  float *data = malloc(width*height*4*sizeof(float));

  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  GLuint tex, fb;
  glClearColor( 1.0, 0.5, 0.0,1.0);
  glClear( GL_COLOR_BUFFER_BIT ); 
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  glGenFramebuffers(1, &fb);
  glBindFramebuffer(GL_FRAMEBUFFER, fb);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

  GLenum fbstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, width, height);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, data);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fb);
  glDeleteTextures(1, &tex);

  printf(" Pixel 0: %f %f %f %f, should be: 1.0 0.5 0.0 1.0\n", data[0], data[1], data[2],data[3]);
  
 }

int main(int argc, char *argv[])
{
  SDL_Surface *screen;
  if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  screen = SDL_SetVideoMode(width, height, 32, SDL_OPENGL);
  if (!screen) {
    printf("Unable to set video mode: %s\n", SDL_GetError());
    return 1;
  }

  FloatReadTest();
  return 0;
}

