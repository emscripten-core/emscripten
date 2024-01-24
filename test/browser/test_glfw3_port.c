/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <GLFW/glfw3.h>
#include <GLFW/emscripten_glfw3.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

void consoleErrorHandler(int iErrorCode, char const *iErrorMessage) {
  printf("glfwError: %d | %s\n", iErrorCode, iErrorMessage);
}

int main() {

  glfwSetErrorCallback(consoleErrorHandler);

  assert(!strcmp(glfwGetVersionString(), "Emscripten/WebAssembly GLFW 3.3.8"));

  assert(glfwInit() == GLFW_TRUE);

  GLFWwindow* window = glfwCreateWindow(320, 200, "test_glfw3_port", 0, 0);
  assert(window != 0);
  // this call ensures that it uses the right port
  assert(emscripten_glfw_is_window_fullscreen(window) == EM_FALSE);

  glfwTerminate();

  return 0;
}
