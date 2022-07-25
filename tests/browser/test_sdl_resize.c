/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <assert.h>
#include <emscripten.h>

int stage = 0;

void loop() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_VIDEORESIZE: {
        SDL_ResizeEvent *r = (SDL_ResizeEvent*)&event;
        printf("resize event! %d:%d\n", r->w, r->h);
        switch (stage) {
          case 0:
            assert(r->w == 100);
            assert(r->h == 200);
            emscripten_set_canvas_size(123, 246);
            stage++;
            break;
          case 1:
            assert(r->w == 123);
            assert(r->h == 246);
            emscripten_force_exit(0);
            break;
        }
      }
    }
  }
}

void main_2();

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  emscripten_set_canvas_size(100, 200);

  emscripten_set_main_loop(loop, 0, 0);
}

