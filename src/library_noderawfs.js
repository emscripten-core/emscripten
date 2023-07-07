/**
 * @license
 * Copyright 2018 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $NODERAWFS__deps: ['$ERRNO_CODES', '$FS', '$NODEFS', '$mmapAlloc', '$FS_modeStringToFlags'],
  $NODERAWFS__postset: `
    if (ENVIRONMENT_IS_NODE) {
      var _wrapNodeError = function(func) {
        return function() {
          try {
            return func.apply(this, arguments)
          } catch (e) {
            if (e.code) {
              throw new FS.ErrnoError(ERRNO_CODES[e.code]);
            }
            throw e;
          }
        }
      };
      var VFS = Object.assign({}, FS);
      for (var _key in NODERAWFS) {
        FS[_key] = _wrapNodeError(NODERAWFS[_key]);
      }
    } else {
      throw new Error("NODERAWFS is currently only supported on Node.js environment.")
    }`,
  $NODERAWFS: {
    lookup(parent, name) {
#if ASSERTIONS
      assert(parent)
      assert(parent.path)
#endif
      return FS.lookupPath(`${parent.path}/${name}`).node;
    },
    lookupPath(path, opts = {}) {
      if (opts.parent) {
        path = nodePath.dirname(path);
      }
      var st = fs.lstatSync(path);
      var mode = NODEFS.getMode(path);
      return { path, node: { id: st.ino, mode, node_ops: NODERAWFS, path }};
    },
    createStandardStreams() {
      FS.createStream({ nfd: 0, position: 0, path: '', flags: 0, tty: true, seekable: false }, 0);
      for (var i = 1; i < 3; i++) {
        FS.createStream({ nfd: i, position: 0, path: '', flags: 577, tty: true, seekable: false }, i);
      }
    },
    // generic function for all node creation
    cwd() { return process.cwd(); },
    chdir() { process.chdir.apply(void 0, arguments); },
    mknod(path, mode) {
      if (FS.isDir(path)) {
        fs.mkdirSync(path, mode);
      } else {
        fs.writeFileSync(path, '', { mode: mode });
      }
    },
    mkdir() { fs.mkdirSync.apply(void 0, arguments); },
    symlink() { fs.symlinkSync.apply(void 0, arguments); },
    rename() { fs.renameSync.apply(void 0, arguments); },
    rmdir() { fs.rmdirSync.apply(void 0, arguments); },
    readdir() { return ['.', '..'].concat(fs.readdirSync.apply(void 0, arguments)); },
    unlink() { fs.unlinkSync.apply(void 0, arguments); },
    readlink() { return fs.readlinkSync.apply(void 0, arguments); },
    stat() { return fs.statSync.apply(void 0, arguments); },
    lstat() { return fs.lstatSync.apply(void 0, arguments); },
    chmod() { fs.chmodSync.apply(void 0, arguments); },
    fchmod(fd, mode) {
      var stream = FS.getStreamChecked(fd);
      fs.fchmodSync(stream.nfd, mode);
    },
    chown() { fs.chownSync.apply(void 0, arguments); },
    fchown(fd, owner, group) {
      var stream = FS.getStreamChecked(fd);
      fs.fchownSync(stream.nfd, owner, group);
    },
    truncate() { fs.truncateSync.apply(void 0, arguments); },
    ftruncate(fd, len) {
      // See https://github.com/nodejs/node/issues/35632
      if (len < 0) {
        throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
      }
      var stream = FS.getStreamChecked(fd);
      fs.ftruncateSync(stream.nfd, len);
    },
    utime(path, atime, mtime) { fs.utimesSync(path, atime/1000, mtime/1000); },
    open(path, flags, mode) {
      if (typeof flags == "string") {
        flags = FS_modeStringToFlags(flags)
      }
      var pathTruncated = path.split('/').map(function(s) { return s.substr(0, 255); }).join('/');
      var nfd = fs.openSync(pathTruncated, NODEFS.flagsForNode(flags), mode);
      var st = fs.fstatSync(nfd);
      if (flags & {{{ cDefs.O_DIRECTORY }}} && !st.isDirectory()) {
        fs.closeSync(nfd);
        throw new FS.ErrnoError(ERRNO_CODES.ENOTDIR);
      }
      var newMode = NODEFS.getMode(pathTruncated);
      var node = { id: st.ino, mode: newMode, node_ops: NODERAWFS, path }
      return FS.createStream({ nfd, position: 0, path, flags, node, seekable: true });
    },
    createStream(stream, fd) {
      // Call the original FS.createStream
      var rtn = VFS.createStream(stream, fd);
      if (typeof rtn.shared.refcnt == 'undefined') {
        rtn.shared.refcnt = 1;
      } else {
        rtn.shared.refcnt++;
      }
      return rtn;
    },
    close(stream) {
      VFS.closeStream(stream.fd);
      if (!stream.stream_ops && --stream.shared.refcnt === 0) {
        // This stream is created by our Node.js filesystem, close the
        // native file descriptor when its reference count drops to 0.
        fs.closeSync(stream.nfd);
      }
    },
    llseek(stream, offset, whence) {
      if (stream.stream_ops) {
        // this stream is created by in-memory filesystem
        return VFS.llseek(stream, offset, whence);
      }
      var position = offset;
      if (whence === {{{ cDefs.SEEK_CUR }}}) {
        position += stream.position;
      } else if (whence === {{{ cDefs.SEEK_END }}}) {
        position += fs.fstatSync(stream.nfd).size;
      } else if (whence !== {{{ cDefs.SEEK_SET }}}) {
        throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
      }

      if (position < 0) {
        throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
      }
      stream.position = position;
      return position;
    },
    read(stream, buffer, offset, length, position) {
      if (stream.stream_ops) {
        // this stream is created by in-memory filesystem
        return VFS.read(stream, buffer, offset, length, position);
      }
      var seeking = typeof position != 'undefined';
      if (!seeking && stream.seekable) position = stream.position;
      var bytesRead = fs.readSync(stream.nfd, Buffer.from(buffer.buffer), offset, length, position);
      // update position marker when non-seeking
      if (!seeking) stream.position += bytesRead;
      return bytesRead;
    },
    write(stream, buffer, offset, length, position) {
      if (stream.stream_ops) {
        // this stream is created by in-memory filesystem
        return VFS.write(stream, buffer, offset, length, position);
      }
      if (stream.flags & +"{{{ cDefs.O_APPEND }}}") {
        // seek to the end before writing in append mode
        FS.llseek(stream, 0, +"{{{ cDefs.SEEK_END }}}");
      }
      var seeking = typeof position != 'undefined';
      if (!seeking && stream.seekable) position = stream.position;
      var bytesWritten = fs.writeSync(stream.nfd, Buffer.from(buffer.buffer), offset, length, position);
      // update position marker when non-seeking
      if (!seeking) stream.position += bytesWritten;
      return bytesWritten;
    },
    allocate() {
      throw new FS.ErrnoError({{{ cDefs.EOPNOTSUPP }}});
    },
    mmap(stream, length, position, prot, flags) {
      if (stream.stream_ops) {
        // this stream is created by in-memory filesystem
        return VFS.mmap(stream, length, position, prot, flags);
      }

      var ptr = mmapAlloc(length);
      FS.read(stream, HEAP8, ptr, length, position);
      return { ptr, allocated: true };
    },
    msync(stream, buffer, offset, length, mmapFlags) {
      if (stream.stream_ops) {
        // this stream is created by in-memory filesystem
        return VFS.msync(stream, buffer, offset, length, mmapFlags);
      }

      FS.write(stream, buffer, 0, length, offset);
      // should we check if bytesWritten and length are the same?
      return 0;
    },
    munmap() {
      return 0;
    },
    ioctl() {
      throw new FS.ErrnoError({{{ cDefs.ENOTTY }}});
    }
  }
});
