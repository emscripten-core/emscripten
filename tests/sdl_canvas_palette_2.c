#include <stdio.h>
#include <SDL/SDL.h>
#include <emscripten.h>

static const int COLOR_COUNT = 32;

static SDL_Surface *screen;
static SDL_Color   pal[COLOR_COUNT +1];

void initializePalette() {
  //initialize sdl palette
  //with red green and blue
  //colors
  pal[0].r = 0;
  pal[0].g = 0;
  pal[0].b = 0;
  pal[0].unused = 0;

  for (int i=1; i< 1 + COLOR_COUNT; i++) {
    pal[i].r = 255 / COLOR_COUNT * i;
    pal[i].g = 0;
    pal[i].b = 0;
    pal[i].unused = 0;
  }

  SDL_SetColors(screen, pal, 0, 1 + COLOR_COUNT);
}

void animatePalette() {
  SDL_Color temporary;
  temporary = pal[1];
  for (int i=2; i< 1 + COLOR_COUNT; i++) {
    pal[i-1] = pal[i];
  }
  pal[COLOR_COUNT] = temporary;

  SDL_SetColors(screen, pal, 1, COLOR_COUNT);

  //refreshing
  SDL_LockSurface(screen);
  SDL_UnlockSurface(screen);

  printf("yet another cycle\n");
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_SetVideoMode(600, 400, 8, SDL_HWSURFACE | SDL_HWPALETTE);

  //test empty pallete
  SDL_LockSurface(screen);
  SDL_UnlockSurface(screen);

  initializePalette();

  //palette is red yellow blue
  SDL_LockSurface(screen);
  int size = screen->h * screen->pitch;
  char *color   = screen->pixels;
  int divider  = size / COLOR_COUNT;
  int i = 0;
  while (i < size) {
    *color = 1 + (i / divider); //red
    color++;
    i++;
  }
  SDL_UnlockSurface(screen);

  //Animation
  printf("you should see red gradient animation\n");
  emscripten_set_main_loop(animatePalette, 0, 1);

  SDL_Quit();

  return 0;
}

