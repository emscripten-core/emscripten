#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_opengl.h"

int main(int argc, char **argv) {
  int expected = atoi(argv[1]);
  int proc = !!SDL_GL_GetProcAddress("glBegin");
  printf("proc exists: %d, expected: %d\n", proc, expected);
  if (proc && !expected) {
    printf("should not exist\n");
    return 1;
  } else if (!proc && expected) {
    printf("should have existed\n");
    return 1;
  }
  assert(proc == expected);
  printf("success\n");
  return 0;
}

