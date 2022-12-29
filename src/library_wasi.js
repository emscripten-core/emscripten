/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var WasiLibrary = {
#if !MINIMAL_RUNTIME
  $ExitStatus__docs: '/** @constructor */',
  $ExitStatus: function(status) {
    this.name = 'ExitStatus';
    this.message = 'Program terminated with exit(' + status + ')';
    this.status = status;
  },
  proc_exit__deps: ['$ExitStatus'],
#endif

  proc_exit__nothrow: true,
  proc_exit__sig: 'vi',
  proc_exit: function(code) {
#if MINIMAL_RUNTIME
    throw 'exit(' + code + ')';
#else
#if RUNTIME_DEBUG
    dbg('proc_exit: ' + code);
#endif
    EXITSTATUS = code;
    if (!keepRuntimeAlive()) {
#if USE_PTHREADS
      PThread.terminateAllThreads();
#endif
#if expectToReceiveOnModule('onExit')
      if (Module['onExit']) Module['onExit'](code);
#endif
      ABORT = true;
    }
    quit_(code, new ExitStatus(code));
#endif // MINIMAL_RUNTIME
  },

  $getEnvStrings__deps: ['$ENV', '$getExecutableName'],
  $getEnvStrings: function() {
    if (!getEnvStrings.strings) {
      // Default values.
#if !DETERMINISTIC
      // Browser language detection #8751
      var lang = ((typeof navigator == 'object' && navigator.languages && navigator.languages[0]) || 'C').replace('-', '_') + '.UTF-8';
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
        // x is a key in ENV; if ENV[x] is undefined, that means it was
        // explicitly set to be so. We allow user code to do that to
        // force variables with default values to remain unset.
        if (ENV[x] === undefined) delete env[x];
        else env[x] = ENV[x];
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
  environ_sizes_get__nothrow: true,
  environ_sizes_get__sig: 'ipp',
  environ_sizes_get: function(penviron_count, penviron_buf_size) {
    var strings = getEnvStrings();
    {{{ makeSetValue('penviron_count', 0, 'strings.length', SIZE_TYPE) }}};
    var bufSize = 0;
    strings.forEach(function(string) {
      bufSize += string.length + 1;
    });
    {{{ makeSetValue('penviron_buf_size', 0, 'bufSize', SIZE_TYPE) }}};
    return 0;
  },

  environ_get__deps: ['$getEnvStrings', '$writeAsciiToMemory'],
  environ_get__nothrow: true,
  environ_get__sig: 'ipp',
  environ_get: function(__environ, environ_buf) {
    var bufSize = 0;
    getEnvStrings().forEach(function(string, i) {
      var ptr = environ_buf + bufSize;
      {{{ makeSetValue('__environ', `i*${Runtime.POINTER_SIZE}`, 'ptr', POINTER_TYPE) }}};
      writeAsciiToMemory(string, ptr);
      bufSize += string.length + 1;
    });
    return 0;
  },

  // In normal (non-standalone) mode arguments are passed direclty
  // to main, and the `mainArgs` global does not exist.
#if STANDALONE_WASM
  args_sizes_get__nothrow: true,
  args_sizes_get__sig: 'ipp',
  args_sizes_get: function(pargc, pargv_buf_size) {
#if MAIN_READS_PARAMS
    {{{ makeSetValue('pargc', 0, 'mainArgs.length', SIZE_TYPE) }}};
    var bufSize = 0;
    mainArgs.forEach(function(arg) {
      bufSize += arg.length + 1;
    });
    {{{ makeSetValue('pargv_buf_size', 0, 'bufSize', SIZE_TYPE) }}};
#else
    {{{ makeSetValue('pargc', 0, '0', SIZE_TYPE) }}};
#endif
    return 0;
  },

  args_get__nothrow: true,
  args_get__sig: 'ipp',
  args_get__deps: ['$writeAsciiToMemory'],
  args_get: function(argv, argv_buf) {
#if MAIN_READS_PARAMS
    var bufSize = 0;
    mainArgs.forEach(function(arg, i) {
      var ptr = argv_buf + bufSize;
      {{{ makeSetValue('argv', `i*${Runtime.POINTER_SIZE}`, 'ptr', POINTER_TYPE) }}};
      writeAsciiToMemory(arg, ptr);
      bufSize += arg.length + 1;
    });
#endif
    return 0;
  },
#endif

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
  clock_time_get__nothrow: true,
  clock_time_get__sig: 'iijp',
  clock_time_get__deps: ['emscripten_get_now', '$nowIsMonotonic', '$checkWasiClock'],
  clock_time_get: function(clk_id, {{{ defineI64Param('ignored_precision') }}}, ptime) {
    if (!checkWasiClock(clk_id)) {
      return {{{ cDefine('EINVAL') }}};
    }
    var now;
    // all wasi clocks but realtime are monotonic
    if (clk_id === {{{ cDefine('__WASI_CLOCKID_REALTIME') }}}) {
      now = Date.now();
    } else if (nowIsMonotonic) {
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

  clock_res_get__nothrow: true,
  clock_res_get__sig: 'iip',
  clock_res_get__deps: ['emscripten_get_now', 'emscripten_get_now_res', '$nowIsMonotonic', '$checkWasiClock'],
  clock_res_get: function(clk_id, pres) {
    if (!checkWasiClock(clk_id)) {
      return {{{ cDefine('EINVAL') }}};
    }
    var nsec;
    // all wasi clocks but realtime are monotonic
    if (clk_id === {{{ cDefine('CLOCK_REALTIME') }}}) {
      nsec = 1000 * 1000; // educated guess that it's milliseconds
    } else if (nowIsMonotonic) {
      nsec = _emscripten_get_now_res();
    } else {
      return {{{ cDefine('ENOSYS') }}};
    }
    {{{ makeSetValue('pres', 0, 'nsec >>> 0', 'i32') }}};
    {{{ makeSetValue('pres', 4, '(nsec / Math.pow(2, 32)) >>> 0', 'i32') }}};
    return 0;
  },

#if SYSCALLS_REQUIRE_FILESYSTEM
  $doReadv__docs: '/** @param {number=} offset */',
  $doReadv: function(stream, iov, iovcnt, offset) {
    var ret = 0;
    for (var i = 0; i < iovcnt; i++) {
      var ptr = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_base, '*') }}};
      var len = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_len, '*') }}};
      iov += {{{ C_STRUCTS.iovec.__size__ }}};
      var curr = FS.read(stream, {{{ heapAndOffset('HEAP8', 'ptr') }}}, len, offset);
      if (curr < 0) return -1;
      ret += curr;
      if (curr < len) break; // nothing more to read
      if (typeof offset !== 'undefined') {
        offset += curr;
      }
    }
    return ret;
  },
  $doWritev__docs: '/** @param {number=} offset */',
  $doWritev: function(stream, iov, iovcnt, offset) {
    var ret = 0;
    for (var i = 0; i < iovcnt; i++) {
      var ptr = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_base, '*') }}};
      var len = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_len, '*') }}};
      iov += {{{ C_STRUCTS.iovec.__size__ }}};
      var curr = FS.write(stream, {{{ heapAndOffset('HEAP8', 'ptr') }}}, len, offset);
      if (curr < 0) return -1;
      ret += curr;
      if (typeof offset !== 'undefined') {
        offset += curr;
      }
    }
    return ret;
  },
#else
  // MEMFS filesystem disabled lite handling of stdout and stderr:
  $printCharBuffers: [null, [], []], // 1 => stdout, 2 => stderr
  $printCharBuffers__internal: true,
  $printChar__internal: true,
  $printChar__deps: ['$printCharBuffers'],
  $printChar: function(stream, curr) {
    var buffer = printCharBuffers[stream];
#if ASSERTIONS
    assert(buffer);
#endif
    if (curr === 0 || curr === {{{ charCode('\n') }}}) {
      (stream === 1 ? out : err)(UTF8ArrayToString(buffer, 0));
      buffer.length = 0;
    } else {
      buffer.push(curr);
    }
  },
#endif // SYSCALLS_REQUIRE_FILESYSTEM

#if SYSCALLS_REQUIRE_FILESYSTEM
  fd_write__deps: ['$doWritev'],
#elif (!MINIMAL_RUNTIME || EXIT_RUNTIME)
  $flush_NO_FILESYSTEM__deps: ['$printChar', '$printCharBuffers'],
  $flush_NO_FILESYSTEM: function() {
    // flush anything remaining in the buffers during shutdown
#if hasExportedSymbol('fflush')
    _fflush(0);
#endif
    if (printCharBuffers[1].length) printChar(1, {{{ charCode("\n") }}});
    if (printCharBuffers[2].length) printChar(2, {{{ charCode("\n") }}});
  },
  fd_write__deps: ['$flush_NO_FILESYSTEM', '$printChar'],
  fd_write__postset: function() {
    addAtExit('flush_NO_FILESYSTEM()');
  },
#else
  fd_write__deps: ['$printChar'],
#endif
  fd_write__sig: 'iippp',
  fd_write: function(fd, iov, iovcnt, pnum) {
#if SYSCALLS_REQUIRE_FILESYSTEM
    var stream = SYSCALLS.getStreamFromFD(fd);
    var num = doWritev(stream, iov, iovcnt);
#else
    // hack to support printf in SYSCALLS_REQUIRE_FILESYSTEM=0
    var num = 0;
    for (var i = 0; i < iovcnt; i++) {
      var ptr = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_base, '*') }}};
      var len = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_len, '*') }}};
      iov += {{{ C_STRUCTS.iovec.__size__ }}};
      for (var j = 0; j < len; j++) {
        printChar(fd, HEAPU8[ptr+j]);
      }
      num += len;
    }
#endif // SYSCALLS_REQUIRE_FILESYSTEM
    {{{ makeSetValue('pnum', 0, 'num', SIZE_TYPE) }}};
    return 0;
  },

  fd_pwrite__deps: [
#if SYSCALLS_REQUIRE_FILESYSTEM
    '$doWritev',
#endif
  ].concat(i53ConversionDeps),
  fd_pwrite__sig: 'iippjp',
  fd_pwrite: function(fd, iov, iovcnt, {{{ defineI64Param('offset') }}}, pnum) {
#if SYSCALLS_REQUIRE_FILESYSTEM
    {{{ receiveI64ParamAsI53('offset', cDefine('EOVERFLOW')) }}}
    var stream = SYSCALLS.getStreamFromFD(fd)
    var num = doWritev(stream, iov, iovcnt, offset);
    {{{ makeSetValue('pnum', 0, 'num', SIZE_TYPE) }}};
    return 0;
#elif ASSERTIONS
    abort('fd_pwrite called without SYSCALLS_REQUIRE_FILESYSTEM');
#else
    return {{{ cDefine('ENOSYS') }}};
#endif
  },

  fd_close__sig: 'ii',
  fd_close: function(fd) {
#if SYSCALLS_REQUIRE_FILESYSTEM
    var stream = SYSCALLS.getStreamFromFD(fd);
    FS.close(stream);
    return 0;
#elif PROXY_POSIX_SOCKETS
    // close() is a tricky function because it can be used to close both regular file descriptors
    // and POSIX network socket handles, hence an implementation would need to track for each
    // file descriptor which kind of item it is. To simplify, when using PROXY_POSIX_SOCKETS
    // option, use shutdown() to close a socket, and this function should behave like a no-op.
    warnOnce('To close sockets with PROXY_POSIX_SOCKETS bridge, prefer to use the function shutdown() that is proxied, instead of close()')
    return 0;
#elif ASSERTIONS
    abort('fd_close called without SYSCALLS_REQUIRE_FILESYSTEM');
#else
    return {{{ cDefine('ENOSYS') }}};
#endif // SYSCALLS_REQUIRE_FILESYSTEM
  },

  fd_read__sig: 'iippp',
#if SYSCALLS_REQUIRE_FILESYSTEM
  fd_read__deps: ['$doReadv'],
#endif
  fd_read: function(fd, iov, iovcnt, pnum) {
#if SYSCALLS_REQUIRE_FILESYSTEM
    var stream = SYSCALLS.getStreamFromFD(fd);
    var num = doReadv(stream, iov, iovcnt);
    {{{ makeSetValue('pnum', 0, 'num', SIZE_TYPE) }}};
    return 0;
#elif ASSERTIONS
    abort('fd_read called without SYSCALLS_REQUIRE_FILESYSTEM');
#else
    return {{{ cDefine('ENOSYS') }}};
#endif // SYSCALLS_REQUIRE_FILESYSTEM
  },

  fd_pread__deps: [
#if SYSCALLS_REQUIRE_FILESYSTEM
    '$doReadv',
#endif
  ].concat(i53ConversionDeps),
  fd_pread__sig: 'iippjp',
  fd_pread: function(fd, iov, iovcnt, {{{ defineI64Param('offset') }}}, pnum) {
#if SYSCALLS_REQUIRE_FILESYSTEM
    {{{ receiveI64ParamAsI53('offset', cDefine('EOVERFLOW')) }}}
    var stream = SYSCALLS.getStreamFromFD(fd)
    var num = doReadv(stream, iov, iovcnt, offset);
    {{{ makeSetValue('pnum', 0, 'num', SIZE_TYPE) }}};
    return 0;
#elif ASSERTIONS
    abort('fd_pread called without SYSCALLS_REQUIRE_FILESYSTEM');
#else
    return {{{ cDefine('ENOSYS') }}};
#endif
  },

  fd_seek__sig: 'iijip',
  fd_seek__deps: i53ConversionDeps,
  fd_seek: function(fd, {{{ defineI64Param('offset') }}}, whence, newOffset) {
#if SYSCALLS_REQUIRE_FILESYSTEM
    {{{ receiveI64ParamAsI53('offset', cDefine('EOVERFLOW')) }}}
    var stream = SYSCALLS.getStreamFromFD(fd);
    FS.llseek(stream, offset, whence);
    {{{ makeSetValue('newOffset', '0', 'stream.position', 'i64') }}};
    if (stream.getdents && offset === 0 && whence === {{{ cDefine('SEEK_SET') }}}) stream.getdents = null; // reset readdir state
    return 0;
#else
    return {{{ cDefine('ESPIPE') }}};
#endif
  },

  fd_fdstat_get__sig: 'iip',
  fd_fdstat_get: function(fd, pbuf) {
#if SYSCALLS_REQUIRE_FILESYSTEM
    var stream = SYSCALLS.getStreamFromFD(fd);
    // All character devices are terminals (other things a Linux system would
    // assume is a character device, like the mouse, we have special APIs for).
    var type = stream.tty ? {{{ cDefine('__WASI_FILETYPE_CHARACTER_DEVICE') }}} :
               FS.isDir(stream.mode) ? {{{ cDefine('__WASI_FILETYPE_DIRECTORY') }}} :
               FS.isLink(stream.mode) ? {{{ cDefine('__WASI_FILETYPE_SYMBOLIC_LINK') }}} :
               {{{ cDefine('__WASI_FILETYPE_REGULAR_FILE') }}};
#else
    // hack to support printf in SYSCALLS_REQUIRE_FILESYSTEM=0
    var type = fd == 1 || fd == 2 ? {{{ cDefine('__WASI_FILETYPE_CHARACTER_DEVICE') }}} : abort();
#endif
    {{{ makeSetValue('pbuf', C_STRUCTS.__wasi_fdstat_t.fs_filetype, 'type', 'i8') }}};
    // TODO {{{ makeSetValue('pbuf', C_STRUCTS.__wasi_fdstat_t.fs_flags, '?', 'i16') }}};
    // TODO {{{ makeSetValue('pbuf', C_STRUCTS.__wasi_fdstat_t.fs_rights_base, '?', 'i64') }}};
    // TODO {{{ makeSetValue('pbuf', C_STRUCTS.__wasi_fdstat_t.fs_rights_inheriting, '?', 'i64') }}};
    return 0;
  },

  fd_sync__sig: 'ii',
  fd_sync: function(fd) {
#if SYSCALLS_REQUIRE_FILESYSTEM
    var stream = SYSCALLS.getStreamFromFD(fd);
#if ASYNCIFY
    return Asyncify.handleSleep(function(wakeUp) {
      var mount = stream.node.mount;
      if (!mount.type.syncfs) {
        // We write directly to the file system, so there's nothing to do here.
        wakeUp(0);
        return;
      }
      mount.type.syncfs(mount, false, function(err) {
        if (err) {
          wakeUp(function() { return {{{ cDefine('EIO') }}} });
          return;
        }
        wakeUp(0);
      });
    });
#else
    if (stream.stream_ops && stream.stream_ops.fsync) {
      return stream.stream_ops.fsync(stream);
    }
    return 0; // we can't do anything synchronously; the in-memory FS is already synced to
#endif // ASYNCIFY
#elif ASSERTIONS
    abort('fd_sync called without SYSCALLS_REQUIRE_FILESYSTEM');
#else
    return {{{ cDefine('ENOSYS') }}};
#endif // SYSCALLS_REQUIRE_FILESYSTEM
  },
};

for (var x in WasiLibrary) {
  wrapSyscallFunction(x, WasiLibrary, true);
}

mergeInto(LibraryManager.library, WasiLibrary);
