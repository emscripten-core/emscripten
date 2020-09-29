// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <emscripten.h>

static pthread_t thread;
static int done = 0;
static int loops = 0;

void *ThreadMain(void *arg) {
  printf("thread\n");
  done = 1;
  pthread_exit(0);
}

void mainloop() {
  // bail out after 100 loops since the thread might've aborted (if it failed the test)
  loops += 1;  
  if(done || loops > 100) {
    emscripten_cancel_main_loop();
  }
}

int main() {
  printf("main\n");
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  int rc = pthread_create(&thread, &attr, ThreadMain, (void*)0);
  assert(rc == 0);
  pthread_attr_destroy(&attr);
  emscripten_set_main_loop(mainloop, 0, 0);
}
