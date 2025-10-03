/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// When bootstrapping struct info, we can't use the full library because
// it itself depends on the struct info information.

#if !BOOTSTRAPPING_STRUCT_INFO
assert(false, "libbootstrap.js only designed for use with BOOTSTRAPPING_STRUCT_INFO")
#endif

assert(Object.keys(LibraryManager.library).length === 0);
addToLibrary({
  $callRuntimeCallbacks: () => {},

  $wasmMemory: 'memory',

  $ExitStatus: class {
    name = 'ExitStatus';
    constructor(status) {
      this.message = `Program terminated with exit(${status})`;
      this.status = status;
    }
  },

  $exitJS__deps: ['$ExitStatus'],
  $exitJS: (code) => quit_(code, new ExitStatus(code)),

  $handleException: (e) => {
    if (e instanceof ExitStatus || e == 'unwind') {
      return EXITSTATUS;
    }
    quit_(1, e);
  },

  fd_write__sig: 'iippp',
  fd_write: (fd, iov, iovcnt, pnum) => {
    // implementation almost copied from libwasi.js one for SYSCALLS_REQUIRE_FILESYSTEM=0
    // (the only difference is that we can't use C_STRUCTS here)
    var num = 0;
    for (var i = 0; i < iovcnt; i++) {
      var ptr = {{{ makeGetValue('iov', 0, '*') }}};
      var len = {{{ makeGetValue('iov', POINTER_SIZE, '*') }}};
      iov += {{{ POINTER_SIZE }}} * 2;
      process.stdout.write(HEAPU8.subarray(ptr, ptr + len));
      num += len;
    }
    {{{ makeSetValue('pnum', 0, 'num', '*') }}};
    return 0;
  },
});
