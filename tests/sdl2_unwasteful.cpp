#include <SDL.h>
#include <emscripten.h>

#define TOTAL_RUNS 20

static SDL_Window *window;
static SDL_Surface *surface;

static void sdlError(const char *str) 
{
    fprintf(stderr, "Error at %s: %s\n", str, SDL_GetError());
    emscripten_force_exit(1);
}

static void main_loop(void)
{
    static unsigned int runs = 0;

    // verify that the ctx and image do not change from frame to frame
    if (runs == 1) {
        EM_ASM({
            Module.the_ctx = Module.SDL2.ctx;
            Module.the_image = Module.SDL2.image;
            Module.print('start with ' + [Module.the_ctx, Module.the_image]);
            assert(!!Module.the_ctx && (typeof Module.the_ctx === 'object'), 'typeof ctx');
            assert(!!Module.the_image && (typeof Module.the_image === 'object'), 'typeof image');
            Module.print('set values');
        });
    } else if (runs > 1) {
        EM_ASM_ARGS({
            assert(Module.the_ctx === Module.SDL2.ctx, 'ctx');
            assert(Module.the_image === Module.SDL2.image, 'image');
            Module.print('check ok ' + $0);
        }, runs);
    }

    int i;
    if (SDL_LockSurface(surface) != 0) sdlError("SDL_LockSurface");
    SDL_UnlockSurface(surface); // does not return a value
    if (SDL_UpdateWindowSurface(window) != 0)
        sdlError("SDL_UpdateWindowSurface");

    runs++;
    if (runs >= TOTAL_RUNS) {
        emscripten_cancel_main_loop();
        int result = 1;
        REPORT_RESULT();
    }
}

int main(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) sdlError("SDL_Init");

    window = SDL_CreateWindow("SDL 2 test",
                              0, 0,
                              640, 480,
                              SDL_WINDOW_SHOWN);
    if (window == NULL) sdlError("SDL_CreateWindow");
    surface = SDL_GetWindowSurface(window);
    if (surface == NULL) sdlError("SDL_GetWindowSurface");

    emscripten_set_main_loop(main_loop, 0, 1);
}

