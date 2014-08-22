#include <stdio.h>
#include <SDL/SDL.h>
#include <assert.h>
#include <emscripten.h>

void tick() {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_WINDOWEVENT: {
        SDL_WindowEvent windowEvent = event.window;
        switch (windowEvent.event)
        {
           case SDL_WINDOWEVENT_SHOWN:{
                printf("SDL_WINDOWEVENT_SHOWN\n");
                break;
          }
          case SDL_WINDOWEVENT_HIDDEN:{
                printf("SDL_WINDOWEVENT_HIDDEN\n");
                break;
          }
          case SDL_WINDOWEVENT_ENTER:{
                printf("SDL_WINDOWEVENT_ENTER\n");
                break;
          }
          case SDL_WINDOWEVENT_LEAVE:{
                printf("SDL_WINDOWEVENT_LEAVE\n");
                break;
          }
          case SDL_WINDOWEVENT_FOCUS_GAINED:{
                printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
                break;
          }
          case SDL_WINDOWEVENT_FOCUS_LOST:{
                printf("SDL_WINDOWEVENT_FOCUS_LOST\n");
                break;
          }
        }
      }
    }
  }
}


int main() {

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  SDL_Rect rect = { 0, 0, 600, 450 };
  SDL_FillRect(screen, &rect, 0x2244ffff);

  emscripten_set_main_loop(tick, 0, 0);

  return 0;
}

