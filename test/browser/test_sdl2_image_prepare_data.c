/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <emscripten.h>

SDL_Renderer *renderer;

int testImage(const char* fileName) {
  SDL_Surface *image = IMG_Load(fileName);
  if (!image) {
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

void ready(void *arg, const char *fileName) {
  printf("ready! %s (%ld)\n", fileName, (long)arg);

  static int first = 1;
  static const char *seenName;
  static void *seenArg;
  if (first) {
    first = 0;
    seenName = fileName;
    seenArg = arg;
  } else {
    printf("%s ? %s == %d\n", fileName, seenName, strcmp(fileName, seenName));
    assert(strcmp(fileName, seenName)); // different names

    assert(seenArg != arg); // different args

    testImage(seenName);

    free((void*)seenName); // As the API docs say, we are responsible for freeing the 'fake' names we are given

    SDL_RenderPresent(renderer);

    EM_ASM(reftestUnblock());
  }
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window;

  SDL_CreateWindowAndRenderer(600, 450, 0, &window, &renderer);

  printf("prepare..\n");

  #define SIZE 203164
  FILE *f = fopen("screenshot.not", "rb");
  char *buffer = malloc(SIZE);
  fread(buffer, SIZE, 1, f);
  fclose(f);

  emscripten_run_preload_plugins_data(buffer, SIZE, "jpg", (void*)25, ready, NULL);
  emscripten_run_preload_plugins_data(buffer, SIZE, "jpg", (void*)33, ready, NULL); // twice to see different filenames

  EM_ASM(reftestBlock());
  return 0;
}

