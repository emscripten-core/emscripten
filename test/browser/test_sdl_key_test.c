/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int result = 1;

int keys[1000];

void one() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_KEYDOWN:
        if (!keys[event.key.keysym.sym]) {
          keys[event.key.keysym.sym] = 1;
          printf("key down: sym=%d scancode=%d name=%s\n", event.key.keysym.sym, event.key.keysym.scancode, SDL_GetKeyName(event.key.keysym.sym));
        }
        break;
      case SDL_KEYUP:
        if (keys[event.key.keysym.sym]) {
          keys[event.key.keysym.sym] = 0;
          printf("key up: sym=%d scancode=%d name=%s\n", event.key.keysym.sym, event.key.keysym.scancode, SDL_GetKeyName(event.key.keysym.sym));
        }
        break;
    }
  }
}

int main(int argc, char **argv) {
  memset(keys, 0, 1000*4);

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

#if __EMSCRIPTEN__
  emscripten_set_main_loop(one, 0, 0);
#else
  while (1) one();
#endif

  return 0;
}

