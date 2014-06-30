#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_opengl.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <emscripten.h>

int result = 1;

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

    SDL_Quit();
    REPORT_RESULT();
    return 0;
}
