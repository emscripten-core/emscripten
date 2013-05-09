#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <emscripten.h>

int result = 1;

int keys[1000];

void one() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_KEYDOWN:
        if (!keys[event.key.keysym.sym]) {
          keys[event.key.keysym.sym] = 1;
          printf("key down: sym %d scancode %d\n", event.key.keysym.sym, event.key.keysym.scancode);
        }
        break;
      case SDL_KEYUP:
        if (keys[event.key.keysym.sym]) {
          keys[event.key.keysym.sym] = 0;
          printf("key up: sym %d scancode %d\n", event.key.keysym.sym, event.key.keysym.scancode);
        }
        break;
    }
  }
}

int main(int argc, char **argv) {
  memset(keys, 0, 1000*4);

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  emscripten_set_main_loop(one, 0, 0);

  return 0;
}

