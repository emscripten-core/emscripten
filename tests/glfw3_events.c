#include <GLFW/glfw3.h>
#include <stdio.h>
#include <emscripten.h>

#define MULTILINE(...) #__VA_ARGS__
#define WIDTH 640
#define HEIGHT 480

// Setup tests
typedef struct {
    double x, y;
    int button;
    int action;
    int modify;
} test_args_t;

typedef struct {
    char cmd[80];
    test_args_t args;
} test_t;

// Javascript event.button 0 = left, 1 = middle, 2 = right
test_t g_tests[] = {
    { "Module.injectMouseEvent(10.0, 10.0, 'mousedown', 0)", { 10.0, 10.0, GLFW_MOUSE_BUTTON_1, GLFW_PRESS, -1 } },
    { "Module.injectMouseEvent(10.0, 20.0, 'mouseup', 0)", { 10.0, 20.0, GLFW_MOUSE_BUTTON_1, GLFW_RELEASE, -1 } },
    { "Module.injectMouseEvent(10.0, 30.0, 'mousedown', 2)", { 10.0, 30.0, GLFW_MOUSE_BUTTON_2, GLFW_PRESS, -1 } },
    { "Module.injectMouseEvent(10.0, 40.0, 'mouseup', 2)", { 10.0, 40.0, GLFW_MOUSE_BUTTON_2, GLFW_RELEASE, -1 } },
    //{ "Module.injectMouseEvent(10.0, 50.0, 'mousewheel', 0)", { 10.0, 50.0, -1, -1, -1 } },
    //{ "Module.injectMouseEvent(10.0, 60.0, 'mousemove', 0)", { 10.0, 60.0, -1, -1, -1 } }
    { "keydown(32)", { 0, 0, GLFW_KEY_SPACE, GLFW_PRESS, -1 } },
    { "keyup(32)", { 0, 0, GLFW_KEY_SPACE, GLFW_RELEASE, -1 } },
    { "keydown(32)", { 0, 0, GLFW_KEY_SPACE, GLFW_PRESS, -1 } },
    { "keydown(32)", { 0, 0, GLFW_KEY_SPACE, GLFW_REPEAT, -1 } },
    { "keydown(32)", { 0, 0, GLFW_KEY_SPACE, GLFW_REPEAT, -1 } },
    { "keyup(32)", { 0, 0, GLFW_KEY_SPACE, GLFW_RELEASE, -1 } },
};

static unsigned int g_test_actual = 0;
static unsigned int g_test_count = sizeof(g_tests) / sizeof(test_t);
static unsigned int g_state = 0;

static void on_mouse_callback(GLFWwindow* window, int button, int action, int modify)
{
    test_args_t args = g_tests[g_test_actual].args;
    if (args.button == button && args.action == action)
    {
        printf("Test on_mouse_callback: OK\n");
        g_state |= 1 << g_test_actual;
    }
    else
    {
        printf("Test %d: FAIL\n", g_test_actual);
    }
}

static void on_mouse_move(GLFWwindow* window, double x, double y)
{
    test_args_t args = g_tests[g_test_actual].args;
    if (args.x == x && args.y == y)
    {
        printf("Test on_mouse_move: OK\n");
        g_state |= 1 << g_test_actual;
    }
    else
    {
        printf("Test %d: FAIL\n", g_test_actual);
    }
}

static void on_mouse_wheel(GLFWwindow* window, double x, double y)
{
    test_args_t args = g_tests[g_test_actual].args;
    if (args.x == x && args.y == y)
    {
        printf("Test on_mouse_wheel: OK\n");
        g_state |= 1 << g_test_actual;
    }
    else
    {
        printf("Test %d: FAIL\n", g_test_actual);
    }
}

static void on_key_action(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    test_args_t args = g_tests[g_test_actual].args;
    if (args.button == key && args.action == action)
    {
        printf("Test on_key_action: OK\n");
        g_state |= 1 << g_test_actual;
    }
    else
    {
        printf("Test %d: FAIL\n", g_test_actual);
    }
}

static void on_error(int error, const char *msg)
{
    printf("%d: %s\n", error, msg);
}

int main()
{
    GLFWwindow * _mainWindow = NULL;
    int result = 0;
    unsigned int success = 0;

    emscripten_run_script(MULTILINE(
        Module.injectMouseEvent = function(x, y, event_, button) {
            var event = document.createEvent("MouseEvents");
            var canvas = Module['canvas'];
            event.initMouseEvent(event_, true, true, window, 0, canvas.offsetLeft + x, canvas.offsetTop + y, canvas.offsetLeft + x, canvas.offsetTop + y, 0, 0, 0, 0, button, null);
            canvas.dispatchEvent(event);
        }
    ));

    glfwSetErrorCallback(on_error);
    glfwInit();
    printf("%s\n", glfwGetVersionString());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    _mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "glfw3_events", NULL, NULL);
    glfwMakeContextCurrent(_mainWindow);

    glfwSetMouseButtonCallback(_mainWindow, on_mouse_callback);
    glfwSetCursorPosCallback(_mainWindow, on_mouse_move);
    glfwSetScrollCallback(_mainWindow, on_mouse_wheel);
    glfwSetKeyCallback(_mainWindow, on_key_action);
    //glfwSetCharCallback(_mainWindow, ...);

    for (int i = 0; i < g_test_count; ++i)
    {
        g_test_actual = i;
        emscripten_run_script(g_tests[g_test_actual].cmd);
    }

    glfwTerminate();

    success = (1 << (sizeof(g_tests) / sizeof(test_t))) - 1; // (2^count)-1

#ifdef REPORT_RESULT
    result = g_state == success;
    REPORT_RESULT();
#else
    printf("%d == %d = %d", g_state, success, g_state == success);
#endif

    return 0;
}
