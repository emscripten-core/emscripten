/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE
#include "pthread_impl.h"
#include <pthread.h>
#include <stdbool.h>
#include <threads.h>

// See musl's pthread_create.c

extern int __cxa_thread_atexit(void (*)(void *), void *, void *);
extern int __pthread_create_js(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
extern _Noreturn void __pthread_exit_js(void* status);
extern int8_t __dso_handle;

void __run_cleanup_handlers(void* _unused) {
  pthread_t self = __pthread_self();
  while (self->cancelbuf) {
    void (*f)(void *) = self->cancelbuf->__f;
    void *x = self->cancelbuf->__x;
    self->cancelbuf = self->cancelbuf->__next;
    f(x);
  }
}

void __do_cleanup_push(struct __ptcb *cb) {
  struct pthread *self = __pthread_self();
  cb->__next = self->cancelbuf;
  self->cancelbuf = cb;
  static thread_local bool registered = false;
  if (!registered) {
    __cxa_thread_atexit(__run_cleanup_handlers, NULL, &__dso_handle);
    registered = true;
  }
}

void __do_cleanup_pop(struct __ptcb *cb) {
  __pthread_self()->cancelbuf = cb->__next;
}

int __pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) {
  return __pthread_create_js(thread, attr, start_routine, arg);
}

_Noreturn void __pthread_exit(void* status) {
   __pthread_exit_js(status);
}

weak_alias(__pthread_create, emscripten_builtin_pthread_create);
weak_alias(__pthread_create, pthread_create);
weak_alias(__pthread_exit, pthread_exit);
