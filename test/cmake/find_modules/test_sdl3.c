// Include emscripten/version.h to ensure that the in-tree
// include directory has not been added to the include path.
#include <emscripten/version.h>
#include <SDL3/SDL.h>

int main() {
  int compiled = SDL_VERSION;
  SDL_Log("SDL version: %d.%d.%d\n",
          SDL_VERSIONNUM_MAJOR(compiled),
          SDL_VERSIONNUM_MINOR(compiled),
          SDL_VERSIONNUM_MICRO(compiled));
  return 0;
}
