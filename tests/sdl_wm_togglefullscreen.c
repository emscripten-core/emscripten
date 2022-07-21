/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>

int result = 1;

SDL_Surface *screen = 0;

int inFullscreen = 0;

int wasFullscreen = 0;

void render() {
  int width, height;
  emscripten_get_canvas_element_size("#canvas", &width, &height);
  SDL_Rect rect = { 0, 0, width, height };
  SDL_FillRect(screen, &rect, 0xff00ffff);
}

void mainloop() {
  render();

  SDL_Event event;
  int isInFullscreen = EM_ASM_INT(return !!(document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement));
  if (isInFullscreen && !wasFullscreen) {
    printf("Successfully transitioned to fullscreen mode!\n");
    wasFullscreen = isInFullscreen;
  }

  if (wasFullscreen && !isInFullscreen) {
    printf("Exited fullscreen. Test succeeded.\n");
    emscripten_force_exit(0);
    return;
  }

  int haveEvent = SDL_PollEvent(&event);
  if (haveEvent) {
    switch(event.type) {
      case SDL_MOUSEBUTTONDOWN: {
        SDL_WM_ToggleFullScreen(screen);
        inFullscreen = 1 - inFullscreen;
        if (inFullscreen == 0) {
          result = wasFullscreen;
          if (result) {
            printf("Exited fullscreen. Test succeeded.\n");
          } else {
            printf("Exited fullscreen. Test failed, fullscreen transition did not happen!\n");
          }
          assert(result);
          emscripten_force_exit(result ? 0 : 1);
        } else {
          printf("Entering fullscreen...\n");
        }
        break;
      }
    }
  }
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);

  printf("You should see a yellow canvas.\n");
  printf("Click on the canvas to enter full screen, and then click on the canvas again to finish the test.\n");
  printf("When in full screen, you should see the whole screen filled yellow, and after exiting, the yellow canvas should be restored in the window.\n");
  emscripten_set_main_loop(mainloop, 0, 0);
  return 0;
}
