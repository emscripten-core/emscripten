#include <stdio.h>
#include <SDL/SDL.h>


int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);

  if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
  if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  SDL_Flip(screen); 

  SDL_Quit();

  return 0;
}

