/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <assert.h>
#include <string.h>
#include <emscripten.h>

void assertJoystickEvent(int expectedGamepad, int expectedType, int expectedIndex, int expectedValue) {
  SDL_Event event;
  while(1) {
    // Loop ends either when assertion fails (we run out of events), or we find
    // the event we're looking for.
    assert(SDL_PollEvent(&event) == 1);
    if (event.type != expectedType) {
      continue;
    }
    switch(event.type) {
      case SDL_JOYAXISMOTION: {
        assert(event.jaxis.which == expectedGamepad);
        assert(event.jaxis.axis == expectedIndex);
        assert(event.jaxis.value == expectedValue);
        break;
      }
      case SDL_JOYBUTTONUP: case SDL_JOYBUTTONDOWN: {
        assert(event.jbutton.which == expectedGamepad);
        assert(event.jbutton.button == expectedIndex);
        assert(event.jbutton.state == expectedValue);
        break;
      }
    }
    // Break out of while loop.
    break;
  }
}

void assertNoJoystickEvent() {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_JOYBUTTONDOWN: case SDL_JOYBUTTONUP: case SDL_JOYAXISMOTION: {
        // Fail.
        assert(0);
      }
    }
  }
}

void main_2(void* arg);

int main() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);
  emscripten_async_call(main_2, NULL, 3000); // avoid startup delays and intermittent errors
  // Return code ignored since emscripten_async_call keeps the runtime alive
  return 99;
}

void main_2(void* arg) {
  // TODO: At the moment, we only support joystick support through polling.
  emscripten_run_script("window.addNewGamepad('Pad Thai', 4, 16)");
  emscripten_run_script("window.addNewGamepad('Pad Kee Mao', 0, 4)");
  // Check that the joysticks exist properly.
  assert(SDL_NumJoysticks() == 2);
  assert(!SDL_JoystickOpened(0));
  assert(!SDL_JoystickOpened(1));
  SDL_Joystick* pad1 = SDL_JoystickOpen(0);
  assert(SDL_JoystickOpened(0));
  assert(SDL_JoystickIndex(pad1) == 0);
  assert(strncmp(SDL_JoystickName(0), "Pad Thai", 9) == 0);
  assert(strncmp(SDL_JoystickName(1), "Pad Kee Mao", 12) == 0);
  assert(SDL_JoystickNumAxes(pad1) == 4);
  assert(SDL_JoystickNumButtons(pad1) == 16);

  // By default, SDL will automatically process events. Test this behavior, and then disable it.
  assert(SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE);
  SDL_JoystickEventState(SDL_DISABLE);
  assert(SDL_JoystickEventState(SDL_QUERY) == SDL_DISABLE);
  // Button events.
  emscripten_run_script("window.simulateGamepadButtonDown(0, 1)");
  // We didn't tell SDL to automatically update this joystick's state.
  assertNoJoystickEvent();
  SDL_JoystickUpdate();
  assertJoystickEvent(0, SDL_JOYBUTTONDOWN, 1, SDL_PRESSED);
  assert(SDL_JoystickGetButton(pad1, 1) == 1);
  // Enable automatic updates.
  SDL_JoystickEventState(SDL_ENABLE);
  assert(SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE);
  emscripten_run_script("window.simulateGamepadButtonUp(0, 1)");
  assertJoystickEvent(0, SDL_JOYBUTTONUP, 1, SDL_RELEASED);
  assert(SDL_JoystickGetButton(pad1, 1) == 0);
  // No button change: Should not result in a new event.
  emscripten_run_script("window.simulateGamepadButtonUp(0, 1)");
  assertNoJoystickEvent();
  // Joystick 1 is not opened; should not result in a new event.
  emscripten_run_script("window.simulateGamepadButtonDown(1, 1)");
  assertNoJoystickEvent();

  // Joystick wiggling
  emscripten_run_script("window.simulateAxisMotion(0, 0, 1)");
  assertJoystickEvent(0, SDL_JOYAXISMOTION, 0, 32767);
  assert(SDL_JoystickGetAxis(pad1, 0) == 32767);
  emscripten_run_script("window.simulateAxisMotion(0, 0, 0)");
  assertJoystickEvent(0, SDL_JOYAXISMOTION, 0, 0);
  assert(SDL_JoystickGetAxis(pad1, 0) == 0);
  emscripten_run_script("window.simulateAxisMotion(0, 1, -1)");
  assertJoystickEvent(0, SDL_JOYAXISMOTION, 1, -32768);
  assert(SDL_JoystickGetAxis(pad1, 1) == -32768);
  emscripten_run_script("window.simulateAxisMotion(0, 1, -1)");
  // No joystick change: Should not result in a new event.
  assertNoJoystickEvent();
  // Joystick 1 is not opened; should not result in a new event.
  emscripten_run_script("window.simulateAxisMotion(1, 1, -1)");
  assertNoJoystickEvent();

  SDL_JoystickClose(pad1);
  assert(!SDL_JoystickOpened(0));

  // Joystick 0 is closed; we should not process any new gamepad events from it.
  emscripten_run_script("window.simulateGamepadButtonDown(0, 1)");
  assertNoJoystickEvent();

  // End test.
  printf("Test passed!\n");
  exit(0);
}

