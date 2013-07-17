#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Surface* surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 10, 10, 32,
                                                0, 0, 0, 0);
    SDL_Surface *reference = surface;
    reference->refcount++;
    SDL_FreeSurface(surface);
    SDL_FreeSurface(reference);
    int result = surface->refcount;
    REPORT_RESULT();
    return 0;
}
