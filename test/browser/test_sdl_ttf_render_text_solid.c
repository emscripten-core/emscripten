/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface *screen = SDL_SetVideoMode(320, 32, 32, SDL_HWSURFACE);

    TTF_Init();
    TTF_Font *font = TTF_OpenFont("Arial", 32);

    SDL_Color color = { 0xff, 0x99, 0x00, 0xff };
    SDL_Surface *text = TTF_RenderText_Solid(font, "Play", color);
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 0, 0));
    SDL_BlitSurface(text, NULL, screen, NULL);
    return 0;
}

