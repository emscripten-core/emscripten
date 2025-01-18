/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>
#include <string.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#include <math.h>

void report_result(int result) {
  if (result == 0) {
    printf("Test successful!\n");
  } else {
    printf("Test failed!\n");
  }
#ifdef REPORT_RESULT
  REPORT_RESULT(result);
#endif
}

static inline const char *emscripten_event_type_to_string(int eventType) {
  const char *events[] = { "(invalid)", "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick", "mousemove", "wheel", "resize",
    "scroll", "blur", "focus", "focusin", "focusout", "deviceorientation", "devicemotion", "orientationchange", "fullscreenchange", "pointerlockchange",
    "visibilitychange", "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected", "beforeunload",
    "batterychargingchange", "batterylevelchange", "webglcontextlost", "webglcontextrestored", "(invalid)" };
  ++eventType;
  if (eventType < 0) eventType = 0;
  if (eventType >= sizeof(events)/sizeof(events[0])) eventType = sizeof(events)/sizeof(events[0])-1;
  return events[eventType];
}

const char *emscripten_result_to_string(EMSCRIPTEN_RESULT result) {
  if (result == EMSCRIPTEN_RESULT_SUCCESS) return "EMSCRIPTEN_RESULT_SUCCESS";
  if (result == EMSCRIPTEN_RESULT_DEFERRED) return "EMSCRIPTEN_RESULT_DEFERRED";
  if (result == EMSCRIPTEN_RESULT_NOT_SUPPORTED) return "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
  if (result == EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED) return "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
  if (result == EMSCRIPTEN_RESULT_INVALID_TARGET) return "EMSCRIPTEN_RESULT_INVALID_TARGET";
  if (result == EMSCRIPTEN_RESULT_UNKNOWN_TARGET) return "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
  if (result == EMSCRIPTEN_RESULT_INVALID_PARAM) return "EMSCRIPTEN_RESULT_INVALID_PARAM";
  if (result == EMSCRIPTEN_RESULT_FAILED) return "EMSCRIPTEN_RESULT_FAILED";
  if (result == EMSCRIPTEN_RESULT_NO_DATA) return "EMSCRIPTEN_RESULT_NO_DATA";
  return "Unknown EMSCRIPTEN_RESULT!";
}

#define TEST_RESULT(x) if (ret != EMSCRIPTEN_RESULT_SUCCESS) printf("%s returned %s.\n", #x, emscripten_result_to_string(ret));

// The event handler functions can return 1 to suppress the event and disable the default action. That calls event.preventDefault();
// Returning 0 signals that the event was not consumed by the code, and will allow the event to pass on and bubble up normally.
bool key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
  if (eventType == EMSCRIPTEN_EVENT_KEYPRESS && (!strcmp(e->key, "f") || e->which == 102)) {
    EmscriptenFullscreenChangeEvent fsce;
    EMSCRIPTEN_RESULT ret = emscripten_get_fullscreen_status(&fsce);
    TEST_RESULT(emscripten_get_fullscreen_status);
    if (!fsce.isFullscreen) {
      printf("Requesting fullscreen..\n");
      ret = emscripten_request_fullscreen("#canvas", 1);
      TEST_RESULT(emscripten_request_fullscreen);
    } else {
      printf("Exiting fullscreen..\n");
      ret = emscripten_exit_fullscreen();
      TEST_RESULT(emscripten_exit_fullscreen);
      ret = emscripten_get_fullscreen_status(&fsce);
      TEST_RESULT(emscripten_get_fullscreen_status);
      if (fsce.isFullscreen) {
        fprintf(stderr, "Fullscreen exit did not work!\n");
      }
    }
  }
  else if (eventType == EMSCRIPTEN_EVENT_KEYPRESS && (!strcmp(e->key, "Esc") || !strcmp(e->key, "Escape") || e->which == 27)) {
    emscripten_exit_soft_fullscreen();
  }
  return 0;
}

int callCount = 0;

bool fullscreenchange_callback(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData) {
  printf("%s, isFullscreen: %d, fullscreenEnabled: %d, fs element nodeName: \"%s\", fs element id: \"%s\". New size: %dx%d pixels. Screen size: %dx%d pixels.\n",
    emscripten_event_type_to_string(eventType), e->isFullscreen, e->fullscreenEnabled, e->nodeName, e->id, e->elementWidth, e->elementHeight, e->screenWidth, e->screenHeight);

  ++callCount;
  if (callCount == 1) { // Transitioned to fullscreen.
    if (!e->isFullscreen) {
      report_result(1);
    }
  } else if (callCount == 2) { // Transitioned to windowed, we must be back to the default pixel size 300x150.
    if (e->isFullscreen || e->elementWidth != 300 || e->elementHeight != 150) {
      report_result(1);
    } else {
      report_result(0);
    }
  }
  return 0;
}

bool mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
  return 0;
}

GLuint program;

void draw() {
  int w, h;
  emscripten_get_canvas_element_size("#canvas", &w, &h);
  float t = emscripten_get_now() / 1000.0f;
  float xs = (float)h / w;
  float ys = 1.0f;
  float mat[] = { cosf(t) * xs, sinf(t) * ys, 0, 0, -sinf(t) * xs, cosf(t) * ys, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
  glUniformMatrix4fv(glGetUniformLocation(program, "mat"), 1, 0, mat);
  glClearColor(0,0,1,1);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

bool on_canvassize_changed(int eventType, const void *reserved, void *userData) {
  int w, h;
  emscripten_get_canvas_element_size("#canvas", &w, &h);
  double cssW, cssH;
  emscripten_get_element_css_size(0, &cssW, &cssH);
  printf("Canvas resized: WebGL RTT size: %dx%d, canvas CSS size: %02gx%02g\n", w, h, cssW, cssH);
  return 0;
}

void requestFullscreen(int scaleMode, int canvasResolutionScaleMode, int filteringMode) {
  EmscriptenFullscreenStrategy s;
  memset(&s, 0, sizeof(s));
  s.scaleMode = scaleMode;
  s.canvasResolutionScaleMode = canvasResolutionScaleMode;
  s.filteringMode = filteringMode;
  s.canvasResizedCallback = on_canvassize_changed;
  EMSCRIPTEN_RESULT ret = emscripten_request_fullscreen_strategy(0, 1, &s);
  TEST_RESULT(requestFullscreen);
}

void enterSoftFullscreen(int scaleMode, int canvasResolutionScaleMode, int filteringMode) {
  EmscriptenFullscreenStrategy s;
  memset(&s, 0, sizeof(s));
  s.scaleMode = scaleMode;
  s.canvasResolutionScaleMode = canvasResolutionScaleMode;
  s.filteringMode = filteringMode;
  s.canvasResizedCallback = on_canvassize_changed;
  EMSCRIPTEN_RESULT ret = emscripten_enter_soft_fullscreen(0, &s);
  TEST_RESULT(enterSoftFullscreen);
}

int on_button_click(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
  switch((long)userData) {
    case 0: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 1: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 2: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 3: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 4: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 5: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 6: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 7: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_CENTER, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;

    case 8: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 9: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 10: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 11: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 12: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 13: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;
    case 14: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_CENTER, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); break;

    case 15: requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST); break;
    case 16: enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE, EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST); break;
    default: return 0;
  }
  return 1;
}

int main() {
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.alpha = attr.depth = attr.stencil = attr.antialias = attr.preserveDrawingBuffer = attr.failIfMajorPerformanceCaveat = 0;
  attr.enableExtensionsByDefault = 1;
  attr.premultipliedAlpha = 0;
  attr.majorVersion = 1;
  attr.minorVersion = 0;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  const char *vss = "attribute vec4 vPosition; uniform mat4 mat; void main() { gl_Position = mat * vPosition; }";
  glShaderSource(vs, 1, &vss, 0);
  glCompileShader(vs);
  GLuint ps = glCreateShader(GL_FRAGMENT_SHADER);
  const char *pss = "precision lowp float; uniform vec3 colors[3]; void main() { gl_FragColor = vec4(1,0,0,1); }";
  glShaderSource(ps, 1, &pss, 0);
  glCompileShader(ps);
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, ps);
  glBindAttribLocation(program, 0, "vPosition");
  glLinkProgram(program);
  glUseProgram(program);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  float verts[] = { 0.0, 0.5, 0.0, -0.5, -0.5, 0.0, 0.5, -0.5, 0.0 };
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
  glEnableVertexAttribArray(0);

  EMSCRIPTEN_RESULT ret = emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
  TEST_RESULT(emscripten_set_keypress_callback);

  ret = emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, 0, 1, fullscreenchange_callback);
  TEST_RESULT(emscripten_set_fullscreenchange_callback);

  // For Internet Explorer, fullscreen and pointer lock requests cannot be run
  // from inside keyboard event handlers. Therefore we must register a callback to
  // mouse events (any other than mousedown) to activate deferred fullscreen/pointerlock
  // requests to occur for IE. The callback itself can be a no-op.
  ret = emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_click_callback);
  ret = emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_mousedown_callback);
  ret = emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_mouseup_callback);
  ret = emscripten_set_dblclick_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback);
  TEST_RESULT(emscripten_set_dblclick_callback);

  emscripten_set_click_callback("#b0", (void*)0, 1, on_button_click);
  emscripten_set_click_callback("#b1", (void*)1, 1, on_button_click);
  emscripten_set_click_callback("#b2", (void*)2, 1, on_button_click);
  emscripten_set_click_callback("#b3", (void*)3, 1, on_button_click);
  emscripten_set_click_callback("#b4", (void*)4, 1, on_button_click);
  emscripten_set_click_callback("#b5", (void*)5, 1, on_button_click);
  emscripten_set_click_callback("#b6", (void*)6, 1, on_button_click);
  emscripten_set_click_callback("#b7", (void*)7, 1, on_button_click);
  emscripten_set_click_callback("#b8", (void*)8, 1, on_button_click);
  emscripten_set_click_callback("#b9", (void*)9, 1, on_button_click);
  emscripten_set_click_callback("#b10", (void*)10, 1, on_button_click);
  emscripten_set_click_callback("#b11", (void*)11, 1, on_button_click);
  emscripten_set_click_callback("#b12", (void*)12, 1, on_button_click);
  emscripten_set_click_callback("#b13", (void*)13, 1, on_button_click);
  emscripten_set_click_callback("#b14", (void*)14, 1, on_button_click);
  emscripten_set_click_callback("#b15", (void*)15, 1, on_button_click);
  emscripten_set_click_callback("#b16", (void*)16, 1, on_button_click);

  printf("To finish this test, press f to enter fullscreen mode, and then exit it.\n");
  printf("On IE, press a mouse key over the canvas after pressing f to activate the fullscreen request event.\n");

  emscripten_set_main_loop(draw, 0, 0);
  return 0;
}
