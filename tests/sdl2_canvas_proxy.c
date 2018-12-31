#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include <emscripten.h>

int main(int argc, char **argv) {
  FILE *f = fopen("data.txt", "rb");
  assert(f);
  assert(fgetc(f) == 'd');
  assert(fgetc(f) == 'a');
  assert(fgetc(f) == 't');
  assert(fgetc(f) == 'u');
  assert(fgetc(f) == 'm');
  fclose(f);

  SDL_Init(SDL_INIT_VIDEO);
  //SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  SDL_Window *window;
  SDL_Renderer *renderer;

  SDL_CreateWindowAndRenderer(600, 450, 0, &window, &renderer);

  SDL_Surface *screen = SDL_CreateRGBSurface(0, 600, 450, 32, 0, 0, 0, 0);

  SDL_LockSurface(screen);
  unsigned int *pixels = (unsigned int *)screen->pixels;
  for (int x = 0; x < screen->w; x++) {
    for (int y = 0; y < screen->h; y++) {
      pixels[x + y*screen->h] = x < 300 ? (y < 200 ? 0x3377AA88 : 0xAA3377CC) : (y < 200 ? 0x0066AA77 : 0xAA006699);
    }
  }
  SDL_UnlockSurface(screen);

  SDL_Texture *screenTexture = SDL_CreateTextureFromSurface(renderer, screen);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
  SDL_RenderPresent(renderer);

  SDL_Quit();

  EM_ASM(window.close());
  return 0;
}

