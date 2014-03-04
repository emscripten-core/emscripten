#include <assert.h>
#include <stdio.h>
#include <SDL/SDL.h>

int main(int argc, char** argv) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);

  // pixels should always be initialized for software surfaces,
  // without having to call SDL_LockSurface / SDL_UnlockSurface
  assert(screen->pixels != NULL);

  SDL_Quit();

#ifdef __EMSCRIPTEN__
  int result = 1;
  REPORT_RESULT();
#endif

  return 0;
}

