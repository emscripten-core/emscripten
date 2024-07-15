/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include <emscripten.h>
// bug - SDL_GetKeyboardState doesn't return scancodes, it returns keycodes, so acts exactly like
// SDL_GetKeyState instead
#define SDL_GetKeyState SDL_GetKeyboardState

void loop1() {
  printf("loop1\n");
  unsigned i;

  // method 1: SDL_PollEvent loop
  SDL_Event e;
  while (SDL_PollEvent(&e));

  const Uint8 *keys = SDL_GetKeyState(NULL);
  assert(keys[SDL_SCANCODE_LEFT]);
}

void loop2() {
  printf("loop2\n");

  unsigned i;

  // method 2: SDL_PumpEvents
  SDL_PumpEvents();

  const Uint8 *keys = SDL_GetKeyState(NULL);
  assert(keys[SDL_SCANCODE_RIGHT]);
}

void alphakey() {
  printf("alpha\n");

  unsigned i;

  SDL_PumpEvents();

  const Uint8 *keys = SDL_GetKeyState(NULL);
  assert(keys[SDL_SCANCODE_A]);
}

int main(int argc, char *argv[])
{
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window;
  SDL_CreateWindow("window", 0, 0, 600, 450, 0);

  emscripten_run_script("simulateKeyDown(37, 'ArrowLeft');"); // left
  loop1();
  emscripten_run_script("simulateKeyDown(39, 'ArrowRight');"); // right
  loop2();
  emscripten_run_script("simulateKeyDown(65, 'KeyA');"); // A
  alphakey();
  return 0;
}
