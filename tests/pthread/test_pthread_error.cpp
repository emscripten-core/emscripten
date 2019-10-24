// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <pthread.h>
#include <emscripten.h>
#include <assert.h>

EM_JS(void*, ThreadMain, (void *arg), {
  throw "pthread FAIL";
})

EMSCRIPTEN_KEEPALIVE
extern "C" void pthread_error(const char* c) {
  printf("pthread reported an error: %s\n", c);
#ifdef REPORT_RESULT
	REPORT_RESULT(1);
#endif
}

int main() {
  if (!emscripten_has_threading_support()) {
#ifdef REPORT_RESULT
    REPORT_RESULT(1);
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

	pthread_t thread;
	int rc = pthread_create(&thread, NULL, ThreadMain, 0);
	assert(rc == 0);

	rc = pthread_join(thread, NULL);
	assert(rc == 0);
}
