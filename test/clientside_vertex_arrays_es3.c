/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>
#include <stdlib.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void onDraw(void* arg) {
  GLFWwindow* window = *((GLFWwindow**)arg);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glfwSwapBuffers(window);
  glfwPollEvents();
}

void onResize(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void onClose(GLFWwindow* window) {
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

int main() {
  GLFWwindow* window;

  if (!glfwInit()) {
    return EXIT_FAILURE;
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  window = glfwCreateWindow(640, 480, "Client-side Vertex Arrays", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwSetFramebufferSizeCallback(window, onResize);
  glfwSetWindowCloseCallback(window, onClose);
  glfwMakeContextCurrent(window);

  GLuint vertex_shader;
  GLuint fragment_shader;
  GLuint program;
  {
    const GLchar* source = "#version 300 es                            \n"
                           "                                           \n"
                           "in vec2 aPosition;                         \n"
                           "in uvec3 aColor;                           \n"
                           "out vec4 color;                            \n"
                           "                                           \n"
                           "void main()                                \n"
                           "{                                          \n"
                           "  gl_Position = vec4(aPosition, 0.f, 1.f); \n"
                           "  color = vec4(vec3(aColor) / 255.f, 1.f); \n"
                           "}                                          \n";
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &source, NULL);
    glCompileShader(vertex_shader);
  }
  {
    const GLchar* source = "#version 300 es          \n"
                           "                         \n"
                           "precision mediump float; \n"
                           "                         \n"
                           "in vec4 color;           \n"
                           "out vec4 FragColor;      \n"
                           "                         \n"
                           "void main()              \n"
                           "{                        \n"
                           "  FragColor = color;     \n"
                           "}                        \n";
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &source, NULL);
    glCompileShader(fragment_shader);
  }
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  glUseProgram(program);
  {
    static const float position[] = {
      -1.f, -1.f,
       1.f, -1.f,
       0.f,  1.f,
    };
    GLint location;
    location = glGetAttribLocation(program, "aPosition");
    glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 0, position);
    glEnableVertexAttribArray(location);
  }
  {
    static const unsigned int color[] = {
      255,   0,   0,
        0, 255,   0,
        0,   0, 255,
    };
    GLint location;
    location = glGetAttribLocation(program, "aColor");
    glVertexAttribIPointer(location, 3, GL_UNSIGNED_INT, 0, color);
    glEnableVertexAttribArray(location);
  }

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(onDraw, &window, 0, 1);
#else
  while (1) {
    onDraw(&window);
  }
#endif

  return EXIT_FAILURE; // not reached
}
