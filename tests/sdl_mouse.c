#include <stdio.h>
#include <SDL/SDL.h>
#include <assert.h>
#include <emscripten.h>

int result = 1;

void one() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_MOUSEMOTION: {
        SDL_MouseMotionEvent *m = (SDL_MouseMotionEvent*)&event;
        assert(m->state == 0);
        int x, y;
        SDL_GetMouseState(&x, &y);
        assert(x == m->x && y == m->y);
        printf("motion: %d,%d  %d,%d\n", m->x, m->y, m->xrel, m->yrel);
        result += 2 * (m->x + m->y + m->xrel + m->yrel);
        break;
      }
      case SDL_MOUSEBUTTONDOWN: {
        SDL_MouseButtonEvent *m = (SDL_MouseButtonEvent*)&event;
        if (m->button == 2) {
          REPORT_RESULT();
          emscripten_run_script("throw 'done'");
        }
        printf("button down: %d,%d  %d,%d\n", m->button, m->state, m->x, m->y);
        result += 3 * (m->button + m->state + m->x + m->y);
        break;
      }
      case SDL_MOUSEBUTTONUP: {
        SDL_MouseButtonEvent *m = (SDL_MouseButtonEvent*)&event;
        printf("button up: %d,%d  %d,%d\n", m->button, m->state, m->x, m->y);
        result += 5 * (m->button + m->state + m->x + m->y);
        // Remove another click we want to ignore
        assert(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONDOWN) == 1);
        assert(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONUP) == 1);
        break;
      }
    }
  }
}

void main_2(void* arg);

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  SDL_Rect rect = { 0, 0, 600, 450 };
  SDL_FillRect(screen, &rect, 0x2244ffff);

  emscripten_async_call(main_2, NULL, 3000); // avoid startup delays and intermittent errors

  return 0;
}

void main_2(void* arg) {
  emscripten_run_script("window.simulateMouseEvent(10, 20, -1)"); // move from 0,0 to 10,20
  emscripten_run_script("window.simulateMouseEvent(10, 20, 0)"); // click
  emscripten_run_script("window.simulateMouseEvent(10, 20, 0)"); // click some more, but this one should be ignored through PeepEvent
  emscripten_run_script("window.simulateMouseEvent(30, 77, -1)"); // move some more
  emscripten_run_script("window.simulateMouseEvent(30, 77, 1)"); // trigger the end

  emscripten_set_main_loop(one, 0, 0);
}

