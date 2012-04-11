#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <assert.h>
#include <emscripten.h>

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_SWSURFACE);

  SDL_Surface *image = IMG_Load("screenshot.jpg");
  if (!image)
  {
     printf("IMG_Load: %s\n", IMG_GetError());
     return 1;
  }
  assert(image->format->BitsPerPixel == 32);
  assert(image->format->BytesPerPixel == 4);
  assert(image->pitch == 4*image->w);

  SDL_BlitSurface (image, NULL, screen, NULL);
  SDL_FreeSurface (image);

  SDL_Flip(screen); 

  printf("you should see an image.\n");

  SDL_Quit();

  int result = image->w;
  REPORT_RESULT();

  return 0;
}

