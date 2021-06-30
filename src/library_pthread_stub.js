/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryPThreadStub = {
  // ===================================================================================
  // Stub implementation for pthread.h when not compiling with pthreads support enabled.
  // ===================================================================================

  emscripten_is_main_browser_thread: function() {
#if MINIMAL_RUNTIME
    return typeof importScripts === 'undefined';
#else
    return !ENVIRONMENT_IS_WORKER;
#endif
  },

  // When pthreads is not enabled, we can't use the Atomics futex api to do
  // proper sleeps, so simulate a busy spin wait loop instead.
  emscripten_thread_sleep__deps: ['emscripten_get_now'],
  emscripten_thread_sleep: function(msecs) {
    var start = _emscripten_get_now();
    while (_emscripten_get_now() - start < msecs) {
      // Do nothing.
    }
  },

  __cxa_thread_atexit: 'atexit',
  __cxa_thread_atexit_impl: 'atexit',
};

mergeInto(LibraryManager.library, LibraryPThreadStub);
