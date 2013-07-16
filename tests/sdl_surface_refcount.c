#include <SDL.h>

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface *screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    SDL_Surface *reference = screen;
    reference->refcount++;
    SDL_FreeSurface(screen);
    SDL_FreeSurface(reference);
    int result = 1;
    REPORT_RESULT();
    return 0;
}
