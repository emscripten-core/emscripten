/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>

static int result = 0;

static void process_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_EVENT_TEXT_EDITING: assert(0); break;
      case SDL_EVENT_TEXT_INPUT:
        printf("Received %s\n", event.text.text);
        if (!strcmp("a", event.text.text)) {
          result = 1;
        } else if (!strcmp("A", event.text.text)) {
          assert(result);
          emscripten_force_exit(0);
        }
        break;
    }
  }
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("window", 600, 450, 0);
  assert(window);
  SDL_StartTextInput(window);

  emscripten_run_script("simulateKeyDown('a'.charCodeAt(0))");
  emscripten_run_script("simulateKeyDown('A'.charCodeAt(0))");

  process_events();

  return 99;
}
