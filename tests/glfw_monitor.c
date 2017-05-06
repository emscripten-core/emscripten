#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

int result = 1;

GLFWwindow* g_window;

void render() {
  glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

int main() {
  if (!glfwInit())
  {
    result = 0;
    printf("Could not create window. Test failed.\n");
#ifdef REPORT_RESULT
    REPORT_RESULT();
#endif
    return -1;
  }
  glfwWindowHint(GLFW_RESIZABLE , 1);
  g_window = glfwCreateWindow(600, 450, "GLFW screen test", NULL, NULL);
  if (!g_window)
  {
    result = 0;
    printf("Could not create window. Test failed.\n");
#ifdef REPORT_RESULT
    REPORT_RESULT();
#endif
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(g_window);

  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  assert(monitor);
  printf("glfwGetPrimaryMonitor = %p\n", monitor);

  int monitor_count;
  GLFWmonitor **monitors = glfwGetMonitors(&monitor_count);
  printf("glfwGetMonitors count = %d\n", monitor_count);
  assert(monitor_count == 1);
  assert(monitors[0] == monitor);

  const GLFWvidmode *mode = glfwGetVideoMode(monitor);
  assert(mode);
  printf("video mode: %d x %d @ %d Hz, bits: %d/%d/%d\n",
    mode->width, mode->height, mode->refreshRate,
    mode->redBits, mode->greenBits, mode->blueBits);
  assert(mode->width > 0);
  assert(mode->height > 0);
  assert(mode->refreshRate == 60);
  assert(mode->redBits == 8);
  assert(mode->greenBits == 8);
  assert(mode->blueBits == 8);

#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(render, 0, 1);
#else
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
#endif

  glfwTerminate();

  return 0;
}
