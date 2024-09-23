/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $WORKERFS__deps: ['$FS'],
  $WORKERFS: {
    DIR_MODE: {{{ cDefs.S_IFDIR }}} | 511 /* 0777 */,
    FILE_MODE: {{{ cDefs.S_IFREG }}} | 511 /* 0777 */,
    reader: null,
    mount(mount) {
      assert(ENVIRONMENT_IS_WORKER);
      WORKERFS.reader ??= new FileReaderSync();
      var root = WORKERFS.createNode(null, '/', WORKERFS.DIR_MODE, 0);
      var createdParents = {};
      function ensureParent(path) {
        // return the parent node, creating subdirs as necessary
        var parts = path.split('/');
        var parent = root;
        for (var i = 0; i < parts.length-1; i++) {
          var curr = parts.slice(0, i+1).join('/');
          // Issue 4254: Using curr as a node name will prevent the node
          // from being found in FS.nameTable when FS.open is called on
          // a path which holds a child of this node,
          // given that all FS functions assume node names
          // are just their corresponding parts within their given path,
          // rather than incremental aggregates which include their parent's
          // directories.
          createdParents[curr] ||= WORKERFS.createNode(parent, parts[i], WORKERFS.DIR_MODE, 0);
          parent = createdParents[curr];
        }
        return parent;
      }
      function base(path) {
        var parts = path.split('/');
        return parts[parts.length-1];
      }
      // We also accept FileList here, by using Array.prototype
      Array.prototype.forEach.call(mount.opts["files"] || [], function(file) {
        WORKERFS.createNode(ensureParent(file.name), base(file.name), WORKERFS.FILE_MODE, 0, file, file.lastModifiedDate);
      });
      (mount.opts["blobs"] || []).forEach((obj) => {
        WORKERFS.createNode(ensureParent(obj["name"]), base(obj["name"]), WORKERFS.FILE_MODE, 0, obj["data"]);
      });
      (mount.opts["packages"] || []).forEach((pack) => {
        pack['metadata'].files.forEach((file) => {
          var name = file.filename.substr(1); // remove initial slash
          WORKERFS.createNode(ensureParent(name), base(name), WORKERFS.FILE_MODE, 0, pack['blob'].slice(file.start, file.end));
        });
      });
      return root;
    },
    createNode(parent, name, mode, dev, contents, mtime) {
      var node = FS.createNode(parent, name, mode);
      node.mode = mode;
      node.node_ops = WORKERFS.node_ops;
      node.stream_ops = WORKERFS.stream_ops;
      node.timestamp = (mtime || new Date).getTime();
      assert(WORKERFS.FILE_MODE !== WORKERFS.DIR_MODE);
      if (mode === WORKERFS.FILE_MODE) {
        node.size = contents.size;
        node.contents = contents;
      } else {
        node.size = 4096;
        node.contents = {};
      }
      if (parent) {
        parent.contents[name] = node;
      }
      return node;
    },
    node_ops: {
      getattr(node) {
        return {
          dev: 1,
          ino: node.id,
          mode: node.mode,
          nlink: 1,
          uid: 0,
          gid: 0,
          rdev: 0,
          size: node.size,
          atime: new Date(node.timestamp),
          mtime: new Date(node.timestamp),
          ctime: new Date(node.timestamp),
          blksize: 4096,
          blocks: Math.ceil(node.size / 4096),
        };
      },
      setattr(node, attr) {
        if (attr.mode !== undefined) {
          node.mode = attr.mode;
        }
        if (attr.timestamp !== undefined) {
          node.timestamp = attr.timestamp;
        }
      },
      lookup(parent, name) {
        throw new FS.ErrnoError({{{ cDefs.ENOENT }}});
      },
      mknod(parent, name, mode, dev) {
        throw new FS.ErrnoError({{{ cDefs.EPERM }}});
      },
      rename(oldNode, newDir, newName) {
        throw new FS.ErrnoError({{{ cDefs.EPERM }}});
      },
      unlink(parent, name) {
        throw new FS.ErrnoError({{{ cDefs.EPERM }}});
      },
      rmdir(parent, name) {
        throw new FS.ErrnoError({{{ cDefs.EPERM }}});
      },
      readdir(node) {
        var entries = ['.', '..'];
        for (var key of Object.keys(node.contents)) {
          entries.push(key);
        }
        return entries;
      },
      symlink(parent, newName, oldPath) {
        throw new FS.ErrnoError({{{ cDefs.EPERM }}});
      },
    },
    stream_ops: {
      read(stream, buffer, offset, length, position) {
        if (position >= stream.node.size) return 0;
        var chunk = stream.node.contents.slice(position, position + length);
        var ab = WORKERFS.reader.readAsArrayBuffer(chunk);
        buffer.set(new Uint8Array(ab), offset);
        return chunk.size;
      },
      write(stream, buffer, offset, length, position) {
        throw new FS.ErrnoError({{{ cDefs.EIO }}});
      },
      llseek(stream, offset, whence) {
        var position = offset;
        if (whence === {{{ cDefs.SEEK_CUR }}}) {
          position += stream.position;
        } else if (whence === {{{ cDefs.SEEK_END }}}) {
          if (FS.isFile(stream.node.mode)) {
            position += stream.node.size;
          }
        }
        if (position < 0) {
          throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
        }
        return position;
      },
    },
  },
});

if (WASMFS) {
  error("using -lworkerfs is not currently supported in WasmFS.");
}
