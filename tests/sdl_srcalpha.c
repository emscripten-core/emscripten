#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#if EMSCRIPTEN
#include <emscripten.h>
#endif

SDL_Surface  *screen, *black;
int fade_level = 0;
void one(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE )
          exit(0);
        break;
    }
  }

  SDL_Rect rect;
  rect.x = rect.y = 0;
  rect.w = screen->w;
  rect.h = screen->h;
  SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 200, 0, 0));
  if (fade_level != 0) {
    SDL_SetAlpha(black, SDL_SRCALPHA, fade_level);
    SDL_BlitSurface(black, 0, screen, 0);
  }
  SDL_Flip(screen);
  fade_level += 5;
  fade_level &= 255;
}

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);
  black = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, screen->h,
                                   screen->format->BitsPerPixel,
                                   screen->format->Rmask,
                                   screen->format->Gmask,
                                   screen->format->Bmask,
                                   screen->format->Amask);
  SDL_Rect rect;
  rect.x = rect.y = 0;
  rect.w = screen->w;
  rect.h = screen->h;
  SDL_FillRect(black, &rect, SDL_MapRGB(black->format, 0, 0, 0));

#if EMSCRIPTEN
  emscripten_set_main_loop(one, 0, 0);
#else
  while (1) {
    Uint32 ticksbefore = SDL_GetTicks();
    one();
    Uint32 ticksafter = SDL_GetTicks();
    int delay = 20 - (ticksafter - ticksbefore);
    if (delay > 0) {
      SDL_Delay(delay);
    }
  }
#endif
  return 0;
}

