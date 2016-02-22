var SyscallsLibrary = {
  $SYSCALLS__deps: [
#if NO_FILESYSTEM == 0
                   '$FS', '$ERRNO_CODES', '$PATH',
#endif
#if SYSCALL_DEBUG
                   '$ERRNO_MESSAGES'
#endif
  ],
  $SYSCALLS: {
#if NO_FILESYSTEM == 0
    // global constants
    DEFAULT_POLLMASK: {{{ cDefine('POLLIN') }}} | {{{ cDefine('POLLOUT') }}},

    // global state
    mappings: {},
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
          if (!dirstream) throw new FS.ErrnoError(ERRNO_CODES.EBADF);
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
          return -ERRNO_CODES.ENOTDIR;
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
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_size, 'stat.size', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_blksize, '4096', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_blocks, 'stat.blocks', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_atim.tv_sec, '(stat.atime.getTime() / 1000)|0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_atim.tv_nsec, '0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_mtim.tv_sec, '(stat.mtime.getTime() / 1000)|0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_mtim.tv_nsec, '0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_ctim.tv_sec, '(stat.ctime.getTime() / 1000)|0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_ctim.tv_nsec, '0', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.stat.st_ino, 'stat.ino', 'i32') }}};
      return 0;
    },
    doMsync: function(addr, stream, len, flags) {
      var buffer = new Uint8Array(HEAPU8.subarray(addr, addr + len));
      FS.msync(stream, buffer, 0, len, flags);
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
        default: return -ERRNO_CODES.EINVAL;
      }
      FS.mknod(path, mode, dev);
      return 0;
    },
    doReadlink: function(path, buf, bufsize) {
      if (bufsize <= 0) return -ERRNO_CODES.EINVAL;
      var ret = FS.readlink(path);
      ret = ret.slice(0, Math.max(0, bufsize));
      writeStringToMemory(ret, buf, true);
      return ret.length;
    },
    doAccess: function(path, amode) {
      if (amode & ~{{{ cDefine('S_IRWXO') }}}) {
        // need a valid mode
        return -ERRNO_CODES.EINVAL;
      }
      var node;
      var lookup = FS.lookupPath(path, { follow: true });
      node = lookup.node;
      var perms = '';
      if (amode & {{{ cDefine('R_OK') }}}) perms += 'r';
      if (amode & {{{ cDefine('W_OK') }}}) perms += 'w';
      if (amode & {{{ cDefine('X_OK') }}}) perms += 'x';
      if (perms /* otherwise, they've just passed F_OK */ && FS.nodePermissions(node, perms)) {
        return -ERRNO_CODES.EACCES;
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
#endif // NO_FILESYSTEM == 0

    // arguments handling

    varargs: 0,

    get: function(varargs) {
      SYSCALLS.varargs += 4;
      var ret = {{{ makeGetValue('SYSCALLS.varargs', '-4', 'i32') }}};
#if SYSCALL_DEBUG
      Module.printErr('    (raw: "' + ret + '")');
#endif
      return ret;
    },
    getStr: function() {
      var ret = Pointer_stringify(SYSCALLS.get());
#if SYSCALL_DEBUG
      Module.printErr('    (str: "' + ret + '")');
#endif
      return ret;
    },
#if NO_FILESYSTEM == 0
    getStreamFromFD: function() {
      var stream = FS.getStream(SYSCALLS.get());
      if (!stream) throw new FS.ErrnoError(ERRNO_CODES.EBADF);
#if SYSCALL_DEBUG
      Module.printErr('    (stream: "' + stream.path + '")');
#endif
      return stream;
    },
    getSocketFromFD: function() {
      var socket = SOCKFS.getSocket(SYSCALLS.get());
      if (!socket) throw new FS.ErrnoError(ERRNO_CODES.EBADF);
#if SYSCALL_DEBUG
      Module.printErr('    (socket: "' + socket.path + '")');
#endif
      return socket;
    },
    getSocketAddress: function(allowNull) {
      var addrp = SYSCALLS.get(), addrlen = SYSCALLS.get();
      if (allowNull && addrp === 0) return null;
      var info = __read_sockaddr(addrp, addrlen);
      if (info.errno) throw new FS.ErrnoError(info.errno);
      info.addr = DNS.lookup_addr(info.addr) || info.addr;
#if SYSCALL_DEBUG
      Module.printErr('    (socketaddress: "' + [info.addr, info.port] + '")');
#endif
      return info;
    },
#endif // NO_FILESYSTEM == 0
    get64: function() {
      var low = SYSCALLS.get(), high = SYSCALLS.get();
      if (low >= 0) assert(high === 0);
      else assert(high === -1);
#if SYSCALL_DEBUG
      Module.printErr('    (i64: "' + low + '")');
#endif
      return low;
    },
    getZero: function() {
      assert(SYSCALLS.get() === 0);
    }
  },

  __syscall1: function(which, varargs) { // exit
    var status = SYSCALLS.get();
    Module['exit'](status);
    return 0;
  },
  __syscall3: function(which, varargs) { // read
    var stream = SYSCALLS.getStreamFromFD(), buf = SYSCALLS.get(), count = SYSCALLS.get();
    return FS.read(stream, {{{ heapAndOffset('HEAP8', 'buf') }}}, count);
  },
  __syscall4: function(which, varargs) { // write
    var stream = SYSCALLS.getStreamFromFD(), buf = SYSCALLS.get(), count = SYSCALLS.get();
    return FS.write(stream, {{{ heapAndOffset('HEAP8', 'buf') }}}, count);
  },
  __syscall5: function(which, varargs) { // open
    var pathname = SYSCALLS.getStr(), flags = SYSCALLS.get(), mode = SYSCALLS.get() // optional TODO
    var stream = FS.open(pathname, flags, mode);
    return stream.fd;
  },
  __syscall6: function(which, varargs) { // close
    var stream = SYSCALLS.getStreamFromFD();
    FS.close(stream);
    return 0;
  },
  __syscall9: function(which, varargs) { // link
    var oldpath = SYSCALLS.get(), newpath = SYSCALLS.get();
    return -ERRNO_CODES.EMLINK; // no hardlinks for us
  },
  __syscall10: function(which, varargs) { // unlink
    var path = SYSCALLS.getStr();
    FS.unlink(path);
    return 0;
  },
  __syscall12: function(which, varargs) { // chdir
    var path = SYSCALLS.getStr();
    FS.chdir(path);
    return 0;
  },
  __syscall14: function(which, varargs) { // mknod
    var path = SYSCALLS.getStr(), mode = SYSCALLS.get(), dev = SYSCALLS.get();
    return SYSCALLS.doMknod(path, mode, dev);
  },
  __syscall15: function(which, varargs) { // chmod
    var path = SYSCALLS.getStr(), mode = SYSCALLS.get();
    FS.chmod(path, mode);
    return 0;
  },
  __syscall20__deps: ['$PROCINFO'],
  __syscall20: function(which, varargs) { // getpid
    return PROCINFO.pid;
  },
  __syscall29: function(which, varargs) { // pause
    return -ERRNO_CODES.EINTR; // we can't pause
  },
  __syscall33: function(which, varargs) { // access
    var path = SYSCALLS.getStr(), amode = SYSCALLS.get();
    return SYSCALLS.doAccess(path, amode);
  },
  __syscall34: function(which, varargs) { // nice
    var inc = SYSCALLS.get();
    return -ERRNO_CODES.EPERM; // no meaning to nice for our single-process environment
  },
  __syscall36: function(which, varargs) { // sync
    return 0;
  },
  __syscall38: function(which, varargs) { // rename
    var old_path = SYSCALLS.getStr(), new_path = SYSCALLS.getStr();
    FS.rename(old_path, new_path);
    return 0;
  },
  __syscall39: function(which, varargs) { // mkdir
    var path = SYSCALLS.getStr(), mode = SYSCALLS.get();
    return SYSCALLS.doMkdir(path, mode);
  },
  __syscall40: function(which, varargs) { // rmdir
    var path = SYSCALLS.getStr();
    FS.rmdir(path);
    return 0;
  },
  __syscall41: function(which, varargs) { // dup
    var old = SYSCALLS.getStreamFromFD();
    return FS.open(old.path, old.flags, 0).fd;
  },
  __syscall42: '__syscall51',      // pipe
  __syscall51: function(which, varargs) { // acct
    return -ERRNO_CODES.ENOSYS; // unsupported features
  },
  __syscall54: function(which, varargs) { // ioctl
#if NO_FILESYSTEM
#if SYSCALL_DEBUG
    Module.printErr('no-op in ioctl syscall due to NO_FILESYSTEM');
#endif
    return 0;
#else
    var stream = SYSCALLS.getStreamFromFD(), op = SYSCALLS.get();
    switch (op) {
      case {{{ cDefine('TCGETS') }}}: {
        if (!stream.tty) return -ERRNO_CODES.ENOTTY;
#if SYSCALL_DEBUG
        Module.printErr('warning: not filling tio struct');
#endif
        return 0;
      }
      case {{{ cDefine('TCSETS') }}}: {
        if (!stream.tty) return -ERRNO_CODES.ENOTTY;
        return 0; // no-op, not actually adjusting terminal settings
      }
      case {{{ cDefine('TIOCGPGRP') }}}: {
        if (!stream.tty) return -ERRNO_CODES.ENOTTY;
        var argp = SYSCALLS.get();
        {{{ makeSetValue('argp', 0, 0, 'i32') }}};
        return 0;
      }
      case {{{ cDefine('TIOCSPGRP') }}}: {
        if (!stream.tty) return -ERRNO_CODES.ENOTTY;
        return -ERRNO_CODES.EINVAL; // not supported
      }
      case {{{ cDefine('FIONREAD') }}}: {
        var argp = SYSCALLS.get();
        return FS.ioctl(stream, op, argp);
      }
      default: abort('bad ioctl syscall ' + op);
    }
#endif // NO_FILESYSTEM
  },
  __syscall57__deps: ['$PROCINFO'],
  __syscall57: function(which, varargs) { // setpgid
    var pid = SYSCALLS.get(), pgid = SYSCALLS.get();
    if (pid && pid !== PROCINFO.pid) return -ERRNO_CODES.ESRCH;
    if (pgid && pgid !== PROCINFO.pgid) return -ERRNO_CODES.EPERM;
    return 0;
  },
  __syscall60: function(which, varargs) { // umask
    var mask = SYSCALLS.get();
    var old = SYSCALLS.umask;
    SYSCALLS.umask = mask;
    return old;
  },
  __syscall63: function(which, varargs) { // dup2
    var old = SYSCALLS.getStreamFromFD(), suggestFD = SYSCALLS.get();
    if (old.fd === suggestFD) return suggestFD;
    return SYSCALLS.doDup(old.path, old.flags, suggestFD);
  },
  __syscall64__deps: ['$PROCINFO'],
  __syscall64: function(which, varargs) { // getppid
    return PROCINFO.ppid;
  },
  __syscall65__deps: ['$PROCINFO'],
  __syscall65: function(which, varargs) { // getpgrp
    return PROCINFO.pgid;
  },
  __syscall66: function(which, varargs) { // setsid
    return 0; // no-op
  },
  __syscall75: function(which, varargs) { // setrlimit
    return 0; // no-op
  },
  __syscall77: function(which, varargs) { // getrusage
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var who = SYSCALLS.get(), usage = SYSCALLS.get();
    _memset(usage, 0, {{{ C_STRUCTS.rusage.__size__ }}});
    {{{ makeSetValue('usage', C_STRUCTS.rusage.ru_utime.tv_sec, '1', 'i32') }}}; // fake some values
    {{{ makeSetValue('usage', C_STRUCTS.rusage.ru_utime.tv_usec, '2', 'i32') }}};
    {{{ makeSetValue('usage', C_STRUCTS.rusage.ru_stime.tv_sec, '3', 'i32') }}};
    {{{ makeSetValue('usage', C_STRUCTS.rusage.ru_stime.tv_usec, '4', 'i32') }}};
    return 0;
  },
  __syscall83: function(which, varargs) { // symlink
    var target = SYSCALLS.getStr(), linkpath = SYSCALLS.getStr();
    FS.symlink(target, linkpath);
    return 0;
  },
  __syscall85: function(which, varargs) { // readlink
    var path = SYSCALLS.getStr(), buf = SYSCALLS.get(), bufsize = SYSCALLS.get();
    return SYSCALLS.doReadlink(path, buf, bufsize);
  },
  __syscall91: function(which, varargs) { // munmap
    var addr = SYSCALLS.get(), len = SYSCALLS.get();
    // TODO: support unmmap'ing parts of allocations
    var info = SYSCALLS.mappings[addr];
    if (!info) return 0;
    if (len === info.len) {
      var stream = FS.getStream(info.fd);
      SYSCALLS.doMsync(addr, stream, len, info.flags)
      FS.munmap(stream);
      SYSCALLS.mappings[addr] = null;
      if (info.allocated) {
        _free(info.malloc);
      }
    }
    return 0;
  },
  __syscall94: function(which, varargs) { // fchmod
    var fd = SYSCALLS.get(), mode = SYSCALLS.get();
    FS.fchmod(fd, mode);
    return 0;
  },
  __syscall96: function(which, varargs) { // getpriority
    return 0;
  },
  __syscall97: function(which, varargs) { // setpriority
    return -ERRNO_CODES.EPERM;
  },
  __syscall102__deps: ['$SOCKFS', '$DNS', '_read_sockaddr', '_write_sockaddr'],
  __syscall102: function(which, varargs) { // socketcall
    var call = SYSCALLS.get(), socketvararg = SYSCALLS.get();
    // socketcalls pass the rest of the arguments in a struct
    SYSCALLS.varargs = socketvararg;
    switch (call) {
      case 1: { // socket
        var domain = SYSCALLS.get(), type = SYSCALLS.get(), protocol = SYSCALLS.get();
        var sock = SOCKFS.createSocket(domain, type, protocol);
        assert(sock.stream.fd < 64); // XXX ? select() assumes socket fd values are in 0..63
        return sock.stream.fd;
      }
      case 2: { // bind
        var sock = SYSCALLS.getSocketFromFD(), info = SYSCALLS.getSocketAddress();
        sock.sock_ops.bind(sock, info.addr, info.port);
        return 0;
      }
      case 3: { // connect
        var sock = SYSCALLS.getSocketFromFD(), info = SYSCALLS.getSocketAddress();
        sock.sock_ops.connect(sock, info.addr, info.port);
        return 0;
      }
      case 4: { // listen
        var sock = SYSCALLS.getSocketFromFD(), backlog = SYSCALLS.get();
        sock.sock_ops.listen(sock, backlog);
        return 0;
      }
      case 5: { // accept
        var sock = SYSCALLS.getSocketFromFD(), addr = SYSCALLS.get(), addrlen = SYSCALLS.get();
        var newsock = sock.sock_ops.accept(sock);
        if (addr) {
          var res = __write_sockaddr(addr, newsock.family, DNS.lookup_name(newsock.daddr), newsock.dport);
          assert(!res.errno);
        }
        return newsock.stream.fd;
      }
      case 6: { // getsockname
        var sock = SYSCALLS.getSocketFromFD(), addr = SYSCALLS.get(), addrlen = SYSCALLS.get();
        // TODO: sock.saddr should never be undefined, see TODO in websocket_sock_ops.getname
        var res = __write_sockaddr(addr, sock.family, DNS.lookup_name(sock.saddr || '0.0.0.0'), sock.sport);
        assert(!res.errno);
        return 0;
      }
      case 7: { // getpeername
        var sock = SYSCALLS.getSocketFromFD(), addr = SYSCALLS.get(), addrlen = SYSCALLS.get();
        if (!sock.daddr) {
          return -ERRNO_CODES.ENOTCONN; // The socket is not connected.
        }
        var res = __write_sockaddr(addr, sock.family, DNS.lookup_name(sock.daddr), sock.dport);
        assert(!res.errno);
        return 0;
      }
      case 11: { // sendto
        var sock = SYSCALLS.getSocketFromFD(), message = SYSCALLS.get(), length = SYSCALLS.get(), flags = SYSCALLS.get(), dest = SYSCALLS.getSocketAddress(true);
        if (!dest) {
          // send, no address provided
          return FS.write(sock.stream, {{{ heapAndOffset('HEAP8', 'message') }}}, length);
        } else {
          // sendto an address
          return sock.sock_ops.sendmsg(sock, {{{ heapAndOffset('HEAP8', 'message') }}}, length, dest.addr, dest.port);
        }
      }
      case 12: { // recvfrom
        var sock = SYSCALLS.getSocketFromFD(), buf = SYSCALLS.get(), len = SYSCALLS.get(), flags = SYSCALLS.get(), addr = SYSCALLS.get(), addrlen = SYSCALLS.get();
        var msg = sock.sock_ops.recvmsg(sock, len);
        if (!msg) return 0; // socket is closed
        if (addr) {
          var res = __write_sockaddr(addr, sock.family, DNS.lookup_name(msg.addr), msg.port);
          assert(!res.errno);
        }
        HEAPU8.set(msg.buffer, buf);
        return msg.buffer.byteLength;
      }
      case 14: { // setsockopt
        return -ERRNO_CODES.ENOPROTOOPT; // The option is unknown at the level indicated.
      }
      case 15: { // getsockopt
        var sock = SYSCALLS.getSocketFromFD(), level = SYSCALLS.get(), optname = SYSCALLS.get(), optval = SYSCALLS.get(), optlen = SYSCALLS.get();
        // Minimal getsockopt aimed at resolving https://github.com/kripken/emscripten/issues/2211
        // so only supports SOL_SOCKET with SO_ERROR.
        if (level === {{{ cDefine('SOL_SOCKET') }}}) {
          if (optname === {{{ cDefine('SO_ERROR') }}}) {
            {{{ makeSetValue('optval', 0, 'sock.error', 'i32') }}};
            {{{ makeSetValue('optlen', 0, 4, 'i32') }}};
            sock.error = null; // Clear the error (The SO_ERROR option obtains and then clears this field).
            return 0;
          }
        }
        return -ERRNO_CODES.ENOPROTOOPT; // The option is unknown at the level indicated.
      }
      case 16: { // sendmsg
        var sock = SYSCALLS.getSocketFromFD(), message = SYSCALLS.get(), flags = SYSCALLS.get();
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
        var sock = SYSCALLS.getSocketFromFD(), message = SYSCALLS.get(), flags = SYSCALLS.get();
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
          assert(!res.errno);
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
      default: abort('unsupported socketcall syscall ' + call);
    }
  },
  __syscall104: function(which, varargs) { // setitimer
    return -ERRNO_CODES.ENOSYS; // unsupported feature
  },
  __syscall114: function(which, varargs) { // wait4
    abort('cannot wait on child processes');
  },
#if EMTERPRETIFY_ASYNC
  __syscall118__deps: ['$EmterpreterAsync'],
#endif
  __syscall118: function(which, varargs) { // fsync
    var stream = SYSCALLS.getStreamFromFD();
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
          resume(function() { return -ERRNO_CODES.EIO });
          return;
        }
        resume(function() { return 0 });
      });
    });
#else
    return 0; // we can't do anything synchronously; the in-memory FS is already synced to
#endif
  },
  __syscall121: function(which, varargs) { // setdomainname
    return -ERRNO_CODES.EPERM;
  },
  __syscall122: function(which, varargs) { // uname
    var buf = SYSCALLS.get();
    if (!buf) return -ERRNO_CODES.EFAULT
    var layout = {{{ JSON.stringify(C_STRUCTS.utsname) }}};
    function copyString(element, value) {
      var offset = layout[element];
      writeAsciiToMemory(value, buf + offset);
    }
    copyString('sysname', 'Emscripten');
    copyString('nodename', 'emscripten');
    copyString('release', '1.0');
    copyString('version', '#1');
    copyString('machine', 'x86-JS');
    return 0;
  },
  __syscall125: function(which, varargs) { // mprotect
    return 0; // let's not and say we did
  },
  __syscall132__deps: ['$PROCINFO'],
  __syscall132: function(which, varargs) { // getpgid
    var pid = SYSCALLS.get();
    if (pid && pid !== PROCINFO.pid) return -ERRNO_CODES.ESRCH;
    return PROCINFO.pgid;
  },
  __syscall133: function(which, varargs) { // fchdir
    var stream = SYSCALLS.getStreamFromFD();
    FS.chdir(stream.path);
    return 0;
  },
  __syscall140: function(which, varargs) { // llseek
    var stream = SYSCALLS.getStreamFromFD(), offset_high = SYSCALLS.get(), offset_low = SYSCALLS.get(), result = SYSCALLS.get(), whence = SYSCALLS.get();
    var offset = offset_low;
    assert(offset_high === 0);
    FS.llseek(stream, offset, whence);
    {{{ makeSetValue('result', '0', 'stream.position', 'i32') }}};
    if (stream.getdents && offset === 0 && whence === {{{ cDefine('SEEK_SET') }}}) stream.getdents = null; // reset readdir state
    return 0;
  },
  __syscall142: function(which, varargs) { // newselect
    // readfds are supported,
    // writefds checks socket open status
    // exceptfds not supported
    // timeout is always 0 - fully async
    var nfds = SYSCALLS.get(), readfds = SYSCALLS.get(), writefds = SYSCALLS.get(), exceptfds = SYSCALLS.get(), timeout = SYSCALLS.get();

    assert(nfds <= 64, 'nfds must be less than or equal to 64');  // fd sets have 64 bits // TODO: this could be 1024 based on current musl headers
    assert(!exceptfds, 'exceptfds not supported');

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

    function check(fd, low, high, val) {
      return (fd < 32 ? (low & val) : (high & val));
    }

    for (var fd = 0; fd < nfds; fd++) {
      var mask = 1 << (fd % 32);
      if (!(check(fd, allLow, allHigh, mask))) {
        continue;  // index isn't in the set
      }

      var stream = FS.getStream(fd);
      if (!stream) throw new FS.ErrnoError(ERRNO_CODES.EBADF);

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
  __syscall144: function(which, varargs) { // msync
    var addr = SYSCALLS.get(), len = SYSCALLS.get(), flags = SYSCALLS.get();
    var info = SYSCALLS.mappings[addr];
    if (!info) return 0;
    SYSCALLS.doMsync(addr, FS.getStream(info.fd), len, info.flags);
    return 0;
  },
  __syscall145: function(which, varargs) { // readv
    var stream = SYSCALLS.getStreamFromFD(), iov = SYSCALLS.get(), iovcnt = SYSCALLS.get();
    return SYSCALLS.doReadv(stream, iov, iovcnt);
  },
#if NO_FILESYSTEM
  __syscall146__postset: '/* flush anything remaining in the buffer during shutdown */ __ATEXIT__.push(function() { var fflush = Module["_fflush"]; if (fflush) fflush(0); var printChar = ___syscall146.printChar; if (!printChar) return; var buffers = ___syscall146.buffers; if (buffers[1].length) printChar(1, {{{ charCode("\n") }}}); if (buffers[2].length) printChar(2, {{{ charCode("\n") }}}); });',
#endif
  __syscall146: function(which, varargs) { // writev
#if NO_FILESYSTEM == 0
    var stream = SYSCALLS.getStreamFromFD(), iov = SYSCALLS.get(), iovcnt = SYSCALLS.get();
    return SYSCALLS.doWritev(stream, iov, iovcnt);
#else
    // hack to support printf in NO_FILESYSTEM
    var stream = SYSCALLS.get(), iov = SYSCALLS.get(), iovcnt = SYSCALLS.get();
    var ret = 0;
    if (!___syscall146.buffer) {
      ___syscall146.buffers = [null, [], []]; // 1 => stdout, 2 => stderr
      ___syscall146.printChar = function(stream, curr) {
        var buffer = ___syscall146.buffers[stream];
        assert(buffer);
        if (curr === 0 || curr === {{{ charCode('\n') }}}) {
          (stream === 1 ? Module['print'] : Module['printErr'])(UTF8ArrayToString(buffer, 0));
          buffer.length = 0;
        } else {
          buffer.push(curr);
        }
      };
    }
    for (var i = 0; i < iovcnt; i++) {
      var ptr = {{{ makeGetValue('iov', 'i*8', 'i32') }}};
      var len = {{{ makeGetValue('iov', 'i*8 + 4', 'i32') }}};
      for (var j = 0; j < len; j++) {
        ___syscall146.printChar(stream, HEAPU8[ptr+j]);
      }
      ret += len;
    }
    return ret;
#endif // NO_FILESYSTEM == 0
  },
  __syscall147__deps: ['$PROCINFO'],
  __syscall147: function(which, varargs) { // getsid
    var pid = SYSCALLS.get();
    if (pid && pid !== PROCINFO.pid) return -ERRNO_CODES.ESRCH;
    return PROCINFO.sid;
  },
  __syscall148: function(which, varargs) { // fdatasync
    var stream = SYSCALLS.getStreamFromFD();
    return 0; // we can't do anything synchronously; the in-memory FS is already synced to
  },
  __syscall150: '__syscall153',     // mlock
  __syscall151: '__syscall153',     // munlock
  __syscall152: '__syscall153',     // mlockall
  __syscall153: function(which, varargs) { // munlockall
    return 0;
  },
  __syscall163: function(which, varargs) { // mremap
    return -ERRNO_CODES.ENOMEM; // never succeed
  },
  __syscall168: function(which, varargs) { // poll
    var fds = SYSCALLS.get(), nfds = SYSCALLS.get(), timeout = SYSCALLS.get();
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
  __syscall180: function(which, varargs) { // pread64
    var stream = SYSCALLS.getStreamFromFD(), buf = SYSCALLS.get(), count = SYSCALLS.get(), zero = SYSCALLS.getZero(), offset = SYSCALLS.get64();
    return FS.read(stream, {{{ heapAndOffset('HEAP8', 'buf') }}}, count, offset);
  },
  __syscall181: function(which, varargs) { // pwrite64
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var stream = SYSCALLS.getStreamFromFD(), buf = SYSCALLS.get(), count = SYSCALLS.get(), zero = SYSCALLS.getZero(), offset = SYSCALLS.get64();
    return FS.write(stream, {{{ heapAndOffset('HEAP8', 'buf') }}}, nbyte, offset);
  },
  __syscall183: function(which, varargs) { // getcwd
    var buf = SYSCALLS.get(), size = SYSCALLS.get();
    if (size === 0) return -ERRNO_CODES.EINVAL;
    var cwd = FS.cwd();
    if (size < cwd.length + 1) return -ERRNO_CODES.ERANGE;
    writeAsciiToMemory(cwd, buf);
    return buf;
  },
  __syscall191: function(which, varargs) { // ugetrlimit
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var resource = SYSCALLS.get(), rlim = SYSCALLS.get();
    {{{ makeSetValue('rlim', C_STRUCTS.rlimit.rlim_cur, '-1', 'i32') }}};  // RLIM_INFINITY
    {{{ makeSetValue('rlim', C_STRUCTS.rlimit.rlim_cur + 4, '-1', 'i32') }}};  // RLIM_INFINITY
    {{{ makeSetValue('rlim', C_STRUCTS.rlimit.rlim_max, '-1', 'i32') }}};  // RLIM_INFINITY
    {{{ makeSetValue('rlim', C_STRUCTS.rlimit.rlim_max + 4, '-1', 'i32') }}};  // RLIM_INFINITY
    return 0; // just report no limits
  },
  __syscall192: function(which, varargs) { // mmap2
    var addr = SYSCALLS.get(), len = SYSCALLS.get(), prot = SYSCALLS.get(), flags = SYSCALLS.get(), fd = SYSCALLS.get(), off = SYSCALLS.get()
    off <<= 12; // undo pgoffset
    var ptr;
    var allocated = false;
    if (fd === -1) {
      ptr = _malloc(len);
      if (!ptr) return -ERRNO_CODES.ENOMEM;
      _memset(ptr, 0, len);
      allocated = true;
    } else {
      var info = FS.getStream(fd);
      if (!info) return -ERRNO_CODES.EBADF;
      var res = FS.mmap(info, HEAPU8, addr, len, off, prot, flags);
      ptr = res.ptr;
      allocated = res.allocated;
    }
    SYSCALLS.mappings[ptr] = { malloc: ptr, len: len, allocated: allocated, fd: fd, flags: flags };
    return ptr;
  },
  __syscall193: function(which, varargs) { // truncate64
    var path = SYSCALLS.getStr(), zero = SYSCALLS.getZero(), length = SYSCALLS.get64();
    FS.truncate(path, length);
    return 0;
  },
  __syscall194: function(which, varargs) { // ftruncate64
    var fd = SYSCALLS.get(), zero = SYSCALLS.getZero(), length = SYSCALLS.get64();
    FS.ftruncate(fd, length);
    return 0;
  },
  __syscall195: function(which, varargs) { // SYS_stat64
    var path = SYSCALLS.getStr(), buf = SYSCALLS.get();
    return SYSCALLS.doStat(FS.stat, path, buf);
  },
  __syscall196: function(which, varargs) { // SYS_lstat64
    var path = SYSCALLS.getStr(), buf = SYSCALLS.get();
    return SYSCALLS.doStat(FS.lstat, path, buf);
  },
  __syscall197: function(which, varargs) { // SYS_fstat64
    var stream = SYSCALLS.getStreamFromFD(), buf = SYSCALLS.get();
    return SYSCALLS.doStat(FS.stat, stream.path, buf);
  },
  __syscall198: function(which, varargs) { // lchown32
    var path = SYSCALLS.getStr(), owner = SYSCALLS.get(), group = SYSCALLS.get();
    FS.chown(path, owner, group); // XXX we ignore the 'l' aspect, and do the same as chown
    return 0;
  },
  __syscall199: '__syscall202',     // getuid32
  __syscall200: '__syscall202',     // getgid32
  __syscall201: '__syscall202',     // geteuid32
  __syscall202: function(which, varargs) { // getgid32
    return 0;
  },
  __syscall207: function(which, varargs) { // fchown32
    var fd = SYSCALLS.get(), owner = SYSCALLS.get(), group = SYSCALLS.get();
    FS.fchown(fd, owner, group);
    return 0;
  },
  __syscall212: function(which, varargs) { // chown32
    var path = SYSCALLS.getStr(), owner = SYSCALLS.get(), group = SYSCALLS.get();
    FS.chown(path, owner, group);
    return 0;
  },
  __syscall203: '__syscall214',     // setreuid32
  __syscall204: '__syscall214',     // setregid32
  __syscall213: '__syscall214',     // setuid32
  __syscall214: function(which, varargs) { // setgid32
    var uid = SYSCALLS.get();
    if (uid !== 0) return -ERRNO_CODES.EPERM;
    return 0;
  },
  __syscall205: function(which, varargs) { // getgroups32
    var size = SYSCALLS.get(), list = SYSCALLS.get();
    if (size < 1) return -ERRNO_CODES.EINVAL;
    {{{ makeSetValue('list', '0', '0', 'i32') }}};
    return 1;
  },
  __syscall208: '__syscall210',     // setresuid32
  __syscall210: function(which, varargs) { // setresgid32
    var ruid = SYSCALLS.get(), euid = SYSCALLS.get(), suid = SYSCALLS.get();
    if (euid !== 0) return -ERRNO_CODES.EPERM;
    return 0;
  },
  __syscall209: '__syscall211',     // getresuid
  __syscall211: function(which, varargs) { // getresgid32
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var ruid = SYSCALLS.get(), euid = SYSCALLS.get(), suid = SYSCALLS.get();
    {{{ makeSetValue('ruid', '0', '0', 'i32') }}};
    {{{ makeSetValue('euid', '0', '0', 'i32') }}};
    {{{ makeSetValue('suid', '0', '0', 'i32') }}};
    return 0;
  },
  __syscall218: function(which, varargs) { // mincore
    return -ERRNO_CODES.ENOSYS; // unsupported feature
  },
  __syscall219: function(which, varargs) { // madvise
    return 0; // advice is welcome, but ignored
  },
  __syscall220: function(which, varargs) { // SYS_getdents64
    var stream = SYSCALLS.getStreamFromFD(), dirp = SYSCALLS.get(), count = SYSCALLS.get();
    if (!stream.getdents) {
      stream.getdents = FS.readdir(stream.path);
    }
    var pos = 0;
    while (stream.getdents.length > 0 && pos + {{{ C_STRUCTS.dirent.__size__ }}} < count) {
      var id;
      var type;
      var name = stream.getdents.pop();
      assert(name.length < 256); // limit of dirent struct
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
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_ino, 'id', 'i32') }}};
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_off, 'stream.position', 'i32') }}};
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_reclen, C_STRUCTS.dirent.__size__, 'i16') }}};
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_type, 'type', 'i8') }}};
      for (var i = 0; i < name.length; i++) {
        {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_name + ' + i', 'name.charCodeAt(i)', 'i8') }}};
      }
      {{{ makeSetValue('dirp + pos', C_STRUCTS.dirent.d_name + ' + i', '0', 'i8') }}};
      pos += {{{ C_STRUCTS.dirent.__size__ }}};
    }
    return pos;
  },
  __syscall221__deps: ['__setErrNo'],
  __syscall221: function(which, varargs) { // fcntl64
    var stream = SYSCALLS.getStreamFromFD(), cmd = SYSCALLS.get();
    switch (cmd) {
      case {{{ cDefine('F_DUPFD') }}}: {
        var arg = SYSCALLS.get();
        if (arg < 0) {
          return -ERRNO_CODES.EINVAL;
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
      case {{{ cDefine('F_GETLK64') }}}: {
        var arg = SYSCALLS.get();
        var offset = {{{ C_STRUCTS.flock.l_type }}};
        // We're always unlocked.
        {{{ makeSetValue('arg', 'offset', cDefine('F_UNLCK'), 'i16') }}};
        return 0;
      }
      case {{{ cDefine('F_SETLK') }}}:
      case {{{ cDefine('F_SETLKW') }}}:
      case {{{ cDefine('F_SETLK64') }}}:
      case {{{ cDefine('F_SETLKW64') }}}:
        return 0; // Pretend that the locking is successful.
      case {{{ cDefine('F_GETOWN_EX') }}}:
      case {{{ cDefine('F_SETOWN') }}}:
        return -ERRNO_CODES.EINVAL; // These are for sockets. We don't have them fully implemented yet.
      case {{{ cDefine('F_GETOWN') }}}:
        // musl trusts getown return values, due to a bug where they must be, as they overlap with errors. just return -1 here, so fnctl() returns that, and we set errno ourselves.
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
      default: {
#if SYSCALL_DEBUG
        Module.printErr('warning: fctl64 unrecognized command ' + cmd);
#endif
        return -ERRNO_CODES.EINVAL;
      }
    }
  },
  __syscall265: function(which, varargs) { // clock_nanosleep
#if SYSCALL_DEBUG
    Module.printErr('warning: ignoring SYS_clock_nanosleep');
#endif
    return 0;
  },
  __syscall268: function(which, varargs) { // statfs64
    var path = SYSCALLS.getStr(), size = SYSCALLS.get(), buf = SYSCALLS.get();
    assert(size === {{{ C_STRUCTS.statfs.__size__ }}});
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
  __syscall269: function(which, varargs) { // fstatfs64
    var stream = SYSCALLS.getStreamFromFD(), size = SYSCALLS.get(), buf = SYSCALLS.get();
    return ___syscall([268, 0, size, buf], 0);
  },
  __syscall272: function(which, varargs) { // fadvise64_64
    return 0; // your advice is important to us (but we can't use it)
  },
  __syscall295: function(which, varargs) { // openat
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var dirfd = SYSCALLS.get(), path = SYSCALLS.getStr(), flags = SYSCALLS.get(), mode = SYSCALLS.get();
    path = SYSCALLS.calculateAt(dirfd, path);
    return FS.open(path, flags, mode).fd;
  },
  __syscall296: function(which, varargs) { // mkdirat
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var dirfd = SYSCALLS.get(), path = SYSCALLS.getStr(), mode = SYSCALLS.get();
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doMkdir(path, mode);
  },
  __syscall297: function(which, varargs) { // mknodat
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var dirfd = SYSCALLS.get(), path = SYSCALLS.getStr(), mode = SYSCALLS.get(), dev = SYSCALLS.get();
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doMknod(path, mode, dev);
  },
  __syscall298: function(which, varargs) { // fchownat
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var dirfd = SYSCALLS.get(), path = SYSCALLS.getStr(), owner = SYSCALLS.get(), group = SYSCALLS.get(), flags = SYSCALLS.get();
    assert(flags === 0);
    path = SYSCALLS.calculateAt(dirfd, path);
    FS.chown(path, owner, group);
    return 0;
  },
  __syscall299: function(which, varargs) { // futimesat
    abort('futimesat is obsolete');
  },
  __syscall300: function(which, varargs) { // fstatat64
    var dirfd = SYSCALLS.get(), path = SYSCALLS.getStr(), buf = SYSCALLS.get(), flags = SYSCALLS.get();
    var nofollow = flags & {{{ cDefine('AT_SYMLINK_NOFOLLOW') }}};
    flags = flags & (~{{{ cDefine('AT_SYMLINK_NOFOLLOW') }}});
    assert(!flags, flags);
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doStat(nofollow ? FS.lstat : FS.stat, path, buf);
  },
  __syscall301: function(which, varargs) { // unlinkat
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var dirfd = SYSCALLS.get(), path = SYSCALLS.getStr(), flags = SYSCALLS.get();
    assert(flags === 0);
    path = SYSCALLS.calculateAt(dirfd, path);
    FS.unlink(path);
    return 0;
  },
  __syscall302: function(which, varargs) { // renameat
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var olddirfd = SYSCALLS.get(), oldpath = SYSCALLS.getStr(), newdirfd = SYSCALLS.get(), newpath = SYSCALLS.getStr();
    oldpath = SYSCALLS.calculateAt(olddirfd, oldpath);
    newpath = SYSCALLS.calculateAt(newdirfd, newpath);
    FS.rename(oldpath, newpath);
    return 0;
  },
  __syscall303: function(which, varargs) { // linkat
    return -ERRNO_CODES.EMLINK; // no hardlinks for us
  },
  __syscall304: function(which, varargs) { // symlinkat
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var target = SYSCALLS.get(), newdirfd = SYSCALLS.get(), linkpath = SYSCALLS.get();
    linkpath = SYSCALLS.calculateAt(newdirfd, linkpath);
    FS.symlink(target, linkpath);
    return 0;
  },
  __syscall305: function(which, varargs) { // readlinkat
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var dirfd = SYSCALLS.get(), path = SYSCALLS.getStr(), buf = SYSCALLS.get(), bufsize = SYSCALLS.get();
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doReadlink(path, buf, bufsize);
  },
  __syscall306: function(which, varargs) { // fchmodat
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var dirfd = SYSCALLS.get(), path = SYSCALLS.getStr(), mode = SYSCALLS.get(), flags = SYSCALLS.get();
    assert(flags === 0);
    path = SYSCALLS.calculateAt(dirfd, path);
    FS.chmod(path, mode);
    return 0;
  },
  __syscall307: function(which, varargs) { // faccessat
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var dirfd = SYSCALLS.get(), path = SYSCALLS.getStr(), amode = SYSCALLS.get(), flags = SYSCALLS.get();
    assert(flags === 0);
    path = SYSCALLS.calculateAt(dirfd, path);
    return SYSCALLS.doAccess(path, amode);
  },
  __syscall308: function(which, varargs) { // pselect
    return -ERRNO_CODES.ENOSYS; // unsupported feature
  },
  __syscall320: function(which, varargs) { // utimensat
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var dirfd = SYSCALLS.get(), path = SYSCALLS.getStr(), times = SYSCALLS.get(), flags = SYSCALLS.get();
    assert(flags === 0);
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
  __syscall324: function(which, varargs) { // fallocate
    var stream = SYSCALLS.getStreamFromFD(), mode = SYSCALLS.get(), offset = SYSCALLS.get64(), len = SYSCALLS.get64();
    assert(mode === 0);
    FS.allocate(stream, offset, len);
    return 0;
  },
  __syscall330: function(which, varargs) { // dup3
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var old = SYSCALLS.getStreamFromFD(), suggestFD = SYSCALLS.get(), flags = SYSCALLS.get();
    assert(!flags);
    if (old.fd === suggestFD) return -ERRNO_CODES.EINVAL;
    return SYSCALLS.doDup(old.path, old.flags, suggestFD);
  },
  __syscall331: function(which, varargs) { // pipe2
    return -ERRNO_CODES.ENOSYS; // unsupported feature
  },
  __syscall333: function(which, varargs) { // preadv
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var stream = SYSCALLS.getStreamFromFD(), iov = SYSCALLS.get(), iovcnt = SYSCALLS.get(), offset = SYSCALLS.get();
    return SYSCALLS.doReadv(stream, iov, iovcnt, offset);
  },
  __syscall334: function(which, varargs) { // pwritev
#if SYSCALL_DEBUG
    Module.printErr('warning: untested syscall');
#endif
    var stream = SYSCALLS.getStreamFromFD(), iov = SYSCALLS.get(), iovcnt = SYSCALLS.get(), offset = SYSCALLS.get();
    return SYSCALLS.doWritev(stream, iov, iovcnt, offset);
  },
  __syscall340: function(which, varargs) { // prlimit64
    var pid = SYSCALLS.get(), resource = SYSCALLS.get(), new_limit = SYSCALLS.get(), old_limit = SYSCALLS.get();
    if (old_limit) { // just report no limits
      {{{ makeSetValue('old_limit', C_STRUCTS.rlimit.rlim_cur, '-1', 'i32') }}};  // RLIM_INFINITY
      {{{ makeSetValue('old_limit', C_STRUCTS.rlimit.rlim_cur + 4, '-1', 'i32') }}};  // RLIM_INFINITY
      {{{ makeSetValue('old_limit', C_STRUCTS.rlimit.rlim_max, '-1', 'i32') }}};  // RLIM_INFINITY
      {{{ makeSetValue('old_limit', C_STRUCTS.rlimit.rlim_max + 4, '-1', 'i32') }}};  // RLIM_INFINITY
    }
    return 0;
  },
};

if (SYSCALL_DEBUG) {
  var SYSCALL_NAME_TO_CODE = {
    SYS_restart_syscall: 0,
    SYS_exit: 1,
    SYS_fork: 2,
    SYS_read: 3,
    SYS_write: 4,
    SYS_open: 5,
    SYS_close: 6,
    SYS_waitpid: 7,
    SYS_creat: 8,
    SYS_link: 9,
    SYS_unlink: 10,
    SYS_execve: 11,
    SYS_chdir: 12,
    SYS_time: 13,
    SYS_mknod: 14,
    SYS_chmod: 15,
    SYS_lchown: 16,
    SYS_break: 17,
    SYS_oldstat: 18,
    SYS_lseek: 19,
    SYS_getpid: 20,
    SYS_mount: 21,
    SYS_umount: 22,
    SYS_setuid: 23,
    SYS_getuid: 24,
    SYS_stime: 25,
    SYS_ptrace: 26,
    SYS_alarm: 27,
    SYS_oldfstat: 28,
    SYS_pause: 29,
    SYS_utime: 30,
    SYS_stty: 31,
    SYS_gtty: 32,
    SYS_access: 33,
    SYS_nice: 34,
    SYS_ftime: 35,
    SYS_sync: 36,
    SYS_kill: 37,
    SYS_rename: 38,
    SYS_mkdir: 39,
    SYS_rmdir: 40,
    SYS_dup: 41,
    SYS_pipe: 42,
    SYS_times: 43,
    SYS_prof: 44,
    SYS_brk: 45,
    SYS_setgid: 46,
    SYS_getgid: 47,
    SYS_signal: 48,
    SYS_geteuid: 49,
    SYS_getegid: 50,
    SYS_acct: 51,
    SYS_umount2: 52,
    SYS_lock: 53,
    SYS_ioctl: 54,
    SYS_fcntl: 55,
    SYS_mpx: 56,
    SYS_setpgid: 57,
    SYS_ulimit: 58,
    SYS_oldolduname: 59,
    SYS_umask: 60,
    SYS_chroot: 61,
    SYS_ustat: 62,
    SYS_dup2: 63,
    SYS_getppid: 64,
    SYS_getpgrp: 65,
    SYS_setsid: 66,
    SYS_sigaction: 67,
    SYS_sgetmask: 68,
    SYS_ssetmask: 69,
    SYS_setreuid: 70,
    SYS_setregid: 71,
    SYS_sigsuspend: 72,
    SYS_sigpending: 73,
    SYS_sethostname: 74,
    SYS_setrlimit: 75,
    SYS_getrlimit: 76   /* Back compatible 2Gig limited rlimit */,
    SYS_getrusage: 77,
    SYS_gettimeofday: 78,
    SYS_settimeofday: 79,
    SYS_getgroups: 80,
    SYS_setgroups: 81,
    SYS_select: 82,
    SYS_symlink: 83,
    SYS_oldlstat: 84,
    SYS_readlink: 85,
    SYS_uselib: 86,
    SYS_swapon: 87,
    SYS_reboot: 88,
    SYS_readdir: 89,
    SYS_mmap: 90,
    SYS_munmap: 91,
    SYS_truncate: 92,
    SYS_ftruncate: 93,
    SYS_fchmod: 94,
    SYS_fchown: 95,
    SYS_getpriority: 96,
    SYS_setpriority: 97,
    SYS_profil: 98,
    SYS_statfs: 99,
    SYS_fstatfs: 100,
    SYS_ioperm: 101,
    SYS_socketcall: 102,
    SYS_syslog: 103,
    SYS_setitimer: 104,
    SYS_getitimer: 105,
    SYS_stat: 106,
    SYS_lstat: 107,
    SYS_fstat: 108,
    SYS_olduname: 109,
    SYS_iopl: 110,
    SYS_vhangup: 111,
    SYS_idle: 112,
    SYS_vm86old: 113,
    SYS_wait4: 114,
    SYS_swapoff: 115,
    SYS_sysinfo: 116,
    SYS_ipc: 117,
    SYS_fsync: 118,
    SYS_sigreturn: 119,
    SYS_clone: 120,
    SYS_setdomainname: 121,
    SYS_uname: 122,
    SYS_modify_ldt: 123,
    SYS_adjtimex: 124,
    SYS_mprotect: 125,
    SYS_sigprocmask: 126,
    SYS_create_module: 127,
    SYS_init_module: 128,
    SYS_delete_module: 129,
    SYS_get_kernel_syms: 130,
    SYS_quotactl: 131,
    SYS_getpgid: 132,
    SYS_fchdir: 133,
    SYS_bdflush: 134,
    SYS_sysfs: 135,
    SYS_personality: 136,
    SYS_afs_syscall: 137,
    SYS_setfsuid: 138,
    SYS_setfsgid: 139,
    SYS__llseek: 140,
    SYS_getdents: 141,
    SYS__newselect: 142,
    SYS_flock: 143,
    SYS_msync: 144,
    SYS_readv: 145,
    SYS_writev: 146,
    SYS_getsid: 147,
    SYS_fdatasync: 148,
    SYS__sysctl: 149,
    SYS_mlock: 150,
    SYS_munlock: 151,
    SYS_mlockall: 152,
    SYS_munlockall: 153,
    SYS_sched_setparam: 154,
    SYS_sched_getparam: 155,
    SYS_sched_setscheduler: 156,
    SYS_sched_getscheduler: 157,
    SYS_sched_yield: 158,
    SYS_sched_get_priority_max: 159,
    SYS_sched_get_priority_min: 160,
    SYS_sched_rr_get_interval: 161,
    SYS_nanosleep: 162,
    SYS_mremap: 163,
    SYS_setresuid: 164,
    SYS_getresuid: 165,
    SYS_vm86: 166,
    SYS_query_module: 167,
    SYS_poll: 168,
    SYS_nfsservctl: 169,
    SYS_setresgid: 170,
    SYS_getresgid: 171,
    SYS_prctl: 172,
    SYS_rt_sigreturn: 173,
    SYS_rt_sigaction: 174,
    SYS_rt_sigprocmask: 175,
    SYS_rt_sigpending: 176,
    SYS_rt_sigtimedwait: 177,
    SYS_rt_sigqueueinfo: 178,
    SYS_rt_sigsuspend: 179,
    SYS_pread64: 180,
    SYS_pwrite64: 181,
    SYS_chown: 182,
    SYS_getcwd: 183,
    SYS_capget: 184,
    SYS_capset: 185,
    SYS_sigaltstack: 186,
    SYS_sendfile: 187,
    SYS_getpmsg: 188,
    SYS_putpmsg: 189,
    SYS_vfork: 190,
    SYS_ugetrlimit: 191,
    SYS_mmap2: 192,
    SYS_truncate64: 193,
    SYS_ftruncate64: 194,
    SYS_stat64: 195,
    SYS_lstat64: 196,
    SYS_fstat64: 197,
    SYS_lchown32: 198,
    SYS_getuid32: 199,
    SYS_getgid32: 200,
    SYS_geteuid32: 201,
    SYS_getegid32: 202,
    SYS_setreuid32: 203,
    SYS_setregid32: 204,
    SYS_getgroups32: 205,
    SYS_setgroups32: 206,
    SYS_fchown32: 207,
    SYS_setresuid32: 208,
    SYS_getresuid32: 209,
    SYS_setresgid32: 210,
    SYS_getresgid32: 211,
    SYS_chown32: 212,
    SYS_setuid32: 213,
    SYS_setgid32: 214,
    SYS_setfsuid32: 215,
    SYS_setfsgid32: 216,
    SYS_pivot_root: 217,
    SYS_mincore: 218,
    SYS_madvise: 219,
    SYS_madvise1: 219,
    SYS_getdents64: 220,
    SYS_fcntl64: 221 /* 223 is unused */,
    SYS_gettid: 224,
    SYS_readahead: 225,
    SYS_setxattr: 226,
    SYS_lsetxattr: 227,
    SYS_fsetxattr: 228,
    SYS_getxattr: 229,
    SYS_lgetxattr: 230,
    SYS_fgetxattr: 231,
    SYS_listxattr: 232,
    SYS_llistxattr: 233,
    SYS_flistxattr: 234,
    SYS_removexattr: 235,
    SYS_lremovexattr: 236,
    SYS_fremovexattr: 237,
    SYS_tkill: 238,
    SYS_sendfile64: 239,
    SYS_futex: 240,
    SYS_sched_setaffinity: 241,
    SYS_sched_getaffinity: 242,
    SYS_set_thread_area: 243,
    SYS_get_thread_area: 244,
    SYS_io_setup: 245,
    SYS_io_destroy: 246,
    SYS_io_getevents: 247,
    SYS_io_submit: 248,
    SYS_io_cancel: 249,
    SYS_fadvise64: 250 /* 251 is available for reuse (was briefly sys_set_zone_reclaim) */,
    SYS_exit_group: 252,
    SYS_lookup_dcookie: 253,
    SYS_epoll_create: 254,
    SYS_epoll_ctl: 255,
    SYS_epoll_wait: 256,
    SYS_remap_file_pages: 257,
    SYS_set_tid_address: 258,
    SYS_timer_create: 259,
    SYS_timer_settime: 260,
    SYS_timer_gettime: 261,
    SYS_timer_getoverrun: 262,
    SYS_timer_delete: 263,
    SYS_clock_settime: 264,
    SYS_clock_gettime: 265,
    SYS_clock_getres: 266,
    SYS_clock_nanosleep: 267,
    SYS_statfs64: 268,
    SYS_fstatfs64: 269,
    SYS_tgkill: 270,
    SYS_utimes: 271,
    SYS_fadvise64_64: 272,
    SYS_vserver: 273,
    SYS_mbind: 274,
    SYS_get_mempolicy: 275,
    SYS_set_mempolicy: 276,
    SYS_mq_open : 277,
    SYS_mq_unlink: 278,
    SYS_mq_timedsend: 279,
    SYS_mq_timedreceive: 280,
    SYS_mq_notify: 281,
    SYS_mq_getsetattr: 282,
    SYS_kexec_load: 283,
    SYS_waitid: 284 /* SYS_sys_setaltroot: 285 */,
    SYS_add_key: 286,
    SYS_request_key: 287,
    SYS_keyctl: 288,
    SYS_ioprio_set: 289,
    SYS_ioprio_get: 290,
    SYS_inotify_init: 291,
    SYS_inotify_add_watch: 292,
    SYS_inotify_rm_watch: 293,
    SYS_migrate_pages: 294,
    SYS_openat: 295,
    SYS_mkdirat: 296,
    SYS_mknodat: 297,
    SYS_fchownat: 298,
    SYS_futimesat: 299,
    SYS_fstatat64: 300,
    SYS_unlinkat: 301,
    SYS_renameat: 302,
    SYS_linkat: 303,
    SYS_symlinkat: 304,
    SYS_readlinkat: 305,
    SYS_fchmodat: 306,
    SYS_faccessat: 307,
    SYS_pselect6: 308,
    SYS_ppoll: 309,
    SYS_unshare: 310,
    SYS_set_robust_list: 311,
    SYS_get_robust_list: 312,
    SYS_splice: 313,
    SYS_sync_file_range: 314,
    SYS_tee: 315,
    SYS_vmsplice: 316,
    SYS_move_pages: 317,
    SYS_getcpu: 318,
    SYS_epoll_pwait: 319,
    SYS_utimensat: 320,
    SYS_signalfd: 321,
    SYS_timerfd_create: 322,
    SYS_eventfd: 323,
    SYS_fallocate: 324,
    SYS_timerfd_settime: 325,
    SYS_timerfd_gettime: 326,
    SYS_signalfd4: 327,
    SYS_eventfd2: 328,
    SYS_epoll_create1: 329,
    SYS_dup3: 330,
    SYS_pipe2: 331,
    SYS_inotify_init1: 332,
    SYS_preadv: 333,
    SYS_pwritev: 334,
    SYS_prlimit64: 340,
    SYS_name_to_handle_at: 341,
    SYS_open_by_handle_at: 342,
    SYS_clock_adjtime: 343,
    SYS_syncfs: 344,
    SYS_sendmmsg: 345,
    SYS_setns: 346,
    SYS_process_vm_readv: 347,
    SYS_process_vm_writev: 348,
    SYS_kcmp: 349,
    SYS_finit_module: 350
  };
  var SYSCALL_CODE_TO_NAME = {};
  for (var name in SYSCALL_NAME_TO_CODE) {
    SYSCALL_CODE_TO_NAME[SYSCALL_NAME_TO_CODE[name]] = name;
  }
}

for (var x in SyscallsLibrary) {
  var m = /^__syscall(\d+)$/.exec(x);
  if (!m) continue;
  var which = +m[1];
  var t = SyscallsLibrary[x];
  if (typeof t === 'string') continue;
  t = t.toString();
  var pre = '', post = '';
#if USE_PTHREADS
  pre += 'if (ENVIRONMENT_IS_PTHREAD) { return _emscripten_sync_run_in_main_thread_2({{{ cDefine("EM_PROXIED_SYSCALL") }}}, ' + which + ', varargs) }\n';
#endif
  pre += 'SYSCALLS.varargs = varargs;\n';
#if SYSCALL_DEBUG
  pre += "Module.printErr('syscall! ' + [" + which + ", '" + SYSCALL_CODE_TO_NAME[which] + "']);\n";
  pre += "var canWarn = true;\n";
  pre += "var ret = (function() {\n";
  post += "})();\n";
  post += "if (ret < 0 && canWarn) {\n";
  post += "  Module.printErr('error: syscall may have failed with ' + (-ret) + ' (' + ERRNO_MESSAGES[-ret] + ')');\n";
  post += "}\n";
  post += "Module.printErr('syscall return: ' + ret);\n";
  post += "return ret;\n";
#endif
  pre += 'try {\n';
  var handler =
  "} catch (e) {\n" +
  "  if (typeof FS === 'undefined' || !(e instanceof FS.ErrnoError)) abort(e);\n";
#if SYSCALL_DEBUG
  handler +=
  "  Module.printErr('error: syscall failed with ' + e.errno + ' (' + ERRNO_MESSAGES[e.errno] + ')');\n" +
  "  canWarn = false;\n";
#endif
  handler +=
  "  return -e.errno;\n" +
  "}\n";
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
}

#if USE_PTHREADS
// emscripten_syscall is a switch over all compiled-in syscalls, used for proxying to the main thread
var switcher =
  'function(which, varargs) {\n' +
  '  switch (which) {\n';
DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.forEach(function(func) {
  var m = /^__syscall(\d+)$/.exec(func);
  if (!m) return;
  var which = +m[1];
  switcher += '    case ' + which + ': return ___syscall' + which + '(which, varargs);\n';
});
switcher +=
  '    default: throw "surprising proxied syscall: " + which;\n' +
  '  }\n' +
  '}\n';
SyscallsLibrary.emscripten_syscall = eval('(' + switcher + ')');
#endif

mergeInto(LibraryManager.library, SyscallsLibrary);

