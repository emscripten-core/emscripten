/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <EGL/egl.h>
#include <emscripten.h>

#define MULTILINE(...) #__VA_ARGS__

int wheel_up = 0;
int wheel_down = 0;

void mouseCB(int button, int state, int x, int y) {
  if (button == 3) {
    wheel_up = 1;
  } else if (button == 4) {
    wheel_down = 1;
  }
}

int main(int argc, char *argv[]) {
  emscripten_run_script(MULTILINE(
      Module.injectWheelEvent = function(x, y, delta) {
          var isFirefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
          var event = document.createEvent("MouseEvents");
          if (!isFirefox) {
          // mouse wheel event for IE9, Chrome, Safari, Opera
          event.initMouseEvent('mousewheel', true, true, window,
                               0, Module['canvas'].offsetLeft + x, Module['canvas'].offsetTop + y, Module['canvas'].offsetLeft + x, Module['canvas'].offsetTop + y,
                               0, 0, 0, 0, 0, null);
                               event.wheelDelta = delta;
          } else {
            // mouse wheel event for Firefox, the delta sign is inversed for that browser and is stored in the detail property of the mouse event
            event.initMouseEvent('DOMMouseScroll', true, true, window,
                                 -delta, Module['canvas'].offsetLeft + x, Module['canvas'].offsetTop + y, Module['canvas'].offsetLeft + x, Module['canvas'].offsetTop + y,
                                 0, 0, 0, 0, 0, null);
          }
          Module['canvas'].dispatchEvent(event);
      }
  ));


  glutInit(&argc, argv);

  glutMouseFunc(&mouseCB);

  // inject wheel up event (delta > 0)
  emscripten_run_script("Module.injectWheelEvent(100, 100, 1)");
  if (wheel_up) {
    printf("%s\n", "mouse wheel up event received");
  }
  // inject wheel down event (delta < 0)
  emscripten_run_script("Module.injectWheelEvent(100, 100, -1)");
  if (wheel_down) {
    printf("%s\n", "mouse wheel down event received");
  }

  assert(wheel_up && wheel_down);

  return 0;
}
