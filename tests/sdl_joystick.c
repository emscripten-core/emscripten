#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <assert.h>
#include <emscripten.h>

int result = 1;

void assertJoystickEvent(int expectedGamepad, int expectedType, int expectedIndex, int expectedValue) {
  SDL_Event event;
  assert(SDL_PollEvent(&event));
  assert(event.type == expectedType);
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
}

void main_2(void* arg);

int main() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  SDL_Surface *screen = SDL_SetVideoMode(600, 450, 32, SDL_HWSURFACE);
  emscripten_async_call(main_2, NULL, 3000); // avoid startup delays and intermittent errors
  return 0;
}

void main_2(void* arg) {
  SDL_Event event;

  // TODO: At the moment, we only support joystick support through polling.
  emscripten_run_script("window.addNewGamepad('Pad Thai', 4, 16)");
  emscripten_run_script("window.addNewGamepad('Pad Kee Mao', 0, 4)");
  // Check that the joysticks exist properly.
  assert(SDL_NumJoysticks() == 2);
  assert(!SDL_JoystickOpened(0));
  assert(!SDL_JoystickOpened(1));
  SDL_Joystick* pad1 = SDL_JoystickOpen(0);
  assert(SDL_JoystickOpened(0));
  assert(strncmp(SDL_JoystickName(0), "Pad Thai", 9) == 0);
  assert(strncmp(SDL_JoystickName(1), "Pad Kee Mao", 12) == 0);

  // Button events.
  emscripten_run_script("window.simulateGamepadButtonDown(0, 1)");
  assertJoystickEvent(0, SDL_JOYBUTTONDOWN, 1, SDL_PRESSED);
  emscripten_run_script("window.simulateGamepadButtonUp(0, 1)");
  assertJoystickEvent(0, SDL_JOYBUTTONDOWN, 1, SDL_RELEASED);
  // No button change: Should not result in a new event.
  emscripten_run_script("window.simulateGamepadButtonUp(0, 1)");
  assert(!SDL_PollEvent(&event));
  // Joystick 1 is not opened; should not result in a new event.
  emscripten_run_script("window.simulateGamepadButtonDown(1, 1)");
  assert(!SDL_PollEvent(&event));

  // Joystick wiggling
  emscripten_run_script("window.simulateAxisMotion(0, 1, 1)")
  assertJoystickEvent(0, SDL_JOYAXISMOTION, 1, 32767);
  emscripten_run_script("window.simulateAxisMotion(0, 0, 0)")
  assertJoystickEvent(0, SDL_JOYAXISMOTION, 0, 0);
  emscripten_run_script("window.simulateAxisMotion(0, 1, -1)")
  assertJoystickEvent(0, SDL_JOYAXISMOTION, 1, -32768);
  emscripten_run_script("window.simulateAxisMotion(0, 1, -1)")
  // No joystick change: Should not result in a new event.
  assert(!SDL_PollEvent(&event));
  // Joystick 1 is not opened; should not result in a new event.
  assertJoystickEvent(1, SDL_JOYAXISMOTION, 1, -32768);
  assert(!SDL_PollEvent(&event));

  SDL_JoystickClose(0);
  assert(!SDL_JoystickOpened(0));

  // Joystick 0 is closed; we should not process any new gamepad events from it.
  emscripten_run_script("window.simulateGamepadButtonDown(0, 1)");
  assert(!SDL_PollEvent(&event));
}

