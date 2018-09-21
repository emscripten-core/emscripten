// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

mergeInto(LibraryManager.library, {
  $NODERAWFS__deps: ['$ERRNO_CODES', '$FS', '$NODEFS'],
  $NODERAWFS__postset: 'if (ENVIRONMENT_IS_NODE) {' +
    'var _wrapNodeError = function(func) { return function() { try { return func.apply(this, arguments) } catch (e) { if (!e.code) throw e; throw new FS.ErrnoError(ERRNO_CODES[e.code]); } } };' +
    'var VFS = Object.assign({}, FS);' +
    'for (var _key in NODERAWFS) FS[_key] = _wrapNodeError(NODERAWFS[_key]);' +
    '}' +
    'else { throw new Error("NODERAWFS is currently only supported on Node.js environment.") }',
  $NODERAWFS: {
    lookupPath: function(path) {
      return { path: path, node: { mode: NODEFS.getMode(path) } };
    },
    createStandardStreams: function() {
      FS.streams[0] = { fd: 0, nfd: 0, position: 0, path: '', flags: 0, tty: true, seekable: false };
      for (var i = 1; i < 3; i++) {
        FS.streams[i] = { fd: i, nfd: i, position: 0, path: '', flags: 577, tty: true, seekable: false };
      }
    },
    // generic function for all node creation
    cwd: function() { return process.cwd(); },
    chdir: function() { process.chdir.apply(void 0, arguments); },
    mknod: function(path, mode) {
      if (FS.isDir(path)) {
        fs.mkdirSync(path, mode);
      } else {
        fs.writeFileSync(path, '', { mode: mode });
      }
    },
    mkdir: function() { fs.mkdirSync.apply(void 0, arguments); },
    symlink: function() { fs.symlinkSync.apply(void 0, arguments); },
    rename: function() { fs.renameSync.apply(void 0, arguments); },
    rmdir: function() { fs.rmdirSync.apply(void 0, arguments); },
    readdir: function() { fs.readdirSync.apply(void 0, arguments); },
    unlink: function() { fs.unlinkSync.apply(void 0, arguments); },
    readlink: function() { return fs.readlinkSync.apply(void 0, arguments); },
    stat: function() { return fs.statSync.apply(void 0, arguments); },
    lstat: function() { return fs.lstatSync.apply(void 0, arguments); },
    chmod: function() { fs.chmodSync.apply(void 0, arguments); },
    fchmod: function() { fs.fchmodSync.apply(void 0, arguments); },
    chown: function() { fs.chownSync.apply(void 0, arguments); },
    fchown: function() { fs.fchownSync.apply(void 0, arguments); },
    truncate: function() { fs.truncateSync.apply(void 0, arguments); },
    ftruncate: function() { fs.ftruncateSync.apply(void 0, arguments); },
    utime: function() { fs.utimesSync.apply(void 0, arguments); },
    open: function(path, flags, mode, suggestFD) {
      if (typeof flags === "string") {
        flags = VFS.modeStringToFlags(flags)
      }
      var nfd = fs.openSync(path, NODEFS.flagsForNode(flags), mode);
      var fd = suggestFD != null ? suggestFD : FS.nextfd(nfd);
      var stream = { fd: fd, nfd: nfd, position: 0, path: path, flags: flags, seekable: true };
      FS.streams[fd] = stream;
      return stream;
    },
    close: function(stream) {
      if (!stream.stream_ops) {
        // this stream is created by in-memory filesystem
        fs.closeSync(stream.nfd);
      }
      FS.closeStream(stream.fd);
    },
    llseek: function(stream, offset, whence) {
      if (stream.stream_ops) {
        // this stream is created by in-memory filesystem
        return VFS.llseek(stream, offset, whence);
      }
      var position = offset;
      if (whence === 1) {  // SEEK_CUR.
        position += stream.position;
      } else if (whence === 2) {  // SEEK_END.
        position += fs.fstatSync(stream.nfd).size;
      }

      if (position < 0) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      stream.position = position;
      return position;
    },
    read: function(stream, buffer, offset, length, position) {
      if (stream.stream_ops) {
        // this stream is created by in-memory filesystem
        return VFS.read(stream, buffer, offset, length, position);
      }
      var seeking = typeof position !== 'undefined';
      if (!seeking && stream.seekable) position = stream.position;
      var bytesRead = fs.readSync(stream.nfd, NODEFS.bufferFrom(buffer.buffer), offset, length, position);
      // update position marker when non-seeking
      if (!seeking) stream.position += bytesRead;
      return bytesRead;
    },
    write: function(stream, buffer, offset, length, position) {
      if (stream.stream_ops) {
        // this stream is created by in-memory filesystem
        return VFS.write(stream, buffer, offset, length, position);
      }
      if (stream.flags & +"{{{ cDefine('O_APPEND') }}}") {
        // seek to the end before writing in append mode
        FS.llseek(stream, 0, +"{{{ cDefine('SEEK_END') }}}");
      }
      var seeking = typeof position !== 'undefined';
      if (!seeking && stream.seekable) position = stream.position;
      var bytesWritten = fs.writeSync(stream.nfd, NODEFS.bufferFrom(buffer.buffer), offset, length, position);
      // update position marker when non-seeking
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
