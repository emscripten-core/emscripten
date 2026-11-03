/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>

#define abs(x) ((x) < 0 ? -(x) : (x))

void one() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_MOUSEMOTION: {
        SDL_MouseMotionEvent *m = (SDL_MouseMotionEvent*)&event;
        printf("motion: abs:%d,%d  rel:%d,%d.  state: %d.\n", m->x, m->y, m->xrel, m->yrel, m->state);
        assert(m->state == 0);
        int x, y;
        SDL_GetMouseState(&x, &y);
        assert(x == m->x);
        assert(y == m->y);
        static bool first_motion = true;
        if (first_motion) {
          first_motion = false;
#ifdef TEST_SDL_MOUSE_OFFSETS
          assert(abs(m->x-5) <= 1);
          assert(abs(m->y-15) <= 1);
          assert(abs(m->xrel-5) <= 1);
          assert(abs(m->yrel-15) <= 1);
#else
          assert(abs(m->x-10) <= 1);
          assert(abs(m->y-20) <= 1);
          assert(abs(m->xrel-10) <= 1);
          assert(abs(m->yrel-20) <= 1);
#endif
        } else {
#ifdef TEST_SDL_MOUSE_OFFSETS
          assert(abs(m->x-25) <= 1);
          assert(abs(m->y-72) <= 1);
          assert(abs(m->xrel-20) <= 1);
          assert(abs(m->yrel-57) <= 1);
#else
          assert(abs(m->x-30) <= 1);
          assert(abs(m->y-77) <= 1);
          assert(abs(m->xrel-20) <= 1);
          assert(abs(m->yrel-57) <= 1);
#endif
        }
        break;
      }
      case SDL_MOUSEBUTTONDOWN: {
        SDL_MouseButtonEvent *m = (SDL_MouseButtonEvent*)&event;
        printf("button down: %d, state: %d  abs: %d,%d.\n", m->button, m->state, m->x, m->y);
        if (m->button == 2) {
          emscripten_force_exit(0);
        }
        assert(m->button == 1);
        assert(m->state == 1);
#ifdef TEST_SDL_MOUSE_OFFSETS
        assert(abs(m->x-5) <= 1);
        assert(abs(m->y-15) <= 1);
#else
        assert(abs(m->x-10) <= 1);
        assert(abs(m->y-20) <= 1);
#endif
        break;
      }
      case SDL_MOUSEBUTTONUP: {
        SDL_MouseButtonEvent *m = (SDL_MouseButtonEvent*)&event;
        printf("button up: %d, state: %d  abs: %d,%d.\n", m->button, m->state, m->x, m->y);
        assert(m->button == 1);
        assert(m->state == 0);
#ifdef TEST_SDL_MOUSE_OFFSETS
        assert(abs(m->x-5) <= 1);
        assert(abs(m->y-15) <= 1);
#else
        assert(abs(m->x-10) <= 1);
        assert(abs(m->y-20) <= 1);
#endif
        // Remove another click we want to ignore
        assert(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONDOWN) == 1);
        assert(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONUP) == 1);
        break;
      }
    }
  }
}

void simulate_events() {
#ifdef TEST_SDL_MOUSE_OFFSETS
  int absolute = true;
#else
  int absolute = false;
#endif

  EM_ASM(simulateMouseMove(10, 20, $0), absolute); // move from 0,0 to 10,20
  EM_ASM(simulateMouseClick(10, 20, 0, $0), absolute); // click
  EM_ASM(simulateMouseClick(10, 20, 0, $0), absolute); // click some more, but this one should be ignored through PeepEvent
  EM_ASM(simulateMouseMove(30, 77, $0), absolute); // move some more
  EM_ASM(simulateMouseClick(30, 77, 1, $0), absolute); // trigger the end

  emscripten_set_main_loop(one, 0, 0);
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  SDL_Rect rect = { 0, 0, 600, 450 };
  SDL_FillRect(screen, &rect, 0x2244ffff);

  // Safari bug: If simulateMouseMove() is called synchronously in the same function that resizes the canvas element
  // (as SDL_SetVideoMode() does above), then the simulated mouse events are dispatched to the wrong x/y locations.
  // We must yield back to the event loop once first to let the browser relayout, and only then we can dispath mouse
  // events with correct coordinates.
  // TODO: Figure out a minimal repro of this, and report it to WebKit bug tracker.
  emscripten_set_timeout(simulate_events, 0, 0);
}
