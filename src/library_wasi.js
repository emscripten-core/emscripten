/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var WasiLibrary = {
  proc_exit__deps: ['exit'],
  proc_exit__sig: 'vi',
  proc_exit: function(code) {
    _exit(code);
  },

  $getEnvStrings__deps: ['$ENV', '$getExecutableName'],
  $getEnvStrings: function() {
    if (!getEnvStrings.strings) {
      // Default values.
#if !DETERMINISTIC
      // Browser language detection #8751
      var lang = ((typeof navigator === 'object' && navigator.languages && navigator.languages[0]) || 'C').replace('-', '_') + '.UTF-8';
#else
      // Deterministic language detection, ignore the browser's language.
      var lang = 'C.UTF-8';
#endif
      var env = {
        'USER': 'web_user',
        'LOGNAME': 'web_user',
        'PATH': '/',
        'PWD': '/',
        'HOME': '/home/web_user',
        'LANG': lang,
        '_': getExecutableName()
      };
      // Apply the user-provided values, if any.
      for (var x in ENV) {
        env[x] = ENV[x];
      }
      var strings = [];
      for (var x in env) {
        strings.push(x + '=' + env[x]);
      }
      getEnvStrings.strings = strings;
    }
    return getEnvStrings.strings;
  },

  environ_sizes_get__deps: ['$getEnvStrings'],
  environ_sizes_get__sig: 'iii',
  environ_sizes_get: function(penviron_count, penviron_buf_size) {
    var strings = getEnvStrings();
    {{{ makeSetValue('penviron_count', 0, 'strings.length', 'i32') }}};
    var bufSize = 0;
    strings.forEach(function(string) {
      bufSize += string.length + 1;
    });
    {{{ makeSetValue('penviron_buf_size', 0, 'bufSize', 'i32') }}};
    return 0;
  },

  environ_get__deps: ['$getEnvStrings'
#if MINIMAL_RUNTIME
    , '$writeAsciiToMemory'
#endif
  ],
  environ_get__sig: 'iii',
  environ_get: function(__environ, environ_buf) {
    var bufSize = 0;
    getEnvStrings().forEach(function(string, i) {
      var ptr = environ_buf + bufSize;
      {{{ makeSetValue('__environ', 'i * 4', 'ptr', 'i32') }}};
      writeAsciiToMemory(string, ptr);
      bufSize += string.length + 1;
    });
    return 0;
  },

  args_sizes_get__sig: 'iii',
  args_sizes_get: function(pargc, pargv_buf_size) {
#if MAIN_READS_PARAMS
    {{{ makeSetValue('pargc', 0, 'mainArgs.length', 'i32') }}};
    var bufSize = 0;
    mainArgs.forEach(function(arg) {
      bufSize += arg.length + 1;
    });
    {{{ makeSetValue('pargv_buf_size', 0, 'bufSize', 'i32') }}};
#else
    {{{ makeSetValue('pargc', 0, '0', 'i32') }}};
#endif
    return 0;
  },

  args_get__sig: 'iii',
#if MINIMAL_RUNTIME && MAIN_READS_PARAMS
  args_get__deps: ['$writeAsciiToMemory'],
#endif
  args_get: function(argv, argv_buf) {
#if MAIN_READS_PARAMS
    var bufSize = 0;
    mainArgs.forEach(function(arg, i) {
      var ptr = argv_buf + bufSize;
      {{{ makeSetValue('argv', 'i * 4', 'ptr', 'i32') }}};
      writeAsciiToMemory(arg, ptr);
      bufSize += arg.length + 1;
    });
#endif
    return 0;
  },

  $checkWasiClock: function(clock_id) {
    return clock_id == {{{ cDefine('__WASI_CLOCKID_REALTIME') }}} ||
           clock_id == {{{ cDefine('__WASI_CLOCKID_MONOTONIC') }}} ||
           clock_id == {{{ cDefine('__WASI_CLOCKID_PROCESS_CPUTIME_ID') }}} ||
           clock_id == {{{ cDefine('__WASI_CLOCKID_THREAD_CPUTIME_ID') }}};
  },

  // TODO: the i64 in the API here must be legalized for this JS code to run,
  // but the wasm file can't be legalized in standalone mode, which is where
  // this is needed. To get this code to be usable as a JS shim we need to
  // either wait for BigInt support or to legalize on the client.
  clock_time_get__sig: 'iiiii',
  clock_time_get__deps: ['emscripten_get_now', 'emscripten_get_now_is_monotonic', '$checkWasiClock'],
  clock_time_get: function(clk_id, {{{ defineI64Param('precision') }}}, ptime) {
    {{{ receiveI64ParamAsI32s('precision') }}}
    if (!checkWasiClock(clk_id)) {
      return {{{ cDefine('EINVAL') }}};
    }
    var now;
    // all wasi clocks but realtime are monotonic
    if (clk_id === {{{ cDefine('__WASI_CLOCKID_REALTIME') }}}) {
      now = Date.now();
    } else if (_emscripten_get_now_is_monotonic) {
      now = _emscripten_get_now();
    } else {
      return {{{ cDefine('ENOSYS') }}};
    }
    // "now" is in ms, and wasi times are in ns.
    var nsec = Math.round(now * 1000 * 1000);
    {{{ makeSetValue('ptime', 0, 'nsec >>> 0', 'i32') }}};
    {{{ makeSetValue('ptime', 4, '(nsec / Math.pow(2, 32)) >>> 0', 'i32') }}};
    return 0;
  },

  clock_res_get__sig: 'iii',
  clock_res_get__deps: ['emscripten_get_now', 'emscripten_get_now_res', 'emscripten_get_now_is_monotonic', '$checkWasiClock'],
  clock_res_get: function(clk_id, pres) {
    if (!checkWasiClock(clk_id)) {
      return {{{ cDefine('EINVAL') }}};
    }
    var nsec;
    // all wasi clocks but realtime are monotonic
    if (clk_id === {{{ cDefine('CLOCK_REALTIME') }}}) {
      nsec = 1000 * 1000; // educated guess that it's milliseconds
    } else if (_emscripten_get_now_is_monotonic) {
      nsec = _emscripten_get_now_res();
    } else {
      return {{{ cDefine('ENOSYS') }}};
    }
    {{{ makeSetValue('pres', 0, 'nsec >>> 0', 'i32') }}};
    {{{ makeSetValue('pres', 4, '(nsec / Math.pow(2, 32)) >>> 0', 'i32') }}};
    return 0;
  },
};

mergeInto(LibraryManager.library, WasiLibrary);
