/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <GL/glut.h>

typedef struct {
  int width;
  int height;
} rect_size_t;

static rect_size_t browser_window_size = { 0, 0 };
static rect_size_t glut_init_size =      { 0, 0 };
static rect_size_t glut_reshape_size =   { 0, 0 };

void print_size_test(const char* name, rect_size_t rect_size) {
  static int test_count = 0;
  printf("Test %d: %s = %d x %d\n", ++test_count, name, rect_size.width, rect_size.height);
}

int equal_size(rect_size_t rect_1, rect_size_t rect_2) {
  return (rect_1.width == rect_2.width && rect_1.height == rect_2.height);
}

/**
 * Obtain various dimensions
 */
EM_JS(void, get_browser_window_size, (int* width, int* height), {
  setValue(width, window.innerWidth, 'i32');
  setValue(height, window.innerHeight, 'i32');
});

EM_JS(void, get_canvas_client_size, (int* width, int* height), {
  const canvas = Module.canvas;
  setValue(width, canvas.clientWidth, 'i32');
  setValue(height, canvas.clientHeight, 'i32');
});

EM_JS(void, get_canvas_size, (int* width, int* height), {
  const canvas = Module.canvas;
  setValue(width, canvas.width, 'i32');
  setValue(height, canvas.height, 'i32');
});

/**
 * Update canvas style with given width and height, then invoke window resize event
 */
EM_JS(void, test_resize_with_CSS, (const char* position, const char* width, const char* height), {
  const canvas = Module.canvas;
  canvas.style.position = UTF8ToString(position);
  canvas.style.width = UTF8ToString(width);
  canvas.style.height = UTF8ToString(height);
 
  window.dispatchEvent(new UIEvent('resize'));
});

/**
 * Verify canvas and reshape callback match target size, and also that
 * canvas width, height matches canvas clientWidth, clientHeight
 */
void assert_sizes_equal(rect_size_t target_size) {
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
 * Resizing tests
 */
void run_tests() {
  
  /* startup */
  print_size_test("startup, no CSS: canvas == glutReshapeFunc == glutInitWindow size", glut_init_size);
  assert_sizes_equal(glut_init_size);

  /* glutReshapeWindow */
  rect_size_t new_reshape_size = { glut_init_size.width + 40, glut_init_size.height + 20 };
  print_size_test("glut reshape, no CSS: canvas == glutReshapeFunc == glutReshapeWindow size", new_reshape_size);
  glutReshapeWindow(new_reshape_size.width, new_reshape_size.height);
  assert_sizes_equal(new_reshape_size);

  /* 100% scale CSS */
  print_size_test("100% window scale CSS: canvas == glutReshapeFunc == browser window size", browser_window_size);
  test_resize_with_CSS("fixed", "100%", "100%"); /* fixed, so canvas is driven by window size */
  assert_sizes_equal(browser_window_size);

  /* specific pixel size CSS */
  rect_size_t css_pixels_size = { glut_init_size.width - 20, glut_init_size.height + 40 };
  print_size_test("specific pixel size CSS: canvas == glutReshapeFunc == CSS specific size", css_pixels_size);
  char css_width[16], css_height[16];
  snprintf (css_width, 16, "%dpx", css_pixels_size.width);
  snprintf (css_height, 16, "%dpx", css_pixels_size.height);
  test_resize_with_CSS("static", css_width, css_height); /* static, canvas is driven by CSS size */
  assert_sizes_equal(css_pixels_size);

  /* mix of CSS scale and pixel size */
  rect_size_t css_mixed_size = { browser_window_size.width * 0.6, 100 };
  print_size_test("60% width, 100px height CSS: canvas == glutReshapeFunc == CSS mixed size", css_mixed_size);
  test_resize_with_CSS("fixed", "60%", "100px"); /* fixed, canvas width is driven by window size */
  assert_sizes_equal(css_mixed_size);

  /* run tests once */
  glutIdleFunc(NULL);  
  emscripten_force_exit(0);
}

/**
 * Reshape callback
 */
void reshape(int w, int h) {
  glut_reshape_size.width = w;
  glut_reshape_size.height = h;
}

int main(int argc, char *argv[]) {
  /* Make glut initial canvas size be 1/2 of browser window */
  get_browser_window_size(&browser_window_size.width, &browser_window_size.height);
  glut_init_size.width = browser_window_size.width / 2;
  glut_init_size.height = browser_window_size.height / 2;
  
  glutInit(&argc, argv);
  glutInitWindowSize(glut_init_size.width, glut_init_size.height);
  glutInitDisplayMode(GLUT_RGB);
  glutCreateWindow("test_glut_resize.c");

  /* Set up glut callback functions */
  glutIdleFunc(run_tests);
  glutReshapeFunc(reshape);
  glutDisplayFunc(NULL);

  glutMainLoop();
  return 0;
}