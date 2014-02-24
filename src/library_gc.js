
// WARNING: this is deprecated. You should just build Boehm from source, it is much faster than the toy version written in emscripten

if (GC_SUPPORT) {
  EXPORTED_FUNCTIONS['_calloc'] = 1;
  EXPORTED_FUNCTIONS['_realloc'] = 1;

  var LibraryGC = {
    $GC__deps: ['sbrk', 'realloc', 'calloc'],
    $GC: {
      ALLOCATIONS_TO_GC: 1*1024*1024,

      sizes: {}, // if in this map, then a live allocated object. this is iterable
      scannables: {},
      uncollectables: {},
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
          Browser.safeSetInterval(function() {
            GC.maybeCollect();
          }, 1000);
        } else {
#if ASSERTIONS
          Module.print('No HTML intervals, so you need to call GC.maybeCollect() or GC.collect() manually');
#endif
        }
      },

      malloc: function(bytes, clear, scannable, collectable) {
        if (!bytes) return 0;
        var ptr;
        if (clear) {
          ptr = _calloc(1, bytes);
        } else {
          ptr = _malloc(bytes);
        }
        if (!collectable) {
          GC.uncollectables[ptr] = true;
        }
        GC.scannables[ptr] = scannable;
        GC.sizes[ptr] = bytes;
        GC.totalAllocations += bytes;
        GC.recentAllocations += bytes;
        return ptr;
      },

      realloc: function(ptr, newBytes) {
        if (newBytes != 0) {
          var oldBytes = GC.sizes[ptr];
          var newPtr = _realloc(ptr, newBytes);
          if (newBytes > oldBytes) {
            _memset(newPtr + oldBytes, 0, newBytes - oldBytes);
          }
          delete GC.sizes[ptr];
          GC.sizes[newPtr] = newBytes;
          scannable = GC.scannables[ptr];
          delete GC.scannables[ptr];
          GC.scannables[newPtr] = scannable;
          var finalizer = GC.finalizers[ptr];
          if (finalizer) {
            delete GC.finalizers[ptr];
            GC.finalizers[newPtr] = finalizer;
          }
          var finalizerArgs = GC.finalizerArgs[ptr];
          if (finalizerArgs) {
            delete GC.finalizerArgs[ptr];
            GC.finalizerArgs[newPtr] = finalizerArgs;
          }
          var uncollectable = GC.uncollectables[ptr];
          if (uncollectable) {
            delete GC.uncollectables[ptr];
            GC.uncollectables[newPtr] = true;
          }
          GC.totalAllocations += (newBytes - oldBytes);
          return newPtr;
        } else {
          GC.free(ptr);
          return 0;
        }
      },

      free: function(ptr) { // does not check if anything refers to it, this is a forced free
        var finalizer = GC.finalizers[ptr];
        if (finalizer) {
          Runtime.getFuncWrapper(finalizer, 'vii')(ptr, GC.finalizerArgs[ptr]);
          GC.finalizers[ptr] = 0;
        }
        _free(ptr);
        GC.totalAllocations -= GC.sizes[ptr];
        delete GC.sizes[ptr];
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

      getHeapSize: function() {
        return GC.totalAllocations;
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
        GC.scan(STATIC_BASE, STATICTOP);
        // TODO: scan stack and registers. Currently we assume we run from a timeout or such, so no stack/regs
        //    stack: STACK_BASE to STACKTOP
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
          if (!GC.reachable[ptr] && !GC.uncollectables[ptr]) {
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
      return GC.malloc(bytes, true, true, true);
    },

    GC_MALLOC_ATOMIC__deps: ['$GC'],
    GC_MALLOC_ATOMIC: function(bytes) {
      return GC.malloc(bytes, false, false, true);
    },

    GC_MALLOC_UNCOLLECTABLE__deps: ['$GC'],
    GC_MALLOC_UNCOLLECTABLE: function(bytes) {
      return GC.malloc(bytes, true, true, false);
    },

    GC_REALLOC__deps: ['$GC'],
    GC_REALLOC: function(ptr, newBytes) {
      return GC.realloc(ptr, newBytes);
    },

    GC_FREE__deps: ['$GC'],
    GC_FREE: function(ptr) {
      GC.free(ptr);
    },

    GC_REGISTER_FINALIZER_NO_ORDER__deps: ['$GC'],
    GC_REGISTER_FINALIZER_NO_ORDER: function(ptr, func, arg, old_func, old_arg) {
      GC.registerFinalizer(ptr, func, arg, old_func, old_arg);
    },

    GC_get_heap_size__deps: ['$GC'],
    GC_get_heap_size: function() {
      return GC.getHeapSize();
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

