// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// See settings.js for more details.

var SyscallsLibraryAsync = {
  // This should be implemented with the proper callbacks.
  $AsyncFSImpl: {},

  $AsyncFS__deps: ['$SYSCALLS', '$Asyncify', '$AsyncFSImpl'],
  $AsyncFS: {
    handle: function(varargs, handle) {
      return Asyncify.handleSleep(function(wakeUp) {
        SYSCALLS.varargs = varargs;
        handle(wakeUp);
      });
    },

    getIovs: function(iov, iovcnt) {
      var iovs = [];
      for (var i = 0; i < iovcnt; i++) {
        var ptr = {{{ makeGetValue('iov', 'i*8', 'i32') }}};
        var len = {{{ makeGetValue('iov', 'i*8 + 4', 'i32') }}};
        iovs.push({ ptr: ptr, len: len });
      }
      return iovs;
    },
  },

  __syscall3: function(which, varargs) { // read
    return AsyncFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), buf = SYSCALLS.get(), count = SYSCALLS.get();
      AsyncFSImpl.readv(fd, [{ ptr: buf, len: count }], wakeUp);
    });
  },

  __syscall4: function(which, varargs) { // write
    return AsyncFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), buf = SYSCALLS.get(), count = SYSCALLS.get();
      AsyncFSImpl.writev(fd, [{ ptr: buf, len: count }], wakeUp);
    });
  },

  __syscall5: function(which, varargs) { // open
    return AsyncFS.handle(varargs, function(wakeUp) {
      var pathname = SYSCALLS.getStr(), flags = SYSCALLS.get(), mode = SYSCALLS.get(); // optional TODO
      AsyncFSImpl.open(pathname, flags, mode, wakeUp);
    });
  },

  __syscall6: function(which, varargs) { // close
    return AsyncFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get();
      AsyncFSImpl.close(fd, wakeUp);
    });
  },

  __syscall10: function(which, varargs) { // unlink
    return AsyncFS.handle(varargs, function(wakeUp) {
      var path = SYSCALLS.getStr();
      AsyncFSImpl.unlink(path, wakeUp);
    });
  },

  __syscall33: function(which, varargs) { // access
    return AsyncFS.handle(varargs, function(wakeUp) {
      var path = SYSCALLS.getStr(), amode = SYSCALLS.get();
      AsyncFSImpl.access(path, amode, wakeUp);
    });
  },

  __syscall54: function(which, varargs) { // ioctl
    return AsyncFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), op = SYSCALLS.get();
      AsyncFSImpl.ioctl(fd, op, wakeUp);
    });
  },

  __syscall91: function(which, varargs) { // munmap
    return AsyncFS.handle(varargs, function(wakeUp) {
      var addr = SYSCALLS.get(), len = SYSCALLS.get();
      AsyncFSImpl.munmap(addr, len, wakeUp);
    });
  },

  __syscall118: function(which, varargs) { // fsync
    return AsyncFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get();
      AsyncFSImpl.fsync(fd, wakeUp);
    });
  },

  __syscall140: function(which, varargs) { // llseek
    return AsyncFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), offset_high = SYSCALLS.get(), offset_low = SYSCALLS.get(), result = SYSCALLS.get(), whence = SYSCALLS.get();
      AsyncFSImpl.llseek(fd, offset_high, offset_low, result, whence, wakeUp);
    });
  },

  __syscall145: function(which, varargs) { // readv
    return AsyncFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), iov = SYSCALLS.get(), iovcnt = SYSCALLS.get();
      AsyncFSImpl.readv(fd, AsyncFS.getIovs(iov, iovcnt), wakeUp);
    });
  },

  __syscall146: function(which, varargs) { // writev
    return AsyncFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), iov = SYSCALLS.get(), iovcnt = SYSCALLS.get();
      AsyncFSImpl.writev(fd, AsyncFS.getIovs(iov, iovcnt), wakeUp);
    });
  },

  __syscall183: function(which, varargs) { // getcwd
    return AsyncFS.handle(varargs, function(wakeUp) {
      var buf = SYSCALLS.get(), size = SYSCALLS.get();
      AsyncFSImpl.getcwd(buf, size, wakeUp);
    });
  },

  __syscall192: function(which, varargs) { // mmap2
    return AsyncFS.handle(varargs, function(wakeUp) {
      var addr = SYSCALLS.get(), len = SYSCALLS.get(), prot = SYSCALLS.get(), flags = SYSCALLS.get(), fd = SYSCALLS.get(), off = SYSCALLS.get()
      AsyncFSImpl.mmap(addr, len, prot, flags, fd, off, wakeUp);
    });
  },

  __syscall194: function(which, varargs) { // ftruncate64
    return AsyncFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), zero = SYSCALLS.getZero(), length = SYSCALLS.get64();
      AsyncFSImpl.truncate(fd, length, wakeUp);
    });
  },

  __syscall195: function(which, varargs) { // stat
    return AsyncFS.handle(varargs, function(wakeUp) {
      var path = SYSCALLS.getStr(), buf = SYSCALLS.get();
      AsyncFSImpl.stat(path, buf, wakeUp);
    });
  },

  __syscall197: function(which, varargs) { // fstat
    return AsyncFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), buf = SYSCALLS.get();
      AsyncFSImpl.fstat(fd, buf, wakeUp);
    });
  },

  __syscall221: function(which, varargs) { // fcntl64
    return AsyncFS.handle(varargs, function(wakeUp) {
      var fd = SYSCALLS.get(), cmd = SYSCALLS.get();
      AsyncFSImpl.fcntl(fd, cmd, wakeUp);
    });
  },

  // TODO all other syscalls that make sense to add
};

autoAddDeps(SyscallsLibraryAsync, '$AsyncFS');

mergeInto(LibraryManager.library, SyscallsLibraryAsync);

assert(ASYNCIFY && WASM_BACKEND, "ASYNCFS requires ASYNCIFY with the wasm backend");

