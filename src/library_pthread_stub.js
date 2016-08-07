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

  pthread_mutex_lock__asm: true,
  pthread_mutex_lock__sig: 'ii',
  pthread_mutex_lock: function(x) {
    x = x | 0;
    return 0;
  },
  pthread_mutex_unlock__asm: true,
  pthread_mutex_unlock__sig: 'ii',
  pthread_mutex_unlock: function(x) {
    x = x | 0;
    return 0;
  },
  pthread_mutex_trylock__asm: true,
  pthread_mutex_trylock__sig: 'ii',
  pthread_mutex_trylock: function(x) {
    x = x | 0;
    return 0;
  },

  pthread_mutexattr_setpshared: function(attr, pshared) {
    // XXX implement if/when getpshared is required
    return 0;
  },

  pthread_cond_init: function() { return 0; },
  pthread_cond_destroy: function() { return 0; },
  pthread_cond_wait: function() { return 0; },
  pthread_cond_timedwait: function() { return 0; },
  pthread_cond_signal: function() { return 0; },

  pthread_condattr_init: function() { return 0; },
  pthread_condattr_destroy: function() { return 0; },
  pthread_condattr_setclock: function() { return 0; },
  pthread_condattr_setpshared: function() { return 0; },
  pthread_condattr_getclock: function() { return 0; },
  pthread_condattr_getpshared: function() { return 0; },

  pthread_cond_broadcast__asm: true,
  pthread_cond_broadcast__sig: 'ii',
  pthread_cond_broadcast: function(x) {
    x = x | 0;
    return 0;
  },

  pthread_self__asm: true,
  pthread_self__sig: 'i',
  pthread_self: function() {
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

  pthread_create: function() {
    return {{{ cDefine('EAGAIN') }}};
  },
  pthread_exit: function() {},

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
  },

  llvm_memory_barrier: function(){},

  llvm_atomic_load_add_i32_p0i32: function(ptr, delta) {
    var ret = {{{ makeGetValue('ptr', '0', 'i32') }}};
    {{{ makeSetValue('ptr', '0', 'ret+delta', 'i32') }}};
    return ret;
  },

  // gnu atomics

  __atomic_is_lock_free: function(size, ptr) {
    return size <= 4 && (size & (size-1)) == 0 && (ptr&(size-1)) == 0;
  },

  __atomic_load_8: function(ptr, memmodel) {
    {{{ makeStructuralReturn([makeGetValue('ptr', 0, 'i32'), makeGetValue('ptr', 4, 'i32')]) }}};
  },

  __atomic_store_8: function(ptr, vall, valh, memmodel) {
    {{{ makeSetValue('ptr', 0, 'vall', 'i32') }}};
    {{{ makeSetValue('ptr', 4, 'valh', 'i32') }}};
  },

  __atomic_exchange_8: function(ptr, vall, valh, memmodel) {
    var l = {{{ makeGetValue('ptr', 0, 'i32') }}};
    var h = {{{ makeGetValue('ptr', 4, 'i32') }}};
    {{{ makeSetValue('ptr', 0, 'vall', 'i32') }}};
    {{{ makeSetValue('ptr', 4, 'valh', 'i32') }}};
    {{{ makeStructuralReturn(['l', 'h']) }}};
  },

  __atomic_compare_exchange_8: function(ptr, expected, desiredl, desiredh, weak, success_memmodel, failure_memmodel) {
    var pl = {{{ makeGetValue('ptr', 0, 'i32') }}};
    var ph = {{{ makeGetValue('ptr', 4, 'i32') }}};
    var el = {{{ makeGetValue('expected', 0, 'i32') }}};
    var eh = {{{ makeGetValue('expected', 4, 'i32') }}};
    if (pl === el && ph === eh) {
      {{{ makeSetValue('ptr', 0, 'desiredl', 'i32') }}};
      {{{ makeSetValue('ptr', 4, 'desiredh', 'i32') }}};
      return 1;
    } else {
      {{{ makeSetValue('expected', 0, 'pl', 'i32') }}};
      {{{ makeSetValue('expected', 4, 'ph', 'i32') }}};
      return 0;
    }
  },

  __atomic_fetch_add_8__deps: ['i64Add'],
  __atomic_fetch_add_8: function(ptr, vall, valh, memmodel) {
    var l = {{{ makeGetValue('ptr', 0, 'i32') }}};
    var h = {{{ makeGetValue('ptr', 4, 'i32') }}};
    {{{ makeSetValue('ptr', 0, '_i64Add(l, h, vall, valh)', 'i32') }}};
    {{{ makeSetValue('ptr', 4, 'Runtime["getTempRet0"]()', 'i32') }}};
    {{{ makeStructuralReturn(['l', 'h']) }}};
  },

  __atomic_fetch_sub_8__deps: ['i64Subtract'],
  __atomic_fetch_sub_8: function(ptr, vall, valh, memmodel) {
    var l = {{{ makeGetValue('ptr', 0, 'i32') }}};
    var h = {{{ makeGetValue('ptr', 4, 'i32') }}};
    {{{ makeSetValue('ptr', 0, '_i64Subtract(l, h, vall, valh)', 'i32') }}};
    {{{ makeSetValue('ptr', 4, 'Runtime["getTempRet0"]()', 'i32') }}};
    {{{ makeStructuralReturn(['l', 'h']) }}};
  },

  __atomic_fetch_and_8__deps: ['i64Subtract'],
  __atomic_fetch_and_8: function(ptr, vall, valh, memmodel) {
    var l = {{{ makeGetValue('ptr', 0, 'i32') }}};
    var h = {{{ makeGetValue('ptr', 4, 'i32') }}};
    {{{ makeSetValue('ptr', 0, 'l&vall', 'i32') }}};
    {{{ makeSetValue('ptr', 4, 'h&valh', 'i32') }}};
    {{{ makeStructuralReturn(['l', 'h']) }}};
  },

  __atomic_fetch_or_8: function(ptr, vall, valh, memmodel) {
    var l = {{{ makeGetValue('ptr', 0, 'i32') }}};
    var h = {{{ makeGetValue('ptr', 4, 'i32') }}};
    {{{ makeSetValue('ptr', 0, 'l|vall', 'i32') }}};
    {{{ makeSetValue('ptr', 4, 'h|valh', 'i32') }}};
    {{{ makeStructuralReturn(['l', 'h']) }}};
  },

  __atomic_fetch_xor_8: function(ptr, vall, valh, memmodel) {
    var l = {{{ makeGetValue('ptr', 0, 'i32') }}};
    var h = {{{ makeGetValue('ptr', 4, 'i32') }}};
    {{{ makeSetValue('ptr', 0, 'l^vall', 'i32') }}};
    {{{ makeSetValue('ptr', 4, 'h^valh', 'i32') }}};
    {{{ makeStructuralReturn(['l', 'h']) }}};
  },

  emscripten_atomic_add_u32: 'llvm_atomic_load_add_i32_p0i32',
  emscripten_atomic_load_u64: '__atomic_load_8',
  emscripten_atomic_store_u64: '__atomic_store_8',
  emscripten_atomic_cas_u64: '__atomic_compare_exchange_8',
  emscripten_atomic_exchange_u64: '__atomic_exchange_8',
  _emscripten_atomic_fetch_and_add_u64: '__atomic_fetch_add_8',
  _emscripten_atomic_fetch_and_sub_u64: '__atomic_fetch_sub_8',
  _emscripten_atomic_fetch_and_and_u64: '__atomic_fetch_and_8',
  _emscripten_atomic_fetch_and_or_u64: '__atomic_fetch_or_8',
  _emscripten_atomic_fetch_and_xor_u64: '__atomic_fetch_xor_8',
};

mergeInto(LibraryManager.library, LibraryPThreadStub);
