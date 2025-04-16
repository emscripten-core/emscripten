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
#define width 320
#define height 240

int main() {
  SDL_Rect rect = {0, 0, width, height};
  SDL_Rect firstRect = {10, 10, 50, 100};
  SDL_Rect secondRect = {30, 50, 100, 100};

  SDL_Rect firstRectForFill = {240 + 10, 10, 50, 100};
  SDL_Rect secondRectForFill = {240 + 30, 50, 100, 100};

  SDL_Rect rectForTest = {0, 0, 0, 0};
  SDL_Rect lastRect = { 100, 150, 120, 40};

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *dst = SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE);
  SDL_Surface *src = SDL_CreateRGBSurface(0, width, height, 32, 
    0x000000ff, 
    0x0000ff00, 
    0x00ff0000, 
    0xff000000);

  /* Fill dst with yellow color */
  SDL_FillRect(src, &rect, SDL_MapRGB(src->format, 255, 255, 0));
  SDL_FillRect(dst, &rect, SDL_MapRGB(dst->format, 0, 0, 0));
  SDL_SetClipRect(dst, NULL);
  SDL_BlitSurface(src, &rect, dst, &rect);

  /* Draw red on dst */
  SDL_FillRect(src, &rect, SDL_MapRGB(src->format, 255, 0, 0));
  SDL_SetClipRect(dst, &firstRect);
  SDL_BlitSurface(src, &rect, dst, &rect);

  assert(rect.x = firstRect.x);
  assert(rect.y = firstRect.y);
  assert(rect.w = firstRect.w);
  assert(rect.h = firstRect.h);

  /* Draw green rect on red rect */
  SDL_FillRect(src, &rect, SDL_MapRGB(src->format, 0, 255, 0));
  SDL_SetClipRect(dst, &secondRect);
  SDL_BlitSurface(src, &rect, dst, &rect);

  assert(rect.x = secondRect.x);
  assert(rect.y = secondRect.y);
  assert(rect.w = firstRect.x + firstRect.w);
  assert(rect.h = firstRect.h + firstRect.h);

  /* Same with fill rect */
  rect.x = 0; rect.y = 0;
  rect.w = width; rect.h = height;

  SDL_SetClipRect(dst, &firstRectForFill);
  SDL_FillRect(dst, &rect, SDL_MapRGB(dst->format, 0, 0, 255));

  assert(rect.x = firstRectForFill.x);
  assert(rect.y = firstRectForFill.y);
  assert(rect.w = firstRectForFill.w);
  assert(rect.h = firstRectForFill.h);

  SDL_SetClipRect(dst, &secondRectForFill);
  SDL_FillRect(dst, &rect, SDL_MapRGBA(dst->format, 255, 0, 255, 255));

  assert(rect.x = secondRectForFill.x);
  assert(rect.y = secondRectForFill.y);
  assert(rect.w = firstRectForFill.x + firstRectForFill.w);
  assert(rect.h = firstRectForFill.h + firstRectForFill.h);

  SDL_GetClipRect(dst, &rectForTest);
  assert(rectForTest.x == 270);
  assert(rectForTest.y == 50);
  assert(rectForTest.w == 50);
  assert(rectForTest.h == 100);

  SDL_SetClipRect(dst, 0);
  SDL_FillRect(dst, &lastRect, SDL_MapRGBA(dst->format, 255, 0, 0, 255));
  SDL_UpdateRect(dst, 0, 0, width, height);

  printf("There should be yellow background\n");
  printf("One the left side there should be red rect with green rect inside\n");
  printf("One the right side there should be blue rect with pink rect inside\n");

  SDL_Quit();

  return 0;
}
