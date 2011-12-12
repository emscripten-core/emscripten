#include <stdio.h>
#include <SDL/SDL.h>


int main() {
  printf("hello, world!\n");

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);

  SDL_LockSurface(screen);
  for (int i = 0; i < 256; i++) {
    for (int j = 0; j < 256; j++) {
      *((char*)screen->pixels + i*256*4 + j*4 + 0) = i;
      *((char*)screen->pixels + i*256*4 + j*4 + 1) = j;
      *((char*)screen->pixels + i*256*4 + j*4 + 2) = 255-i;
      *((char*)screen->pixels + i*256*4 + j*4 + 3) = 255;
    }
  }
  SDL_UnlockSurface(screen);
  SDL_Flip(screen); 

  printf("you should see a colored cube.");

  // SDL_Quit(); // Don't call SDL_Quit so that the canvas is not cleared

  return 0;
}

