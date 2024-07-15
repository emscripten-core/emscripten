/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <emscripten.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef BITSPERPIXEL
#define BITSPERPIXEL 32
#endif

int testImage(SDL_Renderer* renderer, const char* fileName) {
  SDL_Surface *image = IMG_Load(fileName);
  if (!image)
  {
     printf("IMG_Load: %s\n", IMG_GetError());
     return 0;
  }
  assert(image->format->BitsPerPixel == BITSPERPIXEL);
  assert(image->format->BytesPerPixel == BITSPERPIXEL / 8);
  assert(image->pitch == BITSPERPIXEL / 8 * image->w);
  int result = image->w;

#ifndef NO_PRELOADED
  int w, h;
  char *data = emscripten_get_preloaded_image_data(fileName, &w, &h);

  assert(data);
  assert(w == image->w);
  assert(h == image->h);
#endif

  SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, image);

  SDL_RenderCopy (renderer, tex, NULL, NULL);

  SDL_DestroyTexture (tex);

  SDL_FreeSurface (image);

#ifndef NO_PRELOADED
  free(data);
#endif

  return result;
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window;
  SDL_Renderer *renderer;

  SDL_CreateWindowAndRenderer(600, 450, 0, &window, &renderer);

  int result = 0;

  result = testImage(renderer, SCREENSHOT_DIRNAME "/" SCREENSHOT_BASENAME); // absolute path
  assert(result != 0);

  chdir(SCREENSHOT_DIRNAME);
  result = testImage(renderer, "./" SCREENSHOT_BASENAME); // relative path
  assert(result != 0);

  SDL_RenderPresent(renderer);

  printf("you should see an image.\n");

  return result;
}
