/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

GLFWwindow* g_window;

void error_callback(int error, const char* description);

int joy_connected = -1;

void joystick_callback(int joy, int event)
{
  if (event == GLFW_CONNECTED) {
    printf("Joystick %d was connected: %s\n", joy, glfwGetJoystickName(joy));
    joy_connected = joy; // use the most recently connected joystick
  } else if (event == GLFW_DISCONNECTED) {
    printf("Joystick %d was disconnected\n", joy);
    if (joy == joy_connected) joy_connected = -1;
  }
}

void main_2(void *arg) {
  printf("Testing adding new gamepads\n");
  emscripten_run_script("window.addNewGamepad('Pad Thai', 4, 16)");
  emscripten_run_script("window.addNewGamepad('Pad Kee Mao', 0, 4)");
  // Check that the joysticks exist properly.
  assert(glfwJoystickPresent(GLFW_JOYSTICK_1));
  assert(glfwJoystickPresent(GLFW_JOYSTICK_2));

  assert(strcmp(glfwGetJoystickName(GLFW_JOYSTICK_1), "Pad Thai") == 0);
  assert(strcmp(glfwGetJoystickName(GLFW_JOYSTICK_2), "Pad Kee Mao") == 0);

  int axes_count = 0;
  int buttons_count = 0;

  glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
  glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
  assert(axes_count == 4);
  assert(buttons_count == 16);

  glfwGetJoystickAxes(GLFW_JOYSTICK_2, &axes_count);
  glfwGetJoystickButtons(GLFW_JOYSTICK_2, &buttons_count);
  assert(axes_count == 0);
  assert(buttons_count == 4);

  // Buttons
  printf("Testing buttons\n");
  const unsigned char *buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
  assert(buttons_count == 16);
  assert(buttons[0] == 0);
  emscripten_run_script("window.simulateGamepadButtonDown(0, 1)");
  buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
  assert(buttons_count == 16);
  assert(buttons[1] == 1);

  emscripten_run_script("window.simulateGamepadButtonUp(0, 1)");
  buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
  assert(buttons_count == 16);
  assert(buttons[1] == 0);


  emscripten_run_script("window.simulateGamepadButtonDown(1, 0)");
  buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_2, &buttons_count);
  assert(buttons_count == 4);
  assert(buttons[0] == 1);

  emscripten_run_script("window.simulateGamepadButtonUp(1, 0)");
  buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_2, &buttons_count);
  assert(buttons_count == 4);
  assert(buttons[1] == 0);

  // Joystick wiggling
  printf("Testing joystick axes\n");
  emscripten_run_script("window.simulateAxisMotion(0, 0, 1)");
  const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
  assert(axes_count == 4);
  assert(axes[0] == 1);

  emscripten_run_script("window.simulateAxisMotion(0, 0, 0)");
  axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
  assert(axes_count == 4);
  assert(axes[0] == 0);

  emscripten_run_script("window.simulateAxisMotion(0, 1, -1)");
  axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
  assert(axes_count == 4);
  assert(axes[1] == -1);

  // End test.
  printf("Test passed!\n");
  emscripten_force_exit(0);
}

int main() {
  if (!glfwInit())
  {
    printf("Could not create window. Test failed.\n");
    return 1;
  }
  glfwWindowHint(GLFW_RESIZABLE , 1);
  g_window = glfwCreateWindow(600, 450, "GLFW joystick test", NULL, NULL);
  if (!g_window)
  {
    printf("Could not create window. Test failed.\n");
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(g_window);
  glfwSetJoystickCallback(joystick_callback);

  emscripten_async_call(main_2, NULL, 3000); // avoid startup delays and intermittent errors

#ifndef __EMSCRIPTEN__
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
#endif

  glfwTerminate();
  return 99;
}
