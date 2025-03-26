/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <emscripten.h>
#include <emscripten/html5.h>


int main(int argc, char **argv) {
#ifdef __EMSCRIPTEN__
  // include GL stuff, to check that we can compile hybrid 2d/GL apps
  extern void glBegin(int mode);
  extern void glBindBuffer(int target, int buffer);
  if (argc == 9876) {
    glBegin(0);
    glBindBuffer(0, 0);
  }
#endif

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  printf("Init: %d\n", TTF_Init());

  TTF_Font *font = TTF_OpenFont("sans-serif", 40);
  printf("Font: %p\n", font);

  SDL_Color color = { 0xff, 0x99, 0x00, 0xff };

  SDL_Surface *text = TTF_RenderText_Solid(font, "hello orange world", color);

  SDL_Color color2 = { 0xbb, 0, 0xff, 0xff };
  SDL_Surface *text2 = TTF_RenderText_Solid(font, "a second line, purple", color2);

  // render
  SDL_Rect dest = { 0, 50, 0, 0 };
  SDL_BlitSurface (text, NULL, screen, NULL);
  dest.y = 100;
  SDL_BlitSurface (text2, NULL, screen, &dest);

  // fill stuff
  SDL_Rect rect = { 200, 200, 175, 125 };
  SDL_FillRect(screen, &rect, SDL_MapRGBA(screen->format, 0x22, 0x22, 0xff, 0xff));

  SDL_Flip(screen);

  SDL_LockSurface(screen);

  int width, height;
  emscripten_get_canvas_element_size("#canvas", &width, &height);
  assert(width == 600);
  assert(height == 450);

  int sum = 0;
  int row_size = screen->w * 4;
  // Sum the red components of each pixel on the diagonal.
  for (int i = 0; i < screen->h; i++) {
    sum += *((char*)screen->pixels + i*row_size + i*4 + 0);
  }
  printf("Sum: %d\n", sum);

  printf("you should see two lines of text in different colors and a blue rectangle\n");

  SDL_UnlockSurface(screen);

  SDL_Quit();

  printf("done.\n");

  // varies a little on different browsers, font differences?
  assert(sum > 2000 && sum < 5000);

  return 0;
}

