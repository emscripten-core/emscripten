mergeInto(LibraryManager.library, {
  $NODERAWFS__deps: ['$ERRNO_CODES', '$FS', '$NODEFS'],
  $NODERAWFS__postset: 'if (ENVIRONMENT_IS_NODE) {' +
    'var _wrapNodeError = function(func) { return function() { try { return func.apply(this, arguments) } catch (e) { if (!e.code) throw e; throw new FS.ErrnoError(ERRNO_CODES[e.code]); } } };' +
    'for (var key in NODERAWFS) FS[key] = _wrapNodeError(NODERAWFS[key]);' +
    '}',
  $NODERAWFS: {
    createStandardStreams: function() {
      FS.streams[0] = { fd: 0, nfd: 0, position: 0, path: '', flags: 0 };
      for (var i = 1; i < 3; i++) {
        FS.streams[i] = { fd: i, nfd: i, position: 0, path: '', flags: 577 };
      }
    },
    // generic function for all node creation
    mknod: function(path, mode, dev) {
      if (FS.isDir(node.mode)) {
        fs.mkdirSync(path, node.mode);
      } else {
        fs.writeFileSync(path, '', { mode: node.mode });
      }
    },
    mkdir: function() { fs.mkdirSync.apply(void 0, arguments); },
    symlink: function() { fs.symlinkSync.apply(void 0, arguments); },
    rename: function() { fs.renameSync.apply(void 0, arguments); },
    rmdir: function() { fs.rmdirSync.apply(void 0, arguments); },
    readdir: function() { fs.readdirSync.apply(void 0, arguments); },
    unlink: function() { fs.unlinkSync.apply(void 0, arguments); },
    readlink: function() { fs.readlinkSync.apply(void 0, arguments); },
    stat: function() { fs.statSync.apply(void 0, arguments); },
    lstat: function() { fs.lstatSync.apply(void 0, arguments); },
    chmod: function() { fs.chmodSync.apply(void 0, arguments); },
    fchmod: function() { fs.fchmodSync.apply(void 0, arguments); },
    chown: function() { fs.chownSync.apply(void 0, arguments); },
    fchown: function() { fs.fchownSync.apply(void 0, arguments); },
    truncate: function() { fs.truncate.apply(void 0, arguments); },
    ftruncate: function() { fs.ftruncate.apply(void 0, arguments); },
    utime: function() { fs.utimes.apply(void 0, arguments); },
    open: function(path, flags, mode, suggestFD) {
      var nfd = fs.openSync(path, NODEFS.flagsToPermissionString(flags), mode);
      var fd = suggestFD != null ? suggestFD : nfd;
      var stream = { fd: fd, nfd: nfd, position: 0, path: path, flags: flags };
      FS.streams[fd] = stream;
      return stream;
    },
    close: function(stream) {
      fs.closeSync(stream.nfd);
      FS.closeStream(stream);
    },
    llseek: function(stream, offset, whence) {
      var position = offset;
      if (whence === 1) {  // SEEK_CUR.
        position += stream.position;
      } else if (whence === 2) {  // SEEK_END.
        position += fs.fstatSync(stream.nfd).size;
      }

      if (position < 0) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }

      return position;
    },
    read: function(stream, buffer, offset, length, position) {
      var seeking = true;
      if (typeof position === 'undefined') {
        seeking = false;
      }
      var bytesRead = fs.readSync(stream.nfd, Buffer.from(buffer.buffer), offset, length, position);
      if (!seeking) stream.position += bytesRead;
      return bytesRead;
    },
    write: function(stream, buffer, offset, length, position) {
      if (stream.flags & +"{{{ cDefine('O_APPEND') }}}") {
        // seek to the end before writing in append mode
        position = FS.llseek(stream, 0, +"{{{ cDefine('SEEK_END') }}}");
      }
      var seeking = true;
      if (typeof position === 'undefined') {
        seeking = false;
      }
      var bytesWritten = fs.writeSync(stream.nfd, Buffer.from(buffer.buffer), offset, length, position);
      if (!seeking) stream.position += bytesWritten;
      return bytesWritten;
    },
    allocate: function() {
      throw new FS.ErrnoError(ERRNO_CODES.EOPNOTSUPP);
    },
    mmap: function() {
      throw new FS.ErrnoError(ERRNO_CODES.ENODEV);
    },
    msync: function() {
      return 0;
    },
    munmap: function() {
      return 0;
    },
    ioctl: function() {
      throw new FS.ErrnoError(ERRNO_CODES.ENOTTY);
    }
  }
});
