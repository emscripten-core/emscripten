#include "SDL2/SDL.h"
#include "SDL2/SDL2_gfxPrimitives.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);

    const int width = 400;
    const int height = 400;

    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);

    //SDL_Surface *screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
    boxColor(renderer, 0, 0, width, height, 0xff);

    boxColor(renderer, 0, 0, 98, 98, 0xff0000ff);
    boxRGBA(renderer, 100, 0, 198, 98, 0, 0, 0xff, 0xff);
    // check that the x2 > x1 case is handled correctly
    boxColor(renderer, 298, 98, 200, 0, 0x00ff00ff);
    boxColor(renderer, 398, 98, 300, 0, 0xff0000ff);

    rectangleColor(renderer, 0, 100, 98, 198, 0x000ffff);
    rectangleRGBA(renderer, 100, 100, 198, 198, 0xff, 0, 0, 0xff);

    ellipseColor(renderer, 300, 150, 99, 49, 0x00ff00ff);
    filledEllipseColor(renderer, 100, 250, 99, 49, 0x00ff00ff);
    filledEllipseRGBA(renderer, 250, 300, 49, 99, 0, 0, 0xff, 0xff);

    lineColor(renderer, 300, 200, 400, 300, 0x00ff00ff);
    lineRGBA(renderer, 300, 300, 400, 400, 0, 0xff, 0, 0xff);

    //SDL_UpdateRect(screen, 0, 0, 0, 0);
    SDL_RenderPresent(renderer);

#ifndef __EMSCRIPTEN__
    SDL_Event evt;
    /*SDL_SaveBMP(screen, "native_output.bmp");*/
    while (1) {
       if (SDL_PollEvent(&evt) != 0 && evt.type == SDL_QUIT) break;
       SDL_Delay(33);
    }
#endif

    SDL_Quit();

    return 1;
}
