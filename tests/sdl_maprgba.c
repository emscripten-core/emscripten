/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <assert.h>

int main() {
  Uint32 c;
  Uint8 r, g, b, a;
  SDL_Rect rect = {0,0,300,450};

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  c = SDL_MapRGB(screen->format, 0xff, 0x00, 0x00); // OPAQUE RED
  SDL_GetRGB(c, screen->format, &r, &g, &b);
  assert(r == 0xff);
  assert(g == 0x00);
  assert(b == 0x00);
  SDL_FillRect(screen, &rect, c);
  rect.x = 300;
  c = SDL_MapRGB(screen->format, 0x7f, 0x7f, 0x00); // OPAQUE MUSTARD
  SDL_GetRGB(c, screen->format, &r, &g, &b);
  assert(r == 0x7f);
  assert(g == 0x7f);
  assert(b == 0x00);
  SDL_FillRect(screen, &rect, c);
  rect.x = 150;
  rect.y = 112;
  rect.w = 300;
  rect.h = 225;
  c = SDL_MapRGBA(screen->format, 0xff, 0xff, 0xff, 0xff); // OPAQUE WHITE
  SDL_GetRGBA(c, screen->format, &r, &g, &b, &a);
  assert(r == 0xff);
  assert(g == 0xff);
  assert(b == 0xff);
  assert(a == 0xff);
  SDL_FillRect(screen, &rect, c);
  c = SDL_MapRGBA(screen->format, 0x00, 0x00, 0x00, 0x00); // TRANSPARENT BLACK
  SDL_GetRGBA(c, screen->format, &r, &g, &b, &a);
  assert(r == 0x00);
  assert(g == 0x00);
  assert(b == 0x00);
  assert(a == 0x00);
  SDL_FillRect(screen, &rect, c);
  SDL_UpdateRect(screen, 0, 0, 600, 450);

  printf("The left half should be red and the right half mustard.\n");
  printf("There should be a white rectangle in the center.\n");

  SDL_Quit();

  return 0;
}
