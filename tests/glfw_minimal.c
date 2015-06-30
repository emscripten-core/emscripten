#include<stdio.h>
#include<stdlib.h>
#include<emscripten/emscripten.h>
#define GLFW_INCLUDE_ES2 
#include<GL/glfw.h>

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
#ifdef REPORT_RESULT  
    int result = 1;
    REPORT_RESULT();
#endif
    return EXIT_SUCCESS;
}

