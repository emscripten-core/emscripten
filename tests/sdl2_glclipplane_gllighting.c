#include "SDL.h"
#include "SDL_opengl.h"

#include <stdio.h>
#include <string.h>

#define WIDTH 640
#define HEIGHT 480

const GLfloat light_ambient[]  = {0.0f, 0.0f, 0.0f, 1.0f};
const GLfloat light_diffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
const GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
const GLfloat light_position[] = {20.0f, 5.0f, 5.0f, 0.0f};
const GLfloat mat_ambient[]    = {0.7f, 0.7f, 0.7f, 1.0f};
const GLfloat mat_diffuse[]    = {0.8f, 0.8f, 0.8f, 1.0f};
const GLfloat mat_specular[]   = {1.0f, 1.0f, 1.0f, 1.0f};
const GLfloat high_shininess[] = {100.0f};

const float PI2 = 2.0f*3.1415926535;

int NumWraps = 100;
int NumPerWrap = 80;
float MajorRadius = 0.5f;
float MinorRadius = 0.2f;

void putVert(int i, int j) {
  float wrapFrac = (j%NumPerWrap)/(float)NumPerWrap;
  float phi = PI2*wrapFrac;
  float theta = PI2*(i%NumWraps+wrapFrac)/(float)NumWraps;
  float sinphi = sin(phi);
  float cosphi = cos(phi);
  float sintheta = sin(theta);
  float costheta = cos(theta);
  float y = MinorRadius*sinphi;
  float r = MajorRadius + MinorRadius*cosphi;
  float x = sintheta*r;
  float z = costheta*r;
  glNormal3f(sintheta*cosphi, sinphi, costheta*cosphi);
  glVertex3f(x,y,z);
}

int main(int argc, char* argv[]) {
  SDL_Window* window;
  SDL_GLContext context;
  int i,j;

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
  glOrtho(-1.0f, 1.0f, -1.0f * HEIGHT/WIDTH, 1.0f * HEIGHT/WIDTH, -1.0f, 1.0f);
  glMatrixMode(GL_MODELVIEW);

  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    // Clear the screen before drawing
  glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHT0);

  glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

  glEnable(GL_CLIP_PLANE0);

  glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
  glRotatef(60.0f, 0.0f, 1.0f, 0.0f);

  const GLdouble clip_plane[]   = {0.0, 0.0, -1.0, 0.4};
  glClipPlane(GL_CLIP_PLANE0, clip_plane);


  glColor3f(1.0f, 0.5f, 1.0f);
  glBegin(GL_TRIANGLE_STRIP);

  for (i=0; i<NumWraps; i++ ) {
  for (j=0; j<NumPerWrap; j++) {
  putVert(i,j);
  putVert(i+1,j);
  }
  }
  putVert(0,0);
  putVert(1,0);

  glEnd();


  SDL_GL_SwapWindow(window);

  // Don't quit - we need to reftest the canvas! SDL_Quit();

  return 0;
}
