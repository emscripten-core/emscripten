#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <assert.h>
#include <emscripten.h>

int result = 0;

void one() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_QUIT: {
        if (!result) { // prevent infinite recursion since REPORT_RESULT does window.close too.
          result = 1;
          REPORT_RESULT_INTERNAL(1);
        }
      }
    }
  }
}

void main_2();

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  emscripten_set_main_loop(one, 0, 0);

  emscripten_run_script("setTimeout(function() { window.close() }, 2000)");
}

