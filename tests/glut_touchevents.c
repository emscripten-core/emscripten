#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <EGL/egl.h>
#include <emscripten.h>

#define MULTILINE(...) #__VA_ARGS__

int touch_started = 0;
int touch_ended = 0;

int result = 0;

void mouseCB(int button, int state, int x, int y)
{
    if(button == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            touch_started = 1;
        }
        else if(state == GLUT_UP)
        {
            touch_ended = 1;
        }
    }
}

int main(int argc, char *argv[])
{
    emscripten_run_script(MULTILINE(
        Module.injectEvent = function(eventType, x, y) {
            // Desktop browsers do not have the event types for touch events,
            // so we fake them by creating a plain-vanilla UIEvent and then
            // filling in the fields that we look for with appropriate values.
            var touch = {
                pageX: x,
                pageY: y
            };
            var touches = [ touch ];
            touches.item = function(i) { return this[i]; };

            var event = document.createEvent('UIEvent');
            event.target = Module['canvas'];
            event.button = 0;
            event.touches = touches;
            event.initUIEvent(eventType, true, true, window, 1);
            Module['canvas'].dispatchEvent(event);
        }
    ));

    // Fake a touch device so that glut sets up the appropriate event handlers.
    emscripten_run_script("document.documentElement['ontouchstart'] = 1");
    glutInit(&argc, argv);

    glutMouseFunc(&mouseCB);

    emscripten_run_script("Module.injectEvent('touchend', 100, 100)");
    emscripten_run_script("Module.injectEvent('touchstart', 100, 100)");
    result = touch_started && touch_ended;

    REPORT_RESULT();
    return 0;
}
