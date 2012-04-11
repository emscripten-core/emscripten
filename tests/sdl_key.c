#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <emscripten.h>

int result = 1;

void one() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_KEYDOWN:
        break;
      case SDL_KEYUP:
        switch (event.key.keysym.sym) {
          case SDLK_RIGHT: printf("right\n"); result *= 2; break;
          case SDLK_LEFT: printf("left\n"); result *= 3; break;
          case SDLK_DOWN: printf("down\n"); result *= 5; break;
          case SDLK_UP: printf("up\n"); result *= 7; break;
          case SDLK_SPACE: printf("space\n"); result *= 11; break;
          case SDLK_a: printf("a\n"); result *= 13; break;
          default: {
            if (event.key.keysym.scancode == SDL_SCANCODE_B) {
              printf("b scancode\n"); result *= 17; break;
            }
            REPORT_RESULT();
            emscripten_run_script("throw 'done'");
          }
        }
        break;
      default: /* Report an unhandled event */
        printf("I don't know what this event is!\n");
    }
  }
}

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  emscripten_run_script("simulateKeyEvent(38)");
  emscripten_run_script("simulateKeyEvent(40)");
  emscripten_run_script("simulateKeyEvent(37)");
  emscripten_run_script("simulateKeyEvent(39)");
  emscripten_run_script("simulateKeyEvent(32)");
  emscripten_run_script("simulateKeyEvent(97)");
  emscripten_run_script("simulateKeyEvent(98)");
  emscripten_run_script("simulateKeyEvent(100)"); // trigger the end

  if (argc == 1337) one(); // keep it alive

  return 0;
}

