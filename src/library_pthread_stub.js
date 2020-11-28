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

  pthread_cleanup_push__sig: 'vii',
  pthread_cleanup_push: function(routine, arg) {
    __ATEXIT__.push({ func: routine, arg: arg });
    _pthread_cleanup_push.level = __ATEXIT__.length;
  },

  pthread_cleanup_pop__sig: 'vi',
  pthread_cleanup_pop: function(execute) {
    assert(_pthread_cleanup_push.level == __ATEXIT__.length, 'cannot pop if something else added meanwhile!');
    callback = __ATEXIT__.pop();
    if (execute) {
      {{{ makeDynCall('vi', 'callback.func') }}}(callback.arg)
    }
    _pthread_cleanup_push.level = __ATEXIT__.length;
  },

  {{{ USE_LSAN || USE_ASAN ? 'emscripten_builtin_' : '' }}}pthread_create: function() {
    return {{{ cDefine('EAGAIN') }}};
  },

  {{{ USE_LSAN ? 'emscripten_builtin_' : '' }}}pthread_join: function() {
    return {{{ cDefine('EINVAL') }}};
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
};

mergeInto(LibraryManager.library, LibraryPThreadStub);
