/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifdef __EMSCRIPTEN__
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#else
#include <GL/glew.h>
#endif
#include <GL/glut.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef __EMSCRIPTEN__
int fullscreen;
#endif

void trace(char* tag) {
  static char* prev_tag = NULL;
  static int prev_screen_width;
  static int prev_screen_height;
  static int prev_window_width;
  static int prev_window_height;
  static int prev_viewport_width;
  static int prev_viewport_height;
  static int prev_is_fullscreen;
  static int prev_is_resized;
  static int prev_coalesced;
  int screen_width;
  int screen_height;
  int window_width;
  int window_height;
  int viewport_width;
  int viewport_height;
  int is_fullscreen;
  int is_resized;
  int coalesced;

  GLint viewport[4];
  screen_width = glutGet(GLUT_SCREEN_WIDTH);
  screen_height = glutGet(GLUT_SCREEN_HEIGHT);
  window_width = glutGet(GLUT_WINDOW_WIDTH);
  window_height = glutGet(GLUT_WINDOW_HEIGHT);
  glGetIntegerv(GL_VIEWPORT, viewport);
  viewport_width = viewport[2];
  viewport_height = viewport[3];
#ifdef __EMSCRIPTEN__
  EmscriptenFullscreenChangeEvent fullscreen_status;
  emscripten_get_fullscreen_status(&fullscreen_status);
  is_fullscreen = fullscreen_status.isFullscreen;
  is_fullscreen = is_fullscreen ? 1 : -1;
  is_resized = EM_ASM_INT({
    return document.getElementById('resize').checked;
  });
  is_resized = is_resized ? 1 : -1;
#else
  is_fullscreen = 0;
  is_resized = 1;
#endif
  coalesced = prev_tag &&
              !strcmp(tag, prev_tag) &&
              screen_width == prev_screen_width &&
              screen_height == prev_screen_height &&
              window_width == prev_window_width &&
              window_height == prev_window_height &&
              viewport_width == prev_viewport_width &&
              viewport_height == prev_viewport_height &&
              is_fullscreen == prev_is_fullscreen &&
              is_resized == prev_is_resized;

  if (coalesced) {
    if (!prev_coalesced) {
      printf("...\n");
    }
  } else {
    time_t t = time(NULL);
    char* local_time = ctime(&t);
    char* message = malloc(strlen(local_time) + strlen(tag) + 500);
    sprintf(message, "[");
    sprintf(message + strlen(message), "%s", local_time);
    sprintf(message + strlen(message) - 1, " ");
    sprintf(message + strlen(message), "%s", tag);
    sprintf(message + strlen(message), "]");
    sprintf(message + strlen(message), " ");
    sprintf(message + strlen(message),
            "screen width: %d"
            ", "
            "screen height: %d"
            ", "
            "window width: %d"
            ", "
            "window height: %d"
            ", "
            "viewport width: %d"
            ", "
            "viewport height: %d"
            ", "
            "fullscreen: %s"
            ", "
            "resize: %s",
            screen_width,
            screen_height,
            window_width,
            window_height,
            viewport_width,
            viewport_height,
            is_fullscreen > 0 ? "yes" : (is_fullscreen < 0 ? "no" : "unknown"),
            is_resized > 0 ? "yes" : (is_resized < 0 ? "no" : "unknown"));
    printf("%s\n", message);
    free(message);
  }

  prev_tag = tag;
  prev_screen_width = screen_width;
  prev_screen_height = screen_height;
  prev_window_width = window_width;
  prev_window_height = window_height;
  prev_viewport_width = viewport_width;
  prev_viewport_height = viewport_height;
  prev_is_fullscreen = is_fullscreen;
  prev_is_resized = is_resized;
  prev_coalesced = coalesced;
}

void onDisplay()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glutSwapBuffers();
  trace("onDisplay");
}

void onReshape(int width, int height)
{
  glViewport(0, 0, width, height);
  trace("onReshape");
}

void onKeyboard(unsigned char key, int x, int y)
{
  if (key == 'f') {
#ifdef __EMSCRIPTEN__
    EmscriptenFullscreenChangeEvent fullscreen_status;
    int fullscreen;
    emscripten_get_fullscreen_status(&fullscreen_status);
    fullscreen = fullscreen_status.isFullscreen;
#endif
    if (fullscreen) {
      glutReshapeWindow(glutGet(GLUT_INIT_WINDOW_WIDTH), glutGet(GLUT_INIT_WINDOW_HEIGHT));
    } else {
      glutFullScreen();
    }
#ifndef __EMSCRIPTEN__
    fullscreen = !fullscreen;
#endif
    trace("onKeyboard");
  }
}

void onIdle()
{
  glutPostRedisplay();
}

int main(int argc, char* argv[])
{
  int win;
  GLuint vertex_shader;
  GLuint fragment_shader;
  GLuint program;
  GLuint vbo;
  GLuint vao;
  GLfloat vertices[] = {
    /*
       x,    y,
    */

    -1.f, -1.f,
     1.f, -1.f,
    -1.f,  1.f,

     1.f, -1.f,
     1.f,  1.f,
    -1.f,  1.f,
  };

  glutInit(&argc, argv);
  glutInitWindowSize(600, 450);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  win = glutCreateWindow(__FILE__);

#ifndef __EMSCRIPTEN__
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    printf("error: %s\n", glewGetErrorString(err));
    glutDestroyWindow(win);
    return EXIT_FAILURE;
  }
#endif

  glEnable(GL_DEPTH_TEST);
  {
    const GLchar* str = "precision mediump float;\n"
                        "attribute vec2 aPosition;\n"
                        "varying vec2 vPosition;\n"
                        "void main()\n"
                        "{\n"
                        "  gl_Position = vec4(aPosition, 0.0, 1.0);\n"
                        "  vPosition = aPosition;\n"
                        "}\n";
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &str, NULL);
    glCompileShader(vertex_shader);
    {
      GLint params;
      GLchar* log;
      glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &params);
      if (params == GL_FALSE) {
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &params);
        log = malloc(params);
        glGetShaderInfoLog(vertex_shader, params, NULL, log);
        printf("%s", log);
        free(log);
        glutDestroyWindow(win);
        return EXIT_FAILURE;
      }
    }
  }
  {
    const GLchar* str = "precision mediump float;\n"
                        "varying vec2 vPosition;\n"
                        "void main()\n"
                        "{\n"
                        "  if (abs(vPosition.x) > 0.9 || abs(vPosition.y) > 0.9) {\n"
                        "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
                        "  } else {\n"
                        "    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
                        "  }\n"
                        "}\n";
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &str, NULL);
    glCompileShader(fragment_shader);
    {
      GLint params;
      GLchar* log;
      glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &params);
      if (params == GL_FALSE) {
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &params);
        log = malloc(params);
        glGetShaderInfoLog(fragment_shader, params, NULL, log);
        printf("%s", log);
        free(log);
        glutDestroyWindow(win);
        return EXIT_FAILURE;
      }
    }
  }
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glBindAttribLocation(program, 0, "aPosition");
  glLinkProgram(program);
  {
    GLint params;
    GLchar* log;
    glGetProgramiv(program, GL_LINK_STATUS, &params);
    if (params == GL_FALSE) {
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &params);
      log = malloc(params);
      glGetProgramInfoLog(program, params, NULL, log);
      printf("%s", log);
      free(log);
      glutDestroyWindow(win);
      return EXIT_FAILURE;
    }
  }
  glUseProgram(program);
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
  glEnableVertexAttribArray(0);

#ifndef __EMSCRIPTEN__
  fullscreen = 0;
#endif

  printf("You should see a green rectangle with red borders.\n");
  printf("Press 'f' or click the 'Fullscreen' button on the upper right corner to enter full screen, and press 'f' or ESC to exit.\n");
  printf("No matter 'Resize canvas' is checked or not, you should see the whole screen filled by the rectangle when in full screen, and after exiting, the rectangle should be restored in the window.\n");

  glutDisplayFunc(onDisplay);
  glutReshapeFunc(onReshape);
  glutKeyboardFunc(onKeyboard);
  glutIdleFunc(onIdle);
  glutMainLoop();

  glutDestroyWindow(win);
  return EXIT_SUCCESS;
}
