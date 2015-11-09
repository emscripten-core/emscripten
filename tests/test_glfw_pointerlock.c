#include <stdio.h>
#include <assert.h>
#include <emscripten.h>
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

int result = 1;

GLFWwindow* g_window;

void render();
void error_callback(int error, const char* description);

void render() {
  glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

int main() {
  // Setup g_window
  glfwSetErrorCallback(error_callback);
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
  g_window = glfwCreateWindow(600, 450, "GLFW pointerlock test", NULL, NULL);
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

  // Disable cursor: pointer lock
  glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Main loop
  printf("Click the canvas to enter pointer lock mode. The browser might offer to allow hidding mouse pointer. Make sure to accept it.\n");
  printf("Escaping the pointer lock mode should work again upon clicking the canvas.\n");
  emscripten_set_main_loop(render, 0, 1);

  glfwTerminate();

  return 0;
}
