/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

/*
List of implemented syscalls:
  {'name': 'open', 'args': ['path', 'flags', 'varargs']},
  {'name': 'unlink', 'args': ['path']},
  {'name': 'chdir', 'args': ['path']},
  {'name': 'mknod', 'args': ['path', 'mode', 'dev']},
  {'name': 'chmod', 'args': ['path', 'mode']},
  {'name': 'access', 'args': ['path', 'amode']},
  {'name': 'rename', 'args': ['old_path', 'new_path']},
  {'name': 'mkdir', 'args': ['path', 'mode']},
  {'name': 'rmdir', 'args': ['path']},
  {'name': 'dup', 'args': ['fd']},
  {'name': 'ioctl', 'args': ['fd', 'request', 'varargs']},
  {'name': 'dup2', 'args': ['oldfd', 'newfd']},
  {'name': 'symlink', 'args': ['target', 'linkpath']},
  {'name': 'readlink', 'args': ['path', 'buf', 'bufsize']},
  {'name': 'munmap', 'args': ['addr', 'len']},
  {'name': 'fchmod', 'args': ['fd', 'mode']},
  {'name': 'fchdir', 'args': ['fd']},
  {'name': '_newselect', 'args': ['nfds', 'readfds', 'writefds', 'exceptfds', 'timeout']},
  {'name': 'msync', 'args': ['addr', 'len', 'flags']},
  {'name': 'fdatasync', 'args': ['fd']},
  {'name': 'poll', 'args': ['fds', 'nfds', 'timeout']},
  {'name': 'getcwd', 'args': ['buf', 'size']},
  {'name': 'mmap2', 'args': ['addr', 'len', 'prot', 'flags', 'fd', 'off']},
  {'name': 'truncate64', 'args': ['path', 'zero', 'low', 'high']},
  {'name': 'ftruncate64', 'args': ['fd', 'zero', 'low', 'high']},
  {'name': 'stat64', 'args': ['path', 'buf']},
  {'name': 'lstat64', 'args': ['path', 'buf']},
  {'name': 'fstat64', 'args': ['fd', 'buf']},
  {'name': 'lchown32', 'args': ['path', 'owner', 'group']},
  {'name': 'fchown32', 'args': ['fd', 'owner', 'group']},
  {'name': 'chown32', 'args': ['path', 'owner', 'group']},
  {'name': 'getdents64', 'args': ['fd', 'dirp', 'count']},
  {'name': 'fcntl64', 'args': ['fd', 'cmd', 'varargs']},
  {'name': 'statfs64', 'args': ['path', 'size', 'buf']},
  {'name': 'fstatfs64', 'args': ['fd', 'size', 'buf']},
  {'name': 'openat', 'args': ['dirfd', 'path', 'flags', 'varargs']},
  {'name': 'mkdirat', 'args': ['dirfd', 'path', 'mode']},
  {'name': 'mknodat', 'args': ['dirfd', 'path', 'mode', 'dev']},
  {'name': 'fchownat', 'args': ['dirfd', 'path', 'owner', 'group', 'flags']},
  {'name': 'fstatat64', 'args': ['dirfd', 'path', 'buf', 'flags']},
  {'name': 'unlinkat', 'args': ['dirfd', 'path', 'flags']},
  {'name': 'renameat', 'args': ['olddirfd', 'oldpath', 'newdirfd', 'newpath']},
  {'name': 'symlinkat', 'args': ['target', 'newdirfd', 'linkpath']},
  {'name': 'readlinkat', 'args': ['dirfd', 'path', 'bug', 'bufsize']},
  {'name': 'fchmodat', 'args': ['dirfd', 'path', 'mode', 'varargs']},
  {'name': 'faccessat', 'args': ['dirfd', 'path', 'amode', 'flags']},
  {'name': 'utimensat', 'args': ['dirfd', 'path', 'times', 'flags']},
  {'name': 'fallocate', 'args': ['fd', 'mode', 'off_low', 'off_high', 'len_low', 'len_high']},
  {'name': 'dup3', 'args': ['fd', 'suggestfd', 'flags']},
*/

var SyscallsLibrary = {
  $ASYNCSYSCALLS__deps: ['$PThreadFS'],
  $ASYNCSYSCALLS: {
    mappings: {},
    // global constants
    DEFAULT_POLLMASK: {{{ cDefine('POLLIN') }}} | {{{ cDefine('POLLOUT') }}},

    // global state
    umask: 0x1FF,  // S_IRWXU | S_IRWXG | S_IRWXO

    // shared utilities
    calculateAt: async function(dirfd, path, allowEmpty) {
      if (path[0] === '/') {
        return path;
      }
      // relative path
      var dir;
      if (dirfd === {{{ cDefine('AT_FDCWD') }}}) {
        dir = await PThreadFS.cwd();
      } else {
        var dirstream = await PThreadFS.getStream(dirfd);
        if (!dirstream) throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
        dir = dirstream.path;
      }
      if (path.length == 0) {
        if (!allowEmpty) {
          throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});;
        }
        return dir;
      }
      return PATH.join2(dir, path);
    },

    doStat: async function(func, path, buf) {
      try {
        var stat = await func(path);
      } catch (e) {
        if (e && e.node && PATH.normalize(path) !== PATH.normalize(PThreadFS.getPath(e.node))) {
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
    doMsync: async function(addr, stream, len, flags, offset) {
      var buffer = HEAPU8.slice(addr, addr + len);
      await PThreadFS.msync(stream, buffer, offset, len, flags);
    },
    doMkdir: async function(path, mode) {
      // remove a trailing slash, if one - /a/b/ has basename of '', but
      // we want to create b in the context of this function
      path = PATH.normalize(path);
      if (path[path.length-1] === '/') path = path.substr(0, path.length-1);
      await PThreadFS.mkdir(path, mode, 0);
      return 0;
    },
    doMknod: async function(path, mode, dev) {
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
      await PThreadFS.mknod(path, mode, dev);
      return 0;
    },
    doReadlink: async function(path, buf, bufsize) {
      if (bufsize <= 0) return -{{{ cDefine('EINVAL') }}};
      var ret = await PThreadFS.readlink(path);

      var len = Math.min(bufsize, lengthBytesUTF8(ret));
      var endChar = HEAP8[buf+len];
      stringToUTF8(ret, buf, bufsize+1);
      // readlink is one of the rare functions that write out a C string, but does never append a null to the output buffer(!)
      // stringToUTF8() always appends a null byte, so restore the character under the null byte after the write.
      HEAP8[buf+len] = endChar;

      return len;
    },
    doAccess: async function(path, amode) {
      if (amode & ~{{{ cDefine('S_IRWXO') }}}) {
        // need a valid mode
        return -{{{ cDefine('EINVAL') }}};
      }
      var node;
      var lookup = await PThreadFS.lookupPath(path, { follow: true });
      node = lookup.node;
      if (!node) {
        return -{{{ cDefine('ENOENT') }}};
      }
      var perms = '';
      if (amode & {{{ cDefine('R_OK') }}}) perms += 'r';
      if (amode & {{{ cDefine('W_OK') }}}) perms += 'w';
      if (amode & {{{ cDefine('X_OK') }}}) perms += 'x';
      let nodepermissions = await PThreadFS.nodePermissions(node, perms);
      if (perms /* otherwise, they've just passed F_OK */ && nodepermissions) {
        return -{{{ cDefine('EACCES') }}};
      }
      return 0;
    },
    doDup: async function(path, flags, suggestFD) {
      var suggest = await PThreadFS.getStream(suggestFD);
      if (suggest) await PThreadFS.close(suggest);
      let stream = await PThreadFS.open(path, flags, 0, suggestFD, suggestFD);
      return stream.fd;
    },
    doReadv: async function(stream, iov, iovcnt, offset) {
      var ret = 0;
      for (var i = 0; i < iovcnt; i++) {
        var ptr = {{{ makeGetValue('iov', 'i*8', 'i32') }}};
        var len = {{{ makeGetValue('iov', 'i*8 + 4', 'i32') }}};
        var curr = await PThreadFS.read(stream, {{{ heapAndOffset('HEAP8', 'ptr') }}}, len, offset);
        if (curr < 0) return -1;
        ret += curr;
        if (curr < len) break; // nothing more to read
      }
      return ret;
    },
    doWritev: async function(stream, iov, iovcnt, offset) {
      var ret = 0;
      for (var i = 0; i < iovcnt; i++) {
        var ptr = {{{ makeGetValue('iov', 'i*8', 'i32') }}};
        var len = {{{ makeGetValue('iov', 'i*8 + 4', 'i32') }}};
        var curr = await PThreadFS.write(stream, {{{ heapAndOffset('HEAP8', 'ptr') }}}, len, offset);
        if (curr < 0) return -1;
        ret += curr;
      }
      return ret;
    },

    // arguments handling

    varargs: undefined,

    get: function() {
#if ASSERTIONS
      assert(ASYNCSYSCALLS.varargs != undefined);
#endif
      ASYNCSYSCALLS.varargs += 4;
      var ret = {{{ makeGetValue('ASYNCSYSCALLS.varargs', '-4', 'i32') }}};
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
    getStreamFromFD: async function(fd) {
      var stream = await PThreadFS.getStream(fd);
      if (!stream) throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
#if SYSCALL_DEBUG
      err('    (stream: "' + stream.path + '")');
#endif
      return stream;
    },
    get64: function(low, high) {
#if ASSERTIONS
      if (low >= 0) assert(high === 0);
      else assert(high === -1);
#endif
      return low;
    },

    $syscallMmap2_async__deps: ['$ASYNCSYSCALLS', '$zeroMemory', '$mmapAlloc', '$PThreadFS',
    ],
    $syscallMmap2_async: async function(addr, len, prot, flags, fd, off) {
      off <<= 12; // undo pgoffset
      var ptr;
      var allocated = false;
  
      // addr argument must be page aligned if MAP_FIXED flag is set.
      if ((flags & {{{ cDefine('MAP_FIXED') }}}) !== 0 && (addr % {{{ WASM_PAGE_SIZE }}}) !== 0) {
        return -{{{ cDefine('EINVAL') }}};
      }
  
      // MAP_ANONYMOUS (aka MAP_ANON) isn't actually defined by POSIX spec,
      // but it is widely used way to allocate memory pages on Linux, BSD and Mac.
      // In this case fd argument is ignored.
      if ((flags & {{{ cDefine('MAP_ANONYMOUS') }}}) !== 0) {
        ptr = mmapAlloc(len);
        if (!ptr) return -{{{ cDefine('ENOMEM') }}};
        allocated = true;
      } else {
        var info = await PThreadFS.getStream(fd);
        if (!info) return -{{{ cDefine('EBADF') }}};
        var res = await PThreadFS.mmap(info, addr, len, off, prot, flags);
        ptr = res.ptr;
        allocated = res.allocated;
      }
  #if CAN_ADDRESS_2GB
      ptr >>>= 0;
  #endif
      ASYNCSYSCALLS.mappings[ptr] = { malloc: ptr, len: len, allocated: allocated, fd: fd, prot: prot, flags: flags, offset: off };
      return ptr;
    },
  },
  
    $syscallMunmap_async__deps: ['$ASYNCSYSCALLS', '$PThreadFS'],
    $syscallMunmap_async: async function(addr, len) {
  #if CAN_ADDRESS_2GB
      addr >>>= 0;
  #endif
      // TODO: support unmmap'ing parts of allocations
      var info = ASYNCSYSCALLS.mappings[addr];
      if (len === 0 || !info) {
        return -{{{ cDefine('EINVAL') }}};
      }
      if (len === info.len) {
        var stream = await PThreadFS.getStream(info.fd);
        if (stream) {
          if (info.prot & {{{ cDefine('PROT_WRITE') }}}) {
            await ASYNCSYSCALLS.doMsync(addr, stream, len, info.flags, info.offset);
          }
          await PThreadFS.munmap(stream);
        }
        ASYNCSYSCALLS.mappings[addr] = null;
        if (info.allocated) {
          _free(info.malloc);
        }
      }
      return 0;
    },

    // WASI
    fd_write_async: async function(fd, iov, iovcnt, pnum) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd);
      var num = await ASYNCSYSCALLS.doWritev(stream, iov, iovcnt);
      {{{ makeSetValue('pnum', 0, 'num', 'i32') }}}
      return 0;
    },
    fd_read_async: async function(fd, iov, iovcnt, pnum) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd);
      var num = await ASYNCSYSCALLS.doReadv(stream, iov, iovcnt);
      {{{ makeSetValue('pnum', 0, 'num', 'i32') }}}
      return 0;
    },
    fd_close_async: async function(fd, iov, iovcnt, pnum) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd);
      await PThreadFS.close(stream);
      return 0;
    },
    fd_pwrite_async: async function(fd, iov, iovcnt, {{{ defineI64Param('offset') }}}, pnum) {
      {{{ receiveI64ParamAsI32s('offset') }}}
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd)
      var num = await ASYNCSYSCALLS.doWritev(stream, iov, iovcnt, offset_low);
      {{{ makeSetValue('pnum', 0, 'num', 'i32') }}}
      return 0;
    },
    fd_pread_async: async function(fd, iov, iovcnt, {{{ defineI64Param('offset') }}}, pnum) {
      {{{ receiveI64ParamAsI32s('offset') }}}
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd)
      var num = await ASYNCSYSCALLS.doReadv(stream, iov, iovcnt, offset_low);
      {{{ makeSetValue('pnum', 0, 'num', 'i32') }}}
      return 0;
    },
    fd_seek_async: async function(fd, {{{ defineI64Param('offset') }}}, whence, newOffset) {
      {{{ receiveI64ParamAsI32s('offset') }}}
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd);
      var HIGH_OFFSET = 0x100000000; // 2^32
      // use an unsigned operator on low and shift high by 32-bits
      var offset = offset_high * HIGH_OFFSET + (offset_low >>> 0);
  
      var DOUBLE_LIMIT = 0x20000000000000; // 2^53
      // we also check for equality since DOUBLE_LIMIT + 1 == DOUBLE_LIMIT
      if (offset <= -DOUBLE_LIMIT || offset >= DOUBLE_LIMIT) {
        return -{{{ cDefine('EOVERFLOW') }}};
      }
  
      await PThreadFS.llseek(stream, offset, whence);
      {{{ makeSetValue('newOffset', '0', 'stream.position', 'i64') }}};
      if (stream.getdents && offset === 0 && whence === {{{ cDefine('SEEK_SET') }}}) stream.getdents = null; // reset readdir state
      return 0;
    },
    fd_fdstat_get_async: async function(fd, pbuf) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd);
      // All character devices are terminals (other things a Linux system would
      // assume is a character device, like the mouse, we have special APIs for).
      var type = stream.tty ? {{{ cDefine('__WASI_FILETYPE_CHARACTER_DEVICE') }}} :
                  PThreadFS.isDir(stream.mode) ? {{{ cDefine('__WASI_FILETYPE_DIRECTORY') }}} :
                  PThreadFS.isLink(stream.mode) ? {{{ cDefine('__WASI_FILETYPE_SYMBOLIC_LINK') }}} :
                  {{{ cDefine('__WASI_FILETYPE_REGULAR_FILE') }}};
      {{{ makeSetValue('pbuf', C_STRUCTS.__wasi_fdstat_t.fs_filetype, 'type', 'i8') }}};
      // TODO {{{ makeSetValue('pbuf', C_STRUCTS.__wasi_fdstat_t.fs_flags, '?', 'i16') }}};
      // TODO {{{ makeSetValue('pbuf', C_STRUCTS.__wasi_fdstat_t.fs_rights_base, '?', 'i64') }}};
      // TODO {{{ makeSetValue('pbuf', C_STRUCTS.__wasi_fdstat_t.fs_rights_inheriting, '?', 'i64') }}};
      return 0;
    },
    fd_sync_async: async function(fd) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd);
      if (stream.stream_ops && stream.stream_ops.fsync) {
        let res = await stream.stream_ops.fsync(stream);
        return await -res;
      }
      return 0; // TODO(rstz): Consider adding functionality here!
    },

    // Syscalls
    open_async: async function(path, flags, mode) {
      var pathname = ASYNCSYSCALLS.getStr(path);
      var stream = await PThreadFS.open(pathname, flags, mode);
      return stream.fd;
    },
    unlink_async : async function(path) {
      path = ASYNCSYSCALLS.getStr(path);
      await PThreadFS.unlink(path);
      return 0;
    },
    chdir_async : async function(path) {
      path = ASYNCSYSCALLS.getStr(path);
      await PThreadFS.chdir(path);
      return 0;
    },
    mknod_async : async function(path, mode, dev) {
      path = ASYNCSYSCALLS.getStr(path);
      return await ASYNCSYSCALLS.doMknod(path, mode, dev);
    },
    chmod_async : async function(path, mode) {
      path = ASYNCSYSCALLS.getStr(path);
      await PThreadFS.chmod(path, mode);
      return 0;
    },
    access_async : async function(path, amode) {
      path = ASYNCSYSCALLS.getStr(path);
      return await ASYNCSYSCALLS.doAccess(path, amode);
    },
    rename_async : async function(old_path, new_path) {
      old_path = ASYNCSYSCALLS.getStr(old_path);
      new_path = ASYNCSYSCALLS.getStr(new_path);
      await PThreadFS.rename(old_path, new_path);
      return 0;
    },
    mkdir_async : async function(path, mode) {
      path = ASYNCSYSCALLS.getStr(path);
      return await ASYNCSYSCALLS.doMkdir(path, mode);
    },
    rmdir_async : async function(path) {
      path = ASYNCSYSCALLS.getStr(path);
      await PThreadFS.rmdir(path);
      return 0;
    },
    dup_async : async function(fd) {
      let old = await ASYNCSYSCALLS.getStreamFromFD(fd);
      let stream = await PThreadFS.open(old.path, old.flags, 0);
      return stream.fd;
    },
    ioctl_async: async function(fd, op, varargs) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd);
      switch (op) {
        case {{{ cDefine('TCGETA') }}}:
        case {{{ cDefine('TCGETS') }}}: {
          if (!stream.tty) return -{{{ cDefine('ENOTTY') }}};
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
          var argp = ASYNCSYSCALLS.get();
          {{{ makeSetValue('argp', 0, 0, 'i32') }}};
          return 0;
        }
        case {{{ cDefine('TIOCSPGRP') }}}: {
          if (!stream.tty) return -{{{ cDefine('ENOTTY') }}};
          return -{{{ cDefine('EINVAL') }}}; // not supported
        }
        case {{{ cDefine('FIONREAD') }}}: {
          var argp = ASYNCSYSCALLS.get();
          return await PThreadFS.ioctl(stream, op, argp);
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
    },
    dup2_async : async function(oldfd, suggestFD) {
      var old = await ASYNCSYSCALLS.getStreamFromFD(oldfd);
      if (old.fd === suggestFD) return suggestFD;
      return await ASYNCSYSCALLS.doDup(old.path, old.flags, suggestFD);
    },
    symlink_async : async function(target, linkpath) {
      target = ASYNCSYSCALLS.getStr(target);
      linkpath = ASYNCSYSCALLS.getStr(linkpath);
      await PThreadFS.symlink(target, linkpath);
      return 0;
    },
    readlink_async : async function(path, buf, bufsize) {
      path = ASYNCSYSCALLS.getStr(path);
      return await ASYNCSYSCALLS.doReadlink(path, buf, bufsize);
    },
    munmap_async__deps: ['$syscallMunmap_async'],
    munmap_async : async function(addr, len) {
      return await syscallMunmap_async(addr, len);
    },
    fchmod_async : async function(fd, mode) {
      await PThreadFS.fchmod(fd, mode);
      return 0;
    },
    fchdir_async : async function(fd) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd);
      await PThreadFS.chdir(stream.path);
      return 0;
    },
    _newselect_async : async function(nfds, readfds, writefds, exceptfds, timeout) {
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
  
        var stream = await PThreadFS.getStream(fd);
        if (!stream) throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
  
        var flags = ASYNCSYSCALLS.DEFAULT_POLLMASK;
  
        if (stream.stream_ops.poll) {
          flags = await stream.stream_ops.poll(stream);
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
    msync_async: async function(addr, len, flags) {
  #if CAN_ADDRESS_2GB
      addr >>>= 0;
  #endif
      var info = ASYNCSYSCALLS.mappings[addr];
      if (!info) return 0;
      await ASYNCSYSCALLS.doMsync(addr, await PThreadFS.getStream(info.fd), len, info.flags, 0);
      return 0;
    },
    fdatasync_async : async function(fd) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd);
      //TODO(rstz): Consider implementing this, since Storage Foundation supports flush().
      return 0; 
    },
    poll_async : async function(fds, nfds, timeout) {
      var nonzero = 0;
      for (var i = 0; i < nfds; i++) {
        var pollfd = fds + {{{ C_STRUCTS.pollfd.__size__ }}} * i;
        var fd = {{{ makeGetValue('pollfd', C_STRUCTS.pollfd.fd, 'i32') }}};
        var events = {{{ makeGetValue('pollfd', C_STRUCTS.pollfd.events, 'i16') }}};
        var mask = {{{ cDefine('POLLNVAL') }}};
        var stream = await PThreadFS.getStream(fd);
        if (stream) {
          mask = ASYNCSYSCALLS.DEFAULT_POLLMASK;
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
    getcwd_async : async function(buf, size) {
      if (size === 0) return -{{{ cDefine('EINVAL') }}};
      var cwd = await PThreadFS.cwd();
      var cwdLengthInBytes = lengthBytesUTF8(cwd);
      if (size < cwdLengthInBytes + 1) return -{{{ cDefine('ERANGE') }}};
      stringToUTF8(cwd, buf, size);
      return buf;
    },
    mmap2_async__deps: ['$syscallMmap2'],
    mmap2_async: async function(addr, len, prot, flags, fd, off) {
      return await syscallMmap2(addr, len, prot, flags, fd, off);
    },
    truncate64_async: async function(path, zero, low, high) {
      path = ASYNCSYSCALLS.getStr(path);
      var length = ASYNCSYSCALLS.get64(low, high);
      await PThreadFS.truncate(path, length);
      return 0;
    },
    ftruncate64_async : async function(fd, zero, low, high) {
      var length = ASYNCSYSCALLS.get64(low, high);
      await PThreadFS.ftruncate(fd, length);
      return 0;
    },
    stat64_async : async function(path, buf) {
      path = ASYNCSYSCALLS.getStr(path);
      return await ASYNCSYSCALLS.doStat(PThreadFS.stat, path, buf);
    },
    lstat64_async : async function(path, buf) {
      path = ASYNCSYSCALLS.getStr(path);
      return await ASYNCSYSCALLS.doStat(PThreadFS.lstat, path, buf);
    },
    fstat64_async : async function(fd, buf) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd);
      return await ASYNCSYSCALLS.doStat(PThreadFS.stat, stream.path, buf);
    },
    lchown32_async : async function(path, owner, group) {
      path = ASYNCSYSCALLS.getStr(path);
      await PThreadFS.chown(path, owner, group); // XXX we ignore the 'l' aspect, and do the same as chown
      return 0;
    },
    fchown32_async: async function(fd, owner, group) {
      await PThreadFS.fchown(fd, owner, group);
      return 0;
    },
    chown32_async: async function(path, owner, group) {
      path = ASYNCSYSCALLS.getStr(path);
      await PThreadFS.chown(path, owner, group);
      return 0;
    },
    getdents64_async: async function(fd, dirp, count) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd)
      if (!stream.getdents) {
        stream.getdents = await PThreadFS.readdir(stream.path);
      }
  
      var struct_size = {{{ C_STRUCTS.dirent.__size__ }}};
      var pos = 0;
      var off = await PThreadFS.llseek(stream, 0, {{{ cDefine('SEEK_CUR') }}});
  
      var idx = Math.floor(off / struct_size);
  
      while (idx < stream.getdents.length && pos + struct_size <= count) {
        var id;
        var type;
        var name = stream.getdents[idx];
        if (name[0] === '.') {
          id = 1;
          type = 4; // DT_DIR
        } else {
          var child = await PThreadFS.lookupNode(stream.node, name);
          id = child.id;
          type = PThreadFS.isChrdev(child.mode) ? 2 :  // DT_CHR, character device.
                 PThreadFS.isDir(child.mode) ? 4 :     // DT_DIR, directory.
                 PThreadFS.isLink(child.mode) ? 10 :   // DT_LNK, symbolic link.
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
      await PThreadFS.llseek(stream, idx * struct_size, {{{ cDefine('SEEK_SET') }}});
      return pos;
    },
    fcntl64_async__deps: ['$setErrNo'],
    fcntl64_async: async function(fd, cmd, varargs) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd);
      switch (cmd) {
        case {{{ cDefine('F_DUPFD') }}}: {
          var arg = ASYNCSYSCALLS.get();
          if (arg < 0) {
            return -{{{ cDefine('EINVAL') }}};
          }
          var newStream;
          newStream = await PThreadFS.open(stream.path, stream.flags, 0, arg);
          return newStream.fd;
        }
        case {{{ cDefine('F_GETFD') }}}:
        case {{{ cDefine('F_SETFD') }}}:
          return 0;  // FD_CLOEXEC makes no sense for a single process.
        case {{{ cDefine('F_GETFL') }}}:
          return stream.flags;
        case {{{ cDefine('F_SETFL') }}}: {
          var arg = ASYNCSYSCALLS.get();
          stream.flags |= arg;
          return 0;
        }
        case {{{ cDefine('F_GETLK') }}}:
        /* case {{{ cDefine('F_GETLK64') }}}: Currently in musl F_GETLK64 has same value as F_GETLK, so omitted to avoid duplicate case blocks. If that changes, uncomment this */ {
          {{{ assert(cDefine('F_GETLK') === cDefine('F_GETLK64')), '' }}}
          var arg = ASYNCSYSCALLS.get();
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
          return -{{{ cDefine('EINVAL') }}};
        }
      }
    },
    statfs64_async: function(path, size, buf) {
      path = ASYNCSYSCALLS.getStr(path);
      // NOTE: None of the constants here are true. We're just returning safe and
      //       sane values.
      {{{ makeSetValue('buf', C_STRUCTS.statfs.f_bsize, '4096', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.statfs.f_frsize, '4096', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.statfs.f_blocks, '1000000', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.statfs.f_bfree, '500000', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.statfs.f_bavail, '500000', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.statfs.f_files, 'PThreadFS.nextInode', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.statfs.f_ffree, '1000000', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.statfs.f_fsid, '42', 'i32') }}};
      {{{ makeSetValue('buf', C_STRUCTS.statfs.f_flags, '2', 'i32') }}};  // ST_NOSUID
      {{{ makeSetValue('buf', C_STRUCTS.statfs.f_namelen, '255', 'i32') }}};
      return 0;
    },
    fstatfs64_async: async function(fd, size, buf) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd);
      return _statfs64(0, size, buf);
    },
    openat_async: async function(dirfd, path, flags, varargs) {
      path = ASYNCSYSCALLS.getStr(path);
      path = ASYNCSYSCALLS.calculateAt(dirfd, path);
      var mode = varargs ? ASYNCSYSCALLS.get() : 0;
      let stream = await PThreadFS.open(path, flags, mode);
      return stream.fd;
    },
    mkdirat_async: async function(dirfd, path, mode) {
      path = ASYNCSYSCALLS.getStr(path);
      path = ASYNCSYSCALLS.calculateAt(dirfd, path);
      return ASYNCSYSCALLS.doMkdir(path, mode);
    },
    mknodat_async: async function(dirfd, path, mode, dev) {
      path = ASYNCSYSCALLS.getStr(path);
      path = ASYNCSYSCALLS.calculateAt(dirfd, path);
      return ASYNCSYSCALLS.doMknod(path, mode, dev);
    },
    fchownat_async: async function(dirfd, path, owner, group, flags) {
      path = ASYNCSYSCALLS.getStr(path);
      path = ASYNCSYSCALLS.calculateAt(dirfd, path);
      await PThreadFS.chown(path, owner, group);
      return 0;
    },
    fstatat64_async: async function(dirfd, path, buf, flags) {
      path = ASYNCSYSCALLS.getStr(path);
      var nofollow = flags & {{{ cDefine('AT_SYMLINK_NOFOLLOW') }}};
      var allowEmpty = flags & {{{ cDefine('AT_EMPTY_PATH') }}};
      flags = flags & (~{{{ cDefine('AT_SYMLINK_NOFOLLOW') | cDefine('AT_EMPTY_PATH') }}});
      path = ASYNCSYSCALLS.calculateAt(dirfd, path, allowEmpty);
      return await ASYNCSYSCALLS.doStat(nofollow ? PThreadFS.lstat : PThreadFS.stat, path, buf);
    },
    unlinkat_async: async function(dirfd, path, flags) {
      path = ASYNCSYSCALLS.getStr(path);
      path = ASYNCSYSCALLS.calculateAt(dirfd, path);
      if (flags === 0) {
        await PThreadFS.unlink(path);
      } else if (flags === {{{ cDefine('AT_REMOVEDIR') }}}) {
        await PThreadFS.rmdir(path);
      } else {
        abort('Invalid flags passed to unlinkat');
      }
      return 0;
    },
    renameat_async: async function(olddirfd, oldpath, newdirfd, newpath) {
      oldpath = ASYNCSYSCALLS.getStr(oldpath);
      newpath = ASYNCSYSCALLS.getStr(newpath);
      oldpath = ASYNCSYSCALLS.calculateAt(olddirfd, oldpath);
      newpath = ASYNCSYSCALLS.calculateAt(newdirfd, newpath);
      await PThreadFS.rename(oldpath, newpath);
      return 0;
    },
    symlinkat_async: async function(target, newdirfd, linkpath) {
      linkpath = ASYNCSYSCALLS.calculateAt(newdirfd, linkpath);
      await PThreadFS.symlink(target, linkpath);
      return 0;
    },
    readlinkat_async: async function(dirfd, path, buf, bufsize) {
      path = ASYNCSYSCALLS.getStr(path);
      path = ASYNCSYSCALLS.calculateAt(dirfd, path);
      return await ASYNCSYSCALLS.doReadlink(path, buf, bufsize);
    },
    fchmodat_async: async function(dirfd, path, mode, varargs) {
      path = ASYNCSYSCALLS.getStr(path);
      path = ASYNCSYSCALLS.calculateAt(dirfd, path);
      await PThreadFS.chmod(path, mode);
      return 0;
    },
    faccessat_async: async function(dirfd, path, amode, flags) {
      path = ASYNCSYSCALLS.getStr(path);
      path = ASYNCSYSCALLS.calculateAt(dirfd, path);
      return await ASYNCSYSCALLS.doAccess(path, amode);
    },
    utimensat_async: async function(dirfd, path, times, flags) {
      path = ASYNCSYSCALLS.getStr(path);
      path = ASYNCSYSCALLS.calculateAt(dirfd, path, true);
      var seconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_sec, 'i32') }}};
      var nanoseconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_nsec, 'i32') }}};
      var atime = (seconds*1000) + (nanoseconds/(1000*1000));
      times += {{{ C_STRUCTS.timespec.__size__ }}};
      seconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_sec, 'i32') }}};
      nanoseconds = {{{ makeGetValue('times', C_STRUCTS.timespec.tv_nsec, 'i32') }}};
      var mtime = (seconds*1000) + (nanoseconds/(1000*1000));
      await PThreadFS.utime(path, atime, mtime);
      return 0;
    },
    fallocate_async: async function(fd, mode, off_low, off_high, len_low, len_high) {
      var stream = await ASYNCSYSCALLS.getStreamFromFD(fd)
      var offset = ASYNCSYSCALLS.get64(off_low, off_high);
      var len = ASYNCSYSCALLS.get64(len_low, len_high);
      await PThreadFS.allocate(stream, offset, len);
      return 0;
    },
    dup3_async: async function(fd, suggestFD, flags) {
      var old = await ASYNCSYSCALLS.getStreamFromFD(fd);
      if (old.fd === suggestFD) return -{{{ cDefine('EINVAL') }}};
      return await ASYNCSYSCALLS.doDup(old.path, old.flags, suggestFD);
    },
};

function wrapSyscallFunction(x, library, isWasi) {
  if (x[0] === '$' || isJsLibraryConfigIdentifier(x)) {
    return;
  }

  var t = library[x];
  if (typeof t === 'string') return;
  t = t.toString();
  
  var isVariadic = !isWasi && t.includes(', varargs');
  var canThrow = library[x + '__nothrow'] !== true;

  var pre = '', post = '';
  if (isVariadic) {
    pre += 'ASYNCSYSCALLS.varargs = varargs;\n';
  }

  delete library[x + '__nothrow'];
  var handler = '';
  if (canThrow) {
    pre += 'try {\n';
    handler +=
    "} catch (e) {\n" +
    "  if (typeof PThreadFS === 'undefined' || !(e instanceof PThreadFS.ErrnoError)) abort(e);\n";

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

  if (pre) {
    var bodyStart = t.indexOf('{') + 1;
    t = t.substring(0, bodyStart) + pre + t.substring(bodyStart);
  }
  if (post) {
    var bodyEnd = t.lastIndexOf('}');
    t = t.substring(0, bodyEnd) + post + t.substring(bodyEnd);
  }
  library[x] = eval('(' + t + ')');
  if (!library[x + '__deps']) library[x + '__deps'] = [];
  library[x + '__deps'].push('$ASYNCSYSCALLS');
}

for (var x in SyscallsLibrary) {
  let isWasi = x.startsWith('fd_');
  wrapSyscallFunction(x, SyscallsLibrary, isWasi);
}

mergeInto(LibraryManager.library, SyscallsLibrary);