#include <emscripten.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

int is_surface_freed(SDL_Surface *surface)
{
    const char *template = "!SDL.surfaces[%d]";
    int length = snprintf(NULL, 0, template, surface) + 1;
    char *script = malloc(length * sizeof(char));
    snprintf(script, length, template, surface);
    int is_freed = emscripten_run_script_int(script);
    free(script);
    return is_freed;
}

int main(int argc, char *argv[])
{
    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 10, 10, 32,
                                                0, 0, 0, 0);
    SDL_Surface *reference = surface;
    reference->refcount++;
    SDL_FreeSurface(surface);
    SDL_FreeSurface(reference);
    int result = is_surface_freed(surface);
    REPORT_RESULT();
    return 0;
}
