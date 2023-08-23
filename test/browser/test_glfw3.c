/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void errorcb(int error, const char *msg) { (void)error; (void)msg; }
static void monitcb(GLFWmonitor *monitor, int event) { assert(monitor != NULL); (void)event; }
static void wposicb(GLFWwindow *window, int x, int y) { assert(window != NULL); (void)x; (void)y; }
static void wsizecb(GLFWwindow *window, int w, int h) { assert(window != NULL); (void)w; (void)h; }
static void wscalcb(GLFWwindow *window, float xscale, float yscale) { assert(window != NULL); (void)xscale; (void)yscale; }
static void wcloscb(GLFWwindow *window) { assert(window != NULL); }
static void wrfrscb(GLFWwindow *window) { assert(window != NULL); }
static void wfocucb(GLFWwindow *window, int focused) { assert(window != NULL); (void)focused; }
static void wiconcb(GLFWwindow *window, int iconified) { assert(window != NULL); (void)iconified; }
static void wmaxicb(GLFWwindow *window, int maximized) { assert(window != NULL); (void)maximized;}
static void wfsizcb(GLFWwindow *window, int w, int h) { assert(window != NULL); (void)w; (void)h; }
static void wkeypcb(GLFWwindow *window, int key, int scancode, int action, int mods) {
    assert(window != NULL); (void)key; (void)scancode; (void)action; (void)mods;
}
static void wcharcb(GLFWwindow *window, unsigned int cp) { assert(window != NULL); (void)cp; }
static void wmbutcb(GLFWwindow *window, int button, int action, int mods) {
    assert(window != NULL); (void)button; (void)action; (void)mods;
}
static void wcurpcb(GLFWwindow *window, double x, double y) { assert(window != NULL); (void)x; (void)y; }
static void wcurecb(GLFWwindow *window, int entered) { assert(window != NULL); (void)entered; }
static void wscrocb(GLFWwindow *window, double x, double y) { assert(window != NULL); (void)x; (void)y; }
static void wdropcb(GLFWwindow *window, int count, const char **paths) {
    assert(window != NULL); (void)count; (void)paths;
}

#define TEST_GLFW_SET_I(Function, Value) \
assert(glfwSet##Function(Value) == NULL);  /* Default value (no callback was set) */ \
assert(glfwSet##Function(Value) == Value); /* The previously set callback */

#define TEST_GLFW_SET_II(Function, Window, Value) \
assert(glfwSet##Function(Window, Value) == NULL);  /* Default value (no callback was set) */ \
assert(glfwSet##Function(Window, Value) == Value); /* The previously set callback */

static int exited = 0;

__attribute__((destructor))
void onExit() {
  exited = 1;
}

int main()
{
    GLFWwindow *window;
    char *userptr = "userptr";

    TEST_GLFW_SET_I(ErrorCallback, errorcb)
    assert(glfwInit() == GL_TRUE);
    assert(!strcmp(glfwGetVersionString(), "3.2.1 JS WebGL Emscripten"));
    assert(glfwGetCurrentContext() == NULL);

    {
        int major, minor, rev;
        glfwGetVersion(&major, &minor, &rev);
        assert(major == 3);
        assert(minor == 2);
        assert(rev == 1);
    }

    {
        int count, x, y, w, h;
        float xs, ys;
        GLFWmonitor **monitors = glfwGetMonitors(&count);
        assert(count == 1);
        for (int i = 0; i < count; ++i) {
            assert(monitors[i] != NULL);
        }

        assert(glfwGetPrimaryMonitor() != NULL);
        glfwGetMonitorPos(monitors[0], &x, &y);
        glfwGetMonitorPhysicalSize(monitors[0], &w, &h);
        glfwGetMonitorWorkarea(monitors[0], &x, &y, &w, &h);
        glfwGetMonitorContentScale(monitors[0], &xs, &ys);
        assert(glfwGetMonitorName(monitors[0]) != NULL);
        TEST_GLFW_SET_I(MonitorCallback, monitcb)

        // XXX: not implemented
        // assert(glfwGetVideoModes(monitors[0], &count) != NULL);
        // assert(glfwGetVideoMode(monitors[0]) != NULL);
        // glfwSetGamma(monitors[0], 1.0f);
        // assert(glfwGetGammaRamp(monitors[0]) != NULL);
        // glfwSetGammaRamp(monitors[0], ramp);
    }

    {
        int x, y, w, h;
        float xscale, yscale;
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, CLIENT_API);
        glfwWindowHintString(GLFW_X11_CLASS_NAME, "Will be Ignored :)");
        window = glfwCreateWindow(640, 480, "glfw3.c", NULL, NULL);
        assert(window != NULL);

        TEST_GLFW_SET_II(WindowPosCallback, window, wposicb)
        TEST_GLFW_SET_II(WindowSizeCallback, window, wsizecb)
        TEST_GLFW_SET_II(WindowContentScaleCallback, window, wscalcb)
        TEST_GLFW_SET_II(WindowCloseCallback, window, wcloscb)
        TEST_GLFW_SET_II(WindowRefreshCallback, window, wrfrscb)
        TEST_GLFW_SET_II(WindowFocusCallback, window, wfocucb)
        TEST_GLFW_SET_II(WindowIconifyCallback, window, wiconcb)
        TEST_GLFW_SET_II(WindowMaximizeCallback, window, wmaxicb)
        TEST_GLFW_SET_II(FramebufferSizeCallback, window, wfsizcb)
        TEST_GLFW_SET_II(KeyCallback, window, wkeypcb)
        TEST_GLFW_SET_II(CharCallback, window, wcharcb)
        TEST_GLFW_SET_II(MouseButtonCallback, window, wmbutcb)
        TEST_GLFW_SET_II(CursorPosCallback, window, wcurpcb)
        TEST_GLFW_SET_II(CursorEnterCallback, window, wcurecb)
        TEST_GLFW_SET_II(ScrollCallback, window, wscrocb)
        TEST_GLFW_SET_II(DropCallback, window, wdropcb)

        assert(glfwWindowShouldClose(window) == 0);
        glfwSetWindowShouldClose(window, 1);
        assert(glfwWindowShouldClose(window) == 1);

        glfwSetWindowTitle(window, "test");
        glfwSetWindowTitle(window, "glfw3.c");

        // XXX: not implemented
        // glfwSetWindowPos(window, 1, 1);

        glfwGetWindowPos(window, &x, &y); // stub
        glfwGetWindowSize(window, &w, &h);
        assert(w == 640 && h == 480);

        glfwSetWindowSize(window, 1, 1);
        glfwGetWindowSize(window, &w, &h);
        assert(w == 1 && h == 1);
        assert(exited == 0);

        glfwSetWindowSize(window, 640, 480);
        glfwGetFramebufferSize(window, &w, &h);
        assert(exited == 0);

        float opacity = glfwGetWindowOpacity(window); // always returns 1.0 for now
        glfwSetWindowOpacity(window, opacity); // ignored.
        // how to check if x/yscale are correct? use emscripten_device_pixel_ratio from html5 header?
        glfwGetWindowContentScale(window, &xscale, &yscale);
        assert(opacity == 1.0);
        assert(exited == 0);
        // XXX: not implemented
        // glfwIconifyWindow(window);
        // glfwRestoreWindow(window);
        // glfwShowWindow(window);
        // glfwHideWindow(window);

        assert(glfwGetWindowMonitor(window) == NULL);
        glfwDestroyWindow(window);

        window = glfwCreateWindow(640, 480, "glfw3.c", glfwGetPrimaryMonitor(), NULL);
        assert(window != NULL);
        assert(glfwGetWindowMonitor(window) == glfwGetPrimaryMonitor());
        glfwDestroyWindow(window);

        window = glfwCreateWindow(640, 480, "glfw3.c", NULL, NULL);
        assert(window != NULL);

        assert(glfwGetWindowAttrib(window, GLFW_CLIENT_API) == CLIENT_API);
        glfwSetWindowAttrib(window, GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        assert(glfwGetWindowAttrib(window, GLFW_TRANSPARENT_FRAMEBUFFER) == GLFW_TRUE);
        assert(glfwGetWindowUserPointer(window) == NULL);
        glfwSetWindowUserPointer(window, userptr);
        assert(glfwGetWindowUserPointer(window) == userptr);
    }

    {
        double x, y;

        glfwSetKeyCallback(window, wkeypcb);
        glfwSetCharCallback(window, wcharcb);
        glfwSetMouseButtonCallback(window, wmbutcb);
        glfwSetCursorPosCallback(window, wcurpcb);
        glfwSetCursorEnterCallback(window, wcurecb);
        glfwSetScrollCallback(window, wscrocb);

        // XXX: stub, events come immediatly
        // glfwPollEvents();
        // glfwWaitEvents();

        assert(glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL);

        // XXX: not implemented
        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        assert(glfwRawMouseMotionSupported() == 0); // constant false
        glfwGetKey(window, GLFW_KEY_A);
        glfwGetMouseButton(window, 0);
        glfwGetCursorPos(window, &x, &y);

        // XXX: not implemented
        // glfwSetCursorPos(window, 0, 0);
    }

    {
        // XXX: not implemented
        // glfwJoystickPresent(joy);
        // glfwGetJoystickAxes(joy, &count);
        // glfwGetJoystickButtons(joy, &count);
        // glfwGetJoystickName(joy);
    }

    {
        // XXX: not implemented
        // glfwSetClipboardString(window, "string");
        // glfwGetClipboardString(window);
    }

    {
        glfwGetTime();
        glfwSetTime(0);
    }

#if CLIENT_API == GLFW_OPENGL_ES_API
    {
        glfwMakeContextCurrent(window); // stub
        assert(glfwGetCurrentContext() == window);
        glfwSwapBuffers(window); // stub
        glfwSwapInterval(0); // stub
    }

    {
        assert(glfwExtensionSupported("nonexistant") == 0);
        assert(glfwGetProcAddress("nonexistant") == NULL);
    }
#endif

    glfwTerminate();

#ifdef REPORT_RESULT
    REPORT_RESULT(1);
#endif
    return 0;
}
