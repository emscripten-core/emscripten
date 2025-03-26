/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <emscripten.h>
#include <string.h>
#include <SDL/SDL.h>

void report_result(int result) {
  if (result == 0) {
    printf("Test successful!\n");
  } else {
    printf("Test failed!\n");
  }
  emscripten_force_exit(result);
}

#define TEST_RESULT(x) if (ret != EMSCRIPTEN_RESULT_SUCCESS) printf("%s returned %s.\n", #x, emscripten_result_to_string(ret));

int gotWheelUp = 0;
int gotWheelButtonUp = 0;
int gotWheelDown = 0;
int gotWheelButtonDown = 0;
int gotWheelClick = 0;

void instruction() {
  if (!gotWheelUp || !gotWheelButtonUp) printf("Please scroll the mouse wheel upwards by a single notch (slowly move your finger away from you towards the display).\n");
  else if (!gotWheelDown || !gotWheelButtonDown) printf("Please scroll the mouse wheel downwards by a single notch (slowly move your finger towards you).\n");
  else if (!gotWheelClick) printf("Please click the wheel button.\n");
  else if (gotWheelUp && gotWheelButtonUp && gotWheelDown && gotWheelButtonDown && gotWheelClick) report_result(0);
}

void main_tick() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_MOUSEWHEEL:
        printf("SDL_MOUSEWHEEL: timestamp: %u, windowID: %u, which: %u, x: %d, y: %d\n", event.wheel.timestamp, event.wheel.windowID, event.wheel.which, event.wheel.x, event.wheel.y);
        if (!gotWheelUp)
        {
          if (event.wheel.y > 0 && event.wheel.y < 2) { gotWheelUp = 1; instruction(); }
          else if (event.wheel.y >= 2) { printf("The scroll amount was too large. Either you scrolled very fast or the normalization is not working."); report_result(1); }
          else if (event.wheel.y < 0) { printf("You scrolled to the wrong direction (downwards)!\n"); report_result(1); }
        }
        else if (!gotWheelDown)
        {
          if (event.wheel.y < 0 && event.wheel.y > -2) { gotWheelDown = 1; instruction(); }
          else if (event.wheel.y <= -2) { printf("The scroll amount was too large. Either you scrolled very fast or the normalization is not working."); report_result(1); }
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        printf("SDL_MOUSEBUTTONDOWN: button: %d\n", event.button.button);
        if (event.button.button == SDL_BUTTON_WHEELDOWN) { printf("SDL_BUTTON_WHEELDOWN\n"); gotWheelButtonDown = 1; instruction(); }
        else if (event.button.button == SDL_BUTTON_WHEELUP) { printf("SDL_BUTTON_WHEELUP\n"); gotWheelButtonUp = 1; instruction(); }
        else if (event.button.button == SDL_BUTTON_MIDDLE) { printf("SDL_BUTTON_MIDDLE\n"); gotWheelClick = 1; instruction(); }
        break;
    }
  }
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);
  SDL_FillRect(screen, NULL, 0xFF000000);
  instruction();
  emscripten_set_main_loop(main_tick, 0, 0);

#ifdef AUTOMATE_SUCCESS
  EM_ASM(
    function sendEvent(type, data) {
      var event = document.createEvent('Event');
      event.initEvent(type, true, true);
      for (var d in data) event[d] = data[d];
      Module['canvas'].dispatchEvent(event);
    }
    // Scroll up by a tiny amount.
    sendEvent('wheel', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 0, buttons: 0, 'deltaX': 0, 'deltaY': -0.1, 'deltaZ': 0, 'deltaMode': 1 });
    // Scroll down by a tiny amount.
    sendEvent('wheel', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 0, buttons: 0, 'deltaX': 0, 'deltaY': 0.1, 'deltaZ': 0, 'deltaMode': 1 });
    // Press mouse middle button.
    sendEvent('mousedown', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 1, buttons: 2 });
  );
#endif
}
