#include <stdio.h>
#include <SDL.h>

int main() {
  SDL_version compiled;

  SDL_VERSION(&compiled);
  SDL_Log("SDL version: %d.%d.%d\n", compiled.major, compiled.minor, compiled.patch);
  return 0;
}
