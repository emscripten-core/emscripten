#include <stdio.h>
#include <SDL/SDL.h>


int main() {
  printf("hello, world!\n");

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);

  if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
  for (int i = 0; i < 256; i++) {
    for (int j = 0; j < 256; j++) {
      *((char*)screen->pixels + i*256*4 + j*4 + 0) = i;
      *((char*)screen->pixels + i*256*4 + j*4 + 1) = j;
      *((char*)screen->pixels + i*256*4 + j*4 + 2) = 255-i;
      *((char*)screen->pixels + i*256*4 + j*4 + 3) = (i+j)%255; // actually ignored, since this is to the screen
    }
  }
  if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
  SDL_Flip(screen); 

  printf("you should see a smoothly-colored square - no sharp lines but the square borders!\n");
  printf("and here is some text that should be HTML-friendly: amp: |&| double-quote: |\"| quote: |'| less-than, greater-than, html-like tags: |<cheez></cheez>|\nanother line.\n");

  SDL_Quit();

  return 0;
}

