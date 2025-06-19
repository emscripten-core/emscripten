/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

int main(int argc, char *argv[])
{
  int width = 320;
  int height = 48;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("SDL3 TTF Render Text Solid", width, height, 0);
  SDL_Surface *screen = SDL_GetWindowSurface(window);
  
  TTF_Init();
  TTF_Font *font = TTF_OpenFont("LiberationSansBold.ttf", 32);
  if (!font) {
    printf("TTF_OpenFont: %s\n", SDL_GetError());
    return 1;
  }
  
  SDL_Color color = { 0xff, 0x99, 0x00, 0xff };
  SDL_Surface *text = TTF_RenderText_Solid(font, "Play", 0, color);
  
  SDL_FillSurfaceRect(screen, NULL, SDL_MapRGB(SDL_GetPixelFormatDetails(screen->format), NULL, 255, 0, 0));
  SDL_BlitSurface(text, NULL, screen, NULL);
  SDL_UpdateWindowSurface(window);
  
  return 0;
}
