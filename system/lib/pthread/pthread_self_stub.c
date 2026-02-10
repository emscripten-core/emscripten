/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "pthread_impl.h"
#include <unistd.h>
#include <emscripten/stack.h>

static struct pthread __main_pthread;

uintptr_t __get_tp(void) {
  return (uintptr_t)&__main_pthread;
}

// In case the stub syscall is not linked it
static int dummy_getpid() {
  return 42;
}
weak_alias(dummy_getpid, __syscall_getpid);

pthread_t emscripten_main_runtime_thread_id() {
  return &__main_pthread;
}

extern int __stack_high;
extern int __stack_low;

__attribute__((constructor))
static void init_pthread_self(void) {
  __main_pthread.locale = &libc.global_locale;
  __main_pthread.tid = getpid();
  __main_pthread.stack = &__stack_high;
  __main_pthread.stack_size = ((size_t)&__stack_high) - ((size_t)&__stack_low);
  __main_pthread.guard_size = __default_guardsize;
}
