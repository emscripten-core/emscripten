#include <stdio.h>
#include <GLFW/glfw3.h>

int result = 0;
int main() {
    if (!glfwInit()) {
        return -1;
    }

    float t = glfwGetTime();
    printf("glfwGetTime() = %f\n", t);

    printf("glfwSetTime(50)\n");
    glfwSetTime(50);

    // Expect time to be slightly greater than what we set
    t = glfwGetTime();
    printf("glfwGetTime() = %f\n", t);

    if (t < 50 + 1e-3) {
        result = 1;
    }

    glfwTerminate();

#ifdef REPORT_RESULT
    REPORT_RESULT(result);
#endif

    return 0;
}
