
if (GC_SUPPORT) {
  var LibraryGC = {
    $GC__deps: ['sbrk'],
    $GC: {
      ALLOCATIONS_TO_GC: 1*1024*1024,

      sizes: {}, // if in this map, then a live allocated object. this is iterable
      scannables: {},
      finalizers: {},
      finalizerArgs: {},

      totalAllocations: 0, // bytes of all currently active objects
      recentAllocations: 0, // bytes allocated since last gc. ignores free()s

      init: function() {
        assert(!GC.initted);
        GC.initted = true;

        _GC_finalize_on_demand = _malloc(4); setValue(_GC_finalize_on_demand, 0, 'i32')
        _GC_java_finalization = _malloc(4); setValue(_GC_java_finalization, 0, 'i32');
        _GC_finalizer_notifier = _malloc(4); setValue(_GC_finalizer_notifier, 0, 'i32');

        if (ENVIRONMENT_IS_WEB) {
          setInterval(function() {
            GC.maybeCollect();
          }, 1000);
        } else {
#if ASSERTIONS
          Module.print('No HTML intervals, so you need to call GC.maybeCollect() or GC.collect() manually');
#endif
        }
      },

      malloc: function(bytes, clear, scannable) {
        if (!bytes) return 0;
        var ptr;
        if (clear) {
          ptr = _calloc(1, bytes);
        } else {
          ptr = _malloc(bytes);
        }
        GC.scannables[ptr] = scannable;
        GC.sizes[ptr] = bytes;
        GC.totalAllocations += bytes;
        GC.recentAllocations += bytes;
        return ptr;
      },

      free: function(ptr) { // does not check if anything refers to it, this is a forced free
        var finalizer = GC.finalizers[ptr];
        if (finalizer) {
          Runtime.getFuncWrapper(finalizer)(ptr, GC.finalizerArgs[ptr]);
          GC.finalizers[ptr] = 0;
        }
        _free(ptr);
        delete GC.sizes[ptr];
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
        return GC.recentAllocations >= GC.ALLOCATIONS_TO_GC; // TODO: time, etc.
      },

      collect: function() {
        GC.prep();
        GC.mark();
        GC.sweep();
        GC.recentAllocations = 0;
      },

      scan: function(start, end) { // scans a memory region and adds new reachable objects
        for (var i = start; i < end; i += {{{ Runtime.getNativeTypeSize('void*') }}}) {
          var ptr = {{{ makeGetValue('i', '0', 'void*') }}};
          if (GC.sizes[ptr] && !GC.reachable[ptr]) {
            GC.reachable[ptr] = 1;
            if (GC.scannables[ptr]) {
              GC.reachableList.push(ptr);
            }
          }
        }
      },

      prep: function() { // Clear reachables and scan for roots
        GC.reachable = {}; // 1 if reachable. XXX
        GC.reachableList = []; // each reachable is added once to this. XXX
        // static data areas
        var staticStart = STACK_MAX;
        var staticEnd = _sbrk.DYNAMIC_START || STATICTOP; // after DYNAMIC_START, sbrk manages it (but it might not exist yet)
        GC.scan(staticStart, staticEnd);
        // TODO: scan stack and registers. Currently we assume we run from a timeout or such, so no stack/regs
        //    stack: STACK_ROOT to STACKTOP
        //    registers: call scanners
      },

      mark: function() { // mark all reachable from roots
        for (var i = 0; i < GC.reachableList.length; i++) { // note that the list length changes as we push more
          var ptr = GC.reachableList[i];
          GC.scan(ptr, ptr + GC.sizes[ptr]);
        }
      },

      sweep: function() { // traverse all objects and free all unreachable
        var freeList = [];
        for (var ptr in GC.sizes) {
          if (!GC.reachable[ptr]) {
            freeList.push(parseInt(ptr));
          }
        }
        for (var i = 0; i < freeList.length; i++) {
          GC.free(freeList[i]);
        }
      }
    },

    GC_INIT__deps: ['$GC'],
    GC_INIT: function() {
      GC.init();
    },

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
    },

    GC_FORCE_COLLECT__deps: ['$GC'],
    GC_FORCE_COLLECT: function() {
      GC.collect();
    },

    GC_finalize_on_demand: 0,
    GC_java_finalization: 0,
    GC_finalizer_notifier: 0,

    GC_enable_incremental: function(){},
  };

  mergeInto(LibraryManager.library, LibraryGC);
}

