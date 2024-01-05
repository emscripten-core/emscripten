/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>

int result = 1;

EMSCRIPTEN_KEEPALIVE void one() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    printf("got event %d\n", event.type);
    switch(event.type) {
      case SDL_KEYDOWN:
        break;
      case SDL_KEYUP:
        // don't handle the modifier key events
        if (event.key.keysym.sym == SDLK_LCTRL ||
            event.key.keysym.sym == SDLK_LSHIFT ||
            event.key.keysym.sym == SDLK_LALT) {
          return;
        }
        if ((event.key.keysym.mod & KMOD_LCTRL) || (event.key.keysym.mod & KMOD_RCTRL)) {
          result *= 2;
        }
        if ((event.key.keysym.mod & KMOD_LSHIFT) || (event.key.keysym.mod & KMOD_RSHIFT)) {
          result *= 3;
        }
        if ((event.key.keysym.mod & KMOD_LALT) || (event.key.keysym.mod & KMOD_RALT)) {
          result *= 5;
        }
        switch (event.key.keysym.sym) {
          case SDLK_RIGHT: printf("right\n"); result *= 7; break;
          case SDLK_LEFT: printf("left\n"); result *= 11; break;
          case SDLK_DOWN: printf("down\n"); result *= 13; break;
          case SDLK_UP: printf("up\n"); result *= 17; break;
          case SDLK_a: printf("a\n"); result *= 19; break;
          default: {
            if (event.key.keysym.scancode == SDL_SCANCODE_B) {
              printf("b scancode\n"); result *= 23; break;
            }
            printf("unknown key: sym %d scancode %d\n", event.key.keysym.sym, event.key.keysym.scancode);
            emscripten_force_exit(result); // comment this out to leave event handling active. Use the following to log DOM keys:
                                                   // addEventListener('keyup', function(event) { console.log(event.keyCode) }, true)
          }
        }
        break;
      default: /* Report an unhandled event */
        printf("I don't know what this event is!\n");
    }
  }
}

int main(int argc, char **argv) {
  printf("main\n");
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);
  emscripten_runtime_keepalive_push();
  return 0;
}
