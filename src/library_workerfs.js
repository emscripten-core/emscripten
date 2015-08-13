mergeInto(LibraryManager.library, {
  $WORKERFS__deps: ['$FS'],
  $WORKERFS: {
    DIR_MODE: {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */,
    FILE_MODE: {{{ cDefine('S_IFREG') }}} | 511 /* 0777 */,
    reader: null,
    mount: function (mount) {
      assert(ENVIRONMENT_IS_WORKER);
      if (!WORKERFS.reader) WORKERFS.reader = new FileReaderSync();
      var root = WORKERFS.createNode(null, '/', WORKERFS.DIR_MODE, 0);
      // We also accept FileList here.
      Array.prototype.forEach.call(mount.opts["files"] || [], function(file) {
        WORKERFS.createNode(root, file.name, WORKERFS.FILE_MODE, 0, file, file.lastModifiedDate);
      });
      (mount.opts["blobs"] || []).forEach(function(obj) {
        WORKERFS.createNode(root, obj["name"], WORKERFS.FILE_MODE, 0, obj["data"]);
      });
      return root;
    },
    createNode: function (parent, name, mode, dev, contents, mtime) {
      var node = FS.createNode(parent, name, mode);
      node.mode = mode;
      node.node_ops = WORKERFS.node_ops;
      node.stream_ops = WORKERFS.stream_ops;
      node.timestamp = (mtime || new Date).getTime();
      if (parent) {
        node.size = contents.size;
        node.contents = contents;
        parent.contents[name] = node;
      } else {
        node.size = 4096;
        node.contents = {};
      }
      return node;
    },
    node_ops: {
      getattr: function(node) {
        return {
          dev: 1,
          ino: undefined,
          mode: node.mode,
          nlink: 1,
          uid: 0,
          gid: 0,
          rdev: undefined,
          size: node.size,
          atime: new Date(node.timestamp),
          mtime: new Date(node.timestamp),
          ctime: new Date(node.timestamp),
          blksize: 4096,
          blocks: Math.ceil(node.size / 4096),
        };
      },
      setattr: function(node, attr) {
        if (attr.mode !== undefined) {
          node.mode = attr.mode;
        }
        if (attr.timestamp !== undefined) {
          node.timestamp = attr.timestamp;
        }
      },
      lookup: function(parent, name) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
      },
      mknod: function (parent, name, mode, dev) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
      rename: function (oldNode, newDir, newName) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
      unlink: function(parent, name) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
      rmdir: function(parent, name) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
      readdir: function(node) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
      symlink: function(parent, newName, oldPath) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
      readlink: function(node) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      },
    },
    stream_ops: {
      read: function (stream, buffer, offset, length, position) {
        if (position >= stream.node.size) return 0;
        var chunk = stream.node.contents.slice(position, position + length);
        var ab = WORKERFS.reader.readAsArrayBuffer(chunk);
        buffer.set(new Uint8Array(ab), offset);
        return chunk.size;
      },
      write: function (stream, buffer, offset, length, position) {
        throw new FS.ErrnoError(ERRNO_CODES.EIO);
      },
      llseek: function (stream, offset, whence) {
        var position = offset;
        if (whence === 1) {  // SEEK_CUR.
          position += stream.position;
        } else if (whence === 2) {  // SEEK_END.
          if (FS.isFile(stream.node.mode)) {
            position += stream.node.size;
          }
        }
        if (position < 0) {
          throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
        }
        return position;
      },
    },
  },
});
