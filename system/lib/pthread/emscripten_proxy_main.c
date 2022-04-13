/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <pthread.h>

#include <emscripten.h>
#include <emscripten/stack.h>
#include <emscripten/threading.h>

static int _main_argc;
static char** _main_argv;

extern void __call_main(int argc, char** argv);

static void* _main_thread(void* param) {
  // This is the main runtime thread for the application.
  emscripten_set_thread_name(pthread_self(), "Application main thread");
  __call_main(_main_argc, _main_argv);
  return NULL;
}

int emscripten_proxy_main(int argc, char** argv) {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  // Use the size of the current stack, which is the normal size of the stack
  // that main() would have without PROXY_TO_PTHREAD.
  pthread_attr_setstacksize(&attr, emscripten_stack_get_base() - emscripten_stack_get_end());
  // Pass special ID -1 to the list of transferred canvases to denote that the thread creation
  // should instead take a list of canvases that are specified from the command line with
  // -sOFFSCREENCANVASES_TO_PTHREAD linker flag.
  emscripten_pthread_attr_settransferredcanvases(&attr, (const char*)-1);
  _main_argc = argc;
  _main_argv = argv;
  pthread_t thread;
  int rc = pthread_create(&thread, &attr, _main_thread, NULL);
  pthread_attr_destroy(&attr);
  return rc;
}
