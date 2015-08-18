#include <stdio.h>
#include <SDL2/SDL.h>
#include <emscripten.h>

int result = 1;

int EventHandler(void *userdata, SDL_Event *event) {
  int mod;
  
  switch(event->type) {
    case SDL_KEYUP:
      break;
    case SDL_KEYDOWN:
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
          REPORT_RESULT();
          emscripten_run_script("throw 'done'"); // comment this out to leave event handling active. Use the following to log DOM keys:
                                                 // addEventListener('keyup', function(event) { console.log(event->keyCode) }, true)
        }
      }
      break;
    case SDL_TEXTINPUT:
      if (event->text.text[0] == 'A') {
        printf("a\n");result *= 5;
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
  SDL_Window *window;
  window = SDL_CreateWindow("sdl2_key",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            640, 480, 0);

#ifdef TEST_EMSCRIPTEN_SDL_SETEVENTHANDLER
  emscripten_SDL_SetEventHandler(EventHandler, 0);
#else
  one();
#endif

  SDL_StartTextInput();

  emscripten_run_script("keydown(38);keyup(38)"); // up
  emscripten_run_script("keydown(40);keyup(40);"); // down
  emscripten_run_script("keydown(37);keyup(37);"); // left
  emscripten_run_script("keydown(39);keyup(39);"); // right
  emscripten_run_script("keydown(65);keyup(65);"); // a
  emscripten_run_script("keydown(66);keyup(66);"); // b
  emscripten_run_script("keydown(100);keyup(100);"); // trigger the end

  return 0;
}

