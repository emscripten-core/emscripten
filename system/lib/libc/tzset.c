/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <limits.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

#include "emscripten_internal.h"

static char std_name[TZNAME_MAX+1];
static char dst_name[TZNAME_MAX+1];

weak void tzset() {
  static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  static _Atomic bool done_init = false;
  if (!done_init) {
    pthread_mutex_lock(&lock);
    if (!done_init) {
      _tzset_js(&timezone, &daylight, std_name, dst_name);
      tzname[0] = std_name;
      tzname[1] = dst_name;
      done_init = true;
    }
    pthread_mutex_unlock(&lock);
  }
}
