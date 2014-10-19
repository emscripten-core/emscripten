#include "SDL2/SDL.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <emscripten.h>

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
    int w, h, fs;
    emscripten_get_canvas_size(&w, &h, &fs);
    printf("w:%d,h:%d\n", w,h);
    assert(w == 700);
    assert(h == 200);

    // Test 3: Check that resizing the canvas works as well.
    emscripten_set_canvas_size(640, 480);
    emscripten_get_canvas_size(&w, &h, &fs);
    printf("w:%d,h:%d\n", w,h);
    assert(w == 640);
    assert(h == 480);

    SDL_DestroyWindow(window);
    SDL_Quit();
    result = 1;
    REPORT_RESULT();

    return 0;
}
