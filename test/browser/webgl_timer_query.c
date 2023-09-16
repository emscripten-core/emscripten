// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#define GL_GLEXT_PROTOTYPES
#ifdef TEST_WEBGL2
#include <GLES3/gl3.h>
#include <GLES3/gl2ext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

int result = 0;
GLuint timerQuery = 0;

#define GL_CALL(x)                                                             \
  {                                                                            \
    x;                                                                         \
    GLenum error = glGetError();                                               \
    if (error != GL_NO_ERROR) {                                                \
      printf("GL ERROR: %d,  %s\n", (int)error, #x);                           \
      result = 1;                                                              \
    }                                                                          \
  }

void getQueryResult() {
  /* Get the result. It should be nonzero. */
  GLuint64 time = 0;
#ifdef TEST_WEBGL2
  GL_CALL(glGetQueryObjectui64vEXT(timerQuery, GL_QUERY_RESULT, &time));
#else
  GL_CALL(glGetQueryObjectui64vEXT(timerQuery, GL_QUERY_RESULT_EXT, &time));
#endif

  if (!time) return;

  printf("queried time: %llu\n", time);
  emscripten_cancel_main_loop();

#ifdef TEST_WEBGL2
  GL_CALL(glDeleteQueries(1, &timerQuery));
#else
  GL_CALL(glDeleteQueriesEXT(1, &timerQuery));
#endif

  exit(result);
}

int main() {
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);

#ifdef TEST_WEBGL2
  printf("testing for WebGL 2\n");
#else
  printf("testing for WebGL 1\n");
#endif

  attrs.enableExtensionsByDefault = 1;
#ifdef TEST_WEBGL2
  attrs.majorVersion = 2;
#else
  attrs.majorVersion = 1;
#endif
  attrs.minorVersion = 0;

  /* Skip WebGL 2 tests if not supported */
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context( "#canvas", &attrs );
  #ifdef TEST_WEBGL2
  if (!context) {
    printf("Skipped: WebGL 2 is not supported.\n");
    return 0;
  }
  #endif
  emscripten_webgl_make_context_current(context);

  /* Check if the extension is actually supported. Firefox reports
     EXT_disjoint_timer_query on WebGL 2 as well. */
  if (!strstr(glGetString(GL_EXTENSIONS), "EXT_disjoint_timer_query")) {
    printf("EXT_disjoint_timer_query[_webgl2] not supported\n");
    return 0;
  }

  /* Generate a timer query */
#ifdef TEST_WEBGL2
  GL_CALL(glGenQueries(2, &timerQuery));
#else
  GL_CALL(glGenQueriesEXT(1, &timerQuery));
#endif
  assert(timerQuery);

  /* Test the glGetQuery function */
  GLint bits = 0;
#ifdef TEST_WEBGL2
  GL_CALL(glGetQueryiv(GL_TIME_ELAPSED_EXT, GL_QUERY_COUNTER_BITS_EXT, &bits));
#else
  GL_CALL(glGetQueryivEXT(GL_TIME_ELAPSED_EXT, GL_QUERY_COUNTER_BITS_EXT, &bits));
#endif
  printf("time query bits: %d\n", bits);
  assert(bits);

  /* Begin it */
#ifdef TEST_WEBGL2
  GL_CALL(glBeginQuery(GL_TIME_ELAPSED_EXT, timerQuery));
#else
  GL_CALL(glBeginQueryEXT(GL_TIME_ELAPSED_EXT, timerQuery));
#endif

  /* Now, `timerQuery` should be a query */
  bool isQuery = false;
#ifdef TEST_WEBGL2
  GL_CALL(isQuery = glIsQuery(timerQuery));
#else
  GL_CALL(isQuery = glIsQueryEXT(timerQuery));
#endif
  assert(isQuery);

  /* Do a clear to actually make the GPU busy for a bit */
  glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  /* End the query after the clear */
#ifdef TEST_WEBGL2
  GL_CALL(glEndQuery(GL_TIME_ELAPSED_EXT));
#else
  GL_CALL(glEndQueryEXT(GL_TIME_ELAPSED_EXT));
#endif

  /* In WebGL, query results are available only in the next event loop
     iteration, so this should return false */
  GLuint isAvailable;
#ifdef TEST_WEBGL2
  GL_CALL(glGetQueryObjectuiv(timerQuery, GL_QUERY_RESULT_AVAILABLE, &isAvailable));
#else
  GL_CALL(glGetQueryObjectuivEXT(timerQuery, GL_QUERY_RESULT_AVAILABLE_EXT, &isAvailable));
#endif
  assert(!isAvailable);

  /* Run the main loop to get the result */
  emscripten_set_main_loop(getQueryResult, 0, 0);

  return 0;
}
