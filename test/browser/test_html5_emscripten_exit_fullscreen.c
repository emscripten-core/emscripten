#include <assert.h>
#include <stdio.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

bool is_fs = false;

bool fullscreen_change(int eventType, const EmscriptenFullscreenChangeEvent *fullscreenChangeEvent, void *userData) {
    is_fs = fullscreenChangeEvent->isFullscreen;

    printf("%s fullscreen\n", is_fs ? "entered" : "left");

    return 0;
}

static int resizes = 0;

bool canvas_resize(int eventType, const void *reserved, void *userData) {
    double css_w, css_h;
    emscripten_get_element_css_size("#canvas", &css_w, &css_h);

    printf("resized to %fx%f\n", css_w, css_h);

    resizes++;
    if (resizes == 2) {
        REPORT_RESULT(css_w == 800 && css_h == 600); // Back to the size before fullscreen
    }

    return 0;
}

bool key_down(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData) {
    if (keyEvent->keyCode == 0x46/*f*/) {
        if (is_fs) {
            emscripten_exit_fullscreen();
        } else {
            EmscriptenFullscreenStrategy strategy;
            strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
            strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
            strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
            strategy.canvasResizedCallback = canvas_resize;
            strategy.canvasResizedCallbackUserData = NULL;

            emscripten_request_fullscreen_strategy("#canvas", 1, &strategy);
        }
    }

    return 1;
}

void loop() {}

int main(int argc, char** argv) {
#ifdef EXIT_WITH_F
    puts("Click the canvas, then enter fullscreen using 'f', then exit by pressing 'f' again.");
#else
    puts("Click the canvas, then enter fullscreen using 'f', then exit by pressing the escape key.");
#endif

    emscripten_set_canvas_element_size("#canvas", 800, 600);

    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 0, fullscreen_change);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, key_down);
    emscripten_set_main_loop(loop, 0, 1);

    return 0;
}
