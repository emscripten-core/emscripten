/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if PTHREADS
#error "Internal error! PTHREADS should not be enabled when including library_pthread_stub.js."
#endif
#if STANDALONE_WASM && SHARED_MEMORY
#error "STANDALONE_WASM does not support shared memories yet"
#endif

var LibraryPThreadStub = {
  // ===================================================================================
  // Stub implementation for pthread.h when not compiling with pthreads support enabled.
  // ===================================================================================

  emscripten_is_main_browser_thread: () =>
#if MINIMAL_RUNTIME
    typeof importScripts == 'undefined'
#else
    !ENVIRONMENT_IS_WORKER
#endif
  ,
};

addToLibrary(LibraryPThreadStub);
