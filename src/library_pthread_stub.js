var LibraryPThreadStub = {
  // ===================================================================================
  // Stub implementation for pthread.h when not compiling with pthreads support enabled.
  // ===================================================================================

  emscripten_has_threading_support: function() {
    return 0;
  },

  emscripten_num_logical_cores: function() {
    return 1;
  },

  emscripten_force_num_logical_cores: function(cores) {
    // Ignored, no threading available.
  },

  emscripten_is_main_runtime_thread: function() {
    return 1;
  },

  emscripten_is_main_browser_thread: function() {
    return !ENVIRONMENT_IS_WORKER;
  },

  emscripten_main_thread_process_queued_calls: function() {
    // We will never have any queued calls to process, so no-op.
  },

  pthread_mutex_init: function() {},
  pthread_mutex_destroy: function() {},
  pthread_mutexattr_init: function() {},
  pthread_mutexattr_settype: function() {},
  pthread_mutexattr_destroy: function() {},
  pthread_mutex_lock: function() {},
  pthread_mutex_unlock: function() {},
  pthread_mutex_trylock: function() {
    return 0;
  },
  pthread_mutexattr_setpshared: function(attr, pshared) {
    // XXX implement if/when getpshared is required
    return 0;
  },
  pthread_cond_init: function() {},
  pthread_cond_destroy: function() {},
  pthread_cond_broadcast: function() {
    return 0;
  },
  pthread_cond_wait: function() {
    return 0;
  },
  pthread_cond_timedwait: function() {
    return 0;
  },
  pthread_self: function() {
    //FIXME: assumes only a single thread
    return 0;
  },
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
    {{{ makeSetValue('stackaddr', '0', 'STACK_BASE', 'i8*') }}};
    {{{ makeSetValue('stacksize', '0', 'TOTAL_STACK', 'i32') }}};
    return 0;
  },

  pthread_once: function(ptr, func) {
    if (!_pthread_once.seen) _pthread_once.seen = {};
    if (ptr in _pthread_once.seen) return;
    Runtime.dynCall('v', func);
    _pthread_once.seen[ptr] = 1;
  },

  $PTHREAD_SPECIFIC: {},
  $PTHREAD_SPECIFIC_NEXT_KEY: 1,
  pthread_key_create__deps: ['$PTHREAD_SPECIFIC', '$PTHREAD_SPECIFIC_NEXT_KEY', '$ERRNO_CODES'],
  pthread_key_create: function(key, destructor) {
    if (key == 0) {
      return ERRNO_CODES.EINVAL;
    }
    {{{ makeSetValue('key', '0', 'PTHREAD_SPECIFIC_NEXT_KEY', 'i32*') }}};
    // values start at 0
    PTHREAD_SPECIFIC[PTHREAD_SPECIFIC_NEXT_KEY] = 0;
    PTHREAD_SPECIFIC_NEXT_KEY++;
    return 0;
  },

  pthread_getspecific__deps: ['$PTHREAD_SPECIFIC'],
  pthread_getspecific: function(key) {
    return PTHREAD_SPECIFIC[key] || 0;
  },

  pthread_setspecific__deps: ['$PTHREAD_SPECIFIC', '$ERRNO_CODES'],
  pthread_setspecific: function(key, value) {
    if (!(key in PTHREAD_SPECIFIC)) {
      return ERRNO_CODES.EINVAL;
    }
    PTHREAD_SPECIFIC[key] = value;
    return 0;
  },

  pthread_key_delete__deps: ['$PTHREAD_SPECIFIC', '$ERRNO_CODES'],
  pthread_key_delete: function(key) {
    if (key in PTHREAD_SPECIFIC) {
      delete PTHREAD_SPECIFIC[key];
      return 0;
    }
    return ERRNO_CODES.EINVAL;
  },

  pthread_cleanup_push: function(routine, arg) {
    __ATEXIT__.push(function() { Runtime.dynCall('vi', routine, [arg]) })
    _pthread_cleanup_push.level = __ATEXIT__.length;
  },

  pthread_cleanup_pop: function() {
    assert(_pthread_cleanup_push.level == __ATEXIT__.length, 'cannot pop if something else added meanwhile!');
    __ATEXIT__.pop();
    _pthread_cleanup_push.level = __ATEXIT__.length;
  },

  pthread_rwlock_init: function() {
    return 0; // XXX
  },

  pthread_attr_setdetachstate: function() {},

  pthread_create: function() {
    return {{{ cDefine('EAGAIN') }}};
  },
  pthread_exit: function() {},

  pthread_cond_signal: function() {},
  pthread_equal: function() {},
  pthread_join: function() {},
  pthread_detach: function() {},

  // When pthreads is not enabled, we can't use the Atomics futex api to do proper sleeps, so simulate a busy spin wait loop instead.
  usleep: function(useconds) {
    // int usleep(useconds_t useconds);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/usleep.html
    // We're single-threaded, so use a busy loop. Super-ugly.
    var msec = useconds / 1000;
    if ((ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) && self['performance'] && self['performance']['now']) {
      var start = self['performance']['now']();
      while (self['performance']['now']() - start < msec) {
        // Do nothing.
      }
    } else {
      var start = Date.now();
      while (Date.now() - start < msec) {
        // Do nothing.
      }
    }
    return 0;
  }
};

mergeInto(LibraryManager.library, LibraryPThreadStub);
