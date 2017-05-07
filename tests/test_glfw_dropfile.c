#include <stdio.h>
#include <assert.h>
#include <emscripten.h>
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

int result = 1;

GLFWwindow* g_window;

void render();

void render() {
  glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void on_file_drop(GLFWwindow *window, int count, const char **paths) {
  for (int i = 0; i < count; ++i) {
    printf("dropped file %s\n", paths[i]);

    // TODO: read file
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
  g_window = glfwCreateWindow(600, 450, "GLFW drop file", NULL, NULL);
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

  // Install callbacks
  glfwSetDropCallback(g_window, on_file_drop);

  // Main loop
  printf("Drag and drop a file from your desktop onto the green canvas.\n");
  emscripten_set_main_loop(render, 0, 1);

  glfwTerminate();

  return 0;
}
