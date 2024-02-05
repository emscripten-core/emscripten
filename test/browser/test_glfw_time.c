/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

int main() {
    if (!glfwInit()) {
        return 1;
    }

    float t = glfwGetTime();
    printf("glfwGetTime() = %f\n", t);

    printf("glfwSetTime(50)\n");
    glfwSetTime(50);

    // Expect time to be slightly greater than what we set
    t = glfwGetTime();
    printf("glfwGetTime() = %f\n", t);
    assert(t < 50 + 1e-3);

    glfwTerminate();

    return 0;
}
