#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  SDL_LockSurface(screen);
  unsigned int *pixels = (unsigned int *)screen->pixels;
  for (int x = 0; x < screen->w; x++) {
    for (int y = 0; y < screen->h; y++) {
      pixels[x + y*screen->h] = x < 300 ? (y < 200 ? 0x3377AA88 : 0xAA3377CC) : (y < 200 ? 0x0066AA77 : 0xAA006699);
    }
  }
  SDL_UnlockSurface(screen);

  SDL_Quit();

  printf("done.\n");

  int result = 1;
  REPORT_RESULT();

  return 0;
}

