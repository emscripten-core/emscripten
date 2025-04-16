/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <GLFW/glfw3.h>
#include <assert.h>
#include <emscripten/html5.h>
#include <stdio.h>

/*
 * History:
 * - This test was added after the issue #22847 was reported (November 2024).
 * - For historical reasons, the JavaScript GLFW implementation includes a feature: the ability to scale the canvas via CSS.
 * - Note that this feature is not part of GLFW as GLFW does not offer any API to scale the window.
 * - Being undocumented/untested, this feature was accidentally removed when HiDPI support was added (in 3.1.51 / December 2023).
 * - This test was added to document this feature and ensure proper behavior.
 *
 * What does this feature do?
 * - if there is a CSS rule that specifies the size of the canvas (ex: `#canvas { width: 100%;}`), then the canvas
 *   will be scaled to match this value. Note that from a GLFW point of view, the size of the window remains what
 *   gets specified in `glfwCreateWindow` and/or `glfwSetWindowSize`
 * - only global CSS rules apply, as setting a CSS rule on the canvas itself is removed (ex: `<canvas style="width:100%;">` is ignored)
 *
 * In HiDPI mode, (`glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE)`), this feature cannot work and as a result is disabled (because the canvas
 * is sized using `devicePixelRatio` as the factor and CSS is used to scale it at the desired size).
 */

/**
 * As explained above, this function adds a global CSS rule instead of setting the CSS style directly on the canvas
 * because it gets ignored otherwise. */
EM_JS(void, addCSSScalingRule, (), {
  const style = document.createElement('style');
  style.appendChild(document.createTextNode("#canvas { width: 700px; height: 500px; }"));
  document.head.appendChild(style);
})

/**
 * Since it is unclear in which browser/resolution this test will run we compute the actual ratio used by the test.
 * This has the neat effect that the test can be run manually on a HiDPI screen.
 */
EM_JS(double, getDevicePixelRatio, (), {return (typeof devicePixelRatio == 'number' && devicePixelRatio) || 1.0;})

/**
 * Checks window size and framebuffer size according to ratio */
static void checkWindowSize(GLFWwindow *window, int expectedWidth, int expectedHeight, float ratio) {
  // first check the window size
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  printf("windowSize => %d == %d && %d == %d\n", w, expectedWidth, h, expectedHeight);
  assert(w == expectedWidth && h == expectedHeight);

  // second check the frame buffer size
  int fbw, fbh;
  glfwGetFramebufferSize(window, &fbw, &fbh);
  printf("framebufferSize => %d == %d && %d == %d\n", fbw, (int) (expectedWidth * ratio), fbh, (int) (expectedHeight * ratio));
  assert(fbw == (int) (expectedWidth * ratio) && fbh == (int) (expectedHeight * ratio));
}

// Create a window without HiDPI support and without CSS rule => expected sizes to match
void use_case_1() {
  printf("Use case #1\n");
  GLFWwindow* window = glfwCreateWindow(640, 480, "test_glfw3_css_scaling.c", NULL, NULL);
  assert(window);
  checkWindowSize(window, 640, 480, 1.0);
  double w, h;
  emscripten_get_element_css_size("#canvas", &w, &h);
  printf("CSS Size=%.0fx%.0f\n", w, h);
  assert(w == 640 && h == 480);
  glfwDestroyWindow(window);
}

// Create a window without HiDPI support, and with CSS rule =>
// the window size should match the creation size, but the CSS size should match the rule.
void use_case_2() {
  printf("Use case #2\n");
  GLFWwindow* window = glfwCreateWindow(640, 480, "test_glfw3_css_scaling.c", NULL, NULL);
  assert(window);
  checkWindowSize(window, 640, 480, 1.0);
  double w, h;
  emscripten_get_element_css_size("#canvas", &w, &h);
  printf("CSS Size=%.0fx%.0f\n", w, h);
  assert(w == 700 && h == 500); // Rule is "#canvas { width: 700px; height: 500px; }"
  glfwDestroyWindow(window);
}

// Create a window with HiDPI support, and with CSS rule =>
// the window size and framebuffer size should match the creation size (CSS rule is ignored)
void use_case_3() {
  printf("Use case #3\n");
  glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
  GLFWwindow* window = glfwCreateWindow(640, 480, "test_glfw3_css_scaling.c", NULL, NULL);
  assert(window);
  double dpr = getDevicePixelRatio();
  printf("devicePixelRatio=%.0f\n", dpr);
  checkWindowSize(window, 640, 480, dpr);
  double w, h;
  emscripten_get_element_css_size("#canvas", &w, &h);
  printf("CSS Size=%.0fx%.0f\n", w, h);
  assert(w == 640 && h == 480);
  glfwDestroyWindow(window);
}

int main() {
  assert(glfwInit() == GLFW_TRUE);

  use_case_1();

  // Add CSS rule for the following use cases
  addCSSScalingRule();

  use_case_2();
  use_case_3();

  printf("All tests complete\n");
  glfwTerminate();
  return 0;
}
