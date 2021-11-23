/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "pthread_impl.h"
#include <pthread.h>

int _pthread_isduecanceled(struct pthread* pthread_ptr) {
  return pthread_ptr->cancel != 0;
}

void __pthread_testcancel() {
  struct pthread* self = pthread_self();
  if (self->canceldisable)
    return;
  if (_pthread_isduecanceled(self)) {
    pthread_exit(PTHREAD_CANCELED);
  }
}

weak_alias(__pthread_testcancel, pthread_testcancel);
