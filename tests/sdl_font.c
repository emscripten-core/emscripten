#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <emscripten.h>

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  printf("Init: %d\n", TTF_Init());

  TTF_Font *font = TTF_OpenFont("myfont.ttf", 40);
  printf("Font: %p\n", font);

  SDL_Color color = { 0xff, 0x99, 0x00, 0x77 };

  SDL_Surface *text = TTF_RenderText_Solid(font, "hello orange world", color);
  SDL_LockSurface(text);
  int sum = 0;
  for (int i = 0; i < text->h; i++) {
    sum += *((char*)text->pixels + i*text->w*4 + i*4 + 0);
  }
  printf("Sum: %d\n", sum);
  SDL_UnlockSurface(text);

  SDL_Color color2 = { 0xbb, 0, 0xff, 0 };
  SDL_Surface *text2 = TTF_RenderText_Solid(font, "a second line, purple", color2);

  // render
  SDL_Rect dest = { 0, 50, 0, 0 };
  SDL_BlitSurface (text, NULL, screen, NULL);
  dest.y = 100;
  SDL_BlitSurface (text2, NULL, screen, &dest);

  SDL_Flip(screen); 

  printf("you should see two lines of text in different colors.\n");

  SDL_Quit();

  int result = sum;
  REPORT_RESULT();

  return 0;
}

