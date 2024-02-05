/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "SDL/SDL.h"
#include "SDL/SDL_gfxPrimitives.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);

    const int width = 400;
    const int height = 400;
    SDL_Surface *screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
    boxColor(screen, 0, 0, width, height, 0xff);

    boxColor(screen, 0, 0, 98, 98, 0xff0000ff);
    boxRGBA(screen, 100, 0, 198, 98, 0, 0, 0xff, 0xff);
    // check that the x2 > x1 case is handled correctly
    boxColor(screen, 298, 98, 200, 0, 0x00ff00ff);
    boxColor(screen, 398, 98, 300, 0, 0xff0000ff);

    rectangleColor(screen, 0, 100, 98, 198, 0x000ffff);
    rectangleRGBA(screen, 100, 100, 198, 198, 0xff, 0, 0, 0xff);

    ellipseColor(screen, 300, 150, 99, 49, 0x00ff00ff);
    filledEllipseColor(screen, 100, 250, 99, 49, 0x00ff00ff);
    filledEllipseRGBA(screen, 250, 300, 49, 99, 0, 0, 0xff, 0xff);

    lineColor(screen, 300, 200, 400, 300, 0x00ff00ff);
    lineRGBA(screen, 300, 300, 400, 400, 0, 0xff, 0, 0xff);

    SDL_UpdateRect(screen, 0, 0, 0, 0);

#ifndef __EMSCRIPTEN__
    SDL_Event evt;
    SDL_SaveBMP(screen, "native_output.bmp");
    while (1) {
       if (SDL_PollEvent(&evt) != 0 && evt.type == SDL_QUIT) break;
       SDL_Delay(33);
    }
#endif

    SDL_Quit();

    return 1;
}
