#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_opengl.h"

int main(int argc, char **argv) {
  int expected = atoi(argv[1]);
  // Functions that exist only in desktop OpenGL, and not in OpenGL ES or WebGL.
  const char *names[] = { "glBegin", "glClearDepth", "glDepthRange" };
  for (int i = 0; i < 3; ++i) {
    int proc = !!SDL_GL_GetProcAddress(names[i]);
    printf("%s exists: %d, expected: %d\n", names[i], proc, expected);
    assert(proc == expected);
  }
  printf("done\n");
  return 0;
}

