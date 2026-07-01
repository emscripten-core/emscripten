/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <emscripten/html5.h>

/**
 * History:
 * - HiDPI awareness is a feature that was added in 3.1.51 / December 2023
 * - It allows to render the canvas for modern 4k/retina screens in hi resolution
 * - It behaves similarly to the way it is handled on the native desktop platform:
 *   - `glfwGetWindowSize` returns the size of the GLFW window (as set via `glfwCreateWindow` or `glfwSetWindowSize`)
 *   - `glfwGetFramebufferSize` returns the actual size of the framebuffer and is usually used for the viewport (ex: `glViewport(0, 0, fbWidth, fbHeight)`)
 *
 * The feature is enabled via a window hint: `glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE)`
 *
 * From an implementation point of view, the actual size of the canvas (canvas.width and canvas.height) is defined to be the framebuffer size.
 * CSS is used to scale the canvas to the GLFW window size. The factor used for HiDPI scaling is `devicePixelRatio`.
 *
 * For example, assuming a window size of 640x480 and a devicePixelRatio of 2, the canvas element handled by the library would look like this:
 * <canvas width="1280" height="960" style="width: 640; height: 480;">
 *
 * Important note: when HiDPI awareness is enabled, the library takes over the size of the canvas, in particular its CSS width and height.
 * As a result, CSS Scaling (see `test_glfw3_css_scaling.c` for details) is disabled.
 * If you need the canvas to be resized dynamically (for example when the browser window is resized),
 * you can use a different HTML element with CSS sizing and set a callback to update the size of the canvas appropriately.
 *
 * Example:
 *
 * GLFWWindow *window = ...;
 * emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, window, false, OnResize);
 *
 * // assuming HTML like this <div id="canvas-container" style="width: 100%"><canvas id="canvas"></div>
 * static EM_BOOL OnResize(int event_type, const EmscriptenUiEvent* event, void* user_data) {
 *   GLFWWindow *window = (GLFWWindow*) user_data;
 *   double w, h;
 *   // use ANOTHER HTML element
 *   emscripten_get_element_css_size("#canvas-container", &w, &h);
 *   // set the size of the GLFW window accordingly
 *   glfwSetWindowSize(window, (int)w, (int)h);
 *   return true;
 * }
 */

// installing mock devicePixelRatio (independent of the browser/screen resolution)
static void installMockDevicePixelRatio() {
  printf("installing mock devicePixelRatio...\n");
  EM_ASM(
      GLFW.mockDevicePixelRatio = 1.0;
      GLFW.getDevicePixelRatio = () => { console.log("mock getDevicePixelRatio"); return GLFW.mockDevicePixelRatio; };
      );
}

static void setDevicePixelRatio(float ratio) {
  printf("setDevicePixelRatio %.0f\n", ratio);
  EM_ASM({
    GLFW.mockDevicePixelRatio = $0;
    // mocking/simulating the fact that an event should be raised when devicePixelRatio changes
    if (GLFW.devicePixelRatioMQL) {
      GLFW.onDevicePixelRatioChange();
    }
  }, ratio);
}

static void setGLFWIsHiDPIAware(GLFWwindow *window, bool isHiDPIAware) {
  printf("setGLFWIsHiDPIAware %s\n", isHiDPIAware ? "true" : "false");
  glfwSetWindowAttrib(window, GLFW_SCALE_TO_MONITOR, isHiDPIAware ? GLFW_TRUE : GLFW_FALSE);
}

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

static bool getGLFWIsHiDPIAware() {
  return EM_ASM_INT(return GLFW.isHiDPIAware() ? 1 : 0) != 0;
}

static void checkHiDPIAware(GLFWwindow *window, bool expectedAwareness) {
  assert(getGLFWIsHiDPIAware() == expectedAwareness);
  assert(glfwGetWindowAttrib(window, GLFW_SCALE_TO_MONITOR) == (expectedAwareness ? GLFW_TRUE : GLFW_FALSE));
}

int main() {

  GLFWwindow* window;

  assert(glfwInit() == GLFW_TRUE);

  installMockDevicePixelRatio();

  // by default, GLFW is NOT Hi DPI aware
  assert(!getGLFWIsHiDPIAware());

  // Use case 1: GLFW is NOT Hi DPI Aware | devicePixelRatio is 1.0
  // Expected outcome is window size and frame buffer size are the same
  {
    printf("Use case #1\n");
    window = glfwCreateWindow(640, 480, "test_glfw3_hi_dpi_aware.c | #1", NULL, NULL);
    assert(window != NULL);
    checkHiDPIAware(window, false);
    checkWindowSize(window, 640, 480, 1.0);
    glfwSetWindowSize(window, 600, 400);
    checkWindowSize(window, 600, 400, 1.0);
    glfwDestroyWindow(window);
  }

  // Use case 2: GLFW is NOT Hi DPI Aware | devicePixelRatio is 2.0
  // Expected outcome is window size and frame buffer size are the same (because
  // GLFW is not Hi DPI Aware)
  {
    printf("Use case #2\n");
    setDevicePixelRatio(2.0);
    window = glfwCreateWindow(640, 480, "test_glfw3_hi_dpi_aware.c | #2", NULL, NULL);
    assert(window != NULL);
    checkHiDPIAware(window, false);
    checkWindowSize(window, 640, 480, 1.0);
    glfwSetWindowSize(window, 600, 400);
    checkWindowSize(window, 600, 400, 1.0);
    glfwDestroyWindow(window);
  }

  // Use case 3: GLFW is Hi DPI Aware | devicePixelRatio is 1.0
  // Expected outcome is window size and frame buffer size are the same
  {
    printf("Use case #3\n");
    setDevicePixelRatio(1.0);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    window = glfwCreateWindow(640, 480, "test_glfw3_hi_dpi_aware.c | #3", NULL, NULL);
    assert(window != NULL);
    checkHiDPIAware(window, true);
    checkWindowSize(window, 640, 480, 1.0);
    glfwSetWindowSize(window, 600, 400);
    checkWindowSize(window, 600, 400, 1.0);
    glfwDestroyWindow(window);
  }

  // Use case 4: GLFW is Hi DPI Aware | devicePixelRatio is 2.0
  // Expected outcome is frame buffer size is 2x window size
  {
    printf("Use case #4\n");
    setDevicePixelRatio(2.0);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    window = glfwCreateWindow(640, 480, "test_glfw3_hi_dpi_aware.c | #4", NULL, NULL);
    assert(window != NULL);
    checkHiDPIAware(window, true);
    checkWindowSize(window, 640, 480, 2.0);
    glfwSetWindowSize(window, 600, 400);
    checkWindowSize(window, 600, 400, 2.0);
    glfwDestroyWindow(window);
  }

  // Use case 5: GLFW Hi DPI Awareness changes | devicePixelRatio 2.0
  // Expected outcome is that the window sizes is adjusted automatically
  {
    printf("Use case #5\n");
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    window = glfwCreateWindow(640, 480, "test_glfw3_hi_dpi_aware.c | #5", NULL, NULL);
    assert(window != NULL);
    checkHiDPIAware(window, true);
    checkWindowSize(window, 640, 480, 2.0);
    setGLFWIsHiDPIAware(window, false);
    checkWindowSize(window, 640, 480, 1.0);
    checkHiDPIAware(window, false);
    glfwDestroyWindow(window);
  }

  // Use case 6: GLFW is NOT Hi DPI Aware | devicePixelRatio changes
  // Expected outcome is that the window sizes does not change
  {
    printf("Use case #6\n");
    setDevicePixelRatio(1.0);
    glfwDefaultWindowHints(); // reset GLFW_SCALE_TO_MONITOR
    window = glfwCreateWindow(640, 480, "test_glfw3_hi_dpi_aware.c | #6", NULL, NULL);
    assert(window != NULL);
    checkHiDPIAware(window, false);
    checkWindowSize(window, 640, 480, 1.0);
    setDevicePixelRatio(2.0);
    checkWindowSize(window, 640, 480, 1.0);
    glfwDestroyWindow(window);
  }

  // Use case 7: GLFW is Hi DPI Aware | devicePixelRatio changes
  // Expected outcome is that the window sizes is adjusted automatically
  {
    printf("Use case #7\n");
    setDevicePixelRatio(2.0);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    window = glfwCreateWindow(640, 480, "test_glfw3_hi_dpi_aware.c | #7", NULL, NULL);
    assert(window != NULL);
    checkHiDPIAware(window, true);
    checkWindowSize(window, 640, 480, 2.0);
    setDevicePixelRatio(1.0);
    checkWindowSize(window, 640, 480, 1.0);
    glfwDestroyWindow(window);
  }

  glfwTerminate();

  return 0;
}
