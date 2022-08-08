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

int result = 0;

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Surface *surface;
    SDL_Cursor *cursor;

    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(
        "sdl2_custom_cursor",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        800,
        600,
        0
    );

    surface = SDL_LoadBMP("cursor.bmp");
    cursor = SDL_CreateColorCursor(surface, 0, 0);

    assert(cursor != NULL);

    SDL_SetCursor(cursor);

    int cursor_updated = EM_ASM_INT(
        return Module['canvas'].style['cursor'].startsWith("url(");
    );

    assert(cursor_updated != 0);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
