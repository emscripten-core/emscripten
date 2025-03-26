/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <SDL/SDL.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>

int result = 0;

EMSCRIPTEN_KEEPALIVE void one() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_TEXTEDITING: assert(0); break;
      case SDL_TEXTINPUT:
        printf("Received %s\n", event.text.text);
        if (!strcmp("a", event.text.text)) {
          result = 1;
        } else if (!strcmp("A", event.text.text)) {
          assert(result);
          emscripten_force_exit(0);
        }
        break;
      default: /* Report an unhandled event */
        printf("I don't know what this event is!\n");
    }
  }
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);
  SDL_StartTextInput();

  emscripten_run_script("simulateKeyDown('a'.charCodeAt(0))");
  emscripten_run_script("simulateKeyDown('A'.charCodeAt(0))");

  emscripten_exit_with_live_runtime();
}
