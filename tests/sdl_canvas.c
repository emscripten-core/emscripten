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

  SDL_Color color = { 0xff, 0x99, 0x00, 0x4f };

  SDL_Surface *text = TTF_RenderText_Solid(font, "hello faint orange world", color);

  SDL_Color color2 = { 0xbb, 0, 0xff, 0xff };
  SDL_Surface *text2 = TTF_RenderText_Solid(font, "a second line, purple", color2);

  // render
  SDL_Rect dest = { 0, 50, 0, 0 };
  SDL_BlitSurface (text, NULL, screen, NULL);
  dest.y = 100;
  SDL_BlitSurface (text2, NULL, screen, &dest);

  // fill stuff
  SDL_Rect rect = { 200, 200, 175, 125 };
  SDL_FillRect(screen, &rect, 0x2222ffff);

  SDL_Flip(screen); 

  SDL_LockSurface(screen);
  int sum = 0;
  for (int i = 0; i < screen->h; i++) {
    sum += *((char*)screen->pixels + i*screen->w*4 + i*4 + 0);
  }
  printf("Sum: %d\n", sum);

  printf("you should see two lines of text in different colors and a blue rectangle\n");

  SDL_Quit();

  int result = sum > 3000 && sum < 5000; // varies a little on different browsers, font differences?
  REPORT_RESULT();

  return 0;
}

