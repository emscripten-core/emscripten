/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>

SDL_Window *window;
SDL_Renderer *renderer;
TTF_Font *font;

void render()
{
    static SDL_Color colorA = { 0xff, 0x99, 0x00, 0xff };
    static SDL_Color colorB = { 0x11, 0xff, 0xff, 0xff };
    static SDL_Rect upperRect = {0, 0, 640, 240};
    static SDL_Rect lowerRect = {0, 240, 640, 240};

    TTF_SetFontDirection(font, TTF_DIRECTION_RTL);
    TTF_SetFontScriptName(font, "Arab");

    SDL_Surface *helloSurface = TTF_RenderUTF8_Shaded(font, "سلام", colorA, colorB);
    SDL_Surface *worldSurface = TTF_RenderUTF8_Shaded(font, "جهان", colorB, colorA);
    SDL_Texture *helloTexture = SDL_CreateTextureFromSurface(renderer, helloSurface);
    SDL_Texture *worldTexture = SDL_CreateTextureFromSurface(renderer, worldSurface);

    SDL_SetRenderDrawColor(renderer, 0xcc, 0xcc, 0xcc, 0xff);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, helloTexture, NULL, &upperRect);
    SDL_RenderCopy(renderer, worldTexture, NULL, &lowerRect);
    SDL_RenderPresent(renderer);

    SDL_FreeSurface(helloSurface);
    SDL_FreeSurface(worldSurface);
    SDL_DestroyTexture(helloTexture);
    SDL_DestroyTexture(worldTexture);
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_CreateWindowAndRenderer(640, 480, 0, &window, &renderer);
    font = TTF_OpenFont("NotoNaskhArabic-Regular.ttf", 40);
    render();
}
