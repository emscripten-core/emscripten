#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <emscripten.h>

SDL_Renderer *renderer;

int testImage(const char* fileName) {
  SDL_Surface *image = IMG_Load(fileName);
  if (!image)
  {
     printf("IMG_Load: %s\n", IMG_GetError());
     return 0;
  }
  assert(image->format->BitsPerPixel == 32);
  assert(image->format->BytesPerPixel == 4);
  assert(image->pitch == 4*image->w);
  int result = image->w;

  SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, image);

  SDL_RenderCopy (renderer, tex, NULL, NULL);

  SDL_DestroyTexture (tex);
  SDL_FreeSurface (image);

  return result;
}

void ready(const char *f) {
  printf("ready!\n");

  testImage("screenshot.jpg"); // relative path

  SDL_RenderPresent(renderer);
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window;

  SDL_CreateWindowAndRenderer(600, 450, 0, &window, &renderer);

  printf("rename..\n");

  rename("screenshot.not", "screenshot.jpg");

  printf("prepare..\n");

  assert(emscripten_run_preload_plugins("screenshot.jpg", ready, NULL) == 0);

  return 0;
}

