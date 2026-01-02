/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <assert.h>
#include <emscripten.h>
#include <unistd.h>

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  if (!TTF_Init()) {
    printf("TTF_Init: %s\n", SDL_GetError());
    return 1;
  }

  TTF_Quit();
  SDL_Quit();

  return 0;
}
