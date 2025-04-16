/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <GLFW/glfw3.h>
#include <GLFW/emscripten_glfw3.h>
#include <assert.h>

// cpp otherwise it fails to link
int main() {

  assert(glfwInit() == GLFW_TRUE);

  GLFWwindow* window = glfwCreateWindow(320, 200, "test_glfw3_port", 0, 0);
  assert(window != 0);
  // this call ensures that it uses the right port
  assert(emscripten_glfw_is_window_fullscreen(window) == false);
  glfwTerminate();


  return 0;
}
