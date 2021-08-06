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

mergeInto(LibraryManager.library, SyscallWrappers);