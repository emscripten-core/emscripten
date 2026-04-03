/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#if USE_GLFW == 2
    #include <GL/glfw.h>
#else
    #include <GLFW/glfw3.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <emscripten.h>

#define MULTILINE(...) #__VA_ARGS__
#define WIDTH 640
#define HEIGHT 480

// Setup tests
typedef struct {
    int mouse;
    double x, y;
    int button;
    int action;
    int modify;
    int character;
} test_args_t;

typedef struct {
    char cmd[80];
    test_args_t args;
} test_t;

// Javascript event.button 0 = left, 1 = middle, 2 = right
test_t g_tests[] = {
    { "simulateMouseDown(10.0, 10.0, 0)", { 1, 10.0, 10.0, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, -1 } },
    { "simulateMouseUp  (10.0, 20.0, 0)", { 1, 10.0, 20.0, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, -1 } },
    { "simulateMouseDown(10.0, 30.0, 1)", { 1, 10.0, 30.0, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS, -1 } },
    { "simulateMouseUp  (10.0, 40.0, 1)", { 1, 10.0, 40.0, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_RELEASE, -1 } },
    { "simulateMouseDown(10.0, 30.0, 2)", { 1, 10.0, 30.0, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, -1 } },
    { "simulateMouseUp  (10.0, 40.0, 2)", { 1, 10.0, 40.0, GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, -1 } },
    //{ "Module.injectMouseEvent(10.0, 50.0, 'mousewheel', 0)", { 10.0, 50.0, -1, -1, -1 } },
    //{ "Module.injectMouseEvent(10.0, 60.0, 'mousemove', 0)", { 10.0, 60.0, -1, -1, -1 } }

    { "simulateKeyDown(8)", { 0, 0.0, 0.0, GLFW_KEY_BACKSPACE, GLFW_PRESS, -1 } },
    { "simulateKeyUp  (8)", { 0, 0.0, 0.0, GLFW_KEY_BACKSPACE, GLFW_RELEASE, -1 } },
    { "simulateKeyDown(9)", { 0, 0.0, 0.0, GLFW_KEY_TAB, GLFW_PRESS, -1 } },
    { "simulateKeyUp  (9)", { 0, 0.0, 0.0, GLFW_KEY_TAB, GLFW_RELEASE, -1 } },
    { "simulateKeyDown(112)", { 0, 0.0, 0.0, GLFW_KEY_F1, GLFW_PRESS, -1 } },
    { "simulateKeyUp  (112)", { 0, 0.0, 0.0, GLFW_KEY_F1, GLFW_RELEASE, -1 } },
    { "simulateKeyDown(37)", { 0, 0.0, 0.0, GLFW_KEY_LEFT, GLFW_PRESS, -1 } },
    { "simulateKeyUp  (37)", { 0, 0.0, 0.0, GLFW_KEY_LEFT, GLFW_RELEASE, -1 } },
    { "simulateKeyDown(39)", { 0, 0.0, 0.0, GLFW_KEY_RIGHT, GLFW_PRESS, -1 } },
    { "simulateKeyUp  (39)", { 0, 0.0, 0.0, GLFW_KEY_RIGHT, GLFW_RELEASE, -1 } },
    { "simulateKeyDown(38)", { 0, 0.0, 0.0, GLFW_KEY_UP, GLFW_PRESS, -1 } },
    { "simulateKeyUp  (38)", { 0, 0.0, 0.0, GLFW_KEY_UP, GLFW_RELEASE, -1 } },
    { "simulateKeyDown(40)", { 0, 0.0, 0.0, GLFW_KEY_DOWN, GLFW_PRESS, -1 } },
    { "simulateKeyUp  (40)", { 0, 0.0, 0.0, GLFW_KEY_DOWN, GLFW_RELEASE, -1 } },
    #if USE_GLFW == 2
        { "simulateKeyDown(27)", { 0, 0.0, 0.0, GLFW_KEY_ESC, GLFW_PRESS, -1 } },
        { "simulateKeyUp(27)", { 0, 0.0, 0.0, GLFW_KEY_ESC, GLFW_RELEASE, -1 } },

        { "simulateKeyDown(65)", { 0, 0.0, 0.0, 'A', GLFW_PRESS, -1, 'A' } },
        { "simulateKeyEvent('keypress', 65, {charCode: 65})", { 0, 0.0, 0.0, -1, -1, -1, 'A' } },
        { "simulateKeyUp(65)", { 0, 0.0, 0.0, 'A', GLFW_RELEASE, -1, 'A' } },

        { "simulateKeyDown(65, {ctrlKey: true})", { 0, 0.0, 0.0, 'A', GLFW_PRESS, -1, 'A' } },
        { "simulateKeyEvent('keypress', 65, {ctrlKey: true, charCode: 65})", { 0, 0.0, 0.0, -1, -1, -1, -1 } },
        { "simulateKeyUp(65, {ctrlKey: true})", { 0, 0.0, 0.0, 'A', GLFW_RELEASE, -1, 'A' } },
    #else
        { "simulateKeyDown(27)", { 0, 0.0, 0.0, GLFW_KEY_ESCAPE, GLFW_PRESS, -1 } },
        { "simulateKeyUp(27)", { 0, 0.0, 0.0, GLFW_KEY_ESCAPE, GLFW_RELEASE, -1 } },

        { "simulateKeyDown(65)", { 0, 0.0, 0.0, GLFW_KEY_A, GLFW_PRESS, -1 } },
        { "simulateKeyEvent('keypress', 65, {charCode: 65})", { 0, 0.0, 0.0, -1, -1, -1, 'A' } },
        { "simulateKeyUp(65)", { 0, 0.0, 0.0, GLFW_KEY_A, GLFW_RELEASE, -1 } },

        { "simulateKeyDown(65, {ctrlKey: true})", { 0, 0.0, 0.0, GLFW_KEY_A, GLFW_PRESS, -1, 'A' } },
        { "simulateKeyEvent('keypress', 65, {ctrlKey: true, charCode: 65})", { 0, 0.0, 0.0, -1, -1, -1, -1 } },
        { "simulateKeyUp(65, {ctrlKey: true})", { 0, 0.0, 0.0, GLFW_KEY_A, GLFW_RELEASE, -1, 'A' } },
    #endif
};

static bool g_callback_received;
static unsigned int g_current_test;
static unsigned int g_test_count = sizeof(g_tests) / sizeof(test_t);

#if USE_GLFW == 2
static void on_mouse_button_callback(int button, int action) {
#else
static void on_mouse_button_callback(GLFWwindow* window, int button, int action, int modify) {
#endif
  test_args_t args = g_tests[g_current_test].args;
  if (args.button == button && args.action == action) {
    g_callback_received = true;
  } else {
    printf("Test %d: FAIL\n", g_current_test);
    exit(1);
  }
}

#if USE_GLFW == 2
static void on_mouse_move(int x, int y) {
#else
static void on_mouse_move(GLFWwindow* window, double x, double y) {
#endif
  test_args_t args = g_tests[g_current_test].args;
  if (args.x == x && args.y == y) {
    g_callback_received = true;
  } else {
    printf("Test %d: FAIL\n", g_current_test);
    exit(1);
  }
}

#if USE_GLFW == 2
static void on_key_callback(int key, int action) {
#else
static void on_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
#endif
  test_args_t args = g_tests[g_current_test].args;
  if (args.button == key && args.action == action) {
    g_callback_received = true;
  } else {
    printf("Test %d: FAIL\n", g_current_test);
    exit(1);
  }
}

#if USE_GLFW == 2
static void on_char_callback(int character, int action) {
#else
static void on_char_callback(GLFWwindow* window, unsigned int character) {
#endif
  printf("got character: %d\n", character);
  test_args_t args = g_tests[g_current_test].args;
  if (args.character != -1 && args.character == character) {
    g_callback_received = true;
  } else {
    printf("Test %d: FAIL\n", g_current_test);
    exit(1);
  }

}

#if USE_GLFW == 3
static void on_mouse_wheel(GLFWwindow* window, double x, double y) {
  test_args_t args = g_tests[g_current_test].args;
  if (args.x == x && args.y == y) {
    g_callback_received = true;
  } else {
    printf("Test %d: FAIL\n", g_current_test);
    exit(1);
  }
}

static void on_error(int error, const char *msg) {
  printf("%d: %s\n", error, msg);
}
#endif

int main() {
  unsigned int success = (1 << (sizeof(g_tests) / sizeof(test_t))) - 1; // (2^count)-1;

  glfwInit();

#if USE_GLFW == 2
  glfwOpenWindow(WIDTH, HEIGHT, 5, 6, 5, 0, 0, 0, GLFW_WINDOW); // != GL_TRUE)

  glfwSetMousePosCallback(on_mouse_move);
  glfwSetCharCallback(on_char_callback);
#else
  glfwSetErrorCallback(on_error);
  printf("%s\n", glfwGetVersionString());

  GLFWwindow * _mainWindow = NULL;
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  _mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "glfw3_events", NULL, NULL);
  glfwMakeContextCurrent(_mainWindow);

  glfwSetCursorPosCallback(_mainWindow, on_mouse_move);
  glfwSetScrollCallback(_mainWindow, on_mouse_wheel);
  glfwSetCharCallback(_mainWindow, on_char_callback);
#endif

  for (int p = 0; p < 2; ++p) { // 2 passes, with and without callbacks.
    printf("Running Test pass %d\n", p);

#if USE_GLFW == 2
    glfwSetMouseButtonCallback(p == 0 ? NULL : on_mouse_button_callback);
    glfwSetKeyCallback(p == 0 ? NULL : on_key_callback);
#else
    glfwSetMouseButtonCallback(_mainWindow, p == 0 ? NULL : on_mouse_button_callback);
    glfwSetKeyCallback(_mainWindow, p == 0 ? NULL : on_key_callback);
#endif

    for (int i = 0; i < g_test_count; ++i) {
      g_current_test = i;
      test_t test = g_tests[g_current_test];
      g_callback_received = false;

      emscripten_run_script(test.cmd);

      if (test.args.mouse) {
      #if USE_GLFW == 2
        if (glfwGetMouseButton(test.args.button) != test.args.action) {
      #else
        if (glfwGetMouseButton(_mainWindow, test.args.button) != test.args.action) {
      #endif
          printf("Test %d: FAIL\n", g_current_test);
          exit(1);
        }
      } else {
        // Keyboard.
      #if USE_GLFW == 2
        if (test.args.action != -1 && glfwGetKey(test.args.button) != test.args.action) {
      #else
        if (test.args.action != -1 && glfwGetKey(_mainWindow, test.args.button) != test.args.action) {
      #endif
          printf("Test %d: FAIL\n", g_current_test);
          exit(1);
        }
      }

      if (p == 1) {
        if (test.args.character != -1 && !g_callback_received) {
          printf("Test %d: FAIL (event not received)\n", g_current_test);
          exit(1);
        }
        printf("Test %d: PASSED\n", g_current_test);
      }
    }
  }

  glfwTerminate();
  printf("done\n");
  return 0;
}
