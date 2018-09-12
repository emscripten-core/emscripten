/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifdef USE_GLFW
    #if USE_GLFW == 2
        #include <GL/glfw.h>
    #else
        #include <GLFW/glfw3.h>
    #endif
    #include <stdio.h>
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
        { "Module.injectMouseEvent(10.0, 10.0, 'mousedown', 0)", { 1, 10.0, 10.0, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, -1 } },
        { "Module.injectMouseEvent(10.0, 20.0, 'mouseup', 0)", { 1, 10.0, 20.0, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, -1 } },
        { "Module.injectMouseEvent(10.0, 30.0, 'mousedown', 1)", { 1, 10.0, 30.0, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS, -1 } },
        { "Module.injectMouseEvent(10.0, 40.0, 'mouseup', 1)", { 1, 10.0, 40.0, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_RELEASE, -1 } },
        { "Module.injectMouseEvent(10.0, 30.0, 'mousedown', 2)", { 1, 10.0, 30.0, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, -1 } },
        { "Module.injectMouseEvent(10.0, 40.0, 'mouseup', 2)", { 1, 10.0, 40.0, GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, -1 } },
        //{ "Module.injectMouseEvent(10.0, 50.0, 'mousewheel', 0)", { 10.0, 50.0, -1, -1, -1 } },
        //{ "Module.injectMouseEvent(10.0, 60.0, 'mousemove', 0)", { 10.0, 60.0, -1, -1, -1 } }

        { "Module.injectKeyEvent('keydown', 8)", { 0, 0.0, 0.0, GLFW_KEY_BACKSPACE, GLFW_PRESS, -1 } },
        { "Module.injectKeyEvent('keyup', 8)", { 0, 0.0, 0.0, GLFW_KEY_BACKSPACE, GLFW_RELEASE, -1 } },
        { "Module.injectKeyEvent('keydown', 9)", { 0, 0.0, 0.0, GLFW_KEY_TAB, GLFW_PRESS, -1 } },
        { "Module.injectKeyEvent('keyup', 9)", { 0, 0.0, 0.0, GLFW_KEY_TAB, GLFW_RELEASE, -1 } },
        { "Module.injectKeyEvent('keydown', 112)", { 0, 0.0, 0.0, GLFW_KEY_F1, GLFW_PRESS, -1 } },
        { "Module.injectKeyEvent('keyup', 112)", { 0, 0.0, 0.0, GLFW_KEY_F1, GLFW_RELEASE, -1 } },
        { "Module.injectKeyEvent('keydown', 37)", { 0, 0.0, 0.0, GLFW_KEY_LEFT, GLFW_PRESS, -1 } },
        { "Module.injectKeyEvent('keyup', 37)", { 0, 0.0, 0.0, GLFW_KEY_LEFT, GLFW_RELEASE, -1 } },
        { "Module.injectKeyEvent('keydown', 39)", { 0, 0.0, 0.0, GLFW_KEY_RIGHT, GLFW_PRESS, -1 } },
        { "Module.injectKeyEvent('keyup', 39)", { 0, 0.0, 0.0, GLFW_KEY_RIGHT, GLFW_RELEASE, -1 } },
        { "Module.injectKeyEvent('keydown', 38)", { 0, 0.0, 0.0, GLFW_KEY_UP, GLFW_PRESS, -1 } },
        { "Module.injectKeyEvent('keyup', 38)", { 0, 0.0, 0.0, GLFW_KEY_UP, GLFW_RELEASE, -1 } },
        { "Module.injectKeyEvent('keydown', 40)", { 0, 0.0, 0.0, GLFW_KEY_DOWN, GLFW_PRESS, -1 } },
        { "Module.injectKeyEvent('keyup', 40)", { 0, 0.0, 0.0, GLFW_KEY_DOWN, GLFW_RELEASE, -1 } },

        #if USE_GLFW == 2
            { "Module.injectKeyEvent('keydown', 27)", { 0, 0.0, 0.0, GLFW_KEY_ESC, GLFW_PRESS, -1 } },
            { "Module.injectKeyEvent('keyup', 27)", { 0, 0.0, 0.0, GLFW_KEY_ESC, GLFW_RELEASE, -1 } },

            { "Module.injectKeyEvent('keydown', 65)", { 0, 0.0, 0.0, 'A', GLFW_PRESS, -1, 'A' } },
            { "Module.injectKeyEvent('keypress', 65, {charCode: 65})", { 0, 0.0, 0.0, -1, -1, -1, 'A' } },
            { "Module.injectKeyEvent('keyup', 65)", { 0, 0.0, 0.0, 'A', GLFW_RELEASE, -1, 'A' } },

            { "Module.injectKeyEvent('keydown', 65, {ctrlKey: true})", { 0, 0.0, 0.0, 'A', GLFW_PRESS, -1, 'A' } },
            { "Module.injectKeyEvent('keypress', 65, {ctrlKey: true, charCode: 65})", { 0, 0.0, 0.0, -1, -1, -1, -1 } },
            { "Module.injectKeyEvent('keyup', 65, {ctrlKey: true})", { 0, 0.0, 0.0, 'A', GLFW_RELEASE, -1, 'A' } },
        #else
            { "Module.injectKeyEvent('keydown', 27)", { 0, 0.0, 0.0, GLFW_KEY_ESCAPE, GLFW_PRESS, -1 } },
            { "Module.injectKeyEvent('keyup', 27)", { 0, 0.0, 0.0, GLFW_KEY_ESCAPE, GLFW_RELEASE, -1 } },

            { "Module.injectKeyEvent('keydown', 65)", { 0, 0.0, 0.0, GLFW_KEY_A, GLFW_PRESS, -1 } },
            { "Module.injectKeyEvent('keypress', 65, {charCode: 65})", { 0, 0.0, 0.0, -1, -1, -1, 'A' } },
            { "Module.injectKeyEvent('keyup', 65)", { 0, 0.0, 0.0, GLFW_KEY_A, GLFW_RELEASE, -1 } },

            { "Module.injectKeyEvent('keydown', 65, {ctrlKey: true})", { 0, 0.0, 0.0, GLFW_KEY_A, GLFW_PRESS, -1, 'A' } },
            { "Module.injectKeyEvent('keypress', 65, {ctrlKey: true, charCode: 65})", { 0, 0.0, 0.0, -1, -1, -1, -1 } },
            { "Module.injectKeyEvent('keyup', 65, {ctrlKey: true})", { 0, 0.0, 0.0, GLFW_KEY_A, GLFW_RELEASE, -1, 'A' } },
        #endif
    };

    static unsigned int g_test_actual = 0;
    static unsigned int g_test_count = sizeof(g_tests) / sizeof(test_t);
    static unsigned int g_state = 0;

    #if USE_GLFW == 2
        static void on_mouse_button_callback(int button, int action)
    #else
        static void on_mouse_button_callback(GLFWwindow* window, int button, int action, int modify)
    #endif
    {
        test_args_t args = g_tests[g_test_actual].args;
        if (args.button == button && args.action == action)
        {
            g_state |= 1 << g_test_actual;
        }
        else
        {
            printf("Test %d: FAIL\n", g_test_actual);
        }
    }

    #if USE_GLFW == 2
        static void on_mouse_move(int x, int y)
    #else
        static void on_mouse_move(GLFWwindow* window, double x, double y)
    #endif
    {
        test_args_t args = g_tests[g_test_actual].args;
        if (args.x == x && args.y == y)
        {
            g_state |= 1 << g_test_actual;
        }
        else
        {
            printf("Test %d: FAIL\n", g_test_actual);
        }
    }

    #if USE_GLFW == 2
        static void on_key_callback(int key, int action)
    #else
        static void on_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    #endif
    {
        test_args_t args = g_tests[g_test_actual].args;
        if (args.button == key && args.action == action)
        {
            g_state |= 1 << g_test_actual;
        }
        else
        {
            printf("Test %d: FAIL\n", g_test_actual);
        }
    }

    #if USE_GLFW == 2
        static void on_char_callback(int character, int action)
    #else
        static void on_char_callback(GLFWwindow* window, unsigned int character)
    #endif
    {
        test_args_t args = g_tests[g_test_actual].args;
        if (args.character != -1 && args.character == character)
        {
            g_state |= 1 << g_test_actual;
        }
        else
        {
            printf("Test %d: FAIL\n", g_test_actual);
        }

    }

    #if USE_GLFW == 3
        static void on_mouse_wheel(GLFWwindow* window, double x, double y)
        {
            test_args_t args = g_tests[g_test_actual].args;
            if (args.x == x && args.y == y)
            {
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
    #endif

    int main()
    {
        int result = 1;
        unsigned int success = (1 << (sizeof(g_tests) / sizeof(test_t))) - 1; // (2^count)-1;

        emscripten_run_script(MULTILINE(
            Module.injectMouseEvent = function(x, y, event_, button) {
                var canvas = Module['canvas'];
                var event = new MouseEvent(event_, {
                    'view': window,
                    'bubbles': true,
                    'cancelable': true,
                    'screenX': canvas.offsetLeft + x,
                    'screenY': canvas.offsetTop + y,
                    'clientX': canvas.offsetLeft + x,
                    'clientY': canvas.offsetTop + y,
                    'button': button
                });
                canvas.dispatchEvent(event);

                //var event = document.createEvent("MouseEvents");
                //var canvas = Module['canvas'];
                //event.initMouseEvent(event_, true, true, window, 0, canvas.offsetLeft + x, canvas.offsetTop + y, canvas.offsetLeft + x, canvas.offsetTop + y, 0, 0, 0, 0, button, null);
                //canvas.dispatchEvent(event);
            };

            Module.injectKeyEvent = function(type, keyCode, options) {
                // KeyboardEvent constructor always returns 0 keyCode on Chrome, so use generic events
                //var keyboardEvent = new KeyboardEvent(type, Object.assign({ keyCode: keyCode}, options));
                var keyboardEvent = document.createEventObject ?
                        document.createEventObject() : document.createEvent('Events');
                keyboardEvent.initEvent(type, true, true);
                keyboardEvent.keyCode = keyCode;
                keyboardEvent = Object.assign(keyboardEvent,  options);

                canvas.dispatchEvent(keyboardEvent);
            };
        ));
        
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

        for (int p = 0; p < 2 && result; ++p) // 2 passes, with and without callbacks.
        {
            printf("Running Test pass %d\n", p);

        #if USE_GLFW == 2
            glfwSetMouseButtonCallback(p == 0 ? NULL : on_mouse_button_callback);
            glfwSetKeyCallback(p == 0 ? NULL : on_key_callback);
        #else
            glfwSetMouseButtonCallback(_mainWindow, p == 0 ? NULL : on_mouse_button_callback);
            glfwSetKeyCallback(_mainWindow, p == 0 ? NULL : on_key_callback);
        #endif
            g_state = p == 0 ? success : 0;

            for (int i = 0; i < g_test_count; ++i)
            {
                g_test_actual = i;
                test_t test = g_tests[g_test_actual];

                if (test.args.character == -1) {
                     g_state |= 1 << g_test_actual;
                }

                emscripten_run_script(test.cmd);

                if (test.args.mouse) {
                #if USE_GLFW == 2
                    if (glfwGetMouseButton(test.args.button) != test.args.action)
                #else
                    if (glfwGetMouseButton(_mainWindow, test.args.button) != test.args.action)
                #endif
                    {
                        printf("Test %d: FAIL\n", g_test_actual);
                        g_state &= ~(1 << g_test_actual);
                    }
                } else {
                    // Keyboard.
                #if USE_GLFW == 2
                    if (test.args.action != -1 && glfwGetKey(test.args.button) != test.args.action)
                #else
                    if (test.args.action != -1 && glfwGetKey(_mainWindow, test.args.button) != test.args.action)
                #endif
                    {
                        printf("Test %d: FAIL\n", g_test_actual);
                        g_state &= ~(1 << g_test_actual);
                    }
                }
            }
            result = g_state == success;
        }

        glfwTerminate();

    #ifdef REPORT_RESULT
        REPORT_RESULT(result);
    #else
        printf("%d == %d = %d", g_state, success, g_state == success);
    #endif

        return 0;
    }
#endif