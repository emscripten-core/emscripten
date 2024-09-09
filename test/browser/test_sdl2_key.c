/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL2/SDL.h>
#include <emscripten.h>

int result = 1;

int EventHandler(void *userdata, SDL_Event *event) {
  int mod;

  switch(event->type) {
    case SDL_KEYUP:
      break;
    case SDL_KEYDOWN:
      switch (event->key.keysym.sym) {
        case SDLK_RIGHT: printf("KEYDOWN: right\n"); result *= 7; break;
        case SDLK_LEFT: printf("KEYDOWN: left\n"); result *= 11; break;
        case SDLK_DOWN: printf("KEYDOWN: down\n"); result *= 13; break;
        case SDLK_UP: printf("KEYDOWN: up\n"); result *= 17; break;
        case SDLK_a: printf("KEYDOWN: a\n"); result *= 19; break;
        default: {
          if (event->key.keysym.scancode == SDL_SCANCODE_B) {
            printf("KEYDOWN: b scancode\n"); result *= 23; break;
          }
          printf("unknown key: sym %d scancode %d\n", event->key.keysym.sym, event->key.keysym.scancode);
          emscripten_force_exit(result); // comment this out to leave event handling active. Use the following to log DOM keys:
                                         // addEventListener('keyup', function(event) { console.log(event->keyCode) }, true)
        }
      }
      break;
    case SDL_TEXTINPUT:
      printf("TEXTINPUT: %c\n", event->text.text[0]);
      if (event->text.text[0] == 'A') {
        result *= 5;
      }
      break;
    default: /* Report an unhandled event */
      printf("I don't know what this event is (type=%d)!\n", event->type);
  }
  return 0;
}

void pump_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    EventHandler(0, &event);
  }
}

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window;
  window = SDL_CreateWindow("sdl2_key",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            640, 480, 0);

  SDL_StartTextInput();

  emscripten_run_script("simulateKeyDown(38,  'ArrowUp');   simulateKeyUp(38,  'ArrowUp')"); // up
  emscripten_run_script("simulateKeyDown(40,  'ArrowDown'); simulateKeyUp(40,  'ArrowDown')"); // down
  emscripten_run_script("simulateKeyDown(37,  'ArrowLeft'); simulateKeyUp(37,  'ArrowLeft');"); // left
  emscripten_run_script("simulateKeyDown(39,  'ArrowRight');simulateKeyUp(39,  'ArrowRight');"); // right
  emscripten_run_script("simulateKeyDown(65,  'KeyA');      simulateKeyUp(65,  'KeyA');"); // a
  emscripten_run_script("simulateKeyDown(66,  'KeyB');      simulateKeyUp(66,  'KeyB');"); // b
  emscripten_run_script("simulateKeyDown(100, 'Numpad4');   simulateKeyUp(100, 'Numpad4');"); // trigger the end

  emscripten_set_main_loop(pump_events, 3, 0);
  return 99;
}

