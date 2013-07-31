#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <assert.h>
#include <emscripten.h>

#define SIZE 203164
SDL_Surface* screen;
char *buffer;

int testImage() {
  SDL_Surface *image = IMG_Load_RW(SDL_RWFromMem(buffer, SIZE), 1);
  if (!image)
  {
     printf("IMG_Load: %s\n", IMG_GetError());
     return 0;
  }

  printf("load succeeded\n");

  assert(image->format->BitsPerPixel == 32);
  assert(image->format->BytesPerPixel == 4);
  assert(image->pitch == 4*image->w);
  int result = image->w;

  SDL_BlitSurface (image, NULL, screen, NULL);
  SDL_FreeSurface (image);

  return result;
}

void ready() {
  testImage();

  SDL_Flip(screen);
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_SetVideoMode(600, 450, 32, SDL_SWSURFACE);

  printf("prepare..\n");

  FILE *f = fopen("screenshot.not", "rb");
  buffer = malloc(SIZE);
  fread(buffer, SIZE, 1, f);
  fclose(f);

  ready();

  return 0;
}

