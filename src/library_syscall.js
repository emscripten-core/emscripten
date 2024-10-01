/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var SyscallsLibrary = {
  $SYSCALLS__deps: [
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
                   '$PATH',
                   '$FS',
#endif
#if SYSCALL_DEBUG
                   '$strError',
#endif
  ],
  $SYSCALLS: {
#if SYSCALLS_REQUIRE_FILESYSTEM
    // global constants
    DEFAULT_POLLMASK: {{{ cDefs.POLLIN }}} | {{{ cDefs.POLLOUT }}},

    // shared utilities
    calculateAt(dirfd, path, allowEmpty) {
      if (PATH.isAbs(path)) {
        return path;
      }
      // relative path
      var dir;
      if (dirfd === {{{ cDefs.AT_FDCWD }}}) {
        dir = FS.cwd();
      } else {
        var dirstream = SYSCALLS.getStreamFromFD(dirfd);
        dir = dirstream.path;
      }
      if (path.length == 0) {
        if (!allowEmpty) {
          throw new FS.ErrnoError({{{ cDefs.ENOENT }}});;
        }
        return dir;
      }
      return PATH.join2(dir, path);
    },

    doStat(func, path, buf) {
      var stat = func(path);
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_dev, 'stat.dev', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_mode, 'stat.mode', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_nlink, 'stat.nlink', SIZE_TYPE) }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_uid, 'stat.uid', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_gid, 'stat.gid', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_rdev, 'stat.rdev', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_size, 'stat.size', 'i64') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_blksize, '4096', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_blocks, 'stat.blocks', 'i32') }}};
      var atime = stat.atime.getTime();
      var mtime = stat.mtime.getTime();
      var ctime = stat.ctime.getTime();
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_atim.tv_sec, 'Math.floor(atime / 1000)', 'i64') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_atim.tv_nsec, '(atime % 1000) * 1000 * 1000', SIZE_TYPE) }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_mtim.tv_sec, 'Math.floor(mtime / 1000)', 'i64') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_mtim.tv_nsec, '(mtime % 1000) * 1000 * 1000', SIZE_TYPE) }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_ctim.tv_sec, 'Math.floor(ctime / 1000)', 'i64') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_ctim.tv_nsec, '(ctime % 1000) * 1000 * 1000', SIZE_TYPE) }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_ino, 'stat.ino', 'i64') }}};
      return 0;
    },
    doMsync(addr, stream, len, flags, offset) {
      if (!FS.isFile(stream.node.mode)) {
        throw new FS.ErrnoError({{{ cDefs.ENODEV }}});
      }
      if (flags & {{{ cDefs.MAP_PRIVATE }}}) {
        // MAP_PRIVATE calls need not to be synced back to underlying fs
        return 0;
      }
      var buffer = HEAPU8.slice(addr, addr + len);
      FS.msync(stream, buffer, offset, len, flags);
    },
    // Just like `FS.getStream` but will throw EBADF if stream is undefined.
    getStreamFromFD(fd) {
      var stream = FS.getStreamChecked(fd);
#if SYSCALL_DEBUG
      dbg(`    (stream: "${stream.path}")`);
#endif
      return stream;
    },
#endif // SYSCALLS_REQUIRE_FILESYSTEM

    varargs: undefined,

    getStr(ptr) {
      var ret = UTF8ToString(ptr);
#if SYSCALL_DEBUG
      dbg(`    (str: "${ret}")`);
#endif
      return ret;
    },
  },

  $syscallGetVarargI__internal: true,
  $syscallGetVarargI: function() {
#if ASSERTIONS
    assert(SYSCALLS.varargs != undefined);
#endif
    // the `+` prepended here is necessary to convince the JSCompiler that varargs is indeed a number.
    var ret = {{{ makeGetValue('+SYSCALLS.varargs', 0, 'i32') }}};
    SYSCALLS.varargs += 4;
#if SYSCALL_DEBUG
    dbg(`    (raw: "${ret}")`);
#endif
    return ret;
  },

  $syscallGetVarargP__internal: true,
#if MEMORY64
  $syscallGetVarargP: function() {
#if ASSERTIONS
    assert(SYSCALLS.varargs != undefined);
#endif
    var ret = {{{ makeGetValue('SYSCALLS.varargs', 0, '*') }}};
    SYSCALLS.varargs += {{{ POINTER_SIZE }}};
#if SYSCALL_DEBUG
    dbg(`    (raw: "${ret}")`);
#endif
    return ret;
  },
#else
  $syscallGetVarargP: '$syscallGetVarargI',
#endif

  _mmap_js__i53abi: true,
  _mmap_js__deps: ['$SYSCALLS',
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
    '$FS',
    // The dependency of FS on `mmapAlloc` and `mmapAlloc` on
    // `emscripten_builtin_memalign` are not encoding as hard dependencies,
    // so we need to explicitly depend on them here to ensure a working
    // `FS.mmap`.
    // `emscripten_builtin_memalign`).
    '$mmapAlloc',
    'emscripten_builtin_memalign',
#endif
  ],
  _mmap_js: (len, prot, flags, fd, offset, allocated, addr) => {
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
    if (isNaN(offset)) return {{{ cDefs.EOVERFLOW }}};
    var stream = SYSCALLS.getStreamFromFD(fd);
    var res = FS.mmap(stream, len, offset, prot, flags);
    var ptr = res.ptr;
    {{{ makeSetValue('allocated', 0, 'res.allocated', 'i32') }}};
    {{{ makeSetValue('addr', 0, 'ptr', '*') }}};
    return 0;
#else // no filesystem support; report lack of support
    return -{{{ cDefs.ENOSYS }}};
#endif
  },

  _munmap_js__i53abi: true,
  _munmap_js: (addr, len, prot, flags, fd, offset) => {
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
    var stream = SYSCALLS.getStreamFromFD(fd);
    if (prot & {{{ cDefs.PROT_WRITE }}}) {
      SYSCALLS.doMsync(addr, stream, len, flags, offset);
    }
#endif
  },

  __syscall_chdir: (path) => {
    path = SYSCALLS.getStr(path);
    FS.chdir(path);
    return 0;
  },
  __syscall_chmod: (path, mode) => {
    path = SYSCALLS.getStr(path);
    FS.chmod(path, mode);
    return 0;
  },
  __syscall_rmdir: (path) => {
    path = SYSCALLS.getStr(path);
    FS.rmdir(path);
    return 0;
  },
  __syscall_dup: (fd) => {
    var old = SYSCALLS.getStreamFromFD(fd);
    return FS.dupStream(old).fd;
  },
  __syscall_pipe__deps: ['$PIPEFS'],
  __syscall_pipe: (fdPtr) => {
    if (fdPtr == 0) {
      throw new FS.ErrnoError({{{ cDefs.EFAULT }}});
    }

    var res = PIPEFS.createPipe();

    {{{ makeSetValue('fdPtr', 0, 'res.readable_fd', 'i32') }}};
    {{{ makeSetValue('fdPtr', 4, 'res.writable_fd', 'i32') }}};

    return 0;
  },

#if SYSCALLS_REQUIRE_FILESYSTEM
  __syscall_ioctl__deps: ['$syscallGetVarargP'],
#endif
  __syscall_ioctl: (fd, op, varargs) => {
#if SYSCALLS_REQUIRE_FILESYSTEM == 0
#if SYSCALL_DEBUG
    dbg('no-op in ioctl syscall due to SYSCALLS_REQUIRE_FILESYSTEM=0');
#endif
    return 0;
#else
    var stream = SYSCALLS.getStreamFromFD(fd);
    switch (op) {
      case {{{ cDefs.TCGETA }}}: {
        if (!stream.tty) return -{{{ cDefs.ENOTTY }}};
#if SYSCALL_DEBUG
        dbg('warning: not filling tio struct');
#endif
        return 0;
      }
      case {{{ cDefs.TCGETS }}}: {
        if (!stream.tty) return -{{{ cDefs.ENOTTY }}};
        if (stream.tty.ops.ioctl_tcgets) {
          var termios = stream.tty.ops.ioctl_tcgets(stream);
          var argp = syscallGetVarargP();
          {{{ makeSetValue('argp', C_STRUCTS.termios.c_iflag, 'termios.c_iflag || 0', 'i32') }}};
          {{{ makeSetValue('argp', C_STRUCTS.termios.c_oflag, 'termios.c_oflag || 0', 'i32') }}};
          {{{ makeSetValue('argp', C_STRUCTS.termios.c_cflag, 'termios.c_cflag || 0', 'i32') }}};
          {{{ makeSetValue('argp', C_STRUCTS.termios.c_lflag, 'termios.c_lflag || 0', 'i32') }}};
          for (var i = 0; i < {{{ cDefs.NCCS }}}; i++) {
            {{{ makeSetValue('argp + i', C_STRUCTS.termios.c_cc, 'termios.c_cc[i] || 0', 'i8') }}};
          }
          return 0;
        }
#if SYSCALL_DEBUG
        dbg('warning: not filling tio struct');
#endif
        return 0;
      }
      case {{{ cDefs.TCSETA }}}:
      case {{{ cDefs.TCSETAW }}}:
      case {{{ cDefs.TCSETAF }}}: {
        if (!stream.tty) return -{{{ cDefs.ENOTTY }}};
        return 0; // no-op, not actually adjusting terminal settings
      }
      case {{{ cDefs.TCSETS }}}:
      case {{{ cDefs.TCSETSW }}}:
      case {{{ cDefs.TCSETSF }}}: {
        if (!stream.tty) return -{{{ cDefs.ENOTTY }}};
        if (stream.tty.ops.ioctl_tcsets) {
          var argp = syscallGetVarargP();
          var c_iflag = {{{ makeGetValue('argp', C_STRUCTS.termios.c_iflag, 'i32') }}};
          var c_oflag = {{{ makeGetValue('argp', C_STRUCTS.termios.c_oflag, 'i32') }}};
          var c_cflag = {{{ makeGetValue('argp', C_STRUCTS.termios.c_cflag, 'i32') }}};
          var c_lflag = {{{ makeGetValue('argp', C_STRUCTS.termios.c_lflag, 'i32') }}};
          var c_cc = []
          for (var i = 0; i < {{{ cDefs.NCCS }}}; i++) {
            c_cc.push({{{ makeGetValue('argp + i', C_STRUCTS.termios.c_cc, 'i8') }}});
          }
          return stream.tty.ops.ioctl_tcsets(stream.tty, op, { c_iflag, c_oflag, c_cflag, c_lflag, c_cc });
        }
        return 0; // no-op, not actually adjusting terminal settings
      }
      case {{{ cDefs.TIOCGPGRP }}}: {
        if (!stream.tty) return -{{{ cDefs.ENOTTY }}};
        var argp = syscallGetVarargP();
        {{{ makeSetValue('argp', 0, 0, 'i32') }}};
        return 0;
      }
      case {{{ cDefs.TIOCSPGRP }}}: {
        if (!stream.tty) return -{{{ cDefs.ENOTTY }}};
        return -{{{ cDefs.EINVAL }}}; // not supported
      }
      case {{{ cDefs.FIONREAD }}}: {
        var argp = syscallGetVarargP();
        return FS.ioctl(stream, op, argp);
      }
      case {{{ cDefs.TIOCGWINSZ }}}: {
        // TODO: in theory we should write to the winsize struct that gets
        // passed in, but for now musl doesn't read anything on it
        if (!stream.tty) return -{{{ cDefs.ENOTTY }}};
        if (stream.tty.ops.ioctl_tiocgwinsz) {
          var winsize = stream.tty.ops.ioctl_tiocgwinsz(stream.tty);
          var argp = syscallGetVarargP();
          {{{ makeSetValue('argp', 0, 'winsize[0]', 'i16') }}};
          {{{ makeSetValue('argp', 2, 'winsize[1]', 'i16') }}};
        }
        return 0;
      }
      case {{{ cDefs.TIOCSWINSZ }}}: {
        // TODO: technically, this ioctl call should change the window size.
        // but, since emscripten doesn't have any concept of a terminal window
        // yet, we'll just silently throw it away as we do TIOCGWINSZ
        if (!stream.tty) return -{{{ cDefs.ENOTTY }}};
        return 0;
      }
      case {{{ cDefs.TCFLSH }}}: {
        if (!stream.tty) return -{{{ cDefs.ENOTTY }}};
        return 0;
      }
      default: return -{{{ cDefs.EINVAL }}}; // not supported
    }
#endif // SYSCALLS_REQUIRE_FILESYSTEM
  },
  __syscall_symlink: (target, linkpath) => {
    target = SYSCALLS.getStr(target);
    linkpath = SYSCALLS.getStr(linkpath);
    FS.symlink(target, linkpath);
    return 0;
  },
  __syscall_fchmod: (fd, mode) => {
    FS.fchmod(fd, mode);
    return 0;
  },
// When building with PROXY_POSIX_SOCKETS the socket syscalls are implemented
// natively in libsockets.a.
// When building with WASMFS the socket syscalls are implemented natively in
// libwasmfs.a.
#if PROXY_POSIX_SOCKETS == 0 && WASMFS == 0
  $getSocketFromFD__deps: ['$SOCKFS', '$FS'],
  $getSocketFromFD: (fd) => {
    var socket = SOCKFS.getSocket(fd);
    if (!socket) throw new FS.ErrnoError({{{ cDefs.EBADF }}});
#if SYSCALL_DEBUG
    dbg(`    (socket: "${socket.path}")`);
#endif
    return socket;
  },
  $getSocketAddress__deps: ['$readSockaddr', '$FS', '$DNS'],
  $getSocketAddress: (addrp, addrlen) => {
    var info = readSockaddr(addrp, addrlen);
    if (info.errno) throw new FS.ErrnoError(info.errno);
    info.addr = DNS.lookup_addr(info.addr) || info.addr;
#if SYSCALL_DEBUG
    dbg('    (socketaddress: "' + [info.addr, info.port] + '")');
#endif
    return info;
  },
  __syscall_socket__deps: ['$SOCKFS'],
  __syscall_socket: (domain, type, protocol) => {
    var sock = SOCKFS.createSocket(domain, type, protocol);
#if ASSERTIONS
    assert(sock.stream.fd < 64); // XXX ? select() assumes socket fd values are in 0..63
#endif
    return sock.stream.fd;
  },
  __syscall_getsockname__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_getsockname: (fd, addr, addrlen, d1, d2, d3) => {
    var sock = getSocketFromFD(fd);
    // TODO: sock.saddr should never be undefined, see TODO in websocket_sock_ops.getname
    var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(sock.saddr || '0.0.0.0'), sock.sport, addrlen);
#if ASSERTIONS
    assert(!errno);
#endif
    return 0;
  },
  __syscall_getpeername__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_getpeername: (fd, addr, addrlen, d1, d2, d3) => {
    var sock = getSocketFromFD(fd);
    if (!sock.daddr) {
      return -{{{ cDefs.ENOTCONN }}}; // The socket is not connected.
    }
    var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(sock.daddr), sock.dport, addrlen);
#if ASSERTIONS
    assert(!errno);
#endif
    return 0;
  },
  __syscall_connect__deps: ['$getSocketFromFD', '$getSocketAddress'],
  __syscall_connect: (fd, addr, addrlen, d1, d2, d3) => {
    var sock = getSocketFromFD(fd);
    var info = getSocketAddress(addr, addrlen);
    sock.sock_ops.connect(sock, info.addr, info.port);
    return 0;
  },
  __syscall_shutdown__deps: ['$getSocketFromFD'],
  __syscall_shutdown: (fd, how) => {
    getSocketFromFD(fd);
    return -{{{ cDefs.ENOSYS }}}; // unsupported feature
  },
  __syscall_accept4__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_accept4: (fd, addr, addrlen, flags, d1, d2) => {
    var sock = getSocketFromFD(fd);
    var newsock = sock.sock_ops.accept(sock);
    if (addr) {
      var errno = writeSockaddr(addr, newsock.family, DNS.lookup_name(newsock.daddr), newsock.dport, addrlen);
#if ASSERTIONS
      assert(!errno);
#endif
    }
    return newsock.stream.fd;
  },
  __syscall_bind__deps: ['$getSocketFromFD', '$getSocketAddress'],
  __syscall_bind: (fd, addr, addrlen, d1, d2, d3) => {
    var sock = getSocketFromFD(fd);
    var info = getSocketAddress(addr, addrlen);
    sock.sock_ops.bind(sock, info.addr, info.port);
    return 0;
  },
  __syscall_listen__deps: ['$getSocketFromFD'],
  __syscall_listen: (fd, backlog) => {
    var sock = getSocketFromFD(fd);
    sock.sock_ops.listen(sock, backlog);
    return 0;
  },
  __syscall_recvfrom__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_recvfrom: (fd, buf, len, flags, addr, addrlen) => {
    var sock = getSocketFromFD(fd);
    var msg = sock.sock_ops.recvmsg(sock, len);
    if (!msg) return 0; // socket is closed
    if (addr) {
      var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(msg.addr), msg.port, addrlen);
#if ASSERTIONS
      assert(!errno);
#endif
    }
    HEAPU8.set(msg.buffer, buf);
    return msg.buffer.byteLength;
  },
  __syscall_sendto__deps: ['$getSocketFromFD', '$getSocketAddress'],
  __syscall_sendto: (fd, message, length, flags, addr, addr_len) => {
    var sock = getSocketFromFD(fd);
    if (!addr) {
      // send, no address provided
      return FS.write(sock.stream, HEAP8, message, length);
    }
    var dest = getSocketAddress(addr, addr_len);
    // sendto an address
    return sock.sock_ops.sendmsg(sock, HEAP8, message, length, dest.addr, dest.port);
  },
  __syscall_getsockopt__deps: ['$getSocketFromFD'],
  __syscall_getsockopt: (fd, level, optname, optval, optlen, d1) => {
    var sock = getSocketFromFD(fd);
    // Minimal getsockopt aimed at resolving https://github.com/emscripten-core/emscripten/issues/2211
    // so only supports SOL_SOCKET with SO_ERROR.
    if (level === {{{ cDefs.SOL_SOCKET }}}) {
      if (optname === {{{ cDefs.SO_ERROR }}}) {
        {{{ makeSetValue('optval', 0, 'sock.error', 'i32') }}};
        {{{ makeSetValue('optlen', 0, 4, 'i32') }}};
        sock.error = null; // Clear the error (The SO_ERROR option obtains and then clears this field).
        return 0;
      }
    }
    return -{{{ cDefs.ENOPROTOOPT }}}; // The option is unknown at the level indicated.
  },
  __syscall_sendmsg__deps: ['$getSocketFromFD', '$getSocketAddress', '$DNS'],
  __syscall_sendmsg: (fd, message, flags, d1, d2, d3) => {
    var sock = getSocketFromFD(fd);
    var iov = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iov, '*') }}};
    var num = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iovlen, 'i32') }}};
    // read the address and port to send to
    var addr, port;
    var name = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_name, '*') }}};
    var namelen = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_namelen, 'i32') }}};
    if (name) {
      var info = getSocketAddress(name, namelen);
      port = info.port;
      addr = info.addr;
    }
    // concatenate scatter-gather arrays into one message buffer
    var total = 0;
    for (var i = 0; i < num; i++) {
      total += {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
    }
    var view = new Uint8Array(total);
    var offset = 0;
    for (var i = 0; i < num; i++) {
      var iovbase = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_base}`, POINTER_TYPE) }}};
      var iovlen = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
      for (var j = 0; j < iovlen; j++) {
        view[offset++] = {{{ makeGetValue('iovbase', 'j', 'i8') }}};
      }
    }
    // write the buffer
    return sock.sock_ops.sendmsg(sock, view, 0, total, addr, port);
  },
  __syscall_recvmsg__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_recvmsg: (fd, message, flags, d1, d2, d3) => {
    var sock = getSocketFromFD(fd);
    var iov = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iov, POINTER_TYPE) }}};
    var num = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iovlen, 'i32') }}};
    // get the total amount of data we can read across all arrays
    var total = 0;
    for (var i = 0; i < num; i++) {
      total += {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
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
      var errno = writeSockaddr(name, sock.family, DNS.lookup_name(msg.addr), msg.port);
#if ASSERTIONS
      assert(!errno);
#endif
    }
    // write the buffer out to the scatter-gather arrays
    var bytesRead = 0;
    var bytesRemaining = msg.buffer.byteLength;
    for (var i = 0; bytesRemaining > 0 && i < num; i++) {
      var iovbase = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_base}`, POINTER_TYPE) }}};
      var iovlen = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
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
  },
#endif // ~PROXY_POSIX_SOCKETS==0
  __syscall_fchdir: (fd) => {
    var stream = SYSCALLS.getStreamFromFD(fd);
    FS.chdir(stream.path);
    return 0;
  },
  __syscall__newselect: (nfds, readfds, writefds, exceptfds, timeout) => {
    // readfds are supported,
    // writefds checks socket open status
    // exceptfds are supported, although on web, such exceptional conditions never arise in web sockets
    //                          and so the exceptfds list will always return empty.
    // timeout is supported, although on SOCKFS and PIPEFS these are ignored and always treated as 0 - fully async
#if ASSERTIONS
    assert(nfds <= 64, 'nfds must be less than or equal to 64');  // fd sets have 64 bits // TODO: this could be 1024 based on current musl headers
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

      var stream = SYSCALLS.getStreamFromFD(fd);

      var flags = SYSCALLS.DEFAULT_POLLMASK;

      if (stream.stream_ops.poll) {
        var timeoutInMillis = -1;
        if (timeout) {
          // select(2) is declared to accept "struct timeval { time_t tv_sec; suseconds_t tv_usec; }".
          // However, musl passes the two values to the syscall as an array of long values.
          // Note that sizeof(time_t) != sizeof(long) in wasm32. The former is 8, while the latter is 4.
          // This means using "C_STRUCTS.timeval.tv_usec" leads to a wrong offset.
          // So, instead, we use POINTER_SIZE.
          var tv_sec = (readfds ? {{{ makeGetValue('timeout', 0, 'i32') }}} : 0),
              tv_usec = (readfds ? {{{ makeGetValue('timeout', POINTER_SIZE, 'i32') }}} : 0);
          timeoutInMillis = (tv_sec + tv_usec / 1000000) * 1000;
        }
        flags = stream.stream_ops.poll(stream, timeoutInMillis);
      }

      if ((flags & {{{ cDefs.POLLIN }}}) && check(fd, srcReadLow, srcReadHigh, mask)) {
        fd < 32 ? (dstReadLow = dstReadLow | mask) : (dstReadHigh = dstReadHigh | mask);
        total++;
      }
      if ((flags & {{{ cDefs.POLLOUT }}}) && check(fd, srcWriteLow, srcWriteHigh, mask)) {
        fd < 32 ? (dstWriteLow = dstWriteLow | mask) : (dstWriteHigh = dstWriteHigh | mask);
        total++;
      }
      if ((flags & {{{ cDefs.POLLPRI }}}) && check(fd, srcExceptLow, srcExceptHigh, mask)) {
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
  _msync_js__i53abi: true,
  _msync_js: (addr, len, prot, flags, fd, offset) => {
    if (isNaN(offset)) return {{{ cDefs.EOVERFLOW }}};
    SYSCALLS.doMsync(addr, SYSCALLS.getStreamFromFD(fd), len, flags, offset);
    return 0;
  },
  __syscall_fdatasync: (fd) => {
    var stream = SYSCALLS.getStreamFromFD(fd);
    return 0; // we can't do anything synchronously; the in-memory FS is already synced to
  },
  __syscall_poll: (fds, nfds, timeout) => {
    var nonzero = 0;
    for (var i = 0; i < nfds; i++) {
      var pollfd = fds + {{{ C_STRUCTS.pollfd.__size__ }}} * i;
      var fd = {{{ makeGetValue('pollfd', C_STRUCTS.pollfd.fd, 'i32') }}};
      var events = {{{ makeGetValue('pollfd', C_STRUCTS.pollfd.events, 'i16') }}};
      var mask = {{{ cDefs.POLLNVAL }}};
      var stream = FS.getStream(fd);
      if (stream) {
        mask = SYSCALLS.DEFAULT_POLLMASK;
        if (stream.stream_ops.poll) {
          mask = stream.stream_ops.poll(stream, -1);
        }
      }
      mask &= events | {{{ cDefs.POLLERR }}} | {{{ cDefs.POLLHUP }}};
      if (mask) nonzero++;
      {{{ makeSetValue('pollfd', C_STRUCTS.pollfd.revents, 'mask', 'i16') }}};
    }
    return nonzero;
  },
  __syscall_getcwd__deps: ['$lengthBytesUTF8', '$stringToUTF8'],
  __syscall_getcwd: (buf, size) => {
    if (size === 0) return -{{{ cDefs.EINVAL }}};
    var cwd = FS.cwd();
    var cwdLengthInBytes = lengthBytesUTF8(cwd) + 1;
    if (size < cwdLengthInBytes) return -{{{ cDefs.ERANGE }}};
    stringToUTF8(cwd, buf, size);
    return cwdLengthInBytes;
  },
  __syscall_truncate64__i53abi: true,
  __syscall_truncate64: (path, length) => {
    if (isNaN(length)) return {{{ cDefs.EOVERFLOW }}};
    path = SYSCALLS.getStr(path);
    FS.truncate(path, length);
    return 0;
  },
  __syscall_ftruncate64__i53abi: true,
  __syscall_ftruncate64: (fd, length) => {
    if (isNaN(length)) return {{{ cDefs.EOVERFLOW }}};
    FS.ftruncate(fd, length);
    return 0;
  },
  __syscall_stat64: (path, buf) => {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.doStat(FS.stat, path, buf);
  },
  __syscall_lstat64: (path, buf) => {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.doStat(FS.lstat, path, buf);
  },
  __syscall_fstat64: (fd, buf) => {
    var stream = SYSCALLS.getStreamFromFD(fd);
    return SYSCALLS.doStat(FS.stat, stream.path, buf);
  },
  __syscall_fchown32: (fd, owner, group) => {
    FS.fchown(fd, owner, group);
    return 0;
  },
  __syscall_getdents64__deps: ['$stringToUTF8'],
  __syscall_getdents64: (fd, dirp, count) => {
    var stream = SYSCALLS.getStreamFromFD(fd)
    stream.getdents ||= FS.readdir(stream.path);

    var struct_size = {{{ C_STRUCTS.dirent.__size__ }}};
    var pos = 0;
    var off = FS.llseek(stream, 0, {{{ cDefs.SEEK_CUR }}});

    var idx = Math.floor(off / struct_size);

    while (idx < stream.getdents.length && pos + struct_size <= count) {
      var id;
      var type;
      var name = stream.getdents[idx];
      if (name === '.') {
        id = stream.node.id;
        type = 4; // DT_DIR
      }
      else if (name === '..') {
        var lookup = FS.lookupPath(stream.path, { parent: true });
        id = lookup.node.id;
        type = 4; // DT_DIR
      }
      else {
        var child = FS.lookupNode(stream.node, name);
        id = child.id;
        type = FS.isChrdev(child.mode) ? 2 :  // DT_CHR, character device.
               FS.isDir(child.mode) ? 4 :     // DT_DIR, directory.
               FS.isLink(child.mode) ? 10 :   // DT_LNK, symbolic link.
               8;                             // DT_REG, regular file.
      }
#if ASSERTIONS
      assert(id);
#endif
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_ino, 'id', 'i64') }}};
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_off, '(idx + 1) * struct_size', 'i64') }}};
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_reclen, C_STRUCTS.dirent.__size__, 'i16') }}};
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_type, 'type', 'i8') }}};
      stringToUTF8(name, dirp + pos + {{{ C_STRUCTS.dirent.d_name }}}, 256);
      pos += struct_size;
      idx += 1;
    }
    FS.llseek(stream, idx * struct_size, {{{ cDefs.SEEK_SET }}});
    return pos;
  },
#if SYSCALLS_REQUIRE_FILESYSTEM
  __syscall_fcntl64__deps: ['$syscallGetVarargP', '$syscallGetVarargI'],
#endif
  __syscall_fcntl64: (fd, cmd, varargs) => {
#if SYSCALLS_REQUIRE_FILESYSTEM == 0
#if SYSCALL_DEBUG
    dbg('no-op in fcntl syscall due to SYSCALLS_REQUIRE_FILESYSTEM=0');
#endif
    return 0;
#else
    var stream = SYSCALLS.getStreamFromFD(fd);
    switch (cmd) {
      case {{{ cDefs.F_DUPFD }}}: {
        var arg = syscallGetVarargI();
        if (arg < 0) {
          return -{{{ cDefs.EINVAL }}};
        }
        while (FS.streams[arg]) {
          arg++;
        }
        var newStream;
        newStream = FS.dupStream(stream, arg);
        return newStream.fd;
      }
      case {{{ cDefs.F_GETFD }}}:
      case {{{ cDefs.F_SETFD }}}:
        return 0;  // FD_CLOEXEC makes no sense for a single process.
      case {{{ cDefs.F_GETFL }}}:
        return stream.flags;
      case {{{ cDefs.F_SETFL }}}: {
        var arg = syscallGetVarargI();
        stream.flags |= arg;
        return 0;
      }
      case {{{ cDefs.F_GETLK }}}: {
        var arg = syscallGetVarargP();
        var offset = {{{ C_STRUCTS.flock.l_type }}};
        // We're always unlocked.
        {{{ makeSetValue('arg', 'offset', cDefs.F_UNLCK, 'i16') }}};
        return 0;
      }
      case {{{ cDefs.F_SETLK }}}:
      case {{{ cDefs.F_SETLKW }}}:
        return 0; // Pretend that the locking is successful.
#if SYSCALL_DEBUG
      case {{{ cDefs.F_GETOWN_EX }}}:
      case {{{ cDefs.F_SETOWN }}}:
      case {{{ cDefs.F_GETOWN }}}:
        return -{{{ cDefs.EINVAL }}};
      default:
        dbg(`warning: fcntl unrecognized command ${cmd}`);
#endif
    }
    return -{{{ cDefs.EINVAL }}};
#endif // SYSCALLS_REQUIRE_FILESYSTEM
  },

  __syscall_statfs64: (path, size, buf) => {
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
  __syscall_fstatfs64__deps: ['__syscall_statfs64'],
  __syscall_fstatfs64: (fd, size, buf) => {
    var stream = SYSCALLS.getStreamFromFD(fd);
    return ___syscall_statfs64(0, size, buf);
  },
  __syscall_fadvise64__nothrow: true,
  __syscall_fadvise64__proxy: 'none',
  __syscall_fadvise64: (fd, offset, len, advice) => {
    return 0; // your advice is important to us (but we can't use it)
  },
  __syscall_openat__deps: ['$syscallGetVarargI'],
  __syscall_openat: (dirfd, path, flags, varargs) => {
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    var mode = varargs ? syscallGetVarargI() : 0;
    return FS.open(path, flags, mode).fd;
  },
  __syscall_mkdirat: (dirfd, path, mode) => {
#if SYSCALL_DEBUG
    dbg('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    // remove a trailing slash, if one - /a/b/ has basename of '', but
    // we want to create b in the context of this function
    path = PATH.normalize(path);
    if (path[path.length-1] === '/') path = path.substr(0, path.length-1);
    FS.mkdir(path, mode, 0);
    return 0;
  },
  __syscall_mknodat: (dirfd, path, mode, dev) => {
#if SYSCALL_DEBUG
    dbg('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    // we don't want this in the JS API as it uses mknod to create all nodes.
    switch (mode & {{{ cDefs.S_IFMT }}}) {
      case {{{ cDefs.S_IFREG }}}:
      case {{{ cDefs.S_IFCHR }}}:
      case {{{ cDefs.S_IFBLK }}}:
      case {{{ cDefs.S_IFIFO }}}:
      case {{{ cDefs.S_IFSOCK }}}:
        break;
      default: return -{{{ cDefs.EINVAL }}};
    }
    FS.mknod(path, mode, dev);
    return 0;
  },
  __syscall_fchownat: (dirfd, path, owner, group, flags) => {
#if SYSCALL_DEBUG
    dbg('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
    var nofollow = flags & {{{ cDefs.AT_SYMLINK_NOFOLLOW }}};
    flags = flags & (~{{{ cDefs.AT_SYMLINK_NOFOLLOW }}});
#if ASSERTIONS
    assert(flags === 0);
#endif
    path = SYSCALLS.calculateAt(dirfd, path);
    (nofollow ? FS.lchown : FS.chown)(path, owner, group);
    return 0;
  },
  __syscall_newfstatat: (dirfd, path, buf, flags) => {
    path = SYSCALLS.getStr(path);
    var nofollow = flags & {{{ cDefs.AT_SYMLINK_NOFOLLOW }}};
    var allowEmpty = flags & {{{ cDefs.AT_EMPTY_PATH }}};
    flags = flags & (~{{{ cDefs.AT_SYMLINK_NOFOLLOW | cDefs.AT_EMPTY_PATH | cDefs.AT_NO_AUTOMOUNT }}});
#if ASSERTIONS
    assert(!flags, `unknown flags in __syscall_newfstatat: ${flags}`);
#endif
    path = SYSCALLS.calculateAt(dirfd, path, allowEmpty);
    return SYSCALLS.doStat(nofollow ? FS.lstat : FS.stat, path, buf);
  },
  __syscall_unlinkat: (dirfd, path, flags) => {
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    if (flags === 0) {
      FS.unlink(path);
    } else if (flags === {{{ cDefs.AT_REMOVEDIR }}}) {
      FS.rmdir(path);
    } else {
      abort('Invalid flags passed to unlinkat');
    }
    return 0;
  },
  __syscall_renameat: (olddirfd, oldpath, newdirfd, newpath) => {
    oldpath = SYSCALLS.getStr(oldpath);
    newpath = SYSCALLS.getStr(newpath);
    oldpath = SYSCALLS.calculateAt(olddirfd, oldpath);
    newpath = SYSCALLS.calculateAt(newdirfd, newpath);
    FS.rename(oldpath, newpath);
    return 0;
  },
  __syscall_symlinkat: (target, newdirfd, linkpath) => {
#if SYSCALL_DEBUG
    dbg('warning: untested syscall');
#endif
    linkpath = SYSCALLS.calculateAt(newdirfd, linkpath);
    FS.symlink(target, linkpath);
    return 0;
  },
  __syscall_readlinkat__deps: ['$lengthBytesUTF8', '$stringToUTF8'],
  __syscall_readlinkat: (dirfd, path, buf, bufsize) => {
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    if (bufsize <= 0) return -{{{ cDefs.EINVAL }}};
    var ret = FS.readlink(path);

    var len = Math.min(bufsize, lengthBytesUTF8(ret));
    var endChar = HEAP8[buf+len];
    stringToUTF8(ret, buf, bufsize+1);
    // readlink is one of the rare functions that write out a C string, but does never append a null to the output buffer(!)
    // stringToUTF8() always appends a null byte, so restore the character under the null byte after the write.
    HEAP8[buf+len] = endChar;
    return len;
  },
  __syscall_fchmodat2: (dirfd, path, mode, flags) => {
    var nofollow = flags & {{{ cDefs.AT_SYMLINK_NOFOLLOW }}};
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    FS.chmod(path, mode, nofollow);
    return 0;
  },
  __syscall_faccessat: (dirfd, path, amode, flags) => {
#if SYSCALL_DEBUG
    dbg('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
#if ASSERTIONS
    assert(flags === 0 || flags == {{{ cDefs.AT_EACCESS }}});
#endif
    path = SYSCALLS.calculateAt(dirfd, path);
    if (amode & ~{{{ cDefs.S_IRWXO }}}) {
      // need a valid mode
      return -{{{ cDefs.EINVAL }}};
    }
    var lookup = FS.lookupPath(path, { follow: true });
    var node = lookup.node;
    if (!node) {
      return -{{{ cDefs.ENOENT }}};
    }
    var perms = '';
    if (amode & {{{ cDefs.R_OK }}}) perms += 'r';
    if (amode & {{{ cDefs.W_OK }}}) perms += 'w';
    if (amode & {{{ cDefs.X_OK }}}) perms += 'x';
    if (perms /* otherwise, they've just passed F_OK */ && FS.nodePermissions(node, perms)) {
      return -{{{ cDefs.EACCES }}};
    }
    return 0;
  },
  __syscall_utimensat__deps: ['$readI53FromI64'],
  __syscall_utimensat: (dirfd, path, times, flags) => {
    path = SYSCALLS.getStr(path);
#if ASSERTIONS
    assert(flags === 0);
#endif
    path = SYSCALLS.calculateAt(dirfd, path, true);
    var now = Date.now(), atime, mtime;
    if (!times) {
      atime = now;
      mtime = now;
    } else {
      var seconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_sec, 'i53') }}};
      var nanoseconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_nsec, 'i32') }}};
      if (nanoseconds == {{{ cDefs.UTIME_NOW }}}) {
        atime = now;
      } else if (nanoseconds == {{{ cDefs.UTIME_OMIT }}}) {
        atime = -1;
      } else {
        atime = (seconds*1000) + (nanoseconds/(1000*1000));
      }
      times += {{{ C_STRUCTS.timespec.__size__ }}};
      seconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_sec, 'i53') }}};
      nanoseconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_nsec, 'i32') }}};
      if (nanoseconds == {{{ cDefs.UTIME_NOW }}}) {
        mtime = now;
      } else if (nanoseconds == {{{ cDefs.UTIME_OMIT }}}) {
        mtime = -1;
      } else {
        mtime = (seconds*1000) + (nanoseconds/(1000*1000));
      }
    }
    // -1 here means UTIME_OMIT was passed.  FS.utime tables the max of these
    // two values and sets the timestamp to that single value.  If both were
    // set to UTIME_OMIT then we can skip the call completely.
    if (mtime != -1 || atime != -1) {
      FS.utime(path, atime, mtime);
    }
    return 0;
  },
  __syscall_fallocate__i53abi: true,
  __syscall_fallocate: (fd, mode, offset, len) => {
    if (isNaN(offset)) return {{{ cDefs.EOVERFLOW }}};
    var stream = SYSCALLS.getStreamFromFD(fd)
#if ASSERTIONS
    assert(mode === 0);
#endif
    FS.allocate(stream, offset, len);
    return 0;
  },
  __syscall_dup3: (fd, newfd, flags) => {
    var old = SYSCALLS.getStreamFromFD(fd);
#if ASSERTIONS
    assert(!flags);
#endif
    if (old.fd === newfd) return -{{{ cDefs.EINVAL }}};
    // Check newfd is within range of valid open file descriptors.
    if (newfd < 0 || newfd >= FS.MAX_OPEN_FDS) return -{{{ cDefs.EBADF }}};
    var existing = FS.getStream(newfd);
    if (existing) FS.close(existing);
    return FS.dupStream(old, newfd).fd;
  },
};

for (var x in SyscallsLibrary) {
  wrapSyscallFunction(x, SyscallsLibrary, false);
}

addToLibrary(SyscallsLibrary);
