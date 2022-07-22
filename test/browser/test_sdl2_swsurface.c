/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

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

  return 0;
}

