/**
 * @license
 * Copyright 2021 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $SFAFS__deps: ['$PThreadFS'],
  $SFAFS: {

    benchmark: function(name, fct) {
      if('benchmark_results' in Module) {
        let time_pre = performance.now();
        let result = fct();
        let time_needed = performance.now() - time_pre;

        Module.benchmark_results[`${name}_time`] = (Module.benchmark_results[`${name}_time`] || 0) + time_needed;
        Module.benchmark_results[`${name}_num`] = (Module.benchmark_results[`${name}_num`] || 0) + 1;
        return result;
      }
      return fct();
    },

    /* Debugging */

    debug: function(...args) {
      // Uncomment to print debug information.
      //
      // console.log('SFAFS', arguments);
    },

    /* Helper functions */

    realPath: function(node) {
      var parts = [];
      while (node.parent !== node) {
        parts.push(node.name);
        node = node.parent;
      }
      if (!parts.length) {
        return '_';
      }
      parts.push('');
      parts.reverse();
      return parts.join('_');
    },

    encodedPath: function(node) {
      return SFAFS.encodePath(SFAFS.realPath(node));
    },

    joinPaths: function(path1, path2) {
      if (path1.endsWith('_')) {
        if (path2.startsWith('_')) {
          return path1.slice(0, -1) + path2;
        }
        return path1 + path2;
      } else {
        if (path2.startsWith('_')) {
          return path1 + path2;
        }
        return path1 + '_' + path2;
      }
    },

    // directoryPath ensures path ends with a path delimiter ('_').
    //
    // Example:
    // * directoryPath('_dir') = '_dir_'
    // * directoryPath('_dir_') = '_dir_'
    directoryPath: function(path) {
      if (path.length && path.slice(-1) == '_') {
        return path;
      }
      return path + '_';
    },

    // extractFilename strips the parent path and drops suffixes after '_'.
    //
    // Example:
    // * extractFilename('_dir', '_dir_myfile') = 'myfile'
    // * extractFilename('_dir', '_dir_mydir_myfile') = 'mydir'
    extractFilename: function(parent, path) {
      parent = SFAFS.directoryPath(parent);
      path = path.substr(parent.length);
      var index = path.indexOf('_');
      if (index == -1) {
        return path;
      }
      return path.substr(0, index);
    },

    encodePath: function(path) {
      //TODO: this is a random hex encoding decide and document on reasonable
      //scheme
      var s = unescape(encodeURIComponent(path))
      var h = ''
      for (var i = 0; i < s.length; i++) {
          h += s.charCodeAt(i).toString(16)
      }
      return h
    },

    decodePath: function(hex) {
      var s = ''
      for (var i = 0; i < hex.length; i+=2) {
          s += String.fromCharCode(parseInt(hex.substr(i, 2), 16))
      }
      return decodeURIComponent(escape(s))
    },


    listByPrefix: async function(prefix) {
      entries = await storageFoundation.getAll();
      return entries.filter(name => name.startsWith(prefix))
    },

    // Caches open file handles to simulate opening a file multiple times.
    openFileHandles: {},

    /* Filesystem implementation (public interface) */

    createNode: function (parent, name, mode, dev) {
      SFAFS.debug('createNode', arguments);
      if (!PThreadFS.isDir(mode) && !PThreadFS.isFile(mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      var node = PThreadFS.createNode(parent, name, mode);
      node.node_ops = SFAFS.node_ops;
      node.stream_ops = SFAFS.stream_ops;
      if (PThreadFS.isDir(mode)) {
        node.contents = {};
      }
      node.timestamp = Date.now();
      return node;
    },

    mount: function (mount) {
      SFAFS.debug('mount', arguments);
      return SFAFS.createNode(null, '/', {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */, 0);
    },

    cwd: function() { return process.cwd(); },

    chdir: function() { process.chdir.apply(void 0, arguments); },

    allocate: function() {
      SFAFS.debug('allocate', arguments);
      throw new PThreadFS.ErrnoError({{{ cDefine('EOPNOTSUPP') }}});
    },

    ioctl: function() {
      SFAFS.debug('ioctl', arguments);
      throw new PThreadFS.ErrnoError({{{ cDefine('ENOTTY') }}});
    },

    /* Operations on the nodes of the filesystem tree */

    node_ops: {
      getattr: async function(node) {
        SFAFS.debug('getattr', arguments);
        let length;
        if (node.handle) {
          length = await node.handle.getLength();
        } 
        else {
          // TODO: this double caching of opened files is probably redundant.
          // Clean up after publishing a clean design for the PThreadFS.
          var path = SFAFS.realPath(node);
          if(path in SFAFS.openFileHandles) {
            let fileHandle = SFAFS.openFileHandles[path]
            length = await fileHandle.getLength();
          } 
          else {
            let fileHandle = await storageFoundation.open(SFAFS.encodePath(path));
            length = await fileHandle.getLength();
            await fileHandle.close();
          }
        }

        let modificationTime = new Date(node.timestamp);
        return {
          dev: null,
          ino: null,
          mode: node.mode,
          nlink: 1,
          uid: 0,
          gid: 0,
          rdev: null,
          size: length,
          atime: modificationTime,
          mtime: modificationTime,
          ctime: modificationTime,
          blksize: 4096,
          blocks: Math.ceil(length / 4096),
        };
      },

      setattr: async function(node, attr) {
        SFAFS.debug('setattr', arguments);
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
              fileHandle = await storageFoundation.open(SFAFS.encodedPath(node));
            }
            await fileHandle.setLength(attr.size);
            
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
        SFAFS.debug('lookup', arguments);
        var parentPath = SFAFS.directoryPath(SFAFS.realPath(parent));

        var children = await SFAFS.listByPrefix(parentPath);

        var exists = false;
        var mode = 511 /* 0777 */
        for (var i = 0; i < children.length; ++i) {
          var path = children[i].substr(parentPath.length);
          if (path == name) {
            exists = true;
            mode |= {{{ cDefine('S_IFREG') }}};
            break;
          }

          subdirName = SFAFS.directoryPath(name);
          if (path.startsWith(subdirName)) {
            exists = true;
            mode |= {{{ cDefine('S_IFDIR') }}};
            break;
          }
        }

        if (!exists) {
          throw PThreadFS.genericErrors[{{{ cDefine('ENOENT') }}}];
        }

        var node = PThreadFS.createNode(parent, name, mode);
        node.node_ops = SFAFS.node_ops;
        node.stream_ops = SFAFS.stream_ops;
        return node;
      },

      mknod: function (parent, name, mode, dev) {
        SFAFS.debug('mknod', arguments);
        var node = SFAFS.createNode(parent, name, mode, dev);
        if (!PThreadFS.isFile) {
          console.log('SFAFS error: mknod is only implemented for files')
          throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
        }

        node.handle = null;
        node.refcount = 0;
        return node;
      },

      rename: function (oldNode, newParentNode, newName) {
        // TODO(rstz): Use storageFoundation.rename() to implement this.
        SFAFS.debug('rename', arguments);
        console.log('SFAFS error: rename is not implemented')
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
      },

      unlink: async function(parent, name) {
        SFAFS.debug('unlink', arguments);
        var path = SFAFS.joinPaths(SFAFS.realPath(parent), name);
        return await storageFoundation.delete(SFAFS.encodePath(path));
      },

      rmdir: function(parent, name) {
        SFAFS.debug('rmdir', arguments);
        console.log('SFAFS error: rmdir is not implemented')
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
      },

      readdir: async function(node) {
        SFAFS.debug('readdir', arguments);
        var parentPath = SFAFS.realPath(node);
        var children = await SFAFS.listByPrefix(SFAFS.encodePath(parentPath));
        children = children.map(child => SFAFS.extractFilename(parentPath, child));
        // Remove duplicates.
        return Array.from(new Set(children));
      },

      symlink: function(parent, newName, oldPath) {
        console.log('SFAFS error: symlink is not implemented')
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
      },

      readlink: function(node) {
        console.log('SFAFS error: readlink is not implemented')
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
      },
    },

    /* Operations on file streams (i.e., file handles) */

    stream_ops: {
      open: async function (stream) {
        SFAFS.debug('open', arguments);
        if (!PThreadFS.isFile(stream.node.mode)) {
          console.log('SFAFS error: open is only implemented for files')
          throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
        }

        if (stream.node.handle) {
          //TODO: check when this code path is actually executed, it seems to
          //duplicate some of the caching behavior below.
          stream.handle = stream.node.handle;
          ++stream.node.refcount;
        } else {
          var path = SFAFS.realPath(stream.node);

          // Open existing file.
          if(!(path in SFAFS.openFileHandles)) {
            SFAFS.openFileHandles[path] = await storageFoundation.open(SFAFS.encodePath(path));
          }
          stream.handle = SFAFS.openFileHandles[path];
          stream.node.handle = stream.handle;
          stream.node.refcount = 1;
        }
        SFAFS.debug('end open');
      },

      close: async function (stream) {
        SFAFS.debug('close', arguments);
        if (!PThreadFS.isFile(stream.node.mode)) {
          console.log('SFAFS error: close is only implemented for files');
          throw new PThreadFS.ErrnoError({{{ cDefine('ENOSYS') }}});
        }

        stream.handle = null;
        --stream.node.refcount;
        if (stream.node.refcount <= 0) {
          await stream.node.handle.close();
          stream.node.handle = null;
          delete SFAFS.openFileHandles[SFAFS.realPath(stream.node)];
        }
        SFAFS.debug('end close');
      },

      fsync: async function(stream) {
        SFAFS.debug('fsync', arguments);
        if (stream.handle == null) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
        }
        await stream.handle.flush();
        SFAFS.debug('end fsync');
        return 0;
      },

      read: async function (stream, buffer, offset, length, position) {
        SFAFS.debug('read', arguments);
        let data = new Uint8Array(length);
        let result = await stream.handle.read(data, position);
        buffer.set(result.buffer, offset);
        SFAFS.debug('end read');
        return result.readBytes;
      },

      write: async function (stream, buffer, offset, length, position) {
        SFAFS.debug('write', arguments);
        stream.node.timestamp = Date.now();
        let data = new Uint8Array(buffer.slice(offset, offset+length));
        let result = await stream.handle.write(data, position);
        SFAFS.debug('end write');
        return result.writtenBytes;
      },

      llseek: async function (stream, offset, whence) {
        SFAFS.debug('llseek', arguments);
        var position = offset;
        if (whence === 1) {  // SEEK_CUR.
          position += stream.position;
        } else if (whence === 2) {  // SEEK_END.
          position += await stream.handle.getLength();
        } else if (whence !== 0) {  // SEEK_SET.
          throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
        }

        if (position < 0) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
        }
        stream.position = position;
        SFAFS.debug('end llseek');
        return position;
      },

      mmap: function(stream, buffer, offset, length, position, prot, flags) {
        SFAFS.debug('mmap', arguments);
        throw new PThreadFS.ErrnoError({{{ cDefine('EOPNOTSUPP') }}});
      },

      msync: function(stream, buffer, offset, length, mmapFlags) {
        SFAFS.debug('msync', arguments);
        throw new PThreadFS.ErrnoError({{{ cDefine('EOPNOTSUPP') }}});
      },

      munmap: function(stream) {
        SFAFS.debug('munmap', arguments);
        throw new PThreadFS.ErrnoError({{{ cDefine('EOPNOTSUPP') }}});
      },
    }
  }
});