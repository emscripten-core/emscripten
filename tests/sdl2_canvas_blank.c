#include <stdio.h>
#include <SDL2/SDL.h>


int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_CreateWindowAndRenderer(256, 256, 0, &window, &renderer);

  SDL_Surface *screen = SDL_CreateRGBSurface(0, 256, 256, 8, 0, 0, 0, 0);

  if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
  if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  SDL_RenderPresent(renderer);

  SDL_Quit();

  return 0;
}

