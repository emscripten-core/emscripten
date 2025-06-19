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

SDL_Window *window;
SDL_Renderer *renderer;
TTF_Font *font;

#define WIDTH 640
#define HEIGHT 480

void render()
{

    SDL_Color colorA = { 0xff, 0x99, 0x00, 0xff };
    SDL_Color colorB = { 0x11, 0xff, 0xff, 0xff };
    SDL_FRect upperRect = {0, 0, WIDTH, HEIGHT / 2};
    SDL_FRect lowerRect = {0, HEIGHT / 2, WIDTH, HEIGHT / 2};

    SDL_Surface *helloSurface = TTF_RenderText_Shaded(font, "hello", 0, colorA, colorB);
    SDL_Surface *worldSurface = TTF_RenderText_Shaded(font, "world", 0, colorB, colorA);
    SDL_Texture *helloTexture = SDL_CreateTextureFromSurface(renderer, helloSurface);
    SDL_Texture *worldTexture = SDL_CreateTextureFromSurface(renderer, worldSurface);

    SDL_SetRenderDrawColor(renderer, 0xcc, 0xcc, 0xcc, 0xff);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, helloTexture, NULL, &upperRect);
    SDL_RenderTexture(renderer, worldTexture, NULL, &lowerRect);
    SDL_RenderPresent(renderer);

    SDL_DestroySurface(helloSurface);
    SDL_DestroySurface(worldSurface);
    SDL_DestroyTexture(helloTexture);
    SDL_DestroyTexture(worldTexture);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    
    if (!SDL_CreateWindowAndRenderer("SDL3 TTF", WIDTH, HEIGHT, 0, &window, &renderer)) {
        printf("SDL_CreateWindowAndRenderer: %s\n", SDL_GetError());
        return 1;
    }
    font = TTF_OpenFont("LiberationSansBold.ttf", 40);
    if (!font) {
        printf("TTF_OpenFont: %s\n", SDL_GetError());
        return 1;
    }
    render();

    return 0;
}