#include <stdlib.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <emscripten.h>

#define SCREEN_W 360
#define SCREEN_H 360

int main(int argc, char *argv[])
{
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window;
  SDL_Renderer *renderer;

  SDL_CreateWindowAndRenderer(SCREEN_W, SCREEN_H, 0, &window, &renderer);

  SDL_Surface *screen = SDL_CreateRGBSurface(0, SCREEN_W, SCREEN_H, 8, 0, 0, 0, 0);

  SDL_LockSurface(screen);

  SDL_FillRect(screen, NULL, 0x000088); 

  for (int i = 0; i < 40; ++i) {
    int x0 = rand() % SCREEN_W;
    int y0 = rand() % SCREEN_H;
    int x1 = rand() % SCREEN_W;
    int y1 = rand() % SCREEN_H;
    lineColor(renderer, x0, y0, x1, y1, ((rand() % 0xFF) << ((rand() % 3) * 8)) | 0xFF000000);
  }

  roundedRectangleColor(renderer, SCREEN_W * 0.3, SCREEN_H * 0.3, SCREEN_W * 0.7, SCREEN_H * 0.7, SCREEN_W * 0.05, 0xFFFFFF00);

  SDL_UnlockSurface(screen);
}


