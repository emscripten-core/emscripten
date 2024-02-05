#include "SDL.h"
#include "SDL_opengl.h"

#include <stdio.h>
#include <string.h>

#define WIDTH 640
#define HEIGHT 480

int main(int argc, char* argv[]) {
  SDL_Window* window;
  SDL_GLContext context;
  int i, j;

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  window = SDL_CreateWindow(
    "OpenGL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
  if (!window) {
    fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
    return 0;
  }

  context = SDL_GL_CreateContext(window);
  if (!context) {
    fprintf(stderr, "Couldn't create context: %s\n", SDL_GetError());
    return 0;
  }

  glMatrixMode(GL_PROJECTION);
  glOrtho(0.0, 8.0, 0.0, 3.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);

  glClearColor(0, 0, 0, 1);

  glEnable(GL_TEXTURE_2D);

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  GLubyte texture_data[] = {
    0xffu, 0xffu, 0xffu, 0xffu,
    0xffu, 0xffu, 0xffu, 0xeeu,
    0xffu, 0xffu, 0xffu, 0xddu,
    0xffu, 0xffu, 0xffu, 0xccu,

    0xffu, 0xffu, 0xffu, 0xbbu,
    0xffu, 0xffu, 0xffu, 0xaau,
    0xffu, 0xffu, 0xffu, 0x99u,
    0xffu, 0xffu, 0xffu, 0x88u,

    0xffu, 0xffu, 0xffu, 0x77u,
    0xffu, 0xffu, 0xffu, 0x66u,
    0xffu, 0xffu, 0xffu, 0x55u,
    0xffu, 0xffu, 0xffu, 0x44u,

    0xffu, 0xffu, 0xffu, 0x33u,
    0xffu, 0xffu, 0xffu, 0x22u,
    0xffu, 0xffu, 0xffu, 0x11u,
    0xffu, 0xffu, 0xffu, 0x00u,
  };

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

  const GLfloat vertex_data[4][4] = {
    { 0.25f, 0.25f, 0.0f, 0.0f },
    { 0.75f, 0.25f, 1.0f, 0.0f },
    { 0.25f, 0.75f, 0.0f, 1.0f },
    { 0.75f, 0.75f, 1.0f, 1.0f }
  };

  const GLushort indices[] = {
    0u, 1u, 2u, 2u, 1u, 3u,
  };

  const GLenum alpha_test_modes[8] = {
    GL_NEVER, GL_LESS, GL_LEQUAL, GL_EQUAL,
    GL_NOTEQUAL, GL_GEQUAL, GL_GREATER, GL_ALWAYS
  };

  glVertexPointer(2, GL_FLOAT, sizeof (vertex_data[0]), &vertex_data[0][0]);
  glEnableClientState(GL_VERTEX_ARRAY);

  glTexCoordPointer(2, GL_FLOAT, sizeof (vertex_data[0]), &vertex_data[0][2]);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_ALPHA_TEST);

  glClear(GL_COLOR_BUFFER_BIT);

  for(i = 0; i < 8; i++) {
    for(j = 0; j < 3; j++) {
      glLoadIdentity();
      glTranslatef(i, j, 0.0f);

      glAlphaFunc(alpha_test_modes[i], ((float)j)/2.0f );
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    }
  }

  SDL_GL_SwapWindow(window);

  glDeleteTextures(1, &texture);

  // Don't quit - we need to reftest the canvas! SDL_Quit();

  return 0;
}
