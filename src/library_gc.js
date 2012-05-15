
if (GC_SUPPORT) {
  var LibraryGC = {
    $GC: {
      sizes: {},
      scannables: {}, // iterable
      finalizers: {},
      finalizerArgs: {},

      totalAllocations: 0, // bytes of all currently active objects
      recentAllocations: 0, // bytes allocated since last gc. ignores free()s

      malloc: function(bytes, clear, scannable) {
        var ptr;
        if (clear) {
          ptr = _calloc(bytes);
        } else {
          ptr = _malloc(bytes);
        }
        if (scannable) {
          GC.scannables[ptr] = 1;
        }
        GC.sizes[ptr] = bytes;
        GC.totalAllocations += bytes;
        GC.recentAllocations += bytes;
        return ptr;
      },

      free: function(ptr) { // does not check if anything refers to it, this is a forced free
        if (GC.scannables[ptr]) delete GC.scannables[ptr];
        var finalizer = GC.finalizers[ptr];
        if (finalizer) {
          Runtime.getFuncWrapper(finalizer)(GC.finalizerArgs[ptr]);
          GC.finalizers[ptr] = 0;
        }
        _free(ptr);
        GC.totalAllocations -= GC.sizes[ptr];
      },

      registerFinalizer: function(ptr, func, arg, oldFunc, oldArg) {
        var finalizer = GC.finalizers[ptr];
        if (finalizer) {
          if (oldFunc) {
            {{{ makeSetValue('oldFunc', '0', 'finalizer', 'i32') }}};
          }
          if (oldArg) {
            {{{ makeSetValue('oldArg', '0', 'GC.finalizerArgs[ptr]', 'i32') }}};
          }
        }
        GC.finalizers[ptr] = func;
        GC.finalizerArgs[ptr] = arg;
      },

      maybeCollect: function() {
        if (GC.needCollect()) GC.collect();
      },

      needCollect: function() {
        return true; // TODO: heuristics, # of allocations, time, etc.
      },

      collect: function() {
        GC.prep();
        GC.mark();
        GC.sweep();
      },

      prep: function() { // Clear reachables and scan for roots
        GC.reachable = {}; // XXX
        // static data areas: STACK_MAX to sbrk.DYNAMIC_START (after that, sbrk manages it
        //                     for dlmalloc). Note that DYNAMIC_START may not exist yet,
        //                     then use STATICTOP.
        // stack: STACK_ROOT to STACKTOP
        // registers: call scanners
      },

      mark: function() { // mark all reachable from roots
      },

      sweep: function() { // traverse all objects and free all unreachable
      }
    },

    GC_INIT__deps: ['$GC'],
    GC_INIT: function(){},

    GC_MALLOC__deps: ['$GC'],
    GC_MALLOC: function(bytes) {
      return GC.malloc(bytes, true, true);
    },

    GC_MALLOC_ATOMIC__deps: ['$GC'],
    GC_MALLOC_ATOMIC: function(bytes) {
      return GC.malloc(bytes, false, false);
    },

    GC_FREE__deps: ['$GC'],
    GC_FREE: function(ptr) {
      GC.free(ptr);
    },

    GC_REGISTER_FINALIZER_NO_ORDER__deps: ['$GC'],
    GC_REGISTER_FINALIZER_NO_ORDER: function(ptr, func, arg, old_func, old_arg) {
      GC.registerFinalizer(ptr, func, arg, old_func, old_arg);
    },

    GC_MAYBE_COLLECT__deps: ['$GC'],
    GC_MAYBE_COLLECT: function() {
      GC.maybeCollect();
    }
  };

  mergeInto(LibraryManager.library, LibraryGC);
}

