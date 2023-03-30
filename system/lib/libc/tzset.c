/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

#include "emscripten_internal.h"

weak void tzset() {
  static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  static _Atomic bool done_init = false;
  if (!done_init) {
    pthread_mutex_lock(&lock);
    if (!done_init) {
      _tzset_js(&timezone, &daylight, tzname);
      done_init = true;
    }
    pthread_mutex_unlock(&lock);
  }
}
