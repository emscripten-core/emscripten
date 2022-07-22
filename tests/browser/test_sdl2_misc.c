/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "SDL2/SDL.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <emscripten.h>

int main(int argc, char *argv[])
{
    SDL_Window *window;

    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(
        "sdl2_misc",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        100,
        100,
        0
    );

    EM_ASM({
      assert(document.title === 'sdl2_misc');
    });
    const char* intended = "a custom window title";
    SDL_SetWindowTitle(window, intended);
    const char* seen = SDL_GetWindowTitle(window);
    if (strcmp(intended, seen) != 0) {
        printf("Got a weird title back: %s\n", seen);
        return 1;
    }
    EM_ASM({
      assert(document.title === 'a custom window title');
    });

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
