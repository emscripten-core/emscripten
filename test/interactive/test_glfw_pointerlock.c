/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten.h>
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

GLFWwindow* g_window;

void render();
void error_callback(int error, const char* description) {
  fprintf(stderr, "Error %d: %s\n", error, description);
  exit(1);
}

void render() {
  glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

int actions = 0;

void OnMouseClick(GLFWwindow *window, int button, int action, int mods) {
  actions++;
  printf("mouse actions: %d / 4\n", actions);
  if (actions >= 4) {
    printf("done.\n");
    emscripten_force_exit(0);
  }
}

int main() {
  // Setup g_window
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
  {
    printf("Could not create window. Test failed.\n");      

    return 1;
  }
  glfwWindowHint(GLFW_RESIZABLE , 1);
  g_window = glfwCreateWindow(600, 450, "GLFW pointerlock test", NULL, NULL);
  if (!g_window)
  {
    printf("Could not create window. Test failed.\n");      
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(g_window);

  // Try to disable cursor by entering pointer lock. If pointer lock failed (because glfwSetInputMode wasn't called in a user event),
  // then clicking the canvas should also try to lock the pointer, as long as the cursor mode is GLFW_CURSOR_DISABLED.
  glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // When we see enough clicks, we close the window
  glfwSetMouseButtonCallback(g_window, OnMouseClick);

  // Main loop
  printf("Click the canvas to enter pointer lock mode. The browser should offer to hide the mouse pointer. Make sure to accept it.\n");
  printf("If you exit pointer lock (such as pressing ESC on keyboard), clicking the canvas should lock the pointer again.\n");
  emscripten_set_main_loop(render, 0, 1);
  __builtin_trap();

  glfwTerminate();

  return 0;
}
