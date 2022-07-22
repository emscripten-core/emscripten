// Invlude emscripten/version.h to ensure that the in-tree
// include directory has not been added to the include path.
#include <emscripten/version.h>
#include <stdio.h>
#include <AL/al.h>
#include <GL/gl.h>
#include <SDL.h>

int main() {
  SDL_version compiled;
  SDL_VERSION(&compiled);
  SDL_Log("SDL version: %d.%d.%d\n", compiled.major, compiled.minor, compiled.patch);

  printf("AL_VERSION: %s\n", alGetString(AL_VERSION));
  return 0;
}
