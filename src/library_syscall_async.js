// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// See settings.js for more details.

var SyscallsLibraryAsync = {
  $AsyncFS__deps: ['$SYSCALLS', '$Asyncify'],
  $AsyncFS: {
    handle: function(varargs, handle) {
      return Asyncify.handleSleep(function(wakeUp) {
        SYSCALLS.varargs = varargs;
        handle(wakeUp);
      });
    }
  },

  __syscall3: function(which, varargs) { // read
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), buf = SYSCALLS.get(), count = SYSCALLS.get();
      AsyncifyFSImpl.readv(fd, [{ ptr: buf, len: count }], wakeUp);
    });
  },

  __syscall4: function(which, varargs) { // write
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), buf = SYSCALLS.get(), count = SYSCALLS.get();
      AsyncifyFSImpl.writev(fd, [{ ptr: buf, len: count }], wakeUp);
    });
  },

  __syscall5: function(which, varargs) { // open
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var pathname = SYSCALLS.getStr(), flags = SYSCALLS.get(), mode = SYSCALLS.get(); // optional TODO
      AsyncifyFSImpl.open(pathname, flags, mode, wakeUp);
    });
  },

  __syscall6: function(which, varargs) { // close
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get();
      AsyncifyFSImpl.close(path, wakeUp);
    });
  },

  __syscall10: function(which, varargs) { // unlink
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var path = SYSCALLS.getStr();
      AsyncifyFSImpl.unlink(path, wakeUp);
    });
  },

  __syscall33: function(which, varargs) { // access
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var path = SYSCALLS.getStr(), amode = SYSCALLS.get();
      AsyncifyFSImpl.access(path, amode, wakeUp);
    });
  },

  __syscall54: function(which, varargs) { // ioctl
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), op = SYSCALLS.get();
      AsyncifyFSImpl.ioctl(fd, op, wakeUp);
    });
  },

  __syscall91: function(which, varargs) { // munmap
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var addr = SYSCALLS.get(), len = SYSCALLS.get();
      AsyncifyFSImpl.munmap(addr, len, wakeUp);
    });
  },

  __syscall118: function(which, varargs) { // fsync
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get();
      AsyncifyFSImpl.fsync(fd, wakeUp);
    });
  },

  __syscall140: function(which, varargs) { // llseek
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), offset_high = SYSCALLS.get(), offset_low = SYSCALLS.get(), result = SYSCALLS.get(), whence = SYSCALLS.get();
      AsyncifyFSImpl.llseek(fd, offset_high, offset_low, result, whence, wakeUp);
    });
  },

  __syscall146: function(which, varargs) { // writev
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), iov = SYSCALLS.get(), iovcnt = SYSCALLS.get();
      var iovs = [];
      for (var i = 0; i < iovcnt; i++) {
        var ptr = {{{ makeGetValue('iov', 'i*8', 'i32') }}};
        var len = {{{ makeGetValue('iov', 'i*8 + 4', 'i32') }}};
        iovs.push({ ptr: ptr, len: len });
      }
      AsyncifyFSImpl.writev(fd, iovs, wakeUp);
    });
  },

  __syscall183: function(which, varargs) { // getcwd
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var buf = SYSCALLS.get(), size = SYSCALLS.get();
      AsyncifyFSImpl.getcwd(buf, size, wakeUp);
    });
  },

  __syscall192: function(which, varargs) { // mmap2
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var addr = SYSCALLS.get(), len = SYSCALLS.get(), prot = SYSCALLS.get(), flags = SYSCALLS.get(), fd = SYSCALLS.get(), off = SYSCALLS.get()
      AsyncifyFSImpl.mmap(addr, len, prot, flags, fd, off, wakeUp);
    });
  },

  __syscall194: function(which, varargs) { // ftruncate64
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), zero = SYSCALLS.getZero(), length = SYSCALLS.get64();
      AsyncifyFSImpl.truncate(fd, length, wakeUp);
    });
  },

  __syscall195: function(which, varargs) { // stat
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var path = SYSCALLS.getStr(), buf = SYSCALLS.get();
      AsyncifyFSImpl.stat(path, buf, wakeUp);
    });
  },

  __syscall197: function(which, varargs) { // fstat
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), buf = SYSCALLS.get();
      AsyncifyFSImpl.fstat(fd, buf, wakeUp);
    });
  },

  __syscall221: function(which, varargs) { // fcntl64
    return AsyncifyFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), cmd = SYSCALLS.get();
      AsyncifyFSImpl.fcntl(fd, cmd, wakeUp);
    });
  },
};

autoAddDeps(SyscallsLibraryAsync, '$AsyncFS');

mergeInto(LibraryManager.library, SyscallsLibraryAsync);

assert(ASYNCIFY && WASM_BACKEND, "ASYNCFS requires ASYNCIFY with the wasm backend");

