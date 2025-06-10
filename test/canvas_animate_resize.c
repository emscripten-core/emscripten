#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <GLES2/gl2.h>
#include <math.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/threading.h>
#include <bits/errno.h>
#include <stdlib.h>
#include <unistd.h>

//#define TEST_EXPLICIT_CONTEXT_SWAP
//  - If set, test WebGL context creation .explicitSwapControl=true and call emscripten_webgl_commit_frame() to swap. Otherwise utilize implicit swapping.
//#define TEST_EMSCRIPTEN_SET_MAIN_LOOP
//  - If set, test emscripten_set_main_loop(). If unset, run our own synchronously blocking loop
//#define TEST_MANUALLY_SET_ELEMENT_CSS_SIZE
//  - If set, manually update CSS pixel size of the canvas. If unset, leave the CSS size to auto and expect the browser to resize the canvas size.

#if !defined(TEST_EMSCRIPTEN_SET_MAIN_LOOP) && !defined(__EMSCRIPTEN_PTHREADS__)
#error TEST_EMSCRIPTEN_SET_MAIN_LOOP=false, but blocking main loops require using -sPROXY_TO_PTHREADS= and multithreading (build with -sPROXY_TO_PTHREAD and -pthread)
#endif

#if !defined(TEST_EMSCRIPTEN_SET_MAIN_LOOP) && !defined(TEST_EXPLICIT_CONTEXT_SWAP)
#error If using own main loop, must use explicit context swapping (explicitSwapControl=true) (build with either -DTEST_EMSCRIPTEN_SET_MAIN_LOOP=1 or -DTEST_EXPLICIT_CONTEXT_SWAP=1)
#endif

#define NUM_FRAMES_TO_RENDER 100

GLuint vb;
int program;
int frameNumber = 0;
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;

void tick() {
  double sizeScale = 21.0 + 20.0 * sin(emscripten_get_now()*0.001);
  double w = 18.0*sizeScale;
  double h = 11.0*sizeScale;

  EMSCRIPTEN_RESULT r;

#if TEST_MANUALLY_SET_ELEMENT_CSS_SIZE
  r = emscripten_set_element_css_size("#canvas", w / emscripten_get_device_pixel_ratio(), h / emscripten_get_device_pixel_ratio());
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
#endif

  r = emscripten_set_canvas_element_size("#canvas", (int)w, (int)h);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  int verifyWidth, verifyHeight;
  r = emscripten_get_canvas_element_size("#canvas", &verifyWidth, &verifyHeight);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
  assert(verifyWidth == (int)w);
  assert(verifyHeight == (int)h);

  glViewport(0, 0, (int)w, (int)h);
  glUseProgram(program);
  glBindBuffer(GL_ARRAY_BUFFER, vb);
  glDisable(GL_DEPTH_TEST);
  int posLoc = glGetAttribLocation(program, "pos");
  glVertexAttribPointer(posLoc, 2, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(posLoc);
  glUniform1f(glGetUniformLocation(program, "t"), emscripten_get_now()*0.0001f);
  glClearColor(0,1,0,1); // Clear to green color which should never be visible
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#if TEST_EXPLICIT_CONTEXT_SWAP
  r = emscripten_webgl_commit_frame();
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);
#endif

  ++frameNumber;
  if (frameNumber >= NUM_FRAMES_TO_RENDER) {
#if TEST_EMSCRIPTEN_SET_MAIN_LOOP
    emscripten_cancel_main_loop();
#endif
    emscripten_webgl_make_context_current(0);
    emscripten_webgl_destroy_context(ctx);
    printf("quit\n");
    exit(0);
  }
}

void init() {
  printf("init\n");
  glGenBuffers(1, &vb);
  glBindBuffer(GL_ARRAY_BUFFER, vb);
  float vertices[] = { -1, -1, -1,  1, 1, -1, 1,  1 };
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  const char *vsCode =
    "attribute vec2 pos;"
    "varying lowp vec2 tex;"
    "void main() { tex = pos * 0.5 + vec2(0.5,0.5); gl_Position = vec4(pos, 0.0, 1.0); }";
  int vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vsCode, NULL);
  glCompileShader(vs);

  const char *fsCode = 
    "varying lowp vec2 tex;"
    "uniform highp float t;"
    "uniform sampler2D sampler;"
    "void main() { highp float x = tex.x + sin((t + tex.y)*10.0)*0.005; highp float y = tex.y + sin((t + tex.x)*10.0)*0.02; x = x*1.3 - 0.1; y = y * 1.3 - 0.23;"
    "gl_FragColor = (tex.x > 0.05 && tex.x < 0.09) ? vec4(0,0,0,1) : ((x >= 0.0 && x <= 1.0 && y >= 0.0 && y <= 1.0) ? (((x >= 5.0/18.0 && x <= 8.0/18.0) || (x >= 0.0 && x <= 1.0 && y >= 4.0/11.0 && y <= 7.0/11.0)) ? vec4(0,0,1,1) : vec4(1,1,1,1)) : vec4(0.75,0.75,0.75,1)); }";
  int fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fsCode, NULL);
  glCompileShader(fs);
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
}

int main()
{
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
#if TEST_EXPLICIT_CONTEXT_SWAP
  attr.explicitSwapControl = true;
#endif
  ctx = emscripten_webgl_create_context("#canvas", &attr);
  printf("Created context with handle %#lx\n", ctx);
  if (!ctx) {
    if (!emscripten_supports_offscreencanvas()) {
      EM_ASM({
        fetch("http://localhost:8888/report_result?skipped:%20OffscreenCanvas%20is%20not%20supported!")
        .then(() => window.close());
      });
    }
    return 0;
  }
  emscripten_webgl_make_context_current(ctx);

  init();

  printf("You should see the flag of Finland.\n");
#if TEST_EMSCRIPTEN_SET_MAIN_LOOP
  emscripten_set_main_loop(tick, 0, 0);
#else
  for (int i = 0; i < NUM_FRAMES_TO_RENDER; ++i) {
    tick();
    emscripten_current_thread_process_queued_calls();
    usleep(16*1000);
  }
#endif
  return 99;
}
