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

mergeInto(LibraryManager.library, {
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
    return address;
  },

#if SAFE_HEAP
  // Trivial wrappers around runtime functions that make these symbols available
  // to native code.
  segfault: () => segfault(),
  alignfault: () => alignfault(),
#endif

  // ==========================================================================
  // JavaScript <-> C string interop
  // ==========================================================================

#if !MINIMAL_RUNTIME
  $exitJS__docs: '/** @param {boolean|number=} implicit */',
  $exitJS__deps: ['proc_exit'],
  $exitJS: (status, implicit) => {
    EXITSTATUS = status;

#if ASSERTIONS && !EXIT_RUNTIME
    checkUnflushedContent();
#endif // ASSERTIONS && !EXIT_RUNTIME

#if PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) {
      // implict exit can never happen on a pthread
#if ASSERTIONS
      assert(!implicit);
#endif
#if PTHREADS_DEBUG
      dbg(`Pthread ${ptrToString(_pthread_self())} called exit(), posting exitOnMainThread.`);
#endif
      // When running in a pthread we propagate the exit back to the main thread
      // where it can decide if the whole process should be shut down or not.
      // The pthread may have decided not to exit its own runtime, for example
      // because it runs a main loop, but that doesn't affect the main thread.
      exitOnMainThread(status);
      throw 'unwind';
    }
#if PTHREADS_DEBUG
    err(`main thread called exit: keepRuntimeAlive=${keepRuntimeAlive()} (counter=${runtimeKeepaliveCounter})`);
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

#if TEST_MEMORY_GROWTH_FAILS
  $growMemory: (size) => false,
#else

  // Grows the wasm memory to the given byte size, and updates the JS views to
  // it. Returns 1 on success, 0 on error.
  $growMemory: (size) => {
    var b = wasmMemory.buffer;
    var pages = (size - b.byteLength + 65535) >>> 16;
#if RUNTIME_DEBUG
    dbg(`emscripten_resize_heap: ${size} (+${size - b.byteLength} bytes / ${pages} pages)`);
#endif
#if MEMORYPROFILER
    var oldHeapSize = b.byteLength;
#endif
    try {
      // round size grow request up to wasm page size (fixed 64KB per spec)
      wasmMemory.grow(pages); // .grow() takes a delta compared to the previous size
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
#endif // ~TEST_MEMORY_GROWTH_FAILS

  emscripten_resize_heap__deps: [
    '$getHeapMax',
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
      err(`Cannot enlarge memory, asked to go up to ${requestedSize} bytes, but the limit is ${maxHeapSize} bytes!`);
#endif
#if ABORTING_MALLOC
      abortOnCannotGrowMemory(requestedSize);
#else
      return false;
#endif
    }

    var alignUp = (x, multiple) => x + (multiple - x % multiple) % multiple;

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

      var newSize = Math.min(maxHeapSize, alignUp(Math.max(requestedSize, overGrownHeapSize), {{{ WASM_PAGE_SIZE }}}));

#if ASSERTIONS == 2
      var t0 = _emscripten_get_now();
#endif
      var replacement = growMemory(newSize);
#if ASSERTIONS == 2
      var t1 = _emscripten_get_now();
      out(`Heap resize call from ${oldSize} to ${newSize} took ${(t1 - t0)} msecs. Success: ${!!replacement}`);
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

  system__deps: ['$setErrNo'],
  system: (command) => {
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
            case 'SIGHUP': return 1;
            case 'SIGINT': return 2;
            case 'SIGQUIT': return 3;
            case 'SIGFPE': return 8;
            case 'SIGKILL': return 9;
            case 'SIGALRM': return 14;
            case 'SIGTERM': return 15;
          }
          return 2; // SIGINT
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
    setErrNo({{{ cDefs.ENOSYS }}});
    return -1;
  },

  // ==========================================================================
  // stdlib.h
  // ==========================================================================

  // TODO: There are currently two abort() functions that get imported to asm
  // module scope: the built-in runtime function abort(), and this library
  // function _abort(). Remove one of these, importing two functions for the
  // same purpose is wasteful.
  abort: () => {
#if ASSERTIONS
    abort('native code called abort()');
#else
    abort('');
#endif
  },

  // This object can be modified by the user during startup, which affects
  // the initial values of the environment accessible by getenv.
  $ENV: {},

  getloadavg: (loadavg, nelem) => {
    // int getloadavg(double loadavg[], int nelem);
    // http://linux.die.net/man/3/getloadavg
    var limit = Math.min(nelem, 3);
    var doubleSize = {{{ getNativeTypeSize('double') }}};
    for (var i = 0; i < limit; i++) {
      {{{ makeSetValue('loadavg', 'i * doubleSize', '0.1', 'double') }}};
    }
    return limit;
  },

  // In -Oz builds, we replace memcpy() altogether with a non-unrolled wasm
  // variant, so we should never emit emscripten_memcpy_big() in the build.
  // In STANDALONE_WASM we avoid the emscripten_memcpy_big dependency so keep
  // the wasm file standalone.
  // In BULK_MEMORY mode we include native versions of these functions based
  // on memory.fill and memory.copy.
  // In MAIN_MODULE=1 or EMCC_FORCE_STDLIBS mode all of libc is force included
  // so we cannot override parts of it, and therefore cannot use libc_optz.
#if (SHRINK_LEVEL < 2 || LINKABLE || process.env.EMCC_FORCE_STDLIBS) && !STANDALONE_WASM && !BULK_MEMORY

#if MIN_CHROME_VERSION < 45 || MIN_EDGE_VERSION < 14 || MIN_FIREFOX_VERSION < 34 || MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_SAFARI_VERSION < 100101
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
  emscripten_memcpy_big: `= Uint8Array.prototype.copyWithin
    ? (dest, src, num) => HEAPU8.copyWithin(dest, src, src + num)
    : (dest, src, num) => HEAPU8.set(HEAPU8.subarray(src, src+num), dest)`,
#else
  emscripten_memcpy_big: (dest, src, num) => HEAPU8.copyWithin(dest, src, src + num),
#endif

#endif

  // ==========================================================================
  // assert.h
  // ==========================================================================

  __assert_fail: (condition, filename, line, func) => {
    abort(`Assertion failed: ${UTF8ToString(condition)}, at: ` + [filename ? UTF8ToString(filename) : 'unknown filename', line, func ? UTF8ToString(func) : 'unknown function']);
  },

  // ==========================================================================
  // time.h
  // ==========================================================================

  _mktime_js__i53abi: true,
  _mktime_js__deps: ['$ydayFromDate'],
  _mktime_js: (tmPtr) => {
    var date = new Date({{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_year, 'i32') }}} + 1900,
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_min, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'i32') }}},
                        0);

    // There's an ambiguous hour when the time goes back; the tm_isdst field is
    // used to disambiguate it.  Date() basically guesses, so we fix it up if it
    // guessed wrong, or fill in tm_isdst with the guess if it's -1.
    var dst = {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'i32') }}};
    var guessedOffset = date.getTimezoneOffset();
    var start = new Date(date.getFullYear(), 0, 1);
    var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
    var winterOffset = start.getTimezoneOffset();
    var dstOffset = Math.min(winterOffset, summerOffset); // DST is in December in South
    if (dst < 0) {
      // Attention: some regions don't have DST at all.
      {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'Number(summerOffset != winterOffset && dstOffset == guessedOffset)', 'i32') }}};
    } else if ((dst > 0) != (dstOffset == guessedOffset)) {
      var nonDstOffset = Math.max(winterOffset, summerOffset);
      var trueOffset = dst > 0 ? dstOffset : nonDstOffset;
      // Don't try setMinutes(date.getMinutes() + ...) -- it's messed up.
      date.setTime(date.getTime() + (trueOffset - guessedOffset)*60000);
    }

    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getDay()', 'i32') }}};
    var yday = ydayFromDate(date)|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
    // To match expected behavior, update fields from date
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getMonth()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_year, 'date.getYear()', 'i32') }}};

    return date.getTime() / 1000;
  },

  _gmtime_js__i53abi: true,
  _gmtime_js: (time, tmPtr) => {
    var date = new Date(time * 1000);
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getUTCSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getUTCMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getUTCHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getUTCDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getUTCMonth()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_year, 'date.getUTCFullYear()-1900', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getUTCDay()', 'i32') }}};
    var start = Date.UTC(date.getUTCFullYear(), 0, 1, 0, 0, 0, 0);
    var yday = ((date.getTime() - start) / (1000 * 60 * 60 * 24))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
  },

  _timegm_js__i53abi: true,
  _timegm_js: (tmPtr) => {
    var time = Date.UTC({{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_year, 'i32') }}} + 1900,
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_min, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'i32') }}},
                        0);
    var date = new Date(time);

    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getUTCDay()', 'i32') }}};
    var start = Date.UTC(date.getUTCFullYear(), 0, 1, 0, 0, 0, 0);
    var yday = ((date.getTime() - start) / (1000 * 60 * 60 * 24))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};

    return date.getTime() / 1000;
  },

  _localtime_js__i53abi: true,
  _localtime_js__deps: ['$ydayFromDate'],
  _localtime_js: (time, tmPtr) => {
    var date = new Date(time*1000);
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getMonth()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_year, 'date.getFullYear()-1900', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getDay()', 'i32') }}};

    var yday = ydayFromDate(date)|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_gmtoff, '-(date.getTimezoneOffset() * 60)', 'i32') }}};

    // Attention: DST is in December in South, and some regions don't have DST at all.
    var start = new Date(date.getFullYear(), 0, 1);
    var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
    var winterOffset = start.getTimezoneOffset();
    var dst = (summerOffset != winterOffset && date.getTimezoneOffset() == Math.min(winterOffset, summerOffset))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'dst', 'i32') }}};
  },

  // musl-internal function used to implement both `asctime` and `asctime_r`
  __asctime_r: (tmPtr, buf) => {
    var date = {
      tm_sec: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'i32') }}},
      tm_min: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_min, 'i32') }}},
      tm_hour: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'i32') }}},
      tm_mday: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'i32') }}},
      tm_mon: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'i32') }}},
      tm_year: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_year, 'i32') }}},
      tm_wday: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'i32') }}}
    };
    var days = [ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" ];
    var months = [ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" ];
    var s = days[date.tm_wday] + ' ' + months[date.tm_mon] +
        (date.tm_mday < 10 ? '  ' : ' ') + date.tm_mday +
        (date.tm_hour < 10 ? ' 0' : ' ') + date.tm_hour +
        (date.tm_min < 10 ? ':0' : ':') + date.tm_min +
        (date.tm_sec < 10 ? ':0' : ':') + date.tm_sec +
        ' ' + (1900 + date.tm_year) + "\n";

    // asctime_r is specced to behave in an undefined manner if the algorithm would attempt
    // to write out more than 26 bytes (including the null terminator).
    // See http://pubs.opengroup.org/onlinepubs/9699919799/functions/asctime.html
    // Our undefined behavior is to truncate the write to at most 26 bytes, including null terminator.
    stringToUTF8(s, buf, 26);
    return buf;
  },

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
  $withStackSave: (f) => {
    var stack = stackSave();
    var ret = f();
    stackRestore(stack);
    return ret;
  },

  _tzset_js__deps: ['$stringToNewUTF8'],
  _tzset_js__internal: true,
  _tzset_js: (timezone, daylight, tzname) => {
    // TODO: Use (malleable) environment variables instead of system settings.
    var currentYear = new Date().getFullYear();
    var winter = new Date(currentYear, 0, 1);
    var summer = new Date(currentYear, 6, 1);
    var winterOffset = winter.getTimezoneOffset();
    var summerOffset = summer.getTimezoneOffset();

    // Local standard timezone offset. Local standard time is not adjusted for daylight savings.
    // This code uses the fact that getTimezoneOffset returns a greater value during Standard Time versus Daylight Saving Time (DST).
    // Thus it determines the expected output during Standard Time, and it compares whether the output of the given date the same (Standard) or less (DST).
    var stdTimezoneOffset = Math.max(winterOffset, summerOffset);

    // timezone is specified as seconds west of UTC ("The external variable
    // `timezone` shall be set to the difference, in seconds, between
    // Coordinated Universal Time (UTC) and local standard time."), the same
    // as returned by stdTimezoneOffset.
    // See http://pubs.opengroup.org/onlinepubs/009695399/functions/tzset.html
    {{{ makeSetValue('timezone', '0', 'stdTimezoneOffset * 60', POINTER_TYPE) }}};

    {{{ makeSetValue('daylight', '0', 'Number(winterOffset != summerOffset)', 'i32') }}};

    function extractZone(date) {
      var match = date.toTimeString().match(/\(([A-Za-z ]+)\)$/);
      return match ? match[1] : "GMT";
    };
    var winterName = extractZone(winter);
    var summerName = extractZone(summer);
    var winterNamePtr = stringToNewUTF8(winterName);
    var summerNamePtr = stringToNewUTF8(summerName);
    if (summerOffset < winterOffset) {
      // Northern hemisphere
      {{{ makeSetValue('tzname', '0', 'winterNamePtr', POINTER_TYPE) }}};
      {{{ makeSetValue('tzname', POINTER_SIZE, 'summerNamePtr', POINTER_TYPE) }}};
    } else {
      {{{ makeSetValue('tzname', '0', 'summerNamePtr', POINTER_TYPE) }}};
      {{{ makeSetValue('tzname', POINTER_SIZE, 'winterNamePtr', POINTER_TYPE) }}};
    }
  },

  $MONTH_DAYS_REGULAR: [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],
  $MONTH_DAYS_LEAP: [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],
  $MONTH_DAYS_REGULAR_CUMULATIVE: [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334],
  $MONTH_DAYS_LEAP_CUMULATIVE: [0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335],

  $isLeapYear: (year) => {
      return year%4 === 0 && (year%100 !== 0 || year%400 === 0);
  },

  $ydayFromDate__deps: ['$isLeapYear', '$MONTH_DAYS_LEAP_CUMULATIVE', '$MONTH_DAYS_REGULAR_CUMULATIVE'],
  $ydayFromDate: (date) => {
    var leap = isLeapYear(date.getFullYear());
    var monthDaysCumulative = (leap ? MONTH_DAYS_LEAP_CUMULATIVE : MONTH_DAYS_REGULAR_CUMULATIVE);
    var yday = monthDaysCumulative[date.getMonth()] + date.getDate() - 1; // -1 since it's days since Jan 1

    return yday;
  },

  $arraySum: (array, index) => {
    var sum = 0;
    for (var i = 0; i <= index; sum += array[i++]) {
      // no-op
    }
    return sum;
  },

  $addDays__deps: ['$isLeapYear', '$MONTH_DAYS_LEAP', '$MONTH_DAYS_REGULAR'],
  $addDays: (date, days) => {
    var newDate = new Date(date.getTime());
    while (days > 0) {
      var leap = isLeapYear(newDate.getFullYear());
      var currentMonth = newDate.getMonth();
      var daysInCurrentMonth = (leap ? MONTH_DAYS_LEAP : MONTH_DAYS_REGULAR)[currentMonth];

      if (days > daysInCurrentMonth-newDate.getDate()) {
        // we spill over to next month
        days -= (daysInCurrentMonth-newDate.getDate()+1);
        newDate.setDate(1);
        if (currentMonth < 11) {
          newDate.setMonth(currentMonth+1)
        } else {
          newDate.setMonth(0);
          newDate.setFullYear(newDate.getFullYear()+1);
        }
      } else {
        // we stay in current month
        newDate.setDate(newDate.getDate()+days);
        return newDate;
      }
    }

    return newDate;
  },

  // Note: this is not used in STANDALONE_WASM mode, because it is more
  //       compact to do it in JS.
  strftime__deps: ['$isLeapYear', '$arraySum', '$addDays', '$MONTH_DAYS_REGULAR', '$MONTH_DAYS_LEAP',
                   '$intArrayFromString', '$writeArrayToMemory'
  ],
  strftime: (s, maxsize, format, tm) => {
    // size_t strftime(char *restrict s, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/strftime.html

    var tm_zone = {{{ makeGetValue('tm', C_STRUCTS.tm.tm_zone, 'i32') }}};

    var date = {
      tm_sec: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_sec, 'i32') }}},
      tm_min: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_min, 'i32') }}},
      tm_hour: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_hour, 'i32') }}},
      tm_mday: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_mday, 'i32') }}},
      tm_mon: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_mon, 'i32') }}},
      tm_year: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_year, 'i32') }}},
      tm_wday: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_wday, 'i32') }}},
      tm_yday: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_yday, 'i32') }}},
      tm_isdst: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_isdst, 'i32') }}},
      tm_gmtoff: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_gmtoff, 'i32') }}},
      tm_zone: tm_zone ? UTF8ToString(tm_zone) : ''
    };

    var pattern = UTF8ToString(format);

    // expand format
    var EXPANSION_RULES_1 = {
      '%c': '%a %b %d %H:%M:%S %Y',     // Replaced by the locale's appropriate date and time representation - e.g., Mon Aug  3 14:02:01 2013
      '%D': '%m/%d/%y',                 // Equivalent to %m / %d / %y
      '%F': '%Y-%m-%d',                 // Equivalent to %Y - %m - %d
      '%h': '%b',                       // Equivalent to %b
      '%r': '%I:%M:%S %p',              // Replaced by the time in a.m. and p.m. notation
      '%R': '%H:%M',                    // Replaced by the time in 24-hour notation
      '%T': '%H:%M:%S',                 // Replaced by the time
      '%x': '%m/%d/%y',                 // Replaced by the locale's appropriate date representation
      '%X': '%H:%M:%S',                 // Replaced by the locale's appropriate time representation
      // Modified Conversion Specifiers
      '%Ec': '%c',                      // Replaced by the locale's alternative appropriate date and time representation.
      '%EC': '%C',                      // Replaced by the name of the base year (period) in the locale's alternative representation.
      '%Ex': '%m/%d/%y',                // Replaced by the locale's alternative date representation.
      '%EX': '%H:%M:%S',                // Replaced by the locale's alternative time representation.
      '%Ey': '%y',                      // Replaced by the offset from %EC (year only) in the locale's alternative representation.
      '%EY': '%Y',                      // Replaced by the full alternative year representation.
      '%Od': '%d',                      // Replaced by the day of the month, using the locale's alternative numeric symbols, filled as needed with leading zeros if there is any alternative symbol for zero; otherwise, with leading <space> characters.
      '%Oe': '%e',                      // Replaced by the day of the month, using the locale's alternative numeric symbols, filled as needed with leading <space> characters.
      '%OH': '%H',                      // Replaced by the hour (24-hour clock) using the locale's alternative numeric symbols.
      '%OI': '%I',                      // Replaced by the hour (12-hour clock) using the locale's alternative numeric symbols.
      '%Om': '%m',                      // Replaced by the month using the locale's alternative numeric symbols.
      '%OM': '%M',                      // Replaced by the minutes using the locale's alternative numeric symbols.
      '%OS': '%S',                      // Replaced by the seconds using the locale's alternative numeric symbols.
      '%Ou': '%u',                      // Replaced by the weekday as a number in the locale's alternative representation (Monday=1).
      '%OU': '%U',                      // Replaced by the week number of the year (Sunday as the first day of the week, rules corresponding to %U ) using the locale's alternative numeric symbols.
      '%OV': '%V',                      // Replaced by the week number of the year (Monday as the first day of the week, rules corresponding to %V ) using the locale's alternative numeric symbols.
      '%Ow': '%w',                      // Replaced by the number of the weekday (Sunday=0) using the locale's alternative numeric symbols.
      '%OW': '%W',                      // Replaced by the week number of the year (Monday as the first day of the week) using the locale's alternative numeric symbols.
      '%Oy': '%y',                      // Replaced by the year (offset from %C ) using the locale's alternative numeric symbols.
    };
    for (var rule in EXPANSION_RULES_1) {
      pattern = pattern.replace(new RegExp(rule, 'g'), EXPANSION_RULES_1[rule]);
    }

    var WEEKDAYS = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
    var MONTHS = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'];

    function leadingSomething(value, digits, character) {
      var str = typeof value == 'number' ? value.toString() : (value || '');
      while (str.length < digits) {
        str = character[0]+str;
      }
      return str;
    }

    function leadingNulls(value, digits) {
      return leadingSomething(value, digits, '0');
    }

    function compareByDay(date1, date2) {
      function sgn(value) {
        return value < 0 ? -1 : (value > 0 ? 1 : 0);
      }

      var compare;
      if ((compare = sgn(date1.getFullYear()-date2.getFullYear())) === 0) {
        if ((compare = sgn(date1.getMonth()-date2.getMonth())) === 0) {
          compare = sgn(date1.getDate()-date2.getDate());
        }
      }
      return compare;
    }

    function getFirstWeekStartDate(janFourth) {
        switch (janFourth.getDay()) {
          case 0: // Sunday
            return new Date(janFourth.getFullYear()-1, 11, 29);
          case 1: // Monday
            return janFourth;
          case 2: // Tuesday
            return new Date(janFourth.getFullYear(), 0, 3);
          case 3: // Wednesday
            return new Date(janFourth.getFullYear(), 0, 2);
          case 4: // Thursday
            return new Date(janFourth.getFullYear(), 0, 1);
          case 5: // Friday
            return new Date(janFourth.getFullYear()-1, 11, 31);
          case 6: // Saturday
            return new Date(janFourth.getFullYear()-1, 11, 30);
        }
    }

    function getWeekBasedYear(date) {
        var thisDate = addDays(new Date(date.tm_year+1900, 0, 1), date.tm_yday);

        var janFourthThisYear = new Date(thisDate.getFullYear(), 0, 4);
        var janFourthNextYear = new Date(thisDate.getFullYear()+1, 0, 4);

        var firstWeekStartThisYear = getFirstWeekStartDate(janFourthThisYear);
        var firstWeekStartNextYear = getFirstWeekStartDate(janFourthNextYear);

        if (compareByDay(firstWeekStartThisYear, thisDate) <= 0) {
          // this date is after the start of the first week of this year
          if (compareByDay(firstWeekStartNextYear, thisDate) <= 0) {
            return thisDate.getFullYear()+1;
          }
          return thisDate.getFullYear();
        }
        return thisDate.getFullYear()-1;
    }

    var EXPANSION_RULES_2 = {
      '%a': (date) => WEEKDAYS[date.tm_wday].substring(0,3) ,
      '%A': (date) => WEEKDAYS[date.tm_wday],
      '%b': (date) => MONTHS[date.tm_mon].substring(0,3),
      '%B': (date) => MONTHS[date.tm_mon],
      '%C': (date) => {
        var year = date.tm_year+1900;
        return leadingNulls((year/100)|0,2);
      },
      '%d': (date) => leadingNulls(date.tm_mday, 2),
      '%e': (date) => leadingSomething(date.tm_mday, 2, ' '),
      '%g': (date) => {
        // %g, %G, and %V give values according to the ISO 8601:2000 standard week-based year.
        // In this system, weeks begin on a Monday and week 1 of the year is the week that includes
        // January 4th, which is also the week that includes the first Thursday of the year, and
        // is also the first week that contains at least four days in the year.
        // If the first Monday of January is the 2nd, 3rd, or 4th, the preceding days are part of
        // the last week of the preceding year; thus, for Saturday 2nd January 1999,
        // %G is replaced by 1998 and %V is replaced by 53. If December 29th, 30th,
        // or 31st is a Monday, it and any following days are part of week 1 of the following year.
        // Thus, for Tuesday 30th December 1997, %G is replaced by 1998 and %V is replaced by 01.

        return getWeekBasedYear(date).toString().substring(2);
      },
      '%G': (date) => getWeekBasedYear(date),
      '%H': (date) => leadingNulls(date.tm_hour, 2),
      '%I': (date) => {
        var twelveHour = date.tm_hour;
        if (twelveHour == 0) twelveHour = 12;
        else if (twelveHour > 12) twelveHour -= 12;
        return leadingNulls(twelveHour, 2);
      },
      '%j': (date) => {
        // Day of the year (001-366)
        return leadingNulls(date.tm_mday + arraySum(isLeapYear(date.tm_year+1900) ? MONTH_DAYS_LEAP : MONTH_DAYS_REGULAR, date.tm_mon-1), 3);
      },
      '%m': (date) => leadingNulls(date.tm_mon+1, 2),
      '%M': (date) => leadingNulls(date.tm_min, 2),
      '%n': () => '\n',
      '%p': (date) => {
        if (date.tm_hour >= 0 && date.tm_hour < 12) {
          return 'AM';
        }
        return 'PM';
      },
      '%S': (date) => leadingNulls(date.tm_sec, 2),
      '%t': () => '\t',
      '%u': (date) => date.tm_wday || 7,
      '%U': (date) => {
        var days = date.tm_yday + 7 - date.tm_wday;
        return leadingNulls(Math.floor(days / 7), 2);
      },
      '%V': (date) => {
        // Replaced by the week number of the year (Monday as the first day of the week)
        // as a decimal number [01,53]. If the week containing 1 January has four
        // or more days in the new year, then it is considered week 1.
        // Otherwise, it is the last week of the previous year, and the next week is week 1.
        // Both January 4th and the first Thursday of January are always in week 1. [ tm_year, tm_wday, tm_yday]
        var val = Math.floor((date.tm_yday + 7 - (date.tm_wday + 6) % 7 ) / 7);
        // If 1 Jan is just 1-3 days past Monday, the previous week
        // is also in this year.
        if ((date.tm_wday + 371 - date.tm_yday - 2) % 7 <= 2) {
          val++;
        }
        if (!val) {
          val = 52;
          // If 31 December of prev year a Thursday, or Friday of a
          // leap year, then the prev year has 53 weeks.
          var dec31 = (date.tm_wday + 7 - date.tm_yday - 1) % 7;
          if (dec31 == 4 || (dec31 == 5 && isLeapYear(date.tm_year%400-1))) {
            val++;
          }
        } else if (val == 53) {
          // If 1 January is not a Thursday, and not a Wednesday of a
          // leap year, then this year has only 52 weeks.
          var jan1 = (date.tm_wday + 371 - date.tm_yday) % 7;
          if (jan1 != 4 && (jan1 != 3 || !isLeapYear(date.tm_year)))
            val = 1;
        }
        return leadingNulls(val, 2);
      },
      '%w': (date) => date.tm_wday,
      '%W': (date) => {
        var days = date.tm_yday + 7 - ((date.tm_wday + 6) % 7);
        return leadingNulls(Math.floor(days / 7), 2);
      },
      '%y': (date) => {
        // Replaced by the last two digits of the year as a decimal number [00,99]. [ tm_year]
        return (date.tm_year+1900).toString().substring(2);
      },
      // Replaced by the year as a decimal number (for example, 1997). [ tm_year]
      '%Y': (date) => date.tm_year+1900,
      '%z': (date) => {
        // Replaced by the offset from UTC in the ISO 8601:2000 standard format ( +hhmm or -hhmm ).
        // For example, "-0430" means 4 hours 30 minutes behind UTC (west of Greenwich).
        var off = date.tm_gmtoff;
        var ahead = off >= 0;
        off = Math.abs(off) / 60;
        // convert from minutes into hhmm format (which means 60 minutes = 100 units)
        off = (off / 60)*100 + (off % 60);
        return (ahead ? '+' : '-') + String("0000" + off).slice(-4);
      },
      '%Z': (date) => date.tm_zone,
      '%%': () => '%'
    };

    // Replace %% with a pair of NULLs (which cannot occur in a C string), then
    // re-inject them after processing.
    pattern = pattern.replace(/%%/g, '\0\0')
    for (var rule in EXPANSION_RULES_2) {
      if (pattern.includes(rule)) {
        pattern = pattern.replace(new RegExp(rule, 'g'), EXPANSION_RULES_2[rule](date));
      }
    }
    pattern = pattern.replace(/\0\0/g, '%')

    var bytes = intArrayFromString(pattern, false);
    if (bytes.length > maxsize) {
      return 0;
    }

    writeArrayToMemory(bytes, s);
    return bytes.length-1;
  },
  strftime_l__deps: ['strftime'],
  strftime_l: (s, maxsize, format, tm, loc) => {
    return _strftime(s, maxsize, format, tm); // no locale support yet
  },

  strptime__deps: ['$isLeapYear', '$arraySum', '$addDays', '$MONTH_DAYS_REGULAR', '$MONTH_DAYS_LEAP',
                   '$jstoi_q', '$intArrayFromString' ],
  strptime: (buf, format, tm) => {
    // char *strptime(const char *restrict buf, const char *restrict format, struct tm *restrict tm);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/strptime.html
    var pattern = UTF8ToString(format);

    // escape special characters
    // TODO: not sure we really need to escape all of these in JS regexps
    var SPECIAL_CHARS = '\\!@#$^&*()+=-[]/{}|:<>?,.';
    for (var i=0, ii=SPECIAL_CHARS.length; i<ii; ++i) {
      pattern = pattern.replace(new RegExp('\\'+SPECIAL_CHARS[i], 'g'), '\\'+SPECIAL_CHARS[i]);
    }

    // reduce number of matchers
    var EQUIVALENT_MATCHERS = {
      '%A':  '%a',
      '%B':  '%b',
      '%c':  '%a %b %d %H:%M:%S %Y',
      '%D':  '%m\\/%d\\/%y',
      '%e':  '%d',
      '%F':  '%Y-%m-%d',
      '%h':  '%b',
      '%R':  '%H\\:%M',
      '%r':  '%I\\:%M\\:%S\\s%p',
      '%T':  '%H\\:%M\\:%S',
      '%x':  '%m\\/%d\\/(?:%y|%Y)',
      '%X':  '%H\\:%M\\:%S'
    };
    for (var matcher in EQUIVALENT_MATCHERS) {
      pattern = pattern.replace(matcher, EQUIVALENT_MATCHERS[matcher]);
    }

    // TODO: take care of locale

    var DATE_PATTERNS = {
      /* weeday name */     '%a': '(?:Sun(?:day)?)|(?:Mon(?:day)?)|(?:Tue(?:sday)?)|(?:Wed(?:nesday)?)|(?:Thu(?:rsday)?)|(?:Fri(?:day)?)|(?:Sat(?:urday)?)',
      /* month name */      '%b': '(?:Jan(?:uary)?)|(?:Feb(?:ruary)?)|(?:Mar(?:ch)?)|(?:Apr(?:il)?)|May|(?:Jun(?:e)?)|(?:Jul(?:y)?)|(?:Aug(?:ust)?)|(?:Sep(?:tember)?)|(?:Oct(?:ober)?)|(?:Nov(?:ember)?)|(?:Dec(?:ember)?)',
      /* century */         '%C': '\\d\\d',
      /* day of month */    '%d': '0[1-9]|[1-9](?!\\d)|1\\d|2\\d|30|31',
      /* hour (24hr) */     '%H': '\\d(?!\\d)|[0,1]\\d|20|21|22|23',
      /* hour (12hr) */     '%I': '\\d(?!\\d)|0\\d|10|11|12',
      /* day of year */     '%j': '00[1-9]|0?[1-9](?!\\d)|0?[1-9]\\d(?!\\d)|[1,2]\\d\\d|3[0-6]\\d',
      /* month */           '%m': '0[1-9]|[1-9](?!\\d)|10|11|12',
      /* minutes */         '%M': '0\\d|\\d(?!\\d)|[1-5]\\d',
      /* whitespace */      '%n': '\\s',
      /* AM/PM */           '%p': 'AM|am|PM|pm|A\\.M\\.|a\\.m\\.|P\\.M\\.|p\\.m\\.',
      /* seconds */         '%S': '0\\d|\\d(?!\\d)|[1-5]\\d|60',
      /* week number */     '%U': '0\\d|\\d(?!\\d)|[1-4]\\d|50|51|52|53',
      /* week number */     '%W': '0\\d|\\d(?!\\d)|[1-4]\\d|50|51|52|53',
      /* weekday number */  '%w': '[0-6]',
      /* 2-digit year */    '%y': '\\d\\d',
      /* 4-digit year */    '%Y': '\\d\\d\\d\\d',
      /* % */               '%%': '%',
      /* whitespace */      '%t': '\\s',
    };

    var MONTH_NUMBERS = {JAN: 0, FEB: 1, MAR: 2, APR: 3, MAY: 4, JUN: 5, JUL: 6, AUG: 7, SEP: 8, OCT: 9, NOV: 10, DEC: 11};
    var DAY_NUMBERS_SUN_FIRST = {SUN: 0, MON: 1, TUE: 2, WED: 3, THU: 4, FRI: 5, SAT: 6};
    var DAY_NUMBERS_MON_FIRST = {MON: 0, TUE: 1, WED: 2, THU: 3, FRI: 4, SAT: 5, SUN: 6};

    for (var datePattern in DATE_PATTERNS) {
      pattern = pattern.replace(datePattern, '('+datePattern+DATE_PATTERNS[datePattern]+')');
    }

    // take care of capturing groups
    var capture = [];
    for (var i=pattern.indexOf('%'); i>=0; i=pattern.indexOf('%')) {
      capture.push(pattern[i+1]);
      pattern = pattern.replace(new RegExp('\\%'+pattern[i+1], 'g'), '');
    }

    var matches = new RegExp('^'+pattern, "i").exec(UTF8ToString(buf))
    // out(UTF8ToString(buf)+ ' is matched by '+((new RegExp('^'+pattern)).source)+' into: '+JSON.stringify(matches));

    function initDate() {
      function fixup(value, min, max) {
        return (typeof value != 'number' || isNaN(value)) ? min : (value>=min ? (value<=max ? value: max): min);
      };
      return {
        year: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_year, 'i32') }}} + 1900 , 1970, 9999),
        month: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_mon, 'i32') }}}, 0, 11),
        day: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_mday, 'i32') }}}, 1, 31),
        hour: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_hour, 'i32') }}}, 0, 23),
        min: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_min, 'i32') }}}, 0, 59),
        sec: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_sec, 'i32') }}}, 0, 59)
      };
    };

    if (matches) {
      var date = initDate();
      var value;

      var getMatch = (symbol) => {
        var pos = capture.indexOf(symbol);
        // check if symbol appears in regexp
        if (pos >= 0) {
          // return matched value or null (falsy!) for non-matches
          return matches[pos+1];
        }
        return;
      };

      // seconds
      if ((value=getMatch('S'))) {
        date.sec = jstoi_q(value);
      }

      // minutes
      if ((value=getMatch('M'))) {
        date.min = jstoi_q(value);
      }

      // hours
      if ((value=getMatch('H'))) {
        // 24h clock
        date.hour = jstoi_q(value);
      } else if ((value = getMatch('I'))) {
        // AM/PM clock
        var hour = jstoi_q(value);
        if ((value=getMatch('p'))) {
          hour += value.toUpperCase()[0] === 'P' ? 12 : 0;
        }
        date.hour = hour;
      }

      // year
      if ((value=getMatch('Y'))) {
        // parse from four-digit year
        date.year = jstoi_q(value);
      } else if ((value=getMatch('y'))) {
        // parse from two-digit year...
        var year = jstoi_q(value);
        if ((value=getMatch('C'))) {
          // ...and century
          year += jstoi_q(value)*100;
        } else {
          // ...and rule-of-thumb
          year += year<69 ? 2000 : 1900;
        }
        date.year = year;
      }

      // month
      if ((value=getMatch('m'))) {
        // parse from month number
        date.month = jstoi_q(value)-1;
      } else if ((value=getMatch('b'))) {
        // parse from month name
        date.month = MONTH_NUMBERS[value.substring(0,3).toUpperCase()] || 0;
        // TODO: derive month from day in year+year, week number+day of week+year
      }

      // day
      if ((value=getMatch('d'))) {
        // get day of month directly
        date.day = jstoi_q(value);
      } else if ((value=getMatch('j'))) {
        // get day of month from day of year ...
        var day = jstoi_q(value);
        var leapYear = isLeapYear(date.year);
        for (var month=0; month<12; ++month) {
          var daysUntilMonth = arraySum(leapYear ? MONTH_DAYS_LEAP : MONTH_DAYS_REGULAR, month-1);
          if (day<=daysUntilMonth+(leapYear ? MONTH_DAYS_LEAP : MONTH_DAYS_REGULAR)[month]) {
            date.day = day-daysUntilMonth;
          }
        }
      } else if ((value=getMatch('a'))) {
        // get day of month from weekday ...
        var weekDay = value.substring(0,3).toUpperCase();
        if ((value=getMatch('U'))) {
          // ... and week number (Sunday being first day of week)
          // Week number of the year (Sunday as the first day of the week) as a decimal number [00,53].
          // All days in a new year preceding the first Sunday are considered to be in week 0.
          var weekDayNumber = DAY_NUMBERS_SUN_FIRST[weekDay];
          var weekNumber = jstoi_q(value);

          // January 1st
          var janFirst = new Date(date.year, 0, 1);
          var endDate;
          if (janFirst.getDay() === 0) {
            // Jan 1st is a Sunday, and, hence in the 1st CW
            endDate = addDays(janFirst, weekDayNumber+7*(weekNumber-1));
          } else {
            // Jan 1st is not a Sunday, and, hence still in the 0th CW
            endDate = addDays(janFirst, 7-janFirst.getDay()+weekDayNumber+7*(weekNumber-1));
          }
          date.day = endDate.getDate();
          date.month = endDate.getMonth();
        } else if ((value=getMatch('W'))) {
          // ... and week number (Monday being first day of week)
          // Week number of the year (Monday as the first day of the week) as a decimal number [00,53].
          // All days in a new year preceding the first Monday are considered to be in week 0.
          var weekDayNumber = DAY_NUMBERS_MON_FIRST[weekDay];
          var weekNumber = jstoi_q(value);

          // January 1st
          var janFirst = new Date(date.year, 0, 1);
          var endDate;
          if (janFirst.getDay()===1) {
            // Jan 1st is a Monday, and, hence in the 1st CW
             endDate = addDays(janFirst, weekDayNumber+7*(weekNumber-1));
          } else {
            // Jan 1st is not a Monday, and, hence still in the 0th CW
            endDate = addDays(janFirst, 7-janFirst.getDay()+1+weekDayNumber+7*(weekNumber-1));
          }

          date.day = endDate.getDate();
          date.month = endDate.getMonth();
        }
      }

      /*
      tm_sec  int seconds after the minute  0-61*
      tm_min  int minutes after the hour  0-59
      tm_hour int hours since midnight  0-23
      tm_mday int day of the month  1-31
      tm_mon  int months since January  0-11
      tm_year int years since 1900
      tm_wday int days since Sunday 0-6
      tm_yday int days since January 1  0-365
      tm_isdst  int Daylight Saving Time flag
      */

      var fullDate = new Date(date.year, date.month, date.day, date.hour, date.min, date.sec, 0);
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_sec, 'fullDate.getSeconds()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_min, 'fullDate.getMinutes()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_hour, 'fullDate.getHours()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_mday, 'fullDate.getDate()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_mon, 'fullDate.getMonth()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_year, 'fullDate.getFullYear()-1900', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_wday, 'fullDate.getDay()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_yday, 'arraySum(isLeapYear(fullDate.getFullYear()) ? MONTH_DAYS_LEAP : MONTH_DAYS_REGULAR, fullDate.getMonth()-1)+fullDate.getDate()-1', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_isdst, '0', 'i32') }}};

      // we need to convert the matched sequence into an integer array to take care of UTF-8 characters > 0x7F
      // TODO: not sure that intArrayFromString handles all unicode characters correctly
      return buf+intArrayFromString(matches[0]).length-1;
    }

    return 0;
  },
  strptime_l__deps: ['strptime'],
  strptime_l: (buf, format, tm, locale) => {
    return _strptime(buf, format, tm); // no locale support yet
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

  $ERRNO_CODES__postset: `ERRNO_CODES = {
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
  };`,
  $ERRNO_CODES: {},
  $ERRNO_MESSAGES: {
    0: 'Success',
    {{{ cDefs.EPERM }}}: 'Not super-user',
    {{{ cDefs.ENOENT }}}: 'No such file or directory',
    {{{ cDefs.ESRCH }}}: 'No such process',
    {{{ cDefs.EINTR }}}: 'Interrupted system call',
    {{{ cDefs.EIO }}}: 'I/O error',
    {{{ cDefs.ENXIO }}}: 'No such device or address',
    {{{ cDefs.E2BIG }}}: 'Arg list too long',
    {{{ cDefs.ENOEXEC }}}: 'Exec format error',
    {{{ cDefs.EBADF }}}: 'Bad file number',
    {{{ cDefs.ECHILD }}}: 'No children',
    {{{ cDefs.EWOULDBLOCK }}}: 'No more processes',
    {{{ cDefs.ENOMEM }}}: 'Not enough core',
    {{{ cDefs.EACCES }}}: 'Permission denied',
    {{{ cDefs.EFAULT }}}: 'Bad address',
    {{{ cDefs.ENOTBLK }}}: 'Block device required',
    {{{ cDefs.EBUSY }}}: 'Mount device busy',
    {{{ cDefs.EEXIST }}}: 'File exists',
    {{{ cDefs.EXDEV }}}: 'Cross-device link',
    {{{ cDefs.ENODEV }}}: 'No such device',
    {{{ cDefs.ENOTDIR }}}: 'Not a directory',
    {{{ cDefs.EISDIR }}}: 'Is a directory',
    {{{ cDefs.EINVAL }}}: 'Invalid argument',
    {{{ cDefs.ENFILE }}}: 'Too many open files in system',
    {{{ cDefs.EMFILE }}}: 'Too many open files',
    {{{ cDefs.ENOTTY }}}: 'Not a typewriter',
    {{{ cDefs.ETXTBSY }}}: 'Text file busy',
    {{{ cDefs.EFBIG }}}: 'File too large',
    {{{ cDefs.ENOSPC }}}: 'No space left on device',
    {{{ cDefs.ESPIPE }}}: 'Illegal seek',
    {{{ cDefs.EROFS }}}: 'Read only file system',
    {{{ cDefs.EMLINK }}}: 'Too many links',
    {{{ cDefs.EPIPE }}}: 'Broken pipe',
    {{{ cDefs.EDOM }}}: 'Math arg out of domain of func',
    {{{ cDefs.ERANGE }}}: 'Math result not representable',
    {{{ cDefs.ENOMSG }}}: 'No message of desired type',
    {{{ cDefs.EIDRM }}}: 'Identifier removed',
    {{{ cDefs.ECHRNG }}}: 'Channel number out of range',
    {{{ cDefs.EL2NSYNC }}}: 'Level 2 not synchronized',
    {{{ cDefs.EL3HLT }}}: 'Level 3 halted',
    {{{ cDefs.EL3RST }}}: 'Level 3 reset',
    {{{ cDefs.ELNRNG }}}: 'Link number out of range',
    {{{ cDefs.EUNATCH }}}: 'Protocol driver not attached',
    {{{ cDefs.ENOCSI }}}: 'No CSI structure available',
    {{{ cDefs.EL2HLT }}}: 'Level 2 halted',
    {{{ cDefs.EDEADLK }}}: 'Deadlock condition',
    {{{ cDefs.ENOLCK }}}: 'No record locks available',
    {{{ cDefs.EBADE }}}: 'Invalid exchange',
    {{{ cDefs.EBADR }}}: 'Invalid request descriptor',
    {{{ cDefs.EXFULL }}}: 'Exchange full',
    {{{ cDefs.ENOANO }}}: 'No anode',
    {{{ cDefs.EBADRQC }}}: 'Invalid request code',
    {{{ cDefs.EBADSLT }}}: 'Invalid slot',
    {{{ cDefs.EDEADLOCK }}}: 'File locking deadlock error',
    {{{ cDefs.EBFONT }}}: 'Bad font file fmt',
    {{{ cDefs.ENOSTR }}}: 'Device not a stream',
    {{{ cDefs.ENODATA }}}: 'No data (for no delay io)',
    {{{ cDefs.ETIME }}}: 'Timer expired',
    {{{ cDefs.ENOSR }}}: 'Out of streams resources',
    {{{ cDefs.ENONET }}}: 'Machine is not on the network',
    {{{ cDefs.ENOPKG }}}: 'Package not installed',
    {{{ cDefs.EREMOTE }}}: 'The object is remote',
    {{{ cDefs.ENOLINK }}}: 'The link has been severed',
    {{{ cDefs.EADV }}}: 'Advertise error',
    {{{ cDefs.ESRMNT }}}: 'Srmount error',
    {{{ cDefs.ECOMM }}}: 'Communication error on send',
    {{{ cDefs.EPROTO }}}: 'Protocol error',
    {{{ cDefs.EMULTIHOP }}}: 'Multihop attempted',
    {{{ cDefs.EDOTDOT }}}: 'Cross mount point (not really error)',
    {{{ cDefs.EBADMSG }}}: 'Trying to read unreadable message',
    {{{ cDefs.ENOTUNIQ }}}: 'Given log. name not unique',
    {{{ cDefs.EBADFD }}}: 'f.d. invalid for this operation',
    {{{ cDefs.EREMCHG }}}: 'Remote address changed',
    {{{ cDefs.ELIBACC }}}: 'Can   access a needed shared lib',
    {{{ cDefs.ELIBBAD }}}: 'Accessing a corrupted shared lib',
    {{{ cDefs.ELIBSCN }}}: '.lib section in a.out corrupted',
    {{{ cDefs.ELIBMAX }}}: 'Attempting to link in too many libs',
    {{{ cDefs.ELIBEXEC }}}: 'Attempting to exec a shared library',
    {{{ cDefs.ENOSYS }}}: 'Function not implemented',
    {{{ cDefs.ENOTEMPTY }}}: 'Directory not empty',
    {{{ cDefs.ENAMETOOLONG }}}: 'File or path name too long',
    {{{ cDefs.ELOOP }}}: 'Too many symbolic links',
    {{{ cDefs.EOPNOTSUPP }}}: 'Operation not supported on transport endpoint',
    {{{ cDefs.EPFNOSUPPORT }}}: 'Protocol family not supported',
    {{{ cDefs.ECONNRESET }}}: 'Connection reset by peer',
    {{{ cDefs.ENOBUFS }}}: 'No buffer space available',
    {{{ cDefs.EAFNOSUPPORT }}}: 'Address family not supported by protocol family',
    {{{ cDefs.EPROTOTYPE }}}: 'Protocol wrong type for socket',
    {{{ cDefs.ENOTSOCK }}}: 'Socket operation on non-socket',
    {{{ cDefs.ENOPROTOOPT }}}: 'Protocol not available',
    {{{ cDefs.ESHUTDOWN }}}: 'Can\'t send after socket shutdown',
    {{{ cDefs.ECONNREFUSED }}}: 'Connection refused',
    {{{ cDefs.EADDRINUSE }}}: 'Address already in use',
    {{{ cDefs.ECONNABORTED }}}: 'Connection aborted',
    {{{ cDefs.ENETUNREACH }}}: 'Network is unreachable',
    {{{ cDefs.ENETDOWN }}}: 'Network interface is not configured',
    {{{ cDefs.ETIMEDOUT }}}: 'Connection timed out',
    {{{ cDefs.EHOSTDOWN }}}: 'Host is down',
    {{{ cDefs.EHOSTUNREACH }}}: 'Host is unreachable',
    {{{ cDefs.EINPROGRESS }}}: 'Connection already in progress',
    {{{ cDefs.EALREADY }}}: 'Socket already connected',
    {{{ cDefs.EDESTADDRREQ }}}: 'Destination address required',
    {{{ cDefs.EMSGSIZE }}}: 'Message too long',
    {{{ cDefs.EPROTONOSUPPORT }}}: 'Unknown protocol',
    {{{ cDefs.ESOCKTNOSUPPORT }}}: 'Socket type not supported',
    {{{ cDefs.EADDRNOTAVAIL }}}: 'Address not available',
    {{{ cDefs.ENETRESET }}}: 'Connection reset by network',
    {{{ cDefs.EISCONN }}}: 'Socket is already connected',
    {{{ cDefs.ENOTCONN }}}: 'Socket is not connected',
    {{{ cDefs.ETOOMANYREFS }}}: 'Too many references',
    {{{ cDefs.EUSERS }}}: 'Too many users',
    {{{ cDefs.EDQUOT }}}: 'Quota exceeded',
    {{{ cDefs.ESTALE }}}: 'Stale file handle',
    {{{ cDefs.ENOTSUP }}}: 'Not supported',
    {{{ cDefs.ENOMEDIUM }}}: 'No medium (in tape drive)',
    {{{ cDefs.EILSEQ }}}: 'Illegal byte sequence',
    {{{ cDefs.EOVERFLOW }}}: 'Value too large for defined data type',
    {{{ cDefs.ECANCELED }}}: 'Operation canceled',
    {{{ cDefs.ENOTRECOVERABLE }}}: 'State not recoverable',
    {{{ cDefs.EOWNERDEAD }}}: 'Previous owner died',
    {{{ cDefs.ESTRPIPE }}}: 'Streams pipe error',
  },
#if SUPPORT_ERRNO
  $setErrNo__deps: ['__errno_location'],
  $setErrNo: (value) => {
    {{{makeSetValue("___errno_location()", 0, 'value', 'i32') }}};
    return value;
  },
#else
  $setErrNo: (value) => {
#if ASSERTIONS
    err('failed to set errno from JS');
#endif
    return 0;
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

    lookup_name: (name) => {
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

    lookup_addr: (addr) => {
      if (DNS.address_map.names[addr]) {
        return DNS.address_map.names[addr];
      }

      return null;
    }
  },

  // note: lots of leaking here!
  gethostbyaddr__deps: ['$DNS', '$getHostByName', '$inetNtop4', '$setErrNo'],
  gethostbyaddr__proxy: 'sync',
  gethostbyaddr: (addr, addrlen, type) => {
    if (type !== {{{ cDefs.AF_INET }}}) {
      setErrNo({{{ cDefs.EAFNOSUPPORT }}});
      // TODO: set h_errno
      return null;
    }
    addr = {{{ makeGetValue('addr', '0', 'i32') }}}; // addr is in_addr
    var host = inetNtop4(addr);
    var lookup = DNS.lookup_addr(host);
    if (lookup) {
      host = lookup;
    }
    return getHostByName(host);
  },

  gethostbyname__deps: ['$getHostByName'],
  gethostbyname__proxy: 'sync',
  gethostbyname: (name) => {
    return getHostByName(UTF8ToString(name));
  },

  $getHostByName__deps: ['malloc', '$stringToNewUTF8', '$DNS', '$inetPton4'],
  $getHostByName: (name) => {
    // generate hostent
    var ret = _malloc({{{ C_STRUCTS.hostent.__size__ }}}); // XXX possibly leaked, as are others here
    var nameBuf = stringToNewUTF8(name);
    {{{ makeSetValue('ret', C_STRUCTS.hostent.h_name, 'nameBuf', POINTER_TYPE) }}};
    var aliasesBuf = _malloc(4);
    {{{ makeSetValue('aliasesBuf', '0', '0', POINTER_TYPE) }}};
    {{{ makeSetValue('ret', C_STRUCTS.hostent.h_aliases, 'aliasesBuf', 'i8**') }}};
    var afinet = {{{ cDefs.AF_INET }}};
    {{{ makeSetValue('ret', C_STRUCTS.hostent.h_addrtype, 'afinet', 'i32') }}};
    {{{ makeSetValue('ret', C_STRUCTS.hostent.h_length, '4', 'i32') }}};
    var addrListBuf = _malloc(12);
    {{{ makeSetValue('addrListBuf', '0', 'addrListBuf+8', POINTER_TYPE) }}};
    {{{ makeSetValue('addrListBuf', '4', '0', POINTER_TYPE) }}};
    {{{ makeSetValue('addrListBuf', '8', 'inetPton4(DNS.lookup_name(name))', 'i32') }}};
    {{{ makeSetValue('ret', C_STRUCTS.hostent.h_addr_list, 'addrListBuf', 'i8**') }}};
    return ret;
  },

  gethostbyname_r__deps: ['gethostbyname', 'memcpy', 'free'],
  gethostbyname_r__proxy: 'sync',
  gethostbyname_r: (name, ret, buf, buflen, out, err) => {
    var data = _gethostbyname(name);
    _memcpy(ret, data, {{{ C_STRUCTS.hostent.__size__ }}});
    _free(data);
    {{{ makeSetValue('err', '0', '0', 'i32') }}};
    {{{ makeSetValue('out', '0', 'ret', '*') }}};
    return 0;
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
          addr = [0, 0, 0, 1];
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
  $Sockets__deps: ['$setErrNo',
    () => 'var SocketIO = ' + read('../third_party/socket.io.js') + ';\n',
    () => 'var Peer = ' + read('../third_party/wrtcp.js') + ';\n'],
#else
  $Sockets__deps: ['$setErrNo'],
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
    abort("no cryptographic support found for randomDevice. consider polyfilling it if you want to use something insecure like Math.random(), e.g. put this in a --pre-js: var crypto = { getRandomValues: (array) => { for (var i = 0; i < array.length; i++) array[i] = (Math.random()*256)|0 } };");
#else
    abort("initRandomDevice");
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

  emscripten_get_now: `;
#if ENVIRONMENT_MAY_BE_NODE && MIN_NODE_VERSION < 160000
    // The performance global was added to node in v16.0.0:
    // https://nodejs.org/api/globals.html#performance
    if (ENVIRONMENT_IS_NODE) {
      global.performance = require('perf_hooks').performance;
    }
#endif
#if PTHREADS && !AUDIO_WORKLET
    // Pthreads need their clocks synchronized to the execution of the main
    // thread, so, when using them, make sure to adjust all timings to the
    // respective time origins.
    _emscripten_get_now = () => performance.timeOrigin + {{{ getPerformanceNow() }}}();
#else
#if MIN_IE_VERSION <= 9 || MIN_FIREFOX_VERSION <= 14 || MIN_CHROME_VERSION <= 23 || MIN_SAFARI_VERSION <= 80400 || AUDIO_WORKLET // https://caniuse.com/#feat=high-resolution-time
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
#else
    // Modern environment where performance.now() is supported:
    // N.B. a shorter form "_emscripten_get_now = performance.now;" is
    // unfortunately not allowed even in current browsers (e.g. FF Nightly 75).
    _emscripten_get_now = () => {{{ getPerformanceNow() }}}();
#endif
#endif
`,

  emscripten_get_now_res: () => { // return resolution of get_now, in nanoseconds
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE) {
      return 1; // nanoseconds
    }
#endif
#if MIN_IE_VERSION <= 9 || MIN_FIREFOX_VERSION <= 14 || MIN_CHROME_VERSION <= 23 || MIN_SAFARI_VERSION <= 80400 // https://caniuse.com/#feat=high-resolution-time
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
#if MIN_IE_VERSION <= 9 || MIN_FIREFOX_VERSION <= 14 || MIN_CHROME_VERSION <= 23 || MIN_SAFARI_VERSION <= 80400 // https://caniuse.com/#feat=high-resolution-time
  $nowIsMonotonic: `
     ((typeof performance == 'object' && performance && typeof performance['now'] == 'function')
#if ENVIRONMENT_MAY_BE_NODE
      || ENVIRONMENT_IS_NODE
#endif
    );`,
#else
  // Modern environment where performance.now() is supported: (rely on minifier to return true unconditionally from this function)
  $nowIsMonotonic: 'true;',
#endif

  _emscripten_get_now_is_monotonic__internal: true,
  _emscripten_get_now_is_monotonic__deps: ['$nowIsMonotonic'],
  _emscripten_get_now_is_monotonic: () => nowIsMonotonic,

  $warnOnce: (text) => {
    if (!warnOnce.shown) warnOnce.shown = {};
    if (!warnOnce.shown[text]) {
      warnOnce.shown[text] = 1;
#if ENVIRONMENT_MAY_BE_NODE
      if (ENVIRONMENT_IS_NODE) text = 'warning: ' + text;
#endif
      err(text);
    }
  },

  $getCallstack__deps: ['$jsStackTrace', '$warnOnce'],
  $getCallstack__docs: '/** @param {number=} flags */',
  $getCallstack: function(flags) {
    var callstack = jsStackTrace();

    // Find the symbols in the callstack that corresponds to the functions that
    // report callstack information, and remove everything up to these from the
    // output.
    var iThisFunc = callstack.lastIndexOf('_emscripten_log');
    var iThisFunc2 = callstack.lastIndexOf('_emscripten_get_callstack');
    var iNextLine = callstack.indexOf('\n', Math.max(iThisFunc, iThisFunc2))+1;
    callstack = callstack.slice(iNextLine);

    // If user requested to see the original source stack, but no source map
    // information is available, just fall back to showing the JS stack.
    if (flags & {{{ cDefs.EM_LOG_C_STACK }}} && typeof emscripten_source_map == 'undefined') {
      warnOnce('Source map information is not available, emscripten_log with EM_LOG_C_STACK will be ignored. Build with "--pre-js $EMSCRIPTEN/src/emscripten-source-map.min.js" linker flag to add source map loading to code.');
      flags ^= {{{ cDefs.EM_LOG_C_STACK }}};
      flags |= {{{ cDefs.EM_LOG_JS_STACK }}};
    }

    // Process all lines:
    var lines = callstack.split('\n');
    callstack = '';
    // New FF30 with column info: extract components of form:
    // '       Object._main@http://server.com:4324:12'
    var newFirefoxRe = new RegExp('\\s*(.*?)@(.*?):([0-9]+):([0-9]+)');
    // Old FF without column info: extract components of form:
    // '       Object._main@http://server.com:4324'
    var firefoxRe = new RegExp('\\s*(.*?)@(.*):(.*)(:(.*))?');
    // Extract components of form:
    // '    at Object._main (http://server.com/file.html:4324:12)'
    var chromeRe = new RegExp('\\s*at (.*?) \\\((.*):(.*):(.*)\\\)');

    for (var l in lines) {
      var line = lines[l];

      var symbolName = '';
      var file = '';
      var lineno = 0;
      var column = 0;

      var parts = chromeRe.exec(line);
      if (parts && parts.length == 5) {
        symbolName = parts[1];
        file = parts[2];
        lineno = parts[3];
        column = parts[4];
      } else {
        parts = newFirefoxRe.exec(line);
        if (!parts) parts = firefoxRe.exec(line);
        if (parts && parts.length >= 4) {
          symbolName = parts[1];
          file = parts[2];
          lineno = parts[3];
          // Old Firefox doesn't carry column information, but in new FF30, it
          // is present. See https://bugzilla.mozilla.org/show_bug.cgi?id=762556
          column = parts[4]|0;
        } else {
          // Was not able to extract this line for demangling/sourcemapping
          // purposes. Output it as-is.
          callstack += line + '\n';
          continue;
        }
      }

      var haveSourceMap = false;

      if (flags & {{{ cDefs.EM_LOG_C_STACK }}}) {
        var orig = emscripten_source_map.originalPositionFor({line: lineno, column: column});
        haveSourceMap = (orig && orig.source);
        if (haveSourceMap) {
          if (flags & {{{ cDefs.EM_LOG_NO_PATHS }}}) {
            orig.source = orig.source.substring(orig.source.replace(/\\/g, "/").lastIndexOf('/')+1);
          }
          callstack += `    at ${symbolName} (${orig.source}:${orig.line}:${orig.column})\n`;
        }
      }
      if ((flags & {{{ cDefs.EM_LOG_JS_STACK }}}) || !haveSourceMap) {
        if (flags & {{{ cDefs.EM_LOG_NO_PATHS }}}) {
          file = file.substring(file.replace(/\\/g, "/").lastIndexOf('/')+1);
        }
        callstack += (haveSourceMap ? (`     = ${symbolName}`) : (`    at ${symbolName}`)) + ` (${file}:${lineno}:${column})\n`;
      }
    }
    // Trim extra whitespace at the end of the output.
    callstack = callstack.replace(/\s+$/, '');
    return callstack;
  },

  emscripten_get_callstack__deps: ['$getCallstack', '$lengthBytesUTF8', '$stringToUTF8'],
  emscripten_get_callstack: function(flags, str, maxbytes) {
    var callstack = getCallstack(flags);
    // User can query the required amount of bytes to hold the callstack.
    if (!str || maxbytes <= 0) {
      return lengthBytesUTF8(callstack)+1;
    }
    // Output callstack string as C string to HEAP.
    var bytesWrittenExcludingNull = stringToUTF8(callstack, str, maxbytes);

    // Return number of bytes written, including null.
    return bytesWrittenExcludingNull+1;
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
    var str = UTF8ArrayToString(result, 0);
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

    if (!_emscripten_get_compiler_setting.cache) _emscripten_get_compiler_setting.cache = {};
    var cache = _emscripten_get_compiler_setting.cache;
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

  // Generates a representation of the program counter from a line of stack trace.
  // The exact return value depends in whether we are running WASM or JS, and whether
  // the engine supports offsets into WASM. See the function body for details.
  $convertFrameToPC__docs: '/** @returns {number} */',
  $convertFrameToPC__internal: true,
  $convertFrameToPC: (frame) => {
#if !USE_OFFSET_CONVERTER
    abort('Cannot use convertFrameToPC (needed by __builtin_return_address) without -sUSE_OFFSET_CONVERTER');
#else
#if ASSERTIONS
    assert(wasmOffsetConverter);
#endif
    var match;

    if (match = /\bwasm-function\[\d+\]:(0x[0-9a-f]+)/.exec(frame)) {
      // some engines give the binary offset directly, so we use that as return address
      return +match[1];
    } else if (match = /\bwasm-function\[(\d+)\]:(\d+)/.exec(frame)) {
      // other engines only give function index and offset in the function,
      // so we try using the offset converter. If that doesn't work,
      // we pack index and offset into a "return address"
      return wasmOffsetConverter.convert(+match[1], +match[2]);
    } else if (match = /:(\d+):\d+(?:\)|$)/.exec(frame)) {
      // If we are in js, we can use the js line number as the "return address".
      // This should work for wasm2js.  We tag the high bit to distinguish this
      // from wasm addresses.
      return 0x80000000 | +match[1];
    }
#endif
    // return 0 if we can't find any
    return 0;
  },

  // Returns a representation of a call site of the caller of this function, in a manner
  // similar to __builtin_return_address. If level is 0, we return the call site of the
  // caller of this function.
  emscripten_return_address__deps: ['$convertFrameToPC', '$jsStackTrace'],
  emscripten_return_address: (level) => {
    var callstack = jsStackTrace().split('\n');
    if (callstack[0] == 'Error') {
      callstack.shift();
    }
    // skip this function and the caller to get caller's return address
#if MEMORY64
    // MEMORY64 injects and extra wrapper within emscripten_return_address
    // to handle BigInt convertions.
    var caller = callstack[level + 4];
#else
    var caller = callstack[level + 3];
#endif
    return convertFrameToPC(caller);
  },

  $UNWIND_CACHE: {},

  // This function pulls the JavaScript stack trace and updates UNWIND_CACHE so
  // that our representation of the program counter is mapped to the line of the
  // stack trace for every line in the stack trace. This allows
  // emscripten_pc_get_* to lookup the line of the stack trace from the PC and
  // return meaningful information.
  //
  // Additionally, it saves a copy of the entire stack trace and the return
  // address of the caller. This is because there are two common forms of a
  // stack trace.  The first form starts the stack trace at the caller of the
  // function requesting a stack trace. In this case, the function can simply
  // walk down the stack from the return address using emscripten_return_address
  // with increasing values for level.  The second form starts the stack trace
  // at the current function. This requires a helper function to get the program
  // counter. This helper function will return the return address.  This is the
  // program counter at the call site. But there is a problem: when calling into
  // code that performs stack unwinding, the program counter has changed since
  // execution continued from calling the helper function. So we can't just walk
  // down the stack and expect to see the PC value we got. By caching the call
  // stack, we can call emscripten_stack_unwind with the PC value and use that
  // to unwind the cached stack. Naturally, the PC helper function will have to
  // call emscripten_stack_snapshot to cache the stack. We also return the
  // return address of the caller so the PC helper function does not need to
  // call emscripten_return_address, saving a lot of time.
  //
  // One might expect that a sensible solution is to call the stack unwinder and
  // explicitly tell it how many functions to skip from the stack. However,
  // existing libraries do not work this way.  For example, compiler-rt's
  // sanitizer_common library has macros GET_CALLER_PC_BP_SP and
  // GET_CURRENT_PC_BP_SP, which obtains the PC value for the two common cases
  // stated above, respectively. Then, it passes the PC, BP, SP values along
  // until some other function uses them to unwind. On standard machines, the
  // stack can be unwound by treating BP as a linked list.  This makes PC
  // unnecessary to walk the stack, since walking is done with BP, which remains
  // valid until the function returns. But on Emscripten, BP does not exist, at
  // least in JavaScript frames, so we have to rely on PC values. Therefore, we
  // must be able to unwind from a PC value that may no longer be on the
  // execution stack, and so we are forced to cache the entire call stack.
  emscripten_stack_snapshot__deps: ['$convertFrameToPC', '$UNWIND_CACHE', '$saveInUnwindCache', '$jsStackTrace'],
  emscripten_stack_snapshot: function() {
    var callstack = jsStackTrace().split('\n');
    if (callstack[0] == 'Error') {
      callstack.shift();
    }
    saveInUnwindCache(callstack);

    // Caches the stack snapshot so that emscripten_stack_unwind_buffer() can
    // unwind from this spot.
    UNWIND_CACHE.last_addr = convertFrameToPC(callstack[3]);
    UNWIND_CACHE.last_stack = callstack;
    return UNWIND_CACHE.last_addr;
  },

  $saveInUnwindCache__deps: ['$UNWIND_CACHE', '$convertFrameToPC'],
  $saveInUnwindCache__internal: true,
  $saveInUnwindCache: (callstack) => {
    callstack.forEach((frame) => {
      var pc = convertFrameToPC(frame);
      if (pc) {
        UNWIND_CACHE[pc] = frame;
      }
    });
  },

  // Unwinds the stack from a cached PC value. See emscripten_stack_snapshot for
  // how this is used.  addr must be the return address of the last call to
  // emscripten_stack_snapshot, or this function will instead use the current
  // call stack.
  emscripten_stack_unwind_buffer__deps: ['$UNWIND_CACHE', '$saveInUnwindCache', '$convertFrameToPC', '$jsStackTrace'],
  emscripten_stack_unwind_buffer: (addr, buffer, count) => {
    var stack;
    if (UNWIND_CACHE.last_addr == addr) {
      stack = UNWIND_CACHE.last_stack;
    } else {
      stack = jsStackTrace().split('\n');
      if (stack[0] == 'Error') {
        stack.shift();
      }
      saveInUnwindCache(stack);
    }

    var offset = 3;
    while (stack[offset] && convertFrameToPC(stack[offset]) != addr) {
      ++offset;
    }

    for (var i = 0; i < count && stack[i+offset]; ++i) {
      {{{ makeSetValue('buffer', 'i*4', 'convertFrameToPC(stack[i + offset])', 'i32') }}};
    }
    return i;
  },

  // Look up the function name from our stack frame cache with our PC representation.
#if USE_OFFSET_CONVERTER
  emscripten_pc_get_function__deps: ['$UNWIND_CACHE', 'free', '$stringToNewUTF8'],
  // Don't treat allocation of _emscripten_pc_get_function.ret as a leak
  emscripten_pc_get_function__noleakcheck: true,
#endif
  emscripten_pc_get_function: (pc) => {
#if !USE_OFFSET_CONVERTER
    abort('Cannot use emscripten_pc_get_function without -sUSE_OFFSET_CONVERTER');
#else
    var name;
    if (pc & 0x80000000) {
      // If this is a JavaScript function, try looking it up in the unwind cache.
      var frame = UNWIND_CACHE[pc];
      if (!frame) return 0;

      var match;
      if (match = /^\s+at (.*) \(.*\)$/.exec(frame)) {
        name = match[1];
      } else if (match = /^(.+?)@/.exec(frame)) {
        name = match[1];
      } else {
        return 0;
      }
    } else {
      name = wasmOffsetConverter.getName(pc);
    }
    if (_emscripten_pc_get_function.ret) _free(_emscripten_pc_get_function.ret);
    _emscripten_pc_get_function.ret = stringToNewUTF8(name);
    return _emscripten_pc_get_function.ret;
#endif
  },

  $convertPCtoSourceLocation__deps: ['$UNWIND_CACHE', '$convertFrameToPC'],
  $convertPCtoSourceLocation: (pc) => {
    if (UNWIND_CACHE.last_get_source_pc == pc) return UNWIND_CACHE.last_source;

    var match;
    var source;
#if LOAD_SOURCE_MAP
    if (wasmSourceMap) {
      source = wasmSourceMap.lookup(pc);
    }
#endif

    if (!source) {
      var frame = UNWIND_CACHE[pc];
      if (!frame) return null;
      // Example: at callMain (a.out.js:6335:22)
      if (match = /\((.*):(\d+):(\d+)\)$/.exec(frame)) {
        source = {file: match[1], line: match[2], column: match[3]};
      // Example: main@a.out.js:1337:42
      } else if (match = /@(.*):(\d+):(\d+)/.exec(frame)) {
        source = {file: match[1], line: match[2], column: match[3]};
      }
    }
    UNWIND_CACHE.last_get_source_pc = pc;
    UNWIND_CACHE.last_source = source;
    return source;
  },

  // Look up the file name from our stack frame cache with our PC representation.
  emscripten_pc_get_file__deps: ['$convertPCtoSourceLocation', 'free', '$stringToNewUTF8'],
  // Don't treat allocation of _emscripten_pc_get_file.ret as a leak
  emscripten_pc_get_file__noleakcheck: true,
  emscripten_pc_get_file: (pc) => {
    var result = convertPCtoSourceLocation(pc);
    if (!result) return 0;

    if (_emscripten_pc_get_file.ret) _free(_emscripten_pc_get_file.ret);
    _emscripten_pc_get_file.ret = stringToNewUTF8(result.file);
    return _emscripten_pc_get_file.ret;
  },

  // Look up the line number from our stack frame cache with our PC representation.
  emscripten_pc_get_line__deps: ['$convertPCtoSourceLocation'],
  emscripten_pc_get_line: (pc) => {
    var result = convertPCtoSourceLocation(pc);
    return result ? result.line : 0;
  },

  // Look up the column number from our stack frame cache with our PC representation.
  emscripten_pc_get_column__deps: ['$convertPCtoSourceLocation'],
  emscripten_pc_get_column: (pc) => {
    var result = convertPCtoSourceLocation(pc);
    return result ? result.column || 0 : 0;
  },

  emscripten_get_module_name__deps: ['$stringToUTF8'],
  emscripten_get_module_name: (buf, length) => {
#if MINIMAL_RUNTIME
    return stringToUTF8('{{{ TARGET_BASENAME }}}.wasm', buf, length);
#else
    return stringToUTF8(wasmBinaryFile, buf, length);
#endif
  },

#if USE_ASAN || USE_LSAN || UBSAN_RUNTIME
  // When lsan or asan is enabled withBuiltinMalloc temporarily replaces calls
  // to malloc, free, and memalign.
  $withBuiltinMalloc__deps: ['emscripten_builtin_malloc', 'emscripten_builtin_free', 'emscripten_builtin_memalign'
                            ],
  $withBuiltinMalloc__docs: '/** @suppress{checkTypes} */',
  $withBuiltinMalloc: (func) => {
    var prev_malloc = typeof _malloc != 'undefined' ? _malloc : undefined;
    var prev_memalign = typeof _memalign != 'undefined' ? _memalign : undefined;
    var prev_free = typeof _free != 'undefined' ? _free : undefined;
    _malloc = _emscripten_builtin_malloc;
    _memalign = _emscripten_builtin_memalign;
    _free = _emscripten_builtin_free;
    try {
      return func();
    } finally {
      _malloc = prev_malloc;
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
    buf >>= 2;
    while (ch = HEAPU8[sigPtr++]) {
#if ASSERTIONS
      var chr = String.fromCharCode(ch);
      var validChars = ['d', 'f', 'i'];
#if WASM_BIGINT
      // In WASM_BIGINT mode we support passing i64 values as bigint.
      validChars.push('j');
#endif
#if MEMORY64
      // In MEMORY64 mode we also support passing i64 pointer types which
      // get automatically converted to int53/Double.
      validChars.push('p');
#endif
      assert(validChars.includes(chr), `Invalid character ${ch}("${chr}") in readEmAsmArgs! Use only [${validChars}], and do not specify "v" for void return argument.`);
#endif
      // Floats are always passed as doubles, and doubles and int64s take up 8
      // bytes (two 32-bit slots) in memory, align reads to these:
      buf += (ch != 105/*i*/) & buf;
#if MEMORY64
      // Special case for pointers under wasm64 which we read as int53 Numbers.
      if (ch == 112/*p*/) {
        readEmAsmArgsArray.push(readI53FromI64(buf++ << 2));
      } else
#endif
      readEmAsmArgsArray.push(
        ch == 105/*i*/ ? HEAP32[buf] :
#if WASM_BIGINT
       (ch == 106/*j*/ ? HEAP64 : HEAPF64)[buf++ >> 1]
#else
       HEAPF64[buf++ >> 1]
#endif
      );
      ++buf;
    }
    return readEmAsmArgsArray;
  },

#if HAVE_EM_ASM
  $runEmAsmFunction__deps: ['$readEmAsmArgs'],
  $runEmAsmFunction: (code, sigPtr, argbuf) => {
    var args = readEmAsmArgs(sigPtr, argbuf);
#if ASSERTIONS
    if (!ASM_CONSTS.hasOwnProperty(code)) abort(`No EM_ASM constant found at address ${code}`);
#endif
    return ASM_CONSTS[code].apply(null, args);
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
  $runMainThreadEmAsm: (code, sigPtr, argbuf, sync) => {
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
      // -1 - code is the encoding of a proxied EM_ASM, as a negative number
      // (positive numbers are non-EM_ASM calls).
      return proxyToMainThread.apply(null, [-1 - code, sync].concat(args));
    }
#endif
#if ASSERTIONS
    if (!ASM_CONSTS.hasOwnProperty(code)) abort(`No EM_ASM constant found at address ${code}`);
#endif
    return ASM_CONSTS[code].apply(null, args);
  },
  emscripten_asm_const_int_sync_on_main_thread__deps: ['$runMainThreadEmAsm'],
  emscripten_asm_const_int_sync_on_main_thread: (code, sigPtr, argbuf) => {
    return runMainThreadEmAsm(code, sigPtr, argbuf, 1);
  },

  emscripten_asm_const_double_sync_on_main_thread: 'emscripten_asm_const_int_sync_on_main_thread',
  emscripten_asm_const_async_on_main_thread__deps: ['$runMainThreadEmAsm'],
  emscripten_asm_const_async_on_main_thread: (code, sigPtr, argbuf) => runMainThreadEmAsm(code, sigPtr, argbuf, 0),
#endif

#if !DECLARE_ASM_MODULE_EXPORTS
  // When DECLARE_ASM_MODULE_EXPORTS is not set we export native symbols
  // at runtime rather than statically in JS code.
  $exportAsmFunctions__deps: ['$asmjsMangle'],
  $exportAsmFunctions: (asm) => {
#if ENVIRONMENT_MAY_BE_NODE && ENVIRONMENT_MAY_BE_WEB
    var global_object = (typeof process != "undefined" ? global : this);
#elif ENVIRONMENT_MAY_BE_NODE
    var global_object = global;
#else
    var global_object = this;
#endif

    for (var __exportedFunc in asm) {
      var jsname = asmjsMangle(__exportedFunc);
#if MINIMAL_RUNTIME
      global_object[jsname] = asm[__exportedFunc];
#else
      global_object[jsname] = Module[jsname] = asm[__exportedFunc];
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
  $jstoi_s: (str) => Number(str),

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

  _Unwind_GetIPInfo: () => abort('Unwind_GetIPInfo'),

  _Unwind_FindEnclosingFunction: () => 0, // we cannot succeed

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
#if MIN_CHROME_VERSION < 55 || MIN_EDGE_VERSION < 18 || MIN_FIREFOX_VERSION < 50 || MIN_IE_VERSION != TARGET_NOT_SUPPORTED // https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/addEventListener
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
#if MAIN_MODULE == 1
  $dynCallLegacy__deps: ['$createDyncallWrapper'],
#endif
  $dynCallLegacy: (sig, ptr, args) => {
#if ASSERTIONS
#if MINIMAL_RUNTIME
    assert(typeof dynCalls != 'undefined', 'Global dynCalls dictionary was not generated in the build! Pass -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=$dynCall linker flag to include it!');
    assert(sig in dynCalls, `bad function pointer type - sig is not in dynCalls: '${sig}'`);
#else
    assert(('dynCall_' + sig) in Module, `bad function pointer type - dynCall function not found for sig '${sig}'`);
#endif
    if (args && args.length) {
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
    return args && args.length ? f.apply(null, [ptr].concat(args)) : f.call(null, ptr);
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
    var argCache = [];
    return function() {
      argCache.length = 0;
      Object.assign(argCache, arguments);
      return dynCall(sig, ptr, argCache);
    };
  },

  $dynCall__docs: '/** @param {Object=} args */',
  $dynCall: (sig, ptr, args) => {
#if DYNCALLS
    return dynCallLegacy(sig, ptr, args);
#else
#if !WASM_BIGINT
    // Without WASM_BIGINT support we cannot directly call function with i64 as
    // part of thier signature, so we rely the dynCall functions generated by
    // wasm-emscripten-finalize
    if (sig.includes('j')) {
      return dynCallLegacy(sig, ptr, args);
    }
#endif
#if ASSERTIONS
    assert(getWasmTableEntry(ptr), `missing table entry in dynCall: ${ptr}`);
#endif
#if MEMORY64
    // With MEMORY64 we have an additional step to convert `p` arguments to
    // bigint. This is the runtime equivalent of the wrappers we create for wasm
    // exports in `emscripten.py:create_wasm64_wrappers`.
    for (var i = 1; i < sig.length; ++i) {
      if (sig[i] == 'p') args[i-1] = BigInt(args[i-1]);
    }
#endif
    var rtn = getWasmTableEntry(ptr).apply(null, args);
#if MEMORY64
    return sig[0] == 'p' ? Number(rtn) : rtn;
#else
    return rtn;
#endif

#endif
  },

  $callRuntimeCallbacks__internal: true,
  $callRuntimeCallbacks: (callbacks) => {
    while (callbacks.length > 0) {
      // Pass the module as the first argument.
      callbacks.shift()(Module);
    }
  },

#if SHRINK_LEVEL == 0 || ASYNCIFY == 2
  // A mirror copy of contents of wasmTable in JS side, to avoid relatively
  // slow wasmTable.get() call. Only used when not compiling with -Os, -Oz, or
  // JSPI which needs to instrument the functions.
  $wasmTableMirror__internal: true,
  $wasmTableMirror: [],

  $setWasmTableEntry__internal: true,
  $setWasmTableEntry__deps: ['$wasmTableMirror'],
  $setWasmTableEntry: (idx, func) => {
    wasmTable.set(idx, func);
    // With ABORT_ON_WASM_EXCEPTIONS wasmTable.get is overriden to return wrapped
    // functions so we need to call it here to retrieve the potential wrapper correctly
    // instead of just storing 'func' directly into wasmTableMirror
    wasmTableMirror[idx] = wasmTable.get(idx);
  },

  $getWasmTableEntry__internal: true,
  $getWasmTableEntry__deps: ['$wasmTableMirror'],
  $getWasmTableEntry: (funcPtr) => {
#if MEMORY64
    // Function pointers are 64-bit, but wasmTable.get() requires a Number.
    // https://github.com/emscripten-core/emscripten/issues/18200
    funcPtr = Number(funcPtr);
#endif
    var func = wasmTableMirror[funcPtr];
    if (!func) {
      if (funcPtr >= wasmTableMirror.length) wasmTableMirror.length = funcPtr + 1;
      wasmTableMirror[funcPtr] = func = wasmTable.get(funcPtr);
#if ASYNCIFY == 2
      if (Asyncify.isAsyncExport(func)) {
        wasmTableMirror[funcPtr] = func = Asyncify.makeAsyncFunction(func);
      }
#endif
    }
#if ASSERTIONS && ASYNCIFY != 2 // With JSPI the function stored in the table will be a wrapper.
    assert(wasmTable.get(funcPtr) == func, "JavaScript-side Wasm function table mirror is out of date!");
#endif
    return func;
  },

#else

  $setWasmTableEntry: (idx, func) => wasmTable.set(idx, func),

  $getWasmTableEntry: (funcPtr) => {
#if MEMORY64
    // Function pointers are 64-bit, but wasmTable.get() requires a Number.
    // https://github.com/emscripten-core/emscripten/issues/18200
    funcPtr = Number(funcPtr);
#endif
    // In -Os and -Oz builds, do not implement a JS side wasm table mirror for small
    // code size, but directly access wasmTable, which is a bit slower as uncached.
    return wasmTable.get(funcPtr);
  },
#endif // SHRINK_LEVEL == 0

  // Callable in pthread without __proxy needed.
  emscripten_exit_with_live_runtime: () => {
    {{{ runtimeKeepalivePush() }}}
    throw 'unwind';
  },

  emscripten_force_exit__deps: ['exit',
#if !EXIT_RUNTIME && ASSERTIONS
    '$warnOnce',
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
#if !MINIMAL_RUNTIME
    noExitRuntime = false;
    runtimeKeepaliveCounter = 0;
#endif
    _exit(status);
  },

  emscripten_out: (str) => out(UTF8ToString(str)),
  emscripten_outn: (str, len) => out(UTF8ToString(str, len)),

  emscripten_err: (str) => err(UTF8ToString(str)),
  emscripten_errn: (str, len) => err(UTF8ToString(str, len)),

#if ASSERTIONS || RUNTIME_DEBUG
  emscripten_dbg: (str) => dbg(UTF8ToString(str)),
  emscripten_dbgn: (str, len) => dbg(UTF8ToString(str, len)),
#endif

  // Use program_invocation_short_name and program_invocation_name in compiled
  // programs. This function is for implementing them.
#if !MINIMAL_RUNTIME
  _emscripten_get_progname__deps: ['$stringToUTF8'],
#endif
  _emscripten_get_progname: (str, len) => {
#if !MINIMAL_RUNTIME
#if ASSERTIONS
    assert(typeof str == 'number');
    assert(typeof len == 'number');
#endif
    stringToUTF8(thisProgram, str, len);
#endif
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
#if MINIMAL_RUNTIME
    throw e;
#else
    quit_(1, e);
#endif
  },

  // Callable in pthread without __proxy needed.
  $runtimeKeepalivePush__sig: 'v',
  $runtimeKeepalivePush: () => {
    runtimeKeepaliveCounter += 1;
#if RUNTIME_DEBUG
    dbg(`runtimeKeepalivePush -> counter=${runtimeKeepaliveCounter}`);
#endif
  },

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
  // keepRuntimeAlive is a runtime function rather than a library function,
  // so we can't use an alias like we do for the two functions above.
  emscripten_runtime_keepalive_check: () => keepRuntimeAlive(),

  // Used to call user callbacks from the embedder / event loop.  For example
  // setTimeout or any other kind of event handler that calls into user case
  // needs to use this wrapper.
  //
  // The job of this wrapper is the handle emscripten-specfic exceptions such
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

  $maybeExit__deps: ['exit', '$handleException',
#if PTHREADS
    '_emscripten_thread_exit',
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
  $callUserCallback: (func) => {
    func();
  },
#endif // MINIMAL_RUNTIME

  $safeSetTimeout__deps: ['$callUserCallback'],
  $safeSetTimeout__docs: '/** @param {number=} timeout */',
  $safeSetTimeout: (func, timeout) => {
    {{{ runtimeKeepalivePush() }}}
    return setTimeout(() => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(func);
    }, timeout);
  },

  $asmjsMangle: (x) => {
    var unmangledSymbols = {{{ buildStringArray(WASM_SYSTEM_EXPORTS) }}};
    if (x == '__main_argc_argv') {
      x = 'main';
    }
    return x.indexOf('dynCall_') == 0 || unmangledSymbols.includes(x) ? x : '_' + x;
  },

  $asyncLoad__docs: '/** @param {boolean=} noRunDep */',
  $asyncLoad: (url, onload, onerror, noRunDep) => {
    var dep = !noRunDep ? getUniqueRunDependency(`al ${url}`) : '';
    readAsync(url, (arrayBuffer) => {
      assert(arrayBuffer, `Loading data file "${url}" failed (no arrayBuffer).`);
      onload(new Uint8Array(arrayBuffer));
      if (dep) removeRunDependency(dep);
    }, (event) => {
      if (onerror) {
        onerror();
      } else {
        throw `Loading data file "${url}" failed.`;
      }
    });
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
    if (!ptr) return 0;
    return zeroMemory(ptr, size);
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
  __table_base: "new WebAssembly.Global({'value': '{{{ POINTER_WASM_TYPE }}}', 'mutable': false}, {{{ to64(1) }}})",
#if MEMORY64 == 2
  __memory_base32: "new WebAssembly.Global({'value': 'i32', 'mutable': false}, {{{ GLOBAL_BASE }}})",
#endif
#if MEMORY64
  __table_base32: 1,
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

  $handleAllocatorInit: function() {
    Object.assign(HandleAllocator.prototype, /** @lends {HandleAllocator.prototype} */ {
      get(id) {
  #if ASSERTIONS
        assert(this.allocated[id] !== undefined, `invalid handle: ${id}`);
  #endif
        return this.allocated[id];
      },
      has(id) {
        return this.allocated[id] !== undefined;
      },
      allocate(handle) {
        var id = this.freelist.pop() || this.allocated.length;
        this.allocated[id] = handle;
        return id;
      },
      free(id) {
  #if ASSERTIONS
        assert(this.allocated[id] !== undefined);
  #endif
        // Set the slot to `undefined` rather than using `delete` here since
        // apparently arrays with holes in them can be less efficient.
        this.allocated[id] = undefined;
        this.freelist.push(id);
      }
    });
  },

  $HandleAllocator__postset: 'handleAllocatorInit()',
  $HandleAllocator__deps: ['$handleAllocatorInit'],
  $HandleAllocator__docs: '/** @constructor */',
  $HandleAllocator: function() {
    // Reserve slot 0 so that 0 is always an invalid handle
    this.allocated = [undefined];
    this.freelist = [];
  },

  $getNativeTypeSize__deps: ['$POINTER_SIZE'],
  $getNativeTypeSize: {{{ getNativeTypeSize }}},

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
DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push(
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

  if (!library[x + '__deps']) library[x + '__deps'] = [];

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
  post += "  dbg(`error: syscall may have failed with ${-ret} (${ERRNO_MESSAGES[-ret]})`);\n";
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
    "  dbg(`error: syscall failed with ${e.errno} (${ERRNO_MESSAGES[e.errno]})`);\n" +
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
  if (!WASMFS) {
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
