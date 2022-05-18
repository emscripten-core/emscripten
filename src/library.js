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
  // ==========================================================================
  // getTempRet0/setTempRet0: scratch space handling i64 return
  //
  // These are trivial wrappers around runtime functions that make these symbols
  // available to native code.
  // ==========================================================================

  getTempRet0__sig: 'i',
  getTempRet0: function() {
    return getTempRet0();
  },

  setTempRet0__sig: 'vi',
  setTempRet0: function(val) {
    setTempRet0(val);
  },

  $ptrToString: function(ptr) {
    return '0x' + ptr.toString(16).padStart(8, '0');
  },

  $zeroMemory: function(address, size) {
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
  segfault: function() { segfault(); },
  alignfault: function() { alignfault(); },
#endif

  // ==========================================================================
  // JavaScript <-> C string interop
  // ==========================================================================

  $stringToNewUTF8__deps: ['malloc'],
  $stringToNewUTF8: function(jsString) {
    var length = lengthBytesUTF8(jsString)+1;
    var cString = _malloc(length);
    stringToUTF8(jsString, cString, length);
    return cString;
  },

  exit__sig: 'vi',
#if MINIMAL_RUNTIME
  // minimal runtime doesn't do any exit cleanup handling so just
  // map exit directly to the lower-level proc_exit syscall.
  exit: 'proc_exit',
  $exit: 'exit',
#else
  exit: function(status) {
    // void _exit(int status);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/exit.html
    exit(status);
  },
#endif

  // Returns a pointer ('p'), which means an i32 on wasm32 and an i64 wasm64
  emscripten_get_heap_max__sig: 'p',
  emscripten_get_heap_max: function() {
#if ALLOW_MEMORY_GROWTH
    // Stay one Wasm page short of 4GB: while e.g. Chrome is able to allocate
    // full 4GB Wasm memories, the size will wrap back to 0 bytes in Wasm side
    // for any code that deals with heap sizes, which would require special
    // casing all heap size related code to treat 0 specially.
    return {{{ Math.min(MAXIMUM_MEMORY, FOUR_GB - WASM_PAGE_SIZE) }}};
#else // no growth
    return HEAPU8.length;
#endif
  },

#if ABORTING_MALLOC
  $abortOnCannotGrowMemory: function(requestedSize) {
#if ASSERTIONS
#if ALLOW_MEMORY_GROWTH
    abort('Cannot enlarge memory arrays to size ' + requestedSize + ' bytes (OOM). If you want malloc to return NULL (0) instead of this abort, do not link with -sABORTING_MALLOC (that is, the default when growth is enabled is to not abort, but you have overridden that)');
#else // ALLOW_MEMORY_GROWTH
    abort('Cannot enlarge memory arrays to size ' + requestedSize + ' bytes (OOM). Either (1) compile with -sINITIAL_MEMORY=X with X higher than the current value ' + HEAP8.length + ', (2) compile with -sALLOW_MEMORY_GROWTH which allows increasing the size at runtime, or (3) if you want malloc to return NULL (0) instead of this abort, compile with -sABORTING_MALLOC=0');
#endif // ALLOW_MEMORY_GROWTH
#else // ASSERTIONS
    abort('OOM');
#endif // ASSERTIONS
  },
#endif // ABORTING_MALLOC

#if TEST_MEMORY_GROWTH_FAILS
  $emscripten_realloc_buffer: function(size) {
    return false;
  },
#else

  // Grows the wasm memory to the given byte size, and updates the JS views to
  // it. Returns 1 on success, 0 on error.
  $emscripten_realloc_buffer: function(size) {
#if MEMORYPROFILER
    var oldHeapSize = buffer.byteLength;
#endif
    try {
      // round size grow request up to wasm page size (fixed 64KB per spec)
      wasmMemory.grow((size - buffer.byteLength + 65535) >>> 16); // .grow() takes a delta compared to the previous size
      updateGlobalBufferAndViews(wasmMemory.buffer);
#if MEMORYPROFILER
      if (typeof emscriptenMemoryProfiler != 'undefined') {
        emscriptenMemoryProfiler.onMemoryResize(oldHeapSize, buffer.byteLength);
      }
#endif
      return 1 /*success*/;
    } catch(e) {
#if ASSERTIONS
      err('emscripten_realloc_buffer: Attempted to grow heap from ' + buffer.byteLength  + ' bytes to ' + size + ' bytes, but got error: ' + e);
#endif
    }
    // implicit 0 return to save code size (caller will cast "undefined" into 0
    // anyhow)
  },
#endif // ~TEST_MEMORY_GROWTH_FAILS

  emscripten_resize_heap__deps: [
    'emscripten_get_heap_max',
#if ASSERTIONS == 2
    'emscripten_get_now',
#endif
#if ABORTING_MALLOC
    '$abortOnCannotGrowMemory',
#endif
#if ALLOW_MEMORY_GROWTH
    '$emscripten_realloc_buffer',
#endif
  ],
  emscripten_resize_heap: function(requestedSize) {
    var oldSize = HEAPU8.length;
    requestedSize = requestedSize >>> 0;
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
    var maxHeapSize = _emscripten_get_heap_max();
    if (requestedSize > maxHeapSize) {
#if ASSERTIONS
      err('Cannot enlarge memory, asked to go up to ' + requestedSize + ' bytes, but the limit is ' + maxHeapSize + ' bytes!');
#endif
#if ABORTING_MALLOC
      abortOnCannotGrowMemory(requestedSize);
#else
      return false;
#endif
    }

    let alignUp = (x, multiple) => x + (multiple - x % multiple) % multiple;

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
      var replacement = emscripten_realloc_buffer(newSize);
#if ASSERTIONS == 2
      var t1 = _emscripten_get_now();
      out('Heap resize call from ' + oldSize + ' to ' + newSize + ' took ' + (t1 - t0) + ' msecs. Success: ' + !!replacement);
#endif
      if (replacement) {
#if ASSERTIONS && WASM2JS
        err('Warning: Enlarging memory arrays, this is not fast! ' + [oldSize, newSize]);
#endif

#if EMSCRIPTEN_TRACING
        _emscripten_trace_js_log_message("Emscripten", "Enlarging memory arrays from " + oldSize + " to " + newSize);
        // And now report the new layout
        _emscripten_trace_report_memory_layout();
#endif
        return true;
      }
    }
#if ASSERTIONS
    err('Failed to grow the heap from ' + oldSize + ' bytes to ' + newSize + ' bytes, not enough memory!');
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
  emscripten_notify_memory_growth: function(memoryIndex) {
#if ASSERTIONS
    assert(memoryIndex == 0);
#endif
    updateGlobalBufferAndViews(wasmMemory.buffer);
  },

  system__deps: ['$setErrNo'],
  system: function(command) {
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
    setErrNo({{{ cDefine('ENOSYS') }}});
    return -1;
  },

  // ==========================================================================
  // stdlib.h
  // ==========================================================================

  // TODO: There are currently two abort() functions that get imported to asm
  // module scope: the built-in runtime function abort(), and this library
  // function _abort(). Remove one of these, importing two functions for the
  // same purpose is wasteful.
  abort__sig: 'v',
  abort: function() {
#if ASSERTIONS
    abort('native code called abort()');
#else
    abort('');
#endif
  },

  // This object can be modified by the user during startup, which affects
  // the initial values of the environment accessible by getenv.
  $ENV: {},

  getloadavg: function(loadavg, nelem) {
    // int getloadavg(double loadavg[], int nelem);
    // http://linux.die.net/man/3/getloadavg
    var limit = Math.min(nelem, 3);
    var doubleSize = {{{ Runtime.getNativeTypeSize('double') }}};
    for (var i = 0; i < limit; i++) {
      {{{ makeSetValue('loadavg', 'i * doubleSize', '0.1', 'double') }}};
    }
    return limit;
  },

  // In -Oz builds, we replace memcpy() altogether with a non-unrolled wasm
  // variant, so we should never emit emscripten_memcpy_big() in the build.
  // In STANDALONE_WASM we aviud the emscripten_memcpy_big dependency so keep
  // the wasm file standalone.
#if SHRINK_LEVEL < 2 && !STANDALONE_WASM

  emscripten_memcpy_big__sig: 'vppp',
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
  emscripten_memcpy_big__import: true,
  emscripten_memcpy_big: '= Uint8Array.prototype.copyWithin\n' +
    '  ? function(dest, src, num) { HEAPU8.copyWithin(dest, src, src + num); }\n' +
    '  : function(dest, src, num) { HEAPU8.set(HEAPU8.subarray(src, src+num), dest); }\n',
#else
  emscripten_memcpy_big: function(dest, src, num) {
    HEAPU8.copyWithin(dest, src, src + num);
  },
#endif

#endif

  // ==========================================================================
  // assert.h
  // ==========================================================================

  __assert_fail__sig: 'viiii',
  __assert_fail: function(condition, filename, line, func) {
    abort('Assertion failed: ' + UTF8ToString(condition) + ', at: ' + [filename ? UTF8ToString(filename) : 'unknown filename', line, func ? UTF8ToString(func) : 'unknown function']);
  },

  // ==========================================================================
  // time.h
  // ==========================================================================

  _emscripten_date_now__sig: 'j',
  _emscripten_date_now: function() {
    return Date.now();
  },

  _mktime_js__sig: 'ii',
  _mktime_js: function(tmPtr) {
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
    var yday = ((date.getTime() - start.getTime()) / (1000 * 60 * 60 * 24))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
    // To match expected behavior, update fields from date
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getMonth()', 'i32') }}};

    return (date.getTime() / 1000)|0;
  },

  _gmtime_js__sig: 'ipp',
  _gmtime_js: function(time, tmPtr) {
    var date = new Date({{{ makeGetValue('time', 0, 'i32') }}}*1000);
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

  _timegm_js__sig: 'ip',
  _timegm_js: function(tmPtr) {
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

    return (date.getTime() / 1000)|0;
  },

  _localtime_js__sig: 'ipp',
  _localtime_js: function(time, tmPtr) {
    var date = new Date({{{ makeGetValue('time', 0, 'i32') }}}*1000);
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getMonth()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_year, 'date.getFullYear()-1900', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getDay()', 'i32') }}};

    var start = new Date(date.getFullYear(), 0, 1);
    var yday = ((date.getTime() - start.getTime()) / (1000 * 60 * 60 * 24))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_gmtoff, '-(date.getTimezoneOffset() * 60)', 'i32') }}};

    // Attention: DST is in December in South, and some regions don't have DST at all.
    var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
    var winterOffset = start.getTimezoneOffset();
    var dst = (summerOffset != winterOffset && date.getTimezoneOffset() == Math.min(winterOffset, summerOffset))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'dst', 'i32') }}};
  },

  // musl-internal function used to implement both `asctime` and `asctime_r`
  __asctime_r__sig: 'iii',
  __asctime_r: function(tmPtr, buf) {
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

  $withStackSave__internal: true,
  $withStackSave: function(f) {
    var stack = stackSave();
    var ret = f();
    stackRestore(stack);
    return ret;
  },

  // TODO: Initialize these to defaults on startup from system settings.
  // Note: glibc has one fewer underscore for all of these. Also used in other related functions (timegm)
  _tzset_js__deps: ['tzset_impl'],
  _tzset_js__sig: 'vppp',
  _tzset_js: function(timezone, daylight, tzname) {
    // TODO: Use (malleable) environment variables instead of system settings.
    if (__tzset_js.called) return;
    __tzset_js.called = true;
    _tzset_impl(timezone, daylight, tzname);
  },

  tzset_impl__internal: true,
  tzset_impl__proxy: 'sync',
  tzset_impl__sig: 'viii',
  tzset_impl__deps: [
#if MINIMAL_RUNTIME
    '$allocateUTF8'
#endif
  ],
  tzset_impl: function(timezone, daylight, tzname) {
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
    {{{ makeSetValue('timezone', '0', 'stdTimezoneOffset * 60', 'i32') }}};

    {{{ makeSetValue('daylight', '0', 'Number(winterOffset != summerOffset)', 'i32') }}};

    function extractZone(date) {
      var match = date.toTimeString().match(/\(([A-Za-z ]+)\)$/);
      return match ? match[1] : "GMT";
    };
    var winterName = extractZone(winter);
    var summerName = extractZone(summer);
    var winterNamePtr = allocateUTF8(winterName);
    var summerNamePtr = allocateUTF8(summerName);
    if (summerOffset < winterOffset) {
      // Northern hemisphere
      {{{ makeSetValue('tzname', '0', 'winterNamePtr', POINTER_TYPE) }}};
      {{{ makeSetValue('tzname', Runtime.POINTER_SIZE, 'summerNamePtr', POINTER_TYPE) }}};
    } else {
      {{{ makeSetValue('tzname', '0', 'summerNamePtr', POINTER_TYPE) }}};
      {{{ makeSetValue('tzname', Runtime.POINTER_SIZE, 'winterNamePtr', POINTER_TYPE) }}};
    }
  },

  __map_file__deps: ['$setErrNo'],
  __map_file: function(pathname, size) {
    setErrNo({{{ cDefine('EPERM') }}});
    return -1;
  },

  _MONTH_DAYS_REGULAR: [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],
  _MONTH_DAYS_LEAP: [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],

  _isLeapYear: function(year) {
      return year%4 === 0 && (year%100 !== 0 || year%400 === 0);
  },

  _arraySum: function(array, index) {
    var sum = 0;
    for (var i = 0; i <= index; sum += array[i++]) {
      // no-op
    }
    return sum;
  },

  _addDays__deps: ['_isLeapYear', '_MONTH_DAYS_LEAP', '_MONTH_DAYS_REGULAR'],
  _addDays: function(date, days) {
    var newDate = new Date(date.getTime());
    while (days > 0) {
      var leap = __isLeapYear(newDate.getFullYear());
      var currentMonth = newDate.getMonth();
      var daysInCurrentMonth = (leap ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR)[currentMonth];

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
  strftime__deps: ['_isLeapYear', '_arraySum', '_addDays', '_MONTH_DAYS_REGULAR', '_MONTH_DAYS_LEAP'
#if MINIMAL_RUNTIME
    , '$intArrayFromString', '$writeArrayToMemory'
#endif
  ],
  strftime__sig: 'ppppp',
  strftime: function(s, maxsize, format, tm) {
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
        var thisDate = __addDays(new Date(date.tm_year+1900, 0, 1), date.tm_yday);

        var janFourthThisYear = new Date(thisDate.getFullYear(), 0, 4);
        var janFourthNextYear = new Date(thisDate.getFullYear()+1, 0, 4);

        var firstWeekStartThisYear = getFirstWeekStartDate(janFourthThisYear);
        var firstWeekStartNextYear = getFirstWeekStartDate(janFourthNextYear);

        if (compareByDay(firstWeekStartThisYear, thisDate) <= 0) {
          // this date is after the start of the first week of this year
          if (compareByDay(firstWeekStartNextYear, thisDate) <= 0) {
            return thisDate.getFullYear()+1;
          } else {
            return thisDate.getFullYear();
          }
        } else {
          return thisDate.getFullYear()-1;
        }
    }

    var EXPANSION_RULES_2 = {
      '%a': function(date) {
        return WEEKDAYS[date.tm_wday].substring(0,3);
      },
      '%A': function(date) {
        return WEEKDAYS[date.tm_wday];
      },
      '%b': function(date) {
        return MONTHS[date.tm_mon].substring(0,3);
      },
      '%B': function(date) {
        return MONTHS[date.tm_mon];
      },
      '%C': function(date) {
        var year = date.tm_year+1900;
        return leadingNulls((year/100)|0,2);
      },
      '%d': function(date) {
        return leadingNulls(date.tm_mday, 2);
      },
      '%e': function(date) {
        return leadingSomething(date.tm_mday, 2, ' ');
      },
      '%g': function(date) {
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
      '%G': function(date) {
        return getWeekBasedYear(date);
      },
      '%H': function(date) {
        return leadingNulls(date.tm_hour, 2);
      },
      '%I': function(date) {
        var twelveHour = date.tm_hour;
        if (twelveHour == 0) twelveHour = 12;
        else if (twelveHour > 12) twelveHour -= 12;
        return leadingNulls(twelveHour, 2);
      },
      '%j': function(date) {
        // Day of the year (001-366)
        return leadingNulls(date.tm_mday+__arraySum(__isLeapYear(date.tm_year+1900) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, date.tm_mon-1), 3);
      },
      '%m': function(date) {
        return leadingNulls(date.tm_mon+1, 2);
      },
      '%M': function(date) {
        return leadingNulls(date.tm_min, 2);
      },
      '%n': function() {
        return '\n';
      },
      '%p': function(date) {
        if (date.tm_hour >= 0 && date.tm_hour < 12) {
          return 'AM';
        } else {
          return 'PM';
        }
      },
      '%S': function(date) {
        return leadingNulls(date.tm_sec, 2);
      },
      '%t': function() {
        return '\t';
      },
      '%u': function(date) {
        return date.tm_wday || 7;
      },
      '%U': function(date) {
        var days = date.tm_yday + 7 - date.tm_wday;
        return leadingNulls(Math.floor(days / 7), 2);
      },
      '%V': function(date) {
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
          if (dec31 == 4 || (dec31 == 5 && __isLeapYear(date.tm_year%400-1))) {
            val++;
          }
        } else if (val == 53) {
          // If 1 January is not a Thursday, and not a Wednesday of a
          // leap year, then this year has only 52 weeks.
          var jan1 = (date.tm_wday + 371 - date.tm_yday) % 7;
          if (jan1 != 4 && (jan1 != 3 || !__isLeapYear(date.tm_year)))
            val = 1;
        }
        return leadingNulls(val, 2);
      },
      '%w': function(date) {
        return date.tm_wday;
      },
      '%W': function(date) {
        var days = date.tm_yday + 7 - ((date.tm_wday + 6) % 7);
        return leadingNulls(Math.floor(days / 7), 2);
      },
      '%y': function(date) {
        // Replaced by the last two digits of the year as a decimal number [00,99]. [ tm_year]
        return (date.tm_year+1900).toString().substring(2);
      },
      '%Y': function(date) {
        // Replaced by the year as a decimal number (for example, 1997). [ tm_year]
        return date.tm_year+1900;
      },
      '%z': function(date) {
        // Replaced by the offset from UTC in the ISO 8601:2000 standard format ( +hhmm or -hhmm ).
        // For example, "-0430" means 4 hours 30 minutes behind UTC (west of Greenwich).
        var off = date.tm_gmtoff;
        var ahead = off >= 0;
        off = Math.abs(off) / 60;
        // convert from minutes into hhmm format (which means 60 minutes = 100 units)
        off = (off / 60)*100 + (off % 60);
        return (ahead ? '+' : '-') + String("0000" + off).slice(-4);
      },
      '%Z': function(date) {
        return date.tm_zone;
      },
      '%%': function() {
        return '%';
      }
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
  strftime_l: function(s, maxsize, format, tm) {
    return _strftime(s, maxsize, format, tm); // no locale support yet
  },

  strptime__deps: ['_isLeapYear', '_arraySum', '_addDays', '_MONTH_DAYS_REGULAR', '_MONTH_DAYS_LEAP', '$jstoi_q'
#if MINIMAL_RUNTIME
    , '$intArrayFromString'
#endif
  ],
  strptime: function(buf, format, tm) {
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
        var leapYear = __isLeapYear(date.year);
        for (var month=0; month<12; ++month) {
          var daysUntilMonth = __arraySum(leapYear ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, month-1);
          if (day<=daysUntilMonth+(leapYear ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR)[month]) {
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
            endDate = __addDays(janFirst, weekDayNumber+7*(weekNumber-1));
          } else {
            // Jan 1st is not a Sunday, and, hence still in the 0th CW
            endDate = __addDays(janFirst, 7-janFirst.getDay()+weekDayNumber+7*(weekNumber-1));
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
             endDate = __addDays(janFirst, weekDayNumber+7*(weekNumber-1));
          } else {
            // Jan 1st is not a Monday, and, hence still in the 0th CW
            endDate = __addDays(janFirst, 7-janFirst.getDay()+1+weekDayNumber+7*(weekNumber-1));
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
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_yday, '__arraySum(__isLeapYear(fullDate.getFullYear()) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, fullDate.getMonth()-1)+fullDate.getDate()-1', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_isdst, '0', 'i32') }}};

      // we need to convert the matched sequence into an integer array to take care of UTF-8 characters > 0x7F
      // TODO: not sure that intArrayFromString handles all unicode characters correctly
      return buf+intArrayFromString(matches[0]).length-1;
    }

    return 0;
  },
  strptime_l__deps: ['strptime'],
  strptime_l: function(buf, format, tm) {
    return _strptime(buf, format, tm); // no locale support yet
  },

  // ==========================================================================
  // setjmp.h
  // ==========================================================================

#if SUPPORT_LONGJMP == 'emscripten'
  _emscripten_throw_longjmp__sig: 'v',
  _emscripten_throw_longjmp: function() { throw Infinity; },
#endif

#if !SUPPORT_LONGJMP
#if !INCLUDE_FULL_LIBRARY
  // These are in order to print helpful error messages when either longjmp of
  // setjmp is used.
  longjmp__deps: [function() {
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
  _emscripten_throw_longjmp: function() { error('longjmp support was disabled (SUPPORT_LONGJMP=0), but it is required by the code (either set SUPPORT_LONGJMP=1, or remove uses of it in the project)'); },
  get _emscripten_throw_longjmp__deps() {
    return this.longjmp__deps;
  },
#endif
  // will never be emitted, as the dep errors at compile time
  longjmp: function(env, value) {
    abort('longjmp not supported');
  },
  setjmp: function(env, value) {
    abort('setjmp not supported');
  },
#endif

  // ==========================================================================
  // errno.h
  // ==========================================================================

  $ERRNO_CODES__postset: `ERRNO_CODES = {
    'EPERM': {{{ cDefine('EPERM') }}},
    'ENOENT': {{{ cDefine('ENOENT') }}},
    'ESRCH': {{{ cDefine('ESRCH') }}},
    'EINTR': {{{ cDefine('EINTR') }}},
    'EIO': {{{ cDefine('EIO') }}},
    'ENXIO': {{{ cDefine('ENXIO') }}},
    'E2BIG': {{{ cDefine('E2BIG') }}},
    'ENOEXEC': {{{ cDefine('ENOEXEC') }}},
    'EBADF': {{{ cDefine('EBADF') }}},
    'ECHILD': {{{ cDefine('ECHILD') }}},
    'EAGAIN': {{{ cDefine('EAGAIN') }}},
    'EWOULDBLOCK': {{{ cDefine('EWOULDBLOCK') }}},
    'ENOMEM': {{{ cDefine('ENOMEM') }}},
    'EACCES': {{{ cDefine('EACCES') }}},
    'EFAULT': {{{ cDefine('EFAULT') }}},
    'ENOTBLK': {{{ cDefine('ENOTBLK') }}},
    'EBUSY': {{{ cDefine('EBUSY') }}},
    'EEXIST': {{{ cDefine('EEXIST') }}},
    'EXDEV': {{{ cDefine('EXDEV') }}},
    'ENODEV': {{{ cDefine('ENODEV') }}},
    'ENOTDIR': {{{ cDefine('ENOTDIR') }}},
    'EISDIR': {{{ cDefine('EISDIR') }}},
    'EINVAL': {{{ cDefine('EINVAL') }}},
    'ENFILE': {{{ cDefine('ENFILE') }}},
    'EMFILE': {{{ cDefine('EMFILE') }}},
    'ENOTTY': {{{ cDefine('ENOTTY') }}},
    'ETXTBSY': {{{ cDefine('ETXTBSY') }}},
    'EFBIG': {{{ cDefine('EFBIG') }}},
    'ENOSPC': {{{ cDefine('ENOSPC') }}},
    'ESPIPE': {{{ cDefine('ESPIPE') }}},
    'EROFS': {{{ cDefine('EROFS') }}},
    'EMLINK': {{{ cDefine('EMLINK') }}},
    'EPIPE': {{{ cDefine('EPIPE') }}},
    'EDOM': {{{ cDefine('EDOM') }}},
    'ERANGE': {{{ cDefine('ERANGE') }}},
    'ENOMSG': {{{ cDefine('ENOMSG') }}},
    'EIDRM': {{{ cDefine('EIDRM') }}},
    'ECHRNG': {{{ cDefine('ECHRNG') }}},
    'EL2NSYNC': {{{ cDefine('EL2NSYNC') }}},
    'EL3HLT': {{{ cDefine('EL3HLT') }}},
    'EL3RST': {{{ cDefine('EL3RST') }}},
    'ELNRNG': {{{ cDefine('ELNRNG') }}},
    'EUNATCH': {{{ cDefine('EUNATCH') }}},
    'ENOCSI': {{{ cDefine('ENOCSI') }}},
    'EL2HLT': {{{ cDefine('EL2HLT') }}},
    'EDEADLK': {{{ cDefine('EDEADLK') }}},
    'ENOLCK': {{{ cDefine('ENOLCK') }}},
    'EBADE': {{{ cDefine('EBADE') }}},
    'EBADR': {{{ cDefine('EBADR') }}},
    'EXFULL': {{{ cDefine('EXFULL') }}},
    'ENOANO': {{{ cDefine('ENOANO') }}},
    'EBADRQC': {{{ cDefine('EBADRQC') }}},
    'EBADSLT': {{{ cDefine('EBADSLT') }}},
    'EDEADLOCK': {{{ cDefine('EDEADLOCK') }}},
    'EBFONT': {{{ cDefine('EBFONT') }}},
    'ENOSTR': {{{ cDefine('ENOSTR') }}},
    'ENODATA': {{{ cDefine('ENODATA') }}},
    'ETIME': {{{ cDefine('ETIME') }}},
    'ENOSR': {{{ cDefine('ENOSR') }}},
    'ENONET': {{{ cDefine('ENONET') }}},
    'ENOPKG': {{{ cDefine('ENOPKG') }}},
    'EREMOTE': {{{ cDefine('EREMOTE') }}},
    'ENOLINK': {{{ cDefine('ENOLINK') }}},
    'EADV': {{{ cDefine('EADV') }}},
    'ESRMNT': {{{ cDefine('ESRMNT') }}},
    'ECOMM': {{{ cDefine('ECOMM') }}},
    'EPROTO': {{{ cDefine('EPROTO') }}},
    'EMULTIHOP': {{{ cDefine('EMULTIHOP') }}},
    'EDOTDOT': {{{ cDefine('EDOTDOT') }}},
    'EBADMSG': {{{ cDefine('EBADMSG') }}},
    'ENOTUNIQ': {{{ cDefine('ENOTUNIQ') }}},
    'EBADFD': {{{ cDefine('EBADFD') }}},
    'EREMCHG': {{{ cDefine('EREMCHG') }}},
    'ELIBACC': {{{ cDefine('ELIBACC') }}},
    'ELIBBAD': {{{ cDefine('ELIBBAD') }}},
    'ELIBSCN': {{{ cDefine('ELIBSCN') }}},
    'ELIBMAX': {{{ cDefine('ELIBMAX') }}},
    'ELIBEXEC': {{{ cDefine('ELIBEXEC') }}},
    'ENOSYS': {{{ cDefine('ENOSYS') }}},
    'ENOTEMPTY': {{{ cDefine('ENOTEMPTY') }}},
    'ENAMETOOLONG': {{{ cDefine('ENAMETOOLONG') }}},
    'ELOOP': {{{ cDefine('ELOOP') }}},
    'EOPNOTSUPP': {{{ cDefine('EOPNOTSUPP') }}},
    'EPFNOSUPPORT': {{{ cDefine('EPFNOSUPPORT') }}},
    'ECONNRESET': {{{ cDefine('ECONNRESET') }}},
    'ENOBUFS': {{{ cDefine('ENOBUFS') }}},
    'EAFNOSUPPORT': {{{ cDefine('EAFNOSUPPORT') }}},
    'EPROTOTYPE': {{{ cDefine('EPROTOTYPE') }}},
    'ENOTSOCK': {{{ cDefine('ENOTSOCK') }}},
    'ENOPROTOOPT': {{{ cDefine('ENOPROTOOPT') }}},
    'ESHUTDOWN': {{{ cDefine('ESHUTDOWN') }}},
    'ECONNREFUSED': {{{ cDefine('ECONNREFUSED') }}},
    'EADDRINUSE': {{{ cDefine('EADDRINUSE') }}},
    'ECONNABORTED': {{{ cDefine('ECONNABORTED') }}},
    'ENETUNREACH': {{{ cDefine('ENETUNREACH') }}},
    'ENETDOWN': {{{ cDefine('ENETDOWN') }}},
    'ETIMEDOUT': {{{ cDefine('ETIMEDOUT') }}},
    'EHOSTDOWN': {{{ cDefine('EHOSTDOWN') }}},
    'EHOSTUNREACH': {{{ cDefine('EHOSTUNREACH') }}},
    'EINPROGRESS': {{{ cDefine('EINPROGRESS') }}},
    'EALREADY': {{{ cDefine('EALREADY') }}},
    'EDESTADDRREQ': {{{ cDefine('EDESTADDRREQ') }}},
    'EMSGSIZE': {{{ cDefine('EMSGSIZE') }}},
    'EPROTONOSUPPORT': {{{ cDefine('EPROTONOSUPPORT') }}},
    'ESOCKTNOSUPPORT': {{{ cDefine('ESOCKTNOSUPPORT') }}},
    'EADDRNOTAVAIL': {{{ cDefine('EADDRNOTAVAIL') }}},
    'ENETRESET': {{{ cDefine('ENETRESET') }}},
    'EISCONN': {{{ cDefine('EISCONN') }}},
    'ENOTCONN': {{{ cDefine('ENOTCONN') }}},
    'ETOOMANYREFS': {{{ cDefine('ETOOMANYREFS') }}},
    'EUSERS': {{{ cDefine('EUSERS') }}},
    'EDQUOT': {{{ cDefine('EDQUOT') }}},
    'ESTALE': {{{ cDefine('ESTALE') }}},
    'ENOTSUP': {{{ cDefine('ENOTSUP') }}},
    'ENOMEDIUM': {{{ cDefine('ENOMEDIUM') }}},
    'EILSEQ': {{{ cDefine('EILSEQ') }}},
    'EOVERFLOW': {{{ cDefine('EOVERFLOW') }}},
    'ECANCELED': {{{ cDefine('ECANCELED') }}},
    'ENOTRECOVERABLE': {{{ cDefine('ENOTRECOVERABLE') }}},
    'EOWNERDEAD': {{{ cDefine('EOWNERDEAD') }}},
    'ESTRPIPE': {{{ cDefine('ESTRPIPE') }}},
  };`,
  $ERRNO_CODES: {},
  $ERRNO_MESSAGES: {
    0: 'Success',
    {{{ cDefine('EPERM') }}}: 'Not super-user',
    {{{ cDefine('ENOENT') }}}: 'No such file or directory',
    {{{ cDefine('ESRCH') }}}: 'No such process',
    {{{ cDefine('EINTR') }}}: 'Interrupted system call',
    {{{ cDefine('EIO') }}}: 'I/O error',
    {{{ cDefine('ENXIO') }}}: 'No such device or address',
    {{{ cDefine('E2BIG') }}}: 'Arg list too long',
    {{{ cDefine('ENOEXEC') }}}: 'Exec format error',
    {{{ cDefine('EBADF') }}}: 'Bad file number',
    {{{ cDefine('ECHILD') }}}: 'No children',
    {{{ cDefine('EWOULDBLOCK') }}}: 'No more processes',
    {{{ cDefine('ENOMEM') }}}: 'Not enough core',
    {{{ cDefine('EACCES') }}}: 'Permission denied',
    {{{ cDefine('EFAULT') }}}: 'Bad address',
    {{{ cDefine('ENOTBLK') }}}: 'Block device required',
    {{{ cDefine('EBUSY') }}}: 'Mount device busy',
    {{{ cDefine('EEXIST') }}}: 'File exists',
    {{{ cDefine('EXDEV') }}}: 'Cross-device link',
    {{{ cDefine('ENODEV') }}}: 'No such device',
    {{{ cDefine('ENOTDIR') }}}: 'Not a directory',
    {{{ cDefine('EISDIR') }}}: 'Is a directory',
    {{{ cDefine('EINVAL') }}}: 'Invalid argument',
    {{{ cDefine('ENFILE') }}}: 'Too many open files in system',
    {{{ cDefine('EMFILE') }}}: 'Too many open files',
    {{{ cDefine('ENOTTY') }}}: 'Not a typewriter',
    {{{ cDefine('ETXTBSY') }}}: 'Text file busy',
    {{{ cDefine('EFBIG') }}}: 'File too large',
    {{{ cDefine('ENOSPC') }}}: 'No space left on device',
    {{{ cDefine('ESPIPE') }}}: 'Illegal seek',
    {{{ cDefine('EROFS') }}}: 'Read only file system',
    {{{ cDefine('EMLINK') }}}: 'Too many links',
    {{{ cDefine('EPIPE') }}}: 'Broken pipe',
    {{{ cDefine('EDOM') }}}: 'Math arg out of domain of func',
    {{{ cDefine('ERANGE') }}}: 'Math result not representable',
    {{{ cDefine('ENOMSG') }}}: 'No message of desired type',
    {{{ cDefine('EIDRM') }}}: 'Identifier removed',
    {{{ cDefine('ECHRNG') }}}: 'Channel number out of range',
    {{{ cDefine('EL2NSYNC') }}}: 'Level 2 not synchronized',
    {{{ cDefine('EL3HLT') }}}: 'Level 3 halted',
    {{{ cDefine('EL3RST') }}}: 'Level 3 reset',
    {{{ cDefine('ELNRNG') }}}: 'Link number out of range',
    {{{ cDefine('EUNATCH') }}}: 'Protocol driver not attached',
    {{{ cDefine('ENOCSI') }}}: 'No CSI structure available',
    {{{ cDefine('EL2HLT') }}}: 'Level 2 halted',
    {{{ cDefine('EDEADLK') }}}: 'Deadlock condition',
    {{{ cDefine('ENOLCK') }}}: 'No record locks available',
    {{{ cDefine('EBADE') }}}: 'Invalid exchange',
    {{{ cDefine('EBADR') }}}: 'Invalid request descriptor',
    {{{ cDefine('EXFULL') }}}: 'Exchange full',
    {{{ cDefine('ENOANO') }}}: 'No anode',
    {{{ cDefine('EBADRQC') }}}: 'Invalid request code',
    {{{ cDefine('EBADSLT') }}}: 'Invalid slot',
    {{{ cDefine('EDEADLOCK') }}}: 'File locking deadlock error',
    {{{ cDefine('EBFONT') }}}: 'Bad font file fmt',
    {{{ cDefine('ENOSTR') }}}: 'Device not a stream',
    {{{ cDefine('ENODATA') }}}: 'No data (for no delay io)',
    {{{ cDefine('ETIME') }}}: 'Timer expired',
    {{{ cDefine('ENOSR') }}}: 'Out of streams resources',
    {{{ cDefine('ENONET') }}}: 'Machine is not on the network',
    {{{ cDefine('ENOPKG') }}}: 'Package not installed',
    {{{ cDefine('EREMOTE') }}}: 'The object is remote',
    {{{ cDefine('ENOLINK') }}}: 'The link has been severed',
    {{{ cDefine('EADV') }}}: 'Advertise error',
    {{{ cDefine('ESRMNT') }}}: 'Srmount error',
    {{{ cDefine('ECOMM') }}}: 'Communication error on send',
    {{{ cDefine('EPROTO') }}}: 'Protocol error',
    {{{ cDefine('EMULTIHOP') }}}: 'Multihop attempted',
    {{{ cDefine('EDOTDOT') }}}: 'Cross mount point (not really error)',
    {{{ cDefine('EBADMSG') }}}: 'Trying to read unreadable message',
    {{{ cDefine('ENOTUNIQ') }}}: 'Given log. name not unique',
    {{{ cDefine('EBADFD') }}}: 'f.d. invalid for this operation',
    {{{ cDefine('EREMCHG') }}}: 'Remote address changed',
    {{{ cDefine('ELIBACC') }}}: 'Can   access a needed shared lib',
    {{{ cDefine('ELIBBAD') }}}: 'Accessing a corrupted shared lib',
    {{{ cDefine('ELIBSCN') }}}: '.lib section in a.out corrupted',
    {{{ cDefine('ELIBMAX') }}}: 'Attempting to link in too many libs',
    {{{ cDefine('ELIBEXEC') }}}: 'Attempting to exec a shared library',
    {{{ cDefine('ENOSYS') }}}: 'Function not implemented',
    {{{ cDefine('ENOTEMPTY') }}}: 'Directory not empty',
    {{{ cDefine('ENAMETOOLONG') }}}: 'File or path name too long',
    {{{ cDefine('ELOOP') }}}: 'Too many symbolic links',
    {{{ cDefine('EOPNOTSUPP') }}}: 'Operation not supported on transport endpoint',
    {{{ cDefine('EPFNOSUPPORT') }}}: 'Protocol family not supported',
    {{{ cDefine('ECONNRESET') }}}: 'Connection reset by peer',
    {{{ cDefine('ENOBUFS') }}}: 'No buffer space available',
    {{{ cDefine('EAFNOSUPPORT') }}}: 'Address family not supported by protocol family',
    {{{ cDefine('EPROTOTYPE') }}}: 'Protocol wrong type for socket',
    {{{ cDefine('ENOTSOCK') }}}: 'Socket operation on non-socket',
    {{{ cDefine('ENOPROTOOPT') }}}: 'Protocol not available',
    {{{ cDefine('ESHUTDOWN') }}}: 'Can\'t send after socket shutdown',
    {{{ cDefine('ECONNREFUSED') }}}: 'Connection refused',
    {{{ cDefine('EADDRINUSE') }}}: 'Address already in use',
    {{{ cDefine('ECONNABORTED') }}}: 'Connection aborted',
    {{{ cDefine('ENETUNREACH') }}}: 'Network is unreachable',
    {{{ cDefine('ENETDOWN') }}}: 'Network interface is not configured',
    {{{ cDefine('ETIMEDOUT') }}}: 'Connection timed out',
    {{{ cDefine('EHOSTDOWN') }}}: 'Host is down',
    {{{ cDefine('EHOSTUNREACH') }}}: 'Host is unreachable',
    {{{ cDefine('EINPROGRESS') }}}: 'Connection already in progress',
    {{{ cDefine('EALREADY') }}}: 'Socket already connected',
    {{{ cDefine('EDESTADDRREQ') }}}: 'Destination address required',
    {{{ cDefine('EMSGSIZE') }}}: 'Message too long',
    {{{ cDefine('EPROTONOSUPPORT') }}}: 'Unknown protocol',
    {{{ cDefine('ESOCKTNOSUPPORT') }}}: 'Socket type not supported',
    {{{ cDefine('EADDRNOTAVAIL') }}}: 'Address not available',
    {{{ cDefine('ENETRESET') }}}: 'Connection reset by network',
    {{{ cDefine('EISCONN') }}}: 'Socket is already connected',
    {{{ cDefine('ENOTCONN') }}}: 'Socket is not connected',
    {{{ cDefine('ETOOMANYREFS') }}}: 'Too many references',
    {{{ cDefine('EUSERS') }}}: 'Too many users',
    {{{ cDefine('EDQUOT') }}}: 'Quota exceeded',
    {{{ cDefine('ESTALE') }}}: 'Stale file handle',
    {{{ cDefine('ENOTSUP') }}}: 'Not supported',
    {{{ cDefine('ENOMEDIUM') }}}: 'No medium (in tape drive)',
    {{{ cDefine('EILSEQ') }}}: 'Illegal byte sequence',
    {{{ cDefine('EOVERFLOW') }}}: 'Value too large for defined data type',
    {{{ cDefine('ECANCELED') }}}: 'Operation canceled',
    {{{ cDefine('ENOTRECOVERABLE') }}}: 'State not recoverable',
    {{{ cDefine('EOWNERDEAD') }}}: 'Previous owner died',
    {{{ cDefine('ESTRPIPE') }}}: 'Streams pipe error',
  },
#if SUPPORT_ERRNO
  $setErrNo__deps: ['__errno_location'],
  $setErrNo: function(value) {
    {{{makeSetValue("___errno_location()", 0, 'value', 'i32') }}};
    return value;
  },
#else
  $setErrNo: function(value) {
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

  $inetPton4: function(str) {
    var b = str.split('.');
    for (var i = 0; i < 4; i++) {
      var tmp = Number(b[i]);
      if (isNaN(tmp)) return null;
      b[i] = tmp;
    }
    return (b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24)) >>> 0;
  },
  $inetNtop4: function(addr) {
    return (addr & 0xff) + '.' + ((addr >> 8) & 0xff) + '.' + ((addr >> 16) & 0xff) + '.' + ((addr >> 24) & 0xff)
  },
  $inetPton6__deps: ['htons', '$jstoi_q'],
  $inetPton6: function(str) {
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
  $inetNtop6: function(ints) {
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
  $readSockaddr: function (sa, salen) {
    // family / port offsets are common to both sockaddr_in and sockaddr_in6
    var family = {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in.sin_family, 'i16') }}};
    var port = _ntohs({{{ makeGetValue('sa', C_STRUCTS.sockaddr_in.sin_port, 'u16') }}});
    var addr;

    switch (family) {
      case {{{ cDefine('AF_INET') }}}:
        if (salen !== {{{ C_STRUCTS.sockaddr_in.__size__ }}}) {
          return { errno: {{{ cDefine('EINVAL') }}} };
        }
        addr = {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in.sin_addr.s_addr, 'i32') }}};
        addr = inetNtop4(addr);
        break;
      case {{{ cDefine('AF_INET6') }}}:
        if (salen !== {{{ C_STRUCTS.sockaddr_in6.__size__ }}}) {
          return { errno: {{{ cDefine('EINVAL') }}} };
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
        return { errno: {{{ cDefine('EAFNOSUPPORT') }}} };
    }

    return { family: family, addr: addr, port: port };
  },
  $writeSockaddr__docs: '/** @param {number=} addrlen */',
  $writeSockaddr__deps: ['$Sockets', '$inetPton4', '$inetPton6', '$zeroMemory'],
  $writeSockaddr: function (sa, family, addr, port, addrlen) {
    switch (family) {
      case {{{ cDefine('AF_INET') }}}:
        addr = inetPton4(addr);
        zeroMemory(sa, {{{ C_STRUCTS.sockaddr_in.__size__ }}});
        if (addrlen) {
          {{{ makeSetValue('addrlen', 0, C_STRUCTS.sockaddr_in.__size__, 'i32') }}};
        }
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in.sin_family, 'family', 'i16') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in.sin_addr.s_addr, 'addr', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in.sin_port, '_htons(port)', 'i16') }}};
        break;
      case {{{ cDefine('AF_INET6') }}}:
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
        return {{{ cDefine('EAFNOSUPPORT') }}};
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

    lookup_name: function (name) {
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

    lookup_addr: function (addr) {
      if (DNS.address_map.names[addr]) {
        return DNS.address_map.names[addr];
      }

      return null;
    }
  },

  // note: lots of leaking here!
  gethostbyaddr__deps: ['$DNS', '$getHostByName', '$inetNtop4', '$setErrNo'],
  gethostbyaddr__proxy: 'sync',
  gethostbyaddr__sig: 'iiii',
  gethostbyaddr: function (addr, addrlen, type) {
    if (type !== {{{ cDefine('AF_INET') }}}) {
      setErrNo({{{ cDefine('EAFNOSUPPORT') }}});
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
  gethostbyname__sig: 'ii',
  gethostbyname: function(name) {
    return getHostByName(UTF8ToString(name));
  },

  $getHostByName__deps: ['malloc', '$DNS', '$inetPton4'],
  $getHostByName: function(name) {
    // generate hostent
    var ret = _malloc({{{ C_STRUCTS.hostent.__size__ }}}); // XXX possibly leaked, as are others here
    var nameBuf = {{{ makeMalloc('getHostByName', 'name.length+1') }}};
    stringToUTF8(name, nameBuf, name.length+1);
    {{{ makeSetValue('ret', C_STRUCTS.hostent.h_name, 'nameBuf', POINTER_TYPE) }}};
    var aliasesBuf = _malloc(4);
    {{{ makeSetValue('aliasesBuf', '0', '0', POINTER_TYPE) }}};
    {{{ makeSetValue('ret', C_STRUCTS.hostent.h_aliases, 'aliasesBuf', 'i8**') }}};
    var afinet = {{{ cDefine('AF_INET') }}};
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
  gethostbyname_r__sig: 'iiiiiii',
  gethostbyname_r: function(name, ret, buf, buflen, out, err) {
    var data = _gethostbyname(name);
    _memcpy(ret, data, {{{ C_STRUCTS.hostent.__size__ }}});
    _free(data);
    {{{ makeSetValue('err', '0', '0', 'i32') }}};
    {{{ makeSetValue('out', '0', 'ret', '*') }}};
    return 0;
  },

  getaddrinfo__deps: ['$Sockets', '$DNS', '$inetPton4', '$inetNtop4', '$inetPton6', '$inetNtop6', '$writeSockaddr'],
  getaddrinfo__proxy: 'sync',
  getaddrinfo__sig: 'iiiii',
  getaddrinfo: function(node, service, hint, out) {
    // Note getaddrinfo currently only returns a single addrinfo with ai_next defaulting to NULL. When NULL
    // hints are specified or ai_family set to AF_UNSPEC or ai_socktype or ai_protocol set to 0 then we
    // really should provide a linked list of suitable addrinfo values.
    var addrs = [];
    var canon = null;
    var addr = 0;
    var port = 0;
    var flags = 0;
    var family = {{{ cDefine('AF_UNSPEC') }}};
    var type = 0;
    var proto = 0;
    var ai, last;

    function allocaddrinfo(family, type, proto, canon, addr, port) {
      var sa, salen, ai;
      var errno;

      salen = family === {{{ cDefine('AF_INET6') }}} ?
        {{{ C_STRUCTS.sockaddr_in6.__size__ }}} :
        {{{ C_STRUCTS.sockaddr_in.__size__ }}};
      addr = family === {{{ cDefine('AF_INET6') }}} ?
        inetNtop6(addr) :
        inetNtop4(addr);
      sa = _malloc(salen);
      errno = writeSockaddr(sa, family, addr, port);
      assert(!errno);

      ai = _malloc({{{ C_STRUCTS.addrinfo.__size__ }}});
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_family, 'family', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_socktype, 'type', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_protocol, 'proto', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_canonname, 'canon', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_addr, 'sa', '*') }}};
      if (family === {{{ cDefine('AF_INET6') }}}) {
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
      proto = type === {{{ cDefine('SOCK_DGRAM') }}} ? {{{ cDefine('IPPROTO_UDP') }}} : {{{ cDefine('IPPROTO_TCP') }}};
    }
    if (!type && proto) {
      type = proto === {{{ cDefine('IPPROTO_UDP') }}} ? {{{ cDefine('SOCK_DGRAM') }}} : {{{ cDefine('SOCK_STREAM') }}};
    }

    // If type or proto are set to zero in hints we should really be returning multiple addrinfo values, but for
    // now default to a TCP STREAM socket so we can at least return a sensible addrinfo given NULL hints.
    if (proto === 0) {
      proto = {{{ cDefine('IPPROTO_TCP') }}};
    }
    if (type === 0) {
      type = {{{ cDefine('SOCK_STREAM') }}};
    }

    if (!node && !service) {
      return {{{ cDefine('EAI_NONAME') }}};
    }
    if (flags & ~({{{ cDefine('AI_PASSIVE') }}}|{{{ cDefine('AI_CANONNAME') }}}|{{{ cDefine('AI_NUMERICHOST') }}}|
        {{{ cDefine('AI_NUMERICSERV') }}}|{{{ cDefine('AI_V4MAPPED') }}}|{{{ cDefine('AI_ALL') }}}|{{{ cDefine('AI_ADDRCONFIG') }}})) {
      return {{{ cDefine('EAI_BADFLAGS') }}};
    }
    if (hint !== 0 && ({{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_flags, 'i32') }}} & {{{ cDefine('AI_CANONNAME') }}}) && !node) {
      return {{{ cDefine('EAI_BADFLAGS') }}};
    }
    if (flags & {{{ cDefine('AI_ADDRCONFIG') }}}) {
      // TODO
      return {{{ cDefine('EAI_NONAME') }}};
    }
    if (type !== 0 && type !== {{{ cDefine('SOCK_STREAM') }}} && type !== {{{ cDefine('SOCK_DGRAM') }}}) {
      return {{{ cDefine('EAI_SOCKTYPE') }}};
    }
    if (family !== {{{ cDefine('AF_UNSPEC') }}} && family !== {{{ cDefine('AF_INET') }}} && family !== {{{ cDefine('AF_INET6') }}}) {
      return {{{ cDefine('EAI_FAMILY') }}};
    }

    if (service) {
      service = UTF8ToString(service);
      port = parseInt(service, 10);

      if (isNaN(port)) {
        if (flags & {{{ cDefine('AI_NUMERICSERV') }}}) {
          return {{{ cDefine('EAI_NONAME') }}};
        }
        // TODO support resolving well-known service names from:
        // http://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.txt
        return {{{ cDefine('EAI_SERVICE') }}};
      }
    }

    if (!node) {
      if (family === {{{ cDefine('AF_UNSPEC') }}}) {
        family = {{{ cDefine('AF_INET') }}};
      }
      if ((flags & {{{ cDefine('AI_PASSIVE') }}}) === 0) {
        if (family === {{{ cDefine('AF_INET') }}}) {
          addr = _htonl({{{ cDefine('INADDR_LOOPBACK') }}});
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
      if (family === {{{ cDefine('AF_UNSPEC') }}} || family === {{{ cDefine('AF_INET') }}}) {
        family = {{{ cDefine('AF_INET') }}};
      }
      else if (family === {{{ cDefine('AF_INET6') }}} && (flags & {{{ cDefine('AI_V4MAPPED') }}})) {
        addr = [0, 0, _htonl(0xffff), addr];
        family = {{{ cDefine('AF_INET6') }}};
      } else {
        return {{{ cDefine('EAI_NONAME') }}};
      }
    } else {
      addr = inetPton6(node);
      if (addr !== null) {
        // incoming node is a valid ipv6 address
        if (family === {{{ cDefine('AF_UNSPEC') }}} || family === {{{ cDefine('AF_INET6') }}}) {
          family = {{{ cDefine('AF_INET6') }}};
        } else {
          return {{{ cDefine('EAI_NONAME') }}};
        }
      }
    }
    if (addr != null) {
      ai = allocaddrinfo(family, type, proto, node, addr, port);
      {{{ makeSetValue('out', '0', 'ai', '*') }}};
      return 0;
    }
    if (flags & {{{ cDefine('AI_NUMERICHOST') }}}) {
      return {{{ cDefine('EAI_NONAME') }}};
    }

    //
    // try as a hostname
    //
    // resolve the hostname to a temporary fake address
    node = DNS.lookup_name(node);
    addr = inetPton4(node);
    if (family === {{{ cDefine('AF_UNSPEC') }}}) {
      family = {{{ cDefine('AF_INET') }}};
    } else if (family === {{{ cDefine('AF_INET6') }}}) {
      addr = [0, 0, _htonl(0xffff), addr];
    }
    ai = allocaddrinfo(family, type, proto, null, addr, port);
    {{{ makeSetValue('out', '0', 'ai', '*') }}};
    return 0;
  },

  getnameinfo__deps: ['$Sockets', '$DNS', '$readSockaddr'],
  getnameinfo: function (sa, salen, node, nodelen, serv, servlen, flags) {
    var info = readSockaddr(sa, salen);
    if (info.errno) {
      return {{{ cDefine('EAI_FAMILY') }}};
    }
    var port = info.port;
    var addr = info.addr;

    var overflowed = false;

    if (node && nodelen) {
      var lookup;
      if ((flags & {{{ cDefine('NI_NUMERICHOST') }}}) || !(lookup = DNS.lookup_addr(addr))) {
        if (flags & {{{ cDefine('NI_NAMEREQD') }}}) {
          return {{{ cDefine('EAI_NONAME') }}};
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
      return {{{ cDefine('EAI_OVERFLOW') }}};
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
  setprotoent__deps: ['$Protocols'
#if MINIMAL_RUNTIME
    , '$writeAsciiToMemory'
#endif
  ],
  setprotoent: function(stayopen) {
    // void setprotoent(int stayopen);

    // Allocate and populate a protoent structure given a name, protocol number and array of aliases
    function allocprotoent(name, proto, aliases) {
      // write name into buffer
      var nameBuf = _malloc(name.length + 1);
      writeAsciiToMemory(name, nameBuf);

      // write aliases into buffer
      var j = 0;
      var length = aliases.length;
      var aliasListBuf = _malloc((length + 1) * 4); // Use length + 1 so we have space for the terminating NULL ptr.

      for (var i = 0; i < length; i++, j += 4) {
        var alias = aliases[i];
        var aliasBuf = _malloc(alias.length + 1);
        writeAsciiToMemory(alias, aliasBuf);
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

  endprotoent: function() {
    // void endprotoent(void);
    // We're not using a real protocol database so we don't do a real close.
  },

  getprotoent__deps: ['setprotoent', '$Protocols'],
  getprotoent: function(number) {
    // struct protoent *getprotoent(void);
    // reads the  next  entry  from  the  protocols 'database' or return NULL if 'eof'
    if (_setprotoent.index === Protocols.list.length) {
      return 0;
    } else {
      var result = Protocols.list[_setprotoent.index++];
      return result;
    }
  },

  getprotobyname__deps: ['setprotoent', '$Protocols'],
  getprotobyname: function(name) {
    // struct protoent *getprotobyname(const char *);
    name = UTF8ToString(name);
    _setprotoent(true);
    var result = Protocols.map[name];
    return result;
  },

  getprotobynumber__deps: ['setprotoent', '$Protocols'],
  getprotobynumber: function(number) {
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
    function() { return 'var SocketIO = ' + read('../third_party/socket.io.js') + ';\n' },
    function() { return 'var Peer = ' + read('../third_party/wrtcp.js') + ';\n' }],
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

  // TODO: consider allowing the API to get a parameter for the number of
  // bytes.
  $getRandomDevice: function() {
    if (typeof crypto == 'object' && typeof crypto['getRandomValues'] == 'function') {
      // for modern web browsers
      var randomBuffer = new Uint8Array(1);
      return function() { crypto.getRandomValues(randomBuffer); return randomBuffer[0]; };
    } else
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE) {
      // for nodejs with or without crypto support included
      try {
        var crypto_module = require('crypto');
        // nodejs has crypto support
        return function() { return crypto_module['randomBytes'](1)[0]; };
      } catch (e) {
        // nodejs doesn't have crypto support
      }
    }
#endif // ENVIRONMENT_MAY_BE_NODE
    // we couldn't find a proper implementation, as Math.random() is not suitable for /dev/random, see emscripten-core/emscripten/pull/7096
#if ASSERTIONS
    return function() { abort("no cryptographic support found for randomDevice. consider polyfilling it if you want to use something insecure like Math.random(), e.g. put this in a --pre-js: var crypto = { getRandomValues: function(array) { for (var i = 0; i < array.length; i++) array[i] = (Math.random()*256)|0 } };"); };
#else
    return function() { abort("randomDevice"); };
#endif
  },

  getentropy__deps: ['$getRandomDevice'],
  getentropy: function(buffer, size) {
    if (!_getentropy.randomDevice) {
      _getentropy.randomDevice = getRandomDevice();
    }
    for (var i = 0; i < size; i++) {
      {{{ makeSetValue('buffer', 'i', '_getentropy.randomDevice()', 'i8') }}};
    }
    return 0;
  },

  // http://pubs.opengroup.org/onlinepubs/000095399/functions/alarm.html
  alarm__deps: ['raise', '$callUserCallback'],
  alarm: function(seconds) {
    setTimeout(function() {
      callUserCallback(function() {
        _raise({{{ cDefine('SIGALRM') }}});
      });
    }, seconds*1000);
  },

  // Helper for raise() to avoid signature mismatch failures:
  // https://github.com/emscripten-core/posixtestsuite/issues/6
  __call_sighandler__sig: 'vpi',
  __call_sighandler: function(fp, sig) {
    {{{ makeDynCall('vi', 'fp') }}}(sig);
  },

  // ==========================================================================
  // emscripten.h
  // ==========================================================================

  emscripten_run_script__sig: 'vi',
  emscripten_run_script: function(ptr) {
    {{{ makeEval('eval(UTF8ToString(ptr));') }}}
  },

  emscripten_run_script_int__sig: 'ii',
  emscripten_run_script_int__docs: '/** @suppress{checkTypes} */',
  emscripten_run_script_int: function(ptr) {
    {{{ makeEval('return eval(UTF8ToString(ptr))|0;') }}}
  },

  // We use builtin_malloc and builtin_free here because otherwise lsan will
  // report the last returned string as a leak.
  emscripten_run_script_string__deps: ['emscripten_builtin_malloc', 'emscripten_builtin_free'],
  emscripten_run_script_string__sig: 'ii',
  emscripten_run_script_string: function(ptr) {
    {{{ makeEval("var s = eval(UTF8ToString(ptr));") }}}
    if (s == null) {
      return 0;
    }
    s += '';
    var me = _emscripten_run_script_string;
    var len = lengthBytesUTF8(s);
    if (!me.bufferSize || me.bufferSize < len+1) {
      if (me.bufferSize) _emscripten_builtin_free(me.buffer);
      me.bufferSize = len+1;
      me.buffer = _emscripten_builtin_malloc(me.bufferSize);
    }
    stringToUTF8(s, me.buffer, me.bufferSize);
    return me.buffer;
  },

  emscripten_random: function() {
    return Math.random();
  },

  emscripten_get_now__import: true,
  emscripten_get_now: ';' +
#if ENVIRONMENT_MAY_BE_NODE
                               "if (ENVIRONMENT_IS_NODE) {\n" +
                               "  _emscripten_get_now = () => {\n" +
                               "    var t = process['hrtime']();\n" +
                               "    return t[0] * 1e3 + t[1] / 1e6;\n" +
                               "  };\n" +
                               "} else " +
#endif
#if USE_PTHREADS
// Pthreads need their clocks synchronized to the execution of the main thread, so give them a special form of the function.
// N.b. Wasm workers do not provide this kind of clock synchronization.
                               "if (ENVIRONMENT_IS_PTHREAD) {\n" +
                               "  _emscripten_get_now = () => performance.now() - Module['__performance_now_clock_drift'];\n" +
                               "} else " +
#endif
#if ENVIRONMENT_MAY_BE_SHELL
                               "if (typeof dateNow != 'undefined') {\n" +
                               "  _emscripten_get_now = dateNow;\n" +
                               "} else " +
#endif
#if MIN_IE_VERSION <= 9 || MIN_FIREFOX_VERSION <= 14 || MIN_CHROME_VERSION <= 23 || MIN_SAFARI_VERSION <= 80400 // https://caniuse.com/#feat=high-resolution-time
                               "if (typeof performance != 'undefined' && performance.now) {\n" +
                               "  _emscripten_get_now = () => performance.now();\n" +
                               "} else {\n" +
                               "  _emscripten_get_now = Date.now;\n" +
                               "}",
#else
                               // Modern environment where performance.now() is supported:
                               // N.B. a shorter form "_emscripten_get_now = return performance.now;" is unfortunately not allowed even in current browsers (e.g. FF Nightly 75).
                               "_emscripten_get_now = () => performance.now();\n",
#endif

  emscripten_get_now_res: function() { // return resolution of get_now, in nanoseconds
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE) {
      return 1; // nanoseconds
    } else
#endif
#if ENVIRONMENT_MAY_BE_SHELL
    if (typeof dateNow != 'undefined') {
      return 1000; // microseconds (1/1000 of a millisecond)
    } else
#endif
#if MIN_IE_VERSION <= 9 || MIN_FIREFOX_VERSION <= 14 || MIN_CHROME_VERSION <= 23 || MIN_SAFARI_VERSION <= 80400 // https://caniuse.com/#feat=high-resolution-time
    if (typeof performance == 'object' && performance && typeof performance['now'] == 'function') {
      return 1000; // microseconds (1/1000 of a millisecond)
    } else {
      return 1000*1000; // milliseconds
    }
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
#if ENVIRONMENT_MAY_BE_SHELL
      || (typeof dateNow != 'undefined')
#endif
    );`,
#else
  // Modern environment where performance.now() is supported: (rely on minifier to return true unconditionally from this function)
  $nowIsMonotonic: 'true;',
#endif

  _emscripten_get_now_is_monotonic__internal: true,
  _emscripten_get_now_is_monotonic__deps: ['$nowIsMonotonic'],
  _emscripten_get_now_is_monotonic: function() {
    return nowIsMonotonic;
  },

#if MINIMAL_RUNTIME
  $warnOnce: function(text) {
    if (!warnOnce.shown) warnOnce.shown = {};
    if (!warnOnce.shown[text]) {
      warnOnce.shown[text] = 1;
      err(text);
    }
  },
#endif

  // Returns [parentFuncArguments, functionName, paramListName]
  $traverseStack: function(args) {
    if (!args || !args.callee || !args.callee.name) {
      return [null, '', ''];
    }

    var funstr = args.callee.toString();
    var funcname = args.callee.name;
    var str = '(';
    var first = true;
    for (var i in args) {
      var a = args[i];
      if (!first) {
        str += ", ";
      }
      first = false;
      if (typeof a == 'number' || typeof a == 'string') {
        str += a;
      } else {
        str += '(' + typeof a + ')';
      }
    }
    str += ')';
    var caller = args.callee.caller;
    args = caller ? caller.arguments : [];
    if (first)
      str = '';
    return [args, funcname, str];
  },

  emscripten_get_callstack_js__deps: ['$traverseStack', '$jsStackTrace',
#if MINIMAL_RUNTIME
    , '$warnOnce'
#endif
  ],
  emscripten_get_callstack_js__docs: '/** @param {number=} flags */',
  emscripten_get_callstack_js: function(flags) {
    var callstack = jsStackTrace();

    // Find the symbols in the callstack that corresponds to the functions that
    // report callstack information, and remove everything up to these from the
    // output.
    var iThisFunc = callstack.lastIndexOf('_emscripten_log');
    var iThisFunc2 = callstack.lastIndexOf('_emscripten_get_callstack');
    var iNextLine = callstack.indexOf('\n', Math.max(iThisFunc, iThisFunc2))+1;
    callstack = callstack.slice(iNextLine);

    if (flags & {{{ cDefine('EM_LOG_DEMANGLE') }}}) {
      warnOnce('EM_LOG_DEMANGLE is deprecated; ignoring');
    }

    // If user requested to see the original source stack, but no source map
    // information is available, just fall back to showing the JS stack.
    if (flags & {{{ cDefine('EM_LOG_C_STACK') }}} && typeof emscripten_source_map == 'undefined') {
      warnOnce('Source map information is not available, emscripten_log with EM_LOG_C_STACK will be ignored. Build with "--pre-js $EMSCRIPTEN/src/emscripten-source-map.min.js" linker flag to add source map loading to code.');
      flags ^= {{{ cDefine('EM_LOG_C_STACK') }}};
      flags |= {{{ cDefine('EM_LOG_JS_STACK') }}};
    }

    var stack_args = null;
    if (flags & {{{ cDefine('EM_LOG_FUNC_PARAMS') }}}) {
      // To get the actual parameters to the functions, traverse the stack via
      // the unfortunately deprecated 'arguments.callee' method, if it works:
      stack_args = traverseStack(arguments);
      while (stack_args[1].includes('_emscripten_'))
        stack_args = traverseStack(stack_args[0]);
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

      if (flags & {{{ cDefine('EM_LOG_C_STACK') }}}) {
        var orig = emscripten_source_map.originalPositionFor({line: lineno, column: column});
        haveSourceMap = (orig && orig.source);
        if (haveSourceMap) {
          if (flags & {{{ cDefine('EM_LOG_NO_PATHS') }}}) {
            orig.source = orig.source.substring(orig.source.replace(/\\/g, "/").lastIndexOf('/')+1);
          }
          callstack += '    at ' + symbolName + ' (' + orig.source + ':' + orig.line + ':' + orig.column + ')\n';
        }
      }
      if ((flags & {{{ cDefine('EM_LOG_JS_STACK') }}}) || !haveSourceMap) {
        if (flags & {{{ cDefine('EM_LOG_NO_PATHS') }}}) {
          file = file.substring(file.replace(/\\/g, "/").lastIndexOf('/')+1);
        }
        callstack += (haveSourceMap ? ('     = ' + symbolName) : ('    at '+ symbolName)) + ' (' + file + ':' + lineno + ':' + column + ')\n';
      }

      // If we are still keeping track with the callstack by traversing via
      // 'arguments.callee', print the function parameters as well.
      if (flags & {{{ cDefine('EM_LOG_FUNC_PARAMS') }}} && stack_args[0]) {
        if (stack_args[1] == symbolName && stack_args[2].length > 0) {
          callstack = callstack.replace(/\s+$/, '');
          callstack += ' with values: ' + stack_args[1] + stack_args[2] + '\n';
        }
        stack_args = traverseStack(stack_args[0]);
      }
    }
    // Trim extra whitespace at the end of the output.
    callstack = callstack.replace(/\s+$/, '');
    return callstack;
  },

  emscripten_get_callstack__deps: ['emscripten_get_callstack_js'],
  emscripten_get_callstack: function(flags, str, maxbytes) {
    var callstack = _emscripten_get_callstack_js(flags);
    // User can query the required amount of bytes to hold the callstack.
    if (!str || maxbytes <= 0) {
      return lengthBytesUTF8(callstack)+1;
    }
    // Output callstack string as C string to HEAP.
    var bytesWrittenExcludingNull = stringToUTF8(callstack, str, maxbytes);

    // Return number of bytes written, including null.
    return bytesWrittenExcludingNull+1;
  },

  emscripten_log_js__deps: ['emscripten_get_callstack_js'],
  emscripten_log_js: function(flags, str) {
    if (flags & {{{ cDefine('EM_LOG_C_STACK') | cDefine('EM_LOG_JS_STACK') }}}) {
      str = str.replace(/\s+$/, ''); // Ensure the message and the callstack are joined cleanly with exactly one newline.
      str += (str.length > 0 ? '\n' : '') + _emscripten_get_callstack_js(flags);
    }

    if (flags & {{{ cDefine('EM_LOG_CONSOLE') }}}) {
      if (flags & {{{ cDefine('EM_LOG_ERROR') }}}) {
        console.error(str);
      } else if (flags & {{{ cDefine('EM_LOG_WARN') }}}) {
        console.warn(str);
      } else if (flags & {{{ cDefine('EM_LOG_INFO') }}}) {
        console.info(str);
      } else if (flags & {{{ cDefine('EM_LOG_DEBUG') }}}) {
        console.debug(str);
      } else {
        console.log(str);
      }
    } else if (flags & {{{ cDefine('EM_LOG_ERROR') | cDefine('EM_LOG_WARN') }}}) {
      err(str);
    } else {
      out(str);
    }
  },

  emscripten_log__deps: ['$formatString', 'emscripten_log_js'],
  emscripten_log: function(flags, format, varargs) {
    var result = formatString(format, varargs);
    var str = UTF8ArrayToString(result, 0);
    _emscripten_log_js(flags, str);
  },

  // We never free the return values of this function so we need to allocate
  // using builtin_malloc to avoid LSan reporting these as leaks.
  emscripten_get_compiler_setting__deps: ['emscripten_builtin_malloc'],
  emscripten_get_compiler_setting__sig: 'pp',
  emscripten_get_compiler_setting: function(name) {
#if RETAIN_COMPILER_SETTINGS
    name = UTF8ToString(name);

    var ret = getCompilerSetting(name);
    if (typeof ret == 'number' || typeof ret == 'boolean') return ret;

    if (!_emscripten_get_compiler_setting.cache) _emscripten_get_compiler_setting.cache = {};
    var cache = _emscripten_get_compiler_setting.cache;
    var fullret = cache[name];
    if (fullret) return fullret;
    cache[name] = _emscripten_builtin_malloc(ret.length + 1);
    stringToUTF8(ret + '', cache[name], ret.length + 1);
    return cache[name];
#else
    throw 'You must build with -sRETAIN_COMPILER_SETTINGS for getCompilerSetting or emscripten_get_compiler_setting to work';
#endif
  },

  emscripten_has_asyncify: function() {
    return {{{ ASYNCIFY }}};
  },

  emscripten_debugger: function() {
    debugger;
  },

  emscripten_print_double: function(x, to, max) {
    var str = x + '';
    if (to) return stringToUTF8(str, to, max);
    else return lengthBytesUTF8(str);
  },

  // Generates a representation of the program counter from a line of stack trace.
  // The exact return value depends in whether we are running WASM or JS, and whether
  // the engine supports offsets into WASM. See the function body for details.
  $convertFrameToPC__docs: '/** @returns {number} */',
  $convertFrameToPC__internal: true,
  $convertFrameToPC: function(frame) {
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
  emscripten_return_address: function(level) {
    var callstack = jsStackTrace().split('\n');
    if (callstack[0] == 'Error') {
      callstack.shift();
    }
    // skip this function and the caller to get caller's return address
    return convertFrameToPC(callstack[level + 3]);
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
  emscripten_stack_snapshot: function () {
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
  $saveInUnwindCache: function (callstack) {
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
  emscripten_stack_unwind_buffer: function (addr, buffer, count) {
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
  emscripten_pc_get_function__deps: [
    '$UNWIND_CACHE',
    'free',
#if MINIMAL_RUNTIME
    '$allocateUTF8',
#endif
  ],
  // Don't treat allocation of _emscripten_pc_get_function.ret as a leak
  emscripten_pc_get_function__noleakcheck: true,
#endif
  emscripten_pc_get_function: function (pc) {
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
    _emscripten_pc_get_function.ret = allocateUTF8(name);
    return _emscripten_pc_get_function.ret;
#endif
  },

  $convertPCtoSourceLocation__deps: ['$UNWIND_CACHE', '$convertFrameToPC'],
  $convertPCtoSourceLocation: function(pc) {
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
  emscripten_pc_get_file__deps: ['$convertPCtoSourceLocation', 'free',
#if MINIMAL_RUNTIME
    '$allocateUTF8',
#endif
  ],
  // Don't treat allocation of _emscripten_pc_get_file.ret as a leak
  emscripten_pc_get_file__noleakcheck: true,
  emscripten_pc_get_file: function (pc) {
    var result = convertPCtoSourceLocation(pc);
    if (!result) return 0;

    if (_emscripten_pc_get_file.ret) _free(_emscripten_pc_get_file.ret);
    _emscripten_pc_get_file.ret = allocateUTF8(result.file);
    return _emscripten_pc_get_file.ret;
  },

  // Look up the line number from our stack frame cache with our PC representation.
  emscripten_pc_get_line__deps: ['$convertPCtoSourceLocation'],
  emscripten_pc_get_line: function (pc) {
    var result = convertPCtoSourceLocation(pc);
    return result ? result.line : 0;
  },

  // Look up the column number from our stack frame cache with our PC representation.
  emscripten_pc_get_column__deps: ['$convertPCtoSourceLocation'],
  emscripten_pc_get_column: function (pc) {
    var result = convertPCtoSourceLocation(pc);
    return result ? result.column || 0 : 0;
  },

  emscripten_get_module_name: function(buf, length) {
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
  $withBuiltinMalloc: function (func) {
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
#endif

  $readAsmConstArgsArray: '=[]',
  $readAsmConstArgs__deps: [
    '$readAsmConstArgsArray',
#if MEMORY64
    '$readI53FromI64',
#endif
  ],
  $readAsmConstArgs: function(sigPtr, buf) {
    {{{ from64(['sigPtr', 'buf']) }}};
#if ASSERTIONS
    // Nobody should have mutated _readAsmConstArgsArray underneath us to be something else than an array.
    assert(Array.isArray(readAsmConstArgsArray));
    // The input buffer is allocated on the stack, so it must be stack-aligned.
    assert(buf % {{{ STACK_ALIGN }}} == 0);
#endif
    readAsmConstArgsArray.length = 0;
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
      assert(validChars.includes(chr), 'Invalid character ' + ch + '("' + chr + '") in readAsmConstArgs! Use only [' + validChars + '], and do not specify "v" for void return argument.');
#endif
      // Floats are always passed as doubles, and doubles and int64s take up 8
      // bytes (two 32-bit slots) in memory, align reads to these:
      buf += (ch != 105/*i*/) & buf;
#if MEMORY64
      // Special case for pointers under wasm64 which we read as int53 Numbers.
      if (ch == 112/*p*/) {
        readAsmConstArgsArray.push(readI53FromI64(buf++ << 2));
      } else
#endif
      readAsmConstArgsArray.push(
        ch == 105/*i*/ ? HEAP32[buf] :
#if WASM_BIGINT
       (ch == 106/*j*/ ? HEAP64 : HEAPF64)[buf++ >> 1]
#else
       HEAPF64[buf++ >> 1]
#endif
      );
      ++buf;
    }
    return readAsmConstArgsArray;
  },

  emscripten_asm_const_int__sig: 'ippp',
  emscripten_asm_const_int__deps: ['$readAsmConstArgs'],
  emscripten_asm_const_int: function(code, sigPtr, argbuf) {
#if RELOCATABLE
    code -= {{{ GLOBAL_BASE }}};
#endif
    var args = readAsmConstArgs(sigPtr, argbuf);
#if ASSERTIONS
    if (!ASM_CONSTS.hasOwnProperty(code)) abort('No EM_ASM constant found at address ' + code);
#endif
#if MEMORY64
    return Number(ASM_CONSTS[code].apply(null, args));
#else
    return ASM_CONSTS[code].apply(null, args);
#endif
  },
  emscripten_asm_const_double: 'emscripten_asm_const_int',

#if MEMORY64
  emscripten_asm_const_ptr__sig: 'pppp',
  emscripten_asm_const_ptr__deps: ['emscripten_asm_const_int'],
  emscripten_asm_const_ptr: function(code, sigPtr, argbuf) {
    return _emscripten_asm_const_int(code, sigPtr, argbuf);
  },
#else
  emscripten_asm_const_ptr: 'emscripten_asm_const_int',
#endif

  $mainThreadEM_ASM__deps: ['$readAsmConstArgs'],
  $mainThreadEM_ASM: function(code, sigPtr, argbuf, sync) {
#if RELOCATABLE
    code -= {{{ GLOBAL_BASE }}};
#endif
    var args = readAsmConstArgs(sigPtr, argbuf);
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) {
      // EM_ASM functions are variadic, receiving the actual arguments as a buffer
      // in memory. the last parameter (argBuf) points to that data. We need to
      // always un-variadify that, *before proxying*, as in the async case this
      // is a stack allocation that LLVM made, which may go away before the main
      // thread gets the message. For that reason we handle proxying *after* the
      // call to readAsmConstArgs, and therefore we do that manually here instead
      // of using __proxy. (And dor simplicity, do the same in the sync
      // case as well, even though it's not strictly necessary, to keep the two
      // code paths as similar as possible on both sides.)
      // -1 - code is the encoding of a proxied EM_ASM, as a negative number
      // (positive numbers are non-EM_ASM calls).
      return _emscripten_proxy_to_main_thread_js.apply(null, [-1 - code, sync].concat(args));
    }
#endif
#if ASSERTIONS
    if (!ASM_CONSTS.hasOwnProperty(code)) abort('No EM_ASM constant found at address ' + code);
#endif
    return ASM_CONSTS[code].apply(null, args);
  },
  emscripten_asm_const_int_sync_on_main_thread__deps: ['$mainThreadEM_ASM'],
  emscripten_asm_const_int_sync_on_main_thread__sig: 'iiii',
  emscripten_asm_const_int_sync_on_main_thread: function(code, sigPtr, argbuf) {
    return mainThreadEM_ASM(code, sigPtr, argbuf, 1);
  },
  emscripten_asm_const_double_sync_on_main_thread: 'emscripten_asm_const_int_sync_on_main_thread',
  emscripten_asm_const_async_on_main_thread__deps: ['$mainThreadEM_ASM'],
  emscripten_asm_const_async_on_main_thread: function(code, sigPtr, argbuf) {
    return mainThreadEM_ASM(code, sigPtr, argbuf, 0);
  },

#if !DECLARE_ASM_MODULE_EXPORTS
  // When DECLARE_ASM_MODULE_EXPORTS is not set we export native symbols
  // at runtime rather than statically in JS code.
  $exportAsmFunctions__deps: ['$asmjsMangle'],
  $exportAsmFunctions: function(asm) {
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
  $jstoi_q: function(str) {
    return parseInt(str);
  },

  // Converts a JS string to an integer base-10, with signaling error
  // handling (throws a JS exception on error). E.g. jstoi_s("123abc")
  // throws an exception.
  $jstoi_s: function(str) {
    return Number(str);
  },

#if LINK_AS_CXX
  // libunwind

  _Unwind_Backtrace__deps: ['emscripten_get_callstack_js'],
  _Unwind_Backtrace: function(func, arg) {
    var trace = _emscripten_get_callstack_js();
    var parts = trace.split('\n');
    for (var i = 0; i < parts.length; i++) {
      var ret = {{{ makeDynCall('iii', 'func') }}}(0, arg);
      if (ret !== 0) return;
    }
  },

  _Unwind_GetIPInfo: function() {
    abort('Unwind_GetIPInfo');
  },

  _Unwind_FindEnclosingFunction: function() {
    return 0; // we cannot succeed
  },

  _Unwind_RaiseException__deps: ['__cxa_throw'],
  _Unwind_RaiseException: function(ex) {
    err('Warning: _Unwind_RaiseException is not correctly implemented');
    return ___cxa_throw(ex, 0, 0);
  },

  _Unwind_DeleteException: function(ex) {
    err('TODO: Unwind_DeleteException');
  },
#endif

  // autodebugging

  emscripten_autodebug_i64: function(line, valuel, valueh) {
    out('AD:' + [line, valuel, valueh]);
  },
  emscripten_autodebug_i32: function(line, value) {
    out('AD:' + [line, value]);
  },
  emscripten_autodebug_i16: function(line, value) {
    out('AD:' + [line, value]);
  },
  emscripten_autodebug_i8: function(line, value) {
    out('AD:' + [line, value]);
  },
  emscripten_autodebug_float: function(line, value) {
    out('AD:' + [line, value]);
  },
  emscripten_autodebug_double: function(line, value) {
    out('AD:' + [line, value]);
  },

  // special runtime support

#if STACK_OVERFLOW_CHECK
  // Used by wasm-emscripten-finalize to implement STACK_OVERFLOW_CHECK
  __handle_stack_overflow__deps: ['emscripten_stack_get_base'],
  __handle_stack_overflow: function(requested) {
    requested = requested >>> 0;
    abort('stack overflow (Attempt to set SP to 0x' + requested.toString(16) +
          ', with stack limits [0x' + _emscripten_stack_get_end().toString(16) +
          ' - 0x' + _emscripten_stack_get_base().toString(16) + '])');
  },
#endif

  $getExecutableName: function() {
#if MINIMAL_RUNTIME // MINIMAL_RUNTIME does not have a global runtime variable thisProgram
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE && process['argv'].length > 1) {
      return process['argv'][1].replace(/\\/g, '/');
    }
#endif
    return "./this.program";
#else
    return thisProgram || './this.program';
#endif
  },

  $listenOnce: function(object, event, func) {
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
  $autoResumeAudioContext: function(ctx, elements) {
    if (!elements) {
      elements = [document, document.getElementById('canvas')];
    }
    ['keydown', 'mousedown', 'touchstart'].forEach(function(event) {
      elements.forEach(function(element) {
        if (element) {
          listenOnce(element, event, function() {
            if (ctx.state === 'suspended') ctx.resume();
          });
        }
      });
    });
  },

#if DYNCALLS || !WASM_BIGINT
  $dynCallLegacy: function(sig, ptr, args) {
#if ASSERTIONS
#if MINIMAL_RUNTIME
    assert(typeof dynCalls != 'undefined', 'Global dynCalls dictionary was not generated in the build! Pass -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=$dynCall linker flag to include it!');
    assert(sig in dynCalls, 'bad function pointer type - no table for sig \'' + sig + '\'');
#else
    assert(('dynCall_' + sig) in Module, 'bad function pointer type - no table for sig \'' + sig + '\'');
#endif
    if (args && args.length) {
      // j (64-bit integer) must be passed in as two numbers [low 32, high 32].
      assert(args.length === sig.substring(1).replace(/j/g, '--').length);
    } else {
      assert(sig.length == 1);
    }
#endif
#if MINIMAL_RUNTIME
    var f = dynCalls[sig];
#else
    var f = Module["dynCall_" + sig];
#endif
    return args && args.length ? f.apply(null, [ptr].concat(args)) : f.call(null, ptr);
  },
  $dynCall__deps: ['$dynCallLegacy', '$getWasmTableEntry'],

  // Used in library code to get JS function from wasm function pointer.
  // All callers should use direct table access where possible and only fall
  // back to this function if needed.
  $getDynCaller__deps: ['$dynCall'],
  $getDynCaller: function(sig, ptr) {
#if ASSERTIONS && !DYNCALLS
    assert(sig.includes('j'), 'getDynCaller should only be called with i64 sigs')
#endif
    var argCache = [];
    return function() {
      argCache.length = 0;
      Object.assign(argCache, arguments);
      return dynCall(sig, ptr, argCache);
    };
  },
#endif

  $dynCall__docs: '/** @param {Object=} args */',
  $dynCall: function(sig, ptr, args) {
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
    assert(getWasmTableEntry(ptr), 'missing table entry in dynCall: ' + ptr);
#endif
    return getWasmTableEntry(ptr).apply(null, args)
#endif
  },

  $callRuntimeCallbacks__internal: true,
  $callRuntimeCallbacks: function(callbacks) {
    while (callbacks.length > 0) {
      var callback = callbacks.shift();
      if (typeof callback == 'function') {
        callback(Module); // Pass the module as the first argument.
        continue;
      }
      var func = callback.func;
      if (typeof func == 'number') {
        if (callback.arg === undefined) {
          // Run the wasm function ptr with signature 'v'. If no function
          // with such signature was exported, this call does not need
          // to be emitted (and would confuse Closure)
          {{{ makeDynCall('v', 'func') }}}();
        } else {
          // If any function with signature 'vi' was exported, run
          // the callback with that signature.
          {{{ makeDynCall('vi', 'func') }}}(callback.arg);
        }
      } else {
        func(callback.arg === undefined ? null : callback.arg);
      }
    }
  },

#if SHRINK_LEVEL == 0
  // A mirror copy of contents of wasmTable in JS side, to avoid relatively
  // slow wasmTable.get() call. Only used when not compiling with -Os or -Oz.
  $wasmTableMirror__internal: true,
  $wasmTableMirror: [],

  $setWasmTableEntry__internal: true,
  $setWasmTableEntry__deps: ['$wasmTableMirror'],
  $setWasmTableEntry: function(idx, func) {
    wasmTable.set(idx, func);
    // With ABORT_ON_WASM_EXCEPTIONS wasmTable.get is overriden to return wrapped
    // functions so we need to call it here to retrieve the potential wrapper correctly
    // instead of just storing 'func' directly into wasmTableMirror
    wasmTableMirror[idx] = wasmTable.get(idx);
  },

  $getWasmTableEntry__internal: true,
  $getWasmTableEntry__deps: ['$wasmTableMirror'],
  $getWasmTableEntry: function(funcPtr) {
    var func = wasmTableMirror[funcPtr];
    if (!func) {
      if (funcPtr >= wasmTableMirror.length) wasmTableMirror.length = funcPtr + 1;
      wasmTableMirror[funcPtr] = func = wasmTable.get(funcPtr);
    }
#if ASSERTIONS
    assert(wasmTable.get(funcPtr) == func, "JavaScript-side Wasm function table mirror is out of date!");
#endif
    return func;
  },

#else

  $setWasmTableEntry: function(idx, func) {
    wasmTable.set(idx, func);
  },

  $getWasmTableEntry: function(funcPtr) {
    // In -Os and -Oz builds, do not implement a JS side wasm table mirror for small
    // code size, but directly access wasmTable, which is a bit slower as uncached.
    return wasmTable.get(funcPtr);
  },
#endif // SHRINK_LEVEL == 0

  // Callable in pthread without __proxy needed.
  emscripten_exit_with_live_runtime__sig: 'v',
#if !MINIMAL_RUNTIME
  emscripten_exit_with_live_runtime__deps: ['$runtimeKeepalivePush'],
#endif
  emscripten_exit_with_live_runtime: function() {
    {{{ runtimeKeepalivePush() }}}
    throw 'unwind';
  },

#if MINIMAL_RUNTIME
  emscripten_force_exit__deps: ['exit'],
#endif
  emscripten_force_exit__proxy: 'sync',
  emscripten_force_exit__sig: 'vi',
  emscripten_force_exit: function(status) {
#if !EXIT_RUNTIME && ASSERTIONS
    warnOnce('emscripten_force_exit cannot actually shut down the runtime, as the build does not have EXIT_RUNTIME set');
#endif
#if MINIMAL_RUNTIME
    _exit(status);
#else
    noExitRuntime = false;
#if EXIT_RUNTIME
    runtimeKeepaliveCounter = 0;
#endif
    exit(status);
#endif
  },

  _emscripten_out__sig: 'vi',
  _emscripten_out: function(str) {
#if ASSERTIONS
    assert(typeof str == 'number');
#endif
    out(UTF8ToString(str));
  },

  _emscripten_err__sig: 'vi',
  _emscripten_err: function(str) {
#if ASSERTIONS
    assert(typeof str == 'number');
#endif
    err(UTF8ToString(str));
  },

  // Use program_invocation_short_name and program_invocation_name in compiled
  // programs. This function is for implementing them.
  _emscripten_get_progname__sig: 'vii',
  _emscripten_get_progname: function(str, len) {
  #if !MINIMAL_RUNTIME
  #if ASSERTIONS
    assert(typeof str == 'number');
    assert(typeof len == 'number');
  #endif
    stringToUTF8(thisProgram, str, len);
  #endif
  },

  emscripten_console_log__sig: 'vp',
  emscripten_console_log: function(str) {
#if ASSERTIONS
    assert(typeof str == 'number');
#endif
    console.log(UTF8ToString(str));
  },

  emscripten_console_warn__sig: 'vp',
  emscripten_console_warn: function(str) {
#if ASSERTIONS
    assert(typeof str == 'number');
#endif
    console.warn(UTF8ToString(str));
  },

  emscripten_console_error__sig: 'vp',
  emscripten_console_error: function(str) {
#if ASSERTIONS
    assert(typeof str == 'number');
#endif
    console.error(UTF8ToString(str));
  },

  emscripten_throw_number: function(number) {
    throw number;
  },

  emscripten_throw_string: function(str) {
#if ASSERTIONS
    assert(typeof str == 'number');
#endif
    throw UTF8ToString(str);
  },

#if !MINIMAL_RUNTIME
  $handleException: function(e) {
    // Certain exception types we do not treat as errors since they are used for
    // internal control flow.
    // 1. ExitStatus, which is thrown by exit()
    // 2. "unwind", which is thrown by emscripten_unwind_to_js_event_loop() and others
    //    that wish to return to JS event loop.
    if (e instanceof ExitStatus || e == 'unwind') {
      return EXITSTATUS;
    }
#if MINIMAL_RUNTIME
    throw e;
#else
    quit_(1, e);
#endif
  },

  // Callable in pthread without __proxy needed.
  $runtimeKeepalivePush__sig: 'v',
  $runtimeKeepalivePush: function() {
#if EXIT_RUNTIME
    runtimeKeepaliveCounter += 1;
#if RUNTIME_DEBUG
    err('runtimeKeepalivePush -> counter=' + runtimeKeepaliveCounter);
#endif
#endif
  },

  $runtimeKeepalivePop__sig: 'v',
  $runtimeKeepalivePop: function() {
#if EXIT_RUNTIME
#if ASSERTIONS
    assert(runtimeKeepaliveCounter > 0);
#endif
    runtimeKeepaliveCounter -= 1;
#if RUNTIME_DEBUG
    err('runtimeKeepalivePop -> counter=' + runtimeKeepaliveCounter);
#endif
#endif
  },

  // Used to call user callbacks from the embedder / event loop.  For example
  // setTimeout or any other kind of event handler that calls into user case
  // needs to use this wrapper.
  //
  // The job of this wrapper is the handle emscripten-specfic exceptions such
  // as ExitStatus and 'unwind' and prevent these from escaping to the top
  // level.
  $callUserCallback__deps: ['$handleException',
#if EXIT_RUNTIME || USE_PTHREADS
    '$maybeExit',
#endif
  ],
  $callUserCallback__docs: '/** @param {boolean=} synchronous */',
  $callUserCallback: function(func, synchronous) {
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
    // For synchronous calls, let any exceptions propagate, and don't let the runtime exit.
    if (synchronous) {
      func();
      return;
    }
    try {
      func();
#if EXIT_RUNTIME || USE_PTHREADS
#if USE_PTHREADS && !EXIT_RUNTIME
      if (ENVIRONMENT_IS_PTHREAD)
#endif
        maybeExit();
#endif
    } catch (e) {
      handleException(e);
    }
  },

  $maybeExit__deps: ['exit', '$handleException',
#if USE_PTHREADS
    '_emscripten_thread_exit',
#endif
  ],
  $maybeExit: function() {
#if EXIT_RUNTIME
#if RUNTIME_DEBUG
    err('maybeExit: user callback done: runtimeKeepaliveCounter=' + runtimeKeepaliveCounter);
#endif
    if (!keepRuntimeAlive()) {
#if RUNTIME_DEBUG
      err('maybeExit: calling exit() implicitly after user callback completed: ' + EXITSTATUS);
#endif
      try {
#if USE_PTHREADS
        if (ENVIRONMENT_IS_PTHREAD) __emscripten_thread_exit(EXITSTATUS);
        else
#endif
        _exit(EXITSTATUS);
      } catch (e) {
        handleException(e);
      }
    }
#endif // EXIT_RUNTIME
  },
#else
  // MINIMAL_RUNTIME doesn't support the runtimeKeepalive stuff
  $callUserCallback: function(func) {
    func();
  },
#endif

  $safeSetTimeout__deps: ['$callUserCallback',
#if !MINIMAL_RUNTIME
   '$runtimeKeepalivePush',
   '$runtimeKeepalivePop',
#endif
  ],
  $safeSetTimeout__docs: '/** @param {number=} timeout */',
  $safeSetTimeout: function(func, timeout) {
    {{{ runtimeKeepalivePush() }}}
    return setTimeout(function() {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(func);
    }, timeout);
  },

  $asmjsMangle: function(x) {
    var unmangledSymbols = {{{ buildStringArray(WASM_SYSTEM_EXPORTS) }}};
    return x.indexOf('dynCall_') == 0 || unmangledSymbols.includes(x) ? x : '_' + x;
  },

  $asyncLoad__docs: '/** @param {boolean=} noRunDep */',
  $asyncLoad: function(url, onload, onerror, noRunDep) {
    var dep = !noRunDep ? getUniqueRunDependency('al ' + url) : '';
    readAsync(url, function(arrayBuffer) {
      assert(arrayBuffer, 'Loading data file "' + url + '" failed (no arrayBuffer).');
      onload(new Uint8Array(arrayBuffer));
      if (dep) removeRunDependency(dep);
    }, function(event) {
      if (onerror) {
        onerror();
      } else {
        throw 'Loading data file "' + url + '" failed.';
      }
    });
    if (dep) addRunDependency(dep);
  },

  $alignMemory: function(size, alignment) {
#if ASSERTIONS
    assert(alignment, "alignment argument is required");
#endif
    return Math.ceil(size / alignment) * alignment;
  },

  // Allocate memory for an mmap operation. This allocates space of the right
  // page-aligned size, and clears the allocated space.
  $mmapAlloc__deps: ['$zeroMemory', '$alignMemory'],
  $mmapAlloc: function(size) {
#if hasExportedFunction('_emscripten_builtin_memalign')
    size = alignMemory(size, {{{ WASM_PAGE_SIZE }}});
    var ptr = _emscripten_builtin_memalign({{{ WASM_PAGE_SIZE }}}, size);
    if (!ptr) return 0;
    zeroMemory(ptr, size);
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
  // Mark with `__import` so these are usable from native code.  This is needed
  // because, by default, only functions can be be imported.
  __stack_pointer: "new WebAssembly.Global({'value': '{{{ POINTER_WASM_TYPE }}}', 'mutable': true}, {{{ to64(STACK_BASE) }}})",
  __stack_pointer__import: true,
  // tell the memory segments where to place themselves
  __memory_base: "new WebAssembly.Global({'value': '{{{ POINTER_WASM_TYPE }}}', 'mutable': false}, {{{ to64(GLOBAL_BASE) }}})",
  __memory_base__import: true,
  // the wasm backend reserves slot 0 for the NULL function pointer
  __table_base: "new WebAssembly.Global({'value': '{{{ POINTER_WASM_TYPE }}}', 'mutable': false}, {{{ to64(1) }}})",
  __table_base__import: true,
#if MEMORY64
  __table_base32: 1,
#endif
  // To support such allocations during startup, track them on __heap_base and
  // then when the main module is loaded it reads that value and uses it to
  // initialize sbrk (the main module is relocatable itself, and so it does not
  // have __heap_base hardcoded into it - it receives it from JS as an extern
  // global, basically).
  __heap_base: '{{{ to64(HEAP_BASE) }}}',
  __heap_base__import: true,
#if EXCEPTION_HANDLING
  // In dynamic linking we define tags here and feed them to each module
  __cpp_exception: "new WebAssembly.Tag({'parameters': ['{{{ POINTER_WASM_TYPE }}}']})",
  __cpp_exception__import: true,
#endif
#if SUPPORT_LONGJMP == 'wasm'
  __c_longjmp: "new WebAssembly.Tag({'parameters': ['{{{ POINTER_WASM_TYPE }}}']})",
  __c_longjmp_import: true,
#endif
#endif
});

function autoAddDeps(object, name) {
  for (var item in object) {
    if (item.substr(-6) != '__deps') {
      if (!object[item + '__deps']) {
        object[item + '__deps'] = [name];
      } else {
        object[item + '__deps'].push(name); // add to existing list
      }
    }
  }
}
