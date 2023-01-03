/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <assert.h>
#include <SDL/SDL.h>
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
    assert(result);
    return 0;
}
