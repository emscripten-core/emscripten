/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <assert.h>
#include <stdint.h>

#define width 600
#define height 450
uint8_t pixels[width * height * 4];

int main() {
  uint8_t *end = pixels + width * height * 4;
  uint8_t *pixel = pixels;
  SDL_Rect rect = {0, 0, width, height};

  while (pixel != end) {
    *pixel = (pixel - pixels) * 256 / (width * height * 4);
    pixel++;
  }

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE);
  SDL_Surface *image = SDL_CreateRGBSurfaceFrom(pixels, width, height, 32, width * 4,
    0x000000ff,
    0x0000ff00,
    0x00ff0000,
    0xff000000);

  SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255, 0, 0));
  SDL_BlitSurface(image, &rect, screen, &rect);
  SDL_UpdateRect(screen, 0, 0, width, height);

  printf("There should be a red to white gradient\n");

  SDL_Quit();
  return 0;
}
