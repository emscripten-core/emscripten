// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <GL/gl.h>
#include <GL/glut.h>

int main(int argc, char* argv[]) {
  bool stencilActivated = false;
  bool depthActivated = false;
  bool alphaActivated = false;
  bool antiAliasingActivated = false;

  unsigned int mode = GLUT_RGBA | GLUT_DOUBLE;

#ifdef STENCIL_ACTIVATED
    stencilActivated = true;
    mode |= GLUT_STENCIL;
#endif
#ifdef DEPTH_ACTIVATED
    depthActivated = true;
    mode |= GLUT_DEPTH;
#endif
#ifdef ALPHA_ACTIVATED
    alphaActivated = true;
    mode |= GLUT_ALPHA;
#endif
#ifdef AA_ACTIVATED
    antiAliasingActivated = true;
    mode |= GLUT_MULTISAMPLE;
#endif

  glutInit(&argc, argv);
  glutInitWindowSize(640, 480);
  glutInitDisplayMode(mode);
  glutCreateWindow(__FILE__);

  printf("stencil:   %d\n", glutGet(GLUT_WINDOW_STENCIL_SIZE));
  printf("depth:     %d\n", glutGet(GLUT_WINDOW_DEPTH_SIZE));
  printf("alpha:     %d\n", glutGet(GLUT_WINDOW_ALPHA_SIZE));
  printf("antialias: %d\n", glutGet(GLUT_WINDOW_NUM_SAMPLES));
  assert(!stencilActivated      || glutGet(GLUT_WINDOW_STENCIL_SIZE) > 0);
  assert(!depthActivated        || glutGet(GLUT_WINDOW_DEPTH_SIZE)   > 0);
  assert(!alphaActivated        || glutGet(GLUT_WINDOW_ALPHA_SIZE)   > 0);
  assert(!antiAliasingActivated || glutGet(GLUT_WINDOW_NUM_SAMPLES)  > 0);

  // fix-up "ReferenceError: GL is not defined,createContext" due to
  // overzealous JS stripping
  glClear(0);

  return 0;
}
