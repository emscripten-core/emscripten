#include <stdio.h>
#include <SDL2/SDL.h>
#include <emscripten.h>
#include <string.h>

static const int COLOR_COUNT = 32;

static SDL_Surface *screen;
static SDL_Color   pal[COLOR_COUNT +1];

void pallete(int red, int green, int blue) {
  //initialize sdl palette
  //with gradient colors
  pal[0].r = 0;
  pal[0].g = 0;
  pal[0].b = 0;
  pal[0].a = 255;

  for (int i=1; i< 1 + COLOR_COUNT; i++) {
    pal[i].r = (float) red    / COLOR_COUNT * i;
    pal[i].g = (float) green  / COLOR_COUNT * i;
    pal[i].b = (float) blue   / COLOR_COUNT * i;
    pal[i].a = 255;
  }

  SDL_SetPaletteColors(screen->format->palette, pal, 0, 1 + COLOR_COUNT);
}

int main(int argc, char** argv) {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window;
  SDL_Renderer *renderer;

  SDL_CreateWindowAndRenderer(600, 450, 0, &window, &renderer);

  screen = SDL_CreateRGBSurface(0, 600, 450, 8, 0, 0, 0, 0);

  //test empty pallete
  SDL_LockSurface(screen);
  SDL_UnlockSurface(screen);

  //Draw gradient
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

  //Set pallete
  if (argc > 1) {
    printf("%s\n", argv[1]);
    if (strcmp(argv[1], "-r") == 0) {
      printf("set [red]\n");
      pallete(255, 0, 0);
    }
    if (strcmp(argv[1], "-g") == 0) {
      printf("set [green]\n");
      pallete(0, 255, 0);
    }
    if (strcmp(argv[1], "-b") == 0) {
      printf("set [blue]\n");
      pallete(0, 0, 255);
    }
  }

  //refreshing
  SDL_LockSurface(screen);
  SDL_UnlockSurface(screen);

  SDL_Texture *screenTexture = SDL_CreateTextureFromSurface(renderer, screen);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
  SDL_RenderPresent(renderer);

  SDL_Quit();

  return 0;
}

