#include <assert.h>
#include <stdio.h>
#include <SDL/SDL.h>

int main(int argc, char** argv) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("sdl2_swsurface",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        640, 480,
                                        SDL_WINDOW_FULLSCREEN);

  SDL_Surface *screen = SDL_GetWindowSurface(window);

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

