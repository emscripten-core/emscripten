/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "SDL2/SDL.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <emscripten.h>
#include <emscripten/html5.h>

int result = 0;

int main(int argc, char *argv[])
{
    SDL_Window *window;

    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

   // Test 1: Check that initializing video mode
   // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "sdlw_canvas_size",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        700,
        200,
        0
    );

    // Test 2: Check that getting current canvas size works.
    int w, h;
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    printf("w:%d,h:%d\n", w,h);
    assert(w == 700);
    assert(h == 200);

    // Test 3: Check that resizing the canvas works as well.
    emscripten_set_canvas_element_size("#canvas", 640, 480);
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    printf("w:%d,h:%d\n", w,h);
    assert(w == 640);
    assert(h == 480);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
