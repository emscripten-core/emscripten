/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <emscripten.h>

int result = 1;

int SDLCALL EventHandler(void *userdata, SDL_Event *event) {
  switch(event->type) {
    case SDL_KEYDOWN:
      break;
    case SDL_KEYUP:
      // don't handle the modifier key events
      if (event->key.keysym.sym == SDLK_LCTRL ||
          event->key.keysym.sym == SDLK_LSHIFT ||
          event->key.keysym.sym == SDLK_LALT) {
        return 0;
      }
      if ((event->key.keysym.mod & KMOD_LCTRL) || (event->key.keysym.mod & KMOD_RCTRL)) {
        result *= 2;
      }
      if ((event->key.keysym.mod & KMOD_LSHIFT) || (event->key.keysym.mod & KMOD_RSHIFT)) {
        result *= 3;
      }
      if ((event->key.keysym.mod & KMOD_LALT) || (event->key.keysym.mod & KMOD_RALT)) {
        result *= 5;
      }
      switch (event->key.keysym.sym) {
        case SDLK_RIGHT: printf("right\n"); result *= 7; break;
        case SDLK_LEFT: printf("left\n"); result *= 11; break;
        case SDLK_DOWN: printf("down\n"); result *= 13; break;
        case SDLK_UP: printf("up\n"); result *= 17; break;
        case SDLK_a: printf("a\n"); result *= 19; break;
        default: {
          if (event->key.keysym.scancode == SDL_SCANCODE_B) {
            printf("b scancode\n"); result *= 23; break;
          }
          printf("unknown key: sym %d scancode %d\n", event->key.keysym.sym, event->key.keysym.scancode);
          emscripten_force_exit(result); // comment this out to leave event handling active. Use the following to log DOM keys:
                                         // addEventListener('keyup', function(event) { console.log(event->keyCode) }, true)
        }
      }
      break;
    default: /* Report an unhandled event */
      printf("I don't know what this event is (type=%d)!\n", event->type);
  }
  return 0;
}

void one() {
#ifndef TEST_EMSCRIPTEN_SDL_SETEVENTHANDLER
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    EventHandler(0, &event);
  }
#endif
}

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

#ifdef TEST_EMSCRIPTEN_SDL_SETEVENTHANDLER
  emscripten_SDL_SetEventHandler(EventHandler, 0);
#endif

  emscripten_set_main_loop(one, 0, 0);

  EM_ASM({keydown(1250);keydown(38);keyup(38);keyup(1250);}); // alt, up
  EM_ASM({keydown(1248);keydown(1249);keydown(40);keyup(40);keyup(1249);keyup(1248);}); // ctrl, shift, down
  EM_ASM({keydown(37);keyup(37);}); // left
  EM_ASM({keydown(39);keyup(39);}); // right

#ifdef TEST_SLEEP
  printf("sleep...\n");
  emscripten_sleep(2000);
  printf("rise!\n");
#endif

  EM_ASM({keydown(65);keyup(65);}); // a
  EM_ASM({keydown(66);keyup(66);}); // b
  EM_ASM({keydown(100);keyup(100);}); // trigger the end

  return 0;
}

