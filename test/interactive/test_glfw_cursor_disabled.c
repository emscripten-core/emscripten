/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

GLFWwindow *window;

int last_cursor_disabled = -1;
int pointerlock_isActive = 0;

void render() {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int cursor_disabled = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;

    if (cursor_disabled != last_cursor_disabled) {
        last_cursor_disabled = cursor_disabled;

        printf("GLFW_CURSOR_DISABLED? %d\n", cursor_disabled);

        static int step = 2;
        if (cursor_disabled == pointerlock_isActive) {
            printf("Pass %d: glfwGetInputMode GLFW_CURSOR matches pointerlockchange event\n\n", step++);

            if (step == 5) {
                printf("All tests passed.\n");
                exit(0);
            }

            if (cursor_disabled) printf("Press escape to exit Pointer Lock\n");
            else printf("Click again to enable Pointer Lock\n");
        } else {
            printf("FAIL: cursor_disabled(%d) != pointerlock_isActive(%d)\n", cursor_disabled, pointerlock_isActive);
            exit(1);
        }
    }
}


#ifdef __EMSCRIPTEN__
bool on_pointerlockchange(int eventType, const EmscriptenPointerlockChangeEvent *event, void *userData) {
    printf("pointerlockchange, isActive=%d\n", event->isActive);
    pointerlock_isActive = event->isActive;
    return 0;
}
#endif

int main() {
    if (!glfwInit()) {
        return -1;
    }

    window = glfwCreateWindow(640, 480, "test_glfw_cursor_disabled", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        // Browsers do not allow disabling the cursor (Pointer Lock) without a gesture.
        printf("FAIL: glfwGetInputMode returned GLFW_CURSOR_DISABLED prematurely\n");
        exit(1);
    }
    printf("Pass 1: glfwGetInputMode not prematurely returning cursor disabled\n");
    printf("Click within the canvas to activate Pointer Lock\n");

#ifdef __EMSCRIPTEN__
    emscripten_set_pointerlockchange_callback(NULL, NULL, 0, on_pointerlockchange);
    emscripten_set_main_loop(render, 0, 1);
    __builtin_trap();
#else
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
#endif

    glfwTerminate();
    return 0;
}
