#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <assert.h>
#include <emscripten.h>

SDL_Surface* screen;

int testImage(const char* fileName) {
  printf("IMG_Load: %s\n", fileName);
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

  SDL_BlitSurface (image, NULL, screen, NULL);
  SDL_FreeSurface (image);

  return result;
}

void ready(char *data, const char *fileName) {
  printf("ready! %s\n", fileName);

  static int first = 1;
  static const char *seenName;
  if (first) {
    first = 0;
    seenName = fileName;
  } else {
    printf("%s ? %s == %d\n", fileName, seenName, strcmp(fileName, seenName));
    assert(strcmp(fileName, seenName)); // different names

    testImage(seenName);

    free(seenName); // As the API docs say, we are responsible for freeing the 'fake' names we are given

    SDL_Flip(screen);
  }
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_SetVideoMode(600, 450, 32, SDL_SWSURFACE);

  printf("prepare..\n");

  #define SIZE 203164
  FILE *f = open("screenshot.not", "rb");
  char *buffer = malloc(SIZE);
  fread(buffer, SIZE, 1, f);
  fclose(f);

  emscripten_async_prepare_data(buffer, SIZE, "jpg", ready, NULL);
  emscripten_async_prepare_data(buffer, SIZE, "jpg", ready, NULL); // twice to see different filenames

  return 0;
}

