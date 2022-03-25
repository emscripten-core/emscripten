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
                   '$ERRNO_MESSAGES'
#endif
  ],
  $SYSCALLS: {
#if SYSCALLS_REQUIRE_FILESYSTEM
    // global constants
    DEFAULT_POLLMASK: {{{ cDefine('POLLIN') }}} | {{{ cDefine('POLLOUT') }}},

    // shared utilities
    calculateAt: function(dirfd, path, allowEmpty) {
      if (path[0] === '/') {
        return path;
      }
      // relative path
      var dir;
      if (dirfd === {{{ cDefine('AT_FDCWD') }}}) {
        dir = FS.cwd();
      } else {
        var dirstream = FS.getStream(dirfd);
        if (!dirstream) throw new FS.ErrnoError({{{ cDefine('EBADF') }}});
        dir = dirstream.path;
      }
      if (path.length == 0) {
        if (!allowEmpty) {
          throw new FS.ErrnoError({{{ cDefine('ENOENT') }}});;
        }
        return dir;
      }
      return PATH.join2(dir, path);
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
      var lookup = FS.lookupPath(path, { follow: true });
      var node = lookup.node;
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
    doReadv: function(stream, iov, iovcnt, offset) {
      var ret = 0;
      for (var i = 0; i < iovcnt; i++) {
        var ptr = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_base, '*') }}};
        var len = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_len, '*') }}};
        iov += {{{ C_STRUCTS.iovec.__size__ }}};
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
        var ptr = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_base, '*') }}};
        var len = {{{ makeGetValue('iov', C_STRUCTS.iovec.iov_len, '*') }}};
        iov += {{{ C_STRUCTS.iovec.__size__ }}};
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
  },

  _mmap_js__deps: ['$SYSCALLS',
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
    '$FS',
#endif
  ],
  _mmap_js: function(addr, len, prot, flags, fd, off, allocated, builtin) {
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
    var info = FS.getStream(fd);
    if (!info) return -{{{ cDefine('EBADF') }}};
    var res = FS.mmap(info, addr, len, off, prot, flags);
    var ptr = res.ptr;
    {{{ makeSetValue('allocated', 0, 'res.allocated', 'i32') }}};
#if CAN_ADDRESS_2GB
    ptr >>>= 0;
#endif
    return ptr;
#else // no filesystem support; report lack of support
    return -{{{ cDefine('ENOSYS') }}};
#endif
  },

  _munmap_js__deps: ['$SYSCALLS',
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
    '$FS',
#endif
  ],
  _munmap_js: function(addr, len, prot, flags, fd, offset) {
#if CAN_ADDRESS_2GB
    addr >>>= 0;
#endif
#if FILESYSTEM && SYSCALLS_REQUIRE_FILESYSTEM
    var stream = FS.getStream(fd);
    if (stream) {
      if (prot & {{{ cDefine('PROT_WRITE') }}}) {
        SYSCALLS.doMsync(addr, stream, len, flags, offset);
      }
      FS.munmap(stream);
    }
#endif
  },

  __syscall_chdir: function(path) {
    path = SYSCALLS.getStr(path);
    FS.chdir(path);
    return 0;
  },
  __syscall_mknod: function(path, mode, dev) {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.doMknod(path, mode, dev);
  },
  __syscall_chmod: function(path, mode) {
    path = SYSCALLS.getStr(path);
    FS.chmod(path, mode);
    return 0;
  },
  __syscall_rename: function(old_path, new_path) {
    old_path = SYSCALLS.getStr(old_path);
    new_path = SYSCALLS.getStr(new_path);
    FS.rename(old_path, new_path);
    return 0;
  },
  __syscall_mkdir: function(path, mode) {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.doMkdir(path, mode);
  },
  __syscall_rmdir: function(path) {
    path = SYSCALLS.getStr(path);
    FS.rmdir(path);
    return 0;
  },
  __syscall_dup: function(fd) {
    var old = SYSCALLS.getStreamFromFD(fd);
    return FS.open(old.path, old.flags, 0).fd;
  },
  __syscall_pipe__deps: ['$PIPEFS'],
  __syscall_pipe: function(fdPtr) {
    if (fdPtr == 0) {
      throw new FS.ErrnoError({{{ cDefine('EFAULT') }}});
    }

    var res = PIPEFS.createPipe();

    {{{ makeSetValue('fdPtr', 0, 'res.readable_fd', 'i32') }}};
    {{{ makeSetValue('fdPtr', 4, 'res.writable_fd', 'i32') }}};

    return 0;
  },
  __syscall_ioctl: function(fd, op, varargs) {
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
  __syscall_symlink: function(target, linkpath) {
    target = SYSCALLS.getStr(target);
    linkpath = SYSCALLS.getStr(linkpath);
    FS.symlink(target, linkpath);
    return 0;
  },
  __syscall_fchmod: function(fd, mode) {
    FS.fchmod(fd, mode);
    return 0;
  },
#if PROXY_POSIX_SOCKETS == 0
  $getSocketFromFD__deps: ['$SOCKFS', '$FS'],
  $getSocketFromFD: function(fd) {
    var socket = SOCKFS.getSocket(fd);
    if (!socket) throw new FS.ErrnoError({{{ cDefine('EBADF') }}});
#if SYSCALL_DEBUG
    err('    (socket: "' + socket.path + '")');
#endif
    return socket;
  },
  /** @param {boolean=} allowNull */
  $getSocketAddress__deps: ['$readSockaddr', '$FS', '$DNS'],
  $getSocketAddress__docs: '/** @param {boolean=} allowNull */',
  $getSocketAddress: function(addrp, addrlen, allowNull) {
    if (allowNull && addrp === 0) return null;
    var info = readSockaddr(addrp, addrlen);
    if (info.errno) throw new FS.ErrnoError(info.errno);
    info.addr = DNS.lookup_addr(info.addr) || info.addr;
#if SYSCALL_DEBUG
    err('    (socketaddress: "' + [info.addr, info.port] + '")');
#endif
    return info;
  },
  __syscall_socket__deps: ['$SOCKFS'],
  __syscall_socket: function(domain, type, protocol) {
    var sock = SOCKFS.createSocket(domain, type, protocol);
#if ASSERTIONS
    assert(sock.stream.fd < 64); // XXX ? select() assumes socket fd values are in 0..63
#endif
    return sock.stream.fd;
  },
  __syscall_getsockname__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_getsockname: function(fd, addr, addrlen) {
    err("__syscall_getsockname " + fd);
    var sock = getSocketFromFD(fd);
    // TODO: sock.saddr should never be undefined, see TODO in websocket_sock_ops.getname
    var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(sock.saddr || '0.0.0.0'), sock.sport, addrlen);
#if ASSERTIONS
    assert(!errno);
#endif
    return 0;
  },
  __syscall_getpeername__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_getpeername: function(fd, addr, addrlen) {
    var sock = getSocketFromFD(fd);
    if (!sock.daddr) {
      return -{{{ cDefine('ENOTCONN') }}}; // The socket is not connected.
    }
    var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(sock.daddr), sock.dport, addrlen);
#if ASSERTIONS
    assert(!errno);
#endif
    return 0;
  },
  __syscall_connect__deps: ['$getSocketFromFD', '$getSocketAddress'],
  __syscall_connect: function(fd, addr, addrlen) {
    var sock = getSocketFromFD(fd);
    var info = getSocketAddress(addr, addrlen);
    sock.sock_ops.connect(sock, info.addr, info.port);
    return 0;
  },
  __syscall_shutdown__deps: ['$getSocketFromFD'],
  __syscall_shutdown: function(fd, how) {
    getSocketFromFD(fd);
    return -{{{ cDefine('ENOSYS') }}}; // unsupported feature
  },
  __syscall_accept4__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_accept4: function(fd, addr, addrlen, flags) {
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
  __syscall_bind: function(fd, addr, addrlen) {
    var sock = getSocketFromFD(fd);
    var info = getSocketAddress(addr, addrlen);
    sock.sock_ops.bind(sock, info.addr, info.port);
    return 0;
  },
  __syscall_listen__deps: ['$getSocketFromFD'],
  __syscall_listen: function(fd, backlog) {
    var sock = getSocketFromFD(fd);
    sock.sock_ops.listen(sock, backlog);
    return 0;
  },
  __syscall_recvfrom__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_recvfrom: function(fd, buf, len, flags, addr, addrlen) {
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
  __syscall_sendto: function(fd, message, length, flags, addr, addr_len) {
    var sock = getSocketFromFD(fd);
    var dest = getSocketAddress(addr, addr_len, true);
    if (!dest) {
      // send, no address provided
      return FS.write(sock.stream, {{{ heapAndOffset('HEAP8', 'message') }}}, length);
    } else {
      // sendto an address
      return sock.sock_ops.sendmsg(sock, {{{ heapAndOffset('HEAP8', 'message') }}}, length, dest.addr, dest.port);
    }
  },
  __syscall_getsockopt__deps: ['$getSocketFromFD'],
  __syscall_getsockopt: function(fd, level, optname, optval, optlen) {
    var sock = getSocketFromFD(fd);
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
  },
  __syscall_sendmsg__deps: ['$getSocketFromFD', '$readSockaddr', '$DNS'],
  __syscall_sendmsg: function(fd, message, flags) {
    var sock = getSocketFromFD(fd);
    var iov = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iov, '*') }}};
    var num = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iovlen, 'i32') }}};
    // read the address and port to send to
    var addr, port;
    var name = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_name, '*') }}};
    var namelen = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_namelen, 'i32') }}};
    if (name) {
      var info = readSockaddr(name, namelen);
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
  },
  __syscall_recvmsg__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_recvmsg: function(fd, message, flags) {
    var sock = getSocketFromFD(fd);
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
      var errno = writeSockaddr(name, sock.family, DNS.lookup_name(msg.addr), msg.port);
#if ASSERTIONS
      assert(!errno);
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
  },
#endif // ~PROXY_POSIX_SOCKETS==0
  __syscall_fchdir: function(fd) {
    var stream = SYSCALLS.getStreamFromFD(fd);
    FS.chdir(stream.path);
    return 0;
  },
  __syscall__newselect: function(nfds, readfds, writefds, exceptfds, timeout) {
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
  _msync_js: function(addr, len, flags, fd) {
#if CAN_ADDRESS_2GB
    addr >>>= 0;
#endif
    SYSCALLS.doMsync(addr, FS.getStream(fd), len, flags, 0);
    return 0;
  },
  __syscall_fdatasync: function(fd) {
    var stream = SYSCALLS.getStreamFromFD(fd);
    return 0; // we can't do anything synchronously; the in-memory FS is already synced to
  },
  __syscall_poll: function(fds, nfds, timeout) {
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
  __syscall_getcwd: function(buf, size) {
    if (size === 0) return -{{{ cDefine('EINVAL') }}};
    var cwd = FS.cwd();
    var cwdLengthInBytes = lengthBytesUTF8(cwd) + 1;
    if (size < cwdLengthInBytes) return -{{{ cDefine('ERANGE') }}};
    stringToUTF8(cwd, buf, size);
    return cwdLengthInBytes;
  },
  __syscall_truncate64: function(path, {{{ defineI64Param('length') }}}) {
    {{{ receiveI64ParamAsDouble('length') }}}
    path = SYSCALLS.getStr(path);
    FS.truncate(path, length);
    return 0;
  },
  __syscall_ftruncate64: function(fd, {{{ defineI64Param('length') }}}) {
    {{{ receiveI64ParamAsDouble('length') }}}
    FS.ftruncate(fd, length);
    return 0;
  },
  __syscall_stat64: function(path, buf) {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.doStat(FS.stat, path, buf);
  },
  __syscall_lstat64: function(path, buf) {
    path = SYSCALLS.getStr(path);
    return SYSCALLS.doStat(FS.lstat, path, buf);
  },
  __syscall_fstat64: function(fd, buf) {
    var stream = SYSCALLS.getStreamFromFD(fd);
    return SYSCALLS.doStat(FS.stat, stream.path, buf);
  },
  __syscall_fchown32: function(fd, owner, group) {
    FS.fchown(fd, owner, group);
    return 0;
  },
  __syscall_getdents64: function(fd, dirp, count) {
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
    FS.llseek(stream, idx * struct_size, {{{ cDefine('SEEK_SET') }}});
    return pos;
  },
  __syscall_fcntl64__deps: ['$setErrNo'],
  __syscall_fcntl64: function(fd, cmd, varargs) {
#if SYSCALLS_REQUIRE_FILESYSTEM == 0
#if SYSCALL_DEBUG
    err('no-op in fcntl syscall due to SYSCALLS_REQUIRE_FILESYSTEM=0');
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

  __syscall_statfs64: function(path, size, buf) {
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
  __syscall_fstatfs64: function(fd, size, buf) {
    var stream = SYSCALLS.getStreamFromFD(fd);
    return ___syscall_statfs64(0, size, buf);
  },
  __syscall_fadvise64__nothrow: true,
  __syscall_fadvise64__proxy: false,
  __syscall_fadvise64: function(fd, offset, len, advice) {
    return 0; // your advice is important to us (but we can't use it)
  },
  __syscall_openat: function(dirfd, path, flags, varargs) {
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    var mode = varargs ? SYSCALLS.get() : 0;
    return FS.open(path, flags, mode).fd;
  },
  __syscall_mkdirat: function(dirfd, path, mode) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doMkdir(path, mode);
  },
  __syscall_mknodat: function(dirfd, path, mode, dev) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doMknod(path, mode, dev);
  },
  __syscall_fchownat: function(dirfd, path, owner, group, flags) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
    var nofollow = flags & {{{ cDefine('AT_SYMLINK_NOFOLLOW') }}};
    flags = flags & (~{{{ cDefine('AT_SYMLINK_NOFOLLOW') }}});
#if ASSERTIONS
    assert(flags === 0);
#endif
    path = SYSCALLS.calculateAt(dirfd, path);
    (nofollow ? FS.lchown : FS.chown)(path, owner, group);
    return 0;
  },
  __syscall_newfstatat: function(dirfd, path, buf, flags) {
    path = SYSCALLS.getStr(path);
    var nofollow = flags & {{{ cDefine('AT_SYMLINK_NOFOLLOW') }}};
    var allowEmpty = flags & {{{ cDefine('AT_EMPTY_PATH') }}};
    flags = flags & (~{{{ cDefine('AT_SYMLINK_NOFOLLOW') | cDefine('AT_EMPTY_PATH') }}});
#if ASSERTIONS
    assert(!flags, flags);
#endif
    path = SYSCALLS.calculateAt(dirfd, path, allowEmpty);
    return SYSCALLS.doStat(nofollow ? FS.lstat : FS.stat, path, buf);
  },
  __syscall_unlinkat: function(dirfd, path, flags) {
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
  __syscall_renameat: function(olddirfd, oldpath, newdirfd, newpath) {
    oldpath = SYSCALLS.getStr(oldpath);
    newpath = SYSCALLS.getStr(newpath);
    oldpath = SYSCALLS.calculateAt(olddirfd, oldpath);
    newpath = SYSCALLS.calculateAt(newdirfd, newpath);
    FS.rename(oldpath, newpath);
    return 0;
  },
  __syscall_linkat__nothrow: true,
  __syscall_linkat__proxy: false,
  __syscall_linkat: function(olddirfd, oldpath, newdirfd, newpath, flags) {
    return -{{{ cDefine('EMLINK') }}}; // no hardlinks for us
  },
  __syscall_symlinkat: function(target, newdirfd, linkpath) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    linkpath = SYSCALLS.calculateAt(newdirfd, linkpath);
    FS.symlink(target, linkpath);
    return 0;
  },
  __syscall_readlinkat: function(dirfd, path, buf, bufsize) {
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doReadlink(path, buf, bufsize);
  },
  __syscall_fchmodat: function(dirfd, path, mode, varargs) {
#if SYSCALL_DEBUG
    err('warning: untested syscall');
#endif
    path = SYSCALLS.getStr(path);
    path = SYSCALLS.calculateAt(dirfd, path);
    FS.chmod(path, mode);
    return 0;
  },
  __syscall_faccessat: function(dirfd, path, amode, flags) {
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
  __syscall_utimensat: function(dirfd, path, times, flags) {
    path = SYSCALLS.getStr(path);
#if ASSERTIONS
    assert(flags === 0);
#endif
    path = SYSCALLS.calculateAt(dirfd, path, true);
    if (!times) {
      var atime = Date.now();
      var mtime = atime;
    } else {
      var seconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_sec, 'i32') }}};
      var nanoseconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_nsec, 'i32') }}};
      atime = (seconds*1000) + (nanoseconds/(1000*1000));
      times += {{{ C_STRUCTS.timespec.__size__ }}};
      seconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_sec, 'i32') }}};
      nanoseconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_nsec, 'i32') }}};
      mtime = (seconds*1000) + (nanoseconds/(1000*1000));
    }
    FS.utime(path, atime, mtime);
    return 0;
  },
  __syscall_fallocate: function(fd, mode, {{{ defineI64Param('offset') }}}, {{{ defineI64Param('len') }}}) {
    {{{ receiveI64ParamAsDouble('offset') }}}
    {{{ receiveI64ParamAsDouble('len') }}}
    var stream = SYSCALLS.getStreamFromFD(fd)
#if ASSERTIONS
    assert(mode === 0);
#endif
    FS.allocate(stream, offset, len);
    return 0;
  },
  __syscall_dup3: function(fd, suggestFD, flags) {
    var old = SYSCALLS.getStreamFromFD(fd);
#if ASSERTIONS
    assert(!flags);
#endif
    if (old.fd === suggestFD) return -{{{ cDefine('EINVAL') }}};
    var suggest = FS.getStream(suggestFD);
    if (suggest) FS.close(suggest);
    return FS.open(old.path, old.flags, 0, suggestFD, suggestFD).fd;
  },
};

function wrapSyscallFunction(x, library, isWasi) {
  if (x[0] === '$' || isJsLibraryConfigIdentifier(x)) {
    return;
  }

  var t = library[x];
  if (typeof t == 'string') return;
  t = t.toString();

  // If a syscall uses FS, but !SYSCALLS_REQUIRE_FILESYSTEM, then the user
  // has disabled the filesystem or we have proven some other way that this will
  // not be called in practice, and do not need that code.
  if (!SYSCALLS_REQUIRE_FILESYSTEM && t.includes('FS.')) {
    t = modifyFunction(t, function(name, args, body) {
      return 'function ' + name + '(' + args + ') {\n' +
             (ASSERTIONS ? "abort('it should not be possible to operate on streams when !SYSCALLS_REQUIRE_FILESYSTEM');\n" : '') +
             '}';
    });
  }

  var isVariadic = !isWasi && t.includes(', varargs');
#if SYSCALLS_REQUIRE_FILESYSTEM == 0
  var canThrow = false;
#else
  var canThrow = library[x + '__nothrow'] !== true;
#endif

  var pre = '', post = '';
  if (isVariadic) {
    pre += 'SYSCALLS.varargs = varargs;\n';
  }

#if SYSCALL_DEBUG
  if (isVariadic) {
    if (canThrow) {
      post += 'finally { SYSCALLS.varargs = undefined; }\n';
    } else {
      post += 'SYSCALLS.varargs = undefined;\n';
    }
  }
  pre += "err('syscall! " + x + ": [' + Array.prototype.slice.call(arguments) + ']');\n";
  pre += "var canWarn = true;\n";
  pre += "var ret = (function() {\n";
  post += "})();\n";
  post += "if (ret < 0 && canWarn) {\n";
  post += "  err('error: syscall may have failed with ' + (-ret) + ' (' + ERRNO_MESSAGES[-ret] + ')');\n";
  post += "}\n";
  post += "err('syscall return: ' + ret);\n";
  post += "return ret;\n";
#endif
  delete library[x + '__nothrow'];
  var handler = '';
  if (canThrow) {
    pre += 'try {\n';
    handler +=
    "} catch (e) {\n" +
    "  if (typeof FS == 'undefined' || !(e instanceof FS.ErrnoError)) throw e;\n";
#if SYSCALL_DEBUG
    handler +=
    "  err('error: syscall failed with ' + e.errno + ' (' + ERRNO_MESSAGES[e.errno] + ')');\n" +
    "  canWarn = false;\n";
#endif
    // Musl syscalls are negated.
    if (isWasi) {
      handler += "  return e.errno;\n";
    } else {
      // Musl syscalls are negated.
      handler += "  return -e.errno;\n";
    }
    handler += "}\n";
  }
  post = handler + post;

  if (pre || post) {
    t = modifyFunction(t, function(name, args, body) {
      return `function ${name}(${args}) {\n${pre}${body}${post}}\n`;
    });
  }

  if (MEMORY64 && !isWasi) {
    t = modifyFunction(t, function(name, args, body) {
      var argnums = args.split(",").map((a) => 'Number(' + a + ')').join();
      return 'function ' + name + '(' + args + ') {\n' +
             '  return (function ' + name + '_inner(' + args + ') {\n' +
             body +
             '  })(' + argnums + ');' +
             '}';
    });
  }

  library[x] = eval('(' + t + ')');
  if (!library[x + '__deps']) library[x + '__deps'] = [];
  library[x + '__deps'].push('$SYSCALLS');
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
  if (library[x + '__proxy'] === undefined) {
    library[x + '__proxy'] = 'sync';
  }
#endif
}

for (var x in SyscallsLibrary) {
  wrapSyscallFunction(x, SyscallsLibrary, false);
}

mergeInto(LibraryManager.library, SyscallsLibrary);
