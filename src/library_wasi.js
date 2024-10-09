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
    this.message = `Program terminated with exit(${status})`;
    this.status = status;
  },
  proc_exit__deps: ['$ExitStatus', '$keepRuntimeAlive'],
#endif

  proc_exit__nothrow: true,
  proc_exit: (code) => {
#if MINIMAL_RUNTIME
    throw `exit(${code})`;
#else
#if RUNTIME_DEBUG
    dbg(`proc_exit: ${code}`);
#endif
    EXITSTATUS = code;
    if (!keepRuntimeAlive()) {
#if PTHREADS
      PThread.terminateAllThreads();
#endif
#if expectToReceiveOnModule('onExit')
      Module['onExit']?.(code);
#endif
      ABORT = true;
    }
    quit_(code, new ExitStatus(code));
#endif // MINIMAL_RUNTIME
  },

  sched_yield__nothrow: true,
  sched_yield: () => 0,

  random_get__deps: ['getentropy'],
  random_get: (buf, buf_len) => {
    _getentropy(buf, buf_len);
    return 0;
  },

  $getEnvStrings__deps: ['$ENV', '$getExecutableName'],
  $getEnvStrings: () => {
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
#if !PURE_WASI
        'USER': 'web_user',
        'LOGNAME': 'web_user',
        'PATH': '/',
        'PWD': '/',
        'HOME': '/home/web_user',
        'LANG': lang,
        '_': getExecutableName()
#endif
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
        strings.push(`${x}=${env[x]}`);
      }
      getEnvStrings.strings = strings;
    }
    return getEnvStrings.strings;
  },

  environ_sizes_get__deps: ['$getEnvStrings'],
  environ_sizes_get__nothrow: true,
  environ_sizes_get: (penviron_count, penviron_buf_size) => {
    var strings = getEnvStrings();
    {{{ makeSetValue('penviron_count', 0, 'strings.length', SIZE_TYPE) }}};
    var bufSize = 0;
    strings.forEach((string) => bufSize += string.length + 1);
    {{{ makeSetValue('penviron_buf_size', 0, 'bufSize', SIZE_TYPE) }}};
    return 0;
  },

  environ_get__deps: ['$getEnvStrings', '$stringToAscii'],
  environ_get__nothrow: true,
  environ_get: (__environ, environ_buf) => {
    var bufSize = 0;
    getEnvStrings().forEach((string, i) => {
      var ptr = environ_buf + bufSize;
      {{{ makeSetValue('__environ', `i*${POINTER_SIZE}`, 'ptr', POINTER_TYPE) }}};
      stringToAscii(string, ptr);
      bufSize += string.length + 1;
    });
    return 0;
  },

  // In normal (non-standalone) mode arguments are passed directly
  // to main, and the `mainArgs` global does not exist.
#if STANDALONE_WASM
  args_sizes_get__nothrow: true,
  args_sizes_get: (pargc, pargv_buf_size) => {
#if MAIN_READS_PARAMS
    {{{ makeSetValue('pargc', 0, 'mainArgs.length', SIZE_TYPE) }}};
    var bufSize = 0;
    mainArgs.forEach((arg) => bufSize += arg.length + 1);
    {{{ makeSetValue('pargv_buf_size', 0, 'bufSize', SIZE_TYPE) }}};
#else
    {{{ makeSetValue('pargc', 0, '0', SIZE_TYPE) }}};
#endif
    return 0;
  },

  args_get__nothrow: true,
  args_get__deps: ['$stringToAscii'],
  args_get: (argv, argv_buf) => {
#if MAIN_READS_PARAMS
    var bufSize = 0;
    mainArgs.forEach((arg, i) => {
      var ptr = argv_buf + bufSize;
      {{{ makeSetValue('argv', `i*${POINTER_SIZE}`, 'ptr', POINTER_TYPE) }}};
      stringToAscii(arg, ptr);
      bufSize += arg.length + 1;
    });
#endif
    return 0;
  },
#endif

  $checkWasiClock: (clock_id) => {
    return clock_id == {{{ cDefs.__WASI_CLOCKID_REALTIME }}} ||
           clock_id == {{{ cDefs.__WASI_CLOCKID_MONOTONIC }}} ||
           clock_id == {{{ cDefs.__WASI_CLOCKID_PROCESS_CPUTIME_ID }}} ||
           clock_id == {{{ cDefs.__WASI_CLOCKID_THREAD_CPUTIME_ID }}};
  },

  // TODO: the i64 in the API here must be legalized for this JS code to run,
  // but the wasm file can't be legalized in standalone mode, which is where
  // this is needed. To get this code to be usable as a JS shim we need to
  // either wait for BigInt support or to legalize on the client.
  clock_time_get__i53abi: true,
  clock_time_get__nothrow: true,
  clock_time_get__deps: ['emscripten_get_now', '$nowIsMonotonic', '$checkWasiClock'],
  clock_time_get: (clk_id, ignored_precision, ptime) => {
    if (!checkWasiClock(clk_id)) {
      return {{{ cDefs.EINVAL }}};
    }
    var now;
    // all wasi clocks but realtime are monotonic
    if (clk_id === {{{ cDefs.__WASI_CLOCKID_REALTIME }}}) {
      now = Date.now();
    } else if (nowIsMonotonic) {
      now = _emscripten_get_now();
    } else {
      return {{{ cDefs.ENOSYS }}};
    }
    // "now" is in ms, and wasi times are in ns.
    var nsec = Math.round(now * 1000 * 1000);
    {{{ makeSetValue('ptime', 0, 'nsec >>> 0', 'i32') }}};
    {{{ makeSetValue('ptime', 4, '(nsec / Math.pow(2, 32)) >>> 0', 'i32') }}};
    return 0;
  },

  clock_res_get__nothrow: true,
  clock_res_get__deps: ['emscripten_get_now', 'emscripten_get_now_res', '$nowIsMonotonic', '$checkWasiClock'],
  clock_res_get: (clk_id, pres) => {
    if (!checkWasiClock(clk_id)) {
      return {{{ cDefs.EINVAL }}};
    }
    var nsec;
    // all wasi clocks but realtime are monotonic
    if (clk_id === {{{ cDefs.CLOCK_REALTIME }}}) {
      nsec = 1000 * 1000; // educated guess that it's milliseconds
    } else if (nowIsMonotonic) {
      nsec = _emscripten_get_now_res();
    } else {
      return {{{ cDefs.ENOSYS }}};
    }
    {{{ makeSetValue('pres', 0, 'nsec >>> 0', 'i32') }}};
    {{{ makeSetValue('pres', 4, '(nsec / Math.pow(2, 32)) >>> 0', 'i32') }}};
    return 0;
  },

#if SYSCALLS_REQUIRE_FILESYSTEM
  $doReadv__docs: '/** @param {number=} offset */',
  $doReadv: (stream, iov, iovcnt, offset) => {
    var ret = 0;
    for (var i = 0; i < iovcnt; i++) {
      var ptr = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_base, '*') }}};
      var len = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_len, '*') }}};
      iov += {{{ C_STRUCTS.iovec.__size__ }}};
      var curr = FS.read(stream, HEAP8, ptr, len, offset);
      if (curr < 0) return -1;
      ret += curr;
      if (curr < len) break; // nothing more to read
      if (typeof offset != 'undefined') {
        offset += curr;
      }
    }
    return ret;
  },
  $doWritev__docs: '/** @param {number=} offset */',
  $doWritev: (stream, iov, iovcnt, offset) => {
    var ret = 0;
    for (var i = 0; i < iovcnt; i++) {
      var ptr = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_base, '*') }}};
      var len = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_len, '*') }}};
      iov += {{{ C_STRUCTS.iovec.__size__ }}};
      var curr = FS.write(stream, HEAP8, ptr, len, offset);
      if (curr < 0) return -1;
      ret += curr;
      if (curr < len) {
        // No more space to write.
        break;
      }
      if (typeof offset != 'undefined') {
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
  $printChar__deps: ['$printCharBuffers', '$UTF8ArrayToString'],
  $printChar: (stream, curr) => {
    var buffer = printCharBuffers[stream];
#if ASSERTIONS
    assert(buffer);
#endif
    if (curr === 0 || curr === {{{ charCode('\n') }}}) {
      (stream === 1 ? out : err)(UTF8ArrayToString(buffer));
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
  $flush_NO_FILESYSTEM: () => {
    // flush anything remaining in the buffers during shutdown
#if hasExportedSymbol('fflush')
    _fflush(0);
#endif
    if (printCharBuffers[1].length) printChar(1, {{{ charCode("\n") }}});
    if (printCharBuffers[2].length) printChar(2, {{{ charCode("\n") }}});
  },
  fd_write__deps: ['$flush_NO_FILESYSTEM', '$printChar'],
  fd_write__postset: () => addAtExit('flush_NO_FILESYSTEM()'),
#else
  fd_write__deps: ['$printChar'],
#endif
  fd_write: (fd, iov, iovcnt, pnum) => {
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

#if SYSCALLS_REQUIRE_FILESYSTEM
  fd_pwrite__deps: ['$doWritev'],
#endif
  fd_pwrite__i53abi: true,
  fd_pwrite: (fd, iov, iovcnt, offset, pnum) => {
#if SYSCALLS_REQUIRE_FILESYSTEM
    if (isNaN(offset)) return {{{ cDefs.EOVERFLOW }}};
    var stream = SYSCALLS.getStreamFromFD(fd)
    var num = doWritev(stream, iov, iovcnt, offset);
    {{{ makeSetValue('pnum', 0, 'num', SIZE_TYPE) }}};
    return 0;
#elif ASSERTIONS
    abort('fd_pwrite called without SYSCALLS_REQUIRE_FILESYSTEM');
#else
    return {{{ cDefs.ENOSYS }}};
#endif
  },

  fd_close: (fd) => {
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
    return {{{ cDefs.ENOSYS }}};
#endif // SYSCALLS_REQUIRE_FILESYSTEM
  },

#if SYSCALLS_REQUIRE_FILESYSTEM
  fd_read__deps: ['$doReadv'],
#endif
  fd_read: (fd, iov, iovcnt, pnum) => {
#if SYSCALLS_REQUIRE_FILESYSTEM
    var stream = SYSCALLS.getStreamFromFD(fd);
    var num = doReadv(stream, iov, iovcnt);
    {{{ makeSetValue('pnum', 0, 'num', SIZE_TYPE) }}};
    return 0;
#elif ASSERTIONS
    abort('fd_read called without SYSCALLS_REQUIRE_FILESYSTEM');
#else
    return {{{ cDefs.ENOSYS }}};
#endif // SYSCALLS_REQUIRE_FILESYSTEM
  },

#if SYSCALLS_REQUIRE_FILESYSTEM
  fd_pread__deps: ['$doReadv'],
#endif
  fd_pread__i53abi: true,
  fd_pread: (fd, iov, iovcnt, offset, pnum) => {
#if SYSCALLS_REQUIRE_FILESYSTEM
    if (isNaN(offset)) return {{{ cDefs.EOVERFLOW }}};
    var stream = SYSCALLS.getStreamFromFD(fd)
    var num = doReadv(stream, iov, iovcnt, offset);
    {{{ makeSetValue('pnum', 0, 'num', SIZE_TYPE) }}};
    return 0;
#elif ASSERTIONS
    abort('fd_pread called without SYSCALLS_REQUIRE_FILESYSTEM');
#else
    return {{{ cDefs.ENOSYS }}};
#endif
  },

  fd_seek__i53abi: true,
  fd_seek: (fd, offset, whence, newOffset) => {
#if SYSCALLS_REQUIRE_FILESYSTEM
    if (isNaN(offset)) return {{{ cDefs.EOVERFLOW }}};
    var stream = SYSCALLS.getStreamFromFD(fd);
    FS.llseek(stream, offset, whence);
    {{{ makeSetValue('newOffset', '0', 'stream.position', 'i64') }}};
    if (stream.getdents && offset === 0 && whence === {{{ cDefs.SEEK_SET }}}) stream.getdents = null; // reset readdir state
    return 0;
#else
    return {{{ cDefs.ESPIPE }}};
#endif
  },

  $wasiRightsToMuslOFlags: (rights) => {
#if SYSCALL_DEBUG
    dbg(`wasiRightsToMuslOFlags: ${rights}`);
#endif
    if ((rights & {{{ cDefs.__WASI_RIGHTS_FD_READ }}}) && (rights & {{{ cDefs.__WASI_RIGHTS_FD_WRITE }}})) {
      return {{{ cDefs.O_RDWR }}};
    }
    if (rights & {{{ cDefs.__WASI_RIGHTS_FD_READ }}}) {
      return {{{ cDefs.O_RDONLY }}};
    }
    if (rights & {{{ cDefs.__WASI_RIGHTS_FD_WRITE }}}) {
      return {{{ cDefs.O_WRONLY }}};
    }
    throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
  },

  $wasiOFlagsToMuslOFlags: (oflags) => {
    var musl_oflags = 0;
    if (oflags & {{{ cDefs.__WASI_OFLAGS_CREAT }}}) {
      musl_oflags |= {{{ cDefs.O_CREAT }}};
    }
    if (oflags & {{{ cDefs.__WASI_OFLAGS_TRUNC }}}) {
      musl_oflags |= {{{ cDefs.O_TRUNC }}};
    }
    if (oflags & {{{ cDefs.__WASI_OFLAGS_DIRECTORY }}}) {
      musl_oflags |= {{{ cDefs.O_DIRECTORY }}};
    }
    if (oflags & {{{ cDefs.__WASI_OFLAGS_EXCL }}}) {
      musl_oflags |= {{{ cDefs.O_EXCL }}};
    }
    return musl_oflags;
  },

#if PURE_WASI
  // preopen maps open file descriptors to pathname.
  // In emscripten we already have a VFS layer so (for now) we expose the entire
  // VFS to the wasi API.
  $preopens: "{3: '/'}",

  path_open__sig: 'iiiiiiiiii',
  path_open__deps: ['$wasiRightsToMuslOFlags', '$wasiOFlagsToMuslOFlags', '$preopens'],
  path_open: (fd, dirflags, path, path_len, oflags,
              fs_rights_base, fs_rights_inherting,
              fdflags, opened_fd) => {
    if (!(fd in preopens)) {
      return {{{ cDefs.EBADF }}};
    }
    var pathname = UTF8ToString(path, path_len);
    var musl_oflags = wasiRightsToMuslOFlags(Number(fs_rights_base));
#if SYSCALL_DEBUG
    dbg(`oflags1: ${ptrToString(musl_oflags)}`);
#endif
    musl_oflags |= wasiOFlagsToMuslOFlags(Number(oflags));
#if SYSCALL_DEBUG
    dbg(`oflags2: ${ptrToString(musl_oflags)}`);
#endif
    var stream = FS.open(pathname, musl_oflags);
    {{{ makeSetValue('opened_fd', '0', 'stream.fd', 'i32') }}};
    return 0;
  },

  fd_prestat_dir_name__deps: ['$preopens'],
  fd_prestat_dir_name__sig: 'iiii',
  fd_prestat_dir_name__nothrow: true,
  fd_prestat_dir_name: (fd, path, path_len) => {
    if (!(fd in preopens)) {
      return {{{ cDefs.EBADF }}};
    }
    var preopen_path = preopens[fd];
    stringToUTF8Array(preopen_path, HEAP8, path, path_len)
#if SYSCALL_DEBUG
    dbg(`fd_prestat_dir_name -> "${preopen_path}"`);
#endif
    return 0;
  },

  fd_prestat_get__deps: ['$preopens'],
  fd_prestat_get__sig: 'iii',
  fd_prestat_get__nothrow: true,
  fd_prestat_get: (fd, stat_buf) => {
    if (!(fd in preopens)) {
      return {{{ cDefs.EBADF }}};
    }
    var preopen = preopens[fd];
    {{{ makeSetValue('stat_buf', C_STRUCTS.__wasi_prestat_t.pr_type, cDefs.__WASI_PREOPENTYPE_DIR, 'i8') }}};
    {{{ makeSetValue('stat_buf', C_STRUCTS.__wasi_prestat_t.u + C_STRUCTS.__wasi_prestat_dir_t.pr_name_len, 'preopen.length', 'i64') }}};
    return 0;
  },

  fd_fdstat_set_flags__sig: 'iii',
  fd_fdstat_set_flags: (fd, flags) => {
    // TODO(sbc): implement
    var stream = SYSCALLS.getStreamFromFD(fd);
    return 0;
  },

  fd_filestat_get__sig: 'iii',
  fd_filestat_get: (fd, stat_buf) => {
    // TODO(sbc): implement
    var stream = SYSCALLS.getStreamFromFD(fd);
    {{{ makeSetValue('stat_buf', C_STRUCTS.__wasi_filestat_t.dev, '0', 'i64') }}};
    {{{ makeSetValue('stat_buf', C_STRUCTS.__wasi_filestat_t.ino, '0', 'i64') }}};
    {{{ makeSetValue('stat_buf', C_STRUCTS.__wasi_filestat_t.filetype, '0', 'i8') }}};
    {{{ makeSetValue('stat_buf', C_STRUCTS.__wasi_filestat_t.nlink, '0', 'i64') }}};
    {{{ makeSetValue('stat_buf', C_STRUCTS.__wasi_filestat_t.size, '0', 'i64') }}};
    {{{ makeSetValue('stat_buf', C_STRUCTS.__wasi_filestat_t.atim, '0', 'i64') }}};
    {{{ makeSetValue('stat_buf', C_STRUCTS.__wasi_filestat_t.mtim, '0', 'i64') }}};
    {{{ makeSetValue('stat_buf', C_STRUCTS.__wasi_filestat_t.ctim, '0', 'i64') }}};
    return 0;
  },
#endif

#if PURE_WASI
  fd_fdstat_get__deps: ['$preopens'],
#endif
  fd_fdstat_get: (fd, pbuf) => {
    var rightsBase = 0;
    var rightsInheriting = 0;
    var flags = 0;
#if PURE_WASI
    if (fd in preopens) {
      var type = {{{ cDefs.__WASI_FILETYPE_DIRECTORY }}};
      rightsBase =  {{{ cDefs.__WASI_RIGHTS_PATH_CREATE_FILE |
                             cDefs.__WASI_RIGHTS_PATH_OPEN }}};
      rightsInheriting =  {{{ cDefs.__WASI_RIGHTS_FD_READ |
                                   cDefs.__WASI_RIGHTS_FD_WRITE }}}
    } else
#endif
    {
#if SYSCALLS_REQUIRE_FILESYSTEM
      var stream = SYSCALLS.getStreamFromFD(fd);
      // All character devices are terminals (other things a Linux system would
      // assume is a character device, like the mouse, we have special APIs for).
      var type = stream.tty ? {{{ cDefs.__WASI_FILETYPE_CHARACTER_DEVICE }}} :
                 FS.isDir(stream.mode) ? {{{ cDefs.__WASI_FILETYPE_DIRECTORY }}} :
                 FS.isLink(stream.mode) ? {{{ cDefs.__WASI_FILETYPE_SYMBOLIC_LINK }}} :
                 {{{ cDefs.__WASI_FILETYPE_REGULAR_FILE }}};
#else
      // Hack to support printf in SYSCALLS_REQUIRE_FILESYSTEM=0. We support at
      // least stdin, stdout, stderr in a simple way.
#if ASSERTIONS
      assert(fd == 0 || fd == 1 || fd == 2);
#endif
      var type = {{{ cDefs.__WASI_FILETYPE_CHARACTER_DEVICE }}};
      if (fd == 0) {
        rightsBase = {{{ cDefs.__WASI_RIGHTS_FD_READ }}};
      } else if (fd == 1 || fd == 2) {
        rightsBase = {{{ cDefs.__WASI_RIGHTS_FD_WRITE }}};
      }
      flags = {{{ cDefs.__WASI_FDFLAGS_APPEND }}};
#endif
    }
    {{{ makeSetValue('pbuf', C_STRUCTS.__wasi_fdstat_t.fs_filetype, 'type', 'i8') }}};
    {{{ makeSetValue('pbuf', C_STRUCTS.__wasi_fdstat_t.fs_flags, 'flags', 'i16') }}};
    {{{ makeSetValue('pbuf', C_STRUCTS.__wasi_fdstat_t.fs_rights_base, 'rightsBase', 'i64') }}};
    {{{ makeSetValue('pbuf', C_STRUCTS.__wasi_fdstat_t.fs_rights_inheriting, 'rightsInheriting', 'i64') }}};
    return 0;
  },

  fd_sync: (fd) => {
#if SYSCALLS_REQUIRE_FILESYSTEM
    var stream = SYSCALLS.getStreamFromFD(fd);
#if ASYNCIFY
    return Asyncify.handleSleep((wakeUp) => {
      var mount = stream.node.mount;
      if (!mount.type.syncfs) {
        // We write directly to the file system, so there's nothing to do here.
        wakeUp(0);
        return;
      }
      mount.type.syncfs(mount, false, (err) => {
        if (err) {
          wakeUp({{{ cDefs.EIO }}});
          return;
        }
        wakeUp(0);
      });
    });
#else
    if (stream.stream_ops?.fsync) {
      return stream.stream_ops.fsync(stream);
    }
    return 0; // we can't do anything synchronously; the in-memory FS is already synced to
#endif // ASYNCIFY
#elif ASSERTIONS
    abort('fd_sync called without SYSCALLS_REQUIRE_FILESYSTEM');
#else
    return {{{ cDefs.ENOSYS }}};
#endif // SYSCALLS_REQUIRE_FILESYSTEM
  },
  fd_sync__async: true,
};

for (var x in WasiLibrary) {
  wrapSyscallFunction(x, WasiLibrary, true);
}

addToLibrary(WasiLibrary);
