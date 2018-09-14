/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <emscripten.h>

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 400, 8, SDL_HWSURFACE | SDL_HWPALETTE);

  //initialize sdl palette
  //with red green and blue
  //colors
  SDL_Color pal[3];
  pal[0].r = 255;
  pal[0].g = 0;
  pal[0].b = 0;
  pal[0].unused = 0;

  pal[1].r = 0;
  pal[1].g = 255;
  pal[1].b = 0;
  pal[1].unused = 0;

  pal[2].r = 0;
  pal[2].g = 0;
  pal[2].b = 255;
  pal[2].unused = 0;

  SDL_SetColors(screen, pal, 0, 3);

  SDL_FillRect(screen, NULL, 0);

  {
    SDL_Rect rect = { 300, 0, 300, 200 };
    SDL_FillRect(screen, &rect, 1);
  }

  {
    SDL_Rect rect = { 0, 200, 600, 200 };
    SDL_FillRect(screen, &rect, 2);
  }

  //changing green color
  //to yellow
  pal[1].r = 255;
  SDL_SetColors(screen, &pal[1], 1, 1);

  {
    SDL_Rect rect = { 300, 200, 300, 200 };
    SDL_FillRect(screen, &rect, 1);
  }

  printf("you should see red, blue and yellow rectangles\n");

  SDL_Quit();

  return 0;
}

