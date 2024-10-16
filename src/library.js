/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

//"use strict";

// An implementation of basic necessary libraries for the web. This integrates
// with a compiled libc and with the rest of the JS runtime.
//
// We search the Library object when there is an external function. If the
// entry in the Library is a function, we insert it. If it is a string, we
// do another lookup in the library (a simple way to write a function once,
// if it can be called by different names). We also allow dependencies,
// using __deps. Initialization code to be run after allocating all
// global constants can be defined by __postset.
//
// Note that the full function name will be '_' + the name in the Library
// object. For convenience, the short name appears here. Note that if you add a
// new function with an '_', it will not be found.

addToLibrary({
  // JS aliases for native stack manipulation functions and tempret handling
  $stackSave__deps: ['emscripten_stack_get_current'],
  $stackSave: () => _emscripten_stack_get_current(),
  $stackRestore__deps: ['_emscripten_stack_restore'],
  $stackRestore: (val) => __emscripten_stack_restore(val),
  $stackAlloc__deps: ['_emscripten_stack_alloc'],
  $stackAlloc: (sz) => __emscripten_stack_alloc(sz),
  $getTempRet0__deps: ['_emscripten_tempret_get'],
  $getTempRet0: (val) => __emscripten_tempret_get(),
  $setTempRet0__deps: ['_emscripten_tempret_set'],
  $setTempRet0: (val) => __emscripten_tempret_set(val),

  // Aliases that allow legacy names (without leading $) for the
  // functions to continue to work in `__deps` entries.
  stackAlloc: '$stackAlloc',
  stackSave: '$stackSave',
  stackRestore: '$stackSave',
  setTempRet0: '$setTempRet0',
  getTempRet0: '$getTempRet0',

  $ptrToString: (ptr) => {
#if ASSERTIONS
    assert(typeof ptr === 'number');
#endif
#if !CAN_ADDRESS_2GB && !MEMORY64
    // With CAN_ADDRESS_2GB or MEMORY64, pointers are already unsigned.
    ptr >>>= 0;
#endif
    return '0x' + ptr.toString(16).padStart(8, '0');
  },

  $zeroMemory: (address, size) => {
#if LEGACY_VM_SUPPORT
    if (!HEAPU8.fill) {
      for (var i = 0; i < size; i++) {
        HEAPU8[address + i] = 0;
      }
      return;
    }
#endif
    HEAPU8.fill(0, address, address + size);
  },

#if SAFE_HEAP
  // Trivial wrappers around runtime functions that make these symbols available
  // to native code.
  segfault: '=segfault',
  alignfault: '=alignfault',
#endif

  // ==========================================================================
  // JavaScript <-> C string interop
  // ==========================================================================

#if !MINIMAL_RUNTIME
  $exitJS__docs: '/** @param {boolean|number=} implicit */',
  $exitJS__deps: [
    'proc_exit',
#if ASSERTIONS || EXIT_RUNTIME
    '$keepRuntimeAlive',
#endif
#if PTHREADS
    '$exitOnMainThread',
#endif
#if PTHREADS_DEBUG
    '$runtimeKeepaliveCounter',
#endif
  ],
  $exitJS: (status, implicit) => {
    EXITSTATUS = status;

#if ASSERTIONS && !EXIT_RUNTIME
    checkUnflushedContent();
#endif // ASSERTIONS && !EXIT_RUNTIME

#if PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) {
      // implicit exit can never happen on a pthread
#if ASSERTIONS
      assert(!implicit);
#endif
#if PTHREADS_DEBUG
      dbg(`Pthread ${ptrToString(_pthread_self())} called exit(${status}), posting exitOnMainThread.`);
#endif
      // When running in a pthread we propagate the exit back to the main thread
      // where it can decide if the whole process should be shut down or not.
      // The pthread may have decided not to exit its own runtime, for example
      // because it runs a main loop, but that doesn't affect the main thread.
      exitOnMainThread(status);
      throw 'unwind';
    }
#if PTHREADS_DEBUG
    err(`main thread called exit(${status}): keepRuntimeAlive=${keepRuntimeAlive()} (counter=${runtimeKeepaliveCounter})`);
#endif // PTHREADS_DEBUG
#endif // PTHREADS

#if EXIT_RUNTIME
    if (!keepRuntimeAlive()) {
      exitRuntime();
    }
#endif

#if ASSERTIONS
    // if exit() was called explicitly, warn the user if the runtime isn't actually being shut down
    if (keepRuntimeAlive() && !implicit) {
      var msg = `program exited (with status: ${status}), but keepRuntimeAlive() is set (counter=${runtimeKeepaliveCounter}) due to an async operation, so halting execution but not exiting the runtime or preventing further async execution (you can use emscripten_force_exit, if you want to force a true shutdown)`;
#if MODULARIZE
      readyPromiseReject(msg);
#endif // MODULARIZE
      err(msg);
    }
#endif // ASSERTIONS

    _proc_exit(status);
  },
#endif

#if MINIMAL_RUNTIME
  // minimal runtime doesn't do any exit cleanup handling so just
  // map exit directly to the lower-level proc_exit syscall.
  exit: 'proc_exit',
#else
  exit: '$exitJS',
#endif

  // Returns a pointer ('p'), which means an i32 on wasm32 and an i64 wasm64
  // We have a separate JS version `getHeapMax()` which can be called directly
  // avoiding any wrapper added for wasm64.
  emscripten_get_heap_max__deps: ['$getHeapMax'],
  emscripten_get_heap_max: () => getHeapMax(),

  $getHeapMax: () =>
#if ALLOW_MEMORY_GROWTH
#if MEMORY64 == 1
    {{{ MAXIMUM_MEMORY }}},
#else
    // Stay one Wasm page short of 4GB: while e.g. Chrome is able to allocate
    // full 4GB Wasm memories, the size will wrap back to 0 bytes in Wasm side
    // for any code that deals with heap sizes, which would require special
    // casing all heap size related code to treat 0 specially.
    {{{ Math.min(MAXIMUM_MEMORY, FOUR_GB - WASM_PAGE_SIZE) }}},
#endif
#else // no growth
    HEAPU8.length,
#endif

#if ABORTING_MALLOC
  $abortOnCannotGrowMemory: (requestedSize) => {
#if ASSERTIONS
#if ALLOW_MEMORY_GROWTH
    abort(`Cannot enlarge memory arrays to size ${requestedSize} bytes (OOM). If you want malloc to return NULL (0) instead of this abort, do not link with -sABORTING_MALLOC (that is, the default when growth is enabled is to not abort, but you have overridden that)`);
#else // ALLOW_MEMORY_GROWTH
    abort(`Cannot enlarge memory arrays to size ${requestedSize} bytes (OOM). Either (1) compile with -sINITIAL_MEMORY=X with X higher than the current value ${HEAP8.length}, (2) compile with -sALLOW_MEMORY_GROWTH which allows increasing the size at runtime, or (3) if you want malloc to return NULL (0) instead of this abort, compile with -sABORTING_MALLOC=0`);
#endif // ALLOW_MEMORY_GROWTH
#else // ASSERTIONS
    abort('OOM');
#endif // ASSERTIONS
  },
#endif // ABORTING_MALLOC

  // Grows the wasm memory to the given byte size, and updates the JS views to
  // it. Returns 1 on success, 0 on error.
  $growMemory: (size) => {
    var b = wasmMemory.buffer;
    var pages = ((size - b.byteLength + {{{ WASM_PAGE_SIZE - 1 }}}) / {{{ WASM_PAGE_SIZE }}}) | 0;
#if RUNTIME_DEBUG
    dbg(`growMemory: ${size} (+${size - b.byteLength} bytes / ${pages} pages)`);
#endif
#if MEMORYPROFILER
    var oldHeapSize = b.byteLength;
#endif
    try {
      // round size grow request up to wasm page size (fixed 64KB per spec)
      wasmMemory.grow({{{ toIndexType('pages') }}}); // .grow() takes a delta compared to the previous size
      updateMemoryViews();
#if MEMORYPROFILER
      if (typeof emscriptenMemoryProfiler != 'undefined') {
        emscriptenMemoryProfiler.onMemoryResize(oldHeapSize, b.byteLength);
      }
#endif
      return 1 /*success*/;
    } catch(e) {
#if ASSERTIONS
      err(`growMemory: Attempted to grow heap from ${b.byteLength} bytes to ${size} bytes, but got error: ${e}`);
#endif
    }
    // implicit 0 return to save code size (caller will cast "undefined" into 0
    // anyhow)
  },

  emscripten_resize_heap__deps: [
    '$getHeapMax',
    '$alignMemory',
#if ASSERTIONS == 2
    'emscripten_get_now',
#endif
#if ABORTING_MALLOC
    '$abortOnCannotGrowMemory',
#endif
#if ALLOW_MEMORY_GROWTH
    '$growMemory',
#endif
  ],
  emscripten_resize_heap: 'ip',
  emscripten_resize_heap: (requestedSize) => {
    var oldSize = HEAPU8.length;
#if !MEMORY64 && !CAN_ADDRESS_2GB
    // With CAN_ADDRESS_2GB or MEMORY64, pointers are already unsigned.
    requestedSize >>>= 0;
#endif
#if ALLOW_MEMORY_GROWTH == 0
#if ABORTING_MALLOC
    abortOnCannotGrowMemory(requestedSize);
#else
    return false; // malloc will report failure
#endif // ABORTING_MALLOC
#else // ALLOW_MEMORY_GROWTH == 0
    // With multithreaded builds, races can happen (another thread might increase the size
    // in between), so return a failure, and let the caller retry.
#if SHARED_MEMORY
    if (requestedSize <= oldSize) {
      return false;
    }
#elif ASSERTIONS
    assert(requestedSize > oldSize);
#endif

#if EMSCRIPTEN_TRACING
    // Report old layout one last time
    _emscripten_trace_report_memory_layout();
#endif

    // Memory resize rules:
    // 1.  Always increase heap size to at least the requested size, rounded up
    //     to next page multiple.
    // 2a. If MEMORY_GROWTH_LINEAR_STEP == -1, excessively resize the heap
    //     geometrically: increase the heap size according to
    //     MEMORY_GROWTH_GEOMETRIC_STEP factor (default +20%), At most
    //     overreserve by MEMORY_GROWTH_GEOMETRIC_CAP bytes (default 96MB).
    // 2b. If MEMORY_GROWTH_LINEAR_STEP != -1, excessively resize the heap
    //     linearly: increase the heap size by at least
    //     MEMORY_GROWTH_LINEAR_STEP bytes.
    // 3.  Max size for the heap is capped at 2048MB-WASM_PAGE_SIZE, or by
    //     MAXIMUM_MEMORY, or by ASAN limit, depending on which is smallest
    // 4.  If we were unable to allocate as much memory, it may be due to
    //     over-eager decision to excessively reserve due to (3) above.
    //     Hence if an allocation fails, cut down on the amount of excess
    //     growth, in an attempt to succeed to perform a smaller allocation.

    // A limit is set for how much we can grow. We should not exceed that
    // (the wasm binary specifies it, so if we tried, we'd fail anyhow).
    var maxHeapSize = getHeapMax();
    if (requestedSize > maxHeapSize) {
#if ASSERTIONS
      err(`Cannot enlarge memory, requested ${requestedSize} bytes, but the limit is ${maxHeapSize} bytes!`);
#endif
#if ABORTING_MALLOC
      abortOnCannotGrowMemory(requestedSize);
#else
      return false;
#endif
    }

    // Loop through potential heap size increases. If we attempt a too eager
    // reservation that fails, cut down on the attempted size and reserve a
    // smaller bump instead. (max 3 times, chosen somewhat arbitrarily)
    for (var cutDown = 1; cutDown <= 4; cutDown *= 2) {
#if MEMORY_GROWTH_LINEAR_STEP == -1
      var overGrownHeapSize = oldSize * (1 + {{{ MEMORY_GROWTH_GEOMETRIC_STEP }}} / cutDown); // ensure geometric growth
#if MEMORY_GROWTH_GEOMETRIC_CAP
      // but limit overreserving (default to capping at +96MB overgrowth at most)
      overGrownHeapSize = Math.min(overGrownHeapSize, requestedSize + {{{ MEMORY_GROWTH_GEOMETRIC_CAP }}} );
#endif

#else
      var overGrownHeapSize = oldSize + {{{ MEMORY_GROWTH_LINEAR_STEP }}} / cutDown; // ensure linear growth
#endif

      var newSize = Math.min(maxHeapSize, alignMemory(Math.max(requestedSize, overGrownHeapSize), {{{ WASM_PAGE_SIZE }}}));

#if ASSERTIONS == 2
      var t0 = _emscripten_get_now();
#endif
      var replacement = growMemory(newSize);
#if ASSERTIONS == 2
      var t1 = _emscripten_get_now();
      dbg(`Heap resize call from ${oldSize} to ${newSize} took ${(t1 - t0)} msecs. Success: ${!!replacement}`);
#endif
      if (replacement) {
#if ASSERTIONS && WASM2JS
        err('Warning: Enlarging memory arrays, this is not fast! ' + [oldSize, newSize]);
#endif

#if EMSCRIPTEN_TRACING
        traceLogMessage("Emscripten", `Enlarging memory arrays from ${oldSize} to ${newSize}`);
        // And now report the new layout
        _emscripten_trace_report_memory_layout();
#endif
        return true;
      }
    }
#if ASSERTIONS
    err(`Failed to grow the heap from ${oldSize} bytes to ${newSize} bytes, not enough memory!`);
#endif
#if ABORTING_MALLOC
    abortOnCannotGrowMemory(requestedSize);
#else
    return false;
#endif
#endif // ALLOW_MEMORY_GROWTH
  },

  // Called after wasm grows memory. At that time we need to update the views.
  // Without this notification, we'd need to check the buffer in JS every time
  // we return from any wasm, which adds overhead. See
  // https://github.com/WebAssembly/WASI/issues/82
  emscripten_notify_memory_growth: (memoryIndex) => {
#if ASSERTIONS
    assert(memoryIndex == 0);
#endif
    updateMemoryViews();
  },

  _emscripten_system: (command) => {
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE) {
      if (!command) return 1; // shell is available

      var cmdstr = UTF8ToString(command);
      if (!cmdstr.length) return 0; // this is what glibc seems to do (shell works test?)

      var cp = require('child_process');
      var ret = cp.spawnSync(cmdstr, [], {shell:true, stdio:'inherit'});

      var _W_EXITCODE = (ret, sig) => ((ret) << 8 | (sig));

      // this really only can happen if process is killed by signal
      if (ret.status === null) {
        // sadly node doesn't expose such function
        var signalToNumber = (sig) => {
          // implement only the most common ones, and fallback to SIGINT
          switch (sig) {
            case 'SIGHUP': return {{{ cDefs.SIGHUP }}};
            case 'SIGQUIT': return {{{ cDefs.SIGQUIT }}};
            case 'SIGFPE': return {{{ cDefs.SIGFPE }}};
            case 'SIGKILL': return {{{ cDefs.SIGKILL }}};
            case 'SIGALRM': return {{{ cDefs.SIGALRM }}};
            case 'SIGTERM': return {{{ cDefs.SIGTERM }}};
            default: return {{{ cDefs.SIGINT }}};
          }
        }
        return _W_EXITCODE(0, signalToNumber(ret.signal));
      }

      return _W_EXITCODE(ret.status, 0);
    }
#endif // ENVIRONMENT_MAY_BE_NODE
    // int system(const char *command);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/system.html
    // Can't call external programs.
    if (!command) return 0; // no shell available
    return -{{{ cDefs.ENOSYS }}};
  },

  // ==========================================================================
  // stdlib.h
  // ==========================================================================

#if !STANDALONE_WASM
  // Used to implement the native `abort` symbol.  Note that we use the
  // JavaScript `abort` helper in order to implement this function, but we use a
  // distinct name here to avoid confusing the two.
  _abort_js: () => {
#if ASSERTIONS
    abort('native code called abort()');
#else
    abort('');
#endif
  },
#endif

  // This object can be modified by the user during startup, which affects
  // the initial values of the environment accessible by getenv.
  $ENV: {},

  // In -Oz builds, we replace memcpy() altogether with a non-unrolled wasm
  // variant, so we should never emit _emscripten_memcpy_js() in the build.
  // In STANDALONE_WASM we avoid the _emscripten_memcpy_js dependency so keep
  // the wasm file standalone.
  // In BULK_MEMORY mode we include native versions of these functions based
  // on memory.fill and memory.copy.
  // In MAIN_MODULE=1 or EMCC_FORCE_STDLIBS mode all of libc is force included
  // so we cannot override parts of it, and therefore cannot use libc_optz.
#if (SHRINK_LEVEL < 2 || LINKABLE || process.env.EMCC_FORCE_STDLIBS) && !STANDALONE_WASM && !BULK_MEMORY

#if MIN_CHROME_VERSION < 45 || MIN_FIREFOX_VERSION < 34 || MIN_SAFARI_VERSION < 100101
  // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/TypedArray/copyWithin lists browsers that support TypedArray.prototype.copyWithin, but it
  // has outdated information for Safari, saying it would not support it.
  // https://github.com/WebKit/webkit/commit/24a800eea4d82d6d595cdfec69d0f68e733b5c52#diff-c484911d8df319ba75fce0d8e7296333R1 suggests support was added on Aug 28, 2015.
  // Manual testing suggests:
  //   Safari/601.1 Version/9.0 on iPhone 4s with iOS 9.3.6 (released September 30, 2015) does not support copyWithin.
  // but the following systems do:
  //   AppleWebKit/602.2.14 Safari/602.1 Version/10.0 Mobile/14B100 iPhone OS 10_1_1 on iPhone 5s with iOS 10.1.1 (released October 31, 2016)
  //   AppleWebKit/603.3.8 Safari/602.1 Version/10.0 on iPhone 5 with iOS 10.3.4 (released July 22, 2019)
  //   AppleWebKit/605.1.15 iPhone OS 12_3_1 Version/12.1.1 Safari/604.1 on iPhone SE with iOS 12.3.1
  //   AppleWebKit/605.1.15 Safari/604.1 Version/13.0.4 iPhone OS 13_3 on iPhone 6s with iOS 13.3
  //   AppleWebKit/605.1.15 Version/13.0.3 Intel Mac OS X 10_15_1 on Safari 13.0.3 (15608.3.10.1.4) on macOS Catalina 10.15.1
  // Hence the support status of .copyWithin() for Safari version range [10.0.0, 10.1.0] is unknown.
  _emscripten_memcpy_js: `= Uint8Array.prototype.copyWithin
    ? (dest, src, num) => HEAPU8.copyWithin(dest, src, src + num)
    : (dest, src, num) => HEAPU8.set(HEAPU8.subarray(src, src+num), dest)`,
#else
  _emscripten_memcpy_js: (dest, src, num) => HEAPU8.copyWithin(dest, src, src + num),
#endif

#endif

#if !STANDALONE_WASM
  // ==========================================================================
  // assert.h
  // ==========================================================================

  __assert_fail: (condition, filename, line, func) => {
    abort(`Assertion failed: ${UTF8ToString(condition)}, at: ` + [filename ? UTF8ToString(filename) : 'unknown filename', line, func ? UTF8ToString(func) : 'unknown function']);
  },
#endif

#if STACK_OVERFLOW_CHECK >= 2
  // Set stack limits used by binaryen's `StackCheck` pass.
#if MAIN_MODULE
  $setStackLimits__deps: ['$setDylinkStackLimits'],
#endif
  $setStackLimits: () => {
    var stackLow = _emscripten_stack_get_base();
    var stackHigh = _emscripten_stack_get_end();
#if RUNTIME_DEBUG
    dbg(`setStackLimits: ${ptrToString(stackLow)}, ${ptrToString(stackHigh)}`);
#endif
#if MAIN_MODULE
    // With dynamic linking we could have any number of pre-loaded libraries
    // that each need to have their stack limits set.
    setDylinkStackLimits(stackLow, stackHigh);
#else
    ___set_stack_limits(stackLow, stackHigh);
#endif
  },
#endif

  $withStackSave__internal: true,
  $withStackSave__deps: ['$stackSave', '$stackRestore'],
  $withStackSave: (f) => {
    var stack = stackSave();
    var ret = f();
    stackRestore(stack);
    return ret;
  },

  // ==========================================================================
  // setjmp.h
  // ==========================================================================

#if SUPPORT_LONGJMP == 'emscripten'
  // In WebAssemblyLowerEmscriptenEHSjLj pass in the LLVM backend, function
  // calls that exist in the same function with setjmp are converted to a code
  // sequence that includes invokes, malloc, free, saveSetjmp, and
  // emscripten_longjmp.  setThrew is called from invokes, but we don't have
  // any way to express that dependency so we use emscripten_throw_longjmp as
  // a proxy and declare the dependency here.
  _emscripten_throw_longjmp__deps: ['setThrew'],
  _emscripten_throw_longjmp: () => {
#if EXCEPTION_STACK_TRACES
    throw new EmscriptenSjLj;
#else
    throw Infinity;
#endif
  },
#elif !SUPPORT_LONGJMP
#if !INCLUDE_FULL_LIBRARY
  // These are in order to print helpful error messages when either longjmp of
  // setjmp is used.
  longjmp__deps: [() => {
    error('longjmp support was disabled (SUPPORT_LONGJMP=0), but it is required by the code (either set SUPPORT_LONGJMP=1, or remove uses of it in the project)');
  }],
  get setjmp__deps() {
    return this.longjmp__deps;
  },
  // This is to print the correct error message when a program is built with
  // SUPPORT_LONGJMP=1 but linked with SUPPORT_LONGJMP=0. When a program is
  // built with SUPPORT_LONGJMP=1, the object file contains references of not
  // longjmp but _emscripten_throw_longjmp, which is called from
  // emscripten_longjmp.
  get _emscripten_throw_longjmp__deps() {
    return this.longjmp__deps;
  },
#endif
  _emscripten_throw_longjmp: () => {
    error('longjmp support was disabled (SUPPORT_LONGJMP=0), but it is required by the code (either set SUPPORT_LONGJMP=1, or remove uses of it in the project)');
  },
  // will never be emitted, as the dep errors at compile time
  longjmp: (env, value) => {
    abort('longjmp not supported (build with -s SUPPORT_LONGJMP)');
  },
  setjmp: (env) => {
    abort('setjmp not supported (build with -s SUPPORT_LONGJMP)');
  },
#endif

  // ==========================================================================
  // errno.h
  // ==========================================================================

  // We use a string literal here to avoid the string quotes on the object
  // keys being removed when processed by jsifier.
  $ERRNO_CODES: `{
    'EPERM': {{{ cDefs.EPERM }}},
    'ENOENT': {{{ cDefs.ENOENT }}},
    'ESRCH': {{{ cDefs.ESRCH }}},
    'EINTR': {{{ cDefs.EINTR }}},
    'EIO': {{{ cDefs.EIO }}},
    'ENXIO': {{{ cDefs.ENXIO }}},
    'E2BIG': {{{ cDefs.E2BIG }}},
    'ENOEXEC': {{{ cDefs.ENOEXEC }}},
    'EBADF': {{{ cDefs.EBADF }}},
    'ECHILD': {{{ cDefs.ECHILD }}},
    'EAGAIN': {{{ cDefs.EAGAIN }}},
    'EWOULDBLOCK': {{{ cDefs.EWOULDBLOCK }}},
    'ENOMEM': {{{ cDefs.ENOMEM }}},
    'EACCES': {{{ cDefs.EACCES }}},
    'EFAULT': {{{ cDefs.EFAULT }}},
    'ENOTBLK': {{{ cDefs.ENOTBLK }}},
    'EBUSY': {{{ cDefs.EBUSY }}},
    'EEXIST': {{{ cDefs.EEXIST }}},
    'EXDEV': {{{ cDefs.EXDEV }}},
    'ENODEV': {{{ cDefs.ENODEV }}},
    'ENOTDIR': {{{ cDefs.ENOTDIR }}},
    'EISDIR': {{{ cDefs.EISDIR }}},
    'EINVAL': {{{ cDefs.EINVAL }}},
    'ENFILE': {{{ cDefs.ENFILE }}},
    'EMFILE': {{{ cDefs.EMFILE }}},
    'ENOTTY': {{{ cDefs.ENOTTY }}},
    'ETXTBSY': {{{ cDefs.ETXTBSY }}},
    'EFBIG': {{{ cDefs.EFBIG }}},
    'ENOSPC': {{{ cDefs.ENOSPC }}},
    'ESPIPE': {{{ cDefs.ESPIPE }}},
    'EROFS': {{{ cDefs.EROFS }}},
    'EMLINK': {{{ cDefs.EMLINK }}},
    'EPIPE': {{{ cDefs.EPIPE }}},
    'EDOM': {{{ cDefs.EDOM }}},
    'ERANGE': {{{ cDefs.ERANGE }}},
    'ENOMSG': {{{ cDefs.ENOMSG }}},
    'EIDRM': {{{ cDefs.EIDRM }}},
    'ECHRNG': {{{ cDefs.ECHRNG }}},
    'EL2NSYNC': {{{ cDefs.EL2NSYNC }}},
    'EL3HLT': {{{ cDefs.EL3HLT }}},
    'EL3RST': {{{ cDefs.EL3RST }}},
    'ELNRNG': {{{ cDefs.ELNRNG }}},
    'EUNATCH': {{{ cDefs.EUNATCH }}},
    'ENOCSI': {{{ cDefs.ENOCSI }}},
    'EL2HLT': {{{ cDefs.EL2HLT }}},
    'EDEADLK': {{{ cDefs.EDEADLK }}},
    'ENOLCK': {{{ cDefs.ENOLCK }}},
    'EBADE': {{{ cDefs.EBADE }}},
    'EBADR': {{{ cDefs.EBADR }}},
    'EXFULL': {{{ cDefs.EXFULL }}},
    'ENOANO': {{{ cDefs.ENOANO }}},
    'EBADRQC': {{{ cDefs.EBADRQC }}},
    'EBADSLT': {{{ cDefs.EBADSLT }}},
    'EDEADLOCK': {{{ cDefs.EDEADLOCK }}},
    'EBFONT': {{{ cDefs.EBFONT }}},
    'ENOSTR': {{{ cDefs.ENOSTR }}},
    'ENODATA': {{{ cDefs.ENODATA }}},
    'ETIME': {{{ cDefs.ETIME }}},
    'ENOSR': {{{ cDefs.ENOSR }}},
    'ENONET': {{{ cDefs.ENONET }}},
    'ENOPKG': {{{ cDefs.ENOPKG }}},
    'EREMOTE': {{{ cDefs.EREMOTE }}},
    'ENOLINK': {{{ cDefs.ENOLINK }}},
    'EADV': {{{ cDefs.EADV }}},
    'ESRMNT': {{{ cDefs.ESRMNT }}},
    'ECOMM': {{{ cDefs.ECOMM }}},
    'EPROTO': {{{ cDefs.EPROTO }}},
    'EMULTIHOP': {{{ cDefs.EMULTIHOP }}},
    'EDOTDOT': {{{ cDefs.EDOTDOT }}},
    'EBADMSG': {{{ cDefs.EBADMSG }}},
    'ENOTUNIQ': {{{ cDefs.ENOTUNIQ }}},
    'EBADFD': {{{ cDefs.EBADFD }}},
    'EREMCHG': {{{ cDefs.EREMCHG }}},
    'ELIBACC': {{{ cDefs.ELIBACC }}},
    'ELIBBAD': {{{ cDefs.ELIBBAD }}},
    'ELIBSCN': {{{ cDefs.ELIBSCN }}},
    'ELIBMAX': {{{ cDefs.ELIBMAX }}},
    'ELIBEXEC': {{{ cDefs.ELIBEXEC }}},
    'ENOSYS': {{{ cDefs.ENOSYS }}},
    'ENOTEMPTY': {{{ cDefs.ENOTEMPTY }}},
    'ENAMETOOLONG': {{{ cDefs.ENAMETOOLONG }}},
    'ELOOP': {{{ cDefs.ELOOP }}},
    'EOPNOTSUPP': {{{ cDefs.EOPNOTSUPP }}},
    'EPFNOSUPPORT': {{{ cDefs.EPFNOSUPPORT }}},
    'ECONNRESET': {{{ cDefs.ECONNRESET }}},
    'ENOBUFS': {{{ cDefs.ENOBUFS }}},
    'EAFNOSUPPORT': {{{ cDefs.EAFNOSUPPORT }}},
    'EPROTOTYPE': {{{ cDefs.EPROTOTYPE }}},
    'ENOTSOCK': {{{ cDefs.ENOTSOCK }}},
    'ENOPROTOOPT': {{{ cDefs.ENOPROTOOPT }}},
    'ESHUTDOWN': {{{ cDefs.ESHUTDOWN }}},
    'ECONNREFUSED': {{{ cDefs.ECONNREFUSED }}},
    'EADDRINUSE': {{{ cDefs.EADDRINUSE }}},
    'ECONNABORTED': {{{ cDefs.ECONNABORTED }}},
    'ENETUNREACH': {{{ cDefs.ENETUNREACH }}},
    'ENETDOWN': {{{ cDefs.ENETDOWN }}},
    'ETIMEDOUT': {{{ cDefs.ETIMEDOUT }}},
    'EHOSTDOWN': {{{ cDefs.EHOSTDOWN }}},
    'EHOSTUNREACH': {{{ cDefs.EHOSTUNREACH }}},
    'EINPROGRESS': {{{ cDefs.EINPROGRESS }}},
    'EALREADY': {{{ cDefs.EALREADY }}},
    'EDESTADDRREQ': {{{ cDefs.EDESTADDRREQ }}},
    'EMSGSIZE': {{{ cDefs.EMSGSIZE }}},
    'EPROTONOSUPPORT': {{{ cDefs.EPROTONOSUPPORT }}},
    'ESOCKTNOSUPPORT': {{{ cDefs.ESOCKTNOSUPPORT }}},
    'EADDRNOTAVAIL': {{{ cDefs.EADDRNOTAVAIL }}},
    'ENETRESET': {{{ cDefs.ENETRESET }}},
    'EISCONN': {{{ cDefs.EISCONN }}},
    'ENOTCONN': {{{ cDefs.ENOTCONN }}},
    'ETOOMANYREFS': {{{ cDefs.ETOOMANYREFS }}},
    'EUSERS': {{{ cDefs.EUSERS }}},
    'EDQUOT': {{{ cDefs.EDQUOT }}},
    'ESTALE': {{{ cDefs.ESTALE }}},
    'ENOTSUP': {{{ cDefs.ENOTSUP }}},
    'ENOMEDIUM': {{{ cDefs.ENOMEDIUM }}},
    'EILSEQ': {{{ cDefs.EILSEQ }}},
    'EOVERFLOW': {{{ cDefs.EOVERFLOW }}},
    'ECANCELED': {{{ cDefs.ECANCELED }}},
    'ENOTRECOVERABLE': {{{ cDefs.ENOTRECOVERABLE }}},
    'EOWNERDEAD': {{{ cDefs.EOWNERDEAD }}},
    'ESTRPIPE': {{{ cDefs.ESTRPIPE }}},
  }`,

#if PURE_WASI
  $strError: (errno) => errno + '',
#else
  $strError__deps: ['strerror', '$UTF8ToString'],
  $strError: (errno) => {
    return UTF8ToString(_strerror(errno));
  },
#endif

#if PROXY_POSIX_SOCKETS == 0
  // ==========================================================================
  // netdb.h
  // ==========================================================================

  $inetPton4: (str) => {
    var b = str.split('.');
    for (var i = 0; i < 4; i++) {
      var tmp = Number(b[i]);
      if (isNaN(tmp)) return null;
      b[i] = tmp;
    }
    return (b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24)) >>> 0;
  },
  $inetNtop4: (addr) => {
    return (addr & 0xff) + '.' + ((addr >> 8) & 0xff) + '.' + ((addr >> 16) & 0xff) + '.' + ((addr >> 24) & 0xff)
  },
  $inetPton6__deps: ['htons', '$jstoi_q'],
  $inetPton6: (str) => {
    var words;
    var w, offset, z, i;
    /* http://home.deds.nl/~aeron/regex/ */
    var valid6regx = /^((?=.*::)(?!.*::.+::)(::)?([\dA-F]{1,4}:(:|\b)|){5}|([\dA-F]{1,4}:){6})((([\dA-F]{1,4}((?!\3)::|:\b|$))|(?!\2\3)){2}|(((2[0-4]|1\d|[1-9])?\d|25[0-5])\.?\b){4})$/i
    var parts = [];
    if (!valid6regx.test(str)) {
      return null;
    }
    if (str === "::") {
      return [0, 0, 0, 0, 0, 0, 0, 0];
    }
    // Z placeholder to keep track of zeros when splitting the string on ":"
    if (str.startsWith("::")) {
      str = str.replace("::", "Z:"); // leading zeros case
    } else {
      str = str.replace("::", ":Z:");
    }

    if (str.indexOf(".") > 0) {
      // parse IPv4 embedded stress
      str = str.replace(new RegExp('[.]', 'g'), ":");
      words = str.split(":");
      words[words.length-4] = jstoi_q(words[words.length-4]) + jstoi_q(words[words.length-3])*256;
      words[words.length-3] = jstoi_q(words[words.length-2]) + jstoi_q(words[words.length-1])*256;
      words = words.slice(0, words.length-2);
    } else {
      words = str.split(":");
    }

    offset = 0; z = 0;
    for (w=0; w < words.length; w++) {
      if (typeof words[w] == 'string') {
        if (words[w] === 'Z') {
          // compressed zeros - write appropriate number of zero words
          for (z = 0; z < (8 - words.length+1); z++) {
            parts[w+z] = 0;
          }
          offset = z-1;
        } else {
          // parse hex to field to 16-bit value and write it in network byte-order
          parts[w+offset] = _htons(parseInt(words[w],16));
        }
      } else {
        // parsed IPv4 words
        parts[w+offset] = words[w];
      }
    }
    return [
      (parts[1] << 16) | parts[0],
      (parts[3] << 16) | parts[2],
      (parts[5] << 16) | parts[4],
      (parts[7] << 16) | parts[6]
    ];
  },
  $inetNtop6__deps: ['$inetNtop4', 'ntohs'],
  $inetNtop6: (ints) => {
    //  ref:  http://www.ietf.org/rfc/rfc2373.txt - section 2.5.4
    //  Format for IPv4 compatible and mapped  128-bit IPv6 Addresses
    //  128-bits are split into eight 16-bit words
    //  stored in network byte order (big-endian)
    //  |                80 bits               | 16 |      32 bits        |
    //  +-----------------------------------------------------------------+
    //  |               10 bytes               |  2 |      4 bytes        |
    //  +--------------------------------------+--------------------------+
    //  +               5 words                |  1 |      2 words        |
    //  +--------------------------------------+--------------------------+
    //  |0000..............................0000|0000|    IPv4 ADDRESS     | (compatible)
    //  +--------------------------------------+----+---------------------+
    //  |0000..............................0000|FFFF|    IPv4 ADDRESS     | (mapped)
    //  +--------------------------------------+----+---------------------+
    var str = "";
    var word = 0;
    var longest = 0;
    var lastzero = 0;
    var zstart = 0;
    var len = 0;
    var i = 0;
    var parts = [
      ints[0] & 0xffff,
      (ints[0] >> 16),
      ints[1] & 0xffff,
      (ints[1] >> 16),
      ints[2] & 0xffff,
      (ints[2] >> 16),
      ints[3] & 0xffff,
      (ints[3] >> 16)
    ];

    // Handle IPv4-compatible, IPv4-mapped, loopback and any/unspecified addresses

    var hasipv4 = true;
    var v4part = "";
    // check if the 10 high-order bytes are all zeros (first 5 words)
    for (i = 0; i < 5; i++) {
      if (parts[i] !== 0) { hasipv4 = false; break; }
    }

    if (hasipv4) {
      // low-order 32-bits store an IPv4 address (bytes 13 to 16) (last 2 words)
      v4part = inetNtop4(parts[6] | (parts[7] << 16));
      // IPv4-mapped IPv6 address if 16-bit value (bytes 11 and 12) == 0xFFFF (6th word)
      if (parts[5] === -1) {
        str = "::ffff:";
        str += v4part;
        return str;
      }
      // IPv4-compatible IPv6 address if 16-bit value (bytes 11 and 12) == 0x0000 (6th word)
      if (parts[5] === 0) {
        str = "::";
        //special case IPv6 addresses
        if (v4part === "0.0.0.0") v4part = ""; // any/unspecified address
        if (v4part === "0.0.0.1") v4part = "1";// loopback address
        str += v4part;
        return str;
      }
    }

    // Handle all other IPv6 addresses

    // first run to find the longest contiguous zero words
    for (word = 0; word < 8; word++) {
      if (parts[word] === 0) {
        if (word - lastzero > 1) {
          len = 0;
        }
        lastzero = word;
        len++;
      }
      if (len > longest) {
        longest = len;
        zstart = word - longest + 1;
      }
    }

    for (word = 0; word < 8; word++) {
      if (longest > 1) {
        // compress contiguous zeros - to produce "::"
        if (parts[word] === 0 && word >= zstart && word < (zstart + longest) ) {
          if (word === zstart) {
            str += ":";
            if (zstart === 0) str += ":"; //leading zeros case
          }
          continue;
        }
      }
      // converts 16-bit words from big-endian to little-endian before converting to hex string
      str += Number(_ntohs(parts[word] & 0xffff)).toString(16);
      str += word < 7 ? ":" : "";
    }
    return str;
  },

  $readSockaddr__deps: ['$Sockets', '$inetNtop4', '$inetNtop6', 'ntohs'],
  $readSockaddr: (sa, salen) => {
    // family / port offsets are common to both sockaddr_in and sockaddr_in6
    var family = {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in.sin_family, 'i16') }}};
    var port = _ntohs({{{ makeGetValue('sa', C_STRUCTS.sockaddr_in.sin_port, 'u16') }}});
    var addr;

    switch (family) {
      case {{{ cDefs.AF_INET }}}:
        if (salen !== {{{ C_STRUCTS.sockaddr_in.__size__ }}}) {
          return { errno: {{{ cDefs.EINVAL }}} };
        }
        addr = {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in.sin_addr.s_addr, 'i32') }}};
        addr = inetNtop4(addr);
        break;
      case {{{ cDefs.AF_INET6 }}}:
        if (salen !== {{{ C_STRUCTS.sockaddr_in6.__size__ }}}) {
          return { errno: {{{ cDefs.EINVAL }}} };
        }
        addr = [
          {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+0, 'i32') }}},
          {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+4, 'i32') }}},
          {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+8, 'i32') }}},
          {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+12, 'i32') }}}
        ];
        addr = inetNtop6(addr);
        break;
      default:
        return { errno: {{{ cDefs.EAFNOSUPPORT }}} };
    }

    return { family: family, addr: addr, port: port };
  },
  $writeSockaddr__docs: '/** @param {number=} addrlen */',
  $writeSockaddr__deps: ['$Sockets', '$inetPton4', '$inetPton6', '$zeroMemory', 'htons'],
  $writeSockaddr: (sa, family, addr, port, addrlen) => {
    switch (family) {
      case {{{ cDefs.AF_INET }}}:
        addr = inetPton4(addr);
        zeroMemory(sa, {{{ C_STRUCTS.sockaddr_in.__size__ }}});
        if (addrlen) {
          {{{ makeSetValue('addrlen', 0, C_STRUCTS.sockaddr_in.__size__, 'i32') }}};
        }
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in.sin_family, 'family', 'i16') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in.sin_addr.s_addr, 'addr', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in.sin_port, '_htons(port)', 'i16') }}};
        break;
      case {{{ cDefs.AF_INET6 }}}:
        addr = inetPton6(addr);
        zeroMemory(sa, {{{ C_STRUCTS.sockaddr_in6.__size__ }}});
        if (addrlen) {
          {{{ makeSetValue('addrlen', 0, C_STRUCTS.sockaddr_in6.__size__, 'i32') }}};
        }
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_family, 'family', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+0, 'addr[0]', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+4, 'addr[1]', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+8, 'addr[2]', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+12, 'addr[3]', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_port, '_htons(port)', 'i16') }}};
        break;
      default:
        return {{{ cDefs.EAFNOSUPPORT }}};
    }
    return 0;
  },

  // We can't actually resolve hostnames in the browser, so instead
  // we're generating fake IP addresses with lookup_name that we can
  // resolve later on with lookup_addr.
  // We do the aliasing in 172.29.*.*, giving us 65536 possibilities.
  $DNS__deps: ['$inetPton4', '$inetPton6'],
  $DNS: {
    address_map: {
      id: 1,
      addrs: {},
      names: {}
    },

    lookup_name(name) {
      // If the name is already a valid ipv4 / ipv6 address, don't generate a fake one.
      var res = inetPton4(name);
      if (res !== null) {
        return name;
      }
      res = inetPton6(name);
      if (res !== null) {
        return name;
      }

      // See if this name is already mapped.
      var addr;

      if (DNS.address_map.addrs[name]) {
        addr = DNS.address_map.addrs[name];
      } else {
        var id = DNS.address_map.id++;
        assert(id < 65535, 'exceeded max address mappings of 65535');

        addr = '172.29.' + (id & 0xff) + '.' + (id & 0xff00);

        DNS.address_map.names[addr] = name;
        DNS.address_map.addrs[name] = addr;
      }

      return addr;
    },

    lookup_addr(addr) {
      if (DNS.address_map.names[addr]) {
        return DNS.address_map.names[addr];
      }

      return null;
    }
  },

  _emscripten_lookup_name__deps: ['$UTF8ToString', '$DNS', '$inetPton4'],
  _emscripten_lookup_name: (name) => {
    // uint32_t _emscripten_lookup_name(const char *name);
    var nameString = UTF8ToString(name);
    return inetPton4(DNS.lookup_name(nameString));
  },

  getaddrinfo__deps: ['$Sockets', '$DNS', '$inetPton4', '$inetNtop4', '$inetPton6', '$inetNtop6', '$writeSockaddr', 'malloc', 'htonl'],
  getaddrinfo__proxy: 'sync',
  getaddrinfo: (node, service, hint, out) => {
    // Note getaddrinfo currently only returns a single addrinfo with ai_next defaulting to NULL. When NULL
    // hints are specified or ai_family set to AF_UNSPEC or ai_socktype or ai_protocol set to 0 then we
    // really should provide a linked list of suitable addrinfo values.
    var addrs = [];
    var canon = null;
    var addr = 0;
    var port = 0;
    var flags = 0;
    var family = {{{ cDefs.AF_UNSPEC }}};
    var type = 0;
    var proto = 0;
    var ai, last;

    function allocaddrinfo(family, type, proto, canon, addr, port) {
      var sa, salen, ai;
      var errno;

      salen = family === {{{ cDefs.AF_INET6 }}} ?
        {{{ C_STRUCTS.sockaddr_in6.__size__ }}} :
        {{{ C_STRUCTS.sockaddr_in.__size__ }}};
      addr = family === {{{ cDefs.AF_INET6 }}} ?
        inetNtop6(addr) :
        inetNtop4(addr);
      sa = _malloc(salen);
      errno = writeSockaddr(sa, family, addr, port);
      assert(!errno);

      ai = _malloc({{{ C_STRUCTS.addrinfo.__size__ }}});
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_family, 'family', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_socktype, 'type', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_protocol, 'proto', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_canonname, 'canon', '*') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_addr, 'sa', '*') }}};
      if (family === {{{ cDefs.AF_INET6 }}}) {
        {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_addrlen, C_STRUCTS.sockaddr_in6.__size__, 'i32') }}};
      } else {
        {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_addrlen, C_STRUCTS.sockaddr_in.__size__, 'i32') }}};
      }
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_next, '0', 'i32') }}};

      return ai;
    }

    if (hint) {
      flags = {{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_flags, 'i32') }}};
      family = {{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_family, 'i32') }}};
      type = {{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_socktype, 'i32') }}};
      proto = {{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_protocol, 'i32') }}};
    }
    if (type && !proto) {
      proto = type === {{{ cDefs.SOCK_DGRAM }}} ? {{{ cDefs.IPPROTO_UDP }}} : {{{ cDefs.IPPROTO_TCP }}};
    }
    if (!type && proto) {
      type = proto === {{{ cDefs.IPPROTO_UDP }}} ? {{{ cDefs.SOCK_DGRAM }}} : {{{ cDefs.SOCK_STREAM }}};
    }

    // If type or proto are set to zero in hints we should really be returning multiple addrinfo values, but for
    // now default to a TCP STREAM socket so we can at least return a sensible addrinfo given NULL hints.
    if (proto === 0) {
      proto = {{{ cDefs.IPPROTO_TCP }}};
    }
    if (type === 0) {
      type = {{{ cDefs.SOCK_STREAM }}};
    }

    if (!node && !service) {
      return {{{ cDefs.EAI_NONAME }}};
    }
    if (flags & ~({{{ cDefs.AI_PASSIVE }}}|{{{ cDefs.AI_CANONNAME }}}|{{{ cDefs.AI_NUMERICHOST }}}|
        {{{ cDefs.AI_NUMERICSERV }}}|{{{ cDefs.AI_V4MAPPED }}}|{{{ cDefs.AI_ALL }}}|{{{ cDefs.AI_ADDRCONFIG }}})) {
      return {{{ cDefs.EAI_BADFLAGS }}};
    }
    if (hint !== 0 && ({{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_flags, 'i32') }}} & {{{ cDefs.AI_CANONNAME }}}) && !node) {
      return {{{ cDefs.EAI_BADFLAGS }}};
    }
    if (flags & {{{ cDefs.AI_ADDRCONFIG }}}) {
      // TODO
      return {{{ cDefs.EAI_NONAME }}};
    }
    if (type !== 0 && type !== {{{ cDefs.SOCK_STREAM }}} && type !== {{{ cDefs.SOCK_DGRAM }}}) {
      return {{{ cDefs.EAI_SOCKTYPE }}};
    }
    if (family !== {{{ cDefs.AF_UNSPEC }}} && family !== {{{ cDefs.AF_INET }}} && family !== {{{ cDefs.AF_INET6 }}}) {
      return {{{ cDefs.EAI_FAMILY }}};
    }

    if (service) {
      service = UTF8ToString(service);
      port = parseInt(service, 10);

      if (isNaN(port)) {
        if (flags & {{{ cDefs.AI_NUMERICSERV }}}) {
          return {{{ cDefs.EAI_NONAME }}};
        }
        // TODO support resolving well-known service names from:
        // http://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.txt
        return {{{ cDefs.EAI_SERVICE }}};
      }
    }

    if (!node) {
      if (family === {{{ cDefs.AF_UNSPEC }}}) {
        family = {{{ cDefs.AF_INET }}};
      }
      if ((flags & {{{ cDefs.AI_PASSIVE }}}) === 0) {
        if (family === {{{ cDefs.AF_INET }}}) {
          addr = _htonl({{{ cDefs.INADDR_LOOPBACK }}});
        } else {
          addr = [0, 0, 0, _htonl(1)];
        }
      }
      ai = allocaddrinfo(family, type, proto, null, addr, port);
      {{{ makeSetValue('out', '0', 'ai', '*') }}};
      return 0;
    }

    //
    // try as a numeric address
    //
    node = UTF8ToString(node);
    addr = inetPton4(node);
    if (addr !== null) {
      // incoming node is a valid ipv4 address
      if (family === {{{ cDefs.AF_UNSPEC }}} || family === {{{ cDefs.AF_INET }}}) {
        family = {{{ cDefs.AF_INET }}};
      }
      else if (family === {{{ cDefs.AF_INET6 }}} && (flags & {{{ cDefs.AI_V4MAPPED }}})) {
        addr = [0, 0, _htonl(0xffff), addr];
        family = {{{ cDefs.AF_INET6 }}};
      } else {
        return {{{ cDefs.EAI_NONAME }}};
      }
    } else {
      addr = inetPton6(node);
      if (addr !== null) {
        // incoming node is a valid ipv6 address
        if (family === {{{ cDefs.AF_UNSPEC }}} || family === {{{ cDefs.AF_INET6 }}}) {
          family = {{{ cDefs.AF_INET6 }}};
        } else {
          return {{{ cDefs.EAI_NONAME }}};
        }
      }
    }
    if (addr != null) {
      ai = allocaddrinfo(family, type, proto, node, addr, port);
      {{{ makeSetValue('out', '0', 'ai', '*') }}};
      return 0;
    }
    if (flags & {{{ cDefs.AI_NUMERICHOST }}}) {
      return {{{ cDefs.EAI_NONAME }}};
    }

    //
    // try as a hostname
    //
    // resolve the hostname to a temporary fake address
    node = DNS.lookup_name(node);
    addr = inetPton4(node);
    if (family === {{{ cDefs.AF_UNSPEC }}}) {
      family = {{{ cDefs.AF_INET }}};
    } else if (family === {{{ cDefs.AF_INET6 }}}) {
      addr = [0, 0, _htonl(0xffff), addr];
    }
    ai = allocaddrinfo(family, type, proto, null, addr, port);
    {{{ makeSetValue('out', '0', 'ai', '*') }}};
    return 0;
  },

  getnameinfo__deps: ['$Sockets', '$DNS', '$readSockaddr', '$stringToUTF8'],
  getnameinfo: (sa, salen, node, nodelen, serv, servlen, flags) => {
    var info = readSockaddr(sa, salen);
    if (info.errno) {
      return {{{ cDefs.EAI_FAMILY }}};
    }
    var port = info.port;
    var addr = info.addr;

    var overflowed = false;

    if (node && nodelen) {
      var lookup;
      if ((flags & {{{ cDefs.NI_NUMERICHOST }}}) || !(lookup = DNS.lookup_addr(addr))) {
        if (flags & {{{ cDefs.NI_NAMEREQD }}}) {
          return {{{ cDefs.EAI_NONAME }}};
        }
      } else {
        addr = lookup;
      }
      var numBytesWrittenExclNull = stringToUTF8(addr, node, nodelen);

      if (numBytesWrittenExclNull+1 >= nodelen) {
        overflowed = true;
      }
    }

    if (serv && servlen) {
      port = '' + port;
      var numBytesWrittenExclNull = stringToUTF8(port, serv, servlen);

      if (numBytesWrittenExclNull+1 >= servlen) {
        overflowed = true;
      }
    }

    if (overflowed) {
      // Note: even when we overflow, getnameinfo() is specced to write out the truncated results.
      return {{{ cDefs.EAI_OVERFLOW }}};
    }

    return 0;
  },

  // Implement netdb.h protocol entry (getprotoent, getprotobyname, getprotobynumber, setprotoent, endprotoent)
  // http://pubs.opengroup.org/onlinepubs/9699919799/functions/getprotobyname.html
  // The Protocols object holds our 'fake' protocols 'database'.
  $Protocols: {
    list: [],
    map: {}
  },
  setprotoent__deps: ['$Protocols', '$stringToAscii', 'malloc'],
  setprotoent: (stayopen) => {
    // void setprotoent(int stayopen);

    // Allocate and populate a protoent structure given a name, protocol number and array of aliases
    function allocprotoent(name, proto, aliases) {
      // write name into buffer
      var nameBuf = _malloc(name.length + 1);
      stringToAscii(name, nameBuf);

      // write aliases into buffer
      var j = 0;
      var length = aliases.length;
      var aliasListBuf = _malloc((length + 1) * 4); // Use length + 1 so we have space for the terminating NULL ptr.

      for (var i = 0; i < length; i++, j += 4) {
        var alias = aliases[i];
        var aliasBuf = _malloc(alias.length + 1);
        stringToAscii(alias, aliasBuf);
        {{{ makeSetValue('aliasListBuf', 'j', 'aliasBuf', POINTER_TYPE) }}};
      }
      {{{ makeSetValue('aliasListBuf', 'j', '0', POINTER_TYPE) }}}; // Terminating NULL pointer.

      // generate protoent
      var pe = _malloc({{{ C_STRUCTS.protoent.__size__ }}});
      {{{ makeSetValue('pe', C_STRUCTS.protoent.p_name, 'nameBuf', POINTER_TYPE) }}};
      {{{ makeSetValue('pe', C_STRUCTS.protoent.p_aliases, 'aliasListBuf', POINTER_TYPE) }}};
      {{{ makeSetValue('pe', C_STRUCTS.protoent.p_proto, 'proto', 'i32') }}};
      return pe;
    };

    // Populate the protocol 'database'. The entries are limited to tcp and udp, though it is fairly trivial
    // to add extra entries from /etc/protocols if desired - though not sure if that'd actually be useful.
    var list = Protocols.list;
    var map  = Protocols.map;
    if (list.length === 0) {
        var entry = allocprotoent('tcp', 6, ['TCP']);
        list.push(entry);
        map['tcp'] = map['6'] = entry;
        entry = allocprotoent('udp', 17, ['UDP']);
        list.push(entry);
        map['udp'] = map['17'] = entry;
    }

    _setprotoent.index = 0;
  },

  endprotoent: () => {
    // void endprotoent(void);
    // We're not using a real protocol database so we don't do a real close.
  },

  getprotoent__deps: ['setprotoent', '$Protocols'],
  getprotoent: (number) => {
    // struct protoent *getprotoent(void);
    // reads the  next  entry  from  the  protocols 'database' or return NULL if 'eof'
    if (_setprotoent.index === Protocols.list.length) {
      return 0;
    }
    var result = Protocols.list[_setprotoent.index++];
    return result;
  },

  getprotobyname__deps: ['setprotoent', '$Protocols'],
  getprotobyname: (name) => {
    // struct protoent *getprotobyname(const char *);
    name = UTF8ToString(name);
    _setprotoent(true);
    var result = Protocols.map[name];
    return result;
  },

  getprotobynumber__deps: ['setprotoent', '$Protocols'],
  getprotobynumber: (number) => {
    // struct protoent *getprotobynumber(int proto);
    _setprotoent(true);
    var result = Protocols.map[number];
    return result;
  },

  // ==========================================================================
  // sockets. Note that the implementation assumes all sockets are always
  // nonblocking
  // ==========================================================================
#if SOCKET_WEBRTC
  $Sockets__deps: [
    () => 'var SocketIO = ' + read('../third_party/socket.io.js') + ';\n',
    () => 'var Peer = ' + read('../third_party/wrtcp.js') + ';\n'
  ],
#endif
  $Sockets: {
    BUFFER_SIZE: 10*1024, // initial size
    MAX_BUFFER_SIZE: 10*1024*1024, // maximum size we will grow the buffer

    nextFd: 1,
    fds: {},
    nextport: 1,
    maxport: 65535,
    peer: null,
    connections: {},
    portmap: {},
    localAddr: 0xfe00000a, // Local address is always 10.0.0.254
    addrPool: [            0x0200000a, 0x0300000a, 0x0400000a, 0x0500000a,
               0x0600000a, 0x0700000a, 0x0800000a, 0x0900000a, 0x0a00000a,
               0x0b00000a, 0x0c00000a, 0x0d00000a, 0x0e00000a] /* 0x0100000a is reserved */
  },

#endif // PROXY_POSIX_SOCKETS == 0

  // random.h

  $initRandomFill: () => {
    if (typeof crypto == 'object' && typeof crypto['getRandomValues'] == 'function') {
      // for modern web browsers
#if SHARED_MEMORY
      // like with most Web APIs, we can't use Web Crypto API directly on shared memory,
      // so we need to create an intermediate buffer and copy it to the destination
      return (view) => (
        view.set(crypto.getRandomValues(new Uint8Array(view.byteLength))),
        // Return the original view to match modern native implementations.
        view
      );
#else
      return (view) => crypto.getRandomValues(view);
#endif
    } else
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE) {
      // for nodejs with or without crypto support included
      try {
        var crypto_module = require('crypto');
        var randomFillSync = crypto_module['randomFillSync'];
        if (randomFillSync) {
          // nodejs with LTS crypto support
          return (view) => crypto_module['randomFillSync'](view);
        }
        // very old nodejs with the original crypto API
        var randomBytes = crypto_module['randomBytes'];
        return (view) => (
          view.set(randomBytes(view.byteLength)),
          // Return the original view to match modern native implementations.
          view
        );
      } catch (e) {
        // nodejs doesn't have crypto support
      }
    }
#endif // ENVIRONMENT_MAY_BE_NODE
    // we couldn't find a proper implementation, as Math.random() is not suitable for /dev/random, see emscripten-core/emscripten/pull/7096
#if ASSERTIONS
    abort('no cryptographic support found for randomDevice. consider polyfilling it if you want to use something insecure like Math.random(), e.g. put this in a --pre-js: var crypto = { getRandomValues: (array) => { for (var i = 0; i < array.length; i++) array[i] = (Math.random()*256)|0 } };');
#else
    abort('initRandomDevice');
#endif
  },

  $randomFill__deps: ['$initRandomFill'],
  $randomFill: (view) => {
    // Lazily init on the first invocation.
    return (randomFill = initRandomFill())(view);
  },

  getentropy__deps: ['$randomFill'],
  getentropy: (buffer, size) => {
    randomFill(HEAPU8.subarray(buffer, buffer + size));
    return 0;
  },

  $timers: {},

  // Helper function for setitimer that registers timers with the eventloop.
  // Timers always fire on the main thread, either directly from JS (here) or
  // or when the main thread is busy waiting calling _emscripten_yield.
  _setitimer_js__proxy: 'sync',
  _setitimer_js__deps: ['$timers', '$callUserCallback',
                        '_emscripten_timeout', 'emscripten_get_now'],
  _setitimer_js: (which, timeout_ms) => {
#if RUNTIME_DEBUG
    dbg(`setitimer_js ${which} timeout=${timeout_ms}`);
#endif
    // First, clear any existing timer.
    if (timers[which]) {
      clearTimeout(timers[which].id);
      delete timers[which];
    }

    // A timeout of zero simply cancels the current timeout so we have nothing
    // more to do.
    if (!timeout_ms) return 0;

    var id = setTimeout(() => {
#if ASSERTIONS
      assert(which in timers);
#endif
      delete timers[which];
#if RUNTIME_DEBUG
      dbg(`itimer fired: ${which}`);
#endif
      callUserCallback(() => __emscripten_timeout(which, _emscripten_get_now()));
    }, timeout_ms);
    timers[which] = { id, timeout_ms };
    return 0;
  },

  // Helper for raise() to avoid signature mismatch failures:
  // https://github.com/emscripten-core/posixtestsuite/issues/6
  __call_sighandler: (fp, sig) => {{{ makeDynCall('vi', 'fp') }}}(sig),

  // ==========================================================================
  // emscripten.h
  // ==========================================================================

  emscripten_run_script: (ptr) => {
    {{{ makeEval('eval(UTF8ToString(ptr));') }}}
  },

  emscripten_run_script_int__docs: '/** @suppress{checkTypes} */',
  emscripten_run_script_int: (ptr) => {
    {{{ makeEval('return eval(UTF8ToString(ptr))|0;') }}}
  },

  // Mark as `noleakcheck` otherwise lsan will report the last returned string
  // as a leak.
  emscripten_run_script_string__noleakcheck: true,
  emscripten_run_script_string__deps: ['$lengthBytesUTF8', '$stringToUTF8', 'malloc'],
  emscripten_run_script_string: (ptr) => {
    {{{ makeEval("var s = eval(UTF8ToString(ptr));") }}}
    if (s == null) {
      return 0;
    }
    s += '';
    var me = _emscripten_run_script_string;
    var len = lengthBytesUTF8(s);
    if (!me.bufferSize || me.bufferSize < len+1) {
      if (me.bufferSize) _free(me.buffer);
      me.bufferSize = len+1;
      me.buffer = _malloc(me.bufferSize);
    }
    stringToUTF8(s, me.buffer, me.bufferSize);
    return me.buffer;
  },

  emscripten_random: () => Math.random(),

#if PTHREADS && !AUDIO_WORKLET
  // Pthreads need their clocks synchronized to the execution of the main
  // thread, so, when using them, make sure to adjust all timings to the
  // respective time origins.
  emscripten_get_now: () => performance.timeOrigin + {{{ getPerformanceNow() }}}(),
#else
#if MIN_FIREFOX_VERSION <= 14 || MIN_CHROME_VERSION <= 23 || MIN_SAFARI_VERSION <= 80400 || AUDIO_WORKLET // https://caniuse.com/#feat=high-resolution-time
  emscripten_get_now: `;
    // AudioWorkletGlobalScope does not have performance.now()
    // (https://github.com/WebAudio/web-audio-api/issues/2527), so if building
    // with
    // Audio Worklets enabled, do a dynamic check for its presence.
    if (typeof performance != 'undefined' && {{{ getPerformanceNow() }}}) {
#if PTHREADS
      _emscripten_get_now = () => performance.timeOrigin + {{{ getPerformanceNow() }}}();
#else
      _emscripten_get_now = () => {{{ getPerformanceNow() }}}();
#endif
    } else {
      _emscripten_get_now = Date.now;
    }
`,
#else
  // Modern environment where performance.now() is supported:
  // N.B. a shorter form "_emscripten_get_now = performance.now;" is
  // unfortunately not allowed even in current browsers (e.g. FF Nightly 75).
  emscripten_get_now: () => {{{ getPerformanceNow() }}}(),
#endif
#endif

  emscripten_get_now_res: () => { // return resolution of get_now, in nanoseconds
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE) {
      return 1; // nanoseconds
    }
#endif
#if MIN_FIREFOX_VERSION <= 14 || MIN_CHROME_VERSION <= 23 || MIN_SAFARI_VERSION <= 80400 // https://caniuse.com/#feat=high-resolution-time
    if (typeof performance == 'object' && performance && typeof performance['now'] == 'function') {
      return 1000; // microseconds (1/1000 of a millisecond)
    }
    return 1000*1000; // milliseconds
#else
    // Modern environment where performance.now() is supported:
    return 1000; // microseconds (1/1000 of a millisecond)
#endif
  },

  // Represents whether emscripten_get_now is guaranteed monotonic; the Date.now
  // implementation is not :(
  $nowIsMonotonic__internal: true,
#if MIN_FIREFOX_VERSION <= 14 || MIN_CHROME_VERSION <= 23 || MIN_SAFARI_VERSION <= 80400 // https://caniuse.com/#feat=high-resolution-time
  $nowIsMonotonic: `
     ((typeof performance == 'object' && performance && typeof performance['now'] == 'function')
#if ENVIRONMENT_MAY_BE_NODE
      || ENVIRONMENT_IS_NODE
#endif
    );`,
#else
  // Modern environment where performance.now() is supported
  $nowIsMonotonic: 1,
#endif

  _emscripten_get_now_is_monotonic__internal: true,
  _emscripten_get_now_is_monotonic__deps: ['$nowIsMonotonic'],
  _emscripten_get_now_is_monotonic: () => nowIsMonotonic,

  $warnOnce: (text) => {
    warnOnce.shown ||= {};
    if (!warnOnce.shown[text]) {
      warnOnce.shown[text] = 1;
#if ENVIRONMENT_MAY_BE_NODE
      if (ENVIRONMENT_IS_NODE) text = 'warning: ' + text;
#endif
      err(text);
    }
  },

  $emscriptenLog__deps: ['$getCallstack'],
  $emscriptenLog: (flags, str) => {
    if (flags & {{{ cDefs.EM_LOG_C_STACK | cDefs.EM_LOG_JS_STACK }}}) {
      str = str.replace(/\s+$/, ''); // Ensure the message and the callstack are joined cleanly with exactly one newline.
      str += (str.length > 0 ? '\n' : '') + getCallstack(flags);
    }

    if (flags & {{{ cDefs.EM_LOG_CONSOLE }}}) {
      if (flags & {{{ cDefs.EM_LOG_ERROR }}}) {
        console.error(str);
      } else if (flags & {{{ cDefs.EM_LOG_WARN }}}) {
        console.warn(str);
      } else if (flags & {{{ cDefs.EM_LOG_INFO }}}) {
        console.info(str);
      } else if (flags & {{{ cDefs.EM_LOG_DEBUG }}}) {
        console.debug(str);
      } else {
        console.log(str);
      }
    } else if (flags & {{{ cDefs.EM_LOG_ERROR | cDefs.EM_LOG_WARN }}}) {
      err(str);
    } else {
      out(str);
    }
  },

  emscripten_log__deps: ['$formatString', '$emscriptenLog'],
  emscripten_log: (flags, format, varargs) => {
    var result = formatString(format, varargs);
    var str = UTF8ArrayToString(result);
    emscriptenLog(flags, str);
  },

  // We never free the return values of this function so we need to allocate
  // using builtin_malloc to avoid LSan reporting these as leaks.
  emscripten_get_compiler_setting__noleakcheck: true,
#if RETAIN_COMPILER_SETTINGS
  emscripten_get_compiler_setting__deps: ['$stringToNewUTF8'],
#endif
  emscripten_get_compiler_setting: (name) => {
#if RETAIN_COMPILER_SETTINGS
    name = UTF8ToString(name);

    var ret = getCompilerSetting(name);
    if (typeof ret == 'number' || typeof ret == 'boolean') return ret;

    var cache = _emscripten_get_compiler_setting.cache ??= {};
    var fullret = cache[name];
    if (fullret) return fullret;
    return cache[name] = stringToNewUTF8(ret);
#else
    throw 'You must build with -sRETAIN_COMPILER_SETTINGS for getCompilerSetting or emscripten_get_compiler_setting to work';
#endif
  },

  emscripten_has_asyncify: () => {{{ ASYNCIFY }}},

  emscripten_debugger: function() { debugger },

  emscripten_print_double__deps: ['$stringToUTF8', '$lengthBytesUTF8'],
  emscripten_print_double: (x, to, max) => {
    var str = x + '';
    if (to) return stringToUTF8(str, to, max);
    else return lengthBytesUTF8(str);
  },

#if USE_ASAN || USE_LSAN || UBSAN_RUNTIME
  // When lsan or asan is enabled withBuiltinMalloc temporarily replaces calls
  // to malloc, calloc, free, and memalign.
  $withBuiltinMalloc__deps: [
    'malloc', 'calloc', 'free', 'memalign',
    'emscripten_builtin_malloc', 'emscripten_builtin_free', 'emscripten_builtin_memalign', 'emscripten_builtin_calloc'
  ],
  $withBuiltinMalloc__docs: '/** @suppress{checkTypes} */',
  $withBuiltinMalloc: (func) => {
    var prev_malloc = typeof _malloc != 'undefined' ? _malloc : undefined;
    var prev_calloc = typeof _calloc != 'undefined' ? _calloc : undefined;
    var prev_memalign = typeof _memalign != 'undefined' ? _memalign : undefined;
    var prev_free = typeof _free != 'undefined' ? _free : undefined;
    _malloc = _emscripten_builtin_malloc;
    _calloc = _emscripten_builtin_calloc;
    _memalign = _emscripten_builtin_memalign;
    _free = _emscripten_builtin_free;
    try {
      return func();
    } finally {
      _malloc = prev_malloc;
      _calloc = prev_calloc;
      _memalign = prev_memalign;
      _free = prev_free;
    }
  },

  _emscripten_sanitizer_use_colors: () => {
    var setting = Module['printWithColors'];
    if (setting !== undefined) {
      return setting;
    }
    return ENVIRONMENT_IS_NODE && process.stderr.isTTY;
  },

  _emscripten_sanitizer_get_option__deps: ['$withBuiltinMalloc', '$stringToNewUTF8', '$UTF8ToString'],
  _emscripten_sanitizer_get_option__sig: 'pp',
  _emscripten_sanitizer_get_option: (name) => {
    return withBuiltinMalloc(() => stringToNewUTF8(Module[UTF8ToString(name)] || ""));
  },
#endif

  $readEmAsmArgsArray: '=[]',
  $readEmAsmArgs__deps: [
    '$readEmAsmArgsArray',
#if MEMORY64
    '$readI53FromI64',
#endif
  ],
  $readEmAsmArgs: (sigPtr, buf) => {
#if ASSERTIONS
    // Nobody should have mutated _readEmAsmArgsArray underneath us to be something else than an array.
    assert(Array.isArray(readEmAsmArgsArray));
    // The input buffer is allocated on the stack, so it must be stack-aligned.
    assert(buf % {{{ STACK_ALIGN }}} == 0);
#endif
    readEmAsmArgsArray.length = 0;
    var ch;
    // Most arguments are i32s, so shift the buffer pointer so it is a plain
    // index into HEAP32.
    while (ch = HEAPU8[sigPtr++]) {
#if ASSERTIONS
      var chr = String.fromCharCode(ch);
      var validChars = ['d', 'f', 'i', 'p'];
#if WASM_BIGINT
      // In WASM_BIGINT mode we support passing i64 values as bigint.
      validChars.push('j');
#endif
      assert(validChars.includes(chr), `Invalid character ${ch}("${chr}") in readEmAsmArgs! Use only [${validChars}], and do not specify "v" for void return argument.`);
#endif
      // Floats are always passed as doubles, so all types except for 'i'
      // are 8 bytes and require alignment.
      var wide = (ch != {{{ charCode('i') }}});
#if !MEMORY64
      wide &= (ch != {{{ charCode('p') }}});
#endif
      buf += wide && (buf % 8) ? 4 : 0;
      readEmAsmArgsArray.push(
        // Special case for pointers under wasm64 or CAN_ADDRESS_2GB mode.
        ch == {{{ charCode('p') }}} ? {{{ makeGetValue('buf', 0, '*') }}} :
#if WASM_BIGINT
        ch == {{{ charCode('j') }}} ? {{{ makeGetValue('buf', 0, 'i64') }}} :
#endif
        ch == {{{ charCode('i') }}} ?
          {{{ makeGetValue('buf', 0, 'i32') }}} :
          {{{ makeGetValue('buf', 0, 'double') }}}
      );
      buf += wide ? 8 : 4;
    }
    return readEmAsmArgsArray;
  },

#if HAVE_EM_ASM
  $runEmAsmFunction__deps: ['$readEmAsmArgs'],
  $runEmAsmFunction: (code, sigPtr, argbuf) => {
    var args = readEmAsmArgs(sigPtr, argbuf);
#if ASSERTIONS
    assert(ASM_CONSTS.hasOwnProperty(code), `No EM_ASM constant found at address ${code}.  The loaded WebAssembly file is likely out of sync with the generated JavaScript.`);
#endif
    return ASM_CONSTS[code](...args);
  },

  emscripten_asm_const_int__deps: ['$runEmAsmFunction'],
  emscripten_asm_const_int: (code, sigPtr, argbuf) => {
    return runEmAsmFunction(code, sigPtr, argbuf);
  },
  emscripten_asm_const_double__deps: ['$runEmAsmFunction'],
  emscripten_asm_const_double: (code, sigPtr, argbuf) => {
    return runEmAsmFunction(code, sigPtr, argbuf);
  },

  emscripten_asm_const_ptr__deps: ['$runEmAsmFunction'],
  emscripten_asm_const_ptr: (code, sigPtr, argbuf) => {
    return runEmAsmFunction(code, sigPtr, argbuf);
  },

  $runMainThreadEmAsm__deps: ['$readEmAsmArgs',
#if PTHREADS
    '$proxyToMainThread'
#endif
  ],
  $runMainThreadEmAsm: (emAsmAddr, sigPtr, argbuf, sync) => {
    var args = readEmAsmArgs(sigPtr, argbuf);
#if PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) {
      // EM_ASM functions are variadic, receiving the actual arguments as a buffer
      // in memory. the last parameter (argBuf) points to that data. We need to
      // always un-variadify that, *before proxying*, as in the async case this
      // is a stack allocation that LLVM made, which may go away before the main
      // thread gets the message. For that reason we handle proxying *after* the
      // call to readEmAsmArgs, and therefore we do that manually here instead
      // of using __proxy. (And dor simplicity, do the same in the sync
      // case as well, even though it's not strictly necessary, to keep the two
      // code paths as similar as possible on both sides.)
      return proxyToMainThread(0, emAsmAddr, sync, ...args);
    }
#endif
#if ASSERTIONS
    assert(ASM_CONSTS.hasOwnProperty(emAsmAddr), `No EM_ASM constant found at address ${emAsmAddr}.  The loaded WebAssembly file is likely out of sync with the generated JavaScript.`);
#endif
    return ASM_CONSTS[emAsmAddr](...args);
  },
  emscripten_asm_const_int_sync_on_main_thread__deps: ['$runMainThreadEmAsm'],
  emscripten_asm_const_int_sync_on_main_thread: (emAsmAddr, sigPtr, argbuf) => runMainThreadEmAsm(emAsmAddr, sigPtr, argbuf, 1),

  emscripten_asm_const_ptr_sync_on_main_thread__deps: ['$runMainThreadEmAsm'],
  emscripten_asm_const_ptr_sync_on_main_thread: (emAsmAddr, sigPtr, argbuf) => runMainThreadEmAsm(emAsmAddr, sigPtr, argbuf, 1),

  emscripten_asm_const_double_sync_on_main_thread: 'emscripten_asm_const_int_sync_on_main_thread',
  emscripten_asm_const_async_on_main_thread__deps: ['$runMainThreadEmAsm'],
  emscripten_asm_const_async_on_main_thread: (emAsmAddr, sigPtr, argbuf) => runMainThreadEmAsm(emAsmAddr, sigPtr, argbuf, 0),
#endif

#if !DECLARE_ASM_MODULE_EXPORTS
  // When DECLARE_ASM_MODULE_EXPORTS is not set we export native symbols
  // at runtime rather than statically in JS code.
  $exportWasmSymbols__deps: ['$asmjsMangle'],
  $exportWasmSymbols: (wasmExports) => {
#if ENVIRONMENT_MAY_BE_NODE && ENVIRONMENT_MAY_BE_WEB
    var global_object = (typeof process != "undefined" ? global : this);
#elif ENVIRONMENT_MAY_BE_NODE
    var global_object = global;
#else
    var global_object = this;
#endif

    for (var __exportedFunc in wasmExports) {
      var jsname = asmjsMangle(__exportedFunc);
#if MINIMAL_RUNTIME
      global_object[jsname] = wasmExports[__exportedFunc];
#else
      global_object[jsname] = Module[jsname] = wasmExports[__exportedFunc];
#endif
    }

  },
#endif

  // Parses as much of the given JS string to an integer, with quiet error
  // handling (returns a NaN on error). E.g. jstoi_q("123abc") returns 123.
  // Note that "smart" radix handling is employed for input string:
  // "0314" is parsed as octal, and "0x1234" is parsed as base-16.
  $jstoi_q__docs: '/** @suppress {checkTypes} */',
  $jstoi_q: (str) => parseInt(str),

  // Converts a JS string to an integer base-10, with signaling error
  // handling (throws a JS exception on error). E.g. jstoi_s("123abc")
  // throws an exception.
  $jstoi_s: 'Number',

#if LINK_AS_CXX
  // libunwind

  _Unwind_Backtrace__deps: ['$getCallstack'],
  _Unwind_Backtrace: (func, arg) => {
    var trace = getCallstack();
    var parts = trace.split('\n');
    for (var i = 0; i < parts.length; i++) {
      var ret = {{{ makeDynCall('iii', 'func') }}}(0, arg);
      if (ret !== 0) return;
    }
  },

  _Unwind_GetIPInfo: (context, ipBefore) => abort('Unwind_GetIPInfo'),

  _Unwind_FindEnclosingFunction: (ip) => 0, // we cannot succeed

  _Unwind_RaiseException__deps: ['__cxa_throw'],
  _Unwind_RaiseException: (ex) => {
    err('Warning: _Unwind_RaiseException is not correctly implemented');
    return ___cxa_throw(ex, 0, 0);
  },

  _Unwind_DeleteException: (ex) => {
    err('TODO: Unwind_DeleteException');
  },
#endif

  // special runtime support

#if STACK_OVERFLOW_CHECK
  // Used by wasm-emscripten-finalize to implement STACK_OVERFLOW_CHECK
  __handle_stack_overflow__deps: ['emscripten_stack_get_base', 'emscripten_stack_get_end', '$ptrToString'],
  __handle_stack_overflow: (requested) => {
    var base = _emscripten_stack_get_base();
    var end = _emscripten_stack_get_end();
    abort(`stack overflow (Attempt to set SP to ${ptrToString(requested)}` +
          `, with stack limits [${ptrToString(end)} - ${ptrToString(base)}` +
          ']). If you require more stack space build with -sSTACK_SIZE=<bytes>');
  },
#endif

  $getExecutableName: () => {
#if MINIMAL_RUNTIME // MINIMAL_RUNTIME does not have a global runtime variable thisProgram
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE && process.argv.length > 1) {
      return process.argv[1].replace(/\\/g, '/');
    }
#endif
    return "./this.program";
#else
    return thisProgram || './this.program';
#endif
  },

  $listenOnce: (object, event, func) => {
#if MIN_CHROME_VERSION < 55 || MIN_FIREFOX_VERSION < 50 // https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/addEventListener
    object.addEventListener(event, function handler() {
      func();
      object.removeEventListener(event, handler);
    });
#else
    object.addEventListener(event, func, { 'once': true });
#endif
  },

  // Receives a Web Audio context plus a set of elements to listen for user
  // input events on, and registers a context resume() for them. This lets
  // audio work properly in an automatic way, as browsers won't let audio run
  // without user interaction.
  // If @elements is not provided, we default to the document and canvas
  // elements, which handle common use cases.
  // TODO(sbc): Remove seemingly unused elements argument
  $autoResumeAudioContext__docs: '/** @param {Object=} elements */',
  $autoResumeAudioContext__deps: ['$listenOnce'],
  $autoResumeAudioContext: (ctx, elements) => {
    if (!elements) {
      elements = [document, document.getElementById('canvas')];
    }
    ['keydown', 'mousedown', 'touchstart'].forEach((event) => {
      elements.forEach((element) => {
        if (element) {
          listenOnce(element, event, () => {
            if (ctx.state === 'suspended') ctx.resume();
          });
        }
      });
    });
  },

#if DYNCALLS || !WASM_BIGINT
#if MINIMAL_RUNTIME
  $dynCalls: '{}',
#endif
  $dynCallLegacy__deps: [
#if MAIN_MODULE == 1
    '$createDyncallWrapper'
#endif
#if MINIMAL_RUNTIME
    '$dynCalls',
#endif
  ],
  $dynCallLegacy: (sig, ptr, args) => {
    sig = sig.replace(/p/g, {{{ MEMORY64 ? "'j'" : "'i'" }}})
#if ASSERTIONS
#if MINIMAL_RUNTIME
    assert(sig in dynCalls, `bad function pointer type - sig is not in dynCalls: '${sig}'`);
#else
    assert(('dynCall_' + sig) in Module, `bad function pointer type - dynCall function not found for sig '${sig}'`);
#endif
    if (args?.length) {
#if WASM_BIGINT
      // j (64-bit integer) is fine, and is implemented as a BigInt. Without
      // legalization, the number of parameters should match (j is not expanded
      // into two i's).
      assert(args.length === sig.length - 1);
#else
      // j (64-bit integer) must be passed in as two numbers [low 32, high 32].
      assert(args.length === sig.substring(1).replace(/j/g, '--').length);
#endif
    } else {
      assert(sig.length == 1);
    }
#endif
#if MINIMAL_RUNTIME
    var f = dynCalls[sig];
#else
#if MAIN_MODULE == 1
    if (!('dynCall_' + sig in Module)) {
      Module['dynCall_' + sig] = createDyncallWrapper(sig);
    }
#endif
    var f = Module['dynCall_' + sig];
#endif
    return f(ptr, ...args);
  },
  $dynCall__deps: ['$dynCallLegacy', '$getWasmTableEntry'],
#endif

  // Used in library code to get JS function from wasm function pointer.
  // All callers should use direct table access where possible and only fall
  // back to this function if needed.
  $getDynCaller__deps: ['$dynCall'],
  $getDynCaller: (sig, ptr) => {
#if ASSERTIONS && !DYNCALLS
    assert(sig.includes('j') || sig.includes('p'), 'getDynCaller should only be called with i64 sigs')
#endif
    return (...args) => dynCall(sig, ptr, args);
  },

  $dynCall: (sig, ptr, args = []) => {
#if MEMORY64
    // With MEMORY64 we have an additional step to convert `p` arguments to
    // bigint. This is the runtime equivalent of the wrappers we create for wasm
    // exports in `emscripten.py:create_wasm64_wrappers`.
    for (var i = 1; i < sig.length; ++i) {
      if (sig[i] == 'p') args[i-1] = BigInt(args[i-1]);
    }
#endif
#if DYNCALLS
    var rtn = dynCallLegacy(sig, ptr, args);
#else
#if !WASM_BIGINT
    // Without WASM_BIGINT support we cannot directly call function with i64 as
    // part of their signature, so we rely on the dynCall functions generated by
    // wasm-emscripten-finalize
    if (sig.includes('j')) {
      return dynCallLegacy(sig, ptr, args);
    }
#endif
#if ASSERTIONS
    assert(getWasmTableEntry(ptr), `missing table entry in dynCall: ${ptr}`);
#endif
    var rtn = getWasmTableEntry(ptr)(...args);
#endif
#if MEMORY64
    return sig[0] == 'p' ? Number(rtn) : rtn;
#elif CAN_ADDRESS_2GB
    return sig[0] == 'p' ? rtn >>> 0 : rtn;
#else
    return rtn;
#endif
  },

  $callRuntimeCallbacks__internal: true,
  $callRuntimeCallbacks: (callbacks) => {
    // Pass the module as the first argument.
    callbacks.forEach((f) => f(Module));
  },

#if SHRINK_LEVEL == 0 || ASYNCIFY == 2
  // A mirror copy of contents of wasmTable in JS side, to avoid relatively
  // slow wasmTable.get() call. Only used when not compiling with -Os, -Oz, or
  // JSPI which needs to instrument the functions.
  $wasmTableMirror__internal: true,
  $wasmTableMirror: [],

  $setWasmTableEntry__internal: true,
  $setWasmTableEntry__deps: ['$wasmTableMirror', '$wasmTable'],
  $setWasmTableEntry: (idx, func) => {
    wasmTable.set({{{ toIndexType('idx') }}}, func);
    // With ABORT_ON_WASM_EXCEPTIONS wasmTable.get is overridden to return wrapped
    // functions so we need to call it here to retrieve the potential wrapper correctly
    // instead of just storing 'func' directly into wasmTableMirror
    wasmTableMirror[idx] = wasmTable.get({{{ toIndexType('idx') }}});
  },

  $getWasmTableEntry__internal: true,
  $getWasmTableEntry__deps: ['$wasmTableMirror', '$wasmTable'],
  $getWasmTableEntry: (funcPtr) => {
#if MEMORY64
    // Function pointers should show up as numbers, even under wasm64, but
    // we still have some places where bigint values can flow here.
    // https://github.com/emscripten-core/emscripten/issues/18200
    funcPtr = Number(funcPtr);
#endif
    var func = wasmTableMirror[funcPtr];
    if (!func) {
      if (funcPtr >= wasmTableMirror.length) wasmTableMirror.length = funcPtr + 1;
      wasmTableMirror[funcPtr] = func = wasmTable.get({{{ toIndexType('funcPtr') }}});
#if ASYNCIFY == 2
      if (Asyncify.isAsyncExport(func)) {
        wasmTableMirror[funcPtr] = func = Asyncify.makeAsyncFunction(func);
      }
#endif
    }
#if ASSERTIONS && ASYNCIFY != 2 // With JSPI the function stored in the table will be a wrapper.
    assert(wasmTable.get({{{ toIndexType('funcPtr') }}}) == func, 'JavaScript-side Wasm function table mirror is out of date!');
#endif
    return func;
  },

#else

  $setWasmTableEntry__deps: ['$wasmTable'],
  $setWasmTableEntry: (idx, func) => wasmTable.set({{{ toIndexType('idx') }}}, func),

  $getWasmTableEntry__deps: ['$wasmTable'],
  $getWasmTableEntry: (funcPtr) => {
#if MEMORY64
    // Function pointers are 64-bit, but wasmTable.get() requires a Number.
    // https://github.com/emscripten-core/emscripten/issues/18200
    funcPtr = Number(funcPtr);
#endif
    // In -Os and -Oz builds, do not implement a JS side wasm table mirror for small
    // code size, but directly access wasmTable, which is a bit slower as uncached.
    return wasmTable.get({{{ toIndexType('funcPtr') }}});
  },
#endif // SHRINK_LEVEL == 0

  // Callable in pthread without __proxy needed.
  emscripten_exit_with_live_runtime: () => {
    {{{ runtimeKeepalivePush() }}}
    throw 'unwind';
  },

  _emscripten_runtime_keepalive_clear: () => {
#if isSymbolNeeded('$noExitRuntime')
    noExitRuntime = false;
#endif
#if !MINIMAL_RUNTIME
    runtimeKeepaliveCounter = 0;
#endif
  },

  emscripten_force_exit__deps: ['exit', '_emscripten_runtime_keepalive_clear',
#if !EXIT_RUNTIME && ASSERTIONS
    '$warnOnce',
#endif
#if !MINIMAL_RUNTIME
    '$runtimeKeepaliveCounter',
#endif
  ],
  emscripten_force_exit__proxy: 'sync',
  emscripten_force_exit: (status) => {
#if RUNTIME_DEBUG
    dbg('emscripten_force_exit');
#endif
#if !EXIT_RUNTIME && ASSERTIONS
    warnOnce('emscripten_force_exit cannot actually shut down the runtime, as the build does not have EXIT_RUNTIME set');
#endif
    __emscripten_runtime_keepalive_clear();
    _exit(status);
  },

  emscripten_out: (str) => out(UTF8ToString(str)),
  emscripten_outn: (str, len) => out(UTF8ToString(str, len)),

  emscripten_err: (str) => err(UTF8ToString(str)),
  emscripten_errn: (str, len) => err(UTF8ToString(str, len)),

#if ASSERTIONS || RUNTIME_DEBUG
  emscripten_dbg: (str) => dbg(UTF8ToString(str)),
  emscripten_dbgn: (str, len) => dbg(UTF8ToString(str, len)),

  emscripten_dbg_backtrace: (str) => {
    dbg(UTF8ToString(str) + '\n' + new Error().stack);
  },
#endif

  // Use program_invocation_short_name and program_invocation_name in compiled
  // programs. This function is for implementing them.
  _emscripten_get_progname__deps: ['$getExecutableName', '$stringToUTF8'],
  _emscripten_get_progname: (str, len) => {
    stringToUTF8(getExecutableName(), str, len);
  },

  emscripten_console_log: (str) => {
#if ASSERTIONS
    assert(typeof str == 'number');
#endif
    console.log(UTF8ToString(str));
  },

  emscripten_console_warn: (str) => {
#if ASSERTIONS
    assert(typeof str == 'number');
#endif
    console.warn(UTF8ToString(str));
  },

  emscripten_console_error: (str) => {
#if ASSERTIONS
    assert(typeof str == 'number');
#endif
    console.error(UTF8ToString(str));
  },

  emscripten_console_trace: (str) => {
#if ASSERTIONS
    assert(typeof str == 'number');
#endif
    console.trace(UTF8ToString(str));
  },

  emscripten_throw_number: (number) => {
    throw number;
  },

  emscripten_throw_string: (str) => {
#if ASSERTIONS
    assert(typeof str == 'number');
#endif
    throw UTF8ToString(str);
  },

#if !MINIMAL_RUNTIME
#if STACK_OVERFLOW_CHECK
  $handleException__deps: ['emscripten_stack_get_current'],
#endif
  $handleException: (e) => {
    // Certain exception types we do not treat as errors since they are used for
    // internal control flow.
    // 1. ExitStatus, which is thrown by exit()
    // 2. "unwind", which is thrown by emscripten_unwind_to_js_event_loop() and others
    //    that wish to return to JS event loop.
    if (e instanceof ExitStatus || e == 'unwind') {
#if RUNTIME_DEBUG
      dbg(`handleException: unwinding: EXITSTATUS=${EXITSTATUS}`);
#endif
      return EXITSTATUS;
    }
#if STACK_OVERFLOW_CHECK
    checkStackCookie();
    if (e instanceof WebAssembly.RuntimeError) {
      if (_emscripten_stack_get_current() <= 0) {
        err('Stack overflow detected.  You can try increasing -sSTACK_SIZE (currently set to {{{ STACK_SIZE }}})');
      }
    }
#endif
    quit_(1, e);
  },

  $runtimeKeepaliveCounter__internal: true,
  $runtimeKeepaliveCounter: 0,

  $keepRuntimeAlive__deps: ['$runtimeKeepaliveCounter'],
#if isSymbolNeeded('$noExitRuntime')
  $keepRuntimeAlive: () => noExitRuntime || runtimeKeepaliveCounter > 0,
#else
  $keepRuntimeAlive: () => runtimeKeepaliveCounter > 0,
#endif

  // Callable in pthread without __proxy needed.
  $runtimeKeepalivePush__deps: ['$runtimeKeepaliveCounter'],
  $runtimeKeepalivePush__sig: 'v',
  $runtimeKeepalivePush: () => {
    runtimeKeepaliveCounter += 1;
#if RUNTIME_DEBUG
    dbg(`runtimeKeepalivePush -> counter=${runtimeKeepaliveCounter}`);
#endif
  },

  $runtimeKeepalivePop__deps: ['$runtimeKeepaliveCounter'],
  $runtimeKeepalivePop__sig: 'v',
  $runtimeKeepalivePop: () => {
#if ASSERTIONS
    assert(runtimeKeepaliveCounter > 0);
#endif
    runtimeKeepaliveCounter -= 1;
#if RUNTIME_DEBUG
    dbg(`runtimeKeepalivePop -> counter=${runtimeKeepaliveCounter}`);
#endif
  },

  emscripten_runtime_keepalive_push: '$runtimeKeepalivePush',
  emscripten_runtime_keepalive_pop: '$runtimeKeepalivePop',
  emscripten_runtime_keepalive_check: '$keepRuntimeAlive',

  // Used to call user callbacks from the embedder / event loop.  For example
  // setTimeout or any other kind of event handler that calls into user case
  // needs to use this wrapper.
  //
  // The job of this wrapper is the handle emscripten-specific exceptions such
  // as ExitStatus and 'unwind' and prevent these from escaping to the top
  // level.
  $callUserCallback__deps: ['$handleException', '$maybeExit'],
  $callUserCallback: (func) => {
#if EXIT_RUNTIME
    if (runtimeExited || ABORT) {
#else
    if (ABORT) {
#endif
#if ASSERTIONS
      err('user callback triggered after runtime exited or application aborted.  Ignoring.');
#endif
      return;
    }
    try {
      func();
      maybeExit();
    } catch (e) {
      handleException(e);
    }
  },

  $maybeExit__deps: ['exit', '$handleException', '$keepRuntimeAlive',
#if PTHREADS
    '_emscripten_thread_exit',
#endif
#if RUNTIME_DEBUG
    '$runtimeKeepaliveCounter',
#endif
  ],
  $maybeExit: () => {
#if EXIT_RUNTIME
    if (runtimeExited) {
      return;
    }
#endif
#if RUNTIME_DEBUG
    dbg(`maybeExit: user callback done: runtimeKeepaliveCounter=${runtimeKeepaliveCounter}`);
#endif
    if (!keepRuntimeAlive()) {
#if RUNTIME_DEBUG
      dbg(`maybeExit: calling exit() implicitly after user callback completed: ${EXITSTATUS}`);
#endif
      try {
#if PTHREADS
        if (ENVIRONMENT_IS_PTHREAD) __emscripten_thread_exit(EXITSTATUS);
        else
#endif
        _exit(EXITSTATUS);
      } catch (e) {
        handleException(e);
      }
    }
  },

#else // MINIMAL_RUNTIME
  // MINIMAL_RUNTIME doesn't support the runtimeKeepalive stuff
  $callUserCallback: (func) => func(),
#endif // MINIMAL_RUNTIME

  $asmjsMangle: (x) => {
    if (x == '__main_argc_argv') {
      x = 'main';
    }
    return x.startsWith('dynCall_') ? x : '_' + x;
  },

  $asyncLoad__docs: '/** @param {boolean=} noRunDep */',
  $asyncLoad: (url, onload, onerror, noRunDep) => {
    var dep = !noRunDep ? getUniqueRunDependency(`al ${url}`) : '';
    readAsync(url).then(
      (arrayBuffer) => {
#if ASSERTIONS
        assert(arrayBuffer, `Loading data file "${url}" failed (no arrayBuffer).`);
#endif
        onload(new Uint8Array(arrayBuffer));
        if (dep) removeRunDependency(dep);
      },
      (err) => {
        if (onerror) {
          onerror();
        } else {
          throw `Loading data file "${url}" failed.`;
        }
      }
    );
    if (dep) addRunDependency(dep);
  },

  $alignMemory: (size, alignment) => {
#if ASSERTIONS
    assert(alignment, "alignment argument is required");
#endif
    return Math.ceil(size / alignment) * alignment;
  },

  // Allocate memory for an mmap operation. This allocates space of the right
  // page-aligned size, and clears the allocated space.
  $mmapAlloc__deps: ['$zeroMemory', '$alignMemory'],
  $mmapAlloc: (size) => {
#if hasExportedSymbol('emscripten_builtin_memalign')
    size = alignMemory(size, {{{ WASM_PAGE_SIZE }}});
    var ptr = _emscripten_builtin_memalign({{{ WASM_PAGE_SIZE }}}, size);
    if (ptr) zeroMemory(ptr, size);
    return ptr;
#elif ASSERTIONS
    abort('internal error: mmapAlloc called but `emscripten_builtin_memalign` native symbol not exported');
#else
    abort();
#endif
  },

#if RELOCATABLE
  // Globals that are normally exported from the wasm module but in relocatable
  // mode are created here and imported by the module.
  __stack_pointer: "new WebAssembly.Global({'value': '{{{ POINTER_WASM_TYPE }}}', 'mutable': true}, {{{ to64(STACK_HIGH) }}})",
  // tell the memory segments where to place themselves
  __memory_base: "new WebAssembly.Global({'value': '{{{ POINTER_WASM_TYPE }}}', 'mutable': false}, {{{ to64(GLOBAL_BASE) }}})",
  // the wasm backend reserves slot 0 for the NULL function pointer
  __table_base: "new WebAssembly.Global({'value': '{{{ POINTER_WASM_TYPE }}}', 'mutable': false}, {{{ to64(TABLE_BASE) }}})",
#if MEMORY64 == 2
  __memory_base32: "new WebAssembly.Global({'value': 'i32', 'mutable': false}, {{{ GLOBAL_BASE }}})",
#endif
#if MEMORY64
  __table_base32: {{{ TABLE_BASE }}},
#endif
  // To support such allocations during startup, track them on __heap_base and
  // then when the main module is loaded it reads that value and uses it to
  // initialize sbrk (the main module is relocatable itself, and so it does not
  // have __heap_base hardcoded into it - it receives it from JS as an extern
  // global, basically).
  __heap_base: '{{{ HEAP_BASE }}}',
  __stack_high: '{{{ STACK_HIGH }}}',
  __stack_low: '{{{ STACK_LOW }}}',
  __global_base: '{{{ GLOBAL_BASE }}}',
#if WASM_EXCEPTIONS
  // In dynamic linking we define tags here and feed them to each module
  __cpp_exception: "new WebAssembly.Tag({'parameters': ['{{{ POINTER_WASM_TYPE }}}']})",
#endif
#if SUPPORT_LONGJMP == 'wasm'
  __c_longjmp: "new WebAssembly.Tag({'parameters': ['{{{ POINTER_WASM_TYPE }}}']})",
#endif
#if ASYNCIFY == 1
  __asyncify_state: "new WebAssembly.Global({'value': 'i32', 'mutable': true}, 0)",
  __asyncify_data: "new WebAssembly.Global({'value': '{{{ POINTER_WASM_TYPE }}}', 'mutable': true}, {{{ to64(0) }}})",
#endif
#endif

  _emscripten_fs_load_embedded_files__deps: ['$FS', '$PATH'],
  _emscripten_fs_load_embedded_files: (ptr) => {
#if RUNTIME_DEBUG
    dbg('preloading data files');
#endif
    do {
      var name_addr = {{{ makeGetValue('ptr', '0', '*') }}};
      ptr += {{{ POINTER_SIZE }}};
      var len = {{{ makeGetValue('ptr', '0', '*') }}};
      ptr += {{{ POINTER_SIZE }}};
      var content = {{{ makeGetValue('ptr', '0', '*') }}};
      ptr += {{{ POINTER_SIZE }}};
      var name = UTF8ToString(name_addr)
#if RUNTIME_DEBUG
      dbg(`preloading files: ${name}`);
#endif
      FS.createPath('/', PATH.dirname(name), true, true);
      // canOwn this data in the filesystem, it is a slice of wasm memory that will never change
      FS.createDataFile(name, null, HEAP8.subarray(content, content + len), true, true, true);
    } while ({{{ makeGetValue('ptr', '0', '*') }}});
#if RUNTIME_DEBUG
    dbg('done preloading data files');
#endif
  },

  $HandleAllocator: class {
    constructor() {
      // TODO(https://github.com/emscripten-core/emscripten/issues/21414):
      // Use inline field declarations.
      this.allocated = [undefined];
      this.freelist = [];
    }
    get(id) {
#if ASSERTIONS
      assert(this.allocated[id] !== undefined, `invalid handle: ${id}`);
#endif
      return this.allocated[id];
    }
    has(id) {
      return this.allocated[id] !== undefined;
    }
    allocate(handle) {
      var id = this.freelist.pop() || this.allocated.length;
      this.allocated[id] = handle;
      return id;
    }
    free(id) {
#if ASSERTIONS
      assert(this.allocated[id] !== undefined);
#endif
      // Set the slot to `undefined` rather than using `delete` here since
      // apparently arrays with holes in them can be less efficient.
      this.allocated[id] = undefined;
      this.freelist.push(id);
    }
  },

  $getNativeTypeSize__deps: ['$POINTER_SIZE'],
  $getNativeTypeSize: {{{ getNativeTypeSize }}},

  $wasmTable__docs: '/** @type {WebAssembly.Table} */',
#if RELOCATABLE
  // In RELOCATABLE mode we create the table in JS.
  $wasmTable: `=new WebAssembly.Table({
  'initial': {{{ toIndexType(INITIAL_TABLE) }}},
#if !ALLOW_TABLE_GROWTH
  'maximum': {{{ toIndexType(INITIAL_TABLE) }}},
#endif
#if MEMORY64 == 1
  'index': 'i64',
#endif
  'element': 'anyfunc'
});
`,
#else
  $wasmTable: undefined,
#endif

  $noExitRuntime: "{{{ makeModuleReceiveExpr('noExitRuntime', !EXIT_RUNTIME) }}}",

  // We used to define these globals unconditionally in support code.
  // Instead, we now define them here so folks can pull it in explicitly, on
  // demand.
  $STACK_SIZE: {{{ STACK_SIZE }}},
  $STACK_ALIGN: {{{ STACK_ALIGN }}},
  $POINTER_SIZE: {{{ POINTER_SIZE }}},
  $ASSERTIONS: {{{ ASSERTIONS }}},
});

function autoAddDeps(object, name) {
  for (var item in object) {
    if (!item.endsWith('__deps')) {
      if (!object[item + '__deps']) {
        object[item + '__deps'] = [];
      }
      object[item + '__deps'].push(name);
    }
  }
}

#if LEGACY_RUNTIME
// Library functions that were previously included as runtime functions are
// automatically included when `LEGACY_RUNTIME` is set.
extraLibraryFuncs.push(
  '$addFunction',
  '$removeFunction',
  '$allocate',
  '$ALLOC_NORMAL',
  '$ALLOC_STACK',
  '$AsciiToString',
  '$stringToAscii',
  '$UTF16ToString',
  '$stringToUTF16',
  '$lengthBytesUTF16',
  '$UTF32ToString',
  '$stringToUTF32',
  '$lengthBytesUTF32',
  '$stringToNewUTF8',
  '$stringToUTF8OnStack',
  '$writeStringToMemory',
  '$writeArrayToMemory',
  '$writeAsciiToMemory',
  '$intArrayFromString',
  '$intArrayToString',
  '$warnOnce',
  '$ccall',
  '$cwrap',
  '$ExitStatus',
  '$UTF8ArrayToString',
  '$UTF8ToString',
  '$stringToUTF8Array',
  '$stringToUTF8',
  '$lengthBytesUTF8',
);
#endif

function wrapSyscallFunction(x, library, isWasi) {
  if (isJsOnlySymbol(x) || isDecorator(x)) {
    return;
  }

  var t = library[x];
  if (typeof t == 'string') return;
  t = t.toString();

  // If a syscall uses FS, but !SYSCALLS_REQUIRE_FILESYSTEM, then the user
  // has disabled the filesystem or we have proven some other way that this will
  // not be called in practice, and do not need that code.
  if (!SYSCALLS_REQUIRE_FILESYSTEM && t.includes('FS.')) {
    t = modifyJSFunction(t, (args, body) => {
      return `(${args}) => {\n` +
             (ASSERTIONS ? "abort('it should not be possible to operate on streams when !SYSCALLS_REQUIRE_FILESYSTEM');\n" : '') +
             '}';
    });
  }

  var isVariadic = !isWasi && t.includes(', varargs');
#if SYSCALLS_REQUIRE_FILESYSTEM == 0
  var canThrow = false;
#else
  var canThrow = library[x + '__nothrow'] !== true;
#endif

  library[x + '__deps'] ??= [];

#if PURE_WASI
  // In PURE_WASI mode we can't assume the wasm binary was built by emscripten
  // and politely notify us on memory growth.  Instead we have to check for
  // possible memory growth on each syscall.
  var pre = '\nif (!HEAPU8.byteLength) _emscripten_notify_memory_growth(0);\n'
  library[x + '__deps'].push('emscripten_notify_memory_growth');
#else
  var pre = '';
#endif
  var post = '';
  if (isVariadic) {
    pre += 'SYSCALLS.varargs = varargs;\n';
  }

#if SYSCALL_DEBUG
  if (isVariadic) {
    if (canThrow) {
      post += 'finally { SYSCALLS.varargs = undefined; }\n';
    } else {
      post += 'SYSCALLS.varargs = undefined;\n';
    }
  }
  pre += `dbg('syscall! ${x}: [' + Array.prototype.slice.call(arguments) + ']');\n`;
  pre += "var canWarn = true;\n";
  pre += "var ret = (() => {";
  post += "})();\n";
  post += "if (ret && ret < 0 && canWarn) {\n";
  post += "  dbg(`error: syscall may have failed with ${-ret} (${strError(-ret)})`);\n";
  post += "}\n";
  post += "dbg(`syscall return: ${ret}`);\n";
  post += "return ret;\n";
#endif
  delete library[x + '__nothrow'];
  var handler = '';
  if (canThrow) {
    pre += 'try {\n';
    handler +=
    "} catch (e) {\n" +
    "  if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;\n";
#if SYSCALL_DEBUG
    handler +=
    "  dbg(`error: syscall failed with ${e.errno} (${strError(e.errno)})`);\n" +
    "  canWarn = false;\n";
#endif
    // Musl syscalls are negated.
    if (isWasi) {
      handler += "  return e.errno;\n";
    } else {
      // Musl syscalls are negated.
      handler += "  return -e.errno;\n";
    }
    handler += "}\n";
  }
  post = handler + post;

  if (pre || post) {
    t = modifyJSFunction(t, (args, body) => `function (${args}) {\n${pre}${body}${post}}\n`);
  }

  library[x] = eval('(' + t + ')');
  // Automatically add dependency on `$SYSCALLS`
  if (!WASMFS && t.includes('SYSCALLS')) {
    library[x + '__deps'].push('$SYSCALLS');
  }
#if PTHREADS
  // Most syscalls need to happen on the main JS thread (e.g. because the
  // filesystem is in JS and on that thread). Proxy synchronously to there.
  // There are some exceptions, syscalls that we know are ok to just run in
  // any thread; those are marked as not being proxied with
  //  __proxy: false
  // A syscall without a return value could perhaps be proxied asynchronously
  // instead of synchronously, and marked with
  //  __proxy: 'async'
  // (but essentially all syscalls do have return values).
  if (library[x + '__proxy'] === undefined) {
    library[x + '__proxy'] = 'sync';
  }
#endif
}
