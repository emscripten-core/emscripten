/**
 * @license
 * Copyright 2021 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var SyscallWrappers = {}

let SyscallsFunctions = [
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
]

let WasiFunctions = [
  {'name': 'write', 'args': ['iovs', 'iovs_len', 'nwritten']},
  {'name': 'read', 'args': ['iovs', 'iovs_len', 'nread']},
  {'name': 'close', 'args': []},
  {'name': 'pwrite', 'args': ['iov', 'iovcnt', "{{{ defineI64Param('offset') }}}", 'pnum']},
  {'name': 'pread', 'args': ['iov', 'iovcnt', "{{{ defineI64Param('offset') }}}", 'pnum']},
  {'name': 'seek', 'args': ["{{{ defineI64Param('offset') }}}", 'whence', 'newOffset']},
  {'name': 'fdstat_get', 'args': ['pbuf']},
  {'name': 'sync', 'args': []},
]

function createWasiWrapper(name, args, wrappers) {
  let full_args = 'fd';
  if (args.length > 0) {
    full_args = full_args + ',' + args.join(',');
  }
  let full_args_with_resume = full_args + ',resume';
  let wrapper = `function(${full_args_with_resume}) {`;
  wrapper += `_fd_${name}_async(${full_args}).then((res) => {`;
  wrapper += 'wasmTable.get(resume)(res);});}'
  wrappers[`__fd_${name}_async`] = eval('(' + wrapper + ')');
  wrappers[`__fd_${name}_async__deps`] = [`fd_${name}_async`, '$ASYNCSYSCALLS', '$FSAFS'];
}

function createSyscallWrapper(name, args, wrappers) {
  let full_args = '';
  let full_args_with_resume = 'resume';
  if (args.length > 0) {
    full_args = args.join(',');
    full_args_with_resume = full_args + ', resume';
  }
  let wrapper = `function(${full_args_with_resume}) {`;
  wrapper += `_${name}_async(${full_args}).then((res) => {`;
  wrapper += 'wasmTable.get(resume)(res);});}'
  wrappers[`__sys_${name}_async`] = eval('(' + wrapper + ')');
  wrappers[`__sys_${name}_async__deps`] = [`${name}_async`, '$ASYNCSYSCALLS', '$FSAFS'];
}

for (x of WasiFunctions) {
  createWasiWrapper(x.name, x.args, SyscallWrappers);
}
for (x of SyscallsFunctions) {
  createSyscallWrapper(x.name, x.args, SyscallWrappers);
}

SyscallWrappers['init_pthreadfs'] = function (resume) {
  var FSNode = /** @constructor */ function(parent, name, mode, rdev) {
    if (!parent) {
      parent = this;  // root node sets parent to itself
    }
    this.parent = parent;
    this.mount = parent.mount;
    this.mounted = null;
    this.id = PThreadFS.nextInode++;
    this.name = name;
    this.mode = mode;
    this.node_ops = {};
    this.stream_ops = {};
    this.rdev = rdev;
  };
  var readMode = 292/*{{{ cDefine("S_IRUGO") }}}*/ | 73/*{{{ cDefine("S_IXUGO") }}}*/;
  var writeMode = 146/*{{{ cDefine("S_IWUGO") }}}*/;
  Object.defineProperties(FSNode.prototype, {
   read: {
    get: /** @this{FSNode} */function() {
     return (this.mode & readMode) === readMode;
    },
    set: /** @this{FSNode} */function(val) {
     val ? this.mode |= readMode : this.mode &= ~readMode;
    }
   },
   write: {
    get: /** @this{FSNode} */function() {
     return (this.mode & writeMode) === writeMode;
    },
    set: /** @this{FSNode} */function(val) {
     val ? this.mode |= writeMode : this.mode &= ~writeMode;
    }
   },
   isFolder: {
    get: /** @this{FSNode} */function() {
     return PThreadFS.isDir(this.mode);
    }
   },
   isDevice: {
    get: /** @this{FSNode} */function() {
     return PThreadFS.isChrdev(this.mode);
    }
   }
  });
  PThreadFS.FSNode = FSNode;

  PThreadFS.staticInit().then(async ()=> {
    await PThreadFS.init();
    PThreadFS.ignorePermissions = false;
    wasmTable.get(resume)();
  });
}

SyscallWrappers['init_fsafs'] = function(resume) {
  PThreadFS.mkdir('/filesystemaccess').then(async () => {
    await PThreadFS.mount(FSAFS, { root: '.' }, '/filesystemaccess');
    wasmTable.get(resume)();
  });
}

SyscallWrappers['init_sfafs'] = function(resume) {
  PThreadFS.mkdir('/sfa').then(async () => {
    await PThreadFS.mount(SFAFS, { root: '.' }, '/sfa');

    // Storage Foundation requires explicit capacity allocations.
    if (storageFoundation.requestCapacity) {
      await storageFoundation.requestCapacity(1024*1024*100);
    }
    // Delete all old files.
    let files = await storageFoundation.getAll();
    for (file of files) {
      await storageFoundation.delete(file);
    }
    wasmTable.get(resume)();
  });
}

mergeInto(LibraryManager.library, SyscallWrappers);/**
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

mergeInto(LibraryManager.library, SyscallsLibrary);/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

 mergeInto(LibraryManager.library, {
  $PThreadFS__deps: ['$getRandomDevice', '$PATH', '$PATH_FS', '$TTY_ASYNC', '$MEMFS_ASYNC', 
#if ASSERTIONS
    '$ERRNO_MESSAGES', '$ERRNO_CODES',
#endif
    ],
  $PThreadFS: {
    root: null,
    mounts: [],
    devices: {},
    streams: [],
    nextInode: 1,
    nameTable: null,
    currentPath: '/',
    initialized: false,
    // Whether we are currently ignoring permissions. Useful when preparing the
    // filesystem and creating files inside read-only folders.
    // This is set to false during `preInit`, allowing you to modify the
    // filesystem freely up until that point (e.g. during `preRun`).
    ignorePermissions: true,
    trackingDelegate: {},
    tracking: {
      openFlags: {
        READ: 1 << 0,
        WRITE: 1 << 1
      }
    },
    ErrnoError: null, // set during init
    genericErrors: {},
    filesystems: null,
    syncFSRequests: 0, // we warn if there are multiple in flight at once

    //
    // paths
    //
    lookupPath: async function(path, opts) {
      path = PATH_FS.resolve(PThreadFS.cwd(), path);
      opts = opts || {};

      if (!path) return { path: '', node: null };

      var defaults = {
        follow_mount: true,
        recurse_count: 0
      };
      for (var key in defaults) {
        if (opts[key] === undefined) {
          opts[key] = defaults[key];
        }
      }

      if (opts.recurse_count > 8) {  // max recursive lookup of 8
        throw new PThreadFS.ErrnoError({{{ cDefine('ELOOP') }}});
      }

      // split the path
      var parts = PATH.normalizeArray(path.split('/').filter(function(p) {
        return !!p;
      }), false);

      // start at the root
      var current = PThreadFS.root;
      var current_path = '/';

      for (var i = 0; i < parts.length; i++) {
        var islast = (i === parts.length-1);
        if (islast && opts.parent) {
          // stop resolving
          break;
        }

        current = await PThreadFS.lookupNode(current, parts[i]);
        current_path = PATH.join2(current_path, parts[i]);

        // jump to the mount's root node if this is a mountpoint
        if (PThreadFS.isMountpoint(current)) {
          if (!islast || (islast && opts.follow_mount)) {
            current = current.mounted.root;
          }
        }

        // by default, lookupPath will not follow a symlink if it is the final path component.
        // setting opts.follow = true will override this behavior.
        if (!islast || opts.follow) {
          var count = 0;
          while (PThreadFS.isLink(current.mode)) {
            var link = await PThreadFS.readlink(current_path);
            current_path = PATH_FS.resolve(PATH.dirname(current_path), link);

            var lookup = await PThreadFS.lookupPath(current_path, { recurse_count: opts.recurse_count });
            current = lookup.node;

            if (count++ > 40) {  // limit max consecutive symlinks to 40 (SYMLOOP_MAX).
              throw new PThreadFS.ErrnoError({{{ cDefine('ELOOP') }}});
            }
          }
        }
      }

      return { path: current_path, node: current };
    },
    getPath: function(node) {
      var path;
      while (true) {
        if (PThreadFS.isRoot(node)) {
          var mount = node.mount.mountpoint;
          if (!path) return mount;
          return mount[mount.length-1] !== '/' ? mount + '/' + path : mount + path;
        }
        path = path ? node.name + '/' + path : node.name;
        node = node.parent;
      }
    },

    //
    // nodes
    //
    hashName: function(parentid, name) {
      var hash = 0;

#if CASE_INSENSITIVE_FS
      name = name.toLowerCase();
#endif

      for (var i = 0; i < name.length; i++) {
        hash = ((hash << 5) - hash + name.charCodeAt(i)) | 0;
      }
      return ((parentid + hash) >>> 0) % PThreadFS.nameTable.length;
    },
    hashAddNode: function(node) {
      var hash = PThreadFS.hashName(node.parent.id, node.name);
      node.name_next = PThreadFS.nameTable[hash];
      PThreadFS.nameTable[hash] = node;
    },
    hashRemoveNode: function(node) {
      var hash = PThreadFS.hashName(node.parent.id, node.name);
      if (PThreadFS.nameTable[hash] === node) {
        PThreadFS.nameTable[hash] = node.name_next;
      } else {
        var current = PThreadFS.nameTable[hash];
        while (current) {
          if (current.name_next === node) {
            current.name_next = node.name_next;
            break;
          }
          current = current.name_next;
        }
      }
    },
    lookupNode: async function(parent, name) {
      var errCode = PThreadFS.mayLookup(parent);
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode, parent);
      }
      var hash = PThreadFS.hashName(parent.id, name);
#if CASE_INSENSITIVE_FS
      name = name.toLowerCase();
#endif
      for (var node = PThreadFS.nameTable[hash]; node; node = node.name_next) {
        var nodeName = node.name;
#if CASE_INSENSITIVE_FS
        nodeName = nodeName.toLowerCase();
#endif
        if (node.parent.id === parent.id && nodeName === name) {
          return node;
        }
      }
      // if we failed to find it in the cache, call into the VFS
      return await PThreadFS.lookup(parent, name);
    },
    createNode: function(parent, name, mode, rdev) {
#if ASSERTIONS
      assert(typeof parent === 'object')
#endif
      var node = new PThreadFS.FSNode(parent, name, mode, rdev);

      PThreadFS.hashAddNode(node);

      return node;
    },
    destroyNode: function(node) {
      PThreadFS.hashRemoveNode(node);
    },
    isRoot: function(node) {
      return node === node.parent;
    },
    isMountpoint: function(node) {
      return !!node.mounted;
    },
    isFile: function(mode) {
      return (mode & {{{ cDefine('S_IFMT') }}}) === {{{ cDefine('S_IFREG') }}};
    },
    isDir: function(mode) {
      return (mode & {{{ cDefine('S_IFMT') }}}) === {{{ cDefine('S_IFDIR') }}};
    },
    isLink: function(mode) {
      return (mode & {{{ cDefine('S_IFMT') }}}) === {{{ cDefine('S_IFLNK') }}};
    },
    isChrdev: function(mode) {
      return (mode & {{{ cDefine('S_IFMT') }}}) === {{{ cDefine('S_IFCHR') }}};
    },
    isBlkdev: function(mode) {
      return (mode & {{{ cDefine('S_IFMT') }}}) === {{{ cDefine('S_IFBLK') }}};
    },
    isFIFO: function(mode) {
      return (mode & {{{ cDefine('S_IFMT') }}}) === {{{ cDefine('S_IFIFO') }}};
    },
    isSocket: function(mode) {
      return (mode & {{{ cDefine('S_IFSOCK') }}}) === {{{ cDefine('S_IFSOCK') }}};
    },

    //
    // permissions
    //
    flagModes: {
      // Extra quotes used here on the keys to this object otherwise jsifier will
      // erase them in the process of reading and then writing the JS library
      // code.
      '"r"': {{{ cDefine('O_RDONLY') }}},
      '"r+"': {{{ cDefine('O_RDWR') }}},
      '"w"': {{{ cDefine('O_TRUNC') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_WRONLY') }}},
      '"w+"': {{{ cDefine('O_TRUNC') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_RDWR') }}},
      '"a"': {{{ cDefine('O_APPEND') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_WRONLY') }}},
      '"a+"': {{{ cDefine('O_APPEND') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_RDWR') }}},
    },
    // convert the 'r', 'r+', etc. to it's corresponding set of O_* flags
    modeStringToFlags: function(str) {
      var flags = PThreadFS.flagModes[str];
      if (typeof flags === 'undefined') {
        throw new Error('Unknown file open mode: ' + str);
      }
      return flags;
    },
    // convert O_* bitmask to a string for nodePermissions
    flagsToPermissionString: function(flag) {
      var perms = ['r', 'w', 'rw'][flag & 3];
      if ((flag & {{{ cDefine('O_TRUNC') }}})) {
        perms += 'w';
      }
      return perms;
    },
    nodePermissions: function(node, perms) {
      if (PThreadFS.ignorePermissions) {
        return 0;
      }
      // return 0 if any user, group or owner bits are set.
      if (perms.includes('r') && !(node.mode & {{{ cDefine('S_IRUGO') }}})) {
        return {{{ cDefine('EACCES') }}};
      } else if (perms.includes('w') && !(node.mode & {{{ cDefine('S_IWUGO') }}})) {
        return {{{ cDefine('EACCES') }}};
      } else if (perms.includes('x') && !(node.mode & {{{ cDefine('S_IXUGO') }}})) {
        return {{{ cDefine('EACCES') }}};
      }
      return 0;
    },
    mayLookup: function(dir) {
      var errCode = PThreadFS.nodePermissions(dir, 'x');
      if (errCode) return errCode;
      if (!dir.node_ops.lookup) return {{{ cDefine('EACCES') }}};
      return 0;
    },
    mayCreate: async function(dir, name) {
      try {
        var node = await PThreadFS.lookupNode(dir, name);
        return {{{ cDefine('EEXIST') }}};
      } catch (e) {
      }
      return PThreadFS.nodePermissions(dir, 'wx');
    },
    mayDelete: async function(dir, name, isdir) {
      var node;
      try {
        node = await PThreadFS.lookupNode(dir, name);
      } catch (e) {
        return e.errno;
      }
      var errCode = PThreadFS.nodePermissions(dir, 'wx');
      if (errCode) {
        return errCode;
      }
      if (isdir) {
        if (!PThreadFS.isDir(node.mode)) {
          return {{{ cDefine('ENOTDIR') }}};
        }
        if (PThreadFS.isRoot(node) || PThreadFS.getPath(node) === PThreadFS.cwd()) {
          return {{{ cDefine('EBUSY') }}};
        }
      } else {
        if (PThreadFS.isDir(node.mode)) {
          return {{{ cDefine('EISDIR') }}};
        }
      }
      return 0;
    },
    mayOpen: function(node, flags) {
      if (!node) {
        return {{{ cDefine('ENOENT') }}};
      }
      if (PThreadFS.isLink(node.mode)) {
        return {{{ cDefine('ELOOP') }}};
      } else if (PThreadFS.isDir(node.mode)) {
        if (PThreadFS.flagsToPermissionString(flags) !== 'r' || // opening for write
            (flags & {{{ cDefine('O_TRUNC') }}})) { // TODO: check for O_SEARCH? (== search for dir only)
          return {{{ cDefine('EISDIR') }}};
        }
      }
      return PThreadFS.nodePermissions(node, PThreadFS.flagsToPermissionString(flags));
    },

    //
    // streams
    //
    MAX_OPEN_FDS: 4096,
    nextfd: function(fd_start, fd_end) {
      fd_start = fd_start || 0;
      fd_end = fd_end || PThreadFS.MAX_OPEN_FDS;
      for (var fd = fd_start; fd <= fd_end; fd++) {
        if (!PThreadFS.streams[fd]) {
          return fd;
        }
      }
      throw new PThreadFS.ErrnoError({{{ cDefine('EMFILE') }}});
    },
    getStream: function(fd) {
      return PThreadFS.streams[fd];
    },
    // TODO parameterize this function such that a stream
    // object isn't directly passed in. not possible until
    // SOCKFS is completed.
    createStream: function(stream, fd_start, fd_end) {
      if (!PThreadFS.FSStream) {
        PThreadFS.FSStream = /** @constructor */ function(){};
        PThreadFS.FSStream.prototype = {
          object: {
            get: function() { return this.node; },
            set: function(val) { this.node = val; }
          },
          isRead: {
            get: function() { return (this.flags & {{{ cDefine('O_ACCMODE') }}}) !== {{{ cDefine('O_WRONLY') }}}; }
          },
          isWrite: {
            get: function() { return (this.flags & {{{ cDefine('O_ACCMODE') }}}) !== {{{ cDefine('O_RDONLY') }}}; }
          },
          isAppend: {
            get: function() { return (this.flags & {{{ cDefine('O_APPEND') }}}); }
          }
        };
      }
      // clone it, so we can return an instance of FSStream
      var newStream = new PThreadFS.FSStream();
      for (var p in stream) {
        newStream[p] = stream[p];
      }
      stream = newStream;
      var fd = PThreadFS.nextfd(fd_start, fd_end);
      stream.fd = fd;
      PThreadFS.streams[fd] = stream;
      return stream;
    },
    closeStream: function(fd) {
      PThreadFS.streams[fd] = null;
    },

    //
    // devices
    //
    // each character device consists of a device id + stream operations.
    // when a character device node is created (e.g. /dev/stdin) it is
    // assigned a device id that lets us map back to the actual device.
    // by default, each character device stream (e.g. _stdin) uses chrdev_stream_ops.
    // however, once opened, the stream's operations are overridden with
    // the operations of the device its underlying node maps back to.
    chrdev_stream_ops: {
      open: function(stream) {
        var device = PThreadFS.getDevice(stream.node.rdev);
        // override node's stream ops with the device's
        stream.stream_ops = device.stream_ops;
        // forward the open call
        if (stream.stream_ops.open) {
          stream.stream_ops.open(stream);
        }
      },
      llseek: function() {
        throw new PThreadFS.ErrnoError({{{ cDefine('ESPIPE') }}});
      }
    },
    major: function(dev) {
      return ((dev) >> 8);
    },
    minor: function(dev) {
      return ((dev) & 0xff);
    },
    makedev: function(ma, mi) {
      return ((ma) << 8 | (mi));
    },
    registerDevice: function(dev, ops) {
      PThreadFS.devices[dev] = { stream_ops: ops };
    },
    getDevice: function(dev) {
      return PThreadFS.devices[dev];
    },

    //
    // core
    //
    getMounts: function(mount) {
      var mounts = [];
      var check = [mount];

      while (check.length) {
        var m = check.pop();

        mounts.push(m);

        check.push.apply(check, m.mounts);
      }

      return mounts;
    },
    syncfs: function(populate, callback) {
      if (typeof(populate) === 'function') {
        callback = populate;
        populate = false;
      }

      PThreadFS.syncFSRequests++;

      if (PThreadFS.syncFSRequests > 1) {
        err('warning: ' + PThreadFS.syncFSRequests + ' PThreadFS.syncfs operations in flight at once, probably just doing extra work');
      }

      var mounts = PThreadFS.getMounts(PThreadFS.root.mount);
      var completed = 0;

      function doCallback(errCode) {
#if ASSERTIONS
        assert(PThreadFS.syncFSRequests > 0);
#endif
        PThreadFS.syncFSRequests--;
        return callback(errCode);
      }

      function done(errCode) {
        if (errCode) {
          if (!done.errored) {
            done.errored = true;
            return doCallback(errCode);
          }
          return;
        }
        if (++completed >= mounts.length) {
          doCallback(null);
        }
      };

      // sync all mounts
      mounts.forEach(function (mount) {
        if (!mount.type.syncfs) {
          return done(null);
        }
        mount.type.syncfs(mount, populate, done);
      });
    },
    mount: async function(type, opts, mountpoint) {
#if ASSERTIONS
      if (typeof type === 'string') {
        // The filesystem was not included, and instead we have an error
        // message stored in the variable.
        throw type;
      }
#endif
      var root = mountpoint === '/';
      var pseudo = !mountpoint;
      var node;

      if (root && PThreadFS.root) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBUSY') }}});
      } else if (!root && !pseudo) {
        var lookup = await PThreadFS.lookupPath(mountpoint, { follow_mount: false });

        mountpoint = lookup.path;  // use the absolute path
        node = lookup.node;

        if (PThreadFS.isMountpoint(node)) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EBUSY') }}});
        }

        if (!PThreadFS.isDir(node.mode)) {
          throw new PThreadFS.ErrnoError({{{ cDefine('ENOTDIR') }}});
        }
      }

      var mount = {
        type: type,
        opts: opts,
        mountpoint: mountpoint,
        mounts: []
      };

      // create a root node for the fs
      var mountRoot = await type.mount(mount);
      mountRoot.mount = mount;
      mount.root = mountRoot;

      if (root) {
        PThreadFS.root = mountRoot;
      } else if (node) {
        // set as a mountpoint
        node.mounted = mount;

        // add the new mount to the current mount's children
        if (node.mount) {
          node.mount.mounts.push(mount);
        }
      }

      return mountRoot;
    },
    unmount: async function (mountpoint) {
      var lookup = await PThreadFS.lookupPath(mountpoint, { follow_mount: false });

      if (!PThreadFS.isMountpoint(lookup.node)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }

      // destroy the nodes for this mount, and all its child mounts
      var node = lookup.node;
      var mount = node.mounted;
      var mounts = PThreadFS.getMounts(mount);

      Object.keys(PThreadFS.nameTable).forEach(function (hash) {
        var current = PThreadFS.nameTable[hash];

        while (current) {
          var next = current.name_next;

          if (mounts.includes(current.mount)) {
            PThreadFS.destroyNode(current);
          }

          current = next;
        }
      });

      // no longer a mountpoint
      node.mounted = null;

      // remove this mount from the child mounts
      var idx = node.mount.mounts.indexOf(mount);
#if ASSERTIONS
      assert(idx !== -1);
#endif
      node.mount.mounts.splice(idx, 1);
    },
    lookup: async function(parent, name) {
      return await parent.node_ops.lookup(parent, name);
    },
    // generic function for all node creation
    mknod: async function(path, mode, dev) {
      var lookup = await PThreadFS.lookupPath(path, { parent: true });
      var parent = lookup.node;
      var name = PATH.basename(path);
      if (!name || name === '.' || name === '..') {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      var errCode = await PThreadFS.mayCreate(parent, name);
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      if (!parent.node_ops.mknod) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      return await parent.node_ops.mknod(parent, name, mode, dev);
    },
    // helpers to create specific types of nodes
    create: async function(path, mode) {
      mode = mode !== undefined ? mode : 438 /* 0666 */;
      mode &= {{{ cDefine('S_IALLUGO') }}};
      mode |= {{{ cDefine('S_IFREG') }}};
      return await PThreadFS.mknod(path, mode, 0);
    },
    mkdir: async function(path, mode) {
      mode = mode !== undefined ? mode : 511 /* 0777 */;
      mode &= {{{ cDefine('S_IRWXUGO') }}} | {{{ cDefine('S_ISVTX') }}};
      mode |= {{{ cDefine('S_IFDIR') }}};
      return await PThreadFS.mknod(path, mode, 0);
    },
    // Creates a whole directory tree chain if it doesn't yet exist
    mkdirTree: async function(path, mode) {
      var dirs = path.split('/');
      var d = '';
      for (var i = 0; i < dirs.length; ++i) {
        if (!dirs[i]) continue;
        d += '/' + dirs[i];
        try {
          await PThreadFS.mkdir(d, mode);
        } catch(e) {
          if (e.errno != {{{ cDefine('EEXIST') }}}) throw e;
        }
      }
    },
    mkdev: async function(path, mode, dev) {
      if (typeof(dev) === 'undefined') {
        dev = mode;
        mode = 438 /* 0666 */;
      }
      mode |= {{{ cDefine('S_IFCHR') }}};
      return await PThreadFS.mknod(path, mode, dev);
    },
    symlink: async function(oldpath, newpath) {
      if (!PATH_FS.resolve(oldpath)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      var lookup = await PThreadFS.lookupPath(newpath, { parent: true });
      var parent = lookup.node;
      if (!parent) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      var newname = PATH.basename(newpath);
      var errCode = await PThreadFS.mayCreate(parent, newname);
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      if (!parent.node_ops.symlink) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      return parent.node_ops.symlink(parent, newname, oldpath);
    },
    rename: async function(old_path, new_path) {
      var old_dirname = PATH.dirname(old_path);
      var new_dirname = PATH.dirname(new_path);
      var old_name = PATH.basename(old_path);
      var new_name = PATH.basename(new_path);
      // parents must exist
      var lookup, old_dir, new_dir;

      // let the errors from non existant directories percolate up
      lookup = await PThreadFS.lookupPath(old_path, { parent: true });
      old_dir = lookup.node;
      lookup = await PThreadFS.lookupPath(new_path, { parent: true });
      new_dir = lookup.node;

      if (!old_dir || !new_dir) throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      // need to be part of the same mount
      if (old_dir.mount !== new_dir.mount) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EXDEV') }}});
      }
      // source must exist
      var old_node = await PThreadFS.lookupNode(old_dir, old_name);
      // old path should not be an ancestor of the new path
      var relative = PATH_FS.relative(old_path, new_dirname);
      if (relative.charAt(0) !== '.') {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      // new path should not be an ancestor of the old path
      relative = PATH_FS.relative(new_path, old_dirname);
      if (relative.charAt(0) !== '.') {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOTEMPTY') }}});
      }
      // see if the new path already exists
      var new_node;
      try {
        new_node = await PThreadFS.lookupNode(new_dir, new_name);
      } catch (e) {
        // not fatal
      }
      // early out if nothing needs to change
      if (old_node === new_node) {
        return;
      }
      // we'll need to delete the old entry
      var isdir = PThreadFS.isDir(old_node.mode);
      var errCode = await PThreadFS.mayDelete(old_dir, old_name, isdir);
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      // need delete permissions if we'll be overwriting.
      // need create permissions if new doesn't already exist.
      errCode = new_node ?
        await PThreadFS.mayDelete(new_dir, new_name, isdir) :
        await PThreadFS.mayCreate(new_dir, new_name);
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      if (!old_dir.node_ops.rename) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      if (PThreadFS.isMountpoint(old_node) || (new_node && PThreadFS.isMountpoint(new_node))) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBUSY') }}});
      }
      // if we are going to change the parent, check write permissions
      if (new_dir !== old_dir) {
        errCode = PThreadFS.nodePermissions(old_dir, 'w');
        if (errCode) {
          throw new PThreadFS.ErrnoError(errCode);
        }
      }
      try {
        if (PThreadFS.trackingDelegate['willMovePath']) {
          PThreadFS.trackingDelegate['willMovePath'](old_path, new_path);
        }
      } catch(e) {
        err("PThreadFS.trackingDelegate['willMovePath']('"+old_path+"', '"+new_path+"') threw an exception: " + e.message);
      }
      // remove the node from the lookup hash
      PThreadFS.hashRemoveNode(old_node);
      // do the underlying fs rename
      try {
        await old_dir.node_ops.rename(old_node, new_dir, new_name);
      } catch (e) {
        throw e;
      } finally {
        // add the node back to the hash (in case node_ops.rename
        // changed its name)
        PThreadFS.hashAddNode(old_node);
      }
      try {
        if (PThreadFS.trackingDelegate['onMovePath']) PThreadFS.trackingDelegate['onMovePath'](old_path, new_path);
      } catch(e) {
        err("PThreadFS.trackingDelegate['onMovePath']('"+old_path+"', '"+new_path+"') threw an exception: " + e.message);
      }
    },
    rmdir: async function(path) {
      var lookup = await PThreadFS.lookupPath(path, { parent: true });
      var parent = lookup.node;
      var name = PATH.basename(path);
      var node = await PThreadFS.lookupNode(parent, name);
      var errCode = await PThreadFS.mayDelete(parent, name, true);
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      if (!parent.node_ops.rmdir) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      if (PThreadFS.isMountpoint(node)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBUSY') }}});
      }
      try {
        if (PThreadFS.trackingDelegate['willDeletePath']) {
          PThreadFS.trackingDelegate['willDeletePath'](path);
        }
      } catch(e) {
        err("PThreadFS.trackingDelegate['willDeletePath']('"+path+"') threw an exception: " + e.message);
      }
      await parent.node_ops.rmdir(parent, name);
      PThreadFS.destroyNode(node);
      try {
        if (PThreadFS.trackingDelegate['onDeletePath']) PThreadFS.trackingDelegate['onDeletePath'](path);
      } catch(e) {
        err("PThreadFS.trackingDelegate['onDeletePath']('"+path+"') threw an exception: " + e.message);
      }
    },
    readdir: async function(path) {
      var lookup = await PThreadFS.lookupPath(path, { follow: true });
      var node = lookup.node;
      if (!node.node_ops.readdir) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOTDIR') }}});
      }
      return await node.node_ops.readdir(node);
    },
    unlink: async function(path) {
      var lookup = await PThreadFS.lookupPath(path, { parent: true });
      var parent = lookup.node;
      var name = PATH.basename(path);
      var node = await PThreadFS.lookupNode(parent, name);
      var errCode = await PThreadFS.mayDelete(parent, name, false);
      if (errCode) {
        // According to POSIX, we should map EISDIR to EPERM, but
        // we instead do what Linux does (and we must, as we use
        // the musl linux libc).
        throw new PThreadFS.ErrnoError(errCode);
      }
      if (!parent.node_ops.unlink) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      if (PThreadFS.isMountpoint(node)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBUSY') }}});
      }
      try {
        if (PThreadFS.trackingDelegate['willDeletePath']) {
          PThreadFS.trackingDelegate['willDeletePath'](path);
        }
      } catch(e) {
        err("PThreadFS.trackingDelegate['willDeletePath']('"+path+"') threw an exception: " + e.message);
      }
      await parent.node_ops.unlink(parent, name);
      PThreadFS.destroyNode(node);
      try {
        if (PThreadFS.trackingDelegate['onDeletePath']) PThreadFS.trackingDelegate['onDeletePath'](path);
      } catch(e) {
        err("PThreadFS.trackingDelegate['onDeletePath']('"+path+"') threw an exception: " + e.message);
      }
    },
    readlink: async function(path) {
      var lookup = await PThreadFS.lookupPath(path);
      var link = lookup.node;
      if (!link) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      if (!link.node_ops.readlink) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      return PATH_FS.resolve(PThreadFS.getPath(link.parent), link.node_ops.readlink(link));
    },
    stat: async function(path, dontFollow) {
      var lookup = await PThreadFS.lookupPath(path, { follow: !dontFollow });
      var node = lookup.node;
      if (!node) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      if (!node.node_ops.getattr) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      return await node.node_ops.getattr(node);
    },
    lstat: async function(path) {
      return await PThreadFS.stat(path, true);
    },
    chmod: async function(path, mode, dontFollow) {
      var node;
      if (typeof path === 'string') {
        var lookup = await PThreadFS.lookupPath(path, { follow: !dontFollow });
        node = lookup.node;
      } else {
        node = path;
      }
      if (!node.node_ops.setattr) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      await node.node_ops.setattr(node, {
        mode: (mode & {{{ cDefine('S_IALLUGO') }}}) | (node.mode & ~{{{ cDefine('S_IALLUGO') }}}),
        timestamp: Date.now()
      });
    },
    lchmod: async function(path, mode) {
      await PThreadFS.chmod(path, mode, true);
    },
    fchmod: async function(fd, mode) {
      var stream = PThreadFS.getStream(fd);
      if (!stream) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      await PThreadFS.chmod(stream.node, mode);
    },
    chown: async function(path, uid, gid, dontFollow) {
      var node;
      if (typeof path === 'string') {
        var lookup = await PThreadFS.lookupPath(path, { follow: !dontFollow });
        node = lookup.node;
      } else {
        node = path;
      }
      if (!node.node_ops.setattr) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      await node.node_ops.setattr(node, {
        timestamp: Date.now()
        // we ignore the uid / gid for now
      });
    },
    lchown: async function(path, uid, gid) {
      await PThreadFS.chown(path, uid, gid, true);
    },
    fchown: async function(fd, uid, gid) {
      var stream = PThreadFS.getStream(fd);
      if (!stream) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      await PThreadFS.chown(stream.node, uid, gid);
    },
    truncate: async function(path, len) {
      if (len < 0) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      var node;
      if (typeof path === 'string') {
        var lookup = await PThreadFS.lookupPath(path, { follow: true });
        node = lookup.node;
      } else {
        node = path;
      }
      if (!node.node_ops.setattr) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      if (PThreadFS.isDir(node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EISDIR') }}});
      }
      if (!PThreadFS.isFile(node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      var errCode = PThreadFS.nodePermissions(node, 'w');
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      await node.node_ops.setattr(node, {
        size: len,
        timestamp: Date.now()
      });
   },
    ftruncate: async function(fd, len) {
      var stream = PThreadFS.getStream(fd);
      if (!stream) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_RDONLY')}}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      await PThreadFS.truncate(stream.node, len);
    },
    utime: async function(path, atime, mtime) {
      var lookup = await PThreadFS.lookupPath(path, { follow: true });
      var node = lookup.node;
      await node.node_ops.setattr(node, {
        timestamp: Math.max(atime, mtime)
      });
   },
    open: async function(path, flags, mode, fd_start, fd_end) {
      if (path === "") {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      flags = typeof flags === 'string' ? PThreadFS.modeStringToFlags(flags) : flags;
      mode = typeof mode === 'undefined' ? 438 /* 0666 */ : mode;
      if ((flags & {{{ cDefine('O_CREAT') }}})) {
        mode = (mode & {{{ cDefine('S_IALLUGO') }}}) | {{{ cDefine('S_IFREG') }}};
      } else {
        mode = 0;
      }
      var node;
      if (typeof path === 'object') {
        node = path;
      } else {
        path = PATH.normalize(path);
        try {
          var lookup = await PThreadFS.lookupPath(path, {
            follow: !(flags & {{{ cDefine('O_NOFOLLOW') }}})
          });
          node = lookup.node;
        } catch (e) {
          // ignore
        }
      }
      // perhaps we need to create the node
      var created = false;
      if ((flags & {{{ cDefine('O_CREAT') }}})) {
        if (node) {
          // if O_CREAT and O_EXCL are set, error out if the node already exists
          if ((flags & {{{ cDefine('O_EXCL') }}})) {
            throw new PThreadFS.ErrnoError({{{ cDefine('EEXIST') }}});
          }
        } else {
          // node doesn't exist, try to create it
          node = await PThreadFS.mknod(path, mode, 0);
          created = true;
        }
      }
      if (!node) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      // can't truncate a device
      if (PThreadFS.isChrdev(node.mode)) {
        flags &= ~{{{ cDefine('O_TRUNC') }}};
      }
      // if asked only for a directory, then this must be one
      if ((flags & {{{ cDefine('O_DIRECTORY') }}}) && !PThreadFS.isDir(node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOTDIR') }}});
      }
      // check permissions, if this is not a file we just created now (it is ok to
      // create and write to a file with read-only permissions; it is read-only
      // for later use)
      if (!created) {
        var errCode = PThreadFS.mayOpen(node, flags);
        if (errCode) {
          throw new PThreadFS.ErrnoError(errCode);
        }
      }
      // do truncation if necessary
      if ((flags & {{{ cDefine('O_TRUNC')}}})) {
        await PThreadFS.truncate(node, 0);
      }
      // we've already handled these, don't pass down to the underlying vfs
      flags &= ~({{{ cDefine('O_EXCL') }}} | {{{ cDefine('O_TRUNC') }}} | {{{ cDefine('O_NOFOLLOW') }}});

      // register the stream with the filesystem
      var stream = PThreadFS.createStream({
        node: node,
        path: PThreadFS.getPath(node),  // we want the absolute path to the node
        flags: flags,
        seekable: true,
        position: 0,
        stream_ops: node.stream_ops,
        // used by the file family libc calls (fopen, fwrite, ferror, etc.)
        ungotten: [],
        error: false
      }, fd_start, fd_end);
      // call the new stream's open function
      if (stream.stream_ops.open) {
        await stream.stream_ops.open(stream);
      }
      if (Module['logReadFiles'] && !(flags & {{{ cDefine('O_WRONLY')}}})) {
        if (!PThreadFS.readFiles) PThreadFS.readFiles = {};
        if (!(path in PThreadFS.readFiles)) {
          PThreadFS.readFiles[path] = 1;
          err("PThreadFS.trackingDelegate error on read file: " + path);
        }
      }
      try {
        if (PThreadFS.trackingDelegate['onOpenFile']) {
          var trackingFlags = 0;
          if ((flags & {{{ cDefine('O_ACCMODE') }}}) !== {{{ cDefine('O_WRONLY') }}}) {
            trackingFlags |= PThreadFS.tracking.openFlags.READ;
          }
          if ((flags & {{{ cDefine('O_ACCMODE') }}}) !== {{{ cDefine('O_RDONLY') }}}) {
            trackingFlags |= PThreadFS.tracking.openFlags.WRITE;
          }
          PThreadFS.trackingDelegate['onOpenFile'](path, trackingFlags);
        }
      } catch(e) {
        err("PThreadFS.trackingDelegate['onOpenFile']('"+path+"', flags) threw an exception: " + e.message);
      }
      return stream;
    },
    close: async function(stream) {
      if (PThreadFS.isClosed(stream)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if (stream.getdents) stream.getdents = null; // free readdir state
      try {
        if (stream.stream_ops.close) {
          await stream.stream_ops.close(stream);
        }
      } catch (e) {
        throw e;
      } finally {
        PThreadFS.closeStream(stream.fd);
      }
      stream.fd = null;
    },
    isClosed: function(stream) {
      return stream.fd === null;
    },
    llseek: async function(stream, offset, whence) {
      if (PThreadFS.isClosed(stream)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if (!stream.seekable || !stream.stream_ops.llseek) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ESPIPE') }}});
      }
      if (whence != {{{ cDefine('SEEK_SET') }}} && whence != {{{ cDefine('SEEK_CUR') }}} && whence != {{{ cDefine('SEEK_END') }}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      stream.position = await stream.stream_ops.llseek(stream, offset, whence);
      stream.ungotten = [];
      return stream.position;
    },
    read: async function(stream, buffer, offset, length, position) {
#if CAN_ADDRESS_2GB
      offset >>>= 0;
#endif
      if (length < 0 || position < 0) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      if (PThreadFS.isClosed(stream)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_WRONLY')}}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if (PThreadFS.isDir(stream.node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EISDIR') }}});
      }
      if (!stream.stream_ops.read) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      var seeking = typeof position !== 'undefined';
      if (!seeking) {
        position = stream.position;
      } else if (!stream.seekable) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ESPIPE') }}});
      }
      var bytesRead = await stream.stream_ops.read(stream, buffer, offset, length, position);
      if (!seeking) stream.position += bytesRead;
      return bytesRead;
    },
    write: async function(stream, buffer, offset, length, position, canOwn) {
#if CAN_ADDRESS_2GB
      offset >>>= 0;
#endif
      if (length < 0 || position < 0) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      if (PThreadFS.isClosed(stream)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_RDONLY')}}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if (PThreadFS.isDir(stream.node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EISDIR') }}});
      }
      if (!stream.stream_ops.write) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      if (stream.seekable && stream.flags & {{{ cDefine('O_APPEND') }}}) {
        // seek to the end before writing in append mode
        await PThreadFS.llseek(stream, 0, {{{ cDefine('SEEK_END') }}});
      }
      var seeking = typeof position !== 'undefined';
      if (!seeking) {
        position = stream.position;
      } else if (!stream.seekable) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ESPIPE') }}});
      }
      var bytesWritten = await stream.stream_ops.write(stream, buffer, offset, length, position, canOwn);
      if (!seeking) stream.position += bytesWritten;
      try {
        if (stream.path && PThreadFS.trackingDelegate['onWriteToFile']) PThreadFS.trackingDelegate['onWriteToFile'](stream.path);
      } catch(e) {
        err("PThreadFS.trackingDelegate['onWriteToFile']('"+stream.path+"') threw an exception: " + e.message);
      }
      return bytesWritten;
    },
    allocate: async function(stream, offset, length) {
      if (PThreadFS.isClosed(stream)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if (offset < 0 || length <= 0) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_RDONLY')}}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if (!PThreadFS.isFile(stream.node.mode) && !PThreadFS.isDir(stream.node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENODEV') }}});
      }
      if (!stream.stream_ops.allocate) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EOPNOTSUPP') }}});
      }
      await stream.stream_ops.allocate(stream, offset, length);
   },
    mmap: async function(stream, address, length, position, prot, flags) {
#if CAN_ADDRESS_2GB
      address >>>= 0;
#endif
      // User requests writing to file (prot & PROT_WRITE != 0).
      // Checking if we have permissions to write to the file unless
      // MAP_PRIVATE flag is set. According to POSIX spec it is possible
      // to write to file opened in read-only mode with MAP_PRIVATE flag,
      // as all modifications will be visible only in the memory of
      // the current process.
      if ((prot & {{{ cDefine('PROT_WRITE') }}}) !== 0
          && (flags & {{{ cDefine('MAP_PRIVATE')}}}) === 0
          && (stream.flags & {{{ cDefine('O_ACCMODE') }}}) !== {{{ cDefine('O_RDWR')}}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EACCES') }}});
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_WRONLY')}}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EACCES') }}});
      }
      if (!stream.stream_ops.mmap) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENODEV') }}});
      }
      return await stream.stream_ops.mmap(stream, address, length, position, prot, flags);
    },
    msync: async function(stream, buffer, offset, length, mmapFlags) {
#if CAN_ADDRESS_2GB
      offset >>>= 0;
#endif
      if (!stream || !stream.stream_ops.msync) {
        return 0;
      }
      return await stream.stream_ops.msync(stream, buffer, offset, length, mmapFlags);
    },
    munmap: function(stream) {
      return 0;
    },
    ioctl: async function(stream, cmd, arg) {
      if (!stream.stream_ops.ioctl) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOTTY') }}});
      }
      return await stream.stream_ops.ioctl(stream, cmd, arg);
    },
    readFile: async function(path, opts) {
      opts = opts || {};
      opts.flags = opts.flags || {{{ cDefine('O_RDONLY') }}};
      opts.encoding = opts.encoding || 'binary';
      if (opts.encoding !== 'utf8' && opts.encoding !== 'binary') {
        throw new Error('Invalid encoding type "' + opts.encoding + '"');
      }
      var ret;
      var stream = await PThreadFS.open(path, opts.flags);
      var stat = await PThreadFS.stat(path);
      var length = stat.size;
      var buf = new Uint8Array(length);
      await PThreadFS.read(stream, buf, 0, length, 0);
      if (opts.encoding === 'utf8') {
        ret = UTF8ArrayToString(buf, 0);
      } else if (opts.encoding === 'binary') {
        ret = buf;
      }
      await PThreadFS.close(stream);
      return ret;
    },
    writeFile: async function(path, data, opts) {
      opts = opts || {};
      opts.flags = opts.flags || {{{ cDefine('O_TRUNC') | cDefine('O_CREAT') | cDefine('O_WRONLY') }}};
      var stream = await PThreadFS.open(path, opts.flags, opts.mode);
      if (typeof data === 'string') {
        var buf = new Uint8Array(lengthBytesUTF8(data)+1);
        var actualNumBytes = stringToUTF8Array(data, buf, 0, buf.length);
        await PThreadFS.write(stream, buf, 0, actualNumBytes, undefined, opts.canOwn);
      } else if (ArrayBuffer.isView(data)) {
        await PThreadFS.write(stream, data, 0, data.byteLength, undefined, opts.canOwn);
      } else {
        throw new Error('Unsupported data type');
      }
      await PThreadFS.close(stream);
    },

    //
    // module-level FS code
    //
    cwd: function() {
      return PThreadFS.currentPath;
    },
    chdir: async function(path) {
      var lookup = await PThreadFS.lookupPath(path, { follow: true });
      if (lookup.node === null) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      if (!PThreadFS.isDir(lookup.node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOTDIR') }}});
      }
      var errCode = PThreadFS.nodePermissions(lookup.node, 'x');
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      PThreadFS.currentPath = lookup.path;
    },
    createDefaultDirectories: async function() {
      await PThreadFS.mkdir('/tmp');
      await PThreadFS.mkdir('/home');
      await PThreadFS.mkdir('/home/web_user');
    },
    createDefaultDevices: async function() {
      // create /dev
      await PThreadFS.mkdir('/dev');
      // setup /dev/null
      PThreadFS.registerDevice(PThreadFS.makedev(1, 3), {
        read: function() { return 0; },
        write: function(stream, buffer, offset, length, pos) { return length; }
      });
      await PThreadFS.mkdev('/dev/null', PThreadFS.makedev(1, 3));
      // setup /dev/tty and /dev/tty1
      // stderr needs to print output using err() rather than out()
      // so we register a second tty just for it.
      TTY_ASYNC.register(PThreadFS.makedev(5, 0), TTY_ASYNC.default_tty_ops);
      TTY_ASYNC.register(PThreadFS.makedev(6, 0), TTY_ASYNC.default_tty1_ops);
      await PThreadFS.mkdev('/dev/tty', PThreadFS.makedev(5, 0));
      await PThreadFS.mkdev('/dev/tty1', PThreadFS.makedev(6, 0));
      // setup /dev/[u]random
      var random_device = getRandomDevice();
      await PThreadFS.createDevice('/dev', 'random', random_device);
      await PThreadFS.createDevice('/dev', 'urandom', random_device);
      // we're not going to emulate the actual shm device,
      // just create the tmp dirs that reside in it commonly
      await PThreadFS.mkdir('/dev/shm');
      await PThreadFS.mkdir('/dev/shm/tmp');
    },
    createSpecialDirectories: async function() {
      // create /proc/self/fd which allows /proc/self/fd/6 => readlink gives the
      // name of the stream for fd 6 (see test_unistd_ttyname)
      await PThreadFS.mkdir('/proc');
      var proc_self = await PThreadFS.mkdir('/proc/self');
      await PThreadFS.mkdir('/proc/self/fd');
      await PThreadFS.mount({
        mount: function() {
          var node = PThreadFS.createNode(proc_self, 'fd', {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */, {{{ cDefine('S_IXUGO') }}});
          node.node_ops = {
            lookup: function(parent, name) {
              var fd = +name;
              var stream = PThreadFS.getStream(fd);
              if (!stream) throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
              var ret = {
                parent: null,
                mount: { mountpoint: 'fake' },
                node_ops: { readlink: function() { return stream.path } }
              };
              ret.parent = ret; // make it look like a simple root node
              return ret;
            }
          };
          return node;
        }
      }, {}, '/proc/self/fd');
    },
    createStandardStreams: async function() {
      // TODO deprecate the old functionality of a single
      // input / output callback and that utilizes PThreadFS.createDevice
      // and instead require a unique set of stream ops

      // by default, we symlink the standard streams to the
      // default tty devices. however, if the standard streams
      // have been overwritten we create a unique device for
      // them instead.
      if (Module['stdin']) {
        await PThreadFS.createDevice('/dev', 'stdin', Module['stdin']);
      } else {
        await PThreadFS.symlink('/dev/tty', '/dev/stdin');
      }
      if (Module['stdout']) {
        await PThreadFS.createDevice('/dev', 'stdout', null, Module['stdout']);
      } else {
        await PThreadFS.symlink('/dev/tty', '/dev/stdout');
      }
      if (Module['stderr']) {
        await PThreadFS.createDevice('/dev', 'stderr', null, Module['stderr']);
      } else {
        await PThreadFS.symlink('/dev/tty1', '/dev/stderr');
      }

      // open default streams for the stdin, stdout and stderr devices
      var stdin = await PThreadFS.open('/dev/stdin', {{{ cDefine('O_RDONLY') }}});
      var stdout = await PThreadFS.open('/dev/stdout', {{{ cDefine('O_WRONLY') }}});
      var stderr = await PThreadFS.open('/dev/stderr', {{{ cDefine('O_WRONLY') }}});
#if ASSERTIONS
      assert(stdin.fd === 0, 'invalid handle for stdin (' + stdin.fd + ')');
      assert(stdout.fd === 1, 'invalid handle for stdout (' + stdout.fd + ')');
      assert(stderr.fd === 2, 'invalid handle for stderr (' + stderr.fd + ')');
#endif
    },
    ensureErrnoError: function() {
      if (PThreadFS.ErrnoError) return;
      PThreadFS.ErrnoError = /** @this{Object} */ function ErrnoError(errno, node) {
        this.node = node;
        this.setErrno = /** @this{Object} */ function(errno) {
          this.errno = errno;
#if ASSERTIONS
          for (var key in ERRNO_CODES) {
            if (ERRNO_CODES[key] === errno) {
              this.code = key;
              break;
            }
          }
#endif
        };
        this.setErrno(errno);
#if ASSERTIONS
        this.message = ERRNO_MESSAGES[errno];
#else
        this.message = 'PThreadFS error';
#endif

#if ASSERTIONS && !MINIMAL_RUNTIME
        // Try to get a maximally helpful stack trace. On Node.js, getting Error.stack
        // now ensures it shows what we want.
        if (this.stack) {
          // Define the stack property for Node.js 4, which otherwise errors on the next line.
          Object.defineProperty(this, "stack", { value: (new Error).stack, writable: true });
          this.stack = demangleAll(this.stack);
        }
#endif // ASSERTIONS
      };
      PThreadFS.ErrnoError.prototype = new Error();
      PThreadFS.ErrnoError.prototype.constructor = PThreadFS.ErrnoError;
      // Some errors may happen quite a bit, to avoid overhead we reuse them (and suffer a lack of stack info)
      [{{{ cDefine('ENOENT') }}}].forEach(function(code) {
        PThreadFS.genericErrors[code] = new PThreadFS.ErrnoError(code);
        PThreadFS.genericErrors[code].stack = '<generic error, no stack>';
      });
    },
    staticInit: async function() {
      PThreadFS.ensureErrnoError();

      PThreadFS.nameTable = new Array(4096);

      await PThreadFS.mount(MEMFS_ASYNC, {}, '/');

      await PThreadFS.createDefaultDirectories();
      await PThreadFS.createDefaultDevices();
      await PThreadFS.createSpecialDirectories();

      PThreadFS.filesystems = {
        'MEMFS_ASYNC': MEMFS_ASYNC,
      };
    },
    init: async function(input, output, error) {
#if ASSERTIONS
      assert(!PThreadFS.init.initialized, 'PThreadFS.init was previously called. If you want to initialize later with custom parameters, remove any earlier calls (note that one is automatically added to the generated code)');
#endif
      PThreadFS.init.initialized = true;

      PThreadFS.ensureErrnoError();

      // Allow Module.stdin etc. to provide defaults, if none explicitly passed to us here
      Module['stdin'] = input || Module['stdin'];
      Module['stdout'] = output || Module['stdout'];
      Module['stderr'] = error || Module['stderr'];

      await PThreadFS.createStandardStreams();
    },
    quit: async function() {
      // TODO(rstz): This function is never called.
      PThreadFS.init.initialized = false;
      // force-flush all streams, so we get musl std streams printed out
      var fflush = Module['_fflush'];
      if (fflush) fflush(0);
      // close all of our streams
      for (var i = 0; i < PThreadFS.streams.length; i++) {
        var stream = PThreadFS.streams[i];
        if (!stream) {
          continue;
        }
        await PThreadFS.close(stream);
      }
    },

    //
    // old v1 compatibility functions
    //
    getMode: function(canRead, canWrite) {
      var mode = 0;
      if (canRead) mode |= {{{ cDefine('S_IRUGO') }}} | {{{ cDefine('S_IXUGO') }}};
      if (canWrite) mode |= {{{ cDefine('S_IWUGO') }}};
      return mode;
    },
    findObject: async function(path, dontResolveLastLink) {
      var ret = await PThreadFS.analyzePath(path, dontResolveLastLink);
      if (ret.exists) {
        return ret.object;
      } else {
        return null;
      }
    },
    analyzePath: async function(path, dontResolveLastLink) {
      // operate from within the context of the symlink's target
      try {
        var lookup = await PThreadFS.lookupPath(path, { follow: !dontResolveLastLink });
        path = lookup.path;
      } catch (e) {
      }
      var ret = {
        isRoot: false, exists: false, error: 0, name: null, path: null, object: null,
        parentExists: false, parentPath: null, parentObject: null
      };
      try {
        var lookup = await PThreadFS.lookupPath(path, { parent: true });
        ret.parentExists = true;
        ret.parentPath = lookup.path;
        ret.parentObject = lookup.node;
        ret.name = PATH.basename(path);
        lookup = await PThreadFS.lookupPath(path, { follow: !dontResolveLastLink });
        ret.exists = true;
        ret.path = lookup.path;
        ret.object = lookup.node;
        ret.name = lookup.node.name;
        ret.isRoot = lookup.path === '/';
      } catch (e) {
        ret.error = e.errno;
      };
      return ret;
    },
    createPath: async function(parent, path, canRead, canWrite) {
      parent = typeof parent === 'string' ? parent : PThreadFS.getPath(parent);
      var parts = path.split('/').reverse();
      while (parts.length) {
        var part = parts.pop();
        if (!part) continue;
        var current = PATH.join2(parent, part);
        try {
          await PThreadFS.mkdir(current);
        } catch (e) {
          // ignore EEXIST
        }
        parent = current;
      }
      return current;
    },
    createFile: async function(parent, name, properties, canRead, canWrite) {
      var path = PATH.join2(typeof parent === 'string' ? parent : await PThreadFS.getPath(parent), name);
      var mode = await PThreadFS.getMode(canRead, canWrite);
      return await PThreadFS.create(path, mode);
    },
    createDataFile: async function(parent, name, data, canRead, canWrite, canOwn) {
      var path = name ? PATH.join2(typeof parent === 'string' ? parent : await PThreadFS.getPath(parent), name) : parent;
      var mode = await PThreadFS.getMode(canRead, canWrite);
      var node = await PThreadFS.create(path, mode);
      if (data) {
        if (typeof data === 'string') {
          var arr = new Array(data.length);
          for (var i = 0, len = data.length; i < len; ++i) arr[i] = data.charCodeAt(i);
          data = arr;
        }
        // make sure we can write to the file
        await PThreadFS.chmod(node, mode | {{{ cDefine('S_IWUGO') }}});
        var stream = await PThreadFS.open(node, {{{ cDefine('O_TRUNC') | cDefine('O_CREAT') | cDefine('O_WRONLY') }}});
        await PThreadFS.write(stream, data, 0, data.length, 0, canOwn);
        await PThreadFS.close(stream);
        await PThreadFS.chmod(node, mode);
      }
      return node;
    },
    createDevice: async function(parent, name, input, output) {
      var path = PATH.join2(typeof parent === 'string' ? parent : PThreadFS.getPath(parent), name);
      var mode = PThreadFS.getMode(!!input, !!output);
      if (!PThreadFS.createDevice.major) PThreadFS.createDevice.major = 64;
      var dev = PThreadFS.makedev(PThreadFS.createDevice.major++, 0);
      // Create a fake device that a set of stream ops to emulate
      // the old behavior.
      PThreadFS.registerDevice(dev, {
        open: function(stream) {
          stream.seekable = false;
        },
        close: function(stream) {
          // flush any pending line data
          if (output && output.buffer && output.buffer.length) {
            output({{{ charCode('\n') }}});
          }
        },
        read: function(stream, buffer, offset, length, pos /* ignored */) {
          var bytesRead = 0;
          for (var i = 0; i < length; i++) {
            var result;
            try {
              result = input();
            } catch (e) {
              throw new PThreadFS.ErrnoError({{{ cDefine('EIO') }}});
            }
            if (result === undefined && bytesRead === 0) {
              throw new PThreadFS.ErrnoError({{{ cDefine('EAGAIN') }}});
            }
            if (result === null || result === undefined) break;
            bytesRead++;
            buffer[offset+i] = result;
          }
          if (bytesRead) {
            stream.node.timestamp = Date.now();
          }
          return bytesRead;
        },
        write: function(stream, buffer, offset, length, pos) {
          for (var i = 0; i < length; i++) {
            try {
              output(buffer[offset+i]);
            } catch (e) {
              throw new PThreadFS.ErrnoError({{{ cDefine('EIO') }}});
            }
          }
          if (length) {
            stream.node.timestamp = Date.now();
          }
          return i;
        }
      });
      return await PThreadFS.mkdev(path, mode, dev);
    },
    // Makes sure a file's contents are loaded. Returns whether the file has
    // been loaded successfully. No-op for files that have been loaded already.
    forceLoadFile: function(obj) {
      if (obj.isDevice || obj.isFolder || obj.link || obj.contents) return true;
      if (typeof XMLHttpRequest !== 'undefined') {
        throw new Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.");
      } else if (read_) {
        // Command-line.
        try {
          // WARNING: Can't read binary files in V8's d8 or tracemonkey's js, as
          //          read() will try to parse UTF8.
          obj.contents = intArrayFromString(read_(obj.url), true);
          obj.usedBytes = obj.contents.length;
        } catch (e) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EIO') }}});
        }
      } else {
        throw new Error('Cannot load without read() or XMLHttpRequest.');
      }
    },
  },
});/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $MEMFS_ASYNC__deps: ['$PThreadFS', '$mmapAlloc'],
  $MEMFS_ASYNC: {
    ops_table: null,
    mount: function(mount) {
      return MEMFS_ASYNC.createNode(null, '/', {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */, 0);
    },
    createNode: function(parent, name, mode, dev) {
      if (PThreadFS.isBlkdev(mode) || PThreadFS.isFIFO(mode)) {
        // no supported
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      if (!MEMFS_ASYNC.ops_table) {
        MEMFS_ASYNC.ops_table = {
          dir: {
            node: {
              getattr: MEMFS_ASYNC.node_ops.getattr,
              setattr: MEMFS_ASYNC.node_ops.setattr,
              lookup: MEMFS_ASYNC.node_ops.lookup,
              mknod: MEMFS_ASYNC.node_ops.mknod,
              rename: MEMFS_ASYNC.node_ops.rename,
              unlink: MEMFS_ASYNC.node_ops.unlink,
              rmdir: MEMFS_ASYNC.node_ops.rmdir,
              readdir: MEMFS_ASYNC.node_ops.readdir,
              symlink: MEMFS_ASYNC.node_ops.symlink
            },
            stream: {
              llseek: MEMFS_ASYNC.stream_ops.llseek
            }
          },
          file: {
            node: {
              getattr: MEMFS_ASYNC.node_ops.getattr,
              setattr: MEMFS_ASYNC.node_ops.setattr
            },
            stream: {
              llseek: MEMFS_ASYNC.stream_ops.llseek,
              read: MEMFS_ASYNC.stream_ops.read,
              write: MEMFS_ASYNC.stream_ops.write,
              allocate: MEMFS_ASYNC.stream_ops.allocate,
              mmap: MEMFS_ASYNC.stream_ops.mmap,
              msync: MEMFS_ASYNC.stream_ops.msync
            }
          },
          link: {
            node: {
              getattr: MEMFS_ASYNC.node_ops.getattr,
              setattr: MEMFS_ASYNC.node_ops.setattr,
              readlink: MEMFS_ASYNC.node_ops.readlink
            },
            stream: {}
          },
          chrdev: {
            node: {
              getattr: MEMFS_ASYNC.node_ops.getattr,
              setattr: MEMFS_ASYNC.node_ops.setattr
            },
            stream: PThreadFS.chrdev_stream_ops
          }
        };
      }
      var node = PThreadFS.createNode(parent, name, mode, dev);
      if (PThreadFS.isDir(node.mode)) {
        node.node_ops = MEMFS_ASYNC.ops_table.dir.node;
        node.stream_ops = MEMFS_ASYNC.ops_table.dir.stream;
        node.contents = {};
      } else if (PThreadFS.isFile(node.mode)) {
        node.node_ops = MEMFS_ASYNC.ops_table.file.node;
        node.stream_ops = MEMFS_ASYNC.ops_table.file.stream;
        node.usedBytes = 0; // The actual number of bytes used in the typed array, as opposed to contents.length which gives the whole capacity.
        // When the byte data of the file is populated, this will point to either a typed array, or a normal JS array. Typed arrays are preferred
        // for performance, and used by default. However, typed arrays are not resizable like normal JS arrays are, so there is a small disk size
        // penalty involved for appending file writes that continuously grow a file similar to std::vector capacity vs used -scheme.
        node.contents = null; 
      } else if (PThreadFS.isLink(node.mode)) {
        node.node_ops = MEMFS_ASYNC.ops_table.link.node;
        node.stream_ops = MEMFS_ASYNC.ops_table.link.stream;
      } else if (PThreadFS.isChrdev(node.mode)) {
        node.node_ops = MEMFS_ASYNC.ops_table.chrdev.node;
        node.stream_ops = MEMFS_ASYNC.ops_table.chrdev.stream;
      }
      node.timestamp = Date.now();
      // add the new node to the parent
      if (parent) {
        parent.contents[name] = node;
        parent.timestamp = node.timestamp;
      }
      return node;
    },

    // Given a file node, returns its file data converted to a typed array.
    getFileDataAsTypedArray: function(node) {
      if (!node.contents) return new Uint8Array(0);
      if (node.contents.subarray) return node.contents.subarray(0, node.usedBytes); // Make sure to not return excess unused bytes.
      return new Uint8Array(node.contents);
    },

    // Allocates a new backing store for the given node so that it can fit at least newSize amount of bytes.
    // May allocate more, to provide automatic geometric increase and amortized linear performance appending writes.
    // Never shrinks the storage.
    expandFileStorage: function(node, newCapacity) {
#if CAN_ADDRESS_2GB
      newCapacity >>>= 0;
#endif
      var prevCapacity = node.contents ? node.contents.length : 0;
      if (prevCapacity >= newCapacity) return; // No need to expand, the storage was already large enough.
      // Don't expand strictly to the given requested limit if it's only a very small increase, but instead geometrically grow capacity.
      // For small filesizes (<1MB), perform size*2 geometric increase, but for large sizes, do a much more conservative size*1.125 increase to
      // avoid overshooting the allocation cap by a very large margin.
      var CAPACITY_DOUBLING_MAX = 1024 * 1024;
      newCapacity = Math.max(newCapacity, (prevCapacity * (prevCapacity < CAPACITY_DOUBLING_MAX ? 2.0 : 1.125)) >>> 0);
      if (prevCapacity != 0) newCapacity = Math.max(newCapacity, 256); // At minimum allocate 256b for each file when expanding.
      var oldContents = node.contents;
      node.contents = new Uint8Array(newCapacity); // Allocate new storage.
      if (node.usedBytes > 0) node.contents.set(oldContents.subarray(0, node.usedBytes), 0); // Copy old data over to the new storage.
    },

    // Performs an exact resize of the backing file storage to the given size, if the size is not exactly this, the storage is fully reallocated.
    resizeFileStorage: function(node, newSize) {
#if CAN_ADDRESS_2GB
      newSize >>>= 0;
#endif
      if (node.usedBytes == newSize) return;
      if (newSize == 0) {
        node.contents = null; // Fully decommit when requesting a resize to zero.
        node.usedBytes = 0;
      } else {
        var oldContents = node.contents;
        node.contents = new Uint8Array(newSize); // Allocate new storage.
        if (oldContents) {
          node.contents.set(oldContents.subarray(0, Math.min(newSize, node.usedBytes))); // Copy old data over to the new storage.
        }
        node.usedBytes = newSize;
      }
    },

    node_ops: {
      getattr: function(node) {
        var attr = {};
        // device numbers reuse inode numbers.
        attr.dev = PThreadFS.isChrdev(node.mode) ? node.id : 1;
        attr.ino = node.id;
        attr.mode = node.mode;
        attr.nlink = 1;
        attr.uid = 0;
        attr.gid = 0;
        attr.rdev = node.rdev;
        if (PThreadFS.isDir(node.mode)) {
          attr.size = 4096;
        } else if (PThreadFS.isFile(node.mode)) {
          attr.size = node.usedBytes;
        } else if (PThreadFS.isLink(node.mode)) {
          attr.size = node.link.length;
        } else {
          attr.size = 0;
        }
        attr.atime = new Date(node.timestamp);
        attr.mtime = new Date(node.timestamp);
        attr.ctime = new Date(node.timestamp);
        // NOTE: In our implementation, st_blocks = Math.ceil(st_size/st_blksize),
        //       but this is not required by the standard.
        attr.blksize = 4096;
        attr.blocks = Math.ceil(attr.size / attr.blksize);
        return attr;
      },
      setattr: function(node, attr) {
        if (attr.mode !== undefined) {
          node.mode = attr.mode;
        }
        if (attr.timestamp !== undefined) {
          node.timestamp = attr.timestamp;
        }
        if (attr.size !== undefined) {
          MEMFS_ASYNC.resizeFileStorage(node, attr.size);
        }
      },
      lookup: function(parent, name) {
        throw PThreadFS.genericErrors[{{{ cDefine('ENOENT') }}}];
      },
      mknod: function(parent, name, mode, dev) {
        return MEMFS_ASYNC.createNode(parent, name, mode, dev);
      },
      rename: async function(old_node, new_dir, new_name) {
        // if we're overwriting a directory at new_name, make sure it's empty.
        if (PThreadFS.isDir(old_node.mode)) {
          var new_node;
          try {
            new_node = await PThreadFS.lookupNode(new_dir, new_name);
          } catch (e) {
          }
          if (new_node) {
            for (var i in new_node.contents) {
              throw new PThreadFS.ErrnoError({{{ cDefine('ENOTEMPTY') }}});
            }
          }
        }
        // do the internal rewiring
        delete old_node.parent.contents[old_node.name];
        old_node.parent.timestamp = Date.now()
        old_node.name = new_name;
        new_dir.contents[new_name] = old_node;
        new_dir.timestamp = old_node.parent.timestamp;
        old_node.parent = new_dir;
      },
      unlink: function(parent, name) {
        delete parent.contents[name];
        parent.timestamp = Date.now();
      },
      rmdir: async function(parent, name) {
        var node = await PThreadFS.lookupNode(parent, name);
        for (var i in node.contents) {
          throw new PThreadFS.ErrnoError({{{ cDefine('ENOTEMPTY') }}});
        }
        delete parent.contents[name];
        parent.timestamp = Date.now();
      },
      readdir: function(node) {
        var entries = ['.', '..'];
        for (var key in node.contents) {
          if (!node.contents.hasOwnProperty(key)) {
            continue;
          }
          entries.push(key);
        }
        return entries;
      },
      symlink: function(parent, newname, oldpath) {
        var node = MEMFS_ASYNC.createNode(parent, newname, 511 /* 0777 */ | {{{ cDefine('S_IFLNK') }}}, 0);
        node.link = oldpath;
        return node;
      },
      readlink: function(node) {
        if (!PThreadFS.isLink(node.mode)) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
        }
        return node.link;
      },
    },
    stream_ops: {
      read: function(stream, buffer, offset, length, position) {
        var contents = stream.node.contents;
        if (position >= stream.node.usedBytes) return 0;
        var size = Math.min(stream.node.usedBytes - position, length);
#if ASSERTIONS
        assert(size >= 0);
#endif
        if (size > 8 && contents.subarray) { // non-trivial, and typed array
          buffer.set(contents.subarray(position, position + size), offset);
        } else {
          for (var i = 0; i < size; i++) buffer[offset + i] = contents[position + i];
        }
        return size;
      },

      // Writes the byte range (buffer[offset], buffer[offset+length]) to offset 'position' into the file pointed by 'stream'
      // canOwn: A boolean that tells if this function can take ownership of the passed in buffer from the subbuffer portion
      //         that the typed array view 'buffer' points to. The underlying ArrayBuffer can be larger than that, but
      //         canOwn=true will not take ownership of the portion outside the bytes addressed by the view. This means that
      //         with canOwn=true, creating a copy of the bytes is avoided, but the caller shouldn't touch the passed in range
      //         of bytes anymore since their contents now represent file data inside the filesystem.
      write: function(stream, buffer, offset, length, position, canOwn) {
#if ASSERTIONS
        // The data buffer should be a typed array view
        assert(!(buffer instanceof ArrayBuffer));
#endif
#if ALLOW_MEMORY_GROWTH
        // If the buffer is located in main memory (HEAP), and if
        // memory can grow, we can't hold on to references of the
        // memory buffer, as they may get invalidated. That means we
        // need to do copy its contents.
        if (buffer.buffer === HEAP8.buffer) {
          canOwn = false;
        }
#endif // ALLOW_MEMORY_GROWTH

        if (!length) return 0;
        var node = stream.node;
        node.timestamp = Date.now();

        if (buffer.subarray && (!node.contents || node.contents.subarray)) { // This write is from a typed array to a typed array?
          if (canOwn) {
#if ASSERTIONS
            assert(position === 0, 'canOwn must imply no weird position inside the file');
#endif
            node.contents = buffer.subarray(offset, offset + length);
            node.usedBytes = length;
            return length;
          } else if (node.usedBytes === 0 && position === 0) { // If this is a simple first write to an empty file, do a fast set since we don't need to care about old data.
            node.contents = buffer.slice(offset, offset + length);
            node.usedBytes = length;
            return length;
          } else if (position + length <= node.usedBytes) { // Writing to an already allocated and used subrange of the file?
            node.contents.set(buffer.subarray(offset, offset + length), position);
            return length;
          }
        }

        // Appending to an existing file and we need to reallocate, or source data did not come as a typed array.
        MEMFS_ASYNC.expandFileStorage(node, position+length);
        if (node.contents.subarray && buffer.subarray) {
          // Use typed array write which is available.
          node.contents.set(buffer.subarray(offset, offset + length), position);
        } else {
          for (var i = 0; i < length; i++) {
           node.contents[position + i] = buffer[offset + i]; // Or fall back to manual write if not.
          }
        }
        node.usedBytes = Math.max(node.usedBytes, position + length);
        return length;
      },

      llseek: function(stream, offset, whence) {
        var position = offset;
        if (whence === {{{ cDefine('SEEK_CUR') }}}) {
          position += stream.position;
        } else if (whence === {{{ cDefine('SEEK_END') }}}) {
          if (PThreadFS.isFile(stream.node.mode)) {
            position += stream.node.usedBytes;
          }
        }
        if (position < 0) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
        }
        return position;
      },
      allocate: function(stream, offset, length) {
        MEMFS_ASYNC.expandFileStorage(stream.node, offset + length);
        stream.node.usedBytes = Math.max(stream.node.usedBytes, offset + length);
      },
      mmap: function(stream, address, length, position, prot, flags) {
        if (address !== 0) {
          // We don't currently support location hints for the address of the mapping
          throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
        }
        if (!PThreadFS.isFile(stream.node.mode)) {
          throw new PThreadFS.ErrnoError({{{ cDefine('ENODEV') }}});
        }
        var ptr;
        var allocated;
        var contents = stream.node.contents;
        // Only make a new copy when MAP_PRIVATE is specified.
        if (!(flags & {{{ cDefine('MAP_PRIVATE') }}}) && contents.buffer === buffer) {
          // We can't emulate MAP_SHARED when the file is not backed by the buffer
          // we're mapping to (e.g. the HEAP buffer).
          allocated = false;
          ptr = contents.byteOffset;
        } else {
          // Try to avoid unnecessary slices.
          if (position > 0 || position + length < contents.length) {
            if (contents.subarray) {
              contents = contents.subarray(position, position + length);
            } else {
              contents = Array.prototype.slice.call(contents, position, position + length);
            }
          }
          allocated = true;
          ptr = mmapAlloc(length);
          if (!ptr) {
            throw new PThreadFS.ErrnoError({{{ cDefine('ENOMEM') }}});
          }
#if CAN_ADDRESS_2GB
          ptr >>>= 0;
#endif
          HEAP8.set(contents, ptr);
        }
        return { ptr: ptr, allocated: allocated };
      },
      msync: function(stream, buffer, offset, length, mmapFlags) {
        if (!PThreadFS.isFile(stream.node.mode)) {
          throw new PThreadFS.ErrnoError({{{ cDefine('ENODEV') }}});
        }
        if (mmapFlags & {{{ cDefine('MAP_PRIVATE') }}}) {
          // MAP_PRIVATE calls need not to be synced back to underlying fs
          return 0;
        }

        var bytesWritten = MEMFS_ASYNC.stream_ops.write(stream, buffer, 0, length, offset, false);
        // should we check if bytesWritten and length are the same?
        return 0;
      }
    }
  }
});
/**
 * @license
 * Copyright 2021 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $FSAFS__deps: ['$PThreadFS'],
  $FSAFS: {

    /* Debugging */

    debug: function(...args) {
      // Uncomment to print debug information.
      //
      // console.log(args);
    },

    /* Filesystem implementation (public interface) */

    createNode: function (parent, name, mode, dev) {
      FSAFS.debug('createNode', arguments);
      if (!PThreadFS.isDir(mode) && !PThreadFS.isFile(mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      var node = PThreadFS.createNode(parent, name, mode);
      node.node_ops = FSAFS.node_ops;
      node.stream_ops = FSAFS.stream_ops;
      if (PThreadFS.isDir(mode)) {
        node.contents = {};
      }
      node.timestamp = Date.now();
      return node;
    },

    mount: async function (mount) {
      FSAFS.debug('mount', arguments);
      let node = FSAFS.createNode(null, '/', {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */, 0);
      FSAFS.root = await navigator.storage.getDirectory();
      node.localReference = FSAFS.root;
      return node;
    },

    /* Operations on the nodes of the filesystem tree */

    node_ops: {
      getattr: async function(node) {
        FSAFS.debug('getattr', arguments);
        var attr = {};
        // device numbers reuse inode numbers.
        attr.dev = PThreadFS.isChrdev(node.mode) ? node.id : 1;
        attr.ino = node.id;
        attr.mode = node.mode;
        attr.nlink = 1;
        attr.uid = 0;
        attr.gid = 0;
        attr.rdev = node.rdev;
        if (PThreadFS.isDir(node.mode)) {
          attr.size = 4096;
        } else if (PThreadFS.isFile(node.mode)) {
          if (node.handle) {
            attr.size = await node.handle.getSize();
          } 
          else {
            let fileHandle = await node.localReference.createSyncAccessHandle();
            attr.size = await fileHandle.getSize();
            await fileHandle.close();
          }
        } else if (PThreadFS.isLink(node.mode)) {
          attr.size = node.link.length;
        } else {
          attr.size = 0;
        }
        attr.atime = new Date(node.timestamp);
        attr.mtime = new Date(node.timestamp);
        attr.ctime = new Date(node.timestamp);
        // NOTE: In our implementation, st_blocks = Math.ceil(st_size/st_blksize),
        //       but this is not required by the standard.
        attr.blksize = 4096;
        attr.blocks = Math.ceil(attr.size / attr.blksize);
        return attr;
      },

      setattr: async function(node, attr) {
        FSAFS.debug('setattr', arguments);
        if (attr.mode !== undefined) {
          node.mode = attr.mode;
        }
        if (attr.timestamp !== undefined) {
          node.timestamp = attr.timestamp;
        }
        if (attr.size !== undefined) {
          let useOpen = false;
          let fileHandle = node.handle;
          try {
            if (!fileHandle) {
              // Open a handle that is closed later.
              useOpen = true;
              fileHandle = await node.localReference.createSyncAccessHandle();
            }
            await fileHandle.truncate(attr.size);
            
          } catch (e) {
            if (!('code' in e)) throw e;
            throw new PThreadFS.ErrnoError(-e.errno);
          } finally {
            if (useOpen) {
              await fileHandle.close();
            }
          }
        }
      },

      lookup: async function (parent, name) {
        FSAFS.debug('lookup', arguments);
        let childLocalReference = null;
        let mode = null;
        try {
          childLocalReference = await parent.localReference.getDirectoryHandle(name, {create: false});
          mode = {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */
        } catch (e) {
          try {
            childLocalReference = await parent.localReference.getFileHandle(name, {create: false});
            mode = {{{ cDefine('S_IFREG') }}} | 511 /* 0777 */
          } catch (e) {
            throw PThreadFS.genericErrors[{{{ cDefine('ENOENT') }}}];
          }
        }
        var node = PThreadFS.createNode(parent, name, mode);
        node.node_ops = FSAFS.node_ops;
        node.stream_ops = FSAFS.stream_ops;
        node.localReference = childLocalReference;
        return node;
      },

      mknod: async function (parent, name, mode, dev) {
        FSAFS.debug('mknod', arguments);
        let node = FSAFS.createNode(parent, name, mode, dev);
        try {
          if (PThreadFS.isDir(mode)) {
            node.localReference = await parent.localReference.getDirectoryHandle(name, {create: true});
          } else if (PThreadFS.isFile(mode)) {
            node.localReference = await parent.localReference.getFileHandle(name, {create: true});
          }
        } catch (e) {
          if (!('code' in e)) throw e;
          throw new PThreadFS.ErrnoError(-e.errno);
        }

        node.handle = null;
        node.refcount = 0;
        return node;
      },

      rename: function (oldNode, newParentNode, newName) {
        FSAFS.debug('rename', arguments);
        console.log('FSAFS error: rename is not implemented')
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
      },

      unlink: async function(parent, name) {
        FSAFS.debug('unlink', arguments);
        delete parent.contents[name];
        return await parent.localReference.removeEntry(name);
      },

      rmdir: async function(parent, name) {
        FSAFS.debug('rmdir', arguments);
        for (var i in node.contents) {
          throw new FS.ErrnoError({{{ cDefine('ENOTEMPTY') }}});
        }
        delete parent.contents[name];
        return await parent.localReference.removeEntry(name);
      },

      readdir: async function(node) {
        FSAFS.debug('readdir', arguments);
        let entries = ['.', '..'];
        for await (let [name, handle] of node.localReference) {
          entries.push(name);
        }
        return entries;
      },

      symlink: function(parent, newName, oldPath) {
        console.log('FSAFS error: symlink is not implemented')
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
      },

      readlink: function(node) {
        console.log('FSAFS error: readlink is not implemented')
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
      },
    },

    /* Operations on file streams (i.e., file handles) */

    stream_ops: {
      open: async function (stream) {
        FSAFS.debug('open', arguments);
        if (PThreadFS.isDir(stream.node.mode)) {
          console.log('FSAFS error: open for directories is not fully implemented')
          throw new PThreadFS.ErrnoError({{{ cDefine('EISDIR') }}});
        }
        if (!PThreadFS.isFile(stream.node.mode)) {
          console.log('FSAFS error: open is only implemented for files')
          throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
        }

        if (stream.node.handle) {
          stream.handle = stream.node.handle;
          ++stream.node.refcount;
        } else {
          stream.handle = await stream.node.localReference.createSyncAccessHandle();
          stream.node.handle = stream.handle;
          stream.node.refcount = 1;
        }
      },

      close: async function (stream) {
        FSAFS.debug('close', arguments);
        if (!PThreadFS.isFile(stream.node.mode)) {
          console.log('FSAFS error: close is only implemented for files');
          throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
        }

        stream.handle = null;
        --stream.node.refcount;
        if (stream.node.refcount <= 0) {
          await stream.node.handle.close();
          stream.node.handle = null;
        }
      },

      fsync: async function(stream) {
        FSAFS.debug('fsync', arguments);
        if (stream.handle == null) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
        }
        await stream.handle.flush();
        return 0;
      },

      read: async function (stream, buffer, offset, length, position) {
        FSAFS.debug('read', arguments);
        let data = buffer.subarray(offset, offset+length);
        let readBytes = await stream.handle.read(data, {at: position});
        return readBytes;
      },

      write: async function (stream, buffer, offset, length, position) {
        FSAFS.debug('write', arguments);
        stream.node.timestamp = Date.now();
        let data = buffer.subarray(offset, offset+length);
        let writtenBytes = await stream.handle.write(data, {at: position});
        return writtenBytes;
      },

      llseek: async function (stream, offset, whence) {
        FSAFS.debug('llseek', arguments);
        let position = offset;
        if (whence === {{{ cDefine('SEEK_CUR') }}}) {
          position += stream.position;
        } else if (whence === {{{ cDefine('SEEK_END') }}}) {
          if (PThreadFS.isFile(stream.node.mode)) {
            position += await stream.handle.getSize();
          }
        } 

        if (position < 0) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
        }
        return position;
      },

      mmap: function(stream, buffer, offset, length, position, prot, flags) {
        FSAFS.debug('mmap', arguments);
        throw new PThreadFS.ErrnoError({{{ cDefine('EOPNOTSUPP') }}});
      },

      msync: function(stream, buffer, offset, length, mmapFlags) {
        FSAFS.debug('msync', arguments);
        throw new PThreadFS.ErrnoError({{{ cDefine('EOPNOTSUPP') }}});
      },

      munmap: function(stream) {
        FSAFS.debug('munmap', arguments);
        throw new PThreadFS.ErrnoError({{{ cDefine('EOPNOTSUPP') }}});
      },
    }
  }
});/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

 mergeInto(LibraryManager.library, {
  $TTY_ASYNC__deps: ['$PThreadFS'],
#if !MINIMAL_RUNTIME
  $TTY_ASYNC__postset: function() {
    addAtInit('TTY_ASYNC.init();');
    addAtExit('TTY_ASYNC.shutdown();');
  },
#endif
  $TTY_ASYNC: {
    ttys: [],
    init: function () {
      // https://github.com/emscripten-core/emscripten/pull/1555
      // if (ENVIRONMENT_IS_NODE) {
      //   // currently, PThreadFS.init does not distinguish if process.stdin is a file or TTY_ASYNC
      //   // device, it always assumes it's a TTY_ASYNC device. because of this, we're forcing
      //   // process.stdin to UTF8 encoding to at least make stdin reading compatible
      //   // with text files until PThreadFS.init can be refactored.
      //   process['stdin']['setEncoding']('utf8');
      // }
    },
    shutdown: function() {
      // https://github.com/emscripten-core/emscripten/pull/1555
      // if (ENVIRONMENT_IS_NODE) {
      //   // inolen: any idea as to why node -e 'process.stdin.read()' wouldn't exit immediately (with process.stdin being a tty)?
      //   // isaacs: because now it's reading from the stream, you've expressed interest in it, so that read() kicks off a _read() which creates a ReadReq operation
      //   // inolen: I thought read() in that case was a synchronous operation that just grabbed some amount of buffered data if it exists?
      //   // isaacs: it is. but it also triggers a _read() call, which calls readStart() on the handle
      //   // isaacs: do process.stdin.pause() and i'd think it'd probably close the pending call
      //   process['stdin']['pause']();
      // }
    },
    register: function(dev, ops) {
      TTY_ASYNC.ttys[dev] = { input: [], output: [], ops: ops };
      PThreadFS.registerDevice(dev, TTY_ASYNC.stream_ops);
    },
    stream_ops: {
      open: function(stream) {
        var tty = TTY_ASYNC.ttys[stream.node.rdev];
        if (!tty) {
          throw new PThreadFS.ErrnoError({{{ cDefine('ENODEV') }}});
        }
        stream.tty = tty;
        stream.seekable = false;
      },
      close: function(stream) {
        // flush any pending line data
        stream.tty.ops.flush(stream.tty);
      },
      flush: function(stream) {
        stream.tty.ops.flush(stream.tty);
      },
      read: function(stream, buffer, offset, length, pos /* ignored */) {
        if (!stream.tty || !stream.tty.ops.get_char) {
          throw new PThreadFS.ErrnoError({{{ cDefine('ENXIO') }}});
        }
        var bytesRead = 0;
        for (var i = 0; i < length; i++) {
          var result;
          try {
            result = stream.tty.ops.get_char(stream.tty);
          } catch (e) {
            throw new PThreadFS.ErrnoError({{{ cDefine('EIO') }}});
          }
          if (result === undefined && bytesRead === 0) {
            throw new PThreadFS.ErrnoError({{{ cDefine('EAGAIN') }}});
          }
          if (result === null || result === undefined) break;
          bytesRead++;
          buffer[offset+i] = result;
        }
        if (bytesRead) {
          stream.node.timestamp = Date.now();
        }
        return bytesRead;
      },
      write: function(stream, buffer, offset, length, pos) {
        if (!stream.tty || !stream.tty.ops.put_char) {
          throw new PThreadFS.ErrnoError({{{ cDefine('ENXIO') }}});
        }
        try {
          for (var i = 0; i < length; i++) {
            stream.tty.ops.put_char(stream.tty, buffer[offset+i]);
          }
        } catch (e) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EIO') }}});
        }
        if (length) {
          stream.node.timestamp = Date.now();
        }
        return i;
      }
    },
    default_tty_ops: {
      // get_char has 3 particular return values:
      // a.) the next character represented as an integer
      // b.) undefined to signal that no data is currently available
      // c.) null to signal an EOF
      get_char: function(tty) {
        if (!tty.input.length) {
          var result = null;
          if (typeof window != 'undefined' &&
            typeof window.prompt == 'function') {
            // Browser.
            result = window.prompt('Input: ');  // returns null on cancel
            if (result !== null) {
              result += '\n';
            }
          } else if (typeof readline == 'function') {
            // Command line.
            result = readline();
            if (result !== null) {
              result += '\n';
            }
          }
          if (!result) {
            return null;
          }
          tty.input = intArrayFromString(result, true);
        }
        return tty.input.shift();
      },
      put_char: function(tty, val) {
        if (val === null || val === {{{ charCode('\n') }}}) {
          out(UTF8ArrayToString(tty.output, 0));
          tty.output = [];
        } else {
          if (val != 0) tty.output.push(val); // val == 0 would cut text output off in the middle.
        }
      },
      flush: function(tty) {
        if (tty.output && tty.output.length > 0) {
          out(UTF8ArrayToString(tty.output, 0));
          tty.output = [];
        }
      }
    },
    default_tty1_ops: {
      put_char: function(tty, val) {
        if (val === null || val === {{{ charCode('\n') }}}) {
          err(UTF8ArrayToString(tty.output, 0));
          tty.output = [];
        } else {
          if (val != 0) tty.output.push(val);
        }
      },
      flush: function(tty) {
        if (tty.output && tty.output.length > 0) {
          err(UTF8ArrayToString(tty.output, 0));
          tty.output = [];
        }
      }
    }
  }
});