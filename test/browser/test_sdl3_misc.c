/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "SDL3/SDL.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <emscripten.h>

int main(int argc, char* argv[]) {
  SDL_Window* window;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("Unable to initialize SDL: %s\n", SDL_GetError());
    assert(false);
  }

  window = SDL_CreateWindow("sdl3_misc", 100, 100, 0);

  EM_ASM({
    assert(document.title == 'sdl3_misc');
  });
  const char* intended = "a custom window title";
  SDL_SetWindowTitle(window, intended);
  const char* seen = SDL_GetWindowTitle(window);
  if (strcmp(intended, seen) != 0) {
    printf("Got a weird title back: %s\n", seen);
    assert(false);
  }
  EM_ASM({
    assert(document.title == 'a custom window title');
  });

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
