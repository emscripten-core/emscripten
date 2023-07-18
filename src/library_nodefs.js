/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $NODEFS__deps: ['$FS', '$PATH', '$ERRNO_CODES', '$mmapAlloc'],
  $NODEFS__postset: 'if (ENVIRONMENT_IS_NODE) { NODEFS.staticInit(); }',
  $NODEFS: {
    isWindows: false,
    staticInit: () => {
      NODEFS.isWindows = !!process.platform.match(/^win/);
      var flags = process.binding("constants");
      // Node.js 4 compatibility: it has no namespaces for constants
      if (flags["fs"]) {
        flags = flags["fs"];
      }
      NODEFS.flagsForNodeMap = {
        "{{{ cDefs.O_APPEND }}}": flags["O_APPEND"],
        "{{{ cDefs.O_CREAT }}}": flags["O_CREAT"],
        "{{{ cDefs.O_EXCL }}}": flags["O_EXCL"],
        "{{{ cDefs.O_NOCTTY }}}": flags["O_NOCTTY"],
        "{{{ cDefs.O_RDONLY }}}": flags["O_RDONLY"],
        "{{{ cDefs.O_RDWR }}}": flags["O_RDWR"],
        "{{{ cDefs.O_DSYNC }}}": flags["O_SYNC"],
        "{{{ cDefs.O_TRUNC }}}": flags["O_TRUNC"],
        "{{{ cDefs.O_WRONLY }}}": flags["O_WRONLY"],
        "{{{ cDefs.O_NOFOLLOW }}}": flags["O_NOFOLLOW"],
      };
#if ASSERTIONS
      // The 0 define must match on both sides, as otherwise we would not
      // know to add it.
      assert(NODEFS.flagsForNodeMap["0"] === 0);
#endif
    },
    convertNodeCode: (e) => {
      var code = e.code;
#if ASSERTIONS
      assert(code in ERRNO_CODES, `unexpected node error code: ${code} (${e})`);
#endif
      return ERRNO_CODES[code];
    },
    mount: (mount) => {
#if ASSERTIONS
      assert(ENVIRONMENT_IS_NODE);
#endif
      return NODEFS.createNode(null, '/', NODEFS.getMode(mount.opts.root), 0);
    },
    createNode: (parent, name, mode, dev) => {
      if (!FS.isDir(mode) && !FS.isFile(mode) && !FS.isLink(mode)) {
        throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
      }
      var node = FS.createNode(parent, name, mode);
      node.node_ops = NODEFS.node_ops;
      node.stream_ops = NODEFS.stream_ops;
      return node;
    },
    getMode: (path) => {
      var stat;
      try {
        stat = fs.lstatSync(path);
        if (NODEFS.isWindows) {
          // Node.js on Windows never represents permission bit 'x', so
          // propagate read bits to execute bits
          stat.mode = stat.mode | ((stat.mode & 292) >> 2);
        }
      } catch (e) {
        if (!e.code) throw e;
        throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
      }
      return stat.mode;
    },
    realPath: (node) => {
      var parts = [];
      while (node.parent !== node) {
        parts.push(node.name);
        node = node.parent;
      }
      parts.push(node.mount.opts.root);
      parts.reverse();
      return PATH.join.apply(null, parts);
    },
    // This maps the integer permission modes from http://linux.die.net/man/3/open
    // to node.js-specific file open permission strings at http://nodejs.org/api/fs.html#fs_fs_open_path_flags_mode_callback
    flagsForNode: (flags) => {
      flags &= ~{{{ cDefs.O_PATH }}}; // Ignore this flag from musl, otherwise node.js fails to open the file.
      flags &= ~{{{ cDefs.O_NONBLOCK }}}; // Ignore this flag from musl, otherwise node.js fails to open the file.
      flags &= ~{{{ cDefs.O_LARGEFILE }}}; // Ignore this flag from musl, otherwise node.js fails to open the file.
      flags &= ~{{{ cDefs.O_CLOEXEC }}}; // Some applications may pass it; it makes no sense for a single process.
      flags &= ~{{{ cDefs.O_DIRECTORY }}}; // Node.js doesn't need this passed in, it errors.
      var newFlags = 0;
      for (var k in NODEFS.flagsForNodeMap) {
        if (flags & k) {
          newFlags |= NODEFS.flagsForNodeMap[k];
          flags ^= k;
        }
      }
      if (flags) {
        throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
      }
      return newFlags;
    },
    node_ops: {
      getattr: (node) => {
        var path = NODEFS.realPath(node);
        var stat;
        try {
          stat = fs.lstatSync(path);
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
        // node.js v0.10.20 doesn't report blksize and blocks on Windows. Fake them with default blksize of 4096.
        // See http://support.microsoft.com/kb/140365
        if (NODEFS.isWindows && !stat.blksize) {
          stat.blksize = 4096;
        }
        if (NODEFS.isWindows && !stat.blocks) {
          stat.blocks = (stat.size+stat.blksize-1)/stat.blksize|0;
        }
        return {
          dev: stat.dev,
          ino: stat.ino,
          mode: stat.mode,
          nlink: stat.nlink,
          uid: stat.uid,
          gid: stat.gid,
          rdev: stat.rdev,
          size: stat.size,
          atime: stat.atime,
          mtime: stat.mtime,
          ctime: stat.ctime,
          blksize: stat.blksize,
          blocks: stat.blocks
        };
      },
      setattr: (node, attr) => {
        var path = NODEFS.realPath(node);
        try {
          if (attr.mode !== undefined) {
            fs.chmodSync(path, attr.mode);
            // update the common node structure mode as well
            node.mode = attr.mode;
          }
          if (attr.timestamp !== undefined) {
            var date = new Date(attr.timestamp);
            fs.utimesSync(path, date, date);
          }
          if (attr.size !== undefined) {
            fs.truncateSync(path, attr.size);
          }
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
      },
      lookup: (parent, name) => {
        var path = PATH.join2(NODEFS.realPath(parent), name);
        var mode = NODEFS.getMode(path);
        return NODEFS.createNode(parent, name, mode);
      },
      mknod: (parent, name, mode, dev) => {
        var node = NODEFS.createNode(parent, name, mode, dev);
        // create the backing node for this in the fs root as well
        var path = NODEFS.realPath(node);
        try {
          if (FS.isDir(node.mode)) {
            fs.mkdirSync(path, node.mode);
          } else {
            fs.writeFileSync(path, '', { mode: node.mode });
          }
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
        return node;
      },
      rename: (oldNode, newDir, newName) => {
        var oldPath = NODEFS.realPath(oldNode);
        var newPath = PATH.join2(NODEFS.realPath(newDir), newName);
        try {
          fs.renameSync(oldPath, newPath);
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
        oldNode.name = newName;
      },
      unlink: (parent, name) => {
        var path = PATH.join2(NODEFS.realPath(parent), name);
        try {
          fs.unlinkSync(path);
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
      },
      rmdir: (parent, name) => {
        var path = PATH.join2(NODEFS.realPath(parent), name);
        try {
          fs.rmdirSync(path);
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
      },
      readdir: (node) => {
        var path = NODEFS.realPath(node);
        try {
          return fs.readdirSync(path);
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
      },
      symlink: (parent, newName, oldPath) => {
        var newPath = PATH.join2(NODEFS.realPath(parent), newName);
        try {
          fs.symlinkSync(oldPath, newPath);
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
      },
      readlink: (node) => {
        var path = NODEFS.realPath(node);
        try {
          path = fs.readlinkSync(path);
          path = nodePath.relative(nodePath.resolve(node.mount.opts.root), path);
          return path;
        } catch (e) {
          if (!e.code) throw e;
          // node under windows can return code 'UNKNOWN' here:
          // https://github.com/emscripten-core/emscripten/issues/15468
          if (e.code === 'UNKNOWN') throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
      },
    },
    stream_ops: {
      open: (stream) => {
        var path = NODEFS.realPath(stream.node);
        try {
          if (FS.isFile(stream.node.mode)) {
            stream.nfd = fs.openSync(path, NODEFS.flagsForNode(stream.flags));
          }
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
      },
      close: (stream) => {
        try {
          if (FS.isFile(stream.node.mode) && stream.nfd) {
            fs.closeSync(stream.nfd);
          }
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
      },
      read: (stream, buffer, offset, length, position) => {
        // Node.js < 6 compatibility: node errors on 0 length reads
        if (length === 0) return 0;
        try {
          return fs.readSync(stream.nfd, Buffer.from(buffer.buffer), offset, length, position);
        } catch (e) {
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
      },
      write: (stream, buffer, offset, length, position) => {
        try {
          return fs.writeSync(stream.nfd, Buffer.from(buffer.buffer), offset, length, position);
        } catch (e) {
          throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
        }
      },
      llseek: (stream, offset, whence) => {
        var position = offset;
        if (whence === {{{ cDefs.SEEK_CUR }}}) {
          position += stream.position;
        } else if (whence === {{{ cDefs.SEEK_END }}}) {
          if (FS.isFile(stream.node.mode)) {
            try {
              var stat = fs.fstatSync(stream.nfd);
              position += stat.size;
            } catch (e) {
              throw new FS.ErrnoError(NODEFS.convertNodeCode(e));
            }
          }
        }

        if (position < 0) {
          throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
        }

        return position;
      },
      mmap: (stream, length, position, prot, flags) => {
        if (!FS.isFile(stream.node.mode)) {
          throw new FS.ErrnoError({{{ cDefs.ENODEV }}});
        }

        var ptr = mmapAlloc(length);

        NODEFS.stream_ops.read(stream, HEAP8, ptr, length, position);
        return { ptr, allocated: true };
      },
      msync: (stream, buffer, offset, length, mmapFlags) => {
        NODEFS.stream_ops.write(stream, buffer, 0, length, offset, false);
        // should we check if bytesWritten and length are the same?
        return 0;
      }
    }
  }
});
