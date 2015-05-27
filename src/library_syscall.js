mergeInto(LibraryManager.library, {
  $SYSCALLS: {
#if SYSCALL_DEBUG
    NAME_TO_CODE: {
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
    },
    CODE_TO_NAME: {},
    getFromCode: function(code) {
      if (!SYSCALLS.CODE_TO_NAME[0]) {
        for (var name in SYSCALLS.NAME_TO_CODE) {
          SYSCALLS.CODE_TO_NAME[SYSCALLS.NAME_TO_CODE[name]] = name;
        }
      }
      assert(SYSCALLS.CODE_TO_NAME[code]);
      return SYSCALLS.CODE_TO_NAME[code];
    },
#endif
    // shared utilities
    doStat: function(func, buf) {
      try {
        var stat = func();
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
  },

  __syscall__deps: ['$SYSCALLS', '$FS', '$ERRNO_CODES', '$PATH', '__setErrNo'
#if SYSCALL_DEBUG
                   ,'$ERRNO_MESSAGES'
#endif
  ],
  __syscall: function(which, varargs) {
    var get;
    if (typeof which === 'number') {
      get = function() {
        var ret = {{{ makeGetValue('varargs', '0', 'i32') }}};
        varargs += 4;
#if SYSCALL_DEBUG
        Module.printErr('  syscall arg: ' + ret);
#endif
        return ret;
      }
    } else {
#if ASSERTIONS
      assert(varargs === 0);
#endif
      var array = which;
      var which = array[0];
      var index = 1;
      get = function() {
#if ASSERTIONS
        assert(index < array.length);
#endif
#if SYSCALL_DEBUG
        Module.printErr('  syscall arg: ' + array[index]);
#endif
        return array[index++];
      };
    }
    function getStr() {
      var ret = Pointer_stringify(get());
#if SYSCALL_DEBUG
      Module.printErr('    (str: "' + ret + '")');
#endif
      return ret;
    }
    function getStreamFromFD() {
      var stream = FS.getStream(get());
      if (!stream) throw new FS.ErrnoError(ERRNO_CODES.EBADF);
#if SYSCALL_DEBUG
      Module.printErr('    (stream: "' + stream.path + '")');
#endif
      return stream;
    }
#if SYSCALL_DEBUG
    Module.printErr('syscall! ' + [which, SYSCALLS.getFromCode(which)]);
    var ret = (function() {
#endif
    try {
      switch (which) {
        case 4: { // write
          var stream = getStreamFromFD(), buf = get(), count = get();
          return FS.write(stream, {{{ makeGetSlabs('ptr', 'i8', true) }}}, buf, count);
        }
        case 5: { // open
          var pathname = getStr(), flags = get(), mode = get() /* optional TODO */;
          var stream = FS.open(pathname, flags, mode);
          return stream.fd;
        }
        case 6: { // close
          var stream = getStreamFromFD();
          if (stream.getdents) stream.getdents = null; // free readdir state
          FS.close(stream);
          return 0;
        }
        case 10: { // unlink
          var path = getStr();
          FS.unlink(path);
          return 0;
        }
        case 14: { // mknod
          var path = getStr(), mode = get(), dev = get();
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
        }
        case 15: { // chmod
          var path = getStr(), mode = get();
          FS.chmod(path, mode);
          return 0;
        }
        case 33: { // access
          var path = getStr(), amode = get();
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
        }
        case 38: { // rename
          var old_path = getStr(), new_path = getStr();
          FS.rename(old_path, new_path);
          return 0;
        }
        case 39: { // mkdir
          var path = getStr(), mode = get();
          // remove a trailing slash, if one - /a/b/ has basename of '', but
          // we want to create b in the context of this function
          path = PATH.normalize(path);
          if (path[path.length-1] === '/') path = path.substr(0, path.length-1);
          FS.mkdir(path, mode, 0);
          return 0;
        }
        case 40: { // rmdir
          var path = getStr();
          FS.rmdir(path);
          return 0;
        }
        case 54: { // ioctl
          var stream = getStreamFromFD(), op = get(), tio = get();
          switch (op) {
            case 0x5401: { // TCGETS
              if (!stream.tty) return -ERRNO_CODES.ENOTTY;
#if SYSCALL_DEBUG
              Module.printErr('warning: not filling tio struct');
#endif
              return 0;
            }
            case 0x5402: { // TCGETS
              if (!stream.tty) return -ERRNO_CODES.ENOTTY;
              return 0; // no-op, not actually adjusting terminal settings
            }
            default: abort('bad ioctl syscall ' + op);
          }
        }
        case 83: { // symlink
          var target = getStr(), linkpath = getStr();
          FS.symlink(target, linkpath);
          return 0;
        }
        case 94: { // fchmod
          var fd = get(), mode = get();
          FS.fchmod(fd, mode);
          return 0;
        }
        case 140: { // llseek
          var stream = getStreamFromFD(), offset_high = get(), offset_low = get(), result = get(), whence = get();
          var offset = offset_low;
          assert(offset_high === 0);
          FS.llseek(stream, offset, whence);
          {{{ makeSetValue('result', '0', 'stream.position', 'i32') }}};
          if (stream.getdents && offset === 0 && whence === {{{ cDefine('SEEK_SET') }}}) stream.getdents = null; // reset readdir state
          return 0;
        }
        case 145: { // readv
          var stream = getStreamFromFD(), iov = get(), iovcnt = get();
          var ret = 0;
          for (var i = 0; i < iovcnt; i++) {
            var ptr = {{{ makeGetValue('iov', 'i*8', 'i32') }}};
            var len = {{{ makeGetValue('iov', 'i*8 + 4', 'i32') }}};
            var curr = FS.read(stream, {{{ makeGetSlabs('ptr', 'i8', true) }}}, ptr, len);
            if (curr < 0) return -1;
            ret += curr;
            if (curr < len) break; // nothing more to read
          }
          return ret;
        }
        case 146: { // writev
          var stream = getStreamFromFD(), iov = get(), iovcnt = get();
          var ret = 0;
          for (var i = 0; i < iovcnt; i++) {
            var ptr = {{{ makeGetValue('iov', 'i*8', 'i32') }}};
            var len = {{{ makeGetValue('iov', 'i*8 + 4', 'i32') }}};
            var curr = FS.write(stream, {{{ makeGetSlabs('ptr', 'i8', true) }}}, ptr, len);
            if (curr < 0) return -1;
            ret += curr;
          }
          return ret;
        }
        case 195: { // SYS_stat64
          var path = getStr(), buf = get();
          return SYSCALLS.doStat(function() { return FS.stat(path) }, buf);
        }
        case 196: { // SYS_lstat64
          var path = getStr(), buf = get();
          return SYSCALLS.doStat(function() { return FS.lstat(path) }, buf);
        }
        case 197: { // SYS_fstat64
          var stream = getStreamFromFD(), buf = get();
          return SYSCALLS.doStat(function() { return FS.stat(stream.path) }, buf);
        }
        case 220: { // SYS_getdents64
          var stream = getStreamFromFD(), dirp = get(), count = get();
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
        }
        case 221: { // fcntl64
          var stream = getStreamFromFD(), cmd = get();
          switch (cmd) {
            case {{{ cDefine('F_DUPFD') }}}: {
              var arg = get();
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
              var arg = get();
              stream.flags |= arg;
              return 0;
            }
            case {{{ cDefine('F_GETLK') }}}:
            case {{{ cDefine('F_GETLK64') }}}: {
              var arg = get();
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
        }
        case 265: { // clock_nanosleep
#if SYSCALL_DEBUG
          Module.printErr('warning: ignoring SYS_clock_nanosleep');
#endif
          return 0;
        }
        case 272: { // fadvise64_64
          return 0; // your advice is important to us (but we can't use it)
        }
        case 300: { // fstatat64
          var dirfd = get(), path = getStr(), buf = get(), flags = get();
          var nofollow = flags & {{{ cDefine('AT_SYMLINK_NOFOLLOW') }}};
          flags = flags & (~{{{ cDefine('AT_SYMLINK_NOFOLLOW') }}});
          assert(!flags, flags);
          if (path[0] !== '/') {
            // relative path
            var dir;
            if (dirfd === {{{ cDefine('AT_FDCWD') }}}) {
              dir = FS.cwd();
            } else {
              var dirstream = FS.getStream(dirfd);
              if (!dirstream) return -ERRNO_CODES.EBADF;
              dir = dirstream.path;
            }
            path = PATH.join2(dir, path);
          }
          return SYSCALLS.doStat(function() { return (nofollow ? FS.lstat : FS.stat)(path) }, buf);
        }
        default: abort('bad syscall ' + which);
      }
    } catch (e) {
      if (typeof FS === 'undefined' || !(e instanceof FS.ErrnoError)) abort(e);
#if SYSCALL_DEBUG
      Module.printErr('error: syscall failed with ' + e.errno + ' (' + ERRNO_MESSAGES[e.errno] + ')');
#endif
      return -e.errno;
    }
#if SYSCALL_DEBUG
    })();
    Module.printErr('syscall return: ' + ret);
    return ret;
#endif
  },

  __syscall_cp__deps: ['__syscall'],
  __syscall_cp: function() {
    var args = Array.prototype.slice.call(arguments);
    return ___syscall(args, 0);
  },

  // methods that musl could do, but for now we do
  _pthread_cleanup_push: function(){},
  _pthread_cleanup_pop: function(){},
  __pthread_self: function() { abort() },
});

