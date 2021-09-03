/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "pthread_impl.h"
#include <pthread.h>

extern int __pthread_join_js(pthread_t thread, void **retval);
int __pthread_join(pthread_t thread, void **retval) {
  return __pthread_join_js(thread, retval);
}
weak_alias(__pthread_join, emscripten_builtin_pthread_join);
weak_alias(__pthread_join, pthread_join);
