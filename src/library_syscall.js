/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var SyscallsLibrary = {
  $SYSCALLS__deps: ['$PATH',
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
                   '$FS',
#endif
#if SYSCALL_DEBUG
                   '$ERRNO_MESSAGES'
#endif
  ],
  $SYSCALLS: {
    mappings: {},

#if SYSCALLS_REQUIRE_FILESYSTEM
    // global constants
    DEFAULT_POLLMASK: {{{ cDefine('POLLIN') }}} | {{{ cDefine('POLLOUT') }}},

    // global state
    umask: 0x1FF,  // S_IRWXU | S_IRWXG | S_IRWXO

    // shared utilities
    calculateAt: function(dirfd, path) {
      if (path[0] !== '/') {
        // relative path
        var dir;
        if (dirfd === {{{ cDefine('AT_FDCWD') }}}) {
          dir = FS.cwd();
        } else {
          var dirstream = FS.getStream(dirfd);
          if (!dirstream) throw new FS.ErrnoError({{{ cDefine('EBADF') }}});
          dir = dirstream.path;
        }
        path = PATH.join2(dir, path);
      }
      return path;
    },

    doStat: function(func, path, buf) {
      try {
        var stat = func(path);
      } catch (e) {
        if (e && e.node && PATH.normalize(path) !== PATH.normalize(FS.getPath(e.node))) {
          // an error occurred while trying to look up the path; we should just report ENOTDIR
          return -{{{ cDefine('ENOTDIR') }}};
        }
        throw e;
      }
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_dev, 'stat.dev', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.__st_dev_padding, '0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.__st_ino_truncated, 'stat.ino', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_mode, 'stat.mode', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_nlink, 'stat.nlink', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_uid, 'stat.uid', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_gid, 'stat.gid', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_rdev, 'stat.rdev', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.__st_rdev_padding, '0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_size, 'stat.size', 'i64') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_blksize, '4096', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_blocks, 'stat.blocks', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_atim.tv_sec, '(stat.atime.getTime() / 1000)|0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_atim.tv_nsec, '0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_mtim.tv_sec, '(stat.mtime.getTime() / 1000)|0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_mtim.tv_nsec, '0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_ctim.tv_sec, '(stat.ctime.getTime() / 1000)|0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_ctim.tv_nsec, '0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_ino, 'stat.ino', 'i64') }}};
      return 0;
    },
    doMsync: function(addr, stream, len, flags, offset) {
      var buffer = HEAPU8.slice(addr, addr + len);
      FS.msync(stream, buffer, offset, len, flags);
    },
    doMkdir: function(path, mode) {
      // remove a trailing slash, if one - /a/b/ has basename of '', but
      // we want to create b in the context of this function
      path = PATH.normalize(path);
      if (path[path.length-1] === '/') path = path.substr(0, path.length-1);
      FS.mkdir(path, mode, 0);
      return 0;
    },
    doMknod: function(path, mode, dev) {
      // we don't want this in the JS API as it uses mknod to create all nodes.
      switch (mode & {{{ cDefine('S_IFMT') }}}) {
        case {{{ cDefine('S_IFREG') }}}:
        case {{{ cDefine('S_IFCHR') }}}:
        case {{{ cDefine('S_IFBLK') }}}:
        case {{{ cDefine('S_IFIFO') }}}:
        case {{{ cDefine('S_IFSOCK') }}}:
          break;
        default: return -{{{ cDefine('EINVAL') }}};
      }
      FS.mknod(path, mode, dev);
      return 0;
    },
    doReadlink: function(path, buf, bufsize) {
      if (bufsize <= 0) return -{{{ cDefine('EINVAL') }}};
      var ret = FS.readlink(path);

      var len = Math.min(bufsize, lengthBytesUTF8(ret));
      var endChar = HEAP8[buf+len];
      stringToUTF8(ret, buf, bufsize+1);
      // readlink is one of the rare functions that write out a C string, but does never append a null to the output buffer(!)
      // stringToUTF8() always appends a null byte, so restore the character under the null byte after the write.
      HEAP8[buf+len] = endChar;

      return len;
    },
    doAccess: function(path, amode) {
      if (amode & ~{{{ cDefine('S_IRWXO') }}}) {
        // need a valid mode
        return -{{{ cDefine('EINVAL') }}};
      }
      var node;
      var lookup = FS.lookupPath(path, { follow: true });
      node = lookup.node;
      if (!node) {
        return -{{{ cDefine('ENOENT') }}};
      }
      var perms = '';
      if (amode & {{{ cDefine('R_OK') }}}) perms += 'r';
      if (amode & {{{ cDefine('W_OK') }}}) perms += 'w';
      if (amode & {{{ cDefine('X_OK') }}}) perms += 'x';
      if (perms /* otherwise, they've just passed F_OK */ && FS.nodePermissions(node, perms)) {
        return -{{{ cDefine('EACCES') }}};
      }
      return 0;
    },
    doDup: function(path, flags, suggestFD) {
      var suggest = FS.getStream(suggestFD);
      if (suggest) FS.close(suggest);
      return FS.open(path, flags, 0, suggestFD, suggestFD).fd;
    },
    doReadv: function(stream, iov, iovcnt, offset) {
      var ret = 0;
      for (var i = 0; i < iovcnt; i++) {
        var ptr = {{{ makeGetValue('iov', 'i*8', 'i32') }}};
        var len = {{{ makeGetValue('iov', 'i*8 + 4', 'i32') }}};
        var curr = FS.read(stream, {{{ heapAndOffset('HEAP8', 'ptr') }}}, len, offset);
        if (curr < 0) return -1;
        ret += curr;
        if (curr < len) break; // nothing more to read
      }
      return ret;
    },
    doWritev: function(stream, iov, iovcnt, offset) {
      var ret = 0;
      for (var i = 0; i < iovcnt; i++) {
        var ptr = {{{ makeGetValue('iov', 'i*8', 'i32') }}};
        var len = {{{ makeGetValue('iov', 'i*8 + 4', 'i32') }}};
        var curr = FS.write(stream, {{{ heapAndOffset('HEAP8', 'ptr') }}}, len, offset);
        if (curr < 0) return -1;
        ret += curr;
      }
      return ret;
    },
#else
    // MEMFS filesystem disabled lite handling of stdout and stderr:
    buffers: [null, [], []], // 1 => stdout, 2 => stderr
    printChar: function(stream, curr) {
      var buffer = SYSCALLS.buffers[stream];
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

    // arguments handling

    varargs: undefined,

    get: function() {
#if ASSERTIONS
      assert(SYSCALLS.varargs != undefined);
#endif
      SYSCALLS.varargs += 4;
      var ret = {{{ makeGetValue('SYSCALLS.varargs', '-4', 'i32') }}};
#if SYSCALL_DEBUG
      err('    (raw: "' + ret + '")');
#endif
      return ret;
    },
    getStr: function(ptr) {
      var ret = UTF8ToString(ptr);
#if SYSCALL_DEBUG
      err('    (str: "' + ret + '")');
#endif
      return ret;
    },
#if SYSCALLS_REQUIRE_FILESYSTEM
    getStreamFromFD: function(fd) {
      var stream = FS.getStream(fd);
      if (!stream) throw new FS.ErrnoError({{{ cDefine('EBADF') }}});
#if SYSCALL_DEBUG
      err('    (stream: "' + stream.path + '")');
#endif
      return stream;
    },
#endif // SYSCALLS_REQUIRE_FILESYSTEM
    get64: function(low, high) {
#if ASSERTIONS
      if (low >= 0) assert(high === 0);
      else assert(high === -1);
#endif
#if SYSCALL_DEBUG
      err('    (i64: "' + low + '")');
#endif
      return low;
    }
  },

  $syscallMmap2__deps: ['$SYSCALLS',
#if !WASM_BACKEND
    'memalign', 'memset',
#endif
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
    '$FS',
#endif
  ],
  $syscallMmap2: function(addr, len, prot, flags, fd, off) {
    off <<= 12; // undo pgoffset
    var ptr;
    var allocated = false;

    // addr argument must be page aligned if MAP_FIXED flag is set.
    if ((flags & {{{ cDefine('MAP_FIXED') }}}) !== 0 && (addr % {{{ POSIX_PAGE_SIZE }}}) !== 0) {
      return -{{{ cDefine('EINVAL') }}};
    }

    // MAP_ANONYMOUS (aka MAP_ANON) isn't actually defined by POSIX spec,
    // but it is widely used way to allocate memory pages on Linux, BSD and Mac.
    // In this case fd argument is ignored.
    if ((flags & {{{ cDefine('MAP_ANONYMOUS') }}}) !== 0) {
      ptr = _memalign({{{ POSIX_PAGE_SIZE }}}, len);
      if (!ptr) return -{{{ cDefine('ENOMEM') }}};
      _memset(ptr, 0, len);
      allocated = true;
    } else {
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
      var info = FS.getStream(fd);
      if (!info) return -{{{ cDefine('EBADF') }}};
      var res = FS.mmap(info, addr, len, off, prot, flags);
      ptr = res.ptr;
      allocated = res.allocated;
#else // no filesystem support; report lack of support
      return -{{{ cDefine('ENOSYS') }}};
#endif
    }
#if CAN_ADDRESS_2GB
    ptr >>>= 0;
#endif
    SYSCALLS.mappings[ptr] = { malloc: ptr, len: len, allocated: allocated, fd: fd, prot: prot, flags: flags, offset: off };
    return ptr;
  },

  $syscallMunmap__deps: ['$SYSCALLS',
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
    '$FS',
#endif
  ],
  $syscallMunmap: function(addr, len) {
#if CAN_ADDRESS_2GB
    addr >>>= 0;
#endif
    if ((addr | 0) === {{{ cDefine('MAP_FAILED') }}} || len === 0) {
      return -{{{ cDefine('EINVAL') }}};
    }
    // TODO: support unmmap'ing parts of allocations
    var info = SYSCALLS.mappings[addr];
    if (!info) return 0;
    if (len === info.len) {
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
      var stream = FS.getStream(info.fd);
      if (info.prot & {{{ cDefine('PROT_WRITE') }}}) {
        SYSCALLS.doMsync(addr, stream, len, info.flags, info.offset);
      }
      FS.munmap(stream);
#else
#if ASSERTIONS
      // Without FS support, only anonymous mappings are supported.
      assert(SYSCALLS.mappings[addr].flags & {{{ cDefine('MAP_ANONYMOUS') }}});
#endif
#endif
      SYSCALLS.mappings[addr] = null;
      if (info.allocated) {
        _free(info.malloc);
      }
    }
    return 0;
  },

  __sys_exit: function(status) {
    exit(status);
    // no return
  },
  __sys_read: function(fd, buf, count) {
    var stream = SYSCALLS.getStreamFromFD(fd);
    return FS.read(stream, {{{ heapAndOffset('HEAP8', 'buf') }}}, count);
  },
  __sys_write: function(fd, buf, count) {
#if SYSCALLS_REQUIRE_FILESYSTEM
    var stream = SYSCALLS.getStreamFromFD(fd);
    return FS.write(stream, {{{ heapAndOffset('HEAP8', 'buf') }}}, count);
#else
    // hack to support printf in SYSCALLS_REQUIRE_FILESYSTEM=0
    for (var i = 0; i < count; i++) {
      SYSCALLS.printChar(fd, HEAPU8[buf+i]);
    }
    return count;
#endif // SYSCALLS_REQUIRE_FILESYSTEM
  },
  __sys_open: function(path, flags, varargs) {
    var pathname = SYSCALLS.getStr(path);
    var mode = SYSCALLS.get();
    var stream = FS.open(pathname, flags, mode);
    return stream.fd;
  },
  __sys_link__nothrow: true,
  __sys_link__proxy: false,
  __sys_link: function(oldpath, newpath) {
    return -{{{ cDefine('EMLINK') }}}; // no hardlinks for us
  },
  __sys_unlink: function(path) {
    path = SYSCALLS.getStr(path);
    FS.unlink(path);
    return 0;
  },
  __sys_chdir: function(path) {
    path = SYSCALLS.getStr(path);
    FS.chdir(path);
    return 0;
  },
  __sys_mknod: function(path, mode, dev) {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.doMknod(path, mode, dev);
  },
  __sys_chmod: function(path, mode) {
    path = SYSCALLS.getStr(path);
    FS.chmod(path, mode);
    return 0;
  },
  __sys_getpid__nothrow: true,
  __sys_getpid__proxy: false,
  __sys_getpid: function() {
    return {{{ PROCINFO.pid }}};
  },
  __sys_pause__nothrow: true,
  __sys_pause__proxy: false,
  __sys_pause: function() {
    return -{{{ cDefine('EINTR') }}}; // we can't pause
  },
  __sys_access: function(path, amode) {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.doAccess(path, amode);
  },
  __sys_nice__nothrow: true,
  __sys_nice__proxy: false,
  __sys_nice: function(inc) {
    return -{{{ cDefine('EPERM') }}}; // no meaning to nice for our single-process environment
  },
  __sys_sync__proxy: false,
  __sys_sync__nothrow: true,
  __sys_sync: function() {
    return 0;
  },
  __sys_rename: function(old_path, new_path) {
    old_path = SYSCALLS.getStr(old_path);
    new_path = SYSCALLS.getStr(new_path);
    FS.rename(old_path, new_path);
    return 0;
  },
  __sys_mkdir: function(path, mode) {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.doMkdir(path, mode);
  },
  __sys_rmdir: function(path) {
    path = SYSCALLS.getStr(path);
    FS.rmdir(path);
    return 0;
  },
  __sys_dup: function(fd) {
    var old = SYSCALLS.getStreamFromFD(fd);
    return FS.open(old.path, old.flags, 0).fd;
  },
  __sys_pipe__deps: ['$PIPEFS'],
  __sys_pipe: function(fdPtr) {
    if (fdPtr == 0) {
      throw new FS.ErrnoError({{{ cDefine('EFAULT') }}});
    }

    var res = PIPEFS.createPipe();

    {{{ makeSetValue('fdPtr', 0, 'res.readable_fd', 'i32') }}};
    {{{ makeSetValue('fdPtr', 4, 'res.writable_fd', 'i32') }}};

    return 0;
  },
  __sys_acct__nothrow: true,
  __sys_acct__proxy: false,
  __sys_acct: function(filename) {
    return -{{{ cDefine('ENOSYS') }}}; // unsupported features
  },
  __sys_ioctl: function(fd, op, varargs) {
#if SYSCALLS_REQUIRE_FILESYSTEM == 0
#if SYSCALL_DEBUG
    err('no-op in ioctl syscall due to SYSCALLS_REQUIRE_FILESYSTEM=0');
#endif
    return 0;
#else
    var stream = SYSCALLS.getStreamFromFD(fd);
    switch (op) {
      case {{{ cDefine('TCGETA') }}}:
      case {{{ cDefine('TCGETS') }}}: {
        if (!stream.tty) return -{{{ cDefine('ENOTTY') }}};
#if SYSCALL_DEBUG
        err('warning: not filling tio struct');
#endif
        return 0;
      }
      case {{{ cDefine('TCSETA') }}}:
      case {{{ cDefine('TCSETAW') }}}:
      case {{{ cDefine('TCSETAF') }}}:
      case {{{ cDefine('TCSETS') }}}:
      case {{{ cDefine('TCSETSW') }}}:
      case {{{ cDefine('TCSETSF') }}}: {
        if (!stream.tty) return -{{{ cDefine('ENOTTY') }}};
        return 0; // no-op, not actually adjusting terminal settings
      }
      case {{{ cDefine('TIOCGPGRP') }}}: {
        if (!stream.tty) return -{{{ cDefine('ENOTTY') }}};
        var argp = SYSCALLS.get();
        {{{ makeSetValue('argp', 0, 0, 'i32') }}};
        return 0;
      }
      case {{{ cDefine('TIOCSPGRP') }}}: {
        if (!stream.tty) return -{{{ cDefine('ENOTTY') }}};
        return -{{{ cDefine('EINVAL') }}}; // not supported
      }
      case {{{ cDefine('FIONREAD') }}}: {
        var argp = SYSCALLS.get();
        return FS.ioctl(stream, op, argp);
      }
      case {{{ cDefine('TIOCGWINSZ') }}}: {
        // TODO: in theory we should write to the winsize struct that gets
        // passed in, but for now musl doesn't read anything on it
        if (!stream.tty) return -{{{ cDefine('ENOTTY') }}};
        return 0;
      }
      case {{{ cDefine('TIOCSWINSZ') }}}: {
        // TODO: technically, this ioctl call should change the window size.
        // but, since emscripten doesn't have any concept of a terminal window
        // yet, we'll just silently throw it away as we do TIOCGWINSZ
        if (!stream.tty) return -{{{ cDefine('ENOTTY') }}};
        return 0;
      }
      default: abort('bad ioctl syscall ' + op);
    }
#endif // SYSCALLS_REQUIRE_FILESYSTEM
  },
  __sys_setpgid__nothrow: true,
  __sys_setpgid__proxy: false,
  __sys_setpgid: function(pid, pgid) {
    if (pid && pid !== {{{ PROCINFO.pid }}}) return -{{{ cDefine('ESRCH') }}};
    if (pgid && pgid !== {{{ PROCINFO.pgid }}}) return -{{{ cDefine('EPERM') }}};
    return 0;
  },
  __sys_umask: function(mask) {
    var old = SYSCALLS.umask;
    SYSCALLS.umask = mask;
    return old;
  },
  __sys_dup2: function(oldfd, suggestFD) {
    var old = SYSCALLS.getStreamFromFD(oldfd);
    if (old.fd === suggestFD) return suggestFD;
    return SYSCALLS.doDup(old.path, old.flags, suggestFD);
  },
  __sys_getppid__nothrow: true,
  __sys_getppid__proxy: false,
  __sys_getppid: function() {
    return {{{ PROCINFO.ppid }}};
  },
  __sys_getpgrp__nothrow: true,
  __sys_getpgrp__proxy: false,
  __sys_getpgrp: function() {
    return {{{ PROCINFO.pgid }}};
  },
  __sys_setsid__nothrow: true,
  __sys_setsid__proxy: false,
  __sys_setsid: function() {
    return 0; // no-op
  },
  __sys_setrlimit__nothrow: true,
  __sys_setrlimit__proxy: false,
  __sys_setrlimit: function(varargs) {
    return 0; // no-op
  },
  __sys_getrusage: function(who, usage) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    _memset(usage, 0, {{{ C_STRUCTS.rusage.__size__ }}});
    {{{ makeSetValue('usage', C_STRUCTS.rusage.ru_utime.tv_sec, '1', 'i32') }}}; // fake some values
    {{{ makeSetValue('usage', C_STRUCTS.rusage.ru_utime.tv_usec, '2', 'i32') }}};
    {{{ makeSetValue('usage', C_STRUCTS.rusage.ru_stime.tv_sec, '3', 'i32') }}};
    {{{ makeSetValue('usage', C_STRUCTS.rusage.ru_stime.tv_usec, '4', 'i32') }}};
    return 0;
  },
  __sys_symlink: function(target, linkpath) {
    target = SYSCALLS.getStr(target);
    linkpath = SYSCALLS.getStr(linkpath);
    FS.symlink(target, linkpath);
    return 0;
  },
  __sys_readlink: function(path, buf, bufsize) {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.doReadlink(path, buf, bufsize);
  },
  __sys_munmap__deps: ['$syscallMunmap'],
  __sys_munmap: function(addr, len) {
    return syscallMunmap(addr, len);
  },
  __sys_fchmod: function(fd, mode) {
    FS.fchmod(fd, mode);
    return 0;
  },
  __sys_getpriority__nothrow: true,
  __sys_getpriority__proxy: false,
  __sys_getpriority: function() {
    return 0;
  },
  __sys_setpriority__nothrow: true,
  __sys_setpriority__proxy: false,
  __sys_setpriority: function() {
    return -{{{ cDefine('EPERM') }}};
  },
#if PROXY_POSIX_SOCKETS == 0
  __sys_socketcall__deps: ['$SOCKFS', '$DNS', '_read_sockaddr', '_write_sockaddr'],
  __sys_socketcall: function(call, socketvararg) {
#if SYSCALL_DEBUG
      err('    (socketcall begin: "' + call + '")');
#endif
    // socketcalls pass the rest of the arguments in a struct
    SYSCALLS.varargs = socketvararg;

    var getSocketFromFD = function() {
      var socket = SOCKFS.getSocket(SYSCALLS.get());
      if (!socket) throw new FS.ErrnoError({{{ cDefine('EBADF') }}});
#if SYSCALL_DEBUG
      err('    (socket: "' + socket.path + '")');
#endif
      return socket;
    };
    /** @param {boolean=} allowNull */
    var getSocketAddress = function(allowNull) {
      var addrp = SYSCALLS.get(), addrlen = SYSCALLS.get();
      if (allowNull && addrp === 0) return null;
      var info = __read_sockaddr(addrp, addrlen);
      if (info.errno) throw new FS.ErrnoError(info.errno);
      info.addr = DNS.lookup_addr(info.addr) || info.addr;
#if SYSCALL_DEBUG
      err('    (socketaddress: "' + [info.addr, info.port] + '")');
#endif
      return info;
    };

    switch (call) {
      case 1: { // socket
        var domain = SYSCALLS.get(), type = SYSCALLS.get(), protocol = SYSCALLS.get();
        var sock = SOCKFS.createSocket(domain, type, protocol);
#if ASSERTIONS
        assert(sock.stream.fd < 64); // XXX ? select() assumes socket fd values are in 0..63
#endif
        return sock.stream.fd;
      }
      case 2: { // bind
        var sock = getSocketFromFD(), info = getSocketAddress();
        sock.sock_ops.bind(sock, info.addr, info.port);
        return 0;
      }
      case 3: { // connect
        var sock = getSocketFromFD(), info = getSocketAddress();
        sock.sock_ops.connect(sock, info.addr, info.port);
        return 0;
      }
      case 4: { // listen
        var sock = getSocketFromFD(), backlog = SYSCALLS.get();
        sock.sock_ops.listen(sock, backlog);
        return 0;
      }
      case 5: { // accept
        var sock = getSocketFromFD(), addr = SYSCALLS.get(), addrlen = SYSCALLS.get();
        var newsock = sock.sock_ops.accept(sock);
        if (addr) {
          var res = __write_sockaddr(addr, newsock.family, DNS.lookup_name(newsock.daddr), newsock.dport);
#if ASSERTIONS
          assert(!res.errno);
#endif
        }
        return newsock.stream.fd;
      }
      case 6: { // getsockname
        var sock = getSocketFromFD(), addr = SYSCALLS.get(), addrlen = SYSCALLS.get();
        // TODO: sock.saddr should never be undefined, see TODO in websocket_sock_ops.getname
        var res = __write_sockaddr(addr, sock.family, DNS.lookup_name(sock.saddr || '0.0.0.0'), sock.sport);
#if ASSERTIONS
        assert(!res.errno);
#endif
        return 0;
      }
      case 7: { // getpeername
        var sock = getSocketFromFD(), addr = SYSCALLS.get(), addrlen = SYSCALLS.get();
        if (!sock.daddr) {
          return -{{{ cDefine('ENOTCONN') }}}; // The socket is not connected.
        }
        var res = __write_sockaddr(addr, sock.family, DNS.lookup_name(sock.daddr), sock.dport);
#if ASSERTIONS
        assert(!res.errno);
#endif
        return 0;
      }
      case 11: { // sendto
        var sock = getSocketFromFD(), message = SYSCALLS.get(), length = SYSCALLS.get(), flags = SYSCALLS.get(), dest = getSocketAddress(true);
        if (!dest) {
          // send, no address provided
          return FS.write(sock.stream, {{{ heapAndOffset('HEAP8', 'message') }}}, length);
        } else {
          // sendto an address
          return sock.sock_ops.sendmsg(sock, {{{ heapAndOffset('HEAP8', 'message') }}}, length, dest.addr, dest.port);
        }
      }
      case 12: { // recvfrom
        var sock = getSocketFromFD(), buf = SYSCALLS.get(), len = SYSCALLS.get(), flags = SYSCALLS.get(), addr = SYSCALLS.get(), addrlen = SYSCALLS.get();
        var msg = sock.sock_ops.recvmsg(sock, len);
        if (!msg) return 0; // socket is closed
        if (addr) {
          var res = __write_sockaddr(addr, sock.family, DNS.lookup_name(msg.addr), msg.port);
#if ASSERTIONS
          assert(!res.errno);
#endif
        }
        HEAPU8.set(msg.buffer, buf);
        return msg.buffer.byteLength;
      }
      case 14: { // setsockopt
        return -{{{ cDefine('ENOPROTOOPT') }}}; // The option is unknown at the level indicated.
      }
      case 15: { // getsockopt
        var sock = getSocketFromFD(), level = SYSCALLS.get(), optname = SYSCALLS.get(), optval = SYSCALLS.get(), optlen = SYSCALLS.get();
        // Minimal getsockopt aimed at resolving https://github.com/emscripten-core/emscripten/issues/2211
        // so only supports SOL_SOCKET with SO_ERROR.
        if (level === {{{ cDefine('SOL_SOCKET') }}}) {
          if (optname === {{{ cDefine('SO_ERROR') }}}) {
            {{{ makeSetValue('optval', 0, 'sock.error', 'i32') }}};
            {{{ makeSetValue('optlen', 0, 4, 'i32') }}};
            sock.error = null; // Clear the error (The SO_ERROR option obtains and then clears this field).
            return 0;
          }
        }
        return -{{{ cDefine('ENOPROTOOPT') }}}; // The option is unknown at the level indicated.
      }
      case 16: { // sendmsg
        var sock = getSocketFromFD(), message = SYSCALLS.get(), flags = SYSCALLS.get();
        var iov = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iov, '*') }}};
        var num = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iovlen, 'i32') }}};
        // read the address and port to send to
        var addr, port;
        var name = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_name, '*') }}};
        var namelen = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_namelen, 'i32') }}};
        if (name) {
          var info = __read_sockaddr(name, namelen);
          if (info.errno) return -info.errno;
          port = info.port;
          addr = DNS.lookup_addr(info.addr) || info.addr;
        }
        // concatenate scatter-gather arrays into one message buffer
        var total = 0;
        for (var i = 0; i < num; i++) {
          total += {{{ makeGetValue('iov', '(' + C_STRUCTS.iovec.__size__ + ' * i) + ' + C_STRUCTS.iovec.iov_len, 'i32') }}};
        }
        var view = new Uint8Array(total);
        var offset = 0;
        for (var i = 0; i < num; i++) {
          var iovbase = {{{ makeGetValue('iov', '(' + C_STRUCTS.iovec.__size__ + ' * i) + ' + C_STRUCTS.iovec.iov_base, 'i8*') }}};
          var iovlen = {{{ makeGetValue('iov', '(' + C_STRUCTS.iovec.__size__ + ' * i) + ' + C_STRUCTS.iovec.iov_len, 'i32') }}};
          for (var j = 0; j < iovlen; j++) {  
            view[offset++] = {{{ makeGetValue('iovbase', 'j', 'i8') }}};
          }
        }
        // write the buffer
        return sock.sock_ops.sendmsg(sock, view, 0, total, addr, port);
      }
      case 17: { // recvmsg
        var sock = getSocketFromFD(), message = SYSCALLS.get(), flags = SYSCALLS.get();
        var iov = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iov, 'i8*') }}};
        var num = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iovlen, 'i32') }}};
        // get the total amount of data we can read across all arrays
        var total = 0;
        for (var i = 0; i < num; i++) {
          total += {{{ makeGetValue('iov', '(' + C_STRUCTS.iovec.__size__ + ' * i) + ' + C_STRUCTS.iovec.iov_len, 'i32') }}};
        }
        // try to read total data
        var msg = sock.sock_ops.recvmsg(sock, total);
        if (!msg) return 0; // socket is closed

        // TODO honor flags:
        // MSG_OOB
        // Requests out-of-band data. The significance and semantics of out-of-band data are protocol-specific.
        // MSG_PEEK
        // Peeks at the incoming message.
        // MSG_WAITALL
        // Requests that the function block until the full amount of data requested can be returned. The function may return a smaller amount of data if a signal is caught, if the connection is terminated, if MSG_PEEK was specified, or if an error is pending for the socket.

        // write the source address out
        var name = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_name, '*') }}};
        if (name) {
          var res = __write_sockaddr(name, sock.family, DNS.lookup_name(msg.addr), msg.port);
#if ASSERTIONS
          assert(!res.errno);
#endif
        }
        // write the buffer out to the scatter-gather arrays
        var bytesRead = 0;
        var bytesRemaining = msg.buffer.byteLength;
        for (var i = 0; bytesRemaining > 0 && i < num; i++) {
          var iovbase = {{{ makeGetValue('iov', '(' + C_STRUCTS.iovec.__size__ + ' * i) + ' + C_STRUCTS.iovec.iov_base, 'i8*') }}};
          var iovlen = {{{ makeGetValue('iov', '(' + C_STRUCTS.iovec.__size__ + ' * i) + ' + C_STRUCTS.iovec.iov_len, 'i32') }}};
          if (!iovlen) {
            continue;
          }
          var length = Math.min(iovlen, bytesRemaining);
          var buf = msg.buffer.subarray(bytesRead, bytesRead + length);
          HEAPU8.set(buf, iovbase + bytesRead);
          bytesRead += length;
          bytesRemaining -= length;
        }

        // TODO set msghdr.msg_flags
        // MSG_EOR
        // End of record was received (if supported by the protocol).
        // MSG_OOB
        // Out-of-band data was received.
        // MSG_TRUNC
        // Normal data was truncated.
        // MSG_CTRUNC

        return bytesRead;
      }
      default: {
#if SYSCALL_DEBUG
        err('    (socketcall unknown call: ' + call + ')');
#endif
        return -{{{ cDefine('ENOSYS') }}}; // unsupported feature
      }
    }
  },
#endif // ~PROXY_POSIX_SOCKETS==0
  __sys_setitimer__nothrow: true,
  __sys_setitimer__proxy: false,
  __sys_setitimer: function(which, new_value, old_value) {
    return -{{{ cDefine('ENOSYS') }}}; // unsupported feature
  },
  __sys_wait4__proxy: false,
  __sys_wait4: function(pid, wstart, options, rusage) {
    abort('cannot wait on child processes');
  },
  __sys_setdomainname__nothrow: true,
  __sys_setdomainname__proxy: false,
  __sys_setdomainname: function(name, size) {
    return -{{{ cDefine('EPERM') }}};
  },
#if MINIMAL_RUNTIME
  __sys_uname__deps: ['$writeAsciiToMemory'],
#endif
  __sys_uname__proxy: false,
  __sys_uname: function(buf) {
    if (!buf) return -{{{ cDefine('EFAULT') }}}
    var layout = {{{ JSON.stringify(C_STRUCTS.utsname) }}};
    var copyString = function(element, value) {
      var offset = layout[element];
      writeAsciiToMemory(value, buf + offset);
    };
    copyString('sysname', 'Emscripten');
    copyString('nodename', 'emscripten');
    copyString('release', '1.0');
    copyString('version', '#1');
    copyString('machine', 'x86-JS');
    return 0;
  },
  __sys_mprotect__nothrow: true,
  __sys_mprotect__proxy: false,
  __sys_mprotect: function(addr, len, size) {
    return 0; // let's not and say we did
  },
  __sys_getpgid__nothrow: true,
  __sys_getpgid__proxy: false,
  __sys_getpgid: function(pid) {
    if (pid && pid !== {{{ PROCINFO.pid }}}) return -{{{ cDefine('ESRCH') }}};
    return {{{ PROCINFO.pgid }}};
  },
  __sys_fchdir: function(fd) {
    var stream = SYSCALLS.getStreamFromFD(fd);
    FS.chdir(stream.path);
    return 0;
  },
  __sys__newselect: function(nfds, readfds, writefds, exceptfds, timeout) {
    // readfds are supported,
    // writefds checks socket open status
    // exceptfds not supported
    // timeout is always 0 - fully async
#if ASSERTIONS
    assert(nfds <= 64, 'nfds must be less than or equal to 64');  // fd sets have 64 bits // TODO: this could be 1024 based on current musl headers
    assert(!exceptfds, 'exceptfds not supported');
#endif

    var total = 0;
    
    var srcReadLow = (readfds ? {{{ makeGetValue('readfds', 0, 'i32') }}} : 0),
        srcReadHigh = (readfds ? {{{ makeGetValue('readfds', 4, 'i32') }}} : 0);
    var srcWriteLow = (writefds ? {{{ makeGetValue('writefds', 0, 'i32') }}} : 0),
        srcWriteHigh = (writefds ? {{{ makeGetValue('writefds', 4, 'i32') }}} : 0);
    var srcExceptLow = (exceptfds ? {{{ makeGetValue('exceptfds', 0, 'i32') }}} : 0),
        srcExceptHigh = (exceptfds ? {{{ makeGetValue('exceptfds', 4, 'i32') }}} : 0);

    var dstReadLow = 0,
        dstReadHigh = 0;
    var dstWriteLow = 0,
        dstWriteHigh = 0;
    var dstExceptLow = 0,
        dstExceptHigh = 0;

    var allLow = (readfds ? {{{ makeGetValue('readfds', 0, 'i32') }}} : 0) |
                 (writefds ? {{{ makeGetValue('writefds', 0, 'i32') }}} : 0) |
                 (exceptfds ? {{{ makeGetValue('exceptfds', 0, 'i32') }}} : 0);
    var allHigh = (readfds ? {{{ makeGetValue('readfds', 4, 'i32') }}} : 0) |
                  (writefds ? {{{ makeGetValue('writefds', 4, 'i32') }}} : 0) |
                  (exceptfds ? {{{ makeGetValue('exceptfds', 4, 'i32') }}} : 0);

    var check = function(fd, low, high, val) {
      return (fd < 32 ? (low & val) : (high & val));
    };

    for (var fd = 0; fd < nfds; fd++) {
      var mask = 1 << (fd % 32);
      if (!(check(fd, allLow, allHigh, mask))) {
        continue;  // index isn't in the set
      }

      var stream = FS.getStream(fd);
      if (!stream) throw new FS.ErrnoError({{{ cDefine('EBADF') }}});

      var flags = SYSCALLS.DEFAULT_POLLMASK;

      if (stream.stream_ops.poll) {
        flags = stream.stream_ops.poll(stream);
      }

      if ((flags & {{{ cDefine('POLLIN') }}}) && check(fd, srcReadLow, srcReadHigh, mask)) {
        fd < 32 ? (dstReadLow = dstReadLow | mask) : (dstReadHigh = dstReadHigh | mask);
        total++;
      }
      if ((flags & {{{ cDefine('POLLOUT') }}}) && check(fd, srcWriteLow, srcWriteHigh, mask)) {
        fd < 32 ? (dstWriteLow = dstWriteLow | mask) : (dstWriteHigh = dstWriteHigh | mask);
        total++;
      }
      if ((flags & {{{ cDefine('POLLPRI') }}}) && check(fd, srcExceptLow, srcExceptHigh, mask)) {
        fd < 32 ? (dstExceptLow = dstExceptLow | mask) : (dstExceptHigh = dstExceptHigh | mask);
        total++;
      }
    }

    if (readfds) {
      {{{ makeSetValue('readfds', '0', 'dstReadLow', 'i32') }}};
      {{{ makeSetValue('readfds', '4', 'dstReadHigh', 'i32') }}};
    }
    if (writefds) {
      {{{ makeSetValue('writefds', '0', 'dstWriteLow', 'i32') }}};
      {{{ makeSetValue('writefds', '4', 'dstWriteHigh', 'i32') }}};
    }
    if (exceptfds) {
      {{{ makeSetValue('exceptfds', '0', 'dstExceptLow', 'i32') }}};
      {{{ makeSetValue('exceptfds', '4', 'dstExceptHigh', 'i32') }}};
    }
    
    return total;
  },
  __sys_msync: function(addr, len, flags) {
#if CAN_ADDRESS_2GB
    addr >>>= 0;
#endif
    var info = SYSCALLS.mappings[addr];
    if (!info) return 0;
    SYSCALLS.doMsync(addr, FS.getStream(info.fd), len, info.flags, 0);
    return 0;
  },
  __sys_getsid__nothrow: true,
  __sys_getsid__proxy: false,
  __sys_getsid: function(pid) {
    if (pid && pid !== {{{ PROCINFO.pid }}}) return -{{{ cDefine('ESRCH') }}};
    return {{{ PROCINFO.sid }}};
  },
  __sys_fdatasync: function(fd) {
    var stream = SYSCALLS.getStreamFromFD(fd);
    return 0; // we can't do anything synchronously; the in-memory FS is already synced to
  },
  __sys_mlock__nothrow: true,
  __sys_mlock__proxy: false,
  __sys_mlock__sig: 'iii',
  __sys_mlock: function(addr, len) {
    return 0;
  },
  __sys_munlock__nothrow: true,
  __sys_munlock__proxy: false,
  __sys_munlock__sig: 'iii',
  __sys_munlock: function(addr, len) {
    return 0;
  },
  __sys_mlockall__nothrow: true,
  __sys_mlockall__proxy: false,
  __sys_mlockall__sig: 'ii',
  __sys_mlockall: function(flags) {
    return 0;
  },
  __sys_munlockall__nothrow: true,
  __sys_munlockall__proxy: false,
  __sys_munlockall__sig: 'i',
  __sys_munlockall: function() {
    return 0;
  },
  __sys_mremap__nothrow: true,
  __sys_mremap__proxy: false,
  __sys_mremap: function(old_addr, old_size, new_size, flags) {
    return -{{{ cDefine('ENOMEM') }}}; // never succeed
  },
  __sys_poll: function(fds, nfds, timeout) {
    var nonzero = 0;
    for (var i = 0; i < nfds; i++) {
      var pollfd = fds + {{{ C_STRUCTS.pollfd.__size__ }}} * i;
      var fd = {{{ makeGetValue('pollfd', C_STRUCTS.pollfd.fd, 'i32') }}};
      var events = {{{ makeGetValue('pollfd', C_STRUCTS.pollfd.events, 'i16') }}};
      var mask = {{{ cDefine('POLLNVAL') }}};
      var stream = FS.getStream(fd);
      if (stream) {
        mask = SYSCALLS.DEFAULT_POLLMASK;
        if (stream.stream_ops.poll) {
          mask = stream.stream_ops.poll(stream);
        }
      }
      mask &= events | {{{ cDefine('POLLERR') }}} | {{{ cDefine('POLLHUP') }}};
      if (mask) nonzero++;
      {{{ makeSetValue('pollfd', C_STRUCTS.pollfd.revents, 'mask', 'i16') }}};
    }
    return nonzero;
  },
  __sys_rt_sigqueueinfo__nothrow: true,
  __sys_rt_sigqueueinfo__proxy: false,
  __sys_rt_sigqueueinfo: function(tgid, pid, uinfo) {
#if SYSCALL_DEBUG
    err('warning: ignoring SYS_rt_sigqueueinfo');
#endif
    return 0;
  },
  __sys_pread64: function(fd, buf, count, zero, low, high) {
    var stream = SYSCALLS.getStreamFromFD(fd)
    var offset = SYSCALLS.get64(low, high);
    return FS.read(stream, {{{ heapAndOffset('HEAP8', 'buf') }}}, count, offset);
  },
  __sys_pwrite64: function(fd, buf, count, zero, low, high) {
    var stream = SYSCALLS.getStreamFromFD(fd)
    var offset = SYSCALLS.get64(low, high);
    return FS.write(stream, {{{ heapAndOffset('HEAP8', 'buf') }}}, count, offset);
  },
  __sys_getcwd: function(buf, size) {
    if (size === 0) return -{{{ cDefine('EINVAL') }}};
    var cwd = FS.cwd();
    var cwdLengthInBytes = lengthBytesUTF8(cwd);
    if (size < cwdLengthInBytes + 1) return -{{{ cDefine('ERANGE') }}};
    stringToUTF8(cwd, buf, size);
    return buf;
  },
  __sys_ugetrlimit: function(resource, rlim) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    {{{ makeSetValue('rlim', C_STRUCTS.rlimit.rlim_cur, '-1', 'i32') }}};  // RLIM_INFINITY
    {{{ makeSetValue('rlim', C_STRUCTS.rlimit.rlim_cur + 4, '-1', 'i32') }}};  // RLIM_INFINITY
    {{{ makeSetValue('rlim', C_STRUCTS.rlimit.rlim_max, '-1', 'i32') }}};  // RLIM_INFINITY
    {{{ makeSetValue('rlim', C_STRUCTS.rlimit.rlim_max + 4, '-1', 'i32') }}};  // RLIM_INFINITY
    return 0; // just report no limits
  },
  __sys_mmap2__deps: ['$syscallMmap2'],
  __sys_mmap2: function(addr, len, prot, flags, fd, off) {
    return syscallMmap2(addr, len, prot, flags, fd, off);
  },
  __sys_truncate64: function(path, zero, low, high) {
    path = SYSCALLS.getStr(path);
    var length = SYSCALLS.get64(low, high);
    FS.truncate(path, length);
    return 0;
  },
  __sys_ftruncate64: function(fd, zero, low, high) {
    var length = SYSCALLS.get64(low, high);
    FS.ftruncate(fd, length);
    return 0;
  },
  __sys_stat64: function(path, buf) {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.doStat(FS.stat, path, buf);
  },
  __sys_lstat64: function(path, buf) {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.doStat(FS.lstat, path, buf);
  },
  __sys_fstat64: function(fd, buf) {
    var stream = SYSCALLS.getStreamFromFD(fd);
    return SYSCALLS.doStat(FS.stat, stream.path, buf);
  },
  __sys_lchown32: function(path, owner, group) {
    path = SYSCALLS.getStr(path);
    FS.chown(path, owner, group); // XXX we ignore the 'l' aspect, and do the same as chown
    return 0;
  },
  __sys_getuid32__sig: 'i',
  __sys_getuid32__nothrow: true,
  __sys_getuid32__proxy: false,
  __sys_getuid32: '__sys_getegid32',
  __sys_getgid32__sig: 'i',
  __sys_getgid32__nothrow: true,
  __sys_getgid32__proxy: false,
  __sys_getgid32: '__sys_getegid32',
  __sys_geteuid32__sig: 'i',
  __sys_geteuid32__nothrow: true,
  __sys_geteuid32__proxy: false,
  __sys_geteuid32: '__sys_getegid32',
  __sys_getegid32__nothrow: true,
  __sys_getegid32__proxy: false,
  __sys_getegid32: function() {
    return 0;
  },
  __sys_fchown32: function(fd, owner, group) {
    FS.fchown(fd, owner, group);
    return 0;
  },
  __sys_chown32: function(path, owner, group) {
    path = SYSCALLS.getStr(path);
    FS.chown(path, owner, group);
    return 0;
  },
  __sys_setreuid32__sig: 'iii',
  __sys_setreuid32__nothrow: true,
  __sys_setreuid32__proxy: false,
  __sys_setreuid32: '__sys_setregid32',
  __sys_setregid32__sig: 'iii',
  __sys_setregid32__nothrow: true,
  __sys_setregid32__proxy: false,
  __sys_setregid32: function(ruid, euid) {
    if (uid !== 0) return -{{{ cDefine('EPERM') }}};
    return 0;
  },
  __sys_setuid32__sig: 'ii',
  __sys_setuid32__nothrow: true,
  __sys_setuid32__proxy: false,
  __sys_setuid32: '__sys_setgid32',
  __sys_setgid32__sig: 'ii',
  __sys_setgid32__nothrow: true,
  __sys_setgid32__proxy: false,
  __sys_setgid32: function(uid) {
    if (uid !== 0) return -{{{ cDefine('EPERM') }}};
    return 0;
  },
  __sys_getgroups32__nothrow: true,
  __sys_getgroups32__proxy: false,
  __sys_getgroups32: function(size, list) {
    if (size < 1) return -{{{ cDefine('EINVAL') }}};
    {{{ makeSetValue('list', '0', '0', 'i32') }}};
    return 1;
  },
  __sys_setresuid32__nothrow: true,
  __sys_setresuid32__proxy: false,
  __sys_setresuid32__sig: 'iiii',
  __sys_setresuid32: '__sys_setresgid32',
  __sys_setresgid32__nothrow: true,
  __sys_setresgid32__proxy: false,
  __sys_setresgid32: function(ruid, euid, suid) {
    if (euid !== 0) return -{{{ cDefine('EPERM') }}};
    return 0;
  },
  __sys_getresuid32__sig: 'iiii',
  __sys_getresuid32__nothrow: true,
  __sys_getresuid32__proxy: false,
  __sys_getresuid32: '__sys_getresgid32',
  __sys_getresgid32__nothrow: true,
  __sys_getresgid32__proxy: false,
  __sys_getresgid32: function(ruid, euid, suid) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    {{{ makeSetValue('ruid', '0', '0', 'i32') }}};
    {{{ makeSetValue('euid', '0', '0', 'i32') }}};
    {{{ makeSetValue('suid', '0', '0', 'i32') }}};
    return 0;
  },
  __sys_mincore__nothrow: true,
  __sys_mincore__proxy: false,
  __sys_mincore: function(addr, length, vec) {
    return -{{{ cDefine('ENOSYS') }}}; // unsupported feature
  },
  __sys_madvise1__nothrow: true,
  __sys_madvise1__proxy: false,
  __sys_madvise1: function(addr, length, advice) {
    return 0; // advice is welcome, but ignored
  },
  __sys_getdents64: function(fd, dirp, count) {
    var stream = SYSCALLS.getStreamFromFD(fd)
    if (!stream.getdents) {
      stream.getdents = FS.readdir(stream.path);
    }

    var struct_size = {{{ C_STRUCTS.dirent.__size__ }}};
    var pos = 0;
    var off = FS.llseek(stream, 0, {{{ cDefine('SEEK_CUR') }}});

    var idx = Math.floor(off / struct_size);

    while (idx < stream.getdents.length && pos + struct_size <= count) {
      var id;
      var type;
      var name = stream.getdents[idx];
      if (name[0] === '.') {
        id = 1;
        type = 4; // DT_DIR
      } else {
        var child = FS.lookupNode(stream.node, name);
        id = child.id;
        type = FS.isChrdev(child.mode) ? 2 :  // DT_CHR, character device.
               FS.isDir(child.mode) ? 4 :     // DT_DIR, directory.
               FS.isLink(child.mode) ? 10 :   // DT_LNK, symbolic link.
               8;                             // DT_REG, regular file.
      }
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_ino, 'id', 'i64') }}};
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_off, '(idx + 1) * struct_size', 'i64') }}};
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_reclen, C_STRUCTS.dirent.__size__, 'i16') }}};
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_type, 'type', 'i8') }}};
      stringToUTF8(name, dirp + pos + {{{ C_STRUCTS.dirent.d_name }}}, 256);
      pos += struct_size;
      idx += 1;
    }
    FS.llseek(stream, idx * struct_size, {{{ cDefine('SEEK_SET') }}});
    return pos;
  },
  __sys_fcntl64__deps: ['$setErrNo'],
  __sys_fcntl64: function(fd, cmd, varargs) {
#if SYSCALLS_REQUIRE_FILESYSTEM == 0
#if SYSCALL_DEBUG
    err('no-op in fcntl64 syscall due to SYSCALLS_REQUIRE_FILESYSTEM=0');
#endif
    return 0;
#else
    var stream = SYSCALLS.getStreamFromFD(fd);
    switch (cmd) {
      case {{{ cDefine('F_DUPFD') }}}: {
        var arg = SYSCALLS.get();
        if (arg < 0) {
          return -{{{ cDefine('EINVAL') }}};
        }
        var newStream;
        newStream = FS.open(stream.path, stream.flags, 0, arg);
        return newStream.fd;
      }
      case {{{ cDefine('F_GETFD') }}}:
      case {{{ cDefine('F_SETFD') }}}:
        return 0;  // FD_CLOEXEC makes no sense for a single process.
      case {{{ cDefine('F_GETFL') }}}:
        return stream.flags;
      case {{{ cDefine('F_SETFL') }}}: {
        var arg = SYSCALLS.get();
        stream.flags |= arg;
        return 0;
      }
      case {{{ cDefine('F_GETLK') }}}:
      /* case {{{ cDefine('F_GETLK64') }}}: Currently in musl F_GETLK64 has same value as F_GETLK, so omitted to avoid duplicate case blocks. If that changes, uncomment this */ {
        {{{ assert(cDefine('F_GETLK') === cDefine('F_GETLK64')), '' }}}
        var arg = SYSCALLS.get();
        var offset = {{{ C_STRUCTS.flock.l_type }}};
        // We're always unlocked.
        {{{ makeSetValue('arg', 'offset', cDefine('F_UNLCK'), 'i16') }}};
        return 0;
      }
      case {{{ cDefine('F_SETLK') }}}:
      case {{{ cDefine('F_SETLKW') }}}:
      /* case {{{ cDefine('F_SETLK64') }}}: Currently in musl F_SETLK64 has same value as F_SETLK, so omitted to avoid duplicate case blocks. If that changes, uncomment this */
      /* case {{{ cDefine('F_SETLKW64') }}}: Currently in musl F_SETLKW64 has same value as F_SETLKW, so omitted to avoid duplicate case blocks. If that changes, uncomment this */
        {{{ assert(cDefine('F_SETLK64') === cDefine('F_SETLK')), '' }}}
        {{{ assert(cDefine('F_SETLKW64') === cDefine('F_SETLKW')), '' }}}
        return 0; // Pretend that the locking is successful.
      case {{{ cDefine('F_GETOWN_EX') }}}:
      case {{{ cDefine('F_SETOWN') }}}:
        return -{{{ cDefine('EINVAL') }}}; // These are for sockets. We don't have them fully implemented yet.
      case {{{ cDefine('F_GETOWN') }}}:
        // musl trusts getown return values, due to a bug where they must be, as they overlap with errors. just return -1 here, so fnctl() returns that, and we set errno ourselves.
        setErrNo({{{ cDefine('EINVAL') }}});
        return -1;
      default: {
#if SYSCALL_DEBUG
        err('warning: fctl64 unrecognized command ' + cmd);
#endif
        return -{{{ cDefine('EINVAL') }}};
      }
    }
#endif // SYSCALLS_REQUIRE_FILESYSTEM
  },

#if MINIMAL_RUNTIME
  __sys_exit_group__deps: ['$exit'],
#endif
  __sys_exit_group: function(status) {
    exit(status);
    return 0;
  },
  __sys_statfs64: function(path, size, buf) {
    path = SYSCALLS.getStr(path);
#if ASSERTIONS
    assert(size === {{{ C_STRUCTS.statfs.__size__ }}});
#endif
    // NOTE: None of the constants here are true. We're just returning safe and
    //       sane values.
    {{{ makeSetValue('buf', C_STRUCTS.statfs.f_bsize, '4096', 'i32') }}};
    {{{ makeSetValue('buf', C_STRUCTS.statfs.f_frsize, '4096', 'i32') }}};
    {{{ makeSetValue('buf', C_STRUCTS.statfs.f_blocks, '1000000', 'i32') }}};
    {{{ makeSetValue('buf', C_STRUCTS.statfs.f_bfree, '500000', 'i32') }}};
    {{{ makeSetValue('buf', C_STRUCTS.statfs.f_bavail, '500000', 'i32') }}};
    {{{ makeSetValue('buf', C_STRUCTS.statfs.f_files, 'FS.nextInode', 'i32') }}};
    {{{ makeSetValue('buf', C_STRUCTS.statfs.f_ffree, '1000000', 'i32') }}};
    {{{ makeSetValue('buf', C_STRUCTS.statfs.f_fsid, '42', 'i32') }}};
    {{{ makeSetValue('buf', C_STRUCTS.statfs.f_flags, '2', 'i32') }}};  // ST_NOSUID
    {{{ makeSetValue('buf', C_STRUCTS.statfs.f_namelen, '255', 'i32') }}};
    return 0;
  },
  __sys_fstatfs64: function(fd, size, buf) {
    var stream = SYSCALLS.getStreamFromFD(fd);
    return ___sys_statfs64(0, size, buf);
  },
  __sys_fadvise64_64__nothrow: true,
  __sys_fadvise64_64__proxy: false,
  __sys_fadvise64_64: function(fd, offset, len, advice) {
    return 0; // your advice is important to us (but we can't use it)
  },
  __sys_openat: function(dirfd, path, flags, varargs) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    var mode = SYSCALLS.get();
    return FS.open(path, flags, mode).fd;
  },
  __sys_mkdirat: function(dirfd, path, mode) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doMkdir(path, mode);
  },
  __sys_mknodat: function(dirfd, path, mode, dev) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doMknod(path, mode, dev);
  },
  __sys_fchownat: function(dirfd, path, owner, group, flags) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
#if ASSERTIONS
    assert(flags === 0);
#endif
    path = SYSCALLS.calculateAt(dirfd, path);
    FS.chown(path, owner, group);
    return 0;
  },
  __sys_fstatat64: function(dirfd, path, buf, flags) {
    path = SYSCALLS.getStr(path);
    var nofollow = flags & {{{ cDefine('AT_SYMLINK_NOFOLLOW') }}};
    flags = flags & (~{{{ cDefine('AT_SYMLINK_NOFOLLOW') }}});
#if ASSERTIONS
    assert(!flags, flags);
#endif
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doStat(nofollow ? FS.lstat : FS.stat, path, buf);
  },
  __sys_unlinkat: function(dirfd, path, flags) {
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    if (flags === 0) {
      FS.unlink(path);
    } else if (flags === {{{ cDefine('AT_REMOVEDIR') }}}) {
      FS.rmdir(path);
    } else {
      abort('Invalid flags passed to unlinkat');
    }
    return 0;
  },
  __sys_renameat: function(olddirfd, oldpath, newdirfd, newpath) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    oldpath = SYSCALLS.getStr(oldpath);
    newpath = SYSCALLS.getStr(newpath);
    oldpath = SYSCALLS.calculateAt(olddirfd, oldpath);
    newpath = SYSCALLS.calculateAt(newdirfd, newpath);
    FS.rename(oldpath, newpath);
    return 0;
  },
  __sys_linkat__nothrow: true,
  __sys_linkat__proxy: false,
  __sys_linkat: function(olddirfd, oldpath, newdirfd, newpath, flags) {
    return -{{{ cDefine('EMLINK') }}}; // no hardlinks for us
  },
  __sys_symlinkat: function(target, newdirfd, linkpath) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    linkpath = SYSCALLS.calculateAt(newdirfd, linkpath);
    FS.symlink(target, linkpath);
    return 0;
  },
  __sys_readlinkat: function(dirfd, path, buf, bufsize) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doReadlink(path, buf, bufsize);
  },
  __sys_fchmodat: function(dirfd, path, mode, varargs) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    mode = SYSCALLS.get();
    FS.chmod(path, mode);
    return 0;
  },
  __sys_faccessat: function(dirfd, path, amode, flags) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
#if ASSERTIONS
    assert(flags === 0);
#endif
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doAccess(path, amode);
  },
  __sys_pselect6__nothrow: true,
  __sys_pselect6__proxy: false,
  __sys_pselect6: function() {
    return -{{{ cDefine('ENOSYS') }}}; // unsupported feature
  },
  __sys_utimensat: function(dirfd, path, times, flags) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
#if ASSERTIONS
    assert(flags === 0);
#endif
    path = SYSCALLS.calculateAt(dirfd, path);
    var seconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_sec, 'i32') }}};
    var nanoseconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_nsec, 'i32') }}};
    var atime = (seconds*1000) + (nanoseconds/(1000*1000));
    times += {{{ C_STRUCTS.timespec.__size__ }}};
    seconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_sec, 'i32') }}};
    nanoseconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_nsec, 'i32') }}};
    var mtime = (seconds*1000) + (nanoseconds/(1000*1000));
    FS.utime(path, atime, mtime);
    return 0;  
  },
  __sys_fallocate: function(fd, mode, {{{ defineI64Param('off') }}}, {{{ defineI64Param('len') }}}) {
    {{{ receiveI64ParamAsI32s('off') }}}
    {{{ receiveI64ParamAsI32s('len') }}}
    var stream = SYSCALLS.getStreamFromFD(fd)
    var offset = SYSCALLS.get64(off_low, off_high);
    var len = SYSCALLS.get64(len_low, len_high);
#if ASSERTIONS
    assert(mode === 0);
#endif
    FS.allocate(stream, offset, len);
    return 0;
  },
  __sys_dup3: function(fd, suggestFD, flags) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    var old = SYSCALLS.getStreamFromFD(fd);
#if ASSERTIONS
    assert(!flags);
#endif
    if (old.fd === suggestFD) return -{{{ cDefine('EINVAL') }}};
    return SYSCALLS.doDup(old.path, old.flags, suggestFD);
  },
  __sys_pipe2__nothrow: true,
  __sys_pipe2__proxy: false,
  __sys_pipe2: function(fds, flags) {
    return -{{{ cDefine('ENOSYS') }}}; // unsupported feature
  },
  __sys_preadv: function(fd, iov, iovcnt, low, high) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    var stream = SYSCALLS.getStreamFromFD(fd);
    return SYSCALLS.doReadv(stream, iov, iovcnt, offset);
  },
  __sys_pwritev: function(fd, iov, iovcnt, low, high) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    var stream = SYSCALLS.getStreamFromFD(fd);
    var offset = SYSCALLS.get64(low, high);
    return SYSCALLS.doWritev(stream, iov, iovcnt, offset);
  },
  __sys_recvmmsg__nothrow: true,
  __sys_recvmmsg__proxy: false,
  __sys_recvmmsg: function(sockfd, msgvec, vlen, flags) {
#if SYSCALL_DEBUG
    err('warning: ignoring SYS_recvmmsg');
#endif
    return 0;
  },
  __sys_prlimit64: function(pid, resource, new_limit, old_limit) {
    if (old_limit) { // just report no limits
      {{{ makeSetValue('old_limit', C_STRUCTS.rlimit.rlim_cur, '-1', 'i32') }}};  // RLIM_INFINITY
      {{{ makeSetValue('old_limit', C_STRUCTS.rlimit.rlim_cur + 4, '-1', 'i32') }}};  // RLIM_INFINITY
      {{{ makeSetValue('old_limit', C_STRUCTS.rlimit.rlim_max, '-1', 'i32') }}};  // RLIM_INFINITY
      {{{ makeSetValue('old_limit', C_STRUCTS.rlimit.rlim_max + 4, '-1', 'i32') }}};  // RLIM_INFINITY
    }
    return 0;
  },
  __sys_sendmmsg__nothrow: true,
  __sys_sendmmsg__proxy: false,
  __sys_sendmmsg: function(sockfd, msg, flags) {
#if SYSCALL_DEBUG
    err('warning: ignoring SYS_sendmmsg');
#endif
    return 0;
  },

  // WASI (WebAssembly System Interface) I/O support.
  // This is the set of syscalls that use the FS etc. APIs. The rest is in
  // library_wasi.js.

#if SYSCALLS_REQUIRE_FILESYSTEM == 0 && (!MINIMAL_RUNTIME || EXIT_RUNTIME)
  $flush_NO_FILESYSTEM: function() {
    // flush anything remaining in the buffers during shutdown
    if (typeof _fflush !== 'undefined') _fflush(0);
    var buffers = SYSCALLS.buffers;
    if (buffers[1].length) SYSCALLS.printChar(1, {{{ charCode("\n") }}});
    if (buffers[2].length) SYSCALLS.printChar(2, {{{ charCode("\n") }}});
  },
  fd_write__deps: ['$flush_NO_FILESYSTEM'],
#if EXIT_RUNTIME == 1
  fd_write__postset: '__ATEXIT__.push(flush_NO_FILESYSTEM);',
#endif
#endif
  fd_write__sig: 'iiiii',
  fd_write: function(fd, iov, iovcnt, pnum) {
#if SYSCALLS_REQUIRE_FILESYSTEM
    var stream = SYSCALLS.getStreamFromFD(fd);
    var num = SYSCALLS.doWritev(stream, iov, iovcnt);
#else
    // hack to support printf in SYSCALLS_REQUIRE_FILESYSTEM=0
    var num = 0;
    for (var i = 0; i < iovcnt; i++) {
      var ptr = {{{ makeGetValue('iov', 'i*8', 'i32') }}};
      var len = {{{ makeGetValue('iov', 'i*8 + 4', 'i32') }}};
      for (var j = 0; j < len; j++) {
        SYSCALLS.printChar(fd, HEAPU8[ptr+j]);
      }
      num += len;
    }
#endif // SYSCALLS_REQUIRE_FILESYSTEM
    {{{ makeSetValue('pnum', 0, 'num', 'i32') }}}
    return 0;
  },
  fd_close__sig: 'ii',
  fd_close: function(fd) {
#if SYSCALLS_REQUIRE_FILESYSTEM
    var stream = SYSCALLS.getStreamFromFD(fd);
    FS.close(stream);
#else
#if PROXY_POSIX_SOCKETS
    // close() is a tricky function because it can be used to close both regular file descriptors
    // and POSIX network socket handles, hence an implementation would need to track for each
    // file descriptor which kind of item it is. To simplify, when using PROXY_POSIX_SOCKETS
    // option, use shutdown() to close a socket, and this function should behave like a no-op.
    warnOnce('To close sockets with PROXY_POSIX_SOCKETS bridge, prefer to use the function shutdown() that is proxied, instead of close()')
#else
#if ASSERTIONS
    abort('it should not be possible to operate on streams when !SYSCALLS_REQUIRE_FILESYSTEM');
#endif
#endif
#endif
    return 0;
  },
  fd_read__sig: 'iiiii',
  fd_read: function(fd, iov, iovcnt, pnum) {
    var stream = SYSCALLS.getStreamFromFD(fd);
    var num = SYSCALLS.doReadv(stream, iov, iovcnt);
    {{{ makeSetValue('pnum', 0, 'num', 'i32') }}}
    return 0;
  },
  fd_seek__sig: 'iiiiii',
  fd_seek: function(fd, {{{ defineI64Param('offset') }}}, whence, newOffset) {
    {{{ receiveI64ParamAsI32s('offset') }}}
    var stream = SYSCALLS.getStreamFromFD(fd);
    var HIGH_OFFSET = 0x100000000; // 2^32
    // use an unsigned operator on low and shift high by 32-bits
    var offset = offset_high * HIGH_OFFSET + (offset_low >>> 0);

    var DOUBLE_LIMIT = 0x20000000000000; // 2^53
    // we also check for equality since DOUBLE_LIMIT + 1 == DOUBLE_LIMIT
    if (offset <= -DOUBLE_LIMIT || offset >= DOUBLE_LIMIT) {
      return -{{{ cDefine('EOVERFLOW') }}};
    }

    FS.llseek(stream, offset, whence);
    {{{ makeSetValue('newOffset', '0', 'stream.position', 'i64') }}};
    if (stream.getdents && offset === 0 && whence === {{{ cDefine('SEEK_SET') }}}) stream.getdents = null; // reset readdir state
    return 0;
  },
  fd_fdstat_get__sig: 'iii',
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
#if EMTERPRETIFY_ASYNC
  fd_sync__deps: ['$EmterpreterAsync'],
#endif
  fd_sync__sig: 'ii',
  fd_sync: function(fd) {
    var stream = SYSCALLS.getStreamFromFD(fd);
#if EMTERPRETIFY_ASYNC
    return EmterpreterAsync.handle(function(resume) {
      var mount = stream.node.mount;
      if (!mount.type.syncfs) {
        // We write directly to the file system, so there's nothing to do here.
        resume(function() { return 0 });
        return;
      }
      mount.type.syncfs(mount, false, function(err) {
        if (err) {
          resume(function() { return {{{ cDefine('EIO') }}} });
          return;
        }
        resume(function() { return 0 });
      });
    });
#else
#if WASM_BACKEND && ASYNCIFY
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
      return -stream.stream_ops.fsync(stream);
    }
    return 0; // we can't do anything synchronously; the in-memory FS is already synced to
#endif // WASM_BACKEND && ASYNCIFY
#endif // EMTERPRETIFY_ASYNC
  },

#if !WASM_BACKEND
  // Fastcomp doesn't support the import_name attribute so we have to alias
  // the internal musl names to the JS names
  __syscall1__sig: 'ii',
  __syscall1: '__sys_exit',
  __syscall3__sig: 'iiii',
  __syscall3: '__sys_read',
  __syscall4__sig: 'iiii',
  __syscall4: '__sys_write',
  __syscall5__sig: 'iiii',
  __syscall5: '__sys_open',
  __syscall9__sig: 'iii',
  __syscall9: '__sys_link',
  __syscall10__sig: 'ii',
  __syscall10: '__sys_unlink',
  __syscall12__sig: 'ii',
  __syscall12: '__sys_chdir',
  __syscall14__sig: 'iiii',
  __syscall14: '__sys_mknod',
  __syscall15__sig: 'iii',
  __syscall15: '__sys_chmod',
  __syscall20__sig: 'i',
  __syscall20: '__sys_getpid',
  __syscall29__sig: 'i',
  __syscall29: '__sys_pause',
  __syscall33__sig: 'iii',
  __syscall33: '__sys_access',
  __syscall34__sig: 'ii',
  __syscall34: '__sys_nice',
  __syscall36__sig: 'i',
  __syscall36: '__sys_sync',
  __syscall38__sig: 'iii',
  __syscall38: '__sys_rename',
  __syscall39__sig: 'iii',
  __syscall39: '__sys_mkdir',
  __syscall40__sig: 'ii',
  __syscall40: '__sys_rmdir',
  __syscall41__sig: 'ii',
  __syscall41: '__sys_dup',
  __syscall42__sig: 'ii',
  __syscall42: '__sys_pipe',
  __syscall51__sig: 'ii',
  __syscall51: '__sys_acct',
  __syscall54__sig: 'iiii',
  __syscall54: '__sys_ioctl',
  __syscall57__sig: 'iii',
  __syscall57: '__sys_setpgid',
  __syscall60__sig: 'ii',
  __syscall60: '__sys_umask',
  __syscall63__sig: 'iii',
  __syscall63: '__sys_dup2',
  __syscall64__sig: 'i',
  __syscall64: '__sys_getppid',
  __syscall65__sig: 'i',
  __syscall65: '__sys_getpgrp',
  __syscall66__sig: 'i',
  __syscall66: '__sys_setsid',
  __syscall75__sig: 'iii',
  __syscall75: '__sys_setrlimit',
  __syscall77__sig: 'iii',
  __syscall77: '__sys_getrusage',
  __syscall83__sig: 'iii',
  __syscall83: '__sys_symlink',
  __syscall85__sig: 'iiii',
  __syscall85: '__sys_readlink',
  __syscall91__sig: 'iii',
  __syscall91: '__sys_munmap',
  __syscall94__sig: 'iii',
  __syscall94: '__sys_fchmod',
  __syscall96__sig: 'iii',
  __syscall96: '__sys_getpriority',
  __syscall97__sig: 'iiii',
  __syscall97: '__sys_setpriority',
  __syscall102__sig: 'iii',
  __syscall102: '__sys_socketcall',
  __syscall104__sig: 'iiii',
  __syscall104: '__sys_setitimer',
  __syscall114__sig: 'iiiii',
  __syscall114: '__sys_wait4',
  __syscall121__sig: 'iii',
  __syscall121: '__sys_setdomainname',
  __syscall122__sig: 'ii',
  __syscall122: '__sys_uname',
  __syscall125__sig: 'iiii',
  __syscall125: '__sys_mprotect',
  __syscall132__sig: 'ii',
  __syscall132: '__sys_getpgid',
  __syscall133__sig: 'ii',
  __syscall133: '__sys_fchdir',
  __syscall142__sig: 'iiiiii',
  __syscall142: '__sys__newselect',
  __syscall144__sig: 'iiii',
  __syscall144: '__sys_msync',
  __syscall147__sig: 'ii',
  __syscall147: '__sys_getsid',
  __syscall148__sig: 'ii',
  __syscall148: '__sys_fdatasync',
  __syscall150__sig: 'iii',
  __syscall150: '__sys_mlock',
  __syscall151__sig: 'iii',
  __syscall151: '__sys_munlock',
  __syscall152__sig: 'ii',
  __syscall152: '__sys_mlockall',
  __syscall153__sig: 'i',
  __syscall153: '__sys_munlockall',
  __syscall163__sig: 'iiiiii',
  __syscall163: '__sys_mremap',
  __syscall168__sig: 'iiii',
  __syscall168: '__sys_poll',
  __syscall178__sig: 'iiii',
  __syscall178: '__sys_rt_sigqueueinfo',
  __syscall180__sig: 'iiiiiii',
  __syscall180: '__sys_pread64',
  __syscall181__sig: 'iiiiiii',
  __syscall181: '__sys_pwrite64',
  __syscall183__sig: 'iii',
  __syscall183: '__sys_getcwd',
  __syscall191__sig: 'iii',
  __syscall191: '__sys_ugetrlimit',
  __syscall192__sig: 'iiiiiii',
  __syscall192: '__sys_mmap2',
  __syscall193__sig: 'iiiii',
  __syscall193: '__sys_truncate64',
  __syscall194__sig: 'iiiii',
  __syscall194: '__sys_ftruncate64',
  __syscall195__sig: 'iii',
  __syscall195: '__sys_stat64',
  __syscall196__sig: 'iii',
  __syscall196: '__sys_lstat64',
  __syscall197__sig: 'iii',
  __syscall197: '__sys_fstat64',
  __syscall198__sig: 'iiii',
  __syscall198: '__sys_lchown32',
  __syscall199__sig: 'i',
  __syscall199: '__sys_getuid32',
  __syscall200__sig: 'i',
  __syscall200: '__sys_getgid32',
  __syscall201__sig: 'i',
  __syscall201: '__sys_geteuid32',
  __syscall202__sig: 'i',
  __syscall202: '__sys_getegid32',
  __syscall203__sig: 'iii',
  __syscall203: '__sys_setreuid32',
  __syscall204__sig: 'iii',
  __syscall204: '__sys_setregid32',
  __syscall205__sig: 'iii',
  __syscall205: '__sys_getgroups32',
  __syscall207__sig: 'iiii',
  __syscall207: '__sys_fchown32',
  __syscall208__sig: 'iiii',
  __syscall208: '__sys_setresuid32',
  __syscall209__sig: 'iiii',
  __syscall209: '__sys_getresuid32',
  __syscall210__sig: 'iiii',
  __syscall210: '__sys_setresgid32',
  __syscall211__sig: 'iiii',
  __syscall211: '__sys_getresgid32',
  __syscall212__sig: 'iiii',
  __syscall212: '__sys_chown32',
  __syscall213__sig: 'ii',
  __syscall213: '__sys_setuid32',
  __syscall214__sig: 'ii',
  __syscall214: '__sys_setgid32',
  __syscall218__sig: 'iiii',
  __syscall218: '__sys_mincore',
  __syscall219__sig: 'iiii',
  __syscall219: '__sys_madvise1',
  __syscall220__sig: 'iiii',
  __syscall220: '__sys_getdents64',
  __syscall221__sig: 'iiii',
  __syscall221: '__sys_fcntl64',
  __syscall252__sig: 'ii',
  __syscall252: '__sys_exit_group',
  __syscall268__sig: 'iiii',
  __syscall268: '__sys_statfs64',
  __syscall269__sig: 'iiii',
  __syscall269: '__sys_fstatfs64',
  __syscall272__sig: 'iiiiiiii',
  __syscall272: '__sys_fadvise64_64',
  __syscall295__sig: 'iiiii',
  __syscall295: '__sys_openat',
  __syscall296__sig: 'iiii',
  __syscall296: '__sys_mkdirat',
  __syscall297__sig: 'iiiii',
  __syscall297: '__sys_mknodat',
  __syscall298__sig: 'iiiiii',
  __syscall298: '__sys_fchownat',
  __syscall300__sig: 'iiiii',
  __syscall300: '__sys_fstatat64',
  __syscall301__sig: 'iiii',
  __syscall301: '__sys_unlinkat',
  __syscall302__sig: 'iiiii',
  __syscall302: '__sys_renameat',
  __syscall303__sig: 'iiiiii',
  __syscall303: '__sys_linkat',
  __syscall304__sig: 'iiii',
  __syscall304: '__sys_symlinkat',
  __syscall305__sig: 'iiiii',
  __syscall305: '__sys_readlinkat',
  __syscall306__sig: 'iiiii',
  __syscall306: '__sys_fchmodat',
  __syscall307__sig: 'iiiii',
  __syscall307: '__sys_faccessat',
  __syscall308__sig: 'iiiiiii',
  __syscall308: '__sys_pselect6',
  __syscall320__sig: 'iiiii',
  __syscall320: '__sys_utimensat',
  __syscall324__sig: 'iiiiiii',
  __syscall324: '__sys_fallocate',
  __syscall330__sig: 'iiii',
  __syscall330: '__sys_dup3',
  __syscall331__sig: 'iii',
  __syscall331: '__sys_pipe2',
  __syscall333__sig: 'iiiiii',
  __syscall333: '__sys_preadv',
  __syscall334__sig: 'iiiiii',
  __syscall334: '__sys_pwritev',
  __syscall337__sig: 'iiiiii',
  __syscall337: '__sys_recvmmsg',
  __syscall340__sig: 'iiiii',
  __syscall340: '__sys_prlimit64',
  __syscall345__sig: 'iiiiii',
  __syscall345: '__sys_sendmmsg',
#endif
};

var WASI_SYSCALLS = set([
  'fd_write',
  'fd_close',
  'fd_read',
  'fd_seek',
  'fd_fdstat_get',
  'fd_sync',
]);

// Fallback for cases where the wasi_interface_version.name prefixing fails,
// and we have the full name from C. This happens in fastcomp which
// lacks the attribute to set the import module and base names.
if (!WASM_BACKEND) {
  for (var x in WASI_SYSCALLS) {
    SyscallsLibrary['__wasi_' + x] = x;
  }
}

for (var x in SyscallsLibrary) {
  var which = null; // if this is a musl syscall, its number
  var wasi = false;
  if (x in WASI_SYSCALLS) {
    wasi = true;
  } else {
    var match = /^__sys_[^_]*$/.exec(x);
    if (!match) {
      continue;
    }
  }
  var t = SyscallsLibrary[x];
  if (typeof t === 'string') continue;
  t = t.toString();
  // If a syscall uses FS, but !SYSCALLS_REQUIRE_FILESYSTEM, then the user
  // has disabled the filesystem or we have proven some other way that this will
  // not be called in practice, and do not need that code.
  if (!SYSCALLS_REQUIRE_FILESYSTEM && t.indexOf('FS.') >= 0) {
    t = modifyFunction(t, function(name, args, body) {
      return 'function ' + name + '(' + args + ') {\n' +
             (ASSERTIONS ? "abort('it should not be possible to operate on streams when !SYSCALLS_REQUIRE_FILESYSTEM')" : '') +
             '}';
    });
  }
  var pre = '', post = '';
  if (!wasi && t.indexOf(', varargs') != -1) {
    pre += 'SYSCALLS.varargs = varargs;\n';
  }
#if SYSCALL_DEBUG
  if (!wasi && t.indexOf(', varargs') != -1) {
    if (canThrow) {
      post += 'finally { SYSCALLS.varargs = undefined; }\n';
    } else {
      post += 'SYSCALLS.varargs = undefined;\n';
    }
  }
  pre += "err('syscall! " + x + "');\n";
  pre += "var canWarn = true;\n";
  pre += "var ret = (function() {\n";
  post += "})();\n";
  post += "if (ret < 0 && canWarn) {\n";
  post += "  err('error: syscall may have failed with ' + (-ret) + ' (' + ERRNO_MESSAGES[-ret] + ')');\n";
  post += "}\n";
  post += "err('syscall return: ' + ret);\n";
  post += "return ret;\n";
#endif
  var canThrow = SyscallsLibrary[x + '__nothrow'] !== true;
  delete SyscallsLibrary[x + '__nothrow'];
  var handler = '';
#if SYSCALLS_REQUIRE_FILESYSTEM == 0
  canThrow = false;
#endif
  if (canThrow) {
    pre += 'try {\n';
    handler +=
    "} catch (e) {\n" +
    "  if (typeof FS === 'undefined' || !(e instanceof FS.ErrnoError)) abort(e);\n";
#if SYSCALL_DEBUG
    handler +=
    "  err('error: syscall failed with ' + e.errno + ' (' + ERRNO_MESSAGES[e.errno] + ')');\n" +
    "  canWarn = false;\n";
#endif
    if (wasi) {
      handler += "  return e.errno;\n";
    } else {
      // Musl syscalls are negated.
      handler += "  return -e.errno;\n";
    }
    handler +=
    "}\n";
  }
  post = handler + post;

  if (pre) {
    var bodyStart = t.indexOf('{') + 1;
    t = t.substring(0, bodyStart) + pre + t.substring(bodyStart);
  }
  if (post) {
    var bodyEnd = t.lastIndexOf('}');
    t = t.substring(0, bodyEnd) + post + t.substring(bodyEnd);
  }
  SyscallsLibrary[x] = eval('(' + t + ')');
  if (!SyscallsLibrary[x + '__deps']) SyscallsLibrary[x + '__deps'] = [];
  SyscallsLibrary[x + '__deps'].push('$SYSCALLS');
#if USE_PTHREADS
  // Most syscalls need to happen on the main JS thread (e.g. because the
  // filesystem is in JS and on that thread). Proxy synchronously to there.
  // There are some exceptions, syscalls that we know are ok to just run in
  // any thread; those are marked as not being proxied with
  //  __proxy: false
  // A syscall without a return value could perhaps be proxied asynchronously
  // instead of synchronously, and marked with
  //  __proxy: 'async'
  // (but essentially all syscalls do have return values).
  if (SyscallsLibrary[x + '__proxy'] === undefined) {
    SyscallsLibrary[x + '__proxy'] = 'sync';
  }
#endif
}

mergeInto(LibraryManager.library, SyscallsLibrary);
