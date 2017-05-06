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
int window_width, window_height;
int window_xpos, window_ypos;

void render() {
  glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

int inFullscreen = 0;
int wasFullscreen = 0;
void windowSizeCallback(GLFWwindow* window, int width, int height) {
  int isInFullscreen = !!glfwGetWindowMonitor(window);

  if (!isInFullscreen) {
    // Save windowed mode window size for restoration on fullscreen exit
    window_width = width;
    window_height = height;
  }

  if (isInFullscreen && !wasFullscreen) {
    printf("Successfully transitioned to fullscreen mode!\n");
    wasFullscreen = isInFullscreen;
  }

  if (wasFullscreen && !isInFullscreen) {
    printf("Exited fullscreen. Test succeeded.\n");
    result = 1;
#ifdef REPORT_RESULT
    REPORT_RESULT();
#endif
    wasFullscreen = isInFullscreen;
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#endif
    return;
  }
}

void on_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
  //printf("on_key %d\n", key);

  if (action != GLFW_PRESS) return;
  if (key != GLFW_KEY_F) return;

  if (glfwGetWindowMonitor(window)) {
    printf("exiting fullscreen\n");

    glfwSetWindowMonitor(window, NULL, window_xpos, window_ypos, window_width, window_height, GLFW_DONT_CARE);
  } else {
    printf("entering fullscreen\n");
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
  }
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
  glfwGetWindowSize(g_window, &window_width, &window_height);
  glfwGetWindowPos(g_window, &window_xpos, &window_ypos);
  glfwSetKeyCallback(g_window, on_key);

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
  assert(mode->refreshRate > 0);
  assert(mode->redBits == 8);
  assert(mode->greenBits == 8);
  assert(mode->blueBits == 8);

  int modes_count;
  const GLFWvidmode *modes = glfwGetVideoModes(monitor, &modes_count);
  printf("glfwGetVideoModes count = %d\n", modes_count);
  assert(modes);
#ifdef __EMSCRIPTEN__
  assert(modes_count == 1);
  assert(modes == mode);
#endif

  GLFWmonitor *window_monitor = glfwGetWindowMonitor(g_window);
  printf("glfwGetWindowMonitor = %p\n", window_monitor);
  // http://www.glfw.org/docs/latest/window_guide.html#window_monitor
  // "For windowed mode windows, this function returns NULL. This is
  // how to tell full screen windows from windowed mode windows."
  assert(!window_monitor);

  // Web browsers require a user gesture to enter fullscreen
  // so this call is expected to fail (difference from the native glfw)
  printf("calling glfwSetWindowMonitor() without user action\n");
  glfwSetWindowMonitor(g_window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
  int isInFullscreen = !!glfwGetWindowMonitor(g_window);
#ifdef __EMSCRIPTEN__
  assert(!isInFullscreen);
#else
  assert(isInFullscreen);
  glfwSetWindowMonitor(g_window, NULL, window_xpos, window_ypos, window_width, window_height, GLFW_DONT_CARE);
#endif

  glfwSetWindowSizeCallback(g_window, windowSizeCallback);
  printf("Press the F key to enter fullscreen, then press it again to exit\n");

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(render, 0, 1);
#else
  while (!glfwWindowShouldClose(g_window)) {
    render();
    glfwSwapBuffers(g_window);
    glfwPollEvents();
  }
#endif

  glfwTerminate();

  return 0;
}
