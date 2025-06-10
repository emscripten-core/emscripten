#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(int argc, char *argv[]) {
  SDL_Surface *screen;

  assert(SDL_Init(SDL_INIT_VIDEO) == 0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  screen = SDL_SetVideoMode( 640, 480, 16, SDL_OPENGL);
  assert(screen);

  char *extensions = (char*)glGetString(GL_EXTENSIONS);
  printf("extensions: %s\n", extensions);
  assert(extensions);

  int supported = SDL_GL_ExtensionSupported("foobar");
  assert(!supported);

  char* end = strchr(extensions, ' ');
  *end = '\0';
  printf("testing for extension: %s\n", extensions);
  supported = SDL_GL_ExtensionSupported(extensions);
  printf("supported: %d\n", supported);
  assert(supported);

  SDL_Quit();
  return 0;
}
