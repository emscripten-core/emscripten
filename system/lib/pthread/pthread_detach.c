/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "pthread_impl.h"
#include <pthread.h>

extern int __pthread_detach_js(pthread_t t);

int __pthread_detach(pthread_t t) {
  return __pthread_detach_js(t);
}

weak_alias(__pthread_detach, emscripten_builtin_pthread_detach);
weak_alias(__pthread_detach, pthread_detach);
weak_alias(__pthread_detach, thrd_detach);
