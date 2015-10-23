#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <assert.h>
#include <emscripten.h>
#include <unistd.h>
#include <stdlib.h>

int testImage(SDL_Surface* screen, const char* fileName) {
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

  int w, h;
  char *data = emscripten_get_preloaded_image_data(fileName, &w, &h);

  assert(data);
  assert(w == image->w);
  assert(h == image->h);

  SDL_FreeSurface (image);
  free(data);

  return result;
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_SWSURFACE);

  int result = 0;

  result |= testImage(screen, SCREENSHOT_DIRNAME "/" SCREENSHOT_BASENAME); // absolute path
  assert(result != 0);

  chdir(SCREENSHOT_DIRNAME);
  result = testImage(screen, "./" SCREENSHOT_BASENAME); // relative path
  assert(result != 0);

  SDL_Flip(screen);

  printf("you should see an image.\n");

  SDL_Quit();

#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif

  return 0;
}

