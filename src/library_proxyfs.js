/**
 * @license
 * Copyright 2016 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $PROXYFS__deps: ['$FS', '$PATH', '$ERRNO_CODES'],
  $PROXYFS: {
    mount(mount) {
      return PROXYFS.createNode(null, '/', mount.opts.fs.lstat(mount.opts.root).mode, 0);
    },
    createNode(parent, name, mode, dev) {
      if (!FS.isDir(mode) && !FS.isFile(mode) && !FS.isLink(mode)) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      var node = FS.createNode(parent, name, mode);
      node.node_ops = PROXYFS.node_ops;
      node.stream_ops = PROXYFS.stream_ops;
      return node;
    },
    realPath(node) {
      var parts = [];
      while (node.parent !== node) {
        parts.push(node.name);
        node = node.parent;
      }
      parts.push(node.mount.opts.root);
      parts.reverse();
      return PATH.join(...parts);
    },
    node_ops: {
      getattr(node) {
        var path = PROXYFS.realPath(node);
        var stat;
        try {
          stat = node.mount.opts.fs.lstat(path);
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
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
      setattr(node, attr) {
        var path = PROXYFS.realPath(node);
        try {
          if (attr.mode !== undefined) {
            node.mount.opts.fs.chmod(path, attr.mode);
            // update the common node structure mode as well
            node.mode = attr.mode;
          }
          if (attr.timestamp !== undefined) {
            var date = new Date(attr.timestamp);
            node.mount.opts.fs.utime(path, date, date);
          }
          if (attr.size !== undefined) {
            node.mount.opts.fs.truncate(path, attr.size);
          }
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
      },
      lookup(parent, name) {
        try {
          var path = PATH.join2(PROXYFS.realPath(parent), name);
          var mode = parent.mount.opts.fs.lstat(path).mode;
          var node = PROXYFS.createNode(parent, name, mode);
          return node;
        } catch(e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
      },
      mknod(parent, name, mode, dev) {
        var node = PROXYFS.createNode(parent, name, mode, dev);
        // create the backing node for this in the fs root as well
        var path = PROXYFS.realPath(node);
        try {
          if (FS.isDir(node.mode)) {
            node.mount.opts.fs.mkdir(path, node.mode);
          } else {
            node.mount.opts.fs.writeFile(path, '', { mode: node.mode });
          }
        } catch (e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
        return node;
      },
      rename(oldNode, newDir, newName) {
        var oldPath = PROXYFS.realPath(oldNode);
        var newPath = PATH.join2(PROXYFS.realPath(newDir), newName);
        try {
          oldNode.mount.opts.fs.rename(oldPath, newPath);
          oldNode.name = newName;
        } catch(e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
      },
      unlink(parent, name) {
        var path = PATH.join2(PROXYFS.realPath(parent), name);
        try {
          parent.mount.opts.fs.unlink(path);
        } catch(e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
      },
      rmdir(parent, name) {
        var path = PATH.join2(PROXYFS.realPath(parent), name);
        try {
          parent.mount.opts.fs.rmdir(path);
        } catch(e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
      },
      readdir(node) {
        var path = PROXYFS.realPath(node);
        try {
          return node.mount.opts.fs.readdir(path);
        } catch(e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
      },
      symlink(parent, newName, oldPath) {
        var newPath = PATH.join2(PROXYFS.realPath(parent), newName);
        try {
          parent.mount.opts.fs.symlink(oldPath, newPath);
        } catch(e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
      },
      readlink(node) {
        var path = PROXYFS.realPath(node);
        try {
          return node.mount.opts.fs.readlink(path);
        } catch(e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
      },
    },
    stream_ops: {
      open(stream) {
        var path = PROXYFS.realPath(stream.node);
        try {
          stream.nfd = stream.node.mount.opts.fs.open(path,stream.flags);
        } catch(e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
      },
      close(stream) {
        try {
          stream.node.mount.opts.fs.close(stream.nfd);
        } catch(e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
      },
      read(stream, buffer, offset, length, position) {
        try {
          return stream.node.mount.opts.fs.read(stream.nfd, buffer, offset, length, position);
        } catch(e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
      },
      write(stream, buffer, offset, length, position) {
        try {
          return stream.node.mount.opts.fs.write(stream.nfd, buffer, offset, length, position);
        } catch(e) {
          if (!e.code) throw e;
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
      },
      llseek(stream, offset, whence) {
        var position = offset;
        if (whence === {{{ cDefs.SEEK_CUR }}}) {
          position += stream.position;
        } else if (whence === {{{ cDefs.SEEK_END }}}) {
          if (FS.isFile(stream.node.mode)) {
            try {
              var stat = stream.node.node_ops.getattr(stream.node);
              position += stat.size;
            } catch (e) {
              throw new FS.ErrnoError(ERRNO_CODES[e.code]);
            }
          }
        }

        if (position < 0) {
          throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
        }

        return position;
      }
    }
  }
});

if (WASMFS) {
  error("using -lproxyfs is not currently supported in WasmFS.");
}
