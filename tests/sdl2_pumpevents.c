#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include <emscripten.h>
// bug - SDL_GetKeyboardState doesn't return scancodes, it returns keycodes, so acts exactly like
// SDL_GetKeyState instead
#define SDL_GetKeyState SDL_GetKeyboardState

int result = 0;

int loop1()
{
  printf("loop1\n");
  unsigned i;
  int r = 0;

  // method 1: SDL_PollEvent loop
  SDL_Event e;
  while (SDL_PollEvent(&e));

  const Uint8 *keys = SDL_GetKeyState(NULL);
  if (keys[SDL_SCANCODE_LEFT])
    r = 1;

  return r;
}

int loop2()
{
  printf("loop2\n");
  
  unsigned i;
  int r = 0;
   
  // method 2: SDL_PumpEvents
  SDL_PumpEvents();

  const Uint8 *keys = SDL_GetKeyState(NULL);
  if (keys[SDL_SCANCODE_RIGHT])
    r = 2;

  return r;
}

int alphakey()
{
  printf("alpha\n");
  
  unsigned i;
  int r = 0;

  SDL_PumpEvents();

  const Uint8 *keys = SDL_GetKeyState(NULL);
  if (keys[SDL_SCANCODE_A])
    r = 4;

  return r;
}

int main(int argc, char *argv[])
{
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_CreateWindowAndRenderer(600, 450, 0, &window, &renderer);
    
  emscripten_run_script("keydown(37);"); // left
  result += loop1();
  emscripten_run_script("keydown(39);"); // right
  result += loop2();
  emscripten_run_script("keydown(65);"); // A
  result += alphakey();
  REPORT_RESULT();
  return 0;
}
