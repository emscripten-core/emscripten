#include <SDL/SDL.h>

#if EMSCRIPTEN
#include <emscripten.h>
#endif

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(40, 40, 32, SDL_SWSURFACE);

  SDL_FillRect(screen, NULL, 0xff0000ff);
  SDL_LockSurface(screen);
  *((int*)screen->pixels + 95) = 0;
  SDL_UnlockSurface(screen);

  SDL_FillRect(screen, NULL, 0x00ff00ff); // wipe out previous pixel and fill
  SDL_LockSurface(screen);
  *((int*)screen->pixels + 205) = 0;
  SDL_UnlockSurface(screen);

  SDL_Flip(screen);

  while(1) { SDL_WaitEvent(NULL); }

  return 0;
}
