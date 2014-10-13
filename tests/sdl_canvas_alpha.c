#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <emscripten.h>


int main(int argc, char **argv) {
  Uint32 SRC_FLAG = SDL_SRCALPHA;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  if (argc > 1 && strcmp(argv[1], "-0") == 0) {
    SRC_FLAG = 0;
    SDL_FillRect(screen, 0, 0xFF00FF00);
  }

  printf("Init: %d\n", TTF_Init());

  TTF_Font *font = TTF_OpenFont("sans-serif", 40);
  printf("Font: %p\n", font);

  SDL_Color color = { 0xff, 0x99, 0x00, 0xff };
  SDL_Surface *text = TTF_RenderText_Solid(font, "hello orange world", color);

  // render
  for (int i = 0; i < 255; i++) {
    SDL_Rect dest = { i, i, 0, 0 };
    SDL_SetAlpha(text, SRC_FLAG, (((float)i)/255)*(((float)i)/255)*255);
    SDL_BlitSurface (text, NULL, screen, &dest);
  }

  SDL_Flip(screen); 

  SDL_LockSurface(screen);

  int width, height, isFullscreen;
  emscripten_get_canvas_size(&width, &height, &isFullscreen);

  if (width != 600 && height != 450)
  {
    printf("error: wrong width/height\n");
    abort();
  }

  SDL_Quit();

  printf("done.\n");

  return 0;
}

