/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#define GLFW_INCLUDE_ES2 
#include <GL/glfw.h>

int main() {
    printf("main function started\n");
    if (glfwInit() != GL_TRUE) {
        printf("glfwInit() failed\n");
        glfwTerminate();
    } else {
        printf("glfwInit() success\n");
        if (glfwOpenWindow(640, 480, 8, 8, 8, 8, 16, 0, GLFW_WINDOW) != GL_TRUE){
            printf("glfwOpenWindow() failed\n");
            glfwTerminate();
        } else {
          printf("glfwOpenWindow() success\n");
        }
    }
    return 0;
}

