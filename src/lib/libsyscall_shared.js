/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 *
 * Syscalls that are shared between WasmFS and the old-FS.
 * Unlike `libsyscalls.js` this file is included under both filesystem backends.
 */

addToLibrary({
#if NODERAWFS
  // NODERAWFS reports the real host process credentials (0 on Windows, which
  // has no uid/gid concept).
  __syscall_getuid32: () => process.getuid?.() ?? 0,
  __syscall_geteuid32: () => process.geteuid?.() ?? 0,
  __syscall_getgid32: () => process.getgid?.() ?? 0,
  __syscall_getegid32: () => process.getegid?.() ?? 0,
#else
  __syscall_getuid32: () => 0,
  __syscall_geteuid32: () => 0,
  __syscall_getgid32: () => 0,
  __syscall_getegid32: () => 0,
#endif
  __syscall_uname: (buf) => {
    if (!buf) {
      return -{{{ cDefs.EFAULT }}};
    }
    var copyString = (offset, value) => stringToUTF8(value, buf + offset, {{{ C_STRUCTS.utsname.nodename }}});
#if NODERAWFS
    var os = require('node:os');
    copyString({{{ C_STRUCTS.utsname.sysname }}}, os.type());
    copyString({{{ C_STRUCTS.utsname.nodename }}}, os.hostname());
    copyString({{{ C_STRUCTS.utsname.release }}}, os.release());
    copyString({{{ C_STRUCTS.utsname.version }}}, os.version());
    copyString({{{ C_STRUCTS.utsname.machine }}}, os.machine());
#else
    copyString({{{ C_STRUCTS.utsname.sysname }}}, 'Emscripten');
    copyString({{{ C_STRUCTS.utsname.nodename }}}, 'emscripten');
    copyString({{{ C_STRUCTS.utsname.release }}}, '{{{ EMSCRIPTEN_VERSION.split("-")[0] }}}');
    copyString({{{ C_STRUCTS.utsname.version }}}, '#1');
    copyString({{{ C_STRUCTS.utsname.machine }}}, '{{{ MEMORY64 ? "wasm64" : "wasm32" }}}');
#endif
    return 0;
  },
});
