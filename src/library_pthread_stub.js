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

  pthread_mutexattr_init: function() {},
  pthread_mutexattr_setschedparam: function() {},
  pthread_mutexattr_setprotocol: function() {},
  pthread_mutexattr_settype: function() {},
  pthread_mutexattr_destroy: function() {},

  pthread_mutexattr_setpshared: function(attr, pshared) {
    // XXX implement if/when getpshared is required
    return 0;
  },

  pthread_cond_init: function() { return 0; },
  pthread_cond_destroy: function() { return 0; },
  pthread_cond_timedwait: function() { return 0; },

  pthread_condattr_init: function() { return 0; },
  pthread_condattr_destroy: function() { return 0; },
  pthread_condattr_setclock: function() { return 0; },
  pthread_condattr_setpshared: function() { return 0; },
  pthread_condattr_getclock: function() { return 0; },
  pthread_condattr_getpshared: function() { return 0; },

  pthread_attr_init: function(attr) {
    /* int pthread_attr_init(pthread_attr_t *attr); */
    //FIXME: should allocate a pthread_attr_t
    return 0;
  },
  pthread_getattr_np: function(thread, attr) {
    /* int pthread_getattr_np(pthread_t thread, pthread_attr_t *attr); */
    //FIXME: should fill in attributes of the given thread in pthread_attr_t
    return 0;
  },
  pthread_attr_destroy: function(attr) {
    /* int pthread_attr_destroy(pthread_attr_t *attr); */
    //FIXME: should destroy the pthread_attr_t struct
    return 0;
  },
  pthread_attr_getstack: function(attr, stackaddr, stacksize) {
    /* int pthread_attr_getstack(const pthread_attr_t *restrict attr,
       void **restrict stackaddr, size_t *restrict stacksize); */
    /*FIXME: assumes that there is only one thread, and that attr is the
      current thread*/
    {{{ makeSetValue('stackaddr', '0', '_emscripten_stack_get_base()', 'i8*') }}};
    {{{ makeSetValue('stacksize', '0', TOTAL_STACK, 'i32') }}};
    return 0;
  },
  pthread_attr_getdetachstate: function(attr, detachstate) {
    /* int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate); */
    return 0;
  },

  pthread_setcancelstate: function() { return 0; },
  pthread_setcanceltype: function() { return 0; },

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

  // pthread_sigmask - examine and change mask of blocked signals
  pthread_sigmask: function(how, set, oldset) {
    err('pthread_sigmask() is not supported: this is a no-op.');
    return 0;
  },

  pthread_rwlock_init: function() { return 0; },
  pthread_rwlock_destroy: function() { return 0; },
  pthread_rwlock_rdlock: function() { return 0; },
  pthread_rwlock_tryrdlock: function() { return 0; },
  pthread_rwlock_timedrdlock: function() { return 0; },
  pthread_rwlock_wrlock: function() { return 0; },
  pthread_rwlock_trywrlock: function() { return 0; },
  pthread_rwlock_timedwrlock: function() { return 0; },
  pthread_rwlock_unlock: function() { return 0; },

  pthread_rwlockattr_init: function() { return 0; },
  pthread_rwlockattr_destroy: function() { return 0; },
  pthread_rwlockattr_setpshared: function() { return 0; },
  pthread_rwlockattr_getpshared: function() { return 0; },

  pthread_spin_init: function() { return 0; },
  pthread_spin_destroy: function() { return 0; },
  pthread_spin_lock: function() { return 0; },
  pthread_spin_trylock: function() { return 0; },
  pthread_spin_unlock: function() { return 0; },

  pthread_attr_setdetachstate: function() {},
  pthread_attr_setschedparam: function() {},
  pthread_attr_setstacksize: function() {},

  {{{ USE_LSAN || USE_ASAN ? 'emscripten_builtin_' : '' }}}pthread_create: function() {
    return {{{ cDefine('EAGAIN') }}};
  },
  pthread_cancel: function() {},
  pthread_exit__deps: ['exit'],
  pthread_exit: function(status) {
    _exit(status);
  },

  pthread_equal: function(x, y) { return x == y },
  {{{ USE_LSAN ? 'emscripten_builtin_' : '' }}}pthread_join: function() {},
  pthread_detach: function() {},

  sem_init: function() {},
  sem_post: function() {},
  sem_wait: function() {},
  sem_trywait: function() {},
  sem_destroy: function() {},

  emscripten_main_browser_thread_id__deps: ['pthread_self'],
  emscripten_main_browser_thread_id: function() { return _pthread_self(); },

  // When pthreads is not enabled, we can't use the Atomics futex api to do proper sleeps, so simulate a busy spin wait loop instead.
  usleep__deps: ['emscripten_get_now'],
  usleep: function(useconds) {
    // int usleep(useconds_t useconds);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/usleep.html
    // We're single-threaded, so use a busy loop. Super-ugly.
    var start = _emscripten_get_now();
    while (_emscripten_get_now() - start < useconds / 1000) {
      // Do nothing.
    }
  },

  nanosleep__deps: ['usleep', '$setErrNo'],
  nanosleep: function(rqtp, rmtp) {
    // int nanosleep(const struct timespec  *rqtp, struct timespec *rmtp);
    if (rqtp === 0) {
      setErrNo({{{ cDefine('EINVAL') }}});
      return -1;
    }
    var seconds = {{{ makeGetValue('rqtp', C_STRUCTS.timespec.tv_sec, 'i32') }}};
    var nanoseconds = {{{ makeGetValue('rqtp', C_STRUCTS.timespec.tv_nsec, 'i32') }}};
    if (nanoseconds < 0 || nanoseconds > 999999999 || seconds < 0) {
      setErrNo({{{ cDefine('EINVAL') }}});
      return -1;
    }
    if (rmtp !== 0) {
      {{{ makeSetValue('rmtp', C_STRUCTS.timespec.tv_sec, '0', 'i32') }}};
      {{{ makeSetValue('rmtp', C_STRUCTS.timespec.tv_nsec, '0', 'i32') }}};
    }
    return _usleep((seconds * 1e6) + (nanoseconds / 1000));
  },

  __wait: function() {},
};

mergeInto(LibraryManager.library, LibraryPThreadStub);
