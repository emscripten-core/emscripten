#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_rotozoom.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

const int numSprites = 9;
SDL_Surface *screen;
SDL_Surface *sprite[numSprites];

void mainloop() {
    int i;
    int row = 0;
    SDL_Rect rect = { 0, 0, 100, 100 };
    for (i = 0; i < numSprites; i++) {
        rect.x = i & 1 ? 200 : 0;
        rect.y = row * 200;
        row += i & 1;
        SDL_BlitSurface(sprite[i], 0, screen, &rect);
        SDL_UpdateRect(screen, 0, 0, 0, 0);
    }
}

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);

    const int width = 400;
    const int height = 200 * (numSprites + 1) / 2;
    screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
    SDL_Rect rect = { 0, 0, width, height };
    SDL_FillRect(screen, &rect, SDL_MapRGBA(screen->format, 0, 0, 0, 0xff));

    sprite[0] = IMG_Load("screenshot.png");
    sprite[1] = SDL_CreateRGBSurface(SDL_SWSURFACE, 100, 100, 32, 0xFF000000, 0xFF0000, 0xFF00, 0xFF);
    SDL_FillRect(sprite[1], 0, 0xA0A0A0A0);
    sprite[2] = zoomSurface(sprite[0], 0.5, 0.5, SMOOTHING_ON);
    sprite[3] = zoomSurface(sprite[1], 0.5, 0.5, SMOOTHING_ON);
    sprite[4] = rotozoomSurface(sprite[0], -20, 0.3, SMOOTHING_ON);
    sprite[5] = rotozoomSurface(sprite[1], 20, 1, SMOOTHING_ON);
    sprite[6] = zoomSurface(sprite[0], -0.5, 0.5, SMOOTHING_ON);
    sprite[7] = zoomSurface(sprite[0], -0.5, -0.5, SMOOTHING_ON);
    sprite[8] = rotozoomSurface(sprite[1], 0, 0.5, SMOOTHING_ON);

    mainloop();

#ifndef __EMSCRIPTEN__
    SDL_Event evt;
    SDL_SaveBMP(screen, "native_output.bmp");
    while (1) {
       if (SDL_PollEvent(&evt) != 0 && evt.type == SDL_QUIT) break;
       //mainloop();
       SDL_Delay(33);
    }
#endif

    SDL_Quit();

    return 1;
}
