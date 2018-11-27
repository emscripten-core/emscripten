/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_VIDEO);
  //SDL_Surface *screen = SDL_SetVideoMode(40, 40, 32, SDL_SWSURFACE);

  SDL_Window *window;
  SDL_Renderer *renderer;

  SDL_CreateWindowAndRenderer(40, 40, 0, &window, &renderer);

  SDL_Surface *screen = SDL_CreateRGBSurface(0, 40, 40, 32, 0, 0, 0, 0);

  SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format, 0xff, 0, 0, 0xff));
  SDL_LockSurface(screen);
  *((int*)screen->pixels + 95) = 0;
  SDL_UnlockSurface(screen);

  SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format, 0, 0xff, 0, 0xff)); // wipe out previous pixel and fill
  SDL_LockSurface(screen);
  *((int*)screen->pixels + 205) = 0;
  SDL_UnlockSurface(screen);

  SDL_Texture *screenTexture = SDL_CreateTextureFromSurface(renderer, screen);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
  SDL_RenderPresent(renderer);

  return 0;
}
