#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <assert.h>
#include <emscripten.h>

int result = 1;

void one() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_MOUSEMOTION: {
        SDL_MouseMotionEvent *m = (SDL_MouseMotionEvent*)&event;
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
        break;
      }
    }
  }
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  SDL_Rect rect = { 0, 0, 600, 450 };
  SDL_FillRect(screen, &rect, 0x2244ff00);

  emscripten_run_script("simulateMouseEvent(10, 20, -1)"); // move from 0,0 to 10,20
  emscripten_run_script("simulateMouseEvent(10, 20, 0)"); // click
  emscripten_run_script("simulateMouseEvent(30, 77, -1)"); // move some more
  emscripten_run_script("simulateMouseEvent(30, 77, 1)"); // trigger the end

  emscripten_set_main_loop(one, 0);

  return 0;
}

