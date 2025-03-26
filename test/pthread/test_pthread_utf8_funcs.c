// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>
#include <string.h>

char stringBuffer[1024];

#define TEST_STRING "something long so that it hits the TextDecoder path"

EM_JS_DEPS(deps, "$stringToUTF8");

static void *thread1_start(void *arg) {
  EM_ASM({
   var mystr = UTF8ToString($0);
   stringToUTF8(mystr, $1, 1024);
  }, TEST_STRING, stringBuffer);
  return NULL;
}

int main() {
  pthread_t thread;
  pthread_create(&thread, NULL, thread1_start, 0);
  pthread_join(thread, 0);

  if (strcmp(TEST_STRING, stringBuffer) != 0) {
    return 1;
  }

  return 0;
}
