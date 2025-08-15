/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <emscripten/emscripten.h>
#include <GL/glut.h>

typedef struct {
  int32_t width;
  int32_t height;
} rect_size_t;

static rect_size_t browser_window_size = { 0, 0 };
static rect_size_t glut_init_size =      { 0, 0 };
static rect_size_t glut_reshape_size =   { 0, 0 };
static rect_size_t target_size =         { 0, 0 };

/*
 * Set run_async_verification to 0 for sync test cases, and 1 for async tests.
 *
 * Callback sequence for test case 1 & 2 (synchronous):
 * glutMainLoop -> GLUT.onSize -> Browser.setCanvasSize -> updateResizeListeners -> GLUT.reshapeFunc
 * glutResizeWindow -> Browser.setCanvasSize -> updateResizeListeners -> GLUT.reshapeFunc
 *
 * Callback sequence for test cases 3-5 (async):
 * window resize -> async update -> GLUT.onSize -> Browser.setCanvasSize -> updateResizeListeners -> GLUT.reshapeFunc
 *
 * Because window resize does not immediately call GLUT.onSize, we wait to run verification of a test until we get
 * confirmation in GLUT.reshapeFunc.  And after verification is done, we move on to the next test.
 *
 */
static int run_async_verification = 0;

void print_size_test(int test_num, const char* name, rect_size_t rect_size) {
  printf("Test %d: %s = %d x %d\n", test_num, name, rect_size.width, rect_size.height);
}

int equal_size(rect_size_t rect_1, rect_size_t rect_2) {
  return (rect_1.width == rect_2.width && rect_1.height == rect_2.height);
}

/**
 * Obtain various dimensions
 */
EM_JS(void, get_browser_window_size, (int32_t* width, int32_t* height), {
  setValue(Number(width), window.innerWidth, 'i32');
  setValue(Number(height), window.innerHeight, 'i32');
});

EM_JS(void, get_canvas_client_size, (int32_t* width, int32_t* height), {
  const canvas = Module.canvas;
  setValue(Number(width), canvas.clientWidth, 'i32');
  setValue(Number(height), canvas.clientHeight, 'i32');
});

EM_JS(void, get_canvas_size, (int32_t* width, int32_t* height), {
  const canvas = Module.canvas;
  setValue(Number(width), canvas.width, 'i32');
  setValue(Number(height), canvas.height, 'i32');
});

/**
 * Update canvas style with given width and height, then invoke window resize event
 */
EM_JS(void, test_resize_with_CSS, (const char* position, const char* width, const char* height), {
  const canvas = Module.canvas;
  canvas.style.position = UTF8ToString(Number(position));
  canvas.style.width = UTF8ToString(Number(width));
  canvas.style.height = UTF8ToString(Number(height));

  window.dispatchEvent(new UIEvent('resize'));
});

/**
 * Verify canvas and reshape callback match target size, and also that
 * canvas width, height matches canvas clientWidth, clientHeight
 */
void assert_canvas_and_target_sizes_equal() {
  /* verify target size match */
  rect_size_t canvas_size;
  get_canvas_size(&canvas_size.width, &canvas_size.height);
  assert(equal_size(canvas_size, target_size));
  assert(equal_size(glut_reshape_size, target_size));

  /* verify canvas client size match */
  rect_size_t canvas_client_size;
  get_canvas_client_size(&canvas_client_size.width, &canvas_client_size.height);
  assert(equal_size(canvas_size, canvas_client_size));
}

/**
 * Verify the result of the previous test and then run the next one
 */
void verify_test_and_run_next() {
  void run_next_test();

  assert_canvas_and_target_sizes_equal();
  run_next_test();
}

/**
 * Resizing tests
 */
void run_next_test() {
  static int test_num = 0;
  ++test_num;

  switch(test_num) {
    case 1: {
      /* startup */
      target_size = glut_init_size;
      print_size_test(test_num, "startup, no CSS: canvas == glutReshapeFunc == glutInitWindow size", target_size);
      verify_test_and_run_next();
      break;
    }
    case 2: {
      /* glutReshapeWindow */
      target_size.width = glut_init_size.width + 40;
      target_size.height = glut_init_size.height + 20;
      print_size_test(test_num, "glut reshape, no CSS: canvas == glutReshapeFunc == glutReshapeWindow size", target_size);
      glutReshapeWindow(target_size.width, target_size.height);
      verify_test_and_run_next();
      break;
    }
    case 3: {
      /* 100% scale CSS */
      target_size = browser_window_size;
      print_size_test(test_num, "100% window scale CSS: canvas == glutReshapeFunc == browser window size", target_size);
      run_async_verification = 1;
      test_resize_with_CSS("fixed", "100%", "100%"); /* fixed, so canvas is driven by window size */
      break;
    }
    case 4: {
      /* specific pixel size CSS */
      target_size.width = glut_init_size.width - 20;
      target_size.height = glut_init_size.height + 40;
      print_size_test(test_num, "specific pixel size CSS: canvas == glutReshapeFunc == CSS specific size", target_size);
      char css_width[16], css_height[16];
      snprintf (css_width, 16, "%dpx", target_size.width);
      snprintf (css_height, 16, "%dpx", target_size.height);
      run_async_verification = 1;
      test_resize_with_CSS("static", css_width, css_height); /* static, canvas is driven by CSS size */
      break;
    }
    case 5: {
      /* mix of CSS scale and pixel size */
      target_size.width = browser_window_size.width;
      target_size.height = 100;
      print_size_test(test_num, "100% width, 100px height CSS: canvas == glutReshapeFunc == CSS mixed size", target_size);
      run_async_verification = 1;
      test_resize_with_CSS("fixed", "100%", "100px"); /* fixed, canvas width is driven by window size */
      break;
    }
    default: {
      /* all tests complete */
      emscripten_force_exit(0);
      break;
    }
  }
}

/**
 * Idle callback - start tests
 */
void start_tests() {
  glutIdleFunc(NULL);
  run_next_test();
}

/**
 * Reshape callback - record latest size, verify and run next test if async
 */
void reshape(int w, int h) {
  glut_reshape_size.width = (int32_t)w;
  glut_reshape_size.height = (int32_t)h;

  if (run_async_verification) {
    run_async_verification = 0; /* Only one verification per test */
    verify_test_and_run_next();
  }
}

int main(int argc, char *argv[]) {
  get_browser_window_size(&browser_window_size.width, &browser_window_size.height);

  /* Make glut initial canvas size be 1/2 of browser window */
  glut_init_size.width = browser_window_size.width / 2;
  glut_init_size.height = browser_window_size.height / 2;

  glutInit(&argc, argv);
  glutInitWindowSize(glut_init_size.width, glut_init_size.height);
  glutInitDisplayMode(GLUT_RGB);
  glutCreateWindow("test_glut_resize.c");

  /* Set up glut callback functions */
  glutIdleFunc(start_tests);
  glutReshapeFunc(reshape);
  glutDisplayFunc(NULL);

  glutMainLoop();
  return 0;
}
