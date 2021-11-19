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

int touch_started_button = -1;
int touch_started_x = -1;
int touch_started_y = -1;

int touch_ended_button = -1;
int touch_ended_x = -1;
int touch_ended_y = -1;

void mouseCB(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON) {
    if(state == GLUT_DOWN) {
      printf("GLUT_DOWN: button: %d, x: %d, y: %d\n", button, x, y);
      touch_started_button = button;
      touch_started_x = x;
      touch_started_y = y;
    }
    else if(state == GLUT_UP) {
      printf("GLUT_UP: button: %d, x: %d, y: %d\n", button, x, y);
      touch_ended_button = button;
      touch_ended_x = x;
      touch_ended_y = y;
    }
  }
}

#define abs(x) ((x) < 0 ? -(x) : (x))

int main(int argc, char *argv[])
{
  emscripten_run_script(MULTILINE(
      Module.injectEvent = function(eventType, wantedX, wantedY) {
        // Desktop browsers do not have the event types for touch events,
        // so we fake them by creating a plain-vanilla UIEvent and then
        // filling in the fields that we look for with appropriate values.
        var rect = Module["canvas"].getBoundingClientRect();
        out('rect corner: ' + rect.left + ',' + rect.top);
        out('wanted: ' + wantedX + ',' + wantedY);
        var x = wantedX + rect.left;
        var y = wantedY + rect.top;
        var touch = {
            identifier: 0,
            clientX: x,
            clientY: y,
            screenX: x,
            screenY: y,
            pageX: x,
            pageY: y,
            target: Module['canvas']
        };
        var touches = [ touch ];
        touches.item = function(i) { return this[i]; };

        var event = document.createEvent('UIEvent');
        event.target = Module['canvas'];
        event.button = 0;
        event.changedTouches = touches;
        event.initUIEvent(eventType, true, true, window, 1);
        Module['canvas'].dispatchEvent(event);
      }
  ));

  // Fake a touch device so that glut sets up the appropriate event handlers.
  emscripten_run_script("document.documentElement['ontouchstart'] = 1");
  glutInit(&argc, argv);

  glutMouseFunc(&mouseCB);

  emscripten_run_script("Module.injectEvent('touchstart', 101, 112)");
  emscripten_run_script("Module.injectEvent('touchend', 201, 212)");
  printf("touchstarted: button:%d x:%d y:%d\n", touch_started_button, touch_started_x, touch_started_y);
  printf("touchended:   button:%d x:%d y:%d\n", touch_ended_button, touch_ended_x, touch_ended_y);
  assert(touch_started_button == 0);
  assert(abs(touch_started_x - 101) <= 1);
  assert(abs(touch_started_y - 112) <= 1);
  assert(touch_ended_button == 0);
  assert(abs(touch_ended_x - 201) <= 1);
  assert(abs(touch_ended_y - 212) <= 1);
  return 0;
}
