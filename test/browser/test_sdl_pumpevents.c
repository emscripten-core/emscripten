/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include <emscripten.h>
// bug - SDL_GetKeyboardState doesn't return scancodes, it returns keycodes, so acts exactly like
// SDL_GetKeyState instead
#define SDL_GetKeyState SDL_GetKeyboardState

int result = 0;

void loop1() {
   unsigned i;
   int r = 0;

   // method 1: SDL_PollEvent loop
   SDL_Event e;
   while (SDL_PollEvent(&e));

   const Uint8 *keys = SDL_GetKeyState(NULL);
   assert(keys[SDLK_LEFT]);
}

void loop2() {
   unsigned i;
   int r = 0;

   // method 2: SDL_PumpEvents
   SDL_PumpEvents();

   const Uint8 *keys = SDL_GetKeyState(NULL);
   assert(keys[SDLK_RIGHT]);
}

void alphakey() {
   unsigned i;
   int r = 0;

   SDL_PumpEvents();

   const Uint8 *keys = SDL_GetKeyState(NULL);
   assert(keys[SDLK_a]);
}

int main(int argc, char *argv[]) {
   SDL_Init(SDL_INIT_EVERYTHING);
   SDL_SetVideoMode(600, 400, 32, SDL_SWSURFACE);

   emscripten_run_script("simulateKeyDown(37);"); // left
   loop1();
   emscripten_run_script("simulateKeyDown(39);"); // right
   loop2();
   emscripten_run_script("simulateKeyDown(65);"); // A
   alphakey();
   return 0;
}
