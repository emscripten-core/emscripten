#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_rotozoom.h"

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

SDL_Surface *screen;
SDL_Surface *sprite[6];

void mainloop() {
    int i;
    SDL_Rect rect = { 0, 0, 100, 100 };
    for (i = 0; i < 6; i++) {
        rect.x = i & 1 ? 200 : 0;
        rect.y = i & 2 ? 200 : 0;
        SDL_BlitSurface(sprite[i], 0, screen, &rect);
        SDL_UpdateRect(screen, 0, 0, 0, 0);
    }
}

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);

    screen = SDL_SetVideoMode(400, 400, 32, SDL_SWSURFACE);

    sprite[0] = IMG_Load("example.png");
    sprite[1] = SDL_CreateRGBSurface(SDL_SWSURFACE, 100, 100, 32, 0xFF000000, 0xFF0000, 0xFF00, 0xFF);
    SDL_FillRect(sprite[1], 0, 0xA0A0A0A0);
    sprite[2] = zoomSurface(sprite[0], 0.5, 0.5, SMOOTHING_ON);
    sprite[3] = zoomSurface(sprite[1], 0.5, 0.5, SMOOTHING_ON);
    sprite[4] = rotozoomSurface(sprite[0], -20, 0.3, SMOOTHING_ON);
    sprite[5] = rotozoomSurface(sprite[1], 45, 0.5, SMOOTHING_ON);

    mainloop();

#ifndef EMSCRIPTEN
    SDL_Event evt;
    while (1) {
       if (SDL_PollEvent(&evt) != 0 && evt.type == SDL_QUIT) break;
       //mainloop();
       SDL_Delay(33);
    }
#endif

    SDL_Quit();

    return 1;
}
