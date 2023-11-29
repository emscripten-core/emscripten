/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <GLFW/glfw3.h>
#include <assert.h>
#include <emscripten/html5.h>

static void checkDefaultWindowHints() {
  int ok = EM_ASM_INT({
    var res = 1;
    for (var k in TEST_GLFW3_DEFAULTS_HINTS) {
      if (GLFW.defaultHints[k] !== TEST_GLFW3_DEFAULTS_HINTS[k])
        res = 0;
    }
    return res;
  });
  assert(ok == 1);
}

int main() {

  EM_ASM(
    TEST_GLFW3_DEFAULTS_HINTS = {};
    TEST_GLFW3_DEFAULTS_HINTS[0x00020001] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x00020002] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x00020003] = 1;
    TEST_GLFW3_DEFAULTS_HINTS[0x00020004] = 1;
    TEST_GLFW3_DEFAULTS_HINTS[0x00020005] = 1;
    TEST_GLFW3_DEFAULTS_HINTS[0x0002000A] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x0002200C] = 0;

    TEST_GLFW3_DEFAULTS_HINTS[0x00021001] = 8;
    TEST_GLFW3_DEFAULTS_HINTS[0x00021002] = 8;
    TEST_GLFW3_DEFAULTS_HINTS[0x00021003] = 8;
    TEST_GLFW3_DEFAULTS_HINTS[0x00021004] = 8;
    TEST_GLFW3_DEFAULTS_HINTS[0x00021005] = 24;
    TEST_GLFW3_DEFAULTS_HINTS[0x00021006] = 8;
    TEST_GLFW3_DEFAULTS_HINTS[0x00021007] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x00021008] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x00021009] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x0002100A] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x0002100B] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x0002100C] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x0002100D] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x0002100E] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x0002100F] = 0;

    TEST_GLFW3_DEFAULTS_HINTS[0x00022001] = 0x00030001;
    TEST_GLFW3_DEFAULTS_HINTS[0x00022002] = 1;
    TEST_GLFW3_DEFAULTS_HINTS[0x00022003] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x00022004] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x00022005] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x00022006] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x00022007] = 0;
    TEST_GLFW3_DEFAULTS_HINTS[0x00022008] = 0;
  );

  assert(glfwInit() == GL_TRUE);

  // Use case: after glfwInit, default window hints are correct
  {
    checkDefaultWindowHints();
  }

  // Use case: updates a window hint
  // Expected results: window hint is properly updated
  //                   default window hints are not affected
  {
    // GLFW_DEPTH_BITS
    assert(EM_ASM_INT(return GLFW.hints[0x00021005];) == 24);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    assert(EM_ASM_INT(return GLFW.hints[0x00021005];) == 16);
    checkDefaultWindowHints();
  }

  // Use case: resets window hints to default
  // Expected results: previously changed window hint is back to its default value
  //                   default window hints are not affected
  {
    glfwDefaultWindowHints();
    assert(EM_ASM_INT(return GLFW.hints[0x00021005];) == 24);
    checkDefaultWindowHints();
  }

  // Use case: change window hint, create window, then change window hint
  // Expected results: the window hint set at creation time (which is now a
  //                   window attribute that can be read with glfwGetWindowAttrib)
  //                   does not change
  {
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    GLFWwindow* window = glfwCreateWindow(640, 480, "test_glfw3_default_hints.c", NULL, NULL);
    assert(glfwGetWindowAttrib(window, GLFW_DEPTH_BITS) == 16);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    assert(glfwGetWindowAttrib(window, GLFW_DEPTH_BITS) == 16);
  }

  glfwTerminate();

  return 0;
}
