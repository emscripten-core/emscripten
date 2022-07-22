/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <assert.h>
#include <emscripten.h>

int main(int argc, char **argv) {
  FILE *f = fopen("data.txt", "rb");
  assert(f);
  assert(fgetc(f) == 'd');
  assert(fgetc(f) == 'a');
  assert(fgetc(f) == 't');
  assert(fgetc(f) == 'u');
  assert(fgetc(f) == 'm');
  fclose(f);

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  SDL_LockSurface(screen);
  unsigned int *pixels = (unsigned int *)screen->pixels;
  for (int x = 0; x < screen->w; x++) {
    for (int y = 0; y < screen->h; y++) {
      pixels[x + y*screen->h] = x < 300 ? (y < 200 ? 0x3377AA88 : 0xAA3377CC) : (y < 200 ? 0x0066AA77 : 0xAA006699);
    }
  }
  SDL_UnlockSurface(screen);

  SDL_Quit();

  EM_ASM(window.close());
  return 0;
}

