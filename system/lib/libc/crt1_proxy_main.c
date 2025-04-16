/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <pthread.h>
#include <stdlib.h>

#include <emscripten.h>
#include <emscripten/stack.h>
#include <emscripten/threading.h>
#include <emscripten/eventloop.h>

#include "threading_internal.h"

static int _main_argc;
static char** _main_argv;

int __main_argc_argv(int argc, char *argv[]);

weak int __main_void(void) {
  return __main_argc_argv(_main_argc, _main_argv);
}

static void* _main_thread(void* param) {
  // This is the main runtime thread for the application.
  emscripten_set_thread_name(pthread_self(), "Application main thread");
  // Will either call user's __main_void or weak version above.
  int rtn = __main_void();
  if (!emscripten_runtime_keepalive_check()) {
    exit(rtn);
  }
  return NULL;
}

EMSCRIPTEN_KEEPALIVE int _emscripten_proxy_main(int argc, char** argv) {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  // Use the size of the current stack, which is the normal size of the stack
  // that main() would have without PROXY_TO_PTHREAD.
  pthread_attr_setstacksize(&attr, emscripten_stack_get_base() - emscripten_stack_get_end());
  // Pass special ID -1 to the list of transferred canvases to denote that the
  // thread creation should instead take a list of canvases that are specified
  // from the command line with -sOFFSCREENCANVASES_TO_PTHREAD linker flag.
  emscripten_pthread_attr_settransferredcanvases(&attr, (const char*)-1);
  _main_argc = argc;
  _main_argv = argv;
  pthread_t thread;
  int rc = pthread_create(&thread, &attr, _main_thread, NULL);
  pthread_attr_destroy(&attr);
  if (rc == 0) {
    // Mark the thread as strongly referenced, so that Node.js doesn't exit
    // while the pthread is running.
    _emscripten_thread_set_strongref(thread);
  }
  return rc;
}
