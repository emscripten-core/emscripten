/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <GLFW/glfw3.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

GLFWwindow *window;

static int step = 1;
static int last_state = -1;
void render() {
    // http://www.glfw.org/docs/latest/input_guide.html#input_key
    int state = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    if (last_state != state) { // to not spam console
        last_state = state;
        printf("glfwGetKey says space pressed? %d\n", state);
        if (state && step == 1) {
            printf("%d. Switch out of your browser to another window (while holding space)\n", step++);
        }
        if (!state && step == 2) {
            printf("You let go, please press and hold spacebar\n");
            step = 1;
        }
    }

    // Red while space is pressed, green while not
    if (state) {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    } else {
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT);
}

#ifdef __EMSCRIPTEN__
bool on_focuspocus(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData) {
    switch(eventType) {
        case EMSCRIPTEN_EVENT_BLUR:
            printf("blur\n");
            if (step == 2) {
                printf("%d. Switch back to the browser now\n", step++);
            }
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                printf("FAIL: glfwGetKey() is stuck after blur\n");
                assert(false);
            }
            break;
        case EMSCRIPTEN_EVENT_FOCUS:
            printf("focus\n");
            if (step == 3) {
                if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                    printf("FAIL: glfwGetKey() is stuck after blur and focus\n");
                    assert(false);
                }
                printf("Pass\n");
                glfwTerminate();
            }
            break;
        case EMSCRIPTEN_EVENT_FOCUSIN:
            printf("focusin\n");
            break;
        case EMSCRIPTEN_EVENT_FOCUSOUT:
            printf("focusout\n");
            break;
        default:
            printf("focus event %d\n", eventType);
            break;
    }
    return false;
}

#endif

int main() {
    if (!glfwInit()) {
        return 1;
    }

    window = glfwCreateWindow(640, 480, "test_glfw_get_key_stuck", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    printf("%d. Press and hold spacebar\n", step);

#ifdef __EMSCRIPTEN__
    emscripten_set_blur_callback(NULL, NULL, true, on_focuspocus);
    emscripten_set_focus_callback(NULL, NULL, true, on_focuspocus);
    emscripten_set_focusin_callback(NULL, NULL, true, on_focuspocus);
    emscripten_set_focusout_callback(NULL, NULL, true, on_focuspocus);

    emscripten_set_main_loop(render, 0, 1);
    __builtin_trap();
#else
    while (!glfwWindowShouldClose(window)) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
#endif

    glfwTerminate();
    return 0;
}
