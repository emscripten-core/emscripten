/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
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

int testImage(const char* fileName) {
  SDL_Surface *image = IMG_Load(fileName);
  if (!image)
  {
     printf("IMG_Load: %s\n", IMG_GetError());
     return 0;
  }
  assert(image->format->BitsPerPixel == 32);
  assert(image->format->BytesPerPixel == 4);
  assert(image->pitch == 4*image->w);
  int result = image->w;

  SDL_BlitSurface (image, NULL, screen, NULL);
  SDL_FreeSurface (image);

  return result;
}

void ready() {
  printf("ready!\n");

  testImage("screenshot.jpg"); // relative path

  SDL_Flip(screen);
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_SetVideoMode(600, 450, 32, SDL_SWSURFACE);

  printf("rename..\n");

  rename("screenshot.not", "screenshot.jpg");

  printf("decode..\n");

  ready();

  return 0;
}

