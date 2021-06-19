/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_opengl.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <emscripten.h>
#include <emscripten/html5.h>

int main(int argc, char *argv[])
{
    SDL_Surface *screen;

    // Slightly different SDL initialization
    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Test 1: Check that initializing video mode with size (0,0) will use the size from the <canvas> element.
    screen = SDL_SetVideoMode( 0, 0, 16, SDL_OPENGL ); // *changed*

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

    SDL_Quit();
    return 0;
}
