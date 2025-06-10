/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <assert.h>
#include <emscripten.h>

SDL_Surface* screen;

void testImage(const char* fileName, int shouldWork) {
  SDL_Surface *image = IMG_Load(fileName);
  if (!image)
  {
     printf("IMG_Load: %s\n", IMG_GetError());
     assert(!shouldWork);
     return;
  }
  assert(shouldWork);
  assert(image->format->BitsPerPixel == 32);
  assert(image->format->BytesPerPixel == 4);
  assert(image->pitch == 4*image->w);

  SDL_BlitSurface (image, NULL, screen, NULL);
  SDL_FreeSurface (image);
}

void ready(const char *f) {
  printf("ready!\n");

  testImage("screenshot.jpg", 1);

  SDL_Flip(screen);

  EM_ASM(reftestUnblock());
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_SetVideoMode(600, 450, 32, SDL_SWSURFACE);

  testImage("screenshot.jpg", 0);

  printf("prepare..\n");

  assert(emscripten_run_preload_plugins("screenshot.jpg", ready, NULL) == 0);

  EM_ASM(reftestBlock());
  return 0;
}

